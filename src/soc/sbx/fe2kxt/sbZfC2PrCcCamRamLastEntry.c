/*
 * $Id: sbZfC2PrCcCamRamLastEntry.c 1.1.34.4 Broadcom SDK $
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
#include "sbZfC2PrCcCamRamLastEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfC2PrCcCamRamLastEntry_Pack(sbZfC2PrCcCamRamLastEntry_t *pFrom,
                               uint8_t *pToData,
                               uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_C2PRCCCAMRAMLASTENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uDEPort31 */
  (pToData)[9] |= ((pFrom)->m_uDEPort31 & 0x03) <<3;

  /* Pack Member: m_uDEPort30 */
  (pToData)[9] |= ((pFrom)->m_uDEPort30 & 0x03) <<1;

  /* Pack Member: m_uDEPort29 */
  (pToData)[8] |= ((pFrom)->m_uDEPort29 & 0x01) <<7;
  (pToData)[9] |= ((pFrom)->m_uDEPort29 >> 1) & 0x01;

  /* Pack Member: m_uDEPort28 */
  (pToData)[8] |= ((pFrom)->m_uDEPort28 & 0x03) <<5;

  /* Pack Member: m_uDEPort27 */
  (pToData)[8] |= ((pFrom)->m_uDEPort27 & 0x03) <<3;

  /* Pack Member: m_uDEPort26 */
  (pToData)[8] |= ((pFrom)->m_uDEPort26 & 0x03) <<1;

  /* Pack Member: m_uDEPort25 */
  (pToData)[7] |= ((pFrom)->m_uDEPort25 & 0x01) <<7;
  (pToData)[8] |= ((pFrom)->m_uDEPort25 >> 1) & 0x01;

  /* Pack Member: m_uDEPort24 */
  (pToData)[7] |= ((pFrom)->m_uDEPort24 & 0x03) <<5;

  /* Pack Member: m_uDEPort23 */
  (pToData)[7] |= ((pFrom)->m_uDEPort23 & 0x03) <<3;

  /* Pack Member: m_uDEPort22 */
  (pToData)[7] |= ((pFrom)->m_uDEPort22 & 0x03) <<1;

  /* Pack Member: m_uDEPort21 */
  (pToData)[6] |= ((pFrom)->m_uDEPort21 & 0x01) <<7;
  (pToData)[7] |= ((pFrom)->m_uDEPort21 >> 1) & 0x01;

  /* Pack Member: m_uDEPort20 */
  (pToData)[6] |= ((pFrom)->m_uDEPort20 & 0x03) <<5;

  /* Pack Member: m_uDEPort19 */
  (pToData)[6] |= ((pFrom)->m_uDEPort19 & 0x03) <<3;

  /* Pack Member: m_uDEPort18 */
  (pToData)[6] |= ((pFrom)->m_uDEPort18 & 0x03) <<1;

  /* Pack Member: m_uDEPort17 */
  (pToData)[5] |= ((pFrom)->m_uDEPort17 & 0x01) <<7;
  (pToData)[6] |= ((pFrom)->m_uDEPort17 >> 1) & 0x01;

  /* Pack Member: m_uDEPort16 */
  (pToData)[5] |= ((pFrom)->m_uDEPort16 & 0x03) <<5;

  /* Pack Member: m_uDEPort15 */
  (pToData)[5] |= ((pFrom)->m_uDEPort15 & 0x03) <<3;

  /* Pack Member: m_uDEPort14 */
  (pToData)[5] |= ((pFrom)->m_uDEPort14 & 0x03) <<1;

  /* Pack Member: m_uDEPort13 */
  (pToData)[4] |= ((pFrom)->m_uDEPort13 & 0x01) <<7;
  (pToData)[5] |= ((pFrom)->m_uDEPort13 >> 1) & 0x01;

  /* Pack Member: m_uDEPort12 */
  (pToData)[4] |= ((pFrom)->m_uDEPort12 & 0x03) <<5;

  /* Pack Member: m_uDEPort11 */
  (pToData)[4] |= ((pFrom)->m_uDEPort11 & 0x03) <<3;

  /* Pack Member: m_uDEPort10 */
  (pToData)[4] |= ((pFrom)->m_uDEPort10 & 0x03) <<1;

  /* Pack Member: m_uDEPort9 */
  (pToData)[3] |= ((pFrom)->m_uDEPort9 & 0x01) <<7;
  (pToData)[4] |= ((pFrom)->m_uDEPort9 >> 1) & 0x01;

  /* Pack Member: m_uDEPort8 */
  (pToData)[3] |= ((pFrom)->m_uDEPort8 & 0x03) <<5;

  /* Pack Member: m_uDEPort7 */
  (pToData)[3] |= ((pFrom)->m_uDEPort7 & 0x03) <<3;

  /* Pack Member: m_uDEPort6 */
  (pToData)[3] |= ((pFrom)->m_uDEPort6 & 0x03) <<1;

  /* Pack Member: m_uDEPort5 */
  (pToData)[2] |= ((pFrom)->m_uDEPort5 & 0x01) <<7;
  (pToData)[3] |= ((pFrom)->m_uDEPort5 >> 1) & 0x01;

  /* Pack Member: m_uDEPort4 */
  (pToData)[2] |= ((pFrom)->m_uDEPort4 & 0x03) <<5;

  /* Pack Member: m_uDEPort3 */
  (pToData)[2] |= ((pFrom)->m_uDEPort3 & 0x03) <<3;

  /* Pack Member: m_uDEPort2 */
  (pToData)[2] |= ((pFrom)->m_uDEPort2 & 0x03) <<1;

  /* Pack Member: m_uDEPort1 */
  (pToData)[1] |= ((pFrom)->m_uDEPort1 & 0x01) <<7;
  (pToData)[2] |= ((pFrom)->m_uDEPort1 >> 1) & 0x01;

  /* Pack Member: m_uDEPort0 */
  (pToData)[1] |= ((pFrom)->m_uDEPort0 & 0x03) <<5;

  /* Pack Member: m_uDrop */
  (pToData)[1] |= ((pFrom)->m_uDrop & 0x01) <<4;

  /* Pack Member: m_uQueueAction */
  (pToData)[1] |= ((pFrom)->m_uQueueAction & 0x03) <<2;

  /* Pack Member: m_uQueue */
  (pToData)[0] |= ((pFrom)->m_uQueue & 0x3f) <<2;
  (pToData)[1] |= ((pFrom)->m_uQueue >> 6) & 0x03;

  /* Pack Member: m_uE2ECC */
  (pToData)[0] |= ((pFrom)->m_uE2ECC & 0x01) <<1;

  /* Pack Member: m_uLast */
  (pToData)[0] |= ((pFrom)->m_uLast & 0x01);

  return SB_ZF_C2PRCCCAMRAMLASTENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfC2PrCcCamRamLastEntry_Unpack(sbZfC2PrCcCamRamLastEntry_t *pToStruct,
                                 uint8_t *pFromData,
                                 uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: m_uDEPort31 */
  (pToStruct)->m_uDEPort31 =  (uint32_t)  ((pFromData)[9] >> 3) & 0x03;

  /* Unpack Member: m_uDEPort30 */
  (pToStruct)->m_uDEPort30 =  (uint32_t)  ((pFromData)[9] >> 1) & 0x03;

  /* Unpack Member: m_uDEPort29 */
  (pToStruct)->m_uDEPort29 =  (uint32_t)  ((pFromData)[8] >> 7) & 0x01;
  (pToStruct)->m_uDEPort29 |=  (uint32_t)  ((pFromData)[9] & 0x01) << 1;

  /* Unpack Member: m_uDEPort28 */
  (pToStruct)->m_uDEPort28 =  (uint32_t)  ((pFromData)[8] >> 5) & 0x03;

  /* Unpack Member: m_uDEPort27 */
  (pToStruct)->m_uDEPort27 =  (uint32_t)  ((pFromData)[8] >> 3) & 0x03;

  /* Unpack Member: m_uDEPort26 */
  (pToStruct)->m_uDEPort26 =  (uint32_t)  ((pFromData)[8] >> 1) & 0x03;

  /* Unpack Member: m_uDEPort25 */
  (pToStruct)->m_uDEPort25 =  (uint32_t)  ((pFromData)[7] >> 7) & 0x01;
  (pToStruct)->m_uDEPort25 |=  (uint32_t)  ((pFromData)[8] & 0x01) << 1;

  /* Unpack Member: m_uDEPort24 */
  (pToStruct)->m_uDEPort24 =  (uint32_t)  ((pFromData)[7] >> 5) & 0x03;

  /* Unpack Member: m_uDEPort23 */
  (pToStruct)->m_uDEPort23 =  (uint32_t)  ((pFromData)[7] >> 3) & 0x03;

  /* Unpack Member: m_uDEPort22 */
  (pToStruct)->m_uDEPort22 =  (uint32_t)  ((pFromData)[7] >> 1) & 0x03;

  /* Unpack Member: m_uDEPort21 */
  (pToStruct)->m_uDEPort21 =  (uint32_t)  ((pFromData)[6] >> 7) & 0x01;
  (pToStruct)->m_uDEPort21 |=  (uint32_t)  ((pFromData)[7] & 0x01) << 1;

  /* Unpack Member: m_uDEPort20 */
  (pToStruct)->m_uDEPort20 =  (uint32_t)  ((pFromData)[6] >> 5) & 0x03;

  /* Unpack Member: m_uDEPort19 */
  (pToStruct)->m_uDEPort19 =  (uint32_t)  ((pFromData)[6] >> 3) & 0x03;

  /* Unpack Member: m_uDEPort18 */
  (pToStruct)->m_uDEPort18 =  (uint32_t)  ((pFromData)[6] >> 1) & 0x03;

  /* Unpack Member: m_uDEPort17 */
  (pToStruct)->m_uDEPort17 =  (uint32_t)  ((pFromData)[5] >> 7) & 0x01;
  (pToStruct)->m_uDEPort17 |=  (uint32_t)  ((pFromData)[6] & 0x01) << 1;

  /* Unpack Member: m_uDEPort16 */
  (pToStruct)->m_uDEPort16 =  (uint32_t)  ((pFromData)[5] >> 5) & 0x03;

  /* Unpack Member: m_uDEPort15 */
  (pToStruct)->m_uDEPort15 =  (uint32_t)  ((pFromData)[5] >> 3) & 0x03;

  /* Unpack Member: m_uDEPort14 */
  (pToStruct)->m_uDEPort14 =  (uint32_t)  ((pFromData)[5] >> 1) & 0x03;

  /* Unpack Member: m_uDEPort13 */
  (pToStruct)->m_uDEPort13 =  (uint32_t)  ((pFromData)[4] >> 7) & 0x01;
  (pToStruct)->m_uDEPort13 |=  (uint32_t)  ((pFromData)[5] & 0x01) << 1;

  /* Unpack Member: m_uDEPort12 */
  (pToStruct)->m_uDEPort12 =  (uint32_t)  ((pFromData)[4] >> 5) & 0x03;

  /* Unpack Member: m_uDEPort11 */
  (pToStruct)->m_uDEPort11 =  (uint32_t)  ((pFromData)[4] >> 3) & 0x03;

  /* Unpack Member: m_uDEPort10 */
  (pToStruct)->m_uDEPort10 =  (uint32_t)  ((pFromData)[4] >> 1) & 0x03;

  /* Unpack Member: m_uDEPort9 */
  (pToStruct)->m_uDEPort9 =  (uint32_t)  ((pFromData)[3] >> 7) & 0x01;
  (pToStruct)->m_uDEPort9 |=  (uint32_t)  ((pFromData)[4] & 0x01) << 1;

  /* Unpack Member: m_uDEPort8 */
  (pToStruct)->m_uDEPort8 =  (uint32_t)  ((pFromData)[3] >> 5) & 0x03;

  /* Unpack Member: m_uDEPort7 */
  (pToStruct)->m_uDEPort7 =  (uint32_t)  ((pFromData)[3] >> 3) & 0x03;

  /* Unpack Member: m_uDEPort6 */
  (pToStruct)->m_uDEPort6 =  (uint32_t)  ((pFromData)[3] >> 1) & 0x03;

  /* Unpack Member: m_uDEPort5 */
  (pToStruct)->m_uDEPort5 =  (uint32_t)  ((pFromData)[2] >> 7) & 0x01;
  (pToStruct)->m_uDEPort5 |=  (uint32_t)  ((pFromData)[3] & 0x01) << 1;

  /* Unpack Member: m_uDEPort4 */
  (pToStruct)->m_uDEPort4 =  (uint32_t)  ((pFromData)[2] >> 5) & 0x03;

  /* Unpack Member: m_uDEPort3 */
  (pToStruct)->m_uDEPort3 =  (uint32_t)  ((pFromData)[2] >> 3) & 0x03;

  /* Unpack Member: m_uDEPort2 */
  (pToStruct)->m_uDEPort2 =  (uint32_t)  ((pFromData)[2] >> 1) & 0x03;

  /* Unpack Member: m_uDEPort1 */
  (pToStruct)->m_uDEPort1 =  (uint32_t)  ((pFromData)[1] >> 7) & 0x01;
  (pToStruct)->m_uDEPort1 |=  (uint32_t)  ((pFromData)[2] & 0x01) << 1;

  /* Unpack Member: m_uDEPort0 */
  (pToStruct)->m_uDEPort0 =  (uint32_t)  ((pFromData)[1] >> 5) & 0x03;

  /* Unpack Member: m_uDrop */
  (pToStruct)->m_uDrop =  (uint8_t)  ((pFromData)[1] >> 4) & 0x01;

  /* Unpack Member: m_uQueueAction */
  (pToStruct)->m_uQueueAction =  (uint32_t)  ((pFromData)[1] >> 2) & 0x03;

  /* Unpack Member: m_uQueue */
  (pToStruct)->m_uQueue =  (uint32_t)  ((pFromData)[0] >> 2) & 0x3f;
  (pToStruct)->m_uQueue |=  (uint32_t)  ((pFromData)[1] & 0x03) << 6;

  /* Unpack Member: m_uE2ECC */
  (pToStruct)->m_uE2ECC =  (uint8_t)  ((pFromData)[0] >> 1) & 0x01;

  /* Unpack Member: m_uLast */
  (pToStruct)->m_uLast =  (uint8_t)  ((pFromData)[0] ) & 0x01;

}



