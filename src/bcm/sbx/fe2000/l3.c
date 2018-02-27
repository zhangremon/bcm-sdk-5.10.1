/*
 * $Id: l3.c 1.115.2.1 Broadcom SDK $
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
 * File:    l3.c
 * Purpose: Manages L3 interface table, forwarding table, routing table
 */

#include <soc/defs.h>

#ifdef INCLUDE_L3

#include <soc/drv.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/fe2000.h>

#include <soc/enet.h>

#ifdef BCM_FE2000_P3_SUPPORT
#include <soc/sbx/g2p3/g2p3.h>
#endif

#include <bcm_int/sbx/error.h>

#include <shared/gport.h>
#include <bcm/types.h>
#include <bcm/module.h>
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

#include <bcm_int/sbx/fe2000/allocator.h>
#include <bcm_int/sbx/fe2000/l3.h>
#include <bcm_int/sbx/fe2000/port.h>
#include <bcm_int/sbx/state.h>

#ifdef BCM_FE2000_P3_SUPPORT
#include <bcm_int/sbx/fe2000/g2p3.h>
#endif


#define L3_DEBUG(flags, stuff) BCM_DEBUG(flags | BCM_DBG_L3, stuff)
#define L3_NOTICE(stuff)        L3_DEBUG(BCM_DBG_NOTICE, stuff)
#define L3_WARN(stuff)          L3_DEBUG(BCM_DBG_WARN, stuff)
#define L3_ERR(stuff)           L3_DEBUG(BCM_DBG_ERR, stuff)
#define L3_VERB(stuff)          L3_DEBUG(BCM_DBG_VERBOSE, stuff)
#define L3_VVERB(stuff)        L3_DEBUG(BCM_DBG_VVERBOSE, stuff)

#define MAC_FMT       "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x"
#define MAC_PFMT(mac) (mac)[0], (mac)[1], (mac)[2], \
                      (mac)[3], (mac)[4], (mac)[5]


/*
 * XXX: Move these
 */


#define V4UC_STR_SEL_RULE 0
/*
 * Local Defines
 */
#define FE2K_TUNNEL_SUPPORTED_FLAGS  (BCM_TUNNEL_REPLACE)
#define L3_INTF_SUPPORTED_FLAGS  (BCM_L3_UNTAG      | \
                                  BCM_L3_ADD_TO_ARL | \
                                  BCM_L3_WITH_ID    | \
                                  BCM_L3_REPLACE    | \
                                  BCM_L3_IP6        | \
                                  BCM_L3_RPE)

#define L3_ROUTE_SUPPORTED_FLAGS (BCM_L3_REPLACE      | \
                                  BCM_L3_RPF          | \
                                  BCM_L3_TGID         | \
                                  BCM_L3_MULTIPATH    | \
                                  BCM_L3_IP6          | \
                                  BCM_L3_SRC_DISCARD)

#define L3_INTF_LOCATE_BY_IFID                1
#define L3_INTF_LOCATE_BY_VID                 2

#define L3_EGR_INTF_CHG                    0x01
#define L3_EGR_DMAC_CHG                    0x02
#define L3_EGR_MODULE_CHG                  0x04
#define L3_EGR_PORT_CHG                    0x08
#define L3_EGR_TRUNK_CHG                   0x10
#define L3_EGR_TGID_CHG                    0x20
#define L3_EGR_OHI_CHG                     0x40
#define L3_EGR_VIDOP_CHG                   0x80


#define _FE2K_INCR_FTE_REFCNT                 1
#define _FE2K_DECR_FTE_REFCNT                 2


#define L3_CRC_INFO_DONT_FETCH_FTE_DATA       0
#define L3_CRC_INFO_FETCH_FTE_DATA            1

#define L3_WB_VERSION_1_0                SOC_SCACHE_VERSION(1,0)
#define L3_WB_CURRENT_VERSION            L3_WB_VERSION_1_0

/* Assume that bcm param is in network order */
#define _FE2K_L3_CMP_BCM_SB_IPV4(b4, s4)                  \
    (((s4).uByte0 == ((uint8 *)&(b4))[0]) &&              \
     ((s4).uByte1 == ((uint8 *)&(b4))[1]) &&              \
     ((s4).uByte2 == ((uint8 *)&(b4))[2]) &&              \
     ((s4).uByte3 == ((uint8 *)&(b4))[3]))

#define _FE2K_L3_BCM_TO_SB_IPV4(b4, s4)                   \
    do                                  {                 \
        (s4).uByte0 = ((uint8 *)&(b4))[0];                \
        (s4).uByte1 = ((uint8 *)&(b4))[1];                \
        (s4).uByte2 = ((uint8 *)&(b4))[2];                \
        (s4).uByte3 = ((uint8 *)&(b4))[3];                \
    } while (0)

#define _FE2K_L3_SB_IPV4_TO_BCM(s4, b4)                   \
    do                                  {                 \
        ((uint8 *)&(b4))[0] = (s4).uByte0;                \
        ((uint8 *)&(b4))[1] = (s4).uByte1;                \
        ((uint8 *)&(b4))[2] = (s4).uByte2;                \
        ((uint8 *)&(b4))[3] = (s4).uByte3;                \
    } while (0)

#define _FE2K_L3_GET_ETE_RESOURCE_TYPE(ete_type, resource_type)  \
    do {                                                         \
       if (((ete_type) == _FE2K_L3_ETE__UCAST_IP) ||             \
           ((ete_type) == _FE2K_L3_ETE__MCAST_IP) ||             \
           ((ete_type) == _FE2K_L3_ETE__ENCAP_IP) ||             \
           ((ete_type) == _FE2K_L3_ETE__ENCAP_MPLS)) {           \
           (resource_type) = SBX_GU2K_USR_RES_ETE_ENCAP;         \
       } else {                                                  \
            (resource_type)  = SBX_GU2K_USR_RES_MAX;             \
       }                                                         \
    } while (0);

#define _FE2K_MAP_HOST_INFO_TO_ROUTE_INFO(hinfo, rinfo)   \
    do {                                                       \
        (rinfo)->l3a_flags      =  (hinfo)->l3a_flags;         \
        (rinfo)->l3a_vrf        =  (hinfo)->l3a_vrf;           \
        (rinfo)->l3a_subnet     =  (hinfo)->l3a_ip_addr;       \
        sal_memcpy((rinfo)->l3a_ip6_net, (hinfo)->l3a_ip6_addr,\
                   sizeof(bcm_ip6_t));                         \
        (rinfo)->l3a_ip_mask    =  0xffffffff;                 \
        sal_memset((rinfo)->l3a_ip6_mask, 0xff,                \
                   sizeof(bcm_ip6_t));                         \
        (rinfo)->l3a_intf       =  (hinfo)->l3a_intf;          \
        (rinfo)->l3a_pri        =  (hinfo)->l3a_pri;           \
        (rinfo)->l3a_modid      =  (hinfo)->l3a_modid;         \
        (rinfo)->l3a_port_tgid  = (hinfo)->l3a_port_tgid;      \
    } while (0);


/*
 * recovery restrictions go here - we may not want support all possible
 * resources, maybe we do.   Let's allow a compile time option.
 */
#define L3_RCVR_MAX_IFID    (_SBX_GU2_MAX_VALID_IFID >> 5)

/*
 *   Version 1.0 of the scache layout for l3 level-2 recovery
 *  The version is stored in l3_fe->wb_cache->version
 *  This memory map is applied to l3_fe->wb_cache->cache;
 *  This memory map is used to define the amount of memory allocated for the 
 *    purposes of warmboot - if its not here - it's not stored.
 */
typedef struct fe2k_l3_wb_mem_layout_s {

    /* data required to recover l3 interfaces */
    /* Upgrade:  condense 17b entities; saves (max_ifid * 2 * 14) bits!  */
    struct ifdata {
        uint32  ete[_FE2K_L3_ETE_VIDOP__MAX_TYPES];
    } rif[L3_RCVR_MAX_IFID];

} fe2k_l3_wb_mem_layout_t;


/* total amount of persistent storage requiredf for l3 recovery */
#define L3_WB_SCACHE_SIZE  \
  (sizeof(fe2k_l3_wb_mem_layout_t) + SOC_WB_SCACHE_CONTROL_SIZE)

/* Is Level 2 warmboot configured - */
#define L3_WB_L2_CONFIGURED(l3_fe_) ((l3_fe_)->scache_size != 0)


/**
 * GLOBAL variables
 */
_l3_device_unit_t _l3_units[BCM_MAX_NUM_UNITS];
sal_mutex_t       _l3_mlock[BCM_MAX_NUM_UNITS];

/**
 * STATIC function declarations
 */
STATIC int
_bcm_fe2000_l3_update_intf(_fe2k_l3_fe_instance_t *l3_fe,
                           bcm_l3_intf_t          *bcm_intf,
                           _fe2k_l3_intf_t        *l3_intf);
STATIC int
_bcm_fe2000_l3_add_intf(_fe2k_l3_fe_instance_t *l3_fe,
                        bcm_l3_intf_t          *bcm_intf,
                        _fe2k_l3_intf_t        *l3_intf);
STATIC int
_bcm_fe2000_l3_alloc_intf(_fe2k_l3_fe_instance_t *l3_fe,
                          bcm_l3_intf_t          *bcm_intf,
                          uint32                 *reserve_etes,
                          _fe2k_l3_intf_t        **ret_intf);

STATIC int
_bcm_fe2000_link_fte2ete(_fe2k_l3_ete_t         *l3_ete,
                         _fe2k_l3_ete_fte_t     *module_fte);
STATIC int
_bcm_fe2000_unlink_fte2ete(_fe2k_l3_ete_t         *l3_ete,
                           _fe2k_l3_ete_fte_t     *module_fte);
STATIC int
_bcm_fe2000_free_l3_ete(_fe2k_l3_fe_instance_t *l3_fe,
                        _fe2k_l3_ete_t         **p_l3_sw_ete);


STATIC int
_bcm_fe2000_destroy_fte(_fe2k_l3_fe_instance_t *l3_fe,
                        int                     action,
                        uint32                  fte_idx,
                        bcm_module_t            module_id,
                        uint32                  ohi);

STATIC int
_bcm_fe2000_l3_mpath_get(_fe2k_l3_fe_instance_t *l3_fe,
                         uint32                 mpbase,
                         _fe2k_egr_mp_info_t    **ret_info);



const char *
_fe2k_ete_type_string_get(_fe2k_l3_ete_types_t type) {
    switch (type)
    {
    case _FE2K_L3_ETE__UCAST_IP: return   "_FE2K_L3_ETE__UCAST_IP";
    case _FE2K_L3_ETE__MCAST_IP: return   "_FE2K_L3_ETE__MCAST_IP";
    case _FE2K_L3_ETE__ENCAP_IP: return   "_FE2K_L3_ETE__ENCAP_IP";
    case _FE2K_L3_ETE__ENCAP_MPLS: return "_FE2K_L3_ETE__ENCAP_MPLS";
    default:
        return "unknown";
    }
}

STATIC int
_bcm_fe2000_ip_is_valid(bcm_l3_route_t *info)
{
    
    if (info->l3a_flags & BCM_L3_IP6) {
        bcm_ip6_t zero;
        int len = bcm_ip6_mask_length(info->l3a_ip6_mask);

        sal_memset(zero, 0, sizeof(bcm_ip6_t));

        return ((len == 128 || len <= 64)
                && ((BCM_IP6_ADDR_EQ(info->l3a_ip6_mask, zero) != 0) ||
                    (BCM_IP6_ADDR_EQ(info->l3a_ip6_net, zero) == 0)));
    } else  {
        return ((info->l3a_ip_mask != 0) || (info->l3a_subnet == 0));
    }
}


int
_bcm_fe2000_l3_do_lpm_commit(int unit, bcm_l3_route_t *info)
{

    /* For IPv4 we have only LPM and IPv6 maintains
       two seperate tables - LPM and host for subnet
       and host routest 
    */

    if (info->l3a_flags & BCM_L3_IP6) {
        bcm_ip6_t allOnes;
        sal_memset(allOnes, 0xFF, sizeof(bcm_ip6_t));
        if(BCM_IP6_ADDR_EQ(info->l3a_ip6_mask, allOnes)) {
            return ((SOC_SBX_STATE(unit)->cache_l3host == 0) ? TRUE: FALSE);
        } else {
            return ((SOC_SBX_STATE(unit)->cache_l3route == 0) ? TRUE: FALSE);
        }
    } else {
        return ((SOC_SBX_STATE(unit)->cache_l3route == 0) ? TRUE: FALSE);
    } 
}


#ifdef BCM_WARM_BOOT_SUPPORT
int
l3_wb_layout_get(int unit,  fe2k_l3_wb_mem_layout_t **layout)
{
    int rv;
    uint32 size;
    soc_wb_cache_t *wbc;
    _fe2k_l3_fe_instance_t *l3_fe;

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
       return  BCM_E_UNIT;
    }

    *layout = NULL;
    rv = soc_scache_ptr_get(unit, l3_fe->wb_hdl, (uint8**)&wbc, &size);
    if (BCM_FAILURE(rv)) {
        return rv; 
    }
    *layout = (fe2k_l3_wb_mem_layout_t *)wbc->cache;
    return rv;
}

#endif /* BCM_WARM_BOOT_SUPPORT */

#ifdef BCM_WARM_BOOT_SUPPORT
int
l3_wb_store_intf(_fe2k_l3_fe_instance_t *l3_fe, _fe2k_l3_intf_t *l3_intf)
{
    int  etei, unit = l3_fe->fe_unit;
    fe2k_l3_wb_mem_layout_t *layout = NULL;

    if (L3_WB_L2_CONFIGURED(l3_fe) == FALSE) {
        return BCM_E_NONE;
    }

    L3_VERB((_SBX_D(unit, "if=%d base_ete=0x%x\n"),
             l3_intf->if_info.l3a_intf_id, l3_intf->if_l2_ete[0]));

    if (l3_intf->if_info.l3a_intf_id >= L3_RCVR_MAX_IFID) {
        L3_ERR((_SBX_D(unit, "Insufficient space for %d interface ids"
                       " to be recovered.\n"), l3_intf->if_info.l3a_intf_id));
        return BCM_E_RESOURCE;
    }
    
    /* Ensure ete idx fits in a 17b int */
    assert ((l3_intf->if_l2_ete[0].ete_idx & ~0x1FFFF) == 0);

    l3_wb_layout_get(unit, &layout);
    soc_scache_handle_lock(unit, l3_fe->wb_hdl);

    for (etei=0; etei < _FE2K_L3_ETE_VIDOP__MAX_TYPES; etei++) {
        layout->rif[l3_intf->if_info.l3a_intf_id].ete[etei] = 
            l3_intf->if_l2_ete[etei].ete_idx;
    }

    soc_scache_handle_unlock(unit, l3_fe->wb_hdl);

    return BCM_E_NONE;
}
#endif /* BCM_WARM_BOOT_SUPPORT */   


#ifdef BCM_WARM_BOOT_SUPPORT
int
_bcm_fe2000_wb_recover(_fe2k_l3_fe_instance_t *l3_fe,
                       fe2k_l3_wb_mem_layout_t *layout)
{
    int                        etei, idx, rv = BCM_E_NONE;
    int                        unit = l3_fe->fe_unit;
    uint32                     etes[_FE2K_L3_ETE_VIDOP__MAX_TYPES];
    soc_sbx_g2p3_etel2_t       etel2;
    soc_sbx_g2p3_esmac_t       esmac;
    _fe2k_l3_intf_t            *l3_intf;
    bcm_l3_intf_t              bcm_intf;
    
    L3_VERB((_SBX_D(unit, "recovering and reconstructing interface ids\n")));
    for (idx = 0; idx < L3_RCVR_MAX_IFID; idx++) {
        if (layout->rif[idx].ete[0]) {
            
            for (etei=0; etei < _FE2K_L3_ETE_VIDOP__MAX_TYPES; etei++) {
                etes[etei] = layout->rif[idx].ete[etei];
            }

            rv = soc_sbx_g2p3_etel2_get(unit, etes[1], &etel2);
            if (BCM_FAILURE(rv)) {
                L3_ERR((_SBX_D(unit, "Failed to read etel2[0x%x]:%s\n"),
                        etes[1], bcm_errmsg(rv)));
                return rv;
            }
            
            rv = soc_sbx_g2p3_esmac_get(unit, etel2.smacindex, &esmac);
            if (BCM_FAILURE(rv)) {
                L3_ERR((_SBX_D(unit, "Failed to read esmac[0x%x]:%s\n"),
                        etel2.smacindex, bcm_errmsg(rv)));
                return rv;
            }

            bcm_l3_intf_t_init(&bcm_intf);
            bcm_intf.l3a_intf_id = idx;
            bcm_intf.l3a_mtu     = etel2.mtu;
            bcm_intf.l3a_flags   = BCM_L3_WITH_ID;
            bcm_intf.l3a_vid     = etel2.vid;
            bcm_intf.l3a_mac_addr[5] = esmac.smac5;
            bcm_intf.l3a_mac_addr[4] = esmac.smac4;
            bcm_intf.l3a_mac_addr[3] = esmac.smac3;
            bcm_intf.l3a_mac_addr[2] = esmac.smac2;
            bcm_intf.l3a_mac_addr[1] = esmac.smac1;
            bcm_intf.l3a_mac_addr[0] = esmac.smac0;

            rv = _bcm_fe2000_l3_alloc_intf(l3_fe, &bcm_intf, etes, &l3_intf);
            if (BCM_FAILURE(rv)) {
                L3_ERR((_SBX_D(l3_fe->fe_unit, "Failed to recover (alloc) intf "
                               "0x%x:%s\n"), idx, bcm_errmsg(rv)));
                return rv;
            }
                        
            rv = _bcm_fe2000_l3_add_intf(l3_fe, &bcm_intf, l3_intf);
            if (BCM_FAILURE(rv)) {
                L3_ERR((_SBX_D(l3_fe->fe_unit, "Failed to recover (add) intf 0x%x:%s\n"),
                        idx, bcm_errmsg(rv)));
                return rv;
            }

            L3_VERB((_SBX_D(l3_fe->fe_unit, "recovered l3 interface 0x%x\n"), idx));
            
        }
    }

    return rv;
}
#endif /* BCM_WARM_BOOT_SUPPORT */

void
_bcm_fe2000_l3_sw_dump(int unit)
{

}

/*
 * Function:
 *      _bcm_fe2000_get_first_ete_on_intf
 * Purpose:
 *      Get the first of given type
 * Parameters:
 *      l3_fe       - (IN)  l3 fe instance
 *      l3_intf     - (IN)  The egress L3 interface context
 *      ete_type    - (IN)  the type of ete we are looking for
 *      l3_ete      - (OUT) l3 ete, if found
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_fe2000_get_ete_by_type_on_intf(_fe2k_l3_fe_instance_t  *l3_fe,
                                    _fe2k_l3_intf_t         *l3_intf,
                                    _fe2k_l3_ete_types_t     ete_type,
                                    _fe2k_l3_ete_t         **l3_ete)
{
    int               i;
    _fe2k_l3_ete_t   *tmp_ete;

    *l3_ete = NULL;

    if (ete_type == _FE2K_L3_ETE__UCAST_IP) {
       if (l3_intf->if_ip_ete_count > 1) {
          return BCM_E_INTERNAL;
       }
    } else if (ete_type != _FE2K_L3_ETE__ENCAP_MPLS) {
        return BCM_E_INTERNAL;
    }

    tmp_ete = NULL;
    for (i = 0; i < _FE2K_INTF_L3ETE_HASH_SIZE; i++) {
        if (!(DQ_EMPTY(&l3_intf->if_ete_hash[i]))) {
            _FE2K_ALL_L3ETE_PER_IEH_BKT(l3_intf, i, tmp_ete) {
                if (tmp_ete->l3_ete_key.l3_ete_hk.type == ete_type) {
                    *l3_ete = tmp_ete;
                    return BCM_E_NONE;
                }
            } _FE2K_ALL_L3ETE_PER_IEH_BKT_END(l3_intf, i, tmp_ete);
        }
    }

    return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *     _bcm_fe2000_update_l2_ete
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
_bcm_fe2000_update_l2_ete(_fe2k_l3_fe_instance_t    *l3_fe,
                          _fe2k_ete_idx_t           *ete_idx,
                          uint32                     smac_idx,
                          uint32                     vid,
                          uint32                     mtu,
                          uint32                     change_flags)
{
    int status = BCM_E_NONE;
    
    switch (SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        status = _bcm_fe2000_g2p3_update_l2_ete(l3_fe, ete_idx, smac_idx,
                                                vid, mtu, change_flags);
        break;
#endif /* BCM_FE2000_P3_SUPPORT */
    default:
        L3_UKNOWN_UCODE_WARN(l3_fe->fe_unit);
        return BCM_E_CONFIG;
    }
    return status;
}

/*
 * Function:
 *     _bcm_fe2000_l3_get_sw_fte
 * Purpose:
 *     Given an fte_idx, return fte_hash elem
 *
 * Parameters:
 *     l3_fe           - (IN)     l3 fe instance
 *     fte_idx         - (IN)     Fte Index in HW
 *     hash_elem       - (OUT)    the FTE hash elem
 * Returns:
 *     BCM_E_XXX
 * Notes:
 *     Given an Fte index, use the global fteHash table
 *     to locate the element
 */
int
_bcm_fe2000_l3_get_sw_fte(_fe2k_l3_fe_instance_t      *l3_fe,
                          uint32                      fte_idx,
                          _fe2k_l3_fte_t            **sw_fte)
{
    uint32                           fte_hash_idx;
    dq_p_t                           l3_fte_head, l3_fte_elem;
    _fe2k_l3_fte_t                  *elem;
    int                              status;

    *sw_fte      = NULL;
    elem         = NULL;
    status       = BCM_E_NOT_FOUND;
    fte_hash_idx = _FE2K_GET_FTE_HASH_IDX(fte_idx);

    L3_VVERB((_SBX_D(l3_fe->fe_unit, "fte_idx 0x%x, hashIndex %d\n"), fte_idx, fte_hash_idx));

    l3_fte_head    = &l3_fe->fe_fteidx2_fte[fte_hash_idx];

    DQ_TRAVERSE(l3_fte_head, l3_fte_elem) {
        _FE2K_L3FTE_FROM_FTE_HASH_DQ(l3_fte_elem, elem);

        if (elem->fte_idx == fte_idx) {
            *sw_fte = elem;
            return BCM_E_NONE;
        }
    } DQ_TRAVERSE_END(l3_fte_head, l3_fte_elem);

    return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *     _bcm_fe2000_get_l3_ete_context_by_index
 * Purpose:
 *     Given an ete index, return the sw ete structure
 *
 * Parameters:
 *     l3_fe      - (IN)  l3 fe instance
 *     ete_idx     - (IN)     Ete Index in HW
 *     sw_ete      - (OUT)    the ete context in SW
 * Returns:
 *     BCM_E_XXX
 * Notes:
 *     Given an Ete index, use the global eteidx2ETE hash table
 *     to locate the SW ete context
 */
int
_bcm_fe2000_get_l3_ete_context_by_index(_fe2k_l3_fe_instance_t *l3_fe,
                                        uint32                  ete_idx,
                                        _fe2k_l3_ete_t        **sw_ete)
{
    uint32                           ete_hash_idx;
    dq_p_t                           l3_ete_head, l3_ete_elem;
    _fe2k_l3_ete_t                  *l3_ete = NULL;
    int                              status;

    *sw_ete      = NULL;
    status       = BCM_E_NOT_FOUND;
    ete_hash_idx = _FE2K_GET_ETE_HASH_IDX(ete_idx);

    L3_VVERB((_SBX_D(l3_fe->fe_unit, "Enter (Ete Addr 0x%x, hashIndex %d)\n"),
              ete_idx, ete_hash_idx));

    l3_ete_head    = &l3_fe->fe_eteidx2_l3_ete[ete_hash_idx];

    DQ_TRAVERSE(l3_ete_head, l3_ete_elem) {
        _FE2K_L3ETE_FROM_L3ETE_HASH_DQ(l3_ete_elem, l3_ete);

        if (l3_ete->l3_ete_hw_idx.ete_idx == ete_idx) {
            *sw_ete = l3_ete;
            return BCM_E_NONE;
        }
    } DQ_TRAVERSE_END(l3_ete_head, l3_ete_elem);

    return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *
 * Purpose:
 *     Given an ohi index, return the sw ete structure
 *
 * Parameters:
 *     l3_fe      - (IN)      l3 fe instance
 *     ohi        - (IN)      out header index
 *     sw_ete      - (OUT)    the ete context in SW
 * Returns:
 *     BCM_E_XXX
 * Notes:
 *     Given an ohi, use the global ohi2ETE hash table
 *     to locate the SW ete context. This will always
 *     return the the ETE that was created with the egress
 *     object. Even if the HW ohi pointing to a Tunnel, the
 *     SW copy maintains a pointer to the original ETE
 */
int
_bcm_fe2000_l3_sw_ete_find_by_ohi(_fe2k_l3_fe_instance_t *l3_fe,
                                  _fe2k_ohi_t            *ohi,
                                  _fe2k_l3_ete_t        **sw_ete)
{
    uint32           ohi_hash_idx;
    dq_p_t           l3_ohi_head, l3_ete_elem;
    _fe2k_l3_ete_t  *l3_ete = NULL;

    *sw_ete      = NULL;
    ohi_hash_idx = _FE2K_GET_OHI2ETE_HASH_IDX(ohi->ohi);

    L3_VVERB((_SBX_D(l3_fe->fe_unit, "(OHI 0x%x, hashIndex %d)\n"), ohi->ohi, ohi_hash_idx));

    l3_ohi_head    = &l3_fe->fe_ohi2_l3_ete[ohi_hash_idx];
    DQ_TRAVERSE(l3_ohi_head, l3_ete_elem) {
        _FE2K_L3ETE_FROM_L3OHI_HASH_DQ(l3_ete_elem, l3_ete);

        if (l3_ete->l3_ohi.ohi  == ohi->ohi) {
            *sw_ete = l3_ete;
            return BCM_E_NONE;
        }
    } DQ_TRAVERSE_END(l3_ohi_head, l3_ete_elem);

    return BCM_E_NOT_FOUND;
}


/*
 * Function:
 *     _bcm_fe2000_l3_sw_ete_find
 * Purpose:
 *     Given an ete_key, return the sw ete structure
 *
 * Parameters:
 *     unit        - (IN)     the fe unit number
 *     l3_fe       - (IN)     l3 fe instance
 *     l3_intf    -  (IN)     l3 interface
 *     ete_key     - (IN)     the ete key comprises of
 *                            <ete_type, dmac, vidop, ttlop>
 *     sw_ete      - (OUT)    the ete context in SW
 * Returns:
 *     BCM_E_XXX
 * Notes:
 *     Given the ete key and a l3_intf, find the hash bucket based on
 *     <ete_type + dmac> and then traverse all the ete contexts to find
 *     the the one that matches the key passed in
 */
int
_bcm_fe2000_l3_sw_ete_find(int                     unit,
                           _fe2k_l3_fe_instance_t *l3_fe,
                           _fe2k_l3_intf_t        *l3_intf,
                           _fe2k_l3_ete_key_t     *ete_key,
                           _fe2k_l3_ete_t        **sw_ete)
{
    uint32                            hash_idx;
    _fe2k_l3_ete_t                   *l3_sw_ete;

    *sw_ete   = NULL;
    l3_sw_ete = NULL;
    _FE2K_CALC_INTF_L3ETE_HASH(hash_idx,  ete_key->l3_ete_hk.type,
                               ete_key->l3_ete_hk.dmac);
    L3_VVERB((_SBX_D(l3_fe->fe_unit, "ete hash index %d\n"), hash_idx));

    if (DQ_EMPTY(&l3_intf->if_ete_hash[hash_idx])) {
        return BCM_E_NOT_FOUND;
    }

    _FE2K_ALL_L3ETE_PER_IEH_BKT(l3_intf, hash_idx, l3_sw_ete) {
        if (l3_sw_ete &&
            (ete_key->l3_ete_hk.type  == 
             l3_sw_ete->l3_ete_key.l3_ete_hk.type)                           &&
            (ete_key->l3_ete_vidop    == l3_sw_ete->l3_ete_key.l3_ete_vidop) &&
            (ete_key->l3_ete_ttl      == l3_sw_ete->l3_ete_key.l3_ete_ttl)   &&
            (ENET_CMP_MACADDR(ete_key->l3_ete_hk.dmac, 
                              l3_sw_ete->l3_ete_key.l3_ete_hk.dmac) == 0)) {
            L3_VVERB((_SBX_D(l3_fe->fe_unit, "Found matching ETE at 0x%x\n"),
                      l3_sw_ete->l3_ete_hw_idx.ete_idx));
            *sw_ete = l3_sw_ete;
            return BCM_E_NONE;
        }
    } _FE2K_ALL_L3ETE_PER_IEH_BKT_END(l3_intf, hash_idx, l3_sw_ete);

    return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *     _bcm_fe2000_l3_sw_find_v4encap_tnnl_ete_with_vidop
 * Purpose:
 *     Find the tunnel ete on the intf that matches the vidop
 *
 * Parameters:
 *     l3_fe       - (IN)     l3 fe instance
 *     l3_intf     - (IN)     l3 interface
 *     mac         - (IN)     tunnel end point mac
 *     vidop       - (IN)     vidop to match
 *     sw_ete      - (OUT)    the ete context in SW
 * Returns:
 *     BCM_E_XXX
 * Notes:
 */
int
_bcm_fe2000_l3_sw_find_v4encap_tnnl_ete_with_vidop( _fe2k_l3_fe_instance_t *l3_fe,
                                                    _fe2k_l3_intf_t        *l3_intf,
                                                    bcm_mac_t               mac,
                                                    _fe2k_l3_ete_vidop_t    vidop,
                                                    _fe2k_l3_ete_t        **sw_ete)
{
    uint32                            hash_idx;
    _fe2k_l3_ete_t                   *l3_sw_ete;

    *sw_ete   = NULL;
    l3_sw_ete = NULL;
    _FE2K_CALC_INTF_L3ETE_HASH(hash_idx,  _FE2K_L3_ETE__ENCAP_IP, mac);
    L3_VVERB((_SBX_D(l3_fe->fe_unit, "ete hash index 0x%x\n"), hash_idx));

    if (DQ_EMPTY(&l3_intf->if_ete_hash[hash_idx])) {
        return BCM_E_NOT_FOUND;
    }

    _FE2K_ALL_L3ETE_PER_IEH_BKT(l3_intf, hash_idx, l3_sw_ete) {
        if ((_FE2K_L3_ETE__ENCAP_IP == l3_sw_ete->l3_ete_key.l3_ete_hk.type) &&
            (vidop                  == l3_sw_ete->l3_ete_key.l3_ete_vidop) &&
            (ENET_CMP_MACADDR(mac, l3_sw_ete->l3_ete_key.l3_ete_hk.dmac) == 0))
        {
            *sw_ete = l3_sw_ete;
            L3_VVERB((_SBX_D(l3_fe->fe_unit, "Found matching Tunnel ete at 0x%x\n"),
                      l3_sw_ete->l3_ete_hw_idx.ete_idx));
            return BCM_E_NONE;
        }
    } _FE2K_ALL_L3ETE_PER_IEH_BKT_END(l3_intf, hash_idx, l3_sw_ete);

    return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *     _bcm_fe2000_l3_sw_find_v4encap_tnnl_ete_by_intf
 * Purpose:
 *     Find a tunnel ete on the intf
 *
 * Parameters:
 *     l3_fe       - (IN)     l3 fe instance
 *     l3_intf     - (IN)     l3 interface
 *     sw_ete      - (OUT)    the ete context in SW
 * Returns:
 *     BCM_E_XXX
 * Notes:
 *     For V4 Encap tunnels, we always create two etes.
 *     One ete with vidop = NOP and the other with vidop = ADD.
 *     So either one will suffice
 */
int
_bcm_fe2000_l3_sw_find_v4encap_tnnl_ete_by_intf(_fe2k_l3_fe_instance_t *l3_fe,
                                                _fe2k_l3_intf_t        *l3_intf,
                                                _fe2k_l3_ete_t        **sw_ete)
{
    _fe2k_l3_ete_vidop_t     vidop;



    vidop  = _FE2K_L3_ETE_VIDOP__NOP;
    return (_bcm_fe2000_l3_sw_find_v4encap_tnnl_ete_with_vidop(l3_fe,
                                                               l3_intf,
                                                               l3_intf->if_tunnel_info->dmac,
                                                               vidop,
                                                               sw_ete));
}

/*
 * Function:  _bcm_fe2000_l3_get_egrif_from_fte
 *
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
int _bcm_fe2000_l3_get_egrif_from_fte(_fe2k_l3_fe_instance_t *l3_fe,
                                  uint32                  fte_idx,
                                  uint32                  flags,
                                  bcm_l3_egress_t        *bcm_egr)
{
    int status = BCM_E_NONE;
    
    switch (SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        status = _bcm_fe2000_g2p3_l3_get_egrif_from_fte(l3_fe, fte_idx,
                                                        flags, bcm_egr);
        break;
#endif
    default:
        L3_UKNOWN_UCODE_WARN(l3_fe->fe_unit);
        status = BCM_E_CONFIG;
    }

    return status;
}


/*
 * Function:
 *     _bcm_fe2000_get_local_l3_egress_from_ohi
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
_bcm_fe2000_get_local_l3_egress_from_ohi(_fe2k_l3_fe_instance_t *l3_fe,
                                         bcm_l3_egress_t        *bcm_egr)
{
    int status = BCM_E_NONE;

    switch (SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        status = _bcm_fe2000_g2p3_get_local_l3_egress_from_ohi(l3_fe, bcm_egr);
        break;
#endif
    default:
        L3_UKNOWN_UCODE_WARN(l3_fe->fe_unit);
        status = BCM_E_CONFIG;
    }
    return status;
}

/*
 * Function:
 *     _bcm_fe2000_free_l3_ete
 * Purpose:
 *     free all resources that were allocated by alloc routine
 *
 * Parameters:
 *     l3_fe      - (IN)  l3 fe instance
 *     l3_sw_ete  - (IN)  SW context for l3 ete
 * Returns:
 *     None
 *
 * Asumption:
 *     This assumes that the SW context data struct is not in
 *     any link list.
 */
STATIC int
_bcm_fe2000_free_l3_ete(_fe2k_l3_fe_instance_t *l3_fe,
                        _fe2k_l3_ete_t         **sw_ete)
{
    _fe2k_l3_ete_t         *l3_sw_ete;
    uint32                  ete_resource_type;
    int                     status;

    L3_VVERB((_SBX_D(l3_fe->fe_unit, "Enter\n")));

    status    = BCM_E_NONE;
    l3_sw_ete = *sw_ete;

    if (l3_sw_ete->l3_ete_key.l3_ete_hk.type ==  _FE2K_L3_ETE__UCAST_IP) {
        if (l3_sw_ete->u.l3_fte) {
            sal_free(l3_sw_ete->u.l3_fte);
        }
    } else if (l3_sw_ete->l3_ete_key.l3_ete_hk.type == _FE2K_L3_ETE__MCAST_IP) 
    {
        if (l3_sw_ete->u.l3_ipmc) {
            sal_free(l3_sw_ete->u.l3_ipmc);
        }
    }

    if (_FE2K_L3_OHI_DYNAMIC_RANGE(l3_sw_ete->l3_ohi.ohi)) {
        /*
         * This means that the OHI was allocated by us and was not some
         * reserved by user
         */
        status = _sbx_gu2_resource_free(l3_fe->fe_unit,
                                        SBX_GU2K_USR_RES_OHI,
                                        1,
                                        &l3_sw_ete->l3_ohi.ohi,
                                        0);
        if (status != BCM_E_NONE) {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) could not free ohi %d\n"),
                    bcm_errmsg(status), l3_sw_ete->l3_ohi.ohi));
            return status;
        }
    }

    /*
     * If this L3 ete has its own private L2 Ete then free that resource
     */
    if (l3_sw_ete->l3_l2encap.ete_idx.ete_idx != _FE2K_INVALID_ETE_IDX) {
        status = _sbx_gu2_resource_free(l3_fe->fe_unit,
                                        SBX_GU2K_USR_RES_ETE_L2,
                                        1,
                                        &l3_sw_ete->l3_l2encap.ete_idx.ete_idx,
                                        0);
        if (status != BCM_E_NONE) {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) could not free L2 ETE %d\n"),
                    bcm_errmsg(status),
                    l3_sw_ete->l3_l2encap.ete_idx.ete_idx));
            return status;
        }
    }

    /*
     * Free the L3 ete resource
     */
    if (l3_sw_ete->l3_ete_hw_idx.ete_idx != _FE2K_INVALID_ETE_IDX) {
        _FE2K_L3_GET_ETE_RESOURCE_TYPE(l3_sw_ete->l3_ete_key.l3_ete_hk.type,
                                       ete_resource_type);
        if (ete_resource_type != SBX_GU2K_USR_RES_MAX) {
            status = _sbx_gu2_resource_free(l3_fe->fe_unit,
                                            ete_resource_type,
                                            1,
                                            &l3_sw_ete->l3_ete_hw_idx.ete_idx,
                                            0);

            if (status != BCM_E_NONE) {
                L3_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) could not free L3 ETE %d\n"),
                        bcm_errmsg(status),
                        l3_sw_ete->l3_ete_hw_idx.ete_idx));
                return status;
            }
        } else {
            return (BCM_E_INTERNAL);
        }
    }

    sal_free(*sw_ete);
    *sw_ete = NULL;
    return status;
}

/*
 * Function:
 *     _bcm_fe2000_undo_l3_ete_alloc
 * Purpose:
 *     Remove from all link lists and then
 *     free the HW indices and the SW instance
 * Parameters:
 *     l3_fe      - (IN)  l3 fe instance
 *     l3_intf    - (IN)  l3 interface context for the ete
 *     sw_ete     - (IN)  sw ete instance
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 *     XXX: There is currently no way to invalidate an ETE
 *     We just make sure that not FTEs point to it. Point the
 *     OHI to ete #0.
 */
int
_bcm_fe2000_undo_l3_ete_alloc(_fe2k_l3_fe_instance_t *l3_fe,
                              _fe2k_l3_intf_t        *l3_intf,
                              _fe2k_l3_ete_t         **p_l3_sw_ete)
{
    _fe2k_l3_ete_t    *l3_sw_ete;
    const char        *cptr;
    int                status;

    l3_sw_ete   = *p_l3_sw_ete;
    cptr = _fe2k_ete_type_string_get(l3_sw_ete->l3_ete_key.l3_ete_hk.type);
    L3_VVERB((_SBX_D(l3_fe->fe_unit, "Enter : Free ete (0x%x) type %s ohi (0x%x)\n"),
              l3_sw_ete->l3_ete_hw_idx.ete_idx,
              cptr, l3_sw_ete->l3_ohi.ohi));

    if (l3_sw_ete->l3_inuse_ue) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "Attempted to allocate an in-use ete=0x%x "
                       "type %s ohi=0x%x\n"),
                l3_sw_ete->l3_ete_hw_idx.ete_idx,
                cptr, l3_sw_ete->l3_ohi.ohi));
        return BCM_E_INTERNAL;
    }

    DQ_REMOVE(&l3_sw_ete->l3_ete_link);
    DQ_REMOVE(&l3_sw_ete->l3_ieh_link);

    if (l3_sw_ete->l3_ete_key.l3_ete_hk.type == _FE2K_L3_ETE__UCAST_IP) {
        /* keep track of v4 etes on a intf. In case of mpls tunnels on an
         * intf, we can only have one v4-ete
         */
        l3_intf->if_ip_ete_count--;
    }

    if (_FE2K_ETE_TYPE_NEEDS_OHI(l3_sw_ete->l3_ete_key.l3_ete_hk.type)) {

        DQ_REMOVE(&l3_sw_ete->l3_ohi_link);
        switch (SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
        case  SOC_SBX_UCODE_TYPE_G2P3:
        {
            soc_sbx_g2p3_oi2e_t  ohi2etc;

            soc_sbx_g2p3_oi2e_t_init(&ohi2etc);
            ohi2etc.eteptr = SOC_SBX_INVALID_L2ETE(l3_fe->fe_unit);
            status = soc_sbx_g2p3_oi2e_set(l3_fe->fe_unit,
                                           l3_sw_ete->l3_ohi.ohi,
                                           &ohi2etc);
            if (status != BCM_E_NONE) {
                L3_ERR((_SBX_D(l3_fe->fe_unit, "error %s in soc_sbx_g2p3_oi2e_set"),
                        bcm_errmsg(status)));
                return status;
            }
            break;
        }
#endif
        default:
            L3_UKNOWN_UCODE_WARN(l3_fe->fe_unit);
            status = BCM_E_CONFIG;
        }
        
    }

    status = _bcm_fe2000_free_l3_ete(l3_fe, p_l3_sw_ete);

    return status;
}


/*
 * Function:
 *     _bcm_fe2000_alloc_default_l3_ete
 * Purpose:
 *     allocate an l3 ete
 *
 * Parameters:
 *     l3_fe      - (IN)  l3 fe instance
 *     l3_intf    - (IN)  l3 interface
 *     module_fte - (IN)  <mod,fte> which points to this ete
 *     hw_ete     - (OUT) default values in HW ete struct
 *     ohi        - (IN)  ohi
 *     sw_ete     - (OUT) allocate and return
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 *     Allocate a SW instance and link it to
 *     a) l2encap ete
 *     b) eteIdx2ETE hash list
 *     Allocate HW ete_idx, and  a HW ohi (if needed). Finally fill in
 *     default values in the HW ete
 */
int
_bcm_fe2000_alloc_default_l3_ete(_fe2k_l3_fe_instance_t *l3_fe,
                                 _fe2k_l3_intf_t        *l3_intf,
                                 _fe2k_l3_ete_key_t     *l3_ete_key,
                                 uint32                  ohi,
                                 _fe2k_l3_ete_t        **sw_ete)

{
    uint32                           ete_hash_idx;
    uint32                           size, i;
    _sbx_gu2_usr_res_types_t         ete_resource_type;
    int                              status;
    _fe2k_l3_ete_t                  *l3_sw_ete;
    _fe2k_l3_ete_fte_t              *ete_fte;
    _fe2k_l3_ete_ipmc_t             *ete_ipmc;
    _fe2k_l3_ete_t                  *tmp_l3_ete;
    _fe2k_ohi_t                      tmp_ohi;

    *sw_ete     = NULL;
    ete_fte     = NULL;
    ete_ipmc    = NULL;
    status      = BCM_E_NONE;
    tmp_l3_ete  = NULL;

    /*
     * if user has specified an OHI, it should not be in use
     */
    if (ohi) {
        tmp_ohi.ohi = ohi;
        status = _bcm_fe2000_l3_sw_ete_find_by_ohi(l3_fe,
                                                   &tmp_ohi,
                                                   &tmp_l3_ete);
        if (status == BCM_E_NONE) {
            return BCM_E_EXISTS;
        }
    }

    size        = sizeof(_fe2k_l3_ete_t);
    l3_sw_ete   = (_fe2k_l3_ete_t *) sal_alloc(size, "l3-ete");
    if (l3_sw_ete == NULL) {
        status = BCM_E_MEMORY;
        return status;
    }

    sal_memset((l3_sw_ete), 0, size);

    ENET_COPY_MACADDR(l3_ete_key->l3_ete_hk.dmac,
                      l3_sw_ete->l3_ete_key.l3_ete_hk.dmac);

    l3_sw_ete->l3_ete_key.l3_ete_hk.type  = l3_ete_key->l3_ete_hk.type;
    l3_sw_ete->l3_ete_key.l3_ete_vidop    = l3_ete_key->l3_ete_vidop;
    l3_sw_ete->l3_ete_key.l3_ete_ttl      = l3_ete_key->l3_ete_ttl;
    l3_sw_ete->l3_ohi.ohi                 = _FE2K_INVALID_OHI;
    l3_sw_ete->l3_mpls_ohi.ohi            = _FE2K_INVALID_OHI;
    l3_sw_ete->l3_ete_hw_idx.ete_idx      = _FE2K_INVALID_ETE_IDX;
    l3_sw_ete->l3_l2encap.ete_idx.ete_idx = _FE2K_INVALID_ETE_IDX;

    if (l3_ete_key->l3_ete_hk.type == _FE2K_L3_ETE__UCAST_IP) {
        size     = (sizeof(_fe2k_l3_ete_fte_t)) * _FE2K_ETE_USER_SLAB_SIZE;
        ete_fte  = (_fe2k_l3_ete_fte_t *)
            sal_alloc(size, "Ip-ete-fte");
        if (ete_fte == NULL) {
            _bcm_fe2000_free_l3_ete(l3_fe, &l3_sw_ete);
            status = BCM_E_MEMORY;
            return status;
        }
        sal_memset(ete_fte, 0, size);

        l3_sw_ete->u.l3_fte   = ete_fte;
        /*
         * Initial ETE user size. This is a growable array
         */
        l3_sw_ete->l3_alloced_ue = _FE2K_ETE_USER_SLAB_SIZE;
        for (i = 0; i < l3_sw_ete->l3_alloced_ue; i++) {
            ete_fte[i].mod_id = SBX_INVALID_MODID;
        }
    } else if (l3_ete_key->l3_ete_hk.type == _FE2K_L3_ETE__MCAST_IP) {
        size     = (sizeof(_fe2k_l3_ete_ipmc_t)) * _FE2K_ETE_USER_SLAB_SIZE;
        ete_ipmc = (_fe2k_l3_ete_ipmc_t *) sal_alloc(size, "Ip-ete-ipmc");
        if (ete_ipmc == NULL) {
            _bcm_fe2000_free_l3_ete(l3_fe, &l3_sw_ete);
            status = BCM_E_MEMORY;
            return status;
        }
        sal_memset(ete_ipmc, -1, size);
        l3_sw_ete->u.l3_ipmc  = ete_ipmc;
        /*
         * Initial ETE user size. This is a growable array
         */
        l3_sw_ete->l3_alloced_ue = _FE2K_ETE_USER_SLAB_SIZE;
    }

    if ((ohi == 0) &&
        (_FE2K_ETE_TYPE_NEEDS_OHI(l3_ete_key->l3_ete_hk.type))) {
        status =  _sbx_gu2_resource_alloc(l3_fe->fe_unit,
                                          SBX_GU2K_USR_RES_OHI,
                                          1,
                                          &l3_sw_ete->l3_ohi.ohi,
                                          0);
        if (status != BCM_E_NONE) {
            /*
             * Make sure that resource allocator is in sync with us
             */
            L3_ERR((_SBX_D(l3_fe->fe_unit, "Could not allocate ohi: %s\n"),
                    bcm_errmsg(status)));
            _bcm_fe2000_free_l3_ete(l3_fe, &l3_sw_ete);
            return BCM_E_RESOURCE;
        }
    } else if (ohi) {
        l3_sw_ete->l3_ohi.ohi = ohi;
    }

    if (!_FE2K_L2_ETE_SHARED(l3_sw_ete)) {
        status = _sbx_gu2_resource_alloc(l3_fe->fe_unit,
                                         SBX_GU2K_USR_RES_ETE_L2,
                                         1,
                                         &l3_sw_ete->l3_l2encap.ete_idx.ete_idx,
                                         0);

        if (status != BCM_E_NONE) {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "Could not allocate L2 Encap Ete: %s\n"),
                    bcm_errmsg(status)));
            _bcm_fe2000_free_l3_ete(l3_fe, &l3_sw_ete);
            return status;
        }
        L3_VVERB((_SBX_D(l3_fe->fe_unit, "Allocated L2 Encap Ete Index (0x%x)\n"),
                  l3_sw_ete->l3_l2encap.ete_idx.ete_idx));
    }


    _FE2K_L3_GET_ETE_RESOURCE_TYPE(l3_ete_key->l3_ete_hk.type,
                                   ete_resource_type);
    if (ete_resource_type == SBX_GU2K_USR_RES_MAX) {
        _bcm_fe2000_free_l3_ete(l3_fe, &l3_sw_ete);
        L3_ERR((_SBX_D(l3_fe->fe_unit, "Unknown ete type %d\n"), l3_ete_key->l3_ete_hk.type));
        return BCM_E_PARAM;
    }

    status = _sbx_gu2_resource_alloc(l3_fe->fe_unit,
                                     ete_resource_type,
                                     1,
                                     &l3_sw_ete->l3_ete_hw_idx.ete_idx,
                                     0);
    if (status != BCM_E_NONE) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "Could not allocate L3 Ete: %s\n"), 
                bcm_errmsg(status)));
         _bcm_fe2000_free_l3_ete(l3_fe, &l3_sw_ete);
         return status;
    }

    _FE2K_CALC_INTF_L3ETE_HASH(ete_hash_idx, l3_ete_key->l3_ete_hk.type,
                               l3_ete_key->l3_ete_hk.dmac);

    L3_VVERB((_SBX_D(l3_fe->fe_unit, "Successfully allocated ete index (0x%x) intf_hash (%d)\n"),
              l3_sw_ete->l3_ete_hw_idx.ete_idx, ete_hash_idx));

    /*
     * Per interface ETEs
     */
    DQ_INSERT_HEAD(&l3_intf->if_ete_hash[ete_hash_idx], &l3_sw_ete->l3_ieh_link);
    if (l3_ete_key->l3_ete_hk.type == _FE2K_L3_ETE__UCAST_IP) {
        /* keep track of v4 etes on a intf. In case of mpls tunnels on an
         * intf, we can only have one v4-ete
         */
        l3_intf->if_ip_ete_count++;
    }

    /*
     * Global L3 Ete Index to ETE
     */
    ete_hash_idx = _FE2K_GET_ETE_HASH_IDX(l3_sw_ete->l3_ete_hw_idx.ete_idx);
    DQ_INSERT_HEAD(&l3_fe->fe_eteidx2_l3_ete[ete_hash_idx],
                   &l3_sw_ete->l3_ete_link);

    if (l3_sw_ete->l3_ohi.ohi != _FE2K_INVALID_OHI) {
        /*
         * Global OHI to ETE
         */
        ete_hash_idx = _FE2K_GET_OHI2ETE_HASH_IDX(l3_sw_ete->l3_ohi.ohi);
        DQ_INSERT_HEAD(&l3_fe->fe_ohi2_l3_ete[ete_hash_idx],
                   &l3_sw_ete->l3_ohi_link);
    }

    *sw_ete = l3_sw_ete;

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_fe2000_add_l3_ete
 * Purpose:
 *     Create the neccesary state in HW and SW for an ete
 *
 * Parameters:
 *     l3_fe      - (IN)  l3 fe instance
 *     ete_key    - (IN)  key to be added
 *     l3_intf    - (IN)  l3 interface
 *     ohi        - (IN)  ohi
 *     port       - (IN)  egress port
 *     flags      - (IN)  bcm_egr flags
 *     sw_ete     - (IN/OUT) updated sw ete
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 *     This function assumes that the ete does not exist
 *     either in HW or SW. In other words the caller has
 *     done a find before. This function returns after
 *     the following have been programmed in HW and the
 *     associated SW state has been updated.
 *     i)   l2 ete
 *     ii)  l3 ete
 *     iii) outheader2etc
 *     If there is a Tunnel on the interface, set the ohi to point
 *     to the corressponding tunnel ete
 */
int
_bcm_fe2000_add_ipv4_ete(_fe2k_l3_fe_instance_t *l3_fe,
                         _fe2k_l3_ete_key_t     *ete_key,
                         _fe2k_l3_intf_t        *l3_intf,
                         uint32                  ohi,
                         bcm_port_t              port,
                         uint32                  flags,
                         _fe2k_l3_ete_t        **sw_ete)
{
    int                                status;
    _fe2k_l3_ete_t                    *l3_ete, *l3_tnnl_ete;
    _fe2k_ete_idx_t                   *l2_ete_addr;

#ifdef BCM_FE2000_P3_SUPPORT
    soc_sbx_g2p3_eteencap_t            g2p3_hw_ete;
    soc_sbx_g2p3_oi2e_t                g2p3_hw_ohi2etc;
#endif

    *sw_ete    = NULL;

    L3_VVERB((_SBX_D(l3_fe->fe_unit, "Enter ohi(0x%x)\n"), ohi));

    status = _bcm_fe2000_alloc_default_l3_ete(l3_fe,
                                              l3_intf,
                                              ete_key,
                                              ohi,
                                              &l3_ete);

    if (BCM_FAILURE(status)) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) could not allocate l3 ete\n"),
                bcm_errmsg(status)));
        return status;
    }

    /*
     * Store the intf id, because given an ETE we can get back to
     * the l3 intf
     */
    l3_ete->l3_intf_id    = l3_intf->if_info.l3a_intf_id;

    if (!(_FE2K_L2_ETE_SHARED(l3_ete))) {

        L3_VVERB((_SBX_D(l3_fe->fe_unit, "Creating L2 ETE\n")));

        switch (SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
        {
            soc_sbx_g2p3_etel2_t l2_hw_ete;

            _bcm_fe2000_g2p3_map_l2_ete(l3_fe,
                                        l3_ete->l3_ete_key.l3_ete_vidop,
                                        l3_intf->if_egr_smac_idx,
                                        l3_intf->if_info.l3a_vid,
                                        l3_intf->if_info.l3a_mtu,
                                        &l2_hw_ete);
            
            status = _bcm_fe2000_g2p3_set_l2_ete(l3_fe,
                                            &l3_ete->l3_l2encap.ete_idx,
                                            &l2_hw_ete);
            break;
        }
#endif
        default:
            L3_UKNOWN_UCODE_WARN(l3_fe->fe_unit);
            status = BCM_E_CONFIG;
        } 

        if (BCM_FAILURE(status)) {
            /*
             * This will also free any L2 etes (if allocated)
             */
            _bcm_fe2000_undo_l3_ete_alloc(l3_fe,
                                          l3_intf,
                                          &l3_ete);
            return status;
        }
        l2_ete_addr = &l3_ete->l3_l2encap.ete_idx;
    } else {
        l2_ete_addr = &l3_intf->if_l2_ete[l3_ete->l3_ete_key.l3_ete_vidop];
    }

    switch (SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        status = _bcm_fe2000_g2p3_map_l3_ete(l3_fe, l3_ete, l2_ete_addr,
                                             port, flags, &g2p3_hw_ete,
                                             &g2p3_hw_ohi2etc);
        break;
#endif
    default:
        L3_UKNOWN_UCODE_WARN(l3_fe->fe_unit);
        status = BCM_E_CONFIG;
    }

    if (BCM_FAILURE(status)) {
        _bcm_fe2000_undo_l3_ete_alloc(l3_fe, l3_intf, &l3_ete);
        return status;
    }

    l3_tnnl_ete = NULL;

    /*
     * If there is a Encap Tunnel, point the OHI
     * to the corressponding tunnel ete (with correct VIDOP).
     *
     * GNATS 15353 (code review comments): If we cannot find
     * the Encap ETE, fail the L3 ETE add. This behavior is
     * debateable, but was agreed upon
     */
    if ((l3_intf->if_tunnel_info) &&
        (l3_ete->l3_ete_key.l3_ete_hk.type == _FE2K_L3_ETE__UCAST_IP)) {

        /*
         * If there is a tunnel on the interface find the tunnel ete
         * Mcast pkts are not allowed on tunnels
         */
        status = _bcm_fe2000_l3_sw_find_v4encap_tnnl_ete_with_vidop(l3_fe,
                                                  l3_intf,
                                                  l3_intf->if_tunnel_info->dmac,
                                                  l3_ete->l3_ete_key.l3_ete_vidop,
                                                  &l3_tnnl_ete);
        if (BCM_FAILURE(status)) {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "Could not find matching l3 encap "
                           "(tunnel) ETE. Failing L3 ete ADD: %s\n"),
                    bcm_errmsg(status)));
            _bcm_fe2000_undo_l3_ete_alloc(l3_fe, l3_intf, &l3_ete);
            return status;

        } else {
            switch (SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
            case SOC_SBX_UCODE_TYPE_G2P3:
                g2p3_hw_ohi2etc.eteptr = l3_tnnl_ete->l3_ete_hw_idx.ete_idx;
                break;
#endif
            default:
                L3_UKNOWN_UCODE_WARN(l3_fe->fe_unit);
                return BCM_E_CONFIG;
            }
        }
    }

    switch (SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        status = _bcm_fe2000_g2p3_set_l3_ete(l3_fe, l3_ete, &g2p3_hw_ete,
                                             &g2p3_hw_ohi2etc);
        break;
#endif
    default:
        L3_UKNOWN_UCODE_WARN(l3_fe->fe_unit);
        return BCM_E_CONFIG;
    }

    if (BCM_FAILURE(status)) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "Could not set L3 ETE %d"),
                l3_ete->l3_ete_hw_idx.ete_idx));

        _bcm_fe2000_undo_l3_ete_alloc(l3_fe, l3_intf, &l3_ete);
        return status;
    }
    *sw_ete = l3_ete;

    return status;
}

/*
 * Function:
 *     _bcm_fe2000_l3_sw_ete_create
 * Purpose:
 *     Find or create an l3 ete
 *
 * Parameters:
 *     unit         - (IN)  FE unit number
 *     l3_fe        - (IN)  l3 fe instance
 *     l3_intf      - (IN)  l3 interface context
 *     etekey       - (IN)  l3 ete key to be created
 *     ohi          - (IN)  ohi
 *     flags        - (IN)  bcm_egr flags
 *     port         - (IN)  bcm_egr port
 *     l3_ete       - (OUT) l3 ete (either found or added)
 * Returns:
 *     BCM_E_EXIST - found an existing l2encap ete
 *     BCM_E_NONE  - successfully created a new l2encap ete
 *
 * NOTE:
 *     This function is executed on the linecard where the ete
 *     lives. Therefore this function should be callable via an
 *     RPC. It also create an l2encap ete if one is required.
 */
int
_bcm_fe2000_l3_sw_ete_create(int                     unit,
                             _fe2k_l3_fe_instance_t *l3_fe,
                             _fe2k_l3_intf_t        *l3_intf,
                             _fe2k_l3_ete_key_t     *etekey,
                             uint32                  ohi,
                             uint32                  flags,
                             bcm_port_t              port,
                             _fe2k_l3_ete_t        **l3_ete)
{
    int                            status, ignore_status = BCM_E_NONE;
    int                            max_permissible_v4_etes;

    L3_VVERB((_SBX_D(l3_fe->fe_unit, "Enter\n")));

    status = _bcm_fe2000_l3_sw_ete_find(unit,
                                        l3_fe,
                                        l3_intf,
                                        etekey,
                                        l3_ete);

    if (status == BCM_E_NONE) {
        return BCM_E_EXISTS;
    }

    /*
     * We are about to add a new ucast v4 ete. However, if
     * there is a mpls tunnel on the intf we cannot have more
     * than one ucast  ete on the intf. This is because we
     * get the dmac from the v4 ete and the mpls tunnel will
     * point to that dmac. Since in the bcm world all etes that
     * are present on the intf need to go over the tunnel, therefore
     * by implication we can only have one v4 ete.
     *
     * There is one transient case however, when there will be two etes.
     * This is when the ete is being replaced. For example if the MAC addr
     * changes, we create a new v4-ete and then destroy the old one.
     * Therefore during this transition we will have two etes
     */

    if (l3_intf->if_flags &  _FE2K_L3_MPLS_TUNNEL_SET) {
        max_permissible_v4_etes = (flags & BCM_L3_REPLACE) ? 1 : 0;
        if (l3_intf->if_ip_ete_count > max_permissible_v4_etes) {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "mpls tunnel set on interface; "
                           " Cannot have more than one egress "
                           " object on the interface")));
            return BCM_E_PARAM;
        }
    }

    status = _bcm_fe2000_add_ipv4_ete(l3_fe,
                                      etekey,
                                      l3_intf,
                                      ohi,
                                      port,
                                      flags,
                                      l3_ete);
    if (status == BCM_E_NONE) {
        if (l3_intf->if_flags & _FE2K_L3_MPLS_TUNNEL_SET) {
            /*
             * we successfully created the first ucast v4 ete.
             * (Note that we checked this above in case of mpls tunnels)
             * Since the flag is set, there must be a mpls ete. Although
             * this has a dummy mac address
             */
            switch (SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
            case SOC_SBX_UCODE_TYPE_G2P3:
                ignore_status = _bcm_fe2000_g2p3_enable_mpls_tunnel(l3_fe,
                                                                    l3_intf,
                                                                    *l3_ete);
                break;
#endif
            default:
                L3_UKNOWN_UCODE_WARN(l3_fe->fe_unit);
                return BCM_E_CONFIG;
            }
        
            if (ignore_status != BCM_E_NONE) {
                L3_ERR((_SBX_D(l3_fe->fe_unit, "Could not enable mpls tunnel on interface 0x%x\n"),
                        _FE2K_USER_HANDLE_FROM_IFID((*l3_ete)->l3_intf_id)));
            }
        }
    }
    return status;
}

/*
 * Function:
 *     _bcm_fe2000_create_local_ucast_l3_ete
 * Purpose:
 *     Find or create an l3 ete
 *
 * Parameters:
 *     l3_fe      - (IN)  l3 fe instance
 *     bcm_egr    - (IN/OUT)  bcm egress data
 *                        encap_id to contain encoded ohi
 *     module_fte - (IN)  <module,ftidx> module is where the
 *                        fte lives
 * Returns:
 *     BCM_E_EXIST - found an existing l2encap ete
 *     BCM_E_NONE  - successfully created a new l2encap ete
 *
 * NOTE:
 *     This function is executed on the linecard where the ete
 *     lives. Therefore this function should be callable via an
 *     RPC. It also create an l2encap ete if one is required.
 */
int
_bcm_fe2000_create_local_ucast_l3_ete(_fe2k_l3_fe_instance_t *l3_fe,
                                      bcm_l3_egress_t        *bcm_egr,
                                      _fe2k_l3_ete_fte_t     *module_fte)
{
    int                                status;
    _fe2k_l3_ete_t                    *l3_sw_ete;
    _fe2k_l3_intf_t                   *l3_intf;
    _fe2k_l3_ete_key_t                 tkey;

    L3_VVERB((_SBX_D(l3_fe->fe_unit, "Enter\n")));

    l3_sw_ete = NULL;

    status = _bcm_fe2000_l3_find_intf_by_ifid(l3_fe,
                                              bcm_egr->intf,
                                              &l3_intf);
    if (status != BCM_E_NONE) {
        L3_WARN((_SBX_D(l3_fe->fe_unit, "Could not find interfaceId %d on unit %d"),
                 bcm_egr->intf, l3_fe->fe_unit));
        return status;
    }

    _FE2K_MAKE_UCAST_IP_SW_ETE_KEY(&tkey, bcm_egr, l3_intf);

    /*
     * Find an existing ETE and if not found add a new one
     */
    status = _bcm_fe2000_l3_sw_ete_create(l3_fe->fe_unit,
                                          l3_fe,
                                          l3_intf,
                                          &tkey,
                                          SOC_SBX_OHI_FROM_ENCAP_ID(bcm_egr->encap_id),
                                          bcm_egr->flags,
                                          bcm_egr->port,
                                          &l3_sw_ete);
    if ((status == BCM_E_NONE) || (status == BCM_E_EXISTS)) {
        /*
         * add the FTE as one of the users of this ETEG
         */
        bcm_egr->encap_id = SOC_SBX_ENCAP_ID_FROM_OHI(l3_sw_ete->l3_ohi.ohi);

        status = _bcm_fe2000_link_fte2ete(l3_sw_ete, module_fte);
        if ((bcm_egr->flags & BCM_L3_REPLACE) && (status == BCM_E_EXISTS)) {
            status = BCM_E_NONE;
        }
    }

    return status;
}

/*
 * Function:
 *     _bcm_fe2000_l3_sw_ete_destroy
 * Purpose:
 *     Destroy the L3 ete if the last user is going away
 *
 * Parameters:
 *     unit       - (IN)  FE unit #
 *     l3_fe      - (IN)  l3 fe instance
 *     l3_intf    - (IN)  l3 interface
 *     sw_ete     - (IN)  L3 ete to be destroyed
 *
 * Returns:
 *     BCM_E_XXX -
 * NOTE:
 *     Must be called on the Linecard where ete lives
 */
int
_bcm_fe2000_l3_sw_ete_destroy(int                     unit,
                              _fe2k_l3_fe_instance_t *l3_fe,
                              _fe2k_l3_intf_t        *l3_intf,
                              _fe2k_l3_ete_t        **sw_ete)
{
    if ((*sw_ete)->l3_inuse_ue == 0) {
        _bcm_fe2000_undo_l3_ete_alloc(l3_fe,
                                      l3_intf,
                                      sw_ete);
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_fe2000_destroy_local_l3_ete
 * Purpose:
 *     Destroy an ipv4 ete if the last fte is going away
 *
 * Parameters:
 *     l3_fe      - (IN)  l3 fe instance
 *     module_fte - (IN)  <module,ftidx> module is where the
 *                        fte lives
 * Returns:
 *     BCM_E_XXX -
 * NOTE:
 *     This function is executed on the linecard where the ete
 *     lives. Therefore this function should be callable via an
 *     RPC. If the FTE handle is the last one that is using the
 *     ete, then the ete is destroyed.
 */
int
_bcm_fe2000_destroy_local_l3_ete(_fe2k_l3_fe_instance_t *l3_fe,
                                 uint32                  ohi,
                                 _fe2k_l3_ete_fte_t     *module_fte)
{
    int                        status;
    _fe2k_l3_ete_t            *l3_sw_ete;
    _fe2k_ohi_t                tmp_ohi;
    _fe2k_l3_intf_t           *l3_intf;

    L3_VVERB((_SBX_D(l3_fe->fe_unit, "Enter (Ohi 0x%x, module %d fte 0x%x\n"),
              ohi, module_fte->mod_id,
              module_fte->fte_idx.fte_idx));

    /*
     * This will always give the underlying real ete,
     * even if the ohi in HW points to a tunnel
     */
    tmp_ohi.ohi = ohi;
    status = _bcm_fe2000_l3_sw_ete_find_by_ohi(l3_fe, &tmp_ohi, &l3_sw_ete);

    if (BCM_FAILURE(status)) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "Could not get SW context for ohi (%d)\n"), ohi));
        return status;
    }

    status = _bcm_fe2000_unlink_fte2ete(l3_sw_ete, module_fte);

    if (BCM_FAILURE(status)) {
        return status;
    }

    /* XXX: See if we can move it in */
    if (l3_sw_ete->l3_inuse_ue == 0) {
        status = _bcm_fe2000_l3_find_intf_by_ifid(l3_fe,
                                                  l3_sw_ete->l3_intf_id,
                                                  &l3_intf);
        if (BCM_SUCCESS(status)) {
            status = _bcm_fe2000_undo_l3_ete_alloc(l3_fe,
                                                   l3_intf,
                                                   &l3_sw_ete);
        } else {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "Could not free ete because interface 0x%08X"
                           "not found\n"),
                    _FE2K_IFID_FROM_USER_HANDLE(l3_sw_ete->l3_intf_id)));
        }
    }

    return status;
}

/*
 * Function:
 *     _bcm_fe2000_find_fte_by_modid
 * Purpose:
 *     Given an ete and modid find the corresponding fte
 * Parameters:
 *     l3_fe      - (IN)     fe instance corresponsing to unit
 *     fte_module - (IN)     module id for the fte
 *     fte_idx    - (OUT)    fte idx
 * Returns:
 *     BCM_E_XXX
 *
 * Assumption:
 *     Only one FTE from a module can point to an ETE
 */
int
_bcm_fe2000_find_fte_by_modid(_fe2k_l3_fe_instance_t *l3_fe,
                              _fe2k_l3_ete_t *l3_ete,
                              bcm_module_t    fte_module,
                              uint32         *fte_idx)
{
    int                    status;
    int                    i;

    status        = BCM_E_NOT_FOUND;

    if (l3_ete->l3_inuse_ue < 0) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "Invalid in-use count (%d) found on ete 0x%x\n"),
                l3_ete->l3_inuse_ue,
                l3_ete->l3_ete_hw_idx.ete_idx));
        return BCM_E_INTERNAL;
    }

    if ((l3_ete->u.l3_fte == NULL) ||
        (l3_ete->l3_inuse_ue == 0)) {
        return status;
    }

    for (i = 0; i < l3_ete->l3_alloced_ue; i++) {
        if (l3_ete->u.l3_fte[i].mod_id == fte_module) {
            *fte_idx = l3_ete->u.l3_fte[i].fte_idx.fte_idx;
            status = BCM_E_NONE;
            break;
        }
    }

    return status;
}

/*
 * Function:
 *     _bcm_fe2000_local_find_l3_ete_fte
 * Purpose:
 *     Given an fte index get bcm egress info
 * Parameters:
 *     l3_fe      - (IN)     fe instance corresponsing to unit
 *     bcm_egr    - (IN)     Egress object properties to match.
 *                           l3a_intf_id, module, dmac and port
 *     fte_module - (IN)     module of the FTE
 *     ul_fte     - (OUT)    fte Index
 *
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 *     1. This function is called on the unit where ete is local
 *     2. This function should be callable from an RPC wrapper
 */
int
_bcm_fe2000_local_find_l3_ete_fte(_fe2k_l3_fe_instance_t *l3_fe,
                                  bcm_l3_egress_t        *bcm_egr,
                                  bcm_module_t            fte_module,
                                  uint32                 *fte_idx)
{
    int                       status;
    _fe2k_l3_ete_t           *l3_ete;
    _fe2k_l3_intf_t          *l3_intf;
    _fe2k_l3_ete_key_t        tkey;

    status = _bcm_fe2000_l3_find_intf_by_ifid(l3_fe,
                                              bcm_egr->intf,
                                              &l3_intf);
    if (status != BCM_E_NONE) {
        L3_WARN((_SBX_D(l3_fe->fe_unit, "Could not find interfaceId %d on unit %d"),
                 bcm_egr->intf, l3_fe->fe_unit));
        return status;
    }

    _FE2K_MAKE_UCAST_IP_SW_ETE_KEY(&tkey, bcm_egr, l3_intf);

    status = _bcm_fe2000_l3_sw_ete_find(l3_fe->fe_unit,
                                        l3_fe,
                                        l3_intf,
                                        &tkey,
                                        &l3_ete);

    if (status != BCM_E_NONE) {
        L3_WARN((_SBX_D(l3_fe->fe_unit, "Could not find l3 ete\n")));
        return status;
    }

    status = _bcm_fe2000_find_fte_by_modid(l3_fe, l3_ete, fte_module, fte_idx);

    return status;
}

/*
 * Function:
 *     _bcm_fe2000_find_fte
 * Purpose:
 *     Given an fte index get bcm egress info
 * Parameters:
 *     l3_fe      - (IN)     fe instance corresponsing to unit
 *     bcm_egr    - (IN)     Egress object properties to match.
 *                           l3a_intf_id, module, dmac and port
 *     ul_fte     - (OUT)    fte Index
 *
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 */
int
_bcm_fe2000_find_fte(_fe2k_l3_fe_instance_t *l3_fe,
                     bcm_l3_egress_t        *bcm_egr,
                     uint32                 *fte_idx)
{
    int                    status = 0;

    if (bcm_egr->module == l3_fe->fe_my_modid) {
        status = _bcm_fe2000_local_find_l3_ete_fte(l3_fe,
                                                   bcm_egr,
                                                   l3_fe->fe_my_modid,
                                                   fte_idx);
    }

    return status;
}

/*
 * Function:
 *     _bcm_fe2000_alloc_l3_or_mpls_fte
 * Purpose:
 *     Allocate an FTE index on the given unit and
 *     fill in FTE default fields
 *
 * Parameters:
 *     l3_fe      - (IN)     l3 fe instance
 *     flags      - (IN)     BCM_L3_WITH_ID
 *     trunk      - (IN)     trunk id
 *     res_type   - (IN)     type of resource
 *     hw_fte     - (OUT)    default values in HW fte struct
 *     fte_idx    - (IN/OUT) IN : if flag == BCM_L3_WITH_ID
 *
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 */

int
_bcm_fe2000_alloc_l3_or_mpls_fte(_fe2k_l3_fe_instance_t   *l3_fe,
                                 int                       flags,
                                 bcm_trunk_t               trunk,
                                 _sbx_gu2_usr_res_types_t  res_type,
                                 uint32                   *fte_idx)
{
    int                         status;
    _fe2k_l3_fte_t             *fte_hash_elem, *tmp_elem;
    uint32                      hash_idx, res_idx;

    status        = BCM_E_NONE;

    if (flags & ~(BCM_L3_WITH_ID | BCM_L3_TGID)) {
        return BCM_E_INTERNAL;
    }

    /* convert the vsi to an fti, for reserve case */
    if (res_type == SBX_GU2K_USR_RES_VSI) {
        *fte_idx -= l3_fe->vlan_ft_base;
    }

    fte_hash_elem = sal_alloc(sizeof(_fe2k_l3_fte_t), "FTE-Hash");
    if (fte_hash_elem == NULL) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "Could not allocate fte hash elem\n")));
        return BCM_E_MEMORY;
    }
    sal_memset(fte_hash_elem, 0, sizeof(_fe2k_l3_fte_t));


    res_idx = *fte_idx;
    status = _sbx_gu2_resource_alloc(l3_fe->fe_unit, res_type, 1, &res_idx,
                                         (flags & BCM_L3_WITH_ID)?_SBX_GU2_RES_FLAGS_RESERVE:0);

    if (status == BCM_E_RESOURCE) {
        L3_VERB((_SBX_D(l3_fe->fe_unit, "fte 0x%08x marked as externally managed\n"),
                 res_idx));
        status = BCM_E_NONE;

    } else if (status != BCM_E_NONE) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) could not allocate fte index\n"),
                bcm_errmsg(status)));
        sal_free(fte_hash_elem);
        return status;
    }

    /* convert the vsi to an fti */
    if (res_type == SBX_GU2K_USR_RES_VSI) {
        *fte_idx = res_idx + l3_fe->vlan_ft_base;
    } else {
        *fte_idx = res_idx;
    }

    /*
     * As a check, make sure that this fte_idx already does not exist
     */
    status = _bcm_fe2000_l3_get_sw_fte(l3_fe, *fte_idx, &tmp_elem);
    if (status == BCM_E_NONE) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "FTE idx 0x%x already exists in hash\n"),
                tmp_elem->fte_idx));

        sal_free(fte_hash_elem);
        return BCM_E_EXISTS;
    } else if (status == BCM_E_NOT_FOUND) {
        /* Entry does not exsist  - ok */
        status = BCM_E_NONE;
    }
      

    /* All checks passed; allocate and insert the fte hash element
     */

    fte_hash_elem->ref_count = 1;
    fte_hash_elem->fte_idx   = *fte_idx;
    hash_idx = _FE2K_GET_FTE_HASH_IDX(fte_hash_elem->fte_idx);
    DQ_INSERT_HEAD(&l3_fe->fe_fteidx2_fte[hash_idx],
                   &fte_hash_elem->fte_hash_link);

    L3_VERB((_SBX_D(l3_fe->fe_unit, "allocated fte index(0x%x) res=%s (%d)\n"),
             fte_hash_elem->fte_idx, _sbx_gu2_resource_to_str(res_type), 
             res_type));

    return status;
}


/*
 * Function:
 *     _bcm_fe2000_expand_ete_users
 * Purpose:
 *     Create more space to store info about ete users
 * Parameters:
 *     l3_ete     - (IN)     the SW ete struct
 * Returns:
 *     BCM_E_XXX
 * NOTE:
 */
int
_bcm_fe2000_expand_ete_users(_fe2k_l3_ete_t         *l3_ete)
{

    uint32                num_new_entries;
    uint32                size, i;
    _fe2k_l3_ete_fte_t   *fte_info;
    _fe2k_l3_ete_ipmc_t  *ipmc_info;

    num_new_entries   = l3_ete->l3_alloced_ue + _FE2K_ETE_USER_SLAB_SIZE;

    if (l3_ete->l3_ete_key.l3_ete_hk.type == _FE2K_L3_ETE__UCAST_IP) {
        size       = (sizeof(_fe2k_l3_ete_fte_t)) * num_new_entries;
        fte_info   = (_fe2k_l3_ete_fte_t *) sal_alloc(size, "Ip-ete-fte");
        if (!fte_info) {
            return BCM_E_MEMORY;
        }

        for (i = 0; i < num_new_entries; i++) {
            if (i < l3_ete->l3_alloced_ue) {
                fte_info[i] = l3_ete->u.l3_fte[i];
            } else {
                fte_info[i].mod_id = SBX_INVALID_MODID;
            }
        }
        sal_free(l3_ete->u.l3_fte);
        l3_ete->u.l3_fte = fte_info;
    } else if (l3_ete->l3_ete_key.l3_ete_hk.type == _FE2K_L3_ETE__MCAST_IP) {
        size      = (sizeof(_fe2k_l3_ete_ipmc_t)) * num_new_entries;
        ipmc_info = (_fe2k_l3_ete_ipmc_t *) sal_alloc(size, "Ip-ete-ipmc");
        if (!ipmc_info) {
            return BCM_E_MEMORY;
        }
        sal_memset(ipmc_info, 0, size);
        for (i = 0; i < l3_ete->l3_alloced_ue; i++) {
            ipmc_info[i] = l3_ete->u.l3_ipmc[i];
        }
        sal_free(l3_ete->u.l3_ipmc);
        l3_ete->u.l3_ipmc = ipmc_info;
    }
    l3_ete->l3_alloced_ue = num_new_entries;

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_fe2000_unlink_fte2ete
 * Purpose:
 *     Given an fte and the module to which it belongs, update
 *     the ete.
 * Parameters:
 *     l3_ete     - (IN/OUT) l3 ete to which the fte must should point
 *     module     - (IN)     the globally unique module id on which
 *                           the FTE is actually present
 *     fte_idx    - (IN)     FTE index of the FTE on FE2K represented by
 *                           module
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 *     The MODID/FTE pairs are allocated in chunks of
 *     FE2K_ETE_FTE_USER_SIZE to avoid a re-alloc every time.
 */
STATIC int
_bcm_fe2000_unlink_fte2ete(_fe2k_l3_ete_t         *l3_ete,
                           _fe2k_l3_ete_fte_t     *module_fte)
{
    uint32                i;

    if ( (module_fte->mod_id ==0) &&
         (module_fte->fte_idx.fte_idx == 0)) {
        return BCM_E_PARAM;
    }

    for (i = 0; i < l3_ete->l3_alloced_ue; i++) {
        if ((l3_ete->u.l3_fte[i].mod_id == module_fte->mod_id) &&
            (l3_ete->u.l3_fte[i].fte_idx.fte_idx ==
             module_fte->fte_idx.fte_idx)) {
            l3_ete->u.l3_fte[i].mod_id          = SBX_INVALID_MODID;
            l3_ete->u.l3_fte[i].fte_idx.fte_idx = 0;
            l3_ete->l3_inuse_ue--;
            break;
        }
    }
    if (i >= l3_ete->l3_alloced_ue) {
        return BCM_E_NOT_FOUND;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_fe2000_link_fte2ete
 * Purpose:
 *     Given an fte and the module to which it belongs, update
 *     the ete.
 * Parameters:
 *     l3_ete     - (IN/OUT) l3 ete to which the fte must should point
 *     module     - (IN)     the globally unique module id on which
 *                           the FTE is actually present
 *     fte_idx    - (IN)     FTE index of the FTE on FE2K represented by
 *                           module
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 *     The MODID/FTE pairs are allocated in chunks of
 *     _FE2K_ETE_USER_SLAB_SIZE to avoid a re-alloc every time.
 */
STATIC int
_bcm_fe2000_link_fte2ete(_fe2k_l3_ete_t         *l3_ete,
                         _fe2k_l3_ete_fte_t     *module_fte)
{
    _fe2k_l3_ete_fte_t   *fte_info;
    uint32                i;
    int                   status;

    status = BCM_E_NONE;

    /*
     * first make sure that the FTE/MOD pair does not already exist
     */
    for (i = 0; i < l3_ete->l3_alloced_ue; i++) {
        if ((l3_ete->u.l3_fte[i].mod_id == module_fte->mod_id) &&
            (l3_ete->u.l3_fte[i].fte_idx.fte_idx ==
             module_fte->fte_idx.fte_idx)) {
            return BCM_E_EXISTS;
        }
    }

    /*
     * New entry, need to add. Make sure there is space
     */
    if ((l3_ete->l3_inuse_ue > 0) &&
        (l3_ete->l3_inuse_ue == l3_ete->l3_alloced_ue)) {
        status = _bcm_fe2000_expand_ete_users(l3_ete);
    }

    BCM_IF_ERROR_RETURN(status);

    for (i = 0; i < l3_ete->l3_alloced_ue; i++) {
        fte_info = &l3_ete->u.l3_fte[i];
        if ((fte_info->mod_id == SBX_INVALID_MODID) &&
            (fte_info->fte_idx.fte_idx == 0)) {
            *fte_info  = *module_fte;
            l3_ete->l3_inuse_ue += 1;
            break;
        }
    }
    return status;
}


int
_bcm_fe2000_create_ucast_l3_ete(_fe2k_l3_fe_instance_t *l3_fe,
                                uint32                  fte_idx,
                                bcm_l3_egress_t        *bcm_egr)
{
    _fe2k_l3_ete_fte_t    ete_fte;
    int                   status;

    status = BCM_E_NONE;
    ete_fte.fte_idx.fte_idx = fte_idx;

    if ((l3_fe->fe_my_modid == bcm_egr->module) || 
        (bcm_egr->flags & BCM_L3_TGID)) {
        ete_fte.mod_id = l3_fe->fe_my_modid;

        status = _bcm_fe2000_create_local_ucast_l3_ete(l3_fe, bcm_egr, 
                                                       &ete_fte);
    } else {
        /* for Remote Egress objects, ENCAP ID is passed with Egress Interface
         * input, No RPC is issued */
        ete_fte.mod_id = l3_fe->fe_my_modid;
    }

    return status;
}

/*
 * Function:
 *     _bcm_fe2000_map_l3_ucast_fte
 * Purpose:
 *     Translate BCM egress values to fields in FTE
 * Parameters:
 *     l3_fe      - (IN)     l3 fe instance
 *     bcm_egr    - (IN)     egress object passed by user
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 *     This function is called after the ETE has been created
 *     (either local or remote) OR the user has specified an
 *     encoded OHI in the encap_id field of the egress structure
 */
int
_bcm_fe2000_map_l3_ucast_fte(_fe2k_l3_fe_instance_t *l3_fe,
                             _fe2k_fte_idx_t        *fte_idx,
                             bcm_l3_egress_t        *bcm_egr)
{
    int status = BCM_E_NONE;

    switch (SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        status = _bcm_fe2000_g2p3_map_set_l3_ucast_fte(l3_fe, fte_idx, 
                                                       bcm_egr);
        break;
#endif
    default: 
        L3_UKNOWN_UCODE_WARN(l3_fe->fe_unit);
        status = BCM_E_CONFIG;
    }

    if (status != BCM_E_NONE) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "error mapping l3 ucast fte 0x%x\n"),
                fte_idx->fte_idx));
    } else {
        L3_VERB((_SBX_D(l3_fe->fe_unit, "successfully programmed fte(0x%x) with encap-id(0x%x)\n"),
                 fte_idx->fte_idx, bcm_egr->encap_id));
    }

    return status;
}


STATIC int
_bcm_fe2000_destroy_l3_ete(_fe2k_l3_fe_instance_t   *l3_fe,
                           bcm_module_t              egr_modid,
                           uint32                    ohi,
                           uint32                    fte_idx)
{
    int                   status;
    _fe2k_l3_ete_fte_t    ete_fte;

    status = BCM_E_NONE;
    if (l3_fe->fe_my_modid == egr_modid) {

        ete_fte.mod_id          = egr_modid;
        ete_fte.fte_idx.fte_idx = fte_idx;

        status = _bcm_fe2000_destroy_local_l3_ete(l3_fe, ohi, &ete_fte);
    } else {
        ete_fte.mod_id = l3_fe->fe_my_modid;
    }

    return status;
}

/*
 * Function:
 *     _bcm_fe2000_add_l3_fte
 * Purpose:
 *     add an ipv4 fte
 * Parameters:
 *     l3_fe      - (IN)     l3 fe instance
 *     flags      - (IN)     BCM_L3_WITH_ID
 *     bcm_egr    - (IN)     egress object passed by user
 *     fte_idx    - (OUT)    handle that needs to be returned
 *                           to user
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 *
 */
STATIC int
_bcm_fe2000_add_l3_fte(_fe2k_l3_fe_instance_t *l3_fe,
                       uint32                  flags,
                       bcm_l3_egress_t        *bcm_egr,
                       _fe2k_fte_idx_t        *fte_idx)
{
    int      status, ignore_status;
    uint32   flags_tmp;

    flags_tmp = (flags & BCM_L3_WITH_ID) | (bcm_egr->flags & BCM_L3_TGID);
    status = _bcm_fe2000_alloc_l3_or_mpls_fte(l3_fe,
                                              flags_tmp,
                                              bcm_egr->trunk,
                                              SBX_GU2K_USR_RES_FTE_L3,
                                              &fte_idx->fte_idx);
    if (status != BCM_E_NONE) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) could not allocate fte\n"),
                bcm_errmsg(status)));
        return status;
    }

    /*
     * find or create an ete. Then link this FTE to the
     * ETE
     */
    status = _bcm_fe2000_create_ucast_l3_ete(l3_fe, fte_idx->fte_idx, bcm_egr);

    if (status != BCM_E_NONE) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) could not create ETE for fte 0x%x\n"),
                bcm_errmsg(status), fte_idx->fte_idx));
        ignore_status = _bcm_fe2000_destroy_fte(l3_fe,
                                                L3_OR_MPLS_DESTROY_FTE__FTE_ONLY,
                                                fte_idx->fte_idx,
                                                0, /* mod */
                                                0  /* ohi */);
        return status;
    }

    status = _bcm_fe2000_map_l3_ucast_fte(l3_fe,
                                          fte_idx,
                                          bcm_egr);

    if (status != BCM_E_NONE) {
        _bcm_fe2000_destroy_fte(l3_fe,
                                L3_OR_MPLS_DESTROY_FTE__FTE_OHI_ETE_GIVEN_OHI,
                                fte_idx->fte_idx,
                                bcm_egr->module,
                                SOC_SBX_OHI_FROM_ENCAP_ID(bcm_egr->encap_id));
    }

    return status;
}


/*
 * Function:
 *     _bcm_fe2000_update_l3_fte
 * Purpose:
 *     add an l3 fte
 * Parameters:
 *     l3_fe      - (IN)     l3 fe instance
 *     flags      - (IN)     BCM_L3_WITH_ID
 *     bcm_egr    - (IN)     egress object passed by user
 *     intf       - (IN/OUT) fte
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 *
 */
int
_bcm_fe2000_update_l3_fte(_fe2k_l3_fe_instance_t *l3_fe,
                            uint32                  flags,
                            bcm_l3_egress_t        *bcm_egr,
                            _fe2k_fte_idx_t        *fte_data)
{
    bcm_l3_egress_t        old_egr, tmp_egr;
    int                    ignore_status, status;
    int                    new_ete_created;
    uint32                 chg_flags;
    uint32                 ohi;
    uint32                 old_fte_idx;
    uint32                 old_flags;
    _fe2k_fte_idx_t        new_fte;

    /*
     * Note: The fte could change because of port <-> trunk change
     * Therefore the old fte index is saved
     */
    status      = BCM_E_NONE;
    old_fte_idx = fte_data->fte_idx;
    old_flags = bcm_egr->flags;

    L3_VERB((_SBX_D(l3_fe->fe_unit, "Update fte 0x%x\n"), old_fte_idx));

    /*
     * Get the old egress object (possibly from a remote line card)
     */
    bcm_l3_egress_t_init(&old_egr);
    status = _bcm_fe2000_l3_get_egrif_from_fte( l3_fe,
                                                old_fte_idx,
                                                L3_OR_MPLS_GET_FTE__FTE_ETE_BOTH,
                                                &old_egr);
    if (status != BCM_E_NONE) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) unable to get fte(0x%x)\n"),
                bcm_errmsg(status), old_fte_idx));
        return BCM_E_NOT_FOUND;
    }

    if (old_fte_idx < SBX_DYNAMIC_FTE_BASE(l3_fe->fe_unit)) {
        /*
         * The FTE should be in the dynamic range
         */
        return (BCM_E_PARAM);
    }

    /*
     * At this point, old_fte_idx contains the FTE index and old_egr
     * contains the old egress info. Find out what changed and take
     * appropriate action. intf, mac_addr, module, port or trunk may
     * have changed. if intf, mac_addr or module changed then add
     * a new ete and delete the old one. If only port/trunk changed then only the
     * fte needs to be locally updated
     */
    chg_flags = 0;

    /*
     * intf is a reqd parameter for REPLACE. While checking
     * we made sure that it was not zero
     */
    if (bcm_egr->intf != old_egr.intf) {
        L3_VERB((_SBX_D(l3_fe->fe_unit, "interface changed from 0x%x to 0x%x\n"),
                 (int32)old_egr.intf, (int32)bcm_egr->intf));
        chg_flags |= L3_EGR_INTF_CHG;
    }

    /*
     * if dmac is not specified copy over the old one
     * so that we have a valid dmac if some other params changed
     */
    if (BCM_MAC_IS_ZERO(bcm_egr->mac_addr)) {
        ENET_COPY_MACADDR(old_egr.mac_addr, bcm_egr->mac_addr);
    } else {
        if (ENET_CMP_MACADDR(bcm_egr->mac_addr, old_egr.mac_addr)) {
            L3_VERB((_SBX_D(l3_fe->fe_unit, "destination mac changed\n")));
            chg_flags |= L3_EGR_DMAC_CHG;
        }
    }

    if (((bcm_egr->flags & BCM_L3_UNTAG) && (!(old_egr.flags & BCM_L3_UNTAG))) ||
        (!(bcm_egr->flags & BCM_L3_UNTAG) && (old_egr.flags & BCM_L3_UNTAG))) {
        chg_flags |= L3_EGR_VIDOP_CHG;
    }

    if ((bcm_egr->module != old_egr.module) && 
        (!bcm_egr->flags & BCM_L3_TGID)) {
        L3_VERB((_SBX_D(l3_fe->fe_unit, "egress module changed from %d to %d\n"),
                 (int)old_egr.module, (int)bcm_egr->module));
        chg_flags |= L3_EGR_MODULE_CHG;
    }

    ohi =  SOC_SBX_OHI_FROM_ENCAP_ID(bcm_egr->encap_id);
    if  (ohi) {
        /*
         * 1. old_egr will have the internal value of the encap_id,
         *    So to compare with user passed encap_id, we need to
         *    comapre with converted encap_id
         * 2. Another thing that needs to be checked is that if ohi
         *    has changed, it shuould not be in use on the module specified
         *    in bcm_egr.
         */
        if (ohi != SOC_SBX_OHI_FROM_ENCAP_ID(old_egr.encap_id)) {
            L3_VERB((_SBX_D(l3_fe->fe_unit, "OHI changed from 0x%x -> 0x%x\n"),
                     SOC_SBX_OHI_FROM_ENCAP_ID(old_egr.encap_id), ohi));
            chg_flags |= L3_EGR_OHI_CHG;
        }

        /*
         * encap_id specified. The encap_id itself may or may-not have changed,
         * but the module changed. In that case also need to ensure its availability
         */
        if (chg_flags & (L3_EGR_OHI_CHG | L3_EGR_MODULE_CHG)) {
            bcm_l3_egress_t_init(&tmp_egr);
            tmp_egr.encap_id = SOC_SBX_ENCAP_ID_FROM_OHI(ohi);
            L3_VERB((_SBX_D(l3_fe->fe_unit, "validating new ohi(0x%x)\n"), ohi));
            if (l3_fe->fe_my_modid == bcm_egr->module) {
                status = _bcm_fe2000_get_local_l3_egress_from_ohi(l3_fe,
                                                                  &tmp_egr);
            }
            if (status == BCM_E_NONE) {
                status = BCM_E_EXISTS;
                L3_ERR((_SBX_D(l3_fe->fe_unit, "ohi(0x%x) already in use on module %d\n"),
                        ohi, bcm_egr->module));
                return status;
            }
        }
    }

    if (bcm_egr->port != old_egr.port) {
         L3_VERB((_SBX_D(l3_fe->fe_unit, "Egress port changed from %d to %d\n"),
                  old_egr.port, bcm_egr->port));
        chg_flags |= L3_EGR_PORT_CHG;
    }

    /*
     * If port/trunk behavior has changed, we will need to change the
     * FTE as well. This is because Trunk (Lag) ports have special FTEs
     * that are outside the dynamic range
     */
    if ((bcm_egr->flags & BCM_L3_TGID) &&
        (!(old_egr.flags & BCM_L3_TGID))) {
        chg_flags |= L3_EGR_TGID_CHG;
        L3_ERR((_SBX_D(l3_fe->fe_unit, "change from Port (%d) to Trunk (%d) \n"),
                (int)old_egr.port, (int)bcm_egr->trunk));
    }

    if ((!(bcm_egr->flags & BCM_L3_TGID)) &&
        (old_egr.flags & BCM_L3_TGID)) {
        chg_flags |= L3_EGR_TGID_CHG;
        L3_ERR((_SBX_D(l3_fe->fe_unit, "change from Trunk (%d) to Port (%d)\n"),
                (int)old_egr.trunk, (int)bcm_egr->port));
    }

    if (chg_flags == 0 && (bcm_egr->failover_id == 0)) {
        return BCM_E_NONE;
    }

    new_ete_created = 0;
    if (chg_flags & L3_EGR_TGID_CHG) {
        /*
         * We need a new FTE because we went from port to trunk or
         * vice-versa. Since the FTE changed, we need to unlink the
         * old FTE from the old ETE and relink to the ETE. The ETE
         * may or may not change. Allocate a new FTE and link it to the
         * ete and then remove the old fte
         */
        bcm_egr->flags |= BCM_L3_REPLACE;
        status = _bcm_fe2000_add_l3_fte(l3_fe, flags, bcm_egr, &new_fte);
        if (status == BCM_E_NONE) {
            ignore_status = _bcm_fe2000_destroy_fte(l3_fe,
                                                    L3_OR_MPLS_DESTROY_FTE__FTE_ONLY,
                                                    old_fte_idx,
                                                    0, /* module */
                                                    0  /* ohi    */);
            if (ignore_status != BCM_E_NONE) {
                L3_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) in destroying "
                               "fte(0x%x) while REPLACING\n"),
                        bcm_errmsg(status), old_fte_idx));
            }
        } else {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) in creating new FTE while REPLACING"
                           " egress object \n"), bcm_errmsg(status)));
            return status;
        }

        L3_VERB((_SBX_D(l3_fe->fe_unit, "fte changed to 0x%x\n"), (int)new_fte.fte_idx));
        bcm_egr->flags = old_flags;
        return BCM_E_NONE;
    }

    /*
     * If there is no port <-> trunk change, the FTE remains the same
     */
    new_fte.fte_idx = old_fte_idx;

    /*
     * Module, port are reqd parameters, so no need
     * to copy from old_bcm_egr
     */
    if (chg_flags &  (L3_EGR_INTF_CHG  |
                      L3_EGR_DMAC_CHG  |
                      L3_EGR_OHI_CHG   |
                      L3_EGR_VIDOP_CHG |
                      L3_EGR_MODULE_CHG)) {
        /*
         * In this case we also need a new ETE. So find or create the new ete.
         * Then link this FTE to the  ETE.  Since we are creating a ete, it will
         * take care of the mpls ete if there is a mpls tunnel
         */
        bcm_egr->flags |= BCM_L3_REPLACE;
        /* Availablity of this ohi in case of OHI_CHG has been verified 
         * for other cases, we'd be creating a new one
         */
        if (!(chg_flags & L3_EGR_OHI_CHG)) {
            bcm_egr->encap_id = 0;
        }
        status = _bcm_fe2000_create_ucast_l3_ete(l3_fe, new_fte.fte_idx, 
                                                 bcm_egr);

        if (BCM_FAILURE(status)) {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) could not create ete for fte(0x%x)\n"),
                    bcm_errmsg(status), new_fte.fte_idx));
            return status;
        }
        L3_VERB((_SBX_D(l3_fe->fe_unit, "error(%s) created new ete encap-id(0x%x), "
                        "fte(0x%x)\n"),
                 _SHR_ERRMSG(status), bcm_egr->encap_id,
                 new_fte.fte_idx));
        new_ete_created = 1;
    } else {
        if (SOC_SBX_OHI_FROM_ENCAP_ID(bcm_egr->encap_id) == 0) {
            /**
             * user did not specify and there was no new ete created
             * then use the old encap_id. Because map function will need
             * ohi
             */
            bcm_egr->encap_id = old_egr.encap_id;
        }
    }

    /*
     * we need to rewrite the old FTE with new info. We need to do this
     * regardless. Ex only the port may have changed or we may have a
     * new FTE in case port <-> tgid change happened
     */
    status = _bcm_fe2000_map_l3_ucast_fte(l3_fe, &new_fte, bcm_egr);

    if (BCM_FAILURE(status)) {
        if (new_ete_created) {
            ignore_status = _bcm_fe2000_destroy_l3_ete(l3_fe,
                                                       bcm_egr->module,
                                                       SOC_SBX_OHI_FROM_ENCAP_ID(bcm_egr->encap_id),
                                                       new_fte.fte_idx);
            if (BCM_FAILURE(ignore_status)) {
                L3_ERR((_SBX_D(l3_fe->fe_unit, "Could not destroy l3 ete: %s\n"),
                        bcm_errmsg(ignore_status)));
            }
        }
    }

    if (new_ete_created) {
        /*
         * Everything went well... Need to remove the old fte from the old ETE
         * If it was the last FTE on the ETE, then the ETE will be removed too.
         */
        ignore_status = _bcm_fe2000_destroy_l3_ete(l3_fe,
                                                   old_egr.module,
                                                   SOC_SBX_OHI_FROM_ENCAP_ID(old_egr.encap_id),
                                                   old_fte_idx);
        if (ignore_status != BCM_E_NONE) {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "Error in removing fte 0x%x from ETE: %s\n"),
                    old_fte_idx, bcm_errmsg(ignore_status)));
        } else {
            L3_VERB((_SBX_D(l3_fe->fe_unit, "Successfully destroyed old ete "
                            "(FTE 0x%x module %d encap-id(0x%x)\n"),
                     (int)old_fte_idx, (int)old_egr.module,
                     (int) old_egr.encap_id));
        }
        bcm_egr->flags = old_flags;
    }

    return BCM_E_NONE;
}


/*
 * Function:
 *     _bcm_fe2000_invalidate_l3_or_mpls_fte
 * Purpose:
 *     mark the fte as invalid in HW
 * Parameters:
 *     l3_fe      - (IN)     l3 fe instance
 *     fte_idx    - (IN)     handle that needs to be destroyed
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 *
 */
int
_bcm_fe2000_invalidate_l3_or_mpls_fte(_fe2k_l3_fe_instance_t   *l3_fe,
                                      uint32                    fte_idx)
{
    int rv = BCM_E_UNAVAIL;
    
    switch(SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype)
    {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_l3_invalidate_l3_or_mpls_fte(l3_fe, fte_idx);
        break;
#endif
    default:
        L3_WARN((_SBX_D(l3_fe->fe_unit, "uCode type %d is not supported\n"),
                 SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype));
        rv = BCM_E_INTERNAL;
        break;
    }

    return rv;
}



/*
 * Function:
 *     _bcm_fe2000_destroy_fte
 * Purpose:
 *     create a
 * Parameters:
 *     l3_fe      - (IN)     l3 fe instance
 *     action     - (IN)     mode of operation
 *     fte_idx    - (IN)     handle that needs to be destroyed
 *     module_id  - (IN)     module in case of FTE_ONLY
 *     ohi         - (IN)     ohi in case of FTE_ONLY
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
_bcm_fe2000_destroy_fte(_fe2k_l3_fe_instance_t *l3_fe,
                        int                     action,
                        uint32                  fte_idx,
                        bcm_module_t            module_id,
                        uint32                  ohi)
{
    int                   status;
    bcm_l3_egress_t       tmp_egr;
    bcm_module_t          del_modid;
    uint32                del_ohi;


    L3_VERB((_SBX_D(l3_fe->fe_unit, "ohi(0x%x) fte(0x%x)\n"), ohi, fte_idx));

    if (action == L3_OR_MPLS_DESTROY_FTE__FTE_HW_OHI_ETE) {
        status = _bcm_fe2000_l3_get_egrif_from_fte(l3_fe,
                                        fte_idx,
                                        L3_OR_MPLS_GET_FTE__FTE_CONTENTS_ONLY,
                                        &tmp_egr);
        if (status != BCM_E_NONE) {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) could not find fte(0x%x)\n"),
                    bcm_errmsg(status), fte_idx));
            return status;
        }
    }

    /* XXX: TBD: How to handle the MPATH FTEs */
    status = _bcm_fe2000_invalidate_l3_or_mpls_fte(l3_fe, fte_idx);
    if (status != BCM_E_NONE) {
        return status;
    }

    if (action == L3_OR_MPLS_DESTROY_FTE__FTE_ONLY) {
        return BCM_E_NONE;
    } else if (action == L3_OR_MPLS_DESTROY_FTE__FTE_HW_OHI_ETE) {
        del_modid = tmp_egr.module;
        del_ohi   = SOC_SBX_OHI_FROM_ENCAP_ID(tmp_egr.encap_id);
    } else if (action  == L3_OR_MPLS_DESTROY_FTE__FTE_OHI_ETE_GIVEN_OHI) {
        del_modid = module_id;
        del_ohi   = ohi;
    } else {
        return BCM_E_INTERNAL;
    }

    status = _bcm_fe2000_destroy_l3_ete(l3_fe, del_modid, del_ohi, fte_idx);

    if (BCM_FAILURE(status)) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "error in destroying ete. FTE 0x%x not "
                       "being invalidated\n"), fte_idx));
        return status;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_fe2000_l3_egress_create_checks
 * Purpose:
 *     sanity checks before creating egress object
 * Parameters:
 *     l3_fe   - (IN)  fe instance corresponsing to unit
 *     flags   - (IN)  BCM_L3_REPLACE: replace existing.
 *                     BCM_L3_WITH_ID: intf argument is given.
 *     bcm_egr - (IN)  Egress forwarding destination.
 *     intf    - (IN/OUT) fte index interface id corresponding to the
 *                      locally allocated FTE pointing to Egress object.
 * Returns:
 *     BCM_E_XXX
 * Note:
 */
int
_bcm_fe2000_l3_egress_create_checks(_fe2k_l3_fe_instance_t *l3_fe,
                                    uint32                  flags,
                                    bcm_l3_egress_t        *bcm_egr,
                                    bcm_if_t               *intf)
{
    int                 status;
    uint32              l3a_intf_id;

    status = BCM_E_NONE;
    L3_VVERB((_SBX_D(l3_fe->fe_unit, "Enter\n")));

    /*
     * Note: The egress (i.e. intf + ete could be on a remote module
     * Therefore we cannot fetch the l3_intf context here
     */

    if ((NULL == bcm_egr) || (NULL == intf)) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "bcm_egr or intf param is NULL\n")));
        return (BCM_E_PARAM);
    }

    /*
     * In replace case, FTE must be supplied. Therefore the WITH_ID flag
     * must be set. On the other hand in the ADD case, the FTE
     * cannot be specified because we manage the FTE space.  Therefore
     * in case of ADD, WITH_ID flag is invalid
     */
    if (flags & BCM_L3_REPLACE) {
        if (!(flags & BCM_L3_WITH_ID)) {
            L3_ERR((_SBX_D(l3_fe->fe_unit,
              "BCM_L3_REPLACE is supported only with BCM_L3_WITH_ID\n")));
             return BCM_E_PARAM;
        }
        if (!_FE2K_L3_FTE_VALID(l3_fe->fe_unit, *intf)) {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "L3 egress id is not valid\n")));
            return BCM_E_PARAM;
        }
    } else {
        if (flags & BCM_L3_WITH_ID) {
            /*
             * In add adding new egress objects we do not
             * let users choose FTE indices
             */ 
            L3_ERR((_SBX_D(l3_fe->fe_unit,
              "BCM_L3_WITH_ID is not supported in new egress create, "
              " supported only with BCM_L3_REPLACE\n")));
            return BCM_E_PARAM;
        }
    }

    /*
     * Interface Id is required for both Add and Replace cases
     * because egress objects are unique per interface
     */
    l3a_intf_id = bcm_egr->intf;
    if (!(_FE2K_L3_USER_IFID_VALID(l3a_intf_id))) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "l3a_intf_id param is invalid\n")));
        return (BCM_E_PARAM);
    }

    /*
     * For Add case, a valid dmac is reqd. In case of
     * replace, check for validity only if address is specified
     */
    if (!(flags & BCM_L3_REPLACE)) {
        if ((BCM_MAC_IS_MCAST(bcm_egr->mac_addr)) ||
            (BCM_MAC_IS_ZERO(bcm_egr->mac_addr))) {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "mac_addr is invalid\n")));
            return (BCM_E_PARAM);
        }
    } else if ((!BCM_MAC_IS_ZERO(bcm_egr->mac_addr)) &&
               (BCM_MAC_IS_MCAST(bcm_egr->mac_addr))) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "mac_addr is invalid\n")));
        return (BCM_E_PARAM);
    }

    if (!(SOC_SBX_MODID_ADDRESSABLE(l3_fe->fe_unit, bcm_egr->module))) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "modid is invalid\n")));
        return (BCM_E_PARAM);
    }

    if (bcm_egr->encap_id &&
        !SOC_SBX_IS_VALID_ENCAP_ID(bcm_egr->encap_id)) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "encap id is invalid\n")));
            return BCM_E_PARAM;
    }

    if (bcm_egr->flags & BCM_L3_TGID) {
        if (!(SBX_TRUNK_VALID(bcm_egr->trunk))) {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "trunk id is invalid\n")));
            return BCM_E_PARAM;
        }
    } else if (!(SOC_PORT_VALID(l3_fe->fe_unit, bcm_egr->port))) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "port is invalid\n")));
        return (BCM_E_PARAM);
    }

    if (SOC_IS_SBX_G2P3(l3_fe->fe_unit)) {
        if(bcm_egr->flags & BCM_L3_UNTAG) {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "BCM_L3_UNTAG not yet supported in G2P3\n")));
            return (BCM_E_PARAM);
        }
    }

    /*
     * If failover id was supplied, check for valid
     * backup egress object
     */
    if (bcm_egr->failover_id &&
        (bcm_egr->failover_id > 0 && bcm_egr->failover_id < 1024)) {
        if (bcm_egr->failover_if_id == 0 ||
            !_FE2K_L3_FTE_VALID(l3_fe->fe_unit, bcm_egr->failover_if_id)) {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "failover_if_id is invalid\n")));
            return BCM_E_PARAM;
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_fe2000_l3_find_intf_by_ifid
 * Purpose:
 *     lookup l3 interface given a intf id
 * Parameters:
 *     l3_fe  :  (IN)  fe instance corresponsing to unit
 *     intf_id:  (IN)  a unique 32 bit number
 *     l3_intf:  (OUT) interface sw context
 *
 * Returns:
 *     BCM_E_XXX
 */
int
_bcm_fe2000_l3_find_intf_by_ifid(_fe2k_l3_fe_instance_t *l3_fe,
                                 uint32                  intf_id,
                                 _fe2k_l3_intf_t        **ret_l3_intf)
{
    int                             status;
    uint32                          hash_index;
    _fe2k_l3_intf_t                *l3_intf = NULL;
    dq_p_t                          l3_intf_elem;
    dq_p_t                          hash_bucket;

    status      = BCM_E_NOT_FOUND;
    hash_index  = _FE2K_GET_INTF_ID_HASH(intf_id);
    hash_bucket = &l3_fe->fe_intf_by_id[hash_index];

    DQ_TRAVERSE(hash_bucket, l3_intf_elem) {
        _FE2K_L3INTF_FROM_IFID_DQ(l3_intf_elem, l3_intf);

        if (l3_intf->if_info.l3a_intf_id == intf_id) {
            *ret_l3_intf = l3_intf;
            status       = BCM_E_NONE;
            break;
        }

    } DQ_TRAVERSE_END(hash_bucket, l3_intf_elem);

    return status;
}

/*
 * Function:
 *     _bcm_fe2000_l3_find_intf_by_vid_mac
 * Purpose:
 *     lookup l3 interface given a vid and a macaddr
 * Parameters:
 *     l3_fe:   (IN)  fe instance corresponsing to unit
 *     key  :   (IN)  contains vid and macaddr
 *     l3_intf: (OUT) l3 interface context if found
 * Returns:
 *     BCM_E_XXX
 * Note:
 *    This will return the first matching instance in case there are
 *    multiple interfaces with the same vid and macaddr.
 */
int
_bcm_fe2000_l3_find_intf_by_vid_mac(_fe2k_l3_fe_instance_t *l3_fe,
                                    bcm_l3_intf_t          *key,
                                    _fe2k_l3_intf_t        **ret_l3_intf)
{
    int                           status;
    uint32                        hash_index;
    _fe2k_l3_intf_t              *l3_intf = NULL;
    dq_p_t                        l3_intf_elem, hash_bucket;

    status       = BCM_E_NOT_FOUND;
    *ret_l3_intf = NULL;
    hash_index   =  _FE2K_GET_INTF_VID_HASH(key->l3a_vid);
    hash_bucket  = &l3_fe->fe_intf_by_vid[hash_index];

    DQ_TRAVERSE(hash_bucket, l3_intf_elem) {

        _FE2K_L3INTF_FROM_VID_DQ(l3_intf_elem, l3_intf);

        if ((l3_intf->if_info.l3a_vid == key->l3a_vid) &&
            (!(ENET_CMP_MACADDR(l3_intf->if_info.l3a_mac_addr,
                                key->l3a_mac_addr)))) {
            status       = BCM_E_NONE;
            *ret_l3_intf = l3_intf;
            return status;
        }

    } DQ_TRAVERSE_END(hash_bucket, l3_intf_elem);

    return status;
}

/*
 * Function:
 *     bcm_fe2000_l3_intf_locate
 * Purpose:
 *     Find the L3 intf based on  (MAC, VLAN) or InterfaceId
 * Parameters:
 *     unit   - (IN) FE2000 unit number
 *     intf   - (IN) interface (MAC, VLAN) intf number
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_bcm_fe2000_l3_intf_locate(int                  unit,
                           bcm_l3_intf_t       *bcm_intf,
                           uint32               op)
{
    _fe2k_l3_intf_t            *l3_intf;
    _fe2k_l3_fe_instance_t     *l3_fe;
    int                         status;

    if (NULL == bcm_intf) {
        return (BCM_E_PARAM);
    }

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
       return  BCM_E_UNIT;
    }
    status = (op == L3_INTF_LOCATE_BY_VID) ?
        _bcm_fe2000_l3_find_intf_by_vid_mac(l3_fe,
                                            bcm_intf,
                                            &l3_intf) :
        _bcm_fe2000_l3_find_intf_by_ifid(l3_fe,
                                         bcm_intf->l3a_intf_id,
                                         &l3_intf);
    if (status == BCM_E_NONE) {
        *bcm_intf = l3_intf->if_info;
    }

    return status;
}

/*
 * Function:
 *     _bcm_fe2000_l3_free_sw_intf
 * Purpose:
 *     Free internal software information for an l3 interface
 * Parameters:
 *     l3_fe      - (IN) fe instance
 *     IN l3_intf - (IN) l3 interface instance
 * Returns:
 *     NONE
 * Note:
 *     This function cleans up state created by
 *    _bcm_fe2000_l3_alloc_intf.
 * Assumptions:
 *     The caller must ensure that the follwing conditions are met.
 *     This routine blindly cleans up state.
 *     1. The corresponding Smacs should have been removed
 *        prior to calling this
 *     2. It is safe to remove mcast ete and tunnel ete (if they exist)
 *     3. There are no etes hanging.
 *     4. Lock is held by caller
 */
STATIC int
_bcm_fe2000_l3_free_sw_intf(_fe2k_l3_fe_instance_t *l3_fe,
                            _fe2k_l3_intf_t        *l3_intf)
{
    uint32 ii;
    int    status;

    L3_VERB((_SBX_D(l3_fe->fe_unit, "vrf %d Interface Id %d \n"),
             l3_intf->if_info.l3a_vrf,
             l3_intf->if_info.l3a_intf_id));

    if ((l3_intf->if_flags & _FE2K_L3_INTF_SMAC_IN_TBL)) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "SMAC not removed before interface freed.\n")));
        return BCM_E_INTERNAL;
    }

    if (l3_intf->if_flags & _FE2K_L3_INTF_IN_IFID_LIST) {
        DQ_REMOVE(&l3_intf->if_ifid_link);
    }

    if (l3_intf->if_flags & _FE2K_L3_INTF_IN_VID_LIST) {
        DQ_REMOVE(&l3_intf->if_vid_link);
    }
    status = _sbx_gu2_resource_free(l3_fe->fe_unit,
                                    SBX_GU2K_USR_RES_IFID,
                                    1,
                                    (uint32 *)&l3_intf->if_info.l3a_intf_id,
                                    0);

    if (status != BCM_E_NONE) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "Could not free IfId %d\n"), 
                l3_intf->if_info.l3a_intf_id));
    }

    for (ii = _FE2K_L3_ETE_VIDOP__NOP;
         ii < _FE2K_L3_ETE_VIDOP__MAX_TYPES; ii++) {
        if (l3_intf->if_l2_ete[ii].ete_idx ) {
            status = _sbx_gu2_resource_free(l3_fe->fe_unit,
                                            SBX_GU2K_USR_RES_ETE_L2,
                                            1,
                                            &(l3_intf->if_l2_ete[ii].ete_idx),
                                            0);
            if (status != BCM_E_NONE) {
                L3_ERR((_SBX_D(l3_fe->fe_unit, "Could not free L2 ete 0x%x: %s"),
                        l3_intf->if_l2_ete[ii].ete_idx, bcm_errmsg(status)));
            }
        }
    }
    sal_free(l3_intf);

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_fe2000_l3_sw_uninit
 * Purpose:
 *     Free internal software information in l3 module.
 * Parameters:
 *     unit - (IN) bcm Device number
 * Returns:
 *     NONE
 * NOTE:
 *     Assumes valid unit number.
 */
int
_bcm_fe2000_l3_sw_uninit(int unit)
{
    _fe2k_l3_fe_instance_t *l3_fe;
    int                     ignore_status;
    uint32                  vrf_id;

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_ERR((_SBX_D(unit, "software context not found\n")));
        return BCM_E_INTERNAL;
    }

    /**
     * Whether we had allocated or not, try to
     * free the resource on this unit. The goal
     * is to have a clean state after this call.
     */
    vrf_id = BCM_L3_VRF_DEFAULT;
    ignore_status = _sbx_gu2_resource_free(unit,
                       SBX_GU2K_USR_RES_VRF,
                       1,
                       &vrf_id,
                       0);

    if (l3_fe->fe_raw_ete_idx != _FE2K_INVALID_ETE_IDX) {
        ignore_status = _sbx_gu2_resource_free(unit,
                                               SBX_GU2K_USR_RES_ETE_L2,
                                               1,
                                               &l3_fe->fe_raw_ete_idx,
                                               0);
        if (ignore_status != BCM_E_NONE) {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) resource free raw-ete(0x%x)\n"),
                    bcm_errmsg(ignore_status), l3_fe->fe_raw_ete_idx));
        }
    }

    if (l3_fe->fe_drop_vrf != _FE2K_INVALID_VRF) {
        ignore_status = _sbx_gu2_resource_free(unit,
                                               SBX_GU2K_USR_RES_VRF,
                                               1,
                                               &l3_fe->fe_drop_vrf,
                                               0);
    }

    if (l3_fe->fe_vsi_default_vrf != _FE2K_INVALID_VRF) {
        ignore_status = _sbx_gu2_resource_free(unit,
                                               SBX_GU2K_USR_RES_VSI,
                                               1,
                                               &l3_fe->fe_vsi_default_vrf,
                                               0);
    }

    if (_l3_mlock[unit] != NULL) {
        sal_mutex_destroy(_l3_mlock[unit]);
        _l3_mlock[unit] = NULL;
    }

    sal_free(l3_fe);
    l3_fe = NULL;
    _bcm_fe2000_set_l3_instance_for_unit(unit, NULL);

    return BCM_E_NONE;
}




int
_bcm_fe2000_l3_hw_init(int unit, _fe2k_l3_fe_instance_t *l3_fe)
{
    int rv = BCM_E_UNAVAIL;

    switch(SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype)
    {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_l3_hw_init(unit, l3_fe);
        break;
#endif
    default:
        L3_WARN((_SBX_D(l3_fe->fe_unit, "uCode type %d is not supported\n"),
                 SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype));
        rv = BCM_E_INTERNAL;
        break;
    }

    return rv;
}

/*
 * Function:
 *     bcm_fe2000_l3_cleanup
 * Purpose:
 *     Cleanup the L3 and bring it
 *     back to state before l3_init
 * Parameters:
 *     unit  - (IN) FE2K unit number
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      There is ordering restriction
 *      bcm_l3_init();
 *      bcm_mpls_init();
 *      ...
 *      ...
 *      bcm_mpls_cleanup();
 *      bcm_l3_cleanup();
 */

int
bcm_fe2000_l3_cleanup(int unit)
{
    _fe2k_l3_fe_instance_t     *l3_fe;
    int                         rv = BCM_E_NONE;

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_ERR((_SBX_D(unit, "bcm_l3_sw_uninit: "
                       "software context for unit %d not found"), unit));
        L3_UNLOCK(unit);
        return BCM_E_NONE;
    }

    /* XXX: TBD:
     * make sure no resources are being used
     */

    rv = _bcm_fe2000_l3_sw_uninit(unit);

    /* L3_UNLOCK(unit); */
    return rv;
}

/*
 * Function:
 *     _bcm_fe2000_l3_sw_init
 * Purpose:
 *     Initialize internal software information in l3 module.
 * Parameters:
 *     unit - (IN) Device number
 * Returns:
 *     BCM_E_NONE   - Success
 *     BCM_E_MEMORY - Failed to allocate required lock or memory
 * NOTE:
 *     Assumes valid unit number.
 */
STATIC int
_bcm_fe2000_l3_sw_init(int unit)
{
    uint32                     ii, size;
    int                        status;
    uint32                     vrf_id;
    _fe2k_l3_fe_instance_t    *l3_fe;
    bcm_module_t               my_modid;
    uint32                     ignore;

    if (L3_LOCK_CREATED_ALREADY(unit)) {
        status = bcm_fe2000_l3_cleanup(unit);
        if (BCM_FAILURE(status)) {
            return status;
        }
    }

    /**
     * XXX: bcm_l3_init() needs to be called from thread safe
     * user code. Once init() is successful, other calls
     * have the lock to make sure access is ordered.
     */
    if (!(L3_LOCK_CREATED_ALREADY(unit))) {
        if ((_l3_mlock[unit] = sal_mutex_create("bcm_l3_lock")) == NULL) {
            return BCM_E_MEMORY;
        }
    }

    l3_fe     = NULL;
    status    = BCM_E_NONE;

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe) {
        return BCM_E_EXISTS;
    }

    BCM_IF_ERROR_RETURN(
        bcm_stk_my_modid_get(unit,
                             &my_modid));

    size  = sizeof(_fe2k_l3_fe_instance_t);
    l3_fe = sal_alloc(size, (char*)FUNCTION_NAME());
    if (l3_fe == NULL) {
        return (BCM_E_MEMORY);
    }

    sal_memset(l3_fe, 0, size);
    l3_fe->fe_vsi_default_vrf = _FE2K_INVALID_VPN_ID;
    l3_fe->fe_drop_vrf        = _FE2K_INVALID_VRF;
    l3_fe->fe_raw_ete_idx     = _FE2K_INVALID_ETE_IDX;

    _bcm_fe2000_set_l3_instance_for_unit(unit, l3_fe);
    l3_fe->fe_unit     = unit;
    l3_fe->fe_my_modid = my_modid;

    BCM_IF_ERROR_RETURN
        (soc_sbx_g2p3_vlan_ft_base_get(l3_fe->fe_unit, 
                                       &l3_fe->vlan_ft_base));
    BCM_IF_ERROR_RETURN
        (soc_sbx_g2p3_mc_ft_offset_get(l3_fe->fe_unit, 
                                       &l3_fe->umc_ft_offset));
    BCM_IF_ERROR_RETURN
        (soc_sbx_g2p3_vpws_ft_offset_get(l3_fe->fe_unit, 
                                         &l3_fe->vpws_uni_ft_offset));
    BCM_IF_ERROR_RETURN
        (soc_sbx_g2p3_max_pids_get(l3_fe->fe_unit, 
                                   &l3_fe->max_pids));

#ifdef BCM_WARM_BOOT_SUPPORT
    BCM_IF_ERROR_RETURN
        (_sbx_gu2_alloc_range_get(l3_fe->fe_unit, SBX_GU2K_USR_RES_LINE_VSI,
                                  &l3_fe->line_vsi_base, &ignore));
#endif /* BCM_WARM_BOOT_SUPPORT */

    /* validate driver */
    if ((SOC_SBX_CONTROL(unit)->drv) == NULL) {
        _bcm_fe2000_l3_sw_uninit(unit);
        return BCM_E_UNIT;
    }

    for (ii = 0; ii < _FE2K_INTF_ID_HASH_SIZE; ii++) {
        DQ_INIT(&l3_fe->fe_intf_by_id[ii]);
    }

    for (ii = 0; ii < _FE2K_INTF_VID_HASH_SIZE; ii++) {
        DQ_INIT(&l3_fe->fe_intf_by_vid[ii]);
    }

    for (ii = 0; ii <   _FE2K_IPMC_HASH_SIZE; ii++) {
        DQ_INIT(&l3_fe->fe_ipmc_by_id[ii]);
    }

    for (ii=0; ii < _FE2K_EGR_MP_HASH_SIZE; ii++) {
        DQ_INIT(&l3_fe->fe_mp_by_egr[ii]);
        DQ_INIT(&l3_fe->fe_mp_by_mpbase[ii]);
    }

    for (ii=0; ii < _FE2K_ETE_IDX_HASH_SIZE; ii++) {
        DQ_INIT(&l3_fe->fe_eteidx2_l3_ete[ii]);
    }

    for (ii=0; ii < _FE2K_FTE_IDX_HASH_SIZE; ii++) {
        DQ_INIT(&l3_fe->fe_fteidx2_fte[ii]);
    }

    for (ii=0; ii < _FE2K_OHI_IDX_HASH_SIZE; ii++) {
        DQ_INIT(&l3_fe->fe_ohi2_l3_ete[ii]);
    }

    for (ii=0; ii < _FE2K_OHI_IDX_HASH_SIZE; ii++) {
        DQ_INIT(&l3_fe->fe_ohi2_vc_ete[ii]);
    }

    for (ii=0; ii < _FE2K_FTE_IDX_HASH_SIZE; ii++) {
        DQ_INIT(&l3_fe->fe_fteidx2_fte[ii]);
    }

    l3_fe->fe_mp_set_size    = _FE2K_L3_ECMP_DEFAULT;
    l3_fe->fe_flags         &= ~( _FE2K_L3_FE_FLG_MP_SIZE_SET);

    status = bcm_cosq_config_get(l3_fe->fe_unit,
                                 &l3_fe->fe_cosq_config_numcos);
    if (status != BCM_E_NONE) {
        _bcm_fe2000_l3_sw_uninit(unit);
        return status;
    }

    /***
     * Make all our resource allocations upfront, then go into the specialized
     * table writes based on ucode version
     */
    status = _sbx_gu2_resource_alloc(l3_fe->fe_unit,
                                     SBX_GU2K_USR_RES_ETE_L2,
                                     1,
                                     &l3_fe->fe_raw_ete_idx,
                                     0);
    if (status != BCM_E_NONE) {
      _bcm_fe2000_l3_sw_uninit(unit);
      return status;
    }

    /**
     * Allocate the default VRF-ID. This needs to be programmed
     * into the RoutedVlan2Etc in vlan.c [based on the control
     * value passed by the user to bcm_vlan_control_vlan_set()]
     *
     * Do not touch vpn structures here , mpls_init will overwrite.
     *
     */
    vrf_id = BCM_L3_VRF_DEFAULT;
    status = _sbx_gu2_resource_alloc(l3_fe->fe_unit,
                                     SBX_GU2K_USR_RES_VRF,
                                     1,
                                     &vrf_id,
                                     _SBX_GU2_RES_FLAGS_RESERVE);
    if (status != BCM_E_NONE) {
        _bcm_fe2000_l3_sw_uninit(unit);
        return status;
    }

    l3_fe->fe_drop_vrf = _FE2K_L3_VRF_DROP;
    status = _sbx_gu2_resource_alloc(l3_fe->fe_unit,
                                     SBX_GU2K_USR_RES_VRF,
                                     1,
                                     &l3_fe->fe_drop_vrf,
                                     _SBX_GU2_RES_FLAGS_RESERVE);
    if (status != BCM_E_NONE) {
      _bcm_fe2000_l3_sw_uninit(unit);
      return status;
    }

    /* reserve teh default VSI for VRF to simplify warm boot */
    status = _sbx_gu2_alloc_range_get(l3_fe->fe_unit, SBX_GU2K_USR_RES_VSI,
                                      &l3_fe->fe_vsi_default_vrf, &ignore);

    if (status != BCM_E_NONE) {
        _bcm_fe2000_l3_sw_uninit(unit);
        return status;
    }

    status = _sbx_gu2_resource_alloc(l3_fe->fe_unit,
                                     SBX_GU2K_USR_RES_VSI,
                                     1,
                                     &l3_fe->fe_vsi_default_vrf,
                                     _SBX_GU2_RES_FLAGS_RESERVE);

    if (status != BCM_E_NONE) {
      _bcm_fe2000_l3_sw_uninit(unit);
      return status;
    }

    status = 
        soc_sbx_g2p3_ipv4_vrf_bits_get(unit,
                                       (uint32_t *) &l3_fe->fe_ipv4_vrf_bits);
    if (BCM_FAILURE(status)) {
      _bcm_fe2000_l3_sw_uninit(unit);
      return status;
    }
    
#ifdef BCM_WARM_BOOT_SUPPORT

    size = L3_WB_SCACHE_SIZE;
    status = soc_wb_state_alloc_and_check(unit, l3_fe->wb_hdl, &size, 
                                          L3_WB_CURRENT_VERSION,
                                          NULL);
    if (BCM_FAILURE(status)) {
        _bcm_fe2000_l3_sw_uninit(unit);
        return status; 
    }
    l3_fe->scache_size = SOC_WB_SCACHE_SIZE(size);

    if (SOC_WARM_BOOT(unit)) {
        fe2k_l3_wb_mem_layout_t *layout;
        
        l3_wb_layout_get(unit, &layout);
        soc_scache_handle_lock(unit, l3_fe->wb_hdl);
        
        status = _bcm_fe2000_wb_recover(l3_fe, layout);
        
        soc_scache_handle_unlock(unit, l3_fe->wb_hdl);
    }

#endif /* BCM_WARM_BOOT_SUPPORT */ 

    status = _bcm_fe2000_l3_hw_init(unit, l3_fe);
    if (status != BCM_E_NONE) {
      _bcm_fe2000_l3_sw_uninit(unit);
      return status;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_fe2000_l3_intf_create_replace
 * Purpose:
 *     Create or replace l3 intf
 * Parameters:
 *     unit      - (IN)  fe2k unit number
 *     l3_fe     - (IN)  l3 fe instance context
 *     bcm_intf  - (IN)  user (bcm) interface data
 *     ret_intf  - (OUT) L3 internal interface structure info
 *
 * Returns:
 *     BCM_E_XXX
 * NOTE:
 */

int
_bcm_fe2000_l3_intf_create_replace(int                     unit,
                                   _fe2k_l3_fe_instance_t *l3_fe,
                                   bcm_l3_intf_t          *bcm_intf,
                                   _fe2k_l3_intf_t        **ret_intf)
{
    _fe2k_l3_intf_t            *l3_intf;
    uint32                      intf_created;
    int                         status;

    status = BCM_E_NONE;

    intf_created = 0;
    if (!(bcm_intf->l3a_flags & BCM_L3_REPLACE)) {

        /*
         * alloc a new sw context and copy over user data
         */
        status = _bcm_fe2000_l3_alloc_intf(l3_fe, bcm_intf, NULL, &l3_intf);
        if (status != BCM_E_NONE) {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "error in allocating interface: %s\n"),
                    bcm_errmsg(status)));
            return status;
        }

        status = _bcm_fe2000_l3_add_intf(l3_fe, bcm_intf, l3_intf);
        if (status != BCM_E_NONE) {
            _bcm_fe2000_l3_free_sw_intf(l3_fe, l3_intf);
            L3_ERR((_SBX_D(l3_fe->fe_unit, "error in adding interface: %s\n"), 
                    bcm_errmsg(status)));
            l3_intf = NULL;
            return status;
        }
    } else {
        if (bcm_intf->l3a_flags & BCM_L3_WITH_ID) {
            status = _bcm_fe2000_l3_find_intf_by_ifid(l3_fe,
                                                      bcm_intf->l3a_intf_id,
                                                      &l3_intf);
        } else {
            status = _bcm_fe2000_l3_find_intf_by_vid_mac(l3_fe,
                                                         bcm_intf,
                                                         &l3_intf);
        }
        if (status != BCM_E_NONE) {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "could not find Interface: %s\n"),
                    bcm_errmsg(status)));
            return status;
        }

        status = _bcm_fe2000_l3_update_intf(l3_fe,
                                            bcm_intf,
                                            l3_intf);
        if (status != BCM_E_NONE) {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "error in updating interface: %s\n"),
                    bcm_errmsg(status)));
            return status;
        }
    }

    if (status == BCM_E_NONE) {
        *ret_intf = l3_intf;
    }

    return status;
}

/*
 * Function:
 *     _bcm_fe2000_l3_sw_intf_create_checks
 * Purpose:
 *     Consistency checks before a interface create is allowed
 * Parameters:
 *     l3_fe     - (IN) l3 fe instance context
 *     bcm_intf  - (IN) user (bcm) interface data
 * Returns:
 *     BCM_E_XXX
 * NOTE:
 *     Only the following L3 flags are supported for interfaces
 *     - BCM_L3_UNTAG,
 *     - BCM_L3_ADD_TO_ARL
 *     - BCM_L3_WITH_ID
 *     - BCM_L3_REPLACE
 *     - BCM_L3_RPE
 */
