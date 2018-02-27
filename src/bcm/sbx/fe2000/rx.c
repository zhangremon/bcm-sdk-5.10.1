/*
 * $Id: rx.c 1.11 Broadcom SDK $
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
 * File:        rx.c
 */

#include <soc/drv.h>
#include <soc/debug.h>
#include <soc/sbx/sbx_drv.h>

#include <bcm/error.h>
#include <bcm/rx.h>
#include <bcm/debug.h>
#include <bcm/switch.h>
#include <bcm/stack.h>

#include <bcm_int/sbx/rx.h>
#include <bcm_int/sbx/error.h>

#ifdef BCM_FE2000_P3_SUPPORT
#include <soc/sbx/g2p3/g2p3.h>
#endif /* BCM_FE2000_P3_SUPPORT */



#define _fe2000_rx_D(string)   "[%d:%s]: " string, unit, FUNCTION_NAME()

#define RX_DEBUG(flags, stuff)  BCM_DEBUG(flags | BCM_DBG_RX, stuff)
#define RX_OUT(stuff)           RX_DEBUG(BCM_DBG_RX, stuff)
#define RX_WARN(stuff)          RX_DEBUG(BCM_DBG_WARN, stuff)
#define RX_ERR(stuff)           RX_DEBUG(BCM_DBG_ERR, stuff)
#define RX_VERB(stuff)          RX_DEBUG(BCM_DBG_VERBOSE, stuff)

#define _EXC_ARRAY_SIZE  15


int
sbx_fe2000_rx_config(int unit, bcm_rx_cfg_t *cfg)
{
    int rv = BCM_E_NONE;

    if (cfg->pkt_size != RX_PKT_SIZE_DFLT )
        rv = BCM_E_UNAVAIL;

    return rv;
}



#ifdef BCM_FE2000_P3_SUPPORT
/*
 *  Function:
 *    _bcm_fe2000_g2p3_rx_exc_base_qid_get
 *  Purpose:
 *    Get the base queue for all exceptions
 *  Parameters:
 *    (in)  unit   - bcm devcie number
 *    (out) baseQid - storage for baseQid
 */
int 
_bcm_fe2000_g2p3_rx_exc_base_qid_get(int unit, int *baseQid)
{
    int rv;
    int gport;
    uint32_t qePort, qeMod, qeNode;

    rv = bcm_switch_control_get(unit, bcmSwitchCpuCopyDestination, &gport);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    qeMod = BCM_GPORT_MODPORT_MODID_GET(gport);
    qePort = BCM_GPORT_MODPORT_PORT_GET(gport);
    
    /* check for switch gport*/
    if (!BCM_STK_MOD_IS_NODE(qeMod)) {
        int temp;
        if (qePort < SBX_MAX_PORTS && qeMod < SBX_MAX_MODIDS) {
            temp = SOC_SBX_CONTROL(unit)->modport[qeMod][qePort];
            qePort = temp & 0xffff;
            qeMod = (temp >> 16) & 0xffff;
        } else { 
            return BCM_E_PARAM;
        }
    }
    
    qeNode = BCM_STK_MOD_TO_NODE(qeMod);
    *baseQid = SOC_SBX_NODE_PORT_TO_QID(unit,qeNode, 
                                        qePort, NUM_COS(unit));
    RX_OUT((_fe2000_rx_D("Found baseQid=0x%04x, qeNode=%d qePort=%d\n"), 
            *baseQid, qeNode, qePort));

    return rv;
}
#endif

#ifdef BCM_FE2000_P3_SUPPORT
/*
 * Function:
 *   _bcm_fe2000_g2p3_rx_cosq_mapping_set
 * Purpose:
 *   Set the exception destination priority queue for the given set of reasons
 * Parameters:
 *   (in)   unit     - bcm device number
 *   (in)   reasons  - mappable bcm reason to sbx exceptions to update
 *   (in)   int_prio - bcm priority (7=high pri, 0=low pri)
*/
int
_bcm_fe2000_g2p3_rx_cosq_mapping_set(int unit, 
                                     bcm_rx_reasons_t reasons,
                                     uint8 int_prio)
                                     
{
    int                rv, rtn_rv;
    int                reasonIdx, excCount, excIdx;
    int                baseQid, qeCos;
    uint32_t           mappedExcs[_EXC_ARRAY_SIZE];
    soc_sbx_g2p3_xt_t  xt;

    rtn_rv = BCM_E_NONE;

    for (reasonIdx = 0; reasonIdx < bcmRxReasonCount; reasonIdx++) {
        
        rv = BCM_E_NONE;
        excCount = _EXC_ARRAY_SIZE;
        if (BCM_RX_REASON_GET(reasons, reasonIdx)) {
            rv = _bcm_to_sbx_reasons(unit, reasonIdx, mappedExcs, &excCount);
        
            if (BCM_FAILURE(rv) || excCount == 0) {
                rtn_rv = BCM_E_PARAM;
                RX_WARN((_fe2000_rx_D("error mapping reason %d to exception: %d %s\n"),
                         reasonIdx, rv, bcm_errmsg(rv)));
                /* Keep going, don't return */
            } else {
                
                /* get the base Exception Queue Id - */
                rv = _bcm_fe2000_g2p3_rx_exc_base_qid_get(unit, &baseQid);
                if (BCM_FAILURE(rv)) {
                    RX_ERR((_fe2000_rx_D("Failed to get base qid: %d %s\n"),
                            rv, bcm_errmsg(rv)));
                    return rv;
                }
                RX_OUT((_fe2000_rx_D("Found baseQid=0x%04x\n"), baseQid));
                
                /* for each mapped exception index, update the qid to reflect the 
                 * new COS.  Note that the QE's highest priority is 0, while
                 * the highest COS value is 7
                 */
                qeCos = (SBX_MAX_COS - 1) - int_prio;
                for (excIdx = 0; excIdx < excCount; excIdx++) {
                    RX_VERB((_fe2000_rx_D("Updating exception %d to QID=0x%04x "
                                "cos/prio=%d/%d\n"),
                             mappedExcs[excIdx], baseQid + qeCos, 
                             qeCos, int_prio));


                    rv = soc_sbx_g2p3_xt_get(unit, mappedExcs[excIdx], &xt);
                    if (BCM_FAILURE(rv)) {
                        RX_ERR((_fe2000_rx_D("failed to read exception %d: %d %s\n"),
                                mappedExcs[excIdx], rv, bcm_errmsg(rv)));
                        return rv;
                    }
                
                    xt.qid = baseQid + qeCos;
                
                    rv = soc_sbx_g2p3_xt_set(unit, mappedExcs[excIdx], &xt);
                    if (BCM_FAILURE(rv)) {
                        RX_ERR((_fe2000_rx_D("failed to write exception %d: %d %s\n"),
                                mappedExcs[excIdx], rv, bcm_errmsg(rv)));
                        return rv;
                    }
                }
            }
        }
    }    

    return rtn_rv;
}
#endif /* BCM_FE2000_P3_SUPPORT */


/*  
 *  Function:
 *    bcm_fe2000_rx_cosq_mapping_set
 *  Purpose:
 *    Set the exception destination priority queue for the given set of reasons
 *  Parameters:
 *    (in)  unit        - bcm device number
 *    (in)  reasons     - mappable reasons to sbx exceptions update
 *    (in)  reasons_mask - ignored
 *    (in)  int_prio    - bcm priority (7=high, 0=low)
 *    (in)  int_prio_mask- ignored
 *    (in)  packet_type  - ignored
 *    (in)  packet_typemask- ignored
 *    (in)  cosq         - ignored
 */

int
bcm_fe2000_rx_cosq_mapping_set(int unit, 
                               int index, 
                               bcm_rx_reasons_t reasons, 
                               bcm_rx_reasons_t reasons_mask, 
                               uint8 int_prio, 
                               uint8 int_prio_mask, 
                               uint32 packet_type, 
                               uint32 packet_type_mask, 
                               bcm_cos_queue_t cosq)
{
    int            rv = BCM_E_INTERNAL;

    if (int_prio > 7) {
        return BCM_E_PARAM;
    }

    switch(SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_rx_cosq_mapping_set(unit, reasons, int_prio);
        break;
#endif /* BCM_FE2000_P3_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
    }
        
   return rv;
}


#ifdef BCM_FE2000_P3_SUPPORT
/*  
 *  Function:
 *    bcm_fe2000_rx_cosq_mapping_get
 *  Purpose:
 *    Get the exception destination priority queue for ONE reason
 *  Parameters:
 *    (in)  unit        - bcm device number
 *    (in)  reasons     - mappable reasons to sbx exceptions update
 *    (out) int_prio    - bcm priority (7=high, 0=low)
 */
int
_bcm_fe2000_g2p3_rx_cosq_mapping_get(int unit, 
                                     bcm_rx_reasons_t *reasons, 
                                     uint8 *int_prio)
{
    int                rv;
    int                reasonIdx, excCount;
    int                baseQid;
    uint32_t           mappedExcs[_EXC_ARRAY_SIZE];
    soc_sbx_g2p3_xt_t  xt;
    
    for (reasonIdx = 0; reasonIdx < bcmRxReasonCount; reasonIdx++) {
        
        rv = BCM_E_NONE;
        excCount = _EXC_ARRAY_SIZE;
        if (BCM_RX_REASON_GET(*reasons, reasonIdx)) {
            rv = _bcm_to_sbx_reasons(unit, reasonIdx, mappedExcs, &excCount);


            if (BCM_FAILURE(rv) || excCount == 0) {
                RX_WARN((_fe2000_rx_D("error mapping reason %d to exception: %d %s\n"),
                         reasonIdx, rv, bcm_errmsg(rv)));
                /* Keep going, don't return */
            } else {

                /* get the base Exception Queue Id - */
                rv = _bcm_fe2000_g2p3_rx_exc_base_qid_get(unit, &baseQid);
                if (BCM_FAILURE(rv)) {
                    RX_ERR((_fe2000_rx_D("Failed to get base qid: %d %s\n"),
                            rv, bcm_errmsg(rv)));
                    return rv;
                }
                RX_OUT((_fe2000_rx_D("Found baseQid=0x%04x\n"), baseQid));

                /* Since all mapped reasons will have same cos, only need to
                 * read the first one; then return because this routine only
                 * supports one reason for the _get case
                 */

                rv = soc_sbx_g2p3_xt_get(unit, mappedExcs[0], &xt);
                if (BCM_FAILURE(rv)) {
                    RX_ERR((_fe2000_rx_D("failed to read exception %d: %d %s\n"),
                            mappedExcs[0], rv, bcm_errmsg(rv)));
                    return rv;
                }
                
                /* convert the QID to a QeCos, then to a priority */
                *int_prio = (baseQid - xt.qid) + (SBX_MAX_COS - 1);
                return rv;
            }
        }
    }
    
    return rv;
}
#endif



/*  
 *  Function:
 *    bcm_fe2000_rx_cosq_mapping_get
 *  Purpose:
 *    Get the exception destination priority queue for ONE reason
 *  Parameters:
 *    (in)  unit        - bcm device number
 *    (in)  reasons     - mappable reasons to sbx exceptions update
 *    (in)  reasons_mask - ignored
 *    (out)  int_prio    - bcm priority (7=high, 0=low)
 *    (in)  int_prio_mask- ignored
 *    (in)  packet_type  - ignored
 *    (in)  packet_typemask- ignored
 *    (in)  cosq         - ignored
 */
int
bcm_fe2000_rx_cosq_mapping_get(int unit, 
                               int index, 
                               bcm_rx_reasons_t *reasons, 
                               bcm_rx_reasons_t *reasons_mask, 
                               uint8 *int_prio, 
                               uint8 *int_prio_mask, 
                               uint32 *packet_type, 
                               uint32 *packet_type_mask, 
                               bcm_cos_queue_t *cosq)
{
    int            rv = BCM_E_INTERNAL;

    switch(SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        rv = _bcm_fe2000_g2p3_rx_cosq_mapping_get(unit, reasons, int_prio);
        break;
#endif /* BCM_FE2000_P3_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
    }
    
    return rv;
}
