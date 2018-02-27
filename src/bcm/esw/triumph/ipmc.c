/*
 * $Id: ipmc.c 1.47.2.9 Broadcom SDK $
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
 * File:        ipmc.c
 * Purpose:     Tracks and manages IPMC tables.
 */

#ifdef INCLUDE_L3

#include <soc/l3x.h>
#if defined(BCM_BRADLEY_SUPPORT)
#include <soc/bradley.h>
#endif /* BCM_BRADLEY_SUPPORT */

#include <bcm/error.h>
#include <bcm/ipmc.h>

#include <bcm_int/esw/ipmc.h>
#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/firebolt.h>
#include <bcm_int/esw/trx.h>
#include <bcm_int/esw/stack.h>
#include <bcm_int/esw/bradley.h>
#include <bcm_int/esw/triumph.h>
#include <bcm_int/common/multicast.h>
#include <bcm_int/esw/multicast.h>

#ifdef BCM_TRIUMPH2_SUPPORT
#include <bcm_int/esw/triumph2.h>
#endif /* BCM_TRIUMPH2_SUPPORT */

#include <bcm_int/esw_dispatch.h>


#define TR_IPMC_NO_SRC_CHECK_PORT(unit) ((SOC_IS_TRIDENT(unit) || \
                                          SOC_IS_KATANA(unit)) ? 0x7f : \
                                          ((SOC_IS_ENDURO(unit) || \
                                           SOC_IS_HURRICANE(unit)) ? 0x1f : 0x3f))

/*
 * Function:
 *      _bcm_tr_ipmc_l3entry_list_add
 * Purpose:
 *      Add a L3 entry to IPMC group's linked list of L3 entries
 * Parameters:
 *      unit       - (IN) BCM device number.
 *      ipmc_index - (IN) IPMC group ID.
 *      _bcm_l3_cfg_t - (IN) l3 config structure
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_tr_ipmc_l3entry_list_add(int unit, int ipmc_index,
                               _bcm_l3_cfg_t l3cfg)
{
    _bcm_esw_ipmc_l3entry_t *ipmc_l3entry;
   
    ipmc_l3entry = sal_alloc(sizeof(_bcm_esw_ipmc_l3entry_t), "IPMC L3 entry");
    if (ipmc_l3entry == NULL) {
        return BCM_E_MEMORY;
    }
    ipmc_l3entry->ip6 = (l3cfg.l3c_flags & BCM_L3_IP6) ? 1 : 0;
    ipmc_l3entry->l3info.flags = l3cfg.l3c_flags;
    ipmc_l3entry->l3info.vrf = l3cfg.l3c_vrf;
    ipmc_l3entry->l3info.ip_addr = l3cfg.l3c_ipmc_group;
    ipmc_l3entry->l3info.src_ip_addr = l3cfg.l3c_src_ip_addr;
    sal_memcpy(ipmc_l3entry->l3info.ip6, l3cfg.l3c_ip6, BCM_IP6_ADDRLEN);
    sal_memcpy(ipmc_l3entry->l3info.sip6, l3cfg.l3c_sip6, BCM_IP6_ADDRLEN);
    ipmc_l3entry->l3info.vid = l3cfg.l3c_vid;
    ipmc_l3entry->l3info.prio = l3cfg.l3c_prio;
    ipmc_l3entry->l3info.ipmc_ptr = l3cfg.l3c_ipmc_ptr;
    ipmc_l3entry->l3info.lookup_class = l3cfg.l3c_lookup_class;
    ipmc_l3entry->next = IPMC_GROUP_INFO(unit, ipmc_index)->l3entry_list;
    IPMC_GROUP_INFO(unit, ipmc_index)->l3entry_list = ipmc_l3entry;

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_tr_ipmc_l3entry_list_del
 * Purpose:
 *      Delete a L3 entry from IPMC group's linked list of L3 entries
 * Parameters:
 *      unit       - (IN) BCM device number.
 *      ipmc_index - (IN) IPMC group ID.
 *      _bcm_l3_cfg_t - (IN) l3 config structure
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_tr_ipmc_l3entry_list_del(int unit, int ipmc_index,
                              _bcm_l3_cfg_t l3cfg)
{
    _bcm_esw_ipmc_l3entry_t *ipmc_l3entry;
    _bcm_esw_ipmc_l3entry_t *prev_ipmc_l3entry = NULL;
   
    ipmc_l3entry = IPMC_GROUP_INFO(unit, ipmc_index)->l3entry_list;
    while (ipmc_l3entry != NULL) {
        if ((ipmc_l3entry->l3info.ip_addr == l3cfg.l3c_ip_addr) &&
            (ipmc_l3entry->l3info.src_ip_addr == l3cfg.l3c_src_ip_addr) && 
            (!sal_memcmp(ipmc_l3entry->l3info.ip6, l3cfg.l3c_ip6, BCM_IP6_ADDRLEN)) &&
            (!sal_memcmp(ipmc_l3entry->l3info.sip6, l3cfg.l3c_sip6, BCM_IP6_ADDRLEN)) &&
            (ipmc_l3entry->l3info.vid == l3cfg.l3c_vid)) {
           if (ipmc_l3entry == IPMC_GROUP_INFO(unit, ipmc_index)->l3entry_list) {
               IPMC_GROUP_INFO(unit, ipmc_index)->l3entry_list = ipmc_l3entry->next;
           } else {
               /* 
                * In the following line of code, Coverity thinks the
                * prev_ipmc_l3entry pointer may still be NULL when 
                * dereferenced. This situation will never occur because 
                * if ipmc_l3entry is not pointing to the head of the 
                * linked list, prev_ipmc_l3entry would not be NULL.
                */
               /* coverity[var_deref_op : FALSE] */
               prev_ipmc_l3entry->next = ipmc_l3entry->next;
           }
           sal_free(ipmc_l3entry);
           break;
        }
        prev_ipmc_l3entry = ipmc_l3entry;
        ipmc_l3entry = ipmc_l3entry->next;
    }

    if (ipmc_l3entry == NULL) {
        return BCM_E_NOT_FOUND;
    }

    return BCM_E_NONE;
}

STATIC int
_bcm_tr_ipmc_l3entry_list_update(int unit, int ipmc_index,
                                 _bcm_l3_cfg_t l3cfg)
{
    _bcm_esw_ipmc_l3entry_t *ipmc_l3entry;
   
    ipmc_l3entry = IPMC_GROUP_INFO(unit, ipmc_index)->l3entry_list;
    while (ipmc_l3entry != NULL) {
        if ((ipmc_l3entry->l3info.ip_addr == l3cfg.l3c_ip_addr) &&
            (ipmc_l3entry->l3info.src_ip_addr == l3cfg.l3c_src_ip_addr) && 
            (!sal_memcmp(ipmc_l3entry->l3info.ip6, l3cfg.l3c_ip6, BCM_IP6_ADDRLEN)) &&
            (!sal_memcmp(ipmc_l3entry->l3info.sip6, l3cfg.l3c_sip6, BCM_IP6_ADDRLEN)) &&
            (ipmc_l3entry->l3info.vid == l3cfg.l3c_vid)) {
            /* set new values */
            ipmc_l3entry->l3info.flags = l3cfg.l3c_flags;
            ipmc_l3entry->l3info.vrf = l3cfg.l3c_vrf;
            ipmc_l3entry->l3info.prio = l3cfg.l3c_prio;
            ipmc_l3entry->l3info.ipmc_ptr = l3cfg.l3c_ipmc_ptr;
            ipmc_l3entry->l3info.lookup_class = l3cfg.l3c_lookup_class;
            break;
        }
        ipmc_l3entry = ipmc_l3entry->next;
    }

    if (ipmc_l3entry == NULL) {
        return BCM_E_NOT_FOUND;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_tr_ipmc_l3entry_list_size_get
 * Purpose:
 *      Get the number of L3 entries in IPMC group's linked list.
 * Parameters:
 *      unit       - (IN) BCM device number.
 *      ipmc_index - (IN) IPMC group ID.
 *      size       - (OUT) Number of L3 entries.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_tr_ipmc_l3entry_list_size_get(int unit, int ipmc_index,
        int *size)
{
    _bcm_esw_ipmc_l3entry_t *ipmc_l3entry;

    *size = 0;
    ipmc_l3entry = IPMC_GROUP_INFO(unit, ipmc_index)->l3entry_list;
    while (ipmc_l3entry != NULL) {
        (*size)++;
        ipmc_l3entry = ipmc_l3entry->next;
    }

    return BCM_E_NONE;
}

#ifdef BCM_TRIUMPH2_SUPPORT
/*
 * Function:
 *      _tr2_ipmc_glp_get
 * Purpose:
 *      Fill source information to bcm_ipmc_addr_t struct.
 */
STATIC int
_tr2_ipmc_glp_get(int unit, bcm_ipmc_addr_t *ipmc, ipmc_1_entry_t *entry)

{
    int                 mod, port_tgid, is_trunk, rv = BCM_E_NONE;
    int                 no_src_check = FALSE;

    is_trunk = soc_L3_IPMC_1m_field32_get(unit, entry, Tf);
    mod = soc_L3_IPMC_1m_field32_get(unit, entry, MODULE_IDf);
    port_tgid = soc_L3_IPMC_1m_field32_get(unit, entry, PORT_NUMf);
    if (is_trunk) {
        if ((port_tgid == TR_IPMC_NO_SRC_CHECK_PORT(unit)) && (mod == SOC_MODID_MAX(unit))) {
            no_src_check = TRUE;
        } else {
            mod = 0;
            port_tgid = soc_L3_IPMC_1m_field32_get(unit, entry, TGIDf);
        }
    }
    if (no_src_check) {
        ipmc->ts = 0;
        ipmc->mod_id = -1;
        ipmc->port_tgid = -1;
        ipmc->flags |= BCM_IPMC_SOURCE_PORT_NOCHECK;
    } else if (is_trunk) {
        ipmc->ts = 1;
        ipmc->mod_id = 0;
        ipmc->port_tgid = port_tgid;
    } else {
        bcm_module_t    mod_in, mod_out;
        bcm_port_t      port_in, port_out;

        mod_in = mod;
        port_in = port_tgid;
        BCM_IF_ERROR_RETURN
            (_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_GET,
                                    mod_in, port_in,
                                    &mod_out, &port_out));
        ipmc->ts = 0;
        ipmc->mod_id = mod_out;
        ipmc->port_tgid = port_out;
    }
    return rv;
}
#endif

/*
 * Function:
 *      _tr_ipmc_info_get
 * Purpose:
 *      Fill information in bcm_ipmc_addr_t struct.
 */

STATIC int
_tr_ipmc_info_get(int unit, int ipmc_index, bcm_ipmc_addr_t *ipmc, 
                  ipmc_entry_t *entry, uint8 do_l3_lkup, 
                  _bcm_esw_ipmc_l3entry_t *use_ipmc_l3entry)
{
    int           mod = -1, port_tgid = -1, is_trunk = 0;
    int           untag, port, no_src_check = FALSE;
    uint32        cfg2;

    SOC_PBMP_CLEAR(ipmc->l3_pbmp);
    SOC_PBMP_CLEAR(ipmc->l2_pbmp);

    soc_mem_pbmp_field_get(unit, L3_IPMCm, entry, L3_BITMAPf, &ipmc->l3_pbmp);
    soc_mem_pbmp_field_get(unit, L3_IPMCm, entry, L2_BITMAPf, &ipmc->l2_pbmp);

    ipmc->v = soc_L3_IPMCm_field32_get(unit, entry, VALIDf);

    if (soc_mem_field_valid(unit, L3_IPMCm, PORT_NUMf)) {
        is_trunk = soc_L3_IPMCm_field32_get(unit, entry, Tf);
        mod = soc_L3_IPMCm_field32_get(unit, entry, MODULE_IDf);
        port_tgid = soc_L3_IPMCm_field32_get(unit, entry, PORT_NUMf);
        if (is_trunk) {
            if ((port_tgid == TR_IPMC_NO_SRC_CHECK_PORT(unit)) && (mod == SOC_MODID_MAX(unit))) {
                no_src_check = TRUE;
            } else {
                mod = 0;
                port_tgid = soc_L3_IPMCm_field32_get(unit, entry, TGIDf);
            }
        }
        if (no_src_check) {
            ipmc->ts = 0;
            ipmc->mod_id = -1;
            ipmc->port_tgid = -1;
            ipmc->flags |= BCM_IPMC_SOURCE_PORT_NOCHECK;
        } else if (is_trunk) {
            ipmc->ts = 1;
            ipmc->mod_id = 0;
            ipmc->port_tgid = port_tgid;
        } else {
            bcm_module_t    mod_in, mod_out;
            bcm_port_t      port_in, port_out;

            mod_in = mod;
            port_in = port_tgid;
            BCM_IF_ERROR_RETURN
                (_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_GET,
                                        mod_in, port_in,
                                        &mod_out, &port_out));
            ipmc->ts = 0;
            ipmc->mod_id = mod_out;
            ipmc->port_tgid = port_out;
        }
    }

    if ((ipmc->v) && (do_l3_lkup)) {
        _bcm_l3_cfg_t l3cfg;
        _bcm_esw_ipmc_l3entry_t *ipmc_l3entry;

        if (use_ipmc_l3entry) {
            /* use the passed in l3 info */
            ipmc_l3entry = use_ipmc_l3entry;
        } else {
            /* Note: this simply picks up the first l3 info */
            ipmc_l3entry = IPMC_GROUP_INFO(unit, ipmc_index)->l3entry_list;
            if (NULL == ipmc_l3entry) {
                /* No entries in Multicast host table */
                return BCM_E_EMPTY;
            }
        }
        sal_memset(&l3cfg, 0, sizeof(_bcm_l3_cfg_t));
        l3cfg.l3c_flags = BCM_L3_IPMC;
        l3cfg.l3c_vrf = ipmc_l3entry->l3info.vrf;
        l3cfg.l3c_vid = ipmc_l3entry->l3info.vid;

        if (ipmc_l3entry->ip6) {
            ipmc->flags |= BCM_IPMC_IP6;
        } else {
            ipmc->flags &= ~BCM_IPMC_IP6;
        }
        if (ipmc->flags & BCM_IPMC_HIT_CLEAR) {
            l3cfg.l3c_flags |= BCM_L3_HIT_CLEAR;
        }
        
        /* need to get current l3 info from h/w like hit bits */
        if (ipmc->flags & BCM_IPMC_IP6) {
            sal_memcpy(ipmc->s_ip6_addr, &ipmc_l3entry->l3info.sip6, BCM_IP6_ADDRLEN);
            sal_memcpy(ipmc->mc_ip6_addr, &ipmc_l3entry->l3info.ip6, BCM_IP6_ADDRLEN);
            sal_memcpy(l3cfg.l3c_sip6, &ipmc_l3entry->l3info.sip6, BCM_IP6_ADDRLEN);
            sal_memcpy(l3cfg.l3c_ip6, &ipmc_l3entry->l3info.ip6, BCM_IP6_ADDRLEN);
            l3cfg.l3c_flags |= BCM_L3_IP6;
            BCM_IF_ERROR_RETURN(mbcm_driver[unit]->mbcm_l3_ip6_get(unit, &l3cfg));
        } else {
            ipmc->s_ip_addr = ipmc_l3entry->l3info.src_ip_addr;
            ipmc->mc_ip_addr = ipmc_l3entry->l3info.ipmc_group;
            l3cfg.l3c_src_ip_addr = ipmc_l3entry->l3info.src_ip_addr;
            l3cfg.l3c_ipmc_group = ipmc_l3entry->l3info.ipmc_group;
            BCM_IF_ERROR_RETURN(mbcm_driver[unit]->mbcm_l3_ip4_get(unit, &l3cfg));
        }
        
        if (l3cfg.l3c_flags & BCM_L3_HIT) {
            ipmc->flags |= BCM_IPMC_HIT;
        }

        if (ipmc_l3entry->l3info.flags & BCM_L3_RPE) {
            ipmc->cos = ipmc_l3entry->l3info.prio;
            ipmc->flags |= BCM_IPMC_SETPRI;
        } else {
            ipmc->cos = -1;
            ipmc->flags &= ~BCM_IPMC_SETPRI;
        }
        ipmc->ipmc_index = ipmc_index;
        ipmc->lookup_class = ipmc_l3entry->l3info.lookup_class;
        ipmc->vrf = ipmc_l3entry->l3info.vrf;
        ipmc->vid = ipmc_l3entry->l3info.vid;
    }

    /* Calculate untagged bitmap */
    BCM_PBMP_CLEAR(ipmc->l2_ubmp);
    SOC_PBMP_ITER(ipmc->l2_pbmp, port) {
        if (!IS_E_PORT(unit, port)) { /* CMIC, HG never untagged */
            continue;
        }

        cfg2 = 0;
        SOC_IF_ERROR_RETURN(READ_EGR_IPMC_CFG2r(unit, port, &cfg2));
        untag = soc_reg_field_get(unit, EGR_IPMC_CFG2r, cfg2, UNTAGf);

        if (untag) {
            SOC_PBMP_PORT_ADD(ipmc->l2_ubmp, port);
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _tr_ipmc_write
 * Purpose:
 *      Write an ipmc entry from bcm_ipmc_addr_t struct.
 */

STATIC int
_tr_ipmc_write(int unit, int ipmc_id, bcm_ipmc_addr_t *ipmc)
{
    int                 rv;
    ipmc_entry_t        entry;
    int                 mod, port_tgid, is_trunk, no_src_check = FALSE;
#if defined(BCM_TRIUMPH2_SUPPORT)
    ipmc_1_entry_t      entry_1;
    sal_memset(&entry_1, 0, sizeof(ipmc_1_entry_t));
#endif

    BCM_IF_ERROR_RETURN
        (soc_mem_read(unit, L3_IPMCm, MEM_BLOCK_ANY, ipmc_id, &entry));

    soc_L3_IPMCm_field32_set(unit, &entry, VALIDf, ipmc->v);

    soc_mem_pbmp_field_set(unit, L3_IPMCm, &entry, L3_BITMAPf, &ipmc->l3_pbmp);
    soc_mem_pbmp_field_set(unit, L3_IPMCm, &entry, L2_BITMAPf, &ipmc->l2_pbmp);

#ifdef BCM_KATANA_SUPPORT
    if (soc_mem_field_valid(unit,L3_IPMCm,MMU_MC_REDIRECTION_PTRf)) {
        soc_mem_field32_set(unit, L3_IPMCm, &entry, 
                            MMU_MC_REDIRECTION_PTRf, ipmc_id);
    }
#endif

#ifdef BCM_BRADLEY_SUPPORT
    if (SOC_REG_FIELD_VALID(unit, MC_CONTROL_5r, SHARED_TABLE_IPMC_SIZEf)) {
        BCM_IF_ERROR_RETURN
            (_bcm_xgs3_ipmc_bitmap_set(unit, ipmc_id, ipmc->l2_pbmp));
    }
#endif /* BCM_BRADLEY_SUPPORT */

    if ((ipmc->flags & BCM_IPMC_SOURCE_PORT_NOCHECK) ||
        (ipmc->port_tgid < 0)) {                        /* no source port */
        no_src_check = TRUE;
        is_trunk = 0;
        mod = SOC_MODID_MAX(unit);
        port_tgid = TR_IPMC_NO_SRC_CHECK_PORT(unit);
    } else if (ipmc->ts) {                              /* trunk source port */
        is_trunk = 1;
        mod = 0;
        port_tgid = ipmc->port_tgid;
    } else {                                            /* source port */
        bcm_module_t    mod_in, mod_out;
        bcm_port_t      port_in, port_out;

        mod_in = ipmc->mod_id;
        port_in = ipmc->port_tgid;
        BCM_IF_ERROR_RETURN
            (_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_SET,
                                    mod_in, port_in,
                                    &mod_out, &port_out));
        /* Check parameters, since above is an application callback */
        if (!SOC_MODID_ADDRESSABLE(unit, mod_out)) {
            return BCM_E_BADID;
        }
        if (!SOC_PORT_ADDRESSABLE(unit, port_out)) {
            return BCM_E_PORT;
        }
        is_trunk = 0;
        mod = mod_out;
        port_tgid = port_out;
    }

    if (is_trunk) {
#if defined(BCM_TRIUMPH2_SUPPORT)
        if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) ||
                SOC_IS_VALKYRIE2(unit) || SOC_IS_TRIDENT(unit) ||
                SOC_IS_KATANA(unit)) {
            soc_L3_IPMC_1m_field32_set(unit, &entry_1, Tf, 1);
            soc_L3_IPMC_1m_field32_set(unit, &entry_1, TGIDf, port_tgid);
        } else
#endif
        {
            soc_L3_IPMCm_field32_set(unit, &entry, Tf, 1);
            soc_L3_IPMCm_field32_set(unit, &entry, TGIDf, port_tgid);
        }
    } else {
#if defined(BCM_TRIUMPH2_SUPPORT)
        if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) ||
                SOC_IS_VALKYRIE2(unit) || SOC_IS_TRIDENT(unit) ||
                SOC_IS_KATANA(unit)) {
            soc_L3_IPMC_1m_field32_set(unit, &entry_1, MODULE_IDf, mod);
            soc_L3_IPMC_1m_field32_set(unit, &entry_1, PORT_NUMf, port_tgid);
            if (no_src_check) {
                soc_L3_IPMC_1m_field32_set(unit, &entry_1, Tf, 1);
            }
        } else
#endif
        {
            soc_L3_IPMCm_field32_set(unit, &entry, MODULE_IDf, mod);
            soc_L3_IPMCm_field32_set(unit, &entry, PORT_NUMf, port_tgid);
            if (no_src_check) {
                soc_L3_IPMCm_field32_set(unit, &entry, Tf, 1);
            }
        }
    }
    rv = soc_mem_write(unit, L3_IPMCm, MEM_BLOCK_ALL, ipmc_id, &entry);
#if defined(BCM_TRIUMPH2_SUPPORT)
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) ||
            SOC_IS_VALKYRIE2(unit) || SOC_IS_TRIDENT(unit) ||
            SOC_IS_KATANA(unit)) {
        rv = soc_mem_write(unit, L3_IPMC_1m, MEM_BLOCK_ALL, ipmc_id, &entry_1);
    }
#endif
    return (rv);
}

/*
 * Function:
 *      _tr_ipmc_enable
 * Purpose:
 *      Enable/disable IPMC support.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      enable - TRUE: enable IPMC support.
 *               FALSE: disable IPMC support.
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_tr_ipmc_enable(int unit, int enable)
{
    int                 port, do_vlan;

    enable = enable ? 1 : 0;
    do_vlan = soc_property_get(unit, spn_IPMC_DO_VLAN, 1);

    PBMP_PORT_ITER(unit, port) {
        BCM_IF_ERROR_RETURN
            (_bcm_esw_port_config_set(unit, port,
                                      _bcmPortIpmcV4Enable, enable));
        BCM_IF_ERROR_RETURN
            (_bcm_esw_port_config_set(unit, port,
                                      _bcmPortIpmcV6Enable, enable));
        BCM_IF_ERROR_RETURN
            (_bcm_esw_port_config_set(unit, port,
                                      _bcmPortIpmcVlanKey,
                                      (enable && do_vlan) ? 1 : 0));
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_tr_ipmc_init
 * Purpose:
 *      Initialize the IPMC module and enable IPMC support.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      This function has to be called before any other IPMC functions.
 */

