/*
 * $Id: sbZfC2RcDataEntry2Console.c 1.1.34.3 Broadcom SDK $
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
#include "sbZfC2RcDataEntry2Console.hx"



/* Print members in struct */
void
sbZfC2RcDataEntry2_Print(sbZfC2RcDataEntry2_t *pFromStruct) {
  SB_LOG("C2RcDataEntry2:: data39=0x%08x", (unsigned int)  pFromStruct->m_uData39);
  SB_LOG(" data38=0x%08x", (unsigned int)  pFromStruct->m_uData38);
  SB_LOG(" data37=0x%08x", (unsigned int)  pFromStruct->m_uData37);
  SB_LOG("\n");

  SB_LOG("C2RcDataEntry2:: data36=0x%08x", (unsigned int)  pFromStruct->m_uData36);
  SB_LOG(" data35=0x%08x", (unsigned int)  pFromStruct->m_uData35);
  SB_LOG(" data34=0x%08x", (unsigned int)  pFromStruct->m_uData34);
  SB_LOG("\n");

  SB_LOG("C2RcDataEntry2:: data33=0x%08x", (unsigned int)  pFromStruct->m_uData33);
  SB_LOG(" data32=0x%08x", (unsigned int)  pFromStruct->m_uData32);
  SB_LOG(" data31=0x%08x", (unsigned int)  pFromStruct->m_uData31);
  SB_LOG("\n");

  SB_LOG("C2RcDataEntry2:: data30=0x%08x", (unsigned int)  pFromStruct->m_uData30);
  SB_LOG(" data29=0x%08x", (unsigned int)  pFromStruct->m_uData29);
  SB_LOG(" data28=0x%08x", (unsigned int)  pFromStruct->m_uData28);
  SB_LOG("\n");

  SB_LOG("C2RcDataEntry2:: data27=0x%08x", (unsigned int)  pFromStruct->m_uData27);
  SB_LOG(" data26=0x%08x", (unsigned int)  pFromStruct->m_uData26);
  SB_LOG(" data25=0x%08x", (unsigned int)  pFromStruct->m_uData25);
  SB_LOG("\n");

  SB_LOG("C2RcDataEntry2:: data24=0x%08x", (unsigned int)  pFromStruct->m_uData24);
  SB_LOG(" data23=0x%08x", (unsigned int)  pFromStruct->m_uData23);
  SB_LOG(" data22=0x%08x", (unsigned int)  pFromStruct->m_uData22);
  SB_LOG("\n");

  SB_LOG("C2RcDataEntry2:: data21=0x%08x", (unsigned int)  pFromStruct->m_uData21);
  SB_LOG(" data20=0x%08x", (unsigned int)  pFromStruct->m_uData20);
  SB_LOG(" data19=0x%08x", (unsigned int)  pFromStruct->m_uData19);
  SB_LOG("\n");

  SB_LOG("C2RcDataEntry2:: data18=0x%08x", (unsigned int)  pFromStruct->m_uData18);
  SB_LOG(" data17=0x%08x", (unsigned int)  pFromStruct->m_uData17);
  SB_LOG(" data16=0x%08x", (unsigned int)  pFromStruct->m_uData16);
  SB_LOG("\n");

  SB_LOG("C2RcDataEntry2:: data15=0x%08x", (unsigned int)  pFromStruct->m_uData15);
  SB_LOG(" data14=0x%08x", (unsigned int)  pFromStruct->m_uData14);
  SB_LOG(" data13=0x%08x", (unsigned int)  pFromStruct->m_uData13);
  SB_LOG("\n");

  SB_LOG("C2RcDataEntry2:: data12=0x%08x", (unsigned int)  pFromStruct->m_uData12);
  SB_LOG(" data11=0x%08x", (unsigned int)  pFromStruct->m_uData11);
  SB_LOG(" data10=0x%08x", (unsigned int)  pFromStruct->m_uData10);
  SB_LOG("\n");

  SB_LOG("C2RcDataEntry2:: data9=0x%08x", (unsigned int)  pFromStruct->m_uData9);
  SB_LOG(" data8=0x%08x", (unsigned int)  pFromStruct->m_uData8);
  SB_LOG(" spare7=0x%03x", (unsigned int)  pFromStruct->m_uSpare7);
  SB_LOG("\n");

  SB_LOG("C2RcDataEntry2:: data7=0x%06x", (unsigned int)  pFromStruct->m_uData7);
  SB_LOG(" spare6=0x%03x", (unsigned int)  pFromStruct->m_uSpare6);
  SB_LOG(" data6=0x%06x", (unsigned int)  pFromStruct->m_uData6);
  SB_LOG(" spare5=0x%03x", (unsigned int)  pFromStruct->m_uSpare5);
  SB_LOG("\n");

  SB_LOG("C2RcDataEntry2:: data5=0x%06x", (unsigned int)  pFromStruct->m_uData5);
  SB_LOG(" spare4=0x%03x", (unsigned int)  pFromStruct->m_uSpare4);
  SB_LOG(" data4=0x%06x", (unsigned int)  pFromStruct->m_uData4);
  SB_LOG(" spare3=0x%03x", (unsigned int)  pFromStruct->m_uSpare3);
  SB_LOG("\n");

  SB_LOG("C2RcDataEntry2:: data3=0x%06x", (unsigned int)  pFromStruct->m_uData3);
  SB_LOG(" spare2=0x%03x", (unsigned int)  pFromStruct->m_uSpare2);
  SB_LOG(" data2=0x%06x", (unsigned int)  pFromStruct->m_uData2);
  SB_LOG(" spare1=0x%03x", (unsigned int)  pFromStruct->m_uSpare1);
  SB_LOG("\n");

  SB_LOG("C2RcDataEntry2:: data1=0x%06x", (unsigned int)  pFromStruct->m_uData1);
  SB_LOG(" spare0=0x%03x", (unsigned int)  pFromStruct->m_uSpare0);
  SB_LOG(" data0=0x%06x", (unsigned int)  pFromStruct->m_uData0);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfC2RcDataEntry2_SPrint(sbZfC2RcDataEntry2_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDataEntry2:: data39=0x%08x", (unsigned int)  pFromStruct->m_uData39);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data38=0x%08x", (unsigned int)  pFromStruct->m_uData38);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data37=0x%08x", (unsigned int)  pFromStruct->m_uData37);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDataEntry2:: data36=0x%08x", (unsigned int)  pFromStruct->m_uData36);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data35=0x%08x", (unsigned int)  pFromStruct->m_uData35);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data34=0x%08x", (unsigned int)  pFromStruct->m_uData34);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDataEntry2:: data33=0x%08x", (unsigned int)  pFromStruct->m_uData33);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data32=0x%08x", (unsigned int)  pFromStruct->m_uData32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data31=0x%08x", (unsigned int)  pFromStruct->m_uData31);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDataEntry2:: data30=0x%08x", (unsigned int)  pFromStruct->m_uData30);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data29=0x%08x", (unsigned int)  pFromStruct->m_uData29);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data28=0x%08x", (unsigned int)  pFromStruct->m_uData28);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDataEntry2:: data27=0x%08x", (unsigned int)  pFromStruct->m_uData27);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data26=0x%08x", (unsigned int)  pFromStruct->m_uData26);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data25=0x%08x", (unsigned int)  pFromStruct->m_uData25);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDataEntry2:: data24=0x%08x", (unsigned int)  pFromStruct->m_uData24);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data23=0x%08x", (unsigned int)  pFromStruct->m_uData23);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data22=0x%08x", (unsigned int)  pFromStruct->m_uData22);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDataEntry2:: data21=0x%08x", (unsigned int)  pFromStruct->m_uData21);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data20=0x%08x", (unsigned int)  pFromStruct->m_uData20);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data19=0x%08x", (unsigned int)  pFromStruct->m_uData19);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDataEntry2:: data18=0x%08x", (unsigned int)  pFromStruct->m_uData18);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data17=0x%08x", (unsigned int)  pFromStruct->m_uData17);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data16=0x%08x", (unsigned int)  pFromStruct->m_uData16);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDataEntry2:: data15=0x%08x", (unsigned int)  pFromStruct->m_uData15);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data14=0x%08x", (unsigned int)  pFromStruct->m_uData14);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data13=0x%08x", (unsigned int)  pFromStruct->m_uData13);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDataEntry2:: data12=0x%08x", (unsigned int)  pFromStruct->m_uData12);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data11=0x%08x", (unsigned int)  pFromStruct->m_uData11);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data10=0x%08x", (unsigned int)  pFromStruct->m_uData10);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDataEntry2:: data9=0x%08x", (unsigned int)  pFromStruct->m_uData9);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data8=0x%08x", (unsigned int)  pFromStruct->m_uData8);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," spare7=0x%03x", (unsigned int)  pFromStruct->m_uSpare7);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDataEntry2:: data7=0x%06x", (unsigned int)  pFromStruct->m_uData7);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," spare6=0x%03x", (unsigned int)  pFromStruct->m_uSpare6);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data6=0x%06x", (unsigned int)  pFromStruct->m_uData6);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," spare5=0x%03x", (unsigned int)  pFromStruct->m_uSpare5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDataEntry2:: data5=0x%06x", (unsigned int)  pFromStruct->m_uData5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," spare4=0x%03x", (unsigned int)  pFromStruct->m_uSpare4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data4=0x%06x", (unsigned int)  pFromStruct->m_uData4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," spare3=0x%03x", (unsigned int)  pFromStruct->m_uSpare3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDataEntry2:: data3=0x%06x", (unsigned int)  pFromStruct->m_uData3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," spare2=0x%03x", (unsigned int)  pFromStruct->m_uSpare2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data2=0x%06x", (unsigned int)  pFromStruct->m_uData2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," spare1=0x%03x", (unsigned int)  pFromStruct->m_uSpare1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDataEntry2:: data1=0x%06x", (unsigned int)  pFromStruct->m_uData1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," spare0=0x%03x", (unsigned int)  pFromStruct->m_uSpare0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data0=0x%06x", (unsigned int)  pFromStruct->m_uData0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfC2RcDataEntry2_Validate(sbZfC2RcDataEntry2_t *pZf) {

  /* pZf->m_uData39 implicitly masked by data type */
  /* pZf->m_uData38 implicitly masked by data type */
  /* pZf->m_uData37 implicitly masked by data type */
  /* pZf->m_uData36 implicitly masked by data type */
  /* pZf->m_uData35 implicitly masked by data type */
  /* pZf->m_uData34 implicitly masked by data type */
  /* pZf->m_uData33 implicitly masked by data type */
  /* pZf->m_uData32 implicitly masked by data type */
  /* pZf->m_uData31 implicitly masked by data type */
  /* pZf->m_uData30 implicitly masked by data type */
  /* pZf->m_uData29 implicitly masked by data type */
  /* pZf->m_uData28 implicitly masked by data type */
  /* pZf->m_uData27 implicitly masked by data type */
  /* pZf->m_uData26 implicitly masked by data type */
  /* pZf->m_uData25 implicitly masked by data type */
  /* pZf->m_uData24 implicitly masked by data type */
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
  if (pZf->m_uSpare7 > 0x7ff) return 0;
  if (pZf->m_uData7 > 0x1fffff) return 0;
  if (pZf->m_uSpare6 > 0x7ff) return 0;
  if (pZf->m_uData6 > 0x1fffff) return 0;
  if (pZf->m_uSpare5 > 0x7ff) return 0;
  if (pZf->m_uData5 > 0x1fffff) return 0;
  if (pZf->m_uSpare4 > 0x7ff) return 0;
  if (pZf->m_uData4 > 0x1fffff) return 0;
  if (pZf->m_uSpare3 > 0x7ff) return 0;
  if (pZf->m_uData3 > 0x1fffff) return 0;
  if (pZf->m_uSpare2 > 0x7ff) return 0;
  if (pZf->m_uData2 > 0x1fffff) return 0;
  if (pZf->m_uSpare1 > 0x7ff) return 0;
  if (pZf->m_uData1 > 0x1fffff) return 0;
  if (pZf->m_uSpare0 > 0x7ff) return 0;
  if (pZf->m_uData0 > 0x1fffff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfC2RcDataEntry2_SetField(sbZfC2RcDataEntry2_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_udata39") == 0) {
    s->m_uData39 = value;
  } else if (SB_STRCMP(name, "m_udata38") == 0) {
    s->m_uData38 = value;
  } else if (SB_STRCMP(name, "m_udata37") == 0) {
    s->m_uData37 = value;
  } else if (SB_STRCMP(name, "m_udata36") == 0) {
    s->m_uData36 = value;
  } else if (SB_STRCMP(name, "m_udata35") == 0) {
    s->m_uData35 = value;
  } else if (SB_STRCMP(name, "m_udata34") == 0) {
    s->m_uData34 = value;
  } else if (SB_STRCMP(name, "m_udata33") == 0) {
    s->m_uData33 = value;
  } else if (SB_STRCMP(name, "m_udata32") == 0) {
    s->m_uData32 = value;
  } else if (SB_STRCMP(name, "m_udata31") == 0) {
    s->m_uData31 = value;
  } else if (SB_STRCMP(name, "m_udata30") == 0) {
    s->m_uData30 = value;
  } else if (SB_STRCMP(name, "m_udata29") == 0) {
    s->m_uData29 = value;
  } else if (SB_STRCMP(name, "m_udata28") == 0) {
    s->m_uData28 = value;
  } else if (SB_STRCMP(name, "m_udata27") == 0) {
    s->m_uData27 = value;
  } else if (SB_STRCMP(name, "m_udata26") == 0) {
    s->m_uData26 = value;
  } else if (SB_STRCMP(name, "m_udata25") == 0) {
    s->m_uData25 = value;
  } else if (SB_STRCMP(name, "m_udata24") == 0) {
    s->m_uData24 = value;
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
  } else if (SB_STRCMP(name, "m_uspare7") == 0) {
    s->m_uSpare7 = value;
  } else if (SB_STRCMP(name, "m_udata7") == 0) {
    s->m_uData7 = value;
  } else if (SB_STRCMP(name, "m_uspare6") == 0) {
    s->m_uSpare6 = value;
  } else if (SB_STRCMP(name, "m_udata6") == 0) {
    s->m_uData6 = value;
  } else if (SB_STRCMP(name, "m_uspare5") == 0) {
    s->m_uSpare5 = value;
  } else if (SB_STRCMP(name, "m_udata5") == 0) {
    s->m_uData5 = value;
  } else if (SB_STRCMP(name, "m_uspare4") == 0) {
    s->m_uSpare4 = value;
  } else if (SB_STRCMP(name, "m_udata4") == 0) {
    s->m_uData4 = value;
  } else if (SB_STRCMP(name, "m_uspare3") == 0) {
    s->m_uSpare3 = value;
  } else if (SB_STRCMP(name, "m_udata3") == 0) {
    s->m_uData3 = value;
  } else if (SB_STRCMP(name, "m_uspare2") == 0) {
    s->m_uSpare2 = value;
  } else if (SB_STRCMP(name, "m_udata2") == 0) {
    s->m_uData2 = value;
  } else if (SB_STRCMP(name, "m_uspare1") == 0) {
    s->m_uSpare1 = value;
  } else if (SB_STRCMP(name, "m_udata1") == 0) {
    s->m_uData1 = value;
  } else if (SB_STRCMP(name, "m_uspare0") == 0) {
    s->m_uSpare0 = value;
  } else if (SB_STRCMP(name, "m_udata0") == 0) {
    s->m_uData0 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
