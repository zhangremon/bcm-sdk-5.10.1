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
 * Purpose: 	Tracks and manages IPMC tables in Tucana.
 */

#ifdef INCLUDE_L3

#include <soc/mem.h>

#include <bcm/error.h>
#include <bcm/ipmc.h>

#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/tucana.h>
#include <bcm_int/esw/stg.h>

#include <bcm_int/esw_dispatch.h>

#define REPL_PORT_CHECK(unit, port) \
    if (!IS_E_PORT(unit, port)) { return BCM_E_PARAM; }

#define REPL_GROUP_CHECK(unit, group) \
     if ((group < soc_mem_index_min(unit, GE_IPMC_VECm)) || \
         (group > soc_mem_index_max(unit, GE_IPMC_VECm)))   \
         { return BCM_E_PARAM; } 

#define TUCANA_IPMC_GROUPS        256

#define TUCANA_IPMC_GE_VLANS      32
#define TUCANA_IPMC_FE_VLANS      8

#define TUCANA_IPMC_PORT_VLANS(unit, port) \
        (IS_GE_PORT(unit, port) ? TUCANA_IPMC_GE_VLANS : \
        (IS_FE_PORT(unit, port) ? TUCANA_IPMC_FE_VLANS : 0))

#define TUCANA_IPMC_MMU_PORT(port) \
        (((port) > 31) ? ((port) - 3) : (port))

#define TUCANA_IPMC_MMU_INDEX(port, group) \
        (((TUCANA_IPMC_MMU_PORT(port) & 0x3f) << 8) | (group & 0xff))

#define TUCANA_IPMC_FE_VEC_INDEX(port, group) \
        (((port % 8) * TUCANA_IPMC_GROUPS) + group)

#define TUCANA_IPMC_FE_VLAN_INDEX(port, vlan_index) \
        (((port % 8) * TUCANA_IPMC_FE_VLANS) + vlan_index)

typedef struct _tucana_repl_port_info_s {
    int32 vlan_count[TUCANA_IPMC_GROUPS];  /* # VLANs the port repl to */
    int32 *vlan_refs;               /* use count for VLAN at this slot */
    uint32 stg_enabled[_SHR_BITDCLSIZE(BCM_VLAN_COUNT)]; 
                                           /* VLAN allowed by STG STP */
} _tucana_repl_port_info_t;

static _tucana_repl_port_info_t **_tucana_repl_info[BCM_MAX_NUM_UNITS];

#define TUCANA_IPMC_REPL_INFO(_unit_, _port_) \
	_tucana_repl_info[_unit_][_port_]
#define TUCANA_IPMC_REPL_VLAN_COUNT(_unit_, _port_, _group_) \
	_tucana_repl_info[_unit_][_port_]->vlan_count[_group_]
#define TUCANA_IPMC_REPL_VLAN_REFS(_unit_, _port_, _ix_) \
	_tucana_repl_info[_unit_][_port_]->vlan_refs[_ix_]

#define TUCANA_IPMC_REPL_VLAN_STG_ENABLE(_unit_, _port_, _vid_) \
	((SHR_BITGET(_tucana_repl_info[_unit_][_port_]->stg_enabled, \
        _vid_)) != 0)
#define TUCANA_IPMC_REPL_VLAN_STG_SET(_unit_, _port_, _vid_) \
	(SHR_BITSET(_tucana_repl_info[_unit_][_port_]->stg_enabled, _vid_))
#define TUCANA_IPMC_REPL_VLAN_STG_CLR(_unit_, _port_, _vid_) \
	(SHR_BITCLR(_tucana_repl_info[_unit_][_port_]->stg_enabled, _vid_))

#define IPMC_REPL_LOCK(unit)                   \
    {                                          \
        soc_mem_lock(unit, GE_IPMC_VECm);      \
        soc_mem_lock(unit, FE_IPMC_VECm);      \
        soc_mem_lock(unit, GE_IPMC_VLANm);     \
        soc_mem_lock(unit, FE_IPMC_VLANm);     \
        soc_mem_lock(unit, MMU_MEMORIES1_IPMCREPm); \
    }
#define IPMC_REPL_UNLOCK(unit)                 \
    {                                          \
        soc_mem_unlock(unit, MMU_MEMORIES1_IPMCREPm); \
        soc_mem_unlock(unit, FE_IPMC_VLANm);   \
        soc_mem_unlock(unit, GE_IPMC_VLANm);   \
        soc_mem_unlock(unit, FE_IPMC_VECm);    \
        soc_mem_unlock(unit, GE_IPMC_VECm);    \
    }

/* Function:
 *      _bcm_tucana_mstp_state_init
 * Purpose:
 *      Initialize the VLAN MSTP state data.
 */
STATIC int
_bcm_tucana_mstp_state_init(int unit)
{
    bcm_port_t      port;
    bcm_stg_t       stg, *list;
    int             stg_num, stg_p, vlan_num, vlan_p, stp_state, enable;
    int             rv = BCM_E_NONE, rv2 = BCM_E_NONE, rv3 = BCM_E_NONE;
    bcm_vlan_t      vid, *vlist;

    BCM_IF_ERROR_RETURN
        (bcm_esw_stg_list(unit, &list, &stg_num));

    /* Iterate over defined STGs */
    for (stg_p = 0; stg_p < stg_num; stg_p++) {
        stg = list[stg_p];
        rv = bcm_esw_stg_vlan_list(unit, stg, &vlist, &vlan_num);
        if (BCM_FAILURE(rv)) {
            break;
        }

        PBMP_E_ITER(unit, port) {
            rv = bcm_esw_stg_stp_get(unit, stg, port, &stp_state);
            if (BCM_FAILURE(rv)) {
                break;
            }
            enable = _BCM_STG_STP_TO_MSTP_MASK_ENABLE(stp_state);
            for (vlan_p = 0; vlan_p < vlan_num; vlan_p++) {
                vid = vlist[vlan_p];
                if (enable) {    
                    TUCANA_IPMC_REPL_VLAN_STG_SET(unit, port, vid);
                } else {
                    TUCANA_IPMC_REPL_VLAN_STG_CLR(unit, port, vid);
                }
            }
        }

        rv2 = bcm_esw_stg_vlan_list_destroy(unit, vlist, vlan_num);
        if (BCM_FAILURE(rv) || BCM_FAILURE(rv2)) {
            break;
        }
    }

    rv3 = bcm_esw_stg_list_destroy(unit, list, stg_num);
  
    return BCM_FAILURE(rv) ? rv : (BCM_FAILURE(rv2) ? rv2 : rv3);
}

