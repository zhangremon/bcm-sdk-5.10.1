/*
 * $Id: mirror.c 1.1 Broadcom SDK $
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
 * Mirror CLI commands
 */

#include <appl/diag/system.h>
#include <appl/diag/parse.h>
#include <appl/diag/dport.h>


#include <soc/drv.h>
#include <soc/sbx/sbx_drv.h>
#include <bcm/error.h>
#include <bcm/mirror.h>
#include <bcm/stack.h>
#include <bcm/debug.h>

char cmd_sbx_mirror_usage[] = 
"Usage:\n"
" MIRror <port> [mode=off|none|ingress|egress|both] [[dest=mod.port]|port=port]\n\n"
"Installs an ingress and/or egress mirror on a <port>.  If a mirror exists on\n"
"the <port>, then the existing mirror will be removed and replaced by the\n"
"one specified.\nIf only <port> is supplied, the configured mirror is diplayed.\n"
"\n";

cmd_result_t
cmd_sbx_mirror(int unit, args_t *args)
{

    enum { 
        mirrorModeOff, mirrorModeNone,
        mirrorModeIngress, mirrorModeEgress, mirrorModeBoth,
        mirrorModeCount
    };

    char          *str;
    int            mode_arg, rv;
    uint32_t       flags;
    bcm_port_t     mirror_port;
    bcm_mod_port_t modport_dest;
    parse_table_t  pt;
    cmd_result_t   ret_code;
    char          *mode_list[mirrorModeCount+1];

    mode_list[mirrorModeOff]     = "Off";
    mode_list[mirrorModeNone]    = "None";
    mode_list[mirrorModeIngress] = "Ingress";
    mode_list[mirrorModeEgress]  = "Egress";
    mode_list[mirrorModeBoth]    = "Both";
    mode_list[mirrorModeCount]   = NULL;
    
    flags             = 0;
    mode_arg          = -1;
    modport_dest.mod  = BCM_MODID_INVALID;
    modport_dest.port = -1;

    if (SOC_IS_SBX_FE2000(unit) == FALSE) {
        printk("Supported only by BCM8802x devices\n");
        return CMD_USAGE;
    }

    if (ARG_CNT(args) == 0) {
        return CMD_USAGE;
    }

    /* Get the first argument, it must be the port to mirror */
    str = ARG_GET(args);
    if (!isint(str)) {
        printk("'%s' is not a valid port\n", str);
        return CMD_USAGE;
    }

    mirror_port = parse_integer(str);

    if (SOC_PORT_VALID(unit, mirror_port) == FALSE) {
        printk("Invalid mirror port specified: %d\n", mirror_port);
        return CMD_USAGE;
    }

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "dest", PQ_DFL | PQ_MOD_PORT, 0, &modport_dest, NULL);
    parse_table_add(&pt, "port", PQ_DFL | PQ_INT, 0, &modport_dest.port, NULL);
    parse_table_add(&pt, "mode", PQ_DFL | PQ_MULTI, 0, &mode_arg, mode_list);

    /* No more args?  Show the state of the mirror port, else configure it */
    if (ARG_CNT(args) == 0) {
        bcm_port_t     dest_port;
        bcm_module_t   dest_mod;

        rv = bcm_mirror_port_get(unit, mirror_port, 
                                 &dest_mod, &dest_port, &flags);
        if (BCM_FAILURE(rv)) {
            printk("Failed to get mirror port info: %s\n", bcm_errmsg(rv));
            return CMD_FAIL;
        }

        if (flags & BCM_MIRROR_PORT_ENABLE) {
            char *dir = NULL;

            if (flags & BCM_MIRROR_PORT_INGRESS) {
                dir = "Ingress";
            }

            if (flags & BCM_MIRROR_PORT_EGRESS) {
                if (dir == NULL) {
                    dir = "Egress";
                } else {
                    dir = "Ingress/Egress";
                }
            }

            printk("%s Mirror enabled on port %d to modport %d.%d\n", 
                   dir, mirror_port, dest_mod, dest_port);
            
        } else {
            printk("Port %d has no mirrors enabled\n", mirror_port);
        }

        return CMD_OK;
    }

    if (!parseEndOk(args, &pt, &ret_code)) {
        return ret_code;
    }

    /* Configure the mirror(s) */

    flags = BCM_MIRROR_PORT_ENABLE;
    switch (mode_arg) 
    {
    case mirrorModeOff:      /* fall thru intentional */
    case mirrorModeNone:     flags &= ~BCM_MIRROR_PORT_ENABLE;  break;
    case mirrorModeIngress:  flags |=  BCM_MIRROR_PORT_INGRESS; break;
    case mirrorModeEgress:   flags |=  BCM_MIRROR_PORT_EGRESS;  break;
    case mirrorModeBoth:
        flags |= BCM_MIRROR_PORT_INGRESS | BCM_MIRROR_PORT_EGRESS;
        break;
    default:
        return CMD_FAIL;
    }

    /* Validate the mod/port only if the mirror is enabled  */
    if (flags & BCM_MIRROR_PORT_ENABLE) {
        if (SOC_PORT_VALID(unit, modport_dest.port) == FALSE) {
            printk("Invalid destination port specified: %d\n", 
                   modport_dest.port);
            return CMD_USAGE;
        }
        
        if (modport_dest.mod == BCM_MODID_INVALID) {
            
            rv = bcm_stk_modid_get(unit, &modport_dest.mod);
            if (BCM_FAILURE(rv)) {
                printk("Failed to get modid: %s\n", bcm_errmsg(rv));
                return CMD_FAIL;
            }
        }
    }

    rv = bcm_mirror_port_set(unit, mirror_port, 
                             modport_dest.mod, modport_dest.port, flags);
    if (BCM_FAILURE(rv)) {
        printk("Failed to set mirror port: %s\n", bcm_errmsg(rv));
        return CMD_FAIL;
    }

    if (flags & BCM_MIRROR_PORT_ENABLE) {
        printk("Installed mirror on port %d to mod/port %d/%d\n",
               mirror_port, modport_dest.mod, modport_dest.port);
    } else {
        printk("Removed mirror(s) from port %d\n", mirror_port);
    }

    return CMD_OK;
}
