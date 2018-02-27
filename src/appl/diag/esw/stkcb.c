/* 
 * $Id: stkcb.c 1.8 Broadcom SDK $
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
 * File:        stkcb.c
 * Purpose:     Set up a stack port callback so that traffic received by
 *              an SL stack port can be reprioritized on ingress.
 *
 *              Demonstrates the use of the stack port callback API.
 *
 *              This file requires BCM Filter Support.
 *
 */

#include <sal/types.h>
#include <sal/core/libc.h>
#include <sal/core/thread.h>
#include <sal/core/dpc.h>
#include <sal/appl/sal.h>
#include <sal/appl/io.h>

#include <bcm/topo.h>
#include <bcm/init.h>
#include <bcm/stg.h>
#include <bcm/vlan.h>
#include <bcm/stack.h>
#include <bcm/error.h>
#include <bcm/filter.h>
#include <bcm_int/rpc/rpc.h>
#include <bcm_int/rpc/rlink.h>

#include <appl/diag/shell.h>
#include <appl/diag/system.h>
#include <appl/diag/parse.h>

#if defined(BCM_FILTER_SUPPORT)

#define IGNORE_PRIORITY -1
#define DEFAULT_MASK    -1
#define DEFAULT_COS      7
#define DEFAULT_VID      1

#define CALLBACK_OURS 1
#define CALLBACK_NOT_OURS 2
#define CALLBACK_NOT_DEFINED 3
#define CALLBACK_OTHER_ERROR 4
#define CALLBACK_UNDEFINED_ERROR 5

typedef struct SL_filter_s {
    int unit;                   /* Unit filter is on */
    int port;                   /* Port filter is on */
    int installed;              /* TRUE if filter is installed */
    int is_stack;               /* TRUE if this is a stack filter */
    int counter;                /* FFP counter for filter */
    int pri;                    /* Specific filter parameters */
    int vid;
    int cos;
    int mask;
    bcm_filterid_t filter_id;   /* Filter id */
    struct SL_filter_s *next;   /* Pointer to next filter in chain */
} SL_filter_t;

typedef struct {
    int         verbose;            /* Verbose messages */
    int         pri;                /* Stack Filter parameters */
    int         vid;
    int         cos;
    int         mask;
    SL_filter_t *filter;            /* Chain of stack port filters */
} SL_stk_info_t;

STATIC void
stkcb_callback(int unit, bcm_port_t port, uint32 flags, void *data);


/* Error message helper */
STATIC void
print_err(char *msg, int rc)
{
    printk("%s: (%d) %s\n", msg, rc, bcm_errmsg(rc));
}

/* Add an SL port info struct to the chain of port info. This
   info is saved so it can be removed later. */
STATIC SL_filter_t *
add_SL_filter(SL_stk_info_t *info, int unit, bcm_port_t port)
{
    SL_filter_t *filter, *prev_filter;

    filter = sal_alloc(sizeof(SL_filter_t), "SL_filter");
    if (filter != NULL) {
        /* Init filter struct */
        filter->unit = unit;
        filter->port = port;
        filter->installed = FALSE;
        /* Put in chain */
        prev_filter  = info->filter;
        info->filter  = filter;
        filter->next = prev_filter;
    } else {
        printk("Filter alloc error\n");
    }
    return filter;
}

/* Return port info for a specific stack port and unit. If no port
   info was found, creates a new port info struct for the unit and port,
   otherwise returns the previously allocated info */
STATIC SL_filter_t *
SL_filter(SL_stk_info_t *info, int unit, bcm_port_t port)
{
    SL_filter_t *filter = info->filter;
    while (filter != NULL) {
        if (filter->unit == unit && filter->port == port) {
            return filter;
        }
        filter = filter->next;
    }
    filter = add_SL_filter(info, unit, port);
    return filter;
}

/* Keep track of info structures locally generated */
static SL_stk_info_t *infop_list[BCM_MAX_NUM_UNITS];

