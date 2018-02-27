/*
 * $Id: acl_filter.c 1.18 Broadcom SDK $
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
 * File:        acl_filter.c
 *
 * Purpose:     
 *     Fast Filter Processor specific functionality to ACL.
 */

#ifdef INCLUDE_ACL 

#include <shared/alloc.h>
#include <assert.h>

#include <bcm/error.h>
#include <bcm/types.h>
#include <bcmx/filter.h>
#include <bcmx/port.h>

#include <appl/acl/acl.h>

#include "acl_util.h"
#include "acl_filter.h"

/* Macros */

/*
 * Macro:
 *     ACL_FILTER_IS_INIT (internal)
 *
 * Purpose:
 *     Confirm that the ACL filter functions
 *     are initialized.
 *
 * Notes:
 *     Results in return(BCM_E_UNAVAIL), or return(BCM_E_INIT) if fails.
 */
#define ACL_FILTER_IS_INIT()                                            \
    if (acl_filter_control== NULL) {                                    \
        ACL_ERR(("ACL Error: ACL Filter not initialized\n"));           \
        return BCM_E_INIT;                                              \
    }

/*
 * Macro:
 *     ACL_FILTER_LOCK/ACL_FILTER_UNLOCK  (internal)
 * Purpose:
 *     Lock the ACL Filter module.
 */
#define ACL_FILTER_LOCK() \
    sal_mutex_take(acl_filter_control->lock, sal_mutex_FOREVER)

#define ACL_FILTER_UNLOCK(_c_) \
    sal_mutex_give((_c_)->lock)

/*
 * Macro:
 *     ACL_FFP_IF_ERROR_RETURN
 * Purpose:
 *     Test return value. On error, unlock ACL Filter module and return.
 */
#define ACL_FFP_IF_ERROR_RETURN(op)                  \
    do { int __rv__;                                 \
	if ((__rv__ = (op)) < 0) {                   \
	    ACL_FILTER_UNLOCK(acl_filter_control);   \
	    return(__rv__);                          \
	}                                            \
    } while(0)


/* Local typedefs */

/*
 * Typedef: _acl_filter_t
 *
 * Purpose:
 *    Node in list of filter IDs.
 *
 * Fields:
 *     bcm_filterid_t fid  - Filter ID
 *     _acl_filter_t *next - Link to next filter
 * 
 */
typedef struct _acl_filter_s {
    bcm_filterid_t        fid;       /* Filter ID                 */
    bcma_acl_rule_t       *rule;     /* Corresponding rule        */
    int                   prio;      /* Filter hardware priority  */
    struct _acl_filter_s  *next;     /* linked list pointer       */
} _acl_filter_t;

/*
 * Typedef: _acl_filter_control_t
 *
 * Purpose:
 *    ACL device specific Filter information
 *
 * Fields:
 *    _acl_filter_t *filters - Linked-list of filters
 * 
 */
typedef struct acl_filter_control_s {
    struct _acl_filter_s  *filters;
    sal_mutex_t           lock;       /* System lock  */
} _acl_filter_control_t;

/* Local Prototypes */
STATIC int _acl_filter_create(bcma_acl_t *list, bcma_acl_rule_t *rule,
                              int prio);

STATIC _acl_filter_t *_acl_filter_alloc(bcma_acl_rule_t *rule);
STATIC int _acl_filter_actions_add(bcma_acl_action_t *act, bcm_filterid_t fid);
STATIC int _acl_filter_qualify_vlan(bcma_acl_rule_t *rule,
                                    _acl_filter_t *filter);
STATIC int _acl_filter_qualify_l4srcport(bcma_acl_rule_t *rule,
                                         _acl_filter_t *filter);
STATIC int _acl_filter_qualify_l4dstport(bcma_acl_rule_t *rule,
                                         _acl_filter_t *filter);

STATIC int _acl_filter_find(bcma_acl_rule_id_t rule_id, bcm_filterid_t *fid);
STATIC int _acl_filter_remove(bcm_filterid_t fid) ;

    /* ACL Filter Control Globals */
static _acl_filter_control_t *acl_filter_control;


/* ACL Filter Function Definitions */

/*
 * Function: _acl_filter_init
 *
 * Purpose:
 *     Constructor for the ACL Filter sub-module.
 *
 * Parameters:
 *     none
 *
 * Returns:
 *     BCM_E_NONE   - Success
 *     BCM_E_MEMORY - Allocation failure
 */
int
_acl_filter_init(void) 
{
    int                    retval = BCM_E_NONE;
    _acl_filter_control_t  *filter_control;

    ACL_VVERB(("ACL _acl_filter_init()\n"));

    /* Detach first if it has been previously initialized. */
    if (acl_filter_control != NULL) {
        BCM_IF_ERROR_RETURN(_acl_filter_detach());
    }

    BCM_IF_ERROR_RETURN(bcmx_filter_init());

    /* Allocate and initalize Filter Control struct. */
    filter_control = sal_alloc(sizeof(_acl_filter_control_t),
                              "ACL Filter Control");
    if (filter_control == NULL) {
        ACL_ERR(("ACL Error: allocation failure for ACL Filter control.\n"));
        return BCM_E_MEMORY;
    }
    sal_memset(filter_control, 0, sizeof(_acl_filter_control_t));

    filter_control->lock = sal_mutex_create("ACL_filter_control.lock");
    if (filter_control->lock == NULL) {
        sal_free(filter_control);
        ACL_ERR(("ACL Error: creation failure for ACL filter lock.\n"));
        return BCM_E_MEMORY;
    }

    acl_filter_control = filter_control;

    return retval;
}

/*
 * Function: _acl_filter_detach
 *
 * Purpose:
 *     ACL Filter module destructor
 *
 * Parameters:
 *     none
 *
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_INIT - ACL filter module not initialized
 */
int
_acl_filter_detach(void)
{
    _acl_filter_control_t   *control;
    
    ACL_VVERB(("ACL _acl_filter_detach()\n"));
    ACL_FILTER_IS_INIT();

    if (acl_filter_control == NULL) {
        ACL_ERR(("ACL Error: detaching that is not initialized.\n"));
        return BCM_E_PARAM;
    }

    ACL_FILTER_LOCK();
    control            = acl_filter_control;
    acl_filter_control = NULL;

    /* Deallocate filters */
    while (control->filters != NULL) {
        if (BCM_FAILURE(_acl_filter_remove(control->filters->fid))) {
            break;
        }
    }

    ACL_FILTER_UNLOCK(control);
    sal_mutex_destroy(control->lock);
    sal_free(control);

    return BCM_E_NONE;
}

/*
 * Function: _acl_filter_merge
 *
 * Purpose:
 *     Generate a hardware solution but do not install it.
 *
 * Parameters:
 *     none
 *
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_INIT - ACL filter module not initialized
 */
int
_acl_filter_merge(_acl_control_t *acl_control)
{
    int                   retval = BCM_E_NONE;
    _acl_link_t           *acl_link;
    bcma_acl_rule_t       *rule;
    int                   priority;

    ACL_VVERB(("ACL _acl_filter_merge()\n"));
    ACL_FILTER_IS_INIT();

    assert(acl_control != NULL);

    /* For each ACL */
    for (acl_link = _acl_first(acl_control);
         acl_link != NULL;
         acl_link = _acl_next(acl_control))
    {
        priority = BCM_FILTER_QUALIFY_PRIO_MAX;
        /* For each rule in list...*/
        for (rule = _acl_rule_first(acl_link);
             rule != NULL;
             rule = _acl_rule_next(acl_link))
        {
            BCM_IF_ERROR_RETURN(
                _acl_filter_create(acl_link->list, rule, priority--));
        }
    }

    return retval;
}

const bcm_mac_t _bcma_acl_mac_all_ones = _BCMA_ACL_MAC_ALL_ONES;

#define _ACL_MAC_SRC_OFFSET  6
#define _ACL_MAC_DST_OFFSET  0
#define _ACL_MAC_WIDTH       6

#define _ACL_VLAN_OFFSET    14
#define _ACL_VLAN_WIDTH      2

#define _ACL_AND_IP6(result, param0, param1)   \
        result[0] = param0[0] & param1[0];     \
        result[1] = param0[1] & param1[1];     \
        result[2] = param0[2] & param1[2];     \
        result[3] = param0[3] & param1[3];     \
        result[4] = param0[4] & param1[4];     \
        result[5] = param0[5] & param1[5];     \
        result[6] = param0[6] & param1[6];     \
        result[7] = param0[7] & param1[7];

#define _ACL_IP4_SRC_OFFSET         30
#define _ACL_IP4_DST_OFFSET         34
#define _ACL_IP4_L4_SRC_OFFSET      38
#define _ACL_IP4_L4_DST_OFFSET      40

#define _ACL_IP6_SRC_OFFSET         26
#define _ACL_IP6_DST_OFFSET         42
#define _ACL_IP6_WIDTH              16
#define _ACL_IP6_L4_SRC_OFFSET      58
#define _ACL_IP6_L4_DST_OFFSET      60

