/*
 * $Id: auth.c 1.44.6.1 Broadcom SDK $
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
 * 802.1X
 */

#include <sal/core/libc.h>

#include <soc/drv.h>

#include <bcm/error.h>
#include <bcm/auth.h>

#include <bcm_int/esw/field.h> /* _bcm_field_setup_post_ethertype_udf() */

#include <bcm_int/esw_dispatch.h>

#define AUTH_INIT(unit) \
        if (!SOC_UNIT_VALID(unit)) { return (BCM_E_UNIT); } \
        else if (!(soc_feature(unit, soc_feature_field) || \
                   soc_feature(unit, soc_feature_filter))) { \
            return (BCM_E_UNAVAIL); \
        } \
        else if (NUM_E_PORT(unit) <= 0) { return (BCM_E_BADID); } \
        else if (auth_cntl[unit] == NULL) { return (BCM_E_INIT); }
#define AUTH_INIT_NO_PORT(unit) \
        if (!SOC_UNIT_VALID(unit)) { return (BCM_E_UNIT); } \
        else if (!(soc_feature(unit, soc_feature_field) || \
                   soc_feature(unit, soc_feature_filter))) { \
            return (BCM_E_UNAVAIL); \
        } \
        else if (auth_cntl[unit] == NULL) { return (BCM_E_INIT); }
#define AUTH_PORT(unit, port) \
        if (!SOC_PORT_VALID(unit, port) || !IS_E_PORT(unit, port)) \
        { return (BCM_E_PORT); }

#define AUTH_MODE_MASK \
        (BCM_AUTH_MODE_UNCONTROLLED | BCM_AUTH_MODE_AUTH | BCM_AUTH_MODE_UNAUTH)

#define _AUTH_ETHERTYPE_EAPOL 0x888e
#define _AUTH_ETHERTYPE_MASK  0xffff

typedef struct auth_mac_s *auth_mac_p;

typedef struct auth_mac_s {
    bcm_mac_t         mac;  
    bcm_filterid_t    filter;
    bcm_field_entry_t entry;
    pbmp_t            pbmp;
    auth_mac_p        next;
} auth_mac_t;

typedef struct bcm_auth_cntl_s {
    int              mode;
    int              etmp;
    int              mac_set;
    auth_mac_t       *macList;
} bcm_auth_cntl_t;

typedef struct auth_cb_cntl_s {
    int             registered;
    bcm_auth_cb_t   auth_cbs;
    void            *auth_cb_data;
} auth_cb_cntl_t;

typedef struct auth_field_s {
    int               inited;
    int               count;
    bcm_field_group_t group0;
    auth_mac_t        *macList;
    bcm_field_group_t group1;
    bcm_field_entry_t entry1;
    bcm_field_group_t group2;
    bcm_field_entry_t entry2;
} auth_field_cntl_t;

static bcm_auth_cntl_t *auth_cntl[BCM_MAX_NUM_UNITS];
static auth_cb_cntl_t cb_cntl[BCM_MAX_NUM_UNITS];
static auth_field_cntl_t fp_cntl[BCM_MAX_NUM_UNITS];

/* Forward declarations */
STATIC void _auth_linkscan_cb(int unit, bcm_port_t port, bcm_port_info_t *info);
STATIC int _auth_filter_install(int unit, int port, bcm_mac_t mac, 
                                bcm_filterid_t *fid);
STATIC int _auth_filter_remove(int unit, bcm_filterid_t fid);
STATIC int _auth_filter_install_all(int unit, int port);
STATIC int _auth_filter_remove_all(int unit, int port);
STATIC int _auth_field_install(int unit, int port, bcm_mac_t mac); 
STATIC int _auth_field_remove(int unit, int port, bcm_mac_t mac); 
STATIC int _auth_field_install_all(int unit, int port);
STATIC int _auth_field_remove_all(int unit, int port);
#if defined(BCM_FILTER_SUPPORT) || defined(BCM_FIELD_SUPPORT)
STATIC int _auth_maclist_insert(auth_mac_t **list, bcm_mac_t mac, auth_mac_p *ins);
STATIC int _auth_maclist_lookup(auth_mac_t **list, bcm_mac_t mac, auth_mac_p *entry);
#endif /* !BCM_FILTER_SUPPORT && !BCM_FIELD_SUPPORT */
STATIC int _auth_maclist_remove(auth_mac_t **list, bcm_mac_t mac, auth_mac_p *del);
STATIC int _auth_maclist_destroy(auth_mac_t **list);
#ifdef AUTH_DEBUG
STATIC int _auth_maclist_dump(auth_mac_t **list);
#endif

/*
 * Function:
 *	bcm_auth_init
 * Purpose:
 *	Initialize auth module.
 * Parameters:
 *	unit - Device number
 * Returns:
 *     BCM_E_NONE     - Success
 *     BCM_E_UNIT     - Invalid unit number
 *     BCM_E_UNAVAIL  - Insufficient hardware support
 *     BCM_E_MEMORY   - Allocation failure
 *
 * Notes:
 *	All ports are marked as being in the uncontrolled state.
 */

int
bcm_esw_auth_init(int unit)
{
    bcm_port_t   port;
    int          rv;

    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    if (!(soc_feature(unit, soc_feature_field) ||
          soc_feature(unit, soc_feature_filter))) {
        return (BCM_E_UNAVAIL);
    }

    if (auth_cntl[unit] != NULL) {
        rv = bcm_esw_auth_detach(unit);
        BCM_IF_ERROR_RETURN(rv);
    }

    auth_cntl[unit] = sal_alloc((MAX_PORT(unit)+1) * 
                                sizeof(bcm_auth_cntl_t), "auth_cntl");
    if (auth_cntl[unit] == NULL) {
        return BCM_E_MEMORY;
    }
    sal_memset(auth_cntl[unit], 0, (MAX_PORT(unit)+1) * 
               sizeof(bcm_auth_cntl_t));

    for (port = 0; port < (MAX_PORT(unit)+1); port++) {
        auth_cntl[unit][port].mode = BCM_AUTH_MODE_UNCONTROLLED;
        if (soc_feature(unit, soc_feature_field)) {
            BCM_IF_ERROR_RETURN(_auth_field_remove_all(unit, port));
        } else if (soc_feature(unit, soc_feature_filter)) {
            BCM_IF_ERROR_RETURN(_auth_filter_remove_all(unit, port));
        }
    }
    rv = bcm_esw_linkscan_register(unit, _auth_linkscan_cb);
    BCM_IF_ERROR_RETURN(rv);
    cb_cntl[unit].registered = TRUE;

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_auth_detach
 * Purpose:
 *	Stop all auth module processing and deallocate resources.
 * Parameters:
 *	unit - Device number
 * Returns:
 *     BCM_E_NONE     - Success
 *     BCM_E_UNIT     - Invalid unit number
 *     BCM_E_UNAVAIL  - Insufficient hardware support
 * Notes:
 *	All ports are moved to the uncontrolled state.  All internal
 *	callbacks and filters are removed.
 */

int
bcm_esw_auth_detach(int unit)
{
    bcm_port_t        port;
    int rv;

    if (((0 == (soc_feature(unit, soc_feature_field))) && 
         (0 == (soc_feature(unit, soc_feature_filter)))) ||
        (auth_cntl[unit] == NULL)) { 
        return (BCM_E_NONE);
    } 

    AUTH_INIT_NO_PORT(unit);

    if (0 == SOC_HW_ACCESS_DISABLE(unit)) {
        for (port = 0; port < (MAX_PORT(unit)+1); port++) {
            if (IS_E_PORT(unit, port)) {
                if (soc_feature(unit, soc_feature_field)) {
                    BCM_IF_ERROR_RETURN(_auth_field_remove_all(unit, port));
                } else if (soc_feature(unit, soc_feature_filter)) {
                    BCM_IF_ERROR_RETURN(_auth_filter_remove_all(unit, port));
                }
                _auth_maclist_destroy(&auth_cntl[unit][port].macList);
                bcm_esw_port_learn_set(unit, port, 
                                       BCM_PORT_LEARN_ARL | BCM_PORT_LEARN_FWD);
                bcm_esw_port_stp_set(unit, port, BCM_STG_STP_FORWARD);
                auth_cntl[unit][port].mode = BCM_AUTH_MODE_UNCONTROLLED;
                auth_cntl[unit][port].etmp = FALSE;
            }
        }
    }

    for (port = 0; port < (MAX_PORT(unit)+1); port++) {
        _auth_maclist_destroy(&auth_cntl[unit][port].macList);
    }

    fp_cntl[unit].inited = FALSE;
    fp_cntl[unit].count = 0;

    if (cb_cntl[unit].registered) {
        rv = bcm_esw_linkscan_unregister(unit, _auth_linkscan_cb); 
        /* If linkscan thread was restarted callback might be gone. */
        if ((BCM_FAILURE(rv)) && (BCM_E_NOT_FOUND != rv)) {
            return (rv);
        }
        cb_cntl[unit].registered = FALSE;
    }
    sal_free(auth_cntl[unit]);
    auth_cntl[unit] = NULL;
    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_auth_mode_set
 * Purpose:
 *	Set the 802.1X operating mode
 * Parameters:
 *	unit - Device number
 *	port - Port number, -1 to set all non-stack-ports
 *	mode - One of BCM_AUTH_MODE_XXX and other flags (see below)
 * Returns:
 *     BCM_E_NONE     - Success
 *     BCM_E_UNIT     - Invalid unit number
 *     BCM_E_UNAVAIL  - Insufficient hardware support
 *     BCM_E_PORT     - Invalid port
 * Notes:
 *	While in the uncontrolled state, any packets may flow in or out
 *	of the port and normal L2 learning takes place.
 *
 *	If mode is BCM_AUTH_UNAUTH, a set of flags may be ORed into
 *	the mode word:
 *		BCM_AUTH_BLOCK_IN	allow outgoing packets
 *		BCM_AUTH_BLOCK_INOUT	do not allow in or out packets
 *					(default)
 *	While in the unauthorized state, all L2 MAC addresses
 *	associated with the port are removed, L2 learning is disabled,
 *	and packet transfer is blocked as specified by the
 *	BCM_AUTH_BLOCK_* flags.  Incoming EAPOL frames are allowed
 *	either addressed as BPDUs or to the switch CPU's MAC address.
 *	Some hardware may forward all EAPOL frames regardless of the
 *	destination MAC address.  These will be delivered to the CPU
 *	for processing.
 *
 *	Outgoing EAPOL frames to be sent must be sent with a pair of
 *	bcm_auth_egress_set calls, enabling the egress before the
 *	EAPOL packet and disabling the egress after the packet is
 *	sent.  If the BCM_AUTH_BLOCK_IN flag has been given, then the
 *	bcm_auth_egress_set calls will not do anything.
 *
 *	If mode is BCM_AUTH_MODE_AUTH, a set of flags may be ORed into 
 *      the mode word:
 *		BCM_AUTH_LEARN		allow L2 learning while authorized
 *		BCM_AUTH_IGNORE_LINK	do not unauthorize upon link down
 *		BCM_AUTH_IGNORE_VIOLATION do not unauth upon security
 *					 violation
 *	After moving the port to the authorized state, one or more
 *	static L2 entries may be added using bcm_l2_addr_add with the
 *	auth field set.  These L2 addresses would typically be for
 *	the source MAC address of the requesting EAPOL frame that was
 *	authorized, in as many VLANs as are configured for the
 *	authorized user.
 *
 *	While in the authorized state, any packets incoming from source
 *	MAC addresses set by bcm_l2_addr_add will be accepted for
 *	transfer.  Any other source MAC addresses will be accepted if
 *	BCM_AUTH_LEARN was set.  If BCM_AUTH_LEARN was not set and the
 *	hardware supports it and BCM_AUTH_IGNORE_VIOLATION is not set,
 *	then unknown source MAC addresses will cause a security
 *	violation and move the port to the unauthorized state.
 *
 *	If the link goes down on the port while in the authorized
 *	state and BCM_AUTH_IGNORE_LINK is not set, then the port will
 *	be moved to the unauthorized state.
 */

int
bcm_esw_auth_mode_set(int unit, int port, uint32 mode)
{
    pbmp_t   pbmp;
    bcm_port_t p;

    AUTH_INIT(unit);

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port, &port));
    }

    if (port >= 0) {
        AUTH_PORT(unit, port);
    }

    if (port < 0) { 
        pbmp = PBMP_E_ALL(unit);
    } else {
        BCM_PBMP_PORT_SET(pbmp, port);
    }

    /* Remove stack ports from bitmap */
    BCM_PBMP_REMOVE(pbmp, SOC_PBMP_STACK_CURRENT(unit));

    BCM_PBMP_ITER(pbmp, p) {
        switch (mode & AUTH_MODE_MASK) {
        case BCM_AUTH_MODE_UNCONTROLLED:
            if (soc_feature(unit, soc_feature_field)) {
                BCM_IF_ERROR_RETURN(_auth_field_remove_all(unit, p));
            } else if (soc_feature(unit, soc_feature_filter)) {
                BCM_IF_ERROR_RETURN(_auth_filter_remove_all(unit, p));
            }
            bcm_esw_port_learn_set(unit, p, BCM_PORT_LEARN_ARL |
                                   BCM_PORT_LEARN_FWD);
            bcm_esw_port_stp_set(unit, p, BCM_STG_STP_FORWARD);
            auth_cntl[unit][p].etmp = FALSE;
            break;
        case BCM_AUTH_MODE_UNAUTH:
            /* Disable learning on that port */
            bcm_esw_port_learn_set(unit, p, BCM_PORT_LEARN_FWD);

            /* remove all L2 (MAC) addresses associated with the port */
            bcm_esw_l2_addr_delete_by_port(unit, -1, p, BCM_L2_DELETE_STATIC);

            if (mode & BCM_AUTH_BLOCK_IN) {
                if (soc_feature(unit, soc_feature_field)) {
                    _auth_field_install_all(unit, p); 
                } else if (soc_feature(unit, soc_feature_filter)) {
                     _auth_filter_install_all(unit, p); 
                }
            }
            else {
                /* unauthorized for both directions */
                if (soc_feature(unit, soc_feature_field)) {
                    BCM_IF_ERROR_RETURN(_auth_field_remove_all(unit, p));
                } else if (soc_feature(unit, soc_feature_filter)) {
                    BCM_IF_ERROR_RETURN(_auth_filter_remove_all(unit, p));
                }
                mode |= BCM_AUTH_BLOCK_INOUT;
                BCM_IF_ERROR_RETURN(bcm_esw_port_stp_set(unit, p,
                                                         BCM_STG_STP_BLOCK));
            }
            break;
        case BCM_AUTH_MODE_AUTH:
            /* No learn, Send to CPU, Drop the packet */
            bcm_esw_port_learn_set(unit, p, BCM_PORT_LEARN_CPU);
            if (soc_feature(unit, soc_feature_field)) {
                BCM_IF_ERROR_RETURN(_auth_field_remove_all(unit, p));
            } else if (soc_feature(unit, soc_feature_filter)) {
                BCM_IF_ERROR_RETURN(_auth_filter_remove_all(unit, p));
            }
            bcm_esw_port_stp_set(unit, p, BCM_STG_STP_FORWARD);
            if (mode & BCM_AUTH_LEARN) {
                bcm_esw_port_learn_set(unit, p, BCM_PORT_LEARN_ARL
                                       | BCM_PORT_LEARN_FWD);
            }
            auth_cntl[unit][p].etmp = FALSE;
            break;
        default:
            return BCM_E_PARAM;
        }
        auth_cntl[unit][p].mode = mode;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_auth_mode_get
 * Purpose:
 *	Get the 802.1X operating mode
 * Parameters:
 *	unit - Device number
 *	port - Port number
 *	mode - (OUT) One of BCM_AUTH_MODE_XXX and other flags
 * Returns:
 *     BCM_E_NONE     - Success
 *     BCM_E_UNIT     - Invalid unit number
 *     BCM_E_UNAVAIL  - Insufficient hardware support
 *     BCM_E_PORT     - Invalid port
 * Notes:
 */

int
bcm_esw_auth_mode_get(int unit, int port, uint32 *modep)
{

    AUTH_INIT(unit);
    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port, &port));
    }
    AUTH_PORT(unit, port);

    *modep = auth_cntl[unit][port].mode;

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_auth_unauth_callback
 * Purpose:
 *	Set the callback function for 802.1X notifications
 * Parameters:
 *	unit - Device number
 *	func - Callback function
 *	cookie - Arbitrary value passed along to callback function
 * Returns:
 *     BCM_E_NONE     - Success
 *     BCM_E_UNIT     - Invalid unit number
 *     BCM_E_UNAVAIL  - Insufficient hardware support
 * Notes:
 *	Calls func when a port on the unit has been moved from
 *	authorized to unauthorized state.  Reason can be one of:
 *		BCM_AUTH_REASON_UNKNOWN
 *		BCM_AUTH_REASON_LINK
 *		BCM_AUTH_REASON_VIOLATION
 */

