/*
 * $Id: oam.c 1.15 Broadcom SDK $
 * 
 *  OAM diag shell command for SBX devices
 *
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
 */

#include <appl/diag/system.h>
#include <appl/diag/parse.h>

#include <bcm/error.h>
#include <bcm/oam.h>
#include <bcm/stack.h>
#include <bcm/failover.h>

#include <soc/sbx/sbx_drv.h>

typedef enum _oam_ep_type_s {
    _ep_invalid, _ep_enet, _ep_bfd, _ep_mpls, _ep_max
} _oam_ep_type_t;

static _oam_ep_type_t sbx_last_ep_dump_type;


#define _is_ascii(x) (((x) >= ' ') && ((x) <= '~'))

#define GROUP_LIST_HEADER \
"\n ID    Name                                            Tx_RDI Flts\n" \
"----- ------------------------------------------------ ------ ----\n"

static int
_cmd_sbx_oam_group_print(int unit, bcm_oam_group_info_t *info, void *data)
{
    char name[BCM_OAM_GROUP_NAME_LENGTH + 1];
    int i;

    for (i = 0; i < BCM_OAM_GROUP_NAME_LENGTH; i++) {
        name[i] = _is_ascii(info->name[i]) ? info->name[i] : '.';
    }
    name[i] = 0;

    printk("%-5d %48s %-3c    %04x\n", info->id, name,
           (info->flags & BCM_OAM_GROUP_REMOTE_DEFECT_TX) ? '*' : ' ', info->faults);

    return BCM_E_NONE;
}


int
_cmd_sbx_oam_group_visit(int unit, bcm_oam_group_info_t *info, void *data)
{
    bcm_oam_endpoint_traverse_cb visit_f = (bcm_oam_endpoint_traverse_cb)data;
    return bcm_oam_endpoint_traverse(unit, info->id, visit_f, data);
}


cmd_result_t
_cmd_sbx_oam_grp_add_replace(int unit, args_t *args, int replace)
{
    int                  rv, rdi = 0;
    char                *name = NULL;
    bcm_oam_group_info_t grpInfo;
    parse_table_t        pt;

    if (ARG_CNT(args) == 0) {
        return CMD_USAGE;
    }
    
    bcm_oam_group_info_t_init(&grpInfo);
    grpInfo.id = BCM_OAM_GROUP_INVALID;

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "Name", PQ_STRING | PQ_DFL, 0, &name, NULL);
    parse_table_add(&pt, "ID", PQ_INT | PQ_DFL, 0, &grpInfo.id, NULL);
    parse_table_add(&pt, "RemoteDefect", PQ_BOOL | PQ_NO_EQ_OPT | PQ_DFL, 
                    0, &rdi, NULL);

    /* parseEndOk frees memory allocated by the parser to store strings,
     * use parse_arg_eq & parse_arg_eq_done to keep values intact.
     */
    if (parse_arg_eq(args, &pt) < 0) {
        printk("%s: Error: Unknown option: %s\n", ARG_CMD(args), ARG_CUR(args));
	parse_arg_eq_done(&pt);
	return CMD_FAIL;
    }

    if (name == NULL) {
        printk("Group name is required\n");
        parse_arg_eq_done(&pt);
        return CMD_FAIL;
    }

    sal_strncpy((char*)grpInfo.name, name, BCM_OAM_GROUP_NAME_LENGTH - 1);
    parse_arg_eq_done(&pt);

    if (grpInfo.id != BCM_OAM_GROUP_INVALID) {
        grpInfo.flags |= BCM_OAM_GROUP_WITH_ID;
    }

    grpInfo.flags |= rdi ? BCM_OAM_GROUP_REMOTE_DEFECT_TX : 0;
    grpInfo.flags |= replace ? BCM_OAM_GROUP_REPLACE : 0;
    
    rv = bcm_oam_group_create(unit, &grpInfo);
    if (BCM_FAILURE(rv)) {
        printk("Failed to create group: %s\n", bcm_errmsg(rv));
        return CMD_FAIL;
    }

    printk("%s Group ID=%d\n", replace ? "Replaced" : "Created", grpInfo.id);

    return CMD_OK;
}

#define CMD_OAM_USAGE_GRP_ADD \
"    oam group add [ID=<id>] Name=<string> [RemoteDefect]"

cmd_result_t 
_cmd_sbx_oam_grp_add(int unit, args_t *args)
{
    return _cmd_sbx_oam_grp_add_replace(unit, args, FALSE);
}


#define CMD_OAM_USAGE_GRP_DELETE \
"    oam group delete <id>"

cmd_result_t
_cmd_sbx_oam_grp_delete(int unit, args_t *args)
{
    int rv;
    bcm_oam_group_t id;

    if (ARG_CNT(args) == 0) {
        printk("Must supply Group Id to delete.\n");
        return CMD_FAIL;
    } else if (ARG_CNT(args) > 1) {
        printk("Invalid arguments found.\n");
        return CMD_FAIL;
    }

    id = parse_integer(ARG_GET(args));

    rv = bcm_oam_group_destroy(unit, id);
    if (BCM_FAILURE(rv)) {
        printk("Failed to destroy group %d: %s\n", id, bcm_errmsg(rv));
        return CMD_FAIL;
    }
    
    return CMD_OK;
}


#define CMD_OAM_USAGE_GRP_REPLACE \
"    oam group replace [ID=<id>] Name=<string> [RemoteDefect]"

cmd_result_t
_cmd_sbx_oam_grp_replace(int unit, args_t *args)
{
    return _cmd_sbx_oam_grp_add_replace(unit, args, TRUE);
}


#define CMD_OAM_USAGE_GRP_SHOW \
"    oam group show [<id>]"

cmd_result_t
_cmd_sbx_oam_grp_show(int unit, args_t *args)
{
    int rv;

    printk(GROUP_LIST_HEADER);

    if (ARG_CNT(args) == 0) {    
        rv = bcm_oam_group_traverse(unit, _cmd_sbx_oam_group_print, NULL);
        if (BCM_FAILURE(rv)) {
            printk("Failed to travers OAM groups: %s\n", bcm_errmsg(rv));
            return CMD_FAIL;
        }

    } else if (ARG_CNT(args) == 1) {
        bcm_oam_group_info_t info;

        bcm_oam_group_info_t_init(&info);
        info.id = parse_integer(ARG_GET(args));

        rv = bcm_oam_group_get(unit, info.id, &info);
        if (BCM_FAILURE(rv)) {
            printk("Failed to get group %d: %s\n", info.id, bcm_errmsg(rv));
            return CMD_FAIL;
        }

        rv = _cmd_sbx_oam_group_print(unit, &info, NULL);
        if (BCM_FAILURE(rv)) {
            printk("Failed to print group info: %s\n", bcm_errmsg(rv));
            return CMD_FAIL;
        }
    } else {
        printk("Invalid arguments found.\n");
        return CMD_FAIL;
    }

    return CMD_OK;
}


#define CMD_OAM_USAGE_GRP    \
CMD_OAM_USAGE_GRP_ADD "\n"           \
CMD_OAM_USAGE_GRP_DELETE "\n"        \
CMD_OAM_USAGE_GRP_REPLACE "\n"       \
CMD_OAM_USAGE_GRP_SHOW "\n"


static cmd_t _cmd_sbx_oam_grp_list[] = {
    {"add",      _cmd_sbx_oam_grp_add,     "\n" CMD_OAM_USAGE_GRP_ADD, NULL},
    {"delete",   _cmd_sbx_oam_grp_delete,  "\n" CMD_OAM_USAGE_GRP_DELETE, NULL},
    {"replace",  _cmd_sbx_oam_grp_replace, "\n" CMD_OAM_USAGE_GRP_REPLACE, NULL},
    {"show",     _cmd_sbx_oam_grp_show,    "\n" CMD_OAM_USAGE_GRP_SHOW, NULL},
};

cmd_result_t
_cmd_sbx_oam_grp(int unit, args_t *args)
{
    return subcommand_execute(unit, args, 
                              _cmd_sbx_oam_grp_list, COUNTOF(_cmd_sbx_oam_grp_list));
}

#define ENET_ENDPOINT_LIST_HEADER \
    "\n ID   Group Name  LocId Lvl Period Pri Flgs Flts Vlan Mod Port Dir   Src MAC address   Dst MAC address  I/R/TxEna\n" \
      "----- ----- ----- ----- --- ------ --- ---- ---- ---- --- ---- ---- ----------------- ----------------- ---------\n"

static int 
_cmd_sbx_oam_enet_ep_print(int unit, bcm_oam_endpoint_info_t *info, void *data)
{
    char macAddressStr[MACADDR_STR_LEN];

    printk("%-5d %-5d %-5d %-5d %2d   %-5d  %-2d %04x %04x ",
           info->id, info->group, info->name, (info->flags & BCM_OAM_ENDPOINT_REMOTE) ? info->local_id : 0,
           info->level, info->ccm_period, info->pkt_pri, info->flags, info->faults);

    if (info->vlan != BCM_VLAN_NONE) {
        printk("%-4d ", info->vlan);
    } else {
        printk("%4s " , "");
    }

    if (BCM_GPORT_IS_MODPORT(info->gport)) {
        printk(" %-2d  %-3d ", BCM_GPORT_MODPORT_MODID_GET(info->gport),
               BCM_GPORT_MODPORT_PORT_GET(info->gport));
    } else if (BCM_GPORT_IS_LOCAL(info->gport)) {
        printk(" %-2s  %-3d ", "", BCM_GPORT_LOCAL_GET(info->gport));
    } else {
        printk("%3s %4s ", "", "");
    }

    if (info->flags & BCM_OAM_ENDPOINT_UP_FACING) {
        printk("%-4s ", "up");
    } else {
        printk("%4s ", "down");
    }

    format_macaddr(macAddressStr, info->src_mac_address);
    printk("%17s ", macAddressStr);

    format_macaddr(macAddressStr, info->dst_mac_address);
    printk("%17s  ", macAddressStr);

    if (info->flags & BCM_OAM_ENDPOINT_REMOTE) {
        printk("%-8s ", "remote");  

    } else if (info->flags & BCM_OAM_ENDPOINT_INTERMEDIATE) {
        printk("%-8s ", "inter");  

    } else {
        if (info->flags & (BCM_OAM_ENDPOINT_PORT_STATE_TX |
                           BCM_OAM_ENDPOINT_INTERFACE_STATE_TX)) {
           printk("%-8s ", "local/Tx");
        }
        else {
           printk("%-8s ", "local");
        }
    } 

    printk("\n");
    return BCM_E_NONE;
}

