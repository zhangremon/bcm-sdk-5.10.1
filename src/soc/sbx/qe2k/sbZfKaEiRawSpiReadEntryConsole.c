/*
 * $Id: sbZfKaEiRawSpiReadEntryConsole.c 1.1.44.3 Broadcom SDK $
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
#include "sbZfKaEiRawSpiReadEntryConsole.hx"



/* Print members in struct */
void
sbZfKaEiRawSpiReadEntry_Print(sbZfKaEiRawSpiReadEntry_t *pFromStruct) {
  SB_LOG("KaEiRawSpiReadEntry:: destchannel=0x%02x", (unsigned int)  pFromStruct->m_nDestChannel);
  SB_LOG(" sizemask8=0x%01x", (unsigned int)  pFromStruct->m_nSizeMask8);
  SB_LOG(" sizemask7_0=0x%02x", (unsigned int)  pFromStruct->m_nSizeMask7_0);
  SB_LOG("\n");

  SB_LOG("KaEiRawSpiReadEntry:: rb_loopback_only=0x%01x", (unsigned int)  pFromStruct->m_nRbLoopbackOnly);
  SB_LOG(" line_ptr=0x%03x", (unsigned int)  pFromStruct->m_nLinePtr);
  SB_LOG(" byte_ptr=0x%01x", (unsigned int)  pFromStruct->m_nBytePtr);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfKaEiRawSpiReadEntry_SPrint(sbZfKaEiRawSpiReadEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEiRawSpiReadEntry:: destchannel=0x%02x", (unsigned int)  pFromStruct->m_nDestChannel);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sizemask8=0x%01x", (unsigned int)  pFromStruct->m_nSizeMask8);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sizemask7_0=0x%02x", (unsigned int)  pFromStruct->m_nSizeMask7_0);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaEiRawSpiReadEntry:: rb_loopback_only=0x%01x", (unsigned int)  pFromStruct->m_nRbLoopbackOnly);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," line_ptr=0x%03x", (unsigned int)  pFromStruct->m_nLinePtr);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," byte_ptr=0x%01x", (unsigned int)  pFromStruct->m_nBytePtr);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaEiRawSpiReadEntry_Validate(sbZfKaEiRawSpiReadEntry_t *pZf) {

  if (pZf->m_nDestChannel > 0xff) return 0;
  if (pZf->m_nSizeMask8 > 0x1) return 0;
  if (pZf->m_nSizeMask7_0 > 0xff) return 0;
  if (pZf->m_nRbLoopbackOnly > 0x1) return 0;
  if (pZf->m_nLinePtr > 0x3ff) return 0;
  if (pZf->m_nBytePtr > 0xf) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaEiRawSpiReadEntry_SetField(sbZfKaEiRawSpiReadEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_ndestchannel") == 0) {
    s->m_nDestChannel = value;
  } else if (SB_STRCMP(name, "m_nsizemask8") == 0) {
    s->m_nSizeMask8 = value;
  } else if (SB_STRCMP(name, "m_nsizemask7_0") == 0) {
    s->m_nSizeMask7_0 = value;
  } else if (SB_STRCMP(name, "m_nrbloopbackonly") == 0) {
    s->m_nRbLoopbackOnly = value;
  } else if (SB_STRCMP(name, "m_nlineptr") == 0) {
    s->m_nLinePtr = value;
  } else if (SB_STRCMP(name, "m_nbyteptr") == 0) {
    s->m_nBytePtr = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
