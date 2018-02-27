/*
 * $Id: igmp.c 1.10 Broadcom SDK $
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
 * IGMP Snooping
 */

#include <soc/sbx/sbx_drv.h>


#ifdef BCM_FE2000_P3_SUPPORT
#include <soc/sbx/g2p3/g2p3.h>
#endif

#include <bcm_int/sbx/error.h>
#include <bcm/vlan.h>

int
bcm_fe2000_igmp_snooping_init(int unit)
{
    bcm_vlan_data_t *listp = NULL;
    int              count = 0;
    int              rv = BCM_E_NONE;

#ifdef BCM_FE2000_P3_SUPPORT
    soc_sbx_g2p3_v2e_t v2e;
#ifdef BCM_QE2000_SUPPORT
    soc_sbx_g2p3_xt_t  xt;
#endif
#endif /* BCM_FE2000_P3_SUPPORT */

    /*
     * Turn on IGMP snooping on default VID
     */
    BCM_IF_ERROR_RETURN
       (bcm_vlan_list(unit, &listp, &count));

    if (count > 0) {
        int i;

        for (i = 0; i < count; i++) {
            switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
            case SOC_SBX_UCODE_TYPE_G2P3:
                soc_sbx_g2p3_v2e_t_init(&v2e);
                rv = soc_sbx_g2p3_v2e_get(unit, 
                                          SBX_VSI_FROM_VID(listp[i].vlan_tag),
                                          &v2e);
                if (rv == BCM_E_NONE) {
                    v2e.igmp = FALSE;
                    rv = soc_sbx_g2p3_v2e_set(unit, 
                                              SBX_VSI_FROM_VID(listp[i].vlan_tag),
                                              &v2e);
                }
                break;
#endif /* BCM_FE2000_P3_SUPPORT */
            default:
                SBX_UNKNOWN_UCODE_WARN(unit);
                rv = BCM_E_INTERNAL;
                break;
            }
            
            if (rv != BCM_E_NONE) {
                BCM_IF_ERROR_RETURN(bcm_vlan_list_destroy(unit, listp, count));
                return rv;
            }
        }
    }

    BCM_IF_ERROR_RETURN(bcm_vlan_list_destroy(unit, listp, count));

#ifdef BCM_QE2000_SUPPORT
    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
    {
        uint32            exc_idx;
        
        BCM_IF_ERROR_RETURN(soc_sbx_g2p3_exc_igmp_idx_get(unit, &exc_idx));

        soc_sbx_g2p3_xt_t_init(&xt);
        rv = soc_sbx_g2p3_xt_get(unit, exc_idx, &xt);
        if (rv == BCM_E_NONE) {
            /*xt.qid = SBX_EXC_QID_BASE;*/
            xt.forward = TRUE;
            rv = soc_sbx_g2p3_xt_set(unit, exc_idx, &xt); 
        }
    }
    break;
#endif /* BCM_FE2000_P3_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
        break;
    }
 
#else   /* BCM_QE2000_SUPPORT */
    rv = BCM_E_UNAVAIL;
#endif  /* BCM_QE2000_SUPPORT */

    return rv;
}

int
bcm_fe2000_igmp_snooping_enable_get(int unit, int *enable)
{
    int              rv = BCM_E_NONE;

#ifdef BCM_QE2000_SUPPORT
    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
    {
        soc_sbx_g2p3_xt_t xt;
        uint32            exc_idx;

        BCM_IF_ERROR_RETURN(soc_sbx_g2p3_exc_igmp_idx_get(unit, &exc_idx));

        soc_sbx_g2p3_xt_t_init(&xt);
        rv = soc_sbx_g2p3_xt_get(unit, exc_idx, &xt);
        if (rv == BCM_E_NONE) {
            *enable = (xt.forward && (xt.qid == SBX_EXC_QID_BASE(unit)));
        }
    }
    break;
#endif /* BCM_FE2000_P3_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
        break;
    }

#else /* BCM_QE2000_SUPPORT */
    rv = BCM_E_UNAVAIL;
#endif /* BCM_QE2000_SUPPORT */

    return rv;
}

int
bcm_fe2000_igmp_snooping_enable_set(int unit, int enable)
{
    int              rv = BCM_E_NONE;

#ifdef BCM_QE2000_SUPPORT
    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
    {
        soc_sbx_g2p3_xt_t xt;
        uint32            exc_idx;

        BCM_IF_ERROR_RETURN(soc_sbx_g2p3_exc_igmp_idx_get(unit, &exc_idx));

        soc_sbx_g2p3_xt_t_init(&xt);
        rv = soc_sbx_g2p3_xt_get(unit, exc_idx, &xt);
        if (rv == BCM_E_NONE) {
            if (enable) {
                xt.forward = TRUE;
                /* xt.qid = SBX_EXC_QID_BASE; */
            } else {
                xt.forward = FALSE;
                /* xt.qid = 0; */
            }
            rv = soc_sbx_g2p3_xt_set(unit, exc_idx, &xt);
        }
    }
    break;
#endif /* BCM_FE2000_P3_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
        break;
    }
#else /* BCM_QE2000_SUPPORT */
    rv = BCM_E_UNAVAIL;
#endif /* BCM_QE2000_SUPPORT */

    return rv;
}
