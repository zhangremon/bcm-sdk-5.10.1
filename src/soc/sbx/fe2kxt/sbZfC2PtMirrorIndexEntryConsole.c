/*
 * $Id: sbZfC2PtMirrorIndexEntryConsole.c 1.1.34.3 Broadcom SDK $
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
#include "sbZfC2PtMirrorIndexEntryConsole.hx"



/* Print members in struct */
void
sbZfC2PtMirrorIndexEntry_Print(sbZfC2PtMirrorIndexEntry_t *pFromStruct) {
  SB_LOG("C2PtMirrorIndexEntry:: overwritehdr=0x%01x", (unsigned int)  pFromStruct->m_uOverwriteHdr);
  SB_LOG(" higighdr=0x%01x", (unsigned int)  pFromStruct->m_uHiGigHdr);
  SB_LOG(" dropsrcpkt=0x%01x", (unsigned int)  pFromStruct->m_uDropSrcPkt);
  SB_LOG("\n");

  SB_LOG("C2PtMirrorIndexEntry:: hdrpresent=0x%01x", (unsigned int)  pFromStruct->m_uHdrPresent);
  SB_LOG(" mirrorqueue=0x%02x", (unsigned int)  pFromStruct->m_uMirrorQueue);
  SB_LOG("\n");

  SB_LOG("C2PtMirrorIndexEntry:: headerdata3=0x%08x", (unsigned int)  pFromStruct->m_uHeaderData3);
  SB_LOG(" headerdata2=0x%08x", (unsigned int)  pFromStruct->m_uHeaderData2);
  SB_LOG("\n");

  SB_LOG("C2PtMirrorIndexEntry:: headerdata1=0x%08x", (unsigned int)  pFromStruct->m_uHeaderData1);
  SB_LOG(" headerdata0=0x%08x", (unsigned int)  pFromStruct->m_uHeaderData0);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfC2PtMirrorIndexEntry_SPrint(sbZfC2PtMirrorIndexEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PtMirrorIndexEntry:: overwritehdr=0x%01x", (unsigned int)  pFromStruct->m_uOverwriteHdr);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," higighdr=0x%01x", (unsigned int)  pFromStruct->m_uHiGigHdr);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," dropsrcpkt=0x%01x", (unsigned int)  pFromStruct->m_uDropSrcPkt);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PtMirrorIndexEntry:: hdrpresent=0x%01x", (unsigned int)  pFromStruct->m_uHdrPresent);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," mirrorqueue=0x%02x", (unsigned int)  pFromStruct->m_uMirrorQueue);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PtMirrorIndexEntry:: headerdata3=0x%08x", (unsigned int)  pFromStruct->m_uHeaderData3);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," headerdata2=0x%08x", (unsigned int)  pFromStruct->m_uHeaderData2);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"C2PtMirrorIndexEntry:: headerdata1=0x%08x", (unsigned int)  pFromStruct->m_uHeaderData1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," headerdata0=0x%08x", (unsigned int)  pFromStruct->m_uHeaderData0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfC2PtMirrorIndexEntry_Validate(sbZfC2PtMirrorIndexEntry_t *pZf) {

  if (pZf->m_uOverwriteHdr > 0x1) return 0;
  if (pZf->m_uHiGigHdr > 0x1) return 0;
  if (pZf->m_uDropSrcPkt > 0x1) return 0;
  if (pZf->m_uHdrPresent > 0x1) return 0;
  if (pZf->m_uMirrorQueue > 0xff) return 0;
  /* pZf->m_uHeaderData3 implicitly masked by data type */
  /* pZf->m_uHeaderData2 implicitly masked by data type */
  /* pZf->m_uHeaderData1 implicitly masked by data type */
  /* pZf->m_uHeaderData0 implicitly masked by data type */

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfC2PtMirrorIndexEntry_SetField(sbZfC2PtMirrorIndexEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_uoverwritehdr") == 0) {
    s->m_uOverwriteHdr = value;
  } else if (SB_STRCMP(name, "m_uhigighdr") == 0) {
    s->m_uHiGigHdr = value;
  } else if (SB_STRCMP(name, "m_udropsrcpkt") == 0) {
    s->m_uDropSrcPkt = value;
  } else if (SB_STRCMP(name, "m_uhdrpresent") == 0) {
    s->m_uHdrPresent = value;
  } else if (SB_STRCMP(name, "m_umirrorqueue") == 0) {
    s->m_uMirrorQueue = value;
  } else if (SB_STRCMP(name, "m_uheaderdata3") == 0) {
    s->m_uHeaderData3 = value;
  } else if (SB_STRCMP(name, "m_uheaderdata2") == 0) {
    s->m_uHeaderData2 = value;
  } else if (SB_STRCMP(name, "m_uheaderdata1") == 0) {
    s->m_uHeaderData1 = value;
  } else if (SB_STRCMP(name, "m_uheaderdata0") == 0) {
    s->m_uHeaderData0 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
