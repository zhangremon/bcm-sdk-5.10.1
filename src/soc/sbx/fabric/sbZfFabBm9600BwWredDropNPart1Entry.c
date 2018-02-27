/*
 * $Id: sbZfFabBm9600BwWredDropNPart1Entry.c 1.1.44.4 Broadcom SDK $
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
#include "sbZfFabBm9600BwWredDropNPart1Entry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfFabBm9600BwWredDropNPart1Entry_Pack(sbZfFabBm9600BwWredDropNPart1Entry_t *pFrom,
                                        uint8_t *pToData,
                                        uint32_t nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM9600_BWWREDDROPNPART1ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_uDp1 */
  (pToData)[0] |= ((pFrom)->m_uDp1 & 0x01) <<7;

  /* Pack Member: m_uEcn1 */
  (pToData)[0] |= ((pFrom)->m_uEcn1 & 0x01) <<6;

  /* Pack Member: m_uReserved1 */
  (pToData)[0] |= ((pFrom)->m_uReserved1 & 0x0f) <<2;

  /* Pack Member: m_uPbDe1 */
  (pToData)[1] |= ((pFrom)->m_uPbDe1) & 0xFF;
  (pToData)[0] |= ((pFrom)->m_uPbDe1 >> 8) & 0x03;

  /* Pack Member: m_uDp0 */
  (pToData)[2] |= ((pFrom)->m_uDp0 & 0x01) <<7;

  /* Pack Member: m_uEcn0 */
  (pToData)[2] |= ((pFrom)->m_uEcn0 & 0x01) <<6;

  /* Pack Member: m_uReserved0 */
  (pToData)[2] |= ((pFrom)->m_uReserved0 & 0x0f) <<2;

  /* Pack Member: m_uPbDe0 */
  (pToData)[3] |= ((pFrom)->m_uPbDe0) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_uPbDe0 >> 8) & 0x03;
#else
  int i;
  int size = SB_ZF_FAB_BM9600_BWWREDDROPNPART1ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uDp1 */
  (pToData)[3] |= ((pFrom)->m_uDp1 & 0x01) <<7;

  /* Pack Member: m_uEcn1 */
  (pToData)[3] |= ((pFrom)->m_uEcn1 & 0x01) <<6;

  /* Pack Member: m_uReserved1 */
  (pToData)[3] |= ((pFrom)->m_uReserved1 & 0x0f) <<2;

  /* Pack Member: m_uPbDe1 */
  (pToData)[2] |= ((pFrom)->m_uPbDe1) & 0xFF;
  (pToData)[3] |= ((pFrom)->m_uPbDe1 >> 8) & 0x03;

  /* Pack Member: m_uDp0 */
  (pToData)[1] |= ((pFrom)->m_uDp0 & 0x01) <<7;

  /* Pack Member: m_uEcn0 */
  (pToData)[1] |= ((pFrom)->m_uEcn0 & 0x01) <<6;

  /* Pack Member: m_uReserved0 */
  (pToData)[1] |= ((pFrom)->m_uReserved0 & 0x0f) <<2;

  /* Pack Member: m_uPbDe0 */
  (pToData)[0] |= ((pFrom)->m_uPbDe0) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_uPbDe0 >> 8) & 0x03;
#endif

  return SB_ZF_FAB_BM9600_BWWREDDROPNPART1ENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm9600BwWredDropNPart1Entry_Unpack(sbZfFabBm9600BwWredDropNPart1Entry_t *pToStruct,
                                          uint8_t *pFromData,
                                          uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_uDp1 */
  (pToStruct)->m_uDp1 =  (uint32_t)  ((pFromData)[0] >> 7) & 0x01;

  /* Unpack Member: m_uEcn1 */
  (pToStruct)->m_uEcn1 =  (uint32_t)  ((pFromData)[0] >> 6) & 0x01;

  /* Unpack Member: m_uReserved1 */
  (pToStruct)->m_uReserved1 =  (uint32_t)  ((pFromData)[0] >> 2) & 0x0f;

  /* Unpack Member: m_uPbDe1 */
  (pToStruct)->m_uPbDe1 =  (uint32_t)  (pFromData)[1] ;
  (pToStruct)->m_uPbDe1 |=  (uint32_t)  ((pFromData)[0] & 0x03) << 8;

  /* Unpack Member: m_uDp0 */
  (pToStruct)->m_uDp0 =  (uint32_t)  ((pFromData)[2] >> 7) & 0x01;

  /* Unpack Member: m_uEcn0 */
  (pToStruct)->m_uEcn0 =  (uint32_t)  ((pFromData)[2] >> 6) & 0x01;

  /* Unpack Member: m_uReserved0 */
  (pToStruct)->m_uReserved0 =  (uint32_t)  ((pFromData)[2] >> 2) & 0x0f;

  /* Unpack Member: m_uPbDe0 */
  (pToStruct)->m_uPbDe0 =  (uint32_t)  (pFromData)[3] ;
  (pToStruct)->m_uPbDe0 |=  (uint32_t)  ((pFromData)[2] & 0x03) << 8;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_uDp1 */
  (pToStruct)->m_uDp1 =  (uint32_t)  ((pFromData)[3] >> 7) & 0x01;

  /* Unpack Member: m_uEcn1 */
  (pToStruct)->m_uEcn1 =  (uint32_t)  ((pFromData)[3] >> 6) & 0x01;

  /* Unpack Member: m_uReserved1 */
  (pToStruct)->m_uReserved1 =  (uint32_t)  ((pFromData)[3] >> 2) & 0x0f;

  /* Unpack Member: m_uPbDe1 */
  (pToStruct)->m_uPbDe1 =  (uint32_t)  (pFromData)[2] ;
  (pToStruct)->m_uPbDe1 |=  (uint32_t)  ((pFromData)[3] & 0x03) << 8;

  /* Unpack Member: m_uDp0 */
  (pToStruct)->m_uDp0 =  (uint32_t)  ((pFromData)[1] >> 7) & 0x01;

  /* Unpack Member: m_uEcn0 */
  (pToStruct)->m_uEcn0 =  (uint32_t)  ((pFromData)[1] >> 6) & 0x01;

  /* Unpack Member: m_uReserved0 */
  (pToStruct)->m_uReserved0 =  (uint32_t)  ((pFromData)[1] >> 2) & 0x0f;

  /* Unpack Member: m_uPbDe0 */
  (pToStruct)->m_uPbDe0 =  (uint32_t)  (pFromData)[0] ;
  (pToStruct)->m_uPbDe0 |=  (uint32_t)  ((pFromData)[1] & 0x03) << 8;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm9600BwWredDropNPart1Entry_InitInstance(sbZfFabBm9600BwWredDropNPart1Entry_t *pFrame) {

  pFrame->m_uDp1 =  (unsigned int)  0;
  pFrame->m_uEcn1 =  (unsigned int)  0;
  pFrame->m_uReserved1 =  (unsigned int)  0;
  pFrame->m_uPbDe1 =  (unsigned int)  0;
  pFrame->m_uDp0 =  (unsigned int)  0;
  pFrame->m_uEcn0 =  (unsigned int)  0;
  pFrame->m_uReserved0 =  (unsigned int)  0;
  pFrame->m_uPbDe0 =  (unsigned int)  0;

}
