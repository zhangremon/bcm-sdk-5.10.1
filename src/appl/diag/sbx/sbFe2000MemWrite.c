
/*
 * $Id: sbFe2000MemWrite.c 1.9 Broadcom SDK $
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
 * File:        sbFe2000MemWrites.c
 * Purpose:     sbx commands to write fe2000 indirect mems
 * Requires:
 */

#ifdef BCM_FE2000_SUPPORT

#include <appl/diag/shell.h>
#include <appl/diag/system.h>

#include <soc/defs.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_user.h>
#include <soc/sbx/fe2k/fe2k.h>
 
#include <soc/sbx/fe2k/sbZfCaIncludes.h>
#include <soc/sbx/fe2k/sbFe2000ImfDriver.h>

extern int sbFe2000WhichMem(char *memname);
extern int sbFe2000MemMax(int memindex);
extern  void sbFe2000ShowMemNames(void);
extern void sbSwapWords(uint *pdata, int nwords);

char *memberList[] =
{
"PrPortToQueueAg0.queue",
"PrPortToQueueAg1.queue",
"PrPortToQueuePci.queue",
"PrPortToQueueSr0.queue",
"PrPortToQueueSr1.queue",
"PrPortToQueueXg0.queue",
"PrPortToQueueXg1.queue",
"PtPortToQueue.Queue",
"PdLrpOutputHeaderCopyBuffer.HeaderCopy7",
"PdLrpOutputHeaderCopyBuffer.HeaderCopy6",
"PdLrpOutputHeaderCopyBuffer.HeaderCopy5",
"PdLrpOutputHeaderCopyBuffer.HeaderCopy4",
"PdLrpOutputHeaderCopyBuffer.HeaderCopy3",
"PdLrpOutputHeaderCopyBuffer.HeaderCopy2",
"PdLrpOutputHeaderCopyBuffer.HeaderCopy1",
"PdLrpOutputHeaderCopyBuffer.HeaderCopy0",
"PdPdOutputHeaderCopyBuffer.HeaderCopy7",
"PdPdOutputHeaderCopyBuffer.HeaderCopy6",
"PdPdOutputHeaderCopyBuffer.HeaderCopy5",
"PdPdOutputHeaderCopyBuffer.HeaderCopy4",
"PdPdOutputHeaderCopyBuffer.HeaderCopy3",
"PdPdOutputHeaderCopyBuffer.HeaderCopy2",
"PdPdOutputHeaderCopyBuffer.HeaderCopy1",
"PdPdOutputHeaderCopyBuffer.HeaderCopy0",
"LrLrpInstructionMemory.Thread",
"LrLrpInstructionMemory.Opcode",
"LrLrpInstructionMemory.Ra",
"LrLrpInstructionMemory.Rb",
"LrLrpInstructionMemory.Target",
"LrLrpInstructionMemory.SourceA",
"LrLrpInstructionMemory.SourceB",
"LrLrpInstructionMemory.Reserved",
"QmBufferState0.IsHeader",
"QmBufferState0.nPages",
"QmBufferState1.Drop",
"QmBufferState1.CrcMode",
"QmBufferState1.HeaderLength",
"QmBufferState1.ContinueByte",
"QmBufferState1.PacketLength",
"QmBufferState2.MirrorIndex",
"QmBufferState2.SourceQueue",
"QmBufferState2NonHeader.CopyCount",
"QmBufferState3.LastCopy",
"QmBufferState3.RefCntNew",
"QmBufferState3.RefCnt",
"QmDequeueState0.Drop",
"QmDequeueState0.RemainingPagesHi",
"QmDequeueState0.RemainingPagesLo",
"QmDequeueState0.ReplicantNumber",
"QmDequeueState0.ReplicantLength",
"QmDequeueState1.PrevEop",
"QmDequeueState1.NextPage",
"QmFreePageFifo.NextPage",
"QmNextBuffer.NextBuffer",
"QmNextPage.NextPage",
"QmQueueConfig.EnforcePoliceMarkings",
"QmQueueConfig.DropThresh2",
"QmQueueConfig.DropThresh1",
"QmQueueConfig.FlowControlThresh2",
"QmQueueConfig.FlowControlThresh1",
"QmQueueConfig.FlowControlEnable",
"QmQueueConfig.FlowControlTarget",
"QmQueueConfig.FlowControlPort",
"QmQueueConfig.MaxPages",
"QmQueueConfig.MinPages",
"QmQueueConfig.Enable",
"QmQueueCounters.PacketCount",
"QmQueueCounters.ByteCount",
"QmQueueHeadPtr.Page",
"QmQueueState0En.Enable",
"QmQueueState0.TailPtr",
"QmQueueState0.DepthInPages",
"QmQueueState0.Empty",
"QmQueueState0.Enable",
"QmQueueState1.Drop",
"QmQueueState1.Overflow",
"QmQueueState1.PagesAllocated",
"QmReplicationState.ReplState",
"PmProfileMemory.Type2",
"PmProfileMemory.DropOnRed",
"PmProfileMemory.ExcessBucketNoDecrement",
"PmProfileMemory.CommittedBucketNoDecrement",
"PmProfileMemory.LengthShift",
"PmProfileMemory.ExcessInformationRate",
"PmProfileMemory.CommittedInformationRate",
"PmProfileMemory.CouplingFlag",
"PmProfileMemory.RfcMode",
"PmProfileMemory.ColorBlind",
"PmProfileMemory.CommitedBucketSize",
"PmProfileMemory.ExcessBurstSize",
"PmProfileMemory.CommittedBurstSize",
"Sr0Counter.PacketCount",
"Sr0Counter.ByteCount",
"Sr1Counter.PacketCount",
"Sr1Counter.ByteCount",
"PpAggregateHashBitConfig.Data9",
"PpAggregateHashBitConfig.Data8",
"PpAggregateHashBitConfig.Data7",
"PpAggregateHashBitConfig.Data6",
"PpAggregateHashBitConfig.Data5",
"PpAggregateHashBitConfig.Data4",
"PpAggregateHashBitConfig.Data3",
"PpAggregateHashBitConfig.Data2",
"PpAggregateHashBitConfig.Data1",
"PpAggregateHashBitConfig.Data0",
"PpAggregateHashByteConfig.Data1",
"PpAggregateHashByteConfig.Data0",
"LrInstructionAddressFormat.StreamId",
"LrInstructionAddressFormat.InstructionIndex",
"MmClientVirtualAddress.Segment",
"MmClientVirtualAddress.Address",
"MmDataWith36BitsNoProtection.ProtectionBits",
"MmDataWith36BitsNoProtection.Data",
"Mm0Internal0Memory.Data1",
"Mm0Internal0Memory.Data0",
"Mm0Internal0SegmentConfig.Internal0SegmentConfig1",
"Mm0Internal0SegmentConfig.Internal0SegmentConfig0",
"Mm0Internal1Memory.Data0",
"Mm0Internal1Memory.Data1",
"Mm0Internal1SegmentConfig.Internal1SegmentConfig1",
"Mm0Internal1SegmentConfig.Internal1SegmentConfig0",
"Mm0Internal0Memory32BitsLayout.ProtectionBits0",
"Mm0Internal0Memory32BitsLayout.Data0",
"Mm0Internal0Memory32BitsLayout.ProtectionBits1",
"Mm0Internal0Memory32BitsLayout.Data1",
"Mm0Internal0Memory.Resv0",
"Mm0Internal0Memory.WriteAccess",
"Mm0Internal0Memory.ReadAccess",
"Mm0Internal0Memory.DataIntegrity",
"Mm0Internal0Memory.TrapNullPointer",
"Mm0Internal0Memory.Wrap",
"Mm0Internal0Memory.LimitMask",
"Mm0Internal0Memory.BaseAddress",
"Mm0NarrowPort0Memory.Data1",
"Mm0NarrowPort0Memory.Data0",
"Mm0NarrowPort0SegmentConfig.NarrowPort0SegmentConfig1",
"Mm0NarrowPort0SegmentConfig.NarrowPort0SegmentConfig0",
"Mm0NarrowPort1Memory.Data1",
"Mm0NarrowPort1Memory.Data0",
"Mm0NarrowPort1SegmentConfig.NarrowPort1SegmentConfig1",
"Mm0NarrowPort1SegmentConfig.NarrowPort1SegmentConfig0",
"Mm0NarrowPort0Memory.Resv0",
"Mm0NarrowPort0Memory.WriteAccess",
"Mm0NarrowPort0Memory.ReadAccess",
"Mm0NarrowPort0Memory.DataIntegrity",
"Mm0NarrowPort0Memory.TrapNullPointer",
"Mm0NarrowPort0Memory.Wrap",
"Mm0NarrowPort0Memory.LimitMask",
"Mm0NarrowPort0Memory.BaseAddress",
"Mm0WidePortMemory.Data1",
"Mm0WidePortMemory.Data0",
"Mm0WidePortMemory.Resv0",
"Mm0WidePortMemory.WriteAccess",
"Mm0WidePortMemory.ReadAccess",
"Mm0WidePortMemory.DataIntegrity",
"Mm0WidePortMemory.TrapNullPointer",
"Mm0WidePortMemory.Wrap",
"Mm0WidePortMemory.LimitMask",
"Mm0WidePortMemory.BaseAddress",
"Mm0WidePortMemoryVirtualAddress.Resv0",
"Mm0WidePortMemoryVirtualAddress.PhysicalAddress",
"Mm0WidePortMemoryVirtualAddress.Segment",
"Mm0WidePortMemoryVirtualAddress.Address",
"Mm0WidePortSegmentConfig.WidePortSegmentConfig1",
"Mm0WidePortSegmentConfig.WidePortSegmentConfig0",
"PbCounter.Data1",
"PbCounter.Data0",
"Pb0Data.Data2",
"Pb0Data.Data1",
"Pb1Data.Data2",
"Pb1Data.Data1",
"PbData.Data0",
"PpCamConfigurationInstance0.Mask5",
"PpCamConfigurationInstance0.Mask4",
"PpCamConfigurationInstance0.Mask3",
"PpCamConfigurationInstance0.Mask2",
"PpCamConfigurationInstance0.Mask1",
"PpCamConfigurationInstance0.Mask0",
"PpCamConfigurationInstance0.Data5",
"PpCamConfigurationInstance0.Data4",
"PpCamConfigurationInstance0.Data3",
"PpCamConfigurationInstance0.Data2",
"PpCamConfigurationInstance0.Data1",
"PpCamConfigurationInstance0.Data0",
"PpCamConfigurationInstance0.Valid",
"PpCamRamConfiguration.State",
"PpCamStateFormat.m_bEgress",
"PpDmacFormat.Dmac_47_16",
"PpDmacFormat.Dmac_15_0",
"PpDmacMaskFormat.DmacMask_47_32",
"PpDmacMaskFormat.DmacMask_31_0",
"PpFilterDataOverlayFormat.Fragment",
"PpFilterDataOverlayFormat.FilterId1",
"PpFinalStreamSelectionAddress.LocalStationID",
"PpHeaderRecordSize.RecordSize",
"PpInitialQueueState.HeaderShift",
"PpInitialQueueState.headertype",
"PpInitialQueueState.State",
"PpInitialQueueState.Variable",
"PpPpOutHeaderCopy.data7",
"PpPpOutHeaderCopy.data6",
"PpPpOutHeaderCopy.data5",
"PpPpOutHeaderCopy.data4",
"PpPpOutHeaderCopy.data3",
"PpPpOutHeaderCopy.data2",
"PpPpOutHeaderCopy.data1",
"PpPpOutHeaderCopy.data0",
"PpQueuePriorityGroup.Priority",
"PpQueuePriorityGroup.BatchGroup",
"PpRxPortData.Mask1",
"PpRxPortData.Mask0",
"PpRxPortData.Data1",
"PpRxPortData.Data0",
"PpSwtoPpInterface.QueueIndex",
"PpeDescriptor.Variable",
"PrPortState.m_nReserved",
"PtMirrorIndex.DropSrcPkt",
"PtMirrorIndex.HdrPresent",
"PtMirrorIndex.MirrorQueue",
"PtPte0MirrorPortState.HdrPresent",
"PtPte0MirrorPortState.PktLength",
"PtPte0MirrorPortState.NumPagesHi",
"PtPte0MirrorPortState.NumPagesLo",
"PtPte0MirrorPortState.Buffer",
"PtPte0MirrorPortState.SourceQueue",
"PtPte0MirrorPortState.Queue",
"PtPte0PortState.PktInProgress",
"PtPte0PortState.Mirror",
"PtPte0PortState.MirrorSrcDrop",
"PtPte0PortState.DoubleDequeued",
"PtPte0PortState.EopExtraBurst",
"PtPte0PortState.ResidueByte",
"PtPte0PortState.PktRequestState",
"PtPte0PortState.Line",
"PtPte0PortState.LastPage",
"PtPte0PortState.NextPageState",
"PtPte0PortState.NextPage",
"PtPte0PortState.NextLength",
"PtPte0PortState.Page",
"PtPte0PortState.SavedBytes",
"PtPte0PortState.RemainingBytes",
"PtPte0PortState.Eop",
"PtPte0PortState.Sop",
"PtPte0PortState.PktLength",
"PtPte0PortState.ContinueByte",
"PtPte0PortState.SourceQueue",
"PtPte1MirrorPortState.HdrPresent",
"PtPte1MirrorPortState.PktLength",
"PtPte1MirrorPortState.NumPagesHi",
"PtPte1MirrorPortState.NumPagesLo",
"PtPte1MirrorPortState.Buffer",
"PtPte1MirrorPortState.SourceQueue",
"PtPte1MirrorPortState.Queue",
"PtPte1PortState.PktInProgress",
"PtPte1PortState.Mirror",
"PtPte1PortState.MirrorSrcDrop",
"PtPte1PortState.DoubleDequeued",
"PtPte1PortState.EopExtraBurst",
"PtPte1PortState.ResidueByte",
"PtPte1PortState.PktRequestState",
"PtPte1PortState.Line",
"PtPte1PortState.LastPage",
"PtPte1PortState.NextPageState",
"PtPte1PortState.NextPage",
"PtPte1PortState.NextLength",
"PtPte1PortState.Page",
"PtPte1PortState.SavedBytes",
"PtPte1PortState.RemainingBytes",
"PtPte1PortState.Eop",
"PtPte1PortState.Sop",
"PtPte1PortState.PktLength",
"PtPte1PortState.ContinueByte",
"PtPte1PortState.SourceQueue",
"PtPte2MirrorPortState.HdrPresent",
"PtPte2MirrorPortState.PktLength",
"PtPte2MirrorPortState.NumPagesHi",
"PtPte2MirrorPortState.NumPagesLo",
"PtPte2MirrorPortState.Buffer",
"PtPte2MirrorPortState.SourceQueue",
"PtPte2MirrorPortState.Queue",
"PtPte2PortState.PktInProgress",
"PtPte2PortState.Mirror",
"PtPte2PortState.MirrorSrcDrop",
"PtPte2PortState.DoubleDequeued",
"PtPte2PortState.EopExtraBurst",
"PtPte2PortState.ResidueByte",
"PtPte2PortState.PktRequestState",
"PtPte2PortState.Line",
"PtPte2PortState.LastPage",
"PtPte2PortState.NextPageState",
"PtPte2PortState.NextPage",
"PtPte2PortState.NextLength",
"PtPte2PortState.Page",
"PtPte2PortState.SavedBytes",
"PtPte2PortState.RemainingBytes",
"PtPte2PortState.Eop",
"PtPte2PortState.Sop",
"PtPte2PortState.PktLength",
"PtPte2PortState.ContinueByte",
"PtPte2PortState.SourceQueue",
"PtPte3MirrorPortState.HdrPresent",
"PtPte3MirrorPortState.PktLength",
"PtPte3MirrorPortState.NumPagesHi",
"PtPte3MirrorPortState.NumPagesLo",
"PtPte3MirrorPortState.Buffer",
"PtPte3MirrorPortState.SourceQueue",
"PtPte3MirrorPortState.Queue",
"PtPte3PortState.PktInProgress",
"PtPte3PortState.Mirror",
"PtPte3PortState.MirrorSrcDrop",
"PtPte3PortState.DoubleDequeued",
"PtPte3PortState.EopExtraBurst",
"PtPte3PortState.ResidueByte",
"PtPte3PortState.PktRequestState",
"PtPte3PortState.Line",
"PtPte3PortState.LastPage",
"PtPte3PortState.NextPageState",
"PtPte3PortState.NextPage",
"PtPte3PortState.NextLength",
"PtPte3PortState.Page",
"PtPte3PortState.SavedBytes",
"PtPte3PortState.RemainingBytes",
"PtPte3PortState.Eop",
"PtPte3PortState.Sop",
"PtPte3PortState.PktLength",
"PtPte3PortState.ContinueByte",
"PtPte3PortState.SourceQueue",
"PtPte4MirrorPortState.HdrPresent",
"PtPte4MirrorPortState.PktLength",
"PtPte4MirrorPortState.NumPagesHi",
"PtPte4MirrorPortState.NumPagesLo",
"PtPte4MirrorPortState.Buffer",
"PtPte4MirrorPortState.SourceQueue",
"PtPte4MirrorPortState.Queue",
"PtPte4PortState.PktInProgress",
"PtPte4PortState.Mirror",
"PtPte4PortState.MirrorSrcDrop",
"PtPte4PortState.DoubleDequeued",
"PtPte4PortState.EopExtraBurst",
"PtPte4PortState.ResidueByte",
"PtPte4PortState.PktRequestState",
"PtPte4PortState.Line",
"PtPte4PortState.LastPage",
"PtPte4PortState.NextPageState",
"PtPte4PortState.NextPage",
"PtPte4PortState.NextLength",
"PtPte4PortState.Page",
"PtPte4PortState.SavedBytes",
"PtPte4PortState.RemainingBytes",
"PtPte4PortState.Eop",
"PtPte4PortState.Sop",
"PtPte4PortState.PktLength",
"PtPte4PortState.ContinueByte",
"PtPte4PortState.SourceQueue",
"PtPte5MirrorPortState.HdrPresent",
"PtPte5MirrorPortState.PktLength",
"PtPte5MirrorPortState.NumPagesHi",
"PtPte5MirrorPortState.NumPagesLo",
"PtPte5MirrorPortState.Buffer",
"PtPte5MirrorPortState.SourceQueue",
"PtPte5MirrorPortState.Queue",
"PtPte5PortState.PktInProgress",
"PtPte5PortState.Mirror",
"PtPte5PortState.MirrorSrcDrop",
"PtPte5PortState.DoubleDequeued",
"PtPte5PortState.EopExtraBurst",
"PtPte5PortState.ResidueByte",
"PtPte5PortState.PktRequestState",
"PtPte5PortState.Line",
"PtPte5PortState.LastPage",
"PtPte5PortState.NextPageState",
"PtPte5PortState.NextPage",
"PtPte5PortState.NextLength",
"PtPte5PortState.Page",
"PtPte5PortState.SavedBytes",
"PtPte5PortState.RemainingBytes",
"PtPte5PortState.Eop",
"PtPte5PortState.Sop",
"PtPte5PortState.PktLength",
"PtPte5PortState.ContinueByte",
"PtPte5PortState.SourceQueue",
"PtPte6MirrorPortState.HdrPresent",
"PtPte6MirrorPortState.PktLength",
"PtPte6MirrorPortState.NumPagesHi",
"PtPte6MirrorPortState.NumPagesLo",
"PtPte6MirrorPortState.Buffer",
"PtPte6MirrorPortState.SourceQueue",
"PtPte6MirrorPortState.Queue",
"PtPte6PortState.PktInProgress",
"PtPte6PortState.Mirror",
"PtPte6PortState.MirrorSrcDrop",
"PtPte6PortState.DoubleDequeued",
"PtPte6PortState.EopExtraBurst",
"PtPte6PortState.ResidueByte",
"PtPte6PortState.PktRequestState",
"PtPte6PortState.Line",
"PtPte6PortState.LastPage",
"PtPte6PortState.NextPageState",
"PtPte6PortState.NextPage",
"PtPte6PortState.NextLength",
"PtPte6PortState.Page",
"PtPte6PortState.SavedBytes",
"PtPte6PortState.RemainingBytes",
"PtPte6PortState.Eop",
"PtPte6PortState.Sop",
"PtPte6PortState.PktLength",
"PtPte6PortState.ContinueByte",
"PtPte6PortState.SourceQueue",
"Rc0Data.Data1",
"Rc0Data.Spare0",
"Rc0Data.Data0",
"END"};

/*
 * sbFe2000MemShowEntry - read and display contents of fe2000 indirect memory
 *	inputs:	unit number
 *		name of memory 
 *	output:	prints memory contents
 * 	return:	OK on success, else error status
 */
int
sbFe2000MemSetFieldEntry(int unit, int memindex, int addr, char *fieldname, int val)
{
  int uBaseAddress,bClearOnRead = 0;
  int  status;

#ifdef CAD_UINT
#undef CAD_UINT
#define CAD_UINT unsigned int
#endif

    sbZfCaPrPortToQueueAg0Entry_t   PrPortToQueueAg0Record;
    sbZfCaPrPortToQueueAg1Entry_t   PrPortToQueueAg1Record;
    sbZfCaPrPortToQueuePciEntry_t   PrPortToQueuePciRecord;
    sbZfCaPrPortToQueueSr0Entry_t   PrPortToQueueSr0Record;
    sbZfCaPrPortToQueueSr1Entry_t   PrPortToQueueSr1Record;
    sbZfCaPrPortToQueueXg0Entry_t   PrPortToQueueXg0Record;
    sbZfCaPrPortToQueueXg1Entry_t   PrPortToQueueXg1Record;
    sbZfCaPdLrpOutputHeaderCopyBufferEntry_t   PdLrpOutputHeaderCopyBufferRecord;
    sbZfCaPdPdOutputHeaderCopyBufferEntry_t   PdPdOutputHeaderCopyBufferRecord;
    sbZfCaLrLrpInstructionMemoryEntry_t   LrLrpInstructionMemoryRecord;
    sbZfCaQmBufferState0Entry_t   QmBufferState0Record;
    sbZfCaQmBufferState1Entry_t   QmBufferState1Record;
    sbZfCaQmBufferState2Entry_t   QmBufferState2Record;
    sbZfCaQmBufferState3Entry_t   QmBufferState3Record;
    sbZfCaQmDequeueState0Entry_t   QmDequeueState0Record;
    sbZfCaQmDequeueState1Entry_t   QmDequeueState1Record;
    sbZfCaQmFreePageFifoEntry_t    QmFreePageFifoRecord;
    sbZfCaQmNextBufferEntry_t      QmNextBufferRecord;
    sbZfCaQmNextPageEntry_t        QmNextPageRecord;
    sbZfCaQmQueueConfigEntry_t     QmQueueConfigRecord;
    sbZfCaQmQueueCountersEntry_t   QmQueueCountersRecord;
    sbZfCaQmQueueHeadPtrEntry_t    QmQueueHeadPtrRecord;
    sbZfCaQmQueueState0Entry_t   QmQueueState0Record;
    sbZfCaQmQueueState0EnEntry_t   QmQueueState0EnRecord;
    sbZfCaQmQueueState1Entry_t   QmQueueState1Record;
    sbZfCaQmReplicationStateEntry_t   QmReplicationStateRecord;
    sbZfCaPbCounterEntry_t   PbCounterRecord;
    sbZfCaPmProfileMemoryEntry_t   PmProfileMemoryRecord;
    sbZfCaSrCounterEntry_t   SrCounterRecord;
    sbZfCaStCounterEntry_t   StCounterRecord;
    sbZfCaPpAggregateHashBitConfigEntry_t   PpAggregateHashBitConfigRecord;
    sbZfCaPpAggregateHashByteConfigEntry_t   PpAggregateHashByteConfigRecord;
    sbZfCaPpCamConfigurationInstance0Entry_t   PpCamConfigurationInstance0Record;
    sbZfCaPpCamRamConfigurationInstance0Entry_t   PpCamRamConfigurationInstance0Record;
/*    sbZfCaPpCamRamConfigurationInstance1Entry_t   PpCamRamConfigurationInstance1Record;
    sbZfCaPpCamRamConfigurationInstance2Entry_t   PpCamRamConfigurationInstance2Record;
    sbZfCaPpCamRamConfigurationInstance3Entry_t   PpCamRamConfigurationInstance3Record;
*/
    sbZfCaPpHeaderRecordSizeEntry_t   PpHeaderRecordSizeRecord;
    sbZfCaPpInitialQueueStateEntry_t   PpInitialQueueStateRecord;
    sbZfCaPpPpOutHeaderCopyEntry_t   PpPpOutHeaderCopyRecord;
    sbZfCaPpQueuePriorityGroupEntry_t   PpQueuePriorityGroupRecord;
    sbZfCaPpRxPortDataEntry_t   PpRxPortDataRecord;
    sbZfCaRcDataEntry_t         RcDataRecord;
    sbZfCaMmInternal0MemoryEntry_t   MmInternal0MemoryRecord;
    sbZfCaMmInternal1MemoryEntry_t   MmInternal1MemoryRecord;
    sbZfCaMmNarrowPort0MemoryEntry_t   MmNarrowPort0MemoryRecord;
    sbZfCaMmNarrowPort1MemoryEntry_t   MmNarrowPort1MemoryRecord;
    sbZfCaMmWidePortMemoryEntry_t      MmWidePortMemoryRecord;
    sbZfCaPbDataEntry_t   PbDataRecord;
    sbZfCaPtMirrorIndexEntry_t   PtMirrorIndexRecord;
    sbZfCaPtPortToQueueEntry_t      PtPortToQueueRecord;
/* MCM - possibly need specific records fpr Pt Unpack ???
 * PtPort structure same as PrPort ???
    sbZfCaPrPortToQueueAg0Entry_t   PtPortToQueueAg0Record;
    sbZfCaPrPortToQueueAg1Entry_t   PtPortToQueueAg1Record;
    sbZfCaPrPortToQueuePciEntry_t   PtPortToQueuePciRecord;
    sbZfCaPrPortToQueueSr0Entry_t   PtPortToQueueSr0Record;
    sbZfCaPrPortToQueueSr1Entry_t   PtPortToQueueSr1Record;
    sbZfCaPrPortToQueueXg0Entry_t   PtPortToQueueXg0Record;
    sbZfCaPrPortToQueueXg1Entry_t   PtPortToQueueXg1Record;
*/
    sbZfCaPtPte0MirrorPortStateEntry_t   PtPte0MirrorPortStateRecord;
    sbZfCaPtPte0PortStateEntry_t   PtPte0PortStateRecord;
    sbZfCaPtPte1MirrorPortStateEntry_t   PtPte1MirrorPortStateRecord;
    sbZfCaPtPte1PortStateEntry_t   PtPte1PortStateRecord;
    sbZfCaPtPte2MirrorPortStateEntry_t   PtPte2MirrorPortStateRecord;
    sbZfCaPtPte2PortStateEntry_t   PtPte2PortStateRecord;
    sbZfCaPtPte3MirrorPortStateEntry_t   PtPte3MirrorPortStateRecord;
    sbZfCaPtPte3PortStateEntry_t   PtPte3PortStateRecord;
    sbZfCaPtPte4MirrorPortStateEntry_t   PtPte4MirrorPortStateRecord;
    sbZfCaPtPte4PortStateEntry_t   PtPte4PortStateRecord;
    sbZfCaPtPte5MirrorPortStateEntry_t   PtPte5MirrorPortStateRecord;
    sbZfCaPtPte5PortStateEntry_t   PtPte5PortStateRecord;
    sbZfCaPtPte6MirrorPortStateEntry_t   PtPte6MirrorPortStateRecord;
    sbZfCaPtPte6PortStateEntry_t   PtPte6PortStateRecord;
    sbZfCaPtQueueToPortEntry_t   PtQueueToPortRecord;

   CAD_UINT dbuf[40];
   CAD_UINT                     uAddress,
                                *pData  = &dbuf[0],
                                *pData0 = &dbuf[0],
                                *pData1 = &dbuf[1],
                                *pData2 = &dbuf[2],
                                *pData3 = &dbuf[3],
                                *pData4 = &dbuf[4],
                                *pData5 = &dbuf[5],
                                *pData6 = &dbuf[6],
                                *pData7 = &dbuf[7],
                                *pData8 = &dbuf[8],
                                *pData9 = &dbuf[9],
                                *pData10 = &dbuf[10],
                                *pData11 = &dbuf[11],
                                *pData12 = &dbuf[12],
                                *pData13 = &dbuf[13],
                                *pData14 = &dbuf[14],
                                *pData15 = &dbuf[15],
                                *pData16 = &dbuf[16],
                                *pData17 = &dbuf[17],
                                *pData18 = &dbuf[18],
                                *pData19 = &dbuf[19],
                                *pData20 = &dbuf[20],
                                *pData21 = &dbuf[21],
                                *pData22 = &dbuf[22],
                                *pData23 = &dbuf[23],
                                *pData24 = &dbuf[24],
                                *pData25 = &dbuf[25],
                                *pData26 = &dbuf[26],
                                *pData27 = &dbuf[27],
                                *pData28 = &dbuf[28],
                                *pData29 = &dbuf[29],
                                *pData30 = &dbuf[30],
                                *pData31 = &dbuf[31],
                                *pData32 = &dbuf[32],
                                *pData33 = &dbuf[33],
                                *pData34 = &dbuf[34],
                                *pData35 = &dbuf[35];

    uBaseAddress = unit;
    uAddress = addr;

    switch(memindex) {
    case FE2K_MEM_PRPORTTOQUEUEAG0:
        status =
        sbFe2000ImfDriver_PrPortToQueueAg0Read(  uBaseAddress, 
                                                 uAddress,   
                                                 pData);    

        if (status == SAND_DRV_CA_STATUS_OK) {

          sbSwapWords(pData,1);
          sbZfCaPrPortToQueueAg0Entry_Unpack(&PrPortToQueueAg0Record,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPrPortToQueueAg0Entry_SetField(&PrPortToQueueAg0Record,fieldname,val); 
          if(status != 0) return CMD_USAGE;
          sbZfCaPrPortToQueueAg0Entry_Pack(&PrPortToQueueAg0Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000ImfDriver_PrPortToQueueAg0Write( uBaseAddress, 
                                                 uAddress,   
                                                 *pData);    
        }
    break;
    case FE2K_MEM_PRPORTTOQUEUEAG1:
        status =
        sbFe2000ImfDriver_PrPortToQueueAg1Read(  uBaseAddress, 
                                                 uAddress,   
                                                 pData);    
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfCaPrPortToQueueAg1Entry_Unpack(&PrPortToQueueAg1Record,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPrPortToQueueAg1Entry_SetField(&PrPortToQueueAg1Record,fieldname,val); 
          if(status != 0) return CMD_USAGE;
          sbZfCaPrPortToQueueAg1Entry_Pack(&PrPortToQueueAg1Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000ImfDriver_PrPortToQueueAg1Write(  uBaseAddress, 
                                                 uAddress,   
                                                 *pData);    
        }
    break;
    case FE2K_MEM_PRPORTTOQUEUEPCI:
        status =
        sbFe2000ImfDriver_PrPortToQueuePciRead(  uBaseAddress, 
                                                 uAddress,   
                                                 pData);    
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfCaPrPortToQueuePciEntry_Unpack(&PrPortToQueuePciRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPrPortToQueuePciEntry_SetField(&PrPortToQueuePciRecord,fieldname,val); 
          if(status != 0) return CMD_USAGE;
          sbZfCaPrPortToQueuePciEntry_Pack(&PrPortToQueuePciRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000ImfDriver_PrPortToQueuePciWrite(  uBaseAddress, 
                                                 uAddress,   
                                                 *pData);    
        }
    break;
    case FE2K_MEM_PRPORTTOQUEUESR0:
        status =
        sbFe2000ImfDriver_PrPortToQueueSr0Read(  uBaseAddress, 
                                                 uAddress,   
                                                 pData);    
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfCaPrPortToQueueSr0Entry_Unpack(&PrPortToQueueSr0Record,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPrPortToQueueSr0Entry_SetField(&PrPortToQueueSr0Record,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPrPortToQueueSr0Entry_Pack(&PrPortToQueueSr0Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000ImfDriver_PrPortToQueueSr0Write(  uBaseAddress, 
                                                 uAddress,   
                                                 *pData);    
        }
    break;
    case FE2K_MEM_PRPORTTOQUEUESR1:
        status =
        sbFe2000ImfDriver_PrPortToQueueSr1Read(  uBaseAddress, 
                                                 uAddress,   
                                                 pData);    
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfCaPrPortToQueueSr1Entry_Unpack(&PrPortToQueueSr1Record,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPrPortToQueueSr1Entry_SetField(&PrPortToQueueSr1Record,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPrPortToQueueSr1Entry_Pack(&PrPortToQueueSr1Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000ImfDriver_PrPortToQueueSr1Write(  uBaseAddress, 
                                                 uAddress,   
                                                 *pData);    
        }
    break;
    case FE2K_MEM_PRPORTTOQUEUEXG0:
        status =
        sbFe2000ImfDriver_PrPortToQueueXg0Read(  uBaseAddress, 
                                                 uAddress,   
                                                 pData);    
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfCaPrPortToQueueXg0Entry_Unpack(&PrPortToQueueXg0Record,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPrPortToQueueXg0Entry_SetField(&PrPortToQueueXg0Record,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPrPortToQueueXg0Entry_Pack(&PrPortToQueueXg0Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000ImfDriver_PrPortToQueueXg0Write(  uBaseAddress, 
                                                 uAddress,   
                                                 *pData);    
        }
    break;
    case FE2K_MEM_PRPORTTOQUEUEXG1:
        status =
        sbFe2000ImfDriver_PrPortToQueueXg1Read(  uBaseAddress, 
                                                 uAddress,   
                                                 pData);    
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfCaPrPortToQueueXg1Entry_Unpack(&PrPortToQueueXg1Record,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPrPortToQueueXg1Entry_SetField(&PrPortToQueueXg1Record,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPrPortToQueueXg1Entry_Pack(&PrPortToQueueXg1Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000ImfDriver_PrPortToQueueXg1Write(  uBaseAddress, 
                                                 uAddress,   
                                                 *pData);    
        }
    break;
    case FE2K_MEM_PDLRPOUTPUTHEADERCOPYBUFFER:
        status =
        sbFe2000ImfDriver_PdLrpOutputHeaderCopyBufferRead(  uBaseAddress, 
                                                            uAddress, 
                                                            pData0, 
                                                            pData1, 
                                                            pData2, 
                                                            pData3, 
                                                            pData4, 
                                                            pData5, 
                                                            pData6, 
                                                            pData7); 
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,8);
          sbZfCaPdLrpOutputHeaderCopyBufferEntry_Unpack(&PdLrpOutputHeaderCopyBufferRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPdLrpOutputHeaderCopyBufferEntry_SetField(&PdLrpOutputHeaderCopyBufferRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPdLrpOutputHeaderCopyBufferEntry_Pack(&PdLrpOutputHeaderCopyBufferRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,8);
          status =
          sbFe2000ImfDriver_PdLrpOutputHeaderCopyBufferWrite(  uBaseAddress, 
                                                            uAddress, 
                                                            *pData0, 
                                                            *pData1, 
                                                            *pData2, 
                                                            *pData3, 
                                                            *pData4, 
                                                            *pData5, 
                                                            *pData6, 
                                                            *pData7); 

        }
    break;
    case FE2K_MEM_PDPDOUTPUTHEADERCOPYBUFFER:
        status =
        sbFe2000ImfDriver_PdPdOutputHeaderCopyBufferRead(  uBaseAddress, 
                                                           uAddress, 
                                                           pData0, 
                                                           pData1, 
                                                           pData2, 
                                                           pData3, 
                                                           pData4, 
                                                           pData5, 
                                                           pData6, 
                                                           pData7); 
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,8);
          sbZfCaPdPdOutputHeaderCopyBufferEntry_Unpack(&PdPdOutputHeaderCopyBufferRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPdPdOutputHeaderCopyBufferEntry_SetField(&PdPdOutputHeaderCopyBufferRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPdPdOutputHeaderCopyBufferEntry_Pack(&PdPdOutputHeaderCopyBufferRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,8);
          status =
          sbFe2000ImfDriver_PdPdOutputHeaderCopyBufferWrite(  uBaseAddress, 
                                                           uAddress, 
                                                           *pData0, 
                                                           *pData1, 
                                                           *pData2, 
                                                           *pData3, 
                                                           *pData4, 
                                                           *pData5, 
                                                           *pData6, 
                                                           *pData7); 

        }
    break;
    case FE2K_MEM_LRLRPINSTRUCTIONMEMORY:
        status =
        sbFe2000ImfDriver_LrLrpInstructionMemoryRead(  uBaseAddress, 
                                                       uAddress, 
                                                       pData0, 
                                                       pData1, 
                                                       pData2); 
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,3);
          sbZfCaLrLrpInstructionMemoryEntry_Unpack(&LrLrpInstructionMemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaLrLrpInstructionMemoryEntry_SetField(&LrLrpInstructionMemoryRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaLrLrpInstructionMemoryEntry_Pack(&LrLrpInstructionMemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,3);
          status =
          sbFe2000ImfDriver_LrLrpInstructionMemoryWrite(  uBaseAddress, 
                                                       uAddress, 
                                                       *pData0, 
                                                       *pData1, 
                                                       *pData2); 
        }
    break;
    case FE2K_MEM_QMBUFFERSTATE0:
        status =
        sbFe2000ImfDriver_QmBufferState0Read(  uBaseAddress, 
                                               uAddress,     
                                               pData);      
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfCaQmBufferState0Entry_Unpack(&QmBufferState0Record,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaQmBufferState0Entry_SetField(&QmBufferState0Record,fieldname,val);
          if(status != 0) return CMD_USAGE;

          sbZfCaQmBufferState0Entry_Pack(&QmBufferState0Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000ImfDriver_QmBufferState0Write(  uBaseAddress, 
                                               uAddress,     
                                               *pData);      
        }
    break;
    case FE2K_MEM_QMBUFFERSTATE1:
        status =
        sbFe2000ImfDriver_QmBufferState1Read(  uBaseAddress, 
                                               uAddress,     
                                               pData0,      
                                               pData1);     
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfCaQmBufferState1Entry_Unpack(&QmBufferState1Record,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaQmBufferState1Entry_SetField(&QmBufferState1Record,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaQmBufferState1Entry_Pack(&QmBufferState1Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000ImfDriver_QmBufferState1Write(  uBaseAddress, 
                                               uAddress,     
                                               *pData0,      
                                               *pData1);     
        }
    break;
    case FE2K_MEM_QMBUFFERSTATE2:
        status =
        sbFe2000ImfDriver_QmBufferState2Read(  uBaseAddress, 
                                               uAddress,     
                                               pData);      
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfCaQmBufferState2Entry_Unpack(&QmBufferState2Record,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaQmBufferState2Entry_SetField(&QmBufferState2Record,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaQmBufferState2Entry_Pack(&QmBufferState2Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000ImfDriver_QmBufferState2Write(  uBaseAddress, 
                                               uAddress,     
                                               *pData);      
        }
    break;
    case FE2K_MEM_QMBUFFERSTATE3:
        status =
        sbFe2000ImfDriver_QmBufferState3Read(  uBaseAddress, 
                                               uAddress,     
                                               pData);      
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfCaQmBufferState3Entry_Unpack(&QmBufferState3Record,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaQmBufferState3Entry_SetField(&QmBufferState3Record,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaQmBufferState3Entry_Pack(&QmBufferState3Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000ImfDriver_QmBufferState3Write(  uBaseAddress, 
                                               uAddress,     
                                               *pData);      
        }
    break;
/* MCM was missing DequeueState0 code */
    case FE2K_MEM_QMDEQUEUESTATE0:
        status =
        sbFe2000ImfDriver_QmDequeueState0Read(  uBaseAddress, 
                                                uAddress,    
                                                pData0,     
                                                pData1);     
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfCaQmDequeueState0Entry_Unpack(&QmDequeueState0Record,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaQmDequeueState0Entry_SetField(&QmDequeueState0Record,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaQmDequeueState0Entry_Pack(&QmDequeueState0Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000ImfDriver_QmDequeueState0Write(  uBaseAddress, 
                                                uAddress,    
                                                *pData0,     
                                                *pData1);     
        }
    break;

    case FE2K_MEM_QMDEQUEUESTATE1:
        status =
        sbFe2000ImfDriver_QmDequeueState1Read(  uBaseAddress, 
                                                uAddress,    
                                                pData);     
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfCaQmDequeueState1Entry_Unpack(&QmDequeueState1Record,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaQmDequeueState1Entry_SetField(&QmDequeueState1Record,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaQmDequeueState1Entry_Pack(&QmDequeueState1Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000ImfDriver_QmDequeueState1Write(  uBaseAddress, 
                                                uAddress,    
                                                *pData);     
        }
    break;
    case FE2K_MEM_QMFREEPAGEFIFO:
        status =
        sbFe2000ImfDriver_QmFreePageFifoRead(  uBaseAddress, 
                                               uAddress,     
                                               pData);      
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfCaQmFreePageFifoEntry_Unpack(&QmFreePageFifoRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaQmFreePageFifoEntry_SetField(&QmFreePageFifoRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaQmFreePageFifoEntry_Pack(&QmFreePageFifoRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000ImfDriver_QmFreePageFifoWrite(  uBaseAddress, 
                                               uAddress,     
                                               *pData);      
        }
    break;
    case FE2K_MEM_QMNEXTBUFFER:
        status =
        sbFe2000ImfDriver_QmNextBufferRead(  uBaseAddress,   
                                             uAddress,       
                                             pData);        
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfCaQmNextBufferEntry_Unpack(&QmNextBufferRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaQmNextBufferEntry_SetField(&QmNextBufferRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaQmNextBufferEntry_Pack(&QmNextBufferRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000ImfDriver_QmNextBufferWrite(  uBaseAddress,   
                                             uAddress,       
                                             *pData);        
        }
    break;
    case FE2K_MEM_QMNEXTPAGE:
        status =
        sbFe2000ImfDriver_QmNextPageRead(  uBaseAddress,     
                                           uAddress,         
                                           pData);          
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfCaQmNextPageEntry_Unpack(&QmNextPageRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaQmNextPageEntry_SetField(&QmNextPageRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaQmNextPageEntry_Pack(&QmNextPageRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000ImfDriver_QmNextPageWrite(  uBaseAddress,     
                                           uAddress,         
                                           *pData);          
        }
    break;
    case FE2K_MEM_QMQUEUECONFIG:
        status =
        sbFe2000ImfDriver_QmQueueConfigRead(  uBaseAddress,  
                                              uAddress,      
                                              pData0,       
                                              pData1,       
                                              pData2);      
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,3);
          sbZfCaQmQueueConfigEntry_Unpack(&QmQueueConfigRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaQmQueueConfigEntry_SetField(&QmQueueConfigRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaQmQueueConfigEntry_Pack(&QmQueueConfigRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,3);
          status =
          sbFe2000ImfDriver_QmQueueConfigWrite(  uBaseAddress,  
                                              uAddress,      
                                              *pData0,       
                                              *pData1,       
                                              *pData2);      
        }
    break;
    case FE2K_MEM_QMQUEUECOUNTERS:
        status =
        sbFe2000ImfDriver_QmQueueCountersRead(  uBaseAddress, 
                                                uAddress,    
                                                pData0,     
                                                pData1);    
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfCaQmQueueCountersEntry_Unpack(&QmQueueCountersRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaQmQueueCountersEntry_SetField(&QmQueueCountersRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaQmQueueCountersEntry_Pack(&QmQueueCountersRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000ImfDriver_QmQueueCountersWrite(  uBaseAddress, 
                                                uAddress,    
                                                *pData0,     
                                                *pData1);    
        }
    break;
    case FE2K_MEM_QMQUEUEHEADPTR:
        status =
        sbFe2000ImfDriver_QmQueueHeadPtrRead(  uBaseAddress, 
                                               uAddress,     
                                               pData);      
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfCaQmQueueHeadPtrEntry_Unpack(&QmQueueHeadPtrRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaQmQueueHeadPtrEntry_SetField(&QmQueueHeadPtrRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaQmQueueHeadPtrEntry_Pack(&QmQueueHeadPtrRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000ImfDriver_QmQueueHeadPtrWrite(  uBaseAddress, 
                                               uAddress,     
                                               *pData);      
        }
    break;
    case FE2K_MEM_QMQUEUESTATE0:
        status =
        sbFe2000ImfDriver_QmQueueState0Read(  uBaseAddress,  
                                              uAddress,      
                                              pData);       
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfCaQmQueueState0Entry_Unpack(&QmQueueState0Record,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaQmQueueState0Entry_SetField(&QmQueueState0Record,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaQmQueueState0Entry_Pack(&QmQueueState0Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000ImfDriver_QmQueueState0Write(  uBaseAddress,  
                                              uAddress,      
                                              *pData);       
        }
    break;
    case FE2K_MEM_QMQUEUESTATE0EN:
        status =
        sbFe2000ImfDriver_QmQueueState0EnRead(  uBaseAddress, 
                                                uAddress,    
                                                pData);     
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfCaQmQueueState0EnEntry_Unpack(&QmQueueState0EnRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaQmQueueState0EnEntry_SetField(&QmQueueState0EnRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaQmQueueState0EnEntry_Pack(&QmQueueState0EnRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000ImfDriver_QmQueueState0EnWrite(  uBaseAddress, 
                                                uAddress,    
                                                *pData);     
        }
    break;
    case FE2K_MEM_QMQUEUESTATE1:
        status =
        sbFe2000ImfDriver_QmQueueState1Read(  uBaseAddress,  
                                              uAddress,      
                                              pData);       
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfCaQmQueueState1Entry_Unpack(&QmQueueState1Record,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaQmQueueState1Entry_SetField(&QmQueueState1Record,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaQmQueueState1Entry_Pack(&QmQueueState1Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000ImfDriver_QmQueueState1Write(  uBaseAddress,  
                                              uAddress,      
                                              *pData);       
        }
    break;
    case FE2K_MEM_QMREPLICATIONSTATE:
        status =
        sbFe2000ImfDriver_QmReplicationStateRead(  uBaseAddress, 
                                                   uAddress, 
                                                   pData);  
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfCaQmReplicationStateEntry_Unpack(&QmReplicationStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaQmReplicationStateEntry_SetField(&QmReplicationStateRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaQmReplicationStateEntry_Pack(&QmReplicationStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000ImfDriver_QmReplicationStateWrite(  uBaseAddress, 
                                                   uAddress, 
                                                   *pData);  
        }
    break;
    case FE2K_MEM_PMCOUNTERMEMORY:
 /* MCM no PmCounter zframe */
        status =
        sbFe2000ImfDriver_PmCounterMemoryRead(  uBaseAddress,
                                                uAddress,    
                                                bClearOnRead, 
                                                pData0,     
                                                pData1);    
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfCaPbCounterEntry_Unpack(&PbCounterRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPbCounterEntry_SetField(&PbCounterRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPbCounterEntry_Pack(&PbCounterRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000ImfDriver_PmCounterMemoryWrite(  uBaseAddress,
                                                uAddress,    
                                                *pData0,     
                                                *pData1);    
        }
    break;
    case FE2K_MEM_PMPROFILEMEMORY:
        status =
        sbFe2000ImfDriver_PmProfileMemoryRead(  uBaseAddress, 
                                                uAddress,    
                                                pData0,     
                                                pData1);    
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfCaPmProfileMemoryEntry_Unpack(&PmProfileMemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPmProfileMemoryEntry_SetField(&PmProfileMemoryRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPmProfileMemoryEntry_Pack(&PmProfileMemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000ImfDriver_PmProfileMemoryWrite(  uBaseAddress, 
                                                uAddress,    
                                                *pData0,     
                                                *pData1);    
        }
    break;
    case FE2K_MEM_SR0COUNTER:
        status =
        sbFe2000ImfDriver_Sr0CounterRead(  uBaseAddress,     
                                           uAddress,         
                                           bClearOnRead,     
                                           pData0,          
                                           pData1);         
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfCaSrCounterEntry_Unpack(&SrCounterRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaSrCounterEntry_SetField(&SrCounterRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaSrCounterEntry_Pack(&SrCounterRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000ImfDriver_Sr0CounterWrite(  uBaseAddress,     
                                           uAddress,         
                                           *pData0,          
                                           *pData1);         
        }
    break;
    case FE2K_MEM_SR1COUNTER:
        status =
        sbFe2000ImfDriver_Sr1CounterRead(  uBaseAddress,     
                                           uAddress,         
                                           bClearOnRead,     
                                           pData0,          
                                           pData1);         
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfCaSrCounterEntry_Unpack(&SrCounterRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaSrCounterEntry_SetField(&SrCounterRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaSrCounterEntry_Pack(&SrCounterRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000ImfDriver_Sr1CounterWrite(  uBaseAddress,     
                                           uAddress,         
                                           *pData0,          
                                           *pData1);         
        }
    break;
    case FE2K_MEM_PPAGGREGATEHASHBITCONFIG:
        status =
        sbFe2000ImfDriver_PpAggregateHashBitConfigRead(  uBaseAddress, 
                                                         uAddress, 
                                                         pData0, 
                                                         pData1, 
                                                         pData2, 
                                                         pData3, 
                                                         pData4, 
                                                         pData5, 
                                                         pData6, 
                                                         pData7, 
                                                         pData8, 
                                                         pData9); 
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,10);
          sbZfCaPpAggregateHashBitConfigEntry_Unpack(&PpAggregateHashBitConfigRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPpAggregateHashBitConfigEntry_SetField(&PpAggregateHashBitConfigRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPpAggregateHashBitConfigEntry_Pack(&PpAggregateHashBitConfigRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,10);
          status =
          sbFe2000ImfDriver_PpAggregateHashBitConfigWrite(  uBaseAddress, 
                                                         uAddress, 
                                                         *pData0, 
                                                         *pData1, 
                                                         *pData2, 
                                                         *pData3, 
                                                         *pData4, 
                                                         *pData5, 
                                                         *pData6, 
                                                         *pData7, 
                                                         *pData8, 
                                                         *pData9); 
        }
    break;
    case FE2K_MEM_PPAGGREGATEHASHBYTECONFIG:
        status =
        sbFe2000ImfDriver_PpAggregateHashByteConfigRead(  uBaseAddress, 
                                                          uAddress, 
                                                          pData0, 
                                                          pData1); 
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfCaPpAggregateHashByteConfigEntry_Unpack(&PpAggregateHashByteConfigRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPpAggregateHashByteConfigEntry_SetField(&PpAggregateHashByteConfigRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPpAggregateHashByteConfigEntry_Pack(&PpAggregateHashByteConfigRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000ImfDriver_PpAggregateHashByteConfigWrite(  uBaseAddress, 
                                                          uAddress, 
                                                          *pData0, 
                                                          *pData1); 
        }
    break;
    case FE2K_MEM_PPCAMCONFIGURATIONINSTANCE0:
        status =
        sbFe2000ImfDriver_PpCamConfigurationInstance0Read(  uBaseAddress, 
                                                            uAddress, 
                                                            pData0, 
                                                            pData1, 
                                                            pData2, 
                                                            pData3, 
                                                            pData4, 
                                                            pData5, 
                                                            pData6, 
                                                            pData7, 
                                                            pData8, 
                                                            pData9, 
                                                            pData10, 
                                                            pData11); 
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,12);
          sbZfCaPpCamConfigurationInstance0Entry_Unpack(&PpCamConfigurationInstance0Record,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPpCamConfigurationInstance0Entry_SetField(&PpCamConfigurationInstance0Record,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPpCamConfigurationInstance0Entry_Pack(&PpCamConfigurationInstance0Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,12);
          status =
          sbFe2000ImfDriver_PpCamConfigurationInstance0Write(  uBaseAddress, 
                                                            uAddress, 
                                                            *pData0, 
                                                            *pData1, 
                                                            *pData2, 
                                                            *pData3, 
                                                            *pData4, 
                                                            *pData5, 
                                                            *pData6, 
                                                            *pData7, 
                                                            *pData8, 
                                                            *pData9, 
                                                            *pData10, 
                                                            *pData11); 

        }
    break;
    case FE2K_MEM_PPCAMCONFIGURATIONINSTANCE1:
/* MCM - all instances use Instance0 Unpack/Print because no zframe for
 * Instance1 - 4 */
        status =
        sbFe2000ImfDriver_PpCamConfigurationInstance1Read(  uBaseAddress, 
                                                            uAddress, 
                                                            pData0, 
                                                            pData1, 
                                                            pData2, 
                                                            pData3, 
                                                            pData4, 
                                                            pData5, 
                                                            pData6, 
                                                            pData7, 
                                                            pData8, 
                                                            pData9, 
                                                            pData10, 
                                                            pData11); 
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,12);
          sbZfCaPpCamConfigurationInstance0Entry_Unpack(&PpCamConfigurationInstance0Record,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPpCamConfigurationInstance0Entry_SetField(&PpCamConfigurationInstance0Record,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPpCamConfigurationInstance0Entry_Pack(&PpCamConfigurationInstance0Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,12);
          status =
          sbFe2000ImfDriver_PpCamConfigurationInstance1Write(  uBaseAddress, 
                                                            uAddress, 
                                                            *pData0, 
                                                            *pData1, 
                                                            *pData2, 
                                                            *pData3, 
                                                            *pData4, 
                                                            *pData5, 
                                                            *pData6, 
                                                            *pData7, 
                                                            *pData8, 
                                                            *pData9, 
                                                            *pData10, 
                                                            *pData11); 

        }
    break;
    case FE2K_MEM_PPCAMCONFIGURATIONINSTANCE2:
        status =
        sbFe2000ImfDriver_PpCamConfigurationInstance2Read(  uBaseAddress, 
                                                            uAddress, 
                                                            pData0, 
                                                            pData1, 
                                                            pData2, 
                                                            pData3, 
                                                            pData4, 
                                                            pData5, 
                                                            pData6, 
                                                            pData7, 
                                                            pData8, 
                                                            pData9, 
                                                            pData10, 
                                                            pData11); 
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,12);
          sbZfCaPpCamConfigurationInstance0Entry_Unpack(&PpCamConfigurationInstance0Record,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPpCamConfigurationInstance0Entry_SetField(&PpCamConfigurationInstance0Record,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPpCamConfigurationInstance0Entry_Pack(&PpCamConfigurationInstance0Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,12);
          status =
          sbFe2000ImfDriver_PpCamConfigurationInstance2Write(  uBaseAddress, 
                                                            uAddress, 
                                                            *pData0, 
                                                            *pData1, 
                                                            *pData2, 
                                                            *pData3, 
                                                            *pData4, 
                                                            *pData5, 
                                                            *pData6, 
                                                            *pData7, 
                                                            *pData8, 
                                                            *pData9, 
                                                            *pData10, 
                                                            *pData11); 

        }
    break;
    case FE2K_MEM_PPCAMCONFIGURATIONINSTANCE3:
        status =
        sbFe2000ImfDriver_PpCamConfigurationInstance3Read(  uBaseAddress, 
                                                            uAddress, 
                                                            pData0, 
                                                            pData1, 
                                                            pData2, 
                                                            pData3, 
                                                            pData4, 
                                                            pData5, 
                                                            pData6, 
                                                            pData7, 
                                                            pData8, 
                                                            pData9, 
                                                            pData10, 
                                                            pData11); 
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,12);
          sbZfCaPpCamConfigurationInstance0Entry_Unpack(&PpCamConfigurationInstance0Record,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPpCamConfigurationInstance0Entry_SetField(&PpCamConfigurationInstance0Record,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPpCamConfigurationInstance0Entry_Pack(&PpCamConfigurationInstance0Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,12);
          status =
          sbFe2000ImfDriver_PpCamConfigurationInstance3Write(  uBaseAddress, 
                                                            uAddress, 
                                                            *pData0, 
                                                            *pData1, 
                                                            *pData2, 
                                                            *pData3, 
                                                            *pData4, 
                                                            *pData5, 
                                                            *pData6, 
                                                            *pData7, 
                                                            *pData8, 
                                                            *pData9, 
                                                            *pData10, 
                                                            *pData11); 

        }
    break;
    case FE2K_MEM_PPCAMRAMCONFIGURATIONINSTANCE0:
        status =
        sbFe2000ImfDriver_PpCamRamConfigurationInstance0Read(  uBaseAddress, 
                                                               uAddress, 
                                                                       pData0, 
                                                               pData1, 
                                                               pData2, 
                                                               pData3, 
                                                               pData4); 
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,5);
          sbZfCaPpCamRamConfigurationInstance0Entry_Unpack(&PpCamRamConfigurationInstance0Record,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPpCamRamConfigurationInstance0Entry_SetField(&PpCamRamConfigurationInstance0Record,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPpCamRamConfigurationInstance0Entry_Pack(&PpCamRamConfigurationInstance0Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,5);
          status =
          sbFe2000ImfDriver_PpCamRamConfigurationInstance0Write(  uBaseAddress, 
                                                               uAddress, 
                                                                       *pData0, 
                                                               *pData1, 
                                                               *pData2, 
                                                               *pData3, 
                                                               *pData4); 

        }
    break;
    case FE2K_MEM_PPCAMRAMCONFIGURATIONINSTANCE1:
        status =
        sbFe2000ImfDriver_PpCamRamConfigurationInstance1Read(  uBaseAddress, 
                                                               uAddress, 
                                                               pData0, 
                                                               pData1, 
                                                               pData2, 
                                                               pData3, 
                                                               pData4); 
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,5);
          sbZfCaPpCamRamConfigurationInstance0Entry_Unpack(&PpCamRamConfigurationInstance0Record,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPpCamRamConfigurationInstance0Entry_SetField(&PpCamRamConfigurationInstance0Record,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPpCamRamConfigurationInstance0Entry_Pack(&PpCamRamConfigurationInstance0Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,5);
          status =
          sbFe2000ImfDriver_PpCamRamConfigurationInstance1Write(  uBaseAddress, 
                                                               uAddress, 
                                                               *pData0, 
                                                               *pData1, 
                                                               *pData2, 
                                                               *pData3, 
                                                               *pData4); 

        }
    break;
    case FE2K_MEM_PPCAMRAMCONFIGURATIONINSTANCE2:
        status =
        sbFe2000ImfDriver_PpCamRamConfigurationInstance2Read(  uBaseAddress, 
                                                               uAddress, 
                                                               pData0, 
                                                               pData1, 
                                                               pData2, 
                                                               pData3, 
                                                               pData4); 
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,5);
          sbZfCaPpCamRamConfigurationInstance0Entry_Unpack(&PpCamRamConfigurationInstance0Record,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPpCamRamConfigurationInstance0Entry_SetField(&PpCamRamConfigurationInstance0Record,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPpCamRamConfigurationInstance0Entry_Pack(&PpCamRamConfigurationInstance0Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,5);
          status =
          sbFe2000ImfDriver_PpCamRamConfigurationInstance2Write(  uBaseAddress, 
                                                               uAddress, 
                                                               *pData0, 
                                                               *pData1, 
                                                               *pData2, 
                                                               *pData3, 
                                                               *pData4); 

        }
    break;
    case FE2K_MEM_PPCAMRAMCONFIGURATIONINSTANCE3:
        status =
        sbFe2000ImfDriver_PpCamRamConfigurationInstance3Read(  uBaseAddress, 
                                                               uAddress, 
                                                               pData0, 
                                                               pData1, 
                                                               pData2, 
                                                               pData3, 
                                                               pData4); 
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,5);
          sbZfCaPpCamRamConfigurationInstance0Entry_Unpack(&PpCamRamConfigurationInstance0Record,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPpCamRamConfigurationInstance0Entry_SetField(&PpCamRamConfigurationInstance0Record,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPpCamRamConfigurationInstance0Entry_Pack(&PpCamRamConfigurationInstance0Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,5);
          status =
          sbFe2000ImfDriver_PpCamRamConfigurationInstance3Write(  uBaseAddress, 
                                                               uAddress, 
                                                               *pData0, 
                                                               *pData1, 
                                                               *pData2, 
                                                               *pData3, 
                                                               *pData4); 

        }
    break;
    case FE2K_MEM_PPHEADERRECORDSIZE:
        status =
        sbFe2000ImfDriver_PpHeaderRecordSizeRead(  uBaseAddress, 
                                                   uAddress, 
                                                   pData);  
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfCaPpHeaderRecordSizeEntry_Unpack(&PpHeaderRecordSizeRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPpHeaderRecordSizeEntry_SetField(&PpHeaderRecordSizeRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPpHeaderRecordSizeEntry_Pack(&PpHeaderRecordSizeRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000ImfDriver_PpHeaderRecordSizeWrite(  uBaseAddress, 
                                                   uAddress, 
                                                   *pData);  
        }
    break;
    case FE2K_MEM_PPINITIALQUEUESTATE:
        status =
        sbFe2000ImfDriver_PpInitialQueueStateRead(  uBaseAddress, 
                                                    uAddress, 
                                                    pData0, 
                                                    pData1, 
                                                    pData2); 
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,3);
          sbZfCaPpInitialQueueStateEntry_Unpack(&PpInitialQueueStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPpInitialQueueStateEntry_SetField(&PpInitialQueueStateRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPpInitialQueueStateEntry_Pack(&PpInitialQueueStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,3);
          status =
          sbFe2000ImfDriver_PpInitialQueueStateWrite(  uBaseAddress, 
                                                    uAddress, 
                                                    *pData0, 
                                                    *pData1, 
                                                    *pData2); 

        }
    break;
    case FE2K_MEM_PPPPOUTHEADERCOPY:
        status =
        sbFe2000ImfDriver_PpPpOutHeaderCopyRead(  uBaseAddress, 
                                                  uAddress,  
                                                  pData0,   
                                                  pData1,   
                                                  pData2,   
                                                  pData3,   
                                                  pData4,   
                                                  pData5,   
                                                  pData6,   
                                                  pData7);  
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,8);
          sbZfCaPpPpOutHeaderCopyEntry_Unpack(&PpPpOutHeaderCopyRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPpPpOutHeaderCopyEntry_SetField(&PpPpOutHeaderCopyRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPpPpOutHeaderCopyEntry_Pack(&PpPpOutHeaderCopyRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,8);
          status =
          sbFe2000ImfDriver_PpPpOutHeaderCopyWrite(  uBaseAddress, 
                                                  uAddress,  
                                                  *pData0,   
                                                  *pData1,   
                                                  *pData2,   
                                                  *pData3,   
                                                  *pData4,   
                                                  *pData5,   
                                                  *pData6,   
                                                  *pData7);  

        }
    break;
    case FE2K_MEM_PPQUEUEPRIORITYGROUP:
        status =
        sbFe2000ImfDriver_PpQueuePriorityGroupRead(  uBaseAddress, 
                                                     uAddress, 
                                                     pData); 
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfCaPpQueuePriorityGroupEntry_Unpack(&PpQueuePriorityGroupRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPpQueuePriorityGroupEntry_SetField(&PpQueuePriorityGroupRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPpQueuePriorityGroupEntry_Pack(&PpQueuePriorityGroupRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000ImfDriver_PpQueuePriorityGroupWrite(  uBaseAddress, 
                                                     uAddress, 
                                                     *pData); 
        }
    break;
    case FE2K_MEM_PPRXPORTDATA:
        status =
        sbFe2000ImfDriver_PpRxPortDataRead(  uBaseAddress,   
                                             uAddress,       
                                             pData0,        
                                             pData1,        
                                             pData2,        
                                             pData3);       
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,4);
          sbZfCaPpRxPortDataEntry_Unpack(&PpRxPortDataRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPpRxPortDataEntry_SetField(&PpRxPortDataRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPpRxPortDataEntry_Pack(&PpRxPortDataRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,4);
          status =
          sbFe2000ImfDriver_PpRxPortDataWrite(  uBaseAddress,   
                                             uAddress,       
                                             *pData0,        
                                             *pData1,        
                                             *pData2,        
                                             *pData3);    
        }
    break;
    case FE2K_MEM_RC0DATA:
        status =
        sbFe2000ImfDriver_Rc0DataRead(  uBaseAddress,        
                                        uAddress,            
                                        pData0,             
                                        pData1,             
                                        pData2,             
                                        pData3,             
                                        pData4,             
                                        pData5,             
                                        pData6,             
                                        pData7,             
                                        pData8,             
                                        pData9,             
                                        pData10,            
                                        pData11,            
                                        pData12,            
                                        pData13,            
                                        pData14,            
                                        pData15,            
                                        pData16,            
                                        pData17,            
                                        pData18,            
                                        pData19,            
                                        pData20,            
                                        pData21,            
                                        pData22,            
                                        pData23,            
                                        pData24,            
                                        pData25,            
                                        pData26,            
                                        pData27,            
                                        pData28,            
                                        pData29,            
                                        pData30,            
                                        pData31,            
                                        pData32,            
                                        pData33,            
                                        pData34,            
                                        pData35);           
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,36);
          sbZfCaRcDataEntry_Unpack(&RcDataRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaRcDataEntry_SetField(&RcDataRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaRcDataEntry_Pack(&RcDataRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,36);
          status =
          sbFe2000ImfDriver_Rc0DataWrite(  uBaseAddress,        
                                        uAddress,            
                                        *pData0,             
                                        *pData1,             
                                        *pData2,             
                                        *pData3,             
                                        *pData4,             
                                        *pData5,             
                                        *pData6,             
                                        *pData7,             
                                        *pData8,             
                                        *pData9,             
                                        *pData10,            
                                        *pData11,            
                                        *pData12,            
                                        *pData13,            
                                        *pData14,            
                                        *pData15,            
                                        *pData16,            
                                        *pData17,            
                                        *pData18,            
                                        *pData19,            
                                        *pData20,            
                                        *pData21,            
                                        *pData22,            
                                        *pData23,            
                                        *pData24,            
                                        *pData25,            
                                        *pData26,            
                                        *pData27,            
                                        *pData28,            
                                        *pData29,            
                                        *pData30,            
                                        *pData31,            
                                        *pData32,            
                                        *pData33,            
                                        *pData34,            
                                                *pData35);      
        }
    break;
    case FE2K_MEM_RC1DATA:
        status =
        sbFe2000ImfDriver_Rc1DataRead(  uBaseAddress,        
                                        uAddress,            
                                        pData0,             
                                        pData1,             
                                        pData2,             
                                        pData3,             
                                        pData4,             
                                        pData5,             
                                        pData6,             
                                        pData7,             
                                        pData8,             
                                        pData9,             
                                        pData10,            
                                        pData11,            
                                        pData12,            
                                        pData13,            
                                        pData14,            
                                        pData15,            
                                        pData16,            
                                        pData17,            
                                        pData18,            
                                        pData19,            
                                        pData20,            
                                        pData21,            
                                        pData22,            
                                        pData23,            
                                        pData24,            
                                        pData25,            
                                        pData26,            
                                        pData27,            
                                        pData28,            
                                        pData29,            
                                        pData30,            
                                        pData31,            
                                        pData32,            
                                        pData33,            
                                        pData34,            
                                        pData35);           
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,36);
          sbZfCaRcDataEntry_Unpack(&RcDataRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaRcDataEntry_SetField(&RcDataRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaRcDataEntry_Pack(&RcDataRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,36);
          status =
          sbFe2000ImfDriver_Rc1DataWrite(  uBaseAddress,        
                                        uAddress,            
                                        *pData0,             
                                        *pData1,             
                                        *pData2,             
                                        *pData3,             
                                        *pData4,             
                                        *pData5,             
                                        *pData6,             
                                        *pData7,             
                                        *pData8,             
                                        *pData9,             
                                        *pData10,            
                                        *pData11,            
                                        *pData12,            
                                        *pData13,            
                                        *pData14,            
                                        *pData15,            
                                        *pData16,            
                                        *pData17,            
                                        *pData18,            
                                        *pData19,            
                                        *pData20,            
                                        *pData21,            
                                        *pData22,            
                                        *pData23,            
                                        *pData24,            
                                        *pData25,            
                                        *pData26,            
                                        *pData27,            
                                        *pData28,            
                                        *pData29,            
                                        *pData30,            
                                        *pData31,            
                                        *pData32,            
                                        *pData33,            
                                        *pData34,            
                                        *pData35);          
        }
    break;
    case FE2K_MEM_ST0COUNTER:
        status =
        sbFe2000ImfDriver_St0CounterRead(  uBaseAddress,     
                                           uAddress,         
                                           bClearOnRead,     
                                           pData0,          
                                           pData1);         
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfCaStCounterEntry_Unpack(&StCounterRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaStCounterEntry_SetField(&StCounterRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaStCounterEntry_Pack(&StCounterRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000ImfDriver_St0CounterWrite(  uBaseAddress,     
                                           uAddress,         
                                           *pData0,          
                                           *pData1);    
        }
    break;
    case FE2K_MEM_ST1COUNTER:
        status =
        sbFe2000ImfDriver_St1CounterRead(  uBaseAddress,     
                                           uAddress,         
                                           bClearOnRead,     
                                           pData0,          
                                           pData1);         
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfCaStCounterEntry_Unpack(&StCounterRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaStCounterEntry_SetField(&StCounterRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaStCounterEntry_Pack(&StCounterRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000ImfDriver_St1CounterWrite(  uBaseAddress,     
                                           uAddress,         
                                           *pData0,          
                                           *pData1);   
        }
    break;
    case FE2K_MEM_MM0INTERNAL0MEMORY:
        status =
        sbFe2000ImfDriver_Mm0Internal0MemoryRead(  uBaseAddress, 
                                                   uAddress, 
                                                   pData0,  
                                                   pData1); 
        if (status == SAND_DRV_CA_STATUS_OK) {
 printk("Mm0Internal0MemoryRead Read data0=0x%x data1=0x%x\n",*pData0, *pData1);
          sbSwapWords(pData,2);
 printk("Mm0Internal0MemoryRead SWAP data0=0x%x data1=0x%x\n",*pData0, *pData1);
          sbZfCaMmInternal0MemoryEntry_Unpack(&MmInternal0MemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaMmInternal0MemoryEntry_SetField(&MmInternal0MemoryRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaMmInternal0MemoryEntry_Pack(&MmInternal0MemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
 printk("Mm0Internal0MemoryRead UPdate data0=0x%x data1=0x%x\n",*pData0, *pData1);
          sbSwapWords(pData,2);
 printk("Mm0Internal0MemoryRead SwapUpdate data0=0x%x data1=0x%x\n",*pData0, *pData1);
          status =
          sbFe2000ImfDriver_Mm0Internal0MemoryWrite(  uBaseAddress, 
                                                   uAddress, 
                                                   *pData0,  
                                                   *pData1); 

        }
    break;
    case FE2K_MEM_MM0INTERNAL1MEMORY:
        status =
        sbFe2000ImfDriver_Mm0Internal1MemoryRead(  uBaseAddress, 
                                                   uAddress, 
                                                   pData0,  
                                                   pData1); 
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfCaMmInternal1MemoryEntry_Unpack(&MmInternal1MemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaMmInternal1MemoryEntry_SetField(&MmInternal1MemoryRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaMmInternal1MemoryEntry_Pack(&MmInternal1MemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000ImfDriver_Mm0Internal1MemoryWrite(  uBaseAddress, 
                                                   uAddress, 
                                                   *pData0,  
                                                   *pData1); 

        }
    break;
    case FE2K_MEM_MM0NARROWPORT0MEMORY:
        status =
        sbFe2000ImfDriver_Mm0NarrowPort0MemoryRead(  uBaseAddress, 
                                                     uAddress, 
                                                     pData0, 
                                                     pData1); 
        if (status == SAND_DRV_CA_STATUS_OK) {
printk("Mm0NarrowPort0MemoryRead Read data0=0x%x data1=0x%x\n",*pData0, *pData1);
          sbSwapWords(pData,2);
printk("Mm0NarrowPort0MemoryRead Swap data0=0x%x data1=0x%x\n",*pData0, *pData1);
          sbZfCaMmNarrowPort0MemoryEntry_Unpack(&MmNarrowPort0MemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaMmNarrowPort0MemoryEntry_SetField(&MmNarrowPort0MemoryRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaMmNarrowPort0MemoryEntry_Pack(&MmNarrowPort0MemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
printk("Mm0NarrowPort0MemoryRead UPDATdata0=0x%x data1=0x%x\n",*pData0, *pData1);
          sbSwapWords(pData,2);
printk("Mm0NarrowPort0MemoryRead UPDAT SWAPdata0=0x%x data1=0x%x\n",*pData0, *pData1);
          status =
          sbFe2000ImfDriver_Mm0NarrowPort0MemoryWrite(  uBaseAddress, 
                                                     uAddress, 
                                                     *pData0, 
                                                     *pData1); 

        }
    break;
    case FE2K_MEM_MM0NARROWPORT1MEMORY:
        status =
        sbFe2000ImfDriver_Mm0NarrowPort1MemoryRead(  uBaseAddress, 
                                                     uAddress, 
                                                     pData0, 
                                                     pData1); 
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfCaMmNarrowPort1MemoryEntry_Unpack(&MmNarrowPort1MemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaMmNarrowPort1MemoryEntry_SetField(&MmNarrowPort1MemoryRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaMmNarrowPort1MemoryEntry_Pack(&MmNarrowPort1MemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000ImfDriver_Mm0NarrowPort1MemoryWrite(  uBaseAddress, 
                                                     uAddress, 
                                                     *pData0, 
                                                     *pData1); 

        }
    break;
    case FE2K_MEM_MM0WIDEPORTMEMORY:
        status =
        sbFe2000ImfDriver_Mm0WidePortMemoryRead(  uBaseAddress, 
                                                  uAddress,  
                                                  pData0,   
                                                  pData1);  
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfCaMmWidePortMemoryEntry_Unpack(&MmWidePortMemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaMmWidePortMemoryEntry_SetField(&MmWidePortMemoryRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaMmWidePortMemoryEntry_Pack(&MmWidePortMemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000ImfDriver_Mm0WidePortMemoryWrite(  uBaseAddress, 
                                                  uAddress,  
                                                  *pData0,   
                                                  *pData1);  

        }
    break;
    case FE2K_MEM_MM1INTERNAL0MEMORY:
        status =
        sbFe2000ImfDriver_Mm1Internal0MemoryRead(  uBaseAddress, 
                                                   uAddress, 
                                                   pData0,  
                                                   pData1); 
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfCaMmInternal0MemoryEntry_Unpack(&MmInternal0MemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaMmInternal0MemoryEntry_SetField(&MmInternal0MemoryRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaMmInternal0MemoryEntry_Pack(&MmInternal0MemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000ImfDriver_Mm1Internal0MemoryWrite(  uBaseAddress, 
                                                   uAddress, 
                                                   *pData0,  
                                                   *pData1); 

        }
    break;
    case FE2K_MEM_MM1INTERNAL1MEMORY:
        status =
        sbFe2000ImfDriver_Mm1Internal1MemoryRead(  uBaseAddress, 
                                                   uAddress, 
                                                   pData0,  
                                                   pData1); 
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfCaMmInternal1MemoryEntry_Unpack(&MmInternal1MemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaMmInternal1MemoryEntry_SetField(&MmInternal1MemoryRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaMmInternal1MemoryEntry_Pack(&MmInternal1MemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000ImfDriver_Mm1Internal1MemoryWrite(  uBaseAddress, 
                                                   uAddress, 
                                                   *pData0,  
                                                   *pData1); 

        }
    break;
    case FE2K_MEM_MM1NARROWPORT0MEMORY:
        status =
        sbFe2000ImfDriver_Mm1NarrowPort0MemoryRead(  uBaseAddress, 
                                                     uAddress, 
                                                     pData0, 
                                                     pData1); 
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfCaMmNarrowPort0MemoryEntry_Unpack(&MmNarrowPort0MemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaMmNarrowPort0MemoryEntry_SetField(&MmNarrowPort0MemoryRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaMmNarrowPort0MemoryEntry_Pack(&MmNarrowPort0MemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000ImfDriver_Mm1NarrowPort0MemoryWrite(  uBaseAddress, 
                                                     uAddress, 
                                                     *pData0, 
                                                     *pData1); 

        }
    break;
    case FE2K_MEM_MM1NARROWPORT1MEMORY:
        status =
        sbFe2000ImfDriver_Mm1NarrowPort1MemoryRead(  uBaseAddress, 
                                                     uAddress, 
                                                     pData0, 
                                                     pData1); 
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfCaMmNarrowPort1MemoryEntry_Unpack(&MmNarrowPort1MemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaMmNarrowPort1MemoryEntry_SetField(&MmNarrowPort1MemoryRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaMmNarrowPort1MemoryEntry_Pack(&MmNarrowPort1MemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000ImfDriver_Mm1NarrowPort1MemoryWrite(  uBaseAddress, 
                                                     uAddress, 
                                                     *pData0, 
                                                     *pData1); 

        }
    break;
    case FE2K_MEM_MM1WIDEPORTMEMORY:
        status =
        sbFe2000ImfDriver_Mm1WidePortMemoryRead(  uBaseAddress, 
                                                  uAddress,  
                                                  pData0,   
                                                  pData1);  
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfCaMmWidePortMemoryEntry_Unpack(&MmWidePortMemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaMmWidePortMemoryEntry_SetField(&MmWidePortMemoryRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaMmWidePortMemoryEntry_Pack(&MmWidePortMemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000ImfDriver_Mm1WidePortMemoryWrite(  uBaseAddress, 
                                                  uAddress,  
                                                  *pData0,   
                                                  *pData1);  

        }
    break;
    case FE2K_MEM_PB0DATA:
        status =
        sbFe2000ImfDriver_Pb0DataRead(  uBaseAddress,        
                                        uAddress,            
                                        pData0,             
                                        pData1,             
                                        pData2,             
                                        pData3,             
                                        pData4,             
                                        pData5,             
                                        pData6,             
                                        pData7,             
                                        pData8,             
                                        pData9,             
                                        pData10,            
                                        pData11,            
                                        pData12,            
                                        pData13,            
                                        pData14,            
                                        pData15,            
                                        pData16,            
                                        pData17,            
                                        pData18,            
                                        pData19,            
                                        pData20,            
                                        pData21,            
                                        pData22,            
                                        pData23);           
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,24);
          sbZfCaPbDataEntry_Unpack(&PbDataRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPbDataEntry_SetField(&PbDataRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPbDataEntry_Pack(&PbDataRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,24);
          status =
          sbFe2000ImfDriver_Pb0DataWrite(  uBaseAddress,        
                                        uAddress,            
                                        *pData0,             
                                        *pData1,             
                                        *pData2,             
                                        *pData3,             
                                        *pData4,             
                                        *pData5,             
                                        *pData6,             
                                        *pData7,             
                                        *pData8,             
                                        *pData9,             
                                        *pData10,            
                                        *pData11,            
                                        *pData12,            
                                        *pData13,            
                                        *pData14,            
                                        *pData15,            
                                        *pData16,            
                                        *pData17,            
                                        *pData18,            
                                        *pData19,            
                                        *pData20,            
                                        *pData21,            
                                        *pData22,            
                                        *pData23);         
        }
    break;
    case FE2K_MEM_PB1DATA:
        status =
        sbFe2000ImfDriver_Pb1DataRead(  uBaseAddress,        
                                        uAddress,            
                                        pData0,             
                                        pData1,             
                                        pData2,             
                                        pData3,             
                                        pData4,             
                                        pData5,             
                                        pData6,             
                                        pData7,             
                                        pData8,             
                                        pData9,             
                                        pData10,            
                                        pData11,            
                                        pData12,            
                                        pData13,            
                                        pData14,            
                                        pData15,            
                                        pData16,            
                                        pData17,            
                                        pData18,            
                                        pData19,            
                                        pData20,            
                                        pData21,            
                                        pData22,            
                                        pData23);           
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,24);
          sbZfCaPbDataEntry_Unpack(&PbDataRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPbDataEntry_SetField(&PbDataRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPbDataEntry_Pack(&PbDataRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,24);
          status =
          sbFe2000ImfDriver_Pb1DataWrite(  uBaseAddress,        
                                        uAddress,            
                                        *pData0,             
                                        *pData1,             
                                        *pData2,             
                                        *pData3,             
                                        *pData4,             
                                        *pData5,             
                                        *pData6,             
                                        *pData7,             
                                        *pData8,             
                                        *pData9,             
                                        *pData10,            
                                        *pData11,            
                                        *pData12,            
                                        *pData13,            
                                        *pData14,            
                                        *pData15,            
                                        *pData16,            
                                        *pData17,            
                                        *pData18,            
                                        *pData19,            
                                        *pData20,            
                                        *pData21,            
                                        *pData22,            
                                        *pData23);           

        }
    break;
    case FE2K_MEM_PTMIRRORINDEX:
        status =
        sbFe2000ImfDriver_PtMirrorIndexRead(  uBaseAddress,  
                                              uAddress,      
                                              pData);       
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfCaPtMirrorIndexEntry_Unpack(&PtMirrorIndexRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPtMirrorIndexEntry_SetField(&PtMirrorIndexRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPtMirrorIndexEntry_Pack(&PtMirrorIndexRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000ImfDriver_PtMirrorIndexWrite(  uBaseAddress,  
                                              uAddress,      
                                              *pData);    
        }
    break;
    case FE2K_MEM_PTPORTTOQUEUEAG0:
        status =
        sbFe2000ImfDriver_PtPortToQueueAg0Read(  uBaseAddress, 
                                                 uAddress,   
                                                 pData);    
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfCaPtPortToQueueEntry_Unpack(&PtPortToQueueRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPtPortToQueueEntry_SetField(&PtPortToQueueRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPtPortToQueueEntry_Pack(&PtPortToQueueRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000ImfDriver_PtPortToQueueAg0Write(  uBaseAddress, 
                                                 uAddress,   
                                                 *pData);    
        }
    break;
    case FE2K_MEM_PTPORTTOQUEUEAG1:
        status =
        sbFe2000ImfDriver_PtPortToQueueAg1Read(  uBaseAddress, 
                                                 uAddress,   
                                                 pData);    
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfCaPtPortToQueueEntry_Unpack(&PtPortToQueueRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPtPortToQueueEntry_SetField(&PtPortToQueueRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPtPortToQueueEntry_Pack(&PtPortToQueueRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000ImfDriver_PtPortToQueueAg1Write(  uBaseAddress, 
                                                 uAddress,   
                                                 *pData);    
        }
    break;
/* MCM - ? use PrPortXXX_Unpack - or - us PtPortToQueueEntry_Pack ??? */
    case FE2K_MEM_PTPORTTOQUEUEPCI:
        status =
        sbFe2000ImfDriver_PtPortToQueuePciRead(  uBaseAddress, 
                                                 uAddress,   
                                                 pData);    
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfCaPrPortToQueuePciEntry_Unpack(&PrPortToQueuePciRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPrPortToQueuePciEntry_SetField(&PrPortToQueuePciRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPrPortToQueuePciEntry_Pack(&PrPortToQueuePciRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000ImfDriver_PtPortToQueuePciWrite(  uBaseAddress, 
                                                 uAddress,   
                                                 *pData);    
        }
    break;
    case FE2K_MEM_PTPORTTOQUEUEST0:
        status =
        sbFe2000ImfDriver_PtPortToQueueSt0Read(  uBaseAddress, 
                                                 uAddress,   
                                                 pData);    
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfCaPtPortToQueueEntry_Unpack(&PtPortToQueueRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPtPortToQueueEntry_SetField(&PtPortToQueueRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPtPortToQueueEntry_Pack(&PtPortToQueueRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000ImfDriver_PtPortToQueueSt0Write(  uBaseAddress, 
                                                 uAddress,   
                                                 *pData);    
        }
    break;
    case FE2K_MEM_PTPORTTOQUEUEST1:
        status =
        sbFe2000ImfDriver_PtPortToQueueSt1Read(  uBaseAddress, 
                                                 uAddress,   
                                                 pData);    
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfCaPtPortToQueueEntry_Unpack(&PtPortToQueueRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPtPortToQueueEntry_SetField(&PtPortToQueueRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPtPortToQueueEntry_Pack(&PtPortToQueueRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000ImfDriver_PtPortToQueueSt1Write(  uBaseAddress, 
                                                 uAddress,   
                                                 *pData);    
        }
    break;
    case FE2K_MEM_PTPORTTOQUEUEXG0:
        status =
        sbFe2000ImfDriver_PtPortToQueueXg0Read(  uBaseAddress, 
                                                         uAddress,   
                                                 pData);    
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfCaPtPortToQueueEntry_Unpack(&PtPortToQueueRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPtPortToQueueEntry_SetField(&PtPortToQueueRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPtPortToQueueEntry_Pack(&PtPortToQueueRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000ImfDriver_PtPortToQueueXg0Write(  uBaseAddress, 
                                                         uAddress,   
                                                 *pData);    
        }
    break;
    case FE2K_MEM_PTPORTTOQUEUEXG1:
        status =
        sbFe2000ImfDriver_PtPortToQueueXg1Read(  uBaseAddress, 
                                                 uAddress,   
                                                 pData);    
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfCaPtPortToQueueEntry_Unpack(&PtPortToQueueRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPtPortToQueueEntry_SetField(&PtPortToQueueRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPtPortToQueueEntry_Pack(&PtPortToQueueRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000ImfDriver_PtPortToQueueXg1Write(  uBaseAddress, 
                                                 uAddress,   
                                                 *pData);    
        }
    break;
    case FE2K_MEM_PTPTE0MIRRORPORTSTATE:
        status =
        sbFe2000ImfDriver_PtPte0MirrorPortStateRead(  uBaseAddress, 
                                                      uAddress, 
                                                      pData0, 
                                                      pData1); 
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfCaPtPte0MirrorPortStateEntry_Unpack(&PtPte0MirrorPortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPtPte0MirrorPortStateEntry_SetField(&PtPte0MirrorPortStateRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPtPte0MirrorPortStateEntry_Pack(&PtPte0MirrorPortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000ImfDriver_PtPte0MirrorPortStateWrite(  uBaseAddress, 
                                                      uAddress, 
                                                      *pData0, 
                                                      *pData1); 
        }
    break;
    case FE2K_MEM_PTPTE0PORTSTATE:
        status =
        sbFe2000ImfDriver_PtPte0PortStateRead(  uBaseAddress, 
                                                uAddress,    
                                                pData0,     
                                                pData1,     
                                                pData2);    
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,3);
          sbZfCaPtPte0PortStateEntry_Unpack(&PtPte0PortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPtPte0PortStateEntry_SetField(&PtPte0PortStateRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPtPte0PortStateEntry_Pack(&PtPte0PortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,3);
          status =
          sbFe2000ImfDriver_PtPte0PortStateWrite(  uBaseAddress, 
                                                uAddress,    
                                                *pData0,     
                                                *pData1,     
                                                *pData2);    
        }
    break;
    case FE2K_MEM_PTPTE1MIRRORPORTSTATE:
        status =
        sbFe2000ImfDriver_PtPte1MirrorPortStateRead(  uBaseAddress, 
                                                      uAddress, 
                                                      pData0, 
                                                      pData1); 
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfCaPtPte1MirrorPortStateEntry_Unpack(&PtPte1MirrorPortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPtPte1MirrorPortStateEntry_SetField(&PtPte1MirrorPortStateRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPtPte1MirrorPortStateEntry_Pack(&PtPte1MirrorPortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000ImfDriver_PtPte1MirrorPortStateWrite(  uBaseAddress, 
                                                        uAddress, 
                                                              *pData0, 
                                                      *pData1); 
        }
    break;
    case FE2K_MEM_PTPTE1PORTSTATE:
        status =
        sbFe2000ImfDriver_PtPte1PortStateRead(  uBaseAddress, 
                                                uAddress,    
                                                pData0,     
                                                pData1,     
                                                pData2);    
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,3);
          sbZfCaPtPte1PortStateEntry_Unpack(&PtPte1PortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPtPte1PortStateEntry_SetField(&PtPte1PortStateRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPtPte1PortStateEntry_Pack(&PtPte1PortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,3);
          status =
          sbFe2000ImfDriver_PtPte1PortStateWrite(  uBaseAddress, 
                                                uAddress,    
                                                *pData0,     
                                                *pData1,     
                                                *pData2);    
        }
    break;
    case FE2K_MEM_PTPTE2MIRRORPORTSTATE:
        status =
        sbFe2000ImfDriver_PtPte2MirrorPortStateRead(  uBaseAddress, 
                                                      uAddress, 
                                                      pData0, 
                                                      pData1); 
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfCaPtPte2MirrorPortStateEntry_Unpack(&PtPte2MirrorPortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPtPte2MirrorPortStateEntry_SetField(&PtPte2MirrorPortStateRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPtPte2MirrorPortStateEntry_Pack(&PtPte2MirrorPortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000ImfDriver_PtPte2MirrorPortStateWrite(  uBaseAddress, 
                                                      uAddress, 
                                                      *pData0, 
                                                      *pData1); 
        }
    break;
    case FE2K_MEM_PTPTE2PORTSTATE:
        status =
        sbFe2000ImfDriver_PtPte2PortStateRead(  uBaseAddress, 
                                                uAddress,    
                                                pData0,     
                                                pData1,     
                                                pData2);    
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,3);
          sbZfCaPtPte2PortStateEntry_Unpack(&PtPte2PortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPtPte2PortStateEntry_SetField(&PtPte2PortStateRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPtPte2PortStateEntry_Pack(&PtPte2PortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,3);
          status =
          sbFe2000ImfDriver_PtPte2PortStateWrite(  uBaseAddress, 
                                                uAddress,    
                                                *pData0,     
                                                *pData1,     
                                                *pData2);    
        }
    break;
    case FE2K_MEM_PTPTE3MIRRORPORTSTATE:
        status =
        sbFe2000ImfDriver_PtPte3MirrorPortStateRead(  uBaseAddress, 
                                                      uAddress, 
                                                      pData0, 
                                                      pData1); 
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfCaPtPte3MirrorPortStateEntry_Unpack(&PtPte3MirrorPortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPtPte3MirrorPortStateEntry_SetField(&PtPte3MirrorPortStateRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPtPte3MirrorPortStateEntry_Pack(&PtPte3MirrorPortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000ImfDriver_PtPte3MirrorPortStateWrite(  uBaseAddress, 
                                                      uAddress, 
                                                      *pData0, 
                                                      *pData1); 
        }
    break;
    case FE2K_MEM_PTPTE3PORTSTATE:
        status =
        sbFe2000ImfDriver_PtPte3PortStateRead(  uBaseAddress, 
                                                uAddress,    
                                                pData0,     
                                                pData1,     
                                                pData2);    
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,3);
          sbZfCaPtPte3PortStateEntry_Unpack(&PtPte3PortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPtPte3PortStateEntry_SetField(&PtPte3PortStateRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPtPte3PortStateEntry_Pack(&PtPte3PortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,3);
          status =
          sbFe2000ImfDriver_PtPte3PortStateWrite(  uBaseAddress, 
                                                uAddress,    
                                                *pData0,     
                                                *pData1,     
                                                *pData2);    
        }
    break;
    case FE2K_MEM_PTPTE4MIRRORPORTSTATE:
        status =
        sbFe2000ImfDriver_PtPte4MirrorPortStateRead(  uBaseAddress, 
                                                      uAddress, 
                                                      pData0, 
                                                      pData1); 
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfCaPtPte4MirrorPortStateEntry_Unpack(&PtPte4MirrorPortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPtPte4MirrorPortStateEntry_SetField(&PtPte4MirrorPortStateRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPtPte4MirrorPortStateEntry_Pack(&PtPte4MirrorPortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000ImfDriver_PtPte4MirrorPortStateWrite(  uBaseAddress, 
                                                      uAddress, 
                                                      *pData0, 
                                                      *pData1); 
        }
    break;
    case FE2K_MEM_PTPTE4PORTSTATE:
        status =
        sbFe2000ImfDriver_PtPte4PortStateRead(  uBaseAddress, 
                                                uAddress,    
                                                pData0,     
                                                pData1,     
                                                pData2);    
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,3);
          sbZfCaPtPte4PortStateEntry_Unpack(&PtPte4PortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPtPte4PortStateEntry_SetField(&PtPte4PortStateRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPtPte4PortStateEntry_Pack(&PtPte4PortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,3);
          status =
          sbFe2000ImfDriver_PtPte4PortStateWrite(  uBaseAddress, 
                                                uAddress,    
                                                *pData0,     
                                                *pData1,     
                                                *pData2);    
        }
    break;
    case FE2K_MEM_PTPTE5MIRRORPORTSTATE:
        status =
        sbFe2000ImfDriver_PtPte5MirrorPortStateRead(  uBaseAddress, 
                                                      uAddress, 
                                                      pData0, 
                                                      pData1); 
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfCaPtPte5MirrorPortStateEntry_Unpack(&PtPte5MirrorPortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPtPte5MirrorPortStateEntry_SetField(&PtPte5MirrorPortStateRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPtPte5MirrorPortStateEntry_Pack(&PtPte5MirrorPortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000ImfDriver_PtPte5MirrorPortStateWrite(  uBaseAddress, 
                                                      uAddress, 
                                                      *pData0, 
                                                      *pData1); 
        }
    break;
    case FE2K_MEM_PTPTE5PORTSTATE:
        status =
        sbFe2000ImfDriver_PtPte5PortStateRead(  uBaseAddress, 
                                                uAddress,    
                                                pData0,     
                                                pData1,     
                                                pData2);    
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,3);
          sbZfCaPtPte5PortStateEntry_Unpack(&PtPte5PortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPtPte5PortStateEntry_SetField(&PtPte5PortStateRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPtPte5PortStateEntry_Pack(&PtPte5PortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,3);
          status =
          sbFe2000ImfDriver_PtPte5PortStateWrite(  uBaseAddress, 
                                                uAddress,    
                                                *pData0,     
                                                *pData1,     
                                                *pData2);    
        }
    break;
    case FE2K_MEM_PTPTE6MIRRORPORTSTATE:
        status =
        sbFe2000ImfDriver_PtPte6MirrorPortStateRead(  uBaseAddress, 
                                                      uAddress, 
                                                      pData0, 
                                                      pData1); 
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfCaPtPte6MirrorPortStateEntry_Unpack(&PtPte6MirrorPortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPtPte6MirrorPortStateEntry_SetField(&PtPte6MirrorPortStateRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPtPte6MirrorPortStateEntry_Pack(&PtPte6MirrorPortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000ImfDriver_PtPte6MirrorPortStateWrite(  uBaseAddress, 
                                                      uAddress, 
                                                      *pData0, 
                                                      *pData1); 
        }
    break;
    case FE2K_MEM_PTPTE6PORTSTATE:
        status =
        sbFe2000ImfDriver_PtPte6PortStateRead(  uBaseAddress, 
                                                uAddress,    
                                                pData0,     
                                                pData1,     
                                                pData2);    
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,3);
          sbZfCaPtPte6PortStateEntry_Unpack(&PtPte6PortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPtPte6PortStateEntry_SetField(&PtPte6PortStateRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPtPte6PortStateEntry_Pack(&PtPte6PortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,3);
          status =
          sbFe2000ImfDriver_PtPte6PortStateWrite(  uBaseAddress, 
                                                uAddress,    
                                                *pData0,     
                                                *pData1,     
                                                *pData2);    
        }
    break;
    case FE2K_MEM_PTQUEUETOPORT:
        status =
        sbFe2000ImfDriver_PtQueueToPortRead(  uBaseAddress,  
                                              uAddress,      
                                              pData);       
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfCaPtQueueToPortEntry_Unpack(&PtQueueToPortRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfCaPtQueueToPortEntry_SetField(&PtQueueToPortRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfCaPtQueueToPortEntry_Pack(&PtQueueToPortRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000ImfDriver_PtQueueToPortWrite(  uBaseAddress,  
                                              uAddress,      
                                              *pData);       
        }
    break;
    default:
        status = SAND_DRV_CA_STATUS_OK;
    break;
    }
    
    if (status != SAND_DRV_CA_STATUS_OK) {
        printk("Error %d while reading Fe2000 memory\n",status);
    } 
    return 0;
}


/*
 * sbFe2000MemSetField - find memory and field names and assign value
 *	inputs:	unit number
 *		name of memory 
 *	output:	prints memory contents
 * 	return:	OK on success, else error status
 */
int
sbFe2000MemSetTestAll(int unit, char *tempName, int addr, int val)
{
    int i, x, memindex, status=SAND_DRV_CA_STATUS_OK, doingfield;
    char *dptr, *memFieldPtr, curch, memname[100], memfield[100];
    for(x = 0; x < 410; x++) {
        memFieldPtr = memberList[x];
        if(strcmp(memFieldPtr,"END") == 0)
            return status;
       /* memfield comes in as mem.field, so split memname and field name */
        memfield[0] = 'm'; /* don't make user type 'm_u' for each field */
        memfield[1] = '_';
        memfield[2] = 'u';
        doingfield = 0;    /* needed to convert field name to lower chars */
        dptr = &memname[0];
        for(i = 0; i < 100; i++) {
            curch = memFieldPtr[i];
            if (curch == '\0') {
                *dptr = '\0';
                break;
            }
            if (curch == '.') {
                *dptr = '\0';
                dptr = &memfield[3];
                doingfield = 1;
            }
            else {
                if (doingfield)
                    *dptr++ = tolower((unsigned char)curch);
                else
                    *dptr++ = curch;
            }
        }
        printk("MemSetField mem= <%s>  field=<%s> addr=%d val=0x%x\n",
                      memname,&memfield[3],addr,val);

        memindex = sbFe2000WhichMem(memname);
        if(memindex == -1) {
            printk("Error: unrecognized Fe2000 memory: %s\n",memname);
        }
        status = sbFe2000MemSetFieldEntry(unit, memindex,addr,memfield, val);
        if(status == CMD_USAGE) {
            printk("Error: unrecognized field <%s> for this Fe2000 memory: %s\n",&memfield[3],memname);
        }
    }
    return status;
}


/*
 * sbFe2000MemSetField - find memory and field names and assign value
 *	inputs:	unit number
 *		name of memory 
 *	output:	prints memory contents
 * 	return:	OK on success, else error status
 */
int
sbFe2000MemSetField(int unit, char *memFieldName, int addr, int val)
{
    int i, memindex, status, doingfield, max;
    char *dptr, curch, memname[100], memfield[100];

    if (memFieldName[0] == '*') {
        return sbFe2000MemSetTestAll( unit, memFieldName,  addr,  val);
    }
   /* memfield comes in as mem.field, so split memname and field name */
    memfield[0] = 'm'; /* don't make user type 'm_u' for each field */
    memfield[1] = '_';
    memfield[2] = 'u';
    doingfield = 0;    /* needed to convert field name to lower chars */
    dptr = &memname[0];
    for(i = 0; i < 100; i++) {
        curch = memFieldName[i];
        if (memFieldName[i] == '\0') {
            *dptr = '\0';
            break;
        }
        if (curch == '.') {
            *dptr = '\0';
            dptr = &memfield[3];
            doingfield = 1;
        }
        else {
            if (doingfield)
                *dptr++ = tolower((unsigned char)curch);
            else
                *dptr++ = curch;
        }
    }
    printk("MemSetField unit=%d mem= <%s>  field=<%s> val=0x%x\n",
             unit,memname,&memfield[3],val);

    memindex = sbFe2000WhichMem(memname);
    if(memindex == -1) {
        printk("Error: unrecognized Fe2000 memory: %s\n",memname);
        printk("Please pick from one of these:\n");
        sbFe2000ShowMemNames();
        return CMD_FAIL;
    }
    max = sbFe2000MemMax(memindex);
    if (addr > max){
        printk("Error: Addr %d out of range for memory: %s max=%d\n",
                           addr,memname,max);
        return CMD_FAIL;
    }
    status = sbFe2000MemSetFieldEntry(unit, memindex,addr,memfield, val);
    if(status == CMD_USAGE) {
        printk("Error: unrecognized field <%s> for this Fe2000 memory: %s\n",&memfield[3],memname);
    }
    return status;
}

#endif /* BCM_FE2000_SUPPORT */
