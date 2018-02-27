/*
 * $Id: common.c 1.22.2.3 Broadcom SDK $
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
 * File:        soc_common.c
 * Purpose:     Common functions for soc drivers
 * Requires:
 */

#include <soc/defs.h>
#include <assert.h>
#include <sal/core/libc.h>

#include <soc/drv.h>
#include <soc/error.h>
#include <soc/cm.h>
#include <soc/debug.h>
#include <soc/mem.h>

/* Initialize composite block types */
uint32 _soc_block_ports_t[] = {
    SOC_BLK_PORT_INITIALIZER
};

uint32 _soc_block_cpu_t[] = {
    SOC_BLK_CPU_INITIALIZER
};

uint32 _soc_block_ether_t[] = {
    SOC_BLK_ETHER_INITIALIZER
};

uint32 _soc_block_higig_t[] = {
    SOC_BLK_HIGIG_INITIALIZER
};

uint32 _soc_block_fabric_t[] = {
    SOC_BLK_FABRIC_INITIALIZER
};

uint32 _soc_block_net_t[] = {
    SOC_BLK_NET_INITIALIZER
};

uint32 _soc_block_hgport_t[] = {
    SOC_BLK_HGPORT_INITIALIZER
};

uint32 _soc_block_sbx_port_t[] = {
    SOC_BLK_SBX_PORT_INITIALIZER
};

/* The granularity of register addresses, indexed by soc_regtype_t */
int soc_regtype_gran[] = {
    1, /* soc_schan_reg,    */
    1, /* soc_genreg,       */
    1, /* soc_portreg,      */
    1, /* soc_cosreg,       */
    4, /* soc_cpureg,       */
    4, /* soc_mcsreg,       */
    4, /* soc_pci_cfg_reg,  */
    1, /* soc_phy_reg,      */
    1, /* soc_spi_reg,      */
    4, /* soc_hostmem_w,    */
    2, /* soc_hostmem_h,    */
    1, /* soc_hostmem_b,    */
    0, /* soc_invalidreg    */
};

/* Names of register types, indexed by soc_regtype_t */
char *soc_regtypenames[] = {
    "schan",          /* soc_schan_reg,    */
    "general",        /* soc_genreg,       */
    "port",           /* soc_portreg,      */
    "cos",            /* soc_cosreg,       */
    "cpu",            /* soc_cpureg,       */
    "mcs",            /* soc_mcsreg,       */
    "pci_cfg",        /* soc_pci_cfg_reg,  */
    "phy",            /* soc_phy_reg,      */
    "spi",          /* soc_spi_reg,         */
    "mem_word",       /* soc_hostmem_w,    */
    "mem_hword",      /* soc_hostmem_h,    */
    "mem_byte",       /* soc_hostmem_b,    */
    "invalid"         /* soc_invalidreg    */
};

/****************************************************************
 *
 * This array maps from specific chip enum value to the group
 * to which the chip belongs.
 *
 * NB: Order of this array must match soc_chip_types_e from soc/defs.h
 *
 ****************************************************************/
soc_chip_groups_t soc_chip_type_map[SOC_CHIP_TYPES_COUNT] =
    SOC_CHIP_TYPE_MAP_INIT;

/****************************************************************
 *
 * This array maps type indices to names.
 *
 * Indexed by soc_chip_types
 *
 ****************************************************************/
char *soc_chip_type_names[SOC_CHIP_TYPES_COUNT] =
    SOC_CHIP_TYPE_NAMES_INIT;

/****************************************************************
 *
 * This array maps group indices to names.
 *
 * Indexed by soc_chip_groups_t
 *
 ****************************************************************/
char* soc_chip_group_names[SOC_CHIP_GROUPS_COUNT] =
    SOC_CHIP_GROUP_NAMES_INIT;

/*
 * The following routines can be called for CMIC (PCI memory space)
 * registers or for SOC registers.  The register type is indicated
 * in rt.
 */

/* Internally retreives block type from index */
int
SOC_BLOCK_IS_TYPE(int unit, int blk_idx, int *list)
{
    int _bidx = 0;
    while(list[_bidx] != SOC_BLOCK_TYPE_INVALID) {
        if (SOC_BLOCK_TYPE(unit, blk_idx) == list[_bidx]) {
            return TRUE;
        }
        _bidx++;
    }
    return FALSE;
}

