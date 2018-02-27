/*
 * $Id: ipmc.c 1.30 Broadcom SDK $
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
 * File:	ipmc.c
 * Purpose:	Tracks and manages IPMC tables.
 */

#include <soc/defs.h>
#if defined(BCM_EASYRIDER_SUPPORT) && defined(INCLUDE_L3)
#include <soc/debug.h>
#include <soc/l3x.h>

#include <bcm/error.h>

#include <bcm_int/esw/ipmc.h>
#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/easyrider.h>

#include <bcm_int/esw_dispatch.h>

#define REPL_PORT_CHECK(unit, port) \
    if (!IS_E_PORT(unit, port)) { return BCM_E_PARAM; }

#define EASYRIDER_IPMC_GROUPS        4096 
#define EASYRIDER_INTF_NUM           4096 

typedef struct _er_repl_port_info_s {
    uint32 vlan_count[EASYRIDER_IPMC_GROUPS];  /* # VLANs the port repl to */
} _er_repl_port_info_t;

typedef struct _er_repl_info_s {
    uint16 ipmc_vlan_total;       /* Keep track of total and */
    uint32 *bitmap_entries_used;  /* free entries of IPMC_INDEX table */
    _bcm_repl_list_info_t *repl_list_info;
    _er_repl_port_info_t *port_info[SOC_MAX_NUM_PORTS];
} _er_repl_info_t;

static _er_repl_info_t *_er_repl_info[BCM_MAX_NUM_UNITS];

#define IPMC_REPL_LOCK(_u_)                    \
    {                                          \
        soc_mem_lock(_u_, MMU_IPMC_INDEXm);    \
        soc_mem_lock(_u_, MMU_IPMC_PTRm);      \
    }
#define IPMC_REPL_UNLOCK(_u_)                  \
    {                                          \
        soc_mem_unlock(_u_, MMU_IPMC_INDEXm);  \
        soc_mem_unlock(_u_, MMU_IPMC_PTRm);    \
    }
#define IPMC_REPL_TOTAL(_u_) \
        _er_repl_info[_u_]->ipmc_vlan_total
#define IPMC_REPL_INTF_TOTAL(_u_) EASYRIDER_INTF_NUM
#define IPMC_REPL_VE_USED_GET(_u_, _i_) \
        SHR_BITGET(_er_repl_info[_u_]->bitmap_entries_used, _i_)
#define IPMC_REPL_VE_USED_SET(_u_, _i_) \
        SHR_BITSET(_er_repl_info[_u_]->bitmap_entries_used, _i_)
#define IPMC_REPL_VE_USED_CLR(_u_, _i_) \
        SHR_BITCLR(_er_repl_info[_u_]->bitmap_entries_used, _i_)

#define IPMC_REPL_LIST_INFO(_u_) \
	_er_repl_info[_u_]->repl_list_info
#define IPMC_REPL_PORT_INFO(_u_, _p_) \
        _er_repl_info[_u_]->port_info[_p_]
#define IPMC_REPL_PORT_VLAN_COUNT(_u_, _p_, _ipmc_id_) \
        _er_repl_info[_u_]->port_info[_p_]->vlan_count[_ipmc_id_]

#define ER_IPMC_ID(unit, id) \
        if ((id < 0) || (id >= EASYRIDER_IPMC_GROUPS)) \
            { return BCM_E_PARAM; }

STATIC soc_field_t
_ipmc_ptr_get(int unit, bcm_port_t port)
{
    switch (port) {
    case 0:
        return IPMC_PTR_P0f;
    case 1:
        return IPMC_PTR_P1f;
    case 2:
        return IPMC_PTR_P2f;
    case 3:
        return IPMC_PTR_P3f;
    case 4:
        return IPMC_PTR_P4f;
    case 5:
        return IPMC_PTR_P5f;
    case 6:
        return IPMC_PTR_P6f;
    case 7:
        return IPMC_PTR_P7f;
    case 8:
        return IPMC_PTR_P8f;
    case 9:
        return IPMC_PTR_P9f;
    case 10:
        return IPMC_PTR_P10f;
    case 11:
        return IPMC_PTR_P11f;
    default:
        return INVALIDf;
    }
}

/*
 * Function:
 *	bcm_er_ipmc_repl_init
 * Purpose:
 *	Initialize IPMC replication.
 * Parameters:
 *	unit     - SOC unit #
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_er_ipmc_repl_init(int unit)
{
    bcm_port_t          port;
    int                 alloc_size;

    bcm_er_ipmc_repl_detach(unit);

    /* Allocate struct for IPMC replication bookkeeping */
    alloc_size = sizeof(_er_repl_info_t);
    _er_repl_info[unit] = sal_alloc(alloc_size, "IPMC repl info");
    if (_er_repl_info[unit] == NULL) {
        return BCM_E_MEMORY;
    }
    sal_memset(_er_repl_info[unit], 0, alloc_size);

    IPMC_REPL_TOTAL(unit) = soc_mem_index_count(unit, MMU_IPMC_INDEXm); 
    alloc_size = SHR_BITALLOCSIZE(IPMC_REPL_TOTAL(unit));
    _er_repl_info[unit]->bitmap_entries_used =
        sal_alloc(alloc_size, "IPMC repl entries used");
    if (_er_repl_info[unit]->bitmap_entries_used == NULL) {
        bcm_er_ipmc_repl_detach(unit);
        return BCM_E_MEMORY;
    }
    sal_memset(_er_repl_info[unit]->bitmap_entries_used, 0, alloc_size);

    /* Always reserve slot 0 */
    IPMC_REPL_VE_USED_SET(unit, 0);

    PBMP_ITER(PBMP_PORT_ALL(unit), port) {
        alloc_size = sizeof(_er_repl_port_info_t);
        IPMC_REPL_PORT_INFO(unit, port) =
            sal_alloc(alloc_size, "IPMC repl port info");
        if (IPMC_REPL_PORT_INFO(unit, port) == NULL) {
            bcm_er_ipmc_repl_detach(unit);
            return BCM_E_MEMORY;
        }
        sal_memset(IPMC_REPL_PORT_INFO(unit, port), 0, alloc_size);
    }

    SOC_IF_ERROR_RETURN
        (soc_mem_clear(unit, MMU_IPMC_PTRm, COPYNO_ALL, FALSE));

    SOC_IF_ERROR_RETURN
        (soc_mem_clear(unit, MMU_IPMC_INDEXm, COPYNO_ALL, FALSE));

    return BCM_E_NONE;

}

/*
 * Function:
 *	bcm_er_ipmc_repl_detach
 * Purpose:
 *	Initialize IPMC replication.
 * Parameters:
 *	unit     - SOC unit #
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_er_ipmc_repl_detach(int unit)
{
    bcm_port_t          port;
    _bcm_repl_list_info_t *rli_current, *rli_free;

    if (_er_repl_info[unit] != NULL) {
        PBMP_ITER(PBMP_PORT_ALL(unit), port) {
            if (_er_repl_info[unit]->port_info[port] != NULL) {
                sal_free(_er_repl_info[unit]->port_info[port]);
            }
        }

        if (_er_repl_info[unit]->bitmap_entries_used != NULL) {
            sal_free(_er_repl_info[unit]->bitmap_entries_used);
        }

        if (_er_repl_info[unit]->repl_list_info != NULL) {
            rli_current = IPMC_REPL_LIST_INFO(unit);
            while (rli_current != NULL) {
                rli_free = rli_current;
                rli_current = rli_current->next;
                sal_free(rli_free);
            }
        }

        sal_free(_er_repl_info[unit]);
        _er_repl_info[unit] = NULL;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_er_ipmc_repl_get
 * Purpose:
 *	Return set of VLANs selected for port's replication list for chosen
 *	IPMC group.
 * Parameters:
 *	unit     - StrataSwitch PCI device unit number.
 *	ipmc_id  - The index number.
 *	port     - port to list.
 *	vlan_vec - (OUT) vector of replicated VLANs common to selected ports.
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_er_ipmc_repl_get(int unit, int ipmc_id, bcm_port_t port, 
                     bcm_vlan_vector_t vlan_vec)
{
    int                 rv = BCM_E_NONE;
    uint32              ms_bit;
    uint32              ls_bits[2];
    mmu_ipmc_ptr_entry_t group_entry;
    mmu_ipmc_index_entry_t vlan_entry;
    int                 vlan_ptr, last_vlan_ptr;
    uint32              vlan_count;

    REPL_PORT_CHECK(unit, port);

    BCM_VLAN_VEC_ZERO(vlan_vec);

    IPMC_REPL_LOCK(unit);

    if (IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id)) {
        if ((rv = READ_MMU_IPMC_PTRm(unit, MEM_BLOCK_ALL,
                                     ipmc_id, &group_entry)) < 0) {
            goto get_done;
        }
        vlan_ptr = soc_MMU_IPMC_PTRm_field32_get(unit,
                           &group_entry, _ipmc_ptr_get(unit, port));

        last_vlan_ptr = -1;
        vlan_count = 0;
        while (vlan_ptr != last_vlan_ptr) {
            if ((rv = READ_MMU_IPMC_INDEXm(unit, MEM_BLOCK_ALL,
                                              vlan_ptr, &vlan_entry)) < 0) {
                goto get_done;
            }
            ms_bit = soc_MMU_IPMC_INDEXm_field32_get(unit,
                                                     &vlan_entry, MSBf);
            soc_MMU_IPMC_INDEXm_field_get(unit, &vlan_entry, LSBf, ls_bits);
            vlan_vec[2 * ms_bit + 0] = ls_bits[0];
            vlan_vec[2 * ms_bit + 1] = ls_bits[1];
            last_vlan_ptr = vlan_ptr;
            vlan_ptr = soc_MMU_IPMC_INDEXm_field32_get(unit,
                                                      &vlan_entry, NXT_PTRf);

            vlan_count +=
                _shr_popcount(ls_bits[0]) + _shr_popcount(ls_bits[1]);
            if (vlan_count >= IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id)) {
                break;
            }
        }
    }

 get_done:
    IPMC_REPL_UNLOCK(unit);
    return rv;
}

STATIC int
_er_ipmc_repl_next_free_ptr(int unit)
{
    int                 ix, bit;
    SHR_BITDCL          not_ptrs;

    for (ix = 0; ix < _SHR_BITDCLSIZE(IPMC_REPL_TOTAL(unit)); ix++) {
        not_ptrs = ~_er_repl_info[unit]->bitmap_entries_used[ix];
        if (not_ptrs) {
            for (bit = 0; bit < SHR_BITWID; bit++) {
                if (not_ptrs & (1 << bit)) {
                    return (ix * SHR_BITWID) + bit;
                }
            }
        }
    }

    return -1;
}

/*
 * Function:
 *	bcm_er_ipmc_repl_add
 * Purpose:
 *	Add VLAN to selected ports' replication list for chosen
 *	IPMC group.
 *Parameters:
 *	unit     - StrataSwitch PCI device unit number.
 *	ipmc_id  - The index number.
 *	port     - port to add.
 *	vlan     - VLAN to replicate.
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_er_ipmc_repl_add(int unit, int ipmc_id, bcm_port_t port, 
                     bcm_vlan_t vlan)
{
    int                 rv = BCM_E_NONE;
    uint32              ls_bits[2];
    mmu_ipmc_ptr_entry_t   group_entry;
    mmu_ipmc_index_entry_t vlan_entry;
    int                 ms_bit, target_ms_bit, ls_bit;
    int                 vlan_ptr, last_vlan_ptr;
    pbmp_t pbmp, ubmp;
    bcm_l3_intf_t l3_intf;
    uint32 vlan_count;

    REPL_PORT_CHECK(unit, port);

    /* Check if port belongs to this VLAN */
    BCM_IF_ERROR_RETURN
        (bcm_esw_vlan_port_get(unit, vlan, &pbmp, &ubmp));
    if (!SOC_PBMP_MEMBER(pbmp, port)) {
        return BCM_E_PARAM;
    }

    sal_memset(&l3_intf, 0, sizeof(bcm_l3_intf_t));
    l3_intf.l3a_vid = vlan;
    if (bcm_esw_l3_intf_find_vlan(unit, &l3_intf) < 0) {
        return BCM_E_PARAM;
    }

    IPMC_REPL_LOCK(unit);
    if ((rv = READ_MMU_IPMC_PTRm(unit, MEM_BLOCK_ALL, 
                                 ipmc_id, &group_entry)) < 0) {
        goto add_done;
    }

    vlan_ptr = soc_MMU_IPMC_PTRm_field32_get(unit,
                       &group_entry, _ipmc_ptr_get(unit, port));
    target_ms_bit = (l3_intf.l3a_intf_id >> 6) & 0x3f;
    ls_bit = l3_intf.l3a_intf_id & 0x3f;

    last_vlan_ptr = -1;
    vlan_count = 0;
    ms_bit = -1;

    if (IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id)) {
        /*
         * Add to existing chain
         */
        while (vlan_ptr != last_vlan_ptr) {
            if ((rv = READ_MMU_IPMC_INDEXm(unit, MEM_BLOCK_ALL,
                                           vlan_ptr, &vlan_entry)) < 0) {
                goto add_done;
            }

            ms_bit = soc_MMU_IPMC_INDEXm_field32_get(unit,
                                         &vlan_entry, MSBf);

            if (ms_bit == target_ms_bit) {
                /* Add to existing entry */
                break;
            }

            last_vlan_ptr = vlan_ptr;
            vlan_ptr = soc_MMU_IPMC_INDEXm_field32_get(unit,
                                              &vlan_entry, NXT_PTRf);

            soc_MMU_IPMC_INDEXm_field_get(unit, &vlan_entry,
                                             LSBf, ls_bits);
            vlan_count +=
                _shr_popcount(ls_bits[0]) + _shr_popcount(ls_bits[1]);
            if (vlan_count >= IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id)) {
                break;
            }
        }
    }

    if (ms_bit != target_ms_bit) {
        /* New entry, find slot */
        vlan_ptr = _er_ipmc_repl_next_free_ptr(unit);
        if (vlan_ptr < 0) {
            /* Out of LS entries */
            rv = BCM_E_RESOURCE;
            goto add_done;
        }

        if (last_vlan_ptr < 0) {
            /* Update group table */
            soc_MMU_IPMC_PTRm_field32_set(unit, &group_entry,
                              _ipmc_ptr_get(unit, port), vlan_ptr);
            if ((rv = WRITE_MMU_IPMC_PTRm(unit, MEM_BLOCK_ALL,
                                          ipmc_id, &group_entry)) < 0) {
                goto add_done;
            }
        } else {
            /* Update last entry */
            soc_MMU_IPMC_INDEXm_field32_set(unit, &vlan_entry,
                                            NXT_PTRf, vlan_ptr);
            if ((rv = WRITE_MMU_IPMC_INDEXm(unit, MEM_BLOCK_ALL,
                                            last_vlan_ptr, &vlan_entry)) < 0) {
                goto add_done;
            }
        }

        sal_memset(&vlan_entry, 0, sizeof(vlan_entry));
        soc_MMU_IPMC_INDEXm_field32_set(unit, &vlan_entry,
                                        MSBf, target_ms_bit);
        /* Point to self */
        soc_MMU_IPMC_INDEXm_field32_set(unit, &vlan_entry,
                                        NXT_PTRf, vlan_ptr);

        IPMC_REPL_VE_USED_SET(unit, vlan_ptr);
    }

    /* Insert replacement */
    soc_MMU_IPMC_INDEXm_field_get(unit, &vlan_entry,
                                     LSBf, ls_bits);
    if (ls_bits[ls_bit / 32] & (1 << (ls_bit % 32))) {
        /* Already exists! */
        rv = BCM_E_EXISTS;
        goto add_done;
    }
    ls_bits[ls_bit / 32] |= 1 << (ls_bit % 32);
    soc_MMU_IPMC_INDEXm_field_set(unit, &vlan_entry,
                                  LSBf, ls_bits);


    if ((rv = WRITE_MMU_IPMC_INDEXm(unit, MEM_BLOCK_ALL,
                                    vlan_ptr, &vlan_entry)) < 0) {
        goto add_done;
    }

    if (IS_XE_PORT(unit, port)) {
        mmu_ipmc_rep_10g_memory_entry_t   rep10g_entry;

        sal_memset(&rep10g_entry, 0, sizeof(rep10g_entry));
        soc_MMU_IPMC_REP_10G_MEMORYm_field32_set(unit, &rep10g_entry, 
                REPLICATIONSf, IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id));
        rv = WRITE_MMU_IPMC_REP_10G_MEMORYm(unit, MEM_BLOCK_ALL, 
                                            ipmc_id, &rep10g_entry);
    }

    IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id)++;

 add_done:
    IPMC_REPL_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *	bcm_er_ipmc_repl_delete
 * Purpose:
 *	Remove VLAN from selected ports' replication list for chosen
 *	IPMC group.
 * Parameters:
 *	unit     - StrataSwitch PCI device unit number.
 *	ipmc_id  - The index number.
 *	port     - port to remove.
 *	vlan     - VLAN to delete from replication.
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_er_ipmc_repl_delete(int unit, int ipmc_id, bcm_port_t port, 
                        bcm_vlan_t vlan)
{
    int                 rv = BCM_E_NONE;
    uint32              ls_bits[2];
    mmu_ipmc_ptr_entry_t group_entry;
    mmu_ipmc_index_entry_t vlan_entry, prev_vlan_entry;
    int                 ms_bit, target_ms_bit, ls_bit;
    int                 vlan_ptr, last_vlan_ptr, next_vlan_ptr;
    bcm_l3_intf_t       l3_intf;
    uint32              vlan_count;

    REPL_PORT_CHECK(unit, port);

    if (!IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id)) {
        return BCM_E_NOT_FOUND;
    }

    sal_memset(&l3_intf, 0, sizeof(bcm_l3_intf_t));
    l3_intf.l3a_vid = vlan;
    if (bcm_esw_l3_intf_find_vlan(unit, &l3_intf) < 0) {
        return BCM_E_PARAM;
    }

    IPMC_REPL_LOCK(unit);
    if ((rv = READ_MMU_IPMC_PTRm(unit, MEM_BLOCK_ALL, ipmc_id,
                                                &group_entry)) < 0) {
        goto del_done;
    }
    vlan_ptr = soc_MMU_IPMC_PTRm_field32_get(unit,
                       &group_entry, _ipmc_ptr_get(unit, port));

    target_ms_bit = (l3_intf.l3a_intf_id >> 6) & 0x3f;
    ls_bit = l3_intf.l3a_intf_id & 0x3f;

    last_vlan_ptr = 0; /* Since ptr 0 should never be used! */
    vlan_count = 0;
    ms_bit = -1;

    while (vlan_ptr != last_vlan_ptr) {
        if ((rv = READ_MMU_IPMC_INDEXm(unit, MEM_BLOCK_ALL,
                                       vlan_ptr, &vlan_entry)) < 0) {
            goto del_done;
        }

        ms_bit = soc_MMU_IPMC_INDEXm_field32_get(unit,
                                        &vlan_entry, MSBf);
        if (ms_bit == target_ms_bit) {
            /* Delete existing entry */
            soc_MMU_IPMC_INDEXm_field_get(unit, &vlan_entry,
                                          LSBf, ls_bits);
            if (!(ls_bits[ls_bit / 32] & (1 << (ls_bit % 32)))) {
                /* Doesn't exist! */
                rv = BCM_E_NOT_FOUND;
                goto del_done;
            }
            ls_bits[ls_bit / 32] &= ~(1 << (ls_bit % 32));
            soc_MMU_IPMC_INDEXm_field_set(unit, &vlan_entry,
                                             LSBf, ls_bits);

            if ((ls_bits[0] == 0) && (ls_bits[1] == 0)) {
                /* Wiped out the entry */
                /* Record before overwrite */
                next_vlan_ptr =
                    soc_MMU_IPMC_INDEXm_field32_get(unit,
                                            &vlan_entry, NXT_PTRf);
                soc_MMU_IPMC_INDEXm_field32_set(unit,
                                            &vlan_entry, MSBf, 0);
                soc_MMU_IPMC_INDEXm_field32_set(unit,
                                            &vlan_entry, NXT_PTRf, 0);
                if (last_vlan_ptr == 0) {
                    /* First entry for this (group, port) */
                    if (vlan_ptr == next_vlan_ptr) {
                        /* Only link in chain */
                        next_vlan_ptr = 0;
                    }
                    soc_MMU_IPMC_PTRm_field32_set(unit, &group_entry, 
                        _ipmc_ptr_get(unit, port), next_vlan_ptr);
                    if ((rv = WRITE_MMU_IPMC_PTRm(unit,
                                    MEM_BLOCK_ALL, ipmc_id, &group_entry)) < 0) {
                        goto del_done;
                    }
                } else {
                    /* Link over with last entry */
                    if (vlan_ptr == next_vlan_ptr) {
                        /* If this is end of chain, end on previous link */
                        next_vlan_ptr = last_vlan_ptr;
                    }
                    soc_MMU_IPMC_INDEXm_field32_set(unit,
                            &prev_vlan_entry, NXT_PTRf, next_vlan_ptr);
                    if ((rv = WRITE_MMU_IPMC_INDEXm(unit,
                                    MEM_BLOCK_ALL, last_vlan_ptr,
                                    &prev_vlan_entry)) < 0) {
                        goto del_done;
                    }
                }

                IPMC_REPL_VE_USED_CLR(unit, vlan_ptr);
            }

            if ((rv = WRITE_MMU_IPMC_INDEXm(unit,
                                MEM_BLOCK_ALL, vlan_ptr, &vlan_entry)) < 0) {
                goto del_done;
            }

            IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id)--;

            if (IS_XE_PORT(unit, port)) {
                mmu_ipmc_rep_10g_memory_entry_t   rep10g_entry;
                int cnt;

                sal_memset(&rep10g_entry, 0, sizeof(rep10g_entry));
                cnt = IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id) - 1;
                soc_MMU_IPMC_REP_10G_MEMORYm_field32_set(unit, &rep10g_entry,
                                             REPLICATIONSf, (cnt > 0) ? cnt : 0);
                rv = WRITE_MMU_IPMC_REP_10G_MEMORYm(unit, MEM_BLOCK_ALL,
                                                    ipmc_id, &rep10g_entry);
            }

            goto del_done;
        }

        last_vlan_ptr = vlan_ptr;
        vlan_ptr = soc_MMU_IPMC_INDEXm_field32_get(unit,
                                          &vlan_entry, NXT_PTRf);
        sal_memcpy(&prev_vlan_entry, &vlan_entry,
                   sizeof(mmu_ipmc_vlan_tbl_entry_t));

        soc_MMU_IPMC_INDEXm_field_get(unit, &vlan_entry,
                                               LSBf, ls_bits);
        vlan_count +=
            _shr_popcount(ls_bits[0]) + _shr_popcount(ls_bits[1]);
        if (vlan_count >= IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id)) {
            rv = BCM_E_NOT_FOUND;
            goto del_done;
        }
    }

    rv = BCM_E_NOT_FOUND;

 del_done:
    IPMC_REPL_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *	bcm_er_ipmc_repl_delete_all
 * Purpose:
 *	Remove all VLANs from selected ports' replication list for chosen
 *	IPMC group.
 * Parameters:
 *	unit     - StrataSwitch PCI device unit number.
 *	ipmc_id  - The MC index number.
 *	port     - port from which to remove VLANs.
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_er_ipmc_repl_delete_all(int unit, int ipmc_id, bcm_port_t port)
{
    int                 rv = BCM_E_NONE;
    uint32              ls_bits[2];
    mmu_ipmc_ptr_entry_t group_entry;
    mmu_ipmc_index_entry_t vlan_entry;
    int                 vlan_ptr, last_vlan_ptr;
    int                 num_vlans;

    REPL_PORT_CHECK(unit, port);

    if (!IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id)) {
        /* Nothing to do */
        return BCM_E_NONE;
    }

    IPMC_REPL_LOCK(unit);

    if ((rv = READ_MMU_IPMC_PTRm(unit, MEM_BLOCK_ALL, ipmc_id,
                                                 &group_entry)) < 0) {
        goto del_all_done;
    }
    vlan_ptr = soc_MMU_IPMC_PTRm_field32_get(unit,
                       &group_entry, _ipmc_ptr_get(unit, port));

    soc_MMU_IPMC_PTRm_field32_set(unit,
                      &group_entry, _ipmc_ptr_get(unit, port), 0);
    if ((rv = WRITE_MMU_IPMC_PTRm(unit,
                        MEM_BLOCK_ALL, ipmc_id, &group_entry)) < 0) {
        goto del_all_done;
    }

    last_vlan_ptr = 0; /* Since ptr 0 should never be used! */

    while (vlan_ptr != last_vlan_ptr) {
        if ((rv = READ_MMU_IPMC_INDEXm(unit, MEM_BLOCK_ALL,
                                          vlan_ptr, &vlan_entry)) < 0) {
            goto del_all_done;
        }

        if ((rv = WRITE_MMU_IPMC_INDEXm(unit,
                        MEM_BLOCK_ALL, vlan_ptr, soc_mem_entry_null(unit,
                            MMU_IPMC_INDEXm))) < 0) {
            goto del_all_done;
        }

        IPMC_REPL_VE_USED_CLR(unit, vlan_ptr);

        last_vlan_ptr = vlan_ptr;
        vlan_ptr = soc_MMU_IPMC_INDEXm_field32_get(unit,
                                               &vlan_entry, NXT_PTRf);

        soc_MMU_IPMC_INDEXm_field_get(unit, &vlan_entry,
                                               LSBf, ls_bits);
        num_vlans = _shr_popcount(ls_bits[0]) + _shr_popcount(ls_bits[1]);
        if (IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id) < num_vlans) {
            IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id) = 0;
            goto del_all_done;
        }
        IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id) -= num_vlans;
    }
    
    if (IS_XE_PORT(unit, port)) {
        rv = WRITE_MMU_IPMC_REP_10G_MEMORYm(unit, MEM_BLOCK_ALL, ipmc_id, 
                       soc_mem_entry_null(unit, MMU_IPMC_REP_10G_MEMORYm));
    }

 del_all_done:
    IPMC_REPL_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *	bcm_er_ipmc_egress_intf_add
 * Purpose:
 *	Add L3 interface to selected ports' replication list for chosen
 *	IPMC group.
 * Parameters:
 *	unit     - StrataSwitch PCI device unit number.
 *	ipmc_id  - The index number.
 *	port     - port to add.
 *	l3_intf  - L3 interface to replicate.
 * Returns:
 *	BCM_E_XXX
 */
