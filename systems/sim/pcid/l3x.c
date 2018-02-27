/* 
 * $Id: l3x.c 1.43 Broadcom SDK $
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
 * File:        l3x.c
 *
 * Provides:
 *      soc_internal_l3x_read
 *      soc_internal_l3x_write
 *      soc_internal_l3x_init
 *      soc_internal_l3x_del
 *      soc_internal_l3x_ins
 *
 * Requires:    
 */

#include <soc/mem.h>
#include <soc/hash.h>
#include <soc/drv.h>
#ifdef BCM_EASYRIDER_SUPPORT
#include <soc/easyrider.h>
#include <soc/er_cmdmem.h>
#endif /* BCM_EASYRIDER_SUPPORT */
#include <soc/l3x.h>

#include "pcid.h"
#include "mem.h"
#include "cmicsim.h"

#ifdef BCM_XGS_SWITCH_SUPPORT
/* 
 * Draco L3 Table Simulation
 *
 * The L3 table is hashed.
 *
 * L3X is a "virtual" table which aggregates the
 *      L3X_VALID | L3X_HIT | L3X_BASE
 * tables.  It is readable, but not writable by the chip, and is
 * also used in L3 insert/delete S-Channel operations.
 */

int
soc_internal_l3x_read(pcid_info_t * pcid_info, uint32 addr,
                      l3x_entry_t *entry)
{
    uint32          ent_v[SOC_MAX_MEM_WORDS];
    uint32          ent_h[SOC_MAX_MEM_WORDS];
    uint32          ent_b[SOC_MAX_MEM_WORDS];
    int             index = (addr & 0x3fff);
    int             bucket = index / SOC_L3X_BUCKET_SIZE(pcid_info->unit);
    int             bit = (index % SOC_L3X_BUCKET_SIZE(pcid_info->unit));
    int             bmask;

    debugk(DK_VERBOSE, "L3X read addr=0x%x\n", addr);

    bmask = soc_mem_index_max(pcid_info->unit, L3_INTFm);
    soc_internal_read_mem(pcid_info, soc_mem_addr(pcid_info->unit,
                                       L3X_VALIDm, 0, bucket), ent_v);
    soc_internal_read_mem(pcid_info,
                          soc_mem_addr(pcid_info->unit, L3X_HITm, 0, bucket),
                          ent_h);
    soc_internal_read_mem(pcid_info,
                          soc_mem_addr(pcid_info->unit, L3X_BASEm, 0, index),
                          ent_b);

    entry->entry_data[3] = ent_b[3] & bmask;
    entry->entry_data[2] = ent_b[2];
    entry->entry_data[1] = ent_b[1];
    entry->entry_data[0] = ent_b[0];

    soc_mem_field32_set(pcid_info->unit, L3Xm, entry, L3_VALIDf,
                        (ent_v[0] >> bit) & 1);
    soc_mem_field32_set(pcid_info->unit, L3Xm, entry, L3_HITf,
                        (ent_h[0] >> bit) & 1);

    return 0;
}

int
soc_internal_l3x_write(pcid_info_t * pcid_info, uint32 addr,
                       l3x_entry_t *entry)
{
    uint32          ent_v[SOC_MAX_MEM_WORDS];
    uint32          ent_h[SOC_MAX_MEM_WORDS];
    uint32          ent_b[SOC_MAX_MEM_WORDS];
    int             index = (addr & 0x3fff);
    int             bucket = index / SOC_L3X_BUCKET_SIZE(pcid_info->unit);
    int             bit = (index % SOC_L3X_BUCKET_SIZE(pcid_info->unit));
    int             bmask;

    debugk(DK_VERBOSE, "L3X write addr=0x%x\n", addr);

    bmask = soc_mem_index_max(pcid_info->unit, L3_INTFm);
    soc_internal_read_mem(pcid_info, soc_mem_addr(pcid_info->unit,
                                       L3X_VALIDm, 0, bucket), ent_v);
    soc_internal_read_mem(pcid_info,
                          soc_mem_addr(pcid_info->unit, L3X_HITm, 0, bucket),
                          ent_h);
    soc_internal_read_mem(pcid_info,
                          soc_mem_addr(pcid_info->unit, L3X_BASEm, 0, index),
                          ent_b);

    if (soc_mem_field32_get(pcid_info->unit, L3Xm, entry, L3_VALIDf)) {
        ent_v[0] |= 1 << bit;
    } else {
        ent_v[0] &= ~(1 << bit);
    }

    if (soc_mem_field32_get(pcid_info->unit, L3Xm, entry, L3_HITf)) {
        ent_h[0] |= 1 << bit;
    } else {
        ent_h[0] &= ~(1 << bit);
    }

    ent_b[3] = entry->entry_data[3] & bmask;
    ent_b[2] = entry->entry_data[2];
    ent_b[1] = entry->entry_data[1];
    ent_b[0] = entry->entry_data[0];

    soc_internal_write_mem(pcid_info,
                 soc_mem_addr(pcid_info->unit, L3X_VALIDm, 0, bucket), ent_v);
    soc_internal_write_mem(pcid_info,
                 soc_mem_addr(pcid_info->unit, L3X_HITm, 0, bucket), ent_h);
    soc_internal_write_mem(pcid_info,
                 soc_mem_addr(pcid_info->unit, L3X_BASEm, 0, index), ent_b);

    return 0;
}

int
soc_internal_l3x_init(pcid_info_t * pcid_info)
{
    int             index, index_min, index_max;

    debugk(DK_VERBOSE, "L3X Init\n");

    index_min = soc_mem_index_min(pcid_info->unit, L3X_VALIDm);
    index_max = soc_mem_index_max(pcid_info->unit, L3X_VALIDm);

    for (index = index_min; index <= index_max; index++) {
        soc_internal_write_mem(pcid_info,
                       soc_mem_addr(pcid_info->unit, L3X_VALIDm, 0, index),
                       soc_mem_entry_null(pcid_info->unit, L3X_VALIDm));
    }

    return 0;
}

#define ARL_BLOCK_LOCAL 0

int
soc_internal_l3x_sync(pcid_info_t *pcid_info)
{
    uint32              regval[SOC_MAX_MEM_WORDS];
    uint32              ipmc_config;
    int                 port;
    uint32              pte[SOC_MAX_MEM_WORDS];
    int                 unit;

    unit = pcid_info->unit;

    /* get the first enabled port */
    port = -1;
    PBMP_E_ITER(unit, port) {
        break;
    }
    ipmc_config = 0;

    if (port < 0) {
        /* no enable ports, leave everything disabled */
    } else if (SOC_IS_DRACO1(unit)) {
        soc_internal_read_reg(pcid_info,
                              soc_reg_addr(unit, CONFIGr, port, 0),
                              regval);
        if (soc_reg_field_get(unit, CONFIGr, regval[0], IPMC_ENABLEf)) {
            ipmc_config |= L3X_IPMC_ENABLE;
        }
        if (soc_reg_field_get(unit, CONFIGr, regval[0], SRC_IP_CFGf)) {
            ipmc_config |= L3X_IPMC_SIP0;
        } else {
            ipmc_config |= L3X_IPMC_SIP;
        }
    } else if (SOC_IS_DRACO15(unit)) {
        soc_internal_read_mem(pcid_info,
                              soc_mem_addr(unit, PORT_TABm, ARL_BLOCK_LOCAL,
                                           port),
                              pte);
        if (soc_PORT_TABm_field32_get(unit, &pte, IPMC_ENABLEf)) {
            ipmc_config |= L3X_IPMC_ENABLE;
        }
        if (soc_PORT_TABm_field32_get(unit, &pte, IPMC_DO_VLANf)) {
            ipmc_config |= L3X_IPMC_VLAN;
        }
        ipmc_config |= L3X_IPMC_SIP | L3X_IPMC_SIP0;
    } else if (SOC_IS_LYNX(unit)) {
        soc_internal_read_reg(pcid_info,
                              soc_reg_addr(unit, CONFIGr, port, 0),
                              regval);
        if (soc_reg_field_get(unit, CONFIGr, regval[0], IPMC_ENABLEf)) {
            ipmc_config |= L3X_IPMC_ENABLE;
        }
        soc_internal_read_reg(pcid_info,
                              soc_reg_addr(unit, ARL_CONTROLr, port, 0),
                              regval);
        if (soc_reg_field_get(unit, ARL_CONTROLr, regval[0],
                              PIM_DM_SM_ENABLEf)) {
            ipmc_config |= L3X_IPMC_SIP | L3X_IPMC_SIP0;
        } else if (soc_reg_field_get(unit, ARL_CONTROLr, regval[0],
                                     SRC_IP_CFGf)) {
            ipmc_config |= L3X_IPMC_SIP0;
        } else {
            ipmc_config |= L3X_IPMC_SIP;
        }
        if (soc_feature(unit, soc_feature_l3_sgv)) { /* (S,G,V) */
            if (soc_reg_field_get(unit, ARL_CONTROLr, regval[0],
                                  IPMC_DO_VLANf)) {
                ipmc_config |= L3X_IPMC_VLAN;
            }
        }
    } else if (SOC_IS_TUCANA(unit)) {
        if (port & 0x20) {
            soc_internal_read_mem(pcid_info,
                                  soc_mem_addr(unit, PORT_TAB1m, 
                                               ARL_BLOCK_LOCAL, port & 0x1f),
                                  pte);
        } else {
            soc_internal_read_mem(pcid_info,
                                  soc_mem_addr(unit, PORT_TABm, 
                                               ARL_BLOCK_LOCAL, port),
                                  pte);
        }
        if (soc_PORT_TABm_field32_get(unit, &pte, IPMC_ENABLEf)) {
            ipmc_config |= L3X_IPMC_ENABLE;
        }
        if (soc_feature(unit, soc_feature_l3_sgv)) { /* (S,G,V) */
            if (soc_PORT_TABm_field32_get(unit, &pte, IPMC_DO_VLANf)) {
                ipmc_config |= L3X_IPMC_VLAN;
            }
        }
        ipmc_config |= L3X_IPMC_SIP | L3X_IPMC_SIP0;
    }

    ipmc_config |= 0x8000000;   /* "valid" bit */
    SOC_CONTROL(unit)->hash_key_config = ipmc_config;
    return 0;
}

