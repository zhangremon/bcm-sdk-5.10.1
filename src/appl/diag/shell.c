/*
 * $Id: shell.c 1.128.6.11 Broadcom SDK $
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
 * File: 	shell.c
 * Purpose: 	Command parsing and dispatch.
 */

#include <sal/types.h>

#ifndef NO_SAL_APPL

#if defined(VXWORKS)
#include <vxWorks.h>
#include <version.h>
#include "config.h"
#endif

#ifndef __KERNEL__
#include <stdio.h>
#include <time.h>
#endif

#ifndef NO_CTRL_C
#include <setjmp.h>
#include <signal.h>
#endif


#endif /* NO_SAL_APPL */

#include <assert.h>

#include <sal/core/libc.h>
#include <sal/core/boot.h>
#include <sal/appl/sal.h>
#include <sal/appl/io.h>
#include <sal/appl/config.h>

#include <soc/debug.h>
#include <soc/mem.h>

#include <soc/mcm/driver.h>
#include <soc/mcm/robo/driver.h>
#if defined(BCM_EASYRIDER_SUPPORT) || defined(BCM_TRIUMPH_SUPPORT)
#include <soc/er_tcam.h>
#endif

#include <bcm/init.h>
#include <bcm/error.h>
#include <bcm/port.h>
#include <bcm/debug.h>
#include <bcm/vlan.h>
#include <bcm/types.h>
#include <bcm/stg.h>
#include <bcm/link.h>
#include <bcm/rx.h>
#ifdef INCLUDE_MACSEC
#include <bcm/macsec.h>
#include <bcm_int/common/macsec_cmn.h>
#endif /* INCLUDE_MACSEC */

#include <appl/diag/shell.h>
#include <appl/diag/system.h>
#include <appl/diag/sysconf.h>
#include <appl/diag/debug.h>
#ifdef BCM_WARM_BOOT_SUPPORT
#include <appl/diag/warmboot.h>
#endif /* BCM_WARM_BOOT_SUPPORT */

#if defined(INCLUDE_BCMX)
#include <bcmx/debug.h>
#endif

#if defined(INCLUDE_LIB_CPUDB)
#include <appl/cpudb/debug.h>
#endif

#ifdef        INCLUDE_BOARD
#include <board/debug.h>
#endif


#if defined(BCM_SBX_SUPPORT)
#include <soc/sbx/sbx_drv.h>
#endif

#include <soc/phy/phyctrl.h>

#include <appl/diag/infix.h>
#include <appl/diag/diag.h>

#include <shared/shr_resmgr.h>

int	sh_rcload_depth = 0;

/*
 * Variables manipulated by the shell 'set' command
 *
 * NOTE ON ADDING VARIABLES:
 *	(1) Add sh_set_xxx variable (below)
 *	(2) Add extern for sh_set_xxx (in shell.h)
 *	(3) Add parse_table_add() call in sh_set()
 */

int	sh_set_rcload       = TRUE;	/* TRUE --> alias to rcload */
int	sh_set_rcerror      = TRUE;	/* TRUE --> Stop rcload on error */
int	sh_set_lperror      = TRUE;	/* TRUE --> Stop loop on error */
int	sh_set_iferror      = TRUE;	/* TRUE --> Stop if on error */
int	sh_set_more_lines   = 20; 	/* # lines for MORE */
int	sh_set_report_time  = 5; 	/* Test status report interval (sec) */
int	sh_set_report_status = TRUE; 	/* Allows disabling reporting */
#ifdef __KERNEL__
int	sh_set_rctest       = FALSE;	/* FALSE--> No Auto RC */
#else
int	sh_set_rctest       = TRUE;	/* TRUE --> tr tests load */
#endif

static volatile int override_unit = FALSE;

/*
 * Set environment variables pertinent to a new current unit, such as
 * chip name.  Removes those pertaining to an old unit.
 */

void
sh_swap_unit_vars(int new_unit)
{
    static int old_unit = -1;
    char tmp[16];
#ifdef INCLUDE_EDITLINE
    extern int diag_list_possib_unit;
#endif
    extern int diag_user_var_unit;

    if (new_unit != old_unit) {
	if (old_unit >= 0) {
	    sal_sprintf(tmp, "unit%d", old_unit);
        if (NULL != SOC_CONTROL(old_unit)) {
            var_unset(SOC_CHIP_STRING(old_unit), FALSE, TRUE, FALSE);
            var_unset(soc_dev_name(old_unit), FALSE, TRUE, FALSE);
        }
        var_unset(tmp, FALSE, TRUE, FALSE);
        var_unset("devname", FALSE, TRUE, FALSE);
        var_unset("pcidev", FALSE, TRUE, FALSE);
        var_unset("pcirev", FALSE, TRUE, FALSE);
        var_unset("rcpu_only", FALSE, TRUE, FALSE);
	}

	if (new_unit >= 0) {
	    uint16	dev_id;
	    uint8	rev_id;
            char        *chip_string;

            chip_string = SOC_CHIP_STRING(new_unit);
            if (soc_cm_get_id(new_unit, &dev_id, &rev_id) >= 0) {
		sal_sprintf(tmp, "0x%04x", dev_id);
		var_set("pcidev", tmp, FALSE, FALSE);
		sal_sprintf(tmp, "0x%02x", rev_id);
		var_set("pcirev", tmp, FALSE, FALSE);
                if (dev_id == BCM56620_DEVICE_ID) {
                    chip_string = "triumph"; /* SOC_CHIP_STRING is valkyrie */
                }
                if (dev_id == BCM56630_DEVICE_ID) {
                    chip_string = "triumph2"; /* SOC_CHIP_STRING is apollo */
                }
                if (dev_id == BCM56526_DEVICE_ID) {
                    chip_string = "apollo"; /* SOC_CHIP_STRING is triumph2 */
                }
                if (dev_id == BCM56538_DEVICE_ID) {
                    chip_string = "firebolt3"; /* SOC_CHIP_STRING is triumph2 */
                }
                if (dev_id == BCM56534_DEVICE_ID) {
                    chip_string = "firebolt3"; /* SOC_CHIP_STRING is apollo */
                }
        }
        var_set_integer(chip_string, 1, FALSE, FALSE);
        var_set_integer(soc_dev_name(new_unit), 1, FALSE, FALSE);
        sal_sprintf(tmp, "unit%d", new_unit);
        var_set_integer(tmp, 1, FALSE, FALSE);
        var_set("devname", (char *)soc_dev_name(new_unit), FALSE, FALSE);
        if (SOC_IS_RCPU_ONLY(new_unit)) {
            var_set_integer("rcpu_only", 1, FALSE, FALSE);
        }
	}

	if (new_unit >= 0) {
	    var_set_integer("unit", new_unit, 0, 0);
        if (SOC_IS_ROBO(new_unit)) {
            command_mode_set(ROBO_CMD_MODE);
#if defined(BCM_SBX_SUPPORT)
        } else if (SOC_IS_SBX(new_unit)) {
            command_mode_set(SBX_CMD_MODE);
#endif
#if defined(BCM_EA_SUPPORT)
        } else if(SOC_IS_EA(new_unit)) {
            command_mode_set(EA_CMD_MODE);
#endif
        } else {
            command_mode_set(ESW_CMD_MODE);
        }
	} else {
	    var_unset("unit", FALSE, TRUE, FALSE);
	}

	old_unit = new_unit;
    }

#ifdef INCLUDE_EDITLINE
    diag_list_possib_unit = new_unit;
#endif
    diag_user_var_unit = new_unit;
}

/*
 * Variable:	sh_ctrl_c_cnt
 * Purpose:	Index indicating current top of stack for pushed
 *		Control-C handlers. "-1" Indicates no handlers pushed.
 */
#ifndef NO_CTRL_C
static volatile int 	sh_ctrl_c_cnt = -1;
#endif

/*
 * Variable:	sh_ctrl_c_stack
 * Purpose:	Tracks pushed control-C handlers. Both the jmpbuf and the
 *		the setting thread is tracked.
 */
#ifndef NO_CTRL_C
static volatile struct {
    jmp_buf		*pc_jmpbuf;	/* Pointer to jmpbuf for longjmp */
    sal_thread_t	pc_thread;	/* Thread that pushed the entry */
} sh_ctrl_c_stack[PUSH_CTRL_C_CNT];
#endif

/* ARGSUSED */
void
sh_ctrl_c_handler(int sig)
/*
 * Function: 	sh_ctrl_c_handler
 * Purpose:	Actual handler called when Control-C occurs.
 * Parameters:	sig - not used.
 * Returns:	Nothing.
 */
{
#ifndef NO_CTRL_C
    COMPILER_REFERENCE(sig);
    assert(sh_ctrl_c_cnt >= 0);
    if (sal_thread_self() != sh_ctrl_c_stack[sh_ctrl_c_cnt].pc_thread) {
        printk("ERROR: thread 0x%lx took my Control-C!!\n",(unsigned long) sal_thread_self());
	return;
    }
    signal(SIGINT, sh_ctrl_c_handler); /* Enable now */
    printk("\nInterrupt\n");
    longjmp(*sh_ctrl_c_stack[sh_ctrl_c_cnt].pc_jmpbuf, 1);
#endif
}

void
sh_ctrl_c_take(void)
/*
 * Function: 	sh_ctrl_c_take
 * Purpose:	Fake a control-C exception (useful for assert).
 * Parameters:	none.
 * Returns:	Does not return.
 */
{
#ifndef NO_CTRL_C
    printk("\n");
    if (sh_ctrl_c_cnt >= 0) {
	longjmp(*sh_ctrl_c_stack[sh_ctrl_c_cnt].pc_jmpbuf, 1);
    }
    exit(1);
#endif
}

void
sh_push_ctrl_c(jmp_buf *jb)
/*
 * Function: 	sh_push_ctrl_c
 * Purpose:	Push a control C long jump buffer onto the handler stack.
 * Parameters:	jb - pointer to long jump buffer (jmp_buf).
 * Returns:	Nothing
 */
{
#ifndef NO_CTRL_C
    if (sal_thread_self() == sal_thread_main_get()) {
        signal(SIGINT, SIG_IGN);
        assert(sh_ctrl_c_cnt < (PUSH_CTRL_C_CNT - 1));
        sh_ctrl_c_cnt++;
        sh_ctrl_c_stack[sh_ctrl_c_cnt].pc_jmpbuf = jb;
        sh_ctrl_c_stack[sh_ctrl_c_cnt].pc_thread = sal_thread_self();
        signal(SIGINT, sh_ctrl_c_handler); /* Enable now */
    }
#endif
}

void
sh_pop_ctrl_c(void)
/*
 * Function: 	sh_pop_ctrl_c
 * Purpose:	Pop an control-c entry off the handler stack.
 * Parameters:	None.
 * Returns:	Nothing.
 */
{
#ifndef NO_CTRL_C
    if (sal_thread_self() == sal_thread_main_get()) {
        signal(SIGINT, SIG_IGN);
        sh_ctrl_c_cnt--;
        if (0 > sh_ctrl_c_cnt) {
            signal(SIGINT, SIG_DFL);
        } else {
            signal(SIGINT, sh_ctrl_c_handler); /* Enable now */
        }
    }
#endif
}

void
sh_block_ctrl_c(int tf)
{
#ifndef NO_CTRL_C
    signal(SIGINT, tf ? SIG_IGN : sh_ctrl_c_handler);
#endif
}

int
sh_check_attached(const char *pfx, const int u)
/*
 * Function: 	sh_check_attached
 * Purpose:	Check if a unit is attached, and print error if not.
 * Parameters:	pfx - Prefix to error message.
 *		u - unit #.
 * Returns:	FALSE - not attached, TRUE - attached.
 */
{
    if (0 > u) {			/* No unit set */
	printk("%s: Error: No default unit\n", pfx);
	return(FALSE);
    } else if (!soc_attached(u)) {
	/* Not attached, print out error */
	if (override_unit)
	    return TRUE;
	printk("%s: Error: Unit %d not attached\n", pfx, u);
	return(FALSE);
    }
    return(TRUE);
}

#ifndef	NO_FILEIO
typedef struct rccache_s {
    char		*name;
    char		*value;
    struct rccache_s	*next;
} rccache_t;

static rccache_t	*rccache_head;

static char *
rccache_lookup(char *fname)
{
    rccache_t	*r;

    for (r = rccache_head; r != NULL; r = r->next) {
	if (strcmp(fname, r->name) == 0) {
	    return r->value;
	}
    }
    return NULL;
}
#endif	/* NO_FILEIO */


#ifndef NO_SAL_APPL

char sh_rccache_usage[] =
	"Usage:\n"
	"\trccache show [name]   - show cached rc files or contents of name\n"
	"\trccache add name      - add rc file 'name' to cached files\n"
	"\trccache addq name     - like add but only if not already cached\n"
	"\trccache update name   - reload rc file 'name' into the cache\n"
	"\trccache delete name   - delete cached rc file 'name'\n"
	"\trccache clear         - delete all cached rc files\n";

cmd_result_t
sh_rccache(int u, args_t *a)
{
#ifdef	NO_FILEIO
    printk("%s: not supported without file I/O\n", ARG_CMD(a));
    return CMD_FAIL;
#else
    char	*subcmd;
    rccache_t	*r, *prevr;
    char	*name, *s, *s2;
    FILE	*fp;
    char	buf[1024];
    int		fb, sb, upd;
    char        *path, *p;
    char        fn[ARGS_BUFFER + 8];

    subcmd = ARG_GET(a);
    if (subcmd == NULL) {
	subcmd = "show";
    }
    if (sal_strcasecmp(subcmd, "show") == 0) {
	name = ARG_GET(a);
	if (name == NULL) {
	    for (r = rccache_head; r != NULL; r = r->next) {
		printk("cached %s (%d bytes)\n", r->name,
                       (int)strlen(r->value));
	    }
	    return CMD_OK;
	}
	s = rccache_lookup(name);
	if (s == NULL) {
	    printk("%s: ERROR: %s is not cached\n", ARG_CMD(a), name);
	    return CMD_FAIL;
	}
	printk("%s: cached %s (%d bytes)\n%s\n",
	       ARG_CMD(a), name, (int)strlen(s), s);
	return CMD_OK;
    }
    upd = 0;
    if (sal_strcasecmp(subcmd, "update") == 0 ||
	sal_strcasecmp(subcmd, "upd") == 0) {
	upd = 1;
	subcmd = "add";
    }
    if (sal_strcasecmp(subcmd, "add") == 0 ||
	sal_strcasecmp(subcmd, "addq") == 0) {
	name = ARG_GET(a);
	if (name == NULL) {
	    printk("%s: ERROR: missing file name\n", ARG_CMD(a));
	    return CMD_USAGE;
	}

	for (r = rccache_head; r != NULL; r = r->next) {
	    if (strcmp(r->name, name) == 0) {
		break;
	    }
	}

	if (upd == 0 && r != NULL) {
	    if (sal_strcasecmp(subcmd, "addq") == 0) {
		return CMD_OK;
	    }
	    printk("%s: ERROR: %s already cached\n", ARG_CMD(a), name);
	    return CMD_FAIL;
	}

	if (r == NULL) {
	    r = sal_alloc(sizeof(*r), "rccache");
	    if (r == NULL) {
		printk("%s: ERROR: out of memory\n", ARG_CMD(a));
		return CMD_FAIL;
	    }
	    r->name = NULL;
	    r->value = NULL;
	    r->next = NULL;
	}

	if ((path = var_get("path")) == NULL) {
	    path = ".";
	}

	fp = NULL;
	while (*path != 0)
    {
        int character_count = 0;

	    p = fn;

	    while (*path != ' ' && *path != 0)
        {
            *p++ = *path++;
            ++character_count;
	    }

	    if (p > fn)
        {
            *p++ = '/';
            ++character_count;
	    }

	    sal_strncpy(p, name, ARGS_BUFFER + 8 - character_count - 1);
        p[ARGS_BUFFER + 8 - character_count - 1] = 0;

	    if ((fp = sal_fopen(fn, "r")) != NULL)
        {
            break;
	    }

	    while (*path == ' ')
        {
            path++;
	    }
	}

	if (fp == NULL) {
	    printk("%s: ERROR: %s: file not found\n", ARG_CMD(a), name);
	    if (r->name == NULL) {
		sal_free(r);
	    }
	    return CMD_FAIL;
	}
	s = NULL;
	sb = 0;
	while ((fb = fread(buf, 1, sizeof(buf), fp)) > 0) {
	    s2 = sal_alloc(sb + fb + 1, "rccache-read");
	    if (s2 == NULL) {
		printk("%s: ERROR: out of memory\n", ARG_CMD(a));
		sal_free(r);
		if (s != NULL) {
		    sal_free(s);
		}
		return CMD_FAIL;
	    }
	    if (sb) {
		sal_memcpy(s2, s, sb);
	    }
	    sal_memcpy(&s2[sb], buf, fb);
	    s2[sb+fb] = '\0';
	    if (s != NULL) {
		sal_free(s);
	    }
	    s = s2;
	    sb += fb;
	}
	sal_fclose(fp);
	if (sb == 0) {
	    printk("%s: ERROR: %s: file is empty\n", ARG_CMD(a), name);
	    if (r->name == NULL) {
		sal_free(r);
	    }
	    if (s != NULL) {
		sal_free(s);
	    }
	    return CMD_FAIL;
	}
	if (r->name == NULL) {		/* new entry */
	    r->name = sal_strdup(name);
	    r->value = sal_strdup(s);
	    r->next = rccache_head;
	    rccache_head = r;
	} else {			/* update */
	    sal_free(r->value);
	    r->value = sal_strdup(s);
	}
	sal_free(s);
	return CMD_OK;
    }
    if (sal_strcasecmp(subcmd, "delete") == 0 ||
	sal_strcasecmp(subcmd, "del") == 0) {
	name = ARG_GET(a);
	if (name == NULL) {
	    printk("%s: ERROR: missing cached name\n", ARG_CMD(a));
	    return CMD_USAGE;
	}
	prevr = NULL;
	for (r = rccache_head; r != NULL; prevr = r, r = r->next) {
	    if (strcmp(r->name, name) != 0) {
		continue;
	    }
	    if (prevr == NULL) {
		rccache_head = r->next;
	    } else {
		prevr->next = r->next;
	    }
	    sal_free(r->name);
	    sal_free(r->value);
	    sal_free(r);
	    return CMD_OK;
	}
	printk("%s: ERROR: %s is not cached\n", ARG_CMD(a), name);
	return CMD_FAIL;
    }
    if (sal_strcasecmp(subcmd, "clear") == 0) {
	name = ARG_GET(a);
	if (name != NULL) {
	    printk("%s: ERROR: clear takes no arguments\n", ARG_CMD(a));
	    return CMD_USAGE;
	}
	while (rccache_head != NULL) {
	    r = rccache_head;
	    rccache_head = r->next;
	    sal_free(r->name);
	    sal_free(r->value);
	    sal_free(r);
	}
	return CMD_OK;
    }
    return CMD_USAGE;
#endif	/* NO_FILEIO */
}

cmd_result_t
sh_rcload_file(int u, args_t *a, char *f, int add_rc)
/*
 * Function: 	sh_rcload_file
 * Purpose:	Load and execute commands from a file.
 * Parameters:	u - default unit number for commands, may be -1 if none
 *		a - Pointer to args, IF non-null, remaining args
 *		    in a are pushed in a local scope as $1, $2 etc.
 *		f - file name to load
 *		add_rc - if true, filename must end in ".soc", appended
 *			if not there.
 * Returns:	CMD_OK - all commands executed.
 *		CMD_FAIL- some command failed and aborted file processing.
 */
{
#ifdef NO_FILEIO
    if(strstr(f, ".soc")) {
	printk("proxyecho rcload %s\n", f);
    }
    return 0;
#else

    static char fn[ARGS_BUFFER + 8];
    char *path, *p, *add_ext;
    static char line[ARGS_BUFFER], *s, *t;
    void * volatile scope = NULL;
    jmp_buf ctrl_c;

    /* These guys volatile because of long jump */

    volatile cmd_result_t rv = CMD_OK;
    FILE * volatile 	  fp = NULL;
    char * volatile	  fstr = NULL;
    volatile int  	  lineNum = 0;

    scope = var_push_scope(); /* Push current scope */

    if (!setjmp(ctrl_c)) {
	sh_push_ctrl_c(&ctrl_c);
	rv = CMD_OK;
    } else {
	rv = CMD_INTR;
	goto done;
    }

    if (++sh_rcload_depth == RCLOAD_DEPTH_MAX) {
	printk("rcload: recursion too deep\n");
	goto done;
    }

    (void)var_set_integer("?", CMD_OK, TRUE, FALSE);

    /* If arguments - add to local scope */

    if (a) {
	int	v_num;
	char	v_str[12];
	char	*v_val;

	for (v_num = 1,v_val = ARG_GET(a); v_val; v_val=ARG_GET(a),v_num++){
	    sal_sprintf(v_str, "%d", v_num);
	    var_set(v_str, v_val, TRUE, FALSE);
	}
    }

    /*
     * Determine suffix (extension SOC_SCRIPT_EXT = .soc) to add to file name
     */

    add_ext = "";

    if (add_rc &&
	(strlen(f) <= strlen(SOC_SCRIPT_EXT) ||
	 strcmp(f + strlen(f) - strlen(SOC_SCRIPT_EXT), SOC_SCRIPT_EXT))) {

	add_ext = SOC_SCRIPT_EXT;
    }

    /*
     * If name of script has a colon or slash, open it as-is.
     * Otherwise search for script in $path.
     */

    sal_strcpy(fn, f);
    strcat(fn, add_ext);


    if (strchr(fn, '/') != NULL || strchr(fn, ':') != NULL) {
	fp = sal_fopen(fn, "r");
    } else if ((fstr = rccache_lookup(fn)) == NULL) {
	if ((path = var_get("path")) == NULL) {
	    path = ".";
	}

	while (*path != 0) {
	    p = fn;

	    while (*path != ' ' && *path != 0) {
		*p++ = *path++;
	    }

	    if (p > fn) {
		*p++ = '/';
	    }

	    sal_strcpy(p, f);

	    strcat(fn, add_ext);

	    if ((fp = sal_fopen(fn, "r")) != NULL) {
		break;
	    }

	    while (*path == ' ') {
		path++;
	    }
	}
    }

    if (fp == NULL && fstr == NULL) {
	rv = CMD_NFND;
	goto done;
    }

    for (;;) {
	s = line;
	for (;;) {
	    lineNum++;
	    if (fp != NULL) {
		if (fgets(s, line + sizeof (line) - s, fp) == 0) {
		    goto done;
		}
	    } else {
		int left = line + sizeof(line) - s;
		char c;
		char *s2 = s;

		if (fstr == NULL || *fstr == '\0') {
		    goto done;
		}
		while (left--) {
		    c = *fstr++;
		    *s2++ = c;
		    if (c == '\n' || c == '\0') {
			if (left) {
			    *s2 = '\0';
			}
			break;
		    }
		}
	    }
	    line[sizeof (line) - 1] = 0;	/* Make sure NUL-terminated */
	    if ((t = strchr(s, '\r')) != 0)
		strrepl(s, t - s, 1, "");	/* I hate hate hate PCs */
	    if ((t = strchr(s, '#')) != 0) { 	/* Strip out comments */
		*t = 0;
	    } else if ((t = strchr(s, '\n')) != 0) {
		*t = 0;			/* Remove newline */
	    }
	    t = s;			/* Handle backslash continuation */
	    s += strlen(s);
	    if (s > t && s[-1] == '\\') {
		s--;
		continue;
	    }
	    break;
	}

	switch ((rv = sh_process_command(u, line))) {
	case CMD_OK:
	    break;
	case CMD_EXIT:
	case CMD_INTR:
	    goto done_exit;
	default:
	    printk("Error: file %s: line %d (error code %d): script %s\n",
		   fn, lineNum, rv,
		   sh_set_rcerror ? "terminated" : "continuing");
	    rv = CMD_FAIL;		/* Don't return nfnd here */
	    if (sh_set_rcerror) {
		goto done;
	    }
	}
	(void)var_set_integer("?", rv, TRUE, FALSE);
    }

 done:

    if (lineNum == 1) {
	printk("Error: file %s: empty script\n", fn);
	rv = CMD_FAIL;
    }

 done_exit:
    sh_rcload_depth--;
    if (scope) {
	var_pop_scope((void *)scope);
    }

    if (fp) {
	sal_fclose((FILE *)fp);
    }
    sh_pop_ctrl_c();

    if (rv == CMD_EXIT) {
	/* An 'exit' command in script is normal completion of script. */
	rv = CMD_OK;
    }

    return(rv);
#endif /* NO_FILEIO */
}


#endif /* NO_SAL_APPL */

/*
 * Function:	_sh_parse_units
 * Purpose:	Parse a command prefix specifying which units to use
 * Parameters:	unit - current unit
 *		ustr - input command string with or without prefix
 *		umap - (OUT) bitmap of units to execute on
 *		ulen - (OUT) number of characters to start of command
 * Returns:	cmd_result_t
 */

STATIC int
_sh_parse_units(int unit, const char *ustr, int *umap_ptr, int *ulen_ptr)
{
    int		i, u, u2, umap, ulen;
    const char	*ustr_orig = ustr;

    /* See if it looks like a unit specification */

    for (i = 0; ustr[i] != ':'; i++) {
	if (ustr[i] != '-' &&
	    ustr[i] != ',' &&
	    ustr[i] != '*' && !isdigit((unsigned)ustr[i])) {
	    /* Not a unit specification; use default unit */
	    umap = (1 << unit);
	    ulen = 0;
	    goto done;
	}
    }

    /* Parse the unit specification */

    umap = 0;

    do {
	if (ustr[0] == '*') {
	    u = 0;
	    u2 = soc_ndev - 1;
	    ustr++;
	} else {
	    if (!isdigit((unsigned)ustr[0])) {
		printk("Error: Bad unit specification\n");
		return CMD_FAIL;
	    }
	    u = ustr[0] - '0';
	    if (u >= soc_ndev) {
		printk("Error: Unit %d out of range\n", u);
		return CMD_FAIL;
	    }
	    if (ustr[1] == '-' && isdigit((unsigned)ustr[2])) {
		/* Range specified */
		u2 = ustr[2] - '0';
		if (u2 >= soc_ndev) {
		    printk("Error: Unit %d out of range\n", u2);
		    return CMD_FAIL;
		}
		ustr += 3;
	    } else {
		u2 = u;
		ustr++;
	    }
	}
	if (u == u2 && !soc_attached(u)) {
	    /* Specific unit requested - requires unit to be attached */
	    printk("Error: Unit %d is not attached\n", u);
	    return CMD_FAIL;
	}
	for (i = u; i <= u2; i++) {
	    if (soc_attached(i)) {
		umap |= (1 << i);
	    }
	}
	if (*ustr != ',' && *ustr != ':') {
	    printk("Error: Bad unit specification\n");
	    return CMD_FAIL;
	}
    } while (*ustr++ == ',');

    ulen = ustr - ustr_orig;

 done:

    *umap_ptr = umap;
    *ulen_ptr = ulen;

    return CMD_OK;
}

cmd_result_t
sh_process_statement(int u, args_t *a)
/*
 * Function:	sh_process_statement
 * Purpose:	Evaluate a parsed command statement
 * Parameters:	u - unit number, may be -1 if none
 *		a - Single parsed statement
 * Returns:	cmd_result_t
 */
{
    /*
     * Variable:	h_args/h_count
     * Purpose:	Parse list to check for help. If a command is issued with
     *		one parameter and it is "help" or "?", the usage line is
     *		printed.
     */
    static const char * const h_args[] = {"HELP", "?"};
    static const int h_cnt = COUNTOF(h_args);
    int		rv, umap, ulen, ualt;
    char	*ac;
    cmd_t	*cmd;

    ac = ARG_CUR(a);
    if (ac == NULL) {
	return CMD_OK;		/* Ignore empty line */
    }

    /*
     * If command is prefixed with "#:", where # is a unit number, or a
     * unit range "#-#:", or a comma-separated list of unit numbers
     * and/or unit ranges, then recursively execute the command on the
     * specified unit(s).
     *
     * If command is prefixed with "*:", then recursively execute
     * the command on all attached units.
     */

    if ((rv = _sh_parse_units(u, ac, &umap, &ulen)) != CMD_OK) {
	return rv;
    }

    if (ulen > 0) {
	for (ualt = 0; ualt < soc_ndev && rv == CMD_OK; ualt++) {
	    if ((umap & (1 << ualt)) != 0) {
		args_t		*a_copy;

		/* Args structure is too big to replicate on local stack */

		if ((a_copy = sal_alloc(sizeof (args_t), "a_copy")) == NULL) {
		    printk("Error: out of memory\n");
		    return CMD_FAIL;
		}

		sh_swap_unit_vars(ualt);

                parse_args_copy(a_copy, a);

		a_copy->a_argv[0] += ulen;	/* Skip unit specifier */

		if (a_copy->a_argv[0][0] == 0) {
		    /*
		     * If the user put white space after unit specifier,
		     * the command would be in the following word.
		     */
		    ARG_NEXT(a_copy);
		}

		rv = sh_process_statement(ualt, a_copy);

		sal_free(a_copy);
	    }
	}

	sh_swap_unit_vars(u);

	return rv;
    }

    sh_swap_unit_vars(u);
    ARG_NEXT(a);

    /*
     * If it's just an integer, non-zero=success and zero=fail.
     * This is used for "if" conditions.
     */

    if (isint(ac) && ARG_CUR(a) == 0) {
	rv = parse_integer(ac) ? CMD_OK : CMD_FAIL;
	return rv;
    }

    /* First lookup in dynamic list -- allows redefinition of commands */
    cmd = (cmd_t*)parse_lookup(ac, dyn_cmd_list, sizeof(cmd_t), dyn_cmd_cnt); 
    
    if(cmd == NULL) {
        /* Next lookup in common list */
        cmd = (cmd_t *)parse_lookup(ac, bcm_cmd_common,
                                    sizeof(cmd_t), bcm_cmd_common_cnt);
    }

    if (cmd == NULL) { 	 /* Not in common or dynamic list. */
        cmd = (cmd_t *)parse_lookup(ac, cur_cmd_list,
                                    sizeof(cmd_t), cur_cmd_cnt);
        if (cmd == NULL) { 	 /* RC load if not found and enabled */

            if (sh_set_rcload) {
#ifndef NO_SAL_APPL
                rv = sh_rcload_file(u, a, ac, TRUE);
            } else {
#endif 
                rv = CMD_NFND;
            }

            if (rv == CMD_NFND) {
                printk("Unknown command: %s\n", ac);
            }

            return rv;
        }
    }

    if ((ARG_CNT(a) == 1) &&
	parse_lookup(ARG_CUR(a), h_args,
		     sizeof(h_args[0]), h_cnt)) {
	rv = CMD_USAGE;
    } else {
	ARG_CMD(a) = cmd->c_cmd;	/* Set command name */

	rv = cmd->c_f(u, a);
    }

    if (rv == CMD_USAGE) {		/* Print usage is requested */
	if ((cmd->c_usage) &&
        (soc_property_get(u, spn_HELP_CLI_ENABLE, 1))) {
	    printk("Usage (%s): %s", cmd->c_cmd, cmd->c_usage);
	} else if (soc_property_get(u, spn_HELP_CLI_ENABLE, 1)){
	    printk("Usage (%s): %s\n", cmd->c_cmd, cmd->c_help);
	}
    }

    if (rv == CMD_NOTIMPL) {
	printk("%s: Command not implemented\n", cmd->c_cmd);
	rv = CMD_FAIL;
    }

    if (rv == CMD_OK && ARG_CUR(a) != NULL) {
	/*
	 * Warn about unconsumed arguments.  If a command doesn't need
	 * all its arguments, it should use ARG_DISCARD to eat them.
	 */
	printk("%s: WARNING: excess arguments ignored "
	       "beginning with '%s'\n",
	       cmd->c_cmd, ARG_CUR(a));
    }

    return rv;
}

cmd_result_t
sh_process_command(int u, char *c)
/*
 * Function: 	sh_process_command
 * Purpose:	Parse a command, look it up, and dispatch it.
 * Parameters:	u - unit number, may be -1 if none
 *		c - pointer to input line.
 * Returns:	cmd_result_t
 */
{
    cmd_result_t 	rv = CMD_OK;
    args_t		*a;
    char		*c_next;

    if (u >= 0) {
	debugk(DK_RCLOAD, "BCM.%d> %s\n", u, c);
    } else {
	debugk(DK_RCLOAD, "BCM> %s\n", c);
    }

    c_next = c;

    /* Args structure is too big to allocate on local stack */

    if ((a = sal_alloc(sizeof (args_t), "args_t")) == NULL) {
	printk("sh_process_command: Out of memory\n");
	return CMD_FAIL;
    }

    while ((c = c_next) != NULL) {
	while (isspace((unsigned) *c)) {
	    c++;
	}

	if (diag_parse_args(c, &c_next, a)) {	/* Parses up to ; or EOL */
	    rv = CMD_FAIL;
	    break;
	}

	rv = sh_process_statement(u, a);

	(void)var_set_integer("?", rv, TRUE, FALSE);

	if (rv == CMD_USAGE || rv == CMD_INTR || rv == CMD_EXIT) {
	    break;
	}
    }

    sal_free(a);

    return(rv);
}

char sh_help_short_usage[] =
    "Parameters: None\n\t"
    "Display a complete list of commands in a concise format.\n";

cmd_result_t
sh_help_short(int u, args_t *a)
/*
 * Function: 	sh_help_short
 * Purpose:	Print list of all commands in command table.
 * Parameters:	u - unit # (ignored)
 *		a - args, if passed, turns into "??".
 * Returns:	CMD_OK
 */
{
#   define	CMD_PER_LINE	5
    int		i, words;

    if (0 != ARG_CNT(a)) {
	return(sh_help(u, a));
    }
    printk("help: \"??\" or \"help\" for summary\n");

    printk("Commands common to all modes:\n");
    words = 0;
    for (i = 0; i < bcm_cmd_common_cnt; i++) {
	if (bcm_cmd_common[i].c_help[0] != '.') {
	    printk("%-15s%s", bcm_cmd_common[i].c_cmd,
		   (words % CMD_PER_LINE) == (CMD_PER_LINE - 1) ? "\n" : "");
	    words++;
	}
    }
    if (words % CMD_PER_LINE) {
	printk("\n");
    }

    printk("Commands for current mode:\n");
    words = 0;
    for (i = 0; i < cur_cmd_cnt; i++) {
	if (cur_cmd_list[i].c_help[0] != '.') {
	    printk("%-15s%s", cur_cmd_list[i].c_cmd,
		   (words % CMD_PER_LINE) == (CMD_PER_LINE - 1) ? "\n" : "");
	    words++;
	}
    }
    if (words % CMD_PER_LINE) {
	printk("\n");
    }

    if(dyn_cmd_cnt > 0) {
        printk("Dynamic commands for all modes:\n");
        words = 0;
        for (i = 0; i < dyn_cmd_cnt; i++) {
            if (dyn_cmd_list[i].c_help[0] != '.') {
                printk("%-15s%s", dyn_cmd_list[i].c_cmd,
                       (words % CMD_PER_LINE) == (CMD_PER_LINE - 1) ? "\n" : "");
                words++;
            }
        }
        if (words % CMD_PER_LINE) {
            printk("\n");
        }
    }

    return(CMD_OK);
#   undef 	CMD_PER_LINE
}

static void
print_help_list(cmd_t * cmd_list, int command_count, char *title)
{
    cmd_t      *cmd;

    if (command_count > 0) {
        printk("\n%s:\n", title);
        for (cmd = cmd_list; cmd < &cmd_list[command_count]; cmd++) {
            printk("\n\nCOMMAND: %s\tDescription: %s\n\n%s",
                   cmd->c_cmd,
                   cmd->c_help[0] == '.' ? &cmd->c_help[1] : cmd->c_help, cmd->c_usage);
        }
    }
}

char sh_help_usage[] =
    "Parameters: [command] ... \n\t"
    "Display usage information for the listed commands.\n\t"
    "If no parameters given, a list of all commands is printed with\n\t"
    "a short description of the purpose of the command.\n";

/*ARGSUSED*/
cmd_result_t
sh_help(int u, args_t *a)
/*
 * Function: 	sh_help
 * Purpose:	Print usage messages for the listed commands
 * Parameters:	u - unit number (not used)
 * Returns:	CMD_XXX
 */
{
    cmd_t       *cmd;
    char	*c;
    cmd_result_t rv = CMD_OK;
    cmd_t       *clist; /* Current command list */
    int         ccnt;   /* Current count of commands */

    COMPILER_REFERENCE(u);

    if (!soc_property_get(u, spn_HELP_CLI_ENABLE, 1)) {
        return (CMD_OK);
    }
    /* Provide easy method to print out all commands */
    if (ARG_CNT(a) == 1 && !sal_strcasecmp(_ARG_CUR(a), "print-manual")) {
	ARG_NEXT(a);
        /* Print common commands */
	print_help_list(bcm_cmd_common, bcm_cmd_common_cnt,
		"Commands common to all modes");
	print_help_list(cur_cmd_list, cur_cmd_cnt,
		"Commands for current mode");
	print_help_list(dyn_cmd_list, dyn_cmd_cnt,
		"Dynamic commands for all modes");
	return(CMD_OK);
    }

    if (ARG_CNT(a) == 0) {		/* Nothing listed */
	int	i;
	printk("Help: Type help \"command\" for detailed command usage\n");
	printk("Help: Upper case letters signify minimal match\n");
	printk("\nCommands common to all modes:\n");
	for (i = 0; i < bcm_cmd_common_cnt; i++) {
	    if ((*bcm_cmd_common[i].c_help != '@') &&
		(*bcm_cmd_common[i].c_help != '.')) {
		printk("\t%-20s%s\n",
		       bcm_cmd_common[i].c_cmd, bcm_cmd_common[i].c_help);
	    }
	}
	printk("\nCommands for current mode:\n");
	for (i = 0; i < cur_cmd_cnt; i++) {
	    if ((*cur_cmd_list[i].c_help != '@') &&
		(*cur_cmd_list[i].c_help != '.')) {
		printk("\t%-20s%s\n",
		       cur_cmd_list[i].c_cmd, cur_cmd_list[i].c_help);
	    }
	}
        if(dyn_cmd_cnt > 0) {
            printk("\nDynamic commands for all modes:\n");
            for (i = 0; i < dyn_cmd_cnt; i++) {
                if ((*dyn_cmd_list[i].c_help != '@') &&
                    (*dyn_cmd_list[i].c_help != '.')) {
                    printk("\t%-20s%s\n",
                           dyn_cmd_list[i].c_cmd, dyn_cmd_list[i].c_help);
                }
            }
        }
	printk("\nNumber Formats:\n"
	      "\t[-]0x[0-9|A-F|a-f]+ -hex if number begins with \"0x\"\n"
	      "\t[-][0-9]+           -decimal integer\n"
	      "\t[-]0[0-7]+          -octal if number begins with \"0\"\n"
	      "\t[-]0b[0-1]+         -binary if number begins with \"0b\"\n\n"
	       );
	return(CMD_OK);
    }

    while ((c = ARG_GET(a)) != NULL) {  /* Look it up */
	/* Try current command list first */
        clist = cur_cmd_list;
        ccnt = cur_cmd_cnt;
        cmd = (cmd_t *) parse_lookup(c, clist, sizeof(cmd_t), ccnt);
        if (!cmd) {
	    /* Next try common command */
            clist = bcm_cmd_common;
            ccnt = bcm_cmd_common_cnt;
            cmd = (cmd_t *) parse_lookup(c, clist, sizeof(cmd_t), ccnt);
            if (!cmd) {
		/* Lastly try dynamic commands */
                clist = dyn_cmd_list;
                ccnt = dyn_cmd_cnt;
                cmd = (cmd_t *) parse_lookup(c, clist, sizeof(cmd_t), ccnt);
                if (!cmd) {
                    printk("Usage: Command not found: %s\n", c);
                    rv = CMD_FAIL;
                    continue;
                }
            }
        }

        if (*cmd->c_help == '@') {
	    cmd = (cmd_t *)parse_lookup(cmd->c_help + 1, clist,
					sizeof(cmd_t), ccnt);
	    if (!cmd) {
		printk("%s: Error: Aliased command not found\n", ARG_CMD(a));
		continue;
	    }
	    printk("Usage (%s): is an alias for \"%s\"\n", c, cmd->c_cmd);
	} else {
	    c = cmd->c_cmd;
	}
	if (cmd->c_usage) {
	    printk("Usage (%s): %s", cmd->c_cmd, cmd->c_usage);
	} else {
	    printk("Usage: Not available for command: %s\n", c);
	}
    }
    return(rv);
}

STATIC cmd_result_t
exit_clean(void)
/*
 * Function:    exit_clean
 * Purpose:     Free up BCM and SOC resources for all attached units
 * Parameters:  None
 * Returns:     CMD_OK/CMD_USAGE/CMD_FAIL
 */
{
#ifdef BCM_WARM_BOOT_SUPPORT
    int i = 0;

    /*
     * Controlled SOC/BCM tear down: for all units
     * Call clean up routines to free up resources
     * and do not exit BCM shell;
     */
    for (i = 0; i < soc_ndev; i++) {
        if (_bcm_shutdown(i) < 0) {
            return(CMD_FAIL);
        }
#if defined(BCM_SBX_SUPPORT)
        if (SOC_IS_SBX(i)) {
            if (soc_sbx_shutdown(i) < 0) {
                return(CMD_FAIL);
            }
        } else
#endif /* BCM_SBX_SUPPORT */
#if defined(BCM_ESW_SUPPORT)
        if (SOC_IS_ESW(i)) {
            if (soc_shutdown(i) < 0) {
                return(CMD_FAIL);
            }
        } else
#endif /* BCM_ESW_SUPPORT */
        {
            printk("Error: warmboot not supported by unit %d\n", i);
            return (CMD_FAIL);
        }

        printk("bcm/soc shut down on unit %d\n", i);
   }

   return(CMD_EXIT);
#else
   return(CMD_USAGE);
#endif /* BCM_WARM_BOOT_SUPPORT */
}

char sh_exit_usage[] =
#ifdef BCM_WARM_BOOT_SUPPORT
    "Parameters: none\n\t"
    "If clean is specified, free up BCM and SOC resources for attached\n\t"
    "units. Device is untouched (for Reload).\n\t"
#else
    "Parameters: [clean]\n\t"
#endif /* BCM_WARM_BOOT_SUPPORT */
    "Exit from the current shell, if this is the \"socdiag\" shell\n\t"
    "The system is reset\n";

cmd_result_t
sh_exit(int u, args_t *a)
/*
 * Function: 	sh_exit
 * Purpose:	Exit from the current level of the shell.
 * Parameters:	None
 * Returns:	CMD_EXIT/CMD_USAGE
 */
{
    int  arg_count;
    char *arg_str;

    COMPILER_REFERENCE(u);

    arg_count = ARG_CNT(a);

    if (arg_count > 1) {
        return(CMD_USAGE);
    }

    if (arg_count == 1) {
        arg_str = ARG_GET(a);
        if (NULL == arg_str) {
            return (CMD_EXIT);
        }

#ifdef BCM_WARM_BOOT_SUPPORT
#if (defined(LINUX) && !defined(__KERNEL__)) || defined(UNIX)
        if (appl_scache_file_close(u) < 0) {
            printk("Unit %d scache file not closed\n", u);
        }
#endif /* (defined(LINUX) && !defined(__KERNEL__)) || defined(UNIX) */
#endif /* BCM_WARM_BOOT_SUPPORT */

        if(!sal_strcasecmp(arg_str, "clean")) {
            return exit_clean();
        } else {
            return(CMD_USAGE);
        }
    } else {
        return(CMD_EXIT);
    }
}

cmd_result_t
sh_process(int u, const char *pfx, int eof_exit)
/*
 * Function: 	sh_process
 * Purpose:	Process an input stream
 * Parameters:	eof_exit - if true, allow control-d to exit the
 *			command line processing.
 * Returns:	CMD_OK - Processing completed on input stream.
 *		CMD_EXIT - Processing completed on input stream, EXIT
 *		requested
 */
{
    static char cmd[ARGS_BUFFER];
    static char old_cmd[ARGS_BUFFER] = "";
    char * volatile old_cmd_lw = old_cmd;	/* Pointer to last word */

    int	modifier;
    char *tpfx;
    char *t;				/* Tmp buf pointer */
    char *s;				/* cmd buf pointer */
    int	s_len;				/* cmd buf length */
    volatile cmd_result_t rv = CMD_OK;
    volatile int unit = u;
    jmp_buf ctrl_c;
    void * volatile scope = NULL;
    volatile int batchmode = 0;
                                  /* Max batch recursion level */
    void * volatile batchscope[4] = {NULL, NULL, NULL, NULL};
    volatile int bsi = 0;
#ifdef INCLUDE_EDITLINE
    extern void add_history(char *p);
#endif

    COMPILER_REFERENCE(ctrl_c);
#ifndef NO_CTRL_C
    if (!setjmp(ctrl_c)) {
	sh_push_ctrl_c(&ctrl_c);
	scope = var_push_scope();
	(void)var_set_integer("?", 0, TRUE, FALSE);
    } else {
	(void)var_set_integer("?", CMD_INTR, TRUE, FALSE);
    }
#else
    scope = var_push_scope();
    (void)var_set_integer("?", 0, TRUE, FALSE);
#endif
    do {
	/*
	 * If we are overriding the current unit, ignore checks.
	 */
	if (!override_unit) {
	    int		first;

	    /*
	     * If there is no current unit, but some unit is attached,
	     * make the first attached unit the current unit.  If no
	     * unit is attached, change unit to -1.
	     */

	    for (first = 0; first < soc_ndev; first++) {
		if (soc_attached(first)) {
		    break;
		}
	    }

	    if (first == soc_ndev) {
		unit = -1;		/* None attached */
		sh_swap_unit_vars(unit);
	    } else if (unit < 0 || !soc_attached(unit)) {
		unit = first;
		sh_swap_unit_vars(unit);
	    }
	}

        /*
         * Batch mode essentially turns off the command prompt and
         * disables local command line substitution.
         *
         * This is a way of emulating the rcload command when file
         * I/O is unavailable in the BCM shell environment.  In this
         * case the commands are entered through the console (e.g.
         * using copy/paste) or through a proxy with file I/O.
         */

	if ((s = var_get("proxy_cmd_")) != NULL) {
            if (!strcmp(s, "enter_batchmode")) {
                batchmode = 1;
            } else if (!strcmp(s, "exit_batchmode")) {
                batchmode = 0;
            } else if (!strcmp(s, "push_scope")) {
                batchscope[bsi] = var_push_scope();
                if (bsi < COUNTOF(batchscope)-1) {
                    bsi++;
                }
            } else if (!strcmp(s, "pop_scope")) {
                if (bsi) {
                    bsi--;
                }
                var_pop_scope(batchscope[bsi]);
            }
            var_unset("proxy_cmd_", TRUE, FALSE, FALSE);
        }

	/*
	 * If in debug mode, .debug is appended to the prompt.
	 * If the current unit number is non-negative, .UNIT is
	 * added to the prompt.
	 * The cmd buffer is used for the prompt.
	 */

	if (batchmode) {
	    cmd[0] = 0;		/* No prompt */
	} else if (! printk_cons_is_enabled()) {
	    cmd[0] = 0;		/* No prompt */
	}
    else if ((s = var_get("prompt")) != NULL)
    {
	    sal_strncpy(cmd, s, ARGS_BUFFER - 1);	/* User-defined prompt */
        cmd[ARGS_BUFFER - 1] = 0;

	    while ((s = (char *) strcaseindex(cmd, "\\n")) != 0)
        {
            strrepl(s, 0, 2, "\n");		/* Allow \n for newline */
        }
	}
    else
    {
	    sal_strncpy(cmd, pfx, ARGS_BUFFER - 1);	/* Default prompt */
        cmd[ARGS_BUFFER - 1] = 0;

        if (command_mode_get() == BCMX_CMD_MODE) {
#ifdef	INCLUDE_BCMX
                extern int bcmx_unit_count;
                sal_sprintf(cmd + strlen(cmd), "X(%d units)", bcmx_unit_count);
#else
                strcat(cmd, "X");
#endif	/* INCLUDE_BCMX */
            } else if (unit >= 0) {
		int enable = 0;

              if (SOC_IS_ROBO(unit)){
#ifdef BCM_ROBO_SUPPORT
                  soc_robo_mem_debug_get(unit, &enable);
#endif
              } else if(SOC_IS_ESW(unit)){
#ifdef BCM_ESW_SUPPORT
                  soc_mem_debug_get(unit, &enable);
#endif
              }

              if (enable) {
                  strcat(cmd, ".debug");
              }
              sal_sprintf(cmd + strlen(cmd), ".%d", unit);
	    }
	    strcat(cmd, "> ");
	}

	tpfx = cmd;

	s = cmd;
	s_len = sizeof(cmd);

	while (TRUE) {
	    s_len = sizeof(cmd) - (s - cmd);
	    if (NULL == sal_readline(tpfx, s, s_len, NULL)) {
		if (eof_exit) {
		    printk("EOF\n");
		    sh_pop_ctrl_c();
		    if (scope) {
			var_pop_scope(scope);
		    }
		    return(CMD_OK);
		} else {
		    printk("Type \"EXIT\" to exit shell\n");
		    continue;
		}
	    }
	    if(s[0] == '~') {
		printk("EOF\n");
		return CMD_OK;
	    }
	    t = s;
	    s += strlen(s);
	    if (s > t && s[-1] == '\\') {
		*s++ = ' ';		/* Replace with white space */
		tpfx = "? ";
		continue;
	    }
	    break;
	}

	/*
	 * Process each occurrence of !! and !$, unless it would
	 * overflow the buffer.
	 */

	if (!batchmode) {

            modifier = FALSE;

            while ((s = (char *) strcaseindex(cmd, "!!")) != 0 &&
                   strlen(cmd) - 2 + strlen(old_cmd) + 1 < ARGS_BUFFER) {
                strrepl(s, 0, 2, old_cmd);
                modifier = TRUE;
            }

            while ((s = (char *) strcaseindex(cmd, "!$")) != 0 &&
                   strlen(cmd) - 2 + strlen((char *)old_cmd_lw) + 1 <
                   ARGS_BUFFER) {
                strrepl(s, 0, 2, (char *)old_cmd_lw);
                modifier = TRUE;
            }

            if (modifier) {		/* Echo command if modified */
                printk("%s\n", cmd);	/* Logs if enabled */
            }
        }

	/*
	 * Save command in !! buffer, but only if it doesn't consist of
	 * all white space.  Also save in editline history, if active.
	 */

	if (strspn(cmd, " \t") < strlen(cmd)) {
	    sal_strcpy(old_cmd, cmd);
	    for (old_cmd_lw = old_cmd; *old_cmd_lw != 0; old_cmd_lw++)
		;
	    while (old_cmd_lw > old_cmd &&
		   isspace((unsigned) old_cmd_lw[-1])) {
		old_cmd_lw--;
	    }
	    while (old_cmd_lw > old_cmd &&
		   !isspace((unsigned) old_cmd_lw[-1])) {
		old_cmd_lw--;
	    }
#ifdef INCLUDE_EDITLINE
	    add_history(cmd);
#endif /* INCLUDE_EDITLINE */
	}

	/*
	 * If the command is "#:" where # is a unit number, change the
	 * default unit number.
	 *
	 * If the command is "#::" where # is a unit number, change the
	 * default unit number EVEN IF UNIT IS NOT VALID AND ATTACHED.
	 *
	 * Processing of commands preceded by a "#:" prefix is done in
	 * sh_process_command() so that it works from rc scripts.  Rc
	 * scripts are not allowed to change the default unit number.
	 */

	rv = CMD_OK;

        if (cur_mode == BCMX_CMD_MODE) {
	    rv = sh_process_command(unit, cmd);
        } else if (isdigit((unsigned) cmd[0]) &&
	    cmd[1] == ':' && cmd[2] == ':'&& cmd[3] == 0) {
	    unit = cmd[0] - '0';
	    override_unit = TRUE;
	    printk("Override default SOC device set to %d\n", unit);
	} else if (isdigit((unsigned) cmd[0]) &&
		   cmd[1] == ':' && cmd[2] == 0) {
	    u = cmd[0] - '0';
	    if (!soc_attached(u)) {
		printk("Error: Unit %d is not attached\n", u);
		rv = CMD_FAIL;
	    } else {
		/* If we were in override mode, turn it off now. */
		override_unit = FALSE;
		unit = u;
		printk("Default SOC device set to %d\n", unit);
		sh_swap_unit_vars(unit);
	    }
	} else {
	    rv = sh_process_command(unit, cmd);
	}

	(void)var_set_integer("?", rv, TRUE, FALSE);
	tpfx = (char *)pfx;		/* backslash may have changed */
    } while (CMD_EXIT != rv);

    sh_pop_ctrl_c();
    if (scope) {
	var_pop_scope(scope);
    }

    return(rv);
}

#ifndef NO_SAL_APPL

char sh_rcload_usage[] =
    "Parameters: <file> [parameters] \n\t"
    "Load commands from a file until the file is complete or an error\n\t"
    "occurs. If optional parameters are listed after <file> they are\n\t"
    "pushed as local variables for the file processing. For example:\n\t"
    "\t rcload fred a bc will result in the variables:\n\t"
    "\t\t$1 = a\n\t"
    "\t\t$2 = bc\n\t"
    "\tduring processing of the file.\n\t"
    "For platforms where FTP is used, the user name, password and host\n\t"
    "may be specified as: \"user%password@host:\" as a prefix to the\n\t"
    "file name\n";

cmd_result_t
sh_rcload(int u, args_t *a)
/*
 * Function: 	sh_rcload
 * Purpose:	Load commands from a file.
 * Parameters:	u - unit number
 *		a - arguments.
 * Returns:	CMD_NFND 0 file not found or result of last command
 *		executed.
 */
{
    cmd_result_t rv = CMD_OK;
    char *c = NULL;			/* Keep gcc happy */

    if (!ARG_CNT(a)) {
	return(CMD_USAGE);
    }

    if (NULL != (c = ARG_GET(a))) {
	rv = sh_rcload_file(u, a, c, FALSE);
    }
    if (CMD_NFND == rv) {
	printk("%s: Error: file not found: %s\n", ARG_CMD(a), c);
    }
    return(rv);
}

#endif /* NO_SAL_APPL */

char sh_set_usage[] =
    "Parameters: [<option>=<value>\n"
#ifndef COMPILER_STRING_CONST_LIMIT
    "\tSet or display various configuration information, if no arguments\n\t"
    "are given, the current settings are displayed. Otherwise, all of\n\t"
    "the arguments are set. If \"=\" is the sole argument, the user is\n\t"
    "prompted for each settable item. Current allowable options are:\n\n\t"
    "IFError=[on|off]- if set to \"on\" errors on commands processed\n\t"
    "                  terminate the IF command.\n\t"
    "RCLoad=[on|off] - enable or disable looking for RC load file if\n\t"
    "                  command is not found.\n\t"
    "RCError=[on|off]- if set to \"on\", rcload scripts will abort when\n\t"
    "                  a command fails, if off, processing continues\n\t"
    "                  even if an error occurs.\n\t"
    "RCTest=[on|off]-  if set to \"on\", \"testrun\" will load preferred\n\t"
    "                  init scripts before certain tests (for example,\n\t"
    "                  the MAC and PHY loopback tests need rc.soc).\n\t"
    "LoopError=[on|off]- if set to \"on\", loop commands will abort\n\t"
    "                  when a command fails, if off, processing\n\t"
    "                  continues even if an error occurs.\n\t"
    "MoreLines=<#>   - Specify number of lines printed before holding\n\t"
    "                  on a more command\n\t"
    "ReportTime=<#>  - Specify number of seconds between reports printed\n\t"
    "                  by long-running tests\n"
#endif
    ;


cmd_result_t
sh_set(int u, args_t *a)
/*
 * Function: 	sh_set
 * Purpose:	Set various configuration arguments.
 * Parameters:	u - unit
 *		a - arguments.
 * Returns:	CMD_OK or CMD_FAIL.
 * Notes:	This routine will not free strings properly!
 */
{
    parse_table_t	pt;

    parse_table_init(u, &pt);
    parse_table_add(&pt, "IFError", PQ_DFL|PQ_BOOL, 0,
		    &sh_set_iferror, NULL);
    parse_table_add(&pt, "RCLoad", PQ_DFL|PQ_BOOL, 0,
		    &sh_set_rcload, NULL);
    parse_table_add(&pt, "RCError", PQ_DFL|PQ_BOOL, 0,
		    &sh_set_rcerror, NULL);
    parse_table_add(&pt, "RCTest", PQ_DFL|PQ_BOOL, 0,
		    &sh_set_rctest, NULL);
    parse_table_add(&pt, "LoopError", PQ_DFL|PQ_BOOL, 0,
		    &sh_set_lperror, NULL);
    parse_table_add(&pt, "MoreLines", PQ_DFL|PQ_INT, 0,
		    &sh_set_more_lines, NULL);
    parse_table_add(&pt, "ReportStatus", PQ_DFL|PQ_BOOL, 0,
		    &sh_set_report_status, NULL);
    parse_table_add(&pt, "ReportTime", PQ_DFL|PQ_INT, 0,
		    &sh_set_report_time, NULL);

    if (!ARG_CNT(a)) {			/* Display settings */
	printk("Current settings:\n");
	parse_eq_format(&pt);
	parse_arg_eq_done(&pt);
	return(CMD_OK);
    }

    if (0 > parse_arg_eq(a, &pt)) {
	printk("%s: Error: Unknown option: %s\n", ARG_CMD(a), ARG_CUR(a));
	parse_arg_eq_done(&pt);
	return(CMD_FAIL);
    }

    parse_arg_eq_done(&pt);
    return(CMD_OK);
}

char sh_debug_usage[] =
    "Parameters: [[+/-]<option> ...]\n\t"
    "Set or clear debug options, +option adds to list of output,\n\t"
    "-option removes from list, no +/- toggles current state.\n\t"
    "If no options given, a current list is printed out\n";

cmd_result_t
sh_debug(int u, args_t *a)
/*
 * Function: 	sh_debug
 * Purpose:	Set shell debug parameters.
 * Parameters:	u - unit #
 *		a - arguments.
 * Returns:	CMD_OK or CMD_FAIL if no match found.
 */
{
    uint32 		dk = debugk_enable(0);
    static parse_pm_t	dk_map[32 + 3];
    static int		dk_map_initted;
    char 		*c;
    int i;

    COMPILER_REFERENCE(u);

    if (! dk_map_initted) {
	int		p;

	for (p = 0, i = 0; i < 32; i++) {
	    if ((c = soc_cm_debug_names[i]) != NULL) {
		dk_map[p].pm_s = c;
		dk_map[p++].pm_value = (uint32) 1 << i;
	    }
	}

	dk_map[p].pm_s = "@ALL";
	dk_map[p++].pm_value = ~0;

	dk_map[p].pm_s = "@*";
	dk_map[p++].pm_value = ~0;

	dk_map[p].pm_s = NULL;
	dk_map[p].pm_value = 0;

	assert(p < COUNTOF(dk_map));

	dk_map_initted = 1;
    }

    if (!ARG_CNT(a)) {			/* No args - print settings */
	/*
	 * No parameters.  Display current status as a list of enabled
	 * debug output followed by list of disabled debug output.
	 */
	printk("Debugging is enabled for the following subsystems:\n\n");
	parse_mask_format(50, dk_map, dk);
	printk("\nDebugging is disabled for the following subsystems:\n\n");
	parse_mask_format(50, dk_map, ~dk);
	printk("\n");
	return(CMD_OK);
    }

    /* Set new mode */

    while ((c = ARG_GET(a)) != NULL) {
	if (parse_mask(c, dk_map, &dk)) {
	    printk("debug: unknown option: %s\n", c);
	    return(CMD_FAIL);
	}
    }

    debugk_select(dk);

    return(CMD_OK);
}

typedef int (*module_debug_config_fn)(uint32 level);

#if defined(BROADCOM_DEBUG)
STATIC module_debug_config_fn module_debug_func_tbl[BCM_DBG_COUNT] = {
                           NULL, /* BCM_DBG_NORMAL    */
                           NULL, /* BCM_DBG_ERR       */
                           NULL, /* BCM_DBG_WARN      */
                           NULL, /* BCM_DBG_VERBOSE   */
                           NULL, /* BCM_DBG_VVERBOSE  */
                           NULL, /* BCM_DBG_PORT      */
                           NULL, /* BCM_DBG_VLAN      */
                           NULL, /* BCM_DBG_L2        */
                           NULL, /* BCM_DBG_L3        */
                           NULL, /* BCM_DBG_TX        */
                           NULL, /* BCM_DBG_RX        */
                           NULL, /* BCM_DBG_PKT_DATA  */
                           NULL, /* BCM_DBG_LINK      */
                           NULL, /* BCM_DBG_COUNTER   */
                           NULL, /* BCM_DBG_INTR      */
                           NULL, /* BCM_DBG_DMA       */
                           NULL, /* BCM_DBG_STK       */
                           NULL, /* BCM_DBG_ATTACH    */
                           NULL, /* BCM_DBG_MCAST     */
                           NULL, /* BCM_DBG_IPMC      */
                           NULL, /* BCM_DBG_FP        */
                           NULL, /* BCM_DBG_FILTER    */
                           NULL, /* BCM_DBG_API       */
                           NULL, /* BCM_DBG_POLICER   */
                           NULL, /* BCM_DBG_COSQ      */
                           NULL, /* BCM_DBG_OAM       */
                           NULL, /* BCM_DBG_MPLS      */
#ifdef INCLUDE_MACSEC
                           bcm_common_macsec_config_print, /* BCM_DBG_MACSEC    */
#else
                           NULL, /* BCM_DBG_MACSEC    */
#endif /* INCLUDE_MACSEC */
};
#endif  /* BROADCOM_DEBUG */

char sh_debug_mod_usage[] =
    "Parameters: <module> [[-]<option> ...]\n\t"
    "Set or clear debug options for given modules, \n\t"
    "'option' adds to list of output,'-option' removes from list.\n\t"
    "Option 'all' is supported.\n\t"
    "If no options given, the current settings are printed out\n\t"
    "Modules include SOC BCM BCMX DIAG or STK\n\t"
    "Module must be specified and only one module is supported per cmd\n";

cmd_result_t
sh_debug_mod(int unit, args_t *args)
/*
 * Function: 	sh_debug
 * Purpose:	Set shell debug parameters.
 * Parameters:	u - unit #
 *		a - arguments.
 * Returns:	CMD_OK or CMD_FAIL if no match found.
 */
{
#if defined(BROADCOM_DEBUG)
    int i;
    char *ch;
    uint32 *level = NULL, old_level;
    char **names = NULL;
    int name_count = 0;
    int neg;
    int opts_found = FALSE;
    char *mod_name;
    int count = 0;

    COMPILER_REFERENCE(unit);

    if (!ARG_CNT(args)) {
        printk("Module must be specified\n");
        return CMD_USAGE;
    }

    if (NULL != (mod_name = ARG_GET(args))) {
        if (!sal_strcasecmp(mod_name, "soc")) {
            level = &soc_debug_level;
            names = soc_debug_names;
            name_count = SOC_DBG_COUNT;
        } else if (!sal_strcasecmp(mod_name, "bcm")) {
            level = &bcm_debug_level;
            names = bcm_debug_names;
            name_count = BCM_DBG_COUNT;
#ifdef	INCLUDE_BCMX
        } else if (!sal_strcasecmp(mod_name, "bcmx")) {
            level = &bcmx_debug_level;
            names = bcmx_debug_names;
            name_count = BCMX_DBG_COUNT;
#endif
        } else if (!sal_strcasecmp(mod_name, "diag")) {
            level = &diag_debug_level;
            names = diag_debug_names;
            name_count = DIAG_DBG_COUNT;
#ifdef	INCLUDE_LIB_CPUDB
        } else if (!sal_strcasecmp(mod_name, "stk")) {
            level = &tks_debug_level;
            names = tks_debug_names;
            name_count = TKS_DBG_COUNT;
#endif
#ifdef	INCLUDE_BOARD
        } else if (!sal_strcasecmp(mod_name, "board")) {
            level = &board_debug_level;
            names = board_debug_names;
            name_count = BOARD_DBG_COUNT;
#endif
        } else {
            printk("Module not recognized: %s\n", mod_name);
            return CMD_USAGE;
        }
    } else {
        printk("Module unspecified\n");
        return CMD_USAGE;
    }

    old_level = *level;

    /* Now analyze the options */
    while ((ch = ARG_GET(args)) != NULL) {
        neg = FALSE;
        if (ch[0] == '-') {
            neg = TRUE;
            ch++;
        } else if (ch[0] == '+') { /* Ignore + */
            ch++;
        }
        if (!sal_strcasecmp(ch, "all")) {
            *level = neg ? 0 : 0xffffffff;
        } else {
            for (i = 0; i < name_count; i++) {
                if (parse_cmp(names[i], ch, '\0')) {
                    if (neg) {
                        *level &= ~(1 << i);
                    } else {
                        *level |= (1 << i);
                    }
                    break;
                }
            }
            if (i >= name_count) {
                printk("Option %s not recognized for module %s\n",
                       ch, mod_name);
                if (opts_found) {
                    printk("Warning:  Verbose options changed for module %s\n",
                           mod_name);
                }
                return CMD_USAGE;
            }
        }
        opts_found = TRUE;
    }

    if (!opts_found) { /* Report current settings */
        if (*level == 0) {
            printk("No options enabled for %s\n", mod_name);
        } else {
            printk("Enabled options for %s:\n    ", mod_name);
            for (i = 0; i < name_count; i++) {
                if (*level & (1 << i)) {
                    printk("%s ", names[i]);
                    if (!(++count % 4)) {
                        printk("\n    ");
                    }
                }
            }
            printk("\n");
        }
        count = 0;
        printk("Disabled options for %s:\n    ", mod_name);
        for (i = 0; i < name_count; i++) {
            if (!(*level & (1 << i))) {
                printk("%s ", names[i]);
                if (!(++count % 4)) {
                    printk("\n    ");
                }
            }
        }
        printk("\n");
    }

    /*
     * Call module's debug handler.
     */
    for (i = 0; i < name_count; i++) {
        if (module_debug_func_tbl[i] == NULL) {
            continue;
        }
        if ((old_level & (1 << i)) ^ (*level & (1 << i))) {
            module_debug_func_tbl[i](*level);
        }
    }

#else
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(args);
    printk("Debug modules not compiled into this build.\n");
#endif /* BROADCOM_DEBUG */

    return(CMD_OK);
}

#ifndef NO_SAL_APPL

char sh_log_usage[] =
    "Parameters: [file=<filename>] [append=<yes|no>]\n\t"
    "[quiet=<yes|no>] [on|off]\n\t"
    "Set a logfile and/or turn on or off logging. When a file is set\n\t"
    "for logging an implicit \"log on\" is assumed.\n";


cmd_result_t
sh_log(int u, args_t *a)
/*
 * Function: 	sh_log
 * Purpose:	Enable/disable logging to a file.
 * Parameters:	u - unit #
 *		a - argument list.
 * Returns:	CMD_OK or CMD_FAIL if unable to open log file.
 */
{
    cmd_result_t 	rv = CMD_FAIL;
    char		*file, *cur_file;
    int			append;
    int			quiet;
    char		*c;
    parse_table_t 	pt;

    COMPILER_REFERENCE(u);

    if ((cur_file = printk_file_name()) == NULL) {
	cur_file = LOG_DEFAULT_FILE;
    }

    if (ARG_CNT(a) == 0) {
	printk("Logging to file %s: %s\n",
	       cur_file,
	       printk_file_is_enabled() ? "enabled" : "disabled");
	return CMD_OK;
    }

    parse_table_init(u, &pt);
    parse_table_add(&pt, "File", PQ_STRING, cur_file, &file, 0);
    parse_table_add(&pt, "Append", PQ_BOOL, (void *)1, &append, 0);
    parse_table_add(&pt, "Quiet", PQ_BOOL, (void *)0, &quiet, 0);

    if (0 > parse_arg_eq(a, &pt)) {
	printk("%s: Invalid option: %s\n", ARG_CMD(a), ARG_CUR(a));
	parse_arg_eq_done(&pt);
	return CMD_USAGE;
    }

    if ((c = ARG_GET(a)) == NULL) {
	c = "on";		/* Default is to turn logging on */
    }

    if (!sal_strcasecmp("on", c)) {
	int		new_file = 0;

	if (file == NULL || *file == 0) {
	    if (cur_file != NULL) {
		file = cur_file;
	    } else {
		printk("%s: Missing log file name\n", ARG_CMD(a));
		rv = CMD_FAIL;
		goto done;
	    }
	}

	if (cur_file == NULL || strcmp(cur_file, file) != 0) {
	    /* Switch log file name if changed */

	    new_file = 1;

	    if (printk_file_open(file, append) < 0) {
		printk("%s: Error: Could not start logging\n", ARG_CMD(a));
		rv = CMD_FAIL;
		goto done;
	    }
	}

	if (printk_file_enable(TRUE) < 0) {
	    printk("%s: Error: Could not start logging\n", ARG_CMD(a));
	    rv = CMD_FAIL;
	    goto done;
	}

	if (!quiet) {
	    printk("File logging %s to %s\n",
		   new_file ? "started" : "continued",
		   file);
	}

	rv = CMD_OK;
	goto done;
    }

    if (!sal_strcasecmp("off", c)) {
	if (cur_file == NULL || ! printk_file_is_enabled()) {
	    printk("File logging is not active.\n");
	    rv = CMD_FAIL;
	    goto done;
	}

	if (printk_file_enable(FALSE) < 0) {
	    printk("%s: Error: Could not stop logging to %s\n",
		   ARG_CMD(a), cur_file);
	    rv = CMD_FAIL;
	    goto done;
	}

	if (!quiet) {
	    printk("File logging to %s stopped.\n", cur_file);
	}

	rv = CMD_OK;
	goto done;
    }

    rv = CMD_USAGE;

 done:
    parse_arg_eq_done(&pt);
    return(rv);
}

char sh_console_usage[] =
    "Parameters: on|off\n\t"
    "Turn on or off output to the console. This does not affect current\n\t"
    "logging\n";

cmd_result_t
sh_console(int u, args_t *a)
/*
 * Function: 	sh_console
 * Purpose:	Control various console options.
 * Parameters:	u - unit number (ignored)
 *		a - arguments
 * Returns:	CMD_USAGE/CMD_OK
 */
{
    char *c;

    COMPILER_REFERENCE(u);

    if (1 != ARG_CNT(a)) {
	return(CMD_USAGE);
    }
    c = ARG_GET(a);

    if (!sal_strcasecmp("on", c)) {
	printk_cons_enable(TRUE);
    } else if (!sal_strcasecmp("off", c)) {
	printk_cons_enable(FALSE);
    } else {
	return(CMD_USAGE);
    }
    return(CMD_OK);
}

char sh_reboot_usage[] =
    "Parameters: none\n\t"
    "Reboots the processor\n";

cmd_result_t
sh_reboot(int u, args_t *a)
/*
 * Function: 	sh_reboot
 * Purpose:	Reboot the system.
 * Parameters:	u - unit number
 *		a - arguments [expects none].
 * Returns:	CMD_FAIL or does not return.
 */
{
    COMPILER_REFERENCE(u);

    if (ARG_CNT(a)) {
	return(CMD_USAGE);
    }
    sal_reboot();
    return(CMD_OK);
}

#endif /* NO_SAL_APPL */


#ifndef BCM_PLATFORM_STRING
#ifdef KEYSTONE
#define BCM_PLATFORM_STRING "KEYSTONE"
#else
#define BCM_PLATFORM_STRING "unknown"
#endif
#endif

void
sh_print_version(int verbose)
/*
 * Function: 	sh_print_version
 * Purpose:	Prints current version of firmware.
 * Parameters:	verbose - includes more info if TRUE
 * Returns:	Nothing
 */
{
    printk("Broadcom Command Monitor: "
	   "Copyright (c) 1998-2010 Broadcom Corporation\n");
    printk("Release: %s built %s (%s)\n",
	   _build_release, _build_datestamp, _build_date);
    printk("From %s@%s:%s\n",
	   _build_user, _build_host, _build_tree);
    printk("Platform: %s\n", BCM_PLATFORM_STRING); 
    printk("OS: %s\n", sal_os_name() ? sal_os_name() : "unknown"); 

    if (verbose) {
#if (defined(BCM_ROBO_SUPPORT) || defined(BCM_ESW_SUPPORT) || defined(BCM_SBX_SUPPORT))
	int		i, j;
#endif
#ifdef BCM_ESW_SUPPORT
        char *phy_list[64];
        int phys_in_list, phy_list_base, rv;
#endif

#if defined(BOOT_LINE_ADRS) && !defined(NO_SAL_APPL)
	printk("Boot string: %s\n", (char *)BOOT_LINE_ADRS);
#endif

#ifdef BCM_ROBO_SUPPORT      
        for (i = 0; i < SOC_ROBO_NUM_SUPPORTED_CHIPS; i += 4) {
            printk("%s", i == 0 ? "ROBO Chips:" : "      ");
            for (j = i; j < i + 4 && j < SOC_ROBO_NUM_SUPPORTED_CHIPS; j++) {
                if (SOC_ROBO_DRIVER_ACTIVE(j)) {
		    printk(" %s%s",
			   SOC_CHIP_NAME(soc_robo_base_driver_table[j]->type),
			   j < SOC_ROBO_NUM_SUPPORTED_CHIPS - 1 ? "," : "");
                }
            }
            printk("\n");
        }
#endif

#ifdef BCM_EA_SUPPORT
             printk("%s", "EA Chips:");
             printk("%s", "TK3715_A0");
             printk("\n");
#endif

#ifdef BCM_ESW_SUPPORT
	for (i = 0; i < SOC_NUM_SUPPORTED_CHIPS; i += 4) {
	    printk("%s", i == 0 ? "Chips:" : "      ");
	    for (j = i; j < i + 4 && j < SOC_NUM_SUPPORTED_CHIPS; j++) {
		if (SOC_DRIVER_ACTIVE(j)) {
		    printk(" %s%s",
			   SOC_CHIP_NAME(soc_base_driver_table[j]->type),
			   j < SOC_NUM_SUPPORTED_CHIPS - 1 ? "," : "");
		}
	    }
	    printk("\n");
	}
#endif

#ifdef BCM_SBX_SUPPORT
        for (i = 0; i < SOC_SBX_NUM_SUPPORTED_CHIPS; i += 4) {
            printk("%s", i == 0 ? "SBX Chips:" : "      ");
            for (j = i; j < i + 4 && j < SOC_SBX_NUM_SUPPORTED_CHIPS; j++) {
                if (SOC_SBX_DRIVER_ACTIVE(j)) {
                    printk(" %s%s",
                           SOC_CHIP_NAME(soc_sbx_driver_table[j]->type),
                           j < SOC_SBX_NUM_SUPPORTED_CHIPS - 1 ? "," : "");
                }
            }
            printk("\n");


        }
#ifdef BCM_FE2000_SUPPORT
        for (i = 0; i < SOC_SBX_MAX_UCODE_TYPE; i++) {
            if (soc_sbx_ucode_versions[i]) {
                printk("%s microcode: %s\n",
                       SOC_CHIP_NAME(SOC_DRIVER(i)->type),
                       soc_sbx_ucode_versions[i]);
            }
        }
#endif
#endif        

#ifdef BCM_ESW_SUPPORT
        phy_list_base = 0;
        printk("PHYs: ");
        
        do {

            phys_in_list = phy_list_base;
            rv = soc_phy_list_get(phy_list,
                                  COUNTOF(phy_list), &phys_in_list);
           
            /* Save the base for the next iteration if 
               there are more PHYs in the list. */
            phy_list_base += phys_in_list;
           
            if (!((rv == SOC_E_FULL) || (rv == SOC_E_NONE))) {
                break;
            }
            /* Print the PHY list - 2 step 
               4 PHYs in one row */
            for (i = 0; i < phys_in_list; i += 4) {
                for (j = i; j < i + 4 && j < phys_in_list; j++) {
                    printk(" %s%s",
                           phy_list[j],
                           j < phys_in_list - 1 ? "," : "");
                }
                printk("\n   ");
            }
        } while (rv == SOC_E_FULL); 
   
        printk("\n");

#endif 

    }
}

char sh_version_usage[] =
    "Parameters: none\n\t"
    "Prints current firmware version and build status\n";

cmd_result_t
sh_version(int u, args_t *a)
/*
 * Function: 	sh_version
 * Purpose:	Print version of the current socdiag shell/driver
 * Parameters:	u - Unit number (ignored)
 *		a - pointer to arguments.
 * Returns:	CMD_USAGE/CMD_OK
 */
{
    COMPILER_REFERENCE(u);

    if (ARG_CNT(a)) {
	return(CMD_USAGE);
    }
    sh_print_version(TRUE);
    return(CMD_OK);
}

char sh_probe_usage[] =
    "Parameters: <None>\n\t"
    "Probe for available SOC units\n";

cmd_result_t
sh_probe(int u, args_t *a)
/*
 * Function: 	sh_probe
 * Purpose:	Probe for StrataSwitch Devices.
 * Parameters:	u - unit (ignored)
 *		a - arguments (none)
 * Returns:	CMD_USAGE/CMD_FAIL/CMD_OK
 */
{
    COMPILER_REFERENCE(u);

    if (ARG_CNT(a)) {
	return(CMD_USAGE);
    }
    if (sysconf_probe() < 0) {
	printk("%s: Probe failed\n", ARG_CMD(a));
	return(CMD_FAIL);
    } else {
	printk("%s: found %d unit%s\n", ARG_CMD(a), soc_ndev,
               soc_ndev==1 ? "" : "s");
	var_set_integer("units", soc_ndev, FALSE, FALSE);
    }
    return(CMD_OK);
}

char sh_attach_usage[] =
    "Parameters: [Unit #] ...\n\t"
    "If no parameters are given, a current list of attached devices\n\t"
    "is printed. Otherwise, each parameter is considered a unit # and\n\t"
    "an attempt is made to attached it.\n";

cmd_result_t
sh_attach(int unit, args_t *a)
/*
 * Function: 	sh_attach
 * Purpose:	Attach a soc device, and make it the default device.
 * Parameters:	unit - currently attached unit (-1 if none)
 *		a - args, expects a list of unit #'s to attach
 * Returns:	CMD_FAIL/CMD_OK
 */
{
    char	*c;
    int		u;
    int         dev;

    if (soc_ndev < 0) {
	printk("%s: Probe command not yet performed\n", ARG_CMD(a));
	return CMD_FAIL;
    }

    if (!ARG_CNT(a)) {			/* No parameters */

	if (soc_ndev == 0) {
	    printk("%s: No units probed\n", ARG_CMD(a));
        }

	for (dev = 0; dev < soc_ndev; dev++) {
	    printk("%s: Unit %d (%s): %sattached%s\n",
		   ARG_CMD(a),
		   dev, soc_dev_name(dev),
		   soc_attached(dev) ? "" : "not ",
		   (unit == dev) ? " (current unit)" : "");
        }

	return(CMD_OK);
    }

    while ((c = ARG_GET(a)) != NULL) {
        if (*c == '*') {  /* Attach (report) all devices */
            for (dev = 0; dev < soc_ndev; dev++) {
                if (!soc_attached(dev)) {
                    if (sysconf_attach(dev) < 0) {
                        printk("%s: Error: Could not attach unit: %d\n",
                               ARG_CMD(a), dev);
                        return(CMD_FAIL);
                    }
                } else {
                    printk("Unit %d is attached\n", dev);
                }
            }
            return CMD_OK;
        }
	if (!isint(c)) {
	    printk("%s: Error: Invalid unit #: %s\n", ARG_CMD(a), c);
	    return(CMD_FAIL);
	}
	u = parse_integer(c);
	if (u < 0 || u >= soc_ndev) {
	    printk("%s: Error: Unit number out of range (%d - %d)\n",
		   ARG_CMD(a), 0, soc_ndev - 1);
	    return(CMD_FAIL);
	} else if (soc_attached(u)) {
	    printk("%s: Error: Unit already attached: %d\n", ARG_CMD(a), u);
	    return(CMD_FAIL);
	} else if (sysconf_attach(u) < 0) {
	    printk("%s: Error: Could not attach unit: %d\n", ARG_CMD(a), u);
	    return(CMD_FAIL);
	} else if (unit < 0) {		/* No current unit */
	    sh_swap_unit_vars(u);	/* First unit attached */
	}
    }

#if defined(BCM_EA_SUPPORT)
    if(SOC_E_NONE != soc_ea_do_init(soc_ndev)){
        return (CMD_FAIL);
    }
#endif
    return(CMD_OK);
}

static	cmd_result_t
sh_do_detach(int u, args_t *a)
/*
 * Function: 	sh_do_detach
 * Purpose:	Detach the specified SOC unit.
 * Parameters:	u - unit number to detach.
 *		a - pointer to arguments (used only for "cmd" error msg.
 * Returns:	CMD_OK - done, CMD_FAIL - soc device not valid/attached.
 */
{
    if (!sh_check_attached(ARG_CMD(a), u)) {
	return(CMD_FAIL);
    } else if (sysconf_detach(u) < 0) {
	printk("%s: Error: detaching unit %d\n", ARG_CMD(a), u);
	return(CMD_FAIL);
    } else {
	return(CMD_OK);
    }
}

char sh_detach_usage[] =
    "Parameters: [unit #] ...\n\t"
    "If no parameters are given, the default unit is detached.\n\t"
    "If one or more units are specified, each is detached.\n";

cmd_result_t
sh_detach(int u, args_t *a)
/*
 * Function: 	sh_detach
 * Purpose:	Detach one or more SOC devices.
 * Parameters:	u - unit number to detach (if no parameters given)
 *		a - pointer to args (list of units to detach).
 * Returns:	CMD_OK - done
 *		CMD_FAIL - invalid unit # given.
 */
{
    cmd_result_t rv = CMD_OK;
    char *c;
    int unit;

    if (!ARG_CNT(a)) {			/* Use default unit # */
        var_unset(SOC_CHIP_STRING(u), FALSE, TRUE, FALSE);
        var_unset(soc_dev_name(u), FALSE, TRUE, FALSE);
        return(sh_do_detach(u, a));
    }
    while ((CMD_OK == rv) && (c = ARG_GET(a))) {
        if (isint(c)) {
            unit = parse_integer(c);
            if (u == unit) { 
                var_unset(SOC_CHIP_STRING(u), FALSE, TRUE, FALSE);
                var_unset(soc_dev_name(u), FALSE, TRUE, FALSE);
            }
            rv = sh_do_detach(unit, a);
        } else {
            printk("%s: Error: Invalid unit: %s\n", ARG_CMD(a), c);
            rv = CMD_FAIL;
        }
    }
    return(rv);
}

char sh_echo_usage[] =
    "Parameters: [-n] <Any text>\n\t"
    "Echo whatever follows on command line, followed by a newline.\n\t"
    "The newline may be suppressed by using the -n option.\n";

/*ARGSUSED*/
cmd_result_t
sh_echo(int u, args_t *a)
/*
 * Function: 	sh_echo
 * Purpose:	Echo command input
 * Parameters:	u - unit number, ignored
 *		a - pointer to args.
 * Returns:	CMD_OK.
 */
{
    char	*c;
    int		suppress_nl = FALSE;


    COMPILER_REFERENCE(u);

    c = ARG_GET(a);
    if (c != NULL && strcmp(c, "-n") == 0) {
	suppress_nl = TRUE;
	c = ARG_GET(a);
    }

    while (c != NULL) {
	printk("%s", c);
	c = ARG_GET(a);
	if (c != NULL) {
	    printk(" ");
	}
    }
    if (!suppress_nl) {
	printk("\n");
    }
    return(CMD_OK);
}

char sh_noecho_usage[] =
    "Parameters: <Any text>\n\t"
"Ignore whatever follows on command line\n";

/*ARGSUSED*/
cmd_result_t
sh_noecho(int u, args_t *a)
/*
 * Function: 	sh_noecho
 * Purpose:	ignore command input
 * Parameters:	u - unit number, ignored
 *		a - pointer to args.
 * Returns:	CMD_OK.
 */
{
    char	*c;
    COMPILER_REFERENCE(u);

    while ((c = ARG_GET(a)) != NULL) {
    }
    return(CMD_OK);
}

#ifndef NO_SAL_APPL

char sh_pause_usage[] =
    "Parameters: [-x] <Any text>\n\t"
    "Echo whatever follows on the command line, then prompt to\n\t"
    "hit SPACE to continue or Q to quit.  On quit, the pause command\n\t"
    "fails (causing the executing script to terminate).  The -x option\n\t"
    "requires the user to press x instead of SPACE to continue.\n";

cmd_result_t
sh_pause(int u, args_t *a)
/*
 * Function: 	sh_pause
 * Purpose:	Suspend input and wait for confirmation to continue.
 * Parameters:	u - unit # (ignored)
 *		a - pointer to arguments.
 * Returns:	CMD_OK - enter hit
 *		CMD_FAIL - some other input such as "Q".
 */
{
    cmd_result_t	rv = CMD_OK;
    char		*c;
    int			ch, cont_ch = ' ';
    char		prompt[80];

    c = ARG_CUR(a);

    if (c != NULL && c[0] == '-' && c[1] != 0 && c[2] == 0) {
	cont_ch = c[1];
	ARG_NEXT(a);
    }

    if (islower(cont_ch)) {
	cont_ch = toupper(cont_ch);
    }

    sh_echo(u, a);			/* Print out args */

    sal_strcpy(prompt, SAL_VT100_SO "Hit ");

    if (cont_ch == ' ') {
	strcat(prompt, "SPACE");
    } else {
	c = prompt + strlen(prompt);
	c[0] = cont_ch;
	c[1] = 0;
    }

    strcat(prompt, " to continue, Q to quit" SAL_VT100_SE);

    for (;;) {
	if ((ch = sal_readchar(prompt)) < 0) {
	    rv = CMD_FAIL;
	    break;
	}

	printk("\r" SAL_VT100_CE);

	if (islower(ch)) {
	    ch = toupper(ch);
	}

	if (ch == 'Q' || ch == '\033') {	/* ESC */
	    rv = CMD_FAIL;
	    break;
	}

	if (ch == cont_ch) {
	    break;
	}

	printk("\007");
    }

    return(rv);
}

#endif /* NO_SAL_APPL */

char sh_sleep_usage[] =
    "Parameters: [quiet] [seconds [usec]]\n\t"
    "If quiet is specified, no output is generated, otherwise, the\n\t"
    "message \"sleeping for # seconds\" is printed. Seconds specifies\n\t"
    "the number of seconds to sleep.  Usec specifies an additional\n\t"
    "number of microseconds to sleep (0-999999).\n";

/*ARGSUSED*/
cmd_result_t
sh_sleep(int u, args_t *a)
/*
 * Function: 	sh_sleep
 * Purpose:	Put CLI task to sleep for some amount of time.
 * Parameters:	u - unit # (ignored).
 * Returns:	CMD_OK
 */
{
    int sec = 1, usec = 0, quiet = 0;
    char *c = ARG_GET(a);

    COMPILER_REFERENCE(u);

    if (c && !sal_strcasecmp(c, "quiet")) {
	quiet = 1;
	c = ARG_GET(a);
    }

    if (c) {
	if (! isint(c) || (sec = parse_integer(c)) < 0) {
	    return CMD_USAGE;
	}
	c = ARG_GET(a);
    }

    if (c) {
	if (! isint(c) || (usec = parse_integer(c)) < 0) {
	    return CMD_USAGE;
	}
	c = ARG_GET(a);
    }

    if (c) {
	return CMD_USAGE;
    }

    if (! quiet) {
	if (usec) {
	    printk("Sleeping for %d.%06d seconds\n",
		   sec + usec / 1000000, usec % 1000000);
	} else {
	    printk("Sleeping for %d second%s\n", sec, sec > 1 ? "s" : "");
	}
    }

    sal_sleep(sec);
    sal_usleep(usec);

    return CMD_OK;
}

char sh_delay_usage[] =
    "Parameters: usec\n\t"
    "Cause the CLI task to busy-wait for a specified number of\n\t"
    "microseconds.  The actual amount of delay is only approximate.\n";

/*ARGSUSED*/
cmd_result_t
sh_delay(int u, args_t *a)
/*
 * Function: 	sh_delay
 * Purpose:	Make CLI task busy-wait for some amount of time.
 * Parameters:	u - unit # (ignored).
 * Returns:	CMD_OK
 */
{
    uint32		usec;
    char		*c;

    COMPILER_REFERENCE(u);

    if ((c = ARG_GET(a)) == NULL || !isint(c)) {
	return CMD_USAGE;
    }

    usec = parse_integer(c);

    sal_udelay(usec);

    return CMD_OK;
}

#ifndef NO_SAL_APPL

char sh_shell_usage[] =
    "Parameters: None\n\t"
    "Invoke a platform dependent shell. This shell exits when the shell\n\t"
    "exits\n";

/*ARGSUSED*/
cmd_result_t
sh_shell(int u, args_t *a)
/*
 * Function: 	sh_shell
 * Purpose:	Call "sal_shell" to do whatever the shell is supposed to.
 * Parameters:	u - unit number (ignored)
 *		a - pointer to args
 * Returns:	CMD_OK
 */
{
    COMPILER_REFERENCE(u);

    if (ARG_CNT(a)) {
	return(CMD_USAGE);
    }
    sal_shell();
    return(CMD_OK);
}
#endif


#ifndef NO_SAL_APPL

char sh_ls_usage[] =
    "Parameters: <file> ...\n\t"
    "Invoke a platform \"ls\" command, useful to see RC files that may\n\t"
    "be invoked if a command does not match\n";

cmd_result_t
sh_ls(int u, args_t *a)
/*
 * Function: 	sh_ls
 * Purpose:	Perform an "ls" command (platform dependent).
 * Parameters:	u - unit number (ignored)
 *		a - arguments, a list of directories to do an ls in.
 * Returns:	CMD_OK
 */
{
    char	*c, *flags = NULL;

    COMPILER_REFERENCE(u);

    c = ARG_GET(a);

    if (c != NULL && c[0] == '-') {
	flags = c;
	c = ARG_GET(a);
    }

    if (c == NULL)
	c = ".";

    while (c != NULL) {
	sal_ls(c, flags);
	c = ARG_GET(a);
    }

    return(CMD_OK);
}

char sh_pwd_usage[] =
    "Parameters: <None>\n\t"
    "Print the current platform dependent working directory\n";

/*ARGSUSED*/
cmd_result_t
sh_pwd(int u, args_t *a)
/*
 * Function: 	sh_pwd
 * Purpose:	Print the current working directory (Platform dependent).
 * Parameters:	u - unit 3 (ignored)
 *		a - pointer to args (No arguments expected)
 * Returns:	CMD_OK/CMD_FAIL
 */
{
    char	pwd[128];

    COMPILER_REFERENCE(u);

    if (ARG_CNT(a)) {
	return(CMD_USAGE);
    }

    if (!sal_getcwd(pwd, sizeof(pwd))) {
	printk("%s: Error: Unable to determine current directory\n",
	       ARG_CMD(a));
	return(CMD_FAIL);
    } else {
	printk("Working Directory: %s\n", pwd);
	return(CMD_OK);
    }
}

#endif /* NO_SAL_APPL */

char sh_history_usage[] =
    "Parameters: [count]\n\t"
    "Displays the command history.\n\t"
    "If count is given, output is limited to at most count lines.\n";

/*ARGSUSED*/
cmd_result_t
sh_history(int u, args_t *a)
/*
 * Function: 	sh_history
 * Purpose:	Print history in input log.
 * Parameters:	u - unit (ignored)
 *		a - args.
 * Returns:	CMD_OK.
 */
{
#ifdef INCLUDE_EDITLINE
    extern void list_history(int count);
    char *count_str = ARG_GET(a);
    COMPILER_REFERENCE(u);
    list_history(count_str ? parse_integer(count_str) : 999);
    return(CMD_OK);
#else
    COMPILER_REFERENCE(u);
    printk("History not available\n");
    return CMD_FAIL;
#endif
}

char sh_loop_usage[] =
    "Parameters: <loop-count> \"command\" ...\n\t"
    "Loop over a series of commands <loop-count> times, if <loop-count>\n\t"
    "is * then is loops forever. Each argument is a complete command,\n\t"
    "so if it contains more that one word it must be placed in quotes.\n\t"
    "For example:\n\n\t\t"
    "loop 10 \"echo hello world\"\n\n\t"
    "will execute the following command 10 times:\n\n\t"
    "\t\"echo hello world\"\n\n\t"
    "If the series of commands fails, the loop will stop only if the\n\t"
    "\"LoopError\" option is true (see the \"set\" command).\n";

cmd_result_t
sh_loop(int u, args_t *a)
/*
 * Function: 	sh_loop
 * Purpose:	Perform a command inside a loop.
 * Parameters:	u - unit number
 *		a - args, first parameter "#" or "*" for forever.
 *			Remaining args command line as normal.
 * Returns:	Result of command.
 */
{
    volatile int l;
    volatile int l_cur;			/* Current loop counter */
    char	*c;
    volatile cmd_result_t rv = CMD_OK;
    int		s_arg;			/* Saved ARG value */
    volatile int forever = FALSE;
    jmp_buf	ctrl_c;

    if (ARG_CNT(a) < 2) {
	return(CMD_USAGE);
    }

    c = ARG_GET(a);
    if (!strcmp(c, "*")) {
	forever = TRUE;
        l = 0;
    } else if (!isint(c) || (0 > (l = parse_integer(c)))) {
	printk("%s: Error: Invalid loop count: %s\n",
	       ARG_CMD(a), c);
	return(CMD_FAIL);
    }

    /* Push control-c to print nice message when control-c hit */

    l_cur = 1;				/* Assign before setjump */
    COMPILER_REFERENCE(ctrl_c);
#ifndef NO_CTRL_C
    if (setjmp(ctrl_c)) {
	printk("%s: Warning: Looping aborted on the %dth loop\n",
	       ARG_CMD(a), l_cur);
	sh_pop_ctrl_c();
	return(CMD_INTR);
    } else {
	sh_push_ctrl_c(&ctrl_c);
    }
#endif

    /* Now perform the command */

    s_arg = a->a_arg;			/* Save to restore after CMD */

    for ( ;
	 ((CMD_OK == rv) || !sh_set_lperror) && (forever || (l_cur <= l));
	 l_cur++){
	a->a_arg = s_arg;
	while ((c = ARG_GET(a)) && ((CMD_OK == rv) || !sh_set_lperror)) {
	    rv = sh_process_command(u, c);
	}
    }
    sh_pop_ctrl_c();
    return(rv);
}

char sh_for_usage[] =
    "Parameters: <var>=<start>,<stop>[,<step>[,<fmt>]] 'command' ...\n\t"
    "Iterate a series of commands, each time setting <var> to the\n\t"
    "loop value.  Each argument is a complete command, so if it contains\n\t"
    "more than one word, it must be placed in quotes.  For example:\n\n\t"
    "\tfor port=0,23 'echo port=$port'\n"
#ifndef COMPILER_STRING_CONST_LIMIT
    "\n\tNote the use of single quotes "
    "to avoid expanding the $port variable\n\t"
    "before executing the loop.  <fmt> defaults to %d (decimal), but can\n\t"
    "be any other standard printf(1)-style format string.\n\n\t"
    "If the series of commands fails, the loop will stop only if the\n\t"
    "\"LoopError\" option is true (see the \"set\" command).\n"
#endif
    ;

cmd_result_t
sh_for(int u, args_t *a)
/*
 * Function: 	sh_for
 * Purpose:	Perform a command inside a loop.
 * Parameters:	u - unit number
 *		a - args
 * Returns:	Result of command.
 */
{
    char	*c, *c_var, *c_start, *c_stop, *c_step;
    char	*volatile c_fmt;
    volatile int l_start, l_stop, l_step, l_cur;
    volatile cmd_result_t rv = CMD_OK;
    int		s_arg;			/* Saved ARG value */
    jmp_buf	ctrl_c;

    if (ARG_CNT(a) < 2) {
	return(CMD_USAGE);
    }

    c = ARG_GET(a);

    c_var = strtok(c, "=");
    c_start = strtok(NULL, ",");
    c_stop = strtok(NULL, ",");
    c_step = strtok(NULL, ",");
    c_fmt = strtok(NULL, "\n");

    if (c_var == NULL || c_start == NULL || c_stop == NULL) {
	printk("%s: Error: Invalid loop format\n", ARG_CMD(a));
	return(CMD_FAIL);
    }

    l_start = parse_integer(c_start);
    l_stop = parse_integer(c_stop);
    l_step = c_step ? parse_integer(c_step) : 1;
    if (c_fmt == NULL)
	c_fmt = "%d";

    /* Push control-c to print nice message when control-c hit */
    COMPILER_REFERENCE(ctrl_c);
#ifndef NO_CTRL_C
    if (setjmp(ctrl_c)) {
	printk("%s: Warning: Looping aborted on the %dth loop\n",
	       ARG_CMD(a), l_cur);
	rv = CMD_INTR;
	goto done;
    } else {
	sh_push_ctrl_c(&ctrl_c);
    }
#endif

    /* Now perform the command */

    s_arg = a->a_arg;			/* Save to restore after CMD */

    for (l_cur = l_start;
	 (CMD_OK == rv || !sh_set_lperror) &&
	     ((l_step > 0 && l_cur <= l_stop) ||
	      (l_step < 0 && l_cur >= l_stop));
	 l_cur += l_step) {
	char value[80];
	sal_sprintf(value, c_fmt, l_cur, "", "", "");
	var_set(c_var, value, 1, 0);
	a->a_arg = s_arg;
	while ((c = ARG_GET(a)) && ((CMD_OK == rv) || !sh_set_lperror)) {
	    rv = sh_process_command(u, c);
	}
    }
#ifndef NO_CTRL_C
 done:
#endif
    var_unset(c_var, 1, 0, 0);
    sh_pop_ctrl_c();
    return(rv);
}

static cmd_result_t
sh_do_init(int u, int reset)
/*
 * Function: 	sh_do_init
 * Purpose:	Perform init function on a specific unit.
 * Parameters:	u - unit to initialize.
 *		reset - if TRUE, reset device, if FALSE, do not reset.
 * Returns:	CMD_OK/CMD_FAIL
 */
{
    /* Prevent hardware access during chip reset */
    system_shutdown(u, 0);

    if (SOC_IS_ROBO(u)) {
#ifdef BCM_ROBO_SUPPORT
        if (reset) {
	    if (soc_robo_reset_init(u) < 0) {
	        return(CMD_FAIL);
	    }
        } else {
	    if (soc_robo_init(u) < 0) {
	        return(CMD_FAIL);
	    }
        }
#endif
    }
#ifdef BCM_SBX_SUPPORT
      else if (SOC_IS_SBX(u)) {
        if (soc_sbx_init(u) < 0) {
            return(CMD_FAIL);
        }
    }
#endif
#ifdef BCM_EA_SUPPORT
      else if (SOC_IS_EA(u)){
    	 if (reset){
    		 if (soc_ea_reset_init(u) < 0){
    			 return (CMD_FAIL);
    		 }
    	 }else {
         	if (soc_init(u) < 0){
         		return (CMD_FAIL);
         	}
         }
      }
#endif
      else { 
#ifdef BCM_ESW_SUPPORT
        if (reset) {
  	    if (soc_reset_init(u) < 0) {
	        return(CMD_FAIL);
	    }
        } else {
	    if (soc_init(u) < 0) {
	        return(CMD_FAIL);
	    }
        }
#endif
    }


    return(CMD_OK);
}

#ifdef BROADCOM_DEBUG
char sh_break_usage[] =
"Set an breakpoint in this function for `at-will' gdb reentry\n"
;
cmd_result_t
sh_break(int unit, args_t *a)
{
    printk("At-will breakpoint\n");
    return CMD_OK;
}
#endif

char sh_case_usage[] =
    "Parameters: <what> <value> <command> [<value> <command>]...\n\t"
    "Execute a command based on matching a key value.\n";

cmd_result_t
sh_case(int u, args_t *a)
/*
 * Function: 	sh_case
 * Purpose:	Implement simple case statement.
 * Parameters:	u - unit #
 *		a - args.
 * Returns:	CMD_OK/CMD_FAIL/CMD_USAGE, or result of exectued
 *		command.
 */
{
    char	*c, *key;

    if (!(key = ARG_GET(a))) {
	return(CMD_USAGE);
    }
    while ((c = ARG_GET(a)) != NULL) {
	char *cmd = ARG_GET(a);
	if (!cmd) {
	    return(CMD_USAGE);
	}
	if (!strcmp(c, key) || !strcmp("*", c)) {
	    ARG_DISCARD(a);
	    return(sh_process_command(u, cmd));
	}
    }
    return(CMD_OK);
}

#if defined(BCM_XGS3_SWITCH_SUPPORT) && defined(BCM_WARM_BOOT_SUPPORT)
static void
appl_warmboot_storage_requirements(int unit)
{
    int rv, stable_size, total = 0;
    uint8 *scache_ptr;
    soc_scache_handle_t handle;
    uint32 module, storage_per_module[BCM_MODULE__COUNT];

    /* XGS3 module names matched with module.h
     * 'dmux' and 'auth' modules are user for
     * 'common' and 'virtual' modules respectively */
    char module_name[BCM_MODULE__COUNT] [10] = 
          {"port","L2","vlan","trunk","cosq",\
           "mcast", "linkscan", "stat", "filter", "diffserv",\
           "mirror", "L3", "stack", "ipmc", "stg",\
           "tx", "common", "virtual", "rx", "field",\
           "time", "subport", "mpls", "fabric", "mim",\
           "policer", "oam", "failover", "vswitch", "wlan",\
           "qos", "multicast", "trill", "ipfix", "niv", "ces"};

    /* Check if extended warmboot support is configured */
    rv = soc_stable_size_get(unit, &stable_size);
    if (SOC_FAILURE(unit)) {
        soc_cm_print("Unable to determine scache size!\n");
        return;
    }

    if (stable_size == 0) {
        soc_cm_print("External storage not configured!\n");
        return;
    }

    soc_cm_print("\nWarmboot storage requirements:\n");
    soc_cm_print("-------------------------------\n");
    for (module = 0; module < BCM_MODULE__COUNT; module++) {
        SOC_SCACHE_HANDLE_SET(handle, unit, module, 0);
        rv = soc_scache_ptr_get(unit, handle, &scache_ptr,
                                &storage_per_module[module]);  
        if (SOC_SUCCESS(rv)) {
            soc_cm_print("%s module: %d bytes\n",
                         module_name[module],
                         storage_per_module[module]);
            total += storage_per_module[module];
        } else {
            storage_per_module[module] = -1;
        }
    }
    soc_cm_print("------------------------------------\n");
    soc_cm_print("Total storage required: %d bytes\n", total);
    
    return;
}
#endif /* BCM_XGS3_SWITCH_SUPPORT && BCM_WARM_BOOT_SUPPORT */

char sh_warmboot_usage[] =
#if defined(BCM_XGS3_SWITCH_SUPPORT)
    "Parameters: [on | off | show | storage]\n"
    "To signal warm or cold boot or to display current boot status\n"
     "or display the warmboot external storage requirements\n"
    ;
#else 
    "Parameters: [on | off | show]\n"
    "To signal warm or cold boot or to display current boot status\n"
    ;
#endif /* BCM_XGS3_SWITCH_SUPPORT */

/*
 * Function:    sh_warmboot
 * Purpose:     Assert WARMBOOT on to set SOC (reload) state for a unit.
 * Parameters:  u - unit number to operate on
 *              a - args
 * Returns:     CMD_OK/CMD_USAGE.
 */
cmd_result_t
sh_warmboot(int u, args_t *a)
{
    char *c;
    COMPILER_REFERENCE(u);

    if (0 == ARG_CNT(a)) {
        return(CMD_USAGE);
    } else {
        int unit = 0;

        c = ARG_GET(a);
        if (!sal_strcasecmp(c, "on")) {
#ifdef BCM_WARM_BOOT_SUPPORT
            for (unit = 0; unit < soc_ndev; unit++) {
                 SOC_WARM_BOOT_START(unit);
            }
#else /* BCM_WARM_BOOT_SUPPORT */
            soc_cm_print("Warm Boot support not compiled!"
                         "  Enable by defining BCM_WARM_BOOT_SUPPORT.\n");
            return CMD_FAIL;
#endif /* BCM_WARM_BOOT_SUPPORT */
        } else if (!sal_strcasecmp(c, "off")) {
            for (unit = 0; unit < soc_ndev; unit++) {
                 SOC_WARM_BOOT_DONE(unit);
            }
        }
#if defined(BCM_XGS3_SWITCH_SUPPORT) && defined(BCM_WARM_BOOT_SUPPORT)
        else if (!sal_strcasecmp(c, "storage")) {
            appl_warmboot_storage_requirements(u);
        }
#endif /* BCM_XGS3_SWITCH_SUPPORT && BCM_WARM_BOOT_SUPPORT */
        else if (!sal_strcasecmp(c, "show")) {
            for (unit = 0; unit < soc_ndev; unit++) {
                 if (!soc_attached(unit)) {
                     continue;
                 }

                 if (SOC_WARM_BOOT(unit)) {
                     soc_cm_print("Unit %d: Warm Boot\n", unit);
                 } else {
                     soc_cm_print("Unit %d: Cold Boot\n", unit);
                 }
            }
        } else {
            return(CMD_USAGE);
        }
    }
    return(CMD_OK);
}

#ifdef BCM_EASY_RELOAD_SUPPORT

char cmd_xxreload_usage[] =
"Parameters\n"
"\ton   -- Put the chip in reload mode\n"
"\toff  -- Put the chip in the normal mode\n"
"\tshow -- Show the mode\n";

static cmd_result_t
xxreload_set(int unit, int reload)
{
    if (!SOC_UNIT_VALID(unit)) {
        return (CMD_FAIL);
    }
    SOC_RELOAD_MODE_SET(unit, reload);

    return (CMD_OK);
}

static cmd_result_t
xxreload_show(int unit)
{
    int reload;

    if (!SOC_UNIT_VALID(unit)) {
        return (CMD_FAIL);
    }

    reload = SOC_IS_RELOADING(unit);
    printk("Unit %d is in %s mode.\n",
           unit,
           reload ? "reload" : "normal");

    return CMD_OK;
}

cmd_result_t
cmd_xxreload(int unit, args_t *a)
{
    char *c;

    if (!ARG_CNT(a)) {
        return (xxreload_show(unit));
    }

    c = ARG_GET(a);

    if (!sal_strcasecmp(c, "on")) {
        return (xxreload_set(unit, TRUE));
    } else if (!sal_strcasecmp(c, "off")) {
        return (xxreload_set(unit, FALSE));
    } else if (!sal_strcasecmp(c, "show")) {
        return (xxreload_show(unit));
    } else {
        return (CMD_USAGE);
    }

    return (CMD_OK);
}

#endif /* BCM_EASY_RELOAD_SUPPORT */

char sh_init_usage[] =
    "Parameters: [soc|noreset|mmu|bcm|misc|all]\n"
#ifndef COMPILER_STRING_CONST_LIMIT
    "\tPerforms various initializations (default is soc):\n\t"
    "  soc - resets the device, configures minimal register\n\t"
    "        settings and enables interrupts\n\t"
    "  noreset - like soc, but does not reset the device\n\t"
    "  mmu - writes SDRAM and MMU parameters (does not size GBP)\n\t"
    "  bcm - initializes BCM driver internals/registers/tables\n\t"
    "  misc - performs chip-specific fundamental initialization\n\t"
#ifdef BCM_EASYRIDER_SUPPORT
    "  tcam - perform tcam initialization\n\t"
#endif
    "  all - equivalent to performing soc, mmu, misc, and bcm\n\n"
    "Additional selective initialization (components of 'init bcm'):\n\t"
    "port, l2, vlan, trunk, cosq, mcast, linkscan, stat, filter, diffserv\n\t"
    "mirror, packet, l3, stack, ipmc, stg, mbcm, stp, tx, dmux, auth.\n"
#endif
    ;

cmd_result_t
sh_init(int u, args_t *a)
/*
 * Function: 	sh_init
 * Purpose:	Initialize the SOC chip and S/W for a unit.
 * Parameters:	u - unit number to operate on
 *		a - args (none expected)
 * Returns:	CMD_OK/CMD_FAIL.
 */
{
    cmd_result_t rv = CMD_OK;
    int		bcm_rv;
    char	*c;
    int 	usage = 0;

    if (!sh_check_attached(ARG_CMD(a), u)) {
	return(CMD_FAIL);
    }

    c = ARG_GET(a);

    if (!c || !sal_strcasecmp(c, "soc")) {
	if ((rv = sh_do_init(u, TRUE)) < 0) {
	    printk("%s: Unable to initialize device: %d\n",
		   ARG_CMD(a), u);
	}
    } else if (!sal_strcasecmp(c, "bcm")) {
	if ((bcm_rv = bcm_init(u)) < 0) {
	    printk("%s: Unable to initialize BCM driver on unit %d: %s\n",
		   ARG_CMD(a), u, bcm_errmsg(bcm_rv));
	    rv = CMD_FAIL;
	}
    } else if (!sal_strcasecmp(c, "misc")) {
	if ((rv = soc_misc_init(u)) < 0) {
	    printk("%s: Unable to initialize misc for device %d: %s\n",
		   ARG_CMD(a), u, soc_errmsg(rv));
	}
#if defined(BCM_EASYRIDER_SUPPORT) || defined(BCM_TRIUMPH_SUPPORT)
    } else if (!sal_strcasecmp(c, "tcam")) {
        if ((rv = soc_tcam_init(u)) < 0) {
	    printk("%s: Unable to initialize tcam for device %d: %s\n",
		   ARG_CMD(a), u, soc_errmsg(rv));
	}
#endif
#if defined(BCM_XGS_SUPPORT) || defined(BCM_SBX_SUPPORT)
    } else if (!sal_strcasecmp(c, "noreset")) {
	if ((rv = sh_do_init(u, FALSE)) < 0) {
	    printk("%s: Unable to initialize device %d\n",
		   ARG_CMD(a), u);
	}
    } else if (!sal_strcasecmp(c, "mmu")) {
#ifdef BCM_SBX_SUPPORT
        if (SOC_IS_SBX(u)) {
	    /* printk("This option is not supported on SBX devices\n"); */
	    rv = CMD_OK;
        return rv;
	}
#endif
#ifdef BCM_XGS_SUPPORT
	if ((bcm_rv = soc_mmu_init(u)) < 0) {
	    printk("%s: Unable to initialize MMU for device: %s\n",
		   ARG_CMD(a), bcm_errmsg(bcm_rv));
	    rv = CMD_FAIL;
	}
#endif
#endif
    } else if (!sal_strcasecmp(c, "all")) {
        /* Prevent hardware access during chip reset */
        system_shutdown(u, 0);
	if ((bcm_rv = system_init(u)) < 0) {
	    printk("%s: Unable to reset and initialize BCM driver: %s\n",
		   ARG_CMD(a), bcm_errmsg(bcm_rv));
	    rv = CMD_FAIL;
	}
    } else { /* look for selective init */
        uint32      flags = 0;
        int module_num;
        int found;

        if (bcm_attach_check(u) == BCM_E_UNIT) {
            if ((bcm_rv = bcm_attach(u, NULL, NULL, u)) < 0) {
	        return CMD_FAIL;
            }
        }

        do {
            found = 0;
            for (module_num = 0; module_num < BCM_MODULE__COUNT; module_num++) {
                if (!sal_strcasecmp(bcm_module_name(u, module_num), c)) {
                    found = 1;
                    flags = (uint32) module_num;
                    if ((bcm_rv = bcm_init_selective(u, flags)) < 0) {
                        printk("%s: Unable to initialize %s (flags 0x%x): %s\n",
                                    ARG_CMD(a), c, flags, bcm_errmsg(bcm_rv));
                        rv = CMD_FAIL;
                     }
                    break;
                }
            }
            if (!found) {
                printk("Unknown module: %s\n", c);
                usage = 1;
                break;
            }
        } while ((c = ARG_GET(a)) != NULL);
    }

    if (usage) {
	printk("%s: Unknown option: %s\n", ARG_CMD(a), c);
	rv = CMD_USAGE;
    }

    return(rv);
}

char sh_expr_usage[] =
    "Parameters: <c-style arithmetic expression>\n\t"
    "Evaluates an expression and sets the result as the return value.\n";

cmd_result_t
sh_expr(int u, args_t *a)
/*
 * Function: 	sh_expr
 * Purpose:	Expression evaluation support
 * Parameters:	u - unit #
 *		a - pointer to args, concatenates them if multiple
 * Returns:	CMD_OK if not executed, or result of last command execution.
 */
{
    char		expr[128];
    char		*c;
    INFIX_TYPE		result;

    c = ARG_GET(a);

    if (c == NULL) {
	return CMD_USAGE;
    }

    strncpy(expr, c, sizeof (expr));
    expr[sizeof (expr) - 1] = '\0';

    while ((c = ARG_GET(a)) != NULL) {
	int len = strlen(expr);
	strncpy(expr + len, c, sizeof (expr) - len);
	expr[sizeof (expr) - 1] = '\0';
    }

    if (infix_eval(expr, &result) < 0) {
	printk("Invalid expression: %s\n", expr);
	return CMD_FAIL;
    }

    return (int)result;
}

char sh_if_usage[] =
    "Parameters: <condition> [<command> ... [else <command> ...]]\n"
#ifndef COMPILER_STRING_CONST_LIMIT
    "\tExecute a list of command lines(s) only if <condition> is a\n\t"
    "non-zero integer or a successful sub-command-line.  Command lines\n\t"
    "of more than one word must be quoted, for example:\n\t"
    "\tif 1 \"echo hello\" \"echo world\"\n\t"
    "will display hello and world.  Simple left-to-right boolean\n\t"
    "operations are permitted, for example:\n\t"
    "\tif $?quickturn && !\"bist arl\" \"echo BIST failed\"\n\t"
    "Processing of multiple command lines ends when one of them fails\n\t"
    "unless IFError is set to FALSE (see help on set for more info).\n\t"
    "The return value from this command is the result of the last\n\t"
    "executed command.\n"
#endif
    ;

static int
if_cond(int u, args_t *a)
{
    int			negate = FALSE, cond;
    char		*c;

    if ((c = ARG_GET(a)) == NULL) {
	printk("%s: missing test condition\n", ARG_CMD(a));
	return -1;
    } else {
	for (negate = FALSE; *c == '!'; negate = !negate, c++) {
	    ;
	}

	if (*c == 0) {
	    cond = 0;
	} else {
	    cond = (sh_process_command(u, c) == CMD_OK);
	}

	return negate ? !cond : cond;
    }
}

cmd_result_t
sh_if(int u, args_t *a)
/*
 * Function: 	sh_if
 * Purpose:	Basic "if" support.
 * Parameters:	u - unit #
 *		a - pointer to args, expects <bool> <cmd list>
 * Returns:	CMD_OK if not executed, or result of last command execution.
 */
{
    cmd_result_t	rv = CMD_OK;
    char		*c;
    int			cond;

 again:
    if ((cond = if_cond(u, a)) < 0) {
	return CMD_USAGE;
    }

    if (!cond) {
	if ((c = ARG_GET(a)) != NULL && strcmp(c, "||") == 0) {
	    goto again;
	}
	while (c != NULL && sal_strcasecmp(c, "else") != 0) {
	    c = ARG_GET(a);
	}
	if (c == NULL) {
	    return rv;
	}
    }

    while ((rv == CMD_OK || !sh_set_iferror)
	   && (c = ARG_GET(a)) != NULL) {
	if (strcmp(c, "||") == 0) {
	    if (if_cond(u, a) < 0) {
		return CMD_USAGE;
	    }
	} else if (strcmp(c, "&&") == 0) {
	    goto again;
	} else if (sal_strcasecmp(c, "else") == 0) {
	    break;
	} else {
	    rv = sh_process_command(u, c);
	}
    }

    ARG_DISCARD(a);
    return rv;
}

#ifndef NO_SAL_APPL

static	cmd_result_t
sh_do_copy(args_t *a, char *s_src, char *s_dst)
/*
 * Function: 	sh_do_copy
 * Purpose:	Perform actual copy of data from source to destination.
 * Parameters:	a - args (Used only for ARG_CMD)
 *		s_src - file name to copy from
 *		s_dst - file name to copy to.
 * Returns:	CMD_OK/CMD_FAIL
 */
{
#ifdef NO_FILEIO
    return -1;
#else
    cmd_result_t volatile rv = CMD_FAIL;
    jmp_buf	ctrl_c;
    FILE * volatile f_src = NULL,	/* Source/destination FILE */
         * volatile f_dst = NULL;

    if (!setjmp(ctrl_c)) {
	char	buf[1024];
	int 	r, w;

	sh_push_ctrl_c(&ctrl_c);

	if ((f_src = sal_fopen(s_src, "rb")) == NULL) {
	    printk("%s: Error: unable to open input file: %s\n",
		   ARG_CMD(a), s_src);
	    goto done;
	}

	if ((f_dst = sal_fopen(s_dst, "wb")) == NULL) {
	    printk("%s: Error: unable to open output file: %s\n",
		   ARG_CMD(a), s_dst);
	    goto done;
	}

	while ((r = fread(buf, 1, sizeof(buf), (FILE *) f_src)) > 0)
	    if ((w = fwrite(buf, 1, r, (FILE *) f_dst)) != r) {
		printk("%s: Error writing %s\n", ARG_CMD(a), s_dst);
		goto done;
	    }

	if (r < 0) {
	    printk("%s: Error reading %s\n", ARG_CMD(a), s_src);
	    goto done;
	}

	rv = CMD_OK;
    } else {
	rv = CMD_INTR;
    }

 done:
    sh_pop_ctrl_c();
    if (f_src) {
	(void) sal_fclose((FILE *)f_src);
    }
    if (f_dst) {
	if (sal_fclose((FILE *)f_dst)) {
	    printk("%s: Error closing %s\n", ARG_CMD(a), s_dst);
	    rv = CMD_FAIL;
	}
	if (rv < 0) {
	    if (sal_remove(s_dst) < 0) {
		printk("%s: Error removing partial output file %s\n",
		       ARG_CMD(a), s_dst);
	    }
	}
    }
    return(rv);
#endif /* NO_FILEIO */
}

char sh_copy_usage[] =
    "Parameters: <source> [<destination>]\n\t"
    "Copy a file from one location to another, intended to copy\n\t"
    "to/from flash, but any valid source and destination file is valid.\n\t"
    "If destination is omitted, the default destination is a file of\n\t"
    "the same name in the current working directory.\n";

cmd_result_t
sh_copy(int u, args_t *a)
/*
 * Function: 	cmd_copy
 * Purpose:	Copy a file from one location to another, intended to copy
 *		to/from flash device but as long as the targets are
 *		accessible, it will work.
 * Parameters:	u - unit (ignored)
 *		a - arguments, expects "source" "destination", no destination
 *			and it will copy to console.
 * Returns:	CMD_OK/CMD_FAIL/CMD_USAGE.
 */
{
#ifdef NO_FILEIO
    return -1;
#else
    char *s_src,
         *s_dst; /* Source destination names */

    COMPILER_REFERENCE(u);

    if ((ARG_CNT(a) == 0) || (ARG_CNT(a) > 2)) {
	return(CMD_USAGE);
    }

    /*
     * Figure out source/destination names, if no destination then use
     * filename without path from source in current working directory.
     */

    s_src = ARG_GET(a);
    s_dst = ARG_GET(a);			/* May be NULL */

    if (!s_dst) {			/* First, look back for '/' */
	if ((s_dst = strrchr(s_src, '/')) != NULL) {
	    s_dst++;
	}
    }

    if (!s_dst) {			/* Look back for ':' */
	if ((s_dst = strrchr(s_src, ':')) != NULL) {
	    s_dst++;
	}
    }

    if (!s_dst) {
	printk("%s: Error: No file to copy TO\n", ARG_CMD(a));
	return(CMD_FAIL);
    }

    return(sh_do_copy(a, s_src, s_dst));
#endif /* NO_FILEIO */
}

char sh_remove_usage[] =
    "Parameters: <file>\n\t"
    "Remove a file from a filesystem\n";

cmd_result_t
sh_remove(int u, args_t *a)
/*
 * Function: 	sh_remove
 * Purpose:	Remove a file from a file system.
 * Parameters:	u - unit (ignored)
 *		a - arguments, expects "filename", or a serial of file names.
 * Returns:	CMD_OK/CMD_FAIL/CMD_USAGE.
 */
{
#ifdef NO_FILEIO
    return -1;
#else
    char	*c;
    int		rv;
    cmd_result_t result = CMD_OK;

    COMPILER_REFERENCE(u);

    if (0 == ARG_CNT(a)) {
	return(CMD_USAGE);
    }
    while ((c = ARG_GET(a)) != NULL) {
	rv = sal_remove(c);
	if (rv) {
	    printk("%s: Warning: failed to remove file: %s\n",
		   ARG_CMD(a), c);
	    result = CMD_FAIL;
	}
    }
    return(result);
#endif /* NO_FILEIO */
}

char sh_rename_usage[] =
    "Parameters: <file_old> <file_new>\n\t"
    "Rename a file on a filesystem.  For FTP, the new file name\n\t"
    "must be a base name only.  Can't rename across file systems.\n";

cmd_result_t
sh_rename(int u, args_t *a)
/*
 * Function: 	sh_rename
 * Purpose:	Rename a file on a file system.
 * Parameters:	u - unit (ignored)
 *		a - arguments
 * Returns:	CMD_OK/CMD_FAIL/CMD_USAGE.
 */
{
#ifdef NO_FILEIO
    return -1;
#else
    char	*file_old, *file_new;
    cmd_result_t result = CMD_OK;

    COMPILER_REFERENCE(u);

    if (2 != ARG_CNT(a)) {
	return(CMD_USAGE);
    }

    file_old = ARG_GET(a);
    file_new = ARG_GET(a);

    if (sal_rename(file_old, file_new) < 0) {
	printk("%s: Warning: failed to rename file: %s\n",
	       ARG_CMD(a), file_old);
	result = CMD_FAIL;
    }

    return(result);
#endif /* NO_FILEIO */
}

char sh_mkdir_usage[] =
    "Parameters: <path>\n\t"
    "Make a directory.\n";

cmd_result_t
sh_mkdir(int u, args_t *a)
/*
 * Function: 	sh_mkdir
 * Purpose:	Make a directory.
 * Parameters:	u - unit (ignored)
 *		a - arguments
 * Returns:	CMD_OK/CMD_FAIL/CMD_USAGE.
 */
{
    char	*path;
    cmd_result_t result = CMD_OK;

    COMPILER_REFERENCE(u);

    if (ARG_CNT(a) != 1) {
	return(CMD_USAGE);
    }

    path = ARG_GET(a);

    if (sal_mkdir(path) < 0) {
	printk("%s: Warning: failed to create directory: %s\n",
	       ARG_CMD(a), path);
	result = CMD_FAIL;
    }

    return(result);
}

char sh_rmdir_usage[] =
    "Parameters: <path>\n\t"
    "Remove a directory.\n";

cmd_result_t
sh_rmdir(int u, args_t *a)
/*
 * Function: 	sh_rmdir
 * Purpose:	Make a directory.
 * Parameters:	u - unit (ignored)
 *		a - arguments
 * Returns:	CMD_OK/CMD_FAIL/CMD_USAGE.
 */
{
    char	*path;
    cmd_result_t result = CMD_OK;

    COMPILER_REFERENCE(u);

    if (ARG_CNT(a) != 1) {
	return(CMD_USAGE);
    }

    path = ARG_GET(a);

    if (sal_rmdir(path) < 0) {
	printk("%s: Warning: failed to remove directory: %s\n",
	       ARG_CMD(a), path);
	result = CMD_FAIL;
    }

    return(result);
}

char sh_date_usage[] =
    "Parameters: \"YYYY/MM/DD hh:mm:ss\"\n\t"
    "If no options are given, displays the current date.  If date is\n\t"
    "specified in format shown (quotes required), sets the current date.\n\t"
    "Date must be in local time according to the time zone.\n";

#ifndef __KERNEL__
static int _date_val(char **sp)
{
    int val = 0;
    while (**sp && ! isdigit((unsigned) **sp))
	(*sp)++;
    while (isdigit((unsigned) **sp))
	val = val * 10 + ((int) *(*sp)++ - '0');
    return val;
}
#endif

cmd_result_t
sh_date(int u, args_t *a)
{
#ifndef __KERNEL__	
    char       		*c;
    struct tm		tm, *tmp;
    sal_time_t		val;
    char		buf[64];

    COMPILER_REFERENCE(u);

    sal_memset(&tm, 0, sizeof(struct tm));
    if ((c = ARG_GET(a)) != 0) {
	if (ARG_GET(a))
	    return CMD_USAGE;

	tm.tm_year = _date_val(&c) - 1900;
	tm.tm_mon = _date_val(&c) - 1;
	tm.tm_mday = _date_val(&c);
	tm.tm_hour = _date_val(&c);
	tm.tm_min = _date_val(&c);
	tm.tm_sec = _date_val(&c);

	val = mktime(&tm);

	if (sal_date_set(&val) < 0)
	    return CMD_FAIL;
    }

    if (sal_date_get(&val) < 0)
	return CMD_FAIL;

    tmp = localtime((time_t *) &val);

    strftime(buf, sizeof (buf), "%Y/%m/%d %H:%M:%S %Z", tmp);

    printk("%s\n", buf);

#endif
    return CMD_OK;
}

#endif /* NO_SAL_APPL */

char sh_time_usage[] =
    "Parameters: [<command> ...]\n\t"
    "Runs command(s) and displays how much time they took to run.\n\t"
    "Example: time \"sleep 1\" \"sleep 2\"\n\t"
    "If no command is specified, displays the current time.\n";

#ifdef	COMPILER_HAS_DOUBLE
#define	GETTIME	sal_time_double()
#define	DCLTIME	double
#define	FMTTIME	"%.6f sec"
#else
#define	GETTIME	sal_time_usecs()
#define	DCLTIME	sal_usecs_t
#define	FMTTIME	"%u usec"
#endif

cmd_result_t
sh_time(int u, args_t *a)
{
    int			rv = CMD_OK;
    char		*c;
    DCLTIME		t0, ts, te;
    int			ncmd = 0;

    if (ARG_CNT(a) == 0) {
	printk(FMTTIME "\n", GETTIME);
	return CMD_OK;
    }

    t0 = ts = te = 0;
    while (rv == CMD_OK && (c = ARG_GET(a)) != NULL) {
	ts = GETTIME;
	if (ncmd == 0) {
	    t0 = ts;
	}
	if ((rv = sh_process_command(u, c)) != CMD_OK) {
	    break;
	}
	ncmd++;
	te = GETTIME;
	printk("time{%s} = " FMTTIME "\n", c, te - ts);
    }

    if (ncmd > 1) {
	printk("time{TOTAL} = " FMTTIME "\n", te - t0);
    }

    return rv;
}

#undef	GETTIME
#undef	DCLTIME
#undef	FMTTIME


#ifndef NO_SAL_APPL

char sh_flashinit_usage[] =
    "Parameters: [format=yes|no] [loader=<os-loader>] [OS=<os_file>]\n\t"
    "Format (clear) the flash disk contents if a flash disk is available\n\t"
    "The command ALWAYS prompts for confirmation from the console.\n";

cmd_result_t
sh_flashinit(int u, args_t *a)
/*
 * Function: 	sh_flashinit
 * Purpose:	Initialize flash disk, and/or copy files to the flash.
 * Parameters:	u - unit number (ignored)
 *		a - arguments.
 * Returns:	CMD_OK, CMD_USAGE, CMD_FAIL.
 */
{
    cmd_result_t	rv = CMD_OK;
    char		*file_loader, *file_os;
    int			format;
    parse_table_t	pt;

    COMPILER_REFERENCE(u);

    parse_table_init(u, &pt);
    parse_table_add(&pt, "Format", PQ_BOOL, 0, &format, 0);
    parse_table_add(&pt, "Loader", PQ_STRING, 0, &file_loader, 0);
    parse_table_add(&pt, "OS", PQ_STRING, 0, &file_os, 0);

    if (0 == ARG_CNT(a)) {
	return(CMD_USAGE);
    }
    if (0 > parse_arg_eq(a, &pt)) {
	printk("%s: Error: Invalid option: %s\n", ARG_CMD(a), ARG_CUR(a));
	parse_arg_eq_done(&pt);
	return(CMD_FAIL);
    } else if (0 != ARG_CNT(a)) {
	printk("%s: Error: extra options starting with \"%s\"\n",
	       ARG_CMD(a), ARG_CUR(a));
	parse_arg_eq_done(&pt);
	return(CMD_FAIL);
    }

#ifdef ZT6501
    if (file_loader && *file_loader) {
	printk("%s: Warning: Writing loader will re-format flash disk\n",
	       ARG_CMD(a));
	format = TRUE;
    }
#endif /* ZT6501 */

    if (format) {
	char	input[32];

	printk("%s: Warning: Formatting flash will destroy all files\n",
	       ARG_CMD(a));

	if (NULL == sal_readline("OK to continue (yes/no)? ",
				 input, sizeof(input), "no")) {
	    rv = CMD_FAIL;
	} else if (sal_strncasecmp("yes", input, strlen(input))) {
	    rv = CMD_FAIL;
	}
    }

    /*
     * At this point, if rv is still CMD_OK, then go ahead and do
     * what was asked.
     */
    if (CMD_OK == rv) {
	if (file_loader && *file_loader &&
	    (0 != sal_flash_boot(file_loader))) {/* Set boot loader image */
	    printk("%s: Error: Unable to flash loader image: %s\n",
		   ARG_CMD(a), file_loader);
	    rv = CMD_FAIL;
	}
	if ((CMD_OK == rv ) && format && (0 != sal_flash_init(TRUE))) {
	    /* Format file system  */
	    printk("%s: Error: Unable to Initialize flash file system\n",
		   ARG_CMD(a));
	    rv = CMD_FAIL;
	}
	if ((CMD_OK == rv) && file_os && *file_os) { /* Load OS file */
#define VX_BOOT_FILE "flash:bcm"
	    rv = sh_do_copy(a, file_os, VX_BOOT_FILE);
	}
    }
    parse_arg_eq_done(&pt);
    return(rv);
}

char sh_flashsync_usage[] =
    "Sync up the flash disk contents with dosFS (dosFS2 only)\n";

cmd_result_t
sh_flashsync(int u, args_t *a)
/*
 * Function:    sh_flashsync
 * Purpose:     Sync up the flash disk contents with dosFS
 * Parameters:  u - unit number (ignored)
 *              a - arguments.
 * Returns:     CMD_OK, CMD_USAGE, CMD_FAIL.
 */
{
    cmd_result_t rv = CMD_OK;

    COMPILER_REFERENCE(u);

#if defined(VXWORKS) || defined(__ECOS)
    if (sal_flash_sync() < 0)
        rv = CMD_FAIL;
#endif

    return(rv);
}

char sh_cd_usage[] =
    "Parameters: [[-sethome] <directory>]\n\t"
    "Change current working directory. <directory> must be of the\n\t"
    "form \"usr%password@host:<path>\" or \"/flash/<directory>\" for\n\t"
    "FTP to remote system or local flash disk respectively.\n\t"
    "If the target directory does not exist, this command may still\n\t"
    "succeed, but attempts to access files may fail.\n\t"
    "The -sethome option sets the new directory to be the home directory.\n";

/*ARGSUSED*/
cmd_result_t
sh_cd(int u, args_t *a)
/*
 * Function: 	sh_cd
 * Purpose:	Change directory
 * Parameters:	u - unit number (ignored)
 * Returns:	CMD_OK/CMD_FAIL/CMD_USAGE
 */
{
    char	*c;
    int		sethome = 0;

    COMPILER_REFERENCE(u);

    c = ARG_GET(a);

    if (c != NULL && strcmp(c, "-sethome") == 0) {
	sethome = 1;
	c = ARG_GET(a);
    }

    if (sal_cd(c)) {		/* NULL changes to "home" dir */
	if (c == NULL) {
	    printk("%s: Invalid home directory\n", ARG_CMD(a));
	} else {
	    printk("%s: Invalid directory: %s\n", ARG_CMD(a), c);
	}

	return(CMD_FAIL);
    }

    if (sethome && c != NULL && sal_homedir_set(c) < 0) {
	printk("%s: Unable to set home directory: %s\n", ARG_CMD(a), c);
	return CMD_FAIL;
    }

    return(CMD_OK);
}

cmd_result_t
sh_do_more(FILE *f)
/*
 * Function: 	sh_do_more
 * Purpose:	Copy a file to console, stopping line "more".
 * Parameters:	f - pointer to FILE to read from.
 * Returns:	CMD_OK
 */
{
#ifdef NO_FILEIO
    return -1;
#else
    int		line, next_stop;
    char	buf[1024];

    line = 0;
    next_stop = sh_set_more_lines;

    while (fgets(buf, sizeof(buf) - 1, f)) {
	/* Dump data */
	printk("%s", buf);
	if (++line == next_stop) {
	    extern char readchar(char *s);
	    int c;
	    printk(SAL_VT100_SO
		   "Line: %d ----- 'q' to quit, any key to continue -----"
		   SAL_VT100_SE,
		   line);
	    c = sal_readchar("");
	    printk("\r" SAL_VT100_CE);
	    switch (c) {
	    case EOF:
	    case 'q':
	    case 'Q':
		return(CMD_OK);
	    case '\r':
	    case '\n':
		next_stop++;
		break;
	    default:
		next_stop += sh_set_more_lines;
		break;			/* Just keep going */
	    }
	}
    }
    return(CMD_OK);
#endif /* NO_FILEIO */
}

char sh_more_usage[] =
    "Parameters: <file> ...\n\t"
    "Copy the listed files to the console, stopping after a set number\n\t"
    "of lines. The default number of lines is 20, but it may be changed\n\t"
    "by setting \"MoreLines\" to a different number. \"help set\" for\n\t"
    "more information\n";

cmd_result_t
sh_more(int u, args_t *a)
/*
 * Function: 	sh_more
 * Purpose:	More a file to the TTY.
 * Parameters:	u - unit (ignored)
 *		a - args, each of the files to be displayed.
 * Returns:	CMD_OK/CMD_FAIL/CMD_USAGE
 */
{
#ifdef NO_FILEIO
    return -1;
#else
    jmp_buf	ctrl_c;
    volatile cmd_result_t rv = CMD_OK;
    FILE * volatile f = NULL;
    char	*c;

    COMPILER_REFERENCE(u);

    if (0 == ARG_CNT(a)) {
	return(CMD_USAGE);
    }

    /*
     * Try to catch ^C to avoid leaving file open if more is ^C'd.
     * There are still a number of unlikely race conditions here.
     */

    if (!setjmp(ctrl_c)) {
	sh_push_ctrl_c(&ctrl_c);
	while ((CMD_OK == rv) && (NULL != (c = ARG_GET(a)))) {
	    f = sal_fopen(c, "r");
	    if (!f) {
		printk("%s: Error: Unable to open file: %s\n",
		       ARG_CMD(a), c);
		rv = CMD_FAIL;
	    } else {
		rv = sh_do_more((FILE *)f);
		sal_fclose((FILE *)f);		/* Cast for un-volatile */
		f = NULL;
	    }
	}
    } else if (f) {
	sal_fclose((FILE *)f);
	f = NULL;
	rv = CMD_INTR;
    }
    sh_pop_ctrl_c();
    return(rv);
#endif /* NO_FILEIO */
}

char sh_write_usage[] =
    "Parameters: <file> [<word> ...]\n\t"
    "Write data to a file. If only file is given, the console is read\n\t"
    "until EOF (CTRL-D) is seen. Otherwise, all the words on the\n\t"
    "command line are written with a single \"Space\" between them\n";

cmd_result_t
sh_write(int u, args_t *a)
/*
 * Function: 	cmd_write
 * Purpose:	Write words into a file.
 * Parameters:	u - SOC unit number, IGNORED.
 *		a - pointer to args, expects filename and optional text.
 * Returns:	CMD_OK, CMD_FAIL.
 */
{
#ifdef NO_FILEIO
    return -1;
#else
    jmp_buf	ctrl_c;
    volatile cmd_result_t rv = CMD_OK;
    FILE * volatile f = NULL;
    char	*c, *fname;
    char	input[512];

    COMPILER_REFERENCE(u);

    input[sizeof(input) - 1] = '\0';

    if (0 == ARG_CNT(a)) {
	return(CMD_USAGE);
    }

    /*
     * Try to catch ^C to avoid leaving file open if more is ^C'd.
     * There are still a number of unlikely race conditions here.
     */

    if (!setjmp(ctrl_c)) {
	sh_push_ctrl_c(&ctrl_c);

	fname = ARG_GET(a);

	/* If no args, prompt user, otherwise, just write them out */

	f = sal_fopen(fname, "w");
	if (!f) {
	    printk("%s: Error: Unable to open file: %s\n",
		   ARG_CMD(a), fname);
	    rv = CMD_FAIL;
	} else if (0 == ARG_CNT(a)) {	/* Interactive */
	    while((sal_readline("< ", input, sizeof(input) - 1, NULL))) {
		fprintf(f, "%s\n", input);
	    }
	} else {
	    int	first = TRUE;

	    while ((c = ARG_GET(a)) != NULL) {
		fprintf((FILE *)f, "%s%s", first ? "" : " ", c);
		first = FALSE;
	    }
	    fprintf((FILE *)f, "\n");
	}
    } else {
	rv = CMD_INTR;
    }
    if (f) {
	sal_fclose((FILE *)f);
	f = NULL;
    }
    sh_pop_ctrl_c();
    return(rv);
#endif /* NO_FILEIO */
}


#endif /* NO_SAL_APPL */

/*
 * Background-command Support
 */

#define _MAX_BG_TASKS 10

typedef struct {
    int unit;
    int idx;
    int job;
    sal_thread_t thread_id;
    sal_sem_t sem;
    char cmd[ARGS_BUFFER];
} _bg_job_t;

static _bg_job_t	*_bg_jobs[SOC_MAX_NUM_DEVICES][_MAX_BG_TASKS];
static int		_bg_job_count;

/*
 * Run the background thread
 */
static void
_bg_cmd(void *cookie)
{
    _bg_job_t	*info;
    int		rv;

    info = (_bg_job_t *)cookie;

    sal_sem_take(info->sem, sal_sem_FOREVER);

    switch((rv = sh_process_command(info->unit, info->cmd))) {
    case CMD_OK:
    case CMD_EXIT:
    case CMD_INTR:
        break;
    default:
        soc_cm_print("bg: error in job %d: code %d\n", info->job, rv);
        break;
    }

    soc_cm_debug(DK_VERBOSE, "bg: end of job %d\n", info->job);

    sal_sem_destroy(info->sem);
    _bg_jobs[info->unit][info->idx] = NULL;
    sal_free(info);

    sal_thread_exit(rv);
}

void
sh_bg_init(void)
{
    sal_memset(_bg_jobs, 0, sizeof(_bg_jobs));
}

typedef void (*void_fn_ptr)(void *);

char sh_bg_usage[] =
    "bg \"<cmd>\"\n"
    "    Execute cmd in the background\n";

cmd_result_t
sh_bg(int unit, args_t *args)
{
    _bg_job_t	*info;
    char	*cmd;
    int		i;

    if (ARG_CNT(args) != 1) {
        return CMD_USAGE;
    }

    /* find an empty slot */
    info = NULL;
    for (i = 0; i < _MAX_BG_TASKS; i++) {
        if (_bg_jobs[unit][i] == NULL) {
	    info = sal_alloc(sizeof(_bg_job_t), "bg_job");
	    if (info == NULL) {
		soc_cm_print("bg: cannot allocate job info\n");
		return CMD_FAIL;
	    }
	    sal_memset(info, 0, sizeof(_bg_job_t));
	    _bg_jobs[unit][i] = info;
	    break;
        }
    }

    if (info == NULL) {
        soc_cm_print("bg: ERROR: too many background tasks\n");
        return CMD_FAIL;
    }

    info->unit = unit;
    info->idx = i;
    info->job = ++_bg_job_count;
    info->sem = sal_sem_create("bg_job", sal_sem_BINARY, 0);
    if (!info->sem) {
        soc_cm_print("bg: ERROR: cannot create task semaphore\n");
	sal_free(info);
	_bg_jobs[unit][i] = NULL;
        return CMD_FAIL;
    }
    cmd = ARG_GET(args);
    sal_memcpy(info->cmd, cmd, ARGS_BUFFER);

    info->thread_id = sal_thread_create("bcmBG",
					128*1024,
                                        SOC_BG_THREAD_PRI,
                                        (void_fn_ptr)_bg_cmd,
					(void *)info);
    if (!info->thread_id || (info->thread_id == SAL_THREAD_ERROR)) {
        soc_cm_print("bg: ERROR: cannot create thread\n");
	sal_sem_destroy(info->sem);
	sal_free(info);
	_bg_jobs[unit][i] = NULL;
        return CMD_FAIL;
    }
    soc_cm_debug(DK_VERBOSE, "bg: starting job %d\n", info->job);
    sal_sem_give(info->sem);

    return CMD_OK;
}

char sh_jobs_usage[] =
    "jobs\n"
    "    List jobs running in the background\n";

cmd_result_t
sh_jobs(int unit, args_t *args)
{
    int		i;
    _bg_job_t	*info;

    COMPILER_REFERENCE(args);

    for (i = 0; i < _MAX_BG_TASKS; i++) {
	info = _bg_jobs[unit][i];
        if (info != NULL) {
	    soc_cm_print("Job %d: %s\n", info->job, info->cmd);
	}
    }

    return CMD_OK;
}

char sh_kill_usage[] =
    "kill <n>\n"
    "    Destroy a background task.\n"
    "    Use 'jobs' to see a list of tasks currently in the  background.\n";

cmd_result_t
sh_kill(int unit, args_t *args)
{
    int		i, job;
    _bg_job_t	*info;

    if (!ARG_CNT(args)) {
        return CMD_USAGE;
    }

    job = strtoul(_ARG_GET(args), NULL, 10);

    for (i = 0; i < _MAX_BG_TASKS; i++) {
	info = _bg_jobs[unit][i];
        if (info != NULL && info->job == job) {
	    sal_thread_destroy(info->thread_id);
	    soc_cm_print("Job %d killed\n", info->job);
	    sal_sem_destroy(info->sem);
	    sal_free(info);
	    _bg_jobs[unit][i] = NULL;
	    return CMD_OK;
	}
    }
    soc_cm_print("Job %d not found\n", job);
    return CMD_FAIL;
}

int resTestMax(int x,int y) {
    return (x>y)?x:y;
}
#ifdef _GET_NONBLANK
#undef _GET_NONBLANK
#endif /* def _GET_NOBLANK */
#define _GET_NONBLANK(a,c) \
    do { \
        (c) = ARG_GET(a); \
        if (NULL == (c)) { \
            return CMD_USAGE; \
        } \
    } while (0)

#ifdef BCM_ESW_SUPPORT 
char cmd_sh_restest_usage[] =
#ifdef COMPILER_STRING_CONST_LIMIT
    "  restest <args>\n"
#else
    "(caps are obligatory, lower optional; <var> = use var's value; [optional])\n"
    "  ResTest Create [Unit]   (create test resources locally or against unit)\n"
    "  ResTest DEstroy [Unit]  (destroy test resources or unit resources)\n"
    "  ResTest DUmp [Unit]     (dump test resources or unit resources)\n"
    "  ResTest T0              (basic alloc and free, increasing block sizes)\n"
    "  ResTest T1              (basic alloc and free, decreasing block sizes)\n"
    "  ResTest T2              (allocate and then free entire pools)\n"
    "  ResTest T3              (alloc pools +1 alloc, free pools, +1 alloc, free)\n"
    "  ResTest T4              (free unallocated elements/blocks)\n"
    "  ResTest T5              (aligned alloc tests against bitmap allocator)\n"
    "If test resources exist, the T* series will use the test resources (this is\n"
    "the preferred scenario), else they will use the unit resources.  The Create,\n"
    "DEstroy, and DUmp subcommands assume test resources unless the unit keyword\n"
    "is specified.\n"
#endif
    ;

#ifdef TEST_BLOCKS
#undef TEST_BLOCKS
#endif /* def TEST_BLOCKS */
#define TEST_BLOCKS 64

cmd_result_t
cmd_sh_restest(int unit, args_t *a){
    static shr_mres_handle_t         localRes = NULL;
    char                             *c;
    int                              result = BCM_E_NONE;
    int                              xresult;
    int                              index;
    int                              res;
    int                              pool;
    int                              rescount;
    int                              auxpool;
    int                              used;
    shr_res_allocator_t              restype;
    char                             name[32];
    shr_res_idxres_extras_t          idxresExtras;
    shr_res_aidxres_extras_t         aidxresExtras;
    shr_res_mdb_extras_t             mdbExtras;
    int                              base[SHR_RES_ALLOCATOR_COUNT << 1][TEST_BLOCKS];
    int                              size[SHR_RES_ALLOCATOR_COUNT << 1][TEST_BLOCKS];
    int                              skip[SHR_RES_ALLOCATOR_COUNT << 1];
    void                             *extraPtr;
    const void                       *extraGet;

    _GET_NONBLANK(a,c);
    if (!sal_strncasecmp(c,"create",resTestMax(strlen(c),1))) {
        c = ARG_GET(a);
        if ((!c) || (!sal_strncasecmp(c,"test",resTestMax(strlen(c),1)))) {
            if (localRes) {
                /* destroy old resources and reinit */
                result = shr_mres_destroy(localRes);
                if (BCM_E_NONE != result) {
                    printk("unable to destroy old %p resources: %d (%s)\n",
                           (void*)localRes,
                           result,
                           _SHR_ERRMSG(result));
                }
            } else {
                /* okay so far */
                result = BCM_E_NONE;
            }
            if (BCM_E_NONE == result) {
                result = shr_mres_create(&localRes,
                                         SHR_RES_ALLOCATOR_COUNT << 1 /* types */,
                                         SHR_RES_ALLOCATOR_COUNT /* pools */ );
                if (BCM_E_NONE != result) {
                    printk("failed to set up new resources: %d (%s)\n",
                           result,
                           _SHR_ERRMSG(result));
                }
            }
            for (index = 0;
                 (BCM_E_NONE == result) && (index < SHR_RES_ALLOCATOR_COUNT);
                 index++) {
                switch (index) {
                case SHR_RES_ALLOCATOR_BITMAP:
                    extraPtr = NULL;
                    break;
                case SHR_RES_ALLOCATOR_IDXRES:
                    extraPtr = &idxresExtras;
                    idxresExtras.scaling_factor = 1;
                    break;
                case SHR_RES_ALLOCATOR_AIDXRES:
                    extraPtr = &aidxresExtras;
                    aidxresExtras.blocking_factor = 7;
                    break;
                case SHR_RES_ALLOCATOR_MDB:
                    extraPtr = &mdbExtras;
                    mdbExtras.bank_size = 1024;
                    mdbExtras.free_lists = 10;
                    mdbExtras.free_counts[0] = 2;
                    mdbExtras.free_counts[1] = 4;
                    mdbExtras.free_counts[2] = 8;
                    mdbExtras.free_counts[3] = 16;
                    mdbExtras.free_counts[4] = 32;
                    mdbExtras.free_counts[5] = 64;
                    mdbExtras.free_counts[6] = 128;
                    mdbExtras.free_counts[7] = 256;
                    mdbExtras.free_counts[8] = 512;
                    mdbExtras.free_counts[9] = 1024;
                    break;
                default:
                    extraPtr = NULL;
                } /* switch(index) */
                sal_snprintf(&(name[0]),
                             sizeof(name) - 1,
                             "test: pool %d",
                             index);
                result = shr_mres_pool_set(localRes,
                                           index /* as pool ID */,
                                           index /* as manager type */,
                                           1024 /* low ID */,
                                           6144 /* count */,
                                           extraPtr,
                                           &(name[0]));
                if (BCM_E_NONE != result) {
                    printk("failed to set up %p pool %d: %d (%s)\n",
                           (void*)localRes,
                           index,
                           result,
                           _SHR_ERRMSG(result));
                }
            }
            for (index = 0;
                 (BCM_E_NONE == result) &&
                  (index < (SHR_RES_ALLOCATOR_COUNT << 1));
                 index++) {
                sal_snprintf(&(name[0]),
                             sizeof(name) - 1,
                             "test: type %d",
                             index);
                result = shr_mres_type_set(localRes,
                                           index /* as type ID */,
                                           index % SHR_RES_ALLOCATOR_COUNT,
                                           1 /* element size */,
                                           &(name[0]));
                if (BCM_E_NONE != result) {
                    printk("failed to set up %p type %d: %d (%s)\n",
                           (void*)localRes,
                           index,
                           result,
                           _SHR_ERRMSG(result));
                }
            }
            if (BCM_E_NONE == result) {
                printk("initialised %p resources successfully\n",
                       (void*)localRes);
            }
        } else if (!sal_strncasecmp(c,"unit",resTestMax(strlen(c),1))) {
            result = shr_res_init(unit,
                                  SHR_RES_ALLOCATOR_COUNT << 1 /* types */,
                                  SHR_RES_ALLOCATOR_COUNT /* pools */ );
            if (BCM_E_NONE != result) {
                printk("failed to set up unit %d new resources: %d (%s)\n",
                       unit,
                       result,
                       _SHR_ERRMSG(result));
            }
            for (index = 0;
                 (BCM_E_NONE == result) && (index < SHR_RES_ALLOCATOR_COUNT);
                 index++) {
                switch (index) {
                case SHR_RES_ALLOCATOR_BITMAP:
                    extraPtr = NULL;
                    break;
                case SHR_RES_ALLOCATOR_IDXRES:
                    extraPtr = &idxresExtras;
                    idxresExtras.scaling_factor = 1;
                    break;
                case SHR_RES_ALLOCATOR_AIDXRES:
                    extraPtr = &aidxresExtras;
                    aidxresExtras.blocking_factor = 7;
                    break;
                case SHR_RES_ALLOCATOR_MDB:
                    extraPtr = &mdbExtras;
                    mdbExtras.bank_size = 1024;
                    mdbExtras.free_lists = 10;
                    mdbExtras.free_counts[0] = 2;
                    mdbExtras.free_counts[1] = 4;
                    mdbExtras.free_counts[2] = 8;
                    mdbExtras.free_counts[3] = 16;
                    mdbExtras.free_counts[4] = 32;
                    mdbExtras.free_counts[5] = 64;
                    mdbExtras.free_counts[6] = 128;
                    mdbExtras.free_counts[7] = 256;
                    mdbExtras.free_counts[8] = 512;
                    mdbExtras.free_counts[9] = 1024;
                    break;
                default:
                    extraPtr = NULL;
                } /* switch(index) */
                sal_snprintf(&(name[0]),
                             sizeof(name) - 1,
                             "test: unit %d pool %d",
                             unit,
                             index);
                result = shr_res_pool_set(unit,
                                          index /* as pool ID */,
                                          index /* as manager type */,
                                          1024 /* low ID */,
                                          6144 /* count */,
                                          extraPtr,
                                          &(name[0]));
                if (BCM_E_NONE != result) {
                    printk("failed to set up unit %d pool %d: %d (%s)\n",
                           unit,
                           index,
                           result,
                           _SHR_ERRMSG(result));
                }
            }
            for (index = 0;
                 (BCM_E_NONE == result) &&
                  (index < (SHR_RES_ALLOCATOR_COUNT << 1));
                 index++) {
                sal_snprintf(&(name[0]),
                             sizeof(name) - 1,
                             "test: unit %d type %d",
                             unit,
                             index);
                result = shr_res_type_set(unit,
                                          index /* as type ID */,
                                          index % SHR_RES_ALLOCATOR_COUNT,
                                          1 /* element size */,
                                          &(name[0]));
                if (BCM_E_NONE != result) {
                    printk("failed to set up unit %d type %d: %d (%s)\n",
                           unit,
                           index,
                           result,
                           _SHR_ERRMSG(result));
                }
            }
            if (BCM_E_NONE == result) {
                printk("initialised unit %d resources successfully\n", unit);
            }
        } else {
            printk("invalid argument: %s\n", c);
            return CMD_USAGE;
        }
    } else if (!sal_strncasecmp(c,"destroy",resTestMax(strlen(c),2))) {
        c = ARG_GET(a);
        if ((!c) || (!sal_strncasecmp(c,"test",resTestMax(strlen(c),1)))) {
            if (localRes) {
                result = shr_mres_destroy(localRes);
                if (BCM_E_NONE == result) {
                    printk("destroyed %p resources successfully\n",
                           (void*)localRes);
                    localRes = NULL;
                } else {
                    printk("failed to destroy %p resources: %d (%s)\n",
                           (void*)localRes,
                           result,
                           _SHR_ERRMSG(result));
                }
            } else {
                printk("no test resources to destroy\n");
                result = BCM_E_INIT;
            }
        } else if (!sal_strncasecmp(c,"unit",resTestMax(strlen(c),1))) {
            result = shr_res_detach(unit);
            if (BCM_E_NONE == result) {
                printk("destroyed unit %d resources successfully\n", unit);
            } else {
                printk("failed to destroy unit %d resources: %d (%s)\n",
                       unit,
                       result,
                       _SHR_ERRMSG(result));
            }
        } else {
            printk("invalid argument: %s\n", c);
            return CMD_USAGE;
        }
    } else if (!sal_strncasecmp(c,"dump",resTestMax(strlen(c),2))) {
        c = ARG_GET(a);
        if ((!c) || (!sal_strncasecmp(c,"test",resTestMax(strlen(c),1)))) {
            if (localRes) {
                result = shr_mres_dump(localRes);
                if (BCM_E_NONE != result) {
                    printk("failed to dump test resource information:"
                           " %d (%s)\n",
                           result,
                           _SHR_ERRMSG(result));
                }
            } else {
                printk("no test resources to dump\n");
                result = BCM_E_INIT;
            }
        } else if (!sal_strncasecmp(c,"unit",resTestMax(strlen(c),1))) {
            result = shr_res_dump(unit);
            if (BCM_E_NONE != result) {
                printk("failed to dump unit %d resource information:"
                       " %d (%s)\n",
                       unit,
                       result,
                       _SHR_ERRMSG(result));
            }
        } else {
            printk("invalid argument: %s\n", c);
            return CMD_USAGE;
        }
    } else if (!sal_strncasecmp(c, "t0", resTestMax(strlen(c), 2))) {
        if (localRes) {
            result = shr_mres_get(localRes, &rescount, NULL);
            if (BCM_E_NONE == result) {
                if (rescount > SHR_RES_ALLOCATOR_COUNT << 1) {
                    rescount = SHR_RES_ALLOCATOR_COUNT << 1;
                }
            } else {
                printk("unable to retrieve %p resource type count : %d (%s)\n",
                       (void*)localRes,
                       result,
                       _SHR_ERRMSG(result));
                rescount = 0;
            }
            for (res = 0;
                 (BCM_E_NONE == result) && (res < rescount);
                 res++) {
                result = shr_mres_type_get(localRes, res, &pool, NULL, NULL);
                if (BCM_E_NONE == result) {
                    result = shr_mres_pool_get(localRes,
                                               pool,
                                               &restype,
                                               NULL,
                                               NULL,
                                               NULL,
                                               NULL);
                    if (BCM_E_NONE != result) {
                        printk("unable to retrieve %p type %d -> pool %d"
                               " manager: %d (%s)\n",
                               (void*)localRes,
                               res,
                               pool,
                               result,
                               _SHR_ERRMSG(result));
                    }
                } else {
                    printk("unable to retrieve %p type %d pool: %d (%s)\n",
                           (void*)localRes,
                           res,
                           result,
                           _SHR_ERRMSG(result));
                }
                for (index = 0;
                     (BCM_E_NONE == result) && (index < TEST_BLOCKS);
                     index++) {
                    switch (restype) {
                    case SHR_RES_ALLOCATOR_IDXRES:
                        /* can only do 'WITH_ID' for larger than one */
                        size[res][index] = 1;
                        break;
                    default:
                        size[res][index] = 1 + index;
                        break;
                    }
                    result = shr_mres_alloc(localRes,
                                            res,
                                            0 /* flags */,
                                            size[res][index],
                                            &(base[res][index]));
                    printk("allocated %p:%d:%d\r",
                           (void*)localRes,
                           res,
                           index);
                    if (BCM_E_NONE != result) {
                        printk("failed to allocate %p resource %d block %d"
                               " of %d elements: %d (%s)\n",
                               (void*)localRes,
                               res,
                               index,
                               size[res][index],
                               result,
                               _SHR_ERRMSG(result));
                    }
                }
            }
            for (res = 0;
                 (BCM_E_NONE == result) && (res < rescount);
                 res++) {
                for (index = 0;
                     (BCM_E_NONE == result) && (index < TEST_BLOCKS);
                     index++) {
                    result = shr_mres_check(localRes,
                                            res,
                                            size[res][index],
                                            base[res][index]);
                    printk("checked %p:%d:%d     \r",
                           (void*)localRes,
                           res,
                           index);
                    if (BCM_E_EXISTS != result) {
                        printk("unexpected result checking %p resource %d"
                               " block %d at %d (%d elements): %d (%s)\n",
                               (void*)localRes,
                               res,
                               index,
                               base[res][index],
                               size[res][index],
                               result,
                               _SHR_ERRMSG(result));
                        result = BCM_E_INTERNAL;
                    } else {
                        /* correct value */
                        result = BCM_E_NONE;
                    }
                }
            }
            for (res = 0;
                 (BCM_E_NONE == result) && (res < rescount);
                 res++) {
                for (index = 0;
                     (BCM_E_NONE == result) && (index < TEST_BLOCKS);
                     index++) {
                    result = shr_mres_free(localRes,
                                           res,
                                           size[res][index],
                                           base[res][index]);
                    printk("freed %p:%d:%d                \r",
                           (void*)localRes,
                           res,
                           index);
                    if (BCM_E_NONE != result) {
                        printk("failed to free %p resource %d block %d of"
                               " %d elements at %d: %d (%s)\n",
                               (void*)localRes,
                               res,
                               index,
                               size[res][index],
                               base[res][index],
                               result,
                               _SHR_ERRMSG(result));
                    }
                }
            }
            for (res = 0;
                 (BCM_E_NONE == result) && (res < rescount);
                 res++) {
                for (index = 0;
                     (BCM_E_NONE == result) && (index < TEST_BLOCKS);
                     index++) {
                    result = shr_mres_check(localRes,
                                            res,
                                            size[res][index],
                                            base[res][index]);
                    printk("checked %p:%d:%d     \r",
                           (void*)localRes,
                           res,
                           index);
                    if (BCM_E_NOT_FOUND != result) {
                        printk("unexpected result checking %p resource %d"
                               " block %d at %d (%d elements): %d (%s)\n",
                               (void*)localRes,
                               res,
                               index,
                               base[res][index],
                               size[res][index],
                               result,
                               _SHR_ERRMSG(result));
                        result = BCM_E_INTERNAL;
                    } else {
                        /* correct value */
                        result = BCM_E_NONE;
                    }
                }
            }
            for (res = 0;
                 (BCM_E_NONE == result) && (res < rescount);
                 res++) {
                printk("%p resource %d allocated and freed:\n",
                       (void*)localRes,
                       res);
                for (index = 0;
                     (BCM_E_NONE == result) && (index < TEST_BLOCKS);
                     index++) {
                    printk("  %4d(%4d)", base[res][index], size[res][index]);
                    if (3 == (index & 3)) {
                        printk("\n");
                    }
                }
                if (0 != (index & 3)) {
                    printk("\n");
                }
            }
        } else { /* if (localRes) */
            result = shr_res_get(unit, &rescount, NULL);
            if (BCM_E_NONE == result) {
                if (rescount > SHR_RES_ALLOCATOR_COUNT << 1) {
                    rescount = SHR_RES_ALLOCATOR_COUNT << 1;
                }
            } else {
                printk("unable to retrieve unit %d resource type count:"
                       " %d (%s)\n",
                       unit,
                       result,
                       _SHR_ERRMSG(result));
                rescount = 0;
            }
            for (res = 0;
                 (BCM_E_NONE == result) && (res < rescount);
                 res++) {
                result = shr_res_type_get(unit, res, &pool, NULL, NULL);
                if (BCM_E_NONE == result) {
                    result = shr_res_pool_get(unit,
                                              pool,
                                              &restype,
                                              NULL,
                                              NULL,
                                              NULL,
                                              NULL);
                    if (BCM_E_NONE != result) {
                        printk("unable to retrieve unit %d type %d -> pool %d"
                               " manager: %d (%s)\n",
                               unit,
                               res,
                               pool,
                               result,
                               _SHR_ERRMSG(result));
                    }
                } else {
                    printk("unable to retrieve unit %d type %d pool: %d (%s)\n",
                           unit,
                           res,
                           result,
                           _SHR_ERRMSG(result));
                }
                for (index = 0;
                     (BCM_E_NONE == result) && (index < TEST_BLOCKS);
                     index++) {
                    switch (restype) {
                    case SHR_RES_ALLOCATOR_IDXRES:
                        /* can only do 'WITH_ID' for larger than one */
                        size[res][index] = 1;
                        break;
                    default:
                        size[res][index] = 1 + index;
                        break;
                    }
                    result = shr_res_alloc(unit,
                                           res,
                                           0 /* flags */,
                                           size[res][index],
                                           &(base[res][index]));
                    printk("allocated %d:%d:%d\r", unit, res, index);
                    if (BCM_E_NONE != result) {
                        printk("failed to allocate unit %d resource %d block"
                               " %d of %d elements: %d (%s)\n",
                               unit,
                               res,
                               index,
                               size[res][index],
                               result,
                               _SHR_ERRMSG(result));
                    }
                }
            }
            for (res = 0;
                 (BCM_E_NONE == result) && (res < rescount);
                 res++) {
                for (index = 0;
                     (BCM_E_NONE == result) && (index < TEST_BLOCKS);
                     index++) {
                    result = shr_res_check(unit,
                                           res,
                                           size[res][index],
                                           base[res][index]);
                    printk("checked %d:%d:%d     \r", unit, res, index);
                    if (BCM_E_EXISTS != result) {
                        printk("unexpected result checking unit %d resource %d"
                               " block %d at %d (%d elements): %d (%s)\n",
                               unit,
                               res,
                               index,
                               base[res][index],
                               size[res][index],
                               result,
                               _SHR_ERRMSG(result));
                        result = BCM_E_INTERNAL;
                    } else {
                        /* correct value */
                        result = BCM_E_NONE;
                    }
                }
            }
            for (res = 0;
                 (BCM_E_NONE == result) && (res < rescount);
                 res++) {
                for (index = 0;
                     (BCM_E_NONE == result) && (index < TEST_BLOCKS);
                     index++) {
                    result = shr_res_free(unit,
                                          res,
                                          size[res][index],
                                          base[res][index]);
                    printk("freed %d:%d:%d                \r",
                           unit,
                           res,
                           index);
                    if (BCM_E_NONE != result) {
                        printk("failed to free unit %d resource %d block %d of"
                               " %d elements at %d: %d (%s)\n",
                               unit,
                               res,
                               index,
                               size[res][index],
                               base[res][index],
                               result,
                               _SHR_ERRMSG(result));
                    }
                }
            }
            for (res = 0;
                 (BCM_E_NONE == result) && (res < rescount);
                 res++) {
                for (index = 0;
                     (BCM_E_NONE == result) && (index < TEST_BLOCKS);
                     index++) {
                    result = shr_res_check(unit,
                                           res,
                                           size[res][index],
                                           base[res][index]);
                    printk("checked %d:%d:%d     \r", unit, res, index);
                    if (BCM_E_NOT_FOUND != result) {
                        printk("unexpected result checking unit %d resource %d"
                               " block %d at %d (%d elements): %d (%s)\n",
                               unit,
                               res,
                               index,
                               base[res][index],
                               size[res][index],
                               result,
                               _SHR_ERRMSG(result));
                        result = BCM_E_INTERNAL;
                    } else {
                        /* correct value */
                        result = BCM_E_NONE;
                    }
                }
            }
            for (res = 0;
                 (BCM_E_NONE == result) && (res < rescount);
                 res++) {
                printk("unit %d resource %d allocated and freed:\n",
                       unit,
                       res);
                for (index = 0;
                     (BCM_E_NONE == result) && (index < TEST_BLOCKS);
                     index++) {
                    printk("  %4d(%4d)", base[res][index], size[res][index]);
                    if (3 == (index & 3)) {
                        printk("\n");
                    }
                }
                if (0 != (index & 3)) {
                    printk("\n");
                }
            }
        } /* if (localRes) */
    } else if (!sal_strncasecmp(c, "t1", resTestMax(strlen(c), 2))) {
        if (localRes) {
            result = shr_mres_get(localRes, &rescount, NULL);
            if (BCM_E_NONE == result) {
                if (rescount > SHR_RES_ALLOCATOR_COUNT << 1) {
                    rescount = SHR_RES_ALLOCATOR_COUNT << 1;
                }
            } else {
                printk("unable to retrieve %p resource type count : %d (%s)\n",
                       (void*)localRes,
                       result,
                       _SHR_ERRMSG(result));
                rescount = 0;
            }
            for (res = 0;
                 (BCM_E_NONE == result) && (res < rescount);
                 res++) {
                result = shr_mres_type_get(localRes, res, &pool, NULL, NULL);
                if (BCM_E_NONE == result) {
                    result = shr_mres_pool_get(localRes,
                                               pool,
                                               &restype,
                                               NULL,
                                               NULL,
                                               NULL,
                                               NULL);
                    if (BCM_E_NONE != result) {
                        printk("unable to retrieve %p type %d -> pool %d"
                               " manager: %d (%s)\n",
                               (void*)localRes,
                               res,
                               pool,
                               result,
                               _SHR_ERRMSG(result));
                    }
                } else {
                    printk("unable to retrieve %p type %d pool: %d (%s)\n",
                           (void*)localRes,
                           res,
                           result,
                           _SHR_ERRMSG(result));
                }
                for (index = 0;
                     (BCM_E_NONE == result) && (index < TEST_BLOCKS);
                     index++) {
                    switch (restype) {
                    case SHR_RES_ALLOCATOR_IDXRES:
                        /* can only do 'WITH_ID' for larger than one */
                        size[res][index] = 1;
                        break;
                    default:
                        size[res][index] = TEST_BLOCKS + 1 - index;
                        break;
                    }
                    result = shr_mres_alloc(localRes,
                                            res,
                                            0 /* flags */,
                                            size[res][index],
                                            &(base[res][index]));
                    printk("allocated %p:%d:%d\r",
                           (void*)localRes,
                           res,
                           index);
                    if (BCM_E_NONE != result) {
                        printk("failed to allocate %p resource %d block %d"
                               " of %d elements: %d (%s)\n",
                               (void*)localRes,
                               res,
                               index,
                               size[res][index],
                               result,
                               _SHR_ERRMSG(result));
                    }
                }
            }
            for (res = 0;
                 (BCM_E_NONE == result) && (res < rescount);
                 res++) {
                for (index = 0;
                     (BCM_E_NONE == result) && (index < TEST_BLOCKS);
                     index++) {
                    result = shr_mres_check(localRes,
                                            res,
                                            size[res][index],
                                            base[res][index]);
                    printk("checked %p:%d:%d     \r",
                           (void*)localRes,
                           res,
                           index);
                    if (BCM_E_EXISTS != result) {
                        printk("unexpected result checking %p resource %d"
                               " block %d at %d (%d elements): %d (%s)\n",
                               (void*)localRes,
                               res,
                               index,
                               base[res][index],
                               size[res][index],
                               result,
                               _SHR_ERRMSG(result));
                        result = BCM_E_INTERNAL;
                    } else {
                        /* correct value */
                        result = BCM_E_NONE;
                    }
                }
            }
            for (res = 0;
                 (BCM_E_NONE == result) && (res < rescount);
                 res++) {
                for (index = 0;
                     (BCM_E_NONE == result) && (index < TEST_BLOCKS);
                     index++) {
                    result = shr_mres_free(localRes,
                                           res,
                                           size[res][index],
                                           base[res][index]);
                    printk("freed %p:%d:%d                \r",
                           (void*)localRes,
                           res,
                           index);
                    if (BCM_E_NONE != result) {
                        printk("failed to free %p resource %d block %d of"
                               " %d elements at %d: %d (%s)\n",
                               (void*)localRes,
                               res,
                               index,
                               size[res][index],
                               base[res][index],
                               result,
                               _SHR_ERRMSG(result));
                    }
                }
            }
            for (res = 0;
                 (BCM_E_NONE == result) && (res < rescount);
                 res++) {
                for (index = 0;
                     (BCM_E_NONE == result) && (index < TEST_BLOCKS);
                     index++) {
                    result = shr_mres_check(localRes,
                                            res,
                                            size[res][index],
                                            base[res][index]);
                    printk("checked %p:%d:%d     \r",
                           (void*)localRes,
                           res,
                           index);
                    if (BCM_E_NOT_FOUND != result) {
                        printk("unexpected result checking %p resource %d"
                               " block %d at %d (%d elements): %d (%s)\n",
                               (void*)localRes,
                               res,
                               index,
                               base[res][index],
                               size[res][index],
                               result,
                               _SHR_ERRMSG(result));
                        result = BCM_E_INTERNAL;
                    } else {
                        /* correct value */
                        result = BCM_E_NONE;
                    }
                }
            }
            for (res = 0;
                 (BCM_E_NONE == result) && (res < rescount);
                 res++) {
                printk("%p resource %d allocated and freed:\n",
                       (void*)localRes,
                       res);
                for (index = 0;
                     (BCM_E_NONE == result) && (index < TEST_BLOCKS);
                     index++) {
                    printk("  %4d(%4d)", base[res][index], size[res][index]);
                    if (3 == (index & 3)) {
                        printk("\n");
                    }
                }
                if (0 != (index & 3)) {
                    printk("\n");
                }
            }
        } else { /* if (localRes) */
            result = shr_res_get(unit, &rescount, NULL);
            if (BCM_E_NONE == result) {
                if (rescount > SHR_RES_ALLOCATOR_COUNT << 1) {
                    rescount = SHR_RES_ALLOCATOR_COUNT << 1;
                }
            } else {
                printk("unable to retrieve unit %d resource type count:"
                       " %d (%s)\n",
                       unit,
                       result,
                       _SHR_ERRMSG(result));
                rescount = 0;
            }
            for (res = 0;
                 (BCM_E_NONE == result) && (res < rescount);
                 res++) {
                result = shr_res_type_get(unit, res, &pool, NULL, NULL);
                if (BCM_E_NONE == result) {
                    result = shr_res_pool_get(unit,
                                              pool,
                                              &restype,
                                              NULL,
                                              NULL,
                                              NULL,
                                              NULL);
                    if (BCM_E_NONE != result) {
                        printk("unable to retrieve unit %d type %d -> pool %d"
                               " manager: %d (%s)\n",
                               unit,
                               res,
                               pool,
                               result,
                               _SHR_ERRMSG(result));
                    }
                } else {
                    printk("unable to retrieve unit %d type %d pool: %d (%s)\n",
                           unit,
                           res,
                           result,
                           _SHR_ERRMSG(result));
                }
                for (index = 0;
                     (BCM_E_NONE == result) && (index < TEST_BLOCKS);
                     index++) {
                    switch (restype) {
                    case SHR_RES_ALLOCATOR_IDXRES:
                        /* can only do 'WITH_ID' for larger than one */
                        size[res][index] = 1;
                        break;
                    default:
                        size[res][index] = TEST_BLOCKS + 1 - index;
                        break;
                    }
                    result = shr_res_alloc(unit,
                                           res,
                                           0 /* flags */,
                                           size[res][index],
                                           &(base[res][index]));
                    printk("allocated %d:%d:%d\r", unit, res, index);
                    if (BCM_E_NONE != result) {
                        printk("failed to allocate unit %d resource %d block"
                               " %d of %d elements: %d (%s)\n",
                               unit,
                               res,
                               index,
                               size[res][index],
                               result,
                               _SHR_ERRMSG(result));
                    }
                }
            }
            for (res = 0;
                 (BCM_E_NONE == result) && (res < rescount);
                 res++) {
                for (index = 0;
                     (BCM_E_NONE == result) && (index < TEST_BLOCKS);
                     index++) {
                    result = shr_res_check(unit,
                                           res,
                                           size[res][index],
                                           base[res][index]);
                    printk("checked %d:%d:%d     \r", unit, res, index);
                    if (BCM_E_EXISTS != result) {
                        printk("unexpected result checking unit %d resource %d"
                               " block %d at %d (%d elements): %d (%s)\n",
                               unit,
                               res,
                               index,
                               base[res][index],
                               size[res][index],
                               result,
                               _SHR_ERRMSG(result));
                        result = BCM_E_INTERNAL;
                    } else {
                        /* correct value */
                        result = BCM_E_NONE;
                    }
                }
            }
            for (res = 0;
                 (BCM_E_NONE == result) && (res < rescount);
                 res++) {
                for (index = 0;
                     (BCM_E_NONE == result) && (index < TEST_BLOCKS);
                     index++) {
                    result = shr_res_free(unit,
                                          res,
                                          size[res][index],
                                          base[res][index]);
                    printk("freed %d:%d:%d                \r",
                           unit,
                           res,
                           index);
                    if (BCM_E_NONE != result) {
                        printk("failed to free unit %d resource %d block %d of"
                               " %d elements at %d: %d (%s)\n",
                               unit,
                               res,
                               index,
                               size[res][index],
                               base[res][index],
                               result,
                               _SHR_ERRMSG(result));
                    }
                }
            }
            for (res = 0;
                 (BCM_E_NONE == result) && (res < rescount);
                 res++) {
                for (index = 0;
                     (BCM_E_NONE == result) && (index < TEST_BLOCKS);
                     index++) {
                    result = shr_res_check(unit,
                                           res,
                                           size[res][index],
                                           base[res][index]);
                    printk("checked %d:%d:%d     \r", unit, res, index);
                    if (BCM_E_NOT_FOUND != result) {
                        printk("unexpected result checking unit %d resource %d"
                               " block %d at %d (%d elements): %d (%s)\n",
                               unit,
                               res,
                               index,
                               base[res][index],
                               size[res][index],
                               result,
                               _SHR_ERRMSG(result));
                        result = BCM_E_INTERNAL;
                    } else {
                        /* correct value */
                        result = BCM_E_NONE;
                    }
                }
            }
            for (res = 0;
                 (BCM_E_NONE == result) && (res < rescount);
                 res++) {
                printk("unit %d resource %d allocated and freed:\n",
                       unit,
                       res);
                for (index = 0;
                     (BCM_E_NONE == result) && (index < TEST_BLOCKS);
                     index++) {
                    printk("  %4d(%4d)", base[res][index], size[res][index]);
                    if (3 == (index & 3)) {
                        printk("\n");
                    }
                }
                if (0 != (index & 3)) {
                    printk("\n");
                }
            }
        } /* if (localRes) */
    } else if (!sal_strncasecmp(c, "t2", resTestMax(strlen(c), 2))) {
        if (localRes) {
            result = shr_mres_get(localRes, &rescount, NULL);
            if (BCM_E_NONE == result) {
                if (rescount > SHR_RES_ALLOCATOR_COUNT << 1) {
                    rescount = SHR_RES_ALLOCATOR_COUNT << 1;
                }
            } else {
                printk("unable to retrieve %p resource type count : %d (%s)\n",
                       (void*)localRes,
                       result,
                       _SHR_ERRMSG(result));
                rescount = 0;
            }
            for (res = 0;
                 (BCM_E_NONE == result) && (res < rescount);
                 res++) {
                skip[res] = FALSE;
                result = shr_mres_type_get(localRes, res, &pool, NULL, NULL);
                if (BCM_E_NONE == result) {
                    result = shr_mres_pool_get(localRes,
                                               pool,
                                               &restype,
                                               &(base[res][0]),
                                               &(size[res][0]),
                                               NULL,
                                               NULL);
                    if (BCM_E_NONE == result) {
                        /* got the data properly */
                        if (SHR_RES_ALLOCATOR_MDB == restype) {
                            /* mdb does not support alloc > bank size */
                            result = shr_mres_pool_get(localRes,
                                                       pool,
                                                       NULL,
                                                       NULL,
                                                       NULL,
                                                       &extraGet,
                                                       NULL);
                            if (BCM_E_NONE == result) {
                                sal_memcpy(&mdbExtras,
                                           extraGet,
                                           sizeof(mdbExtras));
                                auxpool = size[res][0] - mdbExtras.bank_size;
                                size[res][0] = mdbExtras.bank_size;
                                for (index = 1;
                                     (index < TEST_BLOCKS) &&
                                     (auxpool > 0);
                                     index++) {
                                    base[res][index] = base[res][index-1] +
                                                       mdbExtras.bank_size;
                                    if (auxpool > mdbExtras.bank_size) {
                                        size[res][index] = mdbExtras.bank_size;
                                        auxpool -= mdbExtras.bank_size;
                                    } else {
                                        size[res][index] = auxpool;
                                        auxpool = 0;
                                    }
                                }
                                while (index < TEST_BLOCKS) {
                                    base[res][index] = 0;
                                    size[res][index] = 0;
                                    index++;
                                }
                                if (auxpool) {
                                    printk("test will not cover all of %p"
                                           " resource %d: not enough workspace"
                                           " to cover all banks; %d elements"
                                           " left\n",
                                           (void*)localRes,
                                           res,
                                           auxpool);
                                    skip[res] = TRUE;
                                }
                            }
                        } else {
                            for (index = 1;
                                 index < TEST_BLOCKS;
                                 index++) {
                                base[res][index] = 0;
                                size[res][index] = 0;
                            }
                        }
                    }
                    if (BCM_E_NONE != result) {
                        printk("unable to retrieve %p type %d -> pool %d"
                               " manager info: %d (%s)\n",
                               (void*)localRes,
                               res,
                               pool,
                               result,
                               _SHR_ERRMSG(result));
                    }
                } else {
                    printk("unable to retrieve %p type %d pool: %d (%s)\n",
                           (void*)localRes,
                           res,
                           result,
                           _SHR_ERRMSG(result));
                }
                used = FALSE;
                for (index = 0;
                     (BCM_E_NONE == result) && (index < res);
                     index++) {
                    result = shr_mres_type_get(localRes,
                                               index,
                                               &auxpool,
                                               NULL,
                                               NULL);
                    if (BCM_E_NONE != result) {
                        printk("unable to retrieve %p type %d pool: %d (%s)\n",
                               (void*)localRes,
                               index,
                               result,
                               _SHR_ERRMSG(result));
                    } else if (auxpool == pool) {
                        used = TRUE;
                        break;
                    }
                }
                if (BCM_E_NONE == result) {
                    for (index = 0;
                         index < (TEST_BLOCKS);
                         index++) {
                        if (0 == size[res][index]) {
                            /* no more blocks to cover here */
                            break;
                        }
                        result = shr_mres_alloc(localRes,
                                                res,
                                                SHR_RES_ALLOC_WITH_ID,
                                                size[res][index],
                                                &(base[res][index]));
                        printk("allocated %p:%d:%d\r",
                               (void*)localRes,
                               res,
                               index);
                        if (!used) {
                            /* BCM_E_NONE is expected result */
                            if (BCM_E_NONE != result) {
                                printk("unable to allocate %p type %d (pool"
                                       " %d) range of %d..%d: %d (%s)\n",
                                       (void*)localRes,
                                       res,
                                       pool,
                                       base[res][index],
                                       base[res][index] + size[res][index] - 1,
                                       result,
                                       _SHR_ERRMSG(result));
                            }
                        } else {
                            /* BCM_E_RESOURCE is expected result */
                            if (BCM_E_NONE == result) {
                                if (!skip[res]) {
                                    printk("should not have been able to"
                                           " allocate %p type %d because type"
                                           " %d should have already consumed"
                                           " all of pool %d\n",
                                           (void*)localRes,
                                           res,
                                           index,
                                           pool);
                                    result = BCM_E_INTERNAL;
                                }
                            } else if (BCM_E_RESOURCE != result) {
                                printk("unexpected error allocating %p type %d"
                                       " (pool %d) entire range %d..%d:"
                                       " %d (%s)\n",
                                       (void*)localRes,
                                       res,
                                       pool,
                                       base[res][index],
                                       base[res][index] + size[res][index] - 1,
                                       result,
                                       _SHR_ERRMSG(result));
                            } else {
                                /* expected in use; don't free it either */
                                result = BCM_E_NONE;
                                size[res][index] = 0;
                            }
                        }
                    }
                }
            }
            for (res = 0;
                 (BCM_E_NONE == result) && (res < rescount);
                 res++) {
                for (index = 0;
                     index < TEST_BLOCKS;
                     index++) {
                    if (size[res][index]) {
                        result = shr_mres_check(localRes,
                                                res,
                                                size[res][index],
                                                base[res][index]);
                        printk("checked %p:%d:%d     \r",
                               (void*)localRes,
                               res,
                               index);
                        if (BCM_E_EXISTS != result) {
                            printk("unexpected result checking %p resource %d"
                                   " block %d at %d (%d elements): %d (%s)\n",
                                   (void*)localRes,
                                   res,
                                   index,
                                   base[res][index],
                                   size[res][index],
                                   result,
                                   _SHR_ERRMSG(result));
                            result = BCM_E_INTERNAL;
                        } else {
                            /* correct value */
                            result = BCM_E_NONE;
                        }
                    }
                }
            }
            for (res = 0;
                 (BCM_E_NONE == result) && (res < rescount);
                 res++) {
                for (index = 0;
                     index < TEST_BLOCKS;
                     index++) {
                    if (size[res][index]) {
                        result = shr_mres_free(localRes,
                                               res,
                                               size[res][index],
                                               base[res][index]);
                        printk("freed %p:%d:%d                \r",
                               (void*)localRes,
                               res,
                               index);
                        if (BCM_E_NONE != result) {
                            printk("failed to free %p resource %d block %d of"
                                   " %d elements at %d: %d (%s)\n",
                                   (void*)localRes,
                                   res,
                                   index,
                                   size[res][index],
                                   base[res][index],
                                   result,
                                   _SHR_ERRMSG(result));
                        }
                    }
                }
            }
            for (res = 0;
                 (BCM_E_NONE == result) && (res < rescount);
                 res++) {
                for (index = 0;
                     index < TEST_BLOCKS;
                     index++) {
                    if (size[res][index]) {
                        result = shr_mres_check(localRes,
                                                res,
                                                size[res][index],
                                                base[res][index]);
                        printk("checked %p:%d:%d     \r",
                               (void*)localRes,
                               res,
                               index);
                        if (BCM_E_NOT_FOUND != result) {
                            printk("unexpected result checking %p resource %d"
                                   " block %d at %d (%d elements): %d (%s)\n",
                                   (void*)localRes,
                                   res,
                                   index,
                                   base[res][index],
                                   size[res][index],
                                   result,
                                   _SHR_ERRMSG(result));
                            result = BCM_E_INTERNAL;
                        } else {
                            /* correct value */
                            result = BCM_E_NONE;
                        }
                    }
                }
            }
            for (res = 0;
                 (BCM_E_NONE == result) && (res < rescount);
                 res++) {
                if (size[res][0]) {
                    printk("%p resource %d allocated and freed\n",
                           (void*)localRes,
                           res);
                } else {
                    printk("%p resource %d overlapped another resource"
                           " (same pool)\n",
                           (void*)localRes,
                           res);
                }
            }
        } else { /* if (localRes) */
            result = shr_res_get(unit, &rescount, NULL);
            if (BCM_E_NONE == result) {
                if (rescount > SHR_RES_ALLOCATOR_COUNT << 1) {
                    rescount = SHR_RES_ALLOCATOR_COUNT << 1;
                }
            } else {
                printk("unable to retrieve unit %d resource type count:"
                       " %d (%s)\n",
                       unit,
                       result,
                       _SHR_ERRMSG(result));
                rescount = 0;
            }
            for (res = 0;
                 (BCM_E_NONE == result) && (res < rescount);
                 res++) {
                skip[res] = FALSE;
                result = shr_res_type_get(unit, res, &pool, NULL, NULL);
                if (BCM_E_NONE == result) {
                    result = shr_res_pool_get(unit,
                                              pool,
                                              &restype,
                                              &(base[res][0]),
                                              &(size[res][0]),
                                              NULL,
                                              NULL);
                    if (BCM_E_NONE == result) {
                        /* got the data properly */
                        if (SHR_RES_ALLOCATOR_MDB == restype) {
                            /* mdb does not support alloc > bank size */
                            result = shr_res_pool_get(unit,
                                                      pool,
                                                      NULL,
                                                      NULL,
                                                      NULL,
                                                      &extraGet,
                                                      NULL);
                            if (BCM_E_NONE == result) {
                                sal_memcpy(&mdbExtras,
                                           extraGet,
                                           sizeof(mdbExtras));
                                auxpool = size[res][0] - mdbExtras.bank_size;
                                size[res][0] = mdbExtras.bank_size;
                                for (index = 1;
                                     (index < TEST_BLOCKS) &&
                                     (auxpool > 0);
                                     index++) {
                                    base[res][index] = base[res][index-1] +
                                                       mdbExtras.bank_size;
                                    if (auxpool > mdbExtras.bank_size) {
                                        size[res][index] = mdbExtras.bank_size;
                                        auxpool -= mdbExtras.bank_size;
                                    } else {
                                        size[res][index] = auxpool;
                                        auxpool = 0;
                                    }
                                }
                                while (index < TEST_BLOCKS) {
                                    base[res][index] = 0;
                                    size[res][index] = 0;
                                    index++;
                                }
                                if (auxpool) {
                                    printk("test will not cover all of unit %d"
                                           " resource %d: not enough workspace"
                                           " to cover all banks; %d elements"
                                           " left\n",
                                           unit,
                                           res,
                                           auxpool);
                                    skip[res] = TRUE;
                                }
                            }
                        } else {
                            for (index = 1;
                                 index < TEST_BLOCKS;
                                 index++) {
                                base[res][index] = 0;
                                size[res][index] = 0;
                            }
                        }
                    }
                    if (BCM_E_NONE != result) {
                        printk("unable to retrieve unit %d type %d -> pool %d"
                               " manager info: %d (%s)\n",
                               unit,
                               res,
                               pool,
                               result,
                               _SHR_ERRMSG(result));
                    }
                } else {
                    printk("unable to retrieve unit %d type %d pool: %d (%s)\n",
                           unit,
                           res,
                           result,
                           _SHR_ERRMSG(result));
                }
                used = FALSE;
                for (index = 0;
                     (BCM_E_NONE == result) && (index < res);
                     index++) {
                    result = shr_res_type_get(unit,
                                              index,
                                              &auxpool,
                                              NULL,
                                              NULL);
                    if (BCM_E_NONE != result) {
                        printk("unable to retrieve unit %d type %d pool:"
                               " %d (%s)\n",
                               unit,
                               index,
                               result,
                               _SHR_ERRMSG(result));
                    } else if (auxpool == pool) {
                        used = TRUE;
                        break;
                    }
                }
                if (BCM_E_NONE == result) {
                    for (index = 0;
                         index < (TEST_BLOCKS);
                         index++) {
                        if (0 == size[res][index]) {
                            /* no more blocks to cover here */
                            break;
                        }
                        result = shr_res_alloc(unit,
                                               res,
                                               SHR_RES_ALLOC_WITH_ID,
                                               size[res][index],
                                               &(base[res][index]));
                        printk("allocated %d:%d:%d\r", unit,res,index);
                        if (!used) {
                            /* BCM_E_NONE is expected result */
                            if (BCM_E_NONE != result) {
                                printk("unable to allocate unit %d type %d"
                                       " (pool %d) range of %d..%d: %d (%s)\n",
                                       unit,
                                       res,
                                       pool,
                                       base[res][index],
                                       base[res][index] + size[res][index] - 1,
                                       result,
                                       _SHR_ERRMSG(result));
                            }
                        } else {
                            /* BCM_E_RESOURCE is expected result */
                            if (BCM_E_NONE == result) {
                                if (!skip[res]) {
                                    printk("should not have been able to"
                                           "allocate unit %d type %d because"
                                           " type %d should have already"
                                           " consumed all of pool %d\n",
                                           unit,
                                           res,
                                           index,
                                           pool);
                                    result = BCM_E_INTERNAL;
                                }
                            } else if (BCM_E_RESOURCE != result) {
                                printk("unexpected error allocating unit %d"
                                       " type %d (pool %d) entire range %d..%d:"
                                       " %d (%s)\n",
                                       unit,
                                       res,
                                       pool,
                                       base[res][index],
                                       base[res][index] + size[res][index] - 1,
                                       result,
                                       _SHR_ERRMSG(result));
                            } else {
                                /* expected in use; don't free it either */
                                result = BCM_E_NONE;
                                size[res][index] = 0;
                            }
                        }
                    }
                }
            }
            for (res = 0;
                 (BCM_E_NONE == result) && (res < rescount);
                 res++) {
                for (index = 0;
                     index < TEST_BLOCKS;
                     index++) {
                    if (size[res][index]) {
                        result = shr_res_check(unit,
                                               res,
                                               size[res][index],
                                               base[res][index]);
                        printk("checked %d:%d:%d     \r", unit, res, index);
                        if (BCM_E_EXISTS != result) {
                            printk("unexpected result checking unit %d"
                                   " resource %d block %d at %d (%d elements):"
                                   " %d (%s)\n",
                                   unit,
                                   res,
                                   index,
                                   base[res][index],
                                   size[res][index],
                                   result,
                                   _SHR_ERRMSG(result));
                            result = BCM_E_INTERNAL;
                        } else {
                            /* correct value */
                            result = BCM_E_NONE;
                        }
                    }
                }
            }
            for (res = 0;
                 (BCM_E_NONE == result) && (res < rescount);
                 res++) {
                for (index = 0;
                     index < TEST_BLOCKS;
                     index++) {
                    if (size[res][index]) {
                        result = shr_res_free(unit,
                                              res,
                                              size[res][index],
                                              base[res][index]);
                        printk("freed %d:%d:%d                \r",
                               unit,
                               res,
                               index);
                        if (BCM_E_NONE != result) {
                            printk("failed to free unit %d resource %d block"
                                   " %d of %d elements at %d: %d (%s)\n",
                                   unit,
                                   res,
                                   index,
                                   size[res][index],
                                   base[res][index],
                                   result,
                                   _SHR_ERRMSG(result));
                        }
                    }
                }
            }
            for (res = 0;
                 (BCM_E_NONE == result) && (res < rescount);
                 res++) {
                for (index = 0;
                     index < TEST_BLOCKS;
                     index++) {
                    if (size[res][index]) {
                        result = shr_res_check(unit,
                                               res,
                                               size[res][index],
                                               base[res][index]);
                        printk("checked %d:%d:%d     \r", unit, res, index);
                        if (BCM_E_NOT_FOUND != result) {
                            printk("unexpected result checking unit %d"
                                   " resource %d block %d at %d (%d elements):"
                                   " %d (%s)\n",
                                   unit,
                                   res,
                                   index,
                                   base[res][index],
                                   size[res][index],
                                   result,
                                   _SHR_ERRMSG(result));
                            result = BCM_E_INTERNAL;
                        } else {
                            /* correct value */
                            result = BCM_E_NONE;
                        }
                    }
                }
            }
            for (res = 0;
                 (BCM_E_NONE == result) && (res < rescount);
                 res++) {
                if (size[res][0]) {
                    printk("unit %d resource %d allocated and freed\n",
                           unit,
                           res);
                } else {
                    printk("unit %d resource %d overlapped another resource"
                           " (same pool)\n",
                           unit,
                           res);
                }
            }
        } /* if (localRes) */
    } else if (!sal_strncasecmp(c, "t3", resTestMax(strlen(c), 2))) {
        if (localRes) {
            result = shr_mres_get(localRes, &rescount, NULL);
            if (BCM_E_NONE == result) {
                if (rescount > SHR_RES_ALLOCATOR_COUNT << 1) {
                    rescount = SHR_RES_ALLOCATOR_COUNT << 1;
                }
            } else {
                printk("unable to retrieve %p resource type count : %d (%s)\n",
                       (void*)localRes,
                       result,
                       _SHR_ERRMSG(result));
                rescount = 0;
            }
            for (res = 0;
                 (BCM_E_NONE == result) && (res < rescount);
                 res++) {
                skip[res] = FALSE;
                result = shr_mres_type_get(localRes, res, &pool, NULL, NULL);
                if (BCM_E_NONE == result) {
                    result = shr_mres_pool_get(localRes,
                                               pool,
                                               &restype,
                                               &(base[res][0]),
                                               &(size[res][0]),
                                               NULL,
                                               NULL);
                    if (BCM_E_NONE == result) {
                        /* got the data properly */
                        if (SHR_RES_ALLOCATOR_MDB == restype) {
                            /* mdb does not support alloc > bank size */
                            result = shr_mres_pool_get(localRes,
                                                       pool,
                                                       NULL,
                                                       NULL,
                                                       NULL,
                                                       &extraGet,
                                                       NULL);
                            if (BCM_E_NONE == result) {
                                sal_memcpy(&mdbExtras,
                                           extraGet,
                                           sizeof(mdbExtras));
                                auxpool = size[res][0] - mdbExtras.bank_size;
                                size[res][0] = mdbExtras.bank_size;
                                for (index = 1;
                                     (index < TEST_BLOCKS) &&
                                     (auxpool > 0);
                                     index++) {
                                    base[res][index] = base[res][index-1] +
                                                       mdbExtras.bank_size;
                                    if (auxpool > mdbExtras.bank_size) {
                                        size[res][index] = mdbExtras.bank_size;
                                        auxpool -= mdbExtras.bank_size;
                                    } else {
                                        size[res][index] = auxpool;
                                        auxpool = 0;
                                    }
                                }
                                while (index < TEST_BLOCKS) {
                                    base[res][index] = 0;
                                    size[res][index] = 0;
                                    index++;
                                }
                                if (auxpool) {
                                    printk("test will not cover all of %p"
                                           " resource %d: not enough workspace"
                                           " to cover all banks; %d elements"
                                           " left\n",
                                           (void*)localRes,
                                           res,
                                           auxpool);
                                    skip[res] = TRUE;
                                }
                            }
                        } else {
                            for (index = 1;
                                 index < TEST_BLOCKS;
                                 index++) {
                                base[res][index] = 0;
                                size[res][index] = 0;
                            }
                        }
                    }
                    if (BCM_E_NONE != result) {
                        printk("unable to retrieve %p type %d -> pool %d"
                               " manager info: %d (%s)\n",
                               (void*)localRes,
                               res,
                               pool,
                               result,
                               _SHR_ERRMSG(result));
                    }
                } else {
                    printk("unable to retrieve %p type %d pool: %d (%s)\n",
                           (void*)localRes,
                           res,
                           result,
                           _SHR_ERRMSG(result));
                }
                used = FALSE;
                for (index = 0;
                     (BCM_E_NONE == result) && (index < res);
                     index++) {
                    result = shr_mres_type_get(localRes,
                                               index,
                                               &auxpool,
                                               NULL,
                                               NULL);
                    if (BCM_E_NONE != result) {
                        printk("unable to retrieve %p type %d pool: %d (%s)\n",
                               (void*)localRes,
                               index,
                               result,
                               _SHR_ERRMSG(result));
                    } else if (auxpool == pool) {
                        used = TRUE;
                        break;
                    }
                }
                if (BCM_E_NONE == result) {
                    for (index = 0;
                         index < (TEST_BLOCKS);
                         index++) {
                        if (0 == size[res][index]) {
                            /* no more blocks to cover here */
                            break;
                        }
                        result = shr_mres_alloc(localRes,
                                                res,
                                                SHR_RES_ALLOC_WITH_ID,
                                                size[res][index],
                                                &(base[res][index]));
                        printk("allocated %p:%d:%d\r",
                               (void*)localRes,
                               res,
                               index);
                        if (!used) {
                            /* BCM_E_NONE is expected result */
                            if (BCM_E_NONE != result) {
                                printk("unable to allocate %p type %d (pool"
                                       " %d) range of %d..%d: %d (%s)\n",
                                       (void*)localRes,
                                       res,
                                       pool,
                                       base[res][index],
                                       base[res][index] + size[res][index] - 1,
                                       result,
                                       _SHR_ERRMSG(result));
                            }
                        } else {
                            /* BCM_E_RESOURCE is expected result */
                            if (BCM_E_NONE == result) {
                                if (!skip[res]) {
                                    printk("should not have been able to"
                                           "allocate %p type %d because type"
                                           " %d should have already consumed"
                                           " all of pool %d\n",
                                           (void*)localRes,
                                           res,
                                           index,
                                           pool);
                                    result = BCM_E_INTERNAL;
                                }
                            } else if (BCM_E_RESOURCE != result) {
                                printk("unexpected error allocating %p type %d"
                                       " (pool %d) entire range %d..%d:"
                                       " %d (%s)\n",
                                       (void*)localRes,
                                       res,
                                       pool,
                                       base[res][index],
                                       base[res][index] + size[res][index] - 1,
                                       result,
                                       _SHR_ERRMSG(result));
                            } else {
                                /* expected in use; don't free it either */
                                result = BCM_E_NONE;
                                size[res][index] = 0;
                            }
                        }
                    }
                }
            }
            for (res = 0;
                 (BCM_E_NONE == result) && (res < rescount);
                 res++) {
                result = shr_mres_alloc(localRes,
                                        res,
                                        0 /* flags */,
                                        1 /* count */,
                                        &auxpool);
                if (BCM_E_NONE == result) {
                    if (!skip[res]) {
                        printk("%p res %d was able to allocate another"
                               " element when the entire pool should have been"
                               " already in use\n",
                               (void*)localRes,
                               res);
                    }
                    result = shr_mres_free(localRes, res, 1, auxpool);
                    if (BCM_E_NONE != result) {
                        printk("%p res %d was unable to return element"
                               " %d to free: %d (%s)\n",
                               (void*)localRes,
                               res,
                               auxpool,
                               result,
                               _SHR_ERRMSG(result));
                    }
                    result = BCM_E_INTERNAL;
                } else if (BCM_E_RESOURCE != result) {
                    printk("%p res %d encountered an unexpected error trying"
                           " to allocate another element when it should have"
                           " been full: %d (%s)\n",
                           (void*)localRes,
                           res,
                           result,
                           _SHR_ERRMSG(result));
                } else {
                    /* this is okay, so keep going */
                    result = BCM_E_NONE;
                }
            }
            for (res = 0;
                 (BCM_E_NONE == result) && (res < rescount);
                 res++) {
                for (index = 0;
                     index < TEST_BLOCKS;
                     index++) {
                    if (size[res][index]) {
                        result = shr_mres_free(localRes,
                                               res,
                                               size[res][index],
                                               base[res][index]);
                        printk("freed %p:%d:%d                \r",
                               (void*)localRes,
                               res,
                               index);
                        if (BCM_E_NONE != result) {
                            printk("failed to free %p resource %d block %d of"
                                   " %d elements at %d: %d (%s)\n",
                                   (void*)localRes,
                                   res,
                                   index,
                                   size[res][index],
                                   base[res][index],
                                   result,
                                   _SHR_ERRMSG(result));
                        }
                    }
                }
            }
            for (res = 0;
                 (BCM_E_NONE == result) && (res < rescount);
                 res++) {
                result = shr_mres_alloc(localRes,
                                        res,
                                        0 /* flags */,
                                        1 /* count */,
                                        &auxpool);
                if (BCM_E_NONE != result) {
                    printk("%p res %d was unable to allocate an element"
                           " after being filled and them freed: %d (%s)\n",
                           (void*)localRes,
                           res,
                           result,
                           _SHR_ERRMSG(result));
                } else {
                    result = shr_mres_free(localRes, res, 1, auxpool);
                    if (BCM_E_NONE != result) {
                        printk("%p res %d was unable to return element"
                               " %d to free: %d (%s)\n",
                               (void*)localRes,
                               res,
                               auxpool,
                               result,
                               _SHR_ERRMSG(result));
                    }
                }
            }
            for (res = 0;
                 (BCM_E_NONE == result) && (res < rescount);
                 res++) {
                if (size[res][0]) {
                    printk("%p resource %d allocated and freed\n",
                           (void*)localRes,
                           res);
                } else {
                    printk("%p resource %d overlapped another resource"
                           " (same pool)\n",
                           (void*)localRes,
                           res);
                }
            }
        } else { /* if (localRes) */
            result = shr_res_get(unit, &rescount, NULL);
            if (BCM_E_NONE == result) {
                if (rescount > SHR_RES_ALLOCATOR_COUNT << 1) {
                    rescount = SHR_RES_ALLOCATOR_COUNT << 1;
                }
            } else {
                printk("unable to retrieve unit %d resource type count:"
                       " %d (%s)\n",
                       unit,
                       result,
                       _SHR_ERRMSG(result));
                rescount = 0;
            }
            for (res = 0;
                 (BCM_E_NONE == result) && (res < rescount);
                 res++) {
                skip[res] = FALSE;
                result = shr_res_type_get(unit, res, &pool, NULL, NULL);
                if (BCM_E_NONE == result) {
                    result = shr_res_pool_get(unit,
                                              pool,
                                              &restype,
                                              &(base[res][0]),
                                              &(size[res][0]),
                                              NULL,
                                              NULL);
                    if (BCM_E_NONE == result) {
                        /* got the data properly */
                        if (SHR_RES_ALLOCATOR_MDB == restype) {
                            /* mdb does not support alloc > bank size */
                            result = shr_res_pool_get(unit,
                                                      pool,
                                                      NULL,
                                                      NULL,
                                                      NULL,
                                                      &extraGet,
                                                      NULL);
                            if (BCM_E_NONE == result) {
                                sal_memcpy(&mdbExtras,
                                           extraGet,
                                           sizeof(mdbExtras));
                                auxpool = size[res][0] - mdbExtras.bank_size;
                                size[res][0] = mdbExtras.bank_size;
                                for (index = 1;
                                     (index < TEST_BLOCKS) &&
                                     (auxpool > 0);
                                     index++) {
                                    base[res][index] = base[res][index-1] +
                                                       mdbExtras.bank_size;
                                    if (auxpool > mdbExtras.bank_size) {
                                        size[res][index] = mdbExtras.bank_size;
                                        auxpool -= mdbExtras.bank_size;
                                    } else {
                                        size[res][index] = auxpool;
                                        auxpool = 0;
                                    }
                                }
                                while (index < TEST_BLOCKS) {
                                    base[res][index] = 0;
                                    size[res][index] = 0;
                                    index++;
                                }
                                if (auxpool) {
                                    printk("test will not cover all of unit %d"
                                           " resource %d: not enough workspace"
                                           " to cover all banks; %d elements"
                                           " left\n",
                                           unit,
                                           res,
                                           auxpool);
                                    skip[res] = TRUE;
                                }
                            }
                        } else {
                            for (index = 1;
                                 index < TEST_BLOCKS;
                                 index++) {
                                base[res][index] = 0;
                                size[res][index] = 0;
                            }
                        }
                    }
                    if (BCM_E_NONE != result) {
                        printk("unable to retrieve unit %d type %d -> pool %d"
                               " manager info: %d (%s)\n",
                               unit,
                               res,
                               pool,
                               result,
                               _SHR_ERRMSG(result));
                    }
                } else {
                    printk("unable to retrieve unit %d type %d pool: %d (%s)\n",
                           unit,
                           res,
                           result,
                           _SHR_ERRMSG(result));
                }
                used = FALSE;
                for (index = 0;
                     (BCM_E_NONE == result) && (index < res);
                     index++) {
                    result = shr_res_type_get(unit,
                                              index,
                                              &auxpool,
                                              NULL,
                                              NULL);
                    if (BCM_E_NONE != result) {
                        printk("unable to retrieve unit %d type %d pool:"
                               " %d (%s)\n",
                               unit,
                               index,
                               result,
                               _SHR_ERRMSG(result));
                    } else if (auxpool == pool) {
                        used = TRUE;
                        break;
                    }
                }
                if (BCM_E_NONE == result) {
                    for (index = 0;
                         index < (TEST_BLOCKS);
                         index++) {
                        if (0 == size[res][index]) {
                            /* no more blocks to cover here */
                            break;
                        }
                        result = shr_res_alloc(unit,
                                               res,
                                               SHR_RES_ALLOC_WITH_ID,
                                               size[res][index],
                                               &(base[res][index]));
                        printk("allocated %d:%d:%d\r", unit, res, index);
                        if (!used) {
                            /* BCM_E_NONE is expected result */
                            if (BCM_E_NONE != result) {
                                printk("unable to allocate unit %d type %d"
                                       " (pool %d) range of %d..%d: %d (%s)\n",
                                       unit,
                                       res,
                                       pool,
                                       base[res][index],
                                       base[res][index] + size[res][index] - 1,
                                       result,
                                       _SHR_ERRMSG(result));
                            }
                        } else {
                            /* BCM_E_RESOURCE is expected result */
                            if (BCM_E_NONE == result) {
                                if (!skip[res]) {
                                    printk("should not have been able to"
                                           " allocate unit %d type %d because"
                                           " type %d should have already"
                                           " consumed all of pool %d\n",
                                           unit,
                                           res,
                                           index,
                                           pool);
                                    result = BCM_E_INTERNAL;
                                }
                            } else if (BCM_E_RESOURCE != result) {
                                printk("unexpected error allocating unit %d"
                                       " type %d (pool %d) entire range %d..%d:"
                                       " %d (%s)\n",
                                       unit,
                                       res,
                                       pool,
                                       base[res][index],
                                       base[res][index] + size[res][index] - 1,
                                       result,
                                       _SHR_ERRMSG(result));
                            } else {
                                /* expected in use; don't free it either */
                                result = BCM_E_NONE;
                                size[res][index] = 0;
                            }
                        }
                    }
                }
            }
            for (res = 0;
                 (BCM_E_NONE == result) && (res < rescount);
                 res++) {
                result = shr_res_alloc(unit, res, 0 /* flags */, 1, &auxpool);
                if (BCM_E_NONE == result) {
                    if (!skip[res]) {
                        printk("unit %d res %d was able to allocate another"
                               " element when the entire pool should have been"
                               " already in use\n",
                               unit,
                               res);
                    }
                    result = shr_res_free(unit, res, 1, auxpool);
                    if (BCM_E_NONE != result) {
                        printk("unit %d res %d was unable to return element"
                               " %d to free: %d (%s)\n",
                               unit,
                               res,
                               auxpool,
                               result,
                               _SHR_ERRMSG(result));
                    }
                    result = BCM_E_INTERNAL;
                } else if (BCM_E_RESOURCE != result) {
                    printk("unit %d res %d encountered an unexpected error"
                           " trying to allocate another element when it should"
                           " have been full: %d (%s)\n",
                           unit,
                           res,
                           result,
                           _SHR_ERRMSG(result));
                } else {
                    /* this is okay, so keep going */
                    result = BCM_E_NONE;
                }
            }
            for (res = 0;
                 (BCM_E_NONE == result) && (res < rescount);
                 res++) {
                for (index = 0;
                     index < TEST_BLOCKS;
                     index++) {
                    if (size[res][index]) {
                        result = shr_res_free(unit,
                                              res,
                                              size[res][index],
                                              base[res][index]);
                        printk("freed %d:%d:%d                \r",
                               unit,
                               res,
                               index);
                        if (BCM_E_NONE != result) {
                            printk("failed to free unit %d resource %d block"
                                   " %d of %d elements at %d: %d (%s)\n",
                                   unit,
                                   res,
                                   index,
                                   size[res][index],
                                   base[res][index],
                                   result,
                                   _SHR_ERRMSG(result));
                        }
                    }
                }
            }
            for (res = 0;
                 (BCM_E_NONE == result) && (res < rescount);
                 res++) {
                result = shr_res_alloc(unit, res, 0 /* flags */, 1, &auxpool);
                if (BCM_E_NONE != result) {
                    printk("unit %d res %d was unable to allocate an element"
                           " after being filled and them freed: %d (%s)\n",
                           unit,
                           res,
                           result,
                           _SHR_ERRMSG(result));
                } else {
                    result = shr_res_free(unit, res, 1, auxpool);
                    if (BCM_E_NONE != result) {
                        printk("unit %d res %d was unable to return element"
                               " %d to free: %d (%s)\n",
                               unit,
                               res,
                               auxpool,
                               result,
                               _SHR_ERRMSG(result));
                    }
                }
            }
            for (res = 0;
                 (BCM_E_NONE == result) && (res < rescount);
                 res++) {
                if (size[res][0]) {
                    printk("unit %d resource %d allocated and freed\n",
                           unit,
                           res);
                } else {
                    printk("unit %d resource %d overlapped another resource"
                           " (same pool)\n",
                           unit,
                           res);
                }
            }
        } /* if (localRes) */
    } else if (!sal_strncasecmp(c, "t4", resTestMax(strlen(c), 2))) {
        if (localRes) {
            result = shr_mres_get(localRes, &rescount, NULL);
            if (BCM_E_NONE == result) {
                if (rescount > SHR_RES_ALLOCATOR_COUNT << 1) {
                    rescount = SHR_RES_ALLOCATOR_COUNT << 1;
                }
            } else {
                printk("unable to retrieve %p resource type count : %d (%s)\n",
                       (void*)localRes,
                       result,
                       _SHR_ERRMSG(result));
                rescount = 0;
            }
            for (res = 0;
                 (BCM_E_NONE == result) && (res < rescount);
                 res++) {
                result = shr_mres_type_get(localRes, res, &pool, NULL, NULL);
                if (BCM_E_NONE == result) {
                    result = shr_mres_pool_get(localRes,
                                               pool,
                                               &restype,
                                               &(base[res][0]),
                                               &(size[res][0]),
                                               NULL,
                                               NULL);
                }
                if (BCM_E_NONE == result) {
                    result = shr_mres_free(localRes, res, 1, base[res][0]);
                    if (BCM_E_NONE == result) {
                        printk("%p resource %d freed element %d size 1 when"
                               " such a block should not exist\n",
                               (void*)localRes,
                               res,
                               base[res][0]);
                        result = BCM_E_INTERNAL;
                    } else if (BCM_E_NOT_FOUND != result) {
                        printk("%p resource %d trying to free element %d size"
                               " 1 encountered an unexpected error: %d (%s)\n",
                               (void*)localRes,
                               res,
                               base[res][0],
                               result,
                               _SHR_ERRMSG(result));
                    } else {
                        result = BCM_E_NONE;
                    }
                }
            }
            for (res = 0;
                 (BCM_E_NONE == result) && (res < rescount);
                 res++) {
                printk("was properly unable to free an unallocated block in"
                       " %p resource %d\n",
                       (void*)localRes,
                       res);
            }
        } else { /* if (localRes) */
            result = shr_res_get(unit, &rescount, NULL);
            if (BCM_E_NONE == result) {
                if (rescount > SHR_RES_ALLOCATOR_COUNT << 1) {
                    rescount = SHR_RES_ALLOCATOR_COUNT << 1;
                }
            } else {
                printk("unable to retrieve unit %d resource type count:"
                       " %d (%s)\n",
                       unit,
                       result,
                       _SHR_ERRMSG(result));
                rescount = 0;
            }
            for (res = 0;
                 (BCM_E_NONE == result) && (res < rescount);
                 res++) {
                result = shr_res_type_get(unit, res, &pool, NULL, NULL);
                if (BCM_E_NONE == result) {
                    result = shr_res_pool_get(unit,
                                              pool,
                                              &restype,
                                              &(base[res][0]),
                                              &(size[res][0]),
                                              NULL,
                                              NULL);
                }
                if (BCM_E_NONE == result) {
                    result = shr_res_free(unit, res, 1, base[res][0]);
                    if (BCM_E_NONE == result) {
                        printk("unit %d resource %d freed element %d size 1"
                               " when such a block should not exist\n",
                               unit,
                               res,
                               base[res][0]);
                        result = BCM_E_INTERNAL;
                    } else if (BCM_E_NOT_FOUND != result) {
                        printk("unit %d resource %d trying to free element %d"
                               " size 1 encountered an unexpected error:"
                               " %d (%s)\n",
                               unit,
                               res,
                               base[res][0],
                               result,
                               _SHR_ERRMSG(result));
                    } else {
                        result = BCM_E_NONE;
                    }
                }
            }
            for (res = 0;
                 (BCM_E_NONE == result) && (res < rescount);
                 res++) {
                printk("was properly unable to free an unallocated block in"
                       " unit %d resource %d\n",
                       unit,
                       res);
            }
        } /* if (localRes) */
    } else if (!sal_strncasecmp(c, "t5", resTestMax(strlen(c), 2))) {
        if (localRes) {
            result = shr_mres_get(localRes, &rescount, NULL);
            if (BCM_E_NONE == result) {
                if (rescount > SHR_RES_ALLOCATOR_COUNT << 1) {
                    rescount = SHR_RES_ALLOCATOR_COUNT << 1;
                }
            } else {
                printk("unable to retrieve %p resource type count : %d (%s)\n",
                       (void*)localRes,
                       result,
                       _SHR_ERRMSG(result));
                rescount = 0;
            }
            res = 0;
            do {
                result = shr_mres_type_get(localRes, res, &pool, NULL, NULL);
                if (BCM_E_NONE == result) {
                    result = shr_mres_pool_get(localRes,
                                               pool,
                                               &restype,
                                               &(base[res][0]),
                                               &(size[res][0]),
                                               NULL,
                                               NULL);
                    if ((BCM_E_NONE == result)  &&
                        ((size[res][0] < 512) ||
                         (SHR_RES_ALLOCATOR_BITMAP != restype))) {
                        res++;
                    }
                } else {
                    printk("unable to retrieve %p res type %d info: %d (%s)\n",
                           (void*)localRes,
                           res,
                           result,
                           _SHR_ERRMSG(result));
                    xresult = result;
                }
            } while ((res < rescount) &&
                     (BCM_E_NONE == result) &&
                     ((size[res][0] < 512) ||
                      (SHR_RES_ALLOCATOR_BITMAP != restype)));
            /* now res is a bitmap type and we know its base and size */
            if (res < rescount) {
                /* allocate some aligned to base of pool, no offset */
                xresult = BCM_E_NONE;
                for (index = 1;
                     (index < 17) && (BCM_E_NONE == result);
                     index++) {
                    size[res][index] = 7;
                    result = shr_mres_alloc_align(localRes,
                                                  res,
                                                  0 /* flags */,
                                                  index /* align */,
                                                  0 /* offset */,
                                                  size[res][index],
                                                  &(base[res][index]));
                    if (BCM_E_NONE == result) {
                        printk("allocated %2d elems at %5d, align %2d, offset"
                               " %2d; offset = %2d: ",
                               size[res][index],
                               base[res][index],
                               index,
                               0,
                               (base[res][index] - base[res][0]) % index);
                        if (0 == ((base[res][index] - base[res][0]) % index)) {
                            printk("PASS\n");
                        } else {
                            printk("FAIL\n");
                            xresult = BCM_E_FAIL;
                        }
                    } else {
                        printk("failed to alloc %d elems, align %d, offset %d:"
                               " %d %s)\n",
                               size[res][index],
                               index,
                               0,
                               result,
                               _SHR_ERRMSG(result));
                        xresult = result;
                    }
                }
                while (index > 1) {
                    index--;
                    result = shr_mres_free(localRes,
                                           res,
                                           size[res][index],
                                           base[res][index]);
                    if (BCM_E_NONE != result) {
                        printk("unable to free %d elems at %d: %d (%s)\n",
                               size[res][index],
                               base[res][index],
                               result,
                               _SHR_ERRMSG(result));
                        xresult = result;
                    }
                }
                for (index = 1;
                     (index < 17) && (BCM_E_NONE == result);
                     index++) {
                    size[res][index] = 7;
                    result = shr_mres_alloc_align(localRes,
                                                  res,
                                                  0 /* flags */,
                                                  16 /* align */,
                                                  index - 1 /* offset */,
                                                  size[res][index],
                                                  &(base[res][index]));
                    if (BCM_E_NONE == result) {
                        printk("allocated %2d elems at %5d, align %2d, offset"
                               " %2d; offset = %2d: ",
                               size[res][index],
                               base[res][index],
                               16,
                               index - 1,
                               (base[res][index] - base[res][0]) % (16));
                        if ((index - 1) == ((base[res][index] -
                                             base[res][0]) % 16)) {
                            printk("PASS\n");
                        } else {
                            printk("FAIL\n");
                            xresult = BCM_E_FAIL;
                        }
                    } else {
                        printk("failed to alloc %d elems, align %d, offset %d:"
                               " %d %s)\n",
                               size[res][index],
                               index - 1,
                               0,
                               result,
                               _SHR_ERRMSG(result));
                        xresult = result;
                    }
                }
                while (index > 1) {
                    index--;
                    result = shr_mres_free(localRes,
                                           res,
                                           size[res][index],
                                           base[res][index]);
                    if (BCM_E_NONE != result) {
                        printk("unable to free %d elems at %d: %d (%s)\n",
                               size[res][index],
                               base[res][index],
                               result,
                               _SHR_ERRMSG(result));
                        xresult = result;
                    }
                }
                for (index = 1;
                     (index < 17) && (BCM_E_NONE == result);
                     index++) {
                    size[res][index] = 7;
                    result = shr_mres_alloc_align(localRes,
                                                  res,
                                                  SHR_RES_ALLOC_ALIGN_ZERO,
                                                  index /* align */,
                                                  0 /* offset */,
                                                  size[res][index],
                                                  &(base[res][index]));
                    if (BCM_E_NONE == result) {
                        printk("allocated %2d elems at %5d, align %2d, offset"
                               " %2d; offset = %2d: ",
                               size[res][index],
                               base[res][index],
                               index,
                               0,
                               base[res][index] % index);
                        if (0 == (base[res][index] % index)) {
                            printk("PASS\n");
                        } else {
                            printk("FAIL\n");
                            xresult = BCM_E_FAIL;
                        }
                    } else {
                        printk("failed to alloc %d elems, align %d, offset %d:"
                               " %d %s)\n",
                               size[res][index],
                               index,
                               0,
                               result,
                               _SHR_ERRMSG(result));
                        xresult = result;
                    }
                }
                while (index > 1) {
                    index--;
                    result = shr_mres_free(localRes,
                                           res,
                                           size[res][index],
                                           base[res][index]);
                    if (BCM_E_NONE != result) {
                        printk("unable to free %d elems at %d: %d (%s)\n",
                               size[res][index],
                               base[res][index],
                               result,
                               _SHR_ERRMSG(result));
                        xresult = result;
                    }
                }
                for (index = 1;
                     (index < 17) && (BCM_E_NONE == result);
                     index++) {
                    size[res][index] = 7;
                    result = shr_mres_alloc_align(localRes,
                                                  res,
                                                  SHR_RES_ALLOC_ALIGN_ZERO,
                                                  16 /* align */,
                                                  index - 1 /* offset */,
                                                  size[res][index],
                                                  &(base[res][index]));
                    if (BCM_E_NONE == result) {
                        printk("allocated %2d elems at %5d, align %2d, offset"
                               " %2d; offset = %2d: ",
                               size[res][index],
                               base[res][index],
                               16,
                               index - 1,
                               base[res][index] % 16);
                        if ((index - 1) == (base[res][index] % 16)) {
                            printk("PASS\n");
                        } else {
                            printk("FAIL\n");
                            xresult = BCM_E_FAIL;
                        }
                    } else {
                        printk("failed to alloc %d elems, align %d, offset %d:"
                               " %d %s)\n",
                               size[res][index],
                               index - 1,
                               0,
                               result,
                               _SHR_ERRMSG(result));
                        xresult = result;
                    }
                }
                while (index > 1) {
                    index--;
                    result = shr_mres_free(localRes,
                                           res,
                                           size[res][index],
                                           base[res][index]);
                    if (BCM_E_NONE != result) {
                        printk("unable to free %d elems at %d: %d (%s)\n",
                               size[res][index],
                               base[res][index],
                               result,
                               _SHR_ERRMSG(result));
                        xresult = result;
                    }
                }
                for (index = 1;
                     (index < 17) && (BCM_E_NONE == result);
                     index++) {
                    size[res][index] = 7;
                    result = shr_mres_alloc_align(localRes,
                                                  res,
                                                  SHR_RES_ALLOC_ALIGN_ZERO,
                                                  17 /* align */,
                                                  index - 1 /* offset */,
                                                  size[res][index],
                                                  &(base[res][index]));
                    if (BCM_E_NONE == result) {
                        printk("allocated %2d elems at %5d, align %2d, offset"
                               " %2d; offset = %2d: ",
                               size[res][index],
                               base[res][index],
                               17,
                               index - 1,
                               base[res][index] % 17);
                        if ((index - 1) == (base[res][index] % 17)) {
                            printk("PASS\n");
                        } else {
                            printk("FAIL\n");
                            xresult = BCM_E_FAIL;
                        }
                    } else {
                        printk("failed to alloc %d elems, align %d, offset %d:"
                               " %d %s)\n",
                               size[res][index],
                               index - 1,
                               0,
                               result,
                               _SHR_ERRMSG(result));
                        xresult = result;
                    }
                }
                while (index > 1) {
                    index--;
                    result = shr_mres_free(localRes,
                                           res,
                                           size[res][index],
                                           base[res][index]);
                    if (BCM_E_NONE != result) {
                        printk("unable to free %d elems at %d: %d (%s)\n",
                               size[res][index],
                               base[res][index],
                               result,
                               _SHR_ERRMSG(result));
                        xresult = result;
                    }
                }
                result = xresult;
            } else {
                printk("did not find a sufficiently large bitmap based pool\n");
                result = BCM_E_INTERNAL;
            }
        } else { /* if (localRes) */
            result = shr_res_get(unit, &rescount, NULL);
            if (BCM_E_NONE == result) {
                if (rescount > SHR_RES_ALLOCATOR_COUNT << 1) {
                    rescount = SHR_RES_ALLOCATOR_COUNT << 1;
                }
            } else {
                printk("unable to retrieve unit %d resource type count:"
                       " %d (%s)\n",
                       unit,
                       result,
                       _SHR_ERRMSG(result));
                rescount = 0;
            }
            res = 0;
            do {
                result = shr_res_type_get(unit, res, &pool, NULL, NULL);
                if (BCM_E_NONE == result) {
                    result = shr_res_pool_get(unit,
                                              pool,
                                              &restype,
                                              &(base[res][0]),
                                              &(size[res][0]),
                                              NULL,
                                              NULL);
                    if ((BCM_E_NONE == result)  &&
                        ((size[res][0] < 512) ||
                         (SHR_RES_ALLOCATOR_BITMAP != restype))) {
                        res++;
                    }
                } else {
                    printk("unable to retrieve unit %d res type %d info:"
                           " %d (%s)\n",
                           unit,
                           res,
                           result,
                           _SHR_ERRMSG(result));
                    xresult = result;
                }
            } while ((res < rescount) &&
                     (BCM_E_NONE == result) &&
                     ((size[res][0] < 512) ||
                      (SHR_RES_ALLOCATOR_BITMAP != restype)));
            /* now res is a bitmap type and we know its base and size */
            if (res < rescount) {
                /* allocate some aligned to base of pool, no offset */
                xresult = BCM_E_NONE;
                for (index = 1;
                     (index < 17) && (BCM_E_NONE == result);
                     index++) {
                    size[res][index] = 7;
                    result = shr_res_alloc_align(unit,
                                                 res,
                                                 0 /* flags */,
                                                 index /* align */,
                                                 0 /* offset */,
                                                 size[res][index],
                                                 &(base[res][index]));
                    if (BCM_E_NONE == result) {
                        printk("allocated %2d elems at %5d, align %2d, offset"
                               " %2d; offset = %2d: ",
                               size[res][index],
                               base[res][index],
                               index,
                               0,
                               (base[res][index] - base[res][0]) % index);
                        if (0 == ((base[res][index] - base[res][0]) % index)) {
                            printk("PASS\n");
                        } else {
                            printk("FAIL\n");
                            xresult = BCM_E_FAIL;
                        }
                    } else {
                        printk("failed to alloc %d elems, align %d, offset %d:"
                               " %d %s)\n",
                               size[res][index],
                               index,
                               0,
                               result,
                               _SHR_ERRMSG(result));
                        xresult = result;
                    }
                }
                while (index > 1) {
                    index--;
                    result = shr_res_free(unit,
                                          res,
                                          size[res][index],
                                          base[res][index]);
                    if (BCM_E_NONE != result) {
                        printk("unable to free %d elems at %d: %d (%s)\n",
                               size[res][index],
                               base[res][index],
                               result,
                               _SHR_ERRMSG(result));
                        xresult = result;
                    }
                }
                for (index = 1;
                     (index < 17) && (BCM_E_NONE == result);
                     index++) {
                    size[res][index] = 7;
                    result = shr_res_alloc_align(unit,
                                                 res,
                                                 0 /* flags */,
                                                 16 /* align */,
                                                 index - 1 /* offset */,
                                                 size[res][index],
                                                 &(base[res][index]));
                    if (BCM_E_NONE == result) {
                        printk("allocated %2d elems at %5d, align %2d, offset"
                               " %2d; offset = %2d: ",
                               size[res][index],
                               base[res][index],
                               16,
                               index - 1,
                               (base[res][index] - base[res][0]) % (16));
                        if ((index - 1) == ((base[res][index] -
                                             base[res][0]) % 16)) {
                            printk("PASS\n");
                        } else {
                            printk("FAIL\n");
                            xresult = BCM_E_FAIL;
                        }
                    } else {
                        printk("failed to alloc %d elems, align %d, offset %d:"
                               " %d %s)\n",
                               size[res][index],
                               index - 1,
                               0,
                               result,
                               _SHR_ERRMSG(result));
                        xresult = result;
                    }
                }
                while (index > 1) {
                    index--;
                    result = shr_res_free(unit,
                                          res,
                                          size[res][index],
                                          base[res][index]);
                    if (BCM_E_NONE != result) {
                        printk("unable to free %d elems at %d: %d (%s)\n",
                               size[res][index],
                               base[res][index],
                               result,
                               _SHR_ERRMSG(result));
                        xresult = result;
                    }
                }
                for (index = 1;
                     (index < 17) && (BCM_E_NONE == result);
                     index++) {
                    size[res][index] = 7;
                    result = shr_res_alloc_align(unit,
                                                 res,
                                                 SHR_RES_ALLOC_ALIGN_ZERO,
                                                 index /* align */,
                                                 0 /* offset */,
                                                 size[res][index],
                                                 &(base[res][index]));
                    if (BCM_E_NONE == result) {
                        printk("allocated %2d elems at %5d, align %2d, offset"
                               " %2d; offset = %2d: ",
                               size[res][index],
                               base[res][index],
                               index,
                               0,
                               base[res][index] % index);
                        if (0 == (base[res][index] % index)) {
                            printk("PASS\n");
                        } else {
                            printk("FAIL\n");
                            xresult = BCM_E_FAIL;
                        }
                    } else {
                        printk("failed to alloc %d elems, align %d, offset %d:"
                               " %d %s)\n",
                               size[res][index],
                               index,
                               0,
                               result,
                               _SHR_ERRMSG(result));
                        xresult = result;
                    }
                }
                while (index > 1) {
                    index--;
                    result = shr_res_free(unit,
                                          res,
                                          size[res][index],
                                          base[res][index]);
                    if (BCM_E_NONE != result) {
                        printk("unable to free %d elems at %d: %d (%s)\n",
                               size[res][index],
                               base[res][index],
                               result,
                               _SHR_ERRMSG(result));
                        xresult = result;
                    }
                }
                for (index = 1;
                     (index < 17) && (BCM_E_NONE == result);
                     index++) {
                    size[res][index] = 7;
                    result = shr_res_alloc_align(unit,
                                                 res,
                                                 SHR_RES_ALLOC_ALIGN_ZERO,
                                                 16 /* align */,
                                                 index - 1 /* offset */,
                                                 size[res][index],
                                                 &(base[res][index]));
                    if (BCM_E_NONE == result) {
                        printk("allocated %2d elems at %5d, align %2d, offset"
                               " %2d; offset = %2d: ",
                               size[res][index],
                               base[res][index],
                               16,
                               index - 1,
                               base[res][index] % 16);
                        if ((index - 1) == (base[res][index] % 16)) {
                            printk("PASS\n");
                        } else {
                            printk("FAIL\n");
                            xresult = BCM_E_FAIL;
                        }
                    } else {
                        printk("failed to alloc %d elems, align %d, offset %d:"
                               " %d %s)\n",
                               size[res][index],
                               index - 1,
                               0,
                               result,
                               _SHR_ERRMSG(result));
                        xresult = result;
                    }
                }
                while (index > 1) {
                    index--;
                    result = shr_res_free(unit,
                                          res,
                                          size[res][index],
                                          base[res][index]);
                    if (BCM_E_NONE != result) {
                        printk("unable to free %d elems at %d: %d (%s)\n",
                               size[res][index],
                               base[res][index],
                               result,
                               _SHR_ERRMSG(result));
                        xresult = result;
                    }
                }
                for (index = 1;
                     (index < 17) && (BCM_E_NONE == result);
                     index++) {
                    size[res][index] = 7;
                    result = shr_res_alloc_align(unit,
                                                 res,
                                                 SHR_RES_ALLOC_ALIGN_ZERO,
                                                 17 /* align */,
                                                 index - 1 /* offset */,
                                                 size[res][index],
                                                 &(base[res][index]));
                    if (BCM_E_NONE == result) {
                        printk("allocated %2d elems at %5d, align %2d, offset"
                               " %2d; offset = %2d: ",
                               size[res][index],
                               base[res][index],
                               17,
                               index - 1,
                               base[res][index] % 17);
                        if ((index - 1) == (base[res][index] % 17)) {
                            printk("PASS\n");
                        } else {
                            printk("FAIL\n");
                            xresult = BCM_E_FAIL;
                        }
                    } else {
                        printk("failed to alloc %d elems, align %d, offset %d:"
                               " %d %s)\n",
                               size[res][index],
                               index - 1,
                               0,
                               result,
                               _SHR_ERRMSG(result));
                        xresult = result;
                    }
                }
                while (index > 1) {
                    index--;
                    result = shr_res_free(unit,
                                          res,
                                          size[res][index],
                                          base[res][index]);
                    if (BCM_E_NONE != result) {
                        printk("unable to free %d elems at %d: %d (%s)\n",
                               size[res][index],
                               base[res][index],
                               result,
                               _SHR_ERRMSG(result));
                        xresult = result;
                    }
                }
                result = xresult;
            } else {
                printk("did not find a sufficiently large bitmap based pool\n");
                result = BCM_E_INTERNAL;
            }
        } /* if (localRes) */
    } else {
        return CMD_USAGE;
    }
    return (BCM_E_NONE == result)?CMD_OK:CMD_FAIL;
}

#endif /* BCM_ESW_SUPPORT */
