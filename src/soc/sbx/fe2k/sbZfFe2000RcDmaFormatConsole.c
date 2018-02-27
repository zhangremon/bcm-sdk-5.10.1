/*
 * $Id: sbZfFe2000RcDmaFormatConsole.c 1.3.36.3 Broadcom SDK $
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
#include "sbZfFe2000RcDmaFormatConsole.hx"



/* Print members in struct */
void
sbZfFe2000RcDmaFormat_Print(sbZfFe2000RcDmaFormat_t *pFromStruct) {
  SB_LOG("Fe2000RcDmaFormat:: instr0=0x%08x", (unsigned int)  pFromStruct->uInstruction0);
  SB_LOG(" instr1=0x%08x", (unsigned int)  pFromStruct->uInstruction1);
  SB_LOG(" instr2=0x%08x", (unsigned int)  pFromStruct->uInstruction2);
  SB_LOG("\n");

  SB_LOG("Fe2000RcDmaFormat:: instr3=0x%08x", (unsigned int)  pFromStruct->uInstruction3);
  SB_LOG(" resv0=0x%08x", (unsigned int)  pFromStruct->uResv0);
  SB_LOG(" resv1=0x%08x", (unsigned int)  pFromStruct->uResv1);
  SB_LOG("\n");

  SB_LOG("Fe2000RcDmaFormat:: resv2=0x%08x", (unsigned int)  pFromStruct->uResv2);
  SB_LOG(" resv3=0x%08x", (unsigned int)  pFromStruct->uResv3);
  SB_LOG(" resv4=0x%08x", (unsigned int)  pFromStruct->uResv4);
  SB_LOG("\n");

  SB_LOG("Fe2000RcDmaFormat:: resv5=0x%08x", (unsigned int)  pFromStruct->uResv5);
  SB_LOG(" resv6=0x%08x", (unsigned int)  pFromStruct->uResv6);
  SB_LOG(" resv7=0x%08x", (unsigned int)  pFromStruct->uResv7);
  SB_LOG("\n");

  SB_LOG("Fe2000RcDmaFormat:: resv8=0x%08x", (unsigned int)  pFromStruct->uResv8);
  SB_LOG(" resv9=0x%08x", (unsigned int)  pFromStruct->uResv9);
  SB_LOG(" resv10=0x%08x", (unsigned int)  pFromStruct->uResv10);
  SB_LOG("\n");

  SB_LOG("Fe2000RcDmaFormat:: resv11=0x%08x", (unsigned int)  pFromStruct->uResv11);
  SB_LOG(" resv12=0x%08x", (unsigned int)  pFromStruct->uResv12);
  SB_LOG(" resv13=0x%08x", (unsigned int)  pFromStruct->uResv13);
  SB_LOG("\n");

  SB_LOG("Fe2000RcDmaFormat:: resv14=0x%08x", (unsigned int)  pFromStruct->uResv14);
  SB_LOG(" resv15=0x%08x", (unsigned int)  pFromStruct->uResv15);
  SB_LOG(" resv16=0x%08x", (unsigned int)  pFromStruct->uResv16);
  SB_LOG("\n");

  SB_LOG("Fe2000RcDmaFormat:: resv17=0x%08x", (unsigned int)  pFromStruct->uResv17);
  SB_LOG(" resv18=0x%08x", (unsigned int)  pFromStruct->uResv18);
  SB_LOG(" resv19=0x%08x", (unsigned int)  pFromStruct->uResv19);
  SB_LOG("\n");

  SB_LOG("Fe2000RcDmaFormat:: resv20=0x%08x", (unsigned int)  pFromStruct->uResv20);
  SB_LOG(" resv21=0x%08x", (unsigned int)  pFromStruct->uResv21);
  SB_LOG(" resv22=0x%08x", (unsigned int)  pFromStruct->uResv22);
  SB_LOG("\n");

  SB_LOG("Fe2000RcDmaFormat:: resv23=0x%08x", (unsigned int)  pFromStruct->uResv23);
  SB_LOG(" resv24=0x%08x", (unsigned int)  pFromStruct->uResv24);
  SB_LOG(" resv25=0x%08x", (unsigned int)  pFromStruct->uResv25);
  SB_LOG("\n");

  SB_LOG("Fe2000RcDmaFormat:: resv26=0x%08x", (unsigned int)  pFromStruct->uResv26);
  SB_LOG(" resv27=0x%08x", (unsigned int)  pFromStruct->uResv27);
  SB_LOG("\n");

  SB_LOG("Fe2000RcDmaFormat:: sb00patt310=0x%08x", (unsigned int)  pFromStruct->uSb0Pattern0_31_0);
  SB_LOG(" sb00patt6332=0x%08x", (unsigned int)  pFromStruct->uSb0Pattern0_63_32);
  SB_LOG("\n");

  SB_LOG("Fe2000RcDmaFormat:: sb00patt6495=0x%08x", (unsigned int)  pFromStruct->uSb0Pattern0_95_64);
  SB_LOG(" sb00patt12796=0x%08x", (unsigned int)  pFromStruct->uSb0Pattern0_127_96);
  SB_LOG("\n");

  SB_LOG("Fe2000RcDmaFormat:: sb01patt310=0x%08x", (unsigned int)  pFromStruct->uSb0Pattern1_31_0);
  SB_LOG(" sb01patt6332=0x%08x", (unsigned int)  pFromStruct->uSb0Pattern1_63_32);
  SB_LOG("\n");

  SB_LOG("Fe2000RcDmaFormat:: sb01patt6495=0x%08x", (unsigned int)  pFromStruct->uSb0Pattern1_95_64);
  SB_LOG(" sb01patt12796=0x%08x", (unsigned int)  pFromStruct->uSb0Pattern1_127_96);
  SB_LOG("\n");

  SB_LOG("Fe2000RcDmaFormat:: sb02patt310=0x%08x", (unsigned int)  pFromStruct->uSb0Pattern2_31_0);
  SB_LOG(" sb02patt6332=0x%08x", (unsigned int)  pFromStruct->uSb0Pattern2_63_32);
  SB_LOG("\n");

  SB_LOG("Fe2000RcDmaFormat:: sb02patt6495=0x%08x", (unsigned int)  pFromStruct->uSb0Pattern2_95_64);
  SB_LOG(" sb02patt12796=0x%08x", (unsigned int)  pFromStruct->uSb0Pattern2_127_96);
  SB_LOG("\n");

  SB_LOG("Fe2000RcDmaFormat:: sb03patt310=0x%08x", (unsigned int)  pFromStruct->uSb0Pattern3_31_0);
  SB_LOG(" sb03patt6332=0x%08x", (unsigned int)  pFromStruct->uSb0Pattern3_63_32);
  SB_LOG("\n");

  SB_LOG("Fe2000RcDmaFormat:: sb03patt6495=0x%08x", (unsigned int)  pFromStruct->uSb0Pattern3_95_64);
  SB_LOG(" sb03patt12796=0x%08x", (unsigned int)  pFromStruct->uSb0Pattern3_127_96);
  SB_LOG("\n");

  SB_LOG("Fe2000RcDmaFormat:: sb10patt310=0x%08x", (unsigned int)  pFromStruct->uSb1Pattern0_31_0);
  SB_LOG(" sb10patt6332=0x%08x", (unsigned int)  pFromStruct->uSb1Pattern0_63_32);
  SB_LOG("\n");

  SB_LOG("Fe2000RcDmaFormat:: sb10patt6495=0x%08x", (unsigned int)  pFromStruct->uSb1Pattern0_95_64);
  SB_LOG(" sb10patt12796=0x%08x", (unsigned int)  pFromStruct->uSb1Pattern0_127_96);
  SB_LOG("\n");

  SB_LOG("Fe2000RcDmaFormat:: sb11patt310=0x%08x", (unsigned int)  pFromStruct->uSb1Pattern1_31_0);
  SB_LOG(" sb11patt6332=0x%08x", (unsigned int)  pFromStruct->uSb1Pattern1_63_32);
  SB_LOG("\n");

  SB_LOG("Fe2000RcDmaFormat:: sb11patt6495=0x%08x", (unsigned int)  pFromStruct->uSb1Pattern1_95_64);
  SB_LOG(" sb11patt12796=0x%08x", (unsigned int)  pFromStruct->uSb1Pattern1_127_96);
  SB_LOG("\n");

  SB_LOG("Fe2000RcDmaFormat:: sb12patt310=0x%08x", (unsigned int)  pFromStruct->uSb1Pattern2_31_0);
  SB_LOG(" sb12patt6332=0x%08x", (unsigned int)  pFromStruct->uSb1Pattern2_63_32);
  SB_LOG("\n");

  SB_LOG("Fe2000RcDmaFormat:: sb12patt6495=0x%08x", (unsigned int)  pFromStruct->uSb1Pattern2_95_64);
  SB_LOG(" sb12patt12796=0x%08x", (unsigned int)  pFromStruct->uSb1Pattern2_127_96);
  SB_LOG("\n");

  SB_LOG("Fe2000RcDmaFormat:: sb13patt310=0x%08x", (unsigned int)  pFromStruct->uSb1Pattern3_31_0);
  SB_LOG(" sb13patt6332=0x%08x", (unsigned int)  pFromStruct->uSb1Pattern3_63_32);
  SB_LOG("\n");

  SB_LOG("Fe2000RcDmaFormat:: sb13patt6495=0x%08x", (unsigned int)  pFromStruct->uSb1Pattern3_95_64);
  SB_LOG(" sb13patt12796=0x%08x", (unsigned int)  pFromStruct->uSb1Pattern3_127_96);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfFe2000RcDmaFormat_SPrint(sbZfFe2000RcDmaFormat_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000RcDmaFormat:: instr0=0x%08x", (unsigned int)  pFromStruct->uInstruction0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," instr1=0x%08x", (unsigned int)  pFromStruct->uInstruction1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," instr2=0x%08x", (unsigned int)  pFromStruct->uInstruction2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000RcDmaFormat:: instr3=0x%08x", (unsigned int)  pFromStruct->uInstruction3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv0=0x%08x", (unsigned int)  pFromStruct->uResv0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv1=0x%08x", (unsigned int)  pFromStruct->uResv1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000RcDmaFormat:: resv2=0x%08x", (unsigned int)  pFromStruct->uResv2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv3=0x%08x", (unsigned int)  pFromStruct->uResv3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv4=0x%08x", (unsigned int)  pFromStruct->uResv4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000RcDmaFormat:: resv5=0x%08x", (unsigned int)  pFromStruct->uResv5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv6=0x%08x", (unsigned int)  pFromStruct->uResv6);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv7=0x%08x", (unsigned int)  pFromStruct->uResv7);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000RcDmaFormat:: resv8=0x%08x", (unsigned int)  pFromStruct->uResv8);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv9=0x%08x", (unsigned int)  pFromStruct->uResv9);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv10=0x%08x", (unsigned int)  pFromStruct->uResv10);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000RcDmaFormat:: resv11=0x%08x", (unsigned int)  pFromStruct->uResv11);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv12=0x%08x", (unsigned int)  pFromStruct->uResv12);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv13=0x%08x", (unsigned int)  pFromStruct->uResv13);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000RcDmaFormat:: resv14=0x%08x", (unsigned int)  pFromStruct->uResv14);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv15=0x%08x", (unsigned int)  pFromStruct->uResv15);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv16=0x%08x", (unsigned int)  pFromStruct->uResv16);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000RcDmaFormat:: resv17=0x%08x", (unsigned int)  pFromStruct->uResv17);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv18=0x%08x", (unsigned int)  pFromStruct->uResv18);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv19=0x%08x", (unsigned int)  pFromStruct->uResv19);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000RcDmaFormat:: resv20=0x%08x", (unsigned int)  pFromStruct->uResv20);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv21=0x%08x", (unsigned int)  pFromStruct->uResv21);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv22=0x%08x", (unsigned int)  pFromStruct->uResv22);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000RcDmaFormat:: resv23=0x%08x", (unsigned int)  pFromStruct->uResv23);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv24=0x%08x", (unsigned int)  pFromStruct->uResv24);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv25=0x%08x", (unsigned int)  pFromStruct->uResv25);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000RcDmaFormat:: resv26=0x%08x", (unsigned int)  pFromStruct->uResv26);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv27=0x%08x", (unsigned int)  pFromStruct->uResv27);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000RcDmaFormat:: sb00patt310=0x%08x", (unsigned int)  pFromStruct->uSb0Pattern0_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb00patt6332=0x%08x", (unsigned int)  pFromStruct->uSb0Pattern0_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000RcDmaFormat:: sb00patt6495=0x%08x", (unsigned int)  pFromStruct->uSb0Pattern0_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb00patt12796=0x%08x", (unsigned int)  pFromStruct->uSb0Pattern0_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000RcDmaFormat:: sb01patt310=0x%08x", (unsigned int)  pFromStruct->uSb0Pattern1_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb01patt6332=0x%08x", (unsigned int)  pFromStruct->uSb0Pattern1_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000RcDmaFormat:: sb01patt6495=0x%08x", (unsigned int)  pFromStruct->uSb0Pattern1_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb01patt12796=0x%08x", (unsigned int)  pFromStruct->uSb0Pattern1_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000RcDmaFormat:: sb02patt310=0x%08x", (unsigned int)  pFromStruct->uSb0Pattern2_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb02patt6332=0x%08x", (unsigned int)  pFromStruct->uSb0Pattern2_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000RcDmaFormat:: sb02patt6495=0x%08x", (unsigned int)  pFromStruct->uSb0Pattern2_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb02patt12796=0x%08x", (unsigned int)  pFromStruct->uSb0Pattern2_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000RcDmaFormat:: sb03patt310=0x%08x", (unsigned int)  pFromStruct->uSb0Pattern3_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb03patt6332=0x%08x", (unsigned int)  pFromStruct->uSb0Pattern3_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000RcDmaFormat:: sb03patt6495=0x%08x", (unsigned int)  pFromStruct->uSb0Pattern3_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb03patt12796=0x%08x", (unsigned int)  pFromStruct->uSb0Pattern3_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000RcDmaFormat:: sb10patt310=0x%08x", (unsigned int)  pFromStruct->uSb1Pattern0_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb10patt6332=0x%08x", (unsigned int)  pFromStruct->uSb1Pattern0_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000RcDmaFormat:: sb10patt6495=0x%08x", (unsigned int)  pFromStruct->uSb1Pattern0_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb10patt12796=0x%08x", (unsigned int)  pFromStruct->uSb1Pattern0_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000RcDmaFormat:: sb11patt310=0x%08x", (unsigned int)  pFromStruct->uSb1Pattern1_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb11patt6332=0x%08x", (unsigned int)  pFromStruct->uSb1Pattern1_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000RcDmaFormat:: sb11patt6495=0x%08x", (unsigned int)  pFromStruct->uSb1Pattern1_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb11patt12796=0x%08x", (unsigned int)  pFromStruct->uSb1Pattern1_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000RcDmaFormat:: sb12patt310=0x%08x", (unsigned int)  pFromStruct->uSb1Pattern2_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb12patt6332=0x%08x", (unsigned int)  pFromStruct->uSb1Pattern2_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000RcDmaFormat:: sb12patt6495=0x%08x", (unsigned int)  pFromStruct->uSb1Pattern2_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb12patt12796=0x%08x", (unsigned int)  pFromStruct->uSb1Pattern2_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000RcDmaFormat:: sb13patt310=0x%08x", (unsigned int)  pFromStruct->uSb1Pattern3_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb13patt6332=0x%08x", (unsigned int)  pFromStruct->uSb1Pattern3_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"Fe2000RcDmaFormat:: sb13patt6495=0x%08x", (unsigned int)  pFromStruct->uSb1Pattern3_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb13patt12796=0x%08x", (unsigned int)  pFromStruct->uSb1Pattern3_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFe2000RcDmaFormat_Validate(sbZfFe2000RcDmaFormat_t *pZf) {

  /* pZf->uInstruction0 implicitly masked by data type */
  /* pZf->uInstruction1 implicitly masked by data type */
  /* pZf->uInstruction2 implicitly masked by data type */
  /* pZf->uInstruction3 implicitly masked by data type */
  /* pZf->uResv0 implicitly masked by data type */
  /* pZf->uResv1 implicitly masked by data type */
  /* pZf->uResv2 implicitly masked by data type */
  /* pZf->uResv3 implicitly masked by data type */
  /* pZf->uResv4 implicitly masked by data type */
  /* pZf->uResv5 implicitly masked by data type */
  /* pZf->uResv6 implicitly masked by data type */
  /* pZf->uResv7 implicitly masked by data type */
  /* pZf->uResv8 implicitly masked by data type */
  /* pZf->uResv9 implicitly masked by data type */
  /* pZf->uResv10 implicitly masked by data type */
  /* pZf->uResv11 implicitly masked by data type */
  /* pZf->uResv12 implicitly masked by data type */
  /* pZf->uResv13 implicitly masked by data type */
  /* pZf->uResv14 implicitly masked by data type */
  /* pZf->uResv15 implicitly masked by data type */
  /* pZf->uResv16 implicitly masked by data type */
  /* pZf->uResv17 implicitly masked by data type */
  /* pZf->uResv18 implicitly masked by data type */
  /* pZf->uResv19 implicitly masked by data type */
  /* pZf->uResv20 implicitly masked by data type */
  /* pZf->uResv21 implicitly masked by data type */
  /* pZf->uResv22 implicitly masked by data type */
  /* pZf->uResv23 implicitly masked by data type */
  /* pZf->uResv24 implicitly masked by data type */
  /* pZf->uResv25 implicitly masked by data type */
  /* pZf->uResv26 implicitly masked by data type */
  /* pZf->uResv27 implicitly masked by data type */
  /* pZf->uSb0Pattern0_31_0 implicitly masked by data type */
  /* pZf->uSb0Pattern0_63_32 implicitly masked by data type */
  /* pZf->uSb0Pattern0_95_64 implicitly masked by data type */
  /* pZf->uSb0Pattern0_127_96 implicitly masked by data type */
  /* pZf->uSb0Pattern1_31_0 implicitly masked by data type */
  /* pZf->uSb0Pattern1_63_32 implicitly masked by data type */
  /* pZf->uSb0Pattern1_95_64 implicitly masked by data type */
  /* pZf->uSb0Pattern1_127_96 implicitly masked by data type */
  /* pZf->uSb0Pattern2_31_0 implicitly masked by data type */
  /* pZf->uSb0Pattern2_63_32 implicitly masked by data type */
  /* pZf->uSb0Pattern2_95_64 implicitly masked by data type */
  /* pZf->uSb0Pattern2_127_96 implicitly masked by data type */
  /* pZf->uSb0Pattern3_31_0 implicitly masked by data type */
  /* pZf->uSb0Pattern3_63_32 implicitly masked by data type */
  /* pZf->uSb0Pattern3_95_64 implicitly masked by data type */
  /* pZf->uSb0Pattern3_127_96 implicitly masked by data type */
  /* pZf->uSb1Pattern0_31_0 implicitly masked by data type */
  /* pZf->uSb1Pattern0_63_32 implicitly masked by data type */
  /* pZf->uSb1Pattern0_95_64 implicitly masked by data type */
  /* pZf->uSb1Pattern0_127_96 implicitly masked by data type */
  /* pZf->uSb1Pattern1_31_0 implicitly masked by data type */
  /* pZf->uSb1Pattern1_63_32 implicitly masked by data type */
  /* pZf->uSb1Pattern1_95_64 implicitly masked by data type */
  /* pZf->uSb1Pattern1_127_96 implicitly masked by data type */
  /* pZf->uSb1Pattern2_31_0 implicitly masked by data type */
  /* pZf->uSb1Pattern2_63_32 implicitly masked by data type */
  /* pZf->uSb1Pattern2_95_64 implicitly masked by data type */
  /* pZf->uSb1Pattern2_127_96 implicitly masked by data type */
  /* pZf->uSb1Pattern3_31_0 implicitly masked by data type */
  /* pZf->uSb1Pattern3_63_32 implicitly masked by data type */
  /* pZf->uSb1Pattern3_95_64 implicitly masked by data type */
  /* pZf->uSb1Pattern3_127_96 implicitly masked by data type */

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFe2000RcDmaFormat_SetField(sbZfFe2000RcDmaFormat_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "uinstruction0") == 0) {
    s->uInstruction0 = value;
  } else if (SB_STRCMP(name, "uinstruction1") == 0) {
    s->uInstruction1 = value;
  } else if (SB_STRCMP(name, "uinstruction2") == 0) {
    s->uInstruction2 = value;
  } else if (SB_STRCMP(name, "uinstruction3") == 0) {
    s->uInstruction3 = value;
  } else if (SB_STRCMP(name, "uresv0") == 0) {
    s->uResv0 = value;
  } else if (SB_STRCMP(name, "uresv1") == 0) {
    s->uResv1 = value;
  } else if (SB_STRCMP(name, "uresv2") == 0) {
    s->uResv2 = value;
  } else if (SB_STRCMP(name, "uresv3") == 0) {
    s->uResv3 = value;
  } else if (SB_STRCMP(name, "uresv4") == 0) {
    s->uResv4 = value;
  } else if (SB_STRCMP(name, "uresv5") == 0) {
    s->uResv5 = value;
  } else if (SB_STRCMP(name, "uresv6") == 0) {
    s->uResv6 = value;
  } else if (SB_STRCMP(name, "uresv7") == 0) {
    s->uResv7 = value;
  } else if (SB_STRCMP(name, "uresv8") == 0) {
    s->uResv8 = value;
  } else if (SB_STRCMP(name, "uresv9") == 0) {
    s->uResv9 = value;
  } else if (SB_STRCMP(name, "uresv10") == 0) {
    s->uResv10 = value;
  } else if (SB_STRCMP(name, "uresv11") == 0) {
    s->uResv11 = value;
  } else if (SB_STRCMP(name, "uresv12") == 0) {
    s->uResv12 = value;
  } else if (SB_STRCMP(name, "uresv13") == 0) {
    s->uResv13 = value;
  } else if (SB_STRCMP(name, "uresv14") == 0) {
    s->uResv14 = value;
  } else if (SB_STRCMP(name, "uresv15") == 0) {
    s->uResv15 = value;
  } else if (SB_STRCMP(name, "uresv16") == 0) {
    s->uResv16 = value;
  } else if (SB_STRCMP(name, "uresv17") == 0) {
    s->uResv17 = value;
  } else if (SB_STRCMP(name, "uresv18") == 0) {
    s->uResv18 = value;
  } else if (SB_STRCMP(name, "uresv19") == 0) {
    s->uResv19 = value;
  } else if (SB_STRCMP(name, "uresv20") == 0) {
    s->uResv20 = value;
  } else if (SB_STRCMP(name, "uresv21") == 0) {
    s->uResv21 = value;
  } else if (SB_STRCMP(name, "uresv22") == 0) {
    s->uResv22 = value;
  } else if (SB_STRCMP(name, "uresv23") == 0) {
    s->uResv23 = value;
  } else if (SB_STRCMP(name, "uresv24") == 0) {
    s->uResv24 = value;
  } else if (SB_STRCMP(name, "uresv25") == 0) {
    s->uResv25 = value;
  } else if (SB_STRCMP(name, "uresv26") == 0) {
    s->uResv26 = value;
  } else if (SB_STRCMP(name, "uresv27") == 0) {
    s->uResv27 = value;
  } else if (SB_STRCMP(name, "usb0pattern0_31_0") == 0) {
    s->uSb0Pattern0_31_0 = value;
  } else if (SB_STRCMP(name, "usb0pattern0_63_32") == 0) {
    s->uSb0Pattern0_63_32 = value;
  } else if (SB_STRCMP(name, "usb0pattern0_95_64") == 0) {
    s->uSb0Pattern0_95_64 = value;
  } else if (SB_STRCMP(name, "usb0pattern0_127_96") == 0) {
    s->uSb0Pattern0_127_96 = value;
  } else if (SB_STRCMP(name, "usb0pattern1_31_0") == 0) {
    s->uSb0Pattern1_31_0 = value;
  } else if (SB_STRCMP(name, "usb0pattern1_63_32") == 0) {
    s->uSb0Pattern1_63_32 = value;
  } else if (SB_STRCMP(name, "usb0pattern1_95_64") == 0) {
    s->uSb0Pattern1_95_64 = value;
  } else if (SB_STRCMP(name, "usb0pattern1_127_96") == 0) {
    s->uSb0Pattern1_127_96 = value;
  } else if (SB_STRCMP(name, "usb0pattern2_31_0") == 0) {
    s->uSb0Pattern2_31_0 = value;
  } else if (SB_STRCMP(name, "usb0pattern2_63_32") == 0) {
    s->uSb0Pattern2_63_32 = value;
  } else if (SB_STRCMP(name, "usb0pattern2_95_64") == 0) {
    s->uSb0Pattern2_95_64 = value;
  } else if (SB_STRCMP(name, "usb0pattern2_127_96") == 0) {
    s->uSb0Pattern2_127_96 = value;
  } else if (SB_STRCMP(name, "usb0pattern3_31_0") == 0) {
    s->uSb0Pattern3_31_0 = value;
  } else if (SB_STRCMP(name, "usb0pattern3_63_32") == 0) {
    s->uSb0Pattern3_63_32 = value;
  } else if (SB_STRCMP(name, "usb0pattern3_95_64") == 0) {
    s->uSb0Pattern3_95_64 = value;
  } else if (SB_STRCMP(name, "usb0pattern3_127_96") == 0) {
    s->uSb0Pattern3_127_96 = value;
  } else if (SB_STRCMP(name, "usb1pattern0_31_0") == 0) {
    s->uSb1Pattern0_31_0 = value;
  } else if (SB_STRCMP(name, "usb1pattern0_63_32") == 0) {
    s->uSb1Pattern0_63_32 = value;
  } else if (SB_STRCMP(name, "usb1pattern0_95_64") == 0) {
    s->uSb1Pattern0_95_64 = value;
  } else if (SB_STRCMP(name, "usb1pattern0_127_96") == 0) {
    s->uSb1Pattern0_127_96 = value;
  } else if (SB_STRCMP(name, "usb1pattern1_31_0") == 0) {
    s->uSb1Pattern1_31_0 = value;
  } else if (SB_STRCMP(name, "usb1pattern1_63_32") == 0) {
    s->uSb1Pattern1_63_32 = value;
  } else if (SB_STRCMP(name, "usb1pattern1_95_64") == 0) {
    s->uSb1Pattern1_95_64 = value;
  } else if (SB_STRCMP(name, "usb1pattern1_127_96") == 0) {
    s->uSb1Pattern1_127_96 = value;
  } else if (SB_STRCMP(name, "usb1pattern2_31_0") == 0) {
    s->uSb1Pattern2_31_0 = value;
  } else if (SB_STRCMP(name, "usb1pattern2_63_32") == 0) {
    s->uSb1Pattern2_63_32 = value;
  } else if (SB_STRCMP(name, "usb1pattern2_95_64") == 0) {
    s->uSb1Pattern2_95_64 = value;
  } else if (SB_STRCMP(name, "usb1pattern2_127_96") == 0) {
    s->uSb1Pattern2_127_96 = value;
  } else if (SB_STRCMP(name, "usb1pattern3_31_0") == 0) {
    s->uSb1Pattern3_31_0 = value;
  } else if (SB_STRCMP(name, "usb1pattern3_63_32") == 0) {
    s->uSb1Pattern3_63_32 = value;
  } else if (SB_STRCMP(name, "usb1pattern3_95_64") == 0) {
    s->uSb1Pattern3_95_64 = value;
  } else if (SB_STRCMP(name, "usb1pattern3_127_96") == 0) {
    s->uSb1Pattern3_127_96 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
