/*
 * $Id: config.c 1.16 Broadcom SDK $
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
 * SAL Configuration Variable Support
 *
 * The SAL layer maintains a set of configuration variables for use
 * by any upper layers (such as SOC or BCM).  It is maintained in the
 * SAL layer without using anything other than the standard lib calls
 * to avoid requiring anything other than just itself.
 *
 * These config variables can either be compiled into the image or read
 * from a file during initialization.
 *
 * Precompiled values are defined in the function sal_config_init_defaults()
 * which is automatically built from the uncommented values
 * in $SDK/rc/config.bcm using the bcm2c.pl script.  This generates a source
 * file called platform_defines.c
 *
 * Runtime values are loaded by reading the file config.bcm, typically
 * stored in the switch's flash file system.  The variables are stored one
 * per line in the file, in the form VARIABLE=VALUE.  Comment lines beginning
 * with a # character and empty lines consisting of only white space are
 * ignored.  The file $SDK/rc/config.bcm is an example that can be used
 * as a guide.
 *
 * If the image is compile with NO_FILEIO or SAL_CONFIG_FILE_DISABLE
 * defined, then the runtime values are not loaded from an external file.
 */

#ifdef __KERNEL__
#include <linux/string.h>
#include <linux/ctype.h>
#else
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#endif
#include <assert.h>

#include <sal/core/libc.h>
#include <sal/core/alloc.h>
#include <sal/core/spl.h>
#include <sal/types.h>
#include <sal/appl/config.h>
#include <sal/appl/io.h>

#ifdef	NO_FILEIO
#undef	SAL_CONFIG_FILE_DISABLE
#define	SAL_CONFIG_FILE_DISABLE
#endif	/* NO_FILEIO */

typedef	uint16	sc_hash_t;

/*
 * Speed up searches by first checking if a hash matches.
 * It encodes the first and last character, as well as string length.
 */
#define	SC_HASH(_n, _h)	do { \
	int _l = strlen(_n); \
	_h = (_n[0] << 8) | (_n[_l-1] << 4) | _l; \
	} while (0)

typedef struct sc_s {
    struct sc_s *sc_next;
    char	*sc_name;
    char	*sc_value;
    uint16	sc_flag;  /* Used for duplicate checking and flushing */
    sc_hash_t	sc_hash;
} sc_t;

/* Defines for sc_t->sc_flag */
#define SC_COMPILE_VALUE 0
#define SC_NOT_FLUSHED   0
#define SC_FLUSHED       1

static char *sal_config_file_name = NULL;
static char *sal_config_temp_name = NULL;

static sc_t *sal_config_list = NULL;

#define	FREE_SC(_s)	if (_s) {					\
			    if ((_s)->sc_name) sal_free((_s)->sc_name);	\
			    if ((_s)->sc_value) sal_free((_s)->sc_value);\
			    sal_free(_s);				\
			}

/*
 * Function:
 *	sal_config_file_set
 * Purpose:
 *	Set the name of the active configuration file.
 * Parameters:
 *	fname - name of config file (if NULL, use default; if empty
 *		string, there is no config file)
 *	tname - name of temporary file that gets renamed to config file
 *		(must be given if fname is non-NULL and non-empty)
 * Returns:
 *	-1	Illegal parameter
 *	0	Success
 * Notes:
 *	Makes a copy of the input string.
 */

int
sal_config_file_set(const char *fname, const char *tname)
{
    if (sal_config_file_name != NULL) {
	sal_free(sal_config_file_name);
	sal_config_file_name = NULL;
    }

    if (sal_config_temp_name != NULL) {
	sal_free(sal_config_temp_name);
	sal_config_temp_name = NULL;
    }

    if (fname != NULL) {
	sal_config_file_name = sal_strdup(fname);

	if (fname[0] != 0) {
	    if (tname == NULL || tname[0] == 0) {
		return -1;
	    }

	    sal_config_temp_name = sal_strdup(tname);
	}
    }

    return 0;
}

/*
 * Function:
 *	sal_config_file_get
 * Purpose:
 *	Return the config file names
 * Parameters:
 *	fname - (OUT) Name of config file, NULL if none
 *	tname - (OUT) Name of temporary file, NULL if none
 * Returns:
 *	0	Success
 */

int
sal_config_file_get(const char **fname, const char **tname)
{
    *fname = (sal_config_file_name != NULL ?
	      sal_config_file_name : SAL_CONFIG_FILE);
    *tname = (sal_config_temp_name != NULL ?
	      sal_config_temp_name : SAL_CONFIG_TEMP);

    return 0;
}

/*
 * Function:
 *	sal_config_find
 * Purpose:
 *	Find a config entry for the specified name.
 * Parameters:
 *	name - name of variable to recover
 * Returns:
 *	NULL - not found
 *	!NULL - pointer to value
 */

static sc_t *
sal_config_find(const char *name)
{
    sc_t	*sc;
    sc_hash_t	hash;

    SC_HASH(name, hash);
    for (sc = sal_config_list; sc != NULL; sc = sc->sc_next) {
	if (sc->sc_hash != hash) {
	    continue;
	}
	if (strcmp(sc->sc_name, name) == 0) {
	    return sc;
	}
    }
    return NULL;
}

/*
 * Function:
 * 	sal_config_parse
 * Purpose:
 *	Parse a single input line.
 * Parameters:
 *	str - pointer to null terminated input line.
 * Returns:
 *	NULL - failed to parse
 *	!NULL - pointer to sc entry.
 * Notes:
 *	Does not modify input line.
 */

#ifndef SAL_CONFIG_FILE_DISABLE	/* Only used by fileio functions */
static	sc_t	*
sal_config_parse(char *str)
{
    char	*value;
    sc_t	*sc;

    if ((value = strchr(str, '=')) == NULL) {
	return NULL;
    }
    value += 1;				/* Move past '=' */

    sc = (sc_t *)sal_alloc(sizeof(sc_t), "config parse");
    if (sc != NULL) {
	/* value points past '=' */
	sc->sc_name = sal_alloc(value - str, "config name");
	sc->sc_value = sal_alloc(strlen(value) + 1, "config value");
    }

    if (!sc || !sc->sc_name || !(sc->sc_value)) {
	sal_printf("sal_config_parse: Memory allocation failed\n");
	FREE_SC(sc);
	return NULL;
    }
    strncpy(sc->sc_name, str, value - str - 1);
    sc->sc_name[value - str - 1] = '\0';
    sal_strcpy(sc->sc_value, value);
    SC_HASH(sc->sc_name, sc->sc_hash);
    sc->sc_next = NULL;
    return sc;
}
#endif	/* SAL_CONFIG_FILE_DISABLE */

/*
 * Function:
 * 	sal_config_init
 * Purpose:
 *	If not already initialized, call refresh.
 * Parameters:
 *	None
 * Returns:
 *	0 - success
 *	-1 - failed.
 */

int
sal_config_init(void)
{
    static int initdone = FALSE;

    if (!initdone) {
        initdone = TRUE;
        return sal_config_refresh();
    }

    return 0;
}

/*
 * Function:
 * 	sal_config_refresh
 * Purpose:
 *      Refresh default (compiled-in) configuration.
 *	Refresh the memory image from the configuration file,
 *      clobbering default values (if the config file exists).
 * Parameters:
 *	None
 * Returns:
 *	0 - success
 *	-1 - failed.
 */

int
sal_config_refresh(void)
{
    sc_t	*sc;
#ifndef SAL_CONFIG_FILE_DISABLE
    FILE	*fp;
    sc_t	*last_sc, *found_sc;
    char	str[SAL_CONFIG_STR_MAX], *c;
    int		line = 0;
    char	*fname;
#endif

    /* Clear all previous state */
    while ((sc = sal_config_list) != NULL) {
	sal_config_list = sc->sc_next;
	FREE_SC(sc);
    }

    /* load precompiled values from platform_defines.c */
    sal_config_init_defaults();

#ifndef	SAL_CONFIG_FILE_DISABLE
    fname = (sal_config_file_name != NULL ?
	     sal_config_file_name : SAL_CONFIG_FILE);

    if (fname[0] == 0) {
	return 0;	/* No config file */
    }

    /* Try to load config file ... */
    if ((fp = sal_fopen(fname, "r")) == NULL) {
#if defined(BROADCOM_DEBUG)
	sal_printf("sal_config_refresh: cannot read file: %s, "
		   "variables not loaded\n",
		   fname);
#endif /* BROADCOM_DEBUG */
	return -1;
    }

    /*
     * Add new file-based variables, superseding any matching compiled
     * variables.  Find end-of-list, and initialize the default-or-file
     * flag.
     */
    for (last_sc = sc = sal_config_list; sc != NULL; sc = sc->sc_next) {
	sc->sc_flag = SC_COMPILE_VALUE;
	last_sc = sc;
    }

    /* Read the entire file  - parsing as we go */

    while (fgets(str, sizeof(str), fp)) {

	line++;

	/* Skip comment lines */

	if (str[0] == '#') {
	    continue;
	}

	/* Strip trailing newline/blanks */

	c = str + strlen(str);
	while (c > str && isspace((unsigned) c[-1])) {
	    c--;
	}

	*c = '\0';

	/* Skip blank lines */

	if (str[0] == 0) {
	    continue;
	}

	if ((sc = sal_config_parse(str)) == NULL) {
	    sal_printf("sal_config_refresh: format error "
		       "in %s on line %d (ignored)\n",
		       fname, line);
	    continue;
	}

	/* Check for pre-existing default or duplicates in the file */
 	found_sc = sal_config_find(sc->sc_name);
	if (found_sc != NULL) {
	    if (found_sc->sc_flag != SC_COMPILE_VALUE) {
	        sal_printf("sal_config_refresh: ignoring duplicate entry "
			   "\"%s\"\n"
			   "                    %s line %d "
			   "(first defined on line %d)\n",
			   sc->sc_name, fname,
			   line, found_sc->sc_flag);
	    }
	    else {
	        /* Clobber the compiled-in default value */
	        char *temp = sc->sc_value;         /* New value */
	        sc->sc_value = found_sc->sc_value; /* Old, to be freed */
	        found_sc->sc_value = temp;
		found_sc->sc_flag = line; /* Line # of 1st definition */
	    }
	    FREE_SC(sc);
	    continue;
	}
	if (last_sc) {
	    last_sc->sc_next = sc;
	    last_sc = sc;
	} else {
	    last_sc = sal_config_list = sc;
	}
	sc->sc_flag = line; /* Line # of 1st definition */
    } /* parse config file */

    sal_fclose(fp);
#endif /* SAL_CONFIG_FILE_DISABLE */

    return 0;
}

/*
 * Function:
 * 	sal_config_flush
 * Purpose:
 *	Flush the memory image to the configuration file.
 * Parameters:
 *	None
 * Returns:
 *	0 - success
 *	-1 - failed.
 */

int
sal_config_flush(void)
{
    int		rv = 0;
#ifndef SAL_CONFIG_FILE_DISABLE
    FILE	*old_fp, *new_fp;
    sc_t	*sc, *new_sc;
    char	str[SAL_CONFIG_STR_MAX], *c;
    int		line = 0;
    char	*fname, *tname;

    fname = (sal_config_file_name != NULL ?
	     sal_config_file_name : SAL_CONFIG_FILE);

    if (fname[0] == 0) {
	return 0;	/* No config file */
    }

    tname = (sal_config_temp_name != NULL ?
	     sal_config_temp_name : SAL_CONFIG_TEMP);

    assert(tname != NULL && tname[0] != 0);

    /* Attempt to create temp file */

    if ((new_fp = sal_fopen(tname, "w")) == NULL) {
	rv = -1;
	goto done;
    }

    /* old_fp can be NULL if creating config file for first time */

    old_fp = sal_fopen(fname, "r");

    /* Initialize the "flushed" flag */
    for (sc = sal_config_list; sc != NULL; sc = sc->sc_next) {
	sc->sc_flag = SC_NOT_FLUSHED;
    }

    /* Read the entire file  - parsing as we go */

    while (old_fp != NULL && fgets(str, sizeof(str), old_fp)) {
	char *s;

	line++;

	/* Strip trailing newline/blanks */

	c = str + strlen(str);
	while (c > str && isspace((unsigned) c[-1])) {
	    c--;
	}

	*c = '\0';

	/*
	 * Transfer blank lines and comment lines, but NOT commented-out
	 * variable settings (yet)
	 */

	if (str[0] == 0 || (str[0] == '#' && strchr(str, '=') == NULL)) {
	    fprintf(new_fp, "%s\n", str);
	    continue;
	}

	s = str;
	if (*s == '#') {
	    s++;
	}

	if ((sc = sal_config_parse(s)) == NULL) {
	    sal_printf("sal_config_flush: format error "
		       "in %s on line %d (removed)\n",
		       fname, line);
	    continue;
	}

	/* Write new value (or comment) for this entry */

	if ((new_sc = sal_config_find(sc->sc_name)) == NULL ||
	    new_sc->sc_flag == SC_FLUSHED) {
	    /* Not found or a dup, write out commented assignment */
	    fprintf(new_fp, "#%s=%s\n", sc->sc_name, sc->sc_value);
	} else {
	    fprintf(new_fp, "%s=%s\n", new_sc->sc_name, new_sc->sc_value);
	    new_sc->sc_flag = SC_FLUSHED;
	}

	FREE_SC(sc);
    }

    /* Write out the current values that were not in the old_fp file */

    for (sc = sal_config_list; sc != NULL; sc = sc->sc_next) {
	if (sc->sc_flag == SC_NOT_FLUSHED) {
	    fprintf(new_fp, "%s=%s\n", sc->sc_name, sc->sc_value);
	}
    }

    sal_fclose(new_fp);

    if (old_fp != NULL) {
	sal_fclose(old_fp);
    }

    if (rv == 0) {			/* No error, rename file */
	rv = sal_rename(tname, fname);
    }

    if (rv != 0) {			/* Error, remove temp file */
	(void)sal_remove(tname);
    }

 done:
    if (rv < 0) {
	sal_printf("sal_config_flush: variables not saved\n");
    }
#endif /* SAL_CONFIG_FILE_DISABLE */

    return rv;
}

/*
 * Function:
 *	sal_config_get
 * Purpose:
 *	Recover a sal configuration variable
 * Parameters:
 *	name - name of variable to recover
 * Returns:
 *	NULL - not found
 *	!NULL - pointer to value
 */

char *
sal_config_get(const char *name)
{
    sc_t	*sc;

    sc = sal_config_find(name);
    return sc ? sc->sc_value : NULL;
}

/*
 * Function:
 *	sal_config_get_next
 * Purpose:
 *	Recover a sal configuration variables in order.
 * Parameters:
 *	name - (IN/OUT) name of last variable recovered.
 *	value- (OUT) value of variable recovered.
 * Returns:
 *	0 - variable recovered.
 *	-1 - All complete, not more variables.
 */

int
sal_config_get_next(char **name, char **value)
{
    sc_t	*sc;

    if (*name) {
	sc = sal_config_find(*name);
	assert(sc);
	sc = sc->sc_next;
    } else {
	sc = sal_config_list;
    }
    if (sc) {
	*name = sc->sc_name;
	*value = sc->sc_value;
	return 0;
    } else {
	return -1;
    }
}

/*
 * Function:
 *	sal_config_set
 * Purpose:
 *	Set a sal configuration variable
 * Parameters:
 *	name - name of variable to set
 *	value - name of value; can be NULL to delete a variable
 * Returns:
 *	0 - found and changed
 *	-1 - not found or out of memory
 */

int
sal_config_set(char *name, char *value)
{
    sc_t	*sc, *psc;
    char	*newval;
    sc_hash_t	hash;

    if (name == NULL || *name == '\0') {
	return -1;
    }

    SC_HASH(name, hash);
    psc = NULL;
    for (sc = sal_config_list; sc != NULL; psc = sc, sc = sc->sc_next) {
	if (hash != sc->sc_hash) {
	    continue;
	}
	if (strcmp(sc->sc_name, name) == 0) {
	    break;
	}
    }

    if (sc != NULL) {		/* found */
	if (value == NULL) {	/* delete */
	    if (psc == NULL) {
		sal_config_list = sc->sc_next;
	    } else {
		psc->sc_next = sc->sc_next;
	    }
	    FREE_SC(sc);
	    return 0;
	} else {		/* replace */
	    newval = sal_alloc(strlen(value) + 1, "config value");
	    if (newval == NULL) {
		return -1;
	    }
	    sal_strcpy(newval, value);
	    sal_free(sc->sc_value);
	    sc->sc_value = newval;
	    return 0;
	}
    }

    /* not found, delete */
    if (value == NULL) {
	return -1;
    }

    /* not found, add */
    if ((sc = sal_alloc(sizeof(sc_t), "config set")) == NULL) {
	return -1;
    }

    sc->sc_name = sal_alloc(strlen(name) + 1, "config name");
    sc->sc_value = sal_alloc(strlen(value) + 1, "config value");

    if (sc->sc_name == NULL || sc->sc_value == NULL) {
	FREE_SC(sc);
	return -1;
    }

    sal_strcpy(sc->sc_name, name);
    sal_strcpy(sc->sc_value, value);
    sc->sc_hash = hash;

    sc->sc_next = sal_config_list;
    sal_config_list = sc;

    return 0;
}

/*
 * Function:
 *	sal_config_show
 * Purpose:
 *	Display current configuration variables.
 * Parameters:
 *	none
 * Returns:
 *	Nothing
 */

void
sal_config_show(void)
{
    sc_t	*sc;

    for (sc = sal_config_list; sc != NULL; sc = sc->sc_next) {
	sal_printf("\t%s=%s\n", sc->sc_name, sc->sc_value);
    }
}
