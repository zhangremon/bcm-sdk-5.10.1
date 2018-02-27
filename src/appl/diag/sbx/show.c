/* 
 * $Id: show.c 1.8 Broadcom SDK $
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
 * File:        show.c
 * Purpose:     Show CLI commands
 *
 */

#include <appl/diag/shell.h>
#include <appl/diag/system.h>
#include <appl/diag/parse.h>

#include <soc/sbx/sbx_drv.h>
#include <sal/appl/pci.h>

#include <bcm/error.h>
#include <bcm/debug.h>
#include <bcm/stat.h>

char cmd_sbx_soc_usage[] = 
    "Parameters: [<unit #>] ... \n\t"
    "Print internal SOC driver control information IF compiled as a \n\t"
    "debug version. If not compiled as a debug version, a warning is\n\t"
    "printed and the command completes successfully with no further\n\t"
    "output\n";

cmd_result_t
cmd_sbx_soc(int u, args_t *a)
/*
 * Function:    sh_soc
 * Purpose:     Print soc control information if compiled in debug mode.
 * Parameters:  u - unit #
 *              a - pointer to args, expects <unit> ...., if none passed,
 *                      default unit # used.
 * Returns:     CMD_OK/CMD_FAIL
 */
{
#if defined(BROADCOM_DEBUG)
    char          *c;
    cmd_result_t  rv = CMD_OK;

    if (!sh_check_attached("soc", u)) {
        return(CMD_FAIL);
    }

    if (!ARG_CNT(a)) {
        rv = soc_sbx_dump(u, "  ") ? CMD_FAIL : CMD_OK;
    } else {
        while ((CMD_OK == rv) && (c = ARG_GET(a))) {
            if (!isint(c)) {
                printk("%s: Invalid unit identifier: %s\n", ARG_CMD(a), c);
                rv = CMD_FAIL;
            } else {
                rv = soc_sbx_dump(parse_integer(c), "  ") ?
                    CMD_FAIL : CMD_OK;
            }
        }
    }
    return(rv);
#else /* !defined(BROADCOM_DEBUG) */
    printk("%s: Warning: Not compiled with BROADCOM_DEBUG enabled\n", ARG_CMD(a));
    return(CMD_OK);
#endif /* defined(BROADCOM_DEBUG) */
}

/*
 * Show something to a user.
 */

/*
 * *** ORDER ***
 *
 * The order of these tables must match the switch statements below.
 */
static parse_key_t show_arg[] = {
    "Pci",				/* 0 */
    "Counters",				/* 1 */
    "Errors",				/* 2 */
    "Interrupts",			/* 3 */
    "Chips",				/* 4 */
    "Statistics",			/* 5 */
    "MIB",				/* 6 */
    "Units",                            /* 7 */
};

static int show_arg_cnt = PARSE_ENTRIES(show_arg);

#if defined(BCM_SIRIUS_SUPPORT)
static parse_key_t show_ctr_arg[] = {
    "Changed",				/* 0 */
    "Same",				/* 1 */
    "Zero",				/* 2 */
    "NonZero",				/* 3 */
    "Hex",				/* 4 */
    "Raw",				/* 5 */
    "All",				/* 6 */
    "ErDisc",				/* 7 */
};

static int show_ctr_arg_cnt = PARSE_ENTRIES(show_ctr_arg);
#endif
static char *_stat_names[] = BCM_STAT_NAME_INITIALIZER;

char cmd_sbx_show_usage[] =
    "Usages:\n"
#ifdef COMPILER_STRING_CONST_LIMIT
    "  show <args>\n"
#else
    "  show unit [<unit>]  - Unit list or unit parameters\n"
    "       counter(s)     - Shows TX and RX counters for all E ports\n"
    "       counter <port> - Shows TX and RX counters for a given port\n"
    "       stat [<port>]  - Shows SNMP stats\n"
    "       pci            - Shows PCI bus\n"
#endif
    ;

cmd_result_t
cmd_sbx_show(int unit, args_t *a)
{
    char                *c;
    soc_control_t	*soc = SOC_CONTROL(unit);
    pbmp_t		pbmp;
    bcm_port_t          port;
    bcm_stat_val_t      stat;
    const parse_key_t	*cmd;
    uint64              val64;
    int			rv, flags;
#if defined(BCM_SIRIUS_SUPPORT)
    int                 num=0;
    soc_reg_t		ctr_reg;
    soc_regaddrlist_t	alist;
#endif

    if (!(c = ARG_GET(a))) {    /* Nothing to do    */
        return(CMD_USAGE);      /* Print usage line */
    }
   
    cmd = parse_lookup(c, show_arg, sizeof(show_arg[0]), show_arg_cnt);
    if (!cmd) {
        printk("%s: Error: Invalid option %s\n", ARG_CMD(a), c);
        return(CMD_USAGE);
    }
    
    switch(cmd - show_arg) {
#if !defined(BCM_ICS) && !defined(NO_SAL_APPL)
	case 0:				/* PCI */
	    pci_print_all();
	    break;
#endif
#if defined(BCM_SIRIUS_SUPPORT)
	case 1:				/* Counters */
	    flags = 0;
	    
	    while ((c = ARG_GET(a)) != NULL) {
		cmd = parse_lookup(c,
				   show_ctr_arg, sizeof (show_ctr_arg[0]),
				   show_ctr_arg_cnt);
		
		switch (cmd - show_ctr_arg) {
		    case 0:
			flags |= SHOW_CTR_CHANGED;
			break;
		    case 1:
			flags |= SHOW_CTR_SAME;
			break;
		    case 2:
			flags |= SHOW_CTR_Z;
			break;
		    case 3:
			flags |= SHOW_CTR_NZ;
			break;
		    case 4:
			flags |= SHOW_CTR_HEX;
			break;
		    case 5:
			flags |= SHOW_CTR_RAW;
			break;
		    case 6:
			flags |= (SHOW_CTR_CHANGED | SHOW_CTR_SAME |
				  SHOW_CTR_Z | SHOW_CTR_NZ);
			break;
		    case 7:
			flags |= SHOW_CTR_ED;
			break;
		    default:
			goto break_for;
		}
	    }
	break_for:
	    
	    /*
	     * Supply defaults
	     */
	    
	    if ((flags & (SHOW_CTR_CHANGED | SHOW_CTR_SAME)) == 0) {
		flags |= SHOW_CTR_CHANGED;
	    }
	    
	    if ((flags & (SHOW_CTR_Z | SHOW_CTR_NZ)) == 0) {
		flags |= SHOW_CTR_NZ;
	    }
	    
	    if (c == NULL) {
		ctr_reg = INVALIDr;		/* All registers */
		BCM_PBMP_ASSIGN(pbmp, soc->counter_pbmp);
	    } else if (parse_pbmp(unit, c, &pbmp) >= 0) {
		ctr_reg = INVALIDr;		/* All registers, selected ports */
	    } else {
		int i;
		
		if (soc_regaddrlist_alloc(&alist) < 0) {
		    printk("Could not allocate address list.  Memory error.\n");
		    return CMD_FAIL;
		}
		
		if (parse_symbolic_reference(unit, &alist, c) < 0) {
		    printk("Syntax error parsing \"%s\"\n", c);
		    soc_regaddrlist_free(&alist);
		    return CMD_FAIL;
		}
		
		ctr_reg = alist.ainfo[0].reg;
		
		if (!SOC_REG_IS_COUNTER(unit, ctr_reg)) {
		    printk("%s: Register is not a counter: %s\n", ARG_CMD(a), c);
		    soc_regaddrlist_free(&alist);
		    return(CMD_FAIL);
		}
		
		BCM_PBMP_CLEAR(pbmp);
		
		for (i = 0; i < alist.count; i++) {
		    BCM_PBMP_PORT_ADD(pbmp, alist.ainfo[i].port);
		}
		soc_regaddrlist_free(&alist);
	    }
	    
	    if (soc_feature(unit, soc_feature_cpuport_stat_dma)) {
		pbmp_t temp_pbmp;
		SOC_PBMP_CLEAR(temp_pbmp);
		SOC_PBMP_OR(temp_pbmp, soc->counter_pbmp);
		SOC_PBMP_OR(temp_pbmp, PBMP_CMIC(unit));
		BCM_PBMP_AND(pbmp, temp_pbmp);
	    } else {
		BCM_PBMP_AND(pbmp, soc->counter_pbmp);
	    }

	    SOC_PBMP_COUNT(pbmp, num);
	    if (num == 0)
	      return CMD_FAIL;
	    sbx_do_show_counters(unit, ctr_reg, pbmp, flags);
	    break;
#endif
	case 2:				/* Errors */
	    printk("%s: Errors: SDRAM(%d) CFAP(%d) first-CELL(%d) MMU-SR(%d)\n",
		   ARG_CMD(a),
		   soc->stat.err_sdram, soc->stat.err_cfap,
		   soc->stat.err_fcell, soc->stat.err_sr);
	    printk("%s          Timeout-Schan(%d),MII(%d)\n",
		   ARG_CMD(a), soc->stat.err_sc_tmo, soc->stat.err_mii_tmo);
	    break;
	case 3:				/* Interrupts */
	    printk("%s: Interrupts: Total(%d) Schan-total(%d),error(%d) "
		   "Link(%d)\n",
		   ARG_CMD(a), soc->stat.intr, soc->stat.intr_sc,
		   soc->stat.intr_sce, soc->stat.intr_ls);
	    printk("%s:             GBP-full(%d) PCI-fatal(%d),parity(%d)\n",
		   ARG_CMD(a), soc->stat.intr_gbp, soc->stat.intr_pci_fe,
		   soc->stat.intr_pci_pe);
	    
	    if (soc_feature(unit, soc_feature_tslam_dma)) {
		printk("%s:             Table-DMA(%d),TSLAM DMA(%d)\n",
		       ARG_CMD(a),
		       soc->stat.intr_tdma, soc->stat.intr_tslam);
	    }
	    printk("%s:             I2C(%d),MII(%d),Stats(%d),"
		   "Desc(%d),Chain(%d)\n",
		   ARG_CMD(a),
		   soc->stat.intr_i2c,
		   soc->stat.intr_mii,
		   soc->stat.intr_stats,
		   soc->stat.intr_desc,
		   soc->stat.intr_chain);
	    break;
	case 4:				/* Chips */
	    printk("Known chips:\n");
	    soc_cm_display_known_devices();
	    break;
	case 5:				/* Statistics */
	case 6:				/* MIB */
	    flags = 0;
	    
	    if ((c = ARG_GET(a)) != NULL) {    /* Ports specified? */
		if (parse_pbmp(unit, c, &pbmp)) {
		    printk("%s: Invalid ports: %s\n", ARG_CMD(a), c);
		    return CMD_FAIL;
		}
		if ((c = ARG_GET(a)) != NULL && sal_strcasecmp(c, "all") == 0) {
		    flags = 1;
		}
	    } else {
		BCM_PBMP_ASSIGN(pbmp, PBMP_E_ALL(unit));
	    }
	    
	    PBMP_ITER(pbmp, port) {
		printk("%s: Statistics for Unit %d port %s\n",
		       ARG_CMD(a), unit, SOC_PORT_NAME(unit, port));
		
		bcm_stat_sync(unit);
		
		for (stat = 0; stat < snmpValCount; stat++) {
		    if (!_stat_names[stat]) {
			continue;
		    }
		    
		    rv = bcm_stat_get(unit, port, stat, &val64);
		    if (rv == BCM_E_UNAVAIL) {
			continue;
		    }
		    if (BCM_FAILURE(rv)) {
			printk("%8s\t%s (stat %d): %s\n",
			       "-", _stat_names[stat], stat, bcm_errmsg(rv));
			continue;
		    }
		    
		    if (flags == 0 && COMPILER_64_IS_ZERO(val64)) {
			continue;
		    }
		    
		    if (COMPILER_64_HI(val64) == 0) {
			printk("%8u\t%s (stat %d)\n",
			       COMPILER_64_LO(val64), _stat_names[stat], stat);
		    } else {
			printk("0x%08x%08x\t%s (stat %d)\n",
			       COMPILER_64_HI(val64),
			       COMPILER_64_LO(val64),
			       _stat_names[stat], stat);
		    }
		}
	    }
	    break;
	case 7:				/* units */
	    c = ARG_GET(a);
	    if (c != NULL) {    /* specific unit */
		rv = sal_ctoi(c, 0);
		if (!SOC_UNIT_VALID(rv)) {
		    printk("Unit %d is not valid\n", rv);
		    return CMD_FAIL;
		}
		soc = SOC_CONTROL(rv);
		if (!(soc->soc_flags & SOC_F_ATTACHED)) {
		    printk("Unit %d (detached)\n", rv);
		    return CMD_OK;
		}
		printk("Unit %d chip %s%s\n",
		       rv,
		       soc_dev_name(rv),
		       rv == unit ? " (current)" : "");
		soc_sbx_chip_dump(rv, SOC_DRIVER(rv));
	    } else {        /* all units */
		for (rv = 0; rv < soc_ndev; rv++) {
		    if (!SOC_UNIT_VALID(rv)) {
			continue;
		    }
		    soc = SOC_CONTROL(rv);
		    printk("Unit %d chip %s%s\n",
			   rv,
			   soc_dev_name(rv),
			   rv == unit ? " (current)" : "");
		}
	    }
	    break;
	default:
	    return(CMD_FAIL);
    }
    
    return (CMD_OK);
}


STATIC cmd_result_t
_cmd_sbx_clear_stat(int unit, args_t *args)
{
    char        *c;
    pbmp_t      pbmp;
    bcm_port_t  port;
    int         rv;

    if ((c = ARG_GET(args)) != NULL) {    /* Ports specified? */
        if (parse_pbmp(unit, c, &pbmp)) {
            printk("%s: Invalid ports: %s\n", ARG_CMD(args), c);
            return CMD_FAIL;
        }
    } else {
        BCM_PBMP_ASSIGN(pbmp, PBMP_E_ALL(unit));
    }

    PBMP_ITER(pbmp, port) {
        if ((rv = bcm_stat_clear(unit, port)) != BCM_E_NONE) {
            printk("%s: Unit %d Port %d failed to clear stats: %s\n",
                   ARG_CMD(args), unit, port, bcm_errmsg(rv));
            return CMD_FAIL;
        }
    }

    return CMD_OK;
}


char cmd_sbx_clear_usage[] =
    "Usages:\n"
#ifdef COMPILER_STRING_CONST_LIMIT
    "  clear <args>\n"
#else
    "  counter(s)            - Shows TX and RX counters for all E ports\n"
    "  counter <port>        - Shows TX and RX counters for a given port\n"
#if 0
    "  clear stat [<port>]   - Clears SNMP stats\n"
