/* 
 * $Id: ipmc.c 1.6 Broadcom SDK $
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
 * Purpose:     IPMC CLI Commands
 *
 */

#include <appl/diag/shell.h>
#include <appl/diag/system.h>
#include <appl/diag/parse.h>

#include <bcm/error.h>
#include <bcm/ipmc.h>
#include <bcm/debug.h>

STATIC cmd_result_t
_cmd_sbx_ipmc_sg_flow(int unit, args_t *args);

STATIC cmd_result_t
_cmd_sbx_ipmc_egr_intf(int unit, args_t *args);

#define CMD_IPMC_FLOW_USAGE                                     \
    "    sg-flow [add | replace | remove | delete-all | find | get]\n"       \
    "         [s=<sip>] [g=<dip>]\n"                            \
    "         [vid=<vid>] [ipmc-index=<ipmc-index>] [valid=<1 or 0>] [modid=<modid>]\n"     \
    "    egr-intf [add | delete | delete-all | get | set]\n"    \
    "         [ipmc-index=<ipmc-index>] [port=<port>] [untag=<1|0>]\n"        \
    "         [intf-id=<intf-id>] \n" \
    "         [if-count=<if-count> if1=<intf-id> if2=<intf-id> if3=<intf-id>]\n"

static cmd_t _cmd_sbx_ipmc_list[] = {
    {"sg-flow",   _cmd_sbx_ipmc_sg_flow,   "\n" CMD_IPMC_FLOW_USAGE, NULL},
    {"egr-intf",  _cmd_sbx_ipmc_egr_intf,   "\n" CMD_IPMC_FLOW_USAGE, NULL},    
};

char cmd_sbx_ipmc_usage[] =
    "\n"
#ifdef COMPILER_STRING_CONST_LIMIT
    "    ipmc <option> [args...]\n"
#else
    CMD_IPMC_FLOW_USAGE  "\n"
#endif
    ;
/*
 * Local global variables to remember last values in arguments.
 */

STATIC cmd_result_t
_cmd_sbx_ipmc_sg_flow(int unit, args_t *args)
{
    cmd_result_t       retCode;
    int                rv, vid;
    parse_table_t      pt;
    char	      *subcmd;
    bcm_ipmc_addr_t    data;

    if ((subcmd = ARG_GET(args)) == NULL) {
        printk("%s ERROR: empty cmd\n", __FUNCTION__);
        return CMD_FAIL;
    }

    vid = 0;
    
    bcm_ipmc_addr_t_init(&data);
    
    parse_table_init(unit, &pt);

    /* Parse command option arguments */
    parse_table_add(&pt, "s", PQ_DFL|PQ_IP,
                    0, &data.s_ip_addr, NULL);
    parse_table_add(&pt, "g", PQ_DFL|PQ_IP,
                    0, &data.mc_ip_addr, NULL);
    parse_table_add(&pt, "vid",  PQ_DFL|PQ_INT,
                    0, &vid, NULL);
    parse_table_add(&pt, "ipmc-index", PQ_DFL|PQ_HEX,
                    0, &data.ipmc_index, NULL);
    parse_table_add(&pt, "l2-pbmp", PQ_DFL|PQ_PBMP,
                    0, &data.l2_pbmp, NULL);
    parse_table_add(&pt, "l2-ubmp", PQ_DFL|PQ_PBMP,
                    0, &data.l2_ubmp, NULL);
    parse_table_add(&pt, "valid", PQ_DFL|PQ_INT,
                    0, &data.v, NULL);
    parse_table_add(&pt, "modid", PQ_DFL|PQ_INT,
                    0, &data.mod_id, NULL);
    parse_table_add(&pt, "port_tgid", PQ_DFL|PQ_INT,
                    0, &data.port_tgid, NULL);
    
    if (!parseEndOk(args, &pt, &retCode)) {
        return retCode;
    }

    data.vid = vid;
    
    if (data.ipmc_index) {
        data.flags |= BCM_IPMC_USE_IPMC_INDEX;
    }
    
    if (sal_strcasecmp(subcmd, "add") == 0) {

        rv = bcm_ipmc_add(unit, &data);
        if (rv == BCM_E_NONE) {
            printk("Added (0x%x, 0x%x, %d) with ipmc-index(0x%x)\n",
                   data.s_ip_addr, data.mc_ip_addr,
                   data.vid, data.ipmc_index);
        } else {
            printk("Error(%s) adding (0x%x, 0x%x, %d) ipmc-index(0x%x)\n",
                   bcm_errmsg(rv), data.s_ip_addr, data.mc_ip_addr,
                   data.vid, data.ipmc_index);
            return CMD_FAIL;
        }
        
    } else if (sal_strcasecmp(subcmd, "replace") == 0) {

        data.flags |= BCM_IPMC_REPLACE;
        
        rv = bcm_ipmc_add(unit, &data);
        if (rv == BCM_E_NONE) {
            printk("Replace (0x%x, 0x%x, %d) with ipmc-index(0x%x)\n",
                   data.s_ip_addr, data.mc_ip_addr,
                   data.vid, data.ipmc_index);
        } else {
            printk("Error(%s) replacing (0x%x, 0x%x, %d) ipmc-index(0x%x)\n",
                   bcm_errmsg(rv), data.s_ip_addr, data.mc_ip_addr,
                   data.vid, data.ipmc_index);
            return CMD_FAIL;
        }
        
    } else if (sal_strcasecmp(subcmd, "find") == 0) {

        rv = bcm_ipmc_find(unit, &data);
        if (rv == BCM_E_NONE) {
            printk("(0x%x, 0x%x, %d) --> ipmc-index(0x%x), cos(0x%x)\n",
                   data.s_ip_addr, data.mc_ip_addr, data.vid,
                   data.ipmc_index, data.cos);
        } else {
            printk("error(%s) bcm_ipmc_find()\n",
                   bcm_errmsg(rv));
            return CMD_FAIL;
        }
    } else if (sal_strcasecmp(subcmd, "get") == 0) {
        if (data.ipmc_index) {
            rv = bcm_ipmc_get_by_index(unit,
                                       data.ipmc_index,
                                       &data);
        } else {
            rv = bcm_ipmc_get(unit,
                              data.s_ip_addr,
                              data.mc_ip_addr,
                              data.vid,
                              &data);
        }
        
        if (rv == BCM_E_NONE) {
            printk("(0x%x, 0x%x, %d) --> index(0x%x), cos(0x%x)\n",
                   data.s_ip_addr, data.mc_ip_addr, data.vid,
                   data.ipmc_index, data.cos);
        } else {
            printk("error(%s) bcm_ipmc_get() or bcm_ipmc_get_by_index()\n",
                   bcm_errmsg(rv));
            return CMD_FAIL;
        }
    } else if (sal_strcasecmp(subcmd, "delete-all") == 0) {

        rv = bcm_ipmc_delete_all(unit);
        if (rv == BCM_E_NONE) {
            printk("Successfully deleted all ipmc sg\n");
        } else {
            printk("Error(%s) deleting all ipmc sg\n",
                   bcm_errmsg(rv));
            return CMD_FAIL;
        }
    } else if (sal_strcasecmp(subcmd, "remove") == 0) {

        rv = bcm_ipmc_remove(unit, &data);
        if (rv == BCM_E_NONE) {
            printk("Removed (0x%x, 0x%x, %d) with ipmc-index(0x%x)\n",
                   data.s_ip_addr, data.mc_ip_addr,
                   data.vid, data.ipmc_index);
        } else {
            printk("Error(%s) removing (0x%x, 0x%x, %d) ipmc-index(0x%x)\n",
                   bcm_errmsg(rv), data.s_ip_addr, data.mc_ip_addr,
                   data.vid, data.ipmc_index);
            return CMD_FAIL;
        }
    } else {
        printk("invalid sub-command\n");
        return CMD_FAIL;
    }
    
    return CMD_OK;
}

STATIC cmd_result_t
_cmd_sbx_ipmc_egr_intf(int unit, args_t *args)
{
    cmd_result_t       retCode;
    int                rv, untag;
    parse_table_t      pt;
    char	      *subcmd;
    int                ipmc_index;
    bcm_port_t         port;
    bcm_l3_intf_t      l3_intf;
    int                l3a_intf_id;
    int                if1_intf_id, if2_intf_id, if3_intf_id;
    int                if_max = 40;
    int                if_array[40];
    int                if_count;
    int                ii;

    if ((subcmd = ARG_GET(args)) == NULL) {
        printk("%s ERROR: empty cmd\n", __FUNCTION__);
        return CMD_FAIL;
    }

    port        = 0;
    untag       = 0;
    l3a_intf_id = 0;
    ipmc_index  = 0;
    if_count    = 0;
    if1_intf_id = 0;
    if2_intf_id = 0;
    if3_intf_id = 0;
    
    parse_table_init(unit, &pt);

    /* Parse command option arguments */
    parse_table_add(&pt, "port", PQ_DFL|PQ_PORT,
                    0, &port, NULL);
    parse_table_add(&pt, "untag", PQ_DFL|PQ_INT,
                    0, &untag, NULL);
    parse_table_add(&pt, "intf-id", PQ_DFL|PQ_INT,
                    0, &l3a_intf_id, NULL);
    parse_table_add(&pt, "ipmc-index", PQ_DFL|PQ_HEX,
                    0, &ipmc_index, NULL);
    parse_table_add(&pt, "if-count", PQ_DFL|PQ_INT,
                    0, &if_count, NULL);
    parse_table_add(&pt, "if1", PQ_DFL|PQ_INT,
                    0, &if1_intf_id, NULL);
    parse_table_add(&pt, "if2", PQ_DFL|PQ_INT,
                    0, &if2_intf_id, NULL);
    parse_table_add(&pt, "if3", PQ_DFL|PQ_INT,
                    0, &if3_intf_id, NULL);
    
    if (!parseEndOk(args, &pt, &retCode)) {
        return retCode;
    }

    if (sal_strcasecmp(subcmd, "add") == 0) {

        bcm_l3_intf_t_init(&l3_intf);
        l3_intf.l3a_intf_id  = l3a_intf_id;
        l3_intf.l3a_flags   |= BCM_L3_WITH_ID;

         if (untag) {
            l3_intf.l3a_flags |= BCM_L3_UNTAG;
        }
        
        rv = bcm_ipmc_egress_intf_add(unit,
                                      ipmc_index,
                                      port,
                                      &l3_intf);
        if (rv == BCM_E_NONE) {
            printk("Added l3a-intf-id(0x%x) successfully to "
                   "flow with ipmc-index(0x%x) on port(%d)\n",
                   l3_intf.l3a_intf_id, ipmc_index, port);
        } else {
            printk("Failure(%s) Adding l3a-intf-id(0x%x) to "
                   "flow with ipmc-index(0x%x) on port(%d)\n",
                   bcm_errmsg(rv), l3_intf.l3a_intf_id,
                   ipmc_index, port);
            
            return CMD_FAIL;
        }
        
    } else if (sal_strcasecmp(subcmd, "delete") == 0) {

        bcm_l3_intf_t_init(&l3_intf);
        l3_intf.l3a_intf_id  = l3a_intf_id;
        l3_intf.l3a_flags   |= BCM_L3_WITH_ID;

        if (untag) {
            l3_intf.l3a_flags |= BCM_L3_UNTAG;
        }
         
        rv = bcm_ipmc_egress_intf_delete(unit,
                                         ipmc_index,
                                         port,
                                         &l3_intf);
        if (rv == BCM_E_NONE) {
            printk("Deleted l3a-intf-id(0x%x) successfully from "
                   "flow with ipmc-index(0x%x) on port(%d)\n",
                   l3_intf.l3a_intf_id, ipmc_index, port);
        } else {
            printk("Failure(%s) Deleting l3a-intf-id(0x%x) from "
                   "flow with ipmc-index(0x%x) on port(%d)\n",
                   bcm_errmsg(rv), l3_intf.l3a_intf_id,
                   ipmc_index, port);
            
            return CMD_FAIL;
        }
        
    } else if (sal_strcasecmp(subcmd, "delete-all") == 0) {

        rv = bcm_ipmc_egress_intf_delete_all(unit,
                                             ipmc_index,
                                             port);
        if (rv == BCM_E_NONE) {
            printk("Deleted all egress interface(s) for "
                   "flow with ipmc-index(0x%x) on port(%d)\n",
                   ipmc_index, port);
        } else {
            printk("Failure(%s) Deleting all egress interface(s) for "
                   "flow with ipmc-index(0x%x) on port(%d)\n",
                   bcm_errmsg(rv),
                   ipmc_index, port);
            
            return CMD_FAIL;
        }
        
    } else if (sal_strcasecmp(subcmd, "get") == 0) {

        rv = bcm_ipmc_egress_intf_get(unit,
                                      ipmc_index,
                                      port,
                                      if_max,
                                      if_array,
                                      &if_count);
        if (rv == BCM_E_NONE) {
            printk("Get of egress interface succeeded for "
                   "flow with ipmc-index(0x%x) on port(%d)\n",
                   ipmc_index, port);
            for (ii = 0; ii < if_count; ii++) {
                printk("l3a-intf-id(0x%x)\n",
                       if_array[ii]);
            }
            
        } else {
            printk("Failure(%s) Get of egress interface for "
                   "flow with ipmc-index(0x%x) on port(%d)\n",
                   bcm_errmsg(rv), ipmc_index, port);

            return CMD_FAIL;
        }
        
    } else if (sal_strcasecmp(subcmd, "set") == 0) {

        if (if_count) {
            if_array[0] = if1_intf_id;
            if_array[1] = if2_intf_id;
            if_array[2] = if3_intf_id;
            if (if_count > 3) {
                printk("forcing if-count to 3\n");
                if_count = 3;
            }
        }

        rv = bcm_ipmc_egress_intf_set(unit,
                                      ipmc_index,
                                      port,
                                      if_count,
                                      if_array);
        if (rv == BCM_E_NONE) {
            printk("Set of egress interface succeeded for "
                   "flow with ipmc-index(0x%x) on port(%d), "
                   "if-count(%d) intf-id(0x%x, 0x%x, 0x%x)\n",
                   ipmc_index, port, if_count,
                   if_array[0], if_array[1], if_array[2]);
        } else {
            printk("Failure(%s) Set of egress interface for "
                   "flow with ipmc-index(0x%x) on port(%d), "
                   "if-count(%d) intf-id(0x%x, 0x%x, 0x%x)\n",
                   bcm_errmsg(rv), ipmc_index, port,
                   if_count,
                   if_array[0], if_array[1], if_array[2]);
            
            return CMD_FAIL;
        }
        
    } else {
        printk("invalid sub-command\n");
        return CMD_FAIL;
    }
    
    return CMD_OK;
}

cmd_result_t
cmd_sbx_ipmc(int unit, args_t *args)
{
    return subcommand_execute(unit,
                              args, 
                              _cmd_sbx_ipmc_list,
                              COUNTOF(_cmd_sbx_ipmc_list));
}

/*-----------------------------------------------------------
  TEST RUN:: Convert to tcl script and work with packets(TBD)
 
  pbmp ge0, ge12, cpu
  
  vlan create 2 PortBitMap=0x000000000000000080001001
  vlan create 3 PortBitMap=0x000000000000000080001001
  vlan create 4 PortBitMap=0x000000000000000080001001
  vlan create 5 PortBitMap=0x000000000000000080001001
  vlan create 6 PortBitMap=0x000000000000000080001001

  l3 intf add SMAC=00:00:22:22:22:22 vid=0x2 ttl=1
  l3 intf add SMAC=00:00:33:33:33:33 vid=0x3 ttl=1
  l3 intf add SMAC=00:00:44:44:44:44 vid=0x4 ttl=1
  l3 intf add SMAC=00:00:55:55:55:55 vid=0x5 ttl=1
  l3 intf add SMAC=00:00:66:66:66:66 vid=0x6 ttl=1

  [0xca000020 0xca000021 0xca000022 0xca000023 0xca000024]
  ipmc sg-flow add s=2.2.2.2 g=224.1.1.1 vid=2 ipmc-index=4096
  ipmc sg-flow add s=3.3.3.3 g=224.1.1.1 vid=3 ipmc-index=4097
  ipmc sg-flow add s=4.4.4.4 g=224.1.1.1 vid=4 ipmc-index=4098
  ipmc sg-flow add s=5.5.5.5 g=224.1.1.1 vid=5 ipmc-index=4099
  ipmc sg-flow add s=6.6.6.6 g=224.1.1.1 vid=6 ipmc-index=4100

  ipmc sg-flow get ipmc-index=4096
  ipmc sg-flow get ipmc-index=4097
  ipmc sg-flow get ipmc-index=4098
  ipmc sg-flow get ipmc-index=4099
  ipmc sg-flow get ipmc-index=4100

  ipmc sg-flow find ipmc-index=4096
  ipmc sg-flow find s=2.2.2.2 g=224.1.1.1 vid=2

  #add egress
  ipmc egr-intf add ipmc-index=4096 port=ge0 untag=0 intf-id=0xca000021
  ipmc egr-intf get ipmc-index=4096 port=ge0
  ipmc egr-intf get ipmc-index=4096 port=ge1

  #add duplicate intf
  ipmc egr-intf add ipmc-index=4096 port=ge0 untag=0 intf-id=0xca000021

  #delete egress with 1
  ipmc egr-intf delete ipmc-index=4096 port=ge0 intf-id=0xca000021
  ipmc egr-intf get ipmc-index=4096 port=ge0

  # delete egress when > 1 egr present
  ipmc egr-intf add ipmc-index=4096 port=ge0 untag=0 intf-id=0xca000021
  ipmc egr-intf add ipmc-index=4096 port=ge0 untag=0 intf-id=0xca000022
  ipmc egr-intf add ipmc-index=4096 port=ge0 untag=0 intf-id=0xca000023
  ipmc egr-intf get ipmc-index=4096 port=ge0
  ipmc egr-intf delete ipmc-index=4096 port=ge0 intf-id=0xca000022
  ipmc egr-intf get ipmc-index=4096 port=ge0
  ipmc egr-intf delete ipmc-index=4096 port=ge0 intf-id=0xca000023
  ipmc egr-intf get ipmc-index=4096 port=ge0
  ipmc egr-intf delete ipmc-index=4096 port=ge0 intf-id=0xca000021
  ipmc egr-intf get ipmc-index=4096 port=ge0

  # delete when intf not present
  ipmc egr-intf delete ipmc-index=4096 port=ge0 intf-id=0xca000021

  # delete-all when egr is null
  ipmc egr-intf delete-all ipmc-index=4096 port=ge0

  # delete-all when egr is 1
  ipmc egr-intf add ipmc-index=4096 port=ge0 untag=0 intf-id=0xca000021
  ipmc egr-intf get ipmc-index=4096 port=ge0
  ipmc egr-intf delete-all ipmc-index=4096 port=ge0
  ipmc egr-intf get ipmc-index=4096 port=ge0

  # delete-all when egr is > 1
  ipmc egr-intf add ipmc-index=4096 port=ge0 untag=0 intf-id=0xca000021
  ipmc egr-intf add ipmc-index=4096 port=ge0 untag=0 intf-id=0xca000022
  ipmc egr-intf add ipmc-index=4096 port=ge12 untag=0 intf-id=0xca000022
  ipmc egr-intf get ipmc-index=4096 port=ge0
  ipmc egr-intf get ipmc-index=4096 port=ge12
  ipmc egr-intf delete-all ipmc-index=4096 port=ge0
  ipmc egr-intf get ipmc-index=4096 port=ge0
  ipmc egr-intf get ipmc-index=4096 port=ge12
  ipmc egr-intf delete-all ipmc-index=4096 port=ge12
  ipmc egr-intf get ipmc-index=4096 port=ge0
  ipmc egr-intf get ipmc-index=4096 port=ge12

  # delete 1 intf when egr > 1
  ipmc egr-intf add ipmc-index=4096 port=ge0 untag=0 intf-id=0xca000021
  ipmc egr-intf add ipmc-index=4096 port=ge0 untag=0 intf-id=0xca000022
  ipmc egr-intf add ipmc-index=4096 port=ge0 untag=0 intf-id=0xca000023
  ipmc egr-intf get ipmc-index=4096 port=ge0
  ipmc egr-intf delete ipmc-index=4096 port=ge0 intf-id=0xca000022
  ipmc egr-intf get ipmc-index=4096 port=ge0
  ipmc egr-intf delete ipmc-index=4096 port=ge0 intf-id=0xca000023
  ipmc egr-intf get ipmc-index=4096 port=ge0
  ipmc egr-intf delete ipmc-index=4096 port=ge0 intf-id=0xca000021
  ipmc egr-intf get ipmc-index=4096 port=ge0

  # remove without egr
  ipmc sg-flow remove ipmc-index=4096
  ipmc sg-flow get ipmc-index=4096
  ipmc sg-flow add s=2.2.2.2 g=224.1.1.1 vid=2 ipmc-index=4096

  # remove with 1 egr 
  ipmc egr-intf add ipmc-index=4096 port=ge0 untag=0 intf-id=0xca000021
  ipmc egr-intf get ipmc-index=4096 port=ge0
  ipmc sg-flow remove ipmc-index=4096
  ipmc sg-flow get ipmc-index=4096
  ipmc sg-flow add s=2.2.2.2 g=224.1.1.1 vid=2 ipmc-index=4096
  ipmc sg-flow get ipmc-index=4096
  ipmc egr-intf get ipmc-index=4096 port=ge0

  # remove with > 1 egr 
  ipmc egr-intf add ipmc-index=4096 port=ge0 untag=0 intf-id=0xca000021
  ipmc egr-intf add ipmc-index=4096 port=ge0 untag=0 intf-id=0xca000022
  ipmc egr-intf add ipmc-index=4096 port=ge0 untag=0 intf-id=0xca000023
  ipmc egr-intf get ipmc-index=4096 port=ge0
  ipmc sg-flow remove ipmc-index=4096
  ipmc sg-flow get ipmc-index=4096
  ipmc egr-intf get ipmc-index=4096 port=ge0
  ipmc sg-flow add s=2.2.2.2 g=224.1.1.1 vid=2 ipmc-index=4096
  ipmc sg-flow get ipmc-index=4096
  ipmc egr-intf get ipmc-index=4096 port=ge0

  # egress_set
  ipmc egr-intf add ipmc-index=4096 port=ge0 untag=0 intf-id=0xca000021
  ipmc egr-intf add ipmc-index=4096 port=ge0 untag=0 intf-id=0xca000022
  ipmc egr-intf add ipmc-index=4096 port=ge0 untag=0 intf-id=0xca000023
  ipmc egr-intf add ipmc-index=4096 port=ge12 untag=0 intf-id=0xca000021
  ipmc egr-intf add ipmc-index=4096 port=ge12 untag=0 intf-id=0xca000022
  ipmc egr-intf get ipmc-index=4096 port=ge0
  ipmc egr-intf get ipmc-index=4096 port=ge12
  ipmc egr-intf set ipmc-index=4096 port=ge0 if-count=3 if1=0xca000021 if2=0xca000024 if3=0xca000020
  ipmc egr-intf get ipmc-index=4096 port=ge0
  ipmc egr-intf get ipmc-index=4096 port=ge12

  # final removal of all s,g state
  ipmc sg-flow delete-all ipmc-index=0
  ipmc sg-flow get ipmc-index=4096
  ipmc sg-flow get ipmc-index=4097
  ipmc sg-flow get ipmc-index=4098
  ipmc sg-flow get ipmc-index=4099
  ipmc sg-flow get ipmc-index=4100

  *-------------------------------------------------------*/
