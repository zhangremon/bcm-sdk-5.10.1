/*
 * $Id: sbZfKaRbClassPortEnablesEntry.c 1.1.44.4 Broadcom SDK $
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
#include "sbZfKaRbClassPortEnablesEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfKaRbClassPortEnablesEntry_Pack(sbZfKaRbClassPortEnablesEntry_t *pFrom,
                                   uint8_t *pToData,
                                   uint32_t nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKARBCLASSPORTENABLESENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nReserve */
  (pToData)[1] |= ((pFrom)->m_nReserve & 0x01) <<7;
  (pToData)[0] |= ((pFrom)->m_nReserve >> 1) &0xFF;

  /* Pack Member: m_nUseUser1 */
  (pToData)[1] |= ((pFrom)->m_nUseUser1 & 0x01) <<6;

  /* Pack Member: m_nUseUser0 */
  (pToData)[1] |= ((pFrom)->m_nUseUser0 & 0x01) <<5;

  /* Pack Member: m_nUseVlanPri */
  (pToData)[1] |= ((pFrom)->m_nUseVlanPri & 0x01) <<4;

  /* Pack Member: m_nUseHiPriVlan */
  (pToData)[1] |= ((pFrom)->m_nUseHiPriVlan & 0x01) <<3;

  /* Pack Member: m_nUseDmacMatch */
  (pToData)[1] |= ((pFrom)->m_nUseDmacMatch & 0x01) <<2;

  /* Pack Member: m_nUseLayer4 */
  (pToData)[1] |= ((pFrom)->m_nUseLayer4 & 0x01) <<1;

  /* Pack Member: m_nFlowHashEnable */
  (pToData)[1] |= ((pFrom)->m_nFlowHashEnable & 0x01);

  /* Pack Member: m_nUseHashCos */
  (pToData)[3] |= ((pFrom)->m_nUseHashCos) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nUseHashCos >> 8) &0xFF;
#else
  int i;
  int size = SB_ZF_ZFKARBCLASSPORTENABLESENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nReserve */
  (pToData)[2] |= ((pFrom)->m_nReserve & 0x01) <<7;
  (pToData)[3] |= ((pFrom)->m_nReserve >> 1) &0xFF;

  /* Pack Member: m_nUseUser1 */
  (pToData)[2] |= ((pFrom)->m_nUseUser1 & 0x01) <<6;

  /* Pack Member: m_nUseUser0 */
  (pToData)[2] |= ((pFrom)->m_nUseUser0 & 0x01) <<5;

  /* Pack Member: m_nUseVlanPri */
  (pToData)[2] |= ((pFrom)->m_nUseVlanPri & 0x01) <<4;

  /* Pack Member: m_nUseHiPriVlan */
  (pToData)[2] |= ((pFrom)->m_nUseHiPriVlan & 0x01) <<3;

  /* Pack Member: m_nUseDmacMatch */
  (pToData)[2] |= ((pFrom)->m_nUseDmacMatch & 0x01) <<2;

  /* Pack Member: m_nUseLayer4 */
  (pToData)[2] |= ((pFrom)->m_nUseLayer4 & 0x01) <<1;

  /* Pack Member: m_nFlowHashEnable */
  (pToData)[2] |= ((pFrom)->m_nFlowHashEnable & 0x01);

  /* Pack Member: m_nUseHashCos */
  (pToData)[0] |= ((pFrom)->m_nUseHashCos) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nUseHashCos >> 8) &0xFF;
