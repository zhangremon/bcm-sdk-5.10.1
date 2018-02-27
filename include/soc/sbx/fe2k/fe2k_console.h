/*
 * $Id: fe2k_console.h 1.2 Broadcom SDK $
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

#ifndef _SOC_SBX_FE2K_CONSOLE_H
#define _SOC_SBX_FE2K_CONSOLE_H

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


#endif /* _SOC_SBX_FE2K_CONSOLE_H */
