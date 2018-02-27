/*
 * $Id: sbZfKaQsQueueTableEntry.c 1.1.44.4 Broadcom SDK $
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
#include "sbZfKaQsQueueTableEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfKaQsQueueTableEntry_Pack(sbZfKaQsQueueTableEntry_t *pFrom,
                             uint8_t *pToData,
                             uint32_t nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAQSQUEUETABLEENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nCredit */
  (pToData)[7] |= ((pFrom)->m_nCredit & 0x01) <<7;
  (pToData)[6] |= ((pFrom)->m_nCredit >> 1) &0xFF;
  (pToData)[5] |= ((pFrom)->m_nCredit >> 9) &0xFF;
  (pToData)[4] |= ((pFrom)->m_nCredit >> 17) &0xFF;

  /* Pack Member: m_nHpLen */
  (pToData)[7] |= ((pFrom)->m_nHpLen & 0x03) <<5;

  /* Pack Member: m_nDepth */
  (pToData)[7] |= ((pFrom)->m_nDepth & 0x0f) <<1;

  /* Pack Member: m_nQ2Ec */
  (pToData)[1] |= ((pFrom)->m_nQ2Ec) & 0xFF;
  (pToData)[0] |= ((pFrom)->m_nQ2Ec >> 8) &0xFF;
  (pToData)[7] |= ((pFrom)->m_nQ2Ec >> 16) & 0x01;

  /* Pack Member: m_nLocalQ */
  (pToData)[2] |= ((pFrom)->m_nLocalQ & 0x01) <<7;

  /* Pack Member: m_nMaxHoldTs */
  (pToData)[2] |= ((pFrom)->m_nMaxHoldTs & 0x07) <<4;

  /* Pack Member: m_nQueueType */
  (pToData)[2] |= ((pFrom)->m_nQueueType & 0x0f);

  /* Pack Member: m_nShapeRateMSB */
  (pToData)[3] |= ((pFrom)->m_nShapeRateMSB) & 0xFF;
#else
  int i;
  int size = SB_ZF_ZFKAQSQUEUETABLEENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nCredit */
  (pToData)[4] |= ((pFrom)->m_nCredit & 0x01) <<7;
  (pToData)[5] |= ((pFrom)->m_nCredit >> 1) &0xFF;
  (pToData)[6] |= ((pFrom)->m_nCredit >> 9) &0xFF;
  (pToData)[7] |= ((pFrom)->m_nCredit >> 17) &0xFF;

  /* Pack Member: m_nHpLen */
  (pToData)[4] |= ((pFrom)->m_nHpLen & 0x03) <<5;

  /* Pack Member: m_nDepth */
  (pToData)[4] |= ((pFrom)->m_nDepth & 0x0f) <<1;

  /* Pack Member: m_nQ2Ec */
  (pToData)[2] |= ((pFrom)->m_nQ2Ec) & 0xFF;
  (pToData)[3] |= ((pFrom)->m_nQ2Ec >> 8) &0xFF;
  (pToData)[4] |= ((pFrom)->m_nQ2Ec >> 16) & 0x01;

  /* Pack Member: m_nLocalQ */
  (pToData)[1] |= ((pFrom)->m_nLocalQ & 0x01) <<7;

  /* Pack Member: m_nMaxHoldTs */
  (pToData)[1] |= ((pFrom)->m_nMaxHoldTs & 0x07) <<4;

  /* Pack Member: m_nQueueType */
  (pToData)[1] |= ((pFrom)->m_nQueueType & 0x0f);

  /* Pack Member: m_nShapeRateMSB */
  (pToData)[0] |= ((pFrom)->m_nShapeRateMSB) & 0xFF;
#endif

  return SB_ZF_ZFKAQSQUEUETABLEENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaQsQueueTableEntry_Unpack(sbZfKaQsQueueTableEntry_t *pToStruct,
                               uint8_t *pFromData,
                               uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nCredit */
  (pToStruct)->m_nCredit =  (uint32_t)  ((pFromData)[7] >> 7) & 0x01;
  (pToStruct)->m_nCredit |=  (uint32_t)  (pFromData)[6] << 1;
  (pToStruct)->m_nCredit |=  (uint32_t)  (pFromData)[5] << 9;
  (pToStruct)->m_nCredit |=  (uint32_t)  (pFromData)[4] << 17;

  /* Unpack Member: m_nHpLen */
  (pToStruct)->m_nHpLen =  (uint32_t)  ((pFromData)[7] >> 5) & 0x03;

  /* Unpack Member: m_nDepth */
  (pToStruct)->m_nDepth =  (uint32_t)  ((pFromData)[7] >> 1) & 0x0f;

  /* Unpack Member: m_nQ2Ec */
  (pToStruct)->m_nQ2Ec =  (uint32_t)  (pFromData)[1] ;
  (pToStruct)->m_nQ2Ec |=  (uint32_t)  (pFromData)[0] << 8;
  (pToStruct)->m_nQ2Ec |=  (uint32_t)  ((pFromData)[7] & 0x01) << 16;

  /* Unpack Member: m_nLocalQ */
  (pToStruct)->m_nLocalQ =  (uint32_t)  ((pFromData)[2] >> 7) & 0x01;

  /* Unpack Member: m_nMaxHoldTs */
  (pToStruct)->m_nMaxHoldTs =  (uint32_t)  ((pFromData)[2] >> 4) & 0x07;

  /* Unpack Member: m_nQueueType */
  (pToStruct)->m_nQueueType =  (uint32_t)  ((pFromData)[2] ) & 0x0f;

  /* Unpack Member: m_nShapeRateMSB */
  (pToStruct)->m_nShapeRateMSB =  (uint32_t)  (pFromData)[3] ;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nCredit */
  (pToStruct)->m_nCredit =  (uint32_t)  ((pFromData)[4] >> 7) & 0x01;
  (pToStruct)->m_nCredit |=  (uint32_t)  (pFromData)[5] << 1;
  (pToStruct)->m_nCredit |=  (uint32_t)  (pFromData)[6] << 9;
  (pToStruct)->m_nCredit |=  (uint32_t)  (pFromData)[7] << 17;

  /* Unpack Member: m_nHpLen */
  (pToStruct)->m_nHpLen =  (uint32_t)  ((pFromData)[4] >> 5) & 0x03;

  /* Unpack Member: m_nDepth */
  (pToStruct)->m_nDepth =  (uint32_t)  ((pFromData)[4] >> 1) & 0x0f;

  /* Unpack Member: m_nQ2Ec */
  (pToStruct)->m_nQ2Ec =  (uint32_t)  (pFromData)[2] ;
  (pToStruct)->m_nQ2Ec |=  (uint32_t)  (pFromData)[3] << 8;
  (pToStruct)->m_nQ2Ec |=  (uint32_t)  ((pFromData)[4] & 0x01) << 16;

  /* Unpack Member: m_nLocalQ */
  (pToStruct)->m_nLocalQ =  (uint32_t)  ((pFromData)[1] >> 7) & 0x01;

  /* Unpack Member: m_nMaxHoldTs */
  (pToStruct)->m_nMaxHoldTs =  (uint32_t)  ((pFromData)[1] >> 4) & 0x07;

  /* Unpack Member: m_nQueueType */
  (pToStruct)->m_nQueueType =  (uint32_t)  ((pFromData)[1] ) & 0x0f;

  /* Unpack Member: m_nShapeRateMSB */
  (pToStruct)->m_nShapeRateMSB =  (uint32_t)  (pFromData)[0] ;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaQsQueueTableEntry_InitInstance(sbZfKaQsQueueTableEntry_t *pFrame) {

  pFrame->m_nCredit =  (unsigned int)  0;
  pFrame->m_nHpLen =  (unsigned int)  0;
  pFrame->m_nDepth =  (unsigned int)  0;
  pFrame->m_nQ2Ec =  (unsigned int)  0;
  pFrame->m_nLocalQ =  (unsigned int)  0;
  pFrame->m_nMaxHoldTs =  (unsigned int)  0;
  pFrame->m_nQueueType =  (unsigned int)  0;
  pFrame->m_nShapeRateMSB =  (unsigned int)  0;

}
