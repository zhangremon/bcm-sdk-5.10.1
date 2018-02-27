/*
 * $Id: port.c 1.192 Broadcom SDK $
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
 * FE2000 Port API
 */

#include <soc/debug.h>
#include <soc/phy.h>
#include <soc/phyctrl.h>
#include <soc/phyreg.h>
#include <soc/drv.h>
#include <soc/linkctrl.h>


#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_ca_auto.h>
#include <soc/sbx/hal_c2_auto.h>
#include <soc/sbx/hal_ca_c2.h>
#include <soc/sbx/fe2000.h>
#include <soc/sbx/qe2000.h>
#include <soc/sbx/fe2000_spi.h>
#include <soc/sbx/qe2000_mvt.h>
#include <soc/sbx/fe2k/sbFe2000InitUtils.h>
#include <soc/sbx/fe2kxt/sbFe2000XtInitUtils.h>
#include <soc/sbx/fe2k_common/sbFe2000CommonUtil.h>


#include <bcm/error.h>
#include <bcm/debug.h>
#include <bcm/port.h>
#include <bcm/vlan.h>
#include <bcm/link.h>
#include <bcm/stg.h>
#include <bcm/rate.h>
#include <bcm/stack.h>
#include <bcm/pkt.h>

#include <bcm_int/common/link.h>
#include <bcm_int/sbx/error.h>
#include <bcm_int/sbx/port.h>

#include <bcm_int/sbx/fe2000/vlan.h>
#include <bcm_int/sbx/fe2000/allocator.h>
#include <bcm_int/sbx/fe2000/port.h>
#include <bcm_int/sbx/fe2000/mpls.h>
#include <bcm_int/sbx/fe2000/policer.h>
#include <bcm_int/sbx/fe2000/vswitch.h>
#include <bcm_int/sbx/fe2000/g2p3.h>

#ifdef BCM_FE2000_P3_SUPPORT
#include <bcm_int/sbx/fe2000/g2p3.h>
#endif


/*
 * Port Module Initialization flag
 */
static  int                _port_init[BCM_LOCAL_UNITS_MAX];

/*
 * Mutex Lock
 */
static  sal_mutex_t        _port_mlock[BCM_LOCAL_UNITS_MAX];
#define PORT_LOCK(unit)    sal_mutex_take(_port_mlock[unit], sal_mutex_FOREVER)
#define PORT_UNLOCK(unit)  sal_mutex_give(_port_mlock[unit])

/*
 * General Utility Macros
 */

/* Parameter and module initialization checking */
#define UNIT_VALID_CHECK(unit) \
    if (((unit) < 0) || ((unit) >= BCM_LOCAL_UNITS_MAX)) { return BCM_E_UNIT; }

#define UNIT_INIT_DONE(unit)    (_port_init[unit])

#define UNIT_INIT_CHECK(unit) \
    do { \
        UNIT_VALID_CHECK(unit); \
        if (!_port_init[unit]) { return BCM_E_INIT; } \
    } while (0)

#define PORT_INIT_CHECK(unit, port) \
    do { \
        UNIT_INIT_CHECK(unit); \
        if (!((port) >= 0 && (port) < (SBX_MAX_PORTS))) { return BCM_E_PORT; } \
    } while (0)

#define PORT_INTERFACE_MAC_CHECK(unit, port) \
    if (!INTERFACE_MAC(unit, port)) { return BCM_E_PORT; }

/* Return on error, with unlock */
#define BCM_IF_ERROR_UNLOCK_RETURN(unit, op) \
    do {  \
        int _rv; _rv = (op);  \
        if (BCM_FAILURE(_rv)) { PORT_UNLOCK(unit); return (_rv); }  \
    } while (0)

#define REG_OFFSET(unit, reg)    SAND_HAL_FE2000_REG_OFFSET(unit, reg)

/*
 * Port Attributes
 *
 * Some attributes will return 'available' for mac interfacing port,
 * they are either not applicable, or not implemented yet.
 * Mask out those attributes to avoid returning error in bcm_port_info_get().
 */
#define BCM_FE2000_PORT_ATTR_UNVAIL    (BCM_PORT_ATTR_LEARN_MASK      | \
                                        BCM_PORT_ATTR_DISCARD_MASK    | \
                                        BCM_PORT_ATTR_VLANFILTER_MASK | \
                                        BCM_PORT_ATTR_STP_STATE_MASK  | \
                                        BCM_PORT_ATTR_PFM_MASK        | \
                                        BCM_PORT_ATTR_RATE_MCAST_MASK | \
                                        BCM_PORT_ATTR_RATE_BCAST_MASK | \
                                        BCM_PORT_ATTR_RATE_DLFBC_MASK | \
                                        BCM_PORT_ATTR_FAULT_MASK)

#define BCM_FE2000_PORT_ATTR_ALL_MASK  (BCM_PORT_ATTR_ALL_MASK        & \
                                       ~BCM_FE2000_PORT_ATTR_UNVAIL)

/*
 * Port Handler
 *
 * The following structure stores the PORT module internal software
 * information on a device.
 *
 * Access to following structure should protected by PORT_LOCK.
 */
_sbx_port_handler_t                 *_sbx_port_handler[BCM_LOCAL_UNITS_MAX];

#define PORT(_unit, _port)            (_sbx_port_handler[_unit]->port[_port])

/* Get the SPI bus port for given bcm port number */
#define PORT_SPI_BUS(_unit, _port)    (SOC_PORT_BLOCK_NUMBER(_unit, _port))

/* Get the SPI subport port for given bcm port number */
#define PORT_SPI_SUBPORT(_unit, _port) (SOC_PORT_BLOCK_INDEX(_unit, _port))


/* Other defines */

/*
 * MTU Maximum frame size
 * MAC GE and ucode MTU support the same max limit 0x3fff (14 bits)
 * MAC XE (bigmac) max limit is 16360 (see specs).
 */
#define BCM_FE2000_PORT_FRAME_MAX       0x3fff
#define BCM_FE2000_PORT_XE_FRAME_MAX    16360

#define HIGIG_START                     0xfb

typedef struct _bcm_fe2000_gport_dest_s {
    bcm_port_t      port;
    bcm_module_t    modid;
    bcm_trunk_t     tgid;
    int             mpls_id;
    int             mim_id;
    uint32          gport_type;
} _bcm_fe2000_gport_dest_t;

extern int bcm_sbx_port_clear(int unit);


extern int
bcm_fe2000_mim_port_get_lpid(int unit,
                             bcm_gport_t gport,
                             uint32 *lpid,
                             bcm_port_t *pport);

int
_bcm_fe2000_port_egr_remark_idx_get(int unit, bcm_port_t port, uint32_t *idx)
{
    PORT_INIT_CHECK(unit, port);
    *idx = PORT(unit, port).egr_remark_table_idx;
    return BCM_E_NONE;
}

/*
 * Initialize common state for all ports
 */
STATIC int
_bcm_fe2000_port_ilib_common_init(int unit)
{
    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        return _bcm_fe2000_g2p3_port_ilib_common_init(unit);        
#endif /* BCM_FE2000_P3_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        return BCM_E_CONFIG;
    }

}



STATIC int
_bcm_fe2000_port_ilib_entry_init(int unit,
                                 bcm_port_t port,
                                 bcm_vlan_data_t *vd)
{
    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        return _bcm_fe2000_g2p3_port_ilib_entry_init(unit, port, vd);
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        return BCM_E_INTERNAL;
    }
}



STATIC int
_bcm_fe2000_port_ilib_egr_init(int unit, bcm_port_t port)
{
    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        return _bcm_fe2000_g2p3_port_ilib_egr_init(unit, port);
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        return BCM_E_INTERNAL;
    }
}


STATIC int
_bcm_fe2000_port_ilib_tpid_init(int unit, bcm_port_t port)
{
    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        return _bcm_fe2000_g2p3_port_ilib_tpid_init(unit, port);
#endif /* BCM_FE2000_P3_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        return BCM_E_INTERNAL;
    }
}


STATIC int
_bcm_fe2000_port_ilib_fte_init(int unit, int node, bcm_port_t port)
{
    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        return _bcm_fe2000_g2p3_port_ilib_fte_init(unit, node, port);
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        return BCM_E_INTERNAL;
    }
}



int
_bcm_fe2000_port_ilib_lp_init(int unit, bcm_module_t modid, bcm_port_t port)
{
    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        return _bcm_fe2000_g2p3_port_ilib_lp_init(unit, modid, port);
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        return BCM_E_INTERNAL;
    }
}

int
_bcm_fe2000_port_vlan_lp_set(int unit, bcm_module_t modid,
                             bcm_port_t port, bcm_vlan_t vlan)
{

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        return _bcm_fe2000_g2p3_port_vlan_lp_set(unit, modid, port, vlan);
#endif /* BCM_FE2000_P3_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        return BCM_E_INTERNAL;
    }
}

STATIC int
_bcm_fe2000_port_ilib_well_known_egr_ete_init(int unit)
{
    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        return _bcm_fe2000_g2p3_port_ilib_well_known_egr_init(unit);
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        return BCM_E_INTERNAL;
    }
}


/*
 * Function:
 *     _bcm_fe2000_port_sw_init
 * Purpose:
 *     Initialize internal software information in port module.
 * Parameters:
 *     unit - Device number
 * Returns:
 *     BCM_E_NONE   - Success
 *     BCM_E_MEMORY - Failed to allocate required lock or memory
 * NOTE:
 *     Assumes valid unit number.
 *     Assumes lock is held.
 */
STATIC int
_bcm_fe2000_port_sw_init(int unit)
{
    /* Port handler information */
    if (_sbx_port_handler[unit] == NULL) {
        _sbx_port_handler[unit] = sal_alloc(sizeof(_sbx_port_handler_t),
                                        "sbx_port_info");
        if (_sbx_port_handler[unit] == NULL) {
            return BCM_E_MEMORY;
        }

        sal_memset(_sbx_port_handler[unit], 0, sizeof(_sbx_port_handler_t));
    }

    return BCM_E_NONE;
}


STATIC int
_bcm_fe2000_port_qos_init(int unit)
{
    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        return _bcm_fe2000_g2p3_port_qos_init(unit);
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        return BCM_E_INTERNAL;
    }
}


int
_bcm_fe2000_port_modid_set(int unit, bcm_module_t modid)
{
    int                       rv = BCM_E_NONE;
    bcm_pbmp_t                pbmp;
    bcm_port_t                port;

    UNIT_INIT_CHECK(unit);

    BCM_PBMP_ASSIGN(pbmp, PBMP_PORT_ALL(unit));
    BCM_PBMP_OR(pbmp, PBMP_CMIC(unit));
    BCM_PBMP_REMOVE(pbmp, PBMP_HG_ALL(unit));
    PORT_LOCK(unit);


    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = soc_sbx_g2p3_node_set(unit, modid);
        if (BCM_FAILURE(rv)) {
            PORT_UNLOCK(unit);
            return rv;
        }
        break;
#endif /* BCM_FE2000_P3_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        PORT_UNLOCK(unit);
        return BCM_E_INTERNAL;
    }

    BCM_PBMP_ITER(pbmp, port) {
        /* Skip ports not supported in certain environment (like SIM) */
        if (!SOC_PORT_VALID(unit, port) || (port >= SBX_MAX_PORTS)) {
            continue;
        }
        
        switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
        {
            int junk;
            
            rv = soc_sbx_node_port_get(unit, modid, port, &junk, &junk, &junk);
            if (BCM_SUCCESS(rv)) {
                rv = _bcm_fe2000_g2p3_port_ilib_lp_init(unit, modid, port);
            } else {
                PORT_WARN((_SBX_D(unit, "bcm_stk_fabric_map_set must be called before "
                                  "setting modid\n")));
                /* not necessarily an error; the existing/legacy init sequences
                 * may set the module id before the mapping, but it must be 
                 * done again after the mapping is set.  Don't kill the 
                 * sequence for this case.
                 */
                rv = BCM_E_NONE;
            }
            
            if (BCM_FAILURE(rv)) {
                PORT_ERR((_SBX_D(unit, "Port %d failed to init logical port: "
                                 "%s\n"), port));
            }
        }
        break;
#endif /* BCM_FE2000_P3_SUPPORT */

        default:
            SBX_UNKNOWN_UCODE_WARN(unit);
            rv = BCM_E_INTERNAL;
        }   
    }

   PORT_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *     _bcm_fe2000_port_ilib_init
 * Purpose:
 *     Internal routine to initialize ILIB microcode port state.
 * Parameters:
 *     unit - Device number
 *     vd   - Initial VLAN id
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * NOTE:
 *     Assumes valid unit number.
 *     Assumes lock is held.
 */
STATIC int
_bcm_fe2000_port_ilib_init(int unit, bcm_vlan_data_t *vd)
{
    int                       rv = BCM_E_NONE;
    int                       temp_rv;
    bcm_pbmp_t                pbmp;
    bcm_port_t                port;
    int                       node;
    int                       node_max;

    /*
     * Skip init for customer programming ucode
     */
    if (SOC_IS_SBX_G2XX(unit)) {
        return BCM_E_NONE;
    }

    /*
     * Port Module Initialization
     *   non-port specific state init
     */
    rv = _bcm_fe2000_port_ilib_common_init(unit);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    temp_rv = _bcm_fe2000_port_qos_init(unit);
    if (BCM_FAILURE(temp_rv)) {
        rv = temp_rv;
    }

    BCM_PBMP_ASSIGN(pbmp, PBMP_PORT_ALL(unit));
    BCM_PBMP_OR(pbmp, PBMP_CMIC(unit));
    SOC_PBMP_REMOVE(pbmp, PBMP_HG_ALL(unit));
    BCM_PBMP_ITER(pbmp, port) {
        /* Skip ports not supported in certain environment (like SIM) */
        if (!SOC_PORT_VALID(unit,port) || (port >= SBX_MAX_PORTS)) {
            continue;
        }

        /*
         * Notes:  If Get/Set of G2k tables fail, do not return,
         * continue with init
         */
        temp_rv = _bcm_fe2000_port_ilib_entry_init(unit, port, vd);
        if (BCM_FAILURE(temp_rv)) {
            rv = temp_rv;
        }

        /*
         * Setup ETE and OHI per port -
         */
        temp_rv = _bcm_fe2000_port_ilib_egr_init(unit, port);
        if (BCM_FAILURE(temp_rv)) {
            rv = temp_rv;
        }

        /*
         * Setup the TPID stack for provider bridging
         */
        temp_rv = _bcm_fe2000_port_ilib_tpid_init(unit, port);
        if (BCM_FAILURE(temp_rv)) {
            rv = temp_rv;
        }

    }

    /*
     * Initialize FTEs for local and remote ports
     */
    node_max = soc_property_get(unit, spn_NUM_MODULES, SBX_MAX_NODES);
    /* for Polaris chasses - should be a no-op otherwise */
    node_max = (node_max > 32) ? 32 : node_max;

    /*  use num_modules config setting,  mesh of unicast queues */
    for(node = 0; node < node_max; node++) {
        for (port=0; port < SOC_SBX_CFG(unit)->nMaxFabricPortsOnModule; port++) {


            temp_rv = _bcm_fe2000_port_ilib_fte_init(unit, node, port);
            if (BCM_FAILURE(temp_rv)) {
                rv = temp_rv;
            }
        }
    }

    /*
     * Allocate a raw ETE, and populate well known OHI
     */
    temp_rv = _bcm_fe2000_port_ilib_well_known_egr_ete_init(unit);
    if (BCM_FAILURE(temp_rv)) {
        rv = temp_rv;
    }

    return rv;
}

STATIC int
_bcm_fe2000_port_ilib_detach(int unit)
{
    _bcm_fe2000_g2p3_port_ilib_common_detach(unit);

    _bcm_fe2000_g2p3_port_qos_detach(unit);

    return BCM_E_NONE; 
}

/*
 * Function:
 *     _bcm_fe2000_port_mode_setup
 * Purpose:
 *     Set initial operating mode for a port.
 * Parameters:
 *     unit   - Device number
 *     port   - Device port number
 *     enable - Whether to enable or disable
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
STATIC int
_bcm_fe2000_port_mode_setup(int unit, bcm_port_t port, int enable)
{
    bcm_port_abil_t  pm;
    soc_port_if_t    pif;

    SOC_IF_ERROR_RETURN(bcm_port_ability_get(unit, port, &pm));

    /* If MII supported, enable it, otherwise use TBI */
    if (pm & (SOC_PM_MII | SOC_PM_GMII | SOC_PM_XGMII)) {
        if (IS_GE_PORT(unit, port)) {
            pif = SOC_PORT_IF_GMII;
        } else if (IS_HG_PORT(unit, port) || IS_XE_PORT(unit, port)) {
            pif = SOC_PORT_IF_XGMII;
        } else {
            pif = SOC_PORT_IF_MII;
        }
    } else {
        pif = SOC_PORT_IF_TBI;
    }

    /* PHY/MAC interface */
    if (INTERFACE_MAC(unit, port)) {
        SOC_IF_ERROR_RETURN
            (soc_phyctrl_interface_set(unit, port, pif));
        SOC_IF_ERROR_RETURN
            (MAC_INTERFACE_SET(PORT(unit, port).p_mac, unit, port, pif));
        SOC_IF_ERROR_RETURN
            (MAC_ENABLE_SET(PORT(unit, port).p_mac, unit, port, enable));
    }

    return BCM_E_NONE;
}


/*
 * Function:
 *     _bcm_port_settings_init
 * Purpose:
 *     Initialize port settings if they are to be different from the
 *     default ones
 * Parameters:
 *     unit   - Device number
 *     port   - Device port number
 * Returns:
 *     BCM_E_NONE    - Success (or already initialized)
 *     BCM_E_INTERNAL- Failure
 * Notes:
 *      This function initializes port settings based on the folowing config
 *      variables:
 *           port_init_speed
 *           port_init_duplex
 *           port_init_adv
 *           port_init_autoneg
 *      If a variable is not set, then no additional initialization of the
 *      corresponding parameter is done (and the defaults will normally be
 *      advertize everything you can do and use autonegotiation).
 *
 *      A typical use would be to set:
 *          port_init_adv=0
 *          port_init_autoneg=1
 *      to force link down in the beginning.
 *
 *      Another setup that makes sense is something like:
 *          port_init_speed=10
 *          port_init_duplex=0
 *          port_init_autoneg=0
 *      in order to force link into a certain mode. (It is very important to
 *      disable autonegotiation in this case).
 *
 *      PLEASE NOTE:
 *          The standard rc.soc forces autoneg=on on all the ethernet ports
 *          Thus, to use the second example one has to edit rc.soc as well.
 */
STATIC int
_bcm_fe2000_port_settings_init(int unit, bcm_port_t port)
{
    int             val;
    bcm_port_info_t info;

    /* If not an Ethernet port, just return */
    if (!INTERFACE_MAC(unit, port)) {
        return BCM_E_NONE;
    }

    bcm_port_info_t_init(&info);

    val = soc_property_port_get(unit, port, spn_PORT_INIT_SPEED, -1);
    if (val != -1) {
        info.speed = val;
        info.action_mask |= BCM_PORT_ATTR_SPEED_MASK;
    }

    val = soc_property_port_get(unit, port, spn_PORT_INIT_DUPLEX, -1);
    if (val != -1) {
        info.duplex = val;
        info.action_mask |= BCM_PORT_ATTR_DUPLEX_MASK;
    }

    val = soc_property_port_get(unit, port, spn_PORT_INIT_ADV, -1);
    if (val != -1) {
        info.local_advert = val;
        info.action_mask |= BCM_PORT_ATTR_LOCAL_ADVERT_MASK;
    }

    val = soc_property_port_get(unit, port, spn_PORT_INIT_AUTONEG, -1);
    if (val != -1) {
        info.autoneg = val;
        info.action_mask |= BCM_PORT_ATTR_AUTONEG_MASK;
    }

    return bcm_port_selective_set(unit, port, &info);
}


/*
 * Function:
 *     _bcm_fe2000_port_probe_port
 * Purpose:
 *     Probe the phy and set up the phy and mac of the indicated port.
 * Parameters:
 *     unit - Device number
 *     port - Port to probe
 *     okay - (OUT) True indicates port can be enabled
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Assumes lock is held.
 *     If error is returned, the port should not be enabled.
 */
STATIC int
_bcm_fe2000_port_probe_port(int unit, bcm_port_t port, int *okay)
{
    int                 rv;
    mac_driver_t        *macd;

    *okay = FALSE;

    PORT_VERB(("Init unit=%d port=%d PHY...\n", unit, port));
    
    if (SOC_FAILURE(rv = soc_phyctrl_probe(unit, port))) {
        PORT_WARN(("Unit %d Port %s: Failed to probe PHY: %s\n",
                   unit, SOC_PORT_NAME(unit, port), soc_errmsg(rv)));
        return rv;
    }

    if (SOC_WARM_BOOT(unit) == FALSE) {
        if (SOC_FAILURE(rv = soc_phyctrl_init(unit, port))) {
            PORT_WARN(("Unit %d Port %s: Failed to initialize PHY: %s\n",
                       unit, SOC_PORT_NAME(unit, port), soc_errmsg(rv)));
            return rv;
        }
    }

    /*
     * Currently initializing MAC after PHY is required because of
     * phy_5690_notify_init().
     */

    PORT_VERB(("Init unit=%d port=%d MAC...\n", unit, port));

    if (SOC_FAILURE(rv = soc_sbx_fe2000_mac_probe(unit, port, &macd))) {
        PORT_WARN(("Unit %d Port %s: Failed to probe MAC: %s\n",
                   unit, SOC_PORT_NAME(unit, port), soc_errmsg(rv)));
        return rv;
    }

    PORT(unit, port).p_mac = macd;

    if (SOC_WARM_BOOT(unit) == FALSE) {
        if (SOC_FAILURE(rv = MAC_INIT(PORT(unit, port).p_mac, unit, port))) {
            PORT_WARN(("Unit %d Port %s: Failed to initialize MAC: %s\n",
                       unit, SOC_PORT_NAME(unit, port), soc_errmsg(rv)));
            return rv;
        }
        
    }

    *okay = TRUE;

    return BCM_E_NONE;
}


/*
 * Function:
 *     _bcm_fe2000_port_probe
 * Purpose:
 *     Main routine for bcm_fe2000_port_probe.
 *     Does not check for initialized port module.
 * Parameters:
 *     unit      - Device number
 *     pbmp      - Bitmap of ports to probe
 *     okay_pbmp - (OUT) Ports which were successfully probed
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Assumes valid parameters.
 */
STATIC int
_bcm_fe2000_port_probe(int unit, pbmp_t pbmp, pbmp_t *okay_pbmp)
{
    int         rv = BCM_E_NONE;
    bcm_port_t  port;
    int         okay;

    /*
     * Run probe only on ports with PHY/MAC interface.
     * For other valid ports, just add to returned 'okay' bitmap.
     */
    PBMP_ITER(pbmp, port) {

        if (INTERFACE_MAC(unit, port)) {
            rv = _bcm_fe2000_port_probe_port(unit, port, &okay);
        } else if (SOC_PORT_VALID(unit, port)) {
            okay = TRUE;
        } else {
            okay = FALSE;
            rv = BCM_E_PORT;
        }

        if (okay) {
            SOC_PBMP_PORT_ADD(*okay_pbmp, port);
        }
        if (BCM_FAILURE(rv)) {
            PORT_WARN(("bcm_port_probe: Port probe failed on port %s\n",
                       SOC_PORT_NAME(unit, port)));
            break;
        }
    }

    return rv;
}


/*
 * Function:
 *     _bcm_fe2000_port_enable_get
 * Purpose:
 *     Get the enable state as defined by bcm_port_enable_set().
 *     Main routine for bcm_fe2000_port_enable_get().
 *     Does not check for initialized port module.
 * Parameters:
 *     unit   - Device number
 *     port   - Device port number
 *     enable - (OUT) TRUE if port is enabled, FALSE if port is disabled
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Assume valid parameters.
 *     The PHY enable holds the port enable state set by the user.
 *     The MAC enable transitions up and down automatically via linkscan
 *     even if user port enable is always up.
 */
STATIC int
_bcm_fe2000_port_enable_get(int unit, bcm_port_t port, int *enable)
{
    int  rv = BCM_E_PORT;

    if (INTERFACE_MAC(unit, port)) {
        rv = soc_phyctrl_enable_get(unit, port, enable);

    } else if (INTERFACE_SPI(unit, port)) {

        /* SPI bus port */
        if (IS_SPI_BUS_PORT(unit, port)) {
            uint32_t  data;

            /* Read RX enable field (TX and RX should always match) */
            data = SAND_HAL_FE2000_READ_STRIDE(SOC_SBX_SBHANDLE(unit), SR,
                                        PORT_SPI_BUS(unit, port), SR_CONFIG0);
            *enable = SAND_HAL_FE2000_GET_FIELD(SOC_SBX_SBHANDLE(unit), SR_CONFIG0, RX_ENABLE, data);
            rv = BCM_E_NONE;

        } else {  /* SPI subport */
            int  spi_subport;

            spi_subport = PORT_SPI_SUBPORT(unit, port);

            if ((spi_subport >= 0) &&
                (spi_subport < SB_FE2000_MAX_PORTS_PER_SPI)) {
                uint32_t  data;

                /* SPI subport is 0..31 */
                if (spi_subport < 32) {
                    data = SAND_HAL_FE2000_READ_STRIDE(SOC_SBX_SBHANDLE(unit), SR,
                                                PORT_SPI_BUS(unit, port),
                                                SR_CONFIG2);
                } else {  /* SPI subport is 32..63 */
                    data = SAND_HAL_FE2000_READ_STRIDE(SOC_SBX_SBHANDLE(unit), SR,
                                                PORT_SPI_BUS(unit, port),
                                                SR_CONFIG3);
                }

                /*
                 * Both registers, sr_config2 and sr_config3, have the same
                 * bit field definition
                 */
                *enable =
                    ((SAND_HAL_FE2000_GET_FIELD(SOC_SBX_SBHANDLE(unit), SR_CONFIG2, RX_PORT_ENABLE, data))
                     & (0x1 << (spi_subport % 32))) ? 1 : 0;

                rv = BCM_E_NONE;
            }
        }

    } else {
        
    }

    return rv;
}


/*
 * Function:
 *     _bcm_fe2000_port_enable_set
 * Purpose:
 *     Physically enable/disable the MAC/PHY on this port.
 *     Main routine for bcm_fe2000_port_enable_set().
 *     Does not check for initialized port module.
 * Parameters:
 *     unit    - Device number
 *     port    - Device port number
 *     enable  - TRUE to enable port, FALSE to disable port
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Assume valid parameters.
 *     If linkscan is running, it also controls the MAC enable state.
 */
STATIC int
_bcm_fe2000_port_enable_set(int unit, bcm_port_t port, int enable)
{
    int  rv = BCM_E_PORT;

    if (INTERFACE_MAC(unit, port)) {
        if (enable) {
            rv = MAC_ENABLE_SET(PORT(unit, port).p_mac, unit, port, TRUE);
            if (SOC_SUCCESS(rv)) {
                rv = soc_phyctrl_enable_set(unit, port, TRUE);
            }
        } else {
            rv = soc_phyctrl_enable_set(unit, port, FALSE);
            if (SOC_SUCCESS(rv)) {
                rv = MAC_ENABLE_SET(PORT(unit, port).p_mac, unit, port, FALSE);
            }
        }

    } else if (INTERFACE_SPI(unit, port)) {
        int  cur_enable;

        rv = _bcm_fe2000_port_enable_get(unit, port, &cur_enable);

        if (BCM_SUCCESS(rv)) {

            if ((cur_enable && !enable) || (!cur_enable && enable)) {
                int  spi_bus;

                spi_bus = PORT_SPI_BUS(unit, port);

                /* SPI bus port */
                if (IS_SPI_BUS_PORT(unit, port)) {
                    uint32  data = (enable) ? 0x1 : 0x0;

                    /* Enable/disable TX and RX */
                    SAND_HAL_FE2000_RMW_FIELD_STRIDE(SOC_SBX_SBHANDLE(unit), ST,
                                              spi_bus, ST_CONFIG0, TX_ENABLE,
                                              data);

                    SAND_HAL_FE2000_RMW_FIELD_STRIDE(SOC_SBX_SBHANDLE(unit), SR,
                                              spi_bus, SR_CONFIG0, RX_ENABLE,
                                              data);
                    rv = BCM_E_NONE;

                } else {  /* SPI subport */
                    int     spi_subport;

                    spi_subport = PORT_SPI_SUBPORT(unit, port);

                    if ((spi_subport >= 0) &&
                        (spi_subport < SB_FE2000_MAX_PORTS_PER_SPI)) {
                        uint32  data;

                        /* SPI subport is 0..31 */
                        if (spi_subport < 32) {
                            data = SAND_HAL_FE2000_READ_STRIDE(SOC_SBX_SBHANDLE(unit),
                                                        SR, spi_bus,
                                                        SR_CONFIG2);
                        } else {  /* SPI subport is 32..63 */
                            data = SAND_HAL_FE2000_READ_STRIDE(SOC_SBX_SBHANDLE(unit),
                                                        SR, spi_bus,
                                                        SR_CONFIG3);
                        }

                        /*
                         * Both registers, sr_config2 and sr_config3, have
                         * the same bit field definition
                         */
                        data = SAND_HAL_FE2000_GET_FIELD(SOC_SBX_SBHANDLE(unit), SR_CONFIG2,
                                                  RX_PORT_ENABLE, data);
                        if (enable) {
                            data |= 0x1 << (spi_subport % 32);
                        } else {
                            data &= ~(0x1 << (spi_subport % 32));
                        }

                        if (spi_subport < 32) {
                            SAND_HAL_FE2000_RMW_FIELD_STRIDE(SOC_SBX_SBHANDLE(unit),
                                                      SR, spi_bus,
                                                      SR_CONFIG2,
                                                      RX_PORT_ENABLE, data);
                        } else {  /* SPI subport is 32..63 */
                            SAND_HAL_FE2000_RMW_FIELD_STRIDE(SOC_SBX_SBHANDLE(unit),
                                                      SR, spi_bus,
                                                      SR_CONFIG3,
                                                      RX_PORT_ENABLE, data);
                        }

                        rv = BCM_E_NONE;
                    }
                }
            }
        }

    } else {
        
    }

    return rv;
}


/*
 * Function:
 *     _bcm_fe2000_port_interface_init
 * Purpose:
 *     Internal routine to initialize port interface.
 * Parameters:
 *     unit   - Device number
 *     enable - Indicates if ports interface is enabled/disabled by default
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * NOTE:
 *     Assumes valid unit number.
 */
STATIC int
_bcm_fe2000_port_interface_init(int unit, int enable)
{
    int         rv = BCM_E_NONE;
    bcm_port_t  port;
    pbmp_t      probe_ports;
    pbmp_t      okay_ports;
#ifdef BROADCOM_DEBUG
    char        pfmtok[SOC_PBMP_FMT_LEN],
                pfmtall[SOC_PBMP_FMT_LEN];
#endif /* BROADCOM_DEBUG */

    BCM_PBMP_CLEAR(okay_ports);

    /*
     * Probe for PHY/MAC interfaces
     */
    probe_ports = PBMP_PORT_ALL(unit);
    BCM_PBMP_REMOVE(probe_ports, PBMP_HG_SUBPORT_ALL(unit));
    rv = _bcm_fe2000_port_probe(unit, probe_ports, &okay_ports);
    if (BCM_FAILURE(rv)) {
        PORT_ERR(("bcm_port_init: Failed port probe unit=%d rv=%d(%s)\n",
                  unit, rv, bcm_errmsg(rv)));
        return rv;
    }

    /* Just probe the port to setup the port function pointers during a 
     * warm boot, no need to re-set the port phy properties.
     */
#if defined(BCM_WARM_BOOT_SUPPORT)
    if (SOC_WARM_BOOT(unit)) {
        return BCM_E_NONE;
    }
#endif /* BCM_WARM_BOOT_SUPPORT */

    PORT_VERB(("bcm_port_init: Probed ports okay: %s of %s\n",
               SOC_PBMP_FMT(okay_ports, pfmtok),
               SOC_PBMP_FMT(probe_ports, pfmtall)));

    PBMP_ITER(okay_ports, port) {
        PORT_VERB(("bcm_port_init: unit=%d port=%s\n",
                   unit, SOC_PORT_NAME(unit, port)));

        if ((rv = _bcm_fe2000_port_mode_setup(unit, port, TRUE)) < 0) {
            PORT_WARN(("Warning: Failed to set initial mode \n"
                       "unit=%d port=%s rv=%d(%s)\n",
                       unit, SOC_PORT_NAME(unit, port), rv, bcm_errmsg(rv)));
        }

        if (BCM_FAILURE(rv = _bcm_fe2000_port_settings_init(unit, port))) {
            PORT_WARN(("Warning: Failed to configure initial settings \n"
                       "unit=%d port=%s rv=%d(%s)\n",
                       unit, SOC_PORT_NAME(unit, port), rv, bcm_errmsg(rv)));
        }

        if (BCM_FAILURE(rv = _bcm_fe2000_port_enable_set(unit, port, enable))) {
             PORT_WARN(("Warning: Failed to %s unit=%d port=%s rv=%d(%s)\n",
                        (enable) ? "enable" : "disable",
                        unit, SOC_PORT_NAME(unit, port), rv, bcm_errmsg(rv)));
        }
    }

    return BCM_E_NONE;
}


int
_bcm_fe2000_port_deinit(int unit)
{

    BCM_IF_ERROR_RETURN(_bcm_fe2000_port_ilib_detach(unit));
    BCM_IF_ERROR_RETURN(bcm_sbx_port_clear(unit));

    if (_sbx_port_handler[unit] != NULL) {
        sal_free(_sbx_port_handler[unit]);
        _sbx_port_handler[unit] = NULL;
    }

    sal_mutex_destroy(_port_mlock[unit]);

    _port_mlock[unit] = NULL;
    _port_init[unit] = FALSE;

    return BCM_E_NONE;
}

/*
 * Function:
 *     bcm_port_init
 * Purpose:
 *     Initialize the PORT interface layer for the specified SOC device.
 * Parameters:
 *     unit - Device number
 * Returns:
 *     BCM_E_NONE    - Success (or already initialized)
 *     BCM_E_INTERNAL- Failed to write PTABLE entries
 *     BCM_E_MEMORY  - Failed to allocate required memory
 *     BCM_E_XXX     - Failure, other errors
 * Notes:
 *     By default ports come up enabled. They can be made to come up disabled
 *     at startup by the compile-time application policy flag
 *     BCM_PORT_DEFAULT_DISABLE in your Make.local
 */
int
bcm_fe2000_port_init(int unit)
{
    int              rv;
    bcm_vlan_data_t  vd;
    int              enable;

    PORT_VERB(("bcm_port_init: unit=%d\n", unit));

    UNIT_VALID_CHECK(unit);

    if (_port_init[unit]) {
        rv = _bcm_fe2000_port_deinit(unit);
        BCM_IF_ERROR_RETURN(rv);
    }

    _port_init[unit] = FALSE;

    BCM_IF_ERROR_RETURN(bcm_sbx_port_init(unit));

    /* Create Mutex lock */
    if (_port_mlock[unit] == NULL) {
        if ((_port_mlock[unit] = sal_mutex_create("bcm_port_lock")) == NULL) {
            return BCM_E_MEMORY;
        }
    }

    PORT_LOCK(unit);

    /* Init SW structures */
    rv = _bcm_fe2000_port_sw_init(unit);

    /* Init common PHY driver */
    if (BCM_SUCCESS(rv)) {
        rv = soc_phy_common_init(unit);
    }

    /* Init microcode */
    if (BCM_SUCCESS(rv)) {
        vd.vlan_tag = BCM_VLAN_DEFAULT;
        BCM_PBMP_ASSIGN(vd.port_bitmap, PBMP_ALL(unit));
        BCM_PBMP_ASSIGN(vd.ut_port_bitmap, PBMP_ALL(unit));
        BCM_PBMP_REMOVE(vd.ut_port_bitmap, PBMP_CMIC(unit));
        rv = _bcm_fe2000_port_ilib_init(unit, &vd);
    }

    /* Init port interfaces */
    /*
     * A compile-time application policy may prefer to disable ports
     * when switch boots up
     */
#ifdef BCM_PORT_DEFAULT_DISABLE
    enable = FALSE;
#else
    enable = TRUE;
#endif  /* BCM_PORT_DEFAULT_DISABLE */
    if (BCM_SUCCESS(rv)) {
        rv = _bcm_fe2000_port_interface_init(unit, enable);
    }

    if (BCM_SUCCESS(rv)) {
        _port_init[unit] = TRUE;
    }

    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_init: unit=%d rv=%d(%s)\n",
               unit, rv, bcm_errmsg(rv)));

    return rv;
}


/*
 * Function:
 *     bcm_fe2000_port_probe
 * Purpose:
 *     Probe the PHY and set up the PHY and MAC for the specified ports.
 *     This is purely a discovery routine and does no configuration.
 * Parameters:
 *     unit      - Device number
 *     pbmp      - Bitmap of ports to probe
 *     okay_pbmp - (OUT) Ports which were successfully probed
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     If error is returned, the port should not be enabled.
 *     Assumes port_init done.
 *     Note that if a PHY is not present, the port will still probe
 *     successfully.  The default driver will be installed.
 */
int
bcm_fe2000_port_probe(int unit, pbmp_t pbmp, pbmp_t *okay_pbmp)
{
    int   rv = BCM_E_NONE;
#ifdef BROADCOM_DEBUG
    char  pfmtp[SOC_PBMP_FMT_LEN],
          pfmtok[SOC_PBMP_FMT_LEN];
#endif /* BROADCOM_DEBUG */

    /* Check params */
    UNIT_INIT_CHECK(unit);
    PARAM_NULL_CHECK(okay_pbmp);
    SOC_PBMP_CLEAR(*okay_pbmp);

    PORT_LOCK(unit);

    rv = _bcm_fe2000_port_probe(unit, pbmp, okay_pbmp);

    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_probe: unit=%d pbmp=%s okay_pbmp=%s rv=%d\n",
               unit, SOC_PBMP_FMT(pbmp, pfmtp),
               SOC_PBMP_FMT(*okay_pbmp, pfmtok), rv));

    return rv;
}


/*
 * Function:
 *     bcm_port_clear
 * Purpose:
 *     Initialize the PORT interface layer for the specified SOC device
 *     without resetting stacking ports.
 * Parameters:
 *     unit - Device number
 * Returns:
 *     BCM_E_NONE - Success (or already initialized)
 *     BCM_E_XXX  - Failure
 * Notes:
 *     By default ports come up enabled. They can be made to come up disabled
 *     at startup by a compile-time application policy flag in your Make.local
 */
int
bcm_fe2000_port_clear(int unit)
{
    bcm_port_config_t  port_config;
    bcm_pbmp_t         reset_ports;
    bcm_port_t         port;
    int                rv, port_enable;

    /* Check params */
    UNIT_INIT_CHECK(unit);

    BCM_IF_ERROR_RETURN(bcm_port_config_get(unit, &port_config));

    PORT_LOCK(unit);

    /* Clear all non-stacking ethernet ports */
    BCM_PBMP_ASSIGN(reset_ports, port_config.e);

    PBMP_ITER(reset_ports, port) {
        PORT_VERB(("bcm_port_clear: unit=%d port=%s\n",
                   unit, SOC_PORT_NAME(unit, port)));

        if (BCM_FAILURE(rv = _bcm_fe2000_port_mode_setup(unit, port, TRUE))) {
            PORT_WARN(("Warning: unit=%d port=%s: "
                       "Failed to set initial mode: %s\n",
                       unit, SOC_PORT_NAME(unit, port), bcm_errmsg(rv)));
        }


        /*
         * A compile-time application policy may prefer to disable
         * ports at startup. The same behavior should be observed
         * when bcm_port_clear gets called.
         */

#ifdef BCM_PORT_DEFAULT_DISABLE
        port_enable = FALSE;
#else
        port_enable = TRUE;
#endif  /* BCM_PORT_DEFAULT_DISABLE */

        if (BCM_FAILURE(rv = bcm_port_enable_set(unit, port, port_enable))) {
            PORT_WARN(("Warning: unit=%d port=%s: "
                       "Failed to %s port: %s\n",
                       unit, SOC_PORT_NAME(unit, port),
                       (port_enable) ? "enable" : "disable", bcm_errmsg(rv)));
        }
    }

    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_clear: unit=%d\n", unit));

    return BCM_E_NONE;
}


/*
 * Function:
 *     _bcm_fe2000_port_detach
 * Purpose:
 *     Main part of bcm_port_detach.
 * Notes:
 *     Assumes lock is held.
 */
STATIC int
_bcm_fe2000_port_detach(int unit, pbmp_t pbmp, pbmp_t *detached)
{
    bcm_port_t          port;

    SOC_PBMP_CLEAR(*detached);

    PBMP_ITER(pbmp, port) {
        if (INTERFACE_MAC(unit, port)) {
            BCM_IF_ERROR_RETURN(soc_phyctrl_detach(unit, port));
        }
        BCM_IF_ERROR_RETURN(bcm_port_stp_set(unit, port, BCM_STG_STP_DISABLE));
        BCM_IF_ERROR_RETURN(_bcm_fe2000_port_mode_setup(unit, port, FALSE));
        SOC_PBMP_PORT_ADD(*detached, port);
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     bcm_port_detach
 * Purpose:
 *     Detach a port.  Set phy driver to no connection.
 * Parameters:
 *     unit     - Device number
 *     pbmp     - Bitmap of ports to detach.
 *     detached - (OUT) Bitmap of ports successfully detached.
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     If a port to be detached does not appear in detached, its
 *     state is not defined.
 */
int
bcm_fe2000_port_detach(int unit, pbmp_t pbmp, pbmp_t *detached)
{
    int   rv;
#ifdef BROADCOM_DEBUG
    char  pfmtp[SOC_PBMP_FMT_LEN],
          pfmtd[SOC_PBMP_FMT_LEN];
#endif /* BROADCOM_DEBUG */

    /* Check params */
    UNIT_INIT_CHECK(unit);
    PARAM_NULL_CHECK(detached);
    SOC_PBMP_CLEAR(*detached);

    PORT_LOCK(unit);
    rv = _bcm_fe2000_port_detach(unit, pbmp, detached);
    PORT_UNLOCK(unit);

    rv = _bcm_fe2000_port_deinit(unit);

    PORT_VERB(("bcm_port_detach: unit=%d pbmp=%s det=%s rv=%d\n",
               unit, SOC_PBMP_FMT(pbmp, pfmtp),
               SOC_PBMP_FMT(*detached, pfmtd), rv));

    return rv;
}

/*
 * Function:
 *     bcm_port_linkscan_get
 * Purpose:
 *     Get the link scan state of the port.
 * Parameters:
 *     unit     - Device number
 *     port     - Device port number
 *     linkscan - (OUT) Linkscan value (None, S/W, H/W)
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_port_linkscan_get(int unit, bcm_port_t port, int *linkscan)
{
    return bcm_linkscan_mode_get(unit, port, linkscan);
}


/*
 * Function:
 *     bcm_port_linkscan_set
 * Purpose:
 *     Set the link scan state of the port.
 * Parameters:
 *     unit     - Device number
 *     port     - Device port number
 *     linkscan - Linkscan value (None, S/W, H/W)
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Currently, only SW linkscan is supported.
 */
int
bcm_fe2000_port_linkscan_set(int unit, bcm_port_t port, int linkscan)
{
    if (linkscan != BCM_LINKSCAN_MODE_SW &&
        linkscan != BCM_LINKSCAN_MODE_NONE)
    {
        return BCM_E_CONFIG;
    }

    return bcm_linkscan_mode_set(unit, port, linkscan);
}


/*
 * Function:
 *      _bcm_fe2000_port_ability_get
 * Purpose:
 *      Main part of bcm_port_ability_get.
 * Notes:
 *      Assumes valid port.
 *      Relies on the fact the soc_port_mode_t and bcm_port_abil_t have
 *      the same values.
 */
STATIC int
_bcm_fe2000_port_ability_get(int unit, bcm_port_t port,
                             bcm_port_abil_t *ability_mask)
{
    soc_port_mode_t  mac_ability, phy_ability;

    if (INTERFACE_MAC(unit, port)) {
        SOC_IF_ERROR_RETURN(soc_phyctrl_ability_get(unit, port, &phy_ability));

        SOC_IF_ERROR_RETURN(MAC_ABILITY_GET(PORT(unit, port).p_mac,
                                            unit, port, &mac_ability));
        *ability_mask  = mac_ability & phy_ability;
        *ability_mask |= mac_ability & SOC_PM_ABILITY_MAC;
        *ability_mask |= phy_ability & SOC_PM_ABILITY_PHY;

    } else if (INTERFACE_SPI(unit, port)) {
        
        *ability_mask = SOC_PM_16GB_FD;

    } else {
        
    }

    return BCM_E_NONE;
}


/*
 * Function:
 *     bcm_port_ability_get
 * Purpose:
 *     Retrieve the local port abilities.
 * Parameters:
 *     unit         - Device number
 *     port         - Device port number
 *     ability_mask - (OUT) Mask of BCM_PORT_ABIL_ values indicating the
 *                    ability of the MAC/PHY
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_port_ability_get(int unit, bcm_port_t port,
                            bcm_port_abil_t *ability_mask)
{
    int  rv = BCM_E_PORT;

    /* Check params */
    PORT_INIT_CHECK(unit, port);
    PARAM_NULL_CHECK(ability_mask);

    PORT_LOCK(unit);
    rv = _bcm_fe2000_port_ability_get(unit, port, ability_mask);
    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_ability_get: unit=%d port=%d abil=0x%x rv=%d\n",
               unit, port, *ability_mask, rv));

    return rv;
}

/*
 * Function:
 *      bcm_port_full_ability_get
 * Purpose:
 *      Retrieve the local port abilities.
 * Parameters:
 *      unit - StrataSwitch Unit #.
 *      port - StrataSwitch port #.
 *      ability_mask - (OUT) Mask of BCM_PORT_ABIL_ values indicating the
 *              ability of the MAC/PHY.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 */
int
bcm_fe2000_port_full_ability_get(int unit,
                                 bcm_port_t port,
                                 bcm_port_ability_t *ability_mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *     bcm_port_autoneg_get
 * Purpose:
 *     Get the autonegotiation state of the port.
 * Parameters:
 *     unit    - Device number
 *     port    - Device port number
 *     autoneg - (OUT) Boolean value
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_port_autoneg_get(int unit, bcm_port_t port, int *autoneg)
{
    int  rv = BCM_E_NONE;
    int  busy = -1;

    /* Check params */
    PORT_INIT_CHECK(unit, port);
    PARAM_NULL_CHECK(autoneg);

    PORT_LOCK(unit);
    if (INTERFACE_MAC(unit, port)) {
        rv = soc_phyctrl_auto_negotiate_get(unit, port, autoneg, &busy);

    } else {
        /* AutoNeg is always 'false' for any non-mac interface port */
        *autoneg = FALSE;
    }

    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_autoneg_get: unit=%d port=%d autoneg=%d "
               "busy=%d rv=%d\n",
               unit, port, *autoneg, busy, rv));

    return rv;
}


/*
 * Function:
 *     bcm_port_autoneg_set
 * Purpose:
 *     Set the autonegotiation state for a given port.
 * Parameters:
 *     unit    - Device number
 *     port    - Device port number
 *     autoneg - Boolean value
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_port_autoneg_set(int unit, bcm_port_t port, int autoneg)
{
    int  rv = BCM_E_PORT;

    /* Check params */
    PORT_INIT_CHECK(unit, port);

    PORT_LOCK(unit);
    if (INTERFACE_MAC(unit, port)) {
        rv = soc_phyctrl_auto_negotiate_set(unit, port, autoneg);

    } else {
        /* Autoneg can only be FALSE for any non-mac interface port */
        if (!autoneg) {
            rv = BCM_E_NONE;
        }
    }
    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_autoneg_set: unit=%d port=%d autoneg=%d rv=%d\n",
               unit, port, autoneg, rv));

    return rv;
}


/*
 * Function:
 *     bcm_port_advert_get
 * Purpose:
 *     Retrieve the local port advertisement for autonegotiation.
 * Parameters:
 *     unit         - Device number
 *     port         - Device port number
 *     ability_mask - (OUT) Local advertisement
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_port_advert_get(int unit, bcm_port_t port,
                           bcm_port_abil_t *ability_mask)
{
    int  rv = BCM_E_PORT;

    /* Check params */
    PORT_INIT_CHECK(unit, port);
    PARAM_NULL_CHECK(ability_mask);
    PORT_INTERFACE_MAC_CHECK(unit, port);

    PORT_LOCK(unit);
    rv = soc_phyctrl_adv_local_get(unit, port, ability_mask);
    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_advert_get: unit=%d port=%d abil=0x%x rv=%d\n",
               unit, port, *ability_mask, rv));

    return rv;
}

int
bcm_fe2000_port_autoneg_advert_get(int unit,
                                   bcm_port_t port,
                                   bcm_port_ability_t *ability_mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_autoneg_advert_set(int unit,
                                   bcm_port_t port,
                                   bcm_port_ability_t *ability_mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_ability_advert_get(int unit,
                                   bcm_port_t port,
                                   bcm_port_ability_t *ability_mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_ability_advert_set(int unit,
                                   bcm_port_t port,
                                   bcm_port_ability_t *ability_mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_ability_remote_get(int unit,
                                   bcm_port_t port,
                                   bcm_port_ability_t *ability_mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_ability_local_get(int unit,
                                  bcm_port_t port,
                                  bcm_port_ability_t *ability_mask)
{
    return BCM_E_UNAVAIL;
}


int 
bcm_fe2000_port_local_get(int unit,  
                          bcm_gport_t gport,
                          bcm_port_t *local_port) 
{
    if (BCM_GPORT_IS_MODPORT(gport)) {
        *local_port = BCM_GPORT_MODPORT_PORT_GET(gport);
        return BCM_E_NONE;
    }

    return BCM_E_UNAVAIL;
}


/*
 * Function:
 *     _bcm_fe2000_port_advert_remote_get
 * Purpose:
 *      Main part of bcm_port_advert_get_remote.
 */
STATIC int
_bcm_fe2000_port_advert_remote_get(int unit, bcm_port_t port,
                                   bcm_port_abil_t *ability_mask)
{
    int  an, an_done;

    SOC_IF_ERROR_RETURN(soc_phyctrl_auto_negotiate_get(unit,  port,
                                                       &an, &an_done));

    if (!an) {
        return BCM_E_DISABLED;
    }

    if (!an_done) {
        return BCM_E_BUSY;
    }

    SOC_IF_ERROR_RETURN(soc_phyctrl_adv_remote_get(unit, port, ability_mask));

    return BCM_E_NONE;
}


/*
 * Function:
 *     bcm_port_advert_remote_get
 * Purpose:
 *     Retrieve the local port advertisement for autonegotiation.
 * Parameters:
 *     unit         - Device number
 *     port         - Device port number
 *     ability_mask - (OUT) Remote advertisement
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_port_advert_remote_get(int unit, bcm_port_t port,
                                  bcm_port_abil_t *ability_mask)
{
    int  rv = BCM_E_PORT;

    /* Check params */
    PORT_INIT_CHECK(unit, port);
    PARAM_NULL_CHECK(ability_mask);
    PORT_INTERFACE_MAC_CHECK(unit, port);

    PORT_LOCK(unit);
    rv = _bcm_fe2000_port_advert_remote_get(unit, port, ability_mask);
    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_advert_remote_get: unit=%d port=%d abil=0x%x rv=%d\n",
               unit, port, *ability_mask, rv));

    return rv;
}


/*
 * Function:
 *     bcm_port_advert_set
 * Purpose:
 *     Set the local port advertisement for autonegotiation.
 * Parameters:
 *     unit         - Device number
 *     port         - Device port number
 *     ability_mask - Local advertisement
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     This call MAY NOT restart autonegotiation (depending on the phy).
 *     To do that, follow this call with bcm_port_autoneg_set(TRUE).
 */
int
bcm_fe2000_port_advert_set(int unit, bcm_port_t port,
                           bcm_port_abil_t ability_mask)
{
    int  rv = BCM_E_PORT;

    /* Check params */
    PORT_INIT_CHECK(unit, port);
    PORT_INTERFACE_MAC_CHECK(unit, port);

    if (IS_HG_PORT(unit, port) && SOC_INFO(unit).port_speed_max[port]) {
        if (SOC_INFO(unit).port_speed_max[port] < 16000) {
            ability_mask &= ~(BCM_PORT_ABIL_16GB);
        }

        if (SOC_INFO(unit).port_speed_max[port] < 13000) {
            ability_mask &= ~(BCM_PORT_ABIL_13GB);
        }

        if (SOC_INFO(unit).port_speed_max[port] < 12000) {
            ability_mask &= ~(BCM_PORT_ABIL_12GB);
        }
    }

    PORT_LOCK(unit);
    rv = soc_phyctrl_adv_local_set(unit, port, ability_mask);
    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_advert_set: unit=%d port=%d abil=0x%x rv=%d\n",
               unit, port, ability_mask, rv));

    return rv;
}


/*
 * Function:
 *     bcm_port_cable_diag
 * Description:
 *     Run Cable Diagnostics on port.
 * Parameters:
 *     unit   - Device number
 *     port   - Device port number
 *     status - (OUT) cable diag status structure
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Cable diagnostics are only supported by some phy types
 *     (currently 5248 10/100 phy and 546x 10/100/1000 phys)
 */
int
bcm_fe2000_port_cable_diag(int unit, bcm_port_t port,
                           bcm_port_cable_diag_t *status)
{
    int  rv = BCM_E_PORT;

    /* Check params */
    PORT_INIT_CHECK(unit, port);
    PARAM_NULL_CHECK(status);
    PORT_INTERFACE_MAC_CHECK(unit, port);

    PORT_LOCK(unit);
    rv = soc_phyctrl_cable_diag(unit, port, status);
    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_cable_diag: unit=%d port=%d status=%d rv=%d\n",
               unit, port, status->state, rv));

    return rv;
}


/*
 * Function:
 *     bcm_port_duplex_get
 * Purpose:
 *     Get the port duplex settings.
 * Parameters:
 *     unit   - Device number
 *     port   - Device port number
 *     duplex - (OUT) Duplex setting, one of SOC_PORT_DUPLEX_xxx
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_port_duplex_get(int unit, bcm_port_t port, int *duplex)
{
    int  rv = BCM_E_PORT;
    int  phy_duplex;

    /* Check params */
    PORT_INIT_CHECK(unit, port);
    PARAM_NULL_CHECK(duplex);

    PORT_LOCK(unit);
    if (INTERFACE_MAC(unit, port)) {
        rv = soc_phyctrl_duplex_get(unit, port, &phy_duplex);
        if (SOC_SUCCESS(rv)) {
            *duplex = phy_duplex ? SOC_PORT_DUPLEX_FULL : SOC_PORT_DUPLEX_HALF;
        }

    } else {
        /* Always full duplex for any non-mac interface port */
        *duplex = SOC_PORT_DUPLEX_FULL;
        rv = BCM_E_NONE;
    }
    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_duplex_get: unit=%d port=%d duplex=%d rv=%d\n",
               unit, port, *duplex, rv));

    return rv;
}


/*
 * Function:
 *     bcm_port_duplex_set
 * Purpose:
 *     Set the port duplex settings.
 * Parameters:
 *     unit   - Device number
 *     port   - Device port number
 *     duplex - Duplex setting, one of SOC_PORT_DUPLEX_xxx
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Turns off autonegotiation.  Caller must make sure other forced
 *     parameters (such as speed) are set.
 */
int
bcm_fe2000_port_duplex_set(int unit, bcm_port_t port, int duplex)
{
    int     rv = BCM_E_PORT;
    int     full_duplex = FALSE;
    pbmp_t  pbm;

    /* Check params */
    PORT_INIT_CHECK(unit, port);
    if (duplex == SOC_PORT_DUPLEX_FULL) {
        full_duplex = TRUE;
    } else if (duplex == SOC_PORT_DUPLEX_HALF) {
        full_duplex = FALSE;
    } else {
        return BCM_E_PARAM;
    }

    PORT_LOCK(unit);

    if (INTERFACE_MAC(unit, port)) {
        rv = soc_phyctrl_auto_negotiate_set(unit, port, FALSE);
        if (SOC_FAILURE(rv)) {
            PORT_ERR(("bcm_port_duplex_set: PHY_AUTONEG_SET failed "
                      "rv=%d(%s)\n",
                      rv, soc_errmsg(rv)));
        }

        if (SOC_SUCCESS(rv)) {
            rv = soc_phyctrl_duplex_set(unit, port, full_duplex);
            if (SOC_FAILURE(rv)) {
                PORT_ERR(("bcm_port_duplex_set: PHY_DUPLEX_SET failed "
                          "rv=%d(%s)\n",
                          rv, soc_errmsg(rv)));
            }
        }

        if (SOC_SUCCESS(rv)) {
            rv = MAC_DUPLEX_SET(PORT(unit, port).p_mac, unit, port,
                                full_duplex);
            if (SOC_FAILURE(rv)) {
                PORT_VERB(("bcm_port_duplex_set: MAC_DUPLEX_SET failed "
                           "rv=%d(%s)\n",
                           rv, soc_errmsg(rv)));
            }
        }

    } else {
        /* Can only be full-duplex for any non-mac interface port */
        if (duplex) {
            rv = BCM_E_NONE;
        }
    }

    PORT_UNLOCK(unit);    /* Unlock before link call */

    if (INTERFACE_MAC(unit, port)) {
        if (SOC_SUCCESS(rv) && !SAL_BOOT_SIMULATION) {
            SOC_PBMP_CLEAR(pbm);
            SOC_PBMP_PORT_ADD(pbm, port);
            (void)bcm_link_change(unit, pbm);
        }
    }

    PORT_VERB(("bcm_port_duplex_set: unit=%d port=%d duplex=%d rv=%d\n",
               unit, port, duplex, rv));

    return rv;
}