int
_bcm_fe2000_l3_sw_intf_create_checks(_fe2k_l3_fe_instance_t *l3_fe,
                                     bcm_l3_intf_t          *bcm_intf)
{
    uint32                i, unit, ifid;
    int                   status;
    _fe2k_l3_intf_t      *l3_intf;

    unit     = l3_fe->fe_unit;
    l3_intf  = NULL;

    if (bcm_intf->l3a_flags & ~(L3_INTF_SUPPORTED_FLAGS)) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "unsupported flags 0x%x"), bcm_intf->l3a_flags));
        return BCM_E_PARAM;
    }

    if (!_BCM_VRF_VALID(bcm_intf->l3a_vrf)) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "invalid vrf(0x%x)\n"), bcm_intf->l3a_vrf));
        return BCM_E_PARAM;
    }

    if ((bcm_intf->l3a_flags & BCM_L3_IP6) && bcm_intf->l3a_vrf) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "VRFs not supported on IPV6 interfaces\n")));
        return BCM_E_PARAM;
    }

    if (l3_fe->fe_vpn_by_vrf[bcm_intf->l3a_vrf] == NULL) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "vrf(0x%x) not yet created\n"), bcm_intf->l3a_vrf));
        return BCM_E_PARAM;
    }


    /*
     * REPLACE may not come WITH_ID because user can specify vid+mac
     */
    if (bcm_intf->l3a_flags & BCM_L3_WITH_ID) {
        if (!_FE2K_L3_USER_IFID_VALID(bcm_intf->l3a_intf_id)) {
            return BCM_E_PARAM;
        }
    }

    ifid =  _FE2K_IFID_FROM_USER_HANDLE(bcm_intf->l3a_intf_id);

    if ( bcm_intf->l3a_vid && !BCM_VLAN_VALID(bcm_intf->l3a_vid)) {
        return (BCM_E_PARAM);
    }

    /*
     *  in case of add, if ttl is not specified we will assign a default
     */
    if (bcm_intf->l3a_flags & BCM_L3_REPLACE) {

        if (!BCM_MAC_IS_ZERO(bcm_intf->l3a_mac_addr)) {
            if (BCM_MAC_IS_MCAST(bcm_intf->l3a_mac_addr)) {
                return (BCM_E_PARAM);
            }
        }

        if (bcm_intf->l3a_flags & BCM_L3_WITH_ID) {
            status = _bcm_fe2000_l3_find_intf_by_ifid(l3_fe,
                                                      ifid,
                                                      &l3_intf);
        } else {
            status = _bcm_fe2000_l3_find_intf_by_vid_mac(l3_fe,
                                                         bcm_intf,
                                                         &l3_intf);
        }
        if (status != BCM_E_NONE) {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "Interface not found: %s\n"), bcm_errmsg(status)));
            return BCM_E_NOT_FOUND;
        }
    } else {
        if ((BCM_MAC_IS_MCAST(bcm_intf->l3a_mac_addr)) ||
            (BCM_MAC_IS_ZERO(bcm_intf->l3a_mac_addr))) {
            return (BCM_E_PARAM);
        }

        if (bcm_intf->l3a_flags & (BCM_L3_WITH_ID)) {
            /*
             * Create an interface with specified Id.
             * Make sure it does not exist
             */
            status = _bcm_fe2000_l3_find_intf_by_ifid(l3_fe,
                                                      ifid,
                                                      &l3_intf);
            if (status == BCM_E_NONE) {
                L3_ERR((_SBX_D(l3_fe->fe_unit, "Specified interface (Ifid) exists\n")));
                return BCM_E_EXISTS;
            }

            /*
             * Also make sure that vid+macdoes not exist either
             */
            status = _bcm_fe2000_l3_find_intf_by_vid_mac(l3_fe,
                                                         bcm_intf,
                                                         &l3_intf);
            if (status == BCM_E_NONE) {
                L3_ERR((_SBX_D(l3_fe->fe_unit, "Specified interface (vid+mac)exists\n")));
                return BCM_E_EXISTS;
            }
        }
    }

    /*
     * A change in vrf is not allowed if there are ETEs hanging off this
     * interface. We cannot destroy the underlying ETEs because the user
     * will be holding references
     */
    if ((bcm_intf->l3a_flags & BCM_L3_REPLACE) &&
        (l3_intf != NULL && bcm_intf->l3a_vrf != l3_intf->if_info.l3a_vrf)) {
        for (i = 0; i < _FE2K_INTF_L3ETE_HASH_SIZE; i++) {
            if (!(DQ_EMPTY(&l3_intf->if_ete_hash[i]))) {
                return BCM_E_BUSY;
            }
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_fe2000_l3_update_smac_tables
 * Purpose:
 *     Update the software state and update the following tables
 *     in HW
 *     1. Local Station Match Table
 *     2. SmacIdx2Smac
 *
 * Parameters:
 *     l3_fe      - (IN) l3 fe instance
 *     intf       - (IN) user (bcm) mac address
 *     op         - (IN) ADD/DEL
 *     ul_idx     - (IN/OUT)index into local SMAC table
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 *     1. Lock is held by caller.
 */
int
_bcm_fe2000_l3_update_smac_tables(_fe2k_l3_fe_instance_t *l3_fe,
                                  bcm_mac_t               smac,
                                  int                     op,
                                  uint32                 *ul_ingr_idx,
                                  uint32                 *ul_egr_idx)
{
    int rv = BCM_E_PARAM;

    if (op == L3_INTF_DEL_SMAC) {
        L3_VERB((_SBX_D(l3_fe->fe_unit, "freeing lsmi=%d esmi=%d\n"),
                 *ul_ingr_idx, *ul_egr_idx));

        rv = _sbx_gu2_esmac_idx_free(l3_fe->fe_unit, smac, ul_egr_idx);
        if (rv == BCM_E_EMPTY) {
            rv = BCM_E_NONE;
        }

        if (BCM_FAILURE(rv)) {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "error freeing idx %d, MAC addr(" MAC_FMT
                    "): %d %s\n"),
                    *ul_egr_idx, MAC_PFMT(smac), rv, bcm_errmsg(rv)));
        }

        rv = _sbx_gu2_ismac_idx_free(l3_fe->fe_unit, smac, _SBX_GU2_RES_UNUSED_PORT, ul_ingr_idx);
        if (rv == BCM_E_EMPTY) {
            rv = BCM_E_NONE;
        }
        if (BCM_FAILURE(rv)) {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "error freeing idx %d, MAC addr(" MAC_FMT
                           "): %d %s\n"),
                    *ul_ingr_idx, MAC_PFMT(smac), rv, bcm_errmsg(rv)));
        }

    } else if (op == L3_INTF_ADD_SMAC) {
        int flags = 0;

        if (SOC_WARM_BOOT(l3_fe->fe_unit)) {
            flags = _SBX_GU2_RES_FLAGS_RESERVE;
        }

        rv = _sbx_gu2_esmac_idx_alloc(l3_fe->fe_unit, flags, smac, ul_egr_idx);
        if (BCM_FAILURE(rv)) {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "failed to allocate esmac idx: %d %s\n"),
                    rv, bcm_errmsg(rv)));
            return rv;
        }

        rv = _sbx_gu2_ismac_idx_alloc(l3_fe->fe_unit, flags, smac, _SBX_GU2_RES_UNUSED_PORT, ul_ingr_idx);
        if (BCM_FAILURE(rv)) {
            int junk;
            L3_ERR((_SBX_D(l3_fe->fe_unit, "failed to allocate ismac idx: %d %s\n"),
                    rv, bcm_errmsg(rv)));
            junk = _sbx_gu2_esmac_idx_free(l3_fe->fe_unit, smac, ul_egr_idx);
            return rv;
        }

        L3_VERB((_SBX_D(l3_fe->fe_unit, "%s lsmi=%d esmi=%d rv=%d\n"),
                 ((flags & _SBX_GU2_RES_FLAGS_RESERVE) ? 
                  "reserved" : "allocated"), *ul_ingr_idx, *ul_egr_idx, rv));

        rv = _bcm_fe2000_g2p3_l3_update_smac_tables(l3_fe, smac, op,
                                                    *ul_ingr_idx,
                                                    *ul_egr_idx);

        if (BCM_FAILURE(rv)) {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "failed to update smac tables: %d %s\n"),
                    rv, bcm_errmsg(rv)));
        }
    }

    return rv;
}

/*
 * Function:
 *     _bcm_fe2000_l3_alloc_intf
 * Purpose:
 *     Initialize internal software information in l3 interface
 * Parameters:
 *     l3_fe    - (IN) l3 fe instance context
 *     intf     - (IN) user (bcm) interface data
 *     ret_intf - (OUT) local sw context
 * Returns:
 *     BCM_E_XXX
 * NOTE:
 *     1. Assumes valid vid
 *     2. Is called after it is ascertained that a new instance
 *        needs to be allocated. For xeample if the user has specified
 *        an ifid (and REPLACE flag is not set), it should not exist.
 *        Therefore this routine just creates the software instance
 *        and links it into the ifid list and the vid list.
 *     3. Also allocate an l2 ete for every vid op. These etee can
 *        potentially be shared by other l3 etes
 */
STATIC int
_bcm_fe2000_l3_alloc_intf(_fe2k_l3_fe_instance_t *l3_fe,
                          bcm_l3_intf_t          *bcm_intf,
                          uint32                 *reserve_etes,
                          _fe2k_l3_intf_t        **ret_intf)
{
    int                            status, flags;
    uint32                         vid, ii, hash_index, size, ifid = ~0;
    _fe2k_l3_intf_t               *l3_intf;
    uint32                         etes[_FE2K_L3_ETE_VIDOP__MAX_TYPES];
    dq_p_t                         hash_bucket;

    status       = BCM_E_NONE;
    vid          = bcm_intf->l3a_vid;
    size         = sizeof(_fe2k_l3_intf_t);
    l3_intf      = (_fe2k_l3_intf_t *)sal_alloc(size, "Intf-Data");
    if (l3_intf) {
        sal_memset(l3_intf, 0, size);
    } else {
        return BCM_E_MEMORY;
    }

    flags = 0;
    if (bcm_intf->l3a_flags & BCM_L3_WITH_ID) {
        flags = _SBX_GU2_RES_FLAGS_RESERVE;
    }

    ifid =  bcm_intf->l3a_intf_id;
    status = _sbx_gu2_resource_alloc(l3_fe->fe_unit, SBX_GU2K_USR_RES_IFID,
                                     1, &ifid, flags);
    flags = 0;
    bcm_intf->l3a_intf_id = ifid;
    L3_VERB((_SBX_D(l3_fe->fe_unit, "%s interface id 0x%x\n"), 
             (flags &_SBX_GU2_RES_FLAGS_RESERVE) ? "reserved" : "allocated",
             ifid));

    if (status != BCM_E_NONE) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) could not allocate ifid"),
                bcm_errmsg(status)));
        sal_free(l3_intf);
        return status;
    }

    /* Needed for warm boot */
    if (reserve_etes) {
        for (ii = 0; ii < _FE2K_L3_ETE_VIDOP__MAX_TYPES; ii++) {
            etes[ii] = reserve_etes[ii];
        }
        flags = _SBX_GU2_RES_FLAGS_RESERVE;
    } else {
        for (ii = 0; ii < _FE2K_L3_ETE_VIDOP__MAX_TYPES; ii++) {
            etes[ii] = 0;
        }
    }

    /* allocate the default L2 ete indices -
     * Allocate contiguous ETEs to simplify Warm Boot, there is no ucode
     * depenedency on these being contiguous 
     */
    status = _sbx_gu2_resource_alloc(l3_fe->fe_unit,
                                     SBX_GU2K_USR_RES_ETE_L2,
                                     _FE2K_L3_ETE_VIDOP__MAX_TYPES,
                                     etes, flags);
    if (BCM_FAILURE(status)) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "Could not allocate L2 ete: %s\n"), 
                bcm_errmsg(status)));
        _sbx_gu2_resource_free(l3_fe->fe_unit,SBX_GU2K_USR_RES_IFID, 1, 
                               &ifid, 0);
        sal_free(l3_intf);
        return status;
    }

    for (ii = 0; ii < _FE2K_L3_ETE_VIDOP__MAX_TYPES; ii++) {
        l3_intf->if_l2_ete[ii].ete_idx = etes[ii];
        L3_VERB((_SBX_D(l3_fe->fe_unit, "allocated L2 ete (vid op %d) ete addr 0x%x\n"),
                 ii, l3_intf->if_l2_ete[ii].ete_idx));
    }

    if (bcm_intf->l3a_mtu == 0) {
        bcm_intf->l3a_mtu = _FE2K_DEFAULT_EGR_MTU;
    }

    sal_memcpy(&l3_intf->if_info, bcm_intf,
               sizeof(bcm_l3_intf_t));

    l3_intf->if_info.l3a_intf_id = ifid;

    hash_index         = _FE2K_GET_INTF_ID_HASH(ifid);
    hash_bucket        = &l3_fe->fe_intf_by_id[hash_index];
    l3_intf->if_flags |= _FE2K_L3_INTF_IN_IFID_LIST;
    DQ_INSERT_HEAD(hash_bucket, &l3_intf->if_ifid_link);

    hash_index         = _FE2K_GET_INTF_VID_HASH(vid);
    hash_bucket        = &l3_fe->fe_intf_by_vid[hash_index];;
    l3_intf->if_flags |= _FE2K_L3_INTF_IN_VID_LIST;
    DQ_INSERT_HEAD(hash_bucket, &l3_intf->if_vid_link);

    for (ii=0; ii < _FE2K_INTF_L3ETE_HASH_SIZE; ii++) {
        DQ_INIT(&l3_intf->if_ete_hash[ii]);
    }

    DQ_INIT(&l3_intf->if_oam_ep_list);

    *ret_intf = l3_intf;
    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_fe2000_l3_flush_cache
 * Purpose:
 *     Unconditionally flush the ilib cached transactions to hw
 * Parameters:
 *     unit - BCM Device number
 * Returns:
 *     BCM_E_NONE      - Success
 */
int _bcm_fe2000_l3_flush_cache(int unit, int flag)
{

   int                  status = BCM_E_NONE;
   _fe2k_l3_fe_instance_t      *l3_fe;

   BCM_IF_ERROR_RETURN(L3_LOCK(unit));

   l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
   if (l3_fe == NULL) {
       L3_UNLOCK(unit);
       return BCM_E_UNIT;
   }

   switch (SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
   case SOC_SBX_UCODE_TYPE_G2P3:
       status = _bcm_fe2000_g2p3_l3_flush_cache(l3_fe, flag);
       break;
#endif
   default:
        L3_UKNOWN_UCODE_WARN(l3_fe->fe_unit);
        status = BCM_E_CONFIG;
   }

   L3_UNLOCK(unit);

   return status;
}

/*
 * Function:
 *     _bcm_fe2000_l3_add_intf
 * Purpose:
 *     Update HW tables with mac addr of new interface
 * Parameters:
 *     l3_fe    - (IN) l3 fe instance context
 *     intf     - (IN) user (bcm) interface data
 *     ret_intf - (IN) local sw context
 * Returns:
 *     BCM_E_XXX
 * NOTE:
 *     The caller has made sure that it does not already
 *     exist.
 */
STATIC int
_bcm_fe2000_l3_add_intf(_fe2k_l3_fe_instance_t *l3_fe,
                        bcm_l3_intf_t          *bcm_intf,
                        _fe2k_l3_intf_t        *l3_intf)
{
    int               status, ignore_status;
    uint32            ii, ul_ingr_mac_idx, ul_egr_mac_idx;

    status = _bcm_fe2000_l3_update_smac_tables(l3_fe,
                                               bcm_intf->l3a_mac_addr,
                                               L3_INTF_ADD_SMAC,
                                               &ul_ingr_mac_idx,
                                               &ul_egr_mac_idx);

    if (status != BCM_E_NONE) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "Could not add MAC address into HW: %s\n"),
                bcm_errmsg(status)));
        return status;
    }

    l3_intf->if_ingr_smac_idx   = ul_ingr_mac_idx;
    l3_intf->if_egr_smac_idx    = ul_egr_mac_idx;
    l3_intf->if_flags          |= _FE2K_L3_INTF_SMAC_IN_TBL;

    /*
     * Note that if there is any error we decr the ref_cnt on the mac address
     * (which will free if neccessary). However if there is a error in map or
     * set of ete, the caller will simply free the ete indices. Therefore
     * partial failures are handled correctly
     */
    for (ii = 0; ii < _FE2K_L3_ETE_VIDOP__MAX_TYPES; ii++) {
        soc_sbx_g2p3_etel2_t l2_hw_ete;
        status = _bcm_fe2000_g2p3_map_l2_ete(l3_fe,
                                             (_fe2k_l3_ete_vidop_t) ii,
                                             l3_intf->if_egr_smac_idx,
                                             bcm_intf->l3a_vid,
                                             bcm_intf->l3a_mtu,
                                             (void*)&l2_hw_ete);

        if (status != BCM_E_NONE) {
            break;
        }
        status = _bcm_fe2000_g2p3_set_l2_ete(l3_fe,
                                             &l3_intf->if_l2_ete[ii],
                                             (void*)&l2_hw_ete);
        if (status != BCM_E_NONE) {
            break;
        }
    }

    if (status != BCM_E_NONE) {
        ignore_status = _bcm_fe2000_l3_update_smac_tables(l3_fe,
                                                          bcm_intf->l3a_mac_addr,
                                                          L3_INTF_DEL_SMAC,
                                                          &ul_ingr_mac_idx,
                                                          &ul_egr_mac_idx);
        return status;
    }

    return BCM_E_NONE;
}

int
_bcm_fe2000_l3_oam_endpoint_associate(int      unit, 
                                      bcm_if_t l3_intf_id,
                                      dq_p_t  oamep,
                                      uint8_t  add)
{
    _fe2k_l3_intf_t            *l3_intf;
    _fe2k_l3_fe_instance_t     *l3_fe;
    int                         status = BCM_E_NONE;
    uint32_t                    ifid;

    if (!oamep) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));
    ifid =  _FE2K_IFID_FROM_USER_HANDLE(l3_intf_id);

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (!l3_fe) {
       status = BCM_E_UNIT;
    } else {
        status = _bcm_fe2000_l3_find_intf_by_ifid(l3_fe,
                                                  ifid,
                                                  &l3_intf);
        if (BCM_SUCCESS(status)) {
            if(add) {
                DQ_INSERT_HEAD(&l3_intf->if_oam_ep_list, oamep);
            } else {
                DQ_REMOVE(oamep);
            }
        } else {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "Could not find interface ID 0x%x: %d %s\n"),
                    l3_intf_id, status, bcm_errmsg(status)));
        }
    }

    L3_UNLOCK(unit);
    return status;
}

/*
 * Function:
 *     _bcm_fe2000_l3_handle_smac_change
 * Purpose:
 *     Handle a change in mac addr of the interface
 * Parameters:
 *     l3_fe    - (IN) l3 fe instance context
 *     intf     - (IN) user (bcm) interface data
 *     ret_intf - (IN) local sw context
 * Returns:
 *     BCM_E_XXX
 * NOTE:
 *     Delete the old mac addr (iff ref_count == 0) and
 *     Add the new addr
 */
int
_bcm_fe2000_l3_handle_smac_change(_fe2k_l3_fe_instance_t *l3_fe,
                                  bcm_l3_intf_t          *bcm_intf,
                                  _fe2k_l3_intf_t        *l3_intf)
{
    int       status;
    uint32    old_ul_ingr_idx, old_ul_egr_idx;
    uint32    new_ul_ingr_idx, new_ul_egr_idx;
    bcm_mac_t old_smac;

    old_ul_ingr_idx   = l3_intf->if_ingr_smac_idx;
    old_ul_egr_idx    = l3_intf->if_egr_smac_idx;

    ENET_COPY_MACADDR(l3_intf->if_info.l3a_mac_addr, old_smac);

    status = _bcm_fe2000_l3_update_smac_tables(l3_fe,
                                               bcm_intf->l3a_mac_addr,
                                               L3_INTF_ADD_SMAC,
                                               &new_ul_ingr_idx,
                                               &new_ul_egr_idx);
    if (status == BCM_E_NONE) {
        ENET_COPY_MACADDR(bcm_intf->l3a_mac_addr, 
                          l3_intf->if_info.l3a_mac_addr);

        l3_intf->if_ingr_smac_idx = new_ul_ingr_idx;
        l3_intf->if_egr_smac_idx  = new_ul_egr_idx;
        l3_intf->if_flags        |= _FE2K_L3_INTF_SMAC_IN_TBL;

        status = _bcm_fe2000_l3_update_smac_tables(l3_fe,
                                                   old_smac,
                                                   L3_INTF_DEL_SMAC,
                                                   &old_ul_ingr_idx,
                                                   &old_ul_egr_idx);
    }

    return status;
}

/*
 * Function:
 *     _bcm_fe2000_l3_update_intf
 * Purpose:
 *     Update the HW for any changes in interface properties
 *     i.e. BCM_L3_REPLACE on an interface
 * Parameters:
 *     l3_fe    - (IN) l3 fe instance context
 *     intf     - (IN) user (bcm) interface data
 *     l3_intf  - (IN) local sw context
 * Returns:
 *     BCM_E_XXX
 * NOTE:
 *     Need to handle changes in any of the following
 *     l3a_vrf
 *     l3a_mac_addr
 *     l3a_tunnel_idx
 *     l3a_ttl
 *     l3a_mtu
 */
STATIC int
_bcm_fe2000_l3_update_intf(_fe2k_l3_fe_instance_t *l3_fe,
                           bcm_l3_intf_t          *bcm_intf,
                           _fe2k_l3_intf_t        *l3_intf)
{
    int                             status;
    _fe2k_l3_ete_t                 *l3_ete = NULL;
    uint32                          i, chg_flags;
    uint32                          hash_index;
    dq_p_t                          hash_bucket;
    uint32                          flags = 0;

    chg_flags = 0;
    status    = BCM_E_NONE;

    if (ENET_CMP_MACADDR(bcm_intf->l3a_mac_addr, 
                         l3_intf->if_info.l3a_mac_addr)) {
        chg_flags |= L3_INTF_MAC_CHANGED;
    }

    /*
     * It has already been checked that there are no ETEs on this
     */
    if (bcm_intf->l3a_vrf != l3_intf->if_info.l3a_vrf) {
        chg_flags |=   L3_INTF_VRF_CHANGED;
    }

    if (bcm_intf->l3a_mtu &&
        (bcm_intf->l3a_mtu != l3_intf->if_info.l3a_mtu)) {
        chg_flags |=  L3_INTF_MTU_CHANGED;
    }

    if (bcm_intf->l3a_vid &&
        (bcm_intf->l3a_vid != l3_intf->if_info.l3a_vid)) {
        chg_flags |=  L3_INTF_VID_CHANGED;
    }

    if (chg_flags & (L3_INTF_VID_CHANGED | L3_INTF_MAC_CHANGED)) {
        if (chg_flags & (L3_INTF_MAC_CHANGED)) {
            status = _bcm_fe2000_l3_handle_smac_change(l3_fe,
                                                       bcm_intf,
                                                       l3_intf);
        }
        if (status != BCM_E_NONE) {
            return status;
        }
        DQ_REMOVE(&l3_intf->if_vid_link);
    }

    /*
     * Copy over the new values
     */
    if (chg_flags & L3_INTF_VID_CHANGED) {
        l3_intf->if_info.l3a_vid = bcm_intf->l3a_vid;
    }
    if (chg_flags & L3_INTF_MAC_CHANGED) {
        ENET_COPY_MACADDR(bcm_intf->l3a_mac_addr,
                          l3_intf->if_info.l3a_mac_addr);
    }
    if (chg_flags & L3_INTF_MTU_CHANGED) {
        l3_intf->if_info.l3a_mtu = bcm_intf->l3a_mtu;
    }

    if (chg_flags & (L3_INTF_VRF_CHANGED)) {
        l3_intf->if_info.l3a_vrf = bcm_intf->l3a_vrf;
    }

    if (chg_flags & (L3_INTF_VID_CHANGED | L3_INTF_MAC_CHANGED)) {
        hash_index         =  _FE2K_GET_INTF_VID_HASH(l3_intf->if_info.l3a_vid);
        hash_bucket        =  &l3_fe->fe_intf_by_vid[hash_index];
        DQ_INSERT_HEAD(hash_bucket, &l3_intf->if_vid_link);
    }


    if (chg_flags &
        (L3_INTF_VID_CHANGED |
         L3_INTF_MAC_CHANGED |
         L3_INTF_MTU_CHANGED |
         L3_INTF_TTL_CHANGED)) {

        /**
         * update the shared L2Ete
         */
        for (i = 0; i < _FE2K_L3_ETE_VIDOP__MAX_TYPES; i++) {
            status = _bcm_fe2000_update_l2_ete(l3_fe,
                                               &l3_intf->if_l2_ete[i],
                                               l3_intf->if_egr_smac_idx,
                                               l3_intf->if_info.l3a_vid,
                                               l3_intf->if_info.l3a_mtu,
                                               chg_flags);
            if (status != BCM_E_NONE) {
                return status;
            }
        }

        /**
         * update the non-shared L2Ete that is on each L3Ete.
         * In case of vrf change we already made sure that there were
         * no l3 etes
         */
        _FE2K_ALL_L3ETE_ON_INTF(l3_intf, l3_ete) {
            if (!(_FE2K_L2_ETE_SHARED(l3_ete))) {
                status = _bcm_fe2000_update_l2_ete(l3_fe,
                                                   &l3_ete->l3_l2encap.ete_idx,
                                                   l3_intf->if_egr_smac_idx,
                                                   l3_intf->if_info.l3a_vid,
                                                   l3_intf->if_info.l3a_mtu,
                                                   chg_flags);
                if (status != BCM_E_NONE) {
                    return status;
                }
            }
        } _FE2K_ALL_L3ETE_ON_INTF_END(l3_intf, l3_ete);

    }
    /* Save special flags other than REPLACE/WITH_ID*/
    flags = bcm_intf->l3a_flags;
    flags &= ~(BCM_L3_REPLACE | BCM_L3_WITH_ID);
    if (flags != l3_intf->if_info.l3a_flags) {
        l3_intf->if_info.l3a_flags = flags;
    }

    return status;
}

/*
 * Function:
 *    _bcm_fe2000_l3_intf_delete_one
 * Purpose:
 *     Delete L3 interface based on intf context
 * Parameters:
 *     l3_fe      - (IN)  l3 fe instance
 *     l3_intf    -  (IN) l3 interface
 * Returns:
 *     BCM_E_XXX
 */
int
_bcm_fe2000_l3_intf_delete_one(_fe2k_l3_fe_instance_t     *l3_fe,
                               _fe2k_l3_intf_t            **pl3_intf)
{

    uint32                      ul_ingr_idx, ul_egr_idx;
    uint32                      i;
    int                         status;
    _fe2k_l3_intf_t            *l3_intf;

    l3_intf = *pl3_intf;
    status  = BCM_E_NONE;

    for (i = 0; i < _FE2K_INTF_L3ETE_HASH_SIZE; i++) {
        if (!(DQ_EMPTY(&l3_intf->if_ete_hash[i]))) {
            return BCM_E_BUSY;
        }
    }

    ul_ingr_idx = l3_intf->if_ingr_smac_idx;
    ul_egr_idx  = l3_intf->if_egr_smac_idx;

    if (l3_intf->if_flags & _FE2K_L3_INTF_SMAC_IN_TBL) {
        status = _bcm_fe2000_l3_update_smac_tables(l3_fe,
                                                   l3_intf->if_info.l3a_mac_addr,
                                                   L3_INTF_DEL_SMAC,
                                                   &ul_ingr_idx, &ul_egr_idx);
        if (status != BCM_E_NONE) {
            return status;
        }
        l3_intf->if_flags &= ~(_FE2K_L3_INTF_SMAC_IN_TBL);
    }

    _bcm_fe2000_l3_free_sw_intf(l3_fe,
                                l3_intf);

    l3_intf   = NULL;
    *pl3_intf = NULL;
    return status;
}

/*
 * Function:
 *     _bcm_fe2000_tunnel_initiator_check_and_get_intf
 * Purpose:
 *     basic validations and get of L3 intf for tunnel
 * Parameters:
 *     l3_fe          - (IN)  l3 fe instance context
 *     intf           - (IN)  bcm intf
 *     l3_intf        - (IN)  l3 interface context
 * Returns:
 *     BCM_E_XXX
 * NOTE:
 */
int
_bcm_fe2000_tunnel_initiator_check_and_get_intf(_fe2k_l3_fe_instance_t *l3_fe,
                                                bcm_l3_intf_t          *intf,
                                                _fe2k_l3_intf_t       **l3_intf)
{
    uint32                 l3a_intf_id;
    int                    status;

    status = BCM_E_NONE;

    if (intf->l3a_flags & BCM_L3_WITH_ID) {
        if (!_FE2K_L3_USER_IFID_VALID(intf->l3a_intf_id)) {
            return BCM_E_PARAM;
        }
    } else {
        if (!BCM_VLAN_VALID(intf->l3a_vid)) {
            return BCM_E_PARAM;
        }
        if (BCM_MAC_IS_MCAST(intf->l3a_mac_addr) ||
            BCM_MAC_IS_ZERO(intf->l3a_mac_addr)) {
            return BCM_E_PARAM;
        }
    }

    if (intf->l3a_flags & BCM_L3_WITH_ID) {
        l3a_intf_id = _FE2K_IFID_FROM_USER_HANDLE(intf->l3a_intf_id);

        status = _bcm_fe2000_l3_find_intf_by_ifid(l3_fe,
                                                  l3a_intf_id,
                                                  l3_intf);

    } else {
        status = _bcm_fe2000_l3_find_intf_by_vid_mac(l3_fe,
                                                     intf,
                                                     l3_intf);
    }

    return status;
}

/*
 * Function:
 *     _bcm_fe2000_l3_alloc_mpath_object
 * Purpose:
 *     allocate a multipath object and the base fte index
 * Parameters:
 *     l3_fe      - (IN)  l3 fe instance
 *     flags      - (IN)  BCM_L3_REPLACE, BCM_L3_WITH_ID
 *     crc_info   - (IN)  the sorted mapped(original) fte,
 *                        with their crc16
 *     ret_mp_info- (OUT) allocated mpath object
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 *     alloc ecmp_set_size number of FTE indices and
 *     an object to hold the unique set passed by user
 */
int
_bcm_fe2000_l3_alloc_mpath_object(_fe2k_l3_fe_instance_t    *l3_fe,
                                  uint32                     flags,
                                  _fe2k_l3_mpath_crc_info_t *crc_info,
                                  _fe2k_egr_mp_info_t       **ret_mp_info)
{
    int                         status = 0;
    uint32                      size, i;
    uint32                      fte[_FE2K_L3_ECMP_MAX];
    uint32                      hash_idx;
    uint32                      allocater_flags;
    uint32                     *mapped_fte;
    _fe2k_egr_mp_info_t        *mp_info;
    _fe2k_l3_fte_t             *fte_hash_elem, *tmp_elem;
    uint32                      fte_hash_idx;

    size = sizeof(_fe2k_l3_fte_t);
    fte_hash_elem = sal_alloc(sizeof(_fe2k_l3_fte_t), "FTE-Hash");
    if (fte_hash_elem == NULL) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "Could not allocate fte hash elem\n")));

        return BCM_E_MEMORY;
    }
    sal_memset(fte_hash_elem, 0, size);

    size    = sizeof(_fe2k_egr_mp_info_t) +
        ((sizeof(uint32)) * l3_fe->fe_mp_set_size);
    mp_info = (_fe2k_egr_mp_info_t *) sal_alloc(size, "mpath-info");
    if (mp_info == NULL) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "Error in allocating memory for multipath object")));
        sal_free(fte_hash_elem);
        return BCM_E_MEMORY;
    }
    sal_memset(mp_info, 0, size);

    mp_info->fte_crc   = crc_info->crc16;
    mp_info->num_valid = crc_info->crc_data.count;

    sal_memset(fte, 0, sizeof(uint32) * _FE2K_L3_ECMP_MAX);
    allocater_flags = _SBX_GU2_RES_FLAGS_CONTIGOUS;
    if (flags & BCM_L3_WITH_ID) {
        allocater_flags |= _SBX_GU2_RES_FLAGS_RESERVE;
        for (i = 0; i < l3_fe->fe_mp_set_size; i++) {
            fte[i] = crc_info->mpbase + i;
        }
    }
    status = _sbx_gu2_resource_alloc(l3_fe->fe_unit,
                                     SBX_GU2K_USR_RES_FTE_L3_MPATH,
                                     l3_fe->fe_mp_set_size,
                                     fte,
                                     allocater_flags);
    if (status != BCM_E_NONE) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "Error in allocating multipath resource: %s\n"),
                bcm_errmsg(status)));

        sal_free(mp_info);
        sal_free(fte_hash_elem);
        return status;
    }

    /*
     * As a check, make sure that this fte_idx already does not exist
     */
    status = _bcm_fe2000_l3_get_sw_fte(l3_fe,
                                       fte[0],
                                       &tmp_elem);
    if (status == BCM_E_NONE) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "FTE idx 0x%x already exists in hash\n"), fte[0]));
        sal_free(fte_hash_elem);
        sal_free(mp_info);
        status = _sbx_gu2_resource_free(l3_fe->fe_unit,
                                        SBX_GU2K_USR_RES_FTE_L3_MPATH,
                                        l3_fe->fe_mp_set_size,
                                        fte,
                                        _SBX_GU2_RES_FLAGS_CONTIGOUS);
        return BCM_E_EXISTS;
    }
    status = BCM_E_NONE;
    crc_info->mpbase = mp_info->mp_base = fte[0];

    mapped_fte = &mp_info->mapped_fte[0];
    for (i = 0; i < mp_info->num_valid; i++) {
        *mapped_fte = crc_info->crc_data.fte_indices[i];
        mapped_fte++;
    }

    fte_hash_elem->ref_count = 1;
    fte_hash_elem->fte_idx   = mp_info->mp_base;
    fte_hash_idx = _FE2K_GET_FTE_HASH_IDX(fte_hash_elem->fte_idx);
    DQ_INSERT_HEAD(&l3_fe->fe_fteidx2_fte[fte_hash_idx], &fte_hash_elem->fte_hash_link);


    hash_idx = _FE2K_GET_HASH_EGR_MP(mp_info->fte_crc);
    DQ_INSERT_HEAD(&l3_fe->fe_mp_by_egr[hash_idx], &mp_info->egr_mp_link);

    hash_idx = _FE2K_GET_HASH_MPBASE_MP(mp_info->mp_base);
    DQ_INSERT_HEAD(&l3_fe->fe_mp_by_mpbase[hash_idx], &mp_info->mpbase_link);

    L3_VERB((_SBX_D(l3_fe->fe_unit, "Successfully allocated multipath object"
                    "and Indices (%d - %d)\n"),
             mp_info->mp_base,
             mp_info->mp_base + l3_fe->fe_mp_set_size - 1));

    *ret_mp_info = mp_info;
    return status;
}

