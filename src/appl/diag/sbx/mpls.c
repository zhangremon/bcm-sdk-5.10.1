/* 
 * $Id: mpls.c 1.13 Broadcom SDK $
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
 * File:        mpls.c
 * Purpose:     mpls vpn CLI commands
 *
 */

#include <appl/diag/shell.h>
#include <appl/diag/system.h>
#include <appl/diag/parse.h>

#include <shared/gport.h>
#include <bcm/types.h>
#include <bcm/error.h>
#include <bcm/l3.h>
#include <bcm/debug.h>
#include <bcm/ipmc.h>
#include <bcm/tunnel.h>
#include <bcm/stack.h>
#include <bcm/cosq.h>
#include <bcm/trunk.h>
#include <bcm/mpls.h>

#include <soc/drv.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/qe2000_util.h>

#include <bcm_int/sbx/fe2000/allocator.h>

STATIC cmd_result_t _cmd_sbx_mpls_vpn_id(int unit, args_t *args);
STATIC cmd_result_t _cmd_sbx_mpls_vpn_port(int unit, args_t *args);
STATIC cmd_result_t _cmd_sbx_mpls_switch(int unit, args_t *args);

int
_cmd_sbx_mpls_route_print(int unit, 
                          bcm_mpls_tunnel_switch_t *info,
                          void *cookie);
int
_cmd_sbx_mpls_vpn_print(int unit, 
                        bcm_mpls_vpn_config_t *info,
                        void *cookie);

#define CMD_MPLS_SWITCH_USAGE                                           \
    "    switch [ add | delete | delete-all | get | get-all]\n"                  \
    "           [in-label=<in-label>] [gport=<in-port>]\n"              \
    "           [exp-map=<value>] [int-pri=value]\n"                    \
    "           [action=<SWAP|PHP|POP|POP-DIRECT]\n"                    \
    "           [vpn=<vpn>] [egress-if=<fte>]\n"

#define CMD_MPLS_VPN_ID_USAGE                                  \
    "    vpn-id [create | destroy | destroy-all | get | get-all]\n"      \
    "         [type=<L3|PWE|VPLS>] [lookup-id=<id>]\n"         \
    "         [broadcast-group=<mcg>] [vpn-id=<vpn>]\n"        \

#define CMD_MPLS_VPN_PORT_USAGE                                         \
    "    vpn-port  [add | delete | delete-all | get | get-all]\n"       \
    "         [vpn=<vpn>]\n"                                            \
    "         [mpls-port-id=<mpls-gport>] \n"                           \
    "         [exp-map=<value>] [int-pri=value]\n"                      \
    "         [port=<port>] [module=<module>]\n"                        \
    "         [match-criteria=<NONE|PORT|PORT-VLAN|PORT-SVLAN|LABEL|PORT-LABEL|LABEL-VLAN]\n" \
    "         [match-vlan=<vlan>][match-inner-vlan=<vlan>]\n"           \
    "         [match-label=<label>]\n"                                  \
    "         [service-tpid=<value>][service-vlan=<value>]\n"           \
    "         [egress-if=<fte>] [egress-label-val=<label-out>]\n"       \
    "         [egress-label-exp=<exp>][egress-label-ttl=<ttl>]\n"       \
    "         [encap-id=<ohi>]\n"

static cmd_t _cmd_sbx_mpls_list[] = {
    {"vpn-id",        _cmd_sbx_mpls_vpn_id,        "\n"  CMD_MPLS_VPN_ID_USAGE, NULL},
    {"vpn-port",      _cmd_sbx_mpls_vpn_port,      "\n"  CMD_MPLS_VPN_PORT_USAGE, NULL},
    {"switch",        _cmd_sbx_mpls_switch,        "\n"  CMD_MPLS_SWITCH_USAGE, NULL}

};

char cmd_sbx_mpls_usage[] =
    "\n"
#ifdef COMPILER_STRING_CONST_LIMIT
    "    mpls  <option> [args...]\n"
