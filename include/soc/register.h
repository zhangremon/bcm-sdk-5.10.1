/*
 * $Id: register.h,v 1.20.20.1 2011/03/29 08:48:41 kliao Exp $
 * $Copyright: (c) 2005 Broadcom Corp.
 * All Rights Reserved.$
 *
 * File:        register.h
 * Purpose:     Base definitions for register types
 * Requires:    
 */

#ifndef _SOC_REGISTER_H
#define _SOC_REGISTER_H

#include <soc/defs.h>
#include <soc/mcm/allenum.h>
#include <soc/mcm/robo/allenum.h>
#include <soc/field.h>

/* argument to soc_reg_addr() */
#define	REG_PORT_ANY		(-10)

/* use bit 31 to indicate the port argument is an instance id */
#define SOC_REG_ADDR_INSTANCE_BP         (31)
#define SOC_REG_ADDR_INSTANCE_MASK       (1<<SOC_REG_ADDR_INSTANCE_BP)

#define _SOC_MAX_REGLIST        (2 * SOC_MAX_NUM_PORTS * SOC_MAX_NUM_COS)


#define _SOC_ROBO_MAX_COSREG         (SOC_ROBO_MAX_NUM_COS)
#define _SOC_ROBO_MAX_PORTREG        (SOC_ROBO_MAX_NUM_PORTS)
#define _SOC_ROBO_MAX_REGLIST        (2 * SOC_ROBO_MAX_NUM_PORTS * SOC_ROBO_MAX_NUM_COS)


/* This remaps the names of FE macros according to whether
 * we're in GSL mode or not.  In that mode, the macros have
 * GTH_ at the beginning of their names.
 *
 * Note that this does _not_ apply to Tucana, where the register
 * names are the same.
 */

#define _GTH(unit, port) \
    (!(SOC_IS_TUCANA(unit) || SOC_IS_XGS3_SWITCH(unit)) \
     && IS_GE_PORT((unit), (port)))

#define _REG_NAME(unit, port, reg, gth_reg) \
    (_GTH(unit, port) ? gth_reg : reg)

#define _REG_ADDR(unit, port, reg, gth_reg) \
    soc_reg_addr(unit, _REG_NAME(unit, port, reg, gth_reg), port, 0)

#define FE_GET(unit, port, reg, gth_reg, fld, var)  \
    soc_reg64_field32_get(unit, _REG_NAME(unit, port, reg, gth_reg), \
                          var, fld)

#define FE_SET(unit, port, reg, gth_reg, fld, var, val)  \
    soc_reg64_field32_set(unit, _REG_NAME(unit, port, reg, gth_reg), \
                          &var, fld, val)

#define FE_READ(reg, gth_reg, unit, port, val_p) \
    soc_reg_get(unit, _REG_NAME(unit, port, reg, gth_reg), \
                port, 0, val_p)

#define FE_WRITE(reg, gth_reg, unit, port, val) \
    soc_reg_set(unit, _REG_NAME(unit, port, reg, gth_reg), \
                port, 0, val)

/* SOC Register Routines */

extern int soc_reg_field_length(int unit, soc_reg_t reg, soc_field_t field);
extern uint32 soc_reg_field_get(int unit, soc_reg_t reg, uint32 regval,
			 soc_field_t field);
extern uint64 soc_reg64_field_get(int unit, soc_reg_t reg, uint64 regval,
			   soc_field_t field);
extern void soc_reg_field_set(int unit, soc_reg_t reg, uint32 *regval,
			      soc_field_t field, uint32 value);
extern void soc_reg64_field_set(int unit, soc_reg_t reg, uint64 *regval,
				soc_field_t field, uint64 value);
extern uint32 soc_reg64_field32_get(int unit, soc_reg_t reg, uint64 regval,
			   soc_field_t field);
extern void soc_reg64_field32_set(int unit, soc_reg_t reg, uint64 *regval,
				  soc_field_t field, uint32 value);

extern uint32 soc_reg_addr(int unit, soc_reg_t reg, int port, int index);
extern uint32 soc_reg_addr_get(int unit, soc_reg_t reg, int port, int index, 
                               int *blk, uint8 *acc_type);
extern uint32 soc_reg_datamask(int unit, soc_reg_t reg, int flags);
extern uint64 soc_reg64_datamask(int unit, soc_reg_t reg, int flags);
extern int soc_reg_fields32_modify(int unit, soc_reg_t reg, soc_port_t port,
                                   int field_count, soc_field_t *fields,
                                   uint32 *values);
extern int soc_reg_field32_modify(int unit, soc_reg_t reg, soc_port_t port, 
                                  soc_field_t field, uint32 value);
extern int soc_reg_field_valid(int unit, soc_reg_t reg, soc_field_t field);
#define SOC_REG_FIELD_VALID(_u_,_reg_,_fld_) \
    soc_reg_field_valid(_u_, _reg_, _fld_)

void soc_reg_init(void);

