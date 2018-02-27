/*
 * $Id: sbZfCaPpAggregateHashFormat.c,v 1.1.48.4 2011/05/22 05:39:26 iakramov Exp $
 * $Copyright (c) 2011 Broadcom Corporation
 * All rights reserved.$
 */


#include "sbTypes.h"
#include "sbZfCaPpAggregateHashFormat.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfCaPpAggregateHashFormat_Pack(sbZfCaPpAggregateHashFormat_t *pFrom,
                                 uint8_t *pToData,
                                 uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_ZFCAPPAGGREGATEHASHFORMAT_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uAggregateHash_31_28 */
  (pToData)[3] |= ((pFrom)->m_uAggregateHash_31_28 & 0x0f) <<4;

  /* Pack Member: m_uAggregateHash_27_16 */
  (pToData)[2] |= ((pFrom)->m_uAggregateHash_27_16) & 0xFF;
  (pToData)[3] |= ((pFrom)->m_uAggregateHash_27_16 >> 8) & 0x0f;

  /* Pack Member: m_uAggregateHash_15_0 */
  (pToData)[0] |= ((pFrom)->m_uAggregateHash_15_0) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_uAggregateHash_15_0 >> 8) &0xFF;

  return SB_ZF_ZFCAPPAGGREGATEHASHFORMAT_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfCaPpAggregateHashFormat_Unpack(sbZfCaPpAggregateHashFormat_t *pToStruct,
                                   uint8_t *pFromData,
                                   uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: m_uAggregateHash_31_28 */
  (pToStruct)->m_uAggregateHash_31_28 =  (uint32_t)  ((pFromData)[3] >> 4) & 0x0f;

  /* Unpack Member: m_uAggregateHash_27_16 */
  (pToStruct)->m_uAggregateHash_27_16 =  (uint32_t)  (pFromData)[2] ;
  (pToStruct)->m_uAggregateHash_27_16 |=  (uint32_t)  ((pFromData)[3] & 0x0f) << 8;

  /* Unpack Member: m_uAggregateHash_15_0 */
  (pToStruct)->m_uAggregateHash_15_0 =  (uint32_t)  (pFromData)[0] ;
  (pToStruct)->m_uAggregateHash_15_0 |=  (uint32_t)  (pFromData)[1] << 8;

}



/* initialize an instance of this zframe */
void
sbZfCaPpAggregateHashFormat_InitInstance(sbZfCaPpAggregateHashFormat_t *pFrame) {

  pFrame->m_uAggregateHash_31_28 =  (unsigned int)  0;
  pFrame->m_uAggregateHash_27_16 =  (unsigned int)  0;
  pFrame->m_uAggregateHash_15_0 =  (unsigned int)  0;

}
