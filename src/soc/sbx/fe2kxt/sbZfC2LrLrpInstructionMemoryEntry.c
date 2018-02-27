/*
 * $Id: sbZfC2LrLrpInstructionMemoryEntry.c 1.1.34.4 Broadcom SDK $
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
#include "sbZfC2LrLrpInstructionMemoryEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfC2LrLrpInstructionMemoryEntry_Pack(sbZfC2LrLrpInstructionMemoryEntry_t *pFrom,
                                       uint8_t *pToData,
                                       uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_C2LRLRPINSTRUCTIONMEMORYENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uThread */
  (pToData)[11] |= ((pFrom)->m_uThread & 0x07) <<5;

  /* Pack Member: m_uOpcode */
  (pToData)[10] |= ((pFrom)->m_uOpcode & 0x0f) <<4;
  (pToData)[11] |= ((pFrom)->m_uOpcode >> 4) & 0x1f;

  /* Pack Member: m_uRa */
  (pToData)[10] |= ((pFrom)->m_uRa & 0x01) <<3;

  /* Pack Member: m_uRb */
  (pToData)[10] |= ((pFrom)->m_uRb & 0x01) <<2;

  /* Pack Member: m_uTarget */
  (pToData)[8] |= ((pFrom)->m_uTarget) & 0xFF;
  (pToData)[9] |= ((pFrom)->m_uTarget >> 8) &0xFF;
  (pToData)[10] |= ((pFrom)->m_uTarget >> 16) & 0x03;

  /* Pack Member: m_uSourceA */
  (pToData)[4] |= ((pFrom)->m_uSourceA) & 0xFF;
  (pToData)[5] |= ((pFrom)->m_uSourceA >> 8) &0xFF;
  (pToData)[6] |= ((pFrom)->m_uSourceA >> 16) &0xFF;
  (pToData)[7] |= ((pFrom)->m_uSourceA >> 24) &0xFF;

  /* Pack Member: m_uSourceB */
  (pToData)[2] |= ((pFrom)->m_uSourceB) & 0xFF;
  (pToData)[3] |= ((pFrom)->m_uSourceB >> 8) &0xFF;

  /* Pack Member: m_uReserved */
  (pToData)[0] |= ((pFrom)->m_uReserved) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_uReserved >> 8) &0xFF;

  return SB_ZF_C2LRLRPINSTRUCTIONMEMORYENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfC2LrLrpInstructionMemoryEntry_Unpack(sbZfC2LrLrpInstructionMemoryEntry_t *pToStruct,
                                         uint8_t *pFromData,
                                         uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: m_uThread */
  (pToStruct)->m_uThread =  (uint32_t)  ((pFromData)[11] >> 5) & 0x07;

  /* Unpack Member: m_uOpcode */
  (pToStruct)->m_uOpcode =  (uint32_t)  ((pFromData)[10] >> 4) & 0x0f;
  (pToStruct)->m_uOpcode |=  (uint32_t)  ((pFromData)[11] & 0x1f) << 4;

  /* Unpack Member: m_uRa */
  (pToStruct)->m_uRa =  (uint32_t)  ((pFromData)[10] >> 3) & 0x01;

  /* Unpack Member: m_uRb */
  (pToStruct)->m_uRb =  (uint32_t)  ((pFromData)[10] >> 2) & 0x01;

  /* Unpack Member: m_uTarget */
  (pToStruct)->m_uTarget =  (uint32_t)  (pFromData)[8] ;
  (pToStruct)->m_uTarget |=  (uint32_t)  (pFromData)[9] << 8;
  (pToStruct)->m_uTarget |=  (uint32_t)  ((pFromData)[10] & 0x03) << 16;

  /* Unpack Member: m_uSourceA */
  (pToStruct)->m_uSourceA =  (uint32_t)  (pFromData)[4] ;
  (pToStruct)->m_uSourceA |=  (uint32_t)  (pFromData)[5] << 8;
  (pToStruct)->m_uSourceA |=  (uint32_t)  (pFromData)[6] << 16;
  (pToStruct)->m_uSourceA |=  (uint32_t)  (pFromData)[7] << 24;

  /* Unpack Member: m_uSourceB */
  (pToStruct)->m_uSourceB =  (uint32_t)  (pFromData)[2] ;
  (pToStruct)->m_uSourceB |=  (uint32_t)  (pFromData)[3] << 8;

  /* Unpack Member: m_uReserved */
  (pToStruct)->m_uReserved =  (uint32_t)  (pFromData)[0] ;
  (pToStruct)->m_uReserved |=  (uint32_t)  (pFromData)[1] << 8;

}



/* initialize an instance of this zframe */
void
sbZfC2LrLrpInstructionMemoryEntry_InitInstance(sbZfC2LrLrpInstructionMemoryEntry_t *pFrame) {

  pFrame->m_uThread =  (unsigned int)  0;
  pFrame->m_uOpcode =  (unsigned int)  0;
  pFrame->m_uRa =  (unsigned int)  0;
  pFrame->m_uRb =  (unsigned int)  0;
  pFrame->m_uTarget =  (unsigned int)  0;
  pFrame->m_uSourceA =  (unsigned int)  0;
  pFrame->m_uSourceB =  (unsigned int)  0;
  pFrame->m_uReserved =  (unsigned int)  0;

}
