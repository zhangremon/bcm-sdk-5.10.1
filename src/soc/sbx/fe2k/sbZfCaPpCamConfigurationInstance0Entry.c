/*
 * $Id: sbZfCaPpCamConfigurationInstance0Entry.c 1.1.48.4 Broadcom SDK $
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
#include "sbZfCaPpCamConfigurationInstance0Entry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfCaPpCamConfigurationInstance0Entry_Pack(sbZfCaPpCamConfigurationInstance0Entry_t *pFrom,
                                            uint8_t *pToData,
                                            uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_CAPPCAMCONFIGURATIONINSTANCE0ENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uMask5 */
  (pToData)[43] |= ((pFrom)->m_uMask5 & 0x3f) <<2;
  (pToData)[44] |= ((pFrom)->m_uMask5 >> 6) &0xFF;
  (pToData)[45] |= ((pFrom)->m_uMask5 >> 14) &0xFF;
  (pToData)[46] |= ((pFrom)->m_uMask5 >> 22) & 0x03;

  /* Pack Member: m_uMask4 */
  (pToData)[39] |= ((pFrom)->m_uMask4 & 0x3f) <<2;
  (pToData)[40] |= ((pFrom)->m_uMask4 >> 6) &0xFF;
  (pToData)[41] |= ((pFrom)->m_uMask4 >> 14) &0xFF;
  (pToData)[42] |= ((pFrom)->m_uMask4 >> 22) &0xFF;
  (pToData)[43] |= ((pFrom)->m_uMask4 >> 30) & 0x03;

  /* Pack Member: m_uMask3 */
  (pToData)[35] |= ((pFrom)->m_uMask3 & 0x3f) <<2;
  (pToData)[36] |= ((pFrom)->m_uMask3 >> 6) &0xFF;
  (pToData)[37] |= ((pFrom)->m_uMask3 >> 14) &0xFF;
  (pToData)[38] |= ((pFrom)->m_uMask3 >> 22) &0xFF;
  (pToData)[39] |= ((pFrom)->m_uMask3 >> 30) & 0x03;

  /* Pack Member: m_uMask2 */
  (pToData)[31] |= ((pFrom)->m_uMask2 & 0x3f) <<2;
  (pToData)[32] |= ((pFrom)->m_uMask2 >> 6) &0xFF;
  (pToData)[33] |= ((pFrom)->m_uMask2 >> 14) &0xFF;
  (pToData)[34] |= ((pFrom)->m_uMask2 >> 22) &0xFF;
  (pToData)[35] |= ((pFrom)->m_uMask2 >> 30) & 0x03;

  /* Pack Member: m_uMask1 */
  (pToData)[27] |= ((pFrom)->m_uMask1 & 0x3f) <<2;
  (pToData)[28] |= ((pFrom)->m_uMask1 >> 6) &0xFF;
  (pToData)[29] |= ((pFrom)->m_uMask1 >> 14) &0xFF;
  (pToData)[30] |= ((pFrom)->m_uMask1 >> 22) &0xFF;
  (pToData)[31] |= ((pFrom)->m_uMask1 >> 30) & 0x03;

  /* Pack Member: m_uMask0 */
  (pToData)[23] |= ((pFrom)->m_uMask0 & 0x3f) <<2;
  (pToData)[24] |= ((pFrom)->m_uMask0 >> 6) &0xFF;
  (pToData)[25] |= ((pFrom)->m_uMask0 >> 14) &0xFF;
  (pToData)[26] |= ((pFrom)->m_uMask0 >> 22) &0xFF;
  (pToData)[27] |= ((pFrom)->m_uMask0 >> 30) & 0x03;

  /* Pack Member: m_uData5 */
  (pToData)[20] |= ((pFrom)->m_uData5 & 0x3f) <<2;
  (pToData)[21] |= ((pFrom)->m_uData5 >> 6) &0xFF;
  (pToData)[22] |= ((pFrom)->m_uData5 >> 14) &0xFF;
  (pToData)[23] |= ((pFrom)->m_uData5 >> 22) & 0x03;

  /* Pack Member: m_uData4 */
  (pToData)[16] |= ((pFrom)->m_uData4 & 0x3f) <<2;
  (pToData)[17] |= ((pFrom)->m_uData4 >> 6) &0xFF;
  (pToData)[18] |= ((pFrom)->m_uData4 >> 14) &0xFF;
  (pToData)[19] |= ((pFrom)->m_uData4 >> 22) &0xFF;
  (pToData)[20] |= ((pFrom)->m_uData4 >> 30) & 0x03;

  /* Pack Member: m_uData3 */
  (pToData)[12] |= ((pFrom)->m_uData3 & 0x3f) <<2;
  (pToData)[13] |= ((pFrom)->m_uData3 >> 6) &0xFF;
  (pToData)[14] |= ((pFrom)->m_uData3 >> 14) &0xFF;
  (pToData)[15] |= ((pFrom)->m_uData3 >> 22) &0xFF;
  (pToData)[16] |= ((pFrom)->m_uData3 >> 30) & 0x03;

  /* Pack Member: m_uData2 */
  (pToData)[8] |= ((pFrom)->m_uData2 & 0x3f) <<2;
  (pToData)[9] |= ((pFrom)->m_uData2 >> 6) &0xFF;
  (pToData)[10] |= ((pFrom)->m_uData2 >> 14) &0xFF;
  (pToData)[11] |= ((pFrom)->m_uData2 >> 22) &0xFF;
  (pToData)[12] |= ((pFrom)->m_uData2 >> 30) & 0x03;

  /* Pack Member: m_uData1 */
  (pToData)[4] |= ((pFrom)->m_uData1 & 0x3f) <<2;
  (pToData)[5] |= ((pFrom)->m_uData1 >> 6) &0xFF;
  (pToData)[6] |= ((pFrom)->m_uData1 >> 14) &0xFF;
  (pToData)[7] |= ((pFrom)->m_uData1 >> 22) &0xFF;
  (pToData)[8] |= ((pFrom)->m_uData1 >> 30) & 0x03;

  /* Pack Member: m_uData0 */
  (pToData)[0] |= ((pFrom)->m_uData0 & 0x3f) <<2;
  (pToData)[1] |= ((pFrom)->m_uData0 >> 6) &0xFF;
  (pToData)[2] |= ((pFrom)->m_uData0 >> 14) &0xFF;
  (pToData)[3] |= ((pFrom)->m_uData0 >> 22) &0xFF;
  (pToData)[4] |= ((pFrom)->m_uData0 >> 30) & 0x03;

  /* Pack Member: m_uValid */
  (pToData)[0] |= ((pFrom)->m_uValid & 0x03);

  return SB_ZF_CAPPCAMCONFIGURATIONINSTANCE0ENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfCaPpCamConfigurationInstance0Entry_Unpack(sbZfCaPpCamConfigurationInstance0Entry_t *pToStruct,
                                              uint8_t *pFromData,
                                              uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: m_uMask5 */
  (pToStruct)->m_uMask5 =  (uint32_t)  ((pFromData)[43] >> 2) & 0x3f;
  (pToStruct)->m_uMask5 |=  (uint32_t)  (pFromData)[44] << 6;
  (pToStruct)->m_uMask5 |=  (uint32_t)  (pFromData)[45] << 14;
  (pToStruct)->m_uMask5 |=  (uint32_t)  ((pFromData)[46] & 0x03) << 22;

  /* Unpack Member: m_uMask4 */
  (pToStruct)->m_uMask4 =  (uint32_t)  ((pFromData)[39] >> 2) & 0x3f;
  (pToStruct)->m_uMask4 |=  (uint32_t)  (pFromData)[40] << 6;
  (pToStruct)->m_uMask4 |=  (uint32_t)  (pFromData)[41] << 14;
  (pToStruct)->m_uMask4 |=  (uint32_t)  (pFromData)[42] << 22;
  (pToStruct)->m_uMask4 |=  (uint32_t)  ((pFromData)[43] & 0x03) << 30;

  /* Unpack Member: m_uMask3 */
  (pToStruct)->m_uMask3 =  (uint32_t)  ((pFromData)[35] >> 2) & 0x3f;
  (pToStruct)->m_uMask3 |=  (uint32_t)  (pFromData)[36] << 6;
  (pToStruct)->m_uMask3 |=  (uint32_t)  (pFromData)[37] << 14;
  (pToStruct)->m_uMask3 |=  (uint32_t)  (pFromData)[38] << 22;
  (pToStruct)->m_uMask3 |=  (uint32_t)  ((pFromData)[39] & 0x03) << 30;

  /* Unpack Member: m_uMask2 */
  (pToStruct)->m_uMask2 =  (uint32_t)  ((pFromData)[31] >> 2) & 0x3f;
  (pToStruct)->m_uMask2 |=  (uint32_t)  (pFromData)[32] << 6;
  (pToStruct)->m_uMask2 |=  (uint32_t)  (pFromData)[33] << 14;
  (pToStruct)->m_uMask2 |=  (uint32_t)  (pFromData)[34] << 22;
  (pToStruct)->m_uMask2 |=  (uint32_t)  ((pFromData)[35] & 0x03) << 30;

  /* Unpack Member: m_uMask1 */
  (pToStruct)->m_uMask1 =  (uint32_t)  ((pFromData)[27] >> 2) & 0x3f;
  (pToStruct)->m_uMask1 |=  (uint32_t)  (pFromData)[28] << 6;
  (pToStruct)->m_uMask1 |=  (uint32_t)  (pFromData)[29] << 14;
  (pToStruct)->m_uMask1 |=  (uint32_t)  (pFromData)[30] << 22;
  (pToStruct)->m_uMask1 |=  (uint32_t)  ((pFromData)[31] & 0x03) << 30;

  /* Unpack Member: m_uMask0 */
  (pToStruct)->m_uMask0 =  (uint32_t)  ((pFromData)[23] >> 2) & 0x3f;
  (pToStruct)->m_uMask0 |=  (uint32_t)  (pFromData)[24] << 6;
  (pToStruct)->m_uMask0 |=  (uint32_t)  (pFromData)[25] << 14;
  (pToStruct)->m_uMask0 |=  (uint32_t)  (pFromData)[26] << 22;
  (pToStruct)->m_uMask0 |=  (uint32_t)  ((pFromData)[27] & 0x03) << 30;

  /* Unpack Member: m_uData5 */
  (pToStruct)->m_uData5 =  (uint32_t)  ((pFromData)[20] >> 2) & 0x3f;
  (pToStruct)->m_uData5 |=  (uint32_t)  (pFromData)[21] << 6;
  (pToStruct)->m_uData5 |=  (uint32_t)  (pFromData)[22] << 14;
  (pToStruct)->m_uData5 |=  (uint32_t)  ((pFromData)[23] & 0x03) << 22;

  /* Unpack Member: m_uData4 */
  (pToStruct)->m_uData4 =  (uint32_t)  ((pFromData)[16] >> 2) & 0x3f;
  (pToStruct)->m_uData4 |=  (uint32_t)  (pFromData)[17] << 6;
  (pToStruct)->m_uData4 |=  (uint32_t)  (pFromData)[18] << 14;
  (pToStruct)->m_uData4 |=  (uint32_t)  (pFromData)[19] << 22;
  (pToStruct)->m_uData4 |=  (uint32_t)  ((pFromData)[20] & 0x03) << 30;

  /* Unpack Member: m_uData3 */
  (pToStruct)->m_uData3 =  (uint32_t)  ((pFromData)[12] >> 2) & 0x3f;
  (pToStruct)->m_uData3 |=  (uint32_t)  (pFromData)[13] << 6;
  (pToStruct)->m_uData3 |=  (uint32_t)  (pFromData)[14] << 14;
  (pToStruct)->m_uData3 |=  (uint32_t)  (pFromData)[15] << 22;
  (pToStruct)->m_uData3 |=  (uint32_t)  ((pFromData)[16] & 0x03) << 30;

  /* Unpack Member: m_uData2 */
  (pToStruct)->m_uData2 =  (uint32_t)  ((pFromData)[8] >> 2) & 0x3f;
  (pToStruct)->m_uData2 |=  (uint32_t)  (pFromData)[9] << 6;
  (pToStruct)->m_uData2 |=  (uint32_t)  (pFromData)[10] << 14;
  (pToStruct)->m_uData2 |=  (uint32_t)  (pFromData)[11] << 22;
  (pToStruct)->m_uData2 |=  (uint32_t)  ((pFromData)[12] & 0x03) << 30;

  /* Unpack Member: m_uData1 */
  (pToStruct)->m_uData1 =  (uint32_t)  ((pFromData)[4] >> 2) & 0x3f;
  (pToStruct)->m_uData1 |=  (uint32_t)  (pFromData)[5] << 6;
  (pToStruct)->m_uData1 |=  (uint32_t)  (pFromData)[6] << 14;
  (pToStruct)->m_uData1 |=  (uint32_t)  (pFromData)[7] << 22;
  (pToStruct)->m_uData1 |=  (uint32_t)  ((pFromData)[8] & 0x03) << 30;

  /* Unpack Member: m_uData0 */
  (pToStruct)->m_uData0 =  (uint32_t)  ((pFromData)[0] >> 2) & 0x3f;
  (pToStruct)->m_uData0 |=  (uint32_t)  (pFromData)[1] << 6;
  (pToStruct)->m_uData0 |=  (uint32_t)  (pFromData)[2] << 14;
  (pToStruct)->m_uData0 |=  (uint32_t)  (pFromData)[3] << 22;
  (pToStruct)->m_uData0 |=  (uint32_t)  ((pFromData)[4] & 0x03) << 30;

  /* Unpack Member: m_uValid */
  (pToStruct)->m_uValid =  (uint32_t)  ((pFromData)[0] ) & 0x03;

}



/* initialize an instance of this zframe */
void
sbZfCaPpCamConfigurationInstance0Entry_InitInstance(sbZfCaPpCamConfigurationInstance0Entry_t *pFrame) {

  pFrame->m_uMask5 =  (unsigned int)  0;
  pFrame->m_uMask4 =  (unsigned int)  0;
  pFrame->m_uMask3 =  (unsigned int)  0;
  pFrame->m_uMask2 =  (unsigned int)  0;
  pFrame->m_uMask1 =  (unsigned int)  0;
  pFrame->m_uMask0 =  (unsigned int)  0;
  pFrame->m_uData5 =  (unsigned int)  0;
  pFrame->m_uData4 =  (unsigned int)  0;
  pFrame->m_uData3 =  (unsigned int)  0;
  pFrame->m_uData2 =  (unsigned int)  0;
  pFrame->m_uData1 =  (unsigned int)  0;
  pFrame->m_uData0 =  (unsigned int)  0;
  pFrame->m_uValid =  (unsigned int)  0;

}
