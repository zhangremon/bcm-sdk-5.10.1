/*
 * $Id: sbZfKaQmWredParamEntry.c 1.1.44.4 Broadcom SDK $
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
#include "sbZfKaQmWredParamEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfKaQmWredParamEntry_Pack(sbZfKaQmWredParamEntry_t *pFrom,
                            uint8_t *pToData,
                            uint32_t nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAQMWREDPARAMENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nTMaxExceeded2 */
  (pToData)[7] |= ((pFrom)->m_nTMaxExceeded2 & 0x01) <<3;

  /* Pack Member: m_nEcnExceeded2 */
  (pToData)[7] |= ((pFrom)->m_nEcnExceeded2 & 0x01) <<2;

  /* Pack Member: m_nPDrop2 */
  (pToData)[0] |= ((pFrom)->m_nPDrop2) & 0xFF;
  (pToData)[7] |= ((pFrom)->m_nPDrop2 >> 8) & 0x03;

  /* Pack Member: m_nTMaxExceeded1 */
  (pToData)[1] |= ((pFrom)->m_nTMaxExceeded1 & 0x01) <<7;

  /* Pack Member: m_nEcnExceeded1 */
  (pToData)[1] |= ((pFrom)->m_nEcnExceeded1 & 0x01) <<6;

  /* Pack Member: m_nPDrop1 */
  (pToData)[2] |= ((pFrom)->m_nPDrop1 & 0x0f) <<4;
  (pToData)[1] |= ((pFrom)->m_nPDrop1 >> 4) & 0x3f;

  /* Pack Member: m_nTMaxExceeded0 */
  (pToData)[2] |= ((pFrom)->m_nTMaxExceeded0 & 0x01) <<3;

  /* Pack Member: m_nEcnExceeded0 */
  (pToData)[2] |= ((pFrom)->m_nEcnExceeded0 & 0x01) <<2;

  /* Pack Member: m_nPDrop0 */
  (pToData)[3] |= ((pFrom)->m_nPDrop0) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nPDrop0 >> 8) & 0x03;
#else
  int i;
  int size = SB_ZF_ZFKAQMWREDPARAMENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nTMaxExceeded2 */
  (pToData)[4] |= ((pFrom)->m_nTMaxExceeded2 & 0x01) <<3;

  /* Pack Member: m_nEcnExceeded2 */
  (pToData)[4] |= ((pFrom)->m_nEcnExceeded2 & 0x01) <<2;

  /* Pack Member: m_nPDrop2 */
  (pToData)[3] |= ((pFrom)->m_nPDrop2) & 0xFF;
  (pToData)[4] |= ((pFrom)->m_nPDrop2 >> 8) & 0x03;

  /* Pack Member: m_nTMaxExceeded1 */
  (pToData)[2] |= ((pFrom)->m_nTMaxExceeded1 & 0x01) <<7;

  /* Pack Member: m_nEcnExceeded1 */
  (pToData)[2] |= ((pFrom)->m_nEcnExceeded1 & 0x01) <<6;

  /* Pack Member: m_nPDrop1 */
  (pToData)[1] |= ((pFrom)->m_nPDrop1 & 0x0f) <<4;
  (pToData)[2] |= ((pFrom)->m_nPDrop1 >> 4) & 0x3f;

  /* Pack Member: m_nTMaxExceeded0 */
  (pToData)[1] |= ((pFrom)->m_nTMaxExceeded0 & 0x01) <<3;

  /* Pack Member: m_nEcnExceeded0 */
  (pToData)[1] |= ((pFrom)->m_nEcnExceeded0 & 0x01) <<2;

  /* Pack Member: m_nPDrop0 */
  (pToData)[0] |= ((pFrom)->m_nPDrop0) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nPDrop0 >> 8) & 0x03;
