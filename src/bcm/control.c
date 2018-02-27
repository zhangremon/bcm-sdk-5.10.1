/*
 * $Id: control.c 1.52.6.2 Broadcom SDK $
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
 * BCM Dispatch Control Operations (eg: attach, detach)
 */

#include <shared/alloc.h>
#include <sal/core/libc.h>
#include <sal/core/sync.h>
#include <bcm/debug.h>

#include <soc/drv.h>
#ifdef BCM_SBX_SUPPORT
#include <soc/sbx/sbx_drv.h>
#endif

#include <bcm/error.h>

#include <bcm_int/dispatch.h>
#include <bcm_int/control.h>
#include <bcm_int/api_ref.h>
#include <bcm_int/api_xlate_port.h>


bcm_control_t		*bcm_control[BCM_CONTROL_MAX];
static sal_mutex_t	_bcm_control_lock;

#define	CONTROL_LOCK	sal_mutex_take(_bcm_control_lock, sal_mutex_FOREVER)
#define	CONTROL_UNLOCK	sal_mutex_give(_bcm_control_lock)

#define BCM_DEVICE_CONTROL_DESTROY(_unit_)                  \
        if (BCM_CONTROL(_unit_)->subtype != NULL) {       \
            sal_free(BCM_CONTROL(_unit_)->subtype);       \
        }                                                 \
        sal_free(BCM_CONTROL(_unit_));                    \
        BCM_CONTROL(_unit_) = NULL; 

/*
 * BCM Dispatch Type name table
 */
#define BCM_DLIST_ENTRY(_dtype)\
#_dtype, 

static const char* _bcm_dtype_names[] = {
#include <bcm_int/bcm_dlist.h>
    NULL, 
}; 

/*
 * Name to dispatch type
 */
static bcm_dtype_t
_bcm_type_find(const char* name)
{
    int i; 
    for(i = 0; i < bcmTypeCount; i++) {
        if(!sal_strcmp(_bcm_dtype_names[i], name)) {
            return i; 
        }       
    }
    return bcmTypeNone; 
}


/*
 * The following are functions need to be dispatchable by type within this module only. 
 */

/* _attach */
#define _bcm_fe2000_attach _bcm_sbx_attach
#define BCM_DLIST_ENTRY(_dtype)\
extern int _bcm_##_dtype##_attach(int, char*); 
#include <bcm_int/bcm_dlist.h>

#define BCM_DLIST_ENTRY(_dtype)\
_bcm_##_dtype##_attach,

static int (*_dispatch_attach[])(int, char*) = {
#include <bcm_int/bcm_dlist.h>
};

/* _init */
#define bcm_fe2000_init bcm_sbx_init
#define BCM_DLIST_ENTRY(_dtype)\
extern int bcm_##_dtype##_init(int); 
#include <bcm_int/bcm_dlist.h>

#define BCM_DLIST_ENTRY(_dtype)\
bcm_##_dtype##_init,

static int (*_dispatch_init[])(int) = {
#include <bcm_int/bcm_dlist.h>
};

/* _detach */
#define _bcm_fe2000_detach _bcm_sbx_detach
#define BCM_DLIST_ENTRY(_dtype)\
extern int _bcm_##_dtype##_detach(int); 
#include <bcm_int/bcm_dlist.h>

#define BCM_DLIST_ENTRY(_dtype)\
_bcm_##_dtype##_detach,

static int (*_dispatch_detach[])(int) = {
#include <bcm_int/bcm_dlist.h>
};

int
bcm_attach(int unit, char *type, char *subtype, int remunit)
{
    int			rv;

    BCM_DEBUG(BCM_DBG_ATTACH, ("STK %d: attach %s subtype %s as %d\n",
                               unit, (NULL !=type) ? type: "N/A", 
                               (NULL != subtype) ? subtype:"N/A",
                               remunit));

    /* Protection mutex initialization. */
    if (_bcm_control_lock == NULL) {
        _bcm_control_lock = sal_mutex_create("bcm_control");
        if (_bcm_control_lock == NULL) {
            return (BCM_E_MEMORY);
        }
    }

    CONTROL_LOCK;
    if (unit < 0) {	/* find a free unit */
        for (unit = 0; unit < BCM_CONTROL_MAX; unit++) {
            if (BCM_CONTROL(unit) == NULL) {
                break;
            }
        }
        if (unit >= BCM_CONTROL_MAX) {
            CONTROL_UNLOCK;
            return (BCM_E_FULL);
        }
    }

    /* Check if unit is already attached. */
    if (BCM_CONTROL(unit) != NULL) {
        CONTROL_UNLOCK;
        return (BCM_E_EXISTS);
    }

    /* Allocate unit control structure. */
    BCM_CONTROL(unit) = sal_alloc(sizeof(bcm_control_t), "bcm_control");
    if (BCM_CONTROL(unit) == NULL) {
        CONTROL_UNLOCK;
        return (BCM_E_MEMORY);
    }
    sal_memset(BCM_CONTROL(unit), 0, sizeof(bcm_control_t));

    if (subtype != NULL) {
        BCM_CONTROL(unit)->subtype = sal_alloc(sal_strlen(subtype)+1,
                                               "bcm_control subtype");
        if (BCM_CONTROL(unit)->subtype == NULL) {
            BCM_DEVICE_CONTROL_DESTROY(unit);
            CONTROL_UNLOCK;                                   
            return (BCM_E_MEMORY);
        }
        sal_strcpy(BCM_CONTROL(unit)->subtype, subtype);
    }

    if (type == NULL) {
        if (SOC_IS_ROBO(unit)) {
#ifdef	BCM_ROBO_SUPPORT
            type = "robo";
#endif
        }
#ifdef  BCM_SBX_SUPPORT
        else if (SOC_IS_SBX(unit)) {
	    if (SOC_IS_SBX_QE(unit) || SOC_IS_SBX_BME(unit)) {
		type = "sbx";
	    } else {
		type = SOC_CHIP_STRING(unit);
	    }
        }
#endif
#ifdef BCM_EA_SUPPORT
        else if (SOC_IS_EA(unit)){
        	type = "tk371x";
        }
#endif
#ifdef BCM_SHADOW_SUPPORT
        else if (SOC_IS_SHADOW(unit)) {
            type = "shadow";
        }
#endif
#ifdef BCM_ESW_SUPPORT
        else { 
            type = "esw";
        }
#endif
    }

    if (NULL == type) {
        BCM_DEVICE_CONTROL_DESTROY(unit);
        CONTROL_UNLOCK;                                   
        return (BCM_E_CONFIG);
    }

    /* 
     * Assign dtype based on type string. The string was used for the dispatch table search, 
     * so we leave this alone for backwards compatibility with the rest of the code that calls bcm_attach(). 
     */
    if((BCM_CONTROL(unit)->dtype = _bcm_type_find(type)) == bcmTypeNone) {
        BCM_DEVICE_CONTROL_DESTROY(unit); 
        CONTROL_UNLOCK; 
        return (BCM_E_CONFIG); 
    }

    BCM_CONTROL(unit)->unit = remunit;
    BCM_CONTROL(unit)->name = type; 
    
    
    rv = _dispatch_attach[BCM_DTYPE(unit)](unit, subtype);
    if (BCM_FAILURE(rv)) {
        BCM_DEVICE_CONTROL_DESTROY(unit);
        CONTROL_UNLOCK;                                   
        return (rv);
    }
    CONTROL_UNLOCK;
    return (unit);
}

int
bcm_detach(int unit)
{
    int		rv;

    BCM_DEBUG(BCM_DBG_ATTACH, ("STK %d: unit being detached\n", unit));

    if (unit < 0 || unit >= BCM_CONTROL_MAX) {
        return BCM_E_UNIT;
    }

    if (NULL == BCM_CONTROL(unit)) {
       return (BCM_E_NONE);
    }

    CONTROL_LOCK;

    rv = _dispatch_detach[BCM_DTYPE(unit)](unit); 

    /* Clean up port mappings */
    _bcm_api_xlate_port_cleanup(unit);

    BCM_DEVICE_CONTROL_DESTROY(unit);

    CONTROL_UNLOCK;

    return rv;
}

/* _match */
#define _bcm_fe2000_match _bcm_sbx_match

#define BCM_DLIST_ENTRY(_dtype)\
extern int _bcm_##_dtype##_match(int, char *, char *); 
#include <bcm_int/bcm_dlist.h>

#define BCM_DLIST_ENTRY(_dtype)\
_bcm_##_dtype##_match,

static int (*_dispatch_match[])(int, char *, char *) = {
#include <bcm_int/bcm_dlist.h>
};

int
bcm_find(char *type, char *subtype, int remunit)
{
    int	unit;

    for (unit = 0; unit < BCM_CONTROL_MAX; unit++) {
	if (BCM_CONTROL(unit) == NULL) {
	    continue;
	}
	if (remunit != BCM_CONTROL(unit)->unit) {
	    continue;
	}
	if (type != NULL &&
	    sal_strcmp(type, BCM_TYPE_NAME(unit)) != 0) {
	    continue;
	}
	if (subtype == NULL && BCM_CONTROL(unit)->subtype != NULL) {
	    continue;
	}
	if (subtype != NULL && BCM_CONTROL(unit)->subtype == NULL) {
	    continue;
	}
        if (subtype != NULL &&
            _dispatch_match[BCM_DTYPE(unit)]
            (unit, subtype, BCM_CONTROL(unit)->subtype) != 0) {
            continue;
        }
	return unit;
    }
    return BCM_E_NOT_FOUND;
}

int
bcm_attach_check(int unit)
{
    if (unit < 0 || unit >= BCM_CONTROL_MAX) {
	return BCM_E_UNIT;
    }
    if (BCM_CONTROL(unit) == NULL) {
	return BCM_E_UNIT;
    }
    return BCM_E_NONE;
}

int
bcm_attach_max(int *max_units)
{
    int	unit;

    *max_units = -1;
    for (unit = 0; unit < BCM_CONTROL_MAX; unit++) {
	if (BCM_CONTROL(unit) != NULL) {
	    *max_units = unit;
	}
    }
    return BCM_E_NONE;
}

int
bcm_init(int unit)
{
    int		rv = BCM_E_NONE;
    void    *api_ptr = (void *)bcm_api_tbl;
    int     init_done = FALSE;

    /* This should never be true; done to include api table */
    if (api_ptr == NULL) {
        return 0;
    }

    if (unit < 0 || unit >= BCM_CONTROL_MAX) {
        return BCM_E_UNIT;
    }
    if (BCM_CONTROL(unit) == NULL) {

        if (!SOC_UNIT_VALID(unit)) {
            return BCM_E_UNIT;
        }
        rv = bcm_attach(unit, NULL, NULL, unit);
        if (rv < 0) {
            return rv;
        }
        if (SOC_IS_XGS(unit) ||
        		SOC_IS_ROBO(unit) ||
        		SOC_IS_EA(unit)) {
            init_done = TRUE;
        }
    }

    if (FALSE == init_done) {
        /* Initialize port mappings */
        _bcm_api_xlate_port_init(unit);

        rv = _dispatch_init[BCM_DTYPE(unit)](unit);
    }

#ifdef BCM_CUSTOM_INIT_F
    if(BCM_SUCCESS(rv)) {
        extern int BCM_CUSTOM_INIT_F (int unit); 
        rv = BCM_CUSTOM_INIT_F (unit); 
    }
#endif /* BCM_CUSTOM_INIT_F */

    return rv; 
}


/*
 * Attach and detach dispatchable routines.
 */
int
_bcm_null_attach(int unit, char *subtype)
{
    COMPILER_REFERENCE(subtype);

    BCM_CONTROL(unit)->capability |= BCM_CAPA_LOCAL;

    return BCM_E_NONE;
}

int
_bcm_null_detach(int unit)
{
    return BCM_E_NONE;
}


#ifdef	BCM_LOOP_SUPPORT

STATIC int
_bcm_loop_match(int unit, char *subtype_a, char *subtype_b)
{
    COMPILER_REFERENCE(unit);
    return sal_strcmp(subtype_a, subtype_b);
}

int
_bcm_loop_attach(int unit, char *subtype)
{
    int		dunit;
    uint32	dcap;

    COMPILER_REFERENCE(subtype);

    dunit = BCM_CONTROL(unit)->unit;

    BCM_CONTROL(unit)->chip_vendor = BCM_CONTROL(dunit)->chip_vendor;
    BCM_CONTROL(unit)->chip_device = BCM_CONTROL(dunit)->chip_device;
    BCM_CONTROL(unit)->chip_revision = BCM_CONTROL(dunit)->chip_revision;
    dcap = BCM_CONTROL(dunit)->capability;
    dcap &= ~(BCM_CAPA_REMOTE|BCM_CAPA_COMPOSITE);
    BCM_CONTROL(unit)->capability |= BCM_CAPA_LOCAL | dcap;
    return BCM_E_NONE;
}

int
_bcm_loop_detach(int unit)
{
    return BCM_E_NONE;
}
#endif	/* BCM_LOOP_SUPPORT */

int
bcm_unit_valid(int unit)
{
    return BCM_UNIT_VALID(unit);
}

int
bcm_unit_local(int unit)
{
    return (BCM_UNIT_VALID(unit) && BCM_IS_LOCAL(unit));
}

int
bcm_unit_remote(int unit)
{
    return (BCM_UNIT_VALID(unit) && BCM_IS_REMOTE(unit));
}

int
bcm_unit_max(void)
{
    return BCM_CONTROL_MAX;
}

/*
 * Get the local reference of a remote device for its controlling
 * CPU.
 */

int
bcm_unit_remote_unit_get(int unit, int *remunit)
{
    if (!BCM_UNIT_VALID(unit)) {
        return BCM_E_NOT_FOUND;
    }

    if (remunit == NULL) {
        return BCM_E_PARAM;
    }

    *remunit = BCM_CONTROL(unit)->unit;

    return BCM_E_NONE;
}

/*
 * Get the subtype string that identifies the CPU controlling
 * the given unit.  subtype must point to a preallocated buffer;
 * maxlen is the maximum number of bytes that will be copied.
 *
 * returns the number of bytes copied or < 0 if an error occurs.
 *
 * NOTE:  This is currently the CPU key (mac address) as a
 * formatted string like 00:11:22:33:44:55.  It should be
 * converted to a CPU DB key with cpudb_key_parse.
 */

int
bcm_unit_subtype_get(int unit, char *subtype, int maxlen)
{
    int minlen;
    int stlen;

    if (!BCM_UNIT_VALID(unit)) {
        return BCM_E_NOT_FOUND;
    }

    if ((subtype == NULL) || (maxlen <= 0)) {
        return BCM_E_PARAM;
    }

    stlen = sal_strlen(BCM_CONTROL(unit)->subtype) + 1;
    minlen = maxlen < stlen ? maxlen : stlen;

    sal_memcpy(subtype, BCM_CONTROL(unit)->subtype, minlen);

    return minlen;
}
