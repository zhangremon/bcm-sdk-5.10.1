/*
 * $Id: sbFe2000MemAccess.c 1.14.20.2 Broadcom SDK $
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
 * File:        sbFe2000MemAccess.c
 * Purpose:     sbx commands to read/write fe2000 indirect mems
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
#include <appl/diag/sbx/sbx.h>
#include <appl/diag/sbx/imfswap32.h>

typedef struct {
      char memname[30];
      int  memindex;
      int  rangemax;
} memConfigRec;

memConfigRec memConfigTable[] =
{
        {"PrPortToQueueAg0", FE2K_MEM_PRPORTTOQUEUEAG0, 0xb},
        {"PrPortToQueueAg1", FE2K_MEM_PRPORTTOQUEUEAG1, 0xb},
        {"PrPortToQueuePci", FE2K_MEM_PRPORTTOQUEUEPCI, 0x0},
        {"PrPortToQueueSr0", FE2K_MEM_PRPORTTOQUEUESR0, 0x3f},
        {"PrPortToQueueSr1", FE2K_MEM_PRPORTTOQUEUESR1, 0x3f},
        {"PrPortToQueueXg0", FE2K_MEM_PRPORTTOQUEUEXG0, 0x0},
        {"PrPortToQueueXg1", FE2K_MEM_PRPORTTOQUEUEXG1, 0x0},
        {"PdLrpOutputHeaderCopyBuffer", FE2K_MEM_PDLRPOUTPUTHEADERCOPYBUFFER, 0xff},
        {"PdPdOutputHeaderCopyBuffer", FE2K_MEM_PDPDOUTPUTHEADERCOPYBUFFER, 0xff},
        {"LrLrpInstructionMemory", FE2K_MEM_LRLRPINSTRUCTIONMEMORY, 0x1fff},
        {"QmBufferState0", FE2K_MEM_QMBUFFERSTATE0, 0xe3f},
        {"QmBufferState1", FE2K_MEM_QMBUFFERSTATE1, 0xe3f},
        {"QmBufferState2", FE2K_MEM_QMBUFFERSTATE2, 0xe3f},
        {"QmBufferState3", FE2K_MEM_QMBUFFERSTATE3, 0xe3f},
        {"QmDequeueState0", FE2K_MEM_QMDEQUEUESTATE0, 0xff},
        {"QmDequeueState1", FE2K_MEM_QMDEQUEUESTATE1, 0xff},
        {"QmFreePageFifo", FE2K_MEM_QMFREEPAGEFIFO, 0xff},
        {"QmNextBuffer"  , FE2K_MEM_QMNEXTBUFFER  , 0xe3f},
        {"QmNextPage"    , FE2K_MEM_QMNEXTPAGE    , 0xe3f},
        {"QmQueueConfig" , FE2K_MEM_QMQUEUECONFIG , 0xff},
        {"QmQueueCounters", FE2K_MEM_QMQUEUECOUNTERS, 0x1ff},
        {"QmQueueHeadPtr", FE2K_MEM_QMQUEUEHEADPTR, 0xff},
        {"QmQueueState0" , FE2K_MEM_QMQUEUESTATE0 , 0xff},
        {"QmQueueState0En", FE2K_MEM_QMQUEUESTATE0EN, 0xff},
        {"QmQueueState1" , FE2K_MEM_QMQUEUESTATE1 , 0xff},
        {"QmReplicationState", FE2K_MEM_QMREPLICATIONSTATE, 0xe3f},
        {"PmCounterMemory", FE2K_MEM_PMCOUNTERMEMORY, 0x7ff},
        {"PmProfileMemory", FE2K_MEM_PMPROFILEMEMORY, 0x7ff},
        {"Sr0Counter"    , FE2K_MEM_SR0COUNTER    , 0xff},
        {"Sr1Counter"    , FE2K_MEM_SR1COUNTER    , 0xff},
        {"PpAggregateHashBitConfig", FE2K_MEM_PPAGGREGATEHASHBITCONFIG, 0x3},
        {"PpAggregateHashByteConfig", FE2K_MEM_PPAGGREGATEHASHBYTECONFIG, 0x1f},
        {"PpCamConfigurationInstance0", FE2K_MEM_PPCAMCONFIGURATIONINSTANCE0, 0x7f},
        {"PpCamConfigurationInstance1", FE2K_MEM_PPCAMCONFIGURATIONINSTANCE1, 0x7f},
        {"PpCamConfigurationInstance2", FE2K_MEM_PPCAMCONFIGURATIONINSTANCE2, 0x7f},
        {"PpCamConfigurationInstance3", FE2K_MEM_PPCAMCONFIGURATIONINSTANCE3, 0x7f},
        {"PpCamRamConfigurationInstance0", FE2K_MEM_PPCAMRAMCONFIGURATIONINSTANCE0, 0x7f},
        {"PpCamRamConfigurationInstance1", FE2K_MEM_PPCAMRAMCONFIGURATIONINSTANCE1, 0x7f},
        {"PpCamRamConfigurationInstance2", FE2K_MEM_PPCAMRAMCONFIGURATIONINSTANCE2, 0x7f},
        {"PpCamRamConfigurationInstance3", FE2K_MEM_PPCAMRAMCONFIGURATIONINSTANCE3, 0x7f},
        {"PpHeaderRecordSize", FE2K_MEM_PPHEADERRECORDSIZE, 0x7},
        {"PpInitialQueueState", FE2K_MEM_PPINITIALQUEUESTATE, 0xff},
        {"PpPpOutHeaderCopy", FE2K_MEM_PPPPOUTHEADERCOPY, 0xff},
        {"PpQueuePriorityGroup", FE2K_MEM_PPQUEUEPRIORITYGROUP, 0x7f},
        {"PpRxPortData"  , FE2K_MEM_PPRXPORTDATA  , 0xff},
        {"Rc0Data"       , FE2K_MEM_RC0DATA, 0x7ff},
        {"Rc1Data"       , FE2K_MEM_RC1DATA, 0x7ff},
        {"St0Counter"    , FE2K_MEM_ST0COUNTER    , 0xff},
        {"St1Counter"    , FE2K_MEM_ST1COUNTER    , 0xff},
        {"Mm0Internal0Memory", FE2K_MEM_MM0INTERNAL0MEMORY, 0x1fff},
        {"Mm0Internal1Memory", FE2K_MEM_MM0INTERNAL1MEMORY, 0x1fff},
        {"Mm0NarrowPort0Memory", FE2K_MEM_MM0NARROWPORT0MEMORY, 0x7fffff},
        {"Mm0NarrowPort1Memory", FE2K_MEM_MM0NARROWPORT1MEMORY, 0x7fffff},
        {"Mm0WidePortMemory", FE2K_MEM_MM0WIDEPORTMEMORY, 0x3fffff},
        {"Mm1Internal0Memory", FE2K_MEM_MM1INTERNAL0MEMORY, 0x1fff},
        {"Mm1Internal1Memory", FE2K_MEM_MM1INTERNAL1MEMORY, 0x1fff},
        {"Mm1NarrowPort0Memory", FE2K_MEM_MM1NARROWPORT0MEMORY, 0x7fffff},
        {"Mm1NarrowPort1Memory", FE2K_MEM_MM1NARROWPORT1MEMORY, 0x7fffff},
        {"Mm1WidePortMemory", FE2K_MEM_MM1WIDEPORTMEMORY, 0x3fffff},
        {"Pb0Data"       , FE2K_MEM_PB0DATA, 0xff},
        {"Pb1Data"       , FE2K_MEM_PB1DATA, 0xff},
        {"PbCounter",        FE2K_MEM_PBCOUNTERMEMORY, 0x7ff}, /* MCM no PBCOUNTER */
        {"PtMirrorIndex" ,   FE2K_MEM_PTMIRRORINDEX , 0xff},
        {"PtPortToQueueAg0", FE2K_MEM_PTPORTTOQUEUEAG0, 0xb},
        {"PtPortToQueueAg1", FE2K_MEM_PTPORTTOQUEUEAG1, 0xb},
        {"PtPortToQueuePci", FE2K_MEM_PTPORTTOQUEUEPCI, 0x0},
        {"PtPortToQueueSt0", FE2K_MEM_PTPORTTOQUEUEST0, 0x3f},
        {"PtPortToQueueSt1", FE2K_MEM_PTPORTTOQUEUEST1, 0x3f},
        {"PtPortToQueueXg0", FE2K_MEM_PTPORTTOQUEUEXG0, 0x0},
        {"PtPortToQueueXg1", FE2K_MEM_PTPORTTOQUEUEXG1, 0x0},
        {"PtPte0MirrorPortState", FE2K_MEM_PTPTE0MIRRORPORTSTATE, 0x3f},
        {"PtPte0PortState", FE2K_MEM_PTPTE0PORTSTATE, 0x3f},
        {"PtPte1MirrorPortState", FE2K_MEM_PTPTE1MIRRORPORTSTATE, 0x3f},
        {"PtPte1PortState", FE2K_MEM_PTPTE1PORTSTATE, 0x3f},
        {"PtPte2MirrorPortState", FE2K_MEM_PTPTE2MIRRORPORTSTATE, 0xb},
        {"PtPte2PortState", FE2K_MEM_PTPTE2PORTSTATE, 0xb},
        {"PtPte3MirrorPortState", FE2K_MEM_PTPTE3MIRRORPORTSTATE, 0xb},
        {"PtPte3PortState", FE2K_MEM_PTPTE3PORTSTATE, 0xb},
        {"PtPte4MirrorPortState", FE2K_MEM_PTPTE4MIRRORPORTSTATE, 0x0},
        {"PtPte4PortState", FE2K_MEM_PTPTE4PORTSTATE, 0x0},
        {"PtPte5MirrorPortState", FE2K_MEM_PTPTE5MIRRORPORTSTATE, 0x0},
        {"PtPte5PortState", FE2K_MEM_PTPTE5PORTSTATE, 0x0},
        {"PtPte6MirrorPortState", FE2K_MEM_PTPTE6MIRRORPORTSTATE, 0x0},
        {"PtPte6PortState", FE2K_MEM_PTPTE6PORTSTATE, 0x0},
        {"PtQueueToPort" , FE2K_MEM_PTQUEUETOPORT , 0xff},
        {"PtPte6MirrorPortState", FE2K_MEM_PTPTE6MIRRORPORTSTATE, 0xff},
        {"END", FE2K_MEM_MAX_INDEX, 0xff},
};

#define SBSWAP

void sbSwapWords(uint *pdata, int nwords)
{
#ifdef SBSWAP
  int i;

   for(i = 0; i < nwords; i++) {
       *pdata = imfswap32(*pdata);
       pdata++;
   }
#else
   return;
#endif
}
/*
 * scan memConfigTable for designated name and return memIndex
 */
int
sbFe2000WhichMem(char *memname)
{
   int i;

   for(i = 0; i < FE2K_MEM_MAX_INDEX; i++) {
       if(strcmp(memname,memConfigTable[i].memname) == 0)
          return memConfigTable[i].memindex;
   }
   return -1;

}

int 
sbFe2000MemMax(int memindex)
{
    return memConfigTable[memindex].rangemax;
}

void
sbFe2000ShowMemNames(void)
{
   int i;

   for(i = 0; i < FE2K_MEM_MAX_INDEX; i++) {
       printk("%-32s",memConfigTable[i].memname);
       if ((i+1)%3 == 0)
	 printk("\n");
   }
   printk("\n");
}





/*
 * sbFe2000MemShowEntry - read and display contents of fe2000 indirect memory
 *	inputs:	unit number
 *		name of memory 
 *	output:	prints memory contents
 * 	return:	OK on success, else error status
 */
int
sbFe2000MemShowEntry(int unit, int memindex, int entryindex)
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
    /* sbZfCaPbCounterEntry_t   PbCounterRecord; */
    sbZfCaPmCounterMemoryEntry_t   PmCounterRecord;
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
    uAddress = entryindex;

    printk("---  Entry index: %d ---\n",entryindex);
    switch(memindex) {
    case FE2K_MEM_PRPORTTOQUEUEAG0:
        status =
        sbFe2000ImfDriver_PrPortToQueueAg0Read(  uBaseAddress, 
                                                 uAddress,   
                                                 pData);    
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfCaPrPortToQueueAg0Entry_Unpack(&PrPortToQueueAg0Record,(uint8_t *)dbuf,sizeof(dbuf));
          sbZfCaPrPortToQueueAg0Entry_Print(&PrPortToQueueAg0Record); 
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
          sbZfCaPrPortToQueueAg1Entry_Print(&PrPortToQueueAg1Record); 
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
          sbZfCaPrPortToQueuePciEntry_Print(&PrPortToQueuePciRecord); 
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
          sbZfCaPrPortToQueueSr0Entry_Print(&PrPortToQueueSr0Record);
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
          sbZfCaPrPortToQueueSr1Entry_Print(&PrPortToQueueSr1Record);
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
          sbZfCaPrPortToQueueXg0Entry_Print(&PrPortToQueueXg0Record);
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
          sbZfCaPrPortToQueueXg1Entry_Print(&PrPortToQueueXg1Record);
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
          sbZfCaPdLrpOutputHeaderCopyBufferEntry_Print(&PdLrpOutputHeaderCopyBufferRecord);
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
          sbZfCaPdPdOutputHeaderCopyBufferEntry_Print(&PdPdOutputHeaderCopyBufferRecord);
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
          sbZfCaLrLrpInstructionMemoryEntry_Print(&LrLrpInstructionMemoryRecord);
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
          sbZfCaQmBufferState0Entry_Print(&QmBufferState0Record);
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
          sbZfCaQmBufferState1Entry_Print(&QmBufferState1Record);
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
          sbZfCaQmBufferState2Entry_Print(&QmBufferState2Record);
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
          sbZfCaQmBufferState3Entry_Print(&QmBufferState3Record);
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
          sbZfCaQmDequeueState0Entry_Print(&QmDequeueState0Record);
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
          sbZfCaQmDequeueState1Entry_Print(&QmDequeueState1Record);
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
          sbZfCaQmFreePageFifoEntry_Print(&QmFreePageFifoRecord);
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
          sbZfCaQmNextBufferEntry_Print(&QmNextBufferRecord);
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
          sbZfCaQmNextPageEntry_Print(&QmNextPageRecord);
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
          sbZfCaQmQueueConfigEntry_Print(&QmQueueConfigRecord);
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
          sbZfCaQmQueueCountersEntry_Print(&QmQueueCountersRecord);
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
          sbZfCaQmQueueHeadPtrEntry_Print(&QmQueueHeadPtrRecord);
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
          sbZfCaQmQueueState0Entry_Print(&QmQueueState0Record);
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
          sbZfCaQmQueueState0EnEntry_Print(&QmQueueState0EnRecord);
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
          sbZfCaQmQueueState1Entry_Print(&QmQueueState1Record);
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
          sbZfCaQmReplicationStateEntry_Print(&QmReplicationStateRecord);
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
          sbZfCaPmCounterMemoryEntry_Unpack(&PmCounterRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbZfCaPmCounterMemoryEntry_Print(&PmCounterRecord);
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
          sbZfCaPmProfileMemoryEntry_Print(&PmProfileMemoryRecord);
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
          sbZfCaSrCounterEntry_Print(&SrCounterRecord);
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
          sbZfCaSrCounterEntry_Print(&SrCounterRecord);
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
          sbZfCaPpAggregateHashBitConfigEntry_Print(&PpAggregateHashBitConfigRecord);
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
          sbZfCaPpAggregateHashByteConfigEntry_Print(&PpAggregateHashByteConfigRecord);
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
          sbZfCaPpCamConfigurationInstance0Entry_Print(&PpCamConfigurationInstance0Record);
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
          sbZfCaPpCamConfigurationInstance0Entry_Print(&PpCamConfigurationInstance0Record);
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
          sbZfCaPpCamConfigurationInstance0Entry_Print(&PpCamConfigurationInstance0Record);
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
          sbZfCaPpCamConfigurationInstance0Entry_Print(&PpCamConfigurationInstance0Record);
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
          sbZfCaPpCamRamConfigurationInstance0Entry_Print(&PpCamRamConfigurationInstance0Record);
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
          sbZfCaPpCamRamConfigurationInstance0Entry_Print(&PpCamRamConfigurationInstance0Record);
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
          sbZfCaPpCamRamConfigurationInstance0Entry_Print(&PpCamRamConfigurationInstance0Record);
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
          sbZfCaPpCamRamConfigurationInstance0Entry_Print(&PpCamRamConfigurationInstance0Record);
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
          sbZfCaPpHeaderRecordSizeEntry_Print(&PpHeaderRecordSizeRecord);
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
          sbZfCaPpInitialQueueStateEntry_Print(&PpInitialQueueStateRecord);
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
          sbZfCaPpPpOutHeaderCopyEntry_Print(&PpPpOutHeaderCopyRecord);
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
          sbZfCaPpQueuePriorityGroupEntry_Print(&PpQueuePriorityGroupRecord);
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
          sbZfCaPpRxPortDataEntry_Print(&PpRxPortDataRecord);
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
          sbZfCaRcDataEntry_Print(&RcDataRecord);
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
          sbZfCaRcDataEntry_Print(&RcDataRecord);
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
          sbZfCaStCounterEntry_Print(&StCounterRecord);
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
          sbZfCaStCounterEntry_Print(&StCounterRecord);
        }
    break;
    case FE2K_MEM_MM0INTERNAL0MEMORY:
        status =
        sbFe2000ImfDriver_Mm0Internal0MemoryRead(  uBaseAddress, 
                                                   uAddress, 
                                                   pData0,  
                                                   pData1); 
        if (status == SAND_DRV_CA_STATUS_OK) {
  printk("Mm0Internal0MemoryRead data0=0x%x data1=0x%x\n",*pData0, *pData1);
          sbSwapWords(pData,2);
  printk("Mm0Internal0MemorySWAP data0=0x%x data1=0x%x\n",*pData0, *pData1);
          sbZfCaMmInternal0MemoryEntry_Unpack(&MmInternal0MemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbZfCaMmInternal0MemoryEntry_Print(&MmInternal0MemoryRecord);
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
          sbZfCaMmInternal1MemoryEntry_Print(&MmInternal1MemoryRecord);
        }
    break;
    case FE2K_MEM_MM0NARROWPORT0MEMORY:
        status =
        sbFe2000ImfDriver_Mm0NarrowPort0MemoryRead(  uBaseAddress, 
                                                     uAddress, 
                                                     pData0, 
                                                     pData1); 
        if (status == SAND_DRV_CA_STATUS_OK) {
	  printk("Mm0NarrowPort0[@0x%06x]: 0x%08x 0x%08x\n",uAddress, *pData0, *pData1);
	  /*
  printk("Mm0NarrowPort0MemoryRead data0=0x%x data1=0x%x\n",*pData0, *pData1);
          sbSwapWords(pData,2);
  printk("Mm0NarrowPort0MemorySWAP data0=0x%x data1=0x%x\n",*pData0, *pData1);
          sbZfCaMmNarrowPort0MemoryEntry_Unpack(&MmNarrowPort0MemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbZfCaMmNarrowPort0MemoryEntry_Print(&MmNarrowPort0MemoryRecord);
	  */
        }
    break;
    case FE2K_MEM_MM0NARROWPORT1MEMORY:
        status =
        sbFe2000ImfDriver_Mm0NarrowPort1MemoryRead(  uBaseAddress, 
                                                     uAddress, 
                                                     pData0, 
                                                     pData1); 
        if (status == SAND_DRV_CA_STATUS_OK) {
	  printk("Mm0NarrowPort1[@0x%06x]: 0x%08x 0x%08x\n",uAddress, *pData0, *pData1);
	  /*
          sbSwapWords(pData,2);
          sbZfCaMmNarrowPort1MemoryEntry_Unpack(&MmNarrowPort1MemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbZfCaMmNarrowPort1MemoryEntry_Print(&MmNarrowPort1MemoryRecord);
	  */
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
          sbZfCaMmWidePortMemoryEntry_Print(&MmWidePortMemoryRecord);
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
          sbZfCaMmInternal0MemoryEntry_Print(&MmInternal0MemoryRecord);
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
          sbZfCaMmInternal1MemoryEntry_Print(&MmInternal1MemoryRecord);
        }
    break;
    case FE2K_MEM_MM1NARROWPORT0MEMORY:
        status =
        sbFe2000ImfDriver_Mm1NarrowPort0MemoryRead(  uBaseAddress, 
                                                     uAddress, 
                                                     pData0, 
                                                     pData1); 
        if (status == SAND_DRV_CA_STATUS_OK) {
	  printk("Mm1NarrowPort0[@0x%06x]: 0x%08x 0x%08x\n",uAddress, *pData0, *pData1);
          sbSwapWords(pData,2);
          /* The output below can't be redirected to a file with diag shell "file=". */
          sbZfCaMmNarrowPort0MemoryEntry_Unpack(&MmNarrowPort0MemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbZfCaMmNarrowPort0MemoryEntry_Print(&MmNarrowPort0MemoryRecord);
        }
    break;
    case FE2K_MEM_MM1NARROWPORT1MEMORY:
        status =
        sbFe2000ImfDriver_Mm1NarrowPort1MemoryRead(  uBaseAddress, 
                                                     uAddress, 
                                                     pData0, 
                                                     pData1); 
        if (status == SAND_DRV_CA_STATUS_OK) {
	  printk("Mm1NarrowPort1[@0x%06x]: 0x%08x 0x%08x\n", uAddress, *pData0, *pData1);
          sbSwapWords(pData,2);
          /* The output below can't be redirected to a file with diag shell "file=". 	  */
	  sbZfCaMmNarrowPort1MemoryEntry_Unpack(&MmNarrowPort1MemoryRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbZfCaMmNarrowPort1MemoryEntry_Print(&MmNarrowPort1MemoryRecord);
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
          sbZfCaMmWidePortMemoryEntry_Print(&MmWidePortMemoryRecord);
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
          sbZfCaPbDataEntry_Print(&PbDataRecord);
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
          sbZfCaPbDataEntry_Print(&PbDataRecord);
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
          sbZfCaPtMirrorIndexEntry_Print(&PtMirrorIndexRecord);
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
          sbZfCaPtPortToQueueEntry_Print(&PtPortToQueueRecord);
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
          sbZfCaPtPortToQueueEntry_Print(&PtPortToQueueRecord);
        }
    break;
/* MCM - ? use PrPortXXX_Unpack - or - us PtPortToQueueEntry_Unpack ??? */
    case FE2K_MEM_PTPORTTOQUEUEPCI:
        status =
        sbFe2000ImfDriver_PtPortToQueuePciRead(  uBaseAddress, 
                                                 uAddress,   
                                                 pData);    
        if (status == SAND_DRV_CA_STATUS_OK) {
          sbSwapWords(pData,1);
          sbZfCaPrPortToQueuePciEntry_Unpack(&PrPortToQueuePciRecord,(uint8_t *)dbuf,sizeof(dbuf));
          sbZfCaPrPortToQueuePciEntry_Print(&PrPortToQueuePciRecord);
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
          sbZfCaPtPortToQueueEntry_Print(&PtPortToQueueRecord);
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
          sbZfCaPtPortToQueueEntry_Print(&PtPortToQueueRecord);
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
          sbZfCaPtPortToQueueEntry_Print(&PtPortToQueueRecord);
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
          sbZfCaPtPortToQueueEntry_Print(&PtPortToQueueRecord);
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
          sbZfCaPtPte0MirrorPortStateEntry_Print(&PtPte0MirrorPortStateRecord);
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
          sbZfCaPtPte0PortStateEntry_Print(&PtPte0PortStateRecord);
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
          sbZfCaPtPte1MirrorPortStateEntry_Print(&PtPte1MirrorPortStateRecord);
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
          sbZfCaPtPte1PortStateEntry_Print(&PtPte1PortStateRecord);
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
          sbZfCaPtPte2MirrorPortStateEntry_Print(&PtPte2MirrorPortStateRecord);
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
          sbZfCaPtPte2PortStateEntry_Print(&PtPte2PortStateRecord);
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
          sbZfCaPtPte3MirrorPortStateEntry_Print(&PtPte3MirrorPortStateRecord);
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
          sbZfCaPtPte3PortStateEntry_Print(&PtPte3PortStateRecord);
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
          sbZfCaPtPte4MirrorPortStateEntry_Print(&PtPte4MirrorPortStateRecord);
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
          sbZfCaPtPte4PortStateEntry_Print(&PtPte4PortStateRecord);
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
          sbZfCaPtPte5MirrorPortStateEntry_Print(&PtPte5MirrorPortStateRecord);
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
          sbZfCaPtPte5PortStateEntry_Print(&PtPte5PortStateRecord);
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
          sbZfCaPtPte6MirrorPortStateEntry_Print(&PtPte6MirrorPortStateRecord);
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
          sbZfCaPtPte6PortStateEntry_Print(&PtPte6PortStateRecord);
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
          sbZfCaPtQueueToPortEntry_Print(&PtQueueToPortRecord);
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
 * sbFe2000MemShowRange - call ShowEntry for a range of entries
 *	inputs:	unit number
 *		index of memory 
 *              start of range
 *              end of range
 *	output:	prints memory contents
 * 	return:	OK on success, else error status
 */
int
sbFe2000MemShowRange(int unit, int memindex, int rangemin, int rangemax)
{
    int i,  status;

    for(i = rangemin; i <= rangemax; i++) {
        status = sbFe2000MemShowEntry(unit, memindex,i);
        if(status != 0)
            return status;
    }
    return 0;
}

/* #include "sbFe2000MemWrite.txt"*/

/*
 * sbFe2000MemShow - read and display contents of fe2000 indirect memory
 *	inputs:	unit number
 *		name of memory 
 *	output:	prints memory contents
 * 	return:	OK on success, else error status
 */
int
sbFe2000MemShow(int unit, char *memname, int rangemin, int rangemax)
{
    int i, cnt, len, tempmax;
    int status = CMD_FAIL;

    cnt = 0;
    len = strlen(memname);
    for(i = 0; i < FE2K_MEM_MAX_INDEX; i++) {
        if ((memname[0] == '*') || (strncmp(memname,memConfigTable[i].memname,len) == 0)) {
             cnt++;
             printk("------------- %s ------------\n",memConfigTable[i].memname);
             tempmax = rangemax;
             if (rangemax > memConfigTable[i].rangemax){
                  tempmax = memConfigTable[i].rangemax;
                  printk("Warning: Max range for mem %s is %d\n",memname,tempmax);
             }
             status = sbFe2000MemShowRange(unit,memConfigTable[i].memindex,rangemin,tempmax);
        }
    }
    if(cnt == 0) {
        printk("Error: unrecognized Fe2000 memory: %s\n",memname);
        printk("Please pick from one of these:\n");
        sbFe2000ShowMemNames();
    } 
    return status;
}

int
sbFe2000MemShow_OLD(int unit, char *memname, int rangemin, int rangemax)
{
    int i, memindex, status;

    if (memname[0] == '*') {
       for(i = 0; i < FE2K_MEM_MAX_INDEX; i++) {
           printk("------------- %s ------------\n",memConfigTable[i].memname);
           status = sbFe2000MemShowRange(unit, i,0,0xff);
       }
    }
    else {
        memindex = sbFe2000WhichMem(memname);
        if(memindex == -1) {
            printk("Error: unrecognized Fe2000 memory: %s\n",memname);
            printk("Please pick from one of these:\n");
            sbFe2000ShowMemNames();
            status = SAND_DRV_CA_STATUS_OK;
        }
        else {
            if (rangemax > memConfigTable[memindex].rangemax){
                rangemax = memConfigTable[memindex].rangemax;
                printk("Warning: Max range for mem %s is %d\n",memname,rangemax);
            }
            status = sbFe2000MemShowRange(unit, memindex,rangemin,rangemax);
        }
    }
    return status;
}


#endif /* BCM_FE2000_SUPPORT */
