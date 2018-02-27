/*
 * $Id: sbZfFabBm3200BwDstEntry.c 1.2.28.4 Broadcom SDK $
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
#include "sbZfFabBm3200BwDstEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfFabBm3200BwDstEntry_Pack(sbZfFabBm3200BwDstEntry_t *pFrom,
                             uint8_t *pToData,
                             uint32_t nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM3200_DST_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nDelta */
  (pToData)[1] |= ((pFrom)->m_nDelta & 0x03) <<6;
  (pToData)[0] |= ((pFrom)->m_nDelta >> 2) &0xFF;

  /* Pack Member: m_nDemand */
  (pToData)[3] |= ((pFrom)->m_nDemand) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nDemand >> 8) &0xFF;
  (pToData)[1] |= ((pFrom)->m_nDemand >> 16) & 0x3f;
#else
  int i;
  int size = SB_ZF_FAB_BM3200_DST_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nDelta */
  (pToData)[2] |= ((pFrom)->m_nDelta & 0x03) <<6;
  (pToData)[3] |= ((pFrom)->m_nDelta >> 2) &0xFF;

  /* Pack Member: m_nDemand */
  (pToData)[0] |= ((pFrom)->m_nDemand) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nDemand >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nDemand >> 16) & 0x3f;
#endif

  return SB_ZF_FAB_BM3200_DST_ENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm3200BwDstEntry_Unpack(sbZfFabBm3200BwDstEntry_t *pToStruct,
                               uint8_t *pFromData,
                               uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nDelta */
  (pToStruct)->m_nDelta =  (uint32_t)  ((pFromData)[1] >> 6) & 0x03;
  (pToStruct)->m_nDelta |=  (uint32_t)  (pFromData)[0] << 2;

  /* Unpack Member: m_nDemand */
  (pToStruct)->m_nDemand =  (uint32_t)  (pFromData)[3] ;
  (pToStruct)->m_nDemand |=  (uint32_t)  (pFromData)[2] << 8;
  (pToStruct)->m_nDemand |=  (uint32_t)  ((pFromData)[1] & 0x3f) << 16;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nDelta */
  (pToStruct)->m_nDelta =  (uint32_t)  ((pFromData)[2] >> 6) & 0x03;
  (pToStruct)->m_nDelta |=  (uint32_t)  (pFromData)[3] << 2;

  /* Unpack Member: m_nDemand */
  (pToStruct)->m_nDemand =  (uint32_t)  (pFromData)[0] ;
  (pToStruct)->m_nDemand |=  (uint32_t)  (pFromData)[1] << 8;
  (pToStruct)->m_nDemand |=  (uint32_t)  ((pFromData)[2] & 0x3f) << 16;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm3200BwDstEntry_InitInstance(sbZfFabBm3200BwDstEntry_t *pFrame) {

  pFrame->m_nDelta =  (unsigned int)  0;
  pFrame->m_nDemand =  (unsigned int)  0;

}
#ifdef SB_ZF_INCLUDE_CONSOLE
/*
 * $Id: sbZfFabBm3200BwDstEntry.c,v 1.2.28.4 2011/05/22 05:39:14 iakramov Exp $
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
#include "sbZfFabBm3200BwDstEntry.hx"



/* Print members in struct */
void
sbZfFabBm3200BwDstEntry_Print(sbZfFabBm3200BwDstEntry_t *pFromStruct) {
  SB_LOG("FabBm3200BwDstEntry:: delta=0x%03x", (unsigned int)  pFromStruct->m_nDelta);
  SB_LOG(" demand=0x%06x", (unsigned int)  pFromStruct->m_nDemand);
  SB_LOG("\n");

}

/* SPrint members in struct */
int
sbZfFabBm3200BwDstEntry_SPrint(sbZfFabBm3200BwDstEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm3200BwDstEntry:: delta=0x%03x", (unsigned int)  pFromStruct->m_nDelta);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," demand=0x%06x", (unsigned int)  pFromStruct->m_nDemand);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(WrCnt);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm3200BwDstEntry_Validate(sbZfFabBm3200BwDstEntry_t *pZf) {

  if (pZf->m_nDelta > 0x3ff) return 0;
  if (pZf->m_nDemand > 0x3fffff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm3200BwDstEntry_SetField(sbZfFabBm3200BwDstEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_ndelta") == 0) {
    s->m_nDelta = value;
  } else if (SB_STRCMP(name, "m_ndemand") == 0) {
    s->m_nDemand = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}


#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */
