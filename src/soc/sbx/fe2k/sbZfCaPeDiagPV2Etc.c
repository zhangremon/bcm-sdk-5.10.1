/*
 * $Id: sbZfCaPeDiagPV2Etc.c 1.1.48.4 Broadcom SDK $
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
#include "sbZfCaPeDiagPV2Etc.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfCaPeDiagPV2Etc_Pack(sbZfCaPeDiagPV2Etc_t *pFrom,
                        uint8_t *pToData,
                        uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_CAPEDIAGPV2ETC_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uQid */
  (pToData)[2] |= ((pFrom)->m_uQid & 0x3f) <<2;
  (pToData)[3] |= ((pFrom)->m_uQid >> 6) &0xFF;

  /* Pack Member: m_uTest */
  (pToData)[2] |= ((pFrom)->m_uTest & 0x01) <<1;

  /* Pack Member: m_uDummy */
  (pToData)[1] |= ((pFrom)->m_uDummy & 0x0f) <<4;
  (pToData)[2] |= ((pFrom)->m_uDummy >> 4) & 0x01;

  /* Pack Member: m_uVid */
  (pToData)[0] |= ((pFrom)->m_uVid) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_uVid >> 8) & 0x0f;

  return SB_ZF_CAPEDIAGPV2ETC_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfCaPeDiagPV2Etc_Unpack(sbZfCaPeDiagPV2Etc_t *pToStruct,
                          uint8_t *pFromData,
                          uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: m_uQid */
  (pToStruct)->m_uQid =  (uint32_t)  ((pFromData)[2] >> 2) & 0x3f;
  (pToStruct)->m_uQid |=  (uint32_t)  (pFromData)[3] << 6;

  /* Unpack Member: m_uTest */
  (pToStruct)->m_uTest =  (uint32_t)  ((pFromData)[2] >> 1) & 0x01;

  /* Unpack Member: m_uDummy */
  (pToStruct)->m_uDummy =  (uint32_t)  ((pFromData)[1] >> 4) & 0x0f;
  (pToStruct)->m_uDummy |=  (uint32_t)  ((pFromData)[2] & 0x01) << 4;

  /* Unpack Member: m_uVid */
  (pToStruct)->m_uVid =  (uint32_t)  (pFromData)[0] ;
  (pToStruct)->m_uVid |=  (uint32_t)  ((pFromData)[1] & 0x0f) << 8;

}



/* initialize an instance of this zframe */
void
sbZfCaPeDiagPV2Etc_InitInstance(sbZfCaPeDiagPV2Etc_t *pFrame) {

  pFrame->m_uQid =  (unsigned int)  0;
  pFrame->m_uTest =  (unsigned int)  0;
  pFrame->m_uDummy =  (unsigned int)  0;
  pFrame->m_uVid =  (unsigned int)  0;

}
