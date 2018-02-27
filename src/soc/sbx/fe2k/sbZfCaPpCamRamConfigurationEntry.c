/*
 * $Id: sbZfCaPpCamRamConfigurationEntry.c 1.1.48.4 Broadcom SDK $
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
#include "sbZfCaPpCamRamConfigurationEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfCaPpCamRamConfigurationEntry_Pack(sbZfCaPpCamRamConfigurationEntry_t *pFrom,
                                      uint8_t *pToData,
                                      uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uUnused_Spare */
  (pToData)[16] |= ((pFrom)->m_uUnused_Spare & 0x01) <<4;

  /* Pack Member: m_uState */
  (pToData)[13] |= ((pFrom)->m_uState & 0x0f) <<4;
  (pToData)[14] |= ((pFrom)->m_uState >> 4) &0xFF;
  (pToData)[15] |= ((pFrom)->m_uState >> 12) &0xFF;
  (pToData)[16] |= ((pFrom)->m_uState >> 20) & 0x0f;

  /* Pack Member: m_uHashStart */
  (pToData)[13] |= ((pFrom)->m_uHashStart & 0x01) <<3;

  /* Pack Member: m_uHashTemplate */
  (pToData)[12] |= ((pFrom)->m_uHashTemplate & 0x03) <<6;
  (pToData)[13] |= ((pFrom)->m_uHashTemplate >> 2) & 0x07;

  /* Pack Member: m_uCheckerOffset */
  (pToData)[12] |= ((pFrom)->m_uCheckerOffset & 0x3f);

  /* Pack Member: m_uCheckerType */
  (pToData)[11] |= ((pFrom)->m_uCheckerType & 0x0f) <<4;

  /* Pack Member: m_uHeaderAValid */
  (pToData)[11] |= ((pFrom)->m_uHeaderAValid & 0x01) <<3;

  /* Pack Member: m_uHeaderAType */
  (pToData)[10] |= ((pFrom)->m_uHeaderAType & 0x01) <<7;
  (pToData)[11] |= ((pFrom)->m_uHeaderAType >> 1) & 0x07;

  /* Pack Member: m_uHeaderABaseLength */
  (pToData)[10] |= ((pFrom)->m_uHeaderABaseLength & 0x3f) <<1;

  /* Pack Member: m_uHeaderBValid */
  (pToData)[10] |= ((pFrom)->m_uHeaderBValid & 0x01);

  /* Pack Member: m_uHeaderBType */
  (pToData)[9] |= ((pFrom)->m_uHeaderBType & 0x0f) <<4;

  /* Pack Member: m_uHeaderBBaseLength */
  (pToData)[8] |= ((pFrom)->m_uHeaderBBaseLength & 0x03) <<6;
  (pToData)[9] |= ((pFrom)->m_uHeaderBBaseLength >> 2) & 0x0f;

  /* Pack Member: m_uShift */
  (pToData)[8] |= ((pFrom)->m_uShift & 0x3f);

  /* Pack Member: m_uVariableMask */
  (pToData)[4] |= ((pFrom)->m_uVariableMask) & 0xFF;
  (pToData)[5] |= ((pFrom)->m_uVariableMask >> 8) &0xFF;
  (pToData)[6] |= ((pFrom)->m_uVariableMask >> 16) &0xFF;
  (pToData)[7] |= ((pFrom)->m_uVariableMask >> 24) &0xFF;

  /* Pack Member: m_uVariableData */
  (pToData)[0] |= ((pFrom)->m_uVariableData) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_uVariableData >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_uVariableData >> 16) &0xFF;
  (pToData)[3] |= ((pFrom)->m_uVariableData >> 24) &0xFF;

  return SB_ZF_CAPPCAMRAMCONFIGURATIONENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfCaPpCamRamConfigurationEntry_Unpack(sbZfCaPpCamRamConfigurationEntry_t *pToStruct,
                                        uint8_t *pFromData,
                                        uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: m_uUnused_Spare */
  (pToStruct)->m_uUnused_Spare =  (uint32_t)  ((pFromData)[16] >> 4) & 0x01;

  /* Unpack Member: m_uState */
  (pToStruct)->m_uState =  (uint32_t)  ((pFromData)[13] >> 4) & 0x0f;
  (pToStruct)->m_uState |=  (uint32_t)  (pFromData)[14] << 4;
  (pToStruct)->m_uState |=  (uint32_t)  (pFromData)[15] << 12;
  (pToStruct)->m_uState |=  (uint32_t)  ((pFromData)[16] & 0x0f) << 20;

  /* Unpack Member: m_uHashStart */
  (pToStruct)->m_uHashStart =  (uint32_t)  ((pFromData)[13] >> 3) & 0x01;

  /* Unpack Member: m_uHashTemplate */
  (pToStruct)->m_uHashTemplate =  (uint32_t)  ((pFromData)[12] >> 6) & 0x03;
  (pToStruct)->m_uHashTemplate |=  (uint32_t)  ((pFromData)[13] & 0x07) << 2;

  /* Unpack Member: m_uCheckerOffset */
  (pToStruct)->m_uCheckerOffset =  (uint32_t)  ((pFromData)[12] ) & 0x3f;

  /* Unpack Member: m_uCheckerType */
  (pToStruct)->m_uCheckerType =  (uint32_t)  ((pFromData)[11] >> 4) & 0x0f;

  /* Unpack Member: m_uHeaderAValid */
  (pToStruct)->m_uHeaderAValid =  (uint32_t)  ((pFromData)[11] >> 3) & 0x01;

  /* Unpack Member: m_uHeaderAType */
  (pToStruct)->m_uHeaderAType =  (uint32_t)  ((pFromData)[10] >> 7) & 0x01;
  (pToStruct)->m_uHeaderAType |=  (uint32_t)  ((pFromData)[11] & 0x07) << 1;

  /* Unpack Member: m_uHeaderABaseLength */
  (pToStruct)->m_uHeaderABaseLength =  (uint32_t)  ((pFromData)[10] >> 1) & 0x3f;

  /* Unpack Member: m_uHeaderBValid */
  (pToStruct)->m_uHeaderBValid =  (uint32_t)  ((pFromData)[10] ) & 0x01;

  /* Unpack Member: m_uHeaderBType */
  (pToStruct)->m_uHeaderBType =  (uint32_t)  ((pFromData)[9] >> 4) & 0x0f;

  /* Unpack Member: m_uHeaderBBaseLength */
  (pToStruct)->m_uHeaderBBaseLength =  (uint32_t)  ((pFromData)[8] >> 6) & 0x03;
  (pToStruct)->m_uHeaderBBaseLength |=  (uint32_t)  ((pFromData)[9] & 0x0f) << 2;

  /* Unpack Member: m_uShift */
  (pToStruct)->m_uShift =  (uint32_t)  ((pFromData)[8] ) & 0x3f;

  /* Unpack Member: m_uVariableMask */
  (pToStruct)->m_uVariableMask =  (uint32_t)  (pFromData)[4] ;
  (pToStruct)->m_uVariableMask |=  (uint32_t)  (pFromData)[5] << 8;
  (pToStruct)->m_uVariableMask |=  (uint32_t)  (pFromData)[6] << 16;
  (pToStruct)->m_uVariableMask |=  (uint32_t)  (pFromData)[7] << 24;

  /* Unpack Member: m_uVariableData */
  (pToStruct)->m_uVariableData =  (uint32_t)  (pFromData)[0] ;
  (pToStruct)->m_uVariableData |=  (uint32_t)  (pFromData)[1] << 8;
  (pToStruct)->m_uVariableData |=  (uint32_t)  (pFromData)[2] << 16;
  (pToStruct)->m_uVariableData |=  (uint32_t)  (pFromData)[3] << 24;

}



/* initialize an instance of this zframe */
void
sbZfCaPpCamRamConfigurationEntry_InitInstance(sbZfCaPpCamRamConfigurationEntry_t *pFrame) {

  pFrame->m_uUnused_Spare =  (unsigned int)  0;
  pFrame->m_uState =  (unsigned int)  0;
  pFrame->m_uHashStart =  (unsigned int)  0;
  pFrame->m_uHashTemplate =  (unsigned int)  0;
  pFrame->m_uCheckerOffset =  (unsigned int)  0;
  pFrame->m_uCheckerType =  (unsigned int)  0;
  pFrame->m_uHeaderAValid =  (unsigned int)  0;
  pFrame->m_uHeaderAType =  (unsigned int)  0;
  pFrame->m_uHeaderABaseLength =  (unsigned int)  0;
  pFrame->m_uHeaderBValid =  (unsigned int)  0;
  pFrame->m_uHeaderBType =  (unsigned int)  0;
  pFrame->m_uHeaderBBaseLength =  (unsigned int)  0;
  pFrame->m_uShift =  (unsigned int)  0;
  pFrame->m_uVariableMask =  (unsigned int)  0;
  pFrame->m_uVariableData =  (unsigned int)  0;

}
