/*
 * $Id: allocator.c 1.72 Broadcom SDK $
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
 * Global resource allocator
 */

#include <soc/defs.h>

#include <soc/drv.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/fe2000.h>

#include <soc/sbx/qe2000_util.h>

#ifdef BCM_FE2000_P3_SUPPORT
#include <soc/sbx/g2p3/g2p3.h>
#endif

#include <bcm_int/sbx/error.h>

#include <bcm/types.h>
#include <bcm/error.h>
#include <bcm/debug.h>
#include <shared/idxres_afl.h>
#include <shared/idxres_fl.h>
#include <shared/hash_tbl.h>
#include <bcm_int/sbx/fe2000/allocator.h>
#include <bcm_int/sbx/stat.h>

#define ALCR_DEBUG(flags, stuff)  BCM_DEBUG(flags | BCM_DBG_API, stuff)
#define ALCR_OUT(stuff)           ALCR_DEBUG(BCM_DBG_API, stuff)
#define ALCR_WARN(stuff)          ALCR_DEBUG(BCM_DBG_WARN, stuff)
#define ALCR_ERR(stuff)           ALCR_DEBUG(BCM_DBG_ERR, stuff)
#define ALCR_VERB(stuff)          ALCR_DEBUG(BCM_DBG_VERBOSE, stuff)
#define ALCR_VVERB(stuff)         ALCR_DEBUG(BCM_DBG_VVERBOSE, stuff)

#define MAC_FMT       "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x"
#define MAC_PFMT(mac) (mac)[0], (mac)[1], (mac)[2], \
                      (mac)[3], (mac)[4], (mac)[5]

/*
 *  These are for tracking logical ports.  They are necessary because g2p2 does
 *  have any inherent lport features, and because g2p3 does not store anything
 *  useful for configuring them (such as the match criteria) in its view of the
 *  lport entity (it's all actions for the forwarder).
 *
 *  _sbx_lport[unit]->[lport] is a pointer to the original structure on which
 *  the logical port is based.
 *
 *  _sbx_lport[unit]->[lport] is an indicator of the GPORT_TYPE for the
 *  corresponding lport (so they can be searched).
 */
_sbx_lportList *_sbx_lport[BCM_MAX_NUM_UNITS];
_sbx_lpTypeList *_sbx_lptype[BCM_MAX_NUM_UNITS];
uint8 *_sbx_global_ftType[BCM_MAX_NUM_UNITS];
uint8 *_sbx_global_vsiType[BCM_MAX_NUM_UNITS];

shr_htb_hash_table_t _sbx_mpls_lport[BCM_MAX_NUM_UNITS];

typedef enum {
    SBX_GU2K_HW_RES_ETE_TYPE_ETHERNET,
    SBX_GU2K_HW_RES_ETE_TYPE_ENCAP,
    SBX_GU2K_HW_RES_ETE_TYPE_MAX
} _sbx_gu2_hw_res_ete_t;

#define _SBX_FTE_BLOCKING_FACTOR       5 /* XXX (2^5) derive from max mpaths   */
#define _SBX_VSI_BLOCKING_FACTOR       3 /* XXX (2^3) derive from 8 cosq level */


/* Index is _sbx_gu2_usr_res_types_t and returns
 * _sbx_gu2_hw_res_t
 */
_sbx_gu2_hw_res_t    _g_mapped_hw_res[] = {
    SBX_GU2K_HW_RES_FTE,       /* FTE_L3         */
    SBX_GU2K_HW_RES_FTE,       /* FTE_L3_MPATH   */
    SBX_GU2K_HW_RES_FTE,       /* FTE_IPMC       */
    SBX_GU2K_HW_RES_GLOBAL_GPORT_FTE,/* FTE_GLOBAL_GPORT*/
    SBX_GU2K_HW_RES_LOCAL_GPORT_FTE, /* FTE_LOCAL_GPORT*/
    SBX_GU2K_HW_RES_GLOBAL_GPORT_FTE,/* FTE_L2MC       */
    SBX_GU2K_HW_RES_FTE,       /* FTE_MPLS       */
    SBX_GU2K_HW_RES_FTE,       /* FTE_UNICAST    */
    SBX_GU2K_HW_RES_OHI,       /* OHI            */
    SBX_GU2K_HW_RES_ETE,       /* ETE_l2         */
    SBX_GU2K_HW_RES_ETE,       /* ETE_ENCAP      */
    SBX_GU2K_HW_RES_IFID,      /* IFID           */
    SBX_GU2K_HW_RES_VRF,       /* VRF            */
    SBX_GU2K_HW_RES_VSI,       /* VSI            */
    SBX_GU2K_HW_RES_LINE_VSI,        /* For VPWS and other forms of wires */
    SBX_GU2K_HW_RES_QOS_PROFILE, /* QoS Profiles*/
    SBX_GU2K_HW_RES_QOS_EGR_REMARK, /* QoS Egress Remark */
    SBX_GU2K_HW_RES_LPORT,
    SBX_GU2K_HW_RES_MPLS_LPORT,
    SBX_GU2K_HW_RES_VPLS_COLOR,
    SBX_GU2K_HW_RES_PROTECTION, /* failover      */
    SBX_GU2K_HW_RES_VPWS_UNI_LPORT,
    SBX_GU2K_HW_RES_VPWS_UNI_GPORT_FTE,       /* FTE_MPLS       */
};

#define _SBX_GU2_HW_TABLE_MASK(x)  ( 1 << (x) )

_sbx_gu2_hw_table_attrs_t _g_sbx_gu2_table_attrs[SBX_GU2K_TABLE_MAX] = {
    {_SBX_GU2_HW_TABLE_MASK(SBX_GU2K_TABLE_NONE),        "NO_TABLE"},
    {_SBX_GU2_HW_TABLE_MASK(SBX_GU2K_TABLE_FTE),         "FTE"},
    {_SBX_GU2_HW_TABLE_MASK(SBX_GU2K_TABLE_VSI),         "VSI"},
    {_SBX_GU2_HW_TABLE_MASK(SBX_GU2K_TABLE_OHI),         "OHI"},
    {_SBX_GU2_HW_TABLE_MASK(SBX_GU2K_TABLE_ETE),         "ETE"},
    {_SBX_GU2_HW_TABLE_MASK(SBX_GU2K_TABLE_VRF),         "VRF"},
    {_SBX_GU2_HW_TABLE_MASK(SBX_GU2K_TABLE_QPROFILE),    "QOSPROFILE"},
    {_SBX_GU2_HW_TABLE_MASK(SBX_GU2K_TABLE_QMAP),        "QOSMAP"},
    {_SBX_GU2_HW_TABLE_MASK(SBX_GU2K_TABLE_QMAP_EGR),    "EGRESS QOSMAP"},
    {_SBX_GU2_HW_TABLE_MASK(SBX_GU2K_TABLE_LPORT),       "LOGICAL PORT"},
    {_SBX_GU2_HW_TABLE_MASK(SBX_GU2K_TABLE_PROT),        "PROTECTION"},
    {_SBX_GU2_HW_TABLE_MASK(SBX_GU2K_TABLE_IFID),        "IFID"}
};

#define SBX_GU2K_ALLOC_STYLE_NONE           0
#define SBX_GU2K_ALLOC_STYLE_SINGLE         1
#define SBX_GU2K_ALLOC_STYLE_SCALED         2
#define SBX_GU2K_ALLOC_STYLE_VERSATILE      3

#define SBX_GU2K_DYNAMIC_BASE_ADDR       ~0
#define SBX_GU2K_DYNAMIC_MAX_COUNT       ~0
#define SBX_GU2K_DYNAMIC_SIZE            ~0

#define SBX_GU2K_HW_RES_IFID_BASE        (_SBX_GU2_MIN_VALID_IFID)

#define SBX_GU2K_HW_RES_IFID_COUNT       (_SBX_GU2_MAX_VALID_IFID) - \
                                         (_SBX_GU2_MIN_VALID_IFID)


#define G2P3_DROP_FTE_EXCEPTION         0x7e

typedef struct {
    uint32                          alloc_style;
    union {
        shr_aidxres_list_handle_t   handle_aidx;
        shr_idxres_list_handle_t    handle_idx;
    } u;
} _sbx_gu2_res_handle_t;

#define aidx_handle u.handle_aidx
#define idx_handle  u.handle_idx


typedef struct _sbx_track_mac_data_s {
    bcm_mac_t   mac;
    bcm_port_t  port;
    uint32      flags;
    uint32_t    refCount;
} _sbx_track_mac_data_t;

typedef struct _sbx_mac_tracker_s {
    uint32_t                 numMacs;
    _sbx_track_mac_data_t   *macData;
} _sbx_mac_tracker_t;

typedef struct _sbx_smac_idx_tracker_s {
    _sbx_mac_tracker_t ismac;   /* ingress smac idx tracker */
    _sbx_mac_tracker_t esmac;   /* egress smac idx tracker */
} _sbx_smac_idx_tracker_t;

static int _sbx_mac_tracker_init(_sbx_mac_tracker_t *tracker, uint32 numMacs);
static int _sbx_mac_tracker_destroy(_sbx_mac_tracker_t *tracker);
static int _sbx_mac_tracker_alloc(_sbx_mac_tracker_t *tracker, int flags,
                                  bcm_mac_t mac, bcm_port_t  port, uint32* idx);
static int _sbx_mac_tracker_free(_sbx_mac_tracker_t *tracker, 
                                 bcm_mac_t mac, bcm_port_t  port, uint32* idx);
static int _sbx_mac_tracker_set(_sbx_mac_tracker_t *tracker, bcm_mac_t mac,
                                uint32 idx);
static int _sbx_mac_tracker_find(_sbx_mac_tracker_t *tracker, bcm_mac_t mac, 
                                 uint32 *idx);

static _sbx_smac_idx_tracker_t _sbx_smac_tracker[BCM_MAX_NUM_UNITS];

static uint32 ete_e2i(int unit, uint32 e);
static uint32 ete_i2e(int unit, uint32 i);
static uint32 fte_e2i(int unit, uint32 e);
static uint32 fte_i2e(int unit, uint32 i);


_sbx_gu2_hw_res_attrs_t _g_sbx_gu2_res_attrs[SBX_GU2K_HW_RES_MAX] = {
    /* SBX_GU2K_HW_RES_FTE */
    {SBX_GU2K_ALLOC_STYLE_VERSATILE,
     SBX_GU2K_DYNAMIC_BASE_ADDR,
     SBX_GU2K_DYNAMIC_MAX_COUNT,
     _SBX_FTE_BLOCKING_FACTOR,
     0,
     "FTE",
     _SBX_GU2_HW_TABLE_MASK(SBX_GU2K_TABLE_FTE),
     0, 0,
     fte_e2i, fte_i2e
    },
    /* SBX_GU2K_HW_RES_GLOBAL_GPORT_FTE */
    {SBX_GU2K_ALLOC_STYLE_VERSATILE,
     SBX_GU2K_DYNAMIC_BASE_ADDR,
     SBX_GU2K_DYNAMIC_MAX_COUNT,
     1,
     0,
     "GGPORT-FTE",
     _SBX_GU2_HW_TABLE_MASK(SBX_GU2K_TABLE_FTE),
     0, 0,
     NULL, NULL
    },
    /* SBX_GU2K_HW_RES_LOCAL_GPORT_FTE */
    {SBX_GU2K_ALLOC_STYLE_VERSATILE,
     SBX_GU2K_DYNAMIC_BASE_ADDR,
     SBX_GU2K_DYNAMIC_MAX_COUNT,
     1,
     0,
     "LGPORT-FTE",
     _SBX_GU2_HW_TABLE_MASK(SBX_GU2K_TABLE_FTE),
     0, 0,
     NULL, NULL
    },
    /* SBX_GU2K_HW_RES_OHI */
    {SBX_GU2K_ALLOC_STYLE_SINGLE,
     SBX_GU2K_DYNAMIC_BASE_ADDR,
     SBX_GU2K_DYNAMIC_MAX_COUNT,
     1,
     0,
     "OHI",
     _SBX_GU2_HW_TABLE_MASK(SBX_GU2K_TABLE_OHI),
     0, 0,
     NULL, NULL
    },
    /* SBX_GU2K_HW_RES_ETE */
    {SBX_GU2K_ALLOC_STYLE_SCALED,
     SBX_GU2K_DYNAMIC_BASE_ADDR,
     SBX_GU2K_DYNAMIC_MAX_COUNT,
     1,
     SBX_GU2K_DYNAMIC_SIZE,
     "ETE",
     _SBX_GU2_HW_TABLE_MASK(SBX_GU2K_TABLE_ETE),
     0, 0,
     ete_e2i, ete_i2e
    },
    /* SBX_GU2K_HW_RES_IFID */
    {SBX_GU2K_ALLOC_STYLE_SINGLE,
     SBX_GU2K_HW_RES_IFID_BASE,
     SBX_GU2K_HW_RES_IFID_COUNT,
     1,
     0,
     "IFID",
     _SBX_GU2_HW_TABLE_MASK(SBX_GU2K_TABLE_IFID),
     0, 0,
     NULL, NULL
    },
    /* SBX_GU2K_HW_RES_VRF */
    {SBX_GU2K_ALLOC_STYLE_SINGLE,
     SBX_VRF_BASE,
     SBX_VRF_END - SBX_VRF_BASE + 1,
     1,
     0,
     "VRF",
     _SBX_GU2_HW_TABLE_MASK(SBX_GU2K_TABLE_VRF),
     0, 0,
     NULL, NULL
    },
    /* SBX_GU2K_HW_RES_VSI */
    {SBX_GU2K_ALLOC_STYLE_VERSATILE,
     SBX_GU2K_DYNAMIC_BASE_ADDR,
     SBX_GU2K_DYNAMIC_MAX_COUNT,
     _SBX_VSI_BLOCKING_FACTOR,
     0,
     "VSI",
     _SBX_GU2_HW_TABLE_MASK(SBX_GU2K_TABLE_VSI),
     0, 0,
     NULL, NULL
    },
    /* SBX_GU2K_HW_RES_LINE_VSI */
    {SBX_GU2K_ALLOC_STYLE_VERSATILE,
     SBX_GU2K_DYNAMIC_BASE_ADDR,
     SBX_GU2K_DYNAMIC_MAX_COUNT,
     _SBX_VSI_BLOCKING_FACTOR,
     0,
     "LINE-VSI",
     _SBX_GU2_HW_TABLE_MASK(SBX_GU2K_TABLE_NONE), /* not really a hw table */
     0, 0,
     NULL, NULL
    },
   /* SBX_GU2K_HW_RES_QOS_PROFILE */
    {SBX_GU2K_ALLOC_STYLE_SINGLE,
     SBX_QOS_PROFILE_BASE,
     SBX_GU2K_DYNAMIC_MAX_COUNT,
     1,
     0,
     "QoS-Profile",
     _SBX_GU2_HW_TABLE_MASK(SBX_GU2K_TABLE_QPROFILE),
     0, 0,
     NULL, NULL
    },
    /* SBX_GU2K_HW_RES_QOS_EGR_REMARK */
    {SBX_GU2K_ALLOC_STYLE_SINGLE,
     SBX_QOS_EGR_REMARK_BASE,
     SBX_GU2K_DYNAMIC_MAX_COUNT,
     1,
     0,
     "QoS-Egress-remark",
     _SBX_GU2_HW_TABLE_MASK(SBX_GU2K_TABLE_QMAP_EGR),
     0, 0,
     NULL, NULL
    },
    /* SBX_GU2K_HW_RES_LPORT */
    {SBX_GU2K_ALLOC_STYLE_SINGLE,
     SBX_LPORT_BASE,
     SBX_LPORT_END - SBX_LPORT_BASE + 1,
     1,
     0,
     "Logical-Port",
     _SBX_GU2_HW_TABLE_MASK(SBX_GU2K_TABLE_LPORT),
     0, 0,
     NULL, NULL
    },
     /* SBX_GU2K_HW_RES_MPLS_LPORT */
    {SBX_GU2K_ALLOC_STYLE_SINGLE,
     SBX_MPLS_LPORT_BASE,
     SBX_MPLS_LPORT_END - SBX_MPLS_LPORT_BASE + 1,
     1,
     0,
     "MPLS-Logical-Port",
     _SBX_GU2_HW_TABLE_MASK(SBX_GU2K_TABLE_LPORT),
     0, 0,
     NULL, NULL
    },
     /* SBX_GU2K_HW_RES_VPLS_COLOR */
    {SBX_GU2K_ALLOC_STYLE_SINGLE,
     SBX_VPLS_COLOR_BASE,
     SBX_VPLS_COLOR_END - SBX_VPLS_COLOR_BASE + 1,
     1,
     0,
     "VPLS-Color",
     _SBX_GU2_HW_TABLE_MASK(SBX_GU2K_TABLE_NONE),
     0, 0,
     NULL, NULL
    },

   /* SBX_GU2K_HW_PROTECTION */
    {SBX_GU2K_ALLOC_STYLE_SINGLE,
     SBX_PROTECTION_BASE,
     SBX_PROTECTION_END - SBX_PROTECTION_BASE + 1,
     1,
     0,
     "Protection table",
     _SBX_GU2_HW_TABLE_MASK(SBX_GU2K_TABLE_PROT),
     0, 0,
     NULL, NULL
    },
    /* SBX_GU2K_HW_RES_VPWS_UNI_LPORT */
    /* Used only by C2, mplstp images */
    {SBX_GU2K_ALLOC_STYLE_SINGLE,
     SBX_MPLS_LPORT_BASE,
     SBX_MPLS_LPORT_BASE + 1,
     1,
     0,
     "VPWS UNI Logical-Port",
     _SBX_GU2_HW_TABLE_MASK(SBX_GU2K_TABLE_LPORT),
     0, 0,
     NULL, NULL
    },
    /* SBX_GU2K_HW_RES_VPWS_UNI_GPORT_FTE */
    {SBX_GU2K_ALLOC_STYLE_VERSATILE,
     SBX_GU2K_DYNAMIC_BASE_ADDR,
     SBX_GU2K_DYNAMIC_MAX_COUNT,
     1,
     0,
     "VPWS-UNI-FTE",
     _SBX_GU2_HW_TABLE_MASK(SBX_GU2K_TABLE_FTE),
     0, 0,
     NULL, NULL
    }
};

/*
 * one stucture per unit
 */
_sbx_gu2_res_handle_t
_g_sbx_gu2_res_handles[BCM_LOCAL_UNITS_MAX][SBX_GU2K_HW_RES_MAX];


/*
 * LOCK related defines
 */
sal_mutex_t  _sbx_gu2_resource_mlock[BCM_MAX_NUM_UNITS];

#define _SBX_GU2_RESOURCE_LOCK_CREATED_ALREADY(unit) \
    (_sbx_gu2_resource_mlock[(unit)] != NULL)

#define _SBX_GU2_RESOURCE_LOCK(unit)                    \
    (_SBX_GU2_RESOURCE_LOCK_CREATED_ALREADY(unit) ?      \
     sal_mutex_take(_sbx_gu2_resource_mlock[(unit)], sal_mutex_FOREVER)?BCM_E_UNIT:BCM_E_NONE: \
     BCM_E_UNIT)

#define _SBX_GU2_RESOURCE_UNLOCK(unit)          \
    sal_mutex_give(_sbx_gu2_resource_mlock[(unit)])


/*  ETE translators */
static uint32 ete_e2i(int unit, uint32 e)
{
    return e + _g_sbx_gu2_res_attrs[SBX_GU2K_HW_RES_ETE].base;
}
static uint32 ete_i2e(int unit, uint32 i)
{
    return i - _g_sbx_gu2_res_attrs[SBX_GU2K_HW_RES_ETE].base;
}

/*  FTE translators */
static uint32 fte_e2i(int unit, uint32 e) 
{
    if (e > SBX_DYNAMIC_FTE_END(unit)) {
        if (e < (SBX_EXTRA_FTE_END(unit) - SBX_EXTRA_FTE_BASE(unit) + \
                 SBX_DYNAMIC_FTE_END(unit) - SBX_EXTRA_FTE_BASE(unit))) {
            /* force an invalid entry */
            return SBX_EXTRA_FTE_END(unit) + 1;
        }
        return e - (SBX_EXTRA_FTE_BASE(unit) - SBX_DYNAMIC_FTE_END(unit) - 1);
    } 
    return e;
}
static uint32 fte_i2e(int unit, uint32 i)
{
    if (i >  SBX_DYNAMIC_FTE_END(unit)) {
        return i + (SBX_EXTRA_FTE_BASE(unit) - SBX_DYNAMIC_FTE_END(unit) - 1);
    } 
    return i;
}

/*
 *   Function
 *      _sbx_gu2_get_resource_handle
 *   Purpose
 *      Get the internal resource handle based on unit and type.
 *   Parameters
 *      unit    - (IN)  unit number of the device
 *      type    - (IN)  resource type
 *                      (one of _sbx_gu2_usr_res_types_t)
 *      handle  - (OUT) resource allocation handle
 *   Returns
 *     BCM_E_NONE   if got the handle
 *     BCM_E_*      as appropriate otherwise
 */

STATIC int
_sbx_gu2_get_resource_handle(int                        unit,
                             _sbx_gu2_usr_res_types_t   type,
                             _sbx_gu2_res_handle_t     *handle)
{
    _sbx_gu2_hw_res_t         hw_res;
    _sbx_gu2_res_handle_t     *res_handle;

    if (!SOC_IS_SBX_FE2000(unit)) {
        return BCM_E_UNIT;
    }

    if ((type >= SBX_GU2K_USR_RES_MAX) || (handle == NULL)) {
        return BCM_E_PARAM;
    }
    hw_res = _g_mapped_hw_res[type];

    res_handle = &_g_sbx_gu2_res_handles[unit][hw_res];
    if (res_handle->alloc_style == SBX_GU2K_ALLOC_STYLE_NONE) {
        return BCM_E_PARAM;
    }

    *handle = *res_handle;
    return BCM_E_NONE;
}

/*
 *   Function
 *      _sbx_g2p3_resource_ete_params_get
 *   Purpose
 *      Get the various ucode memory, size for ete space for the
 *      g2p3 application
 *   Parameters
 *      unit          : unit number of the device
 *      base          : ete base address (OUT)
 *      ete_area_size : ete area in ucode memory (OUT)
 *      ete_max_size  : largest ete size (OUT)
 *      array_max     : number of slots in array (IN)
 *      ete_size_array: array to get size for each type of ete (OUT)
 *   Returns
 *       BCM_E_XXX
 *   Notes
 */
#ifdef BCM_FE2000_P3_SUPPORT
STATIC int
_sbx_g2p3_resource_ete_params_get(int unit,
                                 int *base,
                                  int *ete_area_size,
                                  int *ete_max_size,
                                  int  array_max,
                                  int *ete_size_array)
{
    _sbx_gu2_hw_res_ete_t   etype;

    if ((ete_size_array == NULL) ||
        (array_max < SBX_GU2K_HW_RES_ETE_TYPE_MAX)) {
        return BCM_E_PARAM;
    }

    /* all g2p3 resources start at 0 */
    *base = 0;
    *ete_area_size = soc_sbx_g2p3_ete_table_size_get(unit);

    /**
     * Setup ETE max and base values
     */
    *ete_max_size = 0;
    for (etype = SBX_GU2K_HW_RES_ETE_TYPE_ETHERNET;
         etype < SBX_GU2K_HW_RES_ETE_TYPE_MAX; etype++) {

        switch(etype) {
        case SBX_GU2K_HW_RES_ETE_TYPE_ETHERNET:
            ete_size_array[etype] = soc_sbx_g2p3_etel2_size(unit);
            break;
        case SBX_GU2K_HW_RES_ETE_TYPE_ENCAP:
            ete_size_array[etype] = soc_sbx_g2p3_eteencap_size(unit);
            break;
        default:
            return BCM_E_INTERNAL;
        };

        if (ete_size_array[etype] > *ete_max_size) {
            *ete_max_size = ete_size_array[etype];
        }
    }

    return BCM_E_NONE;
}
#endif /* def BCM_FE2000_P3_SUPPORT */

/*
 *   Function
 *      _sbx_gu2_resource_ete_params_get
 *   Purpose
 *      Get the various ucode memory, size for ete space
 *   Parameters
 *      unit          : unit number of the device
 *      base          : ete base address (OUT)
 *      ete_area_size : ete area in ucode memory (OUT)
 *      ete_max_size  : largest ete size (OUT)
 *      array_max     : number of slots in array (IN)
 *      ete_size_array: array to get size for each type of ete (OUT)
 *   Returns
 *       BCM_E_XXX
 *   Notes
 */

STATIC int
_sbx_gu2_resource_ete_params_get(int unit,
                                 int *base,
                                 int *ete_area_size,
                                 int *ete_max_size,
                                 int  array_max,
                                 int *ete_size_array)
{
    if (!SOC_IS_SBX_FE2000(unit)) {
        return BCM_E_UNIT;
    }

#ifdef BCM_FE2000_P3_SUPPORT
    if (SOC_IS_SBX_G2P3(unit)) {
        return _sbx_g2p3_resource_ete_params_get(unit, base, ete_area_size,
                                                 ete_max_size, array_max,
                                                 ete_size_array);
    } else
#endif /* def BCM_FE2000_P3_SUPPORT */
    {
        return BCM_E_UNAVAIL;
    }
}

int
_sbx_gu2_fte_res_attrs_set(int unit, _sbx_gu2_hw_res_attrs_t *fte_res_attrs)
{
#ifdef BCM_FE2000_P3_SUPPORT
    if (SOC_IS_SBX_G2P3(unit)) {

        fte_res_attrs->base      = SBX_DYNAMIC_FTE_BASE(unit);
        fte_res_attrs->max_count = (SBX_DYNAMIC_FTE_END(unit) - 
                                    SBX_DYNAMIC_FTE_BASE(unit) + 1);

        fte_res_attrs->max_count += (SBX_EXTRA_FTE_END(unit) - 
                                     SBX_EXTRA_FTE_BASE(unit) + 1);

        return BCM_E_NONE;
    } else
#endif /* def BCM_FE2000_P3_SUPPORT */
    {
        return BCM_E_UNAVAIL;
    }
}

int
_sbx_gu2_local_gport_fte_res_attrs_set(int unit,
                                       _sbx_gu2_hw_res_attrs_t *res_attrs)
{
    switch(SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        res_attrs->base      = SBX_LOCAL_GPORT_FTE_BASE(unit);
        res_attrs->max_count = (SBX_LOCAL_GPORT_FTE_END(unit) -
                                SBX_LOCAL_GPORT_FTE_BASE(unit) + 1);
        
        break;
#endif
    default:
        return BCM_E_UNAVAIL;
    }
    return BCM_E_NONE;
}

int
_sbx_gu2_global_gport_fte_res_attrs_set(int unit,
                                        _sbx_gu2_hw_res_attrs_t *res_attrs)
{
    switch(SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        res_attrs->base      = SBX_GLOBAL_GPORT_FTE_BASE(unit);
        res_attrs->max_count = (SBX_GLOBAL_GPORT_FTE_END(unit) -
                                SBX_GLOBAL_GPORT_FTE_BASE(unit) + 1);
        break;
#endif
    default:
        return BCM_E_UNAVAIL;
    }
    return BCM_E_NONE;
}


int
_sbx_gu2_vsi_res_attrs_set(int unit,
                           _sbx_gu2_hw_res_attrs_t *res_attrs)
{
    /* dynamic VSI values; first 4K are reserved for vlan vids, remainder
     * available to the application
     */

    switch(SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        if (SOC_IS_SBX_G2P3(unit) && SOC_SBX_CFG(unit)->mplstp_ena) {
            res_attrs->base      = SBX_MAX_VID+1;
        } else {
            res_attrs->base      = SBX_MAX_VID;
        }
        res_attrs->max_count = (soc_sbx_g2p3_v2e_table_size_get(unit) - 
                                res_attrs->base);
        break;
#endif
    default:
        return BCM_E_UNAVAIL;
    }
    return BCM_E_NONE;
}

int 
_sbx_gu2_line_vsi_res_attrs_set(int unit, 
                                _sbx_gu2_hw_res_attrs_t *res_attrs)
{

    /* Must be numerically different than true VSIs */
    switch(SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
    {
        uint32 vsiBase = _g_sbx_gu2_res_attrs[SBX_GU2K_HW_RES_VSI].base;
        uint32 vsiCount = _g_sbx_gu2_res_attrs[SBX_GU2K_HW_RES_VSI].max_count;
        
        if (vsiBase == SBX_GU2K_DYNAMIC_BASE_ADDR  ||
            vsiCount == SBX_GU2K_DYNAMIC_MAX_COUNT) {
            /* SBX_GU2K_HW_RES_VSI must be configured first */
            return BCM_E_INTERNAL;
        }

        /* Each LINE VSI requires two VSIs therefore the max number of LINE
         * VSIs supprtoed is half the number of Dynamic VSIs 
         * (Total VSI - VID VSIs)
         */
        res_attrs->base      = vsiBase + vsiCount;
        res_attrs->max_count = 
            (soc_sbx_g2p3_v2e_table_size_get(unit) - vsiBase)>> 1;
    }
    break;
#endif
    default:
        return BCM_E_UNAVAIL;
    }
    return BCM_E_NONE;
}

int
_sbx_gu2_oh_res_attrs_set(int unit, _sbx_gu2_hw_res_attrs_t *oh_res_attrs)
{
#ifdef BCM_FE2000_P3_SUPPORT
    if (SOC_IS_SBX_G2P3(unit)) {
        /* all g2p3 resources start at index 0 */
        oh_res_attrs->base      = SBX_TRUNK_OHI_END + 1;
        oh_res_attrs->max_count = (soc_sbx_g2p3_oi2e_table_size_get(unit) -
                                   oh_res_attrs->base + SBX_RAW_OHI_BASE);
        return BCM_E_NONE;
    } else
#endif /* def BCM_FE2000_P3_SUPPORT */
    {
        return BCM_E_UNAVAIL;
    }
}


static int
_sbx_gu2_mplstp_vpws_uni_res_attrs_set(int unit)
{
   _sbx_gu2_hw_res_attrs_t    *res_attrs;
   int status = BCM_E_NONE;
 
#ifdef BCM_FE2000_P3_SUPPORT

    if (SOC_IS_SBX_G2P3(unit) && SOC_SBX_CFG(unit)->mplstp_ena) {

        /* Set MPLS Lport resource attributes */
        /* +---------------+  - 0
         * + [bridging lp] +  
         * +---------------+ 16K-1 
         * +---------------+ 16K
         * + [vpws uni lp] +
         * +---------------+ 32K-1 
         * +---------------+ 32K
         * + [mpls lp]     +
         * +---------------+ 96K-1 
         * 
         * #Index notes to address the tables:
         * Bridging LP - 0 based index 
         * VPWS UNI LP - 0 based index
         * MPLS LP     - Index starts from 32K - 96-1K
         */

        res_attrs = &_g_sbx_gu2_res_attrs[SBX_GU2K_HW_RES_VPWS_UNI_LPORT];
        /* all g2p3 resources start at index 0 */
        res_attrs->base      = SBX_LPORT_END+1;
        res_attrs->max_count = SBX_LPORT_END + 1;

        res_attrs = &_g_sbx_gu2_res_attrs[SBX_GU2K_HW_RES_MPLS_LPORT];
        res_attrs->base       = 2 * (SBX_LPORT_END + 1);
        res_attrs->max_count  = SBX_FE2K_NUM_MPLS_LBLS;

        res_attrs = &_g_sbx_gu2_res_attrs[SBX_GU2K_HW_RES_VPWS_UNI_GPORT_FTE];
        res_attrs->base       = SBX_VPWS_UNI_FTE_BASE(unit);
        res_attrs->max_count  = SBX_VPWS_UNI_FTE_END(unit) - SBX_VPWS_UNI_FTE_BASE(unit) - 1;

        /* set ucode constant */
        soc_sbx_g2p3_vpws_ft_offset_set(unit, SBX_VPWS_UNI_FTE_BASE(unit));
    } else
#endif /* def BCM_FE2000_P3_SUPPORT */
    {
        /* assign dummy values, this pool is not used by non-mplstp */
        res_attrs = &_g_sbx_gu2_res_attrs[SBX_GU2K_HW_RES_VPWS_UNI_GPORT_FTE];
        res_attrs->base      = SBX_VPWS_UNI_FTE_BASE(unit);
        res_attrs->max_count = res_attrs->base+1;
    }
    return status;
}

/*
 *   Function
 *      _sbx_gu2_resource_to_str
 *   Purpose
 *      DEBUG: returns a string representing the user resource types
 *   Parameters
 *      type    - (IN)  resource type
 *   Returns
 *      pointer to string of resource name
 */
#ifdef BROADCOM_DEBUG
extern const char*
_sbx_gu2_resource_to_str(_sbx_gu2_usr_res_types_t type)
{
    switch (type) 
    {
    case SBX_GU2K_USR_RES_FTE_L3: return "SBX_GU2K_USR_RES_FTE_L3";
    case SBX_GU2K_USR_RES_FTE_L3_MPATH: return "SBX_GU2K_USR_RES_FTE_L3_MPATH";
    case SBX_GU2K_USR_RES_FTE_IPMC: return "SBX_GU2K_USR_RES_FTE_IPMC";
    case SBX_GU2K_USR_RES_FTE_GLOBAL_GPORT: return "SBX_GU2K_USR_RES_FTE_GLOBAL_GPORT";
    case SBX_GU2K_USR_RES_FTE_LOCAL_GPORT: return "SBX_GU2K_USR_RES_FTE_LOCAL_GPORT";
    case SBX_GU2K_USR_RES_FTE_L2MC: return "SBX_GU2K_USR_RES_FTE_L2MC";
    case SBX_GU2K_USR_RES_FTE_MPLS: return "SBX_GU2K_USR_RES_FTE_MPLS";
    case SBX_GU2K_USR_RES_FTE_UNICAST: return "SBX_GU2K_USR_RES_FTE_UNICAST";
    case SBX_GU2K_USR_RES_OHI: return "SBX_GU2K_USR_RES_OHI";
    case SBX_GU2K_USR_RES_ETE_L2: return "SBX_GU2K_USR_RES_ETE_L2";
    case SBX_GU2K_USR_RES_ETE_ENCAP: return "SBX_GU2K_USR_RES_ETE_ENCAP";
    case SBX_GU2K_USR_RES_IFID: return "SBX_GU2K_USR_RES_IFID";
    case SBX_GU2K_USR_RES_VRF: return "SBX_GU2K_USR_RES_VRF";
    case SBX_GU2K_USR_RES_VSI: return "SBX_GU2K_USR_RES_VSI";
    case SBX_GU2K_USR_RES_LINE_VSI: return "SBX_GU2K_USR_RES_LINE_VSI";
    case SBX_GU2K_USR_RES_QOS_PROFILE: return "SBX_GU2K_USR_RES_QOS_PROFILE";
    case SBX_GU2K_USR_RES_QOS_EGR_REMARK: return "SBX_GU2K_USR_RES_QOS_EGR_REMARK";
    case SBX_GU2K_USR_RES_LPORT: return "SBX_GU2K_USR_RES_LPORT";
    case SBX_GU2K_USR_RES_MPLS_LPORT: return "SBX_GU2K_USR_RES_MPLS_LPORT";
    case SBX_GU2K_USR_RES_VPLS_COLOR: return "SBX_GU2K_USR_RES_VPLS_COLOR";
    case SBX_GU2K_USR_RES_PROTECTION: return "SBX_GU2K_USR_RES_PROTECTION";
    case SBX_GU2K_USR_RES_VPWS_UNI_LPORT: return "SBX_GU2K_USR_RES_VPWS_UNI_GPORT";
    case SBX_GU2K_USR_RES_FTE_VPWS_UNI_GPORT: return "SBX_GU2K_USR_RES_FTE_VPWS_UNI_GPORT";

    default:
        return "<unknown>";
    }
}
#endif /* BROADCOM_DEBUG */

/*
 *   Function
 *     _sbx_gu2_init_mpls_lp_data_store
 *   Purpose
 *      Initialize mpls logical port database
 *   Parameters
 *      (IN) unit   : unit number of the device
 *   Returns
 *       BCM_E_NONE all resources are successfully initialized
 *       BCM_E_* as appropriate otherwise
 *   Notes
 *       Returns error is any of the resources cannot be initialized
 */
static int 
_sbx_gu2_init_mpls_lp_data_store(int unit)
{
    int rv = BCM_E_NONE;

    rv = shr_htb_create(&_sbx_mpls_lport[unit],
                        /* adjust for power of 2 */
                        SBX_FE2K_NUM_MPLS_LBLS + 1,
                        sizeof(uint32_t),
                        "mpls_lport_hash");
    if (BCM_FAILURE(rv)) {
        ALCR_ERR((_SBX_D(unit, "Failed to Allocate MPLS Lport Database %d: %s\n"),
                  rv, bcm_errmsg(rv)));

    } else {
        ALCR_VERB((_SBX_D(unit, "Successfully allocated MPLS Lport Database\n")));

    }
    return rv;
}

/*
 *   Function
 *      _sbx_gu2_resource_init
 *   Purpose
 *      Initialize the shr resource manager for all HW resources
 *      for the unit
 *   Parameters
 *      (IN) unit   : unit number of the device
 *   Returns
 *       BCM_E_NONE all resources are successfully initialized
 *       BCM_E_* as appropriate otherwise
 *   Notes
 *       Returns error is any of the resources cannot be initialized
 *       Does not clean up partial initialisation.
 */

int
_sbx_gu2_resource_init(int unit)
{
    uint32                      i;
    int                         status;
    int                         base, ete_max_size;
    int                         ete_area_size, gport_ete_space;
    int                         ete_size[SBX_GU2K_HW_RES_ETE_TYPE_MAX];
    _sbx_gu2_res_handle_t      *res_handle;
    _sbx_gu2_hw_res_attrs_t    *res_attrs;
    soc_sbx_g2p3_lsmac_t        lsm;
    soc_sbx_g2p3_esmac_t        esmac;
    bcm_mac_t                   mac;
    uint32                      first, last, valid_lo, valid_hi;
    uint32                      scaling_factor, blocking_factor;
    uint32                      temp = 0;

    if (!SOC_IS_SBX_FE2000(unit)) {
        return BCM_E_UNIT;
    }
    
    if (!(SOC_IS_SBX_G2P3(unit)) ) {
        return BCM_E_UNAVAIL;
    }

    /* initialize the Ingress SMAC index tracker */
    if (_sbx_smac_tracker[unit].ismac.macData == NULL) {
        
        status = soc_sbx_g2p3_lsmac_table_size_get(unit, &temp);
        if (BCM_FAILURE(status)) {
            return status;
        }
        _sbx_mac_tracker_init(&_sbx_smac_tracker[unit].ismac, temp);
    }    
    if (_sbx_smac_tracker[unit].ismac.macData == NULL) {
        return BCM_E_MEMORY;
    }

    /* fill in the smac table on a warm boot init, no reference counts
     * are updated here.  Ref counts will be udpated as modules recover
     * their own data
     */
    if (SOC_WARM_BOOT(unit)) {
        for (i=0; i<temp; i++) {

           status = soc_sbx_g2p3_lsmac_get(unit, i, &lsm);
           if (BCM_FAILURE(status)) {
               return status;
           }
           _sbx_mac_tracker_set(&_sbx_smac_tracker[unit].ismac, lsm.mac, i);
        }
        temp = 0;
    }

    /* initialize the Egress SMAC index tracker */
    if (_sbx_smac_tracker[unit].esmac.macData == NULL) {
        
        temp = soc_sbx_g2p3_esmac_table_size_get(unit);
        _sbx_mac_tracker_init(&_sbx_smac_tracker[unit].esmac, temp);
    }    
    if (_sbx_smac_tracker[unit].esmac.macData == NULL) {
        return BCM_E_MEMORY;
    }

    if (SOC_WARM_BOOT(unit)) {
        for (i=0; i<temp; i++) {

           status = soc_sbx_g2p3_esmac_get(unit, i, &esmac);
           if (BCM_FAILURE(status)) {
               return status;
           }
           mac[5] = esmac.smac5;
           mac[4] = esmac.smac4;
           mac[3] = esmac.smac3;
           mac[2] = esmac.smac2;
           mac[1] = esmac.smac1;
           mac[0] = esmac.smac0;

           _sbx_mac_tracker_set(&_sbx_smac_tracker[unit].esmac, mac, i);
        }
    }

    status = _sbx_gu2_resource_ete_params_get(unit,
                                              &base,
                                              &ete_area_size,
                                              &ete_max_size,
                                              SBX_GU2K_HW_RES_ETE_TYPE_MAX,
                                              ete_size);
    if (status != BCM_E_NONE) {
        return status;
    }

    /**
     * dynamic values setting for ete
     * Carve out the GPORT-ETE space and manage the rest
     * MPLS-ETE for tunnel uses the dynamic range
     */
    gport_ete_space = ((ete_size[SBX_GU2K_HW_RES_ETE_TYPE_ENCAP] +
                        ete_size[SBX_GU2K_HW_RES_ETE_TYPE_ETHERNET])
                       * SBX_MAX_GPORTS);
    _g_sbx_gu2_res_attrs[SBX_GU2K_HW_RES_ETE].scaling_factor =
        ete_max_size;
    _g_sbx_gu2_res_attrs[SBX_GU2K_HW_RES_ETE].base =
        base + gport_ete_space;
    _g_sbx_gu2_res_attrs[SBX_GU2K_HW_RES_ETE].max_count =
        (ete_area_size - gport_ete_space);

    /* dynamic values for FTE, above the VSI range  */
    res_attrs = &_g_sbx_gu2_res_attrs[SBX_GU2K_HW_RES_FTE];
    status = _sbx_gu2_fte_res_attrs_set(unit, res_attrs);
    if (BCM_FAILURE(status)) {
        return status;
    }

    /* dynamic values for LOCAL GPORT fte */
    res_attrs = &_g_sbx_gu2_res_attrs[SBX_GU2K_HW_RES_LOCAL_GPORT_FTE];
    status = _sbx_gu2_local_gport_fte_res_attrs_set(unit, res_attrs);
    if (BCM_FAILURE(status)) {
        return status;
    }

    /* dynamic values for GLOBAL GPORT fte */
    res_attrs = &_g_sbx_gu2_res_attrs[SBX_GU2K_HW_RES_GLOBAL_GPORT_FTE];
    status = _sbx_gu2_global_gport_fte_res_attrs_set(unit, res_attrs);
    if (BCM_FAILURE(status)) {
        return status;
    }

    /* dynamic VSI values */
    res_attrs = &_g_sbx_gu2_res_attrs[SBX_GU2K_HW_RES_VSI];
    status = _sbx_gu2_vsi_res_attrs_set(unit, res_attrs);
    if (BCM_FAILURE(status)) {
        return status;
    }

    /* dynamic values for LINE VSIs */
    res_attrs = &_g_sbx_gu2_res_attrs[SBX_GU2K_HW_RES_LINE_VSI];
    status = _sbx_gu2_line_vsi_res_attrs_set(unit, res_attrs);
    if (BCM_FAILURE(status)) {
        return status;
    }

    /* dynamic values setting for OHI */
    res_attrs = &_g_sbx_gu2_res_attrs[SBX_GU2K_HW_RES_OHI];
    status = _sbx_gu2_oh_res_attrs_set(unit, res_attrs);
    if (BCM_FAILURE(status)) {
        return status;
    }

    status = _sbx_gu2_mplstp_vpws_uni_res_attrs_set(unit);
    if (BCM_FAILURE(status)) {
        ALCR_ERR(("Failed to intialize mplstp resource attributes"));
        return status;
    }
    
    /* dynamic value for max qos profile & remark 
     * the remark tables are profiles too, use the same numer as 
     * qos profiles for remark's */
    status = soc_sbx_g2p3_max_qos_profile_index_get(unit, &temp);
    _g_sbx_gu2_res_attrs[SBX_GU2K_HW_RES_QOS_PROFILE].max_count = 
        _g_sbx_gu2_res_attrs[SBX_GU2K_HW_RES_QOS_EGR_REMARK].max_count = 
        (uint32) (temp - SBX_QOS_PROFILE_BASE);
    if (BCM_FAILURE(status)) {
        return status;
    }

#ifdef BCM_FE2000_P3_SUPPORT
    if (SOC_IS_SBX_G2P3(unit)) {

        res_attrs = &_g_sbx_gu2_res_attrs[SBX_GU2K_HW_RES_LPORT];
        res_attrs->base = SBX_MAX_PORTS;
        BCM_IF_ERROR_RETURN
            (soc_sbx_g2p3_vlan_ft_base_get(unit, &res_attrs->max_count));
        res_attrs->max_count -= res_attrs->base;
    }
#endif

    for (i = SBX_GU2K_HW_RES_FTE;
         i < SBX_GU2K_HW_RES_MAX; i++) {
        res_handle              = &_g_sbx_gu2_res_handles[unit][i];
        res_handle->alloc_style = SBX_GU2K_ALLOC_STYLE_NONE;
    }

    for (i = SBX_GU2K_HW_RES_FTE;
         i < SBX_GU2K_HW_RES_MAX; i++) {
        res_attrs       =  &_g_sbx_gu2_res_attrs[i];
        res_handle      =  &_g_sbx_gu2_res_handles[unit][i];
        first           =  res_attrs->base;
        last            =  first + res_attrs->max_count - 1;
        valid_lo        = first;
        valid_hi        = last;
        scaling_factor  = res_attrs->scaling_factor;
        blocking_factor =  res_attrs->blocking_factor;

        if (res_attrs->alloc_style == SBX_GU2K_ALLOC_STYLE_VERSATILE) {
            res_handle->alloc_style = SBX_GU2K_ALLOC_STYLE_VERSATILE;
            status = shr_aidxres_list_create(&res_handle->aidx_handle,
                                             (shr_aidxres_element_t) first,
                                             (shr_aidxres_element_t) last,
                                             (shr_aidxres_element_t) valid_lo,
                                             (shr_aidxres_element_t) valid_hi,
                                             (shr_aidxres_element_t) blocking_factor,
                                             res_attrs->name);
        } else if (res_attrs->alloc_style == SBX_GU2K_ALLOC_STYLE_SCALED) {
            res_handle->alloc_style = SBX_GU2K_ALLOC_STYLE_SCALED;
            status = shr_idxres_list_create_scaled(&res_handle->idx_handle,
                                                   (shr_idxres_element_t) first,
                                                   (shr_idxres_element_t) last,
                                                   (shr_idxres_element_t) valid_lo,
                                                   (shr_idxres_element_t) valid_hi,
                                                   (shr_idxres_element_t) scaling_factor,
                                                   res_attrs->name);
        } else {
            res_handle->alloc_style = SBX_GU2K_ALLOC_STYLE_SINGLE;
            status = shr_idxres_list_create(&res_handle->idx_handle,
                                            (shr_idxres_element_t) first,
                                            (shr_idxres_element_t) last,
                                            (shr_idxres_element_t) valid_lo,
                                            (shr_idxres_element_t) valid_hi,
                                            res_attrs->name);
        }

        if (status != BCM_E_NONE) {
            break;
        }
    }

    if (status == BCM_E_NONE) {
        void *vp;
        uint32 first, last;

        /**
         * xyz_init() needs to be called from thread safe
         * user code. Once _init() is successful, other calls
         * have the lock to make sure access is ordered.
         */
        if (!(_SBX_GU2_RESOURCE_LOCK_CREATED_ALREADY(unit))) {
            if ((_sbx_gu2_resource_mlock[unit] =
                 sal_mutex_create("_sbx_gu2_resource_lock")) == NULL) {
                /* Nothing should work further */
                return BCM_E_MEMORY;
            }


            if(_sbx_gu2_init_mpls_lp_data_store(unit)) {
                return BCM_E_MEMORY;
            }

            _sbx_lport[unit] = (_sbx_lportList*)(sal_alloc(
                                                        sizeof(_sbx_lportList),
                                                        "_sbx_gu2_lport_ptrs"));
            if (_sbx_lport[unit]) {
                sal_memset(_sbx_lport[unit], 0, sizeof(_sbx_lportList));
            } else {
                sal_mutex_destroy(_sbx_gu2_resource_mlock[unit]);
                _sbx_gu2_resource_mlock[unit] = NULL;
                return BCM_E_MEMORY;
            }
            _sbx_lptype[unit] = (_sbx_lpTypeList*)(sal_alloc(
                                                       sizeof(_sbx_lpTypeList),
                                                       "_sbx_gu2_lptype_ptrs"));
            if (_sbx_lptype[unit]) {
                sal_memset(_sbx_lptype[unit],
                           BCM_GPORT_INVALID,
                           sizeof(_sbx_lpTypeList));
            } else {
                sal_mutex_destroy(_sbx_gu2_resource_mlock[unit]);
                _sbx_gu2_resource_mlock[unit] = NULL;
                return BCM_E_MEMORY;
            }

            /* Allocate Global FTE to Type mapping space dynamically based
             * on FTE space */
            if((_sbx_gu2_alloc_range_get(unit, SBX_GU2K_HW_RES_GLOBAL_GPORT_FTE, 
                                         &first, &last)) != BCM_E_NONE) {
                /* could not obtain global fte range */
                return BCM_E_INTERNAL;
            }
            
            vp = sal_alloc(sizeof(uint8) * ((last - first)+1),
                           "_sbx_gu2_global_ftType_ptrs");
            _sbx_global_ftType[unit] = (uint8*)vp;

            if (_sbx_global_ftType[unit]) {
                sal_memset(_sbx_global_ftType[unit],
                           BCM_GPORT_TYPE_NONE,
                           sizeof(uint8) * ((last - first)+1));
            } else {
                sal_mutex_destroy(_sbx_gu2_resource_mlock[unit]);
                _sbx_gu2_resource_mlock[unit] = NULL;
                return BCM_E_MEMORY;
            }
        }

        /* Allocate dynamic VSI to Type & opaque pointer Mapping array */
        res_attrs = &_g_sbx_gu2_res_attrs[SBX_GU2K_HW_RES_VSI];

        vp =  sal_alloc(sizeof(uint8) * (res_attrs->max_count+1),
                        "_sbx_gu2_global_vsiType_ptrs"); 
        _sbx_global_vsiType[unit] = (uint8*)vp;
        
        if (_sbx_global_vsiType[unit]) {
            sal_memset(_sbx_global_vsiType[unit],
                       BCM_GPORT_TYPE_NONE,
                       sizeof(uint8) * (res_attrs->max_count+1));
        } else {
            sal_mutex_destroy(_sbx_gu2_resource_mlock[unit]);
            _sbx_gu2_resource_mlock[unit] = NULL;
            return BCM_E_MEMORY;
        }
    }

    return status;
}

/*
 *   Function
 *      _sbx_gu2_resource_alloc
 *   Purpose
 *      Allocate "count" number of resources.
 *   Parameters
 *      unit    - (IN)  unit number of the device
 *      type    - (IN)  resource type
 *                      (one of _sbx_gu2_usr_res_types_t)
 *      count   - (IN)  Number of resources required
 *      elements- (OUT) Resource Index return by the underlying allocator
 *      flags   - (IN)  _SBX_GU2_RES_FLAGS_CONTIGOUS
 *                      if the resources need to be contigous
 *   Returns
 *      BCM_E_NONE if element allocated successfully
 *      BCM_E_RESOURCE if resource is in reserved range and compliant, not 
 *                     neccessarily an error.
 *      BCM_E_* as appropriate otherwise
 */
int
_sbx_gu2_resource_alloc(int                        unit,
                        _sbx_gu2_usr_res_types_t   type,
                        uint32                     count,
                        uint32                    *elements,
                        uint32                     flags)
{
    _sbx_gu2_res_handle_t    handle;
    _sbx_gu2_hw_res_attrs_t *res_attrs;
    int                       status, res;
    uint32                    i, size;
    uint32                   *done;
    shr_aidxres_element_t     tmp_elem, first, last;    

    if ((count <= 0) || (elements == NULL)) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(_SBX_GU2_RESOURCE_LOCK(unit));

    status = _sbx_gu2_get_resource_handle(unit,
                                          type,
                                          &handle);
    if (status != BCM_E_NONE) {
        _SBX_GU2_RESOURCE_UNLOCK(unit);
        return status;
    }

    res_attrs =  &_g_sbx_gu2_res_attrs[_g_mapped_hw_res[type]];

    /*
     * if the reserve flag is set, make sure that the elements are
     * available
     */
    if (flags & _SBX_GU2_RES_FLAGS_RESERVE) {
        int checkVal = BCM_E_NOT_FOUND;
        int inRange = 0;
        
        if (res_attrs->reservedHigh && res_attrs->reservedLow) {

            for (i = 0; i < count; i++) {
                if (elements[i] >= res_attrs->reservedLow && 
                    elements[i] <= res_attrs->reservedHigh) {
                    inRange++;
                }
            }

            ALCR_VERB((_SBX_D(unit, "Found a reserved range on resource %d: "
                              "0x%08x-0x%08x count=%d inRange=%d\n"), 
                       type, res_attrs->reservedLow, res_attrs->reservedHigh,
                       count, inRange));
            
            if (inRange && inRange != count) {
                /* part some elements are in reserved range, but not all */
                _SBX_GU2_RESOURCE_UNLOCK(unit);
                return BCM_E_PARAM;
            }

            /* If these elements are in the reserved range, 
             * verify they exist 
             */
            if (inRange) {
                checkVal = BCM_E_EXISTS;
            }
        }

        switch (handle.alloc_style) {
            case SBX_GU2K_ALLOC_STYLE_SINGLE:
            case SBX_GU2K_ALLOC_STYLE_SCALED:
                for (i = 0; i < count; i++) {

                    /* check and apply translator */
                    tmp_elem = elements[i];
                    if (res_attrs->e2i) {
                        tmp_elem = res_attrs->e2i(unit, elements[i]);
                    }

                    res = shr_idxres_list_elem_state(handle.idx_handle, 
                                                     tmp_elem);
                    if (res != checkVal) {
                        status = BCM_E_BUSY;
                        break;
                    }
                    
                }
            break;

            case  SBX_GU2K_ALLOC_STYLE_VERSATILE:
                for (i = 0; i < count; i++) {
                    if (flags & _SBX_GU2_RES_FLAGS_CONTIGOUS) {
                        tmp_elem = (shr_aidxres_element_t) (elements[0] + i);
                    } else {
                        tmp_elem = (shr_aidxres_element_t)elements[i];
                    }
                    /* check and apply translator */
                    if (res_attrs->e2i) {
                        tmp_elem = res_attrs->e2i(unit, elements[i]);
                    }

                    res = shr_aidxres_list_elem_state(handle.aidx_handle,
                                                      tmp_elem);
                    if (res != checkVal) {
                        status = BCM_E_BUSY;
                        break;
                    }
                }

            break;
        }

        /* may not be a real error; informing the caller that the resource
         * is in the reserved range, and checks out as OK.
         */ 
        if (inRange && BCM_SUCCESS(status)) {
            status = BCM_E_RESOURCE;
        }
    }

    if (status != BCM_E_NONE) {
        _SBX_GU2_RESOURCE_UNLOCK(unit);
        return status;
    }

    done = NULL;
    if ( (count > 1) &&
         (!(flags & ((_SBX_GU2_RES_FLAGS_CONTIGOUS) |
                     (_SBX_GU2_RES_FLAGS_RESERVE))))) {
        size = (sizeof(uint32) * count);
        done = (uint32 *)sal_alloc(size, "RES-LIB");
        if (done == NULL) {
            _SBX_GU2_RESOURCE_UNLOCK(unit);
            return BCM_E_MEMORY;
        }
        sal_memset(done, 0, size);
    }

    if ((flags & _SBX_GU2_RES_FLAGS_RESERVE) && (res_attrs->e2i)) {
        for (i = 0; i < count; i++) {
            elements[i] = res_attrs->e2i(unit, elements[i]);
        }
    }

    switch (handle.alloc_style) {
        case SBX_GU2K_ALLOC_STYLE_SINGLE:
            if (count > 1) {
                status = BCM_E_PARAM;
            } else {
                if (flags & _SBX_GU2_RES_FLAGS_RESERVE) {
                    first  = (shr_aidxres_element_t)elements[0];
                    last   = (shr_aidxres_element_t)elements[0];
                    status = shr_idxres_list_reserve(handle.idx_handle,
                                                     first,
                                                     last);
                } else {
                    status = shr_idxres_list_alloc(handle.idx_handle,
                                                   (shr_aidxres_element_t *)elements);
                }
            }
        break;

        case  SBX_GU2K_ALLOC_STYLE_SCALED:
            if (flags & _SBX_GU2_RES_FLAGS_CONTIGOUS) {
                status = BCM_E_PARAM;
            } else {
                if (flags & _SBX_GU2_RES_FLAGS_RESERVE) {
                    for (i = 0; i < count; i++) {
                        first  = (shr_aidxres_element_t)elements[i];
                        last   = (shr_aidxres_element_t)elements[i];
                        shr_idxres_list_reserve(handle.idx_handle,
                                                first,
                                                last);
                    }
                } else {
                    if (count == 1) {
                        status = shr_idxres_list_alloc(handle.idx_handle,
                                                       (shr_aidxres_element_t *)elements);
                    } else {
                        status = shr_idxres_list_alloc_set(handle.idx_handle,
                                                           (shr_idxres_element_t)   count,
                                                           (shr_idxres_element_t *) elements,
                                                           (shr_idxres_element_t *) done);
                    }
                }
            }
        break;

        case SBX_GU2K_ALLOC_STYLE_VERSATILE:
            if (flags & _SBX_GU2_RES_FLAGS_RESERVE) {
                if ((count == 1) || ((flags & _SBX_GU2_RES_FLAGS_CONTIGOUS))) {
                    first  = (shr_aidxres_element_t)elements[0];
                    last   = (shr_aidxres_element_t)elements[0] + count - 1;
                    status = shr_aidxres_list_reserve(handle.aidx_handle,
                                                      first,
                                                      last);
                } else {
                    for (i = 0; i < count; i++) {
                        first  = (shr_aidxres_element_t)elements[i];
                        last   = (shr_aidxres_element_t)elements[i];
                        status = shr_aidxres_list_reserve(handle.aidx_handle,
                                                          first,
                                                          last);
                    }
                }
            } else {
                if (count == 1) {
                    status = shr_aidxres_list_alloc(handle.aidx_handle,
                                                    (shr_aidxres_element_t *) elements);
                } else {
                    if (!(flags & _SBX_GU2_RES_FLAGS_CONTIGOUS)) {
                        status = shr_aidxres_list_alloc_set(handle.aidx_handle,
                                                            (shr_aidxres_element_t)   count,
                                                            (shr_aidxres_element_t *) elements,
                                                            (shr_aidxres_element_t *) done);
                    } else {
                        shr_aidxres_list_alloc_block(handle.aidx_handle,
                                                     (shr_aidxres_element_t)  count,
                                                     (shr_aidxres_element_t *)elements);
                    }
                }
            }

        break;
    }

    if (done != NULL) {
        if (status != BCM_E_NONE) {
            for (i = 0; i < count; i++) {
                if (done[i]) {
                    if (handle.alloc_style == SBX_GU2K_ALLOC_STYLE_VERSATILE) {
                        shr_aidxres_list_free(handle.aidx_handle,
                                              done[i]);
                    } else {
                         shr_idxres_list_free(handle.idx_handle,
                                              done[i]);
                    }
                }
            }
        }
        sal_free(done);
    }

    if (status != BCM_E_NONE) {
        _SBX_GU2_RESOURCE_UNLOCK(unit);
        return status;
    }

    /* check and apply translators */
    if (res_attrs->i2e) {
        for (i = 0; i < count; i++) {
            elements[i] = res_attrs->i2e(unit, elements[i]);
        }
    }

    _SBX_GU2_RESOURCE_UNLOCK(unit);
    return BCM_E_NONE;
}

/*
 *   Function
 *      _sbx_gu2_resource_free
 *   Purpose
 *      Free the resources specified in the array
 *   Parameters
 *      unit    - (IN)  unit number of the device
 *      type    - (IN)  resource type
 *                      (one of _sbx_gu2_usr_res_types_t)
 *      count   - (IN)  Number of resources to be freed
 *      elements- (IN)  Resource Ids to be freed
 *   Returns
 *      BCM_E_NONE if element freed successfully
 *      BCM_E_* as appropriate otherwise
 */

int
_sbx_gu2_resource_free(int                        unit,
                       _sbx_gu2_usr_res_types_t   type,
                       uint32                     count,
                       uint32                    *elements,
                       uint32                     flags)
{
    _sbx_gu2_res_handle_t     handle;
    int                       status;
    uint32                    i;
    _sbx_gu2_hw_res_attrs_t *res_attrs;

    if ((count <= 0) || (elements == NULL)) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(_SBX_GU2_RESOURCE_LOCK(unit));

    status = _sbx_gu2_get_resource_handle(unit,
                                          type,
                                          &handle);

    if (status != BCM_E_NONE) {
        _SBX_GU2_RESOURCE_UNLOCK(unit);
        return status;
    }

    res_attrs =  &_g_sbx_gu2_res_attrs[_g_mapped_hw_res[type]];
    
    if (res_attrs->reservedHigh && res_attrs->reservedLow) {
        int inRange = 0;

        for (i = 0; i < count; i++) {
            if (elements[i] >= res_attrs->reservedLow && 
                elements[i] <= res_attrs->reservedHigh) {
                inRange++;
            }
        }

        ALCR_VERB((_SBX_D(unit, "Found a reserved range on resource %d: "
                          "0x%08x-0x%08x count=%d inRange=%d\n"), 
                   type, res_attrs->reservedLow, res_attrs->reservedHigh,
                   count, inRange));

        if (inRange && (inRange != count)) {
            /* part some elements are in reserved range, but not all */
            _SBX_GU2_RESOURCE_UNLOCK(unit);
            return BCM_E_PARAM;
        }

        /* nothing to do */
        if (inRange) {
            _SBX_GU2_RESOURCE_UNLOCK(unit);
            return BCM_E_NONE;
        }
    }

    /* check and apply translators */
    if (res_attrs->e2i) {
        for (i = 0; i < count; i++) {
            elements[i] = res_attrs->e2i(unit, elements[i]);
        }
    }

    if ((handle.alloc_style == SBX_GU2K_ALLOC_STYLE_VERSATILE) &&
        (flags & _SBX_GU2_RES_FLAGS_CONTIGOUS)) {
        status =  shr_aidxres_list_free(handle.aidx_handle,
                                        (shr_aidxres_element_t)elements[0]);
    } else {
        for (i = 0; i < count; i++) {
            if (handle.alloc_style == SBX_GU2K_ALLOC_STYLE_VERSATILE) {
                status = shr_aidxres_list_free(handle.aidx_handle,
                                               (shr_aidxres_element_t)elements[i]);
            } else {
                status =  shr_idxres_list_free(handle.idx_handle,
                                               (shr_idxres_element_t)elements[i]);
            }
        }
    }

    _SBX_GU2_RESOURCE_UNLOCK(unit);
    return status;
}

/*
 *   Function
 *      _sbx_gu2_resource_test
 *   Purpose
 *      Check to see if the specified resource is available
 *   Parameters
 *      unit    - (IN)  unit number of the device
 *      type    - (IN)  resource type
 *                      (one of _sbx_gu2_usr_res_types_t)
 *      element - (IN)  Resource Id to be checked
 *   Returns
 *      BCM_E_NOT_FOUND if resource is available
 *      BCM_E_EXIST     if resource is in use
 *      BCM_E_*         as appropriate otherwise
 */

int
_sbx_gu2_resource_test(int                        unit,
                       _sbx_gu2_usr_res_types_t   type,
                       uint32                     element)
{
    _sbx_gu2_res_handle_t     handle;
    int                       status;
    _sbx_gu2_hw_res_attrs_t  *res_attrs;

    BCM_IF_ERROR_RETURN(_SBX_GU2_RESOURCE_LOCK(unit));

    status = _sbx_gu2_get_resource_handle(unit,
                                          type,
                                          &handle);
    if (status != BCM_E_NONE) {
        _SBX_GU2_RESOURCE_UNLOCK(unit);
        return status;
    }

    res_attrs =  &_g_sbx_gu2_res_attrs[_g_mapped_hw_res[type]];

    /* check and apply translators */
    if (res_attrs->e2i) {
        element = res_attrs->e2i(unit, element);
    }

    if (handle.alloc_style == SBX_GU2K_ALLOC_STYLE_VERSATILE) {
        status = shr_aidxres_list_elem_state(handle.aidx_handle,
                                             (shr_aidxres_element_t)element);
    } else {
        status = shr_idxres_list_elem_state(handle.idx_handle,
                                            (shr_idxres_element_t)element);
    }

    _SBX_GU2_RESOURCE_UNLOCK(unit);
    return status;
}

/*
 *   Function
 *      _sbx_gu2_shr_resource_uninit
 *   Purpose
 *      Destroy the all the resource managers for this unit
 *   Parameters
 *      (IN) unit   : unit number of the device
 *   Returns
 *       BCM_E_NONE all resources are successfully destroyed
 *       BCM_E_* as appropriate otherwise
 *   Notes
 *       Returns error is any of the resources cannot be destroyed
 */

int
_sbx_gu2_resource_uninit(int unit)
{
    uint32                     i;
    int                        status, ret;
    _sbx_gu2_res_handle_t     *res_handle;

    if (!SOC_IS_SBX_FE2000(unit)) {
        return BCM_E_UNIT;
    }

    BCM_IF_ERROR_RETURN(_SBX_GU2_RESOURCE_LOCK(unit));
    ret = BCM_E_NONE;

    for (i = SBX_GU2K_HW_RES_FTE;
         i < SBX_GU2K_HW_RES_MAX; i++) {
         res_handle         = &_g_sbx_gu2_res_handles[unit][i];
         if (res_handle->alloc_style == SBX_GU2K_ALLOC_STYLE_VERSATILE) {
             status = shr_aidxres_list_destroy(res_handle->aidx_handle);
         } else {
             status = shr_idxres_list_destroy(res_handle->idx_handle);
         }
         if (status != BCM_E_NONE) {
             ret = status;
         }
    }
    
     if (_sbx_smac_tracker[unit].ismac.macData) {
         _sbx_mac_tracker_destroy(&_sbx_smac_tracker[unit].ismac);
     }
     if (_sbx_smac_tracker[unit].esmac.macData) {
         _sbx_mac_tracker_destroy(&_sbx_smac_tracker[unit].esmac);
     }
    

    _SBX_GU2_RESOURCE_UNLOCK(unit);
    if (_sbx_gu2_resource_mlock[unit] != NULL) {
        sal_mutex_destroy(_sbx_gu2_resource_mlock[unit]);
        _sbx_gu2_resource_mlock[unit] = NULL;
    }

    /**
     * No use send error here.. we have destroyed the lock,
     * _init() needs to be done
     */
    return BCM_E_NONE;
}

/*
 *   Function
 *      _sbx_gu2_alloc_wellknown_resources
 *   Purpose
 *      Allocate well-known resouces per unit
 *      and save the same in the soc structure
 *      for sharing between modules.
 *
 *   Parameters
 *      (IN) unit   : unit number of the device
 *   Returns
 *       BCM_E_NONE - All required resources are allocated
 *       BCM_E_*    - failure
 *   Notes
 */

int
_sbx_gu2_alloc_wellknown_resources(int unit)
{
    int                status;
    uint32             idx;

    BCM_IF_ERROR_RETURN(_SBX_GU2_RESOURCE_LOCK(unit));

    status = _sbx_gu2_resource_alloc(unit,
                                     SBX_GU2K_USR_RES_ETE_L2,
                                     SBX_MAX_PORTS,
                                     &(SOC_SBX_PORT_ETE(unit, 0)),
                                     0);
    if (status != BCM_E_NONE) {
        _SBX_GU2_RESOURCE_UNLOCK(unit);
        return status;
    }

    status = _sbx_gu2_resource_alloc(unit,
                                     SBX_GU2K_USR_RES_ETE_L2,
                                     SBX_MAX_PORTS,
                                     &(SOC_SBX_PORT_UT_ETE(unit, 0)),
                                     0);
    if (status != BCM_E_NONE) {
        _SBX_GU2_RESOURCE_UNLOCK(unit);
        return status;
    }

    status = _sbx_gu2_resource_alloc(unit,
                                     SBX_GU2K_USR_RES_ETE_L2,
                                     SBX_MAX_TRUNKS,
                                     &(SOC_SBX_TRUNK_ETE(unit, 0)),
                                     0);
    if (status != BCM_E_NONE) {
        _SBX_GU2_RESOURCE_UNLOCK(unit);
        return status;
    }

    /**
     * allocate the invalid-ete from allocator
     */
    status = _sbx_gu2_resource_alloc(unit,
                                     SBX_GU2K_USR_RES_ETE_L2,
                                     1,
                                     &(SOC_SBX_INVALID_L2ETE(unit)),
                                     0);
    if (status != BCM_E_NONE) {
        _SBX_GU2_RESOURCE_UNLOCK(unit);
        return status;
    }

    /**
     * Currently there is no valid bit in the ETE.
     * This will trip the packet on the MTU check (MTU = 0)
     */
    if (SOC_IS_SBX_G2P3(unit)) {
#ifdef  BCM_FE2000_P3_SUPPORT
        soc_sbx_g2p3_etel2_t    sbx_etel2;
        soc_sbx_g2p3_ft_t       ft;

        idx = SOC_SBX_INVALID_L2ETE(unit);
        soc_sbx_g2p3_etel2_t_init(&sbx_etel2);
        status = soc_sbx_g2p3_etel2_set(unit, idx, &sbx_etel2);;
        if (status != BCM_E_NONE) {
            _SBX_GU2_RESOURCE_UNLOCK(unit);
            return status;
        }

        idx = SBX_DROP_FTE(unit);
        soc_sbx_g2p3_ft_t_init(&ft);
        ft.excidx = G2P3_DROP_FTE_EXCEPTION;
        /* forward and learn are zero. So its effectively a drop */
        status = soc_sbx_g2p3_ft_set(unit, idx, &ft);
        if (status != BCM_E_NONE) {
            _SBX_GU2_RESOURCE_UNLOCK(unit);
            return status;
        }

#ifdef BCM_QE2000_SUPPORT
        idx = SBX_CPU_FTE(unit);
        soc_sbx_g2p3_ft_t_init(&ft);
        ft.qid = SBX_EXC_QID_BASE(unit);
        status = soc_sbx_g2p3_ft_set(unit, idx, &ft);
        if (status != BCM_E_NONE) {
            _SBX_GU2_RESOURCE_UNLOCK(unit);
            return status;
        }
#endif /* BCM_QE2000_SUPPORT */
#endif /* BCM_FE2000_P3_SUPPORT */
    }

    _SBX_GU2_RESOURCE_UNLOCK(unit);
    return BCM_E_NONE;
}

/*
 *   Function
 *      _sbx_mac_tracker_init
 *   Purpose
 *     instantiate a mac tracking object
 *
 *   Parameters
 *      (IN)  tracker   : mac tracker object to create
 *      (IN)  numMac    : number of MACs to track
 *   Returns
 *       BCM_E_NONE - All required resources are allocated
 *       BCM_E_*    - failure
 *   Notes
 */
static int
_sbx_mac_tracker_init(_sbx_mac_tracker_t *tracker, uint32 numMacs)
{
    uint32 bufSize;

    tracker->numMacs = numMacs;
    
    bufSize = numMacs * sizeof(_sbx_track_mac_data_t);
    tracker->macData = (_sbx_track_mac_data_t*)
        (sal_alloc(bufSize, "smac tracking state"));

    if (tracker->macData) {
        sal_memset(tracker->macData, 0, bufSize);
        return BCM_E_NONE;
    }
    return BCM_E_MEMORY;
}

/*
 *   Function
 *      _sbx_mac_tracker_destroy
 *   Purpose
 *     destroy a mac tracking object
 *
 *   Parameters
 *      (IN)  tracker   : mac tracker object to destroy
 *   Returns
 *       BCM_E_NONE - All required resources are freed
 *       BCM_E_*    - failure
 *   Notes
 */
static int
_sbx_mac_tracker_destroy(_sbx_mac_tracker_t *tracker)
{
    tracker->numMacs = 0;
    sal_free(tracker->macData);
    tracker->macData = NULL;
    return BCM_E_NONE;
}

/*
 *   Function
 *      _sbx_mac_tracker_alloc
 *   Purpose
 *     allocate an mac index from the general purpose mac tracker
 *
 *   Parameters
 *      (IN)  tracker  : mac tracker object from which to allocate 
 *      (IN)  flags    : reserved or not
 *      (IN)  mac      : MAC to allocate/store
 *      (OUT) idx      : index allcated
 *   Returns
 *       BCM_E_NONE - All required resources are allocated
 *       BCM_E_*    - failure
 *   Notes
 *     uses reference counts when the same MAC is allocated.
 */
static int
_sbx_mac_tracker_alloc(_sbx_mac_tracker_t *tracker, int flags, 
                       bcm_mac_t mac, bcm_port_t port, uint32* idx) 
{
    int     scanIdx, freeIdx = -1;
    int     found = 0;

    if (flags & _SBX_GU2_RES_FLAGS_RESERVE) {
        found = 1;
        scanIdx = *idx;
    } else {
        /* Scan for a match first */
        for (scanIdx = 0; scanIdx < tracker->numMacs; scanIdx++) {
            if (tracker->macData[scanIdx].refCount == 0) {
                freeIdx = scanIdx;
            } else {
                found = (sal_memcmp(tracker->macData[scanIdx].mac, mac,
                                    sizeof(bcm_mac_t))   == 0);
                if (found) {
                    if (!(tracker->macData[scanIdx].port == _SBX_GU2_RES_UNUSED_PORT)) {
                        if (tracker->macData[scanIdx].port == port) {
                            break;
                        } else {
                            found = 0;
                        }
                    } else {
                        break;
                    }

                }
            }
        }
    }

    /* No match found, allocate a new entry */
    if (!found) {
        if (freeIdx < 0) {
            return BCM_E_FULL;
        } else {
            scanIdx = freeIdx;
        }
        sal_memcpy(tracker->macData[scanIdx].mac, mac, sizeof(bcm_mac_t));
        tracker->macData[scanIdx].port = port;
        tracker->macData[scanIdx].flags = flags;
    }

    /* increment the ref count */
    tracker->macData[scanIdx].refCount++;
    *idx = scanIdx;
    return BCM_E_NONE;
}


/*
 *   Function
 *      _sbx_mac_tracker_free
 *   Purpose
 *     free an smac index
 *
 *   Parameters
 *      (IN)     tracker: mac tracker object from which to free 
 *      (IN/OUT) mac    : MAC to free
 *      (IN/OUT) idx    : index freed
 *   Returns
 *       BCM_E_EMPTY  : ref count equals zero, HW may be cleared
 *       BCM_E_NONE   : ref count decremented, but still non-zero
 *       BCM_E_*    - failure
 *   Notes
 *     if idx == ~0, <mac> is used to scan the table for a match, and
 *                   match idx will be returned in *idx
 *     if idx != ~0, the smac found will be copied to <mac>
 */
static int
_sbx_mac_tracker_free(_sbx_mac_tracker_t *tracker, 
                      bcm_mac_t mac, bcm_port_t port, uint32* idx)
{
    uint32 freeIdx = *idx;

    /* Scan for a match? */
    if (freeIdx == ~0) {
        int scanIdx, found = 0;
    
        for (scanIdx = 0; scanIdx < tracker->numMacs; scanIdx++) {
            if (tracker->macData[scanIdx].refCount > 0) {
                found = (sal_memcmp(tracker->macData[scanIdx].mac,
                                    mac, sizeof(bcm_mac_t))  == 0);
                if (found) {
                    if (!(tracker->macData[scanIdx].port == _SBX_GU2_RES_UNUSED_PORT)) {
                        if (tracker->macData[scanIdx].port == port) {
                            break;
                        } else {
                            found = 0;
                        }
                    } else {
                        break;
                    }
                }
            }
        }

        if (!found) {
            return BCM_E_RESOURCE;
        }
        
        freeIdx = scanIdx;
    } else {
        
        if (freeIdx >= tracker->numMacs) {
            return BCM_E_PARAM;
        }
    }

    if (tracker->macData[freeIdx].refCount <= 0) {
        return BCM_E_NOT_FOUND;
    }

    *idx = freeIdx;

    /* decrement the ref count */
    tracker->macData[freeIdx].refCount--;
#if  defined(BROADCOM_DEBUG)
    sal_memcpy(mac, tracker->macData[freeIdx].mac, sizeof(bcm_mac_t));
#endif

    if (tracker->macData[freeIdx].refCount <= 0) {
        sal_memset(tracker->macData[freeIdx].mac, 0, sizeof(bcm_mac_t));
        return BCM_E_EMPTY;
    }

    return BCM_E_NONE;
}

/*
 *   Function
 *      _sbx_mac_tracker_set
 *   Purpose
 *     Set a mac a particular index; reference counts are not adjusted;
 *     useful during a warm boot
 *
 *   Parameters
 *      (IN) tracker: mac tracker object to set mac
 *      (IN) mac    : MAC to set
 *      (IN) idx    : index set
 *   Returns
 *       BCM_E_*    - failure
 *   Notes
 */
static int
_sbx_mac_tracker_set(_sbx_mac_tracker_t *tracker, bcm_mac_t mac, uint32 idx)
{
    if (idx >= tracker->numMacs) {
        return BCM_E_PARAM;
    }
    sal_memcpy(tracker->macData[idx].mac, mac, sizeof(bcm_mac_t));
    return BCM_E_NONE;
}

/*
 *   Function
 *      _sbx_mac_tracker_find
 *   Purpose
 *      find a mac, ignoring ref counts; useful for warm boot
 *   Parameters
 *      (IN)  tracker: mac tracker object to set mac
 *      (IN)  mac    : MAC to set
 *      (OUT) idx    : index set
 *   Returns
 *       BCM_E_*    - failure
 *   Notes
 */
static int
_sbx_mac_tracker_find(_sbx_mac_tracker_t *tracker, bcm_mac_t mac, uint32 *idx)
{
    int scan_idx, found;

    for (scan_idx = 0; scan_idx < tracker->numMacs; scan_idx++) {
        found = (sal_memcmp(tracker->macData[scan_idx].mac, mac,
                            sizeof(bcm_mac_t))   == 0);
        if (found) {
            *idx = scan_idx;
            return BCM_E_NONE;
        }
    }
    return BCM_E_NOT_FOUND;
}


/*
 *   Function
 *      _sbx_gu2_ismac_idx_alloc
 *   Purpose
 *     allocate an ingress smac index for use in local station match
 *
 *   Parameters
 *      (IN)  unit     : unit number of the device
 *      (IN)  flags    : reserved or not
 *      (IN)  smac     : SMAC to allocate
 *      (IN)  port     : port
 *      (OUT) idx      : soc layer smac index to use
 *   Returns
 *       BCM_E_NONE - All required resources are allocated
 *       BCM_E_*    - failure
 *   Notes
 *     uses reference counts when the same MAC is allocated.
 *     No hw is ever written 
 */
int
_sbx_gu2_ismac_idx_alloc(int unit, uint32 flags, bcm_mac_t smac, 
                         bcm_port_t port, uint32* smacIdx)
{
    int rv;
    _sbx_mac_tracker_t* tracker = &_sbx_smac_tracker[unit].ismac;

    if (tracker->macData == NULL) {
        return BCM_E_INIT;
    }

    /* tracker must be pre-loaded with zero-ref count mac addreses 
     * from hardware before we get here.  ie. shd be done at init
     */
    if (SOC_WARM_BOOT(unit) && (flags & _SBX_GU2_RES_FLAGS_RESERVE)) {
        rv = _sbx_mac_tracker_find(tracker, smac, smacIdx);
        if (BCM_FAILURE(rv)) {
            return rv;
        }
    }

    rv = _sbx_mac_tracker_alloc(tracker, flags, smac, port, smacIdx);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    ALCR_VERB((_SBX_D(unit, "allocated ismacIdx for mac: " MAC_FMT
                      " refCount=%d\n"),
               MAC_PFMT(smac), 
               tracker->macData[*smacIdx].refCount));

    return rv;
}

/*
 *   Function
 *      _sbx_gu2_ismac_idx_free
 *   Purpose
 *     free an ingress smac index
 *
 *   Parameters
 *      (IN)     unit   : unit number of the device
 *      (IN/OUT) smac   : SMAC to free (used if idx == ~0)
 *      (IN)            : port
 *      (IN/OUT) idx    : soc layer smac index to free, or ~0 to scan
 *   Returns
 *       BCM_E_EMPTY  : ref count equals zero, HW may be cleared
 *       BCM_E_NONE   : ref count decremented, but still non-zero
 *       BCM_E_*    - failure
 *   Notes
 *     if idx == ~0, <smac> is used to scan the table for a match.
 *                   match idx will be returned
 *     if idx != ~0, the smac found will be copied to <smac>
 */
int
_sbx_gu2_ismac_idx_free(int unit, bcm_mac_t smac, bcm_port_t port, uint32 *smacIdx)
{
    int rv;
    _sbx_mac_tracker_t* tracker = &_sbx_smac_tracker[unit].ismac;

    if (tracker->macData == NULL) {
        return BCM_E_INIT;
    }
    
    rv = _sbx_mac_tracker_free(tracker, smac, port, smacIdx);
    ALCR_VERB((_SBX_D(unit, "freed ismacIdx for mac: " MAC_FMT 
                      " refCount=%d\n"),
               MAC_PFMT(smac),
               tracker->macData[*smacIdx].refCount));

    return rv;
}

/*
 *   Function
 *      _sbx_gu2_esmac_idx_alloc
 *   Purpose
 *     allocate an egress smac index
 *
 *   Parameters
 *      (IN)  unit   : unit number of the device
 *      (IN)  flags  : reserved or not?
 *      (IN)  smac   : SMAC to allocate
 *      (OUT) idx    : soc layer smac index to use
 *   Returns
 *       BCM_E_NONE - All required resources are allocated
 *       BCM_E_*    - failure
 *   Notes
 *     uses reference counts when the same MAC is allocated.
 *     No hw is ever written 
 */
int
_sbx_gu2_esmac_idx_alloc(int unit, uint32 flags, bcm_mac_t smac,
                         uint32* smacIdx)
{
    int rv;
    _sbx_mac_tracker_t* tracker = &_sbx_smac_tracker[unit].esmac;

    if (tracker->macData == NULL) {
        return BCM_E_INIT;
    }

    /* tracker must be pre-loaded with zero-ref count mac addreses 
     * from hardware before we get here.  ie. shd be done at init
     */
    if (SOC_WARM_BOOT(unit) && (flags & _SBX_GU2_RES_FLAGS_RESERVE)) {
        rv = _sbx_mac_tracker_find(tracker, smac, smacIdx);
        if (BCM_FAILURE(rv)) {
            return rv;
        }
    }

    rv = _sbx_mac_tracker_alloc(tracker, flags, smac, _SBX_GU2_RES_UNUSED_PORT, smacIdx);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    ALCR_VERB((_SBX_D(unit, "allocated esmacIdx for mac: " MAC_FMT 
                      " refCount=%d\n"),
               MAC_PFMT(smac), 
               tracker->macData[*smacIdx].refCount));
    
    return rv;
}

/*
 *   Function
 *      _sbx_gu2_esmac_idx_free
 *   Purpose
 *     free an egress smac index
 *
 *   Parameters
 *      (IN)     unit   : unit number of the device
 *      (IN/OUT) smac   : SMAC to free (used if idx == ~0)
 *      (IN/OUT) idx    : soc layer smac index to free, or ~0 to scan
 *   Returns
 *       BCM_E_EMPTY  : ref count equals zero, HW may be cleared
 *       BCM_E_NONE   : ref count decremented, but still non-zero
 *       BCM_E_*    - failure
 *   Notes
 *     if idx == ~0, <smac> is used to scan the table for a match.
 *                   match idx will be returned
 *     if idx != ~0, the smac found will be copied to <smac>
 */
int
_sbx_gu2_esmac_idx_free(int unit, bcm_mac_t smac, uint32 *smacIdx)
{
    int rv;
    _sbx_mac_tracker_t* tracker = &_sbx_smac_tracker[unit].esmac;

    if (tracker->macData == NULL) {
        return BCM_E_INIT;
    }
    
    rv = _sbx_mac_tracker_free(tracker, smac, _SBX_GU2_RES_UNUSED_PORT, smacIdx);
    ALCR_VERB((_SBX_D(unit, "freed esmacIdx for mac: " MAC_FMT 
                      " refCount=%d\n"),
               MAC_PFMT(smac),
               tracker->macData[*smacIdx].refCount));

    return rv;
}


/*
 *   Function
 *      _sbx_gu2_alloc_range_get
 *   Purpose
 *     Retrieve the range of valid IDs for the given resource
 *
 *   Parameters
 *      (IN)  unit   : unit number of the device
 *      (IN)  type   : resource to get
 *      (OUT) first  : first valid ID
 *      (OUT) last   : last valid ID
 *   Returns
 *       BCM_E_NONE - All required resources are allocated
 *       BCM_E_*    - failure
 *   Notes
 */
int _sbx_gu2_alloc_range_get(int unit, _sbx_gu2_usr_res_types_t type,
                             uint32_t *first, uint32_t *last)
{
    _sbx_gu2_res_handle_t      *res_handle;
    _sbx_gu2_hw_res_attrs_t    *res_attrs;
    _sbx_gu2_hw_res_t           res;
    int rv = BCM_E_NONE;
    uint32_t validLow, validHigh, freeCount, allocCount, junk;

    if (type >= SBX_GU2K_USR_RES_MAX) {
        return BCM_E_PARAM;
    }

    res = _g_mapped_hw_res[type];

    BCM_IF_ERROR_RETURN(_SBX_GU2_RESOURCE_LOCK(unit));

    res_handle = &_g_sbx_gu2_res_handles[unit][res];
    res_attrs  =  &_g_sbx_gu2_res_attrs[res];

    if (res_attrs->alloc_style == SBX_GU2K_ALLOC_STYLE_VERSATILE) {
        rv = shr_aidxres_list_state(res_handle->u.handle_aidx,
                                    first, last, 
                                    &validLow, &validHigh, 
                                    &freeCount, &allocCount, &junk, &junk);
        
    } else if (res_attrs->alloc_style == SBX_GU2K_ALLOC_STYLE_SCALED) {
        rv = shr_idxres_list_state_scaled(res_handle->u.handle_idx,
                                          first, last,
                                          &validLow, &validHigh, 
                                          &freeCount, &allocCount, &junk);
    } else { 
        rv = shr_idxres_list_state(res_handle->u.handle_idx,
                                   first, last, 
                                   &validLow, &validHigh, 
                                   &freeCount, &allocCount);
        
    }

    _SBX_GU2_RESOURCE_UNLOCK(unit);
    return rv;
}


/*
 *   Function
 *      _sbx_gu2_range_reserve
 *   Purpose
 *     Reserve a range of IDs of a given resource
 *
 *   Parameters
 *      (IN)  unit   : unit number of the device
 *      (IN) type    : resource to set
 *      (IN) highOrLow  : TRUE - set Upper bounds
 *                      : FALSE - set lower bounds
 *      (IN) val    : inclusive bound to set
 *   Returns
 *       BCM_E_NONE - All required resources are allocated
 *       BCM_E_*    - failure
 *   Notes
 */
int _sbx_gu2_range_reserve(int unit, _sbx_gu2_usr_res_types_t type,
                           int highOrLow, uint32_t val)
{
    _sbx_gu2_res_handle_t      *res_handle;
    _sbx_gu2_hw_res_attrs_t    *res_attrs;
    _sbx_gu2_hw_res_t           res;
    int rv = BCM_E_NONE;
    int clearIt = 0, reserveIt = 0;
    uint32_t first, last;

    if (type >=  SBX_GU2K_USR_RES_MAX) {
        return BCM_E_PARAM;
    }

    res = _g_mapped_hw_res[type];
    
    BCM_IF_ERROR_RETURN(_SBX_GU2_RESOURCE_LOCK(unit));

    res_handle = &_g_sbx_gu2_res_handles[unit][res];
    res_attrs  =  &_g_sbx_gu2_res_attrs[res];

    first = res_attrs->reservedLow;
    last = res_attrs->reservedHigh;

    /* Zero for any value, high or low, will clear the known range */
    if (val == 0) {
        clearIt = 1;
        res_attrs->reservedLow = 0;
        res_attrs->reservedHigh = 0;
    } else {
        if (highOrLow) {
            res_attrs->reservedHigh = val; 
        } else {
            res_attrs->reservedLow = val;
        }

        if (res_attrs->reservedHigh && res_attrs->reservedLow) {
            if (res_attrs->reservedHigh < res_attrs->reservedLow) {
                ALCR_ERR((_SBX_D(unit, "Upper bounds is set less than lower"
                             " bounds: 0x%x < 0x%x\n"),
                          res_attrs->reservedHigh, res_attrs->reservedLow));

                rv = BCM_E_PARAM;
            } else {
                reserveIt = 1;
            }
        }
    }

    if (reserveIt) {
        
        first = res_attrs->reservedLow;
        last = res_attrs->reservedHigh;

        if (res_attrs->alloc_style == SBX_GU2K_ALLOC_STYLE_VERSATILE) {
            rv = shr_aidxres_list_reserve(res_handle->u.handle_aidx, 
                                          first, last);
        } else if (res_attrs->alloc_style == SBX_GU2K_ALLOC_STYLE_SCALED) {
            rv = shr_idxres_list_reserve(res_handle->u.handle_idx, 
                                         first, last);
        } else { 
            rv = shr_idxres_list_reserve(res_handle->u.handle_idx,
                                         first, last);
        }
        
        ALCR_VERB((_SBX_D(unit, "Reserved resource %s (%d/%d) : "
                                "0x%08x-0x%08x rv=%d %s\n"),
                   _sbx_gu2_resource_to_str(type), type, res, 
                   first, last, rv, bcm_errmsg(rv)));

    } else if (clearIt) {

        if (first && last) {
            int elt, ignoreRv;

            for (elt = first; elt <= last; elt++) {

                if (res_attrs->alloc_style == SBX_GU2K_ALLOC_STYLE_VERSATILE) {
                    ignoreRv = 
                        shr_aidxres_list_free(res_handle->u.handle_aidx, elt);
                    
                } else if (res_attrs->alloc_style == SBX_GU2K_ALLOC_STYLE_SCALED) {
                    ignoreRv =
                        shr_idxres_list_free(res_handle->u.handle_idx, elt);
                } else { 
                    ignoreRv = shr_idxres_list_free(res_handle->u.handle_idx, 
                                                    elt);
                }

                if (BCM_FAILURE(ignoreRv)) {
                    ALCR_VERB((_SBX_D(unit, "failed to free element "
                                      "0x%08x  rv=%d %s (ignored)\n"),
                               elt, ignoreRv, bcm_errmsg(ignoreRv)));
                }
            } 

            ALCR_VERB((_SBX_D(unit, "Freed reserved resource %s (%d/%d) ids: "
                              "0x%08x-0x%08x\n"),
                       _sbx_gu2_resource_to_str(type), type, res, 
                       first, last));
        }
    }
    
    _SBX_GU2_RESOURCE_UNLOCK(unit);
    return rv;
}



/*
 *   Function
 *      _sbx_gu2_get_global_fte_type
 *   Purpose
 *     Get Global FTE GPORT type
 *
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) ftidx  : FTIndex
 *      (OUT)type   : FT GPORT type
 *   Returns
 *       BCM_E_NONE - success
 *       BCM_E_*    - failure
 */
int _sbx_gu2_get_global_fte_type(int unit, uint32 ftidx, uint8 *type)
{

    if(ftidx < SBX_GLOBAL_GPORT_FTE_BASE(unit) || 
       ftidx > SBX_GLOBAL_GPORT_FTE_END(unit)) {
        ALCR_ERR(("Bad Global FTIDX [0%x]: Valid Range[0x%x - 0x%x]",
                  SBX_GLOBAL_GPORT_FTE_BASE(unit), SBX_GLOBAL_GPORT_FTE_END(unit)));
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(_SBX_GU2_RESOURCE_LOCK(unit));

    *type = _sbx_global_ftType[unit][ftidx - SBX_GLOBAL_GPORT_FTE_BASE(unit)];
    ALCR_VERB(("Get Global FTE[0x%x] Type = %u", ftidx, *type));

    _SBX_GU2_RESOURCE_UNLOCK(unit);
    return BCM_E_NONE;
}

/*
 *   Function
 *      _sbx_gu2_set_global_fte_type
 *   Purpose
 *     Set Global FTE GPORT type
 *
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) ftidx  : FTIndex
 *      (OUT)type   : FT GPORT type
 *   Returns
 *       BCM_E_NONE - success
 *       BCM_E_*    - failure
 */
int _sbx_gu2_set_global_fte_type(int unit, uint32 ftidx, uint8 type)
{

    if(ftidx < SBX_GLOBAL_GPORT_FTE_BASE(unit) || 
       ftidx > SBX_GLOBAL_GPORT_FTE_END(unit)) {
        ALCR_ERR(("Bad Global FTIDX [0%x]: Valid Range[0x%x - 0x%x]",
                  SBX_GLOBAL_GPORT_FTE_BASE(unit), SBX_GLOBAL_GPORT_FTE_END(unit)));
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(_SBX_GU2_RESOURCE_LOCK(unit));

    _sbx_global_ftType[unit][ftidx - SBX_GLOBAL_GPORT_FTE_BASE(unit)] = type;
    ALCR_VERB(("SET Global FTE[0x%x] Type = %u", ftidx, type));

    _SBX_GU2_RESOURCE_UNLOCK(unit);
    return BCM_E_NONE;
}


/*
 *   Function
 *      _sbx_gu2_get_vsi_type
 *   Purpose
 *     Get Type of the VSI
 *
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) ftidx  : VSI
 *      (OUT)type   : VSI GPORT type
 *   Returns
 *       BCM_E_NONE - success
 *       BCM_E_*    - failure
 */
int _sbx_gu2_get_vsi_type(int unit, uint32 vsi, uint8 *type)
{
    if(vsi < SBX_DYNAMIC_VSI_BASE(unit) || 
       vsi > SBX_DYNAMIC_VSI_END(unit)) {
        ALCR_ERR(("Bad VSI [0%x]: Valid Range[0x%x - 0x%x]",
                  SBX_DYNAMIC_VSI_BASE(unit),  SBX_DYNAMIC_VSI_END(unit)));
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(_SBX_GU2_RESOURCE_LOCK(unit));

    *type = _sbx_global_vsiType[unit][vsi - SBX_DYNAMIC_VSI_BASE(unit)];
    ALCR_VERB(("Get VSI[0x%x] Type = %u", vsi, *type));

    _SBX_GU2_RESOURCE_UNLOCK(unit);
    return BCM_E_NONE;
}

/*
 *   Function
 *      _sbx_gu2_set_vsi_type
 *   Purpose
 *     Get Type of the VSI
 *
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) ftidx  : VSI
 *      (OUT)type   : VSI GPORT type
 *   Returns
 *       BCM_E_NONE - success
 *       BCM_E_*    - failure
 */
int _sbx_gu2_set_vsi_type(int unit, uint32 vsi, uint8 type)
{
    if(vsi < SBX_DYNAMIC_VSI_BASE(unit) || 
       vsi > SBX_DYNAMIC_VSI_END(unit)) {
        ALCR_ERR(("Bad VSI [0%x]: Valid Range[0x%x - 0x%x]",
                  SBX_DYNAMIC_VSI_BASE(unit),  SBX_DYNAMIC_VSI_END(unit)));
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(_SBX_GU2_RESOURCE_LOCK(unit));

    _sbx_global_vsiType[unit][vsi - SBX_DYNAMIC_VSI_BASE(unit)] = type;
    ALCR_VERB(("SET VSI[0x%x] Type = %u", vsi, type));

    _SBX_GU2_RESOURCE_UNLOCK(unit);  
    return BCM_E_NONE;
}

/*
 *   Function
 *      _sbx_gu2_alloc_non_gport_lport_allocator_data
 *   Purpose
 *     allocate private logical port tracking state
 *
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) lport  : logical port index to track
 *      (IN) ownwer : creator/owner of logical port
 *   Returns
 *       BCM_E_*
 *   Notes: 
 *     Assumes lock taken by caller
 */
static int
_sbx_gu2_alloc_non_gport_lport_allocator_data(int unit,
                                              uint32 lport, 
                                              _sbx_lp_owner_type_e_t owner,
                                              _sbx_lport_state_t **lpData_ref) 
{
    _sbx_lport_state_t *lpData = NULL;

    if ((lport > SBX_MPLS_LPORT_END) || (lport < SBX_LPORT_BASE)) {
        ALCR_ERR((_SBX_D(unit, "Bad LPORT [0x5x]: Valid Range[0x%x - 0x%x]\n"),
                  lport, SBX_LPORT_BASE, SBX_MPLS_LPORT_END));
        return BCM_E_PARAM;
    } 

    if (owner >= _LP_OWNER_MAX) {
        ALCR_ERR((_SBX_D(unit, "Bad LPORT [0x%x] owner[%d]\n"), lport, owner));
        return BCM_E_PARAM;
    }

    if (lport >= SBX_MPLS_LPORT_BASE && !lpData_ref) {
        ALCR_ERR((_SBX_D(unit, "Bad argument\n"), lport, owner));
        return BCM_E_PARAM;
    }

    /* Allocate lport data */
    lpData = (_sbx_lport_state_t*) sal_alloc(sizeof(*lpData), 
                                             "non_gport_lport");
    if (lpData == NULL) {
        ALCR_ERR((_SBX_D(unit, "Failed ot alloc mem for  LPORT[0x%x] Data "
                         "reference\n"), lport));
        return BCM_E_MEMORY; 
    }

    if (lpData) {
        lpData->refCount = 1;
        lpData->owner    = owner;

        if (lport >= SBX_MPLS_LPORT_BASE) {
            *lpData_ref = lpData;
        } else {
            SBX_LPORT_TYPE(unit, lport) = BCM_GPORT_TYPE_SPECIAL;
            SBX_LPORT_DATAPTR(unit, lport) = (void*)lpData;
        }

        ALCR_VERB((_SBX_D(unit, "Owner[%d] Setting up Lport Data: lport[0x%x] "
                          "refcount[%d]\n"),
                   owner, lport, lpData->refCount));
    }

    return BCM_E_NONE;
}


/*
 *   Function
 *      _sbx_gu2_free_non_gport_lport_allocator_data
 *   Purpose
 *     Free private logical port tracking state
 *
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) lport  : logical port index to track
 *      (IN) lpData : data to free
 *   Returns
 *       BCM_E_*
 *   Notes: 
 *     Assumes lock taken by caller, 
 *     private data is no longer referenced (ref count == 0)
 */
static int 
_sbx_gu2_free_non_gport_lport_allocator_data(int unit, 
                                             uint32 lport,
                                             _sbx_lport_state_t *lpData)
{
    if (!lpData) {
        ALCR_ERR((_SBX_D(unit, "Bad LPdata Pointer \n")));
        return BCM_E_PARAM;
    }

    if( (lport > SBX_MPLS_LPORT_END) || (lport < SBX_LPORT_BASE)) {
        ALCR_ERR((_SBX_D(unit, "Bad LPORT [0x%x]: Valid Range[0x%x - 0x%x]"),
                  lport, SBX_LPORT_BASE, SBX_MPLS_LPORT_END));
        return BCM_E_PARAM;
    }

    if (lpData->refCount > 0) {
        ALCR_ERR((_SBX_D(unit, "Reference count for LPORT[0x%x] non zero [%d] "
                         "cannot free\n"),
                  lport, lpData->refCount));
        return BCM_E_INTERNAL;
    }

    ALCR_VERB((_SBX_D(unit, "Clearning State LP 0x%x Refcount=0 owner[%d]\n"),
               lport, lpData->owner));                    
    sal_free(lpData);
    lpData = NULL; /* do not reference lp data after this point */

    if (lport < SBX_MPLS_LPORT_BASE) {
        SBX_LPORT_TYPE(unit, lport) = BCM_GPORT_INVALID;
        SBX_LPORT_DATAPTR(unit, lport) = NULL;
    }
    return BCM_E_NONE;
}


/*
 *   Function
 *      _prv_reference_non_gport_lport_type
 *   Purpose
 *     Add an observer to the private logical port tracking state
 *
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) lport  : logical port index to track
 *      (IN) alloc  : allocate a new reference
 *      (IN) owner  : track original allocator
 *   Returns
 *       BCM_E_*
 *   Notes: 
 *     Assumes lock taken by caller, 
 */
static int
_prv_reference_non_gport_lport_type(int unit, 
                                    uint32 lport,
                                    uint8 alloc, 
                                    _sbx_lp_owner_type_e_t owner)
{

    _sbx_lport_state_t *lpData = NULL;
    if (alloc) {
        /* Verify if no other lport data is associated on lpdata and its type */
        if ((BCM_GPORT_TYPE_SPECIAL == SBX_LPORT_TYPE(unit, lport)) ||
           (SBX_LPORT_DATAPTR(unit, lport))) {
            lpData = ( _sbx_lport_state_t*)SBX_LPORT_DATAPTR(unit, lport);
            if (lpData && (lpData->refCount > 0)) {
                ALCR_ERR((_SBX_D(unit, " LPORT[0x%x] Data has reference"
                                 "internally\n"), 
                          lport));
                return BCM_E_INTERNAL;
            }
        }
        
        return _sbx_gu2_alloc_non_gport_lport_allocator_data(unit, lport, 
                                                             owner, NULL);

    } 

    if (BCM_GPORT_TYPE_SPECIAL != SBX_LPORT_TYPE(unit, lport)) {
        ALCR_ERR((_SBX_D(unit, "Bad LPORT[0x%x] Type [%d]\n"),
                  lport, SBX_LPORT_TYPE(unit, lport)));
        return BCM_E_PARAM;             
    }
    
    lpData = ( _sbx_lport_state_t*)SBX_LPORT_DATAPTR(unit, lport);
    
    if (lpData) {
        lpData->refCount++;
        ALCR_VERB((_SBX_D(unit, "Referenced Lport[0x%x] owner[%d] "
                          "refCount[%d]\n"),
                   lport, lpData->owner, lpData->refCount));
    } else {
        ALCR_ERR(("No LPORT[0x%x] Data to reference\n",lport));
        return  BCM_E_PARAM; 
    }

    return BCM_E_NONE;
}


/*
 *   Function
 *      _prv_dereference_non_gport_lport_type
 *   Purpose
 *     Remove an observer from the private logical port tracking state
 *
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) lport  : logical port index to track
 *      (OUT)refCount: new referecne count after reference removal
 *   Returns
 *       BCM_E_*
 *   Notes: 
 *     Assumes lock taken by caller, 
 */
static int
_prv_dereference_non_gport_lport_type(int unit,
                                      uint32 lport,
                                      uint32 *refCount)
{
    int rv = BCM_E_NONE;
    _sbx_lport_state_t *lpData = NULL;
    
    if (BCM_GPORT_TYPE_SPECIAL != SBX_LPORT_TYPE(unit, lport)) {
        ALCR_ERR((_SBX_D(unit, "Bad LPORT[0x%x] Type [%d]\n"),
                  lport, SBX_LPORT_TYPE(unit, lport)));
        return BCM_E_PARAM; 
    }

    lpData = ( _sbx_lport_state_t*)SBX_LPORT_DATAPTR(unit, lport);
    if (lpData == NULL) {
        ALCR_ERR((_SBX_D(unit, "No LPORT[0x%x] Data to reference\n"), lport));
        return BCM_E_PARAM; 
    }

    if (lpData) {
        if (lpData->refCount <= 0) {
            ALCR_ERR((_SBX_D(unit, "Zero Reference LPORT[0x%x]\n"), lport));
            return BCM_E_INTERNAL;
        } 
        
        lpData->refCount--;
        ALCR_VERB((_SBX_D(unit, "DeReferenced Lport[0x%x] owner[%d] "
                          "refCount[%d]\n"), 
                   lport, lpData->owner, lpData->refCount));

        *refCount = lpData->refCount;

        if (lpData->refCount == 0) {
            rv =  _sbx_gu2_free_non_gport_lport_allocator_data(unit, 
                                                               lport, 
                                                               lpData);
        }
        
        /* Do not access lp data after this point,
         * free might have cleaned it up
         */
        lpData = NULL;
    }

    return rv;
}

/*
 *   Function
 *      _prv_reference_non_mpls_gport_lport_type
 *   Purpose
 *     Add an observer to the private mpls logical port tracking state
 *
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) lport  : mpls logical port index to track
 *      (IN) alloc  : allocate a new reference
 *      (IN) owner  : track original allocator
 *   Returns
 *       BCM_E_*
 *   Notes: 
 *     Assumes lock taken by caller, 
 */
static int
_prv_reference_non_mpls_gport_lport_type(int unit, 
                                    uint32 lport,
                                    uint8 alloc, 
                                    _sbx_lp_owner_type_e_t owner)
{
    int rv = BCM_E_NONE;
    _sbx_lport_state_t *lpData = NULL;

    rv = shr_htb_find(_sbx_mpls_lport[unit], &lport,
                      (shr_htb_data_t *)&lpData,
                      0 /* don't remove */);
    if (alloc && BCM_SUCCESS(rv)) {
        ALCR_ERR((_SBX_D(unit, " LPORT[0x%x] Data has reference"
                         "internally\n"), 
                  lport));
        rv = BCM_E_INTERNAL;

    } else if (alloc) {
        /* verify if the logical port does not exist on hash */
        rv =  _sbx_gu2_alloc_non_gport_lport_allocator_data(unit, lport, 
                                                            owner, &lpData);
        if (BCM_FAILURE(rv)) {
            
        } else {
            /* Insert the lport into mpls hash dbase */
            rv = shr_htb_insert(_sbx_mpls_lport[unit], &lport, lpData);
            if(BCM_FAILURE(rv)) {
                rv = _sbx_gu2_free_non_gport_lport_allocator_data(unit, lport, lpData);
            } else {
                ALCR_VERB((_SBX_D(unit, "MPLS Lport[0x%x] owner[%d] "
                                  "refCount[%d] inserted on database\n"),
                           lport, lpData->owner, lpData->refCount));
            }
        }
    } else {
        /* acquire lport data information */
        if (lpData) {
            /* reference */
            lpData->refCount++;
            ALCR_VERB((_SBX_D(unit, "Referenced Lport[0x%x] owner[%d] "
                              "refCount[%d]\n"),
                       lport, lpData->owner, lpData->refCount));
        } else {
            ALCR_ERR(("No LPORT[0x%x] Data to reference\n",lport));
            rv = BCM_E_PARAM; 
        }
    }
    return rv;
}


/*
 *   Function
 *      _prv_dereference_non_mpls_gport_lport_type
 *   Purpose
 *     Remove an observer from the private logical port tracking state
 *
 *   Parameters
 *      (IN) unit   : unit number of the device
 *      (IN) lport  : mpls logical port index to track
 *      (OUT)refCount: new referecne count after reference removal
 *   Returns
 *       BCM_E_*
 *   Notes: 
 *     Assumes lock taken by caller, 
 */
static int
_prv_dereference_non_mpls_gport_lport_type(int unit,
                                      uint32 lport,
                                      uint32 *refCount)
{
    int rv = BCM_E_NONE;
    _sbx_lport_state_t *lpData = NULL;

    if (!refCount) {
        ALCR_ERR((_SBX_D(unit, "Bad input argument\n"), lport));
        rv = BCM_E_PARAM; 

    } else {
        rv = shr_htb_find(_sbx_mpls_lport[unit], &lport,
                          (shr_htb_data_t *)&lpData,
                          0 /* don't remove */);
        if (!lpData || BCM_FAILURE(rv)) {
            ALCR_ERR((_SBX_D(unit, "No LPORT[0x%x] Data to reference\n"), lport));
            rv = BCM_E_PARAM; 

        } else {
            if (lpData->refCount <= 0) {
                ALCR_ERR((_SBX_D(unit, "Zero Reference LPORT[0x%x]\n"), lport));
                rv = BCM_E_INTERNAL;

            } else {
                lpData->refCount--;
                ALCR_VERB((_SBX_D(unit, "DeReferenced Lport[0x%x] owner[%d] "
                                  "refCount[%d]\n"), 
                           lport, lpData->owner, lpData->refCount));
                
                *refCount = lpData->refCount;
                
                if (lpData->refCount == 0) {
                    /* remote lport from hash databse */
                    rv =  shr_htb_find(_sbx_mpls_lport[unit], &lport,
                                       (shr_htb_data_t *)&lpData,
                                       1 /* remove */);
                    if(BCM_FAILURE(rv)) {
                        ALCR_ERR((_SBX_D(unit, "Failed to free lport[%x] database %d:%s\n"), 
                                  lport, rv, bcm_errmsg(rv)));                    
                    } else {
                        /* free the lp data */
                        rv =  _sbx_gu2_free_non_gport_lport_allocator_data(unit, 
                                                                           lport, 
                                                                           lpData);
                        if(BCM_FAILURE(rv)) {
                            ALCR_ERR((_SBX_D(unit, "Failed to free lport data[%x] %d:%s\n"), 
                                      lport, rv, bcm_errmsg(rv)));                    
                        }
                    }
                }
            }
        }
    }

    return rv;
}


/*
 *   Function
 *     _sbx_gu2_reference_non_gport_lport_type
 *   Purpose
 * Logical ports for TB vlan, ports can be allocated without
 * GPORT creation. The following events will trigger logical port 
 * creation on them
 * [1] Apply policers
 * [2] Apply QOS 
 * [3] Apply Statistics - not yet supported 
 * Reference count will be incremented by each of above cases and the 
 * last revertion will free back the logical port when reference count
 * is zero
 * This function will be invoked to reference, dereference lports
 *
 *   Parameters
 *      (IN) unit   : unit number of the device

 *   Returns
 *       BCM_E_NONE - success
 *       BCM_E_*    - failure
 */
int 
_sbx_gu2_reference_non_gport_lport_type(int unit, 
                                        uint32 lport,
                                        uint8 alloc, 
                                        _sbx_lp_owner_type_e_t owner)
{
    int rv;

    if ((lport > SBX_MPLS_LPORT_END) || (lport < SBX_LPORT_BASE)) {
        ALCR_ERR((_SBX_D(unit, "Bad LPORT [0x%x]: Valid Range[0x%x - 0x%x]\n"),
                  lport, SBX_LPORT_BASE, SBX_MPLS_LPORT_END));
        return BCM_E_PARAM;

    }

    BCM_IF_ERROR_RETURN(_SBX_GU2_RESOURCE_LOCK(unit));

    if (lport >= SBX_MPLS_LPORT_BASE) {
        rv = _prv_reference_non_mpls_gport_lport_type(unit, lport, alloc, owner);
    } else {
        rv = _prv_reference_non_gport_lport_type(unit, lport, alloc, owner);
    }

    _SBX_GU2_RESOURCE_UNLOCK(unit);  

    return rv;
}


int
_sbx_gu2_dereference_non_gport_lport_type(int unit,
                                          uint32 lport,
                                          uint32 *refCount)
{
    int rv;

    if (refCount == NULL) {
        ALCR_ERR((_SBX_D(unit, "Bad NULL Refcount\n")));
        return BCM_E_PARAM;
    }
    
    if ((lport > SBX_MPLS_LPORT_END) || (lport < SBX_LPORT_END)) {
        ALCR_ERR((_SBX_D(unit, "Bad LPORT [0x%x]: Valid Range[0x%x - 0x%x]\n"),
                  lport, SBX_LPORT_BASE, SBX_MPLS_LPORT_END));
        return  BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(_SBX_GU2_RESOURCE_LOCK(unit));

    if (lport >= SBX_MPLS_LPORT_BASE) {
        rv = _prv_dereference_non_mpls_gport_lport_type(unit, lport, refCount);
    } else {
        rv = _prv_dereference_non_gport_lport_type(unit, lport, refCount);
    }

    _SBX_GU2_RESOURCE_UNLOCK(unit);  

    return rv;
}


int 
_sbx_gu2_resource_alloc_counters(int unit,
                                 uint32 *counterId, 
                                 int type,
                                 uint8 numCounter)
{
    int rv = BCM_E_NONE;

    if (!counterId || type >= SOC_SBX_G2P3_COUNTER_MAX_ID || numCounter == 0) {
        ALCR_ERR((_SBX_D(unit, "Bad NULL CounterId Pointer or"
                         " Bad counter type[0x%x]\n"), type));
        return BCM_E_PARAM;
    } 
    
    *counterId = 0;
    rv = _bcm_fe2000_stat_block_alloc(unit, type, counterId, numCounter);
    if (BCM_FAILURE(rv)) {
        ALCR_ERR((_SBX_D(unit, "Failed to Allocate Type[%d] Counters "
                         "rv=%d %s\n"),
                  type, rv, bcm_errmsg(rv)));
        return rv;
    }

    ALCR_VERB((_SBX_D(unit, "Allocated Type[%d] Counter ID[%d]"
                      "Number of Counters[0x%x]\n"),
               type, *counterId, numCounter));            
    return rv;
}


int
_sbx_gu2_resource_free_counters(int unit,
                                uint32 counterId,
                                int type)
{
    int rv = BCM_E_NONE;

    if (counterId==0 || type >= SOC_SBX_G2P3_COUNTER_MAX_ID) {
        ALCR_ERR((_SBX_D(unit, "Bad CounterId Pointer or Bad counter"
                         " type[%d]\n"), type));
        return  BCM_E_PARAM;
    }

    rv = _bcm_fe2000_stat_block_free(unit, type, counterId);
    if (BCM_FAILURE(rv)) {
        ALCR_ERR((_SBX_D(unit, "Failed to Free Type[%d] Counters "
                         "rv=%d %s\n"),
                  type, rv, bcm_errmsg(rv)));

    }

    ALCR_VERB((_SBX_D(unit, "Freed Type[%d] Counter ID[0x%x]\n"), 
               type, counterId)); 

    return rv;
}
