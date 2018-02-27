/*
 * $Id: sbZfC2RceInstruction.c 1.1.34.4 Broadcom SDK $
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
#include "sbZfC2RceInstruction.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfC2RceInstruction_Pack(sbZfC2RceInstruction_t *pFrom,
                          uint8_t *pToData,
                          uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_ZFC2RCEINSTRUCTION_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uOpcode */
  (pToData)[2] |= ((pFrom)->m_uOpcode & 0x0f) <<1;

  /* Pack Member: m_uOpcodeSuspension */
  (pToData)[1] |= ((pFrom)->m_uOpcodeSuspension & 0x07) <<5;
  (pToData)[2] |= ((pFrom)->m_uOpcodeSuspension >> 3) & 0x01;

  /* Pack Member: m_uOpcodeMismatch */
  (pToData)[1] |= ((pFrom)->m_uOpcodeMismatch & 0x0f) <<1;

  /* Pack Member: m_uOperandA */
  (pToData)[0] |= ((pFrom)->m_uOperandA) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_uOperandA >> 8) & 0x01;

  return SB_ZF_ZFC2RCEINSTRUCTION_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfC2RceInstruction_Unpack(sbZfC2RceInstruction_t *pToStruct,
                            uint8_t *pFromData,
                            uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: m_uOpcode */
  (pToStruct)->m_uOpcode =  (uint32_t)  ((pFromData)[2] >> 1) & 0x0f;

  /* Unpack Member: m_uOpcodeSuspension */
  (pToStruct)->m_uOpcodeSuspension =  (uint32_t)  ((pFromData)[1] >> 5) & 0x07;
  (pToStruct)->m_uOpcodeSuspension |=  (uint32_t)  ((pFromData)[2] & 0x01) << 3;

  /* Unpack Member: m_uOpcodeMismatch */
  (pToStruct)->m_uOpcodeMismatch =  (uint32_t)  ((pFromData)[1] >> 1) & 0x0f;

  /* Unpack Member: m_uOperandA */
  (pToStruct)->m_uOperandA =  (uint32_t)  (pFromData)[0] ;
  (pToStruct)->m_uOperandA |=  (uint32_t)  ((pFromData)[1] & 0x01) << 8;

}



/* initialize an instance of this zframe */
void
sbZfC2RceInstruction_InitInstance(sbZfC2RceInstruction_t *pFrame) {

  pFrame->m_uOpcode =  (unsigned int)  0;
  pFrame->m_uOpcodeSuspension =  (unsigned int)  0;
  pFrame->m_uOpcodeMismatch =  (unsigned int)  0;
  pFrame->m_uOperandA =  (unsigned int)  0;

}
