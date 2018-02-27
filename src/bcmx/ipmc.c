/*
 * $Id: ipmc.c 1.52 Broadcom SDK $
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
 * File:	bcmx/ipmc.c
 * Purpose:	BCMX IP Multicast APIs
 */

#ifdef	INCLUDE_L3

#include <sal/core/libc.h>

#include <bcm/types.h>

#include <bcmx/ipmc.h>
#include <bcmx/lport.h>
#include <bcmx/bcmx.h>
#include <bcmx/lplist.h>

#include "bcmx_int.h"

#define BCMX_IPMC_INIT_CHECK    BCMX_READY_CHECK

#define BCMX_IPMC_ERROR_CHECK(_unit, _check, _rv)    \
    BCMX_ERROR_CHECK(_unit, _check, _rv)

#define BCMX_IPMC_SET_ERROR_CHECK(_unit, _check, _rv)    \
    BCMX_SET_ERROR_CHECK(_unit, _check, _rv)

#define BCMX_IPMC_DELETE_ERROR_CHECK(_unit, _check, _rv)    \
    BCMX_DELETE_ERROR_CHECK(_unit, _check, _rv)

#define BCMX_IPMC_GET_IS_VALID(_unit, _rv)    \
    BCMX_ERROR_IS_VALID(_unit, _rv)

/*
 * Function:
 *      bcmx_ipmc_init
 */

int
bcmx_ipmc_init(void)
{
    int rv = BCM_E_UNAVAIL, tmp_rv;
    int i, bcm_unit;

    BCMX_IPMC_INIT_CHECK;

    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_ipmc_init(bcm_unit);
        BCM_IF_ERROR_RETURN(BCMX_IPMC_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}


/*
 * Function:
 *      bcmx_ipmc_detach
 */

int
bcmx_ipmc_detach(void)
{
    int rv = BCM_E_UNAVAIL, tmp_rv;
    int i, bcm_unit;

    BCMX_IPMC_INIT_CHECK;

    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_ipmc_detach(bcm_unit);
        BCM_IF_ERROR_RETURN(BCMX_IPMC_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}


/*
 * Function:
 *      bcmx_ipmc_enable
 */

int
bcmx_ipmc_enable(int enable)
{
    int rv = BCM_E_UNAVAIL, tmp_rv;
    int i, bcm_unit;

    BCMX_IPMC_INIT_CHECK;

    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_ipmc_enable(bcm_unit, enable);
        BCM_IF_ERROR_RETURN(BCMX_IPMC_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}


/*
 * Function:
 *      bcmx_ipmc_source_port_check
 */

int
bcmx_ipmc_source_port_check(int enable)
{
    int rv = BCM_E_UNAVAIL, tmp_rv;
    int i, bcm_unit;

    BCMX_IPMC_INIT_CHECK;

    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_ipmc_source_port_check(bcm_unit, enable);
        BCM_IF_ERROR_RETURN(BCMX_IPMC_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}


/*
 * Function:
 *      bcmx_ipmc_source_ip_search
 */

int
bcmx_ipmc_source_ip_search(int enable)
{
    int rv = BCM_E_UNAVAIL, tmp_rv;
    int i, bcm_unit;

    BCMX_IPMC_INIT_CHECK;

    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_ipmc_source_ip_search(bcm_unit, enable);
        BCM_IF_ERROR_RETURN(BCMX_IPMC_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}


/*
 * Function:
 *      bcmx_ipmc_addr_t_init
 */

void
bcmx_ipmc_addr_t_init(bcmx_ipmc_addr_t *data) 
{
    sal_memset(data, 0, sizeof(*data));
    bcmx_lplist_init(&data->l2_ports, 0, 0);
    bcmx_lplist_init(&data->l2_untag_ports, 0, 0);
    bcmx_lplist_init(&data->l3_ports, 0, 0);
}

void
bcmx_ipmc_addr_t_free(bcmx_ipmc_addr_t *data)
{
    bcmx_lplist_free(&data->l2_ports);
    bcmx_lplist_free(&data->l2_untag_ports);
    bcmx_lplist_free(&data->l3_ports);
}


/*
 * Function:
 *      _bcmx_ipmc_addr_t_to_bcm
 * Purpose:
 *      Assigns BCMX src IP, mc IP, Vlan, and VRF to BCM IPMC; 
 *      IP version agnostic
 * Parameters:
 *      data - BCMX IPMC struct
 *      ipmc - (OUT) BCM IPMC struct
 * Returns:
 *      None
 */
void
_bcmx_ipmc_addr_t_to_bcm(bcmx_ipmc_addr_t *data, bcm_ipmc_addr_t *ipmc)
{
    bcm_ipmc_addr_t_init(ipmc);
    ipmc->vid = data->vid;
    ipmc->vrf = data->vrf;

    if (data->flags & BCM_IPMC_IP6) {
        sal_memcpy(ipmc->s_ip6_addr, data->s_ip6_addr, BCM_IP6_ADDRLEN);
        sal_memcpy(ipmc->mc_ip6_addr, data->mc_ip6_addr, BCM_IP6_ADDRLEN);
        ipmc->flags |= BCM_IPMC_IP6;
    } else {
        ipmc->s_ip_addr = data->s_ip_addr;
        ipmc->mc_ip_addr = data->mc_ip_addr;
    }
}

/*
 * Function:
 *      _bcm_ipmc_addr_t_to_bcmx
 * Purpose:
 *      Assigns BCM src IP, mc IP, Vlan, and VRF to BCMX IPMC; 
 *      IP version agnostic
 * Parameters:
 *      data - BCM IPMC struct
 *      ipmc - (OUT) BCMX IPMC struct
 * Returns:
 *      None
 */
void
_bcm_ipmc_addr_t_to_bcmx(bcm_ipmc_addr_t *data, bcmx_ipmc_addr_t *ipmc)
{
    bcmx_ipmc_addr_t_init(ipmc);
    ipmc->vid = data->vid; 
    ipmc->vrf = data->vrf;

    if (data->flags & BCM_IPMC_IP6) {
        sal_memcpy(ipmc->s_ip6_addr, data->s_ip6_addr, BCM_IP6_ADDRLEN);
        sal_memcpy(ipmc->mc_ip6_addr, data->mc_ip6_addr, BCM_IP6_ADDRLEN);
        ipmc->flags |= BCM_IPMC_IP6;
    } else {
        ipmc->s_ip_addr = data->s_ip_addr; 
        ipmc->mc_ip_addr = data->mc_ip_addr; 
    }
}

/*
 * Function:
 *      bcmx_ipmc_add
 */

int
bcmx_ipmc_add(bcmx_ipmc_addr_t *data)
{
    int			    rv = BCM_E_UNAVAIL;/* Function return value.         */  
    int             tmp_rv;        /* Single op return value.         */ 
    int	   		    bcm_unit;      /* BCM device number.              */ 
    int             mcindex;       /* Multicast table index.          */
    int             port;          /* Port iterator.                  */
    int             unit_idx;      /* BCM devices iterator.           */
    bcm_ipmc_addr_t	bcm_ipmc;      /* Added entry buffer.             */
    bcm_ipmc_addr_t	bcm_ipmc_temp; /* Existing (if any) entry buffer. */
    bcmx_lport_t	lport;         /* Logical port number.            */
    bcm_pbmp_t		pbmp;          /* L3 forwarding port bit map.     */
    int             lport_unit;
    bcm_port_t      lport_port;
    bcm_module_t    modid;
    bcm_port_t      modport;

    BCMX_IPMC_INIT_CHECK;

    _bcmx_ipmc_addr_t_to_bcm(data, &bcm_ipmc);
    bcm_ipmc.lookup_class = data->lookup_class;
    bcm_ipmc.cos = data->cos;
    bcm_ipmc.ts = data->ts;

    if (!(data->flags & BCM_IPMC_SOURCE_PORT_NOCHECK)) {
        if (data->ts) {
            bcm_ipmc.port_tgid = data->port_tgid;
        } else {
            BCM_IF_ERROR_RETURN
                (_bcmx_dest_to_modid_port(data->port_tgid,
                                          &modid, &modport,
                                          BCMX_DEST_CONVERT_DEFAULT));
            bcm_ipmc.mod_id    = (int)modid;
            bcm_ipmc.port_tgid = (int)modport;
        }
    }
    bcm_ipmc.v = 1;		/* VALID */
    bcm_ipmc.flags = data->flags;

    /*
     * Find ipmc index already in use for this address (if any)
     */
    if (data->flags & BCM_IPMC_USE_IPMC_INDEX) {
        mcindex = data->ipmc_index;
        bcm_ipmc.ipmc_index = data->ipmc_index;
    } else {
        mcindex = -1;
        BCMX_UNIT_ITER(bcm_unit, unit_idx) {
            if (BCM_IS_FABRIC(bcm_unit)) {
                continue;
            }
            bcm_ipmc_temp = bcm_ipmc;
            tmp_rv = bcm_ipmc_find(bcm_unit, &bcm_ipmc_temp); 
            if (BCM_SUCCESS(tmp_rv)) {
                if (!(bcm_ipmc.flags & BCM_IPMC_REPLACE)) { 
                     return (BCM_E_EXISTS);
                }

                mcindex = bcm_ipmc_temp.ipmc_index;
                if (mcindex >= 0) {
                    bcm_ipmc.flags |= BCM_IPMC_USE_IPMC_INDEX;
                    bcm_ipmc.ipmc_index = mcindex;
                }
                break;
            }
        }
    }

    /*
     * Add ipmc entry into non-fabric switches.
     * First one might allocate an ipmc index if needed.
     */
    BCMX_UNIT_ITER(bcm_unit, unit_idx) {
        if (BCM_IS_FABRIC(bcm_unit)) {
            continue;
        }
        BCMX_LPLIST_TO_PBMP(data->l2_ports, bcm_unit, bcm_ipmc.l2_pbmp);
        BCMX_LPLIST_TO_PBMP(data->l2_untag_ports, bcm_unit, bcm_ipmc.l2_ubmp);
        BCMX_LPLIST_TO_PBMP(data->l3_ports, bcm_unit, bcm_ipmc.l3_pbmp);
        /* add stack and higig ports */
        BCMX_FOREACH_QUALIFIED_LPORT(lport,
                                     BCMX_PORT_F_HG|
                                     BCMX_PORT_F_STACK_INT|
                                     BCMX_PORT_F_STACK_EXT) {
            if (BCM_SUCCESS
                (_bcmx_dest_to_unit_port(lport, &lport_unit, &lport_port,
                                         BCMX_DEST_CONVERT_NON_GPORT))) {
                if (lport_unit == bcm_unit) {
                    BCM_PBMP_PORT_ADD(bcm_ipmc.l2_pbmp, lport_port);
                }
            }
        }
        tmp_rv = bcm_ipmc_add(bcm_unit, &bcm_ipmc);
        BCM_IF_ERROR_RETURN(BCMX_IPMC_SET_ERROR_CHECK(bcm_unit, tmp_rv, &rv));

        if (BCM_SUCCESS(tmp_rv) && mcindex < 0) {
            /* grab ipmc index for use by other units */
            (void)bcm_ipmc_find(bcm_unit, &bcm_ipmc);
            mcindex = bcm_ipmc.ipmc_index;
            bcm_ipmc.flags |= BCM_IPMC_USE_IPMC_INDEX;
        }
    }

    if (mcindex < 0) {
        return rv;
    }

    /*
     * Add ipmc entry into fabric switches.
     */
    BCMX_UNIT_ITER(bcm_unit, unit_idx) {
        if (!BCM_IS_FABRIC(bcm_unit)) {
            continue;
        }
        BCM_PBMP_CLEAR(pbmp);
        BCMX_FOREACH_QUALIFIED_LPORT(lport,
                                     BCMX_PORT_F_HG|
                                     BCMX_PORT_F_STACK_INT|
                                     BCMX_PORT_F_STACK_EXT) {
            if (BCM_SUCCESS
                (_bcmx_dest_to_unit_port(lport, &lport_unit, &lport_port,
                                         BCMX_DEST_CONVERT_NON_GPORT))) {
                if (lport_unit == bcm_unit) {
                    BCM_PBMP_PORT_ADD(pbmp, lport_port);
                }
            }
        }
        BCM_PBMP_ITER(pbmp, port) {
            tmp_rv = bcm_ipmc_bitmap_set(bcm_unit, mcindex, port, pbmp);
            BCM_IF_ERROR_RETURN
                (BCMX_IPMC_SET_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
        }
    }

    return rv;
}




STATIC int
_ipmc_bcmx_to_bcm_addr(int unit,
                       bcmx_ipmc_addr_t *src,
                       bcm_ipmc_addr_t *dest,
                       int do_ports)
{
    _bcmx_ipmc_addr_t_to_bcm(src, dest);
    dest->flags = src->flags;

    /*
     * Convert destination data
     */
    if (!(src->flags & BCM_IPMC_SOURCE_PORT_NOCHECK)) {
        uint32             flags = BCMX_DEST_CONVERT_DEFAULT;
        _bcmx_dest_bcm_t   to_bcm;
        _bcmx_dest_bcmx_t  from_bcmx;

        _bcmx_dest_bcmx_t_init(&from_bcmx);
    
        /* Set flags and data to convert */
        if (src->ts) {
            flags |= BCMX_DEST_TRUNK;
        }
        from_bcmx.port  = src->port_tgid;
        from_bcmx.trunk = src->port_tgid;

        /* Convert */
        BCM_IF_ERROR_RETURN(_bcmx_dest_to_bcm(&from_bcmx, &to_bcm, &flags));

        /* Set converted flags and data */
        if (flags & BCMX_DEST_TRUNK) {
            dest->ts = src->ts;
            dest->port_tgid = to_bcm.trunk;
        } else {
            dest->mod_id    = (int)to_bcm.module_id;
            dest->port_tgid = (int)to_bcm.module_port;
        }
    }

    /* Set remaining fields */
    dest->lookup_class = src->lookup_class;
    dest->cos = src->cos;
    dest->v = src->v;
    dest->ipmc_index = src->ipmc_index;

    if (do_ports) {
        BCMX_LPLIST_TO_PBMP(src->l2_ports, unit, dest->l2_pbmp);
        BCMX_LPLIST_TO_PBMP(src->l2_untag_ports, unit, dest->l2_ubmp);
        BCMX_LPLIST_TO_PBMP(src->l3_ports, unit, dest->l3_pbmp);
    }

    return BCM_E_NONE;
}

STATIC int
_ipmc_bcm_to_bcmx_addr(int unit,
                       bcm_ipmc_addr_t *src,
                       bcmx_ipmc_addr_t *dest,
                       int do_ports)
{
    _bcm_ipmc_addr_t_to_bcmx(src, dest);
    dest->flags = src->flags;

    /*
     * Convert destination data
     */
    if (!(src->flags & BCM_IPMC_SOURCE_PORT_NOCHECK)) {
        uint32             flags = BCMX_DEST_CONVERT_DEFAULT;
        _bcmx_dest_bcm_t   from_bcm;
        _bcmx_dest_bcmx_t  to_bcmx;

        _bcmx_dest_bcm_t_init(&from_bcm);

        /* Set flags and data to convert */
        if (src->ts) {
            flags |= BCMX_DEST_TRUNK;
        }
        from_bcm.module_id   = (bcm_module_t)src->mod_id;
        from_bcm.module_port = (bcm_port_t)src->port_tgid;
        from_bcm.trunk       = (bcm_trunk_t)src->port_tgid;

        /* Convert */
        BCM_IF_ERROR_RETURN(_bcmx_dest_from_bcm(&to_bcmx, &from_bcm, &flags));

        /* Set converted flags and data */
        if (flags & BCMX_DEST_TRUNK) {
            dest->ts = src->ts;
            dest->port_tgid = to_bcmx.trunk;
        } else {
            dest->port_tgid = to_bcmx.port;
        }
    }

    /* Set remaining fields */
    dest->lookup_class = src->lookup_class;
    dest->cos = src->cos;
    dest->v = src->v;
    dest->ipmc_index = src->ipmc_index;

    if (do_ports) {
        BCMX_LPLIST_PBMP_ADD(&dest->l2_ports, unit, src->l2_pbmp);
        BCMX_LPLIST_PBMP_ADD(&dest->l2_untag_ports, unit, src->l2_ubmp);
        BCMX_LPLIST_PBMP_ADD(&dest->l3_ports, unit, src->l3_pbmp);
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcmx_ipmc_remove
 */

int
bcmx_ipmc_remove(bcmx_ipmc_addr_t *data)
{
    int rv = BCM_E_UNAVAIL, tmp_rv;
    int i, bcm_unit;
    bcm_ipmc_addr_t bcm_data;
    bcm_ipmc_addr_t bcm_data_temp;
    int             mcindex;
    bcm_pbmp_t		empty;
    bcm_port_t      bcm_port;
    bcmx_lport_t	lport;


    BCMX_IPMC_INIT_CHECK;

    if (data->flags & BCM_IPMC_USE_IPMC_INDEX) {
        mcindex = data->ipmc_index;
    } else {
        mcindex = -1;
    }

    BCMX_UNIT_ITER(bcm_unit, i) {
        if (BCM_IS_FABRIC(bcm_unit)) {
            continue;
        }

        BCM_IF_ERROR_RETURN
            (_ipmc_bcmx_to_bcm_addr(bcm_unit, data, &bcm_data, FALSE));

        if (mcindex < 0) {
            bcm_data_temp = bcm_data;
            tmp_rv = bcm_ipmc_find(bcm_unit, &bcm_data_temp);
            if (BCM_SUCCESS(tmp_rv)) {
                mcindex = bcm_data_temp.ipmc_index;
            }
        }

        tmp_rv = bcm_ipmc_remove(bcm_unit, &bcm_data);
        BCM_IF_ERROR_RETURN
            (BCMX_IPMC_DELETE_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    if (mcindex < 0) {
        return rv;
    }

    /*
     * Delete ipmc index from fabric
     */
    BCM_PBMP_CLEAR(empty);
    BCMX_FOREACH_QUALIFIED_LPORT(lport,
				 BCMX_PORT_F_HG|
				 BCMX_PORT_F_STACK_INT|
				 BCMX_PORT_F_STACK_EXT) {
        if (BCM_SUCCESS
            (_bcmx_dest_to_unit_port(lport, &bcm_unit, &bcm_port,
                                     BCMX_DEST_CONVERT_NON_GPORT))) {
            if (!BCM_IS_FABRIC(bcm_unit)) {
                continue;
            }
            tmp_rv = bcm_ipmc_bitmap_set(bcm_unit, mcindex, bcm_port, empty);
            BCM_IF_ERROR_RETURN
                (BCMX_IPMC_DELETE_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
        }
    }

    return rv;
}


/*
 * Function:
 *      bcmx_ipmc_remove_all
 */

int
bcmx_ipmc_remove_all(void)
{
    int rv = BCM_E_UNAVAIL, tmp_rv;
    int i, bcm_unit;

    BCMX_IPMC_INIT_CHECK;

    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_ipmc_remove_all(bcm_unit);
        BCM_IF_ERROR_RETURN
            (BCMX_IPMC_DELETE_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}

/*
 * Function:
 *      bcmx_ipmc_delete
 */

int
bcmx_ipmc_delete(bcm_ip_t s_ip_addr,
		 bcm_ip_t mc_ip_addr,
		 bcm_vlan_t vid,
		 int keep)
{
    int             rv = BCM_E_UNAVAIL, tmp_rv;
    int			i, bcm_unit, mcindex;
    bcm_ipmc_addr_t	bcm_ipmc;
    bcmx_lport_t	lport;
    bcm_pbmp_t		empty;
    bcm_port_t      bcm_port;

    BCMX_IPMC_INIT_CHECK;

    mcindex = -1;
    BCMX_UNIT_ITER(bcm_unit, i) {
	if (BCM_IS_FABRIC(bcm_unit)) {
	    continue;
	}
	if (mcindex < 0) {
            sal_memset(&bcm_ipmc, 0, sizeof(bcm_ipmc_addr_t));
            /* get ipmc index if available */
	    tmp_rv = bcm_ipmc_get(bcm_unit, s_ip_addr, mc_ip_addr, vid,
				  &bcm_ipmc);
	    if (BCM_SUCCESS(tmp_rv)) {
		mcindex = bcm_ipmc.ipmc_index;
	    }
	}
        tmp_rv = bcm_ipmc_delete(bcm_unit, s_ip_addr, mc_ip_addr, vid, keep);
        BCM_IF_ERROR_RETURN
            (BCMX_IPMC_DELETE_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    if (mcindex < 0) {
	return rv;
    }

    /*
     * Delete ipmc index from fabric
     */
    BCM_PBMP_CLEAR(empty);
    BCMX_FOREACH_QUALIFIED_LPORT(lport,
				 BCMX_PORT_F_HG|
				 BCMX_PORT_F_STACK_INT|
				 BCMX_PORT_F_STACK_EXT) {
        if (BCM_SUCCESS
            (_bcmx_dest_to_unit_port(lport, &bcm_unit, &bcm_port,
                                     BCMX_DEST_CONVERT_NON_GPORT))) {
            if (!BCM_IS_FABRIC(bcm_unit)) {
                continue;
            }
            tmp_rv = bcm_ipmc_bitmap_set(bcm_unit, mcindex, bcm_port, empty);
            BCM_IF_ERROR_RETURN
                (BCMX_IPMC_SET_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
        }
    }

    return rv;
}


/*
 * Function:
 *      bcmx_ipmc_delete_all
 */

int
bcmx_ipmc_delete_all(void)
{
    int rv = BCM_E_UNAVAIL, tmp_rv;
    int i, bcm_unit;

    BCMX_IPMC_INIT_CHECK;

    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_ipmc_delete_all(bcm_unit);
        BCM_IF_ERROR_RETURN
            (BCMX_IPMC_DELETE_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}


/*
 * Function:
 *      bcmx_ipmc_get
 */

int
bcmx_ipmc_get(bcm_ip_t s_ip_addr,
	      bcm_ip_t mc_ip_addr,
	      bcm_vlan_t vid,
	      bcmx_ipmc_addr_t *data)
{
    int rv = BCM_E_UNAVAIL, tmp_rv;
    int i, bcm_unit, initialized;
    bcm_ipmc_addr_t	bcm_ipmc;
    uint32 flags;
    bcm_vrf_t vrf;

    BCMX_IPMC_INIT_CHECK;

    BCMX_PARAM_NULL_CHECK(data);

    flags = data->flags;
    vrf = data->vrf;
    sal_memset(data, 0, sizeof(*data));
    initialized = 0;
    data->flags = flags;
    data->vrf = vrf;

    BCMX_UNIT_ITER(bcm_unit, i) {
        sal_memset(&bcm_ipmc, 0, sizeof(bcm_ipmc));
        bcm_ipmc.flags = flags;
        bcm_ipmc.vrf   = vrf;
        tmp_rv = bcm_ipmc_get(bcm_unit, s_ip_addr, mc_ip_addr, vid, &bcm_ipmc);

        if (BCMX_IPMC_GET_IS_VALID(bcm_unit, tmp_rv)) {
            rv = tmp_rv;
            if (BCM_FAILURE(tmp_rv)) {
                break;
            }

            if (!initialized) {
                tmp_rv = _ipmc_bcm_to_bcmx_addr(bcm_unit, &bcm_ipmc, data,
                                                TRUE);
                if (BCM_SUCCESS(tmp_rv)) {
                    initialized = !initialized;
                }
            } else {
                data->flags |= bcm_ipmc.flags;
                BCMX_LPLIST_PBMP_ADD(&data->l2_ports,
                                     bcm_unit, bcm_ipmc.l2_pbmp);
                BCMX_LPLIST_PBMP_ADD(&data->l2_untag_ports,
                                     bcm_unit, bcm_ipmc.l2_ubmp);
                BCMX_LPLIST_PBMP_ADD(&data->l3_ports,
                                     bcm_unit, bcm_ipmc.l3_pbmp);
            }
        }
	}

    return rv;
}


/*
 * Function:
 *      bcmx_ipmc_get_by_index
 */

int
bcmx_ipmc_get_by_index(int index,
		       bcmx_ipmc_addr_t *data)
{
    int rv = BCM_E_UNAVAIL, tmp_rv;
    int i, bcm_unit, initialized;
    bcm_ipmc_addr_t	bcm_ipmc;
    uint32 flags;

    BCMX_IPMC_INIT_CHECK;

    BCMX_PARAM_NULL_CHECK(data);

    flags = data->flags;
    sal_memset(data, 0, sizeof(*data));
    initialized = 0;

    bcm_ipmc_addr_t_init(&bcm_ipmc);
    bcm_ipmc.flags = flags;
    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_ipmc_get_by_index(bcm_unit, index, &bcm_ipmc);

        if (BCMX_IPMC_GET_IS_VALID(bcm_unit, tmp_rv)) {
            rv = tmp_rv;
            if (BCM_FAILURE(tmp_rv)) {
                break;
            }

            if (!initialized) {
                tmp_rv = _ipmc_bcm_to_bcmx_addr(bcm_unit, &bcm_ipmc, data,
                                                TRUE);
                if (BCM_SUCCESS(tmp_rv)) {
                    initialized = !initialized;
                }
            } else {
                data->flags |= bcm_ipmc.flags;
                BCMX_LPLIST_PBMP_ADD(&data->l2_ports,
                                     bcm_unit, bcm_ipmc.l2_pbmp);
                BCMX_LPLIST_PBMP_ADD(&data->l2_untag_ports,
                                     bcm_unit, bcm_ipmc.l2_ubmp);
                BCMX_LPLIST_PBMP_ADD(&data->l3_ports,
                                     bcm_unit, bcm_ipmc.l3_pbmp);
            }
        }
    }

    return rv;
}

/*
 * Function:
 *      bcmx_ipmc_find
 * Purpose:
 *      Given a SrcIP, McIP and Vlan, returns matching entry
 * Parameters:
 *      data - (IN)  accepts SrcIP, McIP, Vlan
 *             (OUT) will have MCIndex, L2_Ports, L3_Ports, Untag_Ports
 *                   filled in the same structure      
 */
int
bcmx_ipmc_find(bcmx_ipmc_addr_t *data)
{
    int         rv = BCM_E_UNAVAIL, tmp_rv;
    int			i, bcm_unit;
    bcm_ipmc_addr_t     bcm_ipmc;
    int                 convert_rv = BCM_E_PORT;
    int                 successes = 0;

    BCMX_IPMC_INIT_CHECK;

    BCMX_PARAM_NULL_CHECK(data);

    _bcmx_ipmc_addr_t_to_bcm(data, &bcm_ipmc);

    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_ipmc_find(bcm_unit, &bcm_ipmc);

        if (BCMX_IPMC_GET_IS_VALID(bcm_unit, tmp_rv)) {
            rv = tmp_rv;
            
            if (BCM_FAILURE(tmp_rv)) {
                break;
            }

            successes++;
            data->flags |= bcm_ipmc.flags;
            BCMX_LPLIST_PBMP_ADD(&data->l2_ports,
                                 bcm_unit, bcm_ipmc.l2_pbmp);
            BCMX_LPLIST_PBMP_ADD(&data->l2_untag_ports,
                                 bcm_unit, bcm_ipmc.l2_ubmp);
            BCMX_LPLIST_PBMP_ADD(&data->l3_ports,
                                 bcm_unit, bcm_ipmc.l3_pbmp);

            if (BCM_FAILURE(convert_rv)) {
                /* The following data items will be gathered from the
                   first device found. The initial value for
                   convert_rv forces this to be called at least once.
                   This will continue to be called until a trunk is
                   found or a port is successfully converted. */

                if (!(bcm_ipmc.flags & BCM_IPMC_SOURCE_PORT_NOCHECK)) {
                    if (bcm_ipmc.ts) {
                        /* trunk */
                        data->port_tgid = bcm_ipmc.port_tgid;
                        convert_rv = BCM_E_NONE;
                    } else {
                        /* port */
                        convert_rv =
                            _bcmx_dest_from_modid_port(&data->port_tgid,
                                                      bcm_ipmc.mod_id,
                                                      bcm_ipmc.port_tgid,
                                                      BCMX_DEST_CONVERT_DEFAULT);
                        if (BCM_FAILURE(convert_rv)) {
                            /* If there was an error, see if there's a
                               subsequent unit with a convertible port. */
                            continue;
                        }
                    }
                }
                data->cos = bcm_ipmc.cos;
                data->ts = bcm_ipmc.ts;
                data->v = bcm_ipmc.v;
                data->ipmc_index = bcm_ipmc.ipmc_index;
                data->lookup_class = bcm_ipmc.lookup_class;
            }
        }
    }

    if (BCM_SUCCESS(rv) && successes > 0) {
        /* If there have been otherwise no failures, and there was
           more than one attempt to convert returned IPMC data, then
           the final return value is the IPMC port converion
           result. */
        rv = convert_rv;
    }

    return  rv;
}

/*
 * Function:
 *      bcmx_ipmc_entry_enable_set
 */

int
bcmx_ipmc_entry_enable_set(bcm_ip_t s_ip_addr,
			   bcm_ip_t mc_ip_addr,
			   bcm_vlan_t vid,
			   int enable)
{
    int rv = BCM_E_UNAVAIL, tmp_rv;
    int i, bcm_unit;

    BCMX_IPMC_INIT_CHECK;

    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_ipmc_entry_enable_set(bcm_unit, s_ip_addr,
					   mc_ip_addr, vid, enable);
        BCM_IF_ERROR_RETURN(BCMX_IPMC_SET_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}

/*
 * Function:
 *      bcmx_ipmc_cos_set
 */

int
bcmx_ipmc_cos_set(bcm_ip_t s_ip_addr,
		  bcm_ip_t mc_ip_addr,
		  bcm_vlan_t vid,
		  int cos)
{
    int rv = BCM_E_UNAVAIL, tmp_rv;
    int i, bcm_unit;

    BCMX_IPMC_INIT_CHECK;

    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_ipmc_cos_set(bcm_unit, s_ip_addr, mc_ip_addr, vid, cos);
        BCM_IF_ERROR_RETURN(BCMX_IPMC_SET_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}


/*
 * Function:
 *      bcmx_ipmc_port_tgid_set
 */

int
bcmx_ipmc_port_tgid_set(bcm_ip_t s_ip_addr,
			bcm_ip_t mc_ip_addr,
			bcm_vlan_t vid,
			int ts,
			int port_tgid)
{
    int rv = BCM_E_UNAVAIL, tmp_rv;
    int i, bcm_unit;
    bcm_module_t modid;
    bcm_port_t   modport;

    BCMX_IPMC_INIT_CHECK;

    if (!ts) {
        BCM_IF_ERROR_RETURN
            (_bcmx_dest_to_modid_port((bcmx_lport_t)port_tgid,
                                      &modid, &modport,
                                      BCMX_DEST_CONVERT_DEFAULT));
    } else {
        modid = -1;
        modport = port_tgid;
    }

    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_ipmc_port_tgid_set(bcm_unit, s_ip_addr, mc_ip_addr,
                                        vid, ts, (int)modport);
        BCM_IF_ERROR_RETURN
            (BCMX_IPMC_SET_ERROR_CHECK(bcm_unit, tmp_rv, &rv));

        if (!ts) {
            tmp_rv = bcm_ipmc_port_modid_set(bcm_unit, s_ip_addr, mc_ip_addr,
                                             vid, (int)modid);
            BCM_IF_ERROR_RETURN
                (BCMX_IPMC_SET_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
        }
    }

    return rv;
}

/*
 * Function:
 *      bcmx_ipmc_add_l2_ports
 */

int
bcmx_ipmc_add_l2_ports(bcm_ip_t s_ip_addr,
		       bcm_ip_t mc_ip_addr,
		       bcm_vlan_t vid,
		       bcmx_lplist_t lplist,
		       bcmx_lplist_t ut_lplist)
{
    int rv = BCM_E_UNAVAIL, tmp_rv;
    int i, bcm_unit;
    bcm_pbmp_t	pbmp, ubmp;

    BCMX_IPMC_INIT_CHECK;

    BCMX_UNIT_ITER(bcm_unit, i) {
        BCMX_LPLIST_TO_PBMP(lplist, bcm_unit, pbmp);
        if (BCM_PBMP_IS_NULL(pbmp)) {
            continue;
        }
        BCMX_LPLIST_TO_PBMP(ut_lplist, bcm_unit, ubmp);
        tmp_rv = bcm_ipmc_add_l2_ports(bcm_unit, s_ip_addr, mc_ip_addr, vid,
                                       pbmp, ubmp);
        BCM_IF_ERROR_RETURN(BCMX_IPMC_SET_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}


/*
 * Function:
 *      bcmx_ipmc_delete_l2_ports
 */

int
bcmx_ipmc_delete_l2_ports(bcm_ip_t s_ip_addr,
			  bcm_ip_t mc_ip_addr,
			  bcm_vlan_t vid,
			  bcmx_lport_t port)
{
    int         bcm_unit;
    bcm_port_t  bcm_port;

    BCMX_IPMC_INIT_CHECK;

    BCM_IF_ERROR_RETURN(_bcmx_dest_to_unit_port(port, &bcm_unit, &bcm_port,
                                                BCMX_DEST_CONVERT_DEFAULT));

    return bcm_ipmc_delete_l2_ports(bcm_unit,
                                    s_ip_addr, mc_ip_addr, vid, bcm_port);
}


/*
 * Function:
 *      bcmx_ipmc_add_l3_ports
 */

int
bcmx_ipmc_add_l3_ports(bcm_ip_t s_ip_addr,
		       bcm_ip_t mc_ip_addr,
		       bcm_vlan_t vid,
		       bcmx_lplist_t lplist)
{
    int rv = BCM_E_UNAVAIL, tmp_rv;
    int i, bcm_unit;
    bcm_pbmp_t	pbmp;

    BCMX_IPMC_INIT_CHECK;

    BCMX_UNIT_ITER(bcm_unit, i) {
        BCMX_LPLIST_TO_PBMP(lplist, bcm_unit, pbmp);
        if (BCM_PBMP_IS_NULL(pbmp)) {
            continue;
        }
        tmp_rv = bcm_ipmc_add_l3_ports(bcm_unit, s_ip_addr, mc_ip_addr, vid,
                                       pbmp);
        BCM_IF_ERROR_RETURN(BCMX_IPMC_SET_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}


/*
 * Function:
 *      bcmx_ipmc_delete_l3_port
 */

int
bcmx_ipmc_delete_l3_port(bcm_ip_t s_ip_addr,
			 bcm_ip_t mc_ip_addr,
			 bcm_vlan_t vid,
			 bcmx_lport_t port)
{
    int         bcm_unit;
    bcm_port_t  bcm_port;

    BCMX_IPMC_INIT_CHECK;

    BCM_IF_ERROR_RETURN(_bcmx_dest_to_unit_port(port, &bcm_unit, &bcm_port,
                                                BCMX_DEST_CONVERT_DEFAULT));

    return bcm_ipmc_delete_l3_port(bcm_unit,
                                   s_ip_addr, mc_ip_addr, vid, bcm_port);
}


/*
 * Function:
 *      bcmx_ipmc_egress_port_init
 */

int
bcmx_ipmc_egress_port_init(void)
{
    int rv = BCM_E_UNAVAIL, tmp_rv;
    int i, bcm_unit;

    BCMX_IPMC_INIT_CHECK;

    BCMX_UNIT_ITER(bcm_unit, i) {
        tmp_rv = bcm_ipmc_egress_port_init(bcm_unit);
        BCM_IF_ERROR_RETURN(BCMX_IPMC_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}


/*
 * Function:
 *      bcmx_ipmc_egress_port_set
 */

int
bcmx_ipmc_egress_port_set(bcmx_lport_t port,
			  const bcm_mac_t mac,
			  int untag,
			  bcm_vlan_t vid,
			  int ttl_threshold)
{
    int         bcm_unit;
    bcm_port_t  bcm_port;

    BCMX_IPMC_INIT_CHECK;

    BCM_IF_ERROR_RETURN(_bcmx_dest_to_unit_port(port, &bcm_unit, &bcm_port,
                                                BCMX_DEST_CONVERT_DEFAULT));

    return bcm_ipmc_egress_port_set(bcm_unit, bcm_port,
                                    mac, untag, vid, ttl_threshold);
}


/*
 * Function:
 *      bcmx_ipmc_egress_port_get
 */

int
bcmx_ipmc_egress_port_get(bcmx_lport_t port,
			  bcm_mac_t mac,
			  int *untag,
			  bcm_vlan_t *vid,
			  int *ttl_threshold)
{
    int         bcm_unit;
    bcm_port_t  bcm_port;

    BCMX_IPMC_INIT_CHECK;

    BCMX_PARAM_NULL_CHECK(untag);
    BCMX_PARAM_NULL_CHECK(vid);
    BCMX_PARAM_NULL_CHECK(ttl_threshold);

    BCM_IF_ERROR_RETURN(_bcmx_dest_to_unit_port(port, &bcm_unit, &bcm_port,
                                                BCMX_DEST_CONVERT_DEFAULT));

    return bcm_ipmc_egress_port_get(bcm_unit, bcm_port,
                                    mac, untag, vid, ttl_threshold);
}


/*
 * Function:
 *      bcmx_ipmc_counters_get
 */

int
bcmx_ipmc_counters_get(bcmx_lport_t port,
		       bcm_ipmc_counters_t *counters)
{
    int         bcm_unit;
    bcm_port_t  bcm_port;

    BCMX_IPMC_INIT_CHECK;

    BCMX_PARAM_NULL_CHECK(counters);

    BCM_IF_ERROR_RETURN(_bcmx_dest_to_unit_port(port, &bcm_unit, &bcm_port,
                                                BCMX_DEST_CONVERT_DEFAULT));

    return bcm_ipmc_counters_get(bcm_unit, bcm_port, counters);
}


/*
 * Function:
 *      bcmx_ipmc_repl_get
 */

int
bcmx_ipmc_repl_get(int index,
		   bcmx_lport_t port,
		   bcm_vlan_vector_t vlan_vec)
{
    int         bcm_unit;
    bcm_port_t  bcm_port;

    BCMX_IPMC_INIT_CHECK;

    BCM_IF_ERROR_RETURN(_bcmx_dest_to_unit_port(port, &bcm_unit, &bcm_port,
                                                BCMX_DEST_CONVERT_DEFAULT));

    return bcm_ipmc_repl_get(bcm_unit, index, bcm_port, vlan_vec);
}


/*
 * Function:
 *      bcmx_ipmc_repl_add
 */

int
bcmx_ipmc_repl_add(int index,
		   bcmx_lplist_t lplist,
		   bcm_vlan_t vlan)
{
    int rv = BCM_E_UNAVAIL, tmp_rv;
    int i, bcm_unit;
    bcm_pbmp_t	pbmp;

    BCMX_IPMC_INIT_CHECK;

    BCMX_UNIT_ITER(bcm_unit, i) {
        BCMX_LPLIST_TO_PBMP(lplist, bcm_unit, pbmp);
        if (BCM_PBMP_IS_NULL(pbmp)) {
            continue;
        }
        tmp_rv = bcm_ipmc_repl_add(bcm_unit, index, pbmp, vlan);
        BCM_IF_ERROR_RETURN(BCMX_IPMC_SET_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}


/*
 * Function:
 *      bcmx_ipmc_repl_delete
 */

int
bcmx_ipmc_repl_delete(int index,
		      bcmx_lplist_t lplist,
		      bcm_vlan_t vlan)
{
    int rv = BCM_E_UNAVAIL, tmp_rv;
    int i, bcm_unit;
    bcm_pbmp_t	pbmp;

    BCMX_IPMC_INIT_CHECK;

    BCMX_UNIT_ITER(bcm_unit, i) {
        BCMX_LPLIST_TO_PBMP(lplist, bcm_unit, pbmp);
        if (BCM_PBMP_IS_NULL(pbmp)) {
            continue;
        }
        tmp_rv = bcm_ipmc_repl_delete(bcm_unit, index, pbmp, vlan);
        BCM_IF_ERROR_RETURN
            (BCMX_IPMC_DELETE_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}


/*
 * Function:
 *      bcmx_ipmc_repl_delete_all
 */

int
bcmx_ipmc_repl_delete_all(int index,
			  bcmx_lplist_t lplist)
{
    int rv = BCM_E_UNAVAIL, tmp_rv;
    int i, bcm_unit;
    bcm_pbmp_t	pbmp;

    BCMX_IPMC_INIT_CHECK;

    BCMX_UNIT_ITER(bcm_unit, i) {
        BCMX_LPLIST_TO_PBMP(lplist, bcm_unit, pbmp);
        if (BCM_PBMP_IS_NULL(pbmp)) {
            continue;
        }
        tmp_rv = bcm_ipmc_repl_delete_all(bcm_unit, index, pbmp);
        BCM_IF_ERROR_RETURN
            (BCMX_IPMC_DELETE_ERROR_CHECK(bcm_unit, tmp_rv, &rv));
    }

    return rv;
}

int
bcmx_ipmc_repl_set(int index,
		   bcmx_lport_t port,
		   bcm_vlan_vector_t vlan_vec)
{
    int         bcm_unit;
    bcm_port_t  bcm_port;

    BCMX_IPMC_INIT_CHECK;

    BCM_IF_ERROR_RETURN(_bcmx_dest_to_unit_port(port, &bcm_unit, &bcm_port,
                                                BCMX_DEST_CONVERT_DEFAULT));

    return bcm_ipmc_repl_set(bcm_unit, index, bcm_port, vlan_vec);
}

/*
 * bcmx_ipmc_egress_intf_add
 */
int
bcmx_ipmc_egress_intf_add(int group,
			  bcmx_lport_t port, 
                          bcm_l3_intf_t *l3_intf)
{
    int         bcm_unit;
    bcm_port_t  bcm_port;

    BCMX_IPMC_INIT_CHECK;

    BCMX_PARAM_NULL_CHECK(l3_intf);

    BCM_IF_ERROR_RETURN(_bcmx_dest_to_unit_port(port, &bcm_unit, &bcm_port,
                                                BCMX_DEST_CONVERT_DEFAULT));

    return bcm_ipmc_egress_intf_add(bcm_unit, group, bcm_port, l3_intf);
}

/*
 * bcmx_ipmc_egress_intf_delete
 */
int
bcmx_ipmc_egress_intf_delete(int group,
			     bcmx_lport_t port, 
                             bcm_l3_intf_t *l3_intf)
{
    int         bcm_unit;
    bcm_port_t  bcm_port;

    BCMX_IPMC_INIT_CHECK;

    BCMX_PARAM_NULL_CHECK(l3_intf);

    BCM_IF_ERROR_RETURN(_bcmx_dest_to_unit_port(port, &bcm_unit, &bcm_port,
                                                BCMX_DEST_CONVERT_DEFAULT));

    return bcm_ipmc_egress_intf_delete(bcm_unit, group, bcm_port, l3_intf);
}

/*
 * bcmx_ipmc_egress_intf_delete_all
 */
int
bcmx_ipmc_egress_intf_delete_all(int group, bcmx_lport_t port)
{
    int         bcm_unit;
    bcm_port_t  bcm_port;

    BCMX_IPMC_INIT_CHECK;

    BCM_IF_ERROR_RETURN(_bcmx_dest_to_unit_port(port, &bcm_unit, &bcm_port,
                                                BCMX_DEST_CONVERT_DEFAULT));

    return bcm_ipmc_egress_intf_delete_all(bcm_unit, group, bcm_port);
}

int
bcmx_ipmc_egress_intf_set(int group, bcmx_lport_t port,
			  int if_count, bcm_if_t *if_array)
{
    int         bcm_unit;
    bcm_port_t  bcm_port;

    BCMX_IPMC_INIT_CHECK;

    BCMX_PARAM_ARRAY_NULL_CHECK(if_count, if_array);

    BCM_IF_ERROR_RETURN(_bcmx_dest_to_unit_port(port, &bcm_unit, &bcm_port,
                                                BCMX_DEST_CONVERT_DEFAULT));

    return bcm_ipmc_egress_intf_set(bcm_unit, group, bcm_port,
                                    if_count, if_array);
}

int
bcmx_ipmc_egress_intf_get(int group, bcmx_lport_t port,
			  int if_max, bcm_if_t *if_array,
			  int *if_count)
{
    int         bcm_unit;
    bcm_port_t  bcm_port;

    BCMX_IPMC_INIT_CHECK;

    BCMX_PARAM_NULL_CHECK(if_count);
    BCMX_PARAM_ARRAY_NULL_CHECK(if_max, if_array);

    BCM_IF_ERROR_RETURN(_bcmx_dest_to_unit_port(port, &bcm_unit, &bcm_port,
                                                BCMX_DEST_CONVERT_DEFAULT));

    return bcm_ipmc_egress_intf_get(bcm_unit, group, bcm_port,
                                    if_max, if_array, if_count);
}

/*
 * Function:
 *     bcmx_ipmc_age
 */

int
bcmx_ipmc_age(uint32 flags,
                 bcm_ipmc_traverse_cb age_cb,
                 void *user_data)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *    bcmx_ipmc_traverse
 */

int
bcmx_ipmc_traverse(uint32 flags,
                      bcm_ipmc_traverse_cb cb,
                      void *user_data)
{
    int rv;
    int i, bcm_unit;

    BCMX_IPMC_INIT_CHECK;

    BCMX_LOCAL_UNIT_ITER(bcm_unit, i) {
        rv = bcm_ipmc_traverse(bcm_unit, flags, cb, user_data);
        if (BCMX_IPMC_GET_IS_VALID(bcm_unit, rv)) {
            return rv;
        }
    }

    return BCM_E_UNAVAIL;
}


#endif	/* INCLUDE_L3 */

int _bcmx_ipmc_not_empty;
