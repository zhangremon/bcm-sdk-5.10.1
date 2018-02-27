/*
 * $Id: recovery.c 1.7 Broadcom SDK $
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
 * Internal FE2000 recovery API
 */


#ifdef BCM_WARM_BOOT_SUPPORT

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/g2p3/g2p3.h>

#include <bcm/error.h>
#include <bcm/debug.h>

#include <bcm_int/sbx/error.h>
#include <bcm_int/sbx/fe2000/allocator.h>
#include <bcm_int/sbx/fe2000/recovery.h>

/*
 * Debug Output Macros
 */
#define RCVRY_DEBUG_f(flags, stuff)  BCM_DEBUG(flags | BCM_DBG_ATTACH, stuff)
#define RCVRY_DEBUG(stuff)         RCVRY_DEBUG_f(0, stuff)
#define RCVRY_OUT(stuff)           BCM_DEBUG(0, stuff)
#define RCVRY_WARN(stuff)          RCVRY_DEBUG_f(BCM_DBG_WARN, stuff)
#define RCVRY_ERR(stuff)           RCVRY_DEBUG_f(BCM_DBG_ERR, stuff)
#define RCVRY_VERB(stuff)          RCVRY_DEBUG_f(BCM_DBG_VERBOSE, stuff)
#define RCVRY_VVERB(stuff)         RCVRY_DEBUG_f(BCM_DBG_VVERBOSE, stuff)


#define RCVRY_INIT_CHK(_u) \
    if (_recovery_state[_u] == NULL) { return BCM_E_INIT; }

#define RCVRY_LOCK_TAKE(_u) \
    if (sal_mutex_take(_recovery_state[_u]->lock, sal_mutex_FOREVER)) { \
        return BCM_E_INTERNAL; \
    }

#define RCVRY_LOCK_RELEASE(_u)  \
    if (sal_mutex_give(_recovery_state[_u]->lock)) { \
        return BCM_E_INTERNAL; \
    }

typedef int (*rc_var_read_f)(int unit, uint32 *val);
typedef int (*rc_var_write_f)(int unit, uint32 val);

/* the rc_var_entry_t defines the current value and the read/write accessors
 * for a recovery variable.  There is an entry for each 
 * _bcm_fe2000_recovery_var_t which is automatically configured by pre-
 * processor macros.  
 *
 * when a new variable is added, a read and write function must also be
 * defined in the form: fe2k_rcvr_<var>_(read|write)_f for macro insertion
 */
typedef struct rc_var_entry_s {
    uint32             val;
    rc_var_read_f      read_f;
    rc_var_write_f     write_f;
} rc_var_entry_t;

/* since many bcm modules may be reading the same data, the recovery module
 * will read and store the common table data for faster access.
 */
typedef struct rc_fast_pv2e_s {
    uint32               count; /* num_ports * max_vlan */
    uint32              *vlan;  /* all pv2e.vlan's */
    uint32              *lpi;   /* all pv2e.lpi's */
    
} rc_fast_pv2e_t;

typedef struct _bcm_fe2000_rcvry_state_s {
    sal_mutex_t          lock;
    rc_var_entry_t       entries[fe2k_rcvr_num_elements];
    rc_fast_pv2e_t       fast_pv2e;

    rc_non_tb_vsi_t      non_tb_vsi;
}  _bcm_fe2000_rcvry_state_t;

static _bcm_fe2000_rcvry_state_t *_recovery_state[BCM_MAX_NUM_UNITS];

/*
 * The recovery module is the central location where all spare bits are tracked
 * and used for the purposes of a level 1 warm reboot (warm reboot with out 
 * external storage).  The g2p3 memory locations used to store the otherwise
 * unrecoverable state are chosen because the cannot be used for any reason.
 * 
 * The drop vlan, VSI 0, for example, has many implicit resources that are
 * unused during forwarding.  These resources are used to store software state.
 */

/*
 * Available resources that are not used by g2p3 design during normal 
 * processing:
 *    v2e[0]
 *      .dontlearn         = 1  /required by ucode(?)
 *      .pim               - avail
 *      .igmp              - avail
 *      .forceflood        - avail
 *      .dropunksmac       - avail
 *      .vrf               - fe2k_rcvr_default_vid
 *      .v6mcmode          - avail
 *      .v4mcmode          - avail
 *      .laghash           - avail
 *      .v4route           - avail
 *    ft[soc_sbx_g2p3_vlan_ft_base_get()]  - unicast flood FT
 *      .rridx             - avail
 *      .excidx            = 0x7f  required by ucode
 *      .lenadj            - avail
 *      .mc                - avail
 *      .t                 - avail
 *      .hc                = 0x0 (header capture, shd keep at 0?)
 *      .oi                - avail
 *      .qid               - fe2k_rcvr_default_stg (12b)
 *      .lag               - avail
 *    ft[soc_sbx_g2p3_vlan_ft_base_get() + soc_sbx_g2p3_mc_ft_offset_get()]
 *      .rridx             - avail
 *      .excidx            = 0x7f  required by ucode
 *      .lenadj            - avail
 *      .mc                - avail
 *      .t                 - avail
 *      .hc                = 0x0 (header capture, shd keep at 0?)
 *      .oi                - avail
 *      .qid               - avail
 *      .lag               - avail
 */

/*****************************************************************************
 * Start of recovery var read/write accessors
 * These accessors read/write the given recovery variable from/to unused
 * g2p3 table locations.  They all follwo the form
 *   fe2k_rcvr_<var>_(read|write)_f for macro insertion
 *****************************************************************************/

static int
fe2k_rcvr_default_vid_read_f(int unit, uint32 *val) 
{
    soc_sbx_g2p3_v2e_t v2e;
    int                rv;

    rv = soc_sbx_g2p3_v2e_get(unit, 0, &v2e);
    if (BCM_FAILURE(rv)) {
        RCVRY_ERR((_SBX_D(unit, "failed to read v2e[0]:%s\n"), 
                   bcm_errmsg(rv)));
        return rv;
    }
    
    *val = v2e.vrf;
    return rv;
}
 
static int
fe2k_rcvr_default_vid_write_f(int unit, uint32 val)
{
    int                rv;
    soc_sbx_g2p3_v2e_t v2e;
   
    rv = soc_sbx_g2p3_v2e_get(unit, 0, &v2e);
    if (BCM_FAILURE(rv)) {
        RCVRY_ERR((_SBX_D(unit, "failed to read v2e[0]:%s\n"), 
                   bcm_errmsg(rv)));
        return rv;
    }
    
    v2e.vrf = val;

    rv = soc_sbx_g2p3_v2e_set(unit, 0, &v2e);
    if (BCM_FAILURE(rv)) {
        RCVRY_ERR((_SBX_D(unit, "failed to write v2e[0]:%s\n"), 
                   bcm_errmsg(rv)));
        return rv;
    }

    return rv;
}


static int
fe2k_rcvr_default_stg_read_f(int unit, uint32 *val)
{
    int rv;
    uint32 fti;
    soc_sbx_g2p3_ft_t ft;
    
    rv = soc_sbx_g2p3_vlan_ft_base_get(unit, &fti);
    if (BCM_FAILURE(rv)) {
        RCVRY_ERR((_SBX_D(unit, "failed to get flood fti:%s\n"), 
                   bcm_errmsg(rv)));
        return rv;   
    }
    
    rv = soc_sbx_g2p3_ft_get(unit, fti, &ft);
    if (BCM_FAILURE(rv)) {
        RCVRY_ERR((_SBX_D(unit, "failed to get flood ft[0x%04x]:%s\n"), 
                   fti, bcm_errmsg(rv)));
        return rv;   
    }
    
    *val = ft.qid;
    return rv;
}

static int
fe2k_rcvr_default_stg_write_f(int unit, uint32 val)
{
    int rv;
    uint32 fti;
    soc_sbx_g2p3_ft_t ft;
    
    rv = soc_sbx_g2p3_vlan_ft_base_get(unit, &fti);
    if (BCM_FAILURE(rv)) {
        RCVRY_ERR((_SBX_D(unit, "failed to get flood fti:%s\n"), 
                   bcm_errmsg(rv)));
        return rv;   
    }
    
    rv = soc_sbx_g2p3_ft_get(unit, fti, &ft);
    if (BCM_FAILURE(rv)) {
        RCVRY_ERR((_SBX_D(unit, "failed to get flood ft[0x%04x]:%s\n"), 
                   fti, bcm_errmsg(rv)));
        return rv;   
    }
    
    ft.qid = val;
    
    rv = soc_sbx_g2p3_ft_set(unit, fti, &ft);
    if (BCM_FAILURE(rv)) {
        RCVRY_ERR((_SBX_D(unit, "failed to set flood ft[0x%04x]:%s\n"), 
                   fti, bcm_errmsg(rv)));
        return rv;   
    }

    return rv;
}

/*****************************************************************************
 *  End recovery var read/write accessors
 *****************************************************************************/


static void
rc_non_tb_free(dq_t *elt, int unit)
{
    rc_non_tb_vsi_t *non_tb;
    non_tb = DQ_ELEMENT_GET(rc_non_tb_vsi_t*, elt, dql);
    RCVRY_WARN((_SBX_D(unit, "Unprocessed non-tb vsi on pv2e[%d, 0x%03x]; vsi=0x%04x\n"),
                non_tb->port, non_tb->vid, 
                _recovery_state[unit]->fast_pv2e.vlan[ 
                    RCVR_FAST_PV2E_IDX(non_tb->port, non_tb->vid)]));

    sal_free(non_tb);
}

static int
fe2k_rcvr_mem_free(int unit)
{
    rc_fast_pv2e_t      *fast_pv2e;

    fast_pv2e = &_recovery_state[unit]->fast_pv2e;

    if (fast_pv2e->lpi) {
        sal_free(fast_pv2e->lpi);
        fast_pv2e->lpi = NULL;
    }
    
    if (fast_pv2e->vlan) {
        sal_free(fast_pv2e->vlan);
        fast_pv2e->vlan = NULL;
    }
    return BCM_E_NONE;
}


static int
fe2k_rcvr_fast_mems_get(int unit) 
{
    int                rv;
    rc_fast_pv2e_t      *fast_pv2e;

    fast_pv2e = &_recovery_state[unit]->fast_pv2e;

    fast_pv2e->count = BCM_VLAN_COUNT * MAX_PORT(unit);

    fast_pv2e->lpi = sal_alloc(sizeof(uint32_t) * fast_pv2e->count,
                               "fast_lps");
    if (fast_pv2e->lpi == NULL) {
        rv = BCM_E_MEMORY;
        goto done;
    }

    fast_pv2e->vlan = sal_alloc(sizeof(uint32_t) * fast_pv2e->count,
                                "fast_vlans");
    if (fast_pv2e->vlan == NULL) {
        rv = BCM_E_MEMORY;
        goto done;
    }
    rv = soc_sbx_g2p3_pv2e_lpi_fast_get(unit, 0, 0, BCM_VLAN_MAX, 
                                        MAX_PORT(unit) - 1,
                                        fast_pv2e->lpi, fast_pv2e->count);
    if (BCM_FAILURE(rv)) {
        RCVRY_ERR((_SBX_D(unit, "Failed to read pv2e[*].lpi:%s\n"), 
                   bcm_errmsg(rv)));
        goto done;
    }

    rv = soc_sbx_g2p3_pv2e_vlan_fast_get(unit, 0, 0, BCM_VLAN_MAX, 
                                         MAX_PORT(unit) - 1,
                                         fast_pv2e->vlan, fast_pv2e->count);
    if (BCM_FAILURE(rv)) {
        RCVRY_ERR((_SBX_D(unit, "pv2e_vlan_fast_get failed: %s\n"),
                   bcm_errmsg(rv)));
        goto done;
    }
    
done:
    if (BCM_FAILURE(rv)) {
        fe2k_rcvr_mem_free(unit);
    }

    return rv;
}

/*
 * Function:
 *   _bcm_fe2000_recovery_init
 * Purpose:
 *   Initialize the internal recovery module
 * Parameters:
 *   unit           - bcm device number
 * Returns:
 *   BCM_E_*
 */
int
_bcm_fe2000_recovery_init(int unit)
{
    int                rv = BCM_E_NONE;
    int                idx;
    rc_var_entry_t    *entries;
    rc_fast_pv2e_t    *fast_pv2e;


    if (_recovery_state[unit] == NULL) {
        _recovery_state[unit] = sal_alloc(sizeof(*_recovery_state[unit]), 
                                          "recovery_state");
        if (_recovery_state[unit] == NULL) {
            return BCM_E_MEMORY;
        }

        sal_memset(_recovery_state[unit], 0, sizeof(*_recovery_state[unit]));
        
        _recovery_state[unit]->lock = sal_mutex_create("_bcm_recovery_lock");
        if (_recovery_state[unit]->lock == NULL) {
            return BCM_E_MEMORY;
        }

        /* Setup the read/write functions for each recovery variable 
         * automatically by including the variable list with a macro to 
         * place the var and configure the read/write functions
         */
        entries = _recovery_state[unit]->entries;
#define RECOVERY_ENTRY_VAR_PLACE( var ) \
    entries[var].read_f = var##_read_f; \
    entries[var].write_f = var##_write_f;
#include <bcm_int/sbx/fe2000/recovery_vars.h>

        
        DQ_INIT(&_recovery_state[unit]->non_tb_vsi.dql);
    }

    /* Get and hold all common tables using the fast_ interface */
    rv = fe2k_rcvr_fast_mems_get(unit);
    if (BCM_FAILURE(rv)) {
        return rv;
    }
    fast_pv2e = &_recovery_state[unit]->fast_pv2e;

    
    for (idx=0; idx < fast_pv2e->count; idx++) {
        if (fast_pv2e->lpi[idx] < MAX_PORT(unit)) {
            continue;
        }

        RCVRY_VERB((_SBX_D(unit, "Found lpi 0x%x at idx %d\n"), 
                    fast_pv2e->lpi[idx], idx));
        rv = _sbx_gu2_resource_alloc(unit, SBX_GU2K_USR_RES_LPORT, 1, 
                                     &fast_pv2e->lpi[idx],
                                     _SBX_GU2_RES_FLAGS_RESERVE);   

        /* Logical ports may be shared by many port/vids; ignore allocation
         * failures due to the resource being reserved already
         */
        if (rv == BCM_E_BUSY || BCM_E_EXISTS) {
            rv = BCM_E_NONE;
        }
        if (BCM_FAILURE(rv)) {
           RCVRY_ERR((_SBX_D(unit, "Failed to allocate LP[0x%04x]:%s\n"), 
                      fast_pv2e->lpi[idx], bcm_errmsg(rv)));
           break;
        }
    }

    return rv;
}


/*
 * Function:
 *   _bcm_fe2000_recovery_commit
 * Purpose:
 *   Commit recovery data after a cold boot
 * Parameters:
 *   unit           - bcm device number
 * Returns:
 *   BCM_E_*
 */
int
_bcm_fe2000_recovery_commit(int unit)
{
    int tv, rv, idx;
    rc_var_entry_t *entries = _recovery_state[unit]->entries;

    RCVRY_VERB((_SBX_D(unit, "%s : Commiting recovery data\n"), FUNCTION_NAME()));

    /* Finalize the data after init; some modules will stomp on the overloaded
     * bits used for warmboot storage.  This sequece will re-set them after a
     * cold boot init
     */
    rv = BCM_E_NONE;
    for (idx=0; idx < fe2k_rcvr_num_elements; idx++) {
        if (entries[idx].write_f) {
            tv = entries[idx].write_f(unit, entries[idx].val);
            if (BCM_FAILURE(tv)) {
                rv = tv;
                RCVRY_ERR((_SBX_D(unit, "Failed to commit idx %d:%s\n"), 
                           idx, bcm_errmsg(rv)));
            }
        }
    }

    return rv;
}

/*
 * Function:
 *   _bcm_fe2000_recovery_done
 * Purpose:
 *   Clean up temporal data after a recovery process
 * Parameters:
 *   unit           - bcm device number
 * Returns:
 *   BCM_E_*
 */
int
_bcm_fe2000_recovery_done(int unit)
{
    int rv;

    rv = BCM_E_NONE;
    RCVRY_VERB((_SBX_D(unit, "%s : Cleanning up recovery data\n"), FUNCTION_NAME()));

    DQ_MAP(&_recovery_state[unit]->non_tb_vsi.dql, rc_non_tb_free, unit);
    fe2k_rcvr_mem_free(unit);

    return rv;
}

/*
 * Function:
 *   _bcm_fe2000_recovery_deinit
 * Purpose:
 *   Release all allocated resources
 * Parameters:
 *   unit           - bcm device number
 * Returns:
 *   BCM_E_*
 */
int
_bcm_fe2000_recovery_deinit(int unit)
{
    int rv = BCM_E_NONE;

    if (_recovery_state[unit] == NULL) {
        return BCM_E_NONE;
    }

    if (_recovery_state[unit]->lock) {
        sal_mutex_destroy(_recovery_state[unit]->lock);
    }
    sal_free(_recovery_state[unit]);
    _recovery_state[unit] = NULL;
    return rv;
}


static int
fe2k_rcvr_var_get(int unit, _bcm_fe2000_recovery_var_t var, uint32 *val)
{
    rc_var_entry_t *entries = _recovery_state[unit]->entries;

    if (entries[var].read_f) {
        return entries[var].read_f(unit, val);
    }

    return BCM_E_UNAVAIL;
}

static int
fe2k_rcvr_var_set(int unit, _bcm_fe2000_recovery_var_t var, uint32 val)
{
    int rv = BCM_E_PARAM;
    rc_var_entry_t *entries = _recovery_state[unit]->entries;

    if (entries[var].write_f) {
        rv = entries[var].write_f(unit, val);
    }
    
    if (BCM_SUCCESS(rv)) {
        entries[var].val = val;
    }

    return rv;
}


int 
_bcm_fe2000_recovery_var_get(int unit, 
                             _bcm_fe2000_recovery_var_t var, uint32 *val)
{
    int rv;
    
    if (var >= fe2k_rcvr_num_elements) {
        return BCM_E_PARAM;
    }
    RCVRY_INIT_CHK(unit);    
    RCVRY_LOCK_TAKE(unit);
    rv = fe2k_rcvr_var_get(unit, var, val);
    RCVRY_LOCK_RELEASE(unit);
    return rv;
}


int
_bcm_fe2000_recovery_var_set(int unit, 
                             _bcm_fe2000_recovery_var_t var, uint32 val)
{
    int rv;

    if (var >= fe2k_rcvr_num_elements) {
        return BCM_E_PARAM;
    }
    RCVRY_INIT_CHK(unit);
    RCVRY_LOCK_TAKE(unit);
    rv = fe2k_rcvr_var_set(unit, var, val);
    RCVRY_LOCK_RELEASE(unit);
    return rv;
}



uint32 *
_bcm_fe2000_recovery_pv2e_vlan_get(unit)
{
    return _recovery_state[unit]->fast_pv2e.vlan;
}

uint32 *
_bcm_fe2000_recovery_pv2e_lpi_get(unit)
{
    return _recovery_state[unit]->fast_pv2e.lpi;
}


int
_bcm_fe2000_recovery_non_tb_vsi_insert(int unit, uint16 port, uint16 vid)
{
    rc_non_tb_vsi_t *non_tb;
    RCVRY_VERB((_SBX_D(unit, "Recording non-tb VSI at %d/0x%03x:  0x%04x\n"),
                port, vid, 
                _recovery_state[unit]->
                fast_pv2e.vlan[RCVR_FAST_PV2E_IDX(port,vid)]));
    
    non_tb = sal_alloc(sizeof(*non_tb), "nt");
    if (non_tb == NULL) {
        return BCM_E_MEMORY;
    }
    non_tb->port = port;
    non_tb->vid  = vid;

    DQ_INSERT_TAIL(&_recovery_state[unit]->non_tb_vsi.dql, non_tb);
    
    return BCM_E_NONE;
}

int _bcm_fe2000_recovery_non_tb_vsi_head_get(int unit, rc_non_tb_vsi_t **head)
{
    *head = &_recovery_state[unit]->non_tb_vsi;
    return BCM_E_NONE;
}

#endif /* BCM_WARM_BOOT_SUPPORT */