typedef struct soc_reg_info_t {
    soc_block_types_t  block;
    soc_regtype_t      regtype;		/* Also indicates invalid */
    int                numels;          /* If array, num els in array. */
                                        /* Otherwise -1. */
    uint32             offset;		/* Includes 2-bit form field */
#define SOC_REG_FLAG_64_BITS (1<<0)     /* Register is 64 bits wide */
#define SOC_REG_FLAG_COUNTER (1<<1)     /* Register is a counter */
#define SOC_REG_FLAG_ARRAY   (1<<2)     /* Register is an array */
#define SOC_REG_FLAG_NO_DGNL (1<<3)     /* Array does not have diagonal els */
#define SOC_REG_FLAG_RO      (1<<4)     /* Register is write only */
#define SOC_REG_FLAG_WO      (1<<5)     /* Register is read only */
#define SOC_REG_FLAG_ED_CNTR (1<<6)     /* Counter of discard/error */
#define SOC_REG_FLAG_SPECIAL (1<<7)     /* Counter requires special
                                           processing */
#define	SOC_REG_FLAG_EMULATION	(1<<8)	/* Available only in emulation */
#define	SOC_REG_FLAG_VARIANT1	(1<<9)	/* Not available in chip variants  */
#define	SOC_REG_FLAG_VARIANT2	(1<<10)	/* -- '' -- */
#define	SOC_REG_FLAG_VARIANT3	(1<<11)	/* -- '' -- */
#define	SOC_REG_FLAG_VARIANT4	(1<<12)	/* -- '' -- */
#define SOC_REG_FLAG_32_BITS    (1<<13)     /* Register is 32 bits wide */
#define SOC_REG_FLAG_16_BITS    (1<<14)     /* Register is 16 bits wide */
#define SOC_REG_FLAG_8_BITS     (1<<15)     /* Register is 8 bits wide */
#define SOC_REG_FLAG_ARRAY2     (1<<16)     /* Register is an array with stride 2*/

    uint32             flags;
    int                nFields;
    soc_field_info_t   *fields;
#if !defined(SOC_NO_RESET_VALS)||defined(BCM_ROBO_SUPPORT)
    uint32             rst_val_lo;
    uint32             rst_val_hi;      /* 64-bit regs only */
    uint32             rst_mask_lo;     /* 1 where resetVal is valid */
    uint32             rst_mask_hi;     /* 64-bit regs only */
#endif
    int                ctr_idx;         /* Counters only; sw idx */
} soc_reg_info_t;

typedef struct soc_regaddrinfo_t {
    uint32		addr;
    int			valid;
    soc_reg_t		reg;		/* INVALIDr if not used */
    int			idx;            /* If array, this is an index */
    soc_block_t		block;		/* SOC_BLK_NONE if not used */
    soc_port_t		port;		/* -1 if not used */
    soc_cos_t		cos;		/* -1 if not used */
    uint8               acc_type;
    soc_field_t		field;		/* INVALIDf for entire reg */
} soc_regaddrinfo_t;

typedef struct soc_regaddrlist_t {
    int			count;
    soc_regaddrinfo_t	*ainfo;
} soc_regaddrlist_t;

#ifdef BCM_CMICM_SUPPORT
typedef struct soc_cmicm_reg_t {
    uint32 addr;
    char *name;
    soc_reg_t reg;
} soc_cmicm_reg_t;

extern soc_cmicm_reg_t *soc_cmicm_reg_get(uint32 idx);
extern soc_reg_t soc_cmicm_addr_reg(uint32 addr);
extern char *soc_cmicm_addr_name(uint32 addr);
#endif

extern int soc_regaddrlist_alloc(soc_regaddrlist_t *addrlist);
extern int soc_regaddrlist_free(soc_regaddrlist_t *addrlist);

extern int soc_robo_regaddrlist_alloc(soc_regaddrlist_t *addrlist);
extern int soc_robo_regaddrlist_free(soc_regaddrlist_t *addrlist);

/* Function for register iteration. */
typedef int (*soc_reg_iter_f)(int unit, soc_regaddrinfo_t *ainfo, void *data);

extern void soc_regaddrinfo_get(int unit, soc_regaddrinfo_t *ainfo,
			 uint32 addr);
extern void soc_regaddrinfo_extended_get(int unit, soc_regaddrinfo_t *ainfo,
			 soc_block_t block, uint32 addr);
extern void soc_cpuregaddrinfo_get(int unit, soc_regaddrinfo_t *ainfo,
                            uint32 addr);

extern int soc_reg_iterate(int unit, soc_reg_iter_f do_it, void *data);

extern void soc_reg_sprint_addr(int unit, char *bp,
                                soc_regaddrinfo_t *ainfo);

extern void soc_robo_regaddrinfo_get(int unit, soc_regaddrinfo_t *ainfo,
			 uint32 addr);

extern int soc_robo_reg_iterate(int unit, soc_reg_iter_f do_it, void *data);

extern void soc_robo_reg_sprint_addr(int unit, char *bp,
                                soc_regaddrinfo_t *ainfo);

extern void soc_reg_sprint_data(int unit, char *bp, char *infix,
                                soc_reg_t reg, uint32 value);

/* In soc_common.c */
extern char *soc_regtypenames[];
extern int soc_regtype_gran[];

#endif	/* !_SOC_REGISTER_H */
