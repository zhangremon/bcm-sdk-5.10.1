/*
 * $Id: cmds.c 1.18 Broadcom SDK $
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
 * File:     cmds.c
 * Purpose:  Other BCMX CLI commands
 */

#include <sal/appl/io.h>
#include <sal/appl/sal.h>

#include <bcm/rate.h>

#include <bcm/error.h>
#include <bcmx/bcmx.h>
#include <bcmx/rate.h>
#include <bcmx/link.h>
#include <bcmx/diffserv.h>
#include <bcmx/meter.h>

#include <appl/diag/shell.h>
#include <appl/diag/system.h>
#include <appl/diag/parse.h>

#include "bcmx.h"
#include <appl/diag/diag.h>

STATIC void
_bcmx_link_callback(bcmx_lport_t lport, bcm_port_info_t *info)
{
    if (info->linkstatus) {
        sal_printf("BCMX:  Link change UP, uport %s, %dMb %s.\n",
                   bcmx_lport_to_uport_str(lport), info->speed,
                   info->duplex ? "Full Duplex" : "Half Duplex");
    } else {
        sal_printf("BCMX:  Link change DOWN, uport %s.\n",
                   bcmx_lport_to_uport_str(lport));
    }
}


char bcmx_cmd_link_usage[] =
    "LINKscan [check | reg | unreg | <usecs>]\n"
    "    Simple linkscan control command.\n"
    "    With no arguments, does a simple enable-get\n"
    "    check    - Enable-get with consistency check\n"
    "    reg      - Register local handler\n"
    "    unreg    - Unregister local handler\n"
    "    <usecs>  - Enable set with the given time; 0 disables\n";

cmd_result_t
bcmx_cmd_link(int unit, args_t *args)
{
    int us, consistent;
    char *param;
    int rv;
    static int reg_count = 0;

    if ((param = ARG_GET(args)) == NULL) {
        rv = bcmx_linkscan_enable_get(&us, NULL);
        if (rv < 0) {
            sal_printf("ERROR getting bcmx link status %d: %s\n",
                       rv, bcm_errmsg(rv));
            return CMD_FAIL;
        }
        sal_printf("BCMX linkscan time %d: %s\n", us,
                   us > 0 ? "enabled" : "disabled");
        sal_printf("BCMX register count is %d\n", reg_count);
        return CMD_OK;
    }

    if (!sal_strcasecmp(param, "check")) {
        rv = bcmx_linkscan_enable_get(&us, &consistent);
        if (rv < 0) {
            sal_printf("ERROR getting bcmx link status %d: %s\n",
                       rv, bcm_errmsg(rv));
            return CMD_FAIL;
        }
        sal_printf("BCMX linkscan time %d: %s.  Units are %sconsistent\n",
                   us, us > 0 ? "enabled" : "disabled",
                   consistent ? "" : "not ");
        sal_printf("BCMX register count is %d\n", reg_count);
    } else if (!sal_strcasecmp(param, "reg")) {
        rv = bcmx_linkscan_register(_bcmx_link_callback);
        ++reg_count;
        sal_printf("BCMX linkscan register returns %d: %s\n",
                   rv, bcm_errmsg(rv));
    } else if (!sal_strcasecmp(param, "unreg")) {
        rv = bcmx_linkscan_unregister(_bcmx_link_callback);
        --reg_count;
        sal_printf("BCMX linkscan register returns %d: %s\n",
                   rv, bcm_errmsg(rv));
    } else if (isdigit((unsigned) *param) || *param == '-') {
        us = parse_integer(param);
        rv = bcmx_linkscan_enable_set(us);
        sal_printf("Set BCMX linkscan time to %d.  Result %d: %s\n", us,
                   rv, bcm_errmsg(rv));
    } else {
        sal_printf("Unknown subcommand %s.\n", param);
        return CMD_USAGE;
    }

    return CMD_OK;
}

char bcmx_cmd_meter_usage[] =
    "BCMX METER Usages:\n"
    "    meter init                 -- Call meter init\n"
    "    meter create <uport>       -- Call meter create and display ID\n"
    "    meter delete <uport> <id>  -- Delete the given meter\n"
    "    meter set <uport> <id> <rate> <burst>\n"
    "                               -- Set the rates for the given meter\n"
    "    meter get <uport> <id>     -- Get the rates for the given meter\n";

