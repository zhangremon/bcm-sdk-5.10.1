/*
 * $Id: l2.c 1.115.2.11 Broadcom SDK $
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
 * L2 - Broadcom RoboSwitch Layer-2 switch API.
 */
#include <sal/types.h>
#include <sal/appl/io.h>

#include <soc/mem.h>
#include <soc/drv.h>

#include <soc/l2x.h>
#include <soc/feature.h>
#include <soc/macipadr.h>
#include <soc/debug.h>
#include <soc/arl.h>
#include <soc/mcm/robo/memregs.h>

#include <bcm/error.h>
#include <bcm/types.h>
#include <bcm/l2.h>
#include <bcm/trunk.h>
#include <bcm/vlan.h>
#include <bcm/stack.h>
#include <bcm/switch.h>
#include <bcm_int/common/multicast.h>
#include <bcm_int/robo/l2.h>
#include <bcm_int/robo/trunk.h>
#include <bcm_int/robo/port.h>
#include <bcm_int/robo/subport.h>
#include <bcm_int/robo_dispatch.h>

/****************************************************************
 *
 * L2 software tables, per unit.
 *
 ****************************************************************/
static int _l2_init[BCM_MAX_NUM_UNITS];

/*
 * Define:
 *	L2_INIT
 * Purpose:
 *	Causes a routine to return BCM_E_INIT (or some other
 *	error) if L2 software module is not yet initialized.
 */

#define L2_INIT(unit) do { \
	if (_l2_init[unit] < 0) { \
	    soc_cm_print("%s\n",FUNCTION_NAME());\
	    return _l2_init[unit];} \
	if (_l2_init[unit] == 0) { \
	    soc_cm_print("%s\n",FUNCTION_NAME());\
	    return BCM_E_INIT;} \
	} while (0);


/*
 * Function:
 *	_bcm_robo_l2_gport_parse
 * Description:
 *	Parse gport in the l2 table
 * Parameters:
 *	unit -      [IN] ROBOSwitch PCI device unit number (driver internal).
 *  l2addr -    [IN/OUT] l2 addr structure to parse and fill
 *  params -  [OUT] report the value per gport type indicated.
 * Returns:
 *	BCM_E_XXX
 */
int 
_bcm_robo_l2_gport_parse(int unit, bcm_l2_addr_t *l2addr, 
        _bcm_robo_l2_gport_params_t *params)
{
    int             id = 0;
    bcm_port_t      _port;
    bcm_trunk_t     _trunk;
    bcm_module_t    _modid;

    if ((NULL == l2addr) || (NULL == params)){
        return BCM_E_PARAM;
    }

    params->param0 = -1;
    params->param1 = -1;
    params->type = 0;

    BCM_IF_ERROR_RETURN(
        _bcm_robo_gport_resolve(unit, l2addr->port, &_modid, &_port, 
                               &_trunk, &id));

    if ((-1 != _port) && (-1 == id)) {
        params->param0 = _port;
        if (_port == CMIC_PORT(unit)) {
            params->type = _SHR_GPORT_TYPE_LOCAL_CPU;
            return BCM_E_NONE;
        }
        params->param1 = _modid;
        params->type = _SHR_GPORT_TYPE_MODPORT;
        return BCM_E_NONE;
    }

    if (-1 != id) {
        if (BCM_GPORT_IS_SUBPORT_GROUP(l2addr->port)) {
            /* for ROBO device with SUBPORT_GROUP_TYPE, the "id" after the 
             * gport routine, "_bcm_robo_gport_resolve", will be mc_group id.
             */
            params->type = _SHR_GPORT_TYPE_SUBPORT_GROUP;
            params->param0 = _port;
            params->param1 = id;    /* mc_group id */
            
        } else if (BCM_GPORT_IS_SUBPORT_PORT(l2addr->port)){
            /* for ROBO device with SUBPORT_GROUP_TYPE, the "id" after the 
             * gport routine, "_bcm_robo_gport_resolve", will be vport id.
             */
            params->type = _SHR_GPORT_TYPE_SUBPORT_PORT;
            params->param0 = _port;
            params->param1 = id;    /* vp_id */
        } 
        return BCM_E_NONE;
    }

    return BCM_E_PORT;
}

/* 
 * Function:
 *	_bcm_robo_l2_to_arl
 * Purpose:
 *	Translate hardware-independent L2 entry to ROBO-specific ARL entry
 * Parameters:
 *	unit - Unit number
 *	arl_entry - (OUT) ROBO ARL entry
 *	l2addr - Hardware-independent L2 entry
 */
int
_bcm_robo_l2_to_arl(int unit, l2_arl_sw_entry_t *arl_entry, 
        bcm_l2_addr_t *l2addr)
{
    
    uint64  mac_field;
    uint32  fld_value = 0;
    uint32  no_support_flag = 0;
    uint32  age_hit_val = 0;
    _bcm_robo_l2_gport_params_t  g_params;

    sal_memset(arl_entry, 0, sizeof (*arl_entry));
    COMPILER_64_ZERO(mac_field);

    soc_cm_debug(DK_ARL|DK_TESTS, "%s, flags is 0x%x\n", 
            FUNCTION_NAME(), l2addr->flags);

    no_support_flag = BCM_L2_DISCARD_SRC | BCM_L2_DISCARD_DST | 
            BCM_L2_COPY_TO_CPU | BCM_L2_L3LOOKUP | BCM_L2_TRUNK_MEMBER | 
            BCM_L2_MIRROR | BCM_L2_LEARN_LIMIT_EXEMPT;
    
    if (soc_feature(unit, soc_feature_arl_mode_control)) {
        no_support_flag &= ~BCM_L2_DISCARD_SRC;
        no_support_flag &= ~BCM_L2_DISCARD_DST;
        no_support_flag &= ~BCM_L2_COPY_TO_CPU;
    }

    if (l2addr->flags & no_support_flag) {
        return BCM_E_UNAVAIL;
    }

    SAL_MAC_ADDR_TO_UINT64(l2addr->mac, mac_field);
    /* set MAC field */
    
    BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
            DRV_MEM_FIELD_MAC, (uint32 *)arl_entry, (uint32 *)&mac_field));

    /* valid VLAN check */
    if(l2addr->vid > BCM_VLAN_MAX){
        return BCM_E_PARAM;
    }
    
    /* set VID field */
    fld_value = l2addr->vid; /* vid value */
    BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL,
            DRV_MEM_FIELD_VLANID, (uint32 *)arl_entry, &fld_value));

    /* valid 802.1p CoS check */
    if(l2addr->cos_dst < BCM_PRIO_MIN || l2addr->cos_dst > BCM_PRIO_MAX){
        return BCM_E_PARAM;
    }
    
    if (!SOC_IS_TBX(unit)){
        fld_value = l2addr->cos_dst; /* priority value */
         /* set priority field */
        BCM_IF_ERROR_RETURN(
                DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL,
                DRV_MEM_FIELD_PRIORITY, (uint32 *)arl_entry, &fld_value));
    }

    /* set static field */
    if (l2addr->flags & BCM_L2_STATIC) {
        fld_value = 0x1; /* static value */
        BCM_IF_ERROR_RETURN(
                DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_STATIC, 
                (uint32 *)arl_entry, &fld_value));
        soc_cm_debug(DK_ARL|DK_TESTS, "\t Static flags set in l2addr\n");
    }

    /* set valid field */
    fld_value = 0x1; /* valid value */
    if (soc_feature(unit, soc_feature_l2_pending)) {
        if (SOC_IS_TBX(unit)){
#ifdef  BCM_TB_SUPPORT
            fld_value = (l2addr->flags & BCM_L2_PENDING) ? 
                    _TB_ARL_STATUS_PENDING : _TB_ARL_STATUS_VALID;
#endif
        } else {
            soc_cm_debug(DK_ARL|DK_TESTS, "\t No Pending learning feature!!\n");
            return BCM_E_UNAVAIL;
        }
    }
    BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL,
            DRV_MEM_FIELD_VALID, (uint32 *)arl_entry, &fld_value));

    BCM_IF_ERROR_RETURN(
            DRV_DEV_PROP_GET(unit, DRV_DEV_PROP_AGE_HIT_VALUE, &age_hit_val));

    if ((l2addr->flags & BCM_L2_HIT) || (l2addr->flags & BCM_L2_SRC_HIT)) {
        fld_value = age_hit_val; /* age value */
        /* set hit/age field */
        BCM_IF_ERROR_RETURN(
                DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_AGE, 
                (uint32 *)arl_entry, &fld_value));
        soc_cm_debug(DK_ARL|DK_TESTS,  "\t Age flags set in l2addr\n");
    
    }

    if (soc_feature(unit, soc_feature_arl_mode_control)) {

        /* the arl_mode_control feature for ROBO chips were designed to serve
         *  the control on one specific mode only for each l2 entry.
         *  - That is, not like ESW design, to request one mode control modes  
         *      on ROBO chips will be treated as conflict configuration.
         */
        if (l2addr->flags & BCM_L2_COPY_TO_CPU) {
            if ((l2addr->flags & BCM_L2_DISCARD_DST) || 
                    (l2addr->flags & BCM_L2_DISCARD_SRC)){
                return (BCM_E_CONFIG);
            } else {
                fld_value = 0x3; 
            }
        } else if (l2addr->flags & BCM_L2_DISCARD_DST) {
            if (l2addr->flags & BCM_L2_DISCARD_SRC){
                return (BCM_E_CONFIG);
            } else {
                fld_value = 0x1; 
            }
        }else if (l2addr->flags & BCM_L2_DISCARD_SRC) {
            fld_value = 0x2;
        }else {
            fld_value = 0; 
        }

        /* set arl_control field */
        BCM_IF_ERROR_RETURN(
                DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                DRV_MEM_FIELD_ARL_CONTROL, (uint32 *)arl_entry, &fld_value));
        soc_cm_debug(DK_ARL|DK_TESTS,  "\t arl_control flags set in l2addr %x\n",
                fld_value);
    }

    /* Destination translate section :
     *  - Mcast : l2mc_index or PBMP
     *  - Unicast : port_id
     */
    if (BCM_MAC_IS_MCAST(l2addr->mac)){
        /* Mcast */
        if (l2addr->flags & BCM_L2_MCAST) {
            if (_BCM_MULTICAST_IS_SET(l2addr->l2mc_index)) {
                if (_BCM_MULTICAST_IS_L2(l2addr->l2mc_index)) {
                    fld_value = _BCM_MULTICAST_ID_GET(l2addr->l2mc_index);
                } else {
                    /* No other multicast types in L2 on this device */
                    return BCM_E_PARAM;
                }
            } else {
                fld_value = l2addr->l2mc_index;
            }
            BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET(unit,
                    DRV_MEM_MARL, DRV_MEM_FIELD_DEST_BITMAP,
                    (uint32 *)arl_entry, &fld_value));
        }

        /* for those ROBO chips have no MARL_PBMP table (GE switch devices), 
         *  the DEST_PBMP field will remaining at 0x0 in this process.
         */
    }else {
        /* unicast */

        /* port parsing : GPORT(LOCAL|SUBPORT)/NON-GPORT */
        if (!BCM_GPORT_IS_SET(l2addr->port)) {
            soc_cm_debug(DK_ARL|DK_TESTS, "%s,%d, port is %d\n",
                    FUNCTION_NAME(), __LINE__, l2addr->port);
            if (!SOC_PORT_ADDRESSABLE(unit, l2addr->port)) {
                return BCM_E_PORT;
            }
            
            /* set Port fields */
            fld_value = l2addr->port;
            BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                    DRV_MEM_FIELD_SRC_PORT, (uint32 *)arl_entry, &fld_value));
        } else {
            BCM_IF_ERROR_RETURN(
                    _bcm_robo_l2_gport_parse(unit, l2addr, &g_params));
            
            switch (g_params.type) {
                case _SHR_GPORT_TYPE_LOCAL_CPU:
                case _SHR_GPORT_TYPE_MODPORT:
                    if (!SOC_PORT_ADDRESSABLE(unit, g_params.param0)) {
                        return BCM_E_PORT;
                    }
                    /* port */
                    fld_value = g_params.param0;
                    soc_cm_debug(DK_ARL|DK_TESTS, "%s,%d, port is %d\n",
                            FUNCTION_NAME(), __LINE__, fld_value);
                    /* set Port fields */
                    BCM_IF_ERROR_RETURN(
                            DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                            DRV_MEM_FIELD_SRC_PORT, (uint32 *)arl_entry, 
                            &fld_value));
                    break;
                case _SHR_GPORT_TYPE_SUBPORT_GROUP:
                    /* with this type, param0 is port and param1 is mc_group */ 
                    /* mcast group_id */
                    fld_value = g_params.param1;
                    soc_cm_debug(DK_ARL|DK_TESTS, "%s,%d, port is %d\n",
                            FUNCTION_NAME(), __LINE__, fld_value);
                    /* set mgid fields */
                    BCM_IF_ERROR_RETURN(
                            DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                            DRV_MEM_FIELD_DEST_BITMAP, (uint32 *)arl_entry, 
                            &fld_value));
                    break;
                case _SHR_GPORT_TYPE_SUBPORT_PORT:
                    /* with this type, param0 is port and param1 is vport_id */ 
                    if (!SOC_PORT_ADDRESSABLE(unit, g_params.param0)) {
                        return BCM_E_PORT;
                    }
                    /* port */
                    fld_value = g_params.param0;
                    soc_cm_debug(DK_ARL|DK_TESTS, "%s,%d, port is %d\n",
                            FUNCTION_NAME(), __LINE__, fld_value);
                    /* set Port fields */
                    BCM_IF_ERROR_RETURN(
                            DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                            DRV_MEM_FIELD_SRC_PORT, (uint32 *)arl_entry, 
                            &fld_value));
                            
                    if(SOC_IS_TBX(unit)){
                        /* vp_id is the item in bcm_l2_addr_t to support TB's
                         *  ARL feature of vport_id field.
                         *
                         * Note : 
                         *  TB's vport_id is 0-15 for each physical port. the 
                         *  0-15 vp_id can be retrieved from gport format 
                         *  vport_id(system basis vport_id)
                         */
#ifdef  BCM_TB_SUPPORT
                        /* vport */
                        fld_value = g_params.param1;
                        soc_cm_debug(DK_ARL|DK_TESTS, 
                                "%s,%d, virtual port is %d\n",
                                FUNCTION_NAME(), __LINE__, fld_value);
                        /* set Port fields */
                        BCM_IF_ERROR_RETURN(
                                DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                                DRV_MEM_FIELD_VPORT, (uint32 *)arl_entry, 
                                &fld_value));
#endif  /* BCM_TB_SUPPORT */
                    }
                    break;
                default :
                    return BCM_E_PORT;
            }
        }
    }
    soc_cm_debug(DK_ARL|DK_TESTS,"%s,%d,arl_entry[2-0]=%08x-%08x-%08x\n", 
            FUNCTION_NAME(), __LINE__, *((uint32 *)arl_entry+2), 
            *((uint32 *)arl_entry+1),*(uint32 *)arl_entry);
    return BCM_E_NONE;
}