#else
    CMD_MPLS_VPN_ID_USAGE     "\n"
    CMD_MPLS_VPN_PORT_USAGE   "\n"
    CMD_MPLS_SWITCH_USAGE     "\n"
#endif
    ;

#define SBX_DISPLAY_MPLS_PORT(info)                                     \
    do {                                                                \
        printk("mpls_port_id(0x%x) flags(0x%x) if_class(0x%x) exp_map(0x%x)\n", \
               (info)->mpls_port_id, (info)->flags, (info)->if_class, (info)->exp_map); \
        printk("int_pri(0x%x) service_tpid(0x%x) port(0x%x) criteria(%d) match_vlan(0x%x)\n", \
               (info)->int_pri, (info)->service_tpid, (info)->port, (info)->criteria, (info)->match_vlan); \
        printk("match_inner_vlan(0x%x) match_label(0x%x) egress_tunnel_if(0x%x) egress_label(0x%x)\n", \
               (info)->match_inner_vlan, (info)->match_label, (info)->egress_tunnel_if, (info)->egress_label.label); \
        printk("egress_service_vlan(0x%x) encap_id(0x%x) failover_id(0x%x) failover_port_id(0x%x) policer_id(0x%x)\n", \
               (info)->egress_service_vlan, (info)->encap_id, (info)->failover_id, \
               (info)->failover_port_id, (info)->policer_id);           \
    } while(0)


STATIC cmd_result_t
_cmd_sbx_mpls_switch(int unit, args_t *args)
{
    int                      rv;
    cmd_result_t             retCode;
    parse_table_t            pt;
    uint32                   flags;
    char	            *subcmd;
    char                    *action_str;
    bcm_mpls_tunnel_switch_t info;
    
    if ((subcmd = ARG_GET(args)) == NULL) {
        printk("%s ERROR: empty cmd\n", __FUNCTION__);
        return CMD_FAIL;
    }

    flags          = 0;
    rv             = BCM_E_NONE;
    retCode        = CMD_OK;
    sal_memset(&info, 0, sizeof(info));
    parse_table_init(unit, &pt);
    
    if (sal_strcasecmp(subcmd, "add") == 0) {
        parse_table_add(&pt, "in-label",  PQ_DFL | PQ_HEX,    0, &info.label,   NULL);
        parse_table_add(&pt, "gport",     PQ_DFL | PQ_HEX,    0, &info.port,    NULL);
        parse_table_add(&pt, "exp-map",   PQ_DFL | PQ_INT,    0, &info.exp_map, NULL);
        parse_table_add(&pt, "int-pri",   PQ_DFL | PQ_INT,    0, &info.int_pri, NULL);
        parse_table_add(&pt, "vpn",       PQ_DFL | PQ_HEX,    0, &info.vpn,     NULL);
        parse_table_add(&pt, "action",             PQ_STRING, 0, &action_str,   NULL);
        parse_table_add(&pt, "egress-if", PQ_DFL | PQ_HEX,    0, &info.egress_if, NULL);
        if (!parse_arg_eq(args, &pt)) {
            printk("%s: Unknown options: %s\n",
                       ARG_CMD(args), ARG_CUR(args));
            parse_arg_eq_done(&pt);
            return(CMD_FAIL);
        }

        if (sal_strcasecmp(action_str, "pop") == 0) {
            info.action = BCM_MPLS_SWITCH_ACTION_POP;
        } else if (sal_strcasecmp(action_str, "swap") == 0) {
            info.action = BCM_MPLS_SWITCH_ACTION_SWAP;
        } else if (sal_strcasecmp(action_str, "php") == 0) {
            info.action = BCM_MPLS_SWITCH_ACTION_PHP;
        } else if (sal_strcasecmp(action_str, "pop-direct") == 0) {
            info.action = BCM_MPLS_SWITCH_ACTION_POP_DIRECT;
        } else {
            parse_arg_eq_done(&pt);
            printk("%s ERROR: Invalid action %s", __FUNCTION__, action_str);
            return CMD_FAIL;
        }
        parse_arg_eq_done(&pt);

        rv = bcm_mpls_tunnel_switch_add(unit, &info);

    } else if (sal_strcasecmp(subcmd, "delete") == 0) {
        parse_table_add(&pt, "in-label",  PQ_DFL | PQ_HEX,    0, &info.label, NULL);
        if (!parseEndOk(args, &pt, &retCode)) {
            return retCode;
        }
        printk("Deleting label 0x%x\n", info.label);

        rv = bcm_mpls_tunnel_switch_delete(unit, &info);
        
    } else if (sal_strcasecmp(subcmd, "delete-all") == 0) {
        parse_arg_eq_done(&pt);

        rv = bcm_mpls_tunnel_switch_delete_all(unit);
    } else if (sal_strcasecmp(subcmd, "get") == 0) {
        parse_table_add(&pt, "in-label",  PQ_DFL | PQ_HEX,    0, &info.label, NULL);
        if (!parseEndOk(args, &pt, &retCode)) {
            return retCode;
        }
        rv = bcm_mpls_tunnel_switch_get(unit, &info);
        _cmd_sbx_mpls_route_print(unit, &info, NULL);
    } else if (sal_strcasecmp(subcmd, "get-all") == 0) {
        parse_arg_eq_done(&pt);
        printk("Label\t port\t action\t egr_label\t egr_intf\n");

        rv = bcm_mpls_tunnel_switch_traverse(unit, _cmd_sbx_mpls_route_print, NULL);
    } else {
        parse_arg_eq_done(&pt);
        printk("%s ERROR: Invalid option %s", __FUNCTION__, subcmd);
        return CMD_FAIL;
    }

    if (rv == BCM_E_NONE) {
        rv = CMD_OK;
        printk("Command Successfull\n");
    } else {
        rv = CMD_FAIL;
        printk("Command failed\n");
    }
    
    return rv;
}

int
_cmd_sbx_mpls_route_print(int unit, bcm_mpls_tunnel_switch_t *info, void *cookie)
{
    printk("%05x    %07x  %02x      %05x      %05x\n",
           (uint32) info->label, (uint32) info->port, info->action,
           (uint32) info->egress_label.label, (uint32) info->egress_if);
    

    return BCM_E_NONE;
}

STATIC cmd_result_t
_cmd_sbx_mpls_vpn_id(int unit, args_t *args)
{
    cmd_result_t              retCode;
    int                       rv;
    parse_table_t             pt;
    char	             *subcmd;
    char                     *type_str;
    bcm_mpls_vpn_config_t     info;
    int                       vpn_id;
    
    if ((subcmd = ARG_GET(args)) == NULL) {
        printk("%s ERROR: empty cmd\n", __FUNCTION__);
        return CMD_FAIL;
    }

    rv        = BCM_E_NONE;
    vpn_id    = 0;
    retCode   = CMD_OK;
    sal_memset(&info, 0,  sizeof(bcm_mpls_vpn_config_t));
    
    parse_table_init(unit, &pt);
    if (sal_strcasecmp(subcmd, "create") == 0) {
        parse_table_add(&pt, "type",             PQ_STRING,     0, &type_str,       NULL);
        parse_table_add(&pt, "lookup-id",        PQ_DFL|PQ_INT, 0, &info.lookup_id, NULL);
        parse_table_add(&pt, "vpn-id",           PQ_DFL|PQ_HEX, 0, &vpn_id,         NULL);
        parse_table_add(&pt, "broadcast-group",  PQ_DFL|PQ_HEX, 0, &info.broadcast_group, NULL);
        if (!parse_arg_eq(args, &pt)) {
            printk("%s: Unknown options: %s\n",
                       ARG_CMD(args), ARG_CUR(args));
            parse_arg_eq_done(&pt);
            return(CMD_FAIL);
        }

        if (sal_strcasecmp(type_str, "L3") == 0) {
            info.flags |= BCM_MPLS_VPN_L3;
        } else if (sal_strcasecmp(type_str, "VPLS") == 0) {
            info.flags |= BCM_MPLS_VPN_VPLS;
        } else if (sal_strcasecmp(type_str, "PWE") == 0) {
            info.flags |= BCM_MPLS_VPN_VPWS;
        } else {
            parse_arg_eq_done(&pt);
            printk("%s ERROR: Invalid action %s", __FUNCTION__, type_str);
            return CMD_FAIL;
        }
        
        parse_arg_eq_done(&pt);
        info.vpn = (bcm_vpn_t) vpn_id;
        if (info.vpn) {
            info.flags |= BCM_MPLS_VPN_WITH_ID;
        }

        info.unknown_unicast_group = info.unknown_multicast_group =
            info.broadcast_group;
        
        printk("vpn-id create: lookup-id= %d type= %s vpn= 0x%x "
               "broadcast-group=0x%x flags= 0x%x\n",
               info.lookup_id, type_str, info.vpn, info.broadcast_group,
               info.flags);

        rv = bcm_mpls_vpn_id_create(unit, &info);
        if (rv == BCM_E_NONE) {
            printk("vpn-id 0x%x created successfully\n", info.vpn);
        } else {
            printk("could not create vpn-id error (%s)\n", bcm_errmsg(rv));
        }
        
    } else if (sal_strcasecmp(subcmd, "destroy") == 0) {
        parse_table_add(&pt, "vpn-id",    PQ_DFL|PQ_HEX, 0, &vpn_id, NULL);
        if (!parse_arg_eq(args, &pt)) {
            printk("%s: Unknown options: %s\n",
                       ARG_CMD(args), ARG_CUR(args));
            parse_arg_eq_done(&pt);
            return(CMD_FAIL);
        }
        parse_arg_eq_done(&pt);
        info.vpn = (bcm_vpn_t) vpn_id;
        rv = bcm_mpls_vpn_id_destroy(unit, info.vpn);
        if (rv == BCM_E_NONE) {
            printk("vpn-id 0x%x destroyed successfully\n",
                   info.vpn);
        } else {
            printk("error(%s) destroying vpn-id(0x%x)\n",
                   bcm_errmsg(rv), info.vpn);
        }
        
    } else if (sal_strcasecmp(subcmd, "destroy-all") == 0) {

        rv = bcm_mpls_vpn_id_destroy_all(unit);
        if (rv == BCM_E_NONE) {
            printk("all vpn-id destroyed successfully\n");
        } else {
            printk("error(%s) destroying all vpn(s)\n",
                   bcm_errmsg(rv));
        }
        
    } else if (sal_strcasecmp(subcmd, "get") == 0) {
        parse_table_add(&pt, "vpn-id",    PQ_DFL|PQ_HEX, 0, &vpn_id, NULL);
        if (!parse_arg_eq(args, &pt)) {
            printk("%s: Unknown options: %s\n",
                       ARG_CMD(args), ARG_CUR(args));
            parse_arg_eq_done(&pt);
            return(CMD_FAIL);
        }
        parse_arg_eq_done(&pt);

        rv = bcm_mpls_vpn_id_get(unit, (bcm_vpn_t)vpn_id, &info);
        if (rv == BCM_E_NONE) {
            printk("\nVPN Id= 0x%x\n", info.vpn);
            printk("\tVPN Lookup Id= 0x%x\n", info.lookup_id);
            if (info.flags & BCM_MPLS_VPN_L3) {
                printk("vpn-id(0x%x) lookup-id(0x%x) VPN Type= L3\n",
                       info.vpn, info.lookup_id);
            } else if (info.flags & BCM_MPLS_VPN_VPWS) {
                printk("vpn-id(0x%x) lookup-id(0x%x) VPN Type= PWE\n",
                       info.vpn, info.lookup_id);
            } else if (info.flags & BCM_MPLS_VPN_VPLS) {
                printk("vpn-id(0x%x) lookup-id(0x%x) VPN Type= VPLS\n",
                       info.vpn, info.lookup_id);
            } else {
                printk("vpn-id(0x%x) lookup-id(0x%x) unknown VPN type(0x%x)\n",
                       info.vpn, info.lookup_id, info.flags);
            }
        } else {
            printk("could not get vpn-info for vpn-id(0x%x) error (%s)\n",
                   (int)vpn_id, bcm_errmsg(rv));
        }
    } else if (sal_strcasecmp(subcmd, "get-all") == 0) {
        parse_arg_eq_done(&pt);
        rv = bcm_mpls_vpn_traverse(unit, _cmd_sbx_mpls_vpn_print, NULL);
        if (rv != BCM_E_NONE) {
            printk("could not get vpn-info error (%s)\n", bcm_errmsg(rv));
        }
    } else {
        printk("%s ERROR: Invalid option %s", __FUNCTION__, subcmd);
        parse_arg_eq_done(&pt);
        return CMD_FAIL;
    }
    
    parse_arg_eq_done(&pt);

    return CMD_OK;
}

int
_cmd_sbx_mpls_vpn_print(int unit, 
                          bcm_mpls_vpn_config_t *info,
                          void *cookie)
{
    if (info->flags & BCM_MPLS_VPN_L3) {
        printk("vpn-id(0x%x) lookup-id(0x%x) VPN Type= L3\n",
                info->vpn, info->lookup_id);
    } else if (info->flags & BCM_MPLS_VPN_VPWS) {
        printk("vpn-id(0x%x) lookup-id(0x%x) VPN Type= PWE\n",
                info->vpn, info->lookup_id);
    } else if (info->flags & BCM_MPLS_VPN_VPLS) {
        printk("vpn-id(0x%x) lookup-id(0x%x) VPN Type= VPLS\n",
                info->vpn, info->lookup_id);
    } else {
        printk("vpn-id(0x%x) lookup-id(0x%x) unknown VPN type(0x%x)\n",
                info->vpn, info->lookup_id, info->flags);
    }
    return BCM_E_NONE;

}

STATIC cmd_result_t
_cmd_sbx_mpls_vpn_port(int unit, args_t *args)
{
    cmd_result_t       retCode;
    int                rv;
    parse_table_t      pt;
    char	      *subcmd;
    bcm_mpls_port_t    info;
    int                vpn_int;
    bcm_vpn_t          vpn;
    bcm_port_t         port;
    bcm_module_t       module;
    bcm_gport_t        mpls_port_id = BCM_GPORT_INVALID;
    bcm_mpls_port_t   *all_ports;
    int                i, port_count;
    int                match_vlan, match_inner_vlan;
    char              *match_str;
    
    if ((subcmd = ARG_GET(args)) == NULL) {
        printk("%s ERROR: empty cmd\n", __FUNCTION__);
        return CMD_FAIL;
    }

    match_vlan = match_inner_vlan = BCM_VLAN_INVALID;
    rv      = BCM_E_NONE;
    retCode = CMD_OK;
    bcm_mpls_port_t_init(&info);
    parse_table_init(unit, &pt);
    
    if (sal_strcasecmp(subcmd, "add") == 0) {
    
        parse_table_add(&pt, "vpn",              PQ_DFL|PQ_HEX, 0, &vpn_int, NULL);
        parse_table_add(&pt, "mpls-port-id",     PQ_DFL|PQ_HEX, 0, &info.mpls_port_id,     NULL);
        parse_table_add(&pt, "port",             PQ_DFL|PQ_INT, 0, &port,                  NULL);
        parse_table_add(&pt, "module",           PQ_DFL|PQ_INT, 0, &module,                NULL);
        parse_table_add(&pt, "match-criteria",   PQ_STRING,     0, &match_str,             NULL);
        parse_table_add(&pt, "match-vlan",       PQ_DFL|PQ_INT, 0, &match_vlan,            NULL);
        parse_table_add(&pt, "match-inner-vlan", PQ_DFL|PQ_INT, 0, &match_inner_vlan,      NULL);
        parse_table_add(&pt, "match-label",      PQ_DFL|PQ_INT, 0, &info.match_label,      NULL);
        parse_table_add(&pt, "service-tpid",     PQ_DFL|PQ_INT, 0, &info.service_tpid,     NULL);
        parse_table_add(&pt, "service-vlan",     PQ_DFL|PQ_INT, 0, &info.egress_service_vlan, NULL);
        parse_table_add(&pt, "egress-if",        PQ_DFL|PQ_HEX, 0, &info.egress_tunnel_if, NULL);
        parse_table_add(&pt, "egress-label-val", PQ_DFL|PQ_HEX, 0, &info.egress_label.label, NULL);
        parse_table_add(&pt, "egress-label-exp", PQ_DFL|PQ_HEX, 0, &info.egress_label.exp, NULL);
        parse_table_add(&pt, "egress-label-ttl", PQ_DFL|PQ_HEX, 0, &info.egress_label.ttl, NULL);
        parse_table_add(&pt, "encap-id",         PQ_DFL|PQ_HEX, 0, &info.encap_id,         NULL);
        if (!parse_arg_eq(args, &pt)) {
            printk("%s: Unknown options: %s\n",
                       ARG_CMD(args), ARG_CUR(args));
            parse_arg_eq_done(&pt);
            return(CMD_FAIL);
        }
        
        if (vpn_int > 0xFFFF) {
            return (CMD_FAIL);
        }
        vpn = vpn_int;
        _SHR_GPORT_MODPORT_SET(info.port, module, port);
        if (sal_strcasecmp(match_str, "none") == 0) {
            info.criteria = BCM_MPLS_PORT_MATCH_NONE;
        } else if (sal_strcasecmp(match_str, "port") == 0) {
            info.criteria = BCM_MPLS_PORT_MATCH_PORT;
        } else if (sal_strcasecmp(match_str, "port-vlan") == 0) {
            info.criteria = BCM_MPLS_PORT_MATCH_PORT_VLAN;
        } else if (sal_strcasecmp(match_str, "port-svlan") == 0) {
            info.criteria = BCM_MPLS_PORT_MATCH_PORT_VLAN_STACKED;
        } else if (sal_strcasecmp(match_str, "label") == 0) {
            info.criteria = BCM_MPLS_PORT_MATCH_LABEL;
        } else if (sal_strcasecmp(match_str, "port-label") == 0) {
            info.criteria = BCM_MPLS_PORT_MATCH_LABEL_PORT;
        } else if (sal_strcasecmp(match_str, "label-vlan") == 0) {
            info.criteria = BCM_MPLS_PORT_MATCH_LABEL_VLAN;
        } else {
            parse_arg_eq_done(&pt);
            printk("%s ERROR: Invalid action %s", __FUNCTION__, match_str);
            return CMD_FAIL;
        }
        parse_arg_eq_done(&pt);
       
        info.match_vlan       = match_vlan;
        info.match_inner_vlan = match_inner_vlan; 

        rv = bcm_mpls_port_add(unit, vpn, &info);
        if (rv == BCM_E_NONE) {
            printk("mpls vpn(0x%x) port added: mpls-port-id(0x%x), encap-id(0x%x)\n",
                   vpn, info.mpls_port_id, info.encap_id);
        } else {
            printk("error(%s) adding port to vpn(0x%x)\n", bcm_errmsg(rv), vpn);
        }
        
    } else if (sal_strcasecmp(subcmd, "delete") == 0) {
        parse_table_add(&pt, "vpn",              PQ_DFL|PQ_HEX, 0, &vpn_int,          NULL);
        parse_table_add(&pt, "mpls-port-id",     PQ_DFL|PQ_HEX, 0, &mpls_port_id,     NULL);
        if (!parse_arg_eq(args, &pt)) {
            printk("%s: Unknown options: %s\n",
                       ARG_CMD(args), ARG_CUR(args));
            parse_arg_eq_done(&pt);
            return(CMD_FAIL);
        }
        parse_arg_eq_done(&pt);

        if (vpn_int > 0xFFFF) {
            return (CMD_FAIL);
        }
        vpn = vpn_int;
        
        rv = bcm_mpls_port_delete(unit, vpn, mpls_port_id);
        if (rv == BCM_E_NONE) {
            printk("mpls-port-id(0x%x) deleted from vpn(0x%x)\n",
                   mpls_port_id, vpn);
        } else {
            printk("error(%s) deleting mpls-port-id(0x%x) from vpn(0x%x)\n",
                   bcm_errmsg(rv), mpls_port_id, vpn);
        }
    } else if (sal_strcasecmp(subcmd, "delete-all") == 0) {
        parse_table_add(&pt, "vpn",              PQ_DFL|PQ_HEX, 0, &vpn_int,              NULL);
        if (!parse_arg_eq(args, &pt)) {
            printk("%s: Unknown options: %s\n",
                       ARG_CMD(args), ARG_CUR(args));
            parse_arg_eq_done(&pt);
            return(CMD_FAIL);
        }
        parse_arg_eq_done(&pt);

        if (vpn_int > 0xFFFF) {
            return (CMD_FAIL);
        }
        vpn = vpn_int;
        
        rv = bcm_mpls_port_delete_all(unit, vpn);
        if (rv == BCM_E_NONE) {
            printk("all ports successfully deleted from vpn(0x%x)\n",
                   vpn);
        } else {
            printk("error(%s) deleting all ports from vpn(0x%x)\n",
                   bcm_errmsg(rv), vpn);
        }
    } else if (sal_strcasecmp(subcmd, "get") == 0) {
        int port=0, module=0;
        info.mpls_port_id = 0;

        parse_table_add(&pt, "vpn",              PQ_DFL|PQ_HEX, 0, &vpn_int,               NULL);
        parse_table_add(&pt, "mpls-port-id",     PQ_DFL|PQ_HEX, 0, &info.mpls_port_id,     NULL);
        parse_table_add(&pt, "match-criteria",   PQ_DFL|PQ_STRING, 0, &match_str, NULL);
        parse_table_add(&pt, "match-vlan",   PQ_DFL|PQ_HEX, 0, &info.match_vlan, NULL);
        parse_table_add(&pt, "match-label",   PQ_DFL|PQ_HEX, 0, &info.match_label, NULL);
        parse_table_add(&pt, "port",   PQ_DFL|PQ_HEX, 0, &port, NULL);
        parse_table_add(&pt, "module",   PQ_DFL|PQ_HEX, 0, &module, NULL);

        if (!parse_arg_eq(args, &pt)) {
            printk("%s: Unknown options: %s\n",
                       ARG_CMD(args), ARG_CUR(args));
            parse_arg_eq_done(&pt);
            return(CMD_FAIL);
        }

        if (sal_strcasecmp(match_str, "port") == 0) {
            info.criteria = BCM_MPLS_PORT_MATCH_PORT;
        } else if (sal_strcasecmp(match_str, "port-vlan") == 0) {
            info.criteria = BCM_MPLS_PORT_MATCH_PORT_VLAN;
        } else if (sal_strcasecmp(match_str, "label") == 0) {
            info.criteria = BCM_MPLS_PORT_MATCH_LABEL;
        } else {
            parse_arg_eq_done(&pt);
            printk("%s ERROR: Invalid action %s", __FUNCTION__, match_str);
            return CMD_FAIL;
        }
        parse_arg_eq_done(&pt);

        if (vpn_int > 0xFFFF) {
            return (CMD_FAIL);
        }

        vpn = vpn_int;
        BCM_GPORT_MODPORT_SET(info.port, module, port);

        rv = bcm_mpls_port_get(unit, vpn, &info);
        if (rv == BCM_E_NONE) {
            printk("mpls-port-id(0x%x) successfully got from vpn(0x%x)\n",
                   info.mpls_port_id, vpn);
            SBX_DISPLAY_MPLS_PORT(&info);
        } else {
            printk("error(%s) port-get for mpls-port-id(0x%x) in vpn(0x%x)\n",
                   bcm_errmsg(rv), mpls_port_id, vpn);
        }
    } else if (sal_strcasecmp(subcmd, "get-all") == 0) {
        parse_table_add(&pt, "vpn",              PQ_DFL|PQ_HEX, 0, &vpn_int,                   NULL);
        if (!parse_arg_eq(args, &pt)) {
            printk("%s: Unknown options: %s\n",
                       ARG_CMD(args), ARG_CUR(args));
            parse_arg_eq_done(&pt);
            return(CMD_FAIL);
        }
        parse_arg_eq_done(&pt);

        if (vpn_int > 0xFFFF) {
            return (CMD_FAIL);
        }

        vpn = vpn_int;
        
        all_ports = sal_alloc(sizeof(bcm_mpls_port_t) * SBX_MAX_PORTS, "tmp-mpls-ports");
        if (all_ports == NULL) {
            return CMD_FAIL;
        }
        
        port_count = 0;
        rv = bcm_mpls_port_get_all(unit, vpn, SBX_MAX_PORTS, all_ports, &port_count);
        if (rv == BCM_E_NONE) {
            printk("successfully got %d entries from vpn(0x%x)\n",
                   port_count, vpn);
            for (i = 0; i < port_count; i++) {
                SBX_DISPLAY_MPLS_PORT(&all_ports[i]);
            }
        } else {
            printk("error(%s) port-get-all for vpn(0x%x)\n",
                   bcm_errmsg(rv), vpn);
        }
        sal_free(all_ports);
    } else {
        printk("%s ERROR: Invalid option %s", __FUNCTION__, subcmd);
        return CMD_FAIL;
    }
    
    return (rv == BCM_E_NONE) ? CMD_OK : CMD_FAIL;
}

    
cmd_result_t
cmd_sbx_mpls(int unit, args_t *args)
{
    return subcommand_execute(unit, args, 
                              _cmd_sbx_mpls_list,
                              COUNTOF(_cmd_sbx_mpls_list));
}

        
/*------------------------------------------------
gdb bcm.user.dbg
b _bcm_fe2000_mpls_tunnel_switch_update
b bcm_fe2000_mpls_tunnel_switch_add
b bcm_fe2000_mpls_vpn_id_create
b bcm_fe2000_mpls_port_add
b bcm_fe2000_mpls_port_delete
b _bcm_fe2000_mpls_vpls_encap_get
handle SIG32 noprint nostop
run

  Test for L1 Ports

  # create vpn and check for proper table
  # programming
  # verify that vlan->dontlearn is set
  # routedvlan->vrf is drop-vrf fte->drop is set
  mpls vpn-id create type=PWE
  gu2get vlan2etc vlan 0xfffe
  gu2get routedvlan2etc idx 0xfffe
  gu2get fte idx 0xfffe

  # get out modid
  tcl
  set modid -1
  bcm_stk_my_modid_get 1 modid
  puts $modid
  exit
 
  # Simple case P (1) <---> P (2) 
  mpls vpn-port add vpn=0xfffe port=1 module=0 match-criteria=PORT
  mpls vpn-port add vpn=0xfffe port=2 module=0 match-criteria=PORT

  gu2get port2etc port 1
  gu2get port2etc port 2
  gu2get pvid2etc port 1 vid 1
  # error out adding > 2 ports to PWE
  mpls vpn-port add vpn=0xfffe port=3 module=0 match-criteria=PORT

  # delete port
  mpls vpn-port delete vpn=0xfffe port=1 module=0 match-criteria=PORT
  mpls vpn-port delete vpn=0xfffe port=2 module=0 match-criteria=PORT


  # Create all flavors of L1 Port PWE
  # Case P,Vid (1,100) <---> P,Vid (1,200)
  mpls vpn-port add vpn=0xfffe port=1 match-vlan=100 module=0 match-criteria=PORT-VLAN
  mpls vpn-port add vpn=0xfffe port=1 match-vlan=200 module=0 match-criteria=PORT-VLAN
  gu2get pvid2etc port 1 vid 100
  gu2get vlan2etc vlan 0x3000
  gu2get routedvlan2etc idx 0x3000
  gu2get fte idx 0x3000
  gu2get pvid2etc port 1 vid 200
  gu2get vlan2etc vlan 0x2fff
  gu2get routedvlan2etc idx 0x2fff
  gu2get fte idx 0x2fff
  ------------------------------------------------*/
