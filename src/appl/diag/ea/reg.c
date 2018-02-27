/*
 * $Id: reg.c,v 1.1.2.3 2011/04/14 02:44:46 lintaot Exp $
 * $Copyright: (c) 2010 Broadcom Corp.
 * All Rights Reserved.$
 *
 * socdiag register commands
 */

#include <sal/core/libc.h>
#include <soc/counter.h>
#include <sal/appl/pci.h>
#include <soc/debug.h>
#include <soc/cmic.h>
#include <soc/drv_if.h>

#include <appl/diag/system.h>
#include <appl/diag/sysconf.h>
#include <ibde.h>

cmd_result_t
cmd_ea_reg_list(int unit, args_t *a)
{
	return CMD_OK;
}