#define _ACL_FILTER_ETHER_OFFSET    16
#define _ACL_FILTER_IPPROTO_OFFSET  27
/*
 * Function: _acl_filter_create
 *
 * Purpose:
 *     Create the Filter entries to support a given rule.
 *
 * Parameters:
 *     *control - Filter control data
 *     *list    - List that contains rule
 *     *rule    - ACL rule to use for creating Filter
 *
 * Returns:
 *     BCM_E_NONE     - Success
 *     BCM_E_INTERNAL - No control or No rule?
 *     BCM_E_MEMORY   - Allocation failure
 */
STATIC int
_acl_filter_create(bcma_acl_t *list, bcma_acl_rule_t *rule, int prio)
{
    _acl_filter_t       *filter = NULL;
    bcm_filterid_t      fid;
    bcm_ip_t            ip_data;
    bcm_ip6_t           ip6_data;
    bcmx_lplist_t       lplist_full;

    ACL_VVERB(("ACL _acl_filter_create(rule=%p)\n", (void *)rule));
    if (rule == NULL) {
        ACL_ERR(("ACL Error: rule is NULL.\n"));
        return BCM_E_INTERNAL;
    }

    ACL_FILTER_IS_INIT();
    ACL_FILTER_LOCK();

    /* Allocate a filter ID */
    filter = _acl_filter_alloc(rule);

    ACL_FFP_IF_ERROR_RETURN(bcmx_filter_create(&fid));
    ACL_FFP_IF_ERROR_RETURN(bcmx_filter_qualify_priority(fid, prio));
    filter->fid  = fid;
    filter->prio = prio;

    /* Set the matching criteria */

    /* Limit rule to subset of ports, if specified. */
    bcmx_lplist_init(&lplist_full, 0, 0);
    bcmx_port_lplist_populate(&lplist_full, BCMX_PORT_LP_ALL);
    if (!bcmx_lplist_eq(&list->lplist, &lplist_full)) {
        ACL_FFP_IF_ERROR_RETURN(bcmx_filter_qualify_ingress(fid, list->lplist));
    }

    /* Qualify on Source MAC address */
    if (rule->flags & BCMA_ACL_RULE_SRC_MAC) {
        ACL_FFP_IF_ERROR_RETURN(
            bcmx_filter_qualify_data(fid, _ACL_MAC_SRC_OFFSET, _ACL_MAC_WIDTH,
                                     rule->src_mac,
                                     (uint8*)_bcma_acl_mac_all_ones));
    }
    /* Qualify on Destination MAC address */
    if (rule->flags & BCMA_ACL_RULE_DST_MAC) {
        ACL_FFP_IF_ERROR_RETURN(
            bcmx_filter_qualify_data(fid, _ACL_MAC_DST_OFFSET, _ACL_MAC_WIDTH,
                                     rule->dst_mac,
                                     (uint8*)_bcma_acl_mac_all_ones));
    }

    /* Qualify on Source IPv4 address */
    if (rule->flags & BCMA_ACL_RULE_SRC_IP4) {
        /* Clear masked bits from data field. */
        ip_data = rule->src_ip & rule->src_ip_mask;

        ACL_FFP_IF_ERROR_RETURN(
            bcmx_filter_qualify_data32(fid, _ACL_IP4_SRC_OFFSET, ip_data,
                                       rule->src_ip_mask));
    }

    /* Qualify on Destination IPv4 address */
    if (rule->flags & BCMA_ACL_RULE_DST_IP4) {
        /* Clear masked bits from data field. */
        ip_data = rule->dst_ip & rule->dst_ip_mask;

        ACL_FFP_IF_ERROR_RETURN(
            bcmx_filter_qualify_data32(fid, _ACL_IP4_DST_OFFSET , ip_data,
                                       rule->dst_ip_mask));
    }

    /* Qualify on Source IPv6 address */
    if (rule->flags & BCMA_ACL_RULE_SRC_IP6) {
        /* Clear masked bits from data field. */
        _ACL_AND_IP6(ip6_data, rule->src_ip6, rule->src_ip6_mask);

        ACL_FFP_IF_ERROR_RETURN(
            bcmx_filter_qualify_data(fid, _ACL_IP6_SRC_OFFSET, _ACL_IP6_WIDTH,
                                     ip6_data, rule->src_ip6_mask));
    }

    /* Qualify on Destination IPv6 address */
    if (rule->flags & BCMA_ACL_RULE_DST_IP6) {
        /* Clear masked bits from data field. */
        _ACL_AND_IP6(ip6_data, rule->dst_ip6, rule->dst_ip6_mask);

        ACL_FFP_IF_ERROR_RETURN(
            bcmx_filter_qualify_data(fid, _ACL_IP6_SRC_OFFSET, _ACL_IP6_WIDTH,
                                     ip6_data, rule->dst_ip6_mask));
    }

    /* Qualify on Ether Type */
    if (rule->flags & BCMA_ACL_RULE_ETHERTYPE) {
         ACL_FFP_IF_ERROR_RETURN(
            bcmx_filter_qualify_data16(fid, _ACL_FILTER_ETHER_OFFSET,
                                       rule->ether_type, ACL_MASK_ETHERTYPE));
    }

    /* Qualify on IP Protocol */
    if (rule->flags & BCMA_ACL_RULE_IPPROTOCOL) {
         ACL_FFP_IF_ERROR_RETURN(
            bcmx_filter_qualify_data8(fid, _ACL_FILTER_IPPROTO_OFFSET,
                                      rule->ip_protocol, ACL_MASK_IPPROTOCOL));
    }

    /* Qualify on VLAN range */
    if (rule->flags & BCMA_ACL_RULE_VLAN) {
        ACL_FFP_IF_ERROR_RETURN(_acl_filter_qualify_vlan(rule, filter));
    }

    /* Qualify on L4 source port range */
    if (rule->flags & BCMA_ACL_RULE_L4_SRC_PORT) {
        filter = acl_filter_control->filters;
        while(filter != NULL) {
            if (rule->rule_id == filter->rule->rule_id) {
                ACL_FFP_IF_ERROR_RETURN(
                    _acl_filter_qualify_l4srcport(rule, filter));
            }
            filter = filter->next;
        }
    }

    /* Qualify on L4 destination port range */
    if (rule->flags & BCMA_ACL_RULE_L4_DST_PORT) {
        filter = acl_filter_control->filters;
        while(filter != NULL) {
            if (rule->rule_id == filter->rule->rule_id) {
                ACL_FFP_IF_ERROR_RETURN(
                    _acl_filter_qualify_l4dstport(rule, filter));
            }
            filter = filter->next;
        }
    }

    /* Add actions to all Filter IDs that support this Rule. */
    filter = acl_filter_control->filters;
    while(filter != NULL) {
        if (rule->rule_id == filter->rule->rule_id) {
            _acl_filter_actions_add(&rule->actions, filter->fid);
        }
        filter = filter->next;
    }

    ACL_FILTER_UNLOCK(acl_filter_control);

    return BCM_E_NONE;
}

/*
 * Function: _acl_filter_alloc
 *
 * Purpose:
 *     allocate a new ACL filter in both filter and ACL
 *
 * Parameters:
 *     *rule           - ACL rule that this filter supports
 *
 * Returns:
 *     BCM_E_NONE     - Success
 *     BCM_E_INTERNAL - No control or No rule?
 *     BCM_E_MEMORY   - Allocation failure
 */
STATIC _acl_filter_t *
_acl_filter_alloc(bcma_acl_rule_t *rule)
{
    _acl_filter_t       *filter;

    assert(rule               != NULL);
    assert(acl_filter_control != NULL);

    /* Allocate a filter ID */
    filter = sal_alloc(sizeof(_acl_filter_t), "ACL Filter");
    if (filter == NULL) {
        ACL_ERR(("ACL Error: allocation failure for ACL Filter.\n"));
        return NULL;
    }
    sal_memset(filter, 0, sizeof(_acl_filter_t));

    ACL_FILTER_LOCK();
    /* Put new filter at head of list for this control */
    filter->next = acl_filter_control->filters;
    filter->rule = rule;
    acl_filter_control->filters = filter;
    ACL_FILTER_UNLOCK(acl_filter_control);

    return filter;
}

/*
 * Function: _acl_filter_qualify_vlan
 *
 * Purpose:
 *
 * Parameters:
 *     *rule    - ACL rule to use for creating Filter
 *
 * Returns:
 *     BCM_E_NONE     - Success
 *     BCM_E_INTERNAL - No control or No rule?
 *     BCM_E_MEMORY   - Allocation failure
 */
STATIC int
_acl_filter_qualify_vlan(bcma_acl_rule_t *rule,
                         _acl_filter_t *filter)
{
    acl_node_t      *list;
    acl_node_t      *node_cur;
    int             count;
    int             idx;
    bcm_filterid_t  fid_copy;
    _acl_filter_t  *filter_copy;

    assert(rule    != NULL);
    assert(filter  != NULL);

    ACL_VVERB(("ACL _acl_filer_entry_qualify_vlan(rule_id=%d, fid=%d)\n",
               rule->rule_id, filter->fid));

    /* Get list of range data/mask pairs. */
    acl_range_to_list(rule->vlan_min, rule->vlan_max, &list, &count);

    node_cur = list->next;

    for (idx = 1; idx < count; idx++, node_cur = node_cur->next) {
        /* Allocate new acl_filter. */
        filter_copy = _acl_filter_alloc(rule);
        if (filter_copy == NULL) {
            ACL_ERR(("ACL Error: Filter allocation failure.\n"));
            return BCM_E_MEMORY;
        }

        /* Copy filter ID. */
        BCM_IF_ERROR_RETURN(
            bcmx_filter_copy(filter->fid, &fid_copy));
        filter_copy->fid  = fid_copy;
        BCM_IF_ERROR_RETURN(
             bcmx_filter_qualify_priority(fid_copy, filter->prio));
        filter_copy->prio = filter->prio;

        ACL_VERB(("ACL Filter: Qualifying fid=%d VLAN data=%#x, mask=%#x\n",
                  filter_copy->fid, node_cur->data, node_cur->mask));
        /* Qualify on VLAN. */
        BCM_IF_ERROR_RETURN(
            bcmx_filter_qualify_data(filter_copy->fid, _ACL_VLAN_OFFSET,
                                     _ACL_VLAN_WIDTH, 
                                     (uint8*)&node_cur->data,
                                     (uint8*)&node_cur->mask));
    }

    /* Qualify on first list element on filter ID. */
    BCM_IF_ERROR_RETURN(
        bcmx_filter_qualify_data(filter->fid, _ACL_VLAN_OFFSET,
                                 _ACL_VLAN_WIDTH, 
                                 (uint8*)&list->data,
                                 (uint8*)&list->mask));

    ACL_VERB(("ACL Filter: Qualifying fid=%d VLAN data=%#x, mask=%#x\n",
        filter->fid, list->data, list->mask));

    return BCM_E_NONE;
}

/*
 * Function: _acl_filter_qualify_l4srcport
 *
 * Purpose:
 *     Write the Source L4 Port range into hardware.
 *
 * Parameters:
 *
 * Returns:
 *     BCM_E_NONE      - Success
 */
STATIC int
_acl_filter_qualify_l4srcport(bcma_acl_rule_t *rule, _acl_filter_t *filter)
{
    acl_node_t      *list;
    acl_node_t      *node_cur;
    int             count;
    int             idx;
    int             retval;
    int             offset = 0;
    bcm_filterid_t  fid_copy;
    _acl_filter_t  *filter_copy;

    ACL_VVERB(("ACL _acl_filter_qualify_l4srcport(rule_id=%d, fid=%d)\n",
               rule->rule_id, filter->fid));

    /* Determine the offset based on packet type. */
    if (rule->flags & BCMA_ACL_RULE_ETHERTYPE) {
        switch (rule->ether_type) {
            case 0x0800:
                offset = _ACL_IP4_L4_SRC_OFFSET;
                break;
            case 0x86dd:
                offset = _ACL_IP6_L4_SRC_OFFSET;
                break;
            default:
                ACL_ERR(("ACL Error: bad ethertype=0x%04x\n", rule->ether_type));
                return BCM_E_PARAM;
        }
    } else if (rule->flags & BCMA_ACL_RULE_SRC_IP4) {
                offset = _ACL_IP4_L4_SRC_OFFSET;
    } else if (rule->flags & BCMA_ACL_RULE_SRC_IP6) {
                offset = _ACL_IP6_L4_SRC_OFFSET;
    } else {
        ACL_ERR(("ACL Error: no way to tell packet type in L4 packet\n"));
        return BCM_E_PARAM;
    }

    /* Get list of range data/mask pairs. */
    acl_range_to_list(rule->src_port_min, rule->src_port_max, &list, &count);

    node_cur = list; 
    node_cur = node_cur->next;

    for (idx = 1; idx < count; idx++) {
        /* Allocate new acl_filter. */
        filter_copy = _acl_filter_alloc(rule);
        if (filter_copy == NULL) {
            acl_range_destroy(list, count);
            ACL_ERR(("ACL Error: Filter allocation failure.\n"));
            return BCM_E_MEMORY;
        }

        /* Copy filter ID. */
        retval = bcmx_filter_copy(filter->fid, &fid_copy);
        if (BCM_FAILURE(retval)) {
            acl_range_destroy(list, count);
            sal_free(filter_copy);
            return retval;
        }

        filter_copy->fid  = fid_copy;
        retval = bcmx_filter_qualify_priority(fid_copy, filter->prio);
        if (BCM_FAILURE(retval)) {
            acl_range_destroy(list, count);
            sal_free(filter_copy);
            return retval;
        }

        filter_copy->prio = filter->prio;

        /* Qualify on entries with each data/mask pair from list. */
        retval = bcmx_filter_qualify_data16(fid_copy, offset,
                                            node_cur->data, node_cur->mask);
        if (BCM_FAILURE(retval)) {
            acl_range_destroy(list, count);
            sal_free(filter_copy);
            return retval;
        }

        node_cur = node_cur->next;
    }

    /* Qualify on first element with entry that already exists. */
    BCM_IF_ERROR_RETURN(
        bcmx_filter_qualify_data16(filter->fid, offset,
                                   list->data, list->mask));

    /* Destroy list */
    return acl_range_destroy(list, count);
}

/*
 * Function: _acl_filter_qualify_l4dstport
 *
 * Purpose:
 *     Write the Destination L4 Port range into hardware.
 *
 * Parameters:
 *
 * Returns:
 *     BCM_E_NONE      - Success
 */
STATIC int
_acl_filter_qualify_l4dstport(bcma_acl_rule_t *rule, _acl_filter_t *filter)
{
    acl_node_t      *list;
    acl_node_t      *node_cur;
    int             count;
    int             idx;
    int             retval;
    int             offset = 0;
    bcm_filterid_t  fid_copy;
    _acl_filter_t  *filter_copy;

    ACL_VVERB(("ACL _acl_filter_qualify_l4dstport(rule_id=%d, fid=%d)\n",
               rule->rule_id, filter->fid));

    /* Determine the offset based on packet type. */
    if (rule->flags & BCMA_ACL_RULE_ETHERTYPE) {
        switch (rule->ether_type) {
            case 0x0800:
                offset = _ACL_IP4_L4_DST_OFFSET;
                break;
            case 0x86dd:
                offset = _ACL_IP6_L4_DST_OFFSET;
                break;
            default:
                ACL_ERR(("ACL Error: bad ethertype=0x%04x\n", rule->ether_type));
                return BCM_E_PARAM;
        }
    } else if (rule->flags & BCMA_ACL_RULE_DST_IP4) {
                offset = _ACL_IP4_L4_DST_OFFSET;
    } else if (rule->flags & BCMA_ACL_RULE_DST_IP6) {
                offset = _ACL_IP6_L4_DST_OFFSET;
    } else {
        ACL_ERR(("ACL Error: no way to tell packet type in L4 packet\n"));
        return BCM_E_PARAM;
    }

    /* Get list of range data/mask pairs. */
    acl_range_to_list(rule->dst_port_min, rule->dst_port_max, &list, &count);

    node_cur = list; 
    node_cur = node_cur->next;

    for (idx = 1; idx < count; idx++) {
        /* Allocate new acl_filter. */
        filter_copy = _acl_filter_alloc(rule);
        if (filter_copy == NULL) {
            acl_range_destroy(list, count);
            ACL_ERR(("ACL Error: Filter allocation failure.\n"));
            return BCM_E_MEMORY;
        }

        /* Copy filter ID. */
        retval = bcmx_filter_copy(filter->fid, &fid_copy);
        if (BCM_FAILURE(retval)) {
            acl_range_destroy(list, count);
            sal_free(filter_copy);
            return retval;
	}

        filter_copy->fid  = fid_copy;
        retval = bcmx_filter_qualify_priority(fid_copy, filter->prio);
        if (BCM_FAILURE(retval)) {
            acl_range_destroy(list, count);
            sal_free(filter_copy);
            return retval;
	}

        filter_copy->prio = filter->prio;

        /* Qualify on entries with each data/mask pair from list. */
        retval = bcmx_filter_qualify_data16(fid_copy, offset,
                                            node_cur->data, node_cur->mask);
        if (BCM_FAILURE(retval)) {
            acl_range_destroy(list, count);
            sal_free(filter_copy);
            return retval;
	}

        node_cur = node_cur->next;
    }

    /* Qualify on first element with entry that already exists. */
    BCM_IF_ERROR_RETURN(
        bcmx_filter_qualify_data16(filter->fid, offset,
                                   list->data, list->mask));

    /* Destroy list */
    return acl_range_destroy(list, count);
}

