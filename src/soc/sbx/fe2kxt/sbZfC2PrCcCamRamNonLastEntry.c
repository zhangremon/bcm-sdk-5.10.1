/*
 * $Id: sbZfC2PrCcCamRamNonLastEntry.c 1.1.34.4 Broadcom SDK $
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
#include "sbZfC2PrCcCamRamNonLastEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfC2PrCcCamRamNonLastEntry_Pack(sbZfC2PrCcCamRamNonLastEntry_t *pFrom,
                                  uint8_t *pToData,
                                  uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_C2PRCCCAMRAMNONLASTENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uReserve3 */
  (pToData)[8] |= ((pFrom)->m_uReserve3) & 0xFF;
  (pToData)[9] |= ((pFrom)->m_uReserve3 >> 8) & 0x1f;

  /* Pack Member: m_uReserve2 */
  (pToData)[4] |= ((pFrom)->m_uReserve2) & 0xFF;
  (pToData)[5] |= ((pFrom)->m_uReserve2 >> 8) &0xFF;
  (pToData)[6] |= ((pFrom)->m_uReserve2 >> 16) &0xFF;
  (pToData)[7] |= ((pFrom)->m_uReserve2 >> 24) &0xFF;

  /* Pack Member: m_uReserve1 */
  (pToData)[2] |= ((pFrom)->m_uReserve1 & 0x03) <<6;
  (pToData)[3] |= ((pFrom)->m_uReserve1 >> 2) &0xFF;

  /* Pack Member: m_uShift */
  (pToData)[2] |= ((pFrom)->m_uShift & 0x3f);

  /* Pack Member: m_uNextState */
  (pToData)[0] |= ((pFrom)->m_uNextState & 0x3f) <<2;
  (pToData)[1] |= ((pFrom)->m_uNextState >> 6) &0xFF;

  /* Pack Member: m_uE2ECC */
  (pToData)[0] |= ((pFrom)->m_uE2ECC & 0x01) <<1;

  /* Pack Member: m_uLast */
  (pToData)[0] |= ((pFrom)->m_uLast & 0x01);

  return SB_ZF_C2PRCCCAMRAMNONLASTENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfC2PrCcCamRamNonLastEntry_Unpack(sbZfC2PrCcCamRamNonLastEntry_t *pToStruct,
                                    uint8_t *pFromData,
                                    uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: m_uReserve3 */
  (pToStruct)->m_uReserve3 =  (uint32_t)  (pFromData)[8] ;
  (pToStruct)->m_uReserve3 |=  (uint32_t)  ((pFromData)[9] & 0x1f) << 8;

  /* Unpack Member: m_uReserve2 */
  (pToStruct)->m_uReserve2 =  (uint32_t)  (pFromData)[4] ;
  (pToStruct)->m_uReserve2 |=  (uint32_t)  (pFromData)[5] << 8;
  (pToStruct)->m_uReserve2 |=  (uint32_t)  (pFromData)[6] << 16;
  (pToStruct)->m_uReserve2 |=  (uint32_t)  (pFromData)[7] << 24;

  /* Unpack Member: m_uReserve1 */
  (pToStruct)->m_uReserve1 =  (uint32_t)  ((pFromData)[2] >> 6) & 0x03;
  (pToStruct)->m_uReserve1 |=  (uint32_t)  (pFromData)[3] << 2;

  /* Unpack Member: m_uShift */
  (pToStruct)->m_uShift =  (uint32_t)  ((pFromData)[2] ) & 0x3f;

  /* Unpack Member: m_uNextState */
  (pToStruct)->m_uNextState =  (uint32_t)  ((pFromData)[0] >> 2) & 0x3f;
  (pToStruct)->m_uNextState |=  (uint32_t)  (pFromData)[1] << 6;

  /* Unpack Member: m_uE2ECC */
  (pToStruct)->m_uE2ECC =  (uint8_t)  ((pFromData)[0] >> 1) & 0x01;

  /* Unpack Member: m_uLast */
  (pToStruct)->m_uLast =  (uint8_t)  ((pFromData)[0] ) & 0x01;

}



/* initialize an instance of this zframe */
void
sbZfC2PrCcCamRamNonLastEntry_InitInstance(sbZfC2PrCcCamRamNonLastEntry_t *pFrame) {

  pFrame->m_uReserve3 =  (unsigned int)  0;
  pFrame->m_uReserve2 =  (unsigned int)  0;
  pFrame->m_uReserve1 =  (unsigned int)  0;
  pFrame->m_uShift =  (unsigned int)  0;
  pFrame->m_uNextState =  (unsigned int)  0;
  pFrame->m_uE2ECC =  (unsigned int)  0;
  pFrame->m_uLast =  (unsigned int)  0;

}
