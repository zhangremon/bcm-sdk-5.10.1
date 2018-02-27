/* 
 * $Id: counter.h 1.3 Broadcom SDK $
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
 * File:        counter.h
 * Purpose:     SBX Software Counter Collection module definitions.
 */

#ifndef   _SOC_SBX_COUNTER_H_
#define   _SOC_SBX_COUNTER_H_

#include <sal/types.h>
#include <soc/types.h>

/*
 * Counter Block Definition
 *
 * Contains definition for counter blocks available in a device.
 *
 * Definitions / Assumptions:
 *   - a 'block' contains 1 or more equal 'sets' of counters,
 *   - a 'set' contains 1 or more 'counters'
 */
typedef int (*fn_read)(int unit, int set, int counter, uint64 *val, int *width);
typedef int (*fn_write)(int unit, int set, int counter, uint64 val);

typedef struct soc_sbx_counter_block_info_s {
    int           block;         /* ID for a counter block */
    int           num_sets;      /* Number of counter sets in block */
    int           num_counters;  /* Number of counters in each set */
    fn_read       read;          /* Function to read a given counter */
    fn_write      write;         /* Function to write a given counter */
} soc_sbx_counter_block_info_t;


/*
 * External Functions
 */
extern int soc_sbx_counter_init(int unit,
                                soc_sbx_counter_block_info_t *block_info,
                                int block_count);
extern int soc_sbx_counter_detach(int unit);

extern int soc_sbx_counter_bset_add(int unit, int block, int set);

extern int soc_sbx_counter_start(int unit, uint32 flags, int interval, pbmp_t pbmp);
extern int soc_sbx_counter_status(int unit, uint32 *flags, int *interval, pbmp_t *pbmp);
extern int soc_sbx_counter_stop(int unit);
extern int soc_sbx_counter_sync(int unit);

extern int soc_sbx_counter_get(int unit, int block, int set,
                               int counter, uint64 *val);

extern int soc_sbx_counter_get_zero(int unit, int block, int set,
                                    int counter, uint64 *val);

extern int soc_sbx_counter_set(int unit, int block, int set,
                               int counter, uint64 val);

extern int soc_sbx_counter_dump(int unit);

extern int soc_sbx_counter_bset_clear(int unit, int block);

#endif /* _SOC_SBX_COUNTER_H_ */
