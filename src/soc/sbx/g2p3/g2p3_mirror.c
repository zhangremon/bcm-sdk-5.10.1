/*
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
 * $Id: g2p3_mirror.c 1.5.112.1 Broadcom SDK $
 *
 *-----------------------------------------------------------------------------*/
#include <soc/sbx/g2p3/g2p3_int.h>

#define PUBLIC
#define PRIVATE static


int soc_sbx_g2p3_emirror_init(int unit)
{
    soc_sbx_control_t *sbx;
    soc_sbx_g2p3_state_t *fe;
    sbx = SOC_SBX_CONTROL(unit);
    if (sbx == NULL) {
        return SOC_E_INIT;
    }

    fe = sbx->drv;
    if (fe == NULL) {
        return SOC_E_MEMORY;
    }

    if (SAND_HAL_IS_FE2KXT(unit)){
        if (SOC_SBX_G2P3_ERH_IS_DEFAULT(fe)) {
            /* configure the mirror header mask */
            SAND_HAL_WRITE(unit, C2, PT_MIRROR_HEADER_MASK0, 0xfffc0000);
            SAND_HAL_WRITE(unit, C2, PT_MIRROR_HEADER_MASK1, 0x01ffff00);
        }else if (SOC_SBX_G2P3_ERH_IS_SIRIUS(fe)) {
            /* configure the mirror header mask */
            SAND_HAL_WRITE(unit, C2, PT_MIRROR_HEADER_MASK0, 0x0000ffff);
            SAND_HAL_WRITE(unit, C2, PT_MIRROR_HEADER_MASK1, 0x3fff0000);
        }else if (SOC_SBX_G2P3_ERH_IS_QESS(fe)) {
            
            /* configure the mirror header mask */
            SAND_HAL_WRITE(unit, C2, PT_MIRROR_HEADER_MASK0, 0xfffc0000);
            SAND_HAL_WRITE(unit, C2, PT_MIRROR_HEADER_MASK1, 0x01ffff00);
        }else{
            return SOC_E_PARAM;
        }
    }

    return SOC_E_NONE;
}

int soc_sbx_g2p3_emirror_set_ext(int unit,
                                 int imirror,
                                 soc_sbx_g2p3_emirror_t *e)
{
    soc_sbx_g2p3_emirror20_t e20;
    soc_sbx_g2p3_emirror25_t e25;
    sbFe2000Queues_t *queues;
    soc_sbx_g2p3_state_t *fe;
    soc_sbx_control_t *sbx;

    sbx = SOC_SBX_CONTROL(unit);
    if (sbx == NULL) {
        return SOC_E_INIT;
    }

    fe = sbx->drv;
    if (fe == NULL) {
        return SOC_E_MEMORY;
    }

    /* ENHANCEME...port could be stored in an unused portion of the
     * headerdata.  This would ease the _get function below.
     */
    if (SAND_HAL_IS_FE2KXT(unit)){
        soc_sbx_g2p3_emirror25_t_init(&e25);
        queues = &SOC_SBX_CFG_FE2000(unit)->xt_init_params.queues;

        if (e->local) {
            e25.hdrpresent = 0;
            /* determine SWS queue for the egress port */
            e25.queue = queues->port2eqid[e->port] + SB_FE2000XT_TO_QUEUE_OFFSET;
        }else{
                e25.hdrpresent = 1;
                e25.overwritehdr = 1;
            if (SOC_SBX_G2P3_ERH_IS_DEFAULT(fe)) {
                /* set oi and qid in hdrdata */
                e25.hdrdata0 = e->qid << 18;
                e25.hdrdata1 = e->oi << 8;
            }else if (SOC_SBX_G2P3_ERH_IS_SIRIUS(fe)) {
                /* set oi and qid in hdrdata */
                e25.hdrdata0 = e->qid << 0;
                e25.hdrdata1 = (e->oi & 0xffff) << 16;
            }else if (SOC_SBX_G2P3_ERH_IS_DEFAULT(fe)) {
                
                /* set oi and qid in hdrdata */
                e25.hdrdata0 = e->qid << 18;
                e25.hdrdata1 = e->oi << 8;
            }
            /* super simple load balancing */
            if ((e->port % 2) == 0) {
                e25.queue = (queues->mir0qid ? queues->mir0qid : queues->mir1qid );
            }else{
                e25.queue = (queues->mir1qid ? queues->mir1qid : queues->mir0qid );
            }
        }

        return soc_sbx_g2p3_emirror25_set(unit, imirror, &e25);
    }else{
        soc_sbx_g2p3_emirror20_t_init(&e20);
        queues = &SOC_SBX_CFG_FE2000(unit)->init_params.queues;
        /* local mirroring only */
        if (!e->local){
            return SOC_E_PARAM;
        }else{
            e20.hdrpresent = 0;
            /* determine SWS queue for the egress port */
            e20.queue = queues->port2eqid[e->port] + SB_FE2000XT_TO_QUEUE_OFFSET;
        }

        return soc_sbx_g2p3_emirror20_set(unit, imirror, &e20);
    }

}

int soc_sbx_g2p3_emirror_get_ext(int unit,
                                 int imirror,
                                 soc_sbx_g2p3_emirror_t *e)
{
    soc_sbx_g2p3_emirror20_t e20;
    soc_sbx_g2p3_emirror25_t e25;
    sbFe2000Queues_t *queues;
    uint32_t i;
    int rv;

    soc_sbx_g2p3_emirror_t_init(e);

    if (SAND_HAL_IS_FE2KXT(unit)){
        soc_sbx_g2p3_emirror25_t_init(&e25);
        rv = soc_sbx_g2p3_emirror25_get(unit, imirror, &e25);
        if (rv != SOC_E_NONE){
            return rv;
        }
        if (e25.hdrpresent == 1) {
            e->local = 0;
            e->qid = e25.hdrdata0 >> 18;
            e->oi = e25.hdrdata1 >> 8;
        }else{
            e->local = 1;
            queues = &SOC_SBX_CFG_FE2000(unit)->xt_init_params.queues;
            for (i=0; i<(SB_FE2000_SWS_INIT_MAX_CONNECTIONS/2); i++){
                if (queues->port2eqid[i] == (e25.queue - SB_FE2000XT_TO_QUEUE_OFFSET)){
                    e->port = i;
                    break;
                }
            }
            if (i == (SB_FE2000_SWS_INIT_MAX_CONNECTIONS/2)){
                return SOC_E_NOT_FOUND;
            }
        }
    }else{
        soc_sbx_g2p3_emirror20_t_init(&e20);
        rv = soc_sbx_g2p3_emirror20_get(unit, imirror, &e20);
        if (rv != SOC_E_NONE){
            return rv;
        }
        if (e20.hdrpresent == 1) {
            e->local = 0;
            
        }else{
            e->local = 1;
            queues = &SOC_SBX_CFG_FE2000(unit)->init_params.queues;
            for (i=0; i<(SB_FE2000_SWS_INIT_MAX_CONNECTIONS/2); i++){
                if (queues->port2eqid[i] == (e20.queue - SB_FE2000XT_TO_QUEUE_OFFSET)){
                    e->port = i;
                    break;
                }
            }
            if (i == (SB_FE2000_SWS_INIT_MAX_CONNECTIONS/2)){
                return SOC_E_NOT_FOUND;
            }
        }
    }

    return SOC_E_NONE;
}