/*
 * Function:
 *	bcm_tucana_ipmc_repl_init
 * Purpose:
 *	Initialize IPMC replication.
 * Parameters:
 *	unit     - SOC unit #
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_tucana_ipmc_repl_init(int unit)
{
    bcm_port_t		port;
    int			alloc_size;

    BCM_IF_ERROR_RETURN
        (bcm_tucana_ipmc_repl_detach(unit));

    alloc_size = SOC_MAX_NUM_PORTS * sizeof(_tucana_repl_port_info_t *);
    _tucana_repl_info[unit] = sal_alloc(alloc_size, "IPMC repl ports");
    if (_tucana_repl_info[unit] == NULL) {
        return BCM_E_MEMORY;
    }
    sal_memset(_tucana_repl_info[unit], 0, alloc_size);

    PBMP_ITER(PBMP_GE_ALL(unit), port) {
        alloc_size = sizeof(_tucana_repl_port_info_t);
        TUCANA_IPMC_REPL_INFO(unit, port) =
            sal_alloc(alloc_size, "IPMC repl port info");
        if (TUCANA_IPMC_REPL_INFO(unit, port) == NULL) {
	    bcm_tucana_ipmc_repl_detach(unit);
            return BCM_E_MEMORY;
        }
        sal_memset(TUCANA_IPMC_REPL_INFO(unit, port), 0, alloc_size);

        alloc_size = TUCANA_IPMC_GE_VLANS * sizeof(int32);
        _tucana_repl_info[unit][port]->vlan_refs =
            sal_alloc(alloc_size, "IPMC repl port VLAN references");
        if (_tucana_repl_info[unit][port]->vlan_refs == NULL) {
	    bcm_tucana_ipmc_repl_detach(unit);
            return BCM_E_MEMORY;
        }
        sal_memset(_tucana_repl_info[unit][port]->vlan_refs, 0, alloc_size);
    }
    
    PBMP_ITER(PBMP_FE_ALL(unit), port) {
        alloc_size = sizeof(_tucana_repl_port_info_t);
        TUCANA_IPMC_REPL_INFO(unit, port) =
            sal_alloc(alloc_size, "IPMC repl port info");
        if (TUCANA_IPMC_REPL_INFO(unit, port) == NULL) {
	    bcm_tucana_ipmc_repl_detach(unit);
            return BCM_E_MEMORY;
        }
        sal_memset(TUCANA_IPMC_REPL_INFO(unit, port), 0, alloc_size);

        alloc_size = TUCANA_IPMC_FE_VLANS * sizeof(int32);
        _tucana_repl_info[unit][port]->vlan_refs =
            sal_alloc(alloc_size, "IPMC repl port VLAN references");
        if (_tucana_repl_info[unit][port]->vlan_refs == NULL) {
	    bcm_tucana_ipmc_repl_detach(unit);
            return BCM_E_MEMORY;
        }
        sal_memset(_tucana_repl_info[unit][port]->vlan_refs, 0, alloc_size);
    }
    
    if (!SAL_BOOT_SIMULATION) {
        SOC_IF_ERROR_RETURN
            (soc_mem_clear(unit, GE_IPMC_VECm, COPYNO_ALL, FALSE));
        SOC_IF_ERROR_RETURN
            (soc_mem_clear(unit, GE_IPMC_VLANm, COPYNO_ALL, FALSE));

        SOC_IF_ERROR_RETURN
            (soc_mem_clear(unit, FE_IPMC_VECm, COPYNO_ALL, FALSE));
        SOC_IF_ERROR_RETURN
            (soc_mem_clear(unit, FE_IPMC_VLANm, COPYNO_ALL, FALSE));

        SOC_IF_ERROR_RETURN
            (soc_mem_clear(unit, MMU_MEMORIES1_IPMCREPm, COPYNO_ALL, FALSE));
    }

    /* Access the STG state to fill the VLAN state vector */
    if (soc_feature(unit, soc_feature_mstp_mask)) {
        BCM_IF_ERROR_RETURN
            (_bcm_tucana_mstp_state_init(unit));
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *	_bcm_tucana_ipmc_repl_disable
 * Purpose:
 *	Internal routine to disable IPMC replication on a port.
 */

int
_bcm_tucana_ipmc_repl_disable(int unit, bcm_port_t port)
{
    int ix, group;
    int	blk = SOC_PORT_BLOCK(unit, port);

    for (group = 0; group < TUCANA_IPMC_GROUPS; group++) {
        if (TUCANA_IPMC_REPL_VLAN_COUNT(unit, port, group)) {
            if (!SAL_BOOT_SIMULATION) {
                SOC_IF_ERROR_RETURN
                    (WRITE_MMU_MEMORIES1_IPMCREPm(unit, MEM_BLOCK_ALL, 
                         TUCANA_IPMC_MMU_INDEX(port, group), 
                         soc_mem_entry_null(unit, MMU_MEMORIES1_IPMCREPm)));
            }
            TUCANA_IPMC_REPL_VLAN_COUNT(unit, port, group) = 0;
        }
    }

    if (!SAL_BOOT_SIMULATION) {
        if (IS_GE_PORT(unit, port)) {
            SOC_IF_ERROR_RETURN
                (soc_mem_clear(unit, GE_IPMC_VECm, blk, FALSE));
        } else {
            SOC_IF_ERROR_RETURN
                (soc_mem_clear(unit, FE_IPMC_VECm, blk, FALSE));
        }
    }

    if (_tucana_repl_info[unit][port]->vlan_refs != NULL) {
        for (ix = 0; ix < TUCANA_IPMC_PORT_VLANS(unit, port); ix++) {
            TUCANA_IPMC_REPL_VLAN_REFS(unit, port, ix) = 0;
        }
    }

    return BCM_E_NONE;
}

static soc_field_t _tucana_epc_vlan_fwd_fields[] = {
    VLAN_FWD_STATE0f,
    VLAN_FWD_STATE1f,
    VLAN_FWD_STATE2f,
    VLAN_FWD_STATE3f,
    VLAN_FWD_STATE4f,
    VLAN_FWD_STATE5f,
    VLAN_FWD_STATE6f,
    VLAN_FWD_STATE7f,
};

/* Function:
 *      _bcm_tucana_mstp_mask_update
 * Purpose:
 *      Update the MSTP mask.
 */
STATIC int
_bcm_tucana_mstp_mask_update(int unit, bcm_port_t port,
                             int vlan_index, int enable)
{
    uint64       val64;
    int          vlan_mask;

    if (IS_GE_PORT(unit, port)) {
        BCM_IF_ERROR_RETURN
            (READ_GPC_VLAN_FWD_STATEr(unit, port, &val64));
        vlan_mask = soc_reg64_field32_get(unit, GPC_VLAN_FWD_STATEr, val64,
                                          VLAN_FWD_STATEf);
        if (enable) {
            vlan_mask |= ((uint32)1) << vlan_index;
        } else {
            vlan_mask &= ~(((uint32)1) << vlan_index);
        }
        soc_reg64_field32_set(unit, GPC_VLAN_FWD_STATEr, &val64,
                              VLAN_FWD_STATEf, vlan_mask);
        BCM_IF_ERROR_RETURN
            (WRITE_GPC_VLAN_FWD_STATEr(unit, port, val64));
    } else {
        BCM_IF_ERROR_RETURN
            (READ_EPC_VLAN_FWD_STATEr(unit, port, &val64));
        vlan_mask = soc_reg64_field32_get(unit, EPC_VLAN_FWD_STATEr, val64,
                              _tucana_epc_vlan_fwd_fields[port % 8]);
        if (enable) {
            vlan_mask |= ((uint32)1) << vlan_index;
        } else {
            vlan_mask &= ~(((uint32)1) << vlan_index);
        }
        soc_reg64_field32_set(unit, EPC_VLAN_FWD_STATEr, &val64,
                  _tucana_epc_vlan_fwd_fields[port % 8], vlan_mask);
        BCM_IF_ERROR_RETURN
            (WRITE_EPC_VLAN_FWD_STATEr(unit, port, val64));
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_tucana_ipmc_repl_detach
 * Purpose:
 *	Initialize IPMC replication.
 * Parameters:
 *	unit     - SOC unit #
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_tucana_ipmc_repl_detach(int unit)
{
    int			rv = BCM_E_NONE;
    bcm_port_t		port;

    if (_tucana_repl_info[unit] != NULL) {
	PBMP_ITER(PBMP_E_ALL(unit), port) {
	    if (_tucana_repl_info[unit][port] != NULL) {
		if (rv >= 0) {
		    rv = _bcm_tucana_ipmc_repl_disable(unit, port);
		}

                if (_tucana_repl_info[unit][port]->vlan_refs != NULL) {
                    sal_free(_tucana_repl_info[unit][port]->vlan_refs);
                }
		sal_free(_tucana_repl_info[unit][port]);
                _tucana_repl_info[unit][port] = NULL;
	    }
	}

	sal_free(_tucana_repl_info[unit]);
	_tucana_repl_info[unit] = NULL;
    }

    return rv;
}

/*
 * Function:
 *	bcm_tucana_ipmc_repl_get
 * Purpose:
 *	Return set of VLANs for port's replication list for chosen
 *	IPMC group.
 * Parameters:
 *	unit     - StrataSwitch PCI device unit number.
 *	group    - The IPMC group ID
 *	port     - port to list.
 *	vlan_vec - (OUT) vector of replicated VLANs common to selected ports.
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_tucana_ipmc_repl_get(int unit, int group, bcm_port_t port, 
                         bcm_vlan_vector_t vlan_vec)
{
    int                 rv = BCM_E_NONE;
    int                 ix;
    fe_ipmc_vec_entry_t fe_vec_entry;
    ge_ipmc_vec_entry_t ge_vec_entry;
    fe_ipmc_vlan_entry_t fe_vlan_entry;
    ge_ipmc_vlan_entry_t ge_vlan_entry;
    uint32              bitmap, vlan;
    int			blk = SOC_PORT_BLOCK(unit, port);

    REPL_PORT_CHECK(unit, port);
  
    REPL_GROUP_CHECK(unit, group);

    BCM_VLAN_VEC_ZERO(vlan_vec);

    IPMC_REPL_LOCK(unit);

    if (IS_GE_PORT(unit, port)) {
        sal_memset(&ge_vec_entry, 0, sizeof(ge_vec_entry));
        if ((rv = READ_GE_IPMC_VECm(unit, blk, group, &ge_vec_entry)) < 0) {
            goto get_done;
        }
        soc_GE_IPMC_VECm_field_get(unit, &ge_vec_entry, BITMAPf, &bitmap);

        for (ix = 0; ix < TUCANA_IPMC_GE_VLANS; ix++) {
            if (bitmap & (1 << ix)) {
                sal_memset(&ge_vlan_entry, 0, sizeof(ge_vlan_entry));
                if ((rv = READ_GE_IPMC_VLANm(unit, blk, ix, 
                                             &ge_vlan_entry)) < 0) {
                    goto get_done;
                }
                soc_GE_IPMC_VLANm_field_get(unit, &ge_vlan_entry, 
                                            VLANIDf, &vlan);
                SHR_BITSET(vlan_vec, vlan);
            }
        }
    } else { /* FE port */
        sal_memset(&fe_vec_entry, 0, sizeof(fe_vec_entry));
        if ((rv = READ_FE_IPMC_VECm(unit, blk,
                                    TUCANA_IPMC_FE_VEC_INDEX(port, group),
                                    &fe_vec_entry)) < 0) {
            goto get_done;
        }
        soc_FE_IPMC_VECm_field_get(unit, &fe_vec_entry, BITMAPf, &bitmap);

        for (ix = 0; ix < TUCANA_IPMC_FE_VLANS; ix++) {
            if (bitmap & (1 << ix)) {
                sal_memset(&fe_vlan_entry, 0, sizeof(fe_vlan_entry));
                if ((rv = READ_FE_IPMC_VLANm(unit, blk,
                                    TUCANA_IPMC_FE_VLAN_INDEX(port, ix), 
                                    &fe_vlan_entry)) < 0) {
                    goto get_done;
                }
                soc_FE_IPMC_VLANm_field_get(unit, &fe_vlan_entry, 
                                            VLANIDf, &vlan);
                BCM_VLAN_VEC_SET(vlan_vec, vlan);
            }
        }
    }

 get_done:
    IPMC_REPL_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *	bcm_tucana_ipmc_repl_add
 * Purpose:
 *	Add VLAN to selected ports' replication list for chosen
 *	IPMC group.
 * Parameters:
 *	unit     - StrataSwitch PCI device unit number.
 *	group    - The IPMC group ID
 *	port     - port to add.
 *	vlan     - replicated VLAN to add to port.
 * Returns:
 *	BCM_E_XXX
 * Note:
 *      Because the IPMC_VLAN table holds all the possible VLAN IDs
 *      for replication for all IPMC groups for a particular port,
 *      the number of distinct VLANs for all IPMC groups per port
 *      is 32 for GE, 8 for FE.  So even if a port may belong to
 *      more than 32 VLANs, it is not possible to replicate
 *      to all of them, even by different IPMC groups.
 */

int
bcm_tucana_ipmc_repl_add(int unit, int group, bcm_port_t port, 
                         bcm_vlan_t vlan)
{
    fe_ipmc_vec_entry_t fe_vec_entry, pre_fe_vec_entry;
    ge_ipmc_vec_entry_t ge_vec_entry, pre_ge_vec_entry;
    fe_ipmc_vlan_entry_t fe_vlan_entry;
    ge_ipmc_vlan_entry_t ge_vlan_entry;
    mmu_memories1_ipmcrep_entry_t ipmcrep_entry;
    int                 vlans_per_port = TUCANA_IPMC_PORT_VLANS(unit, port);
    int			blk = SOC_PORT_BLOCK(unit, port);
    int                 empty_vs, vlan_slot;
    uint32              bitmap, mem_vlan;
    int			rv = BCM_E_NONE;

    REPL_PORT_CHECK(unit, port);

    REPL_GROUP_CHECK(unit, group);

    IPMC_REPL_LOCK(unit);

    /* First, scan for VLAN position on this port */
    empty_vs = -1;
    for (vlan_slot = 0; vlan_slot < vlans_per_port; vlan_slot++) {
        if (TUCANA_IPMC_REPL_VLAN_REFS(unit, port, vlan_slot)) {
            /* A VLAN is already in this slot.  Is it the one we need? */
            if (IS_GE_PORT(unit, port)) {
                sal_memset(&ge_vlan_entry, 0, sizeof(ge_vlan_entry));
                if ((rv = READ_GE_IPMC_VLANm(unit, blk, vlan_slot, 
                                             &ge_vlan_entry)) < 0) {
                    goto add_done;
                }
                mem_vlan = soc_GE_IPMC_VLANm_field32_get(unit, &ge_vlan_entry, 
                                                         VLANIDf);
            } else {
                sal_memset(&fe_vlan_entry, 0, sizeof(fe_vlan_entry));
                if ((rv = READ_FE_IPMC_VLANm(unit, blk,
                                  TUCANA_IPMC_FE_VLAN_INDEX(port, vlan_slot), 
                                  &fe_vlan_entry)) < 0) {
                    goto add_done;
                }
                mem_vlan = soc_FE_IPMC_VLANm_field32_get(unit, &fe_vlan_entry, 
                                                         VLANIDf);
            }

            if (mem_vlan == vlan) {
                break;
            }
        } else if (empty_vs < 0) {
            /* Note first empty VLAN slot, in case we need it below */
            empty_vs = vlan_slot;
        }
    }

    if (vlan_slot == vlans_per_port) {
        /* Not found in table */
        if (empty_vs < 0) { /* Table is full */
            rv = BCM_E_RESOURCE;
            goto add_done;
        } else {
            vlan_slot = empty_vs; /* Add VLAN to empty slot */
            if (IS_GE_PORT(unit, port)) {
                sal_memset(&ge_vlan_entry, 0, sizeof(ge_vlan_entry));
                soc_GE_IPMC_VLANm_field32_set(unit, &ge_vlan_entry, 
                                              VLANIDf, vlan);
                if ((rv = WRITE_GE_IPMC_VLANm(unit, blk, vlan_slot, 
                                              &ge_vlan_entry)) < 0) {
                    goto add_done;
                }
            } else {
                sal_memset(&fe_vlan_entry, 0, sizeof(fe_vlan_entry));
                soc_FE_IPMC_VLANm_field32_set(unit, &fe_vlan_entry, 
                                              VLANIDf, vlan);
                if ((rv = WRITE_FE_IPMC_VLANm(unit, blk,
                                    TUCANA_IPMC_FE_VLAN_INDEX(port, vlan_slot), 
                                    &fe_vlan_entry)) < 0) {
                    goto add_done;
                }
            }

            if (soc_feature(unit, soc_feature_mstp_mask)) {
                /* Update MSTP mask */
                if ((rv = 
                     _bcm_tucana_mstp_mask_update(unit, port, vlan_slot,
                     TUCANA_IPMC_REPL_VLAN_STG_ENABLE(unit, port,
                                                      vlan))) < 0) {
                    goto add_done;
                }
            }
        }
    }

    /* Now the desired VLAN is in (vlan_slot) index of IPMC_VLAN table */
    if (IS_GE_PORT(unit, port)) {
        sal_memset(&pre_ge_vec_entry, 0, sizeof(pre_ge_vec_entry));
        if ((rv = READ_GE_IPMC_VECm(unit, blk, group, 
                                    &pre_ge_vec_entry)) < 0) {
            goto add_done;
        }
        bitmap = soc_GE_IPMC_VECm_field32_get(unit, &pre_ge_vec_entry, 
                                                         BITMAPf);
    } else {
        sal_memset(&pre_fe_vec_entry, 0, sizeof(pre_fe_vec_entry));
        if ((rv = READ_FE_IPMC_VECm(unit, blk,
                                    TUCANA_IPMC_FE_VEC_INDEX(port, group),
                                    &pre_fe_vec_entry)) < 0) {
            goto add_done;
        }
        bitmap = soc_FE_IPMC_VECm_field32_get(unit, &pre_fe_vec_entry, 
                                              BITMAPf);
    }

    if (bitmap & (1 << vlan_slot)) {
        rv = BCM_E_EXISTS;
        goto add_done;
    } else {
        bitmap |= (1 << vlan_slot);
    }

    if (IS_GE_PORT(unit, port)) {
        sal_memset(&ge_vec_entry, 0, sizeof(ge_vec_entry));
        soc_GE_IPMC_VECm_field32_set(unit, &ge_vec_entry, BITMAPf, bitmap);
        if ((rv = WRITE_GE_IPMC_VECm(unit, blk, group, &ge_vec_entry)) < 0) {
            goto add_done;
        }
    } else {
        sal_memset(&fe_vec_entry, 0, sizeof(fe_vec_entry));
        soc_FE_IPMC_VECm_field32_set(unit, &fe_vec_entry, BITMAPf, bitmap);
        if ((rv = WRITE_FE_IPMC_VECm(unit, blk,
                                     TUCANA_IPMC_FE_VEC_INDEX(port, group), 
                                     &fe_vec_entry)) < 0) {
            goto add_done;
        }
    }

    sal_memset(&ipmcrep_entry, 0, sizeof(ipmcrep_entry));
    /* IPMCREP mem entry is (count - 1) */
    soc_MMU_MEMORIES1_IPMCREPm_field32_set(unit, &ipmcrep_entry, IPMCREPf, 
                   TUCANA_IPMC_REPL_VLAN_COUNT(unit, port, group));
    if ((rv = WRITE_MMU_MEMORIES1_IPMCREPm(unit, MEM_BLOCK_ALL, 
                   TUCANA_IPMC_MMU_INDEX(port, group), &ipmcrep_entry)) < 0) {
        /* Well, we should undo the vector write above */
        if (IS_GE_PORT(unit, port)) {
            if ((rv = WRITE_GE_IPMC_VECm(unit, blk, group, 
                                         &pre_ge_vec_entry)) < 0) {
                /* Now we're really in a mess.  Bah. */
                goto add_done;
            }
        } else {
            if ((rv = WRITE_FE_IPMC_VECm(unit, blk,
                                         TUCANA_IPMC_FE_VEC_INDEX(port, group), 
                                         &pre_fe_vec_entry)) < 0) {
                /* Now we're really in a mess.  Bah. */
                goto add_done;
            }
        }
        goto add_done;
    }

    /* Update database info last, after memory writes committed */
    TUCANA_IPMC_REPL_VLAN_COUNT(unit, port, group)++;
    TUCANA_IPMC_REPL_VLAN_REFS(unit, port, vlan_slot)++;

 add_done:
    IPMC_REPL_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *	bcm_tucana_ipmc_repl_delete
 * Purpose:
 *	Remove VLAN from selected ports' replication list for chosen
 *	IPMC group.
 * Parameters:
 *	unit     - StrataSwitch PCI device unit number.
 *	group    - The group number.
 *	port     - port to remove.
 *	vlan     - replicated VLAN to delete from port.
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_tucana_ipmc_repl_delete(int unit, int group, bcm_port_t port, 
                            bcm_vlan_t vlan)
{
    int			rv = BCM_E_NONE;
    int                 vlans_per_port = TUCANA_IPMC_PORT_VLANS(unit, port);
    int                 vlan_slot;
    fe_ipmc_vec_entry_t fe_vec_entry;
    ge_ipmc_vec_entry_t ge_vec_entry;
    fe_ipmc_vlan_entry_t fe_vlan_entry;
    ge_ipmc_vlan_entry_t ge_vlan_entry;
    mmu_memories1_ipmcrep_entry_t ipmcrep_entry;
    uint32              bitmap, mem_vlan;
    int			blk = SOC_PORT_BLOCK(unit, port);

    REPL_PORT_CHECK(unit, port);

    REPL_GROUP_CHECK(unit, group);

    IPMC_REPL_LOCK(unit);

    /* First, scan for VLAN position on this port */
    for (vlan_slot = 0; vlan_slot < vlans_per_port; vlan_slot++) {
        if (TUCANA_IPMC_REPL_VLAN_REFS(unit, port, vlan_slot)) {
            /* A VLAN is already in this slot.  Is it the one we need? */
            if (IS_GE_PORT(unit, port)) {
                sal_memset(&ge_vlan_entry, 0, sizeof(ge_vlan_entry));
                if ((rv = READ_GE_IPMC_VLANm(unit, blk, vlan_slot, 
                                             &ge_vlan_entry)) < 0) {
                    goto del_done;
                }
                mem_vlan = soc_GE_IPMC_VLANm_field32_get(unit, &ge_vlan_entry, 
                                                         VLANIDf);
            } else {
                sal_memset(&fe_vlan_entry, 0, sizeof(fe_vlan_entry));
                if ((rv = READ_FE_IPMC_VLANm(unit, blk,
                                    TUCANA_IPMC_FE_VLAN_INDEX(port, vlan_slot), 
                                    &fe_vlan_entry)) < 0) {
                    goto del_done;
                }
                mem_vlan = soc_FE_IPMC_VLANm_field32_get(unit, &fe_vlan_entry, 
                                                         VLANIDf);
            }

            if (mem_vlan == vlan) {
                break;
            }
        }
    }

    if (vlan_slot == vlans_per_port) {
        rv = BCM_E_NOT_FOUND;
        goto del_done;
    }

    /* Now the desired VLAN is in (vlan_slot) index of IPMC_VLAN table */
    if (IS_GE_PORT(unit, port)) {
        sal_memset(&ge_vec_entry, 0, sizeof(ge_vec_entry));
        if ((rv = READ_GE_IPMC_VECm(unit, blk, group, 
                                    &ge_vec_entry)) < 0) {
            goto del_done;
        }
        bitmap = soc_GE_IPMC_VECm_field32_get(unit, &ge_vec_entry, 
                                                         BITMAPf);
    } else {
        sal_memset(&fe_vec_entry, 0, sizeof(fe_vec_entry));
        if ((rv = READ_FE_IPMC_VECm(unit, blk,
                                    TUCANA_IPMC_FE_VEC_INDEX(port, group), 
                                    &fe_vec_entry)) < 0) {
            goto del_done;
        }
        bitmap = soc_FE_IPMC_VECm_field32_get(unit, &fe_vec_entry, 
                                              BITMAPf);
    }

    if (bitmap & (1 << vlan_slot)) {
        bitmap &= ~(1 << vlan_slot);
    } else {
        rv = BCM_E_NOT_FOUND;
        goto del_done;
    }

    if (IS_GE_PORT(unit, port)) {
        sal_memset(&ge_vec_entry, 0, sizeof(ge_vec_entry));
        soc_GE_IPMC_VECm_field32_set(unit, &ge_vec_entry, BITMAPf, bitmap);
        if ((rv = WRITE_GE_IPMC_VECm(unit, blk, group, &ge_vec_entry)) < 0) {
            goto del_done;
        }
    } else {
        sal_memset(&fe_vec_entry, 0, sizeof(fe_vec_entry));
        soc_FE_IPMC_VECm_field32_set(unit, &fe_vec_entry, BITMAPf, bitmap);
        if ((rv = WRITE_FE_IPMC_VECm(unit, blk,
                                     TUCANA_IPMC_FE_VEC_INDEX(port, group), 
                                     &fe_vec_entry)) < 0) {
            goto del_done;
        }
    }

    /* Update database info */
    TUCANA_IPMC_REPL_VLAN_COUNT(unit, port, group)--;
    TUCANA_IPMC_REPL_VLAN_REFS(unit, port, vlan_slot)--;

    /* 1 & 0 repls are overloaded */
    if (TUCANA_IPMC_REPL_VLAN_COUNT(unit, port, group) > 0) {
        sal_memset(&ipmcrep_entry, 0, sizeof(ipmcrep_entry));
        /* IPMCREP mem entry is (count - 1) */
        soc_MMU_MEMORIES1_IPMCREPm_field32_set(unit, &ipmcrep_entry, IPMCREPf, 
                          TUCANA_IPMC_REPL_VLAN_COUNT(unit, port, group) - 1);
        if ((rv = WRITE_MMU_MEMORIES1_IPMCREPm(unit, MEM_BLOCK_ALL, 
                            TUCANA_IPMC_MMU_INDEX(port, group), 
                            &ipmcrep_entry)) < 0) {
            goto del_done;
        }
    }

 del_done:
    IPMC_REPL_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *	bcm_tucana_ipmc_repl_delete_all
 * Purpose:
 *	Remove all VLANs from selected ports' replication list for chosen
 *	IPMC group.
 * Parameters:
 *	unit     - StrataSwitch PCI device unit number.
 *	group    - The MC group number.
 *	port     - port from which to remove VLANs.
 *	Returns:
 *     BCM_E_XXX
 */

int
bcm_tucana_ipmc_repl_delete_all(int unit, int group, bcm_port_t port)
{
    int			rv = BCM_E_NONE;
    int                 vlans_per_port = TUCANA_IPMC_PORT_VLANS(unit, port);
    int                 vlan_slot;
    fe_ipmc_vec_entry_t fe_vec_entry;
    ge_ipmc_vec_entry_t ge_vec_entry;
    uint32              bitmap;
    int			blk = SOC_PORT_BLOCK(unit, port);

    REPL_PORT_CHECK(unit, port);

    REPL_GROUP_CHECK(unit, group);

    IPMC_REPL_LOCK(unit);

    /* Now the desired VLAN is in (vlan_slot) index of IPMC_VLAN table */
    if (IS_GE_PORT(unit, port)) {
        sal_memset(&ge_vec_entry, 0, sizeof(ge_vec_entry));
        if ((rv = READ_GE_IPMC_VECm(unit, blk, group, 
                                    &ge_vec_entry)) < 0) {
            goto del_all_done;
        }
        bitmap = soc_GE_IPMC_VECm_field32_get(unit, &ge_vec_entry, 
                                                         BITMAPf);
        sal_memset(&ge_vec_entry, 0, sizeof(ge_vec_entry));
        if ((rv = WRITE_GE_IPMC_VECm(unit, blk, group, &ge_vec_entry)) < 0) {
            goto del_all_done;
        }
    } else {
        sal_memset(&fe_vec_entry, 0, sizeof(fe_vec_entry));
        if ((rv = READ_FE_IPMC_VECm(unit, blk,
                                    TUCANA_IPMC_FE_VEC_INDEX(port, group), 
                                    &fe_vec_entry)) < 0) {
            goto del_all_done;
        }
        bitmap = soc_FE_IPMC_VECm_field32_get(unit, &fe_vec_entry, 
                                              BITMAPf);
        sal_memset(&fe_vec_entry, 0, sizeof(fe_vec_entry));
        if ((rv = WRITE_FE_IPMC_VECm(unit, blk,
                                     TUCANA_IPMC_FE_VEC_INDEX(port, group), 
                                     &fe_vec_entry)) < 0) {
            goto del_all_done;
        }
    }

    /* Clear entry */
    if ((rv = WRITE_MMU_MEMORIES1_IPMCREPm(unit, MEM_BLOCK_ALL, 
                     TUCANA_IPMC_MMU_INDEX(port, group), 
                     soc_mem_entry_null(unit, MMU_MEMORIES1_IPMCREPm))) < 0) {
        goto del_all_done;
    }

    TUCANA_IPMC_REPL_VLAN_COUNT(unit, port, group) = 0;
    for (vlan_slot = 0; vlan_slot < vlans_per_port; vlan_slot++) {
        if (bitmap & (1 << vlan_slot)) {
            TUCANA_IPMC_REPL_VLAN_REFS(unit, port, vlan_slot)--;
        }
    }

 del_all_done:
    IPMC_REPL_UNLOCK(unit);
    return rv;
}

/* Function:
 *      _bcm_tucana_ipmc_repl_vlan_stp_set
 * Purpose:
 *      Update the enable bit for a (port, VLAN) mask.
 */
int
_bcm_tucana_ipmc_repl_vlan_stp_set(int unit, bcm_port_t port,
                                   bcm_vlan_t vid, int enable)
{
    fe_ipmc_vlan_entry_t fe_vlan_entry;
    ge_ipmc_vlan_entry_t ge_vlan_entry;
    int                 vlans_per_port = TUCANA_IPMC_PORT_VLANS(unit, port);
    int			blk = SOC_PORT_BLOCK(unit, port);
    uint32              mem_vlan;
    int                 vlan_slot;
    int                 cur_enable;
    int			rv = BCM_E_NONE;

    if (_tucana_repl_info[unit] == NULL) {
        /* Not yet initialized, do nothing */
        return BCM_E_NONE;
    }

    REPL_PORT_CHECK(unit, port);

    if (_tucana_repl_info[unit][port] == NULL) {
        
        return BCM_E_NONE;
    }

    IPMC_REPL_LOCK(unit);

    cur_enable = TUCANA_IPMC_REPL_VLAN_STG_ENABLE(unit, port, vid);

    if (cur_enable == enable) {
        /* No update, done */
        IPMC_REPL_UNLOCK(unit);
        return BCM_E_NONE;
    }

    /* Update VLAN state data structure */
    if (enable) {
        TUCANA_IPMC_REPL_VLAN_STG_SET(unit, port, vid);
    } else {
        TUCANA_IPMC_REPL_VLAN_STG_CLR(unit, port, vid);
    }

    /* Check if this is an active replication VLAN */
    for (vlan_slot = 0; vlan_slot < vlans_per_port; vlan_slot++) {
        if (TUCANA_IPMC_REPL_VLAN_REFS(unit, port, vlan_slot)) {
            /* A VLAN is in this slot.  Is it the one we need? */
            if (IS_GE_PORT(unit, port)) {
                sal_memset(&ge_vlan_entry, 0, sizeof(ge_vlan_entry));
                if ((rv = READ_GE_IPMC_VLANm(unit, blk, vlan_slot, 
                                             &ge_vlan_entry)) < 0) {
                    break;
                }
                mem_vlan = soc_GE_IPMC_VLANm_field32_get(unit, &ge_vlan_entry, 
                                                         VLANIDf);
            } else {
                sal_memset(&fe_vlan_entry, 0, sizeof(fe_vlan_entry));
                if ((rv = READ_FE_IPMC_VLANm(unit, blk,
                                  TUCANA_IPMC_FE_VLAN_INDEX(port, vlan_slot), 
                                  &fe_vlan_entry)) < 0) {
                    break;
                }
                mem_vlan = soc_FE_IPMC_VLANm_field32_get(unit, &fe_vlan_entry, 
                                                         VLANIDf);
            }

            if (mem_vlan == vid) {
                /* Update MSTP mask table */
                rv = _bcm_tucana_mstp_mask_update(unit, port,
                                                  vlan_slot, enable);
                break;
            }
        }
    }

    IPMC_REPL_UNLOCK(unit);
    return rv;
}

/*
 * Function:
 *	bcm_tucana_ipmc_repl_reload
 * Purpose:
 *      Re-Initialize IPMC replication software to state consistent with
 *      hardware.
 * Parameters:
 *	unit - SOC unit #
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_tucana_ipmc_repl_reload(int unit)
{
    mmu_memories1_ipmcrep_entry_t ipmcrep_entry;
    fe_ipmc_vec_entry_t fe_vec_entry;
    ge_ipmc_vec_entry_t ge_vec_entry;
    int group, ix, blk;
    bcm_port_t port;
    uint32 bitmap;
    int	alloc_size;

    /*
     * Allocate memory for all IPMC replication software structures
     */
    BCM_IF_ERROR_RETURN
        (bcm_tucana_ipmc_repl_detach(unit));

    alloc_size = SOC_MAX_NUM_PORTS * sizeof(_tucana_repl_port_info_t *);
    _tucana_repl_info[unit] = sal_alloc(alloc_size, "IPMC repl ports");
    if (_tucana_repl_info[unit] == NULL) {
        return BCM_E_MEMORY;
    }
    sal_memset(_tucana_repl_info[unit], 0, alloc_size);

    PBMP_ITER(PBMP_GE_ALL(unit), port) {
        alloc_size = sizeof(_tucana_repl_port_info_t);
        TUCANA_IPMC_REPL_INFO(unit, port) =
            sal_alloc(alloc_size, "IPMC repl port info");
        if (TUCANA_IPMC_REPL_INFO(unit, port) == NULL) {
	    bcm_tucana_ipmc_repl_detach(unit);
            return BCM_E_MEMORY;
        }
        sal_memset(TUCANA_IPMC_REPL_INFO(unit, port), 0, alloc_size);

        alloc_size = TUCANA_IPMC_GE_VLANS * sizeof(int32);
        _tucana_repl_info[unit][port]->vlan_refs =
            sal_alloc(alloc_size, "IPMC repl port VLAN references");
        if (_tucana_repl_info[unit][port]->vlan_refs == NULL) {
	    bcm_tucana_ipmc_repl_detach(unit);
            return BCM_E_MEMORY;
        }
        sal_memset(_tucana_repl_info[unit][port]->vlan_refs, 0, alloc_size);
    }
    
    PBMP_ITER(PBMP_FE_ALL(unit), port) {
        alloc_size = sizeof(_tucana_repl_port_info_t);
        TUCANA_IPMC_REPL_INFO(unit, port) =
            sal_alloc(alloc_size, "IPMC repl port info");
        if (TUCANA_IPMC_REPL_INFO(unit, port) == NULL) {
	    bcm_tucana_ipmc_repl_detach(unit);
            return BCM_E_MEMORY;
        }
        sal_memset(TUCANA_IPMC_REPL_INFO(unit, port), 0, alloc_size);

        alloc_size = TUCANA_IPMC_FE_VLANS * sizeof(int32);
        _tucana_repl_info[unit][port]->vlan_refs =
            sal_alloc(alloc_size, "IPMC repl port VLAN references");
        if (_tucana_repl_info[unit][port]->vlan_refs == NULL) {
	    bcm_tucana_ipmc_repl_detach(unit);
            return BCM_E_MEMORY;
        }
        sal_memset(_tucana_repl_info[unit][port]->vlan_refs, 0, alloc_size);
    }
    
    /*
     * Read IPMCrepl, IPMC_VEC, IPMC_VLAN tables to build up software state
     */

    /* Read IPMCrep table to get replication count per (port, group) */
    for (group = 0; group < TUCANA_IPMC_GROUPS; group++) {
        PBMP_ITER(PBMP_E_ALL(unit), port) {
            sal_memset(&ipmcrep_entry, 0, sizeof(ipmcrep_entry));
            SOC_IF_ERROR_RETURN
                 (READ_MMU_MEMORIES1_IPMCREPm(unit, MEM_BLOCK_ANY,
                       TUCANA_IPMC_MMU_INDEX(port, group), &ipmcrep_entry));
            TUCANA_IPMC_REPL_VLAN_COUNT(unit, port, group) =
                        soc_MMU_MEMORIES1_IPMCREPm_field32_get(unit,
                                           &ipmcrep_entry, IPMCREPf);
        }
    }

    /* Read IPMC_VEC table to get the reference count for (port, group, vlan) */
    for (group = 0; group < TUCANA_IPMC_GROUPS; group++) {
        PBMP_ITER(PBMP_E_ALL(unit), port) {
            blk = SOC_PORT_BLOCK(unit, port);

            if (IS_GE_PORT(unit, port)) {
                sal_memset(&ge_vec_entry, 0, sizeof(ge_vec_entry));
                SOC_IF_ERROR_RETURN
                    (READ_GE_IPMC_VECm(unit, blk, group, &ge_vec_entry));
                soc_GE_IPMC_VECm_field_get(unit, &ge_vec_entry,
                                           BITMAPf, &bitmap);
                for (ix = 0; ix < TUCANA_IPMC_GE_VLANS; ix++) {
                    if (bitmap & (1 << ix)) {
    /*    coverity[overrun-local : FALSE]    */
                        TUCANA_IPMC_REPL_VLAN_REFS(unit, port, ix)++;
                    }
                }
            } else {
                sal_memset(&fe_vec_entry, 0, sizeof(fe_vec_entry));
                SOC_IF_ERROR_RETURN
                    (READ_FE_IPMC_VECm(unit, blk, group, &fe_vec_entry));
                soc_FE_IPMC_VECm_field_get(unit, &fe_vec_entry,
                                           BITMAPf, &bitmap);
                for (ix = 0; ix < TUCANA_IPMC_FE_VLANS; ix++) {
                    if (bitmap & (1 << ix)) {
                        TUCANA_IPMC_REPL_VLAN_REFS(unit, port, ix)++;
                    }
                }
            }
        }
    }

    /* Access the STG state to fill the VLAN state vector */
    if (soc_feature(unit, soc_feature_mstp_mask)) {
        BCM_IF_ERROR_RETURN
            (_bcm_tucana_mstp_state_init(unit));
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_tucana_ipmc_egress_intf_add
 * Purpose:
 *	Add VLAN to selected ports' replication list for chosen
 *	IPMC group.
 * Parameters:
 *	unit     - StrataSwitch PCI device unit number.
 *	group    - The IPMC group ID
 *	port     - port to add.
 *	l3_intf  - L3 interface containing replicated VLAN to add to port.
 * Returns:
 *	BCM_E_XXX
 */
int
bcm_tucana_ipmc_egress_intf_add(int unit, int group, bcm_port_t port, 
                                bcm_l3_intf_t *l3_intf)
{
    int rv = BCM_E_NONE;

    rv = bcm_tucana_ipmc_repl_add(unit, group, port, l3_intf->l3a_vid);
    return rv;
}

/*
 * Function:
 *	bcm_tucana_ipmc_egress_intf_delete
 * Purpose:
 *	Remove VLAN from selected ports' replication list for chosen
 *	IPMC group.
 * Parameters:
 *	unit     - StrataSwitch PCI device unit number.
 *	group    - The group number.
 *	port     - port to remove.
 *	l3_intf  - L3 interface containing replicated VLAN to delete from port.
 * Returns:
 *	BCM_E_XXX
 */
int
bcm_tucana_ipmc_egress_intf_delete(int unit, int group, bcm_port_t port, 
                                   bcm_l3_intf_t *l3_intf)
{
    int rv = BCM_E_NONE;

    rv = bcm_tucana_ipmc_repl_delete(unit, group, port, l3_intf->l3a_vid);
    return rv;
}

#endif	/* INCLUDE_L3 */

int _bcm_esw_tucana_ipmc_not_empty;