/*
 * Function:
 *     bcm_port_encap_get
 * Purpose:
 *     Get the port encapsulation mode.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     mode - (OUT) One of BCM_PORT_ENCAP_xxx (see port.h)
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_port_encap_get(int unit, bcm_port_t port, int *mode)
{
    int  rv = BCM_E_PORT;

    /* Check params */
    PORT_INIT_CHECK(unit, port);
    PARAM_NULL_CHECK(mode);

    PORT_LOCK(unit);
    if (INTERFACE_MAC(unit, port)) {
        rv = MAC_ENCAP_GET(PORT(unit, port).p_mac, unit, port, mode);
    } else {
        
    }
    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_encap_get: unit=%d port=%d mode=%d rv=%d\n",
               unit, port, *mode, rv));

    return rv;
}


/*
 * Function:
 *     bcm_port_encap_set
 * Purpose:
 *     Set the port encapsulation mode.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     mode - One of BCM_PORT_ENCAP_xxx (see port.h)
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_port_encap_set(int unit, bcm_port_t port, int mode)
{
    int  rv = BCM_E_PORT;

    /* Check params */
    PORT_INIT_CHECK(unit, port);

    PORT_LOCK(unit);
    if ((IS_HG_PORT(unit,port) && (mode == BCM_PORT_ENCAP_IEEE)) ||
        (IS_XE_PORT(unit,port) && (mode != BCM_PORT_ENCAP_IEEE))) {
        PORT_UNLOCK(unit);
        return BCM_E_UNAVAIL;
    }

    if (INTERFACE_MAC(unit, port)) {
        rv = MAC_ENCAP_SET(PORT(unit, port).p_mac, unit, port, mode);
    } else {
        
    }
    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_encap_set: unit=%d port=%d mode=%d rv=%d\n",
               unit, port, mode, rv));

    return rv;
}

int
bcm_fe2000_port_encap_config_get(int unit,
                                 bcm_gport_t gport,
                                 bcm_port_encap_config_t *config)
{
    bcm_port_encap_config_t working;
    bcm_module_t            myModId;
    bcm_module_t            module;
    int                     mode = -1;
    int                     rv = BCM_E_PORT;

    /* Check params */
    UNIT_INIT_CHECK(unit);
    PARAM_NULL_CHECK(config);

    /* some local initialisation */
    bcm_port_encap_config_t_init(&working);

    /* get local modid */
    rv = bcm_stk_my_modid_get(unit, &myModId);
    if (BCM_E_NONE != rv) {
        PORT_ERR(("unable to get local module ID: %d (%s)\n",
                  rv,
                  _SHR_ERRMSG(rv)));
        return rv;
    }
    module = myModId;

    PORT_LOCK(unit);

    /* convert modport/local to module and port */
    if (BCM_GPORT_IS_MODPORT(gport)) {
        module = BCM_GPORT_MODPORT_MODID_GET(gport);
        gport = BCM_GPORT_MODPORT_PORT_GET(gport);
    } else if (BCM_GPORT_IS_EGRESS_MODPORT(gport)) {
        module = BCM_GPORT_MODPORT_MODID_GET(gport);
        gport = BCM_GPORT_MODPORT_PORT_GET(gport);
    } else if (BCM_GPORT_IS_LOCAL(gport)) {
        gport = BCM_GPORT_LOCAL_GET(gport);
    }

    if (module != myModId) {
        PORT_ERR(("can't read remote port encap config information\n"));
        rv = BCM_E_PARAM;
    }

    if (BCM_E_NONE == rv) {
        if (!BCM_GPORT_IS_SET(gport)) {
            if ((gport >= 0) && (gport < SBX_MAX_PORTS)) {
                if (INTERFACE_MAC(unit, gport)) {
                    rv = MAC_ENCAP_GET(PORT(unit, gport).p_mac,
                                       unit,
                                       gport,
                                       &(mode));
                    working.encap = mode;
                    
                } else {
                    PORT_ERR(("don't know how to read encap config for"
                              " non-MAC port %d\n",
                              gport));
                    rv = BCM_E_UNAVAIL;
                }
            } else {
                /* not a valid simple port */
                rv = BCM_E_PORT;
            }
        
        /* } else if (BCM_GPORT_IS_SOME_TYPE(gport)) { ... */
        } else {
            PORT_ERR(("can't read GPORT %08X encap config information\n",
                      gport));
            rv = BCM_E_UNAVAIL;
        }
    }
    if (BCM_E_NONE == rv) {
        sal_memcpy(config, &working, sizeof(*config));
    }
    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_fe2000_port_encap_config_get: unit=%d port=%d rv=%d\n",
               unit, gport, rv));

    return rv;
}

int
bcm_fe2000_port_encap_config_set(int unit,
                                 bcm_gport_t gport,
                                 const bcm_port_encap_config_t *config)
{
    bcm_module_t myModId;
    bcm_module_t module;
    int  rv = BCM_E_PORT;

    /* Check params */
    UNIT_INIT_CHECK(unit);
    PARAM_NULL_CHECK(config);

    /* get local modid */
    rv = bcm_stk_my_modid_get(unit, &myModId);
    if (BCM_E_NONE != rv) {
        PORT_ERR(("unable to get local module ID: %d (%s)\n",
                  rv,
                  _SHR_ERRMSG(rv)));
        return rv;
    }
    module = myModId;

    PORT_LOCK(unit);

    /* convert modport/local to module and port */
    if (BCM_GPORT_IS_MODPORT(gport)) {
        module = BCM_GPORT_MODPORT_MODID_GET(gport);
        gport = BCM_GPORT_MODPORT_PORT_GET(gport);
    } else if (BCM_GPORT_IS_EGRESS_MODPORT(gport)) {
        module = BCM_GPORT_MODPORT_MODID_GET(gport);
        gport = BCM_GPORT_MODPORT_PORT_GET(gport);
    } else if (BCM_GPORT_IS_LOCAL(gport)) {
        gport = BCM_GPORT_LOCAL_GET(gport);
    }

    if (module != myModId) {
        PORT_ERR(("can't set remote port encap config information\n"));
        rv = BCM_E_PARAM;
    }

    if (BCM_E_NONE == rv) {
        if (!BCM_GPORT_IS_SET(gport)) {
            /*
             *  Set it via the set routine above, to minimise churn when
             *  something changes in that function.
             */
            rv = bcm_fe2000_port_encap_set(unit,
                                           gport,
                                           config->encap);
        
        /* } else if (BCM_GPORT_IS_SOME_TYPE(gport)) { ... */
        } else {
            PORT_ERR(("can't set GPORT %08X encap config information\n",
                      gport));
            rv = BCM_E_UNAVAIL;
        }
    }
    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_fe2000_port_encap_config_set: unit=%d port=%d rv=%d\n",
               unit, gport, rv));

    return rv;
}

/*
 * Function:
 *     bcm_port_enable_get
 * Purpose:
 *     Get the enable state as defined by bcm_port_enable_set().
 * Parameters:
 *     unit   - Device number
 *     port   - Device port number
 *     enable - (OUT) TRUE if port is enabled, FALSE if port is disabled
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     The PHY enable holds the port enable state set by the user.
 *     The MAC enable transitions up and down automatically via linkscan
 *     even if user port enable is always up.
 */
int
bcm_fe2000_port_enable_get(int unit, bcm_port_t port, int *enable)
{
    int  rv = BCM_E_PORT;

    /* Check params */
    PORT_INIT_CHECK(unit, port);
    PARAM_NULL_CHECK(enable);

    PORT_LOCK(unit);

    rv = _bcm_fe2000_port_enable_get(unit, port, enable);

    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_enable_get: unit=%d port=%d rv=%d enable=%d\n",
               unit, port, rv, *enable));

    return rv;
}


/*
 * Function:
 *     bcm_port_enable_set
 * Purpose:
 *     Physically enable/disable the MAC/PHY on this port.
 * Parameters:
 *     unit    - Device number
 *     port    - Device port number
 *     enable  - TRUE to enable port, FALSE to disable port
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     If linkscan is running, it also controls the MAC enable state.
 */
int
bcm_fe2000_port_enable_set(int unit, bcm_port_t port, int enable)
{
    int  rv = BCM_E_PORT;

    /* Check params */
    PORT_INIT_CHECK(unit, port);

    PORT_LOCK(unit);

    rv = _bcm_fe2000_port_enable_set(unit, port, enable);

    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_enable_set: unit=%d port=%d enable=%d rv=%d\n",
               unit, port, enable, rv));

    return rv;
}


/*
 * Function:
 *     _bcm_fe2000_port_frame_max_access
 * Description:
 *     Set/get the maximum receive frame size for the port in the iLib ucode.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     size - (IN/OUT) Maximum frame size in bytes
 *     set  - Indicates whether to set or get
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Assumes lock is held
 */
STATIC int
_bcm_fe2000_port_frame_max_access(int unit, bcm_port_t port, int *size, int set)
{

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        return _bcm_fe2000_g2p3_port_frame_max_access(unit, port, size, set);
        break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        return BCM_E_INTERNAL;
    }
}


/*
 * Function:
 *     bcm_port_frame_max_get
 * Description:
 *     Get the maximum receive frame size for the port.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     size - (OUT) Maximum frame size in bytes
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_port_frame_max_get(int unit, bcm_port_t port, int *size)
{
    int  rv = BCM_E_PORT;

    /* Check params */
    if (BCM_GPORT_IS_SET(port)) {
        UNIT_INIT_CHECK(unit);
    } else {
        PORT_INIT_CHECK(unit, port);
    }
    PARAM_NULL_CHECK(size);

    PORT_LOCK(unit);
    if (BCM_GPORT_IS_VLAN_PORT(port)) {

        rv = bcm_fe2k_vlan_vgp_frame_max_access(unit, port, size, FALSE);
    } else if (BCM_GPORT_IS_MIM_PORT(port)) {

        rv = bcm_fe2k_mim_frame_max_access(unit, port, size, FALSE);
    } else if (INTERFACE_MAC(unit, port)) {
        /* Get the lesser max-frame value of MAC / ucode */
        rv = MAC_FRAME_MAX_GET(PORT(unit, port).p_mac, unit, port, size);

        if (BCM_SUCCESS(rv)) {
            int ilib_size;

            rv = _bcm_fe2000_port_frame_max_access(unit, port, &ilib_size, 0);
            if (BCM_SUCCESS(rv) && (ilib_size < *size)) {
                *size = ilib_size;
            }
        }

    } else if (INTERFACE_SPI(unit, port)) {
        rv = _bcm_fe2000_port_frame_max_access(unit, port, size, 0);
    } else if (port == CMIC_PORT(unit)) {
        rv = _bcm_fe2000_port_frame_max_access(unit, port, size, 0);
    }

    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_frame_max_get: unit=%d port=%d size=%d rv=%d\n",
               unit, port, *size, rv));

    return rv;
}


/*
 * Function:
 *     bcm_port_frame_max_set
 * Description:
 *     Set the maximum receive frame size for the port.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     size - Maximum frame size in bytes
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_port_frame_max_set(int unit, bcm_port_t port, int size)
{
    int  rv = BCM_E_PORT;
    int  max_limit = BCM_FE2000_PORT_FRAME_MAX;

    /* Check params */
    if (BCM_GPORT_IS_SET(port)) {
        UNIT_INIT_CHECK(unit);
    } else {
        PORT_INIT_CHECK(unit, port);

        if (IS_XE_PORT(unit,port)) {
            max_limit = BCM_FE2000_PORT_XE_FRAME_MAX;
        }
    }

    if ((size < 0) || (size > max_limit)) {
        return BCM_E_PARAM;
    }

    PORT_LOCK(unit);

    if (BCM_GPORT_IS_VLAN_PORT(port)) {

        rv = bcm_fe2k_vlan_vgp_frame_max_access(unit, port, &size, TRUE);
    } else if (BCM_GPORT_IS_MIM_PORT(port)) {

        rv = bcm_fe2k_mim_frame_max_access(unit, port, &size, TRUE);
    } else if (INTERFACE_MAC(unit, port)) {

        /* Set the max frame size in MAC and ucode egress mtu */
        rv = MAC_FRAME_MAX_SET(PORT(unit, port).p_mac, unit, port, size);

        if (BCM_SUCCESS(rv)) {
            rv = _bcm_fe2000_port_frame_max_access(unit, port, &size, TRUE);
        }

    } else if (INTERFACE_SPI(unit, port)) {
        /* Set the ucode egress mtu size */
        rv = _bcm_fe2000_port_frame_max_access(unit, port, &size, TRUE);
    } else if (port == CMIC_PORT(unit)) {
        rv = _bcm_fe2000_port_frame_max_access(unit, port, &size, TRUE);
    }


    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_frame_max_set: unit=%d port=%d size=%d rv=%d\n",
               unit, port, size, rv));

    return rv;
}


/*
 * Function:
 *     bcm_port_ifg_get
 * Description:
 *     Get the new ifg (Inter-frame gap) value.
 * Parameters:
 *     unit   - Device number
 *     port   - Device port number
 *     speed  - The speed for which the IFG is being set
 *     duplex - The duplex for which the IFG is being set
 *     ifg    - (OUT) Inter-frame gap in bit-times
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 */
int
bcm_fe2000_port_ifg_get(int unit, bcm_port_t port,
                        int speed, bcm_port_duplex_t duplex, int *ifg)
{
    int  rv = BCM_E_PORT;

    /* Check params */
    PORT_INIT_CHECK(unit, port);
    PARAM_NULL_CHECK(ifg);
    PORT_INTERFACE_MAC_CHECK(unit, port);

    PORT_LOCK(unit);
    rv = MAC_IFG_GET(PORT(unit, port).p_mac, unit, port,
                     speed, duplex, ifg);
    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_ifg_get: unit=%d port=%d speed=%d duplex=%d "
               "ifg=%d rv=%d\n",
               unit, port, speed, duplex, *ifg, rv));

    return rv;
}


/*
 * Function:
 *     bcm_port_ifg_set
 * Description:
 *     Set the new ifg (Inter-frame gap) value.
 * Parameters:
 *     unit   - Device number
 *     port   - Device port number
 *     speed  - The speed for which the IFG is being set
 *     duplex - The duplex for which the IFG is being set
 *     ifg    - Inter-frame gap in bit-times
 * Return Value:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     The function makes sure the IFG value makes sense and updates the
 *     IPG register in case the speed/duplex match the current settings
 */
int
bcm_fe2000_port_ifg_set(int unit, bcm_port_t port,
                        int speed, bcm_port_duplex_t duplex, int ifg)
{
    int  rv = BCM_E_PORT;

    /* Check params */
    PORT_INIT_CHECK(unit, port);
    PORT_INTERFACE_MAC_CHECK(unit, port);

    PORT_LOCK(unit);
    rv = MAC_IFG_SET(PORT(unit, port).p_mac, unit, port,
                     speed, duplex, ifg);
    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_ifg_set: unit=%d port=%d speed=%d duplex=%d "
               "ifg=%d rv=%d\n",
               unit, port, speed, duplex, ifg, rv));

    return rv;
}


/*
 * Function:
 *     bcm_port_info_get
 * Purpose:
 *     Get all information on the port.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     info - Pointer to structure in which to save values
 * Returns:
 *     BCM_E_XXX
 */
int
bcm_fe2000_port_info_get(int unit, bcm_port_t port, bcm_port_info_t *info)
{
    /* Check params */
    PORT_INIT_CHECK(unit, port);
    PARAM_NULL_CHECK(info);

    bcm_port_info_t_init(info);

    if (INTERFACE_MAC(unit, port)) {
        info->action_mask = BCM_FE2000_PORT_ATTR_ALL_MASK;

    } else if (INTERFACE_SPI(unit, port)) {
        info->action_mask = (BCM_PORT_ATTR_ENABLE_MASK |
                             BCM_PORT_ATTR_LINKSTAT_MASK |
                             BCM_PORT_ATTR_DUPLEX_MASK);
        

    } else {
        
    }

    return bcm_port_selective_get(unit, port, info);
}


/*
 * Function:
 *     bcm_port_info_restore
 * Purpose:
 *     Restore port settings saved by info_save.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     info - Pointer to structure with info from port_info_save
 * Returns:
 *     BCM_E_XXX
 * Notes:
 *     bcm_port_info_save has done all the work.
 *     We just call port_selective_set.
 */
int
bcm_fe2000_port_info_restore(int unit, bcm_port_t port, bcm_port_info_t *info)
{
    return bcm_port_selective_set(unit, port, info);
}


/*
 * Function:
 *     bcm_port_info_save
 * Purpose:
 *     Save the current settings of a port.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     info - Pointer to structure in which to save values
 * Returns:
 *     BCM_E_XXX
 * Notes:
 *     The action_mask will be adjusted so that the
 *     proper values will be set when a restore is made.
 *     This mask should not be altered between these calls.
 */
int
bcm_fe2000_port_info_save(int unit, bcm_port_t port, bcm_port_info_t *info)
{
    /* Check params */
    PORT_INIT_CHECK(unit, port);
    PARAM_NULL_CHECK(info);

    if (INTERFACE_MAC(unit, port)) {
        info->action_mask = BCM_FE2000_PORT_ATTR_ALL_MASK;

    } else if (INTERFACE_SPI(unit, port)) {
        info->action_mask = (BCM_PORT_ATTR_ENABLE_MASK |
                             BCM_PORT_ATTR_LINKSTAT_MASK |
                             BCM_PORT_ATTR_DUPLEX_MASK);
        

    } else {
        
    }

    BCM_IF_ERROR_RETURN(bcm_port_selective_get(unit, port, info));

    if (info->autoneg) {
        info->action_mask &= ~BCM_PORT_AN_ATTRS;
    }

    return BCM_E_NONE;
}


/*
 * Function:
 *     bcm_port_info_set
 * Purpose:
 *     Set all information on the port.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     info - Pointer to structure in which to save values
 * Returns:
 *     BCM_E_XXX
 * Notes:
 *     Checks if AN is on, and if so, clears the
 *     proper bits in the action mask.
 */
int
bcm_fe2000_port_info_set(int unit, bcm_port_t port, bcm_port_info_t *info)
{
    /* Check params */
    PORT_INIT_CHECK(unit, port);
    PARAM_NULL_CHECK(info);

    if (INTERFACE_MAC(unit, port)) {
        info->action_mask = BCM_FE2000_PORT_ATTR_ALL_MASK;

    } else if (INTERFACE_SPI(unit, port)) {
        info->action_mask = (BCM_PORT_ATTR_ENABLE_MASK |
                             BCM_PORT_ATTR_LINKSTAT_MASK |
                             BCM_PORT_ATTR_DUPLEX_MASK);
        

    } else {
        
    }

    /* If autoneg is set, remove those attributes controlled by it */
    if (info->autoneg) {
        info->action_mask &= ~BCM_PORT_AN_ATTRS;
    }

    return bcm_port_selective_set(unit, port, info);
}


/*
 * Function:
 *     bcm_port_interface_get
 * Purpose:
 *     Get the interface type of a port.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     intf - (OUT) BCM_PORT_IF_*
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     WARNING: assumes BCM_PORT_IF_* matches SOC_PORT_IF_*
 */
int
bcm_fe2000_port_interface_get(int unit, bcm_port_t port, bcm_port_if_t *intf)
{
    int  rv = BCM_E_PORT;

    /* Check params */
    PORT_INIT_CHECK(unit, port);
    PARAM_NULL_CHECK(intf);

    PORT_LOCK(unit);
    if (INTERFACE_MAC(unit, port)) {
        rv = soc_phyctrl_interface_get(unit, port, intf);

    } else if (INTERFACE_SPI(unit, port)) {
        
        rv = BCM_E_NONE;
    } else {
        
    }
    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_interface_get: unit=%d port=%d intf=%d rv=%d\n",
               unit, port, *intf, rv));

    return rv;
}


/*
 * Function:
 *     bcm_port_interface_set
 * Purpose:
 *     Set the interface type for a given port.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     intf - BCM_PORT_IF_*
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     WARNING: assumes BCM_PORT_IF_* matches SOC_PORT_IF_*
 */
int
bcm_fe2000_port_interface_set(int unit, bcm_port_t port, bcm_port_if_t intf)
{
    int     rv = BCM_E_PORT;
    pbmp_t  pbm;

    /* Check params */
    PORT_INIT_CHECK(unit, port);

    PORT_LOCK(unit);
    if (INTERFACE_MAC(unit, port)) {
        rv = soc_phyctrl_interface_set(unit, port, intf);

    } else if (INTERFACE_SPI(unit, port)) {
        

    } else {
        
    }
    PORT_UNLOCK(unit);    /* Unlock before link call */

    if (INTERFACE_MAC(unit, port)) {
        if (SOC_SUCCESS(rv)) {
            SOC_PBMP_CLEAR(pbm);
            SOC_PBMP_PORT_ADD(pbm, port);
            (void)bcm_link_change(unit, pbm);
        }
    }

    PORT_VERB(("bcm_port_interface_set: unit=%d port=%d intf=%d rv=%d\n",
               unit, port, intf, rv));

    return rv;
}


/*
 * Function:
 *     _bcm_fe2000_port_hw_link_get
 * Purpose:
 *     Return current port status from hardware linkscan.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     up   - (OUT) TRUE for link up, FALSE for link down
 * Returns:
 *     BCM_E_NONE
 *     BCM_E_XXX
 */
STATIC int
_bcm_fe2000_port_hw_link_get(int unit, bcm_port_t port, int *up)
{
    int       rv = BCM_E_NONE;
    int       miim_port;
    sbhandle  sb_handle;
    uint32    data;
    pbmp_t    hw_linkstat;


    /*
     * Note that since port numbers can be configured,
     * a 'port' number needs to be translated to the
     * corresponding physical port number expected in MIIM data.
     */

    /* Convert bcm port to device miim port */
    SOC_IF_ERROR_RETURN(soc_linkctrl_miim_port_get(unit, port, &miim_port));

    /* Read HW link status */
    sb_handle = SOC_SBX_SBHANDLE(unit);
    data = SAND_HAL_FE2000_READ(sb_handle, PC_MIIM_LINK_STATUS);
    SOC_PBMP_CLEAR(hw_linkstat);
    SOC_PBMP_WORD_SET(hw_linkstat, 0,
                      SAND_HAL_FE2000_GET_FIELD(sb_handle, PC_MIIM_LINK_STATUS,
                                         LINK_STATUS, data));

    *up = PBMP_MEMBER(hw_linkstat, miim_port);
    if (!*up) {
        int ls_ignore;
        rv = BCM_E_NONE;
        /*
         * Clear the Link down bit (LL - So a read is required to clear)
         */
        if (soc_feature(unit, soc_feature_dodeca_serdes) &&
            IS_GE_PORT(unit, port)) {
            rv = soc_phyctrl_link_get(unit, port, &ls_ignore);
        }
    } else {
        /*
         * For some PHYs such as 5228, hardware linkscan may say the
         * link is up while the PHY is actually not quite done
         * autonegotiating.  Therefore we call the actual PHY link
         * routine, which waits for autoneg to complete.
         */
        rv = soc_phyctrl_link_get(unit, port, up);
    }

    return rv;
}


/*
 * Function:
 *     _bcm_fe2000_port_link_get
 * Purpose:
 *     Main internal routine to return current up/down status.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     hw   - If TRUE, assume hardware linkscan is active and use it
 *            to reduce PHY reads.
 *            If FALSE, do not use information from hardware linkscan.
 *     up   - (OUT) TRUE for link up, FALSE for link down
 * Returns:
 *     BCM_E_NONE
 *     BCM_E_XXX
 */
int
_bcm_fe2000_port_link_get(int unit, bcm_port_t port, int hw, int *up)
{
    int  rv = BCM_E_NONE;

    PORT_INIT_CHECK(unit, port);
    PORT_LOCK(unit);

    if (INTERFACE_MAC(unit, port)) {

        if (hw) {
            rv = _bcm_fe2000_port_hw_link_get(unit, port, up);
        } else {
            rv = soc_phyctrl_link_get(unit, port, up);
        }

        if (BCM_SUCCESS(rv) &&
            (IS_XE_PORT(unit, port) || IS_HG_PORT(unit,port))) {
            uint32 data_hi, data_lo;
            uint32 rmt_fault, lcl_fault;

            rv = soc_sbx_fe2000_xm_read(unit,
                                        SOC_PORT_BLOCK_NUMBER(unit, port),
                                        REG_OFFSET(unit, XM_MAC_RX_LSS_STATUS_LO),
                                        &data_hi, &data_lo);

            rmt_fault = SAND_HAL_FE2000_GET_FIELD(SOC_SBX_SBHANDLE(unit), XM_MAC_RX_LSS_STATUS_LO,
                                           REMOTE_FAULT, data_lo);
            lcl_fault = SAND_HAL_FE2000_GET_FIELD(SOC_SBX_SBHANDLE(unit), XM_MAC_RX_LSS_STATUS_LO,
                                           LOCAL_FAULT, data_lo);
            if (rmt_fault || lcl_fault) {
                *up = FALSE;
            }
        }

    } else if (INTERFACE_SPI(unit, port)) {
        /*
         * For SPI subports, use status of SPI bus
         */
        if (hwFe2000Spi4RxStatusNoDebounce(SOC_SBX_SBHANDLE(unit),
                                           PORT_SPI_BUS(unit, port))) {
            *up = FALSE;
        } else {
            *up = TRUE;
        }
    } else {
        
    }

    PORT_UNLOCK(unit);

    if (BCM_SUCCESS(rv)) {
        if (INTERFACE_MAC(unit, port)) {
            if (PHY_FLAGS_TST(unit, port, PHY_FLAGS_MEDIUM_CHANGE)) {
                soc_port_medium_t  medium;
                soc_phyctrl_medium_get(unit, port, &medium);
                soc_phy_medium_status_notify(unit, port, medium);
            }
        }
    }

#if 0 
    PORT_VERB(("_bcm_port_link_get: unit=%d port=%d up=%d rv=%d\n",
              unit, port, *up, rv));
#endif 

    return rv;
}


/*
 * Function:
 *     bcm_port_link_status_get
 * Purpose:
 *     Return current Link up/down status, queries linkscan, if unable to
 *     retrieve status queries the PHY.
 * Parameters:
 *     unit  - Device number
 *     port  - Device port number
 *     up    - (OUT) Boolean value, FALSE for link down and TRUE for link up
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_port_link_status_get(int unit, bcm_port_t port, int *up)
{
    int  rv = BCM_E_NONE;

    /* Check params */
    PORT_INIT_CHECK(unit, port);
    PARAM_NULL_CHECK(up);

    rv = _bcm_link_get(unit, port, up);

    if (rv == BCM_E_DISABLED) {
        rv = _bcm_fe2000_port_link_get(unit, port, 0, up);
    }

    PORT_VERB(("bcm_port_link_status_get: unit=%d port=%d up=%d rv=%d\n",
               unit, port, *up, rv));

    return rv;
}


/*
 * Function:
 *     bcm_port_master_get
 * Purpose:
 *     Get the master status of a port.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     ms   - (OUT) BCM_PORT_MS_*
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     WARNING: assumes BCM_PORT_MS_* matches SOC_PORT_MS_*
 */
int
bcm_fe2000_port_master_get(int unit, bcm_port_t port, int *ms)
{
    int     rv = BCM_E_PORT;

    /* Check params */
    PORT_INIT_CHECK(unit, port);
    PARAM_NULL_CHECK(ms);
    PORT_INTERFACE_MAC_CHECK(unit, port);

    PORT_LOCK(unit);
    rv = soc_phyctrl_master_get(unit, port, ms);
    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_master_get: unit=%d port=%d ms=%d rv=%d\n",
               unit, port, *ms, rv));

    return rv;
}


/*
 * Function:
 *     bcm_port_master_set
 * Purpose:
 *     Set the master status for a given port.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     ms   - BCM_PORT_MS_*
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Ignored if not supported on port.
 *     WARNING: assumes BCM_PORT_MS_* matches SOC_PORT_MS_*
 */
int
bcm_fe2000_port_master_set(int unit, bcm_port_t port, int ms)
{
    int     rv = BCM_E_PORT;
    pbmp_t  pbm;

    /* Check params */
    PORT_INIT_CHECK(unit, port);
    PORT_INTERFACE_MAC_CHECK(unit, port);

    PORT_LOCK(unit);
    rv = soc_phyctrl_master_set(unit, port, ms);
    PORT_UNLOCK(unit);    /* Unlock before link call */

    if (SOC_SUCCESS(rv)) {
        SOC_PBMP_CLEAR(pbm);
        SOC_PBMP_PORT_ADD(pbm, port);
        (void)bcm_link_change(unit, pbm);
    }

    PORT_VERB(("bcm_port_master_set: unit=%d port=%d ms=%d rv=%d\n",
               unit, port, ms, rv));

    return rv;
}


/*
 * Function:
 *     bcm_port_mdix_get
 * Description:
 *     Get the Auto-MDIX mode of a port/PHY.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     mode - (Out) One of:
 *             BCM_PORT_MDIX_AUTO
 *                     Enable auto-MDIX when autonegotiation is enabled
 *             BCM_PORT_MDIX_FORCE_AUTO
 *                     Enable auto-MDIX always
 *             BCM_PORT_MDIX_NORMAL
 *                     Disable auto-MDIX
 *             BCM_PORT_MDIX_XOVER
 *                     Disable auto-MDIX, and swap cable pairs
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_port_mdix_get(int unit, bcm_port_t port, bcm_port_mdix_t *mode)
{
    int  rv = BCM_E_PORT;

    /* Check params */
    PORT_INIT_CHECK(unit, port);
    PARAM_NULL_CHECK(mode);
    PORT_INTERFACE_MAC_CHECK(unit, port);

    PORT_LOCK(unit);
    rv = soc_phyctrl_mdix_get(unit, port, mode);
    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_mdix_get: unit=%d port=%d mode=%d rv=%d\n",
               unit, port, *mode, rv));

    return rv;
}


/*
 * Function:
 *     bcm_port_mdix_set
 * Description:
 *     Set the Auto-MDIX mode of a port/PHY.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     mode - One of:
 *            BCM_PORT_MDIX_AUTO
 *                    Enable auto-MDIX when autonegotiation is enabled
 *            BCM_PORT_MDIX_FORCE_AUTO
 *                    Enable auto-MDIX always
 *            BCM_PORT_MDIX_NORMAL
 *                    Disable auto-MDIX
 *            BCM_PORT_MDIX_XOVER
 *                    Disable auto-MDIX, and swap cable pairs
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_port_mdix_set(int unit, bcm_port_t port, bcm_port_mdix_t mode)
{
    int  rv = BCM_E_PORT;

    /* Check params */
    PORT_INIT_CHECK(unit, port);
    PORT_INTERFACE_MAC_CHECK(unit, port);

    PORT_LOCK(unit);
    rv = soc_phyctrl_mdix_set(unit, port, mode);
    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_mdix_set: unit=%d port=%d mode=%d rv=%d\n",
               unit, port, mode, rv));

    return rv;
}


/*
 * Function:
 *     bcm_port_mdix_status_get
 * Description:
 *     Get the current MDIX status on a port/PHY.
 * Parameters:
 *     unit   - Device number
 *     port   - Device port number
 *     status - (OUT) One of:
 *               BCM_PORT_MDIX_STATUS_NORMAL
 *                     Straight connection
 *               BCM_PORT_MDIX_STATUS_XOVER
 *                     Crossover has been performed
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_port_mdix_status_get(int unit, bcm_port_t port,
                                bcm_port_mdix_status_t *status)
{
    int  rv = BCM_E_PORT;

    /* Check params */
    PORT_INIT_CHECK(unit, port);
    PARAM_NULL_CHECK(status);
    PORT_INTERFACE_MAC_CHECK(unit, port);

    PORT_LOCK(unit);
    rv = soc_phyctrl_mdix_status_get(unit, port, status);
    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_mdix_status_get: unit=%d port=%d status=%d rv=%d\n",
               unit, port, *status, rv));

    return rv;
}


/*
 * Function:
 *     bcm_port_medium_config_get
 * Description:
 *     Get the medium-specific configuration for a combo port.
 * Parameters:
 *     unit   - Device number
 *     port   - Device port number
 *     medium - The medium (BCM_PORT_MEDIUM_COPPER or BCM_PORT_MEDIUM_FIBER)
 *              to get the config for
 *     config - (OUT) Per-medium configuration
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_port_medium_config_get(int unit, bcm_port_t port,
                                  bcm_port_medium_t medium,
                                  bcm_phy_config_t *config)
{
    int  rv = BCM_E_PORT;

    /* Check params */
    PORT_INIT_CHECK(unit, port);
    PARAM_NULL_CHECK(config);
    PORT_INTERFACE_MAC_CHECK(unit, port);

    PORT_LOCK(unit);
    rv = soc_phyctrl_medium_config_get(unit, port, medium, config);
    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_medium_config_get: unit=%d port=%d medium=%d rv=%d\n",
               unit, port, medium, rv));

    return rv;
}


/*
 * Function:
 *     bcm_port_medium_config_set
 * Description:
 *     Set the medium-specific configuration for a combo port.
 * Parameters:
 *     unit   - Device number
 *     port   - Device port number
 *     medium - The medium (BCM_PORT_MEDIUM_COPPER or BCM_PORT_MEDIUM_FIBER)
 *              to apply the configuration to
 *     config - Per-medium configuration
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_port_medium_config_set(int unit, bcm_port_t port,
                                  bcm_port_medium_t medium,
                                  bcm_phy_config_t *config)
{
    int  rv = BCM_E_PORT;

    /* Check params */
    PORT_INIT_CHECK(unit, port);
    PARAM_NULL_CHECK(config);
    PORT_INTERFACE_MAC_CHECK(unit, port);

    PORT_LOCK(unit);
    rv = soc_phyctrl_medium_config_set(unit, port, medium, config);
    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_medium_config_set: unit=%d port=%d medium=%d rv=%d\n",
               unit, port, medium, rv));

    return rv;
}


/*
 * Function:
 *     bcm_port_medium_get
 * Description:
 *     Get the current medium used by a combo port.
 * Parameters:
 *     unit   - Device number
 *     port   - Device port number
 *     medium - (OUT) The medium (BCM_PORT_MEDIUM_COPPER or
 *               BCM_PORT_MEDIUM_FIBER) which is currently selected
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_port_medium_get(int unit, bcm_port_t port,
                           bcm_port_medium_t *medium)
{
    int  rv = BCM_E_PORT;

    /* Check params */
    PORT_INIT_CHECK(unit, port);
    PARAM_NULL_CHECK(medium);
    PORT_INTERFACE_MAC_CHECK(unit, port);

    PORT_LOCK(unit);
    rv = soc_phyctrl_medium_get(unit, port, medium);
    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_medium_get: unit=%d port=%d medium=%d rv=%d\n",
               unit, port, *medium, rv));
    return rv;
}


/*
 * Function:
 *     bcm_port_medium_status_register
 * Description:
 *     Register a callback function to be called on medium change event.
 * Parameters:
 *     unit      - Device number
 *     port      - Device port number
 *     callback  - The callback function to call
 *     user_data - An opaque cookie to pass to callback function
 *                 whenever it is called
 * Returns:
 *     BCM_E_NONE  - Success
 *     BCM_E_PARAM - NULL function pointer or bad {unit, port} combination
 *     BCM_E_FULL  - Cannot register more than 1 callback per {unit, port}
 *     BCM_E_XXX   - Failure
 */
int
bcm_fe2000_port_medium_status_register(int unit, bcm_port_t port,
                                       bcm_port_medium_status_cb_t callback,
                                       void *user_data)
{
    int  rv = BCM_E_PORT;

    /* Check params */
    PORT_INIT_CHECK(unit, port);
    PORT_INTERFACE_MAC_CHECK(unit, port);

    PORT_LOCK(unit);
    rv = soc_phy_medium_status_register(unit, port, callback, user_data);
    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_medium_status_register: unit=%d port=%d rv=%d\n",
               unit, port, rv));

    return rv;
}


/*
 * Function:
 *     bcm_port_medium_status_unregister
 * Description:
 *     Unregister a callback function to be called on medium change event.
 * Parameters:
 *     unit      - Device number
 *     port      - Device port number
 *     callback  - The callback function to call
 *     user_data - An opaque cookie to pass to callback function
 *                 whenever it is called
 * Returns:
 *     BCM_E_NONE      - Success
 *     BCM_E_PARAM     - Bad {unit, port} combination
 *     BCM_E_NOT_FOUND - The specified {unit, port, callback, user_data}
 *                       combination have not been registered before
 *     BCM_E_XXX       - Failure
 */
int
bcm_fe2000_port_medium_status_unregister(int unit,
                                         bcm_port_t port,
                                         bcm_port_medium_status_cb_t callback,
                                         void *user_data)
{
    int  rv = BCM_E_PORT;

    /* Check params */
    PORT_INIT_CHECK(unit, port);
    PORT_INTERFACE_MAC_CHECK(unit, port);

    PORT_LOCK(unit);
    rv = soc_phy_medium_status_unregister(unit, port, callback, user_data);
    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_medium_status_unregister: unit=%d port=%d rv=%d\n",
               unit, port, rv));

    return rv;
}


/*
 * Function:
 *     bcm_port_pause_addr_get
 * Purpose:
 *     Get the source address for transmitted PAUSE frames.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     mac  - (OUT) MAC address sent with pause frames
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_port_pause_addr_get(int unit, bcm_port_t port, bcm_mac_t mac)
{
    int  rv = BCM_E_PORT;

    /* Check params */
    PORT_INIT_CHECK(unit, port);
    PORT_INTERFACE_MAC_CHECK(unit, port);

    PORT_LOCK(unit);
    rv = MAC_PAUSE_ADDR_GET(PORT(unit, port).p_mac, unit, port, mac);
    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_pause_addr_get: unit=%d port=%d rv=%d\n",
               unit, port, rv));

    return rv;
}


/*
 * Function:
 *     bcm_port_pause_addr_set
 * Purpose:
 *     Set the source address for transmitted PAUSE frames.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     mac  - MAC address used for pause frames
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Symmetric pause requires the two "pause" values to be the same.
 */
int
bcm_fe2000_port_pause_addr_set(int unit, bcm_port_t port, bcm_mac_t mac)
{
    int  rv = BCM_E_PORT;

    /* Check params */
    PORT_INIT_CHECK(unit, port);
    PORT_INTERFACE_MAC_CHECK(unit, port);

    PORT_LOCK(unit);
    rv = MAC_PAUSE_ADDR_SET(PORT(unit, port).p_mac, unit, port, mac);
    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_pause_addr_set: unit=%d port=%d rv=%d\n",
               unit, port, rv));

    return rv;
}


/*
 * Function:
 *     bcm_port_pause_get
 * Purpose:
 *     Get the pause state for a given port.
 * Parameters:
 *     unit     - Device number
 *     port     - Device port number
 *     pause_tx - (OUT) Boolean value
 *     pause_rx - (OUT) Boolean value
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_port_pause_get(int unit, bcm_port_t port,
                          int *pause_tx, int *pause_rx)
{
    int  rv = BCM_E_PORT;

    /* Check params */
    PORT_INIT_CHECK(unit, port);
    PARAM_NULL_CHECK(pause_tx);
    PARAM_NULL_CHECK(pause_rx);
    PORT_INTERFACE_MAC_CHECK(unit, port);

    PORT_LOCK(unit);
    rv = MAC_PAUSE_GET(PORT(unit, port).p_mac, unit, port,
                       pause_tx, pause_rx);
    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_pause_get: unit=%d port=%d pause_tx=%d pause_rx=%d "
               "rv=%d\n",
               unit, port, *pause_tx, *pause_rx, rv));

    return rv;
}


/*
 * Function:
 *     bcm_port_pause_set
 * Purpose:
 *     Set the pause state for a given port.
 * Parameters:
 *     unit     - Device number
 *     port     - Device port number
 *     pause_tx - Boolean value, or -1 (don't change)
 *     pause_rx - Boolean value, or -1 (don't change)
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Symmetric pause requires the two "pause" values to be the same.
 */
int
bcm_fe2000_port_pause_set(int unit, bcm_port_t port,
                          int pause_tx, int pause_rx)
{
    int  rv = BCM_E_PORT;

    /* Check params */
    PORT_INIT_CHECK(unit, port);
    PORT_INTERFACE_MAC_CHECK(unit, port);

    PORT_LOCK(unit);
    rv = MAC_PAUSE_SET(PORT(unit, port).p_mac, unit, port,
                       pause_tx, pause_rx);
    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_pause_set: unit=%d port=%d pause_tx=%d pause_rx=%d "
               "rv=%d\n",
               unit, port, pause_tx, pause_rx, rv));

    return rv;
}


/*
 * Function:
 *     bcm_port_pause_sym_get
 * Purpose:
 *     Get the current pause setting for pause.
 * Parameters:
 *     unit  - Device number
 *     port  - Device port number
 *     pause - (OUT) Returns a bcm_port_pause_e enum value
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_port_pause_sym_get(int unit, bcm_port_t port, int *pause)
{
    int  rv = BCM_E_PORT;
    int  pause_rx, pause_tx;

    /* Check params */
    PORT_INIT_CHECK(unit, port);
    PARAM_NULL_CHECK(pause);
    PORT_INTERFACE_MAC_CHECK(unit, port);

    PORT_LOCK(unit);
    rv = MAC_PAUSE_GET(PORT(unit, port).p_mac, unit, port,
                       &pause_tx, &pause_rx);
    PORT_UNLOCK(unit);

    BCM_IF_ERROR_RETURN(rv);
    if (pause_tx) {
        if (pause_rx) {
            *pause = BCM_PORT_PAUSE_SYM;
        } else {
            *pause = BCM_PORT_PAUSE_ASYM_TX;
        }
    } else if (pause_rx) {
        *pause = BCM_PORT_PAUSE_ASYM_RX;
    } else {
        *pause = BCM_PORT_PAUSE_NONE;
    }

    PORT_VERB(("bcm_port_pause_sym_get: unit=%d port=%d pause=%d rv=%d\n",
               unit, port, *pause, rv));

    return rv;
}


/*
 * Function:
 *     bcm_port_pause_sym_set
 * Purpose:
 *     Set the pause values for the port using single integer.
 * Parameters:
 *     unit  - Device number
 *     port  - Device port number
 *     pause - A bcm_port_pause_e enum value
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_port_pause_sym_set(int unit, bcm_port_t port, int pause)
{
    int  rv = BCM_E_PORT;
    int  pause_rx, pause_tx;

    /* Check params */
    PORT_INIT_CHECK(unit, port);
    PORT_INTERFACE_MAC_CHECK(unit, port);

    pause_tx = pause_rx = 0;

    switch (pause) {
    case BCM_PORT_PAUSE_SYM:
        pause_tx = pause_rx = 1;
        break;
    case BCM_PORT_PAUSE_ASYM_RX:
        pause_rx = 1;
        break;
    case BCM_PORT_PAUSE_ASYM_TX:
        pause_tx = 1;
        break;
    case BCM_PORT_PAUSE_NONE:
        break;
    default:
        return BCM_E_PARAM;
    }

    PORT_LOCK(unit);
    rv = MAC_PAUSE_SET(PORT(unit, port).p_mac, unit, port,
                       pause_tx, pause_rx);
    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_pause_sym_set: unit=%d port=%d pause=%d rv=%d\n",
               unit, port, pause, rv));

    return rv;
}


/*
 * Function:
 *     bcm_port_phy_control_get
 * Description:
 *     Get PHY specific properties.
 * Parameters:
 *     unit   - Device number
 *     port   - Device port number
 *     type   - Configuration type
 *     value  - (OUT) Value for the configuration
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_port_phy_control_get(int unit, bcm_port_t port,
                                bcm_port_phy_control_t type, uint32 *value)
{
    int  rv = BCM_E_PORT;

    /* Check params */
    PORT_INIT_CHECK(unit, port);
    PARAM_NULL_CHECK(value);
    PORT_INTERFACE_MAC_CHECK(unit, port);

    PORT_LOCK(unit);
    rv = soc_phyctrl_control_get(unit, port, type, value);
    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_phy_control_get: unit=%d port=%d type=%d value=0x%x "
               "rv=%d\n",
               unit, port, type, *value, rv));

    return rv;
}


/*
 * Function:
 *     bcm_port_phy_control_set
 * Description:
 *     Set PHY specific properties.
 * Parameters:
 *     unit   - Device number
 *     port   - Device port number
 *     type   - Configuration type
 *     value  - New value for the configuration
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_port_phy_control_set(int unit, bcm_port_t port,
                                bcm_port_phy_control_t type, uint32 value)
{
    int  rv = BCM_E_PORT;

    /* Check params */
    PORT_INIT_CHECK(unit, port);
    PORT_INTERFACE_MAC_CHECK(unit, port);

    PORT_LOCK(unit);
    rv = soc_phyctrl_control_set(unit, port, type, value);
    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_phy_control_set: unit=%d port=%d type=%d value=0x%x "
               "rv=%d\n",
               unit, port, type, value, rv));

    return rv;
}

/*
 * Function:
 *      bcm_port_phy_firmware_set
 * Purpose:
 *      Write the firmware to the PHY device's non-volatile storage.
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) Port number
 *      flags - (IN) PHY spcific flags, such as BCM_PORT_PHY_INTERNAL
 *      offset - (IN) Offset to the firmware data array
 *      array - (IN)  The firmware data array
 *      length - (IN) The length of the firmware data array
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_fe2000_port_phy_firmware_set(int unit, bcm_port_t port, uint32 flags,
                              int offset, uint8 *array, int length)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function    : _bcm_fe2000_gport_construct
 * Description : Internal function to construct a gport from
 *                given parameters
 * Parameters  : (IN)  unit       - BCM device number
 *               (IN)  gport_dest - Structure that contains destination
 *                                   to encode into a gport
 *               (OUT) gport      - Global port identifier
 * Returns     : BCM_E_XXX
 * Notes       : The modid and port are translated from the
 *               local modid/port space to application space
 */
int
_bcm_fe2000_gport_construct(int unit, _bcm_fe2000_gport_dest_t *gport_dest, bcm_gport_t *gport)
{

    bcm_gport_t     l_gport = 0;

    if ((NULL == gport_dest) || (NULL == gport) ){
        return BCM_E_PARAM;
    }

    switch (gport_dest->gport_type) {
        case _SHR_GPORT_TYPE_TRUNK:
            SOC_GPORT_TRUNK_SET(l_gport, gport_dest->tgid);
            break;
        case _SHR_GPORT_TYPE_LOCAL_CPU:
            l_gport = BCM_GPORT_LOCAL_CPU;
            break;
        case _SHR_GPORT_TYPE_LOCAL:
            SOC_GPORT_LOCAL_SET(l_gport, gport_dest->port);
            break;
        case _SHR_GPORT_TYPE_MPLS_PORT:
            BCM_GPORT_MPLS_PORT_ID_SET(l_gport, gport_dest->mpls_id);
            break;
        case _SHR_GPORT_TYPE_MIM_PORT:
            BCM_GPORT_MIM_PORT_ID_SET(l_gport, gport_dest->mim_id);
            break;
        case _SHR_GPORT_TYPE_MODPORT:
            SOC_GPORT_MODPORT_SET(l_gport, gport_dest->modid, gport_dest->port);
            break;
        default:
            return BCM_E_PARAM;
    }

    *gport = l_gport;
    return BCM_E_NONE;
}

/*
 * Function    : bcm_port_gport_get
 * Description : Get the GPORT ID for the specified physical port.
 *
 * Parameters  : (IN)  unit      - BCM device number
 *               (IN)  port      - Port number
 *               (OUT) gport     - GPORT ID
 * Returns     : BCM_E_XXX
 */
int
bcm_fe2000_port_gport_get(int unit,
                          bcm_port_t port,
                          bcm_gport_t *gport)
{
    int                 rv;
    _bcm_fe2000_gport_dest_t   dest;

    PORT_INIT_CHECK(unit, port);

    sal_memset(&dest, 0, sizeof(_bcm_fe2000_gport_dest_t));

    rv = bcm_stk_my_modid_get(unit, &dest.modid);
    if (BCM_FAILURE(rv)) {
        return BCM_E_UNAVAIL;
    }

    dest.port = port;
    dest.gport_type = _SHR_GPORT_TYPE_MODPORT;

    return _bcm_fe2000_gport_construct(unit, &dest, gport);
}

/*
 * Function:
 *     bcm_port_phy_drv_name_get
 * Purpose:
 *     Return the name of the PHY driver being used on a port.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     name - (OUT) Buffer for PHY driver name
 *     len  - Length of buffer
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_port_phy_drv_name_get(int unit, bcm_port_t port,
                                 char *name, int len)
{
    int          rv = BCM_E_PORT;
    static char  drv_not_init[] = "driver not initialized";
    static char  invalid_port[] = "invalid port";

    /* Check params */
    PARAM_NULL_CHECK(name);

    UNIT_VALID_CHECK(unit);
    if (!UNIT_INIT_DONE(unit)) {
        if (sal_strlen(drv_not_init) < len) {
            sal_strcpy(name, drv_not_init);
        }
        return BCM_E_INIT;
    }

    if (!SOC_PORT_VALID(unit, port) || !INTERFACE_MAC(unit, port)) {
        if (sal_strlen(invalid_port) < len) {
            sal_strcpy(name, invalid_port);
        }
        return BCM_E_PORT;
    }

    PORT_LOCK(unit);
    rv = soc_phyctrl_drv_name_get(unit, port, name, len);
    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_phy_drv_name_get: unit=%d port=%d name=%s rv=%d\n",
               unit, port, name, rv));

    return rv;
}


/*
 * Function:
 *     bcm_port_phy_reset
 * Description:
 *     This function performs the low-level PHY reset and is intended to be
 *     called ONLY from callback function registered with
 *     bcm_port_phy_reset_register. Attempting to call it from any other
 *     place will break lots of things.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_port_phy_reset(int unit, bcm_port_t port)
{
    int  rv = BCM_E_PORT;

    /* Check params */
    PORT_INIT_CHECK(unit, port);
    PORT_INTERFACE_MAC_CHECK(unit, port);

    PORT_LOCK(unit);
    rv = soc_phy_reset(unit, port);
    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_phy_reset: unit=%d port=%d rv=%d\n",
               unit, port, rv));

    return rv;
}


/*
 * Function:
 *     bcm_port_phy_reset_register
 * Description:
 *     Register a callback function to be called whenever a PHY driver
 *     needs to perform a PHY reset.
 * Parameters:
 *     unit      - Device number
 *     port      - Device port number
 *     callback  - The callback function to call
 *     user_data - An opaque cookie to pass to callback function
 *                 whenever it is called
 * Returns:
 *     BCM_E_NONE      - Success
 *     BCM_E_PARAM     - Bad {unit, port} combination
 *     BCM_E_NOT_FOUND - The specified {unit, port, callback, user_data}
 *                       combination have not been registered before
 *     BCM_E_XXX       - Failure, other
 */
int
bcm_fe2000_port_phy_reset_register(int unit, bcm_port_t port,
                                   bcm_port_phy_reset_cb_t callback,
                                   void *user_data)
{
    int  rv = BCM_E_PORT;

    /* Check params */
    PORT_INIT_CHECK(unit, port);
    PORT_INTERFACE_MAC_CHECK(unit, port);

    PORT_LOCK(unit);
    rv =  soc_phy_reset_register(unit, port, callback, user_data, FALSE);
    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_phy_reset_register: unit=%d port=%d rv=%d\n",
               unit, port, rv));

    return rv;
}


/*
 * Function:
 *     bcm_port_phy_reset_unregister
 * Description:
 *     Unregister a callback function to be called whenever a PHY driver
 *     needs to perform a PHY reset.
 * Parameters:
 *     unit      - Device number
 *     port      - Device port number
 *     callback  - The callback function to call
 *     user_data - An opaque cookie to pass to callback function
 *                 whenever it is called
 * Returns:
 *     BCM_E_NONE      - Success
 *     BCM_E_PARAM     - Bad {unit, port} combination
 *     BCM_E_NOT_FOUND - The specified {unit, port, callback, user_data}
 *                       combination have not been registered before
 *     BCM_E_XXX       - Failure, other
 */
int
bcm_fe2000_port_phy_reset_unregister(int unit, bcm_port_t port,
                                     bcm_port_phy_reset_cb_t callback,
                                     void *user_data)
{
    int  rv = BCM_E_PORT;

    /* Check params */
    PORT_INIT_CHECK(unit, port);
    PORT_INTERFACE_MAC_CHECK(unit, port);

    PORT_LOCK(unit);
    rv = soc_phy_reset_unregister(unit, port, callback, user_data);
    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_phy_reset_unregister: unit=%d port=%d rv=%d\n",
               unit, port, rv));

    return rv;
}


/*
 * Function:
 *     bcm_port_speed_get
 * Purpose:
 *     Get the speed of the port.
 * Parameters:
 *     unit  - Device number
 *     port  - Device port number
 *     speed - (OUT) Value in megabits/sec (10, 100, etc)
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     If port is in MAC loopback, the speed of the loopback is returned.
 */
int
bcm_fe2000_port_speed_get(int unit, bcm_port_t port, int *speed)
{
    int  rv = BCM_E_NONE;
    int  mac_lb;

    /* Check params */
    PORT_INIT_CHECK(unit, port);
    PARAM_NULL_CHECK(speed);

    PORT_LOCK(unit);
    if (INTERFACE_MAC(unit, port)) {
        rv = MAC_LOOPBACK_GET(PORT(unit, port).p_mac, unit, port, &mac_lb);
        if (SOC_SUCCESS(rv)) {
            if (mac_lb || IS_HG_PORT(unit, port)) {
                rv = MAC_SPEED_GET(PORT(unit, port).p_mac, unit, port, speed);
            } else {
                rv = soc_phyctrl_speed_get(unit, port, speed);
                if (BCM_E_UNAVAIL == rv) {
                    /* PHY driver doesn't support speed_get. Get the speed from
                     * MAC.
                     */
                    rv = MAC_SPEED_GET(PORT(unit, port).p_mac, unit, port,
                                       speed);
                }
            }
        }

    } else if (INTERFACE_SPI(unit, port)) {
        *speed = 16000;

    } else {
        
        rv = BCM_E_PORT;
    }

    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_speed_get: unit=%d port=%d speed=%d rv=%d\n",
               unit, port, *speed, rv));

    return rv;
}


/*
 * Function:
 *     bcm_port_speed_max
 * Purpose:
 *     Get the maximum speed of the port.
 * Parameters:
 *     unit  - Device number
 *     port  - Device port number
 *     speed - (OUT) Value in megabits/sec (10, 100, etc)
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_port_speed_max(int unit, bcm_port_t port, int *speed)
{
    int              rv = BCM_E_PORT;
    bcm_port_abil_t  pm;

    /* Check params */
    PORT_INIT_CHECK(unit, port);
    PARAM_NULL_CHECK(speed);

    rv = bcm_port_ability_get(unit, port, &pm);

    if (BCM_SUCCESS(rv)) {
        if (pm & (BCM_PORT_ABIL_16GB_FD | BCM_PORT_ABIL_16GB_HD)) {
            *speed = 16000;
        } else if (pm & BCM_PORT_ABIL_13GB) {
            *speed = 13000;
        } else if (pm & BCM_PORT_ABIL_12GB) {
            *speed = 12000;
        } else if (pm & (BCM_PORT_ABIL_10GB_FD | BCM_PORT_ABIL_10GB_HD)) {
            if (IS_HG_PORT(unit, port) && SOC_INFO(unit).port_speed_max[port]) {
                *speed = SOC_INFO(unit).port_speed_max[port];
            } else {
                *speed = 10000;
            }
        } else if (pm & (BCM_PORT_ABIL_3000MB_FD | BCM_PORT_ABIL_3000MB_HD)) {
            *speed = 3000;
        } else if (pm & (BCM_PORT_ABIL_2500MB_FD | BCM_PORT_ABIL_2500MB_HD)) {
            *speed = 2500;
        } else if (pm & (BCM_PORT_ABIL_1000MB_FD | BCM_PORT_ABIL_1000MB_HD)) {
            *speed = 1000;
        } else if (pm & (BCM_PORT_ABIL_100MB_FD | BCM_PORT_ABIL_100MB_HD)) {
            *speed = 100;
        } else if (pm & (BCM_PORT_ABIL_10MB_FD | BCM_PORT_ABIL_10MB_HD)) {
            *speed = 10;
        }
    }

    PORT_VERB(("bcm_port_speed_max: unit=%d port=%d speed=%d rv=%d\n",
               unit, port, *speed, rv));

    return rv;
}


/*
 * Function:
 *      _bcm_fe2000_port_speed_set
 * Purpose:
 *      Main part of bcm_port_speed_set.
 * Notes:
 *     If port is in MAC loopback, only the MAC speed is set.
 */
STATIC int
_bcm_fe2000_port_speed_set(int unit, bcm_port_t port, int speed)
{
    int  rv = BCM_E_PORT;
    int  mac_lb, lb_phy, enable;

    if (INTERFACE_MAC(unit, port)) {
        /*
         * If port is in MAC loopback mode, do not try setting the PHY
         * speed.  This allows MAC loopback at 10/100 even if the PHY is
         * 1000 only.  Loopback diagnostic tests should enable loopback
         * before setting the speed, and vice versa when cleaning up.
         */
        SOC_IF_ERROR_RETURN(MAC_LOOPBACK_GET(PORT(unit, port).p_mac,
                                             unit, port, &mac_lb));

        if (speed == 0) {
            /* if speed is 0, set the port speed to max */
            SOC_IF_ERROR_RETURN
               (bcm_port_speed_max(unit, port, &speed));
        }

        if (!mac_lb) {
            SOC_IF_ERROR_RETURN(soc_phyctrl_auto_negotiate_set(unit, port,
                                                               FALSE));
            rv = soc_phyctrl_speed_set(unit, port, speed);
            if (SOC_FAILURE(rv)) {
                PORT_ERR(("bcm_port_speed_set: PHY_SPEED_SET failed "
                          "rv=%d(%s)\n",
                          rv, soc_errmsg(rv)));
                return rv;
            }
        }

        /* Prevent PHY register access while resetting BigMAC and Fusion core */
        if (IS_HG_PORT(unit, port)) {
            soc_phyctrl_enable_get(unit, port, &enable);
            soc_phyctrl_enable_set(unit, port, 0);
            soc_phyctrl_loopback_get(unit, port, &lb_phy);
        }

        rv = MAC_SPEED_SET(PORT(unit, port).p_mac, unit, port, speed);
        if (SOC_FAILURE(rv)) {
            PORT_ERR(("bcm_port_speed_set: MAC_SPEED_SET failed rv=%d(%s)\n",
                      rv, bcm_errmsg(rv)));
        }

        /* Restore PHY register access */
        if (IS_HG_PORT(unit, port)) {
            soc_phyctrl_enable_set(unit, port, enable);
            soc_phyctrl_loopback_set(unit, port, lb_phy);
        }
    } else if (INTERFACE_SPI(unit, port)) {
        if (speed == 16000) {
            rv = BCM_E_NONE;
        } else {
            rv = BCM_E_UNAVAIL;
        }
    }

    return rv;
}


/*
 * Function:
 *     bcm_port_speed_set
 * Purpose:
 *     Set the speed for a given port.
 * Parameters:
 *     unit  - Device number
 *     port  - Device port number
 *     speed - Value in megabits/sec (10, 100, etc)
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Turns off autonegotiation.  Caller must make sure other forced
 *     parameters (such as duplex) are set.
 */
int
bcm_fe2000_port_speed_set(int unit, bcm_port_t port, int speed)
{
    int     rv;
    pbmp_t  pbm;

    /* Check params */
    PORT_INIT_CHECK(unit, port);

    PORT_LOCK(unit);
    rv = _bcm_fe2000_port_speed_set(unit, port, speed);
    PORT_UNLOCK(unit);    /* Unlock before link call */

    if (INTERFACE_MAC(unit, port)) {
        if (BCM_SUCCESS(rv) && !SAL_BOOT_SIMULATION) {
            SOC_PBMP_CLEAR(pbm);
            SOC_PBMP_PORT_ADD(pbm, port);
            (void)bcm_link_change(unit, pbm);
        }
    }

    PORT_VERB(("bcm_port_speed_set: unit=%d port=%d speed=%d rv=%d\n",
               unit, port, speed, rv));

    return rv;
}


/*
 * Function:
 *     _bcm_fe2000_port_update
 * Purpose:
 *     Main internal routine to get port characteristics from
 *     PHY and program MAC to match.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     link - TRUE, process as link up
 *            FALSE, process as link down
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Assumes lock is held
 */
STATIC int
_bcm_fe2000_port_update(int unit, bcm_port_t port, int link)
{
    int  rv = BCM_E_NONE;

    if (INTERFACE_MAC(unit, port)) {
        int            duplex, speed, an, an_busy;
        soc_port_if_t  pif;

        SOC_IF_ERROR_RETURN
            (soc_phyctrl_auto_negotiate_get(unit, port, &an, &an_busy));

        if (!link) {
            /* PHY is down, disable MAC */

            SOC_IF_ERROR_RETURN
                (MAC_ENABLE_SET(PORT(unit, port).p_mac, unit, port, FALSE));

            /* PHY link down event */
            rv = soc_phyctrl_linkdn_evt(unit, port);
            if (BCM_FAILURE(rv) && (BCM_E_UNAVAIL != rv)) {
                return rv;
            }

            return BCM_E_NONE;
        }

        /* PHY link up event may not be support by all PHY driver.
         * Just ignore it if not supported */
        rv = soc_phyctrl_linkup_evt(unit, port);
        if (BCM_FAILURE(rv) && (BCM_E_UNAVAIL != rv)) {
            return rv;
        }

        /*
         * Set MAC speed first, since for GTH ports, this will switch
         * between the 1000Mb/s or 10/100Mb/s MACs.
         */

        if (!IS_HG_PORT(unit, port)) {
            rv = soc_phyctrl_speed_get(unit, port, &speed);
            if (BCM_FAILURE(rv) && (BCM_E_UNAVAIL != rv)) {
                return rv;
            }
            if (BCM_E_UNAVAIL == rv) {
                /* If PHY driver doesn't support speed_get, don't change
                 * MAC speed. E.g, Null PHY driver
                 */
                rv = BCM_E_NONE;
            } else {
                SOC_IF_ERROR_RETURN
                    (MAC_SPEED_SET(PORT(unit, port).p_mac, unit, port, speed));
            }

            SOC_IF_ERROR_RETURN
                (soc_phyctrl_duplex_get(unit, port, &duplex));
            SOC_IF_ERROR_RETURN
                (MAC_DUPLEX_SET(PORT(unit, port).p_mac, unit, port, duplex));
        } else {
            duplex = 1;
        }

        SOC_IF_ERROR_RETURN
            (soc_phyctrl_interface_get(unit, port, &pif));
        SOC_IF_ERROR_RETURN
            (MAC_INTERFACE_SET(PORT(unit, port).p_mac, unit, port, pif));

        /*
         * If autonegotiating, check the negotiated PAUSE values, and program
         * MACs accordingly.
         */

        if (an) {
            bcm_port_abil_t  r_advert, l_advert;
            int              tx_pause, rx_pause;

            SOC_IF_ERROR_RETURN
                (soc_phyctrl_adv_local_get(unit, port, &l_advert));
            SOC_IF_ERROR_RETURN
                (soc_phyctrl_adv_remote_get(unit, port, &r_advert));

            /*
             * IEEE 802.3 Flow Control Resolution.
             * Please see $SDK/doc/pause-resolution.txt for more information.
             */

            if (duplex) {
                tx_pause = (((r_advert & SOC_PM_PAUSE_RX) &&
                             (l_advert & SOC_PM_PAUSE_RX)) ||
                            ((r_advert & SOC_PM_PAUSE_RX) &&
                             !(r_advert & SOC_PM_PAUSE_TX) &&
                             (l_advert & SOC_PM_PAUSE_TX)));

                rx_pause = (((r_advert & SOC_PM_PAUSE_RX) &&
                             (l_advert & SOC_PM_PAUSE_RX)) ||
                            ((l_advert & SOC_PM_PAUSE_RX) &&
                             (r_advert & SOC_PM_PAUSE_TX) &&
                             !(l_advert & SOC_PM_PAUSE_TX)));
            } else {
                tx_pause = 0;
                rx_pause = 0;
            }

            SOC_IF_ERROR_RETURN
                (MAC_PAUSE_SET(PORT(unit, port).p_mac,
                               unit, port, tx_pause, rx_pause));
        }

        /* Enable MAC */
        SOC_IF_ERROR_RETURN
            (MAC_ENABLE_SET(PORT(unit, port).p_mac, unit, port, TRUE));

    } else if (INTERFACE_SPI(unit, port)) {
        /* TODO:  what is needed here? */

    } else {
        
    }

    return BCM_E_NONE;
}


/*
 * Function:
 *     bcm_port_update
 * Purpose:
 *     Get port characteristics from PHY and program MAC to match.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     link - TRUE, process as link up
 *            FALSE, process as link down
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_port_update(int unit, bcm_port_t port, int link)
{
    int  rv = BCM_E_NONE;

    /* Check params */
    PORT_INIT_CHECK(unit, port);

    PORT_LOCK(unit);
    rv = _bcm_fe2000_port_update(unit, port, link);
    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_update: unit=%d port=%d link=%d rv=%d\n",
               unit, port, link, rv));

    return rv;
}




/*
 * Function:
 *     _bcm_fe2000_port_lp_untagged_priority_access
 * Purpose:
 *     Sets/Gets priority being assigned to untagged receive packets for
 *  a logical port (vlan_port).
 * Parameters:
 *     (in)     unit     - Device number
 *     (in)     port     - Gport of type VLAN_PORT to access
 *     (in/out) priority -  Pointer to priority for return, or value to set
 *     (in)     set      - set or get boolean directive
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
_bcm_fe2000_port_lp_untagged_priority_access(int unit, bcm_port_t port,
                                             int *priority, int set)
{
    int              rv;

    UNIT_INIT_CHECK(unit);
    PORT_LOCK(unit);

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_port_lp_untagged_priority_access(unit, port,
                                                               priority, set);
        break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
        break;
    }

    PORT_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *     bcm_port_untagged_priority_set
 * Purpose:
 *     Set the 802.1P priority for untagged packets coming in on a
 *     port.  This value will be written into the priority field of the
 *     tag that is added at the ingress.
 * Parameters:
 *     unit     - Device number
 *     port     - Device port number
 *     priority - Priority to be set in 802.1p priority tag, from 0 to 7.
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_port_untagged_priority_set(int unit, bcm_port_t port, int priority)
{
    int rv = BCM_E_NONE;
#ifdef BCM_FE2000_P3_SUPPORT
    soc_sbx_g2p3_p2e_t p2e;
#endif

    if ((priority < 0) || (priority > 7)) {
        return BCM_E_PARAM;
    }

    if (BCM_GPORT_IS_VLAN_PORT(port)) {
        return _bcm_fe2000_port_lp_untagged_priority_access(unit, port,
                                                            &priority,
                                                            1 /* set*/);
    }

    /* Check params and get device handler */
    PORT_INIT_CHECK(unit, port);

    PORT_LOCK(unit);

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        soc_sbx_g2p3_p2e_t_init(&p2e);
        BCM_IF_ERROR_UNLOCK_RETURN(unit,
                             soc_sbx_g2p3_p2e_get(unit, port, &p2e));
        p2e.defpri = priority;
        BCM_IF_ERROR_UNLOCK_RETURN(unit,
                              soc_sbx_g2p3_p2e_set(unit, port, &p2e));
        break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
        break;
    }

    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_untagged_priority_set: unit=%d port=%d pri=%d\n",
               unit, port, priority));

    return rv;
}


/*
 * Function:
 *     bcm_port_untagged_priority_get
 * Purpose:
 *     Returns priority being assigned to untagged receive packets.
 * Parameters:
 *     unit     - Device number
 *     port     - Device port number
 *     priority - (OUT) Pointer to priority for return
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_port_untagged_priority_get(int unit, bcm_port_t port, int *priority)
{
    int rv = BCM_E_UNAVAIL;

    if (BCM_GPORT_IS_VLAN_PORT(port)) {
        return _bcm_fe2000_port_lp_untagged_priority_access(unit, port,
                                                            priority,
                                                            0 /* get */);
    }

    /* Check params  */
    PORT_INIT_CHECK(unit, port);

    PORT_LOCK(unit);
    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_port_untagged_priority_get(unit,
                                                         port,
                                                         priority);
        break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
        break;
    }

    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_untagged_priority_get: unit=%d port=%d pri=%d\n",
               unit, port, *priority));

    return rv;
}


/*
 * Function:
 *     _bcm_fe2000_port_lp_untagged_vlan_access
  * Purpose:
 *     Set the default VLAN ID for the logical port (vlan_port).
 *     This is the VLAN ID assigned to received untagged packets on the given
 *     logical port, physical port and ohter logical ports are not affected.
 * Parameters:
 *     unit  - Device number
 *     port  - Logical port (GPORT of type vlan_port)
 *     vid   - VLAN ID used for packets that ingress the port untagged
 *     set   - set or get flag
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
_bcm_fe2000_port_lp_untagged_vlan_access(int unit, bcm_port_t port,
                                         bcm_vlan_t *vid, int set)
{
    int              rv;

    UNIT_INIT_CHECK(unit);
    PORT_LOCK(unit);

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_port_lp_untagged_vlan_access(unit, port, vid,
                                                           set);
        break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
        break;
    }

    PORT_UNLOCK(unit);
    return rv;
}



/*
 * Function:
 *     bcm_port_untagged_vlan_action
 * Purpose:
 *     Set the default VLAN ID for the port.
 *     This is the VLAN ID assigned to received untagged packets.
 * Parameters:
 *     unit  - Device number
 *     port  - Device port number
 *     vid   - VLAN ID used for packets that ingress the port untagged
 *     action - detailed action to perform on vid
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_port_untagged_vlan_action(int unit,
                                  bcm_port_t port,
                                     bcm_vlan_t vid,
                                      _bcm_fe2k_nvid_action_t action) 
{

    int                       result, flags, isMplsPort;
    int                       drp, keepUntaggedFlags;
    bcm_vlan_t                vlan;

    /*  In the normal case, packets remain untagged */
    keepUntaggedFlags = 0;
    isMplsPort = 0;

    if (BCM_GPORT_IS_VLAN_PORT(port)) {
        return _bcm_fe2000_port_lp_untagged_vlan_access(unit, port, &vid,
                                                        1 /* set */);
    }

    if (BCM_GPORT_IS_MPLS_PORT(port)) {        
        PORT_ERR(("%s[%d]: Feature no longer supported on mpls gports, use"
                  " (physical_port, vid) for the mpls gport instead.\n",
                  FUNCTION_NAME(), unit));
        return BCM_E_UNAVAIL;
    }

    /* Check params and get device handler */
    PORT_INIT_CHECK(unit, port);

    /* allow vid == 0xfff */
    if (vid > BCM_VLAN_MAX) {
        return BCM_E_PARAM;
    }


#if BCM_FE2000_P3_SUPPORT

    if (SOC_IS_SBX_G2P3(unit)) {
        soc_sbx_g2p3_pv2e_t pv2e;
  
        result = SOC_SBX_G2P3_PV2E_GET(unit, port, vid, &pv2e);
        if (BCM_FAILURE(result)) {
            PORT_ERR(("%s[%d]: Error=%d (%s) failed to get port/vid info "
                      " port=%d vid=0x%x\n",
                      FUNCTION_NAME(), unit, result, bcm_errmsg(result),
                      port, vid));
            return result;
        }

        /* Try vswitch to scan known VSIs for gport matches */
        result = _bcm_fe2000_vswitch_port_info_get(unit, port, pv2e.vlan, 
                                                   &keepUntaggedFlags);

        /* nothing found in vswitch, try searching the VPNs for an mpls_port */
        if (BCM_FAILURE(result)) {
            result = _bcm_fe2000_mpls_port_info_get(unit, pv2e.vlan, pv2e.vpws,
                                                    &keepUntaggedFlags);
            if (keepUntaggedFlags) {
                /* signal this is an mpls port */
                isMplsPort = 1;
            }
        }

        result = BCM_E_NONE;
    }

#endif /* BCM_FE2000_P3_SUPPORT */


    /*
     *  In all cases, VLAN must be aware of the untagged VID for a port, as it
     *  has to mirror writes so that priority tagged frames (and untagged
     *  frames and frames whose tags are ignored) show up on the proper vid,
     *  and with the proper STP and similar state.  Also, VLAN needs to make
     *  adjustments to its internal tables when configuring to drop tagged or
     *  drop untagged frames, so this allows for such.
     *
     *  In the traditional bridging case we never set any of the flags.
     */
    flags = 0;

    if (keepUntaggedFlags) {
        flags = (BCM_FE2K_NVID_OVERRIDE_PVV2E |
                 BCM_FE2K_NVID_USE_PVV2E |
                 BCM_FE2K_NVID_SET_UNTAGGEDSTRIP);

        /* set keep or strip only on vlan ports, not mpls ports */
        if (isMplsPort == 0) { 
            flags |= BCM_FE2K_NVID_SET_KEEPORSTRIP;
        }
    }

    result = _bcm_fe2000_vlan_port_native_vlan_set(unit, port, vid, flags,
                                                   action);

    if (BCM_E_NONE == result) {
        /* See if the port is in drop tagged mode */
        result = _bcm_fe2000_vlan_port_tagged_ingress_drop_get(unit,
                                                               port,
                                                               &vlan,
                                                               &drp);
        /* NOTE: the returned VLAN here should be equal to the VID above */
    }

    if (BCM_E_NONE == result) {
        if (!drp) {
            /* not dropping tagged frames; change port2etc */
            PORT_LOCK(unit);
            
            switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
            case SOC_SBX_UCODE_TYPE_G2P3:
                /*
                 *  The g2p3 model is such that p2e.nativeVid is not used and
                 *  should always be left zero.  See bcm_port_untagged_vlan_get
                 *  for one example of retrieving the native VID; there are
                 *  other possible paths (see the vlan module (search for
                 *  'nvid') for the other paths).
                 *
                 *  The reason that p2e.nativeVid is zero is so we have the
                 *  same path for untagged (or frames whose tags are ignored)
                 *  as we have for pritagged frames, so the port filtering
                 *  functions work as expected.
                 *
                 *  The way that untagged/pritagged/ignored-tag frames work is
                 *  the same as the way that pritagged frames work in g2p2 --
                 *  the functions in vlan that manage writes to pv2e/pVid2Etc
                 *  (_sbG2FePVid2EtcSet, _soc_sbx_g2p3_pv2e_set) will always
                 *  mirror any writes to the native VID to vid 0 for the port,
                 *  thus keeping STP and VSI and LogicalPort and other data in
                 *  sync between them, except they also compensate for the port
                 *  membership filters (see bcm_port_vlan_member_set).
                 *
                 *  If, in the odd case that you actually require the port have
                 *  a native VID for which it is NOT explitly a member (not
                 *  explicitly added to that VID by the VLAN code and that VID
                 *  is not the default VLAN), the port MUST have filtering
                 *  disabled. If you don't do this, the port will drop all
                 *  frames, as VLAN will think that it's not a member of the
                 *  VID and route frames to the drop VSI (this happens when
                 *  updating the pv2e/pVid2Etc using the above calls).
                 */
                break;
#endif /* BCM_FE2000_P3_SUPPORT */                
            default:
                SBX_UNKNOWN_UCODE_WARN(unit);
                result = BCM_E_INTERNAL;
            }

            PORT_UNLOCK(unit);
        }
    }

    PORT_VERB(("bcm_port_untagged_vlan_set: unit=%d port=%d vid=%d -> %d (%s)\n",
               unit,
               port,
               vid,
               result,
               _SHR_ERRMSG(result)));

    return result;
}

/*
 * Function:
 *     bcm_port_untagged_vlan_set
 * Purpose:
 *     Set the default VLAN ID for the port.
 *     This is the VLAN ID assigned to received untagged packets.
 * Parameters:
 *     unit  - Device number
 *     port  - Device port number
 *     vid   - VLAN ID used for packets that ingress the port untagged
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_port_untagged_vlan_set(int unit,
                                  bcm_port_t port,
                                  bcm_vlan_t vid)
{
    return bcm_fe2000_port_untagged_vlan_action(unit, port, vid, 
                                                BCM_FE2K_NVID_ACTION_SET);
}


/*
 * Function:
 *     bcm_port_untagged_vlan_touch
 * Purpose:
 *     Set the default VLAN ID for the port.
 *     This is the VLAN ID assigned to received untagged packets, with no change to
 *     logical port divergence
 * Parameters:
 *     unit  - Device number
 *     port  - Device port number
 *     vid   - VLAN ID used for packets that ingress the port untagged
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
_bcm_fe2000_port_untagged_vlan_touch(int unit, 
                                     bcm_port_t port,
                                     bcm_vlan_t vid)
{
    return bcm_fe2000_port_untagged_vlan_action(unit, port, vid, 
                                                BCM_FE2K_NVID_ACTION_TOUCH);
}


/*
 * Function:
 *     bcm_port_untagged_vlan_get
 * Purpose:
 *     Retrieve the default VLAN ID for the port.
 *     This is the VLAN ID assigned to received untagged packets.
 * Parameters:
 *     unit    - Device number
 *     port    - Device port number
 *     vid_ptr - (OUT) Pointer to VLAN ID for return
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_port_untagged_vlan_get(int unit, bcm_port_t port,
                                  bcm_vlan_t *vid_ptr)
{
    int                       result = BCM_E_NONE;

    /* Check params */
    if (vid_ptr == NULL) {
        return BCM_E_PARAM;
    }

    if (BCM_GPORT_IS_VLAN_PORT(port)) {
        return _bcm_fe2000_port_lp_untagged_vlan_access(unit, port, vid_ptr,
                                                        0 /*get*/);
    }

    PORT_INIT_CHECK(unit, port);

    /* Get untagged VID from VLAN, since we always tell VLAN now */
    result = _bcm_fe2000_vlan_port_native_vlan_get(unit,
                                                   port,
                                                   vid_ptr);

    PORT_VERB(("bcm_port_untagged_vlan_get: unit=%d port=%d vid=%d\n",
               unit, port, *vid_ptr));

    return result;
}


/*
 * Function:
 *     bcm_port_dtag_mode_get
 * Description:
 *     Return the current double-tagging mode of a port.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     mode - (OUT) Double-tagging mode
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_port_dtag_mode_get(int unit, bcm_port_t port, int *mode)
{
    int                 rv = BCM_E_NONE;


    /* Check params and get device handler */
    PORT_INIT_CHECK(unit, port);
    PARAM_NULL_CHECK(mode);

    PORT_LOCK(unit);

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_port_dtag_mode_get(unit, port, mode);
        break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
        break;
    }

    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_dtag_mode_get: unit=%d port=%d mode=%d\n",
               unit, port, *mode));

    return rv;
}


/*
 * Function:
 *     bcm_port_dtag_mode_set
 * Description:
 *     Set the double-tagging mode of a port.
 * Parameters:
 *     unit - Device number
 *     port - Port number
 *     mode - Double-tagging mode, one of:
 *            BCM_PORT_DTAG_MODE_NONE            No double tagging
 *            BCM_PORT_DTAG_MODE_TRANSPARENT     No tags parsed
 *            BCM_PORT_DTAG_MODE_INTERNAL        Service Provider port
 *            BCM_PORT_DTAG_MODE_EXTERNAL        Customer port
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_port_dtag_mode_set(int unit, bcm_port_t port, int mode)
{
    int                 rv = BCM_E_NONE;

    /* Check params and get device handler */
    PORT_INIT_CHECK(unit, port);

    if ((mode != BCM_PORT_DTAG_MODE_NONE) &&
        (mode != BCM_PORT_DTAG_MODE_TRANSPARENT) &&
        (mode != BCM_PORT_DTAG_MODE_INTERNAL) &&
        (mode != BCM_PORT_DTAG_MODE_EXTERNAL)) {
        return BCM_E_PARAM;
    }

    PORT_LOCK(unit);

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_port_dtag_mode_set(unit,
                                                 port,
                                                 mode);
        break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
        break;
    }

    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_dtag_mode_set: unit=%d port=%d mode=%d\n",
               unit, port, mode));

    return rv;
}


/*
 * Function:
 *     bcm_port_tpid_set
 * Description:
 *     Set the default Tag Protocol ID for a port.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     tpid - Tag Protocol ID
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     This API is not specifically double-tagging-related, but
 *     the port TPID becomes the service provider TPID when double-tagging
 *     is enabled on a port.
 *
 *     By default all ports have a default TPID 0x8100.
 */
int
bcm_fe2000_port_tpid_set(int unit, bcm_port_t port, uint16 tpid)
{
    int  rv = BCM_E_UNAVAIL;

    PORT_INIT_CHECK(unit, port);

    PORT_LOCK(unit);

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv =_bcm_fe2000_g2p3_port_tpid_set(unit, port, tpid);
        break;
#endif /* BCM_FE2000_P3_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
    } 

    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_tpid_set: unit=%d port=%d tpid=0x%x rv=%d\n",
               unit, port, tpid, rv));

    return rv;
}


/*
 * Function:
 *     bcm_port_tpid_get
 * Description:
 *     Retrieve the default Tag Protocol ID for a port.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     tpid - (OUT) Tag Protocol ID
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_port_tpid_get(int unit, bcm_port_t port, uint16 *tpid)
{
    int  rv = BCM_E_UNAVAIL;

    /* Check params and get device handler */
    PORT_INIT_CHECK(unit, port);
    if (tpid == NULL) {
        return BCM_E_PARAM;
    }

    PORT_LOCK(unit);

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv =_bcm_fe2000_g2p3_port_tpid_get(unit, port, tpid);
        break;
#endif /* BCM_FE2000_P3_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
    }

    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_tpid_get: unit=%d port=%d tpid=%d rv=%d\n",
               unit, port, *tpid, rv));

    return rv;
}


/*
 * Function:
 *     bcm_port_inner_tpid_set
 * Purpose:
 *     Set the expected TPID for the inner tag in double-tagging mode.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     tpid - Tag Protocol ID
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 *
 * Notes:
 *     This is only valid when port is in PROVIDER mode (dtag_mode = INTERNAL).
 *     Inner tag must be set after outer tag is set with 'bcm_port_tpid_set()'
 */
int
bcm_fe2000_port_inner_tpid_set(int unit, bcm_port_t port, uint16 tpid)
{
    int rv;

    /* Check params and get device handler */
    PORT_INIT_CHECK(unit, port);

    PORT_LOCK(unit);

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv =_bcm_fe2000_g2p3_port_inner_tpid_set(unit, port, tpid);
        break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
        break;
    }

    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_inner_tpid_set: unit=%d port=%d tpid=%d rv=%d\n",
               unit, port, tpid, rv));

    return rv;
}


/*
 * Function:
 *     bcm_port_inner_tpid_get
 * Purpose:
 *     Get the expected TPID for the inner tag in double-tagging mode.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     tpid - (OUT) Tag Protocol ID
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_port_inner_tpid_get(int unit, bcm_port_t port, uint16 *tpid)
{
    int rv;

    PORT_INIT_CHECK(unit, port);
    if (tpid == NULL) {
        return BCM_E_PARAM;
    }

    PORT_LOCK(unit);

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv =_bcm_fe2000_g2p3_port_inner_tpid_get(unit, port, tpid);
        break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
        break;
    }

    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_inner_tpid_get: unit=%d port=%d tpid=%d rv=%d\n",
               unit, port, *tpid, rv));

    return rv;
}


/*
 * Function:
 *     bcm_port_tpid_add
 * Purpose:
 *     Add allowed TPID for a port.
 * Parameters:
 *     unit         - Device number
 *     port         - Device port number
 *     tpid         - Tag Protocol ID
 *     color_select - Color mode for TPID, ignored for this device
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 *
 * Notes:
 *     Parameter 'color_select' is ignored in this device.
 */
int
bcm_fe2000_port_tpid_add(int unit, bcm_port_t port,
                         uint16 tpid, int color_select)
{
    int rv;

    PORT_INIT_CHECK(unit, port);

    PORT_LOCK(unit);

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_port_tpid_add(unit, port, tpid, color_select);
        break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
        break;
    }

    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_tpid_add: unit=%d port=%d tpid=%d rv=%d\n",
               unit, port, tpid, rv));

    return rv;
}


/*
 * Function:
 *     bcm_port_tpid_delete
 * Purpose:
 *     Delete allowed TPID for a port.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 *     tpid - Tag Protocol ID
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_port_tpid_delete(int unit, bcm_port_t port, uint16 tpid)
{
    int rv;

    PORT_INIT_CHECK(unit, port);

    PORT_LOCK(unit);

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv =_bcm_fe2000_g2p3_port_tpid_delete(unit, port, tpid);
        break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
        break;
    }

    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_tpid_delete: unit=%d port=%d tpid=%d rv=%d\n",
               unit, port, tpid, rv));

    return rv;
}


/*
 * Function:
 *     bcm_port_tpid_delete_all
 * Purpose:
 *     Delete all allowed TPID for a port.
 * Parameters:
 *     unit - Device number
 *     port - Device port number
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_port_tpid_delete_all(int unit, bcm_port_t port)
{
    int rv;

    PORT_INIT_CHECK(unit, port);

    PORT_LOCK(unit);

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv =_bcm_fe2000_g2p3_port_tpid_delete_all(unit, port);
        break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
        break;
    }

    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_tpid_delete_all: unit=%d port=%d rv=%d\n",
               unit, port, rv));

    return rv;
}


int
bcm_fe2000_port_bpdu_enable_get(int unit, bcm_port_t port, int *enable)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_bpdu_enable_set(int unit, bcm_port_t port, int enable)
{
    return BCM_E_UNAVAIL;
}


/*
 * The following APIs are currently being implemented.
 */

