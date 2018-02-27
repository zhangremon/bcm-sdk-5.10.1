/*
 * $Id: stg.c 1.25 Broadcom SDK $
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
 * Purpose:     Spanning tree group support implementation for g2p3
 *
 * Multiple spanning trees (MST) is supported on this chipset
 */
#ifdef BCM_FE2000_P3_SUPPORT

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/g2p3/g2p3.h>

#include <bcm/types.h>
#include <bcm/debug.h>
#include <bcm/error.h>
#include <bcm/stg.h>
#include <bcm/vlan.h>

#include <bcm_int/control.h>
#include <bcm_int/common/lock.h>
#include <bcm_int/sbx/error.h>
#include <bcm_int/sbx/stg.h>
#include <bcm_int/sbx/fe2000/vlan.h>



/* Debug Macros */
#define BCM_STG_ERR(stuff)          BCM_DEBUG(BCM_DBG_L2 | BCM_DBG_ERR, stuff)
#define BCM_STG_WARN(stuff)         BCM_DEBUG(BCM_DBG_L2 | BCM_DBG_WARN, stuff)
#define BCM_STG_VERB(stuff)         BCM_DEBUG(BCM_DBG_L2 | BCM_DBG_VERBOSE, stuff)
#define BCM_STG_VVERB(stuff)        BCM_DEBUG(BCM_DBG_L2 | BCM_DBG_VVERBOSE, stuff)
#define BCM_STG_MSG(stuff)          BCM_DEBUG(BCM_DBG_NORMAL, stuff)

#define _g2p3_stg_D(string)   "[%d:%s]: " string, unit, FUNCTION_NAME()

uint32_t
_bcm_fe2000_g2p3_stg_stp_translate(int unit, bcm_stg_stp_t bcm_state)
{
    soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);

    if (bcm_state == BCM_STG_STP_FORWARD
        || bcm_state == BCM_STG_STP_DISABLE) {
        return sbx->stpforward;
    } else if (bcm_state == BCM_STG_STP_LEARN) {
        return sbx->stplearn;
    }

    return sbx->stpblock;
}

bcm_stg_stp_t
_bcm_fe2000_g2p3_stg_stp_translate_to_bcm(int unit, int stpstate)
{
    soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);

    if (stpstate == sbx->stpforward) {
        return BCM_STG_STP_FORWARD;
    } else if (stpstate == sbx->stplearn) {
        return BCM_STG_STP_LEARN;
    }

    return BCM_STG_STP_BLOCK;
}

int
_bcm_fe2000_g2p3_stg_vid_stp_set(int unit, bcm_vlan_t vid, bcm_port_t port,
                                 int stp_state)
{
    soc_sbx_g2p3_pv2e_t   sbx_pvid;
    soc_sbx_g2p3_epv2e_t  sbx_egr_pvid;

    BCM_IF_ERROR_RETURN(SOC_SBX_G2P3_PV2E_GET(unit, port, vid, &sbx_pvid));


    sbx_pvid.stpstate = _bcm_fe2000_g2p3_stg_stp_translate(unit, stp_state);

    BCM_IF_ERROR_RETURN(_soc_sbx_g2p3_pv2e_set(unit, port, vid, &sbx_pvid));

    BCM_IF_ERROR_RETURN(SOC_SBX_G2P3_EPV2E_GET(unit, port, vid,
                                               &sbx_egr_pvid));

    sbx_egr_pvid.drop = ((stp_state == BCM_STG_STP_FORWARD ||
                          stp_state == BCM_STG_STP_DISABLE) ? 0 : 1);
    BCM_IF_ERROR_RETURN(SOC_SBX_G2P3_EPV2E_SET(unit, port, vid,
                                               &sbx_egr_pvid));

    return BCM_E_NONE;
}

int
_bcm_fe2000_g2p3_stg_vid_stp_get(int unit,
                                 bcm_vlan_t vid,
                                 bcm_port_t port,
                                 int *stp_state)
{
    soc_sbx_g2p3_pv2e_t   sbx_pvid;

    soc_sbx_g2p3_pv2e_t_init(&sbx_pvid);
    BCM_IF_ERROR_RETURN(SOC_SBX_G2P3_PV2E_GET(unit, port, vid, &sbx_pvid));

    *stp_state = _bcm_fe2000_g2p3_stg_stp_translate_to_bcm(unit,
                                                           sbx_pvid.stpstate);

    return BCM_E_NONE;
}

int
_bcm_fe2000_g2p3_stg_stacked_vid_stp_set_get(int unit,
                                         bcm_vlan_t ovid,
                                         bcm_vlan_t ivid,
                                         bcm_port_t port,
                                         int *stp_state,
                                         int set_or_get)
{
    soc_sbx_g2p3_epv2e_t  sbx_egr_pvid;
    soc_sbx_g2p3_pvv2e_t  sbx_pvv2e;

    soc_sbx_g2p3_pvv2e_t_init(&sbx_pvv2e);

    BCM_IF_ERROR_RETURN
       (soc_sbx_g2p3_pvv2e_get(unit, ivid, ovid, port, &sbx_pvv2e));

    if (set_or_get) { /* SET */
        sbx_pvv2e.stpstate =
            _bcm_fe2000_g2p3_stg_stp_translate(unit, *stp_state);

        BCM_IF_ERROR_RETURN
            (soc_sbx_g2p3_pvv2e_set(unit, ivid, ovid, port, &sbx_pvv2e));

        soc_sbx_g2p3_epv2e_t_init(&sbx_egr_pvid);
        BCM_IF_ERROR_RETURN
            (SOC_SBX_G2P3_EPV2E_GET(unit, port, ovid, &sbx_egr_pvid));

        sbx_egr_pvid.drop = ((*stp_state == BCM_STG_STP_FORWARD ||
                              *stp_state == BCM_STG_STP_DISABLE) ? 0 : 1);
        BCM_IF_ERROR_RETURN
            (SOC_SBX_G2P3_EPV2E_SET(unit, port, ovid, &sbx_egr_pvid));
    } else { /* GET */
        *stp_state =
            _bcm_fe2000_g2p3_stg_stp_translate_to_bcm(unit,
                                                      sbx_pvv2e.stpstate);
    }

    return BCM_E_NONE;
}

int
_bcm_fe2000_g2p3_stg_label_stp_set_get(int unit,
                                       bcm_mpls_label_t label,
                                       int *stp_state,
                                       int set_or_get)
{
    soc_sbx_g2p3_l2e_t sbx_l2e;

    soc_sbx_g2p3_l2e_t_init(&sbx_l2e);

    BCM_IF_ERROR_RETURN
       (soc_sbx_g2p3_l2e_get(unit, label, &sbx_l2e));

    if (set_or_get) { /* SET */
        sbx_l2e.stpstate = _bcm_fe2000_g2p3_stg_stp_translate(unit,
                                                              *stp_state);

        BCM_IF_ERROR_RETURN
           (soc_sbx_g2p3_l2e_set(unit, label, &sbx_l2e));
    } else { /* GET */
        *stp_state = _bcm_fe2000_g2p3_stg_stp_translate_to_bcm(unit,
                                                             sbx_l2e.stpstate);
    }

    return BCM_E_NONE;
}

int
_bcm_fe2000_g2p3_stg_init(int unit)
{
    uint32 exc_idx;
    soc_sbx_g2p3_xt_t     sbx_xt;    /* exception table */
    int                   rv = BCM_E_NONE;

    /* enable support for learning on STP_BLOCKED exception */
    BCM_IF_ERROR_RETURN(soc_sbx_g2p3_exc_stp_blocked_idx_get(unit, &exc_idx));

    BCM_IF_ERROR_RETURN(soc_sbx_g2p3_xt_get(unit, exc_idx, &sbx_xt));
    sbx_xt.learn = 1;
    BCM_IF_ERROR_RETURN(soc_sbx_g2p3_xt_set(unit, exc_idx, &sbx_xt));

    return rv;
}


int
_bcm_fe2000_g2p3_stp_fast_set(int unit, bcm_port_t port,
                              uint32_t stpState,
                              int fastSets[BCM_VLAN_COUNT])
{
    int rv, setNative;
    bcm_vlan_t vid, minVid, maxVid, nativeVid;
    uint32_t hwStpState, drop;
    uint32_t *fastStates = NULL, *fastDrops = NULL;

    fastStates = sal_alloc(BCM_VLAN_COUNT * sizeof(uint32_t),
                           "fastStates temp");
    if (fastStates == NULL) {
        return BCM_E_MEMORY;
    }

    fastDrops = sal_alloc(BCM_VLAN_COUNT * sizeof(uint32_t),
                           "fastStates temp");
    if (fastDrops == NULL) {
        sal_free(fastStates);
        return BCM_E_MEMORY;
    }

    minVid = BCM_VLAN_MAX;
    maxVid = BCM_VLAN_MIN;
    setNative = 0;

    rv = _bcm_fe2000_vlan_port_native_vlan_get(unit, port, &nativeVid);
    if (BCM_E_NONE != rv) {
        goto error;
    }

    hwStpState = _bcm_fe2000_g2p3_stg_stp_translate(unit, stpState);
    drop = !(stpState == BCM_STG_STP_FORWARD
             || stpState == BCM_STG_STP_DISABLE);

    for (vid = BCM_VLAN_MIN;  vid < BCM_VLAN_COUNT; vid++) {
        if (fastSets[vid] == 0) {
            continue;
        }

        if (vid > maxVid) {
            maxVid = vid;
        }
        if (vid < minVid) {
            minVid = vid;
        }
        if (vid == nativeVid) {
            setNative = 1;
        }

        fastStates[vid] = hwStpState;
        fastDrops[vid] = drop;
    }

    if (maxVid < minVid) {
        rv = BCM_E_PARAM;
        goto error;
    }

    rv = soc_sbx_g2p3_pv2e_stpstate_fast_set(unit, minVid, port,
                                             maxVid, port,
                                             &fastSets[minVid],
                                             &fastStates[minVid],
                                             BCM_VLAN_COUNT);
    if (BCM_FAILURE(rv)) {
        BCM_STG_ERR
            ((_g2p3_stg_D("failed to fast set pv2e, p=0x%x,minvid=0x%x,maxvid=0x%x\n"),
              port, minVid, maxVid));
        goto error;
    }

    rv = soc_sbx_g2p3_epv2e_drop_fast_set(unit, minVid, port,
                                          maxVid, port,
                                          &fastSets[minVid],
                                          &fastDrops[minVid],
                                          BCM_VLAN_COUNT);
    if (BCM_FAILURE(rv)) {
        BCM_STG_ERR
            ((_g2p3_stg_D("failed to fast set epv2e, p=0x%x,minvid=0x%x,maxvid=0x%x\n"),
              port, minVid, maxVid));
        goto error;
    }

    if (setNative) {
        rv = bcm_fe2000_g2p3_vlan_port_nativevid_touch(unit, port);
        if (BCM_FAILURE(rv)) {
            BCM_STG_ERR((_g2p3_stg_D("failed to refresh native VID, p=0x%x\n"), port));
            goto error;
        }
    }

 error:
    if (fastStates) {
        sal_free(fastStates);
    }
    if (fastDrops) {
        sal_free(fastDrops);
    }

    return rv;
}


#endif /* BCM_FE2000_P3_SUPPORT */
