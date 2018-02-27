/*
 * $Id: sbZfSbQe2000ElibPCTConsole.c 1.3.36.3 Broadcom SDK $
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
#include "sbTypesGlue.h"
#include <soc/sbx/sbWrappers.h>
#include "sbZfSbQe2000ElibPCTConsole.hx"



/* Print members in struct */
void
sbZfSbQe2000ElibPCT_Print(sbZfSbQe2000ElibPCT_t *pFromStruct) {
  SB_LOG("SbQe2000ElibPCT:: pktcnt15=0x%08x", (unsigned int)  pFromStruct->m_PktClass15);
  SB_LOG(" bytecnt15=0x%09llx", (uint64_t)  pFromStruct->m_ByteClass15);
  SB_LOG("\n");

  SB_LOG("SbQe2000ElibPCT:: pktcnt14=0x%08x", (unsigned int)  pFromStruct->m_PktClass14);
  SB_LOG(" bytecnt14=0x%09llx", (uint64_t)  pFromStruct->m_ByteClass14);
  SB_LOG("\n");

  SB_LOG("SbQe2000ElibPCT:: pktcnt13=0x%08x", (unsigned int)  pFromStruct->m_PktClass13);
  SB_LOG(" bytecnt13=0x%09llx", (uint64_t)  pFromStruct->m_ByteClass13);
  SB_LOG("\n");

  SB_LOG("SbQe2000ElibPCT:: pktcnt12=0x%08x", (unsigned int)  pFromStruct->m_PktClass12);
  SB_LOG(" bytecnt12=0x%09llx", (uint64_t)  pFromStruct->m_ByteClass12);
  SB_LOG("\n");

  SB_LOG("SbQe2000ElibPCT:: pktcnt11=0x%08x", (unsigned int)  pFromStruct->m_PktClass11);
  SB_LOG(" bytecnt11=0x%09llx", (uint64_t)  pFromStruct->m_ByteClass11);
  SB_LOG("\n");

  SB_LOG("SbQe2000ElibPCT:: pktcnt10=0x%08x", (unsigned int)  pFromStruct->m_PktClass10);
  SB_LOG(" bytecnt10=0x%09llx", (uint64_t)  pFromStruct->m_ByteClass10);
  SB_LOG("\n");

  SB_LOG("SbQe2000ElibPCT:: pktcnt09=0x%08x", (unsigned int)  pFromStruct->m_PktClass9);
  SB_LOG(" bytecnt09=0x%09llx", (uint64_t)  pFromStruct->m_ByteClass9);
  SB_LOG("\n");

  SB_LOG("SbQe2000ElibPCT:: pktcnt08=0x%08x", (unsigned int)  pFromStruct->m_PktClass8);
  SB_LOG(" bytecnt08=0x%09llx", (uint64_t)  pFromStruct->m_ByteClass8);
  SB_LOG("\n");

  SB_LOG("SbQe2000ElibPCT:: pktcnt07=0x%08x", (unsigned int)  pFromStruct->m_PktClass7);
  SB_LOG(" bytecnt07=0x%09llx", (uint64_t)  pFromStruct->m_ByteClass7);
  SB_LOG("\n");

  SB_LOG("SbQe2000ElibPCT:: pktcnt06=0x%08x", (unsigned int)  pFromStruct->m_PktClass6);
  SB_LOG(" bytecnt06=0x%09llx", (uint64_t)  pFromStruct->m_ByteClass6);
  SB_LOG("\n");

  SB_LOG("SbQe2000ElibPCT:: pktcnt05=0x%08x", (unsigned int)  pFromStruct->m_PktClass5);
  SB_LOG(" bytecnt05=0x%09llx", (uint64_t)  pFromStruct->m_ByteClass5);
  SB_LOG("\n");

  SB_LOG("SbQe2000ElibPCT:: pktcnt04=0x%08x", (unsigned int)  pFromStruct->m_PktClass4);
  SB_LOG(" bytecnt04=0x%09llx", (uint64_t)  pFromStruct->m_ByteClass4);
  SB_LOG("\n");

  SB_LOG("SbQe2000ElibPCT:: pktcnt03=0x%08x", (unsigned int)  pFromStruct->m_PktClass3);
  SB_LOG(" bytecnt03=0x%09llx", (uint64_t)  pFromStruct->m_ByteClass3);
  SB_LOG("\n");

  SB_LOG("SbQe2000ElibPCT:: pktcnt02=0x%08x", (unsigned int)  pFromStruct->m_PktClass2);
  SB_LOG(" bytecnt02=0x%09llx", (uint64_t)  pFromStruct->m_ByteClass2);
  SB_LOG("\n");

  SB_LOG("SbQe2000ElibPCT:: pktcnt01=0x%08x", (unsigned int)  pFromStruct->m_PktClass1);
  SB_LOG(" bytecnt01=0x%09llx", (uint64_t)  pFromStruct->m_ByteClass1);
  SB_LOG("\n");

  SB_LOG("SbQe2000ElibPCT:: pktcnt00=0x%08x", (unsigned int)  pFromStruct->m_PktClass0);
  SB_LOG(" bytecnt00=0x%09llx", (uint64_t)  pFromStruct->m_ByteClass0);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfSbQe2000ElibPCT_SPrint(sbZfSbQe2000ElibPCT_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPCT:: pktcnt15=0x%08x", (unsigned int)  pFromStruct->m_PktClass15);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," bytecnt15=0x%09llx", (uint64_t)  pFromStruct->m_ByteClass15);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPCT:: pktcnt14=0x%08x", (unsigned int)  pFromStruct->m_PktClass14);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," bytecnt14=0x%09llx", (uint64_t)  pFromStruct->m_ByteClass14);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPCT:: pktcnt13=0x%08x", (unsigned int)  pFromStruct->m_PktClass13);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," bytecnt13=0x%09llx", (uint64_t)  pFromStruct->m_ByteClass13);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPCT:: pktcnt12=0x%08x", (unsigned int)  pFromStruct->m_PktClass12);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," bytecnt12=0x%09llx", (uint64_t)  pFromStruct->m_ByteClass12);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPCT:: pktcnt11=0x%08x", (unsigned int)  pFromStruct->m_PktClass11);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," bytecnt11=0x%09llx", (uint64_t)  pFromStruct->m_ByteClass11);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPCT:: pktcnt10=0x%08x", (unsigned int)  pFromStruct->m_PktClass10);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," bytecnt10=0x%09llx", (uint64_t)  pFromStruct->m_ByteClass10);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPCT:: pktcnt09=0x%08x", (unsigned int)  pFromStruct->m_PktClass9);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," bytecnt09=0x%09llx", (uint64_t)  pFromStruct->m_ByteClass9);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPCT:: pktcnt08=0x%08x", (unsigned int)  pFromStruct->m_PktClass8);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," bytecnt08=0x%09llx", (uint64_t)  pFromStruct->m_ByteClass8);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPCT:: pktcnt07=0x%08x", (unsigned int)  pFromStruct->m_PktClass7);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," bytecnt07=0x%09llx", (uint64_t)  pFromStruct->m_ByteClass7);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPCT:: pktcnt06=0x%08x", (unsigned int)  pFromStruct->m_PktClass6);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," bytecnt06=0x%09llx", (uint64_t)  pFromStruct->m_ByteClass6);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPCT:: pktcnt05=0x%08x", (unsigned int)  pFromStruct->m_PktClass5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," bytecnt05=0x%09llx", (uint64_t)  pFromStruct->m_ByteClass5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPCT:: pktcnt04=0x%08x", (unsigned int)  pFromStruct->m_PktClass4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," bytecnt04=0x%09llx", (uint64_t)  pFromStruct->m_ByteClass4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPCT:: pktcnt03=0x%08x", (unsigned int)  pFromStruct->m_PktClass3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," bytecnt03=0x%09llx", (uint64_t)  pFromStruct->m_ByteClass3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPCT:: pktcnt02=0x%08x", (unsigned int)  pFromStruct->m_PktClass2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," bytecnt02=0x%09llx", (uint64_t)  pFromStruct->m_ByteClass2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPCT:: pktcnt01=0x%08x", (unsigned int)  pFromStruct->m_PktClass1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," bytecnt01=0x%09llx", (uint64_t)  pFromStruct->m_ByteClass1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPCT:: pktcnt00=0x%08x", (unsigned int)  pFromStruct->m_PktClass0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," bytecnt00=0x%09llx", (uint64_t)  pFromStruct->m_ByteClass0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfSbQe2000ElibPCT_Validate(sbZfSbQe2000ElibPCT_t *pZf) {

  if (pZf->m_PktClass15 > 0x1fffffff) return 0;
  if (pZf->m_ByteClass15 > 0x7FFFFFFFFULL) return 0;
  if (pZf->m_PktClass14 > 0x1fffffff) return 0;
  if (pZf->m_ByteClass14 > 0x7FFFFFFFFULL) return 0;
  if (pZf->m_PktClass13 > 0x1fffffff) return 0;
  if (pZf->m_ByteClass13 > 0x7FFFFFFFFULL) return 0;
  if (pZf->m_PktClass12 > 0x1fffffff) return 0;
  if (pZf->m_ByteClass12 > 0x7FFFFFFFFULL) return 0;
  if (pZf->m_PktClass11 > 0x1fffffff) return 0;
  if (pZf->m_ByteClass11 > 0x7FFFFFFFFULL) return 0;
  if (pZf->m_PktClass10 > 0x1fffffff) return 0;
  if (pZf->m_ByteClass10 > 0x7FFFFFFFFULL) return 0;
  if (pZf->m_PktClass9 > 0x1fffffff) return 0;
  if (pZf->m_ByteClass9 > 0x7FFFFFFFFULL) return 0;
  if (pZf->m_PktClass8 > 0x1fffffff) return 0;
  if (pZf->m_ByteClass8 > 0x7FFFFFFFFULL) return 0;
  if (pZf->m_PktClass7 > 0x1fffffff) return 0;
  if (pZf->m_ByteClass7 > 0x7FFFFFFFFULL) return 0;
  if (pZf->m_PktClass6 > 0x1fffffff) return 0;
  if (pZf->m_ByteClass6 > 0x7FFFFFFFFULL) return 0;
  if (pZf->m_PktClass5 > 0x1fffffff) return 0;
  if (pZf->m_ByteClass5 > 0x7FFFFFFFFULL) return 0;
  if (pZf->m_PktClass4 > 0x1fffffff) return 0;
  if (pZf->m_ByteClass4 > 0x7FFFFFFFFULL) return 0;
  if (pZf->m_PktClass3 > 0x1fffffff) return 0;
  if (pZf->m_ByteClass3 > 0x7FFFFFFFFULL) return 0;
  if (pZf->m_PktClass2 > 0x1fffffff) return 0;
  if (pZf->m_ByteClass2 > 0x7FFFFFFFFULL) return 0;
  if (pZf->m_PktClass1 > 0x1fffffff) return 0;
  if (pZf->m_ByteClass1 > 0x7FFFFFFFFULL) return 0;
  if (pZf->m_PktClass0 > 0x1fffffff) return 0;
  if (pZf->m_ByteClass0 > 0x7FFFFFFFFULL) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfSbQe2000ElibPCT_SetField(sbZfSbQe2000ElibPCT_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_pktclass15") == 0) {
    s->m_PktClass15 = value;
  } else if (SB_STRCMP(name, "m_byteclass15") == 0) {
    s->m_ByteClass15 = value;
  } else if (SB_STRCMP(name, "m_pktclass14") == 0) {
    s->m_PktClass14 = value;
  } else if (SB_STRCMP(name, "m_byteclass14") == 0) {
    s->m_ByteClass14 = value;
  } else if (SB_STRCMP(name, "m_pktclass13") == 0) {
    s->m_PktClass13 = value;
  } else if (SB_STRCMP(name, "m_byteclass13") == 0) {
    s->m_ByteClass13 = value;
  } else if (SB_STRCMP(name, "m_pktclass12") == 0) {
    s->m_PktClass12 = value;
  } else if (SB_STRCMP(name, "m_byteclass12") == 0) {
    s->m_ByteClass12 = value;
  } else if (SB_STRCMP(name, "m_pktclass11") == 0) {
    s->m_PktClass11 = value;
  } else if (SB_STRCMP(name, "m_byteclass11") == 0) {
    s->m_ByteClass11 = value;
  } else if (SB_STRCMP(name, "m_pktclass10") == 0) {
    s->m_PktClass10 = value;
  } else if (SB_STRCMP(name, "m_byteclass10") == 0) {
    s->m_ByteClass10 = value;
  } else if (SB_STRCMP(name, "m_pktclass9") == 0) {
    s->m_PktClass9 = value;
  } else if (SB_STRCMP(name, "m_byteclass9") == 0) {
    s->m_ByteClass9 = value;
  } else if (SB_STRCMP(name, "m_pktclass8") == 0) {
    s->m_PktClass8 = value;
  } else if (SB_STRCMP(name, "m_byteclass8") == 0) {
    s->m_ByteClass8 = value;
  } else if (SB_STRCMP(name, "m_pktclass7") == 0) {
    s->m_PktClass7 = value;
  } else if (SB_STRCMP(name, "m_byteclass7") == 0) {
    s->m_ByteClass7 = value;
  } else if (SB_STRCMP(name, "m_pktclass6") == 0) {
    s->m_PktClass6 = value;
  } else if (SB_STRCMP(name, "m_byteclass6") == 0) {
    s->m_ByteClass6 = value;
  } else if (SB_STRCMP(name, "m_pktclass5") == 0) {
    s->m_PktClass5 = value;
  } else if (SB_STRCMP(name, "m_byteclass5") == 0) {
    s->m_ByteClass5 = value;
  } else if (SB_STRCMP(name, "m_pktclass4") == 0) {
    s->m_PktClass4 = value;
  } else if (SB_STRCMP(name, "m_byteclass4") == 0) {
    s->m_ByteClass4 = value;
  } else if (SB_STRCMP(name, "m_pktclass3") == 0) {
    s->m_PktClass3 = value;
  } else if (SB_STRCMP(name, "m_byteclass3") == 0) {
    s->m_ByteClass3 = value;
  } else if (SB_STRCMP(name, "m_pktclass2") == 0) {
    s->m_PktClass2 = value;
  } else if (SB_STRCMP(name, "m_byteclass2") == 0) {
    s->m_ByteClass2 = value;
  } else if (SB_STRCMP(name, "m_pktclass1") == 0) {
    s->m_PktClass1 = value;
  } else if (SB_STRCMP(name, "m_byteclass1") == 0) {
    s->m_ByteClass1 = value;
  } else if (SB_STRCMP(name, "m_pktclass0") == 0) {
    s->m_PktClass0 = value;
  } else if (SB_STRCMP(name, "m_byteclass0") == 0) {
    s->m_ByteClass0 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
