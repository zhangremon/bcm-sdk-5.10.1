/*
 * $Id: sbZfFe2000QmQueueConfigEntry.c 1.3.36.4 Broadcom SDK $
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
#include "sbZfFe2000QmQueueConfigEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfFe2000QmQueueConfigEntry_Pack(sbZfFe2000QmQueueConfigEntry_t *pFrom,
                                  uint8_t *pToData,
                                  uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_FE2000QMQUEUECONFIGENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uEnforcePoliceMarkings */
  (pToData)[9] |= ((pFrom)->m_uEnforcePoliceMarkings & 0x01) <<6;

  /* Pack Member: m_uDropThresh2 */
  (pToData)[8] |= ((pFrom)->m_uDropThresh2 & 0x1f) <<3;
  (pToData)[9] |= ((pFrom)->m_uDropThresh2 >> 5) & 0x3f;

  /* Pack Member: m_uDropThresh1 */
  (pToData)[7] |= ((pFrom)->m_uDropThresh1) & 0xFF;
  (pToData)[8] |= ((pFrom)->m_uDropThresh1 >> 8) & 0x07;

  /* Pack Member: m_uFlowControlThresh2 */
  (pToData)[5] |= ((pFrom)->m_uFlowControlThresh2 & 0x07) <<5;
  (pToData)[6] |= ((pFrom)->m_uFlowControlThresh2 >> 3) &0xFF;

  /* Pack Member: m_uFlowControlThresh1 */
  (pToData)[4] |= ((pFrom)->m_uFlowControlThresh1 & 0x3f) <<2;
  (pToData)[5] |= ((pFrom)->m_uFlowControlThresh1 >> 6) & 0x1f;

  /* Pack Member: m_uFlowControlEnable */
  (pToData)[4] |= ((pFrom)->m_uFlowControlEnable & 0x01) <<1;

  /* Pack Member: m_uFlowControlTarget */
  (pToData)[4] |= ((pFrom)->m_uFlowControlTarget & 0x01);

  /* Pack Member: m_uFlowControlPort */
  (pToData)[2] |= ((pFrom)->m_uFlowControlPort & 0x01) <<7;
  (pToData)[3] |= ((pFrom)->m_uFlowControlPort >> 1) &0xFF;

  /* Pack Member: m_uMaxPages */
  (pToData)[1] |= ((pFrom)->m_uMaxPages & 0x0f) <<4;
  (pToData)[2] |= ((pFrom)->m_uMaxPages >> 4) & 0x7f;

  /* Pack Member: m_uMinPages */
  (pToData)[0] |= ((pFrom)->m_uMinPages & 0x7f) <<1;
  (pToData)[1] |= ((pFrom)->m_uMinPages >> 7) & 0x0f;

  /* Pack Member: m_uEnable */
  (pToData)[0] |= ((pFrom)->m_uEnable & 0x01);

  return SB_ZF_FE2000QMQUEUECONFIGENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFe2000QmQueueConfigEntry_Unpack(sbZfFe2000QmQueueConfigEntry_t *pToStruct,
                                    uint8_t *pFromData,
                                    uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: m_uEnforcePoliceMarkings */
  (pToStruct)->m_uEnforcePoliceMarkings =  (uint32_t)  ((pFromData)[9] >> 6) & 0x01;

  /* Unpack Member: m_uDropThresh2 */
  (pToStruct)->m_uDropThresh2 =  (uint32_t)  ((pFromData)[8] >> 3) & 0x1f;
  (pToStruct)->m_uDropThresh2 |=  (uint32_t)  ((pFromData)[9] & 0x3f) << 5;

  /* Unpack Member: m_uDropThresh1 */
  (pToStruct)->m_uDropThresh1 =  (uint32_t)  (pFromData)[7] ;
  (pToStruct)->m_uDropThresh1 |=  (uint32_t)  ((pFromData)[8] & 0x07) << 8;

  /* Unpack Member: m_uFlowControlThresh2 */
  (pToStruct)->m_uFlowControlThresh2 =  (uint32_t)  ((pFromData)[5] >> 5) & 0x07;
  (pToStruct)->m_uFlowControlThresh2 |=  (uint32_t)  (pFromData)[6] << 3;

  /* Unpack Member: m_uFlowControlThresh1 */
  (pToStruct)->m_uFlowControlThresh1 =  (uint32_t)  ((pFromData)[4] >> 2) & 0x3f;
  (pToStruct)->m_uFlowControlThresh1 |=  (uint32_t)  ((pFromData)[5] & 0x1f) << 6;

  /* Unpack Member: m_uFlowControlEnable */
  (pToStruct)->m_uFlowControlEnable =  (uint32_t)  ((pFromData)[4] >> 1) & 0x01;

  /* Unpack Member: m_uFlowControlTarget */
  (pToStruct)->m_uFlowControlTarget =  (uint32_t)  ((pFromData)[4] ) & 0x01;

  /* Unpack Member: m_uFlowControlPort */
  (pToStruct)->m_uFlowControlPort =  (uint32_t)  ((pFromData)[2] >> 7) & 0x01;
  (pToStruct)->m_uFlowControlPort |=  (uint32_t)  (pFromData)[3] << 1;

  /* Unpack Member: m_uMaxPages */
  (pToStruct)->m_uMaxPages =  (uint32_t)  ((pFromData)[1] >> 4) & 0x0f;
  (pToStruct)->m_uMaxPages |=  (uint32_t)  ((pFromData)[2] & 0x7f) << 4;

  /* Unpack Member: m_uMinPages */
  (pToStruct)->m_uMinPages =  (uint32_t)  ((pFromData)[0] >> 1) & 0x7f;
  (pToStruct)->m_uMinPages |=  (uint32_t)  ((pFromData)[1] & 0x0f) << 7;

  /* Unpack Member: m_uEnable */
  (pToStruct)->m_uEnable =  (uint32_t)  ((pFromData)[0] ) & 0x01;

}



/* initialize an instance of this zframe */
void
sbZfFe2000QmQueueConfigEntry_InitInstance(sbZfFe2000QmQueueConfigEntry_t *pFrame) {

  pFrame->m_uEnforcePoliceMarkings =  (unsigned int)  0;
  pFrame->m_uDropThresh2 =  (unsigned int)  0;
  pFrame->m_uDropThresh1 =  (unsigned int)  0;
  pFrame->m_uFlowControlThresh2 =  (unsigned int)  0;
  pFrame->m_uFlowControlThresh1 =  (unsigned int)  0;
  pFrame->m_uFlowControlEnable =  (unsigned int)  0;
  pFrame->m_uFlowControlTarget =  (unsigned int)  0;
  pFrame->m_uFlowControlPort =  (unsigned int)  0;
  pFrame->m_uMaxPages =  (unsigned int)  0;
  pFrame->m_uMinPages =  (unsigned int)  0;
  pFrame->m_uEnable =  (unsigned int)  0;

}
