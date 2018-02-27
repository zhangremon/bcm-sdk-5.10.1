/*
 * $Id: htls.c 1.11 Broadcom SDK $
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
 * These routines allow configuration of HTLS/VPLS.
 *
 */

#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/debug.h>

#include <bcm/error.h>
#include <bcm/htls.h>

#include <bcm_int/esw_dispatch.h>

#define VPLS_INIT(unit) \
        if (!_vpls_info[unit] || !_vpls_info[unit]->initted) \
        { return BCM_E_INIT; }

#define VPLS_LOCK(unit) \
        soc_mem_lock(unit, VPLS_BITMAP_TABLEm)
#define VPLS_UNLOCK(unit) \
        soc_mem_unlock(unit, VPLS_BITMAP_TABLEm)

#define VPLS_CHECK(unit, vclabel) \
        if ((vclabel == 0) || (vclabel > BCM_VCLABEL_MAX)) {return BCM_E_PARAM;}

typedef struct _bcm_vpls_info_s {
    int          initted;
    uint32       num_labels;
    uint32       *labels;
} _bcm_vpls_info_t;

static _bcm_vpls_info_t *_vpls_info[BCM_MAX_NUM_UNITS];

/*
 * Function:
 *      bcm_esw_vpls_bitmap_init
 * Purpose:
 *      Initialize the VPLS info for a unit.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 * Returns:
 *      BCM_E_NONE - Success.
 * Notes:
 *
 */
int
bcm_esw_vpls_bitmap_init(int unit)
{
    int              alloc_size, num_entries;
    
    if (!SOC_IS_TUCANA(unit)) {
        return BCM_E_UNAVAIL;
    }

    bcm_esw_vpls_bitmap_detach(unit);
    
    alloc_size = sizeof(_bcm_vpls_info_t);
    _vpls_info[unit] = sal_alloc(alloc_size, "VPLS info");
    if (_vpls_info[unit] == NULL) {
        return BCM_E_MEMORY;
    }
    sal_memset(_vpls_info[unit], 0, alloc_size);
    
    num_entries = soc_mem_index_count(unit, VPLS_BITMAP_TABLEm);
    
    alloc_size = num_entries * sizeof(uint32);
    _vpls_info[unit]->labels = sal_alloc(alloc_size, "VPLS entries");
    if (_vpls_info[unit]->labels == NULL) {
        return BCM_E_MEMORY;
    }
    sal_memset(_vpls_info[unit]->labels, 0, alloc_size);

    _vpls_info[unit]->num_labels = num_entries;

    SOC_IF_ERROR_RETURN
        (soc_mem_clear(unit, VPLS_BITMAP_TABLEm, COPYNO_ALL, FALSE));

    _vpls_info[unit]->initted = TRUE;

    return BCM_E_NONE;
}

 int
_bcm_vpls_entry_find(int unit, uint32 vc_label)
{
    uint32 ix;

    for (ix = 0; ix < _vpls_info[unit]->num_labels; ix++) {
        if (vc_label == _vpls_info[unit]->labels[ix]) {
            return ix;
        }
    }

    return -1;
}

static int
_bcm_vpls_entry_free(int unit)
{
    uint32 ix;

    for (ix = 0; ix < _vpls_info[unit]->num_labels; ix++) {
        if (_vpls_info[unit]->labels[ix] == 0) {
            return ix;
        }
    }

    return -1;
}

/*
 * Function:
 *      bcm_esw_vpls_bitmap_set
 * Purpose:
 *      Configure the bitmap and VLAN id for a VC label.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      vc_label - VPLS VC label.
 *      vid - VLAN ID associated with VC label.
 *      pbmp - VPLS port bitmap.
 * Returns:
 *      BCM_E_NONE - Success.
 * Notes:
 *
 */
