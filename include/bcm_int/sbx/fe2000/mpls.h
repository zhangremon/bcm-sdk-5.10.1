/*
 * $Id: mpls.h 1.22 Broadcom SDK $
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
 */

#ifndef _BCM_INT_SBX_FE2000_MPLS_H_
#define _BCM_INT_SBX_FE2000_MPLS_H_

#include <bcm_int/sbx/fe2000/l3.h>
#include <bcm_int/sbx/fe2000/qos.h>
#include <bcm/mpls.h>
#include <bcm/trunk.h>

/* FE2k g2p3 supports 16bits of mpls label for vsi matching.  However, the
 * MSb signals to the PPE that the label is a PWE3 label
 */
#define _FE2K_MPLS_LABEL(_lbl) ((_lbl) & _FE2K_L3_MPLS_LBL_MASK)
#define _FE2K_MPLSTP_LABEL(_lbl) ((_lbl) & _FE2K_L3_MPLSTP_LBL_MASK)

#define _FE2K_MPLS_PORT_FTE_VALID(unit, fte)             \
      (((fte) >= SBX_GLOBAL_GPORT_FTE_BASE(unit) &&      \
        (fte) <= SBX_GLOBAL_GPORT_FTE_END(unit))     ||  \
       ((fte) >= SBX_LOCAL_GPORT_FTE_BASE(unit)  &&      \
        (fte) <= SBX_LOCAL_GPORT_FTE_END(unit))      ||  \
       ((SOC_SBX_CFG(unit)->mplstp_ena) &&               \
        (fte >= SBX_VPWS_UNI_FTE_BASE(unit)) &&          \
        (fte <= SBX_VPWS_UNI_FTE_END(unit)))         ||  \
       ((SOC_SBX_CFG(unit)->mplstp_ena) &&               \
        (fte >= SBX_DYNAMIC_FTE_BASE(unit)) &&           \
        (fte <= SBX_DYNAMIC_FTE_END(unit)))          ||  \
       ((fte) >= SBX_DYNAMIC_VSI_FTE_BASE(unit) &&       \
        (fte) <= SBX_DYNAMIC_VSI_FTE_END(unit)))

typedef enum {
    _FE2K_MPLS_PORT_MATCH_ADD,
    _FE2K_MPLS_PORT_MATCH_UPDATE,
    _FE2K_MPLS_PORT_MATCH_DELETE,
    _FE2K_MPLS_PORT_MATCH_MAX
} _fe2k_mpls_port_action_type;

/* Maximum number of PRIMARY Service Access points
 * valid on a MPLS VPWS private line service */
#define _BCM_FE2K_VPWS_MAX_SAP (2)

/* 
 * Routines and macros shared by ucode impls
 */
#define _BCM_MPLS_VPN_TYPE(_flags)                \
    ((_flags) & (BCM_MPLS_VPN_L3   |              \
                 BCM_MPLS_VPN_VPLS |              \
                 BCM_MPLS_VPN_VPWS))

/* make sure only ONE type of vpn is set in flags */
#define _BCM_MPLS_VPN_VALID_TYPE(_flags)                                \
     (_BCM_MPLS_VPN_TYPE(_flags) &&                                      \
      ((_BCM_MPLS_VPN_TYPE(_flags) & (_BCM_MPLS_VPN_TYPE(_flags) - 1)) == 0))

#define _BCM_MPLS_VPN_VALID_USER_HANDLE(unit, _vpn_id) \
    ( (_vpn_id) >= SBX_DYNAMIC_VSI_BASE(unit) &&        \
      (_vpn_id) <= (SBX_DYNAMIC_VSI_END(unit) * 2))


/*
 *  Unhappily, while the create API for ExpMap has a parameter that says the
 *  map is ingress or egress, the delete API does not have such a thing.  So,
 *  we need to add some additional data to the map ID that's passed back so we
 *  can tell the difference.  Of course, we'll need to mask it off when writing
 *  it to the proper places, as well.  Much work for little omission.
 */
/*
#define _MPLS_EXPMAP_HANDLE_TYPE_MASK 0xFFF00000
#define _MPLS_EXPMAP_HANDLE_TYPE_INGR 0x1E300000
#define _MPLS_EXPMAP_HANDLE_TYPE_EGR  0xEE300000
#define _MPLS_EXPMAP_HANDLE_TYPE(h) (_MPLS_EXPMAP_HANDLE_TYPE_MASK & (h))
*/
#define _MPLS_EXPMAP_HANDLE_DATA(h) \
    ((_MPLS_EXPMAP_HANDLE_IS_INGR(h)) ? (h) : ((h) - QOS_MAP_ID_EGRESS_OFFSET))

#define _MPLS_EXPMAP_HANDLE_IS_INGR(h) ((h) < QOS_MAP_ID_EGRESS_OFFSET)
#define _MPLS_EXPMAP_HANDLE_IS_EGR(h) ((h) >= QOS_MAP_ID_EGRESS_OFFSET)

#define _MPLS_EXPMAP_HANDLE_IS_VALID(h) (_MPLS_EXPMAP_HANDLE_IS_EGR(h) || \
                                         _MPLS_EXPMAP_HANDLE_IS_INGR(h))
#define _MPLS_EXPMAP_HANDLE_MAKE_INGR(h) \
              (_bcm_fe2000_qos_hw_id_to_map_id(BCM_QOS_MAP_INGRESS, h))
#define _MPLS_EXPMAP_HANDLE_MAKE_EGR(h) \
              (_bcm_fe2000_qos_hw_id_to_map_id(BCM_QOS_MAP_EGRESS, h))

#define MAX_LBL_OPCODE (4)

typedef enum {
    _BCM_FE2K_LABEL_OPCODE_PWE,
    _BCM_FE2K_LABEL_OPCODE_CES,        
    _BCM_FE2K_LABEL_OPCODE_LER,
    _BCM_FE2K_LABEL_OPCODE_LSR
} _bcm_fe2k_mplstp_opcode_t;

extern uint32 _sbx_mplstp_lbl_opcode[BCM_MAX_NUM_UNITS][MAX_LBL_OPCODE];

typedef struct {
    dq_t plist;
    bcm_trunk_add_info_t add_info;
} bcm_fe2k_mpls_trunk_association_t;

extern bcm_fe2k_mpls_trunk_association_t 
mpls_trunk_assoc_info[BCM_MAX_NUM_UNITS][SBX_MAX_TRUNKS];


#define _BCM_FE2K_LABEL_LER(unit) _sbx_mplstp_lbl_opcode[unit][_BCM_FE2K_LABEL_OPCODE_LER]
#define _BCM_FE2K_LABEL_LSR(unit) _sbx_mplstp_lbl_opcode[unit][_BCM_FE2K_LABEL_OPCODE_LSR]
#define _BCM_FE2K_LABEL_PWE(unit) _sbx_mplstp_lbl_opcode[unit][_BCM_FE2K_LABEL_OPCODE_PWE]
#define _BCM_FE2K_LABEL_CES(unit) _sbx_mplstp_lbl_opcode[unit][_BCM_FE2K_LABEL_OPCODE_CES]

#define _BCM_FE2K_IS_LABEL_LER(unit, opcode) \
                  ((_sbx_mplstp_lbl_opcode[unit][_BCM_FE2K_LABEL_OPCODE_LER] == (opcode)) ? 1:0)


extern int
_bcm_fe2000_mpls_vpn_stp_set(int        unit,
                             bcm_vpn_t  vpn,
                             bcm_port_t port,
                             int        stp_state);
extern int
_bcm_fe2000_mpls_vpn_stp_get(int        unit,
                             bcm_vpn_t  vpn,
                             bcm_port_t port,
                             int        *stp_state);

int
_bcm_fe2000_mpls_alloc_vpn_sap_hw_resources(_fe2k_l3_fe_instance_t  *l3_fe,
                                            _fe2k_vpn_sap_t         *vpn_sap,
                                            bcm_mpls_port_t         *mpls_port);
int
_bcm_fe2000_mpls_free_vpn_sap_hw_resources(_fe2k_l3_fe_instance_t  *l3_fe,
                                           _fe2k_vpn_sap_t        *vpn_sap);
int
_bcm_fe2000_mpls_gport_get_mod_port(int            unit,
                                    bcm_gport_t    gport,
                                    bcm_module_t  *modid,
                                    bcm_port_t    *port);

extern int
_bcm_fe2000_fill_mpls_label_array_from_ete_idx(_fe2k_l3_fe_instance_t *l3_fe,
                                               uint32                   ete_idx,
                                               int                      label_max,
                                               bcm_mpls_egress_label_t *label_array,
                                               int                     *label_count);


extern int
_bcm_fe2000_mpls_port_vlan_vector_set(int unit,
                                      bcm_gport_t gport,
                                      bcm_vlan_vector_t vlan_vec);

extern int
_bcm_fe2000_mpls_port_vlan_vector_get(int unit,
                                      bcm_gport_t gport,
                                      bcm_vlan_vector_t vlan_vec);

extern int
_bcm_fe2000_mpls_port_gport_attr_get(int unit,
                                     bcm_gport_t gport,
                                     bcm_port_t *phy_port,
                                     bcm_vlan_t *match_vlan,
                                     bcm_vlan_t *vsi);

extern int
_bcm_fe2000_mpls_port_info_get(int unit, bcm_vlan_t vsi, uint8_t vpwsuni, int *keepUntagged);

extern int
_bcm_fe2000_mpls_port_vlan_vector_internal(int unit,
                                           bcm_gport_t gport,
                                           bcm_port_t *phy_port,
                                           bcm_vlan_t *match_vlan,
                                           bcm_vlan_t *vsi,
                                           uint32_t   *logicalPort,
                                           bcm_mpls_port_t *mpls_port,
                                           uint8_t    *vpwsuni);

extern int
_bcm_fe2000_find_mpls_vpncb_by_id(int                      unit,
                                  _fe2k_l3_fe_instance_t  *l3_fe,
                                  bcm_vpn_t                vpn_id,
                                  _fe2k_vpn_control_t    **vpnc);

extern int
bcm_fe2000_mpls_port_qosmap_set(int unit,
                                bcm_gport_t gport,
                                int ingrMap,
                                int egrMap,
                                uint32 ingFlags, 
                                uint32 egrFlags);

extern int
bcm_fe2000_mpls_port_qosmap_get(int unit, bcm_gport_t gport, 
                                int *ing_idx, int *egr_idx,
                                uint32 *ing_flags, uint32 *egr_flags);

/*
 *   Function
 *      bcm_fe2000_g2p3_mpls_port_get_lpid
 *   Purpose
 *      Find a logical port based upon the provided MPLS GPORT ID
 *   Parameters
 *      (in) unit      = BCM device number
 *      (in) gport     = MPLS GPORT to be found
 *      (out) lpid     = LP ID for the GPORT
 *      (out) pport    = physical port for the GPORT
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
extern int
bcm_fe2000_g2p3_mpls_port_get_lpid(int unit,
                                   bcm_gport_t gport,
                                   uint32 *lpid,
                                   bcm_port_t *pport);

/*
 *  Function:
 *    bcm_fe2000_mpls_gport_get
 *  Description:
 *    Get the specified MPLS gport information
 *  Parameters:
 *    in int unit - unit to access
 *    in const bcm_gport_t gport - MPLS GPort to translate
 *    _fe2k_vpn_sap_t * 
 *  Returns:
 *    BCM_E_NONE for success
 *    BCM_E_* as appropriate
 */
extern int
bcm_fe2000_mpls_gport_get(int unit,
                          bcm_gport_t gport,
                          _fe2k_vpn_sap_t **vpn_sap);

#define _BCM_FE2000_VPWS_UNI_OFFSET (16*1024)

extern int 
_bcm_fe2000_g2p3_mplstp_vpws_ft_lp_offset(int         unit,
                                          bcm_gport_t gport,
                                          uint32_t   *lport,
                                          uint32_t   *fte);

extern int
_bcm_fe2000_find_vpn_sap_by_id(_fe2k_l3_fe_instance_t *l3_fe,
                               _fe2k_vpn_control_t   *vpnc,
                               bcm_gport_t            mpls_port_id,
                               _fe2k_vpn_sap_t      **vpn_sap);

#endif /* _BCM_INT_SBX_FE2000_MPLS_H_ */
