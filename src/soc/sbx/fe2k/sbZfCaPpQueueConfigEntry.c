/*
 * $Id: sbZfCaPpQueueConfigEntry.c 1.1.48.4 Broadcom SDK $
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
#include "sbZfCaPpQueueConfigEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfCaPpQueueConfigEntry_Pack(sbZfCaPpQueueConfigEntry_t *pFrom,
                              uint8_t *pToData,
                              uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_CAPPQUEUECONFIGENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_udata2 */
  (pToData)[8] |= ((pFrom)->m_udata2) & 0xFF;
  (pToData)[9] |= ((pFrom)->m_udata2 >> 8) &0xFF;
  (pToData)[10] |= ((pFrom)->m_udata2 >> 16) &0xFF;
  (pToData)[11] |= ((pFrom)->m_udata2 >> 24) &0xFF;

  /* Pack Member: m_udata1 */
  (pToData)[4] |= ((pFrom)->m_udata1) & 0xFF;
  (pToData)[5] |= ((pFrom)->m_udata1 >> 8) &0xFF;
  (pToData)[6] |= ((pFrom)->m_udata1 >> 16) &0xFF;
  (pToData)[7] |= ((pFrom)->m_udata1 >> 24) &0xFF;

  /* Pack Member: m_udata0 */
  (pToData)[0] |= ((pFrom)->m_udata0 & 0x0f) <<4;
  (pToData)[1] |= ((pFrom)->m_udata0 >> 4) &0xFF;
  (pToData)[2] |= ((pFrom)->m_udata0 >> 12) &0xFF;
  (pToData)[3] |= ((pFrom)->m_udata0 >> 20) &0xFF;

  /* Pack Member: m_uInitialHdrType */
  (pToData)[0] |= ((pFrom)->m_uInitialHdrType & 0x0f);

  return SB_ZF_CAPPQUEUECONFIGENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfCaPpQueueConfigEntry_Unpack(sbZfCaPpQueueConfigEntry_t *pToStruct,
                                uint8_t *pFromData,
                                uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: m_udata2 */
  (pToStruct)->m_udata2 =  (uint32_t)  (pFromData)[8] ;
  (pToStruct)->m_udata2 |=  (uint32_t)  (pFromData)[9] << 8;
  (pToStruct)->m_udata2 |=  (uint32_t)  (pFromData)[10] << 16;
  (pToStruct)->m_udata2 |=  (uint32_t)  (pFromData)[11] << 24;

  /* Unpack Member: m_udata1 */
  (pToStruct)->m_udata1 =  (uint32_t)  (pFromData)[4] ;
  (pToStruct)->m_udata1 |=  (uint32_t)  (pFromData)[5] << 8;
  (pToStruct)->m_udata1 |=  (uint32_t)  (pFromData)[6] << 16;
  (pToStruct)->m_udata1 |=  (uint32_t)  (pFromData)[7] << 24;

  /* Unpack Member: m_udata0 */
  (pToStruct)->m_udata0 =  (uint32_t)  ((pFromData)[0] >> 4) & 0x0f;
  (pToStruct)->m_udata0 |=  (uint32_t)  (pFromData)[1] << 4;
  (pToStruct)->m_udata0 |=  (uint32_t)  (pFromData)[2] << 12;
  (pToStruct)->m_udata0 |=  (uint32_t)  (pFromData)[3] << 20;

  /* Unpack Member: m_uInitialHdrType */
  (pToStruct)->m_uInitialHdrType =  (uint32_t)  ((pFromData)[0] ) & 0x0f;

}



/* initialize an instance of this zframe */
void
sbZfCaPpQueueConfigEntry_InitInstance(sbZfCaPpQueueConfigEntry_t *pFrame) {

  pFrame->m_udata2 =  (unsigned int)  0;
  pFrame->m_udata1 =  (unsigned int)  0;
  pFrame->m_udata0 =  (unsigned int)  0;
  pFrame->m_uInitialHdrType =  (unsigned int)  0;

}
