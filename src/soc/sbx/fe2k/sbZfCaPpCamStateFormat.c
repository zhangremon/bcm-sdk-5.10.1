/*
 * $Id: sbZfCaPpCamStateFormat.c 1.1.48.4 Broadcom SDK $
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
#include "sbZfCaPpCamStateFormat.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfCaPpCamStateFormat_Pack(sbZfCaPpCamStateFormat_t *pFrom,
                            uint8_t *pToData,
                            uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_CAPPCAMSTATEFORMAT_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uResv1 */
  (pToData)[3] |= ((pFrom)->m_uResv1) & 0xFF;

  /* Pack Member: m_bIngress */
  (pToData)[2] |= ((pFrom)->m_bIngress & 0x01) <<7;

  /* Pack Member: m_bEgress */
  (pToData)[2] |= ((pFrom)->m_bEgress & 0x01) <<6;

  /* Pack Member: m_bCycle0 */
  (pToData)[2] |= ((pFrom)->m_bCycle0 & 0x01) <<5;

  /* Pack Member: m_bCycle1 */
  (pToData)[2] |= ((pFrom)->m_bCycle1 & 0x01) <<4;

  /* Pack Member: m_uResv0 */
  (pToData)[2] |= ((pFrom)->m_uResv0 & 0x07);

  /* Pack Member: m_uState */
  (pToData)[1] |= ((pFrom)->m_uState) & 0xFF;

  /* Pack Member: m_uPropBits4 */
  (pToData)[0] |= ((pFrom)->m_uPropBits4 & 0x01) <<7;

  /* Pack Member: m_uPropBits3 */
  (pToData)[0] |= ((pFrom)->m_uPropBits3 & 0x01) <<6;

  /* Pack Member: m_uPropBits2 */
  (pToData)[0] |= ((pFrom)->m_uPropBits2 & 0x01) <<5;

  /* Pack Member: m_uPropBits1 */
  (pToData)[0] |= ((pFrom)->m_uPropBits1 & 0x01) <<4;

  /* Pack Member: m_uPropBits0 */
  (pToData)[0] |= ((pFrom)->m_uPropBits0 & 0x01) <<3;

  /* Pack Member: m_uStreamState */
  (pToData)[0] |= ((pFrom)->m_uStreamState & 0x07);

  return SB_ZF_CAPPCAMSTATEFORMAT_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfCaPpCamStateFormat_Unpack(sbZfCaPpCamStateFormat_t *pToStruct,
                              uint8_t *pFromData,
                              uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: m_uResv1 */
  (pToStruct)->m_uResv1 =  (uint32_t)  (pFromData)[3] ;

  /* Unpack Member: m_bIngress */
  (pToStruct)->m_bIngress =  (uint8_t)  ((pFromData)[2] >> 7) & 0x01;

  /* Unpack Member: m_bEgress */
  (pToStruct)->m_bEgress =  (uint8_t)  ((pFromData)[2] >> 6) & 0x01;

  /* Unpack Member: m_bCycle0 */
  (pToStruct)->m_bCycle0 =  (uint8_t)  ((pFromData)[2] >> 5) & 0x01;

  /* Unpack Member: m_bCycle1 */
  (pToStruct)->m_bCycle1 =  (uint8_t)  ((pFromData)[2] >> 4) & 0x01;

  /* Unpack Member: m_uResv0 */
  (pToStruct)->m_uResv0 =  (uint8_t)  ((pFromData)[2] ) & 0x07;

  /* Unpack Member: m_uState */
  (pToStruct)->m_uState =  (uint32_t)  (pFromData)[1] ;

  /* Unpack Member: m_uPropBits4 */
  (pToStruct)->m_uPropBits4 =  (uint32_t)  ((pFromData)[0] >> 7) & 0x01;

  /* Unpack Member: m_uPropBits3 */
  (pToStruct)->m_uPropBits3 =  (uint32_t)  ((pFromData)[0] >> 6) & 0x01;

  /* Unpack Member: m_uPropBits2 */
  (pToStruct)->m_uPropBits2 =  (uint32_t)  ((pFromData)[0] >> 5) & 0x01;

  /* Unpack Member: m_uPropBits1 */
  (pToStruct)->m_uPropBits1 =  (uint32_t)  ((pFromData)[0] >> 4) & 0x01;

  /* Unpack Member: m_uPropBits0 */
  (pToStruct)->m_uPropBits0 =  (uint32_t)  ((pFromData)[0] >> 3) & 0x01;

  /* Unpack Member: m_uStreamState */
  (pToStruct)->m_uStreamState =  (uint32_t)  ((pFromData)[0] ) & 0x07;

}



/* initialize an instance of this zframe */
void
sbZfCaPpCamStateFormat_InitInstance(sbZfCaPpCamStateFormat_t *pFrame) {

  pFrame->m_uResv1 =  (unsigned int)  0;
  pFrame->m_bIngress =  (unsigned int)  0;
  pFrame->m_bEgress =  (unsigned int)  0;
  pFrame->m_bCycle0 =  (unsigned int)  0;
  pFrame->m_bCycle1 =  (unsigned int)  0;
  pFrame->m_uResv0 =  (unsigned int)  0;
  pFrame->m_uState =  (unsigned int)  0;
  pFrame->m_uPropBits4 =  (unsigned int)  0;
  pFrame->m_uPropBits3 =  (unsigned int)  0;
  pFrame->m_uPropBits2 =  (unsigned int)  0;
  pFrame->m_uPropBits1 =  (unsigned int)  0;
  pFrame->m_uPropBits0 =  (unsigned int)  0;
  pFrame->m_uStreamState =  (unsigned int)  0;

}