/* Dump diagnostics info from the list of stack port info */
STATIC void
dump_filter_list(int unit)
{
    SL_stk_info_t *info;
    int rv;
    uint64 counter_val;
    char buf_val[32];

    if ((info = infop_list[unit]) != NULL) {
        SL_filter_t *filter = info->filter;
        int idx = 0;
        while (filter != NULL) {
            if (filter->installed) {
                if ((rv = bcm_ffpcounter_get(filter->unit,
                                             filter->port,
                                             filter->counter,
                                             &counter_val)) == BCM_E_NONE) {
                    format_uint64_decimal(buf_val, counter_val, 0);
                    printk("%2d %s (%d,%d) %s\n",
                           idx,
                           filter->is_stack ? "S" : "F",
                           filter->unit,
                           filter->port,
                           buf_val);
#if defined(BROADCOM_DEBUG)
                    bcm_filter_dump(filter->unit, filter->filter_id);
#endif  /* BROADCOM_DEBUG */
                } else {
                    print_err("Could not get ffp counter", rv);
                }
            } else {
                printk("%2d %s (%d,%d) -\n",
                       idx,
                       filter->is_stack ? "S" : "F",
                       filter->unit,
                       filter->port);
            }
            filter = filter->next;
            idx++;
        }
    } else {
        printk("No filters installed for unit %d.\n", unit);
    }
}

/* Install the SL Management filter on a stack port

   This is where the actual FFP rule is constructed.

 */
STATIC int
install_SL_filter(SL_filter_t *slf)
{
    int vd = slf->vid;
    int mask = slf->mask;
    bcm_pbmp_t  pbmp;           /* Bitmap version of port */

    if (slf->pri != IGNORE_PRIORITY) {
        /* stick in priority if not defaulted */
        vd |= ((slf->pri&7)<<13);
    }
    if (mask == DEFAULT_MASK) {
        /* Set default mask depending of vid and pri settings */
        mask = (slf->pri == IGNORE_PRIORITY) ? 0x0fff : 0xefff;
    }

    BCM_IF_ERROR_RETURN(bcm_ffpcounter_create(slf->unit, slf->port,
                                              &slf->counter));
    BCM_IF_ERROR_RETURN(bcm_filter_create(slf->unit, &slf->filter_id));
    BCM_PBMP_CLEAR(pbmp);
    BCM_PBMP_PORT_ADD(pbmp, slf->port);
    BCM_IF_ERROR_RETURN(bcm_filter_qualify_ingress(slf->unit,
                                                   slf->filter_id,
                                                   pbmp));
    printk("iSLf: u=%d id=%d v=%04x m=%04x\n",slf->unit,slf->filter_id,vd, mask);
    BCM_IF_ERROR_RETURN(bcm_filter_qualify_data16(slf->unit,
                                                  slf->filter_id,
                                                  14, vd, mask));
    BCM_IF_ERROR_RETURN(bcm_filter_action_match(slf->unit,
                                                slf->filter_id,
                                                bcmActionSetPrio, slf->cos));
    BCM_IF_ERROR_RETURN(bcm_filter_action_match(slf->unit,
                                                slf->filter_id,
                                                bcmActionIncrCounter,
                                                slf->counter));
    BCM_IF_ERROR_RETURN(bcm_filter_install(slf->unit, slf->filter_id));
    return BCM_E_NONE;
}

/* Uninstall the BCM filter on a stack port */
STATIC int
uninstall_SL_filter(SL_filter_t *slf)
{
    BCM_IF_ERROR_RETURN(bcm_filter_remove(slf->unit, slf->filter_id));
    BCM_IF_ERROR_RETURN(bcm_filter_destroy(slf->unit, slf->filter_id));
    BCM_IF_ERROR_RETURN(bcm_ffpcounter_delete(slf->unit, slf->port,
                                              slf->counter));
    return BCM_E_NONE;
}

STATIC int
SL_filter_install_handler(SL_stk_info_t *info, int unit, bcm_port_t port)
{
    SL_filter_t *filter;
    int rc = BCM_E_INTERNAL;

    filter = SL_filter(info, unit, port);
    if (filter) {
        if (!filter->installed) {
            printk("Installing SL filter on (%d,%d)\n",unit,port);
            filter->is_stack = TRUE;
            filter->pri  = info->pri;
            filter->vid  = info->vid;
            filter->cos  = info->cos;
            filter->mask = info->mask;
            rc = install_SL_filter(filter);
            if (rc != BCM_E_NONE) {
                print_err("Error installing filter", rc);
            } else {
                filter->installed = TRUE;
            }
        } else {
            if (info->verbose) {
                printk("SL Filter already installed\n");
            }
        }
    } else {
        printk("Could not create filter list entry\n");
    }
    return rc;
}

STATIC int
SL_filter_uninstall_handler(SL_stk_info_t *info, int unit, bcm_port_t port)
{
    SL_filter_t *filter;
    int rc = BCM_E_INTERNAL;

    filter = SL_filter(info, unit, port);
    if (filter) {
        if (filter->installed) {
            printk("Uninstalling SL filter on (%d,%d)\n",unit,port);
            rc = uninstall_SL_filter(filter);
            if (rc != BCM_E_NONE) {
                print_err("Error uninstalling filter", rc);
            } else {
                filter->installed = FALSE;
            }
        } else {
            if (info->verbose) {
                printk("SL Filter already uninstalled\n");
            }
        }
    } else {
        printk("Could not create filter list entry\n");
    }
    return rc;
}


/* Stack callback function */
STATIC void
stkcb_callback(int unit, bcm_port_t port, uint32 flags, void *data)
{
    SL_stk_info_t *info = (SL_stk_info_t *)data;
    static char buf[100];

    if (info->verbose) {
        buf[0] = 0;
        sal_sprintf(buf, "stack callback: unit=%d port=%d ", unit, port);
        strcat(buf, (flags & BCM_STK_ENABLE)   ? "E" : "-");
        strcat(buf, (flags & BCM_STK_INACTIVE) ? "I" : "-");
        strcat(buf, (flags & BCM_STK_CAPABLE)  ? "C" : "-");
        strcat(buf, (flags & BCM_STK_DUPLEX)   ? "2" : "-");
        strcat(buf, (flags & BCM_STK_SIMPLEX)  ? "1" : "-");
        strcat(buf, (flags & BCM_STK_NONE)     ? "0" : "-");
        strcat(buf, (flags & BCM_STK_HG)       ? "H" : "-");
        strcat(buf, (flags & BCM_STK_SL)       ? "S" : "-");
        strcat(buf, (flags & BCM_STK_INTERNAL) ? "X" : "-");
        printk("%s\n", buf);
    }
    if ((flags & BCM_STK_ENABLE) != 0 &&
        (flags & BCM_STK_INACTIVE) == 0 &&
        ((flags & BCM_STK_DUPLEX) != 0 ||
         (flags & BCM_STK_SIMPLEX) != 0)) {
        /* (unit,port) is becoming a stack port */
        if (flags & BCM_STK_SL) {
            SL_filter_install_handler(info, unit, port);
        }
    } else {
        /* (unit,port) is becoming a nonstack port */
        if (flags & BCM_STK_SL) {
            SL_filter_uninstall_handler(info, unit, port);
        }
    }
}

/* Dummy callback for testing */
STATIC void
dummy_stkcb_callback(int unit, bcm_port_t port, uint32 flags, void *data)
{
}

/* Subcommands */

STATIC cmd_result_t
stkcb_install(int unit, args_t *args)
{
    parse_table_t	pt;
    cmd_result_t	rv;
    SL_stk_info_t       *info;
    int rc;

    if (infop_list[unit] == NULL) {
        info = sal_alloc(sizeof(SL_stk_info_t), "SL_filter");
        if (info != NULL) {
            infop_list[unit] = info;
            sal_memset(info, 0, sizeof(SL_stk_info_t));
            info->verbose = FALSE;
            info->vid  = DEFAULT_VID;
            info->pri  = IGNORE_PRIORITY; /* Map all pri of vid to cos */
            info->cos  = DEFAULT_COS;
            info->mask = DEFAULT_MASK;

            parse_table_init(unit, &pt);
            parse_table_add(&pt, "Vlan", 	PQ_DFL|PQ_INT,
                            (void *)(0), &info->vid, NULL);
            parse_table_add(&pt, "Pri", 	PQ_DFL|PQ_INT,
                            (void *)(0), &info->pri, NULL);
            parse_table_add(&pt, "Cos", 	PQ_DFL|PQ_INT,
                            (void *)(0), &info->cos, NULL);
            parse_table_add(&pt, "Mask", 	PQ_DFL|PQ_INT,
                            (void *)(0), &info->mask, NULL);
            parse_table_add(&pt, "Verbose", PQ_DFL|PQ_BOOL,
                            (void *)(0), &info->verbose, NULL);

            if (parseEndOk( args, &pt, &rv)) {
                rc = bcm_stk_update_callback_register(unit, stkcb_callback, info);
                if (rc != BCM_E_NONE) {
                    print_err("Could not set stack callback", rc);
                    rv = CMD_FAIL;
                }
                rv = CMD_OK;
            }
        } else {
            print_err("No memory", BCM_E_MEMORY);
            rv = CMD_FAIL;
        }
    } else {
        printk("Callback already installed.\n");
        rv = CMD_FAIL;
    }
    return rv;
}

