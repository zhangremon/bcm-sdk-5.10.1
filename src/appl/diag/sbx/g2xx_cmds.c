/*
 * $Id: g2xx_cmds.c 1.5 Broadcom SDK $
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
 * File:        g2xx_set_get.c
 * Purpose:     sbx commands for ucode tables
 * Requires:
 *
 */

#include <sal/core/libc.h>
#include <sal/appl/sal.h>
#include <appl/diag/system.h>
#include <appl/diag/parse.h>

#ifdef BCM_FE2000_SUPPORT

#include <bcm/error.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/g2xx/g2xx.h>
#include <soc/sbx/fe2000.h>

cmd_result_t
cmd_sbx_g2xx_get (int unit, args_t *args)
{
    int rv;
    soc_sbx_control_t *sbx;

    if (!SOC_IS_SBX_FE2000(unit)) {
        printk("This command will only work on an FE2000.\n");
        return CMD_FAIL;
    }

    sbx = SOC_SBX_CONTROL(unit);

    if (sbx->ucodetype != SOC_SBX_UCODE_TYPE_G2XX) {
        printk("ERROR: not running g2xx v1.3 microcode\n");
        return CMD_FAIL;
    }

    if (ARG_CNT(args) == 0) {
        return CMD_USAGE;
    }

    rv = soc_sbx_g2xx_shell_print(unit, ARG_CNT(args), &_ARG_CUR(args));
    if (rv == SOC_E_NONE) {
        ARG_DISCARD(args);
        rv = CMD_OK;
    } else if (rv == SOC_E_PARAM) {
        rv = CMD_USAGE;
    } else {
        rv = CMD_FAIL;
    }

    return rv;
}

cmd_result_t
cmd_sbx_g2xx_set (int unit, args_t *args)
{
    int rv;
    soc_sbx_control_t *sbx;

    if (!SOC_IS_SBX_FE2000(unit)) {
        printk("This command will only work on an FE2000.\n");
        return CMD_FAIL;
    }

    sbx = SOC_SBX_CONTROL(unit);

    if (sbx->ucodetype != SOC_SBX_UCODE_TYPE_G2XX) {
        printk("ERROR: not running g2xx v1.3 microcode\n");
        return CMD_FAIL;
    }

    if (ARG_CNT(args) == 0) {
        return CMD_USAGE;
    }

    rv = soc_sbx_g2xx_shell_set(unit, ARG_CNT(args), &_ARG_CUR(args));
    if (rv == SOC_E_NONE) {
        ARG_DISCARD(args);
        rv = CMD_OK;
    } else if (rv == SOC_E_PARAM) {
        rv = CMD_USAGE;
    } else {
        rv = CMD_FAIL;
    }

    return rv;
}

cmd_result_t
cmd_sbx_g2xx_delete (int unit, args_t *args)
{
    int rv;
    soc_sbx_control_t *sbx;

    if (!SOC_IS_SBX_FE2000(unit)) {
        printk("This command will only work on an FE2000.\n");
        return CMD_FAIL;
    }

    sbx = SOC_SBX_CONTROL(unit);

    if (sbx->ucodetype != SOC_SBX_UCODE_TYPE_G2XX) {
        printk("ERROR: not running g2xx v1.3 microcode\n");
        return CMD_FAIL;
    }

    if (ARG_CNT(args) == 0) {
        return CMD_USAGE;
    }

    rv = soc_sbx_g2xx_shell_delete(unit, ARG_CNT(args), &_ARG_CUR(args));
    if (rv == SOC_E_NONE) {
        ARG_DISCARD(args);
        rv = CMD_OK;
    } else if (rv == SOC_E_PARAM) {
        rv = CMD_USAGE;
    } else {
        rv = CMD_FAIL;
    }

    return rv;
}

char cmd_sbx_g2xx_util_usage[] =
"Usage:\n"
"  g2xxutil help               - displays this messge\n"
"  g2xxutil reloaducodebin [cfile] - disable lrp, load ucode from binary, then enable it\n"
;

cmd_result_t
cmd_sbx_g2xx_util(int unit, args_t *a)
{
    char *cmd, *subcmd;
    cmd_result_t rv = CMD_OK;
    soc_sbx_control_t *sbx;

    if (!SOC_IS_SBX_FE2000(unit)) {
        printk("This command will only work on an FE2000.\n");
        return CMD_FAIL;
    }

    sbx = SOC_SBX_CONTROL(unit);

    cmd = ARG_CMD(a);
    if (!sh_check_attached(cmd, unit)) {
        return CMD_FAIL;
    }

    if ((subcmd = ARG_GET(a)) == NULL) {
        return CMD_USAGE;
    }

    if (sbx->ucodetype != SOC_SBX_UCODE_TYPE_G2XX) {
        printk("ERROR: not running g2xx v1.3 microcode\n");
        return CMD_FAIL;
    }

    if (sal_strcasecmp(subcmd, "help") == 0) {
      rv = CMD_USAGE;
    } else if (sal_strcasecmp(subcmd, "reloaducodebin") == 0) {
#ifndef NO_FILEIO
      char *ucodeFile, *subcmd;
      unsigned int rlen=0;
      unsigned int rcount=0;
      unsigned int rvalue;
      FILE *rf;
      unsigned char *rbuf=NULL, *tmpbuf;
      soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);
      int c_file=0;
      int err;

      if ((ucodeFile = ARG_GET(a)) == NULL) {
	printk("Missing ucodeFile\n");
	return CMD_USAGE;
      }

      if ((subcmd = ARG_GET(a)) != NULL) {
        if (sal_strcasecmp(subcmd, "cfile") == 0) {
           c_file = 1;
        }
      }

      rf = fopen(ucodeFile, "rb");
      if(rf == 0) {
	  printf("didn't find the binary ucode image.\n");
	  return SB_UCODE_BAD_IMAGE_ERR_CODE;
      }

      if (c_file) {
        char ch;
        int  size_found=0;
        while((ch = (char)fgetc(rf))!='}') {
          if ((ch=='[') && (!size_found)) {
              fscanf(rf,"%d", &rlen);
              rbuf = (unsigned char*)sal_alloc(rlen, "ucode_buffer");
              if(!rbuf) {
                  printf("system error: malloc (...) \n");
                  return SOC_E_RESOURCE;
              }
              rcount = 0;
              size_found = 1;
          }
          if (((ch=='{') || (ch==',')) && (size_found)) {
              fscanf(rf,"%x", &rvalue);
              rbuf[rcount] = (unsigned char)rvalue;
              rcount++;
          }
        }
        printf("Load ucode from C file %s for %d bytes\n", ucodeFile, rcount);
      } else {
        err = fseek(rf, 0, SEEK_END);
        if (err == -1) {
            printf ("fseek returned error\n");
            fclose(rf);
            return err;
        }
        rlen = ftell(rf);
        if (rlen == -1) {
            printf ("ftell returned error\n");
            fclose(rf);
            return rlen;
        }
        rewind(rf);
        rbuf = (unsigned char*)sal_alloc(rlen, "ucode_buffer");
        if(!rbuf) {
            printf("system error: malloc (...) \n");
            rv = SOC_E_RESOURCE;
            fclose(rf);
            return rv;
        }else{
            tmpbuf = rbuf;
            fread (tmpbuf, rlen, 1, rf);
        }
      }

      fclose(rf);

      err = soc_sbx_fe2000_ucode_load_from_buffer(unit, rbuf, rlen);
      if(err != SB_OK)
      {
	printf("Load ucode from buffer failed with error code: %d\n", err);
	sal_free(rbuf);
        return err;
      }

      if (sbFe2000UcodeLoad(SOC_SBX_SBHANDLE(unit), sbx->ucode) ) {
          printf("soc_sbx_fe2000_ucode_load failed\n");
      }

      sal_free(rbuf);
#endif
    } else {
        return CMD_USAGE;
    }

    return rv;
}

#endif /* BCM_FE2000_SUPPORT */
