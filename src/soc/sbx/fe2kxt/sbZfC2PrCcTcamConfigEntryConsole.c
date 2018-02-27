/*
 * $Id: sbZfC2PrCcTcamConfigEntryConsole.c 1.1.34.3 Broadcom SDK $
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
#include "sbZfC2PrCcTcamConfigEntryConsole.hx"



/* Print members in struct */
void
sbZfC2PrCcTcamConfigEntry_Print(sbZfC2PrCcTcamConfigEntry_t *pFromStruct) {
  SB_LOG("C2PrCcTcamConfigEntry:: mask213_192=0x%06x", (unsigned int)  pFromStruct->m_uMask213_192);
  SB_LOG(" mask191_160=0x%08x", (unsigned int)  pFromStruct->m_uMask191_160);
  SB_LOG("\n");

  SB_LOG("C2PrCcTcamConfigEntry:: mask159_128=0x%08x", (unsigned int)  pFromStruct->m_uMask159_128);
  SB_LOG(" mask127_96=0x%08x", (unsigned int)  pFromStruct->m_uMask127_96);
  SB_LOG("\n");

  SB_LOG("C2PrCcTcamConfigEntry:: mask95_64=0x%08x", (unsigned int)  pFromStruct->m_uMask95_64);
  SB_LOG(" mask63_32=0x%08x", (unsigned int)  pFromStruct->m_uMask63_32);
  SB_LOG("\n");

  SB_LOG("C2PrCcTcamConfigEntry:: mask31_0=0x%08x", (unsigned int)  pFromStruct->m_uMask31_0);
  SB_LOG(" data213_192=0x%06x", (unsigned int)  pFromStruct->m_uData213_192);
  SB_LOG("\n");

  SB_LOG("C2PrCcTcamConfigEntry:: data191_160=0x%08x", (unsigned int)  pFromStruct->m_uData191_160);
  SB_LOG(" data159_128=0x%08x", (unsigned int)  pFromStruct->m_uData159_128);
  SB_LOG("\n");

  SB_LOG("C2PrCcTcamConfigEntry:: data127_96=0x%08x", (unsigned int)  pFromStruct->m_uData127_96);
  SB_LOG(" data95_64=0x%08x", (unsigned int)  pFromStruct->m_uData95_64);
  SB_LOG("\n");

  SB_LOG("C2PrCcTcamConfigEntry:: data63_32=0x%08x", (unsigned int)  pFromStruct->m_uData63_32);
  SB_LOG(" data31_0=0x%08x", (unsigned int)  pFromStruct->m_uData31_0);
  SB_LOG(" valid=0x%01x", (unsigned int)  pFromStruct->m_uValid);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfC2PrCcTcamConfigEntry_SPrint(sbZfC2PrCcTcamConfigEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PrCcTcamConfigEntry:: mask213_192=0x%06x", (unsigned int)  pFromStruct->m_uMask213_192);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," mask191_160=0x%08x", (unsigned int)  pFromStruct->m_uMask191_160);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PrCcTcamConfigEntry:: mask159_128=0x%08x", (unsigned int)  pFromStruct->m_uMask159_128);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," mask127_96=0x%08x", (unsigned int)  pFromStruct->m_uMask127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PrCcTcamConfigEntry:: mask95_64=0x%08x", (unsigned int)  pFromStruct->m_uMask95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," mask63_32=0x%08x", (unsigned int)  pFromStruct->m_uMask63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PrCcTcamConfigEntry:: mask31_0=0x%08x", (unsigned int)  pFromStruct->m_uMask31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data213_192=0x%06x", (unsigned int)  pFromStruct->m_uData213_192);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PrCcTcamConfigEntry:: data191_160=0x%08x", (unsigned int)  pFromStruct->m_uData191_160);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data159_128=0x%08x", (unsigned int)  pFromStruct->m_uData159_128);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PrCcTcamConfigEntry:: data127_96=0x%08x", (unsigned int)  pFromStruct->m_uData127_96);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data95_64=0x%08x", (unsigned int)  pFromStruct->m_uData95_64);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PrCcTcamConfigEntry:: data63_32=0x%08x", (unsigned int)  pFromStruct->m_uData63_32);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," data31_0=0x%08x", (unsigned int)  pFromStruct->m_uData31_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," valid=0x%01x", (unsigned int)  pFromStruct->m_uValid);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfC2PrCcTcamConfigEntry_Validate(sbZfC2PrCcTcamConfigEntry_t *pZf) {

  if (pZf->m_uMask213_192 > 0x3fffff) return 0;
  /* pZf->m_uMask191_160 implicitly masked by data type */
  /* pZf->m_uMask159_128 implicitly masked by data type */
  /* pZf->m_uMask127_96 implicitly masked by data type */
  /* pZf->m_uMask95_64 implicitly masked by data type */
  /* pZf->m_uMask63_32 implicitly masked by data type */
  /* pZf->m_uMask31_0 implicitly masked by data type */
  if (pZf->m_uData213_192 > 0x3fffff) return 0;
  /* pZf->m_uData191_160 implicitly masked by data type */
  /* pZf->m_uData159_128 implicitly masked by data type */
  /* pZf->m_uData127_96 implicitly masked by data type */
  /* pZf->m_uData95_64 implicitly masked by data type */
  /* pZf->m_uData63_32 implicitly masked by data type */
  /* pZf->m_uData31_0 implicitly masked by data type */
  if (pZf->m_uValid > 0x3) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfC2PrCcTcamConfigEntry_SetField(sbZfC2PrCcTcamConfigEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_umask213_192") == 0) {
    s->m_uMask213_192 = value;
  } else if (SB_STRCMP(name, "m_umask191_160") == 0) {
    s->m_uMask191_160 = value;
  } else if (SB_STRCMP(name, "m_umask159_128") == 0) {
    s->m_uMask159_128 = value;
  } else if (SB_STRCMP(name, "m_umask127_96") == 0) {
    s->m_uMask127_96 = value;
  } else if (SB_STRCMP(name, "m_umask95_64") == 0) {
    s->m_uMask95_64 = value;
  } else if (SB_STRCMP(name, "m_umask63_32") == 0) {
    s->m_uMask63_32 = value;
  } else if (SB_STRCMP(name, "m_umask31_0") == 0) {
    s->m_uMask31_0 = value;
  } else if (SB_STRCMP(name, "m_udata213_192") == 0) {
    s->m_uData213_192 = value;
  } else if (SB_STRCMP(name, "m_udata191_160") == 0) {
    s->m_uData191_160 = value;
  } else if (SB_STRCMP(name, "m_udata159_128") == 0) {
    s->m_uData159_128 = value;
  } else if (SB_STRCMP(name, "m_udata127_96") == 0) {
    s->m_uData127_96 = value;
  } else if (SB_STRCMP(name, "m_udata95_64") == 0) {
    s->m_uData95_64 = value;
  } else if (SB_STRCMP(name, "m_udata63_32") == 0) {
    s->m_uData63_32 = value;
  } else if (SB_STRCMP(name, "m_udata31_0") == 0) {
    s->m_uData31_0 = value;
  } else if (SB_STRCMP(name, "m_uvalid") == 0) {
    s->m_uValid = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
