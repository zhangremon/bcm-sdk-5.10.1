/*
 * $Id: sbZfFabBm3200WredDataTableEntry.c 1.1.48.4 Broadcom SDK $
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
#include "sbZfFabBm3200WredDataTableEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfFabBm3200WredDataTableEntry_Pack(sbZfFabBm3200WredDataTableEntry_t *pFrom,
                                     uint8_t *pToData,
                                     uint32_t nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM3200_WRED_DATA_TABLE_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nTemplateOdd */
  (pToData)[0] |= ((pFrom)->m_nTemplateOdd) & 0xFF;

  /* Pack Member: m_nReservedOdd */
  (pToData)[1] |= ((pFrom)->m_nReservedOdd & 0x0f) <<4;

  /* Pack Member: m_nGainOdd */
  (pToData)[1] |= ((pFrom)->m_nGainOdd & 0x0f);

  /* Pack Member: m_nTemplateEven */
  (pToData)[2] |= ((pFrom)->m_nTemplateEven) & 0xFF;

  /* Pack Member: m_nReservedEven */
  (pToData)[3] |= ((pFrom)->m_nReservedEven & 0x0f) <<4;

  /* Pack Member: m_nGainEven */
  (pToData)[3] |= ((pFrom)->m_nGainEven & 0x0f);
#else
  int i;
  int size = SB_ZF_FAB_BM3200_WRED_DATA_TABLE_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nTemplateOdd */
  (pToData)[3] |= ((pFrom)->m_nTemplateOdd) & 0xFF;

  /* Pack Member: m_nReservedOdd */
  (pToData)[2] |= ((pFrom)->m_nReservedOdd & 0x0f) <<4;

  /* Pack Member: m_nGainOdd */
  (pToData)[2] |= ((pFrom)->m_nGainOdd & 0x0f);

  /* Pack Member: m_nTemplateEven */
  (pToData)[1] |= ((pFrom)->m_nTemplateEven) & 0xFF;

  /* Pack Member: m_nReservedEven */
  (pToData)[0] |= ((pFrom)->m_nReservedEven & 0x0f) <<4;

  /* Pack Member: m_nGainEven */
  (pToData)[0] |= ((pFrom)->m_nGainEven & 0x0f);
