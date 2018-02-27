
/*
 * $Id: sbFe2000XtMemWrite.c 1.10 Broadcom SDK $
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
#include <soc/sbx/fe2kxt/fe2kxt.h>
 
#include <soc/sbx/fe2kxt/sbZfC2Includes.h>
#include <soc/sbx/fe2kxt/sbFe2000XtImfDriver.h>

extern int sbFe2000XtWhichMem(char *memname);
extern int sbFe2000XtMemMax(int memindex);
extern  void sbFe2000XtShowMemNames(void);
extern void sbSwapWords(uint *pdata, int nwords);

char *memberListXt[] =
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
 * sbFe2000XtMemShowEntry - read and display contents of fe2000 indirect memory
 *	inputs:	unit number
 *		name of memory 
 *	output:	prints memory contents
 * 	return:	OK on success, else error status
 */
int
sbFe2000XtMemSetFieldEntry(int unit, int memindex, int addr, char *fieldname, int val)
{
  int uBaseAddress,bClearOnRead = 0;
  int  status;
  uint32_t last = 0;

#ifdef CAD_UINT
#undef CAD_UINT
#define CAD_UINT unsigned int
#endif

    sbZfC2PrPortToQueueAg0Entry_t   PrPortToQueueAg0Record;
    sbZfC2PrPortToQueueAg1Entry_t   PrPortToQueueAg1Record;
    sbZfC2PrPortToQueuePciEntry_t   PrPortToQueuePciRecord;
    sbZfC2PrPortToQueueSr0Entry_t   PrPortToQueueSr0Record;
    sbZfC2PrPortToQueueSr1Entry_t   PrPortToQueueSr1Record;
    sbZfC2PrPortToQueueXg0Entry_t   PrPortToQueueXg0Record;
    sbZfC2PrPortToQueueXg1Entry_t   PrPortToQueueXg1Record;
    sbZfC2PrCcTcamConfigEntry_t     PrCcTcamConfigEntry;
    sbZfC2PrCcCamRamNonLastEntry_t  PrCcCamRamNonLastEntry;
    sbZfC2PrCcCamRamLastEntry_t     PrCcCamRamLastEntry;
    sbZfC2PrCcQueueRamEntry_t       PrCcQueueRamEntry;
    sbZfC2PdLrpOutputHeaderCopyBufferEntry_t   PdLrpOutputHeaderCopyBufferRecord;
    sbZfC2PdPdOutputHeaderCopyBufferEntry_t   PdPdOutputHeaderCopyBufferRecord;
    sbZfC2LrLrpInstructionMemoryEntry_t   LrLrpInstructionMemoryRecord;
    sbZfC2QmBufferState0Entry_t   QmBufferState0Record;
    sbZfC2QmBufferState1Entry_t   QmBufferState1Record;
    sbZfC2QmBufferState2Entry_t   QmBufferState2Record;
    sbZfC2QmBufferState3Entry_t   QmBufferState3Record;
    sbZfC2QmDequeueState0Entry_t   QmDequeueState0Record;
    sbZfC2QmDequeueState1Entry_t   QmDequeueState1Record;
    sbZfC2QmFreePageFifoEntry_t    QmFreePageFifoRecord;
    sbZfC2QmNextBufferEntry_t      QmNextBufferRecord;
    sbZfC2QmNextPageEntry_t        QmNextPageRecord;
    sbZfC2QmQueueConfigEntry_t     QmQueueConfigRecord;
    sbZfC2QmQueueCountersEntry_t   QmQueueCountersRecord;
    sbZfC2QmQueueHeadPtrEntry_t    QmQueueHeadPtrRecord;
    sbZfC2QmQueueState0Entry_t   QmQueueState0Record;
    sbZfC2QmQueueState0EnEntry_t   QmQueueState0EnRecord;
    sbZfC2QmQueueState1Entry_t   QmQueueState1Record;
    sbZfC2QmReplicationStateEntry_t   QmReplicationStateRecord;
    sbZfC2PbCounterEntry_t   PbCounterRecord;
    sbZfC2PmProfileMemoryEntry_t   PmProfileMemoryRecord;
    sbZfC2SrCounterEntry_t   SrCounterRecord;
    sbZfC2StCounterEntry_t   StCounterRecord;
    sbZfC2PpAggregateHashBitConfigEntry_t   PpAggregateHashBitConfigRecord;
    sbZfC2PpAggregateHashByteConfigEntry_t   PpAggregateHashByteConfigRecord;
    sbZfC2PpCamConfigurationInstance0Entry_t   PpCamConfigurationInstance0Record;
    sbZfC2PpCamRamConfigurationInstance0Entry_t   PpCamRamConfigurationInstance0Record;
/*    sbZfC2PpCamRamConfigurationInstance1Entry_t   PpCamRamConfigurationInstance1Record;
    sbZfC2PpCamRamConfigurationInstance2Entry_t   PpCamRamConfigurationInstance2Record;
    sbZfC2PpCamRamConfigurationInstance3Entry_t   PpCamRamConfigurationInstance3Record;
*/
    sbZfC2PpHeaderRecordSizeEntry_t   PpHeaderRecordSizeRecord;
    sbZfC2PpInitialQueueStateEntry_t   PpInitialQueueStateRecord;
    sbZfC2PpPpOutHeaderCopyEntry_t   PpPpOutHeaderCopyRecord;
    sbZfC2PpQueuePriorityGroupEntry_t   PpQueuePriorityGroupRecord;
    sbZfC2PpRxPortDataEntry_t   PpRxPortDataRecord;
    sbZfC2RcDataEntry_t         RcDataRecord;
    sbZfC2MmInternal0MemoryEntry_t   MmInternal0MemoryRecord;
    sbZfC2MmInternal1MemoryEntry_t   MmInternal1MemoryRecord;
    sbZfC2MmNarrowPort0MemoryEntry_t   MmNarrowPort0MemoryRecord;
    sbZfC2MmNarrowPort1MemoryEntry_t   MmNarrowPort1MemoryRecord;
    sbZfC2MmWidePortMemoryEntry_t      MmWidePortMemoryRecord;
    sbZfC2PbDataEntry_t   PbDataRecord;
    sbZfC2PtMirrorIndexEntry_t   PtMirrorIndexRecord;
    sbZfC2PtPortToQueueEntry_t      PtPortToQueueRecord;
/* MCM - possibly need specific records fpr Pt Unpack ???
 * PtPort structure same as PrPort ???
    sbZfC2PrPortToQueueAg0Entry_t   PtPortToQueueAg0Record;
    sbZfC2PrPortToQueueAg1Entry_t   PtPortToQueueAg1Record;
    sbZfC2PrPortToQueuePciEntry_t   PtPortToQueuePciRecord;
    sbZfC2PrPortToQueueSr0Entry_t   PtPortToQueueSr0Record;
    sbZfC2PrPortToQueueSr1Entry_t   PtPortToQueueSr1Record;
    sbZfC2PrPortToQueueXg0Entry_t   PtPortToQueueXg0Record;
    sbZfC2PrPortToQueueXg1Entry_t   PtPortToQueueXg1Record;
*/
    sbZfC2PtMirrorPortStateEntry_t   PtPte0MirrorPortStateRecord;
    sbZfC2PtPortStateEntry_t   PtPte0PortStateRecord;
    sbZfC2PtMirrorPortStateEntry_t   PtPte1MirrorPortStateRecord;
    sbZfC2PtPortStateEntry_t   PtPte1PortStateRecord;
    sbZfC2PtMirrorPortStateEntry_t   PtPte2MirrorPortStateRecord;
    sbZfC2PtPortStateEntry_t   PtPte2PortStateRecord;
    sbZfC2PtMirrorPortStateEntry_t   PtPte3MirrorPortStateRecord;
    sbZfC2PtPortStateEntry_t   PtPte3PortStateRecord;
    sbZfC2PtMirrorPortStateEntry_t   PtPte4MirrorPortStateRecord;
    sbZfC2PtPortStateEntry_t   PtPte4PortStateRecord;
    sbZfC2PtMirrorPortStateEntry_t   PtPte5MirrorPortStateRecord;
    sbZfC2PtPortStateEntry_t   PtPte5PortStateRecord;
    sbZfC2PtMirrorPortStateEntry_t   PtPte6MirrorPortStateRecord;
    sbZfC2PtPortStateEntry_t   PtPte6PortStateRecord;
    sbZfC2PtQueueToPortEntry_t   PtQueueToPortRecord;

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
                                *pData35 = &dbuf[35],
                                *pData36 = &dbuf[36],
                                *pData37 = &dbuf[37],
                                *pData38 = &dbuf[38],
                                *pData39 = &dbuf[39];

    uBaseAddress = unit;
    uAddress = addr;

    switch(memindex) {
    case FE2KXT_MEM_PRPORTTOQUEUEAG0:
        status =
        sbFe2000XtImfDriver_PrPortToQueueAg0Read(  uBaseAddress, 
                                                 uAddress,   
                                                 pData);    

        if (status == SAND_DRV_C2_STATUS_OK) {

          sbSwapWords(pData,1);
          sbZfC2PrPortToQueueAg0Entry_Unpack(&PrPortToQueueAg0Record,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PrPortToQueueAg0Entry_SetField(&PrPortToQueueAg0Record,fieldname,val); 
          if(status != 0) return CMD_USAGE;
          sbZfC2PrPortToQueueAg0Entry_Pack(&PrPortToQueueAg0Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000XtImfDriver_PrPortToQueueAg0Write( uBaseAddress, 
                                                 uAddress,   
                                                 *pData);    
        }
    break;
    case FE2KXT_MEM_PRPORTTOQUEUEAG1:
        status =
        sbFe2000XtImfDriver_PrPortToQueueAg1Read(  uBaseAddress, 
                                                 uAddress,   
                                                 pData);    
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfC2PrPortToQueueAg1Entry_Unpack(&PrPortToQueueAg1Record,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PrPortToQueueAg1Entry_SetField(&PrPortToQueueAg1Record,fieldname,val); 
          if(status != 0) return CMD_USAGE;
          sbZfC2PrPortToQueueAg1Entry_Pack(&PrPortToQueueAg1Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000XtImfDriver_PrPortToQueueAg1Write(  uBaseAddress, 
                                                 uAddress,   
                                                 *pData);    
        }
    break;
    case FE2KXT_MEM_PRPORTTOQUEUEPCI:
        status =
        sbFe2000XtImfDriver_PrPortToQueuePciRead(  uBaseAddress, 
                                                 uAddress,   
                                                 pData);    
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfC2PrPortToQueuePciEntry_Unpack(&PrPortToQueuePciRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PrPortToQueuePciEntry_SetField(&PrPortToQueuePciRecord,fieldname,val); 
          if(status != 0) return CMD_USAGE;
          sbZfC2PrPortToQueuePciEntry_Pack(&PrPortToQueuePciRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000XtImfDriver_PrPortToQueuePciWrite(  uBaseAddress, 
                                                 uAddress,   
                                                 *pData);    
        }
    break;
    case FE2KXT_MEM_PRPORTTOQUEUESR0:
        status =
        sbFe2000XtImfDriver_PrPortToQueueSr0Read(  uBaseAddress, 
                                                 uAddress,   
                                                 pData);    
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfC2PrPortToQueueSr0Entry_Unpack(&PrPortToQueueSr0Record,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PrPortToQueueSr0Entry_SetField(&PrPortToQueueSr0Record,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PrPortToQueueSr0Entry_Pack(&PrPortToQueueSr0Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000XtImfDriver_PrPortToQueueSr0Write(  uBaseAddress, 
                                                 uAddress,   
                                                 *pData);    
        }
    break;
    case FE2KXT_MEM_PRPORTTOQUEUESR1:
        status =
        sbFe2000XtImfDriver_PrPortToQueueSr1Read(  uBaseAddress, 
                                                 uAddress,   
                                                 pData);    
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfC2PrPortToQueueSr1Entry_Unpack(&PrPortToQueueSr1Record,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PrPortToQueueSr1Entry_SetField(&PrPortToQueueSr1Record,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PrPortToQueueSr1Entry_Pack(&PrPortToQueueSr1Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000XtImfDriver_PrPortToQueueSr1Write(  uBaseAddress, 
                                                 uAddress,   
                                                 *pData);    
        }
    break;
    case FE2KXT_MEM_PRPORTTOQUEUEXG0:
        status =
        sbFe2000XtImfDriver_PrPortToQueueXg0Read(  uBaseAddress, 
                                                 uAddress,   
                                                 pData);    
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfC2PrPortToQueueXg0Entry_Unpack(&PrPortToQueueXg0Record,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PrPortToQueueXg0Entry_SetField(&PrPortToQueueXg0Record,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PrPortToQueueXg0Entry_Pack(&PrPortToQueueXg0Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000XtImfDriver_PrPortToQueueXg0Write(  uBaseAddress, 
                                                 uAddress,   
                                                 *pData);    
        }
    break;
    case FE2KXT_MEM_PRPORTTOQUEUEXG1:
        status =
        sbFe2000XtImfDriver_PrPortToQueueXg1Read(  uBaseAddress, 
                                                 uAddress,   
                                                 pData);    
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfC2PrPortToQueueXg1Entry_Unpack(&PrPortToQueueXg1Record,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PrPortToQueueXg1Entry_SetField(&PrPortToQueueXg1Record,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PrPortToQueueXg1Entry_Pack(&PrPortToQueueXg1Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000XtImfDriver_PrPortToQueueXg1Write(  uBaseAddress, 
                                                 uAddress,   
                                                 *pData);    
        }
    break;
    case FE2KXT_MEM_PRCC2TCAMCONFIG:
        status =
        sbFe2000XtImfDriver_PrCc2TcamConfigRead(  uBaseAddress, 
                                                 uAddress,   
                                                 &(pData[0]),
                                                 &(pData[1]),
                                                 &(pData[2]),
                                                 &(pData[3]),
                                                 &(pData[4]),
                                                 &(pData[5]),
                                                 &(pData[6]),
                                                 &(pData[7]),
                                                 &(pData[8]),
                                                 &(pData[9]),
                                                 &(pData[10]),
                                                 &(pData[11]),
                                                 &(pData[12]),
                                                 &(pData[13]));
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,14);
          sbZfC2PrCcTcamConfigEntry_Unpack(&PrCcTcamConfigEntry,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PrCcTcamConfigEntry_SetField(&PrCcTcamConfigEntry,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PrCcTcamConfigEntry_Pack(&PrCcTcamConfigEntry,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,14);
          status =
          sbFe2000XtImfDriver_PrCc2TcamConfigWrite(  uBaseAddress, 
                                                 uAddress,   
                                                 pData[0],
                                                 pData[1],
                                                 pData[2],
                                                 pData[3],
                                                 pData[4],
                                                 pData[5],
                                                 pData[6],
                                                 pData[7],
                                                 pData[8],
                                                 pData[9],
                                                 pData[10],
                                                 pData[11],
                                                 pData[12],
                                                 pData[13]);    
        }
    break;
    case FE2KXT_MEM_PRCC3TCAMCONFIG:
        status =
        sbFe2000XtImfDriver_PrCc3TcamConfigRead(  uBaseAddress, 
                                                 uAddress,   
                                                 &(pData[0]),
                                                 &(pData[1]),
                                                 &(pData[2]),
                                                 &(pData[3]),
                                                 &(pData[4]),
                                                 &(pData[5]),
                                                 &(pData[6]),
                                                 &(pData[7]),
                                                 &(pData[8]),
                                                 &(pData[9]),
                                                 &(pData[10]),
                                                 &(pData[11]),
                                                 &(pData[12]),
                                                 &(pData[13]));
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,14);
          sbZfC2PrCcTcamConfigEntry_Unpack(&PrCcTcamConfigEntry,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PrCcTcamConfigEntry_SetField(&PrCcTcamConfigEntry,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PrCcTcamConfigEntry_Pack(&PrCcTcamConfigEntry,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,14);
          status =
          sbFe2000XtImfDriver_PrCc3TcamConfigWrite(  uBaseAddress, 
                                                 uAddress,   
                                                 pData[0],
                                                 pData[1],
                                                 pData[2],
                                                 pData[3],
                                                 pData[4],
                                                 pData[5],
                                                 pData[6],
                                                 pData[7],
                                                 pData[8],
                                                 pData[9],
                                                 pData[10],
                                                 pData[11],
                                                 pData[12],
                                                 pData[13]);    
        }
    break;
    case FE2KXT_MEM_PRCC4TCAMCONFIG:
        status =
        sbFe2000XtImfDriver_PrCc4TcamConfigRead(  uBaseAddress, 
                                                 uAddress,   
                                                 &(pData[0]),
                                                 &(pData[1]),
                                                 &(pData[2]),
                                                 &(pData[3]),
                                                 &(pData[4]),
                                                 &(pData[5]),
                                                 &(pData[6]),
                                                 &(pData[7]),
                                                 &(pData[8]),
                                                 &(pData[9]),
                                                 &(pData[10]),
                                                 &(pData[11]),
                                                 &(pData[12]),
                                                 &(pData[13]));
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,14);
          sbZfC2PrCcTcamConfigEntry_Unpack(&PrCcTcamConfigEntry,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PrCcTcamConfigEntry_SetField(&PrCcTcamConfigEntry,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PrCcTcamConfigEntry_Pack(&PrCcTcamConfigEntry,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,14);
          status =
          sbFe2000XtImfDriver_PrCc4TcamConfigWrite(  uBaseAddress, 
                                                 uAddress,   
                                                 pData[0],
                                                 pData[1],
                                                 pData[2],
                                                 pData[3],
                                                 pData[4],
                                                 pData[5],
                                                 pData[6],
                                                 pData[7],
                                                 pData[8],
                                                 pData[9],
                                                 pData[10],
                                                 pData[11],
                                                 pData[12],
                                                 pData[13]);    
        }
    break;
    case FE2KXT_MEM_PRCC5TCAMCONFIG:
        status =
        sbFe2000XtImfDriver_PrCc5TcamConfigRead(  uBaseAddress, 
                                                 uAddress,   
                                                 &(pData[0]),
                                                 &(pData[1]),
                                                 &(pData[2]),
                                                 &(pData[3]),
                                                 &(pData[4]),
                                                 &(pData[5]),
                                                 &(pData[6]),
                                                 &(pData[7]),
                                                 &(pData[8]),
                                                 &(pData[9]),
                                                 &(pData[10]),
                                                 &(pData[11]),
                                                 &(pData[12]),
                                                 &(pData[13]));
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,14);
          sbZfC2PrCcTcamConfigEntry_Unpack(&PrCcTcamConfigEntry,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PrCcTcamConfigEntry_SetField(&PrCcTcamConfigEntry,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PrCcTcamConfigEntry_Pack(&PrCcTcamConfigEntry,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,14);
          status =
          sbFe2000XtImfDriver_PrCc5TcamConfigWrite(  uBaseAddress, 
                                                 uAddress,   
                                                 pData[0],
                                                 pData[1],
                                                 pData[2],
                                                 pData[3],
                                                 pData[4],
                                                 pData[5],
                                                 pData[6],
                                                 pData[7],
                                                 pData[8],
                                                 pData[9],
                                                 pData[10],
                                                 pData[11],
                                                 pData[12],
                                                 pData[13]);    
        }
    break;
    case FE2KXT_MEM_PRCC2CAMRAM:
        status =
        sbFe2000XtImfDriver_PrCc2CamRamRead(  uBaseAddress, 
                                                 uAddress,   
                                                 &(pData[0]),
                                                 &(pData[1]),
                                                 &(pData[2]));
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,SB_ZF_C2PRCCCAMRAMLASTENTRY_SIZE_IN_WORDS);
          last = 0;
          sbZfC2PrCcCamRamNonLastEntry_Unpack(&PrCcCamRamNonLastEntry,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PrCcCamRamNonLastEntry_SetField(&PrCcCamRamNonLastEntry,fieldname,val);
          if(status != 0) {
              last = 1;
              sbZfC2PrCcCamRamLastEntry_Unpack(&PrCcCamRamLastEntry,(uint8_t *)dbuf,sizeof(dbuf));
              status = sbZfC2PrCcCamRamLastEntry_SetField(&PrCcCamRamLastEntry,fieldname,val);
              if(status != 0) return CMD_USAGE;
          }
          if (!last){
            sbZfC2PrCcCamRamNonLastEntry_Pack(&PrCcCamRamNonLastEntry,(uint8_t *)dbuf,sizeof(dbuf));
          }else{
            sbZfC2PrCcCamRamLastEntry_Pack(&PrCcCamRamLastEntry,(uint8_t *)dbuf,sizeof(dbuf));
          }
          sbSwapWords(pData,SB_ZF_C2PRCCCAMRAMLASTENTRY_SIZE_IN_WORDS);
          status =
          sbFe2000XtImfDriver_PrCc2CamRamWrite(  uBaseAddress, 
                                                 uAddress,   
                                                 pData[0],
                                                 pData[1],
                                                 pData[2]);
        }
    break;
    case FE2KXT_MEM_PRCC3CAMRAM:
        status =
        sbFe2000XtImfDriver_PrCc3CamRamRead(  uBaseAddress, 
                                                 uAddress,   
                                                 &(pData[0]),
                                                 &(pData[1]),
                                                 &(pData[2]));
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,SB_ZF_C2PRCCCAMRAMLASTENTRY_SIZE_IN_WORDS);
          last = 0;
          sbZfC2PrCcCamRamNonLastEntry_Unpack(&PrCcCamRamNonLastEntry,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PrCcCamRamNonLastEntry_SetField(&PrCcCamRamNonLastEntry,fieldname,val);
          if(status != 0) {
              last = 1;
              sbZfC2PrCcCamRamLastEntry_Unpack(&PrCcCamRamLastEntry,(uint8_t *)dbuf,sizeof(dbuf));
              status = sbZfC2PrCcCamRamLastEntry_SetField(&PrCcCamRamLastEntry,fieldname,val);
              if(status != 0) return CMD_USAGE;
          }
          if (!last){
            sbZfC2PrCcCamRamNonLastEntry_Pack(&PrCcCamRamNonLastEntry,(uint8_t *)dbuf,sizeof(dbuf));
          }else{
            sbZfC2PrCcCamRamLastEntry_Pack(&PrCcCamRamLastEntry,(uint8_t *)dbuf,sizeof(dbuf));
          }
          sbSwapWords(pData,SB_ZF_C2PRCCCAMRAMLASTENTRY_SIZE_IN_WORDS);
          status =
          sbFe2000XtImfDriver_PrCc3CamRamWrite(  uBaseAddress, 
                                                 uAddress,   
                                                 pData[0],
                                                 pData[1],
                                                 pData[2]);
        }
    break;
    case FE2KXT_MEM_PRCC4CAMRAM:
        status =
        sbFe2000XtImfDriver_PrCc4CamRamRead(  uBaseAddress, 
                                                 uAddress,   
                                                 &(pData[0]),
                                                 &(pData[1]),
                                                 &(pData[2]));
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,SB_ZF_C2PRCCCAMRAMLASTENTRY_SIZE_IN_WORDS);
          last = 0;
          sbZfC2PrCcCamRamNonLastEntry_Unpack(&PrCcCamRamNonLastEntry,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PrCcCamRamNonLastEntry_SetField(&PrCcCamRamNonLastEntry,fieldname,val);
          if(status != 0) {
              last = 1;
              sbZfC2PrCcCamRamLastEntry_Unpack(&PrCcCamRamLastEntry,(uint8_t *)dbuf,sizeof(dbuf));
              status = sbZfC2PrCcCamRamLastEntry_SetField(&PrCcCamRamLastEntry,fieldname,val);
              if(status != 0) return CMD_USAGE;
          }
          if (!last){
            sbZfC2PrCcCamRamNonLastEntry_Pack(&PrCcCamRamNonLastEntry,(uint8_t *)dbuf,sizeof(dbuf));
          }else{
            sbZfC2PrCcCamRamLastEntry_Pack(&PrCcCamRamLastEntry,(uint8_t *)dbuf,sizeof(dbuf));
          }
          sbSwapWords(pData,SB_ZF_C2PRCCCAMRAMLASTENTRY_SIZE_IN_WORDS);
          status =
          sbFe2000XtImfDriver_PrCc4CamRamWrite(  uBaseAddress, 
                                                 uAddress,   
                                                 pData[0],
                                                 pData[1],
                                                 pData[2]);
        }
    break;
    case FE2KXT_MEM_PRCC5CAMRAM:
        status =
        sbFe2000XtImfDriver_PrCc5CamRamRead(  uBaseAddress, 
                                                 uAddress,   
                                                 &(pData[0]),
                                                 &(pData[1]),
                                                 &(pData[2]));
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,SB_ZF_C2PRCCCAMRAMLASTENTRY_SIZE_IN_WORDS);
          last = 0;
          sbZfC2PrCcCamRamNonLastEntry_Unpack(&PrCcCamRamNonLastEntry,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PrCcCamRamNonLastEntry_SetField(&PrCcCamRamNonLastEntry,fieldname,val);
          if(status != 0) {
              last = 1;
              sbZfC2PrCcCamRamLastEntry_Unpack(&PrCcCamRamLastEntry,(uint8_t *)dbuf,sizeof(dbuf));
              status = sbZfC2PrCcCamRamLastEntry_SetField(&PrCcCamRamLastEntry,fieldname,val);
              if(status != 0) return CMD_USAGE;
          }
          if (!last){
            sbZfC2PrCcCamRamNonLastEntry_Pack(&PrCcCamRamNonLastEntry,(uint8_t *)dbuf,sizeof(dbuf));
          }else{
            sbZfC2PrCcCamRamLastEntry_Pack(&PrCcCamRamLastEntry,(uint8_t *)dbuf,sizeof(dbuf));
          }
          sbSwapWords(pData,SB_ZF_C2PRCCCAMRAMLASTENTRY_SIZE_IN_WORDS);
          status =
          sbFe2000XtImfDriver_PrCc5CamRamWrite(  uBaseAddress, 
                                                 uAddress,   
                                                 pData[0],
                                                 pData[1],
                                                 pData[2]);
        }
    break;
    case FE2KXT_MEM_PRCC2QUEUERAM:
        status =
        sbFe2000XtImfDriver_PrCc2QueueRamRead(  uBaseAddress, 
                                                 uAddress,   
                                                 &(pData[0]));
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,SB_ZF_C2PRCCQUEUERAMENTRY_SIZE_IN_WORDS);
          last = 0;
          sbZfC2PrCcQueueRamEntry_Unpack(&PrCcQueueRamEntry,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PrCcQueueRamEntry_SetField(&PrCcQueueRamEntry,fieldname,val);
          sbZfC2PrCcQueueRamEntry_Pack(&PrCcQueueRamEntry,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,SB_ZF_C2PRCCQUEUERAMENTRY_SIZE_IN_WORDS);
          status =
          sbFe2000XtImfDriver_PrCc2QueueRamWrite(  uBaseAddress, 
                                                 uAddress,   
                                                 pData[0]);
        }
    break;
    case FE2KXT_MEM_PRCC3QUEUERAM:
        status =
        sbFe2000XtImfDriver_PrCc3QueueRamRead(  uBaseAddress, 
                                                 uAddress,   
                                                 &(pData[0]));
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,SB_ZF_C2PRCCQUEUERAMENTRY_SIZE_IN_WORDS);
          last = 0;
          sbZfC2PrCcQueueRamEntry_Unpack(&PrCcQueueRamEntry,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PrCcQueueRamEntry_SetField(&PrCcQueueRamEntry,fieldname,val);
          sbZfC2PrCcQueueRamEntry_Pack(&PrCcQueueRamEntry,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,SB_ZF_C2PRCCQUEUERAMENTRY_SIZE_IN_WORDS);
          status =
          sbFe2000XtImfDriver_PrCc3QueueRamWrite(  uBaseAddress, 
                                                 uAddress,   
                                                 pData[0]);
        }
    break;
    case FE2KXT_MEM_PRCC4QUEUERAM:
        status =
        sbFe2000XtImfDriver_PrCc4QueueRamRead(  uBaseAddress, 
                                                 uAddress,   
                                                 &(pData[0]));
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,SB_ZF_C2PRCCQUEUERAMENTRY_SIZE_IN_WORDS);
          last = 0;
          sbZfC2PrCcQueueRamEntry_Unpack(&PrCcQueueRamEntry,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PrCcQueueRamEntry_SetField(&PrCcQueueRamEntry,fieldname,val);
          sbZfC2PrCcQueueRamEntry_Pack(&PrCcQueueRamEntry,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,SB_ZF_C2PRCCQUEUERAMENTRY_SIZE_IN_WORDS);
          status =
          sbFe2000XtImfDriver_PrCc4QueueRamWrite(  uBaseAddress, 
                                                 uAddress,   
                                                 pData[0]);
        }
    break;
    case FE2KXT_MEM_PRCC5QUEUERAM:
        status =
        sbFe2000XtImfDriver_PrCc5QueueRamRead(  uBaseAddress, 
                                                 uAddress,   
                                                 &(pData[0]));
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,SB_ZF_C2PRCCQUEUERAMENTRY_SIZE_IN_WORDS);
          last = 0;
          sbZfC2PrCcQueueRamEntry_Unpack(&PrCcQueueRamEntry,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PrCcQueueRamEntry_SetField(&PrCcQueueRamEntry,fieldname,val);
          sbZfC2PrCcQueueRamEntry_Pack(&PrCcQueueRamEntry,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,SB_ZF_C2PRCCQUEUERAMENTRY_SIZE_IN_WORDS);
          status =
          sbFe2000XtImfDriver_PrCc5QueueRamWrite(  uBaseAddress, 
                                                 uAddress,   
                                                 pData[0]);
        }
    break;
    case FE2KXT_MEM_PDLRPOUTPUTHEADERCOPYBUFFER:
        status =
        sbFe2000XtImfDriver_PdLrpOutputHeaderCopyBufferRead(  uBaseAddress, 
                                                            uAddress, 
                                                            pData0, 
                                                            pData1, 
                                                            pData2, 
                                                            pData3, 
                                                            pData4, 
                                                            pData5, 
                                                            pData6, 
                                                            pData7); 
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,8);
          sbZfC2PdLrpOutputHeaderCopyBufferEntry_Unpack(&PdLrpOutputHeaderCopyBufferRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PdLrpOutputHeaderCopyBufferEntry_SetField(&PdLrpOutputHeaderCopyBufferRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PdLrpOutputHeaderCopyBufferEntry_Pack(&PdLrpOutputHeaderCopyBufferRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,8);
          status =
          sbFe2000XtImfDriver_PdLrpOutputHeaderCopyBufferWrite(  uBaseAddress, 
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
    case FE2KXT_MEM_PDPDOUTPUTHEADERCOPYBUFFER:
        status =
        sbFe2000XtImfDriver_PdPdOutputHeaderCopyBufferRead(  uBaseAddress, 
                                                           uAddress, 
                                                           pData0, 
                                                           pData1, 
                                                           pData2, 
                                                           pData3, 
                                                           pData4, 
                                                           pData5, 
                                                           pData6, 
                                                           pData7); 
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,8);
          sbZfC2PdPdOutputHeaderCopyBufferEntry_Unpack(&PdPdOutputHeaderCopyBufferRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PdPdOutputHeaderCopyBufferEntry_SetField(&PdPdOutputHeaderCopyBufferRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PdPdOutputHeaderCopyBufferEntry_Pack(&PdPdOutputHeaderCopyBufferRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,8);
          status =
          sbFe2000XtImfDriver_PdPdOutputHeaderCopyBufferWrite(  uBaseAddress, 
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
    case FE2KXT_MEM_LRLRPINSTRUCTIONMEMORY:
        status =
        sbFe2000XtImfDriver_LrLrpInstructionMemoryRead(  uBaseAddress, 
                                                       uAddress, 
                                                       pData0, 
                                                       pData1, 
                                                       pData2); 
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,3);
          sbZfC2LrLrpInstructionMemoryEntry_Unpack(&LrLrpInstructionMemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2LrLrpInstructionMemoryEntry_SetField(&LrLrpInstructionMemoryRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2LrLrpInstructionMemoryEntry_Pack(&LrLrpInstructionMemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,3);
          status =
          sbFe2000XtImfDriver_LrLrpInstructionMemoryWrite(  uBaseAddress, 
                                                       uAddress, 
                                                       *pData0, 
                                                       *pData1, 
                                                       *pData2); 
        }
    break;
    case FE2KXT_MEM_QMBUFFERSTATE0:
        status =
        sbFe2000XtImfDriver_QmBufferState0Read(  uBaseAddress, 
                                               uAddress,     
                                               pData);      
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfC2QmBufferState0Entry_Unpack(&QmBufferState0Record,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2QmBufferState0Entry_SetField(&QmBufferState0Record,fieldname,val);
          if(status != 0) return CMD_USAGE;

          sbZfC2QmBufferState0Entry_Pack(&QmBufferState0Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000XtImfDriver_QmBufferState0Write(  uBaseAddress, 
                                               uAddress,     
                                               *pData);      
        }
    break;
    case FE2KXT_MEM_QMBUFFERSTATE1:
        status =
        sbFe2000XtImfDriver_QmBufferState1Read(  uBaseAddress, 
                                               uAddress,     
                                               pData0,      
                                               pData1);     
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfC2QmBufferState1Entry_Unpack(&QmBufferState1Record,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2QmBufferState1Entry_SetField(&QmBufferState1Record,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2QmBufferState1Entry_Pack(&QmBufferState1Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000XtImfDriver_QmBufferState1Write(  uBaseAddress, 
                                               uAddress,     
                                               *pData0,      
                                               *pData1);     
        }
    break;
    case FE2KXT_MEM_QMBUFFERSTATE2:
        status =
        sbFe2000XtImfDriver_QmBufferState2Read(  uBaseAddress, 
                                               uAddress,     
                                               pData);      
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfC2QmBufferState2Entry_Unpack(&QmBufferState2Record,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2QmBufferState2Entry_SetField(&QmBufferState2Record,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2QmBufferState2Entry_Pack(&QmBufferState2Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000XtImfDriver_QmBufferState2Write(  uBaseAddress, 
                                               uAddress,     
                                               *pData);      
        }
    break;
    case FE2KXT_MEM_QMBUFFERSTATE3:
        status =
        sbFe2000XtImfDriver_QmBufferState3Read(  uBaseAddress, 
                                               uAddress,     
                                               pData);      
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfC2QmBufferState3Entry_Unpack(&QmBufferState3Record,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2QmBufferState3Entry_SetField(&QmBufferState3Record,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2QmBufferState3Entry_Pack(&QmBufferState3Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000XtImfDriver_QmBufferState3Write(  uBaseAddress, 
                                               uAddress,     
                                               *pData);      
        }
    break;
/* MCM was missing DequeueState0 code */
    case FE2KXT_MEM_QMDEQUEUESTATE0:
        status =
        sbFe2000XtImfDriver_QmDequeueState0Read(  uBaseAddress, 
                                                uAddress,    
                                                pData0,     
                                                pData1);     
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfC2QmDequeueState0Entry_Unpack(&QmDequeueState0Record,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2QmDequeueState0Entry_SetField(&QmDequeueState0Record,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2QmDequeueState0Entry_Pack(&QmDequeueState0Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000XtImfDriver_QmDequeueState0Write(  uBaseAddress, 
                                                uAddress,    
                                                *pData0,     
                                                *pData1);     
        }
    break;

    case FE2KXT_MEM_QMDEQUEUESTATE1:
        status =
        sbFe2000XtImfDriver_QmDequeueState1Read(  uBaseAddress, 
                                                uAddress,    
                                                pData);     
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfC2QmDequeueState1Entry_Unpack(&QmDequeueState1Record,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2QmDequeueState1Entry_SetField(&QmDequeueState1Record,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2QmDequeueState1Entry_Pack(&QmDequeueState1Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000XtImfDriver_QmDequeueState1Write(  uBaseAddress, 
                                                uAddress,    
                                                *pData);     
        }
    break;
    case FE2KXT_MEM_QMFREEPAGEFIFO:
        status =
        sbFe2000XtImfDriver_QmFreePageFifoRead(  uBaseAddress, 
                                               uAddress,     
                                               pData);      
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfC2QmFreePageFifoEntry_Unpack(&QmFreePageFifoRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2QmFreePageFifoEntry_SetField(&QmFreePageFifoRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2QmFreePageFifoEntry_Pack(&QmFreePageFifoRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000XtImfDriver_QmFreePageFifoWrite(  uBaseAddress, 
                                               uAddress,     
                                               *pData);      
        }
    break;
    case FE2KXT_MEM_QMNEXTBUFFER:
        status =
        sbFe2000XtImfDriver_QmNextBufferRead(  uBaseAddress,   
                                             uAddress,       
                                             pData);        
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfC2QmNextBufferEntry_Unpack(&QmNextBufferRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2QmNextBufferEntry_SetField(&QmNextBufferRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2QmNextBufferEntry_Pack(&QmNextBufferRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000XtImfDriver_QmNextBufferWrite(  uBaseAddress,   
                                             uAddress,       
                                             *pData);        
        }
    break;
    case FE2KXT_MEM_QMNEXTPAGE:
        status =
        sbFe2000XtImfDriver_QmNextPageRead(  uBaseAddress,     
                                           uAddress,         
                                           pData);          
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfC2QmNextPageEntry_Unpack(&QmNextPageRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2QmNextPageEntry_SetField(&QmNextPageRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2QmNextPageEntry_Pack(&QmNextPageRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000XtImfDriver_QmNextPageWrite(  uBaseAddress,     
                                           uAddress,         
                                           *pData);          
        }
    break;
    case FE2KXT_MEM_QMQUEUECONFIG:
        status =
        sbFe2000XtImfDriver_QmQueueConfigRead(  uBaseAddress,  
                                              uAddress,      
                                              pData0,       
                                              pData1,       
                                              pData2);      
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,3);
          sbZfC2QmQueueConfigEntry_Unpack(&QmQueueConfigRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2QmQueueConfigEntry_SetField(&QmQueueConfigRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2QmQueueConfigEntry_Pack(&QmQueueConfigRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,3);
          status =
          sbFe2000XtImfDriver_QmQueueConfigWrite(  uBaseAddress,  
                                              uAddress,      
                                              *pData0,       
                                              *pData1,       
                                              *pData2);      
        }
    break;
    case FE2KXT_MEM_QMQUEUECOUNTERS:
        status =
        sbFe2000XtImfDriver_QmQueueCountersRead(  uBaseAddress, 
                                                uAddress,    
                                                pData0,     
                                                pData1);    
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfC2QmQueueCountersEntry_Unpack(&QmQueueCountersRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2QmQueueCountersEntry_SetField(&QmQueueCountersRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2QmQueueCountersEntry_Pack(&QmQueueCountersRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000XtImfDriver_QmQueueCountersWrite(  uBaseAddress, 
                                                uAddress,    
                                                *pData0,     
                                                *pData1);    
        }
    break;
    case FE2KXT_MEM_QMQUEUEHEADPTR:
        status =
        sbFe2000XtImfDriver_QmQueueHeadPtrRead(  uBaseAddress, 
                                               uAddress,     
                                               pData);      
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfC2QmQueueHeadPtrEntry_Unpack(&QmQueueHeadPtrRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2QmQueueHeadPtrEntry_SetField(&QmQueueHeadPtrRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2QmQueueHeadPtrEntry_Pack(&QmQueueHeadPtrRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000XtImfDriver_QmQueueHeadPtrWrite(  uBaseAddress, 
                                               uAddress,     
                                               *pData);      
        }
    break;
    case FE2KXT_MEM_QMQUEUESTATE0:
        status =
        sbFe2000XtImfDriver_QmQueueState0Read(  uBaseAddress,  
                                              uAddress,      
                                              pData);       
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfC2QmQueueState0Entry_Unpack(&QmQueueState0Record,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2QmQueueState0Entry_SetField(&QmQueueState0Record,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2QmQueueState0Entry_Pack(&QmQueueState0Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000XtImfDriver_QmQueueState0Write(  uBaseAddress,  
                                              uAddress,      
                                              *pData);       
        }
    break;
    case FE2KXT_MEM_QMQUEUESTATE0EN:
        status =
        sbFe2000XtImfDriver_QmQueueState0EnRead(  uBaseAddress, 
                                                uAddress,    
                                                pData);     
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfC2QmQueueState0EnEntry_Unpack(&QmQueueState0EnRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2QmQueueState0EnEntry_SetField(&QmQueueState0EnRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2QmQueueState0EnEntry_Pack(&QmQueueState0EnRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000XtImfDriver_QmQueueState0EnWrite(  uBaseAddress, 
                                                uAddress,    
                                                *pData);     
        }
    break;
    case FE2KXT_MEM_QMQUEUESTATE1:
        status =
        sbFe2000XtImfDriver_QmQueueState1Read(  uBaseAddress,  
                                              uAddress,      
                                              pData);       
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfC2QmQueueState1Entry_Unpack(&QmQueueState1Record,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2QmQueueState1Entry_SetField(&QmQueueState1Record,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2QmQueueState1Entry_Pack(&QmQueueState1Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000XtImfDriver_QmQueueState1Write(  uBaseAddress,  
                                              uAddress,      
                                              *pData);       
        }
    break;
    case FE2KXT_MEM_QMREPLICATIONSTATE:
        status =
        sbFe2000XtImfDriver_QmReplicationStateRead(  uBaseAddress, 
                                                   uAddress, 
                                                   pData);  
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfC2QmReplicationStateEntry_Unpack(&QmReplicationStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2QmReplicationStateEntry_SetField(&QmReplicationStateRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2QmReplicationStateEntry_Pack(&QmReplicationStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000XtImfDriver_QmReplicationStateWrite(  uBaseAddress, 
                                                   uAddress, 
                                                   *pData);  
        }
    break;
    case FE2KXT_MEM_PMCOUNTERMEMORY:
 /* MCM no PmCounter zframe */
        status =
        sbFe2000XtImfDriver_PmCounterMemoryRead(  uBaseAddress,
                                                uAddress,    
                                                bClearOnRead, 
                                                pData0,     
                                                pData1);    
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfC2PbCounterEntry_Unpack(&PbCounterRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PbCounterEntry_SetField(&PbCounterRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PbCounterEntry_Pack(&PbCounterRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000XtImfDriver_PmCounterMemoryWrite(  uBaseAddress,
                                                uAddress,    
                                                *pData0,     
                                                *pData1);    
        }
    break;
    case FE2KXT_MEM_PMPROFILEMEMORY:
        status =
        sbFe2000XtImfDriver_PmProfileMemoryRead(  uBaseAddress, 
                                                uAddress,    
                                                pData0,     
                                                pData1,
                                                pData2);    
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,3);
          sbZfC2PmProfileMemoryEntry_Unpack(&PmProfileMemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PmProfileMemoryEntry_SetField(&PmProfileMemoryRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PmProfileMemoryEntry_Pack(&PmProfileMemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,3);
          status =
          sbFe2000XtImfDriver_PmProfileMemoryWrite(  uBaseAddress, 
                                                uAddress,    
                                                *pData0,     
                                                *pData1,
                                                *pData2);
        }
    break;
    case FE2KXT_MEM_SR0COUNTER:
        status =
        sbFe2000XtImfDriver_Sr0CounterRead(  uBaseAddress,     
                                           uAddress,         
                                           bClearOnRead,     
                                           pData0,          
                                           pData1);         
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfC2SrCounterEntry_Unpack(&SrCounterRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2SrCounterEntry_SetField(&SrCounterRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2SrCounterEntry_Pack(&SrCounterRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000XtImfDriver_Sr0CounterWrite(  uBaseAddress,     
                                           uAddress,         
                                           *pData0,          
                                           *pData1);         
        }
    break;
    case FE2KXT_MEM_SR1COUNTER:
        status =
        sbFe2000XtImfDriver_Sr1CounterRead(  uBaseAddress,     
                                           uAddress,         
                                           bClearOnRead,     
                                           pData0,          
                                           pData1);         
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfC2SrCounterEntry_Unpack(&SrCounterRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2SrCounterEntry_SetField(&SrCounterRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2SrCounterEntry_Pack(&SrCounterRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000XtImfDriver_Sr1CounterWrite(  uBaseAddress,     
                                           uAddress,         
                                           *pData0,          
                                           *pData1);         
        }
    break;
    case FE2KXT_MEM_PPAGGREGATEHASHBITCONFIG:
        status =
        sbFe2000XtImfDriver_PpAggregateHashBitConfigRead(  uBaseAddress, 
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
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,10);
          sbZfC2PpAggregateHashBitConfigEntry_Unpack(&PpAggregateHashBitConfigRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PpAggregateHashBitConfigEntry_SetField(&PpAggregateHashBitConfigRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PpAggregateHashBitConfigEntry_Pack(&PpAggregateHashBitConfigRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,10);
          status =
          sbFe2000XtImfDriver_PpAggregateHashBitConfigWrite(  uBaseAddress, 
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
    case FE2KXT_MEM_PPAGGREGATEHASHBYTECONFIG:
        status =
        sbFe2000XtImfDriver_PpAggregateHashByteConfigRead(  uBaseAddress, 
                                                          uAddress, 
                                                          pData0, 
                                                          pData1); 
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfC2PpAggregateHashByteConfigEntry_Unpack(&PpAggregateHashByteConfigRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PpAggregateHashByteConfigEntry_SetField(&PpAggregateHashByteConfigRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PpAggregateHashByteConfigEntry_Pack(&PpAggregateHashByteConfigRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000XtImfDriver_PpAggregateHashByteConfigWrite(  uBaseAddress, 
                                                          uAddress, 
                                                          *pData0, 
                                                          *pData1); 
        }
    break;
    case FE2KXT_MEM_PPCAMCONFIGURATIONINSTANCE0:
        status =
        sbFe2000XtImfDriver_PpCamConfigurationInstance0Read(  uBaseAddress, 
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
                                                            pData13); 
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,14);
          sbZfC2PpCamConfigurationInstance0Entry_Unpack(&PpCamConfigurationInstance0Record,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PpCamConfigurationInstance0Entry_SetField(&PpCamConfigurationInstance0Record,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PpCamConfigurationInstance0Entry_Pack(&PpCamConfigurationInstance0Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,14);
          status =
          sbFe2000XtImfDriver_PpCamConfigurationInstance0Write(  uBaseAddress, 
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
                                                            *pData13); 

        }
    break;
    case FE2KXT_MEM_PPCAMCONFIGURATIONINSTANCE1:
/* MCM - all instances use Instance0 Unpack/Print because no zframe for
 * Instance1 - 4 */
        status =
        sbFe2000XtImfDriver_PpCamConfigurationInstance1Read(  uBaseAddress, 
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
                                                            pData13); 
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,14);
          sbZfC2PpCamConfigurationInstance0Entry_Unpack(&PpCamConfigurationInstance0Record,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PpCamConfigurationInstance0Entry_SetField(&PpCamConfigurationInstance0Record,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PpCamConfigurationInstance0Entry_Pack(&PpCamConfigurationInstance0Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,14);
          status =
          sbFe2000XtImfDriver_PpCamConfigurationInstance1Write(  uBaseAddress, 
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
                                                            *pData13); 

        }
    break;
    case FE2KXT_MEM_PPCAMCONFIGURATIONINSTANCE2:
        status =
        sbFe2000XtImfDriver_PpCamConfigurationInstance2Read(  uBaseAddress, 
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
                                                            pData13); 
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,14);
          sbZfC2PpCamConfigurationInstance0Entry_Unpack(&PpCamConfigurationInstance0Record,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PpCamConfigurationInstance0Entry_SetField(&PpCamConfigurationInstance0Record,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PpCamConfigurationInstance0Entry_Pack(&PpCamConfigurationInstance0Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,14);
          status =
          sbFe2000XtImfDriver_PpCamConfigurationInstance2Write(  uBaseAddress, 
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
                                                            *pData13); 

        }
    break;
    case FE2KXT_MEM_PPCAMCONFIGURATIONINSTANCE3:
        status =
        sbFe2000XtImfDriver_PpCamConfigurationInstance3Read(  uBaseAddress, 
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
                                                            pData13); 
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,14);
          sbZfC2PpCamConfigurationInstance0Entry_Unpack(&PpCamConfigurationInstance0Record,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PpCamConfigurationInstance0Entry_SetField(&PpCamConfigurationInstance0Record,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PpCamConfigurationInstance0Entry_Pack(&PpCamConfigurationInstance0Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,14);
          status =
          sbFe2000XtImfDriver_PpCamConfigurationInstance3Write(  uBaseAddress, 
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
                                                            *pData13); 

        }
    break;
    case FE2KXT_MEM_PPCAMRAMCONFIGURATIONINSTANCE0:
        status =
        sbFe2000XtImfDriver_PpCamRamConfigurationInstance0Read(  uBaseAddress, 
                                                               uAddress, 
                                                                       pData0, 
                                                               pData1, 
                                                               pData2, 
                                                               pData3, 
                                                               pData4, 
                                                               pData5); 
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,6);
          sbZfC2PpCamRamConfigurationInstance0Entry_Unpack(&PpCamRamConfigurationInstance0Record,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PpCamRamConfigurationInstance0Entry_SetField(&PpCamRamConfigurationInstance0Record,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PpCamRamConfigurationInstance0Entry_Pack(&PpCamRamConfigurationInstance0Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,6);
          status =
          sbFe2000XtImfDriver_PpCamRamConfigurationInstance0Write(  uBaseAddress, 
                                                               uAddress, 
                                                                       *pData0, 
                                                               *pData1, 
                                                               *pData2, 
                                                               *pData3, 
                                                               *pData4, 
                                                               *pData5); 

        }
    break;
    case FE2KXT_MEM_PPCAMRAMCONFIGURATIONINSTANCE1:
        status =
        sbFe2000XtImfDriver_PpCamRamConfigurationInstance1Read(  uBaseAddress, 
                                                               uAddress, 
                                                               pData0, 
                                                               pData1, 
                                                               pData2, 
                                                               pData3, 
                                                               pData4, 
                                                               pData5); 
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,6);
          sbZfC2PpCamRamConfigurationInstance0Entry_Unpack(&PpCamRamConfigurationInstance0Record,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PpCamRamConfigurationInstance0Entry_SetField(&PpCamRamConfigurationInstance0Record,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PpCamRamConfigurationInstance0Entry_Pack(&PpCamRamConfigurationInstance0Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,6);
          status =
          sbFe2000XtImfDriver_PpCamRamConfigurationInstance1Write(  uBaseAddress, 
                                                               uAddress, 
                                                               *pData0, 
                                                               *pData1, 
                                                               *pData2, 
                                                               *pData3, 
                                                               *pData4, 
                                                               *pData5); 

        }
    break;
    case FE2KXT_MEM_PPCAMRAMCONFIGURATIONINSTANCE2:
        status =
        sbFe2000XtImfDriver_PpCamRamConfigurationInstance2Read(  uBaseAddress, 
                                                               uAddress, 
                                                               pData0, 
                                                               pData1, 
                                                               pData2, 
                                                               pData3, 
                                                               pData4, 
                                                               pData5); 
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,6);
          sbZfC2PpCamRamConfigurationInstance0Entry_Unpack(&PpCamRamConfigurationInstance0Record,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PpCamRamConfigurationInstance0Entry_SetField(&PpCamRamConfigurationInstance0Record,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PpCamRamConfigurationInstance0Entry_Pack(&PpCamRamConfigurationInstance0Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,6);
          status =
          sbFe2000XtImfDriver_PpCamRamConfigurationInstance2Write(  uBaseAddress, 
                                                               uAddress, 
                                                               *pData0, 
                                                               *pData1, 
                                                               *pData2, 
                                                               *pData3, 
                                                               *pData4, 
                                                               *pData5); 

        }
    break;
    case FE2KXT_MEM_PPCAMRAMCONFIGURATIONINSTANCE3:
        status =
        sbFe2000XtImfDriver_PpCamRamConfigurationInstance3Read(  uBaseAddress, 
                                                               uAddress, 
                                                               pData0, 
                                                               pData1, 
                                                               pData2, 
                                                               pData3, 
                                                               pData4, 
                                                               pData5); 
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,6);
          sbZfC2PpCamRamConfigurationInstance0Entry_Unpack(&PpCamRamConfigurationInstance0Record,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PpCamRamConfigurationInstance0Entry_SetField(&PpCamRamConfigurationInstance0Record,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PpCamRamConfigurationInstance0Entry_Pack(&PpCamRamConfigurationInstance0Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,6);
          status =
          sbFe2000XtImfDriver_PpCamRamConfigurationInstance3Write(  uBaseAddress, 
                                                               uAddress, 
                                                               *pData0, 
                                                               *pData1, 
                                                               *pData2, 
                                                               *pData3, 
                                                               *pData4, 
                                                               *pData5); 

        }
    break;
    case FE2KXT_MEM_PPHEADERRECORDSIZE:
        status =
        sbFe2000XtImfDriver_PpHeaderRecordSizeRead(  uBaseAddress, 
                                                   uAddress, 
                                                   pData);  
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfC2PpHeaderRecordSizeEntry_Unpack(&PpHeaderRecordSizeRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PpHeaderRecordSizeEntry_SetField(&PpHeaderRecordSizeRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PpHeaderRecordSizeEntry_Pack(&PpHeaderRecordSizeRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000XtImfDriver_PpHeaderRecordSizeWrite(  uBaseAddress, 
                                                   uAddress, 
                                                   *pData);  
        }
    break;
    case FE2KXT_MEM_PPINITIALQUEUESTATE:
        status =
        sbFe2000XtImfDriver_PpInitialQueueStateRead(  uBaseAddress, 
                                                    uAddress, 
                                                    pData0, 
                                                    pData1, 
                                                    pData2, 
                                                    pData3, 
                                                    pData4); 
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,5);
          sbZfC2PpInitialQueueStateEntry_Unpack(&PpInitialQueueStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PpInitialQueueStateEntry_SetField(&PpInitialQueueStateRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PpInitialQueueStateEntry_Pack(&PpInitialQueueStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,5);
          status =
          sbFe2000XtImfDriver_PpInitialQueueStateWrite(  uBaseAddress, 
                                                    uAddress, 
                                                    *pData0, 
                                                    *pData1, 
                                                    *pData2, 
                                                    *pData3, 
                                                    *pData4); 

        }
    break;
    case FE2KXT_MEM_PPPPOUTHEADERCOPY:
        status =
        sbFe2000XtImfDriver_PpPpOutHeaderCopyRead(  uBaseAddress, 
                                                  uAddress,  
                                                  pData0,   
                                                  pData1,   
                                                  pData2,   
                                                  pData3,   
                                                  pData4,   
                                                  pData5,   
                                                  pData6,   
                                                  pData7);  
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,8);
          sbZfC2PpPpOutHeaderCopyEntry_Unpack(&PpPpOutHeaderCopyRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PpPpOutHeaderCopyEntry_SetField(&PpPpOutHeaderCopyRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PpPpOutHeaderCopyEntry_Pack(&PpPpOutHeaderCopyRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,8);
          status =
          sbFe2000XtImfDriver_PpPpOutHeaderCopyWrite(  uBaseAddress, 
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
    case FE2KXT_MEM_PPQUEUEPRIORITYGROUP:
        status =
        sbFe2000XtImfDriver_PpQueuePriorityGroupRead(  uBaseAddress, 
                                                     uAddress, 
                                                     pData); 
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfC2PpQueuePriorityGroupEntry_Unpack(&PpQueuePriorityGroupRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PpQueuePriorityGroupEntry_SetField(&PpQueuePriorityGroupRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PpQueuePriorityGroupEntry_Pack(&PpQueuePriorityGroupRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000XtImfDriver_PpQueuePriorityGroupWrite(  uBaseAddress, 
                                                     uAddress, 
                                                     *pData); 
        }
    break;
    case FE2KXT_MEM_PPRXPORTDATA:
        status =
        sbFe2000XtImfDriver_PpRxPortDataRead(  uBaseAddress,   
                                             uAddress,       
                                             pData0,        
                                             pData1,        
                                             pData2,        
                                             pData3);       
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,4);
          sbZfC2PpRxPortDataEntry_Unpack(&PpRxPortDataRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PpRxPortDataEntry_SetField(&PpRxPortDataRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PpRxPortDataEntry_Pack(&PpRxPortDataRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,4);
          status =
          sbFe2000XtImfDriver_PpRxPortDataWrite(  uBaseAddress,   
                                             uAddress,       
                                             *pData0,        
                                             *pData1,        
                                             *pData2,        
                                             *pData3);    
        }
    break;
    case FE2KXT_MEM_RC0DATA:
        status =
        sbFe2000XtImfDriver_Rc0DataRead(  uBaseAddress,        
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
                                        pData35,            
                                        pData36,            
                                        pData37,            
                                        pData38,            
                                        pData39);           
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,40);
          sbZfC2RcDataEntry_Unpack(&RcDataRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2RcDataEntry_SetField(&RcDataRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2RcDataEntry_Pack(&RcDataRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,40);
          status =
          sbFe2000XtImfDriver_Rc0DataWrite(  uBaseAddress,        
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
                                        *pData35,            
                                        *pData36,            
                                        *pData37,            
                                        *pData38,            
                                        *pData39);      
        }
    break;
    case FE2KXT_MEM_RC1DATA:
        status =
        sbFe2000XtImfDriver_Rc1DataRead(  uBaseAddress,        
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
                                        pData35,            
                                        pData36,            
                                        pData37,            
                                        pData38,            
                                        pData39);           
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,40);
          sbZfC2RcDataEntry_Unpack(&RcDataRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2RcDataEntry_SetField(&RcDataRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2RcDataEntry_Pack(&RcDataRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,40);
          status =
          sbFe2000XtImfDriver_Rc1DataWrite(  uBaseAddress,        
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
                                        *pData35,            
                                        *pData36,            
                                        *pData37,            
                                        *pData38,            
                                        *pData39);          
        }
    break;
    case FE2KXT_MEM_ST0COUNTER:
        status =
        sbFe2000XtImfDriver_St0CounterRead(  uBaseAddress,     
                                           uAddress,         
                                           bClearOnRead,     
                                           pData0,          
                                           pData1);         
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfC2StCounterEntry_Unpack(&StCounterRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2StCounterEntry_SetField(&StCounterRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2StCounterEntry_Pack(&StCounterRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000XtImfDriver_St0CounterWrite(  uBaseAddress,     
                                           uAddress,         
                                           *pData0,          
                                           *pData1);    
        }
    break;
    case FE2KXT_MEM_ST1COUNTER:
        status =
        sbFe2000XtImfDriver_St1CounterRead(  uBaseAddress,     
                                           uAddress,         
                                           bClearOnRead,     
                                           pData0,          
                                           pData1);         
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfC2StCounterEntry_Unpack(&StCounterRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2StCounterEntry_SetField(&StCounterRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2StCounterEntry_Pack(&StCounterRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000XtImfDriver_St1CounterWrite(  uBaseAddress,     
                                           uAddress,         
                                           *pData0,          
                                           *pData1);   
        }
    break;
    case FE2KXT_MEM_MM0INTERNAL0MEMORY:
        status =
        sbFe2000XtImfDriver_Mm0Internal0MemoryRead(  uBaseAddress, 
                                                   uAddress, 
                                                   pData0,  
                                                   pData1); 
        if (status == SAND_DRV_C2_STATUS_OK) {
 printk("Mm0Internal0MemoryRead Read data0=0x%x data1=0x%x\n",*pData0, *pData1);
          sbSwapWords(pData,2);
 printk("Mm0Internal0MemoryRead SWAP data0=0x%x data1=0x%x\n",*pData0, *pData1);
          sbZfC2MmInternal0MemoryEntry_Unpack(&MmInternal0MemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2MmInternal0MemoryEntry_SetField(&MmInternal0MemoryRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2MmInternal0MemoryEntry_Pack(&MmInternal0MemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
 printk("Mm0Internal0MemoryRead UPdate data0=0x%x data1=0x%x\n",*pData0, *pData1);
          sbSwapWords(pData,2);
 printk("Mm0Internal0MemoryRead SwapUpdate data0=0x%x data1=0x%x\n",*pData0, *pData1);
          status =
          sbFe2000XtImfDriver_Mm0Internal0MemoryWrite(  uBaseAddress, 
                                                   uAddress, 
                                                   *pData0,  
                                                   *pData1); 

        }
    break;
    case FE2KXT_MEM_MM0INTERNAL1MEMORY:
        status =
        sbFe2000XtImfDriver_Mm0Internal1MemoryRead(  uBaseAddress, 
                                                   uAddress, 
                                                   pData0,  
                                                   pData1); 
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfC2MmInternal1MemoryEntry_Unpack(&MmInternal1MemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2MmInternal1MemoryEntry_SetField(&MmInternal1MemoryRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2MmInternal1MemoryEntry_Pack(&MmInternal1MemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000XtImfDriver_Mm0Internal1MemoryWrite(  uBaseAddress, 
                                                   uAddress, 
                                                   *pData0,  
                                                   *pData1); 

        }
    break;
    case FE2KXT_MEM_MM0NARROWPORT0MEMORY:
        status =
        sbFe2000XtImfDriver_Mm0NarrowPort0MemoryRead(  uBaseAddress, 
                                                     uAddress, 
                                                     pData0, 
                                                     pData1); 
        if (status == SAND_DRV_C2_STATUS_OK) {
printk("Mm0NarrowPort0MemoryRead Read data0=0x%x data1=0x%x\n",*pData0, *pData1);
          sbSwapWords(pData,2);
printk("Mm0NarrowPort0MemoryRead Swap data0=0x%x data1=0x%x\n",*pData0, *pData1);
          sbZfC2MmNarrowPort0MemoryEntry_Unpack(&MmNarrowPort0MemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2MmNarrowPort0MemoryEntry_SetField(&MmNarrowPort0MemoryRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2MmNarrowPort0MemoryEntry_Pack(&MmNarrowPort0MemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
printk("Mm0NarrowPort0MemoryRead UPDATdata0=0x%x data1=0x%x\n",*pData0, *pData1);
          sbSwapWords(pData,2);
printk("Mm0NarrowPort0MemoryRead UPDAT SWAPdata0=0x%x data1=0x%x\n",*pData0, *pData1);
          status =
          sbFe2000XtImfDriver_Mm0NarrowPort0MemoryWrite(  uBaseAddress, 
                                                     uAddress, 
                                                     *pData0, 
                                                     *pData1); 

        }
    break;
    case FE2KXT_MEM_MM0NARROWPORT1MEMORY:
        status =
        sbFe2000XtImfDriver_Mm0NarrowPort1MemoryRead(  uBaseAddress, 
                                                     uAddress, 
                                                     pData0, 
                                                     pData1); 
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfC2MmNarrowPort1MemoryEntry_Unpack(&MmNarrowPort1MemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2MmNarrowPort1MemoryEntry_SetField(&MmNarrowPort1MemoryRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2MmNarrowPort1MemoryEntry_Pack(&MmNarrowPort1MemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000XtImfDriver_Mm0NarrowPort1MemoryWrite(  uBaseAddress, 
                                                     uAddress, 
                                                     *pData0, 
                                                     *pData1); 

        }
    break;
    case FE2KXT_MEM_MM0WIDEPORTMEMORY:
        status =
        sbFe2000XtImfDriver_Mm0WidePortMemoryRead(  uBaseAddress, 
                                                  uAddress,  
                                                  pData0,   
                                                  pData1);  
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfC2MmWidePortMemoryEntry_Unpack(&MmWidePortMemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2MmWidePortMemoryEntry_SetField(&MmWidePortMemoryRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2MmWidePortMemoryEntry_Pack(&MmWidePortMemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000XtImfDriver_Mm0WidePortMemoryWrite(  uBaseAddress, 
                                                  uAddress,  
                                                  *pData0,   
                                                  *pData1);  

        }
    break;
    case FE2KXT_MEM_MM1INTERNAL0MEMORY:
        status =
        sbFe2000XtImfDriver_Mm1Internal0MemoryRead(  uBaseAddress, 
                                                   uAddress, 
                                                   pData0,  
                                                   pData1); 
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfC2MmInternal0MemoryEntry_Unpack(&MmInternal0MemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2MmInternal0MemoryEntry_SetField(&MmInternal0MemoryRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2MmInternal0MemoryEntry_Pack(&MmInternal0MemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000XtImfDriver_Mm1Internal0MemoryWrite(  uBaseAddress, 
                                                   uAddress, 
                                                   *pData0,  
                                                   *pData1); 

        }
    break;
    case FE2KXT_MEM_MM1INTERNAL1MEMORY:
        status =
        sbFe2000XtImfDriver_Mm1Internal1MemoryRead(  uBaseAddress, 
                                                   uAddress, 
                                                   pData0,  
                                                   pData1); 
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfC2MmInternal1MemoryEntry_Unpack(&MmInternal1MemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2MmInternal1MemoryEntry_SetField(&MmInternal1MemoryRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2MmInternal1MemoryEntry_Pack(&MmInternal1MemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000XtImfDriver_Mm1Internal1MemoryWrite(  uBaseAddress, 
                                                   uAddress, 
                                                   *pData0,  
                                                   *pData1); 

        }
    break;
    case FE2KXT_MEM_MM1NARROWPORT0MEMORY:
        status =
        sbFe2000XtImfDriver_Mm1NarrowPort0MemoryRead(  uBaseAddress, 
                                                     uAddress, 
                                                     pData0, 
                                                     pData1); 
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfC2MmNarrowPort0MemoryEntry_Unpack(&MmNarrowPort0MemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2MmNarrowPort0MemoryEntry_SetField(&MmNarrowPort0MemoryRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2MmNarrowPort0MemoryEntry_Pack(&MmNarrowPort0MemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000XtImfDriver_Mm1NarrowPort0MemoryWrite(  uBaseAddress, 
                                                     uAddress, 
                                                     *pData0, 
                                                     *pData1); 

        }
    break;
    case FE2KXT_MEM_MM1NARROWPORT1MEMORY:
        status =
        sbFe2000XtImfDriver_Mm1NarrowPort1MemoryRead(  uBaseAddress, 
                                                     uAddress, 
                                                     pData0, 
                                                     pData1); 
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfC2MmNarrowPort1MemoryEntry_Unpack(&MmNarrowPort1MemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2MmNarrowPort1MemoryEntry_SetField(&MmNarrowPort1MemoryRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2MmNarrowPort1MemoryEntry_Pack(&MmNarrowPort1MemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000XtImfDriver_Mm1NarrowPort1MemoryWrite(  uBaseAddress, 
                                                     uAddress, 
                                                     *pData0, 
                                                     *pData1); 

        }
    break;
    case FE2KXT_MEM_MM1WIDEPORTMEMORY:
        status =
        sbFe2000XtImfDriver_Mm1WidePortMemoryRead(  uBaseAddress, 
                                                  uAddress,  
                                                  pData0,   
                                                  pData1);  
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfC2MmWidePortMemoryEntry_Unpack(&MmWidePortMemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2MmWidePortMemoryEntry_SetField(&MmWidePortMemoryRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2MmWidePortMemoryEntry_Pack(&MmWidePortMemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000XtImfDriver_Mm1WidePortMemoryWrite(  uBaseAddress, 
                                                  uAddress,  
                                                  *pData0,   
                                                  *pData1);  

        }
    break;
    case FE2KXT_MEM_PB0DATA:
        status =
        sbFe2000XtImfDriver_Pb0DataRead(  uBaseAddress,        
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
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,24);
          sbZfC2PbDataEntry_Unpack(&PbDataRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PbDataEntry_SetField(&PbDataRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PbDataEntry_Pack(&PbDataRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,24);
          status =
          sbFe2000XtImfDriver_Pb0DataWrite(  uBaseAddress,        
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
    case FE2KXT_MEM_PB1DATA:
        status =
        sbFe2000XtImfDriver_Pb1DataRead(  uBaseAddress,        
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
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,24);
          sbZfC2PbDataEntry_Unpack(&PbDataRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PbDataEntry_SetField(&PbDataRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PbDataEntry_Pack(&PbDataRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,24);
          status =
          sbFe2000XtImfDriver_Pb1DataWrite(  uBaseAddress,        
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
    case FE2KXT_MEM_PTMIRRORINDEX:
        status =
        sbFe2000XtImfDriver_PtMirrorIndexRead(  uBaseAddress,  
                                              uAddress,      
                                              pData0,       
                                              pData1,       
                                              pData2,       
                                              pData3,       
                                              pData4);       
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,5);
          sbZfC2PtMirrorIndexEntry_Unpack(&PtMirrorIndexRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PtMirrorIndexEntry_SetField(&PtMirrorIndexRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PtMirrorIndexEntry_Pack(&PtMirrorIndexRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,5);
          status =
          sbFe2000XtImfDriver_PtMirrorIndexWrite(  uBaseAddress,  
                                              uAddress,      
                                              *pData0,    
                                              *pData1,    
                                              *pData2,    
                                              *pData3,    
                                              *pData4);    
        }
    break;
    case FE2KXT_MEM_PTPORTTOQUEUEAG0:
        status =
        sbFe2000XtImfDriver_PtPortToQueueAg0Read(  uBaseAddress, 
                                                 uAddress,   
                                                 pData);    
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfC2PtPortToQueueEntry_Unpack(&PtPortToQueueRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PtPortToQueueEntry_SetField(&PtPortToQueueRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PtPortToQueueEntry_Pack(&PtPortToQueueRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000XtImfDriver_PtPortToQueueAg0Write(  uBaseAddress, 
                                                 uAddress,   
                                                 *pData);    
        }
    break;
    case FE2KXT_MEM_PTPORTTOQUEUEAG1:
        status =
        sbFe2000XtImfDriver_PtPortToQueueAg1Read(  uBaseAddress, 
                                                 uAddress,   
                                                 pData);    
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfC2PtPortToQueueEntry_Unpack(&PtPortToQueueRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PtPortToQueueEntry_SetField(&PtPortToQueueRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PtPortToQueueEntry_Pack(&PtPortToQueueRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000XtImfDriver_PtPortToQueueAg1Write(  uBaseAddress, 
                                                 uAddress,   
                                                 *pData);    
        }
    break;
/* MCM - ? use PrPortXXX_Unpack - or - us PtPortToQueueEntry_Pack ??? */
    case FE2KXT_MEM_PTPORTTOQUEUEPCI:
        status =
        sbFe2000XtImfDriver_PtPortToQueuePciRead(  uBaseAddress, 
                                                 uAddress,   
                                                 pData);    
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfC2PrPortToQueuePciEntry_Unpack(&PrPortToQueuePciRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PrPortToQueuePciEntry_SetField(&PrPortToQueuePciRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PrPortToQueuePciEntry_Pack(&PrPortToQueuePciRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000XtImfDriver_PtPortToQueuePciWrite(  uBaseAddress, 
                                                 uAddress,   
                                                 *pData);    
        }
    break;
    case FE2KXT_MEM_PTPORTTOQUEUEST0:
        status =
        sbFe2000XtImfDriver_PtPortToQueueSt0Read(  uBaseAddress, 
                                                 uAddress,   
                                                 pData);    
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfC2PtPortToQueueEntry_Unpack(&PtPortToQueueRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PtPortToQueueEntry_SetField(&PtPortToQueueRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PtPortToQueueEntry_Pack(&PtPortToQueueRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000XtImfDriver_PtPortToQueueSt0Write(  uBaseAddress, 
                                                 uAddress,   
                                                 *pData);    
        }
    break;
    case FE2KXT_MEM_PTPORTTOQUEUEST1:
        status =
        sbFe2000XtImfDriver_PtPortToQueueSt1Read(  uBaseAddress, 
                                                 uAddress,   
                                                 pData);    
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfC2PtPortToQueueEntry_Unpack(&PtPortToQueueRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PtPortToQueueEntry_SetField(&PtPortToQueueRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PtPortToQueueEntry_Pack(&PtPortToQueueRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000XtImfDriver_PtPortToQueueSt1Write(  uBaseAddress, 
                                                 uAddress,   
                                                 *pData);    
        }
    break;
    case FE2KXT_MEM_PTPORTTOQUEUEXG0:
        status =
        sbFe2000XtImfDriver_PtPortToQueueXg0Read(  uBaseAddress, 
                                                         uAddress,   
                                                 pData);    
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfC2PtPortToQueueEntry_Unpack(&PtPortToQueueRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PtPortToQueueEntry_SetField(&PtPortToQueueRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PtPortToQueueEntry_Pack(&PtPortToQueueRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000XtImfDriver_PtPortToQueueXg0Write(  uBaseAddress, 
                                                         uAddress,   
                                                 *pData);    
        }
    break;
    case FE2KXT_MEM_PTPORTTOQUEUEXG1:
        status =
        sbFe2000XtImfDriver_PtPortToQueueXg1Read(  uBaseAddress, 
                                                 uAddress,   
                                                 pData);    
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfC2PtPortToQueueEntry_Unpack(&PtPortToQueueRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PtPortToQueueEntry_SetField(&PtPortToQueueRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PtPortToQueueEntry_Pack(&PtPortToQueueRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000XtImfDriver_PtPortToQueueXg1Write(  uBaseAddress, 
                                                 uAddress,   
                                                 *pData);    
        }
    break;
    case FE2KXT_MEM_PTPTE0MIRRORPORTSTATE:
        status =
        sbFe2000XtImfDriver_PtPte0MirrorPortStateRead(  uBaseAddress, 
                                                      uAddress, 
                                                      pData0, 
                                                      pData1); 
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfC2PtMirrorPortStateEntry_Unpack(&PtPte0MirrorPortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PtMirrorPortStateEntry_SetField(&PtPte0MirrorPortStateRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PtMirrorPortStateEntry_Pack(&PtPte0MirrorPortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000XtImfDriver_PtPte0MirrorPortStateWrite(  uBaseAddress, 
                                                      uAddress, 
                                                      *pData0, 
                                                      *pData1); 
        }
    break;
    case FE2KXT_MEM_PTPTE0PORTSTATE:
        status =
        sbFe2000XtImfDriver_PtPte0PortStateRead(  uBaseAddress, 
                                                uAddress,    
                                                pData0,     
                                                pData1,     
                                                pData2);    
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,3);
          sbZfC2PtPortStateEntry_Unpack(&PtPte0PortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PtPortStateEntry_SetField(&PtPte0PortStateRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PtPortStateEntry_Pack(&PtPte0PortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,3);
          status =
          sbFe2000XtImfDriver_PtPte0PortStateWrite(  uBaseAddress, 
                                                uAddress,    
                                                *pData0,     
                                                *pData1,     
                                                *pData2);    
        }
    break;
    case FE2KXT_MEM_PTPTE1MIRRORPORTSTATE:
        status =
        sbFe2000XtImfDriver_PtPte1MirrorPortStateRead(  uBaseAddress, 
                                                      uAddress, 
                                                      pData0, 
                                                      pData1); 
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfC2PtMirrorPortStateEntry_Unpack(&PtPte1MirrorPortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PtMirrorPortStateEntry_SetField(&PtPte1MirrorPortStateRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PtMirrorPortStateEntry_Pack(&PtPte1MirrorPortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000XtImfDriver_PtPte1MirrorPortStateWrite(  uBaseAddress, 
                                                        uAddress, 
                                                              *pData0, 
                                                      *pData1); 
        }
    break;
    case FE2KXT_MEM_PTPTE1PORTSTATE:
        status =
        sbFe2000XtImfDriver_PtPte1PortStateRead(  uBaseAddress, 
                                                uAddress,    
                                                pData0,     
                                                pData1,     
                                                pData2);    
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,3);
          sbZfC2PtPortStateEntry_Unpack(&PtPte1PortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PtPortStateEntry_SetField(&PtPte1PortStateRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PtPortStateEntry_Pack(&PtPte1PortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,3);
          status =
          sbFe2000XtImfDriver_PtPte1PortStateWrite(  uBaseAddress, 
                                                uAddress,    
                                                *pData0,     
                                                *pData1,     
                                                *pData2);    
        }
    break;
    case FE2KXT_MEM_PTPTE2MIRRORPORTSTATE:
        status =
        sbFe2000XtImfDriver_PtPte2MirrorPortStateRead(  uBaseAddress, 
                                                      uAddress, 
                                                      pData0, 
                                                      pData1); 
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfC2PtMirrorPortStateEntry_Unpack(&PtPte2MirrorPortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PtMirrorPortStateEntry_SetField(&PtPte2MirrorPortStateRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PtMirrorPortStateEntry_Pack(&PtPte2MirrorPortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000XtImfDriver_PtPte2MirrorPortStateWrite(  uBaseAddress, 
                                                      uAddress, 
                                                      *pData0, 
                                                      *pData1); 
        }
    break;
    case FE2KXT_MEM_PTPTE2PORTSTATE:
        status =
        sbFe2000XtImfDriver_PtPte2PortStateRead(  uBaseAddress, 
                                                uAddress,    
                                                pData0,     
                                                pData1,     
                                                pData2);    
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,3);
          sbZfC2PtPortStateEntry_Unpack(&PtPte2PortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PtPortStateEntry_SetField(&PtPte2PortStateRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PtPortStateEntry_Pack(&PtPte2PortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,3);
          status =
          sbFe2000XtImfDriver_PtPte2PortStateWrite(  uBaseAddress, 
                                                uAddress,    
                                                *pData0,     
                                                *pData1,     
                                                *pData2);    
        }
    break;
    case FE2KXT_MEM_PTPTE3MIRRORPORTSTATE:
        status =
        sbFe2000XtImfDriver_PtPte3MirrorPortStateRead(  uBaseAddress, 
                                                      uAddress, 
                                                      pData0, 
                                                      pData1); 
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfC2PtMirrorPortStateEntry_Unpack(&PtPte3MirrorPortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PtMirrorPortStateEntry_SetField(&PtPte3MirrorPortStateRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PtMirrorPortStateEntry_Pack(&PtPte3MirrorPortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000XtImfDriver_PtPte3MirrorPortStateWrite(  uBaseAddress, 
                                                      uAddress, 
                                                      *pData0, 
                                                      *pData1); 
        }
    break;
    case FE2KXT_MEM_PTPTE3PORTSTATE:
        status =
        sbFe2000XtImfDriver_PtPte3PortStateRead(  uBaseAddress, 
                                                uAddress,    
                                                pData0,     
                                                pData1,     
                                                pData2);    
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,3);
          sbZfC2PtPortStateEntry_Unpack(&PtPte3PortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PtPortStateEntry_SetField(&PtPte3PortStateRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PtPortStateEntry_Pack(&PtPte3PortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,3);
          status =
          sbFe2000XtImfDriver_PtPte3PortStateWrite(  uBaseAddress, 
                                                uAddress,    
                                                *pData0,     
                                                *pData1,     
                                                *pData2);    
        }
    break;
    case FE2KXT_MEM_PTPTE4MIRRORPORTSTATE:
        status =
        sbFe2000XtImfDriver_PtPte4MirrorPortStateRead(  uBaseAddress, 
                                                      uAddress, 
                                                      pData0, 
                                                      pData1); 
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfC2PtMirrorPortStateEntry_Unpack(&PtPte4MirrorPortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PtMirrorPortStateEntry_SetField(&PtPte4MirrorPortStateRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PtMirrorPortStateEntry_Pack(&PtPte4MirrorPortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000XtImfDriver_PtPte4MirrorPortStateWrite(  uBaseAddress, 
                                                      uAddress, 
                                                      *pData0, 
                                                      *pData1); 
        }
    break;
    case FE2KXT_MEM_PTPTE4PORTSTATE:
        status =
        sbFe2000XtImfDriver_PtPte4PortStateRead(  uBaseAddress, 
                                                uAddress,    
                                                pData0,     
                                                pData1,     
                                                pData2);    
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,3);
          sbZfC2PtPortStateEntry_Unpack(&PtPte4PortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PtPortStateEntry_SetField(&PtPte4PortStateRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PtPortStateEntry_Pack(&PtPte4PortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,3);
          status =
          sbFe2000XtImfDriver_PtPte4PortStateWrite(  uBaseAddress, 
                                                uAddress,    
                                                *pData0,     
                                                *pData1,     
                                                *pData2);    
        }
    break;
    case FE2KXT_MEM_PTPTE5MIRRORPORTSTATE:
        status =
        sbFe2000XtImfDriver_PtPte5MirrorPortStateRead(  uBaseAddress, 
                                                      uAddress, 
                                                      pData0, 
                                                      pData1); 
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfC2PtMirrorPortStateEntry_Unpack(&PtPte5MirrorPortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PtMirrorPortStateEntry_SetField(&PtPte5MirrorPortStateRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PtMirrorPortStateEntry_Pack(&PtPte5MirrorPortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000XtImfDriver_PtPte5MirrorPortStateWrite(  uBaseAddress, 
                                                      uAddress, 
                                                      *pData0, 
                                                      *pData1); 
        }
    break;
    case FE2KXT_MEM_PTPTE5PORTSTATE:
        status =
        sbFe2000XtImfDriver_PtPte5PortStateRead(  uBaseAddress, 
                                                uAddress,    
                                                pData0,     
                                                pData1,     
                                                pData2);    
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,3);
          sbZfC2PtPortStateEntry_Unpack(&PtPte5PortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PtPortStateEntry_SetField(&PtPte5PortStateRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PtPortStateEntry_Pack(&PtPte5PortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,3);
          status =
          sbFe2000XtImfDriver_PtPte5PortStateWrite(  uBaseAddress, 
                                                uAddress,    
                                                *pData0,     
                                                *pData1,     
                                                *pData2);    
        }
    break;
    case FE2KXT_MEM_PTPTE6MIRRORPORTSTATE:
        status =
        sbFe2000XtImfDriver_PtPte6MirrorPortStateRead(  uBaseAddress, 
                                                      uAddress, 
                                                      pData0, 
                                                      pData1); 
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,2);
          sbZfC2PtMirrorPortStateEntry_Unpack(&PtPte6MirrorPortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PtMirrorPortStateEntry_SetField(&PtPte6MirrorPortStateRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PtMirrorPortStateEntry_Pack(&PtPte6MirrorPortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,2);
          status =
          sbFe2000XtImfDriver_PtPte6MirrorPortStateWrite(  uBaseAddress, 
                                                      uAddress, 
                                                      *pData0, 
                                                      *pData1); 
        }
    break;
    case FE2KXT_MEM_PTPTE6PORTSTATE:
        status =
        sbFe2000XtImfDriver_PtPte6PortStateRead(  uBaseAddress, 
                                                uAddress,    
                                                pData0,     
                                                pData1,     
                                                pData2);    
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,3);
          sbZfC2PtPortStateEntry_Unpack(&PtPte6PortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PtPortStateEntry_SetField(&PtPte6PortStateRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PtPortStateEntry_Pack(&PtPte6PortStateRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,3);
          status =
          sbFe2000XtImfDriver_PtPte6PortStateWrite(  uBaseAddress, 
                                                uAddress,    
                                                *pData0,     
                                                *pData1,     
                                                *pData2);    
        }
    break;
    case FE2KXT_MEM_PTQUEUETOPORT:
        status =
        sbFe2000XtImfDriver_PtQueueToPortRead(  uBaseAddress,  
                                              uAddress,      
                                              pData);       
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfC2PtQueueToPortEntry_Unpack(&PtQueueToPortRecord,(uint8_t *)dbuf,sizeof(dbuf));
          status = sbZfC2PtQueueToPortEntry_SetField(&PtQueueToPortRecord,fieldname,val);
          if(status != 0) return CMD_USAGE;
          sbZfC2PtQueueToPortEntry_Pack(&PtQueueToPortRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbSwapWords(pData,1);
          status =
          sbFe2000XtImfDriver_PtQueueToPortWrite(  uBaseAddress,  
                                              uAddress,      
                                              *pData);       
        }
    break;
    default:
        status = SAND_DRV_C2_STATUS_OK;
    break;
    }
    
    if (status != SAND_DRV_C2_STATUS_OK) {
        printk("Error %d while reading Fe2000 memory\n",status);
    } 
    return 0;
}


/*
 * sbFe2000XtMemSetField - find memory and field names and assign value
 *	inputs:	unit number
 *		name of memory 
 *	output:	prints memory contents
 * 	return:	OK on success, else error status
 */
int
sbFe2000XtMemSetTestAll(int unit, char *tempName, int addr, int val)
{
    int i, x, memindex, status=CMD_OK, doingfield;
    char *dptr, *memFieldPtr, curch, memname[100], memfield[100];
    for(x = 0; x < 410; x++) {
        memFieldPtr = memberListXt[x];
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

        memindex = sbFe2000XtWhichMem(memname);
        if(memindex == -1) {
            printk("Error: unrecognized Fe2000 memory: %s\n",memname);
        }
        status = sbFe2000XtMemSetFieldEntry(unit, memindex,addr,memfield, val);
        if(status == CMD_USAGE) {
            printk("Error: unrecognized field <%s> for this Fe2000 memory: %s\n",&memfield[3],memname);
        }
    }
    return status;
}


/*
 * sbFe2000XtMemSetField - find memory and field names and assign value
 *	inputs:	unit number
 *		name of memory 
 *	output:	prints memory contents
 * 	return:	OK on success, else error status
 */
int
sbFe2000XtMemSetField(int unit, char *memFieldName, int addr, int val)
{
    int i, memindex, status, doingfield, max;
    char *dptr, curch, memname[100], memfield[100];

    if (memFieldName[0] == '*') {
        return sbFe2000XtMemSetTestAll( unit, memFieldName,  addr,  val);
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

    memindex = sbFe2000XtWhichMem(memname);
    if(memindex == -1) {
        printk("Error: unrecognized Fe2000 memory: %s\n",memname);
        printk("Please pick from one of these:\n");
        sbFe2000XtShowMemNames();
        return CMD_FAIL;
    }
    max = sbFe2000XtMemMax(memindex);
    if (addr > max){
        printk("Error: Addr %d out of range for memory: %s max=%d\n",
                           addr,memname,max);
        return CMD_FAIL;
    }
    status = sbFe2000XtMemSetFieldEntry(unit, memindex,addr,memfield, val);
    if(status == CMD_USAGE) {
        printk("Error: unrecognized field <%s> for this Fe2000 memory: %s\n",&memfield[3],memname);
    }
    return status;
}

#endif /* BCM_FE2000_SUPPORT */
