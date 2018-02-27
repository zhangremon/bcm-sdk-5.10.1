/*
 * $Id: sbZfSbQe2000ElibCIT.c 1.3.36.4 Broadcom SDK $
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
#include "sbZfSbQe2000ElibCIT.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfSbQe2000ElibCIT_Pack(sbZfSbQe2000ElibCIT_t *pFrom,
                         uint8_t *pToData,
                         uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_SB_QE2000_ELIB_CIT_ENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_Instruction7 */
  (pToData)[28] |= ((pFrom)->m_Instruction7) & 0xFF;
  (pToData)[29] |= ((pFrom)->m_Instruction7 >> 8) &0xFF;
  (pToData)[30] |= ((pFrom)->m_Instruction7 >> 16) &0xFF;
  (pToData)[31] |= ((pFrom)->m_Instruction7 >> 24) &0xFF;

  /* Pack Member: m_Instruction6 */
  (pToData)[24] |= ((pFrom)->m_Instruction6) & 0xFF;
  (pToData)[25] |= ((pFrom)->m_Instruction6 >> 8) &0xFF;
  (pToData)[26] |= ((pFrom)->m_Instruction6 >> 16) &0xFF;
  (pToData)[27] |= ((pFrom)->m_Instruction6 >> 24) &0xFF;

  /* Pack Member: m_Instruction5 */
  (pToData)[20] |= ((pFrom)->m_Instruction5) & 0xFF;
  (pToData)[21] |= ((pFrom)->m_Instruction5 >> 8) &0xFF;
  (pToData)[22] |= ((pFrom)->m_Instruction5 >> 16) &0xFF;
  (pToData)[23] |= ((pFrom)->m_Instruction5 >> 24) &0xFF;

  /* Pack Member: m_Instruction4 */
  (pToData)[16] |= ((pFrom)->m_Instruction4) & 0xFF;
  (pToData)[17] |= ((pFrom)->m_Instruction4 >> 8) &0xFF;
  (pToData)[18] |= ((pFrom)->m_Instruction4 >> 16) &0xFF;
  (pToData)[19] |= ((pFrom)->m_Instruction4 >> 24) &0xFF;

  /* Pack Member: m_Instruction3 */
  (pToData)[12] |= ((pFrom)->m_Instruction3) & 0xFF;
  (pToData)[13] |= ((pFrom)->m_Instruction3 >> 8) &0xFF;
  (pToData)[14] |= ((pFrom)->m_Instruction3 >> 16) &0xFF;
  (pToData)[15] |= ((pFrom)->m_Instruction3 >> 24) &0xFF;

  /* Pack Member: m_Instruction2 */
  (pToData)[8] |= ((pFrom)->m_Instruction2) & 0xFF;
  (pToData)[9] |= ((pFrom)->m_Instruction2 >> 8) &0xFF;
  (pToData)[10] |= ((pFrom)->m_Instruction2 >> 16) &0xFF;
  (pToData)[11] |= ((pFrom)->m_Instruction2 >> 24) &0xFF;

  /* Pack Member: m_Instruction1 */
  (pToData)[4] |= ((pFrom)->m_Instruction1) & 0xFF;
  (pToData)[5] |= ((pFrom)->m_Instruction1 >> 8) &0xFF;
  (pToData)[6] |= ((pFrom)->m_Instruction1 >> 16) &0xFF;
  (pToData)[7] |= ((pFrom)->m_Instruction1 >> 24) &0xFF;

  /* Pack Member: m_Instruction0 */
  (pToData)[0] |= ((pFrom)->m_Instruction0) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_Instruction0 >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_Instruction0 >> 16) &0xFF;
  (pToData)[3] |= ((pFrom)->m_Instruction0 >> 24) &0xFF;

  return SB_ZF_SB_QE2000_ELIB_CIT_ENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfSbQe2000ElibCIT_Unpack(sbZfSbQe2000ElibCIT_t *pToStruct,
                           uint8_t *pFromData,
                           uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: m_Instruction7 */
  (pToStruct)->m_Instruction7 =  (int32_t)  (pFromData)[28] ;
  (pToStruct)->m_Instruction7 |=  (int32_t)  (pFromData)[29] << 8;
  (pToStruct)->m_Instruction7 |=  (int32_t)  (pFromData)[30] << 16;
  (pToStruct)->m_Instruction7 |=  (int32_t)  (pFromData)[31] << 24;

  /* Unpack Member: m_Instruction6 */
  (pToStruct)->m_Instruction6 =  (int32_t)  (pFromData)[24] ;
  (pToStruct)->m_Instruction6 |=  (int32_t)  (pFromData)[25] << 8;
  (pToStruct)->m_Instruction6 |=  (int32_t)  (pFromData)[26] << 16;
  (pToStruct)->m_Instruction6 |=  (int32_t)  (pFromData)[27] << 24;

  /* Unpack Member: m_Instruction5 */
  (pToStruct)->m_Instruction5 =  (int32_t)  (pFromData)[20] ;
  (pToStruct)->m_Instruction5 |=  (int32_t)  (pFromData)[21] << 8;
  (pToStruct)->m_Instruction5 |=  (int32_t)  (pFromData)[22] << 16;
  (pToStruct)->m_Instruction5 |=  (int32_t)  (pFromData)[23] << 24;

  /* Unpack Member: m_Instruction4 */
  (pToStruct)->m_Instruction4 =  (int32_t)  (pFromData)[16] ;
  (pToStruct)->m_Instruction4 |=  (int32_t)  (pFromData)[17] << 8;
  (pToStruct)->m_Instruction4 |=  (int32_t)  (pFromData)[18] << 16;
  (pToStruct)->m_Instruction4 |=  (int32_t)  (pFromData)[19] << 24;

  /* Unpack Member: m_Instruction3 */
  (pToStruct)->m_Instruction3 =  (int32_t)  (pFromData)[12] ;
  (pToStruct)->m_Instruction3 |=  (int32_t)  (pFromData)[13] << 8;
  (pToStruct)->m_Instruction3 |=  (int32_t)  (pFromData)[14] << 16;
  (pToStruct)->m_Instruction3 |=  (int32_t)  (pFromData)[15] << 24;

  /* Unpack Member: m_Instruction2 */
  (pToStruct)->m_Instruction2 =  (int32_t)  (pFromData)[8] ;
  (pToStruct)->m_Instruction2 |=  (int32_t)  (pFromData)[9] << 8;
  (pToStruct)->m_Instruction2 |=  (int32_t)  (pFromData)[10] << 16;
  (pToStruct)->m_Instruction2 |=  (int32_t)  (pFromData)[11] << 24;

  /* Unpack Member: m_Instruction1 */
  (pToStruct)->m_Instruction1 =  (int32_t)  (pFromData)[4] ;
  (pToStruct)->m_Instruction1 |=  (int32_t)  (pFromData)[5] << 8;
  (pToStruct)->m_Instruction1 |=  (int32_t)  (pFromData)[6] << 16;
  (pToStruct)->m_Instruction1 |=  (int32_t)  (pFromData)[7] << 24;

  /* Unpack Member: m_Instruction0 */
  (pToStruct)->m_Instruction0 =  (int32_t)  (pFromData)[0] ;
  (pToStruct)->m_Instruction0 |=  (int32_t)  (pFromData)[1] << 8;
  (pToStruct)->m_Instruction0 |=  (int32_t)  (pFromData)[2] << 16;
  (pToStruct)->m_Instruction0 |=  (int32_t)  (pFromData)[3] << 24;

}



/* initialize an instance of this zframe */
void
sbZfSbQe2000ElibCIT_InitInstance(sbZfSbQe2000ElibCIT_t *pFrame) {

  pFrame->m_Instruction7 =  (unsigned int)  0;
  pFrame->m_Instruction6 =  (unsigned int)  0;
  pFrame->m_Instruction5 =  (unsigned int)  0;
  pFrame->m_Instruction4 =  (unsigned int)  0;
  pFrame->m_Instruction3 =  (unsigned int)  0;
  pFrame->m_Instruction2 =  (unsigned int)  0;
  pFrame->m_Instruction1 =  (unsigned int)  0;
  pFrame->m_Instruction0 =  (unsigned int)  0;

}
