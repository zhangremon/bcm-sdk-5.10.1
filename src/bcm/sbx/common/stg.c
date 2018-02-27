/*
 * $Id: stg.c 1.3 Broadcom SDK $
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
 * File:        stg.c
 * Purpose:     Spanning tree group support
 *
 * Multiple spanning trees (MST) is supported in
 * BCM5690, etc.  STGs are created and VLANs may be added to each STG.
 *
 * Per-VLAN spanning tree (PVST) is supported in BCM5632.  This module
 * supports PVST on BCM5632 by having 4k virtual STGs and allowing only
 * a single VLAN per STG.  Before the application can create a second
 * STG, it must remove all but one VLAN from the default STG.
 */

#include <bcm/debug.h>
#include <soc/drv.h>

#include <bcm/error.h>
#include <bcm/stg.h>

int
bcm_sbx_stg_vlan_list(int unit,
                      bcm_stg_t stg,
                      bcm_vlan_t **list,
                      int *count)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stg_vlan_list_destroy(int unit,
                              bcm_vlan_t *list,
                              int count)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stg_default_get(int unit,
                        bcm_stg_t *stg_ptr)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stg_default_set(int unit,
                        bcm_stg_t stg)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stg_vlan_add(int unit,
                     bcm_stg_t stg,
                     bcm_vlan_t vid)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stg_vlan_remove(int unit,
                        bcm_stg_t stg,
                        bcm_vlan_t vid)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stg_vlan_remove_all(int unit,
                            bcm_stg_t stg)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stg_init(int unit)
{
    return BCM_E_NONE;
}

int
bcm_sbx_stg_clear(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stg_create_id(int unit,
                      bcm_stg_t stg)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stg_create(int unit,
                   bcm_stg_t *stg_ptr)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stg_destroy(int unit,
                    bcm_stg_t stg)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stg_list(int unit,
                 bcm_stg_t **list,
                 int *count)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stg_list_destroy(int unit,
                         bcm_stg_t *list,
                         int count)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stg_stp_set(int unit,
                    bcm_stg_t stg,
                    bcm_port_t port,
                    int stp_state)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stg_stp_get(int unit,
                    bcm_stg_t stg,
                    bcm_port_t port,
                    int *stp_state)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_stg_count_get(int unit,
                      int *max_stg)
{
    return BCM_E_UNAVAIL;
}