/*
 * Function: _acl_filter_actions_add
 *
 * Purpose:
 *     Add the rule's actions to all supporting filters IDs.
 *
 * Parameters:
 *     action - actions to add
 *     fid  - filter to add actions to
 *
 * Returns:
 *     BCM_E_NONE      - Success
 *     BCM_E_xxx       - From Filter qualify APIs
 */
STATIC int
_acl_filter_actions_add(bcma_acl_action_t *action, bcm_filterid_t fid)
{
    assert(action != NULL);

    if (action->flags & BCMA_ACL_ACTION_DENY) {
        BCM_IF_ERROR_RETURN(
            bcmx_filter_action_match(fid, bcmActionDoNotSwitch, 0));
    }

    if (action->flags & BCMA_ACL_ACTION_REDIR) {
        BCM_IF_ERROR_RETURN(
            bcmx_filter_action_match(fid, bcmActionSetPortAll,
                                     action->redir_port));
    }

    if (action->flags & BCMA_ACL_ACTION_MIRROR) {
        BCM_IF_ERROR_RETURN(
            bcmx_filter_action_match(fid, bcmActionCopyToMirror,
                                     action->mirror_port));
    }

    if (action->flags & BCMA_ACL_ACTION_LOG) {
        BCM_IF_ERROR_RETURN(bcmx_filter_action_match(fid, bcmActionCopyToCpu,
                                                     0));
    }

    /* Do nothing for Permit. It's enough to have the filter match and
     * prevent lower priority filters from matching. */

    return BCM_E_NONE;
}

/*
 * Function: _acl_filter_install
 *
 * Purpose:
 *     Install the Filter hardware solution calculated by _acl_filter_merge().
 *
 * Parameters:
 *     none
 *
 * Returns:
 *     BCM_E_NONE      - Success
 *     BCM_E_INIT      - ACL filter module not initialized
 *     BCM_E_RESOURCE  - exceeded ability of hardware filter table
 *     BCM_E_FULL      - ran out of masks or rules
 */
int
_acl_filter_install(void)
{
    int                    retval = BCM_E_NONE;
    _acl_filter_t          *filter;

    ACL_VVERB(("ACL _acl_filter_install()\n"));
    ACL_FILTER_IS_INIT();

    ACL_FILTER_LOCK();

    for (filter = acl_filter_control->filters;
         filter != NULL;
         filter = filter->next) {
        retval = bcmx_filter_install(filter->fid);
        if (BCM_FAILURE(retval)) {
            ACL_FILTER_UNLOCK(acl_filter_control);
            ACL_ERR(("ACL Error: Failed to install fid=%d.\n", filter->fid));
            return retval;
        }
    }
    ACL_FILTER_UNLOCK(acl_filter_control);

    return retval;
}

/*
 * Function: _acl_filter_uninstall
 *
 * Purpose:
 *     Remove Filter entries from hardware.
 *
 * Parameters:
 *     none
 *
 * Returns:
 *     BCM_E_NONE      - Success
 *     BCM_E_INIT      - ACL filter module not initialized
 *     BCM_E_NOT_FOUND - Filter ID not found? (internal error)
 */
int
_acl_filter_uninstall(void)
{
    _acl_filter_t          *filter_old;

    ACL_VVERB(("ACL _acl_filter_uninstall()\n"));
    ACL_FILTER_IS_INIT();
    ACL_FILTER_LOCK();

    while (acl_filter_control->filters != NULL) {
        ACL_FFP_IF_ERROR_RETURN(
            bcmx_filter_remove(acl_filter_control->filters->fid));
        /* Clean up memory used by Filter ID. */
        filter_old                   = acl_filter_control->filters; 
        acl_filter_control->filters  = filter_old->next;
        sal_free(filter_old);
    }
    ACL_FILTER_UNLOCK(acl_filter_control);

    return BCM_E_NONE;
}

/*
 * Function: _acl_filter_rule_remove
 *
 * Purpose:
 *     Remove a single rule from hardware. In pratice this may mean removing
 *     several filter entries.
 *
 * Parameters:
 *     rule_id - ACL rule to remove from filter hardware
 *
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_NOT_FOUND - Rule ID not found.
 */
int
_acl_filter_rule_remove(bcma_acl_rule_id_t rule_id)
{
    bcm_filterid_t          fid;
    int                     retval;

    ACL_VVERB(("ACL _acl_filter_rule_remove(rule_id=%d)\n", rule_id));
    ACL_FILTER_IS_INIT();
    ACL_FILTER_LOCK();

    retval = _acl_filter_find(rule_id, &fid);

    if (retval < 0) {
        ACL_FILTER_UNLOCK(acl_filter_control);
        return retval;
    }

    while (BCM_SUCCESS(retval)) {
        ACL_FFP_IF_ERROR_RETURN(_acl_filter_remove(fid));
        retval = _acl_filter_find(rule_id, &fid);
    }
    ACL_FILTER_UNLOCK(acl_filter_control);

    return (retval == BCM_E_NOT_FOUND) ? BCM_E_NONE : retval;
}

/*
 * Function: _acl_filter_find
 *
 * Purpose:
 *     Find a Filter ID that supports the given Rule ID.
 *
 * Parameters:
 *     rule_id - ACL rule to remove from filter hardware
 *     fid - (OUT) One of the Filter IDs that supports the Rule ID.
 *
 * Returns:
 *     BCM_E_NONE      - Success
 *     BCM_E_NOT_FOUND - No Filter ID supports the Rule ID.
 */
STATIC int
_acl_filter_find(bcma_acl_rule_id_t rule_id, bcm_filterid_t *fid)
{
    _acl_filter_t          *filter_cur;

    ACL_VVERB(("ACL _acl_filter_find(rule_id=%d)\n", rule_id));
    ACL_FILTER_IS_INIT();

    assert(fid != NULL);

    ACL_FILTER_LOCK();
    filter_cur = acl_filter_control->filters;

    /* Traverse filter list, looking for matching Filter ID. */
    while (filter_cur != NULL) {
        if (filter_cur->rule->rule_id == rule_id) {
            *fid = filter_cur->fid;
            ACL_FILTER_UNLOCK(acl_filter_control);
            return BCM_E_NONE;
        }
        filter_cur = filter_cur->next;
    }
    ACL_FILTER_UNLOCK(acl_filter_control);

    return BCM_E_NOT_FOUND;
}

/*
 * Function: _acl_filter_remove
 *
 * Purpose:
 *     Remove a Filter ID from hardware and free ACL Filter node.
 *
 * Parameters:
 *     fid - Filter ID to remove from filter hardware
 *
 * Returns:
 *     BCM_E_NONE - Success
 */
STATIC int
_acl_filter_remove(bcm_filterid_t fid) 
{
    _acl_filter_t          *filter_cur, *filter_prev;

    ACL_VVERB(("ACL _acl_filter_remove()\n"));
    ACL_FILTER_IS_INIT();
    ACL_FILTER_LOCK();

    filter_prev = NULL;
    filter_cur  = acl_filter_control->filters;

    /* Traverse filter list, looking for matching Filter ID. */
    while (filter_cur != NULL) {
        if (filter_cur->fid == fid) {
            /* Remove it from the hardware. */
            ACL_FFP_IF_ERROR_RETURN(bcmx_filter_remove(fid));

            /* Remove the ACL Filter node. */
            if (filter_prev == NULL) { 
               /* It was the first node in the list. */
               acl_filter_control->filters = filter_cur->next;
            } else {
               filter_prev->next = filter_cur->next;
            }
            sal_free(filter_cur);
            ACL_FILTER_UNLOCK(acl_filter_control);
            return BCM_E_NONE;
        }

        filter_prev = filter_cur;
        filter_cur = filter_cur->next;
    }
    ACL_FILTER_UNLOCK(acl_filter_control);

    ACL_ERR(("ACL Error: can't remove Filter ID=%d, not found\n", fid));
    return BCM_E_NOT_FOUND;
}

#ifdef BROADCOM_DEBUG

/*
 * Function: _acl_filter_show
 *
 * Purpose:
 *     Dump Filter data.
 *
 * Parameters:
 *     none
 *
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_INIT - ACL filter module not initialized
 */
int
_acl_filter_show(void)
{
    _acl_filter_t          *filter;

    ACL_VVERB(("ACL _acl_filter_show()\n"));
    ACL_FILTER_IS_INIT();
    ACL_FILTER_LOCK();

    ACL_OUT(("acl_filter={\n"));

    for (filter = acl_filter_control->filters;
         filter != NULL ;
         filter = filter->next) {
        ACL_OUT(("\tfilter={fid=%d, rule_id=%d, prio=%d}\n", filter->fid,
                 filter->rule->rule_id, filter->prio));
    }
    ACL_OUT(("}\n"));
    ACL_FILTER_UNLOCK(acl_filter_control);

    return BCM_E_NONE;
}

#endif /* BROADCOM_DEBUG */
#endif /* INCLUDE_ACL */
