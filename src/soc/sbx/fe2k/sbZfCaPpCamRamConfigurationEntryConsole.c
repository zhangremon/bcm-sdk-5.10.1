/*
 * $Id: sbZfCaPpCamRamConfigurationEntryConsole.c 1.1.48.3 Broadcom SDK $
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
#include "sbZfCaPpCamRamConfigurationEntryConsole.hx"



/* Print members in struct */
void
sbZfCaPpCamRamConfigurationEntry_Print(sbZfCaPpCamRamConfigurationEntry_t *pFromStruct) {
  SB_LOG("CaPpCamRamConfigurationEntry:: unused_spare=0x%01x", (unsigned int)  pFromStruct->m_uUnused_Spare);
  SB_LOG(" state=0x%06x", (unsigned int)  pFromStruct->m_uState);
  SB_LOG("\n");

  SB_LOG("CaPpCamRamConfigurationEntry:: hashstart=0x%01x", (unsigned int)  pFromStruct->m_uHashStart);
  SB_LOG(" hashtemplate=0x%02x", (unsigned int)  pFromStruct->m_uHashTemplate);
  SB_LOG("\n");

  SB_LOG("CaPpCamRamConfigurationEntry:: checkeroffset=0x%02x", (unsigned int)  pFromStruct->m_uCheckerOffset);
  SB_LOG(" checkertype=0x%01x", (unsigned int)  pFromStruct->m_uCheckerType);
  SB_LOG("\n");

  SB_LOG("CaPpCamRamConfigurationEntry:: headeravalid=0x%01x", (unsigned int)  pFromStruct->m_uHeaderAValid);
  SB_LOG(" headeratype=0x%01x", (unsigned int)  pFromStruct->m_uHeaderAType);
  SB_LOG("\n");

  SB_LOG("CaPpCamRamConfigurationEntry:: headerabaselength=0x%02x", (unsigned int)  pFromStruct->m_uHeaderABaseLength);
  SB_LOG(" headerbvalid=0x%01x", (unsigned int)  pFromStruct->m_uHeaderBValid);
  SB_LOG("\n");

  SB_LOG("CaPpCamRamConfigurationEntry:: headerbtype=0x%01x", (unsigned int)  pFromStruct->m_uHeaderBType);
  SB_LOG(" headerbbaselength=0x%02x", (unsigned int)  pFromStruct->m_uHeaderBBaseLength);
  SB_LOG("\n");

  SB_LOG("CaPpCamRamConfigurationEntry:: shift=0x%02x", (unsigned int)  pFromStruct->m_uShift);
  SB_LOG(" variablemask=0x%08x", (unsigned int)  pFromStruct->m_uVariableMask);
  SB_LOG("\n");

  SB_LOG("CaPpCamRamConfigurationEntry:: variabledata=0x%08x", (unsigned int)  pFromStruct->m_uVariableData);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfCaPpCamRamConfigurationEntry_SPrint(sbZfCaPpCamRamConfigurationEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaPpCamRamConfigurationEntry:: unused_spare=0x%01x", (unsigned int)  pFromStruct->m_uUnused_Spare);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," state=0x%06x", (unsigned int)  pFromStruct->m_uState);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaPpCamRamConfigurationEntry:: hashstart=0x%01x", (unsigned int)  pFromStruct->m_uHashStart);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," hashtemplate=0x%02x", (unsigned int)  pFromStruct->m_uHashTemplate);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaPpCamRamConfigurationEntry:: checkeroffset=0x%02x", (unsigned int)  pFromStruct->m_uCheckerOffset);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," checkertype=0x%01x", (unsigned int)  pFromStruct->m_uCheckerType);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaPpCamRamConfigurationEntry:: headeravalid=0x%01x", (unsigned int)  pFromStruct->m_uHeaderAValid);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," headeratype=0x%01x", (unsigned int)  pFromStruct->m_uHeaderAType);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaPpCamRamConfigurationEntry:: headerabaselength=0x%02x", (unsigned int)  pFromStruct->m_uHeaderABaseLength);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," headerbvalid=0x%01x", (unsigned int)  pFromStruct->m_uHeaderBValid);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaPpCamRamConfigurationEntry:: headerbtype=0x%01x", (unsigned int)  pFromStruct->m_uHeaderBType);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," headerbbaselength=0x%02x", (unsigned int)  pFromStruct->m_uHeaderBBaseLength);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaPpCamRamConfigurationEntry:: shift=0x%02x", (unsigned int)  pFromStruct->m_uShift);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," variablemask=0x%08x", (unsigned int)  pFromStruct->m_uVariableMask);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"CaPpCamRamConfigurationEntry:: variabledata=0x%08x", (unsigned int)  pFromStruct->m_uVariableData);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfCaPpCamRamConfigurationEntry_Validate(sbZfCaPpCamRamConfigurationEntry_t *pZf) {

  if (pZf->m_uUnused_Spare > 0x1) return 0;
  if (pZf->m_uState > 0xffffff) return 0;
  if (pZf->m_uHashStart > 0x1) return 0;
  if (pZf->m_uHashTemplate > 0x1f) return 0;
  if (pZf->m_uCheckerOffset > 0x3f) return 0;
  if (pZf->m_uCheckerType > 0xf) return 0;
  if (pZf->m_uHeaderAValid > 0x1) return 0;
  if (pZf->m_uHeaderAType > 0xf) return 0;
  if (pZf->m_uHeaderABaseLength > 0x3f) return 0;
  if (pZf->m_uHeaderBValid > 0x1) return 0;
  if (pZf->m_uHeaderBType > 0xf) return 0;
  if (pZf->m_uHeaderBBaseLength > 0x3f) return 0;
  if (pZf->m_uShift > 0x3f) return 0;
  /* pZf->m_uVariableMask implicitly masked by data type */
  /* pZf->m_uVariableData implicitly masked by data type */

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfCaPpCamRamConfigurationEntry_SetField(sbZfCaPpCamRamConfigurationEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_uunused_spare") == 0) {
    s->m_uUnused_Spare = value;
  } else if (SB_STRCMP(name, "m_ustate") == 0) {
    s->m_uState = value;
  } else if (SB_STRCMP(name, "m_uhashstart") == 0) {
    s->m_uHashStart = value;
  } else if (SB_STRCMP(name, "m_uhashtemplate") == 0) {
    s->m_uHashTemplate = value;
  } else if (SB_STRCMP(name, "m_ucheckeroffset") == 0) {
    s->m_uCheckerOffset = value;
  } else if (SB_STRCMP(name, "m_ucheckertype") == 0) {
    s->m_uCheckerType = value;
  } else if (SB_STRCMP(name, "m_uheaderavalid") == 0) {
    s->m_uHeaderAValid = value;
  } else if (SB_STRCMP(name, "m_uheaderatype") == 0) {
    s->m_uHeaderAType = value;
  } else if (SB_STRCMP(name, "m_uheaderabaselength") == 0) {
    s->m_uHeaderABaseLength = value;
  } else if (SB_STRCMP(name, "m_uheaderbvalid") == 0) {
    s->m_uHeaderBValid = value;
  } else if (SB_STRCMP(name, "m_uheaderbtype") == 0) {
    s->m_uHeaderBType = value;
  } else if (SB_STRCMP(name, "m_uheaderbbaselength") == 0) {
    s->m_uHeaderBBaseLength = value;
  } else if (SB_STRCMP(name, "m_ushift") == 0) {
    s->m_uShift = value;
  } else if (SB_STRCMP(name, "m_uvariablemask") == 0) {
    s->m_uVariableMask = value;
  } else if (SB_STRCMP(name, "m_uvariabledata") == 0) {
    s->m_uVariableData = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