/*
 * Function:
 *     _bcm_fe2000_l3_destroy_mpath_object
 * Purpose:
 *     de-alloc the mpath object and free the fte indices
 * Parameters:
 *     l3_fe      - (IN)  l3 fe instance
 *     mp_info    - (IN)  mpath object to be freed
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 */
int
_bcm_fe2000_l3_destroy_mpath_object(_fe2k_l3_fe_instance_t    *l3_fe,
                                    _fe2k_egr_mp_info_t      **pmp_info)
{
    int                         status=BCM_E_NONE;
    uint32                      derived_fte;
    uint32                      ecmp_size, i;
    uint32                      fte[_FE2K_L3_ECMP_MAX];
    _fe2k_egr_mp_info_t        *mp_info;

    ecmp_size       = l3_fe->fe_mp_set_size;
    status          = BCM_E_NONE;
    mp_info         = *pmp_info;


    L3_VERB((_SBX_D(l3_fe->fe_unit, "Destroying mpath object with Index %d\n"),
             mp_info->mp_base));

    for (i = 0; i < ecmp_size; i++) {
        derived_fte        = mp_info->mp_base + i;
        fte[i]             = derived_fte;


        switch (SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            status = _bcm_fe2000_g2p3_fte_op(l3_fe, derived_fte, NULL,
                                             _FE2K_FT_OPCODE_CLR);
            break;
#endif
        default:
            L3_UKNOWN_UCODE_WARN(l3_fe->fe_unit);
            status = BCM_E_CONFIG;
        }

        if (status != BCM_E_NONE) {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "error(%s) invalidating mpath fte(0x%x)\n"),
                    bcm_errmsg(status), derived_fte));
            return status;
        }
    }

    DQ_REMOVE(&mp_info->egr_mp_link);
    DQ_REMOVE(&mp_info->mpbase_link);
    sal_free(mp_info);
    *pmp_info = NULL;

    status =  _bcm_fe2000_destroy_fte(l3_fe,
                                      L3_OR_MPLS_DESTROY_FTE__FTE_ONLY,
                                      mp_info->mp_base,
                                      0, /* mod */
                                      0  /* ohi */);
    if (status != BCM_E_NONE) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "Could not destroy fte 0x%x: %s\n"),
                mp_info->mp_base, bcm_errmsg(status)));
    }

    return status;
}

/*
 * Function:
 *     _bcm_fe2000_l3_set_mpath_ftes
 * Purpose:
 *     copy the contents of the original ftes to the
 *     derived ones and program HW
 * Parameters:
 *     l3_fe      - (IN)  l3 fe instance
 *     mp_info    - (IN)  mpath object to be programmed in HW
 *     crc_info   - (IN)  provides the fte contents of the base (mapped)
 *                        FTEs
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 */
int
_bcm_fe2000_l3_set_mpath_ftes(_fe2k_l3_fe_instance_t    *l3_fe,
                              _fe2k_egr_mp_info_t       *mp_info,
                              _fe2k_l3_mpath_crc_info_t *crc_info)
{
    int            status = 0;
    uint32         mapped_fte, derived_fte;
    uint32         ecmp_size, num_mapped_ftes, i, map_idx;

    ecmp_size       = l3_fe->fe_mp_set_size;
    num_mapped_ftes = mp_info->num_valid;
    L3_VERB((_SBX_D(l3_fe->fe_unit, "ecmpSize %d mappedFtes %d \n"),
             ecmp_size, num_mapped_ftes));

    /*
     * Get the mapped Ftes and copy them over
     */
    for (i = 0; i < ecmp_size; i++) {
        map_idx     = i % num_mapped_ftes;
        mapped_fte  = mp_info->mapped_fte[map_idx];
        derived_fte = mp_info->mp_base + i;

        L3_VERB((_SBX_D(l3_fe->fe_unit, "[Programming mpath ftes] "
                        "derived fte 0x%x mapped fte 0x%x)\n"),
                 (int) derived_fte, (int) mapped_fte));

        switch (SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            status = _bcm_fe2000_g2p3_fte_op(l3_fe, derived_fte,
                                             crc_info->fte_contents[map_idx],
                                             _FE2K_FT_OPCODE_SET);
            break;
#endif
        default:
            L3_UKNOWN_UCODE_WARN(l3_fe->fe_unit);
            status = BCM_E_CONFIG;
        }

        if (status != BCM_E_NONE) {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "Error in FTE programming: %s\n"
                           "mpbase 0x%x mapped_fte 0x%x derived_fte 0x%x\n"),
                    bcm_errmsg(status),
                    mp_info->mp_base, mapped_fte, derived_fte));
            return status;
        }
    }

    return status;
}

/*
 * Function:
 *     _bcm_fe2000_l3_save_fte_hash_elements
 * Purpose:
 *     save the pointers to fte_indices passed in
 *     Typically this will be used to incr/decr ref_counts
 *     if the underlying operation was successful
 * Parameters:
 *     l3_fe      - (IN)  l3 fe instance
 *     count      - (IN)  number of FTEs
 *     fte_indices- (IN)  array of indices for which we need to get hash elements
 *     elems      - (OUT) array of pointers to fte hash elements
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 */
int
_bcm_fe2000_l3_save_fte_hash_elements(_fe2k_l3_fe_instance_t    *l3_fe,
                                      int                        count,
                                      uint32                    *fte_indices,
                                      _fe2k_l3_fte_t           **elems)
{
    int                         i, status, fte_idx;

    for (i = 0; i < count; i++) {
        fte_idx = fte_indices[i];
        status = _bcm_fe2000_l3_get_sw_fte(l3_fe,
                                           fte_idx,
                                           &elems[i]);
        if (status != BCM_E_NONE) {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "fte index 0x%x not found in hash\n"), fte_idx));
            return status;
        }
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_fe2000_l3_gen_mpath_crc
 * Purpose:
 *     setup the ftes passed by the user in a cannonical form
 *     i.e sort them and compute crc. This form is used by most
 *     functions that operate on mpath objects
 * Parameters:
 *     flags      - (IN)  BCM_L3_REPLACE, BCM_L3_WITH_ID
 *     intf_count - (IN)  number of unique FTEs in mpath
 *     intf_array - (IN)  actual FTEs passed in by user
 *     crc_info   - (OUT) sorted FTEs with crc
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 *
 */
int
_bcm_fe2000_l3_gen_mpath_crc(_fe2k_l3_fe_instance_t    *l3_fe,
                             uint32                     mpbase,
                             uint32                     intf_count,
                             bcm_if_t                  *intf_array,
                             _fe2k_l3_mpath_crc_info_t *crc_info,
                             uint32                     opcode)
{
    uint32                    i, crc_len;
    int                       status = BCM_E_NONE;


    L3_VERB((_SBX_D(l3_fe->fe_unit, "Mpath base 0x%x, count %d\n"),
             mpbase, intf_count));

    status                   = BCM_E_NONE;
    crc_info->crc_data.count = intf_count;
    crc_info->mpbase         = mpbase;

    for (i = 0; i < intf_count; i++) {
        crc_info->crc_data.fte_indices[i] = intf_array[i];
        crc_info->fte_contents[i]         = NULL;
    }
    _shr_sort(&crc_info->crc_data.fte_indices[0],  intf_count, sizeof(uint32),
              _bcm_fe2000_l3_cmp_uint32);

    /*
     * get the FTE hash elements. This will be used later
     * to update ref_counts on the FTEs. This also checks that the
     * FTE is in our hash
     */
    status = _bcm_fe2000_l3_save_fte_hash_elements(l3_fe,
                                                   crc_info->crc_data.count,
                                                   crc_info->crc_data.fte_indices,
                                                   crc_info->fte_hash_elem);
    if (status != BCM_E_NONE) {
        return status;
    }

    /*
     * The extra integer is for the count field itself
     */
    crc_len     = sizeof(uint32) * (intf_count + 1);
    crc_info->crc16 = _shr_crc16(0, (unsigned char *) &crc_info->crc_data,
                                 crc_len);

    /*
     * Get FTE contents only after sorting
     */
    if (opcode == L3_CRC_INFO_FETCH_FTE_DATA) {

        for (i = 0; i < intf_count; i++) {

            switch (SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
            case SOC_SBX_UCODE_TYPE_G2P3:
                crc_info->fte_contents[i] = _bcm_fe2000_g2p3_alloc_fte(1);
                
                if (crc_info->fte_contents[i] == NULL) {
                    status = BCM_E_MEMORY;
                } else {
                    status = _bcm_fe2000_g2p3_fte_op(l3_fe,
                                                     crc_info->crc_data.fte_indices[i],
                                                     crc_info->fte_contents[i],
                                                     _FE2K_FT_OPCODE_GET);
                }
                break;
#endif
            default:
                L3_UKNOWN_UCODE_WARN(l3_fe->fe_unit);
                status = BCM_E_CONFIG;
            }

            if (status != BCM_E_NONE) {
                while(--i > 0){
                    sal_free(crc_info->fte_contents[i]);
                }

                L3_ERR((_SBX_D(l3_fe->fe_unit, "error %s in getting FTE (0x%x) from HW\n"),
                        bcm_errmsg(status), 
                        crc_info->crc_data.fte_indices[i]));
                return status;
            }
        }
    }

    return BCM_E_NONE;
}

int
_bcm_fe2000_l3_free_mpath_crc(_fe2k_l3_mpath_crc_info_t *crc_info,
                              uint32                     intf_count)
{
    int i;
    for (i = 0; i < intf_count; i++) {
        if(crc_info->fte_contents[i]==NULL) {
            sal_free(crc_info->fte_contents[i]);
        }
    }
    sal_free(crc_info);
    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_fe2000_save_old_mpath_state
 * Purpose:
 *     save old math info in case the REPLACE goes awry
 * Parameters:
 *     ol_mp_info - (IN)  the current mpath object
 *     crc_info   - (OUT) the mpath object saved in cannonical form
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 *
 */
int
_bcm_fe2000_save_old_mpath_state(_fe2k_l3_fe_instance_t     *l3_fe,
                                 _fe2k_egr_mp_info_t        *old_mp_info,
                                 _fe2k_l3_mpath_crc_info_t  *crc_info)
{
    int            i;
    int            status = BCM_E_NONE;

    crc_info->crc16          = old_mp_info->fte_crc;
    crc_info->flags          = 0;
    crc_info->mpbase         = old_mp_info->mp_base;
    crc_info->crc_data.count = old_mp_info->num_valid;

    for (i = 0; i < old_mp_info->num_valid; i++) {
        crc_info->crc_data.fte_indices[i] = old_mp_info->mapped_fte[i];

        switch (SOC_SBX_CONTROL(l3_fe->fe_unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            status = _bcm_fe2000_g2p3_fte_op(l3_fe,
                                             crc_info->crc_data.fte_indices[i],
                                             &(crc_info->fte_contents[i]),
                                             _FE2K_FT_OPCODE_GET);
            break;
#endif
        default:
            L3_UKNOWN_UCODE_WARN(l3_fe->fe_unit);
            status = BCM_E_CONFIG;
        }

        if (status != BCM_E_NONE) {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "error %s in getting FTE (0x%x) from HW"),
                    bcm_errmsg(status),
                    crc_info->crc_data.fte_indices[i]));
            return status;
        }
    }

    /*
     * get the FTE hash elements. This will be used later
     * to update ref_counts on the FTEs. This also checks that the
     * FTE is in our hash
     */
    status = _bcm_fe2000_l3_save_fte_hash_elements(l3_fe,
                                                   crc_info->crc_data.count,
                                                   crc_info->crc_data.fte_indices,
                                                   crc_info->fte_hash_elem);
    if (status != BCM_E_NONE) {
        return status;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_fe2000_l3_mpath_find
 * Purpose:
 *     Given the original set of FTEs find the base FTE of the
 * Parameters:
 *     l3_fe      - (IN)  l3 fe instance
 *     crc_info   - (IN)  the mpath object saved in cannonical form
 *     ret_info   - (OUT) mpath object if found
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 *
 */
int
_bcm_fe2000_l3_mpath_find(_fe2k_l3_fe_instance_t     *l3_fe,
                          _fe2k_l3_mpath_crc_info_t  *crc_info,
                          _fe2k_egr_mp_info_t        **ret_info)
{
    uint32               hash_idx;
    dq_p_t               mp_by_egr_head, mp_by_egr_elem;
    _fe2k_egr_mp_info_t *mp_info = NULL;

    hash_idx =  _FE2K_GET_HASH_EGR_MP(crc_info->crc16);
    mp_by_egr_head = &l3_fe->fe_mp_by_egr[hash_idx];
    DQ_TRAVERSE(mp_by_egr_head, mp_by_egr_elem) {
        _FE2K_MP_INFO_FROM_EGR_MP_DQ(mp_by_egr_elem, mp_info);
        if (mp_info->fte_crc == crc_info->crc16) {
            /* both are sorted, hence should be identical */
            if ((mp_info->num_valid == crc_info->crc_data.count) &&
                !sal_memcmp(mp_info->mapped_fte,
                             crc_info->crc_data.fte_indices,
                             sizeof(uint32)*mp_info->num_valid)) {
                *ret_info = mp_info;
                return BCM_E_NONE;
            }
        }
    } DQ_TRAVERSE_END(mp_by_egr_head, mp_by_egr_elem);

    return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *     _bcm_fe2000_l3_mpath_get
 * Purpose:
 *     Given the base FTE find the mpath object and
 *     hence the original set of FTEs.
 * Parameters:
 *     l3_fe      - (IN)  l3 fe instance
 *     mpbase     - (IN)  mpath base FTE for the mpath group
 *     ret_info   - (OUT) mpath object if found
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 *
 */
STATIC int
_bcm_fe2000_l3_mpath_get(_fe2k_l3_fe_instance_t *l3_fe,
                         uint32                 mpbase,
                         _fe2k_egr_mp_info_t    **ret_info)
{
    uint32                hash_idx;
    dq_p_t                mp_by_mpbase_head, mp_by_mpbase_elem;
    _fe2k_egr_mp_info_t  *mp_info = NULL;

    L3_VERB((_SBX_D(l3_fe->fe_unit, "get mpath object with base FTE %d\n"), mpbase));

    hash_idx =  _FE2K_GET_HASH_MPBASE_MP(mpbase);
    mp_by_mpbase_head = &l3_fe->fe_mp_by_mpbase[hash_idx];
    DQ_TRAVERSE(mp_by_mpbase_head, mp_by_mpbase_elem) {
        _FE2K_MP_INFO_FROM_MPBASE_MP_DQ(mp_by_mpbase_elem, mp_info);
        if (mp_info->mp_base == mpbase) {
            *ret_info = mp_info;
            return BCM_E_NONE;
        }
    } DQ_TRAVERSE_END(mp_by_mpbase_head, mp_by_mpbase_elem);

    return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *     _bcm_fe2000_l3_add_mpath_object
 * Purpose:
 *     Add a new mpath object in SW and HW
 * Parameters:
 *     l3_fe      - (IN)  l3 fe instance
 *     flags      - (IN)  BCM_L3_REPLACE, BCM_L3_WITH_ID
 *     crc_info   - (IN)  the mpath object saved in cannonical form
 *     ret_info   - (OUT) mpath object if created
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 *    It is assumed that the caller has done a find and
 *    made sure that it does not exist
 */
int
_bcm_fe2000_l3_add_mpath_object(_fe2k_l3_fe_instance_t    *l3_fe,
                                uint32                     flags,
                                _fe2k_l3_mpath_crc_info_t *crc_info,
                                _fe2k_egr_mp_info_t       **ret_mp_info)
{
    int                         i, status;
    _fe2k_egr_mp_info_t        *mp_info;

    L3_VVERB((_SBX_D(l3_fe->fe_unit, "Enter\n")));

    status = _bcm_fe2000_l3_alloc_mpath_object(l3_fe,
                                               flags,
                                               crc_info,
                                               &mp_info);
    if (status != BCM_E_NONE) {
        return status;
    }
    status = _bcm_fe2000_l3_set_mpath_ftes(l3_fe,
                                           mp_info,
                                           crc_info);
    if (status != BCM_E_NONE) {
        _bcm_fe2000_l3_destroy_mpath_object(l3_fe,
                                            &mp_info);
        mp_info = NULL;
    }

    /*
     * everything went well. Increment the ref_counts for all the
     * mapped ftes.
     */
    for (i = 0; i < crc_info->crc_data.count; i++) {
        (crc_info->fte_hash_elem[i])->ref_count++;
    }

    *ret_mp_info = mp_info;
    return status;
}

/*
 * Function:
 *     _bcm_fe2000_l3_update_mpath_object
 * Purpose:
 *     update mpath object in SW and HW
 * Parameters:
 *     l3_fe      - (IN)  l3 fe instance
 *     crc_info   - (IN)  the new mpath object saved in cannonical form
 *     mp_info    - (IN/OUT) mpath object to be updated
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 */
int
_bcm_fe2000_l3_update_mpath_object(_fe2k_l3_fe_instance_t    *l3_fe,
                                   _fe2k_l3_mpath_crc_info_t *crc_info,
                                   _fe2k_egr_mp_info_t       *mp_info)
{
    int                         status;
    int                         i;
    uint32                     *mapped_fte;
    uint32                      hash_idx;

    L3_VERB((_SBX_D(l3_fe->fe_unit, "Enter\n")));

    /*
     * copy over the new sorted FTEs, their count and CRC
     * into the mpath object. The mpbase does not change
     * in case of update
     */
    mapped_fte = &mp_info->mapped_fte[0];
    for (i = 0; i < crc_info->crc_data.count; i++) {
        *mapped_fte = crc_info->crc_data.fte_indices[i];
        mapped_fte++;
    }
    mp_info->num_valid = crc_info->crc_data.count;
    mp_info->fte_crc   = crc_info->crc16;

    status = _bcm_fe2000_l3_set_mpath_ftes(l3_fe,
                                           mp_info,
                                           crc_info);
    if (status != BCM_E_NONE) {
        /*
         * caller will try to revert back to old-working fte mpath
         * if required
         */
        L3_ERR((_SBX_D(l3_fe->fe_unit, "error in setting mpath object 0x%x: %s\n"),
                mp_info->mp_base, bcm_errmsg(status)));
        return status;
    }

    /*
     * Since the crc may have changed, remove from old hash bkt
     * and insert into new hash bkt
     */
    DQ_REMOVE(&mp_info->egr_mp_link);
    hash_idx =  _FE2K_GET_HASH_EGR_MP(mp_info->fte_crc);
    DQ_INSERT_HEAD(&l3_fe->fe_mp_by_egr[hash_idx],
                   &mp_info->egr_mp_link);

    return status;
}

/*
 * Function:
 *     _bcm_fe2000_handle_mpath_update
 * Purpose:
 *     Update a existing mpath object
 * Parameters:
 *     l3_fe          - (IN)  l3 fe instance
 *     mp_info        - (IN)  the existing mpath object
 *     new_intf_count - (IN)  the new number of contributing FTEs
 *     new_ftes       - (IN)  the actual indice of  contributing FTEs
 * Returns:
 *     BCM_E_XXX
 *
 * NOTE:
 *     This does the initial setup and calls the real update
 *     function.  Also handles roll-back
 */
int
_bcm_fe2000_handle_mpath_update(_fe2k_l3_fe_instance_t  *l3_fe,
                                _fe2k_egr_mp_info_t     *mp_info,
                                uint32                   new_intf_count,
                                bcm_if_t                *new_ftes)
{

    _fe2k_l3_mpath_crc_info_t    *crc_info, *old_crc_info;
    int                          status, ignore_status, size, i;

    status   = ignore_status = BCM_E_NONE;
    size     = sizeof(_fe2k_l3_mpath_crc_info_t) * 2;
    crc_info = sal_alloc(size, "mpath-crc-info");
    if (crc_info == NULL) {
        return BCM_E_MEMORY;
    }
    sal_memset(crc_info, 0, size);

    old_crc_info = crc_info + 1;

    /*
     * save the old info
     */
    status = _bcm_fe2000_save_old_mpath_state(l3_fe,
                                              mp_info,
                                              old_crc_info);
    if (status != BCM_E_NONE) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "could not retrieve existing FTEs: %s\n"), 
                bcm_errmsg(status)));
        sal_free(crc_info);
        crc_info = old_crc_info = NULL;
        return status;
    }

    /*
     * fill up the new FTEs in sorted order into crc_info
     */
    status = _bcm_fe2000_l3_gen_mpath_crc(l3_fe,
                                          mp_info->mp_base,
                                          new_intf_count,
                                          new_ftes,
                                          crc_info,
                                          L3_CRC_INFO_FETCH_FTE_DATA);
    if (status != BCM_E_NONE) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "Error in setting up mpath Info: %s\n"),
                bcm_errmsg(status)));
        sal_free(crc_info);
        crc_info = old_crc_info = NULL;
        return status;
    }

    status = _bcm_fe2000_l3_update_mpath_object(l3_fe,
                                                crc_info,
                                                mp_info);
    if (status != BCM_E_NONE) {
        ignore_status =
            _bcm_fe2000_l3_update_mpath_object(l3_fe,
                                               old_crc_info,
                                               mp_info);
        if (ignore_status != BCM_E_NONE) {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "mpbase(0x%x) neither set"
                           " to new nor could be reverted back to "
                           "old set of ftes\n"),
                    mp_info->mp_base));

        }

        sal_free(crc_info);
        crc_info = old_crc_info = NULL;
        return status;
    }

    /*
     * everything went well. Increment the ref_counts for all the
     * new mapped ftes and decrement the ref_count for the old ones
     * Note that there may be some ftes that are common to both the
     * sets. For those an increment followed by a decrement will
     * cancel out
     */
    for (i = 0; i < crc_info->crc_data.count; i++) {
        (crc_info->fte_hash_elem[i])->ref_count++;
    }

    for (i = 0; i < old_crc_info->crc_data.count; i++) {
        (old_crc_info->fte_hash_elem[i])->ref_count--;
    }

    if (crc_info) {
        sal_free(crc_info);
        crc_info = old_crc_info = NULL;
    }

    return BCM_E_NONE;
}

int
_bcm_fe2000_l3_modid_set(int unit, int modid)
{
    _fe2k_l3_fe_instance_t      *l3_fe;

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_UNIT;
    }

    l3_fe->fe_my_modid = modid;

    L3_UNLOCK(unit);

    return BCM_E_NONE;
}

/************************ ONLY BCM FUNCTIONS BELOW THIS LINE  ************/

/*
 * Function:
 *     bcm_fe2000_l3_init
 * Purpose:
 *     Initialize L3 structures
 * Parameters:
 *     unit  - (IN) FE2K unit number
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      There is ordering restriction
 *      bcm_l3_init();
 *      bcm_mpls_init();
 *      ...
 *      ...
 *      bcm_mpls_cleanup();
 *      bcm_l3_cleanup();
 */

int
bcm_fe2000_l3_init(int unit)
{
    int         status;
    soc_sbx_g2p3_v4uc_str_sel_t soc_sbx_g2p3_v4uc_str_sel;

    if (L3_UNIT_INVALID(unit)) {
        return BCM_E_UNIT;
    }

    status = _bcm_fe2000_l3_sw_init(unit);
    if (status != BCM_E_NONE) {
        L3_ERR((_SBX_D(unit, "error(%s)\n"), bcm_errmsg(status)));
        return status;
    }

    /* Disable IPv4 caching */
    SOC_SBX_STATE(unit)->cache_l3host = FALSE;
    SOC_SBX_STATE(unit)->cache_l3route = FALSE;

    if (SOC_SBX_V4UC_STR_SEL(unit)) {
        soc_sbx_g2p3_v4uc_str_sel.valid = 0;
        soc_sbx_g2p3_v4uc_str_sel_set(unit, V4UC_STR_SEL_RULE, &soc_sbx_g2p3_v4uc_str_sel);
    }

    L3_VERB((_SBX_D(unit, "completed init successfully\n")));

    return BCM_E_NONE;
}

/*
 * Function:
 *     bcm_fe2000_l3_intf_create
 * Purpose:
 *     Create an L3 interface
 * Parameters:
 *     unit  - (IN) FE2K unit number
 *     intf  - (IN) interface info:
 *             (IN) l3a_mac_addr - MAC address;
 *             (IN) l3a_vid - VLAN ID;
 *             (IN) flag BCM_L3_WITH_ID: use specified interface ID l3a_intf.
 *                  flag BCM_L3_REPLACE: overwrite if interface already exists.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      1. The L3 interface ID is automatically assigned unless a specific
 *         ID is requested with the BCM_L3_WITH_ID flag.
 *      2. Create a interface instance and insert it into the hash tables
 *         (<ifid> hash and <vid, mac> hash.
 *      3. Add the mac address to the localStation match table and Smac2Idx table.
 */
int
bcm_fe2000_l3_intf_create(int            unit,
                          bcm_l3_intf_t *bcm_intf)
{
    _fe2k_l3_fe_instance_t     *l3_fe;
    _fe2k_l3_intf_t            *l3_intf;
    int                         status;

    if (bcm_intf == NULL) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
       L3_UNLOCK(unit);
       return  BCM_E_UNIT;
    }

    status = _bcm_fe2000_l3_sw_intf_create_checks(l3_fe,
                                                  bcm_intf);

    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }

    if (bcm_intf->l3a_flags &  BCM_L3_WITH_ID) {
        bcm_intf->l3a_intf_id = _FE2K_IFID_FROM_USER_HANDLE(bcm_intf->l3a_intf_id);
    }

    status = _bcm_fe2000_l3_intf_create_replace(unit,
                                                l3_fe,
                                                bcm_intf,
                                                &l3_intf);

#ifdef BCM_WARM_BOOT_SUPPORT
    if (BCM_SUCCESS(status)) {
        status = l3_wb_store_intf(l3_fe, l3_intf);
    }
#endif /* BCM_WARM_BOOT_SUPPORT */   

    /*
     * At this time, if things have gone well the intf_id is the
     * internal number. Convert it back to user handle
     */
    if ((status == BCM_E_NONE) ||
        (bcm_intf->l3a_flags &  BCM_L3_WITH_ID)) {
        bcm_intf->l3a_intf_id = _FE2K_USER_HANDLE_FROM_IFID(bcm_intf->l3a_intf_id);
    }

    L3_UNLOCK(unit);
    return status;
}


/*
 * Function:
 *     bcm_fe2000_l3_intf_delete
 * Purpose:
 *     Delete L3 interface based on L3 interface ID
 * Parameters:
 *     unit  - (IN) Fe2000 unit number
 *     intf  - (IN) interface structure with L3 interface ID as input
 *
 * Returns:
 *     BCM_E_XXX
 *
 * Assumptions:
 *     The multicast egress object is  not in use.
 *     The HW ete will be deprogrammed and SW structures
 *     will be freed.
 *
 * Notes:
 *     If there is any non ipmcast etes dependent on this intf, then
 *     return EBUSY.
 *
 */
int
bcm_fe2000_l3_intf_delete(int unit,
                          bcm_l3_intf_t *bcm_intf)
{
    _fe2k_l3_fe_instance_t     *l3_fe;
    _fe2k_l3_intf_t            *l3_intf;
    int                         status;

    if (NULL == bcm_intf) {
        return (BCM_E_PARAM);
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
       L3_UNLOCK(unit);
       return  BCM_E_UNIT;
    }

    status = BCM_E_PARAM;
    if (bcm_intf->l3a_flags & BCM_L3_WITH_ID) {
        bcm_intf->l3a_intf_id = _FE2K_IFID_FROM_USER_HANDLE(bcm_intf->l3a_intf_id);
        if (_FE2K_IFID_VALID_RANGE(bcm_intf->l3a_intf_id)) {
            status = _bcm_fe2000_l3_find_intf_by_ifid(l3_fe,
                                                      bcm_intf->l3a_intf_id,
                                                      &l3_intf);
        }
    } else {
        if (BCM_VLAN_VALID(bcm_intf->l3a_vid)) {
            status = _bcm_fe2000_l3_find_intf_by_vid_mac(l3_fe,
                                                         bcm_intf,
                                                         &l3_intf);
        }
    }

    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }

    status = _bcm_fe2000_l3_intf_delete_one(l3_fe,
                                            &l3_intf);
    if (bcm_intf->l3a_flags & BCM_L3_WITH_ID) {
        bcm_intf->l3a_intf_id = _FE2K_USER_HANDLE_FROM_IFID(bcm_intf->l3a_intf_id);
    }

    L3_UNLOCK(unit);
    return BCM_E_NONE;
}

/*
 * Function:
 *     bcm_fe2000_l3_intf_find
 * Purpose:
 *     Find the L3 intf number based on (MAC, VLAN)
 * Parameters:
 *     unit  - (IN) FE2000 unit number
 *     intf  - (IN) interface (MAC, VLAN),
 *             (OUT)intf number
 * Returns:
 *     BCM_E_XXX
 */
int
bcm_fe2000_l3_intf_find(int unit,
                        bcm_l3_intf_t *intf)
{

    int status;

    /*
     * WITH_ID should not be specified and VID,MAC needs to be valid
     */
    if ((intf->l3a_flags & BCM_L3_WITH_ID)         ||

        (BCM_MAC_IS_MCAST(intf->l3a_mac_addr))     ||
        (BCM_MAC_IS_ZERO(intf->l3a_mac_addr))      ||

        (intf->l3a_vid && (!BCM_VLAN_VALID(intf->l3a_vid)))) {
        return (BCM_E_PARAM);
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));


    status = _bcm_fe2000_l3_intf_locate(unit,
                                        intf,
                                        L3_INTF_LOCATE_BY_VID);
    L3_UNLOCK(unit);

    return status;

}

/*
 * Function:
 *     bcm_fe2000_l3_intf_get
 * Purpose:
 *     Given the L3 interface id, return the MAC and VLAN
 * Parameters:
 *     unit  - (IN) FE2000 unit number
 *     intf  - (IN) L3 interface;
 *             (OUT)VLAN ID, 802.3 MAC for this L3 intf
 * Returns:
 *     BCM_E_XXX
 */
int
bcm_fe2000_l3_intf_get(int unit,
                    bcm_l3_intf_t *intf)
{
    int       status;
    uint32    ifid;
    uint32    flags;

    if (!(intf->l3a_flags & BCM_L3_WITH_ID)) {
        L3_VERB((_SBX_D(unit, "l3a_flags(0x%x) does not have BCM_L3_WITH_ID\n"),
                 intf->l3a_flags));
        return BCM_E_PARAM;
    }

    if (!_FE2K_L3_USER_IFID_VALID(intf->l3a_intf_id)) {
        L3_VERB((_SBX_D(unit, "l3a_intf_id(0x%x) is invalid\n"), intf->l3a_intf_id));
        return BCM_E_PARAM;
    }

    ifid =  _FE2K_IFID_FROM_USER_HANDLE(intf->l3a_intf_id);
    if (!_FE2K_IFID_VALID_RANGE(ifid)) {
        L3_VERB((_SBX_D(unit, "internal l3a_intf_id(0x%x) is invalid\n"), ifid));
        return BCM_E_PARAM;
    }

    flags             = intf->l3a_flags;
    bcm_l3_intf_t_init(intf);
    intf->l3a_intf_id = ifid;
    intf->l3a_flags   = flags;

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    status = _bcm_fe2000_l3_intf_locate(unit,
                                        intf,
                                        L3_INTF_LOCATE_BY_IFID);

    intf->l3a_intf_id =  _FE2K_USER_HANDLE_FROM_IFID(ifid);

    L3_UNLOCK(unit);

    return status;


}
/*
 * Function:
 *     bcm_fe2000_l3_intf_find_vlan
 * Purpose:
 *     Given a vid, return the first interface that matches
 *
 * Parameters:
 *     unit  - (IN) FE2000 unit number
 *     intf  - (IN/OUT) BCM L3 interface;
 *                     (IN)VLAN ID
 * Returns:
 *     BCM_E_XXX
 * Note:
 *     return first intf based on vid
 *     Can return different values at diff times
 */
