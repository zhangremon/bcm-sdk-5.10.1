/* 
 * $Id: fe2000_counter.h 1.4 Broadcom SDK $
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
 * File:        fe2000_counter.h
 * Purpose:     SBX Software Counter Collection module definitions for FE2000.
 */

#ifndef   _SOC_SBX_FE2000_COUNTER_H_
#define   _SOC_SBX_FE2000_COUNTER_H_

#include <soc/sbx/counter.h>


/*
 * Counter Blocks Types
 */
typedef enum soc_fe2000_counter_block_e {
    fe2000CounterBlockAm = 0,
    fe2000CounterBlockXm,
    fe2000CounterBlockIlib,
    fe2000CounterBlockCount,
} soc_fe2000_counter_block_t;

/*
 * Counter Set definition for AM Block (unimac)
 */
typedef enum fe2000_counter_am_e {
    fe2000CounterAmRxFcsErrorCnt = 0,
    fe2000CounterAmRxCtrlPktCnt,
    fe2000CounterAmRxPausePktCnt,
    fe2000CounterAmRxUnknownCtrlPktCnt,
    fe2000CounterAmRxAlignmentErrorPktCnt,
    fe2000CounterAmRxBadLengthPktCnt,
    fe2000CounterAmRxInvalidDataCnt,
    fe2000CounterAmRxFalseCarrierCnt,
    fe2000CounterAmRxOversizePktCnt,
    fe2000CounterAmRxJabberPktCnt,
    fe2000CounterAmRxMtuExceededPktCnt,
    fe2000CounterAmRxRuntPktCnt,
    fe2000CounterAmRxUndersizePktCnt,
    fe2000CounterAmRxFragmentPktCnt,
    fe2000CounterAmRxRuntByteCnt,
    fe2000CounterAmRxMcPktCnt,
    fe2000CounterAmRxBroadcastPktCnt,
    fe2000CounterAmRxRange64PktCnt,
    fe2000CounterAmRxRange127PktCnt,
    fe2000CounterAmRxRange255PktCnt,
    fe2000CounterAmRxRange511PktCnt,
    fe2000CounterAmRxRange1023PktCnt,
    fe2000CounterAmRxRange1518PktCnt,
    fe2000CounterAmRxRange1522PktCnt,
    fe2000CounterAmRxRange2047PktCnt,
    fe2000CounterAmRxRange4095PktCnt,
    fe2000CounterAmRxRange9216PktCnt,
    fe2000CounterAmRxPktCnt,
    fe2000CounterAmRxByteCnt,
    fe2000CounterAmRxUcPktCnt,
    fe2000CounterAmRxValidPktCnt,
    fe2000CounterAmTxPausePktCnt,
    fe2000CounterAmTxJabberPktCnt,
    fe2000CounterAmTxFcsErrorCnt,
    fe2000CounterAmTxCtrlPktCnt,
    fe2000CounterAmTxOversizePktCnt,
    fe2000CounterAmTxSingleDeferralPktCnt,
    fe2000CounterAmTxMultipleDeferralPktCnt,
    fe2000CounterAmTxSingleCollisionPktCnt,
    fe2000CounterAmTxMultipleCollisionPktCnt,
    fe2000CounterAmTxLateCollisionPktCnt,
    fe2000CounterAmTxExcessiveCollisionPktCnt,
    fe2000CounterAmTxFragmentPktCnt,
    fe2000CounterAmTxTotalCollisionCnt,
    fe2000CounterAmTxMcPktCnt,
    fe2000CounterAmTxBroadcastPktCnt,
    fe2000CounterAmTxRange64PktCnt,
    fe2000CounterAmTxRange127PktCnt,
    fe2000CounterAmTxRange255PktCnt,
    fe2000CounterAmTxRange511PktCnt,
    fe2000CounterAmTxRange1023PktCnt,
    fe2000CounterAmTxRange1518PktCnt,
    fe2000CounterAmTxRange1522PktCnt,
    fe2000CounterAmTxRange2047PktCnt,
    fe2000CounterAmTxRange4095PktCnt,
    fe2000CounterAmTxRange9216PktCnt,
    fe2000CounterAmTxPktCnt,
    fe2000CounterAmTxByteCnt,
    fe2000CounterAmTxUcPktCnt,
    fe2000CounterAmTxValidPktCnt,
    fe2000CounterAmCount
} fe2000_counter_am_t;

