/*
 * $Id: sbZfFabBm3200NextPriMemEntry.c 1.2.28.4 Broadcom SDK $
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
#include "sbZfFabBm3200NextPriMemEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfFabBm3200NextPriMemEntry_Pack(sbZfFabBm3200NextPriMemEntry_t *pFrom,
                                  uint8_t *pToData,
                                  uint32_t nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM3200_NEXT_PRI_ENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nNextPrid */
  (pToData)[15] |= ((pFrom)->m_nNextPrid) & 0xFF;
  (pToData)[14] |= ((pFrom)->m_nNextPrid >> 8) & 0x0f;

  /* Pack Member: m_nNextPric */
  (pToData)[11] |= ((pFrom)->m_nNextPric) & 0xFF;
  (pToData)[10] |= ((pFrom)->m_nNextPric >> 8) &0xFF;
  (pToData)[9] |= ((pFrom)->m_nNextPric >> 16) &0xFF;
  (pToData)[8] |= ((pFrom)->m_nNextPric >> 24) &0xFF;

  /* Pack Member: m_nNextPrib */
  (pToData)[7] |= ((pFrom)->m_nNextPrib) & 0xFF;
  (pToData)[6] |= ((pFrom)->m_nNextPrib >> 8) &0xFF;
  (pToData)[5] |= ((pFrom)->m_nNextPrib >> 16) &0xFF;
  (pToData)[4] |= ((pFrom)->m_nNextPrib >> 24) &0xFF;

  /* Pack Member: m_nNextPria */
  (pToData)[3] |= ((pFrom)->m_nNextPria) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nNextPria >> 8) &0xFF;
  (pToData)[1] |= ((pFrom)->m_nNextPria >> 16) &0xFF;
  (pToData)[0] |= ((pFrom)->m_nNextPria >> 24) &0xFF;
#else
  int i;
  int size = SB_ZF_FAB_BM3200_NEXT_PRI_ENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nNextPrid */
  (pToData)[12] |= ((pFrom)->m_nNextPrid) & 0xFF;
  (pToData)[13] |= ((pFrom)->m_nNextPrid >> 8) & 0x0f;

  /* Pack Member: m_nNextPric */
  (pToData)[8] |= ((pFrom)->m_nNextPric) & 0xFF;
  (pToData)[9] |= ((pFrom)->m_nNextPric >> 8) &0xFF;
  (pToData)[10] |= ((pFrom)->m_nNextPric >> 16) &0xFF;
  (pToData)[11] |= ((pFrom)->m_nNextPric >> 24) &0xFF;

  /* Pack Member: m_nNextPrib */
  (pToData)[4] |= ((pFrom)->m_nNextPrib) & 0xFF;
  (pToData)[5] |= ((pFrom)->m_nNextPrib >> 8) &0xFF;
  (pToData)[6] |= ((pFrom)->m_nNextPrib >> 16) &0xFF;
  (pToData)[7] |= ((pFrom)->m_nNextPrib >> 24) &0xFF;

  /* Pack Member: m_nNextPria */
  (pToData)[0] |= ((pFrom)->m_nNextPria) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nNextPria >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nNextPria >> 16) &0xFF;
  (pToData)[3] |= ((pFrom)->m_nNextPria >> 24) &0xFF;