int
bcm_esw_auth_unauth_callback(int unit, bcm_auth_cb_t func, void *cookie)
{
    AUTH_INIT_NO_PORT(unit);

    cb_cntl[unit].auth_cbs = func;
    cb_cntl[unit].auth_cb_data = cookie;

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_auth_egress_set
 * Purpose:
 *	Enable/disable the ability of packets to be sent out a port.
 * Parameters:
 *	unit - Device number
 *	port - Port number
 *	enable - TRUE to enable, FALSE to disable
 * Returns:
 *     BCM_E_NONE     - Success
 *     BCM_E_UNIT     - Invalid unit number
 *     BCM_E_UNAVAIL  - Insufficient hardware support
 *     BCM_E_PORT     - Invalid port
 * Notes:
 *	This call should only be used around calls that a CPU uses to
 *	transmit an EAPOL frame out the port.  If the port is in an
 *	unauthorized state with BCM_AUTH_BLOCK_IN set or is not in an
 *	unauthorized state, then this call does nothing.
 */

int
bcm_esw_auth_egress_set(int unit, int port, int enable)
{
    AUTH_INIT(unit);
    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port, &port));
    }
    AUTH_PORT(unit, port);

    if (enable) {
       if ((auth_cntl[unit][port].mode & BCM_AUTH_MODE_UNAUTH) &&
           !(auth_cntl[unit][port].mode & BCM_AUTH_BLOCK_IN)) {
           bcm_esw_port_stp_set(unit, port, BCM_STG_STP_FORWARD);
           if (soc_feature(unit, soc_feature_field)) {
               _auth_field_install_all(unit, port); 
           } else if (soc_feature(unit, soc_feature_filter)) {
               _auth_filter_install_all(unit, port); 
           }
           auth_cntl[unit][port].mode &= ~BCM_AUTH_BLOCK_INOUT;
           auth_cntl[unit][port].mode |= BCM_AUTH_BLOCK_IN;
           auth_cntl[unit][port].etmp= TRUE;
       }
    }
    else {
       if ((auth_cntl[unit][port].mode & BCM_AUTH_MODE_UNAUTH) &&
           (auth_cntl[unit][port].mode & BCM_AUTH_BLOCK_IN)) {
           if (soc_feature(unit, soc_feature_field)) {
               BCM_IF_ERROR_RETURN(_auth_field_remove_all(unit, port)); 
           } else if (soc_feature(unit, soc_feature_filter)) {
               BCM_IF_ERROR_RETURN(_auth_filter_remove_all(unit, port)); 
           }
           bcm_esw_port_stp_set(unit, port, BCM_STG_STP_BLOCK);
           auth_cntl[unit][port].mode &= ~BCM_AUTH_BLOCK_IN;
           auth_cntl[unit][port].mode |= BCM_AUTH_BLOCK_INOUT;
           auth_cntl[unit][port].etmp= FALSE;
       
       }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_auth_egress_get
 * Purpose:
 *	Return enable/disable state of packets being sent out a port.
 * Parameters:
 *	unit - Device number
 *	port - Port number
 *	enable - (OUT) TRUE if enabled, FALSE if disabled
 * Returns:
 *     BCM_E_NONE     - Success
 *     BCM_E_UNIT     - Invalid unit number
 *     BCM_E_UNAVAIL  - Insufficient hardware support
 *     BCM_E_PORT     - Invalid port
 * Notes:
 */

int
bcm_esw_auth_egress_get(int unit, int port, int *enable)
{
    AUTH_INIT(unit);
    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port, &port));
    }
    AUTH_PORT(unit, port);

    *enable = auth_cntl[unit][port].etmp;

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_auth_mac_add
 * Purpose:
 *      Add switch's MAC addresses 
 * Parameters:
 *      unit - Device number
 *	port - Port number, -1 to all ports
 *      mac -  Switch's MAC address
 * Returns:
 *     BCM_E_NONE     - Success
 *     BCM_E_UNIT     - Invalid unit number
 *     BCM_E_UNAVAIL  - Insufficient hardware support
 *     BCM_E_PORT     - Invalid port
 *     BCM_E_PARAM    - bad mac
 * Notes:
 */

