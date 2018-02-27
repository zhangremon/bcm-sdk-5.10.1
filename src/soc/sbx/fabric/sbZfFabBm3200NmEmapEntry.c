/*
 * $Id: sbZfFabBm3200NmEmapEntry.c 1.2.28.4 Broadcom SDK $
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
#include "sbZfFabBm3200NmEmapEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfFabBm3200NmEmapEntry_Pack(sbZfFabBm3200NmEmapEntry_t *pFrom,
                              uint8_t *pToData,
                              uint32_t nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM3200_EMAP_ENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nEmapx */
  (pToData)[19] |= ((pFrom)->m_nEmapx & 0x0f);

  /* Pack Member: m_nEmapd */
  (pToData)[15] |= ((pFrom)->m_nEmapd) & 0xFF;
  (pToData)[14] |= ((pFrom)->m_nEmapd >> 8) &0xFF;
  (pToData)[13] |= ((pFrom)->m_nEmapd >> 16) &0xFF;
  (pToData)[12] |= ((pFrom)->m_nEmapd >> 24) &0xFF;

  /* Pack Member: m_nEmapc */
  (pToData)[11] |= ((pFrom)->m_nEmapc) & 0xFF;
  (pToData)[10] |= ((pFrom)->m_nEmapc >> 8) &0xFF;
  (pToData)[9] |= ((pFrom)->m_nEmapc >> 16) &0xFF;
  (pToData)[8] |= ((pFrom)->m_nEmapc >> 24) &0xFF;

  /* Pack Member: m_nEmapb */
  (pToData)[7] |= ((pFrom)->m_nEmapb) & 0xFF;
  (pToData)[6] |= ((pFrom)->m_nEmapb >> 8) &0xFF;
  (pToData)[5] |= ((pFrom)->m_nEmapb >> 16) &0xFF;
  (pToData)[4] |= ((pFrom)->m_nEmapb >> 24) &0xFF;

  /* Pack Member: m_nEmapa */
  (pToData)[3] |= ((pFrom)->m_nEmapa) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nEmapa >> 8) &0xFF;
  (pToData)[1] |= ((pFrom)->m_nEmapa >> 16) &0xFF;
  (pToData)[0] |= ((pFrom)->m_nEmapa >> 24) &0xFF;
#else
  int i;
  int size = SB_ZF_FAB_BM3200_EMAP_ENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nEmapx */
  (pToData)[16] |= ((pFrom)->m_nEmapx & 0x0f);

  /* Pack Member: m_nEmapd */
  (pToData)[12] |= ((pFrom)->m_nEmapd) & 0xFF;
  (pToData)[13] |= ((pFrom)->m_nEmapd >> 8) &0xFF;
  (pToData)[14] |= ((pFrom)->m_nEmapd >> 16) &0xFF;
  (pToData)[15] |= ((pFrom)->m_nEmapd >> 24) &0xFF;

  /* Pack Member: m_nEmapc */
  (pToData)[8] |= ((pFrom)->m_nEmapc) & 0xFF;
  (pToData)[9] |= ((pFrom)->m_nEmapc >> 8) &0xFF;
  (pToData)[10] |= ((pFrom)->m_nEmapc >> 16) &0xFF;
  (pToData)[11] |= ((pFrom)->m_nEmapc >> 24) &0xFF;

  /* Pack Member: m_nEmapb */
  (pToData)[4] |= ((pFrom)->m_nEmapb) & 0xFF;
  (pToData)[5] |= ((pFrom)->m_nEmapb >> 8) &0xFF;
  (pToData)[6] |= ((pFrom)->m_nEmapb >> 16) &0xFF;
  (pToData)[7] |= ((pFrom)->m_nEmapb >> 24) &0xFF;

  /* Pack Member: m_nEmapa */
  (pToData)[0] |= ((pFrom)->m_nEmapa) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nEmapa >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nEmapa >> 16) &0xFF;
  (pToData)[3] |= ((pFrom)->m_nEmapa >> 24) &0xFF;
