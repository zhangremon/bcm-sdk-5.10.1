/*
 * $Id: multicast.c 1.43 Broadcom SDK $
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
 * File:    multicast.c
 * Purpose: Manages multicast functions
 */

#include <soc/defs.h>
#include <sal/core/libc.h>

#if defined(BCM_TRIUMPH2_SUPPORT) && defined(INCLUDE_L3)

#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/util.h>
#include <soc/debug.h>
#include <soc/triumph.h>

#include <bcm/error.h>

#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/l3.h>
#include <bcm_int/esw/firebolt.h>
#include <bcm_int/esw/xgs3.h>
#include <bcm_int/common/multicast.h>
#include <bcm_int/esw/multicast.h>
#include <bcm_int/esw/triumph.h>
#include <bcm_int/esw/trx.h>
#include <bcm_int/esw/triumph2.h>
#include <bcm_int/esw/stack.h>
#ifdef BCM_TRIDENT_SUPPORT
#include <bcm_int/esw/trill.h>
#include <bcm_int/esw/trident.h>
#endif /* BCM_TRIDENT_SUPPORT */
#include <bcm_int/esw_dispatch.h>
#include <bcm_int/api_xlate_port.h>

#if defined(BCM_TRIDENT_SUPPORT)
/*
 * Function:
 *      bcm_td_multicast_trill_group_update
 * Purpose:
 *      Update TRILL Tree ID of L3-IPMC group
 * Parameters:
 *      unit         - (IN) Unit number.
 *      ipmc_index (IN)
 *      trill_tree_id  (IN)
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_td_multicast_trill_group_update(int unit, int ipmc_index, uint8 trill_tree_id)
{
    int rv = BCM_E_NONE;
    egr_ipmc_entry_t egr_ipmc;

    soc_mem_lock(unit, EGR_IPMCm);

    rv = READ_EGR_IPMCm(unit, MEM_BLOCK_ALL, ipmc_index, &egr_ipmc);
    if (rv < 0 ) {
         soc_mem_unlock(unit, EGR_IPMCm);       
         return rv;
    }

    if(soc_mem_field_valid(unit, EGR_IPMCm, TRILL_TREE_PROFILE_PTRf)) {
         soc_EGR_IPMCm_field32_set(unit, &egr_ipmc, 
              TRILL_TREE_PROFILE_PTRf, trill_tree_id);
    }
    if(soc_mem_field_valid(unit, EGR_IPMCm, IPMC_GROUP_TYPEf)) {
         soc_EGR_IPMCm_field32_set(unit, &egr_ipmc, IPMC_GROUP_TYPEf, 0x1);
    }

    rv =  WRITE_EGR_IPMCm(unit, MEM_BLOCK_ALL, 
              ipmc_index, &egr_ipmc);
    soc_mem_unlock(unit, EGR_IPMCm);

    return rv;
}
#endif /* BCM_TRIDENT_SUPPORT */