#define MPLS_PERF_ENDPOINT_LIST_HEADER \
    "\n ID   Group Name  LocId   GPORT      Intf     Flgs  Dir  Type \n" \
      "----- ----- ----- ----- ---------- ---------- ----- ---- ---- \n"

static int 
_cmd_sbx_oam_mpls_ep_print(int unit, bcm_oam_endpoint_info_t *info, void *data)
{

    printk("%-5d %-5d %-5x %-5d %-10x %-10x %04x ",
           info->id, info->group, info->name, (info->flags & BCM_OAM_ENDPOINT_REMOTE) ? info->local_id : 0,
           info->gport, info->intf_id, info->flags);

    if (info->flags & BCM_OAM_ENDPOINT_UP_FACING) {
        printk("%-4s ", "up");
    } else {
        printk("%4s ", "down");
    }

    printk("%4d", info->type);
    printk("\n");
    return BCM_E_NONE;
}

#define BFD_ENDPOINT_LIST_HEADER \
    "\n ID   Group Name  LocId   GPORT    EP-Flgs BFDFlg State Diag Dir  Type   DIP    Period Mult\n" \
      "----- ----- ----- ----- ---------- ------- ------ ----- ---- ---- ---- -------- ------ ----\n"
static int 
_cmd_sbx_oam_bfd_ep_print(int unit, bcm_oam_endpoint_info_t *info, void *data)
{
    printk("%-5d %-5d %-5x %-5d %-10x %7x %6x %5d %4d",
           info->id, info->group, info->name, (info->flags & BCM_OAM_ENDPOINT_REMOTE) ? info->local_id : 0,
           info->gport, info->flags, info->bfd_flags, info->bfd_state, info->diag_code);

    if (info->flags & BCM_OAM_ENDPOINT_UP_FACING) {
        printk("%-4s ", "up");
    } else {
        printk("%4s ", "down");
    }

    printk("%4d %8x ", info->type, info->dest_ip);
    printk("%6d %4d",
    (info->flags & BCM_OAM_ENDPOINT_REMOTE)? info->min_tx_interval:info->min_rx_interval,
           info->detect_mult);       
    printk("\n");
    return BCM_E_NONE;
}

int
_cmd_sbx_oam_ep_print(int unit, bcm_oam_endpoint_info_t *info, void *data)
{
    switch(info->type) {
        case bcmOAMEndpointTypeEthernet:
            if(sbx_last_ep_dump_type != _ep_enet) {
                printk(ENET_ENDPOINT_LIST_HEADER);
            }
            sbx_last_ep_dump_type = _ep_enet;
            _cmd_sbx_oam_enet_ep_print(unit, info, data);
            break;

        case bcmOAMEndpointTypeMPLSPerformance:
            if(sbx_last_ep_dump_type != _ep_mpls) {
                printk(MPLS_PERF_ENDPOINT_LIST_HEADER);
            }
            sbx_last_ep_dump_type = _ep_mpls;
            _cmd_sbx_oam_mpls_ep_print(unit, info, data);
            break;

        case bcmOAMEndpointTypeBFDVccv:
        case bcmOAMEndpointTypeBFDVccvCcCv:
        case bcmOAMEndpointTypeBFDMPLS:
        case bcmOAMEndpointTypeBFDMPLSCcCv:
            if(sbx_last_ep_dump_type != _ep_bfd) {
                printk(BFD_ENDPOINT_LIST_HEADER);
            }
            sbx_last_ep_dump_type = _ep_bfd;
            _cmd_sbx_oam_bfd_ep_print(unit, info, data);
            break;

        default:
            printk("!!Unsupported Endpoint Type \n");
            sbx_last_ep_dump_type = _ep_invalid;
            break;
    }
    return BCM_E_NONE;
}


cmd_result_t
_cmd_sbx_oam_ep_add_replace(int unit, args_t *args, int replace)
{
    int                      rv;
    bcm_oam_endpoint_info_t  info;
    int                      remote = 0;
    int                      txEnable = 0;
    int                      intermediate = 0;
    int                      direction = 0;
    int                      name, port = -1;
    int                      vid;
    cmd_result_t             retCode;
    parse_table_t            pt;
    bcm_mod_port_t           modPort = {-1, -1};

    if (ARG_CNT(args) == 0) {
        return CMD_USAGE;
    }
    bcm_oam_endpoint_info_t_init(&info);
    info.id       = BCM_OAM_ENDPOINT_INVALID;
    info.local_id = BCM_OAM_ENDPOINT_INVALID;
    info.group = BCM_OAM_GROUP_INVALID;

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "Name", PQ_INT, (void*)-1, &name, NULL);
    parse_table_add(&pt, "ID",    PQ_INT | PQ_DFL, 0, &info.id, NULL);
    parse_table_add(&pt, "LOCALid",    PQ_INT | PQ_DFL, 0, &info.local_id, NULL);
    parse_table_add(&pt, "INTermediate", PQ_BOOL | PQ_NO_EQ_OPT,
                    0, &intermediate, NULL);
    parse_table_add(&pt, "Group", PQ_INT | PQ_DFL, 0, &info.group, NULL);
    parse_table_add(&pt, "PERiod", PQ_INT | PQ_DFL, 0, &info.ccm_period, NULL);
    parse_table_add(&pt, "Vlan",    PQ_INT | PQ_DFL, 0, &vid, NULL);
    parse_table_add(&pt, "ModPort", PQ_MOD_PORT | PQ_DFL, 0, &modPort, NULL);
    parse_table_add(&pt, "POrt",    PQ_PORT | PQ_DFL, 0, &port, NULL);
    parse_table_add(&pt, "Level", PQ_INT, (void*)-1, &info.level, NULL);
    parse_table_add(&pt, "SrcMACaddress", PQ_MAC | PQ_DFL, 
                    0, &info.src_mac_address, NULL);
    parse_table_add(&pt, "DstMACaddress", PQ_MAC | PQ_DFL,
                    0, &info.dst_mac_address, NULL);
    parse_table_add(&pt, "Remote",   PQ_BOOL | PQ_NO_EQ_OPT, 0, &remote, NULL);
    parse_table_add(&pt, "TXENAble", PQ_BOOL | PQ_NO_EQ_OPT,
                    0, &txEnable, NULL);
    parse_table_add(&pt, "Up",   PQ_BOOL | PQ_NO_EQ_OPT, 0, &direction, NULL);

    if (!parseEndOk(args, &pt, &retCode)) {
        return retCode;
    }
    info.name = (uint16_t)name;

    if (modPort.mod == -1) {
        rv = bcm_stk_my_modid_get(unit, &modPort.mod);
        if (BCM_FAILURE(rv)) {
            printk("Failed to get modid: %s\n", bcm_errmsg(rv));
            return CMD_FAIL;
        }
    }

    info.vlan = (bcm_vlan_t)vid;

    if (port != -1) {
        modPort.port = port;
    }

    if (modPort.port == -1) {
        printk("Must specify a Port.\n");
        return CMD_FAIL;
    }

    BCM_GPORT_MODPORT_SET(info.gport, modPort.mod, modPort.port);

    if (remote && !replace && info.local_id == BCM_OAM_ENDPOINT_INVALID) {
        printk("Must supply Local Endpoint ID when creating"
               " remote endpoint.\n");
        return CMD_FAIL;
    }

    info.flags |= ((info.id != BCM_OAM_ENDPOINT_INVALID) ?
                       BCM_OAM_ENDPOINT_WITH_ID : 0);

    info.flags |= replace ? BCM_OAM_ENDPOINT_REPLACE : 0;
    info.flags |= remote ? BCM_OAM_ENDPOINT_REMOTE : 0;
    info.flags |= txEnable ?  BCM_OAM_ENDPOINT_PORT_STATE_TX : 0;
    info.flags |= intermediate ? BCM_OAM_ENDPOINT_INTERMEDIATE : 0;
    info.flags |= direction ? BCM_OAM_ENDPOINT_UP_FACING : 0;

    info.type = bcmOAMEndpointTypeEthernet;

    rv = bcm_oam_endpoint_create(unit, &info);
    if (BCM_FAILURE(rv)) {
        printk("Failed to create endpoint: %s\n", bcm_errmsg(rv));
        return CMD_FAIL;
    }

    printk("%s endpoint ID=%d\n", replace ? "Replaced" : "Created", info.id);

    return CMD_OK;
}


#define CMD_OAM_USAGE_EP_ADD \
"    oam EndPoint add Group=<group id> [ID=<id>] Name=<name> Level=<level>\n" \
"                     PERiod=<period in ms> ModPort=<modid.port>|POrt=<Port>\n" \
"                     [Vlan=<vlan>] [SrcMACaddress=<mac>] [DstMACaddress=<mac>] [LOCALid=<id>]\n" \
"                     [TXENAble] [Remote] [INTermediate] [Up]"

cmd_result_t
_cmd_sbx_oam_ep_add(int unit, args_t *args)
{
    return _cmd_sbx_oam_ep_add_replace(unit, args, FALSE);
}


#define CMD_OAM_USAGE_EP_DELETE \
"    oam EndPoint delete <id>"

cmd_result_t
_cmd_sbx_oam_ep_delete(int unit, args_t *args)
{
    int rv, id;
    
    if (ARG_CNT(args) != 1) {
        printk("Must specify endpoint to delete.\n");
        return CMD_FAIL;
    }

    id = parse_integer(ARG_GET(args));

    rv = bcm_oam_endpoint_destroy(unit, id);
    if (BCM_FAILURE(rv)) {
        printk("Failed to destroy endpoint %d: %s\n", id, bcm_errmsg(rv));
        return CMD_FAIL;
    }

    printk("Endpoint %d destroyed.\n", id);
    return CMD_OK;
}


#define CMD_OAM_USAGE_EP_REPLACE \
"    oam EndPoint replace Group=<group id> [ID=<id>] Name=<name> Level=<level>\n" \
"                         PERiod=<period in ms> ModPort=<modid.port>|POrt=<Port>\n" \
"                         [Vlan=<vlan>] [SrcMACaddress=<mac>] [DstMACaddress=<mac>] [LOCALid=<id>]\n" \
"                         [TXENAble] [Remote] [INTermediate] [Up]"

cmd_result_t
_cmd_sbx_oam_ep_replace(int unit, args_t *args)
{
    return _cmd_sbx_oam_ep_add_replace(unit, args, TRUE);
}


#define CMD_OAM_USAGE_EP_SHOW \
"    oam EndPoint show [<id>]"

cmd_result_t
_cmd_sbx_oam_ep_show(int unit, args_t *args)
{
    int rv;

    sbx_last_ep_dump_type = _ep_invalid;

    if (ARG_CNT(args) == 0) {
        rv = bcm_oam_group_traverse(unit, _cmd_sbx_oam_group_visit, 
                                    _cmd_sbx_oam_ep_print);
        if (BCM_FAILURE(rv)) {
            printk("Failed to traverse OAM endpoints: %s\n", bcm_errmsg(rv));
            return CMD_FAIL;
        }
    } else if (ARG_CNT(args) == 1) {
        int                     id;
        bcm_oam_endpoint_info_t info;

        id = parse_integer(ARG_GET(args));

        bcm_oam_endpoint_info_t_init(&info);
        rv = bcm_oam_endpoint_get(unit, id, &info);
        if (BCM_FAILURE(rv)) {
            printk("Failed to get endpoint %d: %s\n", id, bcm_errmsg(rv));
            return CMD_FAIL;
        }
        
        rv = _cmd_sbx_oam_ep_print(unit, &info, NULL);
        if (BCM_FAILURE(rv)) {
            printk("Failed to print endpoint info %d: %s\n", 
                   id, bcm_errmsg(rv));
            return CMD_FAIL;
        }
        
    } else {
        printk("Invalid arguments found.\n");
        return CMD_FAIL;
    }

    return CMD_OK;
}

/* OAM Endpoint management commands */

#define CMD_OAM_USAGE_EP    \
CMD_OAM_USAGE_EP_ADD "\n"           \
CMD_OAM_USAGE_EP_DELETE "\n"        \
CMD_OAM_USAGE_EP_REPLACE "\n"       \
CMD_OAM_USAGE_EP_SHOW "\n"


static cmd_t _cmd_sbx_oam_ep_list[] = {
    {"add",      _cmd_sbx_oam_ep_add,     "\n" CMD_OAM_USAGE_EP_ADD, NULL},
    {"delete",   _cmd_sbx_oam_ep_delete,  "\n" CMD_OAM_USAGE_EP_DELETE, NULL},
    {"replace",  _cmd_sbx_oam_ep_replace, "\n" CMD_OAM_USAGE_EP_REPLACE, NULL},
    {"show",     _cmd_sbx_oam_ep_show,    "\n" CMD_OAM_USAGE_EP_SHOW, NULL},
};

cmd_result_t
_cmd_sbx_oam_ep(int unit, args_t *args)
{
    return subcommand_execute(unit, args, 
                              _cmd_sbx_oam_ep_list, COUNTOF(_cmd_sbx_oam_ep_list));
}

/* Failover */

#define MAX_OAM_FAILOVER_ENDPOINTS  8096
int oam_timeout_count = 0;
int oam_timeout_init = 0;
sal_usecs_t st, et;
int oam_failover_map[MAX_OAM_FAILOVER_ENDPOINTS];

int _oam_timeout_cb(int unit,
                    uint32 flags,
                    bcm_oam_event_type_t event_type,
                    bcm_oam_group_t group,
                    bcm_oam_endpoint_t endpoint, 
                    void *userdata)
{
   if (!oam_timeout_init) {
      oam_timeout_init = 1;
      st = sal_time_usecs();
   }

   oam_timeout_count++;

   bcm_failover_set(unit, oam_failover_map[endpoint], 1);

   et = sal_time_usecs();

   return 0;
}

#define CMD_OAM_USAGE_FO_REGISTER \
"    oam FailOver register" 

cmd_result_t
_cmd_sbx_oam_fo_register(int unit, args_t *args)
{
   int             rv, i;
   bcm_oam_event_type_t  event_type;
   bcm_oam_event_types_t event_types;

   memset(&event_types, 0, sizeof(bcm_oam_event_types_t));

   
   for(i=0; i<MAX_OAM_FAILOVER_ENDPOINTS; i++) {
      oam_failover_map[i] = 1;
   }

   event_type = bcmOAMEventGroupCCMTimeout;
   BCM_OAM_EVENT_TYPE_SET(event_types, event_type);

   rv = bcm_oam_event_register(unit, event_types, _oam_timeout_cb, 0);
   if (BCM_FAILURE(rv)) {
       printk("Failed to register callback: %s\n", bcm_errmsg(rv));
       return CMD_FAIL;
   }

   printk("Registering OAM Event Callback\n");

   return CMD_OK;
}

#define CMD_OAM_USAGE_FO_UNREGISTER \
"    oam FailOver unregister" 

cmd_result_t
_cmd_sbx_oam_fo_unregister(int unit, args_t *args)
{
   int             rv;
   bcm_oam_event_type_t  event_type;
   bcm_oam_event_types_t event_types;

   memset(&event_types, 0, sizeof(bcm_oam_event_types_t));

   event_type = bcmOAMEventGroupCCMTimeout;
   BCM_OAM_EVENT_TYPE_SET(event_types, event_type);

   rv = bcm_oam_event_unregister(unit, event_types, _oam_timeout_cb);
   if (BCM_FAILURE(rv)) {
       printk("Failed to unregister callback: %s\n", bcm_errmsg(rv));
       return CMD_FAIL;
   }

    printk("Unregistering OAM Event Callback\n");

    return CMD_OK;
}

#define CMD_OAM_USAGE_FO_SHOW \
"    oam FailOver show [<clear>]"

cmd_result_t
_cmd_sbx_oam_fo_show(int unit, args_t *args)
{
    sal_usecs_t tt;

    tt = SAL_USECS_SUB(et, st);

    printk("Oam Callback %d times\n", oam_timeout_count);
    printk("start time %d end timer %d -- total time %d\n", st, et, tt);

    if (ARG_CNT(args) == 0) {
      /* */
    } else if (ARG_CNT(args) == 1) {
      ARG_GET(args);
      st = 0;
      et = 0;
      oam_timeout_count = 0;
      oam_timeout_init  = 0;
      printk("clearing values\n");
    }

    return CMD_OK;
}

/* Failover commands */
#define CMD_OAM_USAGE_FO    \
CMD_OAM_USAGE_FO_REGISTER "\n"           \
CMD_OAM_USAGE_FO_UNREGISTER "\n"        \
CMD_OAM_USAGE_FO_SHOW "\n"


static cmd_t _cmd_sbx_oam_fo_list[] = {
    {"register",   _cmd_sbx_oam_fo_register,   "\n" CMD_OAM_USAGE_FO_REGISTER, NULL},
    {"unregister", _cmd_sbx_oam_fo_unregister, "\n" CMD_OAM_USAGE_FO_UNREGISTER, NULL},
    {"show",       _cmd_sbx_oam_fo_show,       "\n" CMD_OAM_USAGE_FO_SHOW, NULL},
};

cmd_result_t
_cmd_sbx_oam_fo(int unit, args_t *args)
{
    return subcommand_execute(unit, args,
                              _cmd_sbx_oam_fo_list, COUNTOF(_cmd_sbx_oam_fo_list));
}

/* Init command */

#define CMD_OAM_USAGE_INIT \
"    oam init\n"

cmd_result_t 
_cmd_sbx_oam_init(int unit, args_t *args)
{
    int rv;
    rv = bcm_oam_init(unit);
    if (BCM_FAILURE(rv)) {
        printk("Failed to init OAM: %s\n", bcm_errmsg(rv));
        return CMD_FAIL;
    }
    printk("OAM module initialized\n");
    return CMD_OK;
}


/* Delay Measurement commands */

#define CMD_OAM_USAGE_DM_ADD \
"    oam delay add LocalID=<id> RemoteId=<id> PERiod=<ms> [TXENAble] [OneWay]"

cmd_result_t
_cmd_sbx_oam_dm_add(int unit, args_t *args)
{
    int             rv;
    bcm_oam_delay_t delay;
    parse_table_t   pt;
    cmd_result_t    retCode;
    int             txEnable;
    int             oneWay;

    if (ARG_CNT(args) == 0) {
        return CMD_USAGE;
    }
    bcm_oam_delay_t_init(&delay);
    delay.id        = BCM_OAM_ENDPOINT_INVALID;
    delay.remote_id = BCM_OAM_GROUP_INVALID;

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "LocalID",  PQ_INT | PQ_DFL, 0, &delay.id, NULL);
    parse_table_add(&pt, "RemoteID", PQ_INT | PQ_DFL, 
                    0, &delay.remote_id, NULL);
    parse_table_add(&pt, "PERiod",   PQ_INT | PQ_DFL, 0, &delay.period, NULL);
    parse_table_add(&pt, "TXENAble", PQ_BOOL | PQ_NO_EQ_OPT,
                    0, &txEnable, NULL);
    parse_table_add(&pt, "OneWay",   PQ_BOOL | PQ_NO_EQ_OPT, 0, &oneWay, NULL);

    if (!parseEndOk(args, &pt, &retCode)) {
        return retCode;
    }

    delay.flags |= (txEnable) ? BCM_OAM_DELAY_TX_ENABLE : 0;
    delay.flags |= (oneWay) ? BCM_OAM_DELAY_ONE_WAY : 0;

    rv = bcm_oam_delay_add(unit, &delay);
    if (BCM_FAILURE(rv)) {
        printk("Failed to add Delay Measurement to endpoint: %s\n", 
               bcm_errmsg(rv));
        return CMD_FAIL;
    }
    printk("Added Delay measurement to endpoint %d.\n", delay.id);

    return CMD_OK;
}

#define CMD_OAM_USAGE_DM_DELETE \
"    oam delay delete <id>"

cmd_result_t
_cmd_sbx_oam_dm_delete(int unit, args_t *args)
{
    int rv;

    if (ARG_CNT(args) != 2) {
        printk("Must specify local and remote endpoints to remove delay measurement.\n");
        return CMD_FAIL;
    } else {
        bcm_oam_delay_t delay;

        bcm_oam_delay_t_init(&delay);
        delay.id = parse_integer(ARG_GET(args));
        delay.remote_id = parse_integer(ARG_GET(args));

        rv = bcm_oam_delay_delete(unit, &delay);
        if (BCM_FAILURE(rv)) {
            printk("Failed to remove delay measurement from endpoint %d: %s\n", 
                   delay.id, bcm_errmsg(rv));
            return CMD_FAIL;
        }
        printk("Removed Delay Measurement from endpoint %d.\n", delay.id);

    }

    return CMD_OK;
}

#define DELAY_LIST_HEADER \
"Local Remote Type TxEnable\n"  \
"----- ------ ---- --------\n"

int
_cmd_sbx_oam_ep_dm_print(int unit, bcm_oam_endpoint_info_t *info, void *data)
{
    int             rv;
    bcm_oam_delay_t delay;
    
    bcm_oam_delay_t_init(&delay);
    delay.id = info->id;

    rv = bcm_oam_delay_get(unit, &delay);
    if (BCM_SUCCESS(rv)) {
        printk("%5d %6d ", delay.id, delay.remote_id);
        printk("%1dway ", (delay.flags & BCM_OAM_DELAY_ONE_WAY) ? 1 : 2);
        printk("%8s\n", (delay.flags & BCM_OAM_DELAY_TX_ENABLE) ? "  *  " : "");
    }

    return BCM_E_NONE;
}

#define CMD_OAM_USAGE_DM_SHOW \
"    oam delay show [<id>]"

cmd_result_t
_cmd_sbx_oam_dm_show(int unit, args_t *args)
{
    int rv;

    printk(DELAY_LIST_HEADER);

    if (ARG_CNT(args) == 0) {
        rv = bcm_oam_group_traverse(unit, _cmd_sbx_oam_group_visit, 
                                    (void*)_cmd_sbx_oam_ep_dm_print);
        if (BCM_FAILURE(rv)) {
            printk("Failed to traverse OAM groups: %s\n", bcm_errmsg(rv));
            return CMD_FAIL;
        }
    } else if (ARG_CNT(args) == 1) {
        int                     id;
        bcm_oam_endpoint_info_t info;

        id = parse_integer(ARG_GET(args));

        bcm_oam_endpoint_info_t_init(&info);
        rv = bcm_oam_endpoint_get(unit, id, &info);
        if (BCM_FAILURE(rv)) {
            printk("Failed to get endpoint %d: %s\n", id, bcm_errmsg(rv));
            return CMD_FAIL;
        }
        
        rv = _cmd_sbx_oam_ep_dm_print(unit, &info, NULL);

    } else { 
        printk("Invalid arguments found.\n");
        return CMD_FAIL;
    }

    return CMD_OK;
}


#define CMD_OAM_USAGE_DM  \
CMD_OAM_USAGE_DM_ADD "\n"           \
CMD_OAM_USAGE_DM_DELETE "\n"        \
CMD_OAM_USAGE_DM_SHOW "\n"

static cmd_t _cmd_sbx_oam_dm_list[] = {
    {"add",     _cmd_sbx_oam_dm_add,     "\n" CMD_OAM_USAGE_DM_ADD, NULL},
    {"delete",  _cmd_sbx_oam_dm_delete,  "\n" CMD_OAM_USAGE_DM_DELETE, NULL},
    {"show",    _cmd_sbx_oam_dm_show,    "\n" CMD_OAM_USAGE_DM_SHOW, NULL},
};

cmd_result_t
_cmd_sbx_oam_dm(int unit, args_t *args)
{
    return subcommand_execute(unit, args, 
                              _cmd_sbx_oam_dm_list, COUNTOF(_cmd_sbx_oam_dm_list));
}

/* Loss measurement commands */

#define CMD_OAM_USAGE_LM_ADD \
"    oam loss add LocalID=<id> RemoteId=<id> PERiod=<ms> \n"   \
"                 LossThreshold=<thrs in 100ths of percent>\n" \
"                 [TXENAble] [SingleEnded]"

cmd_result_t
_cmd_sbx_oam_lm_add(int unit, args_t *args)
{
    int             rv;
    bcm_oam_loss_t  loss;
    parse_table_t   pt;
    cmd_result_t    retCode;
    int             txEnable;
    int             singleEnded;

    if (ARG_CNT(args) == 0) {
        return CMD_USAGE;
    }

    bcm_oam_loss_t_init(&loss);
    loss.id        = BCM_OAM_ENDPOINT_INVALID;
    loss.remote_id = BCM_OAM_GROUP_INVALID;

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "LocalID",  PQ_INT | PQ_DFL, 0, &loss.id, NULL);
    parse_table_add(&pt, "RemoteID", PQ_INT | PQ_DFL, 
                    0, &loss.remote_id, NULL);
    parse_table_add(&pt, "PERiod",   PQ_INT | PQ_DFL, 0, &loss.period, NULL);
    parse_table_add(&pt, "LossThreshold",   PQ_INT | PQ_DFL, 
                    0, &loss.loss_threshold, NULL);
    parse_table_add(&pt, "TXENAble", PQ_BOOL | PQ_NO_EQ_OPT,
                    0, &txEnable, NULL);
    parse_table_add(&pt, "SingleEnded", PQ_BOOL | PQ_NO_EQ_OPT, 
                    0, &singleEnded, NULL);

    if (!parseEndOk(args, &pt, &retCode)) {
        return retCode;
    }

    loss.flags |= (txEnable)    ? BCM_OAM_LOSS_TX_ENABLE    : 0;
    loss.flags |= (singleEnded) ? BCM_OAM_LOSS_SINGLE_ENDED : 0;

    rv = bcm_oam_loss_add(unit, &loss);
    if (BCM_FAILURE(rv)) {
        printk("Failed to add Loss Measurement to endpoint: %s\n", 
               bcm_errmsg(rv));
        return CMD_FAIL;
    }
    printk("Added Loss measurement to endpoint %d.\n", loss.id);

    return CMD_OK;
}


#define CMD_OAM_USAGE_LM_DELETE \
"    oam loss delete <id>"

cmd_result_t
_cmd_sbx_oam_lm_delete(int unit, args_t *args)
{
    int rv;
    bcm_oam_loss_t loss;

    if (ARG_CNT(args) != 1) {
        printk("oam loss delete <local-id>\n");
        return CMD_FAIL;
    }


    bcm_oam_loss_t_init(&loss);
    loss.id = parse_integer(ARG_GET(args));
    rv = bcm_oam_loss_get(unit, &loss);
    if (BCM_FAILURE(rv)) {
      printk("can't locate loss endpoint %d, error %d (%s)\n", loss.id, rv, bcm_errmsg(rv));
      return CMD_FAIL;
    }
    rv = bcm_oam_loss_delete(unit, &loss);
    if (BCM_FAILURE(rv)) {
      printk("Failed to remove loss measurement from endpoint %d: %s\n", 
	     loss.id, bcm_errmsg(rv));
      return CMD_FAIL;
    }
    printk("Removed Loss Measurement from endpoint %d.\n", loss.id);

    return CMD_OK;
}

#define LOSS_LIST_HEADER \
"Local Remote Type TxEnable\n"  \
"----- ------ ---- --------\n"

int
_cmd_sbx_oam_ep_lm_print(int unit, bcm_oam_endpoint_info_t *info, void *data)
{
    int             rv;
    bcm_oam_loss_t  loss;
    
    bcm_oam_loss_t_init(&loss);
    loss.id = info->id;

    rv = bcm_oam_loss_get(unit, &loss);
    if (BCM_SUCCESS(rv)) {
        printk("%5d %6d ", loss.id, loss.remote_id);
        printk("%4s ", (loss.flags & BCM_OAM_LOSS_SINGLE_ENDED) ? "Sngl" : "Dual");
        printk("%8s\n", (loss.flags & BCM_OAM_LOSS_TX_ENABLE) ? "  *  " : "");
    }

    return BCM_E_NONE;
}


#define CMD_OAM_USAGE_LM_SHOW \
"    oam loss show [<id>]"

cmd_result_t
_cmd_sbx_oam_lm_show(int unit, args_t *args)
{
    int rv;

    printk(LOSS_LIST_HEADER);

    if (ARG_CNT(args) == 0) {
        rv = bcm_oam_group_traverse(unit, _cmd_sbx_oam_group_visit, 
                                    (void*)_cmd_sbx_oam_ep_lm_print);
        if (BCM_FAILURE(rv)) {
            printk("Failed to traverse OAM groups: %s\n", bcm_errmsg(rv));
            return CMD_FAIL;
        }
    } else if (ARG_CNT(args) == 1) {
        int                     id;
        bcm_oam_endpoint_info_t info;

        id = parse_integer(ARG_GET(args));

        bcm_oam_endpoint_info_t_init(&info);
        rv = bcm_oam_endpoint_get(unit, id, &info);
        if (BCM_FAILURE(rv)) {
            printk("Failed to get endpoint %d: %s\n", id, bcm_errmsg(rv));
            return CMD_FAIL;
        }
        
        rv = _cmd_sbx_oam_ep_lm_print(unit, &info, NULL);

    } else { 
        printk("Invalid arguments found.\n");
        return CMD_FAIL;
    }

    return CMD_OK;

}


#define CMD_OAM_USAGE_LM  \
CMD_OAM_USAGE_LM_ADD "\n"           \
CMD_OAM_USAGE_LM_DELETE "\n"        \
CMD_OAM_USAGE_LM_SHOW "\n"

static cmd_t _cmd_sbx_oam_lm_list[] = {
    {"add",      _cmd_sbx_oam_lm_add,     "\n" CMD_OAM_USAGE_LM_ADD, NULL},
    {"delete",   _cmd_sbx_oam_lm_delete,  "\n" CMD_OAM_USAGE_LM_DELETE, NULL},
    {"show",     _cmd_sbx_oam_lm_show,    "\n" CMD_OAM_USAGE_LM_SHOW, NULL},
};

cmd_result_t
_cmd_sbx_oam_lm(int unit, args_t *args)
{
    return subcommand_execute(unit, args, 
                              _cmd_sbx_oam_lm_list, COUNTOF(_cmd_sbx_oam_lm_list));
}

/*  Top level OAM command driver  */
char cmd_sbx_oam_usage[] =
"\n"
CMD_OAM_USAGE_INIT
CMD_OAM_USAGE_GRP
CMD_OAM_USAGE_EP
CMD_OAM_USAGE_FO
CMD_OAM_USAGE_LM
CMD_OAM_USAGE_DM
;

static cmd_t _cmd_sbx_oam_list[] = {
    {"EndPoint",  _cmd_sbx_oam_ep,   "\n" CMD_OAM_USAGE_EP,   NULL},
    {"FailOver",  _cmd_sbx_oam_fo,   "\n" CMD_OAM_USAGE_FO,   NULL},
    {"Delay",     _cmd_sbx_oam_dm,   "\n" CMD_OAM_USAGE_DM,   NULL},
    {"Group",     _cmd_sbx_oam_grp,  "\n" CMD_OAM_USAGE_GRP,  NULL},
    {"init",      _cmd_sbx_oam_init, "\n" CMD_OAM_USAGE_INIT, NULL},
    {"Loss",      _cmd_sbx_oam_lm,   "\n" CMD_OAM_USAGE_LM,   NULL},
};


cmd_result_t 
cmd_sbx_oam(int unit, args_t *args)
{
    if (!SOC_IS_SBX_FE2000(unit)) {
        printk ("OAM command supported only on FE2k devices\n\n");
        return CMD_USAGE;
    }

    return subcommand_execute(unit, args, 
                              _cmd_sbx_oam_list, COUNTOF(_cmd_sbx_oam_list));
}