#endif

  return SB_ZF_FAB_BM3200_WRED_DATA_TABLE_ENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm3200WredDataTableEntry_Unpack(sbZfFabBm3200WredDataTableEntry_t *pToStruct,
                                       uint8_t *pFromData,
                                       uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nTemplateOdd */
  (pToStruct)->m_nTemplateOdd =  (uint32_t)  (pFromData)[0] ;

  /* Unpack Member: m_nReservedOdd */
  (pToStruct)->m_nReservedOdd =  (uint32_t)  ((pFromData)[1] >> 4) & 0x0f;

  /* Unpack Member: m_nGainOdd */
  (pToStruct)->m_nGainOdd =  (uint32_t)  ((pFromData)[1] ) & 0x0f;

  /* Unpack Member: m_nTemplateEven */
  (pToStruct)->m_nTemplateEven =  (uint32_t)  (pFromData)[2] ;

  /* Unpack Member: m_nReservedEven */
  (pToStruct)->m_nReservedEven =  (uint32_t)  ((pFromData)[3] >> 4) & 0x0f;

  /* Unpack Member: m_nGainEven */
  (pToStruct)->m_nGainEven =  (uint32_t)  ((pFromData)[3] ) & 0x0f;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nTemplateOdd */
  (pToStruct)->m_nTemplateOdd =  (uint32_t)  (pFromData)[3] ;

  /* Unpack Member: m_nReservedOdd */
  (pToStruct)->m_nReservedOdd =  (uint32_t)  ((pFromData)[2] >> 4) & 0x0f;

  /* Unpack Member: m_nGainOdd */
  (pToStruct)->m_nGainOdd =  (uint32_t)  ((pFromData)[2] ) & 0x0f;

  /* Unpack Member: m_nTemplateEven */
  (pToStruct)->m_nTemplateEven =  (uint32_t)  (pFromData)[1] ;

  /* Unpack Member: m_nReservedEven */
  (pToStruct)->m_nReservedEven =  (uint32_t)  ((pFromData)[0] >> 4) & 0x0f;

  /* Unpack Member: m_nGainEven */
  (pToStruct)->m_nGainEven =  (uint32_t)  ((pFromData)[0] ) & 0x0f;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm3200WredDataTableEntry_InitInstance(sbZfFabBm3200WredDataTableEntry_t *pFrame) {

  pFrame->m_nTemplateOdd =  (unsigned int)  0;
  pFrame->m_nReservedOdd =  (unsigned int)  0;
  pFrame->m_nGainOdd =  (unsigned int)  0;
  pFrame->m_nTemplateEven =  (unsigned int)  0;
  pFrame->m_nReservedEven =  (unsigned int)  0;
  pFrame->m_nGainEven =  (unsigned int)  0;

}
#ifdef SB_ZF_INCLUDE_CONSOLE
/*
 * $Id: sbZfFabBm3200WredDataTableEntry.c,v 1.1.48.4 2011/05/22 05:39:14 iakramov Exp $
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
#include "sbZfFabBm3200WredDataTableEntry.hx"



/* Print members in struct */
void
sbZfFabBm3200WredDataTableEntry_Print(sbZfFabBm3200WredDataTableEntry_t *pFromStruct) {
  SB_LOG("FabBm3200WredDataTableEntry:: template_odd=0x%02x", (unsigned int)  pFromStruct->m_nTemplateOdd);
  SB_LOG(" reserved_odd=0x%01x", (unsigned int)  pFromStruct->m_nReservedOdd);
  SB_LOG("\n");

  SB_LOG("FabBm3200WredDataTableEntry:: gain_odd=0x%01x", (unsigned int)  pFromStruct->m_nGainOdd);
  SB_LOG(" template_even=0x%02x", (unsigned int)  pFromStruct->m_nTemplateEven);
  SB_LOG("\n");

  SB_LOG("FabBm3200WredDataTableEntry:: reserved_even=0x%01x", (unsigned int)  pFromStruct->m_nReservedEven);
  SB_LOG(" gain_even=0x%01x", (unsigned int)  pFromStruct->m_nGainEven);
  SB_LOG("\n");

}

/* SPrint members in struct */
int
sbZfFabBm3200WredDataTableEntry_SPrint(sbZfFabBm3200WredDataTableEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm3200WredDataTableEntry:: template_odd=0x%02x", (unsigned int)  pFromStruct->m_nTemplateOdd);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," reserved_odd=0x%01x", (unsigned int)  pFromStruct->m_nReservedOdd);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm3200WredDataTableEntry:: gain_odd=0x%01x", (unsigned int)  pFromStruct->m_nGainOdd);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," template_even=0x%02x", (unsigned int)  pFromStruct->m_nTemplateEven);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm3200WredDataTableEntry:: reserved_even=0x%01x", (unsigned int)  pFromStruct->m_nReservedEven);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," gain_even=0x%01x", (unsigned int)  pFromStruct->m_nGainEven);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(WrCnt);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm3200WredDataTableEntry_Validate(sbZfFabBm3200WredDataTableEntry_t *pZf) {

  if (pZf->m_nTemplateOdd > 0xff) return 0;
  if (pZf->m_nReservedOdd > 0xf) return 0;
  if (pZf->m_nGainOdd > 0xf) return 0;
  if (pZf->m_nTemplateEven > 0xff) return 0;
  if (pZf->m_nReservedEven > 0xf) return 0;
  if (pZf->m_nGainEven > 0xf) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm3200WredDataTableEntry_SetField(sbZfFabBm3200WredDataTableEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_ntemplateodd") == 0) {
    s->m_nTemplateOdd = value;
  } else if (SB_STRCMP(name, "m_nreservedodd") == 0) {
    s->m_nReservedOdd = value;
  } else if (SB_STRCMP(name, "m_ngainodd") == 0) {
    s->m_nGainOdd = value;
  } else if (SB_STRCMP(name, "m_ntemplateeven") == 0) {
    s->m_nTemplateEven = value;
  } else if (SB_STRCMP(name, "m_nreservedeven") == 0) {
    s->m_nReservedEven = value;
  } else if (SB_STRCMP(name, "m_ngaineven") == 0) {
    s->m_nGainEven = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}


#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */
