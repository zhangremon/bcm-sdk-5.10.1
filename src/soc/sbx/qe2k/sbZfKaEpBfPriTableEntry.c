/*
 * $Id: sbZfKaEpBfPriTableEntry.c 1.1.44.4 Broadcom SDK $
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
#include "sbZfKaEpBfPriTableAddr.hx"
#include "sbZfKaEpBfPriTableEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfKaEpBfPriTableEntry_Pack(sbZfKaEpBfPriTableEntry_t *pFrom,
                             uint8_t *pToData,
                             uint32_t nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAEPBFPRITABLEENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nPri7 */
  (pToData)[1] |= ((pFrom)->m_nPri7 & 0x07) <<5;

  /* Pack Member: m_nPri6 */
  (pToData)[1] |= ((pFrom)->m_nPri6 & 0x07) <<2;

  /* Pack Member: m_nPri5 */
  (pToData)[2] |= ((pFrom)->m_nPri5 & 0x01) <<7;
  (pToData)[1] |= ((pFrom)->m_nPri5 >> 1) & 0x03;

  /* Pack Member: m_nPri4 */
  (pToData)[2] |= ((pFrom)->m_nPri4 & 0x07) <<4;

  /* Pack Member: m_nPri3 */
  (pToData)[2] |= ((pFrom)->m_nPri3 & 0x07) <<1;

  /* Pack Member: m_nPri2 */
  (pToData)[3] |= ((pFrom)->m_nPri2 & 0x03) <<6;
  (pToData)[2] |= ((pFrom)->m_nPri2 >> 2) & 0x01;

  /* Pack Member: m_nPri1 */
  (pToData)[3] |= ((pFrom)->m_nPri1 & 0x07) <<3;

  /* Pack Member: m_nPri0 */
  (pToData)[3] |= ((pFrom)->m_nPri0 & 0x07);
#else
  int i;
  int size = SB_ZF_ZFKAEPBFPRITABLEENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nPri7 */
  (pToData)[2] |= ((pFrom)->m_nPri7 & 0x07) <<5;

  /* Pack Member: m_nPri6 */
  (pToData)[2] |= ((pFrom)->m_nPri6 & 0x07) <<2;

  /* Pack Member: m_nPri5 */
  (pToData)[1] |= ((pFrom)->m_nPri5 & 0x01) <<7;
  (pToData)[2] |= ((pFrom)->m_nPri5 >> 1) & 0x03;

  /* Pack Member: m_nPri4 */
  (pToData)[1] |= ((pFrom)->m_nPri4 & 0x07) <<4;

  /* Pack Member: m_nPri3 */
  (pToData)[1] |= ((pFrom)->m_nPri3 & 0x07) <<1;

  /* Pack Member: m_nPri2 */
  (pToData)[0] |= ((pFrom)->m_nPri2 & 0x03) <<6;
  (pToData)[1] |= ((pFrom)->m_nPri2 >> 2) & 0x01;

  /* Pack Member: m_nPri1 */
  (pToData)[0] |= ((pFrom)->m_nPri1 & 0x07) <<3;

  /* Pack Member: m_nPri0 */
  (pToData)[0] |= ((pFrom)->m_nPri0 & 0x07);
#endif

  return SB_ZF_ZFKAEPBFPRITABLEENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaEpBfPriTableEntry_Unpack(sbZfKaEpBfPriTableEntry_t *pToStruct,
                               uint8_t *pFromData,
                               uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nPri7 */
  (pToStruct)->m_nPri7 =  (uint32_t)  ((pFromData)[1] >> 5) & 0x07;

  /* Unpack Member: m_nPri6 */
  (pToStruct)->m_nPri6 =  (uint32_t)  ((pFromData)[1] >> 2) & 0x07;

  /* Unpack Member: m_nPri5 */
  (pToStruct)->m_nPri5 =  (uint32_t)  ((pFromData)[2] >> 7) & 0x01;
  (pToStruct)->m_nPri5 |=  (uint32_t)  ((pFromData)[1] & 0x03) << 1;

  /* Unpack Member: m_nPri4 */
  (pToStruct)->m_nPri4 =  (uint32_t)  ((pFromData)[2] >> 4) & 0x07;

  /* Unpack Member: m_nPri3 */
  (pToStruct)->m_nPri3 =  (uint32_t)  ((pFromData)[2] >> 1) & 0x07;

  /* Unpack Member: m_nPri2 */
  (pToStruct)->m_nPri2 =  (uint32_t)  ((pFromData)[3] >> 6) & 0x03;
  (pToStruct)->m_nPri2 |=  (uint32_t)  ((pFromData)[2] & 0x01) << 2;

  /* Unpack Member: m_nPri1 */
  (pToStruct)->m_nPri1 =  (uint32_t)  ((pFromData)[3] >> 3) & 0x07;

  /* Unpack Member: m_nPri0 */
  (pToStruct)->m_nPri0 =  (uint32_t)  ((pFromData)[3] ) & 0x07;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nPri7 */
  (pToStruct)->m_nPri7 =  (uint32_t)  ((pFromData)[2] >> 5) & 0x07;

  /* Unpack Member: m_nPri6 */
  (pToStruct)->m_nPri6 =  (uint32_t)  ((pFromData)[2] >> 2) & 0x07;

  /* Unpack Member: m_nPri5 */
  (pToStruct)->m_nPri5 =  (uint32_t)  ((pFromData)[1] >> 7) & 0x01;
  (pToStruct)->m_nPri5 |=  (uint32_t)  ((pFromData)[2] & 0x03) << 1;

  /* Unpack Member: m_nPri4 */
  (pToStruct)->m_nPri4 =  (uint32_t)  ((pFromData)[1] >> 4) & 0x07;

  /* Unpack Member: m_nPri3 */
  (pToStruct)->m_nPri3 =  (uint32_t)  ((pFromData)[1] >> 1) & 0x07;

  /* Unpack Member: m_nPri2 */
  (pToStruct)->m_nPri2 =  (uint32_t)  ((pFromData)[0] >> 6) & 0x03;
  (pToStruct)->m_nPri2 |=  (uint32_t)  ((pFromData)[1] & 0x01) << 2;

  /* Unpack Member: m_nPri1 */
  (pToStruct)->m_nPri1 =  (uint32_t)  ((pFromData)[0] >> 3) & 0x07;

  /* Unpack Member: m_nPri0 */
  (pToStruct)->m_nPri0 =  (uint32_t)  ((pFromData)[0] ) & 0x07;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaEpBfPriTableEntry_InitInstance(sbZfKaEpBfPriTableEntry_t *pFrame) {

  pFrame->m_nPri7 =  (unsigned int)  0;
  pFrame->m_nPri6 =  (unsigned int)  0;
  pFrame->m_nPri5 =  (unsigned int)  0;
  pFrame->m_nPri4 =  (unsigned int)  0;
  pFrame->m_nPri3 =  (unsigned int)  0;
  pFrame->m_nPri2 =  (unsigned int)  0;
  pFrame->m_nPri1 =  (unsigned int)  0;
  pFrame->m_nPri0 =  (unsigned int)  0;

}
