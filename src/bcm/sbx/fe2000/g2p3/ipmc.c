/*
 * $Id: ipmc.c 1.16 Broadcom SDK $
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

#ifdef BCM_FE2000_P3_SUPPORT
#include <soc/defs.h>


#include <soc/drv.h>
#include <bcm/stack.h>
#include <bcm/vlan.h>
#include <bcm/trunk.h>

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/fe2000.h>

#include <soc/sbx/g2p3/g2p3.h>

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
#include <bcm_int/sbx/fe2000/vlan.h>
#include <bcm_int/sbx/fe2000/allocator.h>
#include <bcm_int/sbx/fe2000/l3.h>

#include <appl/diag/system.h>
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
/* #define IPMC_VVERB(stuff)         IPMC_DEBUG(BCM_DBG_VVERBOSE, stuff) */
#define IPMC_VVERB(stuff)         

/* externs */
extern int
_bcm_fe2000_find_ipmc_route(int                      unit,
                            _fe2k_l3_fe_instance_t  *l3_fe,
                            int                      ipmc_index,
                            _fe2k_ipmc_route_t     **ipmc_rt);
extern void
_bcm_fe2000_insert_ipmc_route(_fe2k_l3_fe_instance_t *l3_fe,
                              int                     ipmc_index,
                              _fe2k_ipmc_route_t     *ipmc_rt);
extern int
_bcm_fe2000_alloc_ipmc_route(_fe2k_l3_fe_instance_t   *l3_fe,
                             _fe2k_ipmc_route_t      **ipmc_rt);
extern void
_bcm_fe2000_free_ipmc_route(_fe2k_l3_fe_instance_t *l3_fe,
                            _fe2k_ipmc_route_t **ipmc_rt);
extern int
_bcm_fe2000_ipmc_delete_all_reference_to_ipmc_index(_fe2k_l3_fe_instance_t *l3_fe,
                                    int ipmc_index);
/* locals */

void bcm_l3_ip6_mcify(bcm_ip6_t addr)
{
    /* Does not check if the addr is mcast,
     * so that there is no need to maintain if addr
     * is already mcified
     */
    BCM_IP6_BYTE(addr, 0) = 0;
}

int bcm_l3_ip6_zero_addr(bcm_ip6_t addr)
{
    bcm_ip6_t zero;
    sal_memset(zero, 0, sizeof(bcm_ip6_t));
    return (BCM_IP6_ADDR_EQ(addr, zero));
}

void
_bcm_fe2000_g2p3_map_ipmc_fte(int                      unit,
                         _fe2k_l3_fe_instance_t  *l3_fe,
                         bcm_ipmc_addr_t         *data,
                         soc_sbx_g2p3_ft_t         *fte,
                         int                      update);

/*
 * Function:
 *      _bcm_fe2000_find_route
 * Description:
 *      find route from ipmc rt hash based on (s,g,vid)
 *      s is zero for (*,g,vid)
 * Parameters:
 *      unit       - fe2k unit
 *      l3_fe      - L3 fe instance
 *      fe         - SB G2 Fe ptr
 *      data       - ipmc (s, g, vid) info
 *      ipmc_rt    - ipmc route 
 * Returns:
 *      BCM_E_NONE - on success
 *      BCM_E_NOT_FOUND - if not found
 * Notes:
 * Assumption:
 *      User will always do BCM_IPMC_USE_IPMC_INDEX. User
 *      needs to and will manage ipmc_index globally
 *      Hence this code is non-optimized.
 */

int
_bcm_fe2000_g2p3_ipv6mc_find_route(int                      unit,
                                  _fe2k_l3_fe_instance_t  *l3_fe,
                                  soc_sbx_g2p3_state_t    *fe,
                                  bcm_ipmc_addr_t         *data,
                                  _fe2k_ipmc_route_t     **ipmc_rt,
                                  soc_sbx_g2p3_ipv6mcg_t  *ipv6mcg,
                                  soc_sbx_g2p3_ipv6mcsg_t  *ipv6mcsg,
                                  soc_sbx_g2p3_ft_t         *fte)
{
    sbStatus_t              sb_ret;
    int                     status;
    int                     flag_null_s_addr = 0;
    uint32                     ftidx;
    uint32                  mc_vsi = 0;
    soc_sbx_g2p3_pv2e_t     pv2e;
    
    flag_null_s_addr = bcm_l3_ip6_zero_addr(data->s_ip6_addr);
    bcm_l3_ip6_mcify(data->mc_ip6_addr);

    status  = SOC_SBX_G2P3_PV2E_GET(l3_fe->fe_unit, data->port_tgid, data->vid, &pv2e);
    if( status != BCM_E_NONE ) {
      IPMC_ERR(("unit %d: [%s] error(%s) obtaining pv2e(port-0x%x, vid-0x%x)\n",
                 unit, FUNCTION_NAME(),
                 bcm_errmsg(status), data->port_tgid, data->vid));
      return status; 
    }

    mc_vsi = pv2e.vlan;
    /* Mask out the Top byte */
    
    if (flag_null_s_addr) {
        sb_ret = soc_sbx_g2p3_ipv6mcg_get(unit,
                                data->mc_ip6_addr,
                                mc_vsi, ipv6mcg);
    } else {
        sb_ret = soc_sbx_g2p3_ipv6mcsg_get(unit,
                                data->mc_ip6_addr,
                                data->s_ip6_addr,
                                mc_vsi, ipv6mcsg);
                               
    }
    if (sb_ret != SB_OK) {
        status = translate_sbx_result(sb_ret);
        /* caller will log the error if reqd */
        IPMC_VERB(("unit %d: [%s] error(%s) returned from soc_sbx_g2p3_ipv6mcxxcomp_get()\n",
                  unit, FUNCTION_NAME(),
                  bcm_errmsg(status)));
        return status;
    }
    BCM_IF_ERROR_RETURN
       (soc_sbx_g2p3_vlan_ft_base_get(unit, &ftidx));

    ftidx += (flag_null_s_addr) ? ipv6mcg->ftidx : ipv6mcsg->ftidx;
    sb_ret = soc_sbx_g2p3_ft_get(unit,
                            ftidx,
                            fte);
    if (sb_ret != SB_OK) {
        status = translate_sbx_result(sb_ret);
        IPMC_ERR(("unit %d: [%s] error(%s) calling soc_sbx_g2p3_ft_get\n",
                  unit, FUNCTION_NAME(),
                  bcm_errmsg(status)));
        return status;
    }

    data->ipmc_index = fte->oi;

    status = 
        _bcm_fe2000_find_ipmc_route(unit,
                                    l3_fe,
                                    data->ipmc_index,
                                    ipmc_rt);
    
    IPMC_VERB(("unit %d: [%s] %s ipmc-rt with s(" IPV6_FMT ") g(" IPV6_FMT ") vid(%d)\n",
               unit, FUNCTION_NAME(),
               (status != BCM_E_NONE) ? "not-found" : "found",
               IPV6_PFMT(data->s_ip6_addr),
               IPV6_PFMT(data->mc_ip6_addr), data->vid));
    
    return status;
}

int
_bcm_fe2000_g2p3_ipv4mc_find_route(int                      unit,
                                  _fe2k_l3_fe_instance_t  *l3_fe,
                                  soc_sbx_g2p3_state_t    *fe,
                                  bcm_ipmc_addr_t         *data,
                                  _fe2k_ipmc_route_t     **ipmc_rt,
                                  soc_sbx_g2p3_ipv4mcg_t  *ipv4mcg,
                                  soc_sbx_g2p3_ipv4mcsg_t  *ipv4mcsg,
                                  soc_sbx_g2p3_ft_t         *fte)
{
    sbStatus_t              sb_ret;
    int                     status;
    int                     flag_null_s_addr = 0;
    uint32                     ftidx;
    uint32                  mc_vsi = 0;
    soc_sbx_g2p3_pv2e_t     pv2e;
    
    flag_null_s_addr = (data->s_ip_addr == 0)?1:0;

    status  = SOC_SBX_G2P3_PV2E_GET(l3_fe->fe_unit, data->port_tgid, data->vid, &pv2e);
    if( status != BCM_E_NONE ) {
      IPMC_ERR(("unit %d: [%s] error(%s) obtaining pv2e(port-0x%x, vid-0x%x)\n",
                 unit, FUNCTION_NAME(),
                 bcm_errmsg(status), data->port_tgid, data->vid));
      return status; 
    }

    mc_vsi = pv2e.vlan;
    
    if (flag_null_s_addr) {
        sb_ret = soc_sbx_g2p3_ipv4mcg_get(unit,
                                (data->mc_ip_addr & 0x0fffffff),
                                mc_vsi, ipv4mcg);
    } else {
        sb_ret = soc_sbx_g2p3_ipv4mcsg_get(unit,
                                (data->mc_ip_addr & 0x0fffffff),
                                data->s_ip_addr,
                                mc_vsi, ipv4mcsg);
                               
    }
    if (sb_ret != SB_OK) {
        status = translate_sbx_result(sb_ret);
        /* caller will log the error if reqd */
        IPMC_VERB(("unit %d: [%s] error(%s) returned from soc_sbx_g2p3_ipv4mcxxcomp_get()\n",
                  unit, FUNCTION_NAME(),
                  bcm_errmsg(status)));
        return status;
    }
    BCM_IF_ERROR_RETURN
       (soc_sbx_g2p3_vlan_ft_base_get(unit, &ftidx));

    ftidx += (flag_null_s_addr) ? ipv4mcg->ftidx : ipv4mcsg->ftidx;
    sb_ret = soc_sbx_g2p3_ft_get(unit,
                            ftidx,
                            fte);
    if (sb_ret != SB_OK) {
        status = translate_sbx_result(sb_ret);
        IPMC_ERR(("unit %d: [%s] error(%s) calling soc_sbx_g2p3_ft_get\n",
                  unit, FUNCTION_NAME(),
                  bcm_errmsg(status)));
        return status;
    }

    data->ipmc_index = fte->oi;

    status = 
        _bcm_fe2000_find_ipmc_route(unit,
                                    l3_fe,
                                    data->ipmc_index,
                                    ipmc_rt);
    
    IPMC_VERB(("unit %d: [%s] %s ipmc-rt with s(0x%08x) g(0x%08x) vid(%d)\n",
               unit, FUNCTION_NAME(),
               (status != BCM_E_NONE) ? "not-found" : "found",
               data->s_ip_addr, data->mc_ip_addr, data->vid));
    
    return status;
}

/*
 * Function:
 *      _bcm_fe2000_ipmc_add
 * Description:
 *      add new ipmc route
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

#ifndef SB_G2_FE_RUN_LENGTH_ALL
#define SB_G2_FE_RUN_LENGTH_ALL 0xffffffffU
#endif

 int
_bcm_fe2000_g2p3_ipv6mc_add(int                     unit,
                          _fe2k_l3_fe_instance_t *l3_fe,
                          bcm_ipmc_addr_t        *data)
{
    soc_sbx_g2p3_state_t             *fe;
    soc_sbx_g2p3_ipv6mcg_t           ipv6mcg = {0};
    soc_sbx_g2p3_ipv6mcsg_t          ipv6mcsg = {0};
    soc_sbx_g2p3_ft_t                 fte;
    uint32_t                          fte_id = ~0;
    sbStatus_t                        sb_ret;
    int                               status, ignore_status;
    _fe2k_ipmc_route_t               *ipmc_rt;
    int                               flag_null_s_addr;
    int                               proccopy;
    soc_sbx_g2p3_pv2e_t               pv2e;
    soc_sbx_g2p3_lp_t                 lp;
    uint32_t                          mc_vsi = 0, 
                                      lp_idx = 0 ;

    fe = (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;


    if (( data->flags & BCM_IPMC_SOURCE_PORT_NOCHECK ) || 
       !(data->flags & BCM_IPMC_IP6)) {
       return BCM_E_PARAM;
    }

    /* Add the RPF related Changes
       1. Obtain the VSI, from Port, Vid.
       2. Lookup the PV2E to get the LP Index
       3. Lookup the LP Table to get the PID value
       4. Configure the Payload.rpfunion with the PID value.
       */
    status  = SOC_SBX_G2P3_PV2E_GET(l3_fe->fe_unit, 
                                    data->port_tgid, data->vid, &pv2e);
    if( status != BCM_E_NONE ) {
       IPMC_ERR(("unit %d: [%s] error(%s) obtaining pv2e(port-0x%x, vid-0x%x)\n",
                  unit, FUNCTION_NAME(),
                  bcm_errmsg(status), data->port_tgid, data->vid));
       return status; 
    }

    mc_vsi = pv2e.vlan;

    /* In case the Pv2e Entry is not configured with LP entry, 
     * that would indicate an physical port 
     */
    if( pv2e.lpi == 0 ) {
        lp_idx = data->port_tgid;
    } else {
        lp_idx = pv2e.lpi;
    }
    
    status  = soc_sbx_g2p3_lp_get(l3_fe->fe_unit, lp_idx, &lp);

    if (status != BCM_E_NONE) {
       IPMC_ERR(("unit %d: [%s] error(%s) obtaining lp(index-0x%x)\n",
                  unit, FUNCTION_NAME(),
                  bcm_errmsg(status), lp_idx));
       return status; 
    }

    /* BCM_IPMC_USE_IPMC_INDEX is always true here */
    status = _bcm_fe2000_find_ipmc_route(unit,
                                         l3_fe,
                                         data->ipmc_index,
                                         &ipmc_rt);
    switch (status) {
    case BCM_E_NOT_FOUND:
        break;
    case BCM_E_NONE:
        IPMC_ERR(("unit %d: [%s] duplicate ipmc-add\n",
                  unit, FUNCTION_NAME()));
        return BCM_E_EXISTS;
    default:
        IPMC_ERR(("unit %d: [%s] error(%s) ipmc-index(0x%x)\n",
                  unit, FUNCTION_NAME(),
                  bcm_errmsg(status), data->ipmc_index));
        return BCM_E_INTERNAL;
    }
 

    status = _bcm_fe2000_alloc_ipmc_route(l3_fe,
                                          &ipmc_rt);
    if (status != BCM_E_NONE) {
        IPMC_ERR(("unit %d: [%s] error(%s) unable to allocate ipmc-rt\n",
                  unit, FUNCTION_NAME(),
                  bcm_errmsg(status)));
        return status;
    }
    
    status = _sbx_gu2_resource_alloc(unit,
                                     SBX_GU2K_USR_RES_FTE_IPMC,
                                     1,
                                     &fte_id,
                                     0);
    if (status != BCM_E_NONE) {
        IPMC_ERR(("unit %d: [%s] failure(%s) to allocate new fte\n",
                  unit, FUNCTION_NAME(), bcm_errmsg(status)));
        _bcm_fe2000_free_ipmc_route(l3_fe, &ipmc_rt);
        return status;
    }


    proccopy = (data->flags & BCM_IPMC_COPYTOCPU)?1:0;
    bcm_l3_ip6_mcify(data->mc_ip6_addr);
    flag_null_s_addr = bcm_l3_ip6_zero_addr(data->s_ip6_addr);
    if (flag_null_s_addr) {
        ipv6mcg.proccopy = proccopy;
        ipv6mcg.rpfunion = lp.pid;
        ipv6mcg.ftidx = fte_id;
    } else {
        ipv6mcsg.proccopy = proccopy;
        ipv6mcsg.rpfunion = lp.pid;
        ipv6mcsg.ftidx = fte_id;
    }
    
    _bcm_fe2000_g2p3_map_ipmc_fte(unit,
                             l3_fe,
                             data,
                             &fte,
                             FALSE);

    sb_ret = soc_sbx_g2p3_ft_set(unit, fte_id, &fte);
                               
    if (sb_ret != SB_OK) {
        status = translate_sbx_result(sb_ret);
        IPMC_ERR(("unit %d: [%s] error(%s) setting fte(0x%x)\n",
                  unit, FUNCTION_NAME(),
                  bcm_errmsg(status), fte_id));
      
        /* free ipmc-rt, fte-id */
        _bcm_fe2000_free_ipmc_route(l3_fe, &ipmc_rt);
        ignore_status = _sbx_gu2_resource_free(unit,
                                               SBX_GU2K_USR_RES_FTE_IPMC,
                                               1,
                                               &fte_id,
                                               0);
        if (ignore_status != BCM_E_NONE) {
            IPMC_ERR(("unit %d: [%s] error(%s) freeing fte(0x%x)\n",
                      unit, FUNCTION_NAME(),
                      bcm_errmsg(status), fte_id));
        }
        
        return status;
    }

    IPMC_VERB(("unit %d: [%s] ipmc-index(0x%x) fte-idx 0x%x "
               "QidLagUnion(0x%x) OI(0x%x)\n",
               unit, FUNCTION_NAME(),
               data->ipmc_index, fte_id,
               fte.qid, fte.oi));
               
    ipmc_rt->ipmc_user_info = *data;
    
    _bcm_fe2000_insert_ipmc_route(l3_fe,
                                  data->ipmc_index,
                                  ipmc_rt);
    if (flag_null_s_addr) {
        if (SOC_SBX_STATE(unit)->cache_ipmc == FALSE) {
            sb_ret = soc_sbx_g2p3_ipv6mcg_set(unit,
                                              data->mc_ip6_addr,
                                              mc_vsi, 
                                              &ipv6mcg);
        } else {
            sb_ret = soc_sbx_g2p3_ipv6mcg_add(unit,
                                              data->mc_ip6_addr,
                                              mc_vsi, 
                                              &ipv6mcg);
        }
    } else {
        if (SOC_SBX_STATE(unit)->cache_ipmc == FALSE) {
            sb_ret = soc_sbx_g2p3_ipv6mcsg_set(unit,
                                               data->mc_ip6_addr,
                                               data->s_ip6_addr,
                                               mc_vsi, 
                                               &ipv6mcsg);
        } else {

            sb_ret = soc_sbx_g2p3_ipv6mcsg_add(unit,
                                               data->mc_ip6_addr,
                                               data->s_ip6_addr,
                                               mc_vsi, 
                                               &ipv6mcsg);
        }
    }
        
    if (sb_ret != SB_OK) {
        status = translate_sbx_result(sb_ret);
        IPMC_ERR(("unit %d: [%s] error(%s) adding ipmc-flow(0x%x)\n",
                  unit, FUNCTION_NAME(),
                  bcm_errmsg(status), data->ipmc_index));
        
        /* free ipmc-rt, fte-id */
        _bcm_fe2000_free_ipmc_route(l3_fe, &ipmc_rt);
        ignore_status = _sbx_gu2_resource_free(unit,
                                               SBX_GU2K_USR_RES_FTE_IPMC,
                                               1,
                                               &fte_id,
                                               0);
        if (ignore_status != BCM_E_NONE) {
            IPMC_ERR(("unit %d: [%s] error(%s) freeing fte(0x%x)\n",
                      unit, FUNCTION_NAME(),
                      bcm_errmsg(status), fte_id));
            /* try other resource free .. */
        }

        return status;
    }
    
    return BCM_E_NONE;
}

 int
_bcm_fe2000_g2p3_ipv4mc_add(int                     unit,
                          _fe2k_l3_fe_instance_t *l3_fe,
                          bcm_ipmc_addr_t        *data)
{
    soc_sbx_g2p3_state_t             *fe;
    soc_sbx_g2p3_ipv4mcg_t           ipv4mcg = {0};
    soc_sbx_g2p3_ipv4mcsg_t          ipv4mcsg = {0};
    soc_sbx_g2p3_ft_t                 fte;
    uint32_t                          fte_id = ~0;
    sbStatus_t                        sb_ret;
    int                               status, ignore_status;
    _fe2k_ipmc_route_t               *ipmc_rt;
    int                               flag_null_s_addr;
    int                               proccopy;
    soc_sbx_g2p3_pv2e_t               pv2e;
    soc_sbx_g2p3_lp_t                 lp;
    uint32_t                          mc_vsi = 0, 
                                      lp_idx = 0 ;

    fe = (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;


    if ((data->flags & BCM_IPMC_SOURCE_PORT_NOCHECK ) || 
        (data->flags & BCM_IPMC_IP6)) {
       return BCM_E_PARAM;
    }

    /* Add the RPF related Changes
       1. Obtain the VSI, from Port, Vid.
       2. Lookup the PV2E to get the LP Index
       3. Lookup the LP Table to get the PID value
       4. Configure the Payload.rpfunion with the PID value.
       */
    status  = SOC_SBX_G2P3_PV2E_GET(l3_fe->fe_unit, 
                                    data->port_tgid, data->vid, &pv2e);
    if( status != BCM_E_NONE ) {
       IPMC_ERR(("unit %d: [%s] error(%s) obtaining pv2e(port-0x%x, vid-0x%x)\n",
                  unit, FUNCTION_NAME(),
                  bcm_errmsg(status), data->port_tgid, data->vid));
       return status; 
    }

    mc_vsi = pv2e.vlan;

    /* In case the Pv2e Entry is not configured with LP entry, 
     * that would indicate an physical port 
     */
    if( pv2e.lpi == 0 ) {
        lp_idx = data->port_tgid;
    } else {
        lp_idx = pv2e.lpi;
    }
    
    status  = soc_sbx_g2p3_lp_get(l3_fe->fe_unit, lp_idx, &lp);

    if (status != BCM_E_NONE) {
       IPMC_ERR(("unit %d: [%s] error(%s) obtaining lp(index-0x%x)\n",
                  unit, FUNCTION_NAME(),
                  bcm_errmsg(status), lp_idx));
       return status; 
    }

    /* BCM_IPMC_USE_IPMC_INDEX is always true here */
    status = _bcm_fe2000_find_ipmc_route(unit,
                                         l3_fe,
                                         data->ipmc_index,
                                         &ipmc_rt);
    switch (status) {
    case BCM_E_NOT_FOUND:
        break;
    case BCM_E_NONE:
        IPMC_ERR(("unit %d: [%s] duplicate ipmc-add\n",
                  unit, FUNCTION_NAME()));
        return BCM_E_EXISTS;
    default:
        IPMC_ERR(("unit %d: [%s] error(%s) ipmc-index(0x%x)\n",
                  unit, FUNCTION_NAME(),
                  bcm_errmsg(status), data->ipmc_index));
        return BCM_E_INTERNAL;
    }
 
    status = _bcm_fe2000_alloc_ipmc_route(l3_fe,
                                          &ipmc_rt);
    if (status != BCM_E_NONE) {
        IPMC_ERR(("unit %d: [%s] error(%s) unable to allocate ipmc-rt\n",
                  unit, FUNCTION_NAME(),
                  bcm_errmsg(status)));
        return status;
    }
    
    status = _sbx_gu2_resource_alloc(unit,
                                     SBX_GU2K_USR_RES_FTE_IPMC,
                                     1,
                                     &fte_id,
                                     0);
    if (status != BCM_E_NONE) {
        IPMC_ERR(("unit %d: [%s] failure(%s) to allocate new fte\n",
                  unit, FUNCTION_NAME(), bcm_errmsg(status)));
        _bcm_fe2000_free_ipmc_route(l3_fe, &ipmc_rt);
        return status;
    }


    proccopy = (data->flags & BCM_IPMC_COPYTOCPU)?1:0;
    flag_null_s_addr = (data->s_ip_addr == 0)?1:0;
    if (flag_null_s_addr) {
        ipv4mcg.proccopy = proccopy;
        ipv4mcg.rpfunion = lp.pid;
        ipv4mcg.ftidx = fte_id;
    } else {
        ipv4mcsg.proccopy = proccopy;
        ipv4mcsg.rpfunion = lp.pid;
        ipv4mcsg.ftidx = fte_id;
    }
    
    _bcm_fe2000_g2p3_map_ipmc_fte(unit,
                             l3_fe,
                             data,
                             &fte,
                             FALSE);

    sb_ret = soc_sbx_g2p3_ft_set(unit, fte_id, &fte);
                               
    if (sb_ret != SB_OK) {
        status = translate_sbx_result(sb_ret);
        IPMC_ERR(("unit %d: [%s] error(%s) setting fte(0x%x)\n",
                  unit, FUNCTION_NAME(),
                  bcm_errmsg(status), fte_id));
      
        /* free ipmc-rt, fte-id */
        _bcm_fe2000_free_ipmc_route(l3_fe, &ipmc_rt);
        ignore_status = _sbx_gu2_resource_free(unit,
                                               SBX_GU2K_USR_RES_FTE_IPMC,
                                               1,
                                               &fte_id,
                                               0);
        if (ignore_status != BCM_E_NONE) {
            IPMC_ERR(("unit %d: [%s] error(%s) freeing fte(0x%x)\n",
                      unit, FUNCTION_NAME(),
                      bcm_errmsg(status), fte_id));
        }
        
        return status;
    }

    IPMC_VERB(("unit %d: [%s] ipmc-index(0x%x) fte-idx 0x%x "
               "QidLagUnion(0x%x) OI(0x%x)\n",
               unit, FUNCTION_NAME(),
               data->ipmc_index, fte_id,
               fte.qid, fte.oi));
               
    ipmc_rt->ipmc_user_info = *data;
    
    _bcm_fe2000_insert_ipmc_route(l3_fe,
                                  data->ipmc_index,
                                  ipmc_rt);
    if (flag_null_s_addr) {
        
        if (SOC_SBX_STATE(unit)->cache_ipmc == FALSE) {
            sb_ret = soc_sbx_g2p3_ipv4mcg_set(unit,
                                              (data->mc_ip_addr & 0x0fffffff),
                                              mc_vsi, 
                                              &ipv4mcg);
         } else {

            sb_ret = soc_sbx_g2p3_ipv4mcg_add(unit,
                                              (data->mc_ip_addr & 0x0fffffff),
                                              mc_vsi, 
                                              &ipv4mcg);
         }
    } else {
        if (SOC_SBX_STATE(unit)->cache_ipmc == FALSE) {
            sb_ret = soc_sbx_g2p3_ipv4mcsg_set(unit,
                                               (data->mc_ip_addr & 0x0fffffff),
                                               data->s_ip_addr,
                                               mc_vsi, 
                                               &ipv4mcsg);

         } else {
            sb_ret = soc_sbx_g2p3_ipv4mcsg_add(unit,
                                               (data->mc_ip_addr & 0x0fffffff),
                                               data->s_ip_addr,
                                               mc_vsi, 
                                               &ipv4mcsg);
         }
    }
        
    if (sb_ret != SB_OK) {
        status = translate_sbx_result(sb_ret);
        IPMC_ERR(("unit %d: [%s] error(%s) adding ipmc-flow(0x%x)\n",
                  unit, FUNCTION_NAME(),
                  bcm_errmsg(status), data->ipmc_index));
        
        /* free ipmc-rt, fte-id */
        _bcm_fe2000_free_ipmc_route(l3_fe, &ipmc_rt);
        ignore_status = _sbx_gu2_resource_free(unit,
                                               SBX_GU2K_USR_RES_FTE_IPMC,
                                               1,
                                               &fte_id,
                                               0);
        if (ignore_status != BCM_E_NONE) {
            IPMC_ERR(("unit %d: [%s] error(%s) freeing fte(0x%x)\n",
                      unit, FUNCTION_NAME(),
                      bcm_errmsg(status), fte_id));
            /* try other resource free .. */
        }

        return status;
    }
   
    return BCM_E_NONE;
}

 int
_bcm_fe2000_g2p3_ipmc_add(int                     unit,
                          _fe2k_l3_fe_instance_t *l3_fe,
                          bcm_ipmc_addr_t        *data)
{
    int status;
    if (data->flags & BCM_IPMC_IP6) {
        status = _bcm_fe2000_g2p3_ipv6mc_add(unit, l3_fe, data); 
    } else {
        status = _bcm_fe2000_g2p3_ipv4mc_add(unit, l3_fe, data);
    }
    return status;
}

/*
 * Function:
 *      _bcm_fe2000_ipmc_replace
 * Description:
 *      replace the ipmc route with new information
 * Parameters:
 *      unit       - FE unit
 *      data       - new info for ipmc route
 *      l3_fe      - L3 fe instance
 *      fe         - SB fe ptr
 * Returns:
 *      BCM_E_NONE - on success
 *      BCM_E_XXX  - on failure
 * Notes:
 * Assumption:
 */
 int
_bcm_fe2000_g2p3_ipv6mc_replace(int                     unit,
                              _fe2k_l3_fe_instance_t *l3_fe,
                              bcm_ipmc_addr_t        *data)
{
    soc_sbx_g2p3_state_t            *fe;
    soc_sbx_g2p3_ft_t                fte;    
    _fe2k_ipmc_route_t              *ipmc_rt;
    soc_sbx_g2p3_ipv6mcg_t           ipv6mcg;
    soc_sbx_g2p3_ipv6mcsg_t          ipv6mcsg;
    sbStatus_t                       sb_ret;
    int                              status;
    int                              flag_null_s_addr;
    uint32                           ftidx = ~0;    
    soc_sbx_g2p3_pv2e_t              pv2e;    
    uint32                           mc_vsi = 0;

    fe = (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
 

    status  = SOC_SBX_G2P3_PV2E_GET(l3_fe->fe_unit, data->port_tgid, data->vid, &pv2e);
    if( status != BCM_E_NONE ) {
       IPMC_ERR(("unit %d: [%s] error(%s) obtaining pv2e(port-0x%x, vid-0x%x)\n",
                  unit, FUNCTION_NAME(),
                  bcm_errmsg(status), data->port_tgid, data->vid));
       return status; 
    }

    mc_vsi = pv2e.vlan;

    status = _bcm_fe2000_find_ipmc_route(unit,
                                         l3_fe,
                                         data->ipmc_index,
                                         &ipmc_rt);
    if (status != BCM_E_NONE) {
        IPMC_ERR(("unit %d: [%s] ipmc-rt(%d) not found\n",
                  unit, FUNCTION_NAME(),
                  data->ipmc_index));
        return status;
    }
    
    flag_null_s_addr = bcm_l3_ip6_zero_addr(data->s_ip6_addr);
    bcm_l3_ip6_mcify(data->mc_ip6_addr);
    
    if (flag_null_s_addr) {
        sb_ret = soc_sbx_g2p3_ipv6mcg_get(unit, 
                               data->mc_ip6_addr,
                               mc_vsi, &ipv6mcg);

        if (sb_ret != SB_OK) {
            status = translate_sbx_result(sb_ret);
            IPMC_ERR(("unit %d: [%s] error(%s) calling soc_sbx_g2p3_ipv6mcg_get\n",
                      unit, FUNCTION_NAME(),
                      bcm_errmsg(status)));
            return status;
        }
    } else {
        sb_ret = soc_sbx_g2p3_ipv6mcsg_get(unit,
                                           data->mc_ip6_addr,
                                           data->s_ip6_addr,
                                           mc_vsi, &ipv6mcsg);

        if (sb_ret != SB_OK) {
            status = translate_sbx_result(sb_ret);
            IPMC_ERR(("unit %d: [%s] error(%s) calling soc_sbx_g2p3_ipv6mcsg_get\n",
                      unit, FUNCTION_NAME(),
                      bcm_errmsg(status)));
            return status;
        }
    }
   
    BCM_IF_ERROR_RETURN(soc_sbx_g2p3_vlan_ft_base_get(unit, &ftidx)); 
    ftidx += (flag_null_s_addr) ? ipv6mcg.ftidx : ipv6mcsg.ftidx;
    sb_ret = soc_sbx_g2p3_ft_get(unit, ftidx, &fte);

    if (sb_ret != SB_OK) {
        status = translate_sbx_result(sb_ret);
        IPMC_ERR(("unit %d: [%s] error(%s) calling soc_sbx_g2p3_ft_get\n",
                  unit, FUNCTION_NAME(),
                  bcm_errmsg(status)));
        return status;
    }
    
    /**
     * update ipmc-index
     */
    if (data->ipmc_index != fte.oi) {
        DQ_REMOVE(&ipmc_rt->id_link);
        _bcm_fe2000_insert_ipmc_route(l3_fe,
                                      data->ipmc_index,
                                      ipmc_rt);
    }

    /* Update does not require commit */
    if (flag_null_s_addr) {
        sb_ret = soc_sbx_g2p3_ipv6mcg_update(unit,
                                             data->mc_ip6_addr,
                                             mc_vsi, &ipv6mcg);

        if (sb_ret != SB_OK) {
            status = translate_sbx_result(sb_ret);
            IPMC_ERR(("unit %d: [%s] error(%s) calling soc_sbx_g2p3_ipv6mcg_update for ipmc-index(0x%x)\n",
                      unit, FUNCTION_NAME(),
                      bcm_errmsg(status), data->ipmc_index));
            return status;
        }
    } else {
        sb_ret = soc_sbx_g2p3_ipv6mcsg_update(unit,
                                              data->mc_ip6_addr, 
                                              data->s_ip6_addr, 
                                              mc_vsi, &ipv6mcsg);
        if (sb_ret != SB_OK) {
            status = translate_sbx_result(sb_ret);
            IPMC_ERR(("unit %d: [%s] error(%s) calling soc_sbx_g2p3_ipv6mcsg_update for ipmc-index(0x%x)\n",
                      unit, FUNCTION_NAME(),
                      bcm_errmsg(status), data->ipmc_index));
            return status;
        }
    }
    
    _bcm_fe2000_g2p3_map_ipmc_fte(unit,
                             l3_fe,
                             data,
                             &fte,
                             TRUE);

    ftidx = (flag_null_s_addr) ? ipv6mcg.ftidx : ipv6mcsg.ftidx;
    sb_ret = soc_sbx_g2p3_ft_set(unit,
                                 ftidx,
                                 &fte);
    if (sb_ret != SB_OK) {
        status = translate_sbx_result(sb_ret);
        IPMC_ERR(("unit %d: [%s] error(%s) calling soc_sbx_g2p3_ft_set for ipmc-index(0x%x) fte(0x%x)\n",
                  unit, FUNCTION_NAME(),
                  bcm_errmsg(status),
                  data->ipmc_index, ftidx));
        return status;
    }
#if 0 
    IPMC_VERB(("unit %d: [%s] ipmc-index(0x%x) fte-idx 0x%x valid(%d) drop(%d) "
               "QidLagUnion(0x%x) ulMcGroup(0x%x)\n",
               unit, FUNCTION_NAME(),
               data->ipmc_index, pay4.m_ulFtIdx,
               fte.ulValid, fte.ulDrop, fte.ulQidLagUnion,
               fte.ulMcGroup));
#endif
    
    ipmc_rt->ipmc_user_info = *data;
    
    return BCM_E_NONE;
}

 int
_bcm_fe2000_g2p3_ipv4mc_replace(int                     unit,
                              _fe2k_l3_fe_instance_t *l3_fe,
                              bcm_ipmc_addr_t        *data)
{
    soc_sbx_g2p3_state_t            *fe;
    soc_sbx_g2p3_ft_t                fte;    
    _fe2k_ipmc_route_t              *ipmc_rt;
    soc_sbx_g2p3_ipv4mcg_t           ipv4mcg;
    soc_sbx_g2p3_ipv4mcsg_t          ipv4mcsg;
    sbStatus_t                       sb_ret;
    int                              status;
    int                              flag_null_s_addr;
    uint32                           ftidx = ~0;    
    soc_sbx_g2p3_pv2e_t              pv2e;    
    uint32                           mc_vsi = 0;

    fe = (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;
 

    status  = SOC_SBX_G2P3_PV2E_GET(l3_fe->fe_unit, data->port_tgid, data->vid, &pv2e);
    if( status != BCM_E_NONE ) {
       IPMC_ERR(("unit %d: [%s] error(%s) obtaining pv2e(port-0x%x, vid-0x%x)\n",
                  unit, FUNCTION_NAME(),
                  bcm_errmsg(status), data->port_tgid, data->vid));
       return status; 
    }

    mc_vsi = pv2e.vlan;

    status = _bcm_fe2000_find_ipmc_route(unit,
                                         l3_fe,
                                         data->ipmc_index,
                                         &ipmc_rt);
    if (status != BCM_E_NONE) {
        IPMC_ERR(("unit %d: [%s] ipmc-rt(%d) not found\n",
                  unit, FUNCTION_NAME(),
                  data->ipmc_index));
        return status;
    }
    
    flag_null_s_addr = (data->s_ip_addr)?0:1;
    
    if (flag_null_s_addr) {
        sb_ret = soc_sbx_g2p3_ipv4mcg_get(unit, 
                               (data->mc_ip_addr & 0x0fffffff),
                               mc_vsi, &ipv4mcg);

        if (sb_ret != SB_OK) {
            status = translate_sbx_result(sb_ret);
            IPMC_ERR(("unit %d: [%s] error(%s) calling soc_sbx_g2p3_ipv4mcgcomp_get\n",
                      unit, FUNCTION_NAME(),
                      bcm_errmsg(status)));
            return status;
        }
    } else {
        sb_ret = soc_sbx_g2p3_ipv4mcsg_get(unit,
                                           (data->mc_ip_addr & 0x0fffffff),
                                           data->s_ip_addr,
                                           mc_vsi, &ipv4mcsg);

        if (sb_ret != SB_OK) {
            status = translate_sbx_result(sb_ret);
            IPMC_ERR(("unit %d: [%s] error(%s) calling soc_sbx_g2p3_ipv4mcsgcomp_get\n",
                      unit, FUNCTION_NAME(),
                      bcm_errmsg(status)));
            return status;
        }
    }
   
    BCM_IF_ERROR_RETURN(soc_sbx_g2p3_vlan_ft_base_get(unit, &ftidx)); 
    ftidx += (flag_null_s_addr) ? ipv4mcg.ftidx : ipv4mcsg.ftidx;
    sb_ret = soc_sbx_g2p3_ft_get(unit, ftidx, &fte);

    if (sb_ret != SB_OK) {
        status = translate_sbx_result(sb_ret);
        IPMC_ERR(("unit %d: [%s] error(%s) calling soc_sbx_g2p3_ft_get\n",
                  unit, FUNCTION_NAME(),
                  bcm_errmsg(status)));
        return status;
    }
    
    /**
     * update ipmc-index
     */
    if (data->ipmc_index != fte.oi) {
        DQ_REMOVE(&ipmc_rt->id_link);
        _bcm_fe2000_insert_ipmc_route(l3_fe,
                                      data->ipmc_index,
                                      ipmc_rt);
    }

    /* Update does not require commit */
    if (flag_null_s_addr) {
        sb_ret = soc_sbx_g2p3_ipv4mcg_update(unit,
                                             (data->mc_ip_addr & 0x0fffffff),
                                             mc_vsi, &ipv4mcg);

        if (sb_ret != SB_OK) {
            status = translate_sbx_result(sb_ret);
            IPMC_ERR(("unit %d: [%s] error(%s) calling soc_sbx_g2p3_ipv4mcgcomp_update for ipmc-index(0x%x)\n",
                      unit, FUNCTION_NAME(),
                      bcm_errmsg(status), data->ipmc_index));
            return status;
        }
    } else {
        sb_ret = soc_sbx_g2p3_ipv4mcsg_update(unit,
                                              (data->mc_ip_addr & 0x0fffffff), 
                                              data->s_ip_addr, 
                                              mc_vsi, &ipv4mcsg);
        if (sb_ret != SB_OK) {
            status = translate_sbx_result(sb_ret);
            IPMC_ERR(("unit %d: [%s] error(%s) calling soc_sbx_g2p3_ipv4mcsgcomp_update for ipmc-index(0x%x)\n",
                      unit, FUNCTION_NAME(),
                      bcm_errmsg(status), data->ipmc_index));
            return status;
        }
    }
    
    _bcm_fe2000_g2p3_map_ipmc_fte(unit,
                             l3_fe,
                             data,
                             &fte,
                             TRUE);
    ftidx = (flag_null_s_addr) ? ipv4mcg.ftidx : ipv4mcsg.ftidx;
    sb_ret = soc_sbx_g2p3_ft_set(unit,
                                 ftidx,
                                 &fte);
    if (sb_ret != SB_OK) {
        status = translate_sbx_result(sb_ret);
        IPMC_ERR(("unit %d: [%s] error(%s) calling soc_sbx_g2p3_ft_set for ipmc-index(0x%x) fte(0x%x)\n",
                  unit, FUNCTION_NAME(),
                  bcm_errmsg(status),
                  data->ipmc_index, ftidx));
        return status;
    }
#if 0 
    IPMC_VERB(("unit %d: [%s] ipmc-index(0x%x) fte-idx 0x%x valid(%d) drop(%d) "
               "QidLagUnion(0x%x) ulMcGroup(0x%x)\n",
               unit, FUNCTION_NAME(),
               data->ipmc_index, pay4.m_ulFtIdx,
               fte.ulValid, fte.ulDrop, fte.ulQidLagUnion,
               fte.ulMcGroup));
#endif
    
    ipmc_rt->ipmc_user_info = *data;
    
    return BCM_E_NONE;
}

 int
_bcm_fe2000_g2p3_ipmc_replace(int                     unit,
                              _fe2k_l3_fe_instance_t *l3_fe,
                              bcm_ipmc_addr_t        *data)
{
    int status;
    if (data->flags & BCM_IPMC_IP6) {
        status = _bcm_fe2000_g2p3_ipv6mc_replace(unit, l3_fe, data); 
    } else {
        status = _bcm_fe2000_g2p3_ipv4mc_replace(unit, l3_fe, data);
    }
    return status;
}

/*
 * Function:
 *      _bcm_fe2000_map_ipmc_fte
 * Description:
 *      map various FTE fields
 * Parameters:
 *      unit    - fe unit
 *      data    - IPMC entry information.
 *      fte     - ipmc fte to be filled
 *      update  - TRUE if updating
 * Returns:
 * Assumption:
 */

void
_bcm_fe2000_g2p3_map_ipmc_fte(int                      unit,
                         _fe2k_l3_fe_instance_t  *l3_fe,
                         bcm_ipmc_addr_t         *data,
                         soc_sbx_g2p3_ft_t         *fte,
                         int                      update)
{
    if (update == FALSE) {
        soc_sbx_g2p3_ft_t_init(fte);
    }

    if (data->distribution_class) {
        
        fte->lagbase = (data->distribution_class * SBX_MAX_COS);
        fte->lagsize = 3;
        fte->lag = 1;
    }  else {
        fte->lag = 0;
        fte->lagsize = 0;
        fte->lagbase = 0;
    }

    fte->qid = SBX_MC_QID_BASE;
    fte->oi        = data->ipmc_index;
    fte->mc        = 1;
#if 0 
#ifdef INCLUDE_SBX_HIGIG
    fte->ulDestMod  = SBX_HIGIG_DEST_MOD_IPMC(data->ipmc_index);
    fte->ulDestPort = SBX_HIGIG_DEST_PORT_IPMC(data->ipmc_index);
    fte->ulOpcode   = BCM_HG_OPCODE_IPMC;
#endif
    if (data->flags & BCM_IPMC_SETPRI) {
        fte->ulUseRCos = TRUE;
        /* XXX: How to map cos to ulRCos */
        fte->ulRCos = data->cos;
    }
#endif  
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

 int
_bcm_fe2000_g2p3_ipv6mc_flow_delete(int                     unit,
                             _fe2k_l3_fe_instance_t *l3_fe,
                             bcm_ipmc_addr_t        *data,
                             int                     remove_egress)
{
    soc_sbx_g2p3_state_t *fe;
    soc_sbx_g2p3_ipv6mcg_t ipv6mcg;
    soc_sbx_g2p3_ipv6mcsg_t ipv6mcsg;
    soc_sbx_g2p3_ft_t      fte;
    _fe2k_ipmc_route_t     *ipmc_rt;
    int                     status;
    int                     ipmc_index;
    int                     flag_null_s_addr;
    uint32                  ftidx = ~0;
    soc_sbx_g2p3_pv2e_t     pv2e;
    uint32                  mc_vsi = 0;

    status     = BCM_E_NONE;
    ipmc_index = -1;
    flag_null_s_addr = bcm_l3_ip6_zero_addr(data->s_ip6_addr);
    bcm_l3_ip6_mcify(data->mc_ip6_addr);
  
    fe = (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;

    if (data->flags & BCM_IPMC_USE_IPMC_INDEX) {
        status = _bcm_fe2000_find_ipmc_route(unit,
                                             l3_fe,
                                             data->ipmc_index,
                                             &ipmc_rt);
        if (status != BCM_E_NONE) {
            IPMC_ERR(("unit %d: [%s] unable to find ipmc-rt based on ipmc-index(0x%x)\n",
                      unit, FUNCTION_NAME(), data->ipmc_index));
            return status;
        }

        /**
         * XXX: It is bad that we need to keep the user
         * data for this.
         */
        /* Need this???? if (ipmc_rt->ipmc_user_info.flags & BCM_IPMC_IP6)*/
            sal_memcpy(data->s_ip6_addr, 
                 ipmc_rt->ipmc_user_info.s_ip6_addr, BCM_IP6_ADDRLEN);
            sal_memcpy(data->mc_ip6_addr,
                 ipmc_rt->ipmc_user_info.mc_ip6_addr, BCM_IP6_ADDRLEN);
            mc_vsi = ipmc_rt->ipmc_user_info.vid;

    } else {
       status  = SOC_SBX_G2P3_PV2E_GET(l3_fe->fe_unit, data->port_tgid, data->vid, &pv2e);
       if( status != BCM_E_NONE ) {
          IPMC_ERR(("unit %d: [%s] error(%s) obtaining pv2e(port-0x%x, vid-0x%x)\n",
                     unit, FUNCTION_NAME(),
                     bcm_errmsg(status), data->port_tgid, data->vid));
          return status; 
        }

        mc_vsi = pv2e.vlan;

        status = _bcm_fe2000_g2p3_ipv6mc_find_route(unit,
                                                   l3_fe,
                                                   fe,
                                                   data,
                                                   &ipmc_rt,
                                                   &ipv6mcg,
                                                   &ipv6mcsg,
                                                   &fte);
        if (status != BCM_E_NONE) {
            IPMC_ERR(("unit %d: [%s] (%s) finding ipmc-rt based on <s,g,v>(" IPV6_FMT "," IPV6_FMT ",%d)\n",
                      unit, FUNCTION_NAME(),
                      bcm_errmsg(status),
                      IPV6_PFMT(data->s_ip6_addr),
                      IPV6_PFMT(data->mc_ip6_addr), mc_vsi));
            return status;
        }
    }
    

    if (data->flags & BCM_IPMC_USE_IPMC_INDEX) {    
        if (flag_null_s_addr) {
            status = soc_sbx_g2p3_ipv6mcg_get(unit,
                                              data->mc_ip6_addr,
                                              mc_vsi,
                                              &ipv6mcg);
        } else {
            status = soc_sbx_g2p3_ipv6mcsg_get(unit,
                                               data->mc_ip6_addr,
                                               data->s_ip6_addr, mc_vsi,
                                               &ipv6mcsg);
        }
        if (BCM_FAILURE(status)) {
            IPMC_ERR(("unit %d: [%s] error(%s) calling v6mcxxget(" IPV6_FMT "," IPV6_FMT ",%d)\n",
                      unit, FUNCTION_NAME(), bcm_errmsg(status),
                      IPV6_PFMT(data->s_ip6_addr), 
                      IPV6_PFMT(data->mc_ip6_addr), mc_vsi));
            return status;
        }
    }
    
    /**
     * Remove the LTE for this ipmc flow
     */
    if (flag_null_s_addr) {
        status = soc_sbx_g2p3_ipv6mcg_remove(unit,
                                             data->mc_ip6_addr,
                                             mc_vsi);
                                   
        if (BCM_FAILURE(status)) {
            IPMC_ERR(("unit %d: [%s] error(%s) calling soc_sbx_g2p3_ipv6mcg_remove\n",
                      unit, FUNCTION_NAME(), bcm_errmsg(status)));
            return status;
        }
    } else {
        status = soc_sbx_g2p3_ipv6mcsg_remove(unit,
                                              data->mc_ip6_addr,
                                              data->s_ip6_addr, mc_vsi);
        if (BCM_FAILURE(status)) {
            IPMC_ERR(("unit %d: [%s] error(%s) calling soc_sbx_g2p3_ipv6mcsg_remove\n",
                      unit, FUNCTION_NAME(), bcm_errmsg(status)));
            return status;
        }
    }
    
    _bcm_fe2000_free_ipmc_route(l3_fe, &ipmc_rt);
    
    if (SOC_SBX_STATE(unit)->cache_ipmc == FALSE) {
        if (flag_null_s_addr) {
            status = soc_sbx_g2p3_ipv6mcg_commit(unit,
                                                 SB_G2_FE_RUN_LENGTH_ALL);
        } else {
            status = soc_sbx_g2p3_ipv6mcsg_commit(unit,
                                                  SB_G2_FE_RUN_LENGTH_ALL);
        }
    }
    if (BCM_FAILURE(status)) {
        IPMC_ERR(("unit %d: [%s] error(%s) calling soc_sbx_g2p3_ipv6mcxx_commit\n",
                  unit, FUNCTION_NAME(), bcm_errmsg(status)));
        return status;
    }
    
    /**
     * Now remove the FTE that was attached for this
     * ipmc flow.
     */
   BCM_IF_ERROR_RETURN
        (soc_sbx_g2p3_vlan_ft_base_get(unit, &ftidx));

    ftidx += (flag_null_s_addr) ? ipv6mcg.ftidx :  ipv6mcsg.ftidx;

    status = soc_sbx_g2p3_ft_get(unit, ftidx, &fte);
    if (BCM_FAILURE(status)) {
        IPMC_ERR(("unit %d: [%s] error(%s) calling soc_sbx_g2p3_vlan_ft_base_get\n",
                  unit, FUNCTION_NAME(), bcm_errmsg(status)));
        return status;
    }

    /* save ipmc_index for later */
    ipmc_index = fte.oi;
    
    /* Invalidate the FTE */
    soc_sbx_g2p3_ft_t_init(&fte);
    
    status = soc_sbx_g2p3_ft_set(unit, ftidx, &fte);
    if (BCM_FAILURE(status)) {
        IPMC_ERR(("unit %d: [%s] error(%s) calling soc_sbx_g2p3_ft_set\n",
                  unit, FUNCTION_NAME(), bcm_errmsg(status)));
        return status;
    }

    if (remove_egress) {
        status =
            _bcm_fe2000_ipmc_delete_all_reference_to_ipmc_index(l3_fe,
                                                                ipmc_index);
        if (status != BCM_E_NONE) {
            IPMC_ERR(("unit %d: [%s] error(%s) removing all egress for ipmc-index(0x%x)\n",
                      unit, FUNCTION_NAME(), bcm_errmsg(status),
                      ipmc_index));
            return status;
        }
    }
    
    return status;
}

 int
_bcm_fe2000_g2p3_ipv4mc_flow_delete(int                     unit,
                             _fe2k_l3_fe_instance_t *l3_fe,
                             bcm_ipmc_addr_t        *data,
                             int                     remove_egress)
{
    soc_sbx_g2p3_state_t *fe;
    soc_sbx_g2p3_ipv4mcg_t ipv4mcg;
    soc_sbx_g2p3_ipv4mcsg_t ipv4mcsg;
    soc_sbx_g2p3_ft_t      fte;
    _fe2k_ipmc_route_t     *ipmc_rt;
    int                     status;
    int                     ipmc_index;
    int                     flag_null_s_addr;
    uint32                  ftidx = ~0;
    soc_sbx_g2p3_pv2e_t     pv2e;
    uint32                  mc_vsi = 0;

    status     = BCM_E_NONE;
    ipmc_index = -1;
    flag_null_s_addr = (data->s_ip_addr)?0:1;
  
    fe = (soc_sbx_g2p3_state_t *) SOC_SBX_CONTROL(unit)->drv;

    if (data->flags & BCM_IPMC_USE_IPMC_INDEX) {
        status = _bcm_fe2000_find_ipmc_route(unit,
                                             l3_fe,
                                             data->ipmc_index,
                                             &ipmc_rt);
        if (status != BCM_E_NONE) {
            IPMC_ERR(("unit %d: [%s] unable to find ipmc-rt based on ipmc-index(0x%x)\n",
                      unit, FUNCTION_NAME(), data->ipmc_index));
            return status;
        }

        /**
         * XXX: It is bad that we need to keep the user
         * data for this.
         */
        data->s_ip_addr   = ipmc_rt->ipmc_user_info.s_ip_addr;
        data->mc_ip_addr  = ipmc_rt->ipmc_user_info.mc_ip_addr;
        mc_vsi = ipmc_rt->ipmc_user_info.vid;

    } else {
       status  = SOC_SBX_G2P3_PV2E_GET(l3_fe->fe_unit, data->port_tgid, data->vid, &pv2e);
       if( status != BCM_E_NONE ) {
          IPMC_ERR(("unit %d: [%s] error(%s) obtaining pv2e(port-0x%x, vid-0x%x)\n",
                     unit, FUNCTION_NAME(),
                     bcm_errmsg(status), data->port_tgid, data->vid));
          return status; 
        }

        mc_vsi = pv2e.vlan;

        status = _bcm_fe2000_g2p3_ipv4mc_find_route(unit,
                                                   l3_fe,
                                                   fe,
                                                   data,
                                                   &ipmc_rt,
                                                   &ipv4mcg,
                                                   &ipv4mcsg,
                                                   &fte);
        if (status != BCM_E_NONE) {
            IPMC_ERR(("unit %d: [%s] (%s) finding ipmc-rt based on <s,g,v>(0x%x,0x%x,%d)\n",
                      unit, FUNCTION_NAME(),
                      bcm_errmsg(status),
                      data->s_ip_addr, data->mc_ip_addr, mc_vsi));
            return status;
        }
    }
    

    if (data->flags & BCM_IPMC_USE_IPMC_INDEX) {    
        if (flag_null_s_addr) {
            status = soc_sbx_g2p3_ipv4mcg_get(unit,
                                              (data->mc_ip_addr & 0x0fffffff),
                                              mc_vsi,
                                              &ipv4mcg);
        } else {
            status = soc_sbx_g2p3_ipv4mcsg_get(unit,
                                               (data->mc_ip_addr & 0x0fffffff), 
                                               data->s_ip_addr, mc_vsi,
                                               &ipv4mcsg);
        }
        if (BCM_FAILURE(status)) {
            IPMC_ERR(("unit %d: [%s] error(%s) calling soc_sbx_g2p3_ipv4mcxx_get(0x%x,0x%x,%d)\n",
                      unit, FUNCTION_NAME(), bcm_errmsg(status),
                      data->s_ip_addr, data->mc_ip_addr, mc_vsi));
            return status;
        }
    }
    
    /**
     * Remove the LTE for this ipmc flow
     */
    if (flag_null_s_addr) {
        status = soc_sbx_g2p3_ipv4mcg_remove(unit,
                                             (data->mc_ip_addr & 0x0fffffff),
                                             mc_vsi);
                                   
        if (BCM_FAILURE(status)) {
            IPMC_ERR(("unit %d: [%s] error(%s) calling soc_sbx_g2p3_ipv4mcg_remove\n",
                      unit, FUNCTION_NAME(), bcm_errmsg(status)));
            return status;
        }
    } else {
        status = soc_sbx_g2p3_ipv4mcsg_remove(unit,
                                              (data->mc_ip_addr & 0x0fffffff), 
                                              data->s_ip_addr, mc_vsi);
        if (BCM_FAILURE(status)) {
                        IPMC_ERR(("unit %d: [%s] error(%s) calling soc_sbx_g2p3_ipv4mcsg_remove\n",
                      unit, FUNCTION_NAME(), bcm_errmsg(status)));
            return status;
        }
    }
    
    _bcm_fe2000_free_ipmc_route(l3_fe, &ipmc_rt);
    
    if (SOC_SBX_STATE(unit)->cache_ipmc == FALSE) {
        if (flag_null_s_addr) {
            status = soc_sbx_g2p3_ipv4mcg_commit(unit,
                                                 SB_G2_FE_RUN_LENGTH_ALL);
        } else {
            status = soc_sbx_g2p3_ipv4mcsg_commit(unit,
                                                  SB_G2_FE_RUN_LENGTH_ALL);
        }
    }
    if (BCM_FAILURE(status)) {
        IPMC_ERR(("unit %d: [%s] error(%s) calling ipv4mc commit\n",
                  unit, FUNCTION_NAME(), bcm_errmsg(status)));
        return status;
    }
    
    /**
     * Now remove the FTE that was attached for this
     * ipmc flow.
     */
   BCM_IF_ERROR_RETURN
        (soc_sbx_g2p3_vlan_ft_base_get(unit, &ftidx));

    ftidx += (flag_null_s_addr) ? ipv4mcg.ftidx :  ipv4mcsg.ftidx;
    status = soc_sbx_g2p3_ft_get(unit, ftidx, &fte);

    if (BCM_FAILURE(status)) {
        IPMC_ERR(("unit %d: [%s] error(%s) calling soc_sbx_g2p3_vlan_ft_base_get\n",
                  unit, FUNCTION_NAME(), bcm_errmsg(status)));
        return status;
    }

    /* save ipmc_index for later */
    ipmc_index = fte.oi;
    
    /* Invalidate the FTE */
    soc_sbx_g2p3_ft_t_init(&fte);
    
    status = soc_sbx_g2p3_ft_set(unit, ftidx, &fte);
    if (BCM_FAILURE(status)) {
        IPMC_ERR(("unit %d: [%s] error(%s) calling soc_sbx_g2p3_ft_set\n",
                  unit, FUNCTION_NAME(), bcm_errmsg(status)));
        return status;
    }

    if (remove_egress) {
        status =
            _bcm_fe2000_ipmc_delete_all_reference_to_ipmc_index(l3_fe,
                                                                ipmc_index);
        if (status != BCM_E_NONE) {
            IPMC_ERR(("unit %d: [%s] error(%s) removing all egress for ipmc-index(0x%x)\n",
                      unit, FUNCTION_NAME(), bcm_errmsg(status),
                      ipmc_index));
            return status;
        }
    }
    
    return status;
}

 int
_bcm_fe2000_g2p3_ipmc_flow_delete(int                     unit,
                             _fe2k_l3_fe_instance_t *l3_fe,
                             bcm_ipmc_addr_t        *data,
                             int                     remove_egress)
{
    int status;
    if (data->flags & BCM_IPMC_IP6) {
        status =  _bcm_fe2000_g2p3_ipv6mc_flow_delete(unit, l3_fe,
                               data, remove_egress);
    } else {
        status =  _bcm_fe2000_g2p3_ipv4mc_flow_delete(unit, l3_fe,
                               data, remove_egress);
    }
    return status;
}


/*
 * Function:
 *      _bcm_fe2000_g2p3_ipmc_find
 * Description:
 *      find the ipmc flow and fill in data
 * Parameters:
 *      unit    - fe unit
 *      l3_fe   - L3 fe ptr
 *      fe      - SB fe
 *      data    - ipmc flow info (IN/OUT)
 * Returns:
 *      BCM_E_NONE - on success
 *      BCM_E_XXX  - on error
 * Assumption:
 */
int
_bcm_fe2000_g2p3_ipv6mc_find(int                      unit,
                      _fe2k_l3_fe_instance_t  *l3_fe,
                      bcm_ipmc_addr_t         *data)
{
    soc_sbx_g2p3_state_t    *fe;
    soc_sbx_g2p3_ipv6mcg_t    ipv6mcg;
    soc_sbx_g2p3_ipv6mcsg_t    ipv6mcsg;
    soc_sbx_g2p3_ft_t       fte;
    sbStatus_t              sb_ret;
    int                     status;
    int                     flag_null_s_addr;
    uint32                  ftidx = ~0;
    soc_sbx_g2p3_pv2e_t     pv2e;
    uint32                  mc_vsi = 0;
    
    fe = (soc_sbx_g2p3_state_t *)SOC_SBX_CONTROL(unit)->drv;

    flag_null_s_addr = bcm_l3_ip6_zero_addr(data->s_ip6_addr);
    bcm_l3_ip6_mcify(data->mc_ip6_addr);

    status  = SOC_SBX_G2P3_PV2E_GET(l3_fe->fe_unit, data->port_tgid, 
                                     data->vid, &pv2e);
    if( status != BCM_E_NONE ) {
       IPMC_ERR(("unit %d: [%s] error(%s) obtaining pv2e(port-0x%x, vid-0x%x)\n",
                  unit, FUNCTION_NAME(),
                  bcm_errmsg(status), data->port_tgid, data->vid));
       return status; 
    }

    mc_vsi = pv2e.vlan;


    if (flag_null_s_addr) {
        sb_ret = soc_sbx_g2p3_ipv6mcg_get(unit,
                                data->mc_ip6_addr, mc_vsi, 
                                &ipv6mcg);
    } else {
        sb_ret = soc_sbx_g2p3_ipv6mcsg_get(unit,
                               data->mc_ip6_addr,
                               data->s_ip6_addr, mc_vsi, 
                               &ipv6mcsg);
    }
    if (sb_ret != SB_OK) {
        status = translate_sbx_result(sb_ret);
        IPMC_ERR(("unit %d: [%s] error(%s) unable to get LTE\n",
                  unit, FUNCTION_NAME(), bcm_errmsg(status)));
        return status;
    }
    BCM_IF_ERROR_RETURN(soc_sbx_g2p3_vlan_ft_base_get(unit, &ftidx));
    ftidx += (flag_null_s_addr) ? ipv6mcg.ftidx : ipv6mcsg.ftidx;
    sb_ret = soc_sbx_g2p3_ft_get(unit,
                            ftidx,
                            &fte);
    if (sb_ret != SB_OK) {
        status = translate_sbx_result(sb_ret);
        IPMC_ERR(("unit %d: [%s] error(%s) unable to get fte(0x%x)\n",
                  unit, FUNCTION_NAME(),
                  bcm_errmsg(status), 
                  (flag_null_s_addr) ? ipv6mcg.ftidx : ipv6mcsg.ftidx));
        return status;
    }
    
    data->ipmc_index = fte.oi;
    data->mod_id     = l3_fe->fe_my_modid;

#if 0  
    if (pay4.m_ulSilentRpfCheck == TRUE) {
        data->flags |= BCM_IPMC_SOURCE_PORT_NOCHECK;
    } else {
        data->flags &= ~BCM_IPMC_SOURCE_PORT_NOCHECK;
    }
#endif  
    
    BCM_PBMP_CLEAR(data->l2_pbmp);
    BCM_PBMP_CLEAR(data->l2_ubmp);
    BCM_PBMP_CLEAR(data->l3_pbmp);
    data->ts        = 0;
    data->port_tgid = 0;
#if 0 
    if (fte.ulUseRCos) {
        data->cos = fte.ulRCos;
    } else {
        data->cos = 0;
    }
#endif
    return BCM_E_NONE;
}

int
_bcm_fe2000_g2p3_ipv4mc_find(int                      unit,
                      _fe2k_l3_fe_instance_t  *l3_fe,
                      bcm_ipmc_addr_t         *data)
{
    soc_sbx_g2p3_state_t    *fe;
    soc_sbx_g2p3_ipv4mcg_t    ipv4mcg;
    soc_sbx_g2p3_ipv4mcsg_t    ipv4mcsg;
    soc_sbx_g2p3_ft_t       fte;
    sbStatus_t              sb_ret;
    int                     status;
    int                     flag_null_s_addr;
    uint32                  ftidx = ~0;
    soc_sbx_g2p3_pv2e_t     pv2e;
    uint32                  mc_vsi = 0;
    
    fe = (soc_sbx_g2p3_state_t *)SOC_SBX_CONTROL(unit)->drv;

    flag_null_s_addr = data->s_ip_addr?0:1;
    status  = SOC_SBX_G2P3_PV2E_GET(l3_fe->fe_unit, data->port_tgid, data->vid, &pv2e);
    if( status != BCM_E_NONE ) {
       IPMC_ERR(("unit %d: [%s] error(%s) obtaining pv2e(port-0x%x, vid-0x%x)\n",
                  unit, FUNCTION_NAME(),
                  bcm_errmsg(status), data->port_tgid, data->vid));
       return status; 
    }

    mc_vsi = pv2e.vlan;


    if (flag_null_s_addr) {
        sb_ret = soc_sbx_g2p3_ipv4mcg_get(unit,
                                (data->mc_ip_addr & 0x0fffffff), mc_vsi, 
                                &ipv4mcg);
    } else {
        sb_ret = soc_sbx_g2p3_ipv4mcsg_get(unit,
                               (data->mc_ip_addr & 0x0fffffff),
                               data->s_ip_addr, mc_vsi, 
                               &ipv4mcsg);
    }
    if (sb_ret != SB_OK) {
        status = translate_sbx_result(sb_ret);
        IPMC_ERR(("unit %d: [%s] error(%s) unable to get LTE\n",
                  unit, FUNCTION_NAME(), bcm_errmsg(status)));
        return status;
    }
    BCM_IF_ERROR_RETURN(soc_sbx_g2p3_vlan_ft_base_get(unit, &ftidx));
    ftidx += (flag_null_s_addr) ? ipv4mcg.ftidx : ipv4mcsg.ftidx;
    sb_ret = soc_sbx_g2p3_ft_get(unit,
                            ftidx,
                            &fte);
    if (sb_ret != SB_OK) {
        status = translate_sbx_result(sb_ret);
        IPMC_ERR(("unit %d: [%s] error(%s) unable to get fte(0x%x)\n",
                  unit, FUNCTION_NAME(),
                  bcm_errmsg(status), 
                  (flag_null_s_addr) ? ipv4mcg.ftidx : ipv4mcsg.ftidx));
        return status;
    }
    
    data->ipmc_index = fte.oi;
    data->mod_id     = l3_fe->fe_my_modid;

#if 0  
    if (pay4.m_ulSilentRpfCheck == TRUE) {
        data->flags |= BCM_IPMC_SOURCE_PORT_NOCHECK;
    } else {
        data->flags &= ~BCM_IPMC_SOURCE_PORT_NOCHECK;
    }
#endif  
    
    BCM_PBMP_CLEAR(data->l2_pbmp);
    BCM_PBMP_CLEAR(data->l2_ubmp);
    BCM_PBMP_CLEAR(data->l3_pbmp);
    data->ts        = 0;
    data->port_tgid = 0;
#if 0 
    if (fte.ulUseRCos) {
        data->cos = fte.ulRCos;
    } else {
        data->cos = 0;
    }
#endif
    return BCM_E_NONE;
}

int
_bcm_fe2000_g2p3_ipmc_find(int                      unit,
                      _fe2k_l3_fe_instance_t  *l3_fe,
                      bcm_ipmc_addr_t         *data)
{
    int status;
    if (data->flags & BCM_IPMC_IP6) {
        status =  _bcm_fe2000_g2p3_ipv6mc_find(unit, l3_fe, data);
    } else {
        status =  _bcm_fe2000_g2p3_ipv4mc_find(unit, l3_fe, data);
    }
    return status;
}

int
_bcm_fe2000_g2p3_ipmc_flush_cache(int unit) 
{
    int status;
    status = soc_sbx_g2p3_ipv4mcg_commit(unit, SB_G2_FE_RUN_LENGTH_ALL);
    if (BCM_FAILURE(status)) {
        IPMC_ERR(("unit %d: [%s] error(%s) calling ipv4mcg commit\n",
                  unit, FUNCTION_NAME(), bcm_errmsg(status)));
        return status;
    }

    status = soc_sbx_g2p3_ipv4mcsg_commit(unit, SB_G2_FE_RUN_LENGTH_ALL);
    if (BCM_FAILURE(status)) {
        IPMC_ERR(("unit %d: [%s] error(%s) calling ipv4mcsg commit\n",
                  unit, FUNCTION_NAME(), bcm_errmsg(status)));
        return status;
    }

    if (soc_sbx_g2p3_v6_ena(unit)) {
        status = soc_sbx_g2p3_ipv6mcg_commit(unit, SB_G2_FE_RUN_LENGTH_ALL);
        if (BCM_FAILURE(status)) {
            IPMC_ERR(("unit %d: [%s] error(%s) calling ipv6mcg commit\n",
                      unit, FUNCTION_NAME(), bcm_errmsg(status)));
            return status;
        }

        status = soc_sbx_g2p3_ipv6mcsg_commit(unit, SB_G2_FE_RUN_LENGTH_ALL);
        if (BCM_FAILURE(status)) {
            IPMC_ERR(("unit %d: [%s] error(%s) calling ipv6mcsg commit\n",
                      unit, FUNCTION_NAME(), bcm_errmsg(status)));
            return status;
        }
     }
     return BCM_E_NONE;
}


#endif
