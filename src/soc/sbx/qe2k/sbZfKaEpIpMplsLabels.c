/*
 * $Id: sbZfKaEpIpMplsLabels.c 1.1.44.4 Broadcom SDK $
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
#include "sbZfKaEpIpMplsLabels.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfKaEpIpMplsLabels_Pack(sbZfKaEpIpMplsLabels_t *pFrom,
                          uint8_t *pToData,
                          uint32_t nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAEPIPMPLSLABELS_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nLabel1 */
  (pToData)[6] |= ((pFrom)->m_nLabel1 & 0x0f) <<4;
  (pToData)[5] |= ((pFrom)->m_nLabel1 >> 4) &0xFF;
  (pToData)[4] |= ((pFrom)->m_nLabel1 >> 12) &0xFF;

  /* Pack Member: m_nOp */
  (pToData)[6] |= ((pFrom)->m_nOp & 0x01) <<3;

  /* Pack Member: m_nLink */
  (pToData)[6] |= ((pFrom)->m_nLink & 0x03) <<1;

  /* Pack Member: m_nStack1 */
  (pToData)[6] |= ((pFrom)->m_nStack1 & 0x01);

  /* Pack Member: m_nTttl1 */
  (pToData)[7] |= ((pFrom)->m_nTttl1) & 0xFF;

  /* Pack Member: m_nLabel0 */
  (pToData)[2] |= ((pFrom)->m_nLabel0 & 0x0f) <<4;
  (pToData)[1] |= ((pFrom)->m_nLabel0 >> 4) &0xFF;
  (pToData)[0] |= ((pFrom)->m_nLabel0 >> 12) &0xFF;

  /* Pack Member: m_nExp */
  (pToData)[2] |= ((pFrom)->m_nExp & 0x07) <<1;

  /* Pack Member: m_nStack0 */
  (pToData)[2] |= ((pFrom)->m_nStack0 & 0x01);

  /* Pack Member: m_nTttl0 */
  (pToData)[3] |= ((pFrom)->m_nTttl0) & 0xFF;
#else
  int i;
  int size = SB_ZF_ZFKAEPIPMPLSLABELS_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nLabel1 */
  (pToData)[5] |= ((pFrom)->m_nLabel1 & 0x0f) <<4;
  (pToData)[6] |= ((pFrom)->m_nLabel1 >> 4) &0xFF;
  (pToData)[7] |= ((pFrom)->m_nLabel1 >> 12) &0xFF;

  /* Pack Member: m_nOp */
  (pToData)[5] |= ((pFrom)->m_nOp & 0x01) <<3;

  /* Pack Member: m_nLink */
  (pToData)[5] |= ((pFrom)->m_nLink & 0x03) <<1;

  /* Pack Member: m_nStack1 */
  (pToData)[5] |= ((pFrom)->m_nStack1 & 0x01);

  /* Pack Member: m_nTttl1 */
  (pToData)[4] |= ((pFrom)->m_nTttl1) & 0xFF;

  /* Pack Member: m_nLabel0 */
  (pToData)[1] |= ((pFrom)->m_nLabel0 & 0x0f) <<4;
  (pToData)[2] |= ((pFrom)->m_nLabel0 >> 4) &0xFF;
  (pToData)[3] |= ((pFrom)->m_nLabel0 >> 12) &0xFF;

  /* Pack Member: m_nExp */
  (pToData)[1] |= ((pFrom)->m_nExp & 0x07) <<1;

  /* Pack Member: m_nStack0 */
  (pToData)[1] |= ((pFrom)->m_nStack0 & 0x01);

  /* Pack Member: m_nTttl0 */
  (pToData)[0] |= ((pFrom)->m_nTttl0) & 0xFF;
#endif

  return SB_ZF_ZFKAEPIPMPLSLABELS_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaEpIpMplsLabels_Unpack(sbZfKaEpIpMplsLabels_t *pToStruct,
                            uint8_t *pFromData,
                            uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nLabel1 */
  (pToStruct)->m_nLabel1 =  (uint32_t)  ((pFromData)[6] >> 4) & 0x0f;
  (pToStruct)->m_nLabel1 |=  (uint32_t)  (pFromData)[5] << 4;
  (pToStruct)->m_nLabel1 |=  (uint32_t)  (pFromData)[4] << 12;

  /* Unpack Member: m_nOp */
  (pToStruct)->m_nOp =  (uint32_t)  ((pFromData)[6] >> 3) & 0x01;

  /* Unpack Member: m_nLink */
  (pToStruct)->m_nLink =  (uint32_t)  ((pFromData)[6] >> 1) & 0x03;

  /* Unpack Member: m_nStack1 */
  (pToStruct)->m_nStack1 =  (uint32_t)  ((pFromData)[6] ) & 0x01;

  /* Unpack Member: m_nTttl1 */
  (pToStruct)->m_nTttl1 =  (uint32_t)  (pFromData)[7] ;

  /* Unpack Member: m_nLabel0 */
  (pToStruct)->m_nLabel0 =  (uint32_t)  ((pFromData)[2] >> 4) & 0x0f;
  (pToStruct)->m_nLabel0 |=  (uint32_t)  (pFromData)[1] << 4;
  (pToStruct)->m_nLabel0 |=  (uint32_t)  (pFromData)[0] << 12;

  /* Unpack Member: m_nExp */
  (pToStruct)->m_nExp =  (uint32_t)  ((pFromData)[2] >> 1) & 0x07;

  /* Unpack Member: m_nStack0 */
  (pToStruct)->m_nStack0 =  (uint32_t)  ((pFromData)[2] ) & 0x01;

  /* Unpack Member: m_nTttl0 */
  (pToStruct)->m_nTttl0 =  (uint32_t)  (pFromData)[3] ;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nLabel1 */
  (pToStruct)->m_nLabel1 =  (uint32_t)  ((pFromData)[5] >> 4) & 0x0f;
  (pToStruct)->m_nLabel1 |=  (uint32_t)  (pFromData)[6] << 4;
  (pToStruct)->m_nLabel1 |=  (uint32_t)  (pFromData)[7] << 12;

  /* Unpack Member: m_nOp */
  (pToStruct)->m_nOp =  (uint32_t)  ((pFromData)[5] >> 3) & 0x01;

  /* Unpack Member: m_nLink */
  (pToStruct)->m_nLink =  (uint32_t)  ((pFromData)[5] >> 1) & 0x03;

  /* Unpack Member: m_nStack1 */
  (pToStruct)->m_nStack1 =  (uint32_t)  ((pFromData)[5] ) & 0x01;

  /* Unpack Member: m_nTttl1 */
  (pToStruct)->m_nTttl1 =  (uint32_t)  (pFromData)[4] ;

  /* Unpack Member: m_nLabel0 */
  (pToStruct)->m_nLabel0 =  (uint32_t)  ((pFromData)[1] >> 4) & 0x0f;
  (pToStruct)->m_nLabel0 |=  (uint32_t)  (pFromData)[2] << 4;
  (pToStruct)->m_nLabel0 |=  (uint32_t)  (pFromData)[3] << 12;

  /* Unpack Member: m_nExp */
  (pToStruct)->m_nExp =  (uint32_t)  ((pFromData)[1] >> 1) & 0x07;

  /* Unpack Member: m_nStack0 */
  (pToStruct)->m_nStack0 =  (uint32_t)  ((pFromData)[1] ) & 0x01;

  /* Unpack Member: m_nTttl0 */
  (pToStruct)->m_nTttl0 =  (uint32_t)  (pFromData)[0] ;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaEpIpMplsLabels_InitInstance(sbZfKaEpIpMplsLabels_t *pFrame) {

  pFrame->m_nLabel1 =  (unsigned int)  0;
  pFrame->m_nOp =  (unsigned int)  0;
  pFrame->m_nLink =  (unsigned int)  0;
  pFrame->m_nStack1 =  (unsigned int)  0;
  pFrame->m_nTttl1 =  (unsigned int)  0;
  pFrame->m_nLabel0 =  (unsigned int)  0;
  pFrame->m_nExp =  (unsigned int)  0;
  pFrame->m_nStack0 =  (unsigned int)  0;
  pFrame->m_nTttl0 =  (unsigned int)  0;

}
