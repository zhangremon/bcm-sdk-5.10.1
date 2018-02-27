/* 
 * $Id: dmux.c 1.19.6.1 Broadcom SDK $
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
 * File:    dmux.c
 * Purpose: BCM level APIs for DMUX
 */

#include <sal/types.h>

#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/drv.h>

#include <bcm/dmux.h>
#include <bcm/error.h>

#include <bcm_int/esw_dispatch.h>
#include <bcm_int/esw/stack.h>

#ifdef BCM_DMUX_SUPPORT

typedef struct dmux_pairs_s {
    bcm_port_t port;
    int        module;
} dmux_pairs_t;

static dmux_pairs_t **_bcm_dmux_info[BCM_MAX_NUM_UNITS];

#define DMUX_INIT(unit)	\
	if (!soc_feature(unit, soc_feature_dmux)) { \
	    return BCM_E_UNAVAIL; \
	} else if (!_bcm_dmux_info[unit]) { return BCM_E_INIT; }

#define DMUX_PORT_INIT(unit, port)    \
        (_bcm_dmux_info[unit] && _bcm_dmux_info[unit][port])

/* 
 * Function:
 *	bcm_esw_dmux_init
 * Purpose:
 *	Initializes the DMUX system for a unit.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 * Returns:
 *      BCM_E_NONE              Success.
 *      BCM_E_XXX
 */

int
bcm_esw_dmux_init(int unit)
{
    int             	alloc_size, blk;
    bcm_port_t          port;

    if (!soc_feature(unit, soc_feature_dmux)) {
        return BCM_E_UNAVAIL;
    }

    /* Known clean state */
    BCM_IF_ERROR_RETURN
        (bcm_esw_dmux_detach(unit));
  
    PBMP_XE_ITER(unit, port) {
	blk = SOC_PORT_BLOCK(unit, port);
        BCM_IF_ERROR_RETURN
            (soc_mem_clear(unit, TRNK_DSTm, blk, FALSE));
    }

    alloc_size = SOC_MAX_NUM_PORTS * sizeof(dmux_pairs_t *);
    _bcm_dmux_info[unit] = sal_alloc(alloc_size, "DMUX ports");
    if (_bcm_dmux_info[unit] == NULL) {
        return BCM_E_MEMORY;
    }
    sal_memset(_bcm_dmux_info[unit], 0, alloc_size);

    return BCM_E_NONE;
}

STATIC int
_bcm_dmux_disable_port(int unit, bcm_port_t port)
{
    uint32              config;

    if (DMUX_PORT_INIT(unit, port)) {
        sal_free(_bcm_dmux_info[unit][port]);
        _bcm_dmux_info[unit][port] = NULL;
    }

    BCM_IF_ERROR_RETURN
        (READ_CONFIGr(unit, port, &config));
    soc_reg_field_set(unit, CONFIGr, &config, DMUX_ENABLEf, 0);
    BCM_IF_ERROR_RETURN
        (WRITE_CONFIGr(unit, port, config));

    return BCM_E_NONE;
}

/* 
 * Function:
 *	bcm_esw_dmux_detach
 * Purpose:
 *	Clean up the DMUX tables.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 * Returns:
 *      BCM_E_NONE              Success.
 *      BCM_E_XXX
 */

int
bcm_esw_dmux_detach(int unit)
{
    int port;

    if (!soc_feature(unit, soc_feature_dmux)) {
        return BCM_E_UNAVAIL;
    }

    PBMP_XE_ITER(unit, port) {
        BCM_IF_ERROR_RETURN
            (_bcm_dmux_disable_port(unit, port));
    }

    if (_bcm_dmux_info[unit]) {
        sal_free(_bcm_dmux_info[unit]);
        _bcm_dmux_info[unit] = NULL;
    }

    return BCM_E_NONE;
}


/*
 * Function:
 *	bcm_esw_dmux_config_set
 * Description:
 *	Set demuxing configuration of a port.
 * Parameters:
 *	unit - StrataSwitch PCI device unit number (driver internal).
 *	port - StrataSwitch port number of port for which to get info.
 *	flags - Logical OR of BCM_DMUX_xxx flags.
 * Returns:
 *      BCM_E_xxxx
 */
int
bcm_esw_dmux_config_set(int unit, bcm_port_t port, uint32 flags)
{
    uint32 dmux_config = 0;

    DMUX_INIT(unit);

    if (!IS_XE_PORT(unit, port)) {
        return BCM_E_PORT;
    }

    /* This translation is really not necessary for Lynx */
    if (flags & BCM_DMUX_L4DS) {
        soc_reg_field_set(unit, DMUX_TRUNKSELr, &dmux_config, EN_L4DSf, 1);
    }

    if (flags & BCM_DMUX_L4SS) {
        soc_reg_field_set(unit, DMUX_TRUNKSELr, &dmux_config, EN_L4SSf, 1);
    }

    if (flags & BCM_DMUX_IPDA) {
        soc_reg_field_set(unit, DMUX_TRUNKSELr, &dmux_config, EN_IPDAf, 1);
    }

    if (flags & BCM_DMUX_IPSA) {
        soc_reg_field_set(unit, DMUX_TRUNKSELr, &dmux_config, EN_IPSAf, 1);
    }

    if (flags & BCM_DMUX_IPVID) {
        soc_reg_field_set(unit, DMUX_TRUNKSELr, &dmux_config, EN_IPVIDf, 1);
    }

    if (flags & BCM_DMUX_IPTYPE) {
        soc_reg_field_set(unit, DMUX_TRUNKSELr, &dmux_config, EN_IPTYPEf, 1);
    }

    if (flags & BCM_DMUX_IPMACDA) {
        soc_reg_field_set(unit, DMUX_TRUNKSELr, &dmux_config, EN_IPMACDAf, 1);
    }

    if (flags & BCM_DMUX_IPMACSA) {
        soc_reg_field_set(unit, DMUX_TRUNKSELr, &dmux_config, EN_IPMACSAf, 1);
    }

    if (flags & BCM_DMUX_VID) {
        soc_reg_field_set(unit, DMUX_TRUNKSELr, &dmux_config, EN_VIDf, 1);
    }

    if (flags & BCM_DMUX_TYPE) {
        soc_reg_field_set(unit, DMUX_TRUNKSELr, &dmux_config, EN_TYPEf, 1);
    }

    if (flags & BCM_DMUX_MACDA) {
        soc_reg_field_set(unit, DMUX_TRUNKSELr, &dmux_config, EN_MACDAf, 1);
    }

    if (flags & BCM_DMUX_MACSA) {
        soc_reg_field_set(unit, DMUX_TRUNKSELr, &dmux_config, EN_MACSAf, 1);
    }

    BCM_IF_ERROR_RETURN
        (WRITE_DMUX_TRUNKSELr(unit, port, dmux_config));

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_esw_dmux_config_get
 * Description:
 *	Retrieve demuxing configuration of a port.
 * Parameters:
 *	unit - StrataSwitch PCI device unit number (driver internal).
 *	port - StrataSwitch port number of port for which to get info.
 *	flags - (OUT) Logical OR of BCM_DMUX_xxx flags.
 * Returns:
 *      BCM_E_xxxx
 */
int
bcm_esw_dmux_config_get(int unit, bcm_port_t port, uint32 *flags)
{
    uint32 dmux_config, temp_flags = 0;

    DMUX_INIT(unit);

    if (!IS_XE_PORT(unit, port)) {
        *flags = 0;
        return BCM_E_NONE;
    }

    BCM_IF_ERROR_RETURN
        (READ_DMUX_TRUNKSELr(unit, port, &dmux_config));

    /* This translation is really not necessary for Lynx */
    if (soc_reg_field_get(unit, DMUX_TRUNKSELr, dmux_config, EN_L4DSf)) {
        temp_flags |= BCM_DMUX_L4DS;
    }

    if (soc_reg_field_get(unit, DMUX_TRUNKSELr, dmux_config, EN_L4SSf)) {
        temp_flags |= BCM_DMUX_L4SS;
    }

    if (soc_reg_field_get(unit, DMUX_TRUNKSELr, dmux_config, EN_IPDAf)) {
        temp_flags |= BCM_DMUX_IPDA;
    }

    if (soc_reg_field_get(unit, DMUX_TRUNKSELr, dmux_config, EN_IPSAf)) {
        temp_flags |= BCM_DMUX_IPSA;
    }

    if (soc_reg_field_get(unit, DMUX_TRUNKSELr, dmux_config, EN_IPVIDf)) {
        temp_flags |= BCM_DMUX_IPVID;
    }

    if (soc_reg_field_get(unit, DMUX_TRUNKSELr, dmux_config, EN_IPTYPEf)) {
        temp_flags |= BCM_DMUX_IPTYPE;
    }

    if (soc_reg_field_get(unit, DMUX_TRUNKSELr, dmux_config, EN_IPMACDAf)) {
        temp_flags |= BCM_DMUX_IPMACDA;
    }

    if (soc_reg_field_get(unit, DMUX_TRUNKSELr, dmux_config, EN_IPMACSAf)) {
        temp_flags |= BCM_DMUX_IPMACSA;
    }

    if (soc_reg_field_get(unit, DMUX_TRUNKSELr, dmux_config, EN_VIDf)) {
        temp_flags |= BCM_DMUX_VID;
    }

    if (soc_reg_field_get(unit, DMUX_TRUNKSELr, dmux_config, EN_TYPEf)) {
        temp_flags |= BCM_DMUX_TYPE;
    }

    if (soc_reg_field_get(unit, DMUX_TRUNKSELr, dmux_config, EN_MACDAf)) {
        temp_flags |= BCM_DMUX_MACDA;
    }

    if (soc_reg_field_get(unit, DMUX_TRUNKSELr, dmux_config, EN_MACSAf)) {
        temp_flags |= BCM_DMUX_MACSA;
    }

    *flags = temp_flags;

    return BCM_E_NONE;
}

STATIC int
_bcm_dmux_search(int unit, bcm_port_t port, int dest_mod, 
                        bcm_port_t dest_port, int *match_idx)
{
    int                pair;
    dmux_pairs_t       *cur_pair;

    for (pair = 0; pair < BCM_MAX_DMUX_PAIRS; pair++) {
        cur_pair = &(_bcm_dmux_info[unit][port][pair]);
        if (cur_pair->module == -1) {
            /* End of list */
            *match_idx = pair;
            return FALSE;
        }

        if ((cur_pair->module == dest_mod) && 
            (cur_pair->port == dest_port)) {
                *match_idx = pair;
                return TRUE;
        }
    }

    /* Ran out of table */
    *match_idx = -1;
    return FALSE;
}

STATIC int
_bcm_dmux_write_table(int unit, bcm_port_t port)
{
    int                 pair, entry;
    dmux_pairs_t        *cur_pair;
    trnk_dst_entry_t    dmux_entry;
    int			blk = SOC_PORT_BLOCK(unit, port);
    bcm_module_t	mod_in, mod_out;
    bcm_port_t		port_in, port_out;

    sal_memset(&dmux_entry, 0, sizeof(trnk_dst_entry_t));
    entry = pair = 0;

    while (entry < BCM_MAX_DMUX_PAIRS) {
        cur_pair = &(_bcm_dmux_info[unit][port][pair]);
        if (cur_pair->module == -1) {
            assert(pair != 0);
            pair = 0;
            cur_pair = &(_bcm_dmux_info[unit][port][pair]);
        }

	mod_in = cur_pair->module;
	port_in = cur_pair->port;
	BCM_IF_ERROR_RETURN
	    (_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_SET,
                                    mod_in, port_in,
                                    &mod_out, &port_out));
        soc_TRNK_DSTm_field32_set(unit, &dmux_entry, DST_MODIDf, mod_out);
        soc_TRNK_DSTm_field32_set(unit, &dmux_entry, DST_PORTIDf, port_out);
        BCM_IF_ERROR_RETURN
            (WRITE_TRNK_DSTm(unit, blk, entry, &dmux_entry));

        pair++;
        entry++;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_esw_dmux_dest_add
 * Description:
 *	Add (module, port) pair to demux destinations for a port.
 * Parameters:
 *	unit - StrataSwitch PCI device unit number (driver internal).
 *	port - StrataSwitch port number of port to configure.
 *	dest_mod - module number of unit to which to forward packet.
 *	dest_port - port number of port to which to forward packet.
 * Returns:
 *      BCM_E_xxxx
 */
int
bcm_esw_dmux_dest_add(int unit, bcm_port_t port, int dest_mod, 
                        bcm_port_t dest_port)
{
    int			alloc_size;
    int                 pair;
    uint32              config;

    DMUX_INIT(unit);

    if (!IS_XE_PORT(unit, port)) {
        return BCM_E_PORT;
    }

    if (!DMUX_PORT_INIT(unit, port)) {
        if (!_bcm_dmux_info[unit][port]) {
            alloc_size = BCM_MAX_DMUX_PAIRS * sizeof(dmux_pairs_t);
            _bcm_dmux_info[unit][port] = 
                sal_alloc(alloc_size, "DMUX port entries");
            if (_bcm_dmux_info[unit][port] == NULL) {
                return BCM_E_MEMORY;
            }
            sal_memset(_bcm_dmux_info[unit][port], 0, alloc_size);
            for (pair = 0; pair < BCM_MAX_DMUX_PAIRS; pair++) {
                _bcm_dmux_info[unit][port][pair].module = -1;
            }
        }
    }

    if (_bcm_dmux_search(unit, port, dest_mod, dest_port, &pair)) {
        /* Already exists (or do we want to use this to weight?) */
        return BCM_E_EXISTS;
    } else if (pair == -1) {
        /* No free entries */
        return BCM_E_RESOURCE;
    }

    /*    coverity[overrun-local]    */
    _bcm_dmux_info[unit][port][pair].module = dest_mod;
    _bcm_dmux_info[unit][port][pair].port = dest_port;

    BCM_IF_ERROR_RETURN
        (_bcm_dmux_write_table(unit, port));

    /* Was this the first entry? Turn on DMUX */
    if (pair == 0) {
        BCM_IF_ERROR_RETURN
            (READ_CONFIGr(unit, port, &config));
        soc_reg_field_set(unit, CONFIGr, &config, DMUX_ENABLEf, 1);
        BCM_IF_ERROR_RETURN
            (WRITE_CONFIGr(unit, port, config));
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_esw_dmux_dest_delete
 * Description:
 *	Remove (module, port) pair from demux destinations for a port.
 * Parameters:
 *	unit - StrataSwitch PCI device unit number (driver internal).
 *	port - StrataSwitch port number of port to configure.
 *	dest_mod - module number of unit to which to forward packet.
 *	dest_port - port number of port to which to forward packet.
 * Returns:
 *      BCM_E_xxxx
 */
int
bcm_esw_dmux_dest_delete(int unit, bcm_port_t port, int dest_mod, 
                           bcm_port_t dest_port)
{
    int                 pair;
    dmux_pairs_t        *cur_pair, *next_pair;

    DMUX_INIT(unit);

    if (!IS_XE_PORT(unit, port)) {
        return BCM_E_PORT;
    }

    if (!DMUX_PORT_INIT(unit, port)) {
        return BCM_E_NONE;
    }

    if (!_bcm_dmux_search(unit, port, dest_mod, dest_port, &pair)) {
        return BCM_E_NOT_FOUND;
    }

    for (; pair < (BCM_MAX_DMUX_PAIRS - 1); pair++) {
        cur_pair = &(_bcm_dmux_info[unit][port][pair]);
        next_pair = &(_bcm_dmux_info[unit][port][pair + 1]);
        
        cur_pair->module = next_pair->module;
        cur_pair->port = next_pair->port;

        if (cur_pair->module == -1) {
            break;
        }
    }

    if (pair == 0) {
        /* We flushed the last entry */
        BCM_IF_ERROR_RETURN
            (_bcm_dmux_disable_port(unit, port));
        return BCM_E_NONE;
    } else if (pair == BCM_MAX_DMUX_PAIRS - 1) {
        /* Clear last entry */
        cur_pair = &(_bcm_dmux_info[unit][port][pair]);
        cur_pair->module = -1;
    }

    BCM_IF_ERROR_RETURN
        (_bcm_dmux_write_table(unit, port));

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_esw_dmux_dest_delete_all
 * Description:
 *	Clear (module, port) demux destinations for a port.
 * Parameters:
 *	unit - StrataSwitch PCI device unit number (driver internal).
 *	port - StrataSwitch port number of port to configure.
 * Returns:
 *      BCM_E_xxxx
 */
int
bcm_esw_dmux_dest_delete_all(int unit, bcm_port_t port)
{
    DMUX_INIT(unit);

    if (!IS_XE_PORT(unit, port)) {
        return BCM_E_PORT;
    }

    if (!DMUX_PORT_INIT(unit, port)) {
        return BCM_E_NONE;
    }

    BCM_IF_ERROR_RETURN
        (_bcm_dmux_disable_port(unit, port));

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_esw_dmux_dest_get
 * Description:
 *	Retrieve port bitmap of demux destinations for a given destination
 *      module configured on the selected port.
 * Parameters:
 *	unit - StrataSwitch PCI device unit number (driver internal).
 *	port - StrataSwitch port number of port to configure.
 *	dest_mod - module number of unit to which to forward packet.
 *	dest_pbmp - (OUT) port bitmap corresponding to module.
 * Returns:
 *      BCM_E_xxxx
 */
int
bcm_esw_dmux_dest_get(int unit, bcm_port_t port, int dest_mod, 
                        bcm_pbmp_t *dest_pbmp)
{
    int                 pair;
    bcm_pbmp_t          temp_pbmp;
    dmux_pairs_t        *cur_pair;

    DMUX_INIT(unit);

    if (!IS_XE_PORT(unit, port) || !DMUX_PORT_INIT(unit, port)) {
        BCM_PBMP_CLEAR(*dest_pbmp);
        return BCM_E_NONE;
    }

    BCM_PBMP_CLEAR(temp_pbmp);
    for (pair = 0; pair < BCM_MAX_DMUX_PAIRS; pair++) {
        cur_pair = &(_bcm_dmux_info[unit][port][pair]);
        if (cur_pair->module == -1) {
            break;
        }

        if (cur_pair->module == dest_mod) {
            BCM_PBMP_PORT_ADD(temp_pbmp, cur_pair->port);
        }
    }

    BCM_PBMP_ASSIGN(*dest_pbmp, temp_pbmp);
    return BCM_E_NONE;
}

#else  /* BCM_DMUX_SUPPORT */
/*
 * No dmux support, so just return BCM_E_UNAVAIL for all apis
 */
int bcm_esw_dmux_init(int unit)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_dmux_detach(int unit)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_dmux_config_set(int unit, bcm_port_t port, uint32 flags)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_dmux_config_get(int unit, bcm_port_t port, uint32 *flags)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_dmux_dest_add(int unit, bcm_port_t port,
		      int dest_mod, bcm_port_t dest_port)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_dmux_dest_delete(int unit, bcm_port_t port,
			 int dest_mod, bcm_port_t dest_port)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_dmux_dest_delete_all(int unit, bcm_port_t port)
{
    return BCM_E_UNAVAIL;
}

int bcm_esw_dmux_dest_get(int unit, bcm_port_t port,
		      int dest_mod, bcm_pbmp_t *dest_pbmp)
{
    return BCM_E_UNAVAIL;
}

#endif  /* BCM_DMUX_SUPPORT */