int
bcm_fe2000_port_cfi_color_get(int unit, bcm_port_t port,
                              int cfi, bcm_color_t *color)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_cfi_color_set(int unit, bcm_port_t port,
                              int cfi, bcm_color_t color)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_class_get(int unit, bcm_port_t port,
                          bcm_port_class_t class, uint32 *class_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_class_set(int unit, bcm_port_t port,
                          bcm_port_class_t class, uint32 class_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_control_get(int unit, bcm_port_t port,
                            bcm_port_control_t type, int *value)
{
    int status = BCM_E_NONE;
    int nSpiPort;
#ifdef BCM_FE2000_P3_SUPPORT
    soc_sbx_g2p3_p2e_t p2e;
    soc_sbx_g2p3_lp_t p3lp;
    bcm_module_t tgtModule;
    bcm_module_t myModId = ~0;
    uint32 logicalPort;
    bcm_port_t physicalPort;
#endif

    PORT_VERB(("bcm_fe2000_port_control_get: unit=%d, type=%d\n",
               unit, type));

    UNIT_INIT_CHECK(unit);

    PORT_LOCK(unit);

    if (BCM_GPORT_IS_SET(port)) {
        switch (type) {
        case bcmPortControlFabricKnockoutId:
            switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
            case SOC_SBX_UCODE_TYPE_G2P3:
                soc_sbx_g2p3_lp_t_init(&p3lp);
                logicalPort = ~0;
                physicalPort = -1;
                
                if (BCM_GPORT_IS_LOCAL(port)) {
                    /* no logical port for this case */
                    physicalPort = BCM_GPORT_LOCAL_GET(port);
                } else if (BCM_GPORT_IS_MODPORT(port)) {
                    /* make sure it's not remote module */
                    status = bcm_stk_my_modid_get(unit, &myModId);
                    if (BCM_E_NONE == status) {
                        tgtModule = BCM_GPORT_MODPORT_MODID_GET(port);
                        if (tgtModule == myModId) {
                            /* it's local & physical; get physical port */
                            physicalPort = BCM_GPORT_MODPORT_PORT_GET(port);
                        }
                    }
                } else if (BCM_GPORT_IS_VLAN_PORT(port)) {
                    /* get VLAN GPORT logical & physical port IDs */
                    status = bcm_fe2000_vlan_port_get_lpid(unit,
                                                           port,
                                                           &logicalPort,
                                                           &physicalPort);
                } else if (BCM_GPORT_IS_MPLS_PORT(port)) {
                    /* get MPLS GPORT logical & physical port IDs */
                    status = bcm_fe2000_g2p3_mpls_port_get_lpid(unit,
                                                                port,
                                                                &logicalPort,
                                                                &physicalPort);
                } else if (BCM_GPORT_IS_MIM_PORT(port)) {
                    /* get MiM GPORT logical & physical port IDs */
                    status = bcm_fe2000_mim_port_get_lpid(unit,
                                                          port,
                                                          &logicalPort,
                                                          &physicalPort);
                } else {
                    /* don't support other types yet */
                    status = BCM_E_UNAVAIL;
                }
                /* 'physical' port likely MODPORT or similar; condition it */
                if ((BCM_E_NONE == status) &&
                    (-1 != physicalPort) &&
                    (BCM_GPORT_IS_SET(physicalPort))) {
                    /* make sure 'physicalPort' is a local port or invalid */
                    if (BCM_GPORT_IS_LOCAL(physicalPort)) {
                        /* it's a 'local' port; assume it's local */
                        physicalPort = BCM_GPORT_LOCAL_GET(physicalPort);
                    } else if (BCM_GPORT_IS_MODPORT(physicalPort)) {
                        /* it's a modport; make sure it's local */
                        if (BCM_GPORT_MODPORT_MODID_GET(physicalPort) == myModId) {
                            physicalPort = BCM_GPORT_MODPORT_PORT_GET(physicalPort);
                        } else {
                            physicalPort = -1;
                        }
                    } else {
                        /* otherwise don't know what to do with it; nonlocal */
                        physicalPort = -1;
                    }
                }
                /* fetch the appropriate logical port descriptor */
                if (BCM_E_NONE == status) {
                    /* we know logical and physical port ID */
                    if (~0 != logicalPort) {
                        /* get logical port's info */
                        status = soc_sbx_g2p3_lp_get(unit, logicalPort, &p3lp);
                    }
                    if ((BCM_E_NONE == status) &&
                        ((~0 == logicalPort) || (0 == p3lp.pid))) {
                        /* no logical port or no PID in logical port */
                        if ((0 <= physicalPort) && (SBX_MAX_PORTS > physicalPort)) {
                            /* local, so get physical */
                            status = soc_sbx_g2p3_lp_get(unit, physicalPort, &p3lp);
                        } else {
                            /* not local, so can't look this up */
                            status = BCM_E_PARAM;
                        }
                    }
                }
                /* fill in the outbound argument */
                if (BCM_E_NONE == status) {
                    /* we have a logicalport descriptor at this point */
                    if (p3lp.usecolor) {
                        /* MPLS (et al?) split flood domain */
                        *value = p3lp.color;
                    } else {
                        /* Normal operation */
                        *value = p3lp.pid;
                    }
                }
                /* Special case for trunks */
                if (BCM_GPORT_IS_TRUNK(port)) {
                    /*
                     *  TRUNK GPORTs would fall through with BCM_E_UNAVAIL, so
                     *  nothing happens until now.  Here, we replace that with
                     *  a direct map between trunk ID and PID.
                     */
                    /* NOTE: use of 'logicalPort' here is only as a register */
                    logicalPort = SOC_SBX_TRUNK_FTE(unit, BCM_GPORT_TRUNK_GET(port));
                    if (SBX_INVALID_FTE != logicalPort) {
                        *value = logicalPort;
                        status = BCM_E_NONE;
                    } else{
                        status = BCM_E_NOT_FOUND;
                    }
                }
                break;
#endif /* def BCM_FE2000_P3_SUPPORT */
            default:
                PORT_WARN(("uCode type %d is not supported\n",
                           SOC_SBX_CONTROL(unit)->ucodetype));
                status = BCM_E_INTERNAL;
            }
            break;
        default:
            PORT_WARN(("port control %d not supported on unit %d gport %08X\n",
                       type,
                       unit,
                       port));
            status = BCM_E_UNAVAIL;
        }
    } else if ((0 <= port) && (SBX_MAX_PORTS > port)) {
        if ( (INTERFACE_SPI(unit, port)) && (IS_SPI_BUS_PORT(unit, port)) ) {
            nSpiPort = PORT_SPI_BUS(unit, port);
            switch (type) {
                case bcmPortControlTrain:
                    status = BCM_E_UNAVAIL;
                    break;

                case bcmPortControlRxEnable:
                    status = BCM_E_UNAVAIL;
                    break;

                case bcmPortControlTxEnable:
                    status = BCM_E_UNAVAIL;
                    break;

                case bcmPortControlRxLink:
                    (*value) = (hwFe2000Spi4RxStatus((sbhandle)unit, nSpiPort) == 0) ? TRUE : FALSE;
                    break;

                case bcmPortControlTxLink:
                    (*value) = (hwFe2000Spi4TxStatus((sbhandle)unit, nSpiPort) == 0) ? TRUE : FALSE;
                    break;

                default:
                    status = BCM_E_UNAVAIL;
                    break;
            }
        }
        else {
            switch (type) {
            case bcmPortControlMpls:
                switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
                case SOC_SBX_UCODE_TYPE_G2P3:
                    /* nothing to do */
                    status = BCM_E_NONE;
                    break;
#endif
                default:
                    PORT_WARN(("uCode type %d is not supported\n",
                               SOC_SBX_CONTROL(unit)->ucodetype));
                    status = BCM_E_INTERNAL;
                    break;
                }
                break;
            case bcmPortControlOamLoopback:
                switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
                case SOC_SBX_UCODE_TYPE_G2P3:
                    soc_sbx_g2p3_p2e_t_init(&p2e);
                    BCM_IF_ERROR_UNLOCK_RETURN(unit,
                                         soc_sbx_g2p3_p2e_get(unit, port, &p2e));
                    *value = p2e.oamloop;
                    break;
#endif
                default:
                    PORT_WARN(("uCode type %d is not supported\n",
                               SOC_SBX_CONTROL(unit)->ucodetype));
                    status = BCM_E_INTERNAL;
                    break;
                }
                break;
            case bcmPortControlIP4Mcast:
                switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
                case SOC_SBX_UCODE_TYPE_G2P3:
                    soc_sbx_g2p3_p2e_t_init(&p2e);
                    BCM_IF_ERROR_UNLOCK_RETURN(unit,
                                         soc_sbx_g2p3_p2e_get(unit, port, &p2e));

                    *value = p2e.ipv4mc;
                    break;
#endif
                default:
                    PORT_WARN(("uCode type %d is not supported\n",
                               SOC_SBX_CONTROL(unit)->ucodetype));
                    status = BCM_E_INTERNAL;
                    break;
                }
                break;
            case bcmPortControlFabricKnockoutId:
                switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
                case SOC_SBX_UCODE_TYPE_G2P3:
                    soc_sbx_g2p3_lp_t_init(&p3lp);
                    status = soc_sbx_g2p3_lp_get(unit, port, &p3lp);
                    if (BCM_E_NONE == status) {
                        *value = p3lp.pid;
                    }
                    break;
#endif /* def BCM_FE2000_P3_SUPPORT */
                default:
                    PORT_WARN(("uCode type %d is not supported\n",
                               SOC_SBX_CONTROL(unit)->ucodetype));
                    status = BCM_E_INTERNAL;
                    break;
                }
                break;
            default:
                PORT_VERB(("bcm_fe2000_port_control_get: unit=%d, unsuppd type=%d\n",
                       unit, type));
                status = BCM_E_UNAVAIL;
            }
        }
    } else {
        status = BCM_E_PORT;
    }

    PORT_UNLOCK(unit);

    return status;
}

int
bcm_fe2000_port_control_set(int unit, bcm_port_t port,
                            bcm_port_control_t type, int value)
{
    int status = BCM_E_NONE;
    int nSpiPort;
#ifdef BCM_FE2000_P3_SUPPORT
    soc_sbx_g2p3_p2e_t  p2e;
    soc_sbx_g2p3_ep2e_t ep2e;
#endif

    PORT_VERB(("bcm_fe2000_port_control_set: unit=%d, type=%d, value=0x%x\n",
               unit, type, value));

    PORT_INIT_CHECK(unit, port);

    PORT_LOCK(unit);

    if ( (INTERFACE_SPI(unit, port)) && (IS_SPI_BUS_PORT(unit, port)) ) {
        nSpiPort = PORT_SPI_BUS(unit, port);
        switch (type) {
            case bcmPortControlTrain:
                if ( (value != TRUE) && (value != FALSE)) {
                    status = BCM_E_PARAM;
                }
                else {
                    (value == TRUE) ? hwFe2000Spi4TxForceTrainingOn((sbhandle)unit, nSpiPort) :
                                      hwFe2000Spi4TxForceTrainingOff((sbhandle)unit, nSpiPort);
                }
                break;

            case bcmPortControlRxEnable:
                if ( (value != TRUE) && (value != FALSE)) {
                    status = BCM_E_PARAM;
                }
                else {
                    hwFe2000Spi4RxEnable((sbhandle)unit, value, nSpiPort);
                }
                break;

            case bcmPortControlTxEnable:
                if ( (value != TRUE) && (value != FALSE)) {
                    status = BCM_E_PARAM;
                }
                else {
                    hwFe2000Spi4TxEnable((sbhandle)unit, value, nSpiPort);
                }
                break;

            case bcmPortControlRxLink:
                status = BCM_E_UNAVAIL;
                break;

            case bcmPortControlTxLink:
                status = BCM_E_UNAVAIL;
                break;

            default:
                status = BCM_E_UNAVAIL;
                break;
        }
    }
    else {
        switch (type) {
        case bcmPortControlOamLoopback:
            switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
            case SOC_SBX_UCODE_TYPE_G2P3:

                /* ingress */
                BCM_IF_ERROR_UNLOCK_RETURN(unit,
                                     soc_sbx_g2p3_p2e_get(unit, port, &p2e));

                p2e.oamloop = !!value;
                BCM_IF_ERROR_UNLOCK_RETURN(unit,
                                     soc_sbx_g2p3_p2e_set(unit, port, &p2e));


                /* egress */
                BCM_IF_ERROR_UNLOCK_RETURN(unit,
                                     soc_sbx_g2p3_ep2e_get(unit, port, &ep2e));

                ep2e.oamloop = !!value;
                BCM_IF_ERROR_UNLOCK_RETURN(unit,
                                     soc_sbx_g2p3_ep2e_set(unit, port, &ep2e));

                break;
#endif
            default:
                PORT_WARN(("uCode type %d is not supported\n",
                           SOC_SBX_CONTROL(unit)->ucodetype));
                status = BCM_E_INTERNAL;
                break;
            }
            break;

        case bcmPortControlIP4Mcast:
            switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
            case SOC_SBX_UCODE_TYPE_G2P3:
                soc_sbx_g2p3_p2e_t_init(&p2e);
                BCM_IF_ERROR_UNLOCK_RETURN(unit,
                                     soc_sbx_g2p3_p2e_get(unit, port, &p2e));

                if(value) {
                    p2e.ipv4mc = 1;
                } else {
                    p2e.ipv4mc = 0;
                }

                BCM_IF_ERROR_UNLOCK_RETURN(unit,
                                     soc_sbx_g2p3_p2e_set(unit, port, &p2e));
                break;
#endif
            default:
                PORT_WARN(("uCode type %d is not supported\n",
                           SOC_SBX_CONTROL(unit)->ucodetype));
                status = BCM_E_INTERNAL;
                break;
            }
            break;  

        case bcmPortControlMpls:      
            switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
            case SOC_SBX_UCODE_TYPE_G2P3:
                soc_sbx_g2p3_p2e_t_init(&p2e);
                BCM_IF_ERROR_UNLOCK_RETURN(unit,
                                     soc_sbx_g2p3_p2e_get(unit, port, &p2e));

                if(value) {
                    p2e.mplstp = 1;
                } else {
                    p2e.mplstp = 0;
                }

                BCM_IF_ERROR_UNLOCK_RETURN(unit,
                                     soc_sbx_g2p3_p2e_set(unit, port, &p2e));
                break;
#endif
            default:
                PORT_WARN(("uCode type %d is not supported\n",
                           SOC_SBX_CONTROL(unit)->ucodetype));
                status = BCM_E_INTERNAL;
                break;
            }
      
            break;

        default:
            PORT_VERB(("bcm_fe2000_port_control_set: unit=%d, unsuppd type=%d\n",
                   unit, type));
            status = BCM_E_UNAVAIL;
        }
    }

    PORT_UNLOCK(unit);

    return status;
}

/*
 *   Function
 *      _bcm_fe2000_port_lp_discard_action
 *   Purpose
 *      Get/Set the discard state of a logical port (vlan_port)
 *   Parameters
 *      (in)  unit    - BCM device number
 *      (in)  port    - GPORT_VLAN_PORT to access discard settings
 *      (in/out) discardTagged   - drop/discard tagged packet setting
 *      (in/out) discardUnagged  - drop/discard untagged packet setting
 *      (in)  set     - set/get action
 */
int
_bcm_fe2000_port_lp_discard_action(int unit, bcm_port_t port,
                                   int *discardTagged, int *discardUntagged,
                                   int set)
{

    int              rv;
#if 0 
    bcm_vlan_port_t *vlanPort;
    int              lpIdx;

    lpIdx = BCM_GPORT_VLAN_PORT_ID_GET(port);
    vlanPort = SBX_LPORT_DATAPTR(unit, lpIdx);

    if (vlanPort == NULL ||
        SBX_LPORT_TYPE(unit, lpIdx) != BCM_GPORT_VLAN_PORT) {
        PORT_ERR(("%s[%d]: invalid vlan_port state found: port=0x%x\n",
                  FUNCTION_NAME(), unit, port));
        return BCM_E_PARAM;
    }

    PORT_INIT_CHECK(unit, vlanPort->port);
#else 

    UNIT_INIT_CHECK(unit);
#endif 
    PORT_LOCK(unit);

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_port_lp_discard_access(unit, port,
                                                     discardTagged,
                                                     discardUntagged,
                                                     set);
        break;
#endif
    default:
        PORT_WARN(("uCode type %d is not supported\n",
                   SOC_SBX_CONTROL(unit)->ucodetype));
        rv = BCM_E_INTERNAL;
        break;
    }

    PORT_UNLOCK(unit);
    return rv;
}



/*
 *   Function
 *      _bcm_fe2000_port_discard_untag_access
 *   Purpose
 *      Get or set the state of untagged frame discard for a port
 *   Parameters
 *      (in) int unit = unit number whose VLAN suport is to be initialised
 *      (in) bcm_port_t port = port number whose untag discard is to be read
 *      (in) int write = TRUE to indicate setting instead of reading
 *      (in/out) int *mode = where to get/put this port's untag discard state
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 */
static int
_bcm_fe2000_port_discard_untag_access(int unit,
                                      bcm_port_t port,
                                      int write,
                                      int *mode)
{
    int                     result;             /* result to caller */

    /* hope for the best */
    result = BCM_E_NONE;

    /* VLAN does all this in the new model */
    if (write) {
        result = _bcm_fe2000_vlan_port_untagged_ingress_drop_set(unit,
                                                                 port,
                                                                 *mode);
    } else {
        result = _bcm_fe2000_vlan_port_untagged_ingress_drop_get(unit,
                                                                 port,
                                                                 mode);
    }

    return result;
}

/*
 *   Function
 *      bcm_fe2000_port_discard_get
 *   Purpose
 *      Get the discard mode for this port
 *   Parameters
 *      (in) int unit = unit number whose VLAN suport is to be initialised
 *      (in) bcm_port_t port = port number whose mode is to be read
 *      (out) int *mode = where to put this port's discard mode
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Currently only supports BCM_PORT_DISCARD_NONE and
 *      BCM_PORT_DISCARD_UNTAG.  Other support to come soon, but will be
 *      implemented by the VLAN module, which needs to know of the others.
 */
int
bcm_fe2000_port_discard_get(int unit, bcm_port_t port, int *mode)
{
    int                     result;
    int                     untagged;
    int                     tagged;
    bcm_vlan_t              vlan;

    if (!mode) {
        /* null pointer for mode */
        PORT_ERR(("%s[%d]: null pointer for mode\n", FUNCTION_NAME(), unit));
        return BCM_E_PARAM;
    }

    if (BCM_GPORT_IS_VLAN_PORT(port)) {
        result = _bcm_fe2000_port_lp_discard_action(unit, port,
                                                    &tagged, &untagged,
                                                    0 /* get */);
    } else {

        /* get untagged drop setting for this port */
        result = _bcm_fe2000_port_discard_untag_access(unit,
                                                       port,
                                                       FALSE,
                                                       &untagged);

        /* get tagged drop setting for this port */
        if (BCM_E_NONE == result) {
            result = _bcm_fe2000_vlan_port_tagged_ingress_drop_get(unit,
                                                                   port,
                                                                   &vlan,
                                                                   &tagged);
        }
    }

    /* build returned mode */
    if (BCM_E_NONE == result) {
        if (tagged && untagged) {
            *mode = BCM_PORT_DISCARD_ALL;
        } else if (tagged) {
            *mode = BCM_PORT_DISCARD_TAG;
        } else if (untagged) {
            *mode = BCM_PORT_DISCARD_UNTAG;
        } else {
            *mode = BCM_PORT_DISCARD_NONE;
        }
    }

    return result;
}

/*
 *   Function
 *      bcm_fe2000_port_discard_set
 *   Purpose
 *      Set the discard mode for this port
 *   Parameters
 *      (in) int unit = unit number whose VLAN suport is to be initialised
 *      (in) bcm_port_t port = port number whose mode is to be read
 *      (in) int mode = this port's new discard mode
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      This function will take a long time to run, in part because it has a
 *      lot to do in the VLAN module, but contributing to this is the fact that
 *      it needs to have one of two locks at various points but only ever holds
 *      one (to avoid deadlock possibilities).
 */
int
bcm_fe2000_port_discard_set(int unit, bcm_port_t port, int mode)
{
    int                     result;
    int                     untagged;
    int                     tagged;
    bcm_vlan_t              vlan;

    /* make sure mode is valid and decode settings */
    switch (mode) {
    case BCM_PORT_DISCARD_NONE:
        tagged = FALSE;
        untagged = FALSE;
        break;
    case BCM_PORT_DISCARD_UNTAG:
        tagged = FALSE;
        untagged = TRUE;
        break;
    case BCM_PORT_DISCARD_TAG:
        tagged = TRUE;
        untagged = FALSE;
        break;
    case BCM_PORT_DISCARD_ALL:
        tagged = TRUE;
        untagged = TRUE;
        break;
    default:
        /* not a valide mode */
        PORT_ERR(("%s[%d]: invalid port discard mode %d\n",
                  FUNCTION_NAME(),
                  unit,
                  mode));
        return BCM_E_PARAM;
    }

    /* vlan ports are handled explictly */
    if (BCM_GPORT_IS_VLAN_PORT(port)) {
        return _bcm_fe2000_port_lp_discard_action(unit, port,
                                                  &tagged, &untagged,
                                                  1 /* set */);
    }

    /* set untagged drop mode */
    result = _bcm_fe2000_port_discard_untag_access(unit,
                                                   port,
                                                   TRUE,
                                                   &untagged);

    /* get current native untagged VID */
    if (BCM_E_NONE == result) {
        /* get current native VID for the port */
        result = bcm_fe2000_port_untagged_vlan_get(unit, port, &vlan);
        if (BCM_E_NONE != result) {
            PORT_ERR(("%s[%d]: unable to get port %d untagged VLAN: %d (%s)\n",
                      FUNCTION_NAME(),
                      unit,
                      port,
                      result,
                      _SHR_ERRMSG(result)));
        }
    }

    /* set ingress drop tagged mode */
    if (BCM_E_NONE == result) {
        result = _bcm_fe2000_vlan_port_tagged_ingress_drop_set(unit,
                                                               port,
                                                               vlan,
                                                               tagged);
        if (BCM_E_NONE != result) {
            PORT_ERR(("%s[%d]: unable to set port %d tagged ingress drop:"
                      " %d (%s)\n",
                      FUNCTION_NAME(),
                      unit,
                      port,
                      result,
                      _SHR_ERRMSG(result)));
        }
    }

#ifdef BCM_FE2000_P3_SUPPORT
    if (SOC_IS_SBX_G2P3(unit) && BCM_SUCCESS(result)) {
        result = _bcm_fe2000_g2p3_mpls_port_discard_set(unit, port, mode);
        if (result == BCM_E_NOT_FOUND) {
            result = BCM_E_NONE;
        }
    }
#endif


    /* done */
    return result;
}


int
bcm_fe2000_port_dscp_map_get(int unit, bcm_port_t port,
                             int srccp, int *mapcp, int *prio)
{

    int rv;

    PORT_INIT_CHECK(unit, port);
    PORT_LOCK(unit);

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_port_dscp_map_get(unit, port, srccp, mapcp, prio);
        break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
        break;
    }

    PORT_UNLOCK(unit);
    return rv;
}


int
bcm_fe2000_port_dscp_map_set(int unit, bcm_port_t port,
                             int srccp, int mapcp, int prio)
{
    int rv;

    PORT_INIT_CHECK(unit, port);
    PORT_LOCK(unit);

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv =  _bcm_fe2000_g2p3_port_dscp_map_set(unit, port, srccp, mapcp, prio);
        break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
        break;
    }

    PORT_UNLOCK(unit);
    return rv;
}

int
bcm_fe2000_port_dscp_unmap_get(int unit, bcm_port_t port,
                                    int internal_pri, bcm_color_t color,
                                    int *pkt_dscp)
{
    int rv;

    PORT_INIT_CHECK(unit, port);
    PORT_LOCK(unit);
    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_port_dscp_unmap_get(unit, port,
                                                  internal_pri, 
                                                  color, pkt_dscp);
    break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
        break;
   }
    PORT_UNLOCK(unit);
    return rv;

}

int
bcm_fe2000_port_dscp_unmap_set(int unit, bcm_port_t port,
                               int internal_pri, bcm_color_t color,
                               int pkt_dscp)
{
    int rv;

    PORT_INIT_CHECK(unit, port);
    PORT_LOCK(unit);
    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_port_dscp_unmap_set(unit, port, 
                                                  internal_pri, 
                                                  color, pkt_dscp);
    break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
        break;
    }

    PORT_UNLOCK(unit);
    return rv;
}

int
bcm_fe2000_port_dscp_map_mode_get(int unit, bcm_port_t port, int *mode)
{
    
    int rv;
    PORT_INIT_CHECK(unit, port);
    PORT_LOCK(unit);
    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_port_dscp_map_mode_get(unit, port, mode);
    break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
        break;
    }

    PORT_UNLOCK(unit);
    return rv;

}

int
bcm_fe2000_port_dscp_map_mode_set(int unit, bcm_port_t port, int mode)
{
    int rv;
    PORT_INIT_CHECK(unit, port);
    PORT_LOCK(unit);
    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_port_dscp_map_mode_set(unit, port, mode);
    break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
        break;
    }

    PORT_UNLOCK(unit);
    return rv;

}


int
bcm_fe2000_port_egress_get(int unit, bcm_port_t port,
                           int modid, bcm_pbmp_t *pbmp)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_egress_set(int unit, bcm_port_t port,
                           int modid, bcm_pbmp_t pbmp)
{
    return BCM_E_UNAVAIL;
}


int
bcm_fe2000_port_fault_get(int unit, bcm_port_t port, uint32 *flags)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_flood_block_set(int unit, bcm_port_t ingress_port,
                                bcm_port_t egress_port, uint32 flags)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_flood_block_get(int unit, bcm_port_t ingress_port,
                                bcm_port_t egress_port, uint32 *flags)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_force_forward_get(int unit, bcm_port_t port,
                                  bcm_port_t *egr_port, int *enabled)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_force_forward_set(int unit, bcm_port_t port,
                                  bcm_port_t egr_port, int enable)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_force_vlan_get(int unit, bcm_port_t port,
                               bcm_vlan_t *vlan, int *pkt_prio, uint32 *flags)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_force_vlan_set(int unit, bcm_port_t port,
                               bcm_vlan_t vlan, int pkt_prio, uint32 flags)
{
    return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_fe2000_port_ifilter_get
 *   Purpose
 *      Get the settings for ingress filtering of frames tagged for a
 *      particular VLAN on the specified port, when the port is not a member of
 *      the VLAN for which the frame is tagged.
 *   Parameters
 *      (in) int unit = unit number whose VID is to have the ports list fetched
 *      (in) bcm_port_t port = port whose filtering is to be checked
 *      (out) int* mode = where to put the filtering mode
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 */
int
bcm_fe2000_port_ifilter_get(int unit, bcm_port_t port, int *mode)
{
    int    result;
    int    ingrFilter;
    int    egrFilter;

    if (mode) {
        /* result pointer good; get ingress filtering state for the port */
        result = _bcm_fe2000_vlan_port_filter_get(unit,
                                                  port,
                                                  &ingrFilter,
                                                  &egrFilter);
        if (ingrFilter) {
            *mode = BCM_PORT_IFILTER_ON;
        } else {
            *mode = BCM_PORT_IFILTER_OFF;
        }
    } else {
        /* result pointer bad; parameter error */
        PORT_ERR(("%s: NULL pointer for mode\n", FUNCTION_NAME()));
        result = BCM_E_PARAM;
    }
    return result;
}

/*
 *   Function
 *      bcm_fe2000_port_ifilter_set
 *   Purpose
 *      Set the settings for ingress/egress filtering of frames tagged for a
 *      particular VLAN on the specified port, when the port is not a member of
 *      the VLAN for which the frame is tagged.
 *   Parameters
 *      (in) int unit = unit number whose VID is to have the ports list fetched
 *      (in) bcm_port_t port = port whose filtering is to be checked
 *      (out) int mode = the filtering mode
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 */
int
bcm_fe2000_port_ifilter_set(int unit, bcm_port_t port, int mode)
{
    /* set the ingress VLAN filtering for this port */
    return _bcm_fe2000_vlan_port_ingress_filter_set(unit,
                                                    port,
                                                    (BCM_PORT_IFILTER_ON == mode));
}

int
bcm_fe2000_port_ipmc_modify_get(int unit, bcm_port_t port, uint32 *flags)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_ipmc_modify_set(int unit, bcm_port_t port, uint32 flags)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_jam_set(int unit, bcm_port_t port, int enable)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_jam_get(int unit, bcm_port_t port, int *enable)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_l3_enable_get(int unit, bcm_port_t port, int *enable)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_l3_enable_set(int unit, bcm_port_t port, int enable)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_l3_modify_get(int unit, bcm_port_t port, uint32 *flags)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_l3_modify_set(int unit, bcm_port_t port, uint32 flags)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_l3_mtu_get(int unit, bcm_port_t port, int *size)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_l3_mtu_set(int unit, bcm_port_t port, int size)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_learn_get(int unit, bcm_port_t port, uint32 *flags)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_learn_modify(int unit, bcm_port_t port,
                             uint32 add, uint32 remove)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_learn_set(int unit, bcm_port_t port, uint32 flags)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *     bcm_port_loopback_get
 * Purpose:
 *     Recover the current loopback operation for the specified port.
 * Parameters:
 *     unit     - Device number
 *     port     - Device port number
 *     loopback - (OUT) one of:
 *                    BCM_PORT_LOOPBACK_NONE
 *                    BCM_PORT_LOOPBACK_MAC
 *                    BCM_PORT_LOOPBACK_PHY
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_port_loopback_get(int unit, bcm_port_t port, int *loopback)
{
    int  rv = BCM_E_NONE;
    int  phy_lb = 0;
    int  mac_lb = 0;

    /* Check params */
    PORT_INIT_CHECK(unit, port);
    PARAM_NULL_CHECK(loopback);

    PORT_LOCK(unit);

    if (INTERFACE_MAC(unit, port)) {

        rv = soc_phyctrl_loopback_get(unit, port, &phy_lb);

        if (SOC_SUCCESS(rv)) {
            rv = MAC_LOOPBACK_GET(PORT(unit, port).p_mac, unit, port, &mac_lb);
        }
        if (SOC_SUCCESS(rv)) {
            if (mac_lb) {
                *loopback = BCM_PORT_LOOPBACK_MAC;
            } else if (phy_lb) {
                *loopback = BCM_PORT_LOOPBACK_PHY;
            } else {
                *loopback = BCM_PORT_LOOPBACK_NONE;
            }
        }

    } else {
        *loopback = BCM_PORT_LOOPBACK_NONE;
    }

    PORT_UNLOCK(unit);

    PORT_VERB(("bcm_port_loopback_get: unit=%d port=%d lb=%d rv=%d\n",
               unit, port, *loopback, rv));

    return rv;
}


/*
 * Function:
 *     bcm_port_loopback_set
 * Purpose:
 *     Set the loopback operation for the specified port.
 * Parameters:
 *     unit     - Device number
 *     port     - Device port number
 *     loopback - One of:
 *                    BCM_PORT_LOOPBACK_NONE
 *                    BCM_PORT_LOOPBACK_MAC
 *                    BCM_PORT_LOOPBACK_PHY
 *                    BCM_PORT_LOOPBACK_CLAUSE57
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 */
int
bcm_fe2000_port_loopback_set(int unit, bcm_port_t port, int loopback)
{
    int  rv = BCM_E_NONE;

    /* Check params */
    PORT_INIT_CHECK(unit, port);
    if ((loopback != BCM_PORT_LOOPBACK_NONE) &&
        (loopback != BCM_PORT_LOOPBACK_MAC) &&
        (loopback != BCM_PORT_LOOPBACK_PHY)) {
        return BCM_E_PARAM;
    }

    /* Check port interface type */
    if (!INTERFACE_MAC(unit, port)) {
        if (loopback != BCM_PORT_LOOPBACK_NONE) {
            return BCM_E_PARAM;
        }
        return BCM_E_NONE;
    }


    /*
     * Always force link before changing hardware to avoid
     * race with the linkscan thread.
     */
    if (!(loopback == BCM_PORT_LOOPBACK_NONE)) {
        rv = _bcm_link_force(unit, port, TRUE, FALSE);
    }

    PORT_LOCK(unit);
    if (BCM_SUCCESS(rv)) {
        rv = MAC_LOOPBACK_SET(PORT(unit, port).p_mac, unit, port,
                              (loopback == BCM_PORT_LOOPBACK_MAC));
    }
    if (BCM_SUCCESS(rv)) {
        rv = soc_phyctrl_loopback_set(unit, port,
                                      (loopback == BCM_PORT_LOOPBACK_PHY));
    }
    PORT_UNLOCK(unit);

    if ((loopback == BCM_PORT_LOOPBACK_NONE) || !BCM_SUCCESS(rv)) {
        _bcm_link_force(unit, port, FALSE, DONT_CARE);

    } else {
        /* Enable only MAC instead of calling bcm_port_enable_set so
         * that this API doesn't silently enable the port if the
         * port is disabled by application.
         */
        rv = MAC_ENABLE_SET(PORT(unit, port).p_mac, unit, port, TRUE);

        if (BCM_SUCCESS(rv)) {
            /* Make sure that the link status is updated only after the
             * MAC is enabled so that link_mask2 is set before the
             * calling thread synchronizes with linkscan thread in
             * _bcm_link_force call.
             * If the link is forced before MAC is enabled, there could
             * be a race condition in _soc_link_update where linkscan
             * may use an old view of link_mask2 and override the
             * EPC_LINK_BMAP after the mac_enable_set updates
             * link_mask2 and EPC_LINK_BMAP.
             */
            rv = _bcm_link_force(unit, port, TRUE, TRUE);
        }
    }

    PORT_VERB(("bcm_port_loopback_set: unit=%d port=%d lb=%d rv=%d\n",
               unit, port, loopback, rv));

    return rv;
}

/*
 * Function:
 *      bcm_port_phy_get
 * Description:
 *      General PHY register read
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      flags - Logical OR of one or more of the following flags:
 *              BCM_PORT_PHY_INTERNAL
 *                      Address internal SERDES PHY for port
 *              BCM_PORT_PHY_NOMAP
 *                      Instead of mapping port to PHY MDIO address,
 *                      treat port parameter as actual PHY MDIO address.
 *              BCM_PORT_PHY_CLAUSE45
 *                      Assume Clause 45 device instead of Clause 22
 *      phy_addr - PHY internal register address
 *      phy_data - (OUT) Data that was read
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_fe2000_port_phy_get(int unit, bcm_port_t port, uint32 flags,
                 uint32 phy_reg_addr, uint32 *phy_data)
{
    uint8  phy_id;
    uint8  phy_devad;
    uint16 phy_reg;
    uint16 phy_rd_data;
    uint32 reg_flag;
    int    rv;

    /* Check params */
    PORT_INIT_CHECK(unit, port);
    PARAM_NULL_CHECK(phy_data);
    PORT_INTERFACE_MAC_CHECK(unit, port);

    rv       = BCM_E_UNAVAIL;
    reg_flag = BCM_PORT_PHY_REG_FLAGS(phy_reg_addr);
    if (reg_flag & SOC_PHY_REG_INDIRECT) {
        if (flags & BCM_PORT_PHY_NOMAP) {
            /* Indirect register access is performed through PHY driver.
             * Therefore, indirect register access is not supported if
             * BCM_PORT_PHY_NOMAP flag is set.
             */
            return BCM_E_PARAM;
        }
        phy_reg_addr &= ~SOC_PHY_REG_INDIRECT;
        PORT_LOCK(unit);
        rv = soc_phyctrl_reg_read(unit, port, flags, phy_reg_addr, phy_data);
        PORT_UNLOCK(unit);
    }

    if (rv == BCM_E_UNAVAIL) {
        if (flags & BCM_PORT_PHY_NOMAP) {
            phy_id = port;
        } else if (flags & BCM_PORT_PHY_INTERNAL) {
            phy_id = PORT_TO_PHY_ADDR_INT(unit, port);
        } else {
            phy_id = PORT_TO_PHY_ADDR(unit, port);
        }

        PORT_LOCK(unit);
        if (flags & BCM_PORT_PHY_CLAUSE45) {
            phy_devad = BCM_PORT_PHY_CLAUSE45_DEVAD(phy_reg_addr);
            phy_reg   = BCM_PORT_PHY_CLAUSE45_REGAD(phy_reg_addr);
            rv = soc_miimc45_read(unit, phy_id, phy_devad,
                                  phy_reg, &phy_rd_data);

        } else {
            phy_reg = phy_reg_addr;
            rv = soc_miim_read(unit, phy_id, phy_reg, &phy_rd_data);
        }
        PORT_UNLOCK(unit);

        if (BCM_SUCCESS(rv)) {
           *phy_data = phy_rd_data;
        }
    }

    SOC_DEBUG_PRINT((DK_PORT,
                     "bcm_fe2000_port_phy_get: u=%d p=%d flags=0x%08x "
                     "phy_reg=0x%08x, phy_data=0x%08x, rv=%d\n",
                     unit, port, flags, phy_reg_addr, *phy_data, rv));

    return rv;
}

/*
 * Function:
 *      bcm_port_phy_set
 * Description:
 *      General PHY register write
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      flags - Logical OR of one or more of the following flags:
 *              BCM_PORT_PHY_INTERNAL
 *                      Address internal SERDES PHY for port
 *              BCM_PORT_PHY_NOMAP
 *                      Instead of mapping port to PHY MDIO address,
 *                      treat port parameter as actual PHY MDIO address.
 *              BCM_PORT_PHY_CLAUSE45
 *                      Assume Clause 45 device instead of Clause 22
 *      phy_addr - PHY internal register address
 *      phy_data - Data to write
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_fe2000_port_phy_set(int unit, bcm_port_t port, uint32 flags,
                 uint32 phy_reg_addr, uint32 phy_data)
{
    uint8  phy_id;
    uint8  phy_devad;
    uint16 phy_reg;
    uint16 phy_wr_data;
    uint32 reg_flag;
    int    rv;

    /* Check params */
    PORT_INIT_CHECK(unit, port);
    PORT_INTERFACE_MAC_CHECK(unit, port);


    SOC_DEBUG_PRINT((DK_PORT,
                     "bcm_fe2000_port_phy_set: u=%d p=%d flags=0x%08x "
                     "phy_reg=0x%08x phy_data=0x%08x\n",
                     unit, port, flags, phy_reg_addr, phy_data));

    rv       = BCM_E_UNAVAIL;
    reg_flag = BCM_PORT_PHY_REG_FLAGS(phy_reg_addr);
    if (reg_flag & SOC_PHY_REG_INDIRECT) {
        if (flags & BCM_PORT_PHY_NOMAP) {
            /* Indirect register access is performed through PHY driver.
             * Therefore, indirect register access is not supported if
             * BCM_PORT_PHY_NOMAP flag is set.
             */
            return BCM_E_PARAM;
        }
        phy_reg_addr &= ~SOC_PHY_REG_INDIRECT;
        PORT_LOCK(unit);
        rv = soc_phyctrl_reg_write(unit, port, flags, phy_reg_addr, phy_data);
        PORT_UNLOCK(unit);
    }

    if (rv == BCM_E_UNAVAIL) {
        if (flags & BCM_PORT_PHY_NOMAP) {
            phy_id = port;
        } else if (flags & BCM_PORT_PHY_INTERNAL) {
            phy_id = PORT_TO_PHY_ADDR_INT(unit, port);
        } else {
            phy_id = PORT_TO_PHY_ADDR(unit, port);
        }

        phy_wr_data = (uint16) (phy_data & 0xffff);
        PORT_LOCK(unit);
        if (flags & BCM_PORT_PHY_CLAUSE45) {
            phy_devad = BCM_PORT_PHY_CLAUSE45_DEVAD(phy_reg_addr);
            phy_reg   = BCM_PORT_PHY_CLAUSE45_REGAD(phy_reg_addr);
            rv = soc_miimc45_write(unit, phy_id, phy_devad,
                                   phy_reg, phy_wr_data);
        } else {
            phy_reg = phy_reg_addr;
            rv = soc_miim_write(unit, phy_id, phy_reg, phy_wr_data);
        }
        PORT_UNLOCK(unit);
    }
    return rv;
}

/*
 * Function:
 *      bcm_port_phy_modify
 * Description:
 *      General PHY register modify
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      flags - Logical OR of one or more of the following flags:
 *              BCM_PORT_PHY_INTERNAL
 *                      Address internal SERDES PHY for port
 *              BCM_PORT_PHY_NOMAP
 *                      Instead of mapping port to PHY MDIO address,
 *                      treat port parameter as actual PHY MDIO address.
 *              BCM_PORT_PHY_CLAUSE45
 *                      Assume Clause 45 device instead of Clause 22
 *      phy_addr - PHY internal register address
 *      phy_data - Data to write
 *      phy_mask - Bits to modify using phy_data
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_fe2000_port_phy_modify(int unit, bcm_port_t port, uint32 flags,
                           uint32 phy_reg_addr, uint32 phy_data, uint32 phy_mask)
{
    uint8  phy_id;
    uint8  phy_devad;
    uint16 phy_reg;
    uint16 phy_rd_data;
    uint16 phy_wr_data;
    uint32 reg_flag;
    int    rv;

    /* Check params */
    PORT_INIT_CHECK(unit, port);
    PORT_INTERFACE_MAC_CHECK(unit, port);

    SOC_DEBUG_PRINT((DK_PORT,
                     "bcm_fe2000_port_phy_modify: u=%d p=%d flags=0x%08x "
                     "phy_reg=0x%08x phy_data=0x%08x phy_mask=0x%08x\n",
                     unit, port, flags, phy_reg_addr, phy_data, phy_mask));

    rv       = BCM_E_UNAVAIL;
    reg_flag = BCM_PORT_PHY_REG_FLAGS(phy_reg_addr);
    if (reg_flag & SOC_PHY_REG_INDIRECT) {
        if (flags & BCM_PORT_PHY_NOMAP) {
            /* Indirect register access is performed through PHY driver.
             * Therefore, indirect register access is not supported if
             * BCM_PORT_PHY_NOMAP flag is set.
             */
            return BCM_E_PARAM;
        }
        phy_reg_addr &= ~SOC_PHY_REG_INDIRECT;
        PORT_LOCK(unit);
        rv = soc_phyctrl_reg_modify(unit, port, flags, phy_reg_addr,
                                    phy_data, phy_mask);
        PORT_UNLOCK(unit);
    }

    if (rv == BCM_E_UNAVAIL) {
        if (flags & BCM_PORT_PHY_NOMAP) {
            phy_id = port;
        } else if (flags & BCM_PORT_PHY_INTERNAL) {
            phy_id = PORT_TO_PHY_ADDR_INT(unit, port);
        } else {
            phy_id = PORT_TO_PHY_ADDR(unit, port);
        }

        phy_wr_data = (uint16) (phy_data & phy_mask & 0xffff);
        PORT_LOCK(unit);
        if (flags & BCM_PORT_PHY_CLAUSE45) {
            phy_devad = BCM_PORT_PHY_CLAUSE45_DEVAD(phy_reg_addr);
            phy_reg   = BCM_PORT_PHY_CLAUSE45_REGAD(phy_reg_addr);
            rv = soc_miimc45_read(unit, phy_id, phy_devad, 
                                  phy_reg, &phy_rd_data);
            phy_wr_data |= (phy_rd_data & ~phy_mask);
            rv = soc_miimc45_write(unit, phy_id, phy_devad, 
                                   phy_reg, phy_wr_data);
        } else {
            phy_reg = phy_reg_addr;
            rv = soc_miim_read(unit, phy_id, phy_reg, &phy_rd_data);
            if (BCM_SUCCESS(rv)) {
                phy_wr_data |= (phy_rd_data & ~phy_mask);
                rv = soc_miim_write(unit, phy_id, phy_reg, phy_wr_data);
            }
        }
        PORT_UNLOCK(unit);
    }
    return rv;
}

int
bcm_fe2000_port_modid_egress_get(int unit, bcm_port_t port,
                                 bcm_module_t modid, bcm_pbmp_t *pbmp)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_modid_egress_set(int unit, bcm_port_t port,
                                 bcm_module_t modid, bcm_pbmp_t pbmp)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_modid_enable_get(int unit, bcm_port_t port,
                                 int modid, int *enable)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_modid_enable_set(int unit, bcm_port_t port,
                                 int modid, int enable)
{
    return BCM_E_UNAVAIL;
}


int
bcm_fe2000_port_pfm_get(int unit, bcm_port_t port, int *mode)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_pfm_set(int unit, bcm_port_t port, int mode)
{
    return BCM_E_UNAVAIL;
}


int
bcm_fe2000_port_protocol_vlan_add(int unit, bcm_port_t port,
                                  bcm_port_frametype_t frame,
                                  bcm_port_ethertype_t ether,
                                  bcm_vlan_t vid)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_protocol_vlan_delete(int unit, bcm_port_t port,
                                     bcm_port_frametype_t frame,
                                     bcm_port_ethertype_t ether)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_protocol_vlan_delete_all(int unit, bcm_port_t port)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_queued_count_get(int unit, bcm_port_t port, uint32 *count)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_rate_egress_get(int unit, bcm_port_t port,
                                uint32 *kbits_sec, uint32 *kbits_burst)
{
    sbFe2000Queues_t *queues;    
    uint32_t qid, connection;
    uint32_t hpp_to_mac_thresh, spi_to_hpp_min_pages;
    uint32_t hppFreq, epoch, contexts;

    PORT_INIT_CHECK(unit, port);

    if (!kbits_sec || !kbits_burst) {
        return BCM_E_PARAM;
    }

    if (SOC_IS_SBX_FE2KXT(unit)) {
        queues = &SOC_SBX_CFG_FE2000(unit)->xt_init_params.queues;
    } else {
        queues = &SOC_SBX_CFG_FE2000(unit)->init_params.queues;
    }

    qid        = queues->port2eqid[port];
    connection = queues->qid2con[qid];

    /*  *->HPP  */
    qid = queues->fromqid[connection];
    spi_to_hpp_min_pages = queues->minfrompages[qid];

    /*  HPP->*  */
    qid = queues->toqid[connection];

    if (SOC_IS_SBX_FE2KXT(unit)) {    
        sbZfC2QmQueueConfigEntry_t qc;
        C2QmQueueConfigRead((sbhandle) unit, qid, &qc);
        hpp_to_mac_thresh = qc.m_uFlowControlThresh1;
        hppFreq  = SOC_SBX_CFG_FE2000(unit)->xt_init_params.uHppFreq / 1000;
        epoch    = SOC_SBX_CFG_FE2000(unit)->xt_init_params.uEpoch;
        contexts = 16;
    } else {
        sbZfFe2000QmQueueConfigEntry_t qc;
        CaQmQueueConfigRead((sbhandle) unit, qid, &qc);
        hpp_to_mac_thresh = qc.m_uFlowControlThresh1;
        hppFreq  = 250;
        epoch    = 404;
        contexts = 12;
    }

    /* For slower speed, the configured  rate will be significanly higher than
     * what may have been _set.  This is due to the rounding feature of _set
     * and size of a single sws page
     */
    sbFe2000SwsConfigGet(hppFreq, epoch, contexts, hpp_to_mac_thresh,
                         spi_to_hpp_min_pages, kbits_burst, kbits_sec);
        
    return BCM_E_NONE;
}

int
bcm_fe2000_port_rate_egress_set(int unit, bcm_port_t port,
                                uint32 kbits_sec, uint32 kbits_burst)
{
    int rv;
    sbFe2000QmParams_t mac_to_hpp, hpp_to_spi, spi_to_hpp, hpp_to_mac;
    sbFe2000Queues_t *queues;    
    sbFe2000Connection_t *connection;
    uint32_t qid, conn_idx;
    uint32_t hppFreq, epoch, contexts, frame_max, frame_size;
    bcm_module_t modid;
    int spi_bus, fab_unit, node, fab_port, higig=0;

    PORT_INIT_CHECK(unit, port);

    
    if (SOC_IS_SBX_FE2KXT(unit)) {
        queues   = &SOC_SBX_CFG_FE2000(unit)->xt_init_params.queues;
        hppFreq  = SOC_SBX_CFG_FE2000(unit)->xt_init_params.uHppFreq / 1000;
        epoch    = SOC_SBX_CFG_FE2000(unit)->xt_init_params.uEpoch;
        contexts = 16;
    } else {
        queues   = &SOC_SBX_CFG_FE2000(unit)->init_params.queues;
        hppFreq  = 250;
        epoch    = 404;
        contexts = 12;
    }

    frame_max = kbits_burst;
    qid        = queues->port2eqid[port];
    conn_idx   = queues->qid2con[qid];
    connection = &queues->connections[conn_idx];
    if (connection->bEgress &&
       SOC_SBX_CFG_FE2000_IF_IS_HIGIG(unit, connection->from.ulInterfacePortType)) {
       higig = 1;
    }

    sbFe2000SwsConfigCompute(hppFreq, epoch, contexts, frame_max, kbits_sec, higig,
                             &mac_to_hpp, &hpp_to_spi, 
                             &spi_to_hpp, &hpp_to_mac);

    /*  Configure *->HPP queue */
    qid = queues->fromqid[conn_idx];

    queues->fromflowthresh[qid] = spi_to_hpp.threshold;
    queues->minfrompages[qid]   = spi_to_hpp.min_pages;
    queues->maxfrompages[qid]   = spi_to_hpp.max_pages;

    if (SOC_IS_SBX_FE2KXT(unit)) {    
        sbZfC2QmQueueConfigEntry_t qc;
        C2QmQueueConfigRead((sbhandle) unit, qid, &qc);
        qc.m_uFlowControlThresh1 = queues->fromflowthresh[qid];
        qc.m_uFlowControlThresh2 = queues->fromflowthresh[qid];
        qc.m_uFlowControlEnable = !!queues->fromflowthresh[qid];
        qc.m_uMinPages = queues->minfrompages[qid];
        qc.m_uMaxPages = queues->maxfrompages[qid];
        C2QmQueueConfigWrite((sbhandle) unit, qid, &qc);
    } else {
        sbZfFe2000QmQueueConfigEntry_t qc;
        CaQmQueueConfigRead((sbhandle) unit, qid, &qc);
        qc.m_uFlowControlThresh1 = queues->fromflowthresh[qid];
        qc.m_uFlowControlThresh2 = queues->fromflowthresh[qid];
        qc.m_uFlowControlEnable = !!queues->fromflowthresh[qid];
        qc.m_uMinPages = queues->minfrompages[qid];
        qc.m_uMaxPages = queues->maxfrompages[qid];
        CaQmQueueConfigWrite((sbhandle) unit, qid, &qc);
    }

    /*  Configure HPP->* queue */
    qid = queues->toqid[conn_idx];

    if (SOC_IS_SBX_FE2KXT(unit)) {    
        sbZfC2QmQueueConfigEntry_t qc;
        C2QmQueueConfigRead((sbhandle) unit, qid, &qc);
        qc.m_uFlowControlThresh1 =  hpp_to_mac.threshold;
        qc.m_uFlowControlThresh2 =  hpp_to_mac.threshold;
        qc.m_uFlowControlEnable = !! hpp_to_mac.threshold;
        qc.m_uMinPages = hpp_to_mac.min_pages;
        qc.m_uMaxPages = hpp_to_mac.max_pages;
        C2QmQueueConfigWrite((sbhandle) unit, qid, &qc);
    } else {
        sbZfFe2000QmQueueConfigEntry_t qc;
        CaQmQueueConfigRead((sbhandle) unit, qid, &qc);
        qc.m_uFlowControlThresh1 =  hpp_to_mac.threshold;
        qc.m_uFlowControlThresh2 =  hpp_to_mac.threshold;
        qc.m_uFlowControlEnable = !! hpp_to_mac.threshold;
        qc.m_uMinPages = hpp_to_mac.min_pages;
        qc.m_uMaxPages = hpp_to_mac.max_pages;
        CaQmQueueConfigWrite((sbhandle) unit, qid, &qc);
    }

    /* configure the spi rx to drop large packets for this port */
    if (connection->bEgress == 0) {
        PORT_ERR(("%s: cannot set egress rates on non-egress paths\n", 
                  FUNCTION_NAME()));
        return BCM_E_CONFIG;
    }

    rv = bcm_stk_my_modid_get(unit, &modid);
    if (BCM_FAILURE(rv)) {
        PORT_ERR(("%s: Failed to get modid: %s\n", 
                  FUNCTION_NAME(), bcm_errmsg(rv)));
        return rv;
    }

    rv = soc_sbx_node_port_get(unit, modid, port, &fab_unit, &node, &fab_port);
    if (BCM_FAILURE(rv)) {
        PORT_ERR(("%s: failed to convert port %d to a fabric port: %s\n", 
                  FUNCTION_NAME(), port, bcm_errmsg(rv)));
        return rv;
    }

    spi_bus = connection->from.ulInterfacePortType == SB_FE2000_IF_PTYPE_SPI1;

    frame_size = 
        SAND_HAL_READ_INDEX_STRIDE(SOC_SBX_SBHANDLE(unit), C2, SR, 
                                   spi_bus, SR0_P0_FRAME_SIZE, fab_port);

    frame_size = 
        SAND_HAL_MOD_FIELD(C2, SR0_P0_FRAME_SIZE, MAX_FRAME_SIZE, frame_size, 
                           frame_max + 8);

    SAND_HAL_WRITE_INDEX_STRIDE(SOC_SBX_SBHANDLE(unit), C2, SR, spi_bus,
                                SR0_P0_FRAME_SIZE, fab_port, frame_size);

    return BCM_E_NONE;
}

int
bcm_fe2000_port_rate_ingress_get(int unit, bcm_port_t port,
                                 uint32 *kbits_sec, uint32 *kbits_burst)
{
    PORT_INIT_CHECK(unit, port);

    if (!soc_feature(unit, soc_feature_egress_metering)) {
        return BCM_E_UNAVAIL;
    }

    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_rate_ingress_set(int unit, bcm_port_t port,
                                 uint32 kbits_sec, uint32 kbits_burst)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_rate_pause_get(int unit, bcm_port_t port,
                               uint32 *kbits_pause, uint32 *kbits_resume)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_rate_pause_set(int unit, bcm_port_t port,
                               uint32 kbits_pause, uint32 kbits_resume)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_sample_rate_get(int unit, bcm_port_t port,
                                int *ingress_rate, int *egress_rate)
{
    PORT_INIT_CHECK(unit, port);

    if (ingress_rate == NULL  || egress_rate == NULL) {
        return BCM_E_PARAM;
    }

    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_sample_rate_set(int unit, bcm_port_t port,
                                int ingress_rate, int egress_rate)
{
    PORT_INIT_CHECK(unit, port);
    if (ingress_rate < 0 || egress_rate < 0) {
        return BCM_E_PARAM;
    }

    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_stp_get(int unit, bcm_port_t port, int *stp_state)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_stp_set(int unit, bcm_port_t port, int stp_state)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_tgid_get(int unit, bcm_port_t port, int *tid, int *psc)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_tgid_set(int unit, bcm_port_t port, int tid, int psc)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_trunk_index_get(int unit, bcm_port_t port, int *port_index)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_trunk_index_set(int unit, bcm_port_t port, int port_index)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_vlan_inner_tag_get(int unit, bcm_port_t port,
                                   uint16 *inner_tag)
{
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_vlan_inner_tag_set(int unit, bcm_port_t port,
                                   uint16 inner_tag)
{
    return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_fe2000_port_vlan_member_get
 *   Purpose
 *      Get the settings for ingress/egress filtering of frames tagged for a
 *      particular VLAN on the specified port, when the port is not a member of
 *      the VLAN for which the frame is tagged.
 *   Parameters
 *      (in) int unit = unit number whose VID is to have the ports list fetched
 *      (in) bcm_port_t port = port whose filtering is to be checked
 *      (out) uint32* flags = where to put the filtering flags
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      No support for egress filtering, so it's always FALSE.
 */
int
bcm_fe2000_port_vlan_member_get(int unit, bcm_port_t port, uint32 *flags)
{
    int    ingrFilter;
    int    egrFilter;
    int    result;

    if (flags) {
        /* result pointer good; get ingress filtering state for the port */
        result = _bcm_fe2000_vlan_port_filter_get(unit,
                                                  port,
                                                  &ingrFilter,
                                                  &egrFilter);
        if (BCM_E_NONE == result) {
            /* successfully read filter information; parse it out */
            if (ingrFilter) {
                /* ingress filtering enabled */
                *flags = BCM_PORT_VLAN_MEMBER_INGRESS;
            } else {
                /* no ingress filtering enabled */
                *flags = 0;
            }
            if (egrFilter) {
                /* egress filtering enabled */
                *flags |= BCM_PORT_VLAN_MEMBER_EGRESS;
            }
        } /* if (BCM_E_NONE == result) */
    } else { /* if (flags) */
        /* result pointer bad; parameter error */
        PORT_ERR(("%s: NULL pointer for flags\n", FUNCTION_NAME()));
        result = BCM_E_PARAM;
    } /* if (flags) */
    return result;
}

/*
 *   Function
 *      bcm_fe2000_port_vlan_member_set
 *   Purpose
 *      Set the settings for ingress/egress filtering of frames tagged for a
 *      particular VLAN on the specified port, when the port is not a member of
 *      the VLAN for which the frame is tagged.
 *   Parameters
 *      (in) int unit = unit number whose VID is to have the ports list fetched
 *      (in) bcm_port_t port = port whose filtering is to be checked
 *      (in) uint32 flags = the filtering flags
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 */
int
bcm_fe2000_port_vlan_member_set(int unit, bcm_port_t port, uint32 flags)
{
    int result;

    /* make sure things look okay */
    if ((~(BCM_PORT_VLAN_MEMBER_INGRESS | BCM_PORT_VLAN_MEMBER_EGRESS)) & flags) {
        /* the flags are not valid */
        PORT_ERR(("%s: invalid or unsupported flags %08X\n",
                  FUNCTION_NAME(),
                  flags));
        return BCM_E_PARAM;
    }

    /* set ingress VLAN filtering mode for this port */
    result = _bcm_fe2000_vlan_port_ingress_filter_set(unit,
                                                      port,
                                                      (0 != (BCM_PORT_VLAN_MEMBER_INGRESS & flags)));

    /* set egress VLAN filtering mode for this port */
    if (BCM_E_NONE == result) {
        result = _bcm_fe2000_vlan_port_egress_filter_set(unit,
                                                         port,
                                                         (0 != (BCM_PORT_VLAN_MEMBER_EGRESS & flags)));
    }

    /* return results to caller */
    return result;
}

STATIC int
_bcm_fe2000_port_gport_attr_get(int unit, bcm_gport_t gport,
                                bcm_port_t *phy_port, bcm_vlan_t *match_vlan,
                                bcm_vlan_vector_t vlan_vec)
{
    if (BCM_GPORT_IS_VLAN_PORT(gport)) {
        bcm_vlan_port_t vlan_port;
        bcm_module_t myModId = ~0;
        bcm_module_t tgtModId = -1;

        sal_memset(&vlan_port, 0, sizeof(bcm_vlan_port_t));
        vlan_port.vlan_port_id = gport;
        vlan_port.criteria = BCM_VLAN_PORT_MATCH_INVALID;
        BCM_IF_ERROR_RETURN(bcm_vlan_port_find(unit, &vlan_port));

        
        BCM_IF_ERROR_RETURN(_bcm_fe2000_map_vlan_gport_target(unit,
                                                              gport,
                                                              &myModId,
                                                              &tgtModId,
                                                              phy_port,
                                                              NULL,
                                                              NULL));
        if (myModId != tgtModId) {
            
            return BCM_E_PARAM;
        }
        *match_vlan = vlan_port.match_vlan;

    } else if (BCM_GPORT_IS_MPLS_PORT(gport)) {
        BCM_IF_ERROR_RETURN
          (_bcm_fe2000_mpls_port_gport_attr_get(unit,
                                                gport,
                                                phy_port,
                                                match_vlan,
                                                NULL));
    } else {
        return BCM_E_PARAM;
    }

    BCM_VLAN_VEC_ZERO(vlan_vec);
    BCM_IF_ERROR_RETURN(bcm_port_vlan_vector_get(unit, gport, vlan_vec));
    
    BCM_VLAN_VEC_SET(vlan_vec, *match_vlan);

    return BCM_E_NONE;
}

int
bcm_fe2000_port_vlan_priority_map_get(int unit, bcm_port_t port,
                                      int pkt_pri, int cfi,
                                      int *internal_pri, bcm_color_t *color)
{
    int rv;

    UNIT_INIT_CHECK(unit);

    if (internal_pri == NULL || color == NULL) {
        return BCM_E_PARAM;
    }

    PORT_LOCK(unit);

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv =  _bcm_fe2000_g2p3_port_vlan_priority_map_get(unit,
                                                          port,
                                                          pkt_pri,
                                                          cfi,
                                                          internal_pri,
                                                          color,
                                                          NULL,
                                                          NULL,
                                                          NULL);
        break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
        break;
    }

    PORT_UNLOCK(unit);
    return rv;
}

int
bcm_fe2000_port_vlan_priority_map_set(int unit, bcm_port_t port,
                                      int pkt_pri, int cfi,
                                      int internal_pri, bcm_color_t color)
{
    int rv;

    UNIT_INIT_CHECK(unit);
    PORT_LOCK(unit);

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
    {
        rv = _bcm_fe2000_g2p3_port_vlan_priority_map_set(unit,
                                                         port,
                                                         pkt_pri,
                                                         cfi,
                                                         internal_pri,
                                                         color,
                                                         internal_pri,
                                                         color,
                                                         internal_pri);
    }
        break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
        break;
    }

    PORT_UNLOCK(unit);
    return rv;
}

int
bcm_fe2000_port_vlan_priority_unmap_get(int unit,
                                        bcm_port_t port,
                                        int internal_pri,
                                        bcm_color_t color,
                                        int *pkt_pri,
                                        int *cfi)
{
    int rv;

    PORT_INIT_CHECK(unit, port);

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_port_vlan_priority_unmap_get(unit,
                                                           port,
                                                           internal_pri,
                                                           color,
                                                           pkt_pri,
                                                           cfi);
        break;
#endif /* def BCM_FE2000_P3_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
        break;
    }

    return rv;
}

int
bcm_fe2000_port_vlan_priority_unmap_set(int unit,
                                        bcm_port_t port,
                                        int internal_pri,
                                        bcm_color_t color,
                                        int pkt_pri,
                                        int cfi)
{
    int rv;

    PORT_INIT_CHECK(unit, port);

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        
        rv = _bcm_fe2000_g2p3_port_vlan_priority_unmap_set(unit,
                                                           port,
                                                           internal_pri,
                                                           color,
                                                           pkt_pri,
                                                           cfi);
        break;
#endif /* def BCM_FE2000_P3_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
        break;
    }

    return rv;
}


int
bcm_fe2000_port_vlan_pri_map_set(int unit, bcm_port_t port,
                                 bcm_vlan_t vlan, int pkt_pri,
                                 int cfi, int internal_pri,
                                 bcm_color_t color)
{
    int rv;

    UNIT_INIT_CHECK(unit);
    PORT_LOCK(unit);

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_port_vlan_pri_map_set(unit, port,
                                                    vlan, pkt_pri,
                                                    cfi, internal_pri,
                                                    color);
        break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
        break;
    }

    PORT_UNLOCK(unit);
    return rv;
}


