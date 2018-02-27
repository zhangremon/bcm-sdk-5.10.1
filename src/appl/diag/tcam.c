/*
 * $Id: tcam.c 1.38 Broadcom SDK $
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
 * Diag CLI "tcam" command (5660x devices only)
 */

#include <soc/defs.h>
#if defined(BCM_EASYRIDER_SUPPORT) || defined(BCM_TRIUMPH_SUPPORT)

#include <appl/diag/system.h>
#include <bcm/error.h>
#include <soc/er_tcam.h>
#include <soc/triumph.h>

char cmd_tcam_usage[] =
    "Usages:\n"
    "\t  tcam info\n"
    "\t       - Show tcam configuration\n"
    "\t  tcam write Partition=<part> Index=<index> Mask=<mask> Data=<data>\n"
    "\t             [Count=<count>][Valid=<v>]\n"
    "\t  tcam dump Partition=<part> Index=<index> [Count=<count>]\n"
    "\t            [Display=<boolean>]\n"
    "\t  tcam search Partition=<part> [Partition2=<part>][Upper] Data=<data>\n"
    "\t  tcam ib Addr=address Data=opcode\n"
    "\t  tcam ima Addr=address [Data=<data>]\n"
    "\t  tcam dbreg Addr=address [Data=<data>]\n"
    "\t  tcam calc Partition=<part> Index=<index> Table=<table>\n"
;

static const char *partition_names[] = {
  "RAW",
  "ACL",
  "LPM",
  "FWD_L2",
  "FWD_IP4",
  "FWD_IP6U",
  "FWD_IP6",
  "ACL_L2",
  "ACL_IP4",
  "ACL_IP6S",
  "ACL_IP6F",
  "ACL_L2C",
  "ACL_IP4C",
  "ACL_IP6C",
  "ACL_L2IP4",
  "ACL_L2IP6",
  "DEV0_TBL72",
  "DEV0_TBL144",
  "DEV1_TBL72",
  "DEV1_TBL144",
  NULL
};

static const char *partition_descs[] = {
  "",
  "",
  "",
  "L2 forwarding table",
  "IPv4 forwarding table",
  "IPv6 64-bit forwarding table",
  "IPv6 128-bit forwarding table",
  "L2 288-bit ACL table",
  "IPv4 288-bit ACL table",
  "IPv6 360-bit ACL table",
  "IPv6 432-bit ACL table",
  "L2 144-bit ACL table",
  "IPv4 144-bit ACL table",
  "IPv6 144-bit ACL table",
  "L2 + IPv4 432-bit ACL table",
  "L2 + IPv6 432-bit ACL table",
  "Device 0 72-bit test table",
  "Device 0 144-bit test table",
  "Device 1 72-bit test table",
  "Device 1 144-bit test table",
};

/*
 * convert hex string to multiple groups of 72-bit binary
 * each 72-bit binary group is padded and stored in a 128-bit unit
 * for multiple 72-bit group, LS bit is in highest offset unit
 * for example: convert following 128-bit string to 144-bit binary
 * hex: [0x]1234567890abcdef1357924680acebdf
 * bin: 00000000 00000000 00123456 7890abcd 00000000 000000ef 13579246 80acebdf
 */
STATIC int
hex_to_72bit_unit_bin(char *hex_str, uint32 *bin_str, int num_unit)
{
    int len, len_per_unit, len_per_word, unit_idx, word_idx, nibble_idx;
    uint32 value;
    char hex_nibble;

    if (hex_str[0] == '0' && (hex_str[1] | 0x20) == 'x') {
        hex_str += 2;
    }

    len = strlen(hex_str);
    for (unit_idx = num_unit - 1; unit_idx >= 0; unit_idx--) {
        len_per_unit = len > 18 ? 18 : len;
        len -= len_per_unit;
        for (word_idx = 3; word_idx >= 0; word_idx--) {
            len_per_word = len_per_unit > 8 ? 8 : len_per_unit;
            len_per_unit -= len_per_word;
            value = 0;
            for (nibble_idx = 0; nibble_idx < len_per_word; nibble_idx++) {
                value <<= 4;
                hex_nibble = hex_str[len + len_per_unit + nibble_idx];
                if (hex_nibble >= '0' && hex_nibble <= '9') {
                    value |= hex_nibble - '0';
                } else {
                    hex_nibble |= 0x20;
                    if (hex_nibble >= 'a' && hex_nibble <= 'f') {
                        value |= hex_nibble - 'a' + 0x0a;
                    } else {
                        return CMD_FAIL;
                    }
                }
            }
            bin_str[unit_idx * 4 + word_idx] = value;
        }
    }

    return CMD_OK;
}

cmd_result_t
cmd_tcam(int u, args_t * a)
{
    soc_tcam_partition_t *partition;
    char * subcmd;
    parse_table_t pt;
    int rv, part, index, addr, valid, count, display, i, j;
    char *mask_str, *data_str, *table_str;
    uint32 mask[32], data[32];
    soc_tcam_info_t *tcam_info;
    int type, subtype, width;

    if ((subcmd = ARG_GET(a)) == NULL) {
        return CMD_USAGE;
    }

    rv = soc_tcam_get_info(u, &type, &subtype, NULL, &tcam_info);
    if (rv >= 0 &&
        (!type || !tcam_info->partitions[TCAM_PARTITION_RAW].num_entries)) {
        rv = SOC_E_INIT;
    }
    if (rv < 0) {
        printk("%s: ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
        return CMD_FAIL;
    }

    rv = SOC_E_NONE;
    if (sal_strcasecmp(subcmd, "init") == 0) {
        soc_tcam_init(u);
    } else if (sal_strcasecmp(subcmd, "info") == 0) {
        printk("tcam type %d subtype %d", type, subtype);
        for (i = TCAM_PARTITION_RAW + 1; i < TCAM_PARTITION_COUNT; i++) {
            partition = &tcam_info->partitions[i];
            if (partition->num_entries) {
                printk("\n%-10s %s",
                       partition_names[i], partition_descs[i]);
                printk("\n           %d entries",
                       partition->num_entries);
                if (partition->num_entries_include_pad !=
                    partition->num_entries) {
                    printk(" plus %d padded entries",
                           partition->num_entries_include_pad -
                           partition->num_entries);
                }
                printk("\n           tcam_base:0x%x width:%d-bit",
                       partition->tcam_base,
                       72 << partition->tcam_width_shift);
                if (partition->sram_base >= 0) {
                    printk("\n           sram_base:0x%x-0x%x width:%d-bit in",
                           partition->sram_base,
                           partition->sram_base +
                           ((partition->num_entries_include_pad - 1) <<
                            partition->sram_width_shift),
                           36 << partition->sram_width_shift);
                    if (partition->flags &
                        TCAM_PARTITION_FLAGS_AD_IN_SRAM0) {
                        printk(" sram0");
                    }
                    if (partition->flags &
                        TCAM_PARTITION_FLAGS_AD_IN_SRAM1) {
                        printk(" sram1");
                    }
                }
                if (partition->counter_base >= 0) {
                    printk("\n           counter_base:0x%x in %s",
                           partition->counter_base, partition->flags &
                           TCAM_PARTITION_FLAGS_COUNTER_IN_SRAM0 ?
                           "sram0" : "sram1");
                }
                if (partition->hbit_base >= 0) {
                    printk("\n           hbit_base:0x%x",
                           partition->hbit_base);
                }
            }
        }
        printk("\n");
    } else if (sal_strcasecmp(subcmd, "write") == 0) {
        uint32 mask_r[32], data_r[32], pattern;
        int valid_r, index_r, readback, search;

        parse_table_init(u, &pt);
        parse_table_add(&pt, "Partition", PQ_MULTI, 0, &part, partition_names);
        parse_table_add(&pt, "Index", PQ_INT, (void *)-1, &index, 0);
        parse_table_add(&pt, "Mask", PQ_STRING, 0, &mask_str, 0);
        parse_table_add(&pt, "Data", PQ_STRING, 0, &data_str, 0);
        parse_table_add(&pt, "Pattern", PQ_INT, (void *)0, &pattern, 0);
        parse_table_add(&pt, "Valid", PQ_INT, (void *)-1, &valid, 0);
        parse_table_add(&pt, "Count", PQ_INT, (void *)1, &count, 0);
        parse_table_add(&pt, "Readback", PQ_INT, (void *)0, &readback, 0);
        parse_table_add(&pt, "Search", PQ_INT, (void *)0, &search, 0);

        if (parse_arg_eq(a, &pt) < 0 || index < 0 ||
            !mask_str[0] || !data_str[0]) {
            parse_arg_eq_done(&pt);
            return CMD_USAGE;
        }
        width = 1 << tcam_info->partitions[part].tcam_width_shift;
        hex_to_72bit_unit_bin(mask_str, mask, width);
        hex_to_72bit_unit_bin(data_str, data, width);
        parse_arg_eq_done(&pt);

        for (i = 0; i < count; i++) {
            rv = soc_tcam_write_entry(u, part, index + i, mask, data);
            if (rv < 0) {
                break;
            }
            if (valid != -1) {
                rv = soc_tcam_set_valid(u, part, index + i, valid);
                if (rv < 0) {
                    break;
                }
            }
            if (readback) {
                rv = soc_tcam_read_entry(u, part, index + i, mask_r, data_r,
                                         &valid_r);
                if (rv < 0) {
                    break;
                }
                if ((data[1] ^ data_r[1]) & 0xff ||
                    (mask[1] ^ mask_r[1]) & 0xff ||
                    data[2] != data_r[2] || data[3] != data_r[3]) {
                    printk("Mismatched readback: index %d\n", index + i);
                }
                if (width == 2) {
                    if ((data[5] ^ data_r[5]) & 0xff ||
                        (mask[5] ^ mask_r[5]) & 0xff ||
                        data[6] != data_r[6] || data[7] != data_r[7]) {
                        printk("Mismatched readback: index %d\n", index + i);
                    }
                }
            }
            if (search) {
                if (SOC_IS_EASYRIDER(u)) {
	            rv = soc_tcam_search_entry(u, part, 0, data, &index_r,
                                               NULL);
                    if (rv == SOC_E_NOT_FOUND) {
                        printk("Lookup fail: index %d\n", index + i);
                        rv = SOC_E_NONE;
                    }
                }
            }
            data[width * 4 - 1] += 0x100;
        }
    } else if (sal_strcasecmp(subcmd, "dump") == 0) {
        parse_table_init(u, &pt);
        parse_table_add(&pt, "Partition", PQ_MULTI, 0, &part, partition_names);
        parse_table_add(&pt, "Index", PQ_INT, (void *)-1, &index, 0);
        parse_table_add(&pt, "Count", PQ_INT, (void *)1, &count, 0);
        parse_table_add(&pt, "Display", PQ_INT, (void *)1, &display, 0);

        if (parse_arg_eq(a, &pt) < 0 || index < 0) {
            parse_arg_eq_done(&pt);
            return CMD_USAGE;
        }
        parse_arg_eq_done(&pt);

        width = 1 << tcam_info->partitions[part].tcam_width_shift;
        for (i = 0; i < count; i++) {
            rv = soc_tcam_read_entry(u, part, index + i, mask, data, &valid);
            if (rv < 0) {
                break;
            }
            if (!display) {
                continue;
            }
            printk("%d: V=%d ", index + i, valid);
            printk("MASK=");
            for (j = 0; j < width; j++) {
                printk("%2.2x%8.8x%8.8x",
                       mask[j * 4 + 1], mask[j * 4 + 2], mask[j * 4 + 3]);
            }
            printk(" DATA=");
            for (j = 0; j < width; j++) {
                printk("%2.2x%8.8x%8.8x",
                       data[j * 4 + 1], data[j * 4 + 2], data[j * 4 + 3]);
            }
            printk("\n");
        }
    } else if (sal_strcasecmp(subcmd, "search") == 0) {
        int upper, part2, index2, bist;

        sal_memset(data, 0, sizeof(data));

        parse_table_init(u, &pt);
        parse_table_add(&pt, "Partition", PQ_MULTI, 0, &part, partition_names);
        parse_table_add(&pt, "Partition2", PQ_MULTI, 0, &part2,
                        partition_names);
        parse_table_add(&pt, "Data", PQ_STRING, 0, &data_str, 0);
        parse_table_add(&pt, "Upper", PQ_INT, (void *)0, &upper, 0);
        parse_table_add(&pt, "Bist", PQ_INT, (void *)0, &bist, 0);
        parse_table_add(&pt, "Index", PQ_INT, (void *)0, &index, 0);
        parse_table_add(&pt, "Index2", PQ_INT, (void *)0, &index2, 0);

        if (parse_arg_eq(a, &pt) < 0 || !data_str[0]) {
            parse_arg_eq_done(&pt);
            return CMD_USAGE;
        }
        width = tcam_info->partitions[part].tcam_width_shift >
            tcam_info->partitions[part2].tcam_width_shift ?
            1 << tcam_info->partitions[part].tcam_width_shift :
            1 << tcam_info->partitions[part2].tcam_width_shift;
        hex_to_72bit_unit_bin(data_str, data, width);
        parse_arg_eq_done(&pt);

        if (SOC_IS_EASYRIDER(u)) {
            rv = soc_tcam_search_entry(u, part, upper, data, &index, NULL);
            if (rv >= 0) {
                printk("Index=0x%x\n", index);
            } else if (rv == SOC_E_NOT_FOUND) {
                printk("NOT FOUND\n");
                rv = SOC_E_NONE;
            }
        }
        if (soc_feature(u, soc_feature_esm_support)) {
#if defined(BCM_TRIUMPH_SUPPORT)
            if (bist) {
                rv = soc_triumph_tcam_search_bist(u, part, part2, data, index,
                                                  index2, 10000);
            } else
#endif /* BCM_TRIUMPH_SUPPORT */
            {
                rv = soc_tcam_search_entry(u, part, part2, data, &index,
                                           &index2);
                if (rv >= 0) {
                    printk("Index=0x%x 0x%x\n", index, index2);
                } else if (rv == SOC_E_NOT_FOUND) {
                    printk("NOT FOUND\n");
                    rv = SOC_E_NONE;
                }
            }
        }
    } else if (sal_strcasecmp(subcmd, "ib") == 0) {
        parse_table_init(u, &pt);
        parse_table_add(&pt, "Addr", PQ_INT, (void *) -1, &addr, 0);
        parse_table_add(&pt, "Data", PQ_STRING, 0, &data_str, 0);

        if (parse_arg_eq(a, &pt) < 0) {
            parse_arg_eq_done(&pt);
            return CMD_USAGE;
        }
        hex_to_72bit_unit_bin(data_str, data, 1);
        parse_arg_eq_done(&pt);

        rv = soc_tcam_write_ib(u, addr, data[3]);
    } else if (sal_strcasecmp(subcmd, "ima") == 0) {
        parse_table_init(u, &pt);
        parse_table_add(&pt, "Addr", PQ_INT, (void *)-1, &addr, 0);
        parse_table_add(&pt, "Data", PQ_STRING, 0, &data_str, 0);

        if (parse_arg_eq(a, &pt) < 0 || addr < 0) {
            parse_arg_eq_done(&pt);
            return CMD_USAGE;
        }
        display = !data_str[0];
        hex_to_72bit_unit_bin(data_str, data, 1);
        parse_arg_eq_done(&pt);

        if (display) {
            rv = soc_tcam_read_ima(u, addr, &data[2], &data[3]);
            if (rv >= 0) {
                printk("Read from addr 0x%08x: 0x%08x %08x\n",
                       addr, data[2], data[3]);
            }
        } else {
            rv = soc_tcam_write_ima(u, addr, data[2], data[3]);
        }
    } else if (sal_strcasecmp(subcmd, "dbreg") == 0) {
        parse_table_init(u, &pt);
        parse_table_add(&pt, "Addr", PQ_INT, (void *)-1, &addr, 0);
        parse_table_add(&pt, "Data", PQ_STRING, 0, &data_str, 0);

        if (parse_arg_eq(a, &pt) < 0 || addr < 0) {
            parse_arg_eq_done(&pt);
            return CMD_USAGE;
        }
        display = !data_str[0];
        hex_to_72bit_unit_bin(data_str, data, 1);
        parse_arg_eq_done(&pt);

        if (display) {
            rv = soc_tcam_read_dbreg(u, addr, &data[1], &data[2], &data[3]);
            if (rv >= 0) {
                printk("Read from addr 0x%08x: 0x%08x 0x%08x %08x\n",
                       addr, data[1], data[2], data[3]);
            }
        } else {
            rv = soc_tcam_write_dbreg(u, addr, data[1], data[2], data[3]);
        }
    } else if (sal_strcasecmp(subcmd, "calc") == 0) {
        int copyno, mem_index, raw_index;
        soc_mem_t mem, real_mem;

        parse_table_init(u, &pt);
        parse_table_add(&pt, "Partition", PQ_MULTI, 0, &part, partition_names);
        parse_table_add(&pt, "Index", PQ_INT, (void *)-1, &index, 0);
        parse_table_add(&pt, "Table", PQ_STRING, 0, &table_str, 0);

        if (parse_arg_eq(a, &pt) < 0 || index < 0) {
            parse_arg_eq_done(&pt);
            return CMD_USAGE;
        }
        if (parse_memory_name(u, &mem, table_str, &copyno) < 0) {
            printk("%s: ERROR: unknown table %s\n", ARG_CMD(a), table_str);
            parse_arg_eq_done(&pt);
            return CMD_FAIL;
        }
        parse_arg_eq_done(&pt);

	rv = soc_tcam_part_index_to_mem_index(u, part, index, mem, &mem_index);
        if (rv >= 0) {
	    rv = soc_tcam_mem_index_to_raw_index(u, mem, mem_index,
                                                 &real_mem, &raw_index);
            if (rv >= 0) {
                printk("logical index 0x%x raw index 0x%x\n",
                       mem_index, raw_index);
            }
        }
    } else {
        return CMD_USAGE;
    }

    if (rv < 0) {
        printk("%s: ERROR: %s\n", ARG_CMD(a), bcm_errmsg(rv));
        return CMD_FAIL;
    }

    return CMD_OK;
}

#endif /* BCM_EASYRIDER_SUPPORT || BCM_TRIUMPH_SUPPORT */
