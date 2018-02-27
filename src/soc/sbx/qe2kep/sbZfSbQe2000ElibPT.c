/*
 * $Id: sbZfSbQe2000ElibPT.c 1.3.36.4 Broadcom SDK $
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


#include "sbTypesGlue.h"
#include "sbZfSbQe2000ElibPT.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfSbQe2000ElibPT_Pack(sbZfSbQe2000ElibPT_t *pFrom,
                        uint8_t *pToData,
                        uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_SB_QE2000_ELIB_PT_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_bClassEnb15 */
  (pToData)[7] |= ((pFrom)->m_bClassEnb15 & 0x01) <<7;

  /* Pack Member: m_bClassEnb14 */
  (pToData)[7] |= ((pFrom)->m_bClassEnb14 & 0x01) <<6;

  /* Pack Member: m_bClassEnb13 */
  (pToData)[7] |= ((pFrom)->m_bClassEnb13 & 0x01) <<5;

  /* Pack Member: m_bClassEnb12 */
  (pToData)[7] |= ((pFrom)->m_bClassEnb12 & 0x01) <<4;

  /* Pack Member: m_bClassEnb11 */
  (pToData)[7] |= ((pFrom)->m_bClassEnb11 & 0x01) <<3;

  /* Pack Member: m_bClassEnb10 */
  (pToData)[7] |= ((pFrom)->m_bClassEnb10 & 0x01) <<2;

  /* Pack Member: m_bClassEnb9 */
  (pToData)[7] |= ((pFrom)->m_bClassEnb9 & 0x01) <<1;

  /* Pack Member: m_bClassEnb8 */
  (pToData)[7] |= ((pFrom)->m_bClassEnb8 & 0x01);

  /* Pack Member: m_bClassEnb7 */
  (pToData)[6] |= ((pFrom)->m_bClassEnb7 & 0x01) <<7;

  /* Pack Member: m_bClassEnb6 */
  (pToData)[6] |= ((pFrom)->m_bClassEnb6 & 0x01) <<6;

  /* Pack Member: m_bClassEnb5 */
  (pToData)[6] |= ((pFrom)->m_bClassEnb5 & 0x01) <<5;

  /* Pack Member: m_bClassEnb4 */
  (pToData)[6] |= ((pFrom)->m_bClassEnb4 & 0x01) <<4;

  /* Pack Member: m_bClassEnb3 */
  (pToData)[6] |= ((pFrom)->m_bClassEnb3 & 0x01) <<3;

  /* Pack Member: m_bClassEnb2 */
  (pToData)[6] |= ((pFrom)->m_bClassEnb2 & 0x01) <<2;

  /* Pack Member: m_bClassEnb1 */
  (pToData)[6] |= ((pFrom)->m_bClassEnb1 & 0x01) <<1;

  /* Pack Member: m_bClassEnb0 */
  (pToData)[6] |= ((pFrom)->m_bClassEnb0 & 0x01);

  /* Pack Member: m_nCountTrans */
  (pToData)[5] |= ((pFrom)->m_nCountTrans) & 0xFF;

  /* Pack Member: m_nReserved1 */
  (pToData)[4] |= ((pFrom)->m_nReserved1 & 0x7f) <<1;

  /* Pack Member: m_bPrepend */
  (pToData)[4] |= ((pFrom)->m_bPrepend & 0x01);

  /* Pack Member: m_bInstValid */
  (pToData)[3] |= ((pFrom)->m_bInstValid & 0x01) <<7;

  /* Pack Member: m_Instruction */
  (pToData)[0] |= ((pFrom)->m_Instruction) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_Instruction >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_Instruction >> 16) &0xFF;
  (pToData)[3] |= ((pFrom)->m_Instruction >> 24) & 0x7f;

  return SB_ZF_SB_QE2000_ELIB_PT_ENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfSbQe2000ElibPT_Unpack(sbZfSbQe2000ElibPT_t *pToStruct,
                          uint8_t *pFromData,
                          uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: m_bClassEnb15 */
  (pToStruct)->m_bClassEnb15 =  (uint8_t)  ((pFromData)[7] >> 7) & 0x01;

  /* Unpack Member: m_bClassEnb14 */
  (pToStruct)->m_bClassEnb14 =  (uint8_t)  ((pFromData)[7] >> 6) & 0x01;

  /* Unpack Member: m_bClassEnb13 */
  (pToStruct)->m_bClassEnb13 =  (uint8_t)  ((pFromData)[7] >> 5) & 0x01;

  /* Unpack Member: m_bClassEnb12 */
  (pToStruct)->m_bClassEnb12 =  (uint8_t)  ((pFromData)[7] >> 4) & 0x01;

  /* Unpack Member: m_bClassEnb11 */
  (pToStruct)->m_bClassEnb11 =  (uint8_t)  ((pFromData)[7] >> 3) & 0x01;

  /* Unpack Member: m_bClassEnb10 */
  (pToStruct)->m_bClassEnb10 =  (uint8_t)  ((pFromData)[7] >> 2) & 0x01;

  /* Unpack Member: m_bClassEnb9 */
  (pToStruct)->m_bClassEnb9 =  (uint8_t)  ((pFromData)[7] >> 1) & 0x01;

  /* Unpack Member: m_bClassEnb8 */
  (pToStruct)->m_bClassEnb8 =  (uint8_t)  ((pFromData)[7] ) & 0x01;

  /* Unpack Member: m_bClassEnb7 */
  (pToStruct)->m_bClassEnb7 =  (uint8_t)  ((pFromData)[6] >> 7) & 0x01;

  /* Unpack Member: m_bClassEnb6 */
  (pToStruct)->m_bClassEnb6 =  (uint8_t)  ((pFromData)[6] >> 6) & 0x01;

  /* Unpack Member: m_bClassEnb5 */
  (pToStruct)->m_bClassEnb5 =  (uint8_t)  ((pFromData)[6] >> 5) & 0x01;

  /* Unpack Member: m_bClassEnb4 */
  (pToStruct)->m_bClassEnb4 =  (uint8_t)  ((pFromData)[6] >> 4) & 0x01;

  /* Unpack Member: m_bClassEnb3 */
  (pToStruct)->m_bClassEnb3 =  (uint8_t)  ((pFromData)[6] >> 3) & 0x01;

  /* Unpack Member: m_bClassEnb2 */
  (pToStruct)->m_bClassEnb2 =  (uint8_t)  ((pFromData)[6] >> 2) & 0x01;

  /* Unpack Member: m_bClassEnb1 */
  (pToStruct)->m_bClassEnb1 =  (uint8_t)  ((pFromData)[6] >> 1) & 0x01;

  /* Unpack Member: m_bClassEnb0 */
  (pToStruct)->m_bClassEnb0 =  (uint8_t)  ((pFromData)[6] ) & 0x01;

  /* Unpack Member: m_nCountTrans */
  (pToStruct)->m_nCountTrans =  (int32_t)  (pFromData)[5] ;

  /* Unpack Member: m_nReserved1 */
  (pToStruct)->m_nReserved1 =  (int32_t)  ((pFromData)[4] >> 1) & 0x7f;

  /* Unpack Member: m_bPrepend */
  (pToStruct)->m_bPrepend =  (int32_t)  ((pFromData)[4] ) & 0x01;

  /* Unpack Member: m_bInstValid */
  (pToStruct)->m_bInstValid =  (uint8_t)  ((pFromData)[3] >> 7) & 0x01;

  /* Unpack Member: m_Instruction */
  (pToStruct)->m_Instruction =  (int32_t)  (pFromData)[0] ;
  (pToStruct)->m_Instruction |=  (int32_t)  (pFromData)[1] << 8;
  (pToStruct)->m_Instruction |=  (int32_t)  (pFromData)[2] << 16;
  (pToStruct)->m_Instruction |=  (int32_t)  ((pFromData)[3] & 0x7f) << 24;

}



/* initialize an instance of this zframe */
void
sbZfSbQe2000ElibPT_InitInstance(sbZfSbQe2000ElibPT_t *pFrame) {

  pFrame->m_bClassEnb15 =  (unsigned int)  0;
  pFrame->m_bClassEnb14 =  (unsigned int)  0;
  pFrame->m_bClassEnb13 =  (unsigned int)  0;
  pFrame->m_bClassEnb12 =  (unsigned int)  0;
  pFrame->m_bClassEnb11 =  (unsigned int)  0;
  pFrame->m_bClassEnb10 =  (unsigned int)  0;
  pFrame->m_bClassEnb9 =  (unsigned int)  0;
  pFrame->m_bClassEnb8 =  (unsigned int)  0;
  pFrame->m_bClassEnb7 =  (unsigned int)  0;
  pFrame->m_bClassEnb6 =  (unsigned int)  0;
  pFrame->m_bClassEnb5 =  (unsigned int)  0;
  pFrame->m_bClassEnb4 =  (unsigned int)  0;
  pFrame->m_bClassEnb3 =  (unsigned int)  0;
  pFrame->m_bClassEnb2 =  (unsigned int)  0;
  pFrame->m_bClassEnb1 =  (unsigned int)  0;
  pFrame->m_bClassEnb0 =  (unsigned int)  0;
  pFrame->m_nCountTrans =  (unsigned int)  0;
  pFrame->m_nReserved1 =  (unsigned int)  0;
  pFrame->m_bPrepend =  (unsigned int)  0;
  pFrame->m_bInstValid =  (unsigned int)  0;
  pFrame->m_Instruction =  (unsigned int)  0;

}
