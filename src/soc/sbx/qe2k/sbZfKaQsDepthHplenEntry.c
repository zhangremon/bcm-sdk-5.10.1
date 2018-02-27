/*
 * $Id: sbZfKaQsDepthHplenEntry.c 1.1.44.4 Broadcom SDK $
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
#include "sbZfKaQsDepthHplenEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfKaQsDepthHplenEntry_Pack(sbZfKaQsDepthHplenEntry_t *pFrom,
                             uint8_t *pToData,
                             uint32_t nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAQSDEPTHHPLENENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nReserved */
  (pToData)[3] |= ((pFrom)->m_nReserved & 0x03) <<6;
  (pToData)[2] |= ((pFrom)->m_nReserved >> 2) &0xFF;
  (pToData)[1] |= ((pFrom)->m_nReserved >> 10) &0xFF;
  (pToData)[0] |= ((pFrom)->m_nReserved >> 18) &0xFF;

  /* Pack Member: m_nHplen */
  (pToData)[3] |= ((pFrom)->m_nHplen & 0x03) <<4;

  /* Pack Member: m_nDepth */
  (pToData)[3] |= ((pFrom)->m_nDepth & 0x0f);
#else
  int i;
  int size = SB_ZF_ZFKAQSDEPTHHPLENENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nReserved */
  (pToData)[0] |= ((pFrom)->m_nReserved & 0x03) <<6;
  (pToData)[1] |= ((pFrom)->m_nReserved >> 2) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nReserved >> 10) &0xFF;
  (pToData)[3] |= ((pFrom)->m_nReserved >> 18) &0xFF;

  /* Pack Member: m_nHplen */
  (pToData)[0] |= ((pFrom)->m_nHplen & 0x03) <<4;

  /* Pack Member: m_nDepth */
  (pToData)[0] |= ((pFrom)->m_nDepth & 0x0f);
#endif

  return SB_ZF_ZFKAQSDEPTHHPLENENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaQsDepthHplenEntry_Unpack(sbZfKaQsDepthHplenEntry_t *pToStruct,
                               uint8_t *pFromData,
                               uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32_t)  ((pFromData)[3] >> 6) & 0x03;
  (pToStruct)->m_nReserved |=  (uint32_t)  (pFromData)[2] << 2;
  (pToStruct)->m_nReserved |=  (uint32_t)  (pFromData)[1] << 10;
  (pToStruct)->m_nReserved |=  (uint32_t)  (pFromData)[0] << 18;

  /* Unpack Member: m_nHplen */
  (pToStruct)->m_nHplen =  (uint32_t)  ((pFromData)[3] >> 4) & 0x03;

  /* Unpack Member: m_nDepth */
  (pToStruct)->m_nDepth =  (uint32_t)  ((pFromData)[3] ) & 0x0f;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32_t)  ((pFromData)[0] >> 6) & 0x03;
  (pToStruct)->m_nReserved |=  (uint32_t)  (pFromData)[1] << 2;
  (pToStruct)->m_nReserved |=  (uint32_t)  (pFromData)[2] << 10;
  (pToStruct)->m_nReserved |=  (uint32_t)  (pFromData)[3] << 18;

  /* Unpack Member: m_nHplen */
  (pToStruct)->m_nHplen =  (uint32_t)  ((pFromData)[0] >> 4) & 0x03;

  /* Unpack Member: m_nDepth */
  (pToStruct)->m_nDepth =  (uint32_t)  ((pFromData)[0] ) & 0x0f;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaQsDepthHplenEntry_InitInstance(sbZfKaQsDepthHplenEntry_t *pFrame) {

  pFrame->m_nReserved =  (unsigned int)  0;
  pFrame->m_nHplen =  (unsigned int)  0;
  pFrame->m_nDepth =  (unsigned int)  0;

}
