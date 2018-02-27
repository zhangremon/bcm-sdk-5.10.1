/*
 * $Id: sbZfCaPrTagInfo.c 1.1.48.4 Broadcom SDK $
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
#include "sbZfCaPrTagInfo.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfCaPrTagInfo_Pack(sbZfCaPrTagInfo_t *pFrom,
                     uint8_t *pToData,
                     uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_ZFCAPRTAGINFO_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_bActive */
  (pToData)[5] |= ((pFrom)->m_bActive & 0x01) <<7;

  /* Pack Member: m_bTestPkt */
  (pToData)[5] |= ((pFrom)->m_bTestPkt & 0x01) <<6;

  /* Pack Member: m_nPort */
  (pToData)[5] |= ((pFrom)->m_nPort & 0x3f);

  /* Pack Member: m_bEndOfPkt */
  (pToData)[4] |= ((pFrom)->m_bEndOfPkt & 0x01) <<7;

  /* Pack Member: m_nEnqueueCount */
  (pToData)[4] |= ((pFrom)->m_nEnqueueCount & 0x03) <<5;

  /* Pack Member: m_nLineCount */
  (pToData)[4] |= ((pFrom)->m_nLineCount & 0x07) <<2;

  /* Pack Member: m_nNumPages */
  (pToData)[3] |= ((pFrom)->m_nNumPages & 0x1f) <<3;
  (pToData)[4] |= ((pFrom)->m_nNumPages >> 5) & 0x03;

  /* Pack Member: m_nBuffer */
  (pToData)[1] |= ((pFrom)->m_nBuffer & 0x01) <<7;
  (pToData)[2] |= ((pFrom)->m_nBuffer >> 1) &0xFF;
  (pToData)[3] |= ((pFrom)->m_nBuffer >> 9) & 0x07;

  /* Pack Member: m_nLength */
  (pToData)[0] |= ((pFrom)->m_nLength & 0x7f) <<1;
  (pToData)[1] |= ((pFrom)->m_nLength >> 7) & 0x7f;

  /* Pack Member: m_bDropPkt */
  (pToData)[0] |= ((pFrom)->m_bDropPkt & 0x01);

  return SB_ZF_ZFCAPRTAGINFO_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfCaPrTagInfo_Unpack(sbZfCaPrTagInfo_t *pToStruct,
                       uint8_t *pFromData,
                       uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: m_bActive */
  (pToStruct)->m_bActive =  (uint32_t)  ((pFromData)[5] >> 7) & 0x01;

  /* Unpack Member: m_bTestPkt */
  (pToStruct)->m_bTestPkt =  (uint32_t)  ((pFromData)[5] >> 6) & 0x01;

  /* Unpack Member: m_nPort */
  (pToStruct)->m_nPort =  (uint32_t)  ((pFromData)[5] ) & 0x3f;

  /* Unpack Member: m_bEndOfPkt */
  (pToStruct)->m_bEndOfPkt =  (uint32_t)  ((pFromData)[4] >> 7) & 0x01;

  /* Unpack Member: m_nEnqueueCount */
  (pToStruct)->m_nEnqueueCount =  (uint32_t)  ((pFromData)[4] >> 5) & 0x03;

  /* Unpack Member: m_nLineCount */
  (pToStruct)->m_nLineCount =  (uint32_t)  ((pFromData)[4] >> 2) & 0x07;

  /* Unpack Member: m_nNumPages */
  (pToStruct)->m_nNumPages =  (uint32_t)  ((pFromData)[3] >> 3) & 0x1f;
  (pToStruct)->m_nNumPages |=  (uint32_t)  ((pFromData)[4] & 0x03) << 5;

  /* Unpack Member: m_nBuffer */
  (pToStruct)->m_nBuffer =  (uint32_t)  ((pFromData)[1] >> 7) & 0x01;
  (pToStruct)->m_nBuffer |=  (uint32_t)  (pFromData)[2] << 1;
  (pToStruct)->m_nBuffer |=  (uint32_t)  ((pFromData)[3] & 0x07) << 9;

  /* Unpack Member: m_nLength */
  (pToStruct)->m_nLength =  (uint32_t)  ((pFromData)[0] >> 1) & 0x7f;
  (pToStruct)->m_nLength |=  (uint32_t)  ((pFromData)[1] & 0x7f) << 7;

  /* Unpack Member: m_bDropPkt */
  (pToStruct)->m_bDropPkt =  (uint32_t)  ((pFromData)[0] ) & 0x01;

}



/* initialize an instance of this zframe */
void
sbZfCaPrTagInfo_InitInstance(sbZfCaPrTagInfo_t *pFrame) {

  pFrame->m_bActive =  (unsigned int)  0;
  pFrame->m_bTestPkt =  (unsigned int)  0;
  pFrame->m_nPort =  (unsigned int)  0;
  pFrame->m_bEndOfPkt =  (unsigned int)  0;
  pFrame->m_nEnqueueCount =  (unsigned int)  0;
  pFrame->m_nLineCount =  (unsigned int)  0;
  pFrame->m_nNumPages =  (unsigned int)  0;
  pFrame->m_nBuffer =  (unsigned int)  0;
  pFrame->m_nLength =  (unsigned int)  0;
  pFrame->m_bDropPkt =  (unsigned int)  0;

}
