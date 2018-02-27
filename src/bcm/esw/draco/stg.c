/*
 * $Id: stg.c 1.15.178.1 Broadcom SDK $
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
 * STG support
 *
 * These functions set or get port related fields in the Spanning tree
 * table.
 */

#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/ptable.h>         /* PVP_* defines */

#include <bcm/error.h>
#include <bcm/stg.h>

#include <bcm_int/esw/stg.h>
#include <bcm_int/esw/draco.h>

#include <bcm_int/esw_dispatch.h>

#define STG_BITS_PER_PORT       2
#define STG_PORT_MASK           ((1 << STG_BITS_PER_PORT)-1)
#define STG_PORTS_PER_WORD      (32 / STG_BITS_PER_PORT)
#define STG_WORD(port)          ((port) / STG_PORTS_PER_WORD)
#define STG_BITS_SHIFT(port)    \
        (STG_BITS_PER_PORT * ((port) % STG_PORTS_PER_WORD))
#define STG_BITS_MASK(port)     (STG_PORT_MASK << (STG_BITS_SHIFT(port)))

/*
 * Function:
 *      bcm_draco_stg_stp_init
 * Purpose:
 *      Write an entry with the spanning tree state DISABLE for all ports.
 */

int
bcm_draco_stg_stp_init(int unit, bcm_stg_t stg)
{
    stg_tab_entry_t     entry;
    bcm_port_t          port;
    bcm_pbmp_t          stacked;
    int                 pvp;

    /* all ports PVP_STP_DISABLED */
    sal_memset(&entry, 0, sizeof(entry));

    /* put stacking ports into forwarding */
    BCM_PBMP_ASSIGN(stacked, PBMP_HG_ALL(unit));
    BCM_PBMP_OR(stacked, SOC_PBMP_STACK_CURRENT(unit));
    pvp = PVP_STP_FORWARDING;
    PBMP_ITER(stacked, port) {
        if (SOC_PORT_VALID(unit, port))
        {
    /*    coverity[overrun-local]    */
            entry.entry_data[STG_WORD(port)] |= pvp << STG_BITS_SHIFT(port);
        }
    }

    BCM_IF_ERROR_RETURN(WRITE_STG_TABm(unit, MEM_BLOCK_ALL, stg, &entry));

    if (soc_feature(unit, soc_feature_mstp_lookup)) {
        /*
         * For 5695_b+, ST_PORT_TBLm[stg] condenses the STG_TABm
         * information into a single forwarding/off bit
         * for each local port.
         * Disable all local ethernet ports.
         */
        mmu_memories1_st_port_tbl_entry_t spt_entry;
        sal_memset(&spt_entry, 0 , sizeof(spt_entry));
        BCM_IF_ERROR_RETURN(WRITE_MMU_MEMORIES1_ST_PORT_TBLm(unit,
                                                             MEM_BLOCK_ALL,
                                                             stg,
                                                             &spt_entry));
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_draco_stg_stp_set
 * Purpose:
 *      Set the spanning tree state for a port in specified STG.
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      stg - STG ID.
 *      port - StrataSwitch port number.
 *      stp_state - State to place port in.
 */

int
bcm_draco_stg_stp_set(int unit, bcm_stg_t stg, bcm_port_t port, int stp_state)
{
    stg_tab_entry_t     entry;
    int                 rv;
    int                 pvp;

    if (!SOC_PORT_VALID(unit, port) || !IS_PORT(unit, port)) {
        return BCM_E_PORT;
    }

    if (soc_feature(unit, soc_feature_mstp_uipmc)) {
        bcm_pbmp_t   pbmp;
        bcm_vlan_t  *list;
        int          count;
        int          i;
        int          add;
        int          rv;

        BCM_PBMP_PORT_SET(pbmp, port);
        add = (stp_state == BCM_STG_STP_FORWARD);

        BCM_IF_ERROR_RETURN(bcm_esw_stg_vlan_list(unit, stg, &list, &count));

        for (i = 0; i < count; i++) {
            rv = bcm_draco_vlan_stg_update(unit, list[i], pbmp, add);
            if (!BCM_SUCCESS(rv)) {
                bcm_esw_stg_vlan_list_destroy(unit, list, count) ;
                return rv;
            }
        }

        BCM_IF_ERROR_RETURN(bcm_esw_stg_vlan_list_destroy(unit, list, count));
    }

    BCM_IF_ERROR_RETURN(_bcm_stg_stp_translate(unit, stp_state, &pvp));

    soc_mem_lock(unit, STG_TABm);

    rv = READ_STG_TABm(unit, MEM_BLOCK_ANY, stg, &entry);

    if (BCM_SUCCESS(rv)) {
        /* coverity[overrun-local] : FALSE */
        entry.entry_data[STG_WORD(port)] &= ~(STG_BITS_MASK(port));
        entry.entry_data[STG_WORD(port)] |= (pvp << STG_BITS_SHIFT(port));

        rv = WRITE_STG_TABm(unit, MEM_BLOCK_ALL, stg, &entry);
    }

    soc_mem_unlock(unit, STG_TABm);

    if (BCM_SUCCESS(rv) &&
        soc_feature(unit, soc_feature_mstp_lookup) &&
        IS_E_PORT(unit, port)) {
        /*
         * For 5695_b+, ST_PORT_TBLm condenses the STG_TABm
         * information into a single forwarding/off bit
         * for each local port.
         * Configure bit[port] as indicated by stp_state.
         */
        mmu_memories1_st_port_tbl_entry_t spt_entry;
        uint32 spt_bits;

        soc_mem_lock(unit, MMU_MEMORIES1_ST_PORT_TBLm);

        rv = READ_MMU_MEMORIES1_ST_PORT_TBLm(unit, MEM_BLOCK_ALL,
                                             stg, &spt_entry);
        if (BCM_SUCCESS(rv)) {
            spt_bits =
                soc_MMU_MEMORIES1_ST_PORT_TBLm_field32_get(unit, &spt_entry,
                                                           ST_PORT_TBLf);
            if (stp_state == BCM_STG_STP_FORWARD) {
                spt_bits |= (1<<port);
            } else {
                spt_bits &= ~(1<<port);
            }
            soc_MMU_MEMORIES1_ST_PORT_TBLm_field32_set(unit, &spt_entry,
                                                       ST_PORT_TBLf,
                                                       spt_bits);
            rv = WRITE_MMU_MEMORIES1_ST_PORT_TBLm(unit, MEM_BLOCK_ALL,
                                                  stg, &spt_entry);
        }

        soc_mem_unlock(unit, MMU_MEMORIES1_ST_PORT_TBLm);
    }

    return rv;
}

/*
 * Function:
 *      bcm_draco_stg_stp_get
 * Purpose:
 *      Retrieve the spanning tree state for a port in specified STG.
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      stg - STG ID.
 *      port - StrataSwitch port number.
 *      stp_state - Pointer where state stored.
 */

int
bcm_draco_stg_stp_get(int unit, bcm_stg_t stg, bcm_port_t port, int *stp_state)
{
    stg_tab_entry_t     entry;
    int                 pvp;

    if (!SOC_PORT_VALID(unit, port) || !IS_PORT(unit, port)) {
        return BCM_E_PORT;
    }

    SOC_IF_ERROR_RETURN(READ_STG_TABm(unit, MEM_BLOCK_ANY, stg, &entry));
    /*    coverity[overrun-local]    */

    /* coverity[overrun-local] */
    pvp = entry.entry_data[STG_WORD(port)] >> STG_BITS_SHIFT(port);
    pvp &= STG_PORT_MASK;

    BCM_IF_ERROR_RETURN(_bcm_stg_pvp_translate(unit, pvp, stp_state));

    return BCM_E_NONE;
}