#endif

  return SB_ZF_ZFKARBCLASSPORTENABLESENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaRbClassPortEnablesEntry_Unpack(sbZfKaRbClassPortEnablesEntry_t *pToStruct,
                                     uint8_t *pFromData,
                                     uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nReserve */
  (pToStruct)->m_nReserve =  (uint32_t)  ((pFromData)[1] >> 7) & 0x01;
  (pToStruct)->m_nReserve |=  (uint32_t)  (pFromData)[0] << 1;

  /* Unpack Member: m_nUseUser1 */
  (pToStruct)->m_nUseUser1 =  (uint8_t)  ((pFromData)[1] >> 6) & 0x01;

  /* Unpack Member: m_nUseUser0 */
  (pToStruct)->m_nUseUser0 =  (uint8_t)  ((pFromData)[1] >> 5) & 0x01;

  /* Unpack Member: m_nUseVlanPri */
  (pToStruct)->m_nUseVlanPri =  (uint8_t)  ((pFromData)[1] >> 4) & 0x01;

  /* Unpack Member: m_nUseHiPriVlan */
  (pToStruct)->m_nUseHiPriVlan =  (uint8_t)  ((pFromData)[1] >> 3) & 0x01;

  /* Unpack Member: m_nUseDmacMatch */
  (pToStruct)->m_nUseDmacMatch =  (uint8_t)  ((pFromData)[1] >> 2) & 0x01;

  /* Unpack Member: m_nUseLayer4 */
  (pToStruct)->m_nUseLayer4 =  (uint8_t)  ((pFromData)[1] >> 1) & 0x01;

  /* Unpack Member: m_nFlowHashEnable */
  (pToStruct)->m_nFlowHashEnable =  (uint8_t)  ((pFromData)[1] ) & 0x01;

  /* Unpack Member: m_nUseHashCos */
  (pToStruct)->m_nUseHashCos =  (uint32_t)  (pFromData)[3] ;
  (pToStruct)->m_nUseHashCos |=  (uint32_t)  (pFromData)[2] << 8;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nReserve */
  (pToStruct)->m_nReserve =  (uint32_t)  ((pFromData)[2] >> 7) & 0x01;
  (pToStruct)->m_nReserve |=  (uint32_t)  (pFromData)[3] << 1;

  /* Unpack Member: m_nUseUser1 */
  (pToStruct)->m_nUseUser1 =  (uint8_t)  ((pFromData)[2] >> 6) & 0x01;

  /* Unpack Member: m_nUseUser0 */
  (pToStruct)->m_nUseUser0 =  (uint8_t)  ((pFromData)[2] >> 5) & 0x01;

  /* Unpack Member: m_nUseVlanPri */
  (pToStruct)->m_nUseVlanPri =  (uint8_t)  ((pFromData)[2] >> 4) & 0x01;

  /* Unpack Member: m_nUseHiPriVlan */
  (pToStruct)->m_nUseHiPriVlan =  (uint8_t)  ((pFromData)[2] >> 3) & 0x01;

  /* Unpack Member: m_nUseDmacMatch */
  (pToStruct)->m_nUseDmacMatch =  (uint8_t)  ((pFromData)[2] >> 2) & 0x01;

  /* Unpack Member: m_nUseLayer4 */
  (pToStruct)->m_nUseLayer4 =  (uint8_t)  ((pFromData)[2] >> 1) & 0x01;

  /* Unpack Member: m_nFlowHashEnable */
  (pToStruct)->m_nFlowHashEnable =  (uint8_t)  ((pFromData)[2] ) & 0x01;

  /* Unpack Member: m_nUseHashCos */
  (pToStruct)->m_nUseHashCos =  (uint32_t)  (pFromData)[0] ;
  (pToStruct)->m_nUseHashCos |=  (uint32_t)  (pFromData)[1] << 8;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaRbClassPortEnablesEntry_InitInstance(sbZfKaRbClassPortEnablesEntry_t *pFrame) {

  pFrame->m_nReserve =  (unsigned int)  0;
  pFrame->m_nUseUser1 =  (unsigned int)  0;
  pFrame->m_nUseUser0 =  (unsigned int)  0;
  pFrame->m_nUseVlanPri =  (unsigned int)  0;
  pFrame->m_nUseHiPriVlan =  (unsigned int)  0;
  pFrame->m_nUseDmacMatch =  (unsigned int)  0;
  pFrame->m_nUseLayer4 =  (unsigned int)  0;
  pFrame->m_nFlowHashEnable =  (unsigned int)  0;
  pFrame->m_nUseHashCos =  (unsigned int)  0;

}
