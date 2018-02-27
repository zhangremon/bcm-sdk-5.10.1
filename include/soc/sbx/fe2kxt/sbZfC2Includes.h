/*
 * $Id: sbZfC2Includes.h 1.3 Broadcom SDK $
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
 * This file contains aggregated definitions for imfdriver indirect mem access
 */

#ifndef _SOC_SBX_FE2K_SB_ZF_C2_INCLUDES_H_
#define _SOC_SBX_FE2K_SB_ZF_C2_INCLUDES_H_

#include <soc/sbx/fe2kxt/sbZfC2CmRangeTracker.hx>
#include <soc/sbx/fe2kxt/sbZfC2CmRangeTrackerConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2CmTurboCounter.hx>
#include <soc/sbx/fe2kxt/sbZfC2CmTurboCounterConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2LrInstructionAddressFormat.hx>
#include <soc/sbx/fe2kxt/sbZfC2LrInstructionAddressFormatConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2LrLrpInstructionMemoryBank0Entry.hx>
#include <soc/sbx/fe2kxt/sbZfC2LrLrpInstructionMemoryBank0EntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2LrLrpInstructionMemoryBank1Entry.hx>
#include <soc/sbx/fe2kxt/sbZfC2LrLrpInstructionMemoryBank1EntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2LrLrpInstructionMemoryEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2LrLrpInstructionMemoryEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2LrpDescriptor.hx>
#include <soc/sbx/fe2kxt/sbZfC2LrpDescriptorConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmClientVirtualAddress.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmClientVirtualAddressConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmDataWith36BitsNoProtection.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmDataWith36BitsNoProtectionConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmInternal0MemoryEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmInternal0MemoryEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmInternal0SegmentConfigEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmInternal0SegmentConfigEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmInternal1MemoryEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmInternal1MemoryEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmInternal1SegmentConfigEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmInternal1SegmentConfigEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmInternalMemory32BitsLayout.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmInternalMemory32BitsLayoutConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmInternalMemory35BitsLayout.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmInternalMemory35BitsLayoutConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmInternalMemorySegmentConfigData.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmInternalMemorySegmentConfigDataConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmInternalMemoryVirtualAddress.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmInternalMemoryVirtualAddressConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmNarrowPort0MemoryEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmNarrowPort0MemoryEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmNarrowPort0SegmentConfigEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmNarrowPort0SegmentConfigEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmNarrowPort1MemoryEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmNarrowPort1MemoryEntry35bit.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmNarrowPort1MemoryEntry35bitConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmNarrowPort1MemoryEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmNarrowPort1SegmentConfigEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmNarrowPort1SegmentConfigEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmNarrowPortMemorySegmentConfigData.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmNarrowPortMemorySegmentConfigDataConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmNarrowPortMemoryVirtualAddress.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmNarrowPortMemoryVirtualAddressConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmWidePortMemory36BitsNoProtection.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmWidePortMemory36BitsNoProtectionConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmWidePortMemoryEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmWidePortMemoryEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmWidePortMemorySegmentConfigData.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmWidePortMemorySegmentConfigDataConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmWidePortMemoryVirtualAddress.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmWidePortMemoryVirtualAddressConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmWidePortSegmentConfigEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2MmWidePortSegmentConfigEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PbCounterEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2PbCounterEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PbDataEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2PbDataEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PdHeaderConfig.hx>
#include <soc/sbx/fe2kxt/sbZfC2PdHeaderConfigConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PdLrpOutputHeaderCopyBufferEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2PdLrpOutputHeaderCopyBufferEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PdPdOutputHeaderCopyBufferEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2PdPdOutputHeaderCopyBufferEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PedDescriptor.hx>
#include <soc/sbx/fe2kxt/sbZfC2PedDescriptorConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PmBurstSize.hx>
#include <soc/sbx/fe2kxt/sbZfC2PmBurstSizeConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PmCheckerTimerCfgEncoding.hx>
#include <soc/sbx/fe2kxt/sbZfC2PmCheckerTimerCfgEncodingConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PmGeneratorSemaphoreCfgEncoding.hx>
#include <soc/sbx/fe2kxt/sbZfC2PmGeneratorSemaphoreCfgEncodingConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PmLrMeterRequest.hx>
#include <soc/sbx/fe2kxt/sbZfC2PmLrMeterRequestConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PmLrMeterResult.hx>
#include <soc/sbx/fe2kxt/sbZfC2PmLrMeterResultConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PmProfileMemoryEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2PmProfileMemoryEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PmSequenceGenState.hx>
#include <soc/sbx/fe2kxt/sbZfC2PmSequenceGenStateConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PmTimerCheckerState.hx>
#include <soc/sbx/fe2kxt/sbZfC2PmTimerCheckerStateConsole.hx>
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
#include <soc/sbx/fe2kxt/sbZfC2PpCamConfigurationInstance0Entry.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpCamConfigurationInstance0EntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpCamQueryFormat.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpCamQueryFormatConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpCamRamConfigurationInstance0Entry.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpCamRamConfigurationInstance0EntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpCamStateFormat.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpCamStateFormatConsole.hx>
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
#include <soc/sbx/fe2kxt/sbZfC2PpeDescriptor.hx>
#include <soc/sbx/fe2kxt/sbZfC2PpeDescriptorConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PrCcCamRamEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2PrCcCamRamEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PrCcCamRamLastEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2PrCcCamRamLastEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PrCcCamRamNonLastEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2PrCcCamRamNonLastEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PrCcQueueRamEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2PrCcQueueRamEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PrCcTcamConfigEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2PrCcTcamConfigEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PrCcTcamConfigEntryFirst.hx>
#include <soc/sbx/fe2kxt/sbZfC2PrCcTcamConfigEntryFirstConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PrCcTcamConfigEntryNonFirst.hx>
#include <soc/sbx/fe2kxt/sbZfC2PrCcTcamConfigEntryNonFirstConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PrPortStateEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2PrPortStateEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PrPortToQueueAg0Entry.hx>
#include <soc/sbx/fe2kxt/sbZfC2PrPortToQueueAg0EntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PrPortToQueueAg1Entry.hx>
#include <soc/sbx/fe2kxt/sbZfC2PrPortToQueueAg1EntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PrPortToQueuePEDEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2PrPortToQueuePEDEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PrPortToQueuePciEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2PrPortToQueuePciEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PrPortToQueueSr0Entry.hx>
#include <soc/sbx/fe2kxt/sbZfC2PrPortToQueueSr0EntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PrPortToQueueSr1Entry.hx>
#include <soc/sbx/fe2kxt/sbZfC2PrPortToQueueSr1EntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PrPortToQueueXg0Entry.hx>
#include <soc/sbx/fe2kxt/sbZfC2PrPortToQueueXg0EntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PrPortToQueueXg1Entry.hx>
#include <soc/sbx/fe2kxt/sbZfC2PrPortToQueueXg1EntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PrTagInfo.hx>
#include <soc/sbx/fe2kxt/sbZfC2PrTagInfoConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PtMirrorIndexEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2PtMirrorIndexEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PtMirrorPortStateEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2PtMirrorPortStateEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PtPortStateEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2PtPortStateEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PtPortToQueueEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2PtPortToQueueEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PtQueueArbParamEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2PtQueueArbParamEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PtQueueToChannelEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2PtQueueToChannelEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2PtQueueToPortEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2PtQueueToPortEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2QmBufferState0Entry.hx>
#include <soc/sbx/fe2kxt/sbZfC2QmBufferState0EntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2QmBufferState1Entry.hx>
#include <soc/sbx/fe2kxt/sbZfC2QmBufferState1EntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2QmBufferState1NonHeaderEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2QmBufferState1NonHeaderEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2QmBufferState2Entry.hx>
#include <soc/sbx/fe2kxt/sbZfC2QmBufferState2EntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2QmBufferState2NonHeaderEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2QmBufferState2NonHeaderEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2QmBufferState3Entry.hx>
#include <soc/sbx/fe2kxt/sbZfC2QmBufferState3EntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2QmDequeueState0Entry.hx>
#include <soc/sbx/fe2kxt/sbZfC2QmDequeueState0EntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2QmDequeueState1Entry.hx>
#include <soc/sbx/fe2kxt/sbZfC2QmDequeueState1EntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2QmFreePageFifoEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2QmFreePageFifoEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2QmNextBufferEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2QmNextBufferEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2QmNextPageEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2QmNextPageEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2QmQueueConfigEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2QmQueueConfigEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2QmQueueConfigFlowControlPort.hx>
#include <soc/sbx/fe2kxt/sbZfC2QmQueueConfigFlowControlPortConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2QmQueueCountersEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2QmQueueCountersEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2QmQueueHeadPtrEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2QmQueueHeadPtrEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2QmQueueState0EnEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2QmQueueState0EnEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2QmQueueState0Entry.hx>
#include <soc/sbx/fe2kxt/sbZfC2QmQueueState0EntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2QmQueueState1Entry.hx>
#include <soc/sbx/fe2kxt/sbZfC2QmQueueState1EntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2QmReplicationStateEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2QmReplicationStateEntryConsole.hx>
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
#include <soc/sbx/fe2kxt/sbZfC2SrCounterEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2SrCounterEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2StCounterEntry.hx>
#include <soc/sbx/fe2kxt/sbZfC2StCounterEntryConsole.hx>
#include <soc/sbx/fe2kxt/sbZfC2UcodeAccessDescriptor.hx>
#include <soc/sbx/fe2kxt/sbZfC2UcodeAccessDescriptorConsole.hx>

#endif
