/*
 * $Id: drv.c 1.68.2.4 Broadcom SDK $
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
 * StrataSwitch driver
 */

#include <sal/core/libc.h>
#include <shared/alloc.h>
#include <shared/switch.h>
#include <sal/core/spl.h>
#include <sal/core/sync.h>
#include <sal/core/boot.h>
#include <sal/core/dpc.h>

#include <soc/cm.h>
#include <soc/drv.h>
#include <soc/debug.h>
#ifdef BCM_SBX_SUPPORT
#include <soc/sbx/sbx_drv.h>
#endif
#ifdef BCM_EA_SUPPORT
#include <soc/ea/ea_drv.h>
#endif
#ifdef BCM_WARM_BOOT_SUPPORT
#include <soc/mem.h>
#endif
#ifdef BCM_CMICM_SUPPORT
#include <soc/cmicm.h>
#endif

/*
 * Driver global variables
 *
 *   soc_control: per-unit driver control structure
 *   soc_persist: persistent per-unit driver control structure
 *   soc_ndev: the number of units created
 *   soc_ndev_attached: the number of units attached
 */
soc_control_t   *soc_control[SOC_MAX_NUM_DEVICES];
soc_persist_t   *soc_persist[SOC_MAX_NUM_DEVICES];
int             soc_ndev = 0;
int             soc_eth_ndev = 0;
int             soc_ndev_attached = 0;
int             soc_all_ndev;
uint32          soc_state[SOC_MAX_NUM_DEVICES];

/*   soc_wb_mim_state: indicate that all virtual ports / VPNs are MiM */
/*                     Level 1 warm boot only */
#ifdef BCM_WARM_BOOT_SUPPORT
int             soc_wb_mim_state[SOC_MAX_NUM_DEVICES];
#endif

/*
 * Function:
 *      soc_misc_init
 * Purpose:
 *      Initialize miscellaneous chip registers
 * Parameters:
 *      unit - unit number
 * Returns:
 *      SOC_E_XXX
 */
int
soc_misc_init(int unit)
{
    int rv = SOC_E_UNAVAIL;

    SOC_DEBUG_PRINT((DK_VERBOSE, "soc_misc_init\n"));

    if (!soc_attached(unit)) {
        return SOC_E_INIT;
    }

    if(SOC_IS_RCPU_ONLY(unit)) {
        return SOC_E_NONE;
    }

    /* Perform Chip-Dependent Initializations */

    if (SOC_FUNCTIONS(unit)) {
        if (SOC_FUNCTIONS(unit)->soc_misc_init) {
            rv = SOC_FUNCTIONS(unit)->soc_misc_init(unit);
        }
    }

    return rv;
}

/*
 * Function:
 *      soc_init
 * Purpose:
 *      Initialize a StrataSwitch without resetting it.
 * Parameters:
 *      unit - StrataSwitch unit #
 * Returns:
 *      SOC_E_XXX
 */
int
soc_init(int unit)
{
    if (SOC_IS_SIRIUS(unit)) {
#ifdef BCM_SIRIUS_SUPPORT
        return (soc_sbx_reset_init(unit));
#endif /* BCM_SIRIUS_SUPPORT */
    } else if (SOC_IS_ROBO(unit)) {
#ifdef BCM_ROBO_SUPPORT
        return(soc_robo_do_init(unit, FALSE));
#endif
    } else {
#ifdef BCM_ESW_SUPPORT
        return(soc_do_init(unit, FALSE));
#endif
    }

    return SOC_E_UNAVAIL;
}

/*
 * Function:
 *      soc_reset_init
 * Purpose:
 *      Reset and initialize a StrataSwitch.
 * Parameters:
 *      unit - StrataSwitch unit #
 * Returns:
 *      SOC_E_XXX
 */
int
soc_reset_init(int unit)
{
#ifdef BCM_SBX_SUPPORT
    if (SOC_IS_SBX(unit)) {
        return (soc_sbx_reset_init(unit));
    } else 
#endif /* BCM_SBX_SUPPORT */
    if (SOC_IS_ROBO(unit)) {
#ifdef BCM_ROBO_SUPPORT
        return(soc_robo_do_init(unit, FALSE));
#endif
    } else {
#ifdef BCM_ESW_SUPPORT
        return(soc_do_init(unit, TRUE));
#endif
    }

    return SOC_E_UNAVAIL;
}

/*
 * Function:
 *	soc_dev_name
 * Purpose:
 *	Return official name (or optionally nickname) of attached chip.
 * Parameters:
 *      unit - unit number
 * Returns:
 *	Pointer to static string
 */

const char *
soc_dev_name(int unit)
{
    const char *name = soc_cm_get_name(unit);
    return (name == NULL ? "<UNKNOWN>" : name);
}

 /*
 * Function:
 *	soc_attached
 * Purpose:
 *	Check if a specified SOC device is probed and attached.
 * Returns:
 *	TRUE if attached, FALSE otherwise.
 */
int
soc_attached(int unit)
{
    if (!SOC_UNIT_VALID(unit)) {
        return(0);
    }

    return ((SOC_CONTROL(unit)->soc_flags & SOC_F_ATTACHED) != 0);
}

STATIC char *
_str_to_val(const char *str, int *val, int scale, char *suffix)
{
    uint32 abs_val;
    int neg, base, shift;
    unsigned char symbol;
    
    if (*str == '0' && str[1] != '.' && str[1] != ',') {
        str++;

        symbol = *str | 0x20;
        if (symbol == 'x') { /* hexadecimal */
            str++;
            for (abs_val = 0; *str != '\0'; str++) {
                symbol = *str - '0';
                if (symbol < 10) {
                    abs_val = (abs_val << 4) | symbol;
                    continue;
                }
                symbol = (*str | 0x20) - 'a';
                if (symbol < 6) {
                    abs_val = (abs_val << 4) | (symbol + 10);
                    continue;
                }
                break;
            }
        } else {
            if (symbol == 'b') { /* binary */
                base = 2;
                shift = 1;
                str++;
            } else { /* octal */
                base = 8;
                shift = 3;
            }
            for (abs_val = 0; *str != '\0'; str++) {
                symbol = *str - '0';
                if (symbol < base) {
                    abs_val = (abs_val << shift) | symbol;
                    continue;
                }
                break;
            }
        }
        *val = abs_val;
        *suffix = '\0';
    } else {
        if (*str == '-') {
            str++;
            neg = TRUE;
        } else {
            neg = FALSE;
        }
        for (abs_val = 0; *str != '\0'; str++) {
            symbol = *str - '0';
            if (symbol < 10) {
                abs_val = abs_val * 10 + symbol;
                continue;
            }
            break;
        }
        if (*str == '.') {
            str++;
            for (; scale > 0 && *str != '\0'; scale--, str++) {
                symbol = *str - '0';
                if (symbol < 10) {
                    abs_val = abs_val * 10 + symbol;
                    continue;
                }
                break;
            }
        }
        for (; scale > 0; scale--) {
            abs_val *= 10;
        }

        *val = neg ? -(int)abs_val : (int)abs_val;
        *suffix = *str;
    }

    return (char *)str;
}

/*
 * Function:
 *	soc_property_get_str
 * Purpose:
 *	Retrieve a global configuration property string.
 * Parameters:
 *	unit - unit number
 *	name - base name of property to look up
 * Notes:
 *	Each property requested is looked up with one suffix after
 *	another until it is found.  Suffixes are tried in the following
 *	order:
 *		.<unit-num>		(e.g. ".0")
 *		.<CHIP_TYPE>		(e.g. BCM5680_B0)
 *		.<CHIP_GROUP>		(e.g. BCM5680)
 *		<nothing>
 */
char *
soc_property_get_str(int unit, const char *name)
{
    if (unit >= 0) {
        int		l = sal_strlen(name) + 1;
        int             ltemp;
        char		name_exp[SOC_PROPERTY_NAME_MAX], *s;

        if (l >= (SOC_PROPERTY_NAME_MAX - 3)) {
            return NULL;
        }
        sal_sprintf(name_exp, "%s.%d", name, unit);
        if ((s = soc_cm_config_var_get(unit, name_exp)) != NULL) {
            return s;
        }

        ltemp = sal_strlen(SOC_UNIT_NAME(unit));
        if (l > (SOC_PROPERTY_NAME_MAX - ltemp)) {
            return NULL;
        }
        sal_strcpy(name_exp + l, SOC_UNIT_NAME(unit));
        if ((s = soc_cm_config_var_get(unit, name_exp)) != NULL) {
            return s;
        }

        ltemp = sal_strlen(SOC_UNIT_GROUP(unit));
        if (l > (SOC_PROPERTY_NAME_MAX - ltemp)) {
            return NULL;
        }
        sal_strcpy(name_exp + l, SOC_UNIT_GROUP(unit));
        if ((s = soc_cm_config_var_get(unit, name_exp)) != NULL) {
            return s;
        }
    }

    return soc_cm_config_var_get(unit, name);
}

/*
 * Function:
 *      soc_property_get
 * Purpose:
 *      Retrieve a global configuration numeric value.
 * Parameters:
 *      unit - unit number
 *      name - base name of numeric property to look up
 *      defl - default value of numeric property
 * Notes:
 *      See soc_property_get_str for suffix handling.
 *      The string is parsed as a C-style numeric constant.
 */

uint32
soc_property_get(int unit, const char *name, uint32 defl)
{
    char        *s;

    if ((s = soc_property_get_str(unit, name)) == NULL) {
        return defl;
    }

    return _shr_ctoi(s);
}

uint32
soc_property_obj_attr_get(int unit, const char *prefix, const char *obj,
                          int index, const char *attr, int scale,
                          char *suffix, uint32 defl)
{
    char        *str;
    int         val;
    char        prop[80];

    str = NULL;

    if (index != -1) {
        sal_sprintf(prop, "%s.%s%d.%s", prefix, obj, index, attr);
        str = soc_property_get_str(unit, prop);
    }

    if (str == NULL) {
        sal_sprintf(prop, "%s.%s.%s", prefix, obj, attr);
        str = soc_property_get_str(unit, prop);
    }

    if (str == NULL) {
        return defl;
    }

    (void)_str_to_val(str, &val, scale, suffix);
    return (uint32)val;
}

/*
 * Function:
 *      soc_property_get_pbmp
 * Purpose:
 *      Retrieve a global configuration bitmap value.
 * Parameters:
 *      unit - unit number
 *      name - base name of numeric property to look up
 *      defneg - should default bitmap be all 1's?
 * Notes:
 *      See soc_property_get_str for suffix handling.
 *      The string can be more than 32 bits worth of
 *      data if it is in hex format (0x...).  If not
 *      hex, it is treated as a 32 bit value.
 */

pbmp_t
soc_property_get_pbmp(int unit, const char *name, int defneg)
{
    pbmp_t      bm;
    char        *s;

    if ((s = soc_property_get_str(unit, name)) == NULL) {
        SOC_PBMP_CLEAR(bm);
        if (defneg) {
            SOC_PBMP_NEGATE(bm, bm);
        }
        return bm;
    }
    if (_shr_pbmp_decode(s, &bm) < 0) {
        SOC_PBMP_CLEAR(bm);
    }
    return bm;
}

/*
 * Function:
 *      soc_property_get_pbmp_default
 * Purpose:
 *      Retrieve a global configuration bitmap value with specific default.
 * Parameters:
 *      unit - unit number
 *      name - base name of numeric property to look up
 *      def_pbmp - default bitmap
 * Notes:
 *      See soc_property_get_str for suffix handling.
 *      The string can be more than 32 bits worth of
 *      data if it is in hex format (0x...).  If not
 *      hex, it is treated as a 32 bit value.
 */

pbmp_t
soc_property_get_pbmp_default(int unit, const char *name,
                              pbmp_t def_pbmp)
{
    pbmp_t      bm;
    char        *s;

    if ((s = soc_property_get_str(unit, name)) == NULL) {
        SOC_PBMP_ASSIGN(bm, def_pbmp);
        return bm;
    }
    if (_shr_pbmp_decode(s, &bm) < 0) {
        SOC_PBMP_CLEAR(bm);
    }
    return bm;
}

/*
 * Function:
 *      soc_property_port_get
 * Purpose:
 *      Retrieve a per-port global configuration value
 * Parameters:
 *      unit - unit number
 *      port - Zero-based port number.
 *      name - base name of numeric property to look up
 *      defl - default value of numeric property
 * Notes:
 *      The variable name is lookup up with a number of suffixes
 *      until it is found.
 *              name_fe0        port name
 *              name_fe         port type
 *              name.port1      physical port number (one based)
 *              name_port1      logical port number (one based)
 *              name            plain name
 *      If none are found then NULL is returned
 *      See soc_property_get_str for additional suffix handling.
 *      The string is parsed as a C-style numeric constant.
 */

char *
soc_property_port_get_str(int unit, soc_port_t port,
                          const char *name)
{
    char        *s, *p;
    char        prop[80];
    int         pno;

    /* try "name_fe0" */
    sal_sprintf(prop, "%s_%s", name, SOC_PORT_NAME(unit, port));
    if ((s = soc_property_get_str(unit, prop)) != NULL) {
        return s;
    }

    /* try "name_fe" (strip off trailing digits) */
    p = &prop[sal_strlen(prop)-1];
    while (*p >= '0' && *p <= '9') {
        --p;
    }
    *++p = '\0';
    if ((s = soc_property_get_str(unit, prop)) != NULL) {
        return s;
    }

    /* try "name.port%d" for explicit match first */
    sal_sprintf(prop, "%s.port%d", name, port+1);
    if ((s = soc_property_get_str(unit, prop)) != NULL) {
        return s;
    }

    /* try "name_port1": search for matching port number */
    for (pno = 0; pno < SOC_PORT_NUM(unit, port); pno++) {
        if (SOC_PORT(unit, port, pno) == port) {
            break;
        }
    }
    sal_sprintf(prop, "%s_port%d", name, pno+1);
    if ((s = soc_property_get_str(unit, prop)) != NULL) {
        return s;
    }

    /* try "name_%d" for physical port match */
    sal_sprintf(prop, "%s_%d", name, port);
    if ((s = soc_property_get_str(unit, prop)) != NULL) {
        return s;
    }

    /* try plain "name" */
    if ((s = soc_property_get_str(unit, name)) != NULL) {
        return s;
    }

    return NULL;
}

/*
 * Function:
 *      soc_property_port_get
 * Purpose:
 *      Retrieve a per-port global configuration numeric value.
 * Parameters:
 *      unit - unit number
 *      port - Zero-based port number.
 *      name - base name of numeric property to look up
 *      defl - default value of numeric property
 * Notes:
 *      The variable name is lookup up with a number of suffixes
 *      until it is found.
 *              name_fe0        port name
 *              name_fe         port type
 *              name_port1      port number (one based)
 *              name            plain name
 *      If none are found then the default value in defl is returned.
 *      See soc_property_get_str for additional suffix handling.
 *      The string is parsed as a C-style numeric constant.
 */

uint32
soc_property_port_get(int unit, soc_port_t port,
                      const char *name, uint32 defl)
{
    char        *s;

    s = soc_property_port_get_str(unit, port, name);
    if (s) {
      return _shr_ctoi(s);
    }

    return defl;
}

uint32
soc_property_port_obj_attr_get(int unit, soc_port_t port, const char *prefix,
                               const char *obj, int index, const char *attr,
                               int scale, char *suffix, uint32 defl)
{
    char        *str;
    int         val;
    char        prop[80];

    str = NULL;

    if (index != -1) {
        sal_sprintf(prop, "%s.%s%d.%s", prefix, obj, index, attr);
        str = soc_property_port_get_str(unit, port, prop);
    }

    if (str == NULL) {
        sal_sprintf(prop, "%s.%s.%s", prefix, obj, attr);
        str = soc_property_port_get_str(unit, port, prop);
    }

    if (str == NULL) {
        return defl;
    }

    (void)_str_to_val(str, &val, scale, suffix);
    return (uint32)val;
}

int
soc_property_port_get_csv(int unit, soc_port_t port, const char *name,
                          int val_max, int *val_array)
{
    char *str, suffix;
    int count;

    str = soc_property_port_get_str(unit, port, name);
    if (str == NULL) {
        return 0;
    }

    count = 0;
    for (count = 0; count < val_max; count++) {
        str = _str_to_val(str, &val_array[count], 0, &suffix);
        if (suffix == ',') {
            str++;
        } else {
            count++;
            break;
        }
    }

    return count;
}

/*
 * Function:
 *      soc_property_cos_get
 * Purpose:
 *      Retrieve a per-port global configuration value
 * Parameters:
 *      unit - unit number
 *      cos - Zero-based cos number.
 *      name - base name of numeric property to look up
 *      defl - default value of numeric property
 * Notes:
 *      The variable name is lookup up with a number of suffixes
 *      until it is found.
 *              name.cos1       cos number (one based)
 *              name            plain name
 *      If none are found then NULL is returned
 *      See soc_property_get_str for additional suffix handling.
 *      The string is parsed as a C-style numeric constant.
 */

char *
soc_property_cos_get_str(int unit, soc_cos_t cos,
                          const char *name)
{
    char        *s;
    char        prop[80];

    /* try "name.cos%d" for explicit match first */
    sal_sprintf(prop, "%s.cos%d", name, (cos + 1));
    if ((s = soc_property_get_str(unit, prop)) != NULL) {
        return(s);
    }

    /* try plain "name" */
    if ((s = soc_property_get_str(unit, name)) != NULL) {
        return(s);
    }

    return(NULL);
}


/*
 * Function:
 *      soc_property_cos_get
 * Purpose:
 *      Retrieve a per-cos global configuration numeric value.
 * Parameters:
 *      unit - unit number
 *      cos - Zero-based cos number.
 *      name - base name of numeric property to look up
 *      defl - default value of numeric property
 * Notes:
 *      The variable name is lookup up with a number of suffixes
 *      until it is found.
 *              name_cos1       cos number (one based)
 *              name            plain name
 *      If none are found then the default value in defl is returned.
 *      See soc_property_get_str for additional suffix handling.
 *      The string is parsed as a C-style numeric constant.
 */
uint32
soc_property_cos_get(int unit, soc_cos_t cos,
                                    const char *name, uint32 defl)
{
    char        *s;

    s = soc_property_cos_get_str(unit, cos, name);
    if (s) {
      return _shr_ctoi(s);
    }

    return defl;
}


/*
 * Function:
 *      soc_property_suffix_num_get
 * Purpose:
 *      Retrieve a per-enuerated suffix global configuration numeric value.
 * Parameters:
 *      unit - unit number
 *      num - enumerated suffix for which property is applicable
 *      name - base name of numeric property to look up
 *      suffix - suffix of numeric property to look up
 *      defl - default value of numeric property
 * Notes:
 *      The variable name is lookup up with a number of suffixes
 *      until it is found.
 *              name_"suffix"0  enumerated suffix name
 *              name            plain name
 *      If none are found then the default value in defl is returned.
 *      See soc_property_get_str for additional suffix handling.
 *      The string is parsed as a C-style numeric constant.
 */

uint32
soc_property_suffix_num_get(int unit, int num, const char *name,
                            const char *suffix, uint32 defl)
{
    char        *s;
    char        prop[80];

    /* try "name_'suffix'0" */
    sal_sprintf(prop, "%s_%s%1d", name, suffix, num);
    if ((s = soc_property_get_str(unit, prop)) != NULL) {
        return _shr_ctoi(s);
    }

    /* try "name.port%d.suffix" for explicit match first */
    sal_sprintf(prop, "%s.port%d.%s", name, num+1, suffix);
    if ((s = soc_property_get_str(unit, prop)) != NULL) {
        return _shr_ctoi(s);
    }

    /* try plain "name" */
    if ((s = soc_property_get_str(unit, name)) != NULL) {
        return _shr_ctoi(s);
    }

    return defl;
}

/*
 * Function:
 *      soc_property_uc_get_str
 * Purpose:
 *      Retrieve a per-uc global configuration value
 * Parameters:
 *      unit - unit number
 *      uc - Microcontroller number. 0 is PCI Host.
 *      name - base name of numeric property to look up
 * Notes:
 *      The variable name is lookup up with a number of suffixes
 *      until it is found.
 *              name_"suffix"   suffix is pci | uc0 | uc1 ...
 *              name            plain name
 *      If none are found then NULL is returned.
 *      See soc_property_get_str for additional suffix handling.
 *      The string is parsed as a C-style numeric constant.
 */ 
char *
soc_property_uc_get_str(int unit, int uc,
                          const char *name)
{
    char        *s;
    char        prop[80];

    /* try "name_uc%d" for explicit match first */
    if (0 == uc) {
        sal_sprintf(prop, "%s_pci", name);
    } else {
        sal_sprintf(prop, "%s_uc%d", name, (uc - 1));
    }    
    if ((s = soc_property_get_str(unit, prop)) != NULL) {
        return(s);
    }

    /* try plain "name" */
    if ((s = soc_property_get_str(unit, name)) != NULL) {
        return(s);
    }

    return(NULL);
}

/*
 * Function:
 *      soc_property_uc_get
 * Purpose:
 *      Retrieve a per-uc global configuration value
 * Parameters:
 *      unit - unit number
 *      uc - Microcontroller number. 0 is PCI Host.
 *      name - base name of numeric property to look up
 *      defl - default value of numeric property
 * Notes:
 *      The variable name is lookup up with a number of suffixes
 *      until it is found.
 *              name_"suffix"   suffix is pci | uc0 | uc1 ...
 *              name            plain name
 *      If none are found then the default value in defl is returned.
 *      See soc_property_get_str for additional suffix handling.
 *      The string is parsed as a C-style numeric constant.
 */ 
uint32
soc_property_uc_get(int unit, int uc,
                    const char *name, uint32 defl)
{
    char        *s;

    s = soc_property_uc_get_str(unit, uc, name);
    if (s) {
      return _shr_ctoi(s);
    }

    return defl;
}

/*
 * Function:
 *      soc_property_ci_get_str
 * Purpose:
 *      Retrieve a per-ci global configuration value
 * Parameters:
 *      unit - unit number
 *      ci - Chip Intergface Number.
 *      name - base name of numeric property to look up
 * Notes:
 *      The variable name is lookup up with a number of suffixes
 *      until it is found.
 *              name_"suffix"   suffix is ci0 | ci1 ...
 *              name            plain name
 *      If none are found then NULL is returned.
 *      See soc_property_get_str for additional suffix handling.
 *      The string is parsed as a C-style numeric constant.
 */ 
char *
soc_property_ci_get_str(int unit, int ci,
                          const char *name)
{
    char        *s;
    char        prop[80];

    /* try "name_ci%d" for explicit match first */
    sal_sprintf(prop, "%s_ci%d", name, ci);

    if ((s = soc_property_get_str(unit, prop)) != NULL) {
        return(s);
    }

    /* try plain "name" */
    if ((s = soc_property_get_str(unit, name)) != NULL) {
        return(s);
    }

    return(NULL);
}

/*
 * Function:
 *      soc_property_ci_get
 * Purpose:
 *      Retrieve a per-ci global configuration value
 * Parameters:
 *      unit - unit number
 *      ci - Chip Intergface Number.
 *      name - base name of numeric property to look up
 *      defl - default value of numeric property
 * Notes:
 *      The variable name is lookup up with a number of suffixes
 *      until it is found.
 *              name_"suffix"   suffix is ci0 | ci1 ...
 *              name            plain name
 *      If none are found then the default value in defl is returned.
 *      See soc_property_get_str for additional suffix handling.
 *      The string is parsed as a C-style numeric constant.
 */ 
uint32
soc_property_ci_get(int unit, int ci,
                    const char *name, uint32 defl)
{
    char        *s;

    s = soc_property_ci_get_str(unit, ci, name);
    if (s) {
      return _shr_ctoi(s);
    }

    return defl;
}

soc_block_name_t        soc_block_port_names[] =
        SOC_BLOCK_PORT_NAMES_INITIALIZER;
soc_block_name_t        soc_block_names[] =
        SOC_BLOCK_NAMES_INITIALIZER;

soc_block_name_t        soc_sbx_block_port_names[] =
        SOC_BLOCK_SBX_PORT_NAMES_INITIALIZER;
soc_block_name_t        soc_sbx_block_names[] =
        SOC_BLOCK_SBX_NAMES_INITIALIZER;

char *
soc_block_port_name_lookup_ext(soc_block_t blk, int unit)
{
    int         i;

#if defined(BCM_SIRIUS_SUPPORT)
    if (SOC_IS_SIRIUS(unit)) {
	for (i = 0; soc_sbx_block_port_names[i].blk != SOC_BLK_NONE; i++) {
	    if (soc_sbx_block_port_names[i].blk == blk) {
		return soc_sbx_block_port_names[i].name;
	    }
	}
    }
    else 
#endif
    {
        for (i = 0; soc_block_port_names[i].blk != SOC_BLK_NONE; i++) {
            if (soc_block_port_names[i].blk == blk) {
                if (blk == SOC_BLK_GXPORT) {
                    if (SOC_IS_TD_TT(unit)) {
                        return "xlport";
                    }
                } else if (blk == SOC_BLK_BSAFE) {
                    if (SOC_IS_TD_TT(unit)) {
                        return "otpc";
                    }
                }
                return soc_block_port_names[i].name;
            }
        }
    }
    return "?";
}

char *
soc_block_name_lookup_ext(soc_block_t blk, int unit)
{
    int         i;

#if defined(BCM_SIRIUS_SUPPORT)
    if (SOC_IS_SIRIUS(unit)) {
	for (i = 0; soc_sbx_block_names[i].blk != SOC_BLK_NONE; i++) {
	    if (soc_sbx_block_names[i].blk == blk) {
		return soc_sbx_block_names[i].name;
	    }
	}
    }
    else
#endif
    {
        for (i = 0; soc_block_names[i].blk != SOC_BLK_NONE; i++) {
            if (soc_block_names[i].blk == blk) {
                if (blk == SOC_BLK_GXPORT) {
                    if (SOC_IS_TD_TT(unit)) {
                        return "xlport";
                    }
                } else if (blk == SOC_BLK_BSAFE) {
                    if (SOC_IS_TD_TT(unit)) {
                        return "otpc";
                    }
                }
                return soc_block_names[i].name;
            }
        }
    }
    return "?";
}

soc_block_t
soc_block_port_name_match(char *name)
{
    int         i;

    
    for (i = 0; soc_block_port_names[i].blk != SOC_BLK_NONE; i++) {
        if (sal_strcmp(soc_block_port_names[i].name, name) == 0) {
            return soc_block_port_names[i].blk;
        }
    }
    return SOC_BLK_NONE;
}

soc_block_t
soc_block_name_match(char *name)
{
    int         i;

    
    for (i = 0; soc_block_names[i].blk != SOC_BLK_NONE; i++) {
        if (sal_strcmp(soc_block_names[i].name, name) == 0) {
            return soc_block_names[i].blk;
        }
    }
    return SOC_BLK_NONE;
}

#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT)
/*
 * Function:
 *      soc_xport_type_update
 * Purpose:
 *      Rewrite the SOC control port structures to reflect the 10G port mode,
 *      and reinitialize the port
 * Parameters:
 *      unit - XGS unit #.
 *      port - XGS port # on unit.
 *      to_hg_port - (TRUE/FALSE)
 * Returns:
 *      None.
 * Notes:
 *      Must pause linkscan and take COUNTER_LOCK before calling this.
 */
void
soc_xport_type_update(int unit, soc_port_t port, int to_hg_port)
{
#if defined(BCM_FIREBOLT_SUPPORT) || defined(BCM_BRADLEY_SUPPORT)
    soc_info_t          *si;
    soc_port_t          it_port;
    int                 blk, blktype;

    si = &SOC_INFO(unit);

    /* We need to lock the SOC structures until we finish the update */
    SOC_CONTROL_LOCK(unit);

    if (to_hg_port) {
        SOC_PBMP_PORT_ADD(si->st.bitmap, port);
        SOC_PBMP_PORT_ADD(si->hg.bitmap, port);
        SOC_PBMP_PORT_REMOVE(si->ether.bitmap, port);
        SOC_PBMP_PORT_REMOVE(si->xe.bitmap, port);
    } else {
        SOC_PBMP_PORT_ADD(si->ether.bitmap, port);
        SOC_PBMP_PORT_ADD(si->xe.bitmap, port);
        SOC_PBMP_PORT_REMOVE(si->st.bitmap, port);
        SOC_PBMP_PORT_REMOVE(si->hg.bitmap, port);
    }

#define RECONFIGURE_PORT_TYPE_INFO(ptype) \
    si->ptype.num = 0; \
    si->ptype.min = si->ptype.max = -1; \
    PBMP_ITER(si->ptype.bitmap, it_port) { \
        si->ptype.port[si->ptype.num++] = it_port; \
        if (si->ptype.min < 0) { \
            si->ptype.min = it_port; \
        } \
        if (it_port > si->ptype.max) { \
            si->ptype.max = it_port; \
        } \
    }

    /* Recalculate port type data */
    RECONFIGURE_PORT_TYPE_INFO(ether);
    RECONFIGURE_PORT_TYPE_INFO(st);
    RECONFIGURE_PORT_TYPE_INFO(hg);
    RECONFIGURE_PORT_TYPE_INFO(xe);
#undef  RECONFIGURE_PORT_TYPE_INFO

    soc_dport_map_update(unit);

    /* Resolve block membership for registers */
    if (SOC_IS_FB_FX_HX(unit)) { /* Not HBS */
        for (blk = 0; SOC_BLOCK_INFO(unit, blk).type >= 0; blk++) {
            blktype = SOC_BLOCK_INFO(unit, blk).type;
            switch (blktype) {
            case SOC_BLK_IPIPE:
                if (!to_hg_port) {
                    SOC_PBMP_PORT_ADD(si->block_bitmap[blk], port);
                } else {
                    SOC_PBMP_PORT_REMOVE(si->block_bitmap[blk], port);
                }
                break;
            case SOC_BLK_IPIPE_HI:
                if (to_hg_port) {
                    SOC_PBMP_PORT_ADD(si->block_bitmap[blk], port);
                } else {
                    SOC_PBMP_PORT_REMOVE(si->block_bitmap[blk], port);
                }
                break;
            case SOC_BLK_EPIPE:
                if (!to_hg_port) {
                    SOC_PBMP_PORT_ADD(si->block_bitmap[blk], port);
                } else {
                    SOC_PBMP_PORT_REMOVE(si->block_bitmap[blk], port);
                }
                break;
            case SOC_BLK_EPIPE_HI:
                if (to_hg_port) {
                    SOC_PBMP_PORT_ADD(si->block_bitmap[blk], port);
                } else {
                    SOC_PBMP_PORT_REMOVE(si->block_bitmap[blk], port);
                }
                break;
            }
        }
    }

    /* Release SOC structures lock */
    SOC_CONTROL_UNLOCK(unit);

#else /* BCM_FIREBOLT_SUPPORT || BCM_BRADLEY_SUPPORT */
    COMPILER_REFERENCE(unit);
    COMPILER_REFERENCE(port);
    COMPILER_REFERENCE(to_hg_port);
#endif
}

/*
 * Function:
 *      soc_pci_burst_enable
 * Purpose:
 *      Turn on read/write bursting in the cmic
 * Parameters:
 *      unit - unit number
 *
 * Our hardware by default does not accept burst transactions.
 * These transactions need to be enabled in the CMIC_CONFIG register
 * before a burst is performed or the cmic will hang.
 *
 * This code should be called directly after endian configuration to
 * enable bursting as soon as possible.
 *
 */

void
soc_pci_burst_enable(int unit)
{
    uint32 reg;

    /* Make sure these reads/writes are not combined */
    sal_usleep(1000);

    /* Enable Read/Write bursting in the CMIC */
#ifdef BCM_CMICM_SUPPORT
    if(soc_feature(unit, soc_feature_cmicm)) {
        return;
    }
#endif /* CMICM Support */
    reg = soc_pci_read(unit, CMIC_CONFIG);
    reg |= (CC_RD_BRST_EN | CC_WR_BRST_EN);
    soc_pci_write(unit, CMIC_CONFIG, reg);

    /* Make sure our previous write is not combined */
    sal_usleep(1000);
}

/*
 * Function:
 *      soc_endian_config (private)
 * Purpose:
 *      Utility routine for configuring CMIC endian select register.
 * Parameters:
 *      unit - unit number
 */

void
soc_endian_config(int unit)
{
    uint32          val = 0;
    int             big_pio, big_packet, big_other;
#ifdef BCM_CMICM_SUPPORT
    int cmc = SOC_PCI_CMC(unit);
#endif

    soc_cm_get_endian(unit, &big_pio, &big_packet, &big_other);

#ifdef BCM_CMICM_SUPPORT
    if (soc_feature(unit, soc_feature_cmicm)) {
        int y;
        if (big_pio) {
            soc_pci_write(unit, CMIC_COMMON_PCIE_PIO_ENDIANESS_OFFSET, 0x01010101);
        }
        if (big_packet) {
            for(y=0; y < N_DMA_CHAN; y++) {
                val = soc_pci_read(unit, CMIC_CMCx_CHy_DMA_CTRL_OFFSET(cmc,y));
                val |= PKTDMA_BIG_ENDIAN;
                soc_pci_write(unit, CMIC_CMCx_CHy_DMA_CTRL_OFFSET(cmc,y), val);
            }
        }
        if (big_other) {
            /* Set Packet DMA Descriptor Endianness */
            for(y=0; y < N_DMA_CHAN; y++) {
                val = soc_pci_read(unit, CMIC_CMCx_CHy_DMA_CTRL_OFFSET(cmc,y));
                val |= PKTDMA_DESC_BIG_ENDIAN;
                soc_pci_write(unit, CMIC_CMCx_CHy_DMA_CTRL_OFFSET(cmc,y), val);
            }
#ifdef BCM_SBUSDMA_SUPPORT
#define CMIC_CMC0_SBUSDMA_CH0_REQUESTr 0
#define MEMWR_ENDIANESSf 0
#define MEMRD_ENDIANESSf 0
            if (soc_feature(unit, soc_feature_sbusdma)) {
                for(y=0; y < N_SBUSDMA_CHAN; y++) {
                    /* dma/slam */
                    val = soc_pci_read(unit, CMIC_CMCx_SBUSDMA_CHy_REQUEST(cmc, y));
                    soc_reg_field_set(unit, CMIC_CMC0_SBUSDMA_CH0_REQUESTr, &val,
                                      MEMWR_ENDIANESSf, 1);
                    soc_reg_field_set(unit, CMIC_CMC0_SBUSDMA_CH0_REQUESTr, &val,
                                      MEMRD_ENDIANESSf, 1);
                    soc_pci_write(unit, CMIC_CMCx_SBUSDMA_CHy_REQUEST(cmc,y), val);
                }
            } else 
#endif
            {
                /* Set SLAM DMA Endianness */
                val = soc_pci_read(unit, CMIC_CMCx_SLAM_DMA_CFG_OFFSET(cmc));
                val |= SLDMA_BIG_ENDIAN;
                soc_pci_write(unit, CMIC_CMCx_SLAM_DMA_CFG_OFFSET(cmc), val);
                
                /* Set STATS DMA Endianness */
                val = soc_pci_read(unit, CMIC_CMCx_STAT_DMA_CFG_OFFSET(cmc));
                val |= STDMA_BIG_ENDIAN;
                soc_pci_write(unit, CMIC_CMCx_STAT_DMA_CFG_OFFSET(cmc), val);
                
                /* Set TABLE DMA Endianness */
                val = soc_pci_read(unit, CMIC_CMCx_TABLE_DMA_CFG_OFFSET(cmc));
                val |= TDMA_BIG_ENDIAN;
                soc_pci_write(unit, CMIC_CMCx_TABLE_DMA_CFG_OFFSET(cmc), val);
            }
                
            /* Set FIFO DMA Endianness */
            for(y=0; y < N_DMA_CHAN; y++) {
                val = soc_pci_read(unit, CMIC_CMCx_FIFO_CHy_RD_DMA_CFG_OFFSET(cmc,y));
                val |= FIFODMA_BIG_ENDIAN;
                soc_pci_write(unit, CMIC_CMCx_FIFO_CHy_RD_DMA_CFG_OFFSET(cmc,y), val);
            }
            
            /* Set Cross Coupled Memory  Endianness */
            /* NA for PCI */
        }
    } else
#endif
    {
        if (big_pio) {
            val |= ES_BIG_ENDIAN_PIO;
        }

        if (big_packet) {
            val |= ES_BIG_ENDIAN_DMA_PACKET;
        }

        if (big_other) {
            val |= ES_BIG_ENDIAN_DMA_OTHER;
        }

        if (SOC_IS_LYNX(unit) && SAL_BOOT_RTLSIM) { /* And what? */
            val |= ES_BIG_ENDIAN_CTR64_WORDS;
        }

        if (SOC_IS_RAVEN(unit) && SOC_IS_RCPU_UNIT(unit)) {
            val |= EN_BIG_ENDIAN_EB2_2B_SEL;
        }
#ifdef BCM_SHADOW_SUPPORT
        if (SOC_IS_SHADOW(unit) && soc_property_get(unit, spn_DEVICE_EB_VLI, 0)) {
            val = 0x30000030;
        }
#endif
        soc_pci_write(unit, CMIC_ENDIAN_SELECT, val);
    }
}

/*
 * Function:
 *      soc_autoz_set
 * Purpose:
 *      Set the autoz bit in the active mac
 * Parameters:
 *      unit - SOC unit #
 *      port - port number on device.
 *      enable - Boolean value to set state
 * Returns:
 *      SOC_E_XXX
 */
int
soc_autoz_set(int unit, soc_port_t port, int enable)
{
    if (IS_HG_PORT(unit, port) ||
        IS_XE_PORT(unit, port) ||
        SOC_IS_SIRIUS(unit) ||
        SOC_IS_XGS3_SWITCH(unit)) {
        /* XGS (Hercules,Lynx): AutoZ not supported */
        return SOC_E_NONE;
    } else {
        /* StrataSwitch, Draco GMACC ports */
        uint64          reg;
        soc_mac_mode_t  mode;

        SOC_IF_ERROR_RETURN(soc_mac_mode_get(unit, port, &mode));

        switch (mode) {
        case SOC_MAC_MODE_1000_T:
            SOC_IF_ERROR_RETURN(READ_GMACC1r(unit, port, &reg));
            soc_reg64_field32_set(unit, GMACC1r, &reg, AUTOZf,
                                  enable ? 1 : 0);
            SOC_IF_ERROR_RETURN(WRITE_GMACC1r(unit, port, reg));
            break;
        case SOC_MAC_MODE_10_100:
            SOC_IF_ERROR_RETURN(FE_READ(FE_MAC2r, GTH_FE_MAC2r,
                                        unit, port, &reg));
            FE_SET(unit, port, FE_MAC2r, GTH_FE_MAC2r,
                   AUTOZf, reg, enable ? 1 : 0);
            SOC_IF_ERROR_RETURN(FE_WRITE(FE_MAC2r, GTH_FE_MAC2r,
                                         unit, port, reg));
            break;
        default:
            return (SOC_E_INTERNAL);
        }

        return (SOC_E_NONE);
    }
}

/*
 * Function:
 *      soc_autoz_get
 * Purpose:
 *      Get the state of the autoz bit in the active MAC
 * Parameters:
 *      unit - SOC unit #
 *      port - port number on device.
 *      enable - (OUT) Boolean pointer to return state
 * Returns:
 *      SOC_E_XXX
 */
int
soc_autoz_get(int unit, soc_port_t port, int *enable)
{
    if (IS_HG_PORT(unit, port) ||
        IS_XE_PORT(unit, port) ||
        SOC_IS_XGS3_SWITCH(unit)) {
        /* XGS (Hercules): AutoZ not supported */
        *enable = 0;
        return SOC_E_NONE;
    } else {
        /* StrataSwitch, Draco Gigabit ports */
        uint64          reg;
        soc_mac_mode_t  mode;

        SOC_IF_ERROR_RETURN(soc_mac_mode_get(unit, port, &mode));

        switch (mode) {
        case SOC_MAC_MODE_1000_T:
            SOC_IF_ERROR_RETURN(READ_GMACC1r(unit, port, &reg));
            *enable = soc_reg64_field32_get(unit, GMACC1r, reg, AUTOZf);
            break;
        case SOC_MAC_MODE_10_100:
            SOC_IF_ERROR_RETURN(FE_READ(FE_MAC2r, GTH_FE_MAC2r,
                                        unit, port, &reg));
            *enable = FE_GET(unit, port, FE_MAC2r, GTH_FE_MAC2r, AUTOZf, reg);
            break;
        default:
            return(SOC_E_INTERNAL);
        }

        return(SOC_E_NONE);
    }
}

#endif /* defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) */


#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || \
  defined(BCM_ROBO_SUPPORT) || defined(BCM_QE2000_SUPPORT) || defined(BCM_EA_SUPPORT)

int 
soc_event_register(int unit, soc_event_cb_t cb, void *userdata)
{
    soc_control_t       *soc;
    soc_even_cb_list_t  *curr, *prev;

    
    /* Input validation */
    if (!SOC_UNIT_VALID(unit)) {
        return SOC_E_UNIT;
    }

    if (NULL == cb) {
        return SOC_E_PARAM;
    }

    soc = SOC_CONTROL(unit);
    curr = prev = soc->ev_cb_head; 

    if (NULL == curr) { /* Fist time a call back registered */
        curr = (soc_even_cb_list_t *)sal_alloc(sizeof(soc_even_cb_list_t),
                                               "event call back linked list");
        if (NULL == curr) {
            return SOC_E_MEMORY;
        }
        curr->cb = cb;
        curr->userdata = userdata;
        curr->next = NULL;
        soc->ev_cb_head = curr;
    } else { /* Not a first registered callback */
        while (NULL != curr) {
            if ((curr->cb == cb) && (curr->userdata == userdata)) {
                /* call back with exact same userdata exists - nothing to be done */
                return SOC_E_NONE;
            }
            prev = curr;
            curr = prev->next;
        }
        curr = (soc_even_cb_list_t *)sal_alloc(sizeof(soc_even_cb_list_t),
                                               "event call back linked list");
        if (NULL == curr) {
            return SOC_E_MEMORY;
        }
        curr->cb = cb;
        curr->userdata = userdata;
        curr->next = NULL;
        prev->next = curr;
    }

    return SOC_E_NONE;
}

int
soc_event_unregister(int unit, soc_event_cb_t cb, void *userdata)
{
    soc_control_t       *soc;
    soc_even_cb_list_t  *curr, *prev;

    /* Input validation */
    if (!SOC_UNIT_VALID(unit)) {
        return SOC_E_UNIT;
    }

    if (NULL == cb) {
        return SOC_E_PARAM;
    }

    soc = SOC_CONTROL(unit);
    curr = prev = soc->ev_cb_head;

    while (NULL != curr) {
        if (curr->cb == cb) {
            if ((NULL != userdata) && (curr->userdata != userdata)) {
                /* No match keep searching */
                continue;
            }
            /* if cb & userdata matches or userdata is NULL -> delete */
            if (curr == soc->ev_cb_head) {
                soc->ev_cb_head = curr->next;
                sal_free((void *)curr);
                break;
            }
            prev->next = curr->next;
            sal_free((void *)curr);
            break;
        }
        prev = curr;
        curr = curr->next;
    }

    return (SOC_E_NONE);
}

int
soc_event_generate(int unit,  soc_switch_event_t event, uint32 arg1,
                   uint32 arg2, uint32 arg3)
{
    soc_control_t       *soc;
    soc_even_cb_list_t  *curr;

    /* Input validation */
    if (!SOC_UNIT_VALID(unit)) {
        return SOC_E_UNIT;
    }

    soc = SOC_CONTROL(unit);
    curr = soc->ev_cb_head;
    while (NULL != curr) {
        curr->cb(unit, event, arg1, arg2, arg3, curr->userdata);
        curr = curr->next;
    }

    return (SOC_E_NONE);
}

void
soc_event_assert(const char *expr, const char *file, int line)
{
    uint32 arg1, arg3;

    arg1 = PTR_TO_INT(file);
    arg3 = PTR_HI_TO_INT(file);
    
    soc_event_generate(0, SOC_SWITCH_EVENT_ASSERT_ERROR, arg1, line, arg3);
   _default_assert(expr, file, line);
}

#endif /*defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || \
    defined(BCM_ROBO_SUPPORT) || defined(BCM_EA_SUPPORT)  */


#if defined(BCM_XGS3_SWITCH_SUPPORT) || defined(BCM_SIRIUS_SUPPORT)
/*
 * LCPLL lock check for FBX devices
 */
void
soc_xgxs_lcpll_lock_check(int unit)
{
    if ((!SAL_BOOT_PLISIM) && (!SAL_BOOT_QUICKTURN) &&
        soc_feature(unit, soc_feature_xgxs_lcpll)) {
        uint32 val;
        int pll_lock_usec = 500000;
        int locked = 0;
        int retry = 3;
        soc_timeout_t to;

        /* Check if LCPLL locked */
        while (!locked && retry--) {
            soc_timeout_init(&to, pll_lock_usec, 0);
            while (!soc_timeout_check(&to)) {
#if defined(BCM_RAVEN_SUPPORT)
                if (SOC_REG_IS_VALID(unit, CMIC_XGXS_PLL_STATUSr)) {
                    READ_CMIC_XGXS_PLL_STATUSr(unit, &val);
                    locked = soc_reg_field_get(unit, CMIC_XGXS_PLL_STATUSr,
                                            val, CMIC_XG_PLL_LOCKf);
                } else
#endif
                {
                    READ_CMIC_XGXS_PLL_CONTROL_2r(unit, &val);
                    locked = soc_reg_field_get(unit, CMIC_XGXS_PLL_CONTROL_2r,
                                               val, PLL_SM_FREQ_PASSf);
                }
                if (locked) {
                    break;
                }
            }
            if (!locked) {
                READ_CMIC_XGXS_PLL_CONTROL_1r(unit, &val);
                soc_reg_field_set(unit, CMIC_XGXS_PLL_CONTROL_1r, &val,
                                  RESETf, 1);
                WRITE_CMIC_XGXS_PLL_CONTROL_1r(unit, val);
                sal_usleep(100);

                READ_CMIC_XGXS_PLL_CONTROL_1r(unit, &val);
                val |= 0xf0000000;
                WRITE_CMIC_XGXS_PLL_CONTROL_1r(unit, val);
                sal_usleep(100);

                READ_CMIC_XGXS_PLL_CONTROL_1r(unit, &val);
                soc_reg_field_set(unit, CMIC_XGXS_PLL_CONTROL_1r, &val,
                                  RESETf, 0);
                WRITE_CMIC_XGXS_PLL_CONTROL_1r(unit, val);
                sal_usleep(50);
            }
        }

        if (!locked) {
            SOC_ERROR_PRINT((DK_ERR,
                             "fbx_lcpll_lock_check: LCPLL not locked on unit %d "
                             "status = 0x%08x\n",
                         unit, val));
        }
    }
}
#endif /* BCM_XGS3_SWITCH_SUPPORT || BCM_SIRIUS_SUPPORT */


#ifdef BCM_BIGMAC_SUPPORT
/* Reset XGXS (unicore, hyperlite, ...) via BigMAC fusion interface or port
 * registers */
int
soc_xgxs_reset(int unit, soc_port_t port)
{
    soc_reg_t           reg;
    uint64              rval64;
    int                 reset_sleep_usec;

    reset_sleep_usec = SAL_BOOT_QUICKTURN ? 500000 : 1100;

    reg = MAC_XGXS_CTRLr;
#if defined(BCM_TRIUMPH_SUPPORT)
    if (SOC_IS_TR_VL(unit) && !SOC_IS_TRIUMPH2(unit) && !SOC_IS_APOLLO(unit) &&
         !SOC_IS_ENDURO(unit) && !SOC_IS_HURRICANE(unit) && !SOC_IS_VALKYRIE2(unit) && 
        (port == 6 || port == 7 || port == 18 || port == 19 ||
         port == 35 || port == 36 || port == 46 || port == 47)) {
        reg = XGPORT_XGXS_CTRLr;
    }
#endif /* BCM_TRIUMPH_SUPPORT */
#if defined(BCM_SCORPION_SUPPORT)
    if (SOC_IS_SC_CQ(unit) && (port >= 25 && port <= 28)) {
        reg = QGPORT_MAC_XGXS_CTRLr;
    }
#endif /* BCM_SCORPION_SUPPORT */
#if defined(BCM_TRIDENT_SUPPORT) || defined(BCM_SHADOW_SUPPORT)
    if (SOC_IS_TD_TT(unit) || SOC_IS_SHADOW(unit)) {
        reg = XLPORT_XGXS_CTRL_REGr;
    }
#endif /* BCM_TRIDENT_SUPPORT */
#if defined(BCM_KATANA_SUPPORT)
    if (SOC_IS_KATANA(unit)) {
        reg = XPORT_XGXS_CTRLr;
    }
#endif /* BCM_KATANA_SUPPORT */

#if defined(BCM_XGS3_SWITCH_SUPPORT) || defined(BCM_SIRIUS_SUPPORT)
    if (SOC_REG_FIELD_VALID(unit, reg, LCREFENf) || 
        SOC_REG_FIELD_VALID(unit, reg, LCREF_ENf)) {
        int lcpll;
        soc_field_t lcpll_f;

        lcpll_f = SOC_REG_FIELD_VALID(unit, reg, LCREFENf) ? LCREFENf : LCREF_ENf;
        /*
         * Reference clock selection
         */
        lcpll = soc_property_port_get(unit, port, spn_XGXS_LCPLL,
                                      SAL_BOOT_QUICKTURN ? 0 : 1);
        if (lcpll) { /* Internal LCPLL reference clock */
            /* Double-check LCPLL lock */
            soc_xgxs_lcpll_lock_check(unit);
        }
        SOC_IF_ERROR_RETURN(soc_reg_get(unit, reg, port, 0, &rval64));
        soc_reg64_field32_set(unit, reg, &rval64, lcpll_f, lcpll ? 1 : 0);
        SOC_IF_ERROR_RETURN(soc_reg_set(unit, reg, port, 0, rval64));
    }
#endif /* BCM_XGS3_SWITCH_SUPPORT || BCM_SIRIUS_SUPPORT */

    /*
     * XGXS MAC initialization steps.
     *
     * A minimum delay is required between various initialization steps.
     * There is no maximum delay.  The values given are very conservative
     * including the timeout for PLL lock.
     */
    /* Release reset (if asserted) to allow bigmac to initialize */
    SOC_IF_ERROR_RETURN(soc_reg_get(unit, reg, port, 0, &rval64));
    soc_reg64_field32_set(unit, reg, &rval64, IDDQf, 0);
    if (soc_reg_field_valid(unit, reg, PWRDWNf)) {
        soc_reg64_field32_set(unit, reg, &rval64, PWRDWNf, 0);
    }
    if (soc_reg_field_valid(unit, reg, PWRDWN_PLLf)) {
        soc_reg64_field32_set(unit, reg, &rval64, PWRDWN_PLLf, 0);
    }
    if (soc_reg_field_valid(unit,reg, HW_RSTLf)) {
        soc_reg64_field32_set(unit, reg, &rval64, HW_RSTLf, 1);
    } else if (soc_reg_field_valid(unit,reg, RSTB_HWf)) {
        soc_reg64_field32_set(unit, reg, &rval64, RSTB_HWf, 1);
    }
    SOC_IF_ERROR_RETURN(soc_reg_set(unit, reg, port, 0, rval64));
    sal_usleep(reset_sleep_usec);

    /* Power down and reset */
    SOC_IF_ERROR_RETURN(soc_reg_get(unit, reg, port, 0, &rval64));
    if (soc_reg_field_valid(unit, reg, PWRDWNf)) {
        soc_reg64_field32_set(unit, reg, &rval64, PWRDWNf, 1);
    }
    if (soc_reg_field_valid(unit, reg, PWRDWN_PLLf)) {
        soc_reg64_field32_set(unit, reg, &rval64, PWRDWN_PLLf, 1);
    }
    soc_reg64_field32_set(unit, reg, &rval64, IDDQf, 1);
    if (soc_reg_field_valid(unit, reg, HW_RSTLf)) {
        soc_reg64_field32_set(unit, reg, &rval64, HW_RSTLf, 0);
    } else if (soc_reg_field_valid(unit, reg, RSTB_HWf)) {
        soc_reg64_field32_set(unit, reg, &rval64, RSTB_HWf, 0);
    }
    if (soc_reg_field_valid(unit, reg, TXFIFO_RSTLf)) {
        soc_reg64_field32_set(unit, reg, &rval64, TXFIFO_RSTLf, 0);
    } else if (soc_reg_field_valid(unit, reg, TXD1G_FIFO_RSTBf)) {
        soc_reg64_field32_set(unit, XLPORT_XGXS_CTRL_REGr, &rval64, 
                              TXD1G_FIFO_RSTBf, 0);
        soc_reg64_field32_set(unit, XLPORT_XGXS_CTRL_REGr, &rval64, 
                              TXD10G_FIFO_RSTBf, 0);
    }
    if (soc_reg_field_valid(unit, reg, AFIFO_RSTf)) {
        soc_reg64_field32_set(unit, reg, &rval64, AFIFO_RSTf, 1);
    }

    if (SOC_IS_TRX(unit) || SOC_IS_SIRIUS(unit)) { /* How about Bradley */
        soc_reg64_field32_set(unit, reg, &rval64, RSTB_MDIOREGSf, 0);
        soc_reg64_field32_set(unit, reg, &rval64, RSTB_PLLf, 0);
        if (soc_reg_field_valid(unit,reg, BIGMACRSTLf)) {
            soc_reg64_field32_set(unit, reg, &rval64, BIGMACRSTLf, 0);
        }
    }
    SOC_IF_ERROR_RETURN(soc_reg_set(unit, reg, port, 0, rval64));
    sal_usleep(reset_sleep_usec);

    /*
     * Bring up both digital and analog clocks
     *
     * NOTE: Many MAC registers are not accessible until the PLL is locked.
     * An S-Channel timeout will occur before that.
     */
    SOC_IF_ERROR_RETURN(soc_reg_get(unit, reg, port, 0, &rval64));
    if (soc_reg_field_valid(unit, reg, PWRDWNf)) {
        soc_reg64_field32_set(unit, reg, &rval64, PWRDWNf, 0);
    }
    if (soc_reg_field_valid(unit, reg, PWRDWN_PLLf)) {
        soc_reg64_field32_set(unit, reg, &rval64, PWRDWN_PLLf, 0);
    }
    soc_reg64_field32_set(unit, reg, &rval64, IDDQf, 0);
    SOC_IF_ERROR_RETURN(soc_reg_set(unit, reg, port, 0, rval64));
    sal_usleep(reset_sleep_usec);

    /* Bring XGXS out of reset, AFIFO_RST stays 1.  */
    SOC_IF_ERROR_RETURN(soc_reg_get(unit, reg, port, 0, &rval64));
    if (soc_reg_field_valid(unit, reg, HW_RSTLf)) {
        soc_reg64_field32_set(unit, reg, &rval64, HW_RSTLf, 1);
    } else if (soc_reg_field_valid(unit, reg, RSTB_HWf)) {
        soc_reg64_field32_set(unit, reg, &rval64, RSTB_HWf, 1);
    }
    SOC_IF_ERROR_RETURN(soc_reg_set(unit, reg, port, 0, rval64));
    sal_usleep(reset_sleep_usec);

    if (SOC_IS_TRX(unit) || SOC_IS_SIRIUS(unit)) { /* How about Bradley */
        /* Bring MDIO registers out of reset */
        SOC_IF_ERROR_RETURN(soc_reg_get(unit, reg, port, 0, &rval64));
        soc_reg64_field32_set(unit, reg, &rval64, RSTB_MDIOREGSf, 1);
        SOC_IF_ERROR_RETURN(soc_reg_set(unit, reg, port, 0, rval64));

        /* Activate all clocks */
        SOC_IF_ERROR_RETURN(soc_reg_get(unit, reg, port, 0, &rval64));
        soc_reg64_field32_set(unit, reg, &rval64, RSTB_PLLf, 1);
        SOC_IF_ERROR_RETURN(soc_reg_set(unit, reg, port, 0, rval64));

        /* Bring BigMac out of reset*/
        if (soc_reg_field_valid(unit,reg, BIGMACRSTLf)) {
            SOC_IF_ERROR_RETURN(soc_reg_get(unit, reg, port, 0, &rval64));
            soc_reg64_field32_set(unit, reg, &rval64, BIGMACRSTLf, 1);
            SOC_IF_ERROR_RETURN(soc_reg_set(unit, reg, port, 0, rval64));
        }
    }

    /* Bring Tx FIFO out of reset */
    SOC_IF_ERROR_RETURN(soc_reg_get(unit, reg, port, 0, &rval64));
    if (soc_reg_field_valid(unit, reg, TXFIFO_RSTLf)) {
        soc_reg64_field32_set(unit, reg, &rval64, TXFIFO_RSTLf, 1);
    } else if (soc_reg_field_valid(unit, reg, TXD1G_FIFO_RSTBf)) {
        soc_reg64_field32_set(unit, XLPORT_XGXS_CTRL_REGr, &rval64, 
                              TXD1G_FIFO_RSTBf, 0xf);
        soc_reg64_field32_set(unit, XLPORT_XGXS_CTRL_REGr, &rval64, 
                              TXD10G_FIFO_RSTBf, 1);
    }
    SOC_IF_ERROR_RETURN(soc_reg_set(unit, reg, port, 0, rval64));

#if defined(BCM_HELIX15_SUPPORT) || defined(BCM_FELIX15_SUPPORT) || \
    defined(BCM_FIREBOLT2_SUPPORT)
    /* Release LMD reset */
    if (soc_feature(unit, soc_feature_lmd)) {
#if defined(BCM_FIREBOLT2_SUPPORT)
        if (SOC_REG_IS_VALID(unit, CMIC_1000_BASE_X_MODEr)) {
            uint32 lmd_cmic, lmd_bmp, lmd_ether_bmp;
            int hg_offset;

            hg_offset = port - SOC_HG_OFFSET(unit);

            SOC_IF_ERROR_RETURN
                (READ_CMIC_1000_BASE_X_MODEr(unit, &lmd_cmic));
            lmd_bmp =
                soc_reg_field_get(unit, CMIC_1000_BASE_X_MODEr,
                                  lmd_cmic, LMD_ENABLEf);
            lmd_ether_bmp =
                soc_reg_field_get(unit, CMIC_1000_BASE_X_MODEr,
                                  lmd_cmic, LMD_1000BASEX_ENABLEf);
            if (IS_LMD_ENABLED_PORT(unit, port)) {
                lmd_bmp |= (1 << hg_offset);
                if (IS_XE_PORT(unit,port)) {
                    lmd_ether_bmp |= (1 << hg_offset);
                } else {
                    lmd_ether_bmp &= ~(1 << hg_offset);
                }
            } else {
                lmd_bmp &= ~(1 << hg_offset);
                lmd_ether_bmp &= ~(1 << hg_offset);
            }
            soc_reg_field_set(unit, CMIC_1000_BASE_X_MODEr,
                              &lmd_cmic, LMD_ENABLEf, lmd_bmp);
            soc_reg_field_set(unit, CMIC_1000_BASE_X_MODEr, &lmd_cmic,
                              LMD_1000BASEX_ENABLEf, lmd_ether_bmp);
            SOC_IF_ERROR_RETURN
                (WRITE_CMIC_1000_BASE_X_MODEr(unit, lmd_cmic));
        }
#endif /* BCM_FIREBOLT2_SUPPORT */

        SOC_IF_ERROR_RETURN(READ_MAC_CTRLr(unit, port, &rval64));
        soc_reg64_field32_set(unit, MAC_CTRLr, &rval64, LMD_RSTBf, 1);
        SOC_IF_ERROR_RETURN(WRITE_MAC_CTRLr(unit, port, rval64));
    }
#endif /* BCM_HELIX15_SUPPORT || BCM_FELIX15_SUPPORT ||
        * BCM_FIREBOLT2_SUPPORT */
    return SOC_E_NONE;
}
#endif /* BCM_BIGMAC_SUPPORT */
