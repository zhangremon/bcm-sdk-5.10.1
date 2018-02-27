/*
 * $Id: vswitch.c 1.45 Broadcom SDK $
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
 * Module: Generic Virtual Switching
 */



#include <soc/sbx/sbx_drv.h>

#include <bcm/types.h>
#include <bcm/debug.h>
#include <bcm/error.h>
#include <bcm/vlan.h>
#include <bcm/stack.h>


#ifdef BCM_FE2000_P3_SUPPORT
#include <soc/sbx/g2p3/g2p3.h>
#endif /* def BCM_FE2000_P3_SUPPORT */

#include <bcm_int/sbx/error.h>
#include <bcm_int/sbx/fe2000/allocator.h>
#include <bcm_int/sbx/fe2000/vlan.h>

/*
 *  Debugging output...
 */
#define VSWITCH_EXCESS_VERBOSITY 0
#if 0 
#undef BCM_DEBUG
#define BCM_DEBUG(flags, stuff)   soc_cm_print stuff
#endif 
#define VSWITCH_OUT(flags, stuff)   BCM_DEBUG(flags | BCM_DBG_VLAN, stuff)
#define VSWITCH_WARN(stuff)         VSWITCH_OUT(BCM_DBG_WARN, stuff)
#define VSWITCH_ERR(stuff)          VSWITCH_OUT(BCM_DBG_ERR, stuff)
#define VSWITCH_VERB(stuff)         VSWITCH_OUT(BCM_DBG_VERBOSE, stuff)
#define VSWITCH_VVERB(stuff)        VSWITCH_OUT(BCM_DBG_VVERBOSE, stuff)
#if VSWITCH_EXCESS_VERBOSITY
#define VSWITCH_EVERB(stuff)        VSWITCH_OUT(BCM_DBG_VVERBOSE, stuff)
#else /* VLAN_EXCESS_VERBOSITY */
#define VSWITCH_EVERB(stuff)
#endif /* VLAN_EXCESS_VERBOSITY */
#if defined(__GNUC__) && !defined(__PEDANTIC__)
#define VSWITCH_FUNCMSG(string) \
    "%s[%d]%s" string, __FILE__, __LINE__, __FUNCTION__
#define VSWITCH_FUNCMSG1(string) \
    "%s[%d]%s: " string, __FILE__, __LINE__, __FUNCTION__
#else /* defined(__GNUC__) && !defined(__PEDANTIC__) */
#define VSWITCH_FUNCMSG(string)    "%s[%d]" string, __FILE__, __LINE__
#define VSWITCH_FUNCMSG1(string)   "%s[%d]: " string, __FILE__, __LINE__
#endif /* defined(__GNUC__) && !defined(__PEDANTIC__) */

#define VSWITCH_UNIT_VALID_CHECK \
    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) { \
        VSWITCH_ERR((VSWITCH_FUNCMSG1("unit %d is not valid\n"), unit)); \
        return BCM_E_UNIT; \
    }
#define VSWITCH_UNIT_INIT_CHECK \
    if ((!(_primary_lock)) || (!(_vswitch_state[unit]))) { \
        VSWITCH_ERR((VSWITCH_FUNCMSG1("unit %d not initialised\n"), unit)); \
        return BCM_E_INIT; \
    }
#define VSWITCH_LOCK_TAKE \
    if (sal_mutex_take(_vswitch_state[unit]->lock, sal_mutex_FOREVER)) { \
        /* Cound not obtain unit lock  */ \
        VSWITCH_ERR((VSWITCH_FUNCMSG1("unable to obtain vswitch lock" \
                                      " on unit %d\n"), \
                  unit)); \
        return BCM_E_INTERNAL; \
    }
#define VSWITCH_LOCK_RELEASE \
    if (sal_mutex_give(_vswitch_state[unit]->lock)) { \
        /* Could not release unit lock */ \
        VSWITCH_ERR((VSWITCH_FUNCMSG1("unable to release vswitch lock" \
                                      " on unit %d\n"), \
                  unit)); \
        return BCM_E_INTERNAL; \
    }

/*
 *  This structure describes a single gport that is a member of a VSI.
 */
typedef struct _bcm_fe2000_vswitch_port_s {
    bcm_gport_t gport;                       /* GPort ID */
    struct _bcm_fe2000_vswitch_port_s *next; /* next entry this list */
    struct _bcm_fe2000_vswitch_port_s *prev; /* prev entry this list */
} _bcm_fe2000_vswitch_port_t;

/*
 *  This structure contains information about a given unit's vswitch state.
 *  Since we must have a list of ports that are included in a VSI (due to the
 *  requirement for delete_all), this also includes pointers to such lists.
 *  We also need to include lists for ALL VSIs since we can add GPORTs to
 *  various other special cases, such as traditional bridging.
 */
typedef struct _bcm_fe2000_vswitch_state_s {
    sal_mutex_t lock;
    _bcm_fe2000_vswitch_port_t *portList[16384];
} _bcm_fe2000_vswitch_state_t;

#define LPORT_IDX(gport) \
    ((~(_SHR_GPORT_TYPE_MASK << _SHR_GPORT_TYPE_SHIFT)) & gport)

volatile static sal_mutex_t _primary_lock = NULL;
static _bcm_fe2000_vswitch_state_t *_vswitch_state[BCM_MAX_NUM_UNITS];


#ifdef BCM_FE2000_P3_SUPPORT
/*
 *  Function
 *    _bcm_fe2000_p3_vswitch_create_id
 *  Purpose
 *    Allocate and prepare a specific VSI for use on g2p3
 *  Arguments
 *    int unit = unit on which the VSI is to be allocated
 *    bcm_vlan_t defVid = default VID for this unit
 *    bcm_vlan_t *vsi = where to put the created VSI ID
 *  Returns
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise
 *  Notes
 *    Must be called with vswitch lock taken
 *    Limited error checking
 */
int
_bcm_fe2000_p3_vswitch_create_id(int unit,
                                 bcm_vlan_t defVid,
                                 bcm_vlan_t vsi)
{
    int igmpSnoop;
    int rv;
    soc_sbx_g2p3_ft_t      p3ft;
    soc_sbx_g2p3_v2e_t     p3vlan2Etc;
    uint32 ftidx = 0;

    /* get IGMP snoop initial state from default VID */
    /* note this is atomic read so not bothering with VLAN lock */
    rv = soc_sbx_g2p3_v2e_get(unit, defVid, &p3vlan2Etc);
    if (BCM_E_NONE == rv) {
        igmpSnoop = (0 != p3vlan2Etc.igmp);
    } else {
        igmpSnoop = FALSE;
    }
    /* otherwise use mostly system defaults for the VSI */
    soc_sbx_g2p3_v2e_t_init(&p3vlan2Etc);
    p3vlan2Etc.dontlearn = FALSE;
    p3vlan2Etc.igmp = igmpSnoop;
    rv = soc_sbx_g2p3_v2e_set(unit, vsi, &p3vlan2Etc);

    

    if (BCM_E_NONE == rv) {
        soc_sbx_g2p3_ft_t_init(&p3ft);
        p3ft.excidx = 0;
        p3ft.oi = vsi;
        p3ft.qid = SBX_MC_QID_BASE;
        p3ft.mc = 1;
        BCM_IF_ERROR_RETURN
           (soc_sbx_g2p3_vlan_ft_base_get(unit, &ftidx));
        ftidx += vsi;
        rv = soc_sbx_g2p3_ft_set(unit, ftidx, &p3ft);
    }

    return rv;
}
#endif /* def BCM_FE2000_P3_SUPPORT */

