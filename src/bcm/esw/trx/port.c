/*
 * $Id: port.c 1.33.8.2 Broadcom SDK $
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
 * File:        port.c
 * Purpose:     Port function implementations
 */


#include <soc/defs.h>

#include <assert.h>

#include <sal/core/libc.h>
#if defined(BCM_TRX_SUPPORT)

#include <shared/util.h>
#include <soc/mem.h>
#include <soc/cm.h>
#include <soc/drv.h>
#include <soc/register.h>
#include <soc/memory.h>
#include <soc/lpm.h>
#include <soc/tnl_term.h>

#include <bcm/port.h>
#include <bcm/error.h>

#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/firebolt.h>
#include <bcm_int/esw/trx.h>
#include <bcm_int/esw/triumph.h>
#include <bcm_int/esw/port.h>
#include <bcm_int/esw/xgs3.h>
#include <bcm_int/esw/stack.h>

#include <bcm_int/esw_dispatch.h>

/*
 * Function:
 *      _bcm_trx_port_cml_hw2flags
 * Description:
 *      Converts a HW value to CML flags 
 * Parameters:
 *      unit         - (IN) Device number
 *      val          - (IN) TRX HW value 
 *      flags        - (OUT) CML flags BCM_PORT_LEARN_*
 * Return Value:
 *      BCM_E_XXX
 */
int _bcm_trx_port_cml_hw2flags(int unit, uint32 val, uint32 *flags)
{
    uint32  lflags = 0;

    if (NULL == flags) {
        return BCM_E_PARAM;
    }

    if (!(val & (1 << 0))) {
       lflags |= BCM_PORT_LEARN_FWD;
    }
    if (val & (1 << 1)) {
       lflags |= BCM_PORT_LEARN_CPU;
    }
    if (val & (1 << 2)) {
       lflags |= BCM_PORT_LEARN_PENDING;
    }
    if (val & (1 << 3)) {
       lflags |= BCM_PORT_LEARN_ARL;
    }

    *flags = lflags;
    return BCM_E_NONE;
}


/*
 * Function:
 *      _bcm_trx_port_cml_flags2hw
 * Description:
 *      Converts CML flags to a HW value.
 * Parameters:
 *      unit         - (IN) Device number
 *      flags        - (IN) CML flags BCM_PORT_LEARN_*
 *      val          - (OUT) TRX HW value
 * Return Value:
 *      BCM_E_XXX
 */
