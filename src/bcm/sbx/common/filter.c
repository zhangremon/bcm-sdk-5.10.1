/*
 * $Id: filter.c 1.3 Broadcom SDK $
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
 * Module: Fast Filter Processor
 *
 * Purpose:
 *     API for filter rules and masks.
 */

#include <bcm/types.h>
#include <bcm/error.h>
#include <bcm/filter.h>

int
bcm_sbx_filter_create(int unit,
                      bcm_filterid_t *f_return)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_filter_create_id(int unit,
                         bcm_filterid_t f)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_filter_destroy(int unit,
                       bcm_filterid_t f)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_filter_copy(int unit,
                    bcm_filterid_t f_src,
                    bcm_filterid_t *f_return)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_filter_copy_id(int unit,
                       bcm_filterid_t f_src,
                       bcm_filterid_t f_dest)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_filter_qualify_priority(int unit,
                                bcm_filterid_t f,
                                int prio)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_filter_qualify_ingress(int unit,
                               bcm_filterid_t f,
                               bcm_pbmp_t pbmp)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_filter_qualify_egress(int unit,
                              bcm_filterid_t f,
                              bcm_pbmp_t pbmp)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_filter_qualify_egress_modid(int unit,
                                    bcm_filterid_t f,
                                    int module_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_filter_qualify_unknown_ucast(int unit,
                                     bcm_filterid_t f)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_filter_qualify_unknown_mcast(int unit,
                                     bcm_filterid_t f)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_filter_qualify_known_ucast(int unit,
                                   bcm_filterid_t f)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_filter_qualify_known_mcast(int unit,
                                   bcm_filterid_t f)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_filter_qualify_broadcast(int unit,
                                 bcm_filterid_t f)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_filter_qualify_stop(int unit,
                            bcm_filterid_t f,
                            int partial_match)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_filter_qualify_format(int unit,
                              bcm_filterid_t f,
                              bcm_filter_format_t format)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_filter_qualify_data(int unit,
                            bcm_filterid_t f,
                            int offset,
                            int len,
                            const uint8 *data,
                            const uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_filter_qualify_data8(int unit,
                             bcm_filterid_t f,
                             int offset,
                             uint8 val,
                             uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_filter_qualify_data16(int unit,
                              bcm_filterid_t f,
                              int offset,
                              uint16 val,
                              uint16 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_filter_qualify_data32(int unit,
                              bcm_filterid_t f,
                              int offset,
                              uint32 val,
                              uint32 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_filter_action_match(int unit,
                            bcm_filterid_t f,
                            bcm_filter_action_t action,
                            uint32 param)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_filter_action_no_match(int unit,
                               bcm_filterid_t f,
                               bcm_filter_action_t action,
                               uint32 param)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_filter_action_out_profile(int unit,
                                  bcm_filterid_t f,
                                  bcm_filter_action_t action,
                                  uint32 param,
                                  int meter_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_filter_install(int unit,
                       bcm_filterid_t f)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_filter_reinstall(int unit,
                         bcm_filterid_t f)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_filter_remove(int unit,
                      bcm_filterid_t f)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_filter_remove_all(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_filter_init(int unit)
{
    return BCM_E_UNAVAIL;
}

#ifdef BROADCOM_DEBUG
int
bcm_sbx_filter_show(int unit,
                    const char *pfx)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_filter_dump(int unit,
                    bcm_filterid_t bf)
{
    return BCM_E_UNAVAIL;
}
#endif /* BROADCOM_DEBUG */
