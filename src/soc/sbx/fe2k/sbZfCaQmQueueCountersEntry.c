/*
 * $Id: sbZfCaQmQueueCountersEntry.c 1.1.48.4 Broadcom SDK $
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
#include "sbZfCaQmQueueCountersEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfCaQmQueueCountersEntry_Pack(sbZfCaQmQueueCountersEntry_t *pFrom,
                                uint8_t *pToData,
                                uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_CAQMQUEUECOUNTERSENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uPacketCount */
  (pToData)[4] |= ((pFrom)->m_uPacketCount) & 0xFF;
  (pToData)[5] |= ((pFrom)->m_uPacketCount >> 8) &0xFF;
  (pToData)[6] |= ((pFrom)->m_uPacketCount >> 16) &0xFF;
  (pToData)[7] |= ((pFrom)->m_uPacketCount >> 24) &0xFF;

  /* Pack Member: m_uByteCount */
  (pToData)[0] |= ((pFrom)->m_uByteCount) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_uByteCount >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_uByteCount >> 16) &0xFF;
  (pToData)[3] |= ((pFrom)->m_uByteCount >> 24) &0xFF;

  return SB_ZF_CAQMQUEUECOUNTERSENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfCaQmQueueCountersEntry_Unpack(sbZfCaQmQueueCountersEntry_t *pToStruct,
                                  uint8_t *pFromData,
                                  uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: m_uPacketCount */
  (pToStruct)->m_uPacketCount =  (uint32_t)  (pFromData)[4] ;
  (pToStruct)->m_uPacketCount |=  (uint32_t)  (pFromData)[5] << 8;
  (pToStruct)->m_uPacketCount |=  (uint32_t)  (pFromData)[6] << 16;
  (pToStruct)->m_uPacketCount |=  (uint32_t)  (pFromData)[7] << 24;

  /* Unpack Member: m_uByteCount */
  (pToStruct)->m_uByteCount =  (uint32_t)  (pFromData)[0] ;
  (pToStruct)->m_uByteCount |=  (uint32_t)  (pFromData)[1] << 8;
  (pToStruct)->m_uByteCount |=  (uint32_t)  (pFromData)[2] << 16;
  (pToStruct)->m_uByteCount |=  (uint32_t)  (pFromData)[3] << 24;

}



/* initialize an instance of this zframe */
void
sbZfCaQmQueueCountersEntry_InitInstance(sbZfCaQmQueueCountersEntry_t *pFrame) {

  pFrame->m_uPacketCount =  (unsigned int)  0;
  pFrame->m_uByteCount =  (unsigned int)  0;

}
