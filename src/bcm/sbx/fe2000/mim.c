/*
 * $Id: mim.c 1.123 Broadcom SDK $
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
 * File:    mim.c
 * Purpose: Manages MIM Provider Back bone Bridging functions for FE2000
 */
#if defined(INCLUDE_L3)

#include <bcm/types.h>
#include <bcm/error.h>
#include <bcm/debug.h>

#include <soc/drv.h>
#include <soc/sbx/sbx_drv.h>
#include <bcm/mim.h>
#include <bcm_int/sbx/fe2000/trunk.h>
#include <bcm/qos.h>
#include <soc/sbx/sbDq.h>
#include <soc/sbx/g2p3/g2p3.h>
#include <shared/avl.h>
#include <shared/idxres_fl.h>
#include <bcm/policer.h>
#include <bcm_int/sbx/fe2000/allocator.h>
#include <bcm_int/sbx/fe2000/policer.h>
#include <bcm_int/sbx/fe2000/g2p3.h>
#include <bcm_int/sbx/fe2000/port.h>
 

#define _MD(str)  "unit %d: [%s] " str, unit, FUNCTION_NAME()
#define MAC_FMT    "%02x:%02x:%02x:%02x:%02x:%02x"
#define MAC_PFMT(m) m[0], m[1], m[2], m[3], m[4], m[5]

/* Fixme - Add PID & tunnelID for smac learning */
typedef enum {
    _BCM_FE2K_MIM_ACCESS_PORT,
    _BCM_FE2K_MIM_BACKBONE_PORT,
    _BCM_FE2K_MIM_DEF_BACKBONE_PORT,
    _BCM_FE2K_MIM_PORT_MAX,
} bcm_fe2k_mim_port_type_e_t;

typedef struct {
    uint32      lport;
    uint32      ftidx;    /* forwarding index */
    uint32      ohi;      /* global */
    uint32      eteencap; /* dont care for non-home units */
    uint32      etel2;    /* dont care for non-home units */
    uint32      ismacidx; /* Ingress LSM index - applicable only for default back bone port*/
    uint32      esmacidx; /* Egress encapsulation smac idx */
    uint32      egrremarkidx; /* Egress remarking Index if applicable */
    uint32      egrremarkflags;
    /*bcm_failover_t failover_id;*/
} bcm_fe2k_mim_g2p3_hw_info_t;

typedef struct {
    /* service access points on this vpn - list of mim-ports excludes default back bone port*/
    dq_t                         vpn_access_sap_head;
    dq_t                         vpn_bbone_sap_head;
    dq_t                         def_bbone_plist;
    bcm_mim_vpn_t                vpnid;
    int                          isid;
    uint32                       lport;                  /* instrumentation */
    bcm_multicast_t              broadcast_group;        /* Broadcast group */
    bcm_multicast_t              unknown_unicast_group;  /* Unknown unicast group */
    bcm_multicast_t              unknown_multicast_group;/* Unknown multicast group */
    bcm_policer_t                policer_id;
} bcm_fe2k_mim_vpn_control_t;

typedef enum {
    _BCM_FE2K_MIM_PORT,
    _BCM_FE2K_MIM_STAG_1_1,
    _BCM_FE2K_MIM_STAG_BUNDLED,
    _BCM_FE2K_MIM_MAX_SUPPORTED,
    _BCM_FE2K_MIM_ITAG
} bcm_fe2k_mim_service_type_e_t;

typedef struct bcm_fe2k_mim_port_control_s {
    dq_t        listnode;
#define _MIM_PCB_TRUNK_NODE_POS (1)
    dq_t        trunklistnode;
    bcm_gport_t gport; /* virtual port  */
    bcm_gport_t port;  /* modport gport */
    bcm_mac_t   smac;  /* SMAC for back bone ports */
    bcm_mac_t   dmac;
    bcm_vlan_t  vlan;  /* back bone vlan, match vlan for access port */
    uint16      tpid;
    bcm_vlan_t  isidvlan; /* ISID - VSI */
    uint32      isid;
    bcm_policer_t policer_id;  /* Policer ID to be associated with the MiM */
    uint32      isidlport;  /* ISID logical port */   
    bcm_fe2k_mim_g2p3_hw_info_t hwinfo;
    bcm_fe2k_mim_port_type_e_t type;
    bcm_mim_port_match_t criteria;  /* Match criteria. */
    uint32 flags;                   /* Configuration flags */
    bcm_fe2k_mim_service_type_e_t service; /* Applicable only for Access ports */
    uint32      refcount; /* used to track number of users on a mim default back bone port*/
}bcm_fe2k_mim_port_control_t;

typedef struct {
    bcm_fe2k_mim_port_type_e_t mimType; /* only _BCM_FE2K_MIM_DEF_BACKBONE_PORT for now */
    dq_t                       plist;
} bcm_fe2k_mim_trunk_association_t;

bcm_fe2k_mim_trunk_association_t mim_trunk_assoc_info[BCM_MAX_NUM_UNITS][SBX_MAX_TRUNKS];

/*
 *  This structure contains information necessary to map MIM GPORT ID to
 *  logical port ID.
 */
typedef struct {
    uint16 lpid[SBX_MAX_GPORTS];  /* map GPORT ID to logical port ID */
} _bcm_fe2k_mimgp_info_t;

/* data needed to convert AVL traversal to bcm traversal */
typedef struct _bcm_fe200_mim_vpn_cb_data_s {
    int                     unit;
    bcm_mim_vpn_traverse_cb cb;
    void                    *userData;
} _bcm_fe200_mim_vpn_cb_data_t;

static  _bcm_fe2k_mimgp_info_t gportinfo[BCM_MAX_NUM_UNITS];
static shr_avl_t   *mim_vpn_db[BCM_MAX_NUM_UNITS];
static sal_mutex_t  mim_mlock[BCM_MAX_NUM_UNITS];

#define _MIM_INVALID_VPN_ID (0)
#define _MIM_INVALID_ISID   (-1)

#define _MIM_UNIT_INVALID(unit) \
    (((unit) < 0) || ((unit) >= BCM_MAX_NUM_UNITS))

#define _MIM_PORT_TYPE_VALID_CHECK(type) \
    (((type) >= _BCM_FE2K_MIM_ACCESS_PORT) || ((type) < _BCM_FE2K_MIM_PORT_MAX))


#define _MIM_PORT_INVALID(unit, port) \
           (((port) < 0) || ((port) >= SBX_MAX_PORTS) || \
             ((port) >= SOC_SBX_CFG_FE2000(unit)->numUcodePorts))

#define _MIM_LOCK_CREATED_ALREADY(unit) \
    (mim_mlock[(unit)] != NULL)

#define _MIM_LOCK(unit)       \
    (_MIM_LOCK_CREATED_ALREADY(unit)? \
     sal_mutex_take(mim_mlock[(unit)], sal_mutex_FOREVER)?BCM_E_UNIT:BCM_E_NONE: \
     BCM_E_UNIT)

#define _MIM_UNLOCK(unit)     \
    sal_mutex_give(mim_mlock[(unit)])


#define MIM_DEBUG(flags, stuff) BCM_DEBUG(flags | BCM_DBG_L2, stuff)
#define MIM_NOTICE(stuff)       MIM_DEBUG(BCM_DBG_NOTICE, stuff)
#define MIM_WARN(stuff)         MIM_DEBUG(BCM_DBG_WARN, stuff)
#define MIM_ERR(stuff)          MIM_DEBUG(BCM_DBG_ERR, stuff)
#define MIM_VERB(stuff)         MIM_DEBUG(BCM_DBG_VERBOSE, stuff)
#define MIM_VVERB(stuff)        MIM_DEBUG(BCM_DBG_VVERBOSE, stuff)

#define _BCM_FE2000_MIM_SUPPORTED_FLAGS (BCM_MIM_VPN_MIM | BCM_MIM_VPN_WITH_ID)

#define G2P3_ONLY_SUPPORT_CHECK(unit)                                                \
    if(!SOC_IS_SBX_G2P3((unit))) {                                                   \
        MIM_WARN(("WARNING %s is supported only for G2P3(%s,%d)\n",                  \
                                FUNCTION_NAME(),__FILE__,__LINE__));                 \
        return BCM_E_UNAVAIL;                                                        \
    }

#define _MIM_VPN_INIT_CHECK(unit)                                                   \
    if(!mim_vpn_db[(unit)]) {                                                       \
        _MIM_UNLOCK(unit);                                                          \
        MIM_WARN(("WARNING %s VPN sussystem not initialized(%s,%d)\n",              \
                                FUNCTION_NAME(),__FILE__,__LINE__));                \
        return BCM_E_UNAVAIL;                                                       \
    }


#define _BCM_FE2000_FTIDX_2_MIM_PORTID(unit, ftidx) ((ftidx) - SBX_GLOBAL_GPORT_FTE_BASE(unit))
#define _BCM_FE2000_MIM_PORTID_2_FTIDX(unit, mimportid) ((mimportid) + SBX_GLOBAL_GPORT_FTE_BASE(unit))

#define G2P3_GPORT_RANGE_CHECK(unit, gportid)                                 \
    do {                                                                      \
        if(BCM_GPORT_IS_MIM_PORT(gportid)) {                                  \
            uint32 _pid = BCM_GPORT_MIM_PORT_ID_GET(gportid);                 \
            if(_pid > (SBX_GLOBAL_GPORT_FTE_END(unit) -                       \
                        SBX_GLOBAL_GPORT_FTE_BASE(unit))) {                   \
                MIM_WARN(("WARNING %s BAD MiM GPORT ID specified(%s,%d)\n",   \
                                FUNCTION_NAME(),__FILE__,__LINE__));          \
                return BCM_E_PARAM;                                           \
            }                                                                 \
        } else {                                                              \
            MIM_WARN(("WARNING %s Not Mim Gport (%s,%d)\n",                   \
                                FUNCTION_NAME(),__FILE__,__LINE__));          \
            return BCM_E_PARAM;                                               \
        }                                                                     \
    } while(0)

#define  _BCM_FE2000_MIM_SUPPORTED_PORT_FLAGS (BCM_MIM_PORT_TYPE_BACKBONE |            \
                                               BCM_MIM_PORT_TYPE_ACCESS   |            \
                                               BCM_MIM_PORT_WITH_ID       |            \
                                               BCM_MIM_PORT_ENCAP_WITH_ID |            \
                                               BCM_MIM_PORT_EGRESS_SERVICE_VLAN_ADD)

#define  _BCM_FE2000_MIM_SUPPORTED_CRITERIA   (BCM_MIM_PORT_MATCH_PORT|                \
                                               BCM_MIM_PORT_MATCH_PORT_VLAN|           \
                                               BCM_MIM_PORT_MATCH_TUNNEL_VLAN_SRCMAC)

#define  _BCM_FE2000_MIM_BB_SUPPORTED_FLAGS   (BCM_MIM_PORT_TYPE_BACKBONE |            \
                                               BCM_MIM_PORT_WITH_ID       |            \
                                               BCM_MIM_PORT_ENCAP_WITH_ID )

#define  _BCM_FE2000_MIM_BB_SUPPORTED_CRITERIA (BCM_MIM_PORT_MATCH_TUNNEL_VLAN_SRCMAC)

#define  _BCM_FE2000_MIM_ACCESS_SUPPORTED_FLAGS (BCM_MIM_PORT_TYPE_ACCESS|             \
                                                 BCM_MIM_PORT_WITH_ID    |             \
                                                 BCM_MIM_PORT_EGRESS_SERVICE_VLAN_ADD |\
                                                 BCM_MIM_PORT_ENCAP_WITH_ID )

#define  _BCM_FE2000_MIM_ACCESS_SUPPORTED_CRITERIA (BCM_MIM_PORT_MATCH_PORT|           \
                                                    BCM_MIM_PORT_MATCH_PORT_VLAN)

#define  _BCM_FE2000_MIM_SUPPORTED_UPDATE_FLAGS (BCM_MIM_PORT_WITH_ID | \
                                                 BCM_MIM_PORT_REPLACE | \
                                                 BCM_MIM_PORT_ENCAP_WITH_ID)


#define _BCM_Fe2000_IS_MAC_EQUAL(m1,m2) \
            ((((m1)[0] == (m2)[0]) && \
             ((m1)[1] == (m2)[1]) && \
             ((m1)[2] == (m2)[2]) && \
             ((m1)[3] == (m2)[3]) && \
             ((m1)[4] == (m2)[4]) && \
             ((m1)[5] == (m2)[5])) ? 1:0)

#define _BCM_FE2000_GET_PORTCB_FROM_LIST(e, var) \
            (var) = DQ_ELEMENT(bcm_fe2k_mim_port_control_t*,\
                               (e), (var), listnode)

#define  _BCM_FE2000_G2P3_ADJUST_TB_OFFSET(ohi) ((ohi) - 8*1024)

#define  _BCM_FE2000_MIM_ETYPE (0x88e7)
#define  _BCM_FE2000_DEFAULT_BTAG_TPID (0x88A8)

extern int bcm_sbx_stk_modid_get(int,int *);

#define _AVL_EMPTY(avl) ((shr_avl_count(avl) > 0)?0:1)

/*-------- Static Functions Start -------*/

STATIC int
_bcm_fe2000_mim_trunk_cb(int unit, 
                         bcm_trunk_t tid, 
                         bcm_trunk_add_info_t *tdata,
                         void *user_data);

int 
_bcm_fe2000_mim_set_egress_remarking(int unit, 
                                     uint32 eteencapidx, 
                                     uint32 egrMap,
                                     uint32 egrFlags);

/* Translate a bcm_fe2k_mim_vpn_control_t to a bcm_mim_vpn_config_t */
static void
_bcm_fe2000_mim_vpn_control_xlate(bcm_fe2k_mim_vpn_control_t *vpnCtl,
                                  bcm_mim_vpn_config_t  *vpnConfig)
{
    
    vpnConfig->vpn       = vpnCtl->vpnid;
    vpnConfig->lookup_id = vpnCtl->isid;
    vpnConfig->broadcast_group         = vpnCtl->broadcast_group;
    vpnConfig->unknown_unicast_group   = vpnCtl->unknown_unicast_group;
    vpnConfig->unknown_multicast_group = vpnCtl->unknown_multicast_group;
}

/* Translate a bcm_fe2k_mim_port_control_t to a bcm_mim_port_t */
static void
_bcm_fe2000_mim_port_control_xlate(bcm_fe2k_mim_port_control_t *portCtl,
                                   bcm_mim_port_t *mimPort)
{
    
    mimPort->mim_port_id = portCtl->gport;
    mimPort->port        = portCtl->port;
    sal_memcpy(mimPort->egress_tunnel_srcmac, portCtl->smac, sizeof(bcm_mac_t));
    sal_memcpy(mimPort->egress_tunnel_dstmac, portCtl->dmac, sizeof(bcm_mac_t));

    mimPort->match_vlan            = portCtl->vlan;
    mimPort->egress_service_vlan   = portCtl->vlan;
    mimPort->egress_service_tpid   = portCtl->tpid;
    mimPort->egress_tunnel_service = portCtl->isid;
    mimPort->encap_id              = SOC_SBX_ENCAP_ID_FROM_OHI(portCtl->hwinfo.ohi);
    mimPort->criteria              = portCtl->criteria;
    mimPort->flags                 = portCtl->flags;
}

/* VPN Database Tree compare function using VPN ID */
/* VPNDB Insert compare function */
static int _bcm_fe2000_mim_insert_compare(void *userdata,
                                          shr_avl_datum_t *datum1,
                                          shr_avl_datum_t *datum2)
{
    bcm_fe2k_mim_vpn_control_t *data1, *data2;

    data1 = (bcm_fe2k_mim_vpn_control_t*)datum1;
    data2 = (bcm_fe2k_mim_vpn_control_t*)datum2;

    /* VPN ID has to be unique + ISID has to be unique */
    if(data1->vpnid < data2->vpnid) {
        return -1;
    }

    if(data1->vpnid > data2->vpnid) {
        return 1;
    }

    return(0);
}


typedef struct {
    bcm_fe2k_mim_vpn_control_t  *key;  /* search key */
    bcm_fe2k_mim_vpn_control_t **datum;
} _bcm_fe2k_mim_lookup_data_t;


/* AVL lookup compare function. Compares both VPNID and ISID
 * to see if there is another entry on the Tree using either the
 * same VPNID or ISID */
static int _bcm_fe2000_mim_vpn_lookup_compare(void *userdata,
                                          shr_avl_datum_t *datum1,
                                          shr_avl_datum_t *datum2,
                                          void *lkupdata)
{
    bcm_fe2k_mim_vpn_control_t *data1, *data2;
    void *vp = datum2; /* grab the pointer to AVL tree datum element */

    data1 = (bcm_fe2k_mim_vpn_control_t*)datum1;
    data2 = (bcm_fe2k_mim_vpn_control_t*)datum2;

    /* VPN ID has to be unique + ISID has to be unique */
    if(data1->vpnid < data2->vpnid) {
        if(data1->isid == data2->isid){
            /* the VPN we are looking for exists, get its address */
            *(((_bcm_fe2k_mim_lookup_data_t*)lkupdata)->datum) = \
                ( bcm_fe2k_mim_vpn_control_t*)vp;
            return 0;
        } else {
            return -1;
        }
    } else if(data1->vpnid > data2->vpnid) {
        if(data1->isid == data2->isid){
            /* the VPN we are looking for exists, get its address */
            *(((_bcm_fe2k_mim_lookup_data_t*)lkupdata)->datum) = \
                ( bcm_fe2k_mim_vpn_control_t*)vp;
            return 0;
        } else {
            return 1;
        }
    } else { /* data1->vpnid == data2->vpnid */
        /* the VPN we are looking for exists, get its address */
        *(((_bcm_fe2k_mim_lookup_data_t*)lkupdata)->datum) = \
            ( bcm_fe2k_mim_vpn_control_t*)vp;
        return 0;
    }
}

static int _bcm_fe2000_mim_vpn_copy_datum(void *userdata,
                                          shr_avl_datum_t *datum1,
                                          shr_avl_datum_t *datum2)
{
    bcm_fe2k_mim_vpn_control_t *data1, *data2;

    data1 = (bcm_fe2k_mim_vpn_control_t*)datum1;
    data2 = (bcm_fe2k_mim_vpn_control_t*)datum2;

    sal_memcpy(datum1, datum2, sizeof(bcm_fe2k_mim_vpn_control_t));

    /* carefully manipute list headers */
    if(DQ_EMPTY(&data2->vpn_access_sap_head)) {
        DQ_INIT(&data1->vpn_access_sap_head);
    } else {
        DQ_SWAP_HEAD(&data1->vpn_access_sap_head, &data2->vpn_access_sap_head);
    }

    if(DQ_EMPTY(&data2->vpn_bbone_sap_head)) {
        DQ_INIT(&data1->vpn_bbone_sap_head);
    } else {
        DQ_SWAP_HEAD(&data1->vpn_bbone_sap_head, &data2->vpn_bbone_sap_head);
    }

    if(DQ_EMPTY(&data2->def_bbone_plist)) {
        DQ_INIT(&data1->def_bbone_plist);
    } else {
        DQ_SWAP_HEAD(&data1->def_bbone_plist, &data2->def_bbone_plist);
    }
    return 0;
}

static int _bcm_fe2000_mim_vpn_lookup_compare_nodata(void *userdata,
                                          shr_avl_datum_t *datum1,
                                          shr_avl_datum_t *datum2)
{
    bcm_fe2k_mim_vpn_control_t *data1, *data2;

    data1 = (bcm_fe2k_mim_vpn_control_t*)datum1;
    data2 = (bcm_fe2k_mim_vpn_control_t*)datum2;

    /* VPN ID has to be unique + ISID has to be unique */
    if(data1->vpnid < data2->vpnid) {
        if(data1->isid == data2->isid){
            return 0;
        } else {
            return -1;
        }
    } else if(data1->vpnid > data2->vpnid) {
        if(data1->isid == data2->isid){
            return 0;
        } else {
            return 1;
        }
    } else { /* data1->vpnid == data2->vpnid */
        return 0;
    }
}

static int _bcm_fe2000_mim_port_check(int unit,
                                      bcm_mim_port_t *mim_port,
                                      bcm_fe2k_mim_vpn_control_t *vpncb,
                                      bcm_fe2k_mim_port_control_t **defbbportcb)
{
    bcm_fe2k_mim_port_control_t *portcb = NULL;
    dq_p_t                       port_elem;
    uint8                        isdefbbone = 0;

    if((!mim_port) || (!vpncb) || (!defbbportcb)) {
        return BCM_E_PARAM;
    }

    /* Verify the flags */
    if(mim_port->flags & ~_BCM_FE2000_MIM_SUPPORTED_PORT_FLAGS) {
        return BCM_E_PARAM;
    }

    if(mim_port->flags & BCM_MIM_PORT_WITH_ID) {
        /* verify if mim-port id is valid */
        G2P3_GPORT_RANGE_CHECK(unit, mim_port->mim_port_id);
    }

    if(mim_port->criteria & ~_BCM_FE2000_MIM_SUPPORTED_CRITERIA) {
        return BCM_E_PARAM;
    }

    if(mim_port->flags & BCM_MIM_PORT_TYPE_ACCESS) {
        /* Verify if user is requesting for only Supported PBBN interface types */
        /* Supported Service Interfaces:
         * [1] Port Mode
         * [2] 1:1 STAG Mode
         */
        if(mim_port->flags & ~_BCM_FE2000_MIM_ACCESS_SUPPORTED_FLAGS) {
            return BCM_E_PARAM;
        }
        if(mim_port->criteria & ~_BCM_FE2000_MIM_ACCESS_SUPPORTED_CRITERIA) {
            return BCM_E_PARAM;
        }
    }

    /* verify if the port is MODPORT or TRUNK gport type */
    if(!BCM_GPORT_IS_MODPORT(mim_port->port) &&
       !BCM_GPORT_IS_TRUNK(mim_port->port)){
         MIM_ERR(("WARNING %s Only modport Gport ID supported !!!!(%s,%d)\n",\
                                  FUNCTION_NAME(),__FILE__,__LINE__));
         return BCM_E_PARAM;
    }


    if(BCM_GPORT_IS_MODPORT(mim_port->port) &&
       _MIM_PORT_INVALID(unit, BCM_GPORT_MODPORT_PORT_GET(mim_port->port))) {
        return BCM_E_PARAM;
    }

    /* if port is created with an ID verify Encap ID and Mim port ID */
    if(mim_port->flags & BCM_MIM_PORT_WITH_ID){
        if(!BCM_GPORT_IS_MIM_PORT(mim_port->mim_port_id)) {
            MIM_ERR(("WARNING %s Invalid MIM port ID !!!!(%s,%d)\n",\
                     FUNCTION_NAME(),__FILE__,__LINE__));
            return BCM_E_PARAM;
        }
        
        if((mim_port->flags & BCM_MIM_PORT_ENCAP_WITH_ID) &&
           (!SOC_SBX_IS_VALID_ENCAP_ID(mim_port->encap_id))) {
            MIM_ERR(("WARNING %s Invalid Encap ID !!!!(%s,%d)\n",\
                     FUNCTION_NAME(),__FILE__,__LINE__));
         return BCM_E_PARAM;
       }
    }

    /* Verify no back bone port is created before default back bone port is created */
    if(mim_port->flags & BCM_MIM_PORT_TYPE_BACKBONE) {

        if(mim_port->flags & ~_BCM_FE2000_MIM_BB_SUPPORTED_FLAGS) {
            return BCM_E_PARAM;
        }

        if(mim_port->criteria & ~_BCM_FE2000_MIM_BB_SUPPORTED_CRITERIA) {
            return BCM_E_PARAM;
        }

        if(BCM_MAC_IS_ZERO(mim_port->egress_tunnel_srcmac)){
           MIM_ERR(("WARNING %s Zero SMAC invalid for Back bone port !!!!(%s,%d)\n",\
                                  FUNCTION_NAME(),__FILE__,__LINE__));
           return BCM_E_PARAM;
        }

        /* if default back bone port is trying to be created verify if no default
         * back bone port exists */
        if(BCM_MAC_IS_ZERO(mim_port->egress_tunnel_dstmac)){

           /* Verify if the Default Back Bone Port Exists */
           DQ_TRAVERSE(&vpncb->def_bbone_plist, port_elem) {

               _BCM_FE2000_GET_PORTCB_FROM_LIST(port_elem, portcb);

               /* Cant create more than one default back bone port on same modport gport */
               if(portcb->port == mim_port->port){
                   MIM_ERR(("WARNING %s Duplicate Default Back bone port creation !!!!(%s,%d)\n",\
                            FUNCTION_NAME(),__FILE__,__LINE__));
                   return BCM_E_PARAM;
               }
           } DQ_TRAVERSE_END(&vpncb->def_bbone_plist, port_elem);
           isdefbbone = 1;
        } else {
           int found = 0;
           /* VIP(ISID) can be associated with only one PIP or BSMAC */
           /* Verify if PIP or BSMAC are same for default and added back bone port */
           DQ_TRAVERSE(&vpncb->def_bbone_plist, port_elem) {

               _BCM_FE2000_GET_PORTCB_FROM_LIST(port_elem, portcb);

               if(_BCM_Fe2000_IS_MAC_EQUAL(portcb->smac, mim_port->egress_tunnel_srcmac)){
                   found = 1;
                   *defbbportcb = portcb;
               }
           } DQ_TRAVERSE_END(&vpncb->def_bbone_plist, port_elem);

           if(!found) {
               MIM_ERR(("WARNING %s Default Back bone not found !!!!(%s,%d)\n",\
                        FUNCTION_NAME(),__FILE__,__LINE__));
               return BCM_E_PARAM;
           }
        }
    }

    if(!isdefbbone) {
        /* verify is no other mim acces port is created on this port */
        if(mim_port->flags & BCM_MIM_PORT_TYPE_ACCESS) {

        /* Check if there is a Mim Port using this port */
            DQ_TRAVERSE(&vpncb->vpn_access_sap_head, port_elem) {

            _BCM_FE2000_GET_PORTCB_FROM_LIST(port_elem, portcb);

            if(mim_port->flags & BCM_MIM_PORT_WITH_ID) {
                /* verify if a virtual port exists with this ID */
                if(portcb->gport == mim_port->mim_port_id) {
                    return BCM_E_EXISTS;
                }
            }

                if(portcb->port == mim_port->port){
                    MIM_ERR(("WARNING %s Access port Exists !!!!(%s,%d)\n",\
                             FUNCTION_NAME(),__FILE__,__LINE__));
                    return BCM_E_EXISTS;
                }
                
            } DQ_TRAVERSE_END(&vpncb->vpn_access_sap_head, port_elem);
        }

            /* compare the mac addresses and see if ports exist with the same mac */
            if(mim_port->flags & BCM_MIM_PORT_TYPE_BACKBONE) {

            /* Check if there is a Mim Port using this port */
            DQ_TRAVERSE(&vpncb->vpn_bbone_sap_head, port_elem) {

                _BCM_FE2000_GET_PORTCB_FROM_LIST(port_elem, portcb);

                if(mim_port->flags & BCM_MIM_PORT_WITH_ID) {
                    /* verify if a virtual port exists with this ID */
                    if(portcb->gport == mim_port->mim_port_id) {
                    return BCM_E_EXISTS;
                }
            }

                /* compare the mac addresses and see if ports exist with the same mac */
                if(_BCM_Fe2000_IS_MAC_EQUAL(portcb->dmac, mim_port->egress_tunnel_dstmac)) {
                    
                    MIM_ERR(("WARNING %s Back bone port Exists !!!!(%s,%d)\n",\
                                    FUNCTION_NAME(),__FILE__,__LINE__));
                    return BCM_E_EXISTS;
                }

            } DQ_TRAVERSE_END(&vpncb->vpn_bbone_sap_head, port_elem);
            }
    }

    return BCM_E_NONE;
}

static int _bcm_fe2000_mim_port_update_check(int unit,
                                             bcm_mim_port_t *mim_port,
                                             bcm_fe2k_mim_vpn_control_t *vpncb,
                                             bcm_fe2k_mim_port_control_t **updateportcb,
                                             bcm_fe2k_mim_port_control_t **newdefportcb,
                                             bcm_fe2k_mim_port_control_t **olddefportcb)
{
    uint32 portid;
    uint16 lport;
    bcm_fe2k_mim_port_control_t *portcb=NULL, *defbbportcb=NULL;
    dq_p_t port_elem;
    int status = BCM_E_NONE;

    if(!mim_port || !vpncb || !updateportcb || !newdefportcb || !olddefportcb) {
        return BCM_E_PARAM;
    }

    /* Verify the flags */
    if(!(mim_port->flags & _BCM_FE2000_MIM_SUPPORTED_UPDATE_FLAGS)) {
        return BCM_E_PARAM;
    }

    /* verify if the port is MODPORT or TRUNK gport type */
    if(!BCM_GPORT_IS_MODPORT(mim_port->port) &&
       !BCM_GPORT_IS_TRUNK(mim_port->port)){
         MIM_ERR(("WARNING %s Only modport Gport ID supported !!!!(%s,%d)\n",\
                                  FUNCTION_NAME(),__FILE__,__LINE__));
         return BCM_E_PARAM;
    }


    if(BCM_GPORT_IS_MODPORT(mim_port->port) &&
       _MIM_PORT_INVALID(unit, BCM_GPORT_MODPORT_PORT_GET(mim_port->port))) {
        return BCM_E_PARAM;
    }

    /* Only update supported now is for Station Movement update of Back Bone port */
    /* obtain portcb using gport id */
    portid = BCM_GPORT_MIM_PORT_ID_GET(mim_port->mim_port_id);

    /* verify the gport id */
    G2P3_GPORT_RANGE_CHECK(unit, mim_port->mim_port_id);

    /* obtain logical port from mim gport */
    lport = gportinfo[unit].lpid[portid - SBX_GLOBAL_GPORT_FTE_BASE(unit)];

    if(BCM_GPORT_MIM_PORT != SBX_LPORT_TYPE(unit,lport)) {

        MIM_ERR(("Error %s LP GPORT not a MiM port !!!!(%s,%d)\n",\
                 FUNCTION_NAME(),__FILE__,__LINE__));

        status = BCM_E_PARAM;

    } else {
        portcb = (bcm_fe2k_mim_port_control_t*)SBX_LPORT_DATAPTR(unit, lport);

        /* This is not a station movement update flag error */
        if((portcb->type == _BCM_FE2K_MIM_BACKBONE_PORT) &&
           (portcb->port == mim_port->port)) {

            *newdefportcb = NULL;
            *olddefportcb = NULL;

           /* Verify if there is Default Back Bone Port for the moved BMAC to associate to */
           DQ_TRAVERSE(&vpncb->def_bbone_plist, port_elem) {

               _BCM_FE2000_GET_PORTCB_FROM_LIST(port_elem, defbbportcb);

               if(defbbportcb->port == portcb->port) {
                   *olddefportcb = defbbportcb;
               }

               if(defbbportcb->port == mim_port->port){
                   *newdefportcb = defbbportcb;
               }

               if(*newdefportcb && *olddefportcb) {
                   break;
               }

           } DQ_TRAVERSE_END(&vpncb->def_bbone_plist, port_elem);

           if((defbbportcb) && (defbbportcb->port == mim_port->port)) {
               *updateportcb = portcb;
           } else {

               MIM_ERR(("ERROR: %s Default Back bone not found !!!!(%s,%d)\n",\
                        FUNCTION_NAME(),__FILE__,__LINE__));
               status = BCM_E_PARAM;
           }
        } else {

            MIM_ERR(("Error %s Not a Back Bone Station Movement Update !!!!(%s,%d)\n",\
                     FUNCTION_NAME(),__FILE__,__LINE__));

            status = BCM_E_PARAM;
        }
    }
    return status;
}

static int _bcm_fe2000_mim_update_bbone_port_movement(int unit,
                                                      bcm_mim_port_t *mim_port,
                                                      bcm_fe2k_mim_port_control_t *portcb,
                                                      bcm_fe2k_mim_port_control_t *defbbportcb,
                                                      bcm_fe2k_mim_port_control_t *olddefbbportcb)
{
    int status = BCM_E_NONE;
    int    fabport, fabunit, node, module, port;

    if(!portcb || !mim_port || !defbbportcb) {
        return BCM_E_PARAM;
    }

    /* port can be moved from
       1. non-trunk to trunk
       2. trunk to non-trunk
       3. non-trunk to non-trunk
       4. trunk to trunk
    */

    /* case 4 */
    if(BCM_GPORT_IS_MODPORT(mim_port->port) &&
       BCM_GPORT_IS_MODPORT(portcb->port)) {
        /* update FTIDX QID for the new modport gport */
        soc_sbx_g2p3_ft_t fte;

        status = soc_sbx_g2p3_ft_get(unit, portcb->hwinfo.ftidx, &fte);
        if(BCM_SUCCESS(status)) {
            module = BCM_GPORT_MODPORT_MODID_GET(portcb->port);
            port   = BCM_GPORT_MODPORT_PORT_GET(portcb->port);


            /* obtain fabric info to program QID */
            status = soc_sbx_node_port_get(unit, module, port,
                                           &fabunit, &node, &fabport);
            if(BCM_FAILURE(status)) {
                MIM_ERR(("Error %s Failed to Get Fabric Port Node info !!!!(%s,%d)\n",\
                                    FUNCTION_NAME(),__FILE__,__LINE__));
            } else {
   	        fte.qid = SOC_SBX_NODE_PORT_TO_QID(unit,node,fabport, NUM_COS(unit));
                MIM_VERB(("unit %d: [%s] soc_sbx_node_port_get mod(0x%x) port(%d)"
                          "-> node(0x%x) port (%d) map to qid (0x%05x) unit(%d)\n",
                          unit, FUNCTION_NAME(), module, port, node,
                          fabport, fte.qid, fabunit));

                status = soc_sbx_g2p3_ft_set(unit,portcb->hwinfo.ftidx, &fte);
                if(BCM_FAILURE(status)) {
                    MIM_ERR(("unit %d: [%s] error %s in programming FTE [0x%x] in HW\n",
                             unit, FUNCTION_NAME(), bcm_errmsg(status), portcb->hwinfo.ftidx));
                } else {
                    /* update bsmac pid */
                    soc_sbx_g2p3_bmac_t bsmac;
                    status = soc_sbx_g2p3_bmac_get(unit, portcb->dmac,
                                                   portcb->vlan, &bsmac);
                    if(BCM_FAILURE(status)) {
                        MIM_ERR(("unit %d: [%s] error %s Getting BSMAC"MAC_FMT"\n",
                                 unit, FUNCTION_NAME(), bcm_errmsg(status),MAC_PFMT(portcb->dmac)));
                    } else {
                        int fabport, fabunit, fabnode;

                        status = soc_sbx_node_port_get(unit, module, port,
                                                       &fabunit, &fabnode, &fabport);
                        if(BCM_FAILURE(status)) {
                            MIM_ERR(("unit %d: [%s] error %s Mapping FE to QE port information",
                                     unit, FUNCTION_NAME(), bcm_errmsg(status)));
                        } else {

                            /* update port of entry on BSMAC */
                            bsmac.bpid = SOC_SBX_PORT_SID(unit, fabnode,
                                                          fabport);

                            status = soc_sbx_g2p3_bmac_set(unit, portcb->dmac,
                                                           portcb->vlan, 
                                                           &bsmac);
                            if(BCM_FAILURE(status)) {
                                MIM_ERR(("unit %d: [%s] error %s Setting BSMAC"MAC_FMT"\n",
                                         unit, FUNCTION_NAME(), bcm_errmsg(status),MAC_PFMT(portcb->dmac)));
                            }
                        }
                    }
                }
            }

        } else {
            MIM_ERR(("Error %s Getting FTE idx [0x%x] !!!!(%s,%d)\n",\
                     FUNCTION_NAME(),portcb->hwinfo.ftidx,__FILE__,__LINE__));
        }
    } else {
        /* add support */
        status = BCM_E_UNAVAIL;
    }

    if(BCM_SUCCESS(status)) {
        portcb->port = mim_port->port; /* update the underneath modport gport */
        defbbportcb->refcount++;
        olddefbbportcb->refcount--;
    }

    return status;
}

/* Terminology
 * INSEG (aka) Insegment - Refers to Ingress Hardware Resources required for SBX forwarding
 *                       - eg., FT, LP
 * OUTSEG (aka) Outsegment - Refers to Egress Hardware Resources required for SBX forwarding
 */

static
int _bcm_fe2000_mim_g2p3_alloc_inseg(int           unit,
                                     int          flags,
                                     bcm_fe2k_mim_port_control_t *portcb)
{
    int status = BCM_E_NONE;

    if(!portcb){
        status = BCM_E_PARAM;
    } else {
        if(_BCM_FE2K_MIM_DEF_BACKBONE_PORT == portcb->type) {
            /* Allocate a LSM entry */
            status = _sbx_gu2_ismac_idx_alloc(unit,
                                              0,
                                              portcb->smac,
                                              _SBX_GU2_RES_UNUSED_PORT,
                                              &portcb->hwinfo.ismacidx);
        }

        if(BCM_SUCCESS(status)) {

            /* Allocate logical port */
            /* Logical Ports for Back bone ports are dummy, they are only used now for quick access
             * of obtaining Port information given a GPORT ID which is O(1) access */
            status = _sbx_gu2_resource_alloc(unit,
                                         SBX_GU2K_USR_RES_LPORT,
                                         1,
                                         &portcb->hwinfo.lport,
                                         0);
            if (BCM_FAILURE(status)) {
                MIM_ERR(("unit %d: [%s] error(%s) could not allocate LP index\n",
                         unit, FUNCTION_NAME(), bcm_errmsg(status)));
            } else {

                status = _sbx_gu2_resource_alloc(unit,
                                                 SBX_GU2K_USR_RES_FTE_GLOBAL_GPORT,
                                                 1,
                                                 &portcb->hwinfo.ftidx,
                                                 (flags & BCM_MIM_PORT_WITH_ID)?
                                                 _SBX_GU2_RES_FLAGS_RESERVE:0);

                if(flags & BCM_MIM_PORT_WITH_ID) {
                   if(status == BCM_E_RESOURCE) {
                       MIM_VERB(("DEBUG Reserved FTE (0x%x) (%s,%d)\n",\
                                 portcb->hwinfo.ftidx,__FILE__,__LINE__));
                       status = BCM_E_NONE;
                   } else {
                       status = BCM_E_FULL;
                   }
                }

                if (BCM_FAILURE(status)) {
                    MIM_ERR(("unit %d: [%s] error(%s) could not allocate fte index\n",
                             unit, FUNCTION_NAME(), bcm_errmsg(status)));
                } else {
                    /* validate if FTIDX doesnt exceed PID number of bits */
                    int result;
                    uint32 min, max;

                    result = _sbx_gu2_alloc_range_get(unit, SBX_GU2K_USR_RES_LPORT,
                                                      &min, &max);

                    if((result == BCM_E_NONE) && (portcb->hwinfo.ftidx > max)) {
                        MIM_ERR(("Error %s FTIDX(%d) greater than PID range(%d<->%d) supported !!!!(%s,%d)\n",\
                                 FUNCTION_NAME(),portcb->hwinfo.ftidx, min, max, __FILE__,__LINE__));
                        status = BCM_E_PARAM;
                    } else {
                        /* Set Global FT to GPORT Type mapping */
                        if(_sbx_gu2_set_global_fte_type(unit, portcb->hwinfo.ftidx, 
                                                        BCM_GPORT_MIM_PORT) != BCM_E_NONE) {
                            MIM_ERR(("Error %s Setting Global FTE Type", FUNCTION_NAME()));
                            status = BCM_E_INTERNAL;
                        }
                    }
                }

            }
        } else {
            MIM_ERR(("unit %d: [%s] error(%s) could not allocate SMAC index\n",
                     unit, FUNCTION_NAME(), bcm_errmsg(status)));
        }
    }
    return status;
}

static
int _bcm_fe2000_mim_g2p3_free_inseg(int           unit,
                                    bcm_fe2k_mim_port_control_t *portcb)
{
    int status = BCM_E_NONE;

    if(!portcb) {
        return BCM_E_PARAM;
    }

    if (portcb->hwinfo.ismacidx) {
        status = _sbx_gu2_ismac_idx_free(unit,
                                         portcb->dmac,
                                         _SBX_GU2_RES_UNUSED_PORT,
                                         &portcb->hwinfo.ismacidx);
        if (status == BCM_E_EMPTY) {
            soc_sbx_g2p3_lsmac_t lsmac;
            soc_sbx_g2p3_lsmac_t_init(&lsmac);

            status = soc_sbx_g2p3_lsmac_set (unit, portcb->hwinfo.ismacidx, &lsmac);
            if (BCM_FAILURE(status)) {
                MIM_ERR((_MD("Failed to clear LSM: %d %s\n"),
                         status, bcm_errmsg(status)));
            } else {
                MIM_VERB(("Freed Back LSM[0x%x] for MAC"MAC_FMT"\n",
                           portcb->hwinfo.ismacidx, MAC_PFMT(portcb->smac)));
            }

        } else if (BCM_FAILURE(status)) {
            MIM_ERR((_MD("error(%s) could not free SMAC IDX [%d]\n"),
                     bcm_errmsg(status), portcb->hwinfo.ismacidx));
        }
    }

    if(portcb->hwinfo.lport){
        /* Free logical port */
        status = _sbx_gu2_resource_free(unit, SBX_GU2K_USR_RES_LPORT,
                                        1, &portcb->hwinfo.lport, 0);

        if (BCM_FAILURE(status)) {
            MIM_ERR((_MD("error(%s) could not free LP[%d]\n"),
                     bcm_errmsg(status), portcb->hwinfo.lport));
        }
    }


    if(portcb->hwinfo.ftidx) {
        /* Free FTE port */
        status = _sbx_gu2_resource_free(unit,
                                        SBX_GU2K_USR_RES_FTE_GLOBAL_GPORT,
                                        1,
                                        &portcb->hwinfo.ftidx,
                                        0);
        if(BCM_FAILURE(status)) {
            MIM_ERR(("unit %d: [%s] error(%s) could not free FTE[%d]\n",
                     unit, FUNCTION_NAME(),bcm_errmsg(status), portcb->hwinfo.ftidx));
        }

        /* Invalidate Global FT to GPORT Type mapping */
        if(_sbx_gu2_set_global_fte_type(unit, portcb->hwinfo.ftidx, 
                                        BCM_GPORT_TYPE_NONE) != BCM_E_NONE) {
            MIM_ERR(("Error %s Setting Global FTE Type", FUNCTION_NAME()));
            status = BCM_E_INTERNAL;
        }
    }

    return status;
}

static
int _bcm_fe2000_mim_g2p3_alloc_outseg(int                          unit,
                                      int                          modid,
                                      int                          flags,
                                      bcm_if_t                    *encap,
                                      bcm_fe2k_mim_port_control_t *portcb)
{
    int status=BCM_E_NONE, result=0;
    int encapfail=0;

    if(!portcb || !encap) {
        status = BCM_E_PARAM;
        MIM_ERR(("WARNING %s Bad Parameter !!!!(%s,%d)\n",\
                                FUNCTION_NAME(),__FILE__,__LINE__));
    } else {

        /* Assumption, Encap ID is validated on port check */
        /* Allocate OI */
        if(flags & BCM_MIM_PORT_ENCAP_WITH_ID) {
            portcb->hwinfo.ohi = SOC_SBX_OHI_FROM_ENCAP_ID(*encap);
        }

        result = _sbx_gu2_resource_alloc(unit,
                     SBX_GU2K_USR_RES_OHI,
                     1,
                     &portcb->hwinfo.ohi,
                     (flags & BCM_MIM_PORT_ENCAP_WITH_ID)?_SBX_GU2_RES_FLAGS_RESERVE:0);

        if(flags & BCM_MIM_PORT_ENCAP_WITH_ID) {
           if(result == BCM_E_RESOURCE) {
               MIM_VERB(("DEBUG Reserved EncapID (0x%x) (%s,%d)\n",\
                         portcb->hwinfo.ohi,__FILE__,__LINE__));
               result = BCM_E_NONE;
           } else {
               result = BCM_E_FULL;
           }
        }

        if(BCM_FAILURE(result)) {
            MIM_VERB(("WARNING %s Encap ID allocation Failed !!!!(%s,%d)\n",\
                     FUNCTION_NAME(),__FILE__,__LINE__));
            encapfail = 1;
            status = BCM_E_FULL;
        } else {
            /* Dont allocate ETE if this is not the Home Unit */

            result = _sbx_gu2_resource_alloc(unit,
                                         SBX_GU2K_USR_RES_ETE_ENCAP,
                                         1,
                                         &portcb->hwinfo.eteencap,
                                         0);
            if(BCM_FAILURE(result)) {
                MIM_VERB(("WARNING %s ETE Encap allocation Failed !!!!(%s,%d)\n",\
                    FUNCTION_NAME(),__FILE__,__LINE__));
                status = BCM_E_FULL;
            } else {
                result = _sbx_gu2_resource_alloc(unit,
                                             SBX_GU2K_USR_RES_ETE_L2,
                                             1,
                                             &portcb->hwinfo.etel2,
                                             0);
                if(BCM_FAILURE(result)) {
                    MIM_VERB(("WARNING %s ETE L2  allocation Failed !!!!(%s,%d)\n",\
                         FUNCTION_NAME(),__FILE__,__LINE__));
                    status = BCM_E_FULL;
                } else {
                    /* If back bone port allocate smac idx */
                    if(portcb->type == _BCM_FE2K_MIM_BACKBONE_PORT ||
                       portcb->type == _BCM_FE2K_MIM_DEF_BACKBONE_PORT) {

                        result = _sbx_gu2_esmac_idx_alloc(unit, 0,
                                                          portcb->smac,
                                                          &portcb->hwinfo.esmacidx);
                        if(BCM_E_NONE != result) {
                            MIM_VERB(("WARNING %s ESMAC allocation Failed !!!!(%s,%d)\n",\
                                      FUNCTION_NAME(),__FILE__,__LINE__));
                            status = BCM_E_FULL;
                        }
                    }
                }

            } /* else of ete encap failure check */
        } /* else of oi allocation failure check */

        if(BCM_E_FULL == status){

            if(portcb->hwinfo.etel2) {
                if(BCM_E_NONE != _sbx_gu2_resource_free(unit,
                                                        SBX_GU2K_USR_RES_ETE_L2,
                                                        1,
                                                        &portcb->hwinfo.etel2,
                                                        0)) {
                    MIM_VERB(("WARNING %s ETE L2 Free Failed !!!!(%s,%d)\n",\
                            FUNCTION_NAME(),__FILE__,__LINE__));
                }
            }

            if(portcb->hwinfo.eteencap) {
                if(BCM_E_NONE != _sbx_gu2_resource_free(unit,
                                                        SBX_GU2K_USR_RES_ETE_ENCAP,
                                                        1,
                                                        &portcb->hwinfo.eteencap,
                                                        0)) {
                    MIM_VERB(("WARNING %s ETE Encap Free Failed !!!!(%s,%d)\n",\
                            FUNCTION_NAME(),__FILE__,__LINE__));
                }
            }

            if(!encapfail) {
                if(BCM_E_NONE != _sbx_gu2_resource_free(unit,
                                                        SBX_GU2K_USR_RES_OHI,
                                                        1,
                                                        (uint32*)encap,
                                                        0)) {
                    MIM_VERB(("WARNING %s ETE Encap Free Failed !!!!(%s,%d)\n",\
                            FUNCTION_NAME(),__FILE__,__LINE__));
                }
            }

            MIM_ERR(("Error %s Enacpsulation Resource not available !!!!(%s,%d)\n",\
                                    FUNCTION_NAME(),__FILE__,__LINE__));
        } else {
            *encap = SOC_SBX_ENCAP_ID_FROM_OHI(portcb->hwinfo.ohi);
        }

    }
    return status;
}

static
int _bcm_fe2000_mim_g2p3_free_outseg(int                          unit,
                                     bcm_fe2k_mim_port_control_t *portcb)
{
    int status = BCM_E_NONE;

    if(!portcb) {
        status = BCM_E_PARAM;
        MIM_ERR(("WARNING %s Bad Parameter !!!!(%s,%d)\n",\
                  FUNCTION_NAME(),__FILE__,__LINE__));
    } else {
        if(portcb->hwinfo.ohi){
            /* Free logical port */
            status = _sbx_gu2_resource_free(unit, SBX_GU2K_USR_RES_OHI,
                                            1, &portcb->hwinfo.ohi, 0);
            if(BCM_FAILURE(status)) {
                MIM_ERR((_MD("error(%s) could not free OI[%d]\n"),
                         bcm_errmsg(status), portcb->hwinfo.ohi));
            }
        }

        if(portcb->hwinfo.eteencap){
            /* Free logical port */
            status = _sbx_gu2_resource_free(unit, SBX_GU2K_USR_RES_ETE_ENCAP,
                                            1, &portcb->hwinfo.eteencap, 0);
            if(BCM_FAILURE(status)) {
                MIM_ERR((_MD("error(%s) could not free Encap ETE[%d]\n"),
                         bcm_errmsg(status), portcb->hwinfo.eteencap));
            }
        }

        if (portcb->hwinfo.etel2) {
            /* Free logical port */
            status = _sbx_gu2_resource_free(unit, SBX_GU2K_USR_RES_ETE_L2,
                                            1, &portcb->hwinfo.etel2, 0);

            if (BCM_FAILURE(status)) {
                MIM_ERR((_MD("error(%s) could not free Encap L2[%d]\n"),
                         bcm_errmsg(status), portcb->hwinfo.etel2));
            }
        }

        if(portcb->hwinfo.esmacidx) {
            /* Free logical port */
            MIM_VERB((_MD("Freeing esmac idx 0x%x mac=" MAC_FMT "\n"),
                      portcb->hwinfo.esmacidx, MAC_PFMT(portcb->smac)));
            status = _sbx_gu2_esmac_idx_free(unit, portcb->smac,
                                             &portcb->hwinfo.esmacidx);

            if (status == BCM_E_EMPTY) {
                soc_sbx_g2p3_esmac_t esmac;
                int esmacIdx = portcb->hwinfo.esmacidx;

                soc_sbx_g2p3_esmac_t_init(&esmac);
                status = soc_sbx_g2p3_esmac_set(unit, esmacIdx, &esmac);
                if (BCM_FAILURE(status)) {
                    MIM_ERR((_MD("failed to clear esmac idx=%d: %d (%s)\n"),
                             esmacIdx, status, bcm_errmsg(status)));
                }
            } else if (BCM_FAILURE(status)) {
                MIM_ERR((_MD("error(%s) could not free ESMAC[%d]\n"),
                             bcm_errmsg(status), portcb->hwinfo.esmacidx));
            }
        }
    }
    return status;
}

static
int _bcm_fe2000_mim_g2p3_map_inseg_outseg(int      unit,
                                          bcm_fe2k_mim_port_control_t *portcb)
{
    int status = BCM_E_NONE;
    soc_sbx_g2p3_ft_t fte;
    int    fabport, fabunit, node;
    int    module, port;

    soc_sbx_g2p3_ft_t_init(&fte);

    if(portcb) {
        if(BCM_GPORT_IS_TRUNK(portcb->port)) {
            fte.lag = 1;
#define TRUNK_SBX_HASH_SIZE  3 /*fixme - copied from trunk.c, couldnt find lag size exposed */
#define TRUNK_SBX_FIXED_PORTCNT         (1<<TRUNK_SBX_HASH_SIZE)
#define TRUNK_INDEX_SET(tid, offset)                \
    (TRUNK_SBX_FIXED_PORTCNT > (offset)) ?          \
     ((((tid))<<(TRUNK_SBX_HASH_SIZE)) | (offset)) :  \
     -1
            fte.lagbase = TRUNK_INDEX_SET(BCM_GPORT_TRUNK_GET(portcb->port), 0);
            fte.lagsize = TRUNK_SBX_HASH_SIZE;
        } else {
            module = _SHR_GPORT_MODPORT_MODID_GET(portcb->port);
            port   = _SHR_GPORT_MODPORT_PORT_GET(portcb->port);


            /* obtain fabric info to program QID */
            status = soc_sbx_node_port_get(unit, module, port,
                                           &fabunit, &node,
                                           &fabport);
            if(BCM_FAILURE(status)) {
                MIM_ERR(("Error %s Failed to Get Fabric Port Node info !!!!(%s,%d)\n",\
                                    FUNCTION_NAME(),__FILE__,__LINE__));
            } else {
 	        fte.qid = SOC_SBX_NODE_PORT_TO_QID(unit,node,fabport, NUM_COS(unit));
                MIM_VERB(("unit %d: [%s] soc_sbx_node_port_get mod(0x%x) port(%d)"
                          "-> node(0x%x) port (%d) map to qid (0x%05x) unit(%d)\n",
                          unit, FUNCTION_NAME(), module, port, node,
                          fabport, fte.qid, fabunit));

            }
        }

        if(BCM_E_NONE == status) {
            /* map ohi to fte */
            fte.oi          = portcb->hwinfo.ohi;

            status = soc_sbx_g2p3_ft_set(unit,
                                         portcb->hwinfo.ftidx,
                                         &fte);
            if(BCM_FAILURE(status)) {
                MIM_ERR(("unit %d: [%s] error %s in programming FTE 0x%x in HW\n",
                         unit, FUNCTION_NAME(), bcm_errmsg(status), portcb->hwinfo.ftidx));
            } else {
                /* LP except for  default  back bone ports are dummy, so set could be eliminated ..*/
                /* provision lp */
                soc_sbx_g2p3_lp_t lp;

                soc_sbx_g2p3_lp_t_init(&lp);

                lp.pid = portcb->hwinfo.ftidx;

                if(portcb->policer_id > 0) {
                    /* for Back bone ports Allow policing only on ISID
                     * i.e., only on vpn isid2e LP */
                    if(portcb->type == _BCM_FE2K_MIM_DEF_BACKBONE_PORT ||
                       portcb->type == _BCM_FE2K_MIM_BACKBONE_PORT) {
                        soc_sbx_g2p3_lp_t isidlp;

                        status = soc_sbx_g2p3_lp_get(unit, portcb->isidlport, &isidlp);
                        if(BCM_SUCCESS(status)) {
                            status = _bcm_fe2000_g2p3_policer_lp_program(unit, 
                                                                     portcb->policer_id,
                                                                     &isidlp);              
                            if(BCM_SUCCESS(status)) {
                                status =  soc_sbx_g2p3_lp_set(unit, portcb->isidlport, &isidlp);

                                if(BCM_SUCCESS(status)) {
                                    MIM_VERB(("Debug %s Provisioned Policer %d on ISID LP 0x%x\n",\
                                              FUNCTION_NAME(),portcb->policer_id,portcb->isidlport));
                                } else {
                                    MIM_ERR(("unit %d: [%s] error %s in setting ISID LP 0x%x in HW\n",
                                             unit, FUNCTION_NAME(), bcm_errmsg(status), portcb->isidlport));
                                }
                            } else {
                                MIM_ERR(("unit %d: [%s] error %s Configuring Policer %d to  ISID LP 0x%x\n",
                                         unit, FUNCTION_NAME(), bcm_errmsg(status), 
                                         portcb->policer_id,
                                         portcb->isidlport));
                            }
                        } else {
                            MIM_ERR(("unit %d: [%s] error %s in reading ISID LP 0x%x in HW\n",
                                     unit, FUNCTION_NAME(), bcm_errmsg(status), portcb->isidlport));
                        }
                        
                    } else {
                        status = _bcm_fe2000_g2p3_policer_lp_program(unit, 
                                                                     portcb->policer_id,
                                                                     &lp);
                        if(BCM_SUCCESS(status)) {
                            status = soc_sbx_g2p3_lp_set(unit, portcb->hwinfo.lport, &lp);
                            if(BCM_FAILURE(status)) {
                                MIM_ERR(("ERROR: Setting LPORT: unit %d gport %08X LpIndex %d\n",
                                         unit, portcb->gport, portcb->hwinfo.lport));
                            } else {
                                MIM_VVERB(("Setting LPORT: unit %d gport %08X LpIndex %d PolicerId: 0x%x\n",
                                           unit, portcb->gport, portcb->hwinfo.lport, portcb->policer_id));                            
                            }
                        } else {
                            MIM_ERR(("ERROR %s Configured Policer %d on Access LP 0x%x\n",\
                                     FUNCTION_NAME(),portcb->policer_id,portcb->hwinfo.lport));
                        }
                    }
                }

                if(BCM_SUCCESS(status)) {
                    status = soc_sbx_g2p3_lp_set(unit,
                                                 portcb->hwinfo.lport,
                                                 &lp);
                    MIM_VERB((_MD("Set lpi 0x%x pid=0x%x\n"),
                              portcb->hwinfo.lport, lp.pid));
                }

                if(BCM_FAILURE(status)) {
                    MIM_ERR(("unit %d: [%s] error %s in programming LP 0x%x\n",
                              unit, FUNCTION_NAME(), bcm_errmsg(status), portcb->hwinfo.lport));
                }
            }
        }
    } else {
        status = BCM_E_PARAM;
    }

    return status;
}