#endif
#endif
    ;

cmd_result_t
cmd_sbx_clear(int unit, args_t *args)
{
    char  *c;
#if defined(BCM_SIRIUS_SUPPORT)
    soc_control_t	*soc = SOC_CONTROL(unit);
    pbmp_t pbmp;
    int    r;
#endif

    if (!(c = ARG_GET(args))) {
        return(CMD_USAGE);
    }
    
    if (!sal_strcasecmp(c, "stat")) {
        return _cmd_sbx_clear_stat(unit, args);
    }

#if defined(BCM_SIRIUS_SUPPORT)
    SOC_PBMP_CLEAR(pbmp);
    if (!sal_strcasecmp(c, "counters") ||
	!sal_strcasecmp(c, "counter")  ||
	!sal_strcasecmp(c, "c")) {
	uint64		val;

	if ((c = ARG_GET(args)) == NULL) {
            if (soc_feature(unit, soc_feature_cpuport_stat_dma)) {
		pbmp_t temp_pbmp;
		SOC_PBMP_CLEAR(temp_pbmp);
		SOC_PBMP_OR(temp_pbmp, soc->counter_pbmp);
		SOC_PBMP_OR(temp_pbmp, PBMP_CMIC(unit));
		BCM_PBMP_AND(pbmp, temp_pbmp);
            } else {
                BCM_PBMP_ASSIGN(pbmp, soc->counter_pbmp);
            }
	} else if (parse_pbmp(unit, c, &pbmp) < 0) {
	    printk("%s: Invalid port bitmap: %s\n", ARG_CMD(args), c);
	    return CMD_FAIL;
	}

	if ((r = soc_counter_set32_by_port(unit, pbmp, 0)) < 0) {
	    printk("ERROR: Clear counters failed: %s\n", soc_errmsg(r));
	    return CMD_FAIL;
	}

	/*
	 * Clear the diagnostics' copy of the counters so 'show
	 * counters' knows they're clear.
	 */

	COMPILER_64_ZERO(val);

	sbx_counter_val_set_by_port(unit, pbmp, val);

	return CMD_OK;
    }
#endif /* BCM_SIRIUS_SUPPORT */
    return CMD_USAGE;
}
