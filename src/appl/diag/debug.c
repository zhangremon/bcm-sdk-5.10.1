/*
 * $Id: debug.c 1.12 Broadcom SDK $
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
 * socdiag debug commands
 */

#include <appl/diag/system.h>
#include <soc/mem.h>

#include <soc/devids.h>
#include <soc/debug.h>

#ifdef BROADCOM_DEBUG

/*
 * INTERNAL: Cell header (CBPHEADER memory) format, for chip debug only
 */
typedef struct cell_header_s {
#if defined(LE_HOST)
    uint32 cpu_opcode:4,        /* Opcode to help out the CPU */
           ncl:2,               /* Next Cell Length */
           purge:1,             /* Purge this packet */
           O:2,                 /* Router help bits */
           timestamp:14,        /* Timestamp for this Packet */
           ipx:1,               /* IPX Packet */
           ip:1,                /* IP Packet */
           src_port:5,          /* The source port for this packet */
           _pad0:1,             /* Reserved */
           ncp_lo:1;            /* Next Cell Pointer (lower bit) */
    uint32 ncp_hi:15,           /* Next Cell Pointer (upper bits) */
           crc:2,               /* Tells Egress how to treat CRC */
           cell_len:7,          /* How many bytes are in this cell */
           lcc:5,               /* How Many copies of this cell are there */
           BCxMC:1,             /* This is a Broadcast or Multicast Packet */
           lc:1,                /* Last Cell of Packet */
           fc:1;                /* First Cell of Packet */
#else /* BE host */
    uint32 ncp_hi:1,
           _pad0:1,
           src_port:5,
           ip:1,
           ipx:1,
           timestamp:14,
           O:2,
           purge:1,
           ncl:2,
           cpu_opcode:4;
    uint32 fc:1,
           lc:1,
           BCxMC:1,
           lcc:5,
           cell_len:7,
           crc:2,
           ncp_lo:15;
#endif
} cell_header_t;

char cellhdr_usage[] =
    "Parameters: cellnum\n\t"
    "Displays the cell header value and fields for a given CBP cell\n";

cmd_result_t
dbg_cellhdr(int unit, args_t *a)
{
    char *cell_str = ARG_GET(a);
    int r, cell;
    int min, max;
    cell_header_t ch, *pch = &ch;

    UNSUPPORTED_COMMAND(unit, SOC_CHIP_BCM56504, a);
    UNSUPPORTED_COMMAND(unit, SOC_CHIP_BCM56304, a);
    UNSUPPORTED_COMMAND(unit, SOC_CHIP_BCM56102, a);
    UNSUPPORTED_COMMAND(unit, SOC_CHIP_BCM56314, a);
    UNSUPPORTED_COMMAND(unit, SOC_CHIP_BCM56112, a);
    UNSUPPORTED_COMMAND(unit, SOC_CHIP_BCM56514, a);

    if (! sh_check_attached(ARG_CMD(a), unit))
        return CMD_FAIL;

    if (! cell_str)
        return CMD_USAGE;

    cell = parse_integer(cell_str);

    min = soc_mem_index_min(unit, MMU_MEMORIES1_CBPHEADERm);
    max = soc_mem_index_max(unit, MMU_MEMORIES1_CBPHEADERm);

    if (cell < min || cell > max) {
        printk("ERROR: Cell number out of range (%d - %d)\n", min, max);
        return CMD_FAIL;
    }

#if 0
    assert(soc_mem_entry_bytes(unit, MMU_MEMORIES1_CBPHEADERm) == sizeof (ch));
#endif

    if ((r = soc_mem_read(unit, MMU_MEMORIES1_CBPHEADERm, MEM_BLOCK_ANY, cell,
                              (uint32 *) pch)) < 0) {
        printk("ERROR: Memory read failed: %s\n", soc_errmsg(r));
        return CMD_FAIL;
    }

    printk("CELL(0x%x):\n", cell);
    printk("  CPU opcode            0x%x\n", ch.cpu_opcode);
    printk("  Next cell length      0x%x\n", ch.ncl);
    printk("  Purge flag            0x%x\n", ch.purge);
    printk("  Router help bits      0x%x\n", ch.O);
    printk("  Packet timestamp      0x%x\n", ch.timestamp);
    printk("  IPX flag              0x%x\n", ch.ipx);
    printk("  IP flag               0x%x\n", ch.ip);
    printk("  Packet source port    0x%x\n", ch.src_port);
    printk("  Next Cell Pointer     0x%x\n", ch.ncp_hi << 1 | ch.ncp_lo);
    printk("  CRC regen bits        0x%x\n", ch.crc);
    printk("  Bytes in this cell    0x%x\n", ch.cell_len);
    printk("  Copies of this cell   0x%x\n", ch.lcc);
    printk("  Bcast or mcast flag   0x%x\n", ch.BCxMC);
    printk("  Last cell flag        0x%x\n", ch.lc);
    printk("  First cell flag       0x%x\n", ch.fc);

    return CMD_OK;
}

/*
 * INTERNAL: Slot header format, for chip debug only
 */
typedef struct slot_header_s {
#if defined(LE_HOST)
    uint32 timestamp:14,        /* Timestamp for this Packet */
           ipx:1,               /* IPX Packet */
           ip:1,                /* IP Packet */
           BCxMC:1,             /* This is a Broadcast or Multicast Packet */
           O:2,                 /* Router help bits */
           cell_len:6,          /* How many bytes are in this cell */
           crc:2,               /* Tells Egress how to treat CRC */
           lc:1,                /* Last Cell of Packet */
           fc:1,                /* First Cell of Packet */
           purge:1,             /* Purge this packet */
           ncl:2;               /* Next Cell Length */
    uint32 cpu_opcode:4,        /* Opcode to help out the CPU */
           _pad0:1,             /* Reserved */
           slot_size:6,         /* Slot Size */
           copy_count:5,        /* How many copies of this slot are valid */
           nsp:16;              /* Next Slot Pointer */
    uint32 ls:1,                /* Last Slot in Chain */
           src_port:5,          /* The source port for this packet */
           _pad1:7,             /* Reserved */
           slot_csum_lo:19;     /* Slot Checksum (lower bits) */
    uint32 slot_csum_hi;        /* Slot Checksum (upper bits) */
#else /* BE host */
    uint32 ncl:2,
           purge:1,
           fc:1,
           lc:1,
           crc:2,
           cell_len:6,
           O:2,
           BCxMC:1,
           ip:1,
           ipx:1,
           timestamp:14;
    uint32 nsp:16,
           copy_count:5,
           slot_size:6,
           _pad0:1,
           cpu_opcode:4;
    uint32 slot_csum_lo:19,
           _pad1:7,
           src_port:5,
           ls:1;
    uint32 slot_csum_hi;
#endif
} slot_header_t;

#endif /* BROADCOM_DEBUG */

char bkpmon_usage[] =
"Parameters: none\n\t"
"Monitors for backpressure discard messages.\n\t"
"Displays the gcccount registers when changes occur.\n";

cmd_result_t
dbg_bkpmon(int unit, args_t *a)
{
    uint32 curBkpReg, prevBkpReg;

    if (! sh_check_attached(ARG_CMD(a), unit))
        return CMD_FAIL;

    prevBkpReg = 0;

    printk("Monitoring Backpressure discard messages\n");

    for (;;) {
        while ((curBkpReg = soc_pci_read(unit,
                                            CMIC_IGBP_DISCARD)) == prevBkpReg)
            ;
        printk("CMIC Bkp Register = 0x%x\n", curBkpReg);
        sh_process_command(unit, "getreg gcccount");
        prevBkpReg = curBkpReg;
    }

    /* NOTREACHED */
}
