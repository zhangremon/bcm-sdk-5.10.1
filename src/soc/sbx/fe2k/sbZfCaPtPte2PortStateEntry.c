/*
 * $Id: sbZfCaPtPte2PortStateEntry.c 1.1.48.4 Broadcom SDK $
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
#include "sbZfCaPtPte2PortStateEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfCaPtPte2PortStateEntry_Pack(sbZfCaPtPte2PortStateEntry_t *pFrom,
                                uint8_t *pToData,
                                uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_CAPTPTE2PORTSTATEENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uPktInProgress */
  (pToData)[11] |= ((pFrom)->m_uPktInProgress & 0x01) <<4;

  /* Pack Member: m_uMirror */
  (pToData)[11] |= ((pFrom)->m_uMirror & 0x01) <<3;

  /* Pack Member: m_uMirrorSrcDrop */
  (pToData)[11] |= ((pFrom)->m_uMirrorSrcDrop & 0x01) <<2;

  /* Pack Member: m_uDoubleDequeued */
  (pToData)[11] |= ((pFrom)->m_uDoubleDequeued & 0x01) <<1;

  /* Pack Member: m_uEopExtraBurst */
  (pToData)[11] |= ((pFrom)->m_uEopExtraBurst & 0x01);

  /* Pack Member: m_uResidueByte */
  (pToData)[10] |= ((pFrom)->m_uResidueByte & 0x0f) <<4;

  /* Pack Member: m_uPktRequestState */
  (pToData)[10] |= ((pFrom)->m_uPktRequestState & 0x0f);

  /* Pack Member: m_uLine */
  (pToData)[9] |= ((pFrom)->m_uLine & 0x01) <<7;

  /* Pack Member: m_uLastPage */
  (pToData)[9] |= ((pFrom)->m_uLastPage & 0x01) <<6;

  /* Pack Member: m_uNextPageState */
  (pToData)[9] |= ((pFrom)->m_uNextPageState & 0x03) <<4;

  /* Pack Member: m_uNextPage */
  (pToData)[8] |= ((pFrom)->m_uNextPage) & 0xFF;
  (pToData)[9] |= ((pFrom)->m_uNextPage >> 8) & 0x0f;

  /* Pack Member: m_uNextLength */
  (pToData)[7] |= ((pFrom)->m_uNextLength) & 0xFF;

  /* Pack Member: m_uPage */
  (pToData)[5] |= ((pFrom)->m_uPage & 0x0f) <<4;
  (pToData)[6] |= ((pFrom)->m_uPage >> 4) &0xFF;

  /* Pack Member: m_uSavedBytes */
  (pToData)[5] |= ((pFrom)->m_uSavedBytes & 0x0f);

  /* Pack Member: m_uRemainingBytes */
  (pToData)[4] |= ((pFrom)->m_uRemainingBytes) & 0xFF;

  /* Pack Member: m_uEop */
  (pToData)[3] |= ((pFrom)->m_uEop & 0x01) <<7;

  /* Pack Member: m_uSop */
  (pToData)[3] |= ((pFrom)->m_uSop & 0x01) <<6;

  /* Pack Member: m_uPktLength */
  (pToData)[2] |= ((pFrom)->m_uPktLength) & 0xFF;
  (pToData)[3] |= ((pFrom)->m_uPktLength >> 8) & 0x3f;

  /* Pack Member: m_uContinueByte */
  (pToData)[1] |= ((pFrom)->m_uContinueByte) & 0xFF;

  /* Pack Member: m_uSourceQueue */
  (pToData)[0] |= ((pFrom)->m_uSourceQueue) & 0xFF;

  return SB_ZF_CAPTPTE2PORTSTATEENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfCaPtPte2PortStateEntry_Unpack(sbZfCaPtPte2PortStateEntry_t *pToStruct,
                                  uint8_t *pFromData,
                                  uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: m_uPktInProgress */
  (pToStruct)->m_uPktInProgress =  (uint32_t)  ((pFromData)[11] >> 4) & 0x01;

  /* Unpack Member: m_uMirror */
  (pToStruct)->m_uMirror =  (uint32_t)  ((pFromData)[11] >> 3) & 0x01;

  /* Unpack Member: m_uMirrorSrcDrop */
  (pToStruct)->m_uMirrorSrcDrop =  (uint32_t)  ((pFromData)[11] >> 2) & 0x01;

  /* Unpack Member: m_uDoubleDequeued */
  (pToStruct)->m_uDoubleDequeued =  (uint32_t)  ((pFromData)[11] >> 1) & 0x01;

  /* Unpack Member: m_uEopExtraBurst */
  (pToStruct)->m_uEopExtraBurst =  (uint32_t)  ((pFromData)[11] ) & 0x01;

  /* Unpack Member: m_uResidueByte */
  (pToStruct)->m_uResidueByte =  (uint32_t)  ((pFromData)[10] >> 4) & 0x0f;

  /* Unpack Member: m_uPktRequestState */
  (pToStruct)->m_uPktRequestState =  (uint32_t)  ((pFromData)[10] ) & 0x0f;

  /* Unpack Member: m_uLine */
  (pToStruct)->m_uLine =  (uint32_t)  ((pFromData)[9] >> 7) & 0x01;

  /* Unpack Member: m_uLastPage */
  (pToStruct)->m_uLastPage =  (uint32_t)  ((pFromData)[9] >> 6) & 0x01;

  /* Unpack Member: m_uNextPageState */
  (pToStruct)->m_uNextPageState =  (uint32_t)  ((pFromData)[9] >> 4) & 0x03;

  /* Unpack Member: m_uNextPage */
  (pToStruct)->m_uNextPage =  (uint32_t)  (pFromData)[8] ;
  (pToStruct)->m_uNextPage |=  (uint32_t)  ((pFromData)[9] & 0x0f) << 8;

  /* Unpack Member: m_uNextLength */
  (pToStruct)->m_uNextLength =  (uint32_t)  (pFromData)[7] ;

  /* Unpack Member: m_uPage */
  (pToStruct)->m_uPage =  (uint32_t)  ((pFromData)[5] >> 4) & 0x0f;
  (pToStruct)->m_uPage |=  (uint32_t)  (pFromData)[6] << 4;

  /* Unpack Member: m_uSavedBytes */
  (pToStruct)->m_uSavedBytes =  (uint32_t)  ((pFromData)[5] ) & 0x0f;

  /* Unpack Member: m_uRemainingBytes */
  (pToStruct)->m_uRemainingBytes =  (uint32_t)  (pFromData)[4] ;

  /* Unpack Member: m_uEop */
  (pToStruct)->m_uEop =  (uint32_t)  ((pFromData)[3] >> 7) & 0x01;

  /* Unpack Member: m_uSop */
  (pToStruct)->m_uSop =  (uint32_t)  ((pFromData)[3] >> 6) & 0x01;

  /* Unpack Member: m_uPktLength */
  (pToStruct)->m_uPktLength =  (uint32_t)  (pFromData)[2] ;
  (pToStruct)->m_uPktLength |=  (uint32_t)  ((pFromData)[3] & 0x3f) << 8;

  /* Unpack Member: m_uContinueByte */
  (pToStruct)->m_uContinueByte =  (uint32_t)  (pFromData)[1] ;

  /* Unpack Member: m_uSourceQueue */
  (pToStruct)->m_uSourceQueue =  (uint32_t)  (pFromData)[0] ;

}



/* initialize an instance of this zframe */
void
sbZfCaPtPte2PortStateEntry_InitInstance(sbZfCaPtPte2PortStateEntry_t *pFrame) {

  pFrame->m_uPktInProgress =  (unsigned int)  0;
  pFrame->m_uMirror =  (unsigned int)  0;
  pFrame->m_uMirrorSrcDrop =  (unsigned int)  0;
  pFrame->m_uDoubleDequeued =  (unsigned int)  0;
  pFrame->m_uEopExtraBurst =  (unsigned int)  0;
  pFrame->m_uResidueByte =  (unsigned int)  0;
  pFrame->m_uPktRequestState =  (unsigned int)  0;
  pFrame->m_uLine =  (unsigned int)  0;
  pFrame->m_uLastPage =  (unsigned int)  0;
  pFrame->m_uNextPageState =  (unsigned int)  0;
  pFrame->m_uNextPage =  (unsigned int)  0;
  pFrame->m_uNextLength =  (unsigned int)  0;
  pFrame->m_uPage =  (unsigned int)  0;
  pFrame->m_uSavedBytes =  (unsigned int)  0;
  pFrame->m_uRemainingBytes =  (unsigned int)  0;
  pFrame->m_uEop =  (unsigned int)  0;
  pFrame->m_uSop =  (unsigned int)  0;
  pFrame->m_uPktLength =  (unsigned int)  0;
  pFrame->m_uContinueByte =  (unsigned int)  0;
  pFrame->m_uSourceQueue =  (unsigned int)  0;

}