static
int _bcm_fe2000_mim_g2p3_provision_outseg(int                          unit,
                                          int                          mymodid,
                                          bcm_fe2k_mim_port_control_t *portcb)
{
    int status = BCM_E_NONE;
    uint8 isRemote = TRUE;

    if(portcb && _MIM_PORT_TYPE_VALID_CHECK(portcb->type)) {
        bcm_trunk_add_info_t tdata;
        int index=0;

        /* If MIM port is over Trunk, Trunk table would have already allocated
         * required OI and OI would have already been reserved. So skip Encap allocation
         * completely */
        if(BCM_GPORT_IS_TRUNK(portcb->port)) {
            /* Compare mymodid with trunk module array, if any match allocate
             * egress resource else skip */
            status = bcm_trunk_get(unit, 
                                   BCM_GPORT_TRUNK_GET(portcb->port),
                                   &tdata);
            if(BCM_SUCCESS(status)) {
                for(index=0; index < tdata.num_ports; index++) {
                    if(tdata.tm[index] == mymodid) {
                        isRemote = FALSE;
                        break;
                    }
                }
            } else {
                MIM_ERR(("unit %d: [%s] Failed to get trunk information\n",
                         unit, FUNCTION_NAME()));
            }
        } else if(BCM_GPORT_MODPORT_MODID_GET(portcb->port) != mymodid) {
            /* program OI & ETE only if the mim port is on our Home unit */
            MIM_VERB(("Debug %s Non-Home Units doesnt require OI/ETE provisioning (%s,%d)\n",\
                      FUNCTION_NAME(),__FILE__,__LINE__));
        } else {
            isRemote = FALSE;
        }

        if(!isRemote) {

             soc_sbx_g2p3_eteencap_t    eteencap;
             soc_sbx_g2p3_etel2_t       etel2;

             /* start from lowel hierarchy to upper so there is no incomplete pointer */
             soc_sbx_g2p3_etel2_t_init(&etel2);
             soc_sbx_g2p3_eteencap_t_init(&eteencap);

             switch(portcb->type) {

                 case _BCM_FE2K_MIM_DEF_BACKBONE_PORT:
                     
                     /* Provision ETE for Default back bone port */
                     eteencap.l2ete       =  portcb->hwinfo.etel2;
                     eteencap.tunnelenter = 1;
                     eteencap.encapmac    = 1;
                     eteencap.encaplen    = 22;
                     eteencap.btag_vid    = portcb->vlan;
                     eteencap.btag_pricfi = 0; /*fixme*/
                     eteencap.btag_tpid   = (portcb->tpid > 0)?portcb->tpid:_BCM_FE2000_DEFAULT_BTAG_TPID;
                     eteencap.isid        = portcb->isid;
                     eteencap.mimtype     = _BCM_FE2000_MIM_ETYPE;
                     eteencap.etepid      = 0;
                     eteencap.nosplitcheck = 0;

                     /* if trunk do skout through ete pid */
                     if(BCM_GPORT_IS_TRUNK(portcb->port)) {
                         eteencap.etepid = 1;
                         eteencap.pid    = SOC_SBX_TRUNK_FTE(unit, BCM_GPORT_TRUNK_GET(portcb->port));
                     }

                     /* Set BDMAC to default back bone group bmac based on isid */
                     /*01_1E_83*/
                     eteencap.dmacset = 1;
                     eteencap.dmac5   = portcb->isid & 0xFF;
                     eteencap.dmac4   = (portcb->isid >> 8) & 0xFF;
                     eteencap.dmac3   = (portcb->isid >> 16) & 0xFF;
                     eteencap.dmac2   = 0x83;
                     eteencap.dmac1   = 0x1E;
                     eteencap.dmac0   = 0x01;

                     /* copy tunnel mac */
                     eteencap.smacset = 1;

                     
                     etel2.stpcheck  = 1;
                     etel2.usevid    = 1;
                     etel2.vid       = 0xFFF;
                     etel2.smacindex = portcb->hwinfo.esmacidx;
                     etel2.mtu       = SBX_DEFAULT_MTU_SIZE;
                     break;

                 case _BCM_FE2K_MIM_BACKBONE_PORT:
                     
                     eteencap.l2ete       =  portcb->hwinfo.etel2;
                     eteencap.tunnelenter = 1;
                     eteencap.encapmac    = 1;
                     eteencap.encaplen    = 22;
                     eteencap.btag_vid    = portcb->vlan;
                     eteencap.btag_pricfi = 0; /*fixme*/
                     eteencap.btag_tpid   = (portcb->tpid > 0)?portcb->tpid:_BCM_FE2000_DEFAULT_BTAG_TPID;
                     eteencap.isid        = portcb->isid;
                     eteencap.mimtype     = 1;
                     eteencap.etepid      = 0;
                     eteencap.mimtype     = _BCM_FE2000_MIM_ETYPE;
                     eteencap.nosplitcheck= 0;

                     /* if trunk do skout through ete pid */
                     if(BCM_GPORT_IS_TRUNK(portcb->port)) {
                         eteencap.etepid = 1;
                         eteencap.pid    = SOC_SBX_TRUNK_FTE(unit, BCM_GPORT_TRUNK_GET(portcb->port));
                     }

                     /* copy tunnel mac */
                     eteencap.smacset = 1;
                     eteencap.dmacset = 1;
                     eteencap.dmac5   = portcb->dmac[5];
                     eteencap.dmac4   = portcb->dmac[4];
                     eteencap.dmac3   = portcb->dmac[3];
                     eteencap.dmac2   = portcb->dmac[2];
                     eteencap.dmac1   = portcb->dmac[1];
                     eteencap.dmac0   = portcb->dmac[0];

                     etel2.stpcheck  = 1;
                     etel2.usevid    = 1;
                     etel2.vid       = 0xFFF;
                     etel2.smacindex = portcb->hwinfo.esmacidx;
                     etel2.mtu       = SBX_DEFAULT_MTU_SIZE;
                 break;

                 /* Access port */
                 case _BCM_FE2K_MIM_ACCESS_PORT:

                     /* Setting will vary based on STAG interface or Port Interface */
                     eteencap.l2ete   =  portcb->hwinfo.etel2;
                     eteencap.tunnelenter = 0;
                     eteencap.encapmac    = 0;
                     eteencap.dmacset     = 0;
                     eteencap.smacset     = 0;
                     eteencap.nosplitcheck = 0;
                     eteencap.etepid = 1;
                     eteencap.pid    = portcb->hwinfo.ftidx;

                     etel2.stpcheck   = 1;
                     etel2.mtu        = SBX_DEFAULT_MTU_SIZE;

                     /* Based on Service Interface Configure VID on ETE */
                     if(portcb->service == _BCM_FE2K_MIM_PORT) {
                         soc_sbx_g2p3_p2e_t  p2e;
                         /* use native vid on ete */

                         /* update native vid from p2e */
                         status = soc_sbx_g2p3_p2e_get(unit,
                                                       BCM_GPORT_MODPORT_PORT_GET(portcb->port),
                                                       &p2e);
                         if(BCM_SUCCESS(status)) {
                             etel2.usevid    = 1;
                             etel2.vid       = p2e.nativevid;
                         }

                     } else if(portcb->service == _BCM_FE2K_MIM_STAG_1_1){
                         /* use stag on ete */
                         etel2.usevid    = 1;
                         etel2.vid       = portcb->vlan;
                     } else if(portcb->service == _BCM_FE2K_MIM_STAG_BUNDLED){
                         /* do nothing */
                     } else {
                         MIM_ERR(("unit %d: [%s] Unsupported Service type\n",unit, FUNCTION_NAME()));
                         status = BCM_E_PARAM;
                     }
                     break;

                 default:
                     /* should never hit here - just to keep compiler happy*/
                     MIM_ERR(("unit %d: [%s] BAD port type\n",unit, FUNCTION_NAME()));
                     status = BCM_E_PARAM;
                     break;
             }

             
             eteencap.ttlcheck = 0;
             eteencap.ipttldec = 0;

             if (eteencap.smacset) {
                 soc_sbx_g2p3_esmac_t esmac;
                 int esmacIdx = portcb->hwinfo.esmacidx;

                 esmac.smac5 = portcb->smac[5];  esmac.smac4 = portcb->smac[4];
                 esmac.smac3 = portcb->smac[3];  esmac.smac2 = portcb->smac[2];
                 esmac.smac1 = portcb->smac[1];  esmac.smac0 = portcb->smac[0];

                 status = soc_sbx_g2p3_esmac_set(unit, esmacIdx, &esmac);
                 if (BCM_FAILURE(status)) {
                     MIM_ERR((_MD("failed to set esmac idx=%d: %d (%s)\n"),
                              esmacIdx, status, bcm_errmsg(status)));
                 }
                 MIM_VERB((_MD("Set esmac idx %d " MAC_FMT " \n"),
                           esmacIdx, MAC_PFMT(portcb->smac)));
             }


             if(BCM_SUCCESS(status)) {
                 status = soc_sbx_g2p3_etel2_set(unit,
                                                 portcb->hwinfo.etel2,
                                                 &etel2);
                 if(BCM_FAILURE(status)) {
                    MIM_ERR(("unit %d: [%s] error %s in programming L2 ETE 0x%x in HW\n",
                             unit, FUNCTION_NAME(), bcm_errmsg(status), portcb->hwinfo.etel2));
                 } else {
                     status = soc_sbx_g2p3_eteencap_set(unit,
                                                        portcb->hwinfo.eteencap,
                                                        &eteencap);
                     if(BCM_FAILURE(status)) {
                         MIM_ERR(("unit %d: [%s] error %s in programming ENACP ETE 0x%x in HW\n",
                                   unit, FUNCTION_NAME(), bcm_errmsg(status), portcb->hwinfo.eteencap));
                     } else {
                         soc_sbx_g2p3_oi2e_t        ohi2etc;
                         soc_sbx_g2p3_oi2e_t_init(&ohi2etc);
                         ohi2etc.eteptr = portcb->hwinfo.eteencap;
                         status = soc_sbx_g2p3_oi2e_set(unit,
                                                        _BCM_FE2000_G2P3_ADJUST_TB_OFFSET(portcb->hwinfo.ohi),
                                                        &ohi2etc);
                         if(BCM_E_NONE != status) {
                            MIM_ERR(("unit %d: [%s] error %s in programming OI 0x%x in HW\n",
                                      unit, FUNCTION_NAME(), bcm_errmsg(status), portcb->hwinfo.ohi));
                         }
                     }
                 }
             }
        }

    } else {
        status = BCM_E_PARAM;
    }

    return status;
}

/* Fixme - expose on g2p3.tables */
#define MIM_ACCESS_PORT_MODE       (1)
#define MIM_ACCESS_1_1_MODE        (2)
#define MIM_ACCESS_BUNDLE_MODE     (3)
#define MIM_BBONE_PORT             (4)

static
int _bcm_fe2000_mim_config_port_interface(int             unit,
                                          bcm_mim_port_t *mim_port,
                                          bcm_fe2k_mim_port_control_t *portcb)
{
    int status = BCM_E_NONE, port=0, index, mymodid;
    soc_sbx_g2p3_p2e_t  p2e;
    soc_sbx_g2p3_ep2e_t ep2e;
    soc_sbx_g2p3_pv2e_t pv2e;
    soc_sbx_g2p3_lp_t lp;
    soc_sbx_g2p3_epv2e_t epv2e;
    uint32             *vlanvector;
    uint32             port_mirror = 0;

    if(!mim_port || !portcb) {
        return BCM_E_PARAM;
    }

    status = bcm_sbx_stk_modid_get(unit, &mymodid);
    if(BCM_FAILURE(status)) {
        MIM_ERR(("ERROR %s Could not obtain my module ID !!!!(%s,%d)\n",\
                 FUNCTION_NAME(),__FILE__,__LINE__));
        return BCM_E_INTERNAL;
    }

    /* only mod port supported for now */
    if(!BCM_GPORT_IS_MODPORT(mim_port->port)) {
        MIM_ERR((_MD("Invalid gport:  gport=0x%08x\n"),
                 mim_port->port));
        return BCM_E_PARAM;
    }

    /* If this is a remote module skip port mode configuration */
    if(mymodid != BCM_GPORT_MODPORT_MODID_GET(mim_port->port)) {
        MIM_VERB(("Skipping Port configuration since module is remote"));
        return BCM_E_NONE;
    }

    port = BCM_GPORT_MODPORT_PORT_GET(mim_port->port);
    if (_MIM_PORT_INVALID(unit, port)) {
        MIM_ERR((_MD("Invalid port: %d, gport=0x%08x\n"),
                 port, mim_port->port));
        return BCM_E_PARAM;
    }

    vlanvector = sal_alloc(sizeof(uint32) * (BCM_VLAN_MAX + 1), "work buffer");

    if (vlanvector == NULL) {
        return BCM_E_PARAM;
    }
    sal_memset(vlanvector, 0, sizeof(uint32) * (BCM_VLAN_MAX + 1));


    portcb->service = _BCM_FE2K_MIM_PORT;

    /* GNATS 35584
     *   Need to clean pvv2e when  aport mode pbb access port is created
     */
    status = soc_sbx_g2p3_pvv2e_delete(unit, 0xFFF, 0x000, port);
    if (BCM_E_NOT_FOUND == status) {
       /*  Don't really care that we couldn't delete the pvv2e
        *  entry if it's not here to be deleted...
        */
        status = BCM_E_NONE;
    }

    /* set mim mode on p2e */
    if(((soc_sbx_g2p3_p2e_get(unit, port, &p2e)) == BCM_E_NONE) &&
       ((soc_sbx_g2p3_ep2e_get(unit, port, &ep2e)) == BCM_E_NONE)) {

        p2e.mim         = MIM_ACCESS_PORT_MODE;
        p2e.customer    = 0;
        p2e.provider    = 1;
        p2e.defstrip    = 1;
        p2e.pstrip      = 0;
        p2e.pbb         = 1;

        ep2e.customer   = 1;
        ep2e.mim        = MIM_ACCESS_PORT_MODE;
        ep2e.pbb         = 1;
        ep2e.stpid0     = 0;
        ep2e.stpid1     = 0;

        soc_sbx_g2p3_pv2e_t_init(&pv2e);
        soc_sbx_g2p3_epv2e_t_init(&epv2e);

        pv2e.untagged_strip = 1;
        pv2e.vlan = portcb->isidvlan; /* Native VID resolves to ISID VSI */
        pv2e.lpi  = portcb->hwinfo.lport;

	/* GNATS 33852 */
        
            soc_sbx_g2p3_lp_t_init(&lp);
            status = soc_sbx_g2p3_lp_get(unit, port, &lp);
            if(BCM_FAILURE(status)) {
                MIM_ERR(("unit %d: [%s] error %s in reading LP 0x%x\n",
                unit, FUNCTION_NAME(), bcm_errmsg(status),port));
            }
            if (lp.mirror) {
                port_mirror = lp.mirror;
                soc_sbx_g2p3_lp_t_init(&lp);
                status = soc_sbx_g2p3_lp_get(unit, pv2e.lpi, &lp);
                if(BCM_FAILURE(status)) {
                    MIM_ERR(("unit %d: [%s] error %s in reading LP 0x%x\n",
                    unit, FUNCTION_NAME(), bcm_errmsg(status),pv2e.lpi));
                }
                lp.mirror = port_mirror;
                status = soc_sbx_g2p3_lp_set(unit, pv2e.lpi, &lp);
                if(BCM_FAILURE(status)) {
                    MIM_ERR(("unit %d: [%s] error %s in programming LP 0x%x\n",
                    unit, FUNCTION_NAME(), bcm_errmsg(status),pv2e.lpi));
                }
            }

        epv2e.strip = 1; /* Native VID is added and then stripped back on egress */

        for(index=0; index < BCM_VLAN_MAX; index++) {
            vlanvector[index] = 0;
        }

        status = soc_sbx_g2p3_pv2e_vlan_fast_set(unit, 0, port, BCM_VLAN_MAX, port,
                                                 NULL, &vlanvector[0], BCM_VLAN_MAX + 1);
        if(BCM_SUCCESS(status)) {

            /* Program Native VID */
            status = soc_sbx_g2p3_pv2e_set(unit, p2e.nativevid, port, &pv2e);

            if(BCM_SUCCESS(status)) {
                /* Set EPV2E */
                status = soc_sbx_g2p3_epv2e_set(unit, p2e.nativevid, port, &epv2e);

                if(BCM_SUCCESS(status)) {
                    /* program P2E */
                    status = soc_sbx_g2p3_p2e_set(unit, port, &p2e);

                    if(BCM_SUCCESS(status)) {
                        /* program EP2E */
                        status = soc_sbx_g2p3_ep2e_set(unit, port, &ep2e);

                        if(BCM_FAILURE(status)) {

                            MIM_ERR(("ERROR %s Programming EP2E %d : %s in (%s,%d)\n",
                                     FUNCTION_NAME(), status, bcm_errmsg(status),
                                     __FILE__,__LINE__));
                        }

                    } else {
                        MIM_ERR(("ERROR %s Programming P2E %d : %s in (%s,%d)\n",
                                 FUNCTION_NAME(), status, bcm_errmsg(status),
                                 __FILE__,__LINE__));
                    }
                } else {
                    MIM_ERR(("ERROR %s Programming EPV2E %d : %s in (%s,%d)\n",
                             FUNCTION_NAME(), status, bcm_errmsg(status),
                             __FILE__,__LINE__));
                }
            } else {
                MIM_ERR(("ERROR %s Programming PV2E %d : %s in (%s,%d)\n",
                         FUNCTION_NAME(), status, bcm_errmsg(status),
                         __FILE__,__LINE__));
            }
        } else {
            MIM_ERR(("ERROR %s Programming PV2E Vector %d : %s in (%s,%d)\n",
                     FUNCTION_NAME(), status, bcm_errmsg(status),
                     __FILE__,__LINE__));
        }
    }

    sal_free(vlanvector);
    return status;
}

static
int _bcm_fe2000_mim_reset_port_interface(int             unit,
                                         bcm_fe2k_mim_port_control_t *portcb)
{
    int status = BCM_E_NONE, port, index, mymodid;
    uint32  *vector;

    if(!portcb) {
        return BCM_E_PARAM;
    }

    status = bcm_sbx_stk_modid_get(unit, &mymodid);
    if(BCM_FAILURE(status)) {
        MIM_ERR(("ERROR %s Could not obtain my module ID !!!!(%s,%d)\n",\
                 FUNCTION_NAME(),__FILE__,__LINE__));
        return BCM_E_INTERNAL;
    }

    /* only mod port supported for now */
    if(!BCM_GPORT_IS_MODPORT(portcb->port)) {
        MIM_ERR((_MD("Invalid gport: gport=0x%08x\n"),
                 portcb->port));
        return BCM_E_PARAM;
    }

    /* If this is a remote module skip port mode configuration */
    if(mymodid != BCM_GPORT_MODPORT_MODID_GET(portcb->port)) {
        MIM_VERB(("Skipping Port configuration since module is remote"));
        return BCM_E_NONE;
    }

    port = BCM_GPORT_MODPORT_PORT_GET(portcb->port);
    if (_MIM_PORT_INVALID(unit, port)) {
        MIM_ERR((_MD("Invalid port: %d, gport=0x%08x\n"),
                 port, portcb->port));
        return BCM_E_PARAM;
    }

    vector = sal_alloc(sizeof(uint32) * (BCM_VLAN_MAX + 1), "work buffer");

    if (vector == NULL) {
        return BCM_E_PARAM;
    }

    for(index=0; index < BCM_VLAN_MAX; index++) {
        vector[index] = index;
    }
    
    status = soc_sbx_g2p3_pv2e_vlan_fast_set(unit, 0, port, BCM_VLAN_MAX, port,
                                             NULL, &vector[0], BCM_VLAN_MAX + 1);
    if(BCM_SUCCESS(status)) {
        MIM_ERR(("ERROR %s Resetting PV2E Vector VSI for Port Interface on port %d in (%s,%d)", 
                 FUNCTION_NAME(),port,__FILE__,__LINE__));
    } else {
        for(index=0; index < BCM_VLAN_MAX; index++) {
            vector[index] = 0;
        }
        status = soc_sbx_g2p3_pv2e_lpi_fast_set(unit, 0, port, BCM_VLAN_MAX, port,
                                                 NULL, &vector[0], BCM_VLAN_MAX + 1);      
        if(BCM_SUCCESS(status)) {
            MIM_ERR(("ERROR %s Resetting PV2E Vector LPI for Port Interface on port %d in (%s,%d)", 
                     FUNCTION_NAME(),port,__FILE__,__LINE__));
        } 
    }

    sal_free(vector);
    return status;
}

static
int _bcm_fe2000_mim_config_stag_interface(int             unit,
                                          bcm_mim_port_t *mim_port,
                                          bcm_fe2k_mim_port_control_t *portcb)
{
    int status = BCM_E_NONE, port, mymodid;
    soc_sbx_g2p3_p2e_t  p2e;
    soc_sbx_g2p3_ep2e_t ep2e;
    soc_sbx_g2p3_pv2e_t pv2e;
    soc_sbx_g2p3_lp_t lp;
    soc_sbx_g2p3_epv2e_t epv2e;
    uint32             port_mirror = 0;

    if(!mim_port || !portcb) {
        return BCM_E_PARAM;
    }

    status = bcm_sbx_stk_modid_get(unit, &mymodid);
    if(BCM_FAILURE(status)) {
        MIM_ERR(("ERROR %s Could not obtain my module ID !!!!(%s,%d)\n",\
                 FUNCTION_NAME(),__FILE__,__LINE__));
        return BCM_E_INTERNAL;
    }

    /* only mod port supported for now */
    if(!BCM_GPORT_IS_MODPORT(mim_port->port)) {
        MIM_ERR((_MD("Invalid gport: gport=0x%08x\n"),
                 mim_port->port));
        return BCM_E_PARAM;
    }

    /* If this is a remote module skip port mode configuration */
    if(mymodid != BCM_GPORT_MODPORT_MODID_GET(mim_port->port)) {
        MIM_VERB(("Skipping Port configuration since module is remote"));
        return BCM_E_NONE;
    }

    port = BCM_GPORT_MODPORT_PORT_GET(mim_port->port);
    if (_MIM_PORT_INVALID(unit, port)) {
        MIM_ERR((_MD("Invalid port: %d, gport=0x%08x\n"),
                 port, mim_port->port));
        return BCM_E_PARAM;
    }

    /* GNATS 35584
     * need to delete pvv2e when a pbb access port is created
     * need to clean pv2e when a tag-mode pbb access port is created
     */
    status = soc_sbx_g2p3_pvv2e_delete(unit, 0xFFF, 0x000, port);
    if (BCM_E_NOT_FOUND == status) {
       /*  Don't really care that we couldn't delete the pvv2e
        *  entry if it's not here to be deleted...
        */
        status = BCM_E_NONE;
    }
        

    /* set mim mode on p2e */
    if(((soc_sbx_g2p3_p2e_get(unit, port, &p2e)) == BCM_E_NONE) &&
       ((soc_sbx_g2p3_ep2e_get(unit, port, &ep2e)) == BCM_E_NONE)) {

        p2e.mim         = MIM_ACCESS_1_1_MODE;
        p2e.customer    = 0;
        p2e.provider    = 1;
        p2e.defstrip    = 0;
        p2e.pstrip      = 1;
        p2e.pbb         = 1;

        ep2e.customer   = 0; /* for Bundled mode, customer bit has to be one */
        ep2e.stpid0     = 1;
        ep2e.stpid1     = 0;
        ep2e.mim        = MIM_ACCESS_1_1_MODE;
        ep2e.pbb         = 1;

        soc_sbx_g2p3_pv2e_t_init(&pv2e);
        soc_sbx_g2p3_epv2e_t_init(&epv2e);

        /* bundle mode */
        if (portcb->service == _BCM_FE2K_MIM_STAG_BUNDLED) {
           p2e.mim         = MIM_ACCESS_BUNDLE_MODE;
           ep2e.customer   = 1; /* customer bit has to be one */
           ep2e.mim        = MIM_ACCESS_BUNDLE_MODE;
        }

        pv2e.vlan = portcb->isidvlan;
        pv2e.lpi  = portcb->hwinfo.lport;

	/* GNATS 33852 */
        
            soc_sbx_g2p3_lp_t_init(&lp);
            status = soc_sbx_g2p3_lp_get(unit, port, &lp);
            if(BCM_FAILURE(status)) {
                MIM_ERR(("unit %d: [%s] error %s in reading LP 0x%x\n",
                unit, FUNCTION_NAME(), bcm_errmsg(status),port));
            }
            if (lp.mirror) {
                port_mirror = lp.mirror;
                soc_sbx_g2p3_lp_t_init(&lp);
                status = soc_sbx_g2p3_lp_get(unit, pv2e.lpi, &lp);
                if(BCM_FAILURE(status)) {
                    MIM_ERR(("unit %d: [%s] error %s in reading LP 0x%x\n",
                    unit, FUNCTION_NAME(), bcm_errmsg(status),pv2e.lpi));
                }
                lp.mirror = port_mirror;
                status = soc_sbx_g2p3_lp_set(unit, pv2e.lpi, &lp);
                if(BCM_FAILURE(status)) {
                    MIM_ERR(("unit %d: [%s] error %s in programming LP 0x%x\n",
                    unit, FUNCTION_NAME(), bcm_errmsg(status),pv2e.lpi));
                }
            }
    
        epv2e.strip = 0; /* dont strip tags at egress */

        /* Program Native VID */
        status = soc_sbx_g2p3_pv2e_set(unit, mim_port->match_vlan, port, &pv2e);

        if(BCM_SUCCESS(status)) {
            /* Set EPV2E */
            status = soc_sbx_g2p3_epv2e_set(unit, mim_port->egress_service_vlan, port, &epv2e);

            if(BCM_SUCCESS(status)) {
                /* program P2E */
                status = soc_sbx_g2p3_p2e_set(unit, port, &p2e);

                if(BCM_SUCCESS(status)) {
                    /* program EP2E */
                    status = soc_sbx_g2p3_ep2e_set(unit, port, &ep2e);

                    if(BCM_FAILURE(status)) {

                        MIM_ERR(("ERROR %s Programming EP2E %d : %s in (%s,%d)\n",
                                 FUNCTION_NAME(), status, bcm_errmsg(status),
                                 __FILE__,__LINE__));
                    }

                } else {
                    MIM_ERR(("ERROR %s Programming P2E %d : %s in (%s,%d)\n",
                             FUNCTION_NAME(), status, bcm_errmsg(status),
                             __FILE__,__LINE__));
                }
            } else {
                MIM_ERR(("ERROR %s Programming EPV2E %d : %s in (%s,%d)\n",
                         FUNCTION_NAME(), status, bcm_errmsg(status),
                         __FILE__,__LINE__));
            }
        } else {
            MIM_ERR(("ERROR %s Programming PV2E %d : %s in (%s,%d)\n",
                     FUNCTION_NAME(), status, bcm_errmsg(status),
                     __FILE__,__LINE__));
        }

    } else {
        MIM_ERR(("ERROR %s Getting P2E & EP2E in (%s,%d)\n",
                 FUNCTION_NAME(), __FILE__,__LINE__));
    }
    return status;
}

