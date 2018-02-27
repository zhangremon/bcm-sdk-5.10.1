/*
 * $Id: sbZfCaPtPte0MirrorPortStateEntry.c 1.1.48.4 Broadcom SDK $
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
#include "sbZfCaPtPte0MirrorPortStateEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfCaPtPte0MirrorPortStateEntry_Pack(sbZfCaPtPte0MirrorPortStateEntry_t *pFrom,
                                      uint8_t *pToData,
                                      uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_CAPTPTE0MIRRORPORTSTATEENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uHdrPresent */
  (pToData)[6] |= ((pFrom)->m_uHdrPresent & 0x01) <<1;

  /* Pack Member: m_uPktLength */
  (pToData)[4] |= ((pFrom)->m_uPktLength & 0x1f) <<3;
  (pToData)[5] |= ((pFrom)->m_uPktLength >> 5) &0xFF;
  (pToData)[6] |= ((pFrom)->m_uPktLength >> 13) & 0x01;

  /* Pack Member: m_uNumPagesHi */
  (pToData)[4] |= ((pFrom)->m_uNumPagesHi & 0x07);

  /* Pack Member: m_uNumPagesLo */
  (pToData)[3] |= ((pFrom)->m_uNumPagesLo & 0x0f) <<4;

  /* Pack Member: m_uBuffer */
  (pToData)[2] |= ((pFrom)->m_uBuffer) & 0xFF;
  (pToData)[3] |= ((pFrom)->m_uBuffer >> 8) & 0x0f;

  /* Pack Member: m_uSourceQueue */
  (pToData)[1] |= ((pFrom)->m_uSourceQueue) & 0xFF;

  /* Pack Member: m_uQueue */
  (pToData)[0] |= ((pFrom)->m_uQueue) & 0xFF;

  return SB_ZF_CAPTPTE0MIRRORPORTSTATEENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfCaPtPte0MirrorPortStateEntry_Unpack(sbZfCaPtPte0MirrorPortStateEntry_t *pToStruct,
                                        uint8_t *pFromData,
                                        uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: m_uHdrPresent */
  (pToStruct)->m_uHdrPresent =  (uint32_t)  ((pFromData)[6] >> 1) & 0x01;

  /* Unpack Member: m_uPktLength */
  (pToStruct)->m_uPktLength =  (uint32_t)  ((pFromData)[4] >> 3) & 0x1f;
  (pToStruct)->m_uPktLength |=  (uint32_t)  (pFromData)[5] << 5;
  (pToStruct)->m_uPktLength |=  (uint32_t)  ((pFromData)[6] & 0x01) << 13;

  /* Unpack Member: m_uNumPagesHi */
  (pToStruct)->m_uNumPagesHi =  (uint32_t)  ((pFromData)[4] ) & 0x07;

  /* Unpack Member: m_uNumPagesLo */
  (pToStruct)->m_uNumPagesLo =  (uint32_t)  ((pFromData)[3] >> 4) & 0x0f;

  /* Unpack Member: m_uBuffer */
  (pToStruct)->m_uBuffer =  (uint32_t)  (pFromData)[2] ;
  (pToStruct)->m_uBuffer |=  (uint32_t)  ((pFromData)[3] & 0x0f) << 8;

  /* Unpack Member: m_uSourceQueue */
  (pToStruct)->m_uSourceQueue =  (uint32_t)  (pFromData)[1] ;

  /* Unpack Member: m_uQueue */
  (pToStruct)->m_uQueue =  (uint32_t)  (pFromData)[0] ;

}



/* initialize an instance of this zframe */
void
sbZfCaPtPte0MirrorPortStateEntry_InitInstance(sbZfCaPtPte0MirrorPortStateEntry_t *pFrame) {

  pFrame->m_uHdrPresent =  (unsigned int)  0;
  pFrame->m_uPktLength =  (unsigned int)  0;
  pFrame->m_uNumPagesHi =  (unsigned int)  0;
  pFrame->m_uNumPagesLo =  (unsigned int)  0;
  pFrame->m_uBuffer =  (unsigned int)  0;
  pFrame->m_uSourceQueue =  (unsigned int)  0;
  pFrame->m_uQueue =  (unsigned int)  0;

}
