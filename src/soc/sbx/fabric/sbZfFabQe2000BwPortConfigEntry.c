/*
 * $Id: sbZfFabQe2000BwPortConfigEntry.c 1.1.48.4 Broadcom SDK $
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
#include "sbZfFabQe2000BwPortConfigEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfFabQe2000BwPortConfigEntry_Pack(sbZfFabQe2000BwPortConfigEntry_t *pFrom,
                                    uint8_t *pToData,
                                    uint32_t nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_QE2000_BW_PORT_CONFIG_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nLineRate */
  (pToData)[3] |= ((pFrom)->m_nLineRate) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nLineRate >> 8) &0xFF;
  (pToData)[1] |= ((pFrom)->m_nLineRate >> 16) & 0x3f;

  /* Pack Member: m_nBaseQueue */
  (pToData)[1] |= ((pFrom)->m_nBaseQueue & 0x03) <<6;
  (pToData)[0] |= ((pFrom)->m_nBaseQueue >> 2) &0xFF;
  (pToData)[7] |= ((pFrom)->m_nBaseQueue >> 10) & 0x0f;

  /* Pack Member: m_nQueues */
  (pToData)[7] |= ((pFrom)->m_nQueues & 0x0f) <<4;
  (pToData)[6] |= ((pFrom)->m_nQueues >> 4) & 0x01;

  /* Pack Member: m_nSpQueues */
  (pToData)[6] |= ((pFrom)->m_nSpQueues & 0x1f) <<1;

  /* Pack Member: m_nSpare */
  (pToData)[6] |= ((pFrom)->m_nSpare & 0x03) <<6;
  (pToData)[5] |= ((pFrom)->m_nSpare >> 2) &0xFF;
  (pToData)[4] |= ((pFrom)->m_nSpare >> 10) &0xFF;
#else
  int i;
  int size = SB_ZF_FAB_QE2000_BW_PORT_CONFIG_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nLineRate */
  (pToData)[0] |= ((pFrom)->m_nLineRate) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nLineRate >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nLineRate >> 16) & 0x3f;

  /* Pack Member: m_nBaseQueue */
  (pToData)[2] |= ((pFrom)->m_nBaseQueue & 0x03) <<6;
  (pToData)[3] |= ((pFrom)->m_nBaseQueue >> 2) &0xFF;
  (pToData)[4] |= ((pFrom)->m_nBaseQueue >> 10) & 0x0f;

  /* Pack Member: m_nQueues */
  (pToData)[4] |= ((pFrom)->m_nQueues & 0x0f) <<4;
  (pToData)[5] |= ((pFrom)->m_nQueues >> 4) & 0x01;

  /* Pack Member: m_nSpQueues */
  (pToData)[5] |= ((pFrom)->m_nSpQueues & 0x1f) <<1;

  /* Pack Member: m_nSpare */
  (pToData)[5] |= ((pFrom)->m_nSpare & 0x03) <<6;
  (pToData)[6] |= ((pFrom)->m_nSpare >> 2) &0xFF;
  (pToData)[7] |= ((pFrom)->m_nSpare >> 10) &0xFF;
#endif

  return SB_ZF_FAB_QE2000_BW_PORT_CONFIG_ENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabQe2000BwPortConfigEntry_Unpack(sbZfFabQe2000BwPortConfigEntry_t *pToStruct,
                                      uint8_t *pFromData,
                                      uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nLineRate */
  (pToStruct)->m_nLineRate =  (uint32_t)  (pFromData)[3] ;
  (pToStruct)->m_nLineRate |=  (uint32_t)  (pFromData)[2] << 8;
  (pToStruct)->m_nLineRate |=  (uint32_t)  ((pFromData)[1] & 0x3f) << 16;

  /* Unpack Member: m_nBaseQueue */
  (pToStruct)->m_nBaseQueue =  (uint32_t)  ((pFromData)[1] >> 6) & 0x03;
  (pToStruct)->m_nBaseQueue |=  (uint32_t)  (pFromData)[0] << 2;
  (pToStruct)->m_nBaseQueue |=  (uint32_t)  ((pFromData)[7] & 0x0f) << 10;

  /* Unpack Member: m_nQueues */
  (pToStruct)->m_nQueues =  (uint32_t)  ((pFromData)[7] >> 4) & 0x0f;
  (pToStruct)->m_nQueues |=  (uint32_t)  ((pFromData)[6] & 0x01) << 4;

  /* Unpack Member: m_nSpQueues */
  (pToStruct)->m_nSpQueues =  (uint32_t)  ((pFromData)[6] >> 1) & 0x1f;

  /* Unpack Member: m_nSpare */
  (pToStruct)->m_nSpare =  (uint32_t)  ((pFromData)[6] >> 6) & 0x03;
  (pToStruct)->m_nSpare |=  (uint32_t)  (pFromData)[5] << 2;
  (pToStruct)->m_nSpare |=  (uint32_t)  (pFromData)[4] << 10;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nLineRate */
  (pToStruct)->m_nLineRate =  (uint32_t)  (pFromData)[0] ;
  (pToStruct)->m_nLineRate |=  (uint32_t)  (pFromData)[1] << 8;
  (pToStruct)->m_nLineRate |=  (uint32_t)  ((pFromData)[2] & 0x3f) << 16;

  /* Unpack Member: m_nBaseQueue */
  (pToStruct)->m_nBaseQueue =  (uint32_t)  ((pFromData)[2] >> 6) & 0x03;
  (pToStruct)->m_nBaseQueue |=  (uint32_t)  (pFromData)[3] << 2;
  (pToStruct)->m_nBaseQueue |=  (uint32_t)  ((pFromData)[4] & 0x0f) << 10;

  /* Unpack Member: m_nQueues */
  (pToStruct)->m_nQueues =  (uint32_t)  ((pFromData)[4] >> 4) & 0x0f;
  (pToStruct)->m_nQueues |=  (uint32_t)  ((pFromData)[5] & 0x01) << 4;

  /* Unpack Member: m_nSpQueues */
  (pToStruct)->m_nSpQueues =  (uint32_t)  ((pFromData)[5] >> 1) & 0x1f;

  /* Unpack Member: m_nSpare */
  (pToStruct)->m_nSpare =  (uint32_t)  ((pFromData)[5] >> 6) & 0x03;
  (pToStruct)->m_nSpare |=  (uint32_t)  (pFromData)[6] << 2;
  (pToStruct)->m_nSpare |=  (uint32_t)  (pFromData)[7] << 10;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabQe2000BwPortConfigEntry_InitInstance(sbZfFabQe2000BwPortConfigEntry_t *pFrame) {

  pFrame->m_nLineRate =  (unsigned int)  0;
  pFrame->m_nBaseQueue =  (unsigned int)  0;
  pFrame->m_nQueues =  (unsigned int)  0;
  pFrame->m_nSpQueues =  (unsigned int)  0;
  pFrame->m_nSpare =  (unsigned int)  0;

}
#ifdef SB_ZF_INCLUDE_CONSOLE
/*
 * $Id: sbZfFabQe2000BwPortConfigEntry.c,v 1.1.48.4 2011/05/22 05:39:16 iakramov Exp $
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
#include "sbZfFabQe2000BwPortConfigEntry.hx"



/* Print members in struct */
void
sbZfFabQe2000BwPortConfigEntry_Print(sbZfFabQe2000BwPortConfigEntry_t *pFromStruct) {
  SB_LOG("FabQe2000BwPortConfigEntry:: linerate=0x%06x", (unsigned int)  pFromStruct->m_nLineRate);
  SB_LOG(" basequeue=0x%04x", (unsigned int)  pFromStruct->m_nBaseQueue);
  SB_LOG(" queues=0x%02x", (unsigned int)  pFromStruct->m_nQueues);
  SB_LOG("\n");

  SB_LOG("FabQe2000BwPortConfigEntry:: sp_queues=0x%02x", (unsigned int)  pFromStruct->m_nSpQueues);
  SB_LOG(" spare=0x%05x", (unsigned int)  pFromStruct->m_nSpare);
  SB_LOG("\n");

}

/* SPrint members in struct */
int
sbZfFabQe2000BwPortConfigEntry_SPrint(sbZfFabQe2000BwPortConfigEntry_t *pFromStruct, char *pcToString, uint32_t lStrSize) {
  uint32_t WrCnt = 0x0;

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabQe2000BwPortConfigEntry:: linerate=0x%06x", (unsigned int)  pFromStruct->m_nLineRate);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," basequeue=0x%04x", (unsigned int)  pFromStruct->m_nBaseQueue);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," queues=0x%02x", (unsigned int)  pFromStruct->m_nQueues);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"FabQe2000BwPortConfigEntry:: sp_queues=0x%02x", (unsigned int)  pFromStruct->m_nSpQueues);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt]," spare=0x%05x", (unsigned int)  pFromStruct->m_nSpare);
  WrCnt += SB_SPRINTF(&pcToString[WrCnt],"\n");

  /* assert if we've overrun the buffer */
  SB_ASSERT(WrCnt < lStrSize);
  return(WrCnt);

}

/* validate members in struct (1 = PASS, 0 = FAIL) */
int
sbZfFabQe2000BwPortConfigEntry_Validate(sbZfFabQe2000BwPortConfigEntry_t *pZf) {

  if (pZf->m_nLineRate > 0x3fffff) return 0;
  if (pZf->m_nBaseQueue > 0x3fff) return 0;
  if (pZf->m_nQueues > 0x1f) return 0;
  if (pZf->m_nSpQueues > 0x1f) return 0;
  if (pZf->m_nSpare > 0x3ffff) return 0;

  return 1; /* success */

}

/* populate field from string and value */
int
sbZfFabQe2000BwPortConfigEntry_SetField(sbZfFabQe2000BwPortConfigEntry_t *s, char* name, int value) {

  if (SB_STRCMP(name, "") == 0 ) {
    return -1;
  } else if (SB_STRCMP(name, "m_nlinerate") == 0) {
    s->m_nLineRate = value;
  } else if (SB_STRCMP(name, "m_nbasequeue") == 0) {
    s->m_nBaseQueue = value;
  } else if (SB_STRCMP(name, "m_nqueues") == 0) {
    s->m_nQueues = value;
  } else if (SB_STRCMP(name, "m_nspqueues") == 0) {
    s->m_nSpQueues = value;
  } else if (SB_STRCMP(name, "m_nspare") == 0) {
    s->m_nSpare = value;
  } else {
    /* string failed to match any field--ignored */
    return -1;
  }

  return(0);

}


#endif /* ifdef SB_ZF_INCLUDE_CONSOLE */
