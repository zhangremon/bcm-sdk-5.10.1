/*
 * $Id: sbx_common.c 1.15 Broadcom SDK $
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
 *
 */

#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_ka_auto.h>
#include <soc/sbx/sirius.h>
#include <soc/debug.h>
#include "bm9600_properties.h"
#include <bcm/cosq.h>


static int soc_sbx_conn_cmp(void *src, void *dest);
static void soc_sbx_conn_cpy(void *a, void *b);
static void soc_sbx_conn_init(void *a);
static int soc_sbx_qsize_cmp(void *src, void *dest);
static void soc_sbx_qsize_update(void *a, void *b);
static void soc_sbx_qsize_cpy(void *a, void *b);
static void soc_sbx_qsize_init(void *a);

static sbx_conn_template_info_t *conn_util[SOC_MAX_NUM_DEVICES];
static sbx_conn_template_info_t *conn_age[SOC_MAX_NUM_DEVICES];
static sbx_conn_template_info_t *qbuf_info[SOC_MAX_NUM_DEVICES];

static sbx_template_clbk_t template_clbk[2] =
{
    { soc_sbx_conn_cmp, soc_sbx_conn_cpy, soc_sbx_conn_cpy, soc_sbx_conn_init },
    { soc_sbx_qsize_cmp, soc_sbx_qsize_update, soc_sbx_qsize_cpy, soc_sbx_qsize_init }
};

#ifdef BCM_WARM_BOOT_SUPPORT
/* 
 * Compression Format:
 *      0-3:    conn_util_max_template
 *      4-7:    conn_age_max_template
 *      7-A:    A = 7 + (conn_util_max_template * sizeof(sbx_conn_template_t))
 *      A-B:    Y = X + (conn_age_max_template * sizeof(sbx_conn_template_t))
 *      B-C:    C = B + 4 : qbuf_info_max_template
 *      C-D:    D = C + (qbuf_info_max_template * sizeof(sbx_conn_template_t))
 */
int
soc_sbx_connect_wb_scache_info_get(int unit, uint32 *sl_ptr, uint32 *cu_ptr, 
                                   uint32 *ca_ptr, uint32 *qb_ptr)
{
    uint32  scache_len, cu_len, ca_len, qbuf_len;

    if ((!conn_util[unit]) || (!conn_age[unit])) {
        return SOC_E_INIT;
    }
    if ((!sl_ptr) || (!cu_ptr) || (!ca_ptr) || (!qb_ptr)) {
        return SOC_E_PARAM;
    }

    scache_len = 8; /* to store conn_[util | age]-max_template's */
    cu_len = sizeof(sbx_conn_template_t) * conn_util[unit]->max_template;
    scache_len += cu_len;
    ca_len = sizeof(sbx_conn_template_t) * conn_age[unit]->max_template;
    scache_len += ca_len;

    qbuf_len = 0;
    if (SOC_IS_SIRIUS(unit) && (qbuf_info[unit] != NULL)) {
        /* qbuf is only used on Sirius devices */
        qbuf_len = (sizeof(sbx_conn_template_t) * 
                    qbuf_info[unit]->max_template);
        scache_len += 4; /* to store qbuf_info_max_template */
    }
    scache_len += qbuf_len;

    *sl_ptr = scache_len;
    *cu_ptr = cu_len;
    *ca_ptr = ca_len;
    *qb_ptr = qbuf_len;

    return SOC_E_NONE;
}
int
soc_sbx_connect_wb_init(int unit)
{
    int                 rv = SOC_E_NONE;
    soc_scache_handle_t handle;
    uint8               *scache_ptr;
    uint32              scache_len, alloc_get, cu_len, ca_len, qbuf_len;

    rv = soc_sbx_connect_wb_scache_info_get(unit, &scache_len, &cu_len, 
                                            &ca_len, &qbuf_len);
    if (rv != SOC_E_NONE) {
        SOC_ERROR_PRINT((DK_ERR, "Internal error in SCACHE info get. \n"));
        return rv;
    }
    SOC_SCACHE_HANDLE_SET(handle, unit, SOC_SBX_WB_MODULE_COMMON, 0);

    if (SOC_WARM_BOOT(unit)) {
        /* If device is during warm-boot, recover the state from scache */
        SOC_IF_ERROR_RETURN(soc_scache_ptr_get(unit, handle, 
                                               &scache_ptr, &alloc_get));
        if (alloc_get != scache_len) {
            SOC_ERROR_PRINT((DK_ERR, "Internal error retrieving state during "
                             "WarmBoot (Size mismatch) \n"));
            return SOC_E_INTERNAL;
        }
        conn_util[unit]->max_template = *((uint32 *)scache_ptr);
        conn_age[unit]->max_template = *((uint32 *)(scache_ptr + 4));
        sal_memcpy(conn_util[unit]->template, (scache_ptr + 8), cu_len);
        sal_memcpy(conn_age[unit]->template, (scache_ptr + (8 + cu_len)), 
                   ca_len);
        if (SOC_IS_SIRIUS(unit)) {
            qbuf_info[unit]->max_template = *((uint32 *)(scache_ptr + 
                                                       (8 + cu_len + ca_len)));
            sal_memcpy(qbuf_info[unit]->template, 
                       (scache_ptr + (8 + cu_len + ca_len + 4)), qbuf_len);
        }
    } else {
        /* During cold-boot. Allocate a stable cache */
        rv = soc_scache_ptr_get(unit, handle, &scache_ptr, &alloc_get);
        if (rv != SOC_E_NOT_FOUND) {
            SOC_ERROR_PRINT((DK_ERR, "WarmBoot handle for "
                             "SOC_SBX_WB_MODULE_COMMON already used.\n"));
            return SOC_E_INIT;
        }
        rv = SOC_E_NONE;
        SOC_IF_ERROR_RETURN(soc_scache_alloc(unit, handle, scache_len));
        SOC_IF_ERROR_RETURN(soc_scache_ptr_get(unit, handle, 
                                               &scache_ptr, &alloc_get));
        if (alloc_get != scache_len) {
            SOC_ERROR_PRINT((DK_ERR, "Size mismatch in requested vs allocated "
                             "Warmboot state for SOC_SBX_WB_MODULE_COMMON\n"));
            return SOC_E_INIT;
        }
        if (!scache_ptr) {
            SOC_ERROR_PRINT((DK_ERR, "Invalid Warmboot scache handle "
                             "allocated for SOC_SBX_WB_MODULE_COMMON \n"));
            return SOC_E_MEMORY;
        }
    }

    return rv;
}

int
soc_sbx_wb_common_state_sync(int unit, int sync)
{
    int                 rv = SOC_E_NONE;
    soc_scache_handle_t handle;
    uint8               *scache_ptr;
    uint32              scache_len, alloc_get, cu_len, ca_len, qbuf_len;

    if (SOC_WARM_BOOT(unit)) {
        SOC_ERROR_PRINT((DK_ERR, "Cannot write to SCACHE during WarmBoot\n"));
        return SOC_E_INTERNAL;
    }

    rv = soc_sbx_connect_wb_scache_info_get(unit, &scache_len, &cu_len, 
                                            &ca_len, &qbuf_len);
    if (rv != SOC_E_NONE) {
        SOC_ERROR_PRINT((DK_ERR, "Internal error in SCACHE info get. \n"));
        return rv;
    }
    SOC_SCACHE_HANDLE_SET(handle, unit, SOC_SBX_WB_MODULE_COMMON, 0);
    SOC_IF_ERROR_RETURN(soc_scache_ptr_get(unit, handle, 
                                           &scache_ptr, &alloc_get));
    if (alloc_get != scache_len) {
        SOC_ERROR_PRINT((DK_ERR, "Size mismatch in expected vs allocated "
                         "Warmboot state for SOC_SBX_WB_MODULE_COMMON\n"));
        return SOC_E_INTERNAL;
    }

    /* now store the state into the compressed format */
    *((uint32 *) scache_ptr) = conn_util[unit]->max_template;
    *((uint32 *) (scache_ptr + 4)) = conn_age[unit]->max_template;
    sal_memcpy((scache_ptr + 8), conn_util[unit]->template, cu_len);
    sal_memcpy((scache_ptr + 8 + cu_len), conn_age[unit]->template, ca_len);
    if (SOC_IS_SIRIUS(unit)) {
        *((uint32 *) (scache_ptr + 8 + cu_len + ca_len)) = qbuf_len;
        sal_memcpy((scache_ptr + 12 + cu_len + ca_len),
                   qbuf_info[unit]->template, qbuf_len);
    }

    /* trigger the scache sync */
    if (sync) {
        rv = soc_scache_commit(unit);
        if (rv != SOC_E_NONE) {
            SOC_ERROR_PRINT((DK_ERR, "%s: Error(%s) sync'ing scache to "
                     "Persistent memory. \n",FUNCTION_NAME(), soc_errmsg(rv)));
            return rv;
        }
    }

    return rv;
}
#endif /* BCM_WARM_BOOT_SUPPORT */

static int
soc_sbx_conn_cmp(void *a, void *b)
{
    return ( ( ((*(uint32_t *)a) == (*(uint32_t *)b)) ? TRUE : FALSE ) );
}

static void
soc_sbx_conn_cpy(void *src, void *dest)
{
    (*(uint32_t *)dest) = (*(uint32_t *)src);
}

static void
soc_sbx_conn_init(void *a)
{
    (*(uint32_t *)a) = 0;
}

static int
soc_sbx_qsize_cmp(void *a, void *b)
{
    sbx_qbuf_info_t *qbuf1 = (*(sbx_qbuf_info_t **)a);
    sbx_qbuf_info_t *qbuf2 = (sbx_qbuf_info_t *)(b);

   
    return( ( ((qbuf1->min == qbuf2->min) && (qbuf1->max == qbuf2->max)) ? TRUE : FALSE ) );
}

static void
soc_sbx_qsize_update(void *src, void *dest)
{
    sbx_qbuf_info_t *qbuf_dest = (*(sbx_qbuf_info_t **)dest);
    sbx_qbuf_info_t *qbuf_src = (sbx_qbuf_info_t *)(src);


    qbuf_dest->max = qbuf_src->max;
    qbuf_dest->min = qbuf_src->min;
}

static void
soc_sbx_qsize_cpy(void *src, void *dest)
{
    sbx_qbuf_info_t *qbuf_dest = (sbx_qbuf_info_t *)(dest);
    sbx_qbuf_info_t *qbuf_src = (*(sbx_qbuf_info_t **)src);


    qbuf_dest->max = qbuf_src->max;
    qbuf_dest->min = qbuf_src->min;
}

static void
soc_sbx_qsize_init(void *a)
{
    sbx_qbuf_info_t *qbuf = (*(sbx_qbuf_info_t **)a);


    qbuf->min = 0;
    qbuf->max = 0;
}


static int
soc_sbx_connect_template_get(int unit, sbx_conn_template_info_t *info,
                             void *value_p, int *template, sbx_template_clbk_t *clbk)
{
    int rc = SOC_E_NONE;
    int cos;
    sbx_conn_template_t *conn = info->template;


    for (cos = 0; cos < info->max_template; cos++) {
        if ( (conn[cos].in_use == TRUE) && (clbk->compare(&conn[cos].value, value_p)) ) {
            (*template) = conn[cos].template;
            break;
        }
    }

    if (cos >= info->max_template) {
        rc = SOC_E_RESOURCE;
        return(rc);
    }

    return(rc);
}

static int
soc_sbx_connect_get(int unit, sbx_conn_template_info_t *info, int template, int *value_p,
                                                              sbx_template_clbk_t *clbk)
{
    int rc = SOC_E_NONE;
    int cos;
    sbx_conn_template_t *conn = info->template;


    for (cos = 0; cos < info->max_template; cos++) {
        if ( (conn[cos].in_use == TRUE) && (conn[cos].template == template) ) {
            clbk->get(&conn[cos].value, value_p);
            break;
        }
    }

    if (cos >= info->max_template) {
        rc = SOC_E_RESOURCE;
        return(rc);
    }

    return(rc);
}

static int
soc_sbx_connect_ref_cnt(int unit, sbx_conn_template_info_t *info, int template, int *count,
                                                              sbx_template_clbk_t *clbk)
{
    int rc = SOC_E_NONE;
    int cos;
    sbx_conn_template_t *conn = info->template;


    for (cos = 0; cos < info->max_template; cos++) {
        if (conn[cos].template == template) {
	    if (conn[cos].in_use == TRUE) {
		*count = conn[cos].ref_cnt;
	    } else {
		*count = 0;
	    }
            break;
        }
    }

    if (cos >= info->max_template) {
        rc = SOC_E_RESOURCE;
        return(rc);
    }

    return(rc);
}

static int
soc_sbx_connect_alloc(int unit, sbx_conn_template_info_t *info, int flags,
                        void *value_p, int *is_allocated, int *template, sbx_template_clbk_t *clbk)
{
    int rc = SOC_E_NONE;
    int cos, cos_available = -1;
    sbx_conn_template_t *conn = info->template;

#if defined(BCM_EASY_RELOAD_SUPPORT)
    if (SOC_IS_RELOADING(unit)) {
        /*
         *  During easy reload, we pretend to allocate, but use the cache that
         *  was read during initialisation.
         */
        for (cos = 0; cos < info->max_template; cos++) {
            if (clbk->compare(&conn[cos].value, value_p)) {
                break;
            }
        }
        if (cos >= info->max_template) {
            /* config change attempted during reload */
            rc = BCM_E_CONFIG;
        }
        if (!conn[cos].in_use) {
            /* not marked as in-use yet, act like just allocated */
            cos_available = cos;
            cos = info->max_template;
        }
    } else { /* if (SOC_IS_RELOADING(unit)) */
#endif /* defined(BCM_EASY_RELOAD_SUPPORT) */
        for (cos = 0; cos < info->max_template; cos++) {
            if ( (conn[cos].in_use == TRUE) && (clbk->compare(&conn[cos].value, value_p)) ) {
                break;
            }

            if ( (cos_available == -1) && (conn[cos].in_use == FALSE) )  {
                cos_available = cos;
            }
        }
        if ( (cos >= info->max_template) && (cos_available == -1) ) {
            rc = SOC_E_RESOURCE;
            return(rc);
        }
#if defined(BCM_EASY_RELOAD_SUPPORT)
    } /* if (SOC_IS_RELOADING(unit)) */
#endif /* defined(BCM_EASY_RELOAD_SUPPORT) */

    if (cos < info->max_template) {
        if (!(conn[cos].flags & SOC_SBX_CONN_FIXED)) {
            conn[cos].ref_cnt++;
        }
        (*template) = cos;
        (*is_allocated) = FALSE;
    } else {
        conn[cos_available].flags = flags;
        conn[cos_available].in_use = TRUE;
        conn[cos_available].template = cos_available;
        clbk->update(value_p, &conn[cos_available].value);
        conn[cos_available].ref_cnt = 1;

        (*template) = cos_available;
        (*is_allocated) = TRUE;
    }

    return(rc);
}

static int
soc_sbx_connect_dealloc(int unit, sbx_conn_template_info_t *info, int flags,
                        void *value_p, int *is_deallocated, int *template, sbx_template_clbk_t *clbk)
{
    int rc = SOC_E_NONE;
    int cos;
    sbx_conn_template_t *conn = info->template;


    for (cos = 0; cos < info->max_template; cos++) {
        if ( (conn[cos].in_use == TRUE) && (clbk->compare(&conn[cos].value, value_p)) ) {
            break;
        }
    }

    if (cos >= info->max_template) {
        rc = SOC_E_RESOURCE;
        return(rc);
    }

   
    (*is_deallocated) = FALSE;
    (*template) = cos;
    if (!(conn[cos].flags & SOC_SBX_CONN_FIXED)) {
        conn[cos].ref_cnt--;
        if (conn[cos].ref_cnt == 0) {
            conn[cos].in_use = FALSE;
            conn[cos].template = -1;
            clbk->init(&conn[cos].value);
            conn[cos].flags = 0;
 
            (*is_deallocated) = TRUE;
        }
    }

    return(rc);
}


/*
 * Templates created for each user specified configuration. Tere are limited number
 * of templates in the system. No rounding to the nearest template done. Template
 * management not at the hardware granularity.
 * TODO: If required coherent template management (QE2K and Sirius inter-operability)
 */
/*
 *  Note that templates are tracked at hardware granularity, so if the value is
 *  represented at BCM layer as a percentage, and the hardware only has eight
 *  possible settings, there will only be eight distinct values in the
 *  templates, and in the cache.
 */
int
soc_sbx_connect_init(int unit, int conn_util_max, int conn_age_max)
{
    int rc = SOC_E_NONE;
    int cos;


    conn_util[unit] = sal_alloc(sizeof(sbx_conn_template_info_t), "conn_util");
    if (conn_util[unit] == NULL) {
        rc = SOC_E_MEMORY;
        goto err;
    }
    sal_memset(conn_util[unit], 0, sizeof(sbx_conn_template_info_t));

    conn_age[unit] = sal_alloc(sizeof(sbx_conn_template_info_t), "conn_age");
    if (conn_age[unit] == NULL) {
        rc = SOC_E_MEMORY;
        goto err;
    }
    sal_memset(conn_age[unit], 0, sizeof(sbx_conn_template_info_t));


    conn_util[unit]->template =
                     sal_alloc(sizeof(sbx_conn_template_t) * SBX_MAX_FABRIC_COS, "conn_template");
    if (conn_util[unit]->template == NULL) {
        rc = SOC_E_MEMORY;
        goto err;
    }

    conn_age[unit]->template =
                      sal_alloc(sizeof(sbx_conn_template_t) * SBX_MAX_FABRIC_COS, "conn_age");
    if (conn_age[unit]->template == NULL) {
        rc = SOC_E_MEMORY;
        goto err;
    }

    sal_memset(conn_util[unit]->template, 0, (sizeof(sbx_conn_template_t) * SBX_MAX_FABRIC_COS));
    sal_memset(conn_age[unit]->template, 0, (sizeof(sbx_conn_template_t) * SBX_MAX_FABRIC_COS));
    conn_util[unit]->max_template = conn_util_max;
    conn_age[unit]->max_template = conn_age_max;
    for (cos = 0; cos < conn_util[unit]->max_template; cos++) {
        conn_util[unit]->template[cos].in_use = FALSE;
        conn_util[unit]->template[cos].template = -1;
    }
    for (cos = 0; cos < conn_age[unit]->max_template; cos++) {
        conn_age[unit]->template[cos].in_use = FALSE;
        conn_age[unit]->template[cos].template = -1;
    }

#ifdef BCM_WARM_BOOT_SUPPORT
    /*
     * Currently, Warmboot not fully supported on sirius
     */
    if (SOC_IS_SIRIUS(unit) == FALSE) {
	rc = soc_sbx_connect_wb_init(unit);
    }
#endif
    return(rc);

err:
    soc_sbx_connect_deinit(unit);
    return(rc);
}

/*
 *  Reload preparation -- puts the proper values into the cache, so that during
 *  reload we can guess the proper template to 'allocate'.
 */
int
soc_sbx_connect_reload(int unit, uint32_t *util, uint32_t *age)
{
#if defined(BCM_EASY_RELOAD_SUPPORT)
    unsigned int index;

    for (index = 0; index < conn_util[unit]->max_template; index++) {
        template_clbk[0].update(&(util[index]),&(conn_util[unit]->template[index].value));
        
        soc_cm_debug(DK_WARN,
                     "set conn_util[%d]->template[%d] to %08X (%d)\n",
                     unit,
                     index,
                     util[index],
                     util[index]);
    }
    for (index = 0; index < conn_age[unit]->max_template; index++) {
        template_clbk[0].update(&(age[index]),&(conn_age[unit]->template[index].value));
        
        soc_cm_debug(DK_WARN,
                     "set conn_age[%d]->template[%d] to %08X (%d)\n",
                     unit,
                     index,
                     age[index],
                     age[index]);
    }
#endif /* defined(BCM_EASY_RELOAD_SUPPORT) */
    return SOC_E_NONE;
}

int
soc_sbx_connect_deinit(int unit)
{
    int rc = SOC_E_NONE;


    if (conn_util[unit] != NULL) {
        if (conn_util[unit]->template != NULL) {
            sal_free(conn_util[unit]->template);
        }
        sal_free(conn_util[unit]);
    }
    if (conn_age[unit] != NULL) {
        if (conn_age[unit]->template != NULL) {
            sal_free(conn_age[unit]->template);
        }
        sal_free(conn_age[unit]);
    }

    return(rc);
}

int
soc_sbx_connect_min_util_template_get(int unit, int utilization, int *template)
{
    int rc = SOC_E_NONE;


    rc = soc_sbx_connect_template_get(unit, conn_util[unit], (void *)&utilization,
                                                            template, &template_clbk[0]);
    return(rc);
}

int
soc_sbx_connect_min_util_get(int unit, int template, int *utilization)
{
    int rc = SOC_E_NONE;


    rc = soc_sbx_connect_get(unit, conn_util[unit], template, (void *)utilization, &template_clbk[0]);
    return(rc);
}

int
soc_sbx_connect_min_util_alloc(int unit, int flags, int utilization, int *is_allocated,
                                                                         int *template)
{
    int rc = SOC_E_NONE;


    rc = soc_sbx_connect_alloc(unit, conn_util[unit], flags, (void *)&utilization, is_allocated,
                                                             template, &template_clbk[0]);

#if defined(BROADCOM_DEBUG)
    soc_cm_debug(SOC_DBG_VERBOSE,
                 "alloc conn_util[%d]->template[%d] to %08X (%d)\n",
                 unit,
                 *template,
                 utilization,
                 utilization);
#endif /* BROADCOM_DEBUG */

    return(rc);
}

int
soc_sbx_connect_min_util_dealloc(int unit, int flags, int utilization, int *is_deallocated,
                                                                                int *template)
{
    int rc = SOC_E_NONE;


    rc = soc_sbx_connect_dealloc(unit, conn_util[unit], flags, (void *)&utilization, is_deallocated,
                                                              template, &template_clbk[0]);
    return(rc);
}

int
soc_sbx_connect_max_age_template_get(int unit, int age, int *template)
{
    int rc = SOC_E_NONE;


    rc = soc_sbx_connect_template_get(unit, conn_age[unit], (void *)&age,
                                                            template, &template_clbk[0]);
    return(rc);
}

int
soc_sbx_connect_max_age_get(int unit, int template, int *age)
{
    int rc = SOC_E_NONE;


    rc = soc_sbx_connect_get(unit, conn_age[unit], template, (void *)age, &template_clbk[0]);
    return(rc);
}

int
soc_sbx_connect_max_age_alloc(int unit, int flags, int age, int *is_allocated, int *template)
{
    int rc = SOC_E_NONE;


    rc = soc_sbx_connect_alloc(unit, conn_age[unit], flags, (void *)&age, is_allocated, template,
                                                                             &template_clbk[0]);

#if defined(BROADCOM_DEBUG)
    soc_cm_debug(SOC_DBG_VERBOSE,
                 "alloc conn_age[%d]->template[%d] to %08X (%d)\n",
                 unit,
                 *template,
                 age,
                 age);
#endif /* BROADCOM_DEBUG */
    return(rc);
}

int
soc_sbx_connect_max_age_dealloc(int unit, int flags, int age, int *is_deallocated, int *template)
{
    int rc = SOC_E_NONE;


    rc = soc_sbx_connect_dealloc(unit, conn_age[unit], flags, (void *)&age, is_deallocated,
                                                                   template, &template_clbk[0]);
    return(rc);
}

