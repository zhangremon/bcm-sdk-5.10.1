/*
 * $Id: sbZfFe2000PtMirrorIndexEntry.c 1.3.36.4 Broadcom SDK $
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
#include "sbZfFe2000PtMirrorIndexEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfFe2000PtMirrorIndexEntry_Pack(sbZfFe2000PtMirrorIndexEntry_t *pFrom,
                                  uint8_t *pToData,
                                  uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_FE2000PTMIRRORINDEXENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uSpare */
  (pToData)[1] |= ((pFrom)->m_uSpare & 0x0f) <<4;
  (pToData)[2] |= ((pFrom)->m_uSpare >> 4) &0xFF;
  (pToData)[3] |= ((pFrom)->m_uSpare >> 12) &0xFF;

  /* Pack Member: m_uOverWriteHdr */
  (pToData)[1] |= ((pFrom)->m_uOverWriteHdr & 0x01) <<3;

  /* Pack Member: m_uHighGigHdr */
  (pToData)[1] |= ((pFrom)->m_uHighGigHdr & 0x01) <<2;

  /* Pack Member: m_uDropSrcPkt */
  (pToData)[1] |= ((pFrom)->m_uDropSrcPkt & 0x01) <<1;

  /* Pack Member: m_uHdrPresent */
  (pToData)[1] |= ((pFrom)->m_uHdrPresent & 0x01);

  /* Pack Member: m_uMirrorQueue */
  (pToData)[0] |= ((pFrom)->m_uMirrorQueue) & 0xFF;

  return SB_ZF_FE2000PTMIRRORINDEXENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFe2000PtMirrorIndexEntry_Unpack(sbZfFe2000PtMirrorIndexEntry_t *pToStruct,
                                    uint8_t *pFromData,
                                    uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: m_uSpare */
  (pToStruct)->m_uSpare =  (uint32_t)  ((pFromData)[1] >> 4) & 0x0f;
  (pToStruct)->m_uSpare |=  (uint32_t)  (pFromData)[2] << 4;
  (pToStruct)->m_uSpare |=  (uint32_t)  (pFromData)[3] << 12;

  /* Unpack Member: m_uOverWriteHdr */
  (pToStruct)->m_uOverWriteHdr =  (uint32_t)  ((pFromData)[1] >> 3) & 0x01;

  /* Unpack Member: m_uHighGigHdr */
  (pToStruct)->m_uHighGigHdr =  (uint32_t)  ((pFromData)[1] >> 2) & 0x01;

  /* Unpack Member: m_uDropSrcPkt */
  (pToStruct)->m_uDropSrcPkt =  (uint32_t)  ((pFromData)[1] >> 1) & 0x01;

  /* Unpack Member: m_uHdrPresent */
  (pToStruct)->m_uHdrPresent =  (uint32_t)  ((pFromData)[1] ) & 0x01;

  /* Unpack Member: m_uMirrorQueue */
  (pToStruct)->m_uMirrorQueue =  (uint32_t)  (pFromData)[0] ;

}



/* initialize an instance of this zframe */
void
sbZfFe2000PtMirrorIndexEntry_InitInstance(sbZfFe2000PtMirrorIndexEntry_t *pFrame) {

  pFrame->m_uSpare =  (unsigned int)  0;
  pFrame->m_uOverWriteHdr =  (unsigned int)  0;
  pFrame->m_uHighGigHdr =  (unsigned int)  0;
  pFrame->m_uDropSrcPkt =  (unsigned int)  0;
  pFrame->m_uHdrPresent =  (unsigned int)  0;
  pFrame->m_uMirrorQueue =  (unsigned int)  0;

}
