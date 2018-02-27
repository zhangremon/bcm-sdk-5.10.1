/*
 * $Id: sbZfFabBm3200BwPortRateEntry.c 1.1.48.4 Broadcom SDK $
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
#include "sbZfFabBm3200BwPortRateEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfFabBm3200BwPortRateEntry_Pack(sbZfFabBm3200BwPortRateEntry_t *pFrom,
                                  uint8_t *pToData,
                                  uint32_t nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM3200_PORT_RATE_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nReserved */
  (pToData)[0] |= ((pFrom)->m_nReserved & 0x3f) <<2;

  /* Pack Member: m_nSpGroups */
  (pToData)[1] |= ((pFrom)->m_nSpGroups & 0x07) <<5;
  (pToData)[0] |= ((pFrom)->m_nSpGroups >> 3) & 0x03;

  /* Pack Member: m_nGroups */
  (pToData)[1] |= ((pFrom)->m_nGroups & 0x1f);

  /* Pack Member: m_nGroup */
  (pToData)[3] |= ((pFrom)->m_nGroup) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nGroup >> 8) &0xFF;

  /* Pack Member: m_nLineRate */
  (pToData)[7] |= ((pFrom)->m_nLineRate) & 0xFF;
  (pToData)[6] |= ((pFrom)->m_nLineRate >> 8) &0xFF;
  (pToData)[5] |= ((pFrom)->m_nLineRate >> 16) & 0x7f;

  /* Pack Member: m_nReserved1 */
  (pToData)[5] |= ((pFrom)->m_nReserved1 & 0x01) <<7;
  (pToData)[4] |= ((pFrom)->m_nReserved1 >> 1) &0xFF;
#else
  int i;
  int size = SB_ZF_FAB_BM3200_PORT_RATE_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nReserved */
  (pToData)[3] |= ((pFrom)->m_nReserved & 0x3f) <<2;

  /* Pack Member: m_nSpGroups */
  (pToData)[2] |= ((pFrom)->m_nSpGroups & 0x07) <<5;
  (pToData)[3] |= ((pFrom)->m_nSpGroups >> 3) & 0x03;

  /* Pack Member: m_nGroups */
  (pToData)[2] |= ((pFrom)->m_nGroups & 0x1f);

  /* Pack Member: m_nGroup */
  (pToData)[0] |= ((pFrom)->m_nGroup) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nGroup >> 8) &0xFF;

  /* Pack Member: m_nLineRate */
  (pToData)[4] |= ((pFrom)->m_nLineRate) & 0xFF;
  (pToData)[5] |= ((pFrom)->m_nLineRate >> 8) &0xFF;
  (pToData)[6] |= ((pFrom)->m_nLineRate >> 16) & 0x7f;

  /* Pack Member: m_nReserved1 */
  (pToData)[6] |= ((pFrom)->m_nReserved1 & 0x01) <<7;
  (pToData)[7] |= ((pFrom)->m_nReserved1 >> 1) &0xFF;