#endif

  return SB_ZF_FAB_BM3200_EMAP_ENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm3200NmEmapEntry_Unpack(sbZfFabBm3200NmEmapEntry_t *pToStruct,
                                uint8_t *pFromData,
                                uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nEmapx */
  (pToStruct)->m_nEmapx =  (uint32_t)  ((pFromData)[19] ) & 0x0f;

  /* Unpack Member: m_nEmapd */
  (pToStruct)->m_nEmapd =  (uint32_t)  (pFromData)[15] ;
  (pToStruct)->m_nEmapd |=  (uint32_t)  (pFromData)[14] << 8;
  (pToStruct)->m_nEmapd |=  (uint32_t)  (pFromData)[13] << 16;
  (pToStruct)->m_nEmapd |=  (uint32_t)  (pFromData)[12] << 24;

  /* Unpack Member: m_nEmapc */
  (pToStruct)->m_nEmapc =  (uint32_t)  (pFromData)[11] ;
  (pToStruct)->m_nEmapc |=  (uint32_t)  (pFromData)[10] << 8;
  (pToStruct)->m_nEmapc |=  (uint32_t)  (pFromData)[9] << 16;
  (pToStruct)->m_nEmapc |=  (uint32_t)  (pFromData)[8] << 24;

  /* Unpack Member: m_nEmapb */
  (pToStruct)->m_nEmapb =  (uint32_t)  (pFromData)[7] ;
  (pToStruct)->m_nEmapb |=  (uint32_t)  (pFromData)[6] << 8;
  (pToStruct)->m_nEmapb |=  (uint32_t)  (pFromData)[5] << 16;
  (pToStruct)->m_nEmapb |=  (uint32_t)  (pFromData)[4] << 24;

  /* Unpack Member: m_nEmapa */
  (pToStruct)->m_nEmapa =  (uint32_t)  (pFromData)[3] ;
  (pToStruct)->m_nEmapa |=  (uint32_t)  (pFromData)[2] << 8;
  (pToStruct)->m_nEmapa |=  (uint32_t)  (pFromData)[1] << 16;
  (pToStruct)->m_nEmapa |=  (uint32_t)  (pFromData)[0] << 24;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nEmapx */
  (pToStruct)->m_nEmapx =  (uint32_t)  ((pFromData)[16] ) & 0x0f;

  /* Unpack Member: m_nEmapd */
  (pToStruct)->m_nEmapd =  (uint32_t)  (pFromData)[12] ;
  (pToStruct)->m_nEmapd |=  (uint32_t)  (pFromData)[13] << 8;
  (pToStruct)->m_nEmapd |=  (uint32_t)  (pFromData)[14] << 16;
  (pToStruct)->m_nEmapd |=  (uint32_t)  (pFromData)[15] << 24;

  /* Unpack Member: m_nEmapc */
  (pToStruct)->m_nEmapc =  (uint32_t)  (pFromData)[8] ;
  (pToStruct)->m_nEmapc |=  (uint32_t)  (pFromData)[9] << 8;
  (pToStruct)->m_nEmapc |=  (uint32_t)  (pFromData)[10] << 16;
  (pToStruct)->m_nEmapc |=  (uint32_t)  (pFromData)[11] << 24;

  /* Unpack Member: m_nEmapb */
  (pToStruct)->m_nEmapb =  (uint32_t)  (pFromData)[4] ;
  (pToStruct)->m_nEmapb |=  (uint32_t)  (pFromData)[5] << 8;
  (pToStruct)->m_nEmapb |=  (uint32_t)  (pFromData)[6] << 16;
  (pToStruct)->m_nEmapb |=  (uint32_t)  (pFromData)[7] << 24;

  /* Unpack Member: m_nEmapa */
  (pToStruct)->m_nEmapa =  (uint32_t)  (pFromData)[0] ;
  (pToStruct)->m_nEmapa |=  (uint32_t)  (pFromData)[1] << 8;
  (pToStruct)->m_nEmapa |=  (uint32_t)  (pFromData)[2] << 16;
  (pToStruct)->m_nEmapa |=  (uint32_t)  (pFromData)[3] << 24;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm3200NmEmapEntry_InitInstance(sbZfFabBm3200NmEmapEntry_t *pFrame) {

  pFrame->m_nEmapx =  (unsigned int)  0;
  pFrame->m_nEmapd =  (unsigned int)  0;
  pFrame->m_nEmapc =  (unsigned int)  0;
  pFrame->m_nEmapb =  (unsigned int)  0;
  pFrame->m_nEmapa =  (unsigned int)  0;

}
#ifdef SB_ZF_INCLUDE_CONSOLE
/*
 * $Id: sbZfFabBm3200NmEmapEntry.c,v 1.2.28.4 2011/05/22 05:39:14 iakramov Exp $
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
#include "sbZfFabBm3200NmEmapEntry.hx"



/* Print members in struct */
void
sbZfFabBm3200NmEmapEntry_Print(sbZfFabBm3200NmEmapEntry_t *pFromStruct) {
  SB_LOG("FabBm3200NmEmapEntry:: emapx=0x%01x", (unsigned int)  pFromStruct->m_nEmapx);
  SB_LOG(" emapd=0x%08x", (unsigned int)  pFromStruct->m_nEmapd);
  SB_LOG(" emapc=0x%08x", (unsigned int)  pFromStruct->m_nEmapc);
  SB_LOG("\n");

  SB_LOG("FabBm3200NmEmapEntry:: emapb=0x%08x", (unsigned int)  pFromStruct->m_nEmapb);
  SB_LOG(" emapa=0x%08x", (unsigned int)  pFromStruct->m_nEmapa);
  SB_LOG("\n");

}

/* SPrint members in struct */
int
sbZfFabBm3200NmEmapEntry_SPrint(sbZfFabBm3200NmEmapEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm3200NmEmapEntry:: emapx=0x%01x", (unsigned int)  pFromStruct->m_nEmapx);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," emapd=0x%08x", (unsigned int)  pFromStruct->m_nEmapd);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," emapc=0x%08x", (unsigned int)  pFromStruct->m_nEmapc);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm3200NmEmapEntry:: emapb=0x%08x", (unsigned int)  pFromStruct->m_nEmapb);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," emapa=0x%08x", (unsigned int)  pFromStruct->m_nEmapa);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(WrCnt);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm3200NmEmapEntry_Validate(sbZfFabBm3200NmEmapEntry_t *pZf) {

  if (pZf->m_nEmapx > 0xf) return 0;
  /* pZf->m_nEmapd implicitly masked by data type */
  /* pZf->m_nEmapc implicitly masked by data type */
  /* pZf->m_nEmapb implicitly masked by data type */
  /* pZf->m_nEmapa implicitly masked by data type */

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm3200NmEmapEntry_SetField(sbZfFabBm3200NmEmapEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nemapx") == 0) {
    s->m_nEmapx = value;
  } else if (SB_STRCMP(name, "m_nemapd") == 0) {
    s->m_nEmapd = value;
  } else if (SB_STRCMP(name, "m_nemapc") == 0) {
    s->m_nEmapc = value;
  } else if (SB_STRCMP(name, "m_nemapb") == 0) {
    s->m_nEmapb = value;
  } else if (SB_STRCMP(name, "m_nemapa") == 0) {
    s->m_nEmapa = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}


#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */
