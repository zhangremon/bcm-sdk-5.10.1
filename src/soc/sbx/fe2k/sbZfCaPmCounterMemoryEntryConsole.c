/*
 * $Id: sbZfCaPmCounterMemoryEntryConsole.c 1.3.36.3 Broadcom SDK $
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
 */
#include "sbTypes.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfCaPmCounterMemoryEntryConsole.hx"



/* Print members in struct */
void
sbZfCaPmCounterMemoryEntry_Print(sbZfCaPmCounterMemoryEntry_t *pFromStruct) {
  SB_LOG("CaPmCounterMemoryEntry:: bytecntr15=0x%09llx", (uint64_t)  pFromStruct->m_uuByteCntr15);
  SB_LOG(" pktcntr15=0x%08x", (unsigned int)  pFromStruct->m_uPktCntr15);
  SB_LOG("\n");

  SB_LOG("CaPmCounterMemoryEntry:: bytecntr14=0x%09llx", (uint64_t)  pFromStruct->m_uuByteCntr14);
  SB_LOG(" pktcntr14=0x%08x", (unsigned int)  pFromStruct->m_uPktCntr14);
  SB_LOG("\n");

  SB_LOG("CaPmCounterMemoryEntry:: bytecntr13=0x%09llx", (uint64_t)  pFromStruct->m_uuByteCntr13);
  SB_LOG(" pktcntr13=0x%08x", (unsigned int)  pFromStruct->m_uPktCntr13);
  SB_LOG("\n");

  SB_LOG("CaPmCounterMemoryEntry:: bytecntr12=0x%09llx", (uint64_t)  pFromStruct->m_uuByteCntr12);
  SB_LOG(" pktcntr12=0x%08x", (unsigned int)  pFromStruct->m_uPktCntr12);
  SB_LOG("\n");

  SB_LOG("CaPmCounterMemoryEntry:: bytecntr11=0x%09llx", (uint64_t)  pFromStruct->m_uuByteCntr11);
  SB_LOG(" pktcntr11=0x%08x", (unsigned int)  pFromStruct->m_uPktCntr11);
  SB_LOG("\n");

  SB_LOG("CaPmCounterMemoryEntry:: bytecntr10=0x%09llx", (uint64_t)  pFromStruct->m_uuByteCntr10);
  SB_LOG(" pktcntr10=0x%08x", (unsigned int)  pFromStruct->m_uPktCntr10);
  SB_LOG("\n");

  SB_LOG("CaPmCounterMemoryEntry:: bytecntr9=0x%09llx", (uint64_t)  pFromStruct->m_uuByteCntr9);
  SB_LOG(" pktcntr9=0x%08x", (unsigned int)  pFromStruct->m_uPktCntr9);
  SB_LOG("\n");

  SB_LOG("CaPmCounterMemoryEntry:: bytecntr8=0x%09llx", (uint64_t)  pFromStruct->m_uuByteCntr8);
  SB_LOG(" pktcntr8=0x%08x", (unsigned int)  pFromStruct->m_uPktCntr8);
  SB_LOG("\n");

  SB_LOG("CaPmCounterMemoryEntry:: bytecntr7=0x%09llx", (uint64_t)  pFromStruct->m_uuByteCntr7);
  SB_LOG(" pktcntr7=0x%08x", (unsigned int)  pFromStruct->m_uPktCntr7);
  SB_LOG("\n");

  SB_LOG("CaPmCounterMemoryEntry:: bytecntr6=0x%09llx", (uint64_t)  pFromStruct->m_uuByteCntr6);
  SB_LOG(" pktcntr6=0x%08x", (unsigned int)  pFromStruct->m_uPktCntr6);
  SB_LOG("\n");

  SB_LOG("CaPmCounterMemoryEntry:: bytecntr5=0x%09llx", (uint64_t)  pFromStruct->m_uuByteCntr5);
  SB_LOG(" pktcntr5=0x%08x", (unsigned int)  pFromStruct->m_uPktCntr5);
  SB_LOG("\n");

  SB_LOG("CaPmCounterMemoryEntry:: bytecntr4=0x%09llx", (uint64_t)  pFromStruct->m_uuByteCntr4);
  SB_LOG(" pktcntr4=0x%08x", (unsigned int)  pFromStruct->m_uPktCntr4);
  SB_LOG("\n");

  SB_LOG("CaPmCounterMemoryEntry:: bytecntr3=0x%09llx", (uint64_t)  pFromStruct->m_uuByteCntr3);
  SB_LOG(" pktcntr3=0x%08x", (unsigned int)  pFromStruct->m_uPktCntr3);
  SB_LOG("\n");

  SB_LOG("CaPmCounterMemoryEntry:: bytecntr2=0x%09llx", (uint64_t)  pFromStruct->m_uuByteCntr2);
  SB_LOG(" pktcntr2=0x%08x", (unsigned int)  pFromStruct->m_uPktCntr2);
  SB_LOG("\n");

  SB_LOG("CaPmCounterMemoryEntry:: bytecntr1=0x%09llx", (uint64_t)  pFromStruct->m_uuByteCntr1);
  SB_LOG(" pktcntr1=0x%08x", (unsigned int)  pFromStruct->m_uPktCntr1);
  SB_LOG("\n");

  SB_LOG("CaPmCounterMemoryEntry:: bytecntr0=0x%09llx", (uint64_t)  pFromStruct->m_uuByteCntr0);
  SB_LOG(" pktcntr0=0x%08x", (unsigned int)  pFromStruct->m_uPktCntr0);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfCaPmCounterMemoryEntry_SPrint(sbZfCaPmCounterMemoryEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaPmCounterMemoryEntry:: bytecntr15=0x%09llx", (uint64_t)  pFromStruct->m_uuByteCntr15);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pktcntr15=0x%08x", (unsigned int)  pFromStruct->m_uPktCntr15);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaPmCounterMemoryEntry:: bytecntr14=0x%09llx", (uint64_t)  pFromStruct->m_uuByteCntr14);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pktcntr14=0x%08x", (unsigned int)  pFromStruct->m_uPktCntr14);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaPmCounterMemoryEntry:: bytecntr13=0x%09llx", (uint64_t)  pFromStruct->m_uuByteCntr13);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pktcntr13=0x%08x", (unsigned int)  pFromStruct->m_uPktCntr13);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaPmCounterMemoryEntry:: bytecntr12=0x%09llx", (uint64_t)  pFromStruct->m_uuByteCntr12);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pktcntr12=0x%08x", (unsigned int)  pFromStruct->m_uPktCntr12);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaPmCounterMemoryEntry:: bytecntr11=0x%09llx", (uint64_t)  pFromStruct->m_uuByteCntr11);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pktcntr11=0x%08x", (unsigned int)  pFromStruct->m_uPktCntr11);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaPmCounterMemoryEntry:: bytecntr10=0x%09llx", (uint64_t)  pFromStruct->m_uuByteCntr10);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pktcntr10=0x%08x", (unsigned int)  pFromStruct->m_uPktCntr10);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaPmCounterMemoryEntry:: bytecntr9=0x%09llx", (uint64_t)  pFromStruct->m_uuByteCntr9);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pktcntr9=0x%08x", (unsigned int)  pFromStruct->m_uPktCntr9);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaPmCounterMemoryEntry:: bytecntr8=0x%09llx", (uint64_t)  pFromStruct->m_uuByteCntr8);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pktcntr8=0x%08x", (unsigned int)  pFromStruct->m_uPktCntr8);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaPmCounterMemoryEntry:: bytecntr7=0x%09llx", (uint64_t)  pFromStruct->m_uuByteCntr7);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pktcntr7=0x%08x", (unsigned int)  pFromStruct->m_uPktCntr7);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaPmCounterMemoryEntry:: bytecntr6=0x%09llx", (uint64_t)  pFromStruct->m_uuByteCntr6);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pktcntr6=0x%08x", (unsigned int)  pFromStruct->m_uPktCntr6);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaPmCounterMemoryEntry:: bytecntr5=0x%09llx", (uint64_t)  pFromStruct->m_uuByteCntr5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pktcntr5=0x%08x", (unsigned int)  pFromStruct->m_uPktCntr5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaPmCounterMemoryEntry:: bytecntr4=0x%09llx", (uint64_t)  pFromStruct->m_uuByteCntr4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pktcntr4=0x%08x", (unsigned int)  pFromStruct->m_uPktCntr4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaPmCounterMemoryEntry:: bytecntr3=0x%09llx", (uint64_t)  pFromStruct->m_uuByteCntr3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pktcntr3=0x%08x", (unsigned int)  pFromStruct->m_uPktCntr3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaPmCounterMemoryEntry:: bytecntr2=0x%09llx", (uint64_t)  pFromStruct->m_uuByteCntr2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pktcntr2=0x%08x", (unsigned int)  pFromStruct->m_uPktCntr2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaPmCounterMemoryEntry:: bytecntr1=0x%09llx", (uint64_t)  pFromStruct->m_uuByteCntr1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pktcntr1=0x%08x", (unsigned int)  pFromStruct->m_uPktCntr1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaPmCounterMemoryEntry:: bytecntr0=0x%09llx", (uint64_t)  pFromStruct->m_uuByteCntr0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pktcntr0=0x%08x", (unsigned int)  pFromStruct->m_uPktCntr0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfCaPmCounterMemoryEntry_Validate(sbZfCaPmCounterMemoryEntry_t *pZf) {

  if (pZf->m_uuByteCntr15 > 0x7FFFFFFFFULL) return 0;
  if (pZf->m_uPktCntr15 > 0x1fffffff) return 0;
  if (pZf->m_uuByteCntr14 > 0x7FFFFFFFFULL) return 0;
  if (pZf->m_uPktCntr14 > 0x1fffffff) return 0;
  if (pZf->m_uuByteCntr13 > 0xFFFFFFFFFULL) return 0;
  if (pZf->m_uPktCntr13 > 0x1fffffff) return 0;
  if (pZf->m_uuByteCntr12 > 0x7FFFFFFFFULL) return 0;
  if (pZf->m_uPktCntr12 > 0x1fffffff) return 0;
  if (pZf->m_uuByteCntr11 > 0xFFFFFFFFFULL) return 0;
  if (pZf->m_uPktCntr11 > 0x1fffffff) return 0;
  if (pZf->m_uuByteCntr10 > 0x7FFFFFFFFULL) return 0;
  if (pZf->m_uPktCntr10 > 0x1fffffff) return 0;
  if (pZf->m_uuByteCntr9 > 0x7FFFFFFFFULL) return 0;
  if (pZf->m_uPktCntr9 > 0x1fffffff) return 0;
  if (pZf->m_uuByteCntr8 > 0xFFFFFFFFFULL) return 0;
  if (pZf->m_uPktCntr8 > 0x1fffffff) return 0;
  if (pZf->m_uuByteCntr7 > 0x7FFFFFFFFULL) return 0;
  if (pZf->m_uPktCntr7 > 0x1fffffff) return 0;
  if (pZf->m_uuByteCntr6 > 0x7FFFFFFFFULL) return 0;
  if (pZf->m_uPktCntr6 > 0x1fffffff) return 0;
  if (pZf->m_uuByteCntr5 > 0x7FFFFFFFFULL) return 0;
  if (pZf->m_uPktCntr5 > 0x1fffffff) return 0;
  if (pZf->m_uuByteCntr4 > 0x7FFFFFFFFULL) return 0;
  if (pZf->m_uPktCntr4 > 0x1fffffff) return 0;
  if (pZf->m_uuByteCntr3 > 0x7FFFFFFFFULL) return 0;
  if (pZf->m_uPktCntr3 > 0x1fffffff) return 0;
  if (pZf->m_uuByteCntr2 > 0x7FFFFFFFFULL) return 0;
  if (pZf->m_uPktCntr2 > 0x1fffffff) return 0;
  if (pZf->m_uuByteCntr1 > 0x7FFFFFFFFULL) return 0;
  if (pZf->m_uPktCntr1 > 0x1fffffff) return 0;
  if (pZf->m_uuByteCntr0 > 0x7FFFFFFFFULL) return 0;
  if (pZf->m_uPktCntr0 > 0x1fffffff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfCaPmCounterMemoryEntry_SetField(sbZfCaPmCounterMemoryEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_uubytecntr15") == 0) {
    s->m_uuByteCntr15 = value;
  } else if (SB_STRCMP(name, "m_upktcntr15") == 0) {
    s->m_uPktCntr15 = value;
  } else if (SB_STRCMP(name, "m_uubytecntr14") == 0) {
    s->m_uuByteCntr14 = value;
  } else if (SB_STRCMP(name, "m_upktcntr14") == 0) {
    s->m_uPktCntr14 = value;
  } else if (SB_STRCMP(name, "m_uubytecntr13") == 0) {
    s->m_uuByteCntr13 = value;
  } else if (SB_STRCMP(name, "m_upktcntr13") == 0) {
    s->m_uPktCntr13 = value;
  } else if (SB_STRCMP(name, "m_uubytecntr12") == 0) {
    s->m_uuByteCntr12 = value;
  } else if (SB_STRCMP(name, "m_upktcntr12") == 0) {
    s->m_uPktCntr12 = value;
  } else if (SB_STRCMP(name, "m_uubytecntr11") == 0) {
    s->m_uuByteCntr11 = value;
  } else if (SB_STRCMP(name, "m_upktcntr11") == 0) {
    s->m_uPktCntr11 = value;
  } else if (SB_STRCMP(name, "m_uubytecntr10") == 0) {
    s->m_uuByteCntr10 = value;
  } else if (SB_STRCMP(name, "m_upktcntr10") == 0) {
    s->m_uPktCntr10 = value;
  } else if (SB_STRCMP(name, "m_uubytecntr9") == 0) {
    s->m_uuByteCntr9 = value;
  } else if (SB_STRCMP(name, "m_upktcntr9") == 0) {
    s->m_uPktCntr9 = value;
  } else if (SB_STRCMP(name, "m_uubytecntr8") == 0) {
    s->m_uuByteCntr8 = value;
  } else if (SB_STRCMP(name, "m_upktcntr8") == 0) {
    s->m_uPktCntr8 = value;
  } else if (SB_STRCMP(name, "m_uubytecntr7") == 0) {
    s->m_uuByteCntr7 = value;
  } else if (SB_STRCMP(name, "m_upktcntr7") == 0) {
    s->m_uPktCntr7 = value;
  } else if (SB_STRCMP(name, "m_uubytecntr6") == 0) {
    s->m_uuByteCntr6 = value;
  } else if (SB_STRCMP(name, "m_upktcntr6") == 0) {
    s->m_uPktCntr6 = value;
  } else if (SB_STRCMP(name, "m_uubytecntr5") == 0) {
    s->m_uuByteCntr5 = value;
  } else if (SB_STRCMP(name, "m_upktcntr5") == 0) {
    s->m_uPktCntr5 = value;
  } else if (SB_STRCMP(name, "m_uubytecntr4") == 0) {
    s->m_uuByteCntr4 = value;
  } else if (SB_STRCMP(name, "m_upktcntr4") == 0) {
    s->m_uPktCntr4 = value;
  } else if (SB_STRCMP(name, "m_uubytecntr3") == 0) {
    s->m_uuByteCntr3 = value;
  } else if (SB_STRCMP(name, "m_upktcntr3") == 0) {
    s->m_uPktCntr3 = value;
  } else if (SB_STRCMP(name, "m_uubytecntr2") == 0) {
    s->m_uuByteCntr2 = value;
  } else if (SB_STRCMP(name, "m_upktcntr2") == 0) {
    s->m_uPktCntr2 = value;
  } else if (SB_STRCMP(name, "m_uubytecntr1") == 0) {
    s->m_uuByteCntr1 = value;
  } else if (SB_STRCMP(name, "m_upktcntr1") == 0) {
    s->m_uPktCntr1 = value;
  } else if (SB_STRCMP(name, "m_uubytecntr0") == 0) {
    s->m_uuByteCntr0 = value;
  } else if (SB_STRCMP(name, "m_upktcntr0") == 0) {
    s->m_uPktCntr0 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
