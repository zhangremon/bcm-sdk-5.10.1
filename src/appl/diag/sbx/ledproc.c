/*
 * $Id: ledproc.c 1.8 Broadcom SDK $
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
 * File: 	ledproc.c
 * Purpose: 	LED Processor Support
 */

#include <assert.h>
#include <sal/core/libc.h>
#include <bcm/error.h>
#include <bcm/link.h>
#include <soc/types.h>
#include <appl/diag/system.h>


#include <soc/sbx/fe2000.h>
#include <soc/sbx/hal_ca_auto.h>
#include <soc/sbx/hal_c2_auto.h>
#include <soc/sbx/sbx_drv.h>
#include <appl/diag/sbx/brd_sbx.h>
#include <appl/diag/sbx/ledproc.h>

#include <sal/appl/io.h>



#define FE2K_LED_CTRL                   0x00000000
#define FE2K_LED_STATUS                 0x00000001
#define FE2K_LED_PROGRAM_RAM_BASE       0x00000200
#define FE2K_LED_DATA_RAM_BASE          0x00000300

#define FE2K_LED_PROGRAM_RAM(_a)        (FE2K_LED_PROGRAM_RAM_BASE + (_a))
#define FE2K_LED_PROGRAM_RAM_SIZE       0x100
#define FE2K_LED_DATA_RAM(_a)           (FE2K_LED_DATA_RAM_BASE + (_a))
#define FE2K_LED_DATA_RAM_SIZE          0x100

#define LC_LED_ENABLE                   0x1     /* Enable */

#define LS_LED_INIT                     0x200   /* Initializing */
#define LS_LED_RUN                      0x100   /* Running */
#define LS_LED_PC                       0xff    /* Current PC */

static void _ledproc_write_addr(int unit, int addr, uint8 value)
{
    uint32_t reg = 0;
    int done;
    int iterations = 0;

    if (SOC_IS_SBX_FE2KXT(unit)) {        
        reg = SAND_HAL_SET_FIELD(C2, PC_LEDUP_ACC_DATA, DATA, value);
        SAND_HAL_WRITE(unit, C2, PC_LEDUP_ACC_DATA, reg);
        
        reg = SAND_HAL_READ(unit, C2, PC_LEDUP_ACC_CTRL); 
        reg = SAND_HAL_MOD_FIELD(C2, PC_LEDUP_ACC_CTRL, ADDRESS, reg, addr);
        reg = SAND_HAL_MOD_FIELD(C2, PC_LEDUP_ACC_CTRL, ACK, reg, 1);
        reg = SAND_HAL_MOD_FIELD(C2, PC_LEDUP_ACC_CTRL, REQ, reg, 1);
        reg = SAND_HAL_MOD_FIELD(C2, PC_LEDUP_ACC_CTRL, RD_WR_N, reg, 0);
        SAND_HAL_WRITE(unit, C2, PC_LEDUP_ACC_CTRL, reg);

        do {
            reg = SAND_HAL_READ(unit, C2, PC_LEDUP_ACC_CTRL);
            done = SAND_HAL_GET_FIELD(C2, PC_LEDUP_ACC_CTRL, ACK, reg);
        } while(!done && (++iterations < 10));
    } else {
        reg = SAND_HAL_SET_FIELD(CA, PC_LEDUP_ACC_DATA, DATA, value);
        SAND_HAL_WRITE(unit, CA, PC_LEDUP_ACC_DATA, reg);
        
        reg = SAND_HAL_READ(unit, CA, PC_LEDUP_ACC_CTRL); 
        reg = SAND_HAL_MOD_FIELD(CA, PC_LEDUP_ACC_CTRL, ADDRESS, reg, addr);
        reg = SAND_HAL_MOD_FIELD(CA, PC_LEDUP_ACC_CTRL, ACK, reg, 1);
        reg = SAND_HAL_MOD_FIELD(CA, PC_LEDUP_ACC_CTRL, REQ, reg, 1);
        reg = SAND_HAL_MOD_FIELD(CA, PC_LEDUP_ACC_CTRL, RD_WR_N, reg, 0);
        SAND_HAL_WRITE(unit, CA, PC_LEDUP_ACC_CTRL, reg);

        do {
            reg = SAND_HAL_READ(unit, CA, PC_LEDUP_ACC_CTRL);
            done = SAND_HAL_GET_FIELD(CA, PC_LEDUP_ACC_CTRL, ACK, reg);
        } while(!done && (++iterations < 10));
        
    }
    if(iterations >= 10) {
        printk("Error reading addr 0x%X\n", addr);
    }
}

static uint8_t _ledproc_read_addr(int unit, int addr)
{
    uint32_t reg = 0;
    int done;
    int iterations = 0;
    
    if (SOC_IS_SBX_FE2KXT(unit)) {
        reg = SAND_HAL_MOD_FIELD(C2, PC_LEDUP_ACC_CTRL, ADDRESS, reg, addr);
        reg = SAND_HAL_MOD_FIELD(C2, PC_LEDUP_ACC_CTRL, ACK, reg, 1);
        reg = SAND_HAL_MOD_FIELD(C2, PC_LEDUP_ACC_CTRL, RD_WR_N, reg, 1);
        reg = SAND_HAL_MOD_FIELD(C2, PC_LEDUP_ACC_CTRL, REQ, reg, 1);
        SAND_HAL_WRITE(unit, C2, PC_LEDUP_ACC_CTRL, reg);
        
        do {
            reg = SAND_HAL_READ(unit, C2, PC_LEDUP_ACC_CTRL);
            done = SAND_HAL_GET_FIELD(C2, PC_LEDUP_ACC_CTRL, ACK, reg);
        } while(!done && (++iterations < 10));
    } else {
        reg = SAND_HAL_MOD_FIELD(CA, PC_LEDUP_ACC_CTRL, ADDRESS, reg, addr);
        reg = SAND_HAL_MOD_FIELD(CA, PC_LEDUP_ACC_CTRL, ACK, reg, 1);
        reg = SAND_HAL_MOD_FIELD(CA, PC_LEDUP_ACC_CTRL, RD_WR_N, reg, 1);
        reg = SAND_HAL_MOD_FIELD(CA, PC_LEDUP_ACC_CTRL, REQ, reg, 1);
        SAND_HAL_WRITE(unit, CA, PC_LEDUP_ACC_CTRL, reg);
        
        do {
            reg = SAND_HAL_READ(unit, CA, PC_LEDUP_ACC_CTRL);
            done = SAND_HAL_GET_FIELD(CA, PC_LEDUP_ACC_CTRL, ACK, reg);
        } while(!done && (++iterations < 10));
    }
    if(iterations >= 10) {
        printk("Error reading addr 0x%X\n", addr);
        return 0;
    } else {
        if (SOC_IS_SBX_FE2KXT(unit)) {
            return SAND_HAL_READ(unit , C2, PC_LEDUP_ACC_DATA) & 0xFF;
        } else {
            return SAND_HAL_READ(unit , CA, PC_LEDUP_ACC_DATA) & 0xFF;
        }
    }
}

/*
 * Function:	ledproc_load
 * Purpose:	Load a program into the LED microprocessor from a buffer
 * Parameters:	unit - unit #
 *		program - Array of up to 256 program bytes
 *		bytes - Number of bytes in array
 * Notes:	Also clears the LED processor data RAM from 0x80-0xff
 *		so the LED program has a known state at startup.
 */

STATIC void
_ledproc_load(int unit, uint8 *program, int bytes)
{
    int		offset;    
    
    for (offset = 0; offset < FE2K_LED_PROGRAM_RAM_SIZE; offset++) {
        _ledproc_write_addr(unit, 
                      FE2K_LED_PROGRAM_RAM(offset),
                      (offset < bytes) ? (uint32) program[offset] : 0);
    }

    for (offset = 0x80; offset < FE2K_LED_DATA_RAM_SIZE; offset++) {
        _ledproc_write_addr(unit,
                      FE2K_LED_DATA_RAM(offset),
                      0);
    }
}


#ifndef NO_FILEIO
STATIC cmd_result_t
_ledproc_load_fp(int unit, char *cmd, char *file, FILE *f)
/*
 * Function:	ledproc_load_fp
 * Purpose:	Load a program into the LED microprocessor from a file
 * Parameters:	unit - unit #
 *		cmd - Name of command for error printing
 *		file - Name of file for error printing
 *		f - Open stdio file file pointer containing program
 * Returns:	CMD_XXX
 *
 * Notes:	File format (ASCII FILE)
 *	00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f
 *	10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f
 *	... for a total of 256 bytes, white space ignored.
 */
{
    uint8	program[FE2K_LED_PROGRAM_RAM_SIZE];
    char        input[256], *cp = NULL, *error = NULL;
    int         bytes = 0, line = 0;
    int         offset_size = 0;

    while (!error && (cp = fgets(input, sizeof(input) - 1, f))) {
        line++;

        while (*cp) {
            if (isspace((unsigned)(*cp))) { /* Skip whitespace */
                cp++;
            } else {
                if (!isxdigit((unsigned)*cp) ||
                    !isxdigit((unsigned)*(cp+1))) {
                    error = "Invalid hex digit";
                    break;
                }
                offset_size = sizeof(program);
                if (bytes >= offset_size) {
                    error = "Program memory exceeded";
                    break;
                }
                program[bytes++] = (xdigit2i(*cp) << 4) | xdigit2i(*(cp + 1));
                cp += 2;
            }
        }
    }

    if (error) {
        printk("%s: Error loading file %s line %d: %s\n",
               cmd, file, line, error);
        return(CMD_FAIL);
    }

    printk("%s: Loading %d bytes into LED program memory\n",
           cmd, bytes);

    _ledproc_load(unit, program, bytes);

    return(CMD_OK);
}
#endif /* NO_FILEIO */


/*
 * Function:	ledproc_load_args
 * Purpose:	Load a program into the LED microprocessor from an args_t
 * Parameters:	unit - unit #
 *		a - Remainder of command line containing raw hex data
 * Returns:	CMD_XXX
 * Notes:	Input strings are concatenated and parsed the
 *		same way as ledproc_load_fp.
 */
STATIC cmd_result_t
_ledproc_load_args(int unit, args_t *a)
/*
 * Notes:	Args format
 *	00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f
 *	10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f
 *	...
 * White space optional.
 */
{
    uint8	program[FE2K_LED_PROGRAM_RAM_SIZE];
    int		bytes = 0;
    char	*cp;
    int         offset_size = 0;

    sal_memset(program, 0, sizeof (program));

    while ((cp = ARG_GET(a)) != NULL) {
	while (*cp) {
	    if (isspace((int)(*cp))) {
		cp++;
	    } else {
		if (!isxdigit((unsigned)*cp) ||
		    !isxdigit((unsigned)*(cp+1))) {
		    printk("%s: Invalid character\n", ARG_CMD(a));
		    return(CMD_FAIL);
		}

                offset_size = sizeof(program);
		if (bytes >= offset_size) {
		    printk("%s: Program memory exceeded\n", ARG_CMD(a));
		    return(CMD_FAIL);
		}
		program[bytes++] = (xdigit2i(*cp) << 4) | xdigit2i(*(cp + 1));
		cp += 2;
	    }
	}
    }

    _ledproc_load(unit, program, bytes);

    return(CMD_OK);
}

/*
 * Function: 	ledproc_linkscan_cb
 * Purpose:	Call back function for LEDs on link change.
 * Parameters:	unit - unit number
 *              port - callback from this port
 *              info - pointer to structure giving status
 * Returns:	nothing
 * Notes:	Each port has one byte of data at address (0x80 + portnum).
 *		In each byte, bit 0 is used for link status.
 */
STATIC void
_ledproc_linkscan_cb(int unit, soc_port_t port, bcm_port_info_t *info)
{
    uint32	portdata;
    int		byte = LS_LED_DATA_OFFSET + port;

    portdata = _ledproc_read_addr(unit, FE2K_LED_DATA_RAM(byte));

    if (info->linkstatus) {
	portdata |= 0x01;
    } else {
	portdata &= ~0x01;
    }

    _ledproc_write_addr(unit, FE2K_LED_DATA_RAM(byte), portdata);
}



char cmd_sbx_ledproc_usage[] =
    "Parameters: [start | stop | load <file.hex> |\n\t"
    "            [auto [on | off]] | prog <hexdata> | dump\n\t"
    "Load the LED Microprocessor with the program specified in <file.hex>.\n\t"
    "Option 'auto' turns on (by default) or off automatic linkscan\n\t"
    "    updates to the LED processor.\n";

cmd_result_t
cmd_sbx_ledproc(int unit, args_t *a)
/*
 * Function: 	sh_led
 * Purpose:	Load a led program and start it.
 * Parameters:	unit - unit
 *		a - args, each of the files to be displayed.
 * Returns:	CMD_OK/CMD_FAIL/CMD_USAGE
 */
{
    VOL cmd_result_t		rv = CMD_OK;
#ifndef NO_FILEIO
    jmp_buf			ctrl_c;
    FILE * volatile		f = NULL;
#endif
    char			*c;
    volatile uint32		led_ctrl, led_status;
    volatile int		auto_on;

    if (!sh_check_attached("ledproc", unit)) {
	return(CMD_FAIL);
    }

    if (!SOC_IS_SBX_FE2000(unit)) {
        printk("%s: LED Micro-controller not available on %s\n",
               ARG_CMD(a),
               soc_dev_name(unit));
        return(CMD_FAIL);
    }

    led_ctrl = _ledproc_read_addr(unit, FE2K_LED_CTRL);
    led_status = _ledproc_read_addr(unit, FE2K_LED_STATUS);

    if (bcm_linkscan_unregister(unit, _ledproc_linkscan_cb) < 0) {
        auto_on = FALSE;
    } else {
        (void)bcm_linkscan_register(unit, _ledproc_linkscan_cb);
        auto_on = TRUE;
    }
    
    if (ARG_CNT(a) == 0) {
        printk("%s: LED Micro-controller is %s (PC=0x%x State=%s%s); "
               "auto updating is %s.\n",
               ARG_CMD(a),
               (led_ctrl & LC_LED_ENABLE) ? "ON" : "OFF",
               (led_status & LS_LED_PC),
               (led_status & LS_LED_INIT) ? "INIT," : "",
               (led_status & LS_LED_RUN) ? "RUN" : "SLEEP",
               auto_on ? "ON" : "OFF");
        soc_cm_debug(DK_VERBOSE, "LED_CTRL=0x%02X LED_STATUS=0x%02X\n",
                     led_ctrl, led_status);
        return(CMD_OK);
    }

    c = ARG_GET(a);
    
    if (!sal_strcasecmp(c, "start")) {
        int		offset;
        
        /* The LED data area should be clear whenever program starts */

        _ledproc_write_addr(unit, FE2K_LED_CTRL, led_ctrl & ~LC_LED_ENABLE);
        
        for (offset = 0x80; offset < FE2K_LED_DATA_RAM_SIZE; offset++) {
            _ledproc_write_addr(unit,
                          FE2K_LED_DATA_RAM(offset),
                          0);
        }
        
        led_ctrl = LC_LED_ENABLE;
        
        _ledproc_write_addr(unit, FE2K_LED_CTRL, led_ctrl);
        
        if (auto_on) {
            (void)bcm_link_change(unit, PBMP_PORT_ALL(unit));
        }
        
        return(CMD_OK);
    } else if (!sal_strcasecmp(c, "stop")) {
        led_ctrl &= ~LC_LED_ENABLE;
        _ledproc_write_addr(unit, FE2K_LED_CTRL, led_ctrl);
        
        return(CMD_OK);
    } else if (!sal_strcasecmp(c, "load")) {
        /*
         * Try to catch ^C to avoid leaving file open if ^C'd.
         * There are still a number of unlikely race conditions here.
         * Temporarily stop LED processor if it is currently running.
         */
#ifdef NO_FILEIO
        printk("no filesystem\n");
#else	
        _ledproc_write_addr(unit, FE2K_LED_CTRL, led_ctrl & ~LC_LED_ENABLE);
        
        if (!setjmp(ctrl_c)) {
            sh_push_ctrl_c(&ctrl_c);
            
            if ((c = ARG_GET(a)) != NULL) {
                f = (FILE *)sal_fopen(c, "r");
                if (!f) {
                    printk("%s: Error: Unable to open file: %s\n",
                           ARG_CMD(a), c);
                    rv = CMD_FAIL;
                } else {
                    rv = _ledproc_load_fp(unit, ARG_CMD(a), c, (FILE *)f);
                    sal_fclose((FILE *)f); /* Cast for un-volatile */
                    f = NULL;
                }
            } else {
                printk("%s: Error: No file specified\n", ARG_CMD(a));
                rv = CMD_USAGE;
            }
        } else if (f) {
            sal_fclose((FILE *)f);
            f = NULL;
            rv = CMD_INTR;
        }
        
        /* Restore LED processor if it was running */
        _ledproc_write_addr(unit, FE2K_LED_CTRL, led_ctrl);
        
        if (auto_on) {
            (void)bcm_link_change(unit, PBMP_PORT_ALL(unit));
        }
        
        sh_pop_ctrl_c();
#endif /* NO_FILEIO */
    } else if (!sal_strcasecmp(c, "prog")) {
        if (ARG_CUR(a) == NULL) {
            return(CMD_USAGE);
        }
        
        led_ctrl &= ~LC_LED_ENABLE;
        _ledproc_write_addr(unit, FE2K_LED_CTRL, led_ctrl);
        
        return _ledproc_load_args(unit, a);
    } else if (!sal_strcasecmp(c, "auto")) {
        int new_auto = 1;
        
        if ((c = ARG_GET(a)) != NULL) {
            if (!sal_strcasecmp(c, "off")) {
                new_auto = 0;
            } else if (!sal_strcasecmp(c, "on")) {
                new_auto = 1;
            } else {
                return CMD_USAGE;
            }
        }
        
        if (new_auto && !auto_on) {
            (void)bcm_linkscan_register(unit, _ledproc_linkscan_cb);
        }
        
        if (!new_auto && auto_on) {
            (void)bcm_linkscan_unregister(unit, _ledproc_linkscan_cb);
        }
        
        if (new_auto) {
            (void)bcm_link_change(unit, PBMP_PORT_ALL(unit));
        }
    } else if (!sal_strcasecmp(c, "dump")) {
        int		offset, max;
        
        for (max = FE2K_LED_PROGRAM_RAM_SIZE - 1; max >= 0; max--) {
            if (_ledproc_read_addr(unit, FE2K_LED_PROGRAM_RAM(max)) != 0) {
                break;
            }
        }
        
	for (offset = 0; offset <= max; offset++) {
	    printk(" %02x", _ledproc_read_addr(unit, FE2K_LED_PROGRAM_RAM(offset)));
	    if ((offset % 16) == 15) {
            printk("\n");
	    }
	}
    
	if (offset % 16 != 0) {
	    printk("\n");
	}
    } else {
        return(CMD_USAGE);
    }
    
    return(rv);
}

void bcma_ledproc_start (int unit, uint8 * prog, int len)
{
    volatile uint32		led_ctrl;
    int offset;
    
    led_ctrl = _ledproc_read_addr(unit, FE2K_LED_CTRL);
    
    /* register linkscan callback */
    (void)bcm_linkscan_register(unit, _ledproc_linkscan_cb);
    
    /* load the program */
    _ledproc_load(unit, prog, len);    
    
    /*
    ** start the program, do this by 1st stopping any running program, 
    ** then clear all RAM and finally activate the program again 
    */
    _ledproc_write_addr(unit, FE2K_LED_CTRL, led_ctrl & ~LC_LED_ENABLE);
    
    
    /* The LED data area should be clear whenever program starts */
    for (offset = 0x80; offset < FE2K_LED_DATA_RAM_SIZE; offset++) {
        _ledproc_write_addr(unit,
                            FE2K_LED_DATA_RAM(offset),
                            0);
    }
    
    _ledproc_write_addr(unit, FE2K_LED_CTRL, LC_LED_ENABLE);
    
    (void)bcm_link_change(unit, PBMP_PORT_ALL(unit));
}
