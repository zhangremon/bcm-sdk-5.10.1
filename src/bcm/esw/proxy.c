/*
 * $Id: proxy.c 1.59.6.1 Broadcom SDK $
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
 */

#include <soc/mem.h>
#include <soc/drv.h>
#include <sal/core/sync.h>

#ifdef INCLUDE_L3

#include <bcm/error.h>
#include <bcm/proxy.h>
#include <bcm/debug.h>

#include <bcm_int/esw/port.h>

#include <bcm_int/esw_dispatch.h>
#include <bcm_int/esw/l3.h>
                            
#define _BCM_PROXY_INFO_ENTRY_CREATED   0x00000001
#define _BCM_PROXY_INFO_GROUP_CREATED   0x00000002
#define _BCM_PROXY_INFO_FILTER_CREATED  0x00000004
#define _BCM_PROXY_INFO_INSTALLED       0x00000008
#define _BCM_PROXY_INFO_SERVER          0x00000010

#define _LOCK(unit) _bcm_proxy_control[unit].proxy_lock

#define _BCM_PROXY_LOCK(unit) \
   _LOCK(unit) ? sal_mutex_take(_LOCK(unit), sal_mutex_FOREVER) : -1

#define _BCM_PROXY_UNLOCK(unit) sal_mutex_give(_LOCK(unit))


typedef struct _bcm_proxy_info_s {
    int                      flags; /* _BCM_PROXY_INFO_* */
    bcm_port_t               client_port;
    bcm_proxy_proto_type_t   proto_type;
    bcm_module_t             server_modid;
    bcm_port_t               server_port;    
    bcm_proxy_mode_t         mode;
    bcm_filterid_t           fid;
    bcm_field_entry_t        eid;
    bcm_field_group_t        gid;
    struct _bcm_proxy_info_s *next;
} _bcm_proxy_info_t;

typedef struct _bcm_proxy_control_s {
    sal_mutex_t       proxy_lock;
    _bcm_proxy_info_t *proxy_list;
    int               num_clients;
} _bcm_proxy_control_t;

typedef int (*proxy_install_method_t)(int unit,
                                      _bcm_proxy_info_t *dst,
                                      _bcm_proxy_info_t *src);

typedef int (*proxy_uninstall_method_t)(int unit,
                                        _bcm_proxy_info_t *src);

typedef int (*proxy_match_method_t)(_bcm_proxy_info_t *dst,
                                    _bcm_proxy_info_t *src);

typedef struct {
    proxy_install_method_t      install;
    proxy_uninstall_method_t    uninstall;
    proxy_match_method_t        match;
} _bcm_proxy_ifc_t;

STATIC int
_bcm_esw_proxy_client_install_xgs3(int unit, _bcm_proxy_info_t *info);

STATIC int
_bcm_esw_proxy_client_install_xgs(int unit, _bcm_proxy_info_t *info);

STATIC int
_bcm_esw_proxy_client_install(int unit,
                              _bcm_proxy_info_t *dst,
                              _bcm_proxy_info_t *src);

STATIC int
_bcm_esw_proxy_client_uninstall_xgs3(int unit, _bcm_proxy_info_t *info);

STATIC int
_bcm_esw_proxy_client_uninstall_xgs(int unit, _bcm_proxy_info_t *info);

STATIC int
_bcm_esw_proxy_client_uninstall(int unit, _bcm_proxy_info_t *info);

STATIC int
_bcm_esw_proxy_client_match(_bcm_proxy_info_t *dst, _bcm_proxy_info_t *src);

STATIC int
_bcm_esw_proxy_server_install(int unit,
                              _bcm_proxy_info_t *dst,
                              _bcm_proxy_info_t *src);

STATIC int
_bcm_esw_proxy_server_uninstall(int unit,
                                _bcm_proxy_info_t *src);

STATIC int
_bcm_esw_proxy_server_match(_bcm_proxy_info_t *dst, _bcm_proxy_info_t *src);

static _bcm_proxy_control_t _bcm_proxy_control[BCM_MAX_NUM_UNITS];

STATIC 
_bcm_proxy_ifc_t client_methods = {
    _bcm_esw_proxy_client_install,
    _bcm_esw_proxy_client_uninstall,
    _bcm_esw_proxy_client_match,
};

STATIC 
_bcm_proxy_ifc_t server_methods = {
    _bcm_esw_proxy_server_install,
    _bcm_esw_proxy_server_uninstall,
    _bcm_esw_proxy_server_match,
};

/***************************************************************** Utilities */

/*
 * Function:
 *      _bcm_esw_proxy_gport_resolve
 * Purpose:
 *      Decodes gport into port and module id
 * Parameters:
 *      unit         -  BCM Unit number
 *      gport        - GPORT 
 *      port_out     - (OUT) port encoded into gport 
 *      modid_out    - (OUT) modid encoded into gport
 *      isLocal      - Indicator that port encoded in gport must be local
 * Returns:
 *      BCM_E_XXX
 */
STATIC int 
_bcm_esw_proxy_gport_resolve(int unit, bcm_gport_t gport,
                             bcm_port_t *port_out,
                             bcm_module_t *modid_out, int isLocal)
{

    bcm_port_t      port; 
    bcm_module_t    modid;
    bcm_trunk_t     tgid;
    int             id;

    if (NULL == port_out || NULL == modid_out) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(
        _bcm_esw_gport_resolve(unit, gport, &modid, &port, &tgid, &id));

    if ((-1 != id) || (BCM_TRUNK_INVALID != tgid)){
        return BCM_E_PORT;
    }

    if (isLocal) {
        int    ismymodid;
        BCM_IF_ERROR_RETURN(
            _bcm_esw_modid_is_local(unit, modid, &ismymodid));
        if (ismymodid != TRUE) {
            return BCM_E_PARAM;
        }
    } 

    *port_out = port;
    *modid_out = modid;
    
    return BCM_E_NONE;
}

/****************************************************************** Generics */

/*
 * Function:
 *      _bcm_esw_proxy_install
 * Purpose:
 *      Install proxy client or server
 * Parameters:
 *      unit           -  BCM Unit number
 *      bcm_proxy_ifc  -  local port for which redirection is applied
 *      data           -  Packet type to classify for redirection
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_esw_proxy_install(int unit,
                       _bcm_proxy_ifc_t *ifc,
                       _bcm_proxy_info_t *src)
{
    int rv = BCM_E_INTERNAL;
    _bcm_proxy_info_t *dst;

    /* Allocate proxy info structure */
    dst = sal_alloc(sizeof(_bcm_proxy_info_t), "bcm_esw_proxy");

    if (dst == NULL) {
        return BCM_E_MEMORY;
    }

    sal_memset(dst, 0, sizeof(_bcm_proxy_info_t));

    if (_BCM_PROXY_LOCK(unit)) {
        sal_free(dst);
        return BCM_E_INIT;
    }

    rv = ifc->install(unit, dst, src);
        
    if (BCM_SUCCESS(rv)) {
        /* Put onto list */
        dst->next = _bcm_proxy_control[unit].proxy_list;
        _bcm_proxy_control[unit].proxy_list = dst;
    } else {
        sal_free(dst);
    }

    _BCM_PROXY_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *      _bcm_esw_proxy_uninstall
 * Purpose:
 *      Uninstall proxy client or server
 * Parameters:
 *      unit           -  BCM Unit number
 *      ifc            -  client/server methods
 *      info           -  info record to uninstall
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_esw_proxy_uninstall(int unit,
                         _bcm_proxy_ifc_t *ifc,
                         _bcm_proxy_info_t *src)
{
    int rv = BCM_E_INTERNAL;
    _bcm_proxy_info_t *dst;

    if (_BCM_PROXY_LOCK(unit)) {
        return BCM_E_INIT;
    }

    /* Uninstall the enabled proxy client if the hardware is still
       capable. */
    if (SOC_HW_ACCESS_DISABLE(unit) == 0) {
        rv = ifc->uninstall(unit, src);
    } else {
        rv = BCM_E_NONE;
    }
    /* Unlink from list */
    if (_bcm_proxy_control[unit].proxy_list == src) {
        /* Info at beginning of the list */
        _bcm_proxy_control[unit].proxy_list = src->next;
    } else {
        for (dst = _bcm_proxy_control[unit].proxy_list;
             dst != NULL;
             dst = dst->next) {
            if (dst->next == src) {
                dst->next = src->next;
                break;
            }
        }
    }
    _BCM_PROXY_UNLOCK(unit);

    sal_memset(src, 0, sizeof(_bcm_proxy_info_t));
    sal_free(src);

    return rv;
}

/*
 * Function:
 *      _bcm_esw_proxy_find
 * Purpose:
 *      Find a previously created proxy client or server
 * Parameters:
 *      unit         -  BCM Unit number
 *      ifc          -  interface methods
 *      src          -  data to find
 *      dstp         -  data found, or NULL
 * Returns:
 *      BCM_E_NONE     - found
 *      BCM_E_NOTFOUND - not found
 *      BCM_E_INIT     - subsystem not initialized
 */

STATIC int
_bcm_esw_proxy_find(int unit,
                    _bcm_proxy_ifc_t *ifc,
                    _bcm_proxy_info_t *src,
                    _bcm_proxy_info_t **dstp)
{
    _bcm_proxy_info_t *dst;
    int rv = BCM_E_NOT_FOUND;

    if (_BCM_PROXY_LOCK(unit)) {
        return BCM_E_INIT;
    }

    for (dst = _bcm_proxy_control[unit].proxy_list;
         dst != NULL;
         dst = dst->next) {
        if (ifc->match(dst, src)) {
            rv = BCM_E_EXISTS;
            break;
        }
    }
    _BCM_PROXY_UNLOCK(unit);
    *dstp = dst;

    return rv;
}

/*
 * Function:
 *      _bcm_esw_proxy
 * Purpose:
 *      Generic proxy client or server install or uninstall
 * Parameters:
 *      unit         -  BCM Unit number
 *      ifc          -  interface methods
 *      src          -  data to find
 * Returns:
 *      Pointer to _bcm_proxy_info_t if found
 *      NULL if not found
 */

STATIC int
_bcm_esw_proxy(int unit, _bcm_proxy_ifc_t *ifc,
               _bcm_proxy_info_t *src, int enable)
{
    int rv;
    _bcm_proxy_info_t *dst = NULL;

    rv = _bcm_esw_proxy_find(unit, ifc, src, &dst);

    if (enable) {

        if (rv != BCM_E_NOT_FOUND) {
            return rv;
        }

        rv = _bcm_esw_proxy_install(unit, ifc, src);

    } else {
        /* Disable */

        if (rv != BCM_E_EXISTS) {
            return rv;
        }
        
        rv = _bcm_esw_proxy_uninstall(unit, ifc, dst);

    }

    return rv;
}

/******************************************************************** Client */

/*
 * Function:
 *      _bcm_proxy_client_enabled
 * Purpose:
 *      Returns true if the proxy client is enabled on the given unit
 * Parameters:
 *      unit         -  BCM Unit number
 * Returns:
 *      TRUE or FALSE
 */

int
_bcm_proxy_client_enabled(int unit)
{
    return (_bcm_proxy_control[unit].num_clients > 0);
}

/*
 * Function:
 *      bcm_esw_proxy_client_set
 * Purpose:
 *      Enables redirection for a certain traffic type using either 
 *      FFP or FP rule
 * Parameters:
 *      unit         -  BCM Unit number
 *      client_port  -  local port for which redirection is applied
 *      proto_type   -  Packet type to classify for redirection
 *      server_modid -  Module ID of remote device which performs lookups
 *      server_port  -  Port on remote device where redirected packets are 
 *                      destined to
 *      enable       -  toggle to enable or disable redirection
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_proxy_client_set(int unit, bcm_port_t client_port, 
                         bcm_proxy_proto_type_t proto_type,
                         bcm_module_t server_modid, bcm_port_t server_port, 
                         int enable)
{
    _bcm_proxy_info_t src;

    if (BCM_GPORT_IS_SET(client_port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, client_port, &client_port));
    }
    if (BCM_GPORT_IS_SET(server_port)) {
        BCM_IF_ERROR_RETURN(
            _bcm_esw_proxy_gport_resolve(unit, server_port, &server_port,
                                         &server_modid, 0));
    }

    if (!SOC_PORT_VALID(unit, client_port)) {
        return BCM_E_PORT;
    }
    if (server_port < 0 ) {
        return BCM_E_PORT;
    }

    sal_memset(&src, 0, sizeof(_bcm_proxy_info_t));
    src.client_port  = client_port;
    src.proto_type   = proto_type;
    src.server_modid = server_modid;
    src.server_port  = server_port;

    return _bcm_esw_proxy(unit, &client_methods, &src, enable);
}

/*
 * Function:
 *      _bcm_esw_proxy_client_match
 * Purpose:
 *      Return true if src client record matches dst
 * Parameters:
 *      dst - 
 *      src - 
 * Returns:
 *      True if matched
 */

STATIC int
_bcm_esw_proxy_client_match(_bcm_proxy_info_t *dst, _bcm_proxy_info_t *src)
{
        return (dst->client_port == src->client_port &&
                dst->proto_type == src->proto_type &&
                dst->server_modid == src->server_modid &&
                dst->server_port == src->server_port &&
                (dst->flags & _BCM_PROXY_INFO_SERVER) == 0);
}

/*
 * Function:
 *      _bcm_esw_proxy_server_match
 * Purpose:
 *      Return true if src server record matches dst
 * Parameters:
 *      dst - 
 *      src - 
 * Returns:
 *      True if matched
 */

STATIC int
_bcm_esw_proxy_server_match(_bcm_proxy_info_t *dst,
                            _bcm_proxy_info_t *src)
{
    return (dst->mode == src->mode &&
            dst->server_port == src->server_port &&
            (dst->flags & _BCM_PROXY_INFO_SERVER) != 0);
}

/*
 * Function:
 *      _bcm_esw_proxy_client_create_xgs3
 * Purpose:
 *      Creates proxy client using FP rules, doesn't clean up on error
 * Parameters:
 *      unit           -  BCM Unit number
 *      bcm_proxy_info -  proxy data
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_bcm_esw_proxy_client_create_xgs3(int unit, _bcm_proxy_info_t *bcm_proxy_info)
{
#ifdef BCM_XGS3_SWITCH_SUPPORT
    bcm_field_entry_t      eid;
    bcm_field_group_t      gid;
    bcm_field_qset_t       qset;
    bcm_pbmp_t             ingress_pbmp;
    bcm_port_t             client_port = bcm_proxy_info->client_port;
    bcm_proxy_proto_type_t proto_type = bcm_proxy_info->proto_type;
    bcm_module_t           server_modid = bcm_proxy_info->server_modid;
    bcm_port_t             server_port = bcm_proxy_info->server_port;

    BCM_FIELD_QSET_INIT(qset);
    BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyInPorts);
    BCM_PBMP_PORT_SET(ingress_pbmp, client_port);
    
    switch(proto_type) {
    case BCM_PROXY_PROTO_IP4_ALL:
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyEtherType);
        break;
    case BCM_PROXY_PROTO_IP6_ALL:
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyEtherType);
        break;
    case BCM_PROXY_PROTO_IP4_MCAST:
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyDstIp);
        break;
    case BCM_PROXY_PROTO_IP6_MCAST:
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyIpType);
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyDstIp6High);
        break;
    case BCM_PROXY_PROTO_MPLS_UCAST:
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyEtherType);
        break;
    case BCM_PROXY_PROTO_MPLS_MCAST:
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyEtherType);
        break;
    case BCM_PROXY_PROTO_MPLS_ALL:
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyEtherType);
        break;
    case BCM_PROXY_PROTO_IP6_IN_IP4:
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyIpProtocol);
        break;
    case BCM_PROXY_PROTO_IP_IN_IP:
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyIpProtocol);
        break;
    case BCM_PROXY_PROTO_GRE_IN_IP:
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyIpProtocol);
        break;
    case BCM_PROXY_PROTO_UNKNOWN_IP4_UCAST:
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyEtherType);
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyPacketRes);
        break;
    /*    coverity[equality_cond]    */
    case BCM_PROXY_PROTO_UNKNOWN_IP6_UCAST:
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyEtherType);
        BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyPacketRes);
        break;
    default:
        return BCM_E_PARAM;
    }
    BCM_IF_ERROR_RETURN(bcm_esw_field_group_create(unit, qset,
                                                   BCM_FIELD_GROUP_PRIO_ANY,
                                                   &gid));
    bcm_proxy_info->flags |= _BCM_PROXY_INFO_GROUP_CREATED;
    bcm_proxy_info->gid = gid;

    BCM_IF_ERROR_RETURN(bcm_esw_field_entry_create(unit, gid, &eid));
    bcm_proxy_info->flags |= _BCM_PROXY_INFO_ENTRY_CREATED;
    bcm_proxy_info->eid = eid;

    BCM_IF_ERROR_RETURN
        (bcm_esw_field_qualify_InPorts(unit, eid, ingress_pbmp,
                                       PBMP_ALL(unit))); 

    switch(proto_type) {
    case BCM_PROXY_PROTO_IP4_ALL:
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_qualify_EtherType(unit, eid, 0x0800, 0xffff));
        break;
    case BCM_PROXY_PROTO_IP6_ALL:
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_qualify_EtherType(unit, eid, 0x86dd, 0xffff));
        break;
    case BCM_PROXY_PROTO_IP4_MCAST:
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_qualify_DstIp(unit, eid, 0xe0000000, 0xf0000000));
        break;
    case BCM_PROXY_PROTO_IP6_MCAST:
        {
            bcm_ip6_t addr, mask;
            BCM_IF_ERROR_RETURN
                (bcm_esw_field_qualify_IpType(unit, eid, bcmFieldIpTypeIpv6));
            sal_memset(addr, 0, 16); /* BCM_IP6_ADDRLEN */
            addr[0] = 0xff;
            sal_memset(mask, 0, 16); /* BCM_IP6_ADDRLEN */
            mask[0] = 0xff;
            mask[1] = 0xff;
            BCM_IF_ERROR_RETURN
                (bcm_esw_field_qualify_DstIp6High(unit, eid, addr, mask));
            break;
        }

    case BCM_PROXY_PROTO_MPLS_UCAST:
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_qualify_EtherType(unit, eid, 0x8847, 0xffff));
        break;
    case BCM_PROXY_PROTO_MPLS_MCAST:
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_qualify_EtherType(unit, eid, 0x8848, 0xffff));
        break;
    case BCM_PROXY_PROTO_MPLS_ALL:
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_qualify_EtherType(unit, eid, 0x8840, 0xfff0));
        break;
    case BCM_PROXY_PROTO_IP6_IN_IP4:
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_qualify_IpProtocol(unit, eid, 0x29, 0xff));
        break;
    case BCM_PROXY_PROTO_IP_IN_IP:
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_qualify_IpProtocol(unit, eid, 0x4, 0xff));
        break;
    case BCM_PROXY_PROTO_GRE_IN_IP:
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_qualify_IpProtocol(unit, eid, 0x2f, 0xff));
        break;
    case BCM_PROXY_PROTO_UNKNOWN_IP4_UCAST:
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_qualify_EtherType(unit, eid, 0x0800, 0xffff));
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_qualify_PacketRes(unit, eid,
                                             BCM_FIELD_PKT_RES_L3UCUNKNOWN,
                                             0xf));
        break;
    case BCM_PROXY_PROTO_UNKNOWN_IP6_UCAST:
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_qualify_EtherType(unit, eid, 0x86dd, 0xffff));
        BCM_IF_ERROR_RETURN
            (bcm_esw_field_qualify_PacketRes(unit, eid,
                                             BCM_FIELD_PKT_RES_L3UCUNKNOWN,
                                             0xf));
        break;
        /* Defensive Default */
        /* coverity[dead_error_begin] */
    default:
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN
        (bcm_esw_field_action_add(unit, eid, bcmFieldActionRedirect, 
                                  server_modid, server_port));
    
     BCM_IF_ERROR_RETURN
         (bcm_esw_field_entry_install(unit, eid));

    bcm_proxy_info->flags |= _BCM_PROXY_INFO_INSTALLED;

    return BCM_E_NONE;
#else
    return BCM_E_UNAVAIL;
#endif
} 

/*
 * Function:
 *      _bcm_esw_proxy_client_uninstall_xgs3
 * Purpose:
 *      Removes and destroys proxy client FP resources
 * Parameters:
 *      unit           -  BCM Unit number
 *      bcm_proxy_info -  proxy data
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_bcm_esw_proxy_client_uninstall_xgs3(int unit,
                                     _bcm_proxy_info_t *info)
{
#ifdef BCM_XGS3_SWITCH_SUPPORT
    bcm_field_entry_t eid   = info->eid;
    bcm_field_group_t gid   = info->gid;
    int flags               = info->flags;

    if (flags & _BCM_PROXY_INFO_INSTALLED) {
        BCM_IF_ERROR_RETURN(bcm_esw_field_entry_remove(unit, eid));
    }

    if (flags & _BCM_PROXY_INFO_ENTRY_CREATED) {
        BCM_IF_ERROR_RETURN(bcm_esw_field_entry_destroy(unit, eid));
    }

    if (flags & _BCM_PROXY_INFO_GROUP_CREATED) {
        BCM_IF_ERROR_RETURN(bcm_esw_field_group_destroy(unit, gid));
    }

    return BCM_E_NONE;
#else
    return BCM_E_UNAVAIL;
#endif
}

/*
 * Function:
 *      _bcm_esw_proxy_client_install_xgs3
 * Purpose:
 *      Creates proxy client using FP rules, clean up on error
 * Parameters:
 *      unit           -  BCM Unit number
 *      bcm_proxy_info -  proxy data
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_bcm_esw_proxy_client_install_xgs3(int unit,
                                   _bcm_proxy_info_t *info)
{
    int rv = BCM_E_INTERNAL;

    rv = _bcm_esw_proxy_client_create_xgs3(unit, info);

    if (BCM_FAILURE(rv)) {
        L3_WARN(("Proxy: could not install client: %s\n", bcm_errmsg(rv)));
        /* Ignore errors from uninstall */
        _bcm_esw_proxy_client_uninstall_xgs3(unit, info);
    }

    return rv;
}

/*
 * Function:
 *      _bcm_esw_proxy_client_create_xgs
 * Purpose:
 *      Creates proxy client using FFP rules, doesn't cleanup on error
 * Parameters:
 *      unit           -  BCM Unit number
 *      bcm_proxy_info -  proxy data
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_bcm_esw_proxy_client_create_xgs(int unit, _bcm_proxy_info_t *bcm_proxy_info)
{
    /* Use FFP for matching */
    bcm_pbmp_t             pbm;
    bcm_filterid_t         bf;
    bcm_port_t             client_port = bcm_proxy_info->client_port;
    bcm_proxy_proto_type_t proto_type = bcm_proxy_info->proto_type;
    bcm_module_t           server_modid = bcm_proxy_info->server_modid;
    bcm_port_t             server_port = bcm_proxy_info->server_port;

    BCM_IF_ERROR_RETURN(bcm_esw_filter_create(unit, &bf));
    bcm_proxy_info->flags |= _BCM_PROXY_INFO_FILTER_CREATED;

    BCM_PBMP_CLEAR(pbm);
    BCM_PBMP_PORT_SET(pbm, client_port);
    BCM_IF_ERROR_RETURN(bcm_esw_filter_qualify_ingress(unit, bf, pbm));    

    switch (proto_type) {
    case BCM_PROXY_PROTO_IP4_ALL:
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_qualify_format(unit, bf, BCM_FILTER_PKTFMT_IPV4));
        break;
    case BCM_PROXY_PROTO_IP6_ALL:
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_qualify_format(unit, bf, BCM_FILTER_PKTFMT_IPV6));
        break;
    case BCM_PROXY_PROTO_IP4_MCAST:
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_qualify_format(unit, bf, BCM_FILTER_PKTFMT_IPV4));
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_qualify_data32(unit, bf, 34,
                                           0xe0000000, 0xf0000000));
        break;
    case BCM_PROXY_PROTO_IP6_MCAST:
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_qualify_format(unit, bf, BCM_FILTER_PKTFMT_IPV6));
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_qualify_data16(unit, bf, 46, 0xff00, 0xffff));
        break;
    case BCM_PROXY_PROTO_MPLS_UCAST:
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_qualify_data16(unit, bf, 16, 0x8847, 0xffff));
        break;
    case BCM_PROXY_PROTO_MPLS_MCAST:
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_qualify_data16(unit, bf, 16, 0x8848, 0xffff));
        break;
    case BCM_PROXY_PROTO_IP6_IN_IP4:
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_qualify_data8(unit, bf, 27, 0x29, 0xff));
        break;
    case BCM_PROXY_PROTO_IP_IN_IP:
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_qualify_data8(unit, bf, 27, 0x4, 0xff));
        break;
    case BCM_PROXY_PROTO_GRE_IN_IP:
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_qualify_data8(unit, bf, 27, 0x2f, 0xff));
        break;
    case BCM_PROXY_PROTO_UNKNOWN_IP4_UCAST:
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_qualify_format(unit, bf, BCM_FILTER_PKTFMT_IPV4));
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_qualify_unknown_ucast(unit, bf));
        break;
    case BCM_PROXY_PROTO_UNKNOWN_IP6_UCAST:
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_qualify_format(unit, bf, BCM_FILTER_PKTFMT_IPV6));
        BCM_IF_ERROR_RETURN
            (bcm_esw_filter_qualify_unknown_ucast(unit, bf));
        break;
    default:
        return BCM_E_PARAM;

    }

    BCM_IF_ERROR_RETURN
        (bcm_esw_filter_action_match(unit, bf, bcmActionSetPortAll,
                                     server_port));
    BCM_IF_ERROR_RETURN
        (bcm_esw_filter_action_match(unit, bf, bcmActionSetModule,
                                     server_modid));

    BCM_IF_ERROR_RETURN
        (bcm_esw_filter_install(unit, bf));

    bcm_proxy_info->flags |= _BCM_PROXY_INFO_INSTALLED;
    bcm_proxy_info->fid = bf;

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_esw_proxy_client_uninstall_xgs
 * Purpose:
 *      Removes and destroys proxy client FFP resources
 * Parameters:
 *      unit           -  BCM Unit number
 *      bcm_proxy_info -  proxy data
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_bcm_esw_proxy_client_uninstall_xgs(int unit,
                                    _bcm_proxy_info_t *info)
{
    int fid   = info->fid;
    int flags = info->flags;

    if (flags & _BCM_PROXY_INFO_INSTALLED) {
        BCM_IF_ERROR_RETURN(bcm_esw_filter_remove(unit, fid));
    }

    if (flags & _BCM_PROXY_INFO_FILTER_CREATED) {
        BCM_IF_ERROR_RETURN(bcm_esw_filter_destroy(unit, fid));
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_esw_proxy_client_install_xgs
 * Purpose:
 *      Creates proxy client using FFP rules, cleanup on error
 * Parameters:
 *      unit           -  BCM Unit number
 *      bcm_proxy_info -  proxy data
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_bcm_esw_proxy_client_install_xgs(int unit, _bcm_proxy_info_t *info)
{
    int rv = BCM_E_INTERNAL;

    rv = _bcm_esw_proxy_client_create_xgs(unit, info);

    if (BCM_FAILURE(rv)) {
        L3_WARN(("Proxy: could not install client: %s\n", bcm_errmsg(rv)));
        /* Ignore errors from uninstall */
        _bcm_esw_proxy_client_uninstall_xgs(unit, info);
    }

    return rv;
}

/*
 * Function:
 *      _bcm_esw_proxy_client_uninstall
 * Purpose:
 *      Uninstall proxy client for all device families
 * Parameters:
 *      unit           -  BCM Unit number
 *      bcm_proxy_info -  proxy data
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_bcm_esw_proxy_client_uninstall(int unit, _bcm_proxy_info_t *info)
{
    int rv = BCM_E_INTERNAL;

    if (SOC_IS_XGS3_SWITCH(unit)) {
        rv = _bcm_esw_proxy_client_uninstall_xgs3(unit, info);
    } else {
        rv = _bcm_esw_proxy_client_uninstall_xgs(unit, info);
    }

    if (BCM_SUCCESS(rv)) {
        _bcm_proxy_control[unit].num_clients--;
    }

    return rv;
}


/*
 * Function:
 *      _bcm_esw_proxy_client_install
 * Purpose:
 *      Install proxy client for all device families
 * Parameters:
 *      unit           -  BCM Unit number
 *      bcm_proxy_info -  proxy data
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_bcm_esw_proxy_client_install(int unit,
                              _bcm_proxy_info_t *dst,
                              _bcm_proxy_info_t *src)
{
    int rv = BCM_E_INTERNAL;

    dst->client_port  = src->client_port;
    dst->proto_type   = src->proto_type;
    dst->server_modid = src->server_modid;
    dst->server_port  = src->server_port;
    dst->server_port  = src->server_port;

    if (SOC_IS_XGS3_SWITCH(unit)) {
        rv = _bcm_esw_proxy_client_install_xgs3(unit, dst);
    } else {
        rv = _bcm_esw_proxy_client_install_xgs(unit, dst);
    }

    if (BCM_SUCCESS(rv)) {
        _bcm_proxy_control[unit].num_clients++;
    }
    return rv;
}

/******************************************************************** Server */

/*
 * Function:
 *      _bcm_esw_proxy_server_set
 * Purpose:
 *      Enables various kinds of lookups for packets coming from remote
 *      (proxy client) devices (internal function)
 * Parameters:
 *      unit        -  BCM Unit number
 *      server_port -  Local port to which packets from remote devices are 
 *                     destined to
 *      mode        -  Indicates lookup type
 *      enable      -  TRUE to enable lookups
 *                     FALSE to disable lookups
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_esw_proxy_server_set(int unit, bcm_port_t server_port, 
                          bcm_proxy_mode_t mode, int enable)
{
    
    if (BCM_GPORT_IS_SET(server_port)) {
        bcm_module_t    modid;
        BCM_IF_ERROR_RETURN(
            _bcm_esw_proxy_gport_resolve(unit, server_port,
                                         &server_port, &modid, 1));
    }
    
    if (IS_HG_PORT(unit, server_port)) {

	if (mode != BCM_PROXY_MODE_HIGIG) {
	    return BCM_E_PARAM;
        }
           
      
#ifdef BCM_XGS3_SWITCH_SUPPORT
        if (SOC_IS_FBX(unit)) {
            uint32 dst_bmp;
            bcm_pbmp_t pbmp;
            uint32 val, mod;
            int lport_idx;
            port_tab_entry_t ptab;
            
            if (!soc_feature(unit, soc_feature_higig_lookup)) {
                return BCM_E_UNAVAIL;
            }

            /*
             * Program PORT table V4L3_ENABLE, V6L3_ENABLE;
             * required for IP-IP tunneling on XGS3 devices.
             * This is not necessary for XGS4 and later.
             */

            
            if (!SOC_IS_TRX(unit)) {
                BCM_IF_ERROR_RETURN
                    (soc_mem_read(unit, PORT_TABm, MEM_BLOCK_ANY, 
                                  server_port, &ptab));
                soc_PORT_TABm_field32_set(unit, &ptab, V4L3_ENABLEf, 
                                          enable ? 1 : 0);
                soc_PORT_TABm_field32_set(unit, &ptab, V6L3_ENABLEf,
                                          enable ? 1 : 0);
                BCM_IF_ERROR_RETURN
                    (soc_mem_write(unit, PORT_TABm, MEM_BLOCK_ALL, 
                                   server_port, &ptab));
            }

            /*
             * Program LPORT table for the HG port;
             * HIGIG_PACKET must not be set; MY_MODID must be
             * set appropriately, and V4/V6 enable bits
             */
            BCM_IF_ERROR_RETURN
                (soc_mem_read(unit, PORT_TABm, MEM_BLOCK_ANY, 
                              server_port, &ptab));
            mod = soc_PORT_TABm_field32_get(unit, &ptab, MY_MODIDf);
            sal_memset(&ptab, 0, sizeof(port_tab_entry_t));
            soc_LPORT_TABm_field32_set(unit, &ptab, MY_MODIDf, mod);
            soc_LPORT_TABm_field32_set(unit, &ptab, V4L3_ENABLEf, 
                                       enable ? 1 : 0);
            soc_LPORT_TABm_field32_set(unit, &ptab, V6L3_ENABLEf,
                                       enable ? 1 : 0);
            soc_LPORT_TABm_field32_set(unit, &ptab, V4IPMC_ENABLEf,
                                       enable ? 1 : 0);
            soc_LPORT_TABm_field32_set(unit, &ptab, V6IPMC_ENABLEf, 
                                       enable ? 1 : 0);
            soc_LPORT_TABm_field32_set(unit, &ptab, IPMC_DO_VLANf,
                                       enable ? 1 : 0);
            soc_LPORT_TABm_field32_set(unit, &ptab, FILTER_ENABLEf,
                                       enable ? 1 : 0);

            /* For Triumph, we use entry 0 so all remote ports
               will use this entry.  This is the behavior for previous
               devices */
            lport_idx = SOC_IS_TR_VL(unit) ? 0 : server_port;
            BCM_IF_ERROR_RETURN
                (soc_mem_write(unit, LPORT_TABm, MEM_BLOCK_ALL, 
                               lport_idx, &ptab));
                    
            SOC_IF_ERROR_RETURN(READ_IHG_LOOKUPr
                                (unit, server_port, &val));
            soc_reg_field_set(unit, IHG_LOOKUPr, &val, 
                              HG_LOOKUP_ENABLEf, enable ? 1: 0);
            if (SOC_IS_TRX(unit)) {
                uint32 hgld;
                /* Set "magic destination" to this server port to
                   match prior devices' behavior */
                SOC_IF_ERROR_RETURN
                    (READ_HG_LOOKUP_DESTINATIONr(unit, &hgld));
                soc_reg_field_set(unit, HG_LOOKUP_DESTINATIONr, &hgld, 
                                  DST_MODIDf, mod);
                soc_reg_field_set(unit, HG_LOOKUP_DESTINATIONr, &hgld, 
                                  DST_PORTf, server_port);
                SOC_IF_ERROR_RETURN
                    (WRITE_HG_LOOKUP_DESTINATIONr(unit, hgld));
            } else {
                BCM_PBMP_PORT_SET(pbmp, server_port);
                dst_bmp = SOC_PBMP_WORD_GET(pbmp, 0);
                dst_bmp >>= SOC_HG_OFFSET(unit);
                soc_reg_field_set(unit, IHG_LOOKUPr, &val, 
                                  DST_HG_LOOKUP_BITMAPf, dst_bmp);
            }
            if (SOC_REG_FIELD_VALID(unit, IHG_LOOKUPr,
                                    LOOKUP_WITH_MH_SRC_PORTf)) {
                soc_reg_field_set(unit, IHG_LOOKUPr, &val, 
                                  LOOKUP_WITH_MH_SRC_PORTf, enable ? 1 :0);
            }
            if (SOC_REG_FIELD_VALID(unit, IHG_LOOKUPr,
                                    USE_MH_INTERNAL_PRIf)) {
                soc_reg_field_set(unit, IHG_LOOKUPr, &val, 
                                  USE_MH_INTERNAL_PRIf, enable ? 1: 0);
            }
            soc_reg_field_set(unit, IHG_LOOKUPr, &val, USE_MH_VIDf, 
                              enable ? 1: 0);
            soc_reg_field_set(unit, IHG_LOOKUPr, &val, USE_MH_PKT_PRIf,
                              enable ? 1: 0);
            SOC_IF_ERROR_RETURN(WRITE_IHG_LOOKUPr(unit, server_port, val));
	} else if (SOC_IS_EASYRIDER(unit)) {
#if defined(BCM_EASYRIDER_SUPPORT)
	    bcm_module_t modid;
            uint32 modid_cfg_val;
            uint32 val;
            /* Note:  server_port parameter is ignored for BCM5660x devices
             * because they only have one HG port. */
	    /*
	     * Program EGR_NEW_MODID_PORT reg to overwrite Source modid
             * for packets that are looked up here and going back to
	     * remote chip
	     */
            SOC_IF_ERROR_RETURN(READ_E2E_MODULE_CONFIGr(unit, &modid_cfg_val));
            modid = soc_reg_field_get(unit, E2E_MODULE_CONFIGr, 
                                      modid_cfg_val, MY_MODULE_IDf);

	    SOC_IF_ERROR_RETURN(READ_EGR_NEW_MODID_PORTr(unit, &val));
            soc_reg_field_set(unit, EGR_NEW_MODID_PORTr, &val,
                              ENABLEf, enable ? 1: 0);
	    soc_reg_field_set(unit, EGR_NEW_MODID_PORTr, &val,
                              INCLUDE_L2f, enable ? 1: 0);
	    soc_reg_field_set(unit, EGR_NEW_MODID_PORTr, &val,
                              SRC_MODIDf, modid);
	    SOC_IF_ERROR_RETURN(WRITE_EGR_NEW_MODID_PORTr(unit, val));

            SOC_IF_ERROR_RETURN(READ_SEER_CONFIGr(unit, &val));
            soc_reg_field_set(unit, SEER_CONFIGr, &val, 
                              HG_DA_LOOKUP_ENABLEf, enable ? 1: 0);
            SOC_IF_ERROR_RETURN(WRITE_SEER_CONFIGr(unit, val));

            if (SOC_REG_IS_VALID(unit, SEER_HG_L2_LOOKUP_CONFIGr)) {
	        SOC_IF_ERROR_RETURN
                    (READ_SEER_HG_L2_LOOKUP_CONFIGr(unit, &val));
	        soc_reg_field_set(unit, SEER_HG_L2_LOOKUP_CONFIGr, &val, 
                                  HG_L2_LOOKUP_ENABLEf, enable ? 1: 0);
	        SOC_IF_ERROR_RETURN
                    (WRITE_SEER_HG_L2_LOOKUP_CONFIGr(unit, val));
            }
            if (SOC_REG_FIELD_VALID(unit, DEF_VLAN_CONTROLr, QINQ_PROXYf)) {
	        SOC_IF_ERROR_RETURN(READ_DEF_VLAN_CONTROLr(unit, &val));
	        soc_reg_field_set(unit, DEF_VLAN_CONTROLr, &val, 
                                  QINQ_PROXYf, enable ? 1: 0);
	        SOC_IF_ERROR_RETURN(WRITE_DEF_VLAN_CONTROLr(unit, val));
            }
#endif /* BCM_EASYRIDER_SUPPORT */
	}
#endif /* BCM_XGS3_SWITCH_SUPPORT */
     } else {

	if (mode != BCM_PROXY_MODE_LOOPBACK) {
	    return BCM_E_PARAM;
	}
        
	if (enable) {
            BCM_IF_ERROR_RETURN
                (bcm_esw_port_loopback_set(unit, server_port, 
                                           BCM_PORT_LOOPBACK_MAC));
	} else {
            BCM_IF_ERROR_RETURN
                (bcm_esw_port_loopback_set(unit, server_port, 
                                           BCM_PORT_LOOPBACK_NONE));
	}
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_esw_proxy_server_get
 * Purpose:
 *      Get the lookup mode of XGS3 device.
 * Parameters:
 *      unit        -  BCM Unit number
 *      server_port -  Local port to which packets from remote devices are 
 *                     destined to
 *      mode        -  proxy server mode
 *      enable      -  (OUT) server status
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_proxy_server_get(int unit, bcm_port_t server_port, 
                         bcm_proxy_mode_t mode, int *enable)
{
    int rv = BCM_E_NONE;
    *enable = FALSE;

    if (BCM_GPORT_IS_SET(server_port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, server_port, &server_port));
    }

    if (!IS_HG_PORT(unit, server_port)) {
        int status;

	if (mode != BCM_PROXY_MODE_LOOPBACK) {
	    return BCM_E_PARAM;
	}
    /*    coverity[uninit_use_in_call : FALSE]    */
	    
        rv = bcm_esw_port_loopback_get(unit, server_port, &status);
	if (BCM_SUCCESS(rv)) {
            if (status == BCM_PORT_LOOPBACK_MAC ||
                status == BCM_PORT_LOOPBACK_PHY) {
		*enable = TRUE;
            } 
	}
    } else {

        if (mode != BCM_PROXY_MODE_HIGIG) {
	    return BCM_E_PARAM;
        }

#ifdef BCM_XGS3_SWITCH_SUPPORT
	if (soc_feature(unit, soc_feature_higig_lookup)) {
            int port, hg_enable;
            uint32 val, dst_bmp;
            bcm_pbmp_t pbmp, r_pbmp;

            if (SOC_IS_TRX(unit)) {
                uint32 hgld;
                /* Set "magic destination" to this server port to
                   match prior devices' behavior */
                SOC_IF_ERROR_RETURN
                    (READ_HG_LOOKUP_DESTINATIONr(unit, &hgld));
                if (server_port == soc_reg_field_get(unit,
                                                    HG_LOOKUP_DESTINATIONr,
                                                    hgld,
                                                    DST_PORTf)) {
                    SOC_IF_ERROR_RETURN
                        (READ_IHG_LOOKUPr(unit, server_port, &val));
                    hg_enable = soc_reg_field_get(unit, IHG_LOOKUPr, val,
                                                  HG_LOOKUP_ENABLEf);
                    *enable = hg_enable;
                }
#if defined(BCM_EASYRIDER_SUPPORT)
            } else if (SOC_IS_EASYRIDER(unit)) {
                /* Note:  server_port parameter is ignored for BCM5660x
                 * devices because they only have one HG port. */
                SOC_IF_ERROR_RETURN(READ_EGR_NEW_MODID_PORTr(unit, &val));
                hg_enable = soc_reg_field_get(unit, EGR_NEW_MODID_PORTr,
                                              val, ENABLEf);
                *enable = hg_enable;
#endif /* BCM_EASYRIDER_SUPPORT */
            } else {
                PBMP_HG_ITER(unit, port) {
                    SOC_PBMP_CLEAR(r_pbmp);
                    BCM_PBMP_PORT_SET(pbmp, server_port);

                    SOC_IF_ERROR_RETURN(READ_IHG_LOOKUPr(unit, port, &val));
                    hg_enable = soc_reg_field_get(unit, IHG_LOOKUPr, val, 
                                                  HG_LOOKUP_ENABLEf);
                    dst_bmp = soc_reg_field_get(unit, IHG_LOOKUPr, val, 
                                                DST_HG_LOOKUP_BITMAPf);
                    dst_bmp <<= SOC_HG_OFFSET(unit);
                    SOC_PBMP_WORD_SET(r_pbmp, 0, dst_bmp);
                    BCM_PBMP_AND(pbmp, r_pbmp);

                    if (hg_enable && BCM_PBMP_EQ(pbmp, r_pbmp)) {
                        *enable = TRUE;
                        break;
                    }
                }
            }
	}
#endif
    }

    return rv;
}

/*
 * Function:
 *      _bcm_esw_proxy_server
 * Purpose:
 *      Enable/disable proxy server
 * Parameters:
 *      unit        -  BCM Unit number
 *      dst         -  Proxy server data
 *      enable      -  server enable/diable flag
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_esw_proxy_server(int unit, _bcm_proxy_info_t *src, int enable)
{
    int rv = BCM_E_INTERNAL;


    rv = _bcm_esw_proxy_server_set(unit, src->server_port, src->mode, enable);

    return rv;
}

/*
 * Function:
 *      _bcm_esw_proxy_server_install
 * Purpose:
 *      Install and enable proxy server
 * Parameters:
 *      unit        -  BCM Unit number
 *      dst         -  Proxy server saved data
 *      src         -  Proxy server initial data
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_esw_proxy_server_install(int unit,
                              _bcm_proxy_info_t *dst,
                              _bcm_proxy_info_t *src)
{
    dst->server_port  = src->server_port;
    dst->mode  = src->mode;
    dst->flags |= (_BCM_PROXY_INFO_INSTALLED|_BCM_PROXY_INFO_SERVER);

    return _bcm_esw_proxy_server(unit, dst, 1);
}

/*
 * Function:
 *      _bcm_esw_proxy_server_uninstall
 * Purpose:
 *      Uninstall and disable proxy server
 * Parameters:
 *      unit        -  BCM Unit number
 *      dst         -  Proxy server saved data
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_esw_proxy_server_uninstall(int unit, _bcm_proxy_info_t *src)
{
    return _bcm_esw_proxy_server(unit, src, 0);
}

/*
 * Function:
 *      bcm_esw_proxy_server_set
 * Purpose:
 *      Enables various kinds of lookups for packets coming from remote
 *      (proxy client)
 * Parameters:
 *      unit        -  BCM Unit number
 *      server_port -  Local port to which packets from remote devices are 
 *                     destined to
 *      mode        -  Indicates lookup type
 *      enable      -  TRUE to enable lookups
 *                     FALSE to disable lookups
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_proxy_server_set(int unit, bcm_port_t server_port, 
                          bcm_proxy_mode_t mode, int enable)
{
    _bcm_proxy_info_t src;

    if (BCM_GPORT_IS_SET(server_port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, server_port, &server_port));
    }

    sal_memset(&src, 0, sizeof(_bcm_proxy_info_t));
    src.server_port  = server_port;
    src.mode  = mode;

    return _bcm_esw_proxy(unit, &server_methods, &src, enable);
}

/*
 * Function:
 *      _bcm_esw_proxy_cleanup_data
 * Purpose:
 *      Uninstall and release proxy data
 * Parameters:
 *      unit        -  BCM Unit number
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_esw_proxy_cleanup_data(int unit)
{
    _bcm_proxy_ifc_t *ifc;

    if (_BCM_PROXY_LOCK(unit)) {
        return BCM_E_INIT;
    }

    /* Cleanup clients/servers */

    while (_bcm_proxy_control[unit].proxy_list) {
        if (_bcm_proxy_control[unit].proxy_list->flags &
            _BCM_PROXY_INFO_SERVER) {
            ifc = &server_methods;
        } else {
            ifc = &client_methods;
        }
        (void)_bcm_esw_proxy_uninstall
            (unit, ifc, _bcm_proxy_control[unit].proxy_list);
    }

    _BCM_PROXY_UNLOCK(unit);

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_esw_proxy_init
 * Purpose:
 *      Initialize the Proxy subsystem
 * Parameters:
 *      unit        -  BCM Unit number
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_proxy_init(int unit)
{
    int rv = BCM_E_NONE;

    if (_LOCK(unit) == NULL) {
        _LOCK(unit) = sal_mutex_create("bcm_proxy_lock");
        rv = (_LOCK(unit) == NULL) ? BCM_E_MEMORY : BCM_E_NONE;
    }

    if (BCM_SUCCESS(rv)) {
        rv = _bcm_esw_proxy_cleanup_data(unit);
    }

    return rv;
}

/*
 * Function:
 *      bcm_esw_proxy_cleanup
 * Purpose:
 *      Deinitialize the Proxy subsystem
 * Parameters:
 *      unit        -  BCM Unit number
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_esw_proxy_cleanup(int unit)
{
    if (_LOCK(unit) != NULL) {
        (void) _bcm_esw_proxy_cleanup_data(unit);
        sal_mutex_destroy(_LOCK(unit));
        _LOCK(unit) = NULL;
    }

    return BCM_E_NONE;
}

#else /* INCLUDE_L3 */
int _bcm_esw_proxy_not_empty;
#endif /* INCLUDE_L3 */
