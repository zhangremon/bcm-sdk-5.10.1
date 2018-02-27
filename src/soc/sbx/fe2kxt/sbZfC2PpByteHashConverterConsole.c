/*
 * $Id: sbZfC2PpByteHashConverterConsole.c 1.1.34.3 Broadcom SDK $
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
#include "sbZfC2PpByteHashConverterConsole.hx"



/* Print members in struct */
void
sbZfC2PpByteHashConverter_Print(sbZfC2PpByteHashConverter_t *pFromStruct) {
  SB_LOG("C2PpByteHashConverter:: hbit39=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit39);
  SB_LOG(" hbit38=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit38);
  SB_LOG(" hbit37=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit37);
  SB_LOG(" hbit36=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit36);
  SB_LOG("\n");

  SB_LOG("C2PpByteHashConverter:: hbit35=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit35);
  SB_LOG(" hbit34=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit34);
  SB_LOG(" hbit33=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit33);
  SB_LOG(" hbit32=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit32);
  SB_LOG("\n");

  SB_LOG("C2PpByteHashConverter:: hbit31=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit31);
  SB_LOG(" hbit30=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit30);
  SB_LOG(" hbit29=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit29);
  SB_LOG(" hbit28=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit28);
  SB_LOG("\n");

  SB_LOG("C2PpByteHashConverter:: hbit27=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit27);
  SB_LOG(" hbit26=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit26);
  SB_LOG(" hbit25=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit25);
  SB_LOG(" hbit24=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit24);
  SB_LOG("\n");

  SB_LOG("C2PpByteHashConverter:: hbit23=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit23);
  SB_LOG(" hbit22=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit22);
  SB_LOG(" hbit21=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit21);
  SB_LOG(" hbit20=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit20);
  SB_LOG("\n");

  SB_LOG("C2PpByteHashConverter:: hbit19=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit19);
  SB_LOG(" hbit18=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit18);
  SB_LOG(" hbit17=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit17);
  SB_LOG(" hbit16=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit16);
  SB_LOG("\n");

  SB_LOG("C2PpByteHashConverter:: hbit15=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit15);
  SB_LOG(" hbit14=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit14);
  SB_LOG(" hbit13=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit13);
  SB_LOG(" hbit12=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit12);
  SB_LOG("\n");

  SB_LOG("C2PpByteHashConverter:: hbit11=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit11);
  SB_LOG(" hbit10=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit10);
  SB_LOG(" hbit9=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit9);
  SB_LOG(" hbit8=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit8);
  SB_LOG(" hbit7=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit7);
  SB_LOG("\n");

  SB_LOG("C2PpByteHashConverter:: hbit6=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit6);
  SB_LOG(" hbit5=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit5);
  SB_LOG(" hbit4=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit4);
  SB_LOG(" hbit3=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit3);
  SB_LOG(" hbit2=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit2);
  SB_LOG("\n");

  SB_LOG("C2PpByteHashConverter:: hbit1=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit1);
  SB_LOG(" hbit0=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit0);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfC2PpByteHashConverter_SPrint(sbZfC2PpByteHashConverter_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PpByteHashConverter:: hbit39=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit39);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," hbit38=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit38);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," hbit37=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit37);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," hbit36=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit36);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PpByteHashConverter:: hbit35=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit35);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," hbit34=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit34);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," hbit33=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit33);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," hbit32=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PpByteHashConverter:: hbit31=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit31);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," hbit30=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit30);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," hbit29=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit29);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," hbit28=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit28);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PpByteHashConverter:: hbit27=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit27);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," hbit26=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit26);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," hbit25=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit25);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," hbit24=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit24);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PpByteHashConverter:: hbit23=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit23);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," hbit22=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit22);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," hbit21=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit21);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," hbit20=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit20);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PpByteHashConverter:: hbit19=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit19);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," hbit18=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit18);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," hbit17=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit17);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," hbit16=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit16);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PpByteHashConverter:: hbit15=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit15);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," hbit14=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit14);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," hbit13=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit13);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," hbit12=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit12);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PpByteHashConverter:: hbit11=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit11);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," hbit10=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit10);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," hbit9=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit9);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," hbit8=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit8);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," hbit7=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit7);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PpByteHashConverter:: hbit6=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit6);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," hbit5=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," hbit4=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," hbit3=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," hbit2=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PpByteHashConverter:: hbit1=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," hbit0=0x%01x", (unsigned int)  pFromStruct->m_uHashByteBit0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfC2PpByteHashConverter_Validate(sbZfC2PpByteHashConverter_t *pZf) {

  if (pZf->m_uHashByteBit39 > 0x1) return 0;
  if (pZf->m_uHashByteBit38 > 0x1) return 0;
  if (pZf->m_uHashByteBit37 > 0x1) return 0;
  if (pZf->m_uHashByteBit36 > 0x1) return 0;
  if (pZf->m_uHashByteBit35 > 0x1) return 0;
  if (pZf->m_uHashByteBit34 > 0x1) return 0;
  if (pZf->m_uHashByteBit33 > 0x1) return 0;
  if (pZf->m_uHashByteBit32 > 0x1) return 0;
  if (pZf->m_uHashByteBit31 > 0x1) return 0;
  if (pZf->m_uHashByteBit30 > 0x1) return 0;
  if (pZf->m_uHashByteBit29 > 0x1) return 0;
  if (pZf->m_uHashByteBit28 > 0x1) return 0;
  if (pZf->m_uHashByteBit27 > 0x1) return 0;
  if (pZf->m_uHashByteBit26 > 0x1) return 0;
  if (pZf->m_uHashByteBit25 > 0x1) return 0;
  if (pZf->m_uHashByteBit24 > 0x1) return 0;
  if (pZf->m_uHashByteBit23 > 0x1) return 0;
  if (pZf->m_uHashByteBit22 > 0x1) return 0;
  if (pZf->m_uHashByteBit21 > 0x1) return 0;
  if (pZf->m_uHashByteBit20 > 0x1) return 0;
  if (pZf->m_uHashByteBit19 > 0x1) return 0;
  if (pZf->m_uHashByteBit18 > 0x1) return 0;
  if (pZf->m_uHashByteBit17 > 0x1) return 0;
  if (pZf->m_uHashByteBit16 > 0x1) return 0;
  if (pZf->m_uHashByteBit15 > 0x1) return 0;
  if (pZf->m_uHashByteBit14 > 0x1) return 0;
  if (pZf->m_uHashByteBit13 > 0x1) return 0;
  if (pZf->m_uHashByteBit12 > 0x1) return 0;
  if (pZf->m_uHashByteBit11 > 0x1) return 0;
  if (pZf->m_uHashByteBit10 > 0x1) return 0;
  if (pZf->m_uHashByteBit9 > 0x1) return 0;
  if (pZf->m_uHashByteBit8 > 0x1) return 0;
  if (pZf->m_uHashByteBit7 > 0x1) return 0;
  if (pZf->m_uHashByteBit6 > 0x1) return 0;
  if (pZf->m_uHashByteBit5 > 0x1) return 0;
  if (pZf->m_uHashByteBit4 > 0x1) return 0;
  if (pZf->m_uHashByteBit3 > 0x1) return 0;
  if (pZf->m_uHashByteBit2 > 0x1) return 0;
  if (pZf->m_uHashByteBit1 > 0x1) return 0;
  if (pZf->m_uHashByteBit0 > 0x1) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfC2PpByteHashConverter_SetField(sbZfC2PpByteHashConverter_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_uhashbytebit39") == 0) {
    s->m_uHashByteBit39 = value;
  } else if (SB_STRCMP(name, "m_uhashbytebit38") == 0) {
    s->m_uHashByteBit38 = value;
  } else if (SB_STRCMP(name, "m_uhashbytebit37") == 0) {
    s->m_uHashByteBit37 = value;
  } else if (SB_STRCMP(name, "m_uhashbytebit36") == 0) {
    s->m_uHashByteBit36 = value;
  } else if (SB_STRCMP(name, "m_uhashbytebit35") == 0) {
    s->m_uHashByteBit35 = value;
  } else if (SB_STRCMP(name, "m_uhashbytebit34") == 0) {
    s->m_uHashByteBit34 = value;
  } else if (SB_STRCMP(name, "m_uhashbytebit33") == 0) {
    s->m_uHashByteBit33 = value;
  } else if (SB_STRCMP(name, "m_uhashbytebit32") == 0) {
    s->m_uHashByteBit32 = value;
  } else if (SB_STRCMP(name, "m_uhashbytebit31") == 0) {
    s->m_uHashByteBit31 = value;
  } else if (SB_STRCMP(name, "m_uhashbytebit30") == 0) {
    s->m_uHashByteBit30 = value;
  } else if (SB_STRCMP(name, "m_uhashbytebit29") == 0) {
    s->m_uHashByteBit29 = value;
  } else if (SB_STRCMP(name, "m_uhashbytebit28") == 0) {
    s->m_uHashByteBit28 = value;
  } else if (SB_STRCMP(name, "m_uhashbytebit27") == 0) {
    s->m_uHashByteBit27 = value;
  } else if (SB_STRCMP(name, "m_uhashbytebit26") == 0) {
    s->m_uHashByteBit26 = value;
  } else if (SB_STRCMP(name, "m_uhashbytebit25") == 0) {
    s->m_uHashByteBit25 = value;
  } else if (SB_STRCMP(name, "m_uhashbytebit24") == 0) {
    s->m_uHashByteBit24 = value;
  } else if (SB_STRCMP(name, "m_uhashbytebit23") == 0) {
    s->m_uHashByteBit23 = value;
  } else if (SB_STRCMP(name, "m_uhashbytebit22") == 0) {
    s->m_uHashByteBit22 = value;
  } else if (SB_STRCMP(name, "m_uhashbytebit21") == 0) {
    s->m_uHashByteBit21 = value;
  } else if (SB_STRCMP(name, "m_uhashbytebit20") == 0) {
    s->m_uHashByteBit20 = value;
  } else if (SB_STRCMP(name, "m_uhashbytebit19") == 0) {
    s->m_uHashByteBit19 = value;
  } else if (SB_STRCMP(name, "m_uhashbytebit18") == 0) {
    s->m_uHashByteBit18 = value;
  } else if (SB_STRCMP(name, "m_uhashbytebit17") == 0) {
    s->m_uHashByteBit17 = value;
  } else if (SB_STRCMP(name, "m_uhashbytebit16") == 0) {
    s->m_uHashByteBit16 = value;
  } else if (SB_STRCMP(name, "m_uhashbytebit15") == 0) {
    s->m_uHashByteBit15 = value;
  } else if (SB_STRCMP(name, "m_uhashbytebit14") == 0) {
    s->m_uHashByteBit14 = value;
  } else if (SB_STRCMP(name, "m_uhashbytebit13") == 0) {
    s->m_uHashByteBit13 = value;
  } else if (SB_STRCMP(name, "m_uhashbytebit12") == 0) {
    s->m_uHashByteBit12 = value;
  } else if (SB_STRCMP(name, "m_uhashbytebit11") == 0) {
    s->m_uHashByteBit11 = value;
  } else if (SB_STRCMP(name, "m_uhashbytebit10") == 0) {
    s->m_uHashByteBit10 = value;
  } else if (SB_STRCMP(name, "m_uhashbytebit9") == 0) {
    s->m_uHashByteBit9 = value;
  } else if (SB_STRCMP(name, "m_uhashbytebit8") == 0) {
    s->m_uHashByteBit8 = value;
  } else if (SB_STRCMP(name, "m_uhashbytebit7") == 0) {
    s->m_uHashByteBit7 = value;
  } else if (SB_STRCMP(name, "m_uhashbytebit6") == 0) {
    s->m_uHashByteBit6 = value;
  } else if (SB_STRCMP(name, "m_uhashbytebit5") == 0) {
    s->m_uHashByteBit5 = value;
  } else if (SB_STRCMP(name, "m_uhashbytebit4") == 0) {
    s->m_uHashByteBit4 = value;
  } else if (SB_STRCMP(name, "m_uhashbytebit3") == 0) {
    s->m_uHashByteBit3 = value;
  } else if (SB_STRCMP(name, "m_uhashbytebit2") == 0) {
    s->m_uHashByteBit2 = value;
  } else if (SB_STRCMP(name, "m_uhashbytebit1") == 0) {
    s->m_uHashByteBit1 = value;
  } else if (SB_STRCMP(name, "m_uhashbytebit0") == 0) {
    s->m_uHashByteBit0 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