/* initialize an instance of this zframe */
void
sbZfC2PrCcCamRamLastEntry_InitInstance(sbZfC2PrCcCamRamLastEntry_t *pFrame) {

  pFrame->m_uDEPort31 =  (unsigned int)  0;
  pFrame->m_uDEPort30 =  (unsigned int)  0;
  pFrame->m_uDEPort29 =  (unsigned int)  0;
  pFrame->m_uDEPort28 =  (unsigned int)  0;
  pFrame->m_uDEPort27 =  (unsigned int)  0;
  pFrame->m_uDEPort26 =  (unsigned int)  0;
  pFrame->m_uDEPort25 =  (unsigned int)  0;
  pFrame->m_uDEPort24 =  (unsigned int)  0;
  pFrame->m_uDEPort23 =  (unsigned int)  0;
  pFrame->m_uDEPort22 =  (unsigned int)  0;
  pFrame->m_uDEPort21 =  (unsigned int)  0;
  pFrame->m_uDEPort20 =  (unsigned int)  0;
  pFrame->m_uDEPort19 =  (unsigned int)  0;
  pFrame->m_uDEPort18 =  (unsigned int)  0;
  pFrame->m_uDEPort17 =  (unsigned int)  0;
  pFrame->m_uDEPort16 =  (unsigned int)  0;
  pFrame->m_uDEPort15 =  (unsigned int)  0;
  pFrame->m_uDEPort14 =  (unsigned int)  0;
  pFrame->m_uDEPort13 =  (unsigned int)  0;
  pFrame->m_uDEPort12 =  (unsigned int)  0;
  pFrame->m_uDEPort11 =  (unsigned int)  0;
  pFrame->m_uDEPort10 =  (unsigned int)  0;
  pFrame->m_uDEPort9 =  (unsigned int)  0;
  pFrame->m_uDEPort8 =  (unsigned int)  0;
  pFrame->m_uDEPort7 =  (unsigned int)  0;
  pFrame->m_uDEPort6 =  (unsigned int)  0;
  pFrame->m_uDEPort5 =  (unsigned int)  0;
  pFrame->m_uDEPort4 =  (unsigned int)  0;
  pFrame->m_uDEPort3 =  (unsigned int)  0;
  pFrame->m_uDEPort2 =  (unsigned int)  0;
  pFrame->m_uDEPort1 =  (unsigned int)  0;
  pFrame->m_uDEPort0 =  (unsigned int)  0;
  pFrame->m_uDrop =  (unsigned int)  0;
  pFrame->m_uQueueAction =  (unsigned int)  0;
  pFrame->m_uQueue =  (unsigned int)  0;
  pFrame->m_uE2ECC =  (unsigned int)  0;
  pFrame->m_uLast =  (unsigned int)  0;

}