int
bcm_esw_vpls_bitmap_add(int unit, uint32 vc_label, bcm_vlan_t vid, 
                    bcm_pbmp_t pbmp)
{
    vpls_bitmap_table_entry_t vbt;
    int             index, rv;

    if (!SOC_IS_TUCANA(unit)) {
        return BCM_E_UNAVAIL;
    }

    VPLS_INIT(unit);
    VPLS_CHECK(unit, vc_label);

    VPLS_LOCK(unit);
    index = _bcm_vpls_entry_find(unit, vc_label);
    if (index < 0) { /* Not found */
        index = _bcm_vpls_entry_free(unit);
        if (index < 0) { /* No free entries */
            VPLS_UNLOCK(unit);
            return BCM_E_RESOURCE;
        }
    }

    sal_memset(&vbt, 0, sizeof(vbt));

    soc_VPLS_BITMAP_TABLEm_field32_set(unit, &vbt, VC_LABELf, vc_label);
    soc_VPLS_BITMAP_TABLEm_field32_set(unit, &vbt, VLAN_IDf, vid);
    soc_VPLS_BITMAP_TABLEm_field32_set(unit, &vbt, VPLS_BITMAP_M0f, 
                                       SOC_PBMP_WORD_GET(pbmp, 0));
    soc_VPLS_BITMAP_TABLEm_field32_set(unit, &vbt, VPLS_BITMAP_M1f, 
                                       SOC_PBMP_WORD_GET(pbmp, 1));
    soc_VPLS_BITMAP_TABLEm_field32_set(unit, &vbt, VALIDf, 1);
    
    if ((rv = WRITE_VPLS_BITMAP_TABLEm(unit, MEM_BLOCK_ALL, index, &vbt)) < 0) {
        VPLS_UNLOCK(unit);
        return rv;
    }

    _vpls_info[unit]->labels[index] = vc_label;

    VPLS_UNLOCK(unit);
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_esw_vpls_bitmap_delete
 * Purpose:
 *      Remove the bitmap and VLAN id for a VC label.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      vc_label - VPLS VC label.
 * Returns:
 *      BCM_E_NONE - Success.
 * Notes:
 *
 */
int
bcm_esw_vpls_bitmap_delete(int unit, uint32 vc_label)
{
    int                index, rv;

    if (!SOC_IS_TUCANA(unit)) {
        return BCM_E_UNAVAIL;
    }

    VPLS_INIT(unit);
    VPLS_CHECK(unit, vc_label);

    VPLS_LOCK(unit);
    index = _bcm_vpls_entry_find(unit, vc_label);
    if (index < 0) { /* Not found */
        VPLS_UNLOCK(unit);
        return BCM_E_PARAM;
     }

    if ((rv = WRITE_VPLS_BITMAP_TABLEm(unit, MEM_BLOCK_ALL, index, 
                    soc_mem_entry_null(unit, VPLS_BITMAP_TABLEm))) < 0) {
        VPLS_UNLOCK(unit);
        return rv;
    }

    _vpls_info[unit]->labels[index] = 0;

    VPLS_UNLOCK(unit);
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_esw_vpls_bitmap_delete
 * Purpose:
 *      Clear the VPLS labels for a unit.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 * Returns:
 *      BCM_E_NONE - Success.
 * Notes:
 *
 */
int
bcm_esw_vpls_bitmap_delete_all(int unit)
{
    int                rv;

    if (!SOC_IS_TUCANA(unit)) {
        return BCM_E_UNAVAIL;
    }

    VPLS_INIT(unit);
    VPLS_LOCK(unit);
    if ((rv = soc_mem_clear(unit, VPLS_BITMAP_TABLEm, COPYNO_ALL, FALSE)) < 0) {
        VPLS_UNLOCK(unit);
        return rv;
    }

    sal_memset(_vpls_info[unit]->labels, 0, _vpls_info[unit]->num_labels);

    VPLS_UNLOCK(unit);
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_esw_vpls_bitmap_get
 * Purpose:
 *      Retrieve the bitmap and VLAN id for a VC label.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      vc_label - VPLS VC label.
 *      vid - (OUT) VLAN ID associated with VC label.
 *      pbmp - (OUT) VPLS port bitmap.
 * Returns:
 *      BCM_E_NONE - Success.
 * Notes:
 *
 */
int
bcm_esw_vpls_bitmap_get(int unit, uint32 vc_label, bcm_vlan_t *vid, 
                    bcm_pbmp_t *pbmp)
{
    vpls_bitmap_table_entry_t vbt;
    int             index, rv;

    if (!SOC_IS_TUCANA(unit)) {
        return BCM_E_UNAVAIL;
    }

    VPLS_INIT(unit);
    VPLS_CHECK(unit, vc_label);

    VPLS_LOCK(unit);
    index = _bcm_vpls_entry_find(unit, vc_label);
    if (index < 0) { /* Not found */
        VPLS_UNLOCK(unit);
        return BCM_E_PARAM;
     }

    if ((rv = READ_VPLS_BITMAP_TABLEm(unit, MEM_BLOCK_ALL, index, &vbt)) < 0) {
        VPLS_UNLOCK(unit);
        return rv;
    }

    *vid = soc_VPLS_BITMAP_TABLEm_field32_get(unit, &vbt, VLAN_IDf);
    SOC_PBMP_WORD_SET(*pbmp, 0, 
        soc_VPLS_BITMAP_TABLEm_field32_get(unit, &vbt, VPLS_BITMAP_M0f));
    SOC_PBMP_WORD_SET(*pbmp, 1, 
        soc_VPLS_BITMAP_TABLEm_field32_get(unit, &vbt, VPLS_BITMAP_M1f));

    VPLS_UNLOCK(unit);
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_esw_vpls_bitmap_detach
 * Purpose:
 *      Clean up the VPLS info for a unit.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 * Returns:
 *      BCM_E_NONE - Success.
 * Notes:
 *
 */
int
bcm_esw_vpls_bitmap_detach(int unit)
{
    if (!SOC_IS_TUCANA(unit)) {
        return BCM_E_UNAVAIL;
    }

    if (_vpls_info[unit] != NULL) {
        if (_vpls_info[unit]->labels) {
            sal_free(_vpls_info[unit]->labels);
            _vpls_info[unit]->labels = NULL;
        }
        sal_free(_vpls_info[unit]);
        _vpls_info[unit] = NULL;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_esw_htls_mode_set
 * Purpose:
 *      Configure the HTLS mode for a unit.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      enable - (BOOL) enable/disable the HTLS mode.
 * Returns:
 *      BCM_E_NONE - Success.
 * Notes:
 *
 */
int
bcm_esw_htls_mode_set(int unit, int enable)
{
    uint32              rval, orval;

    if (!SOC_IS_TUCANA(unit)) {
        return BCM_E_UNAVAIL;
    }

    BCM_IF_ERROR_RETURN(READ_CHIP_CONFIGr(unit, &rval));
    orval = rval;
    soc_reg_field_set(unit, CHIP_CONFIGr, &rval, HTLS_MODEf, enable ? 1 : 0);
    if (rval != orval) {
        BCM_IF_ERROR_RETURN(WRITE_CHIP_CONFIGr(unit, rval));
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_esw_htls_mode_get
 * Purpose:
 *      Get the HTLS mode for a unit.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      enable - (OUT)(BOOL) the HTLS mode enabled/disabled.
 * Returns:
 *      BCM_E_NONE - Success.
 * Notes:
 *
 */
int
bcm_esw_htls_mode_get(int unit, int *enable)
{
    uint32              rval;

    if (!SOC_IS_TUCANA(unit)) {
        return BCM_E_UNAVAIL;
    }

    BCM_IF_ERROR_RETURN(READ_CHIP_CONFIGr(unit, &rval));
    *enable = (soc_reg_field_get(unit, CHIP_CONFIGr, rval, HTLS_MODEf) != 0);
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_esw_htls_vclabel_set
 * Purpose:
 *      Set the default HTLS VC label for a port.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      port - Port number
 *      vc_label - HTLS VC label.
 * Returns:
 *      BCM_E_NONE - Success.
 * Notes:
 *
 */
int
bcm_esw_htls_vclabel_set(int unit, bcm_port_t port, uint32 vc_label)
{
    uint64              rval, orval;

    if (!SOC_IS_TUCANA(unit)) {
        return BCM_E_UNAVAIL;
    }

    VPLS_CHECK(unit,vc_label);

    BCM_IF_ERROR_RETURN(READ_HTLS_VC_LABELr(unit, port, &rval));
    COMPILER_64_SET(orval, COMPILER_64_HI(rval), COMPILER_64_LO(rval));
    soc_reg64_field32_set(unit, HTLS_VC_LABELr, &rval, VCLABELf, vc_label);
    if (COMPILER_64_NE(rval, orval)) {
        BCM_IF_ERROR_RETURN(WRITE_HTLS_VC_LABELr(unit, port, rval));
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_esw_htls_vclabel_get
 * Purpose:
 *      Get the default HTLS VC label for a port.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      port - Port number
 *      vc_label - HTLS VC label.
 * Returns:
 *      BCM_E_NONE - Success.
 * Notes:
 *
 */
int
bcm_esw_htls_vclabel_get(int unit, bcm_port_t port, uint32 *vc_label)
{
    uint64              rval;

    if (!SOC_IS_TUCANA(unit)) {
        return BCM_E_UNAVAIL;
    }

    BCM_IF_ERROR_RETURN(READ_HTLS_VC_LABELr(unit, port, &rval));
    *vc_label = soc_reg64_field32_get(unit, HTLS_VC_LABELr, rval, VCLABELf);
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_esw_htls_tunnel_label_set
 * Purpose:
 *      Set the uplink HTLS VC label for a port.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      port - Port number
 *      tunnel_label - HTLS tunnel label.
 * Returns:
 *      BCM_E_NONE - Success.
 * Notes:
 *
 */
int
bcm_esw_htls_tunnel_label_set(int unit, bcm_port_t port, uint32 tunnel_label)
{
    uint64              rval, orval;

    if (!SOC_IS_TUCANA(unit)) {
        return BCM_E_UNAVAIL;
    }

    if (!IS_GE_PORT(unit, port)) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(READ_HTLS_UPLINK_TUNNELr(unit, port, &rval));
    COMPILER_64_SET(orval, COMPILER_64_HI(rval), COMPILER_64_LO(rval));
    soc_reg64_field32_set(unit, HTLS_UPLINK_TUNNELr, &rval, TUNNEL_LABELf, 
                          tunnel_label);
    if (COMPILER_64_NE(rval, orval)) {
        BCM_IF_ERROR_RETURN(WRITE_HTLS_UPLINK_TUNNELr(unit, port, rval));
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_esw_htls_tunnel_label_get
 * Purpose:
 *      Get the uplink HTLS VC label for a port.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      port - Port number
 *      tunnel_label - HTLS tunnel label.
 * Returns:
 *      BCM_E_NONE - Success.
 * Notes:
 *
 */
int
bcm_esw_htls_tunnel_label_get(int unit, bcm_port_t port, uint32 *tunnel_label)
{
    uint64              rval;

    if (!SOC_IS_TUCANA(unit)) {
        return BCM_E_UNAVAIL;
    }

    if (!IS_GE_PORT(unit, port)) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(READ_HTLS_UPLINK_TUNNELr(unit, port, &rval));
    *tunnel_label = soc_reg64_field32_get(unit, HTLS_UPLINK_TUNNELr, rval, 
                                          TUNNEL_LABELf);
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_esw_htls_uplink_da_set
 * Purpose:
 *      Set DA for HTLS on uplink port.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      port - Port number.
 *      dest_mac - destination Ethernet address for HTLS uplink.
 * Returns:
 *      BCM_E_NONE - Success.
 * Notes:
 *      For 5665, this must be a GE port.
 */
int
bcm_esw_htls_uplink_da_set(int unit, bcm_port_t port, bcm_mac_t dest_mac)
{
    uint64              rval, orval, rmac;

    if (!SOC_IS_TUCANA(unit)) {
        return BCM_E_UNAVAIL;
    }

    if (!IS_GE_PORT(unit, port)) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(READ_HTLS_UPLINK_DAr(unit, port, &rval));
    COMPILER_64_SET(orval, COMPILER_64_HI(rval), COMPILER_64_LO(rval));
    SAL_MAC_ADDR_TO_UINT64(dest_mac, rmac);
    soc_reg64_field_set(unit, HTLS_UPLINK_DAr, &rval, DAf, rmac);
    if (COMPILER_64_NE(rval, orval)) {
        BCM_IF_ERROR_RETURN(WRITE_HTLS_UPLINK_DAr(unit, port, rval));
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_esw_htls_uplink_da_get
 * Purpose:
 *      Get DA for HTLS on uplink port.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      port - Port number.
 *      dest_mac - (OUT) destination Ethernet address for HTLS uplink.
 * Returns:
 *      BCM_E_NONE - Success.
 * Notes:
 *      For 5665, this must be a GE port.
 */
int
bcm_esw_htls_uplink_da_get(int unit, bcm_port_t port, bcm_mac_t dest_mac)
{
    uint64              rval, da64;

    if (!SOC_IS_TUCANA(unit)) {
        return BCM_E_UNAVAIL;
    }

    if (!IS_GE_PORT(unit, port)) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(READ_HTLS_UPLINK_DAr(unit, port, &rval));
    COMPILER_64_ZERO(da64);
    da64 = soc_reg64_field_get(unit, HTLS_UPLINK_DAr, rval, DAf);
    SAL_MAC_ADDR_FROM_UINT64(dest_mac, da64);
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_esw_htls_uplink_sa_set
 * Purpose:
 *      Set SA for HTLS on uplink port.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      port - Port number.
 *      src_mac - source Ethernet address for HTLS uplink.
 * Returns:
 *      BCM_E_NONE - Success.
 * Notes:
 *      For 5665, this must be a GE port.
 */
int
bcm_esw_htls_uplink_sa_set(int unit, bcm_port_t port, bcm_mac_t src_mac)
{
    uint64              rval, orval, rmac;

    if (!SOC_IS_TUCANA(unit)) {
        return BCM_E_UNAVAIL;
    }

    if (!IS_GE_PORT(unit, port)) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(READ_HTLS_UPLINK_SAr(unit, port, &rval));
    COMPILER_64_SET(orval, COMPILER_64_HI(rval), COMPILER_64_LO(rval));
    SAL_MAC_ADDR_TO_UINT64(src_mac, rmac);
    soc_reg64_field_set(unit, HTLS_UPLINK_SAr, &rval, SAf, rmac);
    if (COMPILER_64_NE(rval, orval)) {
        BCM_IF_ERROR_RETURN(WRITE_HTLS_UPLINK_SAr(unit, port, rval));
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_esw_htls_uplink_sa_get
 * Purpose:
 *      Get SA for HTLS on uplink port.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      port - Port number.
 *      src_mac - (OUT) source Ethernet address for HTLS uplink.
 * Returns:
 *      BCM_E_NONE - Success.
 * Notes:
 *      For 5665, this must be a GE port.
 */
int
bcm_esw_htls_uplink_sa_get(int unit, bcm_port_t port, bcm_mac_t src_mac)
{
    uint64              rval, sa64;

    if (!SOC_IS_TUCANA(unit)) {
        return BCM_E_UNAVAIL;
    }

    if (!IS_GE_PORT(unit, port)) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(READ_HTLS_UPLINK_SAr(unit, port, &rval));
    COMPILER_64_ZERO(sa64);
    sa64 = soc_reg64_field_get(unit, HTLS_UPLINK_SAr, rval, SAf);
    SAL_MAC_ADDR_FROM_UINT64(src_mac, sa64);
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_esw_htls_egress_port_set
 * Purpose:
 *      Configure HTLS uplink properties.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      port - port to config.
 *      ttl - HTLS header TTL value on egress.
 *      mmc_type - ethertype for HTLS multicast packets.
 *      flags - other bitfield configurations:
 *              BCM_HTLS_DLF_AS_UNICAST
 *                      Treat unicast DLF lookups as unicast packets
 *              BCM_HTLS_CHECK_DA
 *                      Check H-TLS packet incoming DA in ingress, 
 *                      for match of egress HTLS_SA
 *              BCM_HTLS_INSERT_TUNNEL_LABEL
 *                      Insert tunnel label into H-TLS header in egress
 *              BCM_HTLS_UPLINK
 *                      Port is configured as H-TLS uplink port
 *              BCM_HTLS_USE_EXP
 *                      Use H-TLS EXP field for packet priority
 * Returns:
 *      BCM_E_NONE - Success.
 * Notes:
 *      For 5665, this must be a GE port.
 */
int
bcm_esw_htls_egress_port_set(int unit, bcm_port_t port, int ttl, 
                         uint16 mmc_type, uint32 flags)
{
    uint64              rval, orval;

    if (!SOC_IS_TUCANA(unit)) {
        return BCM_E_UNAVAIL;
    }

    if (!IS_GE_PORT(unit, port)) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(READ_HTLS_UPLINK_CONTROLr(unit, port, &rval));
    COMPILER_64_SET(orval, COMPILER_64_HI(rval), COMPILER_64_LO(rval));
    soc_reg64_field32_set(unit, HTLS_UPLINK_CONTROLr, &rval, TTLf, ttl);
    soc_reg64_field32_set(unit, HTLS_UPLINK_CONTROLr, &rval, MARTINI_MC_TYPEf,
                          mmc_type);
    soc_reg64_field32_set(unit, HTLS_UPLINK_CONTROLr, &rval, DLF_AS_UNICASTf, 
                          (flags & BCM_HTLS_DLF_AS_UNICAST) ? 1 : 0);
    soc_reg64_field32_set(unit, HTLS_UPLINK_CONTROLr, &rval, CHECK_DAf, 
                          (flags & BCM_HTLS_CHECK_DA) ? 1 : 0);
    soc_reg64_field32_set(unit, HTLS_UPLINK_CONTROLr, &rval, 
                          INSERT_TUNNEL_LABELf, 
                          (flags & BCM_HTLS_INSERT_TUNNEL_LABEL) ? 1 : 0);
    soc_reg64_field32_set(unit, HTLS_UPLINK_CONTROLr, &rval, UPLINKf, 
                          (flags & BCM_HTLS_UPLINK) ? 1 : 0);
    soc_reg64_field32_set(unit, HTLS_UPLINK_CONTROLr, &rval, USE_EXPf, 
                          (flags & BCM_HTLS_USE_EXP) ? 1 : 0);
    if (COMPILER_64_NE(rval, orval)) {
        BCM_IF_ERROR_RETURN(WRITE_HTLS_UPLINK_CONTROLr(unit, port, rval));
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_esw_htls_egress_port_get
 * Purpose:
 *      Retrieve HTLS uplink properties.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      port - port to config.
 *      ttl - (OUT) HTLS header TTL value on egress.
 *      mmc_type - (OUT) ethertype for HTLS multicast packets.
 *      flags - (OUT) other bitfield configurations:
 *              BCM_HTLS_DLF_AS_UNICAST
 *                      Treat unicast DLF lookups as unicast packets
 *              BCM_HTLS_CHECK_DA
 *                      Check H-TLS packet incoming DA in ingress, 
 *                      for match of egress HTLS_SA
 *              BCM_HTLS_INSERT_TUNNEL_LABEL
 *                      Insert tunnel label into H-TLS header in egress
 *              BCM_HTLS_UPLINK
 *                      Port is configured as H-TLS uplink port
 *              BCM_HTLS_USE_EXP
 *                      Use H-TLS EXP field for packet priority
 * Returns:
 *      BCM_E_NONE - Success.
 * Notes:
 *      For 5665, this must be a GE port.
 */
int
bcm_esw_htls_egress_port_get(int unit, bcm_port_t port, int *ttl, 
                         uint16 *mmc_type, uint32 *flags)
{
    uint64              rval;
    uint32              lflags = 0;

    if (!SOC_IS_TUCANA(unit)) {
        return BCM_E_UNAVAIL;
    }

    if (!IS_GE_PORT(unit, port)) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(READ_HTLS_UPLINK_CONTROLr(unit, port, &rval));
    *ttl = soc_reg64_field32_get(unit, HTLS_UPLINK_CONTROLr, rval, TTLf);
    *mmc_type = soc_reg64_field32_get(unit, HTLS_UPLINK_CONTROLr, rval, 
                                      MARTINI_MC_TYPEf);
    if (soc_reg64_field32_get(unit, HTLS_UPLINK_CONTROLr, rval, 
                              DLF_AS_UNICASTf) != 0) {
        lflags |= BCM_HTLS_DLF_AS_UNICAST;
    }
    if (soc_reg64_field32_get(unit, HTLS_UPLINK_CONTROLr, rval, 
                              CHECK_DAf) != 0) {
        lflags |= BCM_HTLS_CHECK_DA;
    }
    if (soc_reg64_field32_get(unit, HTLS_UPLINK_CONTROLr, rval, 
                              INSERT_TUNNEL_LABELf) != 0) {
        lflags |= BCM_HTLS_INSERT_TUNNEL_LABEL;
    }
    if (soc_reg64_field32_get(unit, HTLS_UPLINK_CONTROLr, rval, 
                              UPLINKf) != 0) {
        lflags |= BCM_HTLS_UPLINK;
    }
    if (soc_reg64_field32_get(unit, HTLS_UPLINK_CONTROLr, rval, 
                              USE_EXPf) != 0) {
        lflags |= BCM_HTLS_USE_EXP;
    }

    *flags = lflags;
    return BCM_E_NONE;
}