static
int _bcm_fe2000_mim_config_bbone_interface(int             unit,
                                           bcm_mim_port_t *mim_port,
                                           bcm_fe2k_mim_port_control_t *portcb)
{
    soc_sbx_g2p3_p2e_t  p2e;
    soc_sbx_g2p3_ep2e_t ep2e;
    soc_sbx_g2p3_pv2e_t pv2e;
    int port=0, status = BCM_E_NONE;
    soc_sbx_g2p3_lsmac_t lsmmac;
    bcm_trunk_add_info_t tdata;
    bcm_port_t tp[BCM_TRUNK_MAX_PORTCNT];
    int index=0, mymodid, tnumports=1, pindex=0, idx=0;

    if(!mim_port || !portcb) {
        return BCM_E_PARAM;
    }

    /* Only allowed for Default Back Bone ports */
    if(_BCM_FE2K_MIM_DEF_BACKBONE_PORT != portcb->type) {
        return BCM_E_PARAM;
    }

    status = bcm_sbx_stk_modid_get(unit, &mymodid);
    if(BCM_FAILURE(status)) {
        MIM_ERR(("ERROR %s Could not obtain my module ID !!!!(%s,%d)\n",\
                 FUNCTION_NAME(),__FILE__,__LINE__));
        return BCM_E_INTERNAL;
    }

    if(BCM_GPORT_IS_TRUNK(portcb->port)) {
        /* Compare mymodid with trunk module array, if any match allocate
         * egress resource else skip */
        status = bcm_trunk_get(unit, 
                               BCM_GPORT_TRUNK_GET(portcb->port),
                               &tdata);
        if(BCM_FAILURE(status)) {
            MIM_ERR((_MD("Trunk Get Failure: %d, gport=0x%08x\n"),
                     status, mim_port->port));
            return BCM_E_INTERNAL;
        }
        tnumports = tdata.num_ports;
        for(index=0; index <BCM_TRUNK_MAX_PORTCNT; index++) {
            tp[index] = -1;
        }
 
    } else {

        /* If this is a remote module skip port mode configuration */
        if(mymodid != BCM_GPORT_MODPORT_MODID_GET(mim_port->port)) {
            MIM_VERB(("Skipping Port configuration since module is remote"));
            return BCM_E_NONE;
        }
        
        port = BCM_GPORT_MODPORT_PORT_GET(mim_port->port);
        if (_MIM_PORT_INVALID(unit, port)) {
            MIM_ERR((_MD("Invalid port: %d, gport=0x%08x\n"),
                     port, mim_port->port));
            return BCM_E_PARAM;
        }
        tnumports=1;
    }

    /* set local station match entry on ppe */
    soc_sbx_g2p3_lsmac_t_init(&lsmmac);

    lsmmac.useport = 0;
    lsmmac.mac[0] = portcb->smac[0];
    lsmmac.mac[1] = portcb->smac[1];
    lsmmac.mac[2] = portcb->smac[2];
    lsmmac.mac[3] = portcb->smac[3];
    lsmmac.mac[4] = portcb->smac[4];
    lsmmac.mac[5] = portcb->smac[5];

    status = soc_sbx_g2p3_lsmac_set (unit, portcb->hwinfo.ismacidx, &lsmmac);
    /* set mim mode on p2e */
    if(BCM_SUCCESS(status)) {
        for(index=0; index < tnumports; index++) {

            if(BCM_GPORT_IS_TRUNK(portcb->port)) {
                if(tdata.tm[index] == mymodid) {
                    port = tdata.tp[index];
                    idx = 0;
                    /* verify if this port was already taken care due to 
                     * duplicate trunk distribution */
                    while(tp[idx] >= 0) {
                        if(port == tp[idx]) {
                            break;
                        }
                        idx++;
                    }

                    if(port == tp[idx]) {
                        continue;
                    }

                    tp[pindex++] = port;
                } else {
                    continue;
                }
            }

            if(((soc_sbx_g2p3_p2e_get(unit, port, &p2e)) == BCM_E_NONE) &&
               ((soc_sbx_g2p3_ep2e_get(unit, port, &ep2e)) == BCM_E_NONE)) {

                p2e.mim         = MIM_BBONE_PORT;
                p2e.customer    = 0;
                p2e.provider    = 1;
                p2e.defstrip    = 0;
                p2e.pstrip      = 0;
                p2e.pbb         = 1;

                /* if trunk set trunk base FTE as PID (or) set SID as PID */
                ep2e.customer   = 1;
                ep2e.mim        = MIM_BBONE_PORT;
                ep2e.pbb        = 1;

                /* program P2E */
                status = soc_sbx_g2p3_p2e_set(unit, port, &p2e);

                if(BCM_SUCCESS(status)) {
                    /* program EP2E */
                    status = soc_sbx_g2p3_ep2e_set(unit, port, &ep2e);

                    if(BCM_FAILURE(status)) {

                        MIM_ERR(("ERROR %s Programming EP2E %d : %s in (%s,%d)\n",
                                 FUNCTION_NAME(), status, bcm_errmsg(status),
                                 __FILE__,__LINE__));
                    } else {
                        soc_sbx_g2p3_pv2e_t_init(&pv2e);

                        status = soc_sbx_g2p3_pv2e_get(unit, portcb->vlan /*bvlan*/, port, &pv2e);
                        if(BCM_FAILURE(status)) {

                            MIM_ERR(("ERROR %s Getting PV2E %d : %s in (%s,%d)\n",
                                     FUNCTION_NAME(), status, bcm_errmsg(status),
                                     __FILE__,__LINE__));
                        } else {
                            /* Program Bvlan, port 2 Etc -> LPI */
                            /* lpi on pv2e for back bone must be 0 to use physical port as lp */
                            pv2e.lpi = 0;

                            status =  soc_sbx_g2p3_pv2e_set(unit, portcb->vlan /*bvlan*/, port, &pv2e);
                            if(BCM_FAILURE(status)) {

                                MIM_ERR(("ERROR %s Programming PV2E %d : %s in (%s,%d)\n",
                                         FUNCTION_NAME(), status, bcm_errmsg(status),
                                         __FILE__,__LINE__));
                            }
                        }
                    }

                } else {
                    MIM_ERR(("ERROR %s Programming P2E %d : %s in (%s,%d)\n",
                             FUNCTION_NAME(), status, bcm_errmsg(status),
                             __FILE__,__LINE__));
                }

            } else {
                MIM_ERR(("ERROR %s Getting P2E & EP2E in (%s,%d)\n",
                         FUNCTION_NAME(), __FILE__,__LINE__));
            }
        }
    }
    return status;
}

static
int _bcm_fe2000_mim_create_access_port(int                          unit,
                                       bcm_mim_port_t              *mim_port,
                                       bcm_fe2k_mim_port_control_t *portcb)
{
    int status = BCM_E_NONE, result;
    int mymodid = 0;

    if(!mim_port || !portcb) {
        status = BCM_E_PARAM;
        MIM_ERR(("WARNING %s Bad Parameter !!!!(%s,%d)\n",\
                                FUNCTION_NAME(),__FILE__,__LINE__));
    } else {
        result = bcm_sbx_stk_modid_get(unit, &mymodid);

        if(BCM_FAILURE(result)) {
            MIM_ERR(("ERROR %s Could not obtain my module ID !!!!(%s,%d)\n",\
                      FUNCTION_NAME(),__FILE__,__LINE__));
            status = BCM_E_INTERNAL;
        } else {
            if(mim_port->criteria & BCM_MIM_PORT_MATCH_PORT) {
                /* Configure Port Based Interface */
                portcb->service = _BCM_FE2K_MIM_PORT;

            } else if ((mim_port->criteria & BCM_MIM_PORT_MATCH_PORT_VLAN) &&
                           (mim_port->flags & BCM_MIM_PORT_EGRESS_SERVICE_VLAN_ADD)){
                /* Configure 1:1 STAG Based Interface */
                portcb->service = _BCM_FE2K_MIM_STAG_1_1;
                portcb->vlan    = mim_port->egress_service_vlan;

            } else if (mim_port->criteria & BCM_MIM_PORT_MATCH_PORT_VLAN) {
                /* Configure Bundled STAG Based Interface */
                portcb->service = _BCM_FE2K_MIM_STAG_BUNDLED;
                portcb->vlan    = mim_port->match_vlan;

            } else {
                /* should never hit here */
            }

            /* ALLOCATE LP, FT, ENCAPID (OI), ENCAP ETE AND L2 ETE */
            status = _bcm_fe2000_mim_g2p3_alloc_inseg(unit,
                                                      mim_port->flags,
                                                      portcb);
            if(BCM_SUCCESS(status)) {
                status = _bcm_fe2000_mim_g2p3_alloc_outseg(unit,
                                                           mymodid,
                                                           mim_port->flags,
                                                           &mim_port->encap_id,
                                                           portcb);
                if(BCM_SUCCESS(status)) {
                    /* Map OI to Ete  */
                    status = _bcm_fe2000_mim_g2p3_provision_outseg(unit,
                                                                   mymodid,
                                                                   portcb);
                    if(BCM_SUCCESS(status)) {
                        /* Map FT to Encap */
                        status = _bcm_fe2000_mim_g2p3_map_inseg_outseg(unit,
                                                                       portcb);
                    }
                }/* alloc_outseg */
            }/* alloc_inseg */

            if(BCM_SUCCESS(status)) {
                /* Provision access interface setting */
                switch(portcb->service) {
                case _BCM_FE2K_MIM_PORT:
                    /* Configure Port Based Interface */
                    status = _bcm_fe2000_mim_config_port_interface(unit, mim_port, portcb);
                    break;

                case _BCM_FE2K_MIM_STAG_1_1:
                    /* Configure 1:1 STAG Based Interface */
                case _BCM_FE2K_MIM_STAG_BUNDLED:
                    /* Configure Bundled STAG Based Interface */
                    status = _bcm_fe2000_mim_config_stag_interface(unit, mim_port, portcb);
                    break;

                default:
                    /* Should never hit here since port is validated for unsupported
                     * flags & criteria */
                    status = BCM_E_PARAM;
                    break;
                }
            }

            /* Free back resources if allocation failed */
            if(BCM_FAILURE(status)){
                _bcm_fe2000_mim_g2p3_free_inseg(unit,
                                                portcb);

                _bcm_fe2000_mim_g2p3_free_outseg(unit,
                                                 portcb);
            }
        }/* my mod id get */
    }

    return status;
}

static
int _bcm_fe2000_mim_create_back_bone_port(int                          unit,
                                          bcm_mim_port_t              *mim_port,
                                          bcm_fe2k_mim_port_control_t *portcb,
                                          bcm_fe2k_mim_port_control_t *defbbportcb)
{
    int status = BCM_E_NONE, result;
    int mymodid = 0;

    if(!mim_port || !portcb) {
        status = BCM_E_PARAM;
        MIM_ERR(("WARNING %s Bad Parameter !!!!(%s,%d)\n",\
                  FUNCTION_NAME(),__FILE__,__LINE__));
    } else {

        result = bcm_sbx_stk_modid_get(unit, &mymodid);

        if(BCM_FAILURE(result)) {
            MIM_ERR(("ERROR %s Could not obtain my module ID !!!!(%s,%d)\n",\
                      FUNCTION_NAME(),__FILE__,__LINE__));
            status = BCM_E_INTERNAL;
        }

        if((_BCM_FE2K_MIM_DEF_BACKBONE_PORT != portcb->type) && 
           (!defbbportcb)) {
            status = BCM_E_PARAM;
        }

        if(BCM_SUCCESS(status)) {

            /* ALLOCATE LP, FT, ENCAPID (OI), ENCAP ETE AND L2 ETE */
            status = _bcm_fe2000_mim_g2p3_alloc_inseg(unit,
                                                      mim_port->flags,
                                                      portcb);

            if(BCM_SUCCESS(status)) {

                status = _bcm_fe2000_mim_g2p3_alloc_outseg(unit,
                                                           mymodid,
                                                           mim_port->flags,
                                                           &mim_port->encap_id,
                                                           portcb);
                if(BCM_SUCCESS(status)) {
                    /* Map OI to Ete */
                    status = _bcm_fe2000_mim_g2p3_provision_outseg(unit,
                                                                   mymodid,
                                                                   portcb);
                    if(BCM_SUCCESS(status)) {
                        /* Map FT to Encap */
                        status = _bcm_fe2000_mim_g2p3_map_inseg_outseg(unit,
                                                                       portcb);
                    }
                }/* alloc_outseg */
            }/* alloc_inseg */

            /* configure default back bone port */
            if(BCM_SUCCESS(status)) {
               if(_BCM_FE2K_MIM_DEF_BACKBONE_PORT == portcb->type) {
                   status = _bcm_fe2000_mim_config_bbone_interface(unit, mim_port, portcb);
               } else {
                    /* create bsmac */
                    soc_sbx_g2p3_bmac_t bsmac;
                    int module, port, fabport, fabunit, fabnode;

                    soc_sbx_g2p3_bmac_t_init(&bsmac);

                    if(BCM_GPORT_IS_TRUNK(portcb->port)) {
                        bsmac.bpid = SOC_SBX_TRUNK_FTE(unit, BCM_GPORT_TRUNK_GET(portcb->port));
                    } else {
                        /* obtain physical SID */
                        module = BCM_GPORT_MODPORT_MODID_GET(portcb->port);
                        port   = BCM_GPORT_MODPORT_PORT_GET(portcb->port);
                        status = soc_sbx_node_port_get(unit, module, port,
                                                       &fabunit, &fabnode, &fabport);
                        if(BCM_SUCCESS(status)) {
                            bsmac.bpid = SOC_SBX_PORT_SID(unit, fabnode, fabport);
                        }
                    }
                    if(BCM_FAILURE(status)) {
                        MIM_ERR(("unit %d: [%s] error %s Mapping FE to QE port information",
                                 unit, FUNCTION_NAME(), bcm_errmsg(status)));
                    } else {
                        bsmac.btid = portcb->hwinfo.ftidx;
                        status = soc_sbx_g2p3_bmac_set(unit,
                                                       portcb->dmac,
                                                       portcb->vlan, &bsmac);

                        if(BCM_FAILURE(status)) {
                            MIM_ERR(("unit %d: [%s] error %s Setting BSMAC"MAC_FMT"\n",
                                     unit, FUNCTION_NAME(), bcm_errmsg(status),MAC_PFMT(portcb->dmac)));
                        } else {
                            MIM_VERB(("unit %d: [%s] Added %s Setting BSMAC"MAC_FMT"\n",
                                      unit, FUNCTION_NAME(), bcm_errmsg(status),MAC_PFMT(portcb->dmac)));
                        }
                   }
                   /* Inherit Parnet Default Back Bone Properties if applicable */
                    if(defbbportcb->hwinfo.egrremarkidx > 0) {

                        status = _bcm_fe2000_mim_set_egress_remarking(unit,
                                                                      portcb->hwinfo.eteencap,
                                                                      defbbportcb->hwinfo.egrremarkidx,
                                                                      defbbportcb->hwinfo.egrremarkflags);
                        if(BCM_SUCCESS(status)) {
                            MIM_VERB((_MD("Inheritting Egress remarking=0x%x from default backbone\n"),
                                      defbbportcb->hwinfo.egrremarkidx));
                            portcb->hwinfo.egrremarkidx = defbbportcb->hwinfo.egrremarkidx;
                            portcb->hwinfo.egrremarkflags = defbbportcb->hwinfo.egrremarkflags;
                        }  else {
                            MIM_ERR(("Error[%d] %s Inheritting Egress remarking !!!!(%s,%d)\n",\
                                     status, FUNCTION_NAME(),__FILE__,__LINE__));
                        }
                    }
               }
            }

            /* Free back resources if allocation failed */
            if(BCM_FAILURE(status)) {
                _bcm_fe2000_mim_g2p3_free_inseg(unit,
                                                portcb);

                _bcm_fe2000_mim_g2p3_free_outseg(unit,
                                                 portcb);
            }
        }/* my mod id get */
    }

    return status;
}

static int _bcm_fe2000_mim_port_free(int unit,
                                     bcm_fe2k_mim_port_control_t **ppPortCtl)
{
    int status = BCM_E_NONE;
    int mymodid, tnumports=1, index, idx=0, pindex=0, port=0;
    soc_sbx_g2p3_pv2e_t pv2e;
    bcm_fe2k_mim_port_control_t *portcb;
    bcm_port_t tp[BCM_TRUNK_MAX_PORTCNT];
    uint32 portid;

    if(!ppPortCtl || !*ppPortCtl) {
        return BCM_E_PARAM;
    } 
    portcb = (*ppPortCtl);
    portid = BCM_GPORT_MIM_PORT_ID_GET(portcb->gport);

    if ((portcb->type == _BCM_FE2K_MIM_DEF_BACKBONE_PORT) ||
        (portcb->type == _BCM_FE2K_MIM_ACCESS_PORT)) {

        status = bcm_sbx_stk_modid_get(unit, &mymodid);
            if(BCM_FAILURE(status)) {
            MIM_ERR(("ERROR %s Could not obtain my module ID !!!!(%s,%d)\n",\
                     FUNCTION_NAME(),__FILE__,__LINE__));
            return BCM_E_INTERNAL;
        }

        if(BCM_GPORT_IS_TRUNK(portcb->port)) {
            bcm_trunk_add_info_t tdata;

            /* Compare mymodid with trunk module array, if any match allocate
             * egress resource else skip */
            status = bcm_trunk_get(unit, 
                                   BCM_GPORT_TRUNK_GET(portcb->port),
                                   &tdata);
            if(BCM_FAILURE(status)) {
                MIM_ERR((_MD("Trunk Get Failure: %d, gport=0x%08x\n"),
                         status, portcb->port));
                return BCM_E_INTERNAL;
            }   
         
            tnumports = tdata.num_ports;

            for(index=0; index < BCM_TRUNK_MAX_PORTCNT; index++) {
                tp[index] = -1;
            }
            
            for(index=0; index < tnumports; index++) {

                if(tdata.tm[index] == mymodid) {

                    port = tdata.tp[index];
                    idx = 0;
                    /* verify if this port was already taken care due to 
                     * duplicate trunk distribution */
                    while(tp[idx] >= 0) {
                        if(port == tp[idx]) {
                            break;
                        }
                        idx++;
                    }

                    if(port == tp[idx]) {
                        continue;
                    }

                    tp[pindex++] = port;
                } else {
                    continue;
                }
            }

            tnumports = pindex;

        } else {
            tnumports = 0;

            if(mymodid == BCM_GPORT_MODPORT_MODID_GET(portcb->port)) {
                tnumports = 1;
                tp[0] = BCM_GPORT_MODPORT_PORT_GET(portcb->port);
            }
        }

        for(index=0; index < tnumports; index++) {
            if(portcb->hwinfo.lport) {
                /* since BVLAN uses TB, clear LP on pv2e */
                soc_sbx_g2p3_pv2e_t_init(&pv2e);
                pv2e.vlan = portcb->vlan;
                status = soc_sbx_g2p3_pv2e_set(unit, portcb->vlan, tp[index], &pv2e);

                MIM_VERB((_MD("Clearing pv2e[%d,0x%03x]\n"),
                          BCM_GPORT_MODPORT_PORT_GET(portcb->port), portcb->vlan));

                if(BCM_FAILURE(status)) {
                    MIM_ERR(("ERROR %s Cound not Clear PV2E Port[%d] VID[0x%x] Error[%s] (%s,%d)\n",
                             FUNCTION_NAME(), tp[index],
                             portcb->vlan, bcm_errmsg(status),
                             __FILE__,__LINE__));
                }
            }
        }               
    }

    _bcm_fe2000_mim_g2p3_free_inseg(unit,
                                    portcb);

    _bcm_fe2000_mim_g2p3_free_outseg(unit,
                                     portcb);

    if(_BCM_FE2K_MIM_BACKBONE_PORT == portcb->type) {
        /* delete bsmac */
        status = soc_sbx_g2p3_bmac_remove(unit, portcb->dmac, portcb->vlan);
        if(BCM_FAILURE(status)) {
            MIM_ERR(("unit %d: [%s] error %s Setting BSMAC"MAC_FMT"\n",
                     unit, FUNCTION_NAME(), bcm_errmsg(status),
                     MAC_PFMT(portcb->dmac)));
        }
    }

    /* invalidate Gport to Lport mapping */
    SBX_LPORT_DATAPTR(unit, portcb->hwinfo.lport) = NULL;
    SBX_LPORT_TYPE(unit, portcb->hwinfo.lport) = BCM_GPORT_INVALID;
    gportinfo[unit].lpid[portid] = 0;

    sal_free(portcb);
    portcb = NULL;

    return status;
}

static
int _bcm_fe2000_dump_portcb(bcm_fe2k_mim_port_control_t *portcb)
{
    if(portcb) {
        MIM_VERB((" Mim Gport Dump ##: \n"));
        MIM_VERB(("GPORT ID : [0x%x] Mod Port ID[0x%x] \n", portcb->gport, portcb->port));
        MIM_VERB(("Vlan: [0x%x] ISID-Vlan:[0x%x] ISID[0x%x]",
                  portcb->vlan, portcb->isidvlan, portcb->isid));

        MIM_VERB(("FTIDX:[0x%x] OI:[0x%x] ETE-Encap[0x%x] ETE-L2[0x%x] \n",
                  portcb->hwinfo.ftidx, portcb->hwinfo.ohi,
                  portcb->hwinfo.eteencap, portcb->hwinfo.etel2));
        MIM_VERB(("LPORT[0x%x] ISMAC:[0x%x] ESMAC:[0x%x] \n",
                  portcb->hwinfo.lport,portcb->hwinfo.ismacidx, portcb->hwinfo.esmacidx));
        MIM_VERB(("Type[%d], Service[%d], RefCount[%d], Criteria[0x%x] \n",
                  portcb->type, portcb->service, portcb->refcount, portcb->criteria));
        MIM_VERB(("DMac[0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x \n",
                  portcb->dmac[0], portcb->dmac[1], portcb->dmac[2],
                  portcb->dmac[3], portcb->dmac[4], portcb->dmac[5]));
        MIM_VERB(("SMac[0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x \n",
                  portcb->smac[0], portcb->smac[1], portcb->smac[2],
                  portcb->smac[3], portcb->smac[4], portcb->smac[5]));
    }
    return BCM_E_NONE;
}

static
int _bcm_fe2000_dump_vpncb(bcm_fe2k_mim_vpn_control_t *vpncb)
{
    if(vpncb) {
        MIM_VERB(("VPN Dump ##\n"));
        MIM_VERB(("VPNID[0x%x] ISID[0x%x]\n", vpncb->vpnid, vpncb->isid));
        MIM_VERB(("Bcast Group[0x%x] Unk.Ucast[0x%x] Unk.Mcast[0x%x]\n",
                  vpncb->broadcast_group, vpncb->unknown_unicast_group,
                  vpncb->unknown_multicast_group));
    }
    return BCM_E_NONE;
}

static
int _bcm_fe2000_mim_g2p3_setup_learning(int unit, int disable)
{
    soc_sbx_g2p3_xt_t xt;
    uint32 xtidx=0;
    int rv=0;

    BCM_IF_ERROR_RETURN(soc_sbx_g2p3_exc_bmac_learn_idx_get(unit, &xtidx));

    rv = soc_sbx_g2p3_xt_get(unit, xtidx, &xt);
    if (BCM_FAILURE(rv)) {
        MIM_ERR(("Failed to get BMAC learn exception entry "
                "unit=%d rv=%d(%s)\n",
                unit, rv, bcm_errmsg(rv)));
        return rv;
    }

    xt.forward = (disable)?0:1;
    rv = soc_sbx_g2p3_xt_set(unit, xtidx, &xt);
    if (BCM_FAILURE(rv)) {
        MIM_ERR(("Failed to set BMAC learn exception entry "
                "unit=%d rv=%d(%s)\n",
                unit, rv, bcm_errmsg(rv)));
        return rv;
    }
    return BCM_E_NONE;
}


int _bcm_fe2000_mim_port_delete(int                          unit,
                                bcm_fe2k_mim_vpn_control_t  *vpncb,
                                bcm_fe2k_mim_port_control_t *portcb)
{
    int status = BCM_E_NONE;

    if(portcb && vpncb) {

      if(_BCM_FE2K_MIM_DEF_BACKBONE_PORT == portcb->type) {
          /* verify if all back bone ports over this default bbone port were deleted */
          if(portcb->refcount) {
              MIM_ERR(("ERROR %s Default Back bone port has non-zero Backbone ports associated !!!!(%s,%d)\n",\
                       FUNCTION_NAME(),__FILE__,__LINE__));
              status = BCM_E_PARAM;
          } else {
              DQ_REMOVE(&portcb->listnode);

              if(BCM_GPORT_IS_TRUNK(portcb->port)) {
                  DQ_REMOVE(&portcb->trunklistnode);
              }
              
              status = _bcm_fe2000_mim_port_free(unit, &portcb);
              if(BCM_FAILURE(status)) {
                  MIM_ERR(("ERROR %s Count not Free back Default Backbone Port[0x%x] !!!!(%s,%d)\n",\
                           FUNCTION_NAME(),portcb->gport,__FILE__,__LINE__));
              }
          }
      } else {
          /* if back bone de-reference it with default back bone port */
          if(_BCM_FE2K_MIM_BACKBONE_PORT == portcb->type) {
              dq_p_t port_elem;
              bcm_fe2k_mim_port_control_t *defbbportcb = NULL;

              /* find the port on default BB List*/
              DQ_TRAVERSE(&vpncb->def_bbone_plist, port_elem) {

                  _BCM_FE2000_GET_PORTCB_FROM_LIST(port_elem, defbbportcb);

                  /* if this back bone port is associated to this default back bone port deref */
                  if(defbbportcb->port ==  portcb->port){
                      defbbportcb->refcount--;
                      break;
                  }
              } DQ_TRAVERSE_END(&vpncb->def_bbone_plist, port_elem);

          } else if(_BCM_FE2K_MIM_ACCESS_PORT == portcb->type) {
              /* if port mode is access and Port Based interface, set pv2e to default */
              if(_BCM_FE2K_MIM_PORT == portcb->service) {
                  _bcm_fe2000_mim_reset_port_interface(unit, portcb);
              }
          }

          /* remove this port from port list of vpn */
          DQ_REMOVE(&portcb->listnode);

          status = _bcm_fe2000_mim_port_free(unit, &portcb);
      }
    } else {
        status = BCM_E_PARAM;
    }

    return status;
}

/*-------- Static Functions End ----------*/
/* BCM API - Dispatcher Functions */

/*
 * Function:
 *   bcm_fe2000_mim_init
 * Purpose:
 *     Initializes Provider Back Bone Bridging Software Database
 *
 * Parameters:
 *     unit  - (IN)     fe unit to initialize
 * Returns:
 *   BCM_E_XX
 */
int bcm_fe2000_mim_init(int unit)
{
    uint32 min, max;
    int result, i, j;

    G2P3_ONLY_SUPPORT_CHECK(unit);

    if(_MIM_UNIT_INVALID(unit)){
        return BCM_E_PARAM;
    }

    if ((mim_mlock[unit] = sal_mutex_create("bcm_mim_lock")) == NULL) {
        return BCM_E_MEMORY;
    } else {
        /* unit re-initialized without detach flag error */
        /* return BCM_E_EXISTS; */
    }

    result = _bcm_fe2000_mim_g2p3_setup_learning(unit, 0);
    if(BCM_SUCCESS(result)) {

        /* Determine Maximum VPN that can be allocated */
        result = _sbx_gu2_alloc_range_get(unit, SBX_GU2K_USR_RES_VSI,
                                      &min, &max);
        if(BCM_FAILURE(result)) {
            /* free back allocated mutex */
            sal_mutex_destroy(mim_mlock[unit]);
            mim_mlock[unit] = NULL;

            MIM_ERR(("ERROR %s Could not allocate VPN VSI (%s,%d)\n",
                      FUNCTION_NAME(),__FILE__,__LINE__));
            return BCM_E_INTERNAL;
        } else {
            mim_vpn_db[unit] = NULL;

            /* Create a VPN database on this unit */
            shr_avl_create(&mim_vpn_db[unit], INT_TO_PTR(unit),
                           sizeof(bcm_fe2k_mim_vpn_control_t),(max-min));

            if(NULL == mim_vpn_db[unit]){
                /* free back allocated mutex */
                sal_mutex_destroy(mim_mlock[unit]);
                mim_mlock[unit] = NULL;

                MIM_ERR(("ERROR %s Could not allocate memory(%s,%d)\n",
                          FUNCTION_NAME(),__FILE__,__LINE__));

                return BCM_E_MEMORY;
            } else {
                sal_memset(&gportinfo[unit], 0, sizeof(_bcm_fe2k_mimgp_info_t));
                mim_vpn_db[unit]->datum_copy_fn = _bcm_fe2000_mim_vpn_copy_datum;

                /* clear trunk association structure */
                for(i=0; i < BCM_MAX_NUM_UNITS; i++) {
                    for(j=0; j < SBX_MAX_TRUNKS; j++) {
                        mim_trunk_assoc_info[i][j].mimType = _BCM_FE2K_MIM_PORT_MAX; /*invalid*/
                        /* initialize the port list */
                        DQ_INIT(&mim_trunk_assoc_info[i][j].plist);
                    }
                }

                result = bcm_fe2000_trunk_change_register(unit, _bcm_fe2000_mim_trunk_cb, NULL);
                if(BCM_FAILURE(result)) {

                    MIM_ERR(("ERROR %s Could not register for trunk change Error:%d  (%s,%d)\n",
                             FUNCTION_NAME(),result,__FILE__,__LINE__));

                    /* free vpndb */
                    shr_avl_destroy(mim_vpn_db[unit]);
                    mim_vpn_db[unit] = NULL;

                    /* free back allocated mutex */
                    sal_mutex_destroy(mim_mlock[unit]);
                    mim_mlock[unit] = NULL;
                }
            }
        }

        return BCM_E_NONE;
    } else {
        return BCM_E_INTERNAL;
    }
}

/*
 * Function:
 *   bcm_fe2000_mim_detach
 * Purpose:
 *     UnInitializes Provider Back Bone Bridging Software Database
 *
 * Parameters:
 *     unit  - (IN)     fe unit to initialize
 * Returns:
 *   BCM_E_XX
 */
int bcm_fe2000_mim_detach(int unit)
{
    /* sanity checks */
    G2P3_ONLY_SUPPORT_CHECK(unit);

    if(_MIM_UNIT_INVALID(unit)){
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(_MIM_LOCK(unit));

    
    /* free the vpn database */
    if(mim_vpn_db[unit]){
        
        
        shr_avl_destroy(mim_vpn_db[unit]);
        mim_vpn_db[unit] = NULL;
        _bcm_fe2000_mim_g2p3_setup_learning(unit, 1);
    }
    /* free back allocated mutex */
    sal_mutex_destroy(mim_mlock[unit]);

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *   bcm_fe2000_mim_vpn_create
 * Purpose:
 *     Creates a  Provider Back Bone Bridging Software Database
 *
 * Parameters:
 *   unit  - (IN)     fe unit to initialize
 *   info  - (IN)     vpn configuration
 * Returns:
 *   BCM_E_XX
 */
int bcm_fe2000_mim_vpn_create(int                   unit,
                              bcm_mim_vpn_config_t *info)
{

    uint32                      alloc_vsi = ~0;
    int                         res_flags;
    int                         status=BCM_E_NONE;
    int                         result;
    bcm_fe2k_mim_vpn_control_t  vpninfo, *vpncb = NULL;
    soc_sbx_g2p3_v2e_t          vpnv2e;

    /* sanity checks */
    G2P3_ONLY_SUPPORT_CHECK(unit);

    if(_MIM_UNIT_INVALID(unit)){
        return BCM_E_PARAM;
    }

    if(!info) {
        return BCM_E_PARAM;
    }

    /* Verify the flags */
    if(info->flags & ~_BCM_FE2000_MIM_SUPPORTED_FLAGS) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(_MIM_LOCK(unit));
    _MIM_VPN_INIT_CHECK(unit);

    /* if BCM_MIM_VPN_WITH_ID, verify if the vpn id is free */
    if (info->flags & BCM_MIM_VPN_WITH_ID) {
        alloc_vsi = info->vpn;
        res_flags = _SBX_GU2_RES_FLAGS_RESERVE;
    } else {
        res_flags = 0;
    }

    /* if BCM_MIM_VPN_REPLACE, obtain necessary information of
     * vpn & update */
    status = _sbx_gu2_resource_alloc(unit,
                                     SBX_GU2K_USR_RES_VSI,
                                     1,
                                     &alloc_vsi,
                                     res_flags);
    if(res_flags & BCM_MIM_PORT_WITH_ID) {
        if(status == BCM_E_RESOURCE) {
            MIM_VERB(("DEBUG Reserved VPNID (0x%x) (%s,%d)\n",\
                      alloc_vsi,__FILE__,__LINE__));
            status = BCM_E_NONE;
        } else {
            status = BCM_E_FULL;
        }
    }

    if(BCM_FAILURE(status)) {
        MIM_ERR(("ERROR %s Could not allocate VSI for VPN (%s,%d)\n",\
                 FUNCTION_NAME(),__FILE__,__LINE__));
        alloc_vsi = _MIM_INVALID_VPN_ID;
    } else {
        uint32 lport=0;

        /* allocate Logical port for the VPN used for instrumentation + counting */
        status = _sbx_gu2_resource_alloc(unit,
                                         SBX_GU2K_USR_RES_LPORT,
                                         1,
                                         &lport,
                                         0);
        if(BCM_SUCCESS(status)) {
            /* allocate vpn db element */
            sal_memset(&vpninfo, 0, sizeof(bcm_fe2k_mim_vpn_control_t));
            vpninfo.vpnid = alloc_vsi;
            vpninfo.isid  = info->lookup_id;

            /* Verify if the ISID & VPNID is not part of any other VPN */
            result = shr_avl_lookup(mim_vpn_db[unit],
                                    _bcm_fe2000_mim_vpn_lookup_compare_nodata,
                                    (shr_avl_datum_t*)&vpninfo);
            /* validate is the vpn or isid exists on the tree */
            if(result){
                status = BCM_E_EXISTS;
            } else {
                soc_sbx_g2p3_isid2e_t isid2e;
                soc_sbx_g2p3_ft_t fte;
                uint32 ftidx=0, unknown_mc_ft_offset=0;

                vpninfo.lport = lport;
                
                vpninfo.broadcast_group         = info->broadcast_group;
                vpninfo.unknown_unicast_group   = info->unknown_unicast_group;
                vpninfo.unknown_multicast_group = info->unknown_multicast_group;
                vpninfo.policer_id              = -1;
                /* info -> match_service_tpid -> not used */

                /* Make sure Learning is Enabled on the VPN VSI */
                soc_sbx_g2p3_v2e_t_init(&vpnv2e);
                result = soc_sbx_g2p3_v2e_set(unit, alloc_vsi, &vpnv2e);

                if(BCM_SUCCESS(result)) {
                    /* provision the Unknown Unicast and unknown Multicast FTE */
                    soc_sbx_g2p3_ft_t_init(&fte);
                    result = soc_sbx_g2p3_vlan_ft_base_get(unit, &ftidx);
                    if(BCM_SUCCESS(result)){
                        fte.oi  = info->broadcast_group;
                        fte.mc  = 1;
                        fte.qid = SBX_MC_QID_BASE;
                        ftidx += vpninfo.vpnid;
                        result = soc_sbx_g2p3_ft_set(unit, ftidx, &fte);
                        if(BCM_SUCCESS(result)){
                            result =  soc_sbx_g2p3_mc_ft_offset_get(unit, &unknown_mc_ft_offset);
                            if(BCM_SUCCESS(result)) {
                                fte.oi  = info->unknown_multicast_group;
                                fte.mc  = 1;
                                fte.qid = SBX_MC_QID_BASE;
                                result = soc_sbx_g2p3_ft_set(unit,
                                                             ftidx + unknown_mc_ft_offset,
                                                             &fte);
                                if(BCM_SUCCESS(result)) {
                                    soc_sbx_g2p3_lp_t lp;
                                    soc_sbx_g2p3_lp_t_init(&lp);
                                    /* In future Set Policer + Stats on VPN service LP */
                                    /* PID is dont care */
                                    result = soc_sbx_g2p3_lp_set(unit,
                                                                 lport,
                                                                 &lp);
                                    if(BCM_FAILURE(result)) {
                                        MIM_ERR(("unit %d: [%s] error %s in programming LP 0x%x\n",
                                                 unit, FUNCTION_NAME(), bcm_errmsg(status),lport));
                                    } else {

                                        /* Add a ISID2E entry to resolve to the above VSI */
                                        soc_sbx_g2p3_isid2e_t_init(&isid2e);
                                        isid2e.lpi  = lport;
                                        isid2e.vlan = vpninfo.vpnid;
                                        result = soc_sbx_g2p3_isid2e_set(unit, vpninfo.isid, &isid2e);
                                    }
                                } else {
                                    MIM_ERR(("ERROR %s Setting Unknown Multcast Flood Group  (%s,%d)\n",\
                                             FUNCTION_NAME(),__FILE__,__LINE__));
                                }
                            } else {
                                MIM_ERR(("ERROR %s Getting Unknown Multcast Offset (%s,%d)\n",\
                                         FUNCTION_NAME(),__FILE__,__LINE__));
                            }
                        } else {
                            MIM_ERR(("ERROR %s Setting Unknown Unicast Flood Group  (%s,%d)\n",\
                                     FUNCTION_NAME(),__FILE__,__LINE__));
                        }
                    } else {
                        MIM_ERR(("ERROR %s Getting Unknown Unicast Offset (%s,%d)\n",\
                                 FUNCTION_NAME(),__FILE__,__LINE__));
                    }
                } else {
                    MIM_ERR(("ERROR %s Setting VPN VSI V2E (%s,%d)\n",\
                             FUNCTION_NAME(),__FILE__,__LINE__));
                }

                if(BCM_FAILURE(result)) {

                    MIM_ERR(("ERROR %s Setting Forwarding Path for VPN (%s,%d)\n",\
                             FUNCTION_NAME(),__FILE__,__LINE__));
                    status = BCM_E_INTERNAL;
                } else {

                    /* If not error, insert ot the vpn db tree */
                    result = shr_avl_insert(mim_vpn_db[unit],
                                            _bcm_fe2000_mim_insert_compare,
                                            (shr_avl_datum_t*)&vpninfo);
                    if(BCM_FAILURE(result)) {
                        MIM_ERR(("ERROR %s Could not insert into VPN database tree (%s,%d)\n",\
                                 FUNCTION_NAME(),__FILE__,__LINE__));
                        status = BCM_E_INTERNAL;
                    } else {
                        /* since avl allocates memory for vpncb,
                           the tree has to be looked again to get real vpncb*/
                        _bcm_fe2k_mim_lookup_data_t hdl;

                        /* obtain the VPN from database */
                        hdl.key = &vpninfo;
                        hdl.datum = &vpncb;

                        if(!_AVL_EMPTY(mim_vpn_db[unit])) {
                            result=shr_avl_lookup_lkupdata(mim_vpn_db[unit],
                                                           _bcm_fe2000_mim_vpn_lookup_compare,
                                                           (shr_avl_datum_t*)&vpninfo,       
                                                           (void*)&hdl);
                            /* If vpn was found on the database */
                            if(result > 0){
                                /* initialize the port list */
                                DQ_INIT(&vpncb->def_bbone_plist);
                                DQ_INIT(&vpncb->vpn_access_sap_head);
                                DQ_INIT(&vpncb->vpn_bbone_sap_head);
                            } else {
                                MIM_ERR(("ERROR %s Could not find vpncb on VPN database tree (%s,%d)\n",\
                                         FUNCTION_NAME(),__FILE__,__LINE__));
                                status = BCM_E_INTERNAL;
                            }
                        }
                    }
                }
            }
        } else {

            MIM_ERR(("ERROR %s Could not allocate LPORT for VPN (%s,%d)\n",\
                     FUNCTION_NAME(),__FILE__,__LINE__));
        }
    }

    if(BCM_SUCCESS(status)) {
        /* set vsi type */
        status = _sbx_gu2_set_vsi_type(unit, alloc_vsi, BCM_GPORT_MIM_PORT);
        if(BCM_FAILURE(status)) {
            MIM_ERR(("ERROR %s Could not Set VSI type (%s,%d)\n",\
                     FUNCTION_NAME(),__FILE__,__LINE__));            
            status = BCM_E_INTERNAL;
        }
    }

    if(BCM_FAILURE(status)) {

        if(vpncb){
            /* free back logical port */
            result = _sbx_gu2_resource_free(unit,
                                            SBX_GU2K_USR_RES_LPORT,
                                            1,
                                            &vpncb->lport,
                                            0);
            if(BCM_FAILURE(result)) {
                /* cant do much just log this error */
                MIM_WARN(("WARNING %s Could not Free back LPORT(%s,%d)\n",\
                          FUNCTION_NAME(),__FILE__,__LINE__));
            }

            /* Delete ISID2E */
            result = soc_sbx_g2p3_isid2e_delete(unit,
                                                vpncb->isid);
            if(BCM_FAILURE(result)) {
                /* cant do much just log this error */
                MIM_WARN(("WARNING %s Could not Delete ISID2E[0x%x] (%s,%d)\n",\
                          FUNCTION_NAME(), vpncb->isid, __FILE__,__LINE__));
            }
        }

        /* Free back VSI */
        result = _sbx_gu2_resource_free(unit,
                                        SBX_GU2K_USR_RES_VSI,
                                        1,
                                        &alloc_vsi,
                                        res_flags);
        if(BCM_FAILURE(result)) {

            /* cant do much just log this error */
             MIM_WARN(("WARNING %s Could not Free back VSI(%s,%d)\n",\
                                FUNCTION_NAME(),__FILE__,__LINE__));
        }

    } else {
        info->vpn = alloc_vsi;

    }

    if(BCM_SUCCESS(status)) {
        _bcm_fe2000_dump_vpncb(vpncb);
    }

    _MIM_UNLOCK(unit);
    return status;
}

int bcm_fe2000_mim_vpn_destroy(int           unit,
                               bcm_mim_vpn_t  info)
{
    uint32                      alloc_vsi = ~0;
    int                         status=BCM_E_NONE;
    int                         result;
    bcm_fe2k_mim_vpn_control_t  dummycb, *vpncb;
    _bcm_fe2k_mim_lookup_data_t hdl;

    /* sanity checks */
    G2P3_ONLY_SUPPORT_CHECK(unit);

    if(_MIM_UNIT_INVALID(unit)){
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(_MIM_LOCK(unit));
    _MIM_VPN_INIT_CHECK(unit);

    /* verify if the vpn exisits on the database */
    alloc_vsi     = info;
    dummycb.vpnid = info;
    dummycb.isid  = _MIM_INVALID_ISID; /* throw in invalid isid to do vpnid only match */
    hdl.key = &dummycb;
    hdl.datum = &vpncb;

    if(!_AVL_EMPTY(mim_vpn_db[unit])) {
        /* verify is the VPN ID prevails on the vpn database */
        result=shr_avl_lookup_lkupdata(mim_vpn_db[unit],
                                       _bcm_fe2000_mim_vpn_lookup_compare,
                                       (shr_avl_datum_t*)&dummycb, 
                                       (void*)&hdl);
        if(result > 0)/*found*/{
            MIM_VERB(("DEBUG: %s VPN ID [%d] Found on database !!!!(%s,%d)\n",
                      FUNCTION_NAME(),info,__FILE__,__LINE__));

            
            /* Verify if all port controls or SAP are empty before the vpn can be destroyed */
            if((DQ_EMPTY(&vpncb->vpn_access_sap_head)) &&
               (DQ_EMPTY(&vpncb->def_bbone_plist)) && 
               (DQ_EMPTY(&vpncb->vpn_bbone_sap_head))) {
                /* free back the VPN VSI */
                uint32 alloc_vsi = vpncb->vpnid;

                result = _sbx_gu2_resource_free(unit,
                                            SBX_GU2K_USR_RES_VSI,
                                            1,
                                            &alloc_vsi,
                                            0);
                if(BCM_FAILURE(result)) {
                    /* cant do much just log this error */
                    MIM_WARN(("WARNING %s Could not Free back VSI[0x%x] (%s,%d)\n",\
                              FUNCTION_NAME(), alloc_vsi, __FILE__,__LINE__));
                }

                /* free back the VPN LPORT */
                result = _sbx_gu2_resource_free(unit,
                                            SBX_GU2K_USR_RES_LPORT,
                                            1,
                                            &vpncb->lport,
                                            0);
                if(BCM_FAILURE(result)) {
                    /* cant do much just log this error */
                    MIM_WARN(("WARNING %s Could not Free back LPORT[0x%x] (%s,%d)\n",\
                              FUNCTION_NAME(), vpncb->lport, __FILE__,__LINE__));
                }

                /* Delete ISID2E */
                result = soc_sbx_g2p3_isid2e_delete(unit,
                                                    vpncb->isid);
                if(BCM_FAILURE(result)) {
                    /* cant do much just log this error */
                    MIM_WARN(("WARNING %s Could not Delete ISID2E[0x%x] (%s,%d)\n",\
                              FUNCTION_NAME(), vpncb->isid, __FILE__,__LINE__));
                }

                /* remove and destroy vpn tree node */
                result = shr_avl_delete(mim_vpn_db[unit],
                                        _bcm_fe2000_mim_insert_compare,
                                        (shr_avl_datum_t*)&dummycb);
                if(BCM_FAILURE(result)) {
                    MIM_ERR(("ERROR %s Could not delete from VPN database tree (%s,%d)\n",\
                             FUNCTION_NAME(),__FILE__,__LINE__));
                    status = BCM_E_INTERNAL;
                }

                if(BCM_SUCCESS(status)) {
                    /* set vsi type */
                    status = _sbx_gu2_set_vsi_type(unit, alloc_vsi, BCM_GPORT_TYPE_NONE);
                    if(BCM_FAILURE(status)) {
                        MIM_ERR(("ERROR %s Could not Set VSI type (%s,%d)\n",\
                                 FUNCTION_NAME(),__FILE__,__LINE__));            
                        status = BCM_E_INTERNAL;
                    }
                }

            } else {
                dq_p_t port_elem;
                bcm_fe2k_mim_port_control_t *portcb = NULL;

                if (!(DQ_EMPTY(&vpncb->vpn_access_sap_head))) {
                    MIM_VERB((_MD("ACCESS SAP:\n")));
                    DQ_TRAVERSE(&vpncb->vpn_access_sap_head, port_elem) {
                        _BCM_FE2000_GET_PORTCB_FROM_LIST(port_elem, portcb);
                        MIM_VERB(("  gport=0x%08x  isid=0x%03x\n  SMAC=" MAC_FMT
                                  " DMAC=" MAC_FMT "\n", portcb->gport, portcb->isid,
                                  MAC_PFMT(portcb->smac), MAC_PFMT(portcb->dmac)));
                    } DQ_TRAVERSE_END (&vpncb->vpn_access_sap_head, port_elem);
                }

                if (!(DQ_EMPTY(&vpncb->vpn_bbone_sap_head))) {
                    MIM_VERB((_MD("BACKBONE SAP:\n")));
                    DQ_TRAVERSE(&vpncb->vpn_bbone_sap_head, port_elem) {
                        _BCM_FE2000_GET_PORTCB_FROM_LIST(port_elem, portcb);
                        MIM_VERB(("  gport=0x%08x  isid=0x%03x\n  SMAC=" MAC_FMT
                                  " DMAC=" MAC_FMT "\n", portcb->gport, portcb->isid,
                                  MAC_PFMT(portcb->smac), MAC_PFMT(portcb->dmac)));
                    } DQ_TRAVERSE_END (&vpncb->vpn_bbone_sap_head, port_elem);
                }

                if (!(DQ_EMPTY(&vpncb->def_bbone_plist))) {
                    MIM_VERB((_MD("Backbone:\n")));
                    DQ_TRAVERSE(&vpncb->def_bbone_plist, port_elem) {
                        _BCM_FE2000_GET_PORTCB_FROM_LIST(port_elem, portcb);
                        MIM_VERB(("  gport=0x%08x  isid=0x%03x\n  SMAC=" MAC_FMT
                                  " DMAC=" MAC_FMT "\n", portcb->gport, portcb->isid,
                                  MAC_PFMT(portcb->smac), MAC_PFMT(portcb->dmac)));
                    } DQ_TRAVERSE_END(&vpncb->def_bbone_plist, port_elem);
                }

                MIM_ERR(("ERROR %s VPN Service Access Points are not empty (%s,%d)\n",\
                         FUNCTION_NAME(),__FILE__,__LINE__));
                status = BCM_E_PARAM;

            }
        } else {
            /* If vpn was found on the database */
            MIM_ERR(("ERROR %s Could not find VPN in VPN database tree (%s,%d)\n",\
                     FUNCTION_NAME(),__FILE__,__LINE__));
            status = BCM_E_PARAM;
        }
    } else {
        /* If vpn was found on the database */
        MIM_ERR(("ERROR %s VPN not found !! VPN Tree Empty (%s,%d)\n",\
                 FUNCTION_NAME(),__FILE__,__LINE__));
        status = BCM_E_PARAM;
    }

    _MIM_UNLOCK(unit);
    return status;
}

int bcm_fe2000_mim_vpn_destroy_all(int unit)
{
  return BCM_E_UNAVAIL;
}


/* Assumes locks have been taken */
static int
_bcm_fe2000_mim_vpn_control_get(int                   unit,
                                bcm_mim_vpn_t         vpn,
                                bcm_fe2k_mim_vpn_control_t **vpnCtl)
{
    int result;
    bcm_fe2k_mim_vpn_control_t  dummycb;
    _bcm_fe2k_mim_lookup_data_t hdl;

    /* obtain the VPN from database */
    dummycb.vpnid = vpn;
    dummycb.isid  = _MIM_INVALID_ISID; /* throw in invalid isid to do vpnid only match */
    hdl.key = &dummycb;
    hdl.datum = vpnCtl;

    if(!_AVL_EMPTY(mim_vpn_db[unit])) {
        /* verify is the VPN ID prevails on the vpn database */
        result=shr_avl_lookup_lkupdata(mim_vpn_db[unit],
                                       _bcm_fe2000_mim_vpn_lookup_compare,
                                       (shr_avl_datum_t*)&dummycb,
                                       (void*)&hdl);
        if (result > 0)/*found*/{
            MIM_VERB(("DEBUG: %s VPN ID [%d] Found on database !!!!(%s,%d)\n",
                      FUNCTION_NAME(), vpn ,__FILE__,__LINE__));

        } else {
            /* If vpn was found on the database */
            MIM_ERR(("ERROR %s Could not find VPN in VPN database tree (%s,%d)\n",\
                     FUNCTION_NAME(),__FILE__,__LINE__));
            return BCM_E_PARAM;
        }
    } else {
        /* If vpn was found on the database */
        MIM_ERR(("ERROR %s Could not find VPN - VPN tree empty (%s,%d)\n",\
                 FUNCTION_NAME(),__FILE__,__LINE__));
        return BCM_E_PARAM;
    }

    return BCM_E_NONE;
}

int bcm_fe2000_mim_vpn_get(int                   unit,
                           bcm_mim_vpn_t         vpn,
                           bcm_mim_vpn_config_t *info)
{
    int    rv;
    bcm_fe2k_mim_vpn_control_t *vpncb;

    /* sanity checks */
    G2P3_ONLY_SUPPORT_CHECK(unit);

    if(_MIM_UNIT_INVALID(unit)){
        return BCM_E_PARAM;
    }

    if(!info) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(_MIM_LOCK(unit));
    _MIM_VPN_INIT_CHECK(unit);

    rv  = _bcm_fe2000_mim_vpn_control_get(unit, vpn, &vpncb);

    if (BCM_SUCCESS(rv)) {
        /* fill up the vpn info */
        info->vpn                     = vpncb->vpnid;
        info->lookup_id               = vpncb->isid;
        info->broadcast_group         = vpncb->broadcast_group;
        info->unknown_unicast_group   = vpncb->unknown_unicast_group;
        info->unknown_multicast_group = vpncb->unknown_multicast_group;
    }

    _MIM_UNLOCK(unit);
    return rv;
}

static int
_bcm_fe2000_mim_vpn_bcm_visit(void *userdata,
                              shr_avl_datum_t *datum,
                              void *travdata)
{
    bcm_fe2k_mim_vpn_control_t   *vpnCtl;
    _bcm_fe200_mim_vpn_cb_data_t *cbData;
    bcm_mim_vpn_config_t         vpnConfig;
    int rv, unit;

    cbData = (_bcm_fe200_mim_vpn_cb_data_t*)travdata;
    vpnCtl = (bcm_fe2k_mim_vpn_control_t*)datum;

    unit = cbData->unit;

    _bcm_fe2000_mim_vpn_control_xlate(vpnCtl, &vpnConfig);

    rv = cbData->cb(cbData->unit, &vpnConfig, cbData->userData);
    MIM_VVERB((_MD("User callback returned: %d\n"), rv));

    return rv;
}

int
bcm_fe2000_mim_vpn_traverse(int unit,
                            bcm_mim_vpn_traverse_cb cb,
                            void *user_data)
{
    int rv;
    _bcm_fe200_mim_vpn_cb_data_t  cbData;
    cbData.cb = cb;
    cbData.unit = unit;
    cbData.userData = user_data;

    /* don't allow changes to the vpn tree during traversal */
    BCM_IF_ERROR_RETURN(_MIM_LOCK(unit));
    _MIM_VPN_INIT_CHECK(unit);

    rv = shr_avl_traverse(mim_vpn_db[unit],
                          _bcm_fe2000_mim_vpn_bcm_visit,
                          (void*)&cbData);

    _MIM_UNLOCK(unit);

    return rv;
}


/*
 * Function:
 *   bcm_fe2000_mim_port_add
 * Purpose:
 *     Creates a MiM ports and inserts into the VPN configuration
 *
 * Parameters:
 *     unit  - (IN)     fe unit to initialize
 *     vpn   - (IN)     vpn id
 *     mim_port - (IN)  MiM port to create
 * Returns:
 *   BCM_E_XX
 */
int bcm_fe2000_mim_port_add(int            unit,
                            bcm_mim_vpn_t   vpn,
                            bcm_mim_port_t *mim_port)
{
    int status = BCM_E_NONE, result;
    bcm_fe2k_mim_vpn_control_t dummycb, *vpncb;
    _bcm_fe2k_mim_lookup_data_t hdl;
    uint32 portid = 0;

    /* sanity checks */
    G2P3_ONLY_SUPPORT_CHECK(unit);

    if(_MIM_UNIT_INVALID(unit)){
        MIM_ERR((_MD("Invalid Unit\n")));
        return BCM_E_PARAM;
    }

    if( (_MIM_INVALID_VPN_ID == vpn) || (!mim_port)) {
        MIM_ERR((_MD("Invalid VPN (0x%04x) or mimPort(0x%08x)\n"),
                 vpn, (uint32)mim_port));
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(_MIM_LOCK(unit));
    _MIM_VPN_INIT_CHECK(unit);

    /* verify if the vpn exisits on the database */
    dummycb.vpnid = vpn;
    dummycb.isid  = _MIM_INVALID_ISID; /* throw in invalid isid to do vpnid only match */
    hdl.key = &dummycb;
    hdl.datum = &vpncb;

    if (!_AVL_EMPTY(mim_vpn_db[unit])) {
        /* verify is the VPN ID prevails on the vpn database */
        result=shr_avl_lookup_lkupdata(mim_vpn_db[unit],
                                       _bcm_fe2000_mim_vpn_lookup_compare,
                                       (shr_avl_datum_t*)&dummycb,
                                       (void*)&hdl);
        /* If vpn was found on the database */
        if (result > 0) {
            bcm_fe2k_mim_port_control_t *defbboneportcb = NULL, *olddefbboneportcb = NULL;
            bcm_fe2k_mim_port_control_t *portcb = NULL;

            if (mim_port->flags & BCM_MIM_PORT_REPLACE) {

                /* validate input for mim port update */
                status = _bcm_fe2000_mim_port_update_check(unit, mim_port, vpncb, &portcb,
                                                           &defbboneportcb, &olddefbboneportcb);

                /* update mim port */
                if (BCM_SUCCESS(status)) {
                    status = _bcm_fe2000_mim_update_bbone_port_movement(unit, mim_port, portcb,
                                                                        defbboneportcb, olddefbboneportcb);
                }
            } else {
                /* If so, verify if the Mim Port exists on the vpn control base */
                status = _bcm_fe2000_mim_port_check(unit, mim_port, vpncb, &defbboneportcb);
                if(BCM_SUCCESS(status)) {

                    portcb = sal_alloc(sizeof(bcm_fe2k_mim_port_control_t),"MiM Port control");

                    if (portcb) {
                        sal_memset(portcb, 0, sizeof(bcm_fe2k_mim_port_control_t));
                        portcb->port        = mim_port->port;
                        portcb->isidvlan    = vpncb->vpnid;
                        portcb->isidlport   = vpncb->lport;
                        portcb->isid        = vpncb->isid;
                        portcb->tpid        = mim_port->egress_service_tpid;
                        portcb->criteria    = mim_port->criteria;
                        portcb->flags       = mim_port->flags;
                        portcb->policer_id  = mim_port->policer_id;

                        /* failover id not used until protection is enabled on PBBN */
                        /*portcb->failover_id = mim_port->failover_id;*/
                        if(mim_port->flags & BCM_MIM_PORT_WITH_ID){
                            portid = BCM_GPORT_MIM_PORT_ID_GET(mim_port->mim_port_id);
                            /* reserve the mim port id */
                            portcb->hwinfo.ftidx = _BCM_FE2000_MIM_PORTID_2_FTIDX(unit, portid);
                        }

                        if((mim_port->flags & BCM_MIM_PORT_TYPE_BACKBONE) &&
                           (BCM_MAC_IS_ZERO(mim_port->egress_tunnel_dstmac))) {

                            portcb->type = _BCM_FE2K_MIM_DEF_BACKBONE_PORT;
                            portcb->vlan = mim_port->egress_service_vlan; /* BVLAN */

                            sal_memcpy(&portcb->smac, mim_port->egress_tunnel_srcmac, sizeof(bcm_mac_t));

                            status = _bcm_fe2000_mim_create_back_bone_port(unit,
                                                                           mim_port,
                                                                           portcb,
                                                                           NULL);
                            if(BCM_SUCCESS(status)) {
                                DQ_INSERT_HEAD(&vpncb->def_bbone_plist, &portcb->listnode);

                                /* If Backbone is created over trunk, add the portcb to trunk association list */
                                if(BCM_GPORT_IS_TRUNK(portcb->port)) {
                                    bcm_fe2k_mim_trunk_association_t *trunkAssoc;

                                    trunkAssoc = &mim_trunk_assoc_info[unit][BCM_GPORT_TRUNK_GET(portcb->port)];
                                    DQ_INSERT_HEAD(&trunkAssoc->plist, &portcb->trunklistnode);
                                    trunkAssoc->mimType = _BCM_FE2K_MIM_DEF_BACKBONE_PORT;

                                    MIM_VERB(("DEBUG: BackBone Port (0x%x) Added to Trunk(%d) Assocation (%s,%d)\n",
                                              portcb->gport, BCM_GPORT_TRUNK_GET(portcb->port),__FILE__,__LINE__));
                                }
                            }
                        } else {
                            if(mim_port->flags & BCM_MIM_PORT_TYPE_BACKBONE) {

                                portcb->type = _BCM_FE2K_MIM_BACKBONE_PORT;
                                portcb->vlan = mim_port->egress_service_vlan; /* BVLAN */
                                sal_memcpy(&portcb->smac, mim_port->egress_tunnel_srcmac, sizeof(bcm_mac_t));
                                sal_memcpy(&portcb->dmac, mim_port->egress_tunnel_dstmac, sizeof(bcm_mac_t));

                                status = _bcm_fe2000_mim_create_back_bone_port(unit,
                                                                               mim_port,
                                                                               portcb,
                                                                               defbboneportcb);
                                if(BCM_SUCCESS(status)) {
                                    /* Insert the port cb to vpndb sap list */
                                    DQ_INSERT_HEAD(&vpncb->vpn_bbone_sap_head, &portcb->listnode);
                                }

                            } else if (mim_port->flags & BCM_MIM_PORT_TYPE_ACCESS){
                                portcb->type = _BCM_FE2K_MIM_ACCESS_PORT;
                                portcb->vlan = mim_port->match_vlan;
                                status = _bcm_fe2000_mim_create_access_port(unit,
                                                                            mim_port,
                                                                            portcb);
                                if(BCM_SUCCESS(status)) {
                                    /* Insert the port cb to vpndb sap list */
                                    DQ_INSERT_HEAD(&vpncb->vpn_access_sap_head, &portcb->listnode);
                                }

                            } else {
                                MIM_ERR(("WARNING %s Unknown Port Type !!!!(%s,%d)\n",\
                                            FUNCTION_NAME(),__FILE__,__LINE__));
                                status = BCM_E_PARAM;
                            }


                        }

                        /* if successful, allocate a gport Id */
                        if(BCM_SUCCESS(status)) {

                            if(!(mim_port->flags & BCM_MIM_PORT_WITH_ID)) {
                                portid = _BCM_FE2000_FTIDX_2_MIM_PORTID(unit, portcb->hwinfo.ftidx);
                                /* set the mim port id */
                                BCM_GPORT_MIM_PORT_ID_SET(mim_port->mim_port_id, portid);
                            }

                            /* Setup Gport to Lport mapping */
                            SBX_LPORT_DATAPTR(unit, portcb->hwinfo.lport) = (void*)portcb;
                            SBX_LPORT_TYPE(unit, portcb->hwinfo.lport) = BCM_GPORT_MIM_PORT;
                            gportinfo[unit].lpid[portid] = portcb->hwinfo.lport;

                            /* increase reference count on default back bone port */
                            if((defbboneportcb) && (portcb->type == _BCM_FE2K_MIM_BACKBONE_PORT)) {
                                defbboneportcb->refcount++;
                            }

                            portcb->gport = mim_port->mim_port_id;

                        } else {
                            /* if error encountered on port create free up the portcb */
                            sal_free(portcb);
                            portcb = NULL;
                        }

                    } else {
                        status = BCM_E_MEMORY;
                    } /* portcb */

                    if(BCM_SUCCESS(status)) {
                        _bcm_fe2000_dump_portcb(portcb);
                    }
                }  /* port check */
            }
        } else {
            MIM_VERB(("ERROR: %s VPN ID [%d] Not  Found on database !!!!(%s,%d)\n",
                      FUNCTION_NAME(),vpn,__FILE__,__LINE__));

            status = BCM_E_NOT_FOUND;
        }
    } else {
        MIM_VERB(("ERROR: %s VPN ID [%d] Not  Found - VPN Tree Empty !!!!(%s,%d)\n",
                  FUNCTION_NAME(),vpn,__FILE__,__LINE__));

        status = BCM_E_NOT_FOUND;
    }

    _MIM_UNLOCK(unit);
     return status;
}

/*
 *   Function
 *      bcm_fe2000_mim_port_get_lpid
 *   Purpose
 *      Find a logical port based upon the provided MiM GPORT ID
 *   Parameters
 *      (in) unit      = BCM device number
 *      (in) gport     = MiM GPORT to be found
 *      (out) lpid     = LP ID for the GPORT
 *      (out) pport    = physical port for the GPORT
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_fe2000_mim_port_get_lpid(int unit,
                             bcm_gport_t gport,
                             uint32 *lpid,
                             bcm_port_t *pport)
{
    uint32                 logicalPort;          /* logical port number */
    uint32                 gportId;
    bcm_fe2k_mim_port_control_t *mimPortData = NULL;
    int                    result;

    if ((!lpid) || (!pport)) {
        MIM_ERR(("NULL not acceptable for out argument\n"));
        return BCM_E_PARAM;
    }
    if (!BCM_GPORT_IS_MIM_PORT(gport)) {
        MIM_ERR(("GPORT %08X is not MiM GPORT\n", gport));
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(_MIM_LOCK(unit));
    _MIM_VPN_INIT_CHECK(unit);

    gportId = BCM_GPORT_MIM_PORT_ID_GET(gport);
    logicalPort = gportinfo[unit].lpid[gportId];
    result = BCM_E_NONE;
    mimPortData = (bcm_fe2k_mim_port_control_t*)(SBX_LPORT_DATAPTR(unit,
                                                                   logicalPort));
    if (!mimPortData) {
        MIM_ERR(("gport %08X is not valid on unit %d\n",
                  gport,
                  unit));
        result = BCM_E_NOT_FOUND;
    }
    if (BCM_GPORT_MIM_PORT != SBX_LPORT_TYPE(unit, logicalPort)) {
        MIM_ERR(("unit %d gport %08X disagrees with stored type %02X\n",
                  unit,
                  gport,
                  SBX_LPORT_TYPE(unit, logicalPort)));
        result = BCM_E_CONFIG;
    }
    if (BCM_E_NONE == result) {
        *lpid = logicalPort;
        *pport = mimPortData->port;
    }

    _MIM_UNLOCK(unit);
    return result;
}

int bcm_fe2000_mim_port_delete(int            unit,
                                bcm_mim_vpn_t  vpn,
                                bcm_gport_t    mim_port_id)
{
    int status = BCM_E_NONE, result;
    bcm_fe2k_mim_vpn_control_t dummycb, *vpncb;
    _bcm_fe2k_mim_lookup_data_t hdl;
    bcm_fe2k_mim_port_control_t *portcb = NULL;
    uint32 portid=0;
    uint16 lport;

    /* sanity checks */
    G2P3_ONLY_SUPPORT_CHECK(unit);

    if(_MIM_UNIT_INVALID(unit)){
        return BCM_E_PARAM;
    }

    if((_MIM_INVALID_VPN_ID == vpn) ||
       (!BCM_GPORT_IS_MIM_PORT(mim_port_id))) {
      return BCM_E_PARAM;
    }

    /* verify the gport id */
    G2P3_GPORT_RANGE_CHECK(unit, mim_port_id);

    BCM_IF_ERROR_RETURN(_MIM_LOCK(unit));
    _MIM_VPN_INIT_CHECK(unit);

    portid = BCM_GPORT_MIM_PORT_ID_GET(mim_port_id);
    /* obtain logical port from mim gport */
    lport = gportinfo[unit].lpid[portid];

    if(BCM_GPORT_MIM_PORT != SBX_LPORT_TYPE(unit,lport)) {

        MIM_ERR(("Error %s LP GPORT not a MiM port !!!!(%s,%d)\n",\
                 FUNCTION_NAME(),__FILE__,__LINE__));

        status = BCM_E_PARAM;
    } else {
        portcb = (bcm_fe2k_mim_port_control_t*)SBX_LPORT_DATAPTR(unit, lport);

        /* verify if the vpn exisits on the database */
        dummycb.vpnid = vpn;
        dummycb.isid  = _MIM_INVALID_ISID; /* throw in invalid isid to do vpnid only match */
        hdl.key = &dummycb;
        hdl.datum = &vpncb;

        if(!_AVL_EMPTY(mim_vpn_db[unit])) {
            /* verify is the VPN ID prevails on the vpn database */
            result=shr_avl_lookup_lkupdata(mim_vpn_db[unit],
                                           _bcm_fe2000_mim_vpn_lookup_compare,
                                           (shr_avl_datum_t*)&dummycb,
                                           (void*)&hdl);
            /* If vpn was found on the database */
            if(result > 0) {
                status = _bcm_fe2000_mim_port_delete(unit, vpncb, portcb);
                if(BCM_FAILURE(status)) {
                    MIM_ERR(("ERROR %s Count not Free back MiM Port[0x%x] !!!!(%s,%d)\n",\
                             FUNCTION_NAME(),mim_port_id,__FILE__,__LINE__));
                }
            } else {
                MIM_ERR(("ERROR: %s VPN ID [%d] Not  Found in database !!!!(%s,%d)\n",
                          FUNCTION_NAME(),vpn,__FILE__,__LINE__));

                status = BCM_E_NOT_FOUND;
            }


        } else {
            MIM_ERR(("ERROR: %s VPN ID [%d] Not  Found in database - VPN Tree empty !!!!(%s,%d)\n",
                     FUNCTION_NAME(),vpn,__FILE__,__LINE__));

            status = BCM_E_NOT_FOUND;
        }
    }

    _MIM_UNLOCK(unit);
     return status;
}

int
bcm_fe2000_mim_port_delete_all(int            unit,
                               bcm_mim_vpn_t  vpn)
{
    int rv, lastErr = BCM_E_NONE;
    bcm_fe2k_mim_vpn_control_t *vpnCtl;
    bcm_fe2k_mim_port_control_t *portCtl = NULL;
    bcm_gport_t gport;
    dq_p_t dqE;

    BCM_IF_ERROR_RETURN(_MIM_LOCK(unit));
    _MIM_VPN_INIT_CHECK(unit);

    rv = _bcm_fe2000_mim_vpn_control_get(unit, vpn, &vpnCtl);

    if (BCM_FAILURE(rv)) {
        _MIM_UNLOCK(unit);
        MIM_ERR((_MD("Failed to retrieve VPN Control data 0x%08x info: "
                     "%d %s\n"), vpn, rv, bcm_errmsg(rv)));
        return rv;
    }

    /* Grab the configured access ports */
    DQ_TRAVERSE(&vpnCtl->vpn_access_sap_head, dqE) {

        _BCM_FE2000_GET_PORTCB_FROM_LIST(dqE, portCtl);
        gport = portCtl->gport;

        rv = _bcm_fe2000_mim_port_delete(unit, vpnCtl, portCtl);

        if (BCM_FAILURE(rv)) {
            MIM_WARN((_MD("Failed to delete gport 0x%08x: %d %s\n"),
                      gport, rv, bcm_errmsg(rv)));
            lastErr = rv;
            /* keep trying */
        }
    } DQ_TRAVERSE_END(&vpnCtl->vpn_access_sap_head, dqE);

    /* Grab the configured back bone ports */
    DQ_TRAVERSE(&vpnCtl->vpn_bbone_sap_head, dqE) {

        _BCM_FE2000_GET_PORTCB_FROM_LIST(dqE, portCtl);
        gport = portCtl->gport;

        rv = _bcm_fe2000_mim_port_delete(unit, vpnCtl, portCtl);

        if (BCM_FAILURE(rv)) {
            MIM_WARN((_MD("Failed to delete gport 0x%08x: %d %s\n"),
                      gport, rv, bcm_errmsg(rv)));
            lastErr = rv;
            /* keep trying */
        }
    } DQ_TRAVERSE_END(&vpnCtl->vpn_bbone_sap_head, dqE);

    /* Delete the default backbone ports */
    DQ_TRAVERSE (&vpnCtl->def_bbone_plist, dqE) {

        _BCM_FE2000_GET_PORTCB_FROM_LIST(dqE, portCtl);
        gport = portCtl->gport;
        rv = bcm_fe2000_mim_port_delete(unit, vpn, portCtl->gport);

        if (BCM_FAILURE(rv)) {
            MIM_WARN((_MD("Failed to delete gport 0x%08x: %d %s\n"),
                      gport, rv, bcm_errmsg(rv)));
            lastErr = rv;
            /* keep trying */
        }

    } DQ_TRAVERSE_END(&vpnCtl->def_bbone_plist, dqE);

    _MIM_UNLOCK(unit);
    return lastErr;
}

int bcm_fe2000_mim_port_get(int            unit,
                            bcm_mim_vpn_t  vpn,
                            bcm_mim_port_t *mim_port)
{
    int status = BCM_E_NONE, result;
    bcm_fe2k_mim_vpn_control_t    dummycb, *vpncb;
    _bcm_fe2k_mim_lookup_data_t hdl;
    bcm_fe2k_mim_port_control_t *portcb = NULL;
    dq_p_t port_elem;

    /* GPORT for Back bone ports by Tunnel Encap MAC (B-DMAC) and B-SMAC */
    /* GPORT for Access port obtained by match criteria */
    /* sanity checks */
    G2P3_ONLY_SUPPORT_CHECK(unit);

    if(_MIM_UNIT_INVALID(unit)){
        return BCM_E_PARAM;
    }

    if(mim_port->flags & BCM_MIM_PORT_TYPE_ACCESS) {
        /* Verify if user is requesting for only Supported PBBN interface types */
        /* Supported Service Interfaces:
         * [1] Port Mode
         * [2] 1:1 STAG Mode
         */
        if(mim_port->flags & ~_BCM_FE2000_MIM_ACCESS_SUPPORTED_FLAGS) {
            MIM_ERR(("ERROR: %s Bad Access Port Flag !!!!(%s,%d)\n",
                 FUNCTION_NAME(),__FILE__,__LINE__));

            return BCM_E_PARAM;
        }
        if(mim_port->criteria & ~_BCM_FE2000_MIM_ACCESS_SUPPORTED_CRITERIA) {
            MIM_ERR(("ERROR: %s Bad Access Port Criteria !!!!(%s,%d)\n",
                 FUNCTION_NAME(),__FILE__,__LINE__));
            return BCM_E_PARAM;
        }
    }

    if(mim_port->flags & BCM_MIM_PORT_TYPE_BACKBONE) {

        if(mim_port->flags & ~_BCM_FE2000_MIM_BB_SUPPORTED_FLAGS) {
            MIM_ERR(("ERROR: %s Bad Back bone Port Flag !!!!(%s,%d)\n",
                 FUNCTION_NAME(),__FILE__,__LINE__));
            return BCM_E_PARAM;
        }

        if(mim_port->criteria & ~_BCM_FE2000_MIM_BB_SUPPORTED_CRITERIA) {
            MIM_ERR(("ERROR: %s Bad Bone port criteria !!!!(%s,%d)\n",
                 FUNCTION_NAME(),__FILE__,__LINE__));
            return BCM_E_PARAM;
        }

        if(BCM_MAC_IS_ZERO(mim_port->match_tunnel_srcmac)){
            MIM_ERR(("ERROR: %s Bad Back bone Port SMAC zero !!!!(%s,%d)\n",
                 FUNCTION_NAME(),__FILE__,__LINE__));
            return BCM_E_PARAM;
        }
    }

    if((_MIM_INVALID_VPN_ID == vpn) || (!mim_port)) {
        MIM_ERR(("ERROR: %s No vpn or mim_port pointer specified !!!!(%s,%d)\n",
                 FUNCTION_NAME(),__FILE__,__LINE__));
      return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(_MIM_LOCK(unit));
    _MIM_VPN_INIT_CHECK(unit);

    /* verify if the vpn exisits on the database */
    dummycb.vpnid = vpn;
    dummycb.isid  = _MIM_INVALID_ISID; /* throw in invalid isid to do vpnid only match */
    hdl.key = &dummycb;
    hdl.datum = &vpncb;

    mim_port->mim_port_id = 0;

    if(!_AVL_EMPTY(mim_vpn_db[unit])) {
        result=shr_avl_lookup_lkupdata(mim_vpn_db[unit],
                                       _bcm_fe2000_mim_vpn_lookup_compare,
                                       (shr_avl_datum_t*)&dummycb,
                                       (void*)&hdl);
        /* If vpn was found on the database */
        if(result > 0) {

            if((mim_port->flags & BCM_MIM_PORT_TYPE_BACKBONE) &&
               (BCM_MAC_IS_ZERO(mim_port->egress_tunnel_dstmac))) {
                /* find the port on default BB List*/
                DQ_TRAVERSE(&vpncb->def_bbone_plist, port_elem) {

                    _BCM_FE2000_GET_PORTCB_FROM_LIST(port_elem, portcb);

                    if(_BCM_Fe2000_IS_MAC_EQUAL(mim_port->match_tunnel_srcmac, portcb->smac) &&
                       (portcb->vlan == mim_port->egress_service_vlan)){
                        mim_port->mim_port_id = portcb->gport;
                        break;
                    }
                } DQ_TRAVERSE_END(&vpncb->def_bbone_plist, port_elem);
            } else {

                    if(mim_port->flags & BCM_MIM_PORT_TYPE_ACCESS) {
                    /* access port */
                    DQ_TRAVERSE(&vpncb->vpn_access_sap_head, port_elem) {
                        
                        _BCM_FE2000_GET_PORTCB_FROM_LIST(port_elem, portcb);

                            if(mim_port->criteria == portcb->criteria) {

                                if(mim_port->criteria & BCM_MIM_PORT_MATCH_PORT) {

                                    if(mim_port->port == portcb->port) {
                                        mim_port->mim_port_id = portcb->gport;
                                        break;
                                    }
                                } else if(mim_port->criteria & BCM_MIM_PORT_MATCH_PORT_VLAN) {

                                    if((mim_port->port == portcb->port) &&
                                       (mim_port->match_vlan == portcb->vlan)) {
                                        mim_port->mim_port_id = portcb->gport;
                                        break;
                                    }
                                } else {

                                    MIM_ERR(("ERROR %s Bad Match Criteria for Access port !!!!(%s,%d)\n",\
                                             FUNCTION_NAME(),__FILE__,__LINE__));
                                    status = BCM_E_PARAM;
                                }
                            }

                    } DQ_TRAVERSE_END(&vpncb->vpn_access_sap_head, port_elem);

                    } else {
                        /* back bone port */
                    DQ_TRAVERSE(&vpncb->vpn_bbone_sap_head, port_elem) {

                        _BCM_FE2000_GET_PORTCB_FROM_LIST(port_elem, portcb);
                            if(_BCM_Fe2000_IS_MAC_EQUAL(mim_port->egress_tunnel_dstmac, portcb->dmac) &&
                               (portcb->vlan == mim_port->egress_service_vlan)){

                                mim_port->mim_port_id = portcb->gport;
                                break;
                            }
                    } DQ_TRAVERSE_END(&vpncb->vpn_bbone_sap_head, port_elem);
                        }
                    }
        } else {
            MIM_ERR(("ERROR: %s VPN ID [%d] Not  Found on database !!!!(%s,%d)\n",
                      FUNCTION_NAME(),vpn,__FILE__,__LINE__));

            status = BCM_E_NOT_FOUND;
        }
    } else {
        MIM_ERR(("ERROR: %s VPN ID [%d] Not  Found - VPN Tree empty !!!!(%s,%d)\n",
                 FUNCTION_NAME(),vpn,__FILE__,__LINE__));

        status = BCM_E_NOT_FOUND;
    }

    if(BCM_FAILURE(status)) {
        MIM_ERR(("ERROR: %s MiM Port Not  Found on database !!!!(%s,%d)\n",
                  FUNCTION_NAME(),__FILE__,__LINE__));
        status = BCM_E_NOT_FOUND;
    } else if(mim_port->mim_port_id == 0){
        MIM_ERR(("ERROR: %s MiM Port Not  Found on database !!!!(%s,%d)\n",
                  FUNCTION_NAME(),__FILE__,__LINE__));
        status = BCM_E_NOT_FOUND;
    }

    _MIM_UNLOCK(unit);
     return status;
}

int
bcm_fe2000_mim_port_get_all(int            unit,
                            bcm_mim_vpn_t  vpn,
                            int            port_max,
                            bcm_mim_port_t *mim_array,
                            int            *port_count)
{

    int portCount = 0;
    int rv;
    bcm_fe2k_mim_vpn_control_t *vpnCtl;
    bcm_fe2k_mim_port_control_t *portCtl = NULL;
    dq_p_t dqE;

    if (mim_array == NULL || port_count == NULL || port_max <= 0) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(_MIM_LOCK(unit));
    _MIM_VPN_INIT_CHECK(unit);

    rv = _bcm_fe2000_mim_vpn_control_get(unit, vpn, &vpnCtl);

    if (BCM_FAILURE(rv)) {
        _MIM_UNLOCK(unit);
        MIM_ERR((_MD("Failed to retrieve VPN Control data 0x%08x info: "
                     "%d %s\n"), vpn, rv, bcm_errmsg(rv)));
        return rv;
    }

    /* Grab any default backbone ports on the VPN */
    DQ_TRAVERSE (&vpnCtl->def_bbone_plist, dqE) {

        _BCM_FE2000_GET_PORTCB_FROM_LIST(dqE, portCtl);
        if (portCount < port_max) {
            _bcm_fe2000_mim_port_control_xlate(portCtl, &mim_array[portCount]);
            portCount++;
        } else {
            MIM_WARN((_MD("Insufficient memory to return port info"
                          ", skipping\n")));
        }
    } DQ_TRAVERSE_END(&vpnCtl->def_bbone_plist, dqE);

    /* Grab the configured access ports */
    DQ_TRAVERSE(&vpnCtl->vpn_access_sap_head, dqE) {

        _BCM_FE2000_GET_PORTCB_FROM_LIST(dqE, portCtl);
        if (portCount < port_max) {
            _bcm_fe2000_mim_port_control_xlate(portCtl, &mim_array[portCount]);
            portCount++;
        } else {
            MIM_WARN((_MD("Insufficient memory to return port info"
                          ", skipping\n")));
        }
    } DQ_TRAVERSE_END(&vpnCtl->vpn_access_sap_head, dqE);


    /* Grab the configured back bone ports */
    DQ_TRAVERSE(&vpnCtl->vpn_bbone_sap_head, dqE) {

        _BCM_FE2000_GET_PORTCB_FROM_LIST(dqE, portCtl);
        if (portCount < port_max) {
            _bcm_fe2000_mim_port_control_xlate(portCtl, &mim_array[portCount]);
            portCount++;
        } else {
            MIM_WARN((_MD("Insufficient memory to return port info"
                          ", skipping\n")));
        }
    } DQ_TRAVERSE_END(&vpnCtl->vpn_bbone_sap_head, dqE);

    _MIM_UNLOCK(unit);
    *port_count = portCount;
    return rv;
}

/*
 * Function:
 *   bcm_fe2000_multicast_mim_encap_get
 * Purpose:
 *     Returns Encap ID for Access or Default Back bone
 *     MiM port. Non-Default Back bone ports are invalid parameter
 *
 * Parameters:
 *     unit  - (IN)     fe unit to initialize
 *     group - (IN)     Multicast group
 *     port  - (IN)     physical port
 *     mim_port_id - (IN)  MiM port
 *     encap_id    - (IN/OUT) Encap ID
 * Returns:
 *   BCM_E_XX
 */
int _bcm_fe2000_multicast_mim_encap_get(int unit,
                                        bcm_multicast_t group,
                                        bcm_gport_t port,
                                        bcm_gport_t mim_port_id,
                                        bcm_if_t *encap_id)
{
    int status = BCM_E_NONE;

    if(!encap_id) {

        MIM_ERR(("Error %s No Encap ID pointer specified !!!!(%s,%d)\n",\
                 FUNCTION_NAME(),__FILE__,__LINE__));

        status = BCM_E_PARAM;

    } else {
        uint16 lport;
        bcm_fe2k_mim_port_control_t *portcb;
        uint32 portid = BCM_GPORT_MIM_PORT_ID_GET(mim_port_id);


        /* For MiM only the Default Back Bone port can be added to the
         * multicast group. Verify and flag error if it is not */
        /* sanity checks */
        G2P3_ONLY_SUPPORT_CHECK(unit);

        if(_MIM_UNIT_INVALID(unit)) {
            return BCM_E_PARAM;
        }

        /* verify the gport id */
        G2P3_GPORT_RANGE_CHECK(unit, mim_port_id);

        BCM_IF_ERROR_RETURN(_MIM_LOCK(unit));

        /* obtain logical port from mim gport */
        lport = gportinfo[unit].lpid[portid];

        if(BCM_GPORT_MIM_PORT != SBX_LPORT_TYPE(unit,lport)) {

            MIM_ERR(("Error %s LP GPORT not a MiM port !!!!(%s,%d)\n",\
                     FUNCTION_NAME(),__FILE__,__LINE__));

            status = BCM_E_PARAM;

        } else {
            portcb = (bcm_fe2k_mim_port_control_t*)SBX_LPORT_DATAPTR(unit, lport);
            *encap_id = SOC_SBX_ENCAP_ID_FROM_OHI(portcb->hwinfo.ohi);
            MIM_VERB((" %s ENCAP-ID[0x%x] \n", FUNCTION_NAME(), *encap_id));
        }

        _MIM_UNLOCK(unit);
    }

    return status;
}

/*
 * Function:
 *   _bcm_fe2000_mim_fte_gport_get
 * Purpose:
 *     Given MIM Gport, returns FTIDX for the port
 *
 * Parameters:
 *     unit  - (IN)     fe unit to initialize
 *     mim_port - (IN)  MiM port
 *     ftidx    - (IN/OUT) Forwarding index
 * Returns:
 *   BCM_E_XX
 */
int _bcm_fe2000_mim_fte_gport_get(int unit,
                                  bcm_gport_t mim_port_id,
                                  uint32 *ftidx)
{
    int status = BCM_E_NONE;
    uint32 portid = 0;

    if(!ftidx) {

        MIM_ERR(("Error %s No FTIDX pointer specified !!!!(%s,%d)\n",\
                 FUNCTION_NAME(),__FILE__,__LINE__));

        status = BCM_E_PARAM;

    } else {
        /* For MiM only the Default Back Bone port can be added to the
         * multicast group. Verify and flag error if it is not */
        /* sanity checks */
        G2P3_ONLY_SUPPORT_CHECK(unit);

        if(_MIM_UNIT_INVALID(unit)) {
            return BCM_E_PARAM;
        }

        /* verify the gport id */
        G2P3_GPORT_RANGE_CHECK(unit, mim_port_id);
        portid = BCM_GPORT_MIM_PORT_ID_GET(mim_port_id);
        *ftidx = _BCM_FE2000_MIM_PORTID_2_FTIDX(unit, portid);
 
        MIM_VERB((" %s FTE[0x%x] \n", FUNCTION_NAME(), *ftidx));
    }
    return status;
}

bcm_gport_t _bcm_fe2000_mim_fte_to_gport_id(int unit,
                                    uint32 ftidx)
{
    bcm_gport_t gportid = 0;
    uint32 portid = _BCM_FE2000_FTIDX_2_MIM_PORTID(unit, ftidx);

    BCM_GPORT_MIM_PORT_ID_SET(gportid, portid);
    return gportid;
}

/* System Test Support Function */
int soc_sbx_g2p3_mim_lsm_get(int unit,
                            int mim_port_id,
                            int  *lsm)
{
    int status = BCM_E_NONE;

    if(!lsm) {

        MIM_ERR(("Error %s No LSM pointer specified !!!!(%s,%d)\n",\
                 FUNCTION_NAME(),__FILE__,__LINE__));

        status = BCM_E_PARAM;

    } else {
        uint16 lport;
        bcm_fe2k_mim_port_control_t *portcb;
        uint32 portid = BCM_GPORT_MIM_PORT_ID_GET(mim_port_id);


        /* For MiM only the Default Back Bone port can be added to the
         * multicast group. Verify and flag error if it is not */
        /* sanity checks */
        G2P3_ONLY_SUPPORT_CHECK(unit);

        if(_MIM_UNIT_INVALID(unit)) {
            return BCM_E_PARAM;
        }

        /* verify the gport id */
        G2P3_GPORT_RANGE_CHECK(unit, mim_port_id);

        BCM_IF_ERROR_RETURN(_MIM_LOCK(unit));

        /* obtain logical port from mim gport */
        lport = gportinfo[unit].lpid[portid];

        if(BCM_GPORT_MIM_PORT != SBX_LPORT_TYPE(unit,lport)) {

            MIM_ERR(("Error %s LP GPORT not a MiM port !!!!(%s,%d)\n",\
                     FUNCTION_NAME(),__FILE__,__LINE__));

            status = BCM_E_PARAM;

        } else {
            portcb = (bcm_fe2k_mim_port_control_t*)SBX_LPORT_DATAPTR(unit, lport);
            *lsm = portcb->hwinfo.ismacidx;
            MIM_VERB((" %s LSM-ID[0x%x] \n", FUNCTION_NAME(), *lsm));
        }

        _MIM_UNLOCK(unit);
    }

    return status;
}

int
_bcm_fe2000_mim_set_egress_remarking(int unit, 
                                     uint32 eteencapidx, 
                                     uint32 egrMap,
                                     uint32 egrFlags)
{
    int status = BCM_E_NONE;
    soc_sbx_g2p3_eteencap_t    eteencap;
    
    /* If explicit egress qos marking was not set on
     * this bdmac gport, dont disturb the qos remarking */  
    status = soc_sbx_g2p3_eteencap_get(unit,
                                       eteencapidx,
                                       &eteencap);
    if(BCM_FAILURE(status)) {
        MIM_ERR(("Error[%d] %s Setting ETEEncap Remark !!!!(%s,%d)\n",\
                 status, FUNCTION_NAME(),__FILE__,__LINE__));
    } else {
        eteencap.remark = egrMap;
        eteencap.dscpremark = (egrFlags & BCM_QOS_MAP_L3)?1:0;
        status = soc_sbx_g2p3_eteencap_set(unit,
                                           eteencapidx,
                                           &eteencap);   
        if(BCM_FAILURE(status)) {
            MIM_ERR(("Error[%d] %s Setting ETEEncap Remark !!!!(%s,%d)\n",\
                     status, FUNCTION_NAME(),__FILE__,__LINE__));
        } else {
            MIM_VERB((_MD("Updated ete=0x%x remark=0x%x\n"),
                      eteencapidx, egrMap));
        }
    }
    return status;
}

int 
_bcm_fe2000_mim_bbone_egress_qosmap_set(int unit, 
                                        bcm_fe2k_mim_port_control_t *bbportcb,
                                        int egrMap, uint32 egrFlags)
{
    bcm_fe2k_mim_vpn_control_t *vpncb;
    bcm_fe2k_mim_port_control_t *tmpportcb;
    dq_p_t port_elem;
    int status = BCM_E_NONE;
    uint32 oldremarkidx=0;

    if(egrMap >= 0) {

        if(!bbportcb ||
           (bbportcb->type != _BCM_FE2K_MIM_DEF_BACKBONE_PORT &&
            bbportcb->type != _BCM_FE2K_MIM_BACKBONE_PORT)) {
            status = BCM_E_PARAM;
        } else {

            status  = _bcm_fe2000_mim_vpn_control_get(unit, bbportcb->isidvlan, &vpncb);
            if (BCM_FAILURE(status)) {
                
                MIM_ERR(("ERROR: Could not find VPN control for this VPN: unit %d  vpn  0x%x\n",
                         unit, bbportcb->isidvlan));
                
                status = BCM_E_INTERNAL;

            } else {

                if (bbportcb->type == _BCM_FE2K_MIM_DEF_BACKBONE_PORT) {

                    /* new qos profile is trying to be applied */
                    if (bbportcb->hwinfo.egrremarkidx > 0) {
                        oldremarkidx = bbportcb->hwinfo.egrremarkidx;  
                    }

                    /* Check if there is a Mim Port using this port */
                    DQ_TRAVERSE(&vpncb->vpn_bbone_sap_head, port_elem) {
                        
                        _BCM_FE2000_GET_PORTCB_FROM_LIST(port_elem, tmpportcb);

                        if(_BCM_Fe2000_IS_MAC_EQUAL(bbportcb->smac, tmpportcb->smac)) {

                            if((oldremarkidx == 0 && tmpportcb->hwinfo.egrremarkidx == 0) ||
                               (oldremarkidx && oldremarkidx == tmpportcb->hwinfo.egrremarkidx)) {
                                /* set remarking index */
                                status = _bcm_fe2000_mim_set_egress_remarking(unit, 
                                                                              tmpportcb->hwinfo.eteencap,
                                                                              egrMap,
                                                                              egrFlags);
                                if(BCM_FAILURE(status)) {
                                    /* Failed to pass on remarking properties to childs */
                                    break;
                                } else {
                                    tmpportcb->hwinfo.egrremarkidx = egrMap;
                                    tmpportcb->hwinfo.egrremarkflags = egrFlags;
                                }
                            }
                        }
                        
                    } DQ_TRAVERSE_END(&vpncb->vpn_bbone_sap_head, port_elem);

                    if(BCM_SUCCESS(status)) {
                        /* set remarking index */
                        status = _bcm_fe2000_mim_set_egress_remarking(unit, 
                                                                      bbportcb->hwinfo.eteencap,
                                                                      egrMap,
                                                                      egrFlags);
                        if(BCM_SUCCESS(status)) {
                            bbportcb->hwinfo.egrremarkidx = egrMap;
                            bbportcb->hwinfo.egrremarkflags = egrFlags;
                        }
                    }
                } else {

                    /* set Remarking for Back bone gports */
                    /* If egress map is 0, try to inherit remark property from default backbone port */
                    if(egrMap == 0) {
                        bcm_fe2k_mim_port_control_t *defbbportcb = NULL;
                        DQ_TRAVERSE(&vpncb->def_bbone_plist, port_elem) {
                            _BCM_FE2000_GET_PORTCB_FROM_LIST(port_elem, defbbportcb);

                            if(_BCM_Fe2000_IS_MAC_EQUAL(defbbportcb->smac, bbportcb->smac)) {
                                if(defbbportcb->hwinfo.egrremarkidx > 0) {
                                    status = _bcm_fe2000_mim_set_egress_remarking(unit, 
                                                                        bbportcb->hwinfo.eteencap,
                                                                        defbbportcb->hwinfo.egrremarkidx,
                                                                        defbbportcb->hwinfo.egrremarkflags);
                                    if(BCM_FAILURE(status)) {
                                        break;
                                    } else {
                                        /* inherit from default backbone port */
                                        bbportcb->hwinfo.egrremarkidx = defbbportcb->hwinfo.egrremarkidx;
                                        bbportcb->hwinfo.egrremarkflags = defbbportcb->hwinfo.egrremarkflags;
                                        break;
                                    }
                                }
                            }
                        } DQ_TRAVERSE_END(&vpncb->def_bbone_plist, port_elem);
                    } else {
                        /* if non-zero, use it */
                       status = _bcm_fe2000_mim_set_egress_remarking(unit, 
                                                                      bbportcb->hwinfo.eteencap,
                                                                      egrMap,
                                                                      egrFlags);
                        if(BCM_SUCCESS(status)) {
                            bbportcb->hwinfo.egrremarkidx = egrMap;
                            bbportcb->hwinfo.egrremarkflags = egrFlags;
                        }
                    }
                }        
            }
        }     
    }
    return status;
}

/*
 * Function:
 *   _bcm_fe2000_mim_qosmap_set
 * Purpose:
 *     Given MIM Gport, set qos mapping profile to it
 *
 * Parameters:
 *     unit  - (IN)     fe unit to initialize
 *     mim_port - (IN)  MiM port
 *     qos map and flags
 * Returns:
 *   BCM_E_XX
 */
int
_bcm_fe2000_mim_qosmap_set(int unit, bcm_gport_t port, 
                          int ing_idx, int egr_idx,
                          uint32 ingFlags, uint32 egrFlags)
{
    uint16 lport;
    bcm_fe2k_mim_port_control_t *portcb;
    uint32 portid = BCM_GPORT_MIM_PORT_ID_GET(port);
    int status = BCM_E_NONE;

    /* For MiM only the Default Back Bone port can be added to the
     * multicast group. Verify and flag error if it is not */
    /* sanity checks */
    G2P3_ONLY_SUPPORT_CHECK(unit);


    if(_MIM_UNIT_INVALID(unit)) {
        return BCM_E_PARAM;
    }
    
    /* verify the gport id */
    G2P3_GPORT_RANGE_CHECK(unit, port);
    
    BCM_IF_ERROR_RETURN(_MIM_LOCK(unit));
    
    /* obtain logical port from mim gport */
    lport = gportinfo[unit].lpid[portid];

    if(BCM_GPORT_MIM_PORT != SBX_LPORT_TYPE(unit,lport)) {
        MIM_ERR(("Error %s LP GPORT not a MiM port !!!!(%s,%d)\n",\
                 FUNCTION_NAME(),__FILE__,__LINE__));
        
        status = BCM_E_PARAM;
        
    } else {
        /* works only for access & default back bone ports !!! */
        portcb = (bcm_fe2k_mim_port_control_t*)SBX_LPORT_DATAPTR(unit, lport);

        if(ing_idx >= 0) {
            soc_sbx_g2p3_lp_t lp;
            int lpi = -1;

        if (portcb->type == _BCM_FE2K_MIM_DEF_BACKBONE_PORT) {
            soc_sbx_g2p3_isid2e_t isid2e;

            /* when setting the qosMap on the default backbone port, set
             * use the isid2e logical port
             */
            status = soc_sbx_g2p3_isid2e_get(unit, portcb->isid, &isid2e);
            if (BCM_FAILURE(status)) {
                MIM_ERR((_MD("failed to read isid2e[0x%06x]:%d %s\n"), 
                         portcb->isid, status, bcm_errmsg(status)));                
            }

            if (isid2e.lpi == 0) {
                MIM_ERR((_MD("unexpected physical port for logicial port on isid 0x%06x\n"),
                         portcb->isid));
                status = BCM_E_INTERNAL;
            }

            if (BCM_SUCCESS(status)) {
                lpi = isid2e.lpi;
            }

        } else if (portcb->type == _BCM_FE2K_MIM_ACCESS_PORT) {
            lpi = portcb->hwinfo.lport;
        } else {
            MIM_ERR(("Error %s Works only for Access and Default Backbone MiM "
                     "ports !!!!(%s,%d)\n",
                     FUNCTION_NAME(),__FILE__,__LINE__));
            
            status = BCM_E_PARAM;
        }
        
        /* Valid port type an have necessary information */
        if (BCM_SUCCESS(status)) {
            if(ing_idx >= 0) {
                status = soc_sbx_g2p3_lp_get(unit, lpi, &lp);        
                
                if(BCM_SUCCESS(status)) {
                    lp.qos = ing_idx;
                    lp.usedscp = (ingFlags & BCM_QOS_MAP_L3) ? 1 : 0;
                    lp.useexp = (ingFlags & BCM_QOS_MAP_MPLS) ? 1 : 0;
                    status = soc_sbx_g2p3_lp_set(unit, lpi, &lp); 
                    if(BCM_FAILURE(status)) {
                        MIM_ERR(("Error[%d] %s Setting Logical Port !!!!(%s,%d)\n",\
                                 status, FUNCTION_NAME(),__FILE__,__LINE__));
                    } else {
                        MIM_VERB((_MD("Updated lpi=%d qos=%d\n"), 
                                  lpi, lp.qos));
                    }
                } else {
                    MIM_ERR(("Error[%d] %s Reading Logical Port !!!!(%s,%d)\n",\
                             status, FUNCTION_NAME(),__FILE__,__LINE__));
                }
            }
            }     
        }

            if (BCM_SUCCESS(status) && (egr_idx >= 0)) {
                
            if (portcb->type == _BCM_FE2K_MIM_DEF_BACKBONE_PORT ||
                portcb->type == _BCM_FE2K_MIM_BACKBONE_PORT) {
                status = _bcm_fe2000_mim_bbone_egress_qosmap_set(unit, portcb, egr_idx, egrFlags);
                    } else {
                status = _bcm_fe2000_mim_set_egress_remarking(unit, 
                                                              portcb->hwinfo.eteencap,
                                                              egr_idx,
                                                              egrFlags);
                if(BCM_SUCCESS(status)) {
                    portcb->hwinfo.egrremarkidx = egr_idx;
                    portcb->hwinfo.egrremarkflags = egrFlags;
                        MIM_VERB((_MD("Updated ete=0x%x remark=0x%x\n"),
                              portcb->hwinfo.eteencap, egr_idx));
                } else {
                    MIM_ERR(("Error[%d] %s Setting egress remarking !!!!(%s,%d)\n",\
                             status, FUNCTION_NAME(),__FILE__,__LINE__));
                }
            }
        }
    }
    
    _MIM_UNLOCK(unit);
    return status;
}


/*
 * Function:
 *   _bcm_fe2000_g2p3_mim_qosmap_get
 * Purpose:
 *   Get a mim gports qos mappings, for g2p3;
 *
 * Parameters:
 *     unit  - (IN)     fe unit to initialize
 *     mim_port - (IN)  MiM port
 *     qos map and flags
 * Returns:
 *   BCM_E_XX
 *
 * notes:
 *   no param checking
 *   lock must be taken by caller
 */
static int
_bcm_fe2000_g2p3_mim_qosmap_get(int unit, bcm_gport_t port, 
                                int *ing_idx, int *egr_idx,
                                uint32 *ing_flags, uint32 *egr_flags)
{
    int                          rv;
    bcm_fe2k_mim_port_control_t *port_data;
    soc_sbx_g2p3_lp_t            lp;
    soc_sbx_g2p3_etel2_t         ete;
    uint32                       lport, lpi;
    uint32 portid = BCM_GPORT_MIM_PORT_ID_GET(port);

    /* obtain logical port from mim gport */
    lport = gportinfo[unit].lpid[portid];

    if (SBX_LPORT_TYPE(unit, lport) != BCM_GPORT_MIM_PORT) {
        MIM_ERR((_MD("Invalid GPORT type found at 0x%x\n"), lport));
        return BCM_E_PARAM;
    } 

    /* works only for access & default back bone ports */
    port_data = (bcm_fe2k_mim_port_control_t*)SBX_LPORT_DATAPTR(unit, lport);
    
    if (port_data->type == _BCM_FE2K_MIM_DEF_BACKBONE_PORT) {
        soc_sbx_g2p3_isid2e_t isid2e;

        rv = soc_sbx_g2p3_isid2e_get(unit, port_data->isid, &isid2e);
        if (BCM_FAILURE(rv)) {
            MIM_ERR((_MD("failed to read isid2e[0x%06x]:%d %s\n"), 
                     port_data->isid, rv, bcm_errmsg(rv)));
            return rv;
        }
        
        if (isid2e.lpi == 0) {
            MIM_ERR((_MD("unexpected physical port for logicial port "
                         "on isid 0x%06x\n"), port_data->isid));
            return BCM_E_INTERNAL;
        }
        
        lpi = isid2e.lpi;

    } else if (port_data->type == _BCM_FE2K_MIM_ACCESS_PORT) {
        lpi = port_data->hwinfo.lport;

    } else {
        MIM_ERR((_MD("Invalid port type found: expected Access or"
                     " Default backbone\n")));
        return BCM_E_PARAM;
    }
        
    rv = soc_sbx_g2p3_lp_get(unit, lpi, &lp);
    if (BCM_FAILURE(rv)) {
        MIM_ERR((_MD("Failed to read lp[0x%x]: %d %s\n"),
                 lpi, rv, bcm_errmsg(rv)));
        return rv;
    }

    *ing_idx    = lp.qos;
    *ing_flags  = BCM_QOS_MAP_INGRESS | BCM_QOS_MAP_L2;
    if (lp.usedscp) {
        *ing_flags |= BCM_QOS_MAP_L3;
    }
    if (lp.useexp) {
        *ing_flags |=  BCM_QOS_MAP_MPLS;   
    }

    /* Get Egress mapping 
     */
    if (port_data->type == _BCM_FE2K_MIM_DEF_BACKBONE_PORT ||
        port_data->type == _BCM_FE2K_MIM_BACKBONE_PORT) {

        rv = soc_sbx_g2p3_etel2_get(unit, port_data->hwinfo.etel2, &ete);
        if (BCM_FAILURE(rv)) {
            MIM_ERR((_MD("Failed to read etel2[0x%x]: %d %s\n"),
                     port_data->hwinfo.etel2, rv, bcm_errmsg(rv)));
            return rv;
        }
        *egr_idx   = ete.remark;
        *egr_flags = BCM_QOS_MAP_EGRESS | BCM_QOS_MAP_L2;
        if (ete.dscpremark) {
            *egr_flags |= BCM_QOS_MAP_L3;
        }
    } else {
        rv = BCM_E_PARAM;
    }

    return rv;

}

/*
 * Function:
 *   _bcm_fe2000_mim_qosmap_get
 * Purpose:
 *   Get a mim gports qos mappings;
 *
 * Parameters:
 *     unit  - (IN)     fe unit to initialize
 *     mim_port - (IN)  MiM port
 *     qos map and flags
 * Returns:
 *   BCM_E_XX
 */
int 
_bcm_fe2000_mim_qosmap_get(int unit, bcm_gport_t port, 
                           int *ing_idx, int *egr_idx,
                           uint32 *ing_flags, uint32 *egr_flags)
{
    int rv = BCM_E_UNAVAIL;
    G2P3_ONLY_SUPPORT_CHECK(unit);

    if(_MIM_UNIT_INVALID(unit)) {
        return BCM_E_PARAM;
    }
    
    /* verify the gport id */
    G2P3_GPORT_RANGE_CHECK(unit, port);
    
    BCM_IF_ERROR_RETURN(_MIM_LOCK(unit));

    rv = _bcm_fe2000_g2p3_mim_qosmap_get(unit, port, ing_idx, egr_idx,
                                         ing_flags, egr_flags);
    
    _MIM_UNLOCK(unit);
    return rv;

}

/*
 * Function:
 *   _bcm_fe2000_mim_stp_update
 * Purpose:
 *     Given MIM Gport, set stp state on it
 *
 * Parameters:
 *     unit  - (IN)     fe unit to initialize
 *     mim_port - (IN)  MiM port
 *     stp state
 * Returns:
 *   BCM_E_XX
 */
int _bcm_fe2000_mim_stp_update(int unit, 
                               bcm_gport_t port, 
                               int *stp_state,
                               int set)
{
    uint16 lport;
    bcm_fe2k_mim_port_control_t *portcb;
    uint32 portid = BCM_GPORT_MIM_PORT_ID_GET(port);
    int status = BCM_E_NONE;
    soc_sbx_g2p3_pv2e_t   pv2e;
    soc_sbx_g2p3_epv2e_t  epv2e;

    /* For MiM only the Default Back Bone port can be added to the
     * multicast group. Verify and flag error if it is not */
    /* sanity checks */
    G2P3_ONLY_SUPPORT_CHECK(unit);


    if(_MIM_UNIT_INVALID(unit)) {
        return BCM_E_PARAM;
    }
    
    /* verify the gport id */
    G2P3_GPORT_RANGE_CHECK(unit, port);
    
    BCM_IF_ERROR_RETURN(_MIM_LOCK(unit));
    
    /* obtain logical port from mim gport */
    lport = gportinfo[unit].lpid[portid];

    if(BCM_GPORT_MIM_PORT != SBX_LPORT_TYPE(unit,lport)) {
        MIM_ERR(("Error %s LP GPORT not a MiM port !!!!(%s,%d)\n",\
                 FUNCTION_NAME(),__FILE__,__LINE__));
        
        status = BCM_E_PARAM;
        
    } else {
        bcm_vlan_t vid=0;
        uint32 phy_port=0;

        /* works only for access & default back bone ports !!! */
        portcb = (bcm_fe2k_mim_port_control_t*)SBX_LPORT_DATAPTR(unit, lport);
        phy_port = BCM_GPORT_MODPORT_PORT_GET(portcb->port);

        if (portcb->type == _BCM_FE2K_MIM_ACCESS_PORT) {
            
            if(portcb->service == _BCM_FE2K_MIM_PORT) {
                /* set STP on port + All VID */
                vid = 0;
            } else if (portcb->service == _BCM_FE2K_MIM_STAG_1_1) {
                /* set STP on port + stag */
                vid = portcb->vlan;
            } else {
                status = BCM_E_INTERNAL;
                MIM_ERR(("Error %s Unknown Access Port Service Type !!!!(%s,%d)\n",
                         FUNCTION_NAME(),__FILE__,__LINE__));
            }
            if(BCM_SUCCESS(status)) {

                status = soc_sbx_g2p3_pv2e_get(unit, vid, phy_port, &pv2e);
                if(BCM_SUCCESS(status)) {

                    status = soc_sbx_g2p3_epv2e_get(unit, vid, phy_port, &epv2e);
                    if(BCM_SUCCESS(status)) {

                        if(set) {
                            pv2e.stpstate = _bcm_fe2000_g2p3_stg_stp_translate(unit, *stp_state);
                            epv2e.drop = ((*stp_state == BCM_STG_STP_FORWARD ||
                                                  *stp_state == BCM_STG_STP_DISABLE) ? 0 : 1);

                            status = soc_sbx_g2p3_pv2e_set(unit, vid, phy_port, &pv2e);
                            if(BCM_SUCCESS(status)) {
                                /* for port mode set stp on all vid's since packet
                                 * can exit with CTAG on it and STP has to be based on CTAG */
                                if(portcb->service == _BCM_FE2K_MIM_PORT) {
                                    int *fastSets = NULL;
                                    uint32_t  *fastDrops = NULL;

                                    fastSets = sal_alloc(BCM_VLAN_COUNT * sizeof(int),
                                                          "fastSets temp");
                                    if (fastSets == NULL) {
                                        status = BCM_E_MEMORY;
                                    }
                                    
                                    if(fastSets) {
                                    
                                    fastDrops = sal_alloc(BCM_VLAN_COUNT * sizeof(uint32_t),
                                                          "fastDrops temp");
                                    if (fastDrops == NULL) {
                                        status = BCM_E_MEMORY;
                                            sal_free(fastSets);
                                        }
                                    }

                                    if(fastSets && fastDrops && BCM_SUCCESS(status)) {
                                        bcm_vlan_t vididx;
                                        for (vididx = BCM_VLAN_MIN;  vididx < BCM_VLAN_COUNT; vididx++) {
                                            fastSets[vididx] = 1;
                                            fastDrops[vididx] = epv2e.drop;
                                        }

                                        status = soc_sbx_g2p3_epv2e_drop_fast_set(unit, BCM_VLAN_MIN, phy_port,
                                                                              BCM_VLAN_MAX, phy_port,
                                                                              &fastSets[BCM_VLAN_MIN],
                                                                              &fastDrops[BCM_VLAN_MIN],
                                                                              BCM_VLAN_COUNT);
                                        if(BCM_FAILURE(status)) {
                                            status = BCM_E_INTERNAL;
                                            MIM_ERR(("Error %s Could not set EPV2E !!!!(%s,%d)\n",
                                                     FUNCTION_NAME(),__FILE__,__LINE__));
                                        }

                                        sal_free(fastDrops);
                                        sal_free(fastSets);
                                    } else {
                                        status = BCM_E_INTERNAL;
                                        MIM_ERR(("Error %s Could not set EPV2E !!!!(%s,%d)\n",
                                                 FUNCTION_NAME(),__FILE__,__LINE__));
                                    }
                                } else {
                                    status = soc_sbx_g2p3_epv2e_set(unit, vid, phy_port, &epv2e);
                                    if(BCM_FAILURE(status)) {
                                        status = BCM_E_INTERNAL;
                                        MIM_ERR(("Error %s Could not set EPV2E !!!!(%s,%d)\n",
                                                 FUNCTION_NAME(),__FILE__,__LINE__));
                                    }
                                }
                            } else {
                                status = BCM_E_INTERNAL;
                                MIM_ERR(("Error %s Could not set PV2E !!!!(%s,%d)\n",
                                         FUNCTION_NAME(),__FILE__,__LINE__));
                            }
                        } else {
                            *stp_state = _bcm_fe2000_g2p3_stg_stp_translate_to_bcm(unit, pv2e.stpstate);
                        }
                    } else {
                        status = BCM_E_INTERNAL;
                        MIM_ERR(("Error %s Could not read EPV2E !!!!(%s,%d)\n",
                                 FUNCTION_NAME(),__FILE__,__LINE__));
                    }
                } else {
                    status = BCM_E_INTERNAL;
                    MIM_ERR(("Error %s Could not read PV2E !!!!(%s,%d)\n",
                             FUNCTION_NAME(),__FILE__,__LINE__));
                }
            }
        } else {
            MIM_ERR(("Error %s Works only for Access MiM "
                     "ports !!!!(%s,%d)\n",
                     FUNCTION_NAME(),__FILE__,__LINE__));
            
            status = BCM_E_PARAM;
        }
    }
    
    _MIM_UNLOCK(unit);
    return status;
}

/*
 * Function:
 *   _bcm_fe2000_mim_vpn_policer_set
 * Purpose:
 *    Set Policer ID for a VPN
 *
 * Parameters:
 *     unit  - (IN)     fe unit to initialize
 *     verify - if set, verify if policer is set on all logical port
 *              of vpn required to accumulate statistics
 * Returns:
 *   BCM_E_XX
 */
int 
_bcm_fe2000_mim_vpn_policer_set(int unit, 
                               bcm_mim_vpn_t vpn,
                               bcm_policer_t pol_id,
                               uint8_t       verify)
{
    int    rv;
    bcm_fe2k_mim_vpn_control_t *vpncb;
    bcm_fe2k_mim_port_control_t *portcb = NULL;
    dq_p_t                       port_elem;
    soc_sbx_g2p3_lp_t lp;

    /* sanity checks */
    G2P3_ONLY_SUPPORT_CHECK(unit);

    if(_MIM_UNIT_INVALID(unit)){
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(_MIM_LOCK(unit));
    _MIM_VPN_INIT_CHECK(unit);

    rv  = _bcm_fe2000_mim_vpn_control_get(unit, vpn, &vpncb);
    if (BCM_SUCCESS(rv)) {
        /* Set policer on Access MiM Gport's LP */
        /* Check if there is a Mim Port using this port */
        DQ_TRAVERSE(&vpncb->vpn_access_sap_head, port_elem) {
            
            _BCM_FE2000_GET_PORTCB_FROM_LIST(port_elem, portcb);

            if(!portcb->hwinfo.lport) {
                /* access ports lport must be non zero */
                MIM_ERR(("ERROR: BAD Access LPORT: unit %d gport %08X LpIndex %d\n",
                         unit, portcb->gport, portcb->hwinfo.lport));
            } else {
                rv = soc_sbx_g2p3_lp_get(unit, portcb->hwinfo.lport, &lp);
                if(BCM_SUCCESS(rv)) {
                    /* only verify if Logical ports has associate policer */
                    if(verify) {
                        if(lp.policer) {
                            MIM_VVERB(("Policer %d exists on gport %08x\n", lp.policer, portcb->gport));
                        } else {
                            MIM_ERR(("No Policer exists on gport %08x\n", portcb->gport));
                            rv = BCM_E_INTERNAL;
                        }
                    } else {
                        /* set new policer id */
                        rv = _bcm_fe2000_g2p3_policer_lp_program(unit, pol_id, &lp);
                        if(BCM_FAILURE(rv)) {
                            MIM_ERR(("ERROR: Setting LPORT: unit %d gport %08X LpIndex %d\n",
                                     unit, portcb->gport, portcb->hwinfo.lport));
                        } else {
                            
                            rv = soc_sbx_g2p3_lp_set(unit, portcb->hwinfo.lport, &lp);
                            if(BCM_FAILURE(rv)) {
                                MIM_ERR(("ERROR: Setting LPORT: unit %d gport %08X LpIndex %d\n",
                                         unit, portcb->gport, portcb->hwinfo.lport));
                            } else {
                                MIM_VVERB(("Setting LPORT: unit %d gport %08X LpIndex %d PolicerId: 0x%x\n",
                                           unit, portcb->gport, portcb->hwinfo.lport, pol_id));                            
                            }
                        }
                    }
                 } else {
                    MIM_ERR(("ERROR: Getting LPORT: unit %d gport %08X LpIndex %d\n",
                             unit, portcb->gport, portcb->hwinfo.lport));
                }
            }

            if(BCM_FAILURE(rv)) {
                break;
            }

        } DQ_TRAVERSE_END(&vpncb->vpn_access_sap_head, port_elem);
        
        if(BCM_SUCCESS(rv)) {
            /* Set policer on ISID2E LP - Backbone does not have dedicated LP */
            rv = soc_sbx_g2p3_lp_get(unit, vpncb->lport, &lp);
            if(BCM_SUCCESS(rv)) {
                /* only verify if Logical ports has associate policer */
                if(verify) {
                    if(lp.policer) {
                        MIM_VVERB(("Policer %d exists on ISID2E LP \n", lp.policer));
                    } else {
                        MIM_ERR(("No Policer exists on ISID2E LP \n"));
                        rv = BCM_E_INTERNAL;
                    }
                } else {
                    /* set new policer id */
                    rv = _bcm_fe2000_g2p3_policer_lp_program(unit, pol_id, &lp);
                    if(BCM_FAILURE(rv)) {
                        MIM_ERR(("ERROR: Setting Back Bone LPORT: unit %d  LpIndex %d\n",
                                 unit, vpncb->lport));
                    } else {
                        rv = soc_sbx_g2p3_lp_set(unit, vpncb->lport, &lp);
                        if(BCM_FAILURE(rv)) {
                            MIM_ERR(("ERROR: Setting LPORT: unit %d gport %08X LpIndex %d\n",
                                     unit, portcb->gport, vpncb->lport));
                        } else {
                            MIM_VVERB(("Setting LPORT: unit %d gport %08X LpIndex %d PolicerId: 0x%x\n",
                                       unit, portcb->gport, vpncb->lport, pol_id));                            
                        }
                    }
                }
            } else {
                MIM_ERR(("ERROR: Getting Back Bone LPORT: unit %d LpIndex %d\n",
                         unit, vpncb->lport));
            }        
        }
    }

    if(BCM_SUCCESS(rv) && (!verify)) {
        vpncb->policer_id = pol_id;
    }

    _MIM_UNLOCK(unit);
    return rv;
}


/*
 * Function:
 *   _bcm_fe2000_mim_vpn_policer_get
 * Purpose:
 *    Get Policer ID for a VPN
 *
 * Parameters:
 *     unit  - (IN)     fe unit to initialize
 * Returns:
 *   BCM_E_XX
 */
int
_bcm_fe2000_mim_vpn_policer_get(int unit, 
                               bcm_mim_vpn_t vpn,
                               bcm_policer_t *pol_id)
{
    int    rv;
    bcm_fe2k_mim_vpn_control_t *vpncb;

    /* sanity checks */
    G2P3_ONLY_SUPPORT_CHECK(unit);

    if(_MIM_UNIT_INVALID(unit) || (!pol_id)){
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(_MIM_LOCK(unit));
    _MIM_VPN_INIT_CHECK(unit);

    rv  = _bcm_fe2000_mim_vpn_control_get(unit, vpn, &vpncb);
    if (BCM_SUCCESS(rv)) {
        /* Set policer on Access MiM Gport's LP */
        if(vpncb->policer_id < 0) {
            MIM_ERR(("ERROR: No Policer Set on this VPN: unit %d  vpn  0x%x\n",
                     unit, vpn));
            rv = BCM_E_PARAM;
        } else {
            *pol_id = vpncb->policer_id;
        }
    }

    _MIM_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *   _bcm_fe2000_mim_policer_set
 * Purpose:
 *    Set Policer ID for a MiM Gport
 *
 * Parameters:
 *     unit  - (IN)     fe unit to initialize
 * Returns:
 *   BCM_E_XX
 */
int 
_bcm_fe2000_mim_policer_set(int unit, 
                            bcm_gport_t port,
                            bcm_policer_t pol_id)
{
    int status = BCM_E_NONE;
    uint16 lport;
    bcm_fe2k_mim_port_control_t *portcb = NULL;
    uint32 portid = BCM_GPORT_MIM_PORT_ID_GET(port);
    int lpi = -1;

    /* sanity checks */
    G2P3_ONLY_SUPPORT_CHECK(unit);

    if(_MIM_UNIT_INVALID(unit)){
        return BCM_E_PARAM;
    }
    
    /* verify the gport id */
    G2P3_GPORT_RANGE_CHECK(unit, port);
    
    BCM_IF_ERROR_RETURN(_MIM_LOCK(unit));
    
    /* obtain logical port from mim gport */
    lport = gportinfo[unit].lpid[portid];

    if(BCM_GPORT_MIM_PORT != SBX_LPORT_TYPE(unit,lport)) {
        MIM_ERR(("Error %s LP GPORT not a MiM port !!!!(%s,%d)\n",\
                 FUNCTION_NAME(),__FILE__,__LINE__));

        status = BCM_E_PARAM;
        
    } else {

        /* works only for access & default back bone ports !!! */
        portcb = (bcm_fe2k_mim_port_control_t*)SBX_LPORT_DATAPTR(unit, lport);

        if (portcb->type == _BCM_FE2K_MIM_DEF_BACKBONE_PORT) {
            soc_sbx_g2p3_isid2e_t isid2e;

            /* when setting the qosMap on the default backbone port, set
             * use the isid2e logical port
             */
            status = soc_sbx_g2p3_isid2e_get(unit, portcb->isid, &isid2e);
            if (BCM_FAILURE(status)) {
                MIM_ERR((_MD("failed to read isid2e[0x%06x]:%d %s\n"), 
                         portcb->isid, status, bcm_errmsg(status)));                
            }

            if (isid2e.lpi == 0) {
                MIM_ERR((_MD("unexpected physical port for logicial port on isid 0x%06x\n"),
                         portcb->isid));
                status = BCM_E_INTERNAL;
            }

            if (BCM_SUCCESS(status)) {
                lpi = isid2e.lpi;
            }

        } else if (portcb->type == _BCM_FE2K_MIM_ACCESS_PORT) {
            lpi = portcb->hwinfo.lport;
        } else {
            MIM_ERR(("Error %s Works only for Access and Default Backbone MiM "
                     "ports !!!!(%s,%d)\n",
                     FUNCTION_NAME(),__FILE__,__LINE__));
            
            status = BCM_E_PARAM;
        }
    }
        
    if (BCM_SUCCESS(status)) {
        soc_sbx_g2p3_lp_t lp;
        status = soc_sbx_g2p3_lp_get(unit, lpi, &lp);
        if(BCM_SUCCESS(status)) {
            /* set new policer id */
            status = _bcm_fe2000_g2p3_policer_lp_program(unit, pol_id, &lp);
            if(BCM_FAILURE(status)) {
                MIM_ERR(("ERROR: Setting Policer to LPORT: unit %d gport %08X LpIndex %d\n",
                         unit, portcb->gport, lpi));
            } else {
                status = soc_sbx_g2p3_lp_set(unit, lpi, &lp);
                if(BCM_FAILURE(status)) {
                    MIM_ERR(("ERROR: Setting LPORT: unit %d gport %08X LpIndex %d\n",
                             unit, portcb->gport, lpi));
                } else {
                    MIM_VVERB(("Setting LPORT: unit %d gport %08X LpIndex %d PolicerId: 0x%x\n",
                               unit, portcb->gport, lpi, pol_id));                            
                }                           
            }
        }  else {
            MIM_ERR(("Error[%d] %s Reading Logical Port !!!!(%s,%d)\n",\
                     status, FUNCTION_NAME(),__FILE__,__LINE__));
        }
    }
    
    _MIM_UNLOCK(unit);
    return status;
}


/*
 * Function:
 *   _bcm_fe2000_mim_vpn_policer_get
 * Purpose:
 *    Get Policer ID for a VPN
 *
 * Parameters:
 *     unit  - (IN)     fe unit to initialize
 * Returns:
 *   BCM_E_XX
 */
int
_bcm_fe2000_mim_policer_get(int unit, 
                                bcm_gport_t port,
                               bcm_policer_t *pol_id)
{
    int                          status = BCM_E_NONE;
    uint16 lport;
    bcm_fe2k_mim_port_control_t *portcb = NULL;
    uint32 portid = BCM_GPORT_MIM_PORT_ID_GET(port);
    int lpi = -1;

    /* sanity checks */
    G2P3_ONLY_SUPPORT_CHECK(unit);

    if(_MIM_UNIT_INVALID(unit) || (!pol_id)){
        return BCM_E_PARAM;
    }

    /* verify the gport id */
    G2P3_GPORT_RANGE_CHECK(unit, port);

    BCM_IF_ERROR_RETURN(_MIM_LOCK(unit));

    /* obtain logical port from mim gport */
    lport = gportinfo[unit].lpid[portid];

    if(BCM_GPORT_MIM_PORT != SBX_LPORT_TYPE(unit,lport)) {
        MIM_ERR(("Error %s LP GPORT not a MiM port !!!!(%s,%d)\n",\
                 FUNCTION_NAME(),__FILE__,__LINE__));

        status = BCM_E_PARAM;

    } else {

        /* works only for access & default back bone ports !!! */
        portcb = (bcm_fe2k_mim_port_control_t*)SBX_LPORT_DATAPTR(unit, lport);

        if (portcb->type == _BCM_FE2K_MIM_DEF_BACKBONE_PORT) {
            soc_sbx_g2p3_isid2e_t isid2e;

            /* when setting the qosMap on the default backbone port, set
             * use the isid2e logical port
             */
            status = soc_sbx_g2p3_isid2e_get(unit, portcb->isid, &isid2e);
            if (BCM_FAILURE(status)) {
                MIM_ERR((_MD("failed to read isid2e[0x%06x]:%d %s\n"), 
                         portcb->isid, status, bcm_errmsg(status)));                
            }

            if (isid2e.lpi == 0) {
                MIM_ERR((_MD("unexpected physical port for logicial port on isid 0x%06x\n"),
                         portcb->isid));
                status = BCM_E_INTERNAL;
            }

            if (BCM_SUCCESS(status)) {
                lpi = isid2e.lpi;
            }

        } else if (portcb->type == _BCM_FE2K_MIM_ACCESS_PORT) {
            lpi = portcb->hwinfo.lport;
        } else {
            MIM_ERR(("Error %s Works only for Access and Default Backbone MiM "
                     "ports !!!!(%s,%d)\n",
                     FUNCTION_NAME(),__FILE__,__LINE__));
            
            status = BCM_E_PARAM;
             }
         }

    if (BCM_SUCCESS(status)) {
        soc_sbx_g2p3_lp_t lp;

        status = soc_sbx_g2p3_lp_get(unit, lpi, &lp);
        if(BCM_SUCCESS(status)) {
            /* set new policer id */
            *pol_id = lp.policer;
        }  else {
            MIM_ERR(("Error[%d] %s Reading Logical Port !!!!(%s,%d)\n",\
                     status, FUNCTION_NAME(),__FILE__,__LINE__));
         }
    }

    _MIM_UNLOCK(unit);
    return status;
}


/* Support Bundle Mode */

int
_bcm_fe2000_mim_port_vlan_vector_internal(int unit,
                                           bcm_gport_t gport,
                                           bcm_port_t *phy_port,
                                           bcm_vlan_t *vsi,
                                           uint32_t   *logicalPort)
{
    uint16 lport;
    bcm_fe2k_mim_port_control_t *portcb;
    uint32 portid = BCM_GPORT_MIM_PORT_ID_GET(gport);
    int status = BCM_E_NONE;

    /* For MiM only the Default Back Bone port can be added to the
     * multicast group. Verify and flag error if it is not */
    /* sanity checks */
    G2P3_ONLY_SUPPORT_CHECK(unit);


    if(_MIM_UNIT_INVALID(unit) || (!phy_port) || (!vsi) || (!logicalPort)) {
        return BCM_E_PARAM;
    }
    
    /* verify the gport id */
    G2P3_GPORT_RANGE_CHECK(unit, gport);
    
    BCM_IF_ERROR_RETURN(_MIM_LOCK(unit));
    
    /* obtain logical port from mim gport */
    lport = gportinfo[unit].lpid[portid];

    if(BCM_GPORT_MIM_PORT != SBX_LPORT_TYPE(unit,lport)) {
        MIM_ERR(("Error %s LP GPORT not a MiM port !!!!(%s,%d)\n",\
                 FUNCTION_NAME(),__FILE__,__LINE__));

        status = BCM_E_PARAM;
        
    } else {
        /* only for access ports !!! */
        portcb = (bcm_fe2k_mim_port_control_t*)SBX_LPORT_DATAPTR(unit, lport);

        if (portcb->type == _BCM_FE2K_MIM_ACCESS_PORT) {
           *phy_port    = SOC_GPORT_MODPORT_PORT_GET(portcb->port);
           *vsi         = portcb->isidvlan;
           *logicalPort = portcb->hwinfo.lport;
        } else {
            MIM_ERR(("Error %s only for Access MiM ports !!!!(%s,%d)\n",
                     FUNCTION_NAME(),__FILE__,__LINE__));
            
            status = BCM_E_PARAM;
        }
        
    }
    
    _MIM_UNLOCK(unit);
    return status;
}

int
_bcm_fe2000_mim_port_vlan_vector_set(int unit,
                                      bcm_gport_t gport,
                                      bcm_vlan_vector_t vlan_vec)
{
    bcm_vlan_t                   vid, vpn;
    int                          status = BCM_E_NONE;
    bcm_port_t                   phy_port;

    soc_sbx_g2p3_pv2e_t          pv2e;
    uint32                       logicalPort = ~0;

    BCM_IF_ERROR_RETURN
       (_bcm_fe2000_mim_port_vlan_vector_internal(unit,
                                                  gport,
                                                  &phy_port,
                                                  &vpn,
                                                  &logicalPort));

    for (vid = BCM_VLAN_MIN + 1; vid < BCM_VLAN_MAX; vid++) {
         /* Always need to read the entry */
         soc_sbx_g2p3_pv2e_t_init(&pv2e);

         status = soc_sbx_g2p3_pv2e_get(unit, vid, phy_port, &pv2e);
         if (BCM_FAILURE(status)) {
             break;
         }

         if (BCM_VLAN_VEC_GET(vlan_vec, vid)) {
             /* this VID is a member of the vector, set it up */
             pv2e.vlan = vpn;
             pv2e.lpi = logicalPort;
         } else if (!BCM_VLAN_VEC_GET(vlan_vec, vid) &&
                    (pv2e.vlan == vpn) &&
		    (pv2e.lpi == logicalPort)) {
             pv2e.vlan = 0;
             pv2e.lpi = 0;
         }

         status = soc_sbx_g2p3_pv2e_set(unit, vid, phy_port, &pv2e);
         if (BCM_FAILURE(status)) {
             break;
         }
    }

    return status;
}

int
bcm_fe2k_mim_frame_max_access(int unit, bcm_gport_t gport,
                              int *size, int set)
{
    int                     rv = BCM_E_NONE;
    soc_sbx_g2p3_ft_t       ft;
    soc_sbx_g2p3_oi2e_t     oh;
    soc_sbx_g2p3_eteencap_t eteencap;
    soc_sbx_g2p3_etel2_t    etel2;
    uint32                  fti, etei;

    G2P3_GPORT_RANGE_CHECK(unit, gport);

    fti = BCM_GPORT_MIM_PORT_ID_GET(gport);
    fti = _BCM_FE2000_MIM_PORTID_2_FTIDX(unit, fti);

    rv = bcm_fe2k_egr_path_get(unit, fti, &ft, &oh, &eteencap, &etel2, &etei);
    if (BCM_FAILURE(rv)) {
        MIM_ERR(("ERROR %s Failed to get Egress path from "
                 "FT[0x%08x]: %d %s\n",
                 FUNCTION_NAME(), fti, rv, bcm_errmsg(rv)));
        return rv;
    }

    if (set) {
        etel2.mtu = *size - 4;
        rv = soc_sbx_g2p3_etel2_set(unit, etei, &etel2);
    } else {
        *size = etel2.mtu + 4;
    }

    return rv;
}

int
_bcm_fe2000_mim_port_vlan_vector_get(int unit,
                                     bcm_gport_t gport,
                                     bcm_vlan_vector_t vlan_vec)
{
    bcm_vlan_t                   vid, vpn;
    int                          status = BCM_E_NONE;
    bcm_port_t                   phy_port;
    uint32                       logicalPort = 0;

    soc_sbx_g2p3_pv2e_t          pv2e;

    BCM_IF_ERROR_RETURN
       (_bcm_fe2000_mim_port_vlan_vector_internal(unit,
                                                  gport,
                                                  &phy_port,
                                                  &vpn,
                                                  &logicalPort));

    BCM_VLAN_VEC_ZERO(vlan_vec);

    for (vid = BCM_VLAN_MIN + 1; vid < BCM_VLAN_MAX; vid++) {

         soc_sbx_g2p3_pv2e_t_init(&pv2e);
         status = soc_sbx_g2p3_pv2e_get(unit, vid, phy_port, &pv2e);
         if (BCM_E_NONE == status) {
             if (pv2e.vlan == vpn) {
                 BCM_VLAN_VEC_SET(vlan_vec, vid);
             }
         }

         if (BCM_FAILURE(status)) {
             break;
         }
    }

    return status;
}

/* Trunk Important Notes:
 *
 * Only Homogeneous MiM ports are allowed on trunks.
 * i.e., If Default back bone port is created over trunk, only
 * more default back bone ports can exist. 
 * There cannot be Access & Backbone over same Trunk. It is a bad
 * configuration. Checking it on API is expensive.
 *
 * Currently MiM Supports only Back bone ports over Trunk */

/*
 * Function:
 *   _bcm_fe2000_mim_trunk_cb
 * Purpose:
 *     Call back function for Trunk Membership change
 * Returns:
 *   BCM_E_XX
 */ 
STATIC int
_bcm_fe2000_mim_trunk_cb(int unit, 
                         bcm_trunk_t tid, 
                         bcm_trunk_add_info_t *tdata,
                         void *user_data)
{
    int status = BCM_E_NONE;
    soc_sbx_g2p3_p2e_t  p2e;
    soc_sbx_g2p3_ep2e_t ep2e;
    soc_sbx_g2p3_pv2e_t pv2e;
    bcm_port_t tp[BCM_TRUNK_MAX_PORTCNT];
    int index=0, mymodid, pindex=0, idx=0, port=0;
    bcm_fe2k_mim_trunk_association_t *trunkAssoc;
 
    G2P3_ONLY_SUPPORT_CHECK(unit);

    if(_MIM_UNIT_INVALID(unit)) {
        MIM_ERR(("ERROR %s Unknown unit %d !!!!(%s,%d)\n",\
                 FUNCTION_NAME(),unit,__FILE__,__LINE__));
        return BCM_E_PARAM;
    }

    if(tid >= SBX_MAX_TRUNKS) {
        MIM_ERR(("ERROR %s Bad Trunk ID  %d !!!!(%s,%d)\n",\
                 FUNCTION_NAME(),tid,__FILE__,__LINE__));
        return BCM_E_PARAM;
    }

    if(!tdata) {
        MIM_ERR(("ERROR %s Bad Input Parameter !!!!(%s,%d)\n",\
                 FUNCTION_NAME(),__FILE__,__LINE__));
        return BCM_E_PARAM;
    }
    
    status = bcm_sbx_stk_modid_get(unit, &mymodid);
    if(BCM_FAILURE(status)) {
        MIM_ERR(("ERROR %s Could not obtain my module ID !!!!(%s,%d)\n",\
                 FUNCTION_NAME(),__FILE__,__LINE__));
        return BCM_E_INTERNAL;
    }

    if(tdata->num_ports == 0) {
        /* nothing to do */
        return BCM_E_NONE;
    }

    BCM_IF_ERROR_RETURN(_MIM_LOCK(unit));

    for(index=0; index < BCM_TRUNK_MAX_PORTCNT; index++) {
        tp[index] = -1;
    }

    for(index=0; index < tdata->num_ports; index++) {
        if(tdata->tm[index] == mymodid) {
            port = tdata->tp[index];
            idx = 0;
            /* verify if this port was already taken care due to 
             * duplicate trunk distribution */
            while(tp[idx] >= 0) {
                if(port == tp[idx]) {
                    break;
                }
                idx++;
            }

            if(port == tp[idx]) {
                continue;
            }

            tp[pindex++] = port;
        }
    }

    trunkAssoc = &mim_trunk_assoc_info[unit][tid];

    switch(trunkAssoc->mimType) {

        case _BCM_FE2K_MIM_DEF_BACKBONE_PORT:
            for(index=0; index < pindex; index++) {

                port = tp[index];

                /* configure back bone ports mim mode */
                if((port >= 0) && 
                   ((soc_sbx_g2p3_p2e_get(unit, port, &p2e)) == BCM_E_NONE) &&
                   ((soc_sbx_g2p3_ep2e_get(unit, port, &ep2e)) == BCM_E_NONE)) {

                    p2e.mim         = MIM_BBONE_PORT;
                    p2e.customer    = 0;
                    p2e.provider    = 1;
                    p2e.defstrip    = 0;
                    p2e.pstrip      = 0;
                    p2e.pbb         = 1;

                    ep2e.customer   = 1;
                    ep2e.mim        = MIM_BBONE_PORT;
                    ep2e.pbb        = 1;                

                    /* program P2E */
                    status = soc_sbx_g2p3_p2e_set(unit, port, &p2e);

                    if(BCM_SUCCESS(status)) {
                        /* program EP2E */
                        status = soc_sbx_g2p3_ep2e_set(unit, port, &ep2e);

                        if(BCM_FAILURE(status)) {

                            MIM_ERR(("ERROR %s Programming EP2E %d : %s in (%s,%d)\n",
                                     FUNCTION_NAME(), status, bcm_errmsg(status),
                                     __FILE__,__LINE__));
                        } else {
                            dq_p_t port_elem;
                            bcm_fe2k_mim_port_control_t *portcb = NULL;

                            /* For each back bone port on port list, reset lp on bvlan pv2e */
                            DQ_TRAVERSE(&trunkAssoc->plist, port_elem) {

                                port_elem -= _MIM_PCB_TRUNK_NODE_POS;
                                _BCM_FE2000_GET_PORTCB_FROM_LIST(port_elem, portcb);

                                soc_sbx_g2p3_pv2e_t_init(&pv2e);

                                status = soc_sbx_g2p3_pv2e_get(unit, portcb->vlan /*bvlan*/, port, &pv2e);
                                if(BCM_FAILURE(status)) {

                                    MIM_ERR(("ERROR %s Getting PV2E %d : %s in (%s,%d)\n",
                                             FUNCTION_NAME(), status, bcm_errmsg(status),
                                             __FILE__,__LINE__));
                                } else {
                                    /* Program Bvlan, port 2 Etc -> LPI */
                                    /* lpi on pv2e for back bone must be 0 to use physical port as lp */
                                    pv2e.lpi = 0;

                                    status =  soc_sbx_g2p3_pv2e_set(unit, portcb->vlan /*bvlan*/, port, &pv2e);
                                    if(BCM_FAILURE(status)) {

                                        MIM_ERR(("ERROR %s Programming PV2E %d : %s in (%s,%d)\n",
                                                 FUNCTION_NAME(), status, bcm_errmsg(status),
                                                 __FILE__,__LINE__));
                                    }
                                }
                            } DQ_TRAVERSE_END(&trunkAssoc->plist, port_elem);
                        }
                    }

                } else {
                    status = BCM_E_INTERNAL;
                }
            }
            break;

        default:
            status = BCM_E_INTERNAL;
            break;
    }

    _MIM_UNLOCK(unit);

    return status;
}

#else
#endif /* INCLUDE_L3 */
