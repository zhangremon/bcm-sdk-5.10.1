/*
 * $Id: sbZfCaRcDmaFormatConsole.c 1.1.48.3 Broadcom SDK $
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
#include "sbZfCaRcDmaFormatConsole.hx"



/* Print members in struct */
void
sbZfCaRcDmaFormat_Print(sbZfCaRcDmaFormat_t *pFromStruct) {
  SB_LOG("CaRcDmaFormat:: instr0=0x%08x", (unsigned int)  pFromStruct->m_uInstruction0);
  SB_LOG(" instr1=0x%08x", (unsigned int)  pFromStruct->m_uInstruction1);
  SB_LOG(" instr2=0x%08x", (unsigned int)  pFromStruct->m_uInstruction2);
  SB_LOG("\n");

  SB_LOG("CaRcDmaFormat:: instr3=0x%08x", (unsigned int)  pFromStruct->m_uInstruction3);
  SB_LOG(" resv0=0x%08x", (unsigned int)  pFromStruct->m_uResv0);
  SB_LOG(" resv1=0x%08x", (unsigned int)  pFromStruct->m_uResv1);
  SB_LOG("\n");

  SB_LOG("CaRcDmaFormat:: resv2=0x%08x", (unsigned int)  pFromStruct->m_uResv2);
  SB_LOG(" resv3=0x%08x", (unsigned int)  pFromStruct->m_uResv3);
  SB_LOG(" resv4=0x%08x", (unsigned int)  pFromStruct->m_uResv4);
  SB_LOG("\n");

  SB_LOG("CaRcDmaFormat:: resv5=0x%08x", (unsigned int)  pFromStruct->m_uResv5);
  SB_LOG(" resv6=0x%08x", (unsigned int)  pFromStruct->m_uResv6);
  SB_LOG(" resv7=0x%08x", (unsigned int)  pFromStruct->m_uResv7);
  SB_LOG("\n");

  SB_LOG("CaRcDmaFormat:: resv8=0x%08x", (unsigned int)  pFromStruct->m_uResv8);
  SB_LOG(" resv9=0x%08x", (unsigned int)  pFromStruct->m_uResv9);
  SB_LOG(" resv10=0x%08x", (unsigned int)  pFromStruct->m_uResv10);
  SB_LOG("\n");

  SB_LOG("CaRcDmaFormat:: resv11=0x%08x", (unsigned int)  pFromStruct->m_uResv11);
  SB_LOG(" resv12=0x%08x", (unsigned int)  pFromStruct->m_uResv12);
  SB_LOG(" resv13=0x%08x", (unsigned int)  pFromStruct->m_uResv13);
  SB_LOG("\n");

  SB_LOG("CaRcDmaFormat:: resv14=0x%08x", (unsigned int)  pFromStruct->m_uResv14);
  SB_LOG(" resv15=0x%08x", (unsigned int)  pFromStruct->m_uResv15);
  SB_LOG(" resv16=0x%08x", (unsigned int)  pFromStruct->m_uResv16);
  SB_LOG("\n");

  SB_LOG("CaRcDmaFormat:: resv17=0x%08x", (unsigned int)  pFromStruct->m_uResv17);
  SB_LOG(" resv18=0x%08x", (unsigned int)  pFromStruct->m_uResv18);
  SB_LOG(" resv19=0x%08x", (unsigned int)  pFromStruct->m_uResv19);
  SB_LOG("\n");

  SB_LOG("CaRcDmaFormat:: resv20=0x%08x", (unsigned int)  pFromStruct->m_uResv20);
  SB_LOG(" resv21=0x%08x", (unsigned int)  pFromStruct->m_uResv21);
  SB_LOG(" resv22=0x%08x", (unsigned int)  pFromStruct->m_uResv22);
  SB_LOG("\n");

  SB_LOG("CaRcDmaFormat:: resv23=0x%08x", (unsigned int)  pFromStruct->m_uResv23);
  SB_LOG(" resv24=0x%08x", (unsigned int)  pFromStruct->m_uResv24);
  SB_LOG(" resv25=0x%08x", (unsigned int)  pFromStruct->m_uResv25);
  SB_LOG("\n");

  SB_LOG("CaRcDmaFormat:: resv26=0x%08x", (unsigned int)  pFromStruct->m_uResv26);
  SB_LOG(" resv27=0x%08x", (unsigned int)  pFromStruct->m_uResv27);
  SB_LOG(" sb00patt310=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern0_31_0);
  SB_LOG("\n");

  SB_LOG("CaRcDmaFormat:: sb00patt6332=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern0_63_32);
  SB_LOG(" sb00patt6495=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern0_95_64);
  SB_LOG("\n");

  SB_LOG("CaRcDmaFormat:: sb00patt12796=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern0_127_96);
  SB_LOG(" sb01patt310=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern1_31_0);
  SB_LOG("\n");

  SB_LOG("CaRcDmaFormat:: sb01patt6332=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern1_63_32);
  SB_LOG(" sb01patt6495=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern1_95_64);
  SB_LOG("\n");

  SB_LOG("CaRcDmaFormat:: sb01patt12796=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern1_127_96);
  SB_LOG(" sb02patt310=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern2_31_0);
  SB_LOG("\n");

  SB_LOG("CaRcDmaFormat:: sb02patt6332=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern2_63_32);
  SB_LOG(" sb02patt6495=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern2_95_64);
  SB_LOG("\n");

  SB_LOG("CaRcDmaFormat:: sb02patt12796=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern2_127_96);
  SB_LOG(" sb03patt310=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern3_31_0);
  SB_LOG("\n");

  SB_LOG("CaRcDmaFormat:: sb03patt6332=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern3_63_32);
  SB_LOG(" sb03patt6495=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern3_95_64);
  SB_LOG("\n");

  SB_LOG("CaRcDmaFormat:: sb03patt12796=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern3_127_96);
  SB_LOG(" sb10patt310=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern0_31_0);
  SB_LOG("\n");

  SB_LOG("CaRcDmaFormat:: sb10patt6332=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern0_63_32);
  SB_LOG(" sb10patt6495=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern0_95_64);
  SB_LOG("\n");

  SB_LOG("CaRcDmaFormat:: sb10patt12796=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern0_127_96);
  SB_LOG(" sb11patt310=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern1_31_0);
  SB_LOG("\n");

  SB_LOG("CaRcDmaFormat:: sb11patt6332=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern1_63_32);
  SB_LOG(" sb11patt6495=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern1_95_64);
  SB_LOG("\n");

  SB_LOG("CaRcDmaFormat:: sb11patt12796=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern1_127_96);
  SB_LOG(" sb12patt310=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern2_31_0);
  SB_LOG("\n");

  SB_LOG("CaRcDmaFormat:: sb12patt6332=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern2_63_32);
  SB_LOG(" sb12patt6495=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern2_95_64);
  SB_LOG("\n");

  SB_LOG("CaRcDmaFormat:: sb12patt12796=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern2_127_96);
  SB_LOG(" sb13patt310=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern3_31_0);
  SB_LOG("\n");

  SB_LOG("CaRcDmaFormat:: sb13patt6332=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern3_63_32);
  SB_LOG(" sb13patt6495=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern3_95_64);
  SB_LOG("\n");

  SB_LOG("CaRcDmaFormat:: sb13patt12796=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern3_127_96);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfCaRcDmaFormat_SPrint(sbZfCaRcDmaFormat_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaRcDmaFormat:: instr0=0x%08x", (unsigned int)  pFromStruct->m_uInstruction0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," instr1=0x%08x", (unsigned int)  pFromStruct->m_uInstruction1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," instr2=0x%08x", (unsigned int)  pFromStruct->m_uInstruction2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaRcDmaFormat:: instr3=0x%08x", (unsigned int)  pFromStruct->m_uInstruction3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv0=0x%08x", (unsigned int)  pFromStruct->m_uResv0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv1=0x%08x", (unsigned int)  pFromStruct->m_uResv1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaRcDmaFormat:: resv2=0x%08x", (unsigned int)  pFromStruct->m_uResv2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv3=0x%08x", (unsigned int)  pFromStruct->m_uResv3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv4=0x%08x", (unsigned int)  pFromStruct->m_uResv4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaRcDmaFormat:: resv5=0x%08x", (unsigned int)  pFromStruct->m_uResv5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv6=0x%08x", (unsigned int)  pFromStruct->m_uResv6);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv7=0x%08x", (unsigned int)  pFromStruct->m_uResv7);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaRcDmaFormat:: resv8=0x%08x", (unsigned int)  pFromStruct->m_uResv8);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv9=0x%08x", (unsigned int)  pFromStruct->m_uResv9);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv10=0x%08x", (unsigned int)  pFromStruct->m_uResv10);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaRcDmaFormat:: resv11=0x%08x", (unsigned int)  pFromStruct->m_uResv11);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv12=0x%08x", (unsigned int)  pFromStruct->m_uResv12);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv13=0x%08x", (unsigned int)  pFromStruct->m_uResv13);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaRcDmaFormat:: resv14=0x%08x", (unsigned int)  pFromStruct->m_uResv14);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv15=0x%08x", (unsigned int)  pFromStruct->m_uResv15);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv16=0x%08x", (unsigned int)  pFromStruct->m_uResv16);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaRcDmaFormat:: resv17=0x%08x", (unsigned int)  pFromStruct->m_uResv17);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv18=0x%08x", (unsigned int)  pFromStruct->m_uResv18);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv19=0x%08x", (unsigned int)  pFromStruct->m_uResv19);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaRcDmaFormat:: resv20=0x%08x", (unsigned int)  pFromStruct->m_uResv20);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv21=0x%08x", (unsigned int)  pFromStruct->m_uResv21);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv22=0x%08x", (unsigned int)  pFromStruct->m_uResv22);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaRcDmaFormat:: resv23=0x%08x", (unsigned int)  pFromStruct->m_uResv23);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv24=0x%08x", (unsigned int)  pFromStruct->m_uResv24);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv25=0x%08x", (unsigned int)  pFromStruct->m_uResv25);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaRcDmaFormat:: resv26=0x%08x", (unsigned int)  pFromStruct->m_uResv26);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv27=0x%08x", (unsigned int)  pFromStruct->m_uResv27);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb00patt310=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern0_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaRcDmaFormat:: sb00patt6332=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern0_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb00patt6495=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern0_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaRcDmaFormat:: sb00patt12796=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern0_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb01patt310=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern1_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaRcDmaFormat:: sb01patt6332=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern1_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb01patt6495=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern1_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaRcDmaFormat:: sb01patt12796=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern1_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb02patt310=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern2_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaRcDmaFormat:: sb02patt6332=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern2_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb02patt6495=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern2_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaRcDmaFormat:: sb02patt12796=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern2_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb03patt310=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern3_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaRcDmaFormat:: sb03patt6332=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern3_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb03patt6495=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern3_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaRcDmaFormat:: sb03patt12796=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern3_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb10patt310=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern0_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaRcDmaFormat:: sb10patt6332=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern0_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb10patt6495=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern0_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaRcDmaFormat:: sb10patt12796=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern0_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb11patt310=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern1_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaRcDmaFormat:: sb11patt6332=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern1_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb11patt6495=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern1_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaRcDmaFormat:: sb11patt12796=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern1_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb12patt310=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern2_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaRcDmaFormat:: sb12patt6332=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern2_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb12patt6495=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern2_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaRcDmaFormat:: sb12patt12796=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern2_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb13patt310=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern3_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaRcDmaFormat:: sb13patt6332=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern3_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb13patt6495=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern3_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaRcDmaFormat:: sb13patt12796=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern3_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfCaRcDmaFormat_Validate(sbZfCaRcDmaFormat_t *pZf) {

  /* pZf->m_uInstruction0 implicitly masked by data type */
  /* pZf->m_uInstruction1 implicitly masked by data type */
  /* pZf->m_uInstruction2 implicitly masked by data type */
  /* pZf->m_uInstruction3 implicitly masked by data type */
  /* pZf->m_uResv0 implicitly masked by data type */
  /* pZf->m_uResv1 implicitly masked by data type */
  /* pZf->m_uResv2 implicitly masked by data type */
  /* pZf->m_uResv3 implicitly masked by data type */
  /* pZf->m_uResv4 implicitly masked by data type */
  /* pZf->m_uResv5 implicitly masked by data type */
  /* pZf->m_uResv6 implicitly masked by data type */
  /* pZf->m_uResv7 implicitly masked by data type */
  /* pZf->m_uResv8 implicitly masked by data type */
  /* pZf->m_uResv9 implicitly masked by data type */
  /* pZf->m_uResv10 implicitly masked by data type */
  /* pZf->m_uResv11 implicitly masked by data type */
  /* pZf->m_uResv12 implicitly masked by data type */
  /* pZf->m_uResv13 implicitly masked by data type */
  /* pZf->m_uResv14 implicitly masked by data type */
  /* pZf->m_uResv15 implicitly masked by data type */
  /* pZf->m_uResv16 implicitly masked by data type */
  /* pZf->m_uResv17 implicitly masked by data type */
  /* pZf->m_uResv18 implicitly masked by data type */
  /* pZf->m_uResv19 implicitly masked by data type */
  /* pZf->m_uResv20 implicitly masked by data type */
  /* pZf->m_uResv21 implicitly masked by data type */
  /* pZf->m_uResv22 implicitly masked by data type */
  /* pZf->m_uResv23 implicitly masked by data type */
  /* pZf->m_uResv24 implicitly masked by data type */
  /* pZf->m_uResv25 implicitly masked by data type */
  /* pZf->m_uResv26 implicitly masked by data type */
  /* pZf->m_uResv27 implicitly masked by data type */
  /* pZf->m_uSb0Pattern0_31_0 implicitly masked by data type */
  /* pZf->m_uSb0Pattern0_63_32 implicitly masked by data type */
  /* pZf->m_uSb0Pattern0_95_64 implicitly masked by data type */
  /* pZf->m_uSb0Pattern0_127_96 implicitly masked by data type */
  /* pZf->m_uSb0Pattern1_31_0 implicitly masked by data type */
  /* pZf->m_uSb0Pattern1_63_32 implicitly masked by data type */
  /* pZf->m_uSb0Pattern1_95_64 implicitly masked by data type */
  /* pZf->m_uSb0Pattern1_127_96 implicitly masked by data type */
  /* pZf->m_uSb0Pattern2_31_0 implicitly masked by data type */
  /* pZf->m_uSb0Pattern2_63_32 implicitly masked by data type */
  /* pZf->m_uSb0Pattern2_95_64 implicitly masked by data type */
  /* pZf->m_uSb0Pattern2_127_96 implicitly masked by data type */
  /* pZf->m_uSb0Pattern3_31_0 implicitly masked by data type */
  /* pZf->m_uSb0Pattern3_63_32 implicitly masked by data type */
  /* pZf->m_uSb0Pattern3_95_64 implicitly masked by data type */
  /* pZf->m_uSb0Pattern3_127_96 implicitly masked by data type */
  /* pZf->m_uSb1Pattern0_31_0 implicitly masked by data type */
  /* pZf->m_uSb1Pattern0_63_32 implicitly masked by data type */
  /* pZf->m_uSb1Pattern0_95_64 implicitly masked by data type */
  /* pZf->m_uSb1Pattern0_127_96 implicitly masked by data type */
  /* pZf->m_uSb1Pattern1_31_0 implicitly masked by data type */
  /* pZf->m_uSb1Pattern1_63_32 implicitly masked by data type */
  /* pZf->m_uSb1Pattern1_95_64 implicitly masked by data type */
  /* pZf->m_uSb1Pattern1_127_96 implicitly masked by data type */
  /* pZf->m_uSb1Pattern2_31_0 implicitly masked by data type */
  /* pZf->m_uSb1Pattern2_63_32 implicitly masked by data type */
  /* pZf->m_uSb1Pattern2_95_64 implicitly masked by data type */
  /* pZf->m_uSb1Pattern2_127_96 implicitly masked by data type */
  /* pZf->m_uSb1Pattern3_31_0 implicitly masked by data type */
  /* pZf->m_uSb1Pattern3_63_32 implicitly masked by data type */
  /* pZf->m_uSb1Pattern3_95_64 implicitly masked by data type */
  /* pZf->m_uSb1Pattern3_127_96 implicitly masked by data type */

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfCaRcDmaFormat_SetField(sbZfCaRcDmaFormat_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_uinstruction0") == 0) {
    s->m_uInstruction0 = value;
  } else if (SB_STRCMP(name, "m_uinstruction1") == 0) {
    s->m_uInstruction1 = value;
  } else if (SB_STRCMP(name, "m_uinstruction2") == 0) {
    s->m_uInstruction2 = value;
  } else if (SB_STRCMP(name, "m_uinstruction3") == 0) {
    s->m_uInstruction3 = value;
  } else if (SB_STRCMP(name, "m_uresv0") == 0) {
    s->m_uResv0 = value;
  } else if (SB_STRCMP(name, "m_uresv1") == 0) {
    s->m_uResv1 = value;
  } else if (SB_STRCMP(name, "m_uresv2") == 0) {
    s->m_uResv2 = value;
  } else if (SB_STRCMP(name, "m_uresv3") == 0) {
    s->m_uResv3 = value;
  } else if (SB_STRCMP(name, "m_uresv4") == 0) {
    s->m_uResv4 = value;
  } else if (SB_STRCMP(name, "m_uresv5") == 0) {
    s->m_uResv5 = value;
  } else if (SB_STRCMP(name, "m_uresv6") == 0) {
    s->m_uResv6 = value;
  } else if (SB_STRCMP(name, "m_uresv7") == 0) {
    s->m_uResv7 = value;
  } else if (SB_STRCMP(name, "m_uresv8") == 0) {
    s->m_uResv8 = value;
  } else if (SB_STRCMP(name, "m_uresv9") == 0) {
    s->m_uResv9 = value;
  } else if (SB_STRCMP(name, "m_uresv10") == 0) {
    s->m_uResv10 = value;
  } else if (SB_STRCMP(name, "m_uresv11") == 0) {
    s->m_uResv11 = value;
  } else if (SB_STRCMP(name, "m_uresv12") == 0) {
    s->m_uResv12 = value;
  } else if (SB_STRCMP(name, "m_uresv13") == 0) {
    s->m_uResv13 = value;
  } else if (SB_STRCMP(name, "m_uresv14") == 0) {
    s->m_uResv14 = value;
  } else if (SB_STRCMP(name, "m_uresv15") == 0) {
    s->m_uResv15 = value;
  } else if (SB_STRCMP(name, "m_uresv16") == 0) {
    s->m_uResv16 = value;
  } else if (SB_STRCMP(name, "m_uresv17") == 0) {
    s->m_uResv17 = value;
  } else if (SB_STRCMP(name, "m_uresv18") == 0) {
    s->m_uResv18 = value;
  } else if (SB_STRCMP(name, "m_uresv19") == 0) {
    s->m_uResv19 = value;
  } else if (SB_STRCMP(name, "m_uresv20") == 0) {
    s->m_uResv20 = value;
  } else if (SB_STRCMP(name, "m_uresv21") == 0) {
    s->m_uResv21 = value;
  } else if (SB_STRCMP(name, "m_uresv22") == 0) {
    s->m_uResv22 = value;
  } else if (SB_STRCMP(name, "m_uresv23") == 0) {
    s->m_uResv23 = value;
  } else if (SB_STRCMP(name, "m_uresv24") == 0) {
    s->m_uResv24 = value;
  } else if (SB_STRCMP(name, "m_uresv25") == 0) {
    s->m_uResv25 = value;
  } else if (SB_STRCMP(name, "m_uresv26") == 0) {
    s->m_uResv26 = value;
  } else if (SB_STRCMP(name, "m_uresv27") == 0) {
    s->m_uResv27 = value;
  } else if (SB_STRCMP(name, "m_usb0pattern0_31_0") == 0) {
    s->m_uSb0Pattern0_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb0pattern0_63_32") == 0) {
    s->m_uSb0Pattern0_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb0pattern0_95_64") == 0) {
    s->m_uSb0Pattern0_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb0pattern0_127_96") == 0) {
    s->m_uSb0Pattern0_127_96 = value;
  } else if (SB_STRCMP(name, "m_usb0pattern1_31_0") == 0) {
    s->m_uSb0Pattern1_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb0pattern1_63_32") == 0) {
    s->m_uSb0Pattern1_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb0pattern1_95_64") == 0) {
    s->m_uSb0Pattern1_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb0pattern1_127_96") == 0) {
    s->m_uSb0Pattern1_127_96 = value;
  } else if (SB_STRCMP(name, "m_usb0pattern2_31_0") == 0) {
    s->m_uSb0Pattern2_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb0pattern2_63_32") == 0) {
    s->m_uSb0Pattern2_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb0pattern2_95_64") == 0) {
    s->m_uSb0Pattern2_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb0pattern2_127_96") == 0) {
    s->m_uSb0Pattern2_127_96 = value;
  } else if (SB_STRCMP(name, "m_usb0pattern3_31_0") == 0) {
    s->m_uSb0Pattern3_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb0pattern3_63_32") == 0) {
    s->m_uSb0Pattern3_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb0pattern3_95_64") == 0) {
    s->m_uSb0Pattern3_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb0pattern3_127_96") == 0) {
    s->m_uSb0Pattern3_127_96 = value;
  } else if (SB_STRCMP(name, "m_usb1pattern0_31_0") == 0) {
    s->m_uSb1Pattern0_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb1pattern0_63_32") == 0) {
    s->m_uSb1Pattern0_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb1pattern0_95_64") == 0) {
    s->m_uSb1Pattern0_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb1pattern0_127_96") == 0) {
    s->m_uSb1Pattern0_127_96 = value;
  } else if (SB_STRCMP(name, "m_usb1pattern1_31_0") == 0) {
    s->m_uSb1Pattern1_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb1pattern1_63_32") == 0) {
    s->m_uSb1Pattern1_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb1pattern1_95_64") == 0) {
    s->m_uSb1Pattern1_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb1pattern1_127_96") == 0) {
    s->m_uSb1Pattern1_127_96 = value;
  } else if (SB_STRCMP(name, "m_usb1pattern2_31_0") == 0) {
    s->m_uSb1Pattern2_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb1pattern2_63_32") == 0) {
    s->m_uSb1Pattern2_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb1pattern2_95_64") == 0) {
    s->m_uSb1Pattern2_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb1pattern2_127_96") == 0) {
    s->m_uSb1Pattern2_127_96 = value;
  } else if (SB_STRCMP(name, "m_usb1pattern3_31_0") == 0) {
    s->m_uSb1Pattern3_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb1pattern3_63_32") == 0) {
    s->m_uSb1Pattern3_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb1pattern3_95_64") == 0) {
    s->m_uSb1Pattern3_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb1pattern3_127_96") == 0) {
    s->m_uSb1Pattern3_127_96 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
