/*
 * $Id: sbZfCaPpAggregateHashByteConfigEntry.c,v 1.1.48.4 2011/05/22 05:39:26 iakramov Exp $
 * $Copyright (c) 2011 Broadcom Corporation
 * All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfCaPpAggregateHashByteConfigEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfCaPpAggregateHashByteConfigEntry_Pack(sbZfCaPpAggregateHashByteConfigEntry_t *pFrom,
                                          uint8_t *pToData,
                                          uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_CAPPAGGREGATEHASHBYTECONFIGENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uData1 */
  (pToData)[4] |= ((pFrom)->m_uData1) & 0xFF;

  /* Pack Member: m_uData0 */
  (pToData)[0] |= ((pFrom)->m_uData0) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_uData0 >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_uData0 >> 16) &0xFF;
  (pToData)[3] |= ((pFrom)->m_uData0 >> 24) &0xFF;

  return SB_ZF_CAPPAGGREGATEHASHBYTECONFIGENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfCaPpAggregateHashByteConfigEntry_Unpack(sbZfCaPpAggregateHashByteConfigEntry_t *pToStruct,
                                            uint8_t *pFromData,
                                            uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: m_uData1 */
  (pToStruct)->m_uData1 =  (uint32_t)  (pFromData)[4] ;

  /* Unpack Member: m_uData0 */
  (pToStruct)->m_uData0 =  (uint32_t)  (pFromData)[0] ;
  (pToStruct)->m_uData0 |=  (uint32_t)  (pFromData)[1] << 8;
  (pToStruct)->m_uData0 |=  (uint32_t)  (pFromData)[2] << 16;
  (pToStruct)->m_uData0 |=  (uint32_t)  (pFromData)[3] << 24;

}



/* initialize an instance of this zframe */
void
sbZfCaPpAggregateHashByteConfigEntry_InitInstance(sbZfCaPpAggregateHashByteConfigEntry_t *pFrame) {

  pFrame->m_uData1 =  (unsigned int)  0;
  pFrame->m_uData0 =  (unsigned int)  0;

}
