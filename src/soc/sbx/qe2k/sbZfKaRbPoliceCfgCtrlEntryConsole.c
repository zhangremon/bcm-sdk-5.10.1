/*
 * $Id: sbZfKaRbPoliceCfgCtrlEntryConsole.c 1.1.44.3 Broadcom SDK $
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
#include "sbZfKaRbPoliceCfgCtrlEntryConsole.hx"



/* Print members in struct */
void
sbZfKaRbPoliceCfgCtrlEntry_Print(sbZfKaRbPoliceCfgCtrlEntry_t *pFromStruct) {
  SB_LOG("KaRbPoliceCfgCtrlEntry:: res=0x%02x", (unsigned int)  pFromStruct->m_nReserved);
  SB_LOG(" enable=0x%01x", (unsigned int)  pFromStruct->m_nEnable);
  SB_LOG(" notblind=0x%01x", (unsigned int)  pFromStruct->m_nNotBlind);
  SB_LOG(" droponred=0x%01x", (unsigned int)  pFromStruct->m_nDropOnRed);
  SB_LOG("\n");

  SB_LOG("KaRbPoliceCfgCtrlEntry:: enablemon=0x%01x", (unsigned int)  pFromStruct->m_nEnableMon);
  SB_LOG(" moncntid=0x%01x", (unsigned int)  pFromStruct->m_nMonCntId);
  SB_LOG(" incrate=0x%05x", (unsigned int)  pFromStruct->m_nIncRate);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfKaRbPoliceCfgCtrlEntry_SPrint(sbZfKaRbPoliceCfgCtrlEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbPoliceCfgCtrlEntry:: res=0x%02x", (unsigned int)  pFromStruct->m_nReserved);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," enable=0x%01x", (unsigned int)  pFromStruct->m_nEnable);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," notblind=0x%01x", (unsigned int)  pFromStruct->m_nNotBlind);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," droponred=0x%01x", (unsigned int)  pFromStruct->m_nDropOnRed);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbPoliceCfgCtrlEntry:: enablemon=0x%01x", (unsigned int)  pFromStruct->m_nEnableMon);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," moncntid=0x%01x", (unsigned int)  pFromStruct->m_nMonCntId);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," incrate=0x%05x", (unsigned int)  pFromStruct->m_nIncRate);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaRbPoliceCfgCtrlEntry_Validate(sbZfKaRbPoliceCfgCtrlEntry_t *pZf) {

  if (pZf->m_nReserved > 0x1f) return 0;
  if (pZf->m_nEnable > 0x1) return 0;
  if (pZf->m_nNotBlind > 0x1) return 0;
  if (pZf->m_nDropOnRed > 0x1) return 0;
  if (pZf->m_nEnableMon > 0x1) return 0;
  if (pZf->m_nMonCntId > 0x7) return 0;
  if (pZf->m_nIncRate > 0xfffff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaRbPoliceCfgCtrlEntry_SetField(sbZfKaRbPoliceCfgCtrlEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nreserved") == 0) {
    s->m_nReserved = value;
  } else if (SB_STRCMP(name, "m_nenable") == 0) {
    s->m_nEnable = value;
  } else if (SB_STRCMP(name, "m_nnotblind") == 0) {
    s->m_nNotBlind = value;
  } else if (SB_STRCMP(name, "m_ndroponred") == 0) {
    s->m_nDropOnRed = value;
  } else if (SB_STRCMP(name, "m_nenablemon") == 0) {
    s->m_nEnableMon = value;
  } else if (SB_STRCMP(name, "m_nmoncntid") == 0) {
    s->m_nMonCntId = value;
  } else if (SB_STRCMP(name, "m_nincrate") == 0) {
    s->m_nIncRate = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