int
soc_internal_l3x_hash(pcid_info_t *pcid_info, l3x_entry_t *entry)
{
    uint32      regval[SOC_MAX_MEM_WORDS];
    int         hash_sel;
    int         ipmc, key_src_ip;
    int         index;
    uint8       key[XGS_HASH_KEY_SIZE];

    debugk(DK_VERBOSE, "L3X hash\n");

    (void)soc_internal_l3x_sync(pcid_info);

    soc_internal_read_reg(pcid_info,
                          soc_reg_addr(pcid_info->unit, HASH_CONTROLr, REG_PORT_ANY, 0),
                          regval);
    hash_sel = soc_reg_field_get(pcid_info->unit, HASH_CONTROLr, regval[0],
                          HASH_SELECTf);

    debugk(DK_VERBOSE, "hash_sel %d\n", hash_sel);

    key_src_ip = (SOC_CONTROL(pcid_info->unit)->hash_key_config &
        L3X_IPMC_SIP0);

    debugk(DK_VERBOSE, "Source Ip address in key: %c\n",
           key_src_ip ? 'f' : 't');

    soc_draco_l3x_base_entry_to_key(pcid_info->unit, entry, key_src_ip, key);
    ipmc = soc_L3Xm_field32_get(pcid_info->unit, entry, IPMCf);
    index = soc_draco_l3_hash(pcid_info->unit, hash_sel, ipmc, key);

    debugk(DK_VERBOSE, "index %d\n", index);

    return index;
}

int
soc_internal_l3x_ins(pcid_info_t * pcid_info, l3x_entry_t *entry)
{
    uint32          valid_ent[SOC_MAX_MEM_WORDS];
    uint32          tmp[SOC_MAX_MEM_WORDS];
    int             index = 0, bucket, slot, bmask;

    debugk(DK_VERBOSE, "L3X Insert\n");

    bmask = soc_mem_index_max(pcid_info->unit, L3X_VALIDm);
    bucket = soc_internal_l3x_hash(pcid_info, entry) & bmask;

    debugk(DK_VERBOSE, "bucket %d bmask %d\n", bucket, bmask);

    soc_internal_read_mem(pcid_info,
                          soc_mem_addr(pcid_info->unit, L3X_VALIDm,
                                       0, bucket), valid_ent);

    /* Check if it should overwrite an existing entry */

    for (slot = 0; slot < SOC_L3X_BUCKET_SIZE(pcid_info->unit); slot++) {
        if ((valid_ent[0] & (1 << slot)) != 0) {
            index = bucket * SOC_L3X_BUCKET_SIZE(pcid_info->unit) + slot;
            soc_internal_read_mem(pcid_info,
                                  soc_mem_addr(pcid_info->unit, L3Xm,
                                               0, index), tmp);
            if (soc_mem_compare_key(pcid_info->unit, L3Xm, entry, tmp) == 0) {
                goto overwrite;
            }
        }
    }

    /* Find first unused slot in bucket */

    for (slot = 0; slot < SOC_L3X_BUCKET_SIZE(pcid_info->unit); slot++) {
        if ((valid_ent[0] & (1 << slot)) == 0) {
            index = bucket * SOC_L3X_BUCKET_SIZE(pcid_info->unit) + slot;
            goto overwrite;
        }
    }

    debugk(DK_VERBOSE, "Bucket full\n");

    PCIM(pcid_info, CMIC_SCHAN_CTRL) |= SC_MSG_NAK_TST;
    return 0;

 overwrite:
    PCIM(pcid_info, CMIC_SCHAN_CTRL) &= ~SC_MSG_NAK_TST;

    debugk(DK_VERBOSE, "write slot %d, index %d\n", slot, index);

    /* The valid bit is always set by hardware */

    soc_mem_field32_set(pcid_info->unit, L3Xm, entry, L3_VALIDf, 1);

    soc_internal_l3x_write(pcid_info, soc_mem_addr(pcid_info->unit,
                                                   L3Xm, 0, index), entry);

    return 0;
}

int
soc_internal_l3x_del(pcid_info_t * pcid_info, l3x_entry_t *entry)
{
    uint32          valid_ent[SOC_MAX_MEM_WORDS];
    uint32          tmp[SOC_MAX_MEM_WORDS];
    int             index = 0, bucket, slot, bmask;

    debugk(DK_VERBOSE, "L3X Delete\n");

    bmask = soc_mem_index_max(pcid_info->unit, L3X_VALIDm);
    bucket = soc_internal_l3x_hash(pcid_info, entry) & bmask;

    debugk(DK_VERBOSE, "bucket %d bmask %d\n", bucket, bmask);

    soc_internal_read_mem(pcid_info,
                          soc_mem_addr(pcid_info->unit, L3X_VALIDm,
                                       0, bucket), valid_ent);

    for (slot = 0; slot < SOC_L3X_BUCKET_SIZE(pcid_info->unit); slot++) {
        if ((valid_ent[0] & (1 << slot)) != 0) {
            index = bucket * SOC_L3X_BUCKET_SIZE(pcid_info->unit) + slot;

            soc_internal_read_mem(pcid_info,
                                  soc_mem_addr(pcid_info->unit, L3Xm,
                                               0, index), tmp);

            if (soc_mem_compare_key(pcid_info->unit, L3Xm, entry, tmp) == 0) {
                debugk(DK_VERBOSE,
                       "invalidate slot %d, index %d\n", slot, index);
                soc_L3Xm_field32_set(pcid_info->unit, (void *)tmp,
                                     L3_VALIDf, 0);
                soc_internal_l3x_write(pcid_info,
                                       soc_mem_addr(pcid_info->unit, L3Xm,
                                       0, index), (void *)tmp);
                return 0;
            }
        }
    }

    debugk(DK_VERBOSE, "Not found\n");

    return 0;
}

#ifdef BCM_FIREBOLT_SUPPORT
int
soc_internal_l3x2_hash(pcid_info_t *pcid_info, uint32 *entry, int dual)
{
    uint32          tmp_hs[SOC_MAX_MEM_WORDS];
    int             hash_sel;
    uint8           at, key[SOC_MAX_MEM_WORDS * 4];
    int             index;
    int             key_nbits;
    soc_block_t     blk;
    debugk(DK_VERBOSE, "L3X2 hash\n");

    if (dual) {
        soc_internal_extended_read_reg(pcid_info, blk, at,
              soc_reg_addr_get(pcid_info->unit, L3_AUX_HASH_CONTROLr,
                           REG_PORT_ANY, 0, &blk, &at),
              tmp_hs);
        hash_sel = soc_reg_field_get(pcid_info->unit, L3_AUX_HASH_CONTROLr,
                                     tmp_hs[0], HASH_SELECTf);
    } else {
        soc_internal_extended_read_reg(pcid_info, blk, at,
            soc_reg_addr_get(pcid_info->unit, HASH_CONTROLr, REG_PORT_ANY, 0, &blk, &at),
                              tmp_hs);
        hash_sel = soc_reg_field_get(pcid_info->unit, HASH_CONTROLr,
                                     tmp_hs[0], L3_HASH_SELECTf);
    }

    debugk(DK_VERBOSE, "hash_sel%s %d\n", dual ? "" : "(aux)", hash_sel);

#ifdef BCM_TRX_SUPPORT
    if (SOC_MEM_FIELD_VALID(pcid_info->unit, L3_ENTRY_ONLYm, KEY_TYPEf)) {
        key_nbits = soc_fb_l3x_base_entry_to_key(pcid_info->unit, entry, key);
        index = soc_fb_l3_hash(pcid_info->unit, hash_sel, key_nbits, key);
    } else 
#endif
    {
        key_nbits = soc_fb_l3x_base_entry_to_key(pcid_info->unit, entry, key);
        index = soc_fb_l3_hash(pcid_info->unit, hash_sel, key_nbits, key);
    }

    if (SOC_IS_FIREBOLT2(pcid_info->unit)) {
        uint32 addr_mask;
        soc_internal_read_reg(pcid_info,
              soc_reg_addr(pcid_info->unit, L3_ENTRY_ADDR_MASKr,
                           REG_PORT_ANY, 0),
              tmp_hs);
        addr_mask = soc_reg_field_get(pcid_info->unit, L3_ENTRY_ADDR_MASKr,
                                      tmp_hs[0], MASKf);
        index &= addr_mask;
    }

    debugk(DK_VERBOSE, "index %d\n", index);

    return index;
}

/* 
 * Firebolt L3 Table Simulation
 *
 * The L3 table is hashed.
 *
 * L3_ENTRY_IPV4_UNICAST,L3_ENTRY_IPV4_MULTICAST,
 * L3_ENTRY_IPV6_UNICAST, L3_ENTRY_IPV6_MULTICAST
 * are "virtual" table which aggregates the
 * L3_ENTRY_HIT_ONLY | L3_ENTRY_ONLY
 * tables.
 */
enum {
        ENT3f,
        ENT2f,
        ENT1f,
        ENT0f
} soc_L3_ENTRY_DUMMYm_ftype;

static soc_field_info_t soc_L3_ENTRY_DUMMYm_fields[] = {
    { ENT3f, 96, 288, SOCF_LE },
    { ENT2f, 96, 192, SOCF_LE },
    { ENT1f, 96, 96, SOCF_LE },
    { ENT0f, 96, 0, SOCF_LE },
};
static soc_mem_info_t l3_minfo = {
                        /* mem            L3_XXXm */
                        /* flags       */ 0, 
                        /* cmp_fn      */ _soc_mem_cmp_undef,
                        /* *null_entry */ _soc_mem_entry_null_zeroes,
                        /* index_min   */ 0,
                        /* index_max   */ 0,
                        /* minblock    */ 0,
                        /* maxblock    */ 0,
                        /* blocks      */ 0,
                        /* blocks_hi   */ 0,
                        /* base        */ 0,
                        /* gran        */ 1,
                        /* bytes       */ 25,
                        /* nFields     */ 4, /* Used */
                        /* *fields     */ soc_L3_ENTRY_DUMMYm_fields, /* Used */
                        };

#define     L3_ENTRY_IS_ALIGNED(o, a) (((o) % (a)) == 0)

int
soc_internal_l3x2_read(pcid_info_t * pcid_info, soc_mem_t mem, uint32 addr,
                      uint32 *entry)
{
    int         unit;
    int         offset;
    uint32      ent[SOC_MAX_MEM_WORDS];
    int         bit;
    int         bucket;
    int         index;
    int 	bucket_size;
    int         blk;
    soc_block_t sblk;
    uint8       at;

    unit = pcid_info->unit;
    offset = (addr ^ SOC_MEM_INFO(unit, mem).base) & 0xfffff;

    memset(entry, 0, 4 * soc_mem_entry_words(unit, mem));
    memset(ent, 0, 4 * soc_mem_entry_words(unit, mem));

    bucket_size = L3_BUCKET_SIZE(unit);

    /*
     * Fixup l3_minfo using info from L3_ENTRY_ONLYm table
     */    
    soc_L3_ENTRY_DUMMYm_fields[3].len = soc_L3_ENTRY_DUMMYm_fields[2].len = 
    soc_L3_ENTRY_DUMMYm_fields[1].len = soc_L3_ENTRY_DUMMYm_fields[0].len = 
                            soc_mem_entry_bits(unit, L3_ENTRY_ONLYm);
    soc_L3_ENTRY_DUMMYm_fields[3].bp = soc_L3_ENTRY_DUMMYm_fields[0].len * 0;
    soc_L3_ENTRY_DUMMYm_fields[2].bp = soc_L3_ENTRY_DUMMYm_fields[0].len * 1;
    soc_L3_ENTRY_DUMMYm_fields[1].bp = soc_L3_ENTRY_DUMMYm_fields[0].len * 2;
    soc_L3_ENTRY_DUMMYm_fields[0].bp = soc_L3_ENTRY_DUMMYm_fields[0].len * 3;

    /*
     * We could assert on V6 and IPMC bits depending which table
     * is being read/written. But that won't be good for diags.
     */    
    switch(mem) {
        case L3_ENTRY_IPV4_MULTICASTm:
#ifdef BCM_TRIUMPH_SUPPORT
            if (SOC_IS_TR_VL(unit)) {
                /* Uses two slots per entry */
                index = offset * 2;
                bucket = index / bucket_size;
                bit = (index % bucket_size);
                blk = SOC_MEM_BLOCK_ANY(unit, L3_ENTRY_ONLYm);
                sblk = SOC_BLOCK2SCH(unit, blk);
                soc_internal_extended_read_mem(pcid_info, sblk, at,
                          soc_mem_addr_get(unit, L3_ENTRY_ONLYm,
                                           blk, index + 0, &at), entry);

                soc_internal_extended_read_mem(pcid_info, sblk, at,
                          soc_mem_addr_get(unit, L3_ENTRY_ONLYm,
                                           blk, index + 1, &at), ent);
                soc_meminfo_field_set(mem, &l3_minfo, entry, ENT1f, ent);

                blk = SOC_MEM_BLOCK_ANY(unit, L3_ENTRY_HIT_ONLYm);
                sblk = SOC_BLOCK2SCH(unit, blk);
                soc_internal_extended_read_mem(pcid_info, sblk, at,
                          soc_mem_addr_get(unit, L3_ENTRY_HIT_ONLYm,
                                           blk, bucket, &at), ent);
                soc_mem_field32_set(unit, L3_ENTRY_IPV4_MULTICASTm, entry,
                                    HIT_0f, (ent[0] >> (bit + 0)) & 1);
                soc_mem_field32_set(unit, L3_ENTRY_IPV4_MULTICASTm, entry,
                                    HIT_1f, (ent[0] >> (bit + 1)) & 1);
                break;
            }
#endif
            /* fallthrough */
        case L3_ENTRY_IPV4_UNICASTm:
            /* Uses one slot per entry */
            index = offset;
            bucket = index / bucket_size;
            bit = (index % bucket_size);
            blk = SOC_MEM_BLOCK_ANY(unit, L3_ENTRY_ONLYm);
            sblk = SOC_BLOCK2SCH(unit, blk);
            soc_internal_extended_read_mem(pcid_info, sblk, at,
                      soc_mem_addr_get(unit, L3_ENTRY_ONLYm,
                                       blk, index + 0, &at), entry);
            blk = SOC_MEM_BLOCK_ANY(unit, L3_ENTRY_HIT_ONLYm);
            sblk = SOC_BLOCK2SCH(unit, blk);
            soc_internal_extended_read_mem(pcid_info, sblk, at,
                      soc_mem_addr_get(unit, L3_ENTRY_HIT_ONLYm,
                                       blk, bucket, &at), ent);
            soc_mem_field32_set(unit, L3_ENTRY_IPV4_UNICASTm, entry,
                                HITf, (ent[0] >> bit) & 1);
                
            break;
        case L3_ENTRY_IPV6_UNICASTm:
            /* Uses two slots per entry */
            index = offset * 2;
            bucket = index / bucket_size;
            bit = (index % bucket_size);
            blk = SOC_MEM_BLOCK_ANY(unit, L3_ENTRY_ONLYm);
            sblk = SOC_BLOCK2SCH(unit, blk);
            soc_internal_extended_read_mem(pcid_info, sblk, at,
                      soc_mem_addr_get(unit, L3_ENTRY_ONLYm,
                                       blk, index + 0, &at), entry);

            soc_internal_extended_read_mem(pcid_info, sblk, at,
                      soc_mem_addr_get(unit, L3_ENTRY_ONLYm,
                                       blk, index + 1, &at), ent);
            soc_meminfo_field_set(mem, &l3_minfo, entry, ENT1f, ent);

            blk = SOC_MEM_BLOCK_ANY(unit, L3_ENTRY_HIT_ONLYm);
            sblk = SOC_BLOCK2SCH(unit, blk);
            soc_internal_extended_read_mem(pcid_info, sblk, at,
                      soc_mem_addr_get(unit, L3_ENTRY_HIT_ONLYm,
                                       blk, bucket, &at), ent);
            soc_mem_field32_set(unit, L3_ENTRY_IPV6_UNICASTm, entry,
                                HIT_0f, (ent[0] >> (bit + 0)) & 1);
            soc_mem_field32_set(unit, L3_ENTRY_IPV6_UNICASTm, entry,
                                HIT_1f, (ent[0] >> (bit + 1)) & 1);
            break;
        case L3_ENTRY_IPV6_MULTICASTm:
            /* Uses four slots per entry */
            index = offset * 4;
            bucket = index / bucket_size;
            bit = (index % bucket_size);
            blk = SOC_MEM_BLOCK_ANY(unit, L3_ENTRY_ONLYm);
            sblk = SOC_BLOCK2SCH(unit, blk);
            soc_internal_extended_read_mem(pcid_info, sblk, at,
                      soc_mem_addr_get(unit, L3_ENTRY_ONLYm,
                                       blk, index + 0, &at), entry);

            soc_internal_extended_read_mem(pcid_info, sblk, at,
                      soc_mem_addr_get(unit, L3_ENTRY_ONLYm,
                                       blk, index + 1, &at), ent);
            soc_meminfo_field_set(mem, &l3_minfo, entry, ENT1f, ent);

            soc_internal_extended_read_mem(pcid_info, sblk, at,
                      soc_mem_addr_get(unit, L3_ENTRY_ONLYm,
                                       blk, index + 2, &at), ent);
            soc_meminfo_field_set(mem, &l3_minfo, entry, ENT2f, ent);

            soc_internal_extended_read_mem(pcid_info, sblk, at,
                      soc_mem_addr_get(unit, L3_ENTRY_ONLYm,
                                       blk, index + 3, &at), ent);
            soc_meminfo_field_set(mem, &l3_minfo, entry, ENT3f, ent);

            blk = SOC_MEM_BLOCK_ANY(unit, L3_ENTRY_HIT_ONLYm);
            sblk = SOC_BLOCK2SCH(unit, blk);
            soc_internal_extended_read_mem(pcid_info, sblk, at,
                      soc_mem_addr_get(unit, L3_ENTRY_HIT_ONLYm,
                                       blk, bucket, &at), ent);
            soc_mem_field32_set(unit, L3_ENTRY_IPV6_MULTICASTm, entry,
                                HIT_0f, (ent[0] >> (bit + 0)) & 1);
            soc_mem_field32_set(unit, L3_ENTRY_IPV6_MULTICASTm, entry,
                                HIT_1f, (ent[0] >> (bit + 1)) & 1);
            soc_mem_field32_set(unit, L3_ENTRY_IPV6_MULTICASTm, entry,
                                HIT_2f, (ent[0] >> (bit + 2)) & 1);
            soc_mem_field32_set(unit, L3_ENTRY_IPV6_MULTICASTm, entry,
                                HIT_3f, (ent[0] >> (bit + 3)) & 1);
            break;
        default: return(-1);
    }
    return(0);
}

int
soc_internal_l3x2_write(pcid_info_t * pcid_info, soc_mem_t mem, uint32 addr,
                       uint32 *entry)
{
    int         unit;
    int         offset;
    uint32      ent[SOC_MAX_MEM_WORDS];
    int         bit;
    int         bucket;
    int         index;
    int         hit;
    int		bucket_size;
    int         blk;
    soc_block_t sblk;
    uint8       at = 0;

    unit = pcid_info->unit;
    offset = (addr ^ SOC_MEM_INFO(unit, mem).base) & 0xfffff;
    memset(ent, 0, 4 * soc_mem_entry_words(unit, mem));

    bucket_size = L3_BUCKET_SIZE(unit);

    /*
     * Fixup l3_minfo using info from L3_ENTRY_ONLYm table
     */    
    soc_L3_ENTRY_DUMMYm_fields[3].len = soc_L3_ENTRY_DUMMYm_fields[2].len = 
    soc_L3_ENTRY_DUMMYm_fields[1].len = soc_L3_ENTRY_DUMMYm_fields[0].len = 
                            soc_mem_entry_bits(unit, L3_ENTRY_ONLYm);
    soc_L3_ENTRY_DUMMYm_fields[3].bp = soc_L3_ENTRY_DUMMYm_fields[0].len * 0;
    soc_L3_ENTRY_DUMMYm_fields[2].bp = soc_L3_ENTRY_DUMMYm_fields[0].len * 1;
    soc_L3_ENTRY_DUMMYm_fields[1].bp = soc_L3_ENTRY_DUMMYm_fields[0].len * 2;
    soc_L3_ENTRY_DUMMYm_fields[0].bp = soc_L3_ENTRY_DUMMYm_fields[0].len * 3;

    /*
     * We could assert on V6 and IPMC bits depending which table
     * is being read/written. But that won't be good for diags.
     */    
    switch(mem) {
        case L3_ENTRY_IPV4_MULTICASTm:
#ifdef BCM_TRIUMPH_SUPPORT
            if (SOC_IS_TR_VL(unit)) {
                /* Uses two slots per entry */
                index = offset * 2;
                bucket = index / bucket_size;
                bit = (index % bucket_size);
                blk = SOC_MEM_BLOCK_ANY(unit, L3_ENTRY_ONLYm);
                sblk = SOC_BLOCK2SCH(unit, blk);
                soc_meminfo_field_get(mem, &l3_minfo, entry, ENT0f, ent);
                soc_mem_field32_set(unit, L3_ENTRY_IPV4_UNICASTm, ent, HITf, 0);
                soc_internal_extended_write_mem(pcid_info, sblk, at,
                          soc_mem_addr_get(unit, L3_ENTRY_ONLYm,
                                           blk, index + 0, &at), ent);

                soc_meminfo_field_get(mem, &l3_minfo, entry, ENT1f, ent);
                soc_mem_field32_set(unit, L3_ENTRY_IPV4_UNICASTm, ent, HITf, 0);
                soc_internal_extended_write_mem(pcid_info, sblk, at,
                          soc_mem_addr_get(unit, L3_ENTRY_ONLYm,
                                           blk, index + 1, &at), ent);
    
                blk = SOC_MEM_BLOCK_ANY(unit, L3_ENTRY_HIT_ONLYm);
                sblk = SOC_BLOCK2SCH(unit, blk);
                soc_internal_extended_read_mem(pcid_info, sblk, at,
                          soc_mem_addr_get(unit, L3_ENTRY_HIT_ONLYm,
                                           blk, bucket, &at), ent);
                ent[0] &= (~((0x3 << (bit))));
                hit = soc_mem_field32_get(unit, L3_ENTRY_IPV4_MULTICASTm,
                                          entry, HIT_0f);
                ent[0] |= ((hit & 1) << (bit + 0));
                hit = soc_mem_field32_get(unit, L3_ENTRY_IPV4_MULTICASTm,
                                          entry, HIT_1f);
                ent[0] |= ((hit & 1) << (bit + 1));
                soc_internal_extended_write_mem(pcid_info, sblk, at,
                          soc_mem_addr_get(unit, L3_ENTRY_HIT_ONLYm,
                                           blk, bucket, &at), ent);

                break;
            }
#endif
            /* fallthrough */
        case L3_ENTRY_IPV4_UNICASTm:
            /* Uses one slot per entry */
            index = offset;
            bucket = index / bucket_size;
            bit = (index % bucket_size);
            blk = SOC_MEM_BLOCK_ANY(unit, L3_ENTRY_HIT_ONLYm);
            sblk = SOC_BLOCK2SCH(unit, blk);
            soc_internal_extended_read_mem(pcid_info, sblk, at,
                      soc_mem_addr_get(unit, L3_ENTRY_HIT_ONLYm,
                                       blk, bucket, &at), ent);
            ent[0] &= (~((0x1 << (bit))));
            hit = soc_mem_field32_get(unit, L3_ENTRY_IPV4_UNICASTm,
                                      entry, HITf);
            ent[0] |= ((hit  & 1) << (bit));
            soc_internal_extended_write_mem(pcid_info, sblk, at,
                      soc_mem_addr_get(unit, L3_ENTRY_HIT_ONLYm,
                                       blk, bucket, &at), ent);
                
            blk = SOC_MEM_BLOCK_ANY(unit, L3_ENTRY_ONLYm);
            sblk = SOC_BLOCK2SCH(unit, blk);
            soc_mem_field32_set(unit, L3_ENTRY_IPV4_UNICASTm, entry, HITf, 0);
            soc_internal_extended_write_mem(pcid_info, sblk, at,
                      soc_mem_addr_get(unit, L3_ENTRY_ONLYm,
                                       blk, index + 0, &at), entry);
            break;
        case L3_ENTRY_IPV6_UNICASTm:
            /* Uses two slots per entry */
            index = offset * 2;
            bucket = index / bucket_size;
            bit = (index % bucket_size);
            blk = SOC_MEM_BLOCK_ANY(unit, L3_ENTRY_ONLYm);
            sblk = SOC_BLOCK2SCH(unit, blk);
            soc_meminfo_field_get(mem, &l3_minfo, entry, ENT0f, ent);
            soc_mem_field32_set(unit, L3_ENTRY_IPV4_UNICASTm, ent, HITf, 0);
            soc_internal_extended_write_mem(pcid_info, sblk, at,
                      soc_mem_addr_get(unit, L3_ENTRY_ONLYm,
                                       blk, index + 0, &at), ent);

            soc_meminfo_field_get(mem, &l3_minfo, entry, ENT1f, ent);
            soc_mem_field32_set(unit, L3_ENTRY_IPV4_UNICASTm, ent, HITf, 0);
            soc_internal_extended_write_mem(pcid_info, sblk, at,
                      soc_mem_addr_get(unit, L3_ENTRY_ONLYm,
                                       blk, index + 1, &at), ent);

            blk = SOC_MEM_BLOCK_ANY(unit, L3_ENTRY_HIT_ONLYm);
            sblk = SOC_BLOCK2SCH(unit, blk);
            soc_internal_extended_read_mem(pcid_info, sblk, at,
                      soc_mem_addr_get(unit, L3_ENTRY_HIT_ONLYm,
                                       blk, bucket, &at), ent);
            ent[0] &= (~((0x3 << (bit))));
            hit = soc_mem_field32_get(unit, L3_ENTRY_IPV6_UNICASTm,
                                      entry, HIT_0f);
            ent[0] |= ((hit & 1) << (bit + 0));
            hit = soc_mem_field32_get(unit, L3_ENTRY_IPV6_UNICASTm,
                                      entry, HIT_1f);
            ent[0] |= ((hit & 1) << (bit + 1));
            soc_internal_extended_write_mem(pcid_info, sblk, at,
                      soc_mem_addr_get(unit, L3_ENTRY_HIT_ONLYm,
                                       blk, bucket, &at), ent);

            break;
        case L3_ENTRY_IPV6_MULTICASTm:
            /* Uses four slots per entry */
            index = offset * 4;
            bucket = index / bucket_size;
            bit = (index % bucket_size);
            blk = SOC_MEM_BLOCK_ANY(unit, L3_ENTRY_ONLYm);
            sblk = SOC_BLOCK2SCH(unit, blk);
            soc_meminfo_field_get(mem, &l3_minfo, entry, ENT0f, ent);
            soc_mem_field32_set(unit, L3_ENTRY_IPV4_UNICASTm, ent, HITf, 0);
            soc_internal_extended_write_mem(pcid_info, sblk, at,
                      soc_mem_addr_get(unit, L3_ENTRY_ONLYm,
                                       blk, index + 0, &at), ent);

            soc_meminfo_field_get(mem, &l3_minfo, entry, ENT1f, ent);
            soc_mem_field32_set(unit, L3_ENTRY_IPV4_UNICASTm, ent, HITf, 0);
            soc_internal_extended_write_mem(pcid_info, sblk, at,
                      soc_mem_addr_get(unit, L3_ENTRY_ONLYm,
                                       blk, index + 1, &at), ent);

            soc_meminfo_field_get(mem, &l3_minfo, entry, ENT2f, ent);
            soc_mem_field32_set(unit, L3_ENTRY_IPV4_UNICASTm, ent, HITf, 0);
            soc_internal_extended_write_mem(pcid_info, sblk, at,
                      soc_mem_addr_get(unit, L3_ENTRY_ONLYm,
                                       blk, index + 2, &at), ent);

            soc_meminfo_field_get(mem, &l3_minfo, entry, ENT3f, ent);
            soc_mem_field32_set(unit, L3_ENTRY_IPV4_UNICASTm, ent, HITf, 0);
            soc_internal_extended_write_mem(pcid_info, sblk, at,
                      soc_mem_addr_get(unit, L3_ENTRY_ONLYm,
                                       blk, index + 3, &at), ent);

            blk = SOC_MEM_BLOCK_ANY(unit, L3_ENTRY_HIT_ONLYm);
            sblk = SOC_BLOCK2SCH(unit, blk);
            soc_internal_extended_read_mem(pcid_info, sblk, at,
                      soc_mem_addr_get(unit, L3_ENTRY_HIT_ONLYm,
                                       blk, bucket, &at), ent);
            hit = soc_mem_field32_get(unit, L3_ENTRY_IPV6_MULTICASTm,
                                      entry, HIT_0f);
            ent[0] &= (~((0xf << (bit))));
            ent[0] |= ((hit & 1) << (bit + 0));
            hit = soc_mem_field32_get(unit, L3_ENTRY_IPV6_MULTICASTm,
                                      entry, HIT_1f);
            ent[0] |= ((hit & 1) << (bit + 1));
            hit = soc_mem_field32_get(unit, L3_ENTRY_IPV6_MULTICASTm,
                                      entry, HIT_2f);
            ent[0] |= ((hit & 1) << (bit + 2));
            hit = soc_mem_field32_get(unit, L3_ENTRY_IPV6_MULTICASTm,
                                      entry, HIT_3f);
            ent[0] |= ((hit & 1) << (bit + 3));
            soc_internal_extended_write_mem(pcid_info, sblk, at,
                      soc_mem_addr_get(unit, L3_ENTRY_HIT_ONLYm,
                                       blk, bucket, &at), ent);
            break;
        default: return(-1);
    }
    return(0);
}

void
soc_internal_l3x2_dual_banks(pcid_info_t *pcid_info, uint8 banks,
                             int *dual, int *slot_min, int *slot_max)
{
    uint32          tmp[SOC_MAX_MEM_WORDS];
    int		    bucket_size; 
    soc_block_t     blk;
    uint8           at; 

    bucket_size = L3_BUCKET_SIZE(pcid_info->unit);

    soc_internal_extended_read_reg(pcid_info, blk, at,
                          soc_reg_addr_get(pcid_info->unit, L3_AUX_HASH_CONTROLr,
                                           REG_PORT_ANY, 0, &blk, &at),
                          tmp);
    *dual = soc_reg_field_get(pcid_info->unit, L3_AUX_HASH_CONTROLr,
                              tmp[0], ENABLEf);

    if (*dual) {
        switch (banks) {
        case 0:
            return; /* Nothing to do here */
        case 1:
            *slot_min = bucket_size / 2;
            *slot_max = bucket_size - 1;
            break;
        case 2:
            *slot_min = 0;
            *slot_max = bucket_size / 2 - 1;
            break;
        default:
            *slot_min = 0;
            *slot_max = -1;
            break;
        }
    }

    return;
}

int
soc_internal_l3x2_entry_ins(pcid_info_t *pcid_info, uint8 banks,
                            l3_entry_ipv6_multicast_entry_t *entry,
                            uint32 *result)
{
    uint32          tmp[SOC_MAX_MEM_WORDS];
    uint32          ent[SOC_MAX_MEM_WORDS];
    int             index = 0, bucket, slot, free_index;
    int             slot_min = 0, slot_max = 7, dual = FALSE;
    int             unit = pcid_info->unit; 
    int             num_vbits; 
    int             freeslot_cnt; 
    int             validslot_cnt; 
    int             sub_slot; 
    soc_mem_t       mem; 
    int             ss_field[] = {ENT0f, ENT1f, ENT2f, ENT3f};
    schan_genresp_t *genresp = (schan_genresp_t *) result;
    int		    bucket_size;
    int             blk;
    soc_block_t     sblk;
    uint8           at;

    debugk(DK_VERBOSE, "L3_ENTRY Insert\n");

    /* Initialization. */
    sal_memset(ent, 0, SOC_MAX_MEM_WORDS * sizeof(uint32));
    sal_memset(tmp, 0, SOC_MAX_MEM_WORDS * sizeof(uint32));

    bucket_size = L3_BUCKET_SIZE(unit);
    slot_max = bucket_size - 1;

#ifdef BCM_TRX_SUPPORT
    if (SOC_MEM_FIELD_VALID(unit, L3_ENTRY_ONLYm, KEY_TYPEf)) {
        int key_type;

        key_type = soc_mem_field32_get(unit, L3_ENTRY_ONLYm,
                                       entry, KEY_TYPEf);
        switch (key_type) {
            case TR_L3_HASH_KEY_TYPE_V4UC:
                num_vbits = 1;
                mem = L3_ENTRY_IPV4_UNICASTm;
                break;
            case TR_L3_HASH_KEY_TYPE_V4MC:
                num_vbits = SOC_IS_TR_VL(unit) ? 2 : 1;
                mem = L3_ENTRY_IPV4_MULTICASTm;
                break;
            case TR_L3_HASH_KEY_TYPE_V6UC:
                num_vbits = 2;
                mem = L3_ENTRY_IPV6_UNICASTm;
                break;
            case TR_L3_HASH_KEY_TYPE_V6MC:
                num_vbits = 4;
                mem = L3_ENTRY_IPV6_MULTICASTm;
                break;
            case TR_L3_HASH_KEY_TYPE_LMEP:
                num_vbits = 1;
                mem = L3_ENTRY_IPV4_UNICASTm;
                break;
            case TR_L3_HASH_KEY_TYPE_RMEP:
                num_vbits = 1;
                mem = L3_ENTRY_IPV4_UNICASTm;
                break;
            case TR_L3_HASH_KEY_TYPE_TRILL:
                num_vbits = 1;
                mem = L3_ENTRY_ONLYm;
                break;
            default:
                return -1;
        }
    } else
#endif
    if (soc_mem_field32_get(unit, L3_ENTRY_IPV4_UNICASTm, entry, V6f)) {
        if (soc_mem_field32_get(unit, L3_ENTRY_IPV4_UNICASTm, entry, IPMCf)) {
            mem = L3_ENTRY_IPV6_MULTICASTm;
            num_vbits = 4;
        } else {
            mem = L3_ENTRY_IPV6_UNICASTm;
            num_vbits = 2;
        }
    } else {
        num_vbits = 1;
        if (soc_mem_field32_get(unit, L3_ENTRY_IPV4_UNICASTm, entry, IPMCf)) {
            mem = L3_ENTRY_IPV4_MULTICASTm;
        } else {
            mem = L3_ENTRY_IPV4_UNICASTm;
        }
    }

    bucket = soc_internal_l3x2_hash(pcid_info, (uint32 *)entry, FALSE);

    debugk(DK_VERBOSE, "bucket %d\n", bucket);

    if (soc_feature(pcid_info->unit, soc_feature_dual_hash)) {
        soc_internal_l3x2_dual_banks(pcid_info, banks,
                                     &dual, &slot_min, &slot_max);
    }

    /* Check if it should overwrite an existing entry */
    free_index = -1;
    for (slot = slot_min; slot <= slot_max; slot += num_vbits) {
        freeslot_cnt = 0;
        validslot_cnt = 0;
        if (dual && (slot == bucket_size / 2)) {
            bucket = soc_internal_l3x2_hash(pcid_info, (uint32 *)entry, TRUE);
        }
        blk = SOC_MEM_BLOCK_ANY(unit, L3_ENTRY_ONLYm);
        sblk = SOC_BLOCK2SCH(unit, blk);
        for (sub_slot = 0; sub_slot < num_vbits; sub_slot++) {
            index = bucket * bucket_size + slot + sub_slot;
            soc_internal_extended_read_mem(pcid_info, sblk, at,
                                  soc_mem_addr_get(unit, L3_ENTRY_ONLYm,
                                  blk, index, &at), tmp);
            if (soc_mem_field32_get(unit, L3_ENTRY_ONLYm, tmp, VALIDf)) {
                soc_meminfo_field_set(mem, &l3_minfo, ent, ss_field[sub_slot],
                                      tmp);
                validslot_cnt++; 
            } else {
                freeslot_cnt++; 
            }
        }
        if  ((validslot_cnt == num_vbits) && 
             (_soc_mem_cmp_l3x2(unit, entry, ent) == 0)) {

            /* Overwrite the existing entry */
            PCIM(pcid_info, CMIC_SCHAN_CTRL) &= ~SC_MSG_NAK_TST;

            index /= num_vbits;
            debugk(DK_VERBOSE, "write bucket %d, slot %d, index %d\n",
                   (index / (bucket_size / num_vbits)), (index % 
						(bucket_size / num_vbits)), index);
            blk = SOC_MEM_BLOCK_ANY(unit, mem);
            sblk = SOC_BLOCK2SCH(unit, blk);
            soc_internal_extended_write_mem(pcid_info, sblk, at,
                                            soc_mem_addr_get(unit, mem,
                                            blk, index, &at), (uint32 *)entry);

            if (soc_feature(unit, soc_feature_generic_table_ops)) {
                genresp->type = SCHAN_GEN_RESP_TYPE_REPLACED;
                genresp->index = index;
                /* Copy old entry immediately after response word */
                memcpy(&result[1], ent, soc_mem_entry_bytes(unit, mem));
            }
            return 0;
        }
        if  ((freeslot_cnt == num_vbits) && (free_index == -1)) {
            free_index = bucket * bucket_size + slot;
        }
    }

    /* Find first unused slot in bucket */
    if (free_index != -1) {
        index = free_index;
        index /= num_vbits;

        /* Write the existing entry */
        if (soc_feature(unit, soc_feature_generic_table_ops)) {
            genresp->type = SCHAN_GEN_RESP_TYPE_INSERTED;
            genresp->index = index;
        }
        PCIM(pcid_info, CMIC_SCHAN_CTRL) &= ~SC_MSG_NAK_TST;

        debugk(DK_VERBOSE, "write bucket %d, slot %d, index %d\n",
               (index / (bucket_size / num_vbits)), (index % (bucket_size / num_vbits)), index);
        blk = SOC_MEM_BLOCK_ANY(unit, mem);
        sblk = SOC_BLOCK2SCH(unit, blk);
        soc_internal_extended_write_mem(pcid_info, sblk, at, 
                                        soc_mem_addr_get(unit, mem,
                                        blk, index, &at), (uint32 *)entry);
    } else {
        debugk(DK_VERBOSE, "Bucket full\n");

        if (soc_feature(unit, soc_feature_generic_table_ops)) {
            genresp->type = SCHAN_GEN_RESP_TYPE_FULL;
        }

        PCIM(pcid_info, CMIC_SCHAN_CTRL) |= SC_MSG_NAK_TST;
    }
    return 0;
}

int
soc_internal_l3x2_entry_del(pcid_info_t *pcid_info, uint8 banks,
                            l3_entry_ipv6_multicast_entry_t *entry,
                            uint32 *result)
{
    uint32          tmp[SOC_MAX_MEM_WORDS];
    uint32          ent[SOC_MAX_MEM_WORDS];
    int             index = 0, bucket, slot;
    int             slot_min = 0, slot_max = 7, dual = FALSE;
    int             unit = pcid_info->unit; 
    int             num_vbits; 
    int             validslot_cnt; 
    int             sub_slot; 
    soc_mem_t       mem; 
    int             ss_field[] = {ENT0f, ENT1f, ENT2f, ENT3f};
    schan_genresp_t *genresp = (schan_genresp_t *) result;
    int 	    bucket_size;
    int             blk;
    soc_block_t     sblk;
    uint8           at;

    debugk(DK_VERBOSE, "L3_ENTRY Delete\n");

    /* Initialization. */
    sal_memset(ent, 0, SOC_MAX_MEM_WORDS * sizeof(uint32));
    sal_memset(tmp, 0, SOC_MAX_MEM_WORDS * sizeof(uint32));

    bucket_size = L3_BUCKET_SIZE(unit);
    slot_max = bucket_size - 1;

#ifdef BCM_TRX_SUPPORT 
    if (SOC_MEM_FIELD_VALID(unit, L3_ENTRY_ONLYm, KEY_TYPEf)) {
        int key_type;

        key_type = soc_mem_field32_get(unit, L3_ENTRY_ONLYm,
                                       entry, KEY_TYPEf);
        switch (key_type) {
            case TR_L3_HASH_KEY_TYPE_V4UC:
                num_vbits = 1;
                mem = L3_ENTRY_IPV4_UNICASTm;
                break;
            case TR_L3_HASH_KEY_TYPE_V4MC:
                num_vbits = SOC_IS_TR_VL(unit) ? 2 : 1;
                mem = L3_ENTRY_IPV4_MULTICASTm;
                break;
            case TR_L3_HASH_KEY_TYPE_V6UC:
                num_vbits = 2;
                mem = L3_ENTRY_IPV6_UNICASTm;
                break;
            case TR_L3_HASH_KEY_TYPE_V6MC:
                num_vbits = 4;
                mem = L3_ENTRY_IPV6_MULTICASTm;
                break;
            case TR_L3_HASH_KEY_TYPE_LMEP:
                num_vbits = 1;
                mem = L3_ENTRY_IPV4_UNICASTm;
                break;
            case TR_L3_HASH_KEY_TYPE_RMEP:
                num_vbits = 1;
                mem = L3_ENTRY_IPV4_UNICASTm;
                break;
            case TR_L3_HASH_KEY_TYPE_TRILL:
                num_vbits = 1;
                mem = L3_ENTRY_ONLYm;
                break;
            default:
                return -1;
        }
    } else
#endif
    if (soc_mem_field32_get(unit, L3_ENTRY_IPV4_UNICASTm, entry, V6f)) {
        if (soc_mem_field32_get(unit, L3_ENTRY_IPV4_UNICASTm, entry, IPMCf)) {
            mem = L3_ENTRY_IPV6_MULTICASTm;
            num_vbits = 4;
        } else {
            mem = L3_ENTRY_IPV6_UNICASTm;
            num_vbits = 2;
        }
    } else {
        num_vbits = 1;
        if (soc_mem_field32_get(unit, L3_ENTRY_IPV4_UNICASTm, entry, IPMCf)) {
            mem = L3_ENTRY_IPV4_MULTICASTm;
        } else {
            mem = L3_ENTRY_IPV4_UNICASTm;
        }
    }

    bucket = soc_internal_l3x2_hash(pcid_info, (uint32 *)entry, FALSE);

    debugk(DK_VERBOSE, "bucket %d\n", bucket);

    if (soc_feature(pcid_info->unit, soc_feature_dual_hash)) {
        soc_internal_l3x2_dual_banks(pcid_info, banks,
                                     &dual, &slot_min, &slot_max);
    }

    if (soc_feature(unit, soc_feature_generic_table_ops)) {
        genresp->type = SCHAN_GEN_RESP_TYPE_NOT_FOUND;
    }

    /* Check if it should overwrite an existing entry */
    for (slot = slot_min; slot <= slot_max; slot += num_vbits) {
        validslot_cnt = 0;
        if (dual && (slot == bucket_size / 2)) {
            bucket = soc_internal_l3x2_hash(pcid_info, (uint32 *)entry, TRUE);
        }
        blk = SOC_MEM_BLOCK_ANY(unit, L3_ENTRY_ONLYm);
        sblk = SOC_BLOCK2SCH(unit, blk);
        for (sub_slot = 0; sub_slot < num_vbits; sub_slot++) {
            index = bucket * bucket_size + slot + sub_slot;
            soc_internal_extended_read_mem(pcid_info, sblk, at,
                                  soc_mem_addr_get(unit, L3_ENTRY_ONLYm,
                                  blk, index, &at), tmp);
            if (soc_mem_field32_get(unit, L3_ENTRY_ONLYm, tmp, VALIDf)) {
                soc_meminfo_field_set(L3_ENTRY_ONLYm, &l3_minfo, ent,
                                      ss_field[sub_slot], tmp);
                validslot_cnt++; 
            }
        }
        if  ((validslot_cnt == num_vbits) && 
             (_soc_mem_cmp_l3x2(unit, entry, ent) == 0)) {

            if (soc_feature(unit, soc_feature_generic_table_ops)) {
                genresp->type = SCHAN_GEN_RESP_TYPE_DELETED;
                genresp->index = index;
                /* Copy entry immediately after response word */
                memcpy(&result[1], ent, soc_mem_entry_bytes(unit, mem));
            }

            PCIM(pcid_info, CMIC_SCHAN_CTRL) &= ~SC_MSG_NAK_TST;
            index = bucket * (bucket_size / num_vbits) + (slot / num_vbits);
            debugk(DK_VERBOSE, "write slot %d, index %d\n",
                   (slot / num_vbits), index);
            memset(tmp, 0, sizeof (tmp));
            blk = SOC_MEM_BLOCK_ANY(unit, mem);
            sblk = SOC_BLOCK2SCH(unit, blk);
            soc_internal_extended_write_mem(pcid_info, sblk, 0,
                                            soc_mem_addr(unit, mem,
                                            blk, index), (uint32 *)tmp);

            return 0;
        }
    }

    debugk(DK_VERBOSE, "soc_internal_l3x2_entry_del: Not Found\n");

    PCIM(pcid_info, CMIC_SCHAN_CTRL) |= SC_MSG_NAK_TST;
    return 0;

}
int
soc_internal_l3x2_entry_lkup(pcid_info_t * pcid_info, uint8 banks,
                             l3_entry_ipv6_multicast_entry_t *entry,
                             uint32 *result)
{
    uint32          tmp[SOC_MAX_MEM_WORDS];
    uint32          ent[SOC_MAX_MEM_WORDS];
    int             index = 0, bucket, slot;
    int             slot_min = 0, slot_max = 7, dual = FALSE;
    int             unit = pcid_info->unit; 
    int             num_vbits; 
    int             index_bit_offset; 
    int             validslot_cnt; 
    int             sub_slot; 
    int             entry_dw;
    soc_mem_t       mem; 
    int             ss_field[] = {ENT0f, ENT1f, ENT2f, ENT3f};
    schan_genresp_t *genresp = (schan_genresp_t *) result;
    int 	    bucket_size;	
    int             blk;
    soc_block_t     sblk;
    uint8           at;

    debugk(DK_VERBOSE, "L3_ENTRY Lookup\n");

    /* Initialization. */
    sal_memset(ent, 0, SOC_MAX_MEM_WORDS * sizeof(uint32));
    sal_memset(tmp, 0, SOC_MAX_MEM_WORDS * sizeof(uint32));

    bucket_size = L3_BUCKET_SIZE(unit);
    slot_max = bucket_size - 1;

#ifdef BCM_TRX_SUPPORT 
    if (SOC_MEM_FIELD_VALID(unit, L3_ENTRY_ONLYm, KEY_TYPEf)) {
        int key_type; 

        key_type = soc_mem_field32_get(unit, L3_ENTRY_ONLYm,
                                       entry, KEY_TYPEf);
        switch (key_type) {
            case TR_L3_HASH_KEY_TYPE_V4UC:
                num_vbits = 1;
                mem = L3_ENTRY_IPV4_UNICASTm;
                break;
            case TR_L3_HASH_KEY_TYPE_V4MC:
                num_vbits = SOC_IS_TR_VL(unit) ? 2 : 1;
                mem = L3_ENTRY_IPV4_MULTICASTm;
                break;
            case TR_L3_HASH_KEY_TYPE_V6UC:
                num_vbits = 2;
                mem = L3_ENTRY_IPV6_UNICASTm;
                break;
            case TR_L3_HASH_KEY_TYPE_V6MC:
                num_vbits = 4;
                mem = L3_ENTRY_IPV6_MULTICASTm;
                break;
            case TR_L3_HASH_KEY_TYPE_LMEP:
                num_vbits = 1;
                mem = L3_ENTRY_IPV4_UNICASTm;
                break;
            case TR_L3_HASH_KEY_TYPE_RMEP:
                num_vbits = 1;
                mem = L3_ENTRY_IPV4_UNICASTm;
                break;
            case TR_L3_HASH_KEY_TYPE_TRILL:
                num_vbits = 1;
                mem = L3_ENTRY_ONLYm;
                break;
            default:
                return -1; 
        }
    } else
#endif
    if (soc_mem_field32_get(unit, L3_ENTRY_IPV4_UNICASTm, entry, V6f)) {
        if (soc_mem_field32_get(unit, L3_ENTRY_IPV4_UNICASTm, entry, IPMCf)) {
            mem = L3_ENTRY_IPV6_MULTICASTm;
            num_vbits = 4;
        } else {
            mem = L3_ENTRY_IPV6_UNICASTm;
            num_vbits = 2;
        }
    } else {
        num_vbits = 1;
        if (soc_mem_field32_get(unit, L3_ENTRY_IPV4_UNICASTm, entry, IPMCf)) {
            mem = L3_ENTRY_IPV4_MULTICASTm;
        } else {
            mem = L3_ENTRY_IPV4_UNICASTm;
        }
    }

    index_bit_offset = soc_mem_entry_bits(unit, mem) % 32;

    bucket = soc_internal_l3x2_hash(pcid_info, (uint32 *)entry, FALSE);

    debugk(DK_VERBOSE, "bucket %d\n", bucket);

    if (soc_feature(pcid_info->unit, soc_feature_dual_hash)) {
        soc_internal_l3x2_dual_banks(pcid_info, banks,
                                     &dual, &slot_min, &slot_max);
    }

    /* Check if it should overwrite an existing entry */
    for (slot = slot_min; slot <= slot_max; slot += num_vbits) {
        validslot_cnt = 0;
        if (dual && (slot == bucket_size / 2)) {
            bucket = soc_internal_l3x2_hash(pcid_info, (uint32 *)entry, TRUE);
        }
        blk = SOC_MEM_BLOCK_ANY(unit, L3_ENTRY_ONLYm);
        sblk = SOC_BLOCK2SCH(unit, blk);
        for (sub_slot = 0; sub_slot < num_vbits; sub_slot++) {
            index = bucket * bucket_size + slot + sub_slot;
            soc_internal_extended_read_mem(pcid_info, sblk, at,
                                  soc_mem_addr_get(unit, L3_ENTRY_ONLYm,
                                  blk, index, &at), tmp);
            if (soc_mem_field32_get(unit, L3_ENTRY_ONLYm, tmp, VALIDf)) {
                soc_meminfo_field_set(mem, &l3_minfo, ent, ss_field[sub_slot],
                                      tmp);
                validslot_cnt++; 
            }
        }
        if  ((validslot_cnt == num_vbits) && 
             (_soc_mem_cmp_l3x2(unit, entry, ent) == 0)) {
            PCIM(pcid_info, CMIC_SCHAN_CTRL) &= ~SC_MSG_NAK_TST;
            index = bucket * (bucket_size / num_vbits) + (slot / num_vbits);
            debugk(DK_VERBOSE, "write slot %d, index %d\n",
                   (slot / num_vbits), index);

            blk = SOC_MEM_BLOCK_ANY(unit, mem);
            sblk = SOC_BLOCK2SCH(unit, blk);
            soc_internal_extended_read_mem(pcid_info, sblk, at,
                                  soc_mem_addr_get(unit, mem,
                                  blk, index, &at), ent);
            if (soc_feature(unit, soc_feature_generic_table_ops)) {
                genresp->type = SCHAN_GEN_RESP_TYPE_FOUND;
                genresp->index = index;
                /* Copy entry immediately after response word */
                memcpy(&result[1], ent, soc_mem_entry_bytes(unit, mem));
            } else {
                entry_dw = soc_mem_entry_words(unit, mem);
                for(slot = 0; slot < entry_dw - 1; slot++) {
                    result[slot] = ent[slot];
                }
                result[slot] = (ent[slot] &  ((1 << index_bit_offset) - 1)) |   
                	       ((index & ((1 << (32 - index_bit_offset)) - 1))
				<< index_bit_offset);
		result[slot + 1] = (index >> (32 - index_bit_offset)) &
                                ((soc_mem_index_max(unit, mem)) >> (32 - index_bit_offset));
            }

            return 0;
        }
    }

    debugk(DK_VERBOSE, "soc_internal_l3x2_entry_lkup : Not Found\n");

    if (soc_feature(unit, soc_feature_generic_table_ops)) {
        genresp->type = SCHAN_GEN_RESP_TYPE_NOT_FOUND;
    } else {
        result[0] = 0xffffffff;
    }
    PCIM(pcid_info, CMIC_SCHAN_CTRL) |= SC_MSG_NAK_TST;
    return 0;

}
#endif /* BCM_FIREBOLT_SUPPORT */

#ifdef BCM_EASYRIDER_SUPPORT
int
soc_internal_l3_er_hash(pcid_info_t *pcid_info, int l3v6, uint32 *entry)
{
    uint32          tmp_hs[SOC_MAX_MEM_WORDS];
    uint8           key[ER_L3V6_HASH_KEY_SIZE];
    int             hash_sel, index, mem_cfg;
    int             unit = pcid_info->unit;
    sop_memcfg_er_t *mcer = &(SOC_PERSIST(unit)->er_memcfg);

    debugk(DK_VERBOSE, "L3v%d hash\n", l3v6 ? 6 : 4);

    soc_internal_read_reg(pcid_info,
                          soc_reg_addr(unit, HASH_CONTROLr,
                                       REG_PORT_ANY, 0), tmp_hs);
    hash_sel = soc_reg_field_get(unit, HASH_CONTROLr,
                                 tmp_hs[0], L3_HASH_SELECTf);

    soc_internal_read_reg(pcid_info,
                          soc_reg_addr(unit, L2_ISr,
                                       REG_PORT_ANY, 0), tmp_hs);
    mem_cfg = soc_reg_field_get(unit, L2_ISr, tmp_hs[0], MEM_CONFIGf);

    switch (mem_cfg) {
    case ER_SEER_HOST_HASH_CFG_L2_ALL:
        SOC_CONTROL(unit)->hash_mask_l3x = 0;
        SOC_CONTROL(unit)->hash_bits_l3x = -1;
        SOC_CONTROL(unit)->hash_mask_l3v6 = 0;
        SOC_CONTROL(unit)->hash_bits_l3v6 = -1;
        mcer->l3v4_search_offset = 0;
        mcer->l3v6_search_offset = 0;
        break;

    case ER_SEER_HOST_HASH_CFG_L2_HALF_V4_HALF:
        SOC_CONTROL(unit)->hash_mask_l3x = 0x7ff;
        SOC_CONTROL(unit)->hash_bits_l3x = 12;
        SOC_CONTROL(unit)->hash_mask_l3v6 = 0;
        SOC_CONTROL(unit)->hash_bits_l3v6 = -1;
        mcer->l3v4_search_offset = ER_SEER_16K;
        mcer->l3v6_search_offset = 0;
        break;

    case ER_SEER_HOST_HASH_CFG_L2_HALF_V4_QUART_V6_QUART:
        SOC_CONTROL(unit)->hash_mask_l3x = 0x3ff;
        SOC_CONTROL(unit)->hash_bits_l3x = 12;
        SOC_CONTROL(unit)->hash_mask_l3v6 = 0x3ff;
        SOC_CONTROL(unit)->hash_bits_l3v6 = 12;
        mcer->l3v4_search_offset = ER_SEER_16K;
        mcer->l3v6_search_offset = ER_SEER_12K;
        break;

    case ER_SEER_HOST_HASH_CFG_L2_HALF_V6_HALF:
        SOC_CONTROL(unit)->hash_mask_l3x = 0;
        SOC_CONTROL(unit)->hash_bits_l3x = -1;
        SOC_CONTROL(unit)->hash_mask_l3v6 = 0x7ff;
        SOC_CONTROL(unit)->hash_bits_l3v6 = 12;
        mcer->l3v4_search_offset = 0;
        mcer->l3v6_search_offset = ER_SEER_8K;
        break;

    case ER_SEER_HOST_HASH_CFG_L2_QUART_V4_HALF_V6_QUART:
        SOC_CONTROL(unit)->hash_mask_l3x = 0x7ff;
        SOC_CONTROL(unit)->hash_bits_l3x = 12;
        SOC_CONTROL(unit)->hash_mask_l3v6 = 0x3ff;
        SOC_CONTROL(unit)->hash_bits_l3v6 = 12;
        mcer->l3v4_search_offset = ER_SEER_8K;
        mcer->l3v6_search_offset = ER_SEER_12K;
        break;

    case ER_SEER_HOST_HASH_CFG_L2_QUART_V4_QUART_V6_HALF:
        SOC_CONTROL(unit)->hash_mask_l3x = 0x3ff;
        SOC_CONTROL(unit)->hash_bits_l3x = 12;
        SOC_CONTROL(unit)->hash_mask_l3v6 = 0x7ff;
        SOC_CONTROL(unit)->hash_bits_l3v6 = 12;
        mcer->l3v4_search_offset = ER_SEER_8K;
        mcer->l3v6_search_offset = ER_SEER_8K;
        break;

    case ER_SEER_HOST_HASH_CFG_V4_ALL:
        SOC_CONTROL(unit)->hash_mask_l3x = 0xfff;
        SOC_CONTROL(unit)->hash_bits_l3x = 12;
        SOC_CONTROL(unit)->hash_mask_l3v6 = 0;
        SOC_CONTROL(unit)->hash_bits_l3v6 = -1;
        mcer->l3v4_search_offset = 0;
        mcer->l3v6_search_offset = 0;
        break;

    case ER_SEER_HOST_HASH_CFG_V4_HALF_V6_HALF:
        SOC_CONTROL(unit)->hash_mask_l3x = 0x7ff;
        SOC_CONTROL(unit)->hash_bits_l3x = 12;
        SOC_CONTROL(unit)->hash_mask_l3v6 = 0x7ff;
        SOC_CONTROL(unit)->hash_bits_l3v6 = 12;
        mcer->l3v4_search_offset = 0;
        mcer->l3v6_search_offset = ER_SEER_8K;
        break;

    case ER_SEER_HOST_HASH_CFG_V6_ALL:
        SOC_CONTROL(unit)->hash_mask_l3x = 0;
        SOC_CONTROL(unit)->hash_bits_l3x = -1;
        SOC_CONTROL(unit)->hash_mask_l3v6 = 0xfff;
        SOC_CONTROL(unit)->hash_bits_l3v6 = 12;
        mcer->l3v4_search_offset = 0;
        mcer->l3v6_search_offset = 0;
        break;
       
    default:
        SOC_CONTROL(unit)->hash_mask_l3x = 0;
        SOC_CONTROL(unit)->hash_bits_l3x = -1;
        SOC_CONTROL(unit)->hash_mask_l3v6 = 0;
        SOC_CONTROL(unit)->hash_bits_l3v6 = -1;
        mcer->l3v4_search_offset = 0;
        mcer->l3v6_search_offset = 0;
        break;
    }

    if (l3v6) {
        soc_er_l3v6_base_entry_to_key(unit, (uint32 *)entry, key);
        index = soc_er_l3v6_hash(pcid_info->unit, hash_sel, key);
    } else {
        soc_er_l3v4_base_entry_to_key(unit, (uint32 *)entry, key);
        index = soc_er_l3v4_hash(pcid_info->unit, hash_sel, key);
    }

    debugk(DK_VERBOSE, "hash_sel %d\n", hash_sel);
    debugk(DK_VERBOSE, "index %d\n", index);

    return index;
}

int
soc_internal_l3_er_entry_scan_bucket(pcid_info_t *pcid_info,
                                     int l3v6, int bucket,
                                     uint32 *entry, int *free_slot)
{
    uint32          tmp[SOC_MAX_MEM_WORDS];
    soc_mem_t       tmem;
    int             index = 0, bucket_size, slot;
    int             rindex = -1, found_free = FALSE;
    int             unit = pcid_info->unit;

    if (l3v6) {
        tmem = L3_ENTRY_V6m;
        bucket_size = SOC_ER_L3V6_BUCKET_SIZE;
    } else {
        tmem = L3_ENTRY_V4m;
        bucket_size = SOC_L3X_BUCKET_SIZE(unit);
    }

    /* Check if it should overwrite an existing entry */
    for (slot = 0; slot < bucket_size; slot++) {
        index = bucket * bucket_size + slot;
        soc_internal_read_mem(pcid_info, soc_mem_addr(unit, tmem,
                                         SOC_MEM_BLOCK_ANY(unit, tmem),
                                         index), tmp);
        if ((rindex < 0) && soc_mem_field32_get(unit, tmem, tmp, VALIDf)) {
            if (soc_mem_compare_key(unit, tmem, entry, tmp) == 0) {
                rindex = index;
                if (found_free) {
                    break;
                }
            }
        } else {
            if (!found_free) {
                *free_slot = index;
                found_free = TRUE;
                if (rindex >= 0) {
                    break;
                }
            }
        }
    }

    if (!found_free) {
        *free_slot = -1;
    }

    return rindex;
}

void
soc_internal_l3_er_index_to_cm_addr(int unit, int l3v6, uint32 *entry,
                                    uint32 index)
{
    sop_memcfg_er_t *mcer = &(SOC_PERSIST(unit)->er_memcfg);
    int             cmindex;

    if (l3v6) {
        cmindex = (index + mcer->l3v6_search_offset);
        cmindex = (cmindex & SOC_MEM_CMD_EXT_L2_ADDR_MASK_LO) |
            ((cmindex << SOC_MEM_CMD_EXT_L2_ADDR_SHIFT_HI) &
             SOC_MEM_CMD_EXT_L2_ADDR_MASK_HI);
    } else {
        cmindex = (index + mcer->l3v4_search_offset);
    }

    cmindex |= (SOC_MEM_CMD_HASH_TABLE_DFLT << SOC_MEM_CMD_HASH_TABLE_SHIFT);
    soc_mem_field32_set(unit, COMMAND_MEMORY_HSEm, entry,
                            ADDRESS_0f, cmindex);
}

int
soc_internal_l3_er_entry_ins(pcid_info_t *pcid_info, uint32 *entry, int l3v6)
{
    int             index, bucket, free_slot;
    int             unit = pcid_info->unit; 
    soc_mem_t       tmem;

    debugk(DK_VERBOSE, "l3_entry_v%d Insert\n", l3v6 ? 6 : 4);

    index = -1;
    free_slot = -1;
    tmem = l3v6 ? L3_ENTRY_V6m : L3_ENTRY_V4m;

    bucket = soc_internal_l3_er_hash(pcid_info, l3v6, (uint32 *)entry);
    if (bucket >= 0) {
        index =
            soc_internal_l3_er_entry_scan_bucket(pcid_info, l3v6, bucket,
                                                 (uint32 *)entry, &free_slot);
    }
        
    if ((index < 0) && (free_slot < 0)) {
        debugk(DK_VERBOSE, "All target hash buckets full\n");
        soc_mem_field32_set(unit, COMMAND_MEMORY_HSEm, entry, TAB_FULLf, 1);
    } else {
        if (index < 0) {
            index = free_slot;
        }
        debugk(DK_VERBOSE, "write table %s, index %d\n",
               SOC_MEM_NAME(unit, tmem), index);

        /* The valid bit is always set by hardware */
        soc_mem_field32_set(unit, tmem, entry, VALIDf, 1);
        soc_internal_write_mem(pcid_info, soc_mem_addr(unit, tmem,
                                          SOC_MEM_BLOCK_ANY(unit, tmem),
                                          index), (uint32 *)entry);

        soc_internal_l3_er_index_to_cm_addr(unit, l3v6, entry, index);
    }

    soc_mem_field32_set(unit, COMMAND_MEMORY_HSEm, entry, NEWCMDf, 0);

    return 0;
}

int
soc_internal_l3_er_entry_del(pcid_info_t *pcid_info, uint32 *entry, int l3v6)
{
    int             index, bucket, free_slot;
    int             unit = pcid_info->unit; 
    soc_mem_t       tmem;

    debugk(DK_VERBOSE, "l3_entry_v%d Delete\n", l3v6 ? 6 : 4);

    index = -1;
    tmem = l3v6 ? L3_ENTRY_V6m : L3_ENTRY_V4m;

    bucket = soc_internal_l3_er_hash(pcid_info, l3v6, (uint32 *)entry);
    if (bucket >= 0) {
        index =
            soc_internal_l3_er_entry_scan_bucket(pcid_info, l3v6, bucket,
                                                 (uint32 *)entry, &free_slot);
    }
        
    if (index < 0) {
        debugk(DK_VERBOSE, "Entry not foundl\n");
        soc_mem_field32_set(unit, COMMAND_MEMORY_HSEm, entry, NOT_FOUNDf, 1);
    } else {
        debugk(DK_VERBOSE, "write table %s, index %d\n",
               SOC_MEM_NAME(unit, tmem), index);

        /* The valid bit is always set by hardware */
        soc_mem_field32_set(unit, tmem, entry, VALIDf, 0);
        soc_internal_write_mem(pcid_info, soc_mem_addr(unit, tmem,
                                          SOC_MEM_BLOCK_ANY(unit, tmem),
                                          index), (uint32 *)entry);

        soc_internal_l3_er_index_to_cm_addr(unit, l3v6, entry, index);
    }

    soc_mem_field32_set(unit, COMMAND_MEMORY_HSEm, entry, NEWCMDf, 0);

    return 0;
}

int
soc_internal_l3_er_entry_lkup(pcid_info_t *pcid_info, uint32 *entry, int l3v6)
{
    int             index, bucket, free_slot;
    int             unit = pcid_info->unit; 
    soc_mem_t       tmem;

    debugk(DK_VERBOSE, "l3_entry_v%d Lookup\n", l3v6 ? 6 : 4);

    index = -1;
    tmem = l3v6 ? L3_ENTRY_V6m : L3_ENTRY_V4m;

    bucket = soc_internal_l3_er_hash(pcid_info, l3v6, (uint32 *)entry);
    if (bucket >= 0) {
        index =
            soc_internal_l3_er_entry_scan_bucket(pcid_info, l3v6, bucket,
                                                 (uint32 *)entry, &free_slot);
    }
        
    if (index < 0) {
        debugk(DK_VERBOSE, "Entry not foundl\n");
        soc_mem_field32_set(unit, COMMAND_MEMORY_HSEm, entry, NOT_FOUNDf, 1);
    } else {
        debugk(DK_VERBOSE, "read table %s, index %d\n",
               SOC_MEM_NAME(unit, tmem), index);

        soc_internal_read_mem(pcid_info, soc_mem_addr(unit, tmem,
                                          SOC_MEM_BLOCK_ANY(unit, tmem),
                                          index), (uint32 *)entry);

        soc_internal_l3_er_index_to_cm_addr(unit, l3v6, entry, index);
    }

    soc_mem_field32_set(unit, COMMAND_MEMORY_HSEm, entry, NEWCMDf, 0);

    return 0;
}
#endif /* BCM_EASYRIDER_SUPPORT */

#endif
