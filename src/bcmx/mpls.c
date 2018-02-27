/*
 * $Id: mpls.c 1.30 Broadcom SDK $
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
 * File:     mpls.c
 * Purpose:  Manages MPLS functionality at a system level
 */

#include <sal/core/libc.h>
#include <shared/alloc.h>

#include <bcm/types.h>

#include <bcmx/l3.h>
#include <bcmx/lport.h>
#include <bcmx/bcmx.h>
#include <bcmx/lplist.h>

#include "bcmx_int.h"


#include <bcmx/mpls.h>

#ifdef INCLUDE_L3

#define BCMX_MPLS_INIT_CHECK    BCMX_READY_CHECK

#define BCMX_MPLS_ERROR_CHECK(_unit, _check, _rv)    \
    BCMX_ERROR_CHECK(_unit, _check, _rv)

#define BCMX_MPLS_SET_ERROR_CHECK(_unit, _check, _rv)    \
    BCMX_SET_ERROR_CHECK(_unit, _check, _rv)

#define BCMX_MPLS_DELETE_ERROR_CHECK(_unit, _check, _rv)    \
    BCMX_DELETE_ERROR_CHECK(_unit, _check, _rv)

#define BCMX_MPLS_GET_IS_VALID(_unit, _rv)    \
    BCMX_ERROR_IS_VALID(_unit, _rv)


#define BCMX_MPLS_VPN_CONFIG_T_PTR_TO_BCM(_info)    \
    ((bcm_mpls_vpn_config_t *)(_info))

#define BCMX_MPLS_PORT_T_PTR_TO_BCM(_mpls_port)    \
    ((bcm_mpls_port_t *)(_mpls_port))

#define BCMX_MPLS_EGRESS_LABEL_T_PTR_TO_BCM(_label)    \
    ((bcm_mpls_egress_label_t *)(_label))

#define BCMX_MPLS_TUNNEL_SWITCH_T_PTR_TO_BCM(_info)    \
    ((bcm_mpls_tunnel_switch_t *)(_info))


/*  
 * Function:
 *      bcm_mpls_vpn_t_init  
 * Purpose:  
 *      Initialize the VPN struct  
 * Parameters:  
 *      vpn_info - Pointer to the struct to be init'ed  
 */ 
int
bcmx_mpls_vpn_t_init(bcmx_mpls_vpn_t *vpn_info)
{
    if (vpn_info == NULL) {
        return BCM_E_PARAM;
    }

    sal_memset(vpn_info, 0, sizeof(*vpn_info));

    return BCM_E_NONE;
}

/*  
 * Function:
 *      bcmx_mpls_l3_initiator_t_init  
 * Purpose:
 *      Initialize the initiator struct
 * Parameters:  
 *      ftn - Pointer to the struct to be init'ed
 * Returns:  
 */
