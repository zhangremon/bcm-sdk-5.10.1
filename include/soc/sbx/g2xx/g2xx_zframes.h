/*
 * $Id: g2xx_zframes.h 1.3 Broadcom SDK $
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
 * Includes for zframes of g2xx
 */

#ifndef _SOC_SBX_G2XX_ZFRAMES_H
#define _SOC_SBX_G2XX_ZFRAMES_H

#ifdef BCM_SBX_SUPPORT
#include <soc/sbx/sbWrappers.h>

#include <soc/sbx/fe2kxt/sbZfC2PpAggregateHashBitConfigEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpAggregateHashBitConfigEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpAggregateHashByteConfigEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpAggregateHashByteConfigEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpAggregateHashConfigEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpAggregateHashConfigEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpAggregateHashFormat.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpAggregateHashFormatConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpByteHashConverter.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpByteHashConverterConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPpCamConfigurationEntry.hx>
#include <soc/sbx/fe2k/sbZfCaPpCamConfigurationEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpCamConfigurationInstance0Entry.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpCamConfigurationInstance0EntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpCamQueryFormat.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpCamQueryFormatConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpCamRamConfigurationInstance0Entry.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpCamRamConfigurationInstance0EntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpCamStateFormat.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpCamStateFormatConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPpCamRamConfigurationEntry.hx>
#include <soc/sbx/fe2k/sbZfCaPpCamRamConfigurationEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpCamStateRawFormat.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpCamStateRawFormatConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpDmacFormat.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpDmacFormatConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpDmacMaskFormat.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpDmacMaskFormatConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpFilterDataOverlayFormat.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpFilterDataOverlayFormatConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpFinalStreamSelectionAddress.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpFinalStreamSelectionAddressConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpFinalStreamSelectionEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpFinalStreamSelectionEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpHashTemplateEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpHashTemplateEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpHeaderRecordSizeEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpHeaderRecordSizeEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpInitialQueueStateEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpInitialQueueStateEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpLocalStationMatchAddress.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpLocalStationMatchAddressConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpPpOutHeaderCopyEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpPpOutHeaderCopyEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpQueueConfigEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpQueueConfigEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpQueuePriorityGroupEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpQueuePriorityGroupEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpRxPortDataEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpRxPortDataEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpRxPortDataPpDescriptorFormat.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpRxPortDataPpDescriptorFormatConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpRxPortMaskEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpRxPortMaskEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpStreamSelectionStateAddress.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpStreamSelectionStateAddressConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpStreamSelectionStateEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpStreamSelectionStateEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpSwtoPpInterface.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpSwtoPpInterfaceConsole.hx>

#ifdef C2_RCE

#include <soc/sbx/fe2kxt/sbZfC2RcDataEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2RcDataEntry2.hx>
#include <soc/sbx/fe2kxt/sbZfC2RcDataEntry2Console.hx>
#include <soc/sbx/fe2kxt/sbZfC2RcDataEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2RcDmaFormat.hx>
#include <soc/sbx/fe2kxt/sbZfC2RcDmaFormatConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2RcResultResolverBank0Entry.hx>
#include <soc/sbx/fe2kxt/sbZfC2RcResultResolverBank0EntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2RcResultResolverEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2RcResultResolverEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2RcReusltResolverEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2RcReusltResolverEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2RceInstruction.hx>
#include <soc/sbx/fe2kxt/sbZfC2RceInstructionConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2RcePattern.hx>
#include <soc/sbx/fe2kxt/sbZfC2RcePatternConsole.hx>

#else

#include <soc/sbx/fe2k/sbZfFe2000RcSbPattern.hx>
#include <soc/sbx/fe2k/sbZfFe2000RcSbPatternConsole.hx>
#include <soc/sbx/fe2k/sbZfFe2000RcDmaFormat.hx>
#include <soc/sbx/fe2k/sbZfFe2000RcDmaFormatConsole.hx>

#endif /* defined C2_RCE */

#endif /* BCM_SBX_SUPPORT */
#endif