int
bcm_esw_auth_mac_add(int unit, int port, bcm_mac_t mac)
{
    bcm_mac_t mac_zero = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; 
    bcm_mac_t mac_resv = {0x01, 0x80, 0xc2, 0x00, 0x00, 0x03}; 
    pbmp_t   pbm;
#if defined(BCM_FILTER_SUPPORT) || defined(BCM_FIELD_SUPPORT)
    auth_mac_p entry;
    int rv;
    bcm_port_t p;
#endif /* BCM_FILTER_SUPPORT || BCM_FIELD_SUPPORT */
#ifdef BCM_FILTER_SUPPORT
    bcm_filterid_t fid;
#endif /* BCM_FILTER_SUPPORT */

    AUTH_INIT(unit);

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port, &port));
    }

    if (port >= 0) {
        AUTH_PORT(unit, port);
    }

    if ((!sal_memcmp(mac, mac_zero, sizeof(bcm_mac_t))) ||
        (!sal_memcmp(mac, mac_resv, sizeof(bcm_mac_t)))) { 
        return BCM_E_PARAM;
    }

    if (port < 0) { 
        pbm = PBMP_E_ALL(unit);
    } else {
        BCM_PBMP_PORT_SET(pbm, port);
    }

#if defined(BCM_FILTER_SUPPORT) || defined(BCM_FIELD_SUPPORT)
    BCM_PBMP_ITER(pbm, p) {
        if ((rv = _auth_maclist_insert(&auth_cntl[unit][p].macList, 
                                       mac, &entry)) < 0) {
            return rv;
        }

        if (auth_cntl[unit][p].mac_set) {
            if (soc_feature(unit, soc_feature_field)) {
#ifdef BCM_FIELD_SUPPORT
                if ((rv = _auth_field_install(unit, p, mac)) < 0) {
                    _auth_maclist_remove(&auth_cntl[unit][p].macList, 
                                         mac, &entry);    
                    sal_free(entry);
                    return rv;
                }
#endif /* BCM_FIELD_SUPPORT */
            } else if (soc_feature(unit, soc_feature_filter)) {
#ifdef BCM_FILTER_SUPPORT
                if ((rv = _auth_filter_install(unit, p, mac, &fid)) < 0) {
                    _auth_maclist_remove(&auth_cntl[unit][p].macList, 
                                         mac, &entry);    
                    sal_free(entry);
                    return rv;
                }
                entry->filter = fid;
#endif /* BCM_FILTER_SUPPORT */
            }
        }
    }
   
    return BCM_E_NONE;
#else /* !BCM_FILTER_SUPPORT && !BCM_FIELD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* !BCM_FILTER_SUPPORT && !BCM_FIELD_SUPPORT */
}

/*
 * Function:
 *      bcm_auth_mac_delete
 * Purpose:
 *      Delete switch's MAC address.
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      mac  - Switch's MAC address 
 * Returns:
 *     BCM_E_NONE       - Success
 *     BCM_E_UNIT       - Invalid unit number
 *     BCM_E_UNAVAIL    - Insufficient hardware support
 *     BCM_E_PORT       - Invalid port
 *     BCM_E_PARAM      - bad mac
 *     BCM_E_NOT_FOUND  - MAC address not found
 * Notes:
 */

int
bcm_esw_auth_mac_delete(int unit, int port, bcm_mac_t mac)
{
    auth_mac_p entry;
    int rv;

    AUTH_INIT(unit);
    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port, &port));
    }
    AUTH_PORT(unit, port);

    rv = _auth_maclist_remove(&auth_cntl[unit][port].macList, mac, &entry);

    if (rv < 0) {
        return rv;
    }

    if (auth_cntl[unit][port].mac_set) {
        if (soc_feature(unit, soc_feature_field)) {
            BCM_IF_ERROR_RETURN(_auth_field_remove(unit, port, mac)); 
        } else if (soc_feature(unit, soc_feature_filter)) {
            BCM_IF_ERROR_RETURN(_auth_filter_remove(unit, entry->filter));
        }
    }

    sal_free(entry);
    
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_auth_mac_delete_all
 * Purpose:
 *      Delete all switch's MAC addresses.
 * Parameters:
 *      unit - Device number
 *	port - Port number 
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */

int
bcm_esw_auth_mac_delete_all(int unit, int port)
{
    int tmp, rv;

    AUTH_INIT(unit);
    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port, &port));
    }
    AUTH_PORT(unit, port);

    tmp = auth_cntl[unit][port].mac_set;
    if (soc_feature(unit, soc_feature_field)) {
        BCM_IF_ERROR_RETURN(_auth_field_remove_all(unit, port));
    } else if (soc_feature(unit, soc_feature_filter)) {
        BCM_IF_ERROR_RETURN(_auth_filter_remove_all(unit, port));
    }
    rv = _auth_maclist_destroy(&auth_cntl[unit][port].macList);
    auth_cntl[unit][port].mac_set = tmp;

    return rv;
}

/*
 * Function:
 *      _auth_linkscan_cb
 * Description:
 *      Put authorized state to unauthorized state if link down,
 *      given BCM_AUTH_IGNORE_LINK not set
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      info - pointer to structure giving status
 * Returns:
 *      None
 * Notes:
 */

STATIC void
_auth_linkscan_cb(int unit, bcm_port_t port, bcm_port_info_t *info)
{
    if ((auth_cntl[unit] != NULL) && (IS_E_PORT(unit, port))
        && !(auth_cntl[unit][port].mode & BCM_AUTH_MODE_UNCONTROLLED)) {
        if (!info->linkstatus) {
            if ((auth_cntl[unit][port].mode & BCM_AUTH_MODE_AUTH) &&
                !(auth_cntl[unit][port].mode & BCM_AUTH_IGNORE_LINK)) {
                bcm_esw_auth_mode_set(unit, port, BCM_AUTH_MODE_UNAUTH);
                if (cb_cntl[unit].auth_cbs) {
                    cb_cntl[unit].auth_cbs(cb_cntl[unit].auth_cb_data,
                                  unit, port, BCM_AUTH_REASON_LINK);
                }
            }
        }
    }
}

/*
 * Function:
 *      _auth_filter_install
 * Description:
 *      Install filter for accepting EAPOL frames 
 *      destinated for switch CPU's MAC address 
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      mac -  MAC address 
 *      f -  (OUT) Pointer to filter installed 
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */

STATIC int
_auth_filter_install(int unit, int port, bcm_mac_t mac, bcm_filterid_t *fid)
{
#ifdef BCM_FILTER_SUPPORT
    int rv = BCM_E_NONE; 
    pbmp_t   pbm;
    bcm_filterid_t   f;

    BCM_PBMP_PORT_SET(pbm, port);
    BCM_IF_ERROR_RETURN(bcm_esw_filter_create(unit, &f));
    bcm_esw_filter_qualify_ingress(unit, f, pbm);
    bcm_esw_filter_qualify_data(unit, f, 0, 6, mac,
                                (uint8 *)_bcm_filter_mac_all_ones);
    rv = bcm_esw_filter_qualify_data16(unit, f, 16, _AUTH_ETHERTYPE_EAPOL,
                                       _AUTH_ETHERTYPE_MASK);
    if (BCM_FAILURE(rv)) {
        bcm_esw_filter_destroy(unit, f);
        return rv;
    }
    rv = bcm_esw_filter_qualify_data8(unit, f, 18, 0x01, 0xff);
    if (BCM_FAILURE(rv)) {
        bcm_esw_filter_destroy(unit, f);
        return rv;
    }
    rv = bcm_esw_filter_action_match(unit, f, bcmActionSetPortAll, CMIC_PORT(unit));
    if (BCM_FAILURE(rv)) {
        bcm_esw_filter_destroy(unit, f);
        return rv;
    }
#ifdef AUTH_DEBUG
    rv = bcm_esw_filter_action_match(unit, f, bcmActionIncrCounter, 0);
    if (BCM_FAILURE(rv)) {
        bcm_esw_filter_destroy(unit, f);
        return rv;
    }
#endif
    rv = bcm_esw_filter_action_no_match(unit, f, bcmActionDoNotSwitch,0);
    if (BCM_FAILURE(rv)) {
        bcm_esw_filter_destroy(unit, f);
        return rv;
    }
    if ((rv = bcm_esw_filter_install(unit, f)) < 0) {
        bcm_esw_filter_destroy(unit, f);
        return rv;
    } else {
        *fid = f;
    }
#ifdef AUTH_DEBUG
    soc_cm_print("bcm_auth_filter_install:\n");
    bcm_esw_filter_dump(unit, f);
#endif

    return rv;
#else /* !BCM_FILTER_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* !BCM_FILTER_SUPPORT */
}

/*
 * Function:
 *      _auth_filter_install_all
 * Description:
 *      Install filters for accepting EAPOL frames
 *      destinated for switch CPU's MAC addresses
 * Parameters:
 *      unit - Device number
 *      port - Port number
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */

STATIC int
_auth_filter_install_all(int unit, int port)
{
    auth_mac_t **list = &auth_cntl[unit][port].macList;
    bcm_filterid_t fid;
    auth_mac_p entry;
    int rv;

    if (auth_cntl[unit][port].mac_set) {
        return BCM_E_EXISTS;
    }

    while (*list != NULL) {
        rv = _auth_filter_install(unit, port, (*list)->mac, &fid);
        if (rv < 0) {
            _auth_maclist_remove(list, (*list)->mac, &entry);
            sal_free(entry);
        } else {
            (*list)->filter = fid; 
            list = &(*list)->next;
        }
    }
    auth_cntl[unit][port].mac_set = TRUE;

    return BCM_E_NONE;
}

/*
 * Function:
 *      _auth_filter_remove_all
 * Description:
 *      Removed all installed filters in controlled state
 * Parameters:
 *      unit - Device number
 *      port - Port number
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */

STATIC int
_auth_filter_remove_all(int unit, int port)
{
    auth_mac_t **list = &auth_cntl[unit][port].macList;

    if (!auth_cntl[unit][port].mac_set) {
        return BCM_E_NONE;
    }
     
    while (*list != NULL) {
        _auth_filter_remove(unit, (*list)->filter);
        (*list)->filter = 0;
        list = &(*list)->next;
    }
    auth_cntl[unit][port].mac_set = FALSE;

    return BCM_E_NONE;
}

/*
 * Function:
 *      _auth_filter_remove
 * Description:
 *      Removed installed filter  
 * Parameters:
 *	unit - Device number
 *	fid  - Pointer to filter   
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */

STATIC int
_auth_filter_remove(int unit, bcm_filterid_t fid)
{
#ifdef BCM_FILTER_SUPPORT
    BCM_IF_ERROR_RETURN
           (bcm_esw_filter_remove(unit, fid));
    BCM_IF_ERROR_RETURN
           (bcm_esw_filter_destroy(unit, fid));

    return BCM_E_NONE;
#else /* !BCM_FILTER_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* !BCM_FILTER_SUPPORT */
}

/*
 * Function:
 *      _auth_field_install
 * Description:
 *      Install FP for accepting EAPOL frames
 *      destinated for switch CPU's MAC address
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      mac -  MAC address
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */

STATIC int
_auth_field_install(int unit, int port, bcm_mac_t mac) 
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      _auth_field_install_all
 * Description:
 *      Install FP for accepting EAPOL frames
 *      destinated for switch CPU's MAC addresses
 * Parameters:
 *      unit - Device number
 *      port - Port number
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */

STATIC int
_auth_field_install_all(int unit, int port)
{
    auth_mac_t **list=&auth_cntl[unit][port].macList;
    auth_mac_p entry;
    int rv = BCM_E_NONE;

    if (auth_cntl[unit][port].mac_set) {
        return BCM_E_EXISTS;
    }

    while (*list != NULL) {
        rv = _auth_field_install(unit, port, (*list)->mac);
        if (rv < 0) {
            _auth_maclist_remove(list, (*list)->mac, &entry);
            sal_free(entry);
        } else {
            list = &(*list)->next;
        }
    }
    auth_cntl[unit][port].mac_set = TRUE;

    return rv;
}

/*
 * Function:
 *      _auth_field_remove_all
 * Description:
 *      Removed all installed FP in controlled state
 * Parameters:
 *      unit - Device number
 *      port - Port number
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */

STATIC int
_auth_field_remove_all(int unit, int port)
{
    auth_mac_t **list=&fp_cntl[unit].macList;

    if (!auth_cntl[unit][port].mac_set) {
        return BCM_E_NONE;
    }

    while (*list != NULL) {
        pbmp_t   pbm;
        pbm = (*list)->pbmp;
        BCM_PBMP_PORT_REMOVE(pbm, port); 
        if (BCM_PBMP_IS_NULL(pbm)) {
            BCM_IF_ERROR_RETURN
                (bcm_esw_field_entry_remove(unit, (*list)->entry)); 
            BCM_IF_ERROR_RETURN
                (bcm_esw_field_entry_destroy(unit, (*list)->entry));
            fp_cntl[unit].count--;
            (*list)->entry = -1;
        } else {
            bcm_esw_field_qualify_InPorts(unit, (*list)->entry, pbm, PBMP_E_ALL(unit));
            BCM_IF_ERROR_RETURN
                (bcm_esw_field_entry_reinstall(unit, (*list)->entry));
            (*list)->pbmp = pbm;
        }
        list = &(*list)->next;
    }

    if ((fp_cntl[unit].count==0) && (fp_cntl[unit].inited)) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_entry_remove(unit, fp_cntl[unit].entry1)); 
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_entry_destroy(unit, fp_cntl[unit].entry1));
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_entry_remove(unit, fp_cntl[unit].entry2)); 
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_entry_destroy(unit, fp_cntl[unit].entry2));
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_group_destroy(unit, fp_cntl[unit].group0));
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_group_destroy(unit, fp_cntl[unit].group1));
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_group_destroy(unit, fp_cntl[unit].group2));
        _auth_maclist_destroy(&fp_cntl[unit].macList);
        fp_cntl[unit].inited = FALSE;
    } else {
        auth_mac_p    entry;
        list = &fp_cntl[unit].macList;
        while (*list != NULL) {
            if ((*list)->entry == -1) {   
               entry = *list;
               *list = entry->next;
               sal_free(entry);
            } 
            list = &(*list)->next;
        }
    }

    auth_cntl[unit][port].mac_set = FALSE;

    return BCM_E_NONE;
}

/*
 * Function:
 *      _auth_field_remove
 * Description:
 *      Removed installed FP
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      mac -  Switch's MAC address 
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */

STATIC int
_auth_field_remove(int unit, int port, bcm_mac_t mac)
{
#ifdef BCM_FIELD_SUPPORT
    pbmp_t   pbm;
    bcm_field_qset_t qset0;
    auth_mac_p entry;

    if (_auth_maclist_lookup(&fp_cntl[unit].macList, mac, &entry) > 0) {
        BCM_FIELD_QSET_INIT(qset0);
        BCM_FIELD_QSET_ADD(qset0, bcmFieldQualifyInPorts);
        pbm = entry->pbmp;
        BCM_PBMP_PORT_REMOVE(pbm, port);
        if (BCM_PBMP_IS_NULL(pbm)) {
            BCM_IF_ERROR_RETURN
                (bcm_esw_field_entry_remove(unit, entry->entry)); 
            BCM_IF_ERROR_RETURN
                (bcm_esw_field_entry_destroy(unit, entry->entry));
            fp_cntl[unit].count--;
            _auth_maclist_remove(&fp_cntl[unit].macList, mac, &entry);    
            sal_free(entry);
        } else {
            bcm_esw_field_qualify_InPorts(unit, entry->entry, pbm, PBMP_E_ALL(unit));
            BCM_IF_ERROR_RETURN(
                bcm_esw_field_entry_reinstall(unit, entry->entry));
            entry->pbmp = pbm;
        }
    }

    if ((fp_cntl[unit].count==0) && (fp_cntl[unit].inited)) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_entry_remove(unit, fp_cntl[unit].entry1)); 
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_entry_destroy(unit, fp_cntl[unit].entry1));
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_entry_remove(unit, fp_cntl[unit].entry2)); 
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_entry_destroy(unit, fp_cntl[unit].entry2));
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_group_destroy(unit, fp_cntl[unit].group0));
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_group_destroy(unit, fp_cntl[unit].group1));
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_group_destroy(unit, fp_cntl[unit].group2));
        _auth_maclist_destroy(&fp_cntl[unit].macList);
        fp_cntl[unit].inited = FALSE;
    }

    return BCM_E_NONE;
#else /* !BCM_FIELD_SUPPORT */
    return BCM_E_UNAVAIL;
#endif /* !BCM_FIELD_SUPPORT */
}

#if defined(BCM_FILTER_SUPPORT) || defined(BCM_FIELD_SUPPORT)
/*
 * Function:
 *      _auth_maclist_insert 
 * Description:
 *      Add a MAC address to the list 
 * Parameters:
 *      list - list to be inserted 
 *      mac -  MAC address 
 *      ins -  (OUT) entry inserted  
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */

STATIC int
_auth_maclist_insert(auth_mac_t **list, bcm_mac_t mac, auth_mac_p *ins)
{
    auth_mac_p    entry;

    if (_auth_maclist_lookup(list, mac, &entry) > 0) {
        return BCM_E_EXISTS;
    } 

    if ((entry = sal_alloc(sizeof(auth_mac_t), "maclist")) == NULL) {
        return BCM_E_MEMORY;
    }

    sal_memset(entry, 0, sizeof(auth_mac_t));
    sal_memcpy(entry->mac, mac, sizeof(bcm_mac_t)); 
    entry->next = *list;
    *list = entry;
    *ins = entry;

    return BCM_E_NONE;
}
#endif /* !BCM_FILTER_SUPPORT && !BCM_FIELD_SUPPORT */

/*
 * Function:
 *      _auth_maclist_remove
 * Description:
 *      Remove a MAC address from the list
 * Parameters:
 *      list - list to be removed 
 *      mac -  MAC address
 *      ins -  (OUT) entry deleted 
 * Returns:
 *      BCM_E_NONE       - Success
 *      BCM_E_NOT_FOUND  - MAC not found in list
 * Notes:
 */

STATIC int
_auth_maclist_remove(auth_mac_t **list, bcm_mac_t mac, auth_mac_p *del)
{
    auth_mac_p    entry;

    while (*list != NULL) {
        if (!sal_memcmp((*list)->mac, mac, sizeof(bcm_mac_t))) { 
            entry = *list;
            *list = entry->next;
            *del = entry;
            return BCM_E_NONE;
        }
        list = &(*list)->next;
    }

    return BCM_E_NOT_FOUND;
}

#if defined(BCM_FILTER_SUPPORT) || defined(BCM_FIELD_SUPPORT)
/*
 * Function:
 *      _auth_maclist_lookup
 * Description:
 *      Lookup a MAC address in the list
 * Parameters:
 *      list - list to be lookuped 
 *      mac -  MAC address
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */

STATIC int
_auth_maclist_lookup(auth_mac_t **list, bcm_mac_t mac, auth_mac_p *entry)
{
    while (*list != NULL) {
        if (!sal_memcmp((*list)->mac, mac, sizeof(bcm_mac_t))) { 
            *entry = *list;  
            return TRUE;
        }
        list = &(*list)->next;
    }

    return FALSE;
}
#endif /* !BCM_FILTER_SUPPORT && !BCM_FIELD_SUPPORT */

/*
 * Function:
 *      _auth_maclist_destroy
 * Description:
 *      Destroy all MAC addresses in the list
 * Parameters:
 *      list - list to be destroyed 
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */

STATIC int
_auth_maclist_destroy(auth_mac_t **list)
{
    auth_mac_p entry;

    if (*list == NULL) {
        return BCM_E_EMPTY;
    }

    while (*list != NULL) {
        _auth_maclist_remove(list, (*list)->mac, &entry);
        sal_free(entry);
    }
    return BCM_E_NONE;
}

#ifdef AUTH_DEBUG
/*
 * Function:
 *      _auth_maclist_dump
 * Description:
 *      Dump all MAC addresses in the list
 * Parameters:
 *      list - list to be dumped 
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */

STATIC int
_auth_maclist_dump(auth_mac_t **list)
{
    while (*list != NULL) {
        soc_cm_print("%02x:%02x:%02x:%02x:%02x:%02x\n",
                     (*list)->mac[0], (*list)->mac[1],
                     (*list)->mac[2], (*list)->mac[3],
                     (*list)->mac[4], (*list)->mac[5]);
        list = &(*list)->next;
    }

    return BCM_E_NONE;
}
#endif /* AUTH_DEBUG */
