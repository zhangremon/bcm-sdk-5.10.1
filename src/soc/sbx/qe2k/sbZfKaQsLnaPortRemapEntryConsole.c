/*
 * $Id: sbZfKaQsLnaPortRemapEntryConsole.c 1.1.44.3 Broadcom SDK $
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
#include "sbZfKaQsLnaPortRemapEntryConsole.hx"



/* Print members in struct */
void
sbZfKaQsLnaPortRemapEntry_Print(sbZfKaQsLnaPortRemapEntry_t *pFromStruct) {
  SB_LOG("KaQsLnaPortRemapEntry:: res=0x%03x", (unsigned int)  pFromStruct->m_nRes);
  SB_LOG(" port24=0x%02x", (unsigned int)  pFromStruct->m_nPort24);
  SB_LOG(" port23=0x%02x", (unsigned int)  pFromStruct->m_nPort23);
  SB_LOG(" port22=0x%02x", (unsigned int)  pFromStruct->m_nPort22);
  SB_LOG("\n");

  SB_LOG("KaQsLnaPortRemapEntry:: port21=0x%02x", (unsigned int)  pFromStruct->m_nPort21);
  SB_LOG(" port20=0x%02x", (unsigned int)  pFromStruct->m_nPort20);
  SB_LOG(" port19=0x%02x", (unsigned int)  pFromStruct->m_nPort19);
  SB_LOG(" port18=0x%02x", (unsigned int)  pFromStruct->m_nPort18);
  SB_LOG("\n");

  SB_LOG("KaQsLnaPortRemapEntry:: port17=0x%02x", (unsigned int)  pFromStruct->m_nPort17);
  SB_LOG(" port16=0x%02x", (unsigned int)  pFromStruct->m_nPort16);
  SB_LOG(" port15=0x%02x", (unsigned int)  pFromStruct->m_nPort15);
  SB_LOG(" port14=0x%02x", (unsigned int)  pFromStruct->m_nPort14);
  SB_LOG("\n");

  SB_LOG("KaQsLnaPortRemapEntry:: port13=0x%02x", (unsigned int)  pFromStruct->m_nPort13);
  SB_LOG(" port12=0x%02x", (unsigned int)  pFromStruct->m_nPort12);
  SB_LOG(" port11=0x%02x", (unsigned int)  pFromStruct->m_nPort11);
  SB_LOG(" port10=0x%02x", (unsigned int)  pFromStruct->m_nPort10);
  SB_LOG("\n");

  SB_LOG("KaQsLnaPortRemapEntry:: port9=0x%02x", (unsigned int)  pFromStruct->m_nPort9);
  SB_LOG(" port8=0x%02x", (unsigned int)  pFromStruct->m_nPort8);
  SB_LOG(" port7=0x%02x", (unsigned int)  pFromStruct->m_nPort7);
  SB_LOG(" port6=0x%02x", (unsigned int)  pFromStruct->m_nPort6);
  SB_LOG("\n");

  SB_LOG("KaQsLnaPortRemapEntry:: port5=0x%02x", (unsigned int)  pFromStruct->m_nPort5);
  SB_LOG(" port4=0x%02x", (unsigned int)  pFromStruct->m_nPort4);
  SB_LOG(" port3=0x%02x", (unsigned int)  pFromStruct->m_nPort3);
  SB_LOG(" port2=0x%02x", (unsigned int)  pFromStruct->m_nPort2);
  SB_LOG("\n");

  SB_LOG("KaQsLnaPortRemapEntry:: port1=0x%02x", (unsigned int)  pFromStruct->m_nPort1);
  SB_LOG(" port0=0x%02x", (unsigned int)  pFromStruct->m_nPort0);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfKaQsLnaPortRemapEntry_SPrint(sbZfKaQsLnaPortRemapEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsLnaPortRemapEntry:: res=0x%03x", (unsigned int)  pFromStruct->m_nRes);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port24=0x%02x", (unsigned int)  pFromStruct->m_nPort24);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port23=0x%02x", (unsigned int)  pFromStruct->m_nPort23);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port22=0x%02x", (unsigned int)  pFromStruct->m_nPort22);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsLnaPortRemapEntry:: port21=0x%02x", (unsigned int)  pFromStruct->m_nPort21);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port20=0x%02x", (unsigned int)  pFromStruct->m_nPort20);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port19=0x%02x", (unsigned int)  pFromStruct->m_nPort19);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port18=0x%02x", (unsigned int)  pFromStruct->m_nPort18);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsLnaPortRemapEntry:: port17=0x%02x", (unsigned int)  pFromStruct->m_nPort17);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port16=0x%02x", (unsigned int)  pFromStruct->m_nPort16);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port15=0x%02x", (unsigned int)  pFromStruct->m_nPort15);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port14=0x%02x", (unsigned int)  pFromStruct->m_nPort14);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsLnaPortRemapEntry:: port13=0x%02x", (unsigned int)  pFromStruct->m_nPort13);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port12=0x%02x", (unsigned int)  pFromStruct->m_nPort12);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port11=0x%02x", (unsigned int)  pFromStruct->m_nPort11);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port10=0x%02x", (unsigned int)  pFromStruct->m_nPort10);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsLnaPortRemapEntry:: port9=0x%02x", (unsigned int)  pFromStruct->m_nPort9);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port8=0x%02x", (unsigned int)  pFromStruct->m_nPort8);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port7=0x%02x", (unsigned int)  pFromStruct->m_nPort7);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port6=0x%02x", (unsigned int)  pFromStruct->m_nPort6);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsLnaPortRemapEntry:: port5=0x%02x", (unsigned int)  pFromStruct->m_nPort5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port4=0x%02x", (unsigned int)  pFromStruct->m_nPort4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port3=0x%02x", (unsigned int)  pFromStruct->m_nPort3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port2=0x%02x", (unsigned int)  pFromStruct->m_nPort2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaQsLnaPortRemapEntry:: port1=0x%02x", (unsigned int)  pFromStruct->m_nPort1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," port0=0x%02x", (unsigned int)  pFromStruct->m_nPort0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaQsLnaPortRemapEntry_Validate(sbZfKaQsLnaPortRemapEntry_t *pZf) {

  if (pZf->m_nRes > 0x3ff) return 0;
  if (pZf->m_nPort24 > 0x3f) return 0;
  if (pZf->m_nPort23 > 0x3f) return 0;
  if (pZf->m_nPort22 > 0x3f) return 0;
  if (pZf->m_nPort21 > 0x3f) return 0;
  if (pZf->m_nPort20 > 0x3f) return 0;
  if (pZf->m_nPort19 > 0x3f) return 0;
  if (pZf->m_nPort18 > 0x3f) return 0;
  if (pZf->m_nPort17 > 0x3f) return 0;
  if (pZf->m_nPort16 > 0x3f) return 0;
  if (pZf->m_nPort15 > 0x3f) return 0;
  if (pZf->m_nPort14 > 0x3f) return 0;
  if (pZf->m_nPort13 > 0x3f) return 0;
  if (pZf->m_nPort12 > 0x3f) return 0;
  if (pZf->m_nPort11 > 0x3f) return 0;
  if (pZf->m_nPort10 > 0x3f) return 0;
  if (pZf->m_nPort9 > 0x3f) return 0;
  if (pZf->m_nPort8 > 0x3f) return 0;
  if (pZf->m_nPort7 > 0x3f) return 0;
  if (pZf->m_nPort6 > 0x3f) return 0;
  if (pZf->m_nPort5 > 0x3f) return 0;
  if (pZf->m_nPort4 > 0x3f) return 0;
  if (pZf->m_nPort3 > 0x3f) return 0;
  if (pZf->m_nPort2 > 0x3f) return 0;
  if (pZf->m_nPort1 > 0x3f) return 0;
  if (pZf->m_nPort0 > 0x3f) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaQsLnaPortRemapEntry_SetField(sbZfKaQsLnaPortRemapEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nres") == 0) {
    s->m_nRes = value;
  } else if (SB_STRCMP(name, "m_nport24") == 0) {
    s->m_nPort24 = value;
  } else if (SB_STRCMP(name, "m_nport23") == 0) {
    s->m_nPort23 = value;
  } else if (SB_STRCMP(name, "m_nport22") == 0) {
    s->m_nPort22 = value;
  } else if (SB_STRCMP(name, "m_nport21") == 0) {
    s->m_nPort21 = value;
  } else if (SB_STRCMP(name, "m_nport20") == 0) {
    s->m_nPort20 = value;
  } else if (SB_STRCMP(name, "m_nport19") == 0) {
    s->m_nPort19 = value;
  } else if (SB_STRCMP(name, "m_nport18") == 0) {
    s->m_nPort18 = value;
  } else if (SB_STRCMP(name, "m_nport17") == 0) {
    s->m_nPort17 = value;
  } else if (SB_STRCMP(name, "m_nport16") == 0) {
    s->m_nPort16 = value;
  } else if (SB_STRCMP(name, "m_nport15") == 0) {
    s->m_nPort15 = value;
  } else if (SB_STRCMP(name, "m_nport14") == 0) {
    s->m_nPort14 = value;
  } else if (SB_STRCMP(name, "m_nport13") == 0) {
    s->m_nPort13 = value;
  } else if (SB_STRCMP(name, "m_nport12") == 0) {
    s->m_nPort12 = value;
  } else if (SB_STRCMP(name, "m_nport11") == 0) {
    s->m_nPort11 = value;
  } else if (SB_STRCMP(name, "m_nport10") == 0) {
    s->m_nPort10 = value;
  } else if (SB_STRCMP(name, "m_nport9") == 0) {
    s->m_nPort9 = value;
  } else if (SB_STRCMP(name, "m_nport8") == 0) {
    s->m_nPort8 = value;
  } else if (SB_STRCMP(name, "m_nport7") == 0) {
    s->m_nPort7 = value;
  } else if (SB_STRCMP(name, "m_nport6") == 0) {
    s->m_nPort6 = value;
  } else if (SB_STRCMP(name, "m_nport5") == 0) {
    s->m_nPort5 = value;
  } else if (SB_STRCMP(name, "m_nport4") == 0) {
    s->m_nPort4 = value;
  } else if (SB_STRCMP(name, "m_nport3") == 0) {
    s->m_nPort3 = value;
  } else if (SB_STRCMP(name, "m_nport2") == 0) {
    s->m_nPort2 = value;
  } else if (SB_STRCMP(name, "m_nport1") == 0) {
    s->m_nPort1 = value;
  } else if (SB_STRCMP(name, "m_nport0") == 0) {
    s->m_nPort0 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
