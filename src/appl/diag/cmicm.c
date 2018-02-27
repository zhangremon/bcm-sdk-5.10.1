/*
 * $Id: cmicm.c 1.4.2.4 Broadcom SDK $
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
 * CMICm peripherals commands
 */

#include <assert.h>
#include <sal/core/libc.h>
#include <bcm/error.h>

#include <soc/types.h>
#include <soc/mspi.h>
#include <soc/cmicm.h>

#include <appl/diag/shell.h>
#include <appl/diag/system.h>
#include "appl/diag/diag.h"

#ifdef BCM_CMICM_SUPPORT

#define CMICM_MSPI_BLOCK_SIZE 16


char mspi_usage[] =
    "Parameters: [device] load <file.hex> | [write <hexdata> [read <NumBytes>]]\n\t"
    "Optionally select a device. The device number is specific to chip/board.\n\t"
    "<file.hex> is ASCII file, with hex bytes separated by white space\n\t"
    "Write and Read can be used separately or together\n\t"
    "When Write and Read are used together, MSPI writes and reads without de-asserting CS\n";
    

cmd_result_t
mspi_cmd(int unit, args_t *a)
{
    volatile cmd_result_t rv = CMD_OK;
    char *c , *filename;
    int dev, offset_size = 0;
    volatile int wbytes = 0, rbytes=0;
	int start_byte, num_bytes;
    uint8 wdata[256];
    uint8 rdata[32];
#ifndef NO_FILEIO
    char    input[256];
#ifndef NO_CTRL_C
    jmp_buf ctrl_c;
#endif
    FILE * volatile fp = NULL;
#endif

    if (!sh_check_attached("mspi", unit)) {
        return(CMD_FAIL);
    }

    if (!soc_feature(unit, soc_feature_cmicm)) {
        return(CMD_FAIL);
    }

    if (ARG_CNT(a) == 0) {
        return CMD_USAGE;
    }

    c = ARG_GET(a);
    if (isint(c)) {
        dev = parse_integer(c);
        soc_cm_debug(DK_VERBOSE, "MSPI: Setting Device to %d\n", dev);
        c = ARG_GET(a);

        if (soc_mspi_select_device(unit, dev) != SOC_E_NONE){
            soc_cm_print("MSPI: Failure Selecting Device\n");
            return CMD_FAIL;
        }

        if (c == NULL) {
            /* Just select device. No read / Write */
            return CMD_OK;
        }
    }

    sal_memset(wdata, 0, sizeof (wdata));
    sal_memset(rdata, 0, sizeof (rdata));

    if (!sal_strcasecmp(c, "load")) {
        c = ARG_GET(a);
        filename = c;
        if (filename == NULL) {
            soc_cm_print("MSPI: %s: Error: No file specified\n", ARG_CMD(a));
            return(CMD_USAGE);
        }
#ifdef NO_FILEIO
        soc_cm_print("no filesystem\n");
        rv = CMD_FAIL;
#else
#ifndef NO_CTRL_C        
        if (!setjmp(ctrl_c)) {
            sh_push_ctrl_c(&ctrl_c);
#endif            
            fp = sal_fopen(filename, "r");
            if (!fp) {
                soc_cm_print("MSPI: %s: Error: Unable to open file: %s\n",
                   ARG_CMD(a), filename);
                rv = CMD_FAIL;
            } else {
                while ((rv == CMD_OK) && (c = fgets(input, sizeof(input) - 1, fp))) {
                    while (*c) {
                        if (isspace((unsigned)(*c))) { /* Skip whitespace */
                            c++;
                        } else {
                            if (!isxdigit((unsigned)*c) ||
                                !isxdigit((unsigned)*(c+1))) {
                                soc_cm_print("MSPI: %s: Invalid character\n", ARG_CMD(a));
                                rv = CMD_FAIL;
                                break;
                            }
                            offset_size = sizeof(wdata);
                            if (wbytes >= offset_size) {
                                soc_cm_print("MSPI: %s: Data memory exceeded\n", ARG_CMD(a));
                                rv = CMD_FAIL;
                                break;
                            }
                            wdata[wbytes++] = (xdigit2i(*c) << 4) | xdigit2i(*(c + 1));
                            c += 2;
                        }
                    }
                }
                sal_fclose((FILE *)fp);
                fp = NULL;
            }
#ifndef NO_CTRL_C
        } else if (fp) {
            sal_fclose((FILE *)fp);
            fp = NULL;
            rv = CMD_INTR;
        }
#endif        

        sh_pop_ctrl_c();
#endif /* NO_FILEIO */

        if (rv != CMD_OK) {
            return rv;
        }
        soc_cm_debug(DK_VERBOSE, "MSPI: Writing %d bytes from file %s\n", wbytes, filename);
        for(start_byte=0; start_byte < wbytes; start_byte+=CMICM_MSPI_BLOCK_SIZE) {
            num_bytes = ((start_byte + CMICM_MSPI_BLOCK_SIZE) > wbytes) ?
                        (wbytes - start_byte) : CMICM_MSPI_BLOCK_SIZE;
            if (soc_mspi_write8(unit, (uint8 *) &wdata[start_byte], num_bytes) != SOC_E_NONE) {
                soc_cm_print("MSPI: Write Fail\n");
                return CMD_FAIL;
            }
        }
    } else if (!sal_strcasecmp(c, "write")) {
        while ((c = ARG_GET(a)) != NULL) {
            if (!sal_strcasecmp(c, "read")) {
                /* Write & Read in a Single Operation */
                if (ARG_CNT(a) == 0) {
                    return CMD_USAGE;
                }
                c = ARG_GET(a);
                if (!isint(c)) {
                    return CMD_USAGE;
                }
                rbytes = parse_integer(c);
                soc_cm_debug(DK_VERBOSE,"Write %d bytes and read %d bytes\n", wbytes, rbytes);
                if ((rv = soc_mspi_writeread8(unit, (uint8 *) &wdata[0], wbytes, (uint8 *) &rdata[0], rbytes)) != (cmd_result_t)SOC_E_NONE) {
                    soc_cm_print("MSPI: Write-Read Fail\n");
                    return CMD_FAIL;
                }
                /* Dump It */
                for(start_byte=0; start_byte < rbytes; start_byte++) {
                    soc_cm_print("%02x ", rdata[start_byte]);
                    if ((start_byte % 8) == 7) {
                        soc_cm_print("\n");
                    }
                }
                soc_cm_print("\n");
                return CMD_OK;
            }
            while (*c) {
                if (isspace((int)(*c))) {
                    c++;
                } else {
                    if (!isxdigit((unsigned)*c) ||
                        !isxdigit((unsigned)*(c+1))) {
                        soc_cm_print("MSPI: %s: Invalid character\n", ARG_CMD(a));
                        return(CMD_FAIL);
                    }
                    offset_size = sizeof(wdata);
                    if (wbytes >= offset_size) {
                        soc_cm_print("MSPI: %s: Data memory exceeded\n", ARG_CMD(a));
                        return(CMD_FAIL);
                    }
                    wdata[wbytes++] = (xdigit2i(*c) << 4) | xdigit2i(*(c + 1));
                    c += 2;
                }
            }
        }
        soc_cm_debug(DK_VERBOSE, "MSPI: Writing %d bytes\n", wbytes);
        if (soc_mspi_write8(unit, (uint8 *) &wdata[0], wbytes) != SOC_E_NONE) {
            soc_cm_print("MSPI: Write Fail\n");
            return CMD_FAIL;
        }
    }  else if (!sal_strcasecmp(c, "read")) {
        if (ARG_CNT(a) == 0) {
            return CMD_USAGE;
        }
        c = ARG_GET(a);
        if (!isint(c)) {
            return CMD_USAGE;
        }
        rbytes = parse_integer(c);
    
        soc_cm_debug(DK_VERBOSE, "MSPI: Reading %d bytes\n", rbytes);
        if (soc_mspi_read8(unit, (uint8 *) &rdata[0], rbytes) != SOC_E_NONE) {
            soc_cm_print("MSPI: Read Fail\n");
            return CMD_FAIL;
        }
        /* Dump It */
        for(start_byte=0; start_byte < rbytes; start_byte++) {
            soc_cm_print("%02x ", rdata[start_byte]);
            if ((start_byte % 8) == 7) {
                soc_cm_print("\n");
            }
        }
        soc_cm_print("\n");
    } else {
        return CMD_USAGE;
    }
    return rv;
}
#endif
