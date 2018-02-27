/*
 * $Id: l3.c 1.80.6.2 Broadcom SDK $
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
 * L3 CLI commands
 */

#include <appl/diag/system.h>
#include <appl/diag/parse.h>
#include <appl/diag/l23x.h>
#include <appl/diag/l3_defip.h>
#include <appl/diag/dport.h>

#include <soc/debug.h>
#include <soc/hash.h>
#if defined(BCM_EASYRIDER_SUPPORT)
#include <soc/easyrider.h>
#endif

#include <bcm/error.h>
#include <bcm/l3.h>
#include <bcm/tunnel.h>
#include <bcm/ipmc.h>
#include <bcm/debug.h>

#include <bcm_int/esw/l3.h>
#include <bcm_int/esw/firebolt.h>
#include <bcm_int/common/multicast.h>
#include <bcm_int/esw/port.h>

#ifdef INCLUDE_L3
#define ENV_EGRESS_OBJECT_ID "egr_object_id"

/*
 * Function:
 *      _ipmc_cmd_entry_print
 * Description:
 *	Internal function to print out ipmc entry info
 * Parameters:
 *      unit   - device number.
 *	    info   - Pointer to bcm_ipmc_addr_t data structure.
 *      cookie - user cookie
 */
STATIC int
_ipmc_cmd_entry_print(int unit, bcm_ipmc_addr_t *info, void *cookie)
{
    char s_ip_str[IP6ADDR_STR_LEN];
    char mc_ip_str[IP6ADDR_STR_LEN];
    char pfmtl2p[SOC_PBMP_FMT_LEN];
	char pfmtl2u[SOC_PBMP_FMT_LEN];
	char pfmtl3p[SOC_PBMP_FMT_LEN];

    /* Input parameters check. */
    if (NULL == info) {
        return (BCM_E_PARAM);
    }

    /* Print IPMC entry. */
    if (info->flags & BCM_IPMC_IP6) {
        format_ip6addr(s_ip_str, info->s_ip6_addr);
        format_ip6addr(mc_ip_str, info->mc_ip6_addr);
        printk("SRC IP ADDRESS: %s\n", s_ip_str);
        printk("MC  IP ADDRESS: %s\n", mc_ip_str);
        printk("VLAN  PORT   V  TS   ");
        printk("L3_MAP                 L2_MAP ");
        printk("          %s  %s %s\n", "VRF", "CLASS", "HIT");
        printk("%3d %4d %5d %3d %s     %s %d, %5d,  %s\n",
               info->vid, info->port_tgid,
               info->v, info->ts,
               SOC_PBMP_FMT(info->l3_pbmp, pfmtl3p),
               SOC_PBMP_FMT(info->l2_pbmp, pfmtl2p),
               info->vrf, info->lookup_class, 
               (info->flags & BCM_IPMC_HIT) ? "y" :"no");
    } else {
        format_ipaddr(s_ip_str, info->s_ip_addr);
        format_ipaddr(mc_ip_str, info->mc_ip_addr);
        printk("%-16s%-16s%4d %4d %d %3d %d %s %s %s %4d %5d %s\n",
               s_ip_str, mc_ip_str, info->vid,
               info->port_tgid,
               info->v, info->cos, info->ts,
               SOC_PBMP_FMT(info->l3_pbmp, pfmtl3p),
               SOC_PBMP_FMT(info->l2_pbmp, pfmtl2p),
               SOC_PBMP_FMT(info->l2_ubmp, pfmtl2u),
               info->vrf, info->lookup_class, 
               (info->flags & BCM_IPMC_HIT) ? "y" :"no");

    }
    return (BCM_E_NONE);
}
#if defined(BCM_XGS_SWITCH_SUPPORT)
char if_ipmc_usage[] =
    "Usages:\n\t"
#ifdef COMPILER_STRING_CONST_LIMIT
    "  ipmc <options> [arg...]\n"
#else
    "  ipmc init    - Init ipmc function.\n\t"
    "  ipmc detach  - Detach ipmc function.\n\t"
    "  ipmc config [Enable=<bool>] [CheckSrcPort=<bool>] "
                                  "[CheckSrcIp=<bool>]\n\t"
    "  ipmc table add Src_IP=<ipaddr> Mc_IP=<grpaddr> VlanID=<vid>\n\t"
    "         L3_map=<pbmp> src_Port=<port> [L2_map=<pbmp>]\n\t"
    "         [L2_Ut_map=<pbmp>] [Ttl=<val>] [CD=<val>]\n\t"
    "         [COS=<cosl>] [TS=<tsl>] [Valid=<bool>] [MOdule=<val>]\n\t"
    "         [NoCHECK=<bool>] [Replace=<bool>] [VRF=<vrf>]\n\t" 
    "         [LookupClass=<val>]\n\t"
    "              - Add an entry into ipmc table\n\t"
    "  ipmc table del Src_IP=<ipaddr> Mc_IP=<macaddr> VlanID=<vid>\n\t"
    "              - Destroy an entry in ipmc table\n\t"
    "  ipmc table clear\n\t"
    "              - Delete all ipmc table\n\t"
    "  ipmc table show [entry]\n\t"
    "              - Display all ipmc table or a single one\n\t"
    "  ipmc egr set Port=<port> Mac=<macaddr> Vlan=<vid>\n\t"
    "                    Untag=<untag> Ttl_thresh=<ttl threshold>\n\t"
    "              - set egress IP Multicast configuration regs for a port\n\t"
    "  ipmc egr show [<port>]\n\t"
    "              - Display egress IP Multicast configuration regs for\n\t"
    "                all ports or a single port\n\t"
    "  ipmc counter show [<port>]\n\t"
    "              - Display IP Multicast counter information\n\t"
    "  ipmc repl add MCGroup=<group_no> Port_map=<pbmp> VlanID=<vid>\n\t"
    "              - Add VLAN to IPMC replication group on ports\n\t"
    "  ipmc repl del MCGroup=<group_no> Port_map=<pbmp> VlanID=<vid>\n\t"
    "              - Delete VLAN from IPMC replication group on ports\n\t"
    "  ipmc repl clear MCGroup=<group_no> Port_map=<pbmp>\n\t"
    "              - Delete all ipmc replications for group on ports\n\t"
    "  ipmc repl set MCGroup=<group_no> Port_map=<pbmp>\n\t"
    "                VlanList=<list>\n\t"
    "              - Add VLANs in list to IPMC replication group on\n\t"
    "                ports.  <list> is made of comma-separated VLANs.\n\t"
    "  ipmc repl show\n\t"
    "              - Display IP Multicast replication settings\n\t"
    "  ipmc intf set MCGroup=<group_no> Port_map=<pbmp>\n\t"
    "                IntfList=<list>\n\t"
    "              - Add interfaces in list to IPMC replication group on\n\t"
    "                ports.  <list> is made of comma-separated interface\n\t"
    "                numbers.\n\t"
    "  ipmc intf show\n\t"
    "              - Display IP Multicast replication settings\n\t"
    "  ipmc ip6table add Src_IP=<ipaddr> Mc_IP=<grpaddr> VlanID=<vid>\n\t"
    "         L3_map=<pbmp> src_Port=<port> [L2_map=<pbmp>]\n\t"
    "         [L2_Ut_map=<pbmp>] [Ttl=<val>] [CD=<val>] [COS=<cosl>]\n\t"
    "         [NoCHECK=<bool>] [TS=<tsl>] [Valid=<bool>] [MOdule=<val>]\n\t"
    "         [LookupClass=<val>]\n\t"
    "              - Add an entry into ipmc table\n\t"
    "  ipmc ip6table del Src_IP=<ipaddr> Mc_IP=<macaddr> VlanID=<vid>\n\t"
    "              - Destroy an entry in ipmc table\n\t"
    "  ipmc ip6table clear\n\t"
    "              - Delete all ipmc table\n\t"
    "  ipmc ip6table show [entry]\n\t"
    "              - Display all ipmc table or a single one\n"
#endif
    ;

cmd_result_t
if_ipmc(int unit, args_t *a)
{
    char *table, *subcmd_s, *argstr;
    int subcmd = 0;
    parse_table_t pt;
    cmd_result_t retCode;
    bcm_mac_t mac;
    vlan_id_t vid;
    int vlan = 0;
    int r, i, entry = 0, one_entry_only = 0;
    int dport, port = 0, ttl_th = 1, untag = 1;
    char mac_str[SAL_MACADDR_STR_LEN];
    bcm_ipmc_counters_t counters;
    bcm_ip_t s_ip_addr = 0, mc_ip_addr = 0;
    bcm_ipmc_addr_t ipmc_data;
    bcm_pbmp_t pbmp;
    bcm_pbmp_t l3_map;
    bcm_pbmp_t l2_map;
    bcm_pbmp_t l2_ut_map;
    bcm_port_config_t pcfg;
    int valid = 1, cos = 0, ts = 0, nocheck = 0, replace = 0, ipmc_index = 0;
    int cfg_enable = 1, cfg_check_src_port = 1, cfg_check_src_ip = 1;
    int module = 0;
    int mcg_num = 0, mcgroup = 0;
    bcm_vlan_vector_t vlan_vec;
    int first_print, ix, jx;
    char *list_str, *num_str;
    bcm_if_t *if_array = NULL;
    int  intf_num, intf_id, intf_max, alloc_size;
#if defined(BCM_XGS3_SWITCH_SUPPORT)
    int vrf = 0;
    bcm_ip6_t       ip6_addr, sip6_addr;
#endif

#define IPMC_ADD 1
#define IPMC_DEL 2
#define IPMC_CLEAR 3
#define IPMC_SHOW 4
#define IPMC_SET 5

    BCM_PBMP_CLEAR(l3_map);
    BCM_PBMP_CLEAR(l2_map);
    BCM_PBMP_CLEAR(l2_ut_map);

    sal_memset(mac, 0x10, 6);
    sal_memset(&ipmc_data, 0, sizeof(ipmc_data));

    /* Check valid device to operation on ...*/
    if (! sh_check_attached(ARG_CMD(a), unit)) {
	return CMD_FAIL;
    }

    if (bcm_port_config_get(unit, &pcfg) != BCM_E_NONE) {
        printk("%s: Error: bcm ports not initialized\n", ARG_CMD(a));
        return CMD_FAIL;
    }

    if ((table = ARG_GET(a)) == NULL) {
	return CMD_USAGE;
    }

    if (sal_strcasecmp(table, "init") == 0) {
	if ((r = bcm_ipmc_init(unit)) < 0) {
	    printk("%s: error initializing: %s\n", ARG_CMD(a), bcm_errmsg(r));
	}
        return CMD_OK;
    }
    if (sal_strcasecmp(table, "detach") == 0) {
	if ((r = bcm_ipmc_detach(unit)) < 0) {
	    printk("%s: error detaching: %s\n", ARG_CMD(a), bcm_errmsg(r));
	}
        return CMD_OK;
    }

    if (sal_strcasecmp(table, "config") == 0) {
	parse_table_init(unit, &pt);
	parse_table_add(&pt, "Enable", PQ_DFL|PQ_BOOL, 0,
			(void *)&cfg_enable, 0);
	parse_table_add(&pt, "CheckSrcPort", PQ_DFL|PQ_BOOL, 0,
			(void *)&cfg_check_src_port, 0);
	parse_table_add(&pt, "CheckSrcIp", PQ_DFL|PQ_BOOL, 0,
			(void *)&cfg_check_src_ip, 0);
	if (parse_arg_eq(a, &pt) < 0) {
	    printk("%s: Error: Unknown option: %s\n",
		   ARG_CMD(a), ARG_CUR(a));
	    return CMD_FAIL;
	}
	i = CMD_OK;
	if (pt.pt_entries[0].pq_type & PQ_PARSED) {
	    r = bcm_ipmc_enable(unit, cfg_enable);
	    if (r < 0) {
		printk("%s: Error: Enable failed: %s\n",
		       ARG_CMD(a), bcm_errmsg(r));
		i = CMD_FAIL;
	    }
	}
	if (pt.pt_entries[1].pq_type & PQ_PARSED) {
	    r = bcm_ipmc_source_port_check(unit, cfg_check_src_port);
	    if (r < 0) {
		printk("%s: Error: check_source_port failed: %s\n",
		       ARG_CMD(a), bcm_errmsg(r));
		i = CMD_FAIL;
	    }
	}
	if (pt.pt_entries[2].pq_type & PQ_PARSED) {
	    r = bcm_ipmc_source_ip_search(unit, cfg_check_src_ip);
	    if (r < 0) {
		printk("%s: Error: check_source_ip failed: %s\n",
		       ARG_CMD(a), bcm_errmsg(r));
		i = CMD_FAIL;
	    }
	}
	return (i);
    }

    if ((subcmd_s = ARG_GET(a)) == NULL) {
	return CMD_USAGE;
    }

    if (sal_strcasecmp(subcmd_s, "add") == 0) {
	subcmd = IPMC_ADD;
    }

    if (sal_strcasecmp(subcmd_s, "del") == 0) {
	subcmd = IPMC_DEL;
    }

    if (sal_strcasecmp(subcmd_s, "clear") == 0) {
	subcmd = IPMC_CLEAR;
    }

    if (sal_strcasecmp(subcmd_s, "show") == 0) {
	subcmd = IPMC_SHOW;
    }

    if (sal_strcasecmp(subcmd_s, "set") == 0) {
	subcmd = IPMC_SET;
    }

    /*    coverity[equality_cond : FALSE]    */
    if (!subcmd) {
	return CMD_USAGE;
    }

    if (sal_strcasecmp(table, "table") == 0) {
	switch (subcmd) {
	case IPMC_ADD:
	    parse_table_init(unit, &pt);
	    parse_table_add(&pt, "Src_IP", PQ_DFL|PQ_IP, 0,
			    (void *)&s_ip_addr, 0);
	    parse_table_add(&pt, "Mc_IP", PQ_DFL|PQ_IP, 0,
			    (void *)&mc_ip_addr, 0);
	    parse_table_add(&pt, "VlanID", PQ_DFL|PQ_INT, 0, (void *)&vlan, 0);
#if defined(BCM_XGS3_SWITCH_SUPPORT)
		parse_table_add(&pt, "VRF", PQ_DFL|PQ_INT, 0,
				(void *)&vrf, 0);
#endif
	    parse_table_add(&pt, "L3_map", PQ_DFL|PQ_PBMP|PQ_BCM, 0, &l3_map, 0);
	    parse_table_add(&pt, "L2_map", PQ_DFL|PQ_PBMP|PQ_BCM, 0, &l2_map, 0);
	    parse_table_add(&pt, "L2_ut_map", PQ_DFL|PQ_PBMP|PQ_BCM, 0,
			    &l2_ut_map, 0);
	    parse_table_add(&pt, "COS", PQ_DFL|PQ_INT, 0, (void *)&cos, 0);
	    parse_table_add(&pt, "Valid", PQ_DFL|PQ_BOOL, 0,
			    (void *)&valid, 0);
	    parse_table_add(&pt, "src_Port", PQ_INT, 0, (void *)&port, 0);
	    parse_table_add(&pt, "TS", PQ_INT, 0, (void *)&ts, 0);
	    parse_table_add(&pt, "MOdule", PQ_DFL|PQ_INT, 0,
			    (void *)&module, 0);
	    parse_table_add(&pt, "NoCHECK", PQ_DFL|PQ_BOOL, 0,
			    (void *)&nocheck, 0);
	    parse_table_add(&pt, "Replace", PQ_DFL|PQ_BOOL, 0,
			    (void *)&replace, 0);
            parse_table_add(&pt, "IpmcIndex", PQ_DFL|PQ_INT, 0,
                            (void *)&ipmc_index, 0);
	    if (!parseEndOk( a, &pt, &retCode)) {
		return retCode;
	    }
	    vid = vlan;
	    bcm_ipmc_addr_t_init(&ipmc_data);
        ipmc_data.s_ip_addr = s_ip_addr;
        ipmc_data.mc_ip_addr = mc_ip_addr;
        ipmc_data.vid = vid;
#if defined(BCM_XGS3_SWITCH_SUPPORT)
        ipmc_data.vrf = vrf;
#endif
	    ipmc_data.cos = cos;
	    ipmc_data.l2_pbmp = l2_map;
	    ipmc_data.l2_ubmp = l2_ut_map;
	    ipmc_data.l3_pbmp = l3_map;
	    ipmc_data.ts = ts;
	    ipmc_data.port_tgid = port;
	    ipmc_data.mod_id = module;
	    ipmc_data.v = valid;
	    if (nocheck) {
		ipmc_data.flags |= BCM_IPMC_SOURCE_PORT_NOCHECK;
	    }
        if (replace) {
            ipmc_data.flags |= BCM_IPMC_REPLACE;
        }
            if (ipmc_index) {
                ipmc_data.ipmc_index = ipmc_index;
                ipmc_data.flags |= BCM_IPMC_USE_IPMC_INDEX;
            }
	    r = bcm_ipmc_add(unit, &ipmc_data);
	    if (r < 0) {
		printk("%s: Error Add to ipmc table %s\n", ARG_CMD(a),
		       bcm_errmsg(r));
		return CMD_FAIL;
	    }
	    return CMD_OK;
	    break;
	case IPMC_DEL:
	    parse_table_init(unit, &pt);
	    parse_table_add(&pt, "Src_IP", PQ_DFL|PQ_IP, 0,
			    (void *)&s_ip_addr, 0);
	    parse_table_add(&pt, "Mc_IP", PQ_DFL|PQ_IP, 0,
			    (void *)&mc_ip_addr, 0);
	    parse_table_add(&pt, "VlanID", PQ_DFL|PQ_INT, 0, (void *)&vlan, 0);
#if defined(BCM_XGS3_SWITCH_SUPPORT)
		parse_table_add(&pt, "VRF", PQ_DFL|PQ_INT, 0,
				(void *)&vrf, 0);
#endif
	    if (!parseEndOk( a, &pt, &retCode))
		return retCode;
        bcm_ipmc_addr_t_init(&ipmc_data);
        ipmc_data.s_ip_addr = s_ip_addr;
        ipmc_data.mc_ip_addr = mc_ip_addr;
        ipmc_data.vid = vlan;
#if defined(BCM_XGS3_SWITCH_SUPPORT)
	        ipmc_data.vrf = vrf;
#endif
	    r = bcm_ipmc_remove(unit, &ipmc_data);
	    if (r < 0) {
		printk("%s: Error delete from ipmc table %s\n", ARG_CMD(a),
		       bcm_errmsg(r));
		return CMD_FAIL;
	    }
	    return CMD_OK;
	    break;
	case IPMC_CLEAR:
	    r = bcm_ipmc_delete_all(unit);
	    if (r < 0) {
		printk("%s: %s\n",
		       ARG_CMD(a), bcm_errmsg(r));
		return CMD_FAIL;
	    }
	    return (CMD_OK);
	    break;
    case IPMC_SHOW:
        r = bcm_ipmc_get_by_index(unit, 0, &ipmc_data);
        if (r < 0 && r != BCM_E_NOT_FOUND) {
            printk("%s: %s\n",
                   ARG_CMD(a), bcm_errmsg(r));
            return CMD_FAIL;
        }
        if (ARG_CNT(a)) {
            if (!isint(ARG_CUR(a))) {
                printk("%s: Invalid entry number: %s\n", ARG_CMD(a),
                       ARG_CUR(a));
                return (CMD_FAIL);
            }
            entry = parse_integer(ARG_GET(a));
            if (entry == 0)
                return (CMD_OK);
            if (_BCM_MULTICAST_IS_SET(entry) && _BCM_MULTICAST_IS_L3(entry)) {
                entry = _BCM_MULTICAST_ID_GET(entry);
            }
            if (entry < soc_mem_index_min(unit, L3_IPMCm) ||
                entry > soc_mem_index_max(unit, L3_IPMCm)) {
                printk("%s: ERROR: Entry # too high, only permitted %d\n",
                       ARG_CMD(a), soc_mem_index_max(unit, L3_IPMCm));
                return(CMD_FAIL);
            }
            one_entry_only = 1;
        }

        printk("SRC IP ADDRESS  MC IP ADDRESS   VLAN PORT V COS ");
        printk("TS        L3_MAP\t\t L2_MAP\t\t L2_UT_MAP\t    %s  %s %s\n",
               "VRF", "CLASS", "HIT");
        if (one_entry_only) {
            r = bcm_ipmc_get_by_index(unit, entry, &ipmc_data);
            if (r == BCM_E_NONE) {
                _ipmc_cmd_entry_print(unit, &ipmc_data, NULL);
            }
            return CMD_OK;
        }
        bcm_ipmc_traverse(unit, 0, _ipmc_cmd_entry_print, NULL);
        return (CMD_OK);
        break;
	default:
	    return CMD_USAGE;
	    break;
	}
	return CMD_OK;
    }

    if (sal_strcasecmp(table, "egr") == 0) {
	switch (subcmd) {
	case IPMC_SET:
	    parse_table_init(unit, &pt);
	    parse_table_add(&pt, "Port", PQ_PORT, 0, (void *)&port, 0);
	    parse_table_add(&pt, "MAC", PQ_DFL|PQ_MAC, 0, (void *)mac, 0);
	    parse_table_add(&pt, "Vlan", PQ_INT, 0, (void *)&vlan, 0);
	    parse_table_add(&pt, "Untag", PQ_DFL|PQ_BOOL, 0, (void *)&untag,
			    0);
	    parse_table_add(&pt, "Ttl_thresh", PQ_DFL|PQ_INT, 0,
			    (void *)&ttl_th, 0);
	    if (!parseEndOk( a, &pt, &retCode))
		return retCode;
	    vid = vlan;
	    r = bcm_ipmc_egress_port_set(unit, port, mac, untag, vid,
					 ttl_th);
	    if (r < 0) {
		printk("%s: Egress IP Multicast Configuration registers: %s\n",
		       ARG_CMD(a), bcm_errmsg(r));
		return CMD_FAIL;
	    }
	    break;

	case IPMC_SHOW:
	    r = bcm_ipmc_get_by_index(unit, 0, &ipmc_data);
	    if (r < 0 && r != BCM_E_NOT_FOUND) {
		printk("%s: %s\n",
		       ARG_CMD(a), bcm_errmsg(r));
		return CMD_FAIL;
	    }
            if ((argstr  = ARG_GET(a)) == NULL) { 
                pbmp = pcfg.e;
            } else {
		if (parse_bcm_port(unit, argstr, &port) < 0) {
		    printk("%s: Invalid port string: %s\n", ARG_CMD(a),
			   argstr);
		    return CMD_FAIL;
		}
		if (!BCM_PBMP_MEMBER(pcfg.e, port)) {
		    printk("port %d is not a valid Ethernet port\n", port);
		    return CMD_FAIL;
		}
                BCM_PBMP_CLEAR(pbmp);
                BCM_PBMP_PORT_ADD(pbmp, port);
	    }
	    printk("Egress IP Multicast Configuration Register information\n");
	    printk("Port      Mac Address    Vlan Untag TTL_THRESH\n");
	    DPORT_BCM_PBMP_ITER(unit, pbmp, dport, port) {
		r = bcm_ipmc_egress_port_get(unit, port, mac, &untag, &vid,
					     &ttl_th);
		if (r == BCM_E_NONE) {
		    format_macaddr(mac_str, mac);
		    printk("%-4s  %-18s %4d   %s  %5d\n",
                           BCM_PORT_NAME(unit, port), mac_str,
			   vid, untag ? "Y" : "N", ttl_th);
		}
	    }
	    return CMD_OK;
	    break;
	default:
	    return CMD_USAGE;
	    break;
	}

	return CMD_OK;
    }

    if (sal_strcasecmp(table, "counter") == 0) {
        r = bcm_ipmc_get_by_index(unit, 0, &ipmc_data);
        if (r < 0 && r != BCM_E_NOT_FOUND) {
            printk("%s: %s\n",
                   ARG_CMD(a), bcm_errmsg(r));
            return CMD_FAIL;
        }
        if ((argstr  = ARG_GET(a)) == NULL) { 
            pbmp = pcfg.e;
        } else {
            if (parse_bcm_port(unit, argstr, &port) < 0) {
                printk("%s: Invalid port string: %s\n", ARG_CMD(a),
                       argstr);
                return CMD_FAIL;
            }
            if (!BCM_PBMP_MEMBER(pcfg.e, port)) {
                printk("port %d is not a valid Ethernet port\n", port);
                return CMD_FAIL;
            }
            BCM_PBMP_CLEAR(pbmp);
            BCM_PBMP_PORT_ADD(pbmp, port);
        }
	printk("PORT      RMCA     TMCA     IMBP     IMRP    RIMDR    TIMDR\n");
        DPORT_BCM_PBMP_ITER(unit, pbmp, dport, port) {
	    r = bcm_ipmc_counters_get(unit, port, &counters);
	    if (r == BCM_E_NONE) {
		printk("%4d  %8d %8d %8d %8d %8d %8d\n",
		       port,
		       COMPILER_64_LO(counters.rmca),
		       COMPILER_64_LO(counters.tmca),
		       COMPILER_64_LO(counters.imbp),
		       COMPILER_64_LO(counters.imrp),
		       COMPILER_64_LO(counters.rimdr),
		       COMPILER_64_LO(counters.timdr));
	    }
	}
	return CMD_OK;
    }

    if (sal_strcasecmp(table, "repl") == 0) {
	switch (subcmd) {
	case IPMC_ADD:
	    parse_table_init(unit, &pt);
            parse_table_add(&pt, "MCGroup", PQ_DFL|PQ_INT, 0,
                            (void *)&mcgroup, 0);
	    parse_table_add(&pt, "VlanID", PQ_DFL|PQ_INT, 0, (void *)&vlan, 0);
	    parse_table_add(&pt, "Port_map", PQ_DFL|PQ_PBMP|PQ_BCM, 0, &l3_map, 0);
            if (!parseEndOk( a, &pt, &retCode))
		return retCode;
	    vid = vlan;
	    r = bcm_ipmc_repl_add(unit, mcgroup, l3_map, vid);
	    if (r < 0) {
		printk("%s: Error Add to ipmc replication %s\n", ARG_CMD(a),
		       bcm_errmsg(r));
		return CMD_FAIL;
	    }
	    return CMD_OK;
	    break;
	case IPMC_DEL:
	    parse_table_init(unit, &pt);
            parse_table_add(&pt, "MCGroup", PQ_DFL|PQ_INT, 0,
                            (void *)&mcgroup, 0);
	    parse_table_add(&pt, "VlanID", PQ_DFL|PQ_INT, 0, (void *)&vlan, 0);
	    parse_table_add(&pt, "Port_map", PQ_DFL|PQ_PBMP|PQ_BCM, 0, &l3_map, 0);
            if (!parseEndOk( a, &pt, &retCode))
		return retCode;
	    vid = vlan;
	    r = bcm_ipmc_repl_delete(unit, mcgroup, l3_map, vid);
	    if (r < 0) {
		printk("%s: Error Del to ipmc replication %s\n", ARG_CMD(a),
		       bcm_errmsg(r));
		return CMD_FAIL;
	    }
	    return CMD_OK;
	    break;
	case IPMC_CLEAR:
	    parse_table_init(unit, &pt);
            parse_table_add(&pt, "MCGroup", PQ_DFL|PQ_INT, 0,
                            (void *)&mcgroup, 0);
	    parse_table_add(&pt, "Port_map", PQ_DFL|PQ_PBMP|PQ_BCM, 0, &l3_map, 0);
            if (!parseEndOk( a, &pt, &retCode))
		return retCode;
            r = bcm_ipmc_repl_delete_all(unit, mcgroup, l3_map);
            if (r < 0) {
                printk("%s: %s\n",
                       ARG_CMD(a), bcm_errmsg(r));
                return CMD_FAIL;
	    }
	    return (CMD_OK);
	    break;
	case IPMC_SET:
	    parse_table_init(unit, &pt);
            parse_table_add(&pt, "MCGroup", PQ_DFL|PQ_INT, 0,
                            (void *)&mcgroup, 0);
	    parse_table_add(&pt, "Port_map", PQ_DFL|PQ_PBMP|PQ_BCM, 0, &l3_map, 0);
	    parse_table_add(&pt, "VlanList", PQ_STRING, "",
                            &list_str, 0);
            /* Parse remaining arguments */
            if (0 > parse_arg_eq(a, &pt)) {
                printk("%s: Error: Invalid option or malformed expression: %s\n",
                       ARG_CMD(a), ARG_CUR(a));
                parse_arg_eq_done(&pt);
                return(CMD_FAIL);
            }
            BCM_VLAN_VEC_ZERO(vlan_vec);
            num_str = strtok(list_str, ",");
            while (num_str != 0) {
                vid = parse_integer(num_str);
                BCM_VLAN_VEC_SET(vlan_vec, vid);
                num_str = strtok(NULL, ",");
            }
            parse_arg_eq_done(&pt);
            DPORT_BCM_PBMP_ITER(unit, l3_map, dport, port) {
                r = bcm_ipmc_repl_set(unit, mcgroup, port, vlan_vec);
                if (r < 0) {
                    printk("%s: %s\n",
                           ARG_CMD(a), bcm_errmsg(r));
                    return CMD_FAIL;
                }
            }
	    return (CMD_OK);
            break;
	case IPMC_SHOW:
            l3_map = pcfg.e;
            if (SOC_IS_LYNX(unit)) {
                mcg_num = soc_mem_index_count(unit, EGRESS_IPMC_MSm);
            } else if (SOC_IS_TUCANA(unit)) {
                mcg_num = soc_mem_index_count(unit, GE_IPMC_VECm);
            } else if (SOC_IS_DRACO15(unit)) {
                mcg_num = soc_mem_index_count(unit,
                                              MMU_MEMORIES1_IPMC_GROUP_TBLm);
            } else if (SOC_IS_FBX(unit)) {
                l3_map = pcfg.port;
                if (SOC_IS_TD_TT(unit)) {
                    /* Trident does not have MMU_IPMC_GROUP_TBL0m */
                    mcg_num = soc_mem_index_count(unit,
                            MMU_IPMC_GROUP_TBL1m);
                } else {
                    mcg_num = soc_mem_index_count(unit,
                            MMU_IPMC_GROUP_TBL0m);
                }
            } 
#if defined(BCM_EASYRIDER_SUPPORT)
            else if (SOC_IS_EASYRIDER(unit)) {
                mcg_num = soc_mem_index_count(unit, MMU_IPMC_PTRm);
            }
#endif /* BCM_EASYRIDER_SUPPORT */

            for (mcgroup = 0; mcgroup < mcg_num; mcgroup++) {
                DPORT_BCM_PBMP_ITER(unit, l3_map, dport, port) {
                    r = bcm_ipmc_repl_get(unit, mcgroup, port, vlan_vec);
                    if (r < 0) {
                        printk("%s: %s\n",
                               ARG_CMD(a), bcm_errmsg(r));
                        return CMD_FAIL;
                    }
                    first_print = TRUE;
                    for (ix = 0; ix < _SHR_BITDCLSIZE(BCM_VLAN_COUNT); ix++) {
                        if (vlan_vec[ix]) {
                            for (jx = 0; jx < 32; jx++) {
                                if (vlan_vec[ix] & (1 << jx)) {
                                    if (first_print) {
                                        printk(
                                          "MCGroup %3d, Port %5s has VLANs:\t",
                                          mcgroup, BCM_PORT_NAME(unit, port));
                                        first_print = FALSE;
                                    }
                                    printk("%4d ", ix *32 + jx);
                                }
                            }
                        }
                    }
                    if (!first_print) {
                        printk("\n");
                    }
                }
            }

	    return (CMD_OK);
	    break;
	default:
	    return CMD_USAGE;
	    break;
	}
	return CMD_OK;
    }
    if (sal_strcasecmp(table, "intf") == 0) {
	switch (subcmd) {
	case IPMC_SET:
	    parse_table_init(unit, &pt);
            parse_table_add(&pt, "MCGroup", PQ_DFL|PQ_INT, 0,
                            (void *)&mcgroup, 0);
	    parse_table_add(&pt, "Port_map", PQ_DFL|PQ_PBMP|PQ_BCM, 0, &l3_map, 0);
	    parse_table_add(&pt, "IntfList", PQ_STRING, "",
                            &list_str, 0);
            /* Parse remaining arguments */
            if (0 > parse_arg_eq(a, &pt)) {
                printk("%s: Error: Invalid option or malformed expression: %s\n",
                       ARG_CMD(a), ARG_CUR(a));
                parse_arg_eq_done(&pt);
                return(CMD_FAIL);
            }
#if defined(BCM_EASYRIDER_SUPPORT)
            intf_max = soc_mem_index_count(unit,
                 (SOC_IS_FBX(unit) ? EGR_L3_INTFm :
                     (SOC_IS_EASYRIDER(unit) ? L3INTFm : L3_INTFm)));
#else /* BCM_EASYRIDER_SUPPORT */
            intf_max = soc_mem_index_count(unit,
                 (SOC_IS_FBX(unit) ? EGR_L3_INTFm : L3_INTFm));
#endif /* BCM_EASYRIDER_SUPPORT */
            alloc_size = intf_max * sizeof(bcm_if_t);
            if_array = sal_alloc(alloc_size, "IPMC repl interface array");
            if (if_array == NULL) {
                printk("%s: Unable to allocate memory\n",
                       ARG_CMD(a));
                return(CMD_FAIL);
            }
            sal_memset(if_array, 0, alloc_size);
            intf_num = 0;
            num_str = strtok(list_str, ",");
            while (num_str != 0) {
                intf_id = parse_integer(num_str);
                if_array[intf_num++] = intf_id;
                if (intf_num == intf_max) {
                    break;
                }
                num_str = strtok(NULL, ",");
            }
            parse_arg_eq_done(&pt);
            DPORT_BCM_PBMP_ITER(unit, l3_map, dport, port) {
                r = bcm_ipmc_egress_intf_set(unit, mcgroup, port,
                                             intf_num, if_array);
                if (r < 0) {
                    printk("%s: %s\n",
                           ARG_CMD(a), bcm_errmsg(r));
                    sal_free(if_array);
                    return CMD_FAIL;
                }
            }
            sal_free(if_array);
	    return (CMD_OK);
            break;
	case IPMC_SHOW:
            l3_map = pcfg.e;
            if (SOC_IS_LYNX(unit)) {
                mcg_num = soc_mem_index_count(unit, EGRESS_IPMC_MSm);
            } else if (SOC_IS_TUCANA(unit)) {
                mcg_num = soc_mem_index_count(unit, GE_IPMC_VECm);
            } else if (SOC_IS_DRACO15(unit)) {
                mcg_num = soc_mem_index_count(unit,
                                              MMU_MEMORIES1_IPMC_GROUP_TBLm);
            } else if (SOC_IS_FBX(unit)) {
                l3_map = pcfg.port;
                mcg_num = soc_mem_index_count(unit,
                                              MMU_IPMC_GROUP_TBL0m);
            } 
#if defined(BCM_EASYRIDER_SUPPORT)
            else if (SOC_IS_EASYRIDER(unit)) {
                mcg_num = soc_mem_index_count(unit, MMU_IPMC_PTRm);
            }
#endif /* BCM_EASYRIDER_SUPPORT */
#if defined(BCM_EASYRIDER_SUPPORT)
            intf_max = soc_mem_index_count(unit,
                 (SOC_IS_FBX(unit) ? EGR_L3_INTFm :
                     (SOC_IS_EASYRIDER(unit) ? L3INTFm : L3_INTFm)));
#else /* BCM_EASYRIDER_SUPPORT */
            intf_max = soc_mem_index_count(unit,
                 (SOC_IS_FBX(unit) ? EGR_L3_INTFm : L3_INTFm));
#endif /* BCM_EASYRIDER_SUPPORT */
            alloc_size = intf_max * sizeof(bcm_if_t);
            if_array = sal_alloc(alloc_size, "IPMC repl interface array");
            if (if_array == NULL) {
                printk("%s: Unable to allocate memory\n",
                       ARG_CMD(a));
                return(CMD_FAIL);
            }
            sal_memset(if_array, 0, alloc_size);
            intf_num = 0;
            for (mcgroup = 0; mcgroup < mcg_num; mcgroup++) {
                DPORT_BCM_PBMP_ITER(unit, l3_map, dport, port) {
                    r = bcm_ipmc_egress_intf_get(unit, mcgroup, port,
                                                 intf_max, if_array,
                                                 &intf_num);
                    if (r < 0) {
                        printk("%s: %s\n",
                               ARG_CMD(a), bcm_errmsg(r));
                        sal_free(if_array);
                        return CMD_FAIL;
                    }
                    first_print = TRUE;
                    for (intf_id = 0; intf_id < intf_num; intf_id++) {
                        if (first_print) {
                            printk(
                                   "MCGroup %3d, Port %5s has interfaces:\t",
                                   mcgroup, BCM_PORT_NAME(unit, port));
                            first_print = FALSE;
                        }
                        printk("%4d ", if_array[intf_id]);
                    }
                    if (!first_print) {
                        printk("\n");
                    }
                }
            }
            sal_free(if_array);
	    return (CMD_OK);
	    break;
	default:
	    return CMD_USAGE;
	    break;
	}
	return CMD_OK;
    }
#if defined(BCM_XGS3_SWITCH_SUPPORT)
    if (sal_strcasecmp(table, "ip6table") == 0) {
        switch (subcmd) {
        /* ipmc ip6table add */
        case IPMC_ADD:
            parse_table_init(unit, &pt);
            parse_table_add(&pt, "Src_IP", PQ_DFL|PQ_IP6, 0,
                            (void *)&sip6_addr, 0);
            parse_table_add(&pt, "Mc_IP", PQ_DFL|PQ_IP6, 0,
                            (void *)&ip6_addr, 0);
            parse_table_add(&pt, "VlanID", PQ_DFL|PQ_INT, 0, (void *)&vlan, 0);
            parse_table_add(&pt, "L3_map", PQ_DFL|PQ_PBMP|PQ_BCM, 0, &l3_map, 0);
            parse_table_add(&pt, "L2_map", PQ_DFL|PQ_PBMP|PQ_BCM, 0, &l2_map, 0);
            parse_table_add(&pt, "L2_ut_map", PQ_DFL|PQ_PBMP|PQ_BCM, 0,
                            &l2_ut_map, 0);
            parse_table_add(&pt, "Valid", PQ_DFL|PQ_BOOL, 0,
                            (void *)&valid, 0);

            parse_table_add(&pt, "COS", PQ_DFL|PQ_INT, 0, (void *)&cos, 0);
            parse_table_add(&pt, "src_Port", PQ_INT, 0, (void *)&port, 0);
            parse_table_add(&pt, "TS", PQ_INT, 0, (void *)&ts, 0);
            parse_table_add(&pt, "MOdule", PQ_DFL|PQ_INT, 0,
                            (void *)&module, 0);
            parse_table_add(&pt, "NoCHECK", PQ_DFL|PQ_BOOL, 0,
                            (void *)&nocheck, 0);
            if (!parseEndOk( a, &pt, &retCode)) {
                return retCode;
            }
            memset(&ipmc_data, 0, sizeof(bcm_ipmc_addr_t));
            sal_memcpy(ipmc_data.mc_ip6_addr, ip6_addr, BCM_IP6_ADDRLEN);
            sal_memcpy(ipmc_data.s_ip6_addr, sip6_addr, BCM_IP6_ADDRLEN);
            ipmc_data.cos = cos;
            ipmc_data.vid=vlan;
            ipmc_data.flags = BCM_IPMC_IP6;
            ipmc_data.l2_pbmp = l2_map;
            ipmc_data.l2_ubmp = l2_ut_map;
            ipmc_data.l3_pbmp = l3_map;
            ipmc_data.ts = ts;
            ipmc_data.port_tgid = port;
            ipmc_data.mod_id = module;
            ipmc_data.v = valid;
            if (nocheck) {
                ipmc_data.flags |= BCM_IPMC_SOURCE_PORT_NOCHECK;
            }

            r = bcm_ipmc_add(unit, &ipmc_data);
            if (r < 0) {
                printk("%s: Error Add to ipmc table %s\n", ARG_CMD(a),
                       bcm_errmsg(r));
                return CMD_FAIL;
            }
            return CMD_OK;
            break;
        case IPMC_DEL:
        /* ipmc ip6table del */
            parse_table_init(unit, &pt);
            parse_table_add(&pt, "Src_IP", PQ_DFL|PQ_IP6, 0,
                            (void *)&sip6_addr, 0);
            parse_table_add(&pt, "Mc_IP", PQ_DFL|PQ_IP6, 0,
                            (void *)&ip6_addr, 0);
            parse_table_add(&pt, "VlanID", PQ_DFL|PQ_INT, 0, (void *)&vlan, 0);
            if (!parseEndOk( a, &pt, &retCode))
                return retCode;
            memset(&ipmc_data, 0, sizeof(bcm_ipmc_addr_t));
            sal_memcpy(ipmc_data.mc_ip6_addr, ip6_addr, BCM_IP6_ADDRLEN);
            sal_memcpy(ipmc_data.s_ip6_addr, sip6_addr, BCM_IP6_ADDRLEN);
            ipmc_data.vid=vlan;
            ipmc_data.flags = BCM_IPMC_IP6;
            r = bcm_ipmc_remove(unit, &ipmc_data);
            if (r < 0) {
                printk("%s: Error delete from ipmc table %s\n", ARG_CMD(a),
                       bcm_errmsg(r));
                return CMD_FAIL;
            }
            return CMD_OK;
            break;
        case IPMC_CLEAR:
        /* ipmc ip6table clear */
            r = bcm_ipmc_delete_all(unit);
            if (r < 0) {
                printk("%s: %s\n",
                       ARG_CMD(a), bcm_errmsg(r));
                return CMD_FAIL;
            }
            return (CMD_OK);
            break;
        case IPMC_SHOW:
        /* ipmc ip6table show */
            r = bcm_ipmc_get_by_index(unit, 0, &ipmc_data);
            if (r < 0 && r != BCM_E_NOT_FOUND) {
                printk("%s: %s\n",
                       ARG_CMD(a), bcm_errmsg(r));
                return CMD_FAIL;
            }
            if (ARG_CNT(a)) {
                if (!isint(ARG_CUR(a))) {
                    printk("%s: Invalid entry number: %s\n", ARG_CMD(a),
                           ARG_CUR(a));
                    return (CMD_FAIL);
                }
                entry = parse_integer(ARG_GET(a));
                if (entry == 0)
                    return (CMD_OK);
                if (entry < soc_mem_index_min(unit, L3_IPMCm) ||
                    entry > soc_mem_index_max(unit, L3_IPMCm)) {
                    printk("%s: ERROR: Entry # too high, only permitted %d\n",
                           ARG_CMD(a), soc_mem_index_max(unit, L3_IPMCm));
                    return(CMD_FAIL);
                }
                one_entry_only = 1;
            }

            if (one_entry_only) {
                r = bcm_ipmc_get_by_index(unit, entry, &ipmc_data);
                if (r == BCM_E_NONE && (ipmc_data.flags & BCM_IPMC_IP6)) {
                    _ipmc_cmd_entry_print(unit, &ipmc_data, NULL);
                    return CMD_OK;
                }
            }
            bcm_ipmc_traverse(unit, BCM_IPMC_IP6, _ipmc_cmd_entry_print, NULL);

            return (CMD_OK);
            break;
        default:
            return CMD_USAGE;
            break;
        }
        return CMD_OK;
    }
#endif /* BCM_XGS3_SWITCH_SUPPORT */

    return CMD_USAGE;
}
#endif /* BCM_XGS_SWITCH_SUPPORT */
#endif /* INCLUDE_L3 */

#ifdef INCLUDE_L3
char if_l3_usage[] =
    "Usages:\n\t"
#ifdef COMPILER_STRING_CONST_LIMIT
    "  l3 <option> [args...]\n"
#else
    "  l3 init    - Init L3 function.\n\t"
    "  l3 detach  - Detach L3 function.\n\n\t"

    "  l3 intf add <Vlan>=<val> <Mac>=<val> [<INtf>=<val>] [<VRF>=<val>]\n\t"
    "                   [<Mtu>=<val>] [<Group>=<val>]\n\t"
    "                   - Add an entry into l3 interface table\n\t"
    "  l3 intf destroy <INtf>=<val>\n\t"
    "                   - Destroy an entry in l3 interface table\n\t"
    "  l3 intf clear    - Destroy all intf\n\t"
    "  l3 intf show     - [<INtf>=<val>]\n\t"
    "                     Display all Intfs or a single one\n\n\t"
    
    "  l3 egress add <Mac>=<val> <INtf>=<val> [<Trunk>=<val>]\n\t"
    "                  <Port>=<val> [<MOdule>=<val>] [<L2tocpu>=<val>]\n\t" 
    "                  [<MplsLabel>=<val>] [<Drop>=<val>] [<CopyToCpu>=<val>]\n\t"
    "                  - Create egress object entry\n\t"
    "  l3 egress update <EgrId>=<val> <Mac>=<val> <INtf>=<val>\n\t"
    "                  [<Trunk>=<val>] <Port>=<val> [<MOdule>=<val>]\n\t"
    "                  [<L2tocpu>=<val>] [<MplsLabel>=<val>]\n\t"
    "                  - Update egress object entry\n\t"
    "  l3 egress delete <entry>\n\t"
    "                  - Delete egress object entry\n\t"
    "  l3 egress show [entry]\n\t"
    "                  -Show egress object entry\n\t"
    "  l3 egress find <Mac>=<val> <INtf>=<val> [<Trunk>=<val>]\n\t"
    "                  <Port>=<val> [<MOdule>=<val>] [<L2tocpu>=<val>]\n\t" 
    "                  [<MplsLabel>=<val>]\n\t"
    "                  - Find egress object entry\n\t"
    "  l3 multipath add <Size>=<val> [<INtf0>=<val>] [<INtf1>=<val>]\n\t"
    "                  [<INtf2>=<val>] [<INtf3>=<val>]\n\t"
    "                  - Create multipath egress object entry\n\t"
    "  l3 multipath update <EgrId>=<val> <Size>=<val> [<INtf0>=<val>]\n\t"
    "                  [<INtf1>=<val>] [<INtf2>=<val>] [<INtf3>=<val>]\n\t"
    "                  - Update multipath egress object entry\n\t"
    "  l3 multipath delete <entry>\n\t"
    "                  - Delete multipath egress object entry\n\t"
    "  l3 multipath show [entry]\n\t"
    "                  -Show multipath egress object entry\n\t"
    "  l3 multipath insert <EgrId>=<val> <Intf>=<val>\n\t"
    "                  -Add interface to multipath egress object entry\n\t"
    "  l3 multipath remove <EgrId>=<val> <Intf>\n\t"
    "                  -Remove interface from multipath egress object entry\n\t"
    "  l3 multipath find <Size>=<val> [<INtf0>=<val>] [<INtf1>=<val>]\n\t"
    "                  [<INtf2>=<val>] [<INtf3>=<val>]\n\t"
    "                  - Find multipath egress object entry\n\t"
    "  l3 l3table add <VRF>=<val> <IP>=<val> <Mac>=<val> <INtf>=<val>\n\t"
    "       [<Hit>=<val>] [<Trunk>=<val>] [MOdule=<val>] <Port>=<val>\n\t"
    "       <PRI>=<val> <RPE>=<val> [<Untag>=<val>] [<L2tocpu>=<val>]\n\t"
    "       [<HOST_AS_ROUTE>=<val>] [Replace=<bool>] <LookupClass>=<val>\n\t"
    "  l3 l3table destroy <VRF>=<val> <IP>=<val>\n\t"
    "                   - Destroy an entry in l3 table\n\t"
    "  l3 l3table clear - Destroy all l3 table\n\t"
    "  l3 l3table show  - Display all l3 table or a single one\n\t"
    "  l3 l3table age   - Age L3 table\n\t"
    "  l3 l3table sanity <Start=<index> End=<index>\n\t"
    "                   - Check l3x tables for (san)ity in the given range\n\t"
    "  l3 l3table hash <VRF>=<val> <IP>=<val> [<SrcIP>=<val>] [<SipKey>=<val>]\n\t"
    "                    [<VID>=<val>] [<Hash>=<hash_type>]\n\t"
    "                   - Calculate l3 hash index\n\t"
    "  l3 l3table ip6hash <IP>=<val> [<SrcIP>=<val>] [<SipKey>=<val>]\n\t"
    "                    [<VID>=<val>] [<Hash>=<hash_type>]\n\t"
    "                   - Calculate l3 hash index for IPv6 entry\n\t"
    "  l3 l3table untag  - Recalculate untagged bits for L3\n\n\t"
    "  l3 l3table conflict <VRF>=<val> <DIP>=<val> <DIP6>=<val>\n\t"
    "                     <SIP>=<val> <SIP6>=<val> <IP6>=<val> <IPMC>=<val>\n\t"
    "                     <VLAN>=<val> \n\t"
    "                     - Get conflicting l3 entries.\n\n\t"

    "  l3 defip add <VRF>=<val> <IP>=<val> <MaSk>=<val> <Mac>=<val>\n\t"
    "       <INtf>=<val> <Gateway>=<next hop Ip address> <Port>=<val>\n\t"
    "       [MOdule=<val>] [<Untag>=<val>] [<Local>=<val>] [<ECMP>=<1/0>]\n\t"
    "       [<Hit>=<val>] [<VLAN>=<val>](BCM5695 def route 0.0.0.0/0 only)\n\t"
    "       [<DstDiscard>=<val>] <PRI>=<val> <RPE>=<val> [Replace=<bool>]\n\t"
    "       [<MplsLabel>=<val>] <TunnelOpt>=<val> <LookupClass>=<val>\n\t"
    "                   - Add an IPv4 entry into DEFIP table\n\t"
    "  l3 defip destroy <VRF>=<val> <IP>=<val> <MaSk>=<val>\n\t"
    "       [<ECMP=<1/0> <Mac>=<val> <Gateway>=<next hop Ip address>\n\t"
    "        <INtf>=<val> <Port>=<val> [MOdule=<val>]\n\t"
    "       [<VLAN>=<val>](BCM5695 def route 0.0.0.0/0 only)\n\t"
    "                   - Destroy an IPv4 entry in def IP table\n\t"
    "  l3 defip clear   - Destroy all IPv4 DEFIP table\n\t"
    "  l3 defip age     - Age DEFIP(LPM) table(XGS Switch only)\n\t"
    "  l3 defip show    - Display all IPv4 DEFIP table or a single one\n\t"
    "  l3 defip ecmp <max>=<val> - Set MAX number of ECMP paths allowed\n\t"
    "  l3 defip ecmphash <DIP>=<val> <SIP>=<val> <SIP6>=val <DIP6>=val\n\t"
    "                    <COUNT>=<val> <IP6>=val  <L4_SRC_PORT>=val\n\t"
    "                    <L4_DST_PORT>=val [<EHash>=<ECMP hash Sel>] \n\t"
    "                    [<Hash>=<L3 hash type>]\n\t"
    "                   - Calculate ECMP hash index (COUNT is zero based)\n\t"
    "  l3 defip test    - Run L3 defip test\n\t"
    "  l3 defip check cmp/hw/sw - Check L3 defip table with hw or for empty\n\n\t"

    "  l3 ip6host add <VRF>=<val> <IP>=<val> <Mac>=<val> [MOdule=<val>]\n\t"
    "       <Port>=<val> [<Trunk>=<val>] <INtf>=<val> [<Hit>=<val>]\n\t"
    "       <PRI>=<val> <RPE>=<val> [<Replace>=<bool>]\n\t"
    "       [<HOST_AS_ROUTE>=<bool>] <LookupClass>=<val>\n\t"
    "                   - Add/Update an IPv6 entry\n\t"
    "  l3 ip6host delete <VRF>=<val> <IP>=<val>\n\t"
    "                   - Delete an IPv6 entry\n\t"
    "  l3 ip6host clear - Delete all IPv6 host entries\n\t"
    "  l3 ip6host age   - Age IPv6 host entries\n\t"
    "  l3 ip6host show  - Show all IPv6 host entries\n\n\t"

    "  l3 ip6route add  <VRF>=<val> <IP>=<val> <MaskLen>=<val> <Mac>=<val>\n\t" 
    "                   [MOdule=<val>] <Port>=<val> [<Trunk>=<val>]\n\t" 
    "                   <INtf>=<val> <VLan>=<val> [<Hit>=<val>]\n\t" 
    "                   [<ECMP>=<1/0>] [<DstDiscard>=<val>] <PRI>=<val>\n\t" 
    "                   <RPE>=<val> [Replace=<bool>] [<MplsLabel>=<val>]\n\t"
    "                   <TunnelOpt>=<val> <LookupClass>=<val>\n\t"
    "                    - Add IPv6 route\n\t"
    "  l3 ip6route delete <VRF>=<val> <IP>=<val> <MaskLen>=<val>\n\t"
    "       [<ECMP=<1/0> <Mac>=<val> <INtf>=<val> <Port>=<val> [MOdule=<val>]\n\t"
    "                    - Delete an IPv6 route\n\t"
    "  l3 ip6route clear - Delete all IPv6 routes\n\t"
    "  l3 ip6route age   - Clear all IPv6 route HIT bits\n\t"
    "  l3 ip6route show  - Display all IPv6 routes\n\t"

    "  l3 tunnel_init set <INtf>=<val> <TYpe>=<val> <TTL>=<val>\n\t"
    "       <Mac>=<val> <DIP>=<val> <SIP>=<val> <DSCPSel>=<val> <DSCPV>=<val>\n\t"
    "       <DFSEL4>=<val> <DFSEL6>=<val> <SIP6>=<val> <DIP6>=<val>\n\t"
    "  l3 tunnel_init show <INtf>=<val>\n\t"
    "  l3 tunnel_init clear <INtf>=<val>\n\t"

    "  l3 tunnel_term add <TYPE>=<val> <VRF>=<val> <DIP>=<val> <SIP>=<val>\n\t"
    "       <DipMaskLen>=<val> <SipMaskLen>=<val> <DIP6>=<val> <SIP6>=<val>\n\t"
    "       <L4DstPort>=<val> <L4SrcPort>=<val> <OuterTTL>=<bool> \n\t"
    "       <OuterDSCP>=<val> <InnerDSCP>=<val> <TrustDSCP>=<val> \n\t"
    "       <VLanid>=<val> <PBMP>=<val> <AllowIPv6Payload>=<bool> \n\t"
    "       <AllowIPv4Payload>=<bool> <Replace>=<bool>\n\t"
    "  l3 tunnel_term delete <TYPE>=<val> <VRF>=<val> <DIP>=<val> <SIP>=<val>\n\t"
    "       <DipMaskLen>=<val> <SipMaskLen>=<val> <DIP6>=<val> <SIP6>=<val>\n\t"
    "       <L4DstPort>=<val> <L4SrcPort>=<val> \n\t"
    "  l3 tunnel_term show <TYPE>=<val> <VRF>=<val> <DIP>=<val> <SIP>=<val>\n\t"
    "       <DipMaskLen>=<val> <SipMaskLen>=<val> <DIP6>=<val> <SIP6>=<val>\n\t"
    "       <L4DstPort>=<val> <L4SrcPort>=<val> \n\t"

    "  l3 dscp_map add\n\t"
    "  l3 dscp_map destroy <DscpMapId>=<val>\n\t"
    "  l3 dscp_map set <DscpMapId>=<val> <Priority>=<val> <Color>=<val>\n\t"
    "                  <DSCPValue>=<val>\n\t"
    "  l3 dscp_map show <DscpMapId>=<val>\n\t"
    "  l3 dscp_map_port set <Port>=<val> <Priority>=<val> <Color>=<val>\n\t"
    "                  <DSCPValue>=<val>\n\t"
    "  l3 dscp_map_port show <Port>=<val>\n\t"
    "  l3 prefix_map add <IP>=<val>\n\t"
    "  l3 prefix_map delete <IP>=<val>\n\t"
    "                  - Destroy ip6 prefix map to IPv4 entry\n\t"
    "  l3 prefix_map clear\n\t"
    "                  - Flush ip6 prefix map to IPv4 entry\n\t"
    "  l3 prefix_map show [entry]\n\t"
    "                  - Show ip6 prefix map to IPv4 entry\n\t"
#endif
    ;


/*
 * Function:
 *      _l3_cmd_route_print
 * Description:
 *	Internal function to print out route info
 * Parameters:
 *      unit   - device number.
 *      index  - Traversal index number
 *	info   - Pointer to bcm_l3_route_t data structure.
 *      cookie - user cookie
 */
STATIC int
_l3_cmd_route_print(int unit, int index, bcm_l3_route_t *info, void *cookie)
{
    char ip_str[IP6ADDR_STR_LEN + 3];
    char if_mac_str[SAL_MACADDR_STR_LEN];
    char vrf_str[20];
    char *hit;
    char *trunk;
    int masklen;
    int priority;

    hit = (info->l3a_flags & BCM_L3_HIT) ? "y" : "n";
    trunk = (info->l3a_flags & BCM_L3_TGID) ? "t" : " ";
    priority = info->l3a_pri;

    switch (info->l3a_vrf){
      case BCM_L3_VRF_GLOBAL:
          sal_strcpy(vrf_str,"Global"); 
          break;
      case BCM_L3_VRF_OVERRIDE:
          sal_strcpy(vrf_str,"Override"); 
          break;
      default:
          sal_sprintf(vrf_str,"%d",info->l3a_vrf);
    }

    format_macaddr(if_mac_str, info->l3a_nexthop_mac);

    if (info->l3a_flags & BCM_L3_IP6) {
        format_ip6addr(ip_str, info->l3a_ip6_net);
        masklen = bcm_ip6_mask_length(info->l3a_ip6_mask);
        printk("%-5d %-8s %-s/%d %18s %4d %4d %5d%1s %4d %4d %s", index, vrf_str,
               ip_str, masklen, if_mac_str, info->l3a_intf,
               info->l3a_modid, info->l3a_port_tgid, trunk, priority, 
               info->l3a_lookup_class, hit);
    } else {
        format_ipaddr_mask(ip_str, info->l3a_subnet, info->l3a_ip_mask);
        printk("%-5d %-8s %-19s %18s %4d %4d %5d%1s %4d %4d %s", index, vrf_str,
               ip_str, if_mac_str, info->l3a_intf,
               info->l3a_modid, info->l3a_port_tgid, trunk, priority, 
               info->l3a_lookup_class, hit);

        if (SOC_CHIP_GROUP(unit) == SOC_CHIP_BCM5695 &&
            info->l3a_subnet == 0) {
            printk("   %d (Per VLAN)", info->l3a_vid);
        }
    }

    if (info->l3a_flags & BCM_L3_MULTIPATH) {
        printk("      (ECMP)\n");
    } else if (info->l3a_flags & BCM_L3_DEFIP_LOCAL) {
        printk("      (LOCAL ROUTE)\n");
    } 

    if (info->l3a_flags & BCM_L3_ROUTE_LABEL) {
        printk(" (MPLS)%d\n", info->l3a_mpls_label);
    } else if (info->l3a_tunnel_option) {
        printk(" (UDP tunnel) %d\n", info->l3a_tunnel_option);
    } else { 
        printk("\n");
    }

    return BCM_E_NONE;
}


/*
 * Function:
 *      _l3_cmd_egress_intf_print
 * Description:
 *	Internal function to print out egress l3 interface info.
 * Parameters:
 *      unit      - (IN) Device number.
 *      print_hdr - (IN) Print header indicator. 
 *	    intf      - (IN) Pointer to bcm_l3_intf_t data structure.
 */
STATIC int
_l3_cmd_egress_intf_print(int unit, int print_hdr, bcm_l3_intf_t *intf)
{
    char if_mac_str[SAL_MACADDR_STR_LEN];

    if (NULL == intf) {
        return (BCM_E_PARAM);
    }

    if (print_hdr) {
        printk("Unit  Intf  VRF Group VLAN    Source Mac     MTU TTL Tunnel InnerVlan\n");
        printk("-----------------------------------------------------------\n");
    }

    format_macaddr(if_mac_str, intf->l3a_mac_addr);
    printk("%-5d %-5d %-5d %-5d %-4d %-18s %-4d %-4d %-5d %-5d\n", unit, 
           intf->l3a_intf_id, intf->l3a_vrf, intf->l3a_group, 
           intf->l3a_vid, if_mac_str, intf->l3a_mtu, intf->l3a_ttl, 
           intf->l3a_tunnel_idx, intf->l3a_inner_vlan);
    return (CMD_OK);
}


/*
 * Function:
 *      _l3_cmd_tunnel_init_print
 * Description:
 *	Internal function to print out tunnel initiator info.
 * Parameters:
 *      unit      - (IN) Device number.
 *      interface - (IN) L3 egress interface. 
 *	    info      - (IN) Pointer to bcm_tunnel_initiator_t data structure.
 */
STATIC int
_l3_cmd_tunnel_init_print (int unit, bcm_if_t interface, bcm_tunnel_initiator_t *info)
{
    char ip_str[IP6ADDR_STR_LEN + 3];
    char if_mac_str[SAL_MACADDR_STR_LEN];

    printk("Tunnel initiator:\n");
    printk("\tUnit        = %d\n", unit);
    printk("\tInterface   = %d\n", interface);
    printk("\tTUNNEL_TYPE = %d\n", info->type);
    printk("\tTTL         = %d\n", info->ttl);
    format_macaddr(if_mac_str, info->dmac);
    printk("\tMAC_ADDR    = %-18s\n", if_mac_str);
    if (_BCM_TUNNEL_OUTER_HEADER_IPV6(info->type)) { 
        format_ip6addr(ip_str, info->dip6);
        printk("\tDIP         = 0x%-s\n", ip_str);
        format_ip6addr(ip_str, info->sip6);
        printk("\tSIP         = 0x%-s\n", ip_str);
    } else { 
        printk("\tDIP         = 0x%08x\n", info->dip);
        printk("\tSIP         = 0x%08x\n", info->sip);
        if (info->flags & BCM_TUNNEL_INIT_USE_INNER_DF) {
            printk("\tCopy DF from inner header.\n");
        } else if (info->flags & BCM_TUNNEL_INIT_IPV4_SET_DF) {
            printk("\tForce DF to 1 for ipv4 payload.\n");
        }
        if (info->flags & BCM_TUNNEL_INIT_IPV6_SET_DF) {
            printk("\tForce DF to 1 for ipv6 payload.\n");
        }
    }
    printk("\tDSCP_SEL    = 0x%x\n", info->dscp_sel);
    printk("\tDSCP        = 0x%x\n", info->dscp);
    printk("\tDSCP_MAP    = 0x%x\n", info->dscp_map);
    return CMD_OK;
}

/*
 * Function:
 *      _l3_cmd_tunnel_term_print
 * Description:
 *	Internal function to print out tunnel terminator info.
 * Parameters:
 *      unit      - (IN) Device number.
 *	    info      - (IN) Pointer to bcm_tunnel_terminator_t data structure.
 */
STATIC int
_l3_cmd_tunnel_term_print (int unit, bcm_tunnel_terminator_t *info)
{
    char ip_str[IP6ADDR_STR_LEN + 3];
    char buf_pbmp[SOC_PBMP_FMT_LEN];

    printk("Tunnel terminator:\n");
    printk("\tUnit        = %d\n", unit);
    printk("\tTUNNEL_TYPE = %d\n", info->type);
    printk("\tVRF         = %d\n", info->vrf);
    if (_BCM_TUNNEL_OUTER_HEADER_IPV6(info->type)) { 
        format_ip6addr(ip_str, info->dip6);
        printk("\tDIP         = 0x%-s\n", ip_str);
        format_ip6addr(ip_str, info->dip6_mask);
        printk("\tDIP MASK    = 0x%-s\n", ip_str);
        format_ip6addr(ip_str, info->sip6);
        printk("\tSIP         = 0x%-s\n", ip_str);
        format_ip6addr(ip_str, info->sip6_mask);
        printk("\tSIP MASK    = 0x%-s\n", ip_str);
    } else { 
        printk("\tDIP         = 0x%08x\n", info->dip);
        printk("\tDIP_MASK    = 0x%08x\n", info->dip_mask);
        printk("\tSIP         = 0x%08x\n", info->sip);
        printk("\tSIP_MASK    = 0x%08x\n", info->sip_mask);
    }


    printk("\tL4 Destination Port    = 0x%08x\n", info->udp_dst_port);
    printk("\tL4 Source Port         = 0x%08x\n", info->udp_src_port);
    printk("\tVlan id                = 0x%08x\n", info->vlan);
    format_pbmp(unit, buf_pbmp, sizeof(buf_pbmp), info->pbmp);
    printk("\tPbmp                   = 0x%s\n", buf_pbmp);

    if (info->flags & BCM_TUNNEL_TERM_USE_OUTER_DSCP) {
        printk("\tCopy DSCP from outer ip header.\n");
    } else if (info->flags & BCM_TUNNEL_TERM_KEEP_INNER_DSCP) {
        printk("\tCopy DSCP from inner ip header.\n");
    } 


    if (info->flags & BCM_TUNNEL_TERM_DSCP_TRUST) {
        printk("\tTrust incming DSCP based on inner/outer selection.\n");
    }

    if (info->flags & BCM_TUNNEL_TERM_USE_OUTER_TTL) {
        printk("\tCopy TTL from outer ip header.\n");
    } 
    return CMD_OK;
}


/*
 * Function:
 *      _l3_cmd_egress_mpath_obj_print
 * Description:
 *	 Internal function to print out multipath egress object info
 * Parameters:
 *      unit        - device number.
 *      index       - Egress object id. 
 *      intf_count  - Interface count. 
 *   	info        - Pointer to interfaces array.
 *      user_data   - User cookie. 
 */
STATIC int
_l3_cmd_egress_mpath_obj_print(int unit, int index, int intf_count, bcm_if_t
                               *info, void *user_data)
{
    int idx; 
    printk("Multipath Egress Object %d\n", index);

    printk("Interfaces:");
 
    for (idx = 0; idx < intf_count; idx++) {
        printk(" %d", info[idx]);
        if (idx && (!(idx % 10))) {
            printk("\n           ");
        } 
    }
    printk("\n");
    return BCM_E_NONE;
}

/*
 * Function:
 *      _l3_cmd_egress_obj_print
 * Description:
 *	 Internal function to print out egress object info
 * Parameters:
 *      unit   - device number.
 *      index  - Egress object id. 
 *   	info   - Pointer to bcm_l3_egres_t data structure.
 *      cookie - user cookie
 */
STATIC int
_l3_cmd_egress_obj_print(int unit, int index, bcm_l3_egress_t *info, void *cookie)
{
    char if_mac_str[SAL_MACADDR_STR_LEN];
    char *trunk;
    int     port, tgid, module, id;

    trunk = (info->flags & BCM_L3_TGID) ? "t" : " ";

    format_macaddr(if_mac_str, info->mac_addr);
    if (BCM_GPORT_IS_SET(info->port)) {
        BCM_IF_ERROR_RETURN(_bcm_esw_gport_resolve(unit, info->port, &module, &port, &tgid, &id));
        if (id != -1) {
            port = id;
        }
    } else {
        port = info->port;
        module = info->module;
        tgid = info->trunk;
    }

    printk("%d %18s %4d %4d %5d%1s %3d %9d %4s %4s\n", index,
           if_mac_str, info->vlan, info->intf, 
           ((info->flags & BCM_L3_TGID) ? tgid : port),
           trunk, module, info->mpls_label, 
           (info->flags & BCM_L3_COPY_TO_CPU) ? "yes" : "no" , 
           (info->flags & BCM_L3_DST_DISCARD) ? "yes" : "no"); 

    return BCM_E_NONE;
}

/*
 * Function:
 *      _l3_cmd_host_print
 * Description:
 *	Internal function to print out host entry info
 * Parameters:
 *      unit   - device number.
 *      index  - Traversal index number
 *	info   - Pointer to bcm_l3_host_t data structure.
 *      cookie - user cookie
 */
STATIC int
_l3_cmd_host_print(int unit, int index, bcm_l3_host_t *info, void *cookie)
{
    char ip_str[IP6ADDR_STR_LEN];
    char if_mac_str[SAL_MACADDR_STR_LEN];
    char *hit;
    char *trunk = " ";
    int     tgid=0, port, module, id;

    hit = (info->l3a_flags & BCM_L3_HIT) ? "y" : "n";
    
    format_macaddr(if_mac_str, info->l3a_nexthop_mac);
    if (BCM_GPORT_IS_SET(info->l3a_port_tgid)) {
        BCM_IF_ERROR_RETURN(_bcm_esw_gport_resolve(unit, info->l3a_port_tgid, &module, 
                               &port, &tgid, &id));
        if (id != -1) {
            return CMD_USAGE;
        }
        if (tgid != -1) {
            trunk = "t";
        }
    } else {
        if (info->l3a_flags & BCM_L3_TGID) {
            trunk = "t" ; 
            tgid = info->l3a_port_tgid;
            port = -1;
        } else {
            port = info->l3a_port_tgid;
        }
        module = info->l3a_modid;
    }
    
    if (info->l3a_flags & BCM_L3_IP6) {
        format_ip6addr(ip_str, info->l3a_ip6_addr);
        printk("%-5d %-4d %-42s %18s %4d %4d %5d%1s   %6d %s\n", index,
               info->l3a_vrf, ip_str, if_mac_str, info->l3a_intf,
               module, (port == -1) ? tgid : port, trunk,
               info->l3a_lookup_class, hit);
    } else {
        format_ipaddr(ip_str, info->l3a_ip_addr);
        printk("%-5d %-4d %-16s %-18s %4d %4d %4d%1s   %6d %s\n",
               index, info->l3a_vrf, ip_str, if_mac_str, info->l3a_intf,
               module, (port == -1) ? tgid : port, trunk,
               info->l3a_lookup_class, hit);
    }

    return BCM_E_NONE;
}


/*
 * Function:
 *      _l3_cmd_l3_key_print
 * Description:
 *	Internal function to print out host entry info
 * Parameters:
 *      unit   - device number.
 *      index  - Traversal index number
 */
STATIC int
_l3_cmd_l3_key_print(int unit, int index, bcm_l3_key_t *info)
{
    char dip_str[IP6ADDR_STR_LEN];
    char sip_str[IP6ADDR_STR_LEN];

    if (info->l3k_flags & BCM_L3_IPMC) {
        if (info->l3k_flags & BCM_L3_IP6) {
            format_ip6addr(dip_str, info->l3k_ip6_addr);
            format_ip6addr(sip_str, info->l3k_sip6_addr);
            printk("%-5d Group %-42s\n", index, dip_str);
            printk("      Src %-42s\n", sip_str);
            printk("      Vrf %-4d VLAN %-4d\n", info->l3k_vrf, info->l3k_vid);
        } else {
            format_ipaddr(dip_str, info->l3k_ip_addr);
            format_ipaddr(sip_str, info->l3k_sip_addr);
            printk("%-5d Group %-18s  Src %-18s Vrf %-4d Vlan %-4d\n", 
                   index, dip_str, sip_str, info->l3k_vrf, info->l3k_vid);
        }
    } else {
        if (info->l3k_flags & BCM_L3_IP6) {
            format_ip6addr(dip_str, info->l3k_ip6_addr);
            printk("%-5d VRF %-4d DIP %-18s\n",
                   index, info->l3k_vrf, dip_str);
        } else {
            format_ipaddr(dip_str, info->l3k_ip_addr);
            printk("%-5d VRF %-4d DIP %-18s\n", 
                   index, info->l3k_vrf, dip_str);
        }
    }
    return CMD_OK;
}


/*
 * Function:
 *      _l3_cmd_host_age_callback
 * Description:
 *	Internal function to print out host entry being aged out
 * Parameters:
 *      unit   - device number.
 *      index  - Traversal index number
 *	info   - Pointer to bcm_l3_host_t data structure.
 *      cookie - user cookie
 */
STATIC int
_l3_cmd_host_age_callback(int unit, int index, bcm_l3_host_t *info, void *cookie)
{
    if (info->l3a_flags & BCM_L3_IP6) {
        char ip_str[IP6ADDR_STR_LEN];

        format_ip6addr(ip_str, info->l3a_ip6_addr);
        printk("IPv6 Address %s age out\n", ip_str);
    } else {
        printk("IP Address 0x%x age out\n", info->l3a_ip_addr);
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _l3_cmd_route_age_callback
 * Description:
 *	Internal function to print out route being aged (HIT bit cleared)
 * Parameters:
 *      unit   - device number.
 *      index  - Traversal index number
 *	info   - Pointer to bcm_l3_host_t data structure.
 *      cookie - user cookie
 */
STATIC int
_l3_cmd_route_age_callback(int unit, int index, bcm_l3_route_t *net, void *cookie)
{
    if (net->l3a_flags & BCM_L3_IP6) {
        char ip_str[IP6ADDR_STR_LEN];
        int masklen;

        format_ip6addr(ip_str, net->l3a_ip6_net);
        masklen = bcm_ip6_mask_length(net->l3a_ip6_mask);
        printk("Deleted unused route %s/%d)\n", ip_str, masklen);
    } else {
        printk("Deleted unused route 0x%x(mask 0x%x)\n",
               net->l3a_subnet, net->l3a_ip_mask);
    }

    return BCM_E_NONE;
}

/*
 * L3 command catagory
 */
enum _l3_cmd_t {
    L3_ADD = 1,
    L3_DESTROY,
    L3_SET,
    L3_CLEAR,
    L3_SHOW,
    L3_AGE,
    L3_UPDATE,
    L3_TEST,
    L3_CHECK,
    L3_HASH,
    L3_IP6HASH,
    L3_SANITY,
    L3_UNTAG,
    L3_ECMP,
    L3_ECMP_HASH, 
    L3_INSERT, 
    L3_REMOVE, 
    L3_FIND,
    L3_CONFLICT
};

const char *cmd_tunnel_term_parse_type[] = {
    "6to4",
    "6to4-unchecked",
    "isatap",
    "ipip",
    NULL
};


#if defined(BCM_XGS_SWITCH_SUPPORT)
/*
 * Function:
 *      _l3_cmd_l3_conflict_show
 * Description:
 * 	    Service routine used to show l3 table conflicting entries. 
 * Parameters:
 *      unit      - (IN) Device number.
 *      a         - (IN) Command arguments. 
 * Returns: 
 *      CMD_XXX
 */

STATIC cmd_result_t
_l3_cmd_l3_conflict_show(int unit, args_t *a) 
{
    parse_table_t     pt;     
    cmd_result_t      retCode; 
    bcm_ip6_t         sip6_addr;
    bcm_ip6_t         ip6_addr;
    bcm_ip_t          sip_addr = 0;
    bcm_ip_t          ip_addr = 0;
    int               vlan = 0;
    int               ipmc = 0;
    int               ip6 = 0;
    bcm_vrf_t         vrf = BCM_L3_VRF_DEFAULT;
    bcm_l3_key_t      ipkey;
    bcm_l3_key_t      result[SOC_L3X_MAX_BUCKET_SIZE];
    int               count;  
    int               idx;
    int               rv; 

    /* Stack variables initialization. */
    sal_memset(sip6_addr, 0, sizeof(bcm_ip6_t));
    sal_memset(ip6_addr, 0, sizeof(bcm_ip6_t));
    sal_memset(result, 0, SOC_L3X_MAX_BUCKET_SIZE * sizeof(bcm_l3_key_t));

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "DIP",  PQ_DFL | PQ_IP, 0, (void *)&ip_addr, 0);
    parse_table_add(&pt, "SIP",  PQ_DFL | PQ_IP, 0, (void *)&sip_addr, 0);
    parse_table_add(&pt, "DIP6", PQ_DFL | PQ_IP6, 0, (void *)&ip6_addr, 0);
    parse_table_add(&pt, "SIP6", PQ_DFL | PQ_IP6, 0, (void *)&sip6_addr, 0);
    parse_table_add(&pt, "VRF",  PQ_DFL | PQ_INT, 0, (void *)&vrf, 0);
    parse_table_add(&pt, "VLAN", PQ_DFL | PQ_INT, 0, (void *)&vlan, 0);
    parse_table_add(&pt, "IPMC", PQ_DFL | PQ_INT, 0, (void *)&ipmc, 0);
    parse_table_add(&pt, "IP6",  PQ_DFL | PQ_INT, 0, (void *)&ip6, 0);
    if (!parseEndOk(a, &pt, &retCode)) {
        return retCode;
    }

    /* Initialize data structures. */
    sal_memset(&ipkey, 0, sizeof(bcm_l3_key_t));

    /* Fill tunnel info. */
    if (ip6) { 
        ipkey.l3k_flags |= BCM_L3_IP6; 
        sal_memcpy(ipkey.l3k_ip6_addr, ip6_addr, sizeof(bcm_ip6_t));
        sal_memcpy(ipkey.l3k_sip6_addr, sip6_addr, sizeof(bcm_ip6_t));
    } else {
        ipkey.l3k_ip_addr = ip_addr;
        ipkey.l3k_sip_addr = sip_addr;
    }

    if (ipmc) { 
       ipkey.l3k_flags |= BCM_L3_IPMC; 
    }
    ipkey.l3k_vrf = vrf;
    ipkey.l3k_vid = vlan & 0xfff;

    if ((rv = bcm_l3_host_conflict_get(unit, &ipkey, result, 
                                       SOC_L3X_MAX_BUCKET_SIZE, &count)) < 0) {
        printk("ERROR %s: getting l3 conflicts %s\n", 
               ARG_CMD(a), bcm_errmsg(rv));
        return CMD_FAIL;
    }

    for (idx = 0; idx < count; idx++) {
        _l3_cmd_l3_key_print(unit, idx, result + idx);
    }
    return CMD_OK;
}

/*
 * Function:
 *      _l3_cmd_defip_ecmp_hash
 * Description:
 * 	    Service routine used to calculate route ecmp hash 
 * Parameters:
 *      unit      - (IN) Device number.
 *      a         - (IN) Command arguments. 
 * Returns: 
 *      CMD_XXX
 */
STATIC cmd_result_t
_l3_cmd_defip_ecmp_hash(int unit, args_t *a) 
{
    uint32                 l4_dst_port = 0;
    uint32                 l4_src_port = 0;
    int                    ecmp_count = 0;
    bcm_ip6_t              dip6_addr;
    bcm_ip6_t              sip6_addr;
    bcm_ip_t               dip_addr = 0;
    bcm_ip_t               sip_addr = 0;
    cmd_result_t           retCode;
    uint32                 regval = 0;
    int                    hash_v = 0;
    parse_table_t          pt;     
    int                    rv;
    int                    ip6 = 0;
    int                    hash_select = XGS_HASH_COUNT;
    int                    ecmp_hash_sel = 2;
#ifdef BCM_XGS3_SWITCH_SUPPORT
    soc_xgs3_ecmp_hash_t   data; 
#endif /* BCM_XGS3_SWITCH_SUPPORT */    

    if (!SOC_IS_XGS_SWITCH(unit)) {
        printk("Command only valid for XGS switches\n");
        return CMD_FAIL;
    }

    sal_memset(dip6_addr, 0, sizeof (bcm_ip6_t));
    sal_memset(sip6_addr, 0, sizeof (bcm_ip6_t));

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "DIP", PQ_DFL | PQ_IP, 0, (void *)&dip_addr, 0);
    parse_table_add(&pt, "SIP", PQ_DFL | PQ_IP, 0, (void *)&sip_addr, 0);
    parse_table_add(&pt, "DIP6", PQ_DFL | PQ_IP6, 0, (void *)dip6_addr, 0);
    parse_table_add(&pt, "SIP6", PQ_DFL | PQ_IP6, 0, (void *)sip6_addr, 0);
    parse_table_add(&pt, "IP6", PQ_DFL | PQ_INT, 0, (void *)&ip6, 0);
    parse_table_add(&pt, "L4_SRC_PORT", PQ_DFL | PQ_INT, 0,
                    (void *)&l4_dst_port, 0);
    parse_table_add(&pt, "L4_DST_PORT", PQ_DFL | PQ_INT, 0, 
                    (void *)&l4_src_port, 0);
    parse_table_add(&pt, "COUNT", PQ_DFL | PQ_INT, 0,
                    (void *)&ecmp_count, 0);
    parse_table_add(&pt, "EHash", PQ_DFL | PQ_INT, 0,
                    (void *)&ecmp_hash_sel, 0);
    parse_table_add(&pt, "Hash", PQ_DFL | PQ_INT, 0,
                    (void *)&hash_select, 0);
    if (!parseEndOk(a, &pt, &retCode)) {
        return retCode;
    }

#ifdef BCM_XGS3_SWITCH_SUPPORT
    if (SOC_IS_XGS3_SWITCH(unit)) {   /* FBX || ER */
        sal_memset(&data, 0, sizeof (soc_xgs3_ecmp_hash_t));
        /* Fill source/destination address. . */
        if (ip6) { 
            sal_memcpy(data.dip6, dip6_addr, sizeof(bcm_ip6_t));
            sal_memcpy(data.sip6, sip6_addr, sizeof(bcm_ip6_t));
            data.v6 = TRUE;
        } else {
            data.dip = dip_addr;
            data.sip = sip_addr;
        }
        data.l4_src_port = l4_src_port & 0xffff;  /* TCP/UDP src port. */
        data.l4_dst_port = l4_dst_port & 0xffff;  /* TCP/UDP dst port. */
        data.ecmp_count = ecmp_count;      /* Zero based ecmp count. */

        rv = soc_xgs3_l3_ecmp_hash(unit, &data);
        if (rv < 0){
            printk("Internal error\n");
            return CMD_FAIL;
        } 
        printk("ECMP hash is %d \n", rv);
        return CMD_OK; 
    } else 
#endif /* BCM_XGS3_SWITCH_SUPPORT */
    if (ip6) {
        printk("IPv6 is not supported.\n");
        return CMD_OK; 
    }

    if (SOC_IS_TUCANA(unit)) { /* TUCANA */

        hash_v = soc_tucana_l3_ecmp_hash(unit, dip_addr, sip_addr,
                                         ecmp_count);
        printk("ECMP hash of DIP 0x%X SIP 0x%X Count %d is %d\n",
               dip_addr, sip_addr, ecmp_count, hash_v);
        return CMD_OK; 

    } else if (SOC_IS_LYNX(unit)) { /* LYNX */

        hash_v = soc_lynx_l3_ecmp_hash(unit, dip_addr, sip_addr,
                                       ecmp_count);
        printk("ECMP hash of DIP 0x%X SIP 0x%X Count %d is %d\n",
               dip_addr, sip_addr, ecmp_count, hash_v);
        return CMD_OK; 

    } else if (SOC_IS_DRACO15(unit)) {    /* DRACO1.5 */

        if (hash_select == XGS_HASH_COUNT) {
            /* Get the hash selection from hardware */
            if ((rv = READ_HASH_CONTROLr(unit, &regval)) < 0) {
                printk("%s: ERROR: %s\n", ARG_CMD(a), soc_errmsg(rv));
                return CMD_FAIL;
            }
            hash_select = soc_reg_field_get(unit, HASH_CONTROLr, regval,
                                            HASH_SELECTf);
        }
        if (ecmp_hash_sel == 2) {
            /* Get the hash selection from hardware */
            if ((rv = READ_HASH_CONTROLr(unit, &regval)) < 0) {
                printk("%s: ERROR: %s\n", ARG_CMD(a), soc_errmsg(rv));
                return CMD_FAIL;
            }
            ecmp_hash_sel = soc_reg_field_get(unit, HASH_CONTROLr, regval,
                                              ECMP_HASH_SELECTf);
        }

        hash_v = soc_draco_l3_ecmp_hash(unit, dip_addr, sip_addr, ecmp_count,
                                        ecmp_hash_sel, hash_select);
        printk("ECMP hash of DIP 0x%X SIP 0x%X Count %d is %d "
               "(ECMP hash select %d L3 hash select %d)\n",
               dip_addr, sip_addr, ecmp_count, hash_v,
               ecmp_hash_sel, hash_select);
        return CMD_OK;

    }
    printk("Command is not supported.\n"); 
    return CMD_FAIL;
}
#endif /* BCM_XGS_SWITCH_SUPPORT*/

/*
 * Function:
 *      _l3_cmd_tunnel_init_add
 * Description:
 * 	    Service routine used to add tunnel initiator to an interface.
 * Parameters:
 *      unit      - (IN) Device number.
 *      a         - (IN) Command arguments. 
 * Returns: 
 *      CMD_XXX
 */

STATIC cmd_result_t
_l3_cmd_tunnel_init_add(int unit, args_t *a) 
{
    bcm_tunnel_initiator_t tunnel_init;
    bcm_l3_intf_t     intf;
    bcm_tunnel_type_t type = 0;
    parse_table_t     pt;     
    cmd_result_t      retCode; 
    bcm_ip6_t         sip6_addr;
    bcm_ip6_t         ip6_addr;
    bcm_mac_t         mac;
    bcm_if_t          interface = 0;
    bcm_ip_t          sip_addr = 0;
    bcm_ip_t          ip_addr = 0;
    int               ip4_df_sel = 0;
    int               ip6_df_sel = 0;
    int               dscp_val = 0;
    int               dscp_sel = 0;
    int               dscp_map = 0;
    int               ttl = 0;
    int               rv;


    /* Stack variables initialization. */
    sal_memset(&intf, 0, sizeof(bcm_l3_intf_t));
    sal_memset(sip6_addr, 0, sizeof(bcm_ip6_t));
    sal_memset(ip6_addr, 0, sizeof(bcm_ip6_t));
    sal_memset(mac, 0, sizeof(bcm_mac_t));

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "INtf", PQ_DFL|PQ_INT, 0, (void*)&interface, 0);
    parse_table_add(&pt, "TYpe", PQ_DFL|PQ_INT, 0, (void*)&type, 0);
    parse_table_add(&pt, "TTL",  PQ_DFL|PQ_INT, 0, (void*)&ttl, 0);
    parse_table_add(&pt, "Mac",  PQ_DFL|PQ_MAC, 0, (void *)mac, 0);
    parse_table_add(&pt, "DIP",  PQ_DFL|PQ_IP, 0, (void *)&ip_addr, 0);
    parse_table_add(&pt, "SIP",  PQ_DFL|PQ_IP, 0, (void *)&sip_addr, 0);
    parse_table_add(&pt, "DIP6", PQ_DFL | PQ_IP6, 0, (void *)&ip6_addr, 0);
    parse_table_add(&pt, "SIP6", PQ_DFL | PQ_IP6, 0, (void *)&sip6_addr, 0);
    parse_table_add(&pt, "DSCPSel", PQ_DFL|PQ_INT, 0, (void *)&dscp_sel, 0);
    parse_table_add(&pt, "DSCPV", PQ_DFL|PQ_INT, 0, (void *)&dscp_val, 0);
    parse_table_add(&pt, "DFSEL4", PQ_DFL|PQ_INT, 0, (void *)&ip4_df_sel, 0);
    parse_table_add(&pt, "DFSEL6", PQ_DFL|PQ_BOOL, 0, (void *)&ip6_df_sel, 0);
    parse_table_add(&pt, "DSCPMap", PQ_DFL|PQ_INT, 0, (void *)&dscp_map, 0);
    if (!parseEndOk(a, &pt, &retCode)) {
        return retCode;
    }

    /* Initialize data structures. */
    bcm_l3_intf_t_init(&intf);
    bcm_tunnel_initiator_t_init(&tunnel_init);

    /* Fill tunnel info. */
    intf.l3a_intf_id = interface;
    tunnel_init.type = type;
    tunnel_init.ttl = ttl;

    sal_memcpy(tunnel_init.dmac, mac, sizeof(mac));

    if (ip4_df_sel) { 
        tunnel_init.flags |= (ip4_df_sel > 1) ?
            BCM_TUNNEL_INIT_USE_INNER_DF : BCM_TUNNEL_INIT_IPV4_SET_DF;
    }
    if (ip6_df_sel) { 
        tunnel_init.flags |= BCM_TUNNEL_INIT_IPV6_SET_DF;
    }

    sal_memcpy(tunnel_init.dip6, ip6_addr, sizeof(bcm_ip6_t));
    sal_memcpy(tunnel_init.sip6, sip6_addr, sizeof(bcm_ip6_t));
    tunnel_init.dip = ip_addr;
    tunnel_init.sip = sip_addr;
    tunnel_init.dscp_sel = dscp_sel;
    tunnel_init.dscp = dscp_val;
    tunnel_init.dscp_map = dscp_map;

    if ((rv = bcm_tunnel_initiator_set(unit, &intf, &tunnel_init)) < 0) {
        printk("ERROR %s: setting tunnel initiator for %d %s\n",
               ARG_CMD(a), interface, bcm_errmsg(rv));
        return CMD_FAIL;
    }
    return CMD_OK;
}

/*
 * Function:
 *      _l3_cmd_tunnel_init_show
 * Description:
 * 	    Service routine used to get & show tunnel initiator info. 
 * Parameters:
 *      unit      - (IN) Device number.
 *      a         - (IN) Command arguments. 
 * Returns: 
 *      CMD_XXX
 */
STATIC cmd_result_t
_l3_cmd_tunnel_init_show(int unit, args_t *a) 
{
    bcm_tunnel_initiator_t tunnel_init;
    bcm_l3_intf_t          intf;
    parse_table_t          pt;     
    cmd_result_t           retCode; 
    bcm_if_t               interface;
    int                    rv;

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "INtf", PQ_DFL|PQ_INT, 0, (void*)&interface, 0);
    if (!parseEndOk(a, &pt, &retCode)) {
        return retCode;
    }

    bcm_tunnel_initiator_t_init(&tunnel_init);

    bcm_l3_intf_t_init(&intf);
    intf.l3a_intf_id = interface;

    if ((rv = bcm_tunnel_initiator_get(unit, &intf, &tunnel_init)) < 0) {
        printk("ERROR %s: getting tunnel initiator for %d %s\n",
               ARG_CMD(a), interface, bcm_errmsg(rv));
        return CMD_FAIL;
    }

    _l3_cmd_tunnel_init_print(unit, interface, &tunnel_init);
    return CMD_OK;
}

/*
 * Function:
 *      _l3_cmd_tunnel_init_clear
 * Description:
 * 	    Service routine used to remove tunnel initiator from an interface.
 * Parameters:
 *      unit      - (IN) Device number.
 *      a         - (IN) Command arguments. 
 * Returns: 
 *      CMD_XXX
 */
STATIC cmd_result_t
_l3_cmd_tunnel_init_clear(int unit, args_t *a) 
{
    bcm_if_t               interface;
    cmd_result_t           retCode; 
    bcm_l3_intf_t          intf;
    parse_table_t          pt;     
    int                    rv;

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "INtf", PQ_DFL|PQ_INT, 0, (void*)&interface, 0);
    if (!parseEndOk(a, &pt, &retCode)) {
        return retCode;
    }

    bcm_l3_intf_t_init(&intf);
    intf.l3a_intf_id = interface;

    if ((rv = bcm_tunnel_initiator_clear(unit, &intf)) < 0) {
        printk("ERROR %s: clearing tunnel initiator for %d %s\n",
               ARG_CMD(a), interface, bcm_errmsg(rv));
        return CMD_FAIL;
    }
    return CMD_OK;
}

/*
 * Function:
 *      _l3_cmd_tunnel_term_add
 * Description:
 * 	    Service routine used to add tunnel terminator entry.
 * Parameters:
 *      unit      - (IN) Device number.
 *      a         - (IN) Command arguments. 
 * Returns: 
 *      CMD_XXX
 */
STATIC cmd_result_t
_l3_cmd_tunnel_term_add(int unit, args_t *a) 
{
    bcm_tunnel_terminator_t tunnel_term;
    cmd_result_t      retCode; 
    parse_table_t     pt;     
    int               rv;
    bcm_pbmp_t        pbmap;
    bcm_ip6_t         sip6;
    bcm_ip6_t         dip6;
    int               vlan = 0;
    int               dip = 0;
    int               sip = 0;
    int               sip_mask_len = 0; 
    int               dip_mask_len = 0;
    int               dport = 0; 
    int               sport = 0; 
    int               tnl_type = 0;
    int               outer_ttl = 0;
    int               outer_dscp = 0;
    int               inner_dscp = 0;
    int               trust_dscp = 0; 
    int               allow_v4 = 0; 
    int               allow_v6 = 0;
    int               vrf = 0;
    int               replace=0;
    bcm_gport_t       remote_port = BCM_GPORT_INVALID;
    bcm_gport_t       tunnel_id = 0;

    parse_table_init(unit, &pt);
    sal_memset(sip6, 0, sizeof(bcm_ip6_t));
    sal_memset(dip6, 0, sizeof(bcm_ip6_t));
    sal_memset(&pbmap, 0, sizeof(bcm_pbmp_t));

    parse_table_add(&pt, "VRF",  PQ_DFL | PQ_INT, 0, (void *)&vrf, 0);
    parse_table_add(&pt, "DIP",  PQ_DFL|PQ_IP, 0, (void*)&dip, 0);
    parse_table_add(&pt, "SIP",  PQ_DFL|PQ_IP, 0, (void*)&sip, 0);
    parse_table_add(&pt, "DIP6",  PQ_DFL|PQ_IP6, 0, (void*)&dip6, 0);
    parse_table_add(&pt, "SIP6",  PQ_DFL|PQ_IP6, 0, (void*)&sip6, 0);
    parse_table_add(&pt, "DipMaskLen", PQ_DFL|PQ_INT, 0, (void*)&dip_mask_len, 0);
    parse_table_add(&pt, "SipMaskLen", PQ_DFL|PQ_INT, 0, (void*)&sip_mask_len, 0);
    parse_table_add(&pt, "L4DstPort", PQ_DFL|PQ_INT, 0, (void*)&dport, 0);
    parse_table_add(&pt, "L4SrcPort", PQ_DFL|PQ_INT, 0, (void*)&sport, 0);
    parse_table_add(&pt, "TYpe", PQ_DFL|PQ_INT, 0, (void*)&tnl_type, 0);
    parse_table_add(&pt, "OuterDSCP", PQ_DFL|PQ_BOOL, 0,(void *)&outer_dscp, 0);
    parse_table_add(&pt, "OuterTTL", PQ_DFL|PQ_BOOL, 0, (void *)&outer_ttl, 0);
    parse_table_add(&pt, "InnerDSCP", PQ_DFL|PQ_BOOL, 0,(void *)&inner_dscp, 0);
    parse_table_add(&pt, "TrustDSCP", PQ_DFL|PQ_BOOL, 0,(void *)&trust_dscp, 0);
    parse_table_add(&pt, "VLanid", PQ_DFL|PQ_INT, 0, (void*)&vlan, 0);
    parse_table_add(&pt, "AllowIPv6Payload", PQ_DFL|PQ_BOOL, 0, (void*)&allow_v4, 0);
    parse_table_add(&pt, "AllowIPv4Payload", PQ_DFL|PQ_BOOL, 0, (void*)&allow_v6, 0);
    parse_table_add(&pt, "PBMP", PQ_DFL|PQ_PBMP|PQ_BCM, 0, &pbmap, 0);
    parse_table_add(&pt, "Replace", PQ_DFL|PQ_BOOL, 0, &replace, 0);
    parse_table_add(&pt, "RemoteTerminate", PQ_DFL|PQ_INT, 0, &remote_port, 0);
    parse_table_add(&pt, "TunnelID", PQ_DFL|PQ_INT, 0, (void*)&tunnel_id, 0);

    if (!parseEndOk(a, &pt, &retCode)) {
        return retCode;
    }

    sal_memset(&tunnel_term, 0, sizeof (tunnel_term));

    if ((tnl_type == bcmTunnelTypeWlanWtpToAc) || 
        (tnl_type == bcmTunnelTypeWlanAcToAc) ||
        (tnl_type == bcmTunnelTypeWlanWtpToAc6) || 
        (tnl_type == bcmTunnelTypeWlanAcToAc6)) { 
        tunnel_term.flags |= BCM_TUNNEL_TERM_TUNNEL_WITH_ID;
        tunnel_term.tunnel_id = tunnel_id;
        if (remote_port != BCM_GPORT_INVALID) {
            tunnel_term.flags |= BCM_TUNNEL_TERM_WLAN_REMOTE_TERMINATE;
            tunnel_term.remote_port = remote_port;
        }
    }

    if (_BCM_TUNNEL_OUTER_HEADER_IPV6(tnl_type)) {
        sal_memcpy(tunnel_term.sip6, sip6, sizeof(bcm_ip6_t));
        sal_memcpy(tunnel_term.dip6, dip6, sizeof(bcm_ip6_t));
        bcm_ip6_mask_create(tunnel_term.sip6_mask, sip_mask_len);
        bcm_ip6_mask_create(tunnel_term.dip6_mask, dip_mask_len);
    } else {
        tunnel_term.sip = sip;
        tunnel_term.dip = dip;
        tunnel_term.sip_mask = bcm_ip_mask_create(sip_mask_len);
        tunnel_term.dip_mask = bcm_ip_mask_create(dip_mask_len);
    }

    tunnel_term.udp_dst_port = dport;
    tunnel_term.udp_src_port = sport;
    tunnel_term.type = tnl_type;
    tunnel_term.vrf = vrf;

    if (outer_dscp) {
        tunnel_term.flags |= BCM_TUNNEL_TERM_USE_OUTER_DSCP;
    }

    if (outer_ttl) {
        tunnel_term.flags |= BCM_TUNNEL_TERM_USE_OUTER_TTL;
    }
    if (inner_dscp) {
        tunnel_term.flags |= BCM_TUNNEL_TERM_KEEP_INNER_DSCP;
    }

    if (trust_dscp) {
        tunnel_term.flags |= BCM_TUNNEL_TERM_DSCP_TRUST;
    }

    if (replace) {
        tunnel_term.flags |= BCM_TUNNEL_REPLACE;
    }

    tunnel_term.vlan = (bcm_vlan_t)vlan;
    tunnel_term.pbmp = pbmap;

    if ((rv = bcm_tunnel_terminator_add(unit, &tunnel_term)) < 0) {
        printk("ERROR %s: adding tunnel term %s\n",
               ARG_CMD(a), bcm_errmsg(rv));
        return CMD_FAIL;
    }
    return CMD_OK;
}

/*
 * Function:
 *      _l3_cmd_tunnel_term_delete
 * Description:
 * 	    Service routine used to delete tunnel terminator entry.
 * Parameters:
 *      unit      - (IN) Device number.
 *      a         - (IN) Command arguments. 
 * Returns: 
 *      CMD_XXX
 */
STATIC cmd_result_t
_l3_cmd_tunnel_term_delete(int unit, args_t *a) 
{
    bcm_tunnel_terminator_t tunnel_term;
    cmd_result_t      retCode; 
    parse_table_t     pt;     
    int               rv;
    bcm_ip6_t         sip6;
    bcm_ip6_t         dip6;
    int               dip = 0;
    int               sip = 0;
    int               sip_mask_len = 0; 
    int               dip_mask_len = 0;
    int               tnl_type = 0;
    int               vrf = 0;
    int               dport = 0;
    int               sport = 0;

    parse_table_init(unit, &pt);
    sal_memset(sip6, 0, sizeof(bcm_ip6_t));
    sal_memset(dip6, 0, sizeof(bcm_ip6_t));

    parse_table_add(&pt, "VRF",  PQ_DFL | PQ_INT, 0, (void *)&vrf, 0);
    parse_table_add(&pt, "DIP",  PQ_DFL|PQ_IP, 0, (void*)&dip, 0);
    parse_table_add(&pt, "SIP",  PQ_DFL|PQ_IP, 0, (void*)&sip, 0);
    parse_table_add(&pt, "DIP6",  PQ_DFL|PQ_IP6, 0, (void*)&dip6, 0);
    parse_table_add(&pt, "SIP6",  PQ_DFL|PQ_IP6, 0, (void*)&sip6, 0);
    parse_table_add(&pt, "DipMaskLen", PQ_DFL|PQ_INT, 0, (void*)&dip_mask_len, 0);
    parse_table_add(&pt, "SipMaskLen", PQ_DFL|PQ_INT, 0, (void*)&sip_mask_len, 0);
    parse_table_add(&pt, "TYpe", PQ_DFL|PQ_INT, 0, (void*)&tnl_type, 0);
    parse_table_add(&pt, "L4DstPort", PQ_DFL|PQ_INT, 0, (void*)&dport, 0);
    parse_table_add(&pt, "L4SrcPort", PQ_DFL|PQ_INT, 0, (void*)&sport, 0);

    if (!parseEndOk(a, &pt, &retCode)) {
        return retCode;
    }

    sal_memset(&tunnel_term, 0, sizeof (tunnel_term));


    if (_BCM_TUNNEL_OUTER_HEADER_IPV6(tnl_type)) {
        sal_memcpy(tunnel_term.sip6, sip6, sizeof(bcm_ip6_t));
        sal_memcpy(tunnel_term.dip6, dip6, sizeof(bcm_ip6_t));
        bcm_ip6_mask_create(tunnel_term.sip6_mask, sip_mask_len);
        bcm_ip6_mask_create(tunnel_term.dip6_mask, dip_mask_len);
    } else {
        tunnel_term.sip = sip;
        tunnel_term.dip = dip;
        tunnel_term.sip_mask = bcm_ip_mask_create(sip_mask_len);
        tunnel_term.dip_mask = bcm_ip_mask_create(dip_mask_len);
    }

    tunnel_term.udp_dst_port = dport;
    tunnel_term.udp_src_port = sport;
    tunnel_term.type = tnl_type;
    tunnel_term.vrf = vrf;

    if ((rv = bcm_tunnel_terminator_delete(unit, &tunnel_term)) < 0) {
        printk("ERROR %s: adding tunnel term %s\n",
               ARG_CMD(a), bcm_errmsg(rv));
        return CMD_FAIL;
    }
    return CMD_OK;
}

/*
 * Function:
 *      _l3_cmd_tunnel_term_get
 * Description:
 * 	    Service routine used to read tunnel terminator entry.
 * Parameters:
 *      unit      - (IN) Device number.
 *      a         - (IN) Command arguments. 
 * Returns: 
 *      CMD_XXX
 */
STATIC cmd_result_t
_l3_cmd_tunnel_term_get(int unit, args_t *a) 
{
    bcm_tunnel_terminator_t tunnel_term;
    cmd_result_t      retCode; 
    parse_table_t     pt;     
    int               rv;
    bcm_ip6_t         sip6;
    bcm_ip6_t         dip6;
    int               vlan = 0;
    int               dip = 0;
    int               sip = 0;
    int               sip_mask_len = 0; 
    int               dip_mask_len = 0;
    int               tnl_type = 0;
    int               vrf = 0;
    int               dport = 0;
    int               sport = 0;

    parse_table_init(unit, &pt);
    sal_memset(sip6, 0, sizeof(bcm_ip6_t));
    sal_memset(dip6, 0, sizeof(bcm_ip6_t));
    sal_memset(&tunnel_term, 0, sizeof(bcm_tunnel_terminator_t));

    parse_table_add(&pt, "VRF",  PQ_DFL | PQ_INT, 0, (void *)&vrf, 0);
    parse_table_add(&pt, "DIP",  PQ_DFL|PQ_IP, 0, (void*)&dip, 0);
    parse_table_add(&pt, "SIP",  PQ_DFL|PQ_IP, 0, (void*)&sip, 0);
    parse_table_add(&pt, "DIP6",  PQ_DFL|PQ_IP6, 0, (void*)&dip6, 0);
    parse_table_add(&pt, "SIP6",  PQ_DFL|PQ_IP6, 0, (void*)&sip6, 0);
    parse_table_add(&pt, "DipMaskLen", PQ_DFL|PQ_INT, 0, (void*)&dip_mask_len, 0);
    parse_table_add(&pt, "SipMaskLen", PQ_DFL|PQ_INT, 0, (void*)&sip_mask_len, 0);
    parse_table_add(&pt, "TYpe", PQ_DFL|PQ_INT, 0, (void*)&tnl_type, 0);
    parse_table_add(&pt, "L4DstPort", PQ_DFL|PQ_INT, 0, (void*)&dport, 0);
    parse_table_add(&pt, "L4SrcPort", PQ_DFL|PQ_INT, 0, (void*)&sport, 0);
    parse_table_add(&pt, "VLAN", PQ_DFL|PQ_INT, 0, (void*)&vlan, 0);

    if (!parseEndOk(a, &pt, &retCode)) {
        return retCode;
    }

    sal_memset(&tunnel_term, 0, sizeof (tunnel_term));


    if (_BCM_TUNNEL_OUTER_HEADER_IPV6(tnl_type)) {
        sal_memcpy(tunnel_term.sip6, sip6, sizeof(bcm_ip6_t));
        sal_memcpy(tunnel_term.dip6, dip6, sizeof(bcm_ip6_t));
        bcm_ip6_mask_create(tunnel_term.sip6_mask, sip_mask_len);
        bcm_ip6_mask_create(tunnel_term.dip6_mask, dip_mask_len);
    } else {
        tunnel_term.sip = sip;
        tunnel_term.dip = dip;
        tunnel_term.sip_mask = bcm_ip_mask_create(sip_mask_len);
        tunnel_term.dip_mask = bcm_ip_mask_create(dip_mask_len);
    }

    tunnel_term.udp_dst_port = dport;
    tunnel_term.udp_src_port = sport;
    tunnel_term.type = tnl_type;
    tunnel_term.vrf = vrf;
    tunnel_term.vlan = (bcm_vlan_t)vlan;

    if ((rv = bcm_tunnel_terminator_get(unit, &tunnel_term)) < 0) {
        printk("ERROR %s: adding tunnel term %s\n",
               ARG_CMD(a), bcm_errmsg(rv));
        return CMD_FAIL;
    }

    _l3_cmd_tunnel_term_print(unit, &tunnel_term);
    return CMD_OK;
}

/*
 * Function:
 *      _l3_cmd_dscp_map_add
 * Description:
 * 	    Service routine used to add dscp map object.
 * Parameters:
 *      unit      - (IN) Device number.
 *      a         - (IN) Command arguments. 
 * Returns: 
 *      CMD_XXX
 */
STATIC cmd_result_t
_l3_cmd_dscp_map_add(int unit, args_t *a) 
{
    int dscp_map_id = -1;
    int               rv;

    rv = bcm_tunnel_dscp_map_create(unit, 0, &dscp_map_id);
    if (BCM_FAILURE(rv)) {
        printk("%s: Failed to create egress dscp map id: %s\n",
               ARG_CMD(a), bcm_errmsg(rv));
        return (CMD_FAIL);
    } else {
        printk("%s: Returned DSCP Map ID %d\n", ARG_CMD(a), dscp_map_id);
    }
    return (CMD_OK);
}

/*
 * Function:
 *      _l3_cmd_dscp_map_delete
 * Description:
 * 	    Service routine used to delete dscp map object.
 * Parameters:
 *      unit      - (IN) Device number.
 *      a         - (IN) Command arguments. 
 * Returns: 
 *      CMD_XXX
 */
STATIC cmd_result_t
_l3_cmd_dscp_map_delete(int unit, args_t *a) 
{
    cmd_result_t      retCode; 
    parse_table_t     pt;     
    int               dscp_map_id = -1;
    int               rv;

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "DscpMapId", PQ_DFL|PQ_INT, 0, (void*)&dscp_map_id, 0);
    if (!parseEndOk(a, &pt, &retCode)) {
        return retCode;
    }

    if (-1 == dscp_map_id) {
        return(CMD_USAGE);
    }

    rv = bcm_tunnel_dscp_map_destroy(unit, dscp_map_id);
    if (BCM_FAILURE(rv)) {
        printk("%s: Error deleting entry from egress : %s\n",
               ARG_CMD(a), bcm_errmsg(rv));
        return (CMD_FAIL);
    } 
    return (CMD_OK);
}
/*
 * Function:
 *      _l3_cmd_dscp_map_port_set
 * Description:
 * 	    Service routine used to set dscp map priority, color -> dscp mapping.
 * Parameters:
 *      unit      - (IN) Device number.
 *      a         - (IN) Command arguments. 
 * Returns: 
 *      CMD_XXX
 */
STATIC cmd_result_t
_l3_cmd_dscp_map_port_set(int unit, args_t *a) 
{
    parse_table_t     pt;     
    bcm_tunnel_dscp_map_t dscp_map;
    int prio, color, dscp_val;
    cmd_result_t      retCode; 
    bcm_port_t        port = -1;
    int rv;

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "Port", PQ_DFL|PQ_PORT, 0, (void*)&port, 0);
    parse_table_add(&pt, "Priority", PQ_DFL|PQ_INT, 0, (void*)&prio, 0);
    parse_table_add(&pt, "Color",  PQ_DFL|PQ_INT, 0, (void*)&color, 0);
    parse_table_add(&pt, "DSCPValue", PQ_DFL|PQ_INT, 0, (void *)&dscp_val, 0);
    if (!parseEndOk(a, &pt, &retCode)) {
        return retCode;
    }

    if (-1 == port) {
        return (CMD_USAGE);
    }

    dscp_map.priority = prio;
    dscp_map.color = color;
    dscp_map.dscp = dscp_val;

    rv = bcm_tunnel_dscp_map_port_set(unit, port, &dscp_map);

    if (BCM_FAILURE(rv)) {
        printk("ERROR %s: setting tunnel DSCP Map for port %d %s\n",
               ARG_CMD(a), port, bcm_errmsg(rv));
        return (CMD_FAIL);
    }
    return (CMD_OK);
}

/*
 * Function:
 *      _l3_cmd_dscp_map_port_show
 * Description:
 * 	    Service routine used to show dscp map object.
 * Parameters:
 *      unit      - (IN) Device number.
 *      a         - (IN) Command arguments. 
 * Returns: 
 *      CMD_XXX
 */
STATIC cmd_result_t
_l3_cmd_dscp_map_port_show(int unit, args_t *a) 
{
    parse_table_t     pt;     
    bcm_tunnel_dscp_map_t dscp_map;
    int prio;
    cmd_result_t      retCode; 
    bcm_port_t port = -1;
    int rv;

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "Port", PQ_DFL | PQ_PORT, 0,
                    (void *)&port, 0);
    if (!parseEndOk(a, &pt, &retCode))
        return retCode;

    if (-1 == port) {
        return(CMD_USAGE);
    }

    printk("\nDSCP Map for port %d:\n", port);
    printk("\n-----------\n");
    for (prio = 0; prio < 8; prio++) {
        dscp_map.priority = prio;
        dscp_map.color = bcmColorGreen;
        rv = bcm_tunnel_dscp_map_port_get(unit, port, &dscp_map);
        if (BCM_FAILURE(rv)) {
            printk("%s: Error L3 accessing: %s\n", ARG_CMD(a), bcm_errmsg(rv));
            return CMD_FAIL;
        }
        printk(" Priority = %d, Color = Green,  DSCP = %d\n",
               prio, dscp_map.dscp);

        dscp_map.priority = prio;
        dscp_map.color = bcmColorYellow;
        rv = bcm_tunnel_dscp_map_port_get(unit, port, &dscp_map);
        if (BCM_FAILURE(rv)) {
            printk("%s: Error L3 accessing: %s\n", ARG_CMD(a), bcm_errmsg(rv));
            return CMD_FAIL;
        }
        printk(" Priority = %d, Color = Yellow, DSCP = %d\n",
               prio, dscp_map.dscp);

        dscp_map.priority = prio;
        dscp_map.color = bcmColorRed;
        rv = bcm_tunnel_dscp_map_port_get(unit, port, &dscp_map);
        if (BCM_FAILURE(rv)) {
            printk("%s: Error L3 accessing: %s\n", ARG_CMD(a), bcm_errmsg(rv));
            return CMD_FAIL;
        }
        printk(" Priority = %d, Color = Red,    DSCP = %d\n",
               prio, dscp_map.dscp);
    }
    return CMD_OK;
}


/*
 * Function:
 *      _l3_cmd_dscp_map_show
 * Description:
 * 	    Service routine used to set dscp map priority, color -> dscp mapping.
 * Parameters:
 *      unit      - (IN) Device number.
 *      a         - (IN) Command arguments. 
 * Returns: 
 *      CMD_XXX
 */
STATIC cmd_result_t
_l3_cmd_dscp_map_set(int unit, args_t *a) 
{
    parse_table_t     pt;     
    bcm_tunnel_dscp_map_t dscp_map;
    int prio, color, dscp_val;
    cmd_result_t      retCode; 
    int dscp_map_id = -1;
    int rv;

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "DscpMapId", PQ_DFL|PQ_INT, 0, 
                    (void*)&dscp_map_id, 0);
    parse_table_add(&pt, "Priority", PQ_DFL|PQ_INT, 0, (void*)&prio, 0);
    parse_table_add(&pt, "Color",  PQ_DFL|PQ_INT, 0, (void*)&color, 0);
    parse_table_add(&pt, "DSCPValue", PQ_DFL|PQ_INT, 0, (void *)&dscp_val, 0);
    if (!parseEndOk(a, &pt, &retCode)) {
        return retCode;
    }

    if (-1 == dscp_map_id) {
        return(CMD_USAGE);
    }

    dscp_map.priority = prio;
    dscp_map.color = color;
    dscp_map.dscp = dscp_val;

    rv = bcm_tunnel_dscp_map_set(unit, dscp_map_id, &dscp_map);

    if (BCM_FAILURE(rv)) {
        printk("ERROR %s: setting tunnel DSCP Map for %d %s\n",
               ARG_CMD(a), dscp_map_id, bcm_errmsg(rv));
        return (CMD_FAIL);
    }
    return (CMD_OK);
}

/*
 * Function:
 *      _l3_cmd_dscp_map_show
 * Description:
 * 	    Service routine used to show dscp map object.
 * Parameters:
 *      unit      - (IN) Device number.
 *      a         - (IN) Command arguments. 
 * Returns: 
 *      CMD_XXX
 */
STATIC cmd_result_t
_l3_cmd_dscp_map_show(int unit, args_t *a) 
{
    parse_table_t     pt;     
    bcm_tunnel_dscp_map_t dscp_map;
    int prio;
    cmd_result_t      retCode; 
    int dscp_map_id = -1;
    int rv;

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "DscpMapId", PQ_DFL | PQ_INT, 0,
                    (void *)&dscp_map_id, 0);
    if (!parseEndOk(a, &pt, &retCode))
        return retCode;

    if (-1 == dscp_map_id) {
        return(CMD_USAGE);
    }

    printk("\nDSCP Map %d:\n", dscp_map_id);
    printk("\n-----------\n");
    for (prio = 0; prio < 8; prio++) {
        dscp_map.priority = prio;
        dscp_map.color = bcmColorGreen;
        rv = bcm_tunnel_dscp_map_get(unit, dscp_map_id, &dscp_map);
        if (BCM_FAILURE(rv)) {
            printk("%s: Error L3 accessing: %s\n", ARG_CMD(a), bcm_errmsg(rv));
            return CMD_FAIL;
        }
        printk(" Priority = %d, Color = Green,  DSCP = %d\n",
               prio, dscp_map.dscp);

        dscp_map.priority = prio;
        dscp_map.color = bcmColorYellow;
        rv = bcm_tunnel_dscp_map_get(unit, dscp_map_id, &dscp_map);
        if (BCM_FAILURE(rv)) {
            printk("%s: Error L3 accessing: %s\n", ARG_CMD(a), bcm_errmsg(rv));
            return CMD_FAIL;
        }
        printk(" Priority = %d, Color = Yellow, DSCP = %d\n",
               prio, dscp_map.dscp);

        dscp_map.priority = prio;
        dscp_map.color = bcmColorRed;
        rv = bcm_tunnel_dscp_map_get(unit, dscp_map_id, &dscp_map);
        if (BCM_FAILURE(rv)) {
            printk("%s: Error L3 accessing: %s\n", ARG_CMD(a), bcm_errmsg(rv));
            return CMD_FAIL;
        }
        printk(" Priority = %d, Color = Red,    DSCP = %d\n",
               prio, dscp_map.dscp);
    }
    return CMD_OK;
}

/*
 * Function:
 *      _l3_cmd_egress_intf_create
 * Description:
 * 	    Service routine used to create/add egress l3 interface.
 * Parameters:
 *      unit      - (IN) Device number.
 *      a         - (IN) Command arguments. 
 * Returns: 
 *      CMD_XXX
 */
STATIC cmd_result_t
_l3_cmd_egress_intf_create(int unit, args_t *a) 
{
    cmd_result_t      retCode; 
    parse_table_t     pt;     
    int               rv;
    bcm_mac_t         mac;
    bcm_l3_intf_t     intf;
    bcm_if_t          interface = -1;
    int               vid = 0;
#if defined(BCM_XGS3_SWITCH_SUPPORT)
    int               inner_vlan = 0;
    bcm_vrf_t         vrf = 0;
    bcm_if_group_t    group = 0;
    int               mtu = 0;
#endif /* BCM_XGS_SWITCH_SUPPORT */

    parse_table_init(unit, &pt);
    sal_memset(mac, 0, sizeof(bcm_mac_t));
    bcm_l3_intf_t_init(&intf);

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "Vlan", PQ_DFL|PQ_INT, 0, &vid, NULL);
    parse_table_add(&pt, "Mac",  PQ_DFL|PQ_MAC, 0, mac, NULL);
    parse_table_add(&pt, "INtf", PQ_DFL|PQ_INT, 0, (void*)&interface, 0);
#if defined(BCM_XGS3_SWITCH_SUPPORT)
    parse_table_add(&pt, "InnerVlan", PQ_DFL|PQ_INT, 0, &inner_vlan, NULL);
    parse_table_add(&pt, "VRF", PQ_DFL | PQ_INT, 0, (void *)&vrf, 0);
    parse_table_add(&pt, "Group", PQ_DFL | PQ_INT, 0, (void *)&group, 0);
    parse_table_add(&pt, "MTU", PQ_DFL | PQ_INT, 0, (void *)&mtu, 0);
#endif
    if (!parseEndOk(a, &pt, &retCode)) {
        return retCode;
    }

    sal_memcpy(intf.l3a_mac_addr, mac, sizeof(bcm_mac_t));
    intf.l3a_vid = (bcm_vlan_t) vid;
    intf.l3a_flags |= BCM_L3_ADD_TO_ARL;
    if (interface != -1) {
        intf.l3a_intf_id = interface;
        intf.l3a_flags |= BCM_L3_WITH_ID;
    }
#if defined(BCM_XGS3_SWITCH_SUPPORT)
    intf.l3a_vrf = vrf;
    intf.l3a_group = group;
    intf.l3a_mtu = mtu;
    intf.l3a_inner_vlan = inner_vlan;
#endif
    rv = bcm_l3_intf_create(unit, &intf);
    if (BCM_FAILURE(rv)) {
        printk("%s: Error creating entry to L3 Intf table: %s\n",
               ARG_CMD(a), bcm_errmsg(rv));
        return (CMD_FAIL);
    }
    return (CMD_OK);
}
/*
 * Function:
 *      _l3_cmd_egress_intf_delete
 * Description:
 * 	    Service routine used to remove egress l3 interface.
 * Parameters:
 *      unit      - (IN) Device number.
 *      a         - (IN) Command arguments. 
 * Returns: 
 *      CMD_XXX
 */
STATIC cmd_result_t
_l3_cmd_egress_intf_delete(int unit, args_t *a) 
{
    cmd_result_t      retCode; 
    parse_table_t     pt;     
    int               rv;
    bcm_if_t          interface = -1;

    if (!ARG_CNT(a)) {
        printk("Expected command parameters: Intf=id\n");
        return (CMD_FAIL);
    }

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "INtf", PQ_DFL|PQ_INT, 0, (void*)&interface, 0);
    if (!parseEndOk(a, &pt, &retCode)) {
        return retCode;
    }

    if (-1 == interface) {
        printk("Expected command parameters: Intf=id\n");
        return (CMD_FAIL);
    }

    if ((rv = bcm_l3_interface_destroy(unit, interface)) < 0) {
        printk("%s: Error (%s) destroy interface (%d)\n",
               ARG_CMD(a), bcm_errmsg(rv), interface);
        return (CMD_FAIL);
    }

    return (CMD_OK);
}


/*
 * Function:
 *      _l3_cmd_egress_intf_delete_all
 * Description:
 * 	    Service routine used to flush all egress l3 interfaces.
 * Parameters:
 *      unit      - (IN) Device number.
 *      a         - (IN) Command arguments. 
 * Returns: 
 *      CMD_XXX
 */
STATIC cmd_result_t
_l3_cmd_egress_intf_delete_all(int unit, args_t *a) 
{
    int             rv;
    int             idx;
    bcm_l3_info_t   l3_hw_status;

    if ((rv = bcm_l3_info(unit, &l3_hw_status)) < 0) {
        printk("%s: Error L3 accessing: %s\n", ARG_CMD(a), bcm_errmsg(rv));
        return CMD_FAIL;
    }

    for (idx = 0; idx < l3_hw_status.l3info_max_intf; idx++) {
        rv = bcm_l3_interface_destroy(unit, idx);

        if (BCM_FAILURE(rv) && rv != BCM_E_NOT_FOUND) {
            printk("%s: Error destroy interface %d from L3 Intf"
                   "table: %s\n", ARG_CMD(a), idx, bcm_errmsg(rv));
            return (CMD_FAIL);
        }
    }
    return (CMD_OK);
}

/*
 * Function:
 *      _l3_cmd_egress_intf_show
 * Description:
 * 	    Service routine used to show l3 egress interfaces.
 * Parameters:
 *      unit      - (IN) Device number.
 *      a         - (IN) Command arguments. 
 * Returns: 
 *      CMD_XXX
 */
STATIC cmd_result_t
_l3_cmd_egress_intf_show(int unit, args_t *a) 
{
    int             rv;
    parse_table_t   pt;     
    bcm_l3_intf_t   intf;
    cmd_result_t    retCode;
    bcm_l3_info_t   l3_hw_status;
    int             free_l3intf;
    int             print_hdr = TRUE;
    bcm_if_t        interface = -1;


    if ((rv = bcm_l3_info(unit, &l3_hw_status)) < 0) {
        printk("Error in L3 info access: %s\n",  bcm_errmsg(rv));
        return CMD_FAIL;
    }

    free_l3intf  = l3_hw_status.l3info_max_intf -
        l3_hw_status.l3info_occupied_intf;


    if (ARG_CNT(a)) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "INtf", PQ_DFL|PQ_INT, 0, (void*)&interface, 0);
        if (!parseEndOk(a, &pt, &retCode)) {
            return retCode;
        }
    }

    if (interface != -1) {
        if ((interface < 0)  || (interface > l3_hw_status.l3info_max_intf)) {
            printk("Invalid interface index: %d\n", interface);
            return (CMD_FAIL);
        }  
    }

    printk("Free L3INTF entries: %d\n", free_l3intf);

    if (interface != -1) {
        sal_memset(&intf, 0, sizeof (bcm_l3_intf_t));
        intf.l3a_intf_id = interface;
        rv = bcm_l3_intf_get(unit, &intf);
        if (BCM_SUCCESS(rv)) {
            _l3_cmd_egress_intf_print(unit, TRUE, &intf);
            return (CMD_OK);
        } else {
            printk("Error L3 interface %d: %s\n", interface, bcm_errmsg(rv));
            return (CMD_FAIL);
        }
    }

    /* Note: last interface id is reserved for Copy To Cpu purposes. */
    for (interface = 0; 
         interface < l3_hw_status.l3info_max_intf - 1; interface++) {
        sal_memset(&intf, 0, sizeof (bcm_l3_intf_t));
        intf.l3a_intf_id = interface;
        rv = bcm_l3_intf_get(unit, &intf);
        if (BCM_SUCCESS(rv)) {
            _l3_cmd_egress_intf_print(unit, print_hdr, &intf);
            print_hdr = FALSE;
        } else if (rv == BCM_E_NOT_FOUND) {
            continue;
        } else if (BCM_FAILURE(rv)) {
            printk("Error traverse l3 interfaces: %s\n", bcm_errmsg(rv));
        }
    }
    return CMD_OK;
}

#if defined(BCM_XGS3_SWITCH_SUPPORT)
/*
 * Function:
 *      _l3_cmd_egress_object_create
 * Description:
 * 	    Service routine used to create/update l3 egress object.
 * Parameters:
 *      unit      - (IN) Device number.
 *      a         - (IN) Command arguments. 
 * Returns: 
 *      CMD_XXX
 */
STATIC cmd_result_t
_l3_cmd_egress_object_create(int unit, args_t *a) 
{
    cmd_result_t      retCode; 
    parse_table_t     pt;     
    int               rv;
    bcm_mac_t         mac;
    bcm_l3_egress_t   egress_object;
    bcm_if_t          interface = -1;
    bcm_port_t        port = 0;
    bcm_module_t      module = 0;
    bcm_trunk_t       trunk = -1;
    int               l2tocpu = 0;
    int               copytocpu = 0;
    int               drop = 0;
    int               mpls_label = 0;
    int               object_id = -1;
    uint32            flags = 0;

    bcm_l3_egress_t_init(&egress_object);
    sal_memset(mac, 0, sizeof(bcm_mac_t));

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "Mac",  PQ_DFL | PQ_MAC, 0, (void *)mac, 0);
    parse_table_add(&pt, "Port", PQ_DFL | PQ_PORT, 0, (void *)&port, 0);
    parse_table_add(&pt, "INtf", PQ_DFL | PQ_INT, 0, (void *)&interface, 0);
    parse_table_add(&pt, "MOdule", PQ_DFL | PQ_INT, 0, (void *)&module, 0);
    parse_table_add(&pt, "Trunk", PQ_DFL | PQ_INT, 0, (void *)&trunk, 0);
    parse_table_add(&pt, "L2tocpu", PQ_DFL | PQ_BOOL, 0, (void *)&l2tocpu, 0);
    parse_table_add(&pt, "CopyToCpu", PQ_DFL | PQ_BOOL, 0, (void *)&copytocpu, 0);
    parse_table_add(&pt, "Drop", PQ_DFL | PQ_BOOL, 0, (void *)&drop, 0);
    parse_table_add(&pt, "MplsLabel", PQ_DFL | PQ_INT, 0,
                    (void *)&mpls_label, 0);
    parse_table_add(&pt, "EgrId", PQ_DFL | PQ_INT, 0, (void *)&object_id, 0);
    if (!parseEndOk(a, &pt, &retCode))
        return retCode;

    egress_object.intf   = interface;
    egress_object.mpls_label = mpls_label;
    sal_memcpy(egress_object.mac_addr, mac, sizeof(mac));

    if (BCM_GPORT_IS_SET(port)) {
        egress_object.port = port;
    } else {
        egress_object.module = module;
    if (trunk >= 0) {
        egress_object.flags |= BCM_L3_TGID;
        egress_object.trunk = trunk;
    } else {
        egress_object.port = port;
    }
    }

    if (l2tocpu) {
        egress_object.flags |= BCM_L3_L2TOCPU;
    }

    if (copytocpu) {
        egress_object.flags |= (BCM_L3_COPY_TO_CPU);
    }

    if (drop) {
        egress_object.flags |= (BCM_L3_DST_DISCARD);
    }

    if (object_id >= 0) {
        flags = (BCM_L3_WITH_ID | BCM_L3_REPLACE);
    }


    rv = bcm_l3_egress_create(unit, flags, &egress_object, &object_id);
    if (BCM_FAILURE(rv)) {
        printk("%s: Error creating egress object entry: %s\n",
               ARG_CMD(a), bcm_errmsg(rv));
        return (CMD_FAIL);
    }

    var_set_integer(ENV_EGRESS_OBJECT_ID, object_id, TRUE, FALSE);
    printk("New egress object index: %d\n", object_id);
    printk("Environment variable (%s) was set\n", ENV_EGRESS_OBJECT_ID);
    return (CMD_OK);
}

/*
 * Function:
 *      _l3_cmd_egress_object_find
 * Description:
 * 	    Service routine used to locate id of l3 egress object.
 * Parameters:
 *      unit      - (IN) Device number.
 *      a         - (IN) Command arguments. 
 * Returns: 
 *      CMD_XXX
 */
STATIC cmd_result_t
_l3_cmd_egress_object_find(int unit, args_t *a) 
{
    cmd_result_t      retCode; 
    parse_table_t     pt;     
    int               rv;
    bcm_mac_t         mac;
    bcm_l3_egress_t   egress_object;
    bcm_if_t          interface = -1;
    bcm_port_t        port = 0;
    bcm_module_t      module = 0;
    bcm_trunk_t       trunk = -1;
    int               l2tocpu = 0;
    int               mpls_label = 0;
    int               object_id = -1;

    bcm_l3_egress_t_init(&egress_object);
    sal_memset(mac, 0, sizeof(bcm_mac_t));

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "Mac",  PQ_DFL | PQ_MAC, 0, (void *)mac, 0);
    parse_table_add(&pt, "Port", PQ_DFL | PQ_PORT, 0, (void *)&port, 0);
    parse_table_add(&pt, "INtf", PQ_DFL | PQ_INT, 0, (void *)&interface, 0);
    parse_table_add(&pt, "MOdule", PQ_DFL | PQ_INT, 0, (void *)&module, 0);
    parse_table_add(&pt, "Trunk", PQ_DFL | PQ_INT, 0, (void *)&trunk, 0);
    parse_table_add(&pt, "L2tocpu", PQ_DFL | PQ_BOOL, 0, (void *)&l2tocpu, 0);
    parse_table_add(&pt, "MplsLabel", PQ_DFL | PQ_INT, 0,
                    (void *)&mpls_label, 0);
    if (!parseEndOk(a, &pt, &retCode))
        return retCode;

    egress_object.intf   = interface;
    egress_object.mpls_label = mpls_label;
    sal_memcpy(egress_object.mac_addr, mac, sizeof(mac));

    if (BCM_GPORT_IS_SET(port)) {
        egress_object.port = port;
    } else {
        egress_object.module = module;
    if (trunk >= 0) {
        egress_object.flags |= BCM_L3_TGID;
        egress_object.trunk = trunk;
    } else {
        egress_object.port = port;
    }
    }

    if (l2tocpu) {
        egress_object.flags |= BCM_L3_L2TOCPU;
    }

    rv = bcm_l3_egress_find(unit, &egress_object, &object_id);
    if (BCM_FAILURE(rv)) {
        if (rv != BCM_E_NOT_FOUND) {
            printk("%s: Error finding egress object entry: %s\n",
                   ARG_CMD(a), bcm_errmsg(rv));
            return (CMD_FAIL);
        }
        printk("%s: Couldn't find entry\n",ARG_CMD(a));
        return (CMD_OK);
    }
    var_set_integer(ENV_EGRESS_OBJECT_ID, object_id, TRUE, FALSE);
    printk("Egress object interface id: %d\n", object_id);
    printk("Environment variable (%s) was set\n", ENV_EGRESS_OBJECT_ID);
    return (CMD_OK);
}


/*
 * Function:
 *      _l3_cmd_egress_object_destroy
 * Description:
 * 	    Service routine used to destroy l3 egress object by id.
 * Parameters:
 *      unit      - (IN) Device number.
 *      a         - (IN) Command arguments. 
 * Returns: 
 *      CMD_XXX
 */
STATIC cmd_result_t
_l3_cmd_egress_object_destroy(int unit, args_t *a) 
{
    int               rv;
    int               object_id = -1;

    if (ARG_CNT(a)) {
        if (!isint(ARG_CUR(a))) {
            printk("%s: Invalid number: %s\n", ARG_CMD(a), ARG_CUR(a));
            return (CMD_FAIL);
        }
        object_id = parse_integer(ARG_GET(a));
    } else {
        printk("%s: Invalid arguments: %s\n", ARG_CMD(a), ARG_CUR(a));
        return (CMD_FAIL);
    }

    rv = bcm_l3_egress_destroy(unit, object_id);
    if (BCM_FAILURE(rv))
    { 
        printk("%s: Error deleting egress object entry: %s\n", ARG_CMD(a),
               bcm_errmsg(rv));
        return (CMD_FAIL);
    }
    return (CMD_OK);
}

/*
 * Function:
 *      _l3_cmd_egress_object_show
 * Description:
 * 	    Service routine used to display l3 egress object/s.
 *      specific one or all of them. 
 * Parameters:
 *      unit      - (IN) Device number.
 *      a         - (IN) Command arguments. 
 * Returns: 
 *      CMD_XXX
 */
STATIC cmd_result_t
_l3_cmd_egress_object_show(int unit, args_t *a) 
{
    int               rv;
    int               object_id = -1;
    int               one_entry_only = 0;
    bcm_l3_egress_t   egress_object;

    bcm_l3_egress_t_init(&egress_object);

    if (ARG_CNT(a)) {
        if (!isint(ARG_CUR(a))) {
            printk("%s: Invalid number: %s\n", ARG_CMD(a), ARG_CUR(a));
            return (CMD_FAIL);
        }
        object_id = parse_integer(ARG_GET(a));
        one_entry_only = 1;
    }

    printk("Entry  Mac                 Vlan INTF PORT MOD MPLS_LABEL ToCpu Drop\n");
    if (one_entry_only) {
        rv = bcm_l3_egress_get(unit, object_id, &egress_object);
        if (BCM_FAILURE(rv)) {
            printk("%s: Error reading egress object entry: %s\n",
                   ARG_CMD(a), bcm_errmsg(rv));
            return (CMD_FAIL);
        }
        _l3_cmd_egress_obj_print(unit, object_id, &egress_object, NULL);
    } else {
        rv = bcm_l3_egress_traverse(unit, _l3_cmd_egress_obj_print, NULL);
        if (BCM_FAILURE(rv)) {
            printk("%s: Error reading egress object entry: %s\n",
                   ARG_CMD(a), bcm_errmsg(rv));
            return (CMD_FAIL);
        }
    }
    return (CMD_OK);
}


/*
 * Function:
 *      _l3_cmd_egress_mpath_object_create
 * Description:
 * 	    Service routine used to create/update l3 multipath egress object.
 * Parameters:
 *      unit      - (IN) Device number.
 *      a         - (IN) Command arguments. 
 * Returns: 
 *      CMD_XXX
 */
STATIC cmd_result_t
_l3_cmd_egress_mpath_object_create(int unit, args_t *a) 
{
    int               rv;
    parse_table_t     pt;     
    cmd_result_t      retCode; 
    bcm_if_t          mpath_egr[32];
    int               intf_count = 0;
    int               object_id = -1;
    int               flags = 0;


    parse_table_init(unit, &pt);
    parse_table_add(&pt, "EgrId", PQ_DFL | PQ_INT, 0, (void *)&object_id, 0);
    parse_table_add(&pt, "Size",  PQ_DFL | PQ_INT, 0, (void *)&intf_count, 0);
    parse_table_add(&pt, "Intf0", PQ_DFL | PQ_INT, 0, (void *)&mpath_egr[0], 0);
    parse_table_add(&pt, "Intf1", PQ_DFL | PQ_INT, 0, (void *)&mpath_egr[1], 0);
    parse_table_add(&pt, "Intf2", PQ_DFL | PQ_INT, 0, (void *)&mpath_egr[2], 0);
    parse_table_add(&pt, "Intf3", PQ_DFL | PQ_INT, 0, (void *)&mpath_egr[3], 0);

    if (!parseEndOk(a, &pt, &retCode))  { 
        return retCode;
    }

    if (object_id >= 0) {
        flags = (BCM_L3_WITH_ID | BCM_L3_REPLACE);
    }

    rv = bcm_l3_egress_multipath_create(unit, flags, intf_count, 
                                        mpath_egr, &object_id);
    if (BCM_FAILURE(rv)) {
        printk("%s: Error creating mpath egress object entry: %s\n",
               ARG_CMD(a), bcm_errmsg(rv));
        return (CMD_FAIL);
    }

    var_set_integer(ENV_EGRESS_OBJECT_ID, object_id, TRUE, FALSE);
    printk("New multipath egress object index: %d\n", object_id);
    printk("Environment variable (%s) was set\n", ENV_EGRESS_OBJECT_ID);
    return (CMD_OK);
}

/*
 * Function:
 *      _l3_cmd_egress_mpath_object_lookup
 * Description:
 * 	    Service routine used to search for matching l3 
 *      multipath egress object.
 * Parameters:
 *      unit      - (IN) Device number.
 *      a         - (IN) Command arguments. 
 * Returns: 
 *      CMD_XXX
 */
STATIC cmd_result_t
_l3_cmd_egress_mpath_object_lookup(int unit, args_t *a) 
{
    int               rv;
    parse_table_t     pt;     
    cmd_result_t      retCode; 
    bcm_if_t          mpath_egr[32];
    int               intf_count = 0;
    int               object_id = -1;

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "Size",  PQ_DFL | PQ_INT, 0, (void *)&intf_count, 0);
    parse_table_add(&pt, "Intf0", PQ_DFL | PQ_INT, 0, (void *)&mpath_egr[0], 0);
    parse_table_add(&pt, "Intf1", PQ_DFL | PQ_INT, 0, (void *)&mpath_egr[1], 0);
    parse_table_add(&pt, "Intf2", PQ_DFL | PQ_INT, 0, (void *)&mpath_egr[2], 0);
    parse_table_add(&pt, "Intf3", PQ_DFL | PQ_INT, 0, (void *)&mpath_egr[3], 0);
    if (!parseEndOk(a, &pt, &retCode)) {
        return retCode;
    }
    rv = bcm_l3_egress_multipath_find(unit, intf_count, mpath_egr, &object_id);

    if (BCM_FAILURE(rv)) {
        if (rv != BCM_E_NOT_FOUND) {
            printk("%s: Error finding mpath egress object entry: %s\n",
                   ARG_CMD(a), bcm_errmsg(rv));
            return CMD_FAIL;
        } 
        printk("%s: Couldn't find the entry\n",ARG_CMD(a));
        return CMD_OK;
    }

    var_set_integer(ENV_EGRESS_OBJECT_ID, object_id, TRUE, FALSE);
    printk("Egress mpath object interface id: %d\n", object_id);
    printk("Environment variable (%s) was set\n", ENV_EGRESS_OBJECT_ID);
    return CMD_OK;
} 

/*
 * Function:
 *      _l3_cmd_egress_mpath_object_destroy
 * Description:
 * 	    Service routine used to destroy l3 multipath egress object.
 * Parameters:
 *      unit      - (IN) Device number.
 *      a         - (IN) Command arguments. 
 * Returns: 
 *      CMD_XXX
 */
STATIC cmd_result_t
_l3_cmd_egress_mpath_object_destroy(int unit, args_t *a) 
{
    int               rv;
    int               object_id = -1;

    if (ARG_CNT(a)) {
        if (!isint(ARG_CUR(a))) {
            printk("%s: Invalid object id: %s\n", ARG_CMD(a), ARG_CUR(a));
            return (CMD_FAIL);
        }
        object_id = parse_integer(ARG_GET(a));
    } else {
        printk("%s: Invalid arguments: %s\n", ARG_CMD(a), ARG_CUR(a));
        return (CMD_FAIL);
    }

    rv = bcm_l3_egress_multipath_destroy(unit, object_id);
    if (BCM_FAILURE(rv)) {
        printk("%s: Error deleting mpath egress object entry: %s\n",
               ARG_CMD(a), bcm_errmsg(rv));
        return (CMD_FAIL);
    }
    return (CMD_OK);
}

/*
 * Function:
 *      _l3_cmd_egress_mpath_object_show
 * Description:
 * 	    Service routine used to destroy l3 multipath egress object.
 * Parameters:
 *      unit      - (IN) Device number.
 *      a         - (IN) Command arguments. 
 * Returns: 
 *      CMD_XXX
 */
STATIC cmd_result_t
_l3_cmd_egress_mpath_object_show(int unit, args_t *a) 
{
    int               rv;
    bcm_if_t          *mpath_egr;
    int               one_entry_only = 0;
    int               intf_count = 0;
    int               object_id = -1;

    if (ARG_CNT(a)) {
        if (!isint(ARG_CUR(a))) {
            printk("%s: Invalid number: %s\n", ARG_CMD(a), ARG_CUR(a));
            return (CMD_FAIL);
        }
        object_id = parse_integer(ARG_GET(a));
        one_entry_only = 1;
    }

    if (one_entry_only) {
        rv = bcm_l3_egress_multipath_get(unit, object_id, 0, NULL,
                                         &intf_count);
        mpath_egr = sal_alloc((intf_count * sizeof(int)), "ecmp group");
        rv = bcm_l3_egress_multipath_get(unit, object_id, intf_count, 
                                         mpath_egr, &intf_count);
        if (BCM_FAILURE(rv)) {
            printk("%s: Error reading egress object entry: %s\n",
                   ARG_CMD(a), bcm_errmsg(rv));
            return (CMD_FAIL);
        } 
        _l3_cmd_egress_mpath_obj_print(unit, object_id, intf_count, 
                                       mpath_egr, NULL);
    } else {
        rv = bcm_l3_egress_multipath_traverse(unit, 
                                              _l3_cmd_egress_mpath_obj_print, 
                                              NULL);
    }
    if (BCM_FAILURE(rv)) {
        return (CMD_FAIL);
    }
    return (CMD_OK);
}

/*
 * Function:
 *      _l3_cmd_egress_mpath_object_intf_insert
 * Description:
 * 	    Service routine used to add an egress interface to multipath 
 *      egress object. 
 * Parameters:
 *      unit      - (IN) Device number.
 *      a         - (IN) Command arguments. 
 * Returns: 
 *      CMD_XXX
 */
STATIC cmd_result_t
_l3_cmd_egress_mpath_object_intf_insert(int unit, args_t *a) 
{
    int               rv;
    parse_table_t     pt;     
    cmd_result_t      retCode; 
    int               object_id = -1;
    int               interface = 0;

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "EgrId", PQ_DFL | PQ_INT, 0, (void *)&object_id, 0);
    parse_table_add(&pt, "INtf", PQ_DFL | PQ_INT, 0, (void *)&interface, 0);

    if (!parseEndOk(a, &pt, &retCode))
        return retCode;

    rv = bcm_l3_egress_multipath_add(unit, object_id, interface);
    if (BCM_FAILURE(rv)) {
        printk("%s: Error adding interface to egress obj: %s\n", 
               ARG_CMD(a), bcm_errmsg(rv));
        return (CMD_FAIL);
    }
    return (CMD_OK);
}


/*
 * Function:
 *      _l3_cmd_egress_mpath_object_intf_remove
 * Description:
 * 	    Service routine used to remove an egress interface 
 *      from multipath  egress object. 
 * Parameters:
 *      unit      - (IN) Device number.
 *      a         - (IN) Command arguments. 
 * Returns: 
 *      CMD_XXX
 */
STATIC cmd_result_t
_l3_cmd_egress_mpath_object_intf_remove(int unit, args_t *a) 
{
    int               rv;
    parse_table_t     pt;     
    cmd_result_t      retCode; 
    int               object_id = -1;
    int               interface = 0;

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "EgrId", PQ_DFL | PQ_INT, 0, (void *)&object_id, 0);
    parse_table_add(&pt, "INtf", PQ_DFL | PQ_INT, 0, (void *)&interface, 0);

    if (!parseEndOk(a, &pt, &retCode))
        return retCode;

    rv = bcm_l3_egress_multipath_delete(unit, object_id, interface);
    if (BCM_FAILURE(rv)) {
        printk("%s: Error deleting interface from egress obj: %s\n", 
               ARG_CMD(a), bcm_errmsg(rv));
        return (CMD_FAIL);
    }
    return (CMD_OK);
}

/*
 * Function:
 *      _l3_cmd_prefix_map_add
 * Description:
 * 	    Service routine used to add prefix map.
 * Parameters:
 *      unit      - (IN) Device number.
 *      a         - (IN) Command arguments. 
 * Returns: 
 *      CMD_XXX
 */
STATIC cmd_result_t
_l3_cmd_prefix_map_add(int unit, args_t *a) 
{
    bcm_ip6_t         ip6_addr;
    cmd_result_t      retCode;
    parse_table_t     pt;
    int               rv;

    sal_memset(&ip6_addr, 0, sizeof(bcm_ip6_t));
    parse_table_init(unit, &pt);

    parse_table_add(&pt, "IP", PQ_DFL | PQ_IP6, 0, (void *)&ip6_addr, 0);
    if (!parseEndOk(a, &pt, &retCode))
        return retCode;

    rv = bcm_l3_ip6_prefix_map_add(unit, ip6_addr);
    if (BCM_FAILURE(rv)) {
        printk("%s: Error adding ip6 prefix map: %s\n", ARG_CMD(a),
               bcm_errmsg(rv));
        return (CMD_FAIL);
    }
    return (CMD_OK);
}

/*
 * Function:
 *      _l3_cmd_prefix_map_delete
 * Description:
 * 	    Service routine used to delete prefix map.
 * Parameters:
 *      unit      - (IN) Device number.
 *      a         - (IN) Command arguments. 
 * Returns: 
 *      CMD_XXX
 */
STATIC cmd_result_t
_l3_cmd_prefix_map_delete(int unit, args_t *a) 
{
    bcm_ip6_t         ip6_addr;
    cmd_result_t      retCode;
    parse_table_t     pt;
    int               rv;

    sal_memset(&ip6_addr, 0, sizeof(bcm_ip6_t));
    parse_table_init(unit, &pt);

    parse_table_add(&pt, "IP", PQ_DFL | PQ_IP6, 0, (void *)&ip6_addr, 0);
    if (!parseEndOk(a, &pt, &retCode))
        return retCode;

    rv = bcm_l3_ip6_prefix_map_delete(unit, ip6_addr);
    if (BCM_FAILURE(rv)) {
        printk("%s: Error adding ip6 prefix map: %s\n",
               ARG_CMD(a), bcm_errmsg(rv));
        return (CMD_FAIL);
    }
    return (CMD_OK);
}

/*
 * Function:
 *      _l3_cmd_prefix_map_delete_all
 * Description:
 * 	    Service routine used to flush all prefix maps.
 * Parameters:
 *      unit      - (IN) Device number.
 *      a         - (IN) Command arguments. 
 * Returns: 
 *      CMD_XXX
 */
STATIC cmd_result_t
_l3_cmd_prefix_map_delete_all(int unit, args_t *a) 
{
    int               rv;

    if ((rv = bcm_l3_ip6_prefix_map_delete_all(unit)) < 0) {
        printk("%s: Error flushing prefix map : %s\n",
               ARG_CMD(a), bcm_errmsg(rv));
        return (CMD_FAIL);
    } 
    return (CMD_OK);
}

#define BCM_DIAG_CMD_PREFIX_MAP_SIZE  (10)
/*
 * Function:
 *      _l3_cmd_prefix_map_show
 * Description:
 * 	    Service routine used display prefix maps.
 * Parameters:
 *      unit      - (IN) Device number.
 *      a         - (IN) Command arguments. 
 * Returns: 
 *      CMD_XXX
 */
STATIC cmd_result_t
_l3_cmd_prefix_map_show(int unit, args_t *a) 
{
    int        rv;
    int        idx;
    bcm_ip6_t  prefix_map[BCM_DIAG_CMD_PREFIX_MAP_SIZE];
    char       ip_str[IP6ADDR_STR_LEN + 3];
    int        intf_count = 0;     

    rv = bcm_l3_ip6_prefix_map_get(unit,  BCM_DIAG_CMD_PREFIX_MAP_SIZE, 
                                   prefix_map, &intf_count);
    if (BCM_FAILURE(rv)) {
        printk("%s: Error reading prefix map entry: %s\n",
               ARG_CMD(a), bcm_errmsg(rv));
        return (CMD_FAIL);
    } 

    if (intf_count > 0) {
        printk("Entry  IP6_ADDR\n");
    }

    for (idx = 0; idx < intf_count; idx++) {
        format_ip6addr(ip_str, prefix_map[idx]);
        printk ("%-5d %s\n", idx, ip_str);
    }

    return (CMD_OK);
}
#undef BCM_DIAG_CMD_PREFIX_MAP_SIZE 
#endif /* BCM_XGS3_SWITCH_SUPPORT */

/*
 * Function:
 *      _l3_cmd_route_add
 * Description:
 * 	    Service routine used to add route.
 * Parameters:
 *      unit      - (IN) Device number.
 *      a         - (IN) Command arguments. 
 *      v6        - (IN) IPv6 route indication.
 * Returns:
 *      CMD_XXX
 */
STATIC cmd_result_t
_l3_cmd_route_add(int unit, args_t *a, int v6) 
{
    bcm_l3_route_t  route_info;
    cmd_result_t    retCode;
    bcm_mac_t       mac;
    parse_table_t   pt;
    int             rv;
    int             interface = 0;
    bcm_ip_t        ip_addr = 0;
    bcm_ip_t        ip_mask = 0;
    int		        module = 0;
    int             trunk = -1;
    int             untag = 0;
    int             port = 0;
    int             hits = 0;
    int             vlan = 0;
#if defined(BCM_XGS_SWITCH_SUPPORT)
    int             ecmp = 0;
    int             local_route = 0;
#endif /* BCM_XGS_SWITCH_SUPPORT */
#if defined(BCM_XGS12_SWITCH_SUPPORT)
    bcm_ip_t	    gateway = 0;
#endif /* BCM_XGS12_SWITCH_SUPPORT */
#if defined(BCM_XGS3_SWITCH_SUPPORT)
    int             lookup_class = 0;
    int             tunnel_opt = 0;
    int             mpls_label = 0;
    int             replace = 0;
    int             rpe = 0;
    int             pri = 0;
    int             dst_discard = 0;
    int             vrf = 0;
    int             plen = 0;
    bcm_ip6_t       ip6_addr;
#endif

    bcm_l3_route_t_init(&route_info);
    sal_memset(mac, 0, sizeof(bcm_mac_t));
    parse_table_init(unit, &pt);

    parse_table_add(&pt, "Mac",     PQ_DFL | PQ_MAC,  0, (void *)mac, 0);
    parse_table_add(&pt, "Port",    PQ_DFL | PQ_PORT, 0, (void *)&port, 0);
    parse_table_add(&pt, "INtf",    PQ_DFL | PQ_INT,  0, (void *)&interface, 0);
    parse_table_add(&pt, "MOdule",  PQ_DFL | PQ_INT,  0, (void *)&module, 0);
    parse_table_add(&pt, "Trunk",   PQ_DFL | PQ_INT,  0, (void *)&trunk, 0);
    parse_table_add(&pt, "Untag",   PQ_DFL | PQ_BOOL, 0, (void *)&untag, 0);
    parse_table_add(&pt, "Hit",     PQ_DFL | PQ_BOOL, 0, (void *)&hits, 0);
#if defined(BCM_XGS3_SWITCH_SUPPORT)
    parse_table_add(&pt, "VRF",     PQ_DFL | PQ_INT,  0, (void *)&vrf, 0);
    parse_table_add(&pt, "DstDiscard", PQ_DFL | PQ_INT, 0, 
                    (void *)&dst_discard, 0);
    parse_table_add(&pt, "PRI",     PQ_DFL|PQ_INT,    0, (void *)&pri, 0);
    parse_table_add(&pt, "RPE",     PQ_DFL|PQ_BOOL,   0, (void *)&rpe, 0);
    parse_table_add(&pt, "Replace", PQ_DFL | PQ_BOOL, 0, (void *)&replace, 0);
    parse_table_add(&pt, "MplsLabel", PQ_DFL | PQ_INT, 0,
                    (void *)&mpls_label, 0);
    parse_table_add(&pt, "TunnelOpt", PQ_DFL | PQ_INT, 0,
                    (void *)&tunnel_opt, 0);
    parse_table_add(&pt, "LookupClass", PQ_DFL | PQ_INT, 0, (void *)&lookup_class, 0);
    parse_table_add(&pt, "VLAN",    PQ_DFL | PQ_INT,  0, (void *)&vlan, 0);
    if(v6) {
        sal_memset(ip6_addr, 0, sizeof(bcm_ip6_t));
        parse_table_add(&pt, "IP", PQ_DFL | PQ_IP6, 0, (void *)&ip6_addr, 0);
        parse_table_add(&pt, "MaskLen", PQ_DFL | PQ_INT, 0, (void *)&plen, 0);
    } else  
#endif /* BCM_XGS3_SWITCH_SUPPORT */
    {
        parse_table_add(&pt, "IP",   PQ_DFL | PQ_IP,   0, (void *)&ip_addr, 0);
        parse_table_add(&pt, "MaSk", PQ_DFL | PQ_IP,   0, (void *)&ip_mask, 0);
#if defined(BCM_XGS12_SWITCH_SUPPORT)
        parse_table_add(&pt, "Gateway", PQ_DFL | PQ_IP, 0, (void *)&gateway, 0);
#endif /* BCM_XGS12_SWITCH_SUPPORT */
    }

#if defined(BCM_XGS_SWITCH_SUPPORT)
    parse_table_add(&pt, "ECMP",    PQ_DFL | PQ_BOOL, 0, (void *)&ecmp, 0);
    parse_table_add(&pt, "Local",   PQ_DFL | PQ_BOOL, 0, (void *)&local_route, 0);
#endif /* BCM_XGS_SWITCH_SUPPORT */

    if (!parseEndOk(a, &pt, &retCode)) {
        return retCode;
    }

#if defined(BCM_XGS3_SWITCH_SUPPORT)
    route_info.l3a_vrf     = vrf;
    if (dst_discard) {
        route_info.l3a_flags |= BCM_L3_DST_DISCARD;
    }
    if (rpe) {
        route_info.l3a_flags |= BCM_L3_RPE;
    }
    if (replace) {
        route_info.l3a_flags |= BCM_L3_REPLACE;
    }
    route_info.l3a_pri = pri;

    if (lookup_class) {
        route_info.l3a_lookup_class= lookup_class;
    }

    if (mpls_label) { 
        route_info.l3a_mpls_label = mpls_label;
        route_info.l3a_flags |= BCM_L3_ROUTE_LABEL;
    } else if (tunnel_opt) {
        route_info.l3a_tunnel_option = tunnel_opt;
    }
    if (v6) {
        sal_memcpy(route_info.l3a_ip6_net, ip6_addr, BCM_IP6_ADDRLEN);
        bcm_ip6_mask_create(route_info.l3a_ip6_mask, plen);
        route_info.l3a_flags |= BCM_L3_IP6;
    } else 
#endif /* BCM_XGS3_SWITCH_SUPPORT */
    { 
        route_info.l3a_subnet  = ip_addr;
        route_info.l3a_ip_mask = ip_mask;
    }

#ifdef BCM_XGS12_SWITCH_SUPPORT
    route_info.l3a_nexthop_ip = gateway;
#endif /* BCM_XGS12_SWITCH_SUPPORT */

    route_info.l3a_intf = interface;
    route_info.l3a_vid = vlan;
    sal_memcpy(route_info.l3a_nexthop_mac, mac, sizeof(bcm_mac_t));

    if (BCM_GPORT_IS_SET(port)) {
        route_info.l3a_port_tgid = port;
    } else {
    if (trunk >= 0) {
        route_info.l3a_flags |= BCM_L3_TGID;
        route_info.l3a_port_tgid = trunk;
    } else {
        route_info.l3a_port_tgid = port;
    }
        route_info.l3a_modid = module;
    }

    if (untag) {
        route_info.l3a_flags |= BCM_L3_UNTAG;
    }

    if (hits) {
        route_info.l3a_flags |= BCM_L3_HIT;
    }

#ifdef BCM_XGS_SWITCH_SUPPORT
    if (ecmp) {
        route_info.l3a_flags |= BCM_L3_MULTIPATH;
    }

    if (local_route) {
        route_info.l3a_flags |= BCM_L3_DEFIP_LOCAL;
    }
#endif
    rv = bcm_l3_route_add(unit, &route_info);
    if (BCM_FAILURE(rv)) {
        printk("%s: Error adding route table: %s\n", 
               ARG_CMD(a), bcm_errmsg(rv));
        return (CMD_FAIL);
    }
    return (CMD_OK);
}

/*
 * Function:
 *      _l3_cmd_host_add
 * Description:
 * 	    Service routine used to add host entry.
 * Parameters:
 *      unit      - (IN) Device number.
 *      a         - (IN) Command arguments. 
 *      v6        - (IN) IPv6 route indication.
 * Returns:
 *      CMD_XXX
 */
STATIC cmd_result_t
_l3_cmd_host_add(int unit, args_t *a, int v6) 
{
    bcm_l3_host_t   host_info;
    cmd_result_t    retCode;
    bcm_mac_t       mac;
    parse_table_t   pt;
    int             rv;
    int             host_as_route = 0;
    int             interface = 0;
    bcm_ip_t        ip_addr = 0;
    int		        module = 0;
    int             trunk = -1;
    int             untag = 0;
    int             port = 0;
    int             hits = 0;
    int             replace = 0;
#ifdef BCM_XGS_SWITCH_SUPPORT
    int             l2tocpu = 0;
#endif /* BCM_XGS_SWITCH_SUPPORT */ 
#if defined(BCM_XGS3_SWITCH_SUPPORT)
    int             rpe = 0;
    int             pri = 0;
    int             vrf = 0;
    int             lookup_class = 0;
    bcm_ip6_t       ip6_addr;
#endif

    bcm_l3_host_t_init(&host_info);
    sal_memset(mac, 0, sizeof(bcm_mac_t));
    parse_table_init(unit, &pt);

    parse_table_add(&pt, "Mac", PQ_DFL | PQ_MAC, 0, (void *)mac, 0);
#if defined(BCM_XGS3_SWITCH_SUPPORT)
    parse_table_add(&pt, "VRF", PQ_DFL | PQ_INT, 0, (void *)&vrf, 0);
    parse_table_add(&pt, "PRI", PQ_DFL|PQ_INT, 0, (void *)&pri, 0);
    parse_table_add(&pt, "RPE", PQ_DFL|PQ_BOOL, 0, (void *)&rpe, 0);
    parse_table_add(&pt, "Group", PQ_DFL|PQ_INT, 0, (void *)&lookup_class, 0);
    if (v6) {
	    parse_table_add(&pt, "IP", PQ_DFL | PQ_IP6, 0, (void *)&ip6_addr, 0);
    } else 
#endif /* BCM_XGS3_SWITCH_SUPPORT */
    {
        parse_table_add(&pt, "IP", PQ_DFL | PQ_IP, 0, (void *)&ip_addr, 0);
    }
    parse_table_add(&pt, "Replace", PQ_DFL | PQ_BOOL, 0, (void *)&replace, 0);
    parse_table_add(&pt, "Port", PQ_DFL | PQ_PORT, 0, (void *)&port, 0);
    parse_table_add(&pt, "INtf", PQ_DFL | PQ_INT, 0, (void *)&interface, 0);
    parse_table_add(&pt, "Hit", PQ_DFL | PQ_BOOL, 0, (void *)&hits, 0);
    parse_table_add(&pt, "MOdule", PQ_DFL | PQ_INT, 0, (void *)&module, 0);
    parse_table_add(&pt, "Trunk", PQ_DFL | PQ_INT, 0, (void *)&trunk, 0);
    parse_table_add(&pt, "Untag", PQ_DFL | PQ_BOOL, 0, (void *)&untag, 0);
    parse_table_add(&pt, "HOST_AS_ROUTE", PQ_DFL | PQ_BOOL, 0, 
                    (void *)&host_as_route, 0);
#ifdef BCM_XGS_SWITCH_SUPPORT
    parse_table_add(&pt, "L2tocpu", PQ_DFL | PQ_BOOL, 0, (void *)&l2tocpu, 0);
#endif
    if (!parseEndOk(a, &pt, &retCode))
        return retCode;

#if defined(BCM_XGS3_SWITCH_SUPPORT)
    host_info.l3a_vrf     = vrf;
    host_info.l3a_pri     = pri;
    host_info.l3a_lookup_class = lookup_class;

    if (rpe) {
        host_info.l3a_flags |= BCM_L3_RPE;
    }

    if (v6) {
        sal_memcpy(host_info.l3a_ip6_addr, ip6_addr, BCM_IP6_ADDRLEN);
        host_info.l3a_flags |= BCM_L3_IP6;
    } else 
#endif
    {
        host_info.l3a_ip_addr = ip_addr;
    } 

    if (host_as_route) {
        host_info.l3a_flags |= BCM_L3_HOST_AS_ROUTE;
    } 
    host_info.l3a_intf = interface;
    sal_memcpy(host_info.l3a_nexthop_mac, mac, sizeof(mac));

    if (BCM_GPORT_IS_SET(port)) {
        host_info.l3a_port_tgid = port;
    } else {
    host_info.l3a_modid = module;
    if (trunk >= 0) {
        host_info.l3a_flags |= BCM_L3_TGID;
        host_info.l3a_port_tgid = trunk;
    } else {
        host_info.l3a_port_tgid = port;
    }
    }

    if (hits) {
        host_info.l3a_flags |= BCM_L3_HIT;
    }
    if (untag) {
        host_info.l3a_flags |= BCM_L3_UNTAG;
    }

    if (replace) {
        host_info.l3a_flags |= BCM_L3_REPLACE;
    }

#ifdef BCM_XGS_SWITCH_SUPPORT
    if (l2tocpu) {
        host_info.l3a_flags |= BCM_L3_L2TOCPU;
    }
#endif
    rv = bcm_l3_host_add(unit, &host_info);
    if (BCM_FAILURE(rv)) {
        printk("%s: Error creating entry in L3 host table: %s\n",
               ARG_CMD(a), bcm_errmsg(rv));
        return (CMD_FAIL);
    }
    return (CMD_OK);
} 


cmd_result_t
if_l3(int unit, args_t *a)
{
    char           *table, *subcmd_s;
    int             subcmd = 0;
    parse_table_t   pt;
    cmd_result_t    retCode;
    bcm_mac_t      mac;
    int             id = 0;
    int             r, first_entry, last_entry, num_entries;
    int             free_l3, free_defip, free_lpm_blk;
    bcm_l3_info_t   l3_hw_status;
    char            ip_str[IP6ADDR_STR_LEN + 3];
    bcm_ip_t       ip_addr = 0, mask = 0;
#ifdef BCM_XGS_SWITCH_SUPPORT
    int             interface = 0, port = 0;
    int		    module = 0;
    bcm_vlan_t      vlan = 0;
    bcm_ip_t	    gateway = 0;
    int             sip_key = 2, hash_select = XGS_HASH_COUNT;
    int             start = 0, end = 0;
    uint8           key[SOC_MAX_MEM_WORDS * 4];
    int             ecmp = 0, max_ecmp = 0;
#endif
    uint32          flags;
    bcm_l3_route_t  route_info;
    bcm_l3_host_t   ip_info;
#if defined(BCM_XGS3_SWITCH_SUPPORT)
    bcm_ip6_t       ip6_addr, sip6_addr;
    int             vrf = 0;
#endif
#if defined(BCM_FIREBOLT_SUPPORT)
    int i;
#endif  /* BCM_FIREBOLT_SUPPORT */

    memset(mac, 0, sizeof(bcm_mac_t));

    /* Check valid device to operation on ... */
    if (!sh_check_attached(ARG_CMD(a), unit)) {
	return CMD_FAIL;
    }

    if ((table = ARG_GET(a)) == NULL) {
	return CMD_USAGE;
    }

    /* L3 init */
    if (sal_strcasecmp(table, "init") == 0) {
	if ((r = bcm_l3_init(unit)) < 0) {
	    printk("%s: error initializing: %s\n", ARG_CMD(a), bcm_errmsg(r));
	}
	return CMD_OK;
    }

    /* L3 detach */
    if (sal_strcasecmp(table, "detach") == 0) {
	if ((r = bcm_l3_cleanup(unit)) < 0) {
	    printk("%s: error detaching: %s\n", ARG_CMD(a), bcm_errmsg(r));
	}
	return CMD_OK;
    }
    if (sal_strcasecmp(table, "defipinfo") == 0) {
#if defined(BCM_KATANA_SUPPORT)
        if (SOC_IS_KATANA(unit)) {
            bcm_l3_info_t l3_info;
            bcm_l3_info_t_init(&l3_info);
            if ((r = bcm_l3_info(unit, &l3_info)) < 0) {
                printk("%s: Error L3 info: %s\n", ARG_CMD(a), bcm_errmsg(r));
                return CMD_FAIL;
            }
            
            bcm_xgs3_l3_info_dump(unit);
        }
#endif
        return CMD_OK;
    }

    if ((subcmd_s = ARG_GET(a)) == NULL) {
	return CMD_USAGE;
    }
    if (sal_strcasecmp(subcmd_s, "add") == 0) {
	subcmd = L3_ADD;
    }
    if (sal_strcasecmp(subcmd_s, "update") == 0) {
	subcmd = L3_UPDATE;
    }
    if (sal_strcasecmp(subcmd_s, "destroy") == 0 ||
        sal_strcasecmp(subcmd_s, "delete") == 0) {
	subcmd = L3_DESTROY;
    }
    if (sal_strcasecmp(subcmd_s, "set") == 0) {
	subcmd = L3_SET;
    }
    if (sal_strcasecmp(subcmd_s, "clear") == 0) {
	subcmd = L3_CLEAR;
    }
    if (sal_strcasecmp(subcmd_s, "show") == 0) {
	subcmd = L3_SHOW;
    }
    if (sal_strcasecmp(subcmd_s, "age") == 0) {
	subcmd = L3_AGE;
    }
    if (sal_strcasecmp(subcmd_s, "test") == 0) {
	subcmd = L3_TEST;
    }
    if (sal_strcasecmp(subcmd_s, "check") == 0) {
	subcmd = L3_CHECK;
    }
#ifdef BCM_XGS_SWITCH_SUPPORT
    if (sal_strcasecmp(subcmd_s, "conflict") == 0) {
	subcmd = L3_CONFLICT;
    }
    if (sal_strcasecmp(subcmd_s, "hash") == 0) {
	subcmd = L3_HASH;
    }
    if (sal_strcasecmp(subcmd_s, "ip6hash") == 0) {
	subcmd = L3_IP6HASH;
    }
    if (sal_strcasecmp(subcmd_s, "sanity") == 0) {
	subcmd = L3_SANITY;
    }
    if (sal_strcasecmp(subcmd_s, "untag") == 0) {
	subcmd = L3_UNTAG;
    }
    if (sal_strcasecmp(subcmd_s, "ecmp") == 0) {
	subcmd = L3_ECMP;
    }
    if (sal_strcasecmp(subcmd_s, "ecmphash") == 0) {
	subcmd = L3_ECMP_HASH;
    }
#endif
    if (sal_strcasecmp(subcmd_s, "insert") == 0) {
	subcmd = L3_INSERT;
    }
    if (sal_strcasecmp(subcmd_s, "remove") == 0) {
	subcmd = L3_REMOVE;
    }
    if (sal_strcasecmp(subcmd_s, "find") == 0) {
	subcmd = L3_FIND;
    }
    if (!subcmd) {
	return CMD_USAGE;
    }

    if (sal_strcasecmp(table, "intf") == 0) {
        switch (subcmd) {
            /* l3 intf add */
          case L3_ADD:
              return _l3_cmd_egress_intf_create(unit, a); 
              /* l3 intf destroy */
          case L3_DESTROY:
              return  _l3_cmd_egress_intf_delete(unit, a);
              /* l3 intf clear */
          case L3_CLEAR:
              return _l3_cmd_egress_intf_delete_all(unit, a); 
              /* l3 intf show */
          case L3_SHOW:
              return _l3_cmd_egress_intf_show(unit, a);
          default:
              return (CMD_USAGE);
        }
    }

    if (sal_strcasecmp(table, "egress") == 0) {
#if defined(BCM_XGS3_SWITCH_SUPPORT)
        switch (subcmd) {
            /* l3 egress add/update */
          case L3_ADD:
          case L3_UPDATE:
              return _l3_cmd_egress_object_create(unit, a);
              /* l3 egress destroy */
          case L3_DESTROY:
              return _l3_cmd_egress_object_destroy(unit, a);
              /* l3 egress show */
          case L3_SHOW:
              return _l3_cmd_egress_object_show(unit, a); 
              /* l3 egress find */
          case L3_FIND:
              return _l3_cmd_egress_object_find(unit, a); 
          default:
              return (CMD_USAGE);
        }
    /*    coverity[unreachable]    */
#endif /* BCM_XGS3_SWITCH_SUPPORT */
    /*    coverity[unreachable]    */
        printk("Unsupported command. \n");
        return (CMD_OK);
    }
    if (sal_strcasecmp(table, "prefix_map") == 0) {
#if defined(BCM_XGS3_SWITCH_SUPPORT)
        switch (subcmd) {
            /* l3 prefix_map add */
          case L3_ADD:
              return _l3_cmd_prefix_map_add(unit, a);
              /* l3 prefix_map delete */
          case L3_DESTROY:
              return _l3_cmd_prefix_map_delete(unit, a);
              /* l3 prefix_map show */
          case L3_SHOW:
              return _l3_cmd_prefix_map_show(unit, a);
              /* l3 egress clear */
          case L3_CLEAR: 
              return _l3_cmd_prefix_map_delete_all(unit, a);
          default:
              printk("Unsupported command. \n");
              return CMD_OK;
    /*    coverity[unreachable]    */
        }
    /** coverity[unreachable]    **/
        return CMD_OK;
    /*    coverity[unreachable]    */
#endif /* BCM_XGS3_SWITCH_SUPPORT */
    /** coverity[unreachable]    **/
        printk("Unsupported command. \n");
        return CMD_OK;
    }
    if (sal_strcasecmp(table, "multipath") == 0) {
#if defined(BCM_XGS3_SWITCH_SUPPORT)
        switch (subcmd) {
            /* l3 multipath add */
          case L3_ADD:
          case L3_UPDATE:
              return _l3_cmd_egress_mpath_object_create(unit, a); 
              /* l3 multipath destroy */
          case L3_DESTROY:
              return _l3_cmd_egress_mpath_object_destroy(unit, a); 
              /* l3 multipath show */
          case L3_SHOW:
              return _l3_cmd_egress_mpath_object_show(unit, a); 
          case L3_INSERT:
              return _l3_cmd_egress_mpath_object_intf_insert(unit, a);
          case L3_REMOVE:
              return _l3_cmd_egress_mpath_object_intf_remove(unit, a);
          case L3_FIND:
    /** coverity[unreachable]    **/
              return _l3_cmd_egress_mpath_object_lookup(unit, a);
    /** coverity[unreachable]    **/
          default:
              printk("Unsupported command. \n");
    /** coverity[unreachable]    **/
              return CMD_OK;
    /** coverity[unreachable]    **/
         }
    /** coverity[unreachable]    **/
         return CMD_OK;
#endif /* BCM_XGS3_SWITCH_SUPPORT */
    /** coverity[unreachable]    **/
         printk("Unsupported command. \n");
         return CMD_OK;
    }
    if (sal_strcasecmp(table, "l3table") == 0) {
        switch (subcmd) {

            /* l3 l3table add */
          case L3_ADD:
              return _l3_cmd_host_add(unit, a, FALSE);
              /* l3 l3table destroy */
          case L3_DESTROY:
              parse_table_init(unit, &pt);
#if defined(BCM_XGS3_SWITCH_SUPPORT)
              parse_table_add(&pt, "VRF", PQ_DFL | PQ_INT, 0, (void *)&vrf, 0);
#endif
              parse_table_add(&pt, "IP", PQ_DFL | PQ_IP, 0, (void *)&ip_addr, 0);
              if (!parseEndOk(a, &pt, &retCode))
                  return retCode;

              bcm_l3_host_t_init(&ip_info);
#if defined(BCM_XGS3_SWITCH_SUPPORT)
              ip_info.l3a_vrf     = vrf;
#endif
              ip_info.l3a_ip_addr = ip_addr;

              if ((r = bcm_l3_host_delete(unit, &ip_info)) < 0) {
                  format_ipaddr(ip_str, ip_addr);
                  printk("%s: Error destroy %s from L3 "
                         "table: %s\n", ARG_CMD(a), ip_str, bcm_errmsg(r));
                  return CMD_FAIL;
              }
              break;

              /* l3 l3table clear */
          case L3_CLEAR:
              bcm_l3_ip_delete_all(unit);
              break;

              /* l3 l3table show */
          case L3_SHOW:
              if ((r = bcm_l3_info(unit, &l3_hw_status)) < 0) {
                  printk("%s: Error L3 accessing: %s\n", ARG_CMD(a), bcm_errmsg(r));
                  return CMD_FAIL;
              }
              free_l3 = l3_hw_status.l3info_max_host -
                  l3_hw_status.l3info_occupied_host;
              last_entry = l3_hw_status.l3info_max_host;
              if (ARG_CNT(a)) {
                  if (!isint(ARG_CUR(a))) {
                      printk("%s: Invalid number: %s\n", ARG_CMD(a), ARG_CUR(a));
                      return (CMD_FAIL);
                  }
                  first_entry = parse_integer(ARG_GET(a));
                  if (first_entry > last_entry) {
                      printk("%s: Error: Illegal entry number (0 to %d)\n",
                             ARG_CMD(a), last_entry);
                      return (CMD_FAIL);
                  }
                  last_entry = first_entry;
              } else {
                  first_entry = 0;
              }

              printk("Unit %d, free L3 table entries: %d\n", unit, free_l3);
              printk("Entry VRF IP address       Mac Address           "
                     "INTF MOD PORT    CLASS HIT\n");
              bcm_l3_host_traverse(unit, 0, first_entry, last_entry,
                                   _l3_cmd_host_print, NULL);
              return CMD_OK;

              /* l3 l3table age */
          case L3_AGE:
              flags = BCM_L3_S_HIT;

              bcm_l3_host_age(unit, flags, _l3_cmd_host_age_callback, NULL);
              return CMD_OK;

#ifdef BCM_XGS_SWITCH_SUPPORT
          case L3_HASH:

              parse_table_init(unit, &pt);
#if defined(BCM_EASYRIDER_SUPPORT) || defined(BCM_TRX_SUPPORT)
              if (SOC_IS_EASYRIDER(unit) || SOC_IS_TRX(unit)) {
                  parse_table_add(&pt, "VRF", PQ_DFL|PQ_INT, 0,
                                  (void *)&vrf, 0);
              }
#endif
              parse_table_add(&pt, "IP", PQ_DFL | PQ_IP, 0, (void *)&ip_addr, 0);
              parse_table_add(&pt, "SrcIP", PQ_DFL | PQ_IP, 0,
                              (void *)&gateway, 0);
              parse_table_add(&pt, "VID", PQ_DFL | PQ_INT, 0,
                              (void *)&id, 0);
              parse_table_add(&pt, "Hash", PQ_DFL | PQ_INT, 0,
                              (void *)&hash_select, 0);
              parse_table_add(&pt, "SIPKey", PQ_DFL | PQ_INT, 0,
                              (void *)&sip_key, 0);
              if (!parseEndOk(a, &pt, &retCode))
                  return retCode;
              vlan = id;

#ifdef BCM_XGS12_SWITCH_SUPPORT
              if (SOC_IS_XGS12_SWITCH(unit)) {
                  if (hash_select == XGS_HASH_COUNT) {
                      /* Get the hash selection from hardware */
                      uint32          regval;
                      int             rv;

                      if ((rv = READ_HASH_CONTROLr(unit, &regval)) < 0) {
                          printk("%s: ERROR: %s\n", ARG_CMD(a), soc_errmsg(rv));
                          return CMD_FAIL;
                      }
                      hash_select = soc_reg_field_get(unit, HASH_CONTROLr, regval,
                                                      HASH_SELECTf);
                  }

                  if (SOC_L3X_IP_MULTICAST(ip_addr)) {
                      int             hash_bucket;

                      if ((r = _soc_mem_cmp_l3x_sync(unit)) < 0) {
                          printk("%s: ERROR: %s\n", ARG_CMD(a), soc_errmsg(r));
                          return CMD_FAIL;
                      }

                      if (sip_key == 2) {
                          /* Get the SIP Key selection from hardware */
                          sip_key = (SOC_CONTROL(unit)->hash_key_config &
                                     (L3X_IPMC_SIP|L3X_IPMC_SIP0)) == L3X_IPMC_SIP0;
                      }

                      if (soc_feature(unit, soc_feature_l3_sgv)) { /* (S,G,V) */
                          /*
                           * (S,G,V) IPMC key is (DIP + SIP + VID) or
                           * (DIP + SIP + 0)
                           */
                          if (SOC_CONTROL(unit)->hash_key_config & L3X_IPMC_VLAN) {
                              soc_draco_l3x_param_to_key(ip_addr, gateway, vlan,
                                                         key);
                          } else {
                              soc_draco_l3x_param_to_key(ip_addr,
                                                         (sip_key ? 0 : gateway),
                                                         0, key);
                          }
                      } else {
                          soc_draco_l3x_param_to_key(ip_addr,
                                                     (sip_key ? 0 : gateway),
                                                     0, key);
                      }
                      hash_bucket = soc_draco_l3_hash(unit, hash_select,
                                                      TRUE, key);

                      printk("Hash(IPMC)[%d] of key "
                             "0x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x "
                             "is bucket 0x%03x (%d)\n", hash_select,
                             key[9], key[8], key[7], key[6], key[5],
                             key[4], key[3], key[2], key[1], key[0],
                             hash_bucket, hash_bucket);
                  } else {
                      int             hash_bucket;
                      soc_draco_l3x_param_to_key(ip_addr, 0, 0, key);
                      hash_bucket = soc_draco_l3_hash(unit, hash_select, FALSE,
                                                      key);

                      printk("Hash(UC)[%d] of key "
                             "0x%02x%02x%02x%02x "
                             "is bucket 0x%03x (%d)\n", hash_select,
                             key[3], key[2], key[1], key[0],
                             hash_bucket, hash_bucket);
                  }
              }
#endif /* end of BCM_XGS12_SWITCH_SUPPORT */
#ifdef BCM_TRX_SUPPORT
              if (SOC_MEM_FIELD_VALID(unit, L3_ENTRY_ONLYm, KEY_TYPEf)) {

                  int mcast_flag = 0;
                  int hash_bucket;
                  int key_nbits;
                  l3_entry_ipv4_multicast_entry_t  l3x_ip4_mentry;
                  l3_entry_ipv4_unicast_entry_t    l3x_ip4_uentry;
                  soc_field_t key_type_field;


                  if (hash_select == FB_HASH_COUNT) {
                      /* Get the hash selection from hardware */
                      uint32          regval;
                      int             rv;
                      if ((rv = READ_HASH_CONTROLr(unit, &regval)) < 0) {
                          printk("%s: ERROR: %s\n", ARG_CMD(a), soc_errmsg(rv));
                          return CMD_FAIL;
                      }
                      hash_select = soc_reg_field_get(unit, HASH_CONTROLr, regval,
                                                      L3_HASH_SELECTf);
                  }

                  if (SOC_L3X_IP_MULTICAST(ip_addr)) {
                      /* Given IP address is multicast IP address */
                      mcast_flag = 1;
                      key_type_field = (SOC_MEM_FIELD_VALID(unit, 
                                        L3_ENTRY_IPV4_MULTICASTm, KEY_TYPEf))? \
                                        KEY_TYPEf : KEY_TYPE_0f;
                      sal_memset(&l3x_ip4_mentry, 0,
                                 sizeof(l3_entry_ipv4_multicast_entry_t));

                           
                      soc_mem_field32_set(unit, L3_ENTRY_IPV4_MULTICASTm,
                                          (uint32 *) &l3x_ip4_mentry, 
                                          key_type_field,
                                          TR_L3_HASH_KEY_TYPE_V4MC);

                      soc_mem_field32_set(unit, L3_ENTRY_IPV4_MULTICASTm,
                                          (uint32 *) &l3x_ip4_mentry,
                                          GROUP_IP_ADDRf, ip_addr);

                      soc_mem_field32_set(unit, L3_ENTRY_IPV4_MULTICASTm,
                                          (uint32 *) &l3x_ip4_mentry,
                                          SOURCE_IP_ADDRf, gateway);

                      soc_mem_field32_set(unit, L3_ENTRY_IPV4_MULTICASTm,
                                          (uint32 *) &l3x_ip4_mentry, VLAN_IDf,
                                          vlan);

                      soc_mem_field32_set(unit, L3_ENTRY_IPV4_MULTICASTm,
                                          (uint32 *) &l3x_ip4_mentry, VRF_IDf, vrf);

                      key_nbits = soc_fb_l3x_base_entry_to_key(unit,
                                                               (uint32 *) &l3x_ip4_mentry, key);
                  } else {
                      /* Given IP address is Unicast IP address */
                      sal_memset(&l3x_ip4_uentry, 0,
                                 sizeof(l3_entry_ipv4_unicast_entry_t));

                      soc_mem_field32_set(unit, L3_ENTRY_IPV4_UNICASTm,
                                          (uint32 *) &l3x_ip4_mentry, KEY_TYPEf, 
                                          TR_L3_HASH_KEY_TYPE_V4UC);

                      soc_mem_field32_set(unit, L3_ENTRY_IPV4_UNICASTm,
                                          (uint32 *) &l3x_ip4_uentry,
                                          IP_ADDRf, ip_addr);

                      soc_mem_field32_set(unit, L3_ENTRY_IPV4_UNICASTm,
                                          (uint32 *) &l3x_ip4_uentry, VRF_IDf, vrf);

                      key_nbits = soc_fb_l3x_base_entry_to_key(unit,
                                                               (uint32 *) &l3x_ip4_uentry,
                                                               key);
                  }

                  hash_bucket = soc_fb_l3_hash(unit, hash_select, key_nbits, key);

                  if (mcast_flag) {
                      printk("Hash(IPMC)[%d] of key "
                             "0x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x "
                             "is bucket 0x%03x (%d)\n", hash_select,
                             key[9], key[8], key[7], key[6], key[5],
                             key[4], key[3], key[2], key[1], key[0],
                             hash_bucket, hash_bucket);
                  } else {
                      printk("Hash(UC)[%d] of key "
                             "0x%02x%02x%02x%02x "
                             "is bucket 0x%03x (%d)\n", hash_select,
                             key[3], key[2], key[1], key[0],
                             hash_bucket, hash_bucket);
                  }
                  break;
              }
#endif /* end of BCM_TRX_SUPPORT*/
#ifdef BCM_FIREBOLT_SUPPORT
              if (SOC_IS_FBX(unit)) {

                  int mcast_flag = 0;
                  int hash_bucket;
                  int key_nbits;
                  l3_entry_ipv4_multicast_entry_t  l3x_ip4_mentry;
                  l3_entry_ipv4_unicast_entry_t    l3x_ip4_uentry;

                  if (hash_select == FB_HASH_COUNT) {
                      /* Get the hash selection from hardware */
                      uint32          regval;
                      int             rv;
                      if ((rv = READ_HASH_CONTROLr(unit, &regval)) < 0) {
                          printk("%s: ERROR: %s\n", ARG_CMD(a), soc_errmsg(rv));
                          return CMD_FAIL;
                      }
                      hash_select = soc_reg_field_get(unit, HASH_CONTROLr, regval,
                                                      L3_HASH_SELECTf);
                  }

                  if (SOC_L3X_IP_MULTICAST(ip_addr)) {
                      /* Given IP address is multicast IP address */
                      mcast_flag = 1;
                      sal_memset(&l3x_ip4_mentry, 0,
                                 sizeof(l3_entry_ipv4_multicast_entry_t));

                      soc_mem_field32_set(unit, L3_ENTRY_IPV4_MULTICASTm,
                                          (uint32 *) &l3x_ip4_mentry, IPMCf, 1);

                      soc_mem_field32_set(unit, L3_ENTRY_IPV4_MULTICASTm,
                                          (uint32 *) &l3x_ip4_mentry,
                                          GROUP_IP_ADDRf, ip_addr);

                      soc_mem_field32_set(unit, L3_ENTRY_IPV4_MULTICASTm,
                                          (uint32 *) &l3x_ip4_mentry,
                                          SOURCE_IP_ADDRf, gateway);

                      soc_mem_field32_set(unit, L3_ENTRY_IPV4_MULTICASTm,
                                          (uint32 *) &l3x_ip4_mentry, VLAN_IDf,
                                          vlan);

                      key_nbits = soc_fb_l3x_base_entry_to_key(unit,
                                                               (uint32 *) &l3x_ip4_mentry, key);
                  } else {
                      /* Given IP address is Unicast IP address */
                      sal_memset(&l3x_ip4_uentry, 0,
                                 sizeof(l3_entry_ipv4_unicast_entry_t));

                      if (SOC_L3X_IP_MULTICAST(ip_addr)) {
                          soc_mem_field32_set(unit, L3_ENTRY_IPV4_UNICASTm,
                                              (uint32 *) &l3x_ip4_uentry, IPMCf, 1);
                      }

                      soc_mem_field32_set(unit, L3_ENTRY_IPV4_UNICASTm,
                                          (uint32 *) &l3x_ip4_uentry,
                                          IP_ADDRf, ip_addr);

                      key_nbits = soc_fb_l3x_base_entry_to_key(unit,
                                                               (uint32 *) &l3x_ip4_uentry,
                                                               key);
                  }

                  hash_bucket = soc_fb_l3_hash(unit, hash_select, key_nbits, key);

                  if (mcast_flag) {
                      printk("Hash(IPMC)[%d] of key "
                             "0x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x "
                             "is bucket 0x%03x (%d)\n", hash_select,
                             key[9], key[8], key[7], key[6], key[5],
                             key[4], key[3], key[2], key[1], key[0],
                             hash_bucket, hash_bucket);
                  } else {
                      printk("Hash(UC)[%d] of key "
                             "0x%02x%02x%02x%02x "
                             "is bucket 0x%03x (%d)\n", hash_select,
                             key[3], key[2], key[1], key[0],
                             hash_bucket, hash_bucket);
                  }
              }
#endif /* end of BCM_FIREBOLT_SUPPORT */

#ifdef BCM_EASYRIDER_SUPPORT
              if (SOC_IS_EASYRIDER(unit)) {

                  int                 hash_bucket;
                  l3_entry_v4_entry_t l3x_ip4_entry;

                  if (hash_select == FB_HASH_COUNT) {
                      /* Get the hash selection from hardware */
                      uint32          regval;
                      int             rv;
                      if ((rv = READ_HASH_CONTROLr(unit, &regval)) < 0) {
                          printk("%s: ERROR: %s\n", ARG_CMD(a), soc_errmsg(rv));
                          return CMD_FAIL;
                      }
                      hash_select = soc_reg_field_get(unit, HASH_CONTROLr, regval,
                                                      L3_HASH_SELECTf);
                  }

                  sal_memset(&l3x_ip4_entry, 0, sizeof(l3_entry_v4_entry_t));

                  soc_mem_field32_set(unit, L3_ENTRY_V4m,
                                      (uint32 *) &l3x_ip4_entry, IP_ADDRf, ip_addr);

                  soc_mem_field32_set(unit, L3_ENTRY_V4m,
                                      (uint32 *) &l3x_ip4_entry, VRFf, vrf);

                  soc_er_l3v4_base_entry_to_key(unit,
                                                (uint32 *) &l3x_ip4_entry, key);

                  hash_bucket = soc_er_l3v4_hash(unit, hash_select, key);

                  printk("Hash(UC)[%d] of key "
                         "0x%02x%02x%02x%02x%02x%02x "
                         "is bucket 0x%03x (%d)\n", hash_select,
                         key[5], key[4], key[3], key[2], key[1], key[0],
                         hash_bucket, hash_bucket);
              }
#endif /* end of BCM_EASYRIDER_SUPPORT */
              break;

              /* l3 l3table ip6hash */
          case L3_IP6HASH:
              if (!SOC_IS_FBX(unit) && !SOC_IS_EASYRIDER(unit)) {
                  printk("Command valid only for FB and ER switches\n");
                  return CMD_FAIL;
              }
#ifdef BCM_XGS3_SWITCH_SUPPORT
              parse_table_init(unit, &pt);
#if defined(BCM_EASYRIDER_SUPPORT) || defined(BCM_TRX_SUPPORT)
              if (SOC_IS_EASYRIDER(unit) || SOC_IS_TRX(unit)) {
                  parse_table_add(&pt, "VRF", PQ_DFL|PQ_INT, 0,
                                  (void *)&vrf, 0);
              }
#endif
              parse_table_add(&pt, "IP", PQ_DFL | PQ_IP6, 0,
                              (void *)&ip6_addr, 0);
              parse_table_add(&pt, "SrcIP", PQ_DFL | PQ_IP6, 0,
                              (void *)&sip6_addr, 0);
              parse_table_add(&pt, "VID", PQ_DFL | PQ_INT, 0,
                              (void *)&id, 0);
              parse_table_add(&pt, "Hash", PQ_DFL | PQ_INT, 0,
                              (void *)&hash_select, 0);
              parse_table_add(&pt, "SIPKey", PQ_DFL | PQ_INT, 0,
                              (void *)&sip_key, 0);
              if (!parseEndOk(a, &pt, &retCode)) {
                  return retCode;
              }
              vlan = id;

              if (hash_select == FB_HASH_COUNT) {
                  /* Get the hash selection from hardware */
                  uint32 regval;
                  int rv;

                  if ((rv = READ_HASH_CONTROLr(unit, &regval)) < 0) {
                      printk("%s: ERROR: %s\n", ARG_CMD(a), soc_errmsg(rv));
                      return CMD_FAIL;
                  }
                  hash_select = soc_reg_field_get(unit, HASH_CONTROLr, regval,
                                                  L3_HASH_SELECTf);
              }
#endif
#ifdef BCM_TRX_SUPPORT
              if (SOC_MEM_FIELD_VALID(unit, L3_ENTRY_ONLYm, KEY_TYPEf)) {
                  int key_nbits;
                  int hash_bucket = 0;
                  int mcast_flag =0;
                  int loop;
                  l3_entry_ipv6_unicast_entry_t    l3x_ip6_uentry;
                  l3_entry_ipv6_multicast_entry_t  l3x_ip6_mentry;
                  soc_field_t vrf_field, vlan_field;

                  if (BCM_IP6_MULTICAST(ip6_addr)) {
                      soc_field_t typef[] = {KEY_TYPE_0f, KEY_TYPE_1f, 
                                             KEY_TYPE_2f, KEY_TYPE_3f};
                      mcast_flag  = 1;

                      sal_memset(&l3x_ip6_mentry, 0,
                                 sizeof(l3_entry_ipv6_multicast_entry_t));

                      soc_mem_ip6_addr_set(unit, L3_ENTRY_IPV6_MULTICASTm,
                                           &l3x_ip6_mentry,
                                           GROUP_IP_ADDR_LWR_64f,
                                           (uint8*) &ip6_addr,
                                           SOC_MEM_IP6_LOWER_ONLY);

                      ip6_addr[0] = 0x0;
                      soc_mem_ip6_addr_set(unit, L3_ENTRY_IPV6_MULTICASTm,
                                           &l3x_ip6_mentry, GROUP_IP_ADDR_UPR_56f,
                                           (uint8*) &ip6_addr,
                                           SOC_MEM_IP6_UPPER_ONLY);

                      soc_mem_ip6_addr_set(unit, L3_ENTRY_IPV6_MULTICASTm,
                                           &l3x_ip6_mentry, SOURCE_IP_ADDR_LWR_64f,
                                           (uint8*) &sip6_addr,
                                           SOC_MEM_IP6_LOWER_ONLY);

                      soc_mem_ip6_addr_set(unit, L3_ENTRY_IPV6_MULTICASTm,
                                           &l3x_ip6_mentry, SOURCE_IP_ADDR_UPR_64f,
                                           (uint8*) &sip6_addr,
                                           SOC_MEM_IP6_UPPER_ONLY);

                       vlan_field = (SOC_MEM_FIELD_VALID(unit, 
                                        L3_ENTRY_IPV6_MULTICASTm, VLAN_IDf))? \
                                        VLAN_IDf: VLAN_ID_0f;

                       soc_L3_ENTRY_IPV6_MULTICASTm_field32_set(unit,
                                           &l3x_ip6_mentry, vlan_field, vlan);

                       vrf_field = (SOC_MEM_FIELD_VALID(unit, 
                                        L3_ENTRY_IPV6_MULTICASTm, VRF_IDf))? \
                                        VRF_IDf: VRF_ID_0f;

                       soc_L3_ENTRY_IPV6_MULTICASTm_field32_set(unit,
                                           &l3x_ip6_mentry, vrf_field, vrf);

                      for (i=0; i < 4; i++) {
                          soc_L3_ENTRY_IPV6_MULTICASTm_field32_set(unit,
                                              &l3x_ip6_mentry, typef[i], 
                                              TR_L3_HASH_KEY_TYPE_V6MC);
                      }

                      key_nbits = soc_fb_l3x_base_entry_to_key(unit,
                                                               (uint32 *) &l3x_ip6_mentry, key);
                  } else {
                      sal_memset(&l3x_ip6_uentry, 0,
                                 sizeof(l3_entry_ipv6_unicast_entry_t));

                      soc_mem_ip6_addr_set(unit, L3_ENTRY_IPV6_UNICASTm,
                                           &l3x_ip6_uentry, IP_ADDR_LWR_64f,
                                           ip6_addr, SOC_MEM_IP6_LOWER_ONLY);

                      soc_mem_ip6_addr_set(unit, L3_ENTRY_IPV6_UNICASTm,
                                           &l3x_ip6_uentry, IP_ADDR_UPR_64f,
                                           ip6_addr, SOC_MEM_IP6_UPPER_ONLY);

                      soc_L3_ENTRY_IPV6_UNICASTm_field32_set(unit, &l3x_ip6_uentry,
                                                             KEY_TYPE_1f, 
                                                             TR_L3_HASH_KEY_TYPE_V6UC);

                      soc_L3_ENTRY_IPV6_UNICASTm_field32_set(unit, &l3x_ip6_uentry,
                                                             KEY_TYPE_0f, 
                                                             TR_L3_HASH_KEY_TYPE_V6UC);

                       vrf_field = (SOC_MEM_FIELD_VALID(unit, 
                                        L3_ENTRY_IPV6_MULTICASTm, VRF_IDf))? \
                                        VRF_IDf: VRF_ID_0f;

                       soc_L3_ENTRY_IPV6_MULTICASTm_field32_set(unit,
                                           &l3x_ip6_mentry, vrf_field, vrf);

                      key_nbits = soc_fb_l3x_base_entry_to_key(unit,
                                                               (uint32 *) &l3x_ip6_uentry, key);
                  }
                  hash_bucket = soc_fb_l3_hash(unit, hash_select, key_nbits, key);
                  if (mcast_flag) {
                      printk("Hash(IPMC)[%d] of key 0x",hash_select);
                      for (loop = BITS2BYTES(key_nbits); loop > 0; loop--) {
                          printk("%02x", key[loop - 1]);
                      }
                      printk(" ");
                      printk("is bucket 0x%3x (%d)\n", hash_bucket,
                             hash_bucket);
                  } else {
                      printk("Hash(UC)[%d] of key 0x",hash_select);
                      for (loop = BITS2BYTES(key_nbits); loop > 0; loop--) {
                          printk("%02x", key[loop - 1]);
                      }
                      printk(" ");
                      printk("is bucket 0x%3x (%d)\n", hash_bucket,
                             hash_bucket);
                  }
                  break;
              }
#endif /* BCM_TRX_SUPPORT */
#ifdef BCM_FIREBOLT_SUPPORT
              if (SOC_IS_FBX(unit)) {
                  int key_nbits;
                  int hash_bucket = 0;
                  int mcast_flag =0;
                  int loop;
                  l3_entry_ipv6_unicast_entry_t    l3x_ip6_uentry;
                  l3_entry_ipv6_multicast_entry_t  l3x_ip6_mentry;

                  if (BCM_IP6_MULTICAST(ip6_addr)) {
                      soc_field_t v6f[] = {V6_0f, V6_1f, V6_2f, V6_3f};
                      soc_field_t mcf[] = {IPMC_0f, IPMC_1f, IPMC_2f, IPMC_3f};
                      soc_field_t vidf[] = {VLAN_ID_0f, VLAN_ID_1f, VLAN_ID_2f,
                          VLAN_ID_3f};
                          mcast_flag  = 1;

                          sal_memset(&l3x_ip6_mentry, 0,
                                     sizeof(l3_entry_ipv6_multicast_entry_t));

                          soc_mem_ip6_addr_set(unit, L3_ENTRY_IPV6_MULTICASTm,
                                               &l3x_ip6_mentry,
                                               GROUP_IP_ADDR_LWR_64f,
                                               (uint8*) &ip6_addr,
                                               SOC_MEM_IP6_LOWER_ONLY);

                          ip6_addr[0] = 0x0;
                          soc_mem_ip6_addr_set(unit, L3_ENTRY_IPV6_MULTICASTm,
                                               &l3x_ip6_mentry, GROUP_IP_ADDR_UPR_56f,
                                               (uint8*) &ip6_addr,
                                               SOC_MEM_IP6_UPPER_ONLY);

                          soc_mem_ip6_addr_set(unit, L3_ENTRY_IPV6_MULTICASTm,
                                               &l3x_ip6_mentry, SOURCE_IP_ADDR_LWR_64f,
                                               (uint8*) &sip6_addr,
                                               SOC_MEM_IP6_LOWER_ONLY);

                          soc_mem_ip6_addr_set(unit, L3_ENTRY_IPV6_MULTICASTm,
                                               &l3x_ip6_mentry, SOURCE_IP_ADDR_UPR_64f,
                                               (uint8*) &sip6_addr,
                                               SOC_MEM_IP6_UPPER_ONLY);

                          for (i=0; i < 4; i++) {
                              soc_L3_ENTRY_IPV6_MULTICASTm_field32_set(unit,
                                                                       &l3x_ip6_mentry, v6f[i], 1);
                              soc_L3_ENTRY_IPV6_MULTICASTm_field32_set(unit,
                                                                       &l3x_ip6_mentry, mcf[i], 1);
                              soc_L3_ENTRY_IPV6_MULTICASTm_field32_set(unit,
                                                                       &l3x_ip6_mentry, vidf[i], vlan);
                          }

                          key_nbits = soc_fb_l3x_base_entry_to_key(unit,
                                                                   (uint32 *) &l3x_ip6_mentry, key);
                  } else {
                      sal_memset(&l3x_ip6_uentry, 0,
                                 sizeof(l3_entry_ipv6_unicast_entry_t));

                      soc_mem_ip6_addr_set(unit, L3_ENTRY_IPV6_UNICASTm,
                                           &l3x_ip6_uentry, IP_ADDR_LWR_64f,
                                           ip6_addr, SOC_MEM_IP6_LOWER_ONLY);

                      soc_mem_ip6_addr_set(unit, L3_ENTRY_IPV6_UNICASTm,
                                           &l3x_ip6_uentry, IP_ADDR_UPR_64f,
                                           ip6_addr, SOC_MEM_IP6_UPPER_ONLY);

                      soc_L3_ENTRY_IPV6_UNICASTm_field32_set(unit,
                                                             &l3x_ip6_uentry,
                                                             V6_1f, 1);

                      soc_L3_ENTRY_IPV6_UNICASTm_field32_set(unit,
                                                             &l3x_ip6_uentry,
                                                             V6_0f, 1);

                      key_nbits = soc_fb_l3x_base_entry_to_key(unit,
                                                               (uint32 *) &l3x_ip6_uentry, key);
                  }
                  hash_bucket = soc_fb_l3_hash(unit, hash_select,
                                               key_nbits, key);
                  if (mcast_flag) {
                      printk("Hash(IPMC)[%d] of key 0x",hash_select);
                      for (loop = BITS2BYTES(key_nbits); loop > 0; loop--) {
                          printk("%02x", key[loop - 1]);
                      }
                      printk(" ");
                      printk("is bucket 0x%3x (%d)\n", hash_bucket,
                             hash_bucket);
                  } else {
                      printk("Hash(UC)[%d] of key 0x",hash_select);
                      for (loop = BITS2BYTES(key_nbits); loop > 0; loop--) {
                          printk("%02x", key[loop - 1]);
                      }
                      printk(" ");
                      printk("is bucket 0x%3x (%d)\n", hash_bucket,
                             hash_bucket);
                  }
              }
#endif /* BCM_FIREBOLT_SUPPORT */

#ifdef BCM_EASYRIDER_SUPPORT
              if (SOC_IS_EASYRIDER(unit)) {
                  int hash_bucket = 0;
                  int loop;
                  uint8 erkey[ER_L3V6_HASH_KEY_SIZE];
                  l3_entry_v6_entry_t l3x_ip6_entry;

                  sal_memset(&l3x_ip6_entry, 0, sizeof(l3_entry_v6_entry_t));
                  soc_mem_ip6_addr_set(unit, L3_ENTRY_V6m,
                                       (uint32 *) &l3x_ip6_entry,
                                       IP_ADDRf, ip6_addr,
                                       SOC_MEM_IP6_FULL_ADDR);

                  soc_er_l3v6_base_entry_to_key(unit,
                                                (uint32 *)&l3x_ip6_entry, erkey);

                  hash_bucket = soc_er_l3v6_hash(unit, hash_select, erkey);
                  printk("Hash(UC)[%d] of key 0x",hash_select);

                  for (loop = 15; loop >= 0; loop--) {
                      printk("%02x",erkey[loop]);
                  }
                  printk(" ");
                  printk("is bucket 0x%3x (%d)\n", hash_bucket,
                         hash_bucket);
              }
#endif /* end of BCM_EASYRIDER_SUPPORT */
              break;

              /* l3 l3table sanity */
          case L3_SANITY:
              {
                  l3x_entry_t     bucket_entry[SOC_L3X_MAX_BUCKET_SIZE];
                  int             index_min = soc_mem_index_min(unit, L3Xm);
                  int             index_max = soc_mem_index_max(unit, L3Xm);
                  int             low_index, high_index, ix, jx;
                  int             rv, cmd_rv = CMD_OK;
                  int             hash_select, sip_key, ipmc_enable;
                  int		    ge0;
                  uint32          regval;
                  port_tab_entry_t pte;

                  if (!SOC_IS_XGS_SWITCH(unit)) {
                      printk("Command only valid for XGS switches\n");
                      return CMD_FAIL;
                  }

                  parse_table_init(unit, &pt);
                  parse_table_add(&pt, "Start", PQ_DFL | PQ_INT,
                                  (void *)(0), &start, NULL);
                  parse_table_add(&pt, "End", PQ_DFL | PQ_INT,
                                  (void *)(0), &end, NULL);
                  if (!parseEndOk(a, &pt, &retCode))
                      return retCode;

                  low_index = start;
                  high_index = end;

                  if (low_index < index_min) {
                      low_index = index_min;
                  }

                  if (high_index > index_max) {
                      high_index = index_max;
                  }

                  /* Initialize entries to invalid */
                  memset(bucket_entry, 0, sizeof(l3x_entry_t) * SOC_L3X_MAX_BUCKET_SIZE);

                  if ((rv = READ_HASH_CONTROLr(unit, &regval)) < 0) {
                      printk("%s: ERROR: %s\n", ARG_CMD(a), soc_errmsg(rv));
                      return CMD_FAIL;
                  }
                  hash_select = soc_reg_field_get(unit, HASH_CONTROLr,
                                                  regval, HASH_SELECTf);

                  ge0 = SOC_PORT(unit, ge, 0);
                  if ((rv = READ_CONFIGr(unit, ge0, &regval)) < 0) {
                      printk("%s: ERROR: %s\n", ARG_CMD(a), soc_errmsg(rv));
                      return CMD_FAIL;
                  }

                  sip_key = 0;
                  ipmc_enable = 0;
                  if (SOC_CHIP_GROUP(unit) == SOC_CHIP_BCM5690) {    /* Draco */
                      sip_key = soc_reg_field_get(unit, CONFIGr, regval,
                                                  SRC_IP_CFGf);
                      ipmc_enable = soc_reg_field_get(unit, CONFIGr, regval,
                                                      IPMC_ENABLEf);
                  } else if (SOC_IS_LYNX(unit)) {  /* LYNX */
                      ipmc_enable = soc_reg_field_get(unit, CONFIGr, regval,
                                                      IPMC_ENABLEf);
                      BCM_IF_ERROR_RETURN(READ_ARL_CONTROLr(unit, &regval));
                      sip_key = soc_reg_field_get(unit, ARL_CONTROLr,
                                                  regval, SRC_IP_CFGf);
                  } else if (SOC_CHIP_GROUP(unit) == SOC_CHIP_BCM5695) {    /* Draco1.5 */
                      /*
                       * Depending on IPMC_DO_VLANf, draco 1.5 IPMC key is either
                       * (DIP + SIP + VID) or (DIP + SIP + 0), always 76 bits
                       */
                      BCM_IF_ERROR_RETURN
                          (READ_PORT_TABm(unit, MEM_BLOCK_ANY,
                                          SOC_PORT(unit, ge, 0), &pte));
                      ipmc_enable = soc_PORT_TABm_field32_get(unit, &pte, IPMC_ENABLEf);
                  } else if (SOC_IS_TUCANA(unit)) {    /* Tucana */
                      BCM_IF_ERROR_RETURN
                          (READ_PORT_TABm(unit, MEM_BLOCK_ANY,
                                          SOC_PORT(unit, ge, 0), &pte));
                      ipmc_enable = soc_PORT_TABm_field32_get(unit, &pte, IPMC_ENABLEf);
                  }

                  /* Prep the L3 comparison function */
                  if ((rv = _soc_mem_cmp_l3x_sync(unit)) < 0) {
                      printk("%s: ERROR: %s\n", ARG_CMD(a), soc_errmsg(rv));
                      return CMD_FAIL;
                  }

                  printk("Start index: %d\n", low_index);
                  printk("(Each '.' is 100 entries scanned)\n");

                  for (ix = low_index; ix <= high_index; ix++) {
                      int             offset = ix % SOC_L3X_BUCKET_SIZE(unit);

                      if ((rv = diag_l3x_entry_verify(unit, hash_select,
                                                      ipmc_enable, sip_key, ix,
                                                      &(bucket_entry[offset]))) <
                          0) {
                          printk("%s: ERROR: %s at index %d\n", ARG_CMD(a),
                                 soc_errmsg(rv), ix);
                          soc_mem_entry_dump(unit, L3Xm, &(bucket_entry[offset]));
                          printk("\n");
                          cmd_rv = CMD_FAIL;
                          continue;
                      }

                      /* Check bucket array for duplicates */
                      if (soc_L3Xm_field32_get(unit, &(bucket_entry[offset]),
                                               L3_VALIDf)) {
                          for (jx = 0; jx < offset; jx++) {
                              if (soc_L3Xm_field32_get(unit,
                                                       &(bucket_entry[jx]),
                                                       L3_VALIDf)) {
                                  if (soc_mem_compare_key
                                      (unit, L3Xm, &(bucket_entry[offset]),
                                       &(bucket_entry[jx])) == 0) {
                                      /* Found a match! Bad. */
                                      printk("%s: ERROR: Duplicate keys: %d ~= %d\n",
                                             ARG_CMD(a), ix,
                                             ((ix / SOC_L3X_BUCKET_SIZE(unit)) *
                                              SOC_L3X_BUCKET_SIZE(unit)) + jx);
                                      soc_mem_entry_dump(unit, L3Xm,
                                                         &(bucket_entry[offset]));
                                      printk("\n");
                                      soc_mem_entry_dump(unit, L3Xm,
                                                         &(bucket_entry[jx]));
                                      printk("\n");
                                      cmd_rv = CMD_FAIL;
                                  }
                              }
                          }
                      }

                      if (((ix - low_index) % 100) == 99) {
                          printk(".");
                      }
                  }

                  printk("\nEnd index: %d\n", high_index);

                  return cmd_rv;
              }
              break;
          case L3_UNTAG:
              if ((r = bcm_l3_untagged_update(unit)) < 0) {
                  printk("%s: Error updating L3 table untagged status: %s\n",
                         ARG_CMD(a), bcm_errmsg(r));
                  return CMD_FAIL;
              }
              break;
          case L3_CONFLICT:
            return _l3_cmd_l3_conflict_show(unit, a);
#endif /* BCM_XGS_SWITCH_SUPPORT*/
          default:
              return CMD_USAGE;
              break;
        }

        return CMD_OK;
    }

    if (sal_strcasecmp(table, "defip") == 0) {
	switch (subcmd) {
        /* l3 defip add */
	case L3_ADD:
        return _l3_cmd_route_add(unit, a, FALSE); 
        /* l3 defip destroy */
	case L3_DESTROY:
	    parse_table_init(unit, &pt);
#if defined(BCM_XGS3_SWITCH_SUPPORT)
	    parse_table_add(&pt, "VRF", PQ_DFL | PQ_INT, 0, (void *)&vrf, 0);
#endif
	    parse_table_add(&pt, "IP", PQ_DFL | PQ_IP, 0, (void *)&ip_addr, 0);
	    parse_table_add(&pt, "MaSk", PQ_DFL | PQ_IP, 0, (void *)&mask, 0);
#ifdef BCM_XGS_SWITCH_SUPPORT
	    parse_table_add(&pt, "ECMP", PQ_DFL | PQ_BOOL, 0, (void *)&ecmp, 0);
	    parse_table_add(&pt, "Mac", PQ_DFL | PQ_MAC, 0, (void *)mac, 0);
	    parse_table_add(&pt, "Gateway", PQ_DFL | PQ_IP, 0, (void *)&gateway, 0);
	    parse_table_add(&pt, "Port", PQ_DFL | PQ_PORT, 0, (void *)&port, 0);
	    parse_table_add(&pt, "MOdule", PQ_DFL | PQ_INT, 0, (void*)&module, 0);
	    parse_table_add(&pt, "INtf", PQ_DFL | PQ_INT, 0, (void *)&interface, 0);
	    parse_table_add(&pt, "VLAN", PQ_DFL | PQ_INT, 0, (void *)&id, 0);
#endif
	    if (!parseEndOk(a, &pt, &retCode)) {
	        return retCode;
	    }

            bcm_l3_route_t_init(&route_info);

#if defined(BCM_XGS3_SWITCH_SUPPORT)
	    route_info.l3a_vrf     = vrf;
#endif
	    route_info.l3a_subnet = ip_addr;
	    route_info.l3a_ip_mask = mask;
	    route_info.l3a_vid = id; 
	    if (((mask & 0x80000000) == 0) && ip_addr != 0) {
	        debugk(DK_WARN, "%s: Invalid mask\n", ARG_CMD(a));
	        return CMD_FAIL;
	    }
#ifdef BCM_XGS_SWITCH_SUPPORT

	    if (ecmp) {
	        route_info.l3a_flags     |= BCM_L3_MULTIPATH;
	        sal_memcpy(route_info.l3a_nexthop_mac, mac, sizeof(mac));
	        route_info.l3a_nexthop_ip = gateway;
	        route_info.l3a_intf       = interface;
	        route_info.l3a_modid      = module;
	        route_info.l3a_port_tgid  = port;
	    }
#endif

        if ((r = bcm_l3_route_delete(unit, &route_info)) < 0) {
            format_ipaddr(ip_str, ip_addr);
            printk("%s: Error deleting %d.%d.%d.%d from DEF IP "
                   "table: %s\n", ARG_CMD(a),
                   (route_info.l3a_subnet >> 24) & 0xff,
                   (route_info.l3a_subnet >> 16) & 0xff,
                   (route_info.l3a_subnet >> 8) & 0xff,
                   route_info.l3a_subnet & 0xff,
                   bcm_errmsg(r));
            return CMD_FAIL;
        }
	    break;

        /* l3 defip clear */
	case L3_CLEAR:
	    bcm_l3_defip_delete_all(unit);
	    break;

        /* l3 defip age */
	case L3_AGE:
	    bcm_l3_route_age(unit, 0, _l3_cmd_route_age_callback, NULL);
	    return CMD_OK;

        /* l3 defip show */
	case L3_SHOW:
            if ((r = bcm_l3_info(unit, &l3_hw_status)) < 0) {
	        printk("%s: Error L3 accessing: %s\n", ARG_CMD(a), bcm_errmsg(r));
	        return CMD_FAIL;
            }
            free_defip   = l3_hw_status.l3info_max_route -
                           l3_hw_status.l3info_occupied_route;
            free_lpm_blk = l3_hw_status.l3info_max_lpm_block -
                           l3_hw_status.l3info_used_lpm_block;

            if (SOC_CHIP_GROUP(unit) == SOC_CHIP_BCM5690 || /* DRACO */
                SOC_CHIP_GROUP(unit) == SOC_CHIP_BCM5695 || /* DRACO1.5 */
                SOC_IS_TUCANA(unit)) { /* TUCANA */
                last_entry = soc_mem_index_max(unit, DEFIP_HIm);
            } else if (SOC_IS_LYNX(unit)) { /* LYNX */
                last_entry = soc_mem_index_max(unit, DEFIPm);
            } else {
  		last_entry = l3_hw_status.l3info_max_route;
	    }
            num_entries = last_entry + 1;

	    if (ARG_CNT(a)) {
		if (!isint(ARG_CUR(a))) {
		    printk("%s: Invalid entry number: %s\n", ARG_CMD(a),
			   ARG_CUR(a));
		    return (CMD_FAIL);
		}
		first_entry = parse_integer(ARG_GET(a));
		if (first_entry > last_entry) {
		    printk("%s: Error: Illegal entry number (0 to %d)\n",
			   ARG_CMD(a), last_entry);
		    return (CMD_FAIL);
		}
		last_entry = first_entry;
	    } else {
                first_entry = 0;
            }

            printk("Unit %d, Total Number of DEFIP entries: %d\n",
                    unit, num_entries);
            if (SOC_IS_XGS12_SWITCH(unit)) {
#if defined(BCM_XGS_SWITCH12_SUPPORT)
                printk("Free LPM blocks: %d, Free LPM entries: %d\n",
                       free_lpm_blk, free_defip);
                bcm_l3_defip_max_ecmp_get(unit, &max_ecmp);
                if (max_ecmp) {
                    printk("Max number of ECMP paths %d\n", max_ecmp);
                }
#endif
            } 

            printk("#     VRF     Net addr             Next Hop Mac      "
                   "  INTF MODID PORT PRIO CLASS HIT VLAN\n");
            bcm_l3_route_traverse(unit, 0, first_entry, last_entry,
                                      _l3_cmd_route_print, NULL);
	    return CMD_OK;

#ifdef BCM_XGS_SWITCH_SUPPORT
	case L3_TEST:
	    diag_defip_test(unit);
	    break;

	case L3_CHECK:
	    subcmd_s = ARG_GET(a);
	    if (sal_strcasecmp(subcmd_s, "cmp") == 0) {
		diag_defip_check(unit, 0);
	    } else if (sal_strcasecmp(subcmd_s, "sw") == 0) {
		diag_defip_check(unit, 1);
	    } else if (sal_strcasecmp(subcmd_s, "hw") == 0) {
		diag_defip_check(unit, 2);
	    }
	    break;

        /* l3 defip ecmp <MAX>=<val> */
        case L3_ECMP:
            parse_table_init(unit, &pt);
            parse_table_add(&pt, "MAX", PQ_DFL | PQ_INT, 0, (void *)&max_ecmp, 0);
            if (!parseEndOk(a, &pt, &retCode)) {
                return retCode;
            }

            bcm_l3_defip_max_ecmp_set(unit, max_ecmp);
            break;

        /* l3 defip ecmphash */
        case L3_ECMP_HASH:
            return _l3_cmd_defip_ecmp_hash(unit, a);
#endif /* BCM_XGS_SWITCH_SUPPORT */
	default:
	    return CMD_USAGE;
	    break;
	}

	return CMD_OK;
    }

#if defined(BCM_XGS3_SWITCH_SUPPORT)
    if (sal_strcasecmp(table, "ip6host") == 0) {
	switch (subcmd) {
        /* l3 ip6host add */
	case L3_ADD:
        return _l3_cmd_host_add(unit, a, TRUE);
        /* l3 ip6host delete */
	case L3_DESTROY:
	    parse_table_init(unit, &pt);
	    parse_table_add(&pt, "VRF", PQ_DFL | PQ_INT, 0, (void *)&vrf, 0);
	    parse_table_add(&pt, "IP", PQ_DFL | PQ_IP6, 0, (void *)&ip6_addr, 0);
	    if (!parseEndOk(a, &pt, &retCode)) {
		return retCode;
            }

            sal_memcpy(ip_info.l3a_ip6_addr, ip6_addr, BCM_IP6_ADDRLEN);
            ip_info.l3a_vrf     = vrf;
            ip_info.l3a_flags   = BCM_L3_IP6;
	    if ((r = bcm_l3_host_delete(unit, &ip_info)) < 0) {
		printk("%s: Error deleting from L3 table %s\n",
		       ARG_CMD(a), bcm_errmsg(r));
		return CMD_FAIL;
	    }
	    break;

        /* l3 ip6host clear */
	case L3_CLEAR:
            ip_info.l3a_flags   = BCM_L3_IP6;
	    bcm_l3_host_delete_all(unit, &ip_info);
	    break;

        /* l3 ip6host age */
	case L3_AGE:
	    bcm_l3_host_age(unit, BCM_L3_HIT | BCM_L3_IP6,
                            _l3_cmd_host_age_callback, NULL);
	    return CMD_OK;

        /* l3 ip6host show */
	case L3_SHOW:
            if ((r = bcm_l3_info(unit, &l3_hw_status)) < 0) {
                printk("%s: Error L3 accessing: %s\n", ARG_CMD(a), bcm_errmsg(r));
                return CMD_FAIL;
            }
            free_l3 = (l3_hw_status.l3info_max_host -
                       l3_hw_status.l3info_occupied_host) / 2;
            last_entry = l3_hw_status.l3info_max_host / 2;
            if (ARG_CNT(a)) {
                if (!isint(ARG_CUR(a))) {
                    printk("%s: Invalid number: %s\n", ARG_CMD(a), ARG_CUR(a));
                    return (CMD_FAIL);
                }
                first_entry = parse_integer(ARG_GET(a));
                if (first_entry > last_entry) {
                    printk("%s: Error: Illegal entry number (0 to %d)\n",
                           ARG_CMD(a), last_entry);
                    return (CMD_FAIL);
                }
                last_entry = first_entry;
	    } else {
                first_entry = 0;
            }

            printk("Unit %d, free IPv6 table entries: %d\n", unit, free_l3);
            printk("Entry VRF IP address                         "
                   "          Mac Address        INTF MOD PORT     CLASS HIT\n");
            bcm_l3_host_traverse(unit, BCM_L3_IP6, first_entry, last_entry,
                                 _l3_cmd_host_print, NULL);
            printk("\n");
	    return CMD_OK;
        default:
            return CMD_USAGE;
            break;
        }
        return CMD_OK;
    }

    if (sal_strcasecmp(table, "ip6route") == 0) {
        bcm_ip6_t ip6_mask;
        int masklen = -1;
        int mask_len_max = 
            soc_feature(unit, soc_feature_lpm_prefix_length_max_128) ? 128 : 64;


	switch (subcmd) {

        /* l3 ip6route add */
	case L3_ADD:
        return _l3_cmd_route_add(unit, a, TRUE); 
        /* l3 ip6route delete */
	case L3_DESTROY:
	    parse_table_init(unit, &pt);
	    parse_table_add(&pt, "VRF", PQ_DFL | PQ_INT, 0, (void *)&vrf, 0);
	    parse_table_add(&pt, "IP", PQ_DFL | PQ_IP6, 0, (void *)&ip6_addr, 0);
	    parse_table_add(&pt, "MaskLen", PQ_DFL | PQ_INT, 0,
                            (void *)&masklen, 0);
            parse_table_add(&pt, "ECMP", PQ_DFL | PQ_BOOL, 0, (void *)&ecmp, 0);
            parse_table_add(&pt, "Mac", PQ_DFL | PQ_MAC, 0, (void *)mac, 0);
            parse_table_add(&pt, "Port", PQ_DFL | PQ_PORT, 0, (void *)&port, 0);
            parse_table_add(&pt, "MOdule", PQ_DFL | PQ_INT, 0, (void*)&module, 0);
            parse_table_add(&pt, "INtf", PQ_DFL | PQ_INT, 0, (void *)&interface, 0);
            if (!parseEndOk(a, &pt, &retCode)) {
                return retCode;
            }

        if ((masklen < 0) || (masklen > mask_len_max)) {
            printk("%s: Invalid prefix length.\n", ARG_CMD(a));
            return CMD_FAIL;
        }
            bcm_l3_route_t_init(&route_info);

        route_info.l3a_vrf = vrf;
	    sal_memcpy(route_info.l3a_ip6_net, ip6_addr, BCM_IP6_ADDRLEN);
            bcm_ip6_mask_create(ip6_mask, masklen);
            sal_memcpy(route_info.l3a_ip6_mask, ip6_mask, BCM_IP6_ADDRLEN);
	    sal_memcpy(route_info.l3a_nexthop_mac, mac, sizeof(mac));
            route_info.l3a_flags = BCM_L3_IP6;
            if (ecmp) {
                route_info.l3a_flags     |= BCM_L3_MULTIPATH;
                sal_memcpy(route_info.l3a_nexthop_mac, mac, sizeof(mac));
                route_info.l3a_intf       = interface;
                route_info.l3a_modid      = module;
                route_info.l3a_port_tgid  = port;
            }

            if ((r = bcm_l3_route_delete(unit, &route_info)) < 0) {
                format_ip6addr(ip_str, ip6_addr);
                printk("ERROR %s: delete %s/%d from DEF IP "
                       "table: %s\n", ARG_CMD(a), ip_str, masklen, bcm_errmsg(r));
                return CMD_FAIL;
            }
            break;

        /* l3 ip6route clear */
	case L3_CLEAR:
            route_info.l3a_flags   = BCM_L3_IP6;
	    bcm_l3_route_delete_all(unit, &route_info);
	    break;

        /* l3 defip age */
	case L3_AGE:
	    bcm_l3_route_age(unit, BCM_L3_IP6, _l3_cmd_route_age_callback, NULL);
	    break;

        /* l3 ip6route show */
	case L3_SHOW:
#if defined(BCM_EASYRIDER_SUPPORT)
            if (SOC_IS_EASYRIDER(unit)) {
                printk("#     VRF     Net addr                                "
                       "     Next Hop Mac      "
                       "  INTF MODID PORT PRIO CLASS HIT VLAN\n");
                bcm_l3_route_traverse(unit, BCM_L3_IP6, 0, 100,
                                     _l3_cmd_route_print, NULL);
                return CMD_OK;
            }
#endif /* BCM_EASYRIDER_SUPPORT */
            if ((r = bcm_l3_info(unit, &l3_hw_status)) < 0) {
                printk("%s: Error L3 accessing: %s\n", ARG_CMD(a), bcm_errmsg(r));
                return CMD_FAIL;
            }
            free_defip = (l3_hw_status.l3info_max_route -
                          l3_hw_status.l3info_occupied_route) / 2;
            last_entry = l3_hw_status.l3info_max_route / 2;
            if (ARG_CNT(a)) {
                if (!isint(ARG_CUR(a))) {
                    printk("%s: Invalid number: %s\n", ARG_CMD(a), ARG_CUR(a));
                    return (CMD_FAIL);
                }
                first_entry = parse_integer(ARG_GET(a));
                if (first_entry > last_entry) {
                    printk("%s: Error: Illegal entry number (0 to %d)\n",
                           ARG_CMD(a), last_entry);
                    return (CMD_FAIL);
                }
                last_entry = first_entry;
	    } else {
                first_entry = 0;
            }

            printk("Unit %d, Total Number of IPv6 entries: %d\n",
                    unit, last_entry + 1);
            bcm_l3_defip_max_ecmp_get(unit, &max_ecmp);
            if (max_ecmp) {
                printk("Max number of ECMP paths %d\n", max_ecmp);
            }
            printk("Free IPv6 entries available: %d\n", free_defip);

            printk("#     VRF     Net addr                                "
                   "     Next Hop Mac      "
                   "  INTF MODID PORT PRIO CLASS HIT VLAN\n");
            bcm_l3_route_traverse(unit, BCM_L3_IP6, first_entry, last_entry,
                                  _l3_cmd_route_print, NULL);
	    return CMD_OK;

        default:
            return CMD_USAGE;
            break;
        }
        return CMD_OK;
    }

    if (sal_strcasecmp(table, "tunnel_init") == 0) {
        switch (subcmd) {

        /* l3 tunnel_init set */
        case L3_SET:
            return _l3_cmd_tunnel_init_add(unit, a);
        /* l3 tunnel_init show */
        case L3_SHOW:
            return _l3_cmd_tunnel_init_show(unit, a);
        /* l3 tunnel_init clear */
        case L3_CLEAR:
            return _l3_cmd_tunnel_init_clear(unit, a);
        default:
            return CMD_USAGE;
            break;
        }
        return CMD_OK;
    }

    if (sal_strcasecmp(table, "tunnel_term") == 0) {
        switch (subcmd) {
        /* l3 tunnel_term add */
        case L3_ADD:
            return _l3_cmd_tunnel_term_add(unit, a); 
        /* l3 tunnel_term delete */
        case L3_DESTROY:
            return _l3_cmd_tunnel_term_delete(unit, a); 
        /* l3 tunnel_term show */
        case L3_SHOW:
            return _l3_cmd_tunnel_term_get(unit, a); 
        default:
            return CMD_USAGE;
            break;
        }
        return CMD_OK;
    }

    if (sal_strcasecmp(table, "dscp_map_port") == 0) {
        switch (subcmd) {
            /* l3 dscp_map_port show */
          case L3_SHOW:
              return _l3_cmd_dscp_map_port_show(unit, a); 
              /* l3 dscp_map_port set */
          case L3_ADD:
          case L3_SET:
              return _l3_cmd_dscp_map_port_set(unit, a); 
          default:
              return (CMD_USAGE);
        }
        return CMD_OK;
    }
    if (sal_strcasecmp(table, "dscp_map") == 0) {
        switch (subcmd) {
            /* l3 dscp_map add */
          case L3_ADD:
              return _l3_cmd_dscp_map_add(unit, a); 
              /* l3 dscp_map destroy */
          case L3_DESTROY:
              return _l3_cmd_dscp_map_delete(unit, a); 

              /* l3 dscp_map show */
          case L3_SHOW:
              return _l3_cmd_dscp_map_show(unit, a); 
              /* l3 dscp_map set */
          case L3_SET:
              return _l3_cmd_dscp_map_set(unit, a); 

          default:
              return CMD_USAGE;
        }
        return CMD_OK;
    }
#endif

    return CMD_USAGE;
}
#endif /* INCLUDE_L3 */
