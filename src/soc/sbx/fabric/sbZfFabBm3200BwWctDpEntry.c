/*
 * $Id: sbZfFabBm3200BwWctDpEntry.c 1.2.28.4 Broadcom SDK $
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
#include "sbZfFabBm3200BwWctDpEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfFabBm3200BwWctDpEntry_Pack(sbZfFabBm3200BwWctDpEntry_t *pFrom,
                               uint8_t *pToData,
                               uint32_t nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM3200_WCT_DPENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nEcn */
  (pToData)[5] |= ((pFrom)->m_nEcn) & 0xFF;
  (pToData)[4] |= ((pFrom)->m_nEcn >> 8) &0xFF;

  /* Pack Member: m_nScale */
  (pToData)[6] |= ((pFrom)->m_nScale & 0x0f) <<4;

  /* Pack Member: m_nSlope */
  (pToData)[7] |= ((pFrom)->m_nSlope) & 0xFF;
  (pToData)[6] |= ((pFrom)->m_nSlope >> 8) & 0x0f;

  /* Pack Member: m_nMin */
  (pToData)[1] |= ((pFrom)->m_nMin) & 0xFF;
  (pToData)[0] |= ((pFrom)->m_nMin >> 8) &0xFF;

  /* Pack Member: m_nMax */
  (pToData)[3] |= ((pFrom)->m_nMax) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nMax >> 8) &0xFF;
#else
  int i;
  int size = SB_ZF_FAB_BM3200_WCT_DPENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nEcn */
  (pToData)[6] |= ((pFrom)->m_nEcn) & 0xFF;
  (pToData)[7] |= ((pFrom)->m_nEcn >> 8) &0xFF;

  /* Pack Member: m_nScale */
  (pToData)[5] |= ((pFrom)->m_nScale & 0x0f) <<4;

  /* Pack Member: m_nSlope */
  (pToData)[4] |= ((pFrom)->m_nSlope) & 0xFF;
  (pToData)[5] |= ((pFrom)->m_nSlope >> 8) & 0x0f;

  /* Pack Member: m_nMin */
  (pToData)[2] |= ((pFrom)->m_nMin) & 0xFF;
  (pToData)[3] |= ((pFrom)->m_nMin >> 8) &0xFF;

  /* Pack Member: m_nMax */
  (pToData)[0] |= ((pFrom)->m_nMax) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nMax >> 8) &0xFF;
#endif

  return SB_ZF_FAB_BM3200_WCT_DPENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm3200BwWctDpEntry_Unpack(sbZfFabBm3200BwWctDpEntry_t *pToStruct,
                                 uint8_t *pFromData,
                                 uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nEcn */
  (pToStruct)->m_nEcn =  (uint32_t)  (pFromData)[5] ;
  (pToStruct)->m_nEcn |=  (uint32_t)  (pFromData)[4] << 8;

  /* Unpack Member: m_nScale */
  (pToStruct)->m_nScale =  (uint32_t)  ((pFromData)[6] >> 4) & 0x0f;

  /* Unpack Member: m_nSlope */
  (pToStruct)->m_nSlope =  (uint32_t)  (pFromData)[7] ;
  (pToStruct)->m_nSlope |=  (uint32_t)  ((pFromData)[6] & 0x0f) << 8;

  /* Unpack Member: m_nMin */
  (pToStruct)->m_nMin =  (uint32_t)  (pFromData)[1] ;
  (pToStruct)->m_nMin |=  (uint32_t)  (pFromData)[0] << 8;

  /* Unpack Member: m_nMax */
  (pToStruct)->m_nMax =  (uint32_t)  (pFromData)[3] ;
  (pToStruct)->m_nMax |=  (uint32_t)  (pFromData)[2] << 8;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nEcn */
  (pToStruct)->m_nEcn =  (uint32_t)  (pFromData)[6] ;
  (pToStruct)->m_nEcn |=  (uint32_t)  (pFromData)[7] << 8;

  /* Unpack Member: m_nScale */
  (pToStruct)->m_nScale =  (uint32_t)  ((pFromData)[5] >> 4) & 0x0f;

  /* Unpack Member: m_nSlope */
  (pToStruct)->m_nSlope =  (uint32_t)  (pFromData)[4] ;
  (pToStruct)->m_nSlope |=  (uint32_t)  ((pFromData)[5] & 0x0f) << 8;

  /* Unpack Member: m_nMin */
  (pToStruct)->m_nMin =  (uint32_t)  (pFromData)[2] ;
  (pToStruct)->m_nMin |=  (uint32_t)  (pFromData)[3] << 8;

  /* Unpack Member: m_nMax */
  (pToStruct)->m_nMax =  (uint32_t)  (pFromData)[0] ;
  (pToStruct)->m_nMax |=  (uint32_t)  (pFromData)[1] << 8;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm3200BwWctDpEntry_InitInstance(sbZfFabBm3200BwWctDpEntry_t *pFrame) {

  pFrame->m_nEcn =  (unsigned int)  0;
  pFrame->m_nScale =  (unsigned int)  0;
  pFrame->m_nSlope =  (unsigned int)  0;
  pFrame->m_nMin =  (unsigned int)  0;
  pFrame->m_nMax =  (unsigned int)  0;

}
#ifdef SB_ZF_INCLUDE_CONSOLE
/*
 * $Id: sbZfFabBm3200BwWctDpEntry.c,v 1.2.28.4 2011/05/22 05:39:14 iakramov Exp $
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
#include "sbZfFabBm3200BwWctDpEntry.hx"



/* Print members in struct */
void
sbZfFabBm3200BwWctDpEntry_Print(sbZfFabBm3200BwWctDpEntry_t *pFromStruct) {
  SB_LOG("FabBm3200BwWctDpEntry:: ecn=0x%04x", (unsigned int)  pFromStruct->m_nEcn);
  SB_LOG(" scale=0x%01x", (unsigned int)  pFromStruct->m_nScale);
  SB_LOG(" slope=0x%03x", (unsigned int)  pFromStruct->m_nSlope);
  SB_LOG(" min=0x%04x", (unsigned int)  pFromStruct->m_nMin);
  SB_LOG("\n");

  SB_LOG("FabBm3200BwWctDpEntry:: max=0x%04x", (unsigned int)  pFromStruct->m_nMax);
  SB_LOG("\n");

}

/* SPrint members in struct */
int
sbZfFabBm3200BwWctDpEntry_SPrint(sbZfFabBm3200BwWctDpEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm3200BwWctDpEntry:: ecn=0x%04x", (unsigned int)  pFromStruct->m_nEcn);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," scale=0x%01x", (unsigned int)  pFromStruct->m_nScale);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," slope=0x%03x", (unsigned int)  pFromStruct->m_nSlope);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," min=0x%04x", (unsigned int)  pFromStruct->m_nMin);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm3200BwWctDpEntry:: max=0x%04x", (unsigned int)  pFromStruct->m_nMax);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(WrCnt);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm3200BwWctDpEntry_Validate(sbZfFabBm3200BwWctDpEntry_t *pZf) {

  if (pZf->m_nEcn > 0xffff) return 0;
  if (pZf->m_nScale > 0xf) return 0;
  if (pZf->m_nSlope > 0xfff) return 0;
  if (pZf->m_nMin > 0xffff) return 0;
  if (pZf->m_nMax > 0xffff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm3200BwWctDpEntry_SetField(sbZfFabBm3200BwWctDpEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_necn") == 0) {
    s->m_nEcn = value;
  } else if (SB_STRCMP(name, "m_nscale") == 0) {
    s->m_nScale = value;
  } else if (SB_STRCMP(name, "m_nslope") == 0) {
    s->m_nSlope = value;
  } else if (SB_STRCMP(name, "m_nmin") == 0) {
    s->m_nMin = value;
  } else if (SB_STRCMP(name, "m_nmax") == 0) {
    s->m_nMax = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}


#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */
