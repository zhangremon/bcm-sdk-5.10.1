/*
 * $Id: filter.c 1.8 Broadcom SDK $
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
 * File:        filter.c
 * Purpose:	BCMX diagnostic filter command
 */

#ifdef	INCLUDE_BCMX

#include <bcm/types.h>
#include <bcm/error.h>
#include <bcmx/filter.h>
#include <bcmx/lplist.h>

#include <appl/diag/parse.h>
#include <appl/diag/shell.h>
#include <appl/diag/system.h>
#include "bcmx.h"

#ifdef BCM_FILTER_SUPPORT

char bcmx_cmd_filter_usage[] =
	"\n"
	"\tfilter init\n"
	"\tfilter create\n"
	"\tfilter copy <fid>\n"
	"\tfilter destroy <fid>\n"
	"\tfilter install <fid>\n"
	"\tfilter reinstall <fid>\n"
	"\tfilter remove <fid>\n"
	"\tfilter prio <fid> <prio>\n"
	"\tfilter action <fid> <val> <param>\n"
	"\tfilter data <fid> <offset> <value> <mask>\n";

cmd_result_t
bcmx_cmd_filter(int unit, args_t *args)
{
    char		*cmd, *subcmd, *s;
    int			rv;
    bcm_filterid_t	fid, fid2;

    cmd = ARG_CMD(args);
    subcmd = ARG_GET(args);
    if (subcmd == NULL) {
	subcmd = "show";
    }

    if (sal_strcasecmp(subcmd, "show") == 0) {
	sal_printf("%s: ERROR: not implemented. dump individual tables?\n",
		   cmd);
	return CMD_FAIL;
    }

    if (sal_strcasecmp(subcmd, "init") == 0) {
	rv = bcmx_filter_init();
	if (rv < 0) {
	    sal_printf("%s: ERROR: %s failed: %s\n",
		       cmd, subcmd, bcm_errmsg(rv));
	    return CMD_FAIL;
	}
	return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "create") == 0) {
	rv = bcmx_filter_create(&fid);
	if (rv < 0) {
	    sal_printf("%s: ERROR: %s failed: %s\n",
		       cmd, subcmd, bcm_errmsg(rv));
	    return CMD_FAIL;
	}
	sal_printf("[filter 0x%x created]\n", fid);
	return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "copy") == 0) {
	s = ARG_GET(args);
	if (s == NULL) {
	    sal_printf("%s: ERROR: %s missing filter-id argument\n",
		       cmd, subcmd);
	    return CMD_FAIL;
	}
	fid = parse_integer(s);
	rv = bcmx_filter_copy(fid, &fid2);
	if (rv < 0) {
	    sal_printf("%s: ERROR: %s failed: %s\n",
		       cmd, subcmd, bcm_errmsg(rv));
	    return CMD_FAIL;
	}
	sal_printf("[filter 0x%x copied to 0x%x]\n", fid, fid2);
	return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "destroy") == 0) {
	s = ARG_GET(args);
	if (s == NULL) {
	    sal_printf("%s: ERROR: %s missing filter-id argument\n",
		       cmd, subcmd);
	    return CMD_FAIL;
	}
	fid = parse_integer(s);
	rv = bcmx_filter_destroy(fid);
	if (rv < 0) {
	    sal_printf("%s: ERROR: %s failed: %s\n",
		       cmd, subcmd, bcm_errmsg(rv));
	    return CMD_FAIL;
	}
	return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "install") == 0) {
	s = ARG_GET(args);
	if (s == NULL) {
	    sal_printf("%s: ERROR: %s missing filter-id argument\n",
		       cmd, subcmd);
	    return CMD_FAIL;
	}
	fid = parse_integer(s);
	rv = bcmx_filter_install(fid);
	if (rv < 0) {
	    sal_printf("%s: ERROR: %s failed: %s\n",
		       cmd, subcmd, bcm_errmsg(rv));
	    return CMD_FAIL;
	}
	return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "reinstall") == 0) {
	s = ARG_GET(args);
	if (s == NULL) {
	    sal_printf("%s: ERROR: %s missing filter-id argument\n",
		       cmd, subcmd);
	    return CMD_FAIL;
	}
	fid = parse_integer(s);
	rv = bcmx_filter_reinstall(fid);
	if (rv < 0) {
	    sal_printf("%s: ERROR: %s failed: %s\n",
		       cmd, subcmd, bcm_errmsg(rv));
	    return CMD_FAIL;
	}
	return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "remove") == 0) {
	s = ARG_GET(args);
	if (s == NULL) {
	    sal_printf("%s: ERROR: %s missing filter-id argument\n",
		       cmd, subcmd);
	    return CMD_FAIL;
	}
	fid = parse_integer(s);
	rv = bcmx_filter_remove(fid);
	if (rv < 0) {
	    sal_printf("%s: ERROR: %s failed: %s\n",
		       cmd, subcmd, bcm_errmsg(rv));
	    return CMD_FAIL;
	}
	return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "prio") == 0) {
	int	prio;

	s = ARG_GET(args);
	if (s == NULL) {
	    sal_printf("%s: ERROR: %s missing filter-id argument\n",
		       cmd, subcmd);
	    return CMD_FAIL;
	}
	fid = parse_integer(s);

	s = ARG_GET(args);
	if (s == NULL) {
	    sal_printf("%s: ERROR: %s missing priority argument\n",
		       cmd, subcmd);
	    return CMD_FAIL;
	}
	prio = parse_integer(s);

	rv = bcmx_filter_qualify_priority(fid, prio);
	if (rv < 0) {
	    sal_printf("%s: ERROR: %s failed: %s\n",
		       cmd, subcmd, bcm_errmsg(rv));
	    return CMD_FAIL;
	}
	return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "action") == 0) {
	uint32	val, param;

	s = ARG_GET(args);
	if (s == NULL) {
	    sal_printf("%s: ERROR: %s missing filter-id argument\n",
		       cmd, subcmd);
	    return CMD_FAIL;
	}
	fid = parse_integer(s);
	
	s = ARG_GET(args);
	if (s == NULL) {
	    sal_printf("%s: ERROR: %s missing action-val argument\n",
		       cmd, subcmd);
	    return CMD_FAIL;
	}
	if (sal_strcasecmp(s, "copytocpu") == 0) {
	    val = bcmActionCopyToCpu;
	} else {
	    val = parse_integer(s);
	}

	s = ARG_GET(args);
	if (s == NULL) {
	    param = 0;
	} else {
	    param = parse_integer(s);
	}

	rv = bcmx_filter_action_match(fid, (bcm_filter_action_t) val, param);
	if (rv < 0) {
	    sal_printf("%s: ERROR: %s failed: %s\n",
		       cmd, subcmd, bcm_errmsg(rv));
	    return CMD_FAIL;
	}
	return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "data") == 0) {
	int	offset, len;
	char	shex[5];
	uint8	value[64], mask[64];
	char	*vs, *ms;

	s = ARG_GET(args);
	if (s == NULL) {
	    sal_printf("%s: ERROR: %s missing filter-id argument\n",
		       cmd, subcmd);
	    return CMD_FAIL;
	}
	fid = parse_integer(s);
	
	s = ARG_GET(args);
	if (s == NULL) {
	    sal_printf("%s: ERROR: %s missing offset argument\n",
		       cmd, subcmd);
	    return CMD_FAIL;
	}
	offset = parse_integer(s);

	vs = ARG_GET(args);
	ms = ARG_GET(args);
	len = 0;
	shex[0] = '0';
	shex[1] = 'x';
	shex[2] = '0';
	shex[3] = '0';
	shex[4] = '\0';
	while (vs != NULL && *vs != '\0') {
	    shex[2] = *vs++;
	    shex[3] = *vs++;
	    value[len] = parse_integer(shex);
	    if (ms == NULL) {
		mask[len] = 0xff;
	    } else {
		shex[2] = *ms++;
		shex[3] = *ms++;
		mask[len] = parse_integer(shex);
	    }
	    len += 1;
	}
	
	rv = bcmx_filter_qualify_data(fid, offset, len, value, mask);
	if (rv < 0) {
	    sal_printf("%s: ERROR: %s failed: %s\n",
		       cmd, subcmd, bcm_errmsg(rv));
	    return CMD_FAIL;
	}
	return CMD_OK;
    }

    sal_printf("%s: unknown subcommand '%s'\n", ARG_CMD(args), subcmd);
    return CMD_USAGE;
}

#endif /* BCM_FILTER_SUPPORT */

#endif	/* INCLUDE_BCMX */
