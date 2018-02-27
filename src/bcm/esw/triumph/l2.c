/*
 * $Id: l2.c 1.202.2.3 Broadcom SDK $
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
 * File:        l2.c
 * Purpose:     Triumph L2 function implementations
 */

#include <soc/defs.h>

#if defined(BCM_TRX_SUPPORT)

#include <assert.h>

#include <sal/core/libc.h>

#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/cm.h>
#include <soc/drv.h>
#include <soc/register.h>
#include <soc/memory.h>
#include <soc/hash.h>
#include <soc/l2x.h>
#include <soc/triumph.h>

#include <bcm/l2.h>
#include <bcm/error.h>

#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/l2.h>
#include <bcm_int/esw/firebolt.h>
#include <bcm_int/esw/trx.h>
#include <bcm_int/esw/triumph.h>
#include <bcm_int/esw/mpls.h>
#include <bcm_int/esw/mim.h>
#include <bcm_int/esw/stack.h>
#include <bcm_int/common/multicast.h>
#include <bcm_int/esw/multicast.h>
#include <bcm_int/esw/virtual.h>
#include <bcm_int/esw_dispatch.h>
#include <bcm_int/esw/trident.h>

#define DEFAULT_L2DELETE_CHUNKS		64	/* 16k entries / 64 = 256 */

typedef struct _bcm_mac_block_info_s {
    bcm_pbmp_t mb_pbmp;
    int ref_count;
} _bcm_mac_block_info_t;

static _bcm_mac_block_info_t *_mbi_entries[BCM_MAX_NUM_UNITS];
static int _mbi_num[BCM_MAX_NUM_UNITS];

#define L2_DEBUG(flags, stuff)  BCM_DEBUG(flags | BCM_DBG_L2, stuff)
#define L2_ERR(stuff)           L2_DEBUG(BCM_DBG_ERR, stuff)
#define L2_WARN(stuff)          L2_DEBUG(BCM_DBG_WARN, stuff)
#if defined(BCM_SCORPION_SUPPORT) && defined(INCLUDE_L3)
extern int16 * _sc_subport_group_index[BCM_MAX_NUM_UNITS];
#define _SC_SUBPORT_NUM_PORT  (4096)
#define _SC_SUBPORT_NUM_GROUP (4096/8)
#define _SC_SUBPORT_VPG_FIND(unit, vp, grp) \
    do { \
         int ix; \
         grp = -1; \
         for (ix = 0; ix < _SC_SUBPORT_NUM_GROUP; ix++) { \
              if (_sc_subport_group_index[unit][ix] == vp) { \
                  grp = ix * 8; \
                  break;  \
              } \
         } \
       } while ((0))
#endif /* BCM_SCORPION_SUPPORT && INCLUDE_L3 */
#ifdef PLISIM
#ifdef BCM_KATANA_SUPPORT
extern void _bcm_kt_enable_port_age_simulation(uint32 flags, _bcm_l2_replace_t *rep_st);
#endif
#endif

/*
 * Function:
 *      _bcm_tr_l2_from_l2x
 * Purpose:
 *      Convert a Triumph L2X entry to an L2 API data structure
 * Parameters:
 *      unit        Unit number
 *      l2addr      (OUT) L2 API data structure
 *      l2x_entry   Triumph L2X entry
 */
int
_bcm_tr_l2_from_l2x(int unit, bcm_l2_addr_t *l2addr, l2x_entry_t *l2x_entry)
{
    int l2mc_index, mb_index, vfi;

    sal_memset(l2addr, 0, sizeof(*l2addr));

    soc_L2Xm_mac_addr_get(unit, l2x_entry, MAC_ADDRf, l2addr->mac);

    if (soc_L2Xm_field32_get(unit, l2x_entry, KEY_TYPEf) ==
        TR_L2_HASH_KEY_TYPE_VFI) {
        vfi = soc_L2Xm_field32_get(unit, l2x_entry, VFIf);
        /* VPLS or MIM VPN */
#if defined(INCLUDE_L3)
        if (_bcm_vfi_used_get(unit, vfi, _bcmVfiTypeMpls)) {
            _BCM_MPLS_VPN_SET(l2addr->vid, _BCM_MPLS_VPN_TYPE_VPLS, vfi);
        } else {
            _BCM_MIM_VPN_SET(l2addr->vid, _BCM_MIM_VPN_TYPE_MIM, vfi);
        }
#endif
    } else {
        l2addr->vid = soc_L2Xm_field32_get(unit, l2x_entry, VLAN_IDf);
    }

    if (BCM_MAC_IS_MCAST(l2addr->mac)) {
        l2addr->flags |= BCM_L2_MCAST;
        l2mc_index = soc_L2Xm_field32_get(unit, l2x_entry, DESTINATIONf);
        if (soc_mem_field_valid(unit, L2Xm, VPG_TYPEf) &&
            soc_L2Xm_field32_get(unit, l2x_entry, VPG_TYPEf)) {
            if (soc_L2Xm_field32_get(unit, l2x_entry, KEY_TYPEf) ==
                                               TR_L2_HASH_KEY_TYPE_VFI) {
                vfi = soc_L2Xm_field32_get(unit, l2x_entry, VFIf);
                /* VPLS or MIM multicast */
#if defined(INCLUDE_L3)
                if (_bcm_vfi_used_get(unit, vfi, _bcmVfiTypeMpls)) {
                    _BCM_MULTICAST_GROUP_SET(l2addr->l2mc_index,
                            _BCM_MULTICAST_TYPE_VPLS, l2mc_index);
                } else {
                    _BCM_MULTICAST_GROUP_SET(l2addr->l2mc_index,
                            _BCM_MULTICAST_TYPE_MIM, l2mc_index);
                }
#endif
            } else {
#if defined(INCLUDE_L3)
                int rv;
                rv = _bcm_tr_multicast_ipmc_group_type_get(unit,
                            l2mc_index, &l2addr->l2mc_index);
                if (BCM_E_NOT_FOUND == rv) {
                    /* Assume subport multicast */
                    _BCM_MULTICAST_GROUP_SET(l2addr->l2mc_index,
                            _BCM_MULTICAST_TYPE_SUBPORT, l2mc_index);
                } else if (BCM_FAILURE(rv)) {
                    return rv;
                }
#endif
            }
        } else {
            l2addr->l2mc_index = l2mc_index;
        }
    } else {
        _bcm_gport_dest_t       dest;
        int                     isGport = 0;

        _bcm_gport_dest_t_init(&dest);
#if defined(INCLUDE_L3)
        if (soc_L2Xm_field32_get(unit, l2x_entry, DEST_TYPEf) == 2) {
            int vp;
            vp = soc_L2Xm_field32_get(unit, l2x_entry, DESTINATIONf);
            if (soc_L2Xm_field32_get(unit, l2x_entry, KEY_TYPEf) ==
                                     TR_L2_HASH_KEY_TYPE_VFI) {
                /* MPLS/MiM virtual port unicast */
                if (_bcm_vp_used_get(unit, vp, _bcmVpTypeMpls)) {
                    dest.mpls_id = vp;
                    dest.gport_type = _SHR_GPORT_TYPE_MPLS_PORT;
                    isGport=1;
                } else if (_bcm_vp_used_get(unit, vp, _bcmVpTypeMim)) {
                    dest.mim_id = vp;
                    dest.gport_type = _SHR_GPORT_TYPE_MIM_PORT;
                    isGport=1;
                } else {
                    return BCM_E_INTERNAL; /* Cannot reach here */
                }
            } else {
                /* Subport/WLAN unicast */
#if defined(BCM_SCORPION_SUPPORT) && defined(INCLUDE_L3)
                if (SOC_IS_SC_CQ(unit)) {
                   /* Scorpion uses index to L3_NEXT_HOP as VPG */
                   int grp;

                   _SC_SUBPORT_VPG_FIND(unit, vp, grp);
                   if ((vp = grp) == -1) {
                        L2_ERR(("Unit: %d can not find entry for VPG\n", unit));
                   }
                   dest.subport_id = vp;
                   dest.gport_type = _SHR_GPORT_TYPE_SUBPORT_GROUP;
                   isGport=1;
              } else
#endif /* BCM_SCORPION_SUPPORT && INCLUDE_L3 */
              {
                if (_bcm_vp_used_get(unit, vp, _bcmVpTypeSubport)) {
                    dest.subport_id = vp;
                    dest.gport_type = _SHR_GPORT_TYPE_SUBPORT_GROUP;
                    isGport=1;
                } else if (_bcm_vp_used_get(unit, vp, _bcmVpTypeWlan)) {
                    dest.wlan_id = vp;
                    dest.gport_type = _SHR_GPORT_TYPE_WLAN_PORT;
                    isGport=1;
                } else if (_bcm_vp_used_get(unit, vp, _bcmVpTypeVlan)) {
                    dest.vlan_vp_id = vp;
                    dest.gport_type = _SHR_GPORT_TYPE_VLAN_PORT;
                    isGport=1;
                } else if (_bcm_vp_used_get(unit, vp, _bcmVpTypeNiv)) {
                    dest.vlan_vp_id = vp;
                    dest.gport_type = _SHR_GPORT_TYPE_NIV_PORT;
                    isGport=1;
                } else if (_bcm_vp_used_get(unit, vp, _bcmVpTypeTrill)) {
                    dest.trill_id  = vp;
                    dest.gport_type = _SHR_GPORT_TYPE_TRILL_PORT;
                    isGport=1;
                } else {
                    return BCM_E_INTERNAL; /* Cannot reach here */
                }
              }
            }
        } else
#endif /* INCLUDE_L3 */
        if (soc_L2Xm_field32_get(unit, l2x_entry, Tf)) {
            /* Trunk group */
            l2addr->flags |= BCM_L2_TRUNK_MEMBER;
            l2addr->tgid = soc_L2Xm_field32_get(unit, l2x_entry, TGIDf);
            bcm_esw_trunk_psc_get(unit, l2addr->tgid, &(l2addr->rtag));
            if (soc_L2Xm_field32_get(unit, l2x_entry, REMOTE_TRUNKf)) {
                l2addr->flags |= BCM_L2_REMOTE_TRUNK;
            }
            dest.tgid = l2addr->tgid;
            dest.gport_type = _SHR_GPORT_TYPE_TRUNK;
            BCM_IF_ERROR_RETURN(
                bcm_esw_switch_control_get(unit, bcmSwitchUseGport, &isGport));
        } else {
            bcm_module_t    mod_in, mod_out;
            bcm_port_t      port_in, port_out;

            port_in = soc_L2Xm_field32_get(unit, l2x_entry, PORT_NUMf);
            mod_in = soc_L2Xm_field32_get(unit, l2x_entry, MODULE_IDf);
            BCM_IF_ERROR_RETURN
                (_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_GET,
                                        mod_in, port_in, &mod_out, &port_out));
            l2addr->modid = mod_out;
            l2addr->port = port_out;
            dest.port = l2addr->port;
            dest.modid = l2addr->modid;
            dest.gport_type = _SHR_GPORT_TYPE_MODPORT;
            BCM_IF_ERROR_RETURN(
                bcm_esw_switch_control_get(unit, bcmSwitchUseGport, &isGport));
        }

        if (isGport) {
            BCM_IF_ERROR_RETURN(
                _bcm_esw_gport_construct(unit, &dest, &(l2addr->port)));
        }
    }

    if (SOC_MEM_FIELD_VALID(unit, L2Xm, L3f)) {
        if (soc_L2Xm_field32_get(unit, l2x_entry, L3f)) {
            l2addr->flags |= BCM_L2_L3LOOKUP;
        }
    }

    if (SOC_CONTROL(unit)->l2x_group_enable) {
        l2addr->group = soc_L2Xm_field32_get(unit, l2x_entry, CLASS_IDf);
    } else {
        mb_index = soc_L2Xm_field32_get(unit, l2x_entry, MAC_BLOCK_INDEXf);
        if (mb_index) {
            BCM_PBMP_ASSIGN(l2addr->block_bitmap,
                            _mbi_entries[unit][mb_index].mb_pbmp);
        }
    }

    if (soc_L2Xm_field32_get(unit, l2x_entry, RPEf)) {
        l2addr->flags |= BCM_L2_SETPRI;
    }

    l2addr->cos_dst = soc_L2Xm_field32_get(unit, l2x_entry, PRIf);
    l2addr->cos_src = soc_L2Xm_field32_get(unit, l2x_entry, PRIf);

    if (soc_L2Xm_field32_get(unit, l2x_entry, CPUf)) {
        l2addr->flags |= BCM_L2_COPY_TO_CPU;
    }

    if (soc_L2Xm_field32_get(unit, l2x_entry, DST_DISCARDf)) {
        l2addr->flags |= BCM_L2_DISCARD_DST;
    }

    if (soc_L2Xm_field32_get(unit, l2x_entry, SRC_DISCARDf)) {
        l2addr->flags |= BCM_L2_DISCARD_SRC;
    }

    if (soc_L2Xm_field32_get(unit, l2x_entry, SCPf)) {
        l2addr->flags |= BCM_L2_COS_SRC_PRI;
    }


    if (soc_L2Xm_field32_get(unit, l2x_entry, STATIC_BITf)) {
        l2addr->flags |= BCM_L2_STATIC;
    }

    if (SOC_MEM_FIELD_VALID(unit, L2Xm, LIMIT_COUNTEDf)) {
        if (!(l2addr->flags & (BCM_L2_L3LOOKUP | BCM_L2_MCAST |
                               BCM_L2_STATIC | BCM_L2_LEARN_LIMIT))) {
            if (!soc_L2Xm_field32_get(unit, l2x_entry, LIMIT_COUNTEDf)) {
                l2addr->flags |= BCM_L2_LEARN_LIMIT_EXEMPT;
            }
        }
    }

    if (soc_L2Xm_field32_get(unit, l2x_entry, PENDINGf)) {
        l2addr->flags |= BCM_L2_PENDING;
    }

    if (soc_L2Xm_field32_get(unit, l2x_entry, HITDAf)) {
        l2addr->flags |= BCM_L2_DES_HIT;
        l2addr->flags |= BCM_L2_HIT;
    }

    if (soc_L2Xm_field32_get(unit, l2x_entry, HITSAf)) {
        l2addr->flags |= BCM_L2_SRC_HIT;
        l2addr->flags |= BCM_L2_HIT;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_tr_l2_to_l2x
 * Purpose:
 *      Convert an L2 API data structure to a Triumph L2X entry
 * Parameters:
 *      unit        Unit number
 *      l2x_entry   (OUT) Triumph L2X entry
 *      l2addr      L2 API data structure
 *      key_only    Only construct key portion
 */
int
_bcm_tr_l2_to_l2x(int unit, l2x_entry_t *l2x_entry, bcm_l2_addr_t *l2addr,
                  int key_only)
{
    if (l2addr->cos_dst < 0 || l2addr->cos_dst > 15) {
        return BCM_E_PARAM;
    }

    /*  BCM_L2_MIRROR is not supported starting from Triumph */
    if (l2addr->flags & BCM_L2_MIRROR) {
        return BCM_E_PARAM;
    }

    sal_memset(l2x_entry, 0, sizeof (*l2x_entry));

    if (_BCM_MPLS_VPN_IS_VPLS(l2addr->vid)) {
        soc_L2Xm_field32_set(unit, l2x_entry, VFIf,
                             _BCM_MPLS_VPN_ID_GET(l2addr->vid));
        soc_L2Xm_field32_set(unit, l2x_entry, KEY_TYPEf,
                             TR_L2_HASH_KEY_TYPE_VFI);
    } else if (_BCM_IS_MIM_VPN(l2addr->vid)) {
        soc_L2Xm_field32_set(unit, l2x_entry, VFIf,
                             _BCM_MIM_VPN_ID_GET(l2addr->vid));
        soc_L2Xm_field32_set(unit, l2x_entry, KEY_TYPEf,
                             TR_L2_HASH_KEY_TYPE_VFI);
    } else {
         if (!_BCM_MPLS_VPN_IS_VPWS(l2addr->vid)) {
              VLAN_CHK_ID(unit, l2addr->vid);
              soc_L2Xm_field32_set(unit, l2x_entry, VLAN_IDf, l2addr->vid);
              soc_L2Xm_field32_set(unit, l2x_entry, KEY_TYPEf,
                             TR_L2_HASH_KEY_TYPE_BRIDGE);
         }
    }

    soc_L2Xm_mac_addr_set(unit, l2x_entry, MAC_ADDRf, l2addr->mac);

    if (key_only) {
        return BCM_E_NONE;
    }

    if (BCM_MAC_IS_MCAST(l2addr->mac)) {
        if (_BCM_MULTICAST_IS_VPLS(l2addr->l2mc_index) ||
            _BCM_MULTICAST_IS_MIM(l2addr->l2mc_index) ||
            _BCM_MULTICAST_IS_WLAN(l2addr->l2mc_index) ||
            _BCM_MULTICAST_IS_VLAN(l2addr->l2mc_index) ||
            _BCM_MULTICAST_IS_NIV(l2addr->l2mc_index) ||
            _BCM_MULTICAST_IS_SUBPORT(l2addr->l2mc_index)) {
            soc_L2Xm_field32_set(unit, l2x_entry, DEST_TYPEf, 0x3);
        }

        if (_BCM_MULTICAST_IS_SET(l2addr->l2mc_index)) {
            soc_L2Xm_field32_set(unit, l2x_entry, DESTINATIONf,
                                 _BCM_MULTICAST_ID_GET(l2addr->l2mc_index));
        } else {
            soc_L2Xm_field32_set(unit, l2x_entry, L2MC_PTRf, l2addr->l2mc_index);
        }
    } else {
        bcm_port_t      port = -1;
        bcm_trunk_t     tgid = BCM_TRUNK_INVALID;
        bcm_module_t    modid = -1;
        int             gport_id = -1;
        int             vpg_type = 0;

        if (BCM_GPORT_IS_SET(l2addr->port)) {
            _bcm_l2_gport_params_t  g_params;

            if (BCM_GPORT_IS_BLACK_HOLE(l2addr->port)) {
                soc_L2Xm_field32_set(unit, l2x_entry, SRC_DISCARDf, 1);
            } else {
                soc_L2Xm_field32_set(unit, l2x_entry, SRC_DISCARDf, 0);
                BCM_IF_ERROR_RETURN(
                        _bcm_esw_l2_gport_parse(unit, l2addr, &g_params));

                switch (g_params.type) {
                    case _SHR_GPORT_TYPE_TRUNK:
                        tgid = g_params.param0;
                        break;
                    case  _SHR_GPORT_TYPE_MODPORT:
                        port = g_params.param0;
                        modid = g_params.param1;
                        break;
                    case _SHR_GPORT_TYPE_LOCAL_CPU:
                        port = g_params.param0;
                        BCM_IF_ERROR_RETURN(
                                bcm_esw_stk_my_modid_get(unit, &modid));
                        break;
                    case _SHR_GPORT_TYPE_SUBPORT_GROUP:
                        gport_id = g_params.param0;
#if defined(BCM_SCORPION_SUPPORT) && defined(INCLUDE_L3)
                        if (SOC_IS_SC_CQ(unit)) {
                            /* Map the sub_port to index to L3_NEXT_HOP */
                            gport_id = (int) _sc_subport_group_index[unit][gport_id/8];
                        }
#endif /* BCM_SCORPION_SUPPORT && INCLUDE_L3 */
                        vpg_type = 1;
                        break;
                case _SHR_GPORT_TYPE_SUBPORT_PORT:
                    gport_id = g_params.param0;
                    vpg_type = 1;
                    break;
                    case _SHR_GPORT_TYPE_MPLS_PORT:
                    gport_id = g_params.param0;
                    vpg_type = 1;
                    break;
                    case _SHR_GPORT_TYPE_MIM_PORT:
                    case _SHR_GPORT_TYPE_WLAN_PORT:
                    case _SHR_GPORT_TYPE_VLAN_PORT:
                    case _SHR_GPORT_TYPE_NIV_PORT:
                        gport_id = g_params.param0;
                        vpg_type = 1;
                        break;
                    case _SHR_GPORT_TYPE_TRILL_PORT:
                        gport_id = g_params.param0;
                        vpg_type = 0;
                        break;
                    default:
                        return BCM_E_PORT;
                }
            }
        } else if (l2addr->flags & BCM_L2_TRUNK_MEMBER) {
            tgid = l2addr->tgid;

        } else {
            BCM_IF_ERROR_RETURN(
                _bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_SET,
                                       l2addr->modid, l2addr->port,
                                       &modid, &port));
            /* Check parameters */
            if (!SOC_MODID_ADDRESSABLE(unit, modid)) {
                return BCM_E_BADID;
            }
            if (!SOC_PORT_ADDRESSABLE(unit, port)) {
                return BCM_E_PORT;
            }
        }

        /* Setting l2x_entry fields according to parameters */
        if ( BCM_TRUNK_INVALID != tgid) {
            soc_L2Xm_field32_set(unit, l2x_entry, Tf, 1);
            soc_L2Xm_field32_set(unit, l2x_entry, TGIDf, tgid);
            /*
             * Note:  RTAG is ignored here.  Use bcm_trunk_psc_set to
             * to set for a given trunk.
             */
            if (l2addr->flags & BCM_L2_REMOTE_TRUNK) {
                soc_L2Xm_field32_set(unit, l2x_entry, REMOTE_TRUNKf, 1);
            }
        } else if (-1 != port) {
            soc_L2Xm_field32_set(unit, l2x_entry, MODULE_IDf, modid);
            soc_L2Xm_field32_set(unit, l2x_entry, PORT_NUMf, port);
        } else if (-1 != gport_id) {
            soc_L2Xm_field32_set(unit, l2x_entry, DEST_TYPEf, 0x2);
            if (vpg_type) {
                soc_L2Xm_field32_set(unit, l2x_entry, VPGf, gport_id);
                soc_L2Xm_field32_set(unit, l2x_entry, VPG_TYPEf, vpg_type);
            } else {
                soc_L2Xm_field32_set(unit, l2x_entry, DESTINATIONf, gport_id);
            }
        }
    }

    if (SOC_MEM_FIELD_VALID(unit, L2Xm, L3f)) {
        if (l2addr->flags & BCM_L2_L3LOOKUP) {
            soc_L2Xm_field32_set(unit, l2x_entry, L3f, 1);
        }
    }

    if (SOC_CONTROL(unit)->l2x_group_enable) {
        soc_L2Xm_field32_set(unit, l2x_entry, CLASS_IDf, l2addr->group);
    } /* else MAC_BLOCK_INDEXf is handled in the add/remove functions below */

    if (l2addr->flags & BCM_L2_SETPRI) {
        soc_L2Xm_field32_set(unit, l2x_entry, RPEf, 1);
    }

    if (l2addr->flags & BCM_L2_COPY_TO_CPU) {
        soc_L2Xm_field32_set(unit, l2x_entry, CPUf, 1);
    }

    if (l2addr->flags & BCM_L2_DISCARD_DST) {
        soc_L2Xm_field32_set(unit, l2x_entry, DST_DISCARDf, 1);
    }

    if (l2addr->flags & BCM_L2_DISCARD_SRC) {
        soc_L2Xm_field32_set(unit, l2x_entry, SRC_DISCARDf, 1);
    }

    if (l2addr->flags & BCM_L2_COS_SRC_PRI) {
        soc_L2Xm_field32_set(unit, l2x_entry, SCPf, 1);
    }

    if (SOC_MEM_FIELD_VALID(unit, L2Xm, LIMIT_COUNTEDf)) {
        if ((!(l2addr->flags & (BCM_L2_L3LOOKUP | BCM_L2_MCAST | BCM_L2_STATIC |
                               BCM_L2_LEARN_LIMIT_EXEMPT))) || 
            l2addr->flags & BCM_L2_LEARN_LIMIT) {
            soc_L2Xm_field32_set(unit, l2x_entry, LIMIT_COUNTEDf, 1);
        }
    }

    if (l2addr->flags & BCM_L2_PENDING) {
        soc_L2Xm_field32_set(unit, l2x_entry, PENDINGf, 1);
    }

    if (l2addr->flags & BCM_L2_STATIC) {
        soc_L2Xm_field32_set(unit, l2x_entry, STATIC_BITf, 1);
    }

    soc_L2Xm_field32_set(unit, l2x_entry, VALIDf, 1);

    if ((l2addr->flags & BCM_L2_DES_HIT) ||
        (l2addr->flags & BCM_L2_HIT)) {
        soc_L2Xm_field32_set(unit, l2x_entry, HITDAf, 1);
    }

    if ((l2addr->flags & BCM_L2_SRC_HIT) ||
        (l2addr->flags & BCM_L2_HIT)) {
        soc_L2Xm_field32_set(unit, l2x_entry, HITSAf, 1);
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_tr_l2_from_ext_l2
 * Purpose:
 *      Convert an EXT_L2_ENTRY to an L2 API data structure

 * Parameters:
 *      unit         Unit number
 *      l2addr       (OUT) L2 API data structure
 *      ext_l2_entry EXT_L2_ENTRY hardware entry
 */
int
_bcm_tr_l2_from_ext_l2(int unit, bcm_l2_addr_t *l2addr,
                       ext_l2_entry_entry_t *ext_l2_entry)
{
    _bcm_gport_dest_t       dest;
    int                     mb_index, vfi;
    bcm_module_t            mod;
    bcm_port_t              port;
    int  isGport = 0;

    sal_memset(l2addr, 0, sizeof(*l2addr));

     if (soc_mem_field32_get(unit, EXT_L2_ENTRYm, ext_l2_entry,
                             KEY_TYPE_VFIf) == 1) {
         vfi = soc_mem_field32_get(unit, EXT_L2_ENTRYm, ext_l2_entry, VFIf);
         /* VPLS or MIM VPN */
#if defined(INCLUDE_L3)
         if (_bcm_vfi_used_get(unit, vfi, _bcmVfiTypeMpls)) {
              _BCM_MPLS_VPN_SET(l2addr->vid, _BCM_MPLS_VPN_TYPE_VPLS, vfi);
         } else {
              _BCM_MIM_VPN_SET(l2addr->vid, _BCM_MIM_VPN_TYPE_MIM, vfi);
         }
#endif
    } else {
         l2addr->vid = soc_mem_field32_get(unit, EXT_L2_ENTRYm, ext_l2_entry,
                                           VLAN_IDf);
    }

    soc_mem_mac_addr_get(unit, EXT_L2_ENTRYm, ext_l2_entry, MAC_ADDRf,
                         l2addr->mac);

    _bcm_gport_dest_t_init(&dest);
#if defined(INCLUDE_L3)
    if (soc_mem_field32_get(unit, EXT_L2_ENTRYm, ext_l2_entry, DEST_TYPEf) == 0x2) {
         int vp;

         vp = soc_mem_field32_get(unit, EXT_L2_ENTRYm,ext_l2_entry,  DESTINATIONf);
         if (soc_mem_field32_get(unit, EXT_L2_ENTRYm, ext_l2_entry, KEY_TYPE_VFIf) == 0x1) {
              /* MPLS/MiM virtual port unicast */
              if (_bcm_vp_used_get(unit, vp, _bcmVpTypeMpls)) {
                   dest.mpls_id = vp;
                   dest.gport_type = _SHR_GPORT_TYPE_MPLS_PORT;
                   isGport=1;
              } else if (_bcm_vp_used_get(unit, vp, _bcmVpTypeMim)) {
                   dest.mim_id = vp;
                   dest.gport_type = _SHR_GPORT_TYPE_MIM_PORT;
                   isGport=1;
              } else {
                   return BCM_E_INTERNAL; /* Cannot reach here */
              }
         } else {
              /* Subport/WLAN unicast */
              if (_bcm_vp_used_get(unit, vp, _bcmVpTypeSubport)) {
                   dest.subport_id = vp;
                   dest.gport_type = _SHR_GPORT_TYPE_SUBPORT_GROUP;
                   isGport=1;
              } else if (_bcm_vp_used_get(unit, vp, _bcmVpTypeWlan)) {
                   dest.wlan_id = vp;
                   dest.gport_type = _SHR_GPORT_TYPE_WLAN_PORT;
                   isGport=1;
              } else {
                   return BCM_E_INTERNAL; /* Cannot reach here */
              }
         }
    } else {
#endif /* INCLUDE_L3 */
        if (soc_mem_field32_get(unit, EXT_L2_ENTRYm, ext_l2_entry, Tf)) {
            l2addr->tgid = soc_mem_field32_get(unit, EXT_L2_ENTRYm,
                                               ext_l2_entry, TGIDf);
            bcm_esw_trunk_psc_get(unit, l2addr->tgid, &l2addr->rtag);
            dest.tgid = l2addr->tgid;
            dest.gport_type = _SHR_GPORT_TYPE_TRUNK;

            l2addr->flags |= BCM_L2_TRUNK_MEMBER;
            if (soc_mem_field32_get(unit, EXT_L2_ENTRYm, ext_l2_entry,
                                    REMOTE_TRUNKf)) {
                l2addr->flags |= BCM_L2_REMOTE_TRUNK;
            }
        } else {
            mod = soc_mem_field32_get(unit, EXT_L2_ENTRYm, ext_l2_entry,
                                      MODULE_IDf);
            port = soc_mem_field32_get(unit, EXT_L2_ENTRYm, ext_l2_entry,
                                       PORT_NUMf);
            BCM_IF_ERROR_RETURN(_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_GET,
                                                       mod, port, &mod, &port));
            l2addr->modid = mod;
            l2addr->port = port;
            dest.port = l2addr->port;
            dest.modid = l2addr->modid;
            dest.gport_type = _SHR_GPORT_TYPE_MODPORT;
        }
        BCM_IF_ERROR_RETURN(bcm_esw_switch_control_get(unit, bcmSwitchUseGport,
                                                      &isGport));
#if defined(INCLUDE_L3)
    }
#endif /* INCLUDE_L3 */
	
    if (isGport) {
         BCM_IF_ERROR_RETURN
             (_bcm_esw_gport_construct(unit, &dest, &l2addr->port));
    }

    if (SOC_CONTROL(unit)->l2x_group_enable) {
        l2addr->group = soc_mem_field32_get(unit, EXT_L2_ENTRYm, ext_l2_entry,
                                            CLASS_IDf);
    } else {
        mb_index = soc_mem_field32_get(unit, EXT_L2_ENTRYm, ext_l2_entry,
                                       MAC_BLOCK_INDEXf);
        if (mb_index) {
            BCM_PBMP_ASSIGN(l2addr->block_bitmap,
                            _mbi_entries[unit][mb_index].mb_pbmp);
        }
    }

    if (soc_mem_field32_get(unit, EXT_L2_ENTRYm, ext_l2_entry, RPEf)) {
        l2addr->flags |= BCM_L2_SETPRI;
    }

    l2addr->cos_dst = soc_mem_field32_get(unit, EXT_L2_ENTRYm, ext_l2_entry,
                                          PRIf);
    l2addr->cos_src = l2addr->cos_dst;

    if (soc_mem_field32_get(unit, EXT_L2_ENTRYm, ext_l2_entry, CPUf)) {
        l2addr->flags |= BCM_L2_COPY_TO_CPU;
    }

    if (soc_mem_field32_get(unit, EXT_L2_ENTRYm, ext_l2_entry, DST_DISCARDf)) {
        l2addr->flags |= BCM_L2_DISCARD_DST;
    }

    if (soc_mem_field32_get(unit, EXT_L2_ENTRYm, ext_l2_entry, SRC_DISCARDf)) {
        l2addr->flags |= BCM_L2_DISCARD_SRC;
    }

    if (soc_mem_field32_get(unit, EXT_L2_ENTRYm, ext_l2_entry, SCPf)) {
        l2addr->flags |= BCM_L2_COS_SRC_PRI;
    }

    if (soc_mem_field32_get(unit, EXT_L2_ENTRYm, ext_l2_entry, STATIC_BITf)) {
        l2addr->flags |= BCM_L2_STATIC;
    }

    if (soc_mem_field32_get(unit, EXT_L2_ENTRYm, ext_l2_entry, DST_HITf)) {
        l2addr->flags |= BCM_L2_DES_HIT;
        l2addr->flags |= BCM_L2_HIT;
    }

    if (soc_mem_field32_get(unit, EXT_L2_ENTRYm, ext_l2_entry, SRC_HITf)) {
        l2addr->flags |= BCM_L2_SRC_HIT;
        l2addr->flags |= BCM_L2_HIT;
    }

    if (SOC_MEM_FIELD_VALID(unit, EXT_L2_ENTRYm, LIMIT_COUNTEDf)) {
        if ((!(l2addr->flags & BCM_L2_STATIC)) || 
            l2addr->flags & BCM_L2_LEARN_LIMIT ) {
            if (!soc_L2Xm_field32_get(unit, ext_l2_entry, LIMIT_COUNTEDf)) {
                l2addr->flags |= BCM_L2_LEARN_LIMIT_EXEMPT;
            }
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_tr_l2_to_ext_l2
 * Purpose:
 *      Convert an L2 API data structure to an EXT_L2_ENTRY
 * Parameters:
 *      unit         Unit number
 *      ext_l2_entry (OUT) EXT_L2_ENTRY hardware entry
 *      l2addr       L2 API data structure
 *      key_only     Only construct key portion
 */
int
_bcm_tr_l2_to_ext_l2(int unit, ext_l2_entry_entry_t *ext_l2_entry,
                     bcm_l2_addr_t *l2addr, int key_only)
{
    _bcm_l2_gport_params_t  g_params;
    bcm_module_t            mod;
    bcm_port_t              port;
    uint32                  fval;

    /*  BCM_L2_MIRROR is not supported starting from Triumph */
    if (l2addr->flags & BCM_L2_MIRROR) {
        return BCM_E_PARAM;
    }

    sal_memset(ext_l2_entry, 0, sizeof(*ext_l2_entry));

    if (_BCM_MPLS_VPN_IS_VPLS(l2addr->vid)) {
        fval = _BCM_MPLS_VPN_ID_GET(l2addr->vid);
        if (!SOC_MEM_FIELD32_VALUE_FIT(unit, EXT_L2_ENTRYm, VFIf, fval)) {
            return BCM_E_PARAM;
        }
        soc_mem_field32_set(unit, EXT_L2_ENTRYm, ext_l2_entry, VFIf, fval);
        soc_mem_field32_set(unit, EXT_L2_ENTRYm, ext_l2_entry, KEY_TYPE_VFIf,
                            1);
    } else if (_BCM_IS_MIM_VPN(l2addr->vid)) {
        fval = _BCM_MIM_VPN_ID_GET(l2addr->vid);
        if (!SOC_MEM_FIELD32_VALUE_FIT(unit, EXT_L2_ENTRYm, VFIf, fval)) {
            return BCM_E_PARAM;
        }
        soc_mem_field32_set(unit, EXT_L2_ENTRYm, ext_l2_entry, VFIf, fval);
        soc_mem_field32_set(unit, EXT_L2_ENTRYm, ext_l2_entry, KEY_TYPE_VFIf,
                            1);
    } else {
        if (!SOC_MEM_FIELD32_VALUE_FIT(unit, EXT_L2_ENTRYm, VLAN_IDf,
                                       l2addr->vid)) {
            return BCM_E_PARAM;
        }
        soc_mem_field32_set(unit, EXT_L2_ENTRYm, ext_l2_entry, VLAN_IDf,
                            l2addr->vid);
    }

    soc_mem_mac_addr_set(unit, EXT_L2_ENTRYm, ext_l2_entry, MAC_ADDRf,
                         l2addr->mac);

    if (key_only) {
        return BCM_E_NONE;
    }

    if (!BCM_GPORT_IS_SET(l2addr->port)) {
        if (l2addr->flags & BCM_L2_TRUNK_MEMBER) {
            g_params.param0 = l2addr->tgid;
            g_params.type = _SHR_GPORT_TYPE_TRUNK;
        } else {
            PORT_DUALMODID_VALID(unit, l2addr->port);
            BCM_IF_ERROR_RETURN(_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_SET,
                                                       l2addr->modid, l2addr->port,
                                                       &mod, &port));
            g_params.param0 = port;
            g_params.param1 = mod;
            g_params.type = _SHR_GPORT_TYPE_MODPORT;
        }
    } else {
        BCM_IF_ERROR_RETURN(_bcm_esw_l2_gport_parse(unit, l2addr, &g_params));
    }
	
    switch (g_params.type) {
    case _SHR_GPORT_TYPE_TRUNK:
        if (!SOC_MEM_FIELD32_VALUE_FIT(unit, EXT_L2_ENTRYm, TGIDf,
                                       g_params.param0)) {
            return BCM_E_PARAM;
        }
        soc_mem_field32_set(unit, EXT_L2_ENTRYm, ext_l2_entry, Tf, 1);
        soc_mem_field32_set(unit, EXT_L2_ENTRYm, ext_l2_entry, TGIDf,
                            g_params.param0);
        if (l2addr->flags & BCM_L2_REMOTE_TRUNK) {
            soc_mem_field32_set(unit, EXT_L2_ENTRYm, ext_l2_entry,
                                REMOTE_TRUNKf, 1);
        }
        break;
    case _SHR_GPORT_TYPE_MODPORT:
        if (!SOC_MODID_ADDRESSABLE(unit, g_params.param1)) {
            return BCM_E_BADID;
        }
        if (!SOC_PORT_ADDRESSABLE(unit, g_params.param0)) {
            return BCM_E_PORT;
        }
        soc_mem_field32_set(unit, EXT_L2_ENTRYm, ext_l2_entry, MODULE_IDf,
                            g_params.param1);
        soc_mem_field32_set(unit, EXT_L2_ENTRYm, ext_l2_entry, PORT_NUMf,
                            g_params.param0);
        break;
    case _SHR_GPORT_TYPE_LOCAL_CPU:
        if (!SOC_PORT_ADDRESSABLE(unit, g_params.param0)) {
            return BCM_E_PORT;
        }
        soc_mem_field32_set(unit, EXT_L2_ENTRYm, ext_l2_entry, PORT_NUMf,
                            g_params.param0);
        BCM_IF_ERROR_RETURN(bcm_esw_stk_my_modid_get(unit, &mod));
        soc_mem_field32_set(unit, EXT_L2_ENTRYm, ext_l2_entry, MODULE_IDf,
                            mod);
        break;
    case _SHR_GPORT_TYPE_MPLS_PORT:
        if (!SOC_MEM_FIELD32_VALUE_FIT(unit, EXT_L2_ENTRYm, DESTINATIONf,
                                       g_params.param0)) {
            return BCM_E_PARAM;
        }
        soc_mem_field32_set(unit, EXT_L2_ENTRYm, ext_l2_entry, DESTINATIONf,
                            g_params.param0);
        soc_mem_field32_set(unit, EXT_L2_ENTRYm, ext_l2_entry, DEST_TYPEf, 0x2);
        break;
    case _SHR_GPORT_TYPE_MIM_PORT:
        if (!SOC_MEM_FIELD32_VALUE_FIT(unit, EXT_L2_ENTRYm, VPGf,
                                       g_params.param0)) {
            return BCM_E_PARAM;
        }
        soc_mem_field32_set(unit, EXT_L2_ENTRYm, ext_l2_entry, VPGf,
                            g_params.param0);
        soc_mem_field32_set(unit, EXT_L2_ENTRYm, ext_l2_entry, VPG_TYPEf, 1);
        break;
    case _SHR_GPORT_TYPE_SUBPORT_GROUP:
        soc_mem_field32_set(unit, EXT_L2_ENTRYm, ext_l2_entry, VPGf,
                            g_params.param0);
        soc_mem_field32_set(unit, EXT_L2_ENTRYm, ext_l2_entry, VPG_TYPEf, 1);
        break;
    case _SHR_GPORT_TYPE_SUBPORT_PORT:
        soc_mem_field32_set(unit, EXT_L2_ENTRYm, ext_l2_entry, VPGf,
                            g_params.param0);
        soc_mem_field32_set(unit, EXT_L2_ENTRYm, ext_l2_entry, VPG_TYPEf, 1);
        break;
    default:
        return BCM_E_PORT;
    }

    if (SOC_CONTROL(unit)->l2x_group_enable) {
        soc_mem_field32_set(unit, EXT_L2_ENTRYm, ext_l2_entry, CLASS_IDf,
                            l2addr->group);
    } /* else MAC_BLOCK_INDEXf is handled in the add/remove functions */

    if (l2addr->flags & BCM_L2_SETPRI) {
        soc_mem_field32_set(unit, EXT_L2_ENTRYm, ext_l2_entry, RPEf, 1);
    }

    if (!SOC_MEM_FIELD32_VALUE_FIT(unit, EXT_L2_ENTRYm, PRIf,
                                   l2addr->cos_dst)) {
        return BCM_E_PARAM;
    }
    soc_mem_field32_set(unit, EXT_L2_ENTRYm, ext_l2_entry, PRIf,
                        l2addr->cos_dst);

    if (l2addr->flags & BCM_L2_COPY_TO_CPU) {
        soc_mem_field32_set(unit, EXT_L2_ENTRYm, ext_l2_entry, CPUf, 1);
    }

    if (l2addr->flags & BCM_L2_DISCARD_DST) {
        soc_mem_field32_set(unit, EXT_L2_ENTRYm, ext_l2_entry, DST_DISCARDf,
                            1);
    }

    if (l2addr->flags & BCM_L2_DISCARD_SRC) {
        soc_mem_field32_set(unit, EXT_L2_ENTRYm, ext_l2_entry, SRC_DISCARDf,
                            1);
    }

    if (l2addr->flags & BCM_L2_COS_SRC_PRI) {
        soc_mem_field32_set(unit, EXT_L2_ENTRYm, ext_l2_entry, SCPf, 1);
    }

    if (l2addr->flags & BCM_L2_STATIC) {
        soc_mem_field32_set(unit, EXT_L2_ENTRYm, ext_l2_entry, STATIC_BITf, 1);
    }

    soc_mem_field32_set(unit, EXT_L2_ENTRYm, ext_l2_entry, VALIDf, 1);

    if ((l2addr->flags & BCM_L2_DES_HIT) ||
        (l2addr->flags & BCM_L2_HIT)) {
        soc_mem_field32_set(unit, EXT_L2_ENTRYm, ext_l2_entry, DST_HITf, 1);
    }

    if ((l2addr->flags & BCM_L2_SRC_HIT) ||
        (l2addr->flags & BCM_L2_HIT)) {
        soc_mem_field32_set(unit, EXT_L2_ENTRYm, ext_l2_entry, SRC_HITf, 1);
    }

    if (SOC_MEM_FIELD_VALID(unit, EXT_L2_ENTRYm, LIMIT_COUNTEDf)) {
        if ((!(l2addr->flags & (BCM_L2_L3LOOKUP | BCM_L2_MCAST | BCM_L2_STATIC |
                               BCM_L2_LEARN_LIMIT_EXEMPT))) || 
            l2addr->flags & BCM_L2_LEARN_LIMIT) {
            soc_mem_field32_set(unit, EXT_L2_ENTRYm, ext_l2_entry,
                                LIMIT_COUNTEDf, 1);
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_tr_compose_ext_l2_entry
 * Description:
 *      Compose ext_l2_entry from its tcam portion, data portion, and hit bit
 *      Hardware does not support read and write to ext_l2_entry view.
 * Parameters:
 *      unit         Device number
 *      tcam_entry   TCAM portion of ESM L2 entry (ext_l2_entry_tcam_entry_t)
 *      data_entry   DATA portion of ESM L2 entry (ext_l2_entry_data_entry_t)
 *      src_hit      SRC_HIT field value
 *      dst_hit      DST_HIT field value
 *      ext_l2_entry (OUT) Buffer to store the composed ext_l2_entry_entry_t
 *                   result
 * Return:
 *      BCM_E_XXX.
 */
int
_bcm_tr_compose_ext_l2_entry(int unit,
                             ext_l2_entry_tcam_entry_t *tcam_entry,
                             ext_l2_entry_data_entry_t *data_entry,
                             int src_hit,
                             int dst_hit,
                             ext_l2_entry_entry_t *ext_l2_entry)
{
    sal_mac_addr_t      mac;
    uint32              fval;
    uint32              fbuf[2];

    if (tcam_entry == NULL || data_entry == NULL || ext_l2_entry == NULL) {
        return BCM_E_PARAM;
    }

    sal_memset(ext_l2_entry, 0, sizeof(ext_l2_entry_entry_t));

    /******************** Values from TCAM *******************************/
    fval = soc_mem_field32_get(unit, EXT_L2_ENTRY_TCAMm, tcam_entry, VLAN_IDf);
    soc_mem_field32_set(unit, EXT_L2_ENTRYm, ext_l2_entry, VLAN_IDf, fval);

    soc_mem_mac_addr_get(unit, EXT_L2_ENTRY_TCAMm, tcam_entry, MAC_ADDRf, mac);
    soc_mem_mac_addr_set(unit, EXT_L2_ENTRYm, ext_l2_entry, MAC_ADDRf, mac);

    fval = soc_mem_field32_get(unit, EXT_L2_ENTRY_TCAMm, tcam_entry,
                               KEY_TYPE_VFIf);
    soc_mem_field32_set(unit, EXT_L2_ENTRYm, ext_l2_entry, KEY_TYPE_VFIf,
                        fval);

    /******************** Values from DATA *******************************/
    soc_mem_field_get(unit, EXT_L2_ENTRY_DATAm, (uint32 *)data_entry,
                      AD_EXT_L2f, fbuf);
    soc_mem_field_set(unit, EXT_L2_ENTRYm, (uint32 *)ext_l2_entry, AD_EXT_L2f,
                      fbuf);

    fval = soc_mem_field32_get(unit, EXT_L2_ENTRY_DATAm, data_entry, VALIDf);
    soc_mem_field32_set(unit, EXT_L2_ENTRYm, ext_l2_entry, VALIDf, fval);

    /******************** Hit Bits *******************************/
    soc_mem_field32_set(unit, EXT_L2_ENTRYm, ext_l2_entry, DST_HITf, dst_hit);
    soc_mem_field32_set(unit, EXT_L2_ENTRYm, ext_l2_entry, SRC_HITf, src_hit);

    return BCM_E_NONE;
}

/*
 * function:
 *     _bcm_tr_l2_traverse_mem
 * Description:
 *      Helper function to _bcm_esw_l2_traverse to itterate over given memory
 *      and actually read the table and parse entries for Triumph external
 *      memory
 * Parameters:
 *     unit         device number
 *      mem         External L2 memory to read
 *     trav_st      Traverse structure with all the data.
 * Return:
 *     BCM_E_XXX
 */
#ifdef BCM_TRIUMPH_SUPPORT
int
_bcm_tr_l2_traverse_mem(int unit, soc_mem_t mem, _bcm_l2_traverse_t *trav_st)
{
    _soc_tr_l2e_ppa_info_t    *ppa_info;
    ext_l2_entry_entry_t      ext_l2_entry;
    ext_l2_entry_tcam_entry_t tcam_entry;
    ext_l2_entry_data_entry_t data_entry;
    ext_src_hit_bits_l2_entry_t src_hit_entry;
    ext_dst_hit_bits_l2_entry_t dst_hit_entry;
    int                       src_hit, dst_hit;
    int                       idx, idx_max;

    if (mem != EXT_L2_ENTRYm) {
        return BCM_E_UNAVAIL;
    }

    if (!soc_mem_index_count(unit, mem)) {
        return BCM_E_NONE;
    }

    ppa_info = SOC_CONTROL(unit)->ext_l2_ppa_info;
    if (ppa_info == NULL) {
        return BCM_E_NONE;
    }

    idx_max = soc_mem_index_max(unit, mem);
    for (idx = soc_mem_index_min(unit, mem); idx <= idx_max; idx++ ) {
        if (!(ppa_info[idx].data & _SOC_TR_L2E_VALID)) {
            continue;
        }
        SOC_IF_ERROR_RETURN
            (soc_mem_read(unit, EXT_L2_ENTRY_TCAMm, MEM_BLOCK_ANY, idx,
                          &tcam_entry));
        SOC_IF_ERROR_RETURN
            (soc_mem_read(unit, EXT_L2_ENTRY_DATAm, MEM_BLOCK_ANY, idx,
                          &data_entry));
        SOC_IF_ERROR_RETURN
            (soc_mem_read(unit, EXT_SRC_HIT_BITS_L2m, MEM_BLOCK_ANY, idx >> 5,
                          &src_hit_entry));
        SOC_IF_ERROR_RETURN
            (soc_mem_read(unit, EXT_DST_HIT_BITS_L2m, MEM_BLOCK_ANY, idx >> 5,
                          &dst_hit_entry));
        src_hit = (soc_mem_field32_get
                   (unit, EXT_SRC_HIT_BITS_L2m, &src_hit_entry, SRC_HITf) >>
                   (idx & 0x1f)) & 1;
        dst_hit = (soc_mem_field32_get
                   (unit, EXT_DST_HIT_BITS_L2m, &dst_hit_entry, DST_HITf) >>
                   (idx & 0x1f)) & 1;
        BCM_IF_ERROR_RETURN
            (_bcm_tr_compose_ext_l2_entry(unit, &tcam_entry, &data_entry,
                                          src_hit, dst_hit, &ext_l2_entry));
        trav_st->data = (uint32 *)&ext_l2_entry;
        trav_st->mem = mem;

        BCM_IF_ERROR_RETURN(trav_st->int_cb(unit, trav_st));
    }

    return BCM_E_NONE;
}
#endif /* BCM_TRIUMPH_SUPPORT */

/*
 * Function:
 *      _bcm_mac_block_insert
 * Purpose:
 *      Find or create a MAC_BLOCK table entry matching the given bitmap.
 * Parameters:
 *      unit - Unit number
 *      mb_pbmp - egress port bitmap for source MAC blocking
 *      mb_index - (OUT) Index of MAC_BLOCK table with bitmap.
 * Returns:
 *      BCM_E_NONE              Success
 *      BCM_E_INTERNAL          Chip access failure
 *      BCM_E_RESOURCE          No more MAC_BLOCK entries available
 *      BCM_E_PARAM             Bad bitmap supplied
 */
static int
_bcm_mac_block_insert(int unit, bcm_pbmp_t mb_pbmp, int *mb_index)
{
    int cur_index = 0;
    _bcm_mac_block_info_t *mbi = _mbi_entries[unit];
    mac_block_entry_t mbe;
    bcm_pbmp_t temp_pbmp;

    /* Check for reasonable pbmp */
    BCM_PBMP_ASSIGN(temp_pbmp, mb_pbmp);
    BCM_PBMP_AND(temp_pbmp, PBMP_ALL(unit));
    if (BCM_PBMP_NEQ(mb_pbmp, temp_pbmp)) {
        return BCM_E_PARAM;
    }

    for (cur_index = 0; cur_index < _mbi_num[unit]; cur_index++) {
        if (BCM_PBMP_EQ(mbi[cur_index].mb_pbmp, mb_pbmp)) {
            mbi[cur_index].ref_count++;
            *mb_index = cur_index;
            return BCM_E_NONE;
        }
    }

    /* Not in table already, see if any space free */
    for (cur_index = 1; cur_index < _mbi_num[unit]; cur_index++) {
        if (mbi[cur_index].ref_count == 0) {
            /* Attempt insert */
            sal_memset(&mbe, 0, sizeof(mac_block_entry_t));

            if (soc_mem_field_valid(unit, MAC_BLOCKm, MAC_BLOCK_MASK_LOf)) {
                soc_MAC_BLOCKm_field32_set(unit, &mbe, MAC_BLOCK_MASK_LOf,
                                           SOC_PBMP_WORD_GET(mb_pbmp, 0));
            } else if (soc_mem_field_valid(unit, MAC_BLOCKm,
                                           MAC_BLOCK_MASKf)) {
                soc_mem_pbmp_field_set(unit, MAC_BLOCKm, &mbe, MAC_BLOCK_MASKf,
                                       &mb_pbmp); 
            } else {
                return BCM_E_INTERNAL;
            }
            if (soc_mem_field_valid(unit, MAC_BLOCKm, MAC_BLOCK_MASK_HIf)) {
                soc_MAC_BLOCKm_field32_set(unit, &mbe, MAC_BLOCK_MASK_HIf,
                                           SOC_PBMP_WORD_GET(mb_pbmp, 1));
            }
            SOC_IF_ERROR_RETURN(WRITE_MAC_BLOCKm(unit, MEM_BLOCK_ALL,
                                                 cur_index, &mbe));
            mbi[cur_index].ref_count++;
            BCM_PBMP_ASSIGN(mbi[cur_index].mb_pbmp, mb_pbmp);
            *mb_index = cur_index;
            return BCM_E_NONE;
        }
    }

    /* Didn't find a free slot, out of table space */
    return BCM_E_RESOURCE;
}

/*
 * Function:
 *      _bcm_mac_block_delete
 * Purpose:
 *      Remove reference to MAC_BLOCK table entry matching the given bitmap.
 * Parameters:
 *      unit - Unit number
 *      mb_index - Index of MAC_BLOCK table with bitmap.
 */
static void
_bcm_mac_block_delete(int unit, int mb_index)
{
    if (_mbi_entries[unit][mb_index].ref_count > 0) {
        _mbi_entries[unit][mb_index].ref_count--;
    } else if (mb_index) {
        
        /* Someone reran init without flushing the L2 table */
    } /* else mb_index = 0, as expected for learning */
}

#ifdef BCM_TRIUMPH_SUPPORT
STATIC int
_bcm_tr_l2e_ppa_match(int unit, _bcm_l2_replace_t *rep_st)
{
    _soc_tr_l2e_ppa_info_t      *ppa_info;
    _soc_tr_l2e_ppa_vlan_t      *ppa_vlan;
    int                         i, imin, imax, rv, nmatches;
    soc_mem_t                   mem;
    uint32                      entdata, entmask, entvalue, newdata, newmask;
    ext_l2_entry_entry_t        l2entry, old_l2entry;
    int                         same_dest;

    ppa_info = SOC_CONTROL(unit)->ext_l2_ppa_info;
    ppa_vlan = SOC_CONTROL(unit)->ext_l2_ppa_vlan;
    if (ppa_info == NULL) {
        return BCM_E_NONE;
    }

    mem = EXT_L2_ENTRYm;
    imin = soc_mem_index_min(unit, mem);
    imax = soc_mem_index_max(unit, mem);

    /* convert match data */
    entdata = _SOC_TR_L2E_VALID;
    entmask = _SOC_TR_L2E_VALID;
    if (!(rep_st->flags & BCM_L2_REPLACE_MATCH_STATIC)) {
        entdata |= 0x00000000;
        entmask |= _SOC_TR_L2E_STATIC;
    }
    if (rep_st->flags & BCM_L2_REPLACE_MATCH_VLAN) {
        entdata |= (rep_st->key_vlan & _SOC_TR_L2E_VLAN_MASK) <<
            _SOC_TR_L2E_VLAN_SHIFT;
        entmask |= _SOC_TR_L2E_VLAN_MASK << _SOC_TR_L2E_VLAN_SHIFT;
        imin = ppa_vlan->vlan_min[rep_st->key_vlan];
        imax = ppa_vlan->vlan_max[rep_st->key_vlan];
    }
    if (rep_st->flags & BCM_L2_REPLACE_MATCH_DEST) {
        if (rep_st->match_dest.trunk != -1) {
            entdata |= _SOC_TR_L2E_T |
                ((rep_st->match_dest.trunk & _SOC_TR_L2E_TRUNK_MASK)
                 << _SOC_TR_L2E_TRUNK_SHIFT);
            entmask |= _SOC_TR_L2E_T |
                (_SOC_TR_L2E_TRUNK_MASK << _SOC_TR_L2E_TRUNK_SHIFT);
        } else {
            entdata |= 
                ((rep_st->match_dest.module & _SOC_TR_L2E_MOD_MASK) <<
                 _SOC_TR_L2E_MOD_SHIFT) |
                ((rep_st->match_dest.port & _SOC_TR_L2E_PORT_MASK) <<
                 _SOC_TR_L2E_PORT_SHIFT);
            entmask |= _SOC_TR_L2E_T |
                (_SOC_TR_L2E_MOD_MASK << _SOC_TR_L2E_MOD_SHIFT) |
                (_SOC_TR_L2E_PORT_MASK << _SOC_TR_L2E_PORT_SHIFT);
        }
    }

    nmatches = 0;

    if (imin >= 0) {
        for (i = imin; i <= imax; i++) {
            entvalue = ppa_info[i].data;
            if ((entvalue & entmask) != entdata) {
                continue;
            }
            if (rep_st->flags & BCM_L2_REPLACE_MATCH_MAC) {
                if (ENET_CMP_MACADDR(rep_st->key_mac, ppa_info[i].mac)) {
                    continue;
                }
            }
            nmatches += 1;

            /* lookup the matched entry */
            sal_memset(&l2entry, 0, sizeof(l2entry));
            soc_mem_field32_set(unit, mem, &l2entry, VLAN_IDf,
                                (entvalue >> 16) & 0xfff);
            soc_mem_mac_addr_set(unit, mem, &l2entry, MAC_ADDRf,
                                 ppa_info[i].mac);

            /* operate on matched entry */
            if (rep_st->flags & BCM_L2_REPLACE_DELETE) {
                int             mb_index;

                rv = soc_mem_generic_delete(unit, mem, MEM_BLOCK_ANY, 0,
                                            &l2entry, &l2entry, NULL);
                if (rv < 0) {
                    return rv;
                }
                if (!SOC_CONTROL(unit)->l2x_group_enable) {
                    mb_index = soc_mem_field32_get(unit, mem, &l2entry,
                                                   MAC_BLOCK_INDEXf);
                    _bcm_mac_block_delete(unit, mb_index);
                }
                if (entvalue & _SOC_TR_L2E_LIMIT_COUNTED) {
                    rv = soc_triumph_learn_count_update(unit, &l2entry,
                                                        TRUE, -1);
                }
            } else {
                /* replace destination fields */
                rv = soc_mem_generic_lookup(unit, mem, MEM_BLOCK_ANY, 0,
                                            &l2entry, &l2entry, NULL);
                if (rep_st->flags & BCM_L2_REPLACE_NEW_TRUNK) {
                    newdata = _SOC_TR_L2E_T |
                        ((rep_st->new_dest.trunk & _SOC_TR_L2E_TRUNK_MASK) <<
                         _SOC_TR_L2E_TRUNK_SHIFT);
                    newmask = _SOC_TR_L2E_T |
                        (_SOC_TR_L2E_TRUNK_MASK << _SOC_TR_L2E_TRUNK_SHIFT);
                    soc_mem_field32_set(unit, mem, &l2entry, Tf, 1);
                    soc_mem_field32_set(unit, mem, &l2entry, TGIDf,
                                        rep_st->new_dest.trunk);
                } else {
                    newdata =
                        (rep_st->new_dest.module << _SOC_TR_L2E_MOD_SHIFT) |
                        (rep_st->new_dest.port << _SOC_TR_L2E_PORT_SHIFT);
                    newmask = _SOC_TR_L2E_T |
                        (_SOC_TR_L2E_MOD_MASK << _SOC_TR_L2E_MOD_SHIFT) |
                        (_SOC_TR_L2E_PORT_MASK << _SOC_TR_L2E_PORT_SHIFT);
                    soc_mem_field32_set(unit, mem, &l2entry, MODULE_IDf,
                                        rep_st->new_dest.module);
                    soc_mem_field32_set(unit, mem, &l2entry, PORT_NUMf,
                                        rep_st->new_dest.port);
                }
                same_dest = !((entvalue ^ newdata) & newmask);

                if ((entvalue & _SOC_TR_L2E_LIMIT_COUNTED) && !same_dest) {
                    rv = soc_triumph_learn_count_update(unit, &l2entry, FALSE,
                                                        1);
                    if (SOC_FAILURE(rv)) {
                        return rv;
                    }
                }

                /* re-insert entry */
                rv = soc_mem_generic_insert(unit, mem, MEM_BLOCK_ANY, 0,
                                            &l2entry, &old_l2entry, NULL);
                if (rv == BCM_E_EXISTS) {
                    rv = BCM_E_NONE;
                }
                if (rv < 0) {
                    return rv;
                }
                if ((entvalue & _SOC_TR_L2E_LIMIT_COUNTED) && !same_dest) {
                    rv = soc_triumph_learn_count_update(unit, &old_l2entry,
                                                        FALSE, -1);
                    if (SOC_FAILURE(rv)) {
                        return rv;
                    }
                }
            }
        }
    }
    soc_cm_debug(DK_VERBOSE,
                 "tr_l2e_ppa_match: imin=%d imax=%d nmatches=%d flags=0x%x\n",
                 imin, imax, nmatches, rep_st->flags);
    return BCM_E_NONE;
}
#endif /* BCM_TRIUMPH_SUPPORT */

/*
 * Function:
 *      _bcm_l2_hash_dynamic_replace
 * Purpose:
 *      Replace dynamic L2 entries in a dual hash.
 * Parameters:
 *      unit - Unit number
 *      l2x_entry - Entry to insert instead of dynamic entry.
 * Returns:
 *      BCM_E_NONE              Success
 *      BCM_E_XXX          		Error 
 */

STATIC int 
_bcm_l2_hash_dynamic_replace(int unit, l2x_entry_t *l2x_entry)
{
    l2x_entry_t     l2ent;
    uint8           key[XGS_HASH_KEY_SIZE];
    uint32          hash_control;
    int             cf_hit, cf_unhit;
    int             hash_select, bucket, slot, num_bits;
    int             hash_select2, bucket2 = -1;
    int             l2_index, rv; 
    bcm_mac_t       mac;


    BCM_IF_ERROR_RETURN(READ_HASH_CONTROLr(unit, &hash_control)); 
    hash_select = soc_reg_field_get(unit, HASH_CONTROLr, hash_control,
                                    L2_AND_VLAN_MAC_HASH_SELECTf);

    num_bits = soc_tr_l2x_base_entry_to_key(unit, l2x_entry, key);
    bucket = soc_tr_l2x_hash(unit, hash_select, num_bits, l2x_entry, key);

    BCM_IF_ERROR_RETURN(READ_L2_AUX_HASH_CONTROLr(unit, &hash_control));

    if (soc_reg_field_get(unit, L2_AUX_HASH_CONTROLr,
                          hash_control, ENABLEf)) {
        hash_select2 = soc_reg_field_get(unit, L2_AUX_HASH_CONTROLr, 
                                         hash_control, HASH_SELECTf);
        bucket2 = soc_tr_l2x_hash(unit, hash_select2, num_bits, l2x_entry, key);
    } else {
        bucket2 = bucket; 
    }
    
    BCM_IF_ERROR_RETURN(soc_l2x_freeze(unit));

    cf_hit = cf_unhit = -1;
    for (slot = 0; slot < SOC_L2X_BUCKET_SIZE; slot++) {
        if ((bucket2 >=0) && (slot == (SOC_L2X_BUCKET_SIZE/2))) {
            bucket = bucket2;
        }
        l2_index = bucket * SOC_L2X_BUCKET_SIZE + slot;
        rv = soc_mem_read(unit, L2Xm, MEM_BLOCK_ANY, l2_index, &l2ent);
        if ((rv < 0)) {
            (void) soc_l2x_thaw(unit);
            return rv;
        }

        if (!soc_L2Xm_field32_get(unit, &l2ent, VALIDf)) {
            /* Found invalid entry - stop the search victim found */
            cf_unhit = l2_index; 
            break;
        } else {
            if ((soc_L2Xm_field32_get(unit, &l2ent, KEY_TYPEf) != 
                        TR_L2_HASH_KEY_TYPE_BRIDGE) &&
                (soc_L2Xm_field32_get(unit, &l2ent, KEY_TYPEf) != 
                        TR_L2_HASH_KEY_TYPE_VFI)) {
                continue;
            }

            soc_L2Xm_mac_addr_get(unit, &l2ent, MAC_ADDRf, mac);
            /* Skip static entries */
            if ((soc_L2Xm_field32_get(unit, &l2ent, STATIC_BITf)) ||
                (BCM_MAC_IS_MCAST(mac)) ||
                (soc_mem_field_valid(unit, L2Xm, L3f) && 
                 soc_L2Xm_field32_get(unit, &l2ent, L3f))) {
                continue;
            }
            if (soc_L2Xm_field32_get(unit, &l2ent, HITDAf) || 
                soc_L2Xm_field32_get(unit, &l2ent, HITSAf) ) {
                cf_hit =  l2_index;
            } else {
                /* Found unhit entry - stop search victim found */
                cf_unhit = l2_index;
                break;
            }
        }
    }

    if (cf_unhit >= 0) {
        l2_index = cf_unhit;   /* take last unhit dynamic */
    } else if (cf_hit >= 0) {
        l2_index = cf_hit;     /* or last hit dynamic */
    } else {
        rv = BCM_E_FULL;     /* no dynamics to delete */
        (void) soc_l2x_thaw(unit);
         return rv;
    }

    rv = soc_mem_delete_index(unit, L2Xm, MEM_BLOCK_ALL, l2_index);
    if (SOC_SUCCESS(rv)) {
        rv = soc_mem_write(unit, L2Xm, MEM_BLOCK_ALL, l2_index, l2x_entry);
    }
    if (SOC_FAILURE(rv)) {
        (void) soc_l2x_thaw(unit);
        return rv;
    }

    return soc_l2x_thaw(unit);
}

/*
 * Function:
 *      bcm_tr_l2_addr_add
 * Description:
 *      Add a MAC address to the Switch Address Resolution Logic (ARL)
 *      port with the given VLAN ID and parameters.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      l2addr - Pointer to bcm_l2_addr_t containing all valid fields
 * Returns:
 *      BCM_E_NONE              Success
 *      BCM_E_INTERNAL          Chip access failure
 *      BCM_E_RESOURCE          No MAC_BLOCK entries available
 * Notes:
 *      Use CMIC_PORT(unit) to associate the entry with the CPU.
 *      Use flag of BCM_L2_COPY_TO_CPU to send a copy to the CPU.
 *      Use flag of BCM_L2_TRUNK_MEMBER to set trunking (TGID must be
 *      passed as well with non-zero trunk group ID)
 *      In case the L2X table is full (e.g. bucket full), an attempt
 *      will be made to store the entry in the L2_USER_ENTRY table.
 */
int
bcm_tr_l2_addr_add(int unit, bcm_l2_addr_t *l2addr)
{
    l2x_entry_t  l2x_entry, l2x_lookup;
#ifdef BCM_TRIUMPH_SUPPORT
    _soc_tr_l2e_ppa_info_t *ppa_info;
    ext_l2_entry_entry_t ext_l2_entry, ext_l2_lookup;
    int          exist_in_ext_l2, same_dest, update_limit, limit_counted;
    int          rv1, ext_l2_index;
#endif /* BCM_TRIUMPH_SUPPORT */
    int          enable_ppa_war;
    int          rv, l2_index, mb_index = 0;

#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_IS_TRIDENT(unit) || (SOC_IS_KATANA(unit))) {
        if (soc_mem_is_valid(unit, MY_STATION_TCAMm) &&
            (l2addr->flags & BCM_L2_L3LOOKUP)) {
            BCM_IF_ERROR_RETURN(bcm_td_l2_myStation_add(unit, l2addr));
        }
    }
#endif /* BCM_TRIDENT_SUPPORT */

    BCM_IF_ERROR_RETURN(_bcm_tr_l2_to_l2x(unit, &l2x_entry, l2addr, FALSE));

#ifdef BCM_TRIUMPH_SUPPORT
    exist_in_ext_l2 = FALSE;
    ppa_info = SOC_CONTROL(unit)->ext_l2_ppa_info;
    if (soc_mem_is_valid(unit, EXT_L2_ENTRYm) &&
        soc_mem_index_count(unit, EXT_L2_ENTRYm) > 0) {
        same_dest = FALSE;
        update_limit =
            (l2addr->flags & (BCM_L2_STATIC | BCM_L2_LEARN_LIMIT_EXEMPT) && 
             !(l2addr->flags & BCM_L2_LEARN_LIMIT)) ? FALSE : TRUE;
        limit_counted = FALSE;
        BCM_IF_ERROR_RETURN
            (_bcm_tr_l2_to_ext_l2(unit, &ext_l2_entry, l2addr, FALSE));
        soc_mem_lock(unit, EXT_L2_ENTRYm);
        rv1 = soc_mem_generic_lookup(unit, EXT_L2_ENTRYm, MEM_BLOCK_ANY, 0,
                                     &ext_l2_entry, &ext_l2_lookup,
                                     &ext_l2_index);
        if (SOC_SUCCESS(rv1)) {
            exist_in_ext_l2 = TRUE;
        } else if (rv1 != SOC_E_NOT_FOUND) {
            soc_mem_unlock(unit, EXT_L2_ENTRYm);
            return rv1;
        }

        if (!(l2addr->flags & (BCM_L2_L3LOOKUP | BCM_L2_MCAST |
                               BCM_L2_PENDING | BCM_L2_STATIC))) {
            if (exist_in_ext_l2) {
                if (soc_mem_field32_get(unit, EXT_L2_ENTRYm, &ext_l2_entry,
                                        DEST_TYPEf) ==
                    soc_mem_field32_get(unit, EXT_L2_ENTRYm, &ext_l2_lookup,
                                        DEST_TYPEf) &&
                    soc_mem_field32_get(unit, EXT_L2_ENTRYm, &ext_l2_entry,
                                        DESTINATIONf) ==
                    soc_mem_field32_get(unit, EXT_L2_ENTRYm, &ext_l2_lookup,
                                        DESTINATIONf)) {
                    same_dest = TRUE;
                }
                limit_counted =
                    ppa_info[ext_l2_index].data & _SOC_TR_L2E_LIMIT_COUNTED;
            }
            if (update_limit) {
                rv1 = SOC_E_NONE;
                if (!limit_counted) {
                    rv1 = soc_triumph_learn_count_update(unit, &ext_l2_entry,
                                                         TRUE, 1);
                } else if (!same_dest) {
                    rv1 = soc_triumph_learn_count_update(unit, &ext_l2_entry,
                                                         FALSE, 1);
                }
                if (SOC_FAILURE(rv1)) {
                    soc_mem_unlock(unit, EXT_L2_ENTRYm);
                    return rv1;
                }
            }
            if (!SOC_CONTROL(unit)->l2x_group_enable) {
                /* Mac blocking, attempt to associate with bitmap entry */
                rv1 = _bcm_mac_block_insert(unit, l2addr->block_bitmap,
                                           &mb_index);
                if (SOC_FAILURE(rv1)) {
                    soc_mem_unlock(unit, EXT_L2_ENTRYm);
                    return rv1;
                }
                soc_mem_field32_set(unit, EXT_L2_ENTRYm, &ext_l2_entry,
                                    MAC_BLOCK_INDEXf, mb_index);
            }
            rv = soc_mem_generic_insert(unit, EXT_L2_ENTRYm, MEM_BLOCK_ANY, 0,
                                        &ext_l2_entry, &ext_l2_lookup, NULL);
            if (rv == SOC_E_EXISTS) {
                /* entry exists, clear setting for old entry */
                if (!SOC_CONTROL(unit)->l2x_group_enable) {
                    mb_index = soc_mem_field32_get(unit, EXT_L2_ENTRYm,
                                                   &ext_l2_lookup,
                                                   MAC_BLOCK_INDEXf);
                    _bcm_mac_block_delete(unit, mb_index);
                }
                if (limit_counted) {
                    if (!update_limit) {
                        (void)soc_triumph_learn_count_update(unit,
                                                             &ext_l2_lookup,
                                                             TRUE, -1);
                    } else if (!same_dest) {
                        (void)soc_triumph_learn_count_update(unit,
                                                             &ext_l2_lookup,
                                                             FALSE, -1);
                    }
                }
                rv = BCM_E_NONE;
            } else if (SOC_FAILURE(rv)) {
                /* insert fail, undo setting for new entry */
                if (update_limit) {
                    if (!limit_counted) {
                        (void)soc_triumph_learn_count_update(unit,
                                                             &ext_l2_entry,
                                                             TRUE, -1);
                    } else if (!same_dest) {
                        (void)soc_triumph_learn_count_update(unit,
                                                             &ext_l2_entry,
                                                             FALSE, -1);
                    }
                }
            }
            soc_mem_unlock(unit, EXT_L2_ENTRYm);
            if (SOC_SUCCESS(rv)) {
                /* insert to ext_l2_entry OK, delete from l2x if present */
                soc_mem_lock(unit, L2Xm);
                if (SOC_L2_DEL_SYNC_LOCK(SOC_CONTROL(unit)) >= 0) {
                    rv1 = soc_mem_generic_delete(unit, L2Xm, MEM_BLOCK_ANY, 0,
                                                 &l2x_entry, &l2x_lookup,
                                                 &l2_index);
                    if (SOC_SUCCESS(rv1)) {
                        if (!SOC_CONTROL(unit)->l2x_group_enable) {
                            mb_index =
                                soc_mem_field32_get(unit, L2Xm, &l2x_lookup,
                                                    MAC_BLOCK_INDEXf);
                            _bcm_mac_block_delete(unit, mb_index);
                        }
                        rv1 = soc_l2x_sync_delete(unit, (uint32 *)&l2x_lookup,
                                                  l2_index, 0);
                    } else if (rv1 == SOC_E_NOT_FOUND) {
                        rv1 = BCM_E_NONE;
                    }
                    SOC_L2_DEL_SYNC_UNLOCK(SOC_CONTROL(unit));
                } else {
                    rv1 = BCM_E_INTERNAL;
                }
                soc_mem_unlock(unit, L2Xm);
                return rv1;
            }
            if (rv != SOC_E_FULL) {
                goto done;
            }
        } else {
            soc_mem_unlock(unit, EXT_L2_ENTRYm);
        }
    }
#endif

    rv = soc_mem_generic_lookup(unit, L2Xm, MEM_BLOCK_ANY, 0, &l2x_entry,
                                &l2x_lookup, &l2_index);
    if (BCM_FAILURE(rv) && rv != BCM_E_NOT_FOUND) {
        return rv;
    }

    if (!SOC_CONTROL(unit)->l2x_group_enable) {
        /* Mac blocking, attempt to associate with bitmap entry */
        BCM_IF_ERROR_RETURN
            (_bcm_mac_block_insert(unit, l2addr->block_bitmap, &mb_index));
        soc_mem_field32_set(unit, L2Xm, &l2x_entry, MAC_BLOCK_INDEXf,
                            mb_index);
    }

    enable_ppa_war = FALSE;
    if (SOC_CONTROL(unit)->l2x_ppa_bypass == FALSE &&
        soc_feature(unit, soc_feature_ppa_bypass) &&
        soc_mem_field32_get(unit, L2Xm, &l2x_entry, KEY_TYPEf) !=
        TR_L2_HASH_KEY_TYPE_BRIDGE) {
        enable_ppa_war = TRUE;
    }

    rv = soc_mem_insert_return_old(unit, L2Xm, MEM_BLOCK_ANY, 
                                   (void *)&l2x_entry, (void *)&l2x_entry);
    if ((rv == BCM_E_FULL) && (l2addr->flags & BCM_L2_REPLACE_DYNAMIC)) {
        rv = _bcm_l2_hash_dynamic_replace( unit, &l2x_entry);
        if (rv < 0 ) {
            goto done;
        }
    } else if (rv == BCM_E_EXISTS) {
        if (!SOC_CONTROL(unit)->l2x_group_enable) {
            mb_index = soc_mem_field32_get(unit, L2Xm, &l2x_lookup,
                                           MAC_BLOCK_INDEXf);
            _bcm_mac_block_delete(unit, mb_index);
        }
        rv = BCM_E_NONE;
    }

    if (BCM_SUCCESS(rv) && enable_ppa_war) {
        SOC_CONTROL(unit)->l2x_ppa_bypass = TRUE;
    }

#ifdef BCM_TRIUMPH_SUPPORT
    if (SOC_SUCCESS(rv)) {
        if (exist_in_ext_l2) {
            soc_mem_lock(unit, EXT_L2_ENTRYm);
            limit_counted =
                ppa_info[ext_l2_index].data & _SOC_TR_L2E_LIMIT_COUNTED;
            rv1 = soc_mem_generic_delete(unit, EXT_L2_ENTRYm, MEM_BLOCK_ANY,
                                         0, &ext_l2_entry, &ext_l2_lookup,
                                         &ext_l2_index);
            if (SOC_SUCCESS(rv1)) {
                if (!SOC_CONTROL(unit)->l2x_group_enable) {
                    mb_index =
                        soc_mem_field32_get(unit, EXT_L2_ENTRYm,
                                            &ext_l2_lookup, MAC_BLOCK_INDEXf);
                    _bcm_mac_block_delete(unit, mb_index);
                }
                if (limit_counted) {
                    (void)soc_triumph_learn_count_update(unit, &ext_l2_lookup,
                                                         TRUE, -1);
                }
            }
            soc_mem_unlock(unit, EXT_L2_ENTRYm);
        }
    }
#endif

done:
    if (rv < 0) {
        _bcm_mac_block_delete(unit, mb_index);
    }

    return rv;
}

/*
 * Function:
 *      bcm_tr_l2_addr_delete
 * Description:
 *      Delete an L2 address (MAC+VLAN) from the device
 * Parameters:
 *      unit - device unit
 *      mac  - MAC address to delete
 *      vid  - VLAN id
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_tr_l2_addr_delete(int unit, bcm_mac_t mac, bcm_vlan_t vid)
{
    bcm_l2_addr_t  l2addr;
    l2x_entry_t    l2x_entry, l2x_lookup;
#ifdef BCM_TRIUMPH_SUPPORT
    _soc_tr_l2e_ppa_info_t *ppa_info;
    ext_l2_entry_entry_t ext_l2_entry, ext_l2_lookup;
    int limit_counted;
#endif /* BCM_TRIUMPH_SUPPORT */
    int            l2_index, mb_index;
    int            rv;
    soc_control_t  *soc = SOC_CONTROL(unit);

    bcm_l2_addr_t_init(&l2addr, mac, vid);

#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_IS_TRIDENT(unit) || (SOC_IS_KATANA(unit))) {
        if (soc_mem_is_valid(unit, MY_STATION_TCAMm) && BCM_VLAN_VALID(vid)) {
              rv = bcm_td_l2_myStation_delete (unit, mac, vid, &l2_index);
            if ((rv != BCM_E_NOT_FOUND) && (rv != BCM_E_FULL)) {
                   if (rv != BCM_E_NONE) {
                        return rv;
                   }
              }
         }
    }
#endif /* BCM_TRIDENT_SUPPORT */

#ifdef BCM_TRIUMPH_SUPPORT
    if (soc_mem_is_valid(unit, EXT_L2_ENTRYm) &&
        soc_mem_index_count(unit, EXT_L2_ENTRYm)) {
        BCM_IF_ERROR_RETURN
            (_bcm_tr_l2_to_ext_l2(unit, &ext_l2_entry, &l2addr, TRUE));
        rv = soc_mem_generic_lookup(unit, EXT_L2_ENTRYm, MEM_BLOCK_ANY, 0,
                                    &ext_l2_entry, NULL, &l2_index);
        if (BCM_SUCCESS(rv)) {
            ppa_info = SOC_CONTROL(unit)->ext_l2_ppa_info;
            limit_counted =
                ppa_info[l2_index].data & _SOC_TR_L2E_LIMIT_COUNTED;
            rv = soc_mem_generic_delete(unit, EXT_L2_ENTRYm, MEM_BLOCK_ANY, 0,
                                        &ext_l2_entry, &ext_l2_lookup, NULL);
            if (BCM_SUCCESS(rv)) {
                if (!SOC_CONTROL(unit)->l2x_group_enable) {
                    mb_index =
                        soc_mem_field32_get(unit, EXT_L2_ENTRYm,
                                            &ext_l2_lookup, MAC_BLOCK_INDEXf);
                    _bcm_mac_block_delete(unit, mb_index);
                }
                if (limit_counted) {
                    rv = soc_triumph_learn_count_update(unit, &ext_l2_lookup,
                                                        TRUE, -1);
                }
            }
        }
        if (rv != BCM_E_NOT_FOUND) {
            return rv;
        }
    }
#endif

    BCM_IF_ERROR_RETURN(_bcm_tr_l2_to_l2x(unit, &l2x_entry, &l2addr, TRUE));

    soc_mem_lock(unit, L2Xm);

    rv = soc_mem_search(unit, L2Xm, MEM_BLOCK_ANY, &l2_index,
                       (void *)&l2x_entry, (void *)&l2x_lookup, 0);
    if (BCM_E_NONE != rv) {
        soc_mem_unlock(unit, L2Xm);
        return rv;
    }

    if (!SOC_CONTROL(unit)->l2x_group_enable) {
        mb_index = soc_L2Xm_field32_get(unit, &l2x_lookup, MAC_BLOCK_INDEXf);
        _bcm_mac_block_delete(unit, mb_index);
    }

    if (SOC_L2_DEL_SYNC_LOCK(soc) < 0) {
        soc_mem_unlock(unit, L2Xm);
        return BCM_E_RESOURCE;
    }
    rv = soc_mem_delete_return_old(unit, L2Xm, MEM_BLOCK_ANY,
                                   (void *)&l2x_entry, (void *)&l2x_entry);
    if (rv >= 0) {
        rv = soc_l2x_sync_delete(unit, (uint32 *) &l2x_lookup, l2_index, 0);
    }
    SOC_L2_DEL_SYNC_UNLOCK(soc);
    soc_mem_unlock(unit, L2Xm);

    return rv;
}

#ifdef BCM_TRIUMPH_SUPPORT
void
_bcm_tr_ext_l2_entry_callback(int unit, int index, ext_l2_entry_entry_t *entry)
{
    ext_l2_entry_entry_t ext_l2_lookup;
    int rv, mb_index;

    rv = soc_mem_generic_delete(unit, EXT_L2_ENTRYm, MEM_BLOCK_ANY, 0, entry,
                                &ext_l2_lookup, NULL);
    if (!SOC_CONTROL(unit)->l2x_group_enable) {
        mb_index = soc_mem_field32_get(unit, EXT_L2_ENTRYm, &ext_l2_lookup,
                                       MAC_BLOCK_INDEXf);
        _bcm_mac_block_delete(unit, mb_index);
    }
    if (!soc_mem_field32_get(unit, EXT_L2_ENTRYm, &ext_l2_lookup,
                             STATIC_BITf)) {
        (void)soc_triumph_learn_count_update(unit,  &ext_l2_lookup, TRUE, -1);
    }
}

STATIC int
bcm_tr_l2_addr_ext_get(int unit, sal_mac_addr_t mac, bcm_vlan_t vid,
                   bcm_l2_addr_t *l2addr)
{
    bcm_l2_addr_t           l2addr_key;
    ext_l2_entry_entry_t    ext_l2_entry, ext_l2_lookup;
    int                     rv;

    bcm_l2_addr_t_init(&l2addr_key, mac, vid);

    BCM_IF_ERROR_RETURN(
        _bcm_tr_l2_to_ext_l2(unit, &ext_l2_entry, &l2addr_key, TRUE));

    soc_mem_lock(unit, EXT_L2_ENTRYm);
    rv = soc_mem_generic_lookup(unit, EXT_L2_ENTRYm, MEM_BLOCK_ANY, 0,
                               &ext_l2_entry, &ext_l2_lookup, NULL);
    soc_mem_unlock(unit, EXT_L2_ENTRYm);

    if (BCM_SUCCESS(rv)) {
        rv = _bcm_tr_l2_from_ext_l2(unit, l2addr, &ext_l2_lookup);
    }

    return rv;
}

#endif /* BCM_TRIUMPH_SUPPORT */

/*
 * Function:
 *      bcm_tr_l2_addr_get
 * Description:
 *      Given a MAC address and VLAN ID, check if the entry is present
 *      in the L2 table, and if so, return all associated information.
 * Parameters:
 *      unit - Device unit number
 *      mac - input MAC address to search
 *      vid - input VLAN ID to search
 *      l2addr - Pointer to bcm_l2_addr_t structure to receive results
 * Returns:
 *      BCM_E_NONE              Success (l2addr filled in)
 *      BCM_E_PARAM             Illegal parameter (NULL pointer)
 *      BCM_E_INTERNAL          Chip access failure
 *      BCM_E_NOT_FOUND Address not found (l2addr not filled in)
 */

int
bcm_tr_l2_addr_get(int unit, sal_mac_addr_t mac, bcm_vlan_t vid,
                   bcm_l2_addr_t *l2addr)
{
    bcm_l2_addr_t l2addr_key;
    l2x_entry_t  l2x_entry, l2x_lookup;
    int          rv;

    bcm_l2_addr_t_init(&l2addr_key, mac, vid);

#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_IS_TRIDENT(unit) || SOC_IS_KATANA(unit)) {
        if (soc_mem_is_valid(unit, MY_STATION_TCAMm)) {
              rv = bcm_td_l2_myStation_get (unit, mac, vid, l2addr);
              if (BCM_SUCCESS(rv)) {
                   return rv;
              }
         }
    }
#endif /* BCM_TRIDENT_SUPPORT */

    BCM_IF_ERROR_RETURN(
        _bcm_tr_l2_to_l2x(unit, &l2x_entry, &l2addr_key, TRUE));

    soc_mem_lock(unit, L2Xm);

    rv = soc_mem_generic_lookup(unit, L2Xm, MEM_BLOCK_ANY, 0, &l2x_entry,
                                &l2x_lookup, NULL);

    /* If not found in Internal memory and external is available serach there */
#ifdef BCM_TRIUMPH_SUPPORT
    if (rv == BCM_E_NOT_FOUND && soc_mem_is_valid(unit, EXT_L2_ENTRYm) &&
        soc_mem_index_count(unit, EXT_L2_ENTRYm) > 0) {
        rv = bcm_tr_l2_addr_ext_get(unit, mac, vid, l2addr);
            soc_mem_unlock(unit, L2Xm);
            return rv;
        }
#endif /* BCM_TRIUMPH_SUPPORT */

    soc_mem_unlock(unit, L2Xm);
    if (SOC_SUCCESS(rv)) {
        rv = _bcm_tr_l2_from_l2x(unit, l2addr, &l2x_lookup);
    }
    return rv;
}


#ifdef BCM_WARM_BOOT_SUPPORT
/*
 * Function:
 *      _bcm_tr_l2_reload_mbi
 * Description:
 *      Load MAC block info from hardware into software data structures.
 * Parameters:
 *      unit - StrataSwitch unit number.
 */

STATIC int
_bcm_tr_l2_reload_mbi(int unit)
{
    _bcm_mac_block_info_t *mbi = _mbi_entries[unit];
    l2x_entry_t         *l2x_entry, *l2x_table;
    mac_block_entry_t   mbe;
    int                 index, mb_index, l2x_size;
    pbmp_t              mb_pbmp;

    /*
     * Refresh MAC Block information from the hardware tables.
     */

    for (mb_index = 0; mb_index < _mbi_num[unit]; mb_index++) {
        SOC_IF_ERROR_RETURN
            (READ_MAC_BLOCKm(unit, MEM_BLOCK_ANY, mb_index, &mbe));

        SOC_PBMP_CLEAR(mb_pbmp);

        if (soc_mem_field_valid(unit, MAC_BLOCKm, MAC_BLOCK_MASK_LOf)) {
            SOC_PBMP_WORD_SET(mb_pbmp, 0,
                              soc_MAC_BLOCKm_field32_get(unit, &mbe, 
                                                         MAC_BLOCK_MASK_LOf));
        } else {
            SOC_PBMP_WORD_SET(mb_pbmp, 0,
                              soc_MAC_BLOCKm_field32_get(unit, &mbe, 
                                                         MAC_BLOCK_MASKf));
        }
        if (soc_mem_field_valid(unit, MAC_BLOCKm, MAC_BLOCK_MASK_HIf)) {
            SOC_PBMP_WORD_SET(mb_pbmp, 1,
                          soc_MAC_BLOCKm_field32_get(unit, &mbe, 
                                                     MAC_BLOCK_MASK_HIf));
        }
        BCM_PBMP_ASSIGN(mbi[mb_index].mb_pbmp, mb_pbmp);
    }

    if (!SOC_CONTROL(unit)->l2x_group_enable) {
        l2x_size = sizeof(l2x_entry_t) * soc_mem_index_count(unit, L2Xm);
        l2x_table = soc_cm_salloc(unit, l2x_size, "l2 reload");
        if (l2x_table == NULL) {
            return BCM_E_MEMORY;
        }

        memset((void *)l2x_table, 0, l2x_size);
        if (soc_mem_read_range(unit, L2Xm, MEM_BLOCK_ANY,
                               soc_mem_index_min(unit, L2Xm),
                               soc_mem_index_max(unit, L2Xm),
                               l2x_table) < 0) {
            soc_cm_sfree(unit, l2x_table);
            return SOC_E_INTERNAL;
        }

        for (index = soc_mem_index_min(unit, L2Xm);
             index <= soc_mem_index_max(unit, L2Xm); index++) {

             l2x_entry = soc_mem_table_idx_to_pointer(unit, L2Xm,
                                                      l2x_entry_t *,
                                                      l2x_table, index);
             if (!soc_L2Xm_field32_get(unit, l2x_entry, VALIDf)) {
                 continue;
             }
  
             mb_index = soc_L2Xm_field32_get(unit, l2x_entry, MAC_BLOCK_INDEXf);
             mbi[mb_index].ref_count++;
        }
        soc_cm_sfree(unit, l2x_table);
    }

    return BCM_E_NONE;
}
#endif /* BCM_WARM_BOOT_SUPPORT */


/*
 * Function:
 *      _tr_l2x_delete_all
 * Purpose:
 *      Clear the L2 table by invalidating entries.
 * Parameters:
 *      unit - StrataSwitch unit #
 *      static_too - if TRUE, delete static and non-static entries;
 *                   if FALSE, delete only non-static entries
 * Returns:
 *      SOC_E_XXX
 */

static int
_tr_l2x_delete_all(int unit)
{
    soc_control_t *soc = SOC_CONTROL(unit);
    int index_min, index_max, index, mem_max;
    l2_entry_only_entry_t *l2x_entry;
    int rv = SOC_E_NONE;
    int *buffer = NULL;
    int mem_size, idx;
    int modified;
    uint32 key_type;

#ifdef BCM_TRIDENT_SUPPORT
    if (soc_feature(unit, soc_feature_l2_bulk_control)) {
        l2_bulk_match_mask_entry_t match_mask;
        l2_bulk_match_data_entry_t match_data;
        int field_len;

        sal_memset(&match_mask, 0, sizeof(match_mask));
        sal_memset(&match_data, 0, sizeof(match_data));

        soc_mem_field32_set(unit, L2_BULK_MATCH_MASKm, &match_mask, VALIDf, 1);
        soc_mem_field32_set(unit, L2_BULK_MATCH_DATAm, &match_data, VALIDf, 1);

        field_len = soc_mem_field_length(unit, L2_BULK_MATCH_MASKm, KEY_TYPEf);
        soc_mem_field32_set(unit, L2_BULK_MATCH_MASKm, &match_mask, KEY_TYPEf,
                            (1 << field_len) - 1);

        soc_mem_lock(unit, L2Xm);
        rv = soc_reg_field32_modify(unit, L2_BULK_CONTROLr, REG_PORT_ANY,
                                    ACTIONf, 1);
        if (BCM_SUCCESS(rv)) {
            rv = WRITE_L2_BULK_MATCH_MASKm(unit, MEM_BLOCK_ALL, 0,
                                           &match_mask);
        }

        /* Remove all KEY_TYPE 0 entries */
        if (BCM_SUCCESS(rv)) {
            rv = WRITE_L2_BULK_MATCH_DATAm(unit, MEM_BLOCK_ALL, 0,
                                           &match_data);
            if (BCM_SUCCESS(rv)) {
                rv = soc_l2x_port_age(unit, L2_BULK_CONTROLr, INVALIDr);
            }
        }

        /* Remove all KEY_TYPE 3 entries */
        if (BCM_SUCCESS(rv)) {
            soc_mem_field32_set(unit, L2_BULK_MATCH_DATAm, &match_data,
                                KEY_TYPEf, 3);
            rv = WRITE_L2_BULK_MATCH_DATAm(unit, MEM_BLOCK_ALL, 0,
                                           &match_data);
            if (BCM_SUCCESS(rv)) {
                rv = soc_l2x_port_age(unit, L2_BULK_CONTROLr, INVALIDr);
            }
        }
    } else
#endif /* BCM_TRIDENT_SUPPORT */
    {
        index_min = soc_mem_index_min(unit, L2_ENTRY_ONLYm);
        mem_max = soc_mem_index_max(unit, L2_ENTRY_ONLYm);
        mem_size =  DEFAULT_L2DELETE_CHUNKS * sizeof(l2_entry_only_entry_t);
    
        buffer = soc_cm_salloc(unit, mem_size, "L2_ENTRY_ONLY_delete");
        if (NULL == buffer) {
            return SOC_E_MEMORY;
        }

        soc_mem_lock(unit, L2Xm);
        for (idx = index_min; idx < mem_max; idx += DEFAULT_L2DELETE_CHUNKS) {
            index_max = idx + DEFAULT_L2DELETE_CHUNKS - 1;
            if ( index_max > mem_max) {
                index_max = mem_max;
            }
            if ((rv = soc_mem_read_range(unit, L2_ENTRY_ONLYm, MEM_BLOCK_ALL,
                                         idx, index_max, buffer)) < 0 ) {
                soc_cm_sfree(unit, buffer);
                soc_mem_unlock(unit, L2Xm);
                return rv;
            }
            modified = FALSE;
            for (index = 0; index < DEFAULT_L2DELETE_CHUNKS; index++) {
                l2x_entry =
                    soc_mem_table_idx_to_pointer(unit, L2_ENTRY_ONLYm,
                                                 l2_entry_only_entry_t *,
                                                 buffer, index);
                if (!soc_L2_ENTRY_ONLYm_field32_get(unit, l2x_entry, VALIDf)) {
                    continue;
                }
                key_type = soc_L2_ENTRY_ONLYm_field32_get(unit, l2x_entry,
                                                          KEY_TYPEf);
                if (key_type ==  TR_L2_HASH_KEY_TYPE_BRIDGE ||
                    key_type == TR_L2_HASH_KEY_TYPE_VFI) {
                    sal_memcpy(l2x_entry,
                               soc_mem_entry_null(unit, L2_ENTRY_ONLYm),
                               sizeof(l2_entry_only_entry_t));
                    modified = TRUE;
                }
            }
            if (!modified) {
                continue;
            }
            if ((rv = soc_mem_write_range(unit, L2_ENTRY_ONLYm, MEM_BLOCK_ALL, 
                                          idx, index_max, buffer)) < 0) {
                soc_cm_sfree(unit, buffer);
                soc_mem_unlock(unit, L2Xm);
                return rv;
            }
        }
        soc_cm_sfree(unit, buffer);
    }

    if (soc->arlShadow != NULL) {
        sal_mutex_take(soc->arlShadowMutex, sal_mutex_FOREVER);
        (void) shr_avl_delete_all(soc->arlShadow);
        sal_mutex_give(soc->arlShadowMutex);
    }

    /* Clear external L2 table if it exists */
    if (soc_mem_is_valid(unit, EXT_L2_ENTRYm) &&
        soc_mem_index_count(unit, EXT_L2_ENTRYm)) {
        SOC_IF_ERROR_RETURN(soc_mem_clear(unit, EXT_L2_ENTRY_TCAMm,
                                          MEM_BLOCK_ALL, TRUE));
        SOC_IF_ERROR_RETURN(soc_mem_clear(unit, EXT_L2_ENTRY_DATAm,
                                          MEM_BLOCK_ALL, TRUE));
    }
    soc_mem_unlock(unit, L2Xm);

    return rv;
}

/*
 * Function:
 *      bcm_tr_l2_init
 * Description:
 *      Initialize chip-dependent parts of L2 module
 * Parameters:
 *      unit - StrataSwitch unit number.
 */

int
bcm_tr_l2_init(int unit)
{
    int         was_running = FALSE;
    uint32      flags;
    sal_usecs_t interval;

    if (soc_l2x_running(unit, &flags, &interval)) { 	 
        was_running = TRUE; 	 
        BCM_IF_ERROR_RETURN(soc_l2x_stop(unit)); 	 
    }

    if (!SOC_WARM_BOOT(unit) && !SOC_IS_RCPU_ONLY(unit)) {
        if (!(SAL_BOOT_QUICKTURN || SAL_BOOT_SIMULATION || SAL_BOOT_BCMSIM)) {
            _tr_l2x_delete_all(unit);
        }
    }

    if (_mbi_entries[unit] != NULL) {
        sal_free(_mbi_entries[unit]);
        _mbi_entries[unit] = NULL;
    }

    _mbi_num[unit] = (SOC_MEM_INFO(unit, MAC_BLOCKm).index_max -
                      SOC_MEM_INFO(unit, MAC_BLOCKm).index_min + 1);
    _mbi_entries[unit] = sal_alloc(_mbi_num[unit] *
                                   sizeof(_bcm_mac_block_info_t),
                                   "BCM L2X MAC blocking info");
    if (!_mbi_entries[unit]) {
        return BCM_E_MEMORY;
    }

    sal_memset(_mbi_entries[unit], 0,
               _mbi_num[unit] * sizeof(_bcm_mac_block_info_t));

#ifdef BCM_WARM_BOOT_SUPPORT
    if (SOC_WARM_BOOT(unit)) {
        BCM_IF_ERROR_RETURN(_bcm_tr_l2_reload_mbi(unit));
    }
#endif

    /* bcm_l2_register clients */
    
    soc_l2x_register(unit,
            _bcm_l2_register_callback,
            NULL);

    if (was_running) {
        interval = (SAL_BOOT_BCMSIM)? BCMSIM_L2XMSG_INTERVAL : interval;
        soc_l2x_start(unit, flags, interval);
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_tr_l2_term
 * Description:
 *      Finalize chip-dependent parts of L2 module
 * Parameters:
 *      unit - StrataSwitch unit number.
 */

int
bcm_tr_l2_term(int unit)
{
    if (_mbi_entries[unit] != NULL) {
        sal_free(_mbi_entries[unit]);
        _mbi_entries[unit] = NULL;
    }
    return BCM_E_NONE;
}

static int
_bcm_tr_dual_l2_conflict_get(int unit, bcm_l2_addr_t *addr,
                             bcm_l2_addr_t *cf_array, int cf_max,
                             int *cf_count)
{
    l2x_entry_t         l2ent;
    uint8               key[XGS_HASH_KEY_SIZE];
    uint32              tmp_hs;
    int                 hash_sel = 0, bucket, bucket_chunk, slot, bank, num_bits;

    *cf_count = 0;
    BCM_IF_ERROR_RETURN(_bcm_tr_l2_to_l2x(unit, &l2ent, addr, TRUE));
    num_bits = soc_tr_l2x_base_entry_to_key(unit, (void*)&l2ent, key);

    for (bank = 0; bank < 2; bank++) {
        /* Get L2 hash select */
        if (bank > 0) {
            SOC_IF_ERROR_RETURN(READ_L2_AUX_HASH_CONTROLr(unit, &tmp_hs));
            if (soc_reg_field_get(unit, L2_AUX_HASH_CONTROLr,
                                  tmp_hs, ENABLEf)) {
                hash_sel = soc_reg_field_get(unit, L2_AUX_HASH_CONTROLr,
                                             tmp_hs, HASH_SELECTf);
            }
        } else {
            SOC_IF_ERROR_RETURN(READ_HASH_CONTROLr(unit, &tmp_hs));
            hash_sel = soc_reg_field_get(unit, HASH_CONTROLr,
                                         tmp_hs, L2_AND_VLAN_MAC_HASH_SELECTf);
        }

        bucket = soc_tr_l2x_hash(unit, hash_sel, num_bits, &l2ent, key);

        bucket_chunk = SOC_L2X_BUCKET_SIZE / 2;

        for (slot = bucket_chunk * bank;
             (slot < (bucket_chunk * (bank + 1))) && (*cf_count < cf_max);
             slot++) {
            SOC_IF_ERROR_RETURN
                (soc_mem_read(unit, L2Xm, MEM_BLOCK_ANY,
                              bucket * SOC_L2X_BUCKET_SIZE + slot,
                              &l2ent));
            if (!soc_L2Xm_field32_get(unit, &l2ent, VALIDf)) {
                continue;
            }
            if ((soc_L2Xm_field32_get(unit, &l2ent, KEY_TYPEf) ==
                                      TR_L2_HASH_KEY_TYPE_BRIDGE) ||
                (soc_L2Xm_field32_get(unit, &l2ent, KEY_TYPEf) ==
                                      TR_L2_HASH_KEY_TYPE_VFI)) {
                BCM_IF_ERROR_RETURN
                    (_bcm_tr_l2_from_l2x(unit, &cf_array[*cf_count], &l2ent));
                *cf_count += 1;
            }
        }
    }

    return BCM_E_NONE;
}

int
bcm_tr_l2_conflict_get(int unit, bcm_l2_addr_t *addr,
                          bcm_l2_addr_t *cf_array, int cf_max,
                          int *cf_count)
{
    l2x_entry_t         l2ent;
    uint8               key[XGS_HASH_KEY_SIZE];
    int                 hash_sel, bucket, slot, num_bits;
    uint32              hash_control;

    if (soc_feature(unit, soc_feature_dual_hash)) {
        return _bcm_tr_dual_l2_conflict_get(unit, addr, cf_array,
                                            cf_max, cf_count);
    }

    *cf_count = 0;

    BCM_IF_ERROR_RETURN(_bcm_tr_l2_to_l2x(unit, &l2ent, addr, TRUE));

    /* Get L2 hash select */
    SOC_IF_ERROR_RETURN(READ_HASH_CONTROLr(unit, &hash_control));
    hash_sel = soc_reg_field_get(unit, HASH_CONTROLr, hash_control,
                                    L2_AND_VLAN_MAC_HASH_SELECTf);
    num_bits = soc_tr_l2x_base_entry_to_key(unit, &l2ent, key);
    bucket = soc_tr_l2x_hash(unit, hash_sel, num_bits, &l2ent, key);

    for (slot = 0;
         slot < SOC_L2X_BUCKET_SIZE && *cf_count < cf_max;
         slot++) {
        SOC_IF_ERROR_RETURN
            (soc_mem_read(unit, L2Xm, MEM_BLOCK_ANY,
                          bucket * SOC_L2X_BUCKET_SIZE + slot,
                          &l2ent));
        if (!soc_L2Xm_field32_get(unit, &l2ent, VALIDf)) {
            continue;
        }
        if ((soc_L2Xm_field32_get(unit, &l2ent, KEY_TYPEf) == 
                                  TR_L2_HASH_KEY_TYPE_BRIDGE) ||
            (soc_L2Xm_field32_get(unit, &l2ent, KEY_TYPEf) ==
                                  TR_L2_HASH_KEY_TYPE_VFI)) {
            BCM_IF_ERROR_RETURN
                (_bcm_tr_l2_from_l2x(unit, &cf_array[*cf_count], &l2ent));
            *cf_count += 1;
        }
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_vlan_cross_connect_add
 * Purpose:
 *      Add a VLAN cross connect entry
 * Parameters:
 *      unit       - Device unit number
 *      outer_vlan - Outer vlan ID
 *      inner_vlan - Inner vlan ID
 *      port_1     - First port in the cross-connect
 *      port_2     - Second port in the cross-connect
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_XXX
 */
int 
bcm_tr_l2_cross_connect_add(int unit, bcm_vlan_t outer_vlan, 
                            bcm_vlan_t inner_vlan, bcm_gport_t port_1, 
                            bcm_gport_t port_2)
{
    l2x_entry_t  l2x_entry, l2x_lookup;
    int rv, gport_id, l2_index;
    bcm_port_t port_out;
    bcm_module_t mod_out;
    bcm_trunk_t trunk_id;

    sal_memset(&l2x_entry, 0, sizeof (l2x_entry));
    if ((outer_vlan < BCM_VLAN_DEFAULT) || (outer_vlan > BCM_VLAN_MAX)) {
        return BCM_E_PARAM;
    } else if (inner_vlan == BCM_VLAN_INVALID) {
        /* Single cross-connect (use only outer_vid) */
        soc_L2Xm_field32_set(unit, &l2x_entry, KEY_TYPEf,
                             TR_L2_HASH_KEY_TYPE_SINGLE_CROSS_CONNECT);
    } else {
        if ((inner_vlan < BCM_VLAN_DEFAULT) || (inner_vlan > BCM_VLAN_MAX)) {
            return BCM_E_PARAM;
        }
        /* Double cross-connect (use both outer_vid and inner_vid) */
        soc_L2Xm_field32_set(unit, &l2x_entry, KEY_TYPEf,
                             TR_L2_HASH_KEY_TYPE_DOUBLE_CROSS_CONNECT);
        soc_L2Xm_field32_set(unit, &l2x_entry, IVIDf, inner_vlan);
    }
    soc_L2Xm_field32_set(unit, &l2x_entry, STATIC_BITf, 1);
    soc_L2Xm_field32_set(unit, &l2x_entry, VALIDf, 1);
    soc_L2Xm_field32_set(unit, &l2x_entry, VLAN_IDf, outer_vlan);

    /* See if the entry already exists */
    rv = soc_mem_generic_lookup(unit, L2Xm, MEM_BLOCK_ANY, 0, &l2x_entry,
                                &l2x_lookup, &l2_index);
                
    if ((rv < 0) && (rv != BCM_E_NOT_FOUND)) {
         return rv;
    } 

    /* Resolve first port */
    BCM_IF_ERROR_RETURN
        (_bcm_esw_gport_resolve(unit, port_1, &mod_out, &port_out, &trunk_id,
                                &gport_id));
    if (BCM_GPORT_IS_TRUNK(port_1)) {
        soc_L2Xm_field32_set(unit, &l2x_entry, Tf, 1);
        soc_L2Xm_field32_set(unit, &l2x_entry, TGIDf, trunk_id);
    } else if (BCM_GPORT_IS_SUBPORT_GROUP(port_1)) {
        soc_L2Xm_field32_set(unit, &l2x_entry, VPG_TYPEf, 1);
#if defined(BCM_SCORPION_SUPPORT) && defined(INCLUDE_L3)
           if (SOC_IS_SC_CQ(unit)) {
               /* Map the gport_id to index to L3_NEXT_HOP */
               gport_id = (int) _sc_subport_group_index[unit][gport_id/8];
           }
#endif /* BCM_SCORPION_SUPPORT && INCLUDE_L3 */
        soc_L2Xm_field32_set(unit, &l2x_entry, VPGf, gport_id);
    } else {
        if ((mod_out == -1) ||(port_out == -1)) {
            return BCM_E_PORT;
        }
        soc_L2Xm_field32_set(unit, &l2x_entry, MODULE_IDf, mod_out);
        soc_L2Xm_field32_set(unit, &l2x_entry, PORT_NUMf, port_out);
    }

    /* Resolve second port */
    BCM_IF_ERROR_RETURN 
        (_bcm_esw_gport_resolve(unit, port_2, &mod_out, &port_out, &trunk_id,
                                &gport_id));
    if (BCM_GPORT_IS_TRUNK(port_2)) {
        soc_L2Xm_field32_set(unit, &l2x_entry, T_1f, 1);
        soc_L2Xm_field32_set(unit, &l2x_entry, TGID_1f, trunk_id);
    } else if (BCM_GPORT_IS_SUBPORT_GROUP(port_2)) {
        soc_L2Xm_field32_set(unit, &l2x_entry, VPG_TYPE_1f, 1);
#if defined(BCM_SCORPION_SUPPORT) && defined(INCLUDE_L3)
           if (SOC_IS_SC_CQ(unit)) {
               /* Map the gport_id to index to L3_NEXT_HOP */
               gport_id = (int) _sc_subport_group_index[unit][gport_id/8];
           }
#endif /* BCM_SCORPION_SUPPORT && INCLUDE_L3 */
        soc_L2Xm_field32_set(unit, &l2x_entry, VPG_1f, gport_id);
    } else {
        if ((mod_out == -1) ||(port_out == -1)) {
            return BCM_E_PORT;
        }
        soc_L2Xm_field32_set(unit, &l2x_entry, MODULE_ID_1f, mod_out);
        soc_L2Xm_field32_set(unit, &l2x_entry, PORT_NUM_1f, port_out);
    }

    rv = soc_mem_insert_return_old(unit, L2Xm, MEM_BLOCK_ANY, 
                                   (void *)&l2x_entry, (void *)&l2x_entry);
    if (rv == BCM_E_FULL) {
        rv = _bcm_l2_hash_dynamic_replace( unit, &l2x_entry);
    } 
    if (BCM_SUCCESS(rv)) {
        if (soc_feature(unit, soc_feature_ppa_bypass)) {
            SOC_CONTROL(unit)->l2x_ppa_bypass = TRUE;
        }
    }

    return rv;
}

/*
 * Function:
 *      bcm_vlan_cross_connect_delete
 * Purpose:
 *      Delete a VLAN cross connect entry
 * Parameters:
 *      unit       - Device unit number
 *      outer_vlan - Outer vlan ID
 *      inner_vlan - Inner vlan ID
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_XXX
 */
int 
bcm_tr_l2_cross_connect_delete(int unit, bcm_vlan_t outer_vlan, 
                               bcm_vlan_t inner_vlan)
{
    l2x_entry_t  l2x_entry, l2x_lookup;
    int rv, l2_index;

    sal_memset(&l2x_entry, 0, sizeof (l2x_entry));
    if ((outer_vlan < BCM_VLAN_DEFAULT) || (outer_vlan > BCM_VLAN_MAX)) {
        return BCM_E_PARAM;
    } else if (inner_vlan == BCM_VLAN_INVALID) {
        /* Single cross-connect (use only outer_vid) */
        soc_L2Xm_field32_set(unit, &l2x_entry, KEY_TYPEf,
                             TR_L2_HASH_KEY_TYPE_SINGLE_CROSS_CONNECT);
    } else {
        if ((inner_vlan < BCM_VLAN_DEFAULT) || (inner_vlan > BCM_VLAN_MAX)) {
            return BCM_E_PARAM;
        }
        /* Double cross-connect (use both outer_vid and inner_vid) */
        soc_L2Xm_field32_set(unit, &l2x_entry, KEY_TYPEf,
                             TR_L2_HASH_KEY_TYPE_DOUBLE_CROSS_CONNECT);
        soc_L2Xm_field32_set(unit, &l2x_entry, IVIDf, inner_vlan);
    }
    soc_L2Xm_field32_set(unit, &l2x_entry, STATIC_BITf, 1);
    soc_L2Xm_field32_set(unit, &l2x_entry, VALIDf, 1);
    soc_L2Xm_field32_set(unit, &l2x_entry, VLAN_IDf, outer_vlan);

    rv = soc_mem_search(unit, L2Xm, MEM_BLOCK_ANY, &l2_index, 
                        (void *)&l2x_entry, (void *)&l2x_lookup, 0);
                 
    if ((rv < 0) && (rv != BCM_E_NOT_FOUND)) {
         return rv;
    } 

    rv = soc_mem_delete(unit, L2Xm, MEM_BLOCK_ANY, (void *)&l2x_entry);
    return rv;
}

/*
 * Function:
 *      bcm_vlan_cross_connect_delete_all
 * Purpose:
 *      Delete all VLAN cross connect entries
 * Parameters:
 *      unit       - Device unit number
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_XXX
 */
int
bcm_tr_l2_cross_connect_delete_all(int unit)
{
    soc_control_t  *soc = SOC_CONTROL(unit);
    int index_min, index_max, index, mem_max;
    l2_entry_only_entry_t *l2x_entry;
    int rv = SOC_E_NONE;
    int *buffer = NULL;
    int mem_size, idx;
    soc_mem_t mem = L2_ENTRY_ONLYm;
#ifdef BCM_KATANA_SUPPORT
    if (SOC_IS_KATANA(unit)) {
        mem = L2Xm;
    }
#endif 

    index_min = soc_mem_index_min(unit, mem);
    mem_max = soc_mem_index_max(unit, mem);
    mem_size =  DEFAULT_L2DELETE_CHUNKS * sizeof(l2_entry_only_entry_t);
    
    buffer = soc_cm_salloc(unit, mem_size, "L2_ENTRY_ONLY_delete");
    if (NULL == buffer) {
        return SOC_E_MEMORY;
    }

    soc_mem_lock(unit, L2Xm);
    for (idx = index_min; idx < mem_max; idx += DEFAULT_L2DELETE_CHUNKS) {
        index_max = idx + DEFAULT_L2DELETE_CHUNKS - 1;
        if ( index_max > mem_max) {
            index_max = mem_max;
        }
        if ((rv = soc_mem_read_range(unit, mem, MEM_BLOCK_ALL,
                                     idx, index_max, buffer)) < 0 ) {
            soc_cm_sfree(unit, buffer);
            soc_mem_unlock(unit, L2Xm);
            return rv;
        }
        for (index = 0; index < DEFAULT_L2DELETE_CHUNKS; index++) {
            l2x_entry =
                soc_mem_table_idx_to_pointer(unit, mem,
                                             l2_entry_only_entry_t *, buffer, index);
            if (soc_L2_ENTRY_ONLYm_field32_get(unit, l2x_entry, VALIDf) &&
                ((soc_L2_ENTRY_ONLYm_field32_get(unit, l2x_entry, KEY_TYPEf) ==
                                      TR_L2_HASH_KEY_TYPE_SINGLE_CROSS_CONNECT) ||
                 (soc_L2_ENTRY_ONLYm_field32_get(unit, l2x_entry, KEY_TYPEf) ==
                                      TR_L2_HASH_KEY_TYPE_DOUBLE_CROSS_CONNECT))) {
                sal_memcpy(l2x_entry, soc_mem_entry_null(unit, mem),
                           sizeof(l2_entry_only_entry_t));
            }
        }
        if ((rv = soc_mem_write_range(unit, mem, MEM_BLOCK_ALL,
                                     idx, index_max, buffer)) < 0) {
            soc_cm_sfree(unit, buffer);
            soc_mem_unlock(unit, L2Xm);
            return rv;
        }
    }

    if (soc->arlShadow != NULL) {
        sal_mutex_take(soc->arlShadowMutex, sal_mutex_FOREVER);
        (void) shr_avl_delete_all(soc->arlShadow);
        sal_mutex_give(soc->arlShadowMutex);
    }
    soc_cm_sfree(unit, buffer);
    soc_mem_unlock(unit, L2Xm);

    return rv;
}

/*
 * Function:
 *      bcm_vlan_cross_connect_traverse
 * Purpose:
 *      Walks through the valid cross connect entries and calls
 *      the user supplied callback function for each entry.
 * Parameters:
 *      unit       - (IN) bcm device.
 *      trav_fn    - (IN) Callback function.
 *      user_data  - (IN) User data to be passed to callback function.
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_XXX
 */
int
bcm_tr_l2_cross_connect_traverse(int unit,
                                 bcm_vlan_cross_connect_traverse_cb cb,
                                 void *user_data)
{
    int index_min, index_max, index, mem_max;
    l2_entry_only_entry_t *l2x_entry;
    int rv = SOC_E_NONE;
    int *buffer = NULL;
    int mem_size, idx;
    bcm_gport_t port_1, port_2;
    bcm_vlan_t outer_vlan, inner_vlan;
    bcm_port_t port_in, port_out;
    bcm_module_t mod_in, mod_out;
    soc_mem_t mem = L2_ENTRY_ONLYm;
#ifdef BCM_KATANA_SUPPORT
    if (SOC_IS_KATANA(unit)) {
        mem = L2Xm;
    }
#endif 

    index_min = soc_mem_index_min(unit, mem);
    mem_max = soc_mem_index_max(unit, mem);
    mem_size =  DEFAULT_L2DELETE_CHUNKS * sizeof(l2_entry_only_entry_t);
    
    buffer = soc_cm_salloc(unit, mem_size, "cross connect traverse");
    if (NULL == buffer) {
        return SOC_E_MEMORY;
    }
    
    soc_mem_lock(unit, L2Xm);
    for (idx = index_min; idx < mem_max; idx += DEFAULT_L2DELETE_CHUNKS) {
        index_max = idx + DEFAULT_L2DELETE_CHUNKS - 1;
        if ( index_max > mem_max) {
            index_max = mem_max;
        }
        if ((rv = soc_mem_read_range(unit, mem, MEM_BLOCK_ALL,
                                     idx, index_max, buffer)) < 0 ) {
            soc_cm_sfree(unit, buffer);
            soc_mem_unlock(unit, L2Xm);
            return rv;
        }
        for (index = 0; index < DEFAULT_L2DELETE_CHUNKS; index++) {
            l2x_entry = 
                soc_mem_table_idx_to_pointer(unit, mem,
                                             l2_entry_only_entry_t *, buffer, index);
            if (soc_L2_ENTRY_ONLYm_field32_get(unit, l2x_entry, VALIDf)) {
                if (soc_L2_ENTRY_ONLYm_field32_get(unit, l2x_entry, KEY_TYPEf) ==
                                     TR_L2_HASH_KEY_TYPE_DOUBLE_CROSS_CONNECT) {
                    /* Double cross-connect entry */
                    inner_vlan = soc_L2_ENTRY_ONLYm_field32_get(unit, l2x_entry, IVIDf);
                } else if (soc_L2_ENTRY_ONLYm_field32_get(unit, l2x_entry, KEY_TYPEf) ==
                                     TR_L2_HASH_KEY_TYPE_SINGLE_CROSS_CONNECT) {
                    /* Single cross-connect entry */
                    inner_vlan = BCM_VLAN_INVALID;
                } else {
                    /* Not a cross-connect entry, ignore */
                    continue;
                }
                outer_vlan = soc_L2_ENTRY_ONLYm_field32_get(unit, l2x_entry, OVIDf);

                /* Get first port params */
                if (SOC_MEM_FIELD_VALID(unit, mem, VPG_TYPEf) && 
                    soc_L2_ENTRY_ONLYm_field32_get(unit, l2x_entry, VPG_TYPEf)) {
                    int vpg;
                    vpg = soc_L2_ENTRY_ONLYm_field32_get(unit, l2x_entry, VPGf);
#if defined(BCM_SCORPION_SUPPORT) && defined(INCLUDE_L3)
                /* Scorpion uses index to L3_NEXT_HOP as VPG */
                    if (SOC_IS_SC_CQ(unit)) {
                        int grp;
                        _SC_SUBPORT_VPG_FIND(unit, vpg, grp);
                        if ((vpg = grp) == -1) {
                            L2_ERR(("Unit: %d can not find entry for VPG\n", unit));
                        }
                    }             
#endif  /* BCM_SCORPION_SUPPORT && INCLUDE_L3 */     
                    BCM_GPORT_SUBPORT_GROUP_SET(port_1, vpg);
                } else if (soc_L2_ENTRY_ONLYm_field32_get(unit, l2x_entry, Tf)) {
                    BCM_GPORT_TRUNK_SET(port_1, 
                        soc_L2_ENTRY_ONLYm_field32_get(unit, l2x_entry, TGIDf));
                } else {
                    port_in = soc_L2_ENTRY_ONLYm_field32_get(unit, l2x_entry, PORT_NUMf);
                    mod_in = soc_L2_ENTRY_ONLYm_field32_get(unit, l2x_entry, MODULE_IDf);
                    BCM_IF_ERROR_RETURN
                        (_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_GET,
                                                mod_in, port_in, &mod_out, &port_out));
                    BCM_GPORT_MODPORT_SET(port_1, mod_out, port_out);
                }

                /* Get second port params */
                if (SOC_MEM_FIELD_VALID(unit, mem, VPG_TYPE_1f) && 
                    soc_L2_ENTRY_ONLYm_field32_get(unit, l2x_entry, VPG_TYPE_1f)) {
                    int vpg;
                    vpg = soc_L2_ENTRY_ONLYm_field32_get(unit, l2x_entry, VPG_1f);
#if defined(BCM_SCORPION_SUPPORT) && defined(INCLUDE_L3)
                    if (SOC_IS_SC_CQ(unit)) {
                        int grp;
                        _SC_SUBPORT_VPG_FIND(unit, vpg, grp);
                        if ((vpg = grp) == -1) {
                            L2_ERR(("Unit: %d can not find entry for VPG\n", unit));
                        }
                    }             
#endif  /* BCM_SCORPION_SUPPORT && INCLUDE_L3 */     
                    BCM_GPORT_SUBPORT_GROUP_SET(port_2, vpg);
                } else if (soc_L2_ENTRY_ONLYm_field32_get(unit, l2x_entry, T_1f)) {
                    BCM_GPORT_TRUNK_SET(port_2, 
                        soc_L2_ENTRY_ONLYm_field32_get(unit, l2x_entry, TGID_1f));
                } else {
                    port_in = soc_L2_ENTRY_ONLYm_field32_get(unit, l2x_entry, PORT_NUM_1f);
                    mod_in = soc_L2_ENTRY_ONLYm_field32_get(unit, l2x_entry, MODULE_ID_1f);
                    BCM_IF_ERROR_RETURN
                        (_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_GET,
                                                mod_in, port_in, &mod_out, &port_out));
                    BCM_GPORT_MODPORT_SET(port_2, mod_out, port_out);
                }

                /* Call application call-back */
                rv = cb(unit, outer_vlan, inner_vlan, port_1, port_2, user_data);
#ifdef BCM_CB_ABORT_ON_ERR
                if (BCM_FAILURE(rv) && SOC_CB_ABORT_ON_ERR(unit)) {
                    soc_cm_sfree(unit, buffer);
                    soc_mem_unlock(unit, L2Xm);
                    return rv;
                }
#endif
            }
        }
    }
    soc_cm_sfree(unit, buffer);
    soc_mem_unlock(unit, L2Xm);

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_tr_l2_ppa_setup
 * Description:
 *     Setup hardware L2 PPA registers
 * Parameters:
 *     unit         device number
 *     flags        flags BCM_L2_REPLACE_*
 *     rep_st       structure with information of what to replace
 * Return:
 *     BCM_E_XXX
 */
STATIC int
_bcm_tr_l2_ppa_setup(int unit, _bcm_l2_replace_t *rep_st)
{
    soc_field_t field;
    uint32  rval, ppa_mode;
    uint32  rval_limit, limit_en;

    switch (rep_st->flags &
            (BCM_L2_REPLACE_MATCH_DEST | BCM_L2_REPLACE_MATCH_VLAN |
             BCM_L2_REPLACE_DELETE)) {
    case BCM_L2_REPLACE_MATCH_DEST | BCM_L2_REPLACE_DELETE:
        ppa_mode = 0;
        break;
    case BCM_L2_REPLACE_MATCH_VLAN | BCM_L2_REPLACE_DELETE:
        ppa_mode = 1;
        break;
    case BCM_L2_REPLACE_MATCH_DEST | BCM_L2_REPLACE_MATCH_VLAN |
        BCM_L2_REPLACE_DELETE:
        ppa_mode = 2;
        break;
    case BCM_L2_REPLACE_DELETE:
        ppa_mode = 3;
        break;
    case BCM_L2_REPLACE_MATCH_DEST:
        ppa_mode = 4;
        break;
    case BCM_L2_REPLACE_MATCH_VLAN:
        ppa_mode = 5;
        break;
    case BCM_L2_REPLACE_MATCH_DEST | BCM_L2_REPLACE_MATCH_VLAN :
        ppa_mode = 6;
        break;
    default:
        return BCM_E_PARAM;
    }

    SOC_IF_ERROR_RETURN(READ_PER_PORT_REPL_CONTROLr(unit, &rval));
    if ((soc_feature(unit, soc_feature_mac_learn_limit))) {

        SOC_IF_ERROR_RETURN(READ_SYS_MAC_LIMIT_CONTROLr(unit, &rval_limit));
        limit_en = soc_reg_field_get(unit, SYS_MAC_LIMIT_CONTROLr, 
                                 rval_limit, ENABLEf);
        soc_reg_field_set(unit, PER_PORT_REPL_CONTROLr, &rval, LIMIT_COUNTEDf, 
                          limit_en);
        soc_reg_field_set(unit, PER_PORT_REPL_CONTROLr, &rval, 
                      USE_OLD_LIMIT_COUNTEDf, limit_en);
    }
    soc_reg_field_set(unit, PER_PORT_REPL_CONTROLr, &rval, KEY_TYPEf,
                      rep_st->key_type);
    soc_reg_field_set(unit, PER_PORT_REPL_CONTROLr, &rval, EXCL_STATICf,
                      rep_st->flags & BCM_L2_REPLACE_MATCH_STATIC ? 0 : 1);
    soc_reg_field_set(unit, PER_PORT_REPL_CONTROLr, &rval, EXCL_NON_PENDINGf,
                      rep_st->flags & BCM_L2_REPLACE_PENDING ? 1 : 0);
    soc_reg_field_set(unit, PER_PORT_REPL_CONTROLr, &rval, EXCL_PENDINGf,
                      rep_st->flags & BCM_L2_REPLACE_PENDING ? 0 : 1);
    if (!(rep_st->flags & BCM_L2_REPLACE_DELETE)) {
        if (rep_st->new_dest.trunk != -1) {
            soc_reg_field_set(unit, PER_PORT_REPL_CONTROLr, &rval, Tf, 1);
            soc_reg_field_set(unit, PER_PORT_REPL_CONTROLr, &rval, TGIDf,
                              rep_st->new_dest.trunk);
        } else {
            soc_reg_field_set(unit, PER_PORT_REPL_CONTROLr, &rval, Tf, 0);
            soc_reg_field_set(unit, PER_PORT_REPL_CONTROLr, &rval, MODULE_IDf,
                              rep_st->new_dest.module);
            soc_reg_field_set(unit, PER_PORT_REPL_CONTROLr, &rval, PORT_NUMf,
                              rep_st->new_dest.port);
        }
    }
    SOC_IF_ERROR_RETURN(WRITE_PER_PORT_REPL_CONTROLr(unit, rval));

    rval = 0;
    soc_reg_field_set(unit, PER_PORT_AGE_CONTROLr, &rval, PPA_MODEf,
                      ppa_mode);
    if (rep_st->flags & BCM_L2_REPLACE_MATCH_VLAN) {
        soc_reg_field_set(unit, PER_PORT_AGE_CONTROLr, &rval, VLAN_IDf,
                          rep_st->key_vlan);
    }
    if (rep_st->flags & BCM_L2_REPLACE_MATCH_DEST) {
        if (rep_st->match_dest.trunk != -1) {
            soc_reg_field_set(unit, PER_PORT_AGE_CONTROLr, &rval, Tf, 1);
            soc_reg_field_set(unit, PER_PORT_AGE_CONTROLr, &rval, TGIDf,
                              rep_st->match_dest.trunk);
        } else {
            soc_reg_field_set(unit, PER_PORT_AGE_CONTROLr, &rval, MODULE_IDf,
                              rep_st->match_dest.module);
            soc_reg_field_set(unit, PER_PORT_AGE_CONTROLr, &rval, PORT_NUMf,
                              rep_st->match_dest.port);
        }
    }
    SOC_IF_ERROR_RETURN(WRITE_PER_PORT_AGE_CONTROLr(unit, rval));

    if (SOC_CONTROL(unit)->l2x_mode == L2MODE_FIFO) {
        field = rep_st->flags & BCM_L2_REPLACE_DELETE ?
            L2_MOD_FIFO_ENABLE_PPA_DELETEf : L2_MOD_FIFO_ENABLE_PPA_REPLACEf;
        BCM_IF_ERROR_RETURN(soc_reg_field32_modify
                            (unit, AUX_ARB_CONTROLr, REG_PORT_ANY, field,
                             rep_st->flags & BCM_L2_REPLACE_NO_CALLBACKS ?
                             0 : 1));
    }

    return BCM_E_NONE;
}

#ifdef BCM_TRIUMPH_SUPPORT
/*
 * Function:
 *     _bcm_tr_ext_l2_ppa_setup
 * Description:
 *     Setup hardware external L2 PPA registers
 * Parameters:
 *     unit         device number
 *     rep_st       structure with information of what to replace
 * Return:
 *     BCM_E_XXX
 */
STATIC int
_bcm_tr_ext_l2_ppa_setup(int unit, _bcm_l2_replace_t *rep_st)
{
    uint32 rval, ppa_mode;
    ext_l2_mod_fifo_entry_t ext_l2_mod_entry;
    ext_l2_entry_entry_t ext_l2_entry;

    switch (rep_st->flags &
            (BCM_L2_REPLACE_MATCH_DEST | BCM_L2_REPLACE_MATCH_VLAN |
             BCM_L2_REPLACE_DELETE)) {
    case BCM_L2_REPLACE_MATCH_DEST | BCM_L2_REPLACE_DELETE:
        ppa_mode = 0;
        break;
    case BCM_L2_REPLACE_MATCH_VLAN | BCM_L2_REPLACE_DELETE:
        ppa_mode = 1;
        break;
    case BCM_L2_REPLACE_MATCH_DEST | BCM_L2_REPLACE_MATCH_VLAN |
        BCM_L2_REPLACE_DELETE:
        ppa_mode = 2;
        break;
    case BCM_L2_REPLACE_DELETE:
        ppa_mode = 3;
        break;
    case BCM_L2_REPLACE_MATCH_DEST:
        ppa_mode = 4;
        break;
    case BCM_L2_REPLACE_MATCH_VLAN:
        ppa_mode = 5;
        break;
    case BCM_L2_REPLACE_MATCH_DEST | BCM_L2_REPLACE_MATCH_VLAN :
        ppa_mode = 6;
        break;
    default:
        return BCM_E_PARAM;
    }

    SOC_IF_ERROR_RETURN(READ_ESM_PER_PORT_REPL_CONTROLr(unit, &rval));
    soc_reg_field_set(unit, ESM_PER_PORT_REPL_CONTROLr, &rval, KEY_TYPE_VFIf,
                      rep_st->key_vfi != -1 ? 1 : 0);
    soc_reg_field_set(unit, ESM_PER_PORT_REPL_CONTROLr, &rval, EXCL_STATICf,
                      rep_st->flags & BCM_L2_REPLACE_MATCH_STATIC ? 0 : 1);
    if (!(rep_st->flags & BCM_L2_REPLACE_DELETE)) {
        if (rep_st->new_dest.trunk != -1) {
            soc_reg_field_set(unit, ESM_PER_PORT_REPL_CONTROLr, &rval, Tf, 1);
            soc_reg_field_set(unit, ESM_PER_PORT_REPL_CONTROLr, &rval, TGIDf,
                              rep_st->new_dest.trunk);
        } else {
            soc_reg_field_set(unit, ESM_PER_PORT_REPL_CONTROLr, &rval, Tf, 0);
            soc_reg_field_set(unit, ESM_PER_PORT_REPL_CONTROLr, &rval,
                              MODULE_IDf, rep_st->new_dest.module);
            soc_reg_field_set(unit, ESM_PER_PORT_REPL_CONTROLr, &rval,
                              PORT_NUMf, rep_st->new_dest.port);
        }
    }
    SOC_IF_ERROR_RETURN(WRITE_ESM_PER_PORT_REPL_CONTROLr(unit, rval));

    /*
     * Unlike L2_MOD_FIFO, EXT_L2_MOD_FIFO does not report both new and
     * replaced L2 destination for PPA replace command. To workaround
     * the problem, we add an special entry to EXT_L2_MOD_FIFO before
     * issuing the PPA replace command. The special entry has the new
     * destination and a special "type" value. L2 mod fifo processing
     * thread knows all entries after this special entry are associated
     * with this new destination.
     */
    sal_memset(&ext_l2_mod_entry, 0, sizeof(ext_l2_mod_entry));
    sal_memset(&ext_l2_entry, 0, sizeof(ext_l2_entry));

    if (rep_st->new_dest.trunk != -1) {
        soc_mem_field32_set(unit, EXT_L2_ENTRYm, &ext_l2_entry, Tf, 1);
        soc_mem_field32_set(unit, EXT_L2_ENTRYm, &ext_l2_entry, TGIDf,
                            rep_st->new_dest.trunk);
    } else {
        soc_mem_field32_set(unit, EXT_L2_ENTRYm, &ext_l2_entry, MODULE_IDf,
                            rep_st->new_dest.module);
        soc_mem_field32_set(unit, EXT_L2_ENTRYm, &ext_l2_entry, PORT_NUMf,
                            rep_st->new_dest.port);
    }
    soc_mem_field_set(unit, EXT_L2_MOD_FIFOm, (uint32 *)&ext_l2_mod_entry,
                      WR_DATAf, (uint32 *)&ext_l2_entry);
    /* borrow INSERTED type (value 3) as special mark */
    soc_mem_field32_set(unit, EXT_L2_MOD_FIFOm, &ext_l2_mod_entry, TYPf, 3);
    BCM_IF_ERROR_RETURN(soc_mem_write(unit, EXT_L2_MOD_FIFOm, MEM_BLOCK_ANY,
                                      0, &ext_l2_mod_entry));

    rval = 0;
    soc_reg_field_set(unit, ESM_PER_PORT_AGE_CONTROLr, &rval, PPA_MODEf,
                      ppa_mode);
    if (rep_st->flags & BCM_L2_REPLACE_MATCH_VLAN) {
        soc_reg_field_set(unit, ESM_PER_PORT_AGE_CONTROLr, &rval, VLAN_IDf,
                          rep_st->key_vlan);
    }
    if (rep_st->flags & BCM_L2_REPLACE_MATCH_DEST) {
        if (rep_st->match_dest.trunk != -1) {
            soc_reg_field_set(unit, ESM_PER_PORT_AGE_CONTROLr, &rval, Tf, 1);
            soc_reg_field_set(unit, ESM_PER_PORT_AGE_CONTROLr, &rval, TGIDf,
                              rep_st->match_dest.trunk);
        } else {
            soc_reg_field_set(unit, ESM_PER_PORT_AGE_CONTROLr, &rval,
                              MODULE_IDf, rep_st->match_dest.module);
            soc_reg_field_set(unit, ESM_PER_PORT_AGE_CONTROLr, &rval,
                              PORT_NUMf, rep_st->match_dest.port);
        }
    }
    SOC_IF_ERROR_RETURN(WRITE_ESM_PER_PORT_AGE_CONTROLr(unit, rval));

    return BCM_E_NONE;
}
#endif /* BCM_TRIUMPH_SUPPORT */

/*
 * Function:
 *     _bcm_td_l2_bulk_control_setup
 * Description:
 *     Setup hardware L2 bulk control registers
 * Parameters:
 *     unit         device number
 *     rep_st       structure with information of what to replace
 * Return:
 *     BCM_E_XXX
 */
STATIC int
_bcm_td_l2_bulk_control_setup(int unit, _bcm_l2_replace_t *rep_st)
{
    uint32 rval;

    SOC_IF_ERROR_RETURN(READ_L2_BULK_CONTROLr(unit, &rval)); 
    soc_reg_field_set(unit, L2_BULK_CONTROLr, &rval, ACTIONf, 
                      rep_st->flags & BCM_L2_REPLACE_DELETE ? 1 : 2); 
    if (SOC_CONTROL(unit)->l2x_mode == L2MODE_FIFO) {
        soc_reg_field_set(unit, L2_BULK_CONTROLr, &rval, L2_MOD_FIFO_RECORDf, 
                          rep_st->flags & BCM_L2_REPLACE_NO_CALLBACKS ?
                          0 : 1); 
    }
    SOC_IF_ERROR_RETURN(WRITE_L2_BULK_CONTROLr(unit, rval)); 

    BCM_IF_ERROR_RETURN
        (WRITE_L2_BULK_MATCH_MASKm(unit, MEM_BLOCK_ALL, 0,
                                   &rep_st->match_mask));
    BCM_IF_ERROR_RETURN
        (WRITE_L2_BULK_MATCH_DATAm(unit, MEM_BLOCK_ALL, 0,
                                   &rep_st->match_data));

    if (!(rep_st->flags & BCM_L2_REPLACE_DELETE)) {
        BCM_IF_ERROR_RETURN
            (WRITE_L2_BULK_REPLACE_MASKm(unit, MEM_BLOCK_ALL, 0,
                                         &rep_st->new_mask));
        BCM_IF_ERROR_RETURN
            (WRITE_L2_BULK_REPLACE_DATAm(unit, MEM_BLOCK_ALL, 0,
                                         &rep_st->new_data));
    }

    return BCM_E_NONE;
}

/*
 * Match DEST_TYPE + MODULE_ID + PORT_NUM and replace with one of following:
 * DEST_TYPE = 0, new MODULE_ID, new PORT_NUM
 * DEST_TYPE = 1, new TGID
 * Notes:
 *     Need to match all bits of DEST_TYPE + DESTINATION, therefore can not
 *     match on trunk destination because the unused overlay bits can have any
 *     data.
 */
STATIC int
_bcm_td_l2_bulk_replace_modport(int unit, _bcm_l2_replace_t *rep_st)
{
    l2_bulk_match_mask_entry_t match_mask;
    l2_bulk_match_data_entry_t match_data;
    l2_bulk_replace_mask_entry_t repl_mask;
    l2_bulk_replace_data_entry_t repl_data;
    int field_len;

    sal_memset(&match_mask, 0, sizeof(match_mask));
    sal_memset(&match_data, 0, sizeof(match_data));
    sal_memset(&repl_mask, 0, sizeof(repl_mask));
    sal_memset(&repl_data, 0, sizeof(repl_data));

    soc_mem_field32_set(unit, L2_BULK_MATCH_MASKm, &match_mask, VALIDf, 1);
    soc_mem_field32_set(unit, L2_BULK_MATCH_DATAm, &match_data, VALIDf, 1);

    field_len = soc_mem_field_length(unit, L2_BULK_MATCH_MASKm, KEY_TYPEf);
    soc_mem_field32_set(unit, L2_BULK_MATCH_MASKm, &match_mask, KEY_TYPEf,
                        (1 << field_len) - 1);
    /* KEY_TYPE field in data is 0 */

    field_len = soc_mem_field_length(unit, L2_BULK_MATCH_MASKm, DEST_TYPEf);
    soc_mem_field32_set(unit, L2_BULK_MATCH_MASKm, &match_mask, DEST_TYPEf,
                        (1 << field_len) - 1);
    /* DEST_TYPE field in data is 0 */

    field_len = soc_mem_field_length(unit, L2_BULK_MATCH_MASKm, MODULE_IDf);
    soc_mem_field32_set(unit, L2_BULK_MATCH_MASKm, &match_mask, MODULE_IDf,
                        (1 << field_len) - 1);
    soc_mem_field32_set(unit, L2_BULK_MATCH_DATAm, &match_data, MODULE_IDf,
                        rep_st->match_dest.module);

    field_len = soc_mem_field_length(unit, L2_BULK_MATCH_MASKm, PORT_NUMf);
    soc_mem_field32_set(unit, L2_BULK_MATCH_MASKm, &match_mask, PORT_NUMf,
                        (1 << field_len) - 1);
    soc_mem_field32_set(unit, L2_BULK_MATCH_DATAm, &match_data, PORT_NUMf,
                        rep_st->match_dest.port);

    if (!(rep_st->flags & BCM_L2_REPLACE_MATCH_STATIC)) {
        soc_mem_field32_set(unit, L2_BULK_MATCH_MASKm, &match_mask,
                            STATIC_BITf, 1);
        /* STATIC_BIT field in data is 0 */
    }

    soc_mem_field32_set(unit, L2_BULK_MATCH_MASKm, &match_mask, PENDINGf, 1);
    if (rep_st->flags & BCM_L2_REPLACE_PENDING) {
        soc_mem_field32_set(unit, L2_BULK_MATCH_DATAm, &match_data, PENDINGf,
                            1);
    }

    soc_mem_field32_set(unit, L2_BULK_MATCH_MASKm, &match_mask, EVEN_PARITYf,
                        1);

    BCM_IF_ERROR_RETURN
        (WRITE_L2_BULK_MATCH_MASKm(unit, MEM_BLOCK_ALL, 0, &match_mask));

    field_len = soc_mem_field_length(unit, L2_BULK_REPLACE_MASKm, DEST_TYPEf);
    soc_mem_field32_set(unit, L2_BULK_REPLACE_MASKm, &repl_mask, DEST_TYPEf,
                        (1 << field_len) - 1);

    field_len = soc_mem_field_length(unit, L2_BULK_REPLACE_MASKm, MODULE_IDf);
    soc_mem_field32_set(unit, L2_BULK_REPLACE_MASKm, &repl_mask, MODULE_IDf,
                        (1 << field_len) - 1);

    field_len = soc_mem_field_length(unit, L2_BULK_REPLACE_MASKm, PORT_NUMf);
    soc_mem_field32_set(unit, L2_BULK_REPLACE_MASKm, &repl_mask, PORT_NUMf,
                        (1 << field_len) - 1);

    if (rep_st->new_dest.trunk != -1) {
        soc_mem_field32_set(unit, L2_BULK_REPLACE_DATAm, &repl_data, Tf, 1);
        soc_mem_field32_set(unit, L2_BULK_REPLACE_DATAm, &repl_data, TGIDf,
                            rep_st->new_dest.trunk);
    } else {
        /* T field in data is 0 */
        soc_mem_field32_set(unit, L2_BULK_REPLACE_DATAm, &repl_data,
                            MODULE_IDf, rep_st->new_dest.module);
        soc_mem_field32_set(unit, L2_BULK_REPLACE_DATAm, &repl_data, PORT_NUMf,
                            rep_st->new_dest.port);
    }

    soc_mem_field32_set(unit, L2_BULK_REPLACE_MASKm, &repl_mask, EVEN_PARITYf,
                        1);
    BCM_IF_ERROR_RETURN
        (WRITE_L2_BULK_REPLACE_MASKm(unit, MEM_BLOCK_ALL, 0, &repl_mask));

    BCM_IF_ERROR_RETURN
        (soc_reg_field32_modify(unit, L2_BULK_CONTROLr, REG_PORT_ANY, ACTIONf,
                                2));

    /* Replace all entries having EVEN_PARITY == 0 */
    /* EVEN_PARITY field in data is 0 */
    BCM_IF_ERROR_RETURN
        (WRITE_L2_BULK_MATCH_DATAm(unit, MEM_BLOCK_ALL, 0, &match_data));

    soc_mem_field32_set(unit, L2_BULK_REPLACE_DATAm, &repl_data, EVEN_PARITYf,
                        1);
    BCM_IF_ERROR_RETURN
        (WRITE_L2_BULK_REPLACE_DATAm(unit, MEM_BLOCK_ALL, 0, &repl_data));

    BCM_IF_ERROR_RETURN(soc_l2x_port_age(unit, L2_BULK_CONTROLr, INVALIDr));

    /* Replace all entries having EVEN_PARITY == 1 */
    soc_mem_field32_set(unit, L2_BULK_MATCH_DATAm, &match_data, EVEN_PARITYf,
                        1);
    BCM_IF_ERROR_RETURN
        (WRITE_L2_BULK_MATCH_DATAm(unit, MEM_BLOCK_ALL, 0, &match_data));

    soc_mem_field32_set(unit, L2_BULK_REPLACE_DATAm, &repl_data, EVEN_PARITYf,
                        0);
    BCM_IF_ERROR_RETURN
        (WRITE_L2_BULK_REPLACE_DATAm(unit, MEM_BLOCK_ALL, 0, &repl_data));

    BCM_IF_ERROR_RETURN(soc_l2x_port_age(unit, L2_BULK_CONTROLr, INVALIDr));

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_tr_l2_replace_by_hw
 * Description:
 *     Helper function to _bcm_l2_replace_by_hw
 * Parameters:
 *     unit         device number
 *     rep_st       structure with information of what to replace
 * Return:
 *     BCM_E_XXX
 */
int
_bcm_tr_l2_replace_by_hw(int unit, _bcm_l2_replace_t *rep_st)
{
    soc_control_t *soc = SOC_CONTROL(unit);
    soc_reg_t int_reg, ext_reg;
    int rv;
    int bulk_replace_modport;
    uint32 parity_diff;
#if defined(BCM_TRIUMPH_SUPPORT)
    int do_l2e_ppa_match = 1;
#endif	/* BCM_TRIUMPH_SUPPORT*/

    if (NULL == rep_st) {
        return BCM_E_PARAM;
    }

    bulk_replace_modport = FALSE;
    if (soc_feature(unit, soc_feature_l2_bulk_bypass_replace)) {
        if ((rep_st->flags &
             (BCM_L2_REPLACE_MATCH_DEST | BCM_L2_REPLACE_MATCH_VLAN |
              BCM_L2_REPLACE_DELETE)) == BCM_L2_REPLACE_MATCH_DEST) {
            if (rep_st->match_dest.trunk == -1) {
                /* Find out if the new data will cause parity bit change */
                if (rep_st->new_dest.trunk != -1) {
                    parity_diff =
                        rep_st->match_dest.module ^ rep_st->match_dest.port ^
                        rep_st->new_dest.trunk ^ 1;
                } else {
                    parity_diff = 
                        rep_st->match_dest.module ^ rep_st->match_dest.port ^
                        rep_st->new_dest.module ^ rep_st->new_dest.port;
                }
                parity_diff ^= parity_diff >> 4;
                parity_diff ^= parity_diff >> 2;
                parity_diff ^= parity_diff >> 1;
                bulk_replace_modport = parity_diff & 1 ? TRUE : FALSE;
            }
        }
    }

    if (bulk_replace_modport) {
        return _bcm_td_l2_bulk_replace_modport(unit, rep_st);
    } else {
        ext_reg = INVALIDr;
        if (soc_feature(unit, soc_feature_l2_bulk_control)) {
            BCM_IF_ERROR_RETURN(_bcm_td_l2_bulk_control_setup(unit, rep_st));
            int_reg = L2_BULK_CONTROLr;
        } else {
            BCM_IF_ERROR_RETURN(_bcm_tr_l2_ppa_setup(unit, rep_st));
            int_reg = PER_PORT_AGE_CONTROLr;

#if defined(BCM_TRIUMPH_SUPPORT)
            if (soc_mem_is_valid(unit, EXT_L2_ENTRYm) &&
                soc_mem_index_count(unit, EXT_L2_ENTRYm) > 0) {
                if (do_l2e_ppa_match) {
                    BCM_IF_ERROR_RETURN(_bcm_tr_l2e_ppa_match(unit, rep_st));
                } else {
                    BCM_IF_ERROR_RETURN
                        (_bcm_tr_ext_l2_ppa_setup(unit, rep_st));
                    ext_reg = ESM_PER_PORT_AGE_CONTROLr;
                }
            }
#endif	/* BCM_TRIUMPH_SUPPORT*/
        }

#ifdef PLISIM
#ifdef _KATANA_DEBUG /* BCM_KATANA_SUPPORT */
	_bcm_kt_enable_port_age_simulation(flags,rep_st);
#endif
#endif
        BCM_IF_ERROR_RETURN(soc_l2x_port_age(unit, int_reg, ext_reg));

        if (!(rep_st->flags & BCM_L2_REPLACE_DELETE)) {
            return BCM_E_NONE;
        }

        if (SOC_L2_DEL_SYNC_LOCK(soc) < 0) {
            return BCM_E_RESOURCE;
        }
        rv = _soc_l2x_sync_replace
            (unit, &rep_st->match_data, &rep_st->match_mask,
             rep_st->flags & BCM_L2_REPLACE_NO_CALLBACKS ?
             SOC_L2X_NO_CALLBACKS : 0);

        SOC_L2_DEL_SYNC_UNLOCK(soc);
    }

    return rv;
}

#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
/*
 * Function:
 *     _bcm_tr_l2_sw_dump
 * Purpose:
 *     Displays L2 information maintained by software.
 * Parameters:
 *     unit - Device unit number
 * Returns:
 *     None
 */
void
_bcm_tr_l2_sw_dump(int unit)
{
    _bcm_mac_block_info_t *mbi;
     char                 pfmt[SOC_PBMP_FMT_LEN];
     int                  i;

    soc_cm_print("\n");
    soc_cm_print("  TR L2 MAC Blocking Info -\n");
    soc_cm_print("      Number : %d\n", _mbi_num[unit]);

    mbi = _mbi_entries[unit];
    soc_cm_print("      Entries (index: pbmp-count) :\n");
    if (mbi != NULL) {
        for (i = 0; i < _mbi_num[unit]; i++) {
            SOC_PBMP_FMT(mbi[i].mb_pbmp, pfmt);
            soc_cm_print("          %5d: %s-%d\n", i, pfmt, mbi[i].ref_count);
        }
    }
    soc_cm_print("\n");

    soc_cm_print("\n  TR L2 PPA bypass - %s\n",
                 SOC_CONTROL(unit)->l2x_ppa_bypass ? "TRUE" : "FALSE");
    return;
}
#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */

#else /* BCM_TRX_SUPPORT */
int bcm_esw_triumph_l2_not_empty;
#endif  /* BCM_TRX_SUPPORT */
