/*
 * $Id: sbBm3200MemAccess.c 1.10.20.2 Broadcom SDK $
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
 * File:        sbBm3200MemAccess.c
 * Purpose:     sbx commands to read/write BM3200 indirect mems
 * Requires:
 */

#include <appl/diag/shell.h>
#include <appl/diag/system.h>
#include <soc/defs.h>

#ifdef BCM_BME3200_SUPPORT

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_user.h>
#include <soc/sbx/bme3200.h>
#include <soc/sbx/bm3200_mem_access.h>
#include <appl/diag/sbx/sbx.h>
#include <appl/diag/sbx/imfswap32.h>

extern void bm3200SwapWords(uint *pdata, int nwords);

extern uint32
hwBm3200NmEmapReadWrite(sbhandle userDeviceHandle,
			uint32 addr,
			uint32* pData0,
			uint32* pData1,
			uint32* pData2,
			uint32* pData3,
			uint32* pData4,
			uint32  rd_wr_n);

extern uint32_t
hwBm3200InaMemoryReadWrite(sbhandle userDeviceHandle,
                           uint32_t     port,
                           uint32_t     sel,
                           uint32_t     addr,
                           uint32_t*    pData0,
                           uint32_t*    pData1,
                           uint32_t*    pData2,
                           uint32_t*    pData3,
                           uint32_t*    pData4,
                           uint32_t    rd_wr_n);


bm3200MemConfigRec bm3200MemConfigTable[] =
{
    {"PtBwBwpEntry", 		    BM3200_PTBWBWP		  , 1024, 0},
    {"PtBwPrtEntry", 		    BM3200_PTBWPRT		  , 64, 0},
    {"PtBwWctEntry", 		    BM3200_PTBWWCT		  , 256, 0},
    {"PtBwWdtEntry", 		    BM3200_PTBWWDT		  , 1024, 0},
    {"PtBwWstEntry", 		    BM3200_PTBWWST		  , 1024, 0},
    {"PtNextPriMemEntry", 	    BM3200_PTNEXTPRIMEM		  , 32, 70},
    {"PtNmEmapEntry", 		    BM3200_PTNMEMAP		  , 32, 0},
    {"PtPriMemEntry", 		    BM3200_PTPRIMEM		  , 32, 70},
    {"END",                         BM3200_MEM_MAX_INDEX          , 0xff, 0}
    /* Cmode only memories 
       {"PtBwDstEntry", 		    BM3200_PTBWDST		  , 32768, 0},
       {"PtBwLthrEntry", 		    BM3200_PTBWLTHR		  , 1024, 0},
       {"PtBwNPC2QEntry", 		    BM3200_PTBWNPC2Q		  , 2048, 0},
    */
    /* Cmode only memories 
       {"PtBwQ2NPCEntry", 		    BM3200_PTBWQ2NPC		  , 1024, 0},
       {"PtBwQlopEntry", 		    BM3200_PTBWQLOP		  , 640, 0},
       {"PtBwQltEntry", 		    BM3200_PTBWQLT		  , 32768, 0},
       {"PtBwWatEntry", 		    BM3200_PTBWWAT		  , 1024, 0},
    */
};

/*
 * just return for big endian 
 */
void 
bm3200SwapWords(uint *pdata, int nwords)
{
    int i;

    for(i = 0; i < nwords; i++) {
	*pdata = imfswap32(*pdata);
	pdata++;
    }
}


/*
 * scan bm3200MemConfigTable for designated name and return memIndex
 */
int
sbBm3200WhichMem(char *memname)
{
   int i;

   for(i = 0; i < BM3200_MEM_MAX_INDEX; i++) {
       if(strcmp(memname,bm3200MemConfigTable[i].memname) == 0)
          return bm3200MemConfigTable[i].memindex;
   }
   return -1;

}

int 
sbBm3200MemMax(int memindex)
{
    return bm3200MemConfigTable[memindex].rangemax;
}

void
sbBm3200ShowMemNames(void)
{
   int i;

   for(i = 0; i < BM3200_MEM_MAX_INDEX; i++) {
       if (strcmp("END",bm3200MemConfigTable[i].memname) == 0) {
	   break;
       }
       printk("--- %s\n",bm3200MemConfigTable[i].memname);
   }
}





/*
 * sbBm3200MemShowEntry - read and display contents of BM3200 indirect memory
 *	inputs:	 unit 
 *		name of memory 
 *	output:	prints memory contents
 * 	return:	OK on success, else error status
 */
int
sbBm3200MemShowEntry(int unit, int memindex, int entryindex, int instance)
{
    int  status = BM3200_STATUS_BAD_UNIT;

    sbZfFabBm3200BwBwpEntry_t		     BwBwpRecord;		       
    sbZfFabBm3200BwDstEntry_t		     BwDstRecord;		       
    sbZfFabBm3200BwLthrEntry_t		     BwLthrRecord;		       
    sbZfFabBm3200BwNPC2QEntry_t		     BwNPC2QRecord;		       
    sbZfFabBm3200BwPrtEntry_t		     BwPrtRecord;		       
    sbZfFabBm3200BwQ2NPCEntry_t		     BwQ2NPCRecord;		       
    sbZfFabBm3200BwQlopEntry_t		     BwQlopRecord;		       
    sbZfFabBm3200BwQltEntry_t		     BwQltRecord;		       
    sbZfFabBm3200BwWatEntry_t		     BwWatRecord;		       
    sbZfFabBm3200BwWctEntry_t		     BwWctRecord;		       
    sbZfFabBm3200BwWdtEntry_t		     BwWdtRecord;		       
    sbZfFabBm3200BwWstEntry_t		     BwWstRecord;		       
    sbZfFabBm3200NextPriMemEntry_t	     NextPriMemRecord;	       
    sbZfFabBm3200NmEmapEntry_t		     NmEmapRecord;		       
    sbZfFabBm3200PriMemEntry_t		     PriMemRecord;		       

    uint dbuf[40];
    uint    uAddress, *pData  = &dbuf[0];
    int i;

    uAddress = entryindex;
    printk("---  Entry index: %d ---\n",entryindex);
    switch(memindex) {
	case BM3200_PTBWBWP:
	    status = soc_bm3200_bw_mem_read(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_BWP,
					    0 /* repository */, uAddress, (uint32 *)(&(dbuf[0])));
	    if (status == BM3200_STATUS_OK) {
		sbZfFabBm3200BwBwpEntry_Unpack(&BwBwpRecord,(uint8_t *)dbuf,sizeof(dbuf));		    	
		sbZfFabBm3200BwBwpEntry_Print(&BwBwpRecord);
	    }
	    break;
	case BM3200_PTBWDST:
   	    status = soc_bm3200_bw_mem_read(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_DST,
					    0 /* repository */, uAddress, (uint32 *)(&(dbuf[0])));
	    if (status == BM3200_STATUS_OK) {
		sbZfFabBm3200BwDstEntry_Unpack(&BwDstRecord,(uint8_t *)dbuf,sizeof(dbuf));		    	
		sbZfFabBm3200BwDstEntry_Print(&BwDstRecord);
	    }
	    break;
	case BM3200_PTBWLTHR:   
   	    status = soc_bm3200_bw_mem_read(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_LTHR,
					    0 /* repository */, uAddress, (uint32 *)(&(dbuf[0])));
	    if (status == BM3200_STATUS_OK) {
		sbZfFabBm3200BwLthrEntry_Unpack(&BwLthrRecord,(uint8_t *)dbuf,sizeof(dbuf));		    	
		sbZfFabBm3200BwLthrEntry_Print(&BwLthrRecord);
	    }
	    break;
	case BM3200_PTBWNPC2Q:        
   	    status = soc_bm3200_bw_mem_read(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_NPC2Q,
					    0 /* repository */, uAddress, (uint32 *)(&(dbuf[0])));
	    if (status == BM3200_STATUS_OK) {
		sbZfFabBm3200BwNPC2QEntry_Unpack(&BwNPC2QRecord,(uint8_t *)dbuf,sizeof(dbuf));		    	
		sbZfFabBm3200BwNPC2QEntry_Print(&BwNPC2QRecord);
	    }
	    break;
	case BM3200_PTBWPRT:
   	    status = soc_bm3200_bw_mem_read(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_PRT,
					    0 /* repository */, uAddress, (uint32 *)(&(dbuf[0])));

   	    status = soc_bm3200_bw_mem_read(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_PRT,
					    1 /* repository */, uAddress, (uint32 *)(&(dbuf[1])));
	    if (status == BM3200_STATUS_OK) {
		sbZfFabBm3200BwPrtEntry_Unpack(&BwPrtRecord,(uint8_t *)dbuf,sizeof(dbuf));		    	
		sbZfFabBm3200BwPrtEntry_Print(&BwPrtRecord);
	    }
	    break;
	case BM3200_PTBWQ2NPC:        
   	    status = soc_bm3200_bw_mem_read(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_Q2NPC,
					    0 /* repository */, uAddress, (uint32 *)(&(dbuf[0])));
	    if (status == BM3200_STATUS_OK) {
		sbZfFabBm3200BwQ2NPCEntry_Unpack(&BwQ2NPCRecord,(uint8_t *)dbuf,sizeof(dbuf));		    	
		sbZfFabBm3200BwQ2NPCEntry_Print(&BwQ2NPCRecord);
	    }
	    break;
	case BM3200_PTBWQLOP:        
   	    status = soc_bm3200_bw_mem_read(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_QLOP,
					    0 /* repository */, uAddress, (uint32 *)(&(dbuf[0])));
	    if (status == BM3200_STATUS_OK) {
		sbZfFabBm3200BwQlopEntry_Unpack(&BwQlopRecord,(uint8_t *)dbuf,sizeof(dbuf));		    	
		sbZfFabBm3200BwQlopEntry_Print(&BwQlopRecord);
	    }
	    break;
	case BM3200_PTBWQLT:
   	    status = soc_bm3200_bw_mem_read(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_QLT,
					    1 /* repository */, uAddress, (uint32 *)(&(dbuf[0])));
	    if (status == BM3200_STATUS_OK) {
		sbZfFabBm3200BwQltEntry_Unpack(&BwQltRecord,(uint8_t *)dbuf,sizeof(dbuf));		    	
		sbZfFabBm3200BwQltEntry_Print(&BwQltRecord);
	    }
	    break;
	case BM3200_PTBWWAT:
   	    status = soc_bm3200_bw_mem_read(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_WAT,
					    1 /* repository */, uAddress, (uint32 *)(&(dbuf[0])));

   	    status = soc_bm3200_bw_mem_read(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_WAT,
					    0 /* repository */, uAddress, (uint32 *)(&(dbuf[1])));
	    if (status == BM3200_STATUS_OK) {
		sbZfFabBm3200BwWatEntry_Unpack(&BwWatRecord,(uint8_t *)dbuf,sizeof(dbuf));		    	
		sbZfFabBm3200BwWatEntry_Print(&BwWatRecord);
	    }
	    break;
	case BM3200_PTBWWCT:
#define HW_BM3200_WCT_ENTRIES_PER_TEMPLATE 8
	    uAddress = uAddress * HW_BM3200_WCT_ENTRIES_PER_TEMPLATE;
	    for (i = 0; i < HW_BM3200_WCT_ENTRIES_PER_TEMPLATE; i++) {
		status = soc_bm3200_bw_mem_read(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_WCT,
						1 /* repository */, uAddress, (uint32 *)(&(dbuf[i])));
		if (status != BM3200_STATUS_OK) {
		    break;
		}
		uAddress++;
	    }
	    if (status == BM3200_STATUS_OK) {
		sbZfFabBm3200BwWctEntry_Unpack(&BwWctRecord,(uint8_t *)dbuf,sizeof(dbuf));		    	
		sbZfFabBm3200BwWctEntry_Print(&BwWctRecord);
	    }
	    break;
	case BM3200_PTBWWDT:
   	    status = soc_bm3200_bw_mem_read(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_WDT,
					    0 /* repository */, uAddress, (uint32 *)(&(dbuf[0])));
	    if (status == BM3200_STATUS_OK) {
		sbZfFabBm3200BwWdtEntry_Unpack(&BwWdtRecord,(uint8_t *)dbuf,sizeof(dbuf));		    	
		sbZfFabBm3200BwWdtEntry_Print(&BwWdtRecord);
	    }
	    break;
	case BM3200_PTBWWST:
   	    status = soc_bm3200_bw_mem_read(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_WST,
					    1 /* repository */, uAddress, (uint32 *)(&(dbuf[0])));
	    if (status == BM3200_STATUS_OK) {
		sbZfFabBm3200BwWstEntry_Unpack(&BwWstRecord,(uint8_t *)dbuf,sizeof(dbuf));		    	
		sbZfFabBm3200BwWstEntry_Print(&BwWstRecord);
	    }
	    break;
	case BM3200_PTNEXTPRIMEM:        
	    status = hwBm3200InaMemoryReadWrite((sbhandle)unit, instance, 1 /* next pri */,
						uAddress,
						(uint32 *)(&(dbuf[0])),
						(uint32 *)(&(dbuf[1])),
						(uint32 *)(&(dbuf[2])),
						(uint32 *)(&(dbuf[3])),
						(uint32 *)(&(dbuf[4])),
						1 /* read */);
	    if (status == BM3200_STATUS_OK) {
		sbZfFabBm3200NextPriMemEntry_Unpack(&NextPriMemRecord,(uint8_t *)dbuf,sizeof(dbuf));	    	
		sbZfFabBm3200NextPriMemEntry_Print(&NextPriMemRecord);
	    }
	    break;
	case BM3200_PTNMEMAP:        
	    status = hwBm3200NmEmapReadWrite((sbhandle)unit, uAddress,
					     (uint32 *)(&(dbuf[0])),
					     (uint32 *)(&(dbuf[1])),
					     (uint32 *)(&(dbuf[2])),
					     (uint32 *)(&(dbuf[3])),
					     (uint32 *)(&(dbuf[4])),
					     1 /* read */);
	    if (status == BM3200_STATUS_OK) {
		sbZfFabBm3200NmEmapEntry_Unpack(&NmEmapRecord,(uint8_t *)dbuf,sizeof(dbuf));		    
		sbZfFabBm3200NmEmapEntry_Print(&NmEmapRecord);
	    }
	    break;
	case BM3200_PTPRIMEM:        
	    status = hwBm3200InaMemoryReadWrite((sbhandle)unit, instance, 0 /* pri */,
						uAddress,
						(uint32 *)(&(dbuf[0])),
						(uint32 *)(&(dbuf[1])),
						(uint32 *)(&(dbuf[2])),
						(uint32 *)(&(dbuf[3])),
						(uint32 *)(&(dbuf[4])),
						1 /* read */);
	    if (status == BM3200_STATUS_OK) {
		sbZfFabBm3200PriMemEntry_Unpack(&PriMemRecord,(uint8_t *)dbuf,sizeof(dbuf));		    
		sbZfFabBm3200PriMemEntry_Print(&PriMemRecord);
	    }
	    break;
	default:
	    status = BM3200_STATUS_BAD_READ;
	    break;
    }

    if (status != BM3200_STATUS_OK) {
	bm3200SwapWords(pData,1);
        printk("Error %d while reading Bm3200 memory\n",status);
    } 
    return 0;
}