int
bcm_er_ipmc_egress_intf_add(int unit, int ipmc_id, bcm_port_t port, 
                            bcm_l3_intf_t *l3_intf)
{
    int                 rv = BCM_E_NONE;
    uint32              ls_bits[2];
    mmu_ipmc_ptr_entry_t   group_entry;
    mmu_ipmc_index_entry_t vlan_entry;
    int                 ms_bit, target_ms_bit, ls_bit;
    int                 vlan_ptr, last_vlan_ptr;
    pbmp_t pbmp, ubmp;
    uint32              vlan_count;

    REPL_PORT_CHECK(unit, port);

    /* Check if port belongs to this VLAN */
    BCM_IF_ERROR_RETURN
        (bcm_esw_vlan_port_get(unit, l3_intf->l3a_vid, &pbmp, &ubmp));
    if (!SOC_PBMP_MEMBER(pbmp, port)) {
        return BCM_E_PARAM;
    }

    IPMC_REPL_LOCK(unit);
    if ((rv = READ_MMU_IPMC_PTRm(unit, MEM_BLOCK_ALL, 
                                 ipmc_id, &group_entry)) < 0) {
        goto add_done;
    }

    vlan_ptr = soc_MMU_IPMC_PTRm_field32_get(unit,
                       &group_entry, _ipmc_ptr_get(unit, port));
    target_ms_bit = (l3_intf->l3a_intf_id >> 6) & 0x3f;
    ls_bit = l3_intf->l3a_intf_id & 0x3f;

    last_vlan_ptr = -1;
    vlan_count = 0;
    ms_bit = -1;

    if (IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id)) {
        /*
         * Add to existing chain
         */
        while (vlan_ptr != last_vlan_ptr) {
            if ((rv = READ_MMU_IPMC_INDEXm(unit, MEM_BLOCK_ALL,
                                           vlan_ptr, &vlan_entry)) < 0) {
                goto add_done;
            }

            ms_bit = soc_MMU_IPMC_INDEXm_field32_get(unit,
                                         &vlan_entry, MSBf);

            if (ms_bit == target_ms_bit) {
                /* Add to existing entry */
                break;
            }

            last_vlan_ptr = vlan_ptr;
            vlan_ptr = soc_MMU_IPMC_INDEXm_field32_get(unit,
                                              &vlan_entry, NXT_PTRf);

            soc_MMU_IPMC_INDEXm_field_get(unit, &vlan_entry,
                                             LSBf, ls_bits);
            vlan_count +=
                _shr_popcount(ls_bits[0]) + _shr_popcount(ls_bits[1]);
            if (vlan_count >= IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id)) {
                break;
            }
        }
    }

    if (ms_bit != target_ms_bit) {
        /* New entry, find slot */
        vlan_ptr = _er_ipmc_repl_next_free_ptr(unit);
        if (vlan_ptr < 0) {
            /* Out of LS entries */
            rv = BCM_E_RESOURCE;
            goto add_done;
        }

        if (last_vlan_ptr < 0) {
            /* Update group table */
            soc_MMU_IPMC_PTRm_field32_set(unit, &group_entry,
                              _ipmc_ptr_get(unit, port), vlan_ptr);
            if ((rv = WRITE_MMU_IPMC_PTRm(unit, MEM_BLOCK_ALL,
                                          ipmc_id, &group_entry)) < 0) {
                goto add_done;
            }
        } else {
            /* Update last entry */
            soc_MMU_IPMC_INDEXm_field32_set(unit, &vlan_entry,
                                            NXT_PTRf, vlan_ptr);
            if ((rv = WRITE_MMU_IPMC_INDEXm(unit, MEM_BLOCK_ALL,
                                            last_vlan_ptr, &vlan_entry)) < 0) {
                goto add_done;
            }
        }

        sal_memset(&vlan_entry, 0, sizeof(vlan_entry));
        soc_MMU_IPMC_INDEXm_field32_set(unit, &vlan_entry,
                                        MSBf, target_ms_bit);
        /* Point to self */
        soc_MMU_IPMC_INDEXm_field32_set(unit, &vlan_entry,
                                        NXT_PTRf, vlan_ptr);

        IPMC_REPL_VE_USED_SET(unit, vlan_ptr);
    }

    /* Insert replacement */
    soc_MMU_IPMC_INDEXm_field_get(unit, &vlan_entry,
                                     LSBf, ls_bits);
    if (ls_bits[ls_bit / 32] & (1 << (ls_bit % 32))) {
        /* Already exists! */
        rv = BCM_E_EXISTS;
        goto add_done;
    }
    ls_bits[ls_bit / 32] |= 1 << (ls_bit % 32);
    soc_MMU_IPMC_INDEXm_field_set(unit, &vlan_entry,
                                  LSBf, ls_bits);


    if ((rv = WRITE_MMU_IPMC_INDEXm(unit, MEM_BLOCK_ALL,
                                    vlan_ptr, &vlan_entry)) < 0) {
        goto add_done;
    }

    if (IS_XE_PORT(unit, port)) {
        mmu_ipmc_rep_10g_memory_entry_t   rep10g_entry;

        sal_memset(&rep10g_entry, 0, sizeof(rep10g_entry));
        soc_MMU_IPMC_REP_10G_MEMORYm_field32_set(unit, &rep10g_entry, 
                REPLICATIONSf, IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id));
        rv = WRITE_MMU_IPMC_REP_10G_MEMORYm(unit, MEM_BLOCK_ALL, 
                                            ipmc_id, &rep10g_entry);
    }

    IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id)++;

add_done:
    IPMC_REPL_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *	bcm_er_ipmc_egress_intf_delete
 * Purpose:
 *	Remove L3 interface from selected ports' replication list for chosen
 *	IPMC group.
 * Parameters:
 *	unit     - StrataSwitch PCI device unit number.
 *	ipmc_id  - The index number.
 *	port     - port to remove.
 *	l3_intf  - L3 interface to delete from replication.
 * Returns:
 *	BCM_E_XXX
 */
int
bcm_er_ipmc_egress_intf_delete(int unit, int ipmc_id, bcm_port_t port,
			       bcm_l3_intf_t *l3_intf)
{
    int                 rv = BCM_E_NONE;
    uint32              ls_bits[2];
    mmu_ipmc_ptr_entry_t group_entry;
    mmu_ipmc_index_entry_t vlan_entry, prev_vlan_entry;
    int                 ms_bit, target_ms_bit, ls_bit;
    int                 vlan_ptr, last_vlan_ptr, next_vlan_ptr;
    uint32              vlan_count;

    REPL_PORT_CHECK(unit, port);

    if (!IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id)) {
        return BCM_E_NOT_FOUND;
    }

    IPMC_REPL_LOCK(unit);
    if ((rv = READ_MMU_IPMC_PTRm(unit, MEM_BLOCK_ALL, ipmc_id,
                                                &group_entry)) < 0) {
        goto del_done;
    }
    vlan_ptr = soc_MMU_IPMC_PTRm_field32_get(unit,
                       &group_entry, _ipmc_ptr_get(unit, port));

    target_ms_bit = (l3_intf->l3a_intf_id >> 6) & 0x3f;
    ls_bit = l3_intf->l3a_intf_id & 0x3f;

    last_vlan_ptr = 0; /* Since ptr 0 should never be used! */
    vlan_count = 0;
    ms_bit = -1;

    while (vlan_ptr != last_vlan_ptr) {
        if ((rv = READ_MMU_IPMC_INDEXm(unit, MEM_BLOCK_ALL,
                                       vlan_ptr, &vlan_entry)) < 0) {
            goto del_done;
        }

        ms_bit = soc_MMU_IPMC_INDEXm_field32_get(unit,
                                        &vlan_entry, MSBf);
        if (ms_bit == target_ms_bit) {
            /* Delete existing entry */
            soc_MMU_IPMC_INDEXm_field_get(unit, &vlan_entry,
                                          LSBf, ls_bits);
            if (!(ls_bits[ls_bit / 32] & (1 << (ls_bit % 32)))) {
                /* Doesn't exist! */
                rv = BCM_E_NOT_FOUND;
                goto del_done;
            }
            ls_bits[ls_bit / 32] &= ~(1 << (ls_bit % 32));
            soc_MMU_IPMC_INDEXm_field_set(unit, &vlan_entry,
                                             LSBf, ls_bits);

            if ((ls_bits[0] == 0) && (ls_bits[1] == 0)) {
                /* Wiped out the entry */
                /* Record before overwrite */
                next_vlan_ptr =
                    soc_MMU_IPMC_INDEXm_field32_get(unit,
                                            &vlan_entry, NXT_PTRf);
                soc_MMU_IPMC_INDEXm_field32_set(unit,
                                            &vlan_entry, MSBf, 0);
                soc_MMU_IPMC_INDEXm_field32_set(unit,
                                            &vlan_entry, NXT_PTRf, 0);
                if (last_vlan_ptr == 0) {
                    /* First entry for this (group, port) */
                    if (vlan_ptr == next_vlan_ptr) {
                        /* Only link in chain */
                        next_vlan_ptr = 0;
                    }
                    soc_MMU_IPMC_PTRm_field32_set(unit, &group_entry, 
                        _ipmc_ptr_get(unit, port), next_vlan_ptr);
                    if ((rv = WRITE_MMU_IPMC_PTRm(unit,
                                    MEM_BLOCK_ALL, ipmc_id, &group_entry)) < 0) {
                        goto del_done;
                    }
                } else {
                    /* Link over with last entry */
                    if (vlan_ptr == next_vlan_ptr) {
                        /* If this is end of chain, end on previous link */
                        next_vlan_ptr = last_vlan_ptr;
                    }
                    soc_MMU_IPMC_INDEXm_field32_set(unit,
                            &prev_vlan_entry, NXT_PTRf, next_vlan_ptr);
                    if ((rv = WRITE_MMU_IPMC_INDEXm(unit,
                                    MEM_BLOCK_ALL, last_vlan_ptr,
                                    &prev_vlan_entry)) < 0) {
                        goto del_done;
                    }
                }

                IPMC_REPL_VE_USED_CLR(unit, vlan_ptr);
            }

            if ((rv = WRITE_MMU_IPMC_INDEXm(unit,
                                MEM_BLOCK_ALL, vlan_ptr, &vlan_entry)) < 0) {
                goto del_done;
            }

            IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id)--;

            if (IS_XE_PORT(unit, port)) {
                mmu_ipmc_rep_10g_memory_entry_t   rep10g_entry;
                int cnt;

                sal_memset(&rep10g_entry, 0, sizeof(rep10g_entry));
                cnt = IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id) - 1;
                soc_MMU_IPMC_REP_10G_MEMORYm_field32_set(unit, &rep10g_entry,
                                             REPLICATIONSf, (cnt > 0) ? cnt : 0);
                rv = WRITE_MMU_IPMC_REP_10G_MEMORYm(unit, MEM_BLOCK_ALL,
                                                    ipmc_id, &rep10g_entry);
            }

            goto del_done;
        }

        last_vlan_ptr = vlan_ptr;
        vlan_ptr = soc_MMU_IPMC_INDEXm_field32_get(unit,
                                          &vlan_entry, NXT_PTRf);
        sal_memcpy(&prev_vlan_entry, &vlan_entry,
                   sizeof(mmu_ipmc_vlan_tbl_entry_t));

        soc_MMU_IPMC_INDEXm_field_get(unit, &vlan_entry,
                                               LSBf, ls_bits);
        vlan_count +=
            _shr_popcount(ls_bits[0]) + _shr_popcount(ls_bits[1]);
        if (vlan_count >= IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id)) {
            rv = BCM_E_NOT_FOUND;
            goto del_done;
        }
    }

    rv = BCM_E_NOT_FOUND;

 del_done:
    IPMC_REPL_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *	_bcm_er_repl_list_compare
 * Description:
 *	Compare HW list starting at vlan_index to the VLAN list contained
 *      in vlan_vec.
 */

STATIC int
_bcm_er_repl_list_compare(int unit, int vlan_index,
                          SHR_BITDCL *intf_vec)
{
    uint32		ms_bit, hw_ms_bit, ms_max;
    uint32		ls_bits[2], hw_ls_bits[2];
    mmu_ipmc_index_entry_t vlan_entry;
    int	                vlan_ptr, last_vlan_ptr;

    last_vlan_ptr = -1;
    vlan_ptr = vlan_index;
    ms_max = _SHR_BITDCLSIZE(IPMC_REPL_INTF_TOTAL(unit)) / 2; /* 32 -> 64 */

    for (ms_bit = 0; ms_bit < ms_max; ms_bit++) {
        ls_bits[0] = intf_vec[2 * ms_bit + 0];
        ls_bits[1] = intf_vec[2 * ms_bit + 1];
        if ((ls_bits[0] != 0) || (ls_bits[1] != 0)) {
            if (vlan_ptr == last_vlan_ptr) { /* HW list end, not app list */
                return BCM_E_NOT_FOUND;
            }
            SOC_IF_ERROR_RETURN
                (READ_MMU_IPMC_INDEXm(unit, MEM_BLOCK_ALL,
                                      vlan_ptr, &vlan_entry));
            hw_ms_bit = soc_MMU_IPMC_INDEXm_field32_get(unit,
                                       &vlan_entry, MSBf);
            soc_MMU_IPMC_INDEXm_field_get(unit, &vlan_entry,
                                                   LSBf, hw_ls_bits);
            if ((hw_ms_bit != ms_bit) || (ls_bits[0] != hw_ls_bits[0]) ||
                (ls_bits[1] != hw_ls_bits[1])) {
                return BCM_E_NOT_FOUND;
            }
            last_vlan_ptr = vlan_ptr;
            vlan_ptr = soc_MMU_IPMC_INDEXm_field32_get(unit,
                                          &vlan_entry, NXT_PTRf);
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *	_bcm_er_repl_list_free
 * Description:
 *	Release the IPMC_VLAN entries in the HW list starting at start_ptr.
 */

STATIC int
_bcm_er_repl_list_free(int unit, int start_ptr)
{
    mmu_ipmc_index_entry_t vlan_entry;
    int	                vlan_ptr, last_vlan_ptr;

    last_vlan_ptr = -1;
    vlan_ptr = start_ptr;

    while (vlan_ptr != last_vlan_ptr) {
        SOC_IF_ERROR_RETURN
            (READ_MMU_IPMC_INDEXm(unit, MEM_BLOCK_ANY,
                                               vlan_ptr, &vlan_entry));
        last_vlan_ptr = vlan_ptr;
        vlan_ptr = soc_MMU_IPMC_INDEXm_field32_get(unit,
                                          &vlan_entry, NXT_PTRf);
        IPMC_REPL_VE_USED_CLR(unit, last_vlan_ptr);
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *	_bcm_er_repl_list_write
 * Description:
 *	Write the VLAN list contained in vlan_vec into the HW table.
 *      Return the start_index and total VLAN count.
 */

STATIC int
_bcm_er_repl_list_write(int unit, int *start_index, int *count,
                        SHR_BITDCL *intf_vec)
{
    uint32		ms_bit, ms_max;
    uint32		ls_bits[2];
    mmu_ipmc_index_entry_t vlan_entry;
    int	                vlan_ptr, last_vlan_ptr, vlan_count = 0;

    last_vlan_ptr = -1;
    ms_max = _SHR_BITDCLSIZE(IPMC_REPL_INTF_TOTAL(unit)) / 2; /* 32 -> 64 */

    for (ms_bit = 0; ms_bit < ms_max; ms_bit++) {
        ls_bits[0] = intf_vec[2 * ms_bit + 0];
        ls_bits[1] = intf_vec[2 * ms_bit + 1];
        if ((ls_bits[0] != 0) || (ls_bits[1] != 0)) {
            vlan_ptr = _er_ipmc_repl_next_free_ptr(unit);
            if (last_vlan_ptr > 0) {
                /* Write previous non-zero entry */
                soc_MMU_IPMC_INDEXm_field32_set(unit,
                        &vlan_entry, NXT_PTRf,
                        (vlan_ptr > 0) ? vlan_ptr : last_vlan_ptr);
                /* If vlan_ptr <= 0, terminate so
                 * later cleanup can remove the chain. */
                SOC_IF_ERROR_RETURN
                    (WRITE_MMU_IPMC_INDEXm(unit,
                        MEM_BLOCK_ALL, last_vlan_ptr, &vlan_entry));
                if (vlan_ptr < 0) {
                    _bcm_er_repl_list_free(unit, *start_index);
                    return BCM_E_RESOURCE;
                }
            } else {
                if (vlan_ptr < 0) {
                    return BCM_E_RESOURCE;
                }
                *start_index = vlan_ptr;
            }
            sal_memset(&vlan_entry, 0, sizeof(vlan_entry));
            soc_MMU_IPMC_INDEXm_field32_set(unit, &vlan_entry,
                                               MSBf, ms_bit);
            soc_MMU_IPMC_INDEXm_field_set(unit, &vlan_entry,
                                             LSBf, ls_bits);
            IPMC_REPL_VE_USED_SET(unit, vlan_ptr);
            vlan_count +=
                _shr_popcount(ls_bits[0]) + _shr_popcount(ls_bits[1]);
            last_vlan_ptr = vlan_ptr;
        }
    }

    if (last_vlan_ptr > 0) {
        /* Write final entry */
        soc_MMU_IPMC_INDEXm_field32_set(unit,
                          &vlan_entry, NXT_PTRf, last_vlan_ptr);
        SOC_IF_ERROR_RETURN
            (WRITE_MMU_IPMC_INDEXm(unit,
                   MEM_BLOCK_ALL, last_vlan_ptr, &vlan_entry));
    }

    *count = vlan_count;
    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_er_ipmc_egress_intf_set
 * Purpose:
 *	Assign set of egress interfaces to port's replication list for chosen
 *	IPMC group.
 * Parameters:
 *	unit     - StrataSwitch PCI device unit number.
 *	ipmc_id  - The index number.
 *	port     - port to list.
 *	if_count - number of interfaces in replication list.
 *      if_array - (IN) List of interface numbers.
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_er_ipmc_egress_intf_set(int unit, int ipmc_id, bcm_port_t port,
                            int if_count, bcm_if_t *if_array)
{
    int			rv = BCM_E_NONE;
    SHR_BITDCL          *intf_vec = NULL;
    mmu_ipmc_ptr_entry_t   group_entry;
    int			list_start_ptr = 0, prev_start_ptr;
    int                 alloc_size, repl_hash, vlan_count;
    int                 if_num;
    bcm_l3_intf_t       l3_intf;
    pbmp_t              pbmp, ubmp;
    _bcm_repl_list_info_t *rli_start, *rli_current, *rli_prev;

    if (!SOC_IS_EASYRIDER(unit)) {
	return BCM_E_UNAVAIL;
    }

    if (!SOC_IPMCREPLSHR_GET(unit)) {
        return BCM_E_CONFIG;
    }

    ER_IPMC_ID(unit, ipmc_id);
    REPL_PORT_CHECK(unit, port);

    if (if_count > IPMC_REPL_INTF_TOTAL(unit)) {
        return BCM_E_PARAM;
    }

    alloc_size = SHR_BITALLOCSIZE(IPMC_REPL_INTF_TOTAL(unit));
    intf_vec = sal_alloc(alloc_size, "IPMC repl interface vector");
    if (intf_vec == NULL) {
        return BCM_E_MEMORY;
    }
    sal_memset(intf_vec, 0, alloc_size);

    IPMC_REPL_LOCK(unit);

    /* Interface validation and vector construction */
    for (if_num = 0; if_num < if_count; if_num++) {
        if (if_array[if_num] > IPMC_REPL_INTF_TOTAL(unit)) {
            rv = BCM_E_PARAM;
            goto intf_set_done;
        }
        l3_intf.l3a_intf_id = if_array[if_num];
        if ((rv = bcm_esw_l3_intf_get(unit, &l3_intf)) < 0) {
            goto intf_set_done;
        }
        if ((rv = bcm_esw_vlan_port_get(unit, l3_intf.l3a_vid,
                                        &pbmp, &ubmp)) < 0) {
            goto intf_set_done;
        }
        if (!BCM_PBMP_MEMBER(pbmp, port)) {
            rv = BCM_E_PARAM;
            goto intf_set_done;
        }
        SHR_BITSET(intf_vec, if_array[if_num]);
    }

    /* Check previous group pointer */
    if ((rv = READ_MMU_IPMC_PTRm(unit, MEM_BLOCK_ALL, 
                                 ipmc_id, &group_entry)) < 0) {
        goto intf_set_done;
    }

    prev_start_ptr = soc_MMU_IPMC_PTRm_field32_get(unit,
                       &group_entry, _ipmc_ptr_get(unit, port));

    /* Search for list already in table */
    rli_start = IPMC_REPL_LIST_INFO(unit);

    repl_hash =
        _shr_crc32b(0, (uint8 *)intf_vec, IPMC_REPL_INTF_TOTAL(unit));

    for (rli_current = rli_start; rli_current != NULL;
         rli_current = rli_current->next) {
        if (repl_hash == rli_current->hash) {
            rv = _bcm_er_repl_list_compare(unit, rli_current->index,
                                           intf_vec);
            if (rv == BCM_E_NOT_FOUND) {
                continue; /* Not a match */
            } else if (rv < 0) {
                goto intf_set_done; /* Access error */
            } else {
                break; /* Match */
            }
        }
    }

    if (rli_current != NULL) {
        /* Found a match, point to here and increase reference count */
        if (prev_start_ptr == rli_current->index) {
            /* We're already pointing to this list, so done */
            rv = BCM_E_NONE;
            goto intf_set_done;
        } else {
            list_start_ptr = rli_current->index;
            vlan_count = rli_current->list_size;
        }
    } else {
        /* Not a match, make a new chain */
        if ((rv = _bcm_er_repl_list_write(unit, &list_start_ptr,
                                         &vlan_count, intf_vec)) < 0) {
            goto intf_set_done;
        }

        if (vlan_count > 0) {
            /* Update data structures */
            alloc_size = sizeof(_bcm_repl_list_info_t);
            rli_current = sal_alloc(alloc_size, "IPMC repl list info");
            if (rli_current == NULL) {
                /* Release list */
                _bcm_er_repl_list_free(unit, list_start_ptr);
                rv = BCM_E_MEMORY;
                goto intf_set_done;
            }
            sal_memset(rli_current, 0, alloc_size);
            rli_current->index = list_start_ptr;
            rli_current->hash = repl_hash;
            rli_current->next = rli_start;
            rli_current->list_size = vlan_count;
            IPMC_REPL_LIST_INFO(unit) = rli_current;
            rli_start = rli_current;
        }
    }

    if (vlan_count > 0) {
        soc_MMU_IPMC_PTRm_field32_set(unit, &group_entry,
                              _ipmc_ptr_get(unit, port), list_start_ptr);
        if ((rv = WRITE_MMU_IPMC_PTRm(unit, MEM_BLOCK_ALL,
                                      ipmc_id, &group_entry)) < 0) {
            if (rli_current->refcount == 0) {
                /* This was new */
                _bcm_er_repl_list_free(unit, list_start_ptr);
                IPMC_REPL_LIST_INFO(unit) = rli_current->next;
                sal_free(rli_current);
            }
            goto intf_set_done;
        }

        (rli_current->refcount)++;
        /* we don't need this rli_current anymore */
    } else if (prev_start_ptr != 0) {
        soc_MMU_IPMC_PTRm_field32_set(unit, &group_entry,
                              _ipmc_ptr_get(unit, port), 0);
        if ((rv = WRITE_MMU_IPMC_PTRm(unit, MEM_BLOCK_ALL,
                                      ipmc_id, &group_entry)) < 0) {
            goto intf_set_done;
        }
    }
    IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id) = vlan_count;

    if (prev_start_ptr != 0) {
        rli_prev = NULL;
        for (rli_current = rli_start; rli_current != NULL;
             rli_current = rli_current->next) {
            if (prev_start_ptr == rli_current->index) {
                (rli_current->refcount)--;
                if (rli_current->refcount == 0) {
                    /* Free these linked list entries */
                    rv = _bcm_er_repl_list_free(unit, prev_start_ptr);
                    /* If we have an error, we'll fall out anyway */
                    if (rli_prev == NULL) {
                        IPMC_REPL_LIST_INFO(unit) = rli_current->next;
                    } else {
                        rli_prev->next = rli_current->next;
                    }
                    sal_free(rli_current);
                }
                break;
            }
            rli_prev = rli_current;
        }
    }

 intf_set_done:
    IPMC_REPL_UNLOCK(unit);
    if (intf_vec != NULL) {
        sal_free(intf_vec);
    }
    return rv;
}

/*
 * Function:
 *	bcm_er_ipmc_egress_intf_get
 * Purpose:
 *	Retreieve set of egress interfaces in port's replication list
 *      for chosen IPMC group.
 * Parameters:
 *	unit     - StrataSwitch PCI device unit number.
 *	ipmc_id  - The index number.
 *	port     - port to list.
 *	if_max   - maximum number of interfaces in replication list.
 *      if_array - (OUT) List of interface numbers.
 *	if_count - (OUT) number of interfaces returned in replication list.
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *      If the input parameter if_max = 0, return in the output parameter
 *      if_count the total number of interfaces in the specified multicast 
 *      group's replication list.
 */

int
bcm_er_ipmc_egress_intf_get(int unit, int ipmc_id, bcm_port_t port,
                            int if_max, bcm_if_t *if_array, int *if_count)
{
    int			rv = BCM_E_NONE;
    uint32              ms_bit;
    uint32              ls_bits[2];
    mmu_ipmc_ptr_entry_t   group_entry;
    mmu_ipmc_index_entry_t vlan_entry;
    int                 vlan_ptr, last_vlan_ptr;
    uint32              vlan_count;
    int                 ls_pos;

    if (!SOC_IS_EASYRIDER(unit)) {
	return BCM_E_UNAVAIL;
    }

    if (!SOC_IPMCREPLSHR_GET(unit)) {
        return BCM_E_CONFIG;
    }

    ER_IPMC_ID(unit, ipmc_id);
    REPL_PORT_CHECK(unit, port);

    if (if_max < 0) {
        return BCM_E_PARAM;
    }

    IPMC_REPL_LOCK(unit);
    if (IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id) == 0) {
        *if_count = 0;
        IPMC_REPL_UNLOCK(unit);
        return rv;
    }

    if ((rv = READ_MMU_IPMC_PTRm(unit, MEM_BLOCK_ALL, 
                                 ipmc_id, &group_entry)) < 0) {
        goto intf_get_done;
    }

    vlan_ptr = soc_MMU_IPMC_PTRm_field32_get(unit,
                       &group_entry, _ipmc_ptr_get(unit, port));

    last_vlan_ptr = -1;
    vlan_count = 0;
    while (vlan_ptr != last_vlan_ptr) {
        if ((rv = READ_MMU_IPMC_INDEXm(unit, MEM_BLOCK_ALL,
                                          vlan_ptr, &vlan_entry)) < 0) {
            goto intf_get_done;
        }
        ms_bit = soc_MMU_IPMC_INDEXm_field32_get(unit,
                                                    &vlan_entry, MSBf);
        soc_MMU_IPMC_INDEXm_field_get(unit, &vlan_entry,
                                         LSBf, ls_bits);
        for (ls_pos = 0; ls_pos < 64; ls_pos++) {
            if (ls_bits[ls_pos / 32] & (1 << (ls_pos % 32))) {
                if (if_max == 0) {
                    vlan_count++;
                } else {
                    if_array[vlan_count++] = (ms_bit * 64) + ls_pos;
                    if (vlan_count == (uint32)if_max) {
                        *if_count = vlan_count;
                        goto intf_get_done;
                    }
                }
            }
        }
        last_vlan_ptr = vlan_ptr;
        vlan_ptr = soc_MMU_IPMC_INDEXm_field32_get(unit,
                                                      &vlan_entry, NXT_PTRf);

        if (vlan_count >= IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id)) {
            break;
        }
    }

    *if_count = vlan_count;

 intf_get_done:
    IPMC_REPL_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *	bcm_er_ipmc_repl_set
 * Purpose:
 *	Assign set of VLANs provided to port's replication list for chosen
 *	IPMC group.
 * Parameters:
 *	unit     - StrataSwitch PCI device unit number.
 *	ipmc_id  - The index number.
 *	port     - port to list.
 *	vlan_vec - (IN) vector of replicated VLANs common to selected ports.
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_er_ipmc_repl_set(int unit, int ipmc_id, bcm_port_t port,
                     bcm_vlan_vector_t vlan_vec)
{
    int	rv = BCM_E_NONE;
    bcm_if_t *if_array = NULL;
    bcm_l3_intf_t l3_intf;
    pbmp_t pbmp, ubmp;
    int  intf_num, intf_max, alloc_size, vid;

    if (!SOC_IS_EASYRIDER(unit)) {
	return BCM_E_UNAVAIL;
    }

    if (!SOC_IPMCREPLSHR_GET(unit)) {
        return BCM_E_CONFIG;
    }

    ER_IPMC_ID(unit, ipmc_id);
    REPL_PORT_CHECK(unit, port);

    intf_max = IPMC_REPL_INTF_TOTAL(unit);
    alloc_size = intf_max * sizeof(bcm_if_t);
    if_array = sal_alloc(alloc_size, "IPMC repl interface array");
    if (if_array == NULL) {
        return BCM_E_MEMORY;
    }

    sal_memset(if_array, 0, alloc_size);
    intf_num = 0;
    for (vid = BCM_VLAN_MIN; vid < BCM_VLAN_MAX; vid++) {
        if (BCM_VLAN_VEC_GET(vlan_vec, vid)) {
            if ((rv = bcm_esw_vlan_port_get(unit, vid, &pbmp, &ubmp)) < 0) {
                sal_free(if_array);
                return rv;
            }
            if (!BCM_PBMP_MEMBER(pbmp, port)) {
                sal_free(if_array);
                return BCM_E_PARAM;
            }
            l3_intf.l3a_vid = vid;
            if (bcm_esw_l3_intf_find_vlan(unit, &l3_intf) < 0) {
                sal_free(if_array);
                return BCM_E_PARAM;
            }
            if_array[intf_num++] = l3_intf.l3a_intf_id;
        }
    }

    rv = bcm_er_ipmc_egress_intf_set(unit, ipmc_id, port,
                                     intf_num, if_array);

    sal_free(if_array);
    return rv;
}

int
_bcm_er_ipmc_egress_intf_list_add(int unit, int ipmc_id, bcm_port_t port,
                                  int id)
{
    int *if_array = NULL;
    int intf_num, intf_max, alloc_size, rv = BCM_E_NONE;

    if (!SOC_IPMCREPLSHR_GET(unit)) {
        return BCM_E_CONFIG;
    }

    ER_IPMC_ID(unit, ipmc_id);
    REPL_PORT_CHECK(unit, port);

    intf_max = IPMC_REPL_INTF_TOTAL(unit);
    alloc_size = intf_max * sizeof(int);
    if_array = sal_alloc(alloc_size, "IPMC repl interface array");
    if (if_array == NULL) {
        return BCM_E_MEMORY;
    }

    IPMC_REPL_LOCK(unit);
    rv = bcm_er_ipmc_egress_intf_get(unit, ipmc_id, port, intf_max,
                                     if_array, &intf_num);
    if (BCM_SUCCESS(rv)) {
        if (intf_num < intf_max) {
            if_array[intf_num++] = id;
            rv = bcm_er_ipmc_egress_intf_set(unit, ipmc_id, port,
                                             intf_num, if_array);
        } else {
            rv = BCM_E_EXISTS;
        }
    }

    IPMC_REPL_UNLOCK(unit);
    sal_free(if_array); 
    return rv;
}

int
_bcm_er_ipmc_egress_intf_list_delete(int unit, int ipmc_id, bcm_port_t port,
                                     int if_max, int id)
{
    int  *if_array = NULL;
    int  alloc_size, if_count, if_cur, match, rv = BCM_E_NONE;

    if (!SOC_IPMCREPLSHR_GET(unit)) {
        return BCM_E_CONFIG;
    }

    ER_IPMC_ID(unit, ipmc_id);
    REPL_PORT_CHECK(unit, port);

    if (!IPMC_REPL_PORT_VLAN_COUNT(unit, port, ipmc_id)) {
        return BCM_E_NOT_FOUND;
    } else if ((if_max <= 0) || ((uint32)if_max > IPMC_REPL_INTF_TOTAL(unit))) {
        return BCM_E_PARAM;
    }
    alloc_size = if_max * sizeof(int);
    if_array = sal_alloc(alloc_size, "IPMC repl interface array");
    if (if_array == NULL) {
        return BCM_E_MEMORY;
    }

    IPMC_REPL_LOCK(unit);
    rv = bcm_er_ipmc_egress_intf_get(unit, ipmc_id, port, if_max,
                                     if_array, &if_count);
    if (BCM_SUCCESS(rv)) {
        match = FALSE;
        for (if_cur = 0; if_cur < if_count; if_cur++) {
            if (match) {
                if_array[if_cur - 1] = if_array[if_cur];
            } else {
                if (if_array[if_cur] == id) {
                    match = TRUE;
                }
            }
        }
        if (match) {
            if_count--;
            rv = bcm_er_ipmc_egress_intf_set(unit, ipmc_id, port,
                                             if_count, if_array);
        } else { 
            rv = BCM_E_NOT_FOUND;
        }
    }
    
    IPMC_REPL_UNLOCK(unit);
    sal_free(if_array);
    return rv;
}

#else /* INCLUDE_L3 && INCLUDE_EASYRIDER */
int _bcm_esw_easyrider_ipmc_not_empty;
#endif	/* INCLUDE_L3  && INCLUDE_EASYRIDER */