/* checks if a blk type is part of a composite type */
int
SOC_BLOCK_IS_COMPOSITE(int unit, int blk, int type) 
{
    uint32 *blks = NULL;
    uint8  _tidx = 0;

    switch(type) {
    case SOC_BLK_PORT:
         blks = _soc_block_ports_t;
         break;
    case SOC_BLK_CPU:
         blks = _soc_block_cpu_t;
         break;
    case SOC_BLK_ETHER:
         blks = _soc_block_ether_t;
         break;
    case SOC_BLK_HIGIG:
         blks = _soc_block_higig_t;
         break;
    case SOC_BLK_FABRIC:
         blks = _soc_block_fabric_t;
         break;
    case SOC_BLK_NET:
         blks = _soc_block_net_t;
         break;
    case SOC_BLK_HGPORT:
         blks = _soc_block_hgport_t;
         break;
    case SOC_BLK_SBX_PORT:
         blks = _soc_block_sbx_port_t;
         break;
    default: return FALSE;
    }
    while(blks[_tidx] != SOC_BLOCK_TYPE_INVALID) {
        if (SOC_BLOCK_TYPE(unit, blk) == blks[_tidx]) {
            return TRUE;
        }
        _tidx++;
    }
    return FALSE;
}

/* Type could be specific or composite like PORT.
 * As an optimization one could use SOC_BLOCK_IS if type is 
 * known to not be composite. 
 */
int 
SOC_BLOCK_IN_LIST(int *blk, int type) 
{
    int _bidx = 0;
    uint32 *blks = NULL;
    while(blk[_bidx] != SOC_BLOCK_TYPE_INVALID) {
        int _tidx = 0;
        uint8 composite = TRUE;
        switch(type) {
        case SOC_BLK_PORT:
             blks = _soc_block_ports_t;
             break;
        case SOC_BLK_CPU:
             blks = _soc_block_cpu_t;
             break;
        case SOC_BLK_ETHER:
             blks = _soc_block_ether_t;
             break;
        case SOC_BLK_HIGIG:
             blks = _soc_block_higig_t;
             break;
        case SOC_BLK_FABRIC:
             blks = _soc_block_fabric_t;
             break;
        case SOC_BLK_NET:
             blks = _soc_block_net_t;
             break;
        case SOC_BLK_HGPORT:
             blks = _soc_block_hgport_t;
             break;
        case SOC_BLK_SBX_PORT:
             blks = _soc_block_sbx_port_t;
             break;
        default: composite = FALSE;
        }
        if (composite) {
            if (blks == NULL) {
                return FALSE;
            }
            while(blks[_tidx] != SOC_BLOCK_TYPE_INVALID) {
                if (blk[_bidx] == blks[_tidx]) {
                    return TRUE;
                }
                _tidx++;
            }
            return FALSE;
        } else if (blk[_bidx] == type) {
            return TRUE;
        }
        _bidx++;
    }
    return FALSE;
}

#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT)
int
soc_anyreg_read(int unit, soc_regaddrinfo_t *ainfo, uint64 *data)
{
    uint32          tmpdata;
    int             rv = SOC_E_NONE;
    soc_regtype_t   type;

    if (ainfo->reg < 0) {
        type = soc_genreg;
    } else {
        type = SOC_REG_INFO(unit, ainfo->reg).regtype;
    }

    switch (type) {
    case soc_cpureg:
        tmpdata = soc_pci_read(unit, ainfo->addr);
        COMPILER_64_SET(*data, 0, tmpdata);
        break;
#ifdef BCM_CMICM_SUPPORT
    case soc_mcsreg:
        tmpdata = soc_pci_mcs_read(unit, ainfo->addr);
        COMPILER_64_SET(*data, 0, tmpdata);
        break;
#endif
    case soc_genreg:
        if (soc_feature(unit, soc_feature_new_sbus_format)) {
            if (ainfo->valid && (int)ainfo->reg >= 0 &&
                SOC_REG_IS_VALID(unit, ainfo->reg) &&
                SOC_REG_IS_64(unit, ainfo->reg)) {
                rv = soc_reg_get(unit, ainfo->reg, ainfo->port,
                                 ainfo->idx, data);
            } else {
                rv = soc_reg32_get(unit, ainfo->reg, ainfo->port,
                                   ainfo->idx, &tmpdata);
                COMPILER_64_SET(*data, 0, tmpdata);
            }
            break;
        }
    case soc_portreg:
        if (soc_feature(unit, soc_feature_new_sbus_format)) {
            if (ainfo->valid && (int)ainfo->reg >= 0 &&
                SOC_REG_IS_VALID(unit, ainfo->reg) &&
                SOC_REG_IS_64(unit, ainfo->reg)) {
                rv = soc_reg_get(unit, ainfo->reg, ainfo->port, ainfo->idx, data);
            } else {
                rv = soc_reg32_get(unit, ainfo->reg, ainfo->port, ainfo->idx, &tmpdata);
                COMPILER_64_SET(*data, 0, tmpdata);
            }
            break;
        }
    case soc_cosreg:
        if (ainfo->valid && (int)ainfo->reg >= 0 &&
            SOC_REG_IS_VALID(unit, ainfo->reg) &&
            SOC_REG_IS_64(unit, ainfo->reg)) {
            if (soc_feature(unit, soc_feature_new_sbus_format)) {
                rv = soc_reg_get(unit, ainfo->reg, ainfo->cos, ainfo->idx, data);
            } else {
                rv = soc_reg_read(unit, ainfo->reg, ainfo->addr, data);
            }
        } else {
            if (soc_feature(unit, soc_feature_new_sbus_format)) {
                rv = soc_reg32_get(unit, ainfo->reg, ainfo->cos, ainfo->idx, &tmpdata);
            } else {
                rv = soc_reg32_read(unit, ainfo->addr, &tmpdata);
            }    
            COMPILER_64_SET(*data, 0, tmpdata);
        }
        break;
    default:
        assert(0);
        rv = SOC_E_INTERNAL;
        break;
    }

    return rv;
}


/*
 * Function:
 *      soc_anyreg_write
 * Purpose:
 *      Write a soc register according to type
 * Parameters:
 *      unit - soc unit number
 *      ainfo - address information structure
 *      data - 64 bit data.  Will type correctly for register.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      This routine requires ainfo->addr to be
 *      properly set.
 *      If it is a 64-bit register, then additionally, ainfo->valid
 *      must be set and ainfo->reg must be set properly.
 */

int
soc_anyreg_write(int unit, soc_regaddrinfo_t *ainfo, uint64 data)
{
    uint32          tmpdata;
    int             rv = SOC_E_NONE;
    soc_regtype_t   type;

    if (ainfo->reg < 0) {
        type = soc_genreg;
    } else {
        if (!SOC_REG_IS_VALID(unit, ainfo->reg)) {
            return SOC_E_PARAM;
        }
        type = SOC_REG_INFO(unit, ainfo->reg).regtype;
    }

    switch (type) {
    case soc_cpureg:
        COMPILER_64_TO_32_LO(tmpdata, data);
        soc_pci_write(unit, ainfo->addr, tmpdata);
        break;
#ifdef BCM_CMICM_SUPPORT
    case soc_mcsreg:
        COMPILER_64_TO_32_LO(tmpdata, data);
        soc_pci_mcs_write(unit, ainfo->addr, tmpdata);
        break;
#endif
    case soc_genreg:
        if (soc_feature(unit, soc_feature_new_sbus_format)) {
            if (ainfo->valid && (int)ainfo->reg >= 0 &&
                SOC_REG_IS_64(unit, ainfo->reg)) {
                rv = soc_reg64_set(unit, ainfo->reg, ainfo->port,
                                   ainfo->idx, data);
            } else {
#ifdef BCM_EASYRIDER_SUPPORT
                if (soc_feature(unit, soc_feature_prime_ctr_writes) && 
                    SOC_REG_IS_COUNTER(unit, ainfo->reg)) {
                    rv = soc_counter_set(unit, ainfo->port, ainfo->reg,
                                           ainfo->idx, data);
                } else 
#endif      
                {
                    COMPILER_64_TO_32_LO(tmpdata, data);
                    rv = soc_reg32_set(unit, ainfo->reg, ainfo->port,
                                       ainfo->idx, tmpdata);
                }
            }
            break;
        }
    case soc_portreg:
        if (soc_feature(unit, soc_feature_new_sbus_format)) {
            if (ainfo->valid && (int)ainfo->reg >= 0 &&
                SOC_REG_IS_64(unit, ainfo->reg)) {
                rv = soc_reg64_set(unit, ainfo->reg, ainfo->port, ainfo->idx, data);
            } else {
#ifdef BCM_EASYRIDER_SUPPORT
                if (soc_feature(unit, soc_feature_prime_ctr_writes) && 
                    SOC_REG_IS_COUNTER(unit, ainfo->reg)) {
                    rv = soc_counter_set(unit, ainfo->port, ainfo->reg,
                                           ainfo->idx, data);
                } else 
#endif      
                {
                    COMPILER_64_TO_32_LO(tmpdata, data);
                    rv = soc_reg32_set(unit, ainfo->reg, ainfo->port, ainfo->idx, tmpdata);
                }
            }
            break;
        }
    case soc_cosreg:
        if (ainfo->valid && (int)ainfo->reg >= 0 &&
            SOC_REG_IS_64(unit, ainfo->reg)) {
            if (soc_feature(unit, soc_feature_new_sbus_format)) {
                rv = soc_reg64_set(unit, ainfo->reg, ainfo->cos, ainfo->idx, data);
            } else {
                rv = soc_reg64_write(unit, ainfo->addr, data);
            }
        } else {
#ifdef BCM_EASYRIDER_SUPPORT
            if (soc_feature(unit, soc_feature_prime_ctr_writes) && 
                SOC_REG_IS_COUNTER(unit, ainfo->reg)) {
                rv = soc_counter_set(unit, ainfo->port, ainfo->reg,
                                       ainfo->idx, data);
            } else 
#endif
            {
                COMPILER_64_TO_32_LO(tmpdata, data);
                if (soc_feature(unit, soc_feature_new_sbus_format)) {
                    rv = soc_reg32_set(unit, ainfo->reg, ainfo->cos, ainfo->idx, tmpdata);
                } else {
                    rv = soc_reg32_write(unit, ainfo->addr, tmpdata);
                }
            }
        }
        break;
    default:
        assert(0);
        rv = SOC_E_INTERNAL;
        break;
    }

    return rv;
}