int
soc_sbx_qbuf_init(int unit)
{
    int rc = SOC_E_NONE;
    int index;


    qbuf_info[unit] = sal_alloc(sizeof(sbx_conn_template_info_t), "qbuf_info");
    if (qbuf_info[unit] == NULL) {
        rc = SOC_E_MEMORY;
        goto err;
    }
    sal_memset(qbuf_info[unit], 0, sizeof(sbx_conn_template_info_t));

    qbuf_info[unit]->template =
             sal_alloc(sizeof(sbx_conn_template_t) * SIRIUS_MAX_QBUF_TEMPLATES, "qbuf_template");
    if (qbuf_info[unit]->template == NULL) {
        rc = SOC_E_MEMORY;
        goto err;
    }

    sal_memset(qbuf_info[unit]->template, 0,
                                     (sizeof(sbx_conn_template_t) * SIRIUS_MAX_QBUF_TEMPLATES));
    qbuf_info[unit]->max_template = SIRIUS_MAX_QBUF_TEMPLATES;
    for (index = 0; index < qbuf_info[unit]->max_template; index++) {
        qbuf_info[unit]->template[index].in_use = FALSE;
        qbuf_info[unit]->template[index].template = -1;

        qbuf_info[unit]->template[index].value = 
                              (uint32_t)sal_alloc(sizeof(sbx_qbuf_info_t), "qbuf_data");
        if (qbuf_info[unit]->template[index].value == (uint32_t)NULL) {
            rc = SOC_E_MEMORY;
            goto err;
        }
    }

    return(rc);

err:
    soc_sbx_qbuf_deinit(unit);
    return(rc);
}

int
soc_sbx_qbuf_deinit(int unit)
{
    int rc = SOC_E_NONE;
    int index;


    if (qbuf_info[unit] != NULL) {
        if (qbuf_info[unit]->template != NULL) {
            for (index = 0; index < qbuf_info[unit]->max_template; index++) {
                sal_free((void *)(qbuf_info[unit]->template[index].value));
            } 
            sal_free(qbuf_info[unit]->template);
        }
        sal_free(qbuf_info[unit]);
    }

    return(rc);
}

int
soc_sbx_qbuf_template_get(int unit, int min_buf, int max_buf, int *template)
{
    int rc = SOC_E_NONE;
    sbx_qbuf_info_t qbuf;


    qbuf.min = min_buf;
    qbuf.max = max_buf;
    rc = soc_sbx_connect_template_get(unit, qbuf_info[unit], (void *)&qbuf,
                                                            template, &template_clbk[1]);
    return(rc);
}

int
soc_sbx_qbuf_get(int unit, int template, int *min_buf, int *max_buf)
{
    int rc = SOC_E_NONE;
    sbx_qbuf_info_t qbuf;


    rc = soc_sbx_connect_get(unit, qbuf_info[unit], template, (void *)&qbuf, &template_clbk[1]);
    if (rc == SOC_E_NONE) {
        (*min_buf) = qbuf.min;
        (*max_buf) = qbuf.max;
    }

    return(rc);
}

int
soc_sbx_qbuf_ref_cnt(int unit, int template, int *count)
{
    int rc = SOC_E_NONE;

    rc = soc_sbx_connect_ref_cnt(unit, qbuf_info[unit], template, count, &template_clbk[1]);

    if (rc != SOC_E_NONE) {
	/* if the template is not found, return 0 instead of report failure */
	*count = 0;
	rc = SOC_E_NONE;
    }

    return(rc);
}

int
soc_sbx_qbuf_alloc(int unit, int flags, int min_buf, int max_buf, int *is_allocated, int *template)
{
    int rc = SOC_E_NONE;
    sbx_qbuf_info_t qbuf;


    qbuf.min = min_buf;
    qbuf.max = max_buf;
    rc = soc_sbx_connect_alloc(unit, qbuf_info[unit], flags, (void *)&qbuf, is_allocated, template,
                                                                             &template_clbk[1]);
    return(rc);
}

int
soc_sbx_qbuf_dealloc(int unit, int flags, int min_buf, int max_buf, int *is_deallocated, int *template)
{
    int rc = SOC_E_NONE;
    sbx_qbuf_info_t qbuf;


    qbuf.min = min_buf;
    qbuf.max = max_buf;
    rc = soc_sbx_connect_dealloc(unit, qbuf_info[unit], flags, (void *)&qbuf, is_deallocated,
                                                         template, &template_clbk[1]);
    return(rc);
}


int
soc_sbx_sched_get_internal_state(int unit, int sched_mode, int int_pri, int *queue_type, int *priority, int *priority2)
{
    int rc = SOC_E_NONE;


    /* determine the template that is being used */
    /* NOTE: There should be an extra SOC property for the template. */

    (*priority2) = -1;
    if (SOC_SBX_CFG(unit)->sp_mode == SOC_SBX_SP_MODE_ACCOUNT_IN_BAG) {
        switch (sched_mode) {
            case BCM_COSQ_WEIGHTED_FAIR_QUEUING:
                (*queue_type) = 1;
                (*priority) = 8; /* Hungry */
                (*priority2) = 3; /* Satisfied */
                break;

            case BCM_COSQ_EF:
                (*queue_type) = 2;
                (*priority) = 14;
                break;

            case BCM_COSQ_AF:
                (*queue_type) = 1;
                (*priority) = 8; /* Hungry */
                (*priority2) = 3; /* Satisfied */
                break;

            case BCM_COSQ_BE:
                (*queue_type) = 0; 
                (*priority) = 2;
                break;

            case BCM_COSQ_SP:
            case BCM_COSQ_SP_GLOBAL:
                (*queue_type) = (15 - int_pri);
                if ((int_pri >= 0) && (int_pri <= 3)) {
                    (*priority) = 12 - int_pri;
                }
                else { /* ((int_pri >= 4) && (int_pri <= 7)) */
                    (*priority) = 7 - (int_pri - 4);
                }
                break;
            default:
                SOC_ERROR_PRINT((DK_ERR, "scheduler mode(%d) unknown\n", sched_mode));
                return(SOC_E_PARAM);
        }
    }
    else {
        switch (sched_mode) {
            case BCM_COSQ_WEIGHTED_FAIR_QUEUING:
                (*queue_type) = 1;
                (*priority) = 11; /* Hungry */
                (*priority2) = 2; /* Satisfied */
                break;

            case BCM_COSQ_EF:
                (*queue_type) = 2;
                (*priority) = 13;
                (*priority2) = 14; /* Satisfied */
                break;

            case BCM_COSQ_AF:
                (*queue_type) = 1;
                (*priority) = 11; /* Hungry */
                (*priority2) = 2; /* Satisfied */
                break;

            case BCM_COSQ_BE:
                (*queue_type) = 0;
                (*priority) = 2;
                break;

            case BCM_COSQ_SP:
                (*queue_type) = 1;
                (*priority) = 11; /* Hungry */
                (*priority2) = 2; /* Satisfied */
                break;

            case BCM_COSQ_SP_GLOBAL:
                (*queue_type) = (15 - int_pri);
                (*priority) = 10 - int_pri;
                break;

            default:
                SOC_ERROR_PRINT((DK_ERR, "scheduler mode(%d) unknown\n", sched_mode));
                return(SOC_E_PARAM);
        }
    }

    return(rc);
}
/* Similar to previous except supports BCM_COSQ_SP0-7 and int_pri unused for those */
int
soc_sbx_sched_get_internal_state_queue_attach(int unit, int sched_mode, int int_pri,
					      int *queue_type, int *priority, int *hungry_priority)
{
    int rc = SOC_E_NONE;


    /* determine the template that is being used */
    /* NOTE: There should be an extra SOC property for the template. */
    if (SOC_SBX_CFG(unit)->sp_mode == SOC_SBX_SP_MODE_ACCOUNT_IN_BAG) {
        *hungry_priority = 8;
        switch (sched_mode) {
            case BCM_COSQ_WEIGHTED_FAIR_QUEUING:
                (*queue_type) = 1;
                (*priority) = 8; /* Hungry */
                break;

            case BCM_COSQ_EF:
                (*queue_type) = 2;
                (*priority) = 14;
                break;

            case BCM_COSQ_AF:
                (*queue_type) = 1;
                (*priority) = 8; /* Hungry */
                break;

            case BCM_COSQ_BE:
                (*queue_type) = 0; 
                (*priority) = 2;
                break;

            case BCM_COSQ_SP7:
            case BCM_COSQ_GSP7:
                (*queue_type) = 8;
		(*priority) = 12;
                break;

            case BCM_COSQ_SP6:
            case BCM_COSQ_GSP6:
                (*queue_type) = 9;
		(*priority) = 11;
                break;

            case BCM_COSQ_SP5:
            case BCM_COSQ_GSP5:
                (*queue_type) = 10;
		(*priority) = 10;
                break;

            case BCM_COSQ_SP4:
            case BCM_COSQ_GSP4:
                (*queue_type) = 11;
		(*priority) = 9;
                break;

            case BCM_COSQ_SP3:
            case BCM_COSQ_GSP3:
                (*queue_type) = 12;
		(*priority) = 7;
                break;

            case BCM_COSQ_SP2:
            case BCM_COSQ_GSP2:
                (*queue_type) = 13;
		(*priority) = 6;
                break;

	    case BCM_COSQ_SP1:
            case BCM_COSQ_GSP1:
                (*queue_type) = 14;
		(*priority) = 5;
                break;

            case BCM_COSQ_SP0:
            case BCM_COSQ_GSP0:
                (*queue_type) = 15;
		(*priority) = 4;
                break;

            case BCM_COSQ_SP:
            case BCM_COSQ_SP_GLOBAL:
                (*queue_type) = (15 - int_pri);
                if ((int_pri >= 0) && (int_pri <= 3)) {
                    (*priority) = 12 - int_pri;
                }
                else { /* ((int_pri >= 4) && (int_pri <= 7)) */
                    (*priority) = 7 - (int_pri - 4);
                }
                break;

            default:
                SOC_ERROR_PRINT((DK_ERR, "scheduler mode(%d) unknown\n", sched_mode));
                return(SOC_E_PARAM);
        }
    }
    else {
	*hungry_priority = 11;
        switch (sched_mode) {
            case BCM_COSQ_WEIGHTED_FAIR_QUEUING:
                (*queue_type) = 1;
                (*priority) = 11;
                break;

            case BCM_COSQ_EF:
                (*queue_type) = 2;
                (*priority) = 13;
                break;

            case BCM_COSQ_AF:
                (*queue_type) = 1;
                (*priority) = 11;
                break;

            case BCM_COSQ_BE:
                (*queue_type) = 0;
                (*priority) = 2;
                break;

            case BCM_COSQ_SP:
            case BCM_COSQ_SP7:
            case BCM_COSQ_SP6:
            case BCM_COSQ_SP5:
            case BCM_COSQ_SP4:
            case BCM_COSQ_SP3:
            case BCM_COSQ_SP2:
            case BCM_COSQ_SP1:
            case BCM_COSQ_SP0:
                (*queue_type) = 1;
                (*priority) = 11;
                break;

            case BCM_COSQ_GSP7:
                (*queue_type) = 8;
		(*priority) = 10;
                break;

            case BCM_COSQ_GSP6:
                (*queue_type) = 9;
		(*priority) = 9;
                break;

            case BCM_COSQ_GSP5:
                (*queue_type) = 10;
		(*priority) = 8;
                break;

            case BCM_COSQ_GSP4:
                (*queue_type) = 11;
		(*priority) = 7;
                break;

            case BCM_COSQ_GSP3:
                (*queue_type) = 12;
		(*priority) = 6;
                break;

            case BCM_COSQ_GSP2:
                (*queue_type) = 13;
		(*priority) = 5;
                break;

            case BCM_COSQ_GSP1:
                (*queue_type) = 14;
		(*priority) = 4;
                break;

            case BCM_COSQ_GSP0:
                (*queue_type) = 15;
		(*priority) = 3;
                break;

            case BCM_COSQ_SP_GLOBAL:
                (*queue_type) = (15 - int_pri);
                (*priority) = 10 - int_pri;
                break;

            default:
                SOC_ERROR_PRINT((DK_ERR, "scheduler mode(%d) unknown\n", sched_mode));
                return(SOC_E_PARAM);
        }
    }

    return(rc);
}


