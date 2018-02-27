/*
 * $Id: sbZfKaQsPriLutAddr.c 1.1.44.4 Broadcom SDK $
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
#include "sbZfKaQsPriLutAddr.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfKaQsPriLutAddr_Pack(sbZfKaQsPriLutAddr_t *pFrom,
                        uint8_t *pToData,
                        uint32_t nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAQSPRILUTADDR_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nReserved */
  (pToData)[2] |= ((pFrom)->m_nReserved & 0x07) <<5;
  (pToData)[1] |= ((pFrom)->m_nReserved >> 3) &0xFF;
  (pToData)[0] |= ((pFrom)->m_nReserved >> 11) &0xFF;

  /* Pack Member: m_nShaped */
  (pToData)[2] |= ((pFrom)->m_nShaped & 0x01) <<4;

  /* Pack Member: m_nDepth */
  (pToData)[2] |= ((pFrom)->m_nDepth & 0x07) <<1;

  /* Pack Member: m_nAnemicAged */
  (pToData)[2] |= ((pFrom)->m_nAnemicAged & 0x01);

  /* Pack Member: m_nQType */
  (pToData)[3] |= ((pFrom)->m_nQType & 0x0f) <<4;

  /* Pack Member: m_nEfAged */
  (pToData)[3] |= ((pFrom)->m_nEfAged & 0x01) <<3;

  /* Pack Member: m_nCreditLevel */
  (pToData)[3] |= ((pFrom)->m_nCreditLevel & 0x01) <<2;

  /* Pack Member: m_nHoldTs */
  (pToData)[3] |= ((pFrom)->m_nHoldTs & 0x01) <<1;

  /* Pack Member: m_nPktLen */
  (pToData)[3] |= ((pFrom)->m_nPktLen & 0x01);
#else
  int i;
  int size = SB_ZF_ZFKAQSPRILUTADDR_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nReserved */
  (pToData)[1] |= ((pFrom)->m_nReserved & 0x07) <<5;
  (pToData)[2] |= ((pFrom)->m_nReserved >> 3) &0xFF;
  (pToData)[3] |= ((pFrom)->m_nReserved >> 11) &0xFF;

  /* Pack Member: m_nShaped */
  (pToData)[1] |= ((pFrom)->m_nShaped & 0x01) <<4;

  /* Pack Member: m_nDepth */
  (pToData)[1] |= ((pFrom)->m_nDepth & 0x07) <<1;

  /* Pack Member: m_nAnemicAged */
  (pToData)[1] |= ((pFrom)->m_nAnemicAged & 0x01);

  /* Pack Member: m_nQType */
  (pToData)[0] |= ((pFrom)->m_nQType & 0x0f) <<4;

  /* Pack Member: m_nEfAged */
  (pToData)[0] |= ((pFrom)->m_nEfAged & 0x01) <<3;

  /* Pack Member: m_nCreditLevel */
  (pToData)[0] |= ((pFrom)->m_nCreditLevel & 0x01) <<2;

  /* Pack Member: m_nHoldTs */
  (pToData)[0] |= ((pFrom)->m_nHoldTs & 0x01) <<1;

  /* Pack Member: m_nPktLen */
  (pToData)[0] |= ((pFrom)->m_nPktLen & 0x01);
#endif

  return SB_ZF_ZFKAQSPRILUTADDR_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaQsPriLutAddr_Unpack(sbZfKaQsPriLutAddr_t *pToStruct,
                          uint8_t *pFromData,
                          uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32_t)  ((pFromData)[2] >> 5) & 0x07;
  (pToStruct)->m_nReserved |=  (uint32_t)  (pFromData)[1] << 3;
  (pToStruct)->m_nReserved |=  (uint32_t)  (pFromData)[0] << 11;

  /* Unpack Member: m_nShaped */
  (pToStruct)->m_nShaped =  (uint8_t)  ((pFromData)[2] >> 4) & 0x01;

  /* Unpack Member: m_nDepth */
  (pToStruct)->m_nDepth =  (uint32_t)  ((pFromData)[2] >> 1) & 0x07;

  /* Unpack Member: m_nAnemicAged */
  (pToStruct)->m_nAnemicAged =  (uint8_t)  ((pFromData)[2] ) & 0x01;

  /* Unpack Member: m_nQType */
  (pToStruct)->m_nQType =  (uint32_t)  ((pFromData)[3] >> 4) & 0x0f;

  /* Unpack Member: m_nEfAged */
  (pToStruct)->m_nEfAged =  (uint8_t)  ((pFromData)[3] >> 3) & 0x01;

  /* Unpack Member: m_nCreditLevel */
  (pToStruct)->m_nCreditLevel =  (uint32_t)  ((pFromData)[3] >> 2) & 0x01;

  /* Unpack Member: m_nHoldTs */
  (pToStruct)->m_nHoldTs =  (uint8_t)  ((pFromData)[3] >> 1) & 0x01;

  /* Unpack Member: m_nPktLen */
  (pToStruct)->m_nPktLen =  (uint32_t)  ((pFromData)[3] ) & 0x01;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32_t)  ((pFromData)[1] >> 5) & 0x07;
  (pToStruct)->m_nReserved |=  (uint32_t)  (pFromData)[2] << 3;
  (pToStruct)->m_nReserved |=  (uint32_t)  (pFromData)[3] << 11;

  /* Unpack Member: m_nShaped */
  (pToStruct)->m_nShaped =  (uint8_t)  ((pFromData)[1] >> 4) & 0x01;

  /* Unpack Member: m_nDepth */
  (pToStruct)->m_nDepth =  (uint32_t)  ((pFromData)[1] >> 1) & 0x07;

  /* Unpack Member: m_nAnemicAged */
  (pToStruct)->m_nAnemicAged =  (uint8_t)  ((pFromData)[1] ) & 0x01;

  /* Unpack Member: m_nQType */
  (pToStruct)->m_nQType =  (uint32_t)  ((pFromData)[0] >> 4) & 0x0f;

  /* Unpack Member: m_nEfAged */
  (pToStruct)->m_nEfAged =  (uint8_t)  ((pFromData)[0] >> 3) & 0x01;

  /* Unpack Member: m_nCreditLevel */
  (pToStruct)->m_nCreditLevel =  (uint32_t)  ((pFromData)[0] >> 2) & 0x01;

  /* Unpack Member: m_nHoldTs */
  (pToStruct)->m_nHoldTs =  (uint8_t)  ((pFromData)[0] >> 1) & 0x01;

  /* Unpack Member: m_nPktLen */
  (pToStruct)->m_nPktLen =  (uint32_t)  ((pFromData)[0] ) & 0x01;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaQsPriLutAddr_InitInstance(sbZfKaQsPriLutAddr_t *pFrame) {

  pFrame->m_nReserved =  (unsigned int)  0;
  pFrame->m_nShaped =  (unsigned int)  0;
  pFrame->m_nDepth =  (unsigned int)  0;
  pFrame->m_nAnemicAged =  (unsigned int)  0;
  pFrame->m_nQType =  (unsigned int)  0;
  pFrame->m_nEfAged =  (unsigned int)  0;
  pFrame->m_nCreditLevel =  (unsigned int)  0;
  pFrame->m_nHoldTs =  (unsigned int)  0;
  pFrame->m_nPktLen =  (unsigned int)  0;

}