int
bcm_tr_ipmc_init(int unit)
{
    egr_ipmc_entry_t egr_entry;
#if defined(BCM_TRIUMPH2_SUPPORT)
    ipmc_remap_entry_t remap_entry;
#endif
    _bcm_esw_ipmc_t  *info = IPMC_INFO(unit);
    int i, rv = BCM_E_NONE;

    bcm_tr_ipmc_delete_all(unit);
    BCM_IF_ERROR_RETURN(bcm_tr_ipmc_detach(unit));
    BCM_IF_ERROR_RETURN(_tr_ipmc_enable(unit, TRUE));

    IPMC_GROUP_NUM(unit) = soc_mem_index_count(unit, L3_IPMCm);

#ifdef BCM_BRADLEY_SUPPORT
    if (SOC_REG_FIELD_VALID(unit, MC_CONTROL_5r, SHARED_TABLE_IPMC_SIZEf)) {
        int ipmc_base, ipmc_size;
        
        SOC_IF_ERROR_RETURN
            (soc_hbx_ipmc_size_get(unit, &ipmc_base, &ipmc_size));

        if (IPMC_GROUP_NUM(unit) > ipmc_size) {
            /* Reduce to fix allocated table space */
            IPMC_GROUP_NUM(unit) = ipmc_size;
        }
    }
#endif

    info->ipmc_count = 0;

    info->ipmc_group_info =
        sal_alloc(IPMC_GROUP_NUM(unit) * sizeof(_bcm_esw_ipmc_group_info_t),
                  "IPMC group info");
    if (info->ipmc_group_info == NULL) {
        return (BCM_E_MEMORY);
    }
    sal_memset(info->ipmc_group_info, 0, 
               IPMC_GROUP_NUM(unit) * sizeof(_bcm_esw_ipmc_group_info_t));

    /* Initialize EGR_IPMC entries to have L3_PAYLOAD == 1 */
    sal_memset(&egr_entry, 0, sizeof(egr_entry));
    if (soc_mem_field_valid(unit, EGR_IPMCm, L3_PAYLOADf)) {
        soc_EGR_IPMCm_field32_set(unit, &egr_entry, L3_PAYLOADf, 0);
    }
    if (soc_mem_field_valid(unit, EGR_IPMCm, REPLICATION_TYPEf)) {
        soc_EGR_IPMCm_field32_set(unit, &egr_entry, REPLICATION_TYPEf, 0);
    }
    if (soc_mem_field_valid(unit, EGR_IPMCm, DONT_PRUNE_VLANf)) {
        soc_EGR_IPMCm_field32_set(unit, &egr_entry, DONT_PRUNE_VLANf, 0);
    }

    for (i = 0; i < IPMC_GROUP_NUM(unit); i++) {
#if defined(BCM_TRIUMPH2_SUPPORT)
        if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) ||
                SOC_IS_VALKYRIE2(unit) || SOC_IS_TRIDENT(unit) ||
                SOC_IS_KATANA(unit)) {
            /* Initialize identity mapping */
            sal_memset(&remap_entry, 0, sizeof(remap_entry));
            soc_L3_IPMC_REMAPm_field32_set(unit, &remap_entry, L3MC_INDEXf, i);
            rv = WRITE_L3_IPMC_REMAPm(unit, MEM_BLOCK_ALL, i, &remap_entry);
        }
#endif
        if(!SOC_IS_HURRICANE(unit)) {
            rv = WRITE_EGR_IPMCm(unit, MEM_BLOCK_ALL, i, &egr_entry);
            if (rv < 0) {
                sal_free(info->ipmc_group_info);
                info->ipmc_group_info = NULL;
                return rv;
            }
        }
    }
    info->ipmc_initialized = TRUE;
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_tr_ipmc_detach
 * Purpose:
 *      Detach the IPMC module.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_tr_ipmc_detach(int unit)
{
    _bcm_esw_ipmc_t    *info = IPMC_INFO(unit);
    _bcm_esw_ipmc_l3entry_t *ipmc_l3entry; 
    int i;

    if (info->ipmc_initialized) {
        BCM_IF_ERROR_RETURN(_tr_ipmc_enable(unit, FALSE));

        if (info->ipmc_group_info != NULL) {
            for (i = 0; i < IPMC_GROUP_NUM(unit); i++) {
                ipmc_l3entry = IPMC_GROUP_INFO(unit, i)->l3entry_list;
                while (ipmc_l3entry != NULL) {
                    IPMC_GROUP_INFO(unit, i)->l3entry_list = ipmc_l3entry->next;
                    sal_free(ipmc_l3entry);
                    ipmc_l3entry = IPMC_GROUP_INFO(unit, i)->l3entry_list;
                }
            }
            sal_free(info->ipmc_group_info);
            info->ipmc_group_info = NULL;
        }

        info->ipmc_initialized = FALSE;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_tr_ipmc_get
 * Purpose:
 *      Get an IPMC entry by index.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      index - The index number.
 *      ipmc - (OUT) IPMC entry information.
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_tr_ipmc_get(int unit, int index, bcm_ipmc_addr_t *ipmc)
{
    ipmc_entry_t        ipmc_entry;
#ifdef BCM_TRIUMPH2_SUPPORT
    ipmc_1_entry_t      ipmc_1_entry;
#endif

    IPMC_INIT(unit);
    IPMC_ID(unit, index);

    if (IPMC_USED_ISSET(unit, index)) {
        BCM_IF_ERROR_RETURN
            (soc_mem_read(unit, L3_IPMCm, MEM_BLOCK_ANY, index, &ipmc_entry));
#ifdef BCM_TRIUMPH2_SUPPORT
        if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
                SOC_IS_VALKYRIE2(unit) || SOC_IS_TRIDENT(unit) ||
                SOC_IS_KATANA(unit)) {
            BCM_IF_ERROR_RETURN
                (soc_mem_read(unit, L3_IPMC_1m, MEM_BLOCK_ANY, index, 
                              &ipmc_1_entry));
            BCM_IF_ERROR_RETURN
                (_tr2_ipmc_glp_get(unit, ipmc, &ipmc_1_entry));
        }
#endif
        BCM_IF_ERROR_RETURN
            (_tr_ipmc_info_get(unit, index, ipmc, &ipmc_entry, 1, NULL));

        ipmc->ipmc_index = index;

        return BCM_E_NONE;
    } 

    return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *      bcm_tr_ipmc_lookup
 * Purpose:
 *      Look up an IPMC entry by sip, mcip and vid
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      index - (OUT) The index number.
 *      ipmc - (IN, OUT) IPMC entry information.
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_tr_ipmc_lookup(int unit, int *index, bcm_ipmc_addr_t *ipmc)
{
    ipmc_entry_t   ipmc_entry;
#ifdef BCM_TRIUMPH2_SUPPORT
    ipmc_1_entry_t ipmc_1_entry;
#endif
    _bcm_l3_cfg_t  l3cfg;
    int            ipmc_id;

    IPMC_INIT(unit);

    sal_memset(&l3cfg, 0, sizeof(_bcm_l3_cfg_t));
    l3cfg.l3c_vid = ipmc->vid;
    l3cfg.l3c_flags = BCM_L3_IPMC;
    l3cfg.l3c_vrf = ipmc->vrf;
    if (ipmc->flags & BCM_IPMC_IP6) {
        sal_memcpy(l3cfg.l3c_sip6, ipmc->s_ip6_addr, BCM_IP6_ADDRLEN);
        sal_memcpy(l3cfg.l3c_ip6, ipmc->mc_ip6_addr, BCM_IP6_ADDRLEN);
        l3cfg.l3c_flags |= BCM_L3_IP6;
        ipmc->flags |= BCM_IPMC_IP6;
        BCM_IF_ERROR_RETURN(mbcm_driver[unit]->mbcm_l3_ip6_get(unit, &l3cfg));
    } else {
        l3cfg.l3c_src_ip_addr = ipmc->s_ip_addr;
        l3cfg.l3c_ipmc_group = ipmc->mc_ip_addr;
        BCM_IF_ERROR_RETURN(mbcm_driver[unit]->mbcm_l3_ip4_get(unit, &l3cfg));
        ipmc->flags &= ~BCM_IPMC_IP6;
    }

    ipmc_id = l3cfg.l3c_ipmc_ptr;
    BCM_IF_ERROR_RETURN
        (soc_mem_read(unit, L3_IPMCm, MEM_BLOCK_ANY, ipmc_id, &ipmc_entry));

#ifdef BCM_TRIUMPH2_SUPPORT
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) ||
            SOC_IS_VALKYRIE2(unit) || SOC_IS_TRIDENT(unit) ||
            SOC_IS_KATANA(unit)) {
        BCM_IF_ERROR_RETURN
            (soc_mem_read(unit, L3_IPMC_1m, MEM_BLOCK_ANY, ipmc_id,
                          &ipmc_1_entry));
        BCM_IF_ERROR_RETURN
            (_tr2_ipmc_glp_get(unit, ipmc, &ipmc_1_entry));
    }
#endif

    BCM_IF_ERROR_RETURN
        (_tr_ipmc_info_get(unit, ipmc_id, ipmc, &ipmc_entry, 0, NULL));
    
    if (ipmc->v) {
        ipmc->ipmc_index = ipmc_id;
        ipmc->lookup_class = l3cfg.l3c_lookup_class;
        if (l3cfg.l3c_flags & BCM_L3_HIT) {
            ipmc->flags |= BCM_IPMC_HIT;
        }
        if (l3cfg.l3c_flags & BCM_L3_RPE) {
            ipmc->cos =  l3cfg.l3c_prio;
            ipmc->flags |= BCM_IPMC_SETPRI;
        } else {
            ipmc->cos = -1;
            ipmc->flags &= ~BCM_IPMC_SETPRI;
        }
    }    
    if (index != NULL) {
        *index = ipmc_id;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_tr_ipmc_add
 * Purpose:
 *      Add a new entry to the L3 table.
 * Parameters:
 *      unit - (IN) BCM device number.
 *      ipmc - (IN) IPMC entry information.
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_bcm_tr_ipmc_add(int unit, bcm_ipmc_addr_t *ipmc)
{
    _bcm_l3_cfg_t       l3cfg; /* L3 ipmc entry.           */
    int                 rv;    /* Operation return status. */

    sal_memset(&l3cfg, 0, sizeof(_bcm_l3_cfg_t));

    l3cfg.l3c_vid = ipmc->vid;
    l3cfg.l3c_flags = BCM_L3_IPMC;
    l3cfg.l3c_vrf = ipmc->vrf;
    l3cfg.l3c_lookup_class = ipmc->lookup_class;
    if (ipmc->flags & BCM_IPMC_SETPRI) {
        l3cfg.l3c_flags |= BCM_L3_RPE;
        l3cfg.l3c_prio = ipmc->cos;
    }
    if (ipmc->flags & BCM_IPMC_IP6) {
        if (!BCM_IP6_MULTICAST(ipmc->mc_ip6_addr)) {
            return BCM_E_PARAM;
        }
        sal_memcpy(l3cfg.l3c_sip6, ipmc->s_ip6_addr, BCM_IP6_ADDRLEN);
        sal_memcpy(l3cfg.l3c_ip6, ipmc->mc_ip6_addr, BCM_IP6_ADDRLEN);
        l3cfg.l3c_flags |= BCM_L3_IP6;
    } else {
        if (!BCM_IP4_MULTICAST(ipmc->mc_ip_addr)) {
            return BCM_E_PARAM;
        }
        l3cfg.l3c_src_ip_addr = ipmc->s_ip_addr;
        l3cfg.l3c_ipmc_group = ipmc->mc_ip_addr;
    }

    l3cfg.l3c_ipmc_ptr = ipmc->ipmc_index;
    l3cfg.l3c_flags |=  BCM_L3_HIT;
    l3cfg.l3c_vid = ipmc->vid;

    if (ipmc->flags & BCM_IPMC_REPLACE) {
        l3cfg.l3c_flags |= BCM_L3_REPLACE;
    }

    if (ipmc->flags & BCM_IPMC_IP6) {
        rv = mbcm_driver[unit]->mbcm_l3_ip6_add(unit, &l3cfg);
    } else {
        rv = mbcm_driver[unit]->mbcm_l3_ip4_add(unit, &l3cfg);
    }

    if (BCM_SUCCESS(rv)) {
        rv = _bcm_tr_ipmc_l3entry_list_add(unit, ipmc->ipmc_index, 
                                           l3cfg); 
    }
    return (rv);
}

/*
 * Function:
 *      _bcm_tr_ipmc_del
 * Purpose:
 *      Remove an  entry from the L3 table.
 * Parameters:
 *      unit - (IN) BCM device number.
 *      ipmc - (IN)IPMC entry information.
 *      modify_l3entry_list - (IN) Controls whether to modify IPMC group's
 *                                 L3 entry list.
 * Returns:
 *      BCM_E_XXX
 */

STATIC int
_bcm_tr_ipmc_del(int unit, bcm_ipmc_addr_t *ipmc, int modify_l3entry_list)
{
    _bcm_l3_cfg_t       l3cfg; /* L3 ipmc entry.           */
    int                 rv;    /* Operation return status. */
    int                 ipmc_index = 0;

    sal_memset(&l3cfg, 0, sizeof(_bcm_l3_cfg_t));

    l3cfg.l3c_vid = ipmc->vid;
    l3cfg.l3c_flags = BCM_L3_IPMC;
    l3cfg.l3c_vrf = ipmc->vrf;
    l3cfg.l3c_vid = ipmc->vid;

    if (ipmc->flags & BCM_IPMC_IP6) {
        if (!BCM_IP6_MULTICAST(ipmc->mc_ip6_addr)) {
            return BCM_E_PARAM;
        }
        sal_memcpy(l3cfg.l3c_sip6, ipmc->s_ip6_addr, BCM_IP6_ADDRLEN);
        sal_memcpy(l3cfg.l3c_ip6, ipmc->mc_ip6_addr, BCM_IP6_ADDRLEN);
        l3cfg.l3c_flags |= BCM_L3_IP6;
    } else {
        if (!BCM_IP4_MULTICAST(ipmc->mc_ip_addr)) {
            return BCM_E_PARAM;
        }
        l3cfg.l3c_src_ip_addr = ipmc->s_ip_addr;
        l3cfg.l3c_ipmc_group = ipmc->mc_ip_addr;
    }

    if (ipmc->flags & BCM_IPMC_IP6) {
        rv = mbcm_driver[unit]->mbcm_l3_ip6_get(unit, &l3cfg);
        if (BCM_SUCCESS(rv)) {
            ipmc_index = l3cfg.l3c_ipmc_ptr;
            rv = mbcm_driver[unit]->mbcm_l3_ip6_delete(unit, &l3cfg);
        }
    } else {
        rv = mbcm_driver[unit]->mbcm_l3_ip4_get(unit, &l3cfg);
        if (BCM_SUCCESS(rv)) {
            ipmc_index = l3cfg.l3c_ipmc_ptr;
            rv = mbcm_driver[unit]->mbcm_l3_ip4_delete(unit, &l3cfg);
        }
    }

    if (BCM_SUCCESS(rv) && modify_l3entry_list) {
        rv = _bcm_tr_ipmc_l3entry_list_del(unit, ipmc_index, l3cfg); 
    }

    return (rv);
}

/*
 * Function:
 *      _bcm_tr_ipmc_src_port_compare
 * Purpose:
 *      Compare the IPMC source port parameters.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      ipmc_index - IPMC index to be shared.
 *      ipmc - IPMC address entry info.
 *      match - (OUT) Match indication.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_tr_ipmc_src_port_compare(int unit, int ipmc_index,
        bcm_ipmc_addr_t *ipmc, int *match)
{
    ipmc_entry_t entry;
    ipmc_1_entry_t entry_1;
    int no_src_check, is_trunk, tgid, mod, port;
    int t_f, tgid_f, mod_f, port_f;

    if (SOC_MEM_IS_VALID(unit, L3_IPMC_1m)) {
        BCM_IF_ERROR_RETURN
            (soc_mem_read(unit, L3_IPMC_1m, MEM_BLOCK_ANY, ipmc_index,
                          &entry_1));
    } else {
        BCM_IF_ERROR_RETURN
            (soc_mem_read(unit, L3_IPMCm, MEM_BLOCK_ANY, ipmc_index,
                          &entry));
    }

    no_src_check = FALSE;
    is_trunk = 0;
    tgid = -1;
    mod = -1;
    port = -1;
    if (SOC_MEM_IS_VALID(unit, L3_IPMC_1m)) {
        t_f    = soc_L3_IPMC_1m_field32_get(unit, &entry_1, Tf);
        tgid_f = soc_L3_IPMC_1m_field32_get(unit, &entry_1, TGIDf);
        mod_f  = soc_L3_IPMC_1m_field32_get(unit, &entry_1, MODULE_IDf);
        port_f = soc_L3_IPMC_1m_field32_get(unit, &entry_1, PORT_NUMf);
    } else {
        t_f    = soc_L3_IPMCm_field32_get(unit, &entry, Tf);
        tgid_f = soc_L3_IPMCm_field32_get(unit, &entry, TGIDf);
        mod_f  = soc_L3_IPMCm_field32_get(unit, &entry, MODULE_IDf);
        port_f = soc_L3_IPMCm_field32_get(unit, &entry, PORT_NUMf);
    }

    if ((t_f == 1) && (mod_f == SOC_MODID_MAX(unit)) &&
            (port_f == TR_IPMC_NO_SRC_CHECK_PORT(unit))) {
        no_src_check = TRUE;
    } else if (t_f == 1) {
        is_trunk = 1;
        tgid = tgid_f;
    } else {
        mod = mod_f;
        port = port_f;
    }

    *match = FALSE;
    if ((ipmc->flags & BCM_IPMC_SOURCE_PORT_NOCHECK) ||
            (ipmc->port_tgid < 0)) {                        /* no source port */
        if (no_src_check) {
            *match = TRUE;
        }
    } else if (ipmc->ts) {                              /* trunk source port */
        if (is_trunk && (tgid == ipmc->port_tgid)) {
            *match = TRUE;
        }
    } else {                                            /* source port */
        bcm_module_t    mod_in, mod_out;
        bcm_port_t      port_in, port_out;

        mod_in = ipmc->mod_id;
        port_in = ipmc->port_tgid;
        BCM_IF_ERROR_RETURN
            (_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_SET,
                                     mod_in, port_in,
                                     &mod_out, &port_out));
        /* Check parameters, since above is an application callback */
        if (!SOC_MODID_ADDRESSABLE(unit, mod_out)) {
            return BCM_E_BADID;
        }
        if (!SOC_PORT_ADDRESSABLE(unit, port_out)) {
            return BCM_E_PORT;
        }

        if ((mod == mod_out) && (port == port_out)) {
            *match = TRUE;
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_tr_ipmc_src_port_consistency_check
 * Purpose:
 *      When multiple IPMC address entries share the same IPMC
 *      index, they must also have the same port parameter
 *      for the purpose of IPMC source port checking. This
 *      procedure makes sure this is the case.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      ipmc_index - IPMC index to be shared.
 *      ipmc - IPMC address entry info.
 *      already_used - Indicates if the given IPMC address entry is
 *                     already using the ipmc index to be shared.
 * Returns:
 *      BCM_E_XXX
 */
STATIC int
_bcm_tr_ipmc_src_port_consistency_check(int unit, int ipmc_index,
        bcm_ipmc_addr_t *ipmc, int already_used)
{
    int l3entry_list_size;
    int match;

    BCM_IF_ERROR_RETURN(_bcm_tr_ipmc_l3entry_list_size_get(unit,
                ipmc_index, &l3entry_list_size));
    if ((already_used && (l3entry_list_size > 1)) ||
            (!already_used && (l3entry_list_size > 0))) {
        /* If there are IPMC address entries other than the given IPMC address
         * entry that are pointing the given ipmc index, verify that the IPMC
         * source port check parameters are the same. If not, the ipmc index
         * cannot be shared.
         */ 
        BCM_IF_ERROR_RETURN(_bcm_tr_ipmc_src_port_compare(unit,
                    ipmc_index, ipmc, &match));
        if (!match) {
            return BCM_E_PARAM;
        } 
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_tr_ipmc_add
 * Purpose:
 *      Add a new entry to the IPMC table.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      ipmc - IPMC entry information.
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_tr_ipmc_add(int unit, bcm_ipmc_addr_t *ipmc)
{

    bcm_ipmc_addr_t ipmc_lookup_data;
    int             old_ipmc_index;
    int             delete_old_ipmc_repl_tree = 0;
    bcm_pbmp_t      repl_pbmp;
    ipmc_entry_t ipmc_entry;
    int new_entry, rv;
#if defined(BCM_TRIUMPH2_SUPPORT)
    ipmc_1_entry_t ipmc_1_entry;
#endif

    IPMC_INIT(unit);

    /* Check if IPMC entry already exists */

    ipmc_lookup_data = *ipmc;
    rv = bcm_tr_ipmc_lookup(unit, &old_ipmc_index, &ipmc_lookup_data);
    if (BCM_SUCCESS(rv)) {
        if (!(ipmc->flags & BCM_IPMC_REPLACE)) {
           return (BCM_E_EXISTS);
        } else {
            new_entry = FALSE;
        }
    } else {
        /* Return if error occured. */
        if (rv != BCM_E_NOT_FOUND) {
            return (rv);
        }
        new_entry = TRUE;
    }

    /* Resolve ipmc index */

    if(ipmc->flags & BCM_IPMC_USE_IPMC_INDEX) {
        if (_BCM_MULTICAST_IS_SET(ipmc->ipmc_index)) {
            ipmc->ipmc_index = _BCM_MULTICAST_ID_GET(ipmc->ipmc_index); 

            /* The given ipmc_index was created by bcm_multicast APIs. It's
             * assumed that the IPMC group's l2_pbmp and l3_pbmp have already
             * been or will be configured by bcm_multicast APIs. Hence, l2_pbmp and
             * l3_pbmp are updated with what's already configured in hardware.
             */
            BCM_IF_ERROR_RETURN
                (_bcm_esw_multicast_ipmc_read(unit,  ipmc->ipmc_index, 
                                              &ipmc->l2_pbmp, &ipmc->l3_pbmp));
        }

        if (new_entry) { 
            BCM_IF_ERROR_RETURN(_bcm_tr_ipmc_src_port_consistency_check(unit,
                        ipmc->ipmc_index, ipmc, 0));

            /* Increment the reference count of the given ipmc_index. */
            BCM_IF_ERROR_RETURN(bcm_xgs3_ipmc_id_alloc(unit, ipmc->ipmc_index));
        } else {
            if (ipmc->ipmc_index != old_ipmc_index) {
                BCM_IF_ERROR_RETURN
                    (_bcm_tr_ipmc_src_port_consistency_check(unit,
                                                             ipmc->ipmc_index,
                                                             ipmc, 0));
                /* Increment the reference count of the given ipmc_index. */
                BCM_IF_ERROR_RETURN(bcm_xgs3_ipmc_id_alloc(unit, ipmc->ipmc_index));

                /* Decrement the reference count of the old ipmc_index. */
                BCM_IF_ERROR_RETURN(bcm_xgs3_ipmc_id_free(unit, old_ipmc_index));
                if (!IPMC_USED_ISSET(unit, old_ipmc_index)) {
                    delete_old_ipmc_repl_tree = 1; 
                }
            } else {
                BCM_IF_ERROR_RETURN
                    (_bcm_tr_ipmc_src_port_consistency_check(unit,
                                                             ipmc->ipmc_index,
                                                             ipmc, 1));
            }
        }
    } else {
        if (new_entry){
            /* Allocate new unused entry index. */
            BCM_IF_ERROR_RETURN(bcm_xgs3_ipmc_create(unit, &ipmc->ipmc_index));
        } else {
            /* IPMC entry already exists, use the old ipmc index */
            ipmc->ipmc_index = old_ipmc_index;

            BCM_IF_ERROR_RETURN(_bcm_tr_ipmc_src_port_consistency_check(unit,
                        ipmc->ipmc_index, ipmc, 1));
        }
    }

    ipmc->v = (ipmc->flags & BCM_IPMC_ADD_DISABLED) ? 0 : 1;

    if (new_entry) {
        /* Write L3_IPMC table entry. */
        rv = _tr_ipmc_write(unit, ipmc->ipmc_index, ipmc);
        if (BCM_FAILURE(rv)) {
            bcm_xgs3_ipmc_id_free(unit, ipmc->ipmc_index);
            return (rv);
        }

        /* Add new L3 table entry */
        rv = _bcm_tr_ipmc_add(unit, ipmc);
        if (BCM_FAILURE(rv)) {
            bcm_xgs3_ipmc_id_free(unit, ipmc->ipmc_index);
            if (!IPMC_USED_ISSET(unit, ipmc->ipmc_index)) {
                sal_memset(&ipmc_entry, 0, sizeof(ipmc_entry));
                SOC_IF_ERROR_RETURN
                    (soc_mem_write(unit, L3_IPMCm, MEM_BLOCK_ALL, ipmc->ipmc_index, &ipmc_entry));

#if defined(BCM_TRIUMPH2_SUPPORT)
                if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
                        SOC_IS_VALKYRIE2(unit) || SOC_IS_TRIDENT(unit) ||
                        SOC_IS_KATANA(unit)) {
                    sal_memset(&ipmc_1_entry, 0, sizeof(ipmc_1_entry));
                    SOC_IF_ERROR_RETURN
                        (soc_mem_write(unit, L3_IPMC_1m, MEM_BLOCK_ALL, ipmc->ipmc_index, &ipmc_1_entry));
                } 
#endif /* BCM_TRIUMPH2_SUPPORT */
            }

            bcm_xgs3_ipmc_id_free(unit, ipmc->ipmc_index);
            return (rv);
        }
    } else {
        /* Update existing IPMC entry */
        rv = bcm_tr_ipmc_put(unit, ipmc->ipmc_index, ipmc);
        if (BCM_FAILURE(rv)) {
            _bcm_tr_ipmc_del(unit, ipmc, TRUE);
            bcm_xgs3_ipmc_id_free(unit, ipmc->ipmc_index);
            return rv;
        }
        
        if (delete_old_ipmc_repl_tree) {
            /* Delete the old IPMC distribution tree */

            SOC_IF_ERROR_RETURN(soc_mem_read(unit, L3_IPMCm, MEM_BLOCK_ANY, old_ipmc_index, &ipmc_entry));
            SOC_PBMP_CLEAR(repl_pbmp);
            soc_mem_pbmp_field_get(unit, L3_IPMCm, &ipmc_entry, L3_BITMAPf, &repl_pbmp);
            SOC_PBMP_AND(repl_pbmp, PBMP_PORT_ALL(unit));

            sal_memset(&ipmc_entry, 0, sizeof(ipmc_entry));
            SOC_IF_ERROR_RETURN
                (soc_mem_write(unit, L3_IPMCm, MEM_BLOCK_ALL, old_ipmc_index, &ipmc_entry));
#if defined(BCM_TRIUMPH2_SUPPORT)
            if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
                    SOC_IS_VALKYRIE2(unit) || SOC_IS_TRIDENT(unit) ||
                    SOC_IS_KATANA(unit)) {
                sal_memset(&ipmc_1_entry, 0, sizeof(ipmc_1_entry));
                SOC_IF_ERROR_RETURN
                    (soc_mem_write(unit, L3_IPMC_1m, MEM_BLOCK_ALL, old_ipmc_index, &ipmc_1_entry));
            } 
#endif /* BCM_TRIUMPH2_SUPPORT */

            /* Delete all replications */
            BCM_IF_ERROR_RETURN
                (bcm_esw_ipmc_repl_delete_all(unit, old_ipmc_index, repl_pbmp));
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_tr_ipmc_put
 * Purpose:
 *      Overwrite an entry in the IPMC table.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      index - Table index to overwrite.
 *      data - IPMC entry information.
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_tr_ipmc_put(int unit, int index, bcm_ipmc_addr_t *ipmc)
{
    _bcm_l3_cfg_t   l3cfg;
    int old_ipmc_index;

    IPMC_INIT(unit);
    IPMC_ID(unit, index);

    sal_memset(&l3cfg, 0, sizeof(_bcm_l3_cfg_t));

    l3cfg.l3c_vid = ipmc->vid;
    l3cfg.l3c_flags = BCM_L3_IPMC;
    l3cfg.l3c_vrf = ipmc->vrf;
    l3cfg.l3c_lookup_class = ipmc->lookup_class;
    if (ipmc->flags & BCM_IPMC_IP6) {
        sal_memcpy(l3cfg.l3c_sip6, ipmc->s_ip6_addr, BCM_IP6_ADDRLEN);
        sal_memcpy(l3cfg.l3c_ip6, ipmc->mc_ip6_addr, BCM_IP6_ADDRLEN);
        l3cfg.l3c_flags |= BCM_L3_IP6;
        BCM_IF_ERROR_RETURN
            (mbcm_driver[unit]->mbcm_l3_ip6_get(unit, &l3cfg));
    } else {
        l3cfg.l3c_src_ip_addr = ipmc->s_ip_addr;
        l3cfg.l3c_ipmc_group = ipmc->mc_ip_addr;
        BCM_IF_ERROR_RETURN
            (mbcm_driver[unit]->mbcm_l3_ip4_get(unit, &l3cfg));
    }
    old_ipmc_index = l3cfg.l3c_ipmc_ptr;

    if (!(ipmc->flags & BCM_IPMC_SETPRI)) {
        l3cfg.l3c_flags &= ~BCM_L3_RPE;
        l3cfg.l3c_prio = 0;
    } else {
        l3cfg.l3c_flags |= BCM_L3_RPE;
        l3cfg.l3c_prio = ipmc->cos;
    }
    if (ipmc->flags & BCM_IPMC_USE_IPMC_INDEX) {
        l3cfg.l3c_ipmc_ptr = ipmc->ipmc_index;
    }
    BCM_IF_ERROR_RETURN
        (bcm_xgs3_l3_replace(unit, &l3cfg));

    if (ipmc->flags & BCM_IPMC_USE_IPMC_INDEX) {
        if (old_ipmc_index != ipmc->ipmc_index) {
            BCM_IF_ERROR_RETURN
                (_bcm_tr_ipmc_l3entry_list_add(unit, ipmc->ipmc_index,
                                               l3cfg)); 
            BCM_IF_ERROR_RETURN
                (_bcm_tr_ipmc_l3entry_list_del(unit, old_ipmc_index,
                                               l3cfg)); 
        } else {
            BCM_IF_ERROR_RETURN
                (_bcm_tr_ipmc_l3entry_list_update(unit, old_ipmc_index,
                                                  l3cfg));
        }
    } else {
        BCM_IF_ERROR_RETURN
            (_bcm_tr_ipmc_l3entry_list_update(unit, old_ipmc_index,
                                              l3cfg));
    }

    return _tr_ipmc_write(unit, index, ipmc);
}

/*
 * Function:
 *      _bcm_tr_ipmc_delete
 * Purpose:
 *      Delete an entry from the IPMC table.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      data - IPMC entry information.
 *      modify_l3entry_list - Control whether to modify IPMC group's linked
 *                            list of L3 entries.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      If BCM_IPMC_KEEP_ENTRY is true, the entry valid bit is cleared
 *      but the entry is not deleted from the table.
 */

STATIC int
_bcm_tr_ipmc_delete(int unit, bcm_ipmc_addr_t *ipmc, int modify_l3entry_list)
{
    int                 ipmc_id;
    ipmc_entry_t        ipmc_entry;
    _bcm_l3_cfg_t       l3cfg;
    bcm_pbmp_t          repl_pbmp;
#if defined(BCM_TRIUMPH2_SUPPORT)
    ipmc_1_entry_t      ipmc_1_entry;
#endif /* BCM_TRIUMPH2_SUPPORT */

    IPMC_INIT(unit);

    sal_memset(&l3cfg, 0, sizeof(_bcm_l3_cfg_t));
    l3cfg.l3c_vid = ipmc->vid;
    l3cfg.l3c_flags = BCM_L3_IPMC;
    l3cfg.l3c_vrf = ipmc->vrf;
    if (ipmc->flags & BCM_IPMC_IP6) {
        sal_memcpy(l3cfg.l3c_sip6, ipmc->s_ip6_addr, BCM_IP6_ADDRLEN);
        sal_memcpy(l3cfg.l3c_ip6, ipmc->mc_ip6_addr, BCM_IP6_ADDRLEN);
        l3cfg.l3c_flags |= BCM_L3_IP6;
        BCM_IF_ERROR_RETURN(
            mbcm_driver[unit]->mbcm_l3_ip6_get(unit, &l3cfg));
    } else {
        l3cfg.l3c_src_ip_addr = ipmc->s_ip_addr;
        l3cfg.l3c_ipmc_group = ipmc->mc_ip_addr;
        BCM_IF_ERROR_RETURN(
            mbcm_driver[unit]->mbcm_l3_ip4_get(unit, &l3cfg));
    }

    ipmc_id = l3cfg.l3c_ipmc_ptr;
    BCM_IF_ERROR_RETURN
        (soc_mem_read(unit, L3_IPMCm, MEM_BLOCK_ANY, ipmc_id, &ipmc_entry));

    SOC_PBMP_CLEAR(repl_pbmp);
    soc_mem_pbmp_field_get(unit, L3_IPMCm, &ipmc_entry, L3_BITMAPf, &repl_pbmp);
    SOC_PBMP_AND(repl_pbmp, PBMP_PORT_ALL(unit));

    if (!(ipmc->flags & BCM_IPMC_KEEP_ENTRY)) {
        BCM_IF_ERROR_RETURN
            (_bcm_tr_ipmc_del(unit, ipmc, modify_l3entry_list));

        bcm_xgs3_ipmc_id_free(unit, ipmc_id);
        if (!IPMC_USED_ISSET(unit, ipmc_id)) {

            sal_memset(&ipmc_entry, 0, sizeof(ipmc_entry));
            SOC_IF_ERROR_RETURN
                (soc_mem_write(unit, L3_IPMCm, MEM_BLOCK_ALL, ipmc_id, &ipmc_entry));

#if defined(BCM_TRIUMPH2_SUPPORT)
            if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
                    SOC_IS_VALKYRIE2(unit) || SOC_IS_TRIDENT(unit) ||
                    SOC_IS_KATANA(unit)) {
                sal_memset(&ipmc_1_entry, 0, sizeof(ipmc_1_entry));
                SOC_IF_ERROR_RETURN
                    (soc_mem_write(unit, L3_IPMC_1m, MEM_BLOCK_ALL, ipmc_id, &ipmc_1_entry));
            } 
#endif /* BCM_TRIUMPH2_SUPPORT */

            /* Delete all replications */
            BCM_IF_ERROR_RETURN
                (bcm_esw_ipmc_repl_delete_all(unit, ipmc_id, repl_pbmp));
        }

    } else {
        sal_memset(&ipmc_entry, 0, sizeof(ipmc_entry));
        SOC_IF_ERROR_RETURN
            (soc_mem_write(unit, L3_IPMCm, MEM_BLOCK_ALL, ipmc_id, &ipmc_entry));
#if defined(BCM_TRIUMPH2_SUPPORT)
        if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
                SOC_IS_VALKYRIE2(unit) || SOC_IS_TRIDENT(unit) ||
                SOC_IS_KATANA(unit)) {
            sal_memset(&ipmc_1_entry, 0, sizeof(ipmc_1_entry));
            SOC_IF_ERROR_RETURN
                (soc_mem_write(unit, L3_IPMC_1m, MEM_BLOCK_ALL, ipmc_id, &ipmc_1_entry));
        } 
#endif /* BCM_TRIUMPH2_SUPPORT */
    }

#ifdef BCM_BRADLEY_SUPPORT
    if (SOC_REG_FIELD_VALID(unit, MC_CONTROL_5r, SHARED_TABLE_IPMC_SIZEf)) {
        if (!IPMC_USED_ISSET(unit, ipmc_id)) {
            BCM_IF_ERROR_RETURN
                (_bcm_xgs3_ipmc_bitmap_clear(unit, ipmc_id));
        }
    }
#endif /* BRADLEY_SUPPORT */

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_tr_ipmc_delete
 * Purpose:
 *      Delete an entry from the IPMC table.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      data - IPMC entry information.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      If BCM_IPMC_KEEP_ENTRY is true, the entry valid bit is cleared
 *      but the entry is not deleted from the table.
 */

int
bcm_tr_ipmc_delete(int unit, bcm_ipmc_addr_t *ipmc)
{
    return _bcm_tr_ipmc_delete(unit, ipmc, TRUE);
}

/*
 * Function:
 *      bcm_tr_ipmc_delete_all
 * Purpose:
 *      Delete all entries from the IPMC table.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_tr_ipmc_delete_all(int unit)
{
    ipmc_entry_t ipmc_entry;
    bcm_pbmp_t repl_pbmp;
    int i, rv = BCM_E_NONE;
    _bcm_esw_ipmc_l3entry_t *ipmc_l3entry;
    _bcm_l3_cfg_t l3cfg;

    IPMC_INIT(unit);

    IPMC_LOCK(unit);
    for (i = 0; i < IPMC_GROUP_NUM(unit); i++) {
        if (IPMC_USED_ISSET(unit, i)) {
            ipmc_l3entry = IPMC_GROUP_INFO(unit, i)->l3entry_list;
            while (ipmc_l3entry != NULL) {
                sal_memset(&l3cfg, 0, sizeof(_bcm_l3_cfg_t));
                l3cfg.l3c_vid = ipmc_l3entry->l3info.vid;
                l3cfg.l3c_flags = BCM_L3_IPMC;
                l3cfg.l3c_vrf = ipmc_l3entry->l3info.vrf;
                if (ipmc_l3entry->l3info.flags & BCM_L3_IP6) {
                    sal_memcpy(l3cfg.l3c_sip6, ipmc_l3entry->l3info.sip6, BCM_IP6_ADDRLEN);
                    sal_memcpy(l3cfg.l3c_ip6, ipmc_l3entry->l3info.ip6, BCM_IP6_ADDRLEN);
                    l3cfg.l3c_flags |= BCM_L3_IP6;
                } else {
                    l3cfg.l3c_src_ip_addr = ipmc_l3entry->l3info.src_ip_addr;
                    l3cfg.l3c_ipmc_group = ipmc_l3entry->l3info.ip_addr;
                }
                rv = bcm_xgs3_l3_del(unit, &l3cfg);
                if (rv < 0) {
                    goto error;
                }
                IPMC_GROUP_INFO(unit, i)->l3entry_list = ipmc_l3entry->next;
                sal_free(ipmc_l3entry);
                ipmc_l3entry = IPMC_GROUP_INFO(unit, i)->l3entry_list;
            }

            /* Get the replication L3 port bitmap */
            rv = soc_mem_read(unit, L3_IPMCm, MEM_BLOCK_ANY, i, &ipmc_entry);
            if (rv < 0) {
                goto error;
            }
            SOC_PBMP_CLEAR(repl_pbmp);
            soc_mem_pbmp_field_get(unit, L3_IPMCm, &ipmc_entry, L3_BITMAPf, &repl_pbmp);
            SOC_PBMP_AND(repl_pbmp, PBMP_PORT_ALL(unit));

            /* Clear the L3_IPMC entry */
            rv = WRITE_L3_IPMCm(unit, MEM_BLOCK_ALL, i,
                                soc_mem_entry_null(unit, L3_IPMCm));
            if (rv < 0) {
                goto error;
            }
#ifdef BCM_TRIUMPH2_SUPPORT
            if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
                    SOC_IS_VALKYRIE2(unit) || SOC_IS_TRIDENT(unit) ||
                    SOC_IS_KATANA(unit)) {
                rv = WRITE_L3_IPMC_1m(unit, MEM_BLOCK_ALL, i,
                        soc_mem_entry_null(unit, L3_IPMC_1m));
                if (rv < 0) {
                    goto error;
                }
            }
#endif /* BCM_TRIUMPH2_SUPPORT */

#ifdef BCM_BRADLEY_SUPPORT
            if (SOC_REG_FIELD_VALID(unit, MC_CONTROL_5r, SHARED_TABLE_IPMC_SIZEf)) {
                rv = _bcm_xgs3_ipmc_bitmap_clear(unit, i);
                if (rv < 0) {
                    goto error;
                }
            }
#endif /* BCM_BRADLEY_SUPPORT */

            /* Delete all replications */
            rv = bcm_esw_ipmc_repl_delete_all(unit, i, repl_pbmp);
            if (rv < 0) {
                goto error;
            }

            IPMC_USED_ZERO(unit, i);
        }
    }

error:
    IPMC_UNLOCK(unit);

    return rv;
}

/*
 * Function:
 *      bcm_tr_ipmc_age
 * Purpose:
 *      Age out the ipmc entry by clearing the HIT bit when appropriate,
 *      the ipmc entry itself is removed if HIT bit is not set.
 * Parameters:
 *      unit       -  (IN) BCM device number.
 *      flags      -  (IN) The criteria used to age out ipmc table.
 *                         IPv6/IPv4
 *      age_cb     -  (IN) Call back routine.
 *      user_data  -  (IN) User provided cookie for callback.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_tr_ipmc_age(int unit, uint32 flags, bcm_ipmc_traverse_cb age_cb,
                void *user_data)
{
    int idx;                   /* Ipmc table iteration index. */
    bcm_ipmc_addr_t entry;     /* Ipmc entry iterator.        */
    int rv = BCM_E_NONE;       /* Operation return status.    */

    ipmc_entry_t   ipmc_entry;
#ifdef BCM_TRIUMPH2_SUPPORT
    ipmc_1_entry_t ipmc_1_entry;
#endif
    _bcm_esw_ipmc_l3entry_t *ipmc_l3entry;
    _bcm_esw_ipmc_l3entry_t *prev_ipmc_l3entry = NULL;

    IPMC_INIT(unit);
    IPMC_LOCK(unit);

    for (idx = 0; idx < IPMC_GROUP_NUM(unit); idx++) {
        if (IPMC_USED_ISSET(unit, idx)) {

            rv = soc_mem_read(unit, L3_IPMCm, MEM_BLOCK_ANY, idx, &ipmc_entry);
            if (BCM_FAILURE(rv)) {
                goto age_done;
            }

#ifdef BCM_TRIUMPH2_SUPPORT
            if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
                    SOC_IS_VALKYRIE2(unit) || SOC_IS_TRIDENT(unit) ||
                    SOC_IS_KATANA(unit)) {
                rv = soc_mem_read(unit, L3_IPMC_1m, MEM_BLOCK_ANY, idx, 
                                  &ipmc_1_entry);
                if (BCM_FAILURE(rv)) {
                    goto age_done;
                }
            }
#endif /* BCM_TRIUMPH2_SUPPORT */

            ipmc_l3entry = IPMC_GROUP_INFO(unit, idx)->l3entry_list;
            while (ipmc_l3entry != NULL) {
                sal_memset(&entry, 0, sizeof(bcm_ipmc_addr_t));
#ifdef BCM_TRIUMPH2_SUPPORT
                if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
                    SOC_IS_VALKYRIE2(unit) || SOC_IS_TRIDENT(unit) ||
                    SOC_IS_KATANA(unit)) {
                    rv = _tr2_ipmc_glp_get(unit, &entry, &ipmc_1_entry);
                    if (BCM_FAILURE(rv)) {
                        goto age_done;
                    }
                }
#endif /* BCM_TRIUMPH2_SUPPORT */
                rv = _tr_ipmc_info_get(unit, idx, &entry, &ipmc_entry, 1, ipmc_l3entry);
                if (BCM_FAILURE(rv)) {
                    goto age_done;
                }

                /* Make sure update only ipv4 or ipv6 entries. */
                if ((flags & BCM_IPMC_IP6) != (entry.flags & BCM_IPMC_IP6)) {
                    prev_ipmc_l3entry = ipmc_l3entry;
                    ipmc_l3entry = ipmc_l3entry->next;
                    continue;
                }

                if (entry.flags & BCM_IPMC_HIT) {
                    _bcm_l3_cfg_t l3cfg;
                    /* Clear hit bit on used entry (by doing a lookup !!) */
                    sal_memset(&l3cfg, 0, sizeof(_bcm_l3_cfg_t));
                    l3cfg.l3c_flags |= BCM_L3_HIT_CLEAR;
                    l3cfg.l3c_vid = ipmc_l3entry->l3info.vid;
                    l3cfg.l3c_flags |= BCM_L3_IPMC;
                    l3cfg.l3c_vrf = ipmc_l3entry->l3info.vrf;
                    l3cfg.l3c_vid = ipmc_l3entry->l3info.vid;

                    if (ipmc_l3entry->ip6) {
                        sal_memcpy(l3cfg.l3c_sip6, &ipmc_l3entry->l3info.sip6, 
                                   BCM_IP6_ADDRLEN);
                        sal_memcpy(l3cfg.l3c_ip6, &ipmc_l3entry->l3info.ip6, 
                                   BCM_IP6_ADDRLEN);
                        l3cfg.l3c_flags |= BCM_L3_IP6;
                        rv = (mbcm_driver[unit]->mbcm_l3_ip6_get(unit, &l3cfg));
                    } else {
                        l3cfg.l3c_src_ip_addr = ipmc_l3entry->l3info.src_ip_addr;
                        l3cfg.l3c_ipmc_group = ipmc_l3entry->l3info.ipmc_group;
                        rv = (mbcm_driver[unit]->mbcm_l3_ip4_get(unit, &l3cfg));
                    }
                    if (BCM_FAILURE(rv)) {
                        goto age_done;
                    }
                    prev_ipmc_l3entry = ipmc_l3entry;
                    ipmc_l3entry = ipmc_l3entry->next;
                } else {
                    /* Delete IPMC L3 entry. Inhibit modification of IPMC group's
                     * l3entry_list by _bcm_tr_ipmc_delete.
                     */
                    rv = _bcm_tr_ipmc_delete(unit, &entry, FALSE);
                    if (BCM_FAILURE(rv)) {
                        goto age_done;
                    }

                    /* Delete from IPMC group's l3entry_list */
                    if (ipmc_l3entry == IPMC_GROUP_INFO(unit, idx)->l3entry_list) {
                        IPMC_GROUP_INFO(unit, idx)->l3entry_list = ipmc_l3entry->next;
                        sal_free(ipmc_l3entry);
                        ipmc_l3entry = IPMC_GROUP_INFO(unit, idx)->l3entry_list;
                    } else {
                        /* 
                         * In the following line of code, Coverity thinks the
                         * prev_ipmc_l3entry pointer may still be NULL when 
                         * dereferenced. This situation will never occur because 
                         * if ipmc_l3entry is not pointing to the head of the 
                         * linked list, prev_ipmc_l3entry would not be NULL.
                         */
                        /* coverity[var_deref_op : FALSE] */
                        prev_ipmc_l3entry->next = ipmc_l3entry->next;
                        sal_free(ipmc_l3entry);
                        ipmc_l3entry = prev_ipmc_l3entry->next;
                    }

                    /* Invoke user callback. */
                    if (NULL != age_cb) {
                        rv = (*age_cb)(unit, &entry, user_data);
#ifdef BCM_CB_ABORT_ON_ERR
                        if (BCM_FAILURE(rv) && SOC_CB_ABORT_ON_ERR(unit)) {
                            goto age_done;
                        }
#endif
                    }
                }
            }
        }
    }

age_done:
    IPMC_UNLOCK(unit);
    return (rv);
}

/*
 * Function:
 *      bcm_tr_ipmc_traverse
 * Purpose:
 *      Go through all valid ipmc entries, and call the callback function
 *      at each entry
 * Parameters:
 *      unit      - (IN) BCM device number.
 *      flags     - (IN) The criteria used to age out ipmc table.
 *      cb        - (IN) User supplied callback function.
 *      user_data - (IN) User supplied cookie used in parameter
 *                       in callback function.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_tr_ipmc_traverse(int unit, uint32 flags, bcm_ipmc_traverse_cb cb,
                     void *user_data)
{
    int idx;                   /* Ipmc table iteration index. */
    bcm_ipmc_addr_t entry;     /* Ipmc entry iterator.        */
    int rv = BCM_E_NONE;       /* Operation return status.    */

    ipmc_entry_t   ipmc_entry;
#ifdef BCM_TRIUMPH2_SUPPORT
    ipmc_1_entry_t ipmc_1_entry;
#endif
    _bcm_esw_ipmc_l3entry_t *ipmc_l3entry;

    IPMC_INIT(unit);
    IPMC_LOCK(unit);

    for (idx = 0; idx < IPMC_GROUP_NUM(unit); idx++) {
        if (IPMC_USED_ISSET(unit, idx)) {

            rv = soc_mem_read(unit, L3_IPMCm, MEM_BLOCK_ANY, idx, &ipmc_entry);
            if (BCM_FAILURE(rv)) {
                goto traverse_done;
            }

#ifdef BCM_TRIUMPH2_SUPPORT
            if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
                    SOC_IS_VALKYRIE2(unit) || SOC_IS_TRIDENT(unit) ||
                    SOC_IS_KATANA(unit)) {
                rv = soc_mem_read(unit, L3_IPMC_1m, MEM_BLOCK_ANY, idx, 
                        &ipmc_1_entry);
                if (BCM_FAILURE(rv)) {
                    goto traverse_done;
                }
            }
#endif /* BCM_TRIUMPH2_SUPPORT */

            ipmc_l3entry = IPMC_GROUP_INFO(unit, idx)->l3entry_list;
            while (ipmc_l3entry != NULL) {
                sal_memset(&entry, 0, sizeof(bcm_ipmc_addr_t));
#ifdef BCM_TRIUMPH2_SUPPORT
                if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
                        SOC_IS_VALKYRIE2(unit) || SOC_IS_TRIDENT(unit) ||
                        SOC_IS_KATANA(unit)) {
                    rv = _tr2_ipmc_glp_get(unit, &entry, &ipmc_1_entry);
                    if (BCM_FAILURE(rv)) {
                        goto traverse_done;
                    }
                }
#endif /* BCM_TRIUMPH2_SUPPORT */
                rv = _tr_ipmc_info_get(unit, idx, &entry, &ipmc_entry, 1, ipmc_l3entry);
                if (BCM_FAILURE(rv)) {
                    goto traverse_done;
                }

                /* Make sure update only ipv4 or ipv6 entries. */
                if ((flags & BCM_IPMC_IP6) != (entry.flags & BCM_IPMC_IP6)) {
                    ipmc_l3entry = ipmc_l3entry->next;
                    continue;
                }

                /* Invoke user callback. */
                rv = (*cb)(unit, &entry, user_data);
#ifdef BCM_CB_ABORT_ON_ERR
                if (BCM_FAILURE(rv) && SOC_CB_ABORT_ON_ERR(unit)) {
                    goto traverse_done;
                }
#endif
                ipmc_l3entry = ipmc_l3entry->next;
            }
        }
    }

traverse_done:
    IPMC_UNLOCK(unit);
    return (rv);
}

/*
 * Function:
 *      bcm_tr_ipmc_enable
 * Purpose:
 *      Enable or disable IPMC chip functions.
 * Parameters:
 *      unit - Unit number
 *      enable - TRUE to enable; FALSE to disable
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_tr_ipmc_enable(int unit, int enable)
{
    IPMC_INIT(unit);

    return _tr_ipmc_enable(unit, enable);
}


/*
 * Function:
 *      bcm_tr_ipmc_src_port_check
 * Purpose:
 *      Enable or disable Source Port checking in IPMC lookups.
 * Parameters:
 *      unit - Unit number
 *      enable - TRUE to enable; FALSE to disable
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_tr_ipmc_src_port_check(int unit, int enable)
{
    IPMC_INIT(unit);

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_tr_ipmc_src_ip_search
 * Purpose:
 *      Enable or disable Source IP significance in IPMC lookups.
 * Parameters:
 *      unit - Unit number
 *      enable - TRUE to enable; FALSE to disable
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_tr_ipmc_src_ip_search(int unit, int enable)
{
    IPMC_INIT(unit);

    if (enable) {
        return BCM_E_NONE;  /* always on */
    } else {
        return BCM_E_FAIL;  /* cannot be disabled */
    }
}

/*
 * Function:
 *      bcm_tr_ipmc_egress_port_set
 * Purpose:
 *      Configure the IP Multicast egress properties
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      port - port to config.
 *      mac  - MAC address.
 *      untag - 1: The IP multicast packet is transmitted as untagged packet.
 *              0: The IP multicast packet is transmitted as tagged packet
 *              with VLAN tag vid.
 *      vid  - VLAN ID.
 *      ttl  - 1 to disable the TTL decrement, 0 otherwise.
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_tr_ipmc_egress_port_set(int unit, bcm_port_t port,
                            const bcm_mac_t mac, int untag,
                            bcm_vlan_t vid, int ttl)
{
    uint32      cfg2;

    if (!SOC_PBMP_PORT_VALID(port) || !IS_PORT(unit, port)) {
        return BCM_E_BADID;
    }

    SOC_IF_ERROR_RETURN(READ_EGR_IPMC_CFG2r(unit, port, &cfg2));

    soc_reg_field_set(unit, EGR_IPMC_CFG2r, &cfg2,
                      UNTAGf, untag ? 1 : 0);
    soc_reg_field_set(unit, EGR_IPMC_CFG2r, &cfg2,
                      VIDf, vid);

    
    SOC_IF_ERROR_RETURN(WRITE_EGR_IPMC_CFG2r(unit, port, cfg2));

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_tr_ipmc_egress_port_get
 * Purpose:
 *      Return the IP Multicast egress properties
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      port - port to config.
 *      mac - (OUT) MAC address.
 *      untag - (OUT) 1: The IP multicast packet is transmitted as
 *                       untagged packet.
 *                    0: The IP multicast packet is transmitted as tagged
 *                       packet with VLAN tag vid.
 *      vid - (OUT) VLAN ID.
 *      ttl_thresh - (OUT) Drop IPMC packets if TTL <= ttl_thresh.
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_tr_ipmc_egress_port_get(int unit, bcm_port_t port, sal_mac_addr_t mac,
                            int *untag, bcm_vlan_t *vid, int *ttl_thresh)
{
    uint32              cfg2;

    if (!SOC_PBMP_PORT_VALID(port) || !IS_PORT(unit, port)) {
        return BCM_E_BADID;
    }

    SOC_IF_ERROR_RETURN(READ_EGR_IPMC_CFG2r(unit, port, &cfg2));

    *untag = soc_reg_field_get(unit, EGR_IPMC_CFG2r, cfg2, UNTAGf);
    *vid = soc_reg_field_get(unit, EGR_IPMC_CFG2r, cfg2, VIDf);
    *ttl_thresh = -1;

    return BCM_E_NONE;
}

#ifdef BCM_WARM_BOOT_SUPPORT
/*
 * Reload IPMC state on TR class of devices
 */
int
_bcm_tr_ipmc_reinit(int unit)
{
    int i, rv = BCM_E_NONE;
    int ipmc_ptr;
    int l3_min, l3_max;
    _bcm_esw_ipmc_t  *info = IPMC_INFO(unit);
    _bcm_esw_ipmc_l3entry_t *ipmc_l3entry;
    l3_entry_ipv4_multicast_entry_t *l3x_entry, *ipmc_table;
    l3_entry_ipv6_multicast_entry_t *l3x_v6entry, *ipmc6_table;
    ipmc_entry_t *l3_ipmc_entry, *l3_ipmc_table;
    int ipmc_tbl_sz, ipmc6_tbl_sz, l3_ipmc_tbl_sz;
    _bcm_l3_cfg_t l3cfg;
    uint8 flags;

    info->ipmc_group_info = NULL;

    IPMC_GROUP_NUM(unit) = soc_mem_index_count(unit, L3_IPMCm);

    info->ipmc_count = 0;

    info->ipmc_group_info =
        sal_alloc(IPMC_GROUP_NUM(unit) * sizeof(_bcm_esw_ipmc_group_info_t),
                  "IPMC group info");
    if (info->ipmc_group_info == NULL) {
        rv = BCM_E_MEMORY;
        goto ret_err;
    }
    sal_memset(info->ipmc_group_info, 0, 
               IPMC_GROUP_NUM(unit) * sizeof(_bcm_esw_ipmc_group_info_t));


    l3_min = soc_mem_index_min(unit, L3_ENTRY_IPV4_MULTICASTm);
    l3_max = soc_mem_index_max(unit, L3_ENTRY_IPV4_MULTICASTm);
    ipmc_tbl_sz = sizeof(l3_entry_ipv4_multicast_entry_t) * \
        (l3_max - l3_min + 1);
    ipmc_table = soc_cm_salloc(unit, ipmc_tbl_sz, "ipmc tbl dma");
    if (ipmc_table == NULL) {
        rv = BCM_E_MEMORY;
        goto ret_err;
    }

    memset((void *)ipmc_table, 0, ipmc_tbl_sz);
    if ((rv = soc_mem_read_range(unit, L3_ENTRY_IPV4_MULTICASTm, MEM_BLOCK_ANY,
                                 l3_min, l3_max, ipmc_table)) < 0) {
        soc_cm_sfree(unit, ipmc_table);
        goto ret_err;
    }

    for (i = l3_min; i <= l3_max; i++) {
        sal_memset(&l3cfg, 0, sizeof(_bcm_l3_cfg_t));
        l3x_entry = soc_mem_table_idx_to_pointer(unit,
                                                 L3_ENTRY_IPV4_MULTICASTm,
                                          l3_entry_ipv4_multicast_entry_t *,
                                                 ipmc_table, i);

        if (soc_L3_ENTRY_IPV4_MULTICASTm_field32_get(unit, l3x_entry,
                                                     VALID_0f) == 0) {
            continue;
        }

        if (soc_L3_ENTRY_IPV4_MULTICASTm_field32_get(unit, l3x_entry,
                                                     VALID_1f) == 0) {
            continue;
        }

        if (soc_L3_ENTRY_IPV4_MULTICASTm_field32_get(unit, l3x_entry,
                                                     IPMC_0f) == 0) {
            continue;
        }

        if (soc_L3_ENTRY_IPV4_MULTICASTm_field32_get(unit, l3x_entry,
                                                     IPMC_1f) == 0) {
            continue;
        }

        if (soc_L3_ENTRY_IPV4_MULTICASTm_field32_get(unit, l3x_entry,
                                                     V6_0f)) {
            continue;
        }

        if (soc_L3_ENTRY_IPV4_MULTICASTm_field32_get(unit, l3x_entry,
                                                     V6_1f)) {
            continue;
        }

        ipmc_ptr = soc_L3_ENTRY_IPV4_MULTICASTm_field32_get(unit,
                                                l3x_entry, L3MC_INDEXf);
        IPMC_USED_SET(unit, ipmc_ptr);
        l3cfg.l3c_flags |= BCM_L3_IPMC;
        l3cfg.l3c_flags |=  BCM_L3_HIT;
        if (soc_L3_ENTRY_IPV4_MULTICASTm_field32_get(unit,
                                                l3x_entry, RPEf)) {
            l3cfg.l3c_flags |=  BCM_L3_RPE;
        }
        l3cfg.l3c_ipmc_group = soc_L3_ENTRY_IPV4_MULTICASTm_field32_get(unit,
                                                l3x_entry, GROUP_IP_ADDRf);
        l3cfg.l3c_src_ip_addr = soc_L3_ENTRY_IPV4_MULTICASTm_field32_get(unit,
                                                l3x_entry, SOURCE_IP_ADDRf);
        l3cfg.l3c_vid = soc_L3_ENTRY_IPV4_MULTICASTm_field32_get(unit,
                                                l3x_entry, VLAN_IDf);
        l3cfg.l3c_prio = soc_L3_ENTRY_IPV4_MULTICASTm_field32_get(unit,
                                                l3x_entry, PRIf);
        l3cfg.l3c_lookup_class = soc_L3_ENTRY_IPV4_MULTICASTm_field32_get(unit,
                                                l3x_entry, CLASS_IDf);
        rv = _bcm_tr_ipmc_l3entry_list_add(unit, ipmc_ptr, l3cfg);
        if (rv < 0) {
            soc_cm_sfree(unit, ipmc_table);
            goto ret_err;
        }
    }
    soc_cm_sfree(unit, ipmc_table);

    /*
     * Deal with IPv6 multicast table now
     */
    l3_min = soc_mem_index_min(unit, L3_ENTRY_IPV6_MULTICASTm);
    l3_max = soc_mem_index_max(unit, L3_ENTRY_IPV6_MULTICASTm);
    ipmc6_tbl_sz = sizeof(l3_entry_ipv6_multicast_entry_t) * \
        (l3_max - l3_min + 1);

    ipmc6_table = soc_cm_salloc(unit, ipmc6_tbl_sz, "ipmc v6 tbl dma");
    if (ipmc6_table == NULL) {
        rv = BCM_E_MEMORY;
        goto ret_err;
    }

    memset((void *)ipmc6_table, 0, ipmc6_tbl_sz);

    if ((rv = soc_mem_read_range(unit, L3_ENTRY_IPV6_MULTICASTm,
                                 MEM_BLOCK_ANY,
                                 l3_min, l3_max, ipmc6_table)) < 0) {
        soc_cm_sfree(unit, ipmc6_table);
        goto ret_err;
    }

    for (i = l3_min; i <= l3_max; i++) {
        sal_memset(&l3cfg, 0, sizeof(_bcm_l3_cfg_t));

        l3x_v6entry = soc_mem_table_idx_to_pointer(unit,
                                                   L3_ENTRY_IPV6_MULTICASTm,
                                        l3_entry_ipv6_multicast_entry_t *,
                                                   ipmc6_table, i);
        if (soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(unit, l3x_v6entry,
                                                     VALID_0f) == 0) {
            continue;
        }

        if (soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(unit, l3x_v6entry,
                                                     VALID_1f) == 0) {
            continue;
        }

        if (soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(unit, l3x_v6entry,
                                                     VALID_2f) == 0) {
            continue;
        }

        if (soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(unit, l3x_v6entry,
                                                     VALID_3f) == 0) {
            continue;
        }

        if (soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(unit, l3x_v6entry,
                                                     V6_0f) == 0) {
            continue;
        }

        if (soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(unit, l3x_v6entry,
                                                     V6_1f) == 0) {
            continue;
        }

        if (soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(unit, l3x_v6entry,
                                                     V6_2f) == 0) {
            continue;
        }

        if (soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(unit, l3x_v6entry,
                                                     V6_3f) == 0) {
            continue;
        }

        if (soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(unit, l3x_v6entry,
                                                     IPMC_0f) == 0) {
            continue;
        }

        if (soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(unit, l3x_v6entry,
                                                     IPMC_1f) == 0) {
            continue;
        }

        if (soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(unit, l3x_v6entry,
                                                     IPMC_2f) == 0) {
            continue;
        }

        if (soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(unit, l3x_v6entry,
                                                     IPMC_3f) == 0) {
            continue;
        }

        ipmc_ptr = soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(unit, l3x_v6entry,
                                                            L3MC_INDEX_0f);
        IPMC_USED_SET(unit, ipmc_ptr);
        l3cfg.l3c_flags |= BCM_IPMC_IP6;
        l3cfg.l3c_flags |= BCM_L3_IPMC;
        l3cfg.l3c_flags |=  BCM_L3_HIT;
        if (soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(unit,
                                                l3x_v6entry, RPEf)) {
            l3cfg.l3c_flags |=  BCM_L3_RPE;
        }
        /* Get group address. */
        soc_mem_ip6_addr_get(unit, L3_ENTRY_IPV6_MULTICASTm, l3x_v6entry, 
                             GROUP_IP_ADDR_LWR_64f, l3cfg.l3c_ip6, 
                             SOC_MEM_IP6_LOWER_ONLY);

        soc_mem_ip6_addr_get(unit, L3_ENTRY_IPV6_MULTICASTm, l3x_v6entry, 
                             GROUP_IP_ADDR_UPR_56f, l3cfg.l3c_ip6, 
                             SOC_MEM_IP6_UPPER_ONLY);

        /* Get source  address. */
        soc_mem_ip6_addr_get(unit, L3_ENTRY_IPV6_MULTICASTm, l3x_v6entry, 
                             SOURCE_IP_ADDR_LWR_64f, l3cfg.l3c_sip6, 
                             SOC_MEM_IP6_LOWER_ONLY);
        soc_mem_ip6_addr_get(unit, L3_ENTRY_IPV6_MULTICASTm, l3x_v6entry, 
                             SOURCE_IP_ADDR_UPR_64f, l3cfg.l3c_sip6, 
                             SOC_MEM_IP6_UPPER_ONLY);

        l3cfg.l3c_ip6[0] = 0xff;    /* Set entry to multicast*/ 
        l3cfg.l3c_vid = soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(unit,
                                                l3x_v6entry, VLAN_IDf);
        l3cfg.l3c_prio = soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(unit,
                                                l3x_v6entry, PRIf);
        l3cfg.l3c_lookup_class = soc_L3_ENTRY_IPV6_MULTICASTm_field32_get(unit,
                                                l3x_v6entry, CLASS_IDf);
        rv = _bcm_tr_ipmc_l3entry_list_add(unit, ipmc_ptr, l3cfg);
        if (rv < 0) {
            soc_cm_sfree(unit, ipmc6_table);
            goto ret_err;
        }
    }
    soc_cm_sfree(unit, ipmc6_table);

    /* Recover multicast mode from HW cache */
    rv = _bcm_esw_ipmc_repl_wb_flags_get(unit,
                                         _BCM_IPMC_WB_MULTICAST_MODE,
                                         &flags);

    if (flags) {
        /*
         * Increase reference count for all defined mulitcast groups now
         */
        l3_min = soc_mem_index_min(unit, L3_IPMCm);
        l3_max = soc_mem_index_max(unit, L3_IPMCm);
        l3_ipmc_tbl_sz = sizeof(ipmc_entry_t) * \
            (l3_max - l3_min + 1);

        l3_ipmc_table = soc_cm_salloc(unit, l3_ipmc_tbl_sz,
                                      "L3 ipmc tbl dma");
        if (l3_ipmc_table == NULL) {
            rv = BCM_E_MEMORY;
            goto ret_err;
        }

        memset((void *)l3_ipmc_table, 0, l3_ipmc_tbl_sz);

        if ((rv = soc_mem_read_range(unit, L3_IPMCm,
                                     MEM_BLOCK_ANY,
                                     l3_min, l3_max, l3_ipmc_table)) < 0) {
            soc_cm_sfree(unit, l3_ipmc_table);
            goto ret_err;
        }

        for (i = l3_min; i <= l3_max; i++) {
            l3_ipmc_entry = soc_mem_table_idx_to_pointer(unit,
                                                         L3_IPMCm,
                                                         ipmc_entry_t *,
                                                         l3_ipmc_table, i);
            if (0 == soc_L3_IPMCm_field32_get(unit, l3_ipmc_entry,
                                              VALIDf)) {
                continue;
            }
        
            /* It's a multicast group we need to note. */
            IPMC_USED_SET(unit, i);
        }
        soc_cm_sfree(unit, l3_ipmc_table);
    }

    /*
     * Recover replication state
     */
#ifdef BCM_TRIUMPH2_SUPPORT
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
            SOC_IS_VALKYRIE2(unit) || SOC_IS_TRIDENT(unit) ||
            SOC_IS_KATANA(unit)) {
        rv = _bcm_tr2_ipmc_repl_reload(unit);
    } else
#endif /* BCM_TRIUMPH2_SUPPORT */
    {
        rv = _bcm_xgs3_ipmc_repl_reload(unit);
    }

ret_err:
    if (BCM_FAILURE(rv)) {

        if (info->ipmc_group_info != NULL) {
            for (i = 0; i < IPMC_GROUP_NUM(unit); i++) {
                ipmc_l3entry = IPMC_GROUP_INFO(unit, i)->l3entry_list;
                while (ipmc_l3entry != NULL) {
                    IPMC_GROUP_INFO(unit, i)->l3entry_list = ipmc_l3entry->next;
                    sal_free(ipmc_l3entry);
                    ipmc_l3entry = IPMC_GROUP_INFO(unit, i)->l3entry_list;
                }
            }
            sal_free(info->ipmc_group_info);
            info->ipmc_group_info = NULL;
        }
    } else {
        info->ipmc_initialized = TRUE;
    }

    return rv;
}
#endif /* BCM_WARM_BOOT_SUPPORT */

#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
/*
 * Function:
 *     _bcm_tr_ipmc_sw_dump
 * Purpose:
 *     Displays IPMC information maintained by software.
 * Parameters:
 *     unit - Device unit number
 * Returns:
 *     None
 */
void
_bcm_tr_ipmc_sw_dump(int unit)
{
    int                   i, j;
    _bcm_esw_ipmc_t      *ipmc_info;
    _bcm_esw_ipmc_l3entry_t *ipmc_l3entry;


    /*
     * xgs3_ipmc_info
     */

    ipmc_info = IPMC_INFO(unit);

    soc_cm_print("  XGS3 IPMC Info -\n");
    soc_cm_print("    Init        : %d\n", ipmc_info->ipmc_initialized);
    soc_cm_print("    Size        : %d\n", IPMC_GROUP_NUM(unit));
    soc_cm_print("    Count       : %d\n", ipmc_info->ipmc_count);

    soc_cm_print("    Alloc index :");
    if (ipmc_info->ipmc_group_info != NULL) {
        for (i = 0, j = 0; i < IPMC_GROUP_NUM(unit); i++) {
            /* If not set, skip print */
            if (!IPMC_USED_ISSET(unit, i)) {
                continue;
            }
            if (!(j % 10)) {
                soc_cm_print("\n    ");
            }
            soc_cm_print("  %5d", i);
            j++;
        }
    }
    soc_cm_print("\n");

    soc_cm_print("    Reference count (index:value) :");
    if (ipmc_info->ipmc_group_info != NULL) {
        for (i = 0, j = 0; i < IPMC_GROUP_NUM(unit); i++) {
            if (!IPMC_USED_ISSET(unit, i)) {
                continue;
            }
            if (!(j % 4)) {
                soc_cm_print("\n    ");
            }
            soc_cm_print("  %5d:%-5d", i, IPMC_GROUP_INFO(unit, i)->ref_count);
            j++;
        }
    }
    soc_cm_print("\n");

    soc_cm_print("    IP6 (index:value) :");
    if (ipmc_info->ipmc_group_info != NULL) {
        for (i = 0, j = 0; i < IPMC_GROUP_NUM(unit); i++) {
            ipmc_l3entry = IPMC_GROUP_INFO(unit, i)->l3entry_list;
            while (ipmc_l3entry != NULL) {
                if (ipmc_l3entry->ip6 == 1) {
                    if (!(j % 4)) {
                        soc_cm_print("\n    ");
                    }
                    soc_cm_print("  %5d:%-5d", i, ipmc_l3entry->ip6);
                    j++;
                }
                ipmc_l3entry = ipmc_l3entry->next;
            }
        }
    }
    soc_cm_print("\n");

    /* IPMC replication info is elsewhere */
#ifdef BCM_TRIUMPH2_SUPPORT
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || 
            SOC_IS_VALKYRIE2(unit) || SOC_IS_TRIDENT(unit) ||
            SOC_IS_KATANA(unit)) {
        _bcm_tr2_ipmc_repl_sw_dump(unit);
    } else
#endif /* BCM_TRIUMPH2_SUPPORT */
    {
        _bcm_xgs3_ipmc_repl_sw_dump(unit);
    }

    return;
}
#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */

#endif  /* INCLUDE_L3 */

int _bcm_tr_firebolt_ipmc_not_empty;
