/*
 * $Id: ipmc.c 1.22 Broadcom SDK $
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
 * File: 	ipmc.c
 * Purpose: 	Tracks and manages IPMC tables.
 */

#ifdef INCLUDE_L3

#include <sal/core/libc.h>

#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/cm.h>
#include <soc/l3x.h>

#include <bcm/error.h>

#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/lynx.h>

#define REPL_PORT_CHECK(unit, port) \
    if (!IS_E_PORT(unit, port)) { return BCM_E_PARAM; }

typedef struct _lynx_repl_port_info_s {
    uint32 enabled;
    uint32 num_groups;
    int32 *vlan_count; /* # VLANs the port repl to */
    uint32 ls_entries_used[_SHR_BITDCLSIZE(BCM_VLAN_COUNT)]; /* used LS entry */
} _lynx_repl_port_info_t;

static _lynx_repl_port_info_t **_lynx_repl_info[BCM_MAX_NUM_UNITS];

#define LYNX_IPMC_INIT(_unit_, _port_) \
        if (NULL == _lynx_repl_info[_unit_][_port_]) { return BCM_E_INIT; }

#define LYNX_IPMC_ID(_unit_, _port_, _id_) \
        if ((_id_ < 0) || \
            (_id_ >= _lynx_repl_info[_unit_][_port_]->num_groups)) \
            { return BCM_E_PARAM; }

#define LYNX_IPMC_REPL_INFO(_unit_, _port_) \
	_lynx_repl_info[_unit_][_port_]
#define LYNX_IPMC_REPL_ENABLED(_unit_, _port_) \
	_lynx_repl_info[_unit_][_port_]->enabled
#define LYNX_IPMC_REPL_VLAN_COUNT(_unit_, _port_, _group_) \
	_lynx_repl_info[_unit_][_port_]->vlan_count[_group_]
#define LYNX_IPMC_REPL_LS_USED_SET(_unit_, _port_, _i_) \
	SHR_BITSET(_lynx_repl_info[_unit_][_port_]->ls_entries_used, _i_)
#define LYNX_IPMC_REPL_LS_USED_CLR(_unit_, _port_, _i_) \
        SHR_BITCLR(_lynx_repl_info[_unit_][_port_]->ls_entries_used, _i_)

#define IPMC_LOCK(unit) \
        soc_mem_lock(unit, L3_IPMCm)
#define IPMC_UNLOCK(unit) \
        soc_mem_unlock(unit, L3_IPMCm)

#define IPMC_REPL_LOCK(unit)                   \
    {                                          \
        soc_mem_lock(unit, EGRESS_IPMC_LSm);   \
        soc_mem_lock(unit, EGRESS_IPMC_MSm);   \
    }
#define IPMC_REPL_UNLOCK(unit)                 \
    {                                          \
        soc_mem_unlock(unit, EGRESS_IPMC_MSm); \
        soc_mem_unlock(unit, EGRESS_IPMC_LSm); \
    }

typedef struct _lynx_repl_link_s {
    egress_ipmc_ls_entry_t ls_entry;
    int read_index;
    int write_index;
} _lynx_repl_link_t;

#define LYNX_REPL_MAX_LINKS            64

/*
 * Function:
 *	bcm_lynx_ipmc_repl_init
 * Purpose:
 *	Initialize IPMC replication.
 * Parameters:
 *	unit     - SOC unit #
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_lynx_ipmc_repl_init(int unit)
{
    bcm_port_t		port;
    int			alloc_size;

    bcm_lynx_ipmc_repl_detach(unit);

    alloc_size = SOC_MAX_NUM_PORTS * sizeof(_lynx_repl_port_info_t *);
    _lynx_repl_info[unit] = sal_alloc(alloc_size, "IPMC repl ports");
    if (_lynx_repl_info[unit] == NULL) {
        return BCM_E_MEMORY;
    }
    sal_memset(_lynx_repl_info[unit], 0, alloc_size);

    PBMP_ITER(PBMP_XE_ALL(unit), port) {
        alloc_size = sizeof(_lynx_repl_port_info_t);
        LYNX_IPMC_REPL_INFO(unit, port) =
            sal_alloc(alloc_size, "IPMC repl entries free");
        if (LYNX_IPMC_REPL_INFO(unit, port) == NULL) {
	    bcm_lynx_ipmc_repl_detach(unit);
            return BCM_E_MEMORY;
        }
        sal_memset(LYNX_IPMC_REPL_INFO(unit, port), 0, alloc_size);

        LYNX_IPMC_REPL_INFO(unit, port)->num_groups = 
            soc_mem_index_count(unit, EGRESS_IPMC_MSm);
        alloc_size = 
            LYNX_IPMC_REPL_INFO(unit, port)->num_groups * sizeof(int32);
        LYNX_IPMC_REPL_INFO(unit, port)->vlan_count =
            sal_alloc(alloc_size, "IPMC repl VLAN counts");
        if (LYNX_IPMC_REPL_INFO(unit, port)->vlan_count == NULL) {
	    bcm_lynx_ipmc_repl_detach(unit);
            return BCM_E_MEMORY;
        }
        sal_memset(LYNX_IPMC_REPL_INFO(unit, port)->vlan_count,
                   0, alloc_size);
    }

    SOC_IF_ERROR_RETURN
	(soc_mem_clear(unit, EGRESS_IPMC_MSm, COPYNO_ALL, FALSE));

    return BCM_E_NONE;
}

int
_bcm_lynx_ipmc_repl_enabled(int unit, bcm_port_t port)
{
    LYNX_IPMC_INIT(unit, port); 
    return LYNX_IPMC_REPL_ENABLED(unit, port);
}

/*
 * Function:
 *	_bcm_lynx_ipmc_repl_enable
 * Purpose:
 *	Internal routine to enable/disable IPMC replication
 */

int
_bcm_lynx_ipmc_repl_enable(int unit, bcm_port_t port, int enable)
{
    uint32		cfg2;

    LYNX_IPMC_INIT(unit, port); 
    SOC_IF_ERROR_RETURN(READ_EGR_IPMC_CFG2r(unit, port, &cfg2));
    soc_reg_field_set(unit, EGR_IPMC_CFG2r, &cfg2,
                      EGR_DISABLE_IPMC_REPLICATIONf, !enable);
    SOC_IF_ERROR_RETURN(WRITE_EGR_IPMC_CFG2r(unit, port, cfg2));

    LYNX_IPMC_REPL_ENABLED(unit, port) = enable;

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_lynx_ipmc_repl_detach
 * Purpose:
 *	Detach IPMC replication module.
 * Parameters:
 *	unit     - SOC unit #
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_lynx_ipmc_repl_detach(int unit)
{
    int			rv = BCM_E_NONE;
    bcm_port_t		port;

    if (_lynx_repl_info[unit] != NULL) {
	PBMP_ITER(PBMP_XE_ALL(unit), port) {
	    if (LYNX_IPMC_REPL_INFO(unit, port) != NULL) {
		if (rv >= 0) {
		    rv = _bcm_lynx_ipmc_repl_enable(unit, port, FALSE);
		}

                if (LYNX_IPMC_REPL_INFO(unit, port)->vlan_count != NULL) {
                    sal_free(LYNX_IPMC_REPL_INFO(unit, port)->vlan_count);
                }
		sal_free(LYNX_IPMC_REPL_INFO(unit, port));
	    }
	}

	sal_free(_lynx_repl_info[unit]);
	_lynx_repl_info[unit] = NULL;
    }

    return rv;
}

/*
 * Function:
 *	_bcm_lynx_ipmc_repl_next_free_ptr
 * Purpose:
 *	Internal routine to return an available slot
 */

static int
_bcm_lynx_ipmc_repl_next_free_ptr(int unit, bcm_port_t port)
{
    int			ix, bit;
    uint32		not_ptrs;

    LYNX_IPMC_INIT(unit, port); 
    for (ix = 0; ix < BCM_VLAN_COUNT / 32; ix++) {
        not_ptrs = ~_lynx_repl_info[unit][port]->ls_entries_used[ix];
        if (not_ptrs) {
            for (bit = 0; bit < 32; bit++) {
                if (not_ptrs & (1 << bit)) {
                    return (ix * 32) + bit;
                }
            }
        }
    }

    return -1;
}

/*
 * Function:
 *	bcm_lynx_ipmc_repl_get
 * Purpose:
 *	Return set of VLANs selected for port's replication list for chosen
 *	IPMC group.
 * Parameters:
 *	unit     - StrataSwitch PCI device unit number.
 *	group    - The MC index number.
 *	port     - port to list VLANs.
 *	vlan_vec - (OUT) vector of replicated VLANs common to selected ports.
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_lynx_ipmc_repl_get(int unit, int group, bcm_port_t port,
                       bcm_vlan_vector_t vlan_vec)
{
    int			rv = BCM_E_NONE;
    uint32		ms_read_bits[2];
    uint32		ls_bits[2];
    egress_ipmc_ls_entry_t ls_entry;
    egress_ipmc_ms_entry_t ms_entry;
    int			ls_ptr, ms_bit;
    int			blk = SOC_PORT_BLOCK(unit, port);

    LYNX_IPMC_INIT(unit, port); 
    REPL_PORT_CHECK(unit, port);
    LYNX_IPMC_ID(unit, port, (uint32)group);

    BCM_VLAN_VEC_ZERO(vlan_vec);

    IPMC_REPL_LOCK(unit);

    if (LYNX_IPMC_REPL_VLAN_COUNT(unit, port, group)) {
        sal_memset(&ms_entry, 0, sizeof(ms_entry));
        if ((rv = READ_EGRESS_IPMC_MSm(unit, blk, group, &ms_entry)) < 0) {
            goto get_done;
        }
        soc_EGRESS_IPMC_MSm_field_get(unit, &ms_entry, MS_VECTORf,
                                      ms_read_bits);
        ls_ptr = soc_EGRESS_IPMC_MSm_field32_get(unit, &ms_entry,
                                                 HEAD_POINTERf);

        for (ms_bit = 0; ms_bit < 64; ms_bit++) {
            if (ms_read_bits[ms_bit / 32] & (1 << (ms_bit % 32))) {
                sal_memset(&ls_entry, 0, sizeof(ls_entry));
                if ((rv = READ_EGRESS_IPMC_LSm(unit, blk,
                                               ls_ptr, &ls_entry)) < 0) {
                    goto get_done;
                }
                soc_EGRESS_IPMC_LSm_field_get(unit, &ls_entry, LS_VECTORf,
                                              ls_bits);
                vlan_vec[2 * ms_bit + 0] = ls_bits[0];
                vlan_vec[2 * ms_bit + 1] = ls_bits[1];
                ls_ptr = soc_EGRESS_IPMC_LSm_field32_get(unit, &ls_entry,
                                                         NEXT_POINTERf);
            }
        }
    }

 get_done:
    IPMC_REPL_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *	_bcm_lynx_ipmc_repl_disable_port_check
 * Purpose:
 *	Internal routine to clear replication on a port if there
 *	are not more than one vlans.
 */

int
_bcm_lynx_ipmc_repl_disable_port_check(int unit, bcm_port_t port)
{
    uint32			mcgroup;

    LYNX_IPMC_INIT(unit, port); 
    for (mcgroup = 0; 
         mcgroup < LYNX_IPMC_REPL_INFO(unit, port)->num_groups; mcgroup++) {
        if (LYNX_IPMC_REPL_VLAN_COUNT(unit, port, mcgroup) > 1) {
            return BCM_E_NONE;
        }
    }

    return _bcm_lynx_ipmc_repl_enable(unit, port, FALSE);
}

/*
 * This link list get function reads the train of LS entries for a given MS
 * entry and stores them in an array.  This array is used to perform the
 * list rewrite for safe reconfiguration without disabling replication on
 * the port.
 *
 * The array holds the entry data, records the read index of the LS table,
 * and has a spot for the write location determined by the write function
 * below.
 */

STATIC int
_bcm_lynx_ipmc_repl_link_list_get(int unit, int blk,
                                  egress_ipmc_ms_entry_t *ms_entry_ptr,
                                  int target_ms_bit, 
                                  _lynx_repl_link_t *link_list,
                                  int *list_len)
{
    int	                ms_bit, list_index = 0;
    uint32		ms_read_bits[2];
    egress_ipmc_ls_entry_t *ls_entry;
    int			ls_ptr;

    sal_memset(link_list, 0,
               sizeof(_lynx_repl_link_t) * LYNX_REPL_MAX_LINKS);
    
    soc_EGRESS_IPMC_MSm_field_get(unit, ms_entry_ptr, MS_VECTORf,
                                  ms_read_bits);
    ls_ptr = soc_EGRESS_IPMC_MSm_field32_get(unit, ms_entry_ptr,
                                             HEAD_POINTERf);

    for (ms_bit = 0; ms_bit < 64; ms_bit++) {
        if (ms_read_bits[ms_bit / 32] & (1 << (ms_bit % 32))) {
            ls_entry = &(link_list[list_index].ls_entry);
            BCM_IF_ERROR_RETURN
                (READ_EGRESS_IPMC_LSm(unit, blk, ls_ptr, ls_entry));
            link_list[list_index].read_index = ls_ptr;
            ls_ptr = soc_EGRESS_IPMC_LSm_field32_get(unit, ls_entry,
                                                     NEXT_POINTERf);
            list_index++;
        }

        if (ms_bit == target_ms_bit) {
            *list_len = list_index;
            break;
        }
    }

    return BCM_E_NONE;
}

/*
 * This link list write function does the following things:
 *
 * 1) Working from the end of the list backward, it allocates a slot in
 *    the LS table and copies that into the next pointer of the previous
 *    link in the list.
 *
 * 2) If it succeeds, continue on to the writing phase.
 *
 * 3) If it runs out of available LS table slots, but satisfied the
 *    minimum necessary (1 for insert, 0 for delete), then it moves to
 *    the change in place logic (do_disable):
 *
 *    a) Determine the minimum number of elements to write:
 *       1 for delete - to link to the remainder of the list
 *       1 for head of list insert - the new entry
 *       2 for mid-list insert - new entry, and previous entry to link
 *    b) Undo the allocations for any other links which were given entries.
 *    c) Disable replication on the port, and note for calling function.
 *    d) Record write entry as read entry linking changes.
 *
 * 4) Write out new/overwritten entries to LS table.
 *
 * 5) If we did not have to disable replication, then we copied out the
 *    whole list.  Now we need to free up the previous entries.
 *    - If any of the writes failed, release the allocated entries.
 *
 * Notes:
 *    Head of list insert or delete cannot generate do_disable from the
 *    first loop, thus the list_write_start logic is safely >= 0.
 *
 *    Head of list delete has a list_len of 0, so all actions are skipped.
 *    The inserted entry must have its read_index = -1.  This is handled
 *    in the _add function below.
 *
 *    The deleted entry is released in the _delete function below.
 *
 *    Linking the MS entry to the new list is handled in the calling
 *    function after this has succeeded, if a new list was written.
 *
 *    It is OK to set *enable=TRUE in the do_disable area, since we
 *    we must have at least two entries on an to reach that logic
 *    on an insert, and delete double-checks afterward.
 */

STATIC int
_bcm_lynx_ipmc_repl_link_list_write(int unit, int port, int list_len,
                                    _lynx_repl_link_t *link_list,
                                    int insert, int *enable)
{
    int			blk = SOC_PORT_BLOCK(unit, port);
    int                 list_index, ls_ptr = -1, list_write_start = 0;
    int                 rv, do_disable = FALSE;

    for (list_index = list_len - 1; list_index >= 0; list_index--) {
        /* Record next link */
        if (list_index != (list_len - 1)) {
            soc_EGRESS_IPMC_LSm_field32_set(unit, 
                       &(link_list[list_index].ls_entry),
                                            NEXT_POINTERf, ls_ptr);
        }

        /* Find entry for this link */
        if ((ls_ptr = _bcm_lynx_ipmc_repl_next_free_ptr(unit, port)) < 0) {
            if (insert && (list_index == (list_len - 1))) {
                /* No room for inserted entry */
                return BCM_E_RESOURCE;
            }
            /* Out of space, unroll */
            do_disable = TRUE;
            break;
        }

        LYNX_IPMC_REPL_LS_USED_SET(unit, port, ls_ptr);
        link_list[list_index].write_index = ls_ptr;
    }

    list_index++;

    if (do_disable) {
        list_write_start = list_len - 1;
        if (insert) {
            list_write_start--;
        }

        /* 
         * Start from where we left off since those were allocated.
         * We might have allocated the new insert link, but not the
         * pointer before it.  Then list_write_start will be less than
         * list_index at this point.
         */
        for (;list_index < list_write_start; list_index++) {
            /* Free above allocated links */
            LYNX_IPMC_REPL_LS_USED_CLR(unit, port,
                           link_list[list_index].write_index);
        }

        if ((rv = _bcm_lynx_ipmc_repl_enable(unit, port, FALSE)) < 0) {
            /* Failed to disable, abort */
            for (;list_index < list_len; list_index++) {
                LYNX_IPMC_REPL_LS_USED_CLR(unit, port,
                               link_list[list_index].write_index);
            }
            return rv;
        }
        *enable = TRUE;

        /* Overwrite in place */
        link_list[list_write_start].write_index =
            link_list[list_write_start].read_index;
    }

    rv = BCM_E_NONE;
    /* Write out the linked list of LS entries */
    for (list_index = list_write_start;
         list_index < list_len; list_index++) {
        if ((rv = WRITE_EGRESS_IPMC_LSm(unit, blk, 
                          link_list[list_index].write_index,
                          &(link_list[list_index].ls_entry))) < 0) {
            break;
        }
    }

    /* 
     * Note that we are in IPMC_REPL lock, so we can free the entries
     * before we write the MS, since no one else can get here until we do
     */
    if (!do_disable) { /* If we disabled, we already cleaned up above */
        for (list_index = list_write_start;
             list_index < list_len; list_index++) {
            if (rv < 0) { /* If we failed to write, free the write entries */
                LYNX_IPMC_REPL_LS_USED_CLR(unit, port,
                                  link_list[list_index].write_index);
            } else { /* Else, free the read entries */
                if (link_list[list_index].read_index >= 0) {
                    /* -1 for inserted entry, see in _add below */
                    LYNX_IPMC_REPL_LS_USED_CLR(unit, port,
                                      link_list[list_index].read_index);
                }
            }
        }
    }

    return rv;
}

/*
 * Function:
 *	bcm_lynx_ipmc_repl_add
 * Purpose:
 *	Add a VLAN to selected port's replication list for specified
 *	IPMC group.
 * Parameters:
 *	unit     - StrataSwitch PCI device unit number.
 *	group    - The MC index number.
 *	port     - port to which VLAN is added.
 *	vlan     - VLAN selected to add.
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_lynx_ipmc_repl_add(int unit, int group, bcm_port_t port, bcm_vlan_t vlan)
{
    int			rv = BCM_E_NONE;
    uint32		ms_read_bits[2];
    uint32		ls_bits[2];
    egress_ipmc_ls_entry_t *ls_entry, *prev_ls_entry;
    egress_ipmc_ms_entry_t ms_entry;
    _lynx_repl_link_t   link_list[LYNX_REPL_MAX_LINKS];
    int			target_ms_bit, ls_bit;
    int			ls_ptr, list_index, list_len;
    int			blk = SOC_PORT_BLOCK(unit, port);
    int                 enable = FALSE, write_ms = FALSE;

    LYNX_IPMC_INIT(unit, port); 
    REPL_PORT_CHECK(unit, port);
    LYNX_IPMC_ID(unit, port, (uint32)group);

    IPMC_REPL_LOCK(unit);
    sal_memset(&ms_entry, 0, sizeof(ms_entry));
    if ((rv = READ_EGRESS_IPMC_MSm(unit, blk, group, &ms_entry)) < 0) {
        goto add_done;
    }

    soc_EGRESS_IPMC_MSm_field_get(unit, &ms_entry, MS_VECTORf, ms_read_bits);

    target_ms_bit = (vlan >> 6) & 0x3f;
    ls_bit = vlan & 0x3f;

    /* Fill out link_list structure */
    if ((rv = _bcm_lynx_ipmc_repl_link_list_get(unit, blk, &ms_entry,
                                  target_ms_bit, link_list, &list_len)) < 0) {
        goto add_done;
    }

    if (ms_read_bits[target_ms_bit / 32] & (1 << (target_ms_bit % 32))) {
        /* Safe! Add to existing LS entry */
        assert(list_len > 0);
        list_index = list_len - 1;
        ls_entry = &(link_list[list_index].ls_entry);
        soc_EGRESS_IPMC_LSm_field_get(unit, ls_entry, LS_VECTORf, ls_bits);
        if (ls_bits[ls_bit / 32] & (1 << (ls_bit % 32))) {
            /* Already exists! */
            rv = BCM_E_EXISTS;
            goto add_done;
        }
        ls_bits[ls_bit / 32] |= 1 << (ls_bit % 32);
        soc_EGRESS_IPMC_LSm_field_set(unit, ls_entry, LS_VECTORf, ls_bits);
        /* Write it where we read it from */
        ls_ptr = link_list[list_index].read_index;
        if ((rv = WRITE_EGRESS_IPMC_LSm(unit, blk, ls_ptr, ls_entry)) < 0) {
            goto add_done;
        }
    } else {
        int vsbytes;
        /* Insertion of new LS entry */
        ms_read_bits[target_ms_bit / 32] |= 1 << (target_ms_bit % 32);
        soc_EGRESS_IPMC_MSm_field_set(unit, &ms_entry, MS_VECTORf,
                                      ms_read_bits);

        /* We want to use the blank entry at the end of the read list */
        list_index = list_len;
        list_len++; /* We just expanded the list */
        link_list[list_index].read_index = -1; /* For list write (above) */
        ls_entry = &(link_list[list_index].ls_entry);

        vsbytes = soc_mem_entry_words(unit, EGRESS_IPMC_LSm);
        vsbytes = WORDS2BYTES(vsbytes);
        sal_memset(ls_entry, 0, vsbytes);

        soc_EGRESS_IPMC_LSm_field_get(unit, ls_entry, LS_VECTORf, ls_bits);
        ls_bits[ls_bit / 32] |= 1 << (ls_bit % 32);
        soc_EGRESS_IPMC_LSm_field_set(unit, ls_entry, LS_VECTORf, ls_bits);

        if (list_index == 0) {
            /* New lead entry */
            soc_EGRESS_IPMC_LSm_field32_set(unit, ls_entry, NEXT_POINTERf,
                       soc_EGRESS_IPMC_MSm_field32_get(unit, &ms_entry,
                                                       HEAD_POINTERf));
        } else {
            prev_ls_entry = &(link_list[list_index - 1].ls_entry);
            soc_EGRESS_IPMC_LSm_field32_set(unit, ls_entry, NEXT_POINTERf,
                soc_EGRESS_IPMC_LSm_field32_get(unit, prev_ls_entry,
                                                NEXT_POINTERf));
        }

        if ((rv = _bcm_lynx_ipmc_repl_link_list_write(unit, port,
                            list_len, link_list, TRUE, &enable)) < 0) {
            goto add_done;
        }

        if ((list_index == 0) || !enable) {
            /* New head pointer, now that it is known */
            soc_EGRESS_IPMC_MSm_field32_set(unit, &ms_entry, HEAD_POINTERf,
                            link_list[0].write_index);
        }

        write_ms = TRUE;
    }

    /* More than one repl? */
    if (LYNX_IPMC_REPL_VLAN_COUNT(unit, port, group) == 1) {
        soc_EGRESS_IPMC_MSm_field32_set(unit, &ms_entry,
                                        MULTIPLE_ITERf, 1);
        write_ms = TRUE;
        enable = TRUE;
    }

    /* Write MS entry if necessary */
    if (write_ms && ((rv = WRITE_EGRESS_IPMC_MSm(unit, blk,
                                                 group, &ms_entry)) < 0)) {
        goto add_done;
    }

    LYNX_IPMC_REPL_VLAN_COUNT(unit, port, group)++;

    if (enable && !LYNX_IPMC_REPL_ENABLED(unit, port)) {
         if ((rv = _bcm_lynx_ipmc_repl_enable(unit, port, TRUE)) < 0) {	 
             goto add_done;	 
         }	 
     }	 

add_done:
    IPMC_REPL_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *	bcm_lynx_ipmc_repl_delete
 * Purpose:
 *	Remove set of VLANs from selected ports' replication list for chosen
 *	IPMC group.
 * Parameters:
 *	unit     - StrataSwitch PCI device unit number.
 *	group    - The MC index number.
 *	port     - port from which to remove VLAN.
 *	vlan     - VLAN selected to add.
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_lynx_ipmc_repl_delete(int unit, int group, bcm_port_t port,
                          bcm_vlan_t vlan)
{
    int			rv = BCM_E_NONE;
    uint32		ms_read_bits[2];
    uint32		ls_bits[2];
    egress_ipmc_ls_entry_t *ls_entry, *prev_ls_entry;
    egress_ipmc_ms_entry_t ms_entry;
    _lynx_repl_link_t   link_list[LYNX_REPL_MAX_LINKS];
    int			target_ms_bit, ls_bit;
    int			ls_ptr, list_index, list_len;
    int			blk = SOC_PORT_BLOCK(unit, port);
    int                 enable = FALSE, write_ms = FALSE;

    LYNX_IPMC_INIT(unit, port); 
    REPL_PORT_CHECK(unit, port);
    LYNX_IPMC_ID(unit, port, (uint32)group);

    IPMC_REPL_LOCK(unit);

    sal_memset(&ms_entry, 0, sizeof(ms_entry));
    if ((rv = READ_EGRESS_IPMC_MSm(unit, blk, group, &ms_entry)) < 0) {
        goto del_done;
    }
    soc_EGRESS_IPMC_MSm_field_get(unit, &ms_entry, MS_VECTORf, ms_read_bits);

    target_ms_bit = (vlan >> 6) & 0x3f;
    ls_bit = vlan & 0x3f;

    if (!(ms_read_bits[target_ms_bit / 32] & (1 << (target_ms_bit % 32)))) {
        /* Doesn't exist! */
        rv = BCM_E_NOT_FOUND;
        goto del_done;
    }

    /* Fill out link_list structure */
    if ((rv = _bcm_lynx_ipmc_repl_link_list_get(unit, blk, &ms_entry,
                                  target_ms_bit, link_list, &list_len)) < 0) {
        goto del_done;
    }

    assert(list_len > 0);
    list_index = list_len - 1;
    ls_entry = &(link_list[list_index].ls_entry);
    soc_EGRESS_IPMC_LSm_field_get(unit, ls_entry, LS_VECTORf, ls_bits);
    if (!(ls_bits[ls_bit / 32] & (1 << (ls_bit % 32)))) {
        /* Doesn't exist! */
        rv = BCM_E_NOT_FOUND;
        goto del_done;
    }
    ls_bits[ls_bit / 32] &= ~(1 << (ls_bit % 32));
    soc_EGRESS_IPMC_LSm_field_set(unit, ls_entry, LS_VECTORf, ls_bits);

    if ((ls_bits[0] == 0) && (ls_bits[1] == 0)) {
        /* Wiped out the entry */
        ms_read_bits[target_ms_bit / 32] &= ~(1 << (target_ms_bit % 32));
        soc_EGRESS_IPMC_MSm_field_set(unit, &ms_entry, MS_VECTORf,
                                      ms_read_bits);

        if (list_index == 0) { /* Wiped out first link */
            soc_EGRESS_IPMC_MSm_field32_set(unit, &ms_entry, HEAD_POINTERf,
                soc_EGRESS_IPMC_LSm_field32_get(unit, ls_entry,
                                                NEXT_POINTERf));
        } else { /* Wiped out later link */
            prev_ls_entry = &(link_list[list_index - 1].ls_entry);
            soc_EGRESS_IPMC_LSm_field32_set(unit, prev_ls_entry,
                                            NEXT_POINTERf,
                soc_EGRESS_IPMC_LSm_field32_get(unit, ls_entry,
                                                NEXT_POINTERf));
        }

        list_len--; /* One less link in the list */

        write_ms = TRUE;
    }

    /* Dropping to only 1 entry? */
    if (LYNX_IPMC_REPL_VLAN_COUNT(unit, port, group) == 2) {
        soc_EGRESS_IPMC_MSm_field32_set(unit, &ms_entry,
                                        MULTIPLE_ITERf, 0);

        write_ms = TRUE;
    }

    if (write_ms) {
        /* First, LS changes safely */
        if ((rv = _bcm_lynx_ipmc_repl_link_list_write(unit, port,
                            list_len, link_list, FALSE, &enable)) < 0) {
            goto del_done;
        }
        if (list_index == list_len) {
            /* We wiped out an entry, free it */
            LYNX_IPMC_REPL_LS_USED_CLR(unit, port,
                                       link_list[list_index].read_index);
        }
        if (!enable && (list_len != 0)) {
            /* New head pointer, now that it is known */
            soc_EGRESS_IPMC_MSm_field32_set(unit, &ms_entry, HEAD_POINTERf,
                            link_list[0].write_index);
        }

        /* Finally, MS update*/
        if ((rv = WRITE_EGRESS_IPMC_MSm(unit, blk, group, &ms_entry)) < 0) {
            goto del_done;
        }
    } else {
        /* Only LS changed, safe to overwrite */
        ls_ptr = link_list[list_index].read_index;
        if ((rv = WRITE_EGRESS_IPMC_LSm(unit, blk, ls_ptr, ls_entry)) < 0) {
            goto del_done;
        }
    }

    LYNX_IPMC_REPL_VLAN_COUNT(unit, port, group)--;

    if (enable) {
         if ((rv = _bcm_lynx_ipmc_repl_enable(unit, port, TRUE)) < 0) {	 
             goto del_done;	 
         }	 
    }	 

    rv = _bcm_lynx_ipmc_repl_disable_port_check(unit, port);

 del_done:

    IPMC_REPL_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *	bcm_lynx_ipmc_repl_delete_all
 * Purpose:
 *	Remove all VLANs from selected ports' replication list for chosen
 *	IPMC group.
 * Parameters:
 *	unit     - StrataSwitch PCI device unit number.
 *	group    - The MC index number.
 *	port     - port from which to remove VLANs.
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_lynx_ipmc_repl_delete_all(int unit, int group, bcm_port_t port)
{
    int			rv = BCM_E_NONE;
    uint32		ms_read_bits[2];
    uint32		ls_bits[2];
    egress_ipmc_ls_entry_t ls_entry;
    egress_ipmc_ms_entry_t ms_entry;
    int			ls_ptr, ms_bit;
    int			blk = SOC_PORT_BLOCK(unit, port);

    LYNX_IPMC_INIT(unit, port); 
    REPL_PORT_CHECK(unit, port);
    LYNX_IPMC_ID(unit, port, (uint32)group);

    IPMC_REPL_LOCK(unit);

    sal_memset(&ms_entry, 0, sizeof(ms_entry));
    if ((rv = READ_EGRESS_IPMC_MSm(unit, blk, group, &ms_entry)) < 0) {
        goto del_all_done;
    }
    soc_EGRESS_IPMC_MSm_field_get(unit, &ms_entry, MS_VECTORf, ms_read_bits);
    ls_ptr = soc_EGRESS_IPMC_MSm_field32_get(unit, &ms_entry, HEAD_POINTERf);

    /* Clear MS entry first to avoid HW lockup */
    if ((rv = WRITE_EGRESS_IPMC_MSm(unit, blk, group,
                    soc_mem_entry_null(unit, EGRESS_IPMC_MSm))) < 0) {
        goto del_all_done;
    }

    /* Then free the LS entries */
    for (ms_bit = 0; ms_bit < 64; ms_bit++) {
        if (ms_read_bits[ms_bit / 32] & (1 << (ms_bit % 32))) {
            sal_memset(&ls_entry, 0, sizeof(ls_entry));
            if ((rv = READ_EGRESS_IPMC_LSm(unit, blk, ls_ptr,
                                           &ls_entry)) < 0) {
                goto del_all_done;
            }
            soc_EGRESS_IPMC_LSm_field_get(unit, &ls_entry, LS_VECTORf,
                                          ls_bits);
            LYNX_IPMC_REPL_VLAN_COUNT(unit, port, group) -=
                _shr_popcount(ls_bits[0]) + _shr_popcount(ls_bits[1]);
            LYNX_IPMC_REPL_LS_USED_CLR(unit, port, ls_ptr);
            ls_ptr = soc_EGRESS_IPMC_LSm_field32_get(unit, &ls_entry,
                                                     NEXT_POINTERf);
        }
    }

    /* Whether we were out of sync or not, we're now at 0 count */
    LYNX_IPMC_REPL_VLAN_COUNT(unit, port, group) = 0;

    rv = _bcm_lynx_ipmc_repl_disable_port_check(unit, port);

 del_all_done:
    IPMC_REPL_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *	bcm_lynx_ipmc_repl_reload
 * Purpose:
 *	Re-Initialize IPMC replication software to state consistent with
 *      hardware.
 * Parameters:
 *	unit - SOC unit #
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_lynx_ipmc_repl_reload(int unit)
{
    egress_ipmc_ls_entry_t ls_entry;
    egress_ipmc_ms_entry_t ms_entry;
    uint32 ms_read_bits[2];
    uint32 ls_bits[2];
    uint32 cfg2;
    int ls_ptr, ms_bit, num_ipmc_groups;
    int group, alloc_size, blk;
    bcm_port_t port;

    BCM_IF_ERROR_RETURN(bcm_lynx_ipmc_repl_detach(unit));

    alloc_size = SOC_MAX_NUM_PORTS * sizeof(_lynx_repl_port_info_t *);
    _lynx_repl_info[unit] = sal_alloc(alloc_size, "IPMC repl ports");
    if (_lynx_repl_info[unit] == NULL) {
        return BCM_E_MEMORY;
    }
    sal_memset(_lynx_repl_info[unit], 0, alloc_size);

    num_ipmc_groups = soc_mem_index_count(unit, EGRESS_IPMC_MSm);

    PBMP_ITER(PBMP_XE_ALL(unit), port) {
        alloc_size = sizeof(_lynx_repl_port_info_t);
        LYNX_IPMC_REPL_INFO(unit, port) =
            sal_alloc(alloc_size, "IPMC repl entries free");
        if (LYNX_IPMC_REPL_INFO(unit, port) == NULL) {
	    bcm_lynx_ipmc_repl_detach(unit);
            return BCM_E_MEMORY;
        }
        sal_memset(LYNX_IPMC_REPL_INFO(unit, port), 0, alloc_size);

        LYNX_IPMC_REPL_INFO(unit, port)->num_groups = num_ipmc_groups;
        alloc_size = num_ipmc_groups * sizeof(uint32);
        LYNX_IPMC_REPL_INFO(unit, port)->vlan_count =
            sal_alloc(alloc_size, "IPMC repl VLAN counts");
        if (LYNX_IPMC_REPL_INFO(unit, port)->vlan_count == NULL) {
	    bcm_lynx_ipmc_repl_detach(unit);
            return BCM_E_MEMORY;
        }
        sal_memset(LYNX_IPMC_REPL_INFO(unit, port)->vlan_count,
                   0, alloc_size);
    }

    /*
     * Read IPMC_MS, IPMC_LS tables to build up software state
     */
    for (group = 0; group < num_ipmc_groups; group++) {
        PBMP_ITER(PBMP_E_ALL(unit), port) {
            sal_memset(&ms_entry, 0, sizeof(ms_entry));
            blk = SOC_PORT_BLOCK(unit, port);
            SOC_IF_ERROR_RETURN
                (READ_EGRESS_IPMC_MSm(unit, blk, group, &ms_entry));
            ls_ptr = soc_EGRESS_IPMC_MSm_field32_get(unit,
                                        &ms_entry, HEAD_POINTERf);
            soc_EGRESS_IPMC_MSm_field_get(unit, &ms_entry, MS_VECTORf,
                                          ms_read_bits);

            for (ms_bit = 0; ms_bit < 64; ms_bit++) {
                if (ms_read_bits[ms_bit / 32] & (1 << (ms_bit % 32))) {
                    sal_memset(&ls_entry, 0, sizeof(ls_entry));
                    SOC_IF_ERROR_RETURN
                        (READ_EGRESS_IPMC_LSm(unit, blk, ls_ptr, &ls_entry));
                    soc_EGRESS_IPMC_LSm_field_get(unit, &ls_entry,
                                                  LS_VECTORf, ls_bits);
                    LYNX_IPMC_REPL_VLAN_COUNT(unit, port, group) +=
                         _shr_popcount(ls_bits[0]) + _shr_popcount(ls_bits[1]);
                    LYNX_IPMC_REPL_LS_USED_SET(unit, port, ls_ptr);

                    ls_ptr = soc_EGRESS_IPMC_LSm_field32_get(unit,
                                                  &ls_entry, NEXT_POINTERf);
                }
            }
        }
    }

    PBMP_ITER(PBMP_E_ALL(unit), port) {
        SOC_IF_ERROR_RETURN(READ_EGR_IPMC_CFG2r(unit, port, &cfg2));
        LYNX_IPMC_REPL_ENABLED(unit, port) = !soc_reg_field_get(unit,
            EGR_IPMC_CFG2r, cfg2, EGR_DISABLE_IPMC_REPLICATIONf);
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_lynx_ipmc_egress_intf_add
 * Purpose:
 *	Add a VLAN to selected port's replication list for specified
 *	IPMC group.
 * Parameters:
 *	unit     - StrataSwitch PCI device unit number.
 *	group    - The MC index number.
 *	port     - port to which VLAN is added.
 *	l3_intf  - L3 interface containing VLAN selected to add.
 * Returns:
 *	BCM_E_XXX
 */
int
bcm_lynx_ipmc_egress_intf_add(int unit, int group, bcm_port_t port, 
                                bcm_l3_intf_t *l3_intf)
{
    int rv = BCM_E_NONE;

    rv = bcm_lynx_ipmc_repl_add(unit, group, port, l3_intf->l3a_vid);
    return rv;
}

/*
 * Function:
 *	bcm_lynx_ipmc_egress_intf_delete
 * Purpose:
 *	Remove set of VLANs from selected ports' replication list for chosen
 *	IPMC group.
 * Parameters:
 *	unit     - StrataSwitch PCI device unit number.
 *	group    - The MC index number.
 *	port     - port from which to remove VLAN.
 *	l3_intf  - L3 interface containing VLAN selected to delete.
 * Returns:
 *	BCM_E_XXX
 */
int
bcm_lynx_ipmc_egress_intf_delete(int unit, int group, bcm_port_t port, 
                                bcm_l3_intf_t *l3_intf)
{
    int rv = BCM_E_NONE;

    rv = bcm_lynx_ipmc_repl_delete(unit, group, port, l3_intf->l3a_vid);
    return rv;
}

#endif	/* INCLUDE_L3 */

int _bcm_esw_lynx_ipmc_not_empty;
