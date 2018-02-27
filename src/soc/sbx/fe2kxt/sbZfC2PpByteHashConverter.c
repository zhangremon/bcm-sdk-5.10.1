/*
 * $Id: sbZfC2PpByteHashConverter.c 1.1.34.4 Broadcom SDK $
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
#include "sbZfC2PpByteHashConverter.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfC2PpByteHashConverter_Pack(sbZfC2PpByteHashConverter_t *pFrom,
                               uint8_t *pToData,
                               uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_C2PPBYTEHASHCONVERTER_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uHashByteBit39 */
  (pToData)[4] |= ((pFrom)->m_uHashByteBit39 & 0x01) <<7;

  /* Pack Member: m_uHashByteBit38 */
  (pToData)[4] |= ((pFrom)->m_uHashByteBit38 & 0x01) <<6;

  /* Pack Member: m_uHashByteBit37 */
  (pToData)[4] |= ((pFrom)->m_uHashByteBit37 & 0x01) <<5;

  /* Pack Member: m_uHashByteBit36 */
  (pToData)[4] |= ((pFrom)->m_uHashByteBit36 & 0x01) <<4;

  /* Pack Member: m_uHashByteBit35 */
  (pToData)[4] |= ((pFrom)->m_uHashByteBit35 & 0x01) <<3;

  /* Pack Member: m_uHashByteBit34 */
  (pToData)[4] |= ((pFrom)->m_uHashByteBit34 & 0x01) <<2;

  /* Pack Member: m_uHashByteBit33 */
  (pToData)[4] |= ((pFrom)->m_uHashByteBit33 & 0x01) <<1;

  /* Pack Member: m_uHashByteBit32 */
  (pToData)[4] |= ((pFrom)->m_uHashByteBit32 & 0x01);

  /* Pack Member: m_uHashByteBit31 */
  (pToData)[3] |= ((pFrom)->m_uHashByteBit31 & 0x01) <<7;

  /* Pack Member: m_uHashByteBit30 */
  (pToData)[3] |= ((pFrom)->m_uHashByteBit30 & 0x01) <<6;

  /* Pack Member: m_uHashByteBit29 */
  (pToData)[3] |= ((pFrom)->m_uHashByteBit29 & 0x01) <<5;

  /* Pack Member: m_uHashByteBit28 */
  (pToData)[3] |= ((pFrom)->m_uHashByteBit28 & 0x01) <<4;

  /* Pack Member: m_uHashByteBit27 */
  (pToData)[3] |= ((pFrom)->m_uHashByteBit27 & 0x01) <<3;

  /* Pack Member: m_uHashByteBit26 */
  (pToData)[3] |= ((pFrom)->m_uHashByteBit26 & 0x01) <<2;

  /* Pack Member: m_uHashByteBit25 */
  (pToData)[3] |= ((pFrom)->m_uHashByteBit25 & 0x01) <<1;

  /* Pack Member: m_uHashByteBit24 */
  (pToData)[3] |= ((pFrom)->m_uHashByteBit24 & 0x01);

  /* Pack Member: m_uHashByteBit23 */
  (pToData)[2] |= ((pFrom)->m_uHashByteBit23 & 0x01) <<7;

  /* Pack Member: m_uHashByteBit22 */
  (pToData)[2] |= ((pFrom)->m_uHashByteBit22 & 0x01) <<6;

  /* Pack Member: m_uHashByteBit21 */
  (pToData)[2] |= ((pFrom)->m_uHashByteBit21 & 0x01) <<5;

  /* Pack Member: m_uHashByteBit20 */
  (pToData)[2] |= ((pFrom)->m_uHashByteBit20 & 0x01) <<4;

  /* Pack Member: m_uHashByteBit19 */
  (pToData)[2] |= ((pFrom)->m_uHashByteBit19 & 0x01) <<3;

  /* Pack Member: m_uHashByteBit18 */
  (pToData)[2] |= ((pFrom)->m_uHashByteBit18 & 0x01) <<2;

  /* Pack Member: m_uHashByteBit17 */
  (pToData)[2] |= ((pFrom)->m_uHashByteBit17 & 0x01) <<1;

  /* Pack Member: m_uHashByteBit16 */
  (pToData)[2] |= ((pFrom)->m_uHashByteBit16 & 0x01);

  /* Pack Member: m_uHashByteBit15 */
  (pToData)[1] |= ((pFrom)->m_uHashByteBit15 & 0x01) <<7;

  /* Pack Member: m_uHashByteBit14 */
  (pToData)[1] |= ((pFrom)->m_uHashByteBit14 & 0x01) <<6;

  /* Pack Member: m_uHashByteBit13 */
  (pToData)[1] |= ((pFrom)->m_uHashByteBit13 & 0x01) <<5;

  /* Pack Member: m_uHashByteBit12 */
  (pToData)[1] |= ((pFrom)->m_uHashByteBit12 & 0x01) <<4;

  /* Pack Member: m_uHashByteBit11 */
  (pToData)[1] |= ((pFrom)->m_uHashByteBit11 & 0x01) <<3;

  /* Pack Member: m_uHashByteBit10 */
  (pToData)[1] |= ((pFrom)->m_uHashByteBit10 & 0x01) <<2;

  /* Pack Member: m_uHashByteBit9 */
  (pToData)[1] |= ((pFrom)->m_uHashByteBit9 & 0x01) <<1;

  /* Pack Member: m_uHashByteBit8 */
  (pToData)[1] |= ((pFrom)->m_uHashByteBit8 & 0x01);

  /* Pack Member: m_uHashByteBit7 */
  (pToData)[0] |= ((pFrom)->m_uHashByteBit7 & 0x01) <<7;

  /* Pack Member: m_uHashByteBit6 */
  (pToData)[0] |= ((pFrom)->m_uHashByteBit6 & 0x01) <<6;

  /* Pack Member: m_uHashByteBit5 */
  (pToData)[0] |= ((pFrom)->m_uHashByteBit5 & 0x01) <<5;

  /* Pack Member: m_uHashByteBit4 */
  (pToData)[0] |= ((pFrom)->m_uHashByteBit4 & 0x01) <<4;

  /* Pack Member: m_uHashByteBit3 */
  (pToData)[0] |= ((pFrom)->m_uHashByteBit3 & 0x01) <<3;

  /* Pack Member: m_uHashByteBit2 */
  (pToData)[0] |= ((pFrom)->m_uHashByteBit2 & 0x01) <<2;

  /* Pack Member: m_uHashByteBit1 */
  (pToData)[0] |= ((pFrom)->m_uHashByteBit1 & 0x01) <<1;

  /* Pack Member: m_uHashByteBit0 */
  (pToData)[0] |= ((pFrom)->m_uHashByteBit0 & 0x01);

  return SB_ZF_C2PPBYTEHASHCONVERTER_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfC2PpByteHashConverter_Unpack(sbZfC2PpByteHashConverter_t *pToStruct,
                                 uint8_t *pFromData,
                                 uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: m_uHashByteBit39 */
  (pToStruct)->m_uHashByteBit39 =  (uint8_t)  ((pFromData)[4] >> 7) & 0x01;

  /* Unpack Member: m_uHashByteBit38 */
  (pToStruct)->m_uHashByteBit38 =  (uint8_t)  ((pFromData)[4] >> 6) & 0x01;

  /* Unpack Member: m_uHashByteBit37 */
  (pToStruct)->m_uHashByteBit37 =  (uint8_t)  ((pFromData)[4] >> 5) & 0x01;

  /* Unpack Member: m_uHashByteBit36 */
  (pToStruct)->m_uHashByteBit36 =  (uint8_t)  ((pFromData)[4] >> 4) & 0x01;

  /* Unpack Member: m_uHashByteBit35 */
  (pToStruct)->m_uHashByteBit35 =  (uint8_t)  ((pFromData)[4] >> 3) & 0x01;

  /* Unpack Member: m_uHashByteBit34 */
  (pToStruct)->m_uHashByteBit34 =  (uint8_t)  ((pFromData)[4] >> 2) & 0x01;

  /* Unpack Member: m_uHashByteBit33 */
  (pToStruct)->m_uHashByteBit33 =  (uint8_t)  ((pFromData)[4] >> 1) & 0x01;

  /* Unpack Member: m_uHashByteBit32 */
  (pToStruct)->m_uHashByteBit32 =  (uint8_t)  ((pFromData)[4] ) & 0x01;

  /* Unpack Member: m_uHashByteBit31 */
  (pToStruct)->m_uHashByteBit31 =  (uint8_t)  ((pFromData)[3] >> 7) & 0x01;

  /* Unpack Member: m_uHashByteBit30 */
  (pToStruct)->m_uHashByteBit30 =  (uint8_t)  ((pFromData)[3] >> 6) & 0x01;

  /* Unpack Member: m_uHashByteBit29 */
  (pToStruct)->m_uHashByteBit29 =  (uint8_t)  ((pFromData)[3] >> 5) & 0x01;

  /* Unpack Member: m_uHashByteBit28 */
  (pToStruct)->m_uHashByteBit28 =  (uint8_t)  ((pFromData)[3] >> 4) & 0x01;

  /* Unpack Member: m_uHashByteBit27 */
  (pToStruct)->m_uHashByteBit27 =  (uint8_t)  ((pFromData)[3] >> 3) & 0x01;

  /* Unpack Member: m_uHashByteBit26 */
  (pToStruct)->m_uHashByteBit26 =  (uint8_t)  ((pFromData)[3] >> 2) & 0x01;

  /* Unpack Member: m_uHashByteBit25 */
  (pToStruct)->m_uHashByteBit25 =  (uint8_t)  ((pFromData)[3] >> 1) & 0x01;

  /* Unpack Member: m_uHashByteBit24 */
  (pToStruct)->m_uHashByteBit24 =  (uint8_t)  ((pFromData)[3] ) & 0x01;

  /* Unpack Member: m_uHashByteBit23 */
  (pToStruct)->m_uHashByteBit23 =  (uint8_t)  ((pFromData)[2] >> 7) & 0x01;

  /* Unpack Member: m_uHashByteBit22 */
  (pToStruct)->m_uHashByteBit22 =  (uint8_t)  ((pFromData)[2] >> 6) & 0x01;

  /* Unpack Member: m_uHashByteBit21 */
  (pToStruct)->m_uHashByteBit21 =  (uint8_t)  ((pFromData)[2] >> 5) & 0x01;

  /* Unpack Member: m_uHashByteBit20 */
  (pToStruct)->m_uHashByteBit20 =  (uint8_t)  ((pFromData)[2] >> 4) & 0x01;

  /* Unpack Member: m_uHashByteBit19 */
  (pToStruct)->m_uHashByteBit19 =  (uint8_t)  ((pFromData)[2] >> 3) & 0x01;

  /* Unpack Member: m_uHashByteBit18 */
  (pToStruct)->m_uHashByteBit18 =  (uint8_t)  ((pFromData)[2] >> 2) & 0x01;

  /* Unpack Member: m_uHashByteBit17 */
  (pToStruct)->m_uHashByteBit17 =  (uint8_t)  ((pFromData)[2] >> 1) & 0x01;

  /* Unpack Member: m_uHashByteBit16 */
  (pToStruct)->m_uHashByteBit16 =  (uint8_t)  ((pFromData)[2] ) & 0x01;

  /* Unpack Member: m_uHashByteBit15 */
  (pToStruct)->m_uHashByteBit15 =  (uint8_t)  ((pFromData)[1] >> 7) & 0x01;

  /* Unpack Member: m_uHashByteBit14 */
  (pToStruct)->m_uHashByteBit14 =  (uint8_t)  ((pFromData)[1] >> 6) & 0x01;

  /* Unpack Member: m_uHashByteBit13 */
  (pToStruct)->m_uHashByteBit13 =  (uint8_t)  ((pFromData)[1] >> 5) & 0x01;

  /* Unpack Member: m_uHashByteBit12 */
  (pToStruct)->m_uHashByteBit12 =  (uint8_t)  ((pFromData)[1] >> 4) & 0x01;

  /* Unpack Member: m_uHashByteBit11 */
  (pToStruct)->m_uHashByteBit11 =  (uint8_t)  ((pFromData)[1] >> 3) & 0x01;

  /* Unpack Member: m_uHashByteBit10 */
  (pToStruct)->m_uHashByteBit10 =  (uint8_t)  ((pFromData)[1] >> 2) & 0x01;

  /* Unpack Member: m_uHashByteBit9 */
  (pToStruct)->m_uHashByteBit9 =  (uint8_t)  ((pFromData)[1] >> 1) & 0x01;

  /* Unpack Member: m_uHashByteBit8 */
  (pToStruct)->m_uHashByteBit8 =  (uint8_t)  ((pFromData)[1] ) & 0x01;

  /* Unpack Member: m_uHashByteBit7 */
  (pToStruct)->m_uHashByteBit7 =  (uint8_t)  ((pFromData)[0] >> 7) & 0x01;

  /* Unpack Member: m_uHashByteBit6 */
  (pToStruct)->m_uHashByteBit6 =  (uint8_t)  ((pFromData)[0] >> 6) & 0x01;

  /* Unpack Member: m_uHashByteBit5 */
  (pToStruct)->m_uHashByteBit5 =  (uint8_t)  ((pFromData)[0] >> 5) & 0x01;

  /* Unpack Member: m_uHashByteBit4 */
  (pToStruct)->m_uHashByteBit4 =  (uint8_t)  ((pFromData)[0] >> 4) & 0x01;

  /* Unpack Member: m_uHashByteBit3 */
  (pToStruct)->m_uHashByteBit3 =  (uint8_t)  ((pFromData)[0] >> 3) & 0x01;

  /* Unpack Member: m_uHashByteBit2 */
  (pToStruct)->m_uHashByteBit2 =  (uint8_t)  ((pFromData)[0] >> 2) & 0x01;

  /* Unpack Member: m_uHashByteBit1 */
  (pToStruct)->m_uHashByteBit1 =  (uint8_t)  ((pFromData)[0] >> 1) & 0x01;

  /* Unpack Member: m_uHashByteBit0 */
  (pToStruct)->m_uHashByteBit0 =  (uint8_t)  ((pFromData)[0] ) & 0x01;

}



/* initialize an instance of this zframe */
void
sbZfC2PpByteHashConverter_InitInstance(sbZfC2PpByteHashConverter_t *pFrame) {

  pFrame->m_uHashByteBit39 =  (unsigned int)  0;
  pFrame->m_uHashByteBit38 =  (unsigned int)  0;
  pFrame->m_uHashByteBit37 =  (unsigned int)  0;
  pFrame->m_uHashByteBit36 =  (unsigned int)  0;
  pFrame->m_uHashByteBit35 =  (unsigned int)  0;
  pFrame->m_uHashByteBit34 =  (unsigned int)  0;
  pFrame->m_uHashByteBit33 =  (unsigned int)  0;
  pFrame->m_uHashByteBit32 =  (unsigned int)  0;
  pFrame->m_uHashByteBit31 =  (unsigned int)  0;
  pFrame->m_uHashByteBit30 =  (unsigned int)  0;
  pFrame->m_uHashByteBit29 =  (unsigned int)  0;
  pFrame->m_uHashByteBit28 =  (unsigned int)  0;
  pFrame->m_uHashByteBit27 =  (unsigned int)  0;
  pFrame->m_uHashByteBit26 =  (unsigned int)  0;
  pFrame->m_uHashByteBit25 =  (unsigned int)  0;
  pFrame->m_uHashByteBit24 =  (unsigned int)  0;
  pFrame->m_uHashByteBit23 =  (unsigned int)  0;
  pFrame->m_uHashByteBit22 =  (unsigned int)  0;
  pFrame->m_uHashByteBit21 =  (unsigned int)  0;
  pFrame->m_uHashByteBit20 =  (unsigned int)  0;
  pFrame->m_uHashByteBit19 =  (unsigned int)  0;
  pFrame->m_uHashByteBit18 =  (unsigned int)  0;
  pFrame->m_uHashByteBit17 =  (unsigned int)  0;
  pFrame->m_uHashByteBit16 =  (unsigned int)  0;
  pFrame->m_uHashByteBit15 =  (unsigned int)  0;
  pFrame->m_uHashByteBit14 =  (unsigned int)  0;
  pFrame->m_uHashByteBit13 =  (unsigned int)  0;
  pFrame->m_uHashByteBit12 =  (unsigned int)  0;
  pFrame->m_uHashByteBit11 =  (unsigned int)  0;
  pFrame->m_uHashByteBit10 =  (unsigned int)  0;
  pFrame->m_uHashByteBit9 =  (unsigned int)  0;
  pFrame->m_uHashByteBit8 =  (unsigned int)  0;
  pFrame->m_uHashByteBit7 =  (unsigned int)  0;
  pFrame->m_uHashByteBit6 =  (unsigned int)  0;
  pFrame->m_uHashByteBit5 =  (unsigned int)  0;
  pFrame->m_uHashByteBit4 =  (unsigned int)  0;
  pFrame->m_uHashByteBit3 =  (unsigned int)  0;
  pFrame->m_uHashByteBit2 =  (unsigned int)  0;
  pFrame->m_uHashByteBit1 =  (unsigned int)  0;
  pFrame->m_uHashByteBit0 =  (unsigned int)  0;

}
