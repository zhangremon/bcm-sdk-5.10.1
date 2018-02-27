/* 
 * $Id: l2x.c 1.30 Broadcom SDK $
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
 * File:        l2x.c
 *      
 * Provides:
 *      soc_internal_l2x_read
 *      soc_internal_l2x_write
 *      soc_internal_l2x_lkup
 *      soc_internal_l2x_init
 *      soc_internal_l2x_del
 *      soc_internal_l2x_ins
 *      
 * Requires:    
 */

#include <soc/mem.h>
#include <soc/hash.h>
#include <soc/drv.h>
#include <soc/l2x.h>
#ifdef BCM_EASYRIDER_SUPPORT
#include <soc/easyrider.h>
#include <soc/er_cmdmem.h>
#endif /* BCM_EASYRIDER_SUPPORT */

#include "pcid.h"
#include "mem.h"
#include "cmicsim.h"

#ifdef BCM_XGS_SWITCH_SUPPORT
/* 
 * Draco L2 Table Simulation
 *
 * The L2 table is hashed.
 *
 * L2X is a "virtual" table which aggregates the
 *      L2X_VALID | L2X_HIT | L2X_STATIC | L2X_BASE
 * tables.  It is readable, but not writable by the chip, and is
 * also used in L2 insert/delete S-Channel operations.
 */

int
soc_internal_l2x_read(pcid_info_t * pcid_info, uint32 addr,
                      l2x_entry_t *entry)
{
    uint32          ent_v[SOC_MAX_MEM_WORDS];
    uint32          ent_h[SOC_MAX_MEM_WORDS];
    uint32          ent_s[SOC_MAX_MEM_WORDS];
    uint32          ent_b[SOC_MAX_MEM_WORDS];
    int             index = (addr & 0x3fff);
    int             bucket = index / 8;
    int             bit = (index % 8);

    debugk(DK_VERBOSE, "L2X read addr=0x%x\n", addr);

    soc_internal_read_mem(pcid_info,
                          soc_mem_addr(pcid_info->unit, L2X_VALIDm,
                                       0, bucket), ent_v);
    soc_internal_read_mem(pcid_info,
                          soc_mem_addr(pcid_info->unit, L2X_HITm, 0, bucket),
                          ent_h);
    soc_internal_read_mem(pcid_info,
                          soc_mem_addr(pcid_info->unit, L2X_STATICm,
                                       0, bucket), ent_s);
    soc_internal_read_mem(pcid_info,
                          soc_mem_addr(pcid_info->unit, L2X_BASEm, 0, index),
                          ent_b);

    entry->entry_data[2] = ent_b[2] & 0x3fffff;
    entry->entry_data[1] = ent_b[1];
    entry->entry_data[0] = ent_b[0];

    soc_mem_field32_set(pcid_info->unit, L2Xm, entry, VALID_BITf,
                        (ent_v[0] >> bit) & 1);
    soc_mem_field32_set(pcid_info->unit, L2Xm, entry, HIT_BITf,
                        (ent_h[0] >> bit) & 1);
    soc_mem_field32_set(pcid_info->unit, L2Xm, entry, STATIC_BITf,
                        (ent_s[0] >> bit) & 1);

    return 0;
}

int
soc_internal_l2x_write(pcid_info_t * pcid_info, uint32 addr,
                       l2x_entry_t *entry)
{
    uint32          ent_v[SOC_MAX_MEM_WORDS];
    uint32          ent_h[SOC_MAX_MEM_WORDS];
    uint32          ent_s[SOC_MAX_MEM_WORDS];
    uint32          ent_b[SOC_MAX_MEM_WORDS];
    int             index = (addr & 0x3fff);
    int             bucket = index / 8;
    int             bit = (index % 8);

    debugk(DK_VERBOSE, "L2X write addr=0x%x\n", addr);

    soc_internal_read_mem(pcid_info,
                          soc_mem_addr(pcid_info->unit, L2X_VALIDm,
                                       0, bucket), ent_v);
    soc_internal_read_mem(pcid_info,
                          soc_mem_addr(pcid_info->unit, L2X_HITm, 0, bucket),
                          ent_h);
    soc_internal_read_mem(pcid_info,
                          soc_mem_addr(pcid_info->unit, L2X_STATICm,
                                       0, bucket), ent_s);
    soc_internal_read_mem(pcid_info,
                          soc_mem_addr(pcid_info->unit, L2X_BASEm, 0, index),
                          ent_b);

    if (soc_mem_field32_get(pcid_info->unit, L2Xm, entry, VALID_BITf)) {
        ent_v[0] |= 1 << bit;
    } else {
        ent_v[0] &= ~(1 << bit);
    }

    if (soc_mem_field32_get(pcid_info->unit, L2Xm, entry, HIT_BITf)) {
        ent_h[0] |= 1 << bit;
    } else {
        ent_h[0] &= ~(1 << bit);
    }

    if (soc_mem_field32_get(pcid_info->unit, L2Xm, entry, STATIC_BITf)) {
        ent_s[0] |= 1 << bit;
    } else {
        ent_s[0] &= ~(1 << bit);
    }

    ent_b[2] = entry->entry_data[2] & 0x3fffff;
    ent_b[1] = entry->entry_data[1];
    ent_b[0] = entry->entry_data[0];

    soc_internal_write_mem(pcid_info, soc_mem_addr(pcid_info->unit,
                                    L2X_VALIDm, 0, bucket), ent_v);
    soc_internal_write_mem(pcid_info,
                           soc_mem_addr(pcid_info->unit, L2X_HITm, 0, bucket),
                           ent_h);
    soc_internal_write_mem(pcid_info,
                           soc_mem_addr(pcid_info->unit, L2X_STATICm,
                                        0, bucket), ent_s);
    soc_internal_write_mem(pcid_info,
                           soc_mem_addr(pcid_info->unit, L2X_BASEm,
                                        0, index), ent_b);

    return 0;
}

int
soc_internal_l2x_init(pcid_info_t * pcid_info)
{
    int             index, index_min, index_max;

    debugk(DK_VERBOSE, "L2X Init\n");

    index_min = soc_mem_index_min(pcid_info->unit, L2X_VALIDm);
    index_max = soc_mem_index_max(pcid_info->unit, L2X_VALIDm);

    for (index = index_min; index <= index_max; index++) {
        soc_internal_write_mem(pcid_info,
                       soc_mem_addr(pcid_info->unit, L2X_VALIDm, 0, index),
                       soc_mem_entry_null(pcid_info->unit, L2X_VALIDm));
    }

    return 0;
}

int
soc_internal_l2x_hash(pcid_info_t * pcid_info, l2x_entry_t *entry)
{
    uint32          tmp_hs[SOC_MAX_MEM_WORDS];
    int             hash_sel;
    uint8           key[8];
    int             index;

    debugk(DK_VERBOSE, "L2X hash\n");

    soc_internal_read_reg(pcid_info,
                          soc_reg_addr(pcid_info->unit, HASH_CONTROLr, REG_PORT_ANY, 0),
                          tmp_hs);
    hash_sel = soc_reg_field_get(pcid_info->unit, HASH_CONTROLr,
                                 tmp_hs[0], HASH_SELECTf);

    debugk(DK_VERBOSE, "hash_sel %d\n", hash_sel);

    soc_draco_l2x_base_entry_to_key(pcid_info->unit, entry, key);

    index = soc_draco_l2_hash(pcid_info->unit, hash_sel, key);

    debugk(DK_VERBOSE, "index %d\n", index);

    return index;
}

int
soc_internal_l2x_ins(pcid_info_t *pcid_info, l2x_entry_t *entry)
{
    uint32          valid_ent[SOC_MAX_MEM_WORDS];
    uint32          tmp[SOC_MAX_MEM_WORDS];
    int             index = 0, bucket, slot;

    debugk(DK_VERBOSE, "L2X Insert\n");

    bucket = soc_internal_l2x_hash(pcid_info, entry) & 0x7ff;

    debugk(DK_VERBOSE, "bucket %d\n", bucket);

    soc_internal_read_mem(pcid_info,
                          soc_mem_addr(pcid_info->unit, L2X_VALIDm,
                                       0, bucket), valid_ent);

    /* Check if it should overwrite an existing entry */

    for (slot = 0; slot < 8; slot++) {
        if ((valid_ent[0] & (1 << slot)) != 0) {
            index = bucket * 8 + slot;
            soc_internal_read_mem(pcid_info,
                                  soc_mem_addr(pcid_info->unit, L2Xm,
                                               0, index), tmp);
            if (soc_mem_compare_key(pcid_info->unit, L2Xm, entry, tmp) == 0) {
                goto overwrite;
            }
        }
    }

    /* Find first unused slot in bucket */

    for (slot = 0; slot < 8; slot++) {
        if ((valid_ent[0] & (1 << slot)) == 0) {
            index = bucket * 8 + slot;
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

    soc_mem_field32_set(pcid_info->unit, L2Xm, entry, VALID_BITf, 1);

    soc_internal_l2x_write(pcid_info, soc_mem_addr(pcid_info->unit,
                                                   L2Xm, 0, index), entry);

    return 0;
}

int
soc_internal_l2x_del(pcid_info_t * pcid_info, l2x_entry_t *entry)
{
    uint32          valid_ent[SOC_MAX_MEM_WORDS];
    uint32          tmp[SOC_MAX_MEM_WORDS];
    int             index = 0, bucket, slot;

    debugk(DK_VERBOSE, "L2X Delete\n");

    bucket = soc_internal_l2x_hash(pcid_info, entry) & 0x7ff;

    debugk(DK_VERBOSE, "bucket %d\n", bucket);

    soc_internal_read_mem(pcid_info,
                          soc_mem_addr(pcid_info->unit, L2X_VALIDm,
                                       0, bucket), valid_ent);

    for (slot = 0; slot < 8; slot++) {
        if ((valid_ent[0] & (1 << slot)) != 0) {
            index = bucket * 8 + slot;

            soc_internal_read_mem(pcid_info,
                                  soc_mem_addr(pcid_info->unit, L2Xm,
                                               0, index), tmp);

            if (soc_mem_compare_key(pcid_info->unit, L2Xm, entry, tmp) == 0) {
                debugk(DK_VERBOSE,
                       "invalidate slot %d, index %d\n", slot, index);
                soc_L2Xm_field32_set(pcid_info->unit, (void *)tmp,
                                     VALID_BITf, 0);
                soc_internal_l2x_write(pcid_info,
                               soc_mem_addr(pcid_info->unit, L2Xm, 0, index),
                               (void *)tmp);
                return 0;
            }
        }
    }

    debugk(DK_VERBOSE, "Not found\n");

    return 0;
}

/* 
 * soc_internal_l2x_lkup
 *
 *   Note: entry_lookup and entry_result may overlap.
 */

int
soc_internal_l2x_lkup(pcid_info_t * pcid_info,
                      l2x_entry_t *entry_lookup, uint32 * result)
{
    uint32          valid_ent[SOC_MAX_MEM_WORDS];
    uint32          tmp[SOC_MAX_MEM_WORDS];
    int             index = 0, bucket, slot;

    debugk(DK_VERBOSE, "L2X Lookup\n");

    bucket = soc_internal_l2x_hash(pcid_info, entry_lookup) & 0x7ff;

    debugk(DK_VERBOSE, "bucket %d\n", bucket);

    soc_internal_read_mem(pcid_info,
                  soc_mem_addr(pcid_info->unit, L2X_VALIDm, 0, bucket),
                  valid_ent);

    for (slot = 0; slot < 8; slot++) {
        if ((valid_ent[0] & (1 << slot)) != 0) {
            index = bucket * 8 + slot;
            soc_internal_read_mem(pcid_info,
                          soc_mem_addr(pcid_info->unit, L2Xm, 0, index), tmp);
            if (soc_mem_compare_key(pcid_info->unit, L2Xm,
                                    entry_lookup, tmp) == 0) {
                break;
            }
        }
    }

    if (slot == 8) {
        debugk(DK_VERBOSE, "Not found\n");
        memset(result, 0xff, 8);
        return 0;
    }

    debugk(DK_VERBOSE, "Found at index %d\n", index);

    /* 
     * Format of S-Channel response:
     *      word 0            : Memory Read Ack S-channel header
     *      word 1 = result[0]: { 4'h0, hit, static, l2_table_data }
     *      word 2 = result[1]: { 18'h0, index_into_L2_table[13:0] }
     */

    /* tmp[2] = ZERO<6:0>, VALID, HIT, STATIC ... DST_DISCARD */
    /* tmp[1] = CPU, COS<2:0>, VLAN_ID<11:0>, MAC_ADDR<47:32> */
    /* tmp[0] = MAC_ADDR<31:0> */

    result[0] = ((tmp[2] & 0x00ffffff) << 4 | tmp[1] >> 28);
    result[1] = index;

    return 0;
}

#ifdef BCM_FIREBOLT_SUPPORT
int
soc_internal_l2x2_hash(pcid_info_t * pcid_info, l2x_entry_t *entry, int dual)
{
    uint32          tmp_hs[SOC_MAX_MEM_WORDS];
    int             hash_sel;
    uint8           at, key[8];
    int             index;
    soc_block_t     blk;

    debugk(DK_VERBOSE, "L2X2 hash\n");

    if (dual) {
        soc_internal_extended_read_reg(pcid_info, blk, at,
                soc_reg_addr_get(pcid_info->unit, L2_AUX_HASH_CONTROLr,
                               REG_PORT_ANY, 0, &blk, &at),
                tmp_hs);
        hash_sel = soc_reg_field_get(pcid_info->unit, L2_AUX_HASH_CONTROLr,
                                     tmp_hs[0], HASH_SELECTf);
    } else {
        soc_internal_extended_read_reg(pcid_info, blk, at,
                soc_reg_addr_get(pcid_info->unit, HASH_CONTROLr, REG_PORT_ANY, 
                               0, &blk, &at), 
                tmp_hs);
        hash_sel = soc_reg_field_get(pcid_info->unit, HASH_CONTROLr,
                                     tmp_hs[0], L2_AND_VLAN_MAC_HASH_SELECTf);
    }

    debugk(DK_VERBOSE, "hash_sel%s %d\n", dual ? "" : "(aux)", hash_sel);

    soc_draco_l2x_base_entry_to_key(pcid_info->unit, entry, key);

    index = soc_fb_l2_hash(pcid_info->unit, hash_sel, key);

    if (SOC_IS_FIREBOLT2(pcid_info->unit)) {
        uint32 addr_mask;
        soc_internal_read_reg(pcid_info,
              soc_reg_addr(pcid_info->unit, L2_ENTRY_ADDR_MASKr,
                           REG_PORT_ANY, 0),
              tmp_hs);
        addr_mask = soc_reg_field_get(pcid_info->unit, L2_ENTRY_ADDR_MASKr,
                                      tmp_hs[0], MASKf);
        index &= addr_mask;
    }

    debugk(DK_VERBOSE, "index %d\n", index);

    return index;
}

void
soc_internal_l2x2_dual_banks(pcid_info_t *pcid_info, uint8 banks,
                             int *dual, int *slot_min, int *slot_max)
{
    uint32          tmp[SOC_MAX_MEM_WORDS];
    soc_block_t     blk;
    uint8           at;

    soc_internal_extended_read_reg(pcid_info, blk, at,
                                   soc_reg_addr_get(pcid_info->unit, 
                                    L2_AUX_HASH_CONTROLr,
                                    REG_PORT_ANY, 0, &blk, &at),
                                   tmp);
    *dual = soc_reg_field_get(pcid_info->unit, L2_AUX_HASH_CONTROLr,
                              tmp[0], ENABLEf);

    if (*dual) {
        switch (banks) {
        case 0:
            return; /* Nothing to do here */
        case 1:
            *slot_min = 4;
            *slot_max = 7;
            break;
        case 2:
            *slot_min = 0;
            *slot_max = 3;
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
soc_internal_l2x2_entry_ins(pcid_info_t *pcid_info, uint8 banks,
                l2x_entry_t *entry, uint32 *result)
{
    uint32 tmp[SOC_MAX_MEM_WORDS];
    int    index = 0, bucket, slot, free_index;
    int    slot_min = 0, slot_max = 7, dual = FALSE;
    int    unit = pcid_info->unit; 
    int    op_fail_pos;
    uint8  at; 

    debugk(DK_VERBOSE, "l2_entry Insert\n");

    bucket = soc_internal_l2x2_hash(pcid_info, entry, FALSE);

    debugk(DK_VERBOSE, "bucket %d\n", bucket);

    if (soc_feature(pcid_info->unit, soc_feature_dual_hash)) {
        soc_internal_l2x2_dual_banks(pcid_info, banks,
                                     &dual, &slot_min, &slot_max);
    }

    /* Check if it should overwrite an existing entry */
    free_index = -1;
    for (slot = slot_min; slot <= slot_max; slot++) {
        if (dual && (slot == 4)) {
            bucket = soc_internal_l2x2_hash(pcid_info, entry, TRUE);
        }
        index = bucket * 8 + slot;
        soc_internal_extended_read_mem(pcid_info, 
                                       SOC_MEM_BLOCK_ANY(unit, L2Xm), at, 
                                       soc_mem_addr_get(unit, L2Xm,
                                        SOC_MEM_BLOCK_ANY(unit, L2Xm),
                                        index, &at), 
                                       tmp);
        if (soc_mem_field32_get(unit, L2Xm, tmp, VALIDf)) {
            if (soc_mem_compare_key(unit, L2Xm, entry, tmp) == 0) {
                /* Overwrite the existing entry */
                PCIM(pcid_info, CMIC_SCHAN_CTRL) &= ~SC_MSG_NAK_TST;

                debugk(DK_VERBOSE, "write bucket %d, slot %d, index %d\n",
                       index / 8, index % 8, index);
                soc_internal_extended_write_mem(pcid_info, 
                                                SOC_MEM_BLOCK_ANY(unit, L2Xm), at,
                                                soc_mem_addr_get(unit, L2Xm,
                                                 SOC_MEM_BLOCK_ANY(unit, L2Xm),
                                                 index, &at), 
                                                (uint32 *)entry);

                result[3] = 0x000;
                return 0;
            }
        } else {
            if (free_index == -1) {
                free_index = index;
            }
        }
    }

    /* Find first unused slot in bucket */
    if (free_index != -1) {
        index = free_index;

        /* Write the existing entry */
        result[3] = 0x000;
        PCIM(pcid_info, CMIC_SCHAN_CTRL) &= ~SC_MSG_NAK_TST;

        debugk(DK_VERBOSE, "write bucket %d, slot %d, index %d\n",
               index / 8, index % 8, index);
        soc_internal_extended_write_mem(pcid_info, 
                                        SOC_MEM_BLOCK_ANY(unit, L2Xm), at, 
                                        soc_mem_addr_get(unit, L2Xm,
                                         SOC_MEM_BLOCK_ANY(unit, L2Xm),
                                         index, &at), 
                                        (uint32 *)entry);
        return 0;
    }

    debugk(DK_VERBOSE, "Bucket full\n");

    /*
     * =============================================================
     * | PERR_PBM | MFIFO_FULL | OP_FAIL | Index  | L2x entry data |
     * =============================================================
     */
    op_fail_pos = SOC_L2X_OP_FAIL_POS(unit);

    result[3] = (1 << op_fail_pos);
    PCIM(pcid_info, CMIC_SCHAN_CTRL) |= SC_MSG_NAK_TST;
    return 0;
}

int
soc_internal_l2x2_entry_del(pcid_info_t * pcid_info, uint8 banks,
                l2x_entry_t *entry, uint32 *result)
{
    uint32          tmp[SOC_MAX_MEM_WORDS];
    int             op_fail_pos;
    int             index = 0, bucket, slot;
    int             slot_min = 0, slot_max = 7, dual = FALSE;
    int             unit = pcid_info->unit;

    debugk(DK_VERBOSE, "l2_entry Delete\n");

    bucket = soc_internal_l2x2_hash(pcid_info, entry, FALSE);

    debugk(DK_VERBOSE, "bucket %d\n", bucket);

    /*
     * =============================================================
     * | PERR_PBM | MFIFO_FULL | OP_FAIL | Index  | L2x entry data |
     * =============================================================
     */
    op_fail_pos = SOC_L2X_OP_FAIL_POS(unit);

    result[3] = (1 << op_fail_pos);

    if (soc_feature(pcid_info->unit, soc_feature_dual_hash)) {
        soc_internal_l2x2_dual_banks(pcid_info, banks,
                                     &dual, &slot_min, &slot_max);
    }

    /* Check if entry exists */
    for (slot = slot_min; slot <= slot_max; slot++) {
        if (dual && (slot == 4)) {
            bucket = soc_internal_l2x2_hash(pcid_info, entry, TRUE);
        }
        index = bucket * 8 + slot;
        soc_internal_read_mem(pcid_info, soc_mem_addr(unit, L2Xm,
                                         SOC_MEM_BLOCK_ANY(unit, L2Xm),
                                         index), tmp);
        if (soc_mem_field32_get(unit, L2Xm, tmp, VALIDf)) {
            if (soc_mem_compare_key(unit, L2Xm, entry, tmp) == 0) {
                result[3] = 0x000;

                /* Invalidate entry */
                soc_mem_field32_set(unit, L2Xm, tmp, VALIDf, 0);
                soc_internal_write_mem(pcid_info,
                                       soc_mem_addr(unit, L2Xm,
                                       SOC_MEM_BLOCK_ANY(unit, L2Xm),
                                       index), (uint32 *)tmp);
            }
        } 
    }

    debugk(DK_VERBOSE, "Not found\n");

    return 0;
}

/* 
 * soc_internal_l2x2_entry_lkup
 *
 *   Note: entry_lookup and entry_result may overlap.
 */

int
soc_internal_l2x2_entry_lkup(pcid_info_t * pcid_info, uint8 banks,
                             l2x_entry_t *entry_lookup, uint32 *result)
{
    uint32 tmp[SOC_MAX_MEM_WORDS];
    int    op_fail_pos;
    int    index = 0, bucket, slot;
    int    slot_min = 0, slot_max = 7, dual = FALSE;
    int    unit = pcid_info->unit; 
    uint8  at;

    debugk(DK_VERBOSE, "l2_entry Lookup\n");

    bucket = soc_internal_l2x2_hash(pcid_info, entry_lookup, FALSE);

    debugk(DK_VERBOSE, "bucket %d\n", bucket);

    /*
     * =============================================================
     * | PERR_PBM | MFIFO_FULL | OP_FAIL | Index  | L2x entry data |
     * =============================================================
     */
    op_fail_pos = SOC_L2X_OP_FAIL_POS(unit);

    result[3] = (1 << op_fail_pos);
   

    if (soc_feature(unit, soc_feature_dual_hash)) {
        soc_internal_l2x2_dual_banks(pcid_info, banks,
                                     &dual, &slot_min, &slot_max);
    }

    /* Check if entry exists */
    for (slot = slot_min; slot <= slot_max; slot++) {
        if (dual && (slot == 4)) {
            bucket = soc_internal_l2x2_hash(pcid_info, entry_lookup, TRUE);
        }
        index = bucket * 8 + slot;
        soc_internal_extended_read_mem(pcid_info,
                                       SOC_MEM_BLOCK_ANY(unit, L2Xm), at, 
                                       soc_mem_addr_get(unit, L2Xm,
                                        SOC_MEM_BLOCK_ANY(unit, L2Xm),
                                        index, &at), 
                                       tmp);
        if (soc_mem_field32_get(unit, L2Xm, tmp, VALIDf)) {
            if (soc_mem_compare_key(unit, L2Xm, entry_lookup, tmp) == 0) {
                int nbits; /* Data Bits in last word */

                nbits = soc_mem_entry_bits(unit, L2Xm) % 32;

                result[0] = tmp[0];
                result[1] = tmp[1];
                result[2] = (tmp[2] & ((1 << nbits) - 1)) |
                            ((index & ((1 << (32 - nbits)) - 1)) << nbits);
                result[3] = (index >> (32 - nbits)) &
                            ((soc_mem_index_max(unit, L2Xm)) >> (32 - nbits));
                return 0;
            }
        } 
    }

    debugk(DK_VERBOSE, "Not found\n");

    return 0;
}
#endif /* BCM_FIREBOLT_SUPPORT */

#ifdef BCM_EASYRIDER_SUPPORT
int
soc_internal_l2_er_hash(pcid_info_t *pcid_info, soc_mem_t tmem, uint32 *entry)
{
    uint32          tmp_hs[SOC_MAX_MEM_WORDS], tmp2_hs[SOC_MAX_MEM_WORDS];
    uint8           key[8];
    int             hash_sel=0, index=0, mem_cfg;
    int             unit = pcid_info->unit;

    debugk(DK_VERBOSE, "%s hash\n", SOC_MEM_NAME(unit, tmem));

    soc_internal_read_reg(pcid_info,
                          soc_reg_addr(unit, HASH_CONTROLr,
                                       REG_PORT_ANY, 0), tmp_hs);
    soc_internal_read_reg(pcid_info,
                          soc_reg_addr(unit, L2_ISr,
                                       REG_PORT_ANY, 0), tmp2_hs);

    soc_er_l2x_base_entry_to_key(unit, (uint32 *)entry, key);

    switch(tmem) {
    case L2_ENTRY_EXTERNALm:
        hash_sel = soc_reg_field_get(unit, HASH_CONTROLr,
                                     tmp_hs[0], L2_EXT_HASH_SELECTf);
        soc_internal_read_reg(pcid_info,
                              soc_reg_addr(unit, SEER_CONFIGr,
                                           REG_PORT_ANY, 0), tmp_hs);
        if (soc_reg_field_get(unit, SEER_CONFIGr,
                              tmp_hs[0], EXT_TABLE_CONFIGf) !=
                                           ER_SEER_CFG_L2_512_EXT) {
            /* No external L2, return -1 */
            return -1;
        }
        if (soc_reg_field_get(unit, L2_ISr, tmp2_hs[0], EXT_DISf) == 1) {
            /* No external L2, return -1 */
            return -1;
        }

        /* Adjust these for the memory */
        if (SOC_CONTROL(unit)->hash_mask_l2x_ext == 0) {
            SOC_CONTROL(unit)->hash_mask_l2x_ext = 0x1FFFF;
            SOC_CONTROL(unit)->hash_bits_l2x_ext = 17;
        }
        index = soc_er_l2ext_hash(unit, hash_sel, key);
        break;
    case L2_ENTRY_INTERNALm:
        hash_sel = soc_reg_field_get(unit, HASH_CONTROLr,
                                     tmp_hs[0], L2_AND_VLAN_MAC_HASH_SELECTf);
        if (soc_reg_field_get(unit, L2_ISr,
                              tmp2_hs[0], INT_DISf) == 1) {
            /* No internal L2, return -1 */
            return -1;
        }

        mem_cfg = soc_reg_field_get(unit, L2_ISr,
                                     tmp2_hs[0], MEM_CONFIGf);

        switch (mem_cfg) {
        case ER_SEER_HOST_HASH_CFG_L2_ALL:
            SOC_CONTROL(unit)->hash_mask_l2x = 0xFFF;
            SOC_CONTROL(unit)->hash_bits_l2x = 12;
            break;

        case ER_SEER_HOST_HASH_CFG_L2_HALF_V4_HALF:
        case ER_SEER_HOST_HASH_CFG_L2_HALF_V4_QUART_V6_QUART:
        case ER_SEER_HOST_HASH_CFG_L2_HALF_V6_HALF:
            SOC_CONTROL(unit)->hash_mask_l2x = 0x7FF;
            SOC_CONTROL(unit)->hash_bits_l2x = 11;
            break;

        case ER_SEER_HOST_HASH_CFG_L2_QUART_V4_HALF_V6_QUART:
        case ER_SEER_HOST_HASH_CFG_L2_QUART_V4_QUART_V6_HALF:
            SOC_CONTROL(unit)->hash_mask_l2x = 0x3FF;
            SOC_CONTROL(unit)->hash_bits_l2x = 10;
            break;

        case ER_SEER_HOST_HASH_CFG_V4_ALL:
        case ER_SEER_HOST_HASH_CFG_V4_HALF_V6_HALF:
        case ER_SEER_HOST_HASH_CFG_V6_ALL:
        default:
            /* No internal L2, return -1 */
            return -1;
        }
 
        index = soc_er_l2_hash(unit, hash_sel, key);
        break;
    case L2_ENTRY_OVERFLOWm:
        if (soc_reg_field_get(unit, L2_ISr,
                              tmp2_hs[0], OVF_DISf) == 1) {
            /* No overflow L2, return -1 */
            return -1;
        }
        index = 0;
        hash_sel = -1;
        break;
    default:
        break;
    }

    debugk(DK_VERBOSE, "hash_sel %d\n", hash_sel);
    debugk(DK_VERBOSE, "index %d\n", index);

    return index;
}

int
soc_internal_l2_er_entry_scan_bucket(pcid_info_t *pcid_info,
                                     soc_mem_t tmem, int bucket,
                                     uint32 *entry, int *free_slot)
{
    uint32          tmp[SOC_MAX_MEM_WORDS];
    int             index = 0, bucket_size, slot;
    int             rindex = -1, found_free = FALSE;
    int             unit = pcid_info->unit;

    bucket_size = (tmem == L2_ENTRY_EXTERNALm) ?
        SOC_L2EXT_BUCKET_SIZE : SOC_L2X_BUCKET_SIZE;

    /* Check if it should overwrite an existing entry */
    for (slot = 0; slot < bucket_size; slot++) {
        index = bucket * bucket_size + slot;
        soc_internal_read_mem(pcid_info, soc_mem_addr(unit, tmem,
                                         SOC_MEM_BLOCK_ANY(unit, tmem),
                                         index), tmp);
        if ((rindex < 0) && soc_mem_field32_get(unit, tmem, tmp, VALID_BITf) &&
            soc_mem_field32_get(unit, tmem, tmp, ACTIVEf)) {
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
soc_internal_l2_er_index_to_cm_addr(int unit, soc_mem_t tmem,
                                    uint32 *entry, uint32 index)
{
    int             cmindex;

    switch (tmem) {
    case L2_ENTRY_EXTERNALm:
        cmindex = (index & SOC_MEM_CMD_EXT_L2_ADDR_MASK_LO) |
            ((index << SOC_MEM_CMD_EXT_L2_ADDR_SHIFT_HI) &
             SOC_MEM_CMD_EXT_L2_ADDR_MASK_HI);
        break;
    case L2_ENTRY_INTERNALm:
        cmindex = index |
            (SOC_MEM_CMD_HASH_TABLE_DFLT << SOC_MEM_CMD_HASH_TABLE_SHIFT);
        break;
    case L2_ENTRY_OVERFLOWm:
        cmindex = index |
            (SOC_MEM_CMD_HASH_TABLE_L2OVR << SOC_MEM_CMD_HASH_TABLE_SHIFT);
        break;
    default:
        cmindex = 0;
        break;
    }

    soc_mem_field32_set(unit, COMMAND_MEMORY_HSEm, entry,
                            ADDRESS_0f, cmindex);
}

int
soc_internal_l2_er_entry_ins(pcid_info_t *pcid_info, uint32 *entry)
{
    int             index = -1, bucket, free_slot, first_free, mix;
    int             unit = pcid_info->unit; 
    soc_mem_t       tmem, free_mem=INVALIDm;

    debugk(DK_VERBOSE, "l2_entry Insert\n");

    first_free = -1;
    for (mix = 0; mix < 3; mix++) {
        tmem = (mix == 0) ? L2_ENTRY_EXTERNALm :
            ((mix == 1) ? L2_ENTRY_INTERNALm : L2_ENTRY_OVERFLOWm);
        bucket = soc_internal_l2_er_hash(pcid_info, tmem, (uint32 *)entry);
        if (bucket >= 0) {
            index =
                soc_internal_l2_er_entry_scan_bucket(pcid_info, tmem, bucket,
                                   (uint32 *)entry, &free_slot);
            if (index >= 0) {
                break;
            } else if ((first_free < 0) && (free_slot >= 0)) {
                first_free = free_slot;
                free_mem = tmem;
            }
        }
    }

    if ((index < 0) && (first_free < 0)) {
        debugk(DK_VERBOSE, "All target hash buckets full\n");
        soc_mem_field32_set(unit, COMMAND_MEMORY_HSEm, entry, TAB_FULLf, 1);
    } else {
        if (index < 0) {
            tmem = free_mem;
            index = first_free;
        }
        debugk(DK_VERBOSE, "write table %s, index %d\n",
               SOC_MEM_NAME(unit, tmem), index);

        /* The valid bit is always set by hardware */
        soc_mem_field32_set(unit, tmem, entry, VALID_BITf, 1);
        soc_mem_field32_set(unit, tmem, entry, ACTIVEf, 1);
        soc_internal_write_mem(pcid_info, soc_mem_addr(unit, tmem,
                                          SOC_MEM_BLOCK_ANY(unit, tmem),
                                          index), (uint32 *)entry);

        soc_internal_l2_er_index_to_cm_addr(unit, tmem, entry, index);
    }

    soc_mem_field32_set(unit, COMMAND_MEMORY_HSEm, entry, NEWCMDf, 0);

    return 0;
}

int
soc_internal_l2_er_entry_del(pcid_info_t *pcid_info, uint32 *entry)
{
    int             index = -1, bucket, free_slot, mix;
    int             unit = pcid_info->unit; 
    soc_mem_t       tmem;

    debugk(DK_VERBOSE, "l2_entry Delete\n");

    for (mix = 0; mix < 3; mix++) {
        tmem = (mix == 0) ? L2_ENTRY_EXTERNALm :
            ((mix == 1) ? L2_ENTRY_INTERNALm : L2_ENTRY_OVERFLOWm);
        bucket = soc_internal_l2_er_hash(pcid_info, tmem, (uint32 *)entry);
        if (bucket >= 0) {
            index =
                soc_internal_l2_er_entry_scan_bucket(pcid_info, tmem, bucket,
                                   (uint32 *)entry, &free_slot);
            if (index >= 0) {
                break;
            }
        }
    }

    if (index < 0) {
        debugk(DK_VERBOSE, "Entry not foundl\n");
        soc_mem_field32_set(unit, COMMAND_MEMORY_HSEm, entry, NOT_FOUNDf, 1);
    } else {
        debugk(DK_VERBOSE, "write table %s, index %d\n",
               SOC_MEM_NAME(unit, tmem), index);

        /* Should we read this first? */

        /* The valid bit is always set by hardware */
        soc_mem_field32_set(unit, tmem, entry, VALIDf, 0);
        soc_mem_field32_set(unit, tmem, entry, ACTIVEf, 0);
        soc_internal_write_mem(pcid_info, soc_mem_addr(unit, tmem,
                                          SOC_MEM_BLOCK_ANY(unit, tmem),
                                          index), (uint32 *)entry);

        soc_internal_l2_er_index_to_cm_addr(unit, tmem, entry, index);
    }

    soc_mem_field32_set(unit, COMMAND_MEMORY_HSEm, entry, NEWCMDf, 0);

    return 0;
}

int
soc_internal_l2_er_entry_lkup(pcid_info_t *pcid_info, uint32 *entry)
{
    int             index = -1, bucket, free_slot, mix;
    int             unit = pcid_info->unit; 
    soc_mem_t       tmem;

    debugk(DK_VERBOSE, "l2_entry Lookup\n");

    for (mix = 0; mix < 3; mix++) {
        tmem = (mix == 0) ? L2_ENTRY_EXTERNALm :
            ((mix == 1) ? L2_ENTRY_INTERNALm : L2_ENTRY_OVERFLOWm);
        bucket = soc_internal_l2_er_hash(pcid_info, tmem, (uint32 *)entry);
        if (bucket >= 0) {
            index =
                soc_internal_l2_er_entry_scan_bucket(pcid_info, tmem, bucket,
                                   (uint32 *)entry, &free_slot);
            if (index >= 0) {
                break;
            }
        }
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

        soc_internal_l2_er_index_to_cm_addr(unit, tmem, entry, index);
    }

    soc_mem_field32_set(unit, COMMAND_MEMORY_HSEm, entry, NEWCMDf, 0);

    return 0;
}

int
soc_internal_l2_er_delete_by(pcid_info_t *pcid_info, uint32 *entry)
{
    int             index, max_index, mix;
    int             unit = pcid_info->unit; 
    soc_mem_t       tmem;
    uint32          tmp[SOC_MAX_MEM_WORDS];
    uint32          ppa_cmd;
    uint32          modid, tgid_port, vlan, repl_modid, repl_tgid_port, stat2;
    int             cmp_portmod, cmp_vlan, repl_portmod, all_del;

    debugk(DK_VERBOSE, "l2_entry Per Port Aging\n");

    ppa_cmd = soc_mem_field32_get(unit, COMMAND_MEMORY_HSEm, entry,
                                  PPA_MODEf);
    modid = soc_mem_field32_get(unit, COMMAND_MEMORY_HSEm, entry,
                                MODULE_IDf);
    tgid_port = soc_mem_field32_get(unit, COMMAND_MEMORY_HSEm, entry,
                                    TGID_PORTf);
    vlan = soc_mem_field32_get(unit, COMMAND_MEMORY_HSEm, entry,
                               VLAN_IDf);
    repl_modid = soc_mem_field32_get(unit, COMMAND_MEMORY_HSEm, entry,
                                     REPLACE_MODULE_IDf);
    repl_tgid_port = soc_mem_field32_get(unit, COMMAND_MEMORY_HSEm, entry,
                                         REPLACE_PORT_TGIDf);
    stat2 = soc_mem_field32_get(unit, COMMAND_MEMORY_HSEm, entry,
                                EXCL_STATICf);


    switch(ppa_cmd) {
    case SOC_ER_L2_PORTMOD_DEL:
        cmp_portmod = TRUE;
        cmp_vlan = FALSE;
        repl_portmod = FALSE;
        all_del = FALSE;
        break;

    case SOC_ER_L2_VLAN_DEL:
        cmp_portmod = FALSE;
        cmp_vlan = TRUE;
        repl_portmod = FALSE;
        all_del = FALSE;
        break;

    case SOC_ER_L2_PORTMOD_VLAN_DEL:
        cmp_portmod = TRUE;
        cmp_vlan = TRUE;
        repl_portmod = FALSE;
        all_del = FALSE;
        break;

    case SOC_ER_L2_ALL_DEL:
        cmp_portmod = FALSE;
        cmp_vlan = FALSE;
        repl_portmod = FALSE;
        all_del = TRUE;
        break;

    case SOC_ER_L2_PORTMOD_REPL:
        cmp_portmod = TRUE;
        cmp_vlan = FALSE;
        repl_portmod = TRUE;
        all_del = FALSE;
        break;

    case SOC_ER_L2_VLAN_REPL:
        cmp_portmod = FALSE;
        cmp_vlan = TRUE;
        repl_portmod = TRUE;
        all_del = FALSE;
        break;

    case SOC_ER_L2_PORTMOD_VLAN_REPL:
        cmp_portmod = TRUE;
        cmp_vlan = TRUE;
        repl_portmod = TRUE;
        all_del = FALSE;
        break;
    default:
        /* Can't do normal aging */
        goto done;
    }

    for (mix = 0; mix < 3; mix++) {
        tmem = (mix == 0) ? L2_ENTRY_EXTERNALm :
            ((mix == 1) ? L2_ENTRY_INTERNALm : L2_ENTRY_OVERFLOWm);

        sal_memset(tmp, 0, SOC_MAX_MEM_WORDS * 4);
        if (soc_internal_l2_er_hash(pcid_info, tmem, tmp) < 0) {
            continue;
        }
        max_index = (tmem == L2_ENTRY_EXTERNALm) ?
            (((SOC_CONTROL(unit)->hash_mask_l2x_ext + 1) *
             SOC_L2EXT_BUCKET_SIZE) - 1) : 
            ((tmem == L2_ENTRY_INTERNALm) ?
             (((SOC_CONTROL(unit)->hash_mask_l2x + 1) *
               SOC_L2X_BUCKET_SIZE) - 1) : 7);

        for (index = 0; index <= max_index; index++) {
            soc_internal_read_mem(pcid_info, soc_mem_addr(unit, tmem,
                                         SOC_MEM_BLOCK_ANY(unit, tmem),
                                         index), tmp);
            if (!soc_mem_field32_get(unit, tmem, tmp, VALID_BITf)) {
                continue;
            }
            if (!soc_mem_field32_get(unit, tmem, tmp, ACTIVEf)) {
                continue;
            }
 
            if (!stat2 && soc_mem_field32_get(unit, tmem, tmp, STATIC_BITf)) {
                continue;
            }
            /* Main delete by section */

            if (!all_del) {
                if (cmp_vlan && (vlan !=
                      soc_mem_field32_get(unit, tmem, tmp, VLAN_IDf))) {
                    continue;
                }
                if (cmp_portmod && ((tgid_port !=
                      soc_mem_field32_get(unit, tmem, tmp, PORT_TGIDf)) ||
                                     (modid !=
                      soc_mem_field32_get(unit, tmem, tmp, MODULE_IDf)))) {
                    continue;
                }
            }

            if (repl_portmod) {
                soc_mem_field32_set(unit, tmem, tmp, PORT_TGIDf,
                                    repl_tgid_port);
                soc_mem_field32_set(unit, tmem, tmp, MODULE_IDf, repl_modid);
            } else {
                soc_mem_field32_set(unit, tmem, tmp, ACTIVEf, 0);
                soc_mem_field32_set(unit, tmem, tmp, VALID_BITf, 0);
            }

            soc_internal_write_mem(pcid_info, soc_mem_addr(unit, tmem,
                                         SOC_MEM_BLOCK_ANY(unit, tmem),
                                         index), tmp);
        }
    }

 done:
    /* All done now */
    soc_mem_field32_set(unit, COMMAND_MEMORY_HSEm, entry, NEWCMDf, 0);

    return 0;
}

int
soc_internal_mcmd_write(pcid_info_t * pcid_info, soc_mem_t mem,
                        uint32 *data, int cmd)
{
    int             index;
    uint32          address;
    soc_datum_t    *d;
    int             target_tab, ttcomp, command; 
    soc_mem_info_t  *mip;
    int             l2op = FALSE, l3v4op = FALSE, l3v6op = FALSE;
    int             unit = pcid_info->unit;

    if (mem != COMMAND_MEMORY_HSEm) {
        /* print a warning? */
    }

    if (!cmd) {
        /* Simple write here */
        address = soc_mem_addr(unit, mem, HSE_BLOCK(unit), 0);
        index = SOC_HASH_DATUM(address);

        for (d = pcid_info->mem_ht[index]; d != NULL; d = d->next) {
            if (d->addr == address) {
                memcpy(d->data, data, SOC_MAX_MEM_WORDS * 4);
                return 0;
            }
        }

        d = sal_alloc(sizeof(soc_datum_t), "mdatum_t");
        d->addr = address;
        memcpy(d->data, data, SOC_MAX_MEM_WORDS * 4);
        d->next = pcid_info->mem_ht[index];
        pcid_info->mem_ht[index] = d;

        return 0;
    }

    target_tab = soc_mem_field32_get(unit, COMMAND_MEMORY_HSEm, data,
                                     TARGET_TABf);
    command = soc_mem_field32_get(unit, COMMAND_MEMORY_HSEm, data,
                                  COMMANDf);

    mip = &SOC_MEM_INFO(unit, L2_ENTRY_EXTERNALm);
    ttcomp = (mip->base >> SOC_MEM_CMD_BASE_TO_OFFSET) & 
        SOC_MEM_CMD_TARGET_MASK;

    if (target_tab == ttcomp) {
        l2op = TRUE;
    }

    mip = &SOC_MEM_INFO(unit, L2_ENTRY_INTERNALm);
    ttcomp = (mip->base >> SOC_MEM_CMD_BASE_TO_OFFSET) & 
        SOC_MEM_CMD_TARGET_MASK;

    if (target_tab == ttcomp) {
        l2op = TRUE;
    }

    mip = &SOC_MEM_INFO(unit, L2_ENTRY_OVERFLOWm);
    ttcomp = (mip->base >> SOC_MEM_CMD_BASE_TO_OFFSET) & 
        SOC_MEM_CMD_TARGET_MASK;

    if (target_tab == ttcomp) {
        l2op = TRUE;
    }

    mip = &SOC_MEM_INFO(unit, L3_ENTRY_V4m);
    ttcomp = (mip->base >> SOC_MEM_CMD_BASE_TO_OFFSET) & 
        SOC_MEM_CMD_TARGET_MASK;

    if (target_tab == ttcomp) {
        l3v4op = TRUE;
    }

    mip = &SOC_MEM_INFO(unit, L3_ENTRY_V6m);
    ttcomp = (mip->base >> SOC_MEM_CMD_BASE_TO_OFFSET) & 
        SOC_MEM_CMD_TARGET_MASK;

    if (target_tab == ttcomp) {
        l3v6op = TRUE;
    }

    switch (command) {
    case SOC_MEM_CMD_LEARN:
        if (l2op) {
            soc_internal_l2_er_entry_ins(pcid_info, data);
        } else if (l3v4op) {
            soc_internal_l3_er_entry_ins(pcid_info, data, FALSE);
        } else if (l3v6op) {
            soc_internal_l3_er_entry_ins(pcid_info, data, TRUE);
        }
        break;

    case SOC_MEM_CMD_SEARCH:
        if (l2op) {
            soc_internal_l2_er_entry_lkup(pcid_info, data);
        } else if (l3v4op) {
            soc_internal_l3_er_entry_lkup(pcid_info, data, FALSE);
        } else if (l3v6op) {
            soc_internal_l3_er_entry_lkup(pcid_info, data, TRUE);
        }
        break;

    case SOC_MEM_CMD_DELETE:
        if (l2op) {
            soc_internal_l2_er_entry_del(pcid_info, data);
        } else if (l3v4op) {
            soc_internal_l3_er_entry_del(pcid_info, data, FALSE);
        } else if (l3v6op) {
            soc_internal_l3_er_entry_del(pcid_info, data, TRUE);
        }
        break;
    case SOC_MEM_CMD_PER_PORT_AGING:
        if (l2op) {
            soc_internal_l2_er_delete_by(pcid_info, data);
        }
        break;
    }

    soc_internal_mcmd_write(pcid_info, mem, data, FALSE);
    return 0;
}

#endif /* BCM_EASYRIDER_SUPPORT */

#ifdef BCM_TRX_SUPPORT
int
soc_internal_l2_tr_hash(pcid_info_t * pcid_info, l2x_entry_t *entry, int dual)
{
    uint32          tmp_hs[SOC_MAX_MEM_WORDS];
    int             hash_sel;
    uint8           at, key[8];
    int             index, num_bits;
    soc_block_t     blk;

    debugk(DK_VERBOSE, "L2 TR hash\n");

    if (dual) {
        soc_internal_extended_read_reg(pcid_info, blk, at,
              soc_reg_addr_get(pcid_info->unit, L2_AUX_HASH_CONTROLr, REG_PORT_ANY, 0, 
                               &blk, &at),
              tmp_hs);
        hash_sel = soc_reg_field_get(pcid_info->unit, L2_AUX_HASH_CONTROLr,
                                     tmp_hs[0], HASH_SELECTf);
    } else {
        soc_internal_extended_read_reg(pcid_info, blk, at,
              soc_reg_addr_get(pcid_info->unit, HASH_CONTROLr, REG_PORT_ANY, 0, 
                               &blk, &at), tmp_hs);
        hash_sel = soc_reg_field_get(pcid_info->unit, HASH_CONTROLr,
                                     tmp_hs[0], L2_AND_VLAN_MAC_HASH_SELECTf);
    }

    debugk(DK_VERBOSE, "hash_sel%s %d\n", dual ? "" : "(aux)", hash_sel);

    num_bits = soc_tr_l2x_base_entry_to_key(pcid_info->unit, entry, key);
    index = soc_tr_l2x_hash(pcid_info->unit, hash_sel, num_bits, entry, key);

    debugk(DK_VERBOSE, "index %d\n", index);

    return index;
}

void
soc_internal_l2_tr_dual_banks(pcid_info_t *pcid_info, uint8 banks,
                              int *dual, int *slot_min, int *slot_max)
{
    uint32          tmp[SOC_MAX_MEM_WORDS];
    soc_block_t     blk; 
    uint8           at;

    soc_internal_extended_read_reg(pcid_info, blk, at,
                          soc_reg_addr_get(pcid_info->unit, L2_AUX_HASH_CONTROLr,
                                       REG_PORT_ANY, 0, &blk, &at), tmp);
    *dual = soc_reg_field_get(pcid_info->unit, L2_AUX_HASH_CONTROLr,
                              tmp[0], ENABLEf);

    if (*dual) {
        switch (banks) {
        case 0:
            return; /* Nothing to do here */
        case 1:
            *slot_min = 4;
            *slot_max = 7;
            break;
        case 2:
            *slot_min = 0;
            *slot_max = 3;
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
soc_internal_l2_tr_entry_ins(pcid_info_t *pcid_info, uint8 banks,
                             l2x_entry_t *entry, uint32 *result)
{
    uint32          tmp[SOC_MAX_MEM_WORDS];
    int             index = 0, bucket, slot, free_index;
    int             slot_min = 0, slot_max = 7, dual = FALSE;
    int             unit = pcid_info->unit; 
    schan_genresp_t *genresp = (schan_genresp_t *) result;
    int             blk;             
    soc_block_t     sblk;
    uint8           at;

    debugk(DK_VERBOSE, "l2_entry Insert\n");

    bucket = soc_internal_l2_tr_hash(pcid_info, entry, FALSE);

    debugk(DK_VERBOSE, "bucket %d\n", bucket);

    *result = 0;

    if (soc_feature(pcid_info->unit, soc_feature_dual_hash)) {
        soc_internal_l2_tr_dual_banks(pcid_info, banks,
                                      &dual, &slot_min, &slot_max);
    }
    blk = SOC_MEM_BLOCK_ANY(unit, L2Xm);
    sblk = SOC_BLOCK2SCH(unit, blk);
    /* Check if it should overwrite an existing entry */
    free_index = -1;
    for (slot = slot_min; slot <= slot_max; slot++) {
        if (dual && (slot == 4)) {
            bucket = soc_internal_l2_tr_hash(pcid_info, entry, TRUE);
        }
        index = bucket * 8 + slot;
        soc_internal_extended_read_mem(pcid_info, sblk, at,
                                       soc_mem_addr_get(unit, L2Xm,
                                        blk, index, &at), tmp);
        if (soc_mem_field32_get(unit, L2Xm, tmp, VALIDf)) {
            if (soc_mem_compare_key(unit, L2Xm, entry, tmp) == 0) {
                /* Overwrite the existing entry */
                PCIM(pcid_info, CMIC_SCHAN_CTRL) &= ~SC_MSG_NAK_TST;

                debugk(DK_VERBOSE, "write bucket %d, slot %d, index %d\n",
                       index / 8, index % 8, index);
                soc_internal_extended_write_mem(pcid_info, sblk, at,
                                           soc_mem_addr_get(unit, L2Xm,
                                           blk, index, &at), (uint32 *)entry);

                genresp->type = SCHAN_GEN_RESP_TYPE_REPLACED;
                genresp->index = index;
                /* Copy old entry immediately after response word */
                memcpy(&result[1], tmp, soc_mem_entry_bytes(unit, L2Xm));
                return 0;
            }
        } else {
            if (free_index == -1) {
                free_index = index;
            }
        }
    }

    /* Find first unused slot in bucket */
    if (free_index != -1) {
        index = free_index;

        /* Write the existing entry */
        genresp->type = SCHAN_GEN_RESP_TYPE_INSERTED;
        genresp->index = index;
        PCIM(pcid_info, CMIC_SCHAN_CTRL) &= ~SC_MSG_NAK_TST;

        debugk(DK_VERBOSE, "write bucket %d, slot %d, index %d\n",
               index / 8, index % 8, index);
        soc_internal_extended_write_mem(pcid_info, 
                                        sblk, at,
                                        soc_mem_addr_get(unit, L2Xm,
                                         blk,
                                         index, &at), (uint32 *)entry);
        return 0;
    }

    debugk(DK_VERBOSE, "Bucket full\n");

    genresp->type = SCHAN_GEN_RESP_TYPE_FULL;
    PCIM(pcid_info, CMIC_SCHAN_CTRL) |= SC_MSG_NAK_TST;
    return 0;
}

int
soc_internal_l2_tr_entry_del(pcid_info_t * pcid_info, uint8 banks,
                             l2x_entry_t *entry, uint32 *result)
{
    uint32          tmp[SOC_MAX_MEM_WORDS];
    int             index = 0, bucket, slot;
    int             slot_min = 0, slot_max = 7, dual = FALSE;
    int             unit = pcid_info->unit;
    schan_genresp_t *genresp = (schan_genresp_t *) result;

    debugk(DK_VERBOSE, "l2_entry Delete\n");

    bucket = soc_internal_l2_tr_hash(pcid_info, entry, FALSE);

    debugk(DK_VERBOSE, "bucket %d\n", bucket);

    *result = 0;
    genresp->type = SCHAN_GEN_RESP_TYPE_NOT_FOUND;
    PCIM(pcid_info, CMIC_SCHAN_CTRL) |= SC_MSG_NAK_TST;

    if (soc_feature(pcid_info->unit, soc_feature_dual_hash)) {
        soc_internal_l2_tr_dual_banks(pcid_info, banks,
                                      &dual, &slot_min, &slot_max);
    }

    /* Check if entry exists */
    for (slot = slot_min; slot <= slot_max; slot++) {
        if (dual && (slot == 4)) {
            bucket = soc_internal_l2_tr_hash(pcid_info, entry, TRUE);
        }
        index = bucket * 8 + slot;
        soc_internal_read_mem(pcid_info, soc_mem_addr(unit, L2Xm,
                                         SOC_MEM_BLOCK_ANY(unit, L2Xm),
                                         index), tmp);
        if (soc_mem_field32_get(unit, L2Xm, tmp, VALIDf)) {
            if (soc_mem_compare_key(unit, L2Xm, entry, tmp) == 0) {
                genresp->type = SCHAN_GEN_RESP_TYPE_DELETED;
                genresp->index = index;
                PCIM(pcid_info, CMIC_SCHAN_CTRL) &= ~SC_MSG_NAK_TST;

                /* Copy entry immediately after response word */
                memcpy(&result[1], tmp, soc_mem_entry_bytes(unit, L2Xm));

                /* Invalidate entry */
                soc_mem_field32_set(unit, L2Xm, tmp, VALIDf, 0);
                soc_internal_write_mem(pcid_info,
                                       soc_mem_addr(unit, L2Xm,
                                       SOC_MEM_BLOCK_ANY(unit, L2Xm),
                                       index), (uint32 *)tmp);
            }
        } 
    }
    debugk(DK_VERBOSE, "Not found\n");

    return 0;
}

/* 
 * soc_internal_l2_tr_entry_lkup
 *
 *   Note: entry_lookup and entry_result may overlap.
 */

int
soc_internal_l2_tr_entry_lkup(pcid_info_t * pcid_info, uint8 banks,
                              l2x_entry_t *entry_lookup, uint32 *result)
{
    uint32          tmp[SOC_MAX_MEM_WORDS];
    int             index = 0, bucket, slot;
    int             slot_min = 0, slot_max = 7, dual = FALSE;
    int             unit = pcid_info->unit; 
    schan_genresp_t *genresp = (schan_genresp_t *) result;
    int             blk;             
    soc_block_t     sblk;
    uint8           at;

    debugk(DK_VERBOSE, "l2_entry Lookup\n");

    bucket = soc_internal_l2_tr_hash(pcid_info, entry_lookup, FALSE);

    debugk(DK_VERBOSE, "bucket %d\n", bucket);

    *result = 0;
    genresp->type = SCHAN_GEN_RESP_TYPE_NOT_FOUND;
    PCIM(pcid_info, CMIC_SCHAN_CTRL) |= SC_MSG_NAK_TST;

    if (soc_feature(unit, soc_feature_dual_hash)) {
        soc_internal_l2_tr_dual_banks(pcid_info, banks,
                                      &dual, &slot_min, &slot_max);
    }
    blk = SOC_MEM_BLOCK_ANY(unit, L2Xm);
    sblk = SOC_BLOCK2SCH(unit, blk);
    /* Check if entry exists */
    for (slot = slot_min; slot <= slot_max; slot++) {
        if (dual && (slot == 4)) {
            bucket = soc_internal_l2_tr_hash(pcid_info, entry_lookup, TRUE);
        }
        index = bucket * 8 + slot;
        soc_internal_extended_read_mem(pcid_info, sblk, at, 
                                       soc_mem_addr_get(unit, L2Xm, blk,
                                         index, &at), tmp);
        if (soc_mem_field32_get(unit, L2Xm, tmp, VALIDf)) {
            if (soc_mem_compare_key(unit, L2Xm, entry_lookup, tmp) == 0) {
                genresp->type = SCHAN_GEN_RESP_TYPE_FOUND;
                genresp->index = index;
                PCIM(pcid_info, CMIC_SCHAN_CTRL) &= ~SC_MSG_NAK_TST;

                /* Copy entry immediately after response word */
                memcpy(&result[1], tmp, soc_mem_entry_bytes(unit, L2Xm));
            }
        } 
    }
    debugk(DK_VERBOSE, "Not found\n");

    return 0;
}
#endif /* BCM_TRX_SUPPORT */
#endif
