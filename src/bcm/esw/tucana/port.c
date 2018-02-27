/*
 * $Id: port.c 1.8 Broadcom SDK $
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
 * Purpose:     Tucana port function implementations
 */

#include <soc/defs.h>

#if defined(BCM_TUCANA_SUPPORT)

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
#include <bcm/filter.h>
#include <bcm/error.h>
#include <bcm/link.h>

#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/tucana.h>

#define TUCANA_BUCKET_SETTINGS 8
#define TUCANA_PAUSE_SETTINGS 8

uint32 _pause_xlat_table[TUCANA_PAUSE_SETTINGS] = {
    4096*8/1000,
    6144*8/1000,
    8192*8/1000,
    16384*8/1000,
    24576*8/1000,
    32768*8/1000,
    40960*8/1000,
    49152*8/1000
};

int bcm_tucana_port_rate_ingress_set(int unit, bcm_port_t port,
				     uint32 kbits_sec, uint32 kbits_burst)
{
    uint64 regval64;
    int i;

    BCM_IF_ERROR_RETURN(READ_INGR_METER_CTRLr(unit, port, &regval64));

    if (!kbits_sec) {
        /* Disable metering for this port */
	soc_reg64_field32_set(unit, INGR_METER_CTRLr, &regval64,
			      REFRESHf, 0);
	soc_reg64_field32_set(unit, INGR_METER_CTRLr, &regval64,
			      METER_ENf, 0);
	soc_reg64_field32_set(unit, INGR_METER_CTRLr, &regval64,
			      DROP_ENf, 0);
    }
    else {
        /* Enable metering for this port */
	for (i=0; i< TUCANA_BUCKET_SETTINGS; i++) {
	    if (kbits_burst <= (4096U * 8 * (1 << i))/1000)
	        break;
	}
	soc_reg64_field32_set(unit, INGR_METER_CTRLr, &regval64,
			      BSIZEf, i);
	/* Rate has a granularity of 64 kbps */
	soc_reg64_field32_set(unit, INGR_METER_CTRLr, &regval64,
			      REFRESHf, (kbits_sec+63)/64);
	/* Set the METER_EN bit */
	soc_reg64_field32_set(unit, INGR_METER_CTRLr, &regval64,
			      METER_ENf, 1);
	/* Set the DROP_EN bit */
	soc_reg64_field32_set(unit, INGR_METER_CTRLr, &regval64,
			      DROP_ENf, 1);
    }

    BCM_IF_ERROR_RETURN(WRITE_INGR_METER_CTRLr(unit, port, regval64));
    return BCM_E_NONE;
}


int bcm_tucana_port_rate_ingress_get(int unit, bcm_port_t port,
				     uint32 *kbits_sec, uint32 *kbits_burst)
{
    uint32 bs;
    uint64 regval64;
    uint32 refreshrate, burstsize, enable;

    BCM_IF_ERROR_RETURN(READ_INGR_METER_CTRLr(unit, port, &regval64));

    refreshrate = soc_reg64_field32_get(unit, INGR_METER_CTRLr, 
					regval64, REFRESHf);
    burstsize = soc_reg64_field32_get(unit, INGR_METER_CTRLr, 
				      regval64, BSIZEf);
    enable = 
      soc_reg64_field32_get(unit, INGR_METER_CTRLr, regval64, METER_ENf)    
      &&
      soc_reg64_field32_get(unit, INGR_METER_CTRLr, regval64, DROP_ENf);

    if (!refreshrate || !enable) {
        *kbits_sec = 0;
	*kbits_burst = 0;
    }
    else {
        for (bs = 0; bs < TUCANA_BUCKET_SETTINGS; bs++) {
	    if (burstsize == bs)
	        break;
	}
	*kbits_burst = (4096U * 8 * (1 << bs))/1000;
	*kbits_sec = 64 * refreshrate;
    }
    return BCM_E_NONE;
}


int bcm_tucana_port_rate_pause_set(int unit, bcm_port_t port,
				   uint32 kbits_pause, uint32 kbits_resume)
{
    uint64 regval64;
    int i;

    BCM_IF_ERROR_RETURN(READ_INGR_METER_CTRLr(unit, port, &regval64));

    if (!kbits_pause) {
        /* Disable pause frames for this port */
	soc_reg64_field32_set(unit, INGR_METER_CTRLr, &regval64,
			      PTHRESHf, 0);
	soc_reg64_field32_set(unit, INGR_METER_CTRLr, &regval64,
			      PAUSE_ENf, 0);
    }
    else if (!soc_reg64_field32_get(unit, INGR_METER_CTRLr, 
				    regval64, METER_ENf)) {
        /* Metering must be enabled for pause functionality */
        return BCM_E_CONFIG;
    }
    else {
        /* Enable pause frames for this port */
	/* Use next higher pause threshold */
	for (i=0; i<TUCANA_PAUSE_SETTINGS; i++) {
	    if (kbits_pause <= _pause_xlat_table[i])
	        break;
	}
	if (i==TUCANA_PAUSE_SETTINGS) 
	  i--; /* Highest pause threshold */
	soc_reg64_field32_set(unit, INGR_METER_CTRLr, &regval64,
			      PTHRESHf, i);
	/* Use next higher resume threshold */
	for (i=0; i<TUCANA_PAUSE_SETTINGS;  i++) {
	    if (kbits_resume <= (4096U * 8 * (1 << i))/1000)
	        break;
	}
	if (i==TUCANA_PAUSE_SETTINGS) 
	    i--; /* Highest resume threshold */
	soc_reg64_field32_set(unit, INGR_METER_CTRLr, &regval64,
			      RTHRESHf, i);
	/* Set the PAUSE_EN bit */
	soc_reg64_field32_set(unit, INGR_METER_CTRLr, &regval64,
			      PAUSE_ENf, 1);
    }
    
    BCM_IF_ERROR_RETURN(WRITE_INGR_METER_CTRLr(unit, port, regval64));
    return BCM_E_NONE;
}


int bcm_tucana_port_rate_pause_get(int unit, bcm_port_t port,
				   uint32 *kbits_pause, uint32 *kbits_resume)
{
    uint64 regval64;
    uint32 pthresh, rthresh;

    BCM_IF_ERROR_RETURN(READ_INGR_METER_CTRLr(unit, port, &regval64));

    if (!soc_reg64_field32_get(unit, INGR_METER_CTRLr, 
			       regval64, PAUSE_ENf) ||
	!soc_reg64_field32_get(unit, INGR_METER_CTRLr, 
			       regval64, METER_ENf) ) {
        /* Pause frames are disabled */
        *kbits_pause = 0;
	*kbits_resume = 0;
    }
    else {
        /* Extract PAUSE and RESUME values */
        pthresh = soc_reg64_field32_get(unit, INGR_METER_CTRLr, 
					regval64, PTHRESHf);
	*kbits_pause = _pause_xlat_table[pthresh];
        rthresh = soc_reg64_field32_get(unit, INGR_METER_CTRLr, 
					regval64, RTHRESHf);
	*kbits_resume = (4096*8*(1<<rthresh))/1000;
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_tucana_port_dtag_mode_set
 * Description:
 *      Set the double-tagging mode of a port.
 * Parameters:
 *      unit - (IN) BCM device number
 *      port - (IN) Port number
 *      mode - (IN) Double-tagging mode, one of:
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
 */
int
_bcm_tucana_port_dtag_mode_set(int unit, bcm_port_t port, int mode, 
                           int dt_mode, int ignore_tag)
{
    uint32  config, oconfig;
    uint64  vctrl, ovctrl;

    SOC_IF_ERROR_RETURN(READ_CHIP_CONFIGr(unit, &config));
    oconfig = config;
    SOC_IF_ERROR_RETURN(READ_VLAN_CONTROLr(unit, port, &vctrl));
    ovctrl = vctrl;
    soc_reg_field_set(unit, CHIP_CONFIGr, &config,
                      DT_MODEf, dt_mode);
    soc_reg64_field32_set(unit, VLAN_CONTROLr, &vctrl,
                          IGNORE_QTAGf, ignore_tag);
    if (COMPILER_64_NE(vctrl, ovctrl)) {
        SOC_IF_ERROR_RETURN(WRITE_VLAN_CONTROLr(unit, port, vctrl));
    }
    if (config != oconfig) {
        SOC_IF_ERROR_RETURN(WRITE_CHIP_CONFIGr(unit, config));
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_tucana_port_dtag_mode_get
 * Description:
 *      Return the current double-tagging mode of a port.
 * Parameters:
 *      unit - (IN) BCM device number
 *      port - (IN) Port number
 *      mode - (OUT) Double-tagging mode
 * Return Value:
 *      BCM_E_XXX
 */

int
_bcm_tucana_port_dtag_mode_get(int unit, bcm_port_t port, int *mode)
{
    uint32  config;
    uint64  vctrl;

    SOC_IF_ERROR_RETURN(READ_CHIP_CONFIGr(unit, &config));
    if (soc_reg_field_get(unit, CHIP_CONFIGr, config, DT_MODEf) == 0) {
        *mode = BCM_PORT_DTAG_MODE_NONE;
    } else {
        SOC_IF_ERROR_RETURN(READ_VLAN_CONTROLr(unit, port, &vctrl));
        if (soc_reg64_field32_get(unit, VLAN_CONTROLr, vctrl, IGNORE_QTAGf)) {
            *mode = BCM_PORT_DTAG_MODE_EXTERNAL;
        } else {
            *mode = BCM_PORT_DTAG_MODE_INTERNAL;
        }
    }
    return BCM_E_NONE;
}


#endif	/* BCM_TUCANA_SUPPORT */
