/*
 * $Id: sbZfC2RcDmaFormatConsole.c 1.1.34.3 Broadcom SDK $
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
#include "sbZfC2RcDmaFormatConsole.hx"



/* Print members in struct */
void
sbZfC2RcDmaFormat_Print(sbZfC2RcDmaFormat_t *pFromStruct) {
  SB_LOG("C2RcDmaFormat:: instr0=0x%08x", (unsigned int)  pFromStruct->m_uInstruction0);
  SB_LOG(" instr1=0x%08x", (unsigned int)  pFromStruct->m_uInstruction1);
  SB_LOG(" instr2=0x%08x", (unsigned int)  pFromStruct->m_uInstruction2);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: instr3=0x%08x", (unsigned int)  pFromStruct->m_uInstruction3);
  SB_LOG(" instr4=0x%08x", (unsigned int)  pFromStruct->m_uInstruction4);
  SB_LOG(" instr5=0x%08x", (unsigned int)  pFromStruct->m_uInstruction5);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: instr6=0x%08x", (unsigned int)  pFromStruct->m_uInstruction6);
  SB_LOG(" instr7=0x%08x", (unsigned int)  pFromStruct->m_uInstruction7);
  SB_LOG(" resv0=0x%08x", (unsigned int)  pFromStruct->m_uResv0);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: resv1=0x%08x", (unsigned int)  pFromStruct->m_uResv1);
  SB_LOG(" resv2=0x%08x", (unsigned int)  pFromStruct->m_uResv2);
  SB_LOG(" resv3=0x%08x", (unsigned int)  pFromStruct->m_uResv3);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: resv4=0x%08x", (unsigned int)  pFromStruct->m_uResv4);
  SB_LOG(" resv5=0x%08x", (unsigned int)  pFromStruct->m_uResv5);
  SB_LOG(" resv6=0x%08x", (unsigned int)  pFromStruct->m_uResv6);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: resv7=0x%08x", (unsigned int)  pFromStruct->m_uResv7);
  SB_LOG(" resv8=0x%08x", (unsigned int)  pFromStruct->m_uResv8);
  SB_LOG(" resv9=0x%08x", (unsigned int)  pFromStruct->m_uResv9);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: resv10=0x%08x", (unsigned int)  pFromStruct->m_uResv10);
  SB_LOG(" resv11=0x%08x", (unsigned int)  pFromStruct->m_uResv11);
  SB_LOG(" resv12=0x%08x", (unsigned int)  pFromStruct->m_uResv12);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: resv13=0x%08x", (unsigned int)  pFromStruct->m_uResv13);
  SB_LOG(" resv14=0x%08x", (unsigned int)  pFromStruct->m_uResv14);
  SB_LOG(" resv15=0x%08x", (unsigned int)  pFromStruct->m_uResv15);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: resv16=0x%08x", (unsigned int)  pFromStruct->m_uResv16);
  SB_LOG(" resv17=0x%08x", (unsigned int)  pFromStruct->m_uResv17);
  SB_LOG(" resv18=0x%08x", (unsigned int)  pFromStruct->m_uResv18);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: resv19=0x%08x", (unsigned int)  pFromStruct->m_uResv19);
  SB_LOG(" resv20=0x%08x", (unsigned int)  pFromStruct->m_uResv20);
  SB_LOG(" resv21=0x%08x", (unsigned int)  pFromStruct->m_uResv21);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: resv22=0x%08x", (unsigned int)  pFromStruct->m_uResv22);
  SB_LOG(" resv23=0x%08x", (unsigned int)  pFromStruct->m_uResv23);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb0i0patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern0_31_0);
  SB_LOG(" sb0i0patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern0_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb0i0patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern0_95_64);
  SB_LOG(" sb0i0patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern0_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb0i1patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern1_31_0);
  SB_LOG(" sb0i1patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern1_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb0i1patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern1_95_64);
  SB_LOG(" sb0i1patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern1_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb0i2patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern2_31_0);
  SB_LOG(" sb0i2patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern2_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb0i2patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern2_95_64);
  SB_LOG(" sb0i2patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern2_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb0i3patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern3_31_0);
  SB_LOG(" sb0i3patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern3_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb0i3patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern3_95_64);
  SB_LOG(" sb0i3patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern3_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb0i4patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern4_31_0);
  SB_LOG(" sb0i4patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern4_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb0i4patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern4_95_64);
  SB_LOG(" sb0i4patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern4_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb0i5patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern5_31_0);
  SB_LOG(" sb0i5patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern5_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb0i5patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern5_95_64);
  SB_LOG(" sb0i5patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern5_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb0i6patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern6_31_0);
  SB_LOG(" sb0i6patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern6_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb0i6patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern6_95_64);
  SB_LOG(" sb0i6patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern6_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb0i7patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern7_31_0);
  SB_LOG(" sb0i7patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern7_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb0i7patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern7_95_64);
  SB_LOG(" sb0i7patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern7_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb1i0patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern0_31_0);
  SB_LOG(" sb1i0patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern0_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb1i0patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern0_95_64);
  SB_LOG(" sb1i0patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern0_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb1i1patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern1_31_0);
  SB_LOG(" sb1i1patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern1_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb1i1patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern1_95_64);
  SB_LOG(" sb1i1patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern1_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb1i2patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern2_31_0);
  SB_LOG(" sb1i2patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern2_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb1i2patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern2_95_64);
  SB_LOG(" sb1i2patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern2_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb1i3patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern3_31_0);
  SB_LOG(" sb1i3patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern3_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb1i3patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern3_95_64);
  SB_LOG(" sb1i3patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern3_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb1i4patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern4_31_0);
  SB_LOG(" sb1i4patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern4_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb1i4patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern4_95_64);
  SB_LOG(" sb1i4patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern4_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb1i5patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern5_31_0);
  SB_LOG(" sb1i5patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern5_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb1i5patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern5_95_64);
  SB_LOG(" sb1i5patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern5_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb1i6patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern6_31_0);
  SB_LOG(" sb1i6patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern6_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb1i6patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern6_95_64);
  SB_LOG(" sb1i6patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern6_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb1i7patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern7_31_0);
  SB_LOG(" sb1i7patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern7_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb1i7patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern7_95_64);
  SB_LOG(" sb1i7patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern7_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb2i0patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern0_31_0);
  SB_LOG(" sb2i0patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern0_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb2i0patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern0_95_64);
  SB_LOG(" sb2i0patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern0_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb2i1patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern1_31_0);
  SB_LOG(" sb2i1patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern1_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb2i1patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern1_95_64);
  SB_LOG(" sb2i1patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern1_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb2i2patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern2_31_0);
  SB_LOG(" sb2i2patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern2_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb2i2patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern2_95_64);
  SB_LOG(" sb2i2patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern2_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb2i3patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern3_31_0);
  SB_LOG(" sb2i3patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern3_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb2i3patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern3_95_64);
  SB_LOG(" sb2i3patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern3_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb2i4patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern4_31_0);
  SB_LOG(" sb2i4patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern4_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb2i4patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern4_95_64);
  SB_LOG(" sb2i4patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern4_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb2i5patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern5_31_0);
  SB_LOG(" sb2i5patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern5_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb2i5patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern5_95_64);
  SB_LOG(" sb2i5patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern5_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb2i6patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern6_31_0);
  SB_LOG(" sb2i6patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern6_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb2i6patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern6_95_64);
  SB_LOG(" sb2i6patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern6_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb2i7patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern7_31_0);
  SB_LOG(" sb2i7patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern7_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb2i7patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern7_95_64);
  SB_LOG(" sb2i7patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern7_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb3i0patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern0_31_0);
  SB_LOG(" sb3i0patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern0_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb3i0patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern0_95_64);
  SB_LOG(" sb3i0patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern0_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb3i1patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern1_31_0);
  SB_LOG(" sb3i1patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern1_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb3i1patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern1_95_64);
  SB_LOG(" sb3i1patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern1_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb3i2patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern2_31_0);
  SB_LOG(" sb3i2patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern2_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb3i2patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern2_95_64);
  SB_LOG(" sb3i2patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern2_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb3i3patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern3_31_0);
  SB_LOG(" sb3i3patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern3_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb3i3patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern3_95_64);
  SB_LOG(" sb3i3patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern3_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb3i4patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern4_31_0);
  SB_LOG(" sb3i4patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern4_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb3i4patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern4_95_64);
  SB_LOG(" sb3i4patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern4_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb3i5patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern5_31_0);
  SB_LOG(" sb3i5patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern5_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb3i5patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern5_95_64);
  SB_LOG(" sb3i5patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern5_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb3i6patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern6_31_0);
  SB_LOG(" sb3i6patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern6_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb3i6patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern6_95_64);
  SB_LOG(" sb3i6patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern6_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb3i7patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern7_31_0);
  SB_LOG(" sb3i7patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern7_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb3i7patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern7_95_64);
  SB_LOG(" sb3i7patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern7_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb4i0patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern0_31_0);
  SB_LOG(" sb4i0patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern0_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb4i0patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern0_95_64);
  SB_LOG(" sb4i0patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern0_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb4i1patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern1_31_0);
  SB_LOG(" sb4i1patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern1_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb4i1patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern1_95_64);
  SB_LOG(" sb4i1patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern1_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb4i2patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern2_31_0);
  SB_LOG(" sb4i2patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern2_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb4i2patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern2_95_64);
  SB_LOG(" sb4i2patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern2_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb4i3patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern3_31_0);
  SB_LOG(" sb4i3patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern3_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb4i3patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern3_95_64);
  SB_LOG(" sb4i3patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern3_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb4i4patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern4_31_0);
  SB_LOG(" sb4i4patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern4_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb4i4patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern4_95_64);
  SB_LOG(" sb4i4patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern4_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb4i5patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern5_31_0);
  SB_LOG(" sb4i5patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern5_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb4i5patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern5_95_64);
  SB_LOG(" sb4i5patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern5_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb4i6patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern6_31_0);
  SB_LOG(" sb4i6patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern6_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb4i6patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern6_95_64);
  SB_LOG(" sb4i6patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern6_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb4i7patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern7_31_0);
  SB_LOG(" sb4i7patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern7_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb4i7patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern7_95_64);
  SB_LOG(" sb4i7patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern7_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb5i0patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern0_31_0);
  SB_LOG(" sb5i0patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern0_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb5i0patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern0_95_64);
  SB_LOG(" sb5i0patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern0_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb5i1patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern1_31_0);
  SB_LOG(" sb5i1patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern1_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb5i1patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern1_95_64);
  SB_LOG(" sb5i1patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern1_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb5i2patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern2_31_0);
  SB_LOG(" sb5i2patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern2_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb5i2patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern2_95_64);
  SB_LOG(" sb5i2patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern2_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb5i3patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern3_31_0);
  SB_LOG(" sb5i3patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern3_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb5i3patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern3_95_64);
  SB_LOG(" sb5i3patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern3_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb5i4patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern4_31_0);
  SB_LOG(" sb5i4patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern4_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb5i4patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern4_95_64);
  SB_LOG(" sb5i4patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern4_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb5i5patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern5_31_0);
  SB_LOG(" sb5i5patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern5_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb5i5patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern5_95_64);
  SB_LOG(" sb5i5patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern5_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb5i6patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern6_31_0);
  SB_LOG(" sb5i6patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern6_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb5i6patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern6_95_64);
  SB_LOG(" sb5i6patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern6_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb5i7patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern7_31_0);
  SB_LOG(" sb5i7patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern7_63_32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: sb5i7patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern7_95_64);
  SB_LOG(" sb5i7patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern7_127_96);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: resv24=0x%08x", (unsigned int)  pFromStruct->m_uResv24);
  SB_LOG(" resv25=0x%08x", (unsigned int)  pFromStruct->m_uResv25);
  SB_LOG(" resv26=0x%08x", (unsigned int)  pFromStruct->m_uResv26);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: resv27=0x%08x", (unsigned int)  pFromStruct->m_uResv27);
  SB_LOG(" resv28=0x%08x", (unsigned int)  pFromStruct->m_uResv28);
  SB_LOG(" resv29=0x%08x", (unsigned int)  pFromStruct->m_uResv29);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: resv30=0x%08x", (unsigned int)  pFromStruct->m_uResv30);
  SB_LOG(" resv31=0x%08x", (unsigned int)  pFromStruct->m_uResv31);
  SB_LOG(" resv32=0x%08x", (unsigned int)  pFromStruct->m_uResv32);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: resv33=0x%08x", (unsigned int)  pFromStruct->m_uResv33);
  SB_LOG(" resv34=0x%08x", (unsigned int)  pFromStruct->m_uResv34);
  SB_LOG(" resv35=0x%08x", (unsigned int)  pFromStruct->m_uResv35);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: resv36=0x%08x", (unsigned int)  pFromStruct->m_uResv36);
  SB_LOG(" resv37=0x%08x", (unsigned int)  pFromStruct->m_uResv37);
  SB_LOG(" resv38=0x%08x", (unsigned int)  pFromStruct->m_uResv38);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: resv39=0x%08x", (unsigned int)  pFromStruct->m_uResv39);
  SB_LOG(" resv40=0x%08x", (unsigned int)  pFromStruct->m_uResv40);
  SB_LOG(" resv41=0x%08x", (unsigned int)  pFromStruct->m_uResv41);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: resv42=0x%08x", (unsigned int)  pFromStruct->m_uResv42);
  SB_LOG(" resv43=0x%08x", (unsigned int)  pFromStruct->m_uResv43);
  SB_LOG(" resv44=0x%08x", (unsigned int)  pFromStruct->m_uResv44);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: resv45=0x%08x", (unsigned int)  pFromStruct->m_uResv45);
  SB_LOG(" resv46=0x%08x", (unsigned int)  pFromStruct->m_uResv46);
  SB_LOG(" resv47=0x%08x", (unsigned int)  pFromStruct->m_uResv47);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: resv48=0x%08x", (unsigned int)  pFromStruct->m_uResv48);
  SB_LOG(" resv49=0x%08x", (unsigned int)  pFromStruct->m_uResv49);
  SB_LOG(" resv50=0x%08x", (unsigned int)  pFromStruct->m_uResv50);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: resv51=0x%08x", (unsigned int)  pFromStruct->m_uResv51);
  SB_LOG(" resv52=0x%08x", (unsigned int)  pFromStruct->m_uResv52);
  SB_LOG(" resv53=0x%08x", (unsigned int)  pFromStruct->m_uResv53);
  SB_LOG("\n");

  SB_LOG("C2RcDmaFormat:: resv54=0x%08x", (unsigned int)  pFromStruct->m_uResv54);
  SB_LOG(" resv55=0x%08x", (unsigned int)  pFromStruct->m_uResv55);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfC2RcDmaFormat_SPrint(sbZfC2RcDmaFormat_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: instr0=0x%08x", (unsigned int)  pFromStruct->m_uInstruction0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," instr1=0x%08x", (unsigned int)  pFromStruct->m_uInstruction1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," instr2=0x%08x", (unsigned int)  pFromStruct->m_uInstruction2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: instr3=0x%08x", (unsigned int)  pFromStruct->m_uInstruction3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," instr4=0x%08x", (unsigned int)  pFromStruct->m_uInstruction4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," instr5=0x%08x", (unsigned int)  pFromStruct->m_uInstruction5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: instr6=0x%08x", (unsigned int)  pFromStruct->m_uInstruction6);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," instr7=0x%08x", (unsigned int)  pFromStruct->m_uInstruction7);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv0=0x%08x", (unsigned int)  pFromStruct->m_uResv0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: resv1=0x%08x", (unsigned int)  pFromStruct->m_uResv1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv2=0x%08x", (unsigned int)  pFromStruct->m_uResv2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv3=0x%08x", (unsigned int)  pFromStruct->m_uResv3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: resv4=0x%08x", (unsigned int)  pFromStruct->m_uResv4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv5=0x%08x", (unsigned int)  pFromStruct->m_uResv5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv6=0x%08x", (unsigned int)  pFromStruct->m_uResv6);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: resv7=0x%08x", (unsigned int)  pFromStruct->m_uResv7);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv8=0x%08x", (unsigned int)  pFromStruct->m_uResv8);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv9=0x%08x", (unsigned int)  pFromStruct->m_uResv9);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: resv10=0x%08x", (unsigned int)  pFromStruct->m_uResv10);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv11=0x%08x", (unsigned int)  pFromStruct->m_uResv11);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv12=0x%08x", (unsigned int)  pFromStruct->m_uResv12);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: resv13=0x%08x", (unsigned int)  pFromStruct->m_uResv13);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv14=0x%08x", (unsigned int)  pFromStruct->m_uResv14);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv15=0x%08x", (unsigned int)  pFromStruct->m_uResv15);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: resv16=0x%08x", (unsigned int)  pFromStruct->m_uResv16);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv17=0x%08x", (unsigned int)  pFromStruct->m_uResv17);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv18=0x%08x", (unsigned int)  pFromStruct->m_uResv18);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: resv19=0x%08x", (unsigned int)  pFromStruct->m_uResv19);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv20=0x%08x", (unsigned int)  pFromStruct->m_uResv20);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv21=0x%08x", (unsigned int)  pFromStruct->m_uResv21);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: resv22=0x%08x", (unsigned int)  pFromStruct->m_uResv22);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv23=0x%08x", (unsigned int)  pFromStruct->m_uResv23);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb0i0patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern0_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb0i0patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern0_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb0i0patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern0_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb0i0patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern0_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb0i1patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern1_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb0i1patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern1_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb0i1patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern1_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb0i1patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern1_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb0i2patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern2_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb0i2patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern2_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb0i2patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern2_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb0i2patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern2_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb0i3patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern3_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb0i3patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern3_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb0i3patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern3_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb0i3patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern3_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb0i4patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern4_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb0i4patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern4_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb0i4patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern4_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb0i4patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern4_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb0i5patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern5_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb0i5patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern5_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb0i5patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern5_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb0i5patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern5_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb0i6patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern6_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb0i6patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern6_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb0i6patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern6_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb0i6patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern6_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb0i7patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern7_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb0i7patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern7_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb0i7patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern7_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb0i7patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb0Pattern7_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb1i0patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern0_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb1i0patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern0_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb1i0patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern0_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb1i0patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern0_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb1i1patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern1_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb1i1patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern1_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb1i1patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern1_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb1i1patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern1_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb1i2patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern2_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb1i2patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern2_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb1i2patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern2_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb1i2patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern2_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb1i3patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern3_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb1i3patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern3_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb1i3patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern3_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb1i3patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern3_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb1i4patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern4_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb1i4patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern4_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb1i4patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern4_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb1i4patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern4_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb1i5patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern5_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb1i5patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern5_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb1i5patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern5_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb1i5patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern5_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb1i6patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern6_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb1i6patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern6_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb1i6patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern6_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb1i6patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern6_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb1i7patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern7_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb1i7patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern7_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb1i7patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern7_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb1i7patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb1Pattern7_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb2i0patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern0_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb2i0patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern0_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb2i0patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern0_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb2i0patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern0_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb2i1patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern1_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb2i1patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern1_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb2i1patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern1_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb2i1patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern1_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb2i2patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern2_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb2i2patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern2_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb2i2patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern2_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb2i2patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern2_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb2i3patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern3_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb2i3patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern3_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb2i3patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern3_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb2i3patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern3_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb2i4patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern4_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb2i4patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern4_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb2i4patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern4_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb2i4patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern4_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb2i5patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern5_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb2i5patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern5_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb2i5patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern5_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb2i5patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern5_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb2i6patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern6_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb2i6patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern6_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb2i6patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern6_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb2i6patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern6_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb2i7patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern7_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb2i7patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern7_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb2i7patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern7_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb2i7patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb2Pattern7_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb3i0patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern0_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb3i0patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern0_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb3i0patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern0_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb3i0patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern0_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb3i1patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern1_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb3i1patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern1_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb3i1patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern1_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb3i1patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern1_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb3i2patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern2_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb3i2patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern2_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb3i2patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern2_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb3i2patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern2_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb3i3patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern3_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb3i3patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern3_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb3i3patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern3_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb3i3patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern3_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb3i4patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern4_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb3i4patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern4_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb3i4patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern4_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb3i4patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern4_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb3i5patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern5_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb3i5patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern5_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb3i5patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern5_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb3i5patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern5_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb3i6patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern6_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb3i6patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern6_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb3i6patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern6_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb3i6patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern6_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb3i7patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern7_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb3i7patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern7_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb3i7patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern7_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb3i7patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb3Pattern7_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb4i0patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern0_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb4i0patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern0_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb4i0patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern0_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb4i0patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern0_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb4i1patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern1_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb4i1patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern1_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb4i1patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern1_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb4i1patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern1_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb4i2patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern2_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb4i2patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern2_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb4i2patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern2_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb4i2patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern2_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb4i3patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern3_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb4i3patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern3_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb4i3patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern3_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb4i3patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern3_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb4i4patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern4_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb4i4patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern4_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb4i4patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern4_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb4i4patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern4_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb4i5patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern5_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb4i5patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern5_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb4i5patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern5_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb4i5patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern5_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb4i6patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern6_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb4i6patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern6_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb4i6patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern6_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb4i6patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern6_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb4i7patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern7_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb4i7patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern7_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb4i7patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern7_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb4i7patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb4Pattern7_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb5i0patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern0_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb5i0patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern0_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb5i0patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern0_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb5i0patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern0_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb5i1patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern1_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb5i1patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern1_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb5i1patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern1_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb5i1patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern1_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb5i2patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern2_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb5i2patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern2_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb5i2patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern2_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb5i2patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern2_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb5i3patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern3_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb5i3patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern3_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb5i3patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern3_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb5i3patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern3_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb5i4patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern4_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb5i4patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern4_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb5i4patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern4_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb5i4patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern4_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb5i5patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern5_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb5i5patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern5_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb5i5patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern5_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb5i5patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern5_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb5i6patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern6_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb5i6patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern6_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb5i6patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern6_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb5i6patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern6_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb5i7patt31to0=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern7_31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb5i7patt63to32=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern7_63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: sb5i7patt95to64=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern7_95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sb5i7patt127to96=0x%08x", (unsigned int)  pFromStruct->m_uSb5Pattern7_127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: resv24=0x%08x", (unsigned int)  pFromStruct->m_uResv24);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv25=0x%08x", (unsigned int)  pFromStruct->m_uResv25);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv26=0x%08x", (unsigned int)  pFromStruct->m_uResv26);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: resv27=0x%08x", (unsigned int)  pFromStruct->m_uResv27);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv28=0x%08x", (unsigned int)  pFromStruct->m_uResv28);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv29=0x%08x", (unsigned int)  pFromStruct->m_uResv29);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: resv30=0x%08x", (unsigned int)  pFromStruct->m_uResv30);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv31=0x%08x", (unsigned int)  pFromStruct->m_uResv31);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv32=0x%08x", (unsigned int)  pFromStruct->m_uResv32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: resv33=0x%08x", (unsigned int)  pFromStruct->m_uResv33);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv34=0x%08x", (unsigned int)  pFromStruct->m_uResv34);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv35=0x%08x", (unsigned int)  pFromStruct->m_uResv35);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: resv36=0x%08x", (unsigned int)  pFromStruct->m_uResv36);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv37=0x%08x", (unsigned int)  pFromStruct->m_uResv37);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv38=0x%08x", (unsigned int)  pFromStruct->m_uResv38);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: resv39=0x%08x", (unsigned int)  pFromStruct->m_uResv39);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv40=0x%08x", (unsigned int)  pFromStruct->m_uResv40);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv41=0x%08x", (unsigned int)  pFromStruct->m_uResv41);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: resv42=0x%08x", (unsigned int)  pFromStruct->m_uResv42);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv43=0x%08x", (unsigned int)  pFromStruct->m_uResv43);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv44=0x%08x", (unsigned int)  pFromStruct->m_uResv44);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: resv45=0x%08x", (unsigned int)  pFromStruct->m_uResv45);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv46=0x%08x", (unsigned int)  pFromStruct->m_uResv46);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv47=0x%08x", (unsigned int)  pFromStruct->m_uResv47);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: resv48=0x%08x", (unsigned int)  pFromStruct->m_uResv48);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv49=0x%08x", (unsigned int)  pFromStruct->m_uResv49);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv50=0x%08x", (unsigned int)  pFromStruct->m_uResv50);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: resv51=0x%08x", (unsigned int)  pFromStruct->m_uResv51);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv52=0x%08x", (unsigned int)  pFromStruct->m_uResv52);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv53=0x%08x", (unsigned int)  pFromStruct->m_uResv53);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2RcDmaFormat:: resv54=0x%08x", (unsigned int)  pFromStruct->m_uResv54);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," resv55=0x%08x", (unsigned int)  pFromStruct->m_uResv55);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfC2RcDmaFormat_Validate(sbZfC2RcDmaFormat_t *pZf) {

  /* pZf->m_uInstruction0 implicitly masked by data type */
  /* pZf->m_uInstruction1 implicitly masked by data type */
  /* pZf->m_uInstruction2 implicitly masked by data type */
  /* pZf->m_uInstruction3 implicitly masked by data type */
  /* pZf->m_uInstruction4 implicitly masked by data type */
  /* pZf->m_uInstruction5 implicitly masked by data type */
  /* pZf->m_uInstruction6 implicitly masked by data type */
  /* pZf->m_uInstruction7 implicitly masked by data type */
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
  /* pZf->m_uSb0Pattern4_31_0 implicitly masked by data type */
  /* pZf->m_uSb0Pattern4_63_32 implicitly masked by data type */
  /* pZf->m_uSb0Pattern4_95_64 implicitly masked by data type */
  /* pZf->m_uSb0Pattern4_127_96 implicitly masked by data type */
  /* pZf->m_uSb0Pattern5_31_0 implicitly masked by data type */
  /* pZf->m_uSb0Pattern5_63_32 implicitly masked by data type */
  /* pZf->m_uSb0Pattern5_95_64 implicitly masked by data type */
  /* pZf->m_uSb0Pattern5_127_96 implicitly masked by data type */
  /* pZf->m_uSb0Pattern6_31_0 implicitly masked by data type */
  /* pZf->m_uSb0Pattern6_63_32 implicitly masked by data type */
  /* pZf->m_uSb0Pattern6_95_64 implicitly masked by data type */
  /* pZf->m_uSb0Pattern6_127_96 implicitly masked by data type */
  /* pZf->m_uSb0Pattern7_31_0 implicitly masked by data type */
  /* pZf->m_uSb0Pattern7_63_32 implicitly masked by data type */
  /* pZf->m_uSb0Pattern7_95_64 implicitly masked by data type */
  /* pZf->m_uSb0Pattern7_127_96 implicitly masked by data type */
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
  /* pZf->m_uSb1Pattern4_31_0 implicitly masked by data type */
  /* pZf->m_uSb1Pattern4_63_32 implicitly masked by data type */
  /* pZf->m_uSb1Pattern4_95_64 implicitly masked by data type */
  /* pZf->m_uSb1Pattern4_127_96 implicitly masked by data type */
  /* pZf->m_uSb1Pattern5_31_0 implicitly masked by data type */
  /* pZf->m_uSb1Pattern5_63_32 implicitly masked by data type */
  /* pZf->m_uSb1Pattern5_95_64 implicitly masked by data type */
  /* pZf->m_uSb1Pattern5_127_96 implicitly masked by data type */
  /* pZf->m_uSb1Pattern6_31_0 implicitly masked by data type */
  /* pZf->m_uSb1Pattern6_63_32 implicitly masked by data type */
  /* pZf->m_uSb1Pattern6_95_64 implicitly masked by data type */
  /* pZf->m_uSb1Pattern6_127_96 implicitly masked by data type */
  /* pZf->m_uSb1Pattern7_31_0 implicitly masked by data type */
  /* pZf->m_uSb1Pattern7_63_32 implicitly masked by data type */
  /* pZf->m_uSb1Pattern7_95_64 implicitly masked by data type */
  /* pZf->m_uSb1Pattern7_127_96 implicitly masked by data type */
  /* pZf->m_uSb2Pattern0_31_0 implicitly masked by data type */
  /* pZf->m_uSb2Pattern0_63_32 implicitly masked by data type */
  /* pZf->m_uSb2Pattern0_95_64 implicitly masked by data type */
  /* pZf->m_uSb2Pattern0_127_96 implicitly masked by data type */
  /* pZf->m_uSb2Pattern1_31_0 implicitly masked by data type */
  /* pZf->m_uSb2Pattern1_63_32 implicitly masked by data type */
  /* pZf->m_uSb2Pattern1_95_64 implicitly masked by data type */
  /* pZf->m_uSb2Pattern1_127_96 implicitly masked by data type */
  /* pZf->m_uSb2Pattern2_31_0 implicitly masked by data type */
  /* pZf->m_uSb2Pattern2_63_32 implicitly masked by data type */
  /* pZf->m_uSb2Pattern2_95_64 implicitly masked by data type */
  /* pZf->m_uSb2Pattern2_127_96 implicitly masked by data type */
  /* pZf->m_uSb2Pattern3_31_0 implicitly masked by data type */
  /* pZf->m_uSb2Pattern3_63_32 implicitly masked by data type */
  /* pZf->m_uSb2Pattern3_95_64 implicitly masked by data type */
  /* pZf->m_uSb2Pattern3_127_96 implicitly masked by data type */
  /* pZf->m_uSb2Pattern4_31_0 implicitly masked by data type */
  /* pZf->m_uSb2Pattern4_63_32 implicitly masked by data type */
  /* pZf->m_uSb2Pattern4_95_64 implicitly masked by data type */
  /* pZf->m_uSb2Pattern4_127_96 implicitly masked by data type */
  /* pZf->m_uSb2Pattern5_31_0 implicitly masked by data type */
  /* pZf->m_uSb2Pattern5_63_32 implicitly masked by data type */
  /* pZf->m_uSb2Pattern5_95_64 implicitly masked by data type */
  /* pZf->m_uSb2Pattern5_127_96 implicitly masked by data type */
  /* pZf->m_uSb2Pattern6_31_0 implicitly masked by data type */
  /* pZf->m_uSb2Pattern6_63_32 implicitly masked by data type */
  /* pZf->m_uSb2Pattern6_95_64 implicitly masked by data type */
  /* pZf->m_uSb2Pattern6_127_96 implicitly masked by data type */
  /* pZf->m_uSb2Pattern7_31_0 implicitly masked by data type */
  /* pZf->m_uSb2Pattern7_63_32 implicitly masked by data type */
  /* pZf->m_uSb2Pattern7_95_64 implicitly masked by data type */
  /* pZf->m_uSb2Pattern7_127_96 implicitly masked by data type */
  /* pZf->m_uSb3Pattern0_31_0 implicitly masked by data type */
  /* pZf->m_uSb3Pattern0_63_32 implicitly masked by data type */
  /* pZf->m_uSb3Pattern0_95_64 implicitly masked by data type */
  /* pZf->m_uSb3Pattern0_127_96 implicitly masked by data type */
  /* pZf->m_uSb3Pattern1_31_0 implicitly masked by data type */
  /* pZf->m_uSb3Pattern1_63_32 implicitly masked by data type */
  /* pZf->m_uSb3Pattern1_95_64 implicitly masked by data type */
  /* pZf->m_uSb3Pattern1_127_96 implicitly masked by data type */
  /* pZf->m_uSb3Pattern2_31_0 implicitly masked by data type */
  /* pZf->m_uSb3Pattern2_63_32 implicitly masked by data type */
  /* pZf->m_uSb3Pattern2_95_64 implicitly masked by data type */
  /* pZf->m_uSb3Pattern2_127_96 implicitly masked by data type */
  /* pZf->m_uSb3Pattern3_31_0 implicitly masked by data type */
  /* pZf->m_uSb3Pattern3_63_32 implicitly masked by data type */
  /* pZf->m_uSb3Pattern3_95_64 implicitly masked by data type */
  /* pZf->m_uSb3Pattern3_127_96 implicitly masked by data type */
  /* pZf->m_uSb3Pattern4_31_0 implicitly masked by data type */
  /* pZf->m_uSb3Pattern4_63_32 implicitly masked by data type */
  /* pZf->m_uSb3Pattern4_95_64 implicitly masked by data type */
  /* pZf->m_uSb3Pattern4_127_96 implicitly masked by data type */
  /* pZf->m_uSb3Pattern5_31_0 implicitly masked by data type */
  /* pZf->m_uSb3Pattern5_63_32 implicitly masked by data type */
  /* pZf->m_uSb3Pattern5_95_64 implicitly masked by data type */
  /* pZf->m_uSb3Pattern5_127_96 implicitly masked by data type */
  /* pZf->m_uSb3Pattern6_31_0 implicitly masked by data type */
  /* pZf->m_uSb3Pattern6_63_32 implicitly masked by data type */
  /* pZf->m_uSb3Pattern6_95_64 implicitly masked by data type */
  /* pZf->m_uSb3Pattern6_127_96 implicitly masked by data type */
  /* pZf->m_uSb3Pattern7_31_0 implicitly masked by data type */
  /* pZf->m_uSb3Pattern7_63_32 implicitly masked by data type */
  /* pZf->m_uSb3Pattern7_95_64 implicitly masked by data type */
  /* pZf->m_uSb3Pattern7_127_96 implicitly masked by data type */
  /* pZf->m_uSb4Pattern0_31_0 implicitly masked by data type */
  /* pZf->m_uSb4Pattern0_63_32 implicitly masked by data type */
  /* pZf->m_uSb4Pattern0_95_64 implicitly masked by data type */
  /* pZf->m_uSb4Pattern0_127_96 implicitly masked by data type */
  /* pZf->m_uSb4Pattern1_31_0 implicitly masked by data type */
  /* pZf->m_uSb4Pattern1_63_32 implicitly masked by data type */
  /* pZf->m_uSb4Pattern1_95_64 implicitly masked by data type */
  /* pZf->m_uSb4Pattern1_127_96 implicitly masked by data type */
  /* pZf->m_uSb4Pattern2_31_0 implicitly masked by data type */
  /* pZf->m_uSb4Pattern2_63_32 implicitly masked by data type */
  /* pZf->m_uSb4Pattern2_95_64 implicitly masked by data type */
  /* pZf->m_uSb4Pattern2_127_96 implicitly masked by data type */
  /* pZf->m_uSb4Pattern3_31_0 implicitly masked by data type */
  /* pZf->m_uSb4Pattern3_63_32 implicitly masked by data type */
  /* pZf->m_uSb4Pattern3_95_64 implicitly masked by data type */
  /* pZf->m_uSb4Pattern3_127_96 implicitly masked by data type */
  /* pZf->m_uSb4Pattern4_31_0 implicitly masked by data type */
  /* pZf->m_uSb4Pattern4_63_32 implicitly masked by data type */
  /* pZf->m_uSb4Pattern4_95_64 implicitly masked by data type */
  /* pZf->m_uSb4Pattern4_127_96 implicitly masked by data type */
  /* pZf->m_uSb4Pattern5_31_0 implicitly masked by data type */
  /* pZf->m_uSb4Pattern5_63_32 implicitly masked by data type */
  /* pZf->m_uSb4Pattern5_95_64 implicitly masked by data type */
  /* pZf->m_uSb4Pattern5_127_96 implicitly masked by data type */
  /* pZf->m_uSb4Pattern6_31_0 implicitly masked by data type */
  /* pZf->m_uSb4Pattern6_63_32 implicitly masked by data type */
  /* pZf->m_uSb4Pattern6_95_64 implicitly masked by data type */
  /* pZf->m_uSb4Pattern6_127_96 implicitly masked by data type */
  /* pZf->m_uSb4Pattern7_31_0 implicitly masked by data type */
  /* pZf->m_uSb4Pattern7_63_32 implicitly masked by data type */
  /* pZf->m_uSb4Pattern7_95_64 implicitly masked by data type */
  /* pZf->m_uSb4Pattern7_127_96 implicitly masked by data type */
  /* pZf->m_uSb5Pattern0_31_0 implicitly masked by data type */
  /* pZf->m_uSb5Pattern0_63_32 implicitly masked by data type */
  /* pZf->m_uSb5Pattern0_95_64 implicitly masked by data type */
  /* pZf->m_uSb5Pattern0_127_96 implicitly masked by data type */
  /* pZf->m_uSb5Pattern1_31_0 implicitly masked by data type */
  /* pZf->m_uSb5Pattern1_63_32 implicitly masked by data type */
  /* pZf->m_uSb5Pattern1_95_64 implicitly masked by data type */
  /* pZf->m_uSb5Pattern1_127_96 implicitly masked by data type */
  /* pZf->m_uSb5Pattern2_31_0 implicitly masked by data type */
  /* pZf->m_uSb5Pattern2_63_32 implicitly masked by data type */
  /* pZf->m_uSb5Pattern2_95_64 implicitly masked by data type */
  /* pZf->m_uSb5Pattern2_127_96 implicitly masked by data type */
  /* pZf->m_uSb5Pattern3_31_0 implicitly masked by data type */
  /* pZf->m_uSb5Pattern3_63_32 implicitly masked by data type */
  /* pZf->m_uSb5Pattern3_95_64 implicitly masked by data type */
  /* pZf->m_uSb5Pattern3_127_96 implicitly masked by data type */
  /* pZf->m_uSb5Pattern4_31_0 implicitly masked by data type */
  /* pZf->m_uSb5Pattern4_63_32 implicitly masked by data type */
  /* pZf->m_uSb5Pattern4_95_64 implicitly masked by data type */
  /* pZf->m_uSb5Pattern4_127_96 implicitly masked by data type */
  /* pZf->m_uSb5Pattern5_31_0 implicitly masked by data type */
  /* pZf->m_uSb5Pattern5_63_32 implicitly masked by data type */
  /* pZf->m_uSb5Pattern5_95_64 implicitly masked by data type */
  /* pZf->m_uSb5Pattern5_127_96 implicitly masked by data type */
  /* pZf->m_uSb5Pattern6_31_0 implicitly masked by data type */
  /* pZf->m_uSb5Pattern6_63_32 implicitly masked by data type */
  /* pZf->m_uSb5Pattern6_95_64 implicitly masked by data type */
  /* pZf->m_uSb5Pattern6_127_96 implicitly masked by data type */
  /* pZf->m_uSb5Pattern7_31_0 implicitly masked by data type */
  /* pZf->m_uSb5Pattern7_63_32 implicitly masked by data type */
  /* pZf->m_uSb5Pattern7_95_64 implicitly masked by data type */
  /* pZf->m_uSb5Pattern7_127_96 implicitly masked by data type */
  /* pZf->m_uResv24 implicitly masked by data type */
  /* pZf->m_uResv25 implicitly masked by data type */
  /* pZf->m_uResv26 implicitly masked by data type */
  /* pZf->m_uResv27 implicitly masked by data type */
  /* pZf->m_uResv28 implicitly masked by data type */
  /* pZf->m_uResv29 implicitly masked by data type */
  /* pZf->m_uResv30 implicitly masked by data type */
  /* pZf->m_uResv31 implicitly masked by data type */
  /* pZf->m_uResv32 implicitly masked by data type */
  /* pZf->m_uResv33 implicitly masked by data type */
  /* pZf->m_uResv34 implicitly masked by data type */
  /* pZf->m_uResv35 implicitly masked by data type */
  /* pZf->m_uResv36 implicitly masked by data type */
  /* pZf->m_uResv37 implicitly masked by data type */
  /* pZf->m_uResv38 implicitly masked by data type */
  /* pZf->m_uResv39 implicitly masked by data type */
  /* pZf->m_uResv40 implicitly masked by data type */
  /* pZf->m_uResv41 implicitly masked by data type */
  /* pZf->m_uResv42 implicitly masked by data type */
  /* pZf->m_uResv43 implicitly masked by data type */
  /* pZf->m_uResv44 implicitly masked by data type */
  /* pZf->m_uResv45 implicitly masked by data type */
  /* pZf->m_uResv46 implicitly masked by data type */
  /* pZf->m_uResv47 implicitly masked by data type */
  /* pZf->m_uResv48 implicitly masked by data type */
  /* pZf->m_uResv49 implicitly masked by data type */
  /* pZf->m_uResv50 implicitly masked by data type */
  /* pZf->m_uResv51 implicitly masked by data type */
  /* pZf->m_uResv52 implicitly masked by data type */
  /* pZf->m_uResv53 implicitly masked by data type */
  /* pZf->m_uResv54 implicitly masked by data type */
  /* pZf->m_uResv55 implicitly masked by data type */

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfC2RcDmaFormat_SetField(sbZfC2RcDmaFormat_t *s, char* name, int value) {

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
  } else if (SB_STRCMP(name, "m_uinstruction4") == 0) {
    s->m_uInstruction4 = value;
  } else if (SB_STRCMP(name, "m_uinstruction5") == 0) {
    s->m_uInstruction5 = value;
  } else if (SB_STRCMP(name, "m_uinstruction6") == 0) {
    s->m_uInstruction6 = value;
  } else if (SB_STRCMP(name, "m_uinstruction7") == 0) {
    s->m_uInstruction7 = value;
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
  } else if (SB_STRCMP(name, "m_usb0pattern4_31_0") == 0) {
    s->m_uSb0Pattern4_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb0pattern4_63_32") == 0) {
    s->m_uSb0Pattern4_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb0pattern4_95_64") == 0) {
    s->m_uSb0Pattern4_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb0pattern4_127_96") == 0) {
    s->m_uSb0Pattern4_127_96 = value;
  } else if (SB_STRCMP(name, "m_usb0pattern5_31_0") == 0) {
    s->m_uSb0Pattern5_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb0pattern5_63_32") == 0) {
    s->m_uSb0Pattern5_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb0pattern5_95_64") == 0) {
    s->m_uSb0Pattern5_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb0pattern5_127_96") == 0) {
    s->m_uSb0Pattern5_127_96 = value;
  } else if (SB_STRCMP(name, "m_usb0pattern6_31_0") == 0) {
    s->m_uSb0Pattern6_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb0pattern6_63_32") == 0) {
    s->m_uSb0Pattern6_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb0pattern6_95_64") == 0) {
    s->m_uSb0Pattern6_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb0pattern6_127_96") == 0) {
    s->m_uSb0Pattern6_127_96 = value;
  } else if (SB_STRCMP(name, "m_usb0pattern7_31_0") == 0) {
    s->m_uSb0Pattern7_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb0pattern7_63_32") == 0) {
    s->m_uSb0Pattern7_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb0pattern7_95_64") == 0) {
    s->m_uSb0Pattern7_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb0pattern7_127_96") == 0) {
    s->m_uSb0Pattern7_127_96 = value;
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
  } else if (SB_STRCMP(name, "m_usb1pattern4_31_0") == 0) {
    s->m_uSb1Pattern4_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb1pattern4_63_32") == 0) {
    s->m_uSb1Pattern4_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb1pattern4_95_64") == 0) {
    s->m_uSb1Pattern4_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb1pattern4_127_96") == 0) {
    s->m_uSb1Pattern4_127_96 = value;
  } else if (SB_STRCMP(name, "m_usb1pattern5_31_0") == 0) {
    s->m_uSb1Pattern5_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb1pattern5_63_32") == 0) {
    s->m_uSb1Pattern5_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb1pattern5_95_64") == 0) {
    s->m_uSb1Pattern5_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb1pattern5_127_96") == 0) {
    s->m_uSb1Pattern5_127_96 = value;
  } else if (SB_STRCMP(name, "m_usb1pattern6_31_0") == 0) {
    s->m_uSb1Pattern6_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb1pattern6_63_32") == 0) {
    s->m_uSb1Pattern6_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb1pattern6_95_64") == 0) {
    s->m_uSb1Pattern6_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb1pattern6_127_96") == 0) {
    s->m_uSb1Pattern6_127_96 = value;
  } else if (SB_STRCMP(name, "m_usb1pattern7_31_0") == 0) {
    s->m_uSb1Pattern7_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb1pattern7_63_32") == 0) {
    s->m_uSb1Pattern7_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb1pattern7_95_64") == 0) {
    s->m_uSb1Pattern7_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb1pattern7_127_96") == 0) {
    s->m_uSb1Pattern7_127_96 = value;
  } else if (SB_STRCMP(name, "m_usb2pattern0_31_0") == 0) {
    s->m_uSb2Pattern0_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb2pattern0_63_32") == 0) {
    s->m_uSb2Pattern0_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb2pattern0_95_64") == 0) {
    s->m_uSb2Pattern0_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb2pattern0_127_96") == 0) {
    s->m_uSb2Pattern0_127_96 = value;
  } else if (SB_STRCMP(name, "m_usb2pattern1_31_0") == 0) {
    s->m_uSb2Pattern1_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb2pattern1_63_32") == 0) {
    s->m_uSb2Pattern1_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb2pattern1_95_64") == 0) {
    s->m_uSb2Pattern1_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb2pattern1_127_96") == 0) {
    s->m_uSb2Pattern1_127_96 = value;
  } else if (SB_STRCMP(name, "m_usb2pattern2_31_0") == 0) {
    s->m_uSb2Pattern2_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb2pattern2_63_32") == 0) {
    s->m_uSb2Pattern2_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb2pattern2_95_64") == 0) {
    s->m_uSb2Pattern2_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb2pattern2_127_96") == 0) {
    s->m_uSb2Pattern2_127_96 = value;
  } else if (SB_STRCMP(name, "m_usb2pattern3_31_0") == 0) {
    s->m_uSb2Pattern3_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb2pattern3_63_32") == 0) {
    s->m_uSb2Pattern3_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb2pattern3_95_64") == 0) {
    s->m_uSb2Pattern3_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb2pattern3_127_96") == 0) {
    s->m_uSb2Pattern3_127_96 = value;
  } else if (SB_STRCMP(name, "m_usb2pattern4_31_0") == 0) {
    s->m_uSb2Pattern4_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb2pattern4_63_32") == 0) {
    s->m_uSb2Pattern4_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb2pattern4_95_64") == 0) {
    s->m_uSb2Pattern4_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb2pattern4_127_96") == 0) {
    s->m_uSb2Pattern4_127_96 = value;
  } else if (SB_STRCMP(name, "m_usb2pattern5_31_0") == 0) {
    s->m_uSb2Pattern5_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb2pattern5_63_32") == 0) {
    s->m_uSb2Pattern5_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb2pattern5_95_64") == 0) {
    s->m_uSb2Pattern5_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb2pattern5_127_96") == 0) {
    s->m_uSb2Pattern5_127_96 = value;
  } else if (SB_STRCMP(name, "m_usb2pattern6_31_0") == 0) {
    s->m_uSb2Pattern6_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb2pattern6_63_32") == 0) {
    s->m_uSb2Pattern6_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb2pattern6_95_64") == 0) {
    s->m_uSb2Pattern6_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb2pattern6_127_96") == 0) {
    s->m_uSb2Pattern6_127_96 = value;
  } else if (SB_STRCMP(name, "m_usb2pattern7_31_0") == 0) {
    s->m_uSb2Pattern7_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb2pattern7_63_32") == 0) {
    s->m_uSb2Pattern7_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb2pattern7_95_64") == 0) {
    s->m_uSb2Pattern7_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb2pattern7_127_96") == 0) {
    s->m_uSb2Pattern7_127_96 = value;
  } else if (SB_STRCMP(name, "m_usb3pattern0_31_0") == 0) {
    s->m_uSb3Pattern0_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb3pattern0_63_32") == 0) {
    s->m_uSb3Pattern0_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb3pattern0_95_64") == 0) {
    s->m_uSb3Pattern0_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb3pattern0_127_96") == 0) {
    s->m_uSb3Pattern0_127_96 = value;
  } else if (SB_STRCMP(name, "m_usb3pattern1_31_0") == 0) {
    s->m_uSb3Pattern1_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb3pattern1_63_32") == 0) {
    s->m_uSb3Pattern1_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb3pattern1_95_64") == 0) {
    s->m_uSb3Pattern1_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb3pattern1_127_96") == 0) {
    s->m_uSb3Pattern1_127_96 = value;
  } else if (SB_STRCMP(name, "m_usb3pattern2_31_0") == 0) {
    s->m_uSb3Pattern2_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb3pattern2_63_32") == 0) {
    s->m_uSb3Pattern2_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb3pattern2_95_64") == 0) {
    s->m_uSb3Pattern2_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb3pattern2_127_96") == 0) {
    s->m_uSb3Pattern2_127_96 = value;
  } else if (SB_STRCMP(name, "m_usb3pattern3_31_0") == 0) {
    s->m_uSb3Pattern3_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb3pattern3_63_32") == 0) {
    s->m_uSb3Pattern3_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb3pattern3_95_64") == 0) {
    s->m_uSb3Pattern3_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb3pattern3_127_96") == 0) {
    s->m_uSb3Pattern3_127_96 = value;
  } else if (SB_STRCMP(name, "m_usb3pattern4_31_0") == 0) {
    s->m_uSb3Pattern4_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb3pattern4_63_32") == 0) {
    s->m_uSb3Pattern4_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb3pattern4_95_64") == 0) {
    s->m_uSb3Pattern4_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb3pattern4_127_96") == 0) {
    s->m_uSb3Pattern4_127_96 = value;
  } else if (SB_STRCMP(name, "m_usb3pattern5_31_0") == 0) {
    s->m_uSb3Pattern5_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb3pattern5_63_32") == 0) {
    s->m_uSb3Pattern5_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb3pattern5_95_64") == 0) {
    s->m_uSb3Pattern5_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb3pattern5_127_96") == 0) {
    s->m_uSb3Pattern5_127_96 = value;
  } else if (SB_STRCMP(name, "m_usb3pattern6_31_0") == 0) {
    s->m_uSb3Pattern6_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb3pattern6_63_32") == 0) {
    s->m_uSb3Pattern6_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb3pattern6_95_64") == 0) {
    s->m_uSb3Pattern6_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb3pattern6_127_96") == 0) {
    s->m_uSb3Pattern6_127_96 = value;
  } else if (SB_STRCMP(name, "m_usb3pattern7_31_0") == 0) {
    s->m_uSb3Pattern7_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb3pattern7_63_32") == 0) {
    s->m_uSb3Pattern7_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb3pattern7_95_64") == 0) {
    s->m_uSb3Pattern7_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb3pattern7_127_96") == 0) {
    s->m_uSb3Pattern7_127_96 = value;
  } else if (SB_STRCMP(name, "m_usb4pattern0_31_0") == 0) {
    s->m_uSb4Pattern0_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb4pattern0_63_32") == 0) {
    s->m_uSb4Pattern0_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb4pattern0_95_64") == 0) {
    s->m_uSb4Pattern0_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb4pattern0_127_96") == 0) {
    s->m_uSb4Pattern0_127_96 = value;
  } else if (SB_STRCMP(name, "m_usb4pattern1_31_0") == 0) {
    s->m_uSb4Pattern1_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb4pattern1_63_32") == 0) {
    s->m_uSb4Pattern1_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb4pattern1_95_64") == 0) {
    s->m_uSb4Pattern1_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb4pattern1_127_96") == 0) {
    s->m_uSb4Pattern1_127_96 = value;
  } else if (SB_STRCMP(name, "m_usb4pattern2_31_0") == 0) {
    s->m_uSb4Pattern2_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb4pattern2_63_32") == 0) {
    s->m_uSb4Pattern2_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb4pattern2_95_64") == 0) {
    s->m_uSb4Pattern2_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb4pattern2_127_96") == 0) {
    s->m_uSb4Pattern2_127_96 = value;
  } else if (SB_STRCMP(name, "m_usb4pattern3_31_0") == 0) {
    s->m_uSb4Pattern3_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb4pattern3_63_32") == 0) {
    s->m_uSb4Pattern3_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb4pattern3_95_64") == 0) {
    s->m_uSb4Pattern3_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb4pattern3_127_96") == 0) {
    s->m_uSb4Pattern3_127_96 = value;
  } else if (SB_STRCMP(name, "m_usb4pattern4_31_0") == 0) {
    s->m_uSb4Pattern4_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb4pattern4_63_32") == 0) {
    s->m_uSb4Pattern4_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb4pattern4_95_64") == 0) {
    s->m_uSb4Pattern4_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb4pattern4_127_96") == 0) {
    s->m_uSb4Pattern4_127_96 = value;
  } else if (SB_STRCMP(name, "m_usb4pattern5_31_0") == 0) {
    s->m_uSb4Pattern5_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb4pattern5_63_32") == 0) {
    s->m_uSb4Pattern5_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb4pattern5_95_64") == 0) {
    s->m_uSb4Pattern5_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb4pattern5_127_96") == 0) {
    s->m_uSb4Pattern5_127_96 = value;
  } else if (SB_STRCMP(name, "m_usb4pattern6_31_0") == 0) {
    s->m_uSb4Pattern6_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb4pattern6_63_32") == 0) {
    s->m_uSb4Pattern6_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb4pattern6_95_64") == 0) {
    s->m_uSb4Pattern6_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb4pattern6_127_96") == 0) {
    s->m_uSb4Pattern6_127_96 = value;
  } else if (SB_STRCMP(name, "m_usb4pattern7_31_0") == 0) {
    s->m_uSb4Pattern7_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb4pattern7_63_32") == 0) {
    s->m_uSb4Pattern7_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb4pattern7_95_64") == 0) {
    s->m_uSb4Pattern7_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb4pattern7_127_96") == 0) {
    s->m_uSb4Pattern7_127_96 = value;
  } else if (SB_STRCMP(name, "m_usb5pattern0_31_0") == 0) {
    s->m_uSb5Pattern0_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb5pattern0_63_32") == 0) {
    s->m_uSb5Pattern0_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb5pattern0_95_64") == 0) {
    s->m_uSb5Pattern0_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb5pattern0_127_96") == 0) {
    s->m_uSb5Pattern0_127_96 = value;
  } else if (SB_STRCMP(name, "m_usb5pattern1_31_0") == 0) {
    s->m_uSb5Pattern1_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb5pattern1_63_32") == 0) {
    s->m_uSb5Pattern1_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb5pattern1_95_64") == 0) {
    s->m_uSb5Pattern1_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb5pattern1_127_96") == 0) {
    s->m_uSb5Pattern1_127_96 = value;
  } else if (SB_STRCMP(name, "m_usb5pattern2_31_0") == 0) {
    s->m_uSb5Pattern2_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb5pattern2_63_32") == 0) {
    s->m_uSb5Pattern2_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb5pattern2_95_64") == 0) {
    s->m_uSb5Pattern2_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb5pattern2_127_96") == 0) {
    s->m_uSb5Pattern2_127_96 = value;
  } else if (SB_STRCMP(name, "m_usb5pattern3_31_0") == 0) {
    s->m_uSb5Pattern3_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb5pattern3_63_32") == 0) {
    s->m_uSb5Pattern3_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb5pattern3_95_64") == 0) {
    s->m_uSb5Pattern3_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb5pattern3_127_96") == 0) {
    s->m_uSb5Pattern3_127_96 = value;
  } else if (SB_STRCMP(name, "m_usb5pattern4_31_0") == 0) {
    s->m_uSb5Pattern4_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb5pattern4_63_32") == 0) {
    s->m_uSb5Pattern4_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb5pattern4_95_64") == 0) {
    s->m_uSb5Pattern4_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb5pattern4_127_96") == 0) {
    s->m_uSb5Pattern4_127_96 = value;
  } else if (SB_STRCMP(name, "m_usb5pattern5_31_0") == 0) {
    s->m_uSb5Pattern5_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb5pattern5_63_32") == 0) {
    s->m_uSb5Pattern5_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb5pattern5_95_64") == 0) {
    s->m_uSb5Pattern5_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb5pattern5_127_96") == 0) {
    s->m_uSb5Pattern5_127_96 = value;
  } else if (SB_STRCMP(name, "m_usb5pattern6_31_0") == 0) {
    s->m_uSb5Pattern6_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb5pattern6_63_32") == 0) {
    s->m_uSb5Pattern6_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb5pattern6_95_64") == 0) {
    s->m_uSb5Pattern6_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb5pattern6_127_96") == 0) {
    s->m_uSb5Pattern6_127_96 = value;
  } else if (SB_STRCMP(name, "m_usb5pattern7_31_0") == 0) {
    s->m_uSb5Pattern7_31_0 = value;
  } else if (SB_STRCMP(name, "m_usb5pattern7_63_32") == 0) {
    s->m_uSb5Pattern7_63_32 = value;
  } else if (SB_STRCMP(name, "m_usb5pattern7_95_64") == 0) {
    s->m_uSb5Pattern7_95_64 = value;
  } else if (SB_STRCMP(name, "m_usb5pattern7_127_96") == 0) {
    s->m_uSb5Pattern7_127_96 = value;
  } else if (SB_STRCMP(name, "m_uresv24") == 0) {
    s->m_uResv24 = value;
  } else if (SB_STRCMP(name, "m_uresv25") == 0) {
    s->m_uResv25 = value;
  } else if (SB_STRCMP(name, "m_uresv26") == 0) {
    s->m_uResv26 = value;
  } else if (SB_STRCMP(name, "m_uresv27") == 0) {
    s->m_uResv27 = value;
  } else if (SB_STRCMP(name, "m_uresv28") == 0) {
    s->m_uResv28 = value;
  } else if (SB_STRCMP(name, "m_uresv29") == 0) {
    s->m_uResv29 = value;
  } else if (SB_STRCMP(name, "m_uresv30") == 0) {
    s->m_uResv30 = value;
  } else if (SB_STRCMP(name, "m_uresv31") == 0) {
    s->m_uResv31 = value;
  } else if (SB_STRCMP(name, "m_uresv32") == 0) {
    s->m_uResv32 = value;
  } else if (SB_STRCMP(name, "m_uresv33") == 0) {
    s->m_uResv33 = value;
  } else if (SB_STRCMP(name, "m_uresv34") == 0) {
    s->m_uResv34 = value;
  } else if (SB_STRCMP(name, "m_uresv35") == 0) {
    s->m_uResv35 = value;
  } else if (SB_STRCMP(name, "m_uresv36") == 0) {
    s->m_uResv36 = value;
  } else if (SB_STRCMP(name, "m_uresv37") == 0) {
    s->m_uResv37 = value;
  } else if (SB_STRCMP(name, "m_uresv38") == 0) {
    s->m_uResv38 = value;
  } else if (SB_STRCMP(name, "m_uresv39") == 0) {
    s->m_uResv39 = value;
  } else if (SB_STRCMP(name, "m_uresv40") == 0) {
    s->m_uResv40 = value;
  } else if (SB_STRCMP(name, "m_uresv41") == 0) {
    s->m_uResv41 = value;
  } else if (SB_STRCMP(name, "m_uresv42") == 0) {
    s->m_uResv42 = value;
  } else if (SB_STRCMP(name, "m_uresv43") == 0) {
    s->m_uResv43 = value;
  } else if (SB_STRCMP(name, "m_uresv44") == 0) {
    s->m_uResv44 = value;
  } else if (SB_STRCMP(name, "m_uresv45") == 0) {
    s->m_uResv45 = value;
  } else if (SB_STRCMP(name, "m_uresv46") == 0) {
    s->m_uResv46 = value;
  } else if (SB_STRCMP(name, "m_uresv47") == 0) {
    s->m_uResv47 = value;
  } else if (SB_STRCMP(name, "m_uresv48") == 0) {
    s->m_uResv48 = value;
  } else if (SB_STRCMP(name, "m_uresv49") == 0) {
    s->m_uResv49 = value;
  } else if (SB_STRCMP(name, "m_uresv50") == 0) {
    s->m_uResv50 = value;
  } else if (SB_STRCMP(name, "m_uresv51") == 0) {
    s->m_uResv51 = value;
  } else if (SB_STRCMP(name, "m_uresv52") == 0) {
    s->m_uResv52 = value;
  } else if (SB_STRCMP(name, "m_uresv53") == 0) {
    s->m_uResv53 = value;
  } else if (SB_STRCMP(name, "m_uresv54") == 0) {
    s->m_uResv54 = value;
  } else if (SB_STRCMP(name, "m_uresv55") == 0) {
    s->m_uResv55 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