int
bcm_fe2000_l3_intf_find_vlan(int unit,
                          bcm_l3_intf_t *intf)
{
    _fe2k_l3_fe_instance_t     *l3_fe;
    _fe2k_l3_intf_t            *l3_intf = NULL;
    dq_p_t                      l3_intf_elem;
    uint32                      hash_index;

    if (!BCM_VLAN_VALID(intf->l3a_vid)) {
        return (BCM_E_PARAM);
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
       L3_UNLOCK(unit);
       return  BCM_E_UNIT;
    }

    hash_index   =  _FE2K_GET_INTF_VID_HASH(intf->l3a_vid);
    if (DQ_EMPTY(&l3_fe->fe_intf_by_vid[hash_index])) {
        L3_UNLOCK(unit);
        return  BCM_E_NOT_FOUND;
    }
    l3_intf_elem = DQ_HEAD(&l3_fe->fe_intf_by_vid[hash_index], dq_p_t);
    _FE2K_L3INTF_FROM_IFID_DQ(l3_intf_elem, l3_intf);

    *intf = l3_intf->if_info;

    L3_UNLOCK(unit);
    return BCM_E_NONE;
}


int
bcm_fe2000_l3_intf_delete_all(int unit)
{
    _fe2k_l3_fe_instance_t     *l3_fe;
    _fe2k_l3_intf_t            *l3_intf = NULL;
    dq_p_t                      l3_intf_elem;
    int                         i, status, rv;

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
       L3_UNLOCK(unit);
       return  BCM_E_UNIT;
    }

    rv = status = BCM_E_NONE;
    for (i = 0; i < _FE2K_INTF_ID_HASH_SIZE; i++) {
        while (!(DQ_EMPTY(&l3_fe->fe_intf_by_id[i]))) {
            l3_intf_elem = DQ_HEAD(&l3_fe->fe_intf_by_id[i], dq_p_t);
            _FE2K_L3INTF_FROM_IFID_DQ(l3_intf_elem, l3_intf);
            rv = _bcm_fe2000_l3_intf_delete_one(l3_fe,
                                                &l3_intf);
            if (rv != BCM_E_NONE) {
                L3_ERR((_SBX_D(l3_fe->fe_unit, "error in interface delete: %s\n"),
                        bcm_errmsg(rv)));
                status = rv;
            }
        }
    }


    L3_UNLOCK(unit);
    return status;
}


int
bcm_fe2000_l3_interface_create(int unit,
                            bcm_mac_t mac_addr,
                            bcm_vlan_t vid,
                            int add_to_arl)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_l3_interface_id_create(int unit,
                               bcm_mac_t mac_addr,
                               bcm_vlan_t vid,
                               int intf_idx,
                               int add_to_arl)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_l3_interface_id_update(int unit,
                               bcm_mac_t mac_addr,
                               bcm_vlan_t vid,
                               int intf_idx,
                               int add_to_arl)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_l3_interface_lookup(int unit,
                            bcm_mac_t mac_addr,
                            bcm_vlan_t vid,
                            int *intf_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_l3_interface_destroy(int unit,
                             int intf_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_l3_interface_find(int unit,
                          int intf_id,
                          bcm_vlan_t *vid,
                          bcm_mac_t mac_addr)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_l3_interface_find_by_vlan(int unit,
                                  bcm_vlan_t vid,
                                  int *intf_id)
{
    return BCM_E_UNAVAIL;
}


int
bcm_fe2000_l3_host_find(int unit,
                     bcm_l3_host_t *info)
{
    bcm_l3_route_t  route_info;

    bcm_l3_route_t_init(&route_info);
    _FE2K_MAP_HOST_INFO_TO_ROUTE_INFO(info, &route_info);
    return (bcm_l3_route_get(unit, &route_info));
}

int
bcm_fe2000_l3_ip_find(int unit,
                   bcm_l3_host_t *info)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_l3_ip_find_index(int unit,
                         int index,
                         bcm_l3_host_t *info)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_l3_host_add(int unit,
                    bcm_l3_host_t *info)
{
    bcm_l3_route_t  route_info;

    bcm_l3_route_t_init(&route_info);
    _FE2K_MAP_HOST_INFO_TO_ROUTE_INFO(info, &route_info);
    return (bcm_l3_route_add(unit, &route_info));
}

int
bcm_fe2000_l3_ip_add(int unit,
                  bcm_l3_host_t *info)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_l3_host_delete(int unit,
                       bcm_l3_host_t *ip_addr)
{
    bcm_l3_route_t  route_info;

    bcm_l3_route_t_init(&route_info);
    _FE2K_MAP_HOST_INFO_TO_ROUTE_INFO(ip_addr, &route_info);
    return (bcm_l3_route_delete(unit, &route_info));
}

int
bcm_fe2000_l3_ip_delete(int unit,
                     ip_addr_t ip_addr)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_l3_host_delete_by_network(int unit,
                                  bcm_l3_route_t *net_addr)
{
    return (bcm_l3_route_delete(unit, net_addr));
}

int
bcm_fe2000_l3_ip_delete_by_prefix(int unit,
                               ip_addr_t ip_addr,
                               ip_addr_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_l3_host_delete_by_interface(int unit,
                                    bcm_l3_host_t *info)
{
    bcm_l3_route_t  route_info;

    bcm_l3_route_t_init(&route_info);
    _FE2K_MAP_HOST_INFO_TO_ROUTE_INFO(info, &route_info);
    return (bcm_l3_route_delete_by_interface(unit, &route_info));
}

int
bcm_fe2000_l3_ip_delete_by_interface(int unit,
                                  int intf)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_l3_host_delete_all(int unit,
                           bcm_l3_host_t *info)
{
    bcm_l3_route_t  route_info;

    bcm_l3_route_t_init(&route_info);
    _FE2K_MAP_HOST_INFO_TO_ROUTE_INFO(info, &route_info);
    return (bcm_l3_route_delete_by_interface(unit, &route_info));
}

int
bcm_fe2000_l3_ip_delete_all(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_l3_ip_update_entry_by_key(int unit,
                                  bcm_l3_host_t *info)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_l3_host_conflict_get(int unit,
                             bcm_l3_key_t *ipkey,
                             bcm_l3_key_t *cf_array,
                             int cf_max,
                             int *cf_count)
{
    return BCM_E_UNAVAIL;
}


int
bcm_fe2000_l3_host_age(int unit,
                    uint32 flags,
                    bcm_l3_host_traverse_cb age_cb)
{
    return BCM_E_UNAVAIL;
}


int
bcm_fe2000_l3_host_traverse(int unit,
                         uint32 flags,
                         uint32 start,
                         uint32 end,
                         bcm_l3_host_traverse_cb cb,
                         void *user_data)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_l3_age(int unit,
               uint32 flags,
               bcm_l3_age_cb age_out)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_l3_conflict_get(int unit,
                        bcm_l3_key_t *ipkey,
                        bcm_l3_key_t *cf_array,
                        int cf_max,
                        int *cf_count)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_l3_host_invalidate_entry(int unit,
                                 bcm_ip_t ipaddr)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_l3_host_validate_entry(int unit,
                               bcm_ip_t ipaddr)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_l3_invalidate_entry(int unit,
                            bcm_ip_t ipaddr)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_l3_validate_entry(int unit,
                          bcm_ip_t ipaddr)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_fe2000_l3_route_add
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
bcm_fe2000_l3_route_add(int unit,
                        bcm_l3_route_t *info)
{
    int                      status=BCM_E_NONE;
    _fe2k_l3_fe_instance_t  *l3_fe;
    bcm_l3_egress_t          bcm_egr;
    uint32                   vid, fte_idx;
    _fe2k_egr_mp_info_t     *mp_info;
    uint32                   ecmp_size;

    L3_VVERB((_SBX_D(unit, "Enter fteHandle 0x%x\n"), info->l3a_intf));

    if (info->l3a_flags & ~L3_ROUTE_SUPPORTED_FLAGS) {
        soc_cm_print("%x/%x failed", info->l3a_subnet, info->l3a_ip_mask);
        return BCM_E_PARAM;
    }

    if ((info->l3a_flags & BCM_L3_IP6) && info->l3a_vrf) {
        L3_ERR((_SBX_D(unit, "VRFs not supported on IPV6 routes\n")));
        return BCM_E_PARAM;
    }

    if (!_bcm_fe2000_ip_is_valid(info)) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
       L3_UNLOCK(unit);
       return  BCM_E_UNIT;
    }

    if (!_BCM_VRF_VALID(info->l3a_vrf)) {
        L3_ERR((_SBX_D(unit, "invalid vrf(0x%x)\n"), info->l3a_vrf));
        return BCM_E_PARAM;
    }

    if (!(_FE2K_L3_FTE_VALID(l3_fe->fe_unit, info->l3a_intf))) {
        L3_ERR((_SBX_D(unit, "invalid l3 interface: 0x%08x\n"), info->l3a_intf));
        L3_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    if (l3_fe->fe_vpn_by_vrf[info->l3a_vrf] == NULL) {
        L3_ERR((_SBX_D(unit, "vrf(0x%x) not yet created\n"), info->l3a_vrf));
        L3_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    vid     = 0;
    fte_idx =  _FE2K_GET_FTE_IDX_FROM_USER_HANDLE(info->l3a_intf);

    ecmp_size = 1;
    if (info->l3a_flags & BCM_L3_MULTIPATH) {
        /*
         *  if flags specify ecmp, make sure that the FTE is indeed mpath
         */
        status = _bcm_fe2000_l3_mpath_get(l3_fe,
                                          fte_idx,
                                          &mp_info);
        if (status != BCM_E_NONE) {
            L3_UNLOCK(unit);
            return  BCM_E_PARAM;
        }
        ecmp_size = mp_info->num_valid;
    }

    /*
     * In case of RPF, we need to get the VID.
     * Therefore we need to get the info from the egress
     * XXX: Do this to get RpfUnion
     */
    bcm_l3_egress_t_init(&bcm_egr);

    status = bcm_fe2000_g2p3_l3_route_add(l3_fe, info, vid, ecmp_size);

    

    /*
     * restore the user fte handle
     */
    info->l3a_intf =  _FE2K_GET_USER_HANDLE_FROM_FTE_IDX(fte_idx);
    L3_UNLOCK(unit);

    return status;
}

/*
 * Function:
 *      bcm_fe2000_l3_route_delete
 * Purpose:
 *      Delete an entry from the Default IP Routing table.
 * Parameters:
 *      unit  - (IN) FE2000 unit number
 *      info  - Pointer to bcm_l3_route_t structure with valid IP subnet & mask.
 * Returns:
 *      BCM_E_XXX.
 * Notes:
 *      FTE is not removed
 */
int
bcm_fe2000_l3_route_delete(int unit, bcm_l3_route_t *info)
{
    int                     status = 0;
    _fe2k_l3_fe_instance_t *l3_fe;

    if (!_bcm_fe2000_ip_is_valid(info)) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
       L3_UNLOCK(unit);
       return  BCM_E_UNIT;
    }

    if (!_BCM_VRF_VALID(info->l3a_vrf)) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "invalid vrf(0x%x)\n"), info->l3a_vrf));
        return BCM_E_PARAM;
    }

    if (l3_fe->fe_vpn_by_vrf[info->l3a_vrf] == NULL) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "vrf(0x%x) not yet created\n"), info->l3a_vrf));
        L3_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    switch(SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        status = _bcm_fe2000_g2p3_ip_route_delete(l3_fe, info);
        break;
#endif
    default:
        L3_UKNOWN_UCODE_WARN(l3_fe->fe_unit);
        status = BCM_E_CONFIG;
    }

    L3_UNLOCK(unit);
    return status;
}

/*
 * XXX: call route add
 */
int
bcm_fe2000_l3_defip_add(int unit,
                     bcm_l3_route_t *info)
{
    return BCM_E_UNAVAIL;
}

/*
 * XXXX: call route del
 */
int
bcm_fe2000_l3_defip_delete(int unit,
                           bcm_l3_route_t *info)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_l3_route_get(int unit,
                        bcm_l3_route_t *info)
{
    int                     status=BCM_E_NONE;
    _fe2k_l3_fe_instance_t *l3_fe;
    bcm_l3_egress_t         bcm_egr;
    _fe2k_g2p3_da_route_t   junkDa;
    _fe2k_g2p3_sa_route_t   junkSa;
    

    if (!_bcm_fe2000_ip_is_valid(info)) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
       L3_UNLOCK(unit);
       return  BCM_E_UNIT;
    }

    if (!_BCM_VRF_VALID(info->l3a_vrf)) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "invalid vrf(0x%x)\n"), info->l3a_vrf));
        return BCM_E_PARAM;
    }

    if (l3_fe->fe_vpn_by_vrf[info->l3a_vrf] == NULL) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "vrf(0x%x) not yet created\n"), info->l3a_vrf));
        L3_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    /*
     * Tgid filled directly into info
     */
    switch(SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        status = _bcm_fe2000_g2p3_ip_da_route_get(l3_fe, info, &junkDa);
        
        if (BCM_SUCCESS(status) && (info->l3a_flags & BCM_L3_RPF)) {
            status = _bcm_fe2000_g2p3_ip_sa_route_get(l3_fe, info, &junkSa);
        }
        break;
#endif
    default: 
        L3_UKNOWN_UCODE_WARN(l3_fe->fe_unit);
        status = BCM_E_CONFIG;
    }

    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }

    bcm_l3_egress_t_init(&bcm_egr);
    status = 
        _bcm_fe2000_l3_get_egrif_from_fte(l3_fe,
                                          info->l3a_intf,
                                          L3_OR_MPLS_GET_FTE__FTE_ETE_BOTH,
                                          &bcm_egr);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }

    ENET_COPY_MACADDR(bcm_egr.mac_addr, info->l3a_nexthop_mac);
    info->l3a_modid = bcm_egr.module;
    info->l3a_vid   = bcm_egr.vlan;
    if (bcm_egr.flags & BCM_L3_TGID) {
        info->l3a_port_tgid = bcm_egr.trunk;
    } else {
        info->l3a_port_tgid = bcm_egr.port;
    }
    info->l3a_intf =  _FE2K_GET_USER_HANDLE_FROM_FTE_IDX(info->l3a_intf);

    L3_UNLOCK(unit);
    return status;
}

int
bcm_fe2000_l3_defip_get(int unit,
                     bcm_l3_route_t *info)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_l3_route_multipath_get(int             unit,
                                  bcm_l3_route_t *info,
                                  bcm_l3_route_t *path_array,
                                  int             max_path,
                                  int            *path_count)
{
    int                          status;
    _fe2k_l3_fe_instance_t      *l3_fe;
    uint32                       base_fte=0;
    bcm_l3_egress_t              bcm_egr;
    _fe2k_egr_mp_info_t         *mp_info;
    uint32                       i;

    status = BCM_E_NONE;

    if ((path_count == NULL) || (info == NULL) ||
        (path_array == NULL) || (max_path <= 0)) {
        return (BCM_E_PARAM);
    }

    if (info->l3a_flags & BCM_L3_RPF) {
        return (BCM_E_PARAM);
    }

    if (!_bcm_fe2000_ip_is_valid(info)      ||
        BCM_IP4_MULTICAST(info->l3a_subnet) ||
        BCM_IP6_MULTICAST(info->l3a_ip6_net)) {
        return BCM_E_PARAM;
    }

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
       L3_UNLOCK(unit);
       return  BCM_E_UNIT;
    }

    if (!_BCM_VRF_VALID(info->l3a_vrf)) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "invalid vrf(0x%x)\n"), info->l3a_vrf));
        return BCM_E_PARAM;
    }

    if (l3_fe->fe_vpn_by_vrf[info->l3a_vrf] == NULL) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "vrf(0x%x) not yet created\n"), info->l3a_vrf));
        L3_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    /*
     * This is the base FTE of the mpath object. To get the
     * unique set of FTEs, we need the orginal FTEs from which
     * this mpath was derived. So using this base fte, "get" the
     * mpath object. Then for each FTE get the egress info
     */

    switch(SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        status = _bcm_fe2000_g2p3_ip_ecmp_route_get(l3_fe, info, &base_fte);
        break;
#endif
    default:
        L3_UKNOWN_UCODE_WARN(l3_fe->fe_unit);
        status = BCM_E_CONFIG;
    }

    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }

    status = _bcm_fe2000_l3_mpath_get(l3_fe,base_fte, &mp_info);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }

    if (max_path < mp_info->num_valid) {
        L3_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    for (i = 0; i < mp_info->num_valid; i++) {
        bcm_l3_egress_t_init(&bcm_egr);
        status = _bcm_fe2000_l3_get_egrif_from_fte(l3_fe,
                                                   mp_info->mapped_fte[i],
                                                   L3_OR_MPLS_GET_FTE__FTE_ETE_BOTH,
                                                   &bcm_egr);
        if (status != BCM_E_NONE) {
            L3_UNLOCK(unit);
            return status;
        }

        path_array->l3a_modid     = bcm_egr.module;
        path_array->l3a_intf      = _FE2K_GET_USER_HANDLE_FROM_FTE_IDX(mp_info->mapped_fte[i]);
        ENET_COPY_MACADDR(bcm_egr.mac_addr, path_array->l3a_nexthop_mac);
        path_array++;
    }

    L3_UNLOCK(unit);
    return status;
}

int
bcm_fe2000_l3_defip_find_index(int unit,
                            int index,
                            bcm_l3_route_t *info)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_l3_route_delete_by_interface(int unit,
                                        bcm_l3_route_t *info)
{
    int                     status=BCM_E_NONE;
    _fe2k_l3_fe_instance_t *l3_fe;
    uint32                  fte_idx;


    fte_idx =  _FE2K_GET_FTE_IDX_FROM_USER_HANDLE(info->l3a_intf);

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
       L3_UNLOCK(unit);
       return  BCM_E_UNIT;
    }

    if (!(_FE2K_L3_FTE_VALID(l3_fe->fe_unit, info->l3a_intf))) {
        L3_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    switch(SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        status = _bcm_fe2000_g2p3_ipv4_route_delete_all(l3_fe,
                                                        fte_idx,
                                                        L3_ROUTE_DELETE_BY_INTF);
        break;
#endif
    default:
        L3_UKNOWN_UCODE_WARN(l3_fe->fe_unit);
        status = BCM_E_CONFIG;
    }

    L3_UNLOCK(unit);
    return status;
}

int
bcm_fe2000_l3_defip_delete_by_interface(int unit,
                                     int intf)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_l3_route_delete_all(int unit,
                            bcm_l3_route_t *info)
{
    int                     status=BCM_E_NONE;
    _fe2k_l3_fe_instance_t *l3_fe;

    if (info == NULL) {
        return BCM_E_PARAM;
    }

    if (info->l3a_flags & BCM_L3_IP6) {
        return (BCM_E_PARAM);
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
       L3_UNLOCK(unit);
       return  BCM_E_UNIT;
    }

    L3_UNLOCK(unit);
    return status;
}

int
bcm_fe2000_l3_route_traverse(int unit,
                          uint32 flags,
                          uint32 start,
                          uint32 end,
                          bcm_l3_route_traverse_cb trav_fn,
                          void *user_data)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_l3_defip_traverse(int unit,
                          bcm_l3_route_traverse_cb trav_fn,
                          uint32 start,
                          uint32 end)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_l3_route_age(int unit,
                     uint32 flags,
                     bcm_l3_route_traverse_cb age_out)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_l3_defip_age(int unit,
                     bcm_l3_route_traverse_cb age_out)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_l3_status(int unit,
                  int *free_l3intf,
                  int *free_l3,
                  int *free_defip,
                  int *free_lpm_blk)
{
    return BCM_E_UNAVAIL;
}


int
bcm_fe2000_l3_defip_max_ecmp_set(int unit,
                              int max)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_l3_defip_max_ecmp_get(int unit,
                              int *max)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_l3_enable_set(int unit,
                      int enable)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_l3_info(int unit,
                bcm_l3_info_t *l3info)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_l3_untagged_update(int unit)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *     bcm_fe2000_l3_egress_create
 * Purpose:
 *      Create an Egress forwarding object.
 * Parameters:
 *      unit    - (IN)  bcm device.
 *      flags   - (IN)  BCM_L3_REPLACE: replace existing.
 *                      BCM_L3_WITH_ID: intf argument is given.
 *      egr     - (IN)  Egress forwarding destination.
 *      intf    - (OUT) fte index interface id corresponding to the
 *                      locally allocated FTE pointing to Egress object.
 *                      encap_id containg OHI
 * Assumptions
 *      1. The interface object needs to be created prior to this
 *         call.
 *      2. if trunk field is set, it implies ingress lag is set and
 *         encap_id contains OHI
 *      3. Ignore the vlan field.
 * Implementation Notes:
 *      Case (i):: Module is local
 *         1. Check if the egress object already exists. The interface
 *            object has the list of all egress objects that belong to
 *            it.  <type, mac, vidop, ttl> identify a unique egress object.
 *            There are two cases here
 *            a) Egress object and FTE both exist: Return FTEIdx + OHI
 *            b) Egress object exits but no FTE for local module.
 *                  Allocate FteIdx, populate in SW ETE. Program
 *                  HW with OHI copy in SW ETE. Return FTEIdx + OHI
 *            c) Egress object does not exist
 *         2. Case (c) above. Allocate the following HW resources
 *            a) OHI : In case of ingress LAG (BCM_L3_TGID flag set or
 *                     the trunk field is set), the OHI is passed by the
 *                     user in the encap_id field.
 *                     In some other cases (TBD) also the OHI may be passed
 *                     by user. In such cases we need to use the same. This is
 *                     more likely in the remote module case.
 *            b) ETE Index
 *            c) FTE Index
 *         4. Allocate an egress object and link it to the per intf list of
 *            egress objects. Populate with the HW indices and set up the
 *            OHI and ETE arrays to point to it
 *         5. Program the FTE, OHI and ETE in HW and return the FTE index as
 *            an output param and OHI in encap_id
 *      If there is a tunnel_initiator set pending on the interface for then create the
 *      tunnel ete and point the OHI to the tunnel ETE
 */
int
bcm_fe2000_l3_egress_create(int              unit,
                            uint32           flags,
                            bcm_l3_egress_t *egr,
                            bcm_if_t        *intf)
{
    int                     status;
    _fe2k_l3_fe_instance_t *l3_fe;
    _fe2k_fte_idx_t         fte_idx;

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
       L3_UNLOCK(unit);
       return  BCM_E_UNIT;
    }

    /*
     * checks common to add/replace
     */
    status = _bcm_fe2000_l3_egress_create_checks(l3_fe, flags, egr, intf);

    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }

    /*
     * interface id is required in either  case. We made sure that
     * a valid user interface handle is passed in the check routines
     * Now we need to convert it to the internal representation
     */
    egr->intf =  _FE2K_IFID_FROM_USER_HANDLE(egr->intf);

    fte_idx.fte_idx = 0;

    if (flags & BCM_L3_REPLACE) {
        fte_idx.fte_idx = _FE2K_GET_FTE_IDX_FROM_USER_HANDLE(*intf);

        status = _bcm_fe2000_update_l3_fte(l3_fe, flags, egr, &fte_idx);
    } else {

        status = _bcm_fe2000_add_l3_fte(l3_fe, flags, egr, &fte_idx);
    }

    egr->intf =  _FE2K_USER_HANDLE_FROM_IFID(egr->intf);
    if (status == BCM_E_NONE) {
        *intf = _FE2K_GET_USER_HANDLE_FROM_FTE_IDX(fte_idx.fte_idx);
    }

    L3_VERB((_SBX_D(l3_fe->fe_unit, "fte(0x%x) encap-id(0x%x)\n"),
             *intf, egr->encap_id));

    L3_UNLOCK(unit);
    return status;
}

/*
 * Function:
 *      bcm_fe2000_l3_egress_destroy
 * Purpose:
 *      Destroy an Egress forwarding object.
 * Parameters:
 *      unit    - (IN) bcm device.
 *      intf    - (IN)  FTE index pointing to Egress object.
 * Returns:
 *      BCM_E_BUSY - Some other FTEs still using this object
 *
 * Assumptions:
 *      The egress object (FTE-ETE pair) is not being used by
 *      any multipath group.
 *
 * Implementation Notes:
 *      1. Fetch the contents of FTE and get node(module) and OHI from HW
 *      2. Module is local
 *         i)   Get SW ete from OHI.
 *         iii) There are remote FTEs present:
 *                 remove local FTE from SW ete
 *                 Deprogram local and return FTE to allocator
 *         iii) There are no remote FTEs using the ete
 *                 Deprogram  HW ete, HW OHI and HW FTE and free the
 *                 corresponding indices in the allocators
 *                 Remove SW ete structure from interface list
 *                 Clean up SW ETEptr array and OHI2ETE array
 *          iv) If the ete is being removed and this is the last ete
 *              on the interface, then remove the tunnel ete as well.
 */
int
bcm_fe2000_l3_egress_destroy(int      unit,
                             bcm_if_t intf)
{
    int                              status;
    _fe2k_l3_fe_instance_t          *l3_fe;
    uint32                           fte_idx;
    _fe2k_l3_fte_t                  *fte_hash_elem;

    L3_VVERB((_SBX_D(unit, "Enter\n")));

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));
    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
       L3_UNLOCK(unit);
       return  BCM_E_UNIT;
    }

    if (!(_FE2K_L3_FTE_VALID(l3_fe->fe_unit, intf))) {
        L3_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    fte_idx =  _FE2K_GET_FTE_IDX_FROM_USER_HANDLE(intf);

    status = _bcm_fe2000_l3_get_sw_fte(l3_fe,
                                       fte_idx,
                                       &fte_hash_elem);
    if (status != BCM_E_NONE) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "fte 0x%x not found in hash table\n"), fte_idx));
        L3_UNLOCK(unit);
        return status;
    }
    if (fte_hash_elem->ref_count > 1) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "fte 0x%x in use"), fte_idx));
        L3_UNLOCK(unit);
        return BCM_E_BUSY;
    }
    status =  _bcm_fe2000_destroy_fte(l3_fe,
                                      L3_OR_MPLS_DESTROY_FTE__FTE_HW_OHI_ETE,
                                      fte_idx,
                                      0, /* mod */
                                      0  /* ohi */);

    L3_UNLOCK(unit);
    return status;
}

/*
 * Function:
 *      bcm_fe2000_l3_egress_get
 * Purpose:
 *      Get an Egress forwarding object.
 * Parameters:
 *      unit    - (IN) bcm device.
 *      intf    - (IN) L3 fteIdx retruned at egress create time
 *      egr     - (OUT) Egress forwarding destination.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_NOT_FOUND
 *
 * Implementation Notes:
 *
 *     1. Fetch the contents of FTE and get node(module) and OHI from HW
 *     2. Module is local.
 *        Get the SW ete pointer from OHI. From the SW ete ptr get HW eteIdx
 *        Based on HW eteIdx, fetch the contents of ete.
 *     3. Module is remote: pass the OHI to remote module.
 */
int
bcm_fe2000_l3_egress_get(int              unit,
                         bcm_if_t         intf,
                         bcm_l3_egress_t *egr)
{
    int                     status = BCM_E_NONE;
    _fe2k_l3_fe_instance_t *l3_fe;
    uint32                  fte_idx;

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    if (!(_FE2K_L3_FTE_VALID(unit, intf)) || (egr == NULL)) {
        status = BCM_E_PARAM;
    } else {
        l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
        if (l3_fe == NULL) {
            status = BCM_E_UNIT;

        } else {
            fte_idx =  _FE2K_GET_FTE_IDX_FROM_USER_HANDLE(intf);
            bcm_l3_egress_t_init(egr);
            status = _bcm_fe2000_l3_get_egrif_from_fte(l3_fe,fte_idx,
                                                       L3_OR_MPLS_GET_FTE__FTE_ETE_BOTH,
                                                       egr);
            egr->intf     =  _FE2K_USER_HANDLE_FROM_IFID(egr->intf);
        }
    }
    L3_UNLOCK(unit);
    return status;
}

/*
 * Function:
 *      bcm_fe2000_l3_egress_find
 * Purpose:
 *      Find an egress forwarding object.
 * Parameters:
 *      unit       - (IN) bcm device.
 *      egr        - (IN) Egress object properties to match.
 *                        l3a_intf_id, module, dmac and port
 *      intf       - (OUT) FteIdx if found
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_NOT_FOUND
 *
 * Implementation Notes:
 *      1. From the module determine the node, which could be
 *         local or remote.
 *      2. For local node,
 *         i)  determine the l3_intf from the intf_id
 *         ii) Walk the list of egress objects on the intf matching for
 *             dmac and port.
 *         iii)On a match, walk locate the fte by in the <modid, fte>
 *             array by matching the modid.
 *      3. For remote node
 *         Same as above, except when locating the fte in the egress
 *         object , we need to match on the calling units modid.
 *
 */
int
bcm_fe2000_l3_egress_find(int              unit,
                          bcm_l3_egress_t *egr,
                          bcm_if_t        *intf)
{
    int                     status;
    _fe2k_l3_fe_instance_t *l3_fe;
    uint32                  fte_idx;
    bcm_l3_egress_t         bcm_egr;

    if (egr == NULL || intf == NULL) {
        return BCM_E_PARAM;
    }
    if (!(_FE2K_L3_USER_IFID_VALID(egr->intf))) {
        return (BCM_E_PARAM);
    }

    if ((BCM_MAC_IS_MCAST(egr->mac_addr)) ||
        (BCM_MAC_IS_ZERO(egr->mac_addr))) {
        return (BCM_E_PARAM);
    }

    if (!(SOC_SBX_MODID_ADDRESSABLE(unit, egr->module))) {
        return (BCM_E_PARAM);
    }

    if (egr->encap_id && !SOC_SBX_IS_VALID_ENCAP_ID(egr->encap_id)) {
            return BCM_E_PARAM;
    }

    if (egr->flags & BCM_L3_TGID) {
        if (!SBX_TRUNK_VALID(egr->trunk)) {
            return BCM_E_PARAM;
        }
    } else if (!(SOC_PORT_VALID(unit, egr->port))) {
        return (BCM_E_PARAM);
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));
    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
       L3_UNLOCK(unit);
       return  BCM_E_UNIT;
    }

    bcm_egr          = *egr;
    bcm_egr.intf     =  _FE2K_IFID_FROM_USER_HANDLE(egr->intf);
    bcm_egr.encap_id =  egr->encap_id;

    status = _bcm_fe2000_find_fte(l3_fe,
                                  &bcm_egr,
                                  &fte_idx);
    if (status == BCM_E_NONE) {
        *intf = _FE2K_GET_USER_HANDLE_FROM_FTE_IDX(fte_idx);
    }

    L3_UNLOCK(unit);
    return status;
}

int
bcm_fe2000_l3_egress_traverse(int                     unit,
                           bcm_l3_egress_traverse_cb  trav_fn,
                           void                      *user_data)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *    bcm_fe2000_l3_route_max_ecmp_set
 * Purpose:
 *    Set the maximum ECMP paths allowed for a route
 * Parameters:
 *    unit       - (IN) bcm device.
 *    max        - (IN) MAX number of paths for ECMP
 * Returns:
 *      BCM_E_XXX
 * Note:
 *    This function can be called before ECMP routes are added,
 *    normally at the beginning.  Once ECMP routes exist, cannot be reset.
 */
int
bcm_fe2000_l3_route_max_ecmp_set(int unit, int max)
{
    int                      status;
    _fe2k_l3_fe_instance_t  *l3_fe;

    if (max >= _FE2K_L3_ECMP_MAX) {
        return  BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
       L3_UNLOCK(unit);
       return  BCM_E_UNIT;
    }

    status = BCM_E_NONE;
    if (l3_fe->fe_flags & _FE2K_L3_FE_FLG_MP_SIZE_SET) {
        L3_WARN((_SBX_D(l3_fe->fe_unit, "Cannot change ecmp size, Multipath routes exist\n")));
        status = BCM_E_EXISTS;
    } else {
        l3_fe->fe_mp_set_size = max;
    }

    L3_UNLOCK(unit);
    return status;
}