#endif

  return SB_ZF_ZFKAQMWREDPARAMENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaQmWredParamEntry_Unpack(sbZfKaQmWredParamEntry_t *pToStruct,
                              uint8_t *pFromData,
                              uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nTMaxExceeded2 */
  (pToStruct)->m_nTMaxExceeded2 =  (uint8_t)  ((pFromData)[7] >> 3) & 0x01;

  /* Unpack Member: m_nEcnExceeded2 */
  (pToStruct)->m_nEcnExceeded2 =  (uint8_t)  ((pFromData)[7] >> 2) & 0x01;

  /* Unpack Member: m_nPDrop2 */
  (pToStruct)->m_nPDrop2 =  (uint32_t)  (pFromData)[0] ;
  (pToStruct)->m_nPDrop2 |=  (uint32_t)  ((pFromData)[7] & 0x03) << 8;

  /* Unpack Member: m_nTMaxExceeded1 */
  (pToStruct)->m_nTMaxExceeded1 =  (uint8_t)  ((pFromData)[1] >> 7) & 0x01;

  /* Unpack Member: m_nEcnExceeded1 */
  (pToStruct)->m_nEcnExceeded1 =  (uint8_t)  ((pFromData)[1] >> 6) & 0x01;

  /* Unpack Member: m_nPDrop1 */
  (pToStruct)->m_nPDrop1 =  (uint32_t)  ((pFromData)[2] >> 4) & 0x0f;
  (pToStruct)->m_nPDrop1 |=  (uint32_t)  ((pFromData)[1] & 0x3f) << 4;

  /* Unpack Member: m_nTMaxExceeded0 */
  (pToStruct)->m_nTMaxExceeded0 =  (uint8_t)  ((pFromData)[2] >> 3) & 0x01;

  /* Unpack Member: m_nEcnExceeded0 */
  (pToStruct)->m_nEcnExceeded0 =  (uint8_t)  ((pFromData)[2] >> 2) & 0x01;

  /* Unpack Member: m_nPDrop0 */
  (pToStruct)->m_nPDrop0 =  (uint32_t)  (pFromData)[3] ;
  (pToStruct)->m_nPDrop0 |=  (uint32_t)  ((pFromData)[2] & 0x03) << 8;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nTMaxExceeded2 */
  (pToStruct)->m_nTMaxExceeded2 =  (uint8_t)  ((pFromData)[4] >> 3) & 0x01;

  /* Unpack Member: m_nEcnExceeded2 */
  (pToStruct)->m_nEcnExceeded2 =  (uint8_t)  ((pFromData)[4] >> 2) & 0x01;

  /* Unpack Member: m_nPDrop2 */
  (pToStruct)->m_nPDrop2 =  (uint32_t)  (pFromData)[3] ;
  (pToStruct)->m_nPDrop2 |=  (uint32_t)  ((pFromData)[4] & 0x03) << 8;

  /* Unpack Member: m_nTMaxExceeded1 */
  (pToStruct)->m_nTMaxExceeded1 =  (uint8_t)  ((pFromData)[2] >> 7) & 0x01;

  /* Unpack Member: m_nEcnExceeded1 */
  (pToStruct)->m_nEcnExceeded1 =  (uint8_t)  ((pFromData)[2] >> 6) & 0x01;

  /* Unpack Member: m_nPDrop1 */
  (pToStruct)->m_nPDrop1 =  (uint32_t)  ((pFromData)[1] >> 4) & 0x0f;
  (pToStruct)->m_nPDrop1 |=  (uint32_t)  ((pFromData)[2] & 0x3f) << 4;

  /* Unpack Member: m_nTMaxExceeded0 */
  (pToStruct)->m_nTMaxExceeded0 =  (uint8_t)  ((pFromData)[1] >> 3) & 0x01;

  /* Unpack Member: m_nEcnExceeded0 */
  (pToStruct)->m_nEcnExceeded0 =  (uint8_t)  ((pFromData)[1] >> 2) & 0x01;

  /* Unpack Member: m_nPDrop0 */
  (pToStruct)->m_nPDrop0 =  (uint32_t)  (pFromData)[0] ;
  (pToStruct)->m_nPDrop0 |=  (uint32_t)  ((pFromData)[1] & 0x03) << 8;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaQmWredParamEntry_InitInstance(sbZfKaQmWredParamEntry_t *pFrame) {

  pFrame->m_nTMaxExceeded2 =  (unsigned int)  0;
  pFrame->m_nEcnExceeded2 =  (unsigned int)  0;
  pFrame->m_nPDrop2 =  (unsigned int)  0;
  pFrame->m_nTMaxExceeded1 =  (unsigned int)  0;
  pFrame->m_nEcnExceeded1 =  (unsigned int)  0;
  pFrame->m_nPDrop1 =  (unsigned int)  0;
  pFrame->m_nTMaxExceeded0 =  (unsigned int)  0;
  pFrame->m_nEcnExceeded0 =  (unsigned int)  0;
  pFrame->m_nPDrop0 =  (unsigned int)  0;

}