int
soc_sbx_sched_config_params_verify(int unit, int sched_mode, int int_pri)
{
    int rc = SOC_E_NONE;


    /* determine the template that is being used */
    if (SOC_SBX_CFG(unit)->sp_mode == SOC_SBX_SP_MODE_ACCOUNT_IN_BAG) {
        if ( ((sched_mode == BCM_COSQ_SP_GLOBAL) || (sched_mode == SOC_SBX_SP_MODE_ACCOUNT_IN_BAG))
                                                        && !((int_pri >= 0) && (int_pri <= 7)) ) {
            SOC_ERROR_PRINT((DK_ERR, "int_pri(%d) not supported\n", int_pri));
            return(SOC_E_PARAM);
        }
    }
    else {
        if ( ((sched_mode == BCM_COSQ_SP_GLOBAL) || (sched_mode == SOC_SBX_SP_MODE_ACCOUNT_IN_BAG))
                                                        && !((int_pri >= 0) && (int_pri <= 7)) ) {
            SOC_ERROR_PRINT((DK_ERR, "int_pri(%d) not supported\n", int_pri));
            return(SOC_E_PARAM);
        }
    }

    return(rc);
}

int
soc_sbx_sched_config_set_params_verify(int unit, int sched_mode, int int_pri)
{
    int rc = SOC_E_NONE;


    switch (sched_mode) {
        case BCM_COSQ_WEIGHTED_FAIR_QUEUING:
        case BCM_COSQ_EF:
        case BCM_COSQ_AF:
        case BCM_COSQ_BE:
            SOC_ERROR_PRINT((DK_ERR, "mode(%d) not supported for fifo re-direction\n", sched_mode));
            return(SOC_E_PARAM);

        case BCM_COSQ_SP:
        case BCM_COSQ_SP_GLOBAL:
            break;

        default:
            SOC_ERROR_PRINT((DK_ERR, "mode(%d) unknown for fifo re-direction\n", sched_mode));
            return(SOC_E_PARAM);
    }

    /* determine the template that is being used */
    if (SOC_SBX_CFG(unit)->sp_mode == SOC_SBX_SP_MODE_ACCOUNT_IN_BAG) {
        if ( ((sched_mode == BCM_COSQ_SP_GLOBAL) || (sched_mode == SOC_SBX_SP_MODE_ACCOUNT_IN_BAG))
                                                        && !((int_pri >= 0) && (int_pri <= 7)) ) {
            SOC_ERROR_PRINT((DK_ERR, "int_pri(%d) not supported\n", int_pri));
            return(SOC_E_PARAM);
        }
    }
    else {
        if (sched_mode == BCM_COSQ_SP) {
            SOC_ERROR_PRINT((DK_ERR, "mode(%d) not supported for fifo re-direction\n", sched_mode));
            return(SOC_E_PARAM);
        }

        if ( (sched_mode == BCM_COSQ_SP_GLOBAL) && !((int_pri >= 0) && (int_pri <= 7)) ) {
            SOC_ERROR_PRINT((DK_ERR, "int_pri(%d) not supported\n", int_pri));
            return(SOC_E_PARAM);
        }
    }

    return(rc);
}