#endif

  return SB_ZF_FAB_BM3200_PORT_RATE_ENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm3200BwPortRateEntry_Unpack(sbZfFabBm3200BwPortRateEntry_t *pToStruct,
                                    uint8_t *pFromData,
                                    uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32_t)  ((pFromData)[0] >> 2) & 0x3f;

  /* Unpack Member: m_nSpGroups */
  (pToStruct)->m_nSpGroups =  (uint32_t)  ((pFromData)[1] >> 5) & 0x07;
  (pToStruct)->m_nSpGroups |=  (uint32_t)  ((pFromData)[0] & 0x03) << 3;

  /* Unpack Member: m_nGroups */
  (pToStruct)->m_nGroups =  (uint32_t)  ((pFromData)[1] ) & 0x1f;

  /* Unpack Member: m_nGroup */
  (pToStruct)->m_nGroup =  (uint32_t)  (pFromData)[3] ;
  (pToStruct)->m_nGroup |=  (uint32_t)  (pFromData)[2] << 8;

  /* Unpack Member: m_nLineRate */
  (pToStruct)->m_nLineRate =  (uint32_t)  (pFromData)[7] ;
  (pToStruct)->m_nLineRate |=  (uint32_t)  (pFromData)[6] << 8;
  (pToStruct)->m_nLineRate |=  (uint32_t)  ((pFromData)[5] & 0x7f) << 16;

  /* Unpack Member: m_nReserved1 */
  (pToStruct)->m_nReserved1 =  (uint32_t)  ((pFromData)[5] >> 7) & 0x01;
  (pToStruct)->m_nReserved1 |=  (uint32_t)  (pFromData)[4] << 1;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32_t)  ((pFromData)[3] >> 2) & 0x3f;

  /* Unpack Member: m_nSpGroups */
  (pToStruct)->m_nSpGroups =  (uint32_t)  ((pFromData)[2] >> 5) & 0x07;
  (pToStruct)->m_nSpGroups |=  (uint32_t)  ((pFromData)[3] & 0x03) << 3;

  /* Unpack Member: m_nGroups */
  (pToStruct)->m_nGroups =  (uint32_t)  ((pFromData)[2] ) & 0x1f;

  /* Unpack Member: m_nGroup */
  (pToStruct)->m_nGroup =  (uint32_t)  (pFromData)[0] ;
  (pToStruct)->m_nGroup |=  (uint32_t)  (pFromData)[1] << 8;

  /* Unpack Member: m_nLineRate */
  (pToStruct)->m_nLineRate =  (uint32_t)  (pFromData)[4] ;
  (pToStruct)->m_nLineRate |=  (uint32_t)  (pFromData)[5] << 8;
  (pToStruct)->m_nLineRate |=  (uint32_t)  ((pFromData)[6] & 0x7f) << 16;

  /* Unpack Member: m_nReserved1 */
  (pToStruct)->m_nReserved1 =  (uint32_t)  ((pFromData)[6] >> 7) & 0x01;
  (pToStruct)->m_nReserved1 |=  (uint32_t)  (pFromData)[7] << 1;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm3200BwPortRateEntry_InitInstance(sbZfFabBm3200BwPortRateEntry_t *pFrame) {

  pFrame->m_nReserved =  (unsigned int)  0;
  pFrame->m_nSpGroups =  (unsigned int)  0;
  pFrame->m_nGroups =  (unsigned int)  0;
  pFrame->m_nGroup =  (unsigned int)  0;
  pFrame->m_nLineRate =  (unsigned int)  0;
  pFrame->m_nReserved1 =  (unsigned int)  0;

}
#ifdef SB_ZF_INCLUDE_CONSOLE
/*
 * $Id: sbZfFabBm3200BwPortRateEntry.c,v 1.1.48.4 2011/05/22 05:39:14 iakramov Exp $
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
#include "sbZfFabBm3200BwPortRateEntry.hx"



/* Print members in struct */
void
sbZfFabBm3200BwPortRateEntry_Print(sbZfFabBm3200BwPortRateEntry_t *pFromStruct) {
  SB_LOG("FabBm3200BwPortRateEntry:: reserved=0x%02x", (unsigned int)  pFromStruct->m_nReserved);
  SB_LOG(" sp_groups=0x%02x", (unsigned int)  pFromStruct->m_nSpGroups);
  SB_LOG(" groups=0x%02x", (unsigned int)  pFromStruct->m_nGroups);
  SB_LOG("\n");

  SB_LOG("FabBm3200BwPortRateEntry:: group=0x%04x", (unsigned int)  pFromStruct->m_nGroup);
  SB_LOG(" line_rate=0x%06x", (unsigned int)  pFromStruct->m_nLineRate);
  SB_LOG(" reserved1=0x%03x", (unsigned int)  pFromStruct->m_nReserved1);
  SB_LOG("\n");

}

/* SPrint members in struct */
int
sbZfFabBm3200BwPortRateEntry_SPrint(sbZfFabBm3200BwPortRateEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm3200BwPortRateEntry:: reserved=0x%02x", (unsigned int)  pFromStruct->m_nReserved);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," sp_groups=0x%02x", (unsigned int)  pFromStruct->m_nSpGroups);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," groups=0x%02x", (unsigned int)  pFromStruct->m_nGroups);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabBm3200BwPortRateEntry:: group=0x%04x", (unsigned int)  pFromStruct->m_nGroup);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," line_rate=0x%06x", (unsigned int)  pFromStruct->m_nLineRate);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," reserved1=0x%03x", (unsigned int)  pFromStruct->m_nReserved1);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(WrCnt);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabBm3200BwPortRateEntry_Validate(sbZfFabBm3200BwPortRateEntry_t *pZf) {

  if (pZf->m_nReserved > 0x3f) return 0;
  if (pZf->m_nSpGroups > 0x1f) return 0;
  if (pZf->m_nGroups > 0x1f) return 0;
  if (pZf->m_nGroup > 0xffff) return 0;
  if (pZf->m_nLineRate > 0x7fffff) return 0;
  if (pZf->m_nReserved1 > 0x1ff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabBm3200BwPortRateEntry_SetField(sbZfFabBm3200BwPortRateEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nreserved") == 0) {
    s->m_nReserved = value;
  } else if (SB_STRCMP(name, "m_nspgroups") == 0) {
    s->m_nSpGroups = value;
  } else if (SB_STRCMP(name, "m_ngroups") == 0) {
    s->m_nGroups = value;
  } else if (SB_STRCMP(name, "m_ngroup") == 0) {
    s->m_nGroup = value;
  } else if (SB_STRCMP(name, "m_nlinerate") == 0) {
    s->m_nLineRate = value;
  } else if (SB_STRCMP(name, "m_nreserved1") == 0) {
    s->m_nReserved1 = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}


#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */
