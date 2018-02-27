/*
 * $Id: show.c 1.30.2.2 Broadcom SDK $
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
 * Show CLI commands
 *
 * Also contains:
 *	Check CLI command
 *	Clear CLI command
 */

#include <appl/diag/system.h>
#include <appl/diag/dport.h>

#include <bcm/stat.h>
#include <bcm/error.h>
#include <bcm/init.h>
#include <bcm/filter.h>

#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/mcm/driver.h>

#include <sal/appl/pci.h>

#if defined(VXWORKS)
#include <netShow.h>
#include <muxLib.h>
#endif /* VXWORKS */

#if defined(VXWORKS_NETWORK_STACK_6_5)
#include <net/utils/netstat.h>
#endif /* VXWORKS_NETWORK_STACK_6_5 */

#if defined(BCM_TRIDENT_SUPPORT)
#include <soc/trident.h>
#endif /* BCM_TRIDENT_SUPPORT */

#if defined(BCM_KATANA_SUPPORT)
#include <soc/katana.h>
#endif /* BCM_KATANA_SUPPORT */

/*
 * Utility routine for show command
 */

cmd_result_t
do_show_memory(int unit, soc_mem_t mem, int copyno)
{
    int		entries;
    char	*ufname;
    int		blk;

    ufname = SOC_MEM_UFNAME(unit, mem);
    if (!soc_mem_is_sorted(unit, mem) &&
        !soc_mem_is_hashed(unit, mem) &&
        mem != VLAN_TABm) {
            printk("ERROR: %s table is not sorted or hashed\n",
                ufname);
            return CMD_FAIL;
    }

    SOC_MEM_BLOCK_ITER(unit, mem, blk) {
        if (copyno != COPYNO_ALL && copyno != blk) {
            continue;
        }
        if ((entries = soc_mem_entries(unit, mem, blk)) < 0) {
            printk("Error reading number of entries: %s\n",
                soc_errmsg(entries));

            return CMD_FAIL;
        }

        printk("%s.%s contains %d entries\n",
            ufname, SOC_BLOCK_NAME(unit, blk), entries);
    }

    return CMD_OK;
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
    "TeMPmonitor",                      /* 7 */
    "MaterialProcess",                  /* 8 */
    "RingOSC",                          /* 9 */
};

static int show_arg_cnt = PARSE_ENTRIES(show_arg);

static parse_key_t show_ctr_arg[] = {
    "Changed",				/* 0 */
    "Same",				/* 1 */
    "Zero",				/* 2 */
    "NonZero",				/* 3 */
    "Hex",				/* 4 */
    "Raw",				/* 5 */
    "All",				/* 6 */
    "ErDisc",				/* 7 */
    "MmuStatus",       			/* 8 */
};

static int show_ctr_arg_cnt = PARSE_ENTRIES(show_ctr_arg);

static const soc_reg_t thermal_reg[] = {
    CMIC_THERMAL_MON_RESULT_0r, CMIC_THERMAL_MON_RESULT_1r,
    CMIC_THERMAL_MON_RESULT_2r, CMIC_THERMAL_MON_RESULT_3r,
    CMIC_THERMAL_MON_RESULT_4r, CMIC_THERMAL_MON_RESULT_5r,
    CMIC_THERMAL_MON_RESULT_6r, CMIC_THERMAL_MON_RESULT_7r
};

cmd_result_t
cmd_esw_show(int unit, args_t *a)
{
    soc_control_t	*soc = SOC_CONTROL(unit);
    soc_pbmp_t		pbmp;
    bcm_pbmp_t		bcm_pbmp;
    bcm_port_config_t   pcfg;
    const parse_key_t	*cmd;
    char		*c;
    soc_mem_t		mem;
    int			copyno;
    int			flags;
    soc_regaddrlist_t	alist;
    soc_reg_t		ctr_reg;
    soc_port_t		port, dport;

    if (!(c = ARG_GET(a))) {		/* Nothing to do */
        return(CMD_USAGE);		/* Print usage line */
    }

#if defined (VXWORKS) 
#if defined (VXWORKS_NETWORK_STACK_6_5)
    if (!sal_strcasecmp(c, "ip")) {
        netstat("-p ip -f inet");
        return CMD_OK;
    }
    if (!sal_strcasecmp(c, "icmp")) {
        netstat("-p icmp -f inet");
        return CMD_OK;
    }
    if (!sal_strcasecmp(c, "arp")) {
        arpShow();
        return CMD_OK;
    }
    if (!sal_strcasecmp(c, "udp")) {
        netstat("-p udp -f inet");
        return CMD_OK;
    }
    if (!sal_strcasecmp(c, "tcp")) {
        netstat("-p tcp -f inet");
        return CMD_OK;
    }
    if (!sal_strcasecmp(c, "mux")) {
        muxShow(NULL, 0);
        return CMD_OK;
    }
    if (!sal_strcasecmp(c, "routes")) {
        netstat("-a -r");
        return CMD_OK;
    }
    if (!sal_strcasecmp(c, "hosts")){
        hostShow();
        return CMD_OK;
    }
#else /* !VXWORKS_NETWORK_STACK_6_5 */
    if (!sal_strcasecmp(c, "ip")) {
        ipstatShow(FALSE);
        return CMD_OK;
    }
    if (!sal_strcasecmp(c, "icmp")) {
        icmpstatShow();
        return CMD_OK;
    }
    if (!sal_strcasecmp(c, "arp")) {
        arpShow();
        return CMD_OK;
    }
    if (!sal_strcasecmp(c, "udp")) {
        udpstatShow();
        return CMD_OK;
    }
    if (!sal_strcasecmp(c, "tcp")) {
        tcpstatShow();
        return CMD_OK;
    }
    if (!sal_strcasecmp(c, "mux")) {
#if defined(IDTRP334)
        printk("muxShow not available on this BSP\n");
#else
        muxShow(NULL, 0);
#endif
        return CMD_OK;
    }
    if (!sal_strcasecmp(c, "routes")) {
        routeShow();
        return CMD_OK;
    }
    if (!sal_strcasecmp(c, "hosts")){
        hostShow();
        return CMD_OK;
    }
#endif /* VXWORKS_NETWORK_STACK_6_5 */
#endif

#if defined(BROADCOM_DEBUG)
#ifdef BCM_FILTER_SUPPORT
    if (!sal_strcasecmp(c, "filter")) {
        bcm_filter_show(unit, ARG_CMD(a));
        return CMD_OK;
    }
#endif  /* BROADCOM_DEBUG */

    if (!sal_strcasecmp(c, "feature") || !sal_strcasecmp(c, "features")) {
        int		all;
        soc_feature_t	f;

        c = ARG_GET(a);
        if (c == NULL) {
            all = 0;
        } else {
            all = 1;
        }
        printk("Unit %d features:\n", unit);

        for (f = 0; f < soc_feature_count; f++) {
            if (soc_feature(unit, f)) {
                printk("\t%s\n", soc_feature_name[f]);
            } else if (all) {
                printk("\t[%s]\n", soc_feature_name[f]);
            }
        }
        return CMD_OK;
    }

    if (!sal_strcasecmp(c, "param") || !sal_strcasecmp(c, "params")) {
        if (!(c = ARG_GET(a))) {
            /* Current unit */
            soc_chip_dump(unit, SOC_DRIVER(unit));
        } else {
            int chip, pcidev;
            chip = sal_ctoi(c, 0);
            if (chip >= 0x5600 && chip <= 0x56ff) {
                /* search for driver for pci device id */
                pcidev = chip;
                for (chip = 0; chip < SOC_NUM_SUPPORTED_CHIPS; chip++) {
                    if (pcidev == soc_base_driver_table[chip]->pci_device) {
                        soc_chip_dump(-1, soc_base_driver_table[chip]);
                        return CMD_OK;
                    }
                }
                printk("Chip device %x not found\n", pcidev);
                return CMD_FAIL;
            }
            /* specific chip requested */
            if (chip >= SOC_NUM_SUPPORTED_CHIPS) {
                printk("Bad chip parameter:  %d.  Max is %d\n",
                       chip, SOC_NUM_SUPPORTED_CHIPS);
            } else if (!SOC_DRIVER_ACTIVE(chip)) {
                printk("Chip %d is not supported.\n", chip);
            } else {
                soc_chip_dump(-1, soc_base_driver_table[chip]);
            }
        }
        return CMD_OK;
    }
    if (!sal_strcasecmp(c, "unit") || !sal_strcasecmp(c, "units")) {
        int		u;
        soc_control_t	*usoc;

        c = ARG_GET(a);
        if (c != NULL) {	/* specific unit */
            u = sal_ctoi(c, 0);
            if (!SOC_UNIT_VALID(u)) {
                printk("Unit %d is not valid\n", u);
                return CMD_FAIL;
            }
            usoc = SOC_CONTROL(u);
            if (!(usoc->soc_flags & SOC_F_ATTACHED)) {
                printk("Unit %d (detached)\n", u);
                return CMD_OK;
            }
            printk("Unit %d chip %s%s\n",
                   u,
                   soc_dev_name(u),
                   u == unit ? " (current)" : "");
            soc_chip_dump(u, SOC_DRIVER(u));
        } else {		/* all units */
            for (u = 0; u < soc_ndev; u++) {
                if (!SOC_UNIT_VALID(u)) {
                    continue;
                }
                usoc = SOC_CONTROL(u);
                printk("Unit %d chip %s%s\n",
                       u,
                       soc_dev_name(u),
                       u == unit ? " (current)" : "");
            }
        }
        return CMD_OK;
    }
#endif

    /* Can only check memory this way if unit is attached */
    if (soc_attached(unit)) {
        if (!sal_strcasecmp(c, "sa") || !sal_strcasecmp(c, "sarl")) {
            printk("Software ARL table: ");
            if (SOC_CONTROL(unit)->arlShadow == NULL) {
                printk("none\n");
            } else {
                printk("%d entries\n",
                       shr_avl_count(SOC_CONTROL(unit)->arlShadow));
            }
            return CMD_OK;
        }

        if (parse_memory_name(unit, &mem, c, &copyno) >= 0) {
            return do_show_memory(unit, mem, copyno);
        }
    } else { /* show based on chip */
    }

    cmd = parse_lookup(c, show_arg, sizeof(show_arg[0]), show_arg_cnt);
    if (!cmd) {
        printk("%s: Error: Invalid option %s\n", ARG_CMD(a), c);
        return(CMD_FAIL);
    }

    switch(cmd - show_arg) {
#if !defined(BCM_ICS) && !defined(NO_SAL_APPL)
    case 0:				/* PCI */
        pci_print_all();
        break;
#endif
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
            case 8:
                flags |= SHOW_CTR_MS;
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
            SOC_PBMP_ASSIGN(pbmp, PBMP_ALL(unit));
        } else if (parse_pbmp(unit, c, &pbmp) >= 0) {
            ctr_reg = INVALIDr;		/* All registers, selected ports */
        } else {
            int		i;

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

            SOC_PBMP_CLEAR(pbmp);

            for (i = 0; i < alist.count; i++) {
                SOC_PBMP_PORT_ADD(pbmp, alist.ainfo[i].port);
            }
            soc_regaddrlist_free(&alist);
        }

        if (soc_feature(unit, soc_feature_cpuport_stat_dma)) {
            pbmp_t temp_pbmp;
            SOC_PBMP_CLEAR(temp_pbmp);
            SOC_PBMP_OR(temp_pbmp, PBMP_PORT_ALL(unit));
            SOC_PBMP_OR(temp_pbmp, PBMP_CMIC(unit));
            SOC_PBMP_AND(pbmp, temp_pbmp);
        } else {
            SOC_PBMP_AND(pbmp, PBMP_PORT_ALL(unit));
        }
        do_show_counters(unit, ctr_reg, pbmp, flags);

        break;
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
            printk("%s:             Table-DMA(%d),TSlam-DMA(%d)\n",
                   ARG_CMD(a),
                   soc->stat.intr_tdma, soc->stat.intr_tslam);
        }
        if (soc_feature(unit, soc_feature_fifo_dma)) {
            printk("%s:             Ch0-fifo-DMA(%d),Ch1-fifo-DMA(%d)\n",
                   ARG_CMD(a),
                   soc->stat.intr_fifo_dma[0], soc->stat.intr_fifo_dma[1]);
            printk("%s:             Ch2-fifo-DMA(%d),Ch3-fifo-DMA(%d)\n",
                   ARG_CMD(a),
                   soc->stat.intr_fifo_dma[2], soc->stat.intr_fifo_dma[3]);
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
        if (bcm_port_config_get(unit, &pcfg) != BCM_E_NONE) {
            printk("%s: Error: bcm ports not initialized\n", ARG_CMD(a));
            return CMD_FAIL;
        }

        flags = 0;
        if ((c = ARG_GET(a)) != NULL) {		/* Ports specified? */
            if (0 != parse_bcm_pbmp(unit, c, &bcm_pbmp)) {
                printk("%s: Invalid ports: %s\n", ARG_CMD(a), c);
                return(CMD_FAIL);
            }
            if ((c = ARG_GET(a)) != NULL && sal_strcasecmp(c, "all") == 0) {
                    flags = 1;
            }
        } else {
            BCM_PBMP_ASSIGN(bcm_pbmp, pcfg.port);
        }

        DPORT_BCM_PBMP_ITER(unit, bcm_pbmp, dport, port) {
            bcm_stat_val_t s;
            char *sname;
            char *_stat_names[] = BCM_STAT_NAME_INITIALIZER;
            int rv;
            uint64 val;

            printk("%s: Statistics for Unit %d port %s\n",
                   ARG_CMD(a), unit, BCM_PORT_NAME(unit, port));

            bcm_stat_sync(unit);
       
            for (s = 0; s < snmpValCount; s++) {
                 sname = _stat_names[s];
                 if (!sname) {
                     continue;
                 }

                 rv = bcm_stat_get(unit, port, s, &val);;

                 if (rv < 0) {
                     printk("%8s\t%s (stat %d): %s\n",
                           "-", sname, s, bcm_errmsg(rv));
                     continue;
                 }

                 if (flags == 0 && COMPILER_64_IS_ZERO(val)) {
                     continue;
                 }

                 if (COMPILER_64_HI(val) == 0) {
                     printk("%8u\t%s (stat %d)\n",
                           COMPILER_64_LO(val), sname, s);
                 } else {
                        printk("0x%08x%08x\t%s (stat %d)\n",
                           COMPILER_64_HI(val),
                           COMPILER_64_LO(val),
                           sname, s);
                 }
             }
        }
        break; 
    case 7:                             /* TeMPmonitor */
#ifdef BCM_TRIDENT_SUPPORT
        if (SOC_IS_TD_TT(unit)) {
            (void)soc_trident_show_temperature_monitor(unit);
        } else
#endif /* BCM_TRIDENT_SUPPORT */
#ifdef BCM_KATANA_SUPPORT
        if (SOC_IS_KATANA(unit)) {
            (void)soc_katana_show_temperature_monitor(unit);
        } else
#endif /* BCM_KATANA_SUPPORT */
        if (SOC_REG_IS_VALID(unit, CMIC_THERMAL_MON_RESULT_0r)) {

#ifdef BCM_SHADOW_SUPPORT
            if (SOC_IS_SHADOW(unit)) {
                uint32 rval, addr;
                soc_reg_t reg;
                int index;
                int fval, cur, peak, avg_cur, max_peak;

                avg_cur = 0;
                max_peak = 0x80000000;
                for (index = 0; index < 5; index++) {
                    reg = thermal_reg[index];
                    addr = soc_reg_addr(unit, reg, REG_PORT_ANY, 0);
                    soc_pci_getreg(unit, addr, &rval);
                    fval = soc_reg_field_get(unit, reg, rval, TEMP_DATAf);
                    cur = (4180000 - (5556 * fval)) / 10000;
                    fval = soc_reg_field_get(unit, reg, rval, PEAK_TEMP_DATAf);
                    peak = (4180000 - (5556 * fval)) / 10000;
                    printk("temperature monitor %d: current=%3d, peak=%3d\n",
                           index, cur, peak);
                    avg_cur += cur;
                    if (max_peak < peak) {
                        max_peak = peak;
                    }
                }
                printk("average current temperature is %3"
                       COMPILER_DOUBLE_FORMAT"\n", (COMPILER_DOUBLE)avg_cur / index);
                printk("maximum peak temperature is %d\n", max_peak);
                READ_CMIC_SOFT_RESET_REGr(unit, &rval);
                soc_reg_field_set(unit, CMIC_SOFT_RESET_REGr, &rval,
                              TEMP_MON_PEAK_RESET_Nf, 0);
                WRITE_CMIC_SOFT_RESET_REGr(unit, rval);
                soc_reg_field_set(unit, CMIC_SOFT_RESET_REGr, &rval,
                              TEMP_MON_PEAK_RESET_Nf, 1);
                WRITE_CMIC_SOFT_RESET_REGr(unit, rval);

            }
#endif
        }
        break;
    case 8:                             /* MaterialProcess */
#ifdef BCM_TRIDENT_SUPPORT
        if (SOC_IS_TD_TT(unit)) {
            (void)soc_trident_show_material_process(unit);
        }
#endif /* BCM_TRIDENT_SUPPORT */
        break;
    case 9:                             /* RingOSC */
#ifdef BCM_TRIDENT_SUPPORT
        if (SOC_IS_TD_TT(unit)) {
            (void)soc_trident_show_ring_osc(unit);
        }
#endif /* BCM_TRIDENT_SUPPORT */
#ifdef BCM_KATANA_SUPPORT
        if (SOC_IS_KATANA(unit)) {
            (void)soc_katana_show_ring_osc(unit);
        }
#endif /* BCM_KATANA_SUPPORT */
        break;
    default:
        return(CMD_FAIL);
    }

    return(CMD_OK);
}

