/*
 * $Id: sbFe2000XtMemAccess.c 1.14 Broadcom SDK $
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
 * File:        sbFe2000XtMemAccess.c
 * Purpose:     sbx commands to read/write fe2000 indirect mems
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
#include <appl/diag/sbx/sbx.h>

extern void sbSwapWords(uint *pdata, int nwords);
typedef struct {
      char memname[30];
      int  memindex;
      int  rangemax;
} memConfigRecXt;

memConfigRecXt memConfigTableXt[] =
{
        {"PrPortToQueueAg0", FE2KXT_MEM_PRPORTTOQUEUEAG0, 0xb},
        {"PrPortToQueueAg1", FE2KXT_MEM_PRPORTTOQUEUEAG1, 0xb},
        {"PrPortToQueuePci", FE2KXT_MEM_PRPORTTOQUEUEPCI, 0x0},
        {"PrPortToQueueSr0", FE2KXT_MEM_PRPORTTOQUEUESR0, 0x3f},
        {"PrPortToQueueSr1", FE2KXT_MEM_PRPORTTOQUEUESR1, 0x3f},
        {"PrPortToQueueXg0", FE2KXT_MEM_PRPORTTOQUEUEXG0, 0x0},
        {"PrPortToQueueXg1", FE2KXT_MEM_PRPORTTOQUEUEXG1, 0x0},
        {"PrCc2TcamConfig", FE2KXT_MEM_PRCC2TCAMCONFIG, 0xff},
        {"PrCc3TcamConfig", FE2KXT_MEM_PRCC3TCAMCONFIG, 0xff},
        {"PrCc4TcamConfig", FE2KXT_MEM_PRCC4TCAMCONFIG, 0xff},
        {"PrCc5TcamConfig", FE2KXT_MEM_PRCC5TCAMCONFIG, 0xff},
        {"PrCc2CamRam", FE2KXT_MEM_PRCC2CAMRAM, 0xff},
        {"PrCc3CamRam", FE2KXT_MEM_PRCC3CAMRAM, 0xff},
        {"PrCc4CamRam", FE2KXT_MEM_PRCC4CAMRAM, 0xff},
        {"PrCc5CamRam", FE2KXT_MEM_PRCC5CAMRAM, 0xff},
        {"PrCc2QueueRam", FE2KXT_MEM_PRCC2QUEUERAM, 0xb},
        {"PrCc3QueueRam", FE2KXT_MEM_PRCC3QUEUERAM, 0xb},
        {"PrCc4QueueRam", FE2KXT_MEM_PRCC4QUEUERAM, 0x1},
        {"PrCc5QueueRam", FE2KXT_MEM_PRCC5QUEUERAM, 0x1},
        {"PdLrpOutputHeaderCopyBuffer", FE2KXT_MEM_PDLRPOUTPUTHEADERCOPYBUFFER, 0xff},
        {"PdPdOutputHeaderCopyBuffer", FE2KXT_MEM_PDPDOUTPUTHEADERCOPYBUFFER, 0xff},
        {"LrLrpInstructionMemory", FE2KXT_MEM_LRLRPINSTRUCTIONMEMORY, 0x1fff},
        {"QmBufferState0", FE2KXT_MEM_QMBUFFERSTATE0, 0x1fff},
        {"QmBufferState1", FE2KXT_MEM_QMBUFFERSTATE1, 0x1fff},
        {"QmBufferState2", FE2KXT_MEM_QMBUFFERSTATE2, 0x1fff},
        {"QmBufferState3", FE2KXT_MEM_QMBUFFERSTATE3, 0x1fff},
        {"QmDequeueState0", FE2KXT_MEM_QMDEQUEUESTATE0, 0xff},
        {"QmDequeueState1", FE2KXT_MEM_QMDEQUEUESTATE1, 0xff},
        {"QmFreePageFifo", FE2KXT_MEM_QMFREEPAGEFIFO, 0x1fff},
        {"QmNextBuffer"  , FE2KXT_MEM_QMNEXTBUFFER  , 0x1fff},
        {"QmNextPage"    , FE2KXT_MEM_QMNEXTPAGE    , 0x1fff},
        {"QmQueueConfig" , FE2KXT_MEM_QMQUEUECONFIG , 0xff},
        {"QmQueueCounters", FE2KXT_MEM_QMQUEUECOUNTERS, 0x1ff},
        {"QmQueueHeadPtr", FE2KXT_MEM_QMQUEUEHEADPTR, 0xff},
        {"QmQueueState0" , FE2KXT_MEM_QMQUEUESTATE0 , 0xff},
        {"QmQueueState0En", FE2KXT_MEM_QMQUEUESTATE0EN, 0xff},
        {"QmQueueState1" , FE2KXT_MEM_QMQUEUESTATE1 , 0xff},
        {"QmReplicationState", FE2KXT_MEM_QMREPLICATIONSTATE, 0x1fff},
        {"PmCounterMemory", FE2KXT_MEM_PMCOUNTERMEMORY, 0x7ff},
        {"PmProfileMemory", FE2KXT_MEM_PMPROFILEMEMORY, 0x7ff},
        {"Sr0Counter"    , FE2KXT_MEM_SR0COUNTER    , 0xff},
        {"Sr1Counter"    , FE2KXT_MEM_SR1COUNTER    , 0xff},
        {"PpAggregateHashBitConfig", FE2KXT_MEM_PPAGGREGATEHASHBITCONFIG, 0x3},
        {"PpAggregateHashByteConfig", FE2KXT_MEM_PPAGGREGATEHASHBYTECONFIG, 0x1f},
        {"PpCamConfigurationInstance0", FE2KXT_MEM_PPCAMCONFIGURATIONINSTANCE0, 0x7f},
        {"PpCamConfigurationInstance1", FE2KXT_MEM_PPCAMCONFIGURATIONINSTANCE1, 0x7f},
        {"PpCamConfigurationInstance2", FE2KXT_MEM_PPCAMCONFIGURATIONINSTANCE2, 0x7f},
        {"PpCamConfigurationInstance3", FE2KXT_MEM_PPCAMCONFIGURATIONINSTANCE3, 0x7f},
        {"PpCamRamConfigurationInstance0", FE2KXT_MEM_PPCAMRAMCONFIGURATIONINSTANCE0, 0x7f},
        {"PpCamRamConfigurationInstance1", FE2KXT_MEM_PPCAMRAMCONFIGURATIONINSTANCE1, 0x7f},
        {"PpCamRamConfigurationInstance2", FE2KXT_MEM_PPCAMRAMCONFIGURATIONINSTANCE2, 0x7f},
        {"PpCamRamConfigurationInstance3", FE2KXT_MEM_PPCAMRAMCONFIGURATIONINSTANCE3, 0x7f},
        {"PpHeaderRecordSize", FE2KXT_MEM_PPHEADERRECORDSIZE, 0x7},
        {"PpInitialQueueState", FE2KXT_MEM_PPINITIALQUEUESTATE, 0xff},
        {"PpPpOutHeaderCopy", FE2KXT_MEM_PPPPOUTHEADERCOPY, 0xff},
        {"PpQueuePriorityGroup", FE2KXT_MEM_PPQUEUEPRIORITYGROUP, 0x7f},
        {"PpRxPortData"  , FE2KXT_MEM_PPRXPORTDATA  , 0xff},
        {"Rc0Data"       , FE2KXT_MEM_RC0DATA, 0x7ff},
        {"Rc1Data"       , FE2KXT_MEM_RC1DATA, 0x7ff},
        {"St0Counter"    , FE2KXT_MEM_ST0COUNTER    , 0xff},
        {"St1Counter"    , FE2KXT_MEM_ST1COUNTER    , 0xff},
        {"Mm0Internal0Memory", FE2KXT_MEM_MM0INTERNAL0MEMORY, 0xffff},
        {"Mm0Internal1Memory", FE2KXT_MEM_MM0INTERNAL1MEMORY, 0xffff},
        {"Mm0NarrowPort0Memory", FE2KXT_MEM_MM0NARROWPORT0MEMORY, 0x7fffff},
        {"Mm0NarrowPort1Memory", FE2KXT_MEM_MM0NARROWPORT1MEMORY, 0x7fffff},
        {"Mm0WidePortMemory", FE2KXT_MEM_MM0WIDEPORTMEMORY, 0x3fffff},
        {"Mm1Internal0Memory", FE2KXT_MEM_MM1INTERNAL0MEMORY, 0xffff},
        {"Mm1Internal1Memory", FE2KXT_MEM_MM1INTERNAL1MEMORY, 0xffff},
        {"Mm1NarrowPort0Memory", FE2KXT_MEM_MM1NARROWPORT0MEMORY, 0x7fffff},
        {"Mm1NarrowPort1Memory", FE2KXT_MEM_MM1NARROWPORT1MEMORY, 0x7fffff},
        {"Mm1WidePortMemory", FE2KXT_MEM_MM1WIDEPORTMEMORY, 0x3fffff},
        {"Pb0Data"       , FE2KXT_MEM_PB0DATA, 0x1fff},
        {"Pb1Data"       , FE2KXT_MEM_PB1DATA, 0x1fff},
        {"PbCounter",        FE2KXT_MEM_PBCOUNTERMEMORY, 0x7ff}, /* MCM no PBCOUNTER */
        {"PtMirrorIndex" ,   FE2KXT_MEM_PTMIRRORINDEX , 0xf},
        {"PtPortToQueueAg0", FE2KXT_MEM_PTPORTTOQUEUEAG0, 0xb},
        {"PtPortToQueueAg1", FE2KXT_MEM_PTPORTTOQUEUEAG1, 0xb},
        {"PtPortToQueuePci", FE2KXT_MEM_PTPORTTOQUEUEPCI, 0x0},
        {"PtPortToQueueSt0", FE2KXT_MEM_PTPORTTOQUEUEST0, 0x3f},
        {"PtPortToQueueSt1", FE2KXT_MEM_PTPORTTOQUEUEST1, 0x3f},
        {"PtPortToQueueXg0", FE2KXT_MEM_PTPORTTOQUEUEXG0, 0x0},
        {"PtPortToQueueXg1", FE2KXT_MEM_PTPORTTOQUEUEXG1, 0x0},
        {"PtPte0MirrorPortState", FE2KXT_MEM_PTPTE0MIRRORPORTSTATE, 0x3f},
        {"PtPte0PortState", FE2KXT_MEM_PTPTE0PORTSTATE, 0x3f},
        {"PtPte1MirrorPortState", FE2KXT_MEM_PTPTE1MIRRORPORTSTATE, 0x3f},
        {"PtPte1PortState", FE2KXT_MEM_PTPTE1PORTSTATE, 0x3f},
        {"PtPte2MirrorPortState", FE2KXT_MEM_PTPTE2MIRRORPORTSTATE, 0xb},
        {"PtPte2PortState", FE2KXT_MEM_PTPTE2PORTSTATE, 0xb},
        {"PtPte3MirrorPortState", FE2KXT_MEM_PTPTE3MIRRORPORTSTATE, 0xb},
        {"PtPte3PortState", FE2KXT_MEM_PTPTE3PORTSTATE, 0xb},
        {"PtPte4MirrorPortState", FE2KXT_MEM_PTPTE4MIRRORPORTSTATE, 0x0},
        {"PtPte4PortState", FE2KXT_MEM_PTPTE4PORTSTATE, 0x0},
        {"PtPte5MirrorPortState", FE2KXT_MEM_PTPTE5MIRRORPORTSTATE, 0x0},
        {"PtPte5PortState", FE2KXT_MEM_PTPTE5PORTSTATE, 0x0},
        {"PtPte6MirrorPortState", FE2KXT_MEM_PTPTE6MIRRORPORTSTATE, 0x0},
        {"PtPte6PortState", FE2KXT_MEM_PTPTE6PORTSTATE, 0x0},
        {"PtQueueToPort" , FE2KXT_MEM_PTQUEUETOPORT , 0xff},
        {"PtPte6MirrorPortState", FE2KXT_MEM_PTPTE6MIRRORPORTSTATE, 0xff},
        {"END", FE2KXT_MEM_MAX_INDEX, 0xff},
};

/*
 * scan memConfigTable for designated name and return memIndex
 */
int
sbFe2000XtWhichMem(char *memname)
{
   int i;

   for(i = 0; i < FE2KXT_MEM_MAX_INDEX; i++) {
       if(strcmp(memname,memConfigTableXt[i].memname) == 0)
          return memConfigTableXt[i].memindex;
   }
   return -1;

}

int 
sbFe2000XtMemMax(int memindex)
{
    return memConfigTableXt[memindex].rangemax;
}

void
sbFe2000XtShowMemNames(void)
{
   int i;

   for(i = 0; i < FE2KXT_MEM_MAX_INDEX; i++) {
       printk("%-32s",memConfigTableXt[i].memname);
       if ((i+1)%3 == 0)
	 printk("\n");
   }
   printk("\n");
}





/*
 * sbFe2000XtMemShowEntry - read and display contents of fe2000 indirect memory
 *	inputs:	unit number
 *		name of memory 
 *	output:	prints memory contents
 * 	return:	OK on success, else error status
 */
int
sbFe2000XtMemShowEntry(int unit, int memindex, int entryindex)
{
  int uBaseAddress,bClearOnRead = 0;
  int  status;
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
    /* sbZfC2PbCounterEntry_t   PbCounterRecord; */
    /* sbZfC2PmCounterMemoryEntry_t   PmCounterRecord; */
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
    uAddress = entryindex;

    printk("---  Entry index: %d ---\n",entryindex);
    switch(memindex) {
    case FE2KXT_MEM_PRPORTTOQUEUEAG0:
        status =
        sbFe2000XtImfDriver_PrPortToQueueAg0Read(  uBaseAddress, 
                                                 uAddress,   
                                                 pData);    
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfC2PrPortToQueueAg0Entry_Unpack(&PrPortToQueueAg0Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbZfC2PrPortToQueueAg0Entry_Print(&PrPortToQueueAg0Record); 
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
          sbZfC2PrPortToQueueAg1Entry_Print(&PrPortToQueueAg1Record); 
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
          sbZfC2PrPortToQueuePciEntry_Print(&PrPortToQueuePciRecord); 
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
          sbZfC2PrPortToQueueSr0Entry_Print(&PrPortToQueueSr0Record);
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
          sbZfC2PrPortToQueueSr1Entry_Print(&PrPortToQueueSr1Record);
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
          sbZfC2PrPortToQueueXg0Entry_Print(&PrPortToQueueXg0Record);
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
          sbZfC2PrPortToQueueXg1Entry_Print(&PrPortToQueueXg1Record);
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
          sbSwapWords(pData,SB_ZF_C2PRCCTCAMCONFIGENTRY_SIZE_IN_WORDS);
          sbZfC2PrCcTcamConfigEntry_Unpack(&PrCcTcamConfigEntry,(uint8_t *)dbuf,sizeof(dbuf));
          sbZfC2PrCcTcamConfigEntry_Print(&PrCcTcamConfigEntry);
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
          sbSwapWords(pData,SB_ZF_C2PRCCTCAMCONFIGENTRY_SIZE_IN_WORDS);
          sbZfC2PrCcTcamConfigEntry_Unpack(&PrCcTcamConfigEntry,(uint8_t *)dbuf,sizeof(dbuf));
          sbZfC2PrCcTcamConfigEntry_Print(&PrCcTcamConfigEntry);
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
          sbSwapWords(pData,SB_ZF_C2PRCCTCAMCONFIGENTRY_SIZE_IN_WORDS);
          sbZfC2PrCcTcamConfigEntry_Unpack(&PrCcTcamConfigEntry,(uint8_t *)dbuf,sizeof(dbuf));
          sbZfC2PrCcTcamConfigEntry_Print(&PrCcTcamConfigEntry);
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
          sbSwapWords(pData,SB_ZF_C2PRCCTCAMCONFIGENTRY_SIZE_IN_WORDS);
          sbZfC2PrCcTcamConfigEntry_Unpack(&PrCcTcamConfigEntry,(uint8_t *)dbuf,sizeof(dbuf));
          sbZfC2PrCcTcamConfigEntry_Print(&PrCcTcamConfigEntry);
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
          sbZfC2PrCcCamRamNonLastEntry_Unpack(&PrCcCamRamNonLastEntry,(uint8_t *)dbuf,sizeof(dbuf));
          if (PrCcCamRamNonLastEntry.m_uLast == 1) {
             sbZfC2PrCcCamRamLastEntry_Unpack(&PrCcCamRamLastEntry,(uint8_t *)dbuf,sizeof(dbuf));
             sbZfC2PrCcCamRamLastEntry_Print(&PrCcCamRamLastEntry);
          }else{
             sbZfC2PrCcCamRamNonLastEntry_Print(&PrCcCamRamNonLastEntry);
          }
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
          sbZfC2PrCcCamRamNonLastEntry_Unpack(&PrCcCamRamNonLastEntry,(uint8_t *)dbuf,sizeof(dbuf));
          if (PrCcCamRamNonLastEntry.m_uLast == 1) {
             sbZfC2PrCcCamRamLastEntry_Unpack(&PrCcCamRamLastEntry,(uint8_t *)dbuf,sizeof(dbuf));
             sbZfC2PrCcCamRamLastEntry_Print(&PrCcCamRamLastEntry);
          }else{
             sbZfC2PrCcCamRamNonLastEntry_Print(&PrCcCamRamNonLastEntry);
          }
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
          sbZfC2PrCcCamRamNonLastEntry_Unpack(&PrCcCamRamNonLastEntry,(uint8_t *)dbuf,sizeof(dbuf));
          if (PrCcCamRamNonLastEntry.m_uLast == 1) {
             sbZfC2PrCcCamRamLastEntry_Unpack(&PrCcCamRamLastEntry,(uint8_t *)dbuf,sizeof(dbuf));
             sbZfC2PrCcCamRamLastEntry_Print(&PrCcCamRamLastEntry);
          }else{
             sbZfC2PrCcCamRamNonLastEntry_Print(&PrCcCamRamNonLastEntry);
          }
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
          sbZfC2PrCcCamRamNonLastEntry_Unpack(&PrCcCamRamNonLastEntry,(uint8_t *)dbuf,sizeof(dbuf));
          if (PrCcCamRamNonLastEntry.m_uLast == 1) {
             sbZfC2PrCcCamRamLastEntry_Unpack(&PrCcCamRamLastEntry,(uint8_t *)dbuf,sizeof(dbuf));
             sbZfC2PrCcCamRamLastEntry_Print(&PrCcCamRamLastEntry);
          }else{
             sbZfC2PrCcCamRamNonLastEntry_Print(&PrCcCamRamNonLastEntry);
          }
        }
        break;
    case FE2KXT_MEM_PRCC2QUEUERAM:
        status =
        sbFe2000XtImfDriver_PrCc2QueueRamRead(  uBaseAddress, 
                                                 uAddress,   
                                                 &(pData[0]));
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,SB_ZF_C2PRCCQUEUERAMENTRY_SIZE_IN_WORDS);
          sbZfC2PrCcQueueRamEntry_Unpack(&PrCcQueueRamEntry,(uint8_t *)dbuf,sizeof(dbuf));
          sbZfC2PrCcQueueRamEntry_Unpack(&PrCcQueueRamEntry,(uint8_t *)dbuf,sizeof(dbuf));
          sbZfC2PrCcQueueRamEntry_Print(&PrCcQueueRamEntry);
        }
        break;
    case FE2KXT_MEM_PRCC3QUEUERAM:
        status =
        sbFe2000XtImfDriver_PrCc3QueueRamRead(  uBaseAddress, 
                                                 uAddress,   
                                                 &(pData[0]));
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,SB_ZF_C2PRCCQUEUERAMENTRY_SIZE_IN_WORDS);
          sbZfC2PrCcQueueRamEntry_Unpack(&PrCcQueueRamEntry,(uint8_t *)dbuf,sizeof(dbuf));
          sbZfC2PrCcQueueRamEntry_Unpack(&PrCcQueueRamEntry,(uint8_t *)dbuf,sizeof(dbuf));
          sbZfC2PrCcQueueRamEntry_Print(&PrCcQueueRamEntry);
        }
        break;
    case FE2KXT_MEM_PRCC4QUEUERAM:
        status =
        sbFe2000XtImfDriver_PrCc4QueueRamRead(  uBaseAddress, 
                                                 uAddress,   
                                                 &(pData[0]));
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,SB_ZF_C2PRCCQUEUERAMENTRY_SIZE_IN_WORDS);
          sbZfC2PrCcQueueRamEntry_Unpack(&PrCcQueueRamEntry,(uint8_t *)dbuf,sizeof(dbuf));
          sbZfC2PrCcQueueRamEntry_Unpack(&PrCcQueueRamEntry,(uint8_t *)dbuf,sizeof(dbuf));
          sbZfC2PrCcQueueRamEntry_Print(&PrCcQueueRamEntry);
        }
        break;
    case FE2KXT_MEM_PRCC5QUEUERAM:
        status =
        sbFe2000XtImfDriver_PrCc5QueueRamRead(  uBaseAddress, 
                                                 uAddress,   
                                                 &(pData[0]));
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,SB_ZF_C2PRCCQUEUERAMENTRY_SIZE_IN_WORDS);
          sbZfC2PrCcQueueRamEntry_Unpack(&PrCcQueueRamEntry,(uint8_t *)dbuf,sizeof(dbuf));
          sbZfC2PrCcQueueRamEntry_Unpack(&PrCcQueueRamEntry,(uint8_t *)dbuf,sizeof(dbuf));
          sbZfC2PrCcQueueRamEntry_Print(&PrCcQueueRamEntry);
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
          sbZfC2PdLrpOutputHeaderCopyBufferEntry_Print(&PdLrpOutputHeaderCopyBufferRecord);
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
          sbZfC2PdPdOutputHeaderCopyBufferEntry_Print(&PdPdOutputHeaderCopyBufferRecord);
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
          sbZfC2LrLrpInstructionMemoryEntry_Print(&LrLrpInstructionMemoryRecord);
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
          sbZfC2QmBufferState0Entry_Print(&QmBufferState0Record);
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
          sbZfC2QmBufferState1Entry_Print(&QmBufferState1Record);
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
          sbZfC2QmBufferState2Entry_Print(&QmBufferState2Record);
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
          sbZfC2QmBufferState3Entry_Print(&QmBufferState3Record);
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
          sbZfC2QmDequeueState0Entry_Print(&QmDequeueState0Record);
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
          sbZfC2QmDequeueState1Entry_Print(&QmDequeueState1Record);
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
          sbZfC2QmFreePageFifoEntry_Print(&QmFreePageFifoRecord);
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
          sbZfC2QmNextBufferEntry_Print(&QmNextBufferRecord);
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
          sbZfC2QmNextPageEntry_Print(&QmNextPageRecord);
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
          sbZfC2QmQueueConfigEntry_Print(&QmQueueConfigRecord);
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
          sbZfC2QmQueueCountersEntry_Print(&QmQueueCountersRecord);
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
          sbZfC2QmQueueHeadPtrEntry_Print(&QmQueueHeadPtrRecord);
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
          sbZfC2QmQueueState0Entry_Print(&QmQueueState0Record);
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
          sbZfC2QmQueueState0EnEntry_Print(&QmQueueState0EnRecord);
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
          sbZfC2QmQueueState1Entry_Print(&QmQueueState1Record);
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
          sbZfC2QmReplicationStateEntry_Print(&QmReplicationStateRecord);
        }
    break;
#if 0
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
          sbZfC2PmCounterMemoryEntry_Unpack(&PmCounterRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbZfC2PmCounterMemoryEntry_Print(&PmCounterRecord);
        }
    break;
#endif
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
          sbZfC2PmProfileMemoryEntry_Print(&PmProfileMemoryRecord);
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
          sbZfC2SrCounterEntry_Print(&SrCounterRecord);
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
          sbZfC2SrCounterEntry_Print(&SrCounterRecord);
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
          sbZfC2PpAggregateHashBitConfigEntry_Print(&PpAggregateHashBitConfigRecord);
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
          sbZfC2PpAggregateHashByteConfigEntry_Print(&PpAggregateHashByteConfigRecord);
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
          sbZfC2PpCamConfigurationInstance0Entry_Print(&PpCamConfigurationInstance0Record);
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
          sbZfC2PpCamConfigurationInstance0Entry_Print(&PpCamConfigurationInstance0Record);
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
          sbZfC2PpCamConfigurationInstance0Entry_Print(&PpCamConfigurationInstance0Record);
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
          sbZfC2PpCamConfigurationInstance0Entry_Print(&PpCamConfigurationInstance0Record);
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
          sbZfC2PpCamRamConfigurationInstance0Entry_Print(&PpCamRamConfigurationInstance0Record);
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
          sbZfC2PpCamRamConfigurationInstance0Entry_Print(&PpCamRamConfigurationInstance0Record);
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
          sbZfC2PpCamRamConfigurationInstance0Entry_Print(&PpCamRamConfigurationInstance0Record);
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
          sbZfC2PpCamRamConfigurationInstance0Entry_Print(&PpCamRamConfigurationInstance0Record);
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
          sbZfC2PpHeaderRecordSizeEntry_Print(&PpHeaderRecordSizeRecord);
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
          sbZfC2PpInitialQueueStateEntry_Print(&PpInitialQueueStateRecord);
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
          sbZfC2PpPpOutHeaderCopyEntry_Print(&PpPpOutHeaderCopyRecord);
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
          sbZfC2PpQueuePriorityGroupEntry_Print(&PpQueuePriorityGroupRecord);
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
          sbZfC2PpRxPortDataEntry_Print(&PpRxPortDataRecord);
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
          sbZfC2RcDataEntry_Print(&RcDataRecord);
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
          sbZfC2RcDataEntry_Print(&RcDataRecord);
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
          sbZfC2StCounterEntry_Print(&StCounterRecord);
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
          sbZfC2StCounterEntry_Print(&StCounterRecord);
        }
    break;
    case FE2KXT_MEM_MM0INTERNAL0MEMORY:
        status =
        sbFe2000XtImfDriver_Mm0Internal0MemoryRead(  uBaseAddress, 
                                                   uAddress, 
                                                   pData0,  
                                                   pData1); 
        if (status == SAND_DRV_C2_STATUS_OK) {
  printk("Mm0Internal0MemoryRead data0=0x%x data1=0x%x\n",*pData0, *pData1);
          sbSwapWords(pData,2);
  printk("Mm0Internal0MemorySWAP data0=0x%x data1=0x%x\n",*pData0, *pData1);
          sbZfC2MmInternal0MemoryEntry_Unpack(&MmInternal0MemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbZfC2MmInternal0MemoryEntry_Print(&MmInternal0MemoryRecord);
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
          sbZfC2MmInternal1MemoryEntry_Print(&MmInternal1MemoryRecord);
        }
    break;
    case FE2KXT_MEM_MM0NARROWPORT0MEMORY:
        status =
        sbFe2000XtImfDriver_Mm0NarrowPort0MemoryRead(  uBaseAddress, 
                                                     uAddress, 
                                                     pData0, 
                                                     pData1); 
        if (status == SAND_DRV_C2_STATUS_OK) {
	  printk("Mm0NarrowPort0[@0x%06x]: 0x%08x 0x%08x\n",uAddress, *pData0, *pData1);
	  /*
  printk("Mm0NarrowPort0MemoryRead data0=0x%x data1=0x%x\n",*pData0, *pData1);
          sbSwapWords(pData,2);
  printk("Mm0NarrowPort0MemorySWAP data0=0x%x data1=0x%x\n",*pData0, *pData1);
          sbZfC2MmNarrowPort0MemoryEntry_Unpack(&MmNarrowPort0MemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbZfC2MmNarrowPort0MemoryEntry_Print(&MmNarrowPort0MemoryRecord);
	  */
        }
    break;
    case FE2KXT_MEM_MM0NARROWPORT1MEMORY:
        status =
        sbFe2000XtImfDriver_Mm0NarrowPort1MemoryRead(  uBaseAddress, 
                                                     uAddress, 
                                                     pData0, 
                                                     pData1); 
        if (status == SAND_DRV_C2_STATUS_OK) {
	  printk("Mm0NarrowPort1[@0x%06x]: 0x%08x 0x%08x\n",uAddress, *pData0, *pData1);
	  /*
          sbSwapWords(pData,2);
          sbZfC2MmNarrowPort1MemoryEntry_Unpack(&MmNarrowPort1MemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbZfC2MmNarrowPort1MemoryEntry_Print(&MmNarrowPort1MemoryRecord);
	  */
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
          sbZfC2MmWidePortMemoryEntry_Print(&MmWidePortMemoryRecord);
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
          sbZfC2MmInternal0MemoryEntry_Print(&MmInternal0MemoryRecord);
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
          sbZfC2MmInternal1MemoryEntry_Print(&MmInternal1MemoryRecord);
        }
    break;
    case FE2KXT_MEM_MM1NARROWPORT0MEMORY:
        status =
        sbFe2000XtImfDriver_Mm1NarrowPort0MemoryRead(  uBaseAddress, 
                                                     uAddress, 
                                                     pData0, 
                                                     pData1); 
        if (status == SAND_DRV_C2_STATUS_OK) {
	  printk("Mm1NarrowPort0[@0x%06x]: 0x%08x 0x%08x\n",uAddress, *pData0, *pData1);
          sbSwapWords(pData,2);
          /* The output below can't be redirected to a file with diag shell "file=". */
          sbZfC2MmNarrowPort0MemoryEntry_Unpack(&MmNarrowPort0MemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbZfC2MmNarrowPort0MemoryEntry_Print(&MmNarrowPort0MemoryRecord);
        }
    break;
    case FE2KXT_MEM_MM1NARROWPORT1MEMORY:
        status =
        sbFe2000XtImfDriver_Mm1NarrowPort1MemoryRead(  uBaseAddress, 
                                                     uAddress, 
                                                     pData0, 
                                                     pData1); 
        if (status == SAND_DRV_C2_STATUS_OK) {
	  printk("Mm1NarrowPort1[@0x%06x]: 0x%08x 0x%08x\n", uAddress, *pData0, *pData1);
          sbSwapWords(pData,2);
          /* The output below can't be redirected to a file with diag shell "file=". 	  */
	  sbZfC2MmNarrowPort1MemoryEntry_Unpack(&MmNarrowPort1MemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbZfC2MmNarrowPort1MemoryEntry_Print(&MmNarrowPort1MemoryRecord);
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
          sbZfC2MmWidePortMemoryEntry_Print(&MmWidePortMemoryRecord);
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
          sbZfC2PbDataEntry_Print(&PbDataRecord);
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
          sbZfC2PbDataEntry_Print(&PbDataRecord);
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
          sbZfC2PtMirrorIndexEntry_Print(&PtMirrorIndexRecord);
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
          sbZfC2PtPortToQueueEntry_Print(&PtPortToQueueRecord);
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
          sbZfC2PtPortToQueueEntry_Print(&PtPortToQueueRecord);
        }
    break;
/* MCM - ? use PrPortXXX_Unpack - or - us PtPortToQueueEntry_Unpack ??? */
    case FE2KXT_MEM_PTPORTTOQUEUEPCI:
        status =
        sbFe2000XtImfDriver_PtPortToQueuePciRead(  uBaseAddress, 
                                                 uAddress,   
                                                 pData);    
        if (status == SAND_DRV_C2_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfC2PrPortToQueuePciEntry_Unpack(&PrPortToQueuePciRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbZfC2PrPortToQueuePciEntry_Print(&PrPortToQueuePciRecord);
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
          sbZfC2PtPortToQueueEntry_Print(&PtPortToQueueRecord);
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
          sbZfC2PtPortToQueueEntry_Print(&PtPortToQueueRecord);
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
          sbZfC2PtPortToQueueEntry_Print(&PtPortToQueueRecord);
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
          sbZfC2PtPortToQueueEntry_Print(&PtPortToQueueRecord);
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
          sbZfC2PtMirrorPortStateEntry_Print(&PtPte0MirrorPortStateRecord);
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
          sbZfC2PtPortStateEntry_Print(&PtPte0PortStateRecord);
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
          sbZfC2PtMirrorPortStateEntry_Print(&PtPte1MirrorPortStateRecord);
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
          sbZfC2PtPortStateEntry_Print(&PtPte1PortStateRecord);
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
          sbZfC2PtMirrorPortStateEntry_Print(&PtPte2MirrorPortStateRecord);
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
          sbZfC2PtPortStateEntry_Print(&PtPte2PortStateRecord);
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
          sbZfC2PtMirrorPortStateEntry_Print(&PtPte3MirrorPortStateRecord);
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
          sbZfC2PtPortStateEntry_Print(&PtPte3PortStateRecord);
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
          sbZfC2PtMirrorPortStateEntry_Print(&PtPte4MirrorPortStateRecord);
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
          sbZfC2PtPortStateEntry_Print(&PtPte4PortStateRecord);
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
          sbZfC2PtMirrorPortStateEntry_Print(&PtPte5MirrorPortStateRecord);
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
          sbZfC2PtPortStateEntry_Print(&PtPte5PortStateRecord);
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
          sbZfC2PtMirrorPortStateEntry_Print(&PtPte6MirrorPortStateRecord);
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
          sbZfC2PtPortStateEntry_Print(&PtPte6PortStateRecord);
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
          sbZfC2PtQueueToPortEntry_Print(&PtQueueToPortRecord);
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
 * sbFe2000XtMemShowRange - call ShowEntry for a range of entries
 *	inputs:	unit number
 *		index of memory 
 *              start of range
 *              end of range
 *	output:	prints memory contents
 * 	return:	OK on success, else error status
 */
int
sbFe2000XtMemShowRange(int unit, int memindex, int rangemin, int rangemax)
{
    int i,  status;

    for(i = rangemin; i <= rangemax; i++) {
        status = sbFe2000XtMemShowEntry(unit, memindex,i);
        if(status != 0)
            return status;
    }
    return 0;
}

/* #include "sbFe2000MemWrite.txt"*/

/*
 * sbFe2000XtMemShow - read and display contents of fe2000 indirect memory
 *	inputs:	unit number
 *		name of memory 
 *	output:	prints memory contents
 * 	return:	OK on success, else error status
 */
int
sbFe2000XtMemShow(int unit, char *memname, int rangemin, int rangemax)
{
    int i, cnt, len, tempmax;
    int status = CMD_FAIL;

    cnt = 0;
    len = strlen(memname);
    for(i = 0; i < FE2KXT_MEM_MAX_INDEX; i++) {
        if ((memname[0] == '*') || (strncmp(memname,memConfigTableXt[i].memname,len) == 0)) {
             cnt++;
             printk("------------- %s ------------\n",memConfigTableXt[i].memname);
             tempmax = rangemax;
             if (rangemax > memConfigTableXt[i].rangemax){
                  tempmax = memConfigTableXt[i].rangemax;
                  printk("Warning: Max range for mem %s is %d\n",memname,tempmax);
             }
             status = sbFe2000XtMemShowRange(unit,memConfigTableXt[i].memindex,rangemin,tempmax);
        }
    }
    if(cnt == 0) {
        printk("Error: unrecognized Fe2000 memory: %s\n",memname);
        printk("Please pick from one of these:\n");
        sbFe2000XtShowMemNames();
    } 
    return status;
}

int
sbFe2000XtMemShow_OLD(int unit, char *memname, int rangemin, int rangemax)
{
    int i, memindex, status;

    if (memname[0] == '*') {
       for(i = 0; i < FE2KXT_MEM_MAX_INDEX; i++) {
           printk("------------- %s ------------\n",memConfigTableXt[i].memname);
           status = sbFe2000XtMemShowRange(unit, i,0,0xff);
       }
    }
    else {
        memindex = sbFe2000XtWhichMem(memname);
        if(memindex == -1) {
            printk("Error: unrecognized Fe2000 memory: %s\n",memname);
            printk("Please pick from one of these:\n");
            sbFe2000XtShowMemNames();
            status = SAND_DRV_C2_STATUS_OK;
        }
        else {
            if (rangemax > memConfigTableXt[memindex].rangemax){
                rangemax = memConfigTableXt[memindex].rangemax;
                printk("Warning: Max range for mem %s is %d\n",memname,rangemax);
            }
            status = sbFe2000XtMemShowRange(unit, memindex,rangemin,rangemax);
        }
    }
    return status;
}


#endif /* BCM_FE2000_SUPPORT */