/*
 * Function:
 *    bcm_fe2000_l3_route_max_ecmp_get
 * Purpose:
 *    Get the maximum ECMP paths allowed for a route
 * Parameters:
 *    unit       - (IN) bcm device.
 *    max        - (OUT) MAX number of paths for ECMP
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_fe2000_l3_route_max_ecmp_get(int unit, int *max)
{
    int                      status;
    _fe2k_l3_fe_instance_t  *l3_fe;

    if (max == NULL) {
        return  BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
       L3_UNLOCK(unit);
       return  BCM_E_UNIT;
    }

    status = BCM_E_NONE;
    *max   = l3_fe->fe_mp_set_size;

    L3_UNLOCK(unit);
    return status;
}

/* Function:
 *      bcm_fe2000_l3_egress_multipath_create
 * Purpose:
 *      Create an Egress Multipath forwarding object.
 * Parameters:
 *      unit       - (IN) bcm device.
 *      flags      - (IN) BCM_L3_REPLACE: replace existing.
 *                        BCM_L3_WITH_ID: mpintf argument is given.
 *      intf_count - (IN) Number of elements in intf_array.
 *      intf_array - (IN) Array of Egress forwarding objects. (FteIndices)
 *      mpintf     - (OUT) BaseFteIndex of the newly allocated mpath Group.
 *                         This is an IN argument if either BCM_L3_REPLACE
 *                         or BCM_L3_WITH_ID are given in flags.
 * Returns:
 *      BCM_E_XXX
 *
 * Implementation Notes:
 *      1. Sort the FteIndices passed in and work with the sorted list
 *      2. Case I: REPLACE flag not set
 *         i) lookup the list of ftes in the hash table. If found
 *         return error
 *         ii) allocate a contigous set of intf_count ftes (beginning
 *             with mpintf if BCM_L3_WITH_ID if given) .
 *             The first in the array is the base fte that will be
 *             returned in mpintf
 *         iii) For each mapped fte in the list, get HW info and clone
 *              a new Fte.
 *         iv) Insert the list of mapped FTEs in a the hash table.
 *      3. Case II: REPLACE flag set
 *         i)  The FTE list must be in hash table
 *         ii) From mpintf fetch each old fte index and deprogram HW
 *         iii)Clone each FTE in the group as above
 */
int
bcm_fe2000_l3_egress_multipath_create(int       unit,
                                      uint32    flags,
                                      int       intf_count,
                                      bcm_if_t *intf_array,
                                      bcm_if_t *mpintf)
{
    int                          status;
    _fe2k_l3_fe_instance_t      *l3_fe;
    _fe2k_l3_mpath_crc_info_t   *crc_info;
    _fe2k_egr_mp_info_t         *mp_info;
    uint32                       i;
    uint32                       mpbase;
    bcm_if_t                     new_ftes[_FE2K_L3_ECMP_MAX];

    if ((intf_array == NULL) || (mpintf == NULL)) {
        return (BCM_E_PARAM);
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));
    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
       L3_UNLOCK(unit);
       return  BCM_E_UNIT;
    }

    if ((intf_count < 0) ||
        (intf_count > l3_fe->fe_mp_set_size) ||
        (intf_count > _FE2K_L3_ECMP_MAX)) {
        L3_UNLOCK(unit);
        return  BCM_E_PARAM;
    }

    /*
     * Validate and translate the FTEs
     */
    for (i = 0; i < intf_count; i++) {
        if (!(_FE2K_L3_FTE_VALID(unit, intf_array[i]))) {
            L3_UNLOCK(unit);
            return  BCM_E_PARAM;
        }
        new_ftes[i] =  _FE2K_GET_FTE_IDX_FROM_USER_HANDLE(intf_array[i]);
    }

    if ((flags & BCM_L3_REPLACE) || (flags & BCM_L3_WITH_ID)) {
        /*
         * in either case mpintf must contain a valid FTE
         */
        if (!_FE2K_L3_FTE_VALID(unit, *mpintf)) {
            L3_UNLOCK(unit);
            return  BCM_E_PARAM;
        }
    }

    /*
     * crc info contains all the FTEs in sorted order and
     * the calculated CRC16 for hashing
     */
    status   = BCM_E_NONE;
    crc_info = NULL;

    if (flags & BCM_L3_REPLACE) {
        /*
         * case:: update mpath
         */
        mpbase = _FE2K_GET_FTE_IDX_FROM_USER_HANDLE(*mpintf);

        /*
         * The mpath object is returned in mp_info
         */
        status = _bcm_fe2000_l3_mpath_get(l3_fe,
                                          mpbase,
                                          &mp_info);
        if (status != BCM_E_NONE) {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "Multipath object (index = %d) to"
                           " Replace not found\n"),
                    *mpintf));
            L3_UNLOCK(unit);
            return  status;
        }

        status = _bcm_fe2000_handle_mpath_update(l3_fe,
                                                 mp_info,    /* existing mpath object */
                                                 intf_count,
                                                 new_ftes);
        if (status != BCM_E_NONE) {
            L3_UNLOCK(unit);
            return status;
        }
    } else {
        /*
         * case:: new mpath
         */
        if (flags & BCM_L3_WITH_ID) {
            /*
             * do we have the full range of fte's available ?
             */
            mpbase =   _FE2K_GET_FTE_IDX_FROM_USER_HANDLE(*mpintf);

            for (i = mpbase; i < (mpbase+l3_fe->fe_mp_set_size); i++) {
                status = _sbx_gu2_resource_test(unit,
                                                SBX_GU2K_USR_RES_FTE_L3_MPATH,
                                                i);
                if (status != BCM_E_NOT_FOUND) {
                    L3_ERR((_SBX_D(l3_fe->fe_unit, "FTE Index Range (%d - %d) in use\n"),
                            mpbase,
                            mpbase + l3_fe->fe_mp_set_size - 1));
                    L3_UNLOCK(unit);
                    return BCM_E_EXISTS;
                }
            }
        }
        else {
            mpbase = _FE2K_INVALID_FTE;
        }

        crc_info = sal_alloc(sizeof(_fe2k_l3_mpath_crc_info_t), "mpath-crc-info");
        if (crc_info == NULL) {
            L3_UNLOCK(unit);
            return BCM_E_MEMORY;
        }
        sal_memset(crc_info, 0, sizeof(_fe2k_l3_mpath_crc_info_t));
        status = _bcm_fe2000_l3_gen_mpath_crc(l3_fe,
                                              mpbase,
                                              intf_count,
                                              new_ftes,
                                              crc_info,
                                              L3_CRC_INFO_FETCH_FTE_DATA);
        if (status != BCM_E_NONE) {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "error in setting up Mpath Ftes: %s\n"),
                    bcm_errmsg(status)));
            sal_free(crc_info);
            crc_info = NULL;
            L3_UNLOCK(unit);
            return status;
        }

        /*
         * mpbase is either reserved or allocated based on
         * BCM_L3_WITH_ID flag inside add_mpath_object.
         */
        status = _bcm_fe2000_l3_add_mpath_object(l3_fe,
                                                 flags,
                                                 crc_info,
                                                 &mp_info);
        if (status != BCM_E_NONE) {
            L3_UNLOCK(unit);
            sal_free(crc_info);
            crc_info = NULL;
            return status;
        }
    }

    l3_fe->fe_flags |= _FE2K_L3_FE_FLG_MP_SIZE_SET;
    *mpintf =  _FE2K_GET_USER_HANDLE_FROM_FTE_IDX(mp_info->mp_base);

    if (crc_info) {
        _bcm_fe2000_l3_free_mpath_crc(crc_info, intf_count);
        crc_info = NULL;
    }

    L3_UNLOCK(unit);
    return status;
}

/*
 * Function:
 *      bcm_fe2000_l3_egress_multipath_destroy
 * Purpose:
 *      Destroy an Egress Multipath forwarding object.
 * Parameters:
 *      unit    - (IN) bcm device.
 *      mpintf  - (IN) BaseFteIndex
 * Returns:
 *      BCM_E_XXX
 *
 * Implementation Notes:
 *      Free the Derived FTEs and remove from Hash table and
 *      unlink from the mp_base2info array
 */
int
bcm_fe2000_l3_egress_multipath_destroy(int unit,
                                       bcm_if_t mpintf)
{
    int                          status = 0;
    _fe2k_l3_fe_instance_t      *l3_fe;
    _fe2k_egr_mp_info_t         *mp_info;
    uint32                       i, mp_base, count;
    _fe2k_l3_fte_t              *fte_hash_elems[_FE2K_L3_ECMP_MAX];
    _fe2k_l3_fte_t              *mp_base_hash_elem;

    if (!_FE2K_L3_FTE_VALID(unit, mpintf)) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));
    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
       L3_UNLOCK(unit);
       return  BCM_E_UNIT;
    }

    mp_base =  _FE2K_GET_FTE_IDX_FROM_USER_HANDLE(mpintf);

    status = _bcm_fe2000_l3_mpath_get(l3_fe,
                                      mp_base,
                                      &mp_info);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return  status;
    }

    /*
     * Make sure that the ref count on the base fte is not > 1
     */
    status = _bcm_fe2000_l3_get_sw_fte(l3_fe,
                                       mp_base,
                                       &mp_base_hash_elem);
    if (status != BCM_E_NONE) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "fte idx 0x%x not found in hash table\n"), mp_base));
        L3_UNLOCK(unit);
        return status;
    } else {
        /*
         * if the base fte has a ref count more than one -- in use error
         */
        if (mp_base_hash_elem->ref_count > 1) {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "fte idx 0x%x has a refcount of %d\n"),
                    mp_base, mp_base_hash_elem->ref_count));
             L3_UNLOCK(unit);
            return BCM_E_BUSY;
        }
    }

    /*
     * save the fte hash elemnts for the mapped ftes. This
     * info will be used to decr the ref_counts later on.
     * We do this now so that we can bail out before we start
     * mucking with the ftes
     */
    for (i = 0; i < _FE2K_L3_ECMP_MAX; i++) {
        fte_hash_elems[i] = NULL;
    }
    count  = mp_info->num_valid;
    status = _bcm_fe2000_l3_save_fte_hash_elements(l3_fe,
                                                   count,
                                                   mp_info->mapped_fte,
                                                   fte_hash_elems);

    if (status != BCM_E_NONE) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "mapped ftes not found in hash table: %s\n"),
                bcm_errmsg(status)));
        L3_UNLOCK(unit);
        return BCM_E_INTERNAL;
    }
    for (i = 0; i < count; i++) {
        /*
         * The original FTEs should have a ref_count of atleast 2
         * On creation we have ref_count = 1 and when the mpath
         * was created it should have been incremeted by 1
         */
        if ((fte_hash_elems[i])->ref_count < 2) {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "Invalid refCount %d on mapped fte 0x%x\n"),
                    (fte_hash_elems[i])->ref_count,
                    mp_info->mapped_fte[i]));
            L3_UNLOCK(unit);
            return BCM_E_INTERNAL;
        }
    }

    status = _bcm_fe2000_l3_destroy_mpath_object(l3_fe,
                                                 &mp_info);
    if (status != BCM_E_NONE) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "Could not destroy multipath object %d: %s\n"),
                mpintf, bcm_errmsg(status)));
    }

    /*
     * Now decrement the ref counts on the mapped ftes
     */
    for (i = 0; i < count; i++) {
        (fte_hash_elems[i])->ref_count--;
    }

    L3_UNLOCK(unit);
    return status;
}

/*
 * Function:
 *      bcm_fe2000_l3_egress_multipath_get
 * Purpose:
 *      Get an Egress Multipath forwarding object.
 * Parameters:
 *      unit       - (IN) bcm device.
 *      mpintf     - (IN) BaseFteIndex that was returned at create time
 *      intf_size  - (IN) Size of allocated entries in intf_array.
 *      intf_array - (OUT) Array of mapped FTE indices
 *      intf_count - (OUT) Number of entries of intf_count actually filled in.
 *
 * Returns:
 *      BCM_E_XXX
 *
 * Implementation Notes:
 *      Using mpintf and the mp_base2info array, get the mpath group
 *      This contains a list of mapped FTEs that are returned
 */
int
bcm_fe2000_l3_egress_multipath_get(int       unit,
                                   bcm_if_t  mpintf,
                                   int       intf_size,
                                   bcm_if_t *intf_array,
                                   int      *intf_count)
{
    int                           status;
    _fe2k_l3_fe_instance_t       *l3_fe;
    _fe2k_egr_mp_info_t          *mp_info;
    int                           i;
    uint32                        mp_base;

    if ((intf_array == NULL) || (intf_count == NULL) || (intf_size == 0)) {
        return BCM_E_PARAM;
    }

    if (!_FE2K_L3_FTE_VALID(unit, mpintf)) {
        return BCM_E_PARAM;
    }

    mp_base =  _FE2K_GET_FTE_IDX_FROM_USER_HANDLE(mpintf);

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));
    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
       L3_UNLOCK(unit);
       return  BCM_E_UNIT;
    }
    mp_info = NULL;
    status  = _bcm_fe2000_l3_mpath_get(l3_fe,
                                       mp_base,
                                       &mp_info);
    if (status != BCM_E_NONE) {
        L3_UNLOCK(unit);
        return status;
    }

    if (intf_size < mp_info->num_valid) {
        L3_UNLOCK(unit);
        return BCM_E_PARAM;
    }

    *intf_count = mp_info->num_valid;
    for (i = 0; i < mp_info->num_valid; i++) {
        intf_array[i] = _FE2K_GET_USER_HANDLE_FROM_FTE_IDX(mp_info->mapped_fte[i]);
    }

    L3_UNLOCK(unit);
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_fe2000_l3_egress_multipath_add
 * Purpose:
 *      Add an Egress forwarding object to an Egress Multipath
 *      forwarding object.
 * Parameters:
 *      unit    - (IN) bcm device.
 *      mpintf  - (IN) BaseFteIndex that was returned at create time
 *      intf    - (IN) A new mapped FteIndex that needs to be added
 * Returns:
 *      BCM_E_XXX
 *
 * Implementation Notes:
 *      1. Fetch the mpath group based on mpath_intf
 *      2. Check to make sure that we are within the MAX_ECMP limit.
 *      3. remove the group from egr_mp hash.
 *      4. allocate a new FTE and clone the mapped FTE.
 *      5. Re-program the entire FTE set in HW because the FTE arrangement
 *         in the set may have changed. For example if MAX_ECMP was 4 and
 *         the group had two FTEs (A & B) then the HW FTEs would be {A B A B}
 *         With the addition of a new FTE the pattern must become {A B C A}
 *      6. Add the new FTE to the group, recompute hash and insert back
 */
int
bcm_fe2000_l3_egress_multipath_add(int unit,
                                   bcm_if_t mpintf,
                                   bcm_if_t intf)
{
    int                          status, ignore_status;
    _fe2k_egr_mp_info_t         *mp_info;
    _fe2k_l3_fe_instance_t      *l3_fe;
    uint32                       mpbase, new_fte_idx;
    bcm_if_t                     new_ftes[_FE2K_L3_ECMP_MAX];
    uint32                       i;

    if (!_FE2K_L3_FTE_VALID(unit, mpintf) ||
        !_FE2K_L3_FTE_VALID(unit, intf)) {
          return (BCM_E_PARAM);
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));
    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return  BCM_E_UNIT;
    }

    status = ignore_status = BCM_E_NONE;

    mpbase      =  _FE2K_GET_FTE_IDX_FROM_USER_HANDLE(mpintf);
    new_fte_idx =  _FE2K_GET_FTE_IDX_FROM_USER_HANDLE(intf);

    status = _bcm_fe2000_l3_mpath_get(l3_fe,
                                      mpbase,
                                      &mp_info);
    if (status != BCM_E_NONE) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "Could not find mpath object with base 0x%x: %s\n"),
                mpintf, bcm_errmsg(status)));
        L3_UNLOCK(unit);
        return  status;
    }

    if (mp_info->num_valid == l3_fe->fe_mp_set_size) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "Max ECMP set size (%d) exceeded\n"),
                l3_fe->fe_mp_set_size));
        L3_UNLOCK(unit);
        return BCM_E_FULL;
    }

    for (i = 0; i < mp_info->num_valid; i++) {
        new_ftes[i] = mp_info->mapped_fte[i];
    }
    new_ftes[mp_info->num_valid] = new_fte_idx;

    status = _bcm_fe2000_handle_mpath_update(l3_fe,
                                             mp_info,
                                             mp_info->num_valid+1,
                                             new_ftes);

    L3_UNLOCK(unit);
    return  status;
}

/*
 * Function:
 *      bcm_fe2000_l3_egress_multipath_delete
 * Purpose:
 *      Delete an Egress forwarding object to an Egress Multipath
 *      forwarding object.
 * Parameters:
 *      unit    - (IN) bcm device.
 *      mpintf  - (IN) BaseFteIndex that was returned at create time
 *      intf    - (IN) A mapped FteIndex that needs to be deleted
 * Returns:
 *      BCM_E_XXX
 * Implementation Notes:
 *      Follows logic similar to  bcm_fe2000_l3_egress_multipath_add
 *      except that the mapped FTE is deleted from the set
 */
int
bcm_fe2000_l3_egress_multipath_delete(int unit,
                                   bcm_if_t mpintf,
                                   bcm_if_t intf)
{
    int                          status;
    _fe2k_egr_mp_info_t         *mp_info;
    _fe2k_l3_fe_instance_t      *l3_fe;
    uint32                       mpbase, del_fte_idx;
    bcm_if_t                     new_ftes[_FE2K_L3_ECMP_MAX];
    uint32                       i, new_num_valid;

    if (!_FE2K_L3_FTE_VALID(unit, mpintf) ||
        !_FE2K_L3_FTE_VALID(unit, intf)) {
          return (BCM_E_PARAM);
    }

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));
    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return  BCM_E_UNIT;
    }

    status = BCM_E_NONE;

    mpbase      =  _FE2K_GET_FTE_IDX_FROM_USER_HANDLE(mpintf);
    del_fte_idx =  _FE2K_GET_FTE_IDX_FROM_USER_HANDLE(intf);

    status = _bcm_fe2000_l3_mpath_get(l3_fe,
                                      mpbase,
                                      &mp_info);
    if (status != BCM_E_NONE) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "Could not find mpath object with base %d"), mpintf));
        L3_UNLOCK(unit);
        return  status;
    }

    /*
     * remove the deleted fte
     */
    for (i = 0, new_num_valid = 0; i < mp_info->num_valid; i++) {
        if (mp_info->mapped_fte[i] != del_fte_idx) {
            new_ftes[new_num_valid] = mp_info->mapped_fte[i];
            new_num_valid++;
        }
    }

    if (new_num_valid == mp_info->num_valid) {
        L3_UNLOCK(unit);
        return BCM_E_NOT_FOUND;
    }

    status = _bcm_fe2000_handle_mpath_update(l3_fe,
                                             mp_info,
                                             new_num_valid,
                                             new_ftes);
    if (status != BCM_E_NONE) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "error in updating _mpath_ftes: %s\n"), 
                bcm_errmsg(status)));
        L3_UNLOCK(unit);
        return status;
    }

    L3_UNLOCK(unit);
    return  status;
}

/*
 * Function:
 *      bcm_fe2000_l3_egress_multipath_find
 * Purpose:
 *      Find an egress multipath forwarding object.
 * Parameters:
 *      unit       - (IN) bcm device.
 *      intf_count - (IN) Number of elements in intf_array.
 *      intf_array - (IN) Array of egress forwarding objects.
 *      mpintf     - (OUT) base fte of mpath object
 *
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_fe2000_l3_egress_multipath_find(int       unit,
                                    int       intf_count,
                                    bcm_if_t *intf_array,
                                    bcm_if_t *mpintf)
{
    int                           status;
    _fe2k_l3_fe_instance_t       *l3_fe;
    _fe2k_egr_mp_info_t          *mp_info;
    int                           i;
    _fe2k_l3_mpath_crc_info_t    *crc_info;
    bcm_if_t                      ftes[_FE2K_L3_ECMP_MAX];

    if ((mpintf == NULL) || (intf_array == NULL) || (intf_count == 0)) {
        return BCM_E_PARAM;
    }


    BCM_IF_ERROR_RETURN(L3_LOCK(unit));
    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (l3_fe == NULL) {
        L3_UNLOCK(unit);
        return  BCM_E_UNIT;
    }

    if (intf_count > l3_fe->fe_mp_set_size) {
        L3_UNLOCK(unit);
        return  BCM_E_PARAM;
    }

    /*
     * Validate and translate the FTEs
     */
    for (i = 0; i < intf_count; i++) {
        if (!_FE2K_L3_FTE_VALID(unit, intf_array[i])) {
            L3_UNLOCK(unit);
            return  BCM_E_PARAM;
        }
        ftes[i] =  _FE2K_GET_FTE_IDX_FROM_USER_HANDLE(intf_array[i]);
    }

    crc_info = sal_alloc(sizeof(_fe2k_l3_mpath_crc_info_t), "mpath-crc-info");
    if (crc_info == NULL) {
        L3_UNLOCK(unit);
        return BCM_E_MEMORY;
    }
    sal_memset(crc_info, 0, sizeof(_fe2k_l3_mpath_crc_info_t));

    status = _bcm_fe2000_l3_gen_mpath_crc(l3_fe,
                                          _FE2K_INVALID_FTE,
                                          intf_count,
                                          ftes,
                                          crc_info,
                                          L3_CRC_INFO_DONT_FETCH_FTE_DATA);
    if (status != BCM_E_NONE) {
        L3_ERR((_SBX_D(l3_fe->fe_unit, "Error in setting up Mpath Ftes:  %s\n"),
                bcm_errmsg(status)));
        L3_UNLOCK(unit);
        sal_free(crc_info);
        return  BCM_E_PARAM;
    }
    status = _bcm_fe2000_l3_mpath_find(l3_fe,
                                       crc_info,
                                       &mp_info);

    if (status == BCM_E_NONE) {
        *mpintf = _FE2K_GET_USER_HANDLE_FROM_FTE_IDX(mp_info->mp_base);
    }

    L3_UNLOCK(unit);
    _bcm_fe2000_l3_free_mpath_crc(crc_info, intf_count);
    return status;
}


/* VRRP APIs */
int
bcm_fe2000_l3_vrrp_add(int unit,
                       bcm_vlan_t vlan,
                       uint32 vrid)
{
    soc_sbx_g2p3_v2e1_t v2e1;
    int status;
    if (vlan == 0) {
        return BCM_E_PARAM;
    }
    if (vrid == 0) {
        return BCM_E_PARAM;
    }
    status = soc_sbx_g2p3_v2e1_get(unit, vlan, &v2e1);
    if (status != BCM_E_NONE) {
        L3_ERR((_SBX_D(unit, "error %s in soc_sbx_g2p3_v2e1_get"), 
                bcm_errmsg(status)));
        return status;
    }

    /* check whether the vrid is already in the entry */
    if (v2e1.vrid0 == vrid)    return BCM_E_EXISTS;
    if (v2e1.vrid1 == vrid)    return BCM_E_EXISTS;
    if (v2e1.vrid2 == vrid)    return BCM_E_EXISTS;
    if (v2e1.vrid3 == vrid)    return BCM_E_EXISTS;
    if (v2e1.vrid0 == 0) {
        v2e1.vrid0 = vrid;
    } else if (v2e1.vrid1 == 0) {
        v2e1.vrid1 = vrid;
    } else if (v2e1.vrid2 == 0) {
        v2e1.vrid2 = vrid;
    } else if (v2e1.vrid3 == 0) {
        v2e1.vrid3 = vrid;
    } else {
      return BCM_E_RESOURCE;
    }

    status = soc_sbx_g2p3_v2e1_set(unit, vlan, &v2e1);
    if (status != BCM_E_NONE) {
        L3_ERR((_SBX_D(unit, "error %s in soc_sbx_g2p3_v2e1_set"),
                bcm_errmsg(status)));
        return status;
    }

    return BCM_E_NONE;
}

int
bcm_fe2000_l3_vrrp_delete(int unit,
                       bcm_vlan_t vlan,
                       uint32 vrid)
{
    soc_sbx_g2p3_v2e1_t v2e1;
    int status;
    if (vlan == 0) {
        return BCM_E_PARAM;
    }
    if (vrid == 0) {
        return BCM_E_PARAM;
    }
    status = soc_sbx_g2p3_v2e1_get(unit, vlan, &v2e1);
    if (status != BCM_E_NONE) {
        L3_ERR((_SBX_D(unit, "error %s in soc_sbx_g2p3_v2e1_get"),
                bcm_errmsg(status)));
        return status;
    }
    /* check whether the vrid is already in the entry */
    if(v2e1.vrid0 == vrid) {
        v2e1.vrid0 = 0;
    } else if (v2e1.vrid1 == vrid) {
        v2e1.vrid1 = 0;
    } else if (v2e1.vrid2 == vrid) {
        v2e1.vrid2 = 0;
    } else if (v2e1.vrid3 == vrid) {
        v2e1.vrid3 = 0;
    } else return BCM_E_NONE;

    status = soc_sbx_g2p3_v2e1_set(unit, vlan, &v2e1);
    if (status != BCM_E_NONE) {
        L3_ERR((_SBX_D(unit, "error %s in soc_sbx_g2p3_v2e1_set"),
                bcm_errmsg(status)));
        return status;
    }

    return BCM_E_NONE;
}

int
bcm_fe2000_l3_vrrp_delete_all(int unit,
                      bcm_vlan_t vlan)
{
    soc_sbx_g2p3_v2e1_t v2e1;
    int status;
    if (vlan == 0) {
        return BCM_E_PARAM;
    }
    v2e1.vrid0 = 0;
    v2e1.vrid1 = 0;
    v2e1.vrid2 = 0;
    v2e1.vrid3 = 0;
    status = soc_sbx_g2p3_v2e1_set(unit, vlan, &v2e1);
    if (status != BCM_E_NONE) {
        L3_ERR((_SBX_D(unit, "error %s in soc_sbx_g2p3_v2e1_set"),
                bcm_errmsg(status)));
        return status;
    }

    return BCM_E_NONE;
}

int
bcm_fe2000_l3_vrrp_get(int unit,
                       bcm_vlan_t vlan,
                       int alloc_size,
                       int * vrid_array,
                       int * count)
{
    soc_sbx_g2p3_v2e1_t v2e1;
    int status, vrid_count;

    if (vlan == 0) {
        return BCM_E_PARAM;
    }
    if (alloc_size < 4) {
        return BCM_E_PARAM;
    }
    if (vrid_array == NULL) {
        return BCM_E_PARAM;
    }
    if (count == NULL) {
        return BCM_E_PARAM;
    }

    status = soc_sbx_g2p3_v2e1_get (unit, vlan, &v2e1);
    if (status != BCM_E_NONE) {
        L3_ERR((_SBX_D(unit, "error %s in soc_sbx_g2p3_v2e1_get"),
                bcm_errmsg(status)));
        return status;
    }

    vrid_count = 0;
    if (v2e1.vrid0 != 0) {
        vrid_array[vrid_count] = v2e1.vrid0;
        vrid_count++;
    }
    if (v2e1.vrid1 != 0) {
        vrid_array[vrid_count] = v2e1.vrid1;
        vrid_count++;
    }
    if (v2e1.vrid2 != 0) {
        vrid_array[vrid_count] = v2e1.vrid2;
        vrid_count++;
    }
    if (v2e1.vrid3 != 0) {
        vrid_array[vrid_count] = v2e1.vrid3;
        vrid_count++;
    }
    *count = vrid_count;

    return BCM_E_NONE;
}
/* VRRP APIs END */

int
bcm_fe2000_l3_ip6_prefix_map_get(int unit,
                              int map_size,
                              bcm_ip6_t *ip6_array,
                              int *ip6_count)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_l3_ip6_prefix_map_add(int unit,
                              bcm_ip6_t ip6_addr)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_l3_ip6_prefix_map_delete(int unit,
                                 bcm_ip6_t ip6_addr)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_l3_ip6_prefix_map_delete_all(int unit)
{
    return BCM_E_UNAVAIL;
}

int bcm_fe2000_l3_egress_stat_get(int unit, bcm_l3_egress_t *egr,
                           bcm_l3_stat_t stat, uint64 *val)
{
    return BCM_E_UNAVAIL;
}

int bcm_fe2000_l3_egress_stat_get32(int unit, bcm_l3_egress_t *egr,
                             bcm_l3_stat_t stat, uint32 *val)
{
    return BCM_E_UNAVAIL;
}

int bcm_fe2000_l3_egress_stat_set(int unit, bcm_l3_egress_t *egr,
                           bcm_l3_stat_t stat, uint64 val)
{
    return BCM_E_UNAVAIL;
}

int bcm_fe2000_l3_egress_stat_set32(int unit, bcm_l3_egress_t *egr,
                             bcm_l3_stat_t stat, uint32 val)
{
    return BCM_E_UNAVAIL;
}

int bcm_fe2000_l3_egress_stat_enable_set(int unit, bcm_l3_host_t *egr,
                                  int enable)
{
    return BCM_E_UNAVAIL;
}

int bcm_fe2000_l3_host_stat_get(int unit, bcm_l3_host_t *host,
                         bcm_l3_stat_t stat, uint64 *val)
{
    return BCM_E_UNAVAIL;
}

int bcm_fe2000_l3_host_stat_get32(int unit, bcm_l3_host_t *host,
                           bcm_l3_stat_t stat, uint32 *val)
{
    return BCM_E_UNAVAIL;
}

int bcm_fe2000_l3_host_stat_set(int unit, bcm_l3_host_t *host,
                         bcm_l3_stat_t stat, uint64 val)
{
    return BCM_E_UNAVAIL;
}

int bcm_fe2000_l3_host_stat_set32(int unit, bcm_l3_host_t *host,
                           bcm_l3_stat_t stat, uint32 val)
{
    return BCM_E_UNAVAIL;
}

int bcm_fe2000_l3_host_stat_enable_set(int unit, bcm_l3_host_t *host,
                                int enable)
{
    return BCM_E_UNAVAIL;
}

int bcm_fe2000_l3_route_stat_get(int unit, bcm_l3_route_t *route,
                          bcm_l3_stat_t stat, uint64 *val)
{
    return BCM_E_UNAVAIL;
}

int bcm_fe2000_l3_route_stat_get32(int unit, bcm_l3_route_t *route,
                            bcm_l3_stat_t stat, uint32 *val)
{
    return BCM_E_UNAVAIL;
}

int bcm_fe2000_l3_route_stat_set(int unit, bcm_l3_route_t *route,
                          bcm_l3_stat_t stat, uint64 val)
{
    return BCM_E_UNAVAIL;
}

int bcm_fe2000_l3_route_stat_set32(int unit, bcm_l3_route_t *route,
                            bcm_l3_stat_t stat, uint32 val)
{
    return BCM_E_UNAVAIL;
}

int bcm_fe2000_l3_route_stat_enable_set(int unit, bcm_l3_route_t *route,
                                 int enable)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_l3_defip_delete_all(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_l3_egress_multipath_traverse(int unit,
                                        bcm_l3_egress_multipath_traverse_cb trav_fn,
                                        void *user_data)
{
    return BCM_E_UNAVAIL;
}


/* Provided an L3 Interface, reference it. Typically used in cases as 
 * associating OAM on mpls tunnel interface. This will ensure the mpls
 * tunnel is not deleted before the OAM is disassociated 
 * reference > 0 -> References the interface
 * reference <= 0 -> Dereferences the interface
 */
int 
_bcm_fe2000_l3_reference_interface(int unit, bcm_if_t intf, int reference)
{
    _fe2k_l3_fe_instance_t *l3_fe;    
    _fe2k_l3_fte_t         *fte_hash_elem;
    int status = BCM_E_NONE;

    BCM_IF_ERROR_RETURN(L3_LOCK(unit));

    l3_fe = _bcm_fe2000_get_l3_instance_from_unit(unit);
    if (!l3_fe) {
        status = BCM_E_UNIT;

    } else {
        status = _bcm_fe2000_l3_get_sw_fte(l3_fe, 
                                           _FE2K_GET_FTE_IDX_FROM_USER_HANDLE(intf),
                                           &fte_hash_elem);
        if (BCM_FAILURE(status)) {
            L3_ERR((_SBX_D(l3_fe->fe_unit, "fte_idx 0x%x not found in hash table\n"), 
                    _FE2K_GET_FTE_IDX_FROM_USER_HANDLE(intf)));
            status = BCM_E_NOT_FOUND;

        } else {
            if (reference > 0) {
                fte_hash_elem->ref_count++;
            } else {
                fte_hash_elem->ref_count--;
            }

            L3_VERB((_SBX_D(l3_fe->fe_unit, "L3 Interface 0x%x fte_idx 0x%x referenced Count[%d]\n"), 
                     intf,_FE2K_GET_FTE_IDX_FROM_USER_HANDLE(intf), 
                     fte_hash_elem->ref_count));
        }
    }

    L3_UNLOCK(unit);
    return status;
}

#endif  /* INCLUDE_L3 */
