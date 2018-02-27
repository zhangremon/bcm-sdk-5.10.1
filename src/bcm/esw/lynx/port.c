/*
 * $Id: port.c 1.10 Broadcom SDK $
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
 * Purpose:     Lynx port function implementations
 */

#include <soc/defs.h>

#if defined(BCM_LYNX_SUPPORT)

#include <assert.h>

#include <sal/core/libc.h>

#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/cm.h>
#include <soc/drv.h>
#include <soc/register.h>
#include <soc/memory.h>
#include <soc/ll.h>
#include <soc/ptable.h>

#include <bcm/port.h>
#include <bcm/vlan.h>
#include <bcm/filter.h>
#include <bcm/error.h>
#include <bcm/link.h>

#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/lynx.h>

/*
 * Function:
 *      _bcm_lynx_port_protocol_vlan_add
 * Purpose:
 *      Adds a protocol based vlan to a port.  The protocol
 *      is matched by frame type and ether type.  
 * Parameters:
 *      unit  - (IN) BCM device number
 *      port  - (IN) Port number
 *      frame - (IN) one of BCM_PORT_FRAMETYPE_{ETHER2,8023,LLC}
 *      ether - (IN) 16 bit Ethernet type field
 *      vid   - (IN)VLAN ID
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_lynx_port_protocol_vlan_add(int unit,
                           bcm_port_t port,
                           bcm_port_frametype_t frame,
                           bcm_port_ethertype_t ether,
                           bcm_vlan_t vid)
{
    int                     i, empty;
    bcm_port_frametype_t    ft;
    bcm_port_ethertype_t    et;
    int                     idxmin;
    int                     idxmax;
    int                     blk;
    pr_tab_entry_t          pte;
    _bcm_port_info_t        *pinfo;

    blk = SOC_PORT_BLOCK(unit, port);
    idxmin = soc_mem_index_min(unit, PR_TABm);
    idxmax = soc_mem_index_max(unit, PR_TABm);
    empty = -1;

    for (i = idxmin; i <= idxmax; i++) {
        SOC_IF_ERROR_RETURN(READ_PR_TABm(unit, blk, i, &pte));
        ft = soc_PR_TABm_field32_get(unit, &pte, FRAMETYPEf);
        et = soc_PR_TABm_field32_get(unit, &pte, ETHERTYPEf);
        if (empty < 0 && ft == 0) {
            empty = i;
        }
        if (ft == frame && et == ether) {
            return BCM_E_EXISTS;
        }
    }
    if (empty < 0) {
        return BCM_E_FULL;
    }
    soc_PR_TABm_field32_set(unit, &pte, FRAMETYPEf, frame);
    soc_PR_TABm_field32_set(unit, &pte, ETHERTYPEf, ether);
    soc_PR_TABm_field32_set(unit, &pte, VLAN_IDf, vid);
    BCM_IF_ERROR_RETURN(_bcm_port_info_get(unit, port, &pinfo));
    soc_PR_TABm_field32_set(unit, &pte, PRIf, pinfo->p_ut_prio);
    SOC_IF_ERROR_RETURN(WRITE_PR_TABm(unit, blk, empty, &pte));
    return BCM_E_NONE;

}

/*
 * Function:
 *      _bcm_lynx_port_protocol_vlan_delete
 * Purpose:
 *      Remove an already created proto protocol based vlan
 *      on a port.
 * Parameters:
 *      unit - (IN) BCM device number
 *      port - (IN) Port number
 *      frame - (IN) one of BCM_PORT_FRAMETYPE_{ETHER2,8023,LLC}
 *      ether - (IN) 16 bit Ethernet type field
 * Returns:
 *      BCM_E_XXX
 */

int
_bcm_lynx_port_protocol_vlan_delete(int unit,
                              bcm_port_t port,
                              bcm_port_frametype_t frame,
                              bcm_port_ethertype_t ether)
{
    int                          i;
    int                          idxmin, idxmax, blk;
    pr_tab_entry_t               pte;
    bcm_port_frametype_t         ft;
    bcm_port_ethertype_t         et;

    blk = SOC_PORT_BLOCK(unit, port);

    idxmin = soc_mem_index_min(unit, PR_TABm);
    idxmax = soc_mem_index_max(unit, PR_TABm);

    for (i = idxmin; i <= idxmax; i++) {
        SOC_IF_ERROR_RETURN(READ_PR_TABm(unit, blk, i, &pte));
        ft = soc_PR_TABm_field32_get(unit, &pte, FRAMETYPEf);
        et = soc_PR_TABm_field32_get(unit, &pte, ETHERTYPEf);
        if (ft == frame && et == ether){
            sal_memset(&pte, 0, sizeof(pte));
            SOC_IF_ERROR_RETURN(WRITE_PR_TABm(unit, blk, i, &pte));
            return BCM_E_NONE;
        }
    }
    return BCM_E_NOT_FOUND;
}
#ifdef BCM_METER_SUPPORT 

/*
 * Function:
 *	bcm_lynx_port_rate_egress_set
 * Purpose:
 *	Set egress rate limiting parameters for the Lynx chip.
 * Parameters:
 *	unit - Device number
 *	port - Port number
 *	kbits_sec - Rate in kilobits (1000 bits) per second.
 *			Rate of 0 disables rate limiting.
 *	kbits_burst - Maximum burst size in kilobits (1000 bits).
 * Returns:
 *	BCM_E_XXX
 */

int bcm_lynx_port_rate_egress_set(int unit,
				  bcm_port_t port,
				  uint32 kbits_sec,
				  uint32 kbits_burst)
{

    uint64 regval64;

    SOC_IF_ERROR_RETURN(READ_EGR_FLOWCTL_CFGr(unit, port, &regval64));
    if (!kbits_sec) {
        /* Disable egress metering for this port */
	soc_reg64_field32_set(unit, EGR_FLOWCTL_CFGr, &regval64,
			      REFCOUNTf, 0);
	soc_reg64_field32_set(unit, EGR_FLOWCTL_CFGr, &regval64,
			      FLOW_ENf, 0);
	SOC_IF_ERROR_RETURN(WRITE_EGR_FLOWCTL_CFGr(unit, port, regval64));
    }
    else {
        /* Enable metering for this port */

	uint32 regval;

	/* Check kbits_sec upper limit prior to "granularization" */
	if (kbits_sec > (0xFFFFFFFF-999))
	    kbits_sec = (0xFFFFFFFF-999);

	if ((regval = (kbits_sec+999)/1000) > 0x3FFF)
	    regval = 0x3FFF;

	soc_reg64_field32_set(unit, EGR_FLOWCTL_CFGr, &regval64,
			      REFCOUNTf, regval);
	soc_reg64_field32_set(unit, EGR_FLOWCTL_CFGr, &regval64,
			      BKT_SIZEf, 
			      _bcm_lynx_kbits_to_bucketsize(kbits_burst));
	/* Set the FLOW_EN bit */
	soc_reg64_field32_set(unit, EGR_FLOWCTL_CFGr, &regval64,
			      FLOW_ENf, 1);
	SOC_IF_ERROR_RETURN(WRITE_EGR_FLOWCTL_CFGr(unit, port, regval64));
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_lynx_port_rate_egress_get
 * Purpose:
 *	Get egress rate limiting parameters from the Lynx chip.
 * Parameters:
 *	unit - Device number
 *	port - Port number
 *	kbits_sec - (OUT) Rate in kilobits (1000 bits) per second, or
 *			  zero if rate limiting is disabled.
 *	kbits_burst - (OUT) Maximum burst size in kilobits (1000 bits).
 * Returns:
 *	BCM_E_XXX
 */

int bcm_lynx_port_rate_egress_get(int unit,
				  bcm_port_t port,
				  uint32 *kbits_sec,
				  uint32 *kbits_burst)
{

    uint8 regval;
    uint64 regval64;

    if (!kbits_sec || !kbits_burst) {
        return (BCM_E_PARAM);
    }

    SOC_IF_ERROR_RETURN(READ_EGR_FLOWCTL_CFGr(unit, port, &regval64));

    if (!soc_reg64_field32_get(unit, EGR_FLOWCTL_CFGr, regval64, FLOW_ENf)) {
        *kbits_sec = *kbits_burst = 0;
    }
    else {
        /* Convert the REFRESHCOUNT field to kbits/sec (1000 bits/sec). */
        *kbits_sec = 1000 *
	  soc_reg64_field32_get(unit, EGR_FLOWCTL_CFGr, regval64,
				REFCOUNTf);
	/* Convert the BUCKETSIZE field to burst size in kbits. */
	regval =
	  soc_reg64_field32_get(unit, EGR_FLOWCTL_CFGr, regval64,
				BKT_SIZEf);
	*kbits_burst = _bcm_lynx_bucketsize_to_kbits(regval);
    }
    return BCM_E_NONE;
}


/*
 * Function:
 *      _bcm_lynx_port_dtag_mode_get
 * Description:
 *      Return the current double-tagging mode of a port.
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      mode - (OUT) Double-tagging mode
 * Return Value:
 *      BCM_E_XXX
 */
int
_bcm_lynx_port_dtag_mode_get(int unit, bcm_port_t port, int *mode)
{
    uint32  config;
    uint64  vctrl;

    SOC_IF_ERROR_RETURN(READ_DT_CONFIG1r(unit, port, &config));
    if (soc_reg_field_get(unit, DT_CONFIG1r, config, DUMMYTAG_ENABLEf) == 0) {
        *mode = BCM_PORT_DTAG_MODE_NONE;
    } else {
        SOC_IF_ERROR_RETURN(READ_VLAN_CONTROLr(unit, port, &vctrl));
        if (soc_reg64_field32_get(unit, VLAN_CONTROLr, vctrl, IGNORE_TAGf)) {
            *mode = BCM_PORT_DTAG_MODE_EXTERNAL;
        } else {
            *mode = BCM_PORT_DTAG_MODE_INTERNAL;
        }
    }
    return BCM_E_NONE;
}


/*
 * Function:
 *      _bcm_lynx_port_dtag_mode_set
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
 *      dt_mode - (IN) True/False double tagged port.
 *      ignore_tag - (IN) Ignore outer tag. 
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
 *      On some chips, such as BCM5665, double-tag enable is a system-wide
 *              setting rather than a port setting, so enabling double-
 *              tagging on one port may enable it on all ports.
 */
int
_bcm_lynx_port_dtag_mode_set(int unit, bcm_port_t port, int mode, 
                             int dt_mode, int ignore_tag)
{
    uint32  config, oconfig;
    uint64  vctrl, ovctrl;

    SOC_IF_ERROR_RETURN(READ_DT_CONFIG1r(unit, port, &config));
    oconfig = config;
    SOC_IF_ERROR_RETURN(READ_VLAN_CONTROLr(unit, port, &vctrl));
    ovctrl = vctrl;
    soc_reg_field_set(unit, DT_CONFIG1r, &config, DUMMYTAG_ENABLEf, dt_mode);
    soc_reg64_field32_set(unit, VLAN_CONTROLr, &vctrl, IGNORE_TAGf, ignore_tag);
    if (COMPILER_64_NE(vctrl, ovctrl)) {
        SOC_IF_ERROR_RETURN(WRITE_VLAN_CONTROLr(unit, port, vctrl));
    }
    if (config != oconfig) {
        SOC_IF_ERROR_RETURN(WRITE_DT_CONFIG1r(unit, port, config));
    }
    return BCM_E_NONE;
}

#endif /* BCM_METER_SUPPORT */

#endif	/* BCM_LYNX_SUPPORT */