#endif

  return SB_ZF_FAB_BM3200_NEXT_PRI_ENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm3200NextPriMemEntry_Unpack(sbZfFabBm3200NextPriMemEntry_t *pToStruct,
                                    uint8_t *pFromData,
                                    uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nNextPrid */
  (pToStruct)->m_nNextPrid =  (uint32_t)  (pFromData)[15] ;
  (pToStruct)->m_nNextPrid |=  (uint32_t)  ((pFromData)[14] & 0x0f) << 8;

  /* Unpack Member: m_nNextPric */
  (pToStruct)->m_nNextPric =  (uint32_t)  (pFromData)[11] ;
  (pToStruct)->m_nNextPric |=  (uint32_t)  (pFromData)[10] << 8;
  (pToStruct)->m_nNextPric |=  (uint32_t)  (pFromData)[9] << 16;
  (pToStruct)->m_nNextPric |=  (uint32_t)  (pFromData)[8] << 24;

  /* Unpack Member: m_nNextPrib */
  (pToStruct)->m_nNextPrib =  (uint32_t)  (pFromData)[7] ;
  (pToStruct)->m_nNextPrib |=  (uint32_t)  (pFromData)[6] << 8;
  (pToStruct)->m_nNextPrib |=  (uint32_t)  (pFromData)[5] << 16;
  (pToStruct)->m_nNextPrib |=  (uint32_t)  (pFromData)[4] << 24;

  /* Unpack Member: m_nNextPria */
  (pToStruct)->m_nNextPria =  (uint32_t)  (pFromData)[3] ;
  (pToStruct)->m_nNextPria |=  (uint32_t)  (pFromData)[2] << 8;
  (pToStruct)->m_nNextPria |=  (uint32_t)  (pFromData)[1] << 16;
  (pToStruct)->m_nNextPria |=  (uint32_t)  (pFromData)[0] << 24;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nNextPrid */
  (pToStruct)->m_nNextPrid =  (uint32_t)  (pFromData)[12] ;
  (pToStruct)->m_nNextPrid |=  (uint32_t)  ((pFromData)[13] & 0x0f) << 8;

  /* Unpack Member: m_nNextPric */
  (pToStruct)->m_nNextPric =  (uint32_t)  (pFromData)[8] ;
  (pToStruct)->m_nNextPric |=  (uint32_t)  (pFromData)[9] << 8;
  (pToStruct)->m_nNextPric |=  (uint32_t)  (pFromData)[10] << 16;
  (pToStruct)->m_nNextPric |=  (uint32_t)  (pFromData)[11] << 24;

  /* Unpack Member: m_nNextPrib */
  (pToStruct)->m_nNextPrib =  (uint32_t)  (pFromData)[4] ;
  (pToStruct)->m_nNextPrib |=  (uint32_t)  (pFromData)[5] << 8;
  (pToStruct)->m_nNextPrib |=  (uint32_t)  (pFromData)[6] << 16;
  (pToStruct)->m_nNextPrib |=  (uint32_t)  (pFromData)[7] << 24;

  /* Unpack Member: m_nNextPria */
  (pToStruct)->m_nNextPria =  (uint32_t)  (pFromData)[0] ;
  (pToStruct)->m_nNextPria |=  (uint32_t)  (pFromData)[1] << 8;
  (pToStruct)->m_nNextPria |=  (uint32_t)  (pFromData)[2] << 16;
  (pToStruct)->m_nNextPria |=  (uint32_t)  (pFromData)[3] << 24;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm3200NextPriMemEntry_InitInstance(sbZfFabBm3200NextPriMemEntry_t *pFrame) {

  pFrame->m_nNextPrid =  (unsigned int)  0;
  pFrame->m_nNextPric =  (unsigned int)  0;
  pFrame->m_nNextPrib =  (unsigned int)  0;
  pFrame->m_nNextPria =  (unsigned int)  0;

}
#ifdef SB_ZF_INCLUDE_CONSOLE
/*
 * $Id: sbZfFabBm3200NextPriMemEntry.c,v 1.2.28.4 2011/05/22 05:39:14 iakramov Exp $
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
#include "sbZfFabBm3200NextPriMemEntry.hx"



/* Print members in struct */
void
sbZfFabBm3200NextPriMemEntry_Print(sbZfFabBm3200NextPriMemEntry_t *pFromStruct) {
  SB_LOG("FabBm3200NextPriMemEntry:: next_prid=0x%03x", (unsigned int)  pFromStruct->m_nNextPrid);
  SB_LOG(" next_pric=0x%08x", (unsigned int)  pFromStruct->m_nNextPric);
  SB_LOG("\n");

  SB_LOG("FabBm3200NextPriMemEntry:: next_prib=0x%08x", (unsigned int)  pFromStruct->m_nNextPrib);
  SB_LOG(" next_pria=0x%08x", (unsigned int)  pFromStruct->m_nNextPria);
  SB_LOG("\n");

}

/* SPrint members in struct */
int
sbZfFabBm3200NextPriMemEntry_SPrint(sbZfFabBm3200NextPriMemEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm3200NextPriMemEntry:: next_prid=0x%03x", (unsigned int)  pFromStruct->m_nNextPrid);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," next_pric=0x%08x", (unsigned int)  pFromStruct->m_nNextPric);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm3200NextPriMemEntry:: next_prib=0x%08x", (unsigned int)  pFromStruct->m_nNextPrib);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," next_pria=0x%08x", (unsigned int)  pFromStruct->m_nNextPria);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(WrCnt);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm3200NextPriMemEntry_Validate(sbZfFabBm3200NextPriMemEntry_t *pZf) {

  if (pZf->m_nNextPrid > 0xfff) return 0;
  /* pZf->m_nNextPric implicitly masked by data type */
  /* pZf->m_nNextPrib implicitly masked by data type */
  /* pZf->m_nNextPria implicitly masked by data type */

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm3200NextPriMemEntry_SetField(sbZfFabBm3200NextPriMemEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nnextprid") == 0) {
    s->m_nNextPrid = value;
  } else if (SB_STRCMP(name, "m_nnextpric") == 0) {
    s->m_nNextPric = value;
  } else if (SB_STRCMP(name, "m_nnextprib") == 0) {
    s->m_nNextPrib = value;
  } else if (SB_STRCMP(name, "m_nnextpria") == 0) {
    s->m_nNextPria = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}


#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */
