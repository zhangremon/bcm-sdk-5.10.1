/*
 * $Id: port.c 1.20 Broadcom SDK $
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
 * Purpose:     Strata family port function implementations
 */

#include <soc/defs.h>

#include <assert.h>

#include <sal/core/libc.h>

#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/cm.h>
#include <soc/drv.h>
#include <soc/register.h>
#include <soc/memory.h>
#include <soc/phy.h>
#include <soc/ll.h>
#include <soc/ptable.h>

#include <bcm/port.h>
#include <bcm/vlan.h>
#include <bcm/stg.h>
#include <bcm/filter.h>
#include <bcm/error.h>
#include <bcm/link.h>
#include <bcm/stack.h>

#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/stg.h>
#include <bcm_int/esw/strata.h>

#if defined(BCM_DRACO_SUPPORT) || defined(BCM_TUCANA_SUPPORT) 

/* 
 * Egress rate metering routines for Strata2-compatible chips 
 * with 8 bucket sizes.
 */

/*
 * Function:
 *	bcm_strata_port_rate_egress_set
 * Purpose:
 *	Set egress rate limiting parameters for
 *      Strata2, Draco, Tucana and Draco 1.5 chips.
 * Parameters:
 *	unit - Device number
 *	port - Port number
 *	kbits_sec - Rate in kilobits (1000 bits) per second.
 *		    Rate of 0 disables rate limiting.
 *	kbits_burst - Maximum burst size in kilobits (1000 bits).
 * Returns:
 *	BCM_E_XXX
 */

int bcm_strata_port_rate_egress_set(int unit,
				    bcm_port_t port,
				    uint32 kbits_sec,
				    uint32 kbits_burst)
{

    uint64 regval64;
    uint8 bucketsize;
    uint32 refreshcount, enable;
    
    if (!SOC_REG_IS_VALID(unit, EGR_FLOWCTL_CFGr)) {
	return BCM_E_UNAVAIL;
    }

    bucketsize = _bcm_strata_kbits_to_bucketsize(unit, port, 
						 0, kbits_burst);

    if (kbits_sec) {
        enable = 1;
        refreshcount = _bcm_strata_kbps_to_refreshcount(unit, port, 
							0, kbits_sec);
    }
    else {
        enable = refreshcount = 0; 
    }

    BCM_IF_ERROR_RETURN(READ_EGR_FLOWCTL_CFGr(unit, port, &regval64));
    soc_reg64_field32_set(unit, EGR_FLOWCTL_CFGr, &regval64,
			  BKT_SIZEf, bucketsize);
    soc_reg64_field32_set(unit, EGR_FLOWCTL_CFGr, &regval64,
			  REFCOUNTf, refreshcount);
    /* Set the FLOW_EN bit */
    soc_reg64_field32_set(unit, EGR_FLOWCTL_CFGr, &regval64,
			  FLOW_ENf, enable);
    return WRITE_EGR_FLOWCTL_CFGr(unit, port, regval64);
}

/*
 * Function:
 *	bcm_strata_port_rate_egress_get
 * Purpose:
 *	Get egress rate limiting parameters from
 *      Strata2, Draco, Tucana and Draco 1.5 chips.
 * Parameters:
 *	unit - Device number
 *	port - Port number
 *	kbits_sec - (OUT) Rate in kilobits (1000 bits) per second, or
 *			  zero if rate limiting is disabled.
 *	kbits_burst - (OUT) Maximum burst size in kilobits (1000 bits).
 * Returns:
 *	BCM_E_XXX
 */

int bcm_strata_port_rate_egress_get(int unit,
				    bcm_port_t port,
				    uint32 *kbits_sec,
				    uint32 *kbits_burst)
{

    uint8 bucketsize;
    uint16 refreshcount;
    uint64 regval64;

    if (!kbits_sec || !kbits_burst) {
        return (BCM_E_PARAM);
    }

    if (!SOC_REG_IS_VALID(unit, EGR_FLOWCTL_CFGr)) {
	return BCM_E_UNAVAIL;
    }

    BCM_IF_ERROR_RETURN(READ_EGR_FLOWCTL_CFGr(unit, port, &regval64));

    if (!soc_reg64_field32_get(unit, EGR_FLOWCTL_CFGr, regval64, FLOW_ENf)) {
        *kbits_sec = *kbits_burst = 0;
    }
    else {
        /* Convert the REFCOUNT field to kbits/sec (1000 bits/sec). */
        refreshcount = soc_reg64_field32_get(unit, EGR_FLOWCTL_CFGr, 
					     regval64, REFCOUNTf);
	*kbits_sec = _bcm_strata_refreshcount_to_kbps(unit, port,
						      0, refreshcount);
	/* Convert the BUCKETSIZE field to burst size in kbits. */
	bucketsize =
	  soc_reg64_field32_get(unit, EGR_FLOWCTL_CFGr, regval64,
				BKT_SIZEf);
	*kbits_burst = _bcm_strata_bucketsize_to_kbits(unit, port, 
						       0, bucketsize);
    }
    return BCM_E_NONE;
}

/* 
 * Egress rate metering routines for Strata2-compatible chips 
 * with 8 bucket sizes.
 */
#endif /* BCM_DRACO_SUPPORT || BCM_TUCANA_SUPPORT */
