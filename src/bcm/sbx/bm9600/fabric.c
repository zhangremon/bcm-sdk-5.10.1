/*
 * $Id: fabric.c 1.45.20.1 Broadcom SDK $
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
 * BM9600 Fabric Control API
 */

#include <soc/debug.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_pl_auto.h>
#include <soc/sbx/bm9600.h>
#include <soc/sbx/bm9600_soc_init.h>
#include <soc/sbx/bm9600_init.h>
#include <soc/sbx/sbFabCommon.h>
#include <bcm_int/sbx/fabric.h>
#include <bcm_int/sbx/state.h>
#include <bcm_int/sbx/stack.h>
#include <bcm_int/sbx/cosq.h>
#include <bcm_int/sbx/port.h>

#include <bcm/error.h>
#include <bcm/port.h>
#include <bcm/fabric.h>
#include <bcm/stack.h>
#include <bcm/debug.h>


#define BM9600_NS_PER_CLOCK (4) /* 250MHz clock, 4 ns, per clock */

#define PLANE_A 0
#define PLANE_B 1
#define PLANE_BOTH 2

int
bcm_bm9600_fabric_crossbar_connection_set(int unit,
                                          int xbar, 
                                          int src_modid,
                                          bcm_port_t src_xbport,
                                          int dst_modid,
                                          bcm_port_t dst_xbport)
{
    int rv;
    int dst_node, dst_phy_node;
    int src_node;
    bcm_port_t dst_port, src_port;
    int configure_both_channels = FALSE;
    bcm_port_t dst_portb = 0, src_portb = 0;
    int src_dual_channel_even = TRUE;
    int dst_dual_channel_even = TRUE;
    int arbiter_id, plane= PLANE_BOTH;
    static int first_call = TRUE;

    /* The bcm_port_t is a physical port, which is split into two logical ports. We map both
     * logical ports to the equivalent logical ports on the other side of the crossbar.
     */
    if (!BCM_STK_MOD_IS_NODE(dst_modid)) {
	return BCM_E_PARAM;
    }
    dst_node = BCM_STK_MOD_TO_NODE(dst_modid);
    dst_phy_node = SOC_SBX_L2P_NODE(unit, dst_node);

    if (!BCM_STK_MOD_IS_NODE(src_modid)) {
	return(BCM_E_PARAM);
    }
    src_node = BCM_STK_MOD_TO_NODE(src_modid);
    
    /* Sequencing required for sfi_local channels, need to use 2 continguous lxbars   */
    /* so need to call back to back for A-A channels.  State is reserved in this case */
    if (first_call == FALSE) {
	if ((SOC_SBX_CFG_BM9600(unit)->uSerdesAbility[src_xbport] == BCM_PORT_ABILITY_DUAL_SFI) &&
	    (SOC_SBX_STATE(unit)->stack_state->protocol[src_node] == bcmModuleProtocol4)) {
	    src_dual_channel_even = FALSE;
	}
	if ((SOC_SBX_CFG_BM9600(unit)->uSerdesAbility[dst_xbport] == BCM_PORT_ABILITY_DUAL_SFI) &&
	    (SOC_SBX_STATE(unit)->stack_state->protocol[dst_node] == bcmModuleProtocol4)) {
	    dst_dual_channel_even = FALSE;
	}
	first_call = TRUE;
    } else { /* First call=true */
	first_call = FALSE;
    }

    /* bcmModuleProtocol1: (qe2000, plane A)
     *    assuming using even xbar ports
     * bcmModuleProtocol2: (qe2000, plane B)
     *    assuming using odd xbar ports  
     * bcmModuleProtocol3: (sirius, plane A/B)
     *    assuming using both even/odd xbar ports, even connect to even, odd connect to odd
     * bcmModuleProtocol4: (sirius, plane A B local)
     *    assuming using both even/odd xbar ports
     * bcmModuleProtocol5: (sirius plane A for 3.125G single sfi)
     */    

    /* Need to set xbar for correct plane if SFI setup for SFI_SCI link */
    /* By default, Arbiter 0 is plane A and arbiter 1 is plane B        */
    if (SOC_SBX_CFG_BM9600(unit)->uSerdesAbility[src_xbport] == BCM_PORT_ABILITY_SFI_SCI) {

        rv = bcm_fabric_control_get(unit, bcmFabricArbiterId, &arbiter_id);
	if (arbiter_id == 0) {
	    plane = PLANE_A;
	}else {
	    plane = PLANE_B;
	}
	/* And if protocol 4, always operate on the odd channel, there are not 2 logical crossbars for the sfi_sci link, only 1 */
	src_dual_channel_even = FALSE;
	dst_dual_channel_even = FALSE;
    }
    switch (SOC_SBX_STATE(unit)->stack_state->protocol[src_node]) {
	case bcmModuleProtocol1:
	case bcmModuleProtocol5:
	    switch (SOC_SBX_STATE(unit)->stack_state->protocol[dst_node]) {
		case bcmModuleProtocol1:
	        case bcmModuleProtocol5:
		    src_port = src_xbport * 2;
		    dst_port = dst_xbport * 2;
		    break;
		case bcmModuleProtocol2:
		    src_port = (src_xbport * 2);
		    dst_port = (dst_xbport * 2) + 1;
		    break;
		case bcmModuleProtocol3:
		    src_port = src_xbport * 2;
		    dst_port = ((dst_xbport * 2) & 0xFF) | (((dst_xbport * 2 + 1) & 0xFF) << 8);
		    break;
		case bcmModuleProtocol4:
		    /* sequencing, first even, then odd */
		    src_port = src_xbport * 2;

		    if (dst_dual_channel_even) {
			dst_port = dst_xbport * 2;
		    } else {
			dst_port = dst_xbport * 2 + 1;
		    }
		    break;
		default:
		    BCM_ERR(("ERROR: unsupported mode, Unit: %d srcNodeProtocol: 0x%x, destNodeProtocol: 0x%x)\n",
                             unit, SOC_SBX_STATE(unit)->stack_state->protocol[src_node],
			     SOC_SBX_STATE(unit)->stack_state->protocol[dst_node]));
		    return(BCM_E_PARAM);
	    }
	    break;

	case bcmModuleProtocol2:
	    switch (SOC_SBX_STATE(unit)->stack_state->protocol[dst_node]) {
		case bcmModuleProtocol1:
		case bcmModuleProtocol5:
		    src_port = (src_xbport * 2) + 1;
		    dst_port = (dst_xbport * 2);
		    break;
		case bcmModuleProtocol2:
		    src_port = (src_xbport * 2) + 1;
		    dst_port = (dst_xbport * 2) + 1;
		    break;
		case bcmModuleProtocol3:
		    src_port = (src_xbport * 2) + 1;
		    dst_port = ((dst_xbport * 2) & 0xFF) | (((dst_xbport * 2 + 1) & 0xFF) << 8);
		    break;
		case bcmModuleProtocol4:
		    /* sequencing, first even, then odd */
		    src_port = src_xbport * 2 + 1;

		    if (dst_dual_channel_even) {
			dst_port = dst_xbport * 2;
		    } else {
			dst_port = dst_xbport * 2 + 1;
		    }
		    break;
		default:
		    BCM_ERR(("ERROR: unsupported mode, Unit: %d srcNodeProtocol: 0x%x, destNodeProtocol: 0x%x)\n",
                             unit, SOC_SBX_STATE(unit)->stack_state->protocol[src_node],
			     SOC_SBX_STATE(unit)->stack_state->protocol[dst_node]));
		    return(BCM_E_PARAM);
	    }
	    break;
	case bcmModuleProtocol3:
	    switch (SOC_SBX_STATE(unit)->stack_state->protocol[dst_node]) {
		case bcmModuleProtocol1:
		case bcmModuleProtocol5:
		    src_port = src_xbport * 2;
		    dst_port = dst_xbport * 2;

		    configure_both_channels = TRUE;
		    src_portb = src_xbport * 2 + 1;
		    dst_portb = dst_xbport * 2;
		    break;
		case bcmModuleProtocol2:
		    src_port = (src_xbport * 2);
		    dst_port = (dst_xbport * 2) + 1;

		    configure_both_channels = TRUE;
		    src_portb = (src_xbport * 2) + 1;
		    dst_portb = (dst_xbport * 2) + 1;
		    break;
		case bcmModuleProtocol3:
		    src_port = src_xbport * 2;

		    dst_port = ((dst_xbport * 2) & 0xFF) | (((dst_xbport * 2 + 1) & 0xFF) << 8);
		    
		    configure_both_channels = TRUE;
		    src_portb = (src_xbport * 2) + 1;

		    if (plane == PLANE_BOTH) {
			dst_portb = ((dst_xbport * 2) & 0xFF) | (((dst_xbport * 2 + 1) & 0xFF) << 8);
		    } else {
			/* This is an SFI_SCI link, odd channel can be plane A or B, but always destination odd entry set up */
			dst_portb = ((dst_xbport * 2 + 1) & 0xFF) | (((dst_xbport * 2 + 1) & 0xFF) << 8);

			if (plane == PLANE_A){
			  dst_port |= (0xff<<8); /* planeB not used - must point to 0xff */
			  dst_portb |= (0xff<<8);
			}else{
			  dst_port |= 0xff; /* planeA not used - must point to 0xff */
			  dst_portb |= 0xff; /* planeA not used - must point to 0xff */
			}
		    }
		    break;

		case bcmModuleProtocol4:
		    /* sequencing, first even, then odd */
		    src_port = src_xbport * 2;

		    if (dst_dual_channel_even) {
			dst_port = ((dst_xbport * 2) & 0xFF) | (((dst_xbport * 2) & 0xFF) << 8);
		    } else {
			dst_port = ((dst_xbport * 2 + 1) & 0xFF) | (((dst_xbport * 2 + 1) & 0xFF) << 8);
		    }

		    configure_both_channels = TRUE;
		    src_portb = src_xbport * 2 + 1;

		    if (dst_dual_channel_even) {
			dst_portb = ((dst_xbport * 2) & 0xFF) | (((dst_xbport * 2) & 0xFF) << 8); 
		    } else {
			dst_portb = ((dst_xbport * 2 + 1) & 0xFF) | (((dst_xbport * 2 + 1) & 0xFF) << 8);
		    }
		    break;
		default:
		    BCM_ERR(("ERROR: unsupported mode, Unit: %d srcNodeProtocol: 0x%x, destNodeProtocol: 0x%x)\n",
                             unit, SOC_SBX_STATE(unit)->stack_state->protocol[src_node],
			     SOC_SBX_STATE(unit)->stack_state->protocol[dst_node]));
		    return(BCM_E_PARAM);
	    }
	    break;
	case bcmModuleProtocol4:
	    switch (SOC_SBX_STATE(unit)->stack_state->protocol[dst_node]) {
		case bcmModuleProtocol1:
		case bcmModuleProtocol5:
		    if (src_dual_channel_even) {
			src_port = src_xbport * 2;
		    } else {
			src_port = src_xbport * 2 + 1;
		    }

		    dst_port = ((dst_xbport * 2) & 0xFF) | (((dst_xbport * 2) & 0xFF) << 8);
		    break;
		case bcmModuleProtocol2:
		    if (src_dual_channel_even) {
			src_port = src_xbport * 2;
		    } else {
			src_port = src_xbport * 2 + 1;
		    }

		    dst_port = ((dst_xbport * 2 + 1) & 0xFF) | (((dst_xbport * 2 + 1) & 0xFF) << 8);
		    break;
		case bcmModuleProtocol3:
		    src_port = src_xbport * 2;

		    if (dst_dual_channel_even) {
			dst_port = ((dst_xbport * 2) & 0xFF) | (((dst_xbport * 2) & 0xFF) << 8);
		    } else {
			dst_port = ((dst_xbport * 2 + 1) & 0xFF) | (((dst_xbport * 2 + 1) & 0xFF) << 8);
		    }

		    /* Set up XCFG for odd channel (plane B) just in case */
		    configure_both_channels = TRUE;
		    src_portb = (src_xbport * 2) + 1;
		    if (dst_dual_channel_even) {
			dst_port = ((dst_xbport * 2) & 0xFF) | (((dst_xbport * 2) & 0xFF) << 8);
		    } else {
			dst_port = ((dst_xbport * 2 + 1) & 0xFF) | (((dst_xbport * 2 + 1) & 0xFF) << 8);
		    }
		    break;
		case bcmModuleProtocol4:		    
		    src_port = src_xbport * 2;
		    dst_port = ((dst_xbport * 2) & 0xFF) | (((dst_xbport * 2) & 0xFF) << 8);
		    
		    configure_both_channels = TRUE;
		    src_portb = (src_xbport * 2) + 1;
		    dst_portb = ((dst_xbport * 2 + 1) & 0xFF) | (((dst_xbport * 2 + 1) & 0xFF) << 8);
		    break;
		default:
		    BCM_ERR(("ERROR: unsupported mode, Unit: %d srcNodeProtocol: 0x%x, destNodeProtocol: 0x%x)\n",
                             unit, SOC_SBX_STATE(unit)->stack_state->protocol[src_node],
			     SOC_SBX_STATE(unit)->stack_state->protocol[dst_node]));
		    return(BCM_E_PARAM);
	    }
	    break;
	default:
	    BCM_ERR(("ERROR: Unit %d, unsupported srcNodeProtocol: 0x%x)\n",
		     unit, SOC_SBX_STATE(unit)->stack_state->protocol[src_node]));
	    return(BCM_E_PARAM);
    }

    rv = soc_bm9600_XbXcfgRemapSelectWrite(unit, dst_phy_node, src_port, dst_port);
    if (rv != SOC_E_NONE) {
        soc_cm_debug(DK_ERR, "soc_bm9600_XbXcfgRemapSelectWrite failed on xbport %d, addr: %d select: %d value: 0x%x\n",
                              src_xbport, dst_phy_node, src_port, dst_port);
        return(BCM_E_FAIL);
    }

    if (configure_both_channels == TRUE) {
         rv = soc_bm9600_XbXcfgRemapSelectWrite(unit, dst_phy_node, src_portb, dst_portb);
         if (rv != SOC_E_NONE) {
             soc_cm_debug(DK_ERR, "soc_bm9600_XbXcfgRemapSelectWrite failed on xbport %d, addr: %d select: %d value: 0x%x\n",
                              src_xbport, dst_phy_node, src_portb, dst_portb);
             return(BCM_E_FAIL);
         }
    }

    return(BCM_E_NONE);
}

