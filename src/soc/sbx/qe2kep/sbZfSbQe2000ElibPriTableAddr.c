/*
 * $Id: sbZfSbQe2000ElibPriTableAddr.c 1.3.36.4 Broadcom SDK $
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
#include "sbZfSbQe2000ElibPriTableAddr.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfSbQe2000ElibPriTableAddr_Pack(sbZfSbQe2000ElibPriTableAddr_t *pFrom,
                                  uint8_t *pToData,
                                  uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_SB_QE2000_ELIB_PRI_TABLE_ADDR_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on big endian */

  /* Pack Member: m_nPort */
  (pToData)[3] |= ((pFrom)->m_nPort & 0x03) <<6;
  (pToData)[2] |= ((pFrom)->m_nPort >> 2) & 0x0f;

  /* Pack Member: m_nCos */
  (pToData)[3] |= ((pFrom)->m_nCos & 0x07) <<3;

  /* Pack Member: m_nDp */
  (pToData)[3] |= ((pFrom)->m_nDp & 0x03) <<1;

  /* Pack Member: m_nEcn */
  (pToData)[3] |= ((pFrom)->m_nEcn & 0x01);

  return SB_ZF_SB_QE2000_ELIB_PRI_TABLE_ADDR_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfSbQe2000ElibPriTableAddr_Unpack(sbZfSbQe2000ElibPriTableAddr_t *pToStruct,
                                    uint8_t *pFromData,
                                    uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on big endian */

  /* Unpack Member: m_nPort */
  (pToStruct)->m_nPort =  (uint32_t)  ((pFromData)[3] >> 6) & 0x03;
  (pToStruct)->m_nPort |=  (uint32_t)  ((pFromData)[2] & 0x0f) << 2;

  /* Unpack Member: m_nCos */
  (pToStruct)->m_nCos =  (uint32_t)  ((pFromData)[3] >> 3) & 0x07;

  /* Unpack Member: m_nDp */
  (pToStruct)->m_nDp =  (uint32_t)  ((pFromData)[3] >> 1) & 0x03;

  /* Unpack Member: m_nEcn */
  (pToStruct)->m_nEcn =  (uint32_t)  ((pFromData)[3] ) & 0x01;

}



/* initialize an instance of this zframe */
void
sbZfSbQe2000ElibPriTableAddr_InitInstance(sbZfSbQe2000ElibPriTableAddr_t *pFrame) {

  pFrame->m_nPort =  (unsigned int)  0;
  pFrame->m_nCos =  (unsigned int)  0;
  pFrame->m_nDp =  (unsigned int)  0;
  pFrame->m_nEcn =  (unsigned int)  0;

}