/*
 * Function:
 *	_bcm_robo_l2_from_arl
 * Purpose:
 *	Translate ROBO-specific ARL entry to hardware-independent L2 entry
 * Parameters:
 *	unit - Unit number
 *	l2addr - (OUT) hardware-independent L2 entry
 *	arl_entry - ROBO ARL entry
 */
void
_bcm_robo_l2_from_arl(int unit, 
                bcm_l2_addr_t *l2addr, l2_arl_sw_entry_t *arl_entry)
{
    int     rv = BCM_E_NONE;
    uint64 mac_field;
    uint32 field_value = 0, src_port = 0;
    uint32 age_hit_val = 0;
    bcm_gport_t l2_gport = BCM_GPORT_INVALID;

    _bcm_gport_dest_t   dest;
    int     useGport = 0;

    /* keep the l2addr->port for the usage on verifying if any GPORT type 
     * indicated to represent user's request.
     */
    src_port = l2addr->port;
    sal_memset(l2addr, 0, sizeof (bcm_l2_addr_t));
    l2addr->port = src_port;
    
    COMPILER_64_ZERO(mac_field);

    soc_cm_debug(DK_ARL|DK_TESTS,"%s,%d,arl_entry[2-0]=%08x-%08x-%08x\n", 
            FUNCTION_NAME(), __LINE__, *((uint32 *)arl_entry+2), 
            *((uint32 *)arl_entry+1),*(uint32 *)arl_entry);
    
    /* Get MAC field */
    DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC,
            (uint32 *)arl_entry, (uint32 *)&mac_field);

    SAL_MAC_ADDR_FROM_UINT64(l2addr->mac, mac_field);

    soc_cm_debug(DK_ARL|DK_TESTS, "in _bcm_robo_l2_from_arl ,\
        mac  is %02x-%02x-%02x-%02x-%02x-%02x \n",
        l2addr->mac[0], l2addr->mac[1],l2addr->mac[2], l2addr->mac[3],
        l2addr->mac[4], l2addr->mac[5]);

    /* Get VID field */
    DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL,
            DRV_MEM_FIELD_VLANID, (uint32 *)arl_entry, &field_value);
    l2addr->vid = field_value;
    soc_cm_debug(DK_ARL|DK_TESTS, 
            "in _bcm_robo_l2_from_arl, l2addr->vid is %d\n",
            l2addr->vid);

    /* Get entry status field */
    DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL,
            DRV_MEM_FIELD_VALID, (uint32 *)arl_entry, &field_value);
    if (soc_feature(unit, soc_feature_l2_pending)) {
        if (SOC_IS_TBX(unit)){
#ifdef  BCM_TB_SUPPORT
            DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL,
                    DRV_MEM_FIELD_VALID, (uint32 *)arl_entry, &field_value);
            if (field_value == _TB_ARL_STATUS_PENDING) {
                l2addr->flags |= BCM_L2_PENDING;
            }
#endif /* BCM_TB_SUPPORT */
        } else {
            soc_cm_debug(DK_WARN, "\t No Pending learning feature!!\n");
            return;
        }
    }

    /* Get static field */
    DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL,
            DRV_MEM_FIELD_STATIC, (uint32 *)arl_entry, &field_value);
    if (field_value) {
        l2addr->flags |= BCM_L2_STATIC;
    }

    if (l2addr->mac[0] & 0x01) { /* mcast address */
        l2addr->flags |= BCM_L2_MCAST;
        DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_DEST_BITMAP, 
            (uint32 *)arl_entry, &field_value);
        if (field_value) {
            l2addr->l2mc_index = (int)field_value;               
        }
        soc_cm_debug(DK_ARL|DK_TESTS, 
                "in _bcm_robo_l2_from_arl, l2mc_index is %d\n",
                l2addr->l2mc_index);

    } else {
        rv = bcm_robo_switch_control_get(unit, 
                    bcmSwitchUseGport, &useGport);

        /* set Port */
        DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, 
                (uint32 *)arl_entry, &field_value);
        src_port = field_value;

        if (useGport){
            /* if user request any GPORT type to report(in l2addr.port) 
             *      - GPORT_MODPORT : supported on all ROBO chips.
             *      - GPORT_SUBPOPRT : supported on TB only.
             *      - other GPORT : not support.
             * else (no GPORT requested in l2addr.port)
             *      - report GPORT_MODPORT type due to useGport in SDK.
             */
             
            _bcm_robo_gport_dest_t_init(&dest);
            /* assigning the default GPORT type to report l2 entry */
            dest.port = src_port;
            dest.modid = 0;
            dest.gport_type = BCM_GPORT_TYPE_MODPORT;
            
             if (BCM_GPORT_IS_SET(l2addr->port)){
                if (BCM_GPORT_IS_SUBPORT_PORT(l2addr->port)){
                    if (soc_feature(unit, soc_feature_subport)){
#ifdef  BCM_TB_SUPPORT
                        uint32  vp_id;
                        
                        soc_cm_debug(DK_ARL | DK_TESTS, 
                                "%s,src_port=%d,l2_gport=0x%08x\n", 
                                FUNCTION_NAME(), src_port, l2_gport);
                        /* vp_id is a new item in bcm_l2_addr_t to support 
                         *   TB's new feature in ARL entry of vport_id field.
                         */
                         
                        /* get VPort fields */
                        DRV_MEM_FIELD_GET(unit, 
                                DRV_MEM_ARL, DRV_MEM_FIELD_VPORT, 
                                (uint32 *)arl_entry, &field_value);
                        vp_id = field_value;
                        soc_cm_debug(DK_ARL|DK_TESTS, 
                                "in _bcm_robo_l2_from_arl, vport is %d\n",
                                vp_id);
                        
                        dest.subport_id= 
                                _TB_SUBPORT_SYSTEM_ID_SET(src_port, 0, vp_id);
                        dest.gport_type = BCM_GPORT_SUBPORT_PORT;
#else   /* BCM_TB_SUPPORT */
                        soc_cm_debug(DK_WARN, 
                                "%s, GPORT_SUBPORT type is unavailable!\n", 
                                FUNCTION_NAME());
#endif  /* BCM_TB_SUPPORT */
                    } else {
                        soc_cm_debug(DK_WARN, 
                                "%s, GPORT_SUBPORT type is unavailable!\n", 
                                FUNCTION_NAME());
                    }
                } else {
                    /* other GPORT type */
                    soc_cm_debug(DK_WARN, 
                            "%s,requesting a unavailable GPORT type!\n", 
                            FUNCTION_NAME());
                }
             }
             
             rv = _bcm_robo_gport_construct(unit, 
                     &dest, &l2_gport);
             l2addr->port = l2_gport;
            
        } else {
            /* user didn't force to use GPORT in SDK */

            /* TB here will be reported as normal port format(no vport) */
            l2addr->port = src_port;     /* no GPORT type */
        }

        if (!SOC_IS_TBX(unit)){
            DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_PRIORITY, 
                (uint32 *)arl_entry, &field_value);
            l2addr->cos_dst = field_value;
        }
  

        DRV_DEV_PROP_GET(unit, DRV_DEV_PROP_AGE_HIT_VALUE, &age_hit_val);

        DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_AGE, 
            (uint32 *)arl_entry, &field_value);
        if (field_value == age_hit_val) {

            /* ROBO' hit operation(source hit) can be worked on non-static 
             *  L2 entry only.
             */
            l2addr->flags |= BCM_L2_HIT | BCM_L2_SRC_HIT;
        }
        if (soc_feature(unit, soc_feature_arl_mode_control)) {
            /* Get arl control field */
            DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_ARL_CONTROL,
                    (uint32 *)arl_entry, &field_value);

            if (field_value == 0x1) {
                l2addr->flags |= BCM_L2_DISCARD_DST;
            }else if(field_value == 0x2){
                l2addr->flags |= BCM_L2_DISCARD_SRC;        
            }else if(field_value == 0x3){
                l2addr->flags |= BCM_L2_COPY_TO_CPU;
            }
        }

        soc_cm_debug(DK_ARL|DK_TESTS, "%s, port is 0x%x cos_dst 0x%x\n", 
                 FUNCTION_NAME(), l2addr->port, l2addr->cos_dst);

    }    
    /* Valid bit is ignored here; entry is assumed valid */
    
}


/*
 * Function:
 *      bcm_l2_port_native
 * Purpose:
 *      Determine if the given port is "native" from the point
 *      of view of L2.
 * Parameters:
 *      unit       - The unit
 *      modid      - Module ID of device
 *      port       - Physical port on the unit
 * Returns:
 *      TRUE (> 0) if (modid, port) is front panel/CPU port for unit.
 *      FALSE (0) otherwise.
 *      < 0 on error.
 *
 *      Native means front panel, but also includes the CPU port.
 *      HG ports are always excluded as are ports marked as internal or
 *      external stacking
 */
int
bcm_robo_l2_port_native(int	unit, int modid, int port)
{
    bcm_trunk_t     tgid;
    int             id, isLocal = 0;

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(_bcm_robo_gport_resolve(unit, port, &modid,
            &port, &tgid, &id));

        if (-1 != id || BCM_TRUNK_INVALID != tgid) {
            return FALSE;
        }
    }

    BCM_IF_ERROR_RETURN(_bcm_robo_modid_is_local(unit, modid, &isLocal));
    if (isLocal != TRUE) {
        return FALSE;
    } else {
        return TRUE;
    }

}

/*
 * Function:
 *	_bcm_robo_l2_term
 * Description:
 *	Finalize chip-dependent parts of L2 module
 * Parameters:
 *	unit - RoboSwitch unit number.
 */

int
_bcm_robo_l2_term(int unit)
{
    COMPILER_REFERENCE(unit);
    
    return SOC_E_NONE;
}


/****************************************************************************
 *
 * L2 Message Registration
 *
 ****************************************************************************/

static bcm_l2_addr_callback_t _bcm_l2_cbs[SOC_MAX_NUM_SWITCH_DEVICES];
static void *_bcm_l2_cb_data[SOC_MAX_NUM_SWITCH_DEVICES];

/*
 * Function:
 *	_bcm_robo_l2_register_callback
 * Description:
 *	Call back to handle bcm_l2_addr_register clients. 
 * Parameters:
 *	unit - RoboSwitch unit number.
 *	entry_del - Entry to be deleted or updated, NULL if none.
 *	entry_add - Entry to be inserted or updated, NULL if none.
 *	fn_data - unused
 * Notes:
 *	Only one callback per unit to the bcm layer is supported here. 
 *	Multiple bcm client callbacks per unit are supported in the bcm layer. 
 */

static void
_bcm_robo_l2_register_callback(int unit,
              l2_arl_sw_entry_t *entry_del,
              l2_arl_sw_entry_t *entry_add,
              void *fn_data)
{    
    if (_bcm_l2_cbs[unit] != NULL) {
        bcm_l2_addr_t l2addr_del, l2addr_add;
        uint32 flags = 0; /* Common flags: Move, From/to native */

        /* l2addr->port can carry GPORT type to indicate the desired formate.
         *
         *  Note : if the GPORT is forcing to use in SDK (i.e. to assert 
         *      bcmSwitchUseGport), the reported l2addr.port will still be 
         *      no GPORT format.
         */
        if (SOC_IS_TBX(unit)){
            /* Use SUBPORT GPORT type instead of MODPORT GPORT type for 
             * requesting the most detail l2 information(include vport).
             */
            BCM_GPORT_SUBPORT_PORT_SET(l2addr_del.port, 0);
            BCM_GPORT_SUBPORT_PORT_SET(l2addr_add.port, 0);
        } else {
            BCM_GPORT_MODPORT_SET(l2addr_del.port, 0, 0);
            BCM_GPORT_MODPORT_SET(l2addr_add.port, 0, 0);
        }
        
        
        /* First, set up the entries:  decode HW entries and set flags */
        if (entry_del != NULL) {
            _bcm_robo_l2_from_arl(unit, &l2addr_del, entry_del);
        }
        if (entry_add != NULL) {
            _bcm_robo_l2_from_arl(unit, &l2addr_add, entry_add);
        }
        
        if ((entry_del != NULL) && (entry_add != NULL)) { /* It's a move */
            flags |= BCM_L2_MOVE;
            if (SOC_USE_GPORT(unit)) {
                if (l2addr_del.port != l2addr_add.port) {
                    flags |= BCM_L2_MOVE_PORT;
                }
            } else {
                if (l2addr_del.modid != l2addr_add.modid ||
                    l2addr_del.port != l2addr_add.port) {
                    flags |= BCM_L2_MOVE_PORT;
                }
            }
            if (bcm_robo_l2_port_native(unit, l2addr_del.modid,
                                   l2addr_del.port) > 0) {
                flags |= BCM_L2_FROM_NATIVE;
                l2addr_del.flags |= BCM_L2_NATIVE;
            }
            if (bcm_robo_l2_port_native(unit, l2addr_add.modid,
                                   l2addr_add.port) > 0) {
                flags |= BCM_L2_TO_NATIVE;
                l2addr_add.flags |= BCM_L2_NATIVE;
            }
            l2addr_del.flags |= flags;
            l2addr_add.flags |= flags;
        } else if (entry_del != NULL) { /* Age out or simple delete */
            if (bcm_robo_l2_port_native(unit, l2addr_del.modid,
                                   l2addr_del.port) > 0) {
                l2addr_del.flags |= BCM_L2_NATIVE;
            }
        } else if (entry_add != NULL) { /* Insert or learn */
            if (bcm_robo_l2_port_native(unit, l2addr_add.modid,
                                   l2addr_add.port) > 0) {
                l2addr_add.flags |= BCM_L2_NATIVE;
            }
        }
        
        /* The entries are now set up.  Make the callbacks */
        if (entry_del != NULL) {
            _bcm_l2_cbs[unit](unit, &l2addr_del, 0, _bcm_l2_cb_data[unit]);
        }
        if (entry_add != NULL) {
            _bcm_l2_cbs[unit](unit, &l2addr_add, 1, _bcm_l2_cb_data[unit]);
        }
    }

}

/*
 * Function:
 *	_bcm_robo_l2_bpdu_init
 * Description:
 *	Initialize all BPDU addresses to recognize the 802.1D
 *      Spanning Tree address on all chips.
 * Parameters:
 *	unit - unit number (driver internal).
 * Returns:
 *	BCM_E_XXX
 * Notes:
 */

static int
_bcm_robo_l2_bpdu_init(int unit)
{
    int	i, nbpdu;
    sal_mac_addr_t mac;
    pbmp_t null_pbmp;

    nbpdu = bcm_l2_bpdu_count(unit);

    /* Spanning Tree addr (01:80:c2:00:00:00) used as default entries */
    mac[0] = 0x01;
    mac[1] = 0x80;
    mac[2] = 0xc2;
    mac[3] = mac[4] = mac[5] = 0x00;

    for (i = 0; i < nbpdu; i++) {
        BCM_PBMP_CLEAR(null_pbmp);
        BCM_IF_ERROR_RETURN(DRV_MAC_SET(unit, null_pbmp, 
                DRV_MAC_CUSTOM_BPDU, mac, i));
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_53xx_l2_addr_register
 * Description:
 *	Register a callback routine that will be called whenever
 *	an entry is inserted into or deleted from the L2 address table.
 * Parameters:
 *	unit - RoboSwitch PCI device unit number (driver internal).
 *	callback - Callback function of type bcm_l2_addr_callback_t.
 *	userdata - Arbitrary value passed to callback along with messages
 * Returns:
 *	BCM_E_NONE		Success, handle valid
 *	BCM_E_MEMORY		Out of memory
 *	BCM_E_INTERNAL		Chip access failure
 */

int
bcm_53xx_l2_addr_register(int unit,
               bcm_l2_addr_callback_t callback,
               void *userdata)
{
    /* this should be managed properly from above */

    if (_bcm_l2_cbs[unit] != NULL || _bcm_l2_cbs[unit] != NULL){
        soc_cm_debug(DK_ARL, 
                "%s, Reasigning the callback service routine!\n",
                FUNCTION_NAME());
    }
    
    _bcm_l2_cbs[unit] = callback;
    _bcm_l2_cb_data[unit] = userdata;
    
    /* return SOC_E_NONE; kram 2004-01-06*/
    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_53xx_l2_addr_unregister
 * Description:
 *	Unregister a previously registered callback routine.
 * Parameters:
 *	unit - RoboSwitch PCI device unit number (driver internal).
 *	callback - Same callback function used to register callback
 *	userdata - Same arbitrary value used to register callback
 * Returns:
 *	BCM_E_NONE		Success, handle valid
 *	BCM_E_MEMORY		Out of memory
 *	BCM_E_INTERNAL		Chip access failure
 * Notes:
 *	Both callback and userdata must match from original register call.
 */

int
bcm_53xx_l2_addr_unregister(int unit,
                 bcm_l2_addr_callback_t callback,
                 void *userdata)
{
    /* this should be managed properly from above */
    _bcm_l2_cbs[unit] = NULL;
    _bcm_l2_cb_data[unit] = NULL;
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_robo_l2_detach
 * Purpose:
 *      Clean up l2 bcm layer when unit is detached
 * Parameters:
 *      unit - unit being detached
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_robo_l2_detach(int unit)
{
    int     frozen = 0;
    
    /* prevent L2 init process when ARL table still frozen, 
     *  - L2_ARLm is LOCK in frozen state
     *  - frozen counter(f->frozen) not clear in frozen state
     */
    BCM_IF_ERROR_RETURN(soc_robo_arl_is_frozen(unit, &frozen));
    if (TRUE == frozen) {
        return BCM_E_BUSY;
    }

    BCM_IF_ERROR_RETURN(_bcm_robo_l2_term(unit));
    
    _l2_init[unit] = 0;
    
    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_robo_l2_init
 * Description:
 *	Perform required initializations to L2 table.
 * Parameters:
 *	unit - RoboSwitch PCI device unit number (driver internal).
 * Returns:
 *	BCM_E_XXX
 */
int
bcm_robo_l2_init(int unit)
{
    bcm_port_t  port = 0;
    int     rv = BCM_E_NONE;
    int     frozen;
    uint32  arl_mod = 0;

    /* prevent L2 init process when ARL table still frozen, 
     *  - L2_ARLm is LOCK in frozen state
     *  - frozen counter(f->frozen) not clear in frozen state
     */
    BCM_IF_ERROR_RETURN(soc_robo_arl_is_frozen(unit, &frozen));
    if (TRUE == frozen) {
        return BCM_E_BUSY;
    }
    
    _l2_init[unit] = 0;
    
    /* Turn off arl aging */
    BCM_IF_ERROR_RETURN(bcm_l2_age_timer_set(unit, 0));
    
    bcm_robo_l2_detach(unit);

    /*
     * Init BPDU station address registers.
     */
    rv = _bcm_robo_l2_bpdu_init(unit);
    if (rv < 0 && rv != BCM_E_UNAVAIL){
        return rv;
    }
    
    /*
     * Init L2 cache
     */
    rv = bcm_l2_cache_init(unit);
    if (rv < 0 && rv != BCM_E_UNAVAIL) {
        return rv;
    }

    soc_robo_arl_unregister(unit, _bcm_robo_l2_register_callback, NULL);
    BCM_IF_ERROR_RETURN(DRV_MEM_CLEAR(unit, DRV_MEM_ARL));
    
    /* ROBO's SW ARL reset due to HW L2 table cleared */
    /* ARL thread init process */
    arl_mod = soc_property_get(unit, spn_L2XMSG_MODE, 1);
    if(arl_mod) {
        BCM_IF_ERROR_RETURN(
                soc_robo_arl_mode_set(unit, ARL_MODE_ROBO_POLL));
    } else {
        BCM_IF_ERROR_RETURN(
                soc_robo_arl_mode_set(unit, ARL_MODE_NONE));
    }

    /* Reset per-port SA learning count */
    if (soc_feature(unit, soc_feature_mac_learn_limit)) {
        PBMP_E_ITER(unit, port) {
            rv = DRV_ARL_LEARN_COUNT_SET
                (unit, port, DRV_PORT_SA_LRN_CNT_RESET, 0);
            if (rv < 0 && rv != BCM_E_UNAVAIL) {
                return rv;
            }
        }
    }

    /* enable default ROAMING option for proper HW behavior on SA 
     *  learn count maintenance when port moving occurred. 
     */
    if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit) || 
            SOC_IS_TBX(unit) ){
        pbmp_t  pbmp;
        uint32  temp = BCM_PORT_LEARN_ARL;

        if (SOC_IS_TBX(unit)){
            temp |= BCM_PORT_LEARN_FWD;
        }

        BCM_PBMP_ASSIGN(pbmp, PBMP_ALL(unit));
        BCM_PBMP_ITER(pbmp, port){
            BCM_IF_ERROR_RETURN(bcm_robo_port_control_set(unit, port, 
                    bcmPortControlL2Move, temp));
        }
    }

    /* bcm_l2_register clients */
    soc_robo_arl_register(unit, _bcm_robo_l2_register_callback, NULL);

    _l2_init[unit] = 1;
    return (BCM_E_NONE);
}

/*
 * Temporarily stop L2 table from changing (learning, aging, CPU, etc)
 */

/*
 * Function:
 *	bcm_robo_l2_addr_freeze
 * Description:
 *	Temporarily quiesce ARL from all activity (learning, aging)
 * Parameters:
 *	unit - RoboSwitch PCI device unit number (driver internal).
 * Returns:
 *	BCM_E_NONE		Success
 *	BCM_E_INTERNAL		Chip access failure
 */
int
bcm_robo_l2_addr_freeze(int unit)
{
    L2_INIT(unit);
    
    soc_cm_debug(DK_ARL, 
            "BCM API : bcm_robo_l2_addr_freeze().\n");
    BCM_IF_ERROR_RETURN(soc_robo_arl_freeze(unit));
    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_robo_l2_addr_thaw
 * Description:
 *	Restore normal ARL activity.
 * Parameters:
 *	unit - RoboSwitch PCI device unit number (driver internal).
 * Returns:
 *	BCM_E_NONE		Success
 *	BCM_E_INTERNAL		Chip access failure
 */
int
bcm_robo_l2_addr_thaw(int unit)
{
    L2_INIT(unit);
    
    soc_cm_debug(DK_ARL, 
            "BCM API : bcm_robo_l2_addr_thaw().\n");
    BCM_IF_ERROR_RETURN( soc_robo_arl_thaw(unit));
    return BCM_E_NONE;
}

/*
 * Initialize a bcm_l2_addr_t to a specified MAC address and VLAN,
 * zeroing all other fields.
 */

/*
 * Function:
 *	bcm_robo_l2_addr_add
 * Description:
 *	Add a MAC address to the Switch Address Resolution Logic (ARL)
 *	port with the given VLAN ID and parameters.
 * Parameters:
 *	unit - RoboSwitch PCI device unit number (driver internal).
 *	l2addr - Pointer to bcm_l2_addr_t containing all valid fields
 * Returns:
 *	BCM_E_NONE		Success
 *	BCM_E_INTERNAL		Chip access failure
 * Notes:
 *	Use CMIC_PORT(unit) to associate the entry with the CPU.
 *	Use flag of BCM_L2_COPY_TO_CPU to send a copy to the CPU.
 *      Use flag of BCM_L2_TRUNK_MEMBER to set trunking (TGID must be
 *      passed as well with non-zero trunk group ID)
 */

int 
bcm_robo_l2_addr_add(int unit, bcm_l2_addr_t *l2addr)
{
    l2_arl_sw_entry_t   arl_entry;
    uint32              flags = 0;
    int                 rv = BCM_E_NONE, freezing = BCM_E_NONE;

    L2_INIT(unit);
    soc_cm_debug(DK_ARL, 
            "BCM API : bcm_robo_l2_addr_add()..\n");

    BCM_IF_ERROR_RETURN(_bcm_robo_l2_to_arl(unit, &arl_entry, l2addr));
    soc_cm_debug(DK_ARL, 
                "in bcm_robo_l2_addr_add , arl_entry is 0x%x,0x%x,0x%x\n",
                arl_entry.entry_data[2], 
                arl_entry.entry_data[1], 
                arl_entry.entry_data[0]);

    if (l2addr->flags & BCM_L2_REPLACE_DYNAMIC) {
        soc_cm_debug(DK_ARL, 
                "BCM API : bcm_robo_l2_addr_add()..BCM_L2_REPLACE_DYNAMIC\n");
        
        flags = DRV_MEM_OP_BY_HASH_BY_MAC | DRV_MEM_OP_BY_HASH_BY_VLANID | 
                DRV_MEM_OP_REPLACE;
    } else {
        flags = DRV_MEM_OP_BY_HASH_BY_MAC | DRV_MEM_OP_BY_HASH_BY_VLANID;    
    }
    
    if (l2addr->flags & BCM_L2_PENDING){
        if (!soc_feature(unit, soc_feature_l2_pending)) {
            return BCM_E_UNAVAIL;
        }
        flags |=  DRV_MEM_OP_PENDING;
    }
    
    if (l2addr->flags & BCM_L2_HIT){
        flags |=  DRV_MEM_OP_HIT;
    }
    
    /* freeze/thaw the HW ARL to prevent the confilict action on SA_LRN_CNT 
     * handling process between SW and HW.
     */
    freezing = bcm_robo_l2_addr_freeze(unit);
    if (freezing == BCM_E_UNAVAIL || freezing == BCM_E_NONE){
        rv = DRV_MEM_INSERT(unit, DRV_MEM_ARL, (uint32 *)&arl_entry, flags);
        if (freezing == BCM_E_NONE){
            BCM_IF_ERROR_RETURN(bcm_robo_l2_addr_thaw(unit));
        }
    } else {
        rv = freezing;
    }
    
    return rv;

}

/*
 * Function:
 *	bcm_robo_l2_addr_delete
 * Description:
 *	Remove an L2 address from the device's ARL
 * Parameters:
 *	unit - RoboSwitch PCI device unit number (driver internal).
 *	mac - MAC address to remove
 *	vid - associated VLAN ID
 * Returns:
 *	BCM_E_NONE		Success
 *	BCM_E_INTERNAL		Chip access failure
 */
int 
bcm_robo_l2_addr_delete(int unit, bcm_mac_t mac, bcm_vlan_t vid)
{
    bcm_l2_addr_t       l2addr;
    l2_arl_sw_entry_t   arl_delete;
    int                 rv = BCM_E_NONE, freezing = BCM_E_NONE;
    uint32              flags = 0;

    L2_INIT(unit);
    soc_cm_debug(DK_ARL, 
            "BCM API : bcm_robo_l2_addr_delete()..\n");
            
    bcm_l2_addr_init(&l2addr, mac, vid);

    BCM_IF_ERROR_RETURN(_bcm_robo_l2_to_arl(unit, &arl_delete, &l2addr));

    /* freeze/thaw the HW ARL to prevent the confilict action on SA_LRN_CNT 
     * handling process between SW and HW.
     */
    freezing = bcm_robo_l2_addr_freeze(unit);
    if (freezing == BCM_E_UNAVAIL || freezing == BCM_E_NONE){
        /* SDK-34306 : Can't remove Mcast entry.
         *  
         *  Fixing Note : 
         *      1. Root cause is l2 delete with pending flag is now redesigned 
         *          per SDK-33848 to remove (pending entry only) instead of 
         *          (pending || non-pending entry)
         *      2. The fixing process for such designing change on SDK-33848 
         *          in this API will be :
         *          a. normal remove without pending flag.
         *          b. if return BCM_E_NOT_FOUND, use pending flag to remove 
         *              again.
         */
        flags = DRV_MEM_OP_BY_HASH_BY_MAC | DRV_MEM_OP_BY_HASH_BY_VLANID |
                    DRV_MEM_OP_DELETE_BY_STATIC;
        rv = DRV_MEM_DELETE(unit, DRV_MEM_ARL, (uint32 *)&arl_delete, flags);
        if (rv == BCM_E_NOT_FOUND) {
            if (soc_feature(unit, soc_feature_l2_pending)) {
                flags |= DRV_MEM_OP_PENDING;
                rv = DRV_MEM_DELETE(unit, DRV_MEM_ARL, (uint32 *)&arl_delete, 
                            flags);
            } 
        }

        if (freezing == BCM_E_NONE){
            BCM_IF_ERROR_RETURN(bcm_robo_l2_addr_thaw(unit));
        }
    } else {
        rv = freezing;
    }

    return rv;
}


/*
 * Function:
 *	bcm_robo_l2_addr_delete_by_port
 * Description:
 *	Remove all L2 (MAC) addresses associated with the port.
 * Parameters:
 *	unit  - RoboSwitch PCI device unit number (driver internal).
 *	mod   - module id (or -1 for local unit)
 *	pbmp  - bitmap of ports to affect
 *	flags - BCM_L2_REMOVE_XXX
 * Returns:
 *	BCM_E_NONE		Success.
 *	BCM_E_INTERNAL		Chip access failure.
 * Notes:
 *	Static entries are removed only if BCM_L2_DELETE_STATIC flag is used.
 *
 *	ARL aging and learning on all ports is disabled during this
 *	operation.   If these weren't disabled, the hardware could
 *	shift around the contents of the ARL table during the remove
 *	operation, causing some entries that should be removed to remain
 *	in the table.
 */
int
bcm_robo_l2_addr_delete_by_port(int unit, bcm_module_t mod, bcm_port_t port,
            	uint32 flags)
{
    int                 rv = BCM_E_NONE, freezing = BCM_E_NONE;
    l2_arl_sw_entry_t   arl_entry;
    uint32              fld_value = 0;
    uint32              action_flag = 0;

    L2_INIT(unit);
    sal_memset(&arl_entry, 0, sizeof (arl_entry));

    /* valid module/port check */
    if (BCM_GPORT_IS_SET(port)) {
        bcm_port_t  tmp_port;
        bcm_trunk_t tmp_trunk;
        bcm_module_t tmp_mod;
        int         tmp_id;
    
        BCM_IF_ERROR_RETURN(
            _bcm_robo_gport_resolve(unit, port, &tmp_mod, &tmp_port,
                                   &tmp_trunk, &tmp_id));
        port = tmp_port;
    }
    
    /* module == -1 is used for indicating to local modid */
    if (mod < 0) {
        if (!SOC_PORT_VALID(unit, port)) {
            return BCM_E_PORT;
        }
    } else { /* map module/port */
        if (!SOC_MODID_ADDRESSABLE(unit, mod)) {
            return BCM_E_BADID;
        }
    }
    
    
    fld_value = port;    
    soc_cm_debug(DK_ARL, 
            "BCM API : bcm_robo_l2_addr_delete_by_port()..\n");
    /* set port field */
    BCM_IF_ERROR_RETURN(
            DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_SRC_PORT, 
                (uint32 *)&arl_entry, &fld_value));
    action_flag = DRV_MEM_OP_DELETE_BY_PORT ;
#ifdef  BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)){
        action_flag |= (flags & _BCM_TB_L2_REPLACE_MATCH_STATIC_ONLY) ? 
                    DRV_MEM_OP_DELETE_BY_STATIC_ONLY : 0;
        action_flag |= ((flags & BCM_L2_DELETE_STATIC) || 
                        (flags & _BCM_TB_L2_REPLACE_MATCH_STATIC)) ? 
                    DRV_MEM_OP_DELETE_BY_STATIC : 0;
        action_flag |= (flags & _BCM_TB_L2_REPLACE_MATCH_MCAST_ONLY) ? 
                    DRV_MEM_OP_DELETE_BY_MCAST_ONLY : 0;
        action_flag |= (flags & _BCM_TB_L2_REPLACE_MATCH_MCAST) ? 
                    DRV_MEM_OP_DELETE_BY_MCAST : 0;
    } else {
        action_flag |= (flags & BCM_L2_DELETE_STATIC) ? 
                        DRV_MEM_OP_DELETE_BY_STATIC : 0;
    }
#else   /* BCM_TB_SUPPORT */
    action_flag |= (flags & BCM_L2_DELETE_STATIC) ? 
                    DRV_MEM_OP_DELETE_BY_STATIC : 0;
#endif  /* BCM_TB_SUPPORT */
    action_flag |= (flags & BCM_L2_DELETE_PENDING) ? 
                    DRV_MEM_OP_PENDING : 0;

    /* freeze/thaw the HW ARL to prevent the confilict action on SA_LRN_CNT 
     * handling process between SW and HW.
     */
    freezing = bcm_robo_l2_addr_freeze(unit);
    if (freezing == BCM_E_UNAVAIL || freezing == BCM_E_NONE){
        rv = DRV_MEM_DELETE(
                unit, DRV_MEM_ARL,(uint32 *)&arl_entry, action_flag);
        if (freezing == BCM_E_NONE){
            BCM_IF_ERROR_RETURN(bcm_robo_l2_addr_thaw(unit));
        }
    } else {
        rv = freezing;
    }
    return rv;
}


/*
 * Function:
 *	bcm_l2_addr_delete_by_mac
 * Description:
 *	Delete L2 entries associated with a MAC address.
 * Parameters:
 *	unit  - device unit
 *	mac   - MAC address
 *	flags - BCM_L2_DELETE_XXX
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	Static entries are removed only if BCM_L2_DELETE_STATIC flag is used.
 *	L2 aging and learning are disabled during this operation.
 */
int
bcm_robo_l2_addr_delete_by_mac(
	int	unit,
	bcm_mac_t	mac,
	uint32	flags)
{
    bcm_l2_addr_t   l2addr;      
    l2_arl_sw_entry_t		arl_search;     
    bcm_vlan_t      t_vid = VLAN_ID_INVALID;
    int             rv = BCM_E_NONE, freezing = BCM_E_NONE;
    int         	i;
    uint32          action_flag = 0, vlan_count = 0;        

    L2_INIT(unit);
    
    rv = DRV_DEV_PROP_GET(unit,DRV_DEV_PROP_VLAN_ENTRY_NUM, &vlan_count);

    soc_cm_debug(DK_ARL, 
            "BCM API : bcm_robo_l2_addr_delete_by_mac()..\n vlan_count=%d\n",
            vlan_count);
            
    /* freeze/thaw the HW ARL to prevent the confilict action on  
     * SA_LRN_CNT handling process between SW and HW.
     */
    freezing = bcm_robo_l2_addr_freeze(unit);
    if (freezing == BCM_E_UNAVAIL || freezing == BCM_E_NONE){
        for (i = 1; i < vlan_count; i++) {
            t_vid = i;
    
            soc_cm_debug(DK_ARL, 
                    "ARL_DEL: MAC=%02x:%02x:%02x:%02x:%02x:%02x, VID=0x%x\n",
                    mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
                    t_vid);
            /* prepare the l2 entry with mac+vid */   
            bcm_l2_addr_init(&l2addr, mac, t_vid);
            l2addr.flags |= (flags == BCM_L2_DELETE_STATIC) ? 
                            BCM_L2_STATIC : 0;
            BCM_IF_ERROR_RETURN(_bcm_robo_l2_to_arl(
                    unit, &arl_search, &l2addr));
        
            /* delete this l2 entry*/
            action_flag = DRV_MEM_OP_BY_HASH_BY_MAC | 
                        DRV_MEM_OP_BY_HASH_BY_VLANID;
            action_flag |= (flags & BCM_L2_DELETE_STATIC) ? 
                            DRV_MEM_OP_DELETE_BY_STATIC : 0;
            action_flag |= (flags & BCM_L2_DELETE_PENDING) ? 
                            DRV_MEM_OP_PENDING : 0;
                            
            rv = DRV_MEM_DELETE(unit, DRV_MEM_ARL, (uint32 *)&arl_search, 
                    action_flag);
            soc_cm_debug(DK_ARL, 
                    "bcm_robo_l2_addr_delete_by_mac(): mem_delete()=%d\n",rv);
    
            if (rv == BCM_E_NOT_FOUND){
                continue;
            } else if(rv != 0){
                soc_cm_debug(DK_WARN, 
                        "Warnning: bcm_l2_addr_delete_by_mac(),\n\t >>%s\n", 
                        bcm_errmsg(rv));
            }
        }
        rv = BCM_E_NONE;
        if (freezing == BCM_E_NONE){
            BCM_IF_ERROR_RETURN(bcm_robo_l2_addr_thaw(unit));
        }
    } else {
        rv = freezing;
    }

    return rv;
}


/*
 * Function:
 *	bcm_robo_l2_addr_delete_by_vlan
 * Description:
 *	Remove all L2 (MAC) addresses associated with vid.
 * Parameters:
 *	unit - RoboSwitch PCI device unit number (driver internal).
 *	vlan - vid to check
 *	flags - BCM_L2_REMOVE_XXX
 * Returns:
 *	BCM_E_NONE		Success.
 *	BCM_E_INTERNAL		Chip access failure.
 * Notes:
 *	Static entries are removed only if BCM_L2_DELETE_STATIC flag is used.
 *
 *	ARL aging and learning on all ports is disabled during this
 *	operation.   If these weren't disabled, the hardware could
 *	shift around the contents of the ARL table during the remove
 *	operation, causing some entries that should be removed to remain
 *	in the table.
 */
int
bcm_robo_l2_addr_delete_by_vlan(int unit, bcm_vlan_t vid, uint32 flags)
{
    int                 rv = BCM_E_NONE, freezing = BCM_E_NONE;
    uint32              fld_value = 0;
    uint32              action_flag = 0;
    l2_arl_sw_entry_t   arl_entry;
    
    L2_INIT(unit);

    /* valid check */
    if(vid > BCM_VLAN_MAX){
        return BCM_E_PARAM;
    }

    sal_memset(&arl_entry, 0, sizeof (arl_entry));
    fld_value = vid; /* vid value */
    
    soc_cm_debug(DK_ARL, 
            "BCM API : bcm_robo_l2_addr_delete_by_vlan()..\n");
    /* set VID field */
    BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
            DRV_MEM_FIELD_VLANID, (uint32 *)&arl_entry, &fld_value));
    action_flag = DRV_MEM_OP_DELETE_BY_VLANID ;
#ifdef  BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)){
        action_flag |= (flags & _BCM_TB_L2_REPLACE_MATCH_STATIC_ONLY) ? 
                    DRV_MEM_OP_DELETE_BY_STATIC_ONLY : 0;
        action_flag |= ((flags & BCM_L2_DELETE_STATIC) || 
                        (flags & _BCM_TB_L2_REPLACE_MATCH_STATIC)) ? 
                    DRV_MEM_OP_DELETE_BY_STATIC : 0;
        action_flag |= (flags & _BCM_TB_L2_REPLACE_MATCH_MCAST_ONLY) ? 
                    DRV_MEM_OP_DELETE_BY_MCAST_ONLY : 0;
        action_flag |= (flags & _BCM_TB_L2_REPLACE_MATCH_MCAST) ? 
                    DRV_MEM_OP_DELETE_BY_MCAST : 0;
    } else {
        action_flag |= (flags & BCM_L2_DELETE_STATIC) ? 
                        DRV_MEM_OP_DELETE_BY_STATIC : 0;
    }
#else   /* BCM_TB_SUPPORT */
    action_flag |= (flags & BCM_L2_DELETE_STATIC) ? 
                    DRV_MEM_OP_DELETE_BY_STATIC : 0;
#endif  /* BCM_TB_SUPPORT */
    action_flag |= (flags & BCM_L2_DELETE_PENDING) ? 
                    DRV_MEM_OP_PENDING : 0;
    /* freeze/thaw the HW ARL to prevent the confilict action on  
     * SA_LRN_CNT handling process between SW and HW.
     */
    freezing = bcm_robo_l2_addr_freeze(unit);
    if (freezing == BCM_E_UNAVAIL || freezing == BCM_E_NONE){
        rv = DRV_MEM_DELETE(unit, DRV_MEM_ARL, (uint32 *)&arl_entry, 
                action_flag);
        if (freezing == BCM_E_NONE){
            BCM_IF_ERROR_RETURN(bcm_robo_l2_addr_thaw(unit));
        }
    } else {
        rv = freezing;
    }
        
    return rv;
}


/*
 * Function:
 *	bcm_robo_l2_addr_delete_by_trunk
 * Description:
 *	Remove all L2 (MAC) addresses associated with tid.
 * Parameters:
 *	unit - RoboSwitch PCI device unit number (driver internal).
 *	tid - tid to check
 *	flags - BCM_L2_REMOVE_XXX
 * Returns:
 *	BCM_E_NONE		Success.
 *	BCM_E_INTERNAL		Chip access failure.
 * Notes:
 *	Static entries are removed only if BCM_L2_DELETE_STATIC flag is used.
 *
 *	ARL aging and learning on all ports is disabled during this
 *	operation.   If these weren't disabled, the hardware could
 *	shift around the contents of the ARL table during the remove
 *	operation, causing some entries that should be removed to remain
 *	in the table.
 *  (P.S. TB device start to provide the fast aging feature on trunk.)
 */
int
bcm_robo_l2_addr_delete_by_trunk(int unit, bcm_trunk_t tid, uint32 flags)
{
    bcm_trunk_add_info_t t_add_info;
    int         i, rv = BCM_E_NONE, freezing = BCM_E_NONE;
    uint32      key_value = 0;
    uint32      action_flag = 0;
    
    L2_INIT(unit);
    
    soc_cm_debug(DK_ARL, 
            "BCM API : bcm_robo_l2_addr_delete_by_trunk()..\n");

    if (tid == BCM_TRUNK_INVALID) {
    	return BCM_E_PARAM;
    }
    
    rv = bcm_trunk_get(unit, tid, &t_add_info);
    
    if (rv == BCM_E_NOT_FOUND) {
        return BCM_E_NONE;
    } else if (rv < 0){
        return BCM_E_INTERNAL;
    }
    
    if (SOC_IS_TBX(unit)){
        action_flag = DRV_MEM_OP_DELETE_BY_TRUNK;
        key_value = tid;

#ifdef  BCM_TB_SUPPORT
        action_flag |= (flags & _BCM_TB_L2_REPLACE_MATCH_STATIC_ONLY) ? 
                    DRV_MEM_OP_DELETE_BY_STATIC_ONLY : 0;
        action_flag |= ((flags & BCM_L2_DELETE_STATIC) || 
                        (flags & _BCM_TB_L2_REPLACE_MATCH_STATIC)) ? 
                    DRV_MEM_OP_DELETE_BY_STATIC : 0;
        action_flag |= (flags & _BCM_TB_L2_REPLACE_MATCH_MCAST_ONLY) ? 
                    DRV_MEM_OP_DELETE_BY_MCAST_ONLY : 0;
        action_flag |= (flags & _BCM_TB_L2_REPLACE_MATCH_MCAST) ? 
                    DRV_MEM_OP_DELETE_BY_MCAST : 0;
#endif  /* BCM_TB_SUPPORT */

        if (flags & BCM_L2_DELETE_PENDING){
            soc_cm_debug(DK_WARN, 
                    "Delete with PENDING flag is unavailable in TB.\n");
        }
        
        /* freeze/thaw the HW ARL to prevent the confilict action on  
         * SA_LRN_CNT handling process between SW and HW.
         */
        freezing = bcm_robo_l2_addr_freeze(unit);
        if (freezing == BCM_E_UNAVAIL || freezing == BCM_E_NONE){
            rv = DRV_MEM_DELETE(unit, DRV_MEM_ARL, &key_value, action_flag);
            if (freezing == BCM_E_NONE){
                BCM_IF_ERROR_RETURN(bcm_robo_l2_addr_thaw(unit));
            }
        } else {
            rv = freezing;
        }
    } else {
        /* 
         * If no any port assigned to the current trunk,
         * there will be no further processes.
         * Since no tgid information recorded in the arl entry of robo chip.
         */
        if (!t_add_info.num_ports) {
        	return BCM_E_UNAVAIL;
        }
        	
        for (i = 0 ; i < t_add_info.num_ports ; i++){
            BCM_IF_ERROR_RETURN(bcm_robo_l2_addr_delete_by_port
                            (unit, -1, t_add_info.tp[i], flags));
        }
    }

    return rv;
}


/*
 * Function:
 *	bcm_l2_addr_delete_by_mac_port
 * Description:
 *	Delete L2 entries associated with a MAC address and
 *	a destination module/port
 * Parameters:
 *	unit  - device unit
 *	mac   - MAC address
 *	mod   - module id
 *	port  - port
 *	flags - BCM_L2_DELETE_XXX
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	1. Static entries are removed only if BCM_L2_DELETE_STATIC flag is used.
 *	2. Robo L2 learning will prevent that the same MAC+VID learned at 
 *      different ports.
 */
int
bcm_robo_l2_addr_delete_by_mac_port(
	int	unit,
	bcm_mac_t	mac,
	bcm_module_t	mod,
	bcm_port_t	port,
	uint32	flags)
{
    bcm_l2_addr_t   l2addr;
    l2_arl_sw_entry_t		arl_search, arl_result;
    bcm_vlan_t      t_vid = VLAN_ID_INVALID;
    int             rv = BCM_E_NONE, freezing = BCM_E_NONE;
    int         	i;
    uint32          field_value = 0,vlan_count = 0;
    uint32          action_flag = 0;        

    L2_INIT(unit);        

    /* valid module/port check */
    if (BCM_GPORT_IS_SET(port)) {
        bcm_port_t  tmp_port;
        bcm_trunk_t tmp_trunk;
        bcm_module_t tmp_mod;
        int         tmp_id;
    
        BCM_IF_ERROR_RETURN(
            _bcm_robo_gport_resolve(unit, port, &tmp_mod, &tmp_port,
                                   &tmp_trunk, &tmp_id));
        port = tmp_port;
    }
    
    /* module == -1 is used for indicating to local modid */
    if (mod < 0) {
        if (!SOC_PORT_VALID(unit, port)) {
            return BCM_E_PORT;
        }
    } else { /* map module/port */
        if (!SOC_MODID_ADDRESSABLE(unit, mod)) {
            return BCM_E_BADID;
        }
    }

    /* Get the current valid VLAN entry count */
    rv = DRV_DEV_PROP_GET(unit,DRV_DEV_PROP_VLAN_ENTRY_NUM, &vlan_count);
    soc_cm_debug(DK_ARL, 
        "BCM API : bcm_robo_l2_addr_delete_by_mac_port()..\n vlan_count=%d\n",
        vlan_count);

    /* freeze/thaw the HW ARL to prevent the confilict action on  
     * SA_LRN_CNT handling process between SW and HW.
     */
    freezing = bcm_robo_l2_addr_freeze(unit);
    if (freezing == BCM_E_UNAVAIL || freezing == BCM_E_NONE){
        for (i = 1; i < vlan_count; i++) {
            t_vid = i;
    
            /* look up the l2 entry with mac+vid */   
            bcm_l2_addr_init(&l2addr, mac, t_vid);
            BCM_IF_ERROR_RETURN(_bcm_robo_l2_to_arl(unit, &arl_search, &l2addr));
        
            action_flag = DRV_MEM_OP_BY_HASH_BY_MAC | 
                        DRV_MEM_OP_BY_HASH_BY_VLANID;
            
            rv = DRV_MEM_SEARCH(unit, DRV_MEM_ARL, (uint32 *)&arl_search,
                            (uint32 *)&arl_result, NULL,
                            action_flag);
        
            if (rv == BCM_E_EXISTS) {
                if (flags & BCM_L2_DELETE_STATIC){
                    action_flag |= DRV_MEM_OP_DELETE_BY_STATIC;
                }
                if (flags & BCM_L2_DELETE_PENDING){
                    action_flag |= DRV_MEM_OP_PENDING;
                }

                /* get the port field */
                BCM_IF_ERROR_RETURN(
                        DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                            DRV_MEM_FIELD_SRC_PORT,(uint32 *)&arl_result, 
                            &field_value));
                                
                if (field_value != port){
                    continue;
                }
            
                /* delete this l2 entry */
                rv = DRV_MEM_DELETE(unit, DRV_MEM_ARL, (uint32 *)&arl_search,
                                action_flag);
                
                soc_cm_debug(DK_ARL, 
                        "bcm_robo_l2_addr_delete_by_mac(): mem_delete()=%d\n",rv);
                soc_cm_debug(DK_ARL, 
                    "ARL_DEL: MAC=%02x:%02x:%02x:%02x:%02x:%02x, "
                    "VID=0x%x, port=0x%x\n",
                    mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
                    t_vid, port);
                    
            } else if (rv == BCM_E_NOT_FOUND || rv == BCM_E_FULL){
                continue;
            } else if(rv != 0){
                soc_cm_debug(DK_WARN, 
                        "Warnning: bcm_l2_addr_delete_by_mac(),\n\t >>%s\n", 
                        bcm_errmsg(rv));
            }
        }
        rv = BCM_E_NONE;
        if (freezing == BCM_E_NONE){
            BCM_IF_ERROR_RETURN(bcm_robo_l2_addr_thaw(unit));
        }
    } else {
        rv = freezing;
    }

    return rv;
}

/*
 * Function:
 *	bcm_l2_addr_delete_by_vlan_port
 * Description:
 *	Delete L2 entries associated with a VLAN and
 *	a destination module/port
 * Parameters:
 *	unit  - device unit
 *	vid   - VLAN id
 *	mod   - module id
 *	port  - port
 *	flags - BCM_L2_DELETE_XXX
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	Static entries are removed only if BCM_L2_DELETE_STATIC flag is used.
 *	L2 aging and learning are disabled during this operation.
 */
int 
bcm_robo_l2_addr_delete_by_vlan_port(int unit, bcm_vlan_t vid,
                       bcm_module_t mod, bcm_port_t port,
                       uint32 flags)
{
    int                 rv = BCM_E_NONE, freezing = BCM_E_NONE;
    l2_arl_sw_entry_t   arl_entry;
    uint32              fld_value = 0;
    uint32              action_flag = 0;
    
    L2_INIT(unit);
    
    sal_memset(&arl_entry, 0, sizeof (arl_entry));

    /* valid VLAN check */
    if(vid > BCM_VLAN_MAX){
        return BCM_E_PARAM;
    }
    
    soc_cm_debug(DK_ARL, 
            "BCM API : bcm_robo_l2_addr_delete_by_vlan_port()..\n");
    /* set VID field */
    fld_value = vid; /* vid value */
    BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
            DRV_MEM_FIELD_VLANID, (uint32 *)&arl_entry, &fld_value));
            
    /* valid module/port check */
    if (BCM_GPORT_IS_SET(port)) {
        bcm_port_t  tmp_port;
        bcm_trunk_t tmp_trunk;
        bcm_module_t tmp_mod;
        int         tmp_id;
    
        BCM_IF_ERROR_RETURN(
            _bcm_robo_gport_resolve(unit, port, &tmp_mod, &tmp_port,
                                   &tmp_trunk, &tmp_id));
        port = tmp_port;
    }
    
    /* module == -1 is used for indicating to local modid */
    if (mod < 0) {
        if (!SOC_PORT_VALID(unit, port)) {
            return BCM_E_PORT;
        }
    } else { /* map module/port */
        if (!SOC_MODID_ADDRESSABLE(unit, mod)) {
            return BCM_E_BADID;
        }
    }

    fld_value = port; /* port value */
    /* set Port field */
    BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
            DRV_MEM_FIELD_SRC_PORT, (uint32 *)&arl_entry, &fld_value));
    action_flag = DRV_MEM_OP_DELETE_BY_VLANID | 
                    DRV_MEM_OP_DELETE_BY_PORT ;
#ifdef  BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)){
        action_flag |= (flags & _BCM_TB_L2_REPLACE_MATCH_STATIC_ONLY) ? 
                    DRV_MEM_OP_DELETE_BY_STATIC_ONLY : 0;
        action_flag |= ((flags & BCM_L2_DELETE_STATIC) || 
                        (flags & _BCM_TB_L2_REPLACE_MATCH_STATIC)) ? 
                    DRV_MEM_OP_DELETE_BY_STATIC : 0;
        action_flag |= (flags & _BCM_TB_L2_REPLACE_MATCH_MCAST_ONLY) ? 
                    DRV_MEM_OP_DELETE_BY_MCAST_ONLY : 0;
        action_flag |= (flags & _BCM_TB_L2_REPLACE_MATCH_MCAST) ? 
                    DRV_MEM_OP_DELETE_BY_MCAST : 0;
    } else {
        action_flag |= (flags & BCM_L2_DELETE_STATIC) ? 
                        DRV_MEM_OP_DELETE_BY_STATIC : 0;
    }
#else   /* BCM_TB_SUPPORT */
    action_flag |= (flags & BCM_L2_DELETE_STATIC) ? 
                    DRV_MEM_OP_DELETE_BY_STATIC : 0;
#endif  /* BCM_TB_SUPPORT */
    action_flag |= (flags & BCM_L2_DELETE_PENDING) ? 
                    DRV_MEM_OP_PENDING : 0;
    /* freeze/thaw the HW ARL to prevent the confilict action on SA_LRN_CNT 
     * handling process between SW and HW.
     */
    freezing = bcm_robo_l2_addr_freeze(unit);
    if (freezing == BCM_E_UNAVAIL || freezing == BCM_E_NONE){
        rv = DRV_MEM_DELETE(
                unit, DRV_MEM_ARL,(uint32 *)&arl_entry, action_flag);
        if (freezing == BCM_E_NONE){
            BCM_IF_ERROR_RETURN(bcm_robo_l2_addr_thaw(unit));
        }
    } else {
        rv = freezing;
    }

    return rv;
}                       

/*
 * Function:
 *	bcm_l2_addr_delete_by_vlan_trunk
 * Description:
 *	Delete L2 entries associated with a VLAN and a
 *      destination trunk.
 * Parameters:
 *	unit  - device unit
 *	vid   - VLAN id
 *	tid   - trunk group id
 *	flags - BCM_L2_DELETE_XXX
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	Static entries are removed only if BCM_L2_DELETE_STATIC flag is used.
 *	L2 aging and learning are disabled during this operation.
 */
int
bcm_robo_l2_addr_delete_by_vlan_trunk(int unit, bcm_vlan_t vid,
                                 bcm_trunk_t tid, uint32 flags)
{
    bcm_trunk_add_info_t t_add_info;
    int         i, rv = BCM_E_NONE;
    
    L2_INIT(unit);
    
    soc_cm_debug(DK_ARL, 
            "BCM API : bcm_robo_l2_addr_delete_by_vlan_trunk()..\n");

    if (tid == BCM_TRUNK_INVALID) {
    	return BCM_E_PARAM;
    }
    
    rv = bcm_trunk_get(unit, tid, &t_add_info);
    
    if (rv == BCM_E_NOT_FOUND) {
        return BCM_E_NONE;
    } else if (rv < 0){
        return BCM_E_INTERNAL;
    }
    
    /* 
     * If no any port assigned to the current trunk,
     * there will be no further processes.
     * Since no tgid information recorded in the arl entry of robo chip.
     */
    if (!t_add_info.num_ports) {
    	return BCM_E_UNAVAIL;
    }
    	
    for (i = 0 ; i < t_add_info.num_ports ; i++){
        BCM_IF_ERROR_RETURN(bcm_robo_l2_addr_delete_by_vlan_port
                        (unit, vid,-1, t_add_info.tp[i], flags));
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_robo_l2_addr_get
 * Description:
 *	Given a MAC address and VLAN ID, check if the entry is present
 *	in the L2 table, and if so, return all associated information.
 * Parameters:
 *	unit - RoboSwitch PCI device unit number (driver internal).
 *	mac - input MAC address to search
 *	vid - input VLAN ID to search
 *	l2addr - Pointer to bcm_l2_addr_t structure to receive results
 * Returns:
 *	BCM_E_NONE		Success (l2addr filled in)
 *	BCM_E_PARAM		Illegal parameter (NULL pointer)
 *	BCM_E_INTERNAL		Chip access failure
 *	BCM_E_NOT_FOUND	Address not found (l2addr not filled in)
 */
int 
bcm_robo_l2_addr_get(int unit, bcm_mac_t mac, bcm_vlan_t vid, 
                    bcm_l2_addr_t *l2addr)
{
    bcm_l2_addr_t	l2_search;
    l2_arl_sw_entry_t		arl_entry, arl_result;
    int	    	rv = BCM_E_NONE;

    L2_INIT(unit);
    
    soc_cm_debug(DK_ARL, 
            "BCM API : bcm_robo_l2_addr_get()..\n");
    bcm_l2_addr_init(&l2_search, mac, vid);

    BCM_IF_ERROR_RETURN(_bcm_robo_l2_to_arl(unit, &arl_entry, &l2_search));

    rv = DRV_MEM_SEARCH(unit, DRV_MEM_ARL, (uint32 *)&arl_entry,
            (uint32 *)&arl_result, NULL,
            (DRV_MEM_OP_BY_HASH_BY_MAC | DRV_MEM_OP_BY_HASH_BY_VLANID));

    if (rv == BCM_E_EXISTS) {
        _bcm_robo_l2_from_arl(unit, l2addr, &arl_result);
    } else {
        if (rv == BCM_E_FULL){
            rv = BCM_E_NOT_FOUND;
        }
        return rv;
    }

    return BCM_E_NONE;

}



/*
 * Function:
 *	bcm_l2_key_dump
 * Purpose:
 *	Dump the key (VLAN+MAC) portion of a hardware-independent
 *	L2 address for debugging
 * Parameters:
 *	unit - Unit number
 *	pfx - String to print before output
 *	entry - Hardware-independent L2 entry to dump
 *	sfx - String to print after output
 */

int
bcm_robo_l2_key_dump(int unit, char *pfx, bcm_l2_addr_t *entry, char *sfx)
{
    /*
     * In VxWorks, in interrupt context, soc_cm_print uses logMsg, which only
     * allows up to 6 args.  That's why the MAC address is formatted as
     * two hex numbers here.
     */
    soc_cm_print("l2: %sVLAN=0x%03x MAC=0x%02x%02x%02x"
                 "%02x%02x%02x%s", pfx, entry->vid,
         entry->mac[0], entry->mac[1], entry->mac[2],
         entry->mac[3], entry->mac[4], entry->mac[5], sfx);
    return BCM_E_NONE;
}

/****************************************************************************
 *
 * ARL Message Registration
 *
 ****************************************************************************/

#define L2_CB_MAX		3

typedef struct arl_cb_entry_s {
    bcm_l2_addr_callback_t	fn;
    void	        *fn_data;
} l2_cb_entry_t;

typedef struct l2_data_s {
    l2_cb_entry_t		cb[L2_CB_MAX];
    int	        	cb_count;
} l2_data_t;

static l2_data_t l2_data[SOC_MAX_NUM_SWITCH_DEVICES];

/*
 * Function:
 * 	_bcm_l2_addr_callback
 * Description:
 *	Callback used with chip addr registration functions.
 *	This callback calls all the top level client callbacks.
 * Parameters:
 *	unit - RoboSwitch unit number (driver internal).
 *	l2addr
 *	insert
 *	userdata
 * Returns:
 *
 */
static void
_bcm_l2_addr_callback(int unit,
              bcm_l2_addr_t *l2addr,
              int insert,
              void *userdata)
{
    l2_data_t	    *ad = &l2_data[unit];
    int i;

    for(i = 0; i < L2_CB_MAX; i++) {
        if(ad->cb[i].fn) {
            ad->cb[i].fn(unit, l2addr, insert, ad->cb[i].fn_data);
        }
    }
}

/*
 * Function:
 *	bcm_robo_l2_addr_register
 * Description:
 *	Register a callback routine that will be called whenever
 *	an entry is inserted into or deleted from the L2 address table.
 * Parameters:
 *	unit - RoboSwitch PCI device unit number (driver internal).
 *	fn - Callback function of type bcm_l2_addr_callback_t.
 *	fn_data - Arbitrary value passed to callback along with messages
 * Returns:
 *	BCM_E_NONE		Success, handle valid
 *	BCM_E_MEMORY		Out of memory
 *	BCM_E_INTERNAL		Chip access failure
 */
int
bcm_robo_l2_addr_register(int unit,
             bcm_l2_addr_callback_t fn, 
             void *fn_data)
{
    l2_data_t   *ad = &l2_data[unit];
    int         i;

    L2_INIT(unit);
    
    soc_cm_debug(DK_ARL, 
            "BCM API : bcm_robo_l2_addr_register()..\n");
    
    BCM_IF_ERROR_RETURN
        (bcm_53xx_l2_addr_register(unit, _bcm_l2_addr_callback, NULL));

    if (ad->cb_count > L2_CB_MAX) {
        soc_cm_debug(DK_WARN,"%s,!! cb_count=%d over valid range!!\n", 
                FUNCTION_NAME(),ad->cb_count);
        return BCM_E_MEMORY;
    }

    for (i = 0; i < L2_CB_MAX; i++) {
        if (ad->cb[i].fn == NULL) {
            ad->cb[i].fn = fn;
            ad->cb[i].fn_data = fn_data;
            ad->cb_count++;
            break;
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_robo_l2_addr_unregister
 * Description:
 *	Unregister a previously registered callback routine.
 * Parameters:
 *	unit - RoboSwitch PCI device unit number (driver internal).
 *	fn - Same callback function used to register callback
 *	fn_data - Same arbitrary value used to register callback
 * Returns:
 *	BCM_E_NONE		Success, handle valid
 *	BCM_E_MEMORY		Out of memory
 *	BCM_E_INTERNAL		Chip access failure
 * Notes:
 *	Both callback and userdata must match from original register call.
 */
int
bcm_robo_l2_addr_unregister(int unit,
               bcm_l2_addr_callback_t fn, 
               void *fn_data)
{
    l2_data_t   *ad = &l2_data[unit];
    int         i, rv = BCM_E_NOT_FOUND;

    L2_INIT(unit);
    
    soc_cm_debug(DK_ARL, 
            "BCM API : bcm_robo_l2_addr_unregister()..\n");
    for (i = 0; i < L2_CB_MAX; i++) {
        if((ad->cb[i].fn == fn) && (ad->cb[i].fn_data == fn_data)) {
            ad->cb[i].fn = NULL;
            ad->cb[i].fn_data = NULL;
            ad->cb_count--;
            if (ad->cb_count == 0) {
                rv = bcm_53xx_l2_addr_unregister(unit,
                            _bcm_l2_addr_callback,
                            NULL);
            }
        }
    }
    return rv;
}
                  

/*
 * Set L2 table aging time
 */

/*
 * Function:
 *	bcm_robo_l2_age_timer_set
 * Description:
 *	Set the age timer for all blocks.
 *	Setting the value to 0 disables the age timer.
 * Parameters:
 *	unit - RoboSwitch PCI device unit number (driver internal).
 *	age_seconds - Age timer value in seconds
 * Returns:
 *	BCM_E_NONE		Success
 *	BCM_E_INTERNAL		Chip access failure
 */
int
bcm_robo_l2_age_timer_set(int unit, int age_seconds)
{
    uint32  max_value = 0;
    uint32  enabled = 0;

    soc_cm_debug(DK_ARL, 
            "BCM API : bcm_robo_l2_age_timer_set()..\n");
    BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET(unit, 
            DRV_DEV_PROP_AGE_TIMER_MAX_S, &max_value));

    if (age_seconds > max_value) {
        return BCM_E_PARAM;
    }

    enabled = age_seconds ? TRUE : FALSE;

    BCM_IF_ERROR_RETURN(DRV_AGE_TIMER_SET(unit, 
            enabled, (uint32)age_seconds));

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_robo_l2_age_timer_get
 * Description:
 *	Returns the current age timer value.
 *	The value is 0 if aging is not enabled.
 * Parameters:
 *	unit - RoboSwitch PCI device unit number (driver internal).
 *	age_seconds - Place to store returned age timer value in seconds
 * Returns:
 *	BCM_E_NONE		Success
 *	BCM_E_INTERNAL		Chip access failure
 */
int
bcm_robo_l2_age_timer_get(int unit, int *age_seconds)
{
    uint32 enabled;

    soc_cm_debug(DK_ARL, 
            "BCM API : bcm_robo_l2_age_timer_get()..\n");
    if (age_seconds == NULL){
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(DRV_AGE_TIMER_GET(unit, &enabled, 
            (uint32 *)age_seconds));

    if (!enabled) {
        *age_seconds = 0;
    }

    return BCM_E_NONE;
}
    

/*
 * Manage BPDU addresses
 */

/*
 * Function:
 *	bcm_robo_l2_bpdu_count
 * Description:
 *	Retrieve the number of BPDU addresses available on the
 *      specified unit.
 * Parameters:
 *	unit - RoboSwitch PCI device unit number (driver internal).
 * Returns:
 *	number of BPDU address entries
 */
int
bcm_robo_l2_bpdu_count(int unit)
{
    int count = 0;
    
    soc_cm_debug(DK_ARL, 
            "BCM API : bcm_robo_l2_bpdu_count()..\n");
    BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET(unit, 
            DRV_DEV_PROP_BPDU_NUM, (uint32 *) &count));
    return count;
}

/*
 * Function:
 *	bcm_robo_l2_bpdu_set
 * Description:
 *	Set BPDU address
 * Parameters:
 *	unit - switch unit
 *	index - BPDU index
 *	addr - 6 byte mac address
 * Returns:
 *	BCM_E_NONE		Success
 *	BCM_E_*        	Failure
 */
int
bcm_robo_l2_bpdu_set(int unit, int index, sal_mac_addr_t addr)
{
    int	nbpdu = 0;
    pbmp_t null_pbmp;

    soc_cm_debug(DK_ARL, 
            "BCM API : bcm_robo_l2_bpdu_set()..\n");

    nbpdu = bcm_l2_bpdu_count(unit);

    if (nbpdu == 0) {
	    return BCM_E_UNAVAIL;
    }
    if (index < 0 || index >= nbpdu) {
	    return BCM_E_PARAM;
    }
    BCM_PBMP_CLEAR(null_pbmp);
    BCM_IF_ERROR_RETURN(DRV_MAC_SET(unit, null_pbmp, 
            DRV_MAC_CUSTOM_BPDU, addr, index));
    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_robo_l2_bpdu_get
 * Description:
 *	Return BPDU address
 * Parameters:
 *	unit - switch unit
 *	index - BPDU index
 *	addr - (OUT) 6 byte mac address
 * Returns:
 *	BCM_E_NONE		Success
 *	BCM_E_*        	Failure
 */
int
bcm_robo_l2_bpdu_get(int unit, int index, sal_mac_addr_t *addr)
{
    int	nbpdu = 0;
    pbmp_t null_parm;

    soc_cm_debug(DK_ARL, 
            "BCM API : bcm_robo_l2_bpdu_get()..\n");

    nbpdu = bcm_l2_bpdu_count(unit);

    if (nbpdu == 0) {
	    return BCM_E_UNAVAIL;
    }
    if (index < 0 || index >= nbpdu) {
	    return BCM_E_PARAM;
    }
   
    /* null_parm is a null parameter to suit soc driver prototype*/
    BCM_IF_ERROR_RETURN(DRV_MAC_GET(unit, index, 
            DRV_MAC_CUSTOM_BPDU, &null_parm, *addr));
    
    return BCM_E_NONE;
}
    
/*
 * Function:
 *      bcm_l2_clear
 * Purpose:
 *      Clear the L2 layer
 * Parameters:
 *      unit  - BCM unit number
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_l2_clear(int unit)
{
    uint32  no_sw_arl = 0;
    soc_control_t   *soc = SOC_CONTROL(unit);
    
    L2_INIT(unit);

    BCM_IF_ERROR_RETURN(bcm_l2_detach(unit));

    soc_cm_debug(DK_ARL, 
            "BCM API : bcm_robo_l2_clear()..\n");
    /*
     * Call chip-dependent initialization
     */
    soc_robo_arl_unregister(unit, _bcm_robo_l2_register_callback, NULL);
    BCM_IF_ERROR_RETURN(DRV_MEM_CLEAR(unit, DRV_MEM_ARL));
    
    /* ROBO's SW ARL reset due to HW L2 table cleared */
    no_sw_arl = (soc->arl_table == NULL) ? TRUE : FALSE;
    if (!no_sw_arl) {
        /* force ARL thread restart to reset ROBO's SW ARL */
        BCM_IF_ERROR_RETURN(soc_robo_arl_mode_set(unit, ARL_MODE_ROBO_POLL));
    }
    
    /* bcm_l2_register clients */
    soc_robo_arl_register(unit, _bcm_robo_l2_register_callback, NULL);


    /* Clear l2_data structure */
    l2_data[unit].cb_count = 0;
    sal_memset(&l2_data[unit].cb, 0, sizeof(l2_data[unit].cb));

    /* BCM shadow table will go away soon */

    _l2_init[unit] = 1;		/* some positive value */
    
    return BCM_E_NONE;
}


/*
 * Given an L2 or L2 multicast address, return any existing L2 or L2
 * multicast addresses which might prevent it from being inserted
 * because a chip resource (like a hash bucket) is full.
 */

/*
 * Function:
 *	bcm_robo_l2_conflict_get
 * Purpose:
 *	Given an L2 address, return existing addresses which could conflict.
 * Parameters:
 *	unit	    - switch device
 *	addr	    - l2 address to search for conflicts
 *	cf_array	- (OUT) list of l2 addresses conflicting with addr
 *	cf_max	    - number of entries allocated to cf_array
 *	cf_count	- (OUT) actual number of cf_array entries filled
 * Returns:
 *      BCM_E_XXX
 * Note :
 * 1. Different ROBO chips has different l2 hash bucket size.
 *    - l2x.h defined every ROBO chip bucket size annd the max bucket size 
 *      in ROBO chips.
 * 2. all valid entries in the hashed bucket size must be reported through 
 *      this API.
 */

int
bcm_robo_l2_conflict_get(int unit, bcm_l2_addr_t *addr,
            bcm_l2_addr_t *cf_array, int cf_max,
            int *cf_count)
{
    int     i, search_result = SOC_E_NONE, rv = BCM_E_NONE;
    uint64  mac_field;
    l2_arl_sw_entry_t   arl_entry, output[ROBO_MAX_L2_BUCKET_SIZE];
    l2_arl_sw_entry_t   *cf_entry;

    L2_INIT(unit);

    BCM_IF_ERROR_RETURN(_bcm_robo_l2_to_arl(unit, &arl_entry, addr));

    *cf_count = 0;
    sal_memset(output, 0, sizeof(output));
    search_result = DRV_MEM_SEARCH(unit, DRV_MEM_ARL, 
            (uint32 *)&arl_entry, (uint32 *)output, NULL, 
            (DRV_MEM_OP_BY_HASH_BY_MAC | DRV_MEM_OP_BY_HASH_BY_VLANID | 
            DRV_MEM_OP_SEARCH_CONFLICT));

    if ((search_result == SOC_E_EXISTS)) {
        for (i = 0; (i < ROBO_MAX_L2_BUCKET_SIZE) && (*cf_count < cf_max); 
                i++){

            cf_entry = &output[i];
            COMPILER_64_ZERO(mac_field);
            rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_MAC,
                    (uint32 *)cf_entry, (uint32 *)&mac_field);

            if (!COMPILER_64_IS_ZERO(mac_field)){
               _bcm_robo_l2_from_arl(unit, &cf_array[*cf_count], cf_entry);
                *cf_count += 1;
                soc_cm_debug(DK_ARL, 
                        "%s ,conflict entry on bin[%d] is 0x%x,0x%x,0x%x\n",
                        FUNCTION_NAME(), i, 
                        cf_entry->entry_data[2], 
                        cf_entry->entry_data[1], 
                        cf_entry->entry_data[0]);
            }
        }
    }

    soc_cm_debug(DK_ARL, 
            "%s, cf_status=%d, get %d conflict entry.\n", 
            FUNCTION_NAME(), search_result, *cf_count);
    return (BCM_E_NONE);
}

int
bcm_robo_l2_tunnel_add(int unit, bcm_mac_t mac, bcm_vlan_t vlan)
{
    return BCM_E_UNAVAIL;
}
int
bcm_robo_l2_tunnel_delete(int unit, bcm_mac_t mac, bcm_vlan_t vlan)
{
    return BCM_E_UNAVAIL;
}
int
bcm_robo_l2_tunnel_delete_all(int unit)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *     bcm_robo_l2_learn_limit_set
 * Description:
 *     Set the L2 MAC learning limit
 * Parameters:
 *     unit        device number
 *     limit       learn limit control info
 *                 limit->flags - qualifiers bits and action bits
 *                 limit->vlan - vlan identifier
 *                 limit->port - port number
 *                 limit->trunk - trunk identifier
 *                 limit->limit - max number of learned entry, -1 for unlimit
 * Return:
 *     BCM_E_XXX
 */
int
bcm_robo_l2_learn_limit_set(int unit, bcm_l2_learn_limit_t *limit)
{
    bcm_pbmp_t t_pbm;
    uint32 type = 0, action = 0, temp = 0;

    if (!soc_feature(unit, soc_feature_mac_learn_limit)) {
        return BCM_E_UNAVAIL;
    }

    BCM_PBMP_CLEAR(t_pbm); 

    if (!limit) {
        return BCM_E_PARAM;
    }

    /* check valid limit value */
    temp = soc_robo_mem_index_max(unit, L2_ARLm);
    if (limit->limit > (int)temp){
        return BCM_E_PARAM;
    }

    type = limit->flags &
           (BCM_L2_LEARN_LIMIT_SYSTEM | BCM_L2_LEARN_LIMIT_VLAN | 
            BCM_L2_LEARN_LIMIT_PORT | BCM_L2_LEARN_LIMIT_TRUNK);

    action = limit->flags & 
             (BCM_L2_LEARN_LIMIT_ACTION_DROP | BCM_L2_LEARN_LIMIT_ACTION_CPU |
              BCM_L2_LEARN_LIMIT_ACTION_PREFER);

    if (!type) {
        return BCM_E_PARAM;
    }

    if (type != BCM_L2_LEARN_LIMIT_SYSTEM &&
        (action & BCM_L2_LEARN_LIMIT_ACTION_PREFER)) {
        return BCM_E_PARAM;
    }

    if ((action & BCM_L2_LEARN_LIMIT_ACTION_DROP) && 
        (action & BCM_L2_LEARN_LIMIT_ACTION_CPU)) {
        if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)){
            /* support both acitons but can't both worked at the same time */
            return BCM_E_PARAM;
        }
    }

    if (type & BCM_L2_LEARN_LIMIT_SYSTEM) {
        return BCM_E_UNAVAIL;
    }

    if (type & BCM_L2_LEARN_LIMIT_PORT) {

        if (BCM_GPORT_IS_SET(limit->port)) {
            BCM_IF_ERROR_RETURN(bcm_robo_port_local_get(
                    unit, limit->port, &(limit->port)));
        }

        if (!SOC_PORT_VALID(unit, limit->port)) {
            return BCM_E_PORT;
        }

        BCM_PBMP_PORT_ADD(t_pbm, limit->port);
        if (limit->limit < 0){
            temp = DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_NONE;
        } else {
            if (action & BCM_L2_LEARN_LIMIT_ACTION_DROP) {
                temp = DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_DROP;
            } else if (action & BCM_L2_LEARN_LIMIT_ACTION_CPU) {
                temp = DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_CPU;
            } else {
                /* BCM_L2_LEARN_LIMIT_ACTION_PREFER is unavailable for ROBO :
                 *  
                 * P.S indicated in API guide to be served on SYSTEM basis 
                 *      learn limit only.
                 */
                return BCM_E_UNAVAIL;
            }
        }
        BCM_IF_ERROR_RETURN((DRV_PORT_SET
            (unit, t_pbm, temp, limit->limit)));
    }

    if (type & BCM_L2_LEARN_LIMIT_TRUNK) {
        return BCM_E_UNAVAIL;
    }

    if (type & BCM_L2_LEARN_LIMIT_VLAN) {
        return BCM_E_UNAVAIL;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     bcm_robo_l2_learn_limit_get
 * Description:
 *     Get the L2 MAC learning limit
 * Parameters:
 *     unit        device number
 *     limit       learn limit control info
 *                 limit->flags - qualifiers bits and action bits
 *                 limit->vlan - vlan identifier
 *                 limit->port - port number
 *                 limit->trunk - trunk identifier
 *                 limit->limit - max number of learned entry, -1 for unlimit
 * Return:
 *     BCM_E_XXX
 */
int
bcm_robo_l2_learn_limit_get(int unit, bcm_l2_learn_limit_t *limit)
{
    int rv;
    uint32 type = 0, action = 0, temp = 0;
    int max = 0;

    if (!soc_feature(unit, soc_feature_mac_learn_limit)) {
        return BCM_E_UNAVAIL;
    }

    type = limit->flags &
           (BCM_L2_LEARN_LIMIT_SYSTEM | BCM_L2_LEARN_LIMIT_VLAN | 
            BCM_L2_LEARN_LIMIT_PORT | BCM_L2_LEARN_LIMIT_TRUNK);

    rv = BCM_E_UNAVAIL;
    switch (type) {
    case BCM_L2_LEARN_LIMIT_PORT:
        if (BCM_GPORT_IS_SET(limit->port)) {
            BCM_IF_ERROR_RETURN(bcm_robo_port_local_get(
                    unit, limit->port, &(limit->port)));
        }

        if (!SOC_PORT_VALID(unit, limit->port)) {
            return BCM_E_PORT;
        }

        rv = DRV_PORT_GET(unit, limit->port, 
            DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_DROP, &temp);
        if (temp && (rv == SOC_E_NONE)) {
            action |= BCM_L2_LEARN_LIMIT_ACTION_DROP;
        }
        
        rv = DRV_PORT_GET(unit, limit->port, 
            DRV_PORT_PROP_L2_LEARN_LIMIT_PORT_ACTION_CPU, &temp);
        if (temp && (rv == SOC_E_NONE)) {
            action |= BCM_L2_LEARN_LIMIT_ACTION_CPU;
        }

        if (!action) {
            max = -1;
        } else {
            BCM_IF_ERROR_RETURN(DRV_ARL_LEARN_COUNT_GET(unit, 
                    limit->port, DRV_PORT_SA_LRN_CNT_LIMIT, &max));
        }
        break;

    case BCM_L2_LEARN_LIMIT_SYSTEM:
    case BCM_L2_LEARN_LIMIT_TRUNK:
    case BCM_L2_LEARN_LIMIT_VLAN:
        return BCM_E_UNAVAIL;

    default:
        return BCM_E_PARAM;
    }

    limit->flags |= action;
    limit->limit = max;

    return rv;
}

int
bcm_robo_l2_learn_class_set(int unit, int class, int class_prio, uint32 flags)
{   
    return BCM_E_UNAVAIL;
}

int
bcm_robo_l2_learn_class_get(int unit, int class, int *class_prio,
                            uint32 *flags)
{   
    return BCM_E_UNAVAIL;
}

int
bcm_robo_l2_learn_port_class_set(int unit,  bcm_gport_t port, int class)
{
    return BCM_E_UNAVAIL;
}

int
bcm_robo_l2_learn_port_class_get(int unit,  bcm_gport_t port, int *class)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_l2_addr_delete_by_mac_vpn
 * Description:
 *      Delete L2 entry with matching MAC address and VPN.
 * Parameters:
 *      unit  - device unit
 *      mac   - MAC address
 *      vpn   - L2 VPN identifier
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_l2_addr_delete_by_mac_vpn(int unit, bcm_mac_t mac, bcm_vpn_t vpn)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_l2_addr_delete_by_vpn
 * Description:
 *      Delete L2 entries associated with an L2 VPN.
 * Parameters:
 *      unit  - device unit
 *      vpn   - L2 VPN identifier
 *      flags - BCM_L2_DELETE_XXX
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Static entries are removed only if BCM_L2_DELETE_STATIC flag is used.
 *      L2 aging and learning are disabled during this operation.
 */
int
bcm_robo_l2_addr_delete_by_vpn(int unit, bcm_vpn_t vpn, uint32 flags)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *     _bcm_robo_l2_traverse_mem
 * Description:
 *      Helper function to _bcm_robo_l2_traverse to itterate over given memory 
 *      and actually read the table and parse entries.
 * Parameters:
 *     unit         device number
 *      mem         L2 memory to read
 *     trav_st      Traverse structure with all the data.
 * Return:
 *     BCM_E_XXX
 */
int 
_bcm_robo_l2_traverse_mem(int unit, soc_mem_t mem, _bcm_robo_l2_traverse_t *trav_st)
{
    /* Indexes to iterate over memories, chunks and entries */
    int             chnk_idx, ent_idx;
    int             chnk_idx_max = 0, mem_idx_max = 0;
    int             buf_size = 0, chunksize = 0, copysize = 0;
    int             rv = BCM_E_NONE;
    /* Buffer to store chunk of L2 table we currently work on */
    uint32          *l2_tbl_chnk, valid = 0;
    soc_control_t   *soc = SOC_CONTROL(unit);
    l2_arl_sw_entry_t *l2x_entry;
    bcm_l2_addr_t   *this_l2addr;

    if (!soc->arl_table) {
        /* mean's the SW ARL is empty */
        return BCM_E_NONE;
    }

    if (!soc_robo_mem_index_count(unit, mem)) {
        return BCM_E_NONE;
    }

    chunksize = soc_property_get(unit, spn_L2DELETE_CHUNKS,
                                 L2_ROBO_MEM_CHUNKS_DEFAULT);

    buf_size = sizeof(l2_arl_sw_entry_t) * chunksize;
    l2_tbl_chnk = soc_cm_salloc(unit, buf_size, "l2 traverse");
    if (NULL == l2_tbl_chnk) {
        return BCM_E_MEMORY;;
    }

    mem_idx_max = soc_robo_mem_index_max(unit, mem);
    for (chnk_idx = soc_robo_mem_index_min(unit, mem); 
         chnk_idx <= mem_idx_max; 
         chnk_idx += chunksize) {
        sal_memset((void *)l2_tbl_chnk, 0, buf_size);

        chnk_idx_max = 
            ((chnk_idx + chunksize) < mem_idx_max) ? 
            chnk_idx + chunksize : mem_idx_max;

        copysize = chnk_idx_max - chnk_idx + 
            ((chnk_idx_max == mem_idx_max) ? 1 : 0);
        ARL_SW_TABLE_LOCK(soc);
        sal_memcpy((void *)l2_tbl_chnk, &soc->arl_table[chnk_idx], 
            sizeof(l2_arl_sw_entry_t) * copysize);
        ARL_SW_TABLE_UNLOCK(soc);
        
        l2x_entry = (l2_arl_sw_entry_t *)l2_tbl_chnk;
        for (ent_idx = 0 ; ent_idx < copysize; ent_idx ++) {
            rv = DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, DRV_MEM_FIELD_VALID, 
    		        (uint32 *)l2x_entry, &valid);
    		
            if (valid){
                this_l2addr = (bcm_l2_addr_t *)trav_st->data;
                
                /* special process for TB on assigning l2addr->port at 
                 * GPORT_SUBPORT_PORT type to requet the reported l2 entry 
                 * on reporting most detail information(include vport)
                 */
                if (SOC_IS_TBX(unit)){
                    /* assigning a GPORT type only, id assigned to 0 */
                    BCM_GPORT_SUBPORT_PORT_SET(this_l2addr->port, 0);
                }
                _bcm_robo_l2_from_arl(unit, this_l2addr, l2x_entry);    
                soc_cm_debug(DK_ARL | DK_TESTS, "l2x_id=%d,",chnk_idx);
                rv = trav_st->user_cb(unit, trav_st->data, trav_st->user_data);
                if (BCM_FAILURE(rv)) {
                    soc_cm_sfree(unit, l2_tbl_chnk);
                    return rv;
                }
            }
            l2x_entry++;
        }
    }           
    soc_cm_sfree(unit, l2_tbl_chnk);
    return BCM_E_NONE;        
}


/*
 * Function:
 *     _bcm_robo_l2_traverse
 * Description:
 *      Helper function to bcm_robo_l2_traverse to itterate over table 
 *      and actually read the momery
 * Parameters:
 *     unit         device number
 *     trav_st      Traverse structure with all the data.
 * Return:
 *     BCM_E_XXX
 */
int 
_bcm_robo_l2_traverse(int unit, _bcm_robo_l2_traverse_t *trav_st)
{
    int rv = BCM_E_UNAVAIL; 

    rv = _bcm_robo_l2_traverse_mem(unit, L2_ARLm, trav_st);

    return rv;
}


/*
 * Function:
 *     bcm_robo_l2_traverse
 * Description:
 *     To traverse the L2 table and call provided callback function with matched entry
 * Parameters:
 *     unit         device number
 *     trav_fn      User specified callback function 
 *     user_data    User specified cookie
 * Return:
 *     BCM_E_XXX
 */
int 
bcm_robo_l2_traverse(int unit, bcm_l2_traverse_cb trav_fn, void *user_data)
{
    _bcm_robo_l2_traverse_t  trav_st;
    bcm_l2_addr_t       l2_entry;
       
    if (!trav_fn) {
        return (BCM_E_PARAM);
    }

    sal_memset(&trav_st, 0, sizeof(_bcm_robo_l2_traverse_t));
    sal_memset(&l2_entry, 0, sizeof(bcm_l2_addr_t));

    trav_st.pattern = NULL;
    trav_st.data = &l2_entry;
    trav_st.user_cb = trav_fn;
    trav_st.user_data = user_data;

    return (_bcm_robo_l2_traverse(unit, &trav_st));
}

#define _BCM_L2_REPLACE_ALL_LEGAL            (BCM_L2_REPLACE_MATCH_MAC | \
                                              BCM_L2_REPLACE_MATCH_VLAN | \
                                              BCM_L2_REPLACE_MATCH_DEST | \
                                              BCM_L2_REPLACE_MATCH_STATIC   | \
                                              BCM_L2_REPLACE_DELETE | \
                                              BCM_L2_REPLACE_NEW_TRUNK | \
                                              BCM_L2_REPLACE_PENDING)
/*
 * Function:
 *     _bcm_robo_l2_replace_flags_validate
 * Description:
 *     THelper function to bcm_l2_replace API to validate given flags
 * Parameters:
 *     flags        flags BCM_L2_REPLACE_* 
 * Return:
 *     BCM_E_NONE - OK 
 *     BCM_E_PARAM - Failure
 */
int 
_bcm_robo_l2_replace_flags_validate(uint32 flags)
{
    if (!flags) {
        return BCM_E_PARAM;
    }
    if ((flags & BCM_L2_REPLACE_DELETE) && 
            (flags & BCM_L2_REPLACE_NEW_TRUNK)) {
        return BCM_E_PARAM;
    }
    if ((flags > (BCM_L2_REPLACE_MATCH_MAC | BCM_L2_REPLACE_MATCH_VLAN |
        BCM_L2_REPLACE_MATCH_DEST | BCM_L2_REPLACE_MATCH_STATIC)) && 
        (flags < BCM_L2_REPLACE_DELETE)) {
        return BCM_E_PARAM;
    }
    if (flags > _BCM_L2_REPLACE_ALL_LEGAL) {
        return BCM_E_PARAM;
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_robo_l2_addr_replace_by_mac
 * Description:
 *     Helper function to bcm_l2_replace API to replace l2 entries by mac
 * Parameters:
 *     unit         device number
 *     flags        flags BCM_L2_REPLACE_* 
 *     rep_st       structure with information of what to replace 
 * Return:
 *     BCM_E_XXX
 */
int 
_bcm_robo_l2_addr_replace_by_mac(int unit, uint32 flags, 
        _bcm_robo_l2_replace_t *rep_st)
{
    uint32 l2_flags = 0, field_value=0;
    bcm_l2_addr_t	l2_search;
    l2_arl_sw_entry_t		arl_entry, arl_result, arl_search;
    int rv = BCM_E_NONE, i, vlan_count = 0, t_vid = 0;
    
    l2_flags = 0;

    if (rep_st->isDel){
        if (rep_st->isStatic) {
            l2_flags = BCM_L2_DELETE_STATIC;
        }
        return bcm_l2_addr_delete_by_mac(unit, rep_st->match_mac, l2_flags);
    } 

    /* Get the current valid VLAN entry count */
    rv = DRV_DEV_PROP_GET(unit, DRV_DEV_PROP_VLAN_ENTRY_NUM, 
            (uint32 *) &vlan_count);

    for (i = 1; i < vlan_count; i++) {
        t_vid = i;

        bcm_l2_addr_init(&l2_search, rep_st->match_mac, t_vid);

        BCM_IF_ERROR_RETURN(_bcm_robo_l2_to_arl(unit, &arl_entry, &l2_search));
        l2_flags =  DRV_MEM_OP_BY_HASH_BY_MAC|DRV_MEM_OP_BY_HASH_BY_VLANID;
        rv = DRV_MEM_SEARCH(unit, DRV_MEM_ARL, (uint32 *)&arl_entry,
                    (uint32 *)&arl_result, NULL, l2_flags);                    

        if (rv == BCM_E_EXISTS) {
            if (rep_st->isStatic){
                l2_flags |= DRV_MEM_OP_DELETE_BY_STATIC;
            }

            /* get the port field */
            BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                    DRV_MEM_FIELD_SRC_PORT, (uint32 *)&arl_result, 
                    &field_value));

            if (field_value == rep_st->new_port) {
                continue;
            }
                
            /* delete this l2 entry */
            rv = DRV_MEM_DELETE(unit, DRV_MEM_ARL, 
                    (uint32 *)&arl_search, l2_flags);
            soc_cm_debug(DK_ARL, 
                    "_bcm_robo_l2_addr_replace_by_mac(): mem_delete()=%d\n",rv);
            soc_cm_debug(DK_ARL, 
                "ARL_DEL: MAC=%02x:%02x:%02x:%02x:%02x:%02x, "
                "VID=0x%x\n",
                rep_st->match_mac[0], rep_st->match_mac[1], rep_st->match_mac[2], 
                rep_st->match_mac[3], rep_st->match_mac[4], rep_st->match_mac[5],
                t_vid);
        
            l2_flags = DRV_MEM_OP_BY_HASH_BY_MAC | 
                    DRV_MEM_OP_BY_HASH_BY_VLANID | DRV_MEM_OP_REPLACE;
    
            /* set the port field */
            field_value = rep_st->new_port;
            BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_SET(unit, DRV_MEM_ARL, 
                    DRV_MEM_FIELD_SRC_PORT, (uint32 *)&arl_result, 
                    &field_value));
            BCM_IF_ERROR_RETURN(DRV_MEM_INSERT(unit, DRV_MEM_ARL, 
                    (uint32 *)&arl_search,l2_flags));

        } else if (rv == BCM_E_NOT_FOUND || rv == BCM_E_FULL){
            continue;
        } else if(rv != 0){
            soc_cm_debug(DK_WARN, 
                    "Warnning: _bcm_robo_l2_addr_replace_by_mac(),\n\t >>%s\n", 
                    bcm_errmsg(rv));
        }
    }
    
    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_robo_l2_addr_replace_by_mac_vlan
 * Description:
 *     Helper function to bcm_l2_replace API to replace l2 entries by mac
 * Parameters:
 *     unit         device number
 *     flags        flags BCM_L2_REPLACE_* 
 *     rep_st       structure with information of what to replace 
 * Return:
 *     BCM_E_XXX
 */
int 
_bcm_robo_l2_addr_replace_by_mac_vlan(int unit, 
        _bcm_robo_l2_replace_t *rep_st) {

    uint32 action_flag = 0, port = 0, temp = 0;
    int rv = BCM_E_NONE;
    bcm_l2_addr_t	 l2addr;
    bcm_trunk_add_info_t t_add_info;
    l2_arl_sw_entry_t   arl_search, arl_result;
    bcm_trunk_t     tid;
    
    bcm_l2_addr_init(&l2addr, rep_st->match_mac, rep_st->match_vid);

    /* look up the l2 entry with mac+vid */   
    BCM_IF_ERROR_RETURN(_bcm_robo_l2_to_arl(unit, &arl_search, &l2addr));
    
    action_flag = DRV_MEM_OP_BY_HASH_BY_MAC | 
                 DRV_MEM_OP_BY_HASH_BY_VLANID;
        
    rv = DRV_MEM_SEARCH(unit, DRV_MEM_ARL, (uint32 *)&arl_search,
            (uint32 *)&arl_result, NULL, action_flag);
    
    if (rv != BCM_E_EXISTS) {
        return BCM_E_NOT_FOUND;
    }

    if (rep_st->isStatic) {
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                DRV_MEM_FIELD_STATIC,(uint32 *)&arl_result, &temp));
        if (!temp) {
            return BCM_E_NOT_FOUND;
        }
    }

    if (rep_st->isPending) {
#ifdef  BCM_TB_SUPPORT
        if (SOC_IS_TBX(unit)){
            BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                    DRV_MEM_FIELD_VALID, (uint32 *)&arl_result, &temp));
            if (temp != _TB_ARL_STATUS_PENDING){
                return BCM_E_NOT_FOUND;
            }
        }
