/*
 * $Id: sbZfCaIncludes.h 1.3 Broadcom SDK $
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

#ifndef _SOC_SBX_FE2K_SB_ZF_CA_INCLUDES_H_
#define _SOC_SBX_FE2K_SB_ZF_CA_INCLUDES_H_

#include <soc/sbx/fe2k/sbZfCaCmRangeTracker.hx>
#include <soc/sbx/fe2k/sbZfCaCmTurboCounter.hx>
#include <soc/sbx/fe2k/sbZfCaDiagUcodeCtl.hx>
#include <soc/sbx/fe2k/sbZfCaDiagUcodeShared.hx>
#include <soc/sbx/fe2k/sbZfCaIntfPort.hx>
#include <soc/sbx/fe2k/sbZfCaLrInstructionAddressFormat.hx>
#include <soc/sbx/fe2k/sbZfCaLrLrpInstructionMemoryEntry.hx>
#include <soc/sbx/fe2k/sbZfCaLrpDescriptor.hx>
#include <soc/sbx/fe2k/sbZfCaMmClientVirtualAddress.hx>
#include <soc/sbx/fe2k/sbZfCaMmDataWith36BitsNoProtection.hx>
#include <soc/sbx/fe2k/sbZfCaMmInternal0MemoryEntry.hx>
#include <soc/sbx/fe2k/sbZfCaMmInternal0SegmentConfigEntry.hx>
#include <soc/sbx/fe2k/sbZfCaMmInternal1MemoryEntry.hx>
#include <soc/sbx/fe2k/sbZfCaMmInternal1SegmentConfigEntry.hx>
#include <soc/sbx/fe2k/sbZfCaMmInternalMemory32BitsLayout.hx>
#include <soc/sbx/fe2k/sbZfCaMmInternalMemorySegmentConfigData.hx>
#include <soc/sbx/fe2k/sbZfCaMmInternalMemoryVirtualAddress.hx>
#include <soc/sbx/fe2k/sbZfCaMmNarrowPort0MemoryEntry.hx>
#include <soc/sbx/fe2k/sbZfCaMmNarrowPort0SegmentConfigEntry.hx>
#include <soc/sbx/fe2k/sbZfCaMmNarrowPort1MemoryEntry.hx>
#include <soc/sbx/fe2k/sbZfCaMmNarrowPort1SegmentConfigEntry.hx>
#include <soc/sbx/fe2k/sbZfCaMmNarrowPortMemorySegmentConfigData.hx>
#include <soc/sbx/fe2k/sbZfCaMmNarrowPortMemoryVirtualAddress.hx>
#include <soc/sbx/fe2k/sbZfCaMmWidePortMemory36BitsNoProtection.hx>
#include <soc/sbx/fe2k/sbZfCaMmWidePortMemoryEntry.hx>
#include <soc/sbx/fe2k/sbZfCaMmWidePortMemorySegmentConfigData.hx>
#include <soc/sbx/fe2k/sbZfCaMmWidePortMemoryVirtualAddress.hx>
#include <soc/sbx/fe2k/sbZfCaMmWidePortSegmentConfigEntry.hx>
#include <soc/sbx/fe2k/sbZfCaPbCounterEntry.hx>
#include <soc/sbx/fe2k/sbZfCaPbDataEntry.hx>
#include <soc/sbx/fe2k/sbZfCaPdHeaderConfig.hx>
#include <soc/sbx/fe2k/sbZfCaPdLrpOutputHeaderCopyBufferEntry.hx>
#include <soc/sbx/fe2k/sbZfCaPdPdOutputHeaderCopyBufferEntry.hx>
#include <soc/sbx/fe2k/sbZfCaPeDiagPV2Etc.hx>
#include <soc/sbx/fe2k/sbZfCaPedDescriptor.hx>
#include <soc/sbx/fe2k/sbZfCaPmCheckerTimerCfgEncoding.hx>
#include <soc/sbx/fe2k/sbZfCaPmGeneratorSemaphoreCfgEncoding.hx>
#include <soc/sbx/fe2k/sbZfCaPmLrMeterRequest.hx>
#include <soc/sbx/fe2k/sbZfCaPmLrMeterResult.hx>
#include <soc/sbx/fe2k/sbZfCaPmProfileMemoryEntry.hx>
#include <soc/sbx/fe2k/sbZfCaPmSequenceGenState.hx>
#include <soc/sbx/fe2k/sbZfCaPmTimerCheckerState.hx>
#include <soc/sbx/fe2k/sbZfCaPpAggregateHashBitConfigEntry.hx>
#include <soc/sbx/fe2k/sbZfCaPpAggregateHashByteConfigEntry.hx>
#include <soc/sbx/fe2k/sbZfCaPpAggregateHashConfigEntry.hx>
#include <soc/sbx/fe2k/sbZfCaPpAggregateHashFormat.hx>
#include <soc/sbx/fe2k/sbZfCaPpByteHashConverter.hx>
#include <soc/sbx/fe2k/sbZfCaPpCamConfigurationEntry.hx>
#include <soc/sbx/fe2k/sbZfCaPpCamConfigurationInstance0Entry.hx>
#include <soc/sbx/fe2k/sbZfCaPpCamQueryFormat.hx>
#include <soc/sbx/fe2k/sbZfCaPpCamRamConfigurationEntry.hx>
#include <soc/sbx/fe2k/sbZfCaPpCamRamConfigurationInstance0Entry.hx>
#include <soc/sbx/fe2k/sbZfCaPpCamStateFormat.hx>
#include <soc/sbx/fe2k/sbZfCaPpCamStateRawFormat.hx>
#include <soc/sbx/fe2k/sbZfCaPpDmacFormat.hx>
#include <soc/sbx/fe2k/sbZfCaPpDmacMaskFormat.hx>
#include <soc/sbx/fe2k/sbZfCaPpFilterDataOverlayFormat.hx>
#include <soc/sbx/fe2k/sbZfCaPpFinalStreamSelectionAddress.hx>
#include <soc/sbx/fe2k/sbZfCaPpFinalStreamSelectionEntry.hx>
#include <soc/sbx/fe2k/sbZfCaPpHashTemplateEntry.hx>
#include <soc/sbx/fe2k/sbZfCaPpHeaderRecordSizeEntry.hx>
#include <soc/sbx/fe2k/sbZfCaPpInitialQueueStateEntry.hx>
#include <soc/sbx/fe2k/sbZfCaPpLocalStationMatchAddress.hx>
#include <soc/sbx/fe2k/sbZfCaPpPpOutHeaderCopyEntry.hx>
#include <soc/sbx/fe2k/sbZfCaPpQueueConfigEntry.hx>
#include <soc/sbx/fe2k/sbZfCaPpQueuePriorityGroupEntry.hx>
#include <soc/sbx/fe2k/sbZfCaPpRxPortDataEntry.hx>
#include <soc/sbx/fe2k/sbZfCaPpRxPortDataPpDescriptorFormat.hx>
#include <soc/sbx/fe2k/sbZfCaPpRxPortMaskEntry.hx>
#include <soc/sbx/fe2k/sbZfCaPpStreamSelectionStateAddress.hx>
#include <soc/sbx/fe2k/sbZfCaPpStreamSelectionStateEntry.hx>
#include <soc/sbx/fe2k/sbZfCaPpSwtoPpInterface.hx>
#include <soc/sbx/fe2k/sbZfCaPpeDescriptor.hx>
#include <soc/sbx/fe2k/sbZfCaPrPortStateEntry.hx>
#include <soc/sbx/fe2k/sbZfCaPrPortToQueueAg0Entry.hx>
#include <soc/sbx/fe2k/sbZfCaPrPortToQueueAg1Entry.hx>
#include <soc/sbx/fe2k/sbZfCaPrPortToQueuePEDEntry.hx>
#include <soc/sbx/fe2k/sbZfCaPrPortToQueuePciEntry.hx>
#include <soc/sbx/fe2k/sbZfCaPrPortToQueueSr0Entry.hx>
#include <soc/sbx/fe2k/sbZfCaPrPortToQueueSr1Entry.hx>
#include <soc/sbx/fe2k/sbZfCaPrPortToQueueXg0Entry.hx>
#include <soc/sbx/fe2k/sbZfCaPrPortToQueueXg1Entry.hx>
#include <soc/sbx/fe2k/sbZfCaPrTagInfo.hx>
#include <soc/sbx/fe2k/sbZfCaPtMirrorIndexEntry.hx>
#include <soc/sbx/fe2k/sbZfCaPtPortToQueueEntry.hx>
#include <soc/sbx/fe2k/sbZfCaPtPte0MirrorPortStateEntry.hx>
#include <soc/sbx/fe2k/sbZfCaPtPte0PortStateEntry.hx>
#include <soc/sbx/fe2k/sbZfCaPtPte1MirrorPortStateEntry.hx>
#include <soc/sbx/fe2k/sbZfCaPtPte1PortStateEntry.hx>
#include <soc/sbx/fe2k/sbZfCaPtPte2MirrorPortStateEntry.hx>
#include <soc/sbx/fe2k/sbZfCaPtPte2PortStateEntry.hx>
#include <soc/sbx/fe2k/sbZfCaPtPte3MirrorPortStateEntry.hx>
#include <soc/sbx/fe2k/sbZfCaPtPte3PortStateEntry.hx>
#include <soc/sbx/fe2k/sbZfCaPtPte4MirrorPortStateEntry.hx>
#include <soc/sbx/fe2k/sbZfCaPtPte4PortStateEntry.hx>
#include <soc/sbx/fe2k/sbZfCaPtPte5MirrorPortStateEntry.hx>
#include <soc/sbx/fe2k/sbZfCaPtPte5PortStateEntry.hx>
#include <soc/sbx/fe2k/sbZfCaPtPte6MirrorPortStateEntry.hx>
#include <soc/sbx/fe2k/sbZfCaPtPte6PortStateEntry.hx>
#include <soc/sbx/fe2k/sbZfCaPtQueueArbParamEntry.hx>
#include <soc/sbx/fe2k/sbZfCaPtQueueToPortEntry.hx>
#include <soc/sbx/fe2k/sbZfCaQmBufferState0Entry.hx>
#include <soc/sbx/fe2k/sbZfCaQmBufferState1Entry.hx>
#include <soc/sbx/fe2k/sbZfCaQmBufferState1NonHeaderEntry.hx>
#include <soc/sbx/fe2k/sbZfCaQmBufferState2Entry.hx>
#include <soc/sbx/fe2k/sbZfCaQmBufferState2NonHeaderEntry.hx>
#include <soc/sbx/fe2k/sbZfCaQmBufferState3Entry.hx>
#include <soc/sbx/fe2k/sbZfCaQmDequeueState0Entry.hx>
#include <soc/sbx/fe2k/sbZfCaQmDequeueState1Entry.hx>
#include <soc/sbx/fe2k/sbZfCaQmFreePageFifoEntry.hx>
#include <soc/sbx/fe2k/sbZfCaQmNextBufferEntry.hx>
#include <soc/sbx/fe2k/sbZfCaQmNextPageEntry.hx>
#include <soc/sbx/fe2k/sbZfCaQmQueueConfigEntry.hx>
#include <soc/sbx/fe2k/sbZfCaQmQueueConfigFlowControlPort.hx>
#include <soc/sbx/fe2k/sbZfCaQmQueueCountersEntry.hx>
#include <soc/sbx/fe2k/sbZfCaQmQueueHeadPtrEntry.hx>
#include <soc/sbx/fe2k/sbZfCaQmQueueState0EnEntry.hx>
#include <soc/sbx/fe2k/sbZfCaQmQueueState0Entry.hx>
#include <soc/sbx/fe2k/sbZfCaQmQueueState1Entry.hx>
#include <soc/sbx/fe2k/sbZfCaQmReplicationStateEntry.hx>
#include <soc/sbx/fe2k/sbZfCaRcDataEntry.hx>
#include <soc/sbx/fe2k/sbZfCaRcDmaFormat.hx>
#include <soc/sbx/fe2k/sbZfCaRceInstruction.hx>
#include <soc/sbx/fe2k/sbZfCaRcePattern.hx>
#include <soc/sbx/fe2k/sbZfCaSrCounterEntry.hx>
#include <soc/sbx/fe2k/sbZfCaStCounterEntry.hx>
#include <soc/sbx/fe2k/sbZfCaSwsDescriptor.hx>
#include <soc/sbx/fe2k/sbZfCaPmCounterMemoryEntry.hx>









#include <soc/sbx/fe2k/sbZfCaCmRangeTrackerConsole.hx>
#include <soc/sbx/fe2k/sbZfCaCmTurboCounterConsole.hx>
#include <soc/sbx/fe2k/sbZfCaDiagUcodeCtlConsole.hx>
#include <soc/sbx/fe2k/sbZfCaDiagUcodeSharedConsole.hx>
#include <soc/sbx/fe2k/sbZfCaIntfPortConsole.hx>
#include <soc/sbx/fe2k/sbZfCaLrInstructionAddressFormatConsole.hx>
#include <soc/sbx/fe2k/sbZfCaLrLrpInstructionMemoryEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaLrpDescriptorConsole.hx>
#include <soc/sbx/fe2k/sbZfCaMmClientVirtualAddressConsole.hx>
#include <soc/sbx/fe2k/sbZfCaMmDataWith36BitsNoProtectionConsole.hx>
#include <soc/sbx/fe2k/sbZfCaMmInternal0MemoryEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaMmInternal0SegmentConfigEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaMmInternal1MemoryEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaMmInternal1SegmentConfigEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaMmInternalMemory32BitsLayoutConsole.hx>
#include <soc/sbx/fe2k/sbZfCaMmInternalMemorySegmentConfigDataConsole.hx>
#include <soc/sbx/fe2k/sbZfCaMmInternalMemoryVirtualAddressConsole.hx>
#include <soc/sbx/fe2k/sbZfCaMmNarrowPort0MemoryEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaMmNarrowPort0SegmentConfigEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaMmNarrowPort1MemoryEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaMmNarrowPort1SegmentConfigEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaMmNarrowPortMemorySegmentConfigDataConsole.hx>
#include <soc/sbx/fe2k/sbZfCaMmNarrowPortMemoryVirtualAddressConsole.hx>
#include <soc/sbx/fe2k/sbZfCaMmWidePortMemory36BitsNoProtectionConsole.hx>
#include <soc/sbx/fe2k/sbZfCaMmWidePortMemoryEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaMmWidePortMemorySegmentConfigDataConsole.hx>
#include <soc/sbx/fe2k/sbZfCaMmWidePortMemoryVirtualAddressConsole.hx>
#include <soc/sbx/fe2k/sbZfCaMmWidePortSegmentConfigEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPbCounterEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPbDataEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPdHeaderConfigConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPdLrpOutputHeaderCopyBufferEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPdPdOutputHeaderCopyBufferEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPeDiagPV2EtcConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPedDescriptorConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPmCheckerTimerCfgEncodingConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPmGeneratorSemaphoreCfgEncodingConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPmLrMeterRequestConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPmLrMeterResultConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPmProfileMemoryEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPmSequenceGenStateConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPmTimerCheckerStateConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPpAggregateHashBitConfigEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPpAggregateHashByteConfigEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPpAggregateHashConfigEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPpAggregateHashFormatConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPpByteHashConverterConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPpCamConfigurationEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPpCamConfigurationInstance0EntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPpCamQueryFormatConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPpCamRamConfigurationEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPpCamRamConfigurationInstance0EntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPpCamStateFormatConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPpCamStateRawFormatConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPpDmacFormatConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPpDmacMaskFormatConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPpFilterDataOverlayFormatConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPpFinalStreamSelectionAddressConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPpFinalStreamSelectionEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPpHashTemplateEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPpHeaderRecordSizeEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPpInitialQueueStateEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPpLocalStationMatchAddressConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPpPpOutHeaderCopyEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPpQueueConfigEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPpQueuePriorityGroupEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPpRxPortDataEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPpRxPortDataPpDescriptorFormatConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPpRxPortMaskEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPpStreamSelectionStateAddressConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPpStreamSelectionStateEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPpSwtoPpInterfaceConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPpeDescriptorConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPrPortStateEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPrPortToQueueAg0EntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPrPortToQueueAg1EntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPrPortToQueuePEDEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPrPortToQueuePciEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPrPortToQueueSr0EntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPrPortToQueueSr1EntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPrPortToQueueXg0EntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPrPortToQueueXg1EntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPrTagInfoConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPtMirrorIndexEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPtPortToQueueEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPtPte0MirrorPortStateEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPtPte0PortStateEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPtPte1MirrorPortStateEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPtPte1PortStateEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPtPte2MirrorPortStateEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPtPte2PortStateEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPtPte3MirrorPortStateEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPtPte3PortStateEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPtPte4MirrorPortStateEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPtPte4PortStateEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPtPte5MirrorPortStateEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPtPte5PortStateEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPtPte6MirrorPortStateEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPtPte6PortStateEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPtQueueArbParamEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPtQueueToPortEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaQmBufferState0EntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaQmBufferState1EntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaQmBufferState1NonHeaderEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaQmBufferState2EntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaQmBufferState2NonHeaderEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaQmBufferState3EntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaQmDequeueState0EntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaQmDequeueState1EntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaQmFreePageFifoEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaQmNextBufferEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaQmNextPageEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaQmQueueConfigEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaQmQueueConfigFlowControlPortConsole.hx>
#include <soc/sbx/fe2k/sbZfCaQmQueueCountersEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaQmQueueHeadPtrEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaQmQueueState0EnEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaQmQueueState0EntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaQmQueueState1EntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaQmReplicationStateEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaRcDataEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaRcDmaFormatConsole.hx>
#include <soc/sbx/fe2k/sbZfCaRceInstructionConsole.hx>
#include <soc/sbx/fe2k/sbZfCaRcePatternConsole.hx>
#include <soc/sbx/fe2k/sbZfCaSrCounterEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaStCounterEntryConsole.hx>
#include <soc/sbx/fe2k/sbZfCaSwsDescriptorConsole.hx>
#include <soc/sbx/fe2k/sbZfCaUcodeAccessDescriptorConsole.hx>
#include <soc/sbx/fe2k/sbZfCaPmCounterMemoryEntryConsole.hx>

#endif /* _SOC_SBX_FE2K_SB_ZF_CA_INCLUDES_H_ */
