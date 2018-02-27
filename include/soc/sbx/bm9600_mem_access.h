/*
 * $Id: bm9600_mem_access.h 1.6 Broadcom SDK $
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
 * This file contains definitions for BM9600 memory access.
 */

#ifndef _SOC_SBX_BM9600_MEM_ACCESS_H
#define _SOC_SBX_BM9600_MEM_ACCESS_H

#include <soc/sbx/sbTypesGlue.h>

#define BM9600_STATUS_OK         0
#define BM9600_STATUS_BAD_UNIT  -1
#define BM9600_STATUS_BAD_READ  -2
#define BM9600_STATUS_BAD_WRITE -3
#define BM9600_STATUS_BAD_FIELD -4

typedef struct bm9k6MemConfigRec_s {
      char memname[30];
      int  memindex;
      int  rangemax;
      int  instances;
} bm9k6MemConfigRec;

extern void sbBm9600ShowMemNames(void);

/*
 * include other specific structs defined for zframes
 */
#include "sbZfFabBm9600BwAllocCfgBaseEntry.hx"
#include "sbZfFabBm9600BwAllocRateEntry.hx" 
#include "sbZfFabBm9600BwFetchDataEntry.hx" 
#include "sbZfFabBm9600BwFetchSumEntry.hx"  
#include "sbZfFabBm9600BwFetchValidEntry.hx"
#include "sbZfFabBm9600BwR0BagEntry.hx" 
#include "sbZfFabBm9600BwR0BwpEntry.hx" 
#include "sbZfFabBm9600BwR0WdtEntry.hx" 
#include "sbZfFabBm9600BwR1BagEntry.hx" 
#include "sbZfFabBm9600BwR1Wct0AEntry.hx"
#include "sbZfFabBm9600BwR1Wct0BEntry.hx"
#include "sbZfFabBm9600BwR1Wct1AEntry.hx"
#include "sbZfFabBm9600BwR1Wct1BEntry.hx"
#include "sbZfFabBm9600BwR1Wct2AEntry.hx"
#include "sbZfFabBm9600BwR1Wct2BEntry.hx"
#include "sbZfFabBm9600BwR1WstEntry.hx"  
#include "sbZfFabBm9600BwWredCfgBaseEntry.hx"
#include "sbZfFabBm9600BwWredDropNPart1Entry.hx"
#include "sbZfFabBm9600BwWredDropNPart2Entry.hx"
#include "sbZfFabBm9600FoLinkStateTableEntry.hx"
#include "sbZfFabBm9600InaEsetPriEntry.hx"  
#include "sbZfFabBm9600InaHi1Selected_0Entry.hx"
#include "sbZfFabBm9600InaHi1Selected_1Entry.hx"
#include "sbZfFabBm9600InaHi2Selected_0Entry.hx"
#include "sbZfFabBm9600InaHi2Selected_1Entry.hx"
#include "sbZfFabBm9600InaHi3Selected_0Entry.hx"
#include "sbZfFabBm9600InaHi3Selected_1Entry.hx"
#include "sbZfFabBm9600InaHi4Selected_0Entry.hx"
#include "sbZfFabBm9600InaHi4Selected_1Entry.hx"
#include "sbZfFabBm9600InaPortPriEntry.hx"  
#include "sbZfFabBm9600InaRandomNumGenEntry.hx"
#include "sbZfFabBm9600InaSysportMapEntry.hx"  
#include "sbZfFabBm9600NmEgressRankerEntry.hx" 
#include "sbZfFabBm9600NmFullStatusEntry.hx"
#include "sbZfFabBm9600NmEmtEntry.hx"  
#include "sbZfFabBm9600NmEmt_0Entry.hx"
#include "sbZfFabBm9600NmEmt_0_1Entry.hx" 
#include "sbZfFabBm9600NmEmt_1Entry.hx"  
#include "sbZfFabBm9600NmEmtdebugbank0Entry.hx"
#include "sbZfFabBm9600NmEmtdebugbank1Entry.hx"
#include "sbZfFabBm9600NmIngressRankerEntry.hx"
#include "sbZfFabBm9600NmPortsetInfoEntry.hx"  
#include "sbZfFabBm9600NmPortsetLinkEntry.hx"  
#include "sbZfFabBm9600NmRandomNumGenEntry.hx" 
#include "sbZfFabBm9600NmSysportArrayEntry.hx" 
#include "sbZfFabBm9600XbXcfgRemapEntry.hx"  

#include "sbZfFabBm9600BwAllocCfgBaseEntryConsole.hx"
#include "sbZfFabBm9600BwAllocRateEntryConsole.hx" 
#include "sbZfFabBm9600BwFetchDataEntryConsole.hx" 
#include "sbZfFabBm9600BwFetchSumEntryConsole.hx"  
#include "sbZfFabBm9600BwFetchValidEntryConsole.hx"
#include "sbZfFabBm9600BwR0BagEntryConsole.hx" 
#include "sbZfFabBm9600BwR0BwpEntryConsole.hx" 
#include "sbZfFabBm9600BwR0WdtEntryConsole.hx" 
#include "sbZfFabBm9600BwR1BagEntryConsole.hx" 
#include "sbZfFabBm9600BwR1Wct0AEntryConsole.hx"
#include "sbZfFabBm9600BwR1Wct0BEntryConsole.hx"
#include "sbZfFabBm9600BwR1Wct1AEntryConsole.hx"
#include "sbZfFabBm9600BwR1Wct1BEntryConsole.hx"
#include "sbZfFabBm9600BwR1Wct2AEntryConsole.hx"
#include "sbZfFabBm9600BwR1Wct2BEntryConsole.hx"
#include "sbZfFabBm9600BwR1WstEntryConsole.hx"  
#include "sbZfFabBm9600BwWredCfgBaseEntryConsole.hx"
#include "sbZfFabBm9600BwWredDropNPart1EntryConsole.hx"
#include "sbZfFabBm9600BwWredDropNPart2EntryConsole.hx"
#include "sbZfFabBm9600FoLinkStateTableEntryConsole.hx"
#include "sbZfFabBm9600InaEsetPriEntryConsole.hx"  
#include "sbZfFabBm9600InaHi1Selected_0EntryConsole.hx"
#include "sbZfFabBm9600InaHi1Selected_1EntryConsole.hx"
#include "sbZfFabBm9600InaHi2Selected_0EntryConsole.hx"
#include "sbZfFabBm9600InaHi2Selected_1EntryConsole.hx"
#include "sbZfFabBm9600InaHi3Selected_0EntryConsole.hx"
#include "sbZfFabBm9600InaHi3Selected_1EntryConsole.hx"
#include "sbZfFabBm9600InaHi4Selected_0EntryConsole.hx"
#include "sbZfFabBm9600InaHi4Selected_1EntryConsole.hx"
#include "sbZfFabBm9600InaPortPriEntryConsole.hx"  
#include "sbZfFabBm9600InaRandomNumGenEntryConsole.hx"
#include "sbZfFabBm9600InaSysportMapEntryConsole.hx"  
#include "sbZfFabBm9600NmEgressRankerEntryConsole.hx" 
#include "sbZfFabBm9600NmFullStatusEntryConsole.hx" 
#include "sbZfFabBm9600NmEmtEntryConsole.hx"  
#include "sbZfFabBm9600NmEmt_0EntryConsole.hx"
#include "sbZfFabBm9600NmEmt_0_1EntryConsole.hx" 
#include "sbZfFabBm9600NmEmt_1EntryConsole.hx"  
#include "sbZfFabBm9600NmEmtdebugbank0EntryConsole.hx"
#include "sbZfFabBm9600NmEmtdebugbank1EntryConsole.hx"
#include "sbZfFabBm9600NmIngressRankerEntryConsole.hx"
#include "sbZfFabBm9600NmPortsetInfoEntryConsole.hx"  
#include "sbZfFabBm9600NmPortsetLinkEntryConsole.hx"  
#include "sbZfFabBm9600NmRandomNumGenEntryConsole.hx" 
#include "sbZfFabBm9600NmSysportArrayEntryConsole.hx" 
#include "sbZfFabBm9600XbXcfgRemapEntryConsole.hx"  


enum {
BM9600_PLBWALLOCCFGBASE = 0,
BM9600_PLBWALLOCRATE,
BM9600_PLBWFETCHDATA,
BM9600_PLBWFETCHSUM, 
BM9600_PLBWFETCHVALID,
BM9600_PLBWR0BAG, 
BM9600_PLBWR0BWP, 
BM9600_PLBWR0WDT, 
BM9600_PLBWR1BAG, 
BM9600_PLBWR1WCT0A,
BM9600_PLBWR1WCT0B,
BM9600_PLBWR1WCT1A,
BM9600_PLBWR1WCT1B,
BM9600_PLBWR1WCT2A,
BM9600_PLBWR1WCT2B,
BM9600_PLBWR1WST, 
BM9600_PLBWWREDCFGBASE,
BM9600_PLBWWREDDROPNPART1,
BM9600_PLBWWREDDROPNPART2,
BM9600_PLFOLINKSTATETABLE,
BM9600_PLINAESETPRI, 
BM9600_PLINAHI1SELECTED_0,
BM9600_PLINAHI1SELECTED_1,
BM9600_PLINAHI2SELECTED_0,
BM9600_PLINAHI2SELECTED_1,
BM9600_PLINAHI3SELECTED_0,
BM9600_PLINAHI3SELECTED_1,
BM9600_PLINAHI4SELECTED_0,
BM9600_PLINAHI4SELECTED_1,
BM9600_PLINAPORTPRI, 
BM9600_PLINARANDOMNUMGEN,
BM9600_PLINASYSPORTMAP, 
BM9600_PLNMEGRESSRANKER,
BM9600_PLNMFULLSTATUS,
BM9600_PLNMEMT,
BM9600_PLNMEMTDEBUGBANK0,
BM9600_PLNMEMTDEBUGBANK1,
BM9600_PLNMINGRESSRANKER,
BM9600_PLNMPORTSETINFO, 
BM9600_PLNMPORTSETLINK, 
BM9600_PLNMRANDOMNUMGEN,
BM9600_PLNMSYSPORTARRAY,
BM9600_PLXBXCFGREMAP, 
#ifndef _NO_NM_CACHE_FIX_
BM9600_PLNMEMT_HW,
BM9600_PLNMPORTSETINFO_HW, 
BM9600_PLNMPORTSETLINK_HW, 
BM9600_PLNMSYSPORTARRAY_HW,
#endif /* _NO_NM_CACHE_FIX_ */
BM9600_MEM_MAX_INDEX
};

#endif  /* !_SOC_SBX_BM9600_MEM_ACCESS_H */
