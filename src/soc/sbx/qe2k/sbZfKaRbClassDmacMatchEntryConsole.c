/*
 * $Id: sbZfKaRbClassDmacMatchEntryConsole.c 1.1.44.3 Broadcom SDK $
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
#include "sbZfKaRbClassDmacMatchEntryConsole.hx"



/* Print members in struct */
void
sbZfKaRbClassDmacMatchEntry_Print(sbZfKaRbClassDmacMatchEntry_t *pFromStruct) {
  SB_LOG("KaRbClassDmacMatchEntry:: dmacdatalsb=0x%08x", (unsigned int)  pFromStruct->m_nDmacDataLsb);
  SB_LOG(" dmacdatarsv=0x%04x", (unsigned int)  pFromStruct->m_nDmacDataRsv);
  SB_LOG("\n");

  SB_LOG("KaRbClassDmacMatchEntry:: dmacdatamsb=0x%04x", (unsigned int)  pFromStruct->m_nDmacDataMsb);
  SB_LOG(" dmacmasklsb=0x%08x", (unsigned int)  pFromStruct->m_nDmacMaskLsb);
  SB_LOG("\n");

  SB_LOG("KaRbClassDmacMatchEntry:: dmacmaskrsv=0x%04x", (unsigned int)  pFromStruct->m_nDmacMaskRsv);
  SB_LOG(" dmacmaskmsb=0x%04x", (unsigned int)  pFromStruct->m_nDmacMaskMsb);
  SB_LOG("\n");

  SB_LOG("KaRbClassDmacMatchEntry:: dmacreserve=0x%07x", (unsigned int)  pFromStruct->m_nDmacReserve);
  SB_LOG(" dmacenb=0x%01x", (unsigned int)  pFromStruct->m_nDmacEnable);
  SB_LOG(" dmacdp=0x%01x", (unsigned int)  pFromStruct->m_nDmacDp);
  SB_LOG("\n");

  SB_LOG("KaRbClassDmacMatchEntry:: dmaclsb=0x%01x", (unsigned int)  pFromStruct->m_nDmacLsb);
  SB_LOG("\n");

}

/* SPrint members in struct */
char *
sbZfKaRbClassDmacMatchEntry_SPrint(sbZfKaRbClassDmacMatchEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassDmacMatchEntry:: dmacdatalsb=0x%08x", (unsigned int)  pFromStruct->m_nDmacDataLsb);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," dmacdatarsv=0x%04x", (unsigned int)  pFromStruct->m_nDmacDataRsv);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassDmacMatchEntry:: dmacdatamsb=0x%04x", (unsigned int)  pFromStruct->m_nDmacDataMsb);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," dmacmasklsb=0x%08x", (unsigned int)  pFromStruct->m_nDmacMaskLsb);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassDmacMatchEntry:: dmacmaskrsv=0x%04x", (unsigned int)  pFromStruct->m_nDmacMaskRsv);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," dmacmaskmsb=0x%04x", (unsigned int)  pFromStruct->m_nDmacMaskMsb);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassDmacMatchEntry:: dmacreserve=0x%07x", (unsigned int)  pFromStruct->m_nDmacReserve);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," dmacenb=0x%01x", (unsigned int)  pFromStruct->m_nDmacEnable);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," dmacdp=0x%01x", (unsigned int)  pFromStruct->m_nDmacDp);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"KaRbClassDmacMatchEntry:: dmaclsb=0x%01x", (unsigned int)  pFromStruct->m_nDmacLsb);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(pcToString);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfKaRbClassDmacMatchEntry_Validate(sbZfKaRbClassDmacMatchEntry_t *pZf) {

  /* pZf->m_nDmacDataLsb implicitly masked by data type */
  if (pZf->m_nDmacDataRsv > 0xffff) return 0;
  if (pZf->m_nDmacDataMsb > 0xffff) return 0;
  /* pZf->m_nDmacMaskLsb implicitly masked by data type */
  if (pZf->m_nDmacMaskRsv > 0xffff) return 0;
  if (pZf->m_nDmacMaskMsb > 0xffff) return 0;
  if (pZf->m_nDmacReserve > 0x1ffffff) return 0;
  if (pZf->m_nDmacEnable > 0x1) return 0;
  if (pZf->m_nDmacDp > 0x3) return 0;
  if (pZf->m_nDmacLsb > 0xf) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfKaRbClassDmacMatchEntry_SetField(sbZfKaRbClassDmacMatchEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_ndmacdatalsb") == 0) {
    s->m_nDmacDataLsb = value;
  } else if (SB_STRCMP(name, "m_ndmacdatarsv") == 0) {
    s->m_nDmacDataRsv = value;
  } else if (SB_STRCMP(name, "m_ndmacdatamsb") == 0) {
    s->m_nDmacDataMsb = value;
  } else if (SB_STRCMP(name, "m_ndmacmasklsb") == 0) {
    s->m_nDmacMaskLsb = value;
  } else if (SB_STRCMP(name, "m_ndmacmaskrsv") == 0) {
    s->m_nDmacMaskRsv = value;
  } else if (SB_STRCMP(name, "m_ndmacmaskmsb") == 0) {
    s->m_nDmacMaskMsb = value;
  } else if (SB_STRCMP(name, "m_ndmacreserve") == 0) {
    s->m_nDmacReserve = value;
  } else if (SB_STRCMP(name, "m_ndmacenable") == 0) {
    s->m_nDmacEnable = value;
  } else if (SB_STRCMP(name, "m_ndmacdp") == 0) {
    s->m_nDmacDp = value;
  } else if (SB_STRCMP(name, "m_ndmaclsb") == 0) {
    s->m_nDmacLsb = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}
