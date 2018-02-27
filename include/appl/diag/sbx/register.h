/* $Id: register.h,v 1.4.214.1 2011/07/21 16:47:51 yshtil Exp $
 * $Copyright: (c) 2005 Broadcom Corp.
 * All Rights Reserved.$
 *
 * File:        register.h
 * Purpose:     Base definitions for SBX register types
 * Requires:    
 */

#ifndef _SOC_SBX_REGISTER_H
#define _SOC_SBX_REGISTER_H

#define SBX_DEV_NAME_LEN_MAX 64
#define SBX_REG_NAME_LEN_MAX 128
#define SBX_REG_FIELDS_MAX  64
#define SBX_REG_NUM_MAX 6400

#include <soc/sbx/sbTypesGlue.h>
#include <appl/diag/shell.h>
#include <appl/diag/sbx/field.h>

typedef int soc_sbx_reg_idx;
#define SOC_SBX_REG_IS_INDIRECT 0x80000000
typedef struct soc_sbx_reg_info_s {
    char    name[SBX_REG_NAME_LEN_MAX];
    uint32  offset;
    uint32  ioffset;
    uint32  nt_mask;
    uint32  mask;
    uint8   msb;
    uint8   lsb;
    uint8   intreg;
    uint8   nfields;
    soc_sbx_field_info_t *fields[SBX_REG_FIELDS_MAX];
} soc_sbx_reg_info_t;

typedef struct soc_sbx_reg_info_list_s {
    int     count;
    soc_sbx_reg_idx idx[SBX_REG_NUM_MAX];
} soc_sbx_reg_info_list_t;

typedef struct soc_sbx_chip_info_s {
    char    name[SBX_DEV_NAME_LEN_MAX];
    int     id;
    int     nregs;
    int     maxreg;
    soc_sbx_reg_info_t** regs;
} soc_sbx_chip_info_t;

extern cmd_result_t sbx_chip_info_get(int chip_id,
                                      soc_sbx_chip_info_t **chip_info,
				      int create);

#endif	/* !_SOC_SBX_REGISTER_H */