cmd_result_t
bcmx_cmd_meter(int unit, args_t *args)
{
    char *subcmd, *port_str, *mid_str;
    bcmx_lport_t lport;
    int mid;
    uint32 rate, burst;
    int rv = BCM_E_NONE;

    if ((subcmd = ARG_GET(args)) == NULL) {
        sal_printf("Subcommand required\n");
        return CMD_USAGE;
    }

    if (sal_strcasecmp(subcmd, "init") == 0) {
        rv = bcmx_meter_init();
        if (rv < 0) {
            sal_printf("Meter init returned error %d: %s\n", rv,
                       bcm_errmsg(rv));
            return CMD_FAIL;
        }
        return CMD_OK;
    }

    if ((port_str = ARG_GET(args)) == NULL) {
        sal_printf("Subcommand requires uport reference\n");
        return CMD_USAGE;
    }

    lport = bcmx_uport_to_lport(bcmx_uport_parse(port_str, NULL));
    if (!BCMX_LPORT_VALID(lport)) {
        sal_printf("Invalid uport reference %s\n", port_str);
        return CMD_FAIL;
    }

    if (sal_strcasecmp(subcmd, "create") == 0) {
        rv = bcmx_meter_create(lport, &mid);
        if (rv < 0) {
            sal_printf("Meter create failed with error %d: %s\n", rv,
                       bcm_errmsg(rv));
        } else {
            sal_printf("Meter created with id %d\n", mid);
        }
        return CMD_OK;
    }

    if ((mid_str = ARG_GET(args)) == NULL) {
        sal_printf("Subcommand requires meter ID\n");
        return CMD_USAGE;
    }

    mid = parse_integer(mid_str);

    if (sal_strcasecmp(subcmd, "delete") == 0) {
        rv = bcmx_meter_delete(lport, mid);
        if (rv < 0) {
            sal_printf("Meter delete(%d) failed with error %d: %s\n", mid, rv,
                       bcm_errmsg(rv));
            return CMD_FAIL;
        }
        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "set") == 0) {
        char *rate_str, *burst_str;

        if ((rate_str = ARG_GET(args)) == NULL) {
            sal_printf("Subcommand requires rate\n");
            return CMD_USAGE;
        }
        rate = parse_integer(rate_str);

        if ((burst_str = ARG_GET(args)) == NULL) {
            sal_printf("Subcommand requires burst\n");
            return CMD_USAGE;
        }
        burst = parse_integer(burst_str);

        rv = bcmx_meter_set(lport, mid, rate, burst);
        if (rv < 0) {
            sal_printf("Meter set(%d, %d, %d) failed with error %d: %s\n",
                       mid, rate, burst, rv, bcm_errmsg(rv));
            return CMD_FAIL;
        }

        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "get") == 0) {
        rv = bcmx_meter_get(lport, mid, &rate, &burst);
        if (rv < 0) {
            sal_printf("Meter get(%d) failed with error %d: %s\n",
                       mid, rv, bcm_errmsg(rv));
            return CMD_FAIL;
        }
        sal_printf("Meter get(%s, %d): rate %d; burst %d\n",
                   bcmx_lport_to_uport_str(lport), mid, rate, burst);
        return CMD_OK;
    }

    sal_printf("Unknown meter subcommand: %s\n", subcmd);
    return CMD_USAGE;
}

char bcmx_cmd_rate_usage[] =
    "BCMX RATE Usages:\n"
    "    rate <Limit=val> [Bcast=val] [Mcast=val] [Dlf=val]\n"
    "                                - Set specified packet rate limit\n\n"
    "         limit : packets per second\n"
    "         bcast : Enable broadcast rate control\n"
    "         mcast : Enable multicast rate control\n"
    "         dlf   : Enable DLF flooding rate control\n"
    "         If no flags are given, displays the current rate settings\n";

cmd_result_t
bcmx_cmd_rate(int unit, args_t *args)
{
    parse_table_t pt;
    int limit;
    int bc_limit;
    int mc_limit;
    int dlf_limit;
    int flags = 0;
    int rv = BCM_E_NONE;

    if ((rv = bcmx_rate_get(&limit, &flags)) < 0) {
        sal_printf("ERROR: could not get rate: %s\n", bcm_errmsg(rv));
        return (CMD_FAIL);
    }

    bc_limit = ((flags & BCM_RATE_BCAST) ? TRUE : FALSE);
    mc_limit = ((flags & BCM_RATE_MCAST) ? TRUE : FALSE);
    dlf_limit = ((flags & BCM_RATE_DLF) ? TRUE : FALSE);

    parse_table_init(0, &pt);
    parse_table_add(&pt, "Limit", PQ_DFL|PQ_HEX, INT_TO_PTR(limit) , &limit, 0);
    parse_table_add(&pt, "Bcast", PQ_DFL|PQ_BOOL, (void *)0, &bc_limit, 0);
    parse_table_add(&pt, "Mcast", PQ_BOOL, INT_TO_PTR(mc_limit), &mc_limit, 0);
    parse_table_add(&pt, "Dlf", PQ_DFL|PQ_BOOL, INT_TO_PTR(dlf_limit), &dlf_limit, 0);

    if (ARG_GET(args) == NULL) {
        sal_printf("Current settings:\n");
        parse_eq_format(&pt);
        parse_arg_eq_done(&pt);

        return(CMD_OK);
    }

    if (parse_arg_eq(args, &pt) < 0) {
        sal_printf("Error: Unknown option: %s\n", ARG_CUR(args));
        parse_arg_eq_done(&pt);

        return(CMD_FAIL);
    }

    if (bc_limit) {
        flags |= (BCM_RATE_BCAST);
    } else {
        flags &= (~(BCM_RATE_BCAST));
    }

    if (mc_limit) {
        flags |= (BCM_RATE_MCAST);
    } else {
        flags &= (~(BCM_RATE_MCAST));
    }

    if (dlf_limit) {
        flags |= (BCM_RATE_DLF);
    } else {
        flags &= (~(BCM_RATE_DLF));
    }

    if ((rv = bcmx_rate_set(limit, flags)) < 0) {
        sal_printf("ERROR: could not set rate: %s\n", bcm_errmsg(rv));
        parse_arg_eq_done(&pt);

        return (CMD_FAIL);
    }

    parse_arg_eq_done(&pt);

    return CMD_OK;
}


#ifdef BCM_SWITCH_SUPPORT

char bcmx_cmd_diffserv_usage[] =
#ifdef COMPILER_STRING_CONST_LIMIT
    "BCMX DIFFSERV usage:\n"
    "    diffserv <cmd> [<option>=<value>]\n"
    "    Where <cmd> is:\n"
    "        init|show|dpcreate|dpdelete|dpinstall|cfcreate|cfupdate|cfdelete\n"
    "        cfdelete|scheduler|counter|test1|test2|test3\n"
    "    and <option> is:\n"
    "        UPORTS|DPID|EGRESS|CFID|IPPORTLO|IPPORTHI|DA|SA|PORT\n"
#else /* !COMPILER_STRING_CONST_LIMIT */
    "BCMX DIFFSERV usage:\n"
    "    diffserv init\n"
    "    diffserv show\n"
    "    diffserv dpcreate <UPORTS> DPID=<n> EGRESS=[T|F]\n"
    "    diffserv dpdelete DPID=<n>\n"
    "    diffserv dpinstall DPID=<n>\n"
    "    diffserv cfcreate DPID=<n> CFID=<n> ...\n"
    "    diffserv cfupdate DPID=<n> CFID=<n> ...\n"
    "    diffserv cfdelete DPID=<n> CFID=<n>\n"
    "    diffserv scheduler DPID=<n> ...\n"
    "    diffserv counter DPID=<n> CFID=<n>\n"
    "    diffserv test1 DPID=<n> CFID=<n>\n"
    "    diffserv test2 DPID=<n> CFID=<n> IPPORTLO=<n> IPPORTHI=<n>\n"
    "    diffserv test3 DPID=<n> CFID=<n> SA=<mac> DA=<mac> PORT=<lport>\n"
#endif /* !COMPILER_STRING_CONST_LIMIT */
;

cmd_result_t
bcmx_cmd_diffserv(int unit, args_t *args)
{
    int                 rv, dpid, cfid, cookie;
    char                *cmd, *subcmd, *s;
    uint32              flags;
    bcmx_lplist_t       lplist;
    bcm_ds_clfr_t       clfr;
    bcm_ds_inprofile_actn_t     inp_actn;
    bcm_ds_outprofile_actn_t    outp_actn;
    bcm_ds_nomatch_actn_t       nm_actn;
    bcm_ds_scheduler_t          sched;
    bcm_ds_counters_t           counter;
    char                        buf[128];
    int egress=FALSE;
    parse_table_t pt;

    COMPILER_REFERENCE(cookie);
    dpid = -1;
    cfid = -1;

    cmd = ARG_CMD(args);
    subcmd = ARG_GET(args);
    if (subcmd == NULL || sal_strcasecmp(subcmd, "show") == 0) {
        subcmd = "show";
#ifdef  BROADCOM_DEBUG
        rv = bcmx_ds_dump();
#else
        rv = BCM_E_UNAVAIL;
#endif  /* BROADCOM_DEBUG */
        if (rv < 0) {
            sal_printf("ERROR: %s %s: %s\n", cmd, subcmd, bcm_errmsg(rv));
            return CMD_FAIL;
        }
        return CMD_OK;
    }
    if (sal_strcasecmp(subcmd, "init") == 0) {
        rv = bcmx_ds_init();
        if (rv < 0) {
            sal_printf("ERROR: %s %s: %s\n", cmd, subcmd, bcm_errmsg(rv));
            return CMD_FAIL;
        }
        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "dpcreate") == 0) {

        s = ARG_GET(args);
        if (s == NULL) {
            sal_printf("ERROR: %s %s: missing user port list argument\n",
                       cmd, subcmd);
            return CMD_FAIL;
        }

        bcmx_lplist_init(&lplist, 0, 0);
        if (bcmx_lplist_parse(&lplist, s)) {
            sal_printf("ERROR: %s %s: user port list parse failed\n",
                       cmd, subcmd);
            return CMD_FAIL;
        }
        egress = FALSE;
        parse_table_init(0, &pt);
        parse_table_add(&pt, "DPID", PQ_DFL|PQ_INT, 0, &dpid, 0);
        parse_table_add(&pt, "EGRESS", PQ_DFL|PQ_BOOL, (void *)0, &egress, 0);

        if (parse_arg_eq(args, &pt) < 0) {
            sal_printf("Error: Unknown option: %s\n", ARG_CUR(args));
            bcmx_lplist_free(&lplist);
            parse_arg_eq_done(&pt);
            return(CMD_FAIL);
        }

        if (dpid < 0) {
            sal_printf("ERROR: %s %s: Must specify dpid >= 0\n", cmd, subcmd);
            bcmx_lplist_free(&lplist);
            parse_arg_eq_done(&pt);
            return CMD_FAIL;
        }

        flags = 0;
        if (egress) {
            flags |= BCM_DS_EGRESS;
        }

        rv = bcmx_ds_datapath_create(dpid, flags, lplist);
        bcmx_lplist_free(&lplist);
        parse_arg_eq_done(&pt);
        if (rv < 0) {
            sal_printf("ERROR: %s %s: %s\n", cmd, subcmd, bcm_errmsg(rv));
            return CMD_FAIL;
        }

        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "dpdelete") == 0) {
        parse_table_init(0, &pt);
        parse_table_add(&pt, "DPID", PQ_DFL|PQ_INT, 0, &dpid, 0);

        if (parse_arg_eq(args, &pt) < 0) {
            sal_printf("Error: Unknown option: %s\n", ARG_CUR(args));
            parse_arg_eq_done(&pt);
            return(CMD_FAIL);
        }

        if (dpid < 0) {
            sal_printf("ERROR: %s %s: Must specify dpid >= 0\n", cmd, subcmd);
            parse_arg_eq_done(&pt);
            return CMD_FAIL;
        }

        rv = bcmx_ds_datapath_delete(dpid);
        parse_arg_eq_done(&pt);
        if (rv < 0) {
            sal_printf("ERROR: %s %s: %s\n", cmd, subcmd, bcm_errmsg(rv));
            return CMD_FAIL;
        }

        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "dpinstall") == 0) {
        parse_table_init(0, &pt);
        parse_table_add(&pt, "DPID", PQ_DFL|PQ_INT, 0, &dpid, 0);

        if (parse_arg_eq(args, &pt) < 0) {
            sal_printf("Error: Unknown option: %s\n", ARG_CUR(args));
            parse_arg_eq_done(&pt);
            return(CMD_FAIL);
        }

        if (dpid < 0) {
            sal_printf("ERROR: %s %s: Must specify dpid >= 0\n", cmd, subcmd);
            parse_arg_eq_done(&pt);
            return CMD_FAIL;
        }

        rv = bcmx_ds_datapath_install(dpid);
        parse_arg_eq_done(&pt);
        if (rv < 0) {
            sal_printf("ERROR: %s %s: %s\n", cmd, subcmd, bcm_errmsg(rv));
            return CMD_FAIL;
        }

        return CMD_OK;
    }

    /* diffserv cfcreate <DPID> ... */
    if (sal_strcasecmp(subcmd, "cfcreate") == 0) {
        parse_table_init(0, &pt);
        parse_table_add(&pt, "DPID", PQ_DFL|PQ_INT, 0, &dpid, 0);
        parse_table_add(&pt, "CFID", PQ_DFL|PQ_INT, 0, &cfid, 0);

        if (parse_arg_eq(args, &pt) < 0) {
            sal_printf("Error: Unknown option: %s\n", ARG_CUR(args));
            parse_arg_eq_done(&pt);
            return(CMD_FAIL);
        }

        if (dpid < 0 || cfid < 0) {
            sal_printf("ERROR: %s %s: Must specify dpid and cfid >= 0\n",
                       cmd, subcmd);
            parse_arg_eq_done(&pt);
            return CMD_FAIL;
        }

        sal_memset(&clfr, 0, sizeof(clfr));
        sal_memset(&inp_actn, 0, sizeof(inp_actn));
        sal_memset(&outp_actn, 0, sizeof(outp_actn));
        sal_memset(&nm_actn, 0, sizeof(nm_actn));

        

        outp_actn.opa_flags = BCM_DS_OUT_ACTN_DO_NOT_SWITCH;

        /* Hardcode a rate of 40000 kbits/sec, max allowable burst size */
        outp_actn.opa_kbits_burst = BCM_DS_METER_BURST_MAX;
        outp_actn.opa_kbits_sec = 40000; /* 5 MBytes/sec */
        
/*
        rv = bcmx_ds_classifier_create(dpid, cfid, &clfr,
                                       &inp_actn, &outp_actn, &nm_actn);
*/
        rv = bcmx_ds_classifier_create(dpid, cfid, &clfr,
                                       NULL, &outp_actn, NULL);
        parse_arg_eq_done(&pt);
        if (rv < 0) {
            sal_printf("ERROR: %s %s: %s\n", cmd, subcmd, bcm_errmsg(rv));
            return CMD_FAIL;
        }
        return CMD_OK;
    }

    /* diffserv cfupdate <DPID> <CFID> ... */
    if (sal_strcasecmp(subcmd, "cfupdate") == 0) {
        parse_table_init(0, &pt);
        parse_table_add(&pt, "DPID", PQ_DFL|PQ_INT, 0, &dpid, 0);
        parse_table_add(&pt, "CFID", PQ_DFL|PQ_INT, 0, &cfid, 0);

        if (parse_arg_eq(args, &pt) < 0) {
            sal_printf("Error: Unknown option: %s\n", ARG_CUR(args));
            parse_arg_eq_done(&pt);
            return(CMD_FAIL);
        }

        if (dpid < 0 || cfid < 0) {
            sal_printf("ERROR: %s %s: Must specify dpid and cfid >= 0\n",
                       cmd, subcmd);
            parse_arg_eq_done(&pt);
            return CMD_FAIL;
        }

        flags = 0;
        sal_memset(&inp_actn, 0, sizeof(inp_actn));
        /* flags |= BCM_DS_UPDATE_INPROFILE */
        sal_memset(&outp_actn, 0, sizeof(outp_actn));
        /* flags |= BCM_DS_UPDATE_OUTPROFILE */

        

        rv = bcmx_ds_classifier_update(dpid, cfid,
                                       flags, &inp_actn, &outp_actn);
        parse_arg_eq_done(&pt);
        if (rv < 0) {
            sal_printf("ERROR: %s %s: %s\n", cmd, subcmd, bcm_errmsg(rv));
            return CMD_FAIL;
        }
        return CMD_OK;
    }

    /* diffserv cfdelete <DPID> <CFID> */
    if (sal_strcasecmp(subcmd, "cfdelete") == 0) {
        parse_table_init(0, &pt);
        parse_table_add(&pt, "DPID", PQ_DFL|PQ_INT, 0, &dpid, 0);
        parse_table_add(&pt, "CFID", PQ_DFL|PQ_INT, 0, &cfid, 0);

        if (parse_arg_eq(args, &pt) < 0) {
            sal_printf("Error: Unknown option: %s\n", ARG_CUR(args));
            parse_arg_eq_done(&pt);
            return(CMD_FAIL);
        }

        if (dpid < 0 || cfid < 0) {
            sal_printf("ERROR: %s %s: Must specify dpid and cfid >= 0\n",
                       cmd, subcmd);
            parse_arg_eq_done(&pt);
            return CMD_FAIL;
        }

        rv = bcmx_ds_classifier_delete(dpid, cfid);
        parse_arg_eq_done(&pt);
        if (rv < 0) {
            sal_printf("ERROR: %s %s: %s\n", cmd, subcmd, bcm_errmsg(rv));
            return CMD_FAIL;
        }
        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "counter") == 0 || sal_strcasecmp(subcmd, "ctr") == 0) {
        parse_table_init(0, &pt);
        parse_table_add(&pt, "DPID", PQ_DFL|PQ_INT, 0, &dpid, 0);
        parse_table_add(&pt, "CFID", PQ_DFL|PQ_INT, 0, &cfid, 0);

        if (parse_arg_eq(args, &pt) < 0) {
            sal_printf("Error: Unknown option: %s\n", ARG_CUR(args));
            parse_arg_eq_done(&pt);
            return(CMD_FAIL);
        }

        if (dpid < 0 || cfid < 0) {
            sal_printf("ERROR: %s %s: Must specify dpid and cfid >= 0\n",
                       cmd, subcmd);
            parse_arg_eq_done(&pt);
            return CMD_FAIL;
        }

        rv = bcmx_ds_counter_get(dpid, cfid, &counter);
        parse_arg_eq_done(&pt);
        if (rv < 0) {
            sal_printf("ERROR: %s %s: %s\n", cmd, subcmd, bcm_errmsg(rv));
            return CMD_FAIL;
        }
        
        format_uint64(buf, counter.inp_pkt_cntr);
        sal_printf("In profile packets: %s\n", buf);
        format_uint64(buf, counter.outp_pkt_cntr);
        sal_printf("Out of profile packets: %s\n", buf);
        return CMD_OK;
    }

    /* diffserv scheduler <DPID> ... */
    if (sal_strcasecmp(subcmd, "scheduler") == 0) {
        parse_table_init(0, &pt);
        parse_table_add(&pt, "DPID", PQ_DFL|PQ_INT, 0, &dpid, 0);

        if (parse_arg_eq(args, &pt) < 0) {
            sal_printf("Error: Unknown option: %s\n", ARG_CUR(args));
            parse_arg_eq_done(&pt);
            return(CMD_FAIL);
        }

        if (dpid < 0) {
            sal_printf("ERROR: %s %s: Must specify dpid >= 0\n",
                       cmd, subcmd);
            parse_arg_eq_done(&pt);
            return CMD_FAIL;
        }

        sal_memset(&sched, 0, sizeof(sched));

        

        rv = bcmx_ds_scheduler_add(dpid, &sched);
        parse_arg_eq_done(&pt);
        if (rv < 0) {
            sal_printf("ERROR: %s %s: %s\n", cmd, subcmd, bcm_errmsg(rv));
            return CMD_FAIL;
        }
        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "test1") == 0) {
        bcm_ds_clfr_t c1, c2, c3, c4;
        bcm_ds_inprofile_actn_t ipa_deny, ipa_permit;

        parse_table_init(0, &pt);
        parse_table_add(&pt, "DPID", PQ_DFL|PQ_INT, 0, &dpid, 0);
        parse_table_add(&pt, "CFID", PQ_DFL|PQ_INT, 0, &cfid, 0);

        if (parse_arg_eq(args, &pt) < 0) {
            sal_printf("Error: Unknown option: %s\n", ARG_CUR(args));
            parse_arg_eq_done(&pt);
            return(CMD_FAIL);
        }

        if (dpid < 0 || cfid < 0) {
            sal_printf("ERROR: %s %s: Must specify dpid and cfid >= 0\n",
                       cmd, subcmd);
            parse_arg_eq_done(&pt);
            return CMD_FAIL;
        }
        parse_arg_eq_done(&pt);

        /* ipa_deny */
        sal_memset(&ipa_deny, 0, sizeof(ipa_deny));
        ipa_deny.ipa_flags = BCM_DS_ACTN_DO_NOT_SWITCH;

        /* ipa_permit */
        sal_memset(&ipa_permit, 0, sizeof(ipa_permit));
        ipa_permit.ipa_flags = BCM_DS_ACTN_DO_SWITCH;

        /* c1: deny all */
        sal_memset(&c1, 0, sizeof(c1));
        c1.cf_precedence = 1;
        c1.cf_flags = BCM_DS_CLFR_VLAN_TAG;
        c1.cf_vid = 1;

        rv = bcmx_ds_classifier_create(dpid, cfid, &c1,
                                       &ipa_deny, NULL, NULL);
        if (rv < 0) {
            sal_printf("ERROR: %s %s: classifier_create1: %s\n",
                       cmd, subcmd, bcm_errmsg(rv));
            return CMD_FAIL;
        }
        

        /* c2: permit l4 port 21 */
        sal_memset(&c2, 0, sizeof(c2));
        c2.cf_precedence = 2;
        c2.cf_flags = BCM_DS_CLFR_VLAN_TAG |
                        BCM_DS_CLFR_IPV4_L4_DST_PORT;
        c2.cf_vid = 1;
        c2.cf_dst_port = 21;

        rv = bcmx_ds_classifier_create(dpid, cfid + 1, &c2,
                                       &ipa_permit, NULL, NULL);
        if (rv < 0) {
            sal_printf("ERROR: %s %s: classifier_create2: %s\n",
                       cmd, subcmd, bcm_errmsg(rv));
            return CMD_FAIL;
        }

        /* c3: permit source ip 4.x.x.x */
        sal_memset(&c3, 0, sizeof(c3));
        c3.cf_precedence = 3;
        c3.cf_flags = BCM_DS_CLFR_VLAN_TAG |
                        BCM_DS_CLFR_IPV4_SRC_ADDR |
                        BCM_DS_CLFR_IPV4_SRC_PREFIX;
        c3.cf_vid = 1;
        c3.cf_src_ip_addr = 0x04000000;
        c3.cf_src_ip_prefix_len = 8;

        rv = bcmx_ds_classifier_create(dpid, cfid + 2, &c3,
                                       &ipa_permit, NULL, NULL);
        if (rv < 0) {
            sal_printf("ERROR: %s %s: classifier_create3: %s\n",
                       cmd, subcmd, bcm_errmsg(rv));
            return CMD_FAIL;
        }

        /* c4: deny l4 port 23 */
        sal_memset(&c4, 0, sizeof(c4));
        c4.cf_precedence = 4;
        c4.cf_flags = BCM_DS_CLFR_VLAN_TAG |
                        BCM_DS_CLFR_IPV4_L4_DST_PORT;
        c4.cf_vid = 1;
        c4.cf_dst_port = 23;

        rv = bcmx_ds_classifier_create(dpid, cfid + 3, &c4,
                                       &ipa_deny, NULL, NULL);
        if (rv < 0) {
            sal_printf("ERROR: %s %s: classifier_create4: %s\n",
                       cmd, subcmd, bcm_errmsg(rv));
            return CMD_FAIL;
        }

        sal_printf("cfid base: %d.  Increments 0 to 3.\n", cfid);

        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "test2") == 0) {
        bcm_ds_clfr_t c1;
        bcm_ds_inprofile_actn_t ipa_deny, ipa_permit;
        int     ipportlo = -1, ipporthi = -1;

        parse_table_init(0, &pt);
        parse_table_add(&pt, "DPID", PQ_DFL|PQ_INT, 0, &dpid, 0);
        parse_table_add(&pt, "CFID", PQ_DFL|PQ_INT, 0, &cfid, 0);
        parse_table_add(&pt, "IPPORTLO", PQ_DFL|PQ_INT, 0, &ipportlo, 0);
        parse_table_add(&pt, "IPPORTHI", PQ_DFL|PQ_INT, 0, &ipporthi, 0);

        if (parse_arg_eq(args, &pt) < 0) {
            sal_printf("Error: Unknown option: %s\n", ARG_CUR(args));
            parse_arg_eq_done(&pt);
            return(CMD_FAIL);
        }

        if (dpid < 0 || cfid < 0 || ipportlo < 0 || ipporthi < 0) {
            sal_printf("ERROR: %s %s: "
                       "Must specify dpid, cfid, port lo and hi all >= 0\n",
                       cmd, subcmd);
            parse_arg_eq_done(&pt);
            return CMD_FAIL;
        }
        parse_arg_eq_done(&pt);

        /* ipa_deny */
        sal_memset(&ipa_deny, 0, sizeof(ipa_deny));
        ipa_deny.ipa_flags = BCM_DS_ACTN_DO_NOT_SWITCH;

        /* ipa_permit */
        sal_memset(&ipa_permit, 0, sizeof(ipa_permit));
        ipa_permit.ipa_flags = BCM_DS_ACTN_DO_SWITCH;

        /* c1: deny ip src ports %d %d */
        sal_memset(&c1, 0, sizeof(c1));
        c1.cf_precedence = 1;
        c1.cf_flags = BCM_DS_CLFR_VLAN_TAG |
                        BCM_DS_CLFR_IPV4_L4_SRC_PORT_MIN |
                        BCM_DS_CLFR_IPV4_L4_SRC_PORT_MAX;
        c1.cf_vid = 1;
        c1.cf_src_port_min = ipportlo;
        c1.cf_src_port_max = ipporthi;

        rv = bcmx_ds_classifier_create(dpid, cfid, &c1,
                                       &ipa_deny, NULL, NULL);
        if (rv < 0) {
            sal_printf("ERROR: %s %s: classifier_create1: %s\n",
                       cmd, subcmd, bcm_errmsg(rv));
            return CMD_FAIL;
        }

        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "test3") == 0) {
        bcm_ds_clfr_t c1;
        bcm_ds_inprofile_actn_t ipa_redir;
	bcm_mac_t	srcmac, dstmac;
	bcmx_lport_t	lport;

        parse_table_init(0, &pt);
        parse_table_add(&pt, "DPID", PQ_DFL|PQ_INT, 0, &dpid, 0);
        parse_table_add(&pt, "CFID", PQ_DFL|PQ_INT, 0, &cfid, 0);
        parse_table_add(&pt, "SA", PQ_MAC, 0, &srcmac, 0);
        parse_table_add(&pt, "DA", PQ_MAC, 0, &dstmac, 0);
        parse_table_add(&pt, "PORT", PQ_INT, 0, &lport, 0);

        if (parse_arg_eq(args, &pt) < 0) {
            sal_printf("Error: Unknown option: %s\n", ARG_CUR(args));
            parse_arg_eq_done(&pt);
            return(CMD_FAIL);
        }

        if (dpid < 0 || cfid < 0) {
            sal_printf("ERROR: %s %s: Must specify dpid and cfid\n",
                       cmd, subcmd);
            parse_arg_eq_done(&pt);
            return CMD_FAIL;
        }
        parse_arg_eq_done(&pt);

        /* redirect */
        sal_memset(&ipa_redir, 0, sizeof(ipa_redir));
        ipa_redir.ipa_flags = BCM_DS_ACTN_DO_SWITCH |
	    BCM_DS_ACTN_SET_OUT_PORT_ALL;
	ipa_redir.ipa_port = lport;

        /* c1: redirect SA/DA pkts to port */
        sal_memset(&c1, 0, sizeof(c1));
        c1.cf_precedence = 1;
        c1.cf_flags = BCM_DS_CLFR_SRC_MAC_ADDR |
		      BCM_DS_CLFR_DST_MAC_ADDR;
	sal_memcpy(c1.cf_src_mac, srcmac, sizeof(srcmac));
	sal_memcpy(c1.cf_dst_mac, dstmac, sizeof(dstmac));

        rv = bcmx_ds_classifier_create(dpid, cfid, &c1,
                                       &ipa_redir, NULL, NULL);
        if (rv < 0) {
            sal_printf("ERROR: %s %s: classifier_create: %s\n",
                       cmd, subcmd, bcm_errmsg(rv));
            return CMD_FAIL;
        }

        return CMD_OK;
    }

    sal_printf("ERROR: %s %s: unrecognized subcommand\n", cmd, subcmd);
    return CMD_FAIL;
}


#endif  /* BCM_SWITCH_SUPPORT */
