/*
 * $Id: soc.c 1.1.2.6 Broadcom SDK $
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
#include <bcm/filter.h>

#include <soc/mem.h>
#include <soc/ea/onu.h>

/*
 * Function:    cmd_ea_soc
 * Purpose: Print soc control information if compiled in debug mode.
 * Parameters:  u - unit #
 *              a - pointer to args, expects <unit> ...., if none passed,
 *                  default unit # used.
 * Returns: CMD_OK/CMD_FAIL
 */
cmd_result_t
cmd_ea_soc(int u, args_t *a)
{
#if defined(BROADCOM_DEBUG)
    char *subcmd;

	if (! sh_check_attached(ARG_CMD(a), u)) {
		return CMD_FAIL;
	}

	if ((subcmd = ARG_GET(a)) == NULL) {
		return CMD_USAGE;
	}
    
    if(!sal_strncasecmp(subcmd, "debug",5)){
        uint32 dbg_msk = 0;
        if ((subcmd = ARG_GET(a)) == NULL) {
		    return CMD_USAGE;
	    }
        dbg_msk = parse_integer(subcmd);
        soc_ea_dbg_level_set(dbg_msk);
        return CMD_OK;   
    }else if(!sal_strncasecmp(subcmd, "dbgdump",7)){
       soc_ea_dbg_level_dump();
       return CMD_OK;
    }else if(!sal_strncasecmp(subcmd, "gpio",4)){
        uint32 mask;
        uint32 value;
        uint32 flag;

        if ((subcmd = ARG_GET(a)) == NULL) {
		    return CMD_USAGE;
	    }
        if(!sal_strncasecmp(subcmd, "write",5)){
            if ((subcmd = ARG_GET(a)) == NULL) {
    		    return CMD_USAGE;
    	    }
            
            flag = parse_integer(subcmd);
            
            if ((subcmd = ARG_GET(a)) == NULL) {
    		    return CMD_USAGE;
    	    }
           
            mask = parse_integer(subcmd);
            if ((subcmd = ARG_GET(a)) == NULL) {
    		    return CMD_USAGE;
    	    }
            value = parse_integer(subcmd);
            if(SOC_E_NONE != soc_gpio_write(u,flag,mask,value)){
                return CMD_FAIL;
            }else{
                return CMD_OK;
            }
        }else if(!sal_strncasecmp(subcmd, "read",4)){
            if ((subcmd = ARG_GET(a)) == NULL) {
    		    return CMD_USAGE;
    	    }
            flag = parse_integer(subcmd);
            
            if(SOC_E_NONE != soc_gpio_read(u,flag,&value)){
                return CMD_FAIL;
            }else{
                printk("gipo value = %08x\n",value);
                return CMD_OK;
            }
        }else{
            return CMD_USAGE;
        }
    }else if(!sal_strncasecmp(subcmd, "reset",5)){
        if(soc_chip_reset(u) != SOC_E_NONE){
            return CMD_FAIL;
        }else{
            return CMD_OK;
        } 
    }else if(!sal_strncasecmp(subcmd, "nvserase",8)){
        if(soc_nvs_erase(u) != SOC_E_NONE){
            return CMD_FAIL;
        }else{
            return CMD_OK;
        } 
    }else if(!sal_strncasecmp(subcmd,"dbshow",6)){
        soc_ea_private_db_dump(u);
        return CMD_OK;
    }else{
        return CMD_USAGE;
    }
	return CMD_OK;
#else
    printk("%s: Warning: Not compiled with BROADCOM_DEBUG enabled\n", ARG_CMD(a));
    return(CMD_OK);
#endif
}