int
bcmx_mpls_l3_initiator_t_init(bcm_mpls_l3_initiator_t *ftn) 
{
    if (ftn == NULL) {
        return BCM_E_PARAM;
    }

    sal_memset(ftn, 0, sizeof(*ftn));

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_mpls_switch_t_init
 * Purpose:
 *      Initialize the ILM structure
 * Parameters:
 *      mswitch - Pointer to the struct to be init'ed
 * Returns:
 *      BCM_E_XXX
 */
int
bcmx_mpls_switch_t_init(bcmx_mpls_switch_t *mswitch)
{
    if (mswitch == NULL) {
        return BCM_E_PARAM;
    }

    sal_memset(mswitch, 0, sizeof(*mswitch));

    return BCM_E_NONE;
}

/*
 * Function:
 *     bcmx_mpls_vpn_config_t_init
 * Purpose:
 *     Initialize the MPLS VPN config structure.
 * Parameters:
 *     info - Pointer to the struct to be initialized
 * Returns:
 *     None
 */
void
bcmx_mpls_vpn_config_t_init(bcmx_mpls_vpn_config_t *info)
{
    if (info != NULL) {
        bcm_mpls_vpn_config_t_init(BCMX_MPLS_VPN_CONFIG_T_PTR_TO_BCM(info));
    }
}

/*
 * Function:
 *     bcmx_mpls_port_t_init
 * Purpose:
 *     Initialize the MPLS port structure
 * Parameters:
 *     mpls_port - Pointer to the struct to be initialized
 * Returns:
 *     None
 */
void
bcmx_mpls_port_t_init(bcmx_mpls_port_t *mpls_port)
{
    if (mpls_port != NULL) {
        bcm_mpls_port_t_init(BCMX_MPLS_PORT_T_PTR_TO_BCM(mpls_port));
    }
}

/*
 * Function:
 *     bcmx_mpls_egress_label_t_init
 * Purpose:
 *     Initialize the MPLS egress label structure
 * Parameters:
 *     label - Pointer to the struct to be initialized
 * Returns:
 *     None
 */
void
bcmx_mpls_egress_label_t_init(bcmx_mpls_egress_label_t *label)
{
    if (label != NULL) {
        bcm_mpls_egress_label_t_init(BCMX_MPLS_EGRESS_LABEL_T_PTR_TO_BCM
                                     (label));
    }
}

/*
 * Function:
 *     bcmx_mpls_tunnel_switch_t_init
 * Purpose:
 *     Initialize the MPLS tunnel switch structure
 * Parameters:
 *     info - Pointer to the struct to be initialized
 * Returns:
 *     None
 */
void
bcmx_mpls_tunnel_switch_t_init(bcmx_mpls_tunnel_switch_t *info)
{
    if (info != NULL) {
        bcm_mpls_tunnel_switch_t_init(BCMX_MPLS_TUNNEL_SWITCH_T_PTR_TO_BCM
                                      (info));
    }
}

/*
 * Function:
 *     bcmx_mpls_circuit_t_init
 * Purpose:
 *     Initialize the MPLS virtual circuit structure
 * Parameters:
 *     mpls_vc - Pointer to the struct to be initialized
 * Returns:
 *     None
 */
void
bcmx_mpls_circuit_t_init(bcmx_mpls_circuit_t *mpls_vc)
{
    if (mpls_vc != NULL) {
        sal_memset(mpls_vc, 0, sizeof(*mpls_vc));
    }

    return;
}


/*
 * Function:
 *      bcmx_mpls_info
 * Purpose:
 *      Get the HW MPLS table information such as table size etc
 * Parameters: 
 *      info - (OUT) table size and used info
 * Returns:
 *      BCM_E_XXXX
 */                                                                 
int
bcmx_mpls_info(void)
{
    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *      _bcmx_mpls_vpn_t_to_bcm
 * Purpose:
 *      BCMX to BCM struct conversion
 * Parameters:
 *      vpn_in - Pointer to struct to be converted
 *      vpn_out (OUT) - BCM VPN struct
 */
int
_bcmx_mpls_vpn_t_to_bcm(bcmx_mpls_vpn_t *vpn_in,
                        bcm_mpls_vpn_t *vpn_out)
{
    uint32             flags = BCMX_DEST_CONVERT_DEFAULT;
    _bcmx_dest_bcm_t   to_bcm;
    _bcmx_dest_bcmx_t  from_bcmx;

    if (vpn_in == NULL || vpn_out == NULL) {
        return BCM_E_PARAM;
    }

    vpn_out->flags = vpn_in->flags;

    /*
     * Convert destination data
     */
    _bcmx_dest_bcmx_t_init(&from_bcmx);
    
    /* Set flags and data to convert */
    if (vpn_in->flags & BCM_MPLS_SRC_TRUNK) {
        flags |= BCMX_DEST_TRUNK;
    }
    from_bcmx.port  = vpn_in->lport;
    from_bcmx.trunk = vpn_in->trunk;

    /* Convert */
    BCM_IF_ERROR_RETURN(_bcmx_dest_to_bcm(&from_bcmx, &to_bcm, &flags));

    /* Set converted flags and data */
    if (flags & BCMX_DEST_TRUNK) {
        vpn_out->flags |= BCM_MPLS_SRC_TRUNK;
    }
    vpn_out->module = to_bcm.module_id;
    vpn_out->port   = to_bcm.module_port;
    vpn_out->trunk  = to_bcm.trunk;

    /* Set remaining fields */
    vpn_out->vpn = vpn_in->vpn;
    vpn_out->l3_intf = vpn_in->l3_intf;
    vpn_out->vlan = vpn_in->vlan;

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_mpls_vpn_t_to_bcmx
 * Purpose:
 *      BCM to BCMX struct conversion
 * Parameters:
 *      vpn_in - Pointer to struct to be converted
 *      vpn_out (OUT) - BCMX VPN struct
 */
int
_bcm_mpls_vpn_t_to_bcmx(bcm_mpls_vpn_t *vpn_in,
                         bcmx_mpls_vpn_t *vpn_out)
{
    uint32             flags = BCMX_DEST_CONVERT_DEFAULT;
    _bcmx_dest_bcm_t   from_bcm;
    _bcmx_dest_bcmx_t  to_bcmx;

    if (vpn_in == NULL || vpn_out == NULL) {
        return BCM_E_PARAM;
    }

    vpn_out->flags = vpn_in->flags;

    /*
     * Convert destination data
     */
    _bcmx_dest_bcm_t_init(&from_bcm);

    /* Set flags and data to convert */
    if (vpn_in->flags & BCM_MPLS_SRC_TRUNK) {
        flags |= BCMX_DEST_TRUNK;
    }
    from_bcm.module_id   = vpn_in->module;
    from_bcm.module_port = vpn_in->port;
    from_bcm.trunk       = vpn_in->trunk;

    /* Convert */
    BCM_IF_ERROR_RETURN(_bcmx_dest_from_bcm(&to_bcmx, &from_bcm, &flags));

    /* Set converted flags and data */
    if (flags & BCMX_DEST_TRUNK) {
        vpn_out->flags |= BCM_MPLS_SRC_TRUNK;
    }
    vpn_out->lport = to_bcmx.port;
    vpn_out->trunk = to_bcmx.trunk;

    /* Set remaining fields */
    vpn_out->vpn = vpn_in->vpn;
    vpn_out->l3_intf = vpn_in->l3_intf;
    vpn_out->vlan = vpn_in->vlan;

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcmx_mpls_circuit_t_to_bcm
 * Purpose:
 *      BCMX to BCM struct conversion
 * Parameters:
 *      vc_in - Pointer to struct to be converted
 *      vc_out (OUT) - BCM VPN struct
 */
int
_bcmx_mpls_circuit_t_to_bcm(bcmx_mpls_circuit_t *vc_in,
                            bcm_mpls_circuit_t *vc_out)
{
    uint32             flags = BCMX_DEST_CONVERT_DEFAULT;
    _bcmx_dest_bcm_t   to_bcm;
    _bcmx_dest_bcmx_t  from_bcmx;

    if (vc_in == NULL || vc_out == NULL) {
        return BCM_E_PARAM;
    }

    vc_out->flags = vc_in->flags;

    /*
     * Convert destination data
     */
    _bcmx_dest_bcmx_t_init(&from_bcmx);
    
    /* Set flags and data to convert */
    if (vc_in->flags & BCM_MPLS_DST_TRUNK) {
        flags |= BCMX_DEST_TRUNK;
    }
    from_bcmx.port  = vc_in->dst_port;
    from_bcmx.trunk = vc_in->dst_trunk;

    /* Convert */
    BCM_IF_ERROR_RETURN(_bcmx_dest_to_bcm(&from_bcmx, &to_bcm, &flags));

    /* Set converted flags and data */
    if (flags & BCMX_DEST_TRUNK) {
        vc_out->flags |= BCM_MPLS_DST_TRUNK;
    }
    vc_out->dst_mod   = to_bcm.module_id;
    vc_out->dst_port  = to_bcm.module_port;
    vc_out->dst_trunk = to_bcm.trunk;

    /* Set remaining fields */
    vc_out->vpn = vc_in->vpn;
    vc_out->l3_intf = vc_in->l3_intf;
    vc_out->vlan = vc_in->vlan;
    vc_out->label = vc_in->label;
    sal_memcpy(&vc_out->dst_mac, &vc_in->dst_mac, sizeof(bcm_mac_t));

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_mpls_circuit_t_to_bcmx
 * Purpose:
 *      BCM to BCMX struct conversion
 * Parameters:
 *      vc_in - Pointer to struct to be converted
 *      vc_out (OUT) - BCMX VPN struct
 */
int
_bcm_mpls_circuit_t_to_bcmx(bcm_mpls_circuit_t *vc_in,
                            bcmx_mpls_circuit_t *vc_out)
{
    uint32             flags = BCMX_DEST_CONVERT_DEFAULT;
    _bcmx_dest_bcm_t   from_bcm;
    _bcmx_dest_bcmx_t  to_bcmx;

    if (vc_in == NULL || vc_out == NULL) {
        return BCM_E_PARAM;
    }

    vc_out->flags = vc_in->flags;

    /*
     * Convert destination data
     */
    _bcmx_dest_bcm_t_init(&from_bcm);

    /* Set flags and data to convert */
    if (vc_in->flags & BCM_MPLS_DST_TRUNK) {
        flags |= BCMX_DEST_TRUNK;
    }
    from_bcm.module_id   = vc_in->dst_mod;
    from_bcm.module_port = vc_in->dst_port;
    from_bcm.trunk       = vc_in->dst_trunk;

    /* Convert */
    BCM_IF_ERROR_RETURN(_bcmx_dest_from_bcm(&to_bcmx, &from_bcm, &flags));

    /* Set converted flags and data */
    if (flags & BCMX_DEST_TRUNK) {
        vc_out->flags |= BCM_MPLS_DST_TRUNK;
    }
    vc_out->dst_port  = to_bcmx.port;
    vc_out->dst_trunk = to_bcmx.trunk;

    /* Set remaining fields */
    vc_out->vpn = vc_in->vpn;
    vc_out->label = vc_in->label;
    vc_out->l3_intf = vc_in->l3_intf;
    vc_out->vlan = vc_in->vlan;
    sal_memcpy(&vc_out->dst_mac, &vc_in->dst_mac, sizeof(bcm_mac_t));

    return BCM_E_NONE;
}


/*
 * Function:
 *      bcmx_mpls_init
 * Purpose:
 *      Initialize MPLS module system-wide
 * Parameters:
 *      None
 * Returns:
 *      BCM_E_XXX
 */
int
bcmx_mpls_init(void)
{
    int rv = BCM_E_UNAVAIL, tmp_rv;
    int i, bcm_unit;

    BCMX_MPLS_INIT_CHECK;

    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_mpls_init(bcm_unit);
        BCM_IF_ERROR_RETURN(BCMX_MPLS_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}

/*
 * Function:
 *     bcmx_mpls_cleanup
 * Purpose:
 *     Disable MPLS system-wide
 * Parameters:
 *     None
 * Returns:
 *     BCM_E_XXX
 */
int
bcmx_mpls_cleanup(void)
{
    int rv = BCM_E_UNAVAIL, tmp_rv;
    int i, bcm_unit;

    BCMX_MPLS_INIT_CHECK;

    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_mpls_cleanup(bcm_unit);
        BCM_IF_ERROR_RETURN(BCMX_MPLS_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}

/*
 * Function:
 *      bcmx_mpls_port_block_set
 * Purpose:
 *      Stop L2 MPLS packet to go out on certain ports
 * Parameters:
 *      vpn     - the VFI index
 *      lport   - Logical Port
 *      enable  - enable or not
 * Returns:
 *      BCM_E_XXXX
 * Note:
 *      Use lplist 0 to clear the bitmap
 */
int
bcmx_mpls_port_block_set(bcm_vpn_t vpn, bcmx_lport_t lport, int enable)
{
    int bcm_unit;
    bcm_port_t bcm_port;
 
    BCMX_MPLS_INIT_CHECK;

    BCM_IF_ERROR_RETURN
        (_bcmx_dest_to_unit_port(lport, &bcm_unit, &bcm_port,
                                 BCMX_DEST_CONVERT_DEFAULT));

    return bcm_mpls_port_block_set(bcm_unit, vpn, bcm_port, enable);
}

/*
 * Function:
 *      bcmx_mpls_port_block_get
 * Purpose:
 *      Find if ports are enabled or disabled for L2 MPLS packet
 * Parameters:
 *      vpn     - the VFI index
 *      lport   - logical Port
 *      enable  - (OUT) enabled or not
 * Returns:
 *      BCM_E_XXXX
 */

int
bcmx_mpls_port_block_get(bcm_vpn_t vpn, bcmx_lport_t lport, int *enable)
{
    int bcm_unit;
    bcm_port_t bcm_port;
 
    BCMX_MPLS_INIT_CHECK;

    BCMX_PARAM_NULL_CHECK(enable);

    BCM_IF_ERROR_RETURN
        (_bcmx_dest_to_unit_port(lport, &bcm_unit, &bcm_port,
                                 BCMX_DEST_CONVERT_DEFAULT));

    return bcm_mpls_port_block_get(bcm_unit, vpn, bcm_port, enable);
}

/*
 * Function:
 *      bcmx_mpls_vpn_create
 * Purpose:
 *      Create a VPN instance
 * Parameters:
 *      vpn - VPN instance ID
 *      flags - VPN property (BCM_MPLS_L2_VPN or BCM_MPLS_L3_VPN)
 */
int
bcmx_mpls_vpn_create(bcm_vpn_t vpn, uint32 flags)
{
    int rv = BCM_E_UNAVAIL, tmp_rv;
    int i, bcm_unit;

    BCMX_MPLS_INIT_CHECK;

    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_mpls_vpn_create(bcm_unit, vpn, flags);
        BCM_IF_ERROR_RETURN(BCMX_MPLS_SET_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}


/*
 * Function:
 *      bcmx_mpls_vpn_destroy
 * Purpose:
 *      Destroy a VPN instance
 * Parameters:
 *      vpn - VPN instance ID
 */
int
bcmx_mpls_vpn_destroy(bcm_vpn_t vpn)
{
    int rv = BCM_E_UNAVAIL, tmp_rv;
    int i, bcm_unit;

    BCMX_MPLS_INIT_CHECK;

    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_mpls_vpn_destroy(bcm_unit, vpn);
        BCM_IF_ERROR_RETURN
            (BCMX_MPLS_DELETE_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}

/*
 * Function:
 *      bcmx_mpls_vpn_add (bind)
 * Purpose:
 *      Bind interface/port/VLAN to VPN instance
 * Parameters:
 *      vpn - VPN instance ID
 *      vpn_info - VPN association information : interface/port/VLAN
 * Returns:
 *      BCM_E_XXXX
 */
int
bcmx_mpls_vpn_add(bcm_vpn_t vpn, bcmx_mpls_vpn_t *vpn_info)
{
    int unit;
    bcm_mpls_vpn_t bcm_vpn_info;

    BCMX_MPLS_INIT_CHECK;

    BCMX_PARAM_NULL_CHECK(vpn_info);

    BCM_IF_ERROR_RETURN(_bcmx_mpls_vpn_t_to_bcm(vpn_info, &bcm_vpn_info));

    unit = bcmx_lport_bcm_unit(vpn_info->lport);
    if (unit < 0) {
        return BCM_E_PORT;
    }

    return bcm_mpls_vpn_add(unit, vpn, &bcm_vpn_info);
}

/*
 * Function:
 *      bcmx_mpls_vpn_delete (unbind)
 * Purpose:
 *      Unbind interface/port/VLAN to VPN instance
 * Parameters:
 *      vpn      - VPN instance ID
 *      vpn_info - interface/port/VLAN
 * Returns:
 *      BCM_E_XXXX
 */
int
bcmx_mpls_vpn_delete(bcm_vpn_t vpn, bcmx_mpls_vpn_t *vpn_info)
{
    int unit;
    bcm_mpls_vpn_t bcm_vpn_info;

    BCMX_MPLS_INIT_CHECK;

    BCMX_PARAM_NULL_CHECK(vpn_info);

    BCM_IF_ERROR_RETURN(_bcmx_mpls_vpn_t_to_bcm(vpn_info, &bcm_vpn_info));

    unit = bcmx_lport_bcm_unit(vpn_info->lport);
    if (unit < 0) {
        return BCM_E_PORT;
    }

    return bcm_mpls_vpn_delete(unit, vpn, &bcm_vpn_info);
}

/*
 * Function:
 *      bcmx_mpls_vpn_delete_all
 * Purpose:
 *      Unbind all interface/port/VLAN to the VPN instance
 * Parameters:
 *      vpn - VPN instance ID
 * Returns:
 *      BCM_E_XXXX
 */
int
bcmx_mpls_vpn_delete_all(bcm_vpn_t vpn)
{
    int rv = BCM_E_UNAVAIL, tmp_rv;
    int i, bcm_unit;

    BCMX_MPLS_INIT_CHECK;

    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_mpls_vpn_delete_all(bcm_unit, vpn);
        BCM_IF_ERROR_RETURN
            (BCMX_MPLS_DELETE_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}

/*
 * Function:
 *      bcmx_mpls_vpn_get
 * Purpose:
 *      Get the binding of interface/port/VLAN to VPN instance
 * Parameters:
 *      vpn        - VPN instance ID
 *      max        - max elements of the array
 *      vpn_info   - (OUT) array of interface/port/VLAN struct
 *      count      - (OUT) actual count
 * Returns:
 *      BCM_E_XXXX
 */
int
bcmx_mpls_vpn_get(bcm_vpn_t vpn, int max, bcmx_mpls_vpn_t *vpn_info,
                  int *count)
{
    int rv = BCM_E_UNAVAIL, tmp_rv;
    int i, j, bcm_unit, unit_count;
    bcm_mpls_vpn_t *bcm_vpn_info;

    BCMX_MPLS_INIT_CHECK;

    BCMX_PARAM_ARRAY_NULL_CHECK(max, vpn_info);
    BCMX_PARAM_NULL_CHECK(count);

    bcm_vpn_info = sal_alloc(max * sizeof(bcm_mpls_vpn_t), "bcmx_mpls_vpn");
    if (bcm_vpn_info == NULL) {
        return BCM_E_MEMORY;
    }

    *count = 0;
    BCMX_UNIT_ITER(bcm_unit, j) {
        unit_count = 0;
        tmp_rv = bcm_mpls_vpn_get(bcm_unit, vpn, max, bcm_vpn_info, 
                                  &unit_count);
       
        if (BCMX_MPLS_GET_IS_VALID(bcm_unit, tmp_rv)) {
            /*
             * Ignore BCM_E_NOT_FOUND since objects are added
             * for specific unit.  Exit on other failure.
             */
            if (tmp_rv == BCM_E_NOT_FOUND) {
                if (rv == BCM_E_UNAVAIL) {
                    rv = tmp_rv;
                }
                continue;
            }

            rv = tmp_rv;
            if (BCM_FAILURE(tmp_rv)) {
                break;
            }

            if (unit_count > (max - *count)) {
                rv = BCM_E_MEMORY;
                break;
            }

            for (i = 0; i < unit_count; i++) {
                _bcm_mpls_vpn_t_to_bcmx(&bcm_vpn_info[i], 
                                        &vpn_info[(*count)++]);
            }
        }
    }

    sal_free(bcm_vpn_info);
    return rv;
}

/*
 * Function:
 *      bcmx_mpls_vpn_circuit_add
 * Purpose:
 *      Add (Bind) a VC to existing VPN
 * Parameters:
 *      vpn      - The VPN to bind VC to
 *      vc_info  - Virtual circuit info
 * Returns:
 *      BCM_E_XXXX
 */
int
bcmx_mpls_vpn_circuit_add(bcm_vpn_t vpn, bcmx_mpls_circuit_t *vc_info)
{
    int rv = BCM_E_UNAVAIL, tmp_rv;
    int i, bcm_unit;

    BCMX_MPLS_INIT_CHECK;

    BCMX_PARAM_NULL_CHECK(vc_info);

    BCMX_UNIT_ITER(bcm_unit, i) {
        bcm_mpls_circuit_t bcm_vc;
 
        sal_memset(&bcm_vc, 0, sizeof(bcm_mpls_circuit_t));
        _bcmx_mpls_circuit_t_to_bcm(vc_info, &bcm_vc);

        tmp_rv = bcm_mpls_vpn_circuit_add(bcm_unit, vpn, &bcm_vc);
        BCM_IF_ERROR_RETURN(BCMX_MPLS_SET_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}


/*
 * Function:
 *      bcmx_mpls_vpn_circuit_delete
 * Purpose:
 *      Delete (unbind) a VC from existing VPN
 * Parameters:
 *      vpn      - The VPN to bind VC to
 *      vc_info  - Virtual circuit info
 * Returns:
 *      BCM_E_XXXX
 */
int
bcmx_mpls_vpn_circuit_delete(bcm_vpn_t vpn, bcmx_mpls_circuit_t *vc_info)
{
    int rv = BCM_E_UNAVAIL, tmp_rv;
    int i, bcm_unit;

    BCMX_MPLS_INIT_CHECK;

    BCMX_PARAM_NULL_CHECK(vc_info);

    BCMX_UNIT_ITER(bcm_unit, i) {
        bcm_mpls_circuit_t bcm_vc;
 
        sal_memset(&bcm_vc, 0, sizeof(bcm_mpls_circuit_t));
        _bcmx_mpls_circuit_t_to_bcm(vc_info, &bcm_vc);

        tmp_rv = bcm_mpls_vpn_circuit_delete(bcm_unit, vpn, &bcm_vc);
        BCM_IF_ERROR_RETURN
            (BCMX_MPLS_DELETE_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}

/*
 * Function:
 *      bcmx_mpls_vpn_circuit_delete_all
 * Purpose:
 *      Delete (unbind) all VCs from existing VPN
 * Parameters:
 *      vpn      - The VPN to bind VC to
 * Returns:
 *      BCM_E_XXXX
 */
int
bcmx_mpls_vpn_circuit_delete_all(bcm_vpn_t vpn)
{
    int rv = BCM_E_UNAVAIL, tmp_rv;
    int i, bcm_unit;

    BCMX_MPLS_INIT_CHECK;

    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_mpls_vpn_circuit_delete_all(bcm_unit, vpn);
        BCM_IF_ERROR_RETURN
            (BCMX_MPLS_DELETE_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}

/*
 * Function:
 *      bcmx_mpls_vpn_circuit_get
 * Purpose:
 *      Get the VC label and header info for the VPN
 * Parameters:
 *      vpn      - The VPN to bind VC to
 *      max      - max elements of the array
 *      vc_info  - (OUT) Virtual circuit info
 *      count    - (OUT) actual counts
 * Returns:
 *      BCM_E_XXXX
 */
int
bcmx_mpls_vpn_circuit_get(bcm_vpn_t vpn, int max,
                          bcmx_mpls_circuit_t *vc_info, int *count)
{
    int rv = BCM_E_UNAVAIL, tmp_rv;
    int i, j, bcm_unit, unit_count = 0;
    bcm_mpls_circuit_t *vpn_vc_info;

    BCMX_MPLS_INIT_CHECK;

    BCMX_PARAM_ARRAY_NULL_CHECK(max, vc_info);
    BCMX_PARAM_NULL_CHECK(count);

    vpn_vc_info = sal_alloc(max * sizeof(bcm_mpls_circuit_t), "bcm_mpls_vc");
    if (vpn_vc_info == NULL) {
        return BCM_E_MEMORY;
    }

    *count = 0;
    BCMX_UNIT_ITER(bcm_unit, j) {
        tmp_rv = bcm_mpls_vpn_circuit_get(bcm_unit, vpn, max, vpn_vc_info, 
                                          &unit_count);

        if (BCMX_MPLS_GET_IS_VALID(bcm_unit, tmp_rv)) {
            rv = tmp_rv;
            if (BCM_FAILURE(tmp_rv)) {
                break;
            }
            
            if (unit_count > (max - *count)) {
                rv = BCM_E_MEMORY;
                break;
            }

            for (i = 0; i < unit_count; i++) {
                _bcm_mpls_circuit_t_to_bcmx(&vpn_vc_info[i], 
                                            &vc_info[(*count)++]);
            }
        }
    }

    sal_free(vpn_vc_info);
    return rv;
}

/*
 * Function:
 *      bcmx_mpls_l3_initiator_set
 * Purpose:
 *      Set the FEC to MPLS label mapping
 * Parameters:
 *      intf      - the egress L3 interface
 *      mpls_map  - MPLS label and header information
 * Returns:
 *      BCM_E_XXX
 */
int
bcmx_mpls_l3_initiator_set(bcm_l3_intf_t *intf, 
                           bcm_mpls_l3_initiator_t *mpls_map)
{
    int rv = BCM_E_UNAVAIL, tmp_rv;
    int i, bcm_unit;

    BCMX_MPLS_INIT_CHECK;

    BCMX_PARAM_NULL_CHECK(intf);
    BCMX_PARAM_NULL_CHECK(mpls_map);

    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_mpls_l3_initiator_set(bcm_unit, intf, mpls_map);
        BCM_IF_ERROR_RETURN(BCMX_MPLS_SET_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}

/*
 * Function:
 *      bcmx_mpls_l3_initiator_clear
 * Purpose:
 *      Clear the FEC to MPLS label mapping
 * Parameters:  
 *      intf    - the egress L3 interface
 * Returns:
 *      BCM_E_XXX
 */
int
bcmx_mpls_l3_initiator_clear(bcm_l3_intf_t *intf)
{
    int rv = BCM_E_UNAVAIL, tmp_rv;
    int i, bcm_unit;

    BCMX_MPLS_INIT_CHECK;

    BCMX_PARAM_NULL_CHECK(intf);

    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_mpls_l3_initiator_clear(bcm_unit, intf);
        BCM_IF_ERROR_RETURN
            (BCMX_MPLS_DELETE_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}

/*
 * Function:
 *      bcmx_mpls_l3_initiator_get
 * Purpose:
 *      Get the FEC to MPLS label mapping info
 * Parameters:
 *      intf - The egress L3 interface
 *      mpls_label - MPLS header information
 * Returns:
 *      BCM_E_XXXX
 */
int
bcmx_mpls_l3_initiator_get(bcm_l3_intf_t *intf,
                           bcm_mpls_l3_initiator_t *mpls_map, int *count)
{
    int rv = BCM_E_UNAVAIL, tmp_rv;
    int i, bcm_unit;

    BCMX_MPLS_INIT_CHECK;

    BCMX_PARAM_NULL_CHECK(intf);
    BCMX_PARAM_NULL_CHECK(mpls_map);
    BCMX_PARAM_NULL_CHECK(count);

    *count = 0;
    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_mpls_l3_initiator_get(bcm_unit, intf, 
                                           &mpls_map[(*count)++]);
        if (BCMX_MPLS_GET_IS_VALID(bcm_unit, tmp_rv)) {
            rv = tmp_rv;
            if (BCM_FAILURE(tmp_rv)) {
                break;
            }
        }
    }

    return rv;
}

/*
 * Function:
 *      bcmx_mpls_l3_initiator_clear_all
 * Purpose:
 *      Clear all the FEC to MPLS label mapping
 * Parameters:
 *      None
 * Returns:
 *      BCM_E_XXXX
 */
int
bcmx_mpls_l3_initiator_clear_all(void)
{
    int rv = BCM_E_UNAVAIL, tmp_rv;
    int i, bcm_unit;

    BCMX_MPLS_INIT_CHECK;

    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_mpls_l3_initiator_clear_all(bcm_unit);
        BCM_IF_ERROR_RETURN
            (BCMX_MPLS_DELETE_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}

/*
 * Function:
 *      _bcmx_mpls_switch_t_to_bcm
 * Purpose:
 *      BCMX to BCM struct conversion
 * Parameters:
 *      msw_in - Pointer to struct to be converted
 *      msw_out (OUT) - BCM MPLS Switch struct
 */
int
_bcmx_mpls_switch_t_to_bcm(bcmx_mpls_switch_t *msw_in, 
                           bcm_mpls_switch_t *msw_out)
{
    uint32             flags = BCMX_DEST_CONVERT_DEFAULT;
    _bcmx_dest_bcm_t   to_bcm;
    _bcmx_dest_bcmx_t  from_bcmx;

    if (msw_in == NULL || msw_out == NULL) {
        return BCM_E_PARAM;
    }

    bcm_mpls_switch_t_init(msw_out);

    msw_out->flags = msw_in->flags;

    /*
     * Convert destination data
     */

    /*** Set source ***/
    if (msw_in->src_lport == BCMX_LPORT_ETHER_ALL) {
        msw_out->src_mod   = -1;
        msw_out->src_port  = -1;
        msw_out->src_trunk = msw_in->src_trunk;
    } else {
        _bcmx_dest_bcmx_t_init(&from_bcmx);
    
        /* Set flags and data to convert */
        if (msw_in->flags & BCM_MPLS_SRC_TRUNK) {
            flags |= BCMX_DEST_TRUNK;
        }
        from_bcmx.port  = msw_in->src_lport;
        from_bcmx.trunk = msw_in->src_trunk;

        /* Convert */
        BCM_IF_ERROR_RETURN(_bcmx_dest_to_bcm(&from_bcmx, &to_bcm, &flags));

        /* Set converted flags and data */
        if (flags & BCMX_DEST_TRUNK) {
            msw_out->flags |= BCM_MPLS_SRC_TRUNK;
        }
        msw_out->src_mod   = to_bcm.module_id;
        msw_out->src_port  = to_bcm.module_port;
        msw_out->src_trunk = to_bcm.trunk;
    }

    /*** Set NH ***/
    flags = BCMX_DEST_CONVERT_DEFAULT;
    _bcmx_dest_bcmx_t_init(&from_bcmx);
    
    /* Set flags and data to convert */
    if (msw_in->flags & BCM_MPLS_NEXT_TRUNK) {
        flags |= BCMX_DEST_TRUNK;
    }
    from_bcmx.port  = msw_in->next_lport;
    from_bcmx.trunk = msw_in->next_trunk;

    /* Convert */
    BCM_IF_ERROR_RETURN(_bcmx_dest_to_bcm(&from_bcmx, &to_bcm, &flags));

    /* Set converted flags and data */
    if (flags & BCMX_DEST_TRUNK) {
        msw_out->flags |= BCM_MPLS_NEXT_TRUNK;
    }
    msw_out->next_mod   = to_bcm.module_id;
    msw_out->next_port  = to_bcm.module_port;
    msw_out->next_trunk = to_bcm.trunk;

    /*** Set destination ***/
    flags = BCMX_DEST_CONVERT_DEFAULT;
    _bcmx_dest_bcmx_t_init(&from_bcmx);
    
    /* Set flags and data to convert */
    if (msw_in->flags & BCM_MPLS_DST_TRUNK) {
        flags |= BCMX_DEST_TRUNK;
    }
    from_bcmx.port  = msw_in->dst_lport;
    from_bcmx.trunk = msw_in->dst_trunk;

    /* Convert */
    BCM_IF_ERROR_RETURN(_bcmx_dest_to_bcm(&from_bcmx, &to_bcm, &flags));

    /* Set converted flags and data */
    if (flags & BCMX_DEST_TRUNK) {
        msw_out->flags |= BCM_MPLS_DST_TRUNK;
    }
    msw_out->dst_mod   = to_bcm.module_id;
    msw_out->dst_port  = to_bcm.module_port;
    msw_out->dst_trunk = to_bcm.trunk;


    /* Set remaining fields */
    msw_out->src_label1 = msw_in->src_label1;
    msw_out->src_label2 = msw_in->src_label2;
    msw_out->action = msw_in->action;
    msw_out->src_l3_intf = msw_in->src_l3_intf;
    msw_out->label_pri = msw_in->label_pri;
    msw_out->swap_label = msw_in->swap_label;
    msw_out->push_label1 = msw_in->push_label1;
    msw_out->push_label2 = msw_in->push_label2;
    msw_out->next_intf = msw_in->next_intf;
    sal_memcpy(&msw_out->next_mac, &msw_in->next_mac, sizeof(bcm_mac_t));
    msw_out->next_vlan = msw_in->next_vlan;
    msw_out->vpn = msw_in->vpn;
    msw_out->dst_vlan = msw_in->dst_vlan;

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_mpls_switch_t_to_bcmx
 * Purpose:
 *      BCM to BCMX struct conversion
 * Parameters:
 *      msw_in - Pointer to struct to be converted
 *      msw_out (OUT) - BCMX MPLS Switch struct
 */
int
_bcm_mpls_switch_t_to_bcmx(bcm_mpls_switch_t *msw_in, 
                           bcmx_mpls_switch_t *msw_out)
{
    uint32             flags = BCMX_DEST_CONVERT_DEFAULT;
    _bcmx_dest_bcm_t   from_bcm;
    _bcmx_dest_bcmx_t  to_bcmx;

    if (msw_in == NULL || msw_out == NULL) {
        return BCM_E_PARAM;
    }

    bcmx_mpls_switch_t_init(msw_out);

    msw_out->flags = msw_in->flags;

    /*
     * Convert destination data
     */

    /*** Set source ***/
    if (msw_in->src_port == -1) {
        msw_out->src_lport = BCMX_LPORT_ETHER_ALL;
        msw_out->src_trunk = msw_in->src_trunk;
    } else {
        _bcmx_dest_bcm_t_init(&from_bcm);

        /* Set flags and data to convert */
        if (msw_in->flags & BCM_MPLS_SRC_TRUNK) {
            flags |= BCMX_DEST_TRUNK;
        }
        from_bcm.module_id   = msw_in->src_mod;
        from_bcm.module_port = msw_in->src_port;
        from_bcm.trunk       = msw_in->src_trunk;

        /* Convert */
        BCM_IF_ERROR_RETURN(_bcmx_dest_from_bcm(&to_bcmx, &from_bcm, &flags));

        /* Set converted flags and data */
        if (flags & BCMX_DEST_TRUNK) {
            msw_out->flags |= BCM_MPLS_SRC_TRUNK;
        }
        msw_out->src_lport = to_bcmx.port;
        msw_out->src_trunk = to_bcmx.trunk;
    }

    /*** Set NH ***/
    flags = BCMX_DEST_CONVERT_DEFAULT;
    _bcmx_dest_bcm_t_init(&from_bcm);

    /* Set flags and data to convert */
    if (msw_in->flags & BCM_MPLS_NEXT_TRUNK) {
        flags |= BCMX_DEST_TRUNK;
    }
    from_bcm.module_id   = msw_in->next_mod;
    from_bcm.module_port = msw_in->next_port;
    from_bcm.trunk       = msw_in->next_trunk;

    /* Convert */
    BCM_IF_ERROR_RETURN(_bcmx_dest_from_bcm(&to_bcmx, &from_bcm, &flags));

    /* Set converted flags and data */
    if (flags & BCMX_DEST_TRUNK) {
        msw_out->flags |= BCM_MPLS_NEXT_TRUNK;
    }
    msw_out->next_lport = to_bcmx.port;
    msw_out->next_trunk = to_bcmx.trunk;

    /*** Set destination ***/
    flags = BCMX_DEST_CONVERT_DEFAULT;
    _bcmx_dest_bcm_t_init(&from_bcm);

    /* Set flags and data to convert */
    if (msw_in->flags & BCM_MPLS_DST_TRUNK) {
        flags |= BCMX_DEST_TRUNK;
    }
    from_bcm.module_id   = msw_in->dst_mod;
    from_bcm.module_port = msw_in->dst_port;
    from_bcm.trunk       = msw_in->dst_trunk;

    /* Convert */
    BCM_IF_ERROR_RETURN(_bcmx_dest_from_bcm(&to_bcmx, &from_bcm, &flags));

    /* Set converted flags and data */
    if (flags & BCMX_DEST_TRUNK) {
        msw_out->flags |= BCM_MPLS_DST_TRUNK;
    }
    msw_out->dst_lport = to_bcmx.port;
    msw_out->dst_trunk = to_bcmx.trunk;


    /* Set remaining fields */
    msw_out->src_label1 = msw_in->src_label1;
    msw_out->src_label2 = msw_in->src_label2;
    msw_out->action = msw_in->action;
    msw_out->src_l3_intf = msw_in->src_l3_intf;
    msw_out->label_pri = msw_in->label_pri;
    msw_out->swap_label = msw_in->swap_label;
    msw_out->push_label1 = msw_in->push_label1;
    msw_out->push_label2 = msw_in->push_label2;
    msw_out->next_intf = msw_in->next_intf;
    sal_memcpy(&msw_out->next_mac, msw_in->next_mac, sizeof(bcm_mac_t));
    msw_out->next_vlan = msw_in->next_vlan;
    msw_out->vpn = msw_in->vpn;
    msw_out->dst_vlan = msw_in->dst_vlan;

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcmx_mpls_switch_add
 * Purpose:
 *      Add MPLS Label swapping, popping and packet forwarding information.
 *      This will be setup in LPM table (MPLS view).
 *      Actions :-
 *      1) label swapping, popping at LSR
 *      2) Ethernet MPLS payload forwarding at the VC termination point
 *      3) L3 MPLS payload (IP packet) forwarding at the LSP termination point
 * Parameters:
 *      mswitch  - (IN) L3 MPLS label mapping and pkt forwarding info
 * Returns:
 *      BCM_E_XXXX
 */
int
bcmx_mpls_switch_add(bcmx_mpls_switch_t *mswitch)
{
    int rv = BCM_E_UNAVAIL, tmp_rv;
    int i, bcm_unit;
    bcm_mpls_switch_t switch_info;

    BCMX_MPLS_INIT_CHECK;

    BCMX_PARAM_NULL_CHECK(mswitch);

    _bcmx_mpls_switch_t_to_bcm(mswitch, &switch_info);
    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_mpls_switch_add(bcm_unit, &switch_info);
        BCM_IF_ERROR_RETURN(BCMX_MPLS_SET_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}


/*
 * Function:
 *      bcmx_mpls_switch_delete
 * Purpose:
 *      Delete MPLS label swapping and packet forwarding information 
 * Parameters:
 *      mswitch - (IN) L3 MPLS label switching and packet forwarding info
 * Returns:
 *      BCM_E_XXXX
 */
int
bcmx_mpls_switch_delete(bcmx_mpls_switch_t *mswitch)
{
    int rv = BCM_E_UNAVAIL, tmp_rv;
    int i, bcm_unit;
    bcm_mpls_switch_t switch_info;

    BCMX_MPLS_INIT_CHECK;

    BCMX_PARAM_NULL_CHECK(mswitch);

    _bcmx_mpls_switch_t_to_bcm(mswitch, &switch_info);
    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_mpls_switch_delete(bcm_unit, &switch_info);
        BCM_IF_ERROR_RETURN
            (BCMX_MPLS_DELETE_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}

/*
 * Function:
 *      bcmx_mpls_switch_get
 * Purpose:
 *      Get MPLS label swapping and packet forwarding information
 * Parameters:
 *      mswitch_info - (OUT)L3 MPLS label switching info
 * Returns:
 *      BCM_E_XXXX
 */
int
bcmx_mpls_switch_get(bcm_mpls_switch_t *mswitch_info)
{
    int    rv;
    int    i, bcm_unit;

    BCMX_MPLS_INIT_CHECK;

    BCMX_PARAM_NULL_CHECK(mswitch_info);

    BCMX_UNIT_ITER(bcm_unit, i) {
        rv = bcm_mpls_switch_get(bcm_unit, mswitch_info);
        if (BCMX_MPLS_GET_IS_VALID(bcm_unit, rv)) {
            return rv;
        }
    }

    return BCM_E_UNAVAIL;
}

                                                                           
/*
 * Function:
 *      bcmx_mpls_switch_delete_all
 * Purpose:
 *      Delete all MPLS label swapping and packet forwarding information
 * Parameters:
 *      flag - BCM_MPLS_LSR_SWITCH delete LSR action entries
 *             BCM_MPLS_L2_VPN     delete L2 VC termination action entries
 *             BCM_MPLS_L3_VPN     delete L3 LSP termination action entries
 * Returns:
 *      BCM_E_XXXX
 */
int
bcmx_mpls_switch_delete_all(int flag)
{
    int rv = BCM_E_UNAVAIL, tmp_rv;
    int i, bcm_unit;

    BCMX_MPLS_INIT_CHECK;

    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_mpls_switch_delete_all(bcm_unit, flag);
        BCM_IF_ERROR_RETURN
            (BCMX_MPLS_DELETE_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}


/*
 * Function:
 *      bcmx_mpls_exp_map_create
 * Purpose:
 *      Create an MPLS EXP map instance.
 * Parameters:
 *      flags       - (IN)  MPLS flags
 *      exp_map_id  - (OUT) Allocated EXP map ID
 * Returns:
 *      BCM_E_XXX
 */
int
bcmx_mpls_exp_map_create(uint32 flags, int *exp_map_id)
{
    int rv = BCM_E_UNAVAIL, tmp_rv;
    int i, bcm_unit;

    BCMX_MPLS_INIT_CHECK;

    BCMX_PARAM_NULL_CHECK(exp_map_id);

    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_mpls_exp_map_create(bcm_unit, flags, exp_map_id);
        BCM_IF_ERROR_RETURN(BCMX_MPLS_SET_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}

/*
 * Function:
 *      bcmx_mpls_exp_map_destroy
 * Purpose:
 *      Destroy an existing MPLS EXP map instance.
 * Parameters:
 *      exp_map_id - (IN) EXP map ID
 * Returns:
 *      BCM_E_XXX
 */
int
bcmx_mpls_exp_map_destroy(int exp_map_id)
{
    int rv = BCM_E_UNAVAIL, tmp_rv;
    int i, bcm_unit;

    BCMX_MPLS_INIT_CHECK;

    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_mpls_exp_map_destroy(bcm_unit, exp_map_id);
        BCM_IF_ERROR_RETURN
            (BCMX_MPLS_DELETE_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}


/*
 * Function:
 *      bcmx_mpls_exp_map_set
 * Purpose:
 *      Set the mapping of { internal priority, color }
 *      to a EXP value for MPLS headers
 *      in the specified EXP map instance.
 * Parameters:
 *      exp_map_id   - (IN) EXP map ID
 *      exp_map      - (IN) EXP map info
 * Returns:
 *      BCM_E_XXX
 */
int
bcmx_mpls_exp_map_set(int exp_map_id,
                      bcm_mpls_exp_map_t *exp_map)
{
    int rv = BCM_E_UNAVAIL, tmp_rv;
    int i, bcm_unit;

    BCMX_MPLS_INIT_CHECK;

    BCMX_PARAM_NULL_CHECK(exp_map);

    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_mpls_exp_map_set(bcm_unit, exp_map_id, exp_map);
        BCM_IF_ERROR_RETURN(BCMX_MPLS_SET_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}


/*
 * Function:
 *      bcmx_mpls_exp_map_get
 * Purpose:
 *      Get the EXP map information for the specified EXP map instance.
 * Parameters:
 *      exp_map_id   - (IN)  EXP map ID
 *      exp_map      - (OUT) Returning EXP map info
 * Returns:
 *      BCM_E_XXX
 */
int
bcmx_mpls_exp_map_get(int exp_map_id,
                      bcm_mpls_exp_map_t *exp_map)
{
    int    rv;
    int    i, bcm_unit;

    BCMX_MPLS_INIT_CHECK;

    BCMX_UNIT_ITER(bcm_unit, i) {
        rv = bcm_mpls_exp_map_get(bcm_unit, exp_map_id, exp_map);
        if (BCMX_MPLS_GET_IS_VALID(bcm_unit, rv)) {
            return rv;
        }
    }

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *     bcmx_mpls_vpn_id_create
 */
int
bcmx_mpls_vpn_id_create(bcmx_mpls_vpn_config_t *info)
{
    int  rv = BCM_E_UNAVAIL, tmp_rv;
    int  i;
    int  bcm_unit;
    uint32  flags_orig;

    BCMX_MPLS_INIT_CHECK;

    BCMX_PARAM_NULL_CHECK(info);

    /* Store original 'xxx_WITH_ID' flag bit */
    flags_orig = info->flags & BCM_MPLS_VPN_WITH_ID;

    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_mpls_vpn_id_create(bcm_unit,
                                        BCMX_MPLS_VPN_CONFIG_T_PTR_TO_BCM
                                        (info));
        if (BCM_FAILURE(BCMX_MPLS_SET_ERROR_CHECK(bcm_unit, tmp_rv, &rv))) {
            break;
        }
        
        /*
         * If vpn id is not specified, use returned id from
         * first successful 'create' for remaining units.
         */
        if (!(info->flags & BCM_MPLS_VPN_WITH_ID)) {
            if (BCM_SUCCESS(tmp_rv)) {
                info->flags |= BCM_MPLS_VPN_WITH_ID;
            }
        }
    }

    /* Restore 'xxx_WITH_ID' flag bit */
    info->flags &= ~BCM_MPLS_VPN_WITH_ID;
    info->flags |= flags_orig;

    return rv;
}

/*
 * Function:
 *     bcmx_mpls_vpn_id_destroy
 */
int
bcmx_mpls_vpn_id_destroy(bcm_vpn_t vpn)
{
    int  rv = BCM_E_UNAVAIL, tmp_rv;
    int  i;
    int  bcm_unit;

    BCMX_MPLS_INIT_CHECK;

    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_mpls_vpn_id_destroy(bcm_unit, vpn);
        BCM_IF_ERROR_RETURN
            (BCMX_MPLS_DELETE_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}

/*
 * Function:
 *     bcmx_mpls_vpn_id_destroy_all
 */
int
bcmx_mpls_vpn_id_destroy_all(void)
{
    int  rv = BCM_E_UNAVAIL, tmp_rv;
    int  i;
    int  bcm_unit;

    BCMX_MPLS_INIT_CHECK;

    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_mpls_vpn_id_destroy_all(bcm_unit);
        BCM_IF_ERROR_RETURN
            (BCMX_MPLS_DELETE_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}

/*
 * Function:
 *     bcmx_mpls_vpn_id_get
 */
int
bcmx_mpls_vpn_id_get(bcm_vpn_t vpn, bcmx_mpls_vpn_config_t *info)
{
    int  rv;
    int  i, bcm_unit;

    BCMX_MPLS_INIT_CHECK;

    BCMX_PARAM_NULL_CHECK(info);

    BCMX_UNIT_ITER(bcm_unit, i) {
        rv = bcm_mpls_vpn_id_get(bcm_unit,  vpn,
                                 BCMX_MPLS_VPN_CONFIG_T_PTR_TO_BCM(info));
        if (BCMX_MPLS_GET_IS_VALID(bcm_unit, rv)) {
            return rv;
        }
    }

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *     bcmx_mpls_port_add
 */
int
bcmx_mpls_port_add(bcm_vpn_t vpn, bcmx_mpls_port_t *mpls_port)
{
    int  rv = BCM_E_UNAVAIL, tmp_rv;
    int  i;
    int  bcm_unit;
    uint32  flags_orig;

    BCMX_MPLS_INIT_CHECK;

    BCMX_PARAM_NULL_CHECK(mpls_port);

    /* Store original 'xxx_WITH_ID' flag bit */
    flags_orig = mpls_port->flags & BCM_MPLS_PORT_WITH_ID;

    /*
     * Use returned 'mpls_port_id' from first success 'add'
     * to set rest of the units.
     */
    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_mpls_port_add(bcm_unit, vpn,
                                   BCMX_MPLS_PORT_T_PTR_TO_BCM(mpls_port));
        if (BCM_FAILURE(BCMX_MPLS_SET_ERROR_CHECK(bcm_unit, tmp_rv, &rv))) {
            break;
        }

        /*
         * If mpls id is not specified, use returned id from
         * first successful 'add' for remaining units.
         */
        if (!(mpls_port->flags & BCM_MPLS_PORT_WITH_ID)) {
            if (BCM_SUCCESS(tmp_rv)) {
                mpls_port->flags |= BCM_MPLS_PORT_WITH_ID;
            }
        }
    }

    /* Restore 'xxx_WITH_ID' flag bit */
    mpls_port->flags &= ~BCM_MPLS_PORT_WITH_ID;
    mpls_port->flags |= flags_orig;

    return rv;
}

/*
 * Function:
 *     bcmx_mpls_port_delete
 */
int
bcmx_mpls_port_delete(bcm_vpn_t vpn, bcm_gport_t mpls_port_id)
{
    int  rv = BCM_E_UNAVAIL, tmp_rv;
    int  i;
    int  bcm_unit;

    BCMX_MPLS_INIT_CHECK;

    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_mpls_port_delete(bcm_unit, vpn, mpls_port_id);
        BCM_IF_ERROR_RETURN
            (BCMX_MPLS_DELETE_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}

/*
 * Function:
 *     bcmx_mpls_port_delete_all
 */
int
bcmx_mpls_port_delete_all(bcm_vpn_t vpn)
{
    int  rv = BCM_E_UNAVAIL, tmp_rv;
    int  i;
    int  bcm_unit;

    BCMX_MPLS_INIT_CHECK;

    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_mpls_port_delete_all(bcm_unit, vpn);
        BCM_IF_ERROR_RETURN
            (BCMX_MPLS_DELETE_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}

/*
 * Function:
 *     bcmx_mpls_port_get
 */
int
bcmx_mpls_port_get(bcm_vpn_t vpn, bcmx_mpls_port_t *mpls_port)
{
    int  rv;
    int  i, bcm_unit;

    BCMX_MPLS_INIT_CHECK;

    BCMX_PARAM_NULL_CHECK(mpls_port);

    BCMX_UNIT_ITER(bcm_unit, i) {
        rv = bcm_mpls_port_get(bcm_unit,  vpn,
                               BCMX_MPLS_PORT_T_PTR_TO_BCM(mpls_port));
        if (BCMX_MPLS_GET_IS_VALID(bcm_unit, rv)) {
            return rv;
        }
    }

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *     bcmx_mpls_port_get_all
 */
int
bcmx_mpls_port_get_all(bcm_vpn_t vpn, int port_max,
                       bcmx_mpls_port_t *port_array, int *port_count)
{
    int  rv;
    int  i, bcm_unit;

    BCMX_MPLS_INIT_CHECK;

    BCMX_PARAM_ARRAY_NULL_CHECK(port_max, port_array);
    BCMX_PARAM_NULL_CHECK(port_count);

    BCMX_UNIT_ITER(bcm_unit, i) {
        rv = bcm_mpls_port_get_all(bcm_unit,  vpn, port_max,
                                   BCMX_MPLS_PORT_T_PTR_TO_BCM(port_array),
                                   port_count);
        if (BCMX_MPLS_GET_IS_VALID(bcm_unit, rv)) {
            return rv;
        }
    }

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *     bcmx_mpls_tunnel_initiator_set
 */
int
bcmx_mpls_tunnel_initiator_set(bcm_if_t intf, int num_labels,
                               bcmx_mpls_egress_label_t *label_array)
{
    int  rv = BCM_E_UNAVAIL, tmp_rv;
    int  i;
    int  bcm_unit;

    BCMX_MPLS_INIT_CHECK;

    BCMX_PARAM_NULL_CHECK(label_array);

    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_mpls_tunnel_initiator_set(bcm_unit, intf, num_labels,
                                               BCMX_MPLS_EGRESS_LABEL_T_PTR_TO_BCM
                                               (label_array));
        BCM_IF_ERROR_RETURN(BCMX_MPLS_SET_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}

/*
 * Function:
 *     bcmx_mpls_tunnel_initiator_get
 */
int
bcmx_mpls_tunnel_initiator_get(bcm_if_t intf, int label_max,
                               bcmx_mpls_egress_label_t *label_array,
                               int *label_count)
{
    int  rv;
    int  i, bcm_unit;

    BCMX_MPLS_INIT_CHECK;

    BCMX_PARAM_ARRAY_NULL_CHECK(label_max, label_array);
    BCMX_PARAM_NULL_CHECK(label_count);

    BCMX_UNIT_ITER(bcm_unit, i) {
        rv = bcm_mpls_tunnel_initiator_get(bcm_unit, intf, label_max,
                                           BCMX_MPLS_EGRESS_LABEL_T_PTR_TO_BCM
                                           (label_array),
                                           label_count);
        if (BCMX_MPLS_GET_IS_VALID(bcm_unit, rv)) {
            return rv;
        }
    }

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *     bcmx_mpls_tunnel_initiator_clear
 */
int
bcmx_mpls_tunnel_initiator_clear(bcm_if_t intf)
{
    int  rv = BCM_E_UNAVAIL, tmp_rv;
    int  i;
    int  bcm_unit;

    BCMX_MPLS_INIT_CHECK;

    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_mpls_tunnel_initiator_clear(bcm_unit, intf);
        BCM_IF_ERROR_RETURN
            (BCMX_MPLS_DELETE_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}

/*
 * Function:
 *     bcmx_mpls_tunnel_initiator_clear_all
 */
int
bcmx_mpls_tunnel_initiator_clear_all(void)
{
    int  rv = BCM_E_UNAVAIL, tmp_rv;
    int  i;
    int  bcm_unit;

    BCMX_MPLS_INIT_CHECK;

    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_mpls_tunnel_initiator_clear_all(bcm_unit);
        BCM_IF_ERROR_RETURN
            (BCMX_MPLS_DELETE_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}

/*
 * Function:
 *     bcmx_mpls_tunnel_switch_add
 */
int
bcmx_mpls_tunnel_switch_add(bcmx_mpls_tunnel_switch_t *info)
{
    int         rv = BCM_E_UNAVAIL, tmp_rv;
    int         i;
    int         bcm_unit;
    bcm_port_t  bcm_port;

    BCMX_MPLS_INIT_CHECK;

    BCMX_PARAM_NULL_CHECK(info);

    /*
     * If 'port' member is a valid BCMX logical port, then port
     * refers to a specific physical port (mod/port or dev/port).
     * Else, 'port' is a system-wide value.
     */
    if (BCM_SUCCESS(_bcmx_dest_to_unit_port(info->port,
                                            &bcm_unit, &bcm_port,
                                            BCMX_DEST_CONVERT_DEFAULT))) {
        return bcm_mpls_tunnel_switch_add(bcm_unit,
                                          BCMX_MPLS_TUNNEL_SWITCH_T_PTR_TO_BCM
                                          (info));
    }

    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_mpls_tunnel_switch_add(bcm_unit,
                                            BCMX_MPLS_TUNNEL_SWITCH_T_PTR_TO_BCM
                                            (info));
        BCM_IF_ERROR_RETURN(BCMX_MPLS_SET_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}

/*
 * Function:
 *     bcmx_mpls_tunnel_switch_delete
 */
int
bcmx_mpls_tunnel_switch_delete(bcmx_mpls_tunnel_switch_t *info)
{
    int         rv = BCM_E_UNAVAIL, tmp_rv;
    int         i;
    int         bcm_unit;
    bcm_port_t  bcm_port;

    BCMX_MPLS_INIT_CHECK;

    BCMX_PARAM_NULL_CHECK(info);

    /*
     * If 'port' member is a valid BCMX logical port, then port
     * refers to a specific physical port (mod/port or dev/port).
     * Else, 'port' is a system-wide value.
     */
    if (BCM_SUCCESS(_bcmx_dest_to_unit_port(info->port,
                                            &bcm_unit, &bcm_port,
                                            BCMX_DEST_CONVERT_DEFAULT))) {
        return bcm_mpls_tunnel_switch_delete(bcm_unit,
                                             BCMX_MPLS_TUNNEL_SWITCH_T_PTR_TO_BCM
                                             (info));
    }

    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_mpls_tunnel_switch_delete(bcm_unit,
                                               BCMX_MPLS_TUNNEL_SWITCH_T_PTR_TO_BCM
                                               (info));
        BCM_IF_ERROR_RETURN
            (BCMX_MPLS_DELETE_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}

/*
 * Function:
 *     bcmx_mpls_tunnel_switch_delete_all
 */
int
bcmx_mpls_tunnel_switch_delete_all(void)
{
    int  rv = BCM_E_UNAVAIL, tmp_rv;
    int  i;
    int  bcm_unit;

    BCMX_MPLS_INIT_CHECK;

    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_mpls_tunnel_switch_delete_all(bcm_unit);
        BCM_IF_ERROR_RETURN
            (BCMX_MPLS_DELETE_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}

/*
 * Function:
 *     bcmx_mpls_tunnel_switch_get
 */
int
bcmx_mpls_tunnel_switch_get(bcmx_mpls_tunnel_switch_t *info)
{
    int         rv;
    int         i;
    int         bcm_unit;
    bcm_port_t  bcm_port;

    BCMX_MPLS_INIT_CHECK;

    BCMX_PARAM_NULL_CHECK(info);

    /*
     * If 'port' member is a valid BCMX logical port, then port
     * refers to a specific physical port (mod/port or dev/port).
     * Else, 'port' is a system-wide value.
     */
    if (BCM_SUCCESS(_bcmx_dest_to_unit_port(info->port,
                                            &bcm_unit, &bcm_port,
                                            BCMX_DEST_CONVERT_DEFAULT))) {
        return bcm_mpls_tunnel_switch_get(bcm_unit,
                                          BCMX_MPLS_TUNNEL_SWITCH_T_PTR_TO_BCM
                                          (info));
    }

    BCMX_UNIT_ITER(bcm_unit, i) {
        rv = bcm_mpls_tunnel_switch_get(bcm_unit,
                                        BCMX_MPLS_TUNNEL_SWITCH_T_PTR_TO_BCM
                                        (info));
        if (BCMX_MPLS_GET_IS_VALID(bcm_unit, rv)) {
            return rv;
        }
    }

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *     bcmx_mpls_label_stat_get
 */
int
bcmx_mpls_label_stat_get(bcm_mpls_label_t label, bcm_gport_t port,
                         bcm_mpls_stat_t stat, uint64 *val)
{
    int         rv = BCM_E_UNAVAIL, tmp_rv;
    int         i;
    int         bcm_unit;
    bcm_port_t  bcm_port;
    uint64      tmp_val;

    BCMX_MPLS_INIT_CHECK;

    BCMX_PARAM_NULL_CHECK(val);

    /*
     * If gport is a valid BCMX logical port, then gport refers to
     * a specific physical port (mod/port or dev/port).
     * Else, gport is a system-wide value, gather stats from all units.
     */
    if (BCM_SUCCESS(_bcmx_dest_to_unit_port(port, &bcm_unit, &bcm_port,
                                            BCMX_DEST_CONVERT_DEFAULT))) {
        return bcm_mpls_label_stat_get(bcm_unit, label, port, stat, val);
    }

    COMPILER_64_ZERO(*val);
    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_mpls_label_stat_get(bcm_unit, label, port, stat,
                                         &tmp_val);
        if (BCMX_MPLS_GET_IS_VALID(bcm_unit, tmp_rv)) {
            rv = tmp_rv;
            if (BCM_SUCCESS(tmp_rv)) {
                COMPILER_64_ADD_64(*val, tmp_val);
            } else {
                break;
            }
        }
    }

    return rv;
}

/*
 * Function:
 *     bcmx_mpls_label_stat_get32
 */
int
bcmx_mpls_label_stat_get32(bcm_mpls_label_t label, bcm_gport_t port,
                           bcm_mpls_stat_t stat, uint32 *val)
{
    int         rv = BCM_E_UNAVAIL, tmp_rv;
    int         i;
    int         bcm_unit;
    bcm_port_t  bcm_port;
    uint32      tmp_val;

    BCMX_MPLS_INIT_CHECK;

    BCMX_PARAM_NULL_CHECK(val);

    /*
     * If gport is a valid BCMX logical port, then gport refers to
     * a specific physical port (mod/port or dev/port).
     * Else, gport is a system-wide value, gather stats from all units.
     */
    if (BCM_SUCCESS(_bcmx_dest_to_unit_port(port, &bcm_unit, &bcm_port,
                                            BCMX_DEST_CONVERT_DEFAULT))) {
        return bcm_mpls_label_stat_get32(bcm_unit, label, port, stat, val);
    }

    *val = 0;
    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_mpls_label_stat_get32(bcm_unit, label, port, stat,
                                           &tmp_val);
        if (BCMX_MPLS_GET_IS_VALID(bcm_unit, tmp_rv)) {
            rv = tmp_rv;
            if (BCM_SUCCESS(tmp_rv)) {
                *val += tmp_val;
            } else {
                break;
            }
        }
    }

    return rv;
}

/*
 * Function:
 *     bcmx_mpls_label_stat_clear
 */
int
bcmx_mpls_label_stat_clear(bcm_mpls_label_t label, bcm_gport_t port,
                           bcm_mpls_stat_t stat)
{
    int         rv = BCM_E_UNAVAIL, tmp_rv;
    int         i;
    int         bcm_unit;
    bcm_port_t  bcm_port;

    BCMX_MPLS_INIT_CHECK;

    /*
     * If gport is a valid BCMX logical port, then gport refers to
     * a specific physical port (mod/port or dev/port).
     * Else, gport is a system-wide value.
     */
    if (BCM_SUCCESS(_bcmx_dest_to_unit_port(port, &bcm_unit, &bcm_port,
                                            BCMX_DEST_CONVERT_DEFAULT))) {
        return bcm_mpls_label_stat_clear(bcm_unit, label, port, stat);
    }

    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_mpls_label_stat_clear(bcm_unit, label, port, stat);
        BCM_IF_ERROR_RETURN
            (BCMX_MPLS_DELETE_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}

#endif  /* INCLUDE_L3 */
