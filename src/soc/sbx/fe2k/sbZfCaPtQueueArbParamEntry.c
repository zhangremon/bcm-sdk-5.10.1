/*
 * $Id: sbZfCaPtQueueArbParamEntry.c 1.1.48.4 Broadcom SDK $
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
#include "sbZfCaPtQueueArbParamEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfCaPtQueueArbParamEntry_Pack(sbZfCaPtQueueArbParamEntry_t *pFrom,
                                uint8_t *pToData,
                                uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_CAPTQUEUEARBPARAMENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uPpeEnqueue */
  (pToData)[7] |= ((pFrom)->m_uPpeEnqueue & 0x01) <<7;

  /* Pack Member: m_uSpare0 */
  (pToData)[6] |= ((pFrom)->m_uSpare0 & 0x01) <<7;
  (pToData)[7] |= ((pFrom)->m_uSpare0 >> 1) & 0x7f;

  /* Pack Member: m_uWeight */
  (pToData)[6] |= ((pFrom)->m_uWeight & 0x7f);

  /* Pack Member: m_uRxPort */
  (pToData)[5] |= ((pFrom)->m_uRxPort) & 0xFF;

  /* Pack Member: m_uQueue */
  (pToData)[4] |= ((pFrom)->m_uQueue) & 0xFF;

  /* Pack Member: m_uPbSelect */
  (pToData)[3] |= ((pFrom)->m_uPbSelect & 0x01) <<7;

  /* Pack Member: m_uSpare1 */
  (pToData)[2] |= ((pFrom)->m_uSpare1 & 0x0f) <<4;
  (pToData)[3] |= ((pFrom)->m_uSpare1 >> 4) & 0x7f;

  /* Pack Member: m_uDstInterface */
  (pToData)[2] |= ((pFrom)->m_uDstInterface & 0x0f);

  /* Pack Member: m_uSpace2 */
  (pToData)[1] |= ((pFrom)->m_uSpace2 & 0x03) <<6;

  /* Pack Member: m_uDstPort */
  (pToData)[1] |= ((pFrom)->m_uDstPort & 0x3f);

  /* Pack Member: m_uDstQueue */
  (pToData)[0] |= ((pFrom)->m_uDstQueue) & 0xFF;

  return SB_ZF_CAPTQUEUEARBPARAMENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfCaPtQueueArbParamEntry_Unpack(sbZfCaPtQueueArbParamEntry_t *pToStruct,
                                  uint8_t *pFromData,
                                  uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: m_uPpeEnqueue */
  (pToStruct)->m_uPpeEnqueue =  (uint32_t)  ((pFromData)[7] >> 7) & 0x01;

  /* Unpack Member: m_uSpare0 */
  (pToStruct)->m_uSpare0 =  (uint32_t)  ((pFromData)[6] >> 7) & 0x01;
  (pToStruct)->m_uSpare0 |=  (uint32_t)  ((pFromData)[7] & 0x7f) << 1;

  /* Unpack Member: m_uWeight */
  (pToStruct)->m_uWeight =  (uint32_t)  ((pFromData)[6] ) & 0x7f;

  /* Unpack Member: m_uRxPort */
  (pToStruct)->m_uRxPort =  (uint32_t)  (pFromData)[5] ;

  /* Unpack Member: m_uQueue */
  (pToStruct)->m_uQueue =  (uint32_t)  (pFromData)[4] ;

  /* Unpack Member: m_uPbSelect */
  (pToStruct)->m_uPbSelect =  (uint32_t)  ((pFromData)[3] >> 7) & 0x01;

  /* Unpack Member: m_uSpare1 */
  (pToStruct)->m_uSpare1 =  (uint32_t)  ((pFromData)[2] >> 4) & 0x0f;
  (pToStruct)->m_uSpare1 |=  (uint32_t)  ((pFromData)[3] & 0x7f) << 4;

  /* Unpack Member: m_uDstInterface */
  (pToStruct)->m_uDstInterface =  (uint32_t)  ((pFromData)[2] ) & 0x0f;

  /* Unpack Member: m_uSpace2 */
  (pToStruct)->m_uSpace2 =  (uint32_t)  ((pFromData)[1] >> 6) & 0x03;

  /* Unpack Member: m_uDstPort */
  (pToStruct)->m_uDstPort =  (uint32_t)  ((pFromData)[1] ) & 0x3f;

  /* Unpack Member: m_uDstQueue */
  (pToStruct)->m_uDstQueue =  (uint32_t)  (pFromData)[0] ;

}



/* initialize an instance of this zframe */
void
sbZfCaPtQueueArbParamEntry_InitInstance(sbZfCaPtQueueArbParamEntry_t *pFrame) {

  pFrame->m_uPpeEnqueue =  (unsigned int)  0;
  pFrame->m_uSpare0 =  (unsigned int)  0;
  pFrame->m_uWeight =  (unsigned int)  0;
  pFrame->m_uRxPort =  (unsigned int)  0;
  pFrame->m_uQueue =  (unsigned int)  0;
  pFrame->m_uPbSelect =  (unsigned int)  0;
  pFrame->m_uSpare1 =  (unsigned int)  0;
  pFrame->m_uDstInterface =  (unsigned int)  0;
  pFrame->m_uSpace2 =  (unsigned int)  0;
  pFrame->m_uDstPort =  (unsigned int)  0;
  pFrame->m_uDstQueue =  (unsigned int)  0;

}