/*
 * sbBm3200MemShowRange - call ShowEntry for a range of entries
 *	inputs:	unit
 *		index of memory 
 *              start of range
 *              end of range
 *	output:	prints memory contents
 * 	return:	OK on success, else error status
 */
int
sbBm3200MemShowRange(int unit, int memindex, int rangemin, int rangemax, int instance)
{
    int i,  status;

    for(i = rangemin; i <= rangemax; i++) {
        status = sbBm3200MemShowEntry(unit, memindex, i, instance);
        if(status != 0)
            return status;
    }
    return 0;
}

/*
 * sbBm3200MemShow - read and display contents of BM3200 indirect memory
 *	inputs:	unit number
 *		name of memory 
 *	output:	prints memory contents
 * 	return:	OK on success, else error status
 */
int
sbBm3200MemShow(int unit, char *memname, int rangemin, int rangemax, int instance)
{
    int i, cnt, len, tempmax, tempinstance;
    int status = CMD_FAIL;

    cnt = 0;
    len = strlen(memname);
    for(i = 0; i < BM3200_MEM_MAX_INDEX; i++) {
        if ((memname[0] == '*') || (strncmp(memname,bm3200MemConfigTable[i].memname,len) == 0)) {
             cnt++;
             printk("------------- %s ------------\n",bm3200MemConfigTable[i].memname);
             tempmax = rangemax;
             if (rangemax > bm3200MemConfigTable[i].rangemax){
                  tempmax = bm3200MemConfigTable[i].rangemax;
                  printk("Warning: Max range for mem %s is %d\n",memname,tempmax);
             }
	     tempinstance = instance;
             if (bm3200MemConfigTable[i].instances != 0 &&
		 (instance < 0 || instance >= bm3200MemConfigTable[i].instances)) {
                  tempinstance = 0;
                  printk("Warning: Mem %s is instanced: showing instance %d\n",memname,tempinstance);
             }
             status = sbBm3200MemShowRange(unit,bm3200MemConfigTable[i].memindex,rangemin,tempmax,tempinstance);
        }
    }
    if(cnt == 0) {
        printk("Error: unrecognized BM3200 memory: %s\n",memname);
        printk("Please pick from one of these:\n");
        sbBm3200ShowMemNames();
    } 
    return status;
}

/*
 * sbBm3200MemSetFieldEntry - read and change contents of bm3200 indirect memory
 *	inputs:	unit number
 *		name of memory 
 *	output:	prints memory contents
 * 	return:	OK on success, else error status
 */
int
sbBm3200MemSetFieldEntry(int unit, int memIndex, int uAddress, char *fieldname, int val, int instance)
{
    int  status = BM3200_STATUS_BAD_UNIT;
    
    sbZfFabBm3200BwBwpEntry_t		     BwBwpRecord;		       
    sbZfFabBm3200BwDstEntry_t		     BwDstRecord;		       
    sbZfFabBm3200BwLthrEntry_t		     BwLthrRecord;		       
    sbZfFabBm3200BwNPC2QEntry_t		     BwNPC2QRecord;		       
    sbZfFabBm3200BwPrtEntry_t		     BwPrtRecord;		       
    sbZfFabBm3200BwQ2NPCEntry_t		     BwQ2NPCRecord;		       
    sbZfFabBm3200BwQlopEntry_t		     BwQlopRecord;		       
    sbZfFabBm3200BwQltEntry_t		     BwQltRecord;		       
    sbZfFabBm3200BwWatEntry_t		     BwWatRecord;		       
    sbZfFabBm3200BwWctEntry_t		     BwWctRecord;		       
    sbZfFabBm3200BwWdtEntry_t		     BwWdtRecord;		       
    sbZfFabBm3200BwWstEntry_t		     BwWstRecord;		       
    sbZfFabBm3200NextPriMemEntry_t	     NextPriMemRecord;	       
    sbZfFabBm3200NmEmapEntry_t		     NmEmapRecord;		       
    sbZfFabBm3200PriMemEntry_t		     PriMemRecord;		       

    uint dbuf[40];
    int i;

    printk("---  Entry index: %d ---\n", uAddress);
    switch(memIndex) {
	case BM3200_PTBWBWP:
	    status = soc_bm3200_bw_mem_read(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_BWP,
					    0 /* repository */, uAddress, (uint32 *)(&(dbuf[0])));

	    if (status != BM3200_STATUS_OK) return BM3200_STATUS_BAD_READ;

	    sbZfFabBm3200BwBwpEntry_Unpack(&BwBwpRecord,(uint8_t *)dbuf,sizeof(dbuf));		    	

	    status = sbZfFabBm3200BwBwpEntry_SetField(&BwBwpRecord, fieldname, val);

	    if (status != BM3200_STATUS_OK) return BM3200_STATUS_BAD_FIELD;

	    sbZfFabBm3200BwBwpEntry_Pack(&BwBwpRecord,(uint8_t *)dbuf,sizeof(dbuf));

	    status = soc_bm3200_bw_mem_write(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_BWP,
					     0 /* repository */, uAddress, (uint32 )(dbuf[0]));

	    if (status != BM3200_STATUS_OK) return BM3200_STATUS_BAD_WRITE;      
	    break;
	case BM3200_PTBWDST:
   	    status = soc_bm3200_bw_mem_read(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_DST,
					    0 /* repository */, uAddress, (uint32 *)(&(dbuf[0])));


	    sbZfFabBm3200BwDstEntry_Unpack(&BwDstRecord,(uint8_t *)dbuf,sizeof(dbuf));		    	

	    if (status != BM3200_STATUS_OK) return BM3200_STATUS_BAD_READ;

	    sbZfFabBm3200BwDstEntry_Unpack(&BwDstRecord,(uint8_t *)dbuf,sizeof(dbuf));		    	

	    status = sbZfFabBm3200BwDstEntry_SetField(&BwDstRecord, fieldname, val);

	    if (status != BM3200_STATUS_OK) return BM3200_STATUS_BAD_FIELD;

	    sbZfFabBm3200BwDstEntry_Pack(&BwDstRecord,(uint8_t *)dbuf,sizeof(dbuf));

	    status = soc_bm3200_bw_mem_write(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_DST,
					     0 /* repository */, uAddress, (uint32 )(dbuf[0]));

	    if (status != BM3200_STATUS_OK) return BM3200_STATUS_BAD_WRITE;      
	    break;
	case BM3200_PTBWLTHR:   
   	    status = soc_bm3200_bw_mem_read(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_LTHR,
					    0 /* repository */, uAddress, (uint32 *)(&(dbuf[0])));
	    sbZfFabBm3200BwLthrEntry_Unpack(&BwLthrRecord,(uint8_t *)dbuf,sizeof(dbuf));		    	
	    if (status != BM3200_STATUS_OK) return BM3200_STATUS_BAD_READ;

	    sbZfFabBm3200BwLthrEntry_Unpack(&BwLthrRecord,(uint8_t *)dbuf,sizeof(dbuf));		    	

	    status = sbZfFabBm3200BwLthrEntry_SetField(&BwLthrRecord, fieldname, val);

	    if (status != BM3200_STATUS_OK) return BM3200_STATUS_BAD_FIELD;

	    sbZfFabBm3200BwLthrEntry_Pack(&BwLthrRecord,(uint8_t *)dbuf,sizeof(dbuf));

	    status = soc_bm3200_bw_mem_write(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_LTHR,
					     0 /* repository */, uAddress, (uint32 )(dbuf[0]));

	    if (status != BM3200_STATUS_OK) return BM3200_STATUS_BAD_WRITE;      
	    break;
	case BM3200_PTBWNPC2Q:        
   	    status = soc_bm3200_bw_mem_read(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_NPC2Q,
					    0 /* repository */, uAddress, (uint32 *)(&(dbuf[0])));
	    sbZfFabBm3200BwNPC2QEntry_Unpack(&BwNPC2QRecord,(uint8_t *)dbuf,sizeof(dbuf));		    	
	    if (status != BM3200_STATUS_OK) return BM3200_STATUS_BAD_READ;

	    sbZfFabBm3200BwNPC2QEntry_Unpack(&BwNPC2QRecord,(uint8_t *)dbuf,sizeof(dbuf));		    	

	    status = sbZfFabBm3200BwNPC2QEntry_SetField(&BwNPC2QRecord, fieldname, val);

	    if (status != BM3200_STATUS_OK) return BM3200_STATUS_BAD_FIELD;

	    sbZfFabBm3200BwNPC2QEntry_Pack(&BwNPC2QRecord,(uint8_t *)dbuf,sizeof(dbuf));

	    status = soc_bm3200_bw_mem_write(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_NPC2Q,
					     0 /* repository */, uAddress, (uint32 )(dbuf[0]));

	    if (status != BM3200_STATUS_OK) return BM3200_STATUS_BAD_WRITE;      
	    break;
	case BM3200_PTBWPRT:
   	    status = soc_bm3200_bw_mem_read(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_PRT,
					    1 /* repository */, uAddress, (uint32 *)(&(dbuf[0])));

   	    status = soc_bm3200_bw_mem_read(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_PRT,
					    0 /* repository */, uAddress, (uint32 *)(&(dbuf[1])));
	    sbZfFabBm3200BwPrtEntry_Unpack(&BwPrtRecord,(uint8_t *)dbuf,sizeof(dbuf));		    	
	    if (status != BM3200_STATUS_OK) return BM3200_STATUS_BAD_READ;

	    sbZfFabBm3200BwPrtEntry_Unpack(&BwPrtRecord,(uint8_t *)dbuf,sizeof(dbuf));		    	

	    status = sbZfFabBm3200BwPrtEntry_SetField(&BwPrtRecord, fieldname, val);

	    if (status != BM3200_STATUS_OK) return BM3200_STATUS_BAD_FIELD;

	    sbZfFabBm3200BwPrtEntry_Pack(&BwPrtRecord,(uint8_t *)dbuf,sizeof(dbuf));

   	    status = soc_bm3200_bw_mem_write(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_PRT,
					     1 /* repository */, uAddress, (uint32 )(dbuf[0]));

   	    status = soc_bm3200_bw_mem_write(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_PRT,
					     0 /* repository */, uAddress, (uint32 )(dbuf[1]));

	    if (status != BM3200_STATUS_OK) return BM3200_STATUS_BAD_WRITE;      

	    break;
	case BM3200_PTBWQ2NPC:        
   	    status = soc_bm3200_bw_mem_read(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_Q2NPC,
					    0 /* repository */, uAddress, (uint32 *)(&(dbuf[0])));
	    sbZfFabBm3200BwQ2NPCEntry_Unpack(&BwQ2NPCRecord,(uint8_t *)dbuf,sizeof(dbuf));		    	
	    if (status != BM3200_STATUS_OK) return BM3200_STATUS_BAD_READ;

	    sbZfFabBm3200BwQ2NPCEntry_Unpack(&BwQ2NPCRecord,(uint8_t *)dbuf,sizeof(dbuf));		    	

	    status = sbZfFabBm3200BwQ2NPCEntry_SetField(&BwQ2NPCRecord, fieldname, val);

	    if (status != BM3200_STATUS_OK) return BM3200_STATUS_BAD_FIELD;

	    sbZfFabBm3200BwQ2NPCEntry_Pack(&BwQ2NPCRecord,(uint8_t *)dbuf,sizeof(dbuf));

	    status = soc_bm3200_bw_mem_write(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_Q2NPC,
					     0 /* repository */, uAddress, (uint32 )(dbuf[0]));

	    if (status != BM3200_STATUS_OK) return BM3200_STATUS_BAD_WRITE;      

	    break;
	case BM3200_PTBWQLOP:        
   	    status = soc_bm3200_bw_mem_read(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_QLOP,
					    0 /* repository */, uAddress, (uint32 *)(&(dbuf[0])));
	    sbZfFabBm3200BwQlopEntry_Unpack(&BwQlopRecord,(uint8_t *)dbuf,sizeof(dbuf));		    	
	    if (status != BM3200_STATUS_OK) return BM3200_STATUS_BAD_READ;

	    sbZfFabBm3200BwQlopEntry_Unpack(&BwQlopRecord,(uint8_t *)dbuf,sizeof(dbuf));		    	

	    status = sbZfFabBm3200BwQlopEntry_SetField(&BwQlopRecord, fieldname, val);

	    if (status != BM3200_STATUS_OK) return BM3200_STATUS_BAD_FIELD;

	    sbZfFabBm3200BwQlopEntry_Pack(&BwQlopRecord,(uint8_t *)dbuf,sizeof(dbuf));

	    status = soc_bm3200_bw_mem_write(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_QLOP,
					     0 /* repository */, uAddress, (uint32 )(dbuf[0]));

	    if (status != BM3200_STATUS_OK) return BM3200_STATUS_BAD_WRITE;      

	    break;
	case BM3200_PTBWQLT:
   	    status = soc_bm3200_bw_mem_read(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_QLT,
					    1 /* repository */, uAddress, (uint32 *)(&(dbuf[0])));
	    sbZfFabBm3200BwQltEntry_Unpack(&BwQltRecord,(uint8_t *)dbuf,sizeof(dbuf));		    	
	    if (status != BM3200_STATUS_OK) return BM3200_STATUS_BAD_READ;

	    sbZfFabBm3200BwQltEntry_Unpack(&BwQltRecord,(uint8_t *)dbuf,sizeof(dbuf));		    	

	    status = sbZfFabBm3200BwQltEntry_SetField(&BwQltRecord, fieldname, val);

	    if (status != BM3200_STATUS_OK) return BM3200_STATUS_BAD_FIELD;

	    sbZfFabBm3200BwQltEntry_Pack(&BwQltRecord,(uint8_t *)dbuf,sizeof(dbuf));

	    status = soc_bm3200_bw_mem_write(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_QLT,
					     1 /* repository */, uAddress, (uint32 )(dbuf[0]));

	    if (status != BM3200_STATUS_OK) return BM3200_STATUS_BAD_WRITE;      

	    break;
	case BM3200_PTBWWAT:
   	    status = soc_bm3200_bw_mem_read(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_WAT,
					    1 /* repository */, uAddress, (uint32 *)(&(dbuf[0])));

   	    status = soc_bm3200_bw_mem_read(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_WAT,
					    0 /* repository */, uAddress, (uint32 *)(&(dbuf[1])));
	    sbZfFabBm3200BwWatEntry_Unpack(&BwWatRecord,(uint8_t *)dbuf,sizeof(dbuf));		    	
	    if (status != BM3200_STATUS_OK) return BM3200_STATUS_BAD_READ;

	    sbZfFabBm3200BwWatEntry_Unpack(&BwWatRecord,(uint8_t *)dbuf,sizeof(dbuf));		    	

	    status = sbZfFabBm3200BwWatEntry_SetField(&BwWatRecord, fieldname, val);

	    if (status != BM3200_STATUS_OK) return BM3200_STATUS_BAD_FIELD;

	    sbZfFabBm3200BwWatEntry_Pack(&BwWatRecord,(uint8_t *)dbuf,sizeof(dbuf));

	    status = soc_bm3200_bw_mem_write(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_WAT,
					     1 /* repository */, uAddress, (uint32 )(dbuf[0]));

	    status = soc_bm3200_bw_mem_write(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_WAT,
					     0 /* repository */, uAddress, (uint32 )(dbuf[1]));

	    if (status != BM3200_STATUS_OK) return BM3200_STATUS_BAD_WRITE;      

	    break;
	case BM3200_PTBWWCT:
	    uAddress = uAddress * HW_BM3200_WCT_ENTRIES_PER_TEMPLATE;
	    for (i = 0; i < HW_BM3200_WCT_ENTRIES_PER_TEMPLATE; i++) {
		status = soc_bm3200_bw_mem_read(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_WCT,
						1 /* repository */, uAddress, (uint32 *)(&(dbuf[i])));
		if (status != BM3200_STATUS_OK) {
		    break;
		}
		uAddress++;
	    }
	    sbZfFabBm3200BwWctEntry_Unpack(&BwWctRecord,(uint8_t *)dbuf,sizeof(dbuf));		    	
	    if (status != BM3200_STATUS_OK) return BM3200_STATUS_BAD_READ;

	    sbZfFabBm3200BwWctEntry_Unpack(&BwWctRecord,(uint8_t *)dbuf,sizeof(dbuf));		    	

	    status = sbZfFabBm3200BwWctEntry_SetField(&BwWctRecord, fieldname, val);

	    if (status != BM3200_STATUS_OK) return BM3200_STATUS_BAD_FIELD;

	    sbZfFabBm3200BwWctEntry_Pack(&BwWctRecord,(uint8_t *)dbuf,sizeof(dbuf));

	    uAddress -= HW_BM3200_WCT_ENTRIES_PER_TEMPLATE;
	    for (i = 0; i < HW_BM3200_WCT_ENTRIES_PER_TEMPLATE; i++) {
		status = soc_bm3200_bw_mem_write(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_WCT,
						1 /* repository */, uAddress, (uint32)(dbuf[i]));
		if (status != BM3200_STATUS_OK) {
		    break;
		}
		uAddress++;
	    }

	    if (status != BM3200_STATUS_OK) return BM3200_STATUS_BAD_WRITE;      

	    break;
	case BM3200_PTBWWDT:
   	    status = soc_bm3200_bw_mem_read(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_WDT,
					    0 /* repository */, uAddress, (uint32 *)(&(dbuf[0])));
	    sbZfFabBm3200BwWdtEntry_Unpack(&BwWdtRecord,(uint8_t *)dbuf,sizeof(dbuf));		    	
	    if (status != BM3200_STATUS_OK) return BM3200_STATUS_BAD_READ;

	    sbZfFabBm3200BwWdtEntry_Unpack(&BwWdtRecord,(uint8_t *)dbuf,sizeof(dbuf));		    	

	    status = sbZfFabBm3200BwWdtEntry_SetField(&BwWdtRecord, fieldname, val);

	    if (status != BM3200_STATUS_OK) return BM3200_STATUS_BAD_FIELD;

	    sbZfFabBm3200BwWdtEntry_Pack(&BwWdtRecord,(uint8_t *)dbuf,sizeof(dbuf));

	    status = soc_bm3200_bw_mem_write(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_WDT,
					     0 /* repository */, uAddress, (uint32 )(dbuf[0]));

	    if (status != BM3200_STATUS_OK) return BM3200_STATUS_BAD_WRITE;      

	    break;
	case BM3200_PTBWWST:
   	    status = soc_bm3200_bw_mem_read(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_WST,
					    1 /* repository */, uAddress, (uint32 *)(&(dbuf[0])));
	    sbZfFabBm3200BwWstEntry_Unpack(&BwWstRecord,(uint8_t *)dbuf,sizeof(dbuf));		    	
	    if (status != BM3200_STATUS_OK) return BM3200_STATUS_BAD_READ;

	    sbZfFabBm3200BwWstEntry_Unpack(&BwWstRecord,(uint8_t *)dbuf,sizeof(dbuf));		    	

	    status = sbZfFabBm3200BwWstEntry_SetField(&BwWstRecord, fieldname, val);

	    if (status != BM3200_STATUS_OK) return BM3200_STATUS_BAD_FIELD;

	    sbZfFabBm3200BwWstEntry_Pack(&BwWstRecord,(uint8_t *)dbuf,sizeof(dbuf));

	    status = soc_bm3200_bw_mem_write(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_WST,
					     1 /* repository */, uAddress, (uint32 )(dbuf[0]));

	    if (status != BM3200_STATUS_OK) return BM3200_STATUS_BAD_WRITE;      

	    break;
	case BM3200_PTNEXTPRIMEM:        
	    status = hwBm3200InaMemoryReadWrite((sbhandle)unit, instance, 1 /* next pri */,
						uAddress,
						(uint32 *)(&(dbuf[0])),
						(uint32 *)(&(dbuf[1])),
						(uint32 *)(&(dbuf[2])),
						(uint32 *)(&(dbuf[3])),
						(uint32 *)(&(dbuf[4])),
						1 /* read */);
	    sbZfFabBm3200NextPriMemEntry_Unpack(&NextPriMemRecord,(uint8_t *)dbuf,sizeof(dbuf));	    	
	    if (status != BM3200_STATUS_OK) return BM3200_STATUS_BAD_READ;

	    sbZfFabBm3200NextPriMemEntry_Unpack(&NextPriMemRecord,(uint8_t *)dbuf,sizeof(dbuf));		    	

	    status = sbZfFabBm3200NextPriMemEntry_SetField(&NextPriMemRecord, fieldname, val);

	    if (status != BM3200_STATUS_OK) return BM3200_STATUS_BAD_FIELD;

	    sbZfFabBm3200NextPriMemEntry_Pack(&NextPriMemRecord,(uint8_t *)dbuf,sizeof(dbuf));

	    status = hwBm3200InaMemoryReadWrite((sbhandle)unit, instance, 1 /* next pri */,
						uAddress,
						(uint32 *)(&(dbuf[0])),
						(uint32 *)(&(dbuf[1])),
						(uint32 *)(&(dbuf[2])),
						(uint32 *)(&(dbuf[3])),
						(uint32 *)(&(dbuf[4])),
						0 /* write */);

	    if (status != BM3200_STATUS_OK) return BM3200_STATUS_BAD_WRITE;      

	    break;
	case BM3200_PTNMEMAP:        
	    status = hwBm3200NmEmapReadWrite((sbhandle)unit, uAddress,
					     (uint32 *)(&(dbuf[0])),
					     (uint32 *)(&(dbuf[1])),
					     (uint32 *)(&(dbuf[2])),
					     (uint32 *)(&(dbuf[3])),
					     (uint32 *)(&(dbuf[4])),
					     1 /* read */);
	    sbZfFabBm3200NmEmapEntry_Unpack(&NmEmapRecord,(uint8_t *)dbuf,sizeof(dbuf));		    
	    if (status != BM3200_STATUS_OK) return BM3200_STATUS_BAD_READ;

	    sbZfFabBm3200NmEmapEntry_Unpack(&NmEmapRecord,(uint8_t *)dbuf,sizeof(dbuf));		    	

	    status = sbZfFabBm3200NmEmapEntry_SetField(&NmEmapRecord, fieldname, val);

	    if (status != BM3200_STATUS_OK) return BM3200_STATUS_BAD_FIELD;

	    sbZfFabBm3200NmEmapEntry_Pack(&NmEmapRecord,(uint8_t *)dbuf,sizeof(dbuf));

	    status = hwBm3200NmEmapReadWrite((sbhandle)unit, uAddress,
					     (uint32 *)(&(dbuf[0])),
					     (uint32 *)(&(dbuf[1])),
					     (uint32 *)(&(dbuf[2])),
					     (uint32 *)(&(dbuf[3])),
					     (uint32 *)(&(dbuf[4])),
					     0 /* write */);
	    if (status != BM3200_STATUS_OK) return BM3200_STATUS_BAD_WRITE;      

	    break;
	case BM3200_PTPRIMEM:        
	    status = hwBm3200InaMemoryReadWrite((sbhandle)unit, instance, 0 /* pri */,
						uAddress,
						(uint32 *)(&(dbuf[0])),
						(uint32 *)(&(dbuf[1])),
						(uint32 *)(&(dbuf[2])),
						(uint32 *)(&(dbuf[3])),
						(uint32 *)(&(dbuf[4])),
						1 /* read */);
	    sbZfFabBm3200PriMemEntry_Unpack(&PriMemRecord,(uint8_t *)dbuf,sizeof(dbuf));		    
	    if (status != BM3200_STATUS_OK) return BM3200_STATUS_BAD_READ;

	    sbZfFabBm3200PriMemEntry_Unpack(&PriMemRecord,(uint8_t *)dbuf,sizeof(dbuf));		    	

	    status = sbZfFabBm3200PriMemEntry_SetField(&PriMemRecord, fieldname, val);

	    if (status != BM3200_STATUS_OK) return BM3200_STATUS_BAD_FIELD;

	    sbZfFabBm3200PriMemEntry_Pack(&PriMemRecord,(uint8_t *)dbuf,sizeof(dbuf));

	    status = hwBm3200InaMemoryReadWrite((sbhandle)unit, instance, 0 /* pri */,
						uAddress,
						(uint32 *)(&(dbuf[0])),
						(uint32 *)(&(dbuf[1])),
						(uint32 *)(&(dbuf[2])),
						(uint32 *)(&(dbuf[3])),
						(uint32 *)(&(dbuf[4])),
						0 /* write */);

	    if (status != BM3200_STATUS_OK) return BM3200_STATUS_BAD_WRITE;      

	    break;
	default:
	    status = BM3200_STATUS_BAD_READ;
	    break;
    }
    
    return status;
}

