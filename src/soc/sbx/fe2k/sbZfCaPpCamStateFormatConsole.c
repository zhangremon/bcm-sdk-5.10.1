/*
 * $Id: sbZfCaPpCamStateFormatConsole.c 1.1.48.3 Broadcom SDK $
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
#include "sbZfCaPpCamStateFormatConsole.hx"



/* Print members in struct */
void
sbZfCaPpCamStateFormat_Print(sbZfCaPpCamStateFormat_t *pFromStruct) {
  SB_LOG("CaPpCamStateFormat:: resv1=0x%02x", (unsigned int)  pFromStruct->m_uResv1);
  SB_LOG(" ingr=0x%01x", (unsigned int)  pFromStruct->m_bIngress);
  SB_LOG(" egr=0x%01x", (unsigned int)  pFromStruct->m_bEgress);
  SB_LOG(" cycle0=0x%01x", (unsigned int)  pFromStruct->m_bCycle0);
  SB_LOG(" cycle1=0x%01x", (unsigned int)  pFromStruct->m_bCycle1);
  SB_LOG("\n");

  SB_LOG("CaPpCamStateFormat:: resv0=0x%01x", (unsigned int)  pFromStruct->m_uResv0);
  SB_LOG(" state=0x%02x", (unsigned int)  pFromStruct->m_uState);
  SB_LOG(" pbits4=0x%01x", (unsigned int)  pFromStruct->m_uPropBits4);
  SB_LOG(" pbits3=0x%01x", (unsigned int)  pFromStruct->m_uPropBits3);
  SB_LOG(" pbits2=0x%01x", (unsigned int)  pFromStruct->m_uPropBits2);
  SB_LOG("\n");

  SB_LOG("CaPpCamStateFormat:: pbits1=0x%01x", (unsigned int)  pFromStruct->m_uPropBits1);
  SB_LOG(" pbits0=0x%01x", (unsigned int)  pFromStruct->m_uPropBits0);
  SB_LOG(" ssl=0x%01x", (unsigned int)  pFromStruct->m_uStreamState);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfCaPpCamStateFormat_SPrint(sbZfCaPpCamStateFormat_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaPpCamStateFormat:: resv1=0x%02x", (unsigned int)  pFromStruct->m_uResv1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," ingr=0x%01x", (unsigned int)  pFromStruct->m_bIngress);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," egr=0x%01x", (unsigned int)  pFromStruct->m_bEgress);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," cycle0=0x%01x", (unsigned int)  pFromStruct->m_bCycle0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," cycle1=0x%01x", (unsigned int)  pFromStruct->m_bCycle1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaPpCamStateFormat:: resv0=0x%01x", (unsigned int)  pFromStruct->m_uResv0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," state=0x%02x", (unsigned int)  pFromStruct->m_uState);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pbits4=0x%01x", (unsigned int)  pFromStruct->m_uPropBits4);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pbits3=0x%01x", (unsigned int)  pFromStruct->m_uPropBits3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pbits2=0x%01x", (unsigned int)  pFromStruct->m_uPropBits2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaPpCamStateFormat:: pbits1=0x%01x", (unsigned int)  pFromStruct->m_uPropBits1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pbits0=0x%01x", (unsigned int)  pFromStruct->m_uPropBits0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," ssl=0x%01x", (unsigned int)  pFromStruct->m_uStreamState);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfCaPpCamStateFormat_Validate(sbZfCaPpCamStateFormat_t *pZf) {

  if (pZf->m_uResv1 > 0xff) return 0;
  if (pZf->m_bIngress > 0x1) return 0;
  if (pZf->m_bEgress > 0x1) return 0;
  if (pZf->m_bCycle0 > 0x1) return 0;
  if (pZf->m_bCycle1 > 0x1) return 0;
  if (pZf->m_uResv0 > 0x7) return 0;
  if (pZf->m_uState > 0xff) return 0;
  if (pZf->m_uPropBits4 > 0x1) return 0;
  if (pZf->m_uPropBits3 > 0x1) return 0;
  if (pZf->m_uPropBits2 > 0x1) return 0;
  if (pZf->m_uPropBits1 > 0x1) return 0;
  if (pZf->m_uPropBits0 > 0x1) return 0;
  if (pZf->m_uStreamState > 0x7) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfCaPpCamStateFormat_SetField(sbZfCaPpCamStateFormat_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_uresv1") == 0) {
    s->m_uResv1 = value;
  } else if (SB_STRCMP(name, "ingress") == 0) {
    s->m_bIngress = value;
  } else if (SB_STRCMP(name, "egress") == 0) {
    s->m_bEgress = value;
  } else if (SB_STRCMP(name, "cycle0") == 0) {
    s->m_bCycle0 = value;
  } else if (SB_STRCMP(name, "cycle1") == 0) {
    s->m_bCycle1 = value;
  } else if (SB_STRCMP(name, "m_uresv0") == 0) {
    s->m_uResv0 = value;
  } else if (SB_STRCMP(name, "m_ustate") == 0) {
    s->m_uState = value;
  } else if (SB_STRCMP(name, "m_upropbits4") == 0) {
    s->m_uPropBits4 = value;
  } else if (SB_STRCMP(name, "m_upropbits3") == 0) {
    s->m_uPropBits3 = value;
  } else if (SB_STRCMP(name, "m_upropbits2") == 0) {
    s->m_uPropBits2 = value;
  } else if (SB_STRCMP(name, "m_upropbits1") == 0) {
    s->m_uPropBits1 = value;
  } else if (SB_STRCMP(name, "m_upropbits0") == 0) {
    s->m_uPropBits0 = value;
  } else if (SB_STRCMP(name, "m_ustreamstate") == 0) {
    s->m_uStreamState = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