/*
 * Counter Set definition for XM Block (xmac)
 */
typedef enum fe2000_counter_xm_e {
    fe2000CounterXmRxRange64PktCnt = 0,
    fe2000CounterXmRxRange127PktCnt,
    fe2000CounterXmRxRange255PktCnt,
    fe2000CounterXmRxRange511PktCnt,
    fe2000CounterXmRxRange1023PktCnt,
    fe2000CounterXmRxRange1518PktCnt,
    fe2000CounterXmRxRange2047PktCnt,
    fe2000CounterXmRxRange4095PktCnt,
    fe2000CounterXmRxRange9216PktCnt,
    fe2000CounterXmRxRange16383PktCnt,
    fe2000CounterXmRxMaxSizePktCnt,
    fe2000CounterXmRxPktCnt,
    fe2000CounterXmRxFcsPktCnt,
    fe2000CounterXmRxUnicastPktCnt,
    fe2000CounterXmRxMcPktCnt,
    fe2000CounterXmRxBroadcastPktCnt,
    fe2000CounterXmRxCtrlPktCnt,
    fe2000CounterXmRxPausePktCnt,
    fe2000CounterXmRxUnknownCtrlPktCnt,
    fe2000CounterXmRxJabberPktCnt,
    fe2000CounterXmRxOversizePktCnt,
    fe2000CounterXmRxBadLengthPktCnt,
    fe2000CounterXmRxAllPktCnt,
    fe2000CounterXmRxMtuExceededPktCnt,
    fe2000CounterXmRxBadOversizePktCnt,
    fe2000CounterXmRxByteCnt,
    fe2000CounterXmRxUndersizePktCnt,
    fe2000CounterXmRxFragmentPktCnt,
    fe2000CounterXmRxErrorPktCnt,
    fe2000CounterXmRxFrameErrorCnt,
    fe2000CounterXmRxIpgJunkCnt,
    fe2000CounterXmTxAllPktCnt,
    fe2000CounterXmTxPauseCnt,
    fe2000CounterXmTxFcsErrorCnt,
    fe2000CounterXmTxUnicastPktCnt,
    fe2000CounterXmTxMcPktCnt,
    fe2000CounterXmTxBroadcastPktCnt,
    fe2000CounterXmTxOversizePktCnt,
    fe2000CounterXmTxFragmentPktCnt,
    fe2000CounterXmTxPktCnt,
    fe2000CounterXmTxRange64PktCnt,
    fe2000CounterXmTxRange127PktCnt,
    fe2000CounterXmTxRange255PktCnt,
    fe2000CounterXmTxRange511PktCnt,
    fe2000CounterXmTxRange1023PktCnt,
    fe2000CounterXmTxRange1518PktCnt,
    fe2000CounterXmTxRange2047PktCnt,
    fe2000CounterXmTxRange4095PktCnt,
    fe2000CounterXmTxRange9216PktCnt,
    fe2000CounterXmTxRange16383PktCnt,
    fe2000CounterXmTxMaxSizePktCnt,
    fe2000CounterXmTxUnderflowPktCnt,
    fe2000CounterXmTxErrorPktCnt,
    fe2000CounterXmTxByteCnt,
    fe2000CounterXmCount
} fe2000_counter_xm_t;

/*
 * Counter Set definition for Ilib Block
 */
typedef enum fe2000_counter_ilib_e {
    fe2000CounterIlibDiscardPktCnt = 0,
    fe2000CounterIlibCount
} fe2000_counter_ilib_t;


extern int soc_sbx_fe2000_counter_init(int unit, uint32 flags,
                                       int interval, pbmp_t pbmp);

extern int soc_sbx_fe2000_counter_port_get(int unit, soc_port_t port,
                                           int block, int counter,
                                           uint64 *val);

extern int soc_sbx_fe2000_counter_port_set(int unit, soc_port_t port,
                                           int block, int counter,
                                           uint64 val);

extern int soc_sbx_fe2000_counter_max_pkt_size(int unit, int max_size);

#endif /* _SOC_SBX_FE2000_COUNTER_H_ */
