/* 
 * $Id: l2.c 1.11.178.1 Broadcom SDK $
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
 * File:        l2.c
 * Purpose:     L2 CLI commands
 *
 */

#include <appl/diag/shell.h>
#include <appl/diag/system.h>
#include <appl/diag/parse.h>

#ifdef BCM_FE2000_SUPPORT

#include <soc/sbx/sbx_drv.h>
#include <bcm_int/sbx/l2.h>
#include <bcm/error.h>
#include <bcm/l2.h>
#include <bcm/debug.h>

STATIC cmd_result_t _cmd_sbx_l2_add(int unit, args_t *args);
STATIC cmd_result_t _cmd_sbx_l2_clear(int unit, args_t *args);
STATIC cmd_result_t _cmd_sbx_l2_del(int unit, args_t *args);
STATIC cmd_result_t _cmd_sbx_l2_init(int unit, args_t *args);
STATIC cmd_result_t _cmd_sbx_l2_redirect(int unit, args_t *args);
STATIC cmd_result_t _cmd_sbx_l2_show(int unit, args_t *args);


#define CMD_L2_USAGE_ADD \
    "    l2 add [MACaddress=<mac>] [Vlanid=<id>]\n" \
    "           [Module=<n>] [PortBitMap=<pbmp>]\n" \
    "           [Trunk=true|false] [TrunkGroupId=<id>]\n" \
    "           [DiscardSource=true|false] [DiscardDest]=true|false]\n" \
    "           [STatic=true|false]\n" \
    "        - Add incrementing L2 addresses associated with port(s)\n"

#define CMD_L2_USAGE_CLEAR \
    "    l2 clear [MACaddress=<mac>] [Vlanid=<id>]\n" \
    "             [Module=<n>] [Port=<port>]\n" \
    "             [TrunkGroupID=<id>] [Static=true|false]\n" \
    "        - Remove all L2 entries on the given module/port, MAC,\n" \
    "          VLAN, or trunk group ID\n"

#define CMD_L2_USAGE_DEL \
    "    l2 del [MACaddress=<mac>] [Vlanid=<id>] [Count=<value>]\n" \
    "        - Delete L2 address(s)\n"

#define CMD_L2_USAGE_INIT \
    "    l2 init\n" \
    "        - Initialize L2\n"

#define CMD_L2_USAGE_SHOW \
    "    l2 show [MACaddress=<mac> Vlanid=<id>] [Count=<value>]\n" \
    "        - Show L2 addresses starting with given VLAN MAC key\n"

#define CMD_L2_USAGE_REDIRECT \
    "    l2 redirect [MACaddress=<mac>] [Vlanid=<id>]\n" \
    "           [qidunion=<n>]\n" 


char cmd_sbx_l2_usage[] =
    "\n"
#ifdef COMPILER_STRING_CONST_LIMIT
    "    l2 <option> [args...]\n"
#else
    CMD_L2_USAGE_ADD   "\n"
    CMD_L2_USAGE_CLEAR "\n"
    CMD_L2_USAGE_DEL   "\n"
    CMD_L2_USAGE_INIT  "\n"
    CMD_L2_USAGE_REDIRECT "\n"
    CMD_L2_USAGE_SHOW  "\n"
#endif
    ;

static cmd_t _cmd_sbx_l2_list[] = {
    {"ADD",    _cmd_sbx_l2_add,   "\n" CMD_L2_USAGE_ADD,   NULL},
    {"+",      _cmd_sbx_l2_add,   "\n" CMD_L2_USAGE_ADD,   NULL},
    {"CLEAR",  _cmd_sbx_l2_clear, "\n" CMD_L2_USAGE_CLEAR, NULL},
    {"DELete", _cmd_sbx_l2_del,   "\n" CMD_L2_USAGE_DEL,   NULL},
    {"-",      _cmd_sbx_l2_del,   "\n" CMD_L2_USAGE_DEL,   NULL},
    {"INIT",   _cmd_sbx_l2_init,  "\n" CMD_L2_USAGE_INIT,  NULL},
    {"Redirect", _cmd_sbx_l2_redirect, "\n" CMD_L2_USAGE_REDIRECT, NULL},
    {"SHOW",   _cmd_sbx_l2_show,  "\n" CMD_L2_USAGE_SHOW,  NULL}
};



/*
 * Local global variables to remember last values in arguments.
 *
 * Notes:
 * Initialize MAC address field for the user to the first real
 * address which does not conflict.
 */
static sal_mac_addr_t _l2_macaddr = {0x0, 0x0, 0x0, 0x0, 0x0, 0x1};

static int _l2_vlan = VLAN_ID_DEFAULT;

static int _l2_modid = 0, _l2_port, _l2_trunk = 0, _l2_tgid = 0, 
           _l2_dsrc = 0, _l2_ddest = 0, _l2_static = 0;

static pbmp_t  _l2_pbmp;


STATIC cmd_result_t
_cmd_sbx_l2_add(int unit, args_t *args)
{
    cmd_result_t   retCode;
    int            rv;
    parse_table_t  pt;
    pbmp_t         pbmp;
    bcm_l2_addr_t  l2addr;
    int            port = BCM_GPORT_INVALID;

    /* Parse command option arguments */
    parse_table_init(unit, &pt);
    parse_table_add(&pt, "MACaddress", PQ_DFL|PQ_MAC, 0, &_l2_macaddr, NULL);
    parse_table_add(&pt, "Vlanid", PQ_DFL|PQ_HEX, 0, &_l2_vlan, NULL);
    parse_table_add(&pt, "Module", PQ_DFL|PQ_INT, 0, &_l2_modid, NULL);
    parse_table_add(&pt, "PortBitMap", PQ_DFL|PQ_PBMP, (void *)(0),
                    &_l2_pbmp, NULL);
    parse_table_add(&pt, "Trunk", PQ_DFL|PQ_BOOL, 0, &_l2_trunk, NULL);
    parse_table_add(&pt, "TrunkGroupID", PQ_DFL|PQ_INT, 0, &_l2_tgid, NULL);
    parse_table_add(&pt, "DiscardSource", PQ_DFL|PQ_BOOL, 0, &_l2_dsrc, NULL);
    parse_table_add(&pt, "DiscardDest",  PQ_DFL|PQ_BOOL, 0, &_l2_ddest, NULL);
    parse_table_add(&pt, "STatic", PQ_DFL|PQ_BOOL, 0, &_l2_static, NULL);

    if (!parseEndOk(args, &pt, &retCode)) {
        return retCode;
    }

    pbmp = _l2_pbmp;
    if (BCM_PBMP_IS_NULL(pbmp) && !_l2_trunk) {
        printk("%s ERROR: empty port bitmap\n", ARG_CMD(args));
        return CMD_FAIL;
    }

    /*
     * If we are setting the range, the MAC address is incremented by
     * 1 for each port.
     */
    if (!_l2_trunk) {
    PBMP_ITER(pbmp, port) {
        bcm_l2_addr_t_init(&l2addr, _l2_macaddr, _l2_vlan);

        /* Configure L2 addr flags */
        if (_l2_static) {
            l2addr.flags |= BCM_L2_STATIC;
        }
        if (_l2_ddest) {
            l2addr.flags |= BCM_L2_DISCARD_DST;
        }
        if (_l2_dsrc) {
            l2addr.flags |= BCM_L2_DISCARD_SRC;
        }
        if (_l2_trunk) {
            l2addr.flags |= BCM_L2_TRUNK_MEMBER;
        }

        l2addr.modid = _l2_modid;
        l2addr.port  = port;
        l2addr.tgid  = _l2_tgid;

        if (BCM_DEBUG_CHECK(BCM_DBG_VERBOSE) &&
            BCM_DEBUG_CHECK(BCM_DBG_L2)) {
            dump_l2_addr(unit, "ADD: ", &l2addr);
        }

        rv = bcm_l2_addr_add(unit, &l2addr);
        if (BCM_FAILURE(rv)) {
            printk("%s ERROR: %s\n", ARG_CMD(args), bcm_errmsg(rv));
            return CMD_FAIL;
        }

        /* Set up for next call */
        increment_macaddr(_l2_macaddr, 1);
        }
    } else {
        bcm_l2_addr_t_init(&l2addr, _l2_macaddr, _l2_vlan);

        /* Configure L2 addr flags */
        if (_l2_static) {
            l2addr.flags |= BCM_L2_STATIC;
        }
        if (_l2_ddest) {
            l2addr.flags |= BCM_L2_DISCARD_DST;
        }
        if (_l2_dsrc) {
            l2addr.flags |= BCM_L2_DISCARD_SRC;
        }
        if (_l2_trunk) {
            l2addr.flags |= BCM_L2_TRUNK_MEMBER;
        }

        l2addr.modid = _l2_modid;
        l2addr.port  = port;
        l2addr.tgid  = _l2_tgid;

        if (BCM_DEBUG_CHECK(BCM_DBG_VERBOSE) &&
            BCM_DEBUG_CHECK(BCM_DBG_L2)) {
            dump_l2_addr(unit, "ADD: ", &l2addr);
        }

        rv = bcm_l2_addr_add(unit, &l2addr);
        if (BCM_FAILURE(rv)) {
            printk("%s ERROR: %s\n", ARG_CMD(args), bcm_errmsg(rv));
            return CMD_FAIL;
        }
    }

    return CMD_OK;
}

/*
 * This function redirects this UC mac,vid to a different mod/port (FTE)
 */
STATIC cmd_result_t
_cmd_sbx_l2_redirect(int unit, args_t *args)
{
    cmd_result_t   retCode;
    int            rv = BCM_E_UNAVAIL;
    parse_table_t  pt;
    bcm_l2_addr_t  l2addr;
    int            qidunion;

    /* Parse command option arguments */
    parse_table_init(unit, &pt);
    parse_table_add(&pt, "MACaddress", PQ_DFL|PQ_MAC, 0, &_l2_macaddr, NULL);
    parse_table_add(&pt, "Vlanid", PQ_DFL|PQ_HEX, 0, &_l2_vlan, NULL);
    parse_table_add(&pt, "QidUnion", PQ_DFL|PQ_INT, 0, &qidunion, NULL);

    if (!parseEndOk(args, &pt, &retCode)) {
        return retCode;
    }

    bcm_l2_addr_t_init(&l2addr, _l2_macaddr, _l2_vlan);

    l2addr.modid = _l2_modid;
    l2addr.tgid  = _l2_tgid;

    if (BCM_DEBUG_CHECK(BCM_DBG_VERBOSE) &&
	BCM_DEBUG_CHECK(BCM_DBG_L2)) {
        dump_l2_addr(unit, "REDIRECT: ", &l2addr);
    }
    
    if (SOC_IS_SBX_FE2000(unit)){
#ifdef BCM_FE2000_SUPPORT
      extern int _bcm_fe2000_l2_addr_update_dest(int unit, bcm_l2_addr_t *l2addr, int qidunion);
        rv = _bcm_fe2000_l2_addr_update_dest(unit, &l2addr, qidunion);
        if (BCM_FAILURE(rv)) {
	    printk("%s ERROR: %s\n", ARG_CMD(args), bcm_errmsg(rv));
	    return CMD_FAIL;
        }
#endif
    } else {
      printk("%s ERROR: %s: redirect only supported on FE2K\n",
		   ARG_CMD(args), bcm_errmsg(rv));
      return CMD_FAIL;
    }
    return CMD_OK;
}

/* CLEAR command options to delete MAC entries 'by' these arguments */
#define DELETE_BY_MAC     0x00000001
#define DELETE_BY_VLAN    0x00000002
#define DELETE_BY_MODID   0x00000004
#define DELETE_BY_PORT    0x00000008
#define DELETE_BY_TGID    0x00000010

STATIC cmd_result_t
_cmd_sbx_l2_clear(int unit, args_t *args)
{
    cmd_result_t   retCode;
    int            rv;
    parse_table_t  pt;
    char           *static_str;
    bcm_mac_t      macaddr;
    int            vlan = -1, modid = -1, port = -1, tgid = -1;
    int            mac_static = TRUE;
    int            delete_by = 0;


    ENET_SET_MACADDR(macaddr, _soc_mac_all_zeroes);

    if (!ARG_CNT(args)) {
        /*
         * Use current global L2 argument values if none arguments given
         */
        ENET_SET_MACADDR(macaddr, _l2_macaddr);
        vlan  = _l2_vlan;
        modid = _l2_modid;
        port  = _l2_port;
        tgid  = _l2_tgid;
        mac_static = _l2_static;

    } else {
        /* Parse command option arguments */
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "MACaddress", PQ_DFL|PQ_MAC, 0, &macaddr, NULL);
        parse_table_add(&pt, "Vlanid", PQ_DFL|PQ_HEX, 0, &vlan, NULL);
        parse_table_add(&pt, "Module", PQ_DFL|PQ_INT, 0, &modid, NULL);
        parse_table_add(&pt, "Port", PQ_DFL|PQ_PORT, 0, &port, NULL);
        parse_table_add(&pt, "TrunkGroupID", PQ_DFL|PQ_INT, 0, &tgid, NULL);
        parse_table_add(&pt, "STatic", PQ_DFL|PQ_BOOL, 0, &mac_static, NULL);

        if (!parseEndOk(args, &pt, &retCode)) {
            return retCode;
        }
    }
    
    /*
     * Determine which arguments were supplied
     */
    if (ENET_CMP_MACADDR(macaddr, _soc_mac_all_zeroes)) {
        delete_by |= DELETE_BY_MAC;
    }
    if (vlan >= 0) {
        delete_by |= DELETE_BY_VLAN;
    }
    if (modid >=0) {
        delete_by |= DELETE_BY_MODID;
    }
    if (port >= 0) {
        delete_by |= DELETE_BY_PORT;
    }
    if (tgid >= 0) {
        delete_by |= DELETE_BY_TGID;
    }

    static_str = mac_static ? "static and non-static" : "non-static";
    mac_static = mac_static ? BCM_L2_DELETE_STATIC : 0;

    /* Delete 'by' */
    switch (delete_by) {
    case DELETE_BY_MAC:
        printk("%s: Deleting %s addresses by MAC\n",
               ARG_CMD(args), static_str);
        rv = bcm_l2_addr_delete_by_mac(unit, macaddr, mac_static);
        break;

    case DELETE_BY_VLAN:
        printk("%s: Deleting %s addresses by VLAN\n",
               ARG_CMD(args), static_str);
        rv = bcm_l2_addr_delete_by_vlan(unit, vlan, mac_static);
        break;
        
    case DELETE_BY_MAC | DELETE_BY_VLAN:
        printk("%s: Deleting an address by MAC and VLAN\n",
               ARG_CMD(args));
        rv = bcm_l2_addr_delete(unit, macaddr, vlan);
        break;

    case DELETE_BY_PORT:
        printk("%s: Deleting %s addresses by port, local module ID\n",
               ARG_CMD(args), static_str);
        rv = bcm_l2_addr_delete_by_port(unit, -1, port, mac_static);
        break;

    case DELETE_BY_PORT | DELETE_BY_MODID:
        printk("%s: Deleting %s addresses by module/port\n",
               ARG_CMD(args), static_str);
        rv = bcm_l2_addr_delete_by_port(unit, modid, port, mac_static);
        break;

    case DELETE_BY_TGID:
        printk("%s: Deleting %s addresses by trunk ID\n",
               ARG_CMD(args), static_str);
        rv = bcm_l2_addr_delete_by_trunk(unit, tgid, mac_static);
        break;

    case DELETE_BY_MAC | DELETE_BY_PORT:
        printk("%s: Deleting %s addresses by MAC and port\n",
               ARG_CMD(args), static_str);
        rv = bcm_l2_addr_delete_by_mac_port(unit, macaddr, -1, port,
                                            mac_static);
        break;

    case DELETE_BY_MAC | DELETE_BY_PORT | DELETE_BY_MODID:
        printk("%s: Deleting %s addresses by MAC and module/port\n",
               ARG_CMD(args), static_str);
        rv = bcm_l2_addr_delete_by_mac_port(unit, macaddr, modid, port,
                                            mac_static);
        break;

    case DELETE_BY_VLAN | DELETE_BY_PORT:
	    printk("%s: Deleting %s addresses by VLAN and port\n",
               ARG_CMD(args), static_str);
	    rv = bcm_l2_addr_delete_by_vlan_port(unit, vlan, -1, port,
                                             mac_static);
        break;

    case DELETE_BY_VLAN | DELETE_BY_PORT | DELETE_BY_MODID:
	    printk("%s: Deleting %s addresses by VLAN and module/port\n",
               ARG_CMD(args), static_str);
	    rv = bcm_l2_addr_delete_by_vlan_port(unit, vlan, modid, port,
                                             mac_static);
        break;

    case DELETE_BY_VLAN | DELETE_BY_TGID:
	    printk("%s: Deleting %s addresses by VLAN and trunk ID\n",
               ARG_CMD(args), static_str);
	    rv = bcm_l2_addr_delete_by_vlan_trunk(unit, vlan, tgid, mac_static);
        break;

    default:
	    printk("%s: Unknown argument combination\n", ARG_CMD(args));
        return CMD_USAGE;
        break;
	}

    if (BCM_FAILURE(rv)) {
        printk("%s ERROR: %s\n", ARG_CMD(args), bcm_errmsg(rv));
        return CMD_FAIL;
    }

	return CMD_OK;
}


STATIC cmd_result_t
_cmd_sbx_l2_del(int unit, args_t *args)
{
    cmd_result_t   retCode;
    int            rv;
    parse_table_t  pt;
    bcm_l2_addr_t  l2addr;
    int            idx;
    int            count = 1;

    /* Parse command option arguments */
    parse_table_init(unit, &pt);
    parse_table_add(&pt, "MACaddress", PQ_DFL|PQ_MAC, 0, &_l2_macaddr, NULL);
    parse_table_add(&pt, "Vlanid", PQ_DFL|PQ_HEX, 0, &_l2_vlan, NULL);
    parse_table_add(&pt, "Count", PQ_DFL|PQ_INT, (void *)(1), &count, NULL);
    if (!parseEndOk(args, &pt, &retCode)) {
        return retCode;
    }

	for (idx = 0; idx < count; idx++) {
	    rv = bcm_l2_addr_get(unit, _l2_macaddr, _l2_vlan, &l2addr);
        if (BCM_FAILURE(rv)) {
            printk("%s: ERROR: %s\n", ARG_CMD(args), bcm_errmsg(rv));
            return CMD_FAIL;
	    }

        if (BCM_DEBUG_CHECK(BCM_DBG_VERBOSE) &&
            BCM_DEBUG_CHECK(BCM_DBG_L2)) {
            dump_l2_addr(unit, "DEL: ", &l2addr);
        }

        rv = bcm_l2_addr_delete(unit, _l2_macaddr, _l2_vlan);
        if (BCM_FAILURE(rv)) {
            printk("%s: ERROR: %s\n", ARG_CMD(args), bcm_errmsg(rv));
            return CMD_FAIL;
	    }

	    increment_macaddr(_l2_macaddr, 1);
	}

	return CMD_OK;
}


STATIC cmd_result_t
_cmd_sbx_l2_init(int unit, args_t *args)
{
    int  rv;

    if (ARG_CNT(args) > 0) {
        return CMD_USAGE;
    }

    if (BCM_FAILURE(rv = bcm_l2_init(unit))) {
        printk("%s ERROR: %s\n", ARG_CMD(args), bcm_errmsg(rv));
        return CMD_FAIL;
    }

    return CMD_OK;
}


STATIC cmd_result_t
_cmd_sbx_l2_show(int unit, args_t *args)
{
    cmd_result_t   ret_code;
    int            rv;
    bcm_mac_t      mac = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
    int            vid = 0;
    int            max_count = 0;

    if (ARG_CNT(args) > 0) {
        parse_table_t  pt;
        
        /* Parse option */
        parse_table_init(0, &pt);
        parse_table_add(&pt, "MACaddress", PQ_DFL|PQ_MAC, 0, &mac, NULL);
        parse_table_add(&pt, "Vlanid", PQ_DFL|PQ_HEX, 0, &vid, NULL);
        parse_table_add(&pt, "COUNT", PQ_DFL | PQ_INT, 0, &max_count,  NULL);
        if (!parseEndOk(args, &pt, &ret_code)) {
            return ret_code;
        }
    }

    if (BCM_FAILURE(rv = _bcm_fe2000_l2_addr_dump(unit, mac,
                                                  vid, max_count))) {
        printk("%s ERROR: %s\n", ARG_CMD(args), bcm_errmsg(rv));
        return CMD_FAIL;
    }

    return CMD_OK;
}

cmd_result_t
cmd_sbx_l2(int unit, args_t *args)
{
    return subcommand_execute(unit, args, 
                              _cmd_sbx_l2_list, COUNTOF(_cmd_sbx_l2_list));
}


/*
 * L2 Age
 *
 * Note:
 *     The CLI to manipulate aging is set in a command group different
 *     from L2.  This maintains the same CLI command menu as the
 *     current XGS/ROBO CLI.
 */
char cmd_sbx_age_usage[] =
    "Parameters:  [<seconds>]\n\t"
    "   Set the L2 age timer to the indicated number of seconds.\n\t"
    "   With no parameter, displays current value.\n\t"
    "   Setting to 0 disables L2 aging.\n";

cmd_result_t
cmd_sbx_age(int unit, args_t *args)
{
    int  seconds;
    int  rv;

    if (!ARG_CNT(args)) {    /* Display settings */
        rv = bcm_l2_age_timer_get(unit, &seconds);
        if (BCM_FAILURE(rv)) {
            printk("%s ERROR: could not get age time: %s\n",
                   ARG_CMD(args), bcm_errmsg(rv));
            return CMD_FAIL;
        }

        printk("Current age timer is %d %s\n",
               seconds, seconds ? "seconds" : "(disabled)");

        return CMD_OK;
    }

    seconds = sal_ctoi(ARG_GET(args), 0);

    rv = bcm_l2_age_timer_set(unit, seconds);
    if (BCM_FAILURE(rv)) {
        printk("%s ERROR: could not set age time: %s\n",
               ARG_CMD(args), bcm_errmsg(rv));
        return CMD_FAIL;
    }

    printk("Set age timer to %d %s\n",
           seconds, seconds ? "seconds" : "(disabled)");

    return CMD_OK;
}

#if 0 /* code to test MAC deletion performance */
#include <time.h>
#include <sal/core/time.h>
void l2_addr_del_by_test()
{
    int ret=0;
    int unit=1;
    bcm_l2_addr_t  l2Addr; 
    int g_DrvMacTstAddSucc;
    int g_DrvMacTstAddFail;
    sal_usecs_t start,end;

    g_DrvMacTstAddSucc=0;
    g_DrvMacTstAddFail=0;

    srand((unsigned )time(NULL));
    while(1)
    {
        memset(&l2Addr, 0, sizeof(bcm_l2_addr_t));

        l2Addr.mac[0] = (rand()%256)&0xFE;
        l2Addr.mac[1] = rand()%256;
        l2Addr.mac[2] = rand()%256;
        l2Addr.mac[3] = rand()%256;
        l2Addr.mac[4] = rand()%256;
        l2Addr.mac[5] = rand()%256;
        l2Addr.vid = 0x2;
        l2Addr.modid = 0;
        l2Addr.port = rand()%24;

        ret = bcm_l2_addr_add(unit, &l2Addr);
        if( BCM_E_NONE == ret )
        {
	  if (g_DrvMacTstAddSucc++ > 350000) ; /*break;*/
            g_DrvMacTstAddFail=0;
	    printf("%7d\r", g_DrvMacTstAddSucc);
        }
	 else if(BCM_E_MEMORY== ret)
	 {
            g_DrvMacTstAddFail++;
            if(g_DrvMacTstAddFail > 1000 )
            {
                break;
            }
	 }
    }

    printf("added g_DrvMacTstAddSucc=%d MACs\n", g_DrvMacTstAddSucc);
    
 start = sal_time_usecs();

    ret = bcm_l2_addr_delete_by_vlan(unit, 0x2, 0x0);

 end = sal_time_usecs();
    
 printk("bcm_l2_addr_delete_by_vlan return %d at %d us\n", ret, (end-start));
}
#endif /* l2_addr_del_by_test */

#endif /* BCM_FE2000_SUPPORT */