/*
 * Function:
 *      bcm_multicast_vpls_encap_get
 * Purpose:
 *      Get the Encap ID for a MPLS port.
 * Parameters:
 *      unit         - (IN) Unit number.
 *      group        - (IN) Multicast group ID.
 *      port         - (IN) Physical port.
 *      mpls_port_id - (IN) MPLS port ID.
 *      encap_id     - (OUT) Encap ID.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tr2_multicast_vpls_encap_get(int unit, bcm_multicast_t group, bcm_gport_t port,
                                bcm_gport_t mpls_port_id, bcm_if_t *encap_id)
{
    BCM_IF_ERROR_RETURN(
        bcm_tr_multicast_vpls_encap_get(unit, group, port, 
                                        mpls_port_id, encap_id));
    /* Triumph2 is able to do multicast replications over next hops and interfaces */
    *encap_id += BCM_XGS3_DVP_EGRESS_IDX_MIN;
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_multicast_mim_encap_get
 * Purpose:
 *      Get the Encap ID for a MiM port.
 * Parameters:
 *      unit         - (IN) Unit number.
 *      group        - (IN) Multicast group ID.
 *      port         - (IN) Physical port.
 *      mim_port_id -  (IN) MIM port ID.
 *      encap_id     - (OUT) Encap ID.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tr2_multicast_mim_encap_get(int unit, bcm_multicast_t group, bcm_gport_t port,
                                bcm_gport_t mim_port_id, bcm_if_t *encap_id)
{
    int vp;
    ing_dvp_table_entry_t dvp;

    if (!BCM_GPORT_IS_MIM_PORT(mim_port_id)) {
        return BCM_E_PARAM;
    }
    vp = BCM_GPORT_MIM_PORT_ID_GET(mim_port_id); 
    if (vp >= soc_mem_index_count(unit, SOURCE_VPm)) {
        return BCM_E_PARAM;
    }
    BCM_IF_ERROR_RETURN (READ_ING_DVP_TABLEm(unit, MEM_BLOCK_ANY, vp, &dvp));

    /* Next-hop index is used for multicast replication */
    *encap_id = (bcm_if_t) soc_ING_DVP_TABLEm_field32_get(unit, &dvp, NEXT_HOP_INDEXf);
    if (!SOC_IS_ENDURO(unit)) {
        *encap_id += BCM_XGS3_DVP_EGRESS_IDX_MIN;
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_multicast_wlan_encap_get
 * Purpose:
 *      Get the Encap ID for a WLAN port.
 * Parameters:
 *      unit         - (IN) Unit number.
 *      group        - (IN) Multicast group ID.
 *      port         - (IN) Physical port.
 *      wlan_port_id - (IN) WLAN port ID.
 *      encap_id     - (OUT) Encap ID.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tr2_multicast_wlan_encap_get(int unit, bcm_multicast_t group, bcm_gport_t port,
                                bcm_gport_t wlan_port_id, bcm_if_t *encap_id)
{
    int vp;
    ing_dvp_table_entry_t dvp;

    if (!BCM_GPORT_IS_WLAN_PORT(wlan_port_id)) {
        return BCM_E_PARAM;
    }
    vp = BCM_GPORT_WLAN_PORT_ID_GET(wlan_port_id); 
    if (vp >= soc_mem_index_count(unit, WLAN_SVP_TABLEm)) {
        return BCM_E_PARAM;
    }
    BCM_IF_ERROR_RETURN (READ_ING_DVP_TABLEm(unit, MEM_BLOCK_ANY, vp, &dvp));

    /* Next-hop index is used for multicast replication */
    *encap_id = (bcm_if_t) soc_ING_DVP_TABLEm_field32_get(unit, &dvp, NEXT_HOP_INDEXf);
    *encap_id += BCM_XGS3_DVP_EGRESS_IDX_MIN;
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_multicast_subport_encap_get
 * Purpose:
 *      Get the Encap ID for a subport.
 * Parameters:
 *      unit      - (IN) Unit number.
 *      group     - (IN) Multicast group ID.
 *      port      - (IN) Physical port.
 *      subport   - (IN) Subport ID.
 *      encap_id  - (OUT) Encap ID.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tr2_multicast_subport_encap_get(int unit, bcm_multicast_t group, 
                                    bcm_gport_t port,
                                    bcm_gport_t subport, bcm_if_t *encap_id)
{
    int vp, l3_idx,rv;
    ing_dvp_table_entry_t dvp;
    egr_l3_intf_entry_t l3_intf;

    if (!BCM_GPORT_IS_SUBPORT_PORT(subport)) {
        return BCM_E_PARAM;
    }

    l3_idx = BCM_GPORT_SUBPORT_PORT_GET(subport) & 0xfff;
    if (l3_idx >= BCM_XGS3_L3_IF_TBL_SIZE(unit)) { 
        return (BCM_E_PARAM);
    }

    rv = soc_mem_read(unit, EGR_L3_INTFm, MEM_BLOCK_ALL, l3_idx, &l3_intf);
    BCM_IF_ERROR_RETURN(rv);

    vp = soc_mem_field32_get(unit, EGR_L3_INTFm, &l3_intf, IVIDf);
    if (vp >= soc_mem_index_count(unit, SOURCE_VPm)) {
        return BCM_E_PARAM;
    }
    BCM_IF_ERROR_RETURN (READ_ING_DVP_TABLEm(unit, MEM_BLOCK_ANY, vp, &dvp));

    /* Next-hop index is used for multicast replication */
    *encap_id = (bcm_if_t) soc_ING_DVP_TABLEm_field32_get(unit, &dvp, NEXT_HOP_INDEXf);
    if (!SOC_IS_ENDURO(unit)) {
        *encap_id += BCM_XGS3_DVP_EGRESS_IDX_MIN;
    }
    return BCM_E_NONE;
}


#else  /* INCLUDE_L3 && BCM_TRIUMPH2_SUPPORT */
int bcm_esw_triumph2_multicast_not_empty;
#endif /* INCLUDE_L3 && BCM_TRIUMPH2_SUPPORT */
