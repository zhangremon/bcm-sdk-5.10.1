/*
 * $Id: sbZfSbQe2000ElibPriTableConsole.c 1.3.36.3 Broadcom SDK $
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
#include "sbZfSbQe2000ElibPriTableConsole.hx"



/* Print members in struct */
void
sbZfSbQe2000ElibPriTable_Print(sbZfSbQe2000ElibPriTable_t *pFromStruct) {
  SB_LOG("SbQe2000ElibPriTable:: rsvd7=0x%02x", (unsigned int)  pFromStruct->Rsvd7);
  SB_LOG(" pri63=0x%01x", (unsigned int)  pFromStruct->Pri63);
  SB_LOG(" pri62=0x%01x", (unsigned int)  pFromStruct->Pri62);
  SB_LOG(" pri61=0x%01x", (unsigned int)  pFromStruct->Pri61);
  SB_LOG(" pri60=0x%01x", (unsigned int)  pFromStruct->Pri60);
  SB_LOG("\n");

  SB_LOG("SbQe2000ElibPriTable:: pri59=0x%01x", (unsigned int)  pFromStruct->Pri59);
  SB_LOG(" pri58=0x%01x", (unsigned int)  pFromStruct->Pri58);
  SB_LOG(" pri57=0x%01x", (unsigned int)  pFromStruct->Pri57);
  SB_LOG(" pri56=0x%01x", (unsigned int)  pFromStruct->Pri56);
  SB_LOG(" rsvd6=0x%02x", (unsigned int)  pFromStruct->Rsvd6);
  SB_LOG("\n");

  SB_LOG("SbQe2000ElibPriTable:: pri55=0x%01x", (unsigned int)  pFromStruct->Pri55);
  SB_LOG(" pri54=0x%01x", (unsigned int)  pFromStruct->Pri54);
  SB_LOG(" pri53=0x%01x", (unsigned int)  pFromStruct->Pri53);
  SB_LOG(" pri52=0x%01x", (unsigned int)  pFromStruct->Pri52);
  SB_LOG(" pri51=0x%01x", (unsigned int)  pFromStruct->Pri51);
  SB_LOG("\n");

  SB_LOG("SbQe2000ElibPriTable:: pri50=0x%01x", (unsigned int)  pFromStruct->Pri50);
  SB_LOG(" pri49=0x%01x", (unsigned int)  pFromStruct->Pri49);
  SB_LOG(" pri48=0x%01x", (unsigned int)  pFromStruct->Pri48);
  SB_LOG(" rsvd5=0x%02x", (unsigned int)  pFromStruct->Rsvd5);
  SB_LOG(" pri47=0x%01x", (unsigned int)  pFromStruct->Pri47);
  SB_LOG("\n");

  SB_LOG("SbQe2000ElibPriTable:: pri46=0x%01x", (unsigned int)  pFromStruct->Pri46);
  SB_LOG(" pri45=0x%01x", (unsigned int)  pFromStruct->Pri45);
  SB_LOG(" pri44=0x%01x", (unsigned int)  pFromStruct->Pri44);
  SB_LOG(" pri43=0x%01x", (unsigned int)  pFromStruct->Pri43);
  SB_LOG(" pri42=0x%01x", (unsigned int)  pFromStruct->Pri42);
  SB_LOG("\n");

  SB_LOG("SbQe2000ElibPriTable:: pri41=0x%01x", (unsigned int)  pFromStruct->Pri41);
  SB_LOG(" pri40=0x%01x", (unsigned int)  pFromStruct->Pri40);
  SB_LOG(" rsvd4=0x%02x", (unsigned int)  pFromStruct->Rsvd4);
  SB_LOG(" pri39=0x%01x", (unsigned int)  pFromStruct->Pri39);
  SB_LOG(" pri38=0x%01x", (unsigned int)  pFromStruct->Pri38);
  SB_LOG("\n");

  SB_LOG("SbQe2000ElibPriTable:: pri37=0x%01x", (unsigned int)  pFromStruct->Pri37);
  SB_LOG(" pri36=0x%01x", (unsigned int)  pFromStruct->Pri36);
  SB_LOG(" pri35=0x%01x", (unsigned int)  pFromStruct->Pri35);
  SB_LOG(" pri34=0x%01x", (unsigned int)  pFromStruct->Pri34);
  SB_LOG(" pri33=0x%01x", (unsigned int)  pFromStruct->Pri33);
  SB_LOG("\n");

  SB_LOG("SbQe2000ElibPriTable:: pri32=0x%01x", (unsigned int)  pFromStruct->Pri32);
  SB_LOG(" rsvd3=0x%02x", (unsigned int)  pFromStruct->Rsvd3);
  SB_LOG(" pri31=0x%01x", (unsigned int)  pFromStruct->Pri31);
  SB_LOG(" pri30=0x%01x", (unsigned int)  pFromStruct->Pri30);
  SB_LOG(" pri29=0x%01x", (unsigned int)  pFromStruct->Pri29);
  SB_LOG("\n");

  SB_LOG("SbQe2000ElibPriTable:: pri28=0x%01x", (unsigned int)  pFromStruct->Pri28);
  SB_LOG(" pri27=0x%01x", (unsigned int)  pFromStruct->Pri27);
  SB_LOG(" pri26=0x%01x", (unsigned int)  pFromStruct->Pri26);
  SB_LOG(" pri25=0x%01x", (unsigned int)  pFromStruct->Pri25);
  SB_LOG(" pri24=0x%01x", (unsigned int)  pFromStruct->Pri24);
  SB_LOG("\n");

  SB_LOG("SbQe2000ElibPriTable:: rsvd2=0x%02x", (unsigned int)  pFromStruct->Rsvd2);
  SB_LOG(" pri23=0x%01x", (unsigned int)  pFromStruct->Pri23);
  SB_LOG(" pri22=0x%01x", (unsigned int)  pFromStruct->Pri22);
  SB_LOG(" pri21=0x%01x", (unsigned int)  pFromStruct->Pri21);
  SB_LOG(" pri20=0x%01x", (unsigned int)  pFromStruct->Pri20);
  SB_LOG("\n");

  SB_LOG("SbQe2000ElibPriTable:: pri19=0x%01x", (unsigned int)  pFromStruct->Pri19);
  SB_LOG(" pri18=0x%01x", (unsigned int)  pFromStruct->Pri18);
  SB_LOG(" pri17=0x%01x", (unsigned int)  pFromStruct->Pri17);
  SB_LOG(" pri16=0x%01x", (unsigned int)  pFromStruct->Pri16);
  SB_LOG(" rsvd1=0x%02x", (unsigned int)  pFromStruct->Rsvd1);
  SB_LOG("\n");

  SB_LOG("SbQe2000ElibPriTable:: pri15=0x%01x", (unsigned int)  pFromStruct->Pri15);
  SB_LOG(" pri14=0x%01x", (unsigned int)  pFromStruct->Pri14);
  SB_LOG(" pri13=0x%01x", (unsigned int)  pFromStruct->Pri13);
  SB_LOG(" pri12=0x%01x", (unsigned int)  pFromStruct->Pri12);
  SB_LOG(" pri11=0x%01x", (unsigned int)  pFromStruct->Pri11);
  SB_LOG("\n");

  SB_LOG("SbQe2000ElibPriTable:: pri10=0x%01x", (unsigned int)  pFromStruct->Pri10);
  SB_LOG(" pri9=0x%01x", (unsigned int)  pFromStruct->Pri9);
  SB_LOG(" pri8=0x%01x", (unsigned int)  pFromStruct->Pri8);
  SB_LOG(" rsvd0=0x%02x", (unsigned int)  pFromStruct->Rsvd0);
  SB_LOG(" pri7=0x%01x", (unsigned int)  pFromStruct->Pri7);
  SB_LOG("\n");

  SB_LOG("SbQe2000ElibPriTable:: pri6=0x%01x", (unsigned int)  pFromStruct->Pri6);
  SB_LOG(" pri5=0x%01x", (unsigned int)  pFromStruct->Pri5);
  SB_LOG(" pri4=0x%01x", (unsigned int)  pFromStruct->Pri4);
  SB_LOG(" pri3=0x%01x", (unsigned int)  pFromStruct->Pri3);
  SB_LOG(" pri2=0x%01x", (unsigned int)  pFromStruct->Pri2);
  SB_LOG("\n");

  SB_LOG("SbQe2000ElibPriTable:: pri1=0x%01x", (unsigned int)  pFromStruct->Pri1);
  SB_LOG(" pri0=0x%01x", (unsigned int)  pFromStruct->Pri0);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfSbQe2000ElibPriTable_SPrint(sbZfSbQe2000ElibPriTable_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPriTable:: rsvd7=0x%02x", (unsigned int)  pFromStruct->Rsvd7);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri63=0x%01x", (unsigned int)  pFromStruct->Pri63);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri62=0x%01x", (unsigned int)  pFromStruct->Pri62);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri61=0x%01x", (unsigned int)  pFromStruct->Pri61);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri60=0x%01x", (unsigned int)  pFromStruct->Pri60);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPriTable:: pri59=0x%01x", (unsigned int)  pFromStruct->Pri59);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri58=0x%01x", (unsigned int)  pFromStruct->Pri58);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri57=0x%01x", (unsigned int)  pFromStruct->Pri57);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri56=0x%01x", (unsigned int)  pFromStruct->Pri56);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," rsvd6=0x%02x", (unsigned int)  pFromStruct->Rsvd6);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPriTable:: pri55=0x%01x", (unsigned int)  pFromStruct->Pri55);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri54=0x%01x", (unsigned int)  pFromStruct->Pri54);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri53=0x%01x", (unsigned int)  pFromStruct->Pri53);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri52=0x%01x", (unsigned int)  pFromStruct->Pri52);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri51=0x%01x", (unsigned int)  pFromStruct->Pri51);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPriTable:: pri50=0x%01x", (unsigned int)  pFromStruct->Pri50);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri49=0x%01x", (unsigned int)  pFromStruct->Pri49);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri48=0x%01x", (unsigned int)  pFromStruct->Pri48);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," rsvd5=0x%02x", (unsigned int)  pFromStruct->Rsvd5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri47=0x%01x", (unsigned int)  pFromStruct->Pri47);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPriTable:: pri46=0x%01x", (unsigned int)  pFromStruct->Pri46);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri45=0x%01x", (unsigned int)  pFromStruct->Pri45);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri44=0x%01x", (unsigned int)  pFromStruct->Pri44);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri43=0x%01x", (unsigned int)  pFromStruct->Pri43);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri42=0x%01x", (unsigned int)  pFromStruct->Pri42);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPriTable:: pri41=0x%01x", (unsigned int)  pFromStruct->Pri41);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri40=0x%01x", (unsigned int)  pFromStruct->Pri40);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," rsvd4=0x%02x", (unsigned int)  pFromStruct->Rsvd4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri39=0x%01x", (unsigned int)  pFromStruct->Pri39);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri38=0x%01x", (unsigned int)  pFromStruct->Pri38);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPriTable:: pri37=0x%01x", (unsigned int)  pFromStruct->Pri37);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri36=0x%01x", (unsigned int)  pFromStruct->Pri36);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri35=0x%01x", (unsigned int)  pFromStruct->Pri35);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri34=0x%01x", (unsigned int)  pFromStruct->Pri34);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri33=0x%01x", (unsigned int)  pFromStruct->Pri33);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPriTable:: pri32=0x%01x", (unsigned int)  pFromStruct->Pri32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," rsvd3=0x%02x", (unsigned int)  pFromStruct->Rsvd3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri31=0x%01x", (unsigned int)  pFromStruct->Pri31);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri30=0x%01x", (unsigned int)  pFromStruct->Pri30);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri29=0x%01x", (unsigned int)  pFromStruct->Pri29);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPriTable:: pri28=0x%01x", (unsigned int)  pFromStruct->Pri28);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri27=0x%01x", (unsigned int)  pFromStruct->Pri27);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri26=0x%01x", (unsigned int)  pFromStruct->Pri26);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri25=0x%01x", (unsigned int)  pFromStruct->Pri25);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri24=0x%01x", (unsigned int)  pFromStruct->Pri24);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPriTable:: rsvd2=0x%02x", (unsigned int)  pFromStruct->Rsvd2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri23=0x%01x", (unsigned int)  pFromStruct->Pri23);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri22=0x%01x", (unsigned int)  pFromStruct->Pri22);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri21=0x%01x", (unsigned int)  pFromStruct->Pri21);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri20=0x%01x", (unsigned int)  pFromStruct->Pri20);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPriTable:: pri19=0x%01x", (unsigned int)  pFromStruct->Pri19);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri18=0x%01x", (unsigned int)  pFromStruct->Pri18);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri17=0x%01x", (unsigned int)  pFromStruct->Pri17);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri16=0x%01x", (unsigned int)  pFromStruct->Pri16);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," rsvd1=0x%02x", (unsigned int)  pFromStruct->Rsvd1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPriTable:: pri15=0x%01x", (unsigned int)  pFromStruct->Pri15);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri14=0x%01x", (unsigned int)  pFromStruct->Pri14);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri13=0x%01x", (unsigned int)  pFromStruct->Pri13);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri12=0x%01x", (unsigned int)  pFromStruct->Pri12);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri11=0x%01x", (unsigned int)  pFromStruct->Pri11);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPriTable:: pri10=0x%01x", (unsigned int)  pFromStruct->Pri10);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri9=0x%01x", (unsigned int)  pFromStruct->Pri9);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri8=0x%01x", (unsigned int)  pFromStruct->Pri8);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," rsvd0=0x%02x", (unsigned int)  pFromStruct->Rsvd0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri7=0x%01x", (unsigned int)  pFromStruct->Pri7);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPriTable:: pri6=0x%01x", (unsigned int)  pFromStruct->Pri6);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri5=0x%01x", (unsigned int)  pFromStruct->Pri5);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri4=0x%01x", (unsigned int)  pFromStruct->Pri4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri3=0x%01x", (unsigned int)  pFromStruct->Pri3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri2=0x%01x", (unsigned int)  pFromStruct->Pri2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"SbQe2000ElibPriTable:: pri1=0x%01x", (unsigned int)  pFromStruct->Pri1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri0=0x%01x", (unsigned int)  pFromStruct->Pri0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfSbQe2000ElibPriTable_Validate(sbZfSbQe2000ElibPriTable_t *pZf) {

  if (pZf->Rsvd7 > 0xff) return 0;
  if (pZf->Pri63 > 0x7) return 0;
  if (pZf->Pri62 > 0x7) return 0;
  if (pZf->Pri61 > 0x7) return 0;
  if (pZf->Pri60 > 0x7) return 0;
  if (pZf->Pri59 > 0x7) return 0;
  if (pZf->Pri58 > 0x7) return 0;
  if (pZf->Pri57 > 0x7) return 0;
  if (pZf->Pri56 > 0x7) return 0;
  if (pZf->Rsvd6 > 0xff) return 0;
  if (pZf->Pri55 > 0x7) return 0;
  if (pZf->Pri54 > 0x7) return 0;
  if (pZf->Pri53 > 0x7) return 0;
  if (pZf->Pri52 > 0x7) return 0;
  if (pZf->Pri51 > 0x7) return 0;
  if (pZf->Pri50 > 0x7) return 0;
  if (pZf->Pri49 > 0x7) return 0;
  if (pZf->Pri48 > 0x7) return 0;
  if (pZf->Rsvd5 > 0xff) return 0;
  if (pZf->Pri47 > 0x7) return 0;
  if (pZf->Pri46 > 0x7) return 0;
  if (pZf->Pri45 > 0x7) return 0;
  if (pZf->Pri44 > 0x7) return 0;
  if (pZf->Pri43 > 0x7) return 0;
  if (pZf->Pri42 > 0x7) return 0;
  if (pZf->Pri41 > 0x7) return 0;
  if (pZf->Pri40 > 0x7) return 0;
  if (pZf->Rsvd4 > 0xff) return 0;
  if (pZf->Pri39 > 0x7) return 0;
  if (pZf->Pri38 > 0x7) return 0;
  if (pZf->Pri37 > 0x7) return 0;
  if (pZf->Pri36 > 0x7) return 0;
  if (pZf->Pri35 > 0x7) return 0;
  if (pZf->Pri34 > 0x7) return 0;
  if (pZf->Pri33 > 0x7) return 0;
  if (pZf->Pri32 > 0x7) return 0;
  if (pZf->Rsvd3 > 0xff) return 0;
  if (pZf->Pri31 > 0x7) return 0;
  if (pZf->Pri30 > 0x7) return 0;
  if (pZf->Pri29 > 0x7) return 0;
  if (pZf->Pri28 > 0x7) return 0;
  if (pZf->Pri27 > 0x7) return 0;
  if (pZf->Pri26 > 0x7) return 0;
  if (pZf->Pri25 > 0x7) return 0;
  if (pZf->Pri24 > 0x7) return 0;
  if (pZf->Rsvd2 > 0xff) return 0;
  if (pZf->Pri23 > 0x7) return 0;
  if (pZf->Pri22 > 0x7) return 0;
  if (pZf->Pri21 > 0x7) return 0;
  if (pZf->Pri20 > 0x7) return 0;
  if (pZf->Pri19 > 0x7) return 0;
  if (pZf->Pri18 > 0x7) return 0;
  if (pZf->Pri17 > 0x7) return 0;
  if (pZf->Pri16 > 0x7) return 0;
  if (pZf->Rsvd1 > 0xff) return 0;
  if (pZf->Pri15 > 0x7) return 0;
  if (pZf->Pri14 > 0x7) return 0;
  if (pZf->Pri13 > 0x7) return 0;
  if (pZf->Pri12 > 0x7) return 0;
  if (pZf->Pri11 > 0x7) return 0;
  if (pZf->Pri10 > 0x7) return 0;
  if (pZf->Pri9 > 0x7) return 0;
  if (pZf->Pri8 > 0x7) return 0;
  if (pZf->Rsvd0 > 0xff) return 0;
  if (pZf->Pri7 > 0x7) return 0;
  if (pZf->Pri6 > 0x7) return 0;
  if (pZf->Pri5 > 0x7) return 0;
  if (pZf->Pri4 > 0x7) return 0;
  if (pZf->Pri3 > 0x7) return 0;
  if (pZf->Pri2 > 0x7) return 0;
  if (pZf->Pri1 > 0x7) return 0;
  if (pZf->Pri0 > 0x7) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfSbQe2000ElibPriTable_SetField(sbZfSbQe2000ElibPriTable_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "rsvd7") == 0) {
    s->Rsvd7 = value;
  } else if (SB_STRCMP(name, "pri63") == 0) {
    s->Pri63 = value;
  } else if (SB_STRCMP(name, "pri62") == 0) {
    s->Pri62 = value;
  } else if (SB_STRCMP(name, "pri61") == 0) {
    s->Pri61 = value;
  } else if (SB_STRCMP(name, "pri60") == 0) {
    s->Pri60 = value;
  } else if (SB_STRCMP(name, "pri59") == 0) {
    s->Pri59 = value;
  } else if (SB_STRCMP(name, "pri58") == 0) {
    s->Pri58 = value;
  } else if (SB_STRCMP(name, "pri57") == 0) {
    s->Pri57 = value;
  } else if (SB_STRCMP(name, "pri56") == 0) {
    s->Pri56 = value;
  } else if (SB_STRCMP(name, "rsvd6") == 0) {
    s->Rsvd6 = value;
  } else if (SB_STRCMP(name, "pri55") == 0) {
    s->Pri55 = value;
  } else if (SB_STRCMP(name, "pri54") == 0) {
    s->Pri54 = value;
  } else if (SB_STRCMP(name, "pri53") == 0) {
    s->Pri53 = value;
  } else if (SB_STRCMP(name, "pri52") == 0) {
    s->Pri52 = value;
  } else if (SB_STRCMP(name, "pri51") == 0) {
    s->Pri51 = value;
  } else if (SB_STRCMP(name, "pri50") == 0) {
    s->Pri50 = value;
  } else if (SB_STRCMP(name, "pri49") == 0) {
    s->Pri49 = value;
  } else if (SB_STRCMP(name, "pri48") == 0) {
    s->Pri48 = value;
  } else if (SB_STRCMP(name, "rsvd5") == 0) {
    s->Rsvd5 = value;
  } else if (SB_STRCMP(name, "pri47") == 0) {
    s->Pri47 = value;
  } else if (SB_STRCMP(name, "pri46") == 0) {
    s->Pri46 = value;
  } else if (SB_STRCMP(name, "pri45") == 0) {
    s->Pri45 = value;
  } else if (SB_STRCMP(name, "pri44") == 0) {
    s->Pri44 = value;
  } else if (SB_STRCMP(name, "pri43") == 0) {
    s->Pri43 = value;
  } else if (SB_STRCMP(name, "pri42") == 0) {
    s->Pri42 = value;
  } else if (SB_STRCMP(name, "pri41") == 0) {
    s->Pri41 = value;
  } else if (SB_STRCMP(name, "pri40") == 0) {
    s->Pri40 = value;
  } else if (SB_STRCMP(name, "rsvd4") == 0) {
    s->Rsvd4 = value;
  } else if (SB_STRCMP(name, "pri39") == 0) {
    s->Pri39 = value;
  } else if (SB_STRCMP(name, "pri38") == 0) {
    s->Pri38 = value;
  } else if (SB_STRCMP(name, "pri37") == 0) {
    s->Pri37 = value;
  } else if (SB_STRCMP(name, "pri36") == 0) {
    s->Pri36 = value;
  } else if (SB_STRCMP(name, "pri35") == 0) {
    s->Pri35 = value;
  } else if (SB_STRCMP(name, "pri34") == 0) {
    s->Pri34 = value;
  } else if (SB_STRCMP(name, "pri33") == 0) {
    s->Pri33 = value;
  } else if (SB_STRCMP(name, "pri32") == 0) {
    s->Pri32 = value;
  } else if (SB_STRCMP(name, "rsvd3") == 0) {
    s->Rsvd3 = value;
  } else if (SB_STRCMP(name, "pri31") == 0) {
    s->Pri31 = value;
  } else if (SB_STRCMP(name, "pri30") == 0) {
    s->Pri30 = value;
  } else if (SB_STRCMP(name, "pri29") == 0) {
    s->Pri29 = value;
  } else if (SB_STRCMP(name, "pri28") == 0) {
    s->Pri28 = value;
  } else if (SB_STRCMP(name, "pri27") == 0) {
    s->Pri27 = value;
  } else if (SB_STRCMP(name, "pri26") == 0) {
    s->Pri26 = value;
  } else if (SB_STRCMP(name, "pri25") == 0) {
    s->Pri25 = value;
  } else if (SB_STRCMP(name, "pri24") == 0) {
    s->Pri24 = value;
  } else if (SB_STRCMP(name, "rsvd2") == 0) {
    s->Rsvd2 = value;
  } else if (SB_STRCMP(name, "pri23") == 0) {
    s->Pri23 = value;
  } else if (SB_STRCMP(name, "pri22") == 0) {
    s->Pri22 = value;
  } else if (SB_STRCMP(name, "pri21") == 0) {
    s->Pri21 = value;
  } else if (SB_STRCMP(name, "pri20") == 0) {
    s->Pri20 = value;
  } else if (SB_STRCMP(name, "pri19") == 0) {
    s->Pri19 = value;
  } else if (SB_STRCMP(name, "pri18") == 0) {
    s->Pri18 = value;
  } else if (SB_STRCMP(name, "pri17") == 0) {
    s->Pri17 = value;
  } else if (SB_STRCMP(name, "pri16") == 0) {
    s->Pri16 = value;
  } else if (SB_STRCMP(name, "rsvd1") == 0) {
    s->Rsvd1 = value;
  } else if (SB_STRCMP(name, "pri15") == 0) {
    s->Pri15 = value;
  } else if (SB_STRCMP(name, "pri14") == 0) {
    s->Pri14 = value;
  } else if (SB_STRCMP(name, "pri13") == 0) {
    s->Pri13 = value;
  } else if (SB_STRCMP(name, "pri12") == 0) {
    s->Pri12 = value;
  } else if (SB_STRCMP(name, "pri11") == 0) {
    s->Pri11 = value;
  } else if (SB_STRCMP(name, "pri10") == 0) {
    s->Pri10 = value;
  } else if (SB_STRCMP(name, "pri9") == 0) {
    s->Pri9 = value;
  } else if (SB_STRCMP(name, "pri8") == 0) {
    s->Pri8 = value;
  } else if (SB_STRCMP(name, "rsvd0") == 0) {
    s->Rsvd0 = value;
  } else if (SB_STRCMP(name, "pri7") == 0) {
    s->Pri7 = value;
  } else if (SB_STRCMP(name, "pri6") == 0) {
    s->Pri6 = value;
  } else if (SB_STRCMP(name, "pri5") == 0) {
    s->Pri5 = value;
  } else if (SB_STRCMP(name, "pri4") == 0) {
    s->Pri4 = value;
  } else if (SB_STRCMP(name, "pri3") == 0) {
    s->Pri3 = value;
  } else if (SB_STRCMP(name, "pri2") == 0) {
    s->Pri2 = value;
  } else if (SB_STRCMP(name, "pri1") == 0) {
    s->Pri1 = value;
  } else if (SB_STRCMP(name, "pri0") == 0) {
    s->Pri0 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
