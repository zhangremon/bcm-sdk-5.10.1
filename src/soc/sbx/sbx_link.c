/* 
 * $Id: sbx_link.c 1.5 Broadcom SDK $
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
 * File:        sbx_link.c
 * Purpose:     Hardware Linkscan module
 *
 */

#include <soc/types.h>
#include <soc/error.h>
#include <soc/drv.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/link.h>

#include <soc/linkctrl.h>


static soc_linkctrl_port_info_t  _soc_sbx_linkctrl_port_info[] = {
    {            -1,   -1,   -1,  -1 }   /* Last */
};


/*
 * Function:
 *     _soc_sbx_linkctrl_linkscan_hw_init
 * Purpose:
 *     Initialize hardware linkscan.
 * Parameters:
 *     unit  - Device number
 * Returns:
 *     SOC_E_XXX
 */
STATIC int
_soc_sbx_linkctrl_linkscan_hw_init(int unit)
{
    if (SOC_IS_SIRIUS(unit)) {
#ifdef BCM_SIRIUS_SUPPORT
        return (soc_linkscan_hw_init(unit)); 
#endif
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *     _soc_sbx_linkctrl_linkscan_config
 * Purpose:
 *     Set ports to hardware linkscan.
 * Parameters:
 *     unit          - Device number
 *     hw_mii_pbm    - Port bit map of ports to scan with MIIM registers
 *     hw_direct_pbm - Port bit map of ports to scan using NON MII
 * Returns:
 *     SOC_E_XXX
 */
STATIC int
_soc_sbx_linkctrl_linkscan_config(int unit, pbmp_t hw_mii_pbm,
                                     pbmp_t hw_direct_pbm)
{
    COMPILER_REFERENCE(unit);

    if (SOC_PBMP_NOT_NULL(hw_mii_pbm) || SOC_PBMP_NOT_NULL(hw_direct_pbm)) {
         if (SOC_IS_SIRIUS(unit)) { 
             return (soc_linkscan_config(unit, hw_mii_pbm, hw_direct_pbm));
         } 
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *     _soc_sbx_linkctrl_linkscan_pause
 * Purpose:
 *     Pause link scanning, without disabling it.
 *     This call is used to pause scanning temporarily.
 * Parameters:
 *     unit  - Device number
 * Returns:
 *     SOC_E_XXX
 */
STATIC int
_soc_sbx_linkctrl_linkscan_pause(int unit)
{
    if (SOC_IS_SIRIUS(unit)) {
        soc_linkscan_pause(unit);
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *     _soc_sbx_linkctrl_linkscan_continue
 * Purpose:
 *     Continue link scanning after it has been paused.
 * Parameters:
 *     unit  - Device number
 * Returns:
 *     SOC_E_XXX
 */
STATIC int
_soc_sbx_linkctrl_linkscan_continue(int unit)
{
    if (SOC_IS_SIRIUS(unit)) {
        soc_linkscan_continue(unit);
    }

    return SOC_E_NONE;
}

/*
 * Function:    
 *     _soc_sbx_linkctrl_update
 * Purpose:
 *     Update the forwarding state in device.
 * Parameters:  
 *      unit - Device unit number
 * Returns:
 *     SOC_E_XXX
 */
STATIC int
_soc_sbx_linkctrl_update(int unit)
{
    return SOC_E_NONE;
}

STATIC int
_soc_sbx_linkctrl_hw_link_get(int unit, soc_pbmp_t *hw_link)
{
    if (SOC_IS_SBX_SIRIUS(unit)) {
        uint32              link_stat;
        uint32              link_pbmp;
        soc_pbmp_t          tmp_pbmp;

        if (NULL == hw_link) {
            return SOC_E_PARAM;
        }

        SOC_PBMP_CLEAR(tmp_pbmp);
        SOC_IF_ERROR_RETURN(READ_CMIC_LINK_STATr(unit, &link_stat));
        link_pbmp = soc_reg_field_get(unit, CMIC_LINK_STATr,
                                      link_stat, PORT_BITMAPf);
        SOC_PBMP_WORD_SET(tmp_pbmp, 0, link_pbmp);
        SOC_IF_ERROR_RETURN(READ_CMIC_LINK_STAT_HIr(unit, &link_stat));
        SOC_PBMP_WORD_SET(tmp_pbmp, 1,
                          soc_reg_field_get(unit, CMIC_LINK_STATr,
                                            link_stat, PORT_BITMAPf));
        SOC_PBMP_ASSIGN(*hw_link, tmp_pbmp);
    }

    return SOC_E_NONE;
}

/*
 * Link Control Driver - SBX
 */
soc_linkctrl_driver_t  soc_linkctrl_driver_sbx = {
    _soc_sbx_linkctrl_port_info,            /* port mapping */
    _soc_sbx_linkctrl_linkscan_hw_init,     /* ld_linkscan_hw_init */
    _soc_sbx_linkctrl_linkscan_config,      /* ld_linkscan_config */
    _soc_sbx_linkctrl_linkscan_pause,       /* ld_linkscan_pause */
    _soc_sbx_linkctrl_linkscan_continue,    /* ld_linkscan_continue */
    _soc_sbx_linkctrl_update,               /* ld_update */
    _soc_sbx_linkctrl_hw_link_get           /* ld_hw_link_get */
};
