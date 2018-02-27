/*
 * $Id: cfp.c 1.9 Broadcom SDK $
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
 * CFP driver service
 */
#include <soc/types.h>
#include <soc/error.h>
#include <soc/cfp.h>

int 
drv_cfp_init(int unit)
{
    return SOC_E_UNAVAIL;
}

int 
drv_cfp_action_get(int unit, uint32* action, drv_cfp_entry_t* entry, 
            uint32* act_param)
{
    return SOC_E_UNAVAIL;
}

int 
drv_cfp_action_set(int unit, uint32 action, drv_cfp_entry_t* rntry, 
            uint32 act_param1, uint32 act_param2)
{
    return SOC_E_UNAVAIL;
}

int 
drv_cfp_control_get(int unit, uint32 control_type, uint32 param1, 
            uint32 *param2)
{
    return SOC_E_UNAVAIL;
}

int 
drv_cfp_control_set(int unit, uint32 control_type, uint32 param1, 
            uint32 param2)
{
    return SOC_E_UNAVAIL;
}

int 
drv_cfp_entry_read(int unit, uint32 index, uint32 ram_type, 
            drv_cfp_entry_t *entry)
{
    return SOC_E_UNAVAIL;
}

int 
drv_cfp_entry_search(int unit, uint32 flags, uint32 *index, 
            drv_cfp_entry_t *entry)
{
    return SOC_E_UNAVAIL;
}

int 
drv_cfp_entry_write(int unit, uint32 index, uint32 ram_type, 
            drv_cfp_entry_t *entry)
{
    return SOC_E_UNAVAIL;
}

int 
drv_cfp_field_get(int unit, uint32 mem_type, uint32 field_type, 
            drv_cfp_entry_t* entry, uint32* fld_val)
{
    return SOC_E_UNAVAIL;
}

int 
drv_cfp_field_set(int unit, uint32 mem_type, uint32 field_type, 
            drv_cfp_entry_t* entry, uint32* fld_val)
{
    return SOC_E_UNAVAIL;
}

int 
drv_cfp_meter_get(int unit, drv_cfp_entry_t* entry, uint32 *kbits_sec, 
            uint32 *kbits_burst)
{
    return SOC_E_UNAVAIL;
}

int 
drv_cfp_meter_set(int unit, drv_cfp_entry_t* entry, uint32 kbits_sec, 
            uint32 kbits_burst)
{
    return SOC_E_UNAVAIL;
}

int 
drv_cfp_qset_get(int unit, uint32 qual, drv_cfp_entry_t *entry, uint32 *val)
{
    return SOC_E_UNAVAIL;
}

int 
drv_cfp_qset_set(int unit, uint32 qual, drv_cfp_entry_t *entry, uint32 val)
{
    return SOC_E_UNAVAIL;
}

int 
drv_cfp_slice_id_select(int unit, drv_cfp_entry_t *entry, uint32 *slice_id, uint32 flags)
{
    return SOC_E_UNAVAIL;
}

int
drv_cfp_slice_to_qset(int unit, uint32 slice_id, drv_cfp_entry_t *entry)
{
    return SOC_E_UNAVAIL;
}

int 
drv_cfp_stat_get(int unit, uint32 stat_type, uint32 index, uint32* counter)
{
    return SOC_E_UNAVAIL;
}

int 
drv_cfp_stat_set(int unit, uint32 stat_type, uint32 index, uint32 counter)
{
    return SOC_E_UNAVAIL;
}

int 
drv_cfp_udf_get(int unit, uint32 port, uint32 udf_index, uint32 *offset, uint32 *base)
{
    return SOC_E_UNAVAIL;
}

int 
drv_cfp_udf_set(int unit, uint32 port, uint32 udf_index, uint32 offset, uint32 base)
{
    return SOC_E_UNAVAIL;
}

int 
drv_cfp_ranger(int unit, uint32 flags, uint32 min, uint32 max)
{
    return SOC_E_UNAVAIL;
}

int 
drv_cfp_range_set(int unit, uint32 type, uint32 id, uint32 param1, uint32 param2)
{
    return SOC_E_UNAVAIL;
}

int 
drv_cfp_range_get(int unit, uint32 type, uint32 id, uint32 *param1, uint32 *param2)
{
    return SOC_E_UNAVAIL;
}

int
drv_cfp_sub_qual_by_udf(int unit, int enable, int slice_id, uint32 sub_qual, 
    drv_cfp_qual_udf_info_t * qual_udf_info)
{
    return SOC_E_UNAVAIL;
}
