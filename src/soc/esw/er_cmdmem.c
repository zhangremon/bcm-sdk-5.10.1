/*
 * $Id: er_cmdmem.c 1.6 Broadcom SDK $
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
 * Easyrider SOC Command Memory (Table) Utilities
 */

#include <sal/core/libc.h>

#include <soc/mem.h>
#include <soc/debug.h>

#include <soc/cmic.h>
#include <soc/error.h>
#include <soc/register.h>
#include <soc/drv.h>
#include <soc/er_cmdmem.h>

#ifdef BCM_EASYRIDER_SUPPORT

STATIC char *_soc_mem_cmd_op_names[] = {
    "READ",               /*  0 */
    "WRITE",              /*  1 */
    "SEARCH",             /*  2 */
    "LEARN",              /*  3 */
    "DELETE",             /*  4 */
    "PASS_CAM_WRITE",     /*  5 */
    "PASS_CAM_READ",      /*  6 */
    "SHIFT_DOWN",         /*  7 */
    "SHIFT_UP",           /*  8 */
    "CLEAR_L3_HIT",       /*  9 */
    "SET_L3_HIT",         /* 10 */
    "SWAP_L3_HIT",        /* 11 */
    "READ_ONE",           /* 12 */
    "SEARCH_72",          /* 13 */
    "INSERT_LPM",         /* 14 */
    "REMOVE_LPM",         /* 15 */
    "PER_PORT_AGING",     /* 16 */
    "UNKNOWN_OPCODE",     /* 17 */
};

char *
soc_mem_cmd_op_name(int op)
{
    if (op < 0 || op >= COUNTOF(_soc_mem_cmd_op_names)) {
        op = 17;
    }

    return _soc_mem_cmd_op_names[op];
}

/*
 * Resets the memory command interface. This is required when we sent
 * a message and did not receive a response after the poll count was
 * exceeded.
 */

STATIC void
_soc_mem_cmd_reset(int unit)
{
  
}

/*
 * Dump a memory command message for debugging
 */

void
soc_mem_cmd_dump(int unit, soc_mem_cmd_t *cmd, soc_mem_t mem, uint32 *data)
{
    char                buf[128];
    int                 i, j, dwc = SOC_MEM_CMD_DATA_WORDS;


    soc_cm_print("  MemCmd[Memory=%s Cmd=%d=%s SCmd=%d\n"
                 "         A0=0x%08x A1=0x%08x]\n",
                 SOC_MEM_NAME(unit, mem),
                 cmd->command, soc_mem_cmd_op_name(cmd->command),
                 cmd->sub_command, cmd->addr0, cmd->addr1);

    if (data) {
    soc_cm_print("  Input:\n");
    for (i = 0; i < dwc; i += 4) {
        buf[0] = 0;

        for (j = i; j < i + 4 && j < dwc; j++) {
            sal_sprintf(buf + sal_strlen(buf),
                        " DW[%2d]=0x%08x", j,
                        j < soc_mem_entry_words(unit, mem) ? data[j] : 0);
                        
        }

        soc_cm_print(" %s\n", buf);
    }
    } else {
    soc_cm_print("  Output:\n");
    for (i = 0; i < dwc; i += 4) {
        buf[0] = 0;

        for (j = i; j < i + 4 && j < dwc; j++) {
            sal_sprintf(buf + sal_strlen(buf),
                        " DW[%2d]=0x%08x", j, 
                        (j < soc_mem_entry_words(unit, mem)) ? 
                        cmd->output_data[j] : 0);
        }

        soc_cm_print(" %s\n", buf);
    }
    }
}

int
_soc_mem_cmd_errors(uint32 cmd_reg_data)
{
    if ((cmd_reg_data & SOC_MEM_CMD_TAB_FULL) != 0) {
        return SOC_E_FULL;
    }

    if ((cmd_reg_data & SOC_MEM_CMD_PARITY_ERR) != 0) {
        return SOC_E_FAIL;
    }

    if ((cmd_reg_data & SOC_MEM_CMD_INVALID_ADDR) != 0) {
        return SOC_E_PARAM;
    }

    /* Correct error? */
    if ((cmd_reg_data & SOC_MEM_CMD_L2_MOD_FIFO_FULL) != 0) {
        return SOC_E_BUSY;
    }

    if ((cmd_reg_data & SOC_MEM_CMD_INVALID_CMD) != 0) {
        return SOC_E_PARAM;
    }

    if ((cmd_reg_data & SOC_MEM_CMD_NOT_FOUND) != 0) {
        return SOC_E_NOT_FOUND;
    }

    return SOC_E_NONE;
}


/*
 * Function:
 *	soc_mem_cmd_op
 * Purpose:
 *	Perform a command memory operation.
 * Notes:
 *
 */

int
soc_mem_cmd_op(int unit, soc_mem_t mem, soc_mem_cmd_t *cmd_info, int intr)
{
    int                        rv = SOC_E_NONE;
    command_memory_bse_entry_t cmd_entry;
    soc_mem_t                  cmd_mem;
    soc_reg_t                  cmd_reg;
    soc_mem_info_t	       *mip;
    int                        entry_dw = soc_mem_entry_words(unit, mem);
    uint32                     target_tab, cmd_reg_data = 0;
    int                        mcmd_ix;
    int                        use_intr, do_dump = FALSE;
    uint32                     intr_mask;
    soc_control_t              *soc;

    assert(cmd_info->entry_data);
    assert(entry_dw <= SOC_MEM_CMD_DATA_WORDS);

    sal_memset(&cmd_entry, 0, sizeof(cmd_entry));

    mip = &SOC_MEM_INFO(unit, mem);
    use_intr = intr && SOC_CONTROL(unit)->memCmdIntrEnb;

    soc = SOC_CONTROL(unit);

    switch (SOC_BLOCK_TYPE(unit, SOC_MEM_BLOCK_ANY(unit, mem))) {
    case SOC_BLK_BSE:
        cmd_mem = COMMAND_MEMORY_BSEm;
        cmd_reg = CMDWORD_SHADOW_BSEr;
        mcmd_ix = 0;
        intr_mask = IRQ_BSE_CMDMEM_DONE;
        break;

    case SOC_BLK_CSE:
        cmd_mem = COMMAND_MEMORY_CSEm;
        cmd_reg = CMDWORD_SHADOW_CSEr;
        mcmd_ix = 1;
        intr_mask = IRQ_CSE_CMDMEM_DONE;
        break;

    case SOC_BLK_HSE:
        cmd_mem = COMMAND_MEMORY_HSEm;
        cmd_reg = CMDWORD_SHADOW_HSEr;
        mcmd_ix = 2;
        intr_mask = IRQ_HSE_CMDMEM_DONE;
        break;

    default:
        soc_cm_debug(DK_ERR,
                     "soc_mem_cmd_op: command attempt on memory %s\n",
                     SOC_MEM_NAME(unit, mem));
        return SOC_E_PARAM;
    }

    if ((cmd_info->command == SOC_MEM_CMD_PASS_WRITE) ||
        (cmd_info->command == SOC_MEM_CMD_PASS_READ)) {
        entry_dw = 2 * SOC_MEM_CMD_TCAM_ADDR_MAX;
    } else if (cmd_info->command == SOC_MEM_CMD_PER_PORT_AGING) {
        entry_dw = 2;
    }

    sal_memcpy(&cmd_entry, cmd_info->entry_data, entry_dw * sizeof (uint32));

    target_tab = (mip->base >> SOC_MEM_CMD_BASE_TO_OFFSET) & 
        SOC_MEM_CMD_TARGET_MASK;

    soc_mem_field32_set(unit, cmd_mem, (uint32 *) &cmd_entry,
                      TARGET_TABf, target_tab);

    soc_mem_field32_set(unit, cmd_mem, (uint32 *) &cmd_entry,
                      COMMANDf, cmd_info->command);

    /* Only relevant if DEFIP table */
    if ((mem == L3_DEFIP_CAMm) || (mem == L3_DEFIP_ALGm)) {
        /* DEFIP tables need sub command field filled in */
        if (soc_mem_field32_get(unit, mem,
                                cmd_info->entry_data, FFf) == 0xff) {
            switch (soc_mem_field32_get(unit, mem,
                                        cmd_info->entry_data, KEY_TYPEf)) {
            case SOC_ER_DEFIP_KEY_TYPE_IPV4:
                cmd_info->sub_command = SOC_MEM_CMD_SUB_V4;
                break;
            case SOC_ER_DEFIP_KEY_TYPE_MPLS_1L:
                cmd_info->sub_command = SOC_MEM_CMD_SUB_MPLS1;
                break;
            case SOC_ER_DEFIP_KEY_TYPE_MPLS_2L:
                cmd_info->sub_command = SOC_MEM_CMD_SUB_MPLS2;
                break;
            default:
                return SOC_E_PARAM;
            }
        } else {
            cmd_info->sub_command = SOC_MEM_CMD_SUB_V6;
        }
    }
    soc_mem_field32_set(unit, cmd_mem, &cmd_entry, SUB_CMDf,
                        cmd_info->sub_command);

    soc_mem_field32_set(unit, cmd_mem, (uint32 *) &cmd_entry,
                      ADDRESS_0f, cmd_info->addr0);
    soc_mem_field32_set(unit, cmd_mem, (uint32 *) &cmd_entry,
                      ADDRESS_1f, cmd_info->addr1);

    soc_mem_field32_set(unit, cmd_mem, (uint32 *) &cmd_entry, NEWCMDf, 1);

    MEM_LOCK(unit, cmd_mem);

    if (soc_cm_debug_check(DK_SOCMEM)) {
        soc_cm_print("Memory command %s: (unit %d)\n",
                     soc_mem_cmd_op_name(cmd_info->command), unit);
        do_dump = TRUE;
    }

    if (do_dump) {
        soc_mem_cmd_dump(unit, cmd_info, mem, (uint32 *)&cmd_entry);
    } 

    do {
        rv = SOC_E_NONE; 

        if (use_intr) {
            soc_mem_field32_set(unit, cmd_mem,
                                (uint32 *) &cmd_entry, ENABLE_INTRf, 1);
        }

        /* Write command memory operation*/
	/* Override schan write-suppression during warm reboot */

        if (SOC_WARM_BOOT(unit)) {
    	    if (cmd_info->command == SOC_MEM_CMD_READ || cmd_info->command == 
	        SOC_MEM_CMD_SEARCH || cmd_info->command == SOC_MEM_CMD_READ_ONE || 
	        cmd_info->command == SOC_MEM_CMD_PASS_READ) {
	        soc->schan_override = 1;
	    }
        }
	
        if ((rv = soc_mem_write(unit, cmd_mem, COPYNO_ALL,
                                0, &cmd_entry)) < 0) {
            if (soc_cm_debug_check(DK_ERR)) {
                soc_cm_print("soc_mem_cmd_op:  Write to %s failed\n",
                             SOC_MEM_NAME(unit, cmd_mem));
                do_dump = TRUE;
            }
            if (SOC_WARM_BOOT(unit)) {
    	        if (cmd_info->command == SOC_MEM_CMD_READ || cmd_info->command == 
	            SOC_MEM_CMD_SEARCH || cmd_info->command == SOC_MEM_CMD_READ_ONE
	       	    || cmd_info->command == SOC_MEM_CMD_PASS_READ) {
	            soc->schan_override = 0;
	        }
            }		    
            break;
        }

        if (SOC_WARM_BOOT(unit)) {
    	    if (cmd_info->command == SOC_MEM_CMD_READ || cmd_info->command == 
	        SOC_MEM_CMD_SEARCH || cmd_info->command == SOC_MEM_CMD_READ_ONE || 
	        cmd_info->command == SOC_MEM_CMD_PASS_READ) {
	        soc->schan_override = 0;
	    }
        }	

        /* Wait for completion using either the interrupt or polling method */

        if (use_intr) {
            soc_intr_enable(unit, intr_mask);

            /* Do we need different ones here? */
            if (sal_sem_take(SOC_CONTROL(unit)->memCmdIntr[mcmd_ix],
                             SOC_CONTROL(unit)->memCmdTimeout) != 0) {
                rv = SOC_E_TIMEOUT;
                soc_intr_disable(unit, intr_mask);
                break;
            } else {
                soc_cm_debug(DK_SOCMEM, "  Interrupt received\n");

                /* Use outstanding xact counter here */
                soc_intr_disable(unit, intr_mask);

                if ((rv = soc_reg32_get(unit, cmd_reg, 0, 0,
                                         &cmd_reg_data)) < 0) {
                    if (soc_cm_debug_check(DK_ERR)) {
                        soc_cm_print("soc_mem_cmd_op:  Read from %s failed\n",
                                     SOC_REG_NAME(unit, cmd_reg));
                        do_dump = TRUE;
                    }
                    break;
                }
            }
        } else {
            soc_timeout_t to;

            soc_timeout_init(&to, SOC_CONTROL(unit)->memCmdTimeout, 100);

            while ((rv = soc_reg32_get(unit, cmd_reg, 0, 0,
                                        &cmd_reg_data)) >= 0) {
                /* Operation done? */
                if ((cmd_reg_data & SOC_MEM_CMD_NEWCMD) == 0) {
                    break;
                }
                if (soc_timeout_check(&to)) {
                    rv = SOC_E_TIMEOUT;
                    break;
                }
            }

            if (rv == SOC_E_NONE) {
                soc_cm_debug(DK_SOCMEM, "  Done in %d polls\n", to.polls);
            }
        }

        if (rv == SOC_E_TIMEOUT) {
            soc_cm_debug(DK_VERBOSE,
                         "soc_mem_cmd_op: operation attempt timed out\n");
            SOC_CONTROL(unit)->stat.err_mc_tmo++;
            _soc_mem_cmd_reset(unit);
            break;
        }

        /* Analyze errors */
        if ((rv = _soc_mem_cmd_errors(cmd_reg_data)) < 0) {
            if ((rv == SOC_E_BUSY) &&
                (cmd_info->command == SOC_MEM_CMD_SEARCH)) {
                /* Ignore mod fifo full on search */
                rv = SOC_E_NONE;
            } else {
                if (soc_cm_debug_check(DK_SOCMEM)) {
                    soc_cm_print("soc_mem_cmd_op:  Error: %s\n",
                                 soc_errmsg(rv));
                    do_dump = TRUE;
                }
                break;
            }
        }

        /* Should we do this read by default? Probably not */
        /* Read in data from memory command, if any */

        if ((rv = soc_mem_read(unit, cmd_mem, MEM_BLOCK_ANY,
                               0, &cmd_entry)) < 0) {
            if (soc_cm_debug_check(DK_ERR)) {
                soc_cm_print("soc_mem_cmd_op:  Read from %s failed\n",
                             SOC_MEM_NAME(unit, cmd_mem));
                do_dump = TRUE;
            }
            break;
        }

        cmd_info->addr0 = 
            soc_mem_field32_get(unit, cmd_mem, (uint32 *) &cmd_entry,
                      ADDRESS_0f);
        cmd_info->addr1 = 
            soc_mem_field32_get(unit, cmd_mem, (uint32 *) &cmd_entry,
                      ADDRESS_1f);
        

        sal_memcpy(cmd_info->output_data, &cmd_entry, 
                   SOC_MEM_CMD_DATA_WORDS * sizeof (uint32));

        if (soc_cm_debug_check(DK_SOCMEM)) {
            do_dump = TRUE;
        }

        SOC_CONTROL(unit)->stat.mem_cmd_op++;

    } while (0);
   
    MEM_UNLOCK(unit, cmd_mem);

    if (rv == SOC_E_TIMEOUT) {
        if (soc_cm_debug_check(DK_ERR)) {
            soc_cm_print("soc_mem_cmd_op: operation timed out\n");
            do_dump = TRUE;
        }
    }

    if (do_dump) {
        soc_mem_cmd_dump(unit, cmd_info, mem, NULL);
    } 

    return rv;
}


/*
 * Function:
 *	soc_mem_tcam_op
 * Purpose:
 *	Perform an external TCAM read or write operation of up to 6 addresses.
 * Notes:
 */

int
soc_mem_tcam_op(int unit, uint32 *addr, uint32 *data, int num_addr,
                int write)
{
    soc_mem_cmd_t cmd_info;
    int           ix, rv;

    if ((num_addr < 1) || (num_addr > SOC_MEM_CMD_TCAM_ADDR_MAX)) {
        return SOC_E_PARAM;
    }

    sal_memset(&cmd_info, 0, sizeof(cmd_info));

    cmd_info.entry_data = cmd_info.output_data;

    for (ix = 0; ix < num_addr; ix++) {
        if (write) {
            cmd_info.output_data[ix] = data[ix];
        }
        cmd_info.output_data[ix + SOC_MEM_CMD_TCAM_ADDR_MAX] = addr[ix];
    }

    cmd_info.addr1 = num_addr - 1; /* Number of xacts */

    cmd_info.command = write ? SOC_MEM_CMD_PASS_WRITE : SOC_MEM_CMD_PASS_READ;

    rv = soc_mem_cmd_op(unit, FP_EXTERNALm, &cmd_info, TRUE);

    if ((rv >= 0) && !write) {
        for (ix = 0; ix < num_addr; ix++) {
            data[ix] = cmd_info.output_data[ix];
        }
    }

    return rv;
}


#endif