int
bcm_fe2000_port_vlan_dscp_map_set(int unit, bcm_port_t port,
                                  bcm_vlan_t vlan, int dscp,
                                  int internal_pri, bcm_color_t color)
{
    int rv;

    if (0 == vlan) {
        return BCM_E_PARAM;
    }

    UNIT_INIT_CHECK(unit);
    PORT_LOCK(unit);

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_port_vlan_dscp_map_set(unit, port,
                                                       vlan, dscp,
                                                       internal_pri, color);
        break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
        break;
    }

    PORT_UNLOCK(unit);
    return rv;

}

int
bcm_fe2000_port_vlan_dscp_map_get(int unit, bcm_port_t port,
                                  bcm_vlan_t vlan,
                                  int dscp, int *internal_pri,
                                  bcm_color_t *color)
{
    int rv;

    if (0 == vlan) {
        return BCM_E_PARAM;
    }

    UNIT_INIT_CHECK(unit);
    PORT_LOCK(unit);

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
          rv = _bcm_fe2000_g2p3_port_vlan_dscp_map_get(unit, port,
                                                       vlan, dscp,
                                                       internal_pri, color);
        break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
        break;
    }

    PORT_UNLOCK(unit);
    return rv;
}

int
bcm_fe2000_port_vlan_pri_map_get(int unit, bcm_port_t port,
                                 bcm_vlan_t vlan, int pkt_pri,
                                 int cfi, int *internal_pri,
                                 bcm_color_t *color)
{
    int  rv;

#if 0 
    if (0 == vlan) {
        return BCM_E_PARAM;
    }
#endif 

    UNIT_INIT_CHECK(unit);
    PORT_LOCK(unit);

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_port_vlan_pri_map_get(unit, port,
                                                    vlan, pkt_pri,
                                                    cfi, internal_pri,
                                                    color);
        break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
        break;
    }

    PORT_UNLOCK(unit);
    return rv;
}

int
bcm_fe2000_port_vlan_priority_mapping_set(int unit,
                                          bcm_port_t gport,
                                          bcm_vlan_t vlan,
                                          int pkt_pri,
                                          int cfi,
                                          bcm_priority_mapping_t *pri_map)
{
    if (SOC_IS_SBX_G2P3(unit)) {
#ifdef BCM_FE2000_P3_SUPPORT
        bcm_port_t      phy_port;

        if (pri_map == NULL) {
            return BCM_E_PARAM;
        }
        
        if (BCM_GPORT_IS_VLAN_PORT(gport) ||
            BCM_GPORT_IS_MPLS_PORT(gport)) {
            bcm_vlan_t vid, match_vlan = BCM_VLAN_INVALID;
            bcm_vlan_vector_t vlan_vec;
            
            BCM_IF_ERROR_RETURN
                (_bcm_fe2000_port_gport_attr_get(unit,
                                                 gport,
                                                 &phy_port,
                                                 &match_vlan,
                                                 vlan_vec));
            
            if (match_vlan == BCM_VLAN_INVALID) {
                return(_bcm_fe2000_g2p3_port_vlan_priority_map_set(unit,
                                                                   phy_port,
                                                                   pkt_pri,
                                                                   cfi,
                                                                   pri_map->internal_pri,
                                                                   pri_map->color,
                                                                   pri_map->remark_internal_pri,
                                                                   pri_map->remark_color,
                                                                   pri_map->policer_offset));
            } else {
                for (vid = BCM_VLAN_MIN; vid <= BCM_VLAN_MAX; vid++) {
                    if (BCM_VLAN_VEC_GET(vlan_vec, vid)) {
                        BCM_IF_ERROR_RETURN
                            (_bcm_fe2000_g2p3_port_vlan_priority_mapping_set(unit,
                                                                             phy_port,
                                                                             vid,
                                                                             pkt_pri,
                                                                             cfi,
                                                                             pri_map));
                        /* optimize for vectors - All pv2e's will point to the same
                         * logical port, therefore an update to one, will update them
                         * all.
                         */
                        return BCM_E_NONE;
                    }
                }
                
            }
        } else if (BCM_GPORT_IS_LOCAL(gport)) {
            phy_port = BCM_GPORT_LOCAL_GET(gport);
        } else if (SOC_PORT_VALID(unit, gport)) {
            phy_port = gport;
        } else {
            PORT_ERR(("unit %d: port param %d is invalid\n",
                      unit, gport));
            return BCM_E_PORT;
        }


        if (vlan == BCM_VLAN_INVALID || vlan > BCM_VLAN_MAX) {

            return(_bcm_fe2000_g2p3_port_vlan_priority_map_set(unit,
                                                               phy_port,
                                                               pkt_pri,
                                                               cfi,
                                                               pri_map->internal_pri,
                                                               pri_map->color,
                                                               pri_map->remark_internal_pri,
                                                               pri_map->remark_color,
                                                               pri_map->policer_offset));
        }  else {
            return(_bcm_fe2000_g2p3_port_vlan_priority_mapping_set(unit,
                                                                   phy_port,
                                                                   vlan,
                                                                   pkt_pri,
                                                                   cfi,
                                                                   pri_map));
        }
#endif
    } else {

    }
    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_vlan_priority_mapping_get(int unit,
                                          bcm_port_t gport,
                                          bcm_vlan_t vlan,
                                          int pkt_pri,
                                          int cfi,
                                          bcm_priority_mapping_t *pri_map)
{
    if (SOC_IS_SBX_G2P3(unit)) {
#ifdef BCM_FE2000_P3_SUPPORT
        bcm_port_t phy_port;

        if (pri_map == NULL) {
            return BCM_E_PARAM;
        }

        if (BCM_GPORT_IS_VLAN_PORT(gport) ||
            BCM_GPORT_IS_MPLS_PORT(gport)) {
            bcm_vlan_t match_vlan = BCM_VLAN_INVALID;
            bcm_vlan_vector_t vlan_vec;

            BCM_IF_ERROR_RETURN
               (_bcm_fe2000_port_gport_attr_get(unit,
                                                 gport,
                                                 &phy_port,
                                                 &match_vlan,
                                                 vlan_vec));

            if (match_vlan == BCM_VLAN_INVALID) {
                return(_bcm_fe2000_g2p3_port_vlan_priority_map_get(unit,
                                                           phy_port,
                                                           pkt_pri,
                                                           cfi,
                                                           &pri_map->internal_pri,
                                                           &pri_map->color,
                                                           &pri_map->remark_internal_pri,
                                                           &pri_map->remark_color,
                                                           &pri_map->policer_offset));
            } else {
                return(_bcm_fe2000_g2p3_port_vlan_priority_mapping_get(unit,
                                                           phy_port,
                                                           match_vlan,
                                                           pkt_pri,
                                                           cfi,
                                                           pri_map));
            }
        } else if (BCM_GPORT_IS_LOCAL(gport)) {
            phy_port = BCM_GPORT_LOCAL_GET(gport);
        } else if (SOC_PORT_VALID(unit, gport)) {
            phy_port = gport;
        } else {
            PORT_ERR(("unit %d: port param %d is invalid\n",
                       unit, gport));
            return BCM_E_PORT;
        }

        if (vlan <= 0 || vlan == BCM_VLAN_INVALID || vlan > BCM_VLAN_MAX) {
            return(_bcm_fe2000_g2p3_port_vlan_priority_map_get(unit,
                                                           phy_port,
                                                           pkt_pri,
                                                           cfi,
                                                           &pri_map->internal_pri,
                                                           &pri_map->color,
                                                           &pri_map->remark_internal_pri,
                                                           &pri_map->remark_color,
                                                           &pri_map->policer_offset));
        }  else {
            return(_bcm_fe2000_g2p3_port_vlan_priority_mapping_get(unit,
                                                           phy_port,
                                                               vlan,
                                                               pkt_pri,
                                                               cfi,
                                                               pri_map));
        }
#endif /* BCM_FE2000_P3_SUPPORT */
    } else {

    }

    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_priority_color_get(int unit, bcm_port_t port,
                                   int prio, bcm_color_t *color)
{
    int ignore;
    return bcm_fe2000_port_vlan_priority_map_get(unit, port,
                                                 prio, 0,
                                                 &ignore, color);
}

int
bcm_fe2000_port_priority_color_set(int unit, bcm_port_t port,
                                   int prio, bcm_color_t color)
{
    return bcm_fe2000_port_vlan_priority_map_set(unit, port,
                                                 prio, 0,
                                                 prio, color);
}

int
bcm_fe2000_port_vlan_vector_set(int unit, bcm_gport_t gport,
                                bcm_vlan_vector_t vlan_vec)
{
    if (!BCM_GPORT_IS_SET(gport)) {
        return BCM_E_PARAM;
    }

    if (BCM_GPORT_IS_MPLS_PORT(gport)) {
        return (_bcm_fe2000_mpls_port_vlan_vector_set(unit,
                                                     gport,
                                                     vlan_vec));
    } else if (BCM_GPORT_IS_VLAN_PORT(gport)) {
        return (_bcm_fe2000_vlan_port_vlan_vector_set(unit,
                                                     gport,
                                                     vlan_vec));
    } else if (BCM_GPORT_IS_MIM_PORT(gport)) {
        return (_bcm_fe2000_mim_port_vlan_vector_set(unit,
                                                     gport,
                                                     vlan_vec));
    }

    return BCM_E_UNAVAIL;
}

int
bcm_fe2000_port_vlan_vector_get(int unit, bcm_gport_t gport,
                                bcm_vlan_vector_t vlan_vec)
{
    if (!BCM_GPORT_IS_SET(gport)) {
        return BCM_E_PARAM;
    }

    if (BCM_GPORT_IS_MPLS_PORT(gport)) {
        return (_bcm_fe2000_mpls_port_vlan_vector_get(unit,
                                                     gport,
                                                     vlan_vec));
    } else if (BCM_GPORT_IS_VLAN_PORT(gport)) {
        return (_bcm_fe2000_vlan_port_vlan_vector_get(unit,
                                                     gport,
                                                     vlan_vec));
    } else if (BCM_GPORT_IS_MIM_PORT(gport)) {
        return (_bcm_fe2000_mim_port_vlan_vector_get(unit,
                                                     gport,
                                                     vlan_vec));
    }

    return BCM_E_UNAVAIL;
}


int
_bcm_fe2000_port_strip_tag(int unit, bcm_port_t port, int strip)
{
    int  rv = BCM_E_UNAVAIL;

    UNIT_INIT_CHECK(unit);
    PORT_INIT_CHECK(unit, port);

    PORT_LOCK(unit);

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_port_strip_tag(unit, port, strip);
        break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
        break;
    }

    PORT_UNLOCK(unit);
    return rv;
}

int 
_bcm_fe2000_port_default_qos_profile_set(int unit, bcm_port_t port,
                                         bcm_vlan_t vid)
{
    int  rv = BCM_E_UNAVAIL;

    UNIT_INIT_CHECK(unit);
    PORT_INIT_CHECK(unit, port);

    PORT_LOCK(unit);

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_port_default_qos_profile_set(unit, port, vid);
        break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
        break;
    }

    PORT_UNLOCK(unit);
    return rv;
}


int
bcm_fe2000_port_policer_get(int unit, bcm_port_t port, bcm_policer_t *pol_id)
{
    int rv = BCM_E_NONE;
#ifdef BCM_FE2000_P3_SUPPORT
    soc_sbx_g2p3_lp_t   lp;
#endif

    if(!pol_id) {
        return BCM_E_PARAM;
    }

    if(BCM_GPORT_IS_MIM_PORT(port)) {
        /* Configure policer for MiM port */
        return _bcm_fe2000_mim_policer_get(unit, port, pol_id);
    }

    if(BCM_GPORT_IS_MPLS_PORT(port)) {
        /* Configure policer for mpls port */
        return _bcm_fe2000_mpls_policer_get(unit, port, pol_id);
    }

    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PARAM;
    }    

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        soc_sbx_g2p3_lp_t_init(&lp);
        /* Assumption: port is lp_index*/
        rv = soc_sbx_g2p3_lp_get(unit, port, &lp);
        if (rv == BCM_E_NONE) {
            *pol_id = lp.policer;
        }
        break;
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
        break;
    }
    
    return rv;
}

int
bcm_fe2000_port_policer_set(int unit, bcm_port_t port, bcm_policer_t pol_id)
{
    int                 rv = BCM_E_NONE;
#ifdef BCM_FE2000_P3_SUPPORT
    soc_sbx_g2p3_lp_t   lp;
    soc_sbx_g2p3_pv2e_t pv2e;
    int                 vIndex;
    bcm_policer_t       old_pol_id=0;
#endif

    if((pol_id < 0) || (pol_id > _bcm_policer_max_id_get(unit))) {
        return BCM_E_PARAM;
    }

    if (BCM_GPORT_IS_MIM_PORT(port)) {
        /* Configure policer for MiM port */
        return _bcm_fe2000_mim_policer_set(unit, port, pol_id);
    }

    if (BCM_GPORT_IS_MPLS_PORT(port)) {
        /* Configure policer for Mpls port */
        return _bcm_fe2000_mpls_policer_set(unit, port, pol_id);
    }

    if (BCM_GPORT_IS_VLAN_PORT(port)) {
        return bcm_fe2000_vgp_policer_set(unit, port, pol_id);
    }
    
    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PARAM;
    }

   switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        /* program the default lpi which is indexed by port */
        soc_sbx_g2p3_lp_t_init(&lp);
        rv = soc_sbx_g2p3_lp_get(unit, port, &lp);
        if (rv == BCM_E_NONE) {
            old_pol_id = lp.policer;
            rv = _bcm_fe2000_g2p3_policer_lp_program(unit, pol_id, &lp);
        }
        if (rv == BCM_E_NONE) {
            rv = soc_sbx_g2p3_lp_set(unit, port, &lp);
        }
        /* now program all pv2e entries with valid lpi & if current
          pv2e.lpi.policer is null or same as previous port pol_id */
        for (vIndex = 0; ((vIndex < BCM_VLAN_MAX) && (rv == BCM_E_NONE));
             vIndex++) {
            rv = SOC_SBX_G2P3_PV2E_GET(unit, port, vIndex, &pv2e);
            if ((rv == BCM_E_NONE) && (pv2e.lpi)) {
                soc_sbx_g2p3_lp_t_init(&lp);
                rv = soc_sbx_g2p3_lp_get(unit, pv2e.lpi, &lp);
                if ((rv == BCM_E_NONE) &&
                    ((!lp.policer) || (lp.policer == old_pol_id))) {
                    rv = _bcm_fe2000_g2p3_policer_lp_program(unit, pol_id,
                                                             &lp);
                    if (rv == BCM_E_NONE) {
                        rv = soc_sbx_g2p3_lp_set(unit, pv2e.lpi, &lp);
                    }
                }
            }
        }
        break;
#endif
   default:
       SBX_UNKNOWN_UCODE_WARN(unit);
       rv = BCM_E_INTERNAL;
       break;
   }

    return rv;
}


/*
 *   Function
 *      bcm_fe2000_port_qosmap_set
 *   Purpose
 *      Set QoS mapping behaviour on a physical port
 *   Parameters
 *      (in) int unit          = BCM device number
 *      (in) bcm_port_t gport = physical port
 *      (in) int ing_idx     = ingress map
 *      (in) int egr_map      = egress map
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *     Little parameter checking is done here.
 */
int
bcm_fe2000_port_qosmap_set(int unit, bcm_port_t port, 
                           int ing_idx, int egr_idx,
                           uint32 ing_flags, uint32 egr_flags)
{
    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        return _bcm_fe2000_g2p3_port_qosmap_set(unit, port, 
                                                ing_idx, egr_idx,
                                                ing_flags, egr_flags);
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        return BCM_E_INTERNAL;
    }
}

int
bcm_fe2000_port_qosmap_get(int unit, bcm_port_t port, 
                           int *ing_idx, int *egr_idx,
                           uint32 *ing_flags, uint32 *egr_flags)
{
    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        return _bcm_fe2000_g2p3_port_qosmap_get(unit, port, 
                                                ing_idx, egr_idx,
                                                ing_flags, egr_flags);
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        return BCM_E_INTERNAL;
    }
}

int
bcm_fe2000_port_vlan_qosmap_set(int unit, bcm_port_t port, 
                                bcm_vlan_t vid,
                                int ing_idx, int egr_idx,
                                uint32 ing_flags, uint32 egr_flags)
{
    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        return _bcm_fe2000_g2p3_port_vlan_qosmap_set(unit, port, vid,
                                                     ing_idx, egr_idx,
                                                     ing_flags, egr_flags);
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        return BCM_E_INTERNAL;
    }
}


int
bcm_fe2000_port_vlan_qosmap_get(int unit, bcm_port_t port, 
                                bcm_vlan_t vid,
                                int *ing_idx, int *egr_idx,
                                uint32 *ing_flags, uint32 *egr_flags)
{
    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        return _bcm_fe2000_g2p3_port_vlan_qosmap_get(unit, port, vid,
                                                     ing_idx, egr_idx,
                                                     ing_flags, egr_flags);
#endif
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        return BCM_E_INTERNAL;
    }
}


int
bcm_fe2000_port_congestion_config_get(int unit, bcm_gport_t port, 
                                      bcm_port_congestion_config_t *config)
{
  int rv = BCM_E_UNAVAIL;

  if (!SOC_IS_SBX_FE2KXT(unit)) {
    return rv;
  }

  if (config == NULL) {
    return BCM_E_PARAM;
  }

  /* Make sure port module is initialized. */
  PORT_INIT_CHECK(unit,port);

  if (IS_XE_PORT(unit, port) || IS_HG_PORT(unit, port)) {
    if (PORT(unit,port).e2ecc_config == NULL) {
      soc_cm_debug(DK_ERR,"Error: Port %s(%d) bcm_port_congestion_config was not set\n",
		   SOC_PORT_NAME(unit,port),port);
      return BCM_E_NOT_FOUND;
    }

    /* Retrieve E2ECC configuration */
    *config = *(PORT(unit, port).e2ecc_config);
    return BCM_E_NONE;

  } else {
    return BCM_E_UNAVAIL;
  }
}


int
bcm_fe2000_port_congestion_config_set(int unit, bcm_gport_t port,
                                      bcm_port_congestion_config_t *config)
{

    int rv = BCM_E_UNAVAIL;
    uint32 val, tmp;
    int iface = 0;
    int nXgm = 0;
    uint32 uDataLo = 0;
    uint32 uDataHi = 0;

    if (!SOC_IS_SBX_FE2KXT(unit)) {
        return rv;
    }

    if (config == NULL) {
        return BCM_E_PARAM;
    }

    if (!IS_XE_PORT(unit, port) && !IS_HG_PORT(unit, port)) {
        return BCM_E_PARAM;
    }

    if (!(config->flags & BCM_PORT_CONGESTION_CONFIG_E2ECC)) {
        return BCM_E_PARAM;
    }

    /* Make sure port module is initialized. */
    PORT_INIT_CHECK(unit,port);

    PORT_LOCK(unit);

    /* Allocate per-port congestion configuration state on demand */
    if (PORT(unit, port).e2ecc_config == NULL) {
      PORT(unit, port).e2ecc_config = sal_alloc(sizeof(bcm_port_congestion_config_t),
                                                            "bcm_port_congestion_config");
      if (PORT(unit, port).e2ecc_config == NULL) {
	soc_cm_debug(DK_ERR, "Error: unable to allocate memory for bcm_port_congestion_config.\n");
	PORT_UNLOCK(unit);
	return BCM_E_MEMORY;
      }
    }

    /* Store E2ECC configuration */
    *(PORT(unit, port).e2ecc_config) = *(config);

    PORT_UNLOCK(unit);

    /* program the requested config parameters */

    iface = sbFe2000TranslateBlock(SOC_PORT_BLOCK_TYPE(unit, port),
                                    SOC_PORT_BLOCK_NUMBER(unit, port));
    switch (iface) {
    case SB_FE2000_IF_PTYPE_XGM0:
        nXgm = 0;
        break;
    case SB_FE2000_IF_PTYPE_XGM1:
        nXgm = 1;
        break;
    case SB_FE2000_IF_PTYPE_AGM0: /* also XGM2 */
        nXgm = 2;
        break;
    case SB_FE2000_IF_PTYPE_AGM1: /* also XGM3 */
        nXgm = 3;
	break;
    default:
        return BCM_E_CONFIG;
    }

    /* disable tx pause frames in MAC to prevent C2 from pausing entire rx interface */
    rv = sbFe2000UtilXgmRead(SOC_SBX_SBHANDLE(unit),nXgm,
			     SAND_HAL_REG_OFFSET(C2,XM_MAC_TX_CONFIG_LO),&uDataHi,&uDataLo);
    if (rv != SB_FE2000_STS_INIT_OK_K) {
      return rv;
    }
			     
    uDataLo = SAND_HAL_MOD_FIELD(C2, XM_MAC_TX_CONFIG_LO, PAUSE_ENABLE,uDataLo, 0x0);
    rv = sbFe2000UtilXgmWrite(SOC_SBX_SBHANDLE(unit),nXgm,
			      SAND_HAL_REG_OFFSET(C2,XM_MAC_TX_CONFIG_LO),uDataHi,uDataLo);
    if (rv != SB_FE2000_STS_INIT_OK_K ) {
      return rv;
    }

    /* disable rx_pause_enable to prevent the paused TX interface from blocking e2ecc messages */
    rv = sbFe2000UtilXgmRead(SOC_SBX_SBHANDLE(unit),nXgm,
			     SAND_HAL_REG_OFFSET(C2,XM_MAC_RX_CONFIG_LO),&uDataHi,&uDataLo);
    if (rv != SB_FE2000_STS_INIT_OK_K) {
      return rv;
    }

    uDataLo = SAND_HAL_MOD_FIELD(C2, XM_MAC_RX_CONFIG_LO, RX_PAUSE_ENABLE,uDataLo, 0x0);
    rv = sbFe2000UtilXgmWrite(SOC_SBX_SBHANDLE(unit),nXgm,
			      SAND_HAL_REG_OFFSET(C2,XM_MAC_RX_CONFIG_LO),uDataHi,uDataLo);
    if (rv != SB_FE2000_STS_INIT_OK_K ) {
      return rv;
    }
    
    /* be sure force_port_xoff is clear or you will flow control the interface even
     * if not oversubscribed
     */
    
    SAND_HAL_RMW_FIELD_STRIDE(unit,C2,XG,nXgm,XG_E2ECC_DEBUG0,FORCE_PORT_XOFF,0x0);
    

    /* build the higig header for the e2ecc packet */
    val = (HIGIG_START << 24) |                                 /* KSOP */
      (((1 << 4) | (config->traffic_class & 0x0F)) << 16) |     /* MC and TC */ 
      (((config->multicast_id >> 8) & 0xFF) << 8) |             /* MGID (msb) */
      (config->multicast_id  & 0xFF);                           /* MGID (lsb) */

    SAND_HAL_RMW_FIELD_STRIDE(unit,C2,XG,nXgm,XG_E2ECC_HG_HDR0,HEADER,val);

    val = (config->src_port << 24) |                             /* C2 modid */
      ((config->color & 0x03) << 6);                             /* DP */

    SAND_HAL_RMW_FIELD_STRIDE(unit,C2,XG,nXgm,XG_E2ECC_HG_HDR1,HEADER,val);
      
    val = SAND_HAL_READ_STRIDE(unit, C2, XG, nXgm, XG_E2ECC_CONFIG0);
    val = SAND_HAL_MOD_FIELD(C2, XG_E2ECC_CONFIG0, TX_ENABLE, val, TRUE);
    val = SAND_HAL_MOD_FIELD(C2, XG_E2ECC_CONFIG0, TX_PERIOD, val, config->packets_per_sec);
    SAND_HAL_WRITE_STRIDE(unit, C2, XG, nXgm, XG_E2ECC_CONFIG0, val);

    SAND_HAL_RMW_FIELD_STRIDE(unit, C2, XG, nXgm, XG_E2ECC_FRAME3, CTL_OP, config->opcode);
    SAND_HAL_RMW_FIELD_STRIDE(unit, C2, XG, nXgm, XG_E2ECC_FRAME3, ETHERTYPE, config->ethertype);
    SAND_HAL_RMW_FIELD_STRIDE(unit, C2, XG, nXgm, XG_E2ECC_FRAME4, NUM_PORTS, config->port_bits);

    tmp = (((uint32)(config->dest_mac)[0] << 24) | 
           ((uint32)(config->dest_mac)[1] << 16) |
           ((uint32)(config->dest_mac)[2] << 8)  |
           ((uint32)(config->dest_mac)[3]));
    SAND_HAL_RMW_FIELD_STRIDE(unit, C2, XG, nXgm, XG_E2ECC_FRAME0, MACDA0, tmp);
    tmp = (((uint32)(config->dest_mac)[4] << 8) | 
            (uint32)(config->dest_mac)[5]); 
    SAND_HAL_RMW_FIELD_STRIDE(unit, C2, XG, nXgm, XG_E2ECC_FRAME1, MACDA1, tmp);
    tmp = (((uint32)(config->src_mac)[0] << 8) | 
            (uint32)(config->src_mac)[1]); 
    SAND_HAL_RMW_FIELD_STRIDE(unit, C2, XG, nXgm, XG_E2ECC_FRAME1, MACSA0, tmp);
    tmp = (((uint32)(config->src_mac)[2] << 24) | 
           ((uint32)(config->src_mac)[3] << 16) |
           ((uint32)(config->src_mac)[4] << 8)  |
           ((uint32)(config->src_mac)[5]));
    SAND_HAL_RMW_FIELD_STRIDE(unit, C2, XG, nXgm, XG_E2ECC_FRAME2, MACSA1, tmp);

    return BCM_E_NONE;
}