#endif  /* BCM_TB_SUPPORT */
    }

    if (rep_st->isTrunk) {
        /* get the port field */
        BCM_IF_ERROR_RETURN(DRV_MEM_FIELD_GET(unit, DRV_MEM_ARL, 
                DRV_MEM_FIELD_SRC_PORT, (uint32 *)&arl_result, &port));
                            
        bcm_trunk_find(unit, 0, port, &tid);
        /* check the port existed in the match_trunk ? */
        if (tid != rep_st->match_trunk) {
            return BCM_E_PARAM;
        }

        /* check the new_trunk valid ?*/
        if (rep_st->new_trunk== BCM_TRUNK_INVALID) {
        	return BCM_E_PARAM;
        }    
        rv = bcm_trunk_get(unit, rep_st->new_trunk, &t_add_info);
    
        if (rv < 0){
            return BCM_E_PARAM;
        }
    
        /* 
         * If no any port assigned to the current trunk,
         * there will be no further processes.
         * Since no tgid information recorded in the arl entry of robo chip.
         */
        if (!t_add_info.num_ports) {
            return BCM_E_UNAVAIL;
        }
        /* pick any port in the new_trunk */
        l2addr.port = t_add_info.tp[0];
    }else {

        if (rep_st->isDel){            
            return bcm_l2_addr_delete(unit, 
                    rep_st->match_mac, rep_st->match_vid);
        } 

        l2addr.port = rep_st->new_port;
    }
    bcm_l2_addr_add(unit, &l2addr);

    return BCM_E_NONE;
}

