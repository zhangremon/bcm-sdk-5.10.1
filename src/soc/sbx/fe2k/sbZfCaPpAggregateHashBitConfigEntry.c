/*
 * $Id: sbZfCaPpAggregateHashBitConfigEntry.c,v 1.1.48.4 2011/05/22 05:39:26 iakramov Exp $
 * $Copyright (c) 2011 Broadcom Corporation
 * All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfCaPpAggregateHashBitConfigEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfCaPpAggregateHashBitConfigEntry_Pack(sbZfCaPpAggregateHashBitConfigEntry_t *pFrom,
                                         uint8_t *pToData,
                                         uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_CAPPAGGREGATEHASHBITCONFIGENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uData9 */
  (pToData)[36] |= ((pFrom)->m_uData9) & 0xFF;
  (pToData)[37] |= ((pFrom)->m_uData9 >> 8) &0xFF;
  (pToData)[38] |= ((pFrom)->m_uData9 >> 16) &0xFF;
  (pToData)[39] |= ((pFrom)->m_uData9 >> 24) &0xFF;

  /* Pack Member: m_uData8 */
  (pToData)[32] |= ((pFrom)->m_uData8) & 0xFF;
  (pToData)[33] |= ((pFrom)->m_uData8 >> 8) &0xFF;
  (pToData)[34] |= ((pFrom)->m_uData8 >> 16) &0xFF;
  (pToData)[35] |= ((pFrom)->m_uData8 >> 24) &0xFF;

  /* Pack Member: m_uData7 */
  (pToData)[28] |= ((pFrom)->m_uData7) & 0xFF;
  (pToData)[29] |= ((pFrom)->m_uData7 >> 8) &0xFF;
  (pToData)[30] |= ((pFrom)->m_uData7 >> 16) &0xFF;
  (pToData)[31] |= ((pFrom)->m_uData7 >> 24) &0xFF;

  /* Pack Member: m_uData6 */
  (pToData)[24] |= ((pFrom)->m_uData6) & 0xFF;
  (pToData)[25] |= ((pFrom)->m_uData6 >> 8) &0xFF;
  (pToData)[26] |= ((pFrom)->m_uData6 >> 16) &0xFF;
  (pToData)[27] |= ((pFrom)->m_uData6 >> 24) &0xFF;

  /* Pack Member: m_uData5 */
  (pToData)[20] |= ((pFrom)->m_uData5) & 0xFF;
  (pToData)[21] |= ((pFrom)->m_uData5 >> 8) &0xFF;
  (pToData)[22] |= ((pFrom)->m_uData5 >> 16) &0xFF;
  (pToData)[23] |= ((pFrom)->m_uData5 >> 24) &0xFF;

  /* Pack Member: m_uData4 */
  (pToData)[16] |= ((pFrom)->m_uData4) & 0xFF;
  (pToData)[17] |= ((pFrom)->m_uData4 >> 8) &0xFF;
  (pToData)[18] |= ((pFrom)->m_uData4 >> 16) &0xFF;
  (pToData)[19] |= ((pFrom)->m_uData4 >> 24) &0xFF;

  /* Pack Member: m_uData3 */
  (pToData)[12] |= ((pFrom)->m_uData3) & 0xFF;
  (pToData)[13] |= ((pFrom)->m_uData3 >> 8) &0xFF;
  (pToData)[14] |= ((pFrom)->m_uData3 >> 16) &0xFF;
  (pToData)[15] |= ((pFrom)->m_uData3 >> 24) &0xFF;

  /* Pack Member: m_uData2 */
  (pToData)[8] |= ((pFrom)->m_uData2) & 0xFF;
  (pToData)[9] |= ((pFrom)->m_uData2 >> 8) &0xFF;
  (pToData)[10] |= ((pFrom)->m_uData2 >> 16) &0xFF;
  (pToData)[11] |= ((pFrom)->m_uData2 >> 24) &0xFF;

  /* Pack Member: m_uData1 */
  (pToData)[4] |= ((pFrom)->m_uData1) & 0xFF;
  (pToData)[5] |= ((pFrom)->m_uData1 >> 8) &0xFF;
  (pToData)[6] |= ((pFrom)->m_uData1 >> 16) &0xFF;
  (pToData)[7] |= ((pFrom)->m_uData1 >> 24) &0xFF;

  /* Pack Member: m_uData0 */
  (pToData)[0] |= ((pFrom)->m_uData0) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_uData0 >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_uData0 >> 16) &0xFF;
  (pToData)[3] |= ((pFrom)->m_uData0 >> 24) &0xFF;

  return SB_ZF_CAPPAGGREGATEHASHBITCONFIGENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfCaPpAggregateHashBitConfigEntry_Unpack(sbZfCaPpAggregateHashBitConfigEntry_t *pToStruct,
                                           uint8_t *pFromData,
                                           uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: m_uData9 */
  (pToStruct)->m_uData9 =  (uint32_t)  (pFromData)[36] ;
  (pToStruct)->m_uData9 |=  (uint32_t)  (pFromData)[37] << 8;
  (pToStruct)->m_uData9 |=  (uint32_t)  (pFromData)[38] << 16;
  (pToStruct)->m_uData9 |=  (uint32_t)  (pFromData)[39] << 24;

  /* Unpack Member: m_uData8 */
  (pToStruct)->m_uData8 =  (uint32_t)  (pFromData)[32] ;
  (pToStruct)->m_uData8 |=  (uint32_t)  (pFromData)[33] << 8;
  (pToStruct)->m_uData8 |=  (uint32_t)  (pFromData)[34] << 16;
  (pToStruct)->m_uData8 |=  (uint32_t)  (pFromData)[35] << 24;

  /* Unpack Member: m_uData7 */
  (pToStruct)->m_uData7 =  (uint32_t)  (pFromData)[28] ;
  (pToStruct)->m_uData7 |=  (uint32_t)  (pFromData)[29] << 8;
  (pToStruct)->m_uData7 |=  (uint32_t)  (pFromData)[30] << 16;
  (pToStruct)->m_uData7 |=  (uint32_t)  (pFromData)[31] << 24;

  /* Unpack Member: m_uData6 */
  (pToStruct)->m_uData6 =  (uint32_t)  (pFromData)[24] ;
  (pToStruct)->m_uData6 |=  (uint32_t)  (pFromData)[25] << 8;
  (pToStruct)->m_uData6 |=  (uint32_t)  (pFromData)[26] << 16;
  (pToStruct)->m_uData6 |=  (uint32_t)  (pFromData)[27] << 24;

  /* Unpack Member: m_uData5 */
  (pToStruct)->m_uData5 =  (uint32_t)  (pFromData)[20] ;
  (pToStruct)->m_uData5 |=  (uint32_t)  (pFromData)[21] << 8;
  (pToStruct)->m_uData5 |=  (uint32_t)  (pFromData)[22] << 16;
  (pToStruct)->m_uData5 |=  (uint32_t)  (pFromData)[23] << 24;

  /* Unpack Member: m_uData4 */
  (pToStruct)->m_uData4 =  (uint32_t)  (pFromData)[16] ;
  (pToStruct)->m_uData4 |=  (uint32_t)  (pFromData)[17] << 8;
  (pToStruct)->m_uData4 |=  (uint32_t)  (pFromData)[18] << 16;
  (pToStruct)->m_uData4 |=  (uint32_t)  (pFromData)[19] << 24;

  /* Unpack Member: m_uData3 */
  (pToStruct)->m_uData3 =  (uint32_t)  (pFromData)[12] ;
  (pToStruct)->m_uData3 |=  (uint32_t)  (pFromData)[13] << 8;
  (pToStruct)->m_uData3 |=  (uint32_t)  (pFromData)[14] << 16;
  (pToStruct)->m_uData3 |=  (uint32_t)  (pFromData)[15] << 24;

  /* Unpack Member: m_uData2 */
  (pToStruct)->m_uData2 =  (uint32_t)  (pFromData)[8] ;
  (pToStruct)->m_uData2 |=  (uint32_t)  (pFromData)[9] << 8;
  (pToStruct)->m_uData2 |=  (uint32_t)  (pFromData)[10] << 16;
  (pToStruct)->m_uData2 |=  (uint32_t)  (pFromData)[11] << 24;

  /* Unpack Member: m_uData1 */
  (pToStruct)->m_uData1 =  (uint32_t)  (pFromData)[4] ;
  (pToStruct)->m_uData1 |=  (uint32_t)  (pFromData)[5] << 8;
  (pToStruct)->m_uData1 |=  (uint32_t)  (pFromData)[6] << 16;
  (pToStruct)->m_uData1 |=  (uint32_t)  (pFromData)[7] << 24;

  /* Unpack Member: m_uData0 */
  (pToStruct)->m_uData0 =  (uint32_t)  (pFromData)[0] ;
  (pToStruct)->m_uData0 |=  (uint32_t)  (pFromData)[1] << 8;
  (pToStruct)->m_uData0 |=  (uint32_t)  (pFromData)[2] << 16;
  (pToStruct)->m_uData0 |=  (uint32_t)  (pFromData)[3] << 24;

}



/* initialize an instance of this zframe */
void
sbZfCaPpAggregateHashBitConfigEntry_InitInstance(sbZfCaPpAggregateHashBitConfigEntry_t *pFrame) {

  pFrame->m_uData9 =  (unsigned int)  0;
  pFrame->m_uData8 =  (unsigned int)  0;
  pFrame->m_uData7 =  (unsigned int)  0;
  pFrame->m_uData6 =  (unsigned int)  0;
  pFrame->m_uData5 =  (unsigned int)  0;
  pFrame->m_uData4 =  (unsigned int)  0;
  pFrame->m_uData3 =  (unsigned int)  0;
  pFrame->m_uData2 =  (unsigned int)  0;
  pFrame->m_uData1 =  (unsigned int)  0;
  pFrame->m_uData0 =  (unsigned int)  0;

}
