/*
 * $Id: sbx_common.h 1.8 Broadcom SDK $
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
 * configuration common to sbx devices
 */

#ifndef _SBX_COMMON_H_
#define _SBX_COMMON_H_

#include <soc/sbx/sbTypesGlue.h>
#include <soc/sbx/sbFabStatus.h>

/* Some compilers do not support 'packed' attribute */
#if defined(VXWORKS) && (CPU == PPC603) && (VX_VERSION == 54)
#define PACK_STRUCT
#else
#define PACK_STRUCT __attribute__ ((__packed__))
#endif

#define SOC_SBX_CONN_FIXED            (0x01)
#define SOC_SBX_CONN_RESERVED         (0x02)

#define SOC_SBX_TEMPLATE_FIXED        (SOC_SBX_CONN_FIXED)
#define SOC_SBX_TEMPLATE_RESERVED     (SOC_SBX_CONN_RESERVED)

typedef struct sbx_qbuf_info_s {
    int   min;
    int   max;
} sbx_qbuf_info_t;

typedef struct sbx_conn_template_s {
    sbBool_t    in_use;
    int8_t      template;
    int8_t      flags;
    uint32_t    value;
    uint32_t    ref_cnt;
} sbx_conn_template_t, sbx_template_t;

typedef struct sbx_conn_template_info_s {
    int                    max_template;
    sbx_conn_template_t   *template;
} sbx_conn_template_info_t, sbx_template_info_t;

typedef int (*sbx_template_compare_f)(void *a, void *b);
typedef void (*sbx_template_copy_f)(void *src, void *dest);
typedef void (*sbx_template_init_f)(void *a);

typedef struct sbx_template_clbk_s {
    sbx_template_compare_f    compare;
    sbx_template_copy_f       update;
    sbx_template_copy_f       get;
    sbx_template_init_f       init;
} sbx_template_clbk_t;



int
soc_sbx_connect_init(int unit, int conn_util_max, int conn_age_max);

int
soc_sbx_connect_reload(int unit, uint32_t *util, uint32_t *age);

int
soc_sbx_connect_deinit(int unit);

int
soc_sbx_connect_min_util_template_get(int unit, int utilization, int *template);

int
soc_sbx_connect_min_util_get(int unit, int template, int *utilization);

int
soc_sbx_connect_min_util_alloc(int unit, int flags, int utilization, int *is_allocated, int *template);

int
soc_sbx_connect_min_util_dealloc(int unit, int flags, int utilization, int *is_deallocated, int *template);

int
soc_sbx_connect_max_age_template_get(int unit, int age, int *template);

int
soc_sbx_connect_max_age_get(int unit, int template, int *age);

int
soc_sbx_connect_max_age_alloc(int unit, int flags, int age, int *is_allocated, int *template);

int
soc_sbx_connect_max_age_dealloc(int unit, int flags, int age, int *is_deallocated, int *template);

int
soc_sbx_qbuf_init(int unit);

int
soc_sbx_qbuf_deinit(int unit);

int
soc_sbx_qbuf_template_get(int unit, int min_buf, int max_buf, int *template);

int
soc_sbx_qbuf_get(int unit, int template, int *min_buf, int *max_buf);

int
soc_sbx_qbuf_ref_count(int unit, int template, int *count);

int
soc_sbx_qbuf_alloc(int unit, int flags, int min_buf, int max_buf, int *is_allocated, int *template);

int
soc_sbx_qbuf_dealloc(int unit, int flags, int min_buf, int max_buf, int *is_deallocated, int *template);

int
soc_sbx_sched_get_internal_state(int unit, int sched_mode, int int_pri, int *queue_type, int *priority, int *priority2);

int
soc_sbx_sched_get_internal_state_queue_attach(int unit, int sched_mode, int int_pri, int *queue_type, int *priority, int *hungry_priority);

int
soc_sbx_sched_config_params_verify(int unit, int sched_mode, int int_pri);

int
soc_sbx_sched_config_set_params_verify(int unit, int sched_mode, int int_pri);

extern int 
soc_sbx_process_custom_stats(int unit, int links);

#endif /* _SBX_COMMON_H_ */
