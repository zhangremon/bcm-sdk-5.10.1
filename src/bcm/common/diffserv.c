/*
 * $Id: diffserv.c 1.3 Broadcom SDK $
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
 * Common Diffserv Structure Initializers.
 *
 */

#include <sal/core/libc.h>

#include <bcm/diffserv.h>

/*
 * Function:
 *      bcm_ds_clfr_t_init
 * Purpose:
 *      Initialize the Diffserv classifier structure.
 * Parameters:
 *      ds_clfr - Pointer to Diffserv classifier structure.
 * Returns:
 *      NONE
 */
void
bcm_ds_clfr_t_init(bcm_ds_clfr_t *ds_clfr)
{
    if (ds_clfr != NULL) {
        sal_memset(ds_clfr, 0, sizeof (*ds_clfr));
    }
    return;
}

/*
 * Function:
 *      bcm_ds_inprofile_actn_t_init
 * Purpose:
 *      Initialize the Diffserv Inprofile Action structure.
 * Parameters:
 *      ds_inprof_actn - Pointer to Diffserv Inprofile Action structure.
 * Returns:
 *      NONE
 */
void
bcm_ds_inprofile_actn_t_init(bcm_ds_inprofile_actn_t *ds_inprof_actn)
{
    if (ds_inprof_actn != NULL) {
        sal_memset(ds_inprof_actn, 0, sizeof (*ds_inprof_actn));
    }
    return;
}

/*
 * Function:
 *      bcm_ds_outprofile_actn_t_init
 * Purpose:
 *      Initialize the Diffserv Out of profile action structure.
 * Parameters:
 *      ds_outprof_actn - Pointer to Diffserv Out of profile Action structure.
 * Returns:
 *      NONE
 */
void
bcm_ds_outprofile_actn_t_init(bcm_ds_outprofile_actn_t *ds_outprof_actn)
{
    if (ds_outprof_actn != NULL) {
        sal_memset(ds_outprof_actn, 0, sizeof (*ds_outprof_actn));
    }
    return;
}

/*
 * Function:
 *      bcm_ds_nomatch_actn_t_init
 * Purpose:
 *      Initialize the Diffserv No match action structure.
 * Parameters:
 *      ds_nomatch_actn - Pointer to Diffserv No match Action structure.
 * Returns:
 *      NONE
 */
void
bcm_ds_nomatch_actn_t_init(bcm_ds_nomatch_actn_t *ds_nomatch_actn)
{
    if (ds_nomatch_actn != NULL) {
        sal_memset(ds_nomatch_actn, 0, sizeof (*ds_nomatch_actn));
    }
    return;
}

/*
 * Function:
 *      bcm_ds_scheduler_t_init
 * Purpose:
 *      Initialize the Diffserv Scheduler structure.
 * Parameters:
 *      ds_sched - Pointer to Diffserv Scheduler structure.
 * Returns:
 *      NONE
 */
void
bcm_ds_scheduler_t_init(bcm_ds_scheduler_t *ds_sched)
{
    if (ds_sched != NULL) {
        sal_memset(ds_sched, 0, sizeof (*ds_sched));
    }
    return;
}

/*
 * Function:
 *      bcm_ds_counters_t_init
 * Purpose:
 *      Initialize the Diffserv counters structure.
 * Parameters:
 *      ds_count - Pointer to Diffserv counters structure.
 * Returns:
 *      NONE
 */
void
bcm_ds_counters_t_init(bcm_ds_counters_t *ds_count)
{
    if (ds_count != NULL) {
        sal_memset(ds_count, 0, sizeof (*ds_count));
    }
    return;
}