/*
 * Function:
 *     bcm_robo_l2_replace
 * Description:
 *     To replace destination (or delete) multiple L2 entries
 * Parameters:
 *     unit         device number
 *     flags        flags BCM_L2_REPLACE_* 
 *     match_addr   L2 parameters to match on delete/replace
 *     new_module   new module ID for a replace 
 *     new_port     new port for a replace
 *     new_trunk    new trunk for a replace  
 * Return:
 *     BCM_E_XXX
 */
int 
bcm_robo_l2_replace(int unit, uint32 flags, bcm_l2_addr_t *match_addr,
                   bcm_module_t new_module, bcm_port_t new_port, 
                   bcm_trunk_t new_trunk)
{
    uint32              cmp_flags = 0, l2_flags = 0;
    int                 rv = BCM_E_UNAVAIL, freezing = BCM_E_NONE;
    _bcm_robo_l2_replace_t   rep_st;

    L2_INIT(unit);

    /* SDK-33848 : Follow the desig change on match_addr = NULL */
    /* API Guide : Using the BCM_L2_REPLACE_DELETE flag and matching_addr as 
     *      NULL will delete all addresses from the L2 table and parameters 
     *      new_module, new_port and new_trunk will be ignored. 
     */
    if (NULL == match_addr) {
        if (!(flags & BCM_L2_REPLACE_DELETE)) {
            return BCM_E_PARAM;
        }
        flags &= ~(BCM_L2_REPLACE_MATCH_VLAN | BCM_L2_REPLACE_MATCH_MAC |
                   BCM_L2_REPLACE_MATCH_DEST);

        /* performing delete all process 
         *  - call DRV_MEM_CLEAR() to remove all existed ARL entries.
         */
        freezing = bcm_robo_l2_addr_freeze(unit);
        if (freezing == BCM_E_UNAVAIL || freezing == BCM_E_NONE){
            rv = DRV_MEM_CLEAR(unit, DRV_MEM_ARL);
            if (freezing == BCM_E_NONE){
                BCM_IF_ERROR_RETURN(bcm_robo_l2_addr_thaw(unit));
            }
        } else {
            rv = freezing;
        }
        return rv;
    }

    BCM_IF_ERROR_RETURN(_bcm_robo_l2_replace_flags_validate(flags));
    sal_memset(&rep_st, 0, sizeof(_bcm_robo_l2_replace_t));

    if (0 == (flags & BCM_L2_REPLACE_DELETE)) {
        if (BCM_GPORT_IS_SET(new_port)) {
            bcm_port_t  tmp_port;
            int         tmp_id;

            BCM_IF_ERROR_RETURN(
                _bcm_robo_gport_resolve(unit, new_port, &new_module, &tmp_port,
                                       &new_trunk, &tmp_id));
            new_port = tmp_port;
        } 
        if (flags & BCM_L2_REPLACE_NEW_TRUNK) { 

            /*   All current ROBO devices have no feature of TRUNK base L2 
             * learning. Thus there will be no trunk_identifier in L2 entry on
             * ROBO devices.
             */
            return BCM_E_UNAVAIL;
        } else {
            if (new_module != 0) {
                return BCM_E_BADID;
            }
            if (!SOC_PORT_VALID(unit, new_port)) {
                return BCM_E_PORT;
            }
        }
        rep_st.new_module = new_module;
        rep_st.new_port = new_port;
    } else {
        rep_st.isDel = 1;
    }
    cmp_flags = flags &  ( BCM_L2_REPLACE_MATCH_MAC | 
                                        BCM_L2_REPLACE_MATCH_VLAN |
                                        BCM_L2_REPLACE_MATCH_DEST );

#ifdef BCM_TB_SUPPORT
    if (SOC_IS_TBX(unit)){
        if ((flags & BCM_L2_REPLACE_MATCH_STATIC) || 
                (flags & _BCM_TB_L2_REPLACE_MATCH_STATIC)) {
            rep_st.isStatic |= _BCM_TB_L2_REPLACE_MATCH_STATIC;
        }
        
        if (flags & _BCM_TB_L2_REPLACE_MATCH_STATIC_ONLY){
            rep_st.isStatic |= _BCM_TB_L2_REPLACE_MATCH_STATIC_ONLY;
        }
        
        if (flags & _BCM_TB_L2_REPLACE_MATCH_MCAST){
            rep_st.isMcast |= _BCM_TB_L2_REPLACE_MATCH_MCAST;
        }

        if (flags & _BCM_TB_L2_REPLACE_MATCH_MCAST_ONLY){
            rep_st.isMcast |= _BCM_TB_L2_REPLACE_MATCH_MCAST_ONLY;
        }
    } else {
        if (flags & BCM_L2_REPLACE_MATCH_STATIC) {
            rep_st.isStatic = 1;
        }
    }
#else   /* BCM_TB_SUPPORT */
    if (flags & BCM_L2_REPLACE_MATCH_STATIC) {
        rep_st.isStatic = 1;
    }
#endif  /* BCM_TB_SUPPORT */


    if (flags & BCM_L2_REPLACE_PENDING) {
        rep_st.isPending = 1;
    }

    switch (cmp_flags) {
        case BCM_L2_REPLACE_MATCH_MAC: 
        {        
            if (rep_st.isDel) {
                if (rep_st.isStatic) {
                    l2_flags = BCM_L2_DELETE_STATIC;
                }
                if (rep_st.isPending) {
                    l2_flags |= BCM_L2_DELETE_PENDING;
                }
                rv = bcm_l2_addr_delete_by_mac(unit, 
                        match_addr->mac, l2_flags);
            } else {
                rv = BCM_E_UNAVAIL;
            }
            break;
        }
        case BCM_L2_REPLACE_MATCH_VLAN: 
        {
            if (rep_st.isDel) {
                if (rep_st.isStatic) {
                    if (SOC_IS_TBX(unit)){
                        /* use ROBO bcm internal define to indicate more 
                         *  detail action on supporting TB feature.
                         */
                        l2_flags |= rep_st.isStatic;
                    } else {
                        l2_flags = BCM_L2_DELETE_STATIC;
                    }
                }
                
                if (rep_st.isPending) {
                    l2_flags |= BCM_L2_DELETE_PENDING;
                }
                
                if (rep_st.isMcast){
                    if (SOC_IS_TBX(unit)){
                        /* use ROBO bcm internal define to indicate more 
                         *  detail action on supporting TB feature.
                         */
                        l2_flags |= rep_st.isMcast;
                    }
                }
                rv = bcm_l2_addr_delete_by_vlan(unit, 
                        match_addr->vid, l2_flags);
            } else {
                rv = BCM_E_UNAVAIL;
            }
            break;
        }
        case (BCM_L2_REPLACE_MATCH_MAC | BCM_L2_REPLACE_MATCH_VLAN):
        {
            sal_memcpy(&(rep_st.match_mac), match_addr->mac, 
                    sizeof(bcm_mac_t));
            rep_st.match_vid = match_addr->vid;
            rv =_bcm_robo_l2_addr_replace_by_mac_vlan(unit, &rep_st);
            break;
        }
        case BCM_L2_REPLACE_MATCH_DEST: 
            if (rep_st.isDel) {
                if (rep_st.isStatic) {
                    if (SOC_IS_TBX(unit)){
                        /* use ROBO bcm internal define to indicate more 
                         *  detail action on supporting TB feature.
                         */
                        l2_flags |= rep_st.isStatic;
                    } else {
                        l2_flags = BCM_L2_DELETE_STATIC;
                    }
                }
                
                if (rep_st.isPending) {
                    l2_flags |= BCM_L2_DELETE_PENDING;
                }
                
                if (rep_st.isMcast){
                    if (SOC_IS_TBX(unit)){
                        /* use ROBO bcm internal define to indicate more 
                         *  detail action on supporting TB feature.
                         */
                        l2_flags |= rep_st.isMcast;
                    }
                }

                rv = bcm_robo_l2_addr_delete_by_port(unit, 
                        match_addr->modid, match_addr->port, l2_flags);
            } else {
                rv = BCM_E_UNAVAIL;
            }
            break;
        case (BCM_L2_REPLACE_MATCH_MAC |BCM_L2_REPLACE_MATCH_DEST):
            if (rep_st.isDel) {
                if (rep_st.isStatic) {
                    if (SOC_IS_TBX(unit)){
                        /* use ROBO bcm internal define to indicate more 
                         *  detail action on supporting TB feature.
                         */
                        l2_flags |= rep_st.isStatic;
                    } else {
                        l2_flags = BCM_L2_DELETE_STATIC;
                    }
                }
                
                if (rep_st.isPending) {
                    l2_flags |= BCM_L2_DELETE_PENDING;
                }
                
                if (rep_st.isMcast){
                    if (SOC_IS_TBX(unit)){
                        /* use ROBO bcm internal define to indicate more 
                         *  detail action on supporting TB feature.
                         */
                        l2_flags |= rep_st.isMcast;
                    }
                }
                rv = bcm_robo_l2_addr_delete_by_mac_port(unit, 
                        match_addr->mac, match_addr->modid, 
                        match_addr->port, l2_flags);
            } else {
                rv = BCM_E_UNAVAIL;
            }
            break;
        case (BCM_L2_REPLACE_MATCH_VLAN |
                    BCM_L2_REPLACE_MATCH_DEST ):
            if (rep_st.isDel) {
                if (rep_st.isStatic) {
                    if (SOC_IS_TBX(unit)){
                        /* use ROBO bcm internal define to indicate more 
                         *  detail action on supporting TB feature.
                         */
                        l2_flags |= rep_st.isStatic;
                    } else {
                        l2_flags = BCM_L2_DELETE_STATIC;
                    }
                }
                
                if (rep_st.isPending) {
                    l2_flags |= BCM_L2_DELETE_PENDING;
                }
                
                if (rep_st.isMcast){
                    if (SOC_IS_TBX(unit)){
                        /* use ROBO bcm internal define to indicate more 
                         *  detail action on supporting TB feature.
                         */
                        l2_flags |= rep_st.isMcast;
                    }
                }
                rv = bcm_robo_l2_addr_delete_by_vlan_port(unit, 
                        match_addr->vid, match_addr->modid, 
                        match_addr->port, l2_flags);
            } else {
                rv = BCM_E_UNAVAIL;
            }
            break;
        case (BCM_L2_REPLACE_MATCH_MAC |BCM_L2_REPLACE_MATCH_VLAN |
                    BCM_L2_REPLACE_MATCH_DEST ):
        {
            rv = BCM_E_UNAVAIL;
            break;
        }
        default:
            break;

    }

    return rv;
}