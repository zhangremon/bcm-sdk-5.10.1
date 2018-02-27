/*
 * $Id: trunk.c 1.9.32.1 Broadcom SDK $
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
#include <bcm/trunk.h>
#include <bcm/port.h>
#include <bcm/stack.h>
#include <bcm/debug.h>
#include <bcm/switch.h>

#include <bcm_int/robo/trunk.h>

/* NOTE:
 * Using PBMP in the add/remove makes the commands useless in stacking
 * where the stack port has to be added multiple times
 */

static char *pscnames[] = BCM_TRUNK_PSC_NAMES_INITIALIZER;

cmd_result_t
if_robo_trunk(int unit, args_t *a)
{
    char *subcmd, *c;
    bcm_trunk_t tid = -1;
    pbmp_t pbmp;
    int r;
    int psc;
    static pbmp_t arg_pbmp;
    static uint32 arg_rtag=0;
    static uint32 arg_vlan=VLAN_ID_DEFAULT, arg_tid=0;
    parse_table_t pt;
    cmd_result_t ret_code;
    bcm_trunk_add_info_t t_add_info;
    sal_mac_addr_t mac;
    vlan_id_t vid;
    char *s;
    int i, j;

    if (!sh_check_attached(ARG_CMD(a), unit)) {
    return CMD_FAIL;
    }

    if ((subcmd = ARG_GET(a)) == NULL) {
    return CMD_USAGE;
    }

    sal_memset(&t_add_info, 0, sizeof (t_add_info));

    if (sal_strcasecmp(subcmd, "init") == 0) {
        r = bcm_trunk_init(unit);
        if (r < 0) {
            printk("%s: trunk init failed: %s\n", ARG_CMD(a), bcm_errmsg(r));
            return CMD_FAIL;
        }

	return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "deinit") == 0) {
        r = bcm_trunk_detach(unit);
        if (r < 0) {
            printk("%s: trunk deinit failed: %s\n", ARG_CMD(a), bcm_errmsg(r));
            return CMD_FAIL;
        }

        return CMD_OK;
    }

    /* trunk show command */
    if (sal_strcasecmp(subcmd, "show") == 0) {
        bcm_trunk_chip_info_t ti;
        int tid_min, tid_max, found, isGport = 0;
    
        if ((r = bcm_trunk_chip_info_get(unit, &ti)) < 0) {
            printk("%s: %s\n", ARG_CMD(a), bcm_errmsg(r));
            return CMD_FAIL;
        }
    
        tid_min = ti.trunk_id_min;
        tid_max = ti.trunk_id_max;
    
        if ((c = ARG_CUR(a)) != NULL) {
            parse_table_init(unit, &pt);
                parse_table_add(&pt, "Id", PQ_DFL|PQ_INT, 0, &arg_tid, NULL);
            if (!parseEndOk( a, &pt, &ret_code)) {
                return ret_code;
            }
            tid_min = tid_max = arg_tid;
        }

        r = bcm_switch_control_get(unit, bcmSwitchUseGport, &isGport);
        if (r < 0) {
            printk("%s: bcm_switch_control_get (bcmSwitchUseGport) failed: %s\n",
                ARG_CMD(a), bcm_errmsg(r));
            return CMD_FAIL;
        }
    
        found = 0;
        for (tid = tid_min; tid <= tid_max; tid++) {
            r = bcm_trunk_get(unit, tid, &t_add_info);
    
            if (r == BCM_E_NOT_FOUND) {
                /* Only show existing trunk groups */
                continue;
            }
    
            if (r < 0) {
                printk("%s: trunk %d get failed: %s\n",
                       ARG_CMD(a), tid, bcm_errmsg(r));
                return CMD_FAIL;
            }

            if (isGport) {
                r = _bcm_robo_trunk_gport_resolve(unit, &t_add_info);
                if (r < 0) {
                    printk("%s: _bcm_robo_trunk_gport_resolve failed: %s\n",
                        ARG_CMD(a), bcm_errmsg(r));
                    return CMD_FAIL;
                }
            }

            found += 1;
            printk("trunk %d: %d ports=",
            tid, t_add_info.num_ports);
    
            for (i = 0; i < t_add_info.num_ports; i++) 
            {
                if (!SOC_PORT_VALID(unit, t_add_info.tp[i])) {
                    printk("invalid port!\n");
                    return CMD_FAIL;
                }
                printk("%s%s",
                i == 0 ? "" : ",",
                SOC_PORT_NAME(unit, t_add_info.tp[i]));
            }
            if ((t_add_info.psc & _BCM_TRUNK_PSC_VALID_VAL) <= 0 || 
                (t_add_info.psc & _BCM_TRUNK_PSC_VALID_VAL) >= 7) 
            {
                s= "unknown";
            } else 
            {
                s = pscnames[(t_add_info.psc & _BCM_TRUNK_PSC_VALID_VAL)];
            }
            printk(" psc/rtag=%s (%d or'ed 0x%x)\n", 
                s, 
                (t_add_info.psc & _BCM_TRUNK_PSC_VALID_VAL), 
                (t_add_info.psc & ~_BCM_TRUNK_PSC_VALID_VAL));
    
        }
        if (found == 0) {
            printk("[no matching trunks defined]\n");
        }
    
        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "add") == 0) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Id",   PQ_DFL|PQ_INT,  0, &arg_tid,  NULL);
        parse_table_add(&pt, "Rtag", PQ_DFL|PQ_INT,  0, &arg_rtag, NULL);
        parse_table_add(&pt, "Pbmp", PQ_DFL|PQ_PBMP, 0, &arg_pbmp, NULL);
        if (!parseEndOk(a, &pt, &ret_code)) {
           return ret_code;
        }

        tid = arg_tid;
        psc = arg_rtag;
        pbmp = arg_pbmp;
    
        r = bcm_trunk_get(unit, tid, &t_add_info);
        if(r>=0) {  /* not first creat */
            j=t_add_info.num_ports;
            SOC_PBMP_ITER(pbmp, i) {
                t_add_info.tp[j] = i;
                j++;
            };
            t_add_info.num_ports = j;
        }  
        else {/* first creat */
            j = 0;
            SOC_PBMP_ITER(pbmp, i) { 
                t_add_info.tp[j++] = i;
            };
    
            t_add_info.num_ports = j;   
            t_add_info.psc = psc;     	
            bcm_trunk_create_with_tid(unit, tid);        	
        }    
                
        r = bcm_trunk_set(unit, tid, &t_add_info);     
    } else if (sal_strcasecmp(subcmd, "remove") == 0) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Id", PQ_DFL|PQ_INT, 0, &arg_tid, NULL);
        parse_table_add(&pt, "Pbmp", PQ_DFL|PQ_PBMP, \
                       (void *)(0), &arg_pbmp, NULL);
        if (!parseEndOk( a, &pt, &ret_code)) {
            return ret_code;
        }

        tid = arg_tid;
        r = bcm_trunk_get(unit, tid, &t_add_info);

        if (r >= 0) {
            SOC_PBMP_CLEAR(pbmp);
            for (i = 0; i < t_add_info.num_ports; i++) { 
                SOC_PBMP_PORT_ADD(pbmp, t_add_info.tp[i]);
            }
       
            SOC_PBMP_REMOVE(pbmp, arg_pbmp);
            

            j = 0;
            SOC_PBMP_ITER(pbmp, i) {
            	/*
                 * Check Me : The module id should be get from API.
                 *            But bcm_stk_my_modid_get() is stack API and not 
                 *            designed yet!
                 *            Modify the following line if API ready.
            	 */
                t_add_info.tm[j] = 0;	
                t_add_info.tp[j++] = i;
            }
            t_add_info.num_ports = j;
     
            if (j) {
                r = bcm_trunk_set(unit, tid, &t_add_info);
            } else {
                r = bcm_trunk_destroy(unit, tid);
            }
        }
    } else if (sal_strcasecmp(subcmd, "mcast") == 0) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Id", PQ_DFL|PQ_INT, 0, &arg_tid, NULL);
        parse_table_add(&pt, "Mac", PQ_DFL|PQ_MAC, 0, (void *)mac, NULL);
        parse_table_add(&pt, "Vlanid", 	PQ_DFL|PQ_HEX, 0, &arg_vlan, NULL);
        if (!parseEndOk( a, &pt, &ret_code)) {
            return ret_code;
        }

        tid = arg_tid;
        vid = arg_vlan;
        r = bcm_trunk_mcast_join(unit, tid, vid, mac);
    } else if (sal_strcasecmp(subcmd, "psc") == 0) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Id",   PQ_DFL|PQ_INT,  0, &arg_tid,  NULL);
        parse_table_add(&pt, "Rtag", PQ_DFL|PQ_INT,  0, &arg_rtag, NULL);
        if (!parseEndOk(a, &pt, &ret_code)) {
            return ret_code;
        }

        tid = arg_tid;
        psc = arg_rtag;

        r = bcm_trunk_psc_set(unit, tid, psc);
    } else {
        return CMD_USAGE;
    }

    if (r < 0) {
        printk("%s: ERROR: %s\n", ARG_CMD(a), bcm_errmsg(r));
        return CMD_FAIL;
    }

    return CMD_OK;
}
