/*
 * $Id: sbZfC2PbCounterEntryConsole.c 1.1.34.3 Broadcom SDK $
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
#include "sbZfC2PbCounterEntryConsole.hx"



/* Print members in struct */
void
sbZfC2PbCounterEntry_Print(sbZfC2PbCounterEntry_t *pFromStruct) {
  SB_LOG("C2PbCounterEntry:: data23=0x%08x", (unsigned int)  pFromStruct->m_uData23);
  SB_LOG(" data22=0x%08x", (unsigned int)  pFromStruct->m_uData22);
  SB_LOG(" data21=0x%08x", (unsigned int)  pFromStruct->m_uData21);
  SB_LOG("\n");

  SB_LOG("C2PbCounterEntry:: data20=0x%08x", (unsigned int)  pFromStruct->m_uData20);
  SB_LOG(" data19=0x%08x", (unsigned int)  pFromStruct->m_uData19);
  SB_LOG(" data18=0x%08x", (unsigned int)  pFromStruct->m_uData18);
  SB_LOG("\n");

  SB_LOG("C2PbCounterEntry:: data17=0x%08x", (unsigned int)  pFromStruct->m_uData17);
  SB_LOG(" data16=0x%08x", (unsigned int)  pFromStruct->m_uData16);
  SB_LOG(" data15=0x%08x", (unsigned int)  pFromStruct->m_uData15);
  SB_LOG("\n");

  SB_LOG("C2PbCounterEntry:: data14=0x%08x", (unsigned int)  pFromStruct->m_uData14);
  SB_LOG(" data13=0x%08x", (unsigned int)  pFromStruct->m_uData13);
  SB_LOG(" data12=0x%08x", (unsigned int)  pFromStruct->m_uData12);
  SB_LOG("\n");

  SB_LOG("C2PbCounterEntry:: data11=0x%08x", (unsigned int)  pFromStruct->m_uData11);
  SB_LOG(" data10=0x%08x", (unsigned int)  pFromStruct->m_uData10);
  SB_LOG(" data9=0x%08x", (unsigned int)  pFromStruct->m_uData9);
  SB_LOG("\n");

  SB_LOG("C2PbCounterEntry:: data8=0x%08x", (unsigned int)  pFromStruct->m_uData8);
  SB_LOG(" data7=0x%08x", (unsigned int)  pFromStruct->m_uData7);
  SB_LOG(" data6=0x%08x", (unsigned int)  pFromStruct->m_uData6);
  SB_LOG("\n");

  SB_LOG("C2PbCounterEntry:: data5=0x%08x", (unsigned int)  pFromStruct->m_uData5);
  SB_LOG(" data4=0x%08x", (unsigned int)  pFromStruct->m_uData4);
  SB_LOG(" data3=0x%08x", (unsigned int)  pFromStruct->m_uData3);
  SB_LOG("\n");

  SB_LOG("C2PbCounterEntry:: data2=0x%08x", (unsigned int)  pFromStruct->m_uData2);
  SB_LOG(" data1=0x%08x", (unsigned int)  pFromStruct->m_uData1);
  SB_LOG(" data0=0x%08x", (unsigned int)  pFromStruct->m_uData0);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfC2PbCounterEntry_SPrint(sbZfC2PbCounterEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PbCounterEntry:: data23=0x%08x", (unsigned int)  pFromStruct->m_uData23);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data22=0x%08x", (unsigned int)  pFromStruct->m_uData22);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data21=0x%08x", (unsigned int)  pFromStruct->m_uData21);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PbCounterEntry:: data20=0x%08x", (unsigned int)  pFromStruct->m_uData20);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data19=0x%08x", (unsigned int)  pFromStruct->m_uData19);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data18=0x%08x", (unsigned int)  pFromStruct->m_uData18);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PbCounterEntry:: data17=0x%08x", (unsigned int)  pFromStruct->m_uData17);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data16=0x%08x", (unsigned int)  pFromStruct->m_uData16);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data15=0x%08x", (unsigned int)  pFromStruct->m_uData15);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PbCounterEntry:: data14=0x%08x", (unsigned int)  pFromStruct->m_uData14);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data13=0x%08x", (unsigned int)  pFromStruct->m_uData13);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data12=0x%08x", (unsigned int)  pFromStruct->m_uData12);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PbCounterEntry:: data11=0x%08x", (unsigned int)  pFromStruct->m_uData11);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data10=0x%08x", (unsigned int)  pFromStruct->m_uData10);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data9=0x%08x", (unsigned int)  pFromStruct->m_uData9);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PbCounterEntry:: data8=0x%08x", (unsigned int)  pFromStruct->m_uData8);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data7=0x%08x", (unsigned int)  pFromStruct->m_uData7);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data6=0x%08x", (unsigned int)  pFromStruct->m_uData6);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PbCounterEntry:: data5=0x%08x", (unsigned int)  pFromStruct->m_uData5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data4=0x%08x", (unsigned int)  pFromStruct->m_uData4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data3=0x%08x", (unsigned int)  pFromStruct->m_uData3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PbCounterEntry:: data2=0x%08x", (unsigned int)  pFromStruct->m_uData2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data1=0x%08x", (unsigned int)  pFromStruct->m_uData1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data0=0x%08x", (unsigned int)  pFromStruct->m_uData0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfC2PbCounterEntry_Validate(sbZfC2PbCounterEntry_t *pZf) {

  /* pZf->m_uData23 implicitly masked by data type */
  /* pZf->m_uData22 implicitly masked by data type */
  /* pZf->m_uData21 implicitly masked by data type */
  /* pZf->m_uData20 implicitly masked by data type */
  /* pZf->m_uData19 implicitly masked by data type */
  /* pZf->m_uData18 implicitly masked by data type */
  /* pZf->m_uData17 implicitly masked by data type */
  /* pZf->m_uData16 implicitly masked by data type */
  /* pZf->m_uData15 implicitly masked by data type */
  /* pZf->m_uData14 implicitly masked by data type */
  /* pZf->m_uData13 implicitly masked by data type */
  /* pZf->m_uData12 implicitly masked by data type */
  /* pZf->m_uData11 implicitly masked by data type */
  /* pZf->m_uData10 implicitly masked by data type */
  /* pZf->m_uData9 implicitly masked by data type */
  /* pZf->m_uData8 implicitly masked by data type */
  /* pZf->m_uData7 implicitly masked by data type */
  /* pZf->m_uData6 implicitly masked by data type */
  /* pZf->m_uData5 implicitly masked by data type */
  /* pZf->m_uData4 implicitly masked by data type */
  /* pZf->m_uData3 implicitly masked by data type */
  /* pZf->m_uData2 implicitly masked by data type */
  /* pZf->m_uData1 implicitly masked by data type */
  /* pZf->m_uData0 implicitly masked by data type */

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfC2PbCounterEntry_SetField(sbZfC2PbCounterEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_udata23") == 0) {
    s->m_uData23 = value;
  } else if (SB_STRCMP(name, "m_udata22") == 0) {
    s->m_uData22 = value;
  } else if (SB_STRCMP(name, "m_udata21") == 0) {
    s->m_uData21 = value;
  } else if (SB_STRCMP(name, "m_udata20") == 0) {
    s->m_uData20 = value;
  } else if (SB_STRCMP(name, "m_udata19") == 0) {
    s->m_uData19 = value;
  } else if (SB_STRCMP(name, "m_udata18") == 0) {
    s->m_uData18 = value;
  } else if (SB_STRCMP(name, "m_udata17") == 0) {
    s->m_uData17 = value;
  } else if (SB_STRCMP(name, "m_udata16") == 0) {
    s->m_uData16 = value;
  } else if (SB_STRCMP(name, "m_udata15") == 0) {
    s->m_uData15 = value;
  } else if (SB_STRCMP(name, "m_udata14") == 0) {
    s->m_uData14 = value;
  } else if (SB_STRCMP(name, "m_udata13") == 0) {
    s->m_uData13 = value;
  } else if (SB_STRCMP(name, "m_udata12") == 0) {
    s->m_uData12 = value;
  } else if (SB_STRCMP(name, "m_udata11") == 0) {
    s->m_uData11 = value;
  } else if (SB_STRCMP(name, "m_udata10") == 0) {
    s->m_uData10 = value;
  } else if (SB_STRCMP(name, "m_udata9") == 0) {
    s->m_uData9 = value;
  } else if (SB_STRCMP(name, "m_udata8") == 0) {
    s->m_uData8 = value;
  } else if (SB_STRCMP(name, "m_udata7") == 0) {
    s->m_uData7 = value;
  } else if (SB_STRCMP(name, "m_udata6") == 0) {
    s->m_uData6 = value;
  } else if (SB_STRCMP(name, "m_udata5") == 0) {
    s->m_uData5 = value;
  } else if (SB_STRCMP(name, "m_udata4") == 0) {
    s->m_uData4 = value;
  } else if (SB_STRCMP(name, "m_udata3") == 0) {
    s->m_uData3 = value;
  } else if (SB_STRCMP(name, "m_udata2") == 0) {
    s->m_uData2 = value;
  } else if (SB_STRCMP(name, "m_udata1") == 0) {
    s->m_uData1 = value;
  } else if (SB_STRCMP(name, "m_udata0") == 0) {
    s->m_uData0 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