/*
 * sbBm3200MemSetField - find memory and field names and assign value
 *	inputs:	unit number
 *		name of memory 
 *	output:	prints memory contents
 * 	return:	OK on success, else error status
 */
int
sbBm3200MemSetField(int unit, char *memFieldName, int addr, int val, int instance)
{
    int i, memindex, status, doingfield, max;
    char *dptr, curch, memname[100], tmpfield[100], memfield[100];

    sal_memset(tmpfield, 0x0, sizeof(tmpfield));

   /* memfield comes in as mem.field, so split memname and field name */
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
            dptr = &tmpfield[0];
            doingfield = 1;
        }
        else {
            if (doingfield)
                *dptr++ = tolower((unsigned char)curch);
            else
                *dptr++ = curch;
        }
    }

    memindex = sbBm3200WhichMem(memname);
    if(memindex == -1) {
        printk("Error: unrecognized Bm3200 memory: %s\n",memname);
        printk("Please pick from one of these:\n");
        sbBm3200ShowMemNames();
        return CMD_FAIL;
    }
    max = sbBm3200MemMax(memindex);
    if (addr > max){
        printk("Error: Addr %d out of range for memory: %s max=%d\n",
                           addr,memname,max);
        return CMD_FAIL;
    }
    if (bm3200MemConfigTable[memindex].instances != 0 &&
	(instance < 0 || instance >= bm3200MemConfigTable[memindex].instances)) {
	printk("Error: Mem %s is instanced and does not contain instance %d\n",memname, instance);
    }

    SB_STRCPY(memfield, "m_n");
    SB_STRCAT(memfield, tmpfield);

    printk("MemSetField unit=%d memIndex =%d instance=%d\n",unit,memindex, instance);

    status = sbBm3200MemSetFieldEntry(unit, memindex,addr,memfield, val, instance);
    switch(status) {
        case  BM3200_STATUS_BAD_READ:
           printk("Error Reading Bm3200 memory\n");
           break;
        case  BM3200_STATUS_BAD_WRITE:
           printk("Error Writing Bm3200 memory\n");
           break;
        case  BM3200_STATUS_BAD_FIELD:
           printk("Error: unrecognized field <%s> for this Bm3200 memory: %s\n",&memfield[3],memname);
           break;
    }
    if (status != BM3200_STATUS_OK) {
        status = CMD_USAGE;
    }
    return status;
}



#endif /* BCM_BME3200_SUPPORT */