int _bcm_trx_port_cml_flags2hw(int unit, uint32 flags, uint32 *val)
{
    uint32 hw_val = 0;

    if (NULL == val) {
        return BCM_E_PARAM;
    }
    if (!(flags & BCM_PORT_LEARN_FWD)) {
       hw_val |= (1 << 0);
    }
    if (flags & BCM_PORT_LEARN_CPU) {
       hw_val |= (1 << 1);
    }
    if (flags & BCM_PORT_LEARN_PENDING) {
       hw_val |= (1 << 2);
    }
    if (flags & BCM_PORT_LEARN_ARL) {
       hw_val |= (1 << 3);
    }

    if (hw_val & ~0xf) {
        return (BCM_E_PARAM);
    }

    *val = hw_val;
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_trx_vp_tpid_add
 * Description:
 *      Add allowed TPID for a virtual port.
 * Parameters:
 *      unit         - (IN) Device number
 *      vport        - (IN) Virtual Port identifier (MPLS ot MIM)
 *      tpid         - (IN) Tag Protocol ID
 *      color_select - (IN) Color mode for TPID
 * Return Value:
 *      BCM_E_XXX
 */
int _bcm_trx_vp_tpid_add(int unit, bcm_gport_t vport, uint16 tpid, int color_select)
{
    int                 vp, rv, tpid_idx=0, tpid_enable, islocal;
    source_vp_entry_t   svp;
    uint32              ena_f, cfi_cng, egr_vlan_ctrl;
    bcm_port_t          port;
    bcm_module_t        modid, mymodid;
    bcm_trunk_t         tgid;


    BCM_IF_ERROR_RETURN(
        _bcm_esw_gport_resolve(unit, vport, &modid, &port, &tgid, &vp));

    if ((-1 == vp) || (tgid != -1) ){ 
        return BCM_E_PORT;
    }
    BCM_IF_ERROR_RETURN( _bcm_esw_modid_is_local(unit, modid, &islocal));
    if (!islocal) {
        return BCM_E_PORT;
    }
    BCM_IF_ERROR_RETURN(bcm_esw_stk_my_modid_get(unit, &mymodid));
    while (mymodid < modid) {
        port += 32;
        modid -= 1;
    }

    _bcm_fb2_outer_tpid_tab_lock(unit);

    rv = READ_SOURCE_VPm(unit, MEM_BLOCK_ANY, vp, &svp);
    if (BCM_FAILURE(rv)) {
        _bcm_fb2_outer_tpid_tab_unlock(unit);
        return rv;
    }
    
    /* See if the entry already exist */
    rv = _bcm_fb2_outer_tpid_lkup(unit, tpid, &tpid_idx);
    ena_f = soc_SOURCE_VPm_field32_get(unit, &svp, TPID_ENABLEf);

    if ((BCM_E_NOT_FOUND == rv) || !(ena_f & (1 << tpid_idx)) ) {
        rv = _bcm_fb2_outer_tpid_entry_add(unit, tpid, &tpid_idx);
        if (BCM_FAILURE(rv)) {
            _bcm_fb2_outer_tpid_tab_unlock(unit);
            return rv;
        }

    }
    tpid_enable = (1 << tpid_idx);
    soc_SOURCE_VPm_field32_set(unit, &svp, SD_TAG_MODEf, 1);
    ena_f |= tpid_enable;
    soc_SOURCE_VPm_field32_set(unit, &svp, TPID_ENABLEf, ena_f);

    rv = WRITE_SOURCE_VPm(unit, MEM_BLOCK_ALL, vp, &svp);

    if (BCM_FAILURE(rv)) {
        rv = _bcm_fb2_outer_tpid_entry_delete(unit,tpid_idx);
        _bcm_fb2_outer_tpid_tab_unlock(unit);
        return rv;
    }

    rv = READ_EGR_VLAN_CONTROL_1r(unit, port, &egr_vlan_ctrl);
    if (BCM_FAILURE(rv)) {
        rv = _bcm_fb2_outer_tpid_entry_delete(unit,tpid_idx);
        _bcm_fb2_outer_tpid_tab_unlock(unit);
        return rv;
    }

    /* Update color selection per TPID per port */
    cfi_cng = soc_reg_field_get(unit, EGR_VLAN_CONTROL_1r,
                                egr_vlan_ctrl, CFI_AS_CNGf);
    switch (color_select) {
      case BCM_COLOR_PRIORITY:
          cfi_cng &= ~tpid_enable;
          break;
      case BCM_COLOR_OUTER_CFI:
      case BCM_COLOR_INNER_CFI:
          cfi_cng |= tpid_enable;
          break;
      default:
          /* Already checked color_select param */
          /* Should never get here              */ 
          break;
    }

    soc_reg_field_set(unit, EGR_VLAN_CONTROL_1r, &egr_vlan_ctrl,
                      CFI_AS_CNGf, cfi_cng);
    rv = WRITE_EGR_VLAN_CONTROL_1r(unit, port, egr_vlan_ctrl);
    if (BCM_FAILURE(rv)) {
        rv = _bcm_fb2_outer_tpid_entry_delete(unit,tpid_idx);
        _bcm_fb2_outer_tpid_tab_unlock(unit);
        return rv;
    }

    _bcm_fb2_outer_tpid_tab_unlock(unit);
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_trx_vp_tpid_delete
 * Description:
 *      Delete allowed TPID for a virtual port.
 * Parameters:
 *      unit  - (IN) Device number
 *      vport - (IN) Virtual Port identifier (WLAN or MIM)
 *      tpid  - (IN) Tag Protocol ID
 * Return Value:
 *      BCM_E_XXX
 */
int _bcm_trx_vp_tpid_delete(int unit, bcm_gport_t vport, uint16 tpid)
{
    int                 vp, rv, tpid_idx;
    source_vp_entry_t   svp;
    uint32              ena_f;

    vp = -1;
    if (BCM_GPORT_IS_MPLS_PORT(vport)) {
        vp = BCM_GPORT_MPLS_PORT_ID_GET(vport);
    } 
    if (BCM_GPORT_IS_MIM_PORT(vport)) {
        vp = BCM_GPORT_MIM_PORT_ID_GET(vport);
    }
    if (-1 == vp) { 
        return BCM_E_PORT;
    }

    BCM_IF_ERROR_RETURN(READ_SOURCE_VPm(unit, MEM_BLOCK_ANY, vp, &svp));
    ena_f = soc_SOURCE_VPm_field32_get(unit, &svp, TPID_ENABLEf);

    _bcm_fb2_outer_tpid_tab_lock(unit);

    rv = _bcm_fb2_outer_tpid_lkup(unit, tpid, &tpid_idx);
    if (BCM_FAILURE(rv)) {
        _bcm_fb2_outer_tpid_tab_unlock(unit);
        return (rv);
    } 

    if (ena_f & (1 << tpid_idx)) {
        ena_f &= ~(1 << tpid_idx);
        soc_SOURCE_VPm_field32_set(unit, &svp, TPID_ENABLEf, ena_f);
        rv = WRITE_SOURCE_VPm(unit, MEM_BLOCK_ALL, vp, &svp);

        if (BCM_FAILURE(rv)) {
            rv = _bcm_fb2_outer_tpid_entry_delete(unit,tpid_idx);
            _bcm_fb2_outer_tpid_tab_unlock(unit);
            return rv;
        }
    } else {
        _bcm_fb2_outer_tpid_tab_unlock(unit);
        return BCM_E_NOT_FOUND;
    }

    rv = _bcm_fb2_outer_tpid_entry_delete(unit, tpid_idx);
    _bcm_fb2_outer_tpid_tab_unlock(unit);
    return rv;
}

/*
 * Function:
 *      _bcm_trx_vp_tpid_delete_all
 * Description:
 *      Delete all allowed TPID for a virtual port.
 * Parameters:
 *      unit  - (IN) Device number
 *      vport - (IN) Virtual Port identifier
 * Return Value:
 *      BCM_E_XXX
 */
int _bcm_trx_vp_tpid_delete_all(int unit, bcm_gport_t vport)
{
    int                 vp, rv, tpid_idx;
    source_vp_entry_t   svp;
    uint32              ena_f;

    vp = -1;
    if (BCM_GPORT_IS_MPLS_PORT(vport)) {
        vp = BCM_GPORT_MPLS_PORT_ID_GET(vport);
    } 
    if (BCM_GPORT_IS_MIM_PORT(vport)) {
        vp = BCM_GPORT_MIM_PORT_ID_GET(vport);
    }
    if (-1 == vp) { 
        return BCM_E_PORT;
    }

    BCM_IF_ERROR_RETURN(READ_SOURCE_VPm(unit, MEM_BLOCK_ANY, vp, &svp));
    ena_f = soc_SOURCE_VPm_field32_get(unit, &svp, TPID_ENABLEf);
    /* If was not set - do nothing */
    if (!ena_f) {
        return BCM_E_NONE;
    }
    soc_SOURCE_VPm_field32_set(unit, &svp, SD_TAG_MODEf, 0);
    soc_SOURCE_VPm_field32_set(unit, &svp, TPID_ENABLEf, 0);
    BCM_IF_ERROR_RETURN(WRITE_SOURCE_VPm(unit, MEM_BLOCK_ALL, vp, &svp));

    /* Perform SW delition only after HW was updated */
    _bcm_fb2_outer_tpid_tab_lock(unit);
    tpid_idx = 0;
    while (ena_f) {
        if (ena_f & 1) {
            rv = _bcm_fb2_outer_tpid_entry_delete(unit, tpid_idx);
            if (BCM_FAILURE(rv)) {
                _bcm_fb2_outer_tpid_tab_unlock(unit);
                return (rv);
            }
        }
        ena_f = (ena_f >> 1);
        tpid_idx++;
    }
    _bcm_fb2_outer_tpid_tab_unlock(unit);
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_trx_vp_tpid_set
 * Description:
 *      Set the default Tag Protocol ID for a virtual gport.
 * Parameters:
 *      unit  - Device number
 *      vport - Virtual Port identifier
 *      tpid  - Tag Protocol ID
 * Return Value:
 *      BCM_E_XXX
 */
int _bcm_trx_vp_tpid_set(int unit, bcm_gport_t vport, uint16 tpid)
{
    int                 vp, rv, tpid_idx, old_idx, islocal;
    source_vp_entry_t   svp;
    uint32              ena_f, evc;
    bcm_port_t          port;
    bcm_module_t        modid, mymodid;
    bcm_trunk_t         tgid;
    
    BCM_IF_ERROR_RETURN(
        _bcm_esw_gport_resolve(unit, vport, &modid, &port, &tgid, &vp));

    if ((-1 == vp) || (tgid != -1) ){ 
        return BCM_E_PORT;
    }
    BCM_IF_ERROR_RETURN( _bcm_esw_modid_is_local(unit, modid, &islocal));
    if (!islocal) {
        return BCM_E_PORT;
    }
    BCM_IF_ERROR_RETURN(bcm_esw_stk_my_modid_get(unit, &mymodid));
    while (mymodid < modid) {
        port += 32;
        modid -= 1;
    }
    BCM_IF_ERROR_RETURN(READ_EGR_VLAN_CONTROL_1r(unit, port, &evc));
    old_idx =  soc_reg_field_get(unit, EGR_VLAN_CONTROL_1r, 
                                 evc, OUTER_TPID_INDEXf);
    BCM_IF_ERROR_RETURN(READ_SOURCE_VPm(unit, MEM_BLOCK_ANY, vp, &svp));
    ena_f = soc_SOURCE_VPm_field32_get(unit, &svp, TPID_ENABLEf);

    _bcm_fb2_outer_tpid_tab_lock(unit);
    /* Delete the TPID referenced by EGR_VLAN_CONTROL_1r */
    rv = _bcm_fb2_outer_tpid_entry_delete(unit, old_idx);
    if (BCM_FAILURE(rv)) {
        _bcm_fb2_outer_tpid_tab_unlock(unit);
        return (rv);
    }
    /* Delete the TPID referenced by PORT and EGR_SRC_PORT
     * tables. We keep only one reference for both table because
     * the outer TPID enabled will be the same.
     */

    tpid_idx = 0;
    while (ena_f) {
        if (ena_f & 1) {
            rv = _bcm_fb2_outer_tpid_entry_delete(unit, tpid_idx);
            if (BCM_FAILURE(rv)) {
                _bcm_fb2_outer_tpid_tab_unlock(unit);
                return (rv);
            }
        }
        ena_f = ena_f >> 1;
        tpid_idx++;
    }
        /* Add TPID reference for EGR_VLAN_CONTROL_1r. */
    rv = _bcm_fb2_outer_tpid_entry_add(unit, tpid, &tpid_idx);
    if (BCM_FAILURE(rv)) {
        _bcm_fb2_outer_tpid_tab_unlock(unit);
        return (rv);
    }

    /* Add TPID reference for EGR_SRC_PORT and PORT tables. 
     * The first insertion and second insertion will return
     * the same status because we are adding the same TPID value.
     */
    rv = _bcm_fb2_outer_tpid_entry_add(unit, tpid, &tpid_idx);
    if (BCM_FAILURE(rv)) {
        _bcm_fb2_outer_tpid_entry_delete(unit, tpid_idx);
        _bcm_fb2_outer_tpid_tab_unlock(unit);
        return (rv);
    }

    soc_reg_field_set(unit, EGR_VLAN_CONTROL_1r, &evc,
                      OUTER_TPID_INDEXf, tpid_idx);
    rv = WRITE_EGR_VLAN_CONTROL_1r(unit, port, evc);
    if (BCM_FAILURE(rv)) {
        _bcm_fb2_outer_tpid_entry_delete(unit, tpid_idx);
        _bcm_fb2_outer_tpid_entry_delete(unit, tpid_idx);
        _bcm_fb2_outer_tpid_tab_unlock(unit);
        return (rv);
    }
    ena_f = 1 << tpid_idx;
    soc_SOURCE_VPm_field32_set(unit, &svp, TPID_ENABLEf, ena_f);
    
    rv = WRITE_SOURCE_VPm(unit, MEM_BLOCK_ANY, vp, &svp); 
    if (BCM_FAILURE(rv)) {
        _bcm_fb2_outer_tpid_entry_delete(unit, tpid_idx);
        _bcm_fb2_outer_tpid_entry_delete(unit, tpid_idx);
        _bcm_fb2_outer_tpid_tab_unlock(unit);
        return (rv);
    }
    
    _bcm_fb2_outer_tpid_tab_unlock(unit);
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_trx_vp_tpid_get
 * Description:
 *      Get the default Tag Protocol ID for a virtual port.
 * Parameters:
 *      unit  - Device number
 *      vport - Virtual Port identifier
 *      tpid  - Tag Protocol ID
 * Return Value:
 *      BCM_E_XXX
 */
int _bcm_trx_vp_tpid_get(int unit, bcm_gport_t vport, uint16 *tpid)
{
    int                 tpid_idx, vp, islocal;
    uint32              evc;
    bcm_port_t          port;
    bcm_module_t        modid, mymodid;
    bcm_trunk_t         tgid;
    

    BCM_IF_ERROR_RETURN(
        _bcm_esw_gport_resolve(unit, vport, &modid, &port, &tgid, &vp));

    if ((-1 == vp) || (tgid != -1) ){ 
        return BCM_E_PORT;
    }
    BCM_IF_ERROR_RETURN( _bcm_esw_modid_is_local(unit, modid, &islocal));
    if (!islocal) {
        return BCM_E_PORT;
    }
    BCM_IF_ERROR_RETURN(bcm_esw_stk_my_modid_get(unit, &mymodid));
    while (mymodid < modid) {
        port += 32;
        modid -= 1;
    }
    SOC_IF_ERROR_RETURN(READ_EGR_VLAN_CONTROL_1r(unit, port, &evc));
    tpid_idx = soc_reg_field_get(unit, EGR_VLAN_CONTROL_1r,
                              evc, OUTER_TPID_INDEXf);
    _bcm_fb2_outer_tpid_entry_get(unit, tpid, tpid_idx);
    return (BCM_E_NONE);
}



/*
 * Function:
 *      _bcm_trx_egr_src_port_outer_tpid_set
 * Description:
 *      Enable/Disable  outer tpid on all egress ports 
 * Parameters:
 *      unit - Device number
 *      tpid_index - Tag Protocol ID
 * Return Value:
 *      BCM_E_XXX
 */
int
_bcm_trx_egr_src_port_outer_tpid_set(int unit, int tpid_index, int enable)
{
    uint32      egr_src_port;
    uint32      tpid_enable;
    uint32      new_tpid_enable;
    bcm_port_t  port;
    soc_reg_t   reg;

    if (SOC_IS_TD_TT(unit) || SOC_IS_HURRICANE(unit) || SOC_IS_KATANA(unit)) {
        reg = EGR_PORT_1r;
    } else {
        reg = EGR_SRC_PORTr;
    }

    PBMP_E_ITER(unit, port) {
        BCM_IF_ERROR_RETURN(soc_reg32_get(unit, reg, port, 0, &egr_src_port));
        tpid_enable = soc_reg_field_get(unit, reg,
                                        egr_src_port, OUTER_TPID_ENABLEf);
        if (enable) {
            new_tpid_enable = (tpid_enable | (1 << tpid_index));
        } else {
            new_tpid_enable = (tpid_enable & ~(1 << tpid_index));
        } 

        if (new_tpid_enable != tpid_enable) {
            soc_reg_field_set(unit, reg, &egr_src_port,
                              OUTER_TPID_ENABLEf, new_tpid_enable);

            BCM_IF_ERROR_RETURN(soc_reg32_set(unit, reg, port, 0, egr_src_port));
        }
    } 
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_esw_source_trunk_map_set
 * Description:
 *      Helper funtion to modify fields in SOURCE_TRUNK_MAP memory.
 * Parameters:
 *      unit  - Device number
 *      port  - Port number
 *      field - Field name within SOURCE_TRUNK_MAP table entry
 *      value - new field value
 * Return Value:
 *      BCM_E_XXX 
 */
int
_bcm_trx_source_trunk_map_set(int unit, bcm_port_t port, 
                              soc_field_t field, uint32 value)
{
    bcm_module_t my_modid;
    int index;

    /* Port sanity check. */
    if (!SOC_PORT_ADDRESSABLE(unit, port)) {
        return (BCM_E_PORT);
    }

    /* Get local module id. */
    BCM_IF_ERROR_RETURN(bcm_esw_stk_my_modid_get(unit, &my_modid));

    /* Calculate table index. */
    BCM_IF_ERROR_RETURN(_bcm_esw_src_mod_port_table_index_get(unit, 
        my_modid, port, &index));

    return soc_mem_field32_modify(unit, SOURCE_TRUNK_MAP_TABLEm, index, field, value);
}

/*
 * Function:
 *      _bcm_esw_source_trunk_map_get
 * Description:
 *      Helper funtion to get fields in SOURCE_TRUNK_MAP memory.
 * Parameters:
 *      unit  - Device number
 *      port  - Port number
 *      field - Field name within SOURCE_TRUNK_MAP table entry
 *      value - New field value
 * Return Value:
 *      BCM_E_XXX 
 */
int
_bcm_trx_source_trunk_map_get(int unit, bcm_port_t port, 
                              soc_field_t field, uint32 *value)
{
    uint32 buf[SOC_MAX_MEM_FIELD_WORDS];
    bcm_module_t my_modid;
    int index;

    /* Input parameters check */
    if (NULL == value) {
        return (BCM_E_PARAM);
    }

    /* Memory field is valid check. */ 
    if (!SOC_MEM_FIELD_VALID(unit, SOURCE_TRUNK_MAP_TABLEm, field)) {
        return (BCM_E_UNAVAIL);
    }

    /* Port sanity check. */
    if (!SOC_PORT_ADDRESSABLE(unit, port)) {
        return (BCM_E_PORT);
    }

    /* Get local module id. */
    BCM_IF_ERROR_RETURN(bcm_esw_stk_my_modid_get(unit, &my_modid));

    /* Get table index. */
    BCM_IF_ERROR_RETURN(_bcm_esw_src_mod_port_table_index_get(unit,
        my_modid, port, &index));

    if ((index > soc_mem_index_max(unit, SOURCE_TRUNK_MAP_TABLEm)) ||
        (index < soc_mem_index_min(unit, SOURCE_TRUNK_MAP_TABLEm))) {
        return (BCM_E_INTERNAL);
    }

    /* Read table entry. */
    BCM_IF_ERROR_RETURN
        (soc_mem_read(unit, SOURCE_TRUNK_MAP_TABLEm, MEM_BLOCK_ANY, index, buf));

    /* Read requested field value. */
    *value = soc_mem_field32_get(unit, SOURCE_TRUNK_MAP_TABLEm, buf, field);

    return (BCM_E_NONE);
}

/*
 * Function:
 *     _bcm_trx_port_force_vlan_set
 * Description:
 *     To set the force vlan attribute of a port
 * Parameters:
 *     unit        device number
 *     port        port number
 *     vlan        vlan identifier
 *                 (0 - 4095) - use this VLAN id if egress packet is tagged
 *     pkt_prio    egress packet priority (-1, 0..7)
 *                 any negative priority value disable the priority
 *                 override if the egress packet is tagged
 *     flags       bit fields
 *                 BCM_PORT_FORCE_VLAN_ENABLE - enable force vlan on this
 *                                              port
 *                 BCM_PORT_FORCE_VLAN_UNTAG - egress untagged when force
 *                                             vlan is enabled on this port
 *
 * Return:
 *     BCM_E_XXX
 */
int
_bcm_trx_port_force_vlan_set(int unit, bcm_port_t port, bcm_vlan_t vlan,
                            int pkt_prio, uint32 flags)
{
    bcm_port_cfg_t pcfg;
    uint32 reg_val;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    if (!BCM_VLAN_VALID(vlan)) {
        return BCM_E_PARAM;
    }

    if (pkt_prio > 7) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(mbcm_driver[unit]->mbcm_port_cfg_get(unit, port,
                                                             &pcfg));
    reg_val = 0;

    if (flags & BCM_PORT_FORCE_VLAN_ENABLE) {
        pcfg.pc_pvlan_enable = 1;
        soc_reg_field_set(unit, EGR_PVLAN_EPORT_CONTROLr, &reg_val,
                          PVLAN_ENABLEf, 1);
        if (!(flags & BCM_PORT_FORCE_VLAN_UNTAG)) {
            soc_reg_field_set(unit, EGR_PVLAN_EPORT_CONTROLr, &reg_val,
                              PVLAN_PVIDf, vlan);
            if (pkt_prio >= 0) {
                soc_reg_field_set(unit, EGR_PVLAN_EPORT_CONTROLr, &reg_val,
                                  PVLAN_PRIf, pkt_prio);
                soc_reg_field_set(unit, EGR_PVLAN_EPORT_CONTROLr, &reg_val,
                                  PVLAN_RPEf, 1);
            }
        } else {
            soc_reg_field_set(unit, EGR_PVLAN_EPORT_CONTROLr, &reg_val,
                              PVLAN_UNTAGf, 1);
        }
    } else {
        pcfg.pc_pvlan_enable = 0;
    }

    BCM_IF_ERROR_RETURN(mbcm_driver[unit]->mbcm_port_cfg_set(unit, port, &pcfg));
    SOC_IF_ERROR_RETURN(WRITE_EGR_PVLAN_EPORT_CONTROLr(unit, port, reg_val));

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_trx_port_force_vlan_get
 * Description:
 *     To get the force vlan attribute of a port
 * Parameters:
 *     unit        device number
 *     port        port number
 *     vlan        pointer to vlan identifier
 *                 (0 - 4095) - use this VLAN id if egress packet is tagged
 *                 valid only when BCM_PORT_FORCE_VLAN_ENABLE is set and
 *                 BCM_PORT_FORCE_VLAN_UNTAG is clear
 *     pkt_prio    egress packet priority (-1, 0 - 7)
 *                 valid only when BCM_PORT_FORCE_VLAN_ENABLE is set and
 *                 BCM_PORT_FORCE_VLAN_UNTAG is clear
 *     flags       bit fields
 *                 BCM_PORT_FORCE_VLAN_ENABLE - enable force vlan on this
 *                                              port
 *                 BCM_PORT_FORCE_VLAN_UNTAG - egress untagged when force
 *                                             vlan is enabled on this port
 *
 * Return:
 *     BCM_E_XXX
 */
int
_bcm_trx_port_force_vlan_get(int unit, bcm_port_t port, bcm_vlan_t *vlan,
                            int *pkt_prio, uint32 *flags)
{
    bcm_port_cfg_t pcfg;
    uint32 reg_val;

    BCM_IF_ERROR_RETURN(_bcm_esw_port_gport_validate(unit, port, &port));

    if (!vlan || !pkt_prio || !flags) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(mbcm_driver[unit]->mbcm_port_cfg_get(unit, port,
                                                             &pcfg));
    SOC_IF_ERROR_RETURN(READ_EGR_PVLAN_EPORT_CONTROLr(unit, port,
                                                      &reg_val));

    *flags = 0;
    if (pcfg.pc_pvlan_enable) {
        *flags |= BCM_PORT_FORCE_VLAN_ENABLE;
        if (!soc_reg_field_get(unit, EGR_PVLAN_EPORT_CONTROLr, reg_val,
                               PVLAN_UNTAGf)) {
            *vlan = soc_reg_field_get(unit, EGR_PVLAN_EPORT_CONTROLr,
                                      reg_val, PVLAN_PVIDf);
            if (soc_reg_field_get(unit, EGR_PVLAN_EPORT_CONTROLr, reg_val,
                                  PVLAN_RPEf)) {
                *pkt_prio = soc_reg_field_get(unit,
                                              EGR_PVLAN_EPORT_CONTROLr,
                                              reg_val, PVLAN_PRIf);
            } else {
                *pkt_prio = -1;
            }
        } else {
            *flags |= BCM_PORT_FORCE_VLAN_UNTAG;
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_trx_port_dtag_mode_set
 * Description:
 *      Set the double-tagging mode of a port.
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      mode - Double-tagging mode, one of:
 *              BCM_PORT_DTAG_MODE_NONE            No double tagging
 *              BCM_PORT_DTAG_MODE_INTERNAL        Service Provider port
 *              BCM_PORT_DTAG_MODE_EXTERNAL        Customer port
 *              BCM_PORT_DTAG_REMOVE_EXTERNAL_TAG  Remove customer tag
 *              BCM_PORT_DTAG_ADD_EXTERNAL_TAG     Add customer tag
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *      BCM_PORT_DTAG_MODE_INTERNAL is for service provider ports.
 *              A tag will be added if the packet does not already
 *              have the internal TPID (see bcm_port_tpid_set below).
 *              Internally this sets DT_MODE and clears IGNORE_TAG.
 *      BCM_PORT_DTAG_MODE_EXTERNAL is for customer ports.
 *              The service provider TPID will always be added
 *              (see bcm_port_tpid_set below).
 *              Internally this sets DT_MODE and sets IGNORE_TAG.
 */

int
_bcm_trx_port_dtag_mode_set(int unit, bcm_port_t port, int mode)
{
    bcm_vlan_action_set_t action;
    _bcm_port_info_t      *pinfo;
    int                   color_mode;
    int                   rv;

    if (!IS_HG_PORT(unit, port)) {
        /* Modify default ingress actions */
        BCM_IF_ERROR_RETURN
            (_bcm_trx_vlan_port_default_action_get(unit, port, &action));
        action.dt_outer      = bcmVlanActionNone;
        action.dt_outer_prio = bcmVlanActionReplace;
        action.dt_inner      = bcmVlanActionNone;
        action.dt_inner_prio = bcmVlanActionNone;
        action.ot_outer      = bcmVlanActionNone;
        action.ot_outer_prio = bcmVlanActionReplace;
        action.ot_inner      = bcmVlanActionNone;
        action.it_outer      = bcmVlanActionAdd;
        action.it_inner      = bcmVlanActionNone;
        action.it_inner_prio = bcmVlanActionNone;
        action.ut_outer      = bcmVlanActionAdd;
        if (mode & BCM_PORT_DTAG_ADD_EXTERNAL_TAG) {
            action.ut_inner = bcmVlanActionAdd;
        } else {
            action.ut_inner = bcmVlanActionNone;
        }
        rv = _bcm_trx_vlan_port_default_action_set(unit, port, &action);
        BCM_IF_ERROR_RETURN(rv);

        /* Modify default egress actions */
        rv = _bcm_trx_vlan_port_egress_default_action_get(unit, port, &action);
        BCM_IF_ERROR_RETURN(rv);

        action.ot_inner      = bcmVlanActionNone;
        if (mode & BCM_PORT_DTAG_REMOVE_EXTERNAL_TAG) {
            action.dt_inner = bcmVlanActionDelete;
            action.dt_inner_prio = bcmVlanActionDelete;
        } else {
            action.dt_inner = bcmVlanActionNone;
            action.dt_inner_prio = bcmVlanActionNone;
        }
        action.dt_outer      = bcmVlanActionNone;
        action.dt_outer_prio = bcmVlanActionNone;
        action.ot_outer      = bcmVlanActionNone;
        action.ot_outer_prio = bcmVlanActionNone;
        rv = _bcm_trx_vlan_port_egress_default_action_set(unit, port, &action);
        BCM_IF_ERROR_RETURN(rv);
    }

    rv = _bcm_port_info_get(unit, port, &pinfo);
    BCM_IF_ERROR_RETURN(rv);

    pinfo->dtag_mode = mode;
    mode &= BCM_PORT_DTAG_MODE_INTERNAL | BCM_PORT_DTAG_MODE_EXTERNAL;
    if (mode == BCM_PORT_DTAG_MODE_NONE) {
        /* Set the default outer TPID. */
        rv = bcm_esw_port_tpid_set(unit, port, 
                                   _bcm_fb2_outer_tpid_default_get(unit));
        BCM_IF_ERROR_RETURN(rv);
    } else if (mode == BCM_PORT_DTAG_MODE_INTERNAL) {
        /* Add the default outer TPID. */
        rv = bcm_esw_switch_control_port_get(unit, port, 
                                             bcmSwitchColorSelect, &color_mode);
        BCM_IF_ERROR_RETURN(rv);

        rv = bcm_esw_port_tpid_add(unit, port, _bcm_fb2_outer_tpid_default_get(unit),
                                   color_mode);
        BCM_IF_ERROR_RETURN(rv);

    } else if (mode == BCM_PORT_DTAG_MODE_EXTERNAL) {
        /* Disable all outer TPIDs. */
        BCM_IF_ERROR_RETURN (bcm_esw_port_tpid_delete_all(unit, port));
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_trx_lport_tab_default_entry_add
 * Purpose:
 *      Internal function for initializing common resource management
 *      Shared by Virtual-port based  modules
 * Parameters:
 *      unit    -  (IN) Device number.
 *      prof    -  (IN) LPORT memory profile. 
 * Returns:
 *      BCM_X_XXX
 */
int 
_bcm_trx_lport_tab_default_entry_add(int unit, soc_profile_mem_t *prof) 
{
    soc_info_t *si;
    lport_tab_entry_t buf;	
    source_trunk_map_table_entry_t stm_entry;
    uint32 index;
    void *entry[1];
    int rv=BCM_E_NONE;
    int idx;
    bcm_module_t my_modid=0;
    bcm_port_t port;

    /* Input parameters check. */
    if (NULL == prof) {
        return (BCM_E_PARAM);
    }

    si = &SOC_INFO(unit);

    sal_memcpy(&buf, soc_mem_entry_null(unit, LPORT_TABm),
               soc_mem_entry_words(unit, LPORT_TABm) * sizeof(uint32)); 

    /* Create a default LPORT profile entry */
    rv = bcm_esw_stk_my_modid_get(unit, &my_modid);
    if (rv < 0) {
        return rv;
    }

    /* Enable IFP. */
    soc_LPORT_TABm_field32_set(unit, &buf, FILTER_ENABLEf, 1);

    /* Enable VFP. */
    if (!SOC_IS_HURRICANE(unit)) {
        soc_LPORT_TABm_field32_set(unit, &buf, VFP_ENABLEf, 1);
    }

    /* Set port filed select index to PFS index max - 1. */
    if(SOC_MEM_FIELD_VALID(unit, LPORT_TABm, FP_PORT_FIELD_SEL_INDEXf)) {
        soc_LPORT_TABm_field32_set(unit, &buf, FP_PORT_FIELD_SEL_INDEXf,
                                   (soc_mem_index_max(unit, FP_PORT_FIELD_SELm) - 1));
    }

    /* Enable MPLS */
    if (SOC_MEM_FIELD_VALID(unit, LPORT_TABm, MPLS_ENABLEf)) {
         soc_LPORT_TABm_field32_set(unit, &buf, MPLS_ENABLEf, 1);
    }

    /* Enable TRILL */
    if (SOC_MEM_FIELD_VALID(unit, LPORT_TABm, TRILL_ENABLEf)) {
         soc_LPORT_TABm_field32_set(unit, &buf, TRILL_ENABLEf, 1);
    }
    if (SOC_MEM_FIELD_VALID(unit, LPORT_TABm, ALLOW_TRILL_FRAMESf)) {
         soc_LPORT_TABm_field32_set(unit, &buf, ALLOW_TRILL_FRAMESf, 1);
    }
    if (SOC_MEM_FIELD_VALID(unit, LPORT_TABm, ALLOW_NON_TRILL_FRAMESf)) {
         soc_LPORT_TABm_field32_set(unit, &buf, ALLOW_NON_TRILL_FRAMESf, 1);
    }
    if (SOC_MEM_FIELD_VALID(unit, LPORT_TABm, COPY_CORE_IS_IS_TO_CPUf)) {
         soc_LPORT_TABm_field32_set(unit, &buf, COPY_CORE_IS_IS_TO_CPUf, 1);
    }
    if (SOC_MEM_FIELD_VALID(unit, LPORT_TABm, RTAG7_HASH_CFG_SEL_TRILL_ECMPf)) {
         soc_LPORT_TABm_field32_set(unit, &buf, RTAG7_HASH_CFG_SEL_TRILL_ECMPf, 1);
    }

    /* Enable OAM */
    if (SOC_MEM_FIELD_VALID(unit, LPORT_TABm, OAM_ENABLEf)) {
         soc_LPORT_TABm_field32_set(unit, &buf, OAM_ENABLEf, 1);
    }

    /* Enable MIM */
    if (SOC_MEM_FIELD_VALID(unit, LPORT_TABm, MIM_TERM_ENABLEf)) {
         soc_LPORT_TABm_field32_set(unit, &buf, MIM_TERM_ENABLEf, 1);
    }
    if (SOC_MEM_FIELD_VALID(unit, LPORT_TABm, MIM_MC_TERM_ENABLEf)) {
        soc_LPORT_TABm_field32_set(unit, &buf, MIM_MC_TERM_ENABLEf, 1);
    } 
    if (SOC_MEM_FIELD_VALID(unit, LPORT_TABm, SRC_SYS_PORT_IDf)) {
        soc_LPORT_TABm_field32_set(unit, &buf, SRC_SYS_PORT_IDf,
                                   si->lb_port);
    }

    soc_LPORT_TABm_field32_set(unit, &buf, MY_MODIDf, my_modid);
    if (soc_mem_field_valid(unit, LPORT_TABm, FP_PORT_FIELD_SEL_INDEXf)) {
        soc_LPORT_TABm_field32_set(unit, &buf, FP_PORT_FIELD_SEL_INDEXf,
                                   (soc_mem_index_max(unit, 
                                                  FP_PORT_FIELD_SELm) - 1));
    }

    entry[0] = &buf;
    rv = soc_profile_mem_add(unit, prof, entry, 1, &index);
    if (index != soc_mem_index_min(unit, LPORT_TABm)) {
        /* Something went horribly wrong. */
        return (BCM_E_INTERNAL);
    }

    for (port = 0; port < 64; port++) {
        idx = my_modid * 64 + port;
        rv = READ_SOURCE_TRUNK_MAP_TABLEm(unit, MEM_BLOCK_ANY, idx, &stm_entry);
        if (rv < 0) {
            return rv;
        }
        soc_SOURCE_TRUNK_MAP_TABLEm_field32_set(unit, &stm_entry, 
                                                LPORT_PROFILE_IDXf, index);
        rv = WRITE_SOURCE_TRUNK_MAP_TABLEm(unit, MEM_BLOCK_ALL, idx, &stm_entry);
        if (rv < 0) {
            return rv;
        }
    }
    return (rv);
}

#else /* BCM_TRX_SUPPORT */
int bcm_esw_trx_port_not_empty;
#endif  /* BCM_TRX_SUPPORT */
