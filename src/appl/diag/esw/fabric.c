/*
 * $Id: fabric.c 1.9 Broadcom SDK $
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
 * Fabric and Higig related CLI commands
 */

#include <appl/diag/system.h>
#include <appl/diag/parse.h>
#include <appl/diag/dport.h>

#include <bcm/error.h>
#include <bcm/dmux.h>
#include <bcm/stack.h>
#include <bcm/debug.h>

#ifdef BCM_XGS_SUPPORT
#include <soc/higig.h>
#endif

#ifdef	BCM_XGS_SUPPORT
#define PACK_LONG(buf, val) \
    do {                                               \
        uint32 v2;                                     \
        v2 = bcm_htonl(val);                           \
        sal_memcpy(buf, &v2, sizeof(uint32));          \
    } while (0)

char if_h2higig_usage[] =
    "H2Higig [<word0> <word1> <word2>]:  Convert hex to higig info\n";

cmd_result_t
if_h2higig(int unit, args_t *args)
{
    uint32 val;
    char *arg;
    soc_higig_hdr_t hghdr;
    int i;

    sal_memset(&hghdr, 0, sizeof(soc_higig_hdr_t));
    for (i = 0; i < 3; i++) {
        if ((arg = ARG_GET(args)) == NULL) {
            break;
        }

        val = strtoul(arg, NULL, 0);
        PACK_LONG(&hghdr.overlay0.bytes[i * 4], val);
    }

    soc_higig_dump(unit, "", &hghdr);

    return CMD_OK;
}

#ifdef BCM_HIGIG2_SUPPORT

char if_h2higig2_usage[] =
    "H2Higig2 [<word0> <word1> <word2> <word3> [<word4>]]:  Convert hex to higig2 info\n";

cmd_result_t
if_h2higig2(int unit, args_t *args)
{
    uint32 val;
    char *arg;
    soc_higig2_hdr_t hghdr;
    int i;

    sal_memset(&hghdr, 0, sizeof(soc_higig2_hdr_t));
    for (i = 0; i < 4; i++) {
        if ((arg = ARG_GET(args)) == NULL) {
            break;
        }

        val = strtoul(arg, NULL, 0);
        PACK_LONG(&hghdr.overlay0.bytes[i * 4], val);
    }

    soc_higig2_dump(unit, "", &hghdr);

    /* Decode extension header if present */
    if ((arg = ARG_GET(args)) != NULL) {
        int eh_type;
        val = strtoul(arg, NULL, 0);
        eh_type = val >> 28;
        printk("0x%08x <EHT=%d", val, eh_type);
        if (eh_type == 0) {
            printk(" TM=%d", (val >> 24) & 0x1);
            printk(" SEG_SEL=%d", (val >> 21) & 0x7);
            printk(" TAG_TYPE=%d", (val >> 19) & 0x3);
            printk(" QTAG=0x%04x", val & 0xffff);
        }
        printk(">\n");
    }

    return CMD_OK;
}
#endif /* BCM_HIGIG2_SUPPORT */

#endif	/* BCM_XGS_SUPPORT */

#ifdef BCM_DMUX_SUPPORT

char if_dmux_usage[] =
    "Usages:\n\t"
#ifdef COMPILER_STRING_CONST_LIMIT
    "  dmux <option> [args...]\n"
#else
    "  dmux init\n\t"
    "  dmux detach\n\t"
    "  dmux add <Port=port> <DestMod=val> <DestPort=val>\n\t"
    "       - Add dest (module,port) pair to dmux list for port.\n\t"
    "  dmux delete <Port=port> <DestMod=val> <DestPort=val>\n\t"
    "       - Remove dest (module,port) pair from dmux list for port.\n\t"
    "  dmux clear <Port=port>\n\t"
    "       - Clear dmux list for port.\n\t"
    "  dmux show <Port=port> \n\t"
    "       - Display dmux list for port.\n\t"
    "  dmux config <pbmp> [[+/-]flags] \n\t"
    "       - Set or clear dmux packet parameter selections for port(s).\n\t"
    "         +option adds to list of output,\n\t"
    "         -option removes from list, no +/- toggles current state.\n\t"
    "         If no options given, a current list is printed out\n"
#endif
    ;

/* Must by in sync with config flags in bcm/dmux.h */
char *dmux_names[12] = {
    "L4DS",    /* TCP/UDP destination socket */
    "L4SS",    /* TCP/UDP source socket */
    "IPDA",    /* Destination IP address */
    "IPSA",    /* Source IP address */
    "IPVID",   /* IP: VLAN ID */
    "IPTYPE",  /* IP: Etherype */
    "IPMACDA", /* IP: MAC source address */
    "IPMACSA", /* IP: MAC destination address */
    "VID",     /* Non-IP: VLAN ID */
    "TYPE",    /* Non-IP: Etherype */
    "MACDA",   /* Non-IP: MAC source address */
    "MACSA"    /* Non-IP: MAC dest address */
};

cmd_result_t
if_dmux(int unit, args_t *a)
{
    static parse_pm_t	dmux_map[12 + 3];
    static int		dmux_map_initted;
    static uint32       arg_dest_mod = 0, arg_dest_port = 0;
    static bcm_port_t   arg_port = 0;
    bcm_port_t          port, dport;
    char 		*c, *subcmd;
    int                 i, r = 0;
    parse_table_t	pt;
    cmd_result_t	retCode;

    if (!sh_check_attached(ARG_CMD(a), unit)) {
	return CMD_FAIL;
    }

    if ((subcmd = ARG_GET(a)) == NULL) {
	return CMD_USAGE;
    }

    if (sal_strcasecmp(subcmd, "config") == 0) {
        uint32          dmux_config;
        bcm_pbmp_t      pbmp, cmp_pbmp;
        bcm_port_config_t pcfg;
        char            *argpbm;

        BCM_IF_ERROR_RETURN(bcm_port_config_get(unit, &pcfg));

        if (!dmux_map_initted) {
            int		p;

            for (p = 0, i = 0; i < 12; i++) {
                if ((c = dmux_names[i]) != NULL) {
                    dmux_map[p].pm_s = c;
                    dmux_map[p++].pm_value = (uint32) 1 << i;
                }
            }

            dmux_map[p].pm_s = "@ALL";
            dmux_map[p++].pm_value = ~0;

            dmux_map[p].pm_s = "@*";
            dmux_map[p++].pm_value = ~0;

            dmux_map[p].pm_s = NULL;
            dmux_map[p].pm_value = 0;

            assert(p < COUNTOF(dmux_map));

            dmux_map_initted = 1;
        }

        if ((argpbm = ARG_GET(a)) == NULL) {
            pbmp = pcfg.xe;
        } else {
            if (parse_bcm_pbmp(unit, argpbm, &pbmp) < 0) {
                printk("%s: Error: unrecognized port bitmap: %s\n",
                       ARG_CMD(a), argpbm);
                return CMD_FAIL;
            }

            BCM_PBMP_ASSIGN(cmp_pbmp, pbmp);
            BCM_PBMP_AND(cmp_pbmp, pcfg.xe);
            if (BCM_PBMP_NEQ(cmp_pbmp, pbmp)) {
                printk("%s: Error: Unsupported ports in bitmap: %s\n",
                       ARG_CMD(a), argpbm);
                return CMD_FAIL;
            }
        }

        DPORT_BCM_PBMP_ITER(unit, pbmp, dport, port) {
            if ((r = bcm_dmux_config_get(unit, port, &dmux_config)) < 0) {
                printk("%s: Error: Cannot retrieve DMUX config for port %s\n",
                       ARG_CMD(a), BCM_PORT_NAME(unit, port));
                return CMD_FAIL;
            }

            if (!ARG_CNT(a)) {			/* No args - print settings */
                /*
                 * No parameters.  Display current status as a list of enabled
                 * demux params followed by list of disabled demux params.
		 */
                printk("Demux uses the following packet parameters:\n\n");
                parse_mask_format(50, dmux_map, dmux_config);
                printk("\nDemux ignores the following packet parameters:\n\n");
                parse_mask_format(50, dmux_map, ~dmux_config);
                printk("\n");
                return(CMD_OK);
            }

            /* Set new mode */

            while ((c = ARG_GET(a)) != NULL) {
                if (parse_mask(c, dmux_map, &dmux_config)) {
                    printk("debug: unknown option: %s\n", c);
                    return(CMD_FAIL);
                }
            }

            if ((r = bcm_dmux_config_set(unit, port, dmux_config)) < 0) {
                printk("%s: Error: Cannot set DMUX config for port %s\n",
                       ARG_CMD(a), BCM_PORT_NAME(unit, port));
                return CMD_FAIL;
            }
        }
    } else if (sal_strcasecmp(subcmd, "show") == 0) {
        uint32          module;
        bcm_pbmp_t      pbmp;
	parse_table_init(unit, &pt);
        parse_table_add(&pt, "Port",   PQ_DFL|PQ_PORT|PQ_BCM,
                        0, &arg_port,  NULL);
	if (!parseEndOk(a, &pt, &retCode)) {
	    return retCode;
        }

        printk("Demux destinations for port %s:\n",
               BCM_PORT_NAME(unit, arg_port));
        for (module = 0; module <= SOC_MODID_MAX(unit); module++) {
            r = bcm_dmux_dest_get(unit, arg_port, module, &pbmp);
            if (r < 0) {
                 printk("%s: Error: "
			"Unable to retrieve DMUX settings for port %s\n",
                       ARG_CMD(a), BCM_PORT_NAME(unit, arg_port));
                 return CMD_FAIL;
            }

            BCM_PBMP_ITER(pbmp, port) {
                printk("\t Module %d, Port %d\n", module, port);
            }
        }
    } else if (sal_strcasecmp(subcmd, "add") == 0) {
	parse_table_init(unit, &pt);
        parse_table_add(&pt, "Port", PQ_DFL|PQ_PORT|PQ_BCM, 0,
			&arg_port,  NULL);
        parse_table_add(&pt, "DestMod", PQ_DFL|PQ_INT, 0,
			&arg_dest_mod, NULL);
	parse_table_add(&pt, "DestPort", PQ_DFL|PQ_INT, 0,
			&arg_dest_port, NULL);
	if (!parseEndOk(a, &pt, &retCode)) {
	    return retCode;
        }

	r = bcm_dmux_dest_add(unit, arg_port,
                                    arg_dest_mod, arg_dest_port);
    } else if (sal_strcasecmp(subcmd, "delete") == 0) {
	parse_table_init(unit, &pt);
        parse_table_add(&pt, "Port", PQ_DFL|PQ_PORT|PQ_BCM, 0,
			&arg_port,  NULL);
        parse_table_add(&pt, "DestMod", PQ_DFL|PQ_INT, 0,
			&arg_dest_mod, NULL);
	parse_table_add(&pt, "DestPort", PQ_DFL|PQ_INT, 0,
			&arg_dest_port, NULL);
	if (!parseEndOk(a, &pt, &retCode)) {
	    return retCode;
        }

	r = bcm_dmux_dest_delete(unit, arg_port,
                                    arg_dest_mod, arg_dest_port);
    } else if (sal_strcasecmp(subcmd, "clear") == 0) {
	parse_table_init(unit, &pt);
        parse_table_add(&pt, "Port",   PQ_DFL|PQ_PORT|PQ_BCM,
                        0, &arg_port,  NULL);
	if (!parseEndOk(a, &pt, &retCode)) {
	    return retCode;
        }

	r = bcm_dmux_dest_delete_all(unit, arg_port);
    } else if (sal_strcasecmp(subcmd, "init") == 0) {
	r = bcm_dmux_init(unit);
    } else if (sal_strcasecmp(subcmd, "detach") == 0) {
	r = bcm_dmux_detach(unit);
    }

    if (r < 0) {
	printk("%s: ERROR: %s\n", ARG_CMD(a), bcm_errmsg(r));
	return CMD_FAIL;
    }

    return CMD_OK;
}

#endif  /* BCM_DMUX_SUPPORT */

int _bcm_diag_fabric_not_empty;