int
bcm_bm9600_fabric_crossbar_connection_get(int unit,
                                          int xbar, 
                                          int src_modid,
                                          bcm_port_t src_xbport,
                                          int dst_modid,
                                          bcm_port_t *dst_xbport)
{
    int rv;
    int dst_node, dst_phy_node;
    int src_node;
    bcm_port_t src_port, src_portb, dst_xbportb;
    int src_dual_channel_even = TRUE;
    static int first_call = TRUE;
    int arbiter_id, plane= PLANE_BOTH;
    int configure_both_channels = FALSE;

    src_portb = 0;

    if (!BCM_STK_MOD_IS_NODE(dst_modid)) {
        return BCM_E_PARAM;
    }
    dst_node = BCM_STK_MOD_TO_NODE(dst_modid);
    dst_phy_node = SOC_SBX_L2P_NODE(unit, dst_node);

    if (!BCM_STK_MOD_IS_NODE(src_modid)) {
        return(BCM_E_PARAM);
    }
    src_node = BCM_STK_MOD_TO_NODE(src_modid);

    /* Sequencing required for sfi_local channels, need to use 2 continguous lxbars   */
    /* so need to call back to back for A-A channels.  State is reserved in this case */
    if (first_call == FALSE) {
	if ((SOC_SBX_CFG_BM9600(unit)->uSerdesAbility[src_xbport] == BCM_PORT_ABILITY_DUAL_SFI) &&
	    (SOC_SBX_STATE(unit)->stack_state->protocol[src_node] == bcmModuleProtocol4)) {
	    src_dual_channel_even = FALSE;
	}
	first_call = TRUE;
    } else { /* First call=true */
	first_call = FALSE;
    }

    if (SOC_SBX_CFG_BM9600(unit)->uSerdesAbility[src_xbport] == BCM_PORT_ABILITY_SFI_SCI) {

        rv = bcm_fabric_control_get(unit, bcmFabricArbiterId, &arbiter_id);
	if (arbiter_id == 0) {
	    plane = PLANE_A;
	}else {
	    plane = PLANE_B;
	}
	/* And if protocol 4, always operate on the odd channel, there are not 2 logical crossbars for the sfi_sci link, only 1 */
	src_dual_channel_even = FALSE;
    }

    switch (SOC_SBX_STATE(unit)->stack_state->protocol[src_node]) {
	case bcmModuleProtocol1:
        case bcmModuleProtocol5:
	    switch (SOC_SBX_STATE(unit)->stack_state->protocol[dst_node]) {
		case bcmModuleProtocol1:
		case bcmModuleProtocol2:
		case bcmModuleProtocol3:
		case bcmModuleProtocol4:
		case bcmModuleProtocol5:
		    src_port = src_xbport * 2;
		    break;
		default:
		    BCM_ERR(("ERROR: unsupported mode, Unit: %d srcNodeProtocol: 0x%x, destNodeProtocol: 0x%x)\n",
                             unit, SOC_SBX_STATE(unit)->stack_state->protocol[src_node],
			     SOC_SBX_STATE(unit)->stack_state->protocol[dst_node]));
		    return(BCM_E_PARAM);
	    }
	    break;
	case bcmModuleProtocol2:
	    switch (SOC_SBX_STATE(unit)->stack_state->protocol[dst_node]) {
		case bcmModuleProtocol1:
		case bcmModuleProtocol2:
		case bcmModuleProtocol3:
		case bcmModuleProtocol5:
		case bcmModuleProtocol4:
		    src_port = (src_xbport * 2) + 1;
		    break;
		default:
		    BCM_ERR(("ERROR: unsupported mode, Unit: %d srcNodeProtocol: 0x%x, destNodeProtocol: 0x%x)\n",
                             unit, SOC_SBX_STATE(unit)->stack_state->protocol[src_node],
			     SOC_SBX_STATE(unit)->stack_state->protocol[dst_node]));
		    return(BCM_E_PARAM);
	    }
	    break;
	case bcmModuleProtocol3:
	    switch (SOC_SBX_STATE(unit)->stack_state->protocol[dst_node]) {
		case bcmModuleProtocol1:
	        case bcmModuleProtocol5:
		case bcmModuleProtocol2:
		case bcmModuleProtocol3:
		case bcmModuleProtocol4:
		    configure_both_channels = TRUE;
		    src_port = src_xbport * 2;
		    src_portb = src_xbport * 2 + 1;
		    break;
		default:
		    BCM_ERR(("ERROR: unsupported mode, Unit: %d srcNodeProtocol: 0x%x, destNodeProtocol: 0x%x)\n",
                             unit, SOC_SBX_STATE(unit)->stack_state->protocol[src_node],
			     SOC_SBX_STATE(unit)->stack_state->protocol[dst_node]));
		    return(BCM_E_PARAM);
	    }
	    break;

	case bcmModuleProtocol4:
	    switch (SOC_SBX_STATE(unit)->stack_state->protocol[dst_node]) {
		case bcmModuleProtocol1:
		case bcmModuleProtocol2:
	        case bcmModuleProtocol5:
		    if (src_dual_channel_even) {
			src_port = src_xbport * 2;
		    } else {
			src_port = src_xbport * 2 + 1;
		    }
		    break;
		case bcmModuleProtocol3:
		case bcmModuleProtocol4:
		    src_port = src_xbport * 2;
		    configure_both_channels = TRUE;
		    src_portb = (src_xbport * 2) + 1;
		    break;
		default:
		    BCM_ERR(("ERROR: unsupported mode, Unit: %d srcNodeProtocol: 0x%x, destNodeProtocol: 0x%x)\n",
                             unit, SOC_SBX_STATE(unit)->stack_state->protocol[src_node],
			     SOC_SBX_STATE(unit)->stack_state->protocol[dst_node]));
		    return(BCM_E_PARAM);
	    }
	    break;
	default:
	    BCM_ERR(("ERROR: Unit %d, unsupported srcNodeProtocol: 0x%x)\n",
		     unit, SOC_SBX_STATE(unit)->stack_state->protocol[src_node]));
	    return(BCM_E_PARAM);
    }

    /* Read the first source */
    rv = soc_bm9600_XbXcfgRemapSelectRead(unit, dst_phy_node, src_port, (uint32*)dst_xbport);
    if (rv != SOC_E_NONE) {
      soc_cm_debug(DK_ERR, "soc_bm9600_XbXcfgRemapSelectRead failed on xbport %d\n", src_xbport);
      return BCM_E_FAIL;
    }

    if (plane == PLANE_B) {
	(*dst_xbport) = ((*dst_xbport) & (0xFF << 8)) >> 8;
    } else {
	(*dst_xbport) = (*dst_xbport) & 0xFF;
    }
    if ((*dst_xbport) & 0x1) {
        (*dst_xbport) &= ~(0x1);
    }
    *dst_xbport /= 2;

    if (configure_both_channels == TRUE) {
	rv = soc_bm9600_XbXcfgRemapSelectRead(unit, dst_phy_node, src_portb, (uint32*)&dst_xbportb);
	if (rv != SOC_E_NONE) {
	    soc_cm_debug(DK_ERR, "soc_bm9600_XbXcfgRemapSelectRead failed on xbport %d\n", src_portb);
	    return BCM_E_FAIL;
	}

	(dst_xbportb) = (dst_xbportb) & 0xFF;
	if ((dst_xbportb) & 0x1) {
	    (dst_xbportb) &= ~(0x1);
	}
	dst_xbportb /= 2;

#if 0
	soc_cm_debug(DK_VERBOSE, "second channel dst_node(%d) src_port(%d) read dst_port(0x%x)\n",
		     dst_phy_node, src_portb, (int) dst_xbportb);
#endif

    }

    return BCM_E_NONE;
}

int
bcm_bm9600_fabric_calendar_active(int unit)
{
    int rv = BCM_E_NONE;
    uint32 uData;

    /* Swap active calendar */
    uData = SAND_HAL_READ(unit, PL, BW_GROUP_CONFIG);
    if (uData & 1) {
        uData = uData & 0xffffffe;
    }else {
        uData = uData | 1;
    }
    SAND_HAL_WRITE(unit, PL, BW_GROUP_CONFIG, uData);
    return rv;
}

int
bcm_bm9600_fabric_crossbar_mapping_set(int unit,
                                       int modid,
                                       int switch_fabric_arbiter_id, 
                                       int xbar,
                                       bcm_port_t port)
{
 
    return BCM_E_NONE;
}

int
bcm_bm9600_fabric_crossbar_mapping_get(int unit,
                                       int modid,
                                       int switch_fabric_arbiter_id, 
                                       int xbar,
                                       bcm_port_t *port)
{
    return BCM_E_UNAVAIL;
}

int
bcm_bm9600_fabric_crossbar_enable_set(int unit,
                                      uint64 xbars)
{
    uint32 i;
    int32  nTsSizeNormNs;
    int32  nSfiCount;
    int32  nHalfBus;
    int32  nOldTsSizeNormNs;
    int32  bw_group;
    uint32 num_queues_in_bag, base_queue, bag_rate_bytes_per_epoch;
    int32  nOldDemandScale, nDemandScale;
    uint32 gamma = 0, sigma = 0, new_sigma = 0;
    int32  queue;
    int    rv = BCM_E_NONE;
    bcm_sbx_cosq_queue_state_t *p_qstate;
    bcm_sbx_cosq_bw_group_state_t *p_bwstate;
    bcm_sbx_cosq_bw_group_state_t *p_bwstate_start;
    uint64 uu_epoch_length_in_ns = 0;
    uint64 uu_bag_rate_in_bytes_per_sec;
    int32 bag_rate_kbps;
    uint32 new_bag_rate_bytes_per_epoch = 0;
    uint32 guarantee_in_kbps = 0;
    uint64 uu_guarantee_in_bytes_per_sec;
    uint32 guarantee_in_bytes_per_epoch = 0;

    
    nHalfBus = SOC_SBX_CFG(unit)->bHalfBus;
    /* set the link enables on the BME */
    nSfiCount = 0;
    for (i=0; i<64; i++) {
        /* Count number of enabled links */
        nSfiCount += ((xbars >> i) & 0x1);
    }

    nTsSizeNormNs = soc_sbx_fabric_get_timeslot_size(unit, nSfiCount, nHalfBus, soc_feature(unit, soc_feature_hybrid) );

    nOldTsSizeNormNs = SOC_SBX_STATE(unit)->fabric_state->timeslot_size;
    SOC_SBX_STATE(unit)->fabric_state->timeslot_size = nTsSizeNormNs;

    /* Force null grants */
    SAND_HAL_RMW_FIELD(unit, PL, FO_CONFIG0, FORCE_NULL_GRANT, 1);
    /* configure the fabric data plane */
    SAND_HAL_RMW_FIELD(unit, PL, FO_CONFIG3, LINK_ENABLE, xbars);
    /* And re-enable */
    SAND_HAL_RMW_FIELD(unit, PL, FO_CONFIG0, FORCE_NULL_GRANT, 0);

    /* reconfigure bags */
    nOldDemandScale = SOC_SBX_STATE(unit)->fabric_state->old_demand_scale;
    nDemandScale = SOC_SBX_CFG(unit)->demand_scale;

    BCM_DEBUG(BCM_DBG_COSQ, ("%s Unit(%d), DemandScale: %d\n", FUNCTION_NAME(), unit,
                                                      SOC_SBX_CFG(unit)->demand_scale));

    if ( (nOldTsSizeNormNs != nTsSizeNormNs) || (nOldDemandScale != nDemandScale) ) {
	/* go through all bags and reconfig all non-zero bag rate */
	p_bwstate_start = (bcm_sbx_cosq_bw_group_state_t*)SOC_SBX_STATE(unit)->bw_group_state;

        uu_epoch_length_in_ns = ((SOC_SBX_CFG(unit)->epoch_length_in_timeslots) * (nTsSizeNormNs));
	p_qstate = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;
	
	for (bw_group = 0; bw_group < SOC_SBX_CFG(unit)->num_bw_groups; bw_group++) {
	    p_bwstate = &p_bwstate_start[bw_group];
	    
	    if ( ( p_bwstate->in_use == FALSE ) || (p_bwstate->path.bag_rate_kbps == 0) ) {
		continue;
	    }

            bag_rate_kbps = p_bwstate->path.bag_rate_kbps;
            uu_bag_rate_in_bytes_per_sec = bag_rate_kbps / 8;
            if (soc_sbx_div64((uu_bag_rate_in_bytes_per_sec * uu_epoch_length_in_ns), 1000000, &new_bag_rate_bytes_per_epoch) == -1) {
                BCM_ERR(("ERROR: update BAG rate per epoch failed\n"));
                return(BCM_E_INTERNAL);
            }
            new_bag_rate_bytes_per_epoch >>= nDemandScale;

	    rv = soc_bm9600_bag_read(unit, bw_group, &num_queues_in_bag,
				     &base_queue, &bag_rate_bytes_per_epoch);
	    if (rv != SOC_E_NONE) {
		BCM_ERR(("ERROR: BM9600 read of PRT table failed for bw_group(%d)\n", bw_group));
		return(BCM_E_FAIL);
	    }

	    rv = soc_bm9600_bag_write(unit, bw_group, num_queues_in_bag,
				      base_queue, new_bag_rate_bytes_per_epoch);
	    if (rv != SOC_E_NONE) {
		BCM_ERR(("ERROR: BM9600 Write to PRT table failed for bw_group(%d)\n", bw_group));
		return(BCM_E_FAIL);
	    }

            base_queue = p_bwstate->base_queue;
            for (queue = base_queue; queue < (base_queue + p_bwstate->num_cos); queue++) {
                new_sigma = 0;
		rv = soc_bm9600_bwp_read(unit, queue, &gamma, &sigma);
		if (rv != SOC_E_NONE) {
		    BCM_ERR(("ERROR: BM9600 Read to BWP table failed for queue(%d)\n", queue));
		    return(BCM_E_FAIL);
		}

                if (!( (p_qstate[queue].ingress.bw_mode == BCM_COSQ_SP) ||
                        (p_qstate[queue].ingress.bw_mode == BCM_COSQ_AF) ||
                        (p_qstate[queue].ingress.bw_mode == BCM_COSQ_EF) ||
                        (p_qstate[queue].ingress.bw_mode == BCM_COSQ_GSP0) ||
                        (p_qstate[queue].ingress.bw_mode == BCM_COSQ_GSP1) ||
                        (p_qstate[queue].ingress.bw_mode == BCM_COSQ_GSP3) ||
                        (p_qstate[queue].ingress.bw_mode == BCM_COSQ_GSP4) ||
                        (p_qstate[queue].ingress.bw_mode == BCM_COSQ_GSP5) ||
                        (p_qstate[queue].ingress.bw_mode == BCM_COSQ_GSP6) ||
                        (p_qstate[queue].ingress.bw_mode == BCM_COSQ_GSP7) )) {
                    continue;
                }

                if (p_qstate[queue].ingress.bw_mode == BCM_COSQ_SP) {
                    new_sigma = (int32_t) new_bag_rate_bytes_per_epoch;
                }

                if ( (p_qstate[queue].ingress.bw_mode == BCM_COSQ_GSP0) ||
                     (p_qstate[queue].ingress.bw_mode == BCM_COSQ_GSP1) ||
                     (p_qstate[queue].ingress.bw_mode == BCM_COSQ_GSP2) ||
                     (p_qstate[queue].ingress.bw_mode == BCM_COSQ_GSP3) ||
                     (p_qstate[queue].ingress.bw_mode == BCM_COSQ_GSP4) ||
                     (p_qstate[queue].ingress.bw_mode == BCM_COSQ_GSP5) ||
                     (p_qstate[queue].ingress.bw_mode == BCM_COSQ_GSP6) ||
                     (p_qstate[queue].ingress.bw_mode == BCM_COSQ_GSP7) ) {
                    new_sigma = (int32_t) new_bag_rate_bytes_per_epoch;
                }

                if ( (p_qstate[queue].ingress.bw_mode == BCM_COSQ_AF) ||
                     (p_qstate[queue].ingress.bw_mode == BCM_COSQ_EF) ) {

                    guarantee_in_kbps = bcm_sbx_cosq_get_bw_guarantee(unit, queue);
                    uu_guarantee_in_bytes_per_sec = guarantee_in_kbps / 8;

                    if (soc_sbx_div64((uu_guarantee_in_bytes_per_sec * uu_epoch_length_in_ns), 1000000, &guarantee_in_bytes_per_epoch) == -1) {
                      BCM_ERR(("ERROR: update Guarantee failed\n"));
                      return(BCM_E_INTERNAL);
                    }
                    guarantee_in_bytes_per_epoch >>= nDemandScale;

                    new_sigma = (int32_t) guarantee_in_bytes_per_epoch;

                    BCM_DEBUG(BCM_DBG_COSQ | BCM_DBG_VERBOSE, ("queue(%d) guarantee in bytes/epoch(%d) guarantee_in_bytes_per_msec(%lld) epoch_length_in_ns(%lld)\n",
                                     queue, guarantee_in_bytes_per_epoch, uu_guarantee_in_bytes_per_sec, uu_epoch_length_in_ns));
                }

		rv = soc_bm9600_bwp_write(unit, queue, gamma, new_sigma);
		if (rv != SOC_E_NONE) {
		    BCM_ERR(("ERROR: BM9600 Write to BWP table failed for queue(%d)\n", queue));
		    return(BCM_E_FAIL);
		}
            }
	}
    }

    return BCM_E_NONE;
}

int
bcm_bm9600_fabric_crossbar_enable_get(int unit,
                                      uint64 *xbars)
{
    uint32 uData;

    uData = SAND_HAL_READ(unit, PL, FO_CONFIG3);
    *xbars = SAND_HAL_GET_FIELD(PL, FO_CONFIG3, LINK_ENABLE, uData);

    return BCM_E_NONE;
}

int
bcm_bm9600_fabric_crossbar_status_get(int unit,
                                      uint64 *xbars)
{
    uint32 uData;

    uData = SAND_HAL_READ(unit, PL, FO_STATUS1);
    *xbars = SAND_HAL_GET_FIELD(PL, FO_STATUS1, GLOBAL_ENABLED_LINKS, uData);

    return BCM_E_NONE;
}

int
bcm_bm9600_fabric_distribution_create(int unit,
                                      bcm_fabric_distribution_t *ds_id)
{
    int rv = BCM_E_UNAVAIL;


    return rv;
}

int
bcm_bm9600_fabric_distribution_destroy(int unit,
                                       bcm_fabric_distribution_t  ds_id)
{
    int rv = BCM_E_UNAVAIL;
    bcm_fabric_distribution_t nef_ds_id;

    rv = soc_bm9600_eset_set(unit, ds_id, 0, 0, BCM_FABRIC_DISTRIBUTION_SCHED_ALL, 0);
    if (rv != SOC_E_NONE) {
        return(rv);
    }

    if (soc_bm9600_features(unit, soc_feature_egr_multicast_independent_fc)) {
        nef_ds_id = BCM_INT_SBX_COSQ_ESET_TO_NEF_ESET(unit, ds_id);
        rv = soc_bm9600_eset_set(unit, nef_ds_id, 0, 0, BCM_FABRIC_DISTRIBUTION_SCHED_ALL, 1);
        if (rv != BCM_E_NONE) {
            return(rv);
        }
    }

    return rv;
}

int
bcm_bm9600_fabric_distribution_set(int unit,
                                   bcm_fabric_distribution_t  ds_id,
                                   int modid_count,
                                   int *dist_modids,
				   int mc_full_eval_min)
{
    int rv = BCM_E_UNAVAIL;
    int i;
    uint64 low_eset_value;
    uint32 hi_eset_value;
    int    node;
    uint64 mask = 1;
    bcm_fabric_distribution_t nef_ds_id;


    for (i = 0, low_eset_value = 0, hi_eset_value = 0; i < modid_count; i++) {
        node = BCM_STK_MOD_TO_NODE((*(dist_modids + i)));
        if (node <= SB_FAN_DEVICE_BM9600_MAX_NODE_LOW_ESET_FLD) {
            low_eset_value |= (mask << node);
        }
        else {
            hi_eset_value |= (1 << (node - SB_FAN_DEVICE_BM9600_MAX_NODE_LOW_ESET_FLD - 1));
        }
    }

    rv = soc_bm9600_eset_set(unit, ds_id, low_eset_value, hi_eset_value, mc_full_eval_min, 0);
    if (rv != BCM_E_NONE) {
        return(rv);
    }

    if (soc_bm9600_features(unit, soc_feature_egr_multicast_independent_fc)) {
        nef_ds_id = BCM_INT_SBX_COSQ_ESET_TO_NEF_ESET(unit, ds_id);
        rv = soc_bm9600_eset_set(unit, nef_ds_id, low_eset_value, hi_eset_value, mc_full_eval_min, 1);
        if (rv != BCM_E_NONE) {
            return(rv);
        }
    }

    return rv;
}

int
bcm_bm9600_fabric_distribution_get(int unit,
                                   bcm_fabric_distribution_t  ds_id,
                                   int max_count,
                                   int *dist_modids,
                                   int *count)
{
    int rv = BCM_E_UNAVAIL;
    int node, num_members;
    uint64 low_eset_value;
    uint32 hi_eset_value;
    uint32 mc_full_eval_min;
    uint32 eset_full_status_mode;

    rv = soc_bm9600_eset_get(unit, ds_id, &low_eset_value, &hi_eset_value, &mc_full_eval_min, &eset_full_status_mode);
    if (rv != BCM_E_NONE) {
        return(rv);
    }

    /* It is the BCM layer resonsibility to convert node to modId */
    for (node = 0, num_members = 0; ((node < SB_FAB_DEVICE_BM9600_MAX_NODES) & (num_members < max_count)); node++) {
        if (node <= SB_FAN_DEVICE_BM9600_MAX_NODE_LOW_ESET_FLD) {
            if (low_eset_value & (1 << node)) {
                *(dist_modids + num_members++) = node;
            }
        }
        else {
            if (hi_eset_value & (1 << (node - SB_FAN_DEVICE_BM9600_MAX_NODE_LOW_ESET_FLD))) {
                *(dist_modids + num_members++) = node;
            }
        }
    }
    *count = num_members;

    return rv;
}

int
bcm_bm9600_fabric_distribution_control_set(int unit,
					   bcm_fabric_distribution_t ds_id,
					   bcm_fabric_distribution_control_t type,
					   int value)
{
    int rv = BCM_E_NONE;
    uint64 low_eset_value;
    uint32 hi_eset_value;
    uint32 mc_full_eval_min;
    bcm_fabric_distribution_t nef_ds_id;
    uint32 eset_full_status_mode;

    rv = soc_bm9600_eset_get(unit, ds_id, &low_eset_value, &hi_eset_value, &mc_full_eval_min, &eset_full_status_mode);
    if (rv != BCM_E_NONE) {
        return(rv);
    }
    mc_full_eval_min = value;

    rv = soc_bm9600_eset_set(unit, ds_id, low_eset_value, hi_eset_value, mc_full_eval_min, 0);
    if (rv != BCM_E_NONE) {
        return(rv);
    }

    if (soc_bm9600_features(unit, soc_feature_egr_multicast_independent_fc)) {
        nef_ds_id = BCM_INT_SBX_COSQ_ESET_TO_NEF_ESET(unit, ds_id);
        rv = soc_bm9600_eset_set(unit, nef_ds_id, low_eset_value, hi_eset_value, mc_full_eval_min, 1);
        if (rv != BCM_E_NONE) {
            return(rv);
        }
    }

    return rv;
}
int
bcm_bm9600_fabric_distribution_control_get(int unit,
					   bcm_fabric_distribution_t ds_id,
					   bcm_fabric_distribution_control_t type,
					   int *value)
{
    int rv = BCM_E_NONE;
    uint64 low_eset_value;
    uint32 hi_eset_value;
    uint32 mc_full_eval_min;
    uint32 eset_full_status_mode;

    rv = soc_bm9600_eset_get(unit, ds_id, &low_eset_value, &hi_eset_value, &mc_full_eval_min, &eset_full_status_mode);
    if (rv != BCM_E_NONE) {
        return(rv);
    }

    return rv;
}

int
bcm_bm9600_fabric_control_set(int unit, 
                              bcm_fabric_control_t type,
                              int arg)
{
    int bEnableAutoFailover;
    int bEnableAutoLinkDisable;
    int bEnableUseGlobalLink;
    uint32 uData;
    int rv = BCM_E_NONE;
    int node, modid;

    switch (type) {
        case bcmFabricArbiterId:
            uData = SAND_HAL_READ(unit, PL, FO_CONFIG0);
            uData = SAND_HAL_MOD_FIELD(PL, FO_CONFIG0, LOCAL_BM_ID, uData, arg);
            SAND_HAL_WRITE(unit, PL, FO_CONFIG0, uData);
            break;
        case bcmFabricActiveArbiterId:
            uData = SAND_HAL_READ(unit, PL, FO_CONFIG0);
            uData = SAND_HAL_MOD_FIELD(PL, FO_CONFIG0, DEFAULT_BM_ID, uData, arg);
            SAND_HAL_WRITE(unit, PL, FO_CONFIG0, uData);            
            break;
        case bcmFabricArbiterConfig:
            SAND_HAL_RMW_FIELD(unit, PL, FO_CONFIG0, FORCE_NULL_GRANT, (arg == 0));
            break;
        case bcmFabricMaximumFailedLinks:
            /* update the max failed link configuration */
            uData = SAND_HAL_READ(unit, PL, FO_CONFIG0);
            uData = SAND_HAL_MOD_FIELD(PL, FO_CONFIG0, MAX_DIS_LINKS, uData, arg);
            SAND_HAL_WRITE(unit, PL, FO_CONFIG0, uData);

            SOC_SBX_CFG(unit)->uMaxFailedLinks = arg;

	    /* Unlike Petronius, I believe we don't need to change the degraded timeslot size */

            break;
        case bcmFabricManager:
	    switch (arg) {
		case bcmFabricModeArbiterCrossbar:
		    if (soc_feature(unit, soc_feature_arbiter_capable)) {
			/* Handle arbiter crossbar mode here - reinitialize BM9600 without affecting Crossbar links */
			SOC_FEATURE_SET(unit, soc_feature_arbiter_capable);
			SOC_FEATURE_SET(unit, soc_feature_arbiter);
			SOC_SBX_CFG_BM9600(unit)->uDeviceMode = SOC_SBX_BME_ARBITER_XBAR_MODE;
			SOC_SBX_CFG_BM9600(unit)->bElectArbiterReconfig = TRUE;
			soc_cm_print("arbiter capable crossbar, setting device mode to %d\n", SOC_SBX_CFG_BM9600(unit)->uDeviceMode);

			for (node = 0; node < SBX_MAXIMUM_NODES; node++) {
				
			    modid = soc_property_port_get(unit, node, spn_SCI_PORT_MODID,
							  (BCM_MODULE_FABRIC_BASE + SOC_SBX_CFG(unit)->p2l_node[node]));
				
			    if (modid != (BCM_MODULE_FABRIC_BASE + SOC_SBX_CFG(unit)->p2l_node[node])) {

				SOC_SBX_CFG(unit)->l2p_node[BCM_STK_MOD_TO_NODE(modid)] = node;
				SOC_SBX_CFG(unit)->p2l_node[node] = BCM_STK_MOD_TO_NODE(modid);
				
				if (SOC_SBX_CFG(unit)->p2l_node[BCM_STK_MOD_TO_NODE(modid)]
				    == BCM_STK_MOD_TO_NODE(modid)) {
				    SOC_SBX_CFG(unit)->p2l_node[BCM_STK_MOD_TO_NODE(modid)] = SBX_MAXIMUM_NODES - 1;
				}
			    }
			}
			rv = soc_bm9600_init(unit, SOC_SBX_CFG(unit));

		    } else {
			BCM_ERR(("Unit(%d) BM9600 is not capable of converting to an arbiter/crossbar device\n", unit));
			return BCM_E_UNAVAIL;
		    }
		    break;
		case  bcmFabricModeCrossbar:
		    /* Handle crossbar only mode here */
		    SOC_SBX_CFG_BM9600(unit)->uDeviceMode = SOC_SBX_BME_ARBITER_XBAR_MODE;
		    SOC_SBX_CFG_BM9600(unit)->bElectArbiterReconfig = TRUE;
		    rv = soc_bm9600_init(unit, SOC_SBX_CFG(unit));
		    break;
		    /* currently unsupported mode */
		case bcmFabricModeArbiterCapableCrossbar:
		default:
		    BCM_ERR(("Unit(%d) bcm_fabric_control_set type bcmFabricManager, unsupported mode\n", unit));
		    rv = BCM_E_PARAM;
		    break;
	    }
	    break;
        case bcmFabricRedundancyMode:
	    /* clear interrupts */
	    uData = (SAND_HAL_PL_FO_EVENT_AUTO_FAILOVER_MASK |
		     SAND_HAL_PL_FO_EVENT_AUTO_QE_DIS_MASK |
		     SAND_HAL_PL_FO_EVENT_AUTO_LINK_DIS_MASK);
	    SAND_HAL_WRITE(unit, PL, FO_EVENT, uData);

	    /* configure redundancy mode */
            bEnableAutoFailover = 0;
            bEnableAutoLinkDisable = 0;
            bEnableUseGlobalLink = 0;
            switch (arg) {
                case bcmFabricRed1Plus1Both:
                    bEnableAutoFailover = 1;
                    break;
                case bcmFabricRed1Plus1LS:
                    bEnableUseGlobalLink = 1; /* intentional fall through */
                case bcmFabricRed1Plus1ELS:
                    bEnableAutoFailover = 1;
                    bEnableAutoLinkDisable = 1;
                    break;
                case bcmFabricRedLS:
                    bEnableUseGlobalLink = 1; /* intentional fall through */
                case bcmFabricRedELS:
                    bEnableAutoLinkDisable = 1;
                    break;
                case bcmFabricRedManual:
                    break;
                default:
                    soc_cm_debug(DK_ERR, "bcmFabricRedundancyMode %d not supported by bm9600\n", arg);
                    return BCM_E_PARAM;
            }
            uData = SAND_HAL_READ(unit, PL, FO_CONFIG0);

            /* Update h/w configuration for node failure detection */
            if ( (arg == bcmFabricRed1Plus1ELS) || (arg == bcmFabricRedELS) ) {
                uData = SAND_HAL_MOD_FIELD(PL, FO_CONFIG0, MAX_DIS_LINKS, uData, 24);
            }
            else {
                uData = SAND_HAL_MOD_FIELD(PL, FO_CONFIG0, MAX_DIS_LINKS, uData, SOC_SBX_CFG(unit)->uMaxFailedLinks);
            }

            uData = SAND_HAL_MOD_FIELD(PL, FO_CONFIG0, AUTO_FAILOVER_ENABLE, uData, bEnableAutoFailover);
            uData = SAND_HAL_MOD_FIELD(PL, FO_CONFIG0, USE_GLOBAL_LINK_ENABLE, uData, bEnableUseGlobalLink);
            uData = SAND_HAL_MOD_FIELD(PL, FO_CONFIG0, AUTO_LINK_DIS_ENABLE, uData, bEnableAutoLinkDisable);
            SAND_HAL_WRITE(unit, PL, FO_CONFIG0, uData);

	    /* arm interrupts */
	    uData = 0xFFFFFFFF;
	    if (bEnableAutoFailover) {
		uData &= ~(SAND_HAL_PL_FO_EVENT_MASK_AUTO_FAILOVER_DISINT_MASK);
	    }
	    if (bEnableAutoLinkDisable) {
		uData &= ~(SAND_HAL_PL_FO_EVENT_MASK_AUTO_LINK_DIS_DISINT_MASK);
		uData &= ~(SAND_HAL_PL_FO_EVENT_MASK_AUTO_QE_DIS_DISINT_MASK);
	    }
	    SAND_HAL_WRITE(unit, PL, FO_EVENT_MASK, uData);

            break;

        case bcmFabricArbitrationMapFabric:
        case bcmFabricArbitrationMapSubscriber:
        case bcmFabricArbitrationMapHierarchicalSubscriber:
            return(BCM_E_UNAVAIL);
            break;

        case bcmFabricQueueMin:
            if (arg) {
                BCM_ERR(("Unit %d: Invalid value specified (%d) for "
                         "bcmFabricQueueMin. Only 0 is supported. \n", unit, arg));
                rv = BCM_E_PARAM;
                break;
            } 
            /* nothing to do. Already set to 0. */
            break;
        case bcmFabricQueueMax:
            if ((arg < 0) || (arg > BM9600_BW_MAX_VOQ_NUM)) {
                BCM_ERR(("%d: Invalid Queue Max (%d) specified. Valid range: "
                         "0-%d \n", unit, arg, BM9600_BW_MAX_VOQ_NUM));
                rv = BCM_E_PARAM;
                break;
            }
            rv = soc_bm9600_epoch_in_timeslot_config_get(unit, arg, &uData);
            if (rv == SOC_E_NONE) {
                SOC_SBX_CFG(unit)->num_queues = arg;
                SOC_SBX_CFG(unit)->epoch_length_in_timeslots = uData;

                /* Adjust the sw state to reflect this new settings */
                rv = _bcm_sbx_cosq_queue_regions_set(unit);
                if (rv != BCM_E_NONE) {
                    BCM_ERR(("%d: Could not set queue_regions to reflect new "
                             "Max VOQs (%d) \n", unit, arg));
                    break;
                }

                /* set the HW config registers */
                uData = SAND_HAL_READ(unit, PL, BW_EPOCH_CONFIG);
                uData = SAND_HAL_MOD_FIELD(PL, BW_EPOCH_CONFIG, NUM_TIMESLOTS, 
                          uData, SOC_SBX_CFG(unit)->epoch_length_in_timeslots);
                SAND_HAL_WRITE(unit, PL, BW_EPOCH_CONFIG, uData);

                uData = SAND_HAL_READ(unit, PL, BW_GROUP_CONFIG);
                uData = SAND_HAL_MOD_FIELD(PL, BW_GROUP_CONFIG, NUM_GROUPS, 
                          uData, (SOC_SBX_CFG(unit)->num_queues -1));
                SAND_HAL_WRITE(unit, PL, BW_GROUP_CONFIG, uData);
            } else {
                BCM_ERR(("%d: Could not calculate Epoch length for specified "
                         "bcmFabricQueueMax value (%d) \n", unit, arg));
                break;
            }
            break;
        case bcmFabricEgressQueueMin: /* intentional fall thru */
        case bcmFabricEgressQueueMax:
	case bcmFabricDemandCalculationEnable:
            rv = BCM_E_UNAVAIL;
            break;
        case bcmFabricOperatingIntervalEnable:
	    /* Enable or disable on standby arbiter */
	    uData = SAND_HAL_READ(unit, PL, BW_EPOCH_CONFIG);
	    
	    if (arg == TRUE) {
		uData = SAND_HAL_MOD_FIELD(PL, BW_EPOCH_CONFIG, NUM_TIMESLOTS, 
					   uData, SOC_SBX_CFG(unit)->epoch_length_in_timeslots);
	    } else {
		uData = SAND_HAL_MOD_FIELD(PL, BW_EPOCH_CONFIG, NUM_TIMESLOTS, 
					   uData, 0);

	    }
	    SAND_HAL_WRITE(unit, PL, BW_EPOCH_CONFIG, uData);
	    break;

        default:
            BCM_ERR(("Unit %d: Unsupported fabric control type(%d) "
                     "specified \n", unit, type));
            rv = BCM_E_PARAM;
    }

    return rv;
}

int
bcm_bm9600_fabric_control_get(int unit, 
                              bcm_fabric_control_t type,
                              int *arg)
{
    int     rv = BCM_E_NONE;
    uint32  uData;

    switch (type) {
        case bcmFabricArbiterId:
            uData = SAND_HAL_READ(unit, PL, FO_CONFIG0);
            *arg = SAND_HAL_GET_FIELD(PL, FO_CONFIG0, LOCAL_BM_ID, uData);
            break;
        case bcmFabricActiveArbiterId:
            uData = SAND_HAL_READ(unit, PL, FO_CONFIG0);
            *arg = SAND_HAL_GET_FIELD(PL, FO_CONFIG0, DEFAULT_BM_ID, uData);
            break;
        case bcmFabricArbiterConfig:
            uData = SAND_HAL_READ(unit, PL, FO_CONFIG0);
            *arg = (SAND_HAL_GET_FIELD(PL, FO_CONFIG0, FORCE_NULL_GRANT, uData) == 0);
            break;
        case bcmFabricMaximumFailedLinks:
            uData = SAND_HAL_READ(unit, PL, FO_CONFIG0);
            *arg = SAND_HAL_GET_FIELD(PL, FO_CONFIG0, MAX_DIS_LINKS, uData);
            break;
        case bcmFabricActiveId:
            uData = SAND_HAL_READ(unit, PL, FO_STATUS0);
            *arg = SAND_HAL_GET_FIELD(PL, FO_STATUS0, ACTIVE_BM, uData);     
            break;
        case bcmFabricRedundancyMode:
            /* return cached value */
            *arg = SOC_SBX_CFG(unit)->uRedMode;
            break;

        case bcmFabricArbitrationMapFabric:
        case bcmFabricArbitrationMapSubscriber:
        case bcmFabricArbitrationMapHierarchicalSubscriber:
            rv = BCM_E_UNAVAIL;
            break;

        case bcmFabricQueueMin:
            *arg = 0; /* always starts at 0 */
            break;
        case bcmFabricQueueMax:
            *arg = SOC_SBX_CFG(unit)->num_queues;
            break;
        case bcmFabricEgressQueueMin: /* intentional fall thru */
        case bcmFabricEgressQueueMax:
            rv = BCM_E_UNAVAIL;
            break;

        default:
            rv = BCM_E_PARAM;
    }

    return rv;
}


/* CHANGES, START */

#if 0
int
bcm_bm9600_lcm_crossbar_connection_set(int unit,
                                          int xbar,
                                          int src_modid,
                                          bcm_port_t src_xbport,
                                          int dst_porta,
                                          int dst_portb)
{
    int rv;
    uint32 remap_entry;

    /* The bcm_port_t is a physical port, which is split into two logical ports. We map both
     * logical ports to the equivalent logical ports on the other side of the crossbar.
     */
    remap_entry = ((dst_portb & 0xff) << 8) | (dst_porta & 0xff);
    rv = soc_bm9600_XbXcfgRemapSelectWrite(unit, 71, src_xbport * 2, remap_entry);
    if (rv != SOC_E_NONE) {
      soc_cm_debug(DK_ERR, "soc_bm9600_XbXcfgRemapSelectWrite failed on xbport %d\n", src_xbport);
      return BCM_E_FAIL;
    }
    return BCM_E_NONE;
}

int
bcm_bm9600_lcm_crossbar_connection_get(int unit,
                                          int xbar,
                                          int src_modid,
                                          bcm_port_t src_xbport,
                                          int dst_modid,
                                          bcm_port_t *dst_xbport)
{
    int rv;

    if (!BCM_STK_MOD_IS_NODE(dst_modid)) {
        return BCM_E_PARAM;
    }

    /* The two logical ports should be mapped near-identically, so we just read the A grant */
    rv = soc_bm9600_XbXcfgRemapSelectRead(unit, BCM_STK_MOD_TO_NODE(dst_modid),
                                          src_xbport * 2, (uint32*)dst_xbport);
    *dst_xbport /= 2;
    if (rv != SOC_E_NONE) {
      soc_cm_debug(DK_ERR, "soc_bm9600_XbXcfgRemapSelectRead failed on xbport %d\n", src_xbport);
      return BCM_E_FAIL;
    }
    return BCM_E_NONE;
}
#endif /* 0 */

/* CHANGES, END */