/*
 * Clear something for a user.
 */

cmd_result_t
cmd_esw_clear(int unit, args_t *a)
{
    char *parm = ARG_GET(a);
    soc_mem_t mem;
    soc_pbmp_t pbmp;
    int force_all = 0;
    int r, copyno;

    if (!sh_check_attached(ARG_CMD(a), unit)) {
	return CMD_FAIL;
    }

    if (!parm) {
	return CMD_USAGE;
    }

    if (!sal_strcasecmp(parm, "all")) {
	force_all = 1;
	parm = ARG_GET(a);
    }

    if (!parm) {
    return CMD_USAGE;
    }


    if (!sal_strcasecmp(parm, "counters") ||
	!sal_strcasecmp(parm, "c")) {
	uint64		val;

	if ((parm = ARG_GET(a)) == NULL) {
            if (soc_feature(unit, soc_feature_cpuport_stat_dma)) {
                SOC_PBMP_ASSIGN(pbmp, PBMP_ALL(unit));
            } else {
                SOC_PBMP_ASSIGN(pbmp, PBMP_PORT_ALL(unit));
            }
	} else if (parse_pbmp(unit, parm, &pbmp) < 0) {
	    printk("%s: Invalid port bitmap: %s\n", ARG_CMD(a), parm);
	    return CMD_FAIL;
	}
        /* Remove the lb ports from the bitmap */
        if (soc_feature(unit, soc_feature_internal_loopback)) {
            SOC_PBMP_REMOVE(pbmp, PBMP_LB(unit));
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

	counter_val_set_by_port(unit, pbmp, val);

	return CMD_OK;
    }

    if (!sal_strcasecmp(parm, "stats")) {
	int		rv = CMD_OK;
	soc_port_t	port, dport;
        bcm_pbmp_t      bcm_pbmp;
        bcm_port_config_t pcfg;

        if (bcm_port_config_get(unit, &pcfg) != BCM_E_NONE) {
            printk("%s: Error: bcm ports not initialized\n", ARG_CMD(a));
            return CMD_FAIL;
        }

	if ((parm = ARG_GET(a)) == NULL) {
            if (soc_feature(unit, soc_feature_cpuport_stat_dma)) {
                SOC_PBMP_ASSIGN(pbmp, pcfg.all);
            } else {
                SOC_PBMP_ASSIGN(pbmp, pcfg.port);
            }
	} else if (parse_bcm_pbmp(unit, parm, &bcm_pbmp) < 0) {
	    printk("%s: Invalid port bitmap: %s\n", ARG_CMD(a), parm);
	    return CMD_FAIL;
	}

    /*    coverity[uninit_use : FALSE]    */
        DPORT_BCM_PBMP_ITER(unit, bcm_pbmp, dport, port) {
	    int	rv;

	    if ((rv = bcm_stat_clear(unit, port)) != BCM_E_NONE) {
		printk("%s: Unit %d Port %d failed to clear stats: %s\n",
		       ARG_CMD(a), unit, port, bcm_errmsg(rv));
		rv = CMD_FAIL;
	    }
	}
	return(rv);
    }

    if (!sal_strcasecmp(parm, "dev")) {
        int rv;
        rv = bcm_clear(unit);
        if (rv < 0) {
            printk("%s ERROR: Unit %d.  bcm_clear returned %d: %s\n",
                   ARG_CMD(a), unit, rv, bcm_errmsg(rv));
            return CMD_FAIL;
        }
        return CMD_OK;
    }

    if (!sal_strcasecmp(parm, "sa") || !sal_strcasecmp(parm, "sarl")) {
	if (SOC_CONTROL(unit)->arlShadow != NULL) {
	    sal_mutex_take(SOC_CONTROL(unit)->arlShadowMutex,
			   sal_mutex_FOREVER);
	    shr_avl_delete_all(SOC_CONTROL(unit)->arlShadow);
	    sal_mutex_give(SOC_CONTROL(unit)->arlShadowMutex);
	}
	return CMD_OK;
    }

    do {
	if (parse_memory_name(unit, &mem, parm, &copyno) < 0) {
	    printk("ERROR: unknown table \"%s\"\n", parm);
	    return CMD_FAIL;
	}

	if ((r = soc_mem_clear(unit, mem, copyno, force_all)) < 0) {
	    printk("ERROR: clear table %s failed: %s\n",
		   SOC_MEM_UFNAME(unit, mem), soc_errmsg(r));
	    return CMD_FAIL;
	}
    } while ((parm = ARG_GET(a)) != NULL);

    return CMD_OK;
}

/*
 * Check something for a user.
 */

char check_usage[] =
    "Parameters: check [SYNC] <TABLE>\n\t"
    "Verifies consistency of a memory table (sorted tables only).\n\t"
    "If the SYNC flag is used, checks that all copies are equal.\n";

cmd_result_t
cmd_check(int unit, args_t *a)
{
    char *parm = ARG_GET(a);
    soc_mem_t mem;
    int blk, copyno;
    char *ufname;
    int index, min, max, entries, entry_bytes;
    uint32 previous[SOC_MAX_MEM_WORDS], current[SOC_MAX_MEM_WORDS];
    uint32 *entry_null;
    int r, rv = CMD_OK;
    int sync = 0;
    int port_counts[SOC_MAX_NUM_PORTS];
    int iters, blkfirst;

    if (!sh_check_attached(ARG_CMD(a), unit))
	return CMD_FAIL;

    if (parm != NULL && sal_strcasecmp(parm, "sync") == 0) {
	sync = 1;
	parm = ARG_GET(a);
    }

    if (parm == NULL) {
	return CMD_USAGE;
    }

    if (parse_memory_name(unit, &mem, parm, &copyno) < 0) {
	printk("ERROR: unknown table \"%s\"\n", parm);
	return CMD_FAIL;
    }

    ufname = SOC_MEM_UFNAME(unit, mem);
    min = soc_mem_index_min(unit, mem);
    max = soc_mem_index_max(unit, mem);
    entry_bytes = 4 * soc_mem_entry_words(unit, mem);
    entry_null = soc_mem_entry_null(unit, mem);

    if (!soc_mem_is_sorted(unit, mem)) {
	goto skip_sort_check;
    }

    memset(port_counts, 0, sizeof(port_counts));

    iters = 0;
    SOC_MEM_BLOCK_ITER(unit, mem, blk) {
	if (copyno != COPYNO_ALL && copyno != blk) {
	    continue;
	}
	iters += 1;
	entries = soc_mem_entries(unit, mem, blk);

    for (index = min; index <= max; index++) {
        if ((r = soc_mem_read(unit, mem, blk, index, current)) < 0) {
            printk("ERROR: failed reading %s.%s[%d]: %s\n",
                ufname, SOC_BLOCK_NAME(unit, blk),
                index, soc_errmsg(r));
            return CMD_FAIL;
        }

        if (memcmp(current, entry_null, entry_bytes) == 0)
            break;	/* Found null key */

        if (index > min &&
            soc_mem_compare_key(unit, mem, current, previous) <= 0) {
                printk("ERROR: non-increasing key at %s.%s[%d]\n",
                    ufname, SOC_BLOCK_NAME(unit, blk), index);
                rv = CMD_FAIL;
        }

        memcpy(previous, current, entry_bytes);
    }

	/* Make sure entry count matches number of entries found */

	if (index != min + entries) {
	    printk("ERROR: last non-null key was found at %s.%s[%d]\n",
		   ufname, SOC_BLOCK_NAME(unit, blk), index - 1);
	    printk("ERROR: but was expected to be at %s.%s[%d]\n",
		   ufname, SOC_BLOCK_NAME(unit, blk), min + entries);
	    printk("ERROR: based on a table entry count of %d\n",
		   entries);
	    rv = CMD_FAIL;
	}

	/* Make sure remaining entries in table are null */

	for (; index <= max; index++) {
	    if ((r = soc_mem_read(unit, mem, blk, index, current)) < 0) {
		printk("ERROR: failed reading %s.%s[%d]: %s\n",
		       ufname, SOC_BLOCK_NAME(unit, blk),
		       index, soc_errmsg(r));
		return CMD_FAIL;
	    }

	    if (memcmp(current, entry_null, entry_bytes) != 0) {
		printk("ERROR: entry at %s.%s[%d] should contain null key\n",
		       ufname, SOC_BLOCK_NAME(unit, blk), index);
		rv = CMD_FAIL;
	    }
	}
    }

    /*
     * Optionally check that all copies of a table have identical contents.
     */

 skip_sort_check:

    if (sync) {
        for (index = min; index <= max; index++) {
            iters = 0;
            blkfirst = 0;
            SOC_MEM_BLOCK_ITER(unit, mem, blk) {
                if (copyno != COPYNO_ALL && copyno != blk) {
                    continue;
                }
                if (++iters == 1) {
                    blkfirst = blk;
                }
                if ((r = soc_mem_read(unit, mem, blk, index,
                    iters == 1 ? previous : current) < 0)) {
                    printk("ERROR: failed reading %s.%s[%d]: %s\n",
                        ufname, SOC_BLOCK_NAME(unit, blk),
                        index, soc_errmsg(r));
                    return CMD_FAIL;
                }

                if (iters > 1) {
                    if (memcmp(current, previous, entry_bytes) != 0) {
                        printk("ERROR: %s.%s[%d] does not match %s.%s[%d]\n",
                            ufname, SOC_BLOCK_NAME(unit, blk), index,
                            ufname, SOC_BLOCK_NAME(unit, blkfirst), index);
                        rv = CMD_FAIL;
                    }
                }
            }
        }
    }

    return rv;
}
