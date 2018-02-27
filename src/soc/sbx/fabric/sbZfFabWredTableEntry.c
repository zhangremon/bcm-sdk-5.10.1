/*
 * $Id: sbZfFabWredTableEntry.c 1.1.48.4 Broadcom SDK $
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
#include "sbZfFabWredParameters.hx"
#include "sbZfFabWredTableEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfFabWredTableEntry_Pack(sbZfFabWredTableEntry_t *pFrom,
                           uint8_t *pToData,
                           uint32_t nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_WRED_TABLE_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nDp0 */

  sbZfFabWredParameters_Pack(&((pFrom)->m_nDp0), (pToData)+16, nMaxToDataIndex-16);

  /* Pack Member: m_nDp1 */

  sbZfFabWredParameters_Pack(&((pFrom)->m_nDp1), (pToData)+8, nMaxToDataIndex-8);

  /* Pack Member: m_nDp2 */

  sbZfFabWredParameters_Pack(&((pFrom)->m_nDp2), (pToData)+0, nMaxToDataIndex-0);
#else
  int i;
  int size = SB_ZF_WRED_TABLE_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nDp0 */

  sbZfFabWredParameters_Pack(&((pFrom)->m_nDp0), (pToData)+16, nMaxToDataIndex-16);

  /* Pack Member: m_nDp1 */

  sbZfFabWredParameters_Pack(&((pFrom)->m_nDp1), (pToData)+8, nMaxToDataIndex-8);

  /* Pack Member: m_nDp2 */

  sbZfFabWredParameters_Pack(&((pFrom)->m_nDp2), (pToData)+0, nMaxToDataIndex-0);
#endif

  return SB_ZF_WRED_TABLE_ENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabWredTableEntry_Unpack(sbZfFabWredTableEntry_t *pToStruct,
                             uint8_t *pFromData,
                             uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nDp0 */

  sbZfFabWredParameters_Unpack(&((pToStruct)->m_nDp0), (pFromData)+16, nMaxToDataIndex-16);

  /* Unpack Member: m_nDp1 */

  sbZfFabWredParameters_Unpack(&((pToStruct)->m_nDp1), (pFromData)+8, nMaxToDataIndex-8);

  /* Unpack Member: m_nDp2 */

  sbZfFabWredParameters_Unpack(&((pToStruct)->m_nDp2), (pFromData)+0, nMaxToDataIndex-0);
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nDp0 */

  sbZfFabWredParameters_Unpack(&((pToStruct)->m_nDp0), (pFromData)+16, nMaxToDataIndex-16);

  /* Unpack Member: m_nDp1 */

  sbZfFabWredParameters_Unpack(&((pToStruct)->m_nDp1), (pFromData)+8, nMaxToDataIndex-8);

  /* Unpack Member: m_nDp2 */

  sbZfFabWredParameters_Unpack(&((pToStruct)->m_nDp2), (pFromData)+0, nMaxToDataIndex-0);
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabWredTableEntry_InitInstance(sbZfFabWredTableEntry_t *pFrame) {

  sbZfFabWredParameters_InitInstance(&(pFrame->m_nDp0));
  sbZfFabWredParameters_InitInstance(&(pFrame->m_nDp1));
  sbZfFabWredParameters_InitInstance(&(pFrame->m_nDp2));

}
#ifdef SB_ZF_INCLUDE_CONSOLE
/*
 * $Id: sbZfFabWredTableEntry.c,v 1.1.48.4 2011/05/22 05:39:16 iakramov Exp $
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
#include "sbZfFabWredParameters.hx"
#include <soc/sbx/sbWrappers.h>
#include "sbZfFabWredTableEntry.hx"



/* Print members in struct */
void
sbZfFabWredTableEntry_Print(sbZfFabWredTableEntry_t *pFromStruct) {
  SB_LOG("FabWredTableEntry:: dp0 below:\n");
  sbZfFabWredParameters_Print(&(pFromStruct->m_nDp0));
  SB_LOG("FabWredTableEntry:: dp1 below:\n");
  sbZfFabWredParameters_Print(&(pFromStruct->m_nDp1));
  SB_LOG("FabWredTableEntry:: dp2 below:\n");
  sbZfFabWredParameters_Print(&(pFromStruct->m_nDp2));
  SB_LOG("\n");

}

/* SPrint members in struct */
int
sbZfFabWredTableEntry_SPrint(sbZfFabWredTableEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += sbZfFabWredParameters_SPrint(&(pFromStruct->m_nDp0), &pcToString[WrCnt], lStrSize-WrCnt);
  WrCnt += sbZfFabWredParameters_SPrint(&(pFromStruct->m_nDp1), &pcToString[WrCnt], lStrSize-WrCnt);
  WrCnt += sbZfFabWredParameters_SPrint(&(pFromStruct->m_nDp2), &pcToString[WrCnt], lStrSize-WrCnt);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(WrCnt);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabWredTableEntry_Validate(sbZfFabWredTableEntry_t *pZf) {

  if (!sbZfFabWredParameters_Validate(&pZf->m_nDp0)) return 0;
  if (!sbZfFabWredParameters_Validate(&pZf->m_nDp1)) return 0;
  if (!sbZfFabWredParameters_Validate(&pZf->m_nDp2)) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabWredTableEntry_SetField(sbZfFabWredTableEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRSTR(name, "m_ndp0.") == name) {
    sbZfFabWredParameters_SetField(&(s->m_nDp0), name+7, value);
  } else if (SB_STRSTR(name, "m_ndp1.") == name) {
    sbZfFabWredParameters_SetField(&(s->m_nDp1), name+7, value);
  } else if (SB_STRSTR(name, "m_ndp2.") == name) {
    sbZfFabWredParameters_SetField(&(s->m_nDp2), name+7, value);
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}


#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */
