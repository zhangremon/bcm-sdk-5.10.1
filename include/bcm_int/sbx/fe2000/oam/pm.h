/*
 * $Id: pm.h 1.6 Broadcom SDK $
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
 */
#ifndef _BCM_INT_SBX_FE2000_OAM_PM_H_
#define _BCM_INT_SBX_FE2000_OAM_PM_H_


/* g2p3 ucode requires 2 consecutive counter ids for loss measurement.
 * The size is cut in half where 
 *   RxCounterIdx = id * 2
 *   TxCounterIdx = (id * 2) + 1
 */
#define OAM_NUM_COHERENT_COUNTERS   (8 * 1024)
#define OAM_COCO_BASE(x)    ((x) * 2)
#define OAM_COCO_RES(x)     ((x) / 2)
#define OAM_COCO_RX(x)      (x)
#define OAM_COCO_TX(x)      (OAM_COCO_RX(x) + 1)

#define OAM_COCO_VALID(x)     ((x) < OAM_NUM_COHERENT_COUNTERS)
#define OAM_COCO_INVAID_ID     OAM_NUM_COHERENT_COUNTERS


#define SUPPORTED_DELAY_FLAGS (BCM_OAM_DELAY_ONE_WAY | BCM_OAM_DELAY_TX_ENABLE)
#define SUPPORTED_LOSS_FLAGS  (BCM_OAM_LOSS_TX_ENABLE              | \
                               BCM_OAM_LOSS_SINGLE_ENDED           | \
                               BCM_OAM_LOSS_FIRST_RX_COPY_TO_CPU)

extern int _oam_g2p3_lm_create(int unit, 
                               int flags,
                               uint16_t lmIdx, 
                               uint16_t epIdx, 
                               int cocoIdx,
                               int multId,
                               int oameptype);

extern int _oam_g2p3_coco_configure(int unit,
                                    oam_sw_hash_data_t *ep_data,
                                    int coco_idx);

extern int _oam_g2p3_dm_create(int unit, 
                               int flags, 
                               uint16_t dmIdx, 
                               uint16_t epIdx, 
                               uint32_t ftIdx);

extern int convert_ep_to_time_spec(bcm_time_spec_t* bts,
                                   int sec,
                                   int ns);

extern int time_spec_subtract(bcm_time_spec_t* d,
                              bcm_time_spec_t* m,
                              bcm_time_spec_t* s);

#endif  /* _BCM_INT_SBX_FE2000_OAM_PM_H_  */
