/*
 * $Id: sbZfFabBm9600InaNmPriorityUpdateConsole.c 1.1.38.3 Broadcom SDK $
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
#include "sbZfFabBm9600InaNmPriorityUpdateConsole.hx"



/* Print members in struct */
void
sbZfFabBm9600InaNmPriorityUpdate_Print(sbZfFabBm9600InaNmPriorityUpdate_t *pFromStruct) {
  SB_LOG("FabBm9600InaNmPriorityUpdate:: ina=0x%02x", (unsigned int)  pFromStruct->m_uIna);
  SB_LOG(" sysport=0x%03x", (unsigned int)  pFromStruct->m_uSystemPort);
  SB_LOG(" eset=0x%03x", (unsigned int)  pFromStruct->m_uEset);
  SB_LOG("\n");

  SB_LOG("FabBm9600InaNmPriorityUpdate:: portaddr=0x%02x", (unsigned int)  pFromStruct->m_uPortSetAddress);
  SB_LOG(" portoffs=0x%01x", (unsigned int)  pFromStruct->m_uPortSetOffset);
  SB_LOG(" ncu=0x%01x", (unsigned int)  pFromStruct->m_bNoCriticalUpdate);
  SB_LOG(" cu=0x%01x", (unsigned int)  pFromStruct->m_bCriticalUpdate);
  SB_LOG("\n");

  SB_LOG("FabBm9600InaNmPriorityUpdate:: mc=0x%01x", (unsigned int)  pFromStruct->m_bMulticast);
  SB_LOG(" pri=0x%01x", (unsigned int)  pFromStruct->m_uPriority);
  SB_LOG(" maxpri=0x%01x", (unsigned int)  pFromStruct->m_bMaxPriority);
  SB_LOG(" nxtpri=0x%01x", (unsigned int)  pFromStruct->m_uNextPriority);
  SB_LOG("\n");

  SB_LOG("FabBm9600InaNmPriorityUpdate:: nxtmax=0x%01x", (unsigned int)  pFromStruct->m_bNextMaxPriority);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfFabBm9600InaNmPriorityUpdate_SPrint(sbZfFabBm9600InaNmPriorityUpdate_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600InaNmPriorityUpdate:: ina=0x%02x", (unsigned int)  pFromStruct->m_uIna);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sysport=0x%03x", (unsigned int)  pFromStruct->m_uSystemPort);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," eset=0x%03x", (unsigned int)  pFromStruct->m_uEset);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600InaNmPriorityUpdate:: portaddr=0x%02x", (unsigned int)  pFromStruct->m_uPortSetAddress);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," portoffs=0x%01x", (unsigned int)  pFromStruct->m_uPortSetOffset);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," ncu=0x%01x", (unsigned int)  pFromStruct->m_bNoCriticalUpdate);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," cu=0x%01x", (unsigned int)  pFromStruct->m_bCriticalUpdate);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600InaNmPriorityUpdate:: mc=0x%01x", (unsigned int)  pFromStruct->m_bMulticast);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," pri=0x%01x", (unsigned int)  pFromStruct->m_uPriority);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," maxpri=0x%01x", (unsigned int)  pFromStruct->m_bMaxPriority);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," nxtpri=0x%01x", (unsigned int)  pFromStruct->m_uNextPriority);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm9600InaNmPriorityUpdate:: nxtmax=0x%01x", (unsigned int)  pFromStruct->m_bNextMaxPriority);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm9600InaNmPriorityUpdate_Validate(sbZfFabBm9600InaNmPriorityUpdate_t *pZf) {

  if (pZf->m_uIna > 0x7f) return 0;
  if (pZf->m_uSystemPort > 0xfff) return 0;
  if (pZf->m_uEset > 0x3ff) return 0;
  if (pZf->m_uPortSetAddress > 0xff) return 0;
  if (pZf->m_uPortSetOffset > 0xf) return 0;
  if (pZf->m_bNoCriticalUpdate > 0x1) return 0;
  if (pZf->m_bCriticalUpdate > 0x1) return 0;
  if (pZf->m_bMulticast > 0x1) return 0;
  if (pZf->m_uPriority > 0xf) return 0;
  if (pZf->m_bMaxPriority > 0x1) return 0;
  if (pZf->m_uNextPriority > 0xf) return 0;
  if (pZf->m_bNextMaxPriority > 0x1) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm9600InaNmPriorityUpdate_SetField(sbZfFabBm9600InaNmPriorityUpdate_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_uina") == 0) {
    s->m_uIna = value;
  } else if (SB_STRCMP(name, "m_usystemport") == 0) {
    s->m_uSystemPort = value;
  } else if (SB_STRCMP(name, "m_ueset") == 0) {
    s->m_uEset = value;
  } else if (SB_STRCMP(name, "m_uportsetaddress") == 0) {
    s->m_uPortSetAddress = value;
  } else if (SB_STRCMP(name, "m_uportsetoffset") == 0) {
    s->m_uPortSetOffset = value;
  } else if (SB_STRCMP(name, "nocriticalupdate") == 0) {
    s->m_bNoCriticalUpdate = value;
  } else if (SB_STRCMP(name, "criticalupdate") == 0) {
    s->m_bCriticalUpdate = value;
  } else if (SB_STRCMP(name, "multicast") == 0) {
    s->m_bMulticast = value;
  } else if (SB_STRCMP(name, "m_upriority") == 0) {
    s->m_uPriority = value;
  } else if (SB_STRCMP(name, "maxpriority") == 0) {
    s->m_bMaxPriority = value;
  } else if (SB_STRCMP(name, "m_unextpriority") == 0) {
    s->m_uNextPriority = value;
  } else if (SB_STRCMP(name, "nextmaxpriority") == 0) {
    s->m_bNextMaxPriority = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
