/*
 * $Id: link.c 1.4 Broadcom SDK $
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
 * QE/BME implementation of Link Scan callbacks for SFI, SCI and SPI ports
 */

#include <sal/types.h>

#include <soc/linkctrl.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_ca_auto.h>

#include <bcm/error.h>
#include <bcm/link.h>
#include <bcm_int/sbx/lock.h>
#include <bcm_int/common/link.h>
#include <bcm_int/sbx/bm9600.h>
#include <bcm_int/sbx/qe2000.h>
/*
 * SBX driver routines for linkscan module
 */
static _bcm_ls_driver_t    _bcm_ls_driver_sbx;

extern int
_bcm_sbx_port_link_get(int unit, bcm_port_t port, int hw, int *up);

/*
 * Function:
 *     _bcm_sbx_link_hw_interrupt
 * Purpose:
 *     Routine handler for hardware linkscan interrupt.
 * Parameters:
 *     unit - Device unit number
 *     pbmp - (OUT) Returns bitmap of ports that require hardware re-scan
 */
STATIC void
_bcm_sbx_link_hw_interrupt(int unit, bcm_pbmp_t *pbmp)
{

}

/*
 * Function:
 *     _bcm_sbx_link_port_link_get
 * Purpose:
 *     Return current PHY up/down status.
 * Parameters:
 *     unit - Device unit number
 *     port - Device port number
 *     hw   - If TRUE, assume hardware linkscan is active and use it
 *              to reduce PHY reads.
 *            If FALSE, do not use information from hardware linkscan.
 *     up   - (OUT) TRUE for link up, FALSE for link down.
 * Returns:
 *     BCM_E_NONE
 *     BCM_E_XXX
 */
STATIC int
_bcm_sbx_link_port_link_get(int unit, bcm_port_t port, int hw, int *up)
{
    if (SOC_IS_SBX_SIRIUS(unit)) {
        return (_bcm_sbx_port_link_get(unit, port, hw, up));
    } else if (SOC_IS_SBX_QE2000(unit)) {
        return bcm_qe2000_port_link_get(unit, port, up);
    } else if (SOC_IS_SBX_BM9600(unit)) {
        return bcm_bm9600_port_link_get(unit, port, up);
    } else {
        return bcm_port_link_status_get(unit, port, up);
    }
}


/*
 * Function:
 *     _bcm_sbx_link_internal_select
 * Purpose:
 *     Select the source of the CMIC link status interrupt
 *     to be the Internal Serdes on given port.
 * Parameters:
 *     unit - Device unit number
 *     port - Device port number
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_bcm_sbx_link_internal_select(int unit, bcm_port_t port)
{
    if (SOC_IS_SBX_SIRIUS(unit)) {
        uint32 regval;
 
        if (port < 32) {
            READ_CMIC_MIIM_INT_SEL_MAPr(unit, &regval);
            regval |= (1 << port);
            WRITE_CMIC_MIIM_INT_SEL_MAPr(unit, regval);
        } else {
            READ_CMIC_MIIM_INT_SEL_MAP_HIr(unit, &regval);
            regval |= (1 << (port - 32));
            WRITE_CMIC_MIIM_INT_SEL_MAP_HIr(unit, regval);
        }
    }

    /* No action for SBX device */
    return BCM_E_NONE;
}



/*
 * Function:
 *     _bcm_sbx_link_update_asf
 * Purpose:
 *     Update Alternate Store and Forward parameters for a port.
 * Parameters:
 *     unit   - Device unit number
 *     port   - Device port number
 *     linkup - port link state (0=down, 1=up)
 *     speed  - port speed
 *     duplex - port duplex (0=half, 1=full)
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_bcm_sbx_link_update_asf(int unit, bcm_port_t port, int linkup,
                            int speed, int duplex)
{
    /* No action for SBX device */
    return BCM_E_NONE;
}



/*
 * Function:
 *     _bcm_sbx_link_trunk_sw_ftrigger
 * Purpose:
 *     Remove specified ports with link down from trunks.
 * Parameters:
 *     unit        - Device unit number
 *     pbmp_active - Bitmap of ports
 *     pbmp_status - Bitmap of port status
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_bcm_sbx_link_trunk_sw_ftrigger(int unit, bcm_pbmp_t pbmp_active,
                                   bcm_pbmp_t pbmp_status)
{
    /* No action for SBX device */
    return BCM_E_NONE;
}



/*
 * Function:
 *     bcm_linkscan_init
 * Purpose:
 *     Initialize the linkscan software module.
 * Parameters:
 *     unit - Device unit number
 * Returns:
 *     BCM_E_XXX
 * Notes:
 *     If specific HW linkscan initialization is required by device,
 *     driver should call that.
 */
int
bcm_sbx_linkscan_init(int unit)
{
    int rv;
    
    BCM_SBX_LOCK(unit);

    rv = _bcm_linkscan_init(unit, &_bcm_ls_driver_sbx);

    LINK_OUT(("BCM linkscan init unit=%d rv=%d(%s)\n",
              unit, rv, bcm_errmsg(rv)));

    BCM_SBX_UNLOCK(unit);
    return rv;
}


static _bcm_ls_driver_t  _bcm_ls_driver_sbx = {
    _bcm_sbx_link_hw_interrupt,      /* ld_hw_interrupt */
    _bcm_sbx_link_port_link_get,     /* ld_port_link_get */
    _bcm_sbx_link_internal_select,   /* ld_internal_select */
    _bcm_sbx_link_update_asf,        /* ld_update_asf */
    _bcm_sbx_link_trunk_sw_ftrigger  /* ld_trunk_sw_failover_trigger */
};
