/*
 * $Id: sbZfCaPmLrMeterRequest.c 1.1.48.4 Broadcom SDK $
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
#include "sbZfCaPmLrMeterRequest.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfCaPmLrMeterRequest_Pack(sbZfCaPmLrMeterRequest_t *pFrom,
                            uint8_t *pToData,
                            uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_CAPMLRMETERREQUEST_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uResv0 */
  (pToData)[7] |= ((pFrom)->m_uResv0 & 0x7f) <<1;

  /* Pack Member: m_uPolicerId */
  (pToData)[4] |= ((pFrom)->m_uPolicerId) & 0xFF;
  (pToData)[5] |= ((pFrom)->m_uPolicerId >> 8) &0xFF;
  (pToData)[6] |= ((pFrom)->m_uPolicerId >> 16) &0xFF;
  (pToData)[7] |= ((pFrom)->m_uPolicerId >> 24) & 0x01;

  /* Pack Member: m_uResv1 */
  (pToData)[2] |= ((pFrom)->m_uResv1 & 0x3f) <<2;
  (pToData)[3] |= ((pFrom)->m_uResv1 >> 6) &0xFF;

  /* Pack Member: m_uBlind */
  (pToData)[2] |= ((pFrom)->m_uBlind & 0x01) <<1;

  /* Pack Member: m_uDropOnRed */
  (pToData)[2] |= ((pFrom)->m_uDropOnRed & 0x01);

  /* Pack Member: m_uPacketLength */
  (pToData)[0] |= ((pFrom)->m_uPacketLength & 0x3f) <<2;
  (pToData)[1] |= ((pFrom)->m_uPacketLength >> 6) &0xFF;

  /* Pack Member: m_uLrpDp */
  (pToData)[0] |= ((pFrom)->m_uLrpDp & 0x03);

  return SB_ZF_CAPMLRMETERREQUEST_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfCaPmLrMeterRequest_Unpack(sbZfCaPmLrMeterRequest_t *pToStruct,
                              uint8_t *pFromData,
                              uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: m_uResv0 */
  (pToStruct)->m_uResv0 =  (uint32_t)  ((pFromData)[7] >> 1) & 0x7f;

  /* Unpack Member: m_uPolicerId */
  (pToStruct)->m_uPolicerId =  (uint32_t)  (pFromData)[4] ;
  (pToStruct)->m_uPolicerId |=  (uint32_t)  (pFromData)[5] << 8;
  (pToStruct)->m_uPolicerId |=  (uint32_t)  (pFromData)[6] << 16;
  (pToStruct)->m_uPolicerId |=  (uint32_t)  ((pFromData)[7] & 0x01) << 24;

  /* Unpack Member: m_uResv1 */
  (pToStruct)->m_uResv1 =  (uint32_t)  ((pFromData)[2] >> 2) & 0x3f;
  (pToStruct)->m_uResv1 |=  (uint32_t)  (pFromData)[3] << 6;

  /* Unpack Member: m_uBlind */
  (pToStruct)->m_uBlind =  (uint8_t)  ((pFromData)[2] >> 1) & 0x01;

  /* Unpack Member: m_uDropOnRed */
  (pToStruct)->m_uDropOnRed =  (uint8_t)  ((pFromData)[2] ) & 0x01;

  /* Unpack Member: m_uPacketLength */
  (pToStruct)->m_uPacketLength =  (uint32_t)  ((pFromData)[0] >> 2) & 0x3f;
  (pToStruct)->m_uPacketLength |=  (uint32_t)  (pFromData)[1] << 6;

  /* Unpack Member: m_uLrpDp */
  (pToStruct)->m_uLrpDp =  (uint8_t)  ((pFromData)[0] ) & 0x03;

}



/* initialize an instance of this zframe */
void
sbZfCaPmLrMeterRequest_InitInstance(sbZfCaPmLrMeterRequest_t *pFrame) {

  pFrame->m_uResv0 =  (unsigned int)  0;
  pFrame->m_uPolicerId =  (unsigned int)  0;
  pFrame->m_uResv1 =  (unsigned int)  0;
  pFrame->m_uBlind =  (unsigned int)  0;
  pFrame->m_uDropOnRed =  (unsigned int)  0;
  pFrame->m_uPacketLength =  (unsigned int)  0;
  pFrame->m_uLrpDp =  (unsigned int)  0;

}
