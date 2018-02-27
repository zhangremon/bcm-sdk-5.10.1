/*
 * $Id: sbZfKaQsLnaPriEntry.c 1.2.12.4 Broadcom SDK $
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
#include "sbZfKaQsLnaPriEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfKaQsLnaPriEntry_Pack(sbZfKaQsLnaPriEntry_t *pFrom,
                         uint8_t *pToData,
                         uint32_t nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAQSLNAPRIENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nPri4 */
  (pToData)[19] |= ((pFrom)->m_nPri4) & 0xFF;
  (pToData)[18] |= ((pFrom)->m_nPri4 >> 8) &0xFF;
  (pToData)[17] |= ((pFrom)->m_nPri4 >> 16) &0xFF;
  (pToData)[16] |= ((pFrom)->m_nPri4 >> 24) &0xFF;

  /* Pack Member: m_nPri3 */
  (pToData)[15] |= ((pFrom)->m_nPri3) & 0xFF;
  (pToData)[14] |= ((pFrom)->m_nPri3 >> 8) &0xFF;
  (pToData)[13] |= ((pFrom)->m_nPri3 >> 16) &0xFF;
  (pToData)[12] |= ((pFrom)->m_nPri3 >> 24) &0xFF;

  /* Pack Member: m_nPri2 */
  (pToData)[11] |= ((pFrom)->m_nPri2) & 0xFF;
  (pToData)[10] |= ((pFrom)->m_nPri2 >> 8) &0xFF;
  (pToData)[9] |= ((pFrom)->m_nPri2 >> 16) &0xFF;
  (pToData)[8] |= ((pFrom)->m_nPri2 >> 24) &0xFF;

  /* Pack Member: m_nPri1 */
  (pToData)[7] |= ((pFrom)->m_nPri1) & 0xFF;
  (pToData)[6] |= ((pFrom)->m_nPri1 >> 8) &0xFF;
  (pToData)[5] |= ((pFrom)->m_nPri1 >> 16) &0xFF;
  (pToData)[4] |= ((pFrom)->m_nPri1 >> 24) &0xFF;

  /* Pack Member: m_nPri0 */
  (pToData)[3] |= ((pFrom)->m_nPri0) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nPri0 >> 8) &0xFF;
  (pToData)[1] |= ((pFrom)->m_nPri0 >> 16) &0xFF;
  (pToData)[0] |= ((pFrom)->m_nPri0 >> 24) &0xFF;
#else
  int i;
  int size = SB_ZF_ZFKAQSLNAPRIENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nPri4 */
  (pToData)[16] |= ((pFrom)->m_nPri4) & 0xFF;
  (pToData)[17] |= ((pFrom)->m_nPri4 >> 8) &0xFF;
  (pToData)[18] |= ((pFrom)->m_nPri4 >> 16) &0xFF;
  (pToData)[19] |= ((pFrom)->m_nPri4 >> 24) &0xFF;

  /* Pack Member: m_nPri3 */
  (pToData)[12] |= ((pFrom)->m_nPri3) & 0xFF;
  (pToData)[13] |= ((pFrom)->m_nPri3 >> 8) &0xFF;
  (pToData)[14] |= ((pFrom)->m_nPri3 >> 16) &0xFF;
  (pToData)[15] |= ((pFrom)->m_nPri3 >> 24) &0xFF;

  /* Pack Member: m_nPri2 */
  (pToData)[8] |= ((pFrom)->m_nPri2) & 0xFF;
  (pToData)[9] |= ((pFrom)->m_nPri2 >> 8) &0xFF;
  (pToData)[10] |= ((pFrom)->m_nPri2 >> 16) &0xFF;
  (pToData)[11] |= ((pFrom)->m_nPri2 >> 24) &0xFF;

  /* Pack Member: m_nPri1 */
  (pToData)[4] |= ((pFrom)->m_nPri1) & 0xFF;
  (pToData)[5] |= ((pFrom)->m_nPri1 >> 8) &0xFF;
  (pToData)[6] |= ((pFrom)->m_nPri1 >> 16) &0xFF;
  (pToData)[7] |= ((pFrom)->m_nPri1 >> 24) &0xFF;

  /* Pack Member: m_nPri0 */
  (pToData)[0] |= ((pFrom)->m_nPri0) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nPri0 >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nPri0 >> 16) &0xFF;
  (pToData)[3] |= ((pFrom)->m_nPri0 >> 24) &0xFF;
#endif

  return SB_ZF_ZFKAQSLNAPRIENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaQsLnaPriEntry_Unpack(sbZfKaQsLnaPriEntry_t *pToStruct,
                           uint8_t *pFromData,
                           uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nPri4 */
  (pToStruct)->m_nPri4 =  (uint32_t)  (pFromData)[19] ;
  (pToStruct)->m_nPri4 |=  (uint32_t)  (pFromData)[18] << 8;
  (pToStruct)->m_nPri4 |=  (uint32_t)  (pFromData)[17] << 16;
  (pToStruct)->m_nPri4 |=  (uint32_t)  (pFromData)[16] << 24;

  /* Unpack Member: m_nPri3 */
  (pToStruct)->m_nPri3 =  (uint32_t)  (pFromData)[15] ;
  (pToStruct)->m_nPri3 |=  (uint32_t)  (pFromData)[14] << 8;
  (pToStruct)->m_nPri3 |=  (uint32_t)  (pFromData)[13] << 16;
  (pToStruct)->m_nPri3 |=  (uint32_t)  (pFromData)[12] << 24;

  /* Unpack Member: m_nPri2 */
  (pToStruct)->m_nPri2 =  (uint32_t)  (pFromData)[11] ;
  (pToStruct)->m_nPri2 |=  (uint32_t)  (pFromData)[10] << 8;
  (pToStruct)->m_nPri2 |=  (uint32_t)  (pFromData)[9] << 16;
  (pToStruct)->m_nPri2 |=  (uint32_t)  (pFromData)[8] << 24;

  /* Unpack Member: m_nPri1 */
  (pToStruct)->m_nPri1 =  (uint32_t)  (pFromData)[7] ;
  (pToStruct)->m_nPri1 |=  (uint32_t)  (pFromData)[6] << 8;
  (pToStruct)->m_nPri1 |=  (uint32_t)  (pFromData)[5] << 16;
  (pToStruct)->m_nPri1 |=  (uint32_t)  (pFromData)[4] << 24;

  /* Unpack Member: m_nPri0 */
  (pToStruct)->m_nPri0 =  (uint32_t)  (pFromData)[3] ;
  (pToStruct)->m_nPri0 |=  (uint32_t)  (pFromData)[2] << 8;
  (pToStruct)->m_nPri0 |=  (uint32_t)  (pFromData)[1] << 16;
  (pToStruct)->m_nPri0 |=  (uint32_t)  (pFromData)[0] << 24;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nPri4 */
  (pToStruct)->m_nPri4 =  (uint32_t)  (pFromData)[16] ;
  (pToStruct)->m_nPri4 |=  (uint32_t)  (pFromData)[17] << 8;
  (pToStruct)->m_nPri4 |=  (uint32_t)  (pFromData)[18] << 16;
  (pToStruct)->m_nPri4 |=  (uint32_t)  (pFromData)[19] << 24;

  /* Unpack Member: m_nPri3 */
  (pToStruct)->m_nPri3 =  (uint32_t)  (pFromData)[12] ;
  (pToStruct)->m_nPri3 |=  (uint32_t)  (pFromData)[13] << 8;
  (pToStruct)->m_nPri3 |=  (uint32_t)  (pFromData)[14] << 16;
  (pToStruct)->m_nPri3 |=  (uint32_t)  (pFromData)[15] << 24;

  /* Unpack Member: m_nPri2 */
  (pToStruct)->m_nPri2 =  (uint32_t)  (pFromData)[8] ;
  (pToStruct)->m_nPri2 |=  (uint32_t)  (pFromData)[9] << 8;
  (pToStruct)->m_nPri2 |=  (uint32_t)  (pFromData)[10] << 16;
  (pToStruct)->m_nPri2 |=  (uint32_t)  (pFromData)[11] << 24;

  /* Unpack Member: m_nPri1 */
  (pToStruct)->m_nPri1 =  (uint32_t)  (pFromData)[4] ;
  (pToStruct)->m_nPri1 |=  (uint32_t)  (pFromData)[5] << 8;
  (pToStruct)->m_nPri1 |=  (uint32_t)  (pFromData)[6] << 16;
  (pToStruct)->m_nPri1 |=  (uint32_t)  (pFromData)[7] << 24;

  /* Unpack Member: m_nPri0 */
  (pToStruct)->m_nPri0 =  (uint32_t)  (pFromData)[0] ;
  (pToStruct)->m_nPri0 |=  (uint32_t)  (pFromData)[1] << 8;
  (pToStruct)->m_nPri0 |=  (uint32_t)  (pFromData)[2] << 16;
  (pToStruct)->m_nPri0 |=  (uint32_t)  (pFromData)[3] << 24;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaQsLnaPriEntry_InitInstance(sbZfKaQsLnaPriEntry_t *pFrame) {

  pFrame->m_nPri4 =  (unsigned int)  0;
  pFrame->m_nPri3 =  (unsigned int)  0;
  pFrame->m_nPri2 =  (unsigned int)  0;
  pFrame->m_nPri1 =  (unsigned int)  0;
  pFrame->m_nPri0 =  (unsigned int)  0;

}
