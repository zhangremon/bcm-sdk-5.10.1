/*
 * $Id: tcam.c 1.66.122.1 Broadcom SDK $
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
 * File:      tcam.c
 */

#include <soc/debug.h>
#include <soc/error.h>
#include <soc/drv.h>
#include <soc/mem.h>
#if defined(BCM_EASYRIDER_SUPPORT)
#include <soc/easyrider.h>
#endif /* BCM_EASYRIDER_SUPPORT */
#include <soc/er_tcam.h>

/*
 * seer_config_ext_table_config (an actual config value?)
 * min_blk_num_acl (always 0?)
 * max_blk_num_acl
 * min_blk_num_lpm (always "max_blk_num_acl + 1"?)
 * max_blk_num_lpm (always max block?)
 * The nstart is calculated as:
 * nstart = nse_table_start / relative entry size
 * Assuming that the tcam is always partitioned into 2 tables, with
 * a 144-bit table goes from address 0, the only configuration parameter would
 * number of blocks in it.
 * min_blk_num_lpm = nse_144_blk_cnt;
 * nstart_72b_table = nse_144_blk_cnt * entries_per_block
 */

#ifdef BCM_EASYRIDER_SUPPORT
er_tcam_config_t configs[] = {
/*  #tcam, #acl entries, #lpm entries */
    { 0,   0,            0 },             /* 0 */
    { 0,   0,            0 },             /* 1 */
    { 0,   0,            0 },             /* 2 */
    { 2,   192 * 1024,   0 },             /* 3 */
    { 1,   96 * 1024,    0 },             /* 4 */
    { 2,   128 * 1024,   256 * 1024 },    /* 5 */
    { 2,   64 * 1024,    384 * 1024 },    /* 6 */
    { 1,   64 * 1024,    128 * 1024 },    /* 7 */
    { 1,   32 * 1024,    192 * 1024 },    /* 8 */
    { 2,   0,            448 * 1024 },    /* 9 */
    { 4,   0,            896 * 1024 },    /* 10 */
};

STATIC int
soc_easyrider_tcam_init(int unit)
{
    soc_control_t *soc;
    soc_persist_t *sop;
    sop_memcfg_er_t *mcer;
    soc_tcam_info_t *tcam_info;
    uint32 rval, reset_usec;
    uint32 field_buf[5];
    int status, retryCnt = 0;

    soc = SOC_CONTROL(unit);
    sop = SOC_PERSIST(unit);
    mcer = &(sop->er_memcfg);

    if (soc->tcam_info == NULL) {
        soc->tcam_info = sal_alloc(sizeof(soc_tcam_info_t), "tcam info");
        if (soc->tcam_info == NULL) {
            return SOC_E_MEMORY;
        }
    }
    tcam_info = soc->tcam_info;
    sal_memset(tcam_info, 0, sizeof(soc_tcam_info_t));
    tcam_info->num_tcams = configs[mcer->ext_table_cfg].tcams;

    tcam_info->partitions[TCAM_PARTITION_ACL].num_entries =
        configs[mcer->ext_table_cfg].acl_entries;
    tcam_info->partitions[TCAM_PARTITION_ACL].num_entries_include_pad =
        configs[mcer->ext_table_cfg].acl_entries;
    tcam_info->partitions[TCAM_PARTITION_ACL].tcam_width_shift = 1;
    tcam_info->partitions[TCAM_PARTITION_ACL].tcam_base = 0;
    tcam_info->partitions[TCAM_PARTITION_ACL].sram_base = -1;
    tcam_info->partitions[TCAM_PARTITION_ACL].counter_base = -1;
    tcam_info->partitions[TCAM_PARTITION_ACL].hbit_base = -1;

    tcam_info->partitions[TCAM_PARTITION_LPM].num_entries =
        configs[mcer->ext_table_cfg].lpm_entries;
    tcam_info->partitions[TCAM_PARTITION_LPM].num_entries_include_pad =
        configs[mcer->ext_table_cfg].lpm_entries;
    tcam_info->partitions[TCAM_PARTITION_LPM].tcam_width_shift = 0;
    tcam_info->partitions[TCAM_PARTITION_LPM].tcam_base =
        configs[mcer->ext_table_cfg].acl_entries * 2;
    tcam_info->partitions[TCAM_PARTITION_LPM].sram_base = -1;
    tcam_info->partitions[TCAM_PARTITION_LPM].counter_base = -1;
    tcam_info->partitions[TCAM_PARTITION_LPM].hbit_base = -1;

    tcam_info->partitions[TCAM_PARTITION_RAW].num_entries =
        configs[mcer->ext_table_cfg].acl_entries * 2 +
        configs[mcer->ext_table_cfg].lpm_entries;

    if (mcer->tcam_select == ER_EXT_TCAM_NONE) {
        return SOC_E_NONE;
    }

    reset_usec = soc_property_get(unit, spn_TCAM_RESET_USEC, 1000000);
retry:
    sal_usleep(reset_usec);

    /* Reset TCAM, active low */
    SOC_IF_ERROR_RETURN(READ_CSE_CONFIGr(unit, &rval));
    soc_reg_field_set(unit, CSE_CONFIGr, &rval, EXT_TCAM_RESETf, 0);
    soc_reg_field_set(unit, CSE_CONFIGr, &rval, EXT_TCAM_SLOWf, 1);
    SOC_IF_ERROR_RETURN(WRITE_CSE_CONFIGr(unit, rval));

    sal_usleep(reset_usec);

    SOC_IF_ERROR_RETURN(READ_CSE_CONFIGr(unit, &rval));
    soc_reg_field_set(unit, CSE_CONFIGr, &rval, EXT_TCAM_RESETf, 1);
    SOC_IF_ERROR_RETURN(WRITE_CSE_CONFIGr(unit, rval));

    sal_usleep(reset_usec);

    switch (mcer->tcam_select) {
    case ER_EXT_TCAM_TYPE1:
         if ((status = er_tcam_init_type1(unit, mcer->ext_table_cfg)) < 0)
             {
             /* reset the external TCAM if device NOT ready */

             if (retryCnt++ < 3)
                 goto retry;
             else
                 return status;
             }

        sal_memset(soc_mem_entry_null(unit, L3_DEFIP_CAMm), 0,
                   soc_mem_entry_bytes(unit, L3_DEFIP_CAMm));
        sal_memset(soc_mem_entry_null(unit, L3_DEFIP_TCAMm), 0,
                   soc_mem_entry_bytes(unit, L3_DEFIP_TCAMm));
        sal_memset(soc_mem_entry_null(unit, FP_EXTERNALm), 0,
                   soc_mem_entry_bytes(unit, FP_EXTERNALm));
        sal_memset(soc_mem_entry_null(unit, FP_TCAM_EXTERNALm), 0,
                   soc_mem_entry_bytes(unit, FP_TCAM_EXTERNALm));

        if (tcam_info->subtype == 2) {
            /* subtype 2 is XY TCAM */
            field_buf[0] = 0xffffffff;
            field_buf[1] = 0xffffffff;
            field_buf[2] = 0xffffffff;
            field_buf[3] = 0xffffffff;
            field_buf[4] = 0xffff;

            soc_mem_field32_set(unit, L3_DEFIP_CAMm,
                                soc_mem_entry_null(unit, L3_DEFIP_CAMm),
                                KEY_VALIDf, 1);
            soc_mem_field32_set(unit, L3_DEFIP_CAMm,
                                soc_mem_entry_null(unit, L3_DEFIP_CAMm),
                                KEY_V6f, 1);
            soc_mem_field32_set(unit, L3_DEFIP_CAMm,
                                soc_mem_entry_null(unit, L3_DEFIP_CAMm),
                                PREFIX_LENGTHf, 0x3f);
            soc_mem_field_set(unit, L3_DEFIP_CAMm,
                              soc_mem_entry_null(unit, L3_DEFIP_CAMm),
                              IP_ADDR_V6f, field_buf);

            soc_mem_field32_set(unit, L3_DEFIP_TCAMm,
                                soc_mem_entry_null(unit, L3_DEFIP_TCAMm),
                                KEY_VALIDf, 1);
            soc_mem_field32_set(unit, L3_DEFIP_TCAMm,
                                soc_mem_entry_null(unit, L3_DEFIP_TCAMm),
                                KEY_V6f, 1);
            soc_mem_field32_set(unit, L3_DEFIP_TCAMm,
                                soc_mem_entry_null(unit, L3_DEFIP_TCAMm),
                                PREFIX_LENGTHf, 0x3f);
            soc_mem_field_set(unit, L3_DEFIP_TCAMm,
                              soc_mem_entry_null(unit, L3_DEFIP_TCAMm),
                              IP_ADDR_V6f, field_buf);

            soc_mem_field_set(unit, FP_EXTERNALm,
                              soc_mem_entry_null(unit, FP_EXTERNALm),
                              KEYf, field_buf);
            soc_mem_field_set(unit, FP_TCAM_EXTERNALm,
                              soc_mem_entry_null(unit, FP_TCAM_EXTERNALm),
                              KEYf, field_buf);
        }

        if (sop->memState[L3_DEFIP_CAMm].index_max > 0) {
            /* Write 2 blanks to DEFIP CAM table for proper operation */
            SOC_IF_ERROR_RETURN
                (WRITE_L3_DEFIP_CAMm(unit, SOC_BLOCK_ALL, 0,
                                     soc_mem_entry_null(unit,
                                                        L3_DEFIP_TCAMm)));
            SOC_IF_ERROR_RETURN
                (WRITE_L3_DEFIP_CAMm(unit, SOC_BLOCK_ALL, 1,
                                     soc_mem_entry_null(unit,
                                                        L3_DEFIP_TCAMm)));
        }
        break;
    case ER_EXT_TCAM_TYPE2:
         if ((status = er_tcam_init_type2(unit, mcer->ext_table_cfg)) < 0)
             {
             /* reset the external TCAM if device NOT ready */
             if (retryCnt++ < 3)
                 goto retry;
             else
                return status;
             }
        break;
    default:
        return SOC_E_INTERNAL;
    }


    return SOC_E_NONE;
}

STATIC int
soc_easyrider_tcam_read_entry(int u, int part, int index,
                              uint32 * mask, uint32 * data, int * valid)
{
    soc_persist_t * sop;
    sop_memcfg_er_t * mcer;
    int cfg;

    sop = SOC_PERSIST(u);
    mcer = &(sop->er_memcfg);
    cfg = sop->er_memcfg.ext_table_cfg;

    switch (mcer->tcam_select) {
    case ER_EXT_TCAM_NONE:
        return SOC_E_NONE;
    case ER_EXT_TCAM_TYPE1:
        return soc_er_tcam_type1_read_entry(u, cfg, part, index, mask, data,
                                            valid);
    case ER_EXT_TCAM_TYPE2:
        return soc_er_tcam_type2_read_entry(u, cfg, part, index, mask, data,
                                            valid);
    default:
        return SOC_E_INTERNAL;
    }

    return SOC_E_NONE;
}

STATIC int
soc_easyrider_tcam_write_entry(int u, int part, int index,
                               uint32 * mask, uint32 * data)
{
    soc_persist_t * sop;
    sop_memcfg_er_t * mcer;
    int cfg;

    sop = SOC_PERSIST(u);
    mcer = &(sop->er_memcfg);
    cfg = sop->er_memcfg.ext_table_cfg;

    switch (mcer->tcam_select) {
    case ER_EXT_TCAM_NONE:
        return SOC_E_NONE;
    case ER_EXT_TCAM_TYPE1:
        return soc_er_tcam_type1_write_entry(u, cfg, part, index, mask, data);
    case ER_EXT_TCAM_TYPE2:
        return soc_er_tcam_type2_write_entry(u, cfg, part, index, mask, data);
    default:
        return SOC_E_INTERNAL;
    }

    return SOC_E_NONE;
}

STATIC int
soc_easyrider_tcam_set_valid(int u, int part, int index, int valid)
{
    soc_persist_t * sop;
    sop_memcfg_er_t * mcer;
    int cfg;

    sop = SOC_PERSIST(u);
    mcer = &(sop->er_memcfg);
    cfg = sop->er_memcfg.ext_table_cfg;

    switch (mcer->tcam_select) {
    case ER_EXT_TCAM_NONE:
        return SOC_E_NONE;
    case ER_EXT_TCAM_TYPE1:
        return soc_er_tcam_type1_set_valid(u, cfg, part, index, valid);
    case ER_EXT_TCAM_TYPE2:
        return soc_er_tcam_type2_set_valid(u, cfg, part, index, valid);
    default:
        return SOC_E_INTERNAL;
    }

    return SOC_E_NONE;
}

STATIC int
soc_easyrider_tcam_search_entry(int u, int part, int upper, uint32 *data,
                                int *index)
{
    soc_persist_t * sop;
    sop_memcfg_er_t * mcer;
    int cfg;

    sop = SOC_PERSIST(u);
    mcer = &(sop->er_memcfg);
    cfg = sop->er_memcfg.ext_table_cfg;

    switch (mcer->tcam_select) {
    case ER_EXT_TCAM_NONE:
        return SOC_E_NONE;
    case ER_EXT_TCAM_TYPE1:
        return soc_er_tcam_type1_search_entry(u, cfg, part, upper, data,
                                              index);
    case ER_EXT_TCAM_TYPE2:
        return SOC_E_UNAVAIL;
    default:
        return SOC_E_INTERNAL;
    }

    return SOC_E_NONE;
}

STATIC int
soc_easyrider_tcam_write_ib(int u, uint32 addr, uint32 data)
{
    soc_persist_t * sop;
    sop_memcfg_er_t * mcer;

    sop = SOC_PERSIST(u);
    mcer = &(sop->er_memcfg);

    switch (mcer->tcam_select) {
    case ER_EXT_TCAM_NONE:
        return SOC_E_NONE;
    case ER_EXT_TCAM_TYPE1:
        return soc_er_tcam_type1_write_ib(u, addr, data);
    case ER_EXT_TCAM_TYPE2:
        return SOC_E_UNAVAIL;
    default:
        return SOC_E_INTERNAL;
    }

    return SOC_E_NONE;
}

STATIC int
soc_easyrider_tcam_write_ima(int u, uint32 addr, uint32 d0_msb, uint32 d1_lsb)
{
    soc_persist_t * sop;
    sop_memcfg_er_t * mcer;

    sop = SOC_PERSIST(u);
    mcer = &(sop->er_memcfg);

    switch (mcer->tcam_select) {
    case ER_EXT_TCAM_NONE:
        return SOC_E_NONE;
    case ER_EXT_TCAM_TYPE1:
        return soc_er_tcam_type1_write_ima(u, addr, d0_msb, d1_lsb);
    case ER_EXT_TCAM_TYPE2:
        return SOC_E_UNAVAIL;
    default:
        return SOC_E_INTERNAL;
    }

    return SOC_E_NONE;
}

STATIC int
soc_easyrider_tcam_read_ima(int u, uint32 addr, uint32 *d0_msb, uint32 *d1_lsb)
{
    soc_persist_t * sop;
    sop_memcfg_er_t * mcer;

    sop = SOC_PERSIST(u);
    mcer = &(sop->er_memcfg);

    switch (mcer->tcam_select) {
    case ER_EXT_TCAM_NONE:
        return SOC_E_NONE;
    case ER_EXT_TCAM_TYPE1:
        return soc_er_tcam_type1_read_ima(u, addr, d0_msb, d1_lsb);
    case ER_EXT_TCAM_TYPE2:
        return SOC_E_UNAVAIL;
    default:
        return SOC_E_INTERNAL;
    }

    return SOC_E_NONE;
}

STATIC int
soc_easyrider_tcam_write_dbreg(int u, uint32 addr, uint32 d0_msb,
                               uint32 d1_lsb)
{
    soc_persist_t * sop;
    sop_memcfg_er_t * mcer;

    sop = SOC_PERSIST(u);
    mcer = &(sop->er_memcfg);

    switch (mcer->tcam_select) {
    case ER_EXT_TCAM_NONE:
        return SOC_E_NONE;
    case ER_EXT_TCAM_TYPE1:
        return soc_er_tcam_type1_write_dbreg(u, addr, d0_msb, d1_lsb);
    case ER_EXT_TCAM_TYPE2:
        return SOC_E_UNAVAIL;
    default:
        return SOC_E_INTERNAL;
    }

    return SOC_E_NONE;
}

STATIC int
soc_easyrider_tcam_read_dbreg(int u, uint32 addr, uint32 *d0_msb,
                              uint32 *d1_lsb)
{
    soc_persist_t * sop;
    sop_memcfg_er_t * mcer;

    sop = SOC_PERSIST(u);
    mcer = &(sop->er_memcfg);

    switch (mcer->tcam_select) {
    case ER_EXT_TCAM_NONE:
        return SOC_E_NONE;
    case ER_EXT_TCAM_TYPE1:
        return soc_er_tcam_type1_read_dbreg(u, addr, d0_msb, d1_lsb);
    case ER_EXT_TCAM_TYPE2:
        return SOC_E_UNAVAIL;
    default:
        return SOC_E_INTERNAL;
    }

    return SOC_E_NONE;
}

STATIC int
soc_easyrider_tcam_get_info(int u, int *type, int *subtype, int *dc_val,
                            soc_tcam_info_t **tcam_info)
{
    soc_control_t *soc;
    soc_persist_t *sop;
    sop_memcfg_er_t *mcer;

    soc = SOC_CONTROL(u);
    sop = SOC_PERSIST(u);
    mcer = &(sop->er_memcfg);

    if (type != NULL) {
        *type = mcer->tcam_select;
    }

    if (subtype != NULL) {
        switch (mcer->tcam_select) {
        case ER_EXT_TCAM_TYPE1:
            *subtype = ((soc_tcam_info_t *)soc->tcam_info)->subtype;
            if (dc_val != NULL) {
                *dc_val = 1;
            }
            break;
        default:
            *subtype = 0;
            break;
        }
    }

    if (tcam_info != NULL) {
        *tcam_info = soc->tcam_info;
    }

    return SOC_E_NONE;
}
#endif

#ifdef BCM_TRIUMPH_SUPPORT
STATIC int
soc_triumph_tcam_init(int unit)
{
    return tr_tcam_init_type1(unit);
}

STATIC int
soc_triumph_tcam_write_entry(int unit, int part, int index,
                             uint32 *mask, uint32 *data)
{
    return soc_tr_tcam_type1_write_entry(unit, part, index, mask, data);
}

STATIC int
soc_triumph_tcam_read_entry(int unit, int part, int index,
                            uint32 *mask, uint32 *data, int *valid)
{
    return soc_tr_tcam_type1_read_entry(unit, part, index, mask, data, valid);
}

STATIC int
soc_triumph_tcam_search_entry(int unit, int part1, int part2, uint32 *data,
                              int *index1, int *index2)
{
    return soc_tr_tcam_type1_search_entry(unit, part1, part2, data, index1,
                                          index2);
}

STATIC int
soc_triumph_tcam_write_reg(int unit, uint32 addr, uint32 d0_msb, uint32 d1,
                           uint32 d2_lsb)
{
    return soc_tr_tcam_type1_write_reg(unit, addr, d0_msb, d1, d2_lsb);
}

STATIC int
soc_triumph_tcam_read_reg(int unit, uint32 addr, uint32 *d0_msb, uint32 *d1,
                          uint32 *d2_lsb)
{
    return soc_tr_tcam_type1_read_reg(unit, addr, d0_msb, d1, d2_lsb);
}

STATIC int
soc_triumph_tcam_get_info(int u, int *type, int *subtype, int *dc_val,
                          soc_tcam_info_t **tcam_info)
{
    soc_control_t *soc;

    soc = SOC_CONTROL(u);
    if (soc->tcam_info == NULL) {
        return SOC_E_INIT;
    }

    if (type != NULL) {
        *type = 1;
    }

    if (subtype != NULL) {
        *subtype = ((soc_tcam_info_t *)soc->tcam_info)->subtype;

    }

    if (dc_val != NULL) {
        *dc_val = 1;
    }

    if (tcam_info != NULL) {
        *tcam_info = soc->tcam_info;
    }

    return SOC_E_NONE;
}

/*
 * Partition to tcam memory name mapping
 * TCAM_PARTITION_FWD_L2    EXT_L2_ENTRY_TCAM
 * TCAM_PARTITION_FWD_IP4   EXT_IPV4_DEFIP_TCAM
 * TCAM_PARTITION_FWD_IP6U  EXT_IPV6_64_DEFIP_TCAM
 * TCAM_PARTITION_FWD_IP6   EXT_IPV6_128_DEFIP_TCAM
 * TCAM_PARTITION_ACL_L2    EXT_ACL288_TCAM (part 1)
 * TCAM_PARTITION_ACL_IP4   EXT_ACL288_TCAM (part 2)
 * TCAM_PARTITION_ACL_IP6S  EXT_ACL360_TCAM_DATA, EXT_ACL360_TCAM_MASK
 * TCAM_PARTITION_ACL_IP6F  EXT_ACL432_TCAM_DATA, EXT_ACL432_TCAM_MASK (part 1)
 * TCAM_PARTITION_ACL_L2C   EXT_ACL144_TCAM (part 1)
 * TCAM_PARTITION_ACL_IP4C  EXT_ACL144_TCAM (part 2)
 * TCAM_PARTITION_ACL_IP6C  EXT_ACL144_TCAM (part 3)
 * TCAM_PARTITION_ACL_L2IP4 EXT_ACL432_TCAM_DATA, EXT_ACL432_TCAM_MASK (part 2)
 * TCAM_PARTITION_ACL_L2IP6 EXT_ACL432_TCAM_DATA, EXT_ACL432_TCAM_MASK (part 3)
 *
 * partition to assiciated data memory name mapping
 * TCAM_PARTITION_FWD_L2    EXT_L2_ENTRY_DATA
 * TCAM_PARTITION_FWD_IP4   EXT_DEFIP_DATA (part 1)
 * TCAM_PARTITION_FWD_IP6U  EXT_DEFIP_DATA (part 2)
 * TCAM_PARTITION_FWD_IP6   EXT_DEFIP_DATA (part 3)
 * TCAM_PARTITION_ACL_L2    EXT_FP_POLICY (part 1)
 * TCAM_PARTITION_ACL_IP4   EXT_FP_POLICY (part 2)
 * TCAM_PARTITION_ACL_IP6S  EXT_FP_POLICY (part 3)
 * TCAM_PARTITION_ACL_IP6F  EXT_FP_POLICY (part 4)
 * TCAM_PARTITION_ACL_L2C   EXT_FP_POLICY (part 5)
 * TCAM_PARTITION_ACL_IP4C  EXT_FP_POLICY (part 6)
 * TCAM_PARTITION_ACL_IP6C  EXT_FP_POLICY (part 7)
 * TCAM_PARTITION_ACL_L2IP4 EXT_FP_POLICY (part 8)
 * TCAM_PARTITION_ACL_L2IP6 EXT_FP_POLICY (part 9)
 *
 * Note: if there are multiple partitions in a single memory table, logical
 *       index calculation does not count padded entries. For example if the
 *       system is configured to have 100 ACL_L2 entries and 4096 ACL_IP4
 *       entries. The corresponding logical index for ACL_L2 in EXT_FP_POLICY
 *       is from 0 to 99, and the logical index for ACL_IP4 in EXT_FP_POLICY
 *       is from 100 to 4195 instead of 4096 to 8191, 3996 padded ACL_L2
 *       entries in both TCAM and SRAM are not counted.
 */
STATIC int
soc_triumph_tcam_part_index_to_mem_index(int unit,
                                         soc_tcam_partition_type_t part,
                                         int part_index,
                                         soc_mem_t mem,
                                         int *mem_index)
{
    soc_tcam_info_t *tcam_info;
    soc_tcam_partition_t *partitions;

    tcam_info = SOC_CONTROL(unit)->tcam_info;
    if (tcam_info == NULL) {
        return SOC_E_INIT;
    }
    partitions = tcam_info->partitions;

    if (part_index >= partitions[part].num_entries) {
        return SOC_E_PARAM;
    }

    switch (part) {
    case TCAM_PARTITION_FWD_L2:
        switch (mem) {
     /* case EXT_L2_ENTRYm: this view does not support read/write operation */
        case EXT_L2_ENTRY_TCAMm:
        case EXT_L2_ENTRY_DATAm:
            *mem_index = part_index;
            break;
        case EXT_SRC_HIT_BITSm:
        case EXT_SRC_HIT_BITS_L2m:
        case EXT_DST_HIT_BITSm:
        case EXT_DST_HIT_BITS_L2m:
            *mem_index = part_index / 32;
            break;
        default:
            return SOC_E_PARAM;
        }
        break;
    case TCAM_PARTITION_FWD_IP4:
        switch (mem) {
        case EXT_IPV4_DEFIPm:
        case EXT_IPV4_DEFIP_TCAMm:
        case EXT_DEFIP_DATAm:
        case EXT_DEFIP_DATA_IPV4m:
            *mem_index = part_index;
            break;
        case EXT_SRC_HIT_BITS_IPV4m:
        case EXT_DST_HIT_BITS_IPV4m:
            *mem_index = part_index / 32;
            break;
        case EXT_SRC_HIT_BITSm:
        case EXT_DST_HIT_BITSm:
            *mem_index =
                (partitions[TCAM_PARTITION_FWD_L2].num_entries + 31) / 32 +
                part_index / 32;
            break;
        default:
            return SOC_E_PARAM;
        }
        break;
    case TCAM_PARTITION_FWD_IP6U:
        switch (mem) {
        case EXT_IPV6_64_DEFIPm:
        case EXT_IPV6_64_DEFIP_TCAMm:
        case EXT_DEFIP_DATA_IPV6_64m:
            *mem_index = part_index;
            break;
        case EXT_SRC_HIT_BITS_IPV6_64m:
        case EXT_DST_HIT_BITS_IPV6_64m:
            *mem_index = part_index / 32;
            break;
        case EXT_DEFIP_DATAm:
            *mem_index = partitions[TCAM_PARTITION_FWD_IP4].num_entries +
                part_index;
            break;
        case EXT_SRC_HIT_BITSm:
        case EXT_DST_HIT_BITSm:
            *mem_index =
                (partitions[TCAM_PARTITION_FWD_L2].num_entries + 31) / 32 +
                (partitions[TCAM_PARTITION_FWD_IP4].num_entries + 31) / 32 +
                part_index / 32;
            break;
        default:
            return SOC_E_PARAM;
        }
        break;
    case TCAM_PARTITION_FWD_IP6:
        switch (mem) {
        case EXT_IPV6_128_DEFIPm:
        case EXT_IPV6_128_DEFIP_TCAMm:
        case EXT_DEFIP_DATA_IPV6_128m:
            *mem_index = part_index;
            break;
        case EXT_SRC_HIT_BITS_IPV6_128m:
        case EXT_DST_HIT_BITS_IPV6_128m:
            *mem_index = part_index / 32;
            break;
        case EXT_DEFIP_DATAm:
            *mem_index = partitions[TCAM_PARTITION_FWD_IP4].num_entries +
                partitions[TCAM_PARTITION_FWD_IP6U].num_entries +
                part_index;
            break;
        case EXT_SRC_HIT_BITSm:
        case EXT_DST_HIT_BITSm:
            *mem_index =
                (partitions[TCAM_PARTITION_FWD_L2].num_entries + 31) / 32 +
                (partitions[TCAM_PARTITION_FWD_IP4].num_entries + 31) / 32 +
                (partitions[TCAM_PARTITION_FWD_IP6U].num_entries + 31) / 32 +
                part_index / 32;
            break;
        default:
            return SOC_E_PARAM;
        }
        break;
    case TCAM_PARTITION_ACL_L2:
        switch (mem) {
        case EXT_ACL288_TCAMm:
        case EXT_ACL288_TCAM_L2m:
        case EXT_FP_POLICYm:
        case EXT_FP_POLICY_ACL288_L2m:
        case EXT_FP_CNTRm:
        case EXT_FP_CNTR_ACL288_L2m:
            *mem_index = part_index;
            break;
        case EXT_FP_CNTR8m:
        case EXT_FP_CNTR8_ACL288_L2m:
            *mem_index = part_index / 8;
            break;
        default:
            return SOC_E_PARAM;
        }
        break;
    case TCAM_PARTITION_ACL_IP4:
        switch (mem) {
        case EXT_ACL288_TCAM_IPV4m:
        case EXT_FP_POLICY_ACL288_IPV4m:
        case EXT_FP_CNTR_ACL288_IPV4m:
            *mem_index = part_index;
            break;
        case EXT_FP_CNTR8_ACL288_IPV4m:
            *mem_index = part_index / 8;
            break;
        case EXT_ACL288_TCAMm:
        case EXT_FP_POLICYm:
        case EXT_FP_CNTRm:
            *mem_index = partitions[TCAM_PARTITION_ACL_L2].num_entries +
                part_index;
            break;
        case EXT_FP_CNTR8m:
            *mem_index =
                (partitions[TCAM_PARTITION_ACL_L2].num_entries + 7) / 8 +
                part_index / 8;
            break;
        default:
            return SOC_E_PARAM;
        }
        break;
    case TCAM_PARTITION_ACL_IP6S:
        switch (mem) {
        case EXT_ACL360_TCAM_DATAm:
        case EXT_ACL360_TCAM_DATA_IPV6_SHORTm:
        case EXT_FP_POLICY_ACL360_IPV6_SHORTm:
        case EXT_FP_CNTR_ACL360_IPV6_SHORTm:
            *mem_index = part_index;
            break;
        case EXT_FP_CNTR8_ACL360_IPV6_SHORTm:
            *mem_index = part_index / 8;
            break;
        case EXT_ACL360_TCAM_MASKm:
            *mem_index = 0;
            break;
        case EXT_FP_POLICYm:
        case EXT_FP_CNTRm:
            *mem_index = partitions[TCAM_PARTITION_ACL_L2].num_entries +
                partitions[TCAM_PARTITION_ACL_IP4].num_entries +
                part_index;
            break;
        case EXT_FP_CNTR8m:
            *mem_index =
                (partitions[TCAM_PARTITION_ACL_L2].num_entries + 7) / 8 +
                (partitions[TCAM_PARTITION_ACL_IP4].num_entries + 7) / 8 +
                part_index / 8;
            break;
        default:
            return SOC_E_PARAM;
        }
        break;
    case TCAM_PARTITION_ACL_IP6F:
        switch (mem) {
        case EXT_ACL432_TCAM_DATAm:
        case EXT_ACL432_TCAM_DATA_IPV6_LONGm:
        case EXT_FP_POLICY_ACL432_IPV6_LONGm:
        case EXT_FP_CNTR_ACL432_IPV6_LONGm:
            *mem_index = part_index;
            break;
        case EXT_FP_CNTR8_ACL432_IPV6_LONGm:
            *mem_index = part_index / 8;
            break;
        case EXT_ACL432_TCAM_MASKm:
            *mem_index = 0;
            break;
        case EXT_FP_POLICYm:
        case EXT_FP_CNTRm:
            *mem_index = partitions[TCAM_PARTITION_ACL_L2].num_entries +
                partitions[TCAM_PARTITION_ACL_IP4].num_entries +
                partitions[TCAM_PARTITION_ACL_IP6S].num_entries +
                part_index;
            break;
        case EXT_FP_CNTR8m:
            *mem_index =
                (partitions[TCAM_PARTITION_ACL_L2].num_entries + 7) / 8 +
                (partitions[TCAM_PARTITION_ACL_IP4].num_entries + 7) / 8 +
                (partitions[TCAM_PARTITION_ACL_IP6S].num_entries + 7) / 8 +
                part_index / 8;
            break;
        default:
            return SOC_E_PARAM;
        }
        break;
    case TCAM_PARTITION_ACL_L2C:
        switch (mem) {
        case EXT_ACL144_TCAMm:
        case EXT_ACL144_TCAM_L2m:
        case EXT_FP_POLICY_ACL144_L2m:
        case EXT_FP_CNTR_ACL144_L2m:
            *mem_index = part_index;
            break;
        case EXT_FP_CNTR8_ACL144_L2m:
            *mem_index = part_index / 8;
            break;
        case EXT_FP_POLICYm:
        case EXT_FP_CNTRm:
            *mem_index = partitions[TCAM_PARTITION_ACL_L2].num_entries +
                partitions[TCAM_PARTITION_ACL_IP4].num_entries +
                partitions[TCAM_PARTITION_ACL_IP6S].num_entries +
                partitions[TCAM_PARTITION_ACL_IP6F].num_entries +
                part_index;
            break;
        case EXT_FP_CNTR8m:
            *mem_index =
                (partitions[TCAM_PARTITION_ACL_L2].num_entries + 7) / 8 +
                (partitions[TCAM_PARTITION_ACL_IP4].num_entries + 7) / 8 +
                (partitions[TCAM_PARTITION_ACL_IP6S].num_entries + 7) / 8 +
                (partitions[TCAM_PARTITION_ACL_IP6F].num_entries + 7) / 8 +
                part_index / 8;
            break;
        default:
            return SOC_E_PARAM;
        }
        break;
    case TCAM_PARTITION_ACL_IP4C:
        switch (mem) {
        case EXT_ACL144_TCAM_IPV4m:
        case EXT_FP_POLICY_ACL144_IPV4m:
        case EXT_FP_CNTR_ACL144_IPV4m:
            *mem_index = part_index;
            break;
        case EXT_FP_CNTR8_ACL144_IPV4m:
            *mem_index = part_index / 8;
            break;
        case EXT_ACL144_TCAMm:
            *mem_index = partitions[TCAM_PARTITION_ACL_L2C].num_entries +
                part_index;
            break;
        case EXT_FP_POLICYm:
        case EXT_FP_CNTRm:
            *mem_index = partitions[TCAM_PARTITION_ACL_L2].num_entries +
                partitions[TCAM_PARTITION_ACL_IP4].num_entries +
                partitions[TCAM_PARTITION_ACL_IP6S].num_entries +
                partitions[TCAM_PARTITION_ACL_IP6F].num_entries +
                partitions[TCAM_PARTITION_ACL_L2C].num_entries +
                part_index;
            break;
        case EXT_FP_CNTR8m:
            *mem_index =
                (partitions[TCAM_PARTITION_ACL_L2].num_entries + 7) / 8 +
                (partitions[TCAM_PARTITION_ACL_IP4].num_entries + 7) / 8 +
                (partitions[TCAM_PARTITION_ACL_IP6S].num_entries + 7) / 8 +
                (partitions[TCAM_PARTITION_ACL_IP6F].num_entries + 7) / 8 +
                (partitions[TCAM_PARTITION_ACL_L2C].num_entries + 7) / 8 +
                part_index / 8;
            break;
        default:
            return SOC_E_PARAM;
        }
        break;
    case TCAM_PARTITION_ACL_IP6C:
        switch (mem) {
        case EXT_ACL144_TCAM_IPV6m:
        case EXT_FP_POLICY_ACL144_IPV6m:
        case EXT_FP_CNTR_ACL144_IPV6m:
            *mem_index = part_index;
            break;
        case EXT_FP_CNTR8_ACL144_IPV6m:
            *mem_index = part_index / 8;
            break;
        case EXT_ACL144_TCAMm:
            *mem_index = partitions[TCAM_PARTITION_ACL_L2C].num_entries +
                partitions[TCAM_PARTITION_ACL_IP4C].num_entries +
                part_index;
            break;
        case EXT_FP_POLICYm:
        case EXT_FP_CNTRm:
            *mem_index = partitions[TCAM_PARTITION_ACL_L2].num_entries +
                partitions[TCAM_PARTITION_ACL_IP4].num_entries +
                partitions[TCAM_PARTITION_ACL_IP6S].num_entries +
                partitions[TCAM_PARTITION_ACL_IP6F].num_entries +
                partitions[TCAM_PARTITION_ACL_L2C].num_entries +
                partitions[TCAM_PARTITION_ACL_IP4C].num_entries +
                part_index;
            break;
        case EXT_FP_CNTR8m:
            *mem_index =
                (partitions[TCAM_PARTITION_ACL_L2].num_entries + 7) / 8 +
                (partitions[TCAM_PARTITION_ACL_IP4].num_entries + 7) / 8 +
                (partitions[TCAM_PARTITION_ACL_IP6S].num_entries + 7) / 8 +
                (partitions[TCAM_PARTITION_ACL_IP6F].num_entries + 7) / 8 +
                (partitions[TCAM_PARTITION_ACL_L2C].num_entries + 7) / 8 +
                (partitions[TCAM_PARTITION_ACL_IP4C].num_entries + 7) / 8 +
                part_index / 8;
            break;
        default:
            return SOC_E_PARAM;
        }
        break;
    case TCAM_PARTITION_ACL_L2IP4:
        switch (mem) {
        case EXT_ACL432_TCAM_DATA_L2_IPV4m:
        case EXT_FP_POLICY_ACL432_L2_IPV4m:
        case EXT_FP_CNTR_ACL432_L2_IPV4m:
            *mem_index = part_index;
            break;
        case EXT_FP_CNTR8_ACL432_L2_IPV4m:
            *mem_index = part_index / 8;
            break;
        case EXT_ACL432_TCAM_DATAm:
            *mem_index = partitions[TCAM_PARTITION_ACL_IP6F].num_entries +
                part_index;
            break;
        case EXT_ACL432_TCAM_MASKm:
            *mem_index = 0;
            break;
        case EXT_FP_POLICYm:
        case EXT_FP_CNTRm:
            *mem_index = partitions[TCAM_PARTITION_ACL_L2].num_entries +
                partitions[TCAM_PARTITION_ACL_IP4].num_entries +
                partitions[TCAM_PARTITION_ACL_IP6S].num_entries +
                partitions[TCAM_PARTITION_ACL_IP6F].num_entries +
                partitions[TCAM_PARTITION_ACL_L2C].num_entries +
                partitions[TCAM_PARTITION_ACL_IP4C].num_entries +
                partitions[TCAM_PARTITION_ACL_IP6C].num_entries +
                part_index;
            break;
        case EXT_FP_CNTR8m:
            *mem_index =
                (partitions[TCAM_PARTITION_ACL_L2].num_entries + 7) / 8 +
                (partitions[TCAM_PARTITION_ACL_IP4].num_entries + 7) / 8 +
                (partitions[TCAM_PARTITION_ACL_IP6S].num_entries + 7) / 8 +
                (partitions[TCAM_PARTITION_ACL_IP6F].num_entries + 7) / 8 +
                (partitions[TCAM_PARTITION_ACL_L2C].num_entries + 7) / 8 +
                (partitions[TCAM_PARTITION_ACL_IP4C].num_entries + 7) / 8 +
                (partitions[TCAM_PARTITION_ACL_IP6C].num_entries + 7) / 8 +
                part_index / 8;
            break;
        default:
            return SOC_E_PARAM;
        }
        break;
    case TCAM_PARTITION_ACL_L2IP6:
        switch (mem) {
        case EXT_ACL432_TCAM_DATA_L2_IPV6m:
        case EXT_FP_POLICY_ACL432_L2_IPV6m:
        case EXT_FP_CNTR_ACL432_L2_IPV6m:
            *mem_index = part_index;
            break;
        case EXT_FP_CNTR8_ACL432_L2_IPV6m:
            *mem_index = part_index / 8;
            break;
        case EXT_ACL432_TCAM_DATAm:
            *mem_index = partitions[TCAM_PARTITION_ACL_IP6F].num_entries +
                partitions[TCAM_PARTITION_ACL_L2IP4].num_entries +
                part_index;
            break;
        case EXT_ACL432_TCAM_MASKm:
            *mem_index = 0;
            break;
        case EXT_FP_POLICYm:
        case EXT_FP_CNTRm:
            *mem_index = partitions[TCAM_PARTITION_ACL_L2].num_entries +
                partitions[TCAM_PARTITION_ACL_IP4].num_entries +
                partitions[TCAM_PARTITION_ACL_IP6S].num_entries +
                partitions[TCAM_PARTITION_ACL_IP6F].num_entries +
                partitions[TCAM_PARTITION_ACL_L2C].num_entries +
                partitions[TCAM_PARTITION_ACL_IP4C].num_entries +
                partitions[TCAM_PARTITION_ACL_IP6C].num_entries +
                partitions[TCAM_PARTITION_ACL_L2IP4].num_entries +
                part_index;
            break;
        case EXT_FP_CNTR8m:
            *mem_index =
                (partitions[TCAM_PARTITION_ACL_L2].num_entries + 7) / 8 +
                (partitions[TCAM_PARTITION_ACL_IP4].num_entries + 7) / 8 +
                (partitions[TCAM_PARTITION_ACL_IP6S].num_entries + 7) / 8 +
                (partitions[TCAM_PARTITION_ACL_IP6F].num_entries + 7) / 8 +
                (partitions[TCAM_PARTITION_ACL_L2C].num_entries + 7) / 8 +
                (partitions[TCAM_PARTITION_ACL_IP4C].num_entries + 7) / 8 +
                (partitions[TCAM_PARTITION_ACL_IP6C].num_entries + 7) / 8 +
                (partitions[TCAM_PARTITION_ACL_L2IP4].num_entries + 7) / 8 +
                part_index / 8;
            break;
        default:
            return SOC_E_PARAM;
        }
        break;
    default:
        return SOC_E_PARAM;
    }

    return SOC_E_NONE;
}

/*
 * Following are a list of memory that may require index translation.
 * These tables are addressed by the raw TCAM index of its corresponding
 * TCAM table.
 *
 * EXT_SRC_HIT_BITS
 * EXT_SRC_HIT_BITS_L2 (software view)
 * EXT_SRC_HIT_BITS_IPV4 (software view)
 * EXT_SRC_HIT_BITS_IPV6_64 (software view)
 * EXT_SRC_HIT_BITS_IPV6_128 (software view)
 *
 * EXT_DST_HIT_BITS
 * EXT_DST_HIT_BITS_L2 (software view)
 * EXT_DST_HIT_BITS_IPV4 (software view)
 * EXT_DST_HIT_BITS_IPV6_64 (software view)
 * EXT_DST_HIT_BITS_IPV6_128 (software view)
 *
 * EXT_L2_ENTRY (TCAM + SRAM + HBIT combine view, no read/write support)
 * EXT_L2_ENTRY_TCAM
 * EXT_L2_ENTRY_DATA
 *
 * EXT_IPV4_DEFIP (TCAM + SRAM + HBIT combine view)
 * EXT_IPV6_64_DEFIP (TCAM + SRAM + HBIT combine view)
 * EXT_IPV6_128_DEFIP (TCAM + SRAM + HBIT combine view)
 *
 * EXT_IPV4_DEFIP_TCAM
 * EXT_IPV6_64_DEFIP_TCAM
 * EXT_IPV6_128_DEFIP_TCAM
 *
 * EXT_DEFIP_DATA
 * EXT_DEFIP_DATA_IPV4 (software view)
 * EXT_DEFIP_DATA_IPV6_64 (software view)
 * EXT_DEFIP_DATA_IPV6_128 (software view)
 *
 * EXT_ACL144_TCAM
 * EXT_ACL288_TCAM
 * EXT_ACL360_TCAM_DATA
 * EXT_ACL360_TCAM_MASK (has only 1 entry)
 * EXT_ACL432_TCAM_DATA
 * EXT_ACL432_TCAM_MASK (has only 1 entry)
 *
 * EXT_ACL288_TCAM_L2 (software view)
 * EXT_ACL288_TCAM_IPV4 (software view)
 * EXT_ACL360_TCAM_DATA_IPV6_SHORT (software view)
 * EXT_ACL432_TCAM_DATA_IPV6_LONG (software view)
 * EXT_ACL144_TCAM_L2 (software view)
 * EXT_ACL144_TCAM_IPV4 (software view)
 * EXT_ACL144_TCAM_IPV6 (software view)
 * EXT_ACL432_TCAM_DATA_L2_IPV4 (software view)
 * EXT_ACL432_TCAM_DATA_L2_IPV6 (software view)
 *
 * EXT_FP_POLICY
 * EXT_FP_POLICY_ACL288_L2 (software view)
 * EXT_FP_POLICY_ACL288_IPV4 (software view)
 * EXT_FP_POLICY_ACL360_IPV6_SHORT (software view)
 * EXT_FP_POLICY_ACL432_IPV6_LONG (software view)
 * EXT_FP_POLICY_ACL144_L2 (software view)
 * EXT_FP_POLICY_ACL144_IPV4 (software view)
 * EXT_FP_POLICY_ACL144_IPV6 (software view)
 * EXT_FP_POLICY_ACL432_L2_IPV4 (software view)
 * EXT_FP_POLICY_ACL432_L2_IPV6 (software view)
 *
 * EXT_FP_CNTR
 * EXT_FP_CNTR_ACL288_L2 (software view)
 * EXT_FP_CNTR_ACL288_IPV4 (software view)
 * EXT_FP_CNTR_ACL360_IPV6_SHORT (software view)
 * EXT_FP_CNTR_ACL432_IPV6_LONG (software view)
 * EXT_FP_CNTR_ACL144_L2 (software view)
 * EXT_FP_CNTR_ACL144_IPV4 (software view)
 * EXT_FP_CNTR_ACL144_IPV6 (software view)
 * EXT_FP_CNTR_ACL432_L2_IPV4 (software view)
 * EXT_FP_CNTR_ACL432_L2_IPV6 (software view)
 *
 * EXT_FP_CNTR8
 * EXT_FP_CNTR8_ACL288_L2 (software view)
 * EXT_FP_CNTR8_ACL288_IPV4 (software view)
 * EXT_FP_CNTR8_ACL360_IPV6_SHORT (software view)
 * EXT_FP_CNTR8_ACL432_IPV6_LONG (software view)
 * EXT_FP_CNTR8_ACL144_L2 (software view)
 * EXT_FP_CNTR8_ACL144_IPV4 (software view)
 * EXT_FP_CNTR8_ACL144_IPV6 (software view)
 * EXT_FP_CNTR8_ACL432_L2_IPV4 (software view)
 * EXT_FP_CNTR8_ACL432_L2_IPV6 (software view)
 */

STATIC int
soc_triumph_tcam_mem_index_to_raw_index(int unit, soc_mem_t mem,
                                        int mem_index, soc_mem_t *real_mem,
                                        int *raw_index)
{
    soc_tcam_info_t *tcam_info;
    soc_tcam_partition_t *partitions;
    int part;
    int entries_per_index;

    /* convert the software invented table to real table */
    switch (mem) {
    case EXT_DEFIP_DATA_IPV4m:
    case EXT_DEFIP_DATA_IPV6_64m:
    case EXT_DEFIP_DATA_IPV6_128m:
        *real_mem = EXT_DEFIP_DATAm;
        break;
    case EXT_SRC_HIT_BITS_L2m:
    case EXT_SRC_HIT_BITS_IPV4m:
    case EXT_SRC_HIT_BITS_IPV6_64m:
    case EXT_SRC_HIT_BITS_IPV6_128m:
        *real_mem = EXT_SRC_HIT_BITSm;
        break;
    case EXT_DST_HIT_BITS_L2m:
    case EXT_DST_HIT_BITS_IPV4m:
    case EXT_DST_HIT_BITS_IPV6_64m:
    case EXT_DST_HIT_BITS_IPV6_128m:
        *real_mem = EXT_DST_HIT_BITSm;
        break;
    case EXT_ACL144_TCAM_L2m:
    case EXT_ACL144_TCAM_IPV4m:
    case EXT_ACL144_TCAM_IPV6m:
        *real_mem = EXT_ACL144_TCAMm;
        break;
    case EXT_ACL288_TCAM_L2m:
    case EXT_ACL288_TCAM_IPV4m:
        *real_mem = EXT_ACL288_TCAMm;
        break;
    case EXT_ACL360_TCAM_DATA_IPV6_SHORTm:
        *real_mem = EXT_ACL360_TCAM_DATAm;
        break;
    case EXT_ACL432_TCAM_DATA_IPV6_LONGm:
    case EXT_ACL432_TCAM_DATA_L2_IPV4m:
    case EXT_ACL432_TCAM_DATA_L2_IPV6m:
        *real_mem = EXT_ACL432_TCAM_DATAm;
        break;
    case EXT_FP_POLICY_ACL288_L2m:
    case EXT_FP_POLICY_ACL288_IPV4m:
    case EXT_FP_POLICY_ACL360_IPV6_SHORTm:
    case EXT_FP_POLICY_ACL432_IPV6_LONGm:
    case EXT_FP_POLICY_ACL144_L2m:
    case EXT_FP_POLICY_ACL144_IPV4m:
    case EXT_FP_POLICY_ACL144_IPV6m:
    case EXT_FP_POLICY_ACL432_L2_IPV4m:
    case EXT_FP_POLICY_ACL432_L2_IPV6m:
        *real_mem = EXT_FP_POLICYm;
        break;
    case EXT_FP_CNTR_ACL288_L2m:
    case EXT_FP_CNTR_ACL288_IPV4m:
    case EXT_FP_CNTR_ACL360_IPV6_SHORTm:
    case EXT_FP_CNTR_ACL432_IPV6_LONGm:
    case EXT_FP_CNTR_ACL144_L2m:
    case EXT_FP_CNTR_ACL144_IPV4m:
    case EXT_FP_CNTR_ACL144_IPV6m:
    case EXT_FP_CNTR_ACL432_L2_IPV4m:
    case EXT_FP_CNTR_ACL432_L2_IPV6m:
        *real_mem = EXT_FP_CNTRm;
        break;
    case EXT_FP_CNTR8_ACL288_L2m:
    case EXT_FP_CNTR8_ACL288_IPV4m:
    case EXT_FP_CNTR8_ACL360_IPV6_SHORTm:
    case EXT_FP_CNTR8_ACL432_IPV6_LONGm:
    case EXT_FP_CNTR8_ACL144_L2m:
    case EXT_FP_CNTR8_ACL144_IPV4m:
    case EXT_FP_CNTR8_ACL144_IPV6m:
    case EXT_FP_CNTR8_ACL432_L2_IPV4m:
    case EXT_FP_CNTR8_ACL432_L2_IPV6m:
        *real_mem = EXT_FP_CNTR8m;
        break;
    case EXT_L2_ENTRYm:
    case EXT_L2_ENTRY_TCAMm:
    case EXT_L2_ENTRY_DATAm:
    case EXT_IPV4_DEFIPm:
    case EXT_IPV4_DEFIP_TCAMm:
    case EXT_IPV6_64_DEFIPm:
    case EXT_IPV6_64_DEFIP_TCAMm:
    case EXT_IPV6_128_DEFIPm:
    case EXT_IPV6_128_DEFIP_TCAMm:
    case EXT_DEFIP_DATAm:
    case EXT_SRC_HIT_BITSm:
    case EXT_DST_HIT_BITSm:
    case EXT_ACL144_TCAMm:
    case EXT_ACL288_TCAMm:
    case EXT_ACL360_TCAM_DATAm:
    case EXT_ACL432_TCAM_DATAm:
    case EXT_FP_POLICYm:
    case EXT_FP_CNTRm:
    case EXT_FP_CNTR8m:
        *real_mem = mem;
        break;
    default:
        *real_mem = mem;
        *raw_index = mem_index;
        return SOC_E_NONE;
    }

    tcam_info = SOC_CONTROL(unit)->tcam_info;
    if (tcam_info == NULL) {
        return SOC_E_INIT;
    }
    partitions = tcam_info->partitions;
    part = TCAM_PARTITION_RAW;
    entries_per_index = 1;

    if (*real_mem == EXT_SRC_HIT_BITSm || *real_mem == EXT_DST_HIT_BITSm) {
        entries_per_index = 32;
    } else if (*real_mem == EXT_FP_CNTR8m) {
        entries_per_index = 8;
    }

    /* find partition and calculate logical index within partition */
    switch (mem) {
    case EXT_L2_ENTRYm:
    case EXT_L2_ENTRY_TCAMm:
    case EXT_L2_ENTRY_DATAm:
    case EXT_SRC_HIT_BITS_L2m:
    case EXT_DST_HIT_BITS_L2m:
        part = TCAM_PARTITION_FWD_L2;
        break;
    case EXT_IPV4_DEFIPm:
    case EXT_IPV4_DEFIP_TCAMm:
    case EXT_DEFIP_DATA_IPV4m:
    case EXT_SRC_HIT_BITS_IPV4m:
    case EXT_DST_HIT_BITS_IPV4m:
        part = TCAM_PARTITION_FWD_IP4;
        break;
    case EXT_IPV6_64_DEFIPm:
    case EXT_IPV6_64_DEFIP_TCAMm:
    case EXT_DEFIP_DATA_IPV6_64m:
    case EXT_SRC_HIT_BITS_IPV6_64m:
    case EXT_DST_HIT_BITS_IPV6_64m:
        part = TCAM_PARTITION_FWD_IP6U;
        break;
    case EXT_IPV6_128_DEFIPm:
    case EXT_IPV6_128_DEFIP_TCAMm:
    case EXT_DEFIP_DATA_IPV6_128m:
    case EXT_SRC_HIT_BITS_IPV6_128m:
    case EXT_DST_HIT_BITS_IPV6_128m:
        part = TCAM_PARTITION_FWD_IP6;
        break;
    case EXT_DEFIP_DATAm:
        for (part = TCAM_PARTITION_FWD_IP4; part < TCAM_PARTITION_FWD_IP6;
             part++) {
            if (mem_index < partitions[part].num_entries) {
                break;
            }
            mem_index -= partitions[part].num_entries;
        }
        break;
    case EXT_SRC_HIT_BITSm:
    case EXT_DST_HIT_BITSm:
        for (part = TCAM_PARTITION_FWD_L2; part < TCAM_PARTITION_FWD_IP6;
             part++) {
            if (mem_index < (partitions[part].num_entries + 31) / 32) {
                break;
            }
            mem_index -= (partitions[part].num_entries + 31) / 32;
        }
        break;
    case EXT_ACL144_TCAMm:
        if (mem_index < partitions[TCAM_PARTITION_ACL_L2C].num_entries) {
            part = TCAM_PARTITION_ACL_L2C;
            break;
        }
        mem_index -= partitions[TCAM_PARTITION_ACL_L2C].num_entries;
        if (mem_index < partitions[TCAM_PARTITION_ACL_IP4C].num_entries) {
            part = TCAM_PARTITION_ACL_IP4C;
            break;
        }
        mem_index -= partitions[TCAM_PARTITION_ACL_IP4C].num_entries;
        part = TCAM_PARTITION_ACL_IP6C;
        break;
    case EXT_ACL288_TCAMm:
        if (mem_index < partitions[TCAM_PARTITION_ACL_L2].num_entries) {
            part = TCAM_PARTITION_ACL_L2;
            break;
        }
        mem_index -= partitions[TCAM_PARTITION_ACL_L2].num_entries;
        part = TCAM_PARTITION_ACL_IP4;
        break;
    case EXT_ACL360_TCAM_DATAm:
        part = TCAM_PARTITION_ACL_IP6S;
        break;
    case EXT_ACL432_TCAM_DATAm:
        if (mem_index < partitions[TCAM_PARTITION_ACL_IP6F].num_entries) {
            part = TCAM_PARTITION_ACL_IP6F;
            break;
        }
        mem_index -= partitions[TCAM_PARTITION_ACL_IP6F].num_entries;
        if (mem_index < partitions[TCAM_PARTITION_ACL_L2IP4].num_entries) {
            part = TCAM_PARTITION_ACL_L2IP4;
            break;
        }
        mem_index -= partitions[TCAM_PARTITION_ACL_L2IP4].num_entries;
        part = TCAM_PARTITION_ACL_L2IP6;
        break;
    case EXT_FP_POLICYm:
    case EXT_FP_CNTRm:
        for (part = TCAM_PARTITION_ACL_L2; part < TCAM_PARTITION_ACL_L2IP6;
             part++) {
            if (mem_index < partitions[part].num_entries) {
                break;
            }
            mem_index -= partitions[part].num_entries;
        }
        break;
    case EXT_FP_CNTR8m:
        for (part = TCAM_PARTITION_ACL_L2; part < TCAM_PARTITION_ACL_L2IP6;
             part++) {
            if (mem_index < (partitions[part].num_entries + 7) / 8) {
                break;
            }
            mem_index -= (partitions[part].num_entries + 7) / 8;
        }
        break;
    case EXT_ACL288_TCAM_L2m:
    case EXT_FP_POLICY_ACL288_L2m:
    case EXT_FP_CNTR_ACL288_L2m:
    case EXT_FP_CNTR8_ACL288_L2m:
        part = TCAM_PARTITION_ACL_L2;
        break;
    case EXT_ACL288_TCAM_IPV4m:
    case EXT_FP_POLICY_ACL288_IPV4m:
    case EXT_FP_CNTR_ACL288_IPV4m:
    case EXT_FP_CNTR8_ACL288_IPV4m:
        part = TCAM_PARTITION_ACL_IP4;
        break;
    case EXT_ACL360_TCAM_DATA_IPV6_SHORTm:
    case EXT_FP_POLICY_ACL360_IPV6_SHORTm:
    case EXT_FP_CNTR_ACL360_IPV6_SHORTm:
    case EXT_FP_CNTR8_ACL360_IPV6_SHORTm:
        part = TCAM_PARTITION_ACL_IP6S;
        break;
    case EXT_ACL432_TCAM_DATA_IPV6_LONGm:
    case EXT_FP_POLICY_ACL432_IPV6_LONGm:
    case EXT_FP_CNTR_ACL432_IPV6_LONGm:
    case EXT_FP_CNTR8_ACL432_IPV6_LONGm:
        part = TCAM_PARTITION_ACL_IP6F;
        break;
    case EXT_ACL144_TCAM_L2m:
    case EXT_FP_POLICY_ACL144_L2m:
    case EXT_FP_CNTR_ACL144_L2m:
    case EXT_FP_CNTR8_ACL144_L2m:
        part = TCAM_PARTITION_ACL_L2C;
        break;
    case EXT_ACL144_TCAM_IPV4m:
    case EXT_FP_POLICY_ACL144_IPV4m:
    case EXT_FP_CNTR_ACL144_IPV4m:
    case EXT_FP_CNTR8_ACL144_IPV4m:
        part = TCAM_PARTITION_ACL_IP4C;
        break;
    case EXT_ACL144_TCAM_IPV6m:
    case EXT_FP_POLICY_ACL144_IPV6m:
    case EXT_FP_CNTR_ACL144_IPV6m:
    case EXT_FP_CNTR8_ACL144_IPV6m:
        part = TCAM_PARTITION_ACL_IP6C;
        break;
    case EXT_ACL432_TCAM_DATA_L2_IPV4m:
    case EXT_FP_POLICY_ACL432_L2_IPV4m:
    case EXT_FP_CNTR_ACL432_L2_IPV4m:
    case EXT_FP_CNTR8_ACL432_L2_IPV4m:
        part = TCAM_PARTITION_ACL_L2IP4;
        break;
    case EXT_ACL432_TCAM_DATA_L2_IPV6m:
    case EXT_FP_POLICY_ACL432_L2_IPV6m:
    case EXT_FP_CNTR_ACL432_L2_IPV6m:
    case EXT_FP_CNTR8_ACL432_L2_IPV6m:
        part = TCAM_PARTITION_ACL_L2IP6;
        break;
    default:
        return SOC_E_INTERNAL;
    }

    *raw_index = partitions[part].tcam_base +
        (mem_index * entries_per_index << partitions[part].tcam_width_shift);

    return SOC_E_NONE;
}
#endif

int
soc_tcam_init(int unit)
{
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        return soc_easyrider_tcam_init(unit);
    }
#endif
#ifdef BCM_TRIUMPH_SUPPORT
    if (soc_feature(unit, soc_feature_esm_support)) {
        return soc_triumph_tcam_init(unit);
    }
#endif
    return SOC_E_UNAVAIL;
}

int
soc_tcam_read_entry(int unit, int part, int index, uint32 *mask, uint32 *data,
                    int *valid)
{
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        return soc_easyrider_tcam_read_entry(unit, part, index, mask, data,
                                             valid);
    }
#endif
#ifdef BCM_TRIUMPH_SUPPORT
    if (soc_feature(unit, soc_feature_esm_support)) {
        return soc_triumph_tcam_read_entry(unit, part, index, mask, data,
                                           valid);
    }
#endif
    return SOC_E_UNAVAIL;
}

int
soc_tcam_write_entry(int unit, int part, int index, uint32 *mask, uint32 *data)
{
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
      return soc_easyrider_tcam_write_entry(unit, part, index, mask, data);
    }
#endif
#ifdef BCM_TRIUMPH_SUPPORT
    if (soc_feature(unit, soc_feature_esm_support)) {
        return soc_triumph_tcam_write_entry(unit, part, index, mask, data);
    }
#endif
    return SOC_E_UNAVAIL;
}

int
soc_tcam_set_valid(int unit, int part, int index, int valid)
{
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        return soc_easyrider_tcam_set_valid(unit, part, index, valid);
    }
#endif
#ifdef BCM_TRIUMPH_SUPPORT
    if (soc_feature(unit, soc_feature_esm_support)) {
        return SOC_E_NONE;
    }
#endif
    return SOC_E_UNAVAIL;
}

int
soc_tcam_search_entry(int unit, int part1, int part2, uint32 *data,
                      int *index1, int *index2)
{
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        /* use part2 as boolean to specify upper half in tcam sharing */
        return soc_easyrider_tcam_search_entry(unit, part1, part2, data,
                                               index1);
    }
#endif
#ifdef BCM_TRIUMPH_SUPPORT
    if (soc_feature(unit, soc_feature_esm_support)) {
        return soc_triumph_tcam_search_entry(unit, part1, part2, data, index1,
                                             index2);
    }
#endif
    return SOC_E_UNAVAIL;
}

int
soc_tcam_write_ib(int unit, uint32 addr, uint32 data)
{
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        return soc_easyrider_tcam_write_ib(unit, addr, data);
    }
#endif
    return SOC_E_UNAVAIL;
}

int
soc_tcam_write_ima(int unit, uint32 addr, uint32 d0_msb, uint32 d1_lsb)
{
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        return soc_easyrider_tcam_write_ima(unit, addr, d0_msb, d1_lsb);
    }
#endif
    return SOC_E_UNAVAIL;
}

int
soc_tcam_read_ima(int unit, uint32 addr, uint32 *d0_msb, uint32 *d1_lsb)
{
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        return soc_easyrider_tcam_read_ima(unit, addr, d0_msb, d1_lsb);
    }
#endif
    return SOC_E_UNAVAIL;
}

int
soc_tcam_write_dbreg(int unit, uint32 addr, uint32 d0_msb, uint32 d1,
                     uint32 d2_lsb)
{
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        /* use only least significant 64-bit from the 72-bit argument */
        return soc_easyrider_tcam_write_dbreg(unit, addr, d1, d2_lsb);
    }
#endif
#ifdef BCM_TRIUMPH_SUPPORT
    if (soc_feature(unit, soc_feature_esm_support)) {
        return soc_triumph_tcam_write_reg(unit, addr, d0_msb, d1, d2_lsb);
    }
#endif
    return SOC_E_UNAVAIL;
}

int
soc_tcam_read_dbreg(int unit, uint32 addr, uint32 *d0_msb, uint32 *d1,
                    uint32 *d2_lsb)
{
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        /* use only least significant 64-bit from the 72-bit argument */
        return soc_easyrider_tcam_read_dbreg(unit, addr, d1, d2_lsb);
    }
#endif
#ifdef BCM_TRIUMPH_SUPPORT
    if (soc_feature(unit, soc_feature_esm_support)) {
        return soc_triumph_tcam_read_reg(unit, addr, d0_msb, d1, d2_lsb);
    }
#endif
    return SOC_E_UNAVAIL;
}

int
soc_tcam_get_info(int unit, int *type, int *subtype, int *dc_val,
                  soc_tcam_info_t **tcam_info)
{
#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        return soc_easyrider_tcam_get_info(unit, type, subtype, dc_val,
                                           tcam_info);
    }
#endif
#ifdef BCM_TRIUMPH_SUPPORT
    if (soc_feature(unit, soc_feature_esm_support)) {
        return soc_triumph_tcam_get_info(unit, type, subtype, dc_val,
                                         tcam_info);
    }
#endif
    return SOC_E_UNAVAIL;
}

int
soc_tcam_get_part_size(int unit, soc_tcam_partition_type_t part,
                       int *num_entries, int *num_entries_include_pad)
{
#if defined(BCM_EASYRIDER_SUPPORT) || defined(BCM_TRIUMPH_SUPPORT)
    if (SOC_IS_EASYRIDER(unit) || soc_feature(unit, soc_feature_esm_support)) {
        soc_tcam_partition_t *partitions;

        partitions =
            ((soc_tcam_info_t *)SOC_CONTROL(unit)->tcam_info)->partitions;

        if (part < TCAM_PARTITION_RAW || part >= TCAM_PARTITION_COUNT) {
            return SOC_E_PARAM;
        }
        if (num_entries != NULL) {
            *num_entries = partitions[part].num_entries;
        }
        if (num_entries_include_pad != NULL) {
            *num_entries_include_pad =
                partitions[part].num_entries_include_pad;
        }
        return SOC_E_NONE;
    }
#endif
    return SOC_E_UNAVAIL;
}

int
soc_tcam_part_index_to_mem_index(int unit, soc_tcam_partition_type_t part,
                                 int part_index, soc_mem_t mem, int *mem_index)
{
#ifdef BCM_TRIUMPH_SUPPORT
    if (soc_feature(unit, soc_feature_esm_support)) {
        return soc_triumph_tcam_part_index_to_mem_index(unit, part, part_index,
                                                        mem, mem_index);
    }
#endif
    return SOC_E_UNAVAIL;
}

int
soc_tcam_mem_index_to_raw_index(int unit, soc_mem_t mem, int mem_index,
                                soc_mem_t *real_mem, int *raw_index)
{
#ifdef BCM_TRIUMPH_SUPPORT
    if (soc_feature(unit, soc_feature_esm_support)) {
        return soc_triumph_tcam_mem_index_to_raw_index(unit, mem, mem_index,
                                                       real_mem, raw_index);
    }
#endif
    return SOC_E_UNAVAIL;
}