/*
 *  Function
 *    bcm_fe2000_vswitch_create
 *  Purpose
 *    Allocate and prepare a VSI for use
 *  Arguments
 *    int unit = unit on which the VSI is to be allocated
 *    bcm_vlan_t *vsi = where to put the created VSI ID
 *  Returns
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise
 *  Notes
 */
int
bcm_fe2000_vswitch_create(int unit,
                          bcm_vlan_t *vsi)
{
    int    rv = BCM_E_UNAVAIL;

#ifdef BCM_FE2000_P3_SUPPORT

    uint32     alloc_vsi = ~0;
    bcm_vlan_t defVid;

    VSWITCH_VERB((VSWITCH_FUNCMSG("(%d,*) enter\n"), unit));

    VSWITCH_UNIT_VALID_CHECK;
    VSWITCH_UNIT_INIT_CHECK;

    if (!vsi) {
        VSWITCH_ERR((VSWITCH_FUNCMSG1("NULL pointer for outbound argument\n")));
        return BCM_E_PARAM;
    }

    /* get default VID; assume 1 if error */
    defVid = 1;
    rv = bcm_fe2000_vlan_default_get(unit, &defVid);

    VSWITCH_LOCK_TAKE;

    rv = _sbx_gu2_resource_alloc(unit,
                                 SBX_GU2K_USR_RES_VSI,
                                 1,
                                 &alloc_vsi,
                                 0);
    if (rv == BCM_E_NONE) {
        *vsi = alloc_vsi & 0xffff;
        switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            rv = _bcm_fe2000_p3_vswitch_create_id(unit, defVid, *vsi);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            SBX_UNKNOWN_UCODE_WARN(unit);
            rv = BCM_E_INTERNAL;
            _sbx_gu2_resource_free(unit,
                                   SBX_GU2K_USR_RES_VSI,
                                   1,
                                   &alloc_vsi,
                                   0);
        }
    }

    VSWITCH_LOCK_RELEASE;

    VSWITCH_VERB((VSWITCH_FUNCMSG("(%d,&(%04X)) return %d (%s)\n"),
                  unit,
                  *vsi,
                  rv,
                  _SHR_ERRMSG(rv)));

#endif /* BCM_FE2000_P3_SUPPORT */

    return rv;
}

/*
 *  Function
 *    bcm_fe2000_vswitch_create_with_id
 *  Purpose
 *    Allocate and prepare a specific VSI for use
 *  Arguments
 *    int unit = unit on which the VSI is to be allocated
 *    bcm_vlan_t *vsi = where to put the created VSI ID
 *  Returns
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise
 *  Notes
 */
int
bcm_fe2000_vswitch_create_with_id(int unit,
                                  bcm_vlan_t vsi)
{
    int    rv = BCM_E_UNAVAIL;

#ifdef BCM_FE2000_P3_SUPPORT

    uint32 alloc_vsi = (vsi & 0xFFFF); /* stupid signed extension workaround */
    bcm_vlan_t defVid;

    VSWITCH_VERB((VSWITCH_FUNCMSG("(%d,%04X) enter\n"), unit, vsi));

    VSWITCH_UNIT_VALID_CHECK;
    VSWITCH_UNIT_INIT_CHECK;

    if ((vsi < SBX_DYNAMIC_VSI_BASE(unit))
        || (vsi > SBX_DYNAMIC_VSI_END(unit))
       ) {
        VSWITCH_ERR((VSWITCH_FUNCMSG1("invalid VSI %04X for unit %d\n"),
                     vsi,
                     unit));
        return BCM_E_PARAM;
    }

    /* get default VID; assume 1 if error */
    defVid = 1;
    rv = bcm_fe2000_vlan_default_get(unit, &defVid);

    VSWITCH_LOCK_TAKE;

    /* try to allocate the specified VSI */
    rv = _sbx_gu2_resource_alloc(unit,
                                 SBX_GU2K_USR_RES_VSI,
                                 1,
                                 &alloc_vsi,
                                 _SBX_GU2_RES_FLAGS_RESERVE);
    if (BCM_E_BUSY == rv) {
        /* for some reason, we get 'busy' if it already exists */
        VSWITCH_ERR((VSWITCH_FUNCMSG1("VSI %04X already exists on unit %d\n"),
                     vsi,
                     unit));
        rv = BCM_E_EXISTS;
    }

    if (BCM_E_NONE == rv) {

        switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
        case SOC_SBX_UCODE_TYPE_G2P3:
            rv = _bcm_fe2000_p3_vswitch_create_id(unit, defVid, vsi);
            break;
#endif /* def BCM_FE2000_P3_SUPPORT */
        default:
            SBX_UNKNOWN_UCODE_WARN(unit);
            rv = BCM_E_INTERNAL;
            _sbx_gu2_resource_free(unit,
                                   SBX_GU2K_USR_RES_VSI,
                                   1,
                                   &alloc_vsi,
                                   0);
        }
    }

    VSWITCH_LOCK_RELEASE;

    VSWITCH_VERB((VSWITCH_FUNCMSG("(%d,%04X) return %d (%s)\n"),
                  unit,
                  vsi,
                  rv,
                  _SHR_ERRMSG(rv)));

#endif /* BCM_FE2000_P3_SUPPORT */

    return rv;
}


#ifdef BCM_FE2000_P3_SUPPORT
/*
 *  Function
 *    _bcm_fe2000_p3_vswitch_destroy
 *  Purpose
 *    Destroy an existing VSI on g2p3
 *  Arguments
 *    int unit = on which unit to destroy the VSI
 *    bcm_vlan_t = the VSI to destroy
 *  Returns
 *    bcm_error_t cast as int
 *      BCM_E_NONE for success
 *      BCM_E_* otherwise
 *  Notes
 */
int
_bcm_fe2000_p3_vswitch_destroy(int unit,
                               bcm_vlan_t vsi)
{
    int rv;
    soc_sbx_g2p3_ft_t      p3ft;
    soc_sbx_g2p3_v2e_t     p3vlan2Etc;
    uint32 ftidx = 0;

    soc_sbx_g2p3_ft_t_init(&p3ft);
    p3ft.excidx = VLAN_INV_FTE_EXC;



        soc_sbx_g2p3_v2e_t_init(&p3vlan2Etc);
        p3vlan2Etc.dontlearn = TRUE;
        BCM_IF_ERROR_RETURN
           (soc_sbx_g2p3_vlan_ft_base_get(unit, &ftidx));
        ftidx += vsi;
        rv = soc_sbx_g2p3_ft_set(unit, ftidx, &p3ft);

    return rv;
}
#endif /* def BCM_FE2000_P3_SUPPORT */

/*
 *  Function
 *    _bcm_fe2000_vswitch_destroy
 *  Purpose
 *    Destroy an existing VSI
 *  Arguments
 *    int unit = on which unit to destroy the VSI
 *    bcm_vlan_t = the VSI to destroy
 *  Returns
 *    bcm_error_t cast as int
 *      BCM_E_NONE for success
 *      BCM_E_* otherwise
 *  Notes
 */
int
_bcm_fe2000_vswitch_destroy(int unit,
                            bcm_vlan_t vsi)
{
    int rv = BCM_E_UNAVAIL;

    /* destroy the VSI */
    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_p3_vswitch_destroy(unit, vsi);
        break;
#endif /* def BCM_FE2000_P3_SUPPORT */
   default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
    } 

    return rv;
}

/*
 *  Function
 *    bcm_fe2000_vswitch_destroy
 *  Purpose
 *    Destroy an existing VSI
 *  Arguments
 *    int unit = on which unit to destroy the VSI
 *    bcm_vlan_t = the VSI to destroy
 *  Returns
 *    bcm_error_t cast as int
 *      BCM_E_NONE for success
 *      BCM_E_* otherwise
 *  Notes
 */
int
bcm_fe2000_vswitch_destroy(int unit,
                           bcm_vlan_t vsi)
{
    int rv;
    uint32 tempVsi = (vsi & 0xFFFF); /* stupid signed extension workaround */

    VSWITCH_VERB((VSWITCH_FUNCMSG("(%d,%04X) enter\n"), unit, vsi));

    VSWITCH_UNIT_VALID_CHECK;
    VSWITCH_UNIT_INIT_CHECK;

    VSWITCH_LOCK_TAKE;

    /* make sure the VSI is allocated */
    rv = _sbx_gu2_resource_test(unit, SBX_GU2K_USR_RES_VSI, vsi);
    if (BCM_E_EXISTS == rv) {
        rv = BCM_E_NONE;
    } else {
        VSWITCH_ERR((VSWITCH_FUNCMSG1("access unit %d VSI %04X: %d (%s)\n"),
                     unit,
                     vsi,
                     rv,
                     _SHR_ERRMSG(rv)));
    }

    /* make sure VSI has no attached ports */
    if (BCM_E_NONE == rv) {
        if (_vswitch_state[unit]->portList[tempVsi]) {
            rv = BCM_E_BUSY;
            VSWITCH_ERR((VSWITCH_FUNCMSG1("unit %d VSI %04X still has"
                                          " ports attached\n"),
                         unit,
                         vsi));
        }
    }

    /* destroy the VSI */
    if (BCM_E_NONE == rv) {
        rv = _bcm_fe2000_vswitch_destroy(unit, vsi);
    }

    /* release the VSI for reuse */
    if (BCM_E_NONE == rv) {
        rv = _sbx_gu2_resource_free(unit,
                                    SBX_GU2K_USR_RES_VSI,
                                    1,
                                    &tempVsi,
                                    0);
    }

    VSWITCH_LOCK_RELEASE;

    VSWITCH_VERB((VSWITCH_FUNCMSG("(%d,%04X) return %d (%s)\n"),
                  unit,
                  vsi,
                  rv,
                  _SHR_ERRMSG(rv)));

    return rv;
}

/*
 *  Function
 *    _bcm_fe2000_vswitch_port_get
 *  Purpose
 *    Get the vswitch of which the port is a member
 *  Arguments
 *    int unit = on which unit to destroy the VSI
 *    bcm_gport_t port = the gport to check
 *    bcm_vlan_t *vsi = where to put the VSI of which the port is a member
 *  Returns
 *    bcm_error_t cast as int
 *      BCM_E_NONE for success
 *      BCM_E_* otherwise
 *  Notes
 *    Defers to the appropriate module to make the check for its own gport
 *    type rather than trying to include that knowledge (and access to the
 *    appropriate locks) here.
 *    This is used internally because we need a way to get the VSI from a given
 *    gport.  If 0 == VSI, we assume it has no assigned VSI.
 */
static int
_bcm_fe2000_vswitch_port_get(int unit,
                            bcm_gport_t gport,
                            bcm_vlan_t *vsi)
{
    int rv = BCM_E_UNAVAIL;
    int gpType = (gport >> _SHR_GPORT_TYPE_SHIFT) & _SHR_GPORT_TYPE_MASK;

    /* get the VSI from the port */
    switch (gpType) {
#if 0 
    case _SHR_GPORT_TYPE_MODPORT:
        /* fallthrough intentional -- port should do this also */
    case _SHR_GPORT_TYPE_LOCAL:
        /* need to have port do the work for this type */
        /*
         *  To work as I'd expect, this function would have to put the port
         *  into 'raw' mode, so that VID tags are considered part of the
         *  frame payload instead of something for the forwarder to parse,
         *  except that it probably needs to strip the tags so the proper
         *  tags can be put in the expected place on egress.  This would,
         *  as does VLAN, have to specify the egress encapsulation, though
         *  I'm not sure what kind of examples make real sense here.
         */
        rv = _bcm_fe2000_port_vsi_gport_get(unit, vsi, gport);
        break;
    case _SHR_GPORT_TYPE_MPLS_PORT:
        /* need to have MPLS do the work for this type */
        /*
         *  So this would probably set up that anything received with a
         *  specific MPLS label or label stack would have the labels tossed
         *  and the payload inserted to the VSI for forwarding.
         */
        rv = _bcm_fe2000_mpls_vsi_gport_get(unit, vsi, gport);
        break;
    /* Probably will also be other types that make sense here */
#endif 
    case BCM_GPORT_VLAN_PORT:
        /* need to have VLAN do the work for this type */
        rv = _bcm_fe2000_vlan_vsi_gport_get(unit, gport, vsi);
        break;
    default:
        VSWITCH_ERR((VSWITCH_FUNCMSG1("can't query unacceptable gport %08X"
                                      " type %d on unit %d\n"),
                     gport,
                     gpType,
                     unit));
        rv = BCM_E_PARAM;
    }

    return rv;
}

/*
 *  Function
 *    bcm_fe2000_vswitch_port_add
 *  Purpose
 *    Add a gport to a vswitch
 *  Arguments
 *    int unit = on which unit to destroy the VSI
 *    bcm_vlan_t vsi = the VSI to which the port is to be added
 *    bcm_gport_t port = the gport to add
 *  Returns
 *    bcm_error_t cast as int
 *      BCM_E_NONE for success
 *      BCM_E_* otherwise
 *  Notes
 *    Defers to the appropriate module to make the changes for its own gport
 *    type rather than trying to include that knowledge (and access to the
 *    appropriate locks) here.
 */
int
bcm_fe2000_vswitch_port_add(int unit,
                            bcm_vlan_t vsi,
                            bcm_gport_t gport)
{
    int rv = BCM_E_UNAVAIL;
    int valid = FALSE;
    int gpType = (gport >> _SHR_GPORT_TYPE_SHIFT) & _SHR_GPORT_TYPE_MASK;
    _bcm_fe2000_vswitch_port_t *listEntry = NULL;
    bcm_vlan_t currVsi = 0;
    uint32 tempVsi = (vsi & 0xFFFF); /* stupid signed extension workaround */

    VSWITCH_VERB((VSWITCH_FUNCMSG("(%d,%04X,%08X) enter\n"),
                  unit,
                  vsi,
                  gport));

    VSWITCH_UNIT_VALID_CHECK;
    VSWITCH_UNIT_INIT_CHECK;

    VSWITCH_LOCK_TAKE;

    /* check the VSI to see if it's valid */
    if (BCM_VLAN_MAX < vsi) {
        /* not TB; make sure the VSI has been allocated */
        rv = _sbx_gu2_resource_test(unit, SBX_GU2K_USR_RES_VSI, vsi);
        if (BCM_E_EXISTS == rv) {
            rv = BCM_E_NONE;
        } else {
            VSWITCH_ERR((VSWITCH_FUNCMSG1("access unit %d VSI %04X: %d (%s)\n"),
                         unit,
                         vsi,
                         rv,
                         _SHR_ERRMSG(rv)));
        }
    } else { /* if (BCM_VLAN_MAX < vsi) */
        /* Traditional bridging; ask VLAN if the VSI has been allocated */
        rv = _bcm_fe2000_vlan_check_exists(unit, vsi, &valid);
        if ((BCM_E_NONE == rv) && !valid) {
            /* VSI successfully found to not exist, so still an error here */
            rv = BCM_E_NOT_FOUND;
        }
        if (BCM_E_NONE != rv) {
            VSWITCH_ERR((VSWITCH_FUNCMSG1("access unit %d VLAN %03X: %d (%s)\n"),
                         unit,
                         vsi,
                         rv,
                         _SHR_ERRMSG(rv)));
        }
    } /* if (BCM_VLAN_MAX < vsi) */

    /* allocate the list entry for this port */
    if (BCM_E_NONE == rv) {
        listEntry = sal_alloc(sizeof(_bcm_fe2000_vswitch_port_t),
                              "_bcm_fe2000_vswitch_port_list_entry");
        if (listEntry) {
            listEntry->gport = gport;
            listEntry->next = _vswitch_state[unit]->portList[tempVsi];
            listEntry->prev = NULL;
        } else {
            VSWITCH_ERR((VSWITCH_FUNCMSG1("unable to create VSI"
                                          " gport list entry on unit %d\n"),
                         unit));
            rv = BCM_E_MEMORY;
        }
    }

    /* since we're keeping a list, must move by delete then add */
    if (BCM_E_NONE == rv) {
        rv = _bcm_fe2000_vswitch_port_get(unit, gport, &currVsi);
        if (BCM_E_NONE == rv) {
            if (currVsi && (currVsi != vsi)) {
                /* moving the gport to another VSI without removing it first */
                VSWITCH_ERR((VSWITCH_FUNCMSG1("trying to move unit %d gport"
                                              " %08X to VSI %04X without"
                                              " removing it from VSI %04X"
                                              " first\n"),
                             unit,
                             gport,
                             vsi,
                             currVsi));
                rv = BCM_E_CONFIG;
            }
        } else {
            VSWITCH_ERR((VSWITCH_FUNCMSG1("unable to read current VSI for "
                                          "unit %d gport %08X\n"),
                         unit,
                         gport));
        }
    }

    /* add the port to the VSI */
    if (BCM_E_NONE == rv) {
        switch (gpType) {
#if 0 
        case _SHR_GPORT_TYPE_MODPORT:
            /* fallthrough intentional -- port should do this also */
        case _SHR_GPORT_TYPE_LOCAL:
            /* need to have port do the work for this type */
            /*
             *  To work as I'd expect, this function would have to put the port
             *  into 'raw' mode, so that VID tags are considered part of the
             *  frame payload instead of something for the forwarder to parse,
             *  except that it probably needs to strip the tags so the proper
             *  tags can be put in the expected place on egress.  This would,
             *  as does VLAN, have to specify the egress encapsulation, though
             *  I'm not sure what kind of examples make real sense here.
             */
            rv = _bcm_fe2000_port_vsi_gport_add(unit, vsi, gport);
            break;
        case _SHR_GPORT_TYPE_MPLS_PORT:
            /* need to have MPLS do the work for this type */
            /*
             *  So this would probably set up that anything received with a
             *  specific MPLS label or label stack would have the labels tossed
             *  and the payload inserted to the VSI for forwarding.
             */
            rv = _bcm_fe2000_mpls_vsi_gport_add(unit, vsi, gport);
            break;
        /* Probably will also be other types that make sense here */
#endif 
        case BCM_GPORT_VLAN_PORT:
            /* need to have VLAN do the work for this type */
            rv = _bcm_fe2000_vlan_vsi_gport_add(unit, vsi, gport);
            break;
        default:
            VSWITCH_ERR((VSWITCH_FUNCMSG1("can't add unacceptable gport %08X"
                                          " type %d to unit %d VSI %04X\n"),
                         gport,
                         gpType,
                         unit,
                         vsi));
            rv = BCM_E_PARAM;
        }
    }

    if (BCM_E_NONE == rv) {
        /* add this port to those in the VSI */
        _vswitch_state[unit]->portList[tempVsi] = listEntry;
        if (listEntry->next) {
            listEntry->next->prev = listEntry;
        }
    } else {
        /* don't track this port */
        if (listEntry) {
            sal_free(listEntry);
        }
    }

    VSWITCH_LOCK_RELEASE;

    VSWITCH_VERB((VSWITCH_FUNCMSG("(%d,%04X,%08X) return %d (%s)\n"),
                  unit,
                  vsi,
                  gport,
                  rv,
                  _SHR_ERRMSG(rv)));

    return rv;
}

/*
 *  Function
 *    _bcm_fe2000_vswitch_port_delete
 *  Purpose
 *    Remove a gport from a vswitch
 *  Arguments
 *    int unit = on which unit to destroy the VSI
 *    bcm_vlan_t vsi = the VSI from which the port is to be removed
 *    bcm_gport_t port = the gport to remove
 *  Returns
 *    bcm_error_t cast as int
 *      BCM_E_NONE for success
 *      BCM_E_* otherwise
 *  Notes
 *    Defers to the appropriate module to make the changes for its own gport
 *    type rather than trying to include that knowledge (and access to the
 *    appropriate locks) here.
 *    This is called from delete and delete_all.
 */
static int
_bcm_fe2000_vswitch_port_delete(int unit,
                               bcm_vlan_t vsi,
                               bcm_gport_t gport)
{
    int rv;
    int gpType = (gport >> _SHR_GPORT_TYPE_SHIFT) & _SHR_GPORT_TYPE_MASK;

    /* remove the gport from the VSI */
    switch (gpType) {
#if 0 
    case _SHR_GPORT_TYPE_MODPORT:
        /* fallthrough intentional -- port should do this also */
    case _SHR_GPORT_TYPE_LOCAL:
        /* need to have port do the work for this type */
        /*
         *  To work as I'd expect, this function would have to put the port
         *  into 'raw' mode, so that VID tags are considered part of the
         *  frame payload instead of something for the forwarder to parse,
         *  except that it probably needs to strip the tags so the proper
         *  tags can be put in the expected place on egress.  This would,
         *  as does VLAN, have to specify the egress encapsulation, though
         *  I'm not sure what kind of examples make real sense here.
         */
        rv = _bcm_fe2000_port_vsi_gport_delete(unit, vsi, gport);
        break;
    case _SHR_GPORT_TYPE_MPLS_PORT:
        /* need to have MPLS do the work for this type */
        /*
         *  So this would probably set up that anything received with a
         *  specific MPLS label or label stack would have the labels tossed
         *  and the payload inserted to the VSI for forwarding.  Of course,
         *  it also has to specify egress encapsulation (labels &c).
         */
        rv = _bcm_fe2000_mpls_vsi_gport_delete(unit, vsi, gport);
        break;
    /* Probably will also be other types that make sense here */
#endif 
    case BCM_GPORT_VLAN_PORT:
        /* need to have VLAN do the work for this type */
        rv = _bcm_fe2000_vlan_vsi_gport_delete(unit, vsi, gport);
        break;
    default:
        VSWITCH_ERR((VSWITCH_FUNCMSG1("can't remove unacceptable gport %08X"
                                      " type %d from unit %d VSI %04X\n"),
                     gport,
                     gpType,
                     unit,
                     vsi));
        rv = BCM_E_PARAM;
    }

    return rv;
}

/*
 *  Function
 *    bcm_fe2000_vswitch_port_delete
 *  Purpose
 *    Remove a gport from a vswitch
 *  Arguments
 *    int unit = on which unit to destroy the VSI
 *    bcm_vlan_t vsi = the VSI from which the port is to be removed
 *    bcm_gport_t port = the gport to remove
 *  Returns
 *    bcm_error_t cast as int
 *      BCM_E_NONE for success
 *      BCM_E_* otherwise
 *  Notes
 *    Defers to the appropriate module to make the changes for its own gport
 *    type rather than trying to include that knowledge (and access to the
 *    appropriate locks) here.
 */
int
bcm_fe2000_vswitch_port_delete(int unit,
                               bcm_vlan_t vsi,
                               bcm_gport_t gport)
{
    int rv = BCM_E_NONE;
    _bcm_fe2000_vswitch_port_t *listCurr = NULL;
    bcm_vlan_t currVsi = 0;
    uint32 tempVsi = (vsi & 0xFFFF); /* stupid signed extension workaround */

    VSWITCH_VERB((VSWITCH_FUNCMSG("(%d,%04X,%08X) enter\n"),
                  unit,
                  vsi,
                  gport));

    VSWITCH_UNIT_VALID_CHECK;
    VSWITCH_UNIT_INIT_CHECK;

    VSWITCH_LOCK_TAKE;

    /*
     *  Due to problems trying to coordinate TB VSIs with VSWITCH VSIs, we
     *  don't bother seeing if the VSI is valid here; all we do is make sure
     *  the VSI has some port in it (if not, we know we can return not found
     *  since the specified port isn't in the VSI).  Otherwise we have to check
     *  the GPORT to see if it's a member of the claimed VSI.
     */
    if (!(_vswitch_state[unit]->portList[tempVsi])) {
        VSWITCH_ERR((VSWITCH_FUNCMSG1("unit %d VSI %04X has no member GPORT\n"),
                     unit,
                     vsi));
        rv = BCM_E_NOT_FOUND;
    }

    /*
     *  Make sure the GPORT is actually a member of the claimed VSI, so we
     *  don't do something stupid like try to remove something that isn't in a
     *  list from that list.
     */
    if (BCM_E_NONE == rv) {
        rv = _bcm_fe2000_vswitch_port_get(unit, gport, &currVsi);
        if (BCM_E_NONE == rv) {
            if (currVsi && (currVsi != vsi)) {
                /* deleting the gport from the wrong VSI */
                VSWITCH_ERR((VSWITCH_FUNCMSG1("trying to remove unit %d"
                                              " gport %08X from VSI %04X but"
                                              " it belongs to VSI %04X\n"),
                             unit,
                             gport,
                             vsi,
                             currVsi));
                rv = BCM_E_CONFIG;
            }
        } else {
            VSWITCH_ERR((VSWITCH_FUNCMSG1("unable to read current VSI for "
                                          "unit %d gport %08X\n"),
                         unit,
                         gport));
        }
    }

    /* remove the gport from the VSI */
    if (BCM_E_NONE == rv) {
        rv = _bcm_fe2000_vswitch_port_delete(unit, vsi, gport);
    }

    /* remove the gport from the list of ports on this VSI */
    if (BCM_E_NONE == rv) {
        /* look for the gport to delete from the list */
        listCurr = _vswitch_state[unit]->portList[tempVsi];
        while (listCurr && (listCurr->gport != gport)) {
            /* still in the list and not found it yet */
            listCurr = listCurr->next;
        }
        if (listCurr) {
            /* found the gport to delete; delete it */
            if (listCurr->prev) {
                listCurr->prev->next = listCurr->next;
            } else {
                _vswitch_state[unit]->portList[tempVsi] = listCurr->next;
                if (_vswitch_state[unit]->portList[tempVsi]) {
                    _vswitch_state[unit]->portList[tempVsi]->prev = NULL;
                }
            }
            if (listCurr->next) {
                listCurr->next->prev = listCurr->prev;
            }

            sal_free(listCurr);
        }
        /*
         *  Going to take a lazy attitude here about not finding the list
         *  element to be deleted -- since it's already not in the list, we
         *  don't need to delete it, even though this situation should not
         *  happen unless due to corruption.
         */
    }

    VSWITCH_LOCK_RELEASE;

    VSWITCH_VERB((VSWITCH_FUNCMSG("(%d,%04X,%08X) return %d (%s)\n"),
                  unit,
                  vsi,
                  gport,
                  rv,
                  _SHR_ERRMSG(rv)));

    return rv;
}

/*
 *  Function
 *    _bcm_fe2000_vswitch_port_delete_all
 *  Purpose
 *    Remove all ports from a VSI
 *  Arguments
 *    int unit = the unit on which to operate
 *    bcm_vlan_t vsi = the VSI whose ports are to be removed
 *  Returns
 *    bcm_error_t cast as int
 *      BCM_E_NONE for success
 *      BCM_E_* as appropriate otherwise
 *  Notes
 *    Walks the VSI's gport member list, deleting each one.
 *    THIS WILL NOT REMOVE NON-VSWITCH PORTS FROM THE VSI!
 */
static int
_bcm_fe2000_vswitch_port_delete_all(int unit,
                                    bcm_vlan_t vsi,
                                    _bcm_fe2000_vswitch_state_t *tempUnit)
{
    int rv = BCM_E_NONE;
    _bcm_fe2000_vswitch_port_t *listCurr = NULL;
    uint32 tempVsi = (vsi & 0xFFFF); /* stupid signed extension workaround */

    /* for all gports in the list for this VSI, delete the gport */
    do {
        listCurr = tempUnit->portList[tempVsi];
        if (listCurr) {
            rv = _bcm_fe2000_vswitch_port_delete(unit, vsi, listCurr->gport);
            if (BCM_E_NONE == rv) {
                /* deleted the port; remove it from the list */
                tempUnit->portList[tempVsi] = listCurr->next;
                sal_free(listCurr);
            }
        }
    } while (listCurr && (BCM_E_NONE == rv));

    return rv;
}

/*
 *  Function
 *    bcm_fe2000_vswitch_port_delete_all
 *  Purpose
 *    Remove all ports from a VSI
 *  Arguments
 *    int unit = the unit on which to operate
 *    bcm_vlan_t vsi = the VSI whose ports are to be removed
 *  Returns
 *    bcm_error_t cast as int
 *      BCM_E_NONE for success
 *      BCM_E_* as appropriate otherwise
 *  Notes
 *    Walks the VSI's gport member list, deleting each one.
 *    THIS WILL NOT REMOVE NON-VSWITCH PORTS FROM THE VSI!
 */

int
bcm_fe2000_vswitch_port_delete_all(int unit,
                                   bcm_vlan_t vsi)
{
    int rv = BCM_E_UNAVAIL;
    int valid = FALSE;

    VSWITCH_VERB((VSWITCH_FUNCMSG("(%d,%04X) enter\n"),
                  unit,
                  vsi));

    VSWITCH_UNIT_VALID_CHECK;
    VSWITCH_UNIT_INIT_CHECK;

    VSWITCH_LOCK_TAKE;

    /* check the VSI to see if it's valid */
    if (BCM_VLAN_MAX < vsi) {
        /* not TB; make sure the VSI has been allocated */
        rv = _sbx_gu2_resource_test(unit, SBX_GU2K_USR_RES_VSI, vsi);
        if (BCM_E_EXISTS == rv) {
            rv = BCM_E_NONE;
        } else {
            VSWITCH_ERR((VSWITCH_FUNCMSG1("access unit %d VSI %04X: %d (%s)\n"),
                         unit,
                         vsi,
                         rv,
                         _SHR_ERRMSG(rv)));
        }
    } else { /* if (BCM_VLAN_MAX < vsi) */
        /* Traditional bridging; ask VLAN if the VSI has been allocated */
        rv = _bcm_fe2000_vlan_check_exists(unit, vsi, &valid);
        if ((BCM_E_NONE == rv) && !valid) {
            /*
             *  Now here's where this gets interesting -- it's possible to
             *  delete a TB VSI without getting rid of its GPORTs.  Because of
             *  this, we need to check whether there are GPORTs in an invalid
             *  TB VSI and allow delete_all to work even in that case (but it
             *  should return an error if there are no member GPORTs *and* the
             *  VSI does not exist).
             */
            if (!(_vswitch_state[unit]->portList[vsi])) {
                rv = BCM_E_NOT_FOUND;
            }
        }
        if (BCM_E_NONE != rv) {
            VSWITCH_ERR((VSWITCH_FUNCMSG1("access unit %d VLAN %03X: %d (%s)\n"),
                         unit,
                         vsi,
                         rv,
                         _SHR_ERRMSG(rv)));
        }
    } /* if (BCM_VLAN_MAX < vsi) */

    /* for all gports in the list for this VSI, delete the gport */
    if (BCM_E_NONE == rv) {
        rv = _bcm_fe2000_vswitch_port_delete_all(unit,
                                                 vsi,
                                                 _vswitch_state[unit]);
    }

    VSWITCH_LOCK_RELEASE;

    VSWITCH_VERB((VSWITCH_FUNCMSG("(%d,%04X) return %d (%s)\n"),
                  unit,
                  vsi,
                  rv,
                  _SHR_ERRMSG(rv)));

    return rv;
}

/*
 *  Function
 *    bcm_fe2000_vswitch_port_get
 *  Purpose
 *    Get the vswitch of which the port is a member
 *  Arguments
 *    int unit = on which unit to destroy the VSI
 *    bcm_gport_t port = the gport to check
 *    bcm_vlan_t *vsi = where to put the VSI of which the port is a member
 *  Returns
 *    bcm_error_t cast as int
 *      BCM_E_NONE for success
 *      BCM_E_* otherwise
 *  Notes
 *    Defers to the appropriate module to make the changes for its own gport
 *    type rather than trying to include that knowledge (and access to the
 *    appropriate locks) here.
 */
int
bcm_fe2000_vswitch_port_get(int unit,
                            bcm_gport_t gport,
                            bcm_vlan_t *vsi)
{
    int rv = BCM_E_UNAVAIL;

    VSWITCH_VERB((VSWITCH_FUNCMSG("(%d,%08X,*) enter\n"),
                  unit,
                  gport));

    VSWITCH_UNIT_VALID_CHECK;
    VSWITCH_UNIT_INIT_CHECK;

    if (!vsi) {
        VSWITCH_ERR((VSWITCH_FUNCMSG1("NULL pointer for outbound argument\n")));
        return BCM_E_PARAM;
    }

    VSWITCH_LOCK_TAKE;

    rv = _bcm_fe2000_vswitch_port_get(unit, gport, vsi);

    VSWITCH_LOCK_RELEASE;

    VSWITCH_VERB((VSWITCH_FUNCMSG("(%d,%08X,&(%04X)) return %d (%s)\n"),
                  unit,
                  gport,
                  *vsi,
                  rv,
                  _SHR_ERRMSG(rv)));

    return rv;
}

/*
 *  Function
 *    _bcm_fe2000_vswitch_detach
 *  Purpose
 *    Shut down the vswitch functionality
 *  Arguments
 *    int unit = the unit whose vswitch function is to be initialised
 *  Returns
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* as appropriate otherwise
 *  Notes
 */
static int
_bcm_fe2000_vswitch_detach(int unit)
{
    int rv = BCM_E_NONE;
    _bcm_fe2000_vswitch_state_t *tempUnit = _vswitch_state[unit];

    VSWITCH_EVERB((VSWITCH_FUNCMSG("(%d) enter\n"), unit));

    /* disconnect the unit information (keep others from seeing unit) */
    VSWITCH_EVERB((VSWITCH_FUNCMSG1("unlink unit %d data\n"), unit));
    _vswitch_state[unit] = NULL;

    /* destroy the lock (this should wait until no other owners) */
    VSWITCH_EVERB((VSWITCH_FUNCMSG1("destroy unit %d lock\n"), unit));
    sal_mutex_destroy(tempUnit->lock);

    /* get rid of the unit state information */
    VSWITCH_EVERB((VSWITCH_FUNCMSG1("release unit %d state structure\n"),
                   unit));
    sal_free(tempUnit);

    VSWITCH_EVERB((VSWITCH_FUNCMSG("(%d) return %d (%s)\n"),
                   unit,
                   rv,
                   _SHR_ERRMSG(rv)));

    return rv;
}

/*
 *  Function
 *    bcm_fe2000_vswitch_detach
 *  Purpose
 *    Shut down the vswitch functionality
 *  Arguments
 *    int unit = the unit whose vswitch function is to be initialised
 *  Returns
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* as appropriate otherwise
 *  Notes
 */
int
bcm_fe2000_vswitch_detach(int unit)
{
    int result;

    VSWITCH_VERB((VSWITCH_FUNCMSG("(%d) enter\n"), unit));

    VSWITCH_UNIT_VALID_CHECK;
    VSWITCH_UNIT_INIT_CHECK;

    if (sal_mutex_take(_primary_lock, sal_mutex_FOREVER)) {
        VSWITCH_ERR((VSWITCH_FUNCMSG1("unable to take primary lock\n")));
        return BCM_E_INTERNAL;
    }

    result = _bcm_fe2000_vswitch_detach(unit);

    if (sal_mutex_give(_primary_lock)) {
        VSWITCH_ERR((VSWITCH_FUNCMSG1("unable to release primary lock\n")));
        return BCM_E_INTERNAL;
    }

    VSWITCH_VERB((VSWITCH_FUNCMSG("(%d) return %d (%s)\n"),
                  unit,
                  result,
                  _SHR_ERRMSG(result)));

    return result;
}

/*
 *  Function
 *    bcm_fe2000_vswitch_init
 *  Purpose
 *    Initialise the vswitch functionality
 *  Arguments
 *    int unit = the unit whose vswitch function is to be initialised
 *  Returns
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* as appropriate otherwise
 *  Notes
 */
int
bcm_fe2000_vswitch_init(int unit)
{
    sal_mutex_t tempLock = NULL;
    _bcm_fe2000_vswitch_state_t *tempUnit = NULL;
    bcm_vlan_t vIndex;
    int didDetach = FALSE;
    int result;
    uint32             fti;
    soc_sbx_g2p3_ft_t  fte;

    VSWITCH_VERB((VSWITCH_FUNCMSG("(%d) enter\n"), unit));

    VSWITCH_UNIT_VALID_CHECK;

    /* configure the primary lock */
    if (!_primary_lock) {
        tempLock = sal_mutex_create("_bcm_fe2000_vswitch_primary_lock");
        if (NULL == tempLock) {
            VSWITCH_ERR((VSWITCH_FUNCMSG1("unable to create temp"
                                          "primary lock\n")));
            return BCM_E_RESOURCE;
        }
        _primary_lock = tempLock;
    }

    if (sal_mutex_take(_primary_lock, sal_mutex_FOREVER)) {
        VSWITCH_ERR((VSWITCH_FUNCMSG1("unable to take primary lock\n")));
        return BCM_E_INTERNAL;
    }
    if (tempLock && (_primary_lock != tempLock)) {
        VSWITCH_WARN((VSWITCH_FUNCMSG1("concurrent init; discarding spare"
                                       " primary lock instance\n")));
        sal_mutex_destroy(tempLock);
    }

    /* hope for the best */
    result = BCM_E_NONE;

    /* do reinit cleanly (detach first) */
    if (_vswitch_state[unit]) {
        /* already initialised; do detach first */
        VSWITCH_VVERB((VSWITCH_FUNCMSG1("detach unit %d first\n"), unit));
        result = _bcm_fe2000_vswitch_detach(unit);
        didDetach = TRUE;
    }

    /* create new unit state information */
    if (BCM_E_NONE == result) {
        VSWITCH_VVERB((VSWITCH_FUNCMSG1("allocate %d bytes unit %d state\n"),
                       sizeof(_bcm_fe2000_vswitch_state_t),
                       unit));
        tempUnit = sal_alloc(sizeof(_bcm_fe2000_vswitch_state_t),
                                    "_bcm_fe2000_vswitch_unit_state");
        if (tempUnit) {
            /* success; zero the memory cell */
            sal_memset(tempUnit, 0, sizeof(_bcm_fe2000_vswitch_state_t));
            /* create the unit lock */
            tempUnit->lock = sal_mutex_create("_bcm_fe2000_vswitch_unit_lock");
            if (!tempUnit->lock) {
                /* failed to create the unit lock */
                result = BCM_E_RESOURCE;
            }
        } else { /* if (tempUnit) */
            /* failed to create the working unit descriptor */
            result = BCM_E_MEMORY;
        } /* if (tempUnit) */
    } /* if (BCM_E_NONE == result) */

    if ((BCM_E_NONE == result) && !didDetach) {
        /* initialise appropriate tables */
        VSWITCH_VVERB((VSWITCH_FUNCMSG1("initialise unit %d VSI tables\n"),
                       unit));
        for (vIndex = SBX_DYNAMIC_VSI_BASE(unit);
             vIndex >= SBX_DYNAMIC_VSI_END(unit);
             vIndex--) {
            _bcm_fe2000_vswitch_destroy(unit, vIndex);
        }
    }

    VSWITCH_VVERB((VSWITCH_FUNCMSG1("commit unit %d changes\n"), unit));
    if (BCM_E_NONE == result) {
        /* all unit preparation went well; attach unit structure */
        _vswitch_state[unit] = tempUnit;
    } else { /* if (BCM_E_NONE == result) */
        /* something went wrong; undo any remaining unit allocation */
        if (tempUnit) {
            if (tempUnit->lock) {
                sal_mutex_destroy(tempUnit->lock);
            }
            sal_free(tempUnit);
        } /* if (tempUnit) */
        /*
         *  Note that we don't try to destroy the global lock; it's shared
         *  between all units and should not be destroyed once it exists.
         */
    } /* if (BCM_E_NONE == result) */

    if (SOC_WARM_BOOT(unit)) {
        uint32 vsi, base_fti;

        result = soc_sbx_g2p3_vlan_ft_base_get(unit, &base_fti);
        if (BCM_FAILURE(result)) {
            VSWITCH_ERR((_SBX_D(unit, "Failed to get base ft: %s\n"),
                         bcm_errmsg(result)));
            return result;
        }

        for (fti = SBX_DYNAMIC_VSI_FTE_BASE(unit);
             fti <= SBX_DYNAMIC_VSI_FTE_END(unit);
             fti++) 
        {

            result = soc_sbx_g2p3_ft_get(unit, fti, &fte);
            if (BCM_FAILURE(result)) {
                VSWITCH_ERR((_SBX_D(unit, "Failed to read fte[0x%x]: %s\n"),
                             fti, bcm_errmsg(result)));
                return result;
            }

            if (fte.mc && fte.excidx != VLAN_INV_FTE_EXC) {
                vsi = fti - base_fti;
                result = _sbx_gu2_resource_alloc(unit,
                                                 SBX_GU2K_USR_RES_VSI,
                                                 1, &vsi, 
                                                 _SBX_GU2_RES_FLAGS_RESERVE);
                VSWITCH_VERB((_SBX_D(unit, "Recover VSI FTE[0x%04x]: %d\n"), 
                              fti, result));
                             
                if (BCM_FAILURE(result)) {
                    VSWITCH_ERR((_SBX_D(unit, "Error reserving VSI: %s\n"),
                                 bcm_errmsg(result)));
                }
            }
        }

    } else {

        soc_sbx_g2p3_ft_t_init(&fte);
        fte.excidx = VLAN_INV_FTE_EXC;
        
        for (fti = SBX_DYNAMIC_VSI_FTE_BASE(unit);
             fti <= SBX_DYNAMIC_VSI_FTE_END(unit);
             fti++) 
        {
            result = soc_sbx_g2p3_ft_set(unit, fti, &fte);
            if (BCM_FAILURE(result)) {
                VSWITCH_ERR((_SBX_D(unit, "Failed to write fte[0x%x]: %s\n"),
                             fti, bcm_errmsg(result)));
                return result;
            }        
        }
    }

    VSWITCH_VERB((VSWITCH_FUNCMSG("(%d) return %d (%s)\n"),
                  unit, result, _SHR_ERRMSG(result)));
    
    return result;
}



/*
 *  Function
 *    _bcm_fe2000_vswitch_port_info_get
 *  Purpose
 *    Obtain tagged/untagged information with regard to double tagging for
 *    ports on a vsi
 *  Arguments
 *    unit    - bcm device number
 *    port    - local target port
 *    vsi     - vsi to scan
 *    keepUntagged - storage location for behavior setting
 */
int
_bcm_fe2000_vswitch_port_info_get(int unit, bcm_port_t port,
                                  bcm_vlan_t vsi, int *keepUntagged)
{
    int rv = BCM_E_NOT_FOUND;
    _bcm_fe2000_vswitch_port_t *pCur = NULL;
    bcm_vlan_port_t vlanPort;
    bcm_module_t    localMod, vpMod;
    bcm_port_t      vpPort;

    VSWITCH_VERB((VSWITCH_FUNCMSG("(%d,0x%X,0x%04X) enter\n"), 
                  unit, port, vsi));

    BCM_IF_ERROR_RETURN(bcm_stk_my_modid_get(unit, &localMod));


    VSWITCH_LOCK_TAKE;

    /* iterate all gports in the VSI -
     *   for each vlan gport -
     *     vlan_port from vlan with bcm_vlan_port_find
     *     set keepUntagged if port/mods match and not set to ADD a vid
     */

    pCur = _vswitch_state[unit]->portList[vsi];
    while (pCur && (rv == BCM_E_NOT_FOUND)) {
        if (BCM_GPORT_IS_VLAN_PORT(pCur->gport)) {
            int tmpRv;

            sal_memset(&vlanPort, 0, sizeof(bcm_vlan_port_t));
            vlanPort.vlan_port_id = pCur->gport;
            
            tmpRv = bcm_vlan_port_find(unit, &vlanPort);
            
            if (BCM_SUCCESS(tmpRv)) {
                vpMod  = BCM_GPORT_MODPORT_MODID_GET(vlanPort.port);
                vpPort = BCM_GPORT_MODPORT_PORT_GET(vlanPort.port);
                
                VSWITCH_VERB((VSWITCH_FUNCMSG(" gp=0x%08x flags=0x%08x "
                                              "port=%d mod=%d\n"),
                              vlanPort.vlan_port_id, vlanPort.flags,
                              vpPort, vpMod));
                
                /* not adding a vlan, keep the packet untagged when egress the
                 * provider port (ie. don't add the nativevid)
                 */
                if (vpMod == localMod && vpPort == port &&
                    (!(vlanPort.flags & BCM_VLAN_PORT_INNER_VLAN_PRESERVE) ||
                     !(vlanPort.flags & BCM_VLAN_PORT_INNER_VLAN_ADD)))
                {
                    *keepUntagged = 1;
                    rv = BCM_E_NONE;
                }
            }
        }

        pCur = pCur->next;
    }

    VSWITCH_LOCK_RELEASE;

    return rv;
}

/*
 *  Function
 *    _bcm_fe2000_vswitch_gport_get
 *  Purpose
 *    Gets a matching gport from a port,VSI
 *  Arguments
 *    int unit = the unit on which to operate
 *    bcm_port_t port = the matching port
 *    bcm_vlan_t vsi = the matching VSI
 *    bcm_gport_t gport = the return gport
 *  Returns
 *    bcm_error_t cast as int
 *      BCM_E_NONE for success
 *      BCM_E_* as appropriate otherwise
 *  Notes
 *    only finds the first matching gport.
 *    there could be more than one match.
 */
int
_bcm_fe2000_vswitch_port_gport_get(int unit,
                                   bcm_port_t  port,
                                   bcm_vlan_t  vsi,
                                   bcm_gport_t *gport)
{
    int rv = BCM_E_NOT_FOUND;

    int             tmpRv;
    bcm_vlan_port_t vlanPort;
    bcm_module_t    localMod;
    bcm_module_t    vpMod;
    bcm_port_t      vpPort;
    uint32          tempVsi = (vsi & 0xFFFF);

    _bcm_fe2000_vswitch_port_t *listCurr = NULL;


    VSWITCH_VERB((VSWITCH_FUNCMSG("(%d,0x%X,0x%04X) enter\n"),
                  unit, port, vsi));

    BCM_IF_ERROR_RETURN(bcm_stk_my_modid_get(unit, &localMod));

    VSWITCH_LOCK_TAKE;
   listCurr = _vswitch_state[unit]->portList[tempVsi];

    /* for all gports in the list for this VSI, find a matching port */
    do {
        if (listCurr) {
            if (BCM_GPORT_IS_VLAN_PORT(listCurr->gport)) {
                sal_memset(&vlanPort, 0, sizeof(bcm_vlan_port_t));
                vlanPort.vlan_port_id = listCurr->gport;
                tmpRv = bcm_vlan_port_find(unit, &vlanPort);

                if (BCM_SUCCESS(tmpRv)) {
                    vpMod  = BCM_GPORT_MODPORT_MODID_GET(vlanPort.port);
                    vpPort = BCM_GPORT_MODPORT_PORT_GET(vlanPort.port);

                    VSWITCH_VERB((VSWITCH_FUNCMSG(" gp=0x%08x flags=0x%08x "
                                                  "port=%d mod=%d\n"),
                                  vlanPort.vlan_port_id, vlanPort.flags,
                                  vpPort, vpMod));

                    if ((port == vpPort) && (localMod == vpMod)) {
                        *gport = listCurr->gport;
                        rv = BCM_E_NONE;
                    }
                }
            listCurr = listCurr->next;
            }
        }
    } while (listCurr && (BCM_E_NOT_FOUND == rv));

    VSWITCH_LOCK_RELEASE;

    return rv;
}
