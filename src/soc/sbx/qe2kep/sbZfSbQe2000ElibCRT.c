/*
 * $Id: sbZfSbQe2000ElibCRT.c 1.3.36.4 Broadcom SDK $
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
#include "sbZfSbQe2000ElibCRT.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfSbQe2000ElibCRT_Pack(sbZfSbQe2000ElibCRT_t *pFrom,
                         uint8_t *pToData,
                         uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_SB_QE2000_ELIB_CRT_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nClass15 */
  (pToData)[7] |= ((pFrom)->m_nClass15 & 0x0f) <<4;

  /* Pack Member: m_nClass14 */
  (pToData)[7] |= ((pFrom)->m_nClass14 & 0x0f);

  /* Pack Member: m_nClass13 */
  (pToData)[6] |= ((pFrom)->m_nClass13 & 0x0f) <<4;

  /* Pack Member: m_nClass12 */
  (pToData)[6] |= ((pFrom)->m_nClass12 & 0x0f);

  /* Pack Member: m_nClass11 */
  (pToData)[5] |= ((pFrom)->m_nClass11 & 0x0f) <<4;

  /* Pack Member: m_nClass10 */
  (pToData)[5] |= ((pFrom)->m_nClass10 & 0x0f);

  /* Pack Member: m_nClass9 */
  (pToData)[4] |= ((pFrom)->m_nClass9 & 0x0f) <<4;

  /* Pack Member: m_nClass8 */
  (pToData)[4] |= ((pFrom)->m_nClass8 & 0x0f);

  /* Pack Member: m_nClass7 */
  (pToData)[3] |= ((pFrom)->m_nClass7 & 0x0f) <<4;

  /* Pack Member: m_nClass6 */
  (pToData)[3] |= ((pFrom)->m_nClass6 & 0x0f);

  /* Pack Member: m_nClass5 */
  (pToData)[2] |= ((pFrom)->m_nClass5 & 0x0f) <<4;

  /* Pack Member: m_nClass4 */
  (pToData)[2] |= ((pFrom)->m_nClass4 & 0x0f);

  /* Pack Member: m_nClass3 */
  (pToData)[1] |= ((pFrom)->m_nClass3 & 0x0f) <<4;

  /* Pack Member: m_nClass2 */
  (pToData)[1] |= ((pFrom)->m_nClass2 & 0x0f);

  /* Pack Member: m_nClass1 */
  (pToData)[0] |= ((pFrom)->m_nClass1 & 0x0f) <<4;

  /* Pack Member: m_nClass0 */
  (pToData)[0] |= ((pFrom)->m_nClass0 & 0x0f);

  return SB_ZF_SB_QE2000_ELIB_CRT_ENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfSbQe2000ElibCRT_Unpack(sbZfSbQe2000ElibCRT_t *pToStruct,
                           uint8_t *pFromData,
                           uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: m_nClass15 */
  (pToStruct)->m_nClass15 =  (uint32_t)  ((pFromData)[7] >> 4) & 0x0f;

  /* Unpack Member: m_nClass14 */
  (pToStruct)->m_nClass14 =  (uint32_t)  ((pFromData)[7] ) & 0x0f;

  /* Unpack Member: m_nClass13 */
  (pToStruct)->m_nClass13 =  (uint32_t)  ((pFromData)[6] >> 4) & 0x0f;

  /* Unpack Member: m_nClass12 */
  (pToStruct)->m_nClass12 =  (uint32_t)  ((pFromData)[6] ) & 0x0f;

  /* Unpack Member: m_nClass11 */
  (pToStruct)->m_nClass11 =  (uint32_t)  ((pFromData)[5] >> 4) & 0x0f;

  /* Unpack Member: m_nClass10 */
  (pToStruct)->m_nClass10 =  (uint32_t)  ((pFromData)[5] ) & 0x0f;

  /* Unpack Member: m_nClass9 */
  (pToStruct)->m_nClass9 =  (uint32_t)  ((pFromData)[4] >> 4) & 0x0f;

  /* Unpack Member: m_nClass8 */
  (pToStruct)->m_nClass8 =  (uint32_t)  ((pFromData)[4] ) & 0x0f;

  /* Unpack Member: m_nClass7 */
  (pToStruct)->m_nClass7 =  (uint32_t)  ((pFromData)[3] >> 4) & 0x0f;

  /* Unpack Member: m_nClass6 */
  (pToStruct)->m_nClass6 =  (uint32_t)  ((pFromData)[3] ) & 0x0f;

  /* Unpack Member: m_nClass5 */
  (pToStruct)->m_nClass5 =  (uint32_t)  ((pFromData)[2] >> 4) & 0x0f;

  /* Unpack Member: m_nClass4 */
  (pToStruct)->m_nClass4 =  (uint32_t)  ((pFromData)[2] ) & 0x0f;

  /* Unpack Member: m_nClass3 */
  (pToStruct)->m_nClass3 =  (uint32_t)  ((pFromData)[1] >> 4) & 0x0f;

  /* Unpack Member: m_nClass2 */
  (pToStruct)->m_nClass2 =  (uint32_t)  ((pFromData)[1] ) & 0x0f;

  /* Unpack Member: m_nClass1 */
  (pToStruct)->m_nClass1 =  (uint32_t)  ((pFromData)[0] >> 4) & 0x0f;

  /* Unpack Member: m_nClass0 */
  (pToStruct)->m_nClass0 =  (uint32_t)  ((pFromData)[0] ) & 0x0f;

}



/* initialize an instance of this zframe */
void
sbZfSbQe2000ElibCRT_InitInstance(sbZfSbQe2000ElibCRT_t *pFrame) {

  pFrame->m_nClass15 =  (unsigned int)  0;
  pFrame->m_nClass14 =  (unsigned int)  0;
  pFrame->m_nClass13 =  (unsigned int)  0;
  pFrame->m_nClass12 =  (unsigned int)  0;
  pFrame->m_nClass11 =  (unsigned int)  0;
  pFrame->m_nClass10 =  (unsigned int)  0;
  pFrame->m_nClass9 =  (unsigned int)  0;
  pFrame->m_nClass8 =  (unsigned int)  0;
  pFrame->m_nClass7 =  (unsigned int)  0;
  pFrame->m_nClass6 =  (unsigned int)  0;
  pFrame->m_nClass5 =  (unsigned int)  0;
  pFrame->m_nClass4 =  (unsigned int)  0;
  pFrame->m_nClass3 =  (unsigned int)  0;
  pFrame->m_nClass2 =  (unsigned int)  0;
  pFrame->m_nClass1 =  (unsigned int)  0;
  pFrame->m_nClass0 =  (unsigned int)  0;

}
