/*
 * $Id: l2.c 1.4.216.1 Broadcom SDK $
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
 * L2 - Broadcom StrataSwitch Layer-2 switch API.
 */

#include <soc/cm.h>

#include <bcm/types.h>
#include <bcm/error.h>

#include <soc/l2x.h>
#include <bcm/l2.h>

#include <bcm_int/sbx/mbcm.h>

int
bcm_sbx_l2_init(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l2_detach(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l2_addr_add(int unit,
                    bcm_l2_addr_t *l2addr)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l2_addr_delete(int unit,
                       bcm_mac_t mac,
                       bcm_vlan_t vid)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l2_addr_delete_by_port(int unit,
                               bcm_module_t mod,
                               bcm_port_t port,
                               uint32 flags)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l2_addr_delete_by_mac(int unit,
                              bcm_mac_t mac,
                              uint32 flags)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l2_addr_delete_by_vlan(int unit,
                               bcm_vlan_t vid,
                               uint32 flags)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l2_addr_delete_by_trunk(int unit,
                                bcm_trunk_t tid,
                                uint32 flags)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l2_addr_delete_by_mac_port(int unit,
                                   bcm_mac_t mac,
                                   bcm_module_t mod,
                                   bcm_port_t port,
                                   uint32 flags)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l2_addr_delete_by_vlan_port(int unit,
                                    bcm_vlan_t vid,
                                    bcm_module_t mod,
                                    bcm_port_t port,
                                    uint32 flags)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l2_addr_delete_by_vlan_trunk(int unit,
                                     bcm_vlan_t vid,
                                     bcm_trunk_t tid,
                                     uint32 flags)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l2_addr_delete_by_mac_vpn(int unit,
                                  bcm_mac_t mac, 
                                  bcm_vpn_t vpn)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l2_addr_delete_by_vpn(int unit, bcm_vpn_t vpn, uint32 flags)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l2_addr_get(int unit,
                    sal_mac_addr_t mac,
                    bcm_vlan_t vid,
                    bcm_l2_addr_t *l2addr)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l2_port_native(int unit,
                       int modid,
                       int port)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l2_addr_register(int unit,
                         bcm_l2_addr_callback_t fn,
                         void *fn_data)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l2_addr_unregister(int unit,
                           bcm_l2_addr_callback_t fn,
                           void *fn_data)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l2_age_timer_set(int unit,
                         int age_seconds)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l2_age_timer_get(int unit,
                         int *age_seconds)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l2_addr_freeze(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l2_addr_thaw(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l2_bpdu_count(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l2_bpdu_set(int unit,
                    int index,
                    sal_mac_addr_t addr)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l2_bpdu_get(int unit,
                    int index,
                    sal_mac_addr_t *addr)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *     bcm_l2_key_dump
 * Purpose:
 *     Dump the key (VLAN+MAC) portion of a hardware-independent
 *     L2 address for debugging.
 * Parameters:
 *     unit  - Unit number
 *     pfx   - String to print before output
 *     entry - Hardware-independent L2 entry to dump
 *     sfx   - String to print after output
 * Returns:
 *     BCM_E_NONE  - Success
 *     BCM_E_PARAM - Failure, null param
 */
int
bcm_sbx_l2_key_dump(int unit, char *pfx, bcm_l2_addr_t *entry, char *sfx)
{
    if ((pfx == NULL) || (entry == NULL) || (sfx == NULL)) {
        return BCM_E_PARAM;
    }

    soc_cm_print("l2: %sVLAN=0x%03x MAC=0x%02x%02x%02x"
                 "%02x%02x%02x%s", pfx, entry->vid,
                 entry->mac[0], entry->mac[1], entry->mac[2],
                 entry->mac[3], entry->mac[4], entry->mac[5], sfx);

    return BCM_E_NONE;
}

int
bcm_sbx_l2_conflict_get(int unit,
                        bcm_l2_addr_t *addr,
                        bcm_l2_addr_t *cf_array,
                        int cf_max,
                        int *cf_count)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l2_tunnel_add(int unit,
                      bcm_mac_t mac,
                      bcm_vlan_t vlan)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l2_tunnel_delete(int unit,
                         bcm_mac_t mac,
                         bcm_vlan_t vlan)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l2_tunnel_delete_all(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l2_clear(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l2_learn_limit_set(int unit,
                           bcm_l2_learn_limit_t *limit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l2_learn_limit_get(int unit,
                           bcm_l2_learn_limit_t *limit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l2_learn_class_set(int unit,
                           int class,
                           int class_prio,
                           uint32 flags)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l2_learn_class_get(int unit,
                           int class,
                           int *class_prio,
                           uint32 *flags)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l2_learn_port_class_set(int unit,
                                bcm_module_t mod,
                                bcm_port_t port,
                                int class)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l2_learn_port_class_get(int unit,
                                bcm_module_t mod,
                                bcm_port_t port,
                                int *class)
{
    return BCM_E_UNAVAIL;
}

int bcm_sbx_l2_stat_get(int unit, bcm_l2_addr_t *l2_addr, bcm_l2_stat_t stat, 
                        uint64 *val)
{
    return BCM_E_UNAVAIL;
}

int bcm_sbx_l2_stat_get32(int unit, bcm_l2_addr_t *l2_addr, bcm_l2_stat_t stat,
                          uint32 *val)
{
    return BCM_E_UNAVAIL;
}    

int bcm_sbx_l2_stat_set(int unit, bcm_l2_addr_t *l2_addr, bcm_l2_stat_t stat,
                        uint64 val)
{
    return BCM_E_UNAVAIL;
}

int bcm_sbx_l2_stat_set32(int unit, bcm_l2_addr_t *l2_addr, bcm_l2_stat_t stat,
                          uint32 val)
{
    return BCM_E_UNAVAIL;
}

int bcm_sbx_l2_stat_enable_set(int unit, bcm_l2_addr_t *l2_addr, int enable)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_l2_traverse(int unit,
                    bcm_l2_traverse_cb trav_fn,
                    void *user_data)
{
    return BCM_E_UNAVAIL;
}
int 
bcm_sbx_l2_replace(int unit, uint32 flags, bcm_l2_addr_t *match_addr,
                   bcm_module_t new_module, bcm_port_t new_port, 
                   bcm_trunk_t new_trunk)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_l2_egress_create(int unit, bcm_l2_egress_t *egr)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l2_egress_destroy(int unit, bcm_if_t encap_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l2_egress_get(int unit, bcm_if_t encap_id, bcm_l2_egress_t *egr)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l2_egress_find(int unit, bcm_l2_egress_t *egr, bcm_if_t *encap_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_l2_egress_traverse(int unit, bcm_l2_egress_traverse_cb trav_fn, 
                       void *user_data)
{
    return BCM_E_UNAVAIL;
}