#define UPDATE_BP(bp) while (*(bp)) (bp)++;

/* This is inefficient, because it does a lookup for each field. */
void
soc_reg_sprint_data(int unit, char *bp, char *infix, soc_reg_t reg,
                    uint32 value)
{
    int             i, n;
    soc_field_t     field;
    int             len;
    char            buf[24];

    if (!SOC_REG_IS_VALID(unit, reg)) {
        return;
    }
    n = SOC_REG_INFO(unit, reg).nFields;
    for (i = 0; i < n; i++) {
        field = SOC_REG_INFO(unit, reg).fields[i].field;
        len = SOC_REG_INFO(unit, reg).fields[i].len;
        if (i == n - 1) {
            infix = "";
        }
#if defined(SOC_NO_NAMES)
        sal_sprintf(buf, "#%%d = 0x%%0%dx%%s", (len + 3) >> 2);
        sal_sprintf(bp, buf, field,
                    soc_reg_field_get(unit, reg, value, field), infix);
#else
        sal_sprintf(buf, "%%s = 0x%%0%dx%%s", (len + 3) >> 2);
        sal_sprintf(bp, buf, soc_fieldnames[field],
                    soc_reg_field_get(unit, reg, value, field), infix);
#endif
        UPDATE_BP(bp);
    }
}

void
soc_reg_sprint_addr(int unit, char *bp, soc_regaddrinfo_t *ainfo)
{
    soc_field_info_t *finfop = 0;
    int             msb, lsb;
    soc_reg_info_t *reginfo;

    if (!ainfo->valid) {
        sal_strcpy(bp, "Invalid Address");
        return;
    }
    if (ainfo->reg != INVALIDr && SOC_REG_IS_VALID(unit, ainfo->reg)) {
        reginfo = &SOC_REG_INFO(unit, ainfo->reg);
        if (ainfo->field != INVALIDf) {
            SOC_FIND_FIELD(ainfo->field,
                           reginfo->fields,
                           reginfo->nFields,
                           finfop);
            assert(finfop);
            if (finfop->flags & SOCF_LE) {
                msb = finfop->bp + finfop->len - 1;
                lsb = finfop->bp;
            } else {
                msb = finfop->bp;
                lsb = finfop->bp + finfop->len - 1;
            }
            sal_sprintf(bp, "[%d:%d] ", msb, lsb);
            UPDATE_BP(bp);
        }
    } else {
        sal_strcpy(bp, "Reserved");
        return;
    }

#ifdef SOC_NO_NAMES
    sal_sprintf(bp, "#%d", ainfo->reg);
#else
    sal_sprintf(bp, "%s", SOC_REG_NAME(unit, ainfo->reg));
#endif /* SOC_NO_NAMES */
    UPDATE_BP(bp);

    if (SOC_REG_ARRAY(unit, ainfo->reg)) {
        sal_sprintf(bp, "(%d)", ainfo->idx);
        UPDATE_BP(bp);
    }
    if (ainfo->cos != -1) {
        sal_sprintf(bp, ".%d", ainfo->cos);
        UPDATE_BP(bp);
    }
    if (reginfo->regtype != soc_portreg) {
        *bp++ = '.';
        sal_strcpy(bp, SOC_BLOCK_NAME(unit, ainfo->block));
        UPDATE_BP(bp);
    }
    if (reginfo->regtype == soc_portreg) {
        *bp++ = '.';
        sal_strcpy(bp, SOC_PORT_NAME(unit, ainfo->port));
        UPDATE_BP(bp);
    }
    if (finfop) {
#ifdef SOC_NO_NAMES
        sal_sprintf(bp, ".#%d", ainfo->field);
#else
        sal_sprintf(bp, ".%s", soc_fieldnames[ainfo->field]);
#endif /* SOC_NO_NAMES */
        UPDATE_BP(bp);
    }
}
#undef  UPDATE_BP

#define SOC_E_IGNORE -6000

/*
 * This handles the iteration for a particular register addr info
 * It does it across all array entries if the register is an array reg.
 */
static int
_do_reg_iter(int unit, soc_reg_iter_f do_it,
             soc_regaddrinfo_t *ainfo, void *data, int *done)
{
    int             idx;
    int             num_els, gran;
    int             rv = SOC_E_NONE;
    uint32          base_addr;
    soc_reg_t       reg;

    reg = ainfo->reg;
    if (ainfo->reg == INVALIDr || !SOC_REG_IS_VALID(unit, ainfo->reg)) {
        return SOC_E_PARAM;
    }

    base_addr = ainfo->addr;
    gran = SOC_REG_GRAN(unit, reg);
    if (SOC_REG_ARRAY(unit, reg)) {
        num_els = SOC_REG_NUMELS(unit, reg);
	if (SOC_REG_ARRAY2(unit, reg)) {
	    num_els *= 2;
	}
    } else {
        num_els = 1;
    }

    for (idx = 0; (idx < num_els) && !*done; idx++) {
        ainfo->addr = base_addr + (idx * gran);
        ainfo->idx = idx;
        if ((SOC_REG_INFO(unit, reg).flags & SOC_REG_FLAG_NO_DGNL) &&
                (idx == ainfo->port)) {
	    if (SOC_REG_ARRAY2(unit, reg)) {
		idx++;
	    }
            continue;
        }
        rv = (*do_it)(unit, ainfo, data);
        switch (rv) {
        case SOC_E_NONE:
            break;
        case SOC_E_IGNORE:
            rv = SOC_E_NONE;
            *done = 1;
            break;
        default:
            *done = 1;
            break;
        }
	if (SOC_REG_ARRAY2(unit, reg)) {
	    idx++;
	}
    }

    ainfo->addr = base_addr;
    return rv;
}

int
soc_reg_iterate(int unit, soc_reg_iter_f do_it, void *data)
{
    soc_reg_t       reg;
    soc_port_t      port;
    soc_cos_t       cos;
    int             blk;
    soc_block_types_t regblktype;
    soc_regaddrinfo_t ainfo;
    int             done;
    int             skip_cpu;

    ainfo.valid = 1;
    ainfo.field = INVALIDf;

    skip_cpu = soc_property_get(unit, "cmic_regtest_skip", 1);

    for (reg = 0; reg < NUM_SOC_REG; reg++) {
        if (!SOC_REG_IS_VALID(unit, reg)) {
            continue;
        }
        if (!SOC_REG_IS_ENABLED(unit, reg)) {
            continue;
        }
        done = 0;
        ainfo.reg = reg;
        regblktype = SOC_REG_INFO(unit, reg).block;
        if (SAL_BOOT_QUICKTURN && (SOC_BLOCK_IS(regblktype, SOC_BLK_BSAFE))) {
            continue;
        }
        switch (SOC_REG_INFO(unit, reg).regtype) {
        case soc_portreg:
            PBMP_ALL_ITER(unit, port) {
                if (SOC_BLOCK_IN_LIST(regblktype, SOC_BLK_PORT) &&
                    !SOC_BLOCK_IN_LIST(regblktype, SOC_PORT_TYPE(unit, port))) {
                    continue;
                }

#if defined(BCM_5695)
                /* Skip Draco 1.5 "port indexed" registers  */
                /* which are unimplemented on non-ge ports. */
                if (SOC_IS_DRACO15(unit) && (port > 11) &&
                    ((reg == E2EIBPCELLSETLIMIT1r) || 
                     (reg == E2EIBPCELLSETLIMIT2r) || 
                     (reg == E2EIBPCELLSETLIMIT3r) || 
                     (reg == E2EIBPPKTSETLIMIT1r) || 
                     (reg == E2EIBPPKTSETLIMIT2r) || 
                     (reg == E2EIBPPKTSETLIMIT3r) || 
                     (reg == IPMCREPLICATIONCOUNTr))
                    ) {
                    continue;
                }
#endif
#ifdef BCM_FIREBOLT_SUPPORT
                if (SOC_IS_FBX(unit)) {
                    if (SOC_BLOCK_IS(regblktype, SOC_BLK_IPIPE_HI) &&
                        ((!PBMP_MEMBER(SOC_BLOCK_BITMAP(unit,
                                        IPIPE_HI_BLOCK(unit)), port)))) {
                        continue;
                    }
                    if (SOC_BLOCK_IS(regblktype, SOC_BLK_EPIPE_HI) &&
                        ((!PBMP_MEMBER(SOC_BLOCK_BITMAP(unit,
                                        EPIPE_HI_BLOCK(unit)), port)))) {
                        continue;
                    }
                }
#endif
                ainfo.port = port;
                ainfo.addr = soc_reg_addr_get(unit, reg, ainfo.port, 0, 
                                              &ainfo.block, &ainfo.acc_type);
                ainfo.block = -1;
                ainfo.cos = -1;
                _do_reg_iter(unit, do_it, &ainfo, data, &done);
                if (done) {
                    break;
                }
            }
            break;
        case soc_cosreg:
            for (cos = 0; cos < NUM_COS(unit) && !done; cos++) {
                SOC_BLOCKS_ITER(unit, blk, regblktype) {
                    ainfo.port = SOC_BLOCK_PORT(unit, blk);
                    ainfo.addr = soc_reg_addr_get(unit, reg, ainfo.port, cos,
                                                  &ainfo.block, &ainfo.acc_type);
                    ainfo.block = blk;
                    ainfo.cos = cos;
                    _do_reg_iter(unit, do_it, &ainfo, data, &done);
                    if (done) {
                        break;
                    }
                }
            }
            break;
        case soc_genreg:
            if (SOC_BLOCK_IS(regblktype, SOC_BLK_ESM) && 
                !soc_feature(unit, soc_feature_esm_support)) {
                break;
            }
            SOC_BLOCKS_ITER(unit, blk, regblktype) {
                ainfo.port = SOC_BLOCK_PORT(unit, blk);
                ainfo.addr = soc_reg_addr_get(unit, reg, ainfo.port, 0,
                                              &ainfo.block, &ainfo.acc_type);
                ainfo.block = blk;
                ainfo.cos = -1;
                _do_reg_iter(unit, do_it, &ainfo, data, &done);
                if (done) {
                    break;
                }
            }
            break;
        case soc_cpureg:
            /* Skip CPU registers in soc iteration. */
#ifdef BCM_TRIUMPH2_SUPPORT
            if (SOC_IS_TRIUMPH2(unit) && !skip_cpu) {
                SOC_BLOCKS_ITER(unit, blk, regblktype) {
                    ainfo.port = SOC_BLOCK_PORT(unit, blk);
                    ainfo.addr = soc_reg_addr(unit, reg, ainfo.port, 0);
                    ainfo.block = blk;
                    ainfo.cos = -1;
                    _do_reg_iter(unit, do_it, &ainfo, data, &done);
                    if (done) {
                        break;
                    }
                }
            }
#endif
            break;
        case soc_mcsreg:
            /* Skip MCS registers in soc iteration. */
            break;
        default:
            assert(0);
            break;
        }
    }

    return 0;
}

/*
 * Given an address, fill in an regaddrinfo structure
 * This can only be called for SOC registers, not for CPU registers
 */
void
soc_regaddrinfo_get(int unit, soc_regaddrinfo_t *ainfo, uint32 addr)
{
    int                 i;
    int                 blk = 0, bindex, port, phy_port;
    uint32              minadr, maxadr;
    soc_regtype_t       regtype;
    int                 reg;
    uint32              offset;         /* base address */
    int                 block = -1;     /* block number */
    int                 pindex;         /* port/cos index field */
    int                 aindex = 0;     /* array index field */
    soc_block_t         portblktype;

    ainfo->addr = addr;
    ainfo->valid = 1;
    ainfo->reg = INVALIDr;

#ifdef  BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(unit)) {
	portblktype = SOC_BLK_SBX_PORT;
    } 
    else 
#endif
    {
	portblktype = SOC_BLK_PORT;
    }
    if (!soc_feature(unit, soc_feature_new_sbus_format)) {
        block = ((addr >> SOC_BLOCK_BP) & 0xf) | 
                (((addr >> SOC_BLOCK_MSB_BP) & 0x3) << 4);
        /* extract fields from addr */
        if (SOC_IS_HERCULES(unit)) {
            offset = addr & 0xfffff;
            pindex = 0;
            aindex = 0;
#if defined(BCM_TRIDENT_SUPPORT)
        } else if (SOC_IS_TD_TT(unit)) {
            offset = addr & 0x3f080fff;
            pindex = (addr >> SOC_REGIDX_BP) & 0x7f;
            aindex = addr & 0xfff;
#endif /* BCM_TRIDENT_SUPPORT */
#if defined(BCM_FIREBOLT_SUPPORT)
        } else if (SOC_IS_FBX(unit)) {
            offset = addr & 0x3f0c0fff;
            pindex = (addr >> SOC_REGIDX_BP) & 0x3f;
            if (addr & 0x80000000) {
                pindex += 64;
            }
            aindex = addr & 0xfff;
#endif
        } else {
            offset = addr & 0xc0fff;
            pindex = (addr >> SOC_REGIDX_BP) & 0x3f;
            aindex = addr & 0xfff;
        }
    } else {
        pindex = addr & 0xff;
        if (addr & 0x02000000) {
            offset = addr;
        } else {
            offset = addr & 0xffffff00;
        }
    }

    /* find the matching block */
    /* we find the last matching block to make things work with cmic/cpic */
    if (!soc_feature(unit, soc_feature_new_sbus_format)) {
        blk = -1;
        for (i = 0; SOC_BLOCK_INFO(unit, i).type >= 0; i++) {
            if (SOC_BLOCK_INFO(unit, i).cmic == block) {
                blk = i;
                /* break; */
            }
        }
    }
    assert(blk >= 0);
    for (reg = 0; reg < NUM_SOC_REG; reg++) {
        if (!SOC_REG_IS_VALID(unit, reg)) {
            continue;
        }
        minadr = SOC_REG_INFO(unit, reg).offset;
        if (offset < minadr) {
            continue;
        }
        if (SOC_REG_ARRAY(unit, reg)) {
	    if (SOC_REG_ARRAY2(unit, reg)) {
		maxadr = minadr +
		    2 * (SOC_REG_GRAN(unit, reg) * (SOC_REG_NUMELS(unit, reg) - 1));
	    } else {
		maxadr = minadr +
		    (SOC_REG_GRAN(unit, reg) * (SOC_REG_NUMELS(unit, reg) - 1));
	    }
        } else {
            maxadr = minadr;
        }
        if (offset > maxadr) {
            continue;
        }
	if (SOC_REG_ARRAY2(unit, reg)) {
	    if ( (offset & 0x1) != (minadr & 0x1) ) {
		/* make sure last bit are same as the base addr when array stride is 2 */
		continue;
	    }
	}
        regtype = SOC_REG_INFO(unit, reg).regtype;
        if ((regtype == soc_cpureg) || (regtype == soc_mcsreg))  {
            continue;
        }
        if (!soc_feature(unit, soc_feature_new_sbus_format)) {
            if (!SOC_BLOCK_IS_TYPE(unit, blk, SOC_REG_INFO(unit, reg).block)) {
                continue;
            }
        }
        if (SOC_REG_ARRAY(unit, reg)) {
            if (soc_feature(unit, soc_feature_new_sbus_format)) {
                aindex = (offset - SOC_REG_BASE(unit, reg)) >> 8; 
            }
	    if (SOC_REG_ARRAY2(unit, reg)) {
		ainfo->idx = aindex / (2 * SOC_REG_GRAN(unit, reg));
	    } else {
		ainfo->idx = aindex / SOC_REG_GRAN(unit, reg);
	    }
        } else {
            ainfo->idx = -1;
        }
        if (!soc_feature(unit, soc_feature_new_sbus_format)) {
            ainfo->block = blk;
        }
        ainfo->reg = reg;
        ainfo->field = INVALIDf;
        ainfo->cos = -1;
        ainfo->port = -1;
        switch (regtype) {
        case soc_portreg:
            if (SOC_BLOCK_IN_LIST(SOC_REG_INFO(unit, reg).block, portblktype)) {
                for (phy_port = 0; ; phy_port++) {
                    if (soc_feature(unit, soc_feature_logical_port_num)) {
                        port = SOC_INFO(unit).port_p2l_mapping[phy_port];
                        if (port < 0) {
                            continue;
                        }
                    } else {
                        port = phy_port;
                    }
                    blk = SOC_PORT_INFO(unit, phy_port).blk;
                    bindex = SOC_PORT_INFO(unit, phy_port).bindex;
                    if (blk < 0 && bindex < 0) {        /* end of list */
                        break;
                    }
                    if (blk < 0) {                      /* empty slot */
                        continue;
                    }
                    
                    if ((uint32)blk == ainfo->block && bindex == pindex) {
                        ainfo->port = port;
                        break;
                    }
                }
                if (ainfo->port == -1) {
                    ainfo->reg = INVALIDr;
                }
            } else {
                ainfo->port = pindex;
            }
            break;
        case soc_cosreg:
            if (pindex >= 0 && pindex <= NUM_COS(unit)) {
                ainfo->cos = pindex;
            } else {
                ainfo->reg = INVALIDr;
            }
            break;
        case soc_genreg:
            if (pindex != 0) {
                ainfo->reg = INVALIDr;
            }
            break;
        default:
            assert(0);
        }
        break;
    }
}

/*
 * Given an address, fill in an regaddrinfo structure
 * This can only be called for SOC registers, not for CPU registers
 */
void
soc_regaddrinfo_extended_get(int unit, soc_regaddrinfo_t *ainfo, 
                             soc_block_t block, uint32 addr)
{
    int                 i;
    int                 blk, bindex, port, phy_port, cmic;
    uint32              minadr, maxadr;
    soc_regtype_t       regtype;
    int                 reg;
    uint32              offset;         /* base address */
    int                 pindex;         /* port/cos index field */
    int                 aindex;         /* array index field */
    soc_block_t         portblktype;

    ainfo->addr = addr;
    ainfo->valid = 1;
    ainfo->reg = INVALIDr;
    portblktype = SOC_BLK_PORT;
    pindex = addr & 0xff;
    if (addr & 0x02000000) {
        offset = addr;
    } else {
        offset = addr & 0xffffff00;
    }

    /* find the matching block */
    /* we find the last matching block to make things work with cmic/cpic */
    blk = -1;
    for (i = 0; SOC_BLOCK_INFO(unit, i).type >= 0; i++) {
        cmic = SOC_BLOCK_INFO(unit, i).cmic;
        if (cmic == block) {
            blk = i;
            /* break; */
        }
    }
    assert(blk >= 0);

    for (reg = 0; reg < NUM_SOC_REG; reg++) {
        if (!SOC_REG_IS_VALID(unit, reg)) {
            continue;
        }
        regtype = SOC_REG_INFO(unit, reg).regtype;
        minadr = SOC_REG_INFO(unit, reg).offset;
        if (offset < minadr) {
            continue;
        }
        if (SOC_REG_ARRAY(unit, reg)) {
	    if (SOC_REG_ARRAY2(unit, reg)) {
		maxadr = minadr +
		    ((2 * (SOC_REG_GRAN(unit, reg) * 
                     (SOC_REG_NUMELS(unit, reg) - 1))) << 8);
	    } else {
		maxadr = minadr +
		    (((SOC_REG_GRAN(unit, reg) * 
                     (SOC_REG_NUMELS(unit, reg) - 1))) << 8);
	    }
        } else {
            maxadr = minadr;
        }
        if (offset > maxadr) {
            continue;
        }
	if (SOC_REG_ARRAY2(unit, reg)) {
	    if ( (offset & 0x100) != (minadr & 0x100) ) {
		/* make sure last bit are same as the base addr when array stride is 2 */
		continue;
	    }
	}
        if ((regtype == soc_cpureg) || (regtype == soc_mcsreg))  {
            continue;
        }
        if (!SOC_BLOCK_IS_TYPE(unit, blk, SOC_REG_INFO(unit, reg).block)) {
            continue;
        }
        if (SOC_REG_ARRAY(unit, reg)) {
            aindex = (offset - SOC_REG_BASE(unit, reg)) >> 8; 
	    if (SOC_REG_ARRAY2(unit, reg)) {
		ainfo->idx = aindex / (2 * SOC_REG_GRAN(unit, reg));
	    } else {
		ainfo->idx = aindex / SOC_REG_GRAN(unit, reg);
	    }
        } else {
            ainfo->idx = -1;
        }
        ainfo->block = blk;
        ainfo->reg = reg;
        ainfo->field = INVALIDf;
        ainfo->cos = -1;
        ainfo->port = -1;
        switch (regtype) {
        case soc_portreg:
            if (SOC_BLOCK_IN_LIST(SOC_REG_INFO(unit, reg).block, portblktype)) {
                for (phy_port = 0; ; phy_port++) {
                    if (soc_feature(unit, soc_feature_logical_port_num)) {
                        port = SOC_INFO(unit).port_p2l_mapping[phy_port];
                        if (port < 0) {
                            continue;
                        }
                    } else {
                        port = phy_port;
                    }
                    blk = SOC_PORT_INFO(unit, phy_port).blk;
                    bindex = SOC_PORT_INFO(unit, phy_port).bindex;
                    if (blk < 0 && bindex < 0) {        /* end of list */
                        break;
                    }
                    if (blk < 0) {                      /* empty slot */
                        continue;
                    }
                    
                    if ((uint32)blk == ainfo->block && bindex == pindex) {
                        ainfo->port = port;
                        break;
                    }
                }
                if (ainfo->port == -1) {
                    ainfo->reg = INVALIDr;
                }
            } else {
                ainfo->port = pindex;
            }
            break;
        case soc_cosreg:
            if (pindex >= 0 && pindex <= NUM_COS(unit)) {
                ainfo->cos = pindex;
            } else {
                ainfo->reg = INVALIDr;
            }
            break;
        case soc_genreg:
            if (pindex != 0) {
                ainfo->reg = INVALIDr;
            }
            break;
        default:
            assert(0);
        }
        break;
    }
}

#endif /* defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) */
