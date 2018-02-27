/*
 * $Id: sbZfCaPmProfileMemoryEntry.c 1.1.48.4 Broadcom SDK $
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
#include "sbZfCaPmProfileMemoryEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfCaPmProfileMemoryEntry_Pack(sbZfCaPmProfileMemoryEntry_t *pFrom,
                                uint8_t *pToData,
                                uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_CAPMPROFILEMEMORYENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uType2 */
  (pToData)[7] |= ((pFrom)->m_uType2 & 0x01) <<6;

  /* Pack Member: m_uDropOnRed */
  (pToData)[7] |= ((pFrom)->m_uDropOnRed & 0x01) <<5;

  /* Pack Member: m_uExcessBucketNoDecrement */
  (pToData)[7] |= ((pFrom)->m_uExcessBucketNoDecrement & 0x01) <<4;

  /* Pack Member: m_uCommittedBucketNoDecrement */
  (pToData)[7] |= ((pFrom)->m_uCommittedBucketNoDecrement & 0x01) <<3;

  /* Pack Member: m_uLengthShift */
  (pToData)[7] |= ((pFrom)->m_uLengthShift & 0x07);

  /* Pack Member: m_uExcessInformationRate */
  (pToData)[5] |= ((pFrom)->m_uExcessInformationRate & 0x0f) <<4;
  (pToData)[6] |= ((pFrom)->m_uExcessInformationRate >> 4) &0xFF;

  /* Pack Member: m_uCommittedInformationRate */
  (pToData)[4] |= ((pFrom)->m_uCommittedInformationRate) & 0xFF;
  (pToData)[5] |= ((pFrom)->m_uCommittedInformationRate >> 8) & 0x0f;

  /* Pack Member: m_uCouplingFlag */
  (pToData)[3] |= ((pFrom)->m_uCouplingFlag & 0x01) <<7;

  /* Pack Member: m_uRfcMode */
  (pToData)[3] |= ((pFrom)->m_uRfcMode & 0x01) <<6;

  /* Pack Member: m_uColorBlind */
  (pToData)[3] |= ((pFrom)->m_uColorBlind & 0x01) <<5;

  /* Pack Member: m_uCommitedBucketSize */
  (pToData)[3] |= ((pFrom)->m_uCommitedBucketSize & 0x1f);

  /* Pack Member: m_uExcessBurstSize */
  (pToData)[1] |= ((pFrom)->m_uExcessBurstSize & 0x0f) <<4;
  (pToData)[2] |= ((pFrom)->m_uExcessBurstSize >> 4) &0xFF;

  /* Pack Member: m_uCommittedBurstSize */
  (pToData)[0] |= ((pFrom)->m_uCommittedBurstSize) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_uCommittedBurstSize >> 8) & 0x0f;

  return SB_ZF_CAPMPROFILEMEMORYENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfCaPmProfileMemoryEntry_Unpack(sbZfCaPmProfileMemoryEntry_t *pToStruct,
                                  uint8_t *pFromData,
                                  uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: m_uType2 */
  (pToStruct)->m_uType2 =  (uint32_t)  ((pFromData)[7] >> 6) & 0x01;

  /* Unpack Member: m_uDropOnRed */
  (pToStruct)->m_uDropOnRed =  (uint32_t)  ((pFromData)[7] >> 5) & 0x01;

  /* Unpack Member: m_uExcessBucketNoDecrement */
  (pToStruct)->m_uExcessBucketNoDecrement =  (uint32_t)  ((pFromData)[7] >> 4) & 0x01;

  /* Unpack Member: m_uCommittedBucketNoDecrement */
  (pToStruct)->m_uCommittedBucketNoDecrement =  (uint32_t)  ((pFromData)[7] >> 3) & 0x01;

  /* Unpack Member: m_uLengthShift */
  (pToStruct)->m_uLengthShift =  (uint32_t)  ((pFromData)[7] ) & 0x07;

  /* Unpack Member: m_uExcessInformationRate */
  (pToStruct)->m_uExcessInformationRate =  (uint32_t)  ((pFromData)[5] >> 4) & 0x0f;
  (pToStruct)->m_uExcessInformationRate |=  (uint32_t)  (pFromData)[6] << 4;

  /* Unpack Member: m_uCommittedInformationRate */
  (pToStruct)->m_uCommittedInformationRate =  (uint32_t)  (pFromData)[4] ;
  (pToStruct)->m_uCommittedInformationRate |=  (uint32_t)  ((pFromData)[5] & 0x0f) << 8;

  /* Unpack Member: m_uCouplingFlag */
  (pToStruct)->m_uCouplingFlag =  (uint32_t)  ((pFromData)[3] >> 7) & 0x01;

  /* Unpack Member: m_uRfcMode */
  (pToStruct)->m_uRfcMode =  (uint32_t)  ((pFromData)[3] >> 6) & 0x01;

  /* Unpack Member: m_uColorBlind */
  (pToStruct)->m_uColorBlind =  (uint32_t)  ((pFromData)[3] >> 5) & 0x01;

  /* Unpack Member: m_uCommitedBucketSize */
  (pToStruct)->m_uCommitedBucketSize =  (uint32_t)  ((pFromData)[3] ) & 0x1f;

  /* Unpack Member: m_uExcessBurstSize */
  (pToStruct)->m_uExcessBurstSize =  (uint32_t)  ((pFromData)[1] >> 4) & 0x0f;
  (pToStruct)->m_uExcessBurstSize |=  (uint32_t)  (pFromData)[2] << 4;

  /* Unpack Member: m_uCommittedBurstSize */
  (pToStruct)->m_uCommittedBurstSize =  (uint32_t)  (pFromData)[0] ;
  (pToStruct)->m_uCommittedBurstSize |=  (uint32_t)  ((pFromData)[1] & 0x0f) << 8;

}



/* initialize an instance of this zframe */
void
sbZfCaPmProfileMemoryEntry_InitInstance(sbZfCaPmProfileMemoryEntry_t *pFrame) {

  pFrame->m_uType2 =  (unsigned int)  0;
  pFrame->m_uDropOnRed =  (unsigned int)  0;
  pFrame->m_uExcessBucketNoDecrement =  (unsigned int)  0;
  pFrame->m_uCommittedBucketNoDecrement =  (unsigned int)  0;
  pFrame->m_uLengthShift =  (unsigned int)  0;
  pFrame->m_uExcessInformationRate =  (unsigned int)  0;
  pFrame->m_uCommittedInformationRate =  (unsigned int)  0;
  pFrame->m_uCouplingFlag =  (unsigned int)  0;
  pFrame->m_uRfcMode =  (unsigned int)  0;
  pFrame->m_uColorBlind =  (unsigned int)  0;
  pFrame->m_uCommitedBucketSize =  (unsigned int)  0;
  pFrame->m_uExcessBurstSize =  (unsigned int)  0;
  pFrame->m_uCommittedBurstSize =  (unsigned int)  0;

}