STATIC cmd_result_t
stkcb_remove(int unit, args_t *args)
{
    SL_stk_info_t *info;
    SL_filter_t *slf, *next;
    int rc;

    if ((info = infop_list[unit]) != NULL) {
        bcm_stk_update_callback_unregister(unit, stkcb_callback, info);
        slf = info->filter;
        while (slf != NULL) {
            if (slf->installed) {
                rc = uninstall_SL_filter(slf);
                if (rc != BCM_E_NONE) {
                    print_err("uninstall_SL_filter", rc);
                }
            }
            next = slf->next;
            sal_free(slf);
            slf = next;
        }
        info->filter = NULL;
    } else {
        printk("Callback info not defined for unit %d\n", unit);
    }
    return CMD_OK;
}

STATIC cmd_result_t
stkcb_fp(int unit, args_t *args)
{
    parse_table_t	pt;
    cmd_result_t	rv = CMD_FAIL;
    int rc, port, vid, pri, cos, mask;
    SL_stk_info_t *info;
    SL_filter_t *slf;

    if ((info = infop_list[unit]) != NULL) {
        vid  = DEFAULT_VID;
        pri  = IGNORE_PRIORITY; /* Map all pri of vid to cos */
        cos  = DEFAULT_COS;
        mask = DEFAULT_MASK;
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Port", 	PQ_DFL|PQ_PORT,
                        (void *)(0), &port, NULL);
        parse_table_add(&pt, "Vlan", 	PQ_DFL|PQ_INT,
                        (void *)(0), &vid, NULL);
        parse_table_add(&pt, "Pri", 	PQ_DFL|PQ_INT,
                        (void *)(0), &pri, NULL);
        parse_table_add(&pt, "Cos", 	PQ_DFL|PQ_INT,
                        (void *)(0), &cos, NULL);
        parse_table_add(&pt, "Mask", 	PQ_DFL|PQ_INT,
                        (void *)(0), &mask, NULL);

        if (parseEndOk( args, &pt, &rv)) {
            slf = SL_filter(info, unit, port);
            if (slf) {
                slf->is_stack = FALSE;
                slf->port = port;
                slf->vid  = vid;
                slf->pri  = pri;
                slf->cos  = cos;
                slf->mask = mask;
                if (slf->installed) {
                    rc = uninstall_SL_filter(slf);
                    if (rc != BCM_E_NONE) {
                        print_err("Error uninstalling filter", rc);
                    } else {
                        slf->installed = FALSE;
                    }
                }
                rc = install_SL_filter(slf);
                if (rc != BCM_E_NONE) {
                    print_err("Could not set front panel filter", rc);
                } else {
                    slf->installed = TRUE;
                }
                rv = CMD_OK;
            } else {
                printk("Could not create filter list entry\n");
            }
        }
    } else {
        printk("Callback info not defined for unit %d\n", unit);
    }

    return rv;
}

char cmd_stkcb_usage[] =
    "\n"
    "  install Vlan=VLANID Pri=PRI Cos=COS Mask=MASK Verbose=T/F\n"
    "    installs SL stack port filters\n"
    "  remove\n"
    "    removes installed SL stack port filters\n"
    "  dump\n"
    "    dumps list of SL stack port filters\n"
    "  fp Port=port Vlan=VLANID pri=PRI cos=COS mask=MASK"
    "    installs a front panel filter on port [for testing only]\n"
    "  dummy\n"
    "    installs a dummy stack port filter [for testing only]\n"
    "  destroy\n"
    "    unconditionally removes all stack port callbacks\n"
    "\n";

cmd_result_t
cmd_stkcb(int unit, args_t *args)
{
    char	*subcmd;
    cmd_result_t rv = CMD_FAIL;

    subcmd = ARG_GET(args);

    if (subcmd != NULL) {
        if (!sal_strcasecmp(subcmd, "install")) {
            rv = stkcb_install(unit, args);
        } else if (!sal_strcasecmp(subcmd, "remove")) {
            rv = stkcb_remove(unit, args);
        } else if (!sal_strcasecmp(subcmd, "dump")) {
            dump_filter_list(unit);
            rv = CMD_OK;
        } else if (!sal_strcasecmp(subcmd, "fp")) {
            rv = stkcb_fp(unit, args);
        } else if (!sal_strcasecmp(subcmd, "dummy")) {
            bcm_stk_update_callback_register(unit, dummy_stkcb_callback, NULL);
            rv = CMD_OK;
        } else if (!sal_strcasecmp(subcmd, "destroy")) {
            bcm_stk_update_callback_unregister(unit, dummy_stkcb_callback, NULL);
            rv = CMD_OK;
        } else {
            rv = CMD_USAGE;
        }
    } else {
        rv = CMD_USAGE;
    }
    
    return rv;
}

#endif /* BCM_FILTER_SUPPORT */
