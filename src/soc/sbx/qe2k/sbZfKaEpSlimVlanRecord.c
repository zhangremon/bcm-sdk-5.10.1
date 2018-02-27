/*
 * $Id: sbZfKaEpSlimVlanRecord.c 1.1.44.4 Broadcom SDK $
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
#include "sbZfKaEpSlimVlanRecord.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfKaEpSlimVlanRecord_Pack(sbZfKaEpSlimVlanRecord_t *pFrom,
                            uint8_t *pToData,
                            uint32_t nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAEPSLIMVLANRECORD_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nVid0 */
  (pToData)[29] |= ((pFrom)->m_nVid0 & 0x0f) <<4;
  (pToData)[28] |= ((pFrom)->m_nVid0 >> 4) &0xFF;

  /* Pack Member: m_nVid1 */
  (pToData)[21] |= ((pFrom)->m_nVid1 & 0x0f) <<4;
  (pToData)[20] |= ((pFrom)->m_nVid1 >> 4) &0xFF;

  /* Pack Member: m_nOffset */
  (pToData)[21] |= ((pFrom)->m_nOffset & 0x0f);

  /* Pack Member: m_nVlanPacketCounter */
  (pToData)[15] |= ((pFrom)->m_nVlanPacketCounter & 0x1f) <<3;
  (pToData)[14] |= ((pFrom)->m_nVlanPacketCounter >> 5) &0xFF;
  (pToData)[13] |= ((pFrom)->m_nVlanPacketCounter >> 13) &0xFF;
  (pToData)[12] |= ((pFrom)->m_nVlanPacketCounter >> 21) &0xFF;

  /* Pack Member: m_nnVlanByteCounter */
  (pToData)[11] |= ((pFrom)->m_nnVlanByteCounter) & 0xFF;
  (pToData)[10] |= ((pFrom)->m_nnVlanByteCounter >> 8) &0xFF;
  (pToData)[9] |= ((pFrom)->m_nnVlanByteCounter >> 16) &0xFF;
  (pToData)[8] |= ((pFrom)->m_nnVlanByteCounter >> 24) &0xFF;
  (pToData)[15] |= ((pFrom)->m_nnVlanByteCounter >> 32) & 0x07;

  /* Pack Member: m_nDropPacketCounter */
  (pToData)[7] |= ((pFrom)->m_nDropPacketCounter & 0x1f) <<3;
  (pToData)[6] |= ((pFrom)->m_nDropPacketCounter >> 5) &0xFF;
  (pToData)[5] |= ((pFrom)->m_nDropPacketCounter >> 13) &0xFF;
  (pToData)[4] |= ((pFrom)->m_nDropPacketCounter >> 21) &0xFF;

  /* Pack Member: m_nnDropByteCounter */
  (pToData)[3] |= ((pFrom)->m_nnDropByteCounter) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nnDropByteCounter >> 8) &0xFF;
  (pToData)[1] |= ((pFrom)->m_nnDropByteCounter >> 16) &0xFF;
  (pToData)[0] |= ((pFrom)->m_nnDropByteCounter >> 24) &0xFF;
  (pToData)[7] |= ((pFrom)->m_nnDropByteCounter >> 32) & 0x07;

  /* Pack Member: m_nPortState_0 */
  (pToData)[27] |= ((pFrom)->m_nPortState_0 & 0x03);

  /* Pack Member: m_nPortState_1 */
  (pToData)[27] |= ((pFrom)->m_nPortState_1 & 0x03) <<2;

  /* Pack Member: m_nPortState_2 */
  (pToData)[27] |= ((pFrom)->m_nPortState_2 & 0x03) <<4;

  /* Pack Member: m_nPortState_3 */
  (pToData)[27] |= ((pFrom)->m_nPortState_3 & 0x03) <<6;

  /* Pack Member: m_nPortState_4 */
  (pToData)[26] |= ((pFrom)->m_nPortState_4 & 0x03);

  /* Pack Member: m_nPortState_5 */
  (pToData)[26] |= ((pFrom)->m_nPortState_5 & 0x03) <<2;

  /* Pack Member: m_nPortState_6 */
  (pToData)[26] |= ((pFrom)->m_nPortState_6 & 0x03) <<4;

  /* Pack Member: m_nPortState_7 */
  (pToData)[26] |= ((pFrom)->m_nPortState_7 & 0x03) <<6;

  /* Pack Member: m_nPortState_8 */
  (pToData)[25] |= ((pFrom)->m_nPortState_8 & 0x03);

  /* Pack Member: m_nPortState_9 */
  (pToData)[25] |= ((pFrom)->m_nPortState_9 & 0x03) <<2;

  /* Pack Member: m_nPortState_10 */
  (pToData)[25] |= ((pFrom)->m_nPortState_10 & 0x03) <<4;

  /* Pack Member: m_nPortState_11 */
  (pToData)[25] |= ((pFrom)->m_nPortState_11 & 0x03) <<6;

  /* Pack Member: m_nPortState_12 */
  (pToData)[24] |= ((pFrom)->m_nPortState_12 & 0x03);

  /* Pack Member: m_nPortState_13 */
  (pToData)[24] |= ((pFrom)->m_nPortState_13 & 0x03) <<2;

  /* Pack Member: m_nPortState_14 */
  (pToData)[24] |= ((pFrom)->m_nPortState_14 & 0x03) <<4;

  /* Pack Member: m_nPortState_15 */
  (pToData)[24] |= ((pFrom)->m_nPortState_15 & 0x03) <<6;

  /* Pack Member: m_nPortState_16 */
  (pToData)[31] |= ((pFrom)->m_nPortState_16 & 0x03);

  /* Pack Member: m_nPortState_17 */
  (pToData)[31] |= ((pFrom)->m_nPortState_17 & 0x03) <<2;

  /* Pack Member: m_nPortState_18 */
  (pToData)[31] |= ((pFrom)->m_nPortState_18 & 0x03) <<4;

  /* Pack Member: m_nPortState_19 */
  (pToData)[31] |= ((pFrom)->m_nPortState_19 & 0x03) <<6;

  /* Pack Member: m_nPortState_20 */
  (pToData)[30] |= ((pFrom)->m_nPortState_20 & 0x03);

  /* Pack Member: m_nPortState_21 */
  (pToData)[30] |= ((pFrom)->m_nPortState_21 & 0x03) <<2;

  /* Pack Member: m_nPortState_22 */
  (pToData)[30] |= ((pFrom)->m_nPortState_22 & 0x03) <<4;

  /* Pack Member: m_nPortState_23 */
  (pToData)[30] |= ((pFrom)->m_nPortState_23 & 0x03) <<6;

  /* Pack Member: m_nPortState_24 */
  (pToData)[19] |= ((pFrom)->m_nPortState_24 & 0x03);

  /* Pack Member: m_nPortState_25 */
  (pToData)[19] |= ((pFrom)->m_nPortState_25 & 0x03) <<2;

  /* Pack Member: m_nPortState_26 */
  (pToData)[19] |= ((pFrom)->m_nPortState_26 & 0x03) <<4;

  /* Pack Member: m_nPortState_27 */
  (pToData)[19] |= ((pFrom)->m_nPortState_27 & 0x03) <<6;

  /* Pack Member: m_nPortState_28 */
  (pToData)[18] |= ((pFrom)->m_nPortState_28 & 0x03);

  /* Pack Member: m_nPortState_29 */
  (pToData)[18] |= ((pFrom)->m_nPortState_29 & 0x03) <<2;

  /* Pack Member: m_nPortState_30 */
  (pToData)[18] |= ((pFrom)->m_nPortState_30 & 0x03) <<4;

  /* Pack Member: m_nPortState_31 */
  (pToData)[18] |= ((pFrom)->m_nPortState_31 & 0x03) <<6;

  /* Pack Member: m_nPortState_32 */
  (pToData)[17] |= ((pFrom)->m_nPortState_32 & 0x03);

  /* Pack Member: m_nPortState_33 */
  (pToData)[17] |= ((pFrom)->m_nPortState_33 & 0x03) <<2;

  /* Pack Member: m_nPortState_34 */
  (pToData)[17] |= ((pFrom)->m_nPortState_34 & 0x03) <<4;

  /* Pack Member: m_nPortState_35 */
  (pToData)[17] |= ((pFrom)->m_nPortState_35 & 0x03) <<6;

  /* Pack Member: m_nPortState_36 */
  (pToData)[16] |= ((pFrom)->m_nPortState_36 & 0x03);

  /* Pack Member: m_nPortState_37 */
  (pToData)[16] |= ((pFrom)->m_nPortState_37 & 0x03) <<2;

  /* Pack Member: m_nPortState_38 */
  (pToData)[16] |= ((pFrom)->m_nPortState_38 & 0x03) <<4;

  /* Pack Member: m_nPortState_39 */
  (pToData)[16] |= ((pFrom)->m_nPortState_39 & 0x03) <<6;

  /* Pack Member: m_nPortState_40 */
  (pToData)[23] |= ((pFrom)->m_nPortState_40 & 0x03);

  /* Pack Member: m_nPortState_41 */
  (pToData)[23] |= ((pFrom)->m_nPortState_41 & 0x03) <<2;

  /* Pack Member: m_nPortState_42 */
  (pToData)[23] |= ((pFrom)->m_nPortState_42 & 0x03) <<4;

  /* Pack Member: m_nPortState_43 */
  (pToData)[23] |= ((pFrom)->m_nPortState_43 & 0x03) <<6;

  /* Pack Member: m_nPortState_44 */
  (pToData)[22] |= ((pFrom)->m_nPortState_44 & 0x03);

  /* Pack Member: m_nPortState_45 */
  (pToData)[22] |= ((pFrom)->m_nPortState_45 & 0x03) <<2;

  /* Pack Member: m_nPortState_46 */
  (pToData)[22] |= ((pFrom)->m_nPortState_46 & 0x03) <<4;

  /* Pack Member: m_nPortState_47 */
  (pToData)[22] |= ((pFrom)->m_nPortState_47 & 0x03) <<6;

  /* Pack Member: m_nPortState_48 */
  (pToData)[29] |= ((pFrom)->m_nPortState_48 & 0x03);

  /* Pack Member: m_nPortState_49 */
  (pToData)[29] |= ((pFrom)->m_nPortState_49 & 0x03) <<2;
#else
  int i;
  int size = SB_ZF_ZFKAEPSLIMVLANRECORD_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nVid0 */
  (pToData)[30] |= ((pFrom)->m_nVid0 & 0x0f) <<4;
  (pToData)[31] |= ((pFrom)->m_nVid0 >> 4) &0xFF;

  /* Pack Member: m_nVid1 */
  (pToData)[22] |= ((pFrom)->m_nVid1 & 0x0f) <<4;
  (pToData)[23] |= ((pFrom)->m_nVid1 >> 4) &0xFF;

  /* Pack Member: m_nOffset */
  (pToData)[22] |= ((pFrom)->m_nOffset & 0x0f);

  /* Pack Member: m_nVlanPacketCounter */
  (pToData)[12] |= ((pFrom)->m_nVlanPacketCounter & 0x1f) <<3;
  (pToData)[13] |= ((pFrom)->m_nVlanPacketCounter >> 5) &0xFF;
  (pToData)[14] |= ((pFrom)->m_nVlanPacketCounter >> 13) &0xFF;
  (pToData)[15] |= ((pFrom)->m_nVlanPacketCounter >> 21) &0xFF;

  /* Pack Member: m_nnVlanByteCounter */
  (pToData)[8] |= ((pFrom)->m_nnVlanByteCounter) & 0xFF;
  (pToData)[9] |= ((pFrom)->m_nnVlanByteCounter >> 8) &0xFF;
  (pToData)[10] |= ((pFrom)->m_nnVlanByteCounter >> 16) &0xFF;
  (pToData)[11] |= ((pFrom)->m_nnVlanByteCounter >> 24) &0xFF;
  (pToData)[12] |= ((pFrom)->m_nnVlanByteCounter >> 32) & 0x07;

  /* Pack Member: m_nDropPacketCounter */
  (pToData)[4] |= ((pFrom)->m_nDropPacketCounter & 0x1f) <<3;
  (pToData)[5] |= ((pFrom)->m_nDropPacketCounter >> 5) &0xFF;
  (pToData)[6] |= ((pFrom)->m_nDropPacketCounter >> 13) &0xFF;
  (pToData)[7] |= ((pFrom)->m_nDropPacketCounter >> 21) &0xFF;

  /* Pack Member: m_nnDropByteCounter */
  (pToData)[0] |= ((pFrom)->m_nnDropByteCounter) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nnDropByteCounter >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nnDropByteCounter >> 16) &0xFF;
  (pToData)[3] |= ((pFrom)->m_nnDropByteCounter >> 24) &0xFF;
  (pToData)[4] |= ((pFrom)->m_nnDropByteCounter >> 32) & 0x07;

  /* Pack Member: m_nPortState_0 */
  (pToData)[24] |= ((pFrom)->m_nPortState_0 & 0x03);

  /* Pack Member: m_nPortState_1 */
  (pToData)[24] |= ((pFrom)->m_nPortState_1 & 0x03) <<2;

  /* Pack Member: m_nPortState_2 */
  (pToData)[24] |= ((pFrom)->m_nPortState_2 & 0x03) <<4;

  /* Pack Member: m_nPortState_3 */
  (pToData)[24] |= ((pFrom)->m_nPortState_3 & 0x03) <<6;

  /* Pack Member: m_nPortState_4 */
  (pToData)[25] |= ((pFrom)->m_nPortState_4 & 0x03);

  /* Pack Member: m_nPortState_5 */
  (pToData)[25] |= ((pFrom)->m_nPortState_5 & 0x03) <<2;

  /* Pack Member: m_nPortState_6 */
  (pToData)[25] |= ((pFrom)->m_nPortState_6 & 0x03) <<4;

  /* Pack Member: m_nPortState_7 */
  (pToData)[25] |= ((pFrom)->m_nPortState_7 & 0x03) <<6;

  /* Pack Member: m_nPortState_8 */
  (pToData)[26] |= ((pFrom)->m_nPortState_8 & 0x03);

  /* Pack Member: m_nPortState_9 */
  (pToData)[26] |= ((pFrom)->m_nPortState_9 & 0x03) <<2;

  /* Pack Member: m_nPortState_10 */
  (pToData)[26] |= ((pFrom)->m_nPortState_10 & 0x03) <<4;

  /* Pack Member: m_nPortState_11 */
  (pToData)[26] |= ((pFrom)->m_nPortState_11 & 0x03) <<6;

  /* Pack Member: m_nPortState_12 */
  (pToData)[27] |= ((pFrom)->m_nPortState_12 & 0x03);

  /* Pack Member: m_nPortState_13 */
  (pToData)[27] |= ((pFrom)->m_nPortState_13 & 0x03) <<2;

  /* Pack Member: m_nPortState_14 */
  (pToData)[27] |= ((pFrom)->m_nPortState_14 & 0x03) <<4;

  /* Pack Member: m_nPortState_15 */
  (pToData)[27] |= ((pFrom)->m_nPortState_15 & 0x03) <<6;

  /* Pack Member: m_nPortState_16 */
  (pToData)[28] |= ((pFrom)->m_nPortState_16 & 0x03);

  /* Pack Member: m_nPortState_17 */
  (pToData)[28] |= ((pFrom)->m_nPortState_17 & 0x03) <<2;

  /* Pack Member: m_nPortState_18 */
  (pToData)[28] |= ((pFrom)->m_nPortState_18 & 0x03) <<4;

  /* Pack Member: m_nPortState_19 */
  (pToData)[28] |= ((pFrom)->m_nPortState_19 & 0x03) <<6;

  /* Pack Member: m_nPortState_20 */
  (pToData)[29] |= ((pFrom)->m_nPortState_20 & 0x03);

  /* Pack Member: m_nPortState_21 */
  (pToData)[29] |= ((pFrom)->m_nPortState_21 & 0x03) <<2;

  /* Pack Member: m_nPortState_22 */
  (pToData)[29] |= ((pFrom)->m_nPortState_22 & 0x03) <<4;

  /* Pack Member: m_nPortState_23 */
  (pToData)[29] |= ((pFrom)->m_nPortState_23 & 0x03) <<6;

  /* Pack Member: m_nPortState_24 */
  (pToData)[16] |= ((pFrom)->m_nPortState_24 & 0x03);

  /* Pack Member: m_nPortState_25 */
  (pToData)[16] |= ((pFrom)->m_nPortState_25 & 0x03) <<2;

  /* Pack Member: m_nPortState_26 */
  (pToData)[16] |= ((pFrom)->m_nPortState_26 & 0x03) <<4;

  /* Pack Member: m_nPortState_27 */
  (pToData)[16] |= ((pFrom)->m_nPortState_27 & 0x03) <<6;

  /* Pack Member: m_nPortState_28 */
  (pToData)[17] |= ((pFrom)->m_nPortState_28 & 0x03);

  /* Pack Member: m_nPortState_29 */
  (pToData)[17] |= ((pFrom)->m_nPortState_29 & 0x03) <<2;

  /* Pack Member: m_nPortState_30 */
  (pToData)[17] |= ((pFrom)->m_nPortState_30 & 0x03) <<4;

  /* Pack Member: m_nPortState_31 */
  (pToData)[17] |= ((pFrom)->m_nPortState_31 & 0x03) <<6;

  /* Pack Member: m_nPortState_32 */
  (pToData)[18] |= ((pFrom)->m_nPortState_32 & 0x03);

  /* Pack Member: m_nPortState_33 */
  (pToData)[18] |= ((pFrom)->m_nPortState_33 & 0x03) <<2;

  /* Pack Member: m_nPortState_34 */
  (pToData)[18] |= ((pFrom)->m_nPortState_34 & 0x03) <<4;

  /* Pack Member: m_nPortState_35 */
  (pToData)[18] |= ((pFrom)->m_nPortState_35 & 0x03) <<6;

  /* Pack Member: m_nPortState_36 */
  (pToData)[19] |= ((pFrom)->m_nPortState_36 & 0x03);

  /* Pack Member: m_nPortState_37 */
  (pToData)[19] |= ((pFrom)->m_nPortState_37 & 0x03) <<2;

  /* Pack Member: m_nPortState_38 */
  (pToData)[19] |= ((pFrom)->m_nPortState_38 & 0x03) <<4;

  /* Pack Member: m_nPortState_39 */
  (pToData)[19] |= ((pFrom)->m_nPortState_39 & 0x03) <<6;

  /* Pack Member: m_nPortState_40 */
  (pToData)[20] |= ((pFrom)->m_nPortState_40 & 0x03);

  /* Pack Member: m_nPortState_41 */
  (pToData)[20] |= ((pFrom)->m_nPortState_41 & 0x03) <<2;

  /* Pack Member: m_nPortState_42 */
  (pToData)[20] |= ((pFrom)->m_nPortState_42 & 0x03) <<4;

  /* Pack Member: m_nPortState_43 */
  (pToData)[20] |= ((pFrom)->m_nPortState_43 & 0x03) <<6;

  /* Pack Member: m_nPortState_44 */
  (pToData)[21] |= ((pFrom)->m_nPortState_44 & 0x03);

  /* Pack Member: m_nPortState_45 */
  (pToData)[21] |= ((pFrom)->m_nPortState_45 & 0x03) <<2;

  /* Pack Member: m_nPortState_46 */
  (pToData)[21] |= ((pFrom)->m_nPortState_46 & 0x03) <<4;

  /* Pack Member: m_nPortState_47 */
  (pToData)[21] |= ((pFrom)->m_nPortState_47 & 0x03) <<6;

  /* Pack Member: m_nPortState_48 */
  (pToData)[30] |= ((pFrom)->m_nPortState_48 & 0x03);

  /* Pack Member: m_nPortState_49 */
  (pToData)[30] |= ((pFrom)->m_nPortState_49 & 0x03) <<2;
#endif

  return SB_ZF_ZFKAEPSLIMVLANRECORD_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaEpSlimVlanRecord_Unpack(sbZfKaEpSlimVlanRecord_t *pToStruct,
                              uint8_t *pFromData,
                              uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nVid0 */
  (pToStruct)->m_nVid0 =  (uint32_t)  ((pFromData)[29] >> 4) & 0x0f;
  (pToStruct)->m_nVid0 |=  (uint32_t)  (pFromData)[28] << 4;

  /* Unpack Member: m_nVid1 */
  (pToStruct)->m_nVid1 =  (uint32_t)  ((pFromData)[21] >> 4) & 0x0f;
  (pToStruct)->m_nVid1 |=  (uint32_t)  (pFromData)[20] << 4;

  /* Unpack Member: m_nOffset */
  (pToStruct)->m_nOffset =  (uint32_t)  ((pFromData)[21] ) & 0x0f;

  /* Unpack Member: m_nVlanPacketCounter */
  (pToStruct)->m_nVlanPacketCounter =  (uint32_t)  ((pFromData)[15] >> 3) & 0x1f;
  (pToStruct)->m_nVlanPacketCounter |=  (uint32_t)  (pFromData)[14] << 5;
  (pToStruct)->m_nVlanPacketCounter |=  (uint32_t)  (pFromData)[13] << 13;
  (pToStruct)->m_nVlanPacketCounter |=  (uint32_t)  (pFromData)[12] << 21;

  /* Unpack Member: m_nnVlanByteCounter */
  COMPILER_64_SET((pToStruct)->m_nnVlanByteCounter, 0,  (unsigned int) (pFromData)[11]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnVlanByteCounter;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[10]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnVlanByteCounter;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[9]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnVlanByteCounter;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[8]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnVlanByteCounter;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[15]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nDropPacketCounter */
  (pToStruct)->m_nDropPacketCounter =  (uint32_t)  ((pFromData)[7] >> 3) & 0x1f;
  (pToStruct)->m_nDropPacketCounter |=  (uint32_t)  (pFromData)[6] << 5;
  (pToStruct)->m_nDropPacketCounter |=  (uint32_t)  (pFromData)[5] << 13;
  (pToStruct)->m_nDropPacketCounter |=  (uint32_t)  (pFromData)[4] << 21;

  /* Unpack Member: m_nnDropByteCounter */
  COMPILER_64_SET((pToStruct)->m_nnDropByteCounter, 0,  (unsigned int) (pFromData)[3]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnDropByteCounter;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[2]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnDropByteCounter;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[1]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnDropByteCounter;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[0]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnDropByteCounter;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[7]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nPortState_0 */
  (pToStruct)->m_nPortState_0 =  (uint32_t)  ((pFromData)[27] ) & 0x03;

  /* Unpack Member: m_nPortState_1 */
  (pToStruct)->m_nPortState_1 =  (uint32_t)  ((pFromData)[27] >> 2) & 0x03;

  /* Unpack Member: m_nPortState_2 */
  (pToStruct)->m_nPortState_2 =  (uint32_t)  ((pFromData)[27] >> 4) & 0x03;

  /* Unpack Member: m_nPortState_3 */
  (pToStruct)->m_nPortState_3 =  (uint32_t)  ((pFromData)[27] >> 6) & 0x03;

  /* Unpack Member: m_nPortState_4 */
  (pToStruct)->m_nPortState_4 =  (uint32_t)  ((pFromData)[26] ) & 0x03;

  /* Unpack Member: m_nPortState_5 */
  (pToStruct)->m_nPortState_5 =  (uint32_t)  ((pFromData)[26] >> 2) & 0x03;

  /* Unpack Member: m_nPortState_6 */
  (pToStruct)->m_nPortState_6 =  (uint32_t)  ((pFromData)[26] >> 4) & 0x03;

  /* Unpack Member: m_nPortState_7 */
  (pToStruct)->m_nPortState_7 =  (uint32_t)  ((pFromData)[26] >> 6) & 0x03;

  /* Unpack Member: m_nPortState_8 */
  (pToStruct)->m_nPortState_8 =  (uint32_t)  ((pFromData)[25] ) & 0x03;

  /* Unpack Member: m_nPortState_9 */
  (pToStruct)->m_nPortState_9 =  (uint32_t)  ((pFromData)[25] >> 2) & 0x03;

  /* Unpack Member: m_nPortState_10 */
  (pToStruct)->m_nPortState_10 =  (uint32_t)  ((pFromData)[25] >> 4) & 0x03;

  /* Unpack Member: m_nPortState_11 */
  (pToStruct)->m_nPortState_11 =  (uint32_t)  ((pFromData)[25] >> 6) & 0x03;

  /* Unpack Member: m_nPortState_12 */
  (pToStruct)->m_nPortState_12 =  (uint32_t)  ((pFromData)[24] ) & 0x03;

  /* Unpack Member: m_nPortState_13 */
  (pToStruct)->m_nPortState_13 =  (uint32_t)  ((pFromData)[24] >> 2) & 0x03;

  /* Unpack Member: m_nPortState_14 */
  (pToStruct)->m_nPortState_14 =  (uint32_t)  ((pFromData)[24] >> 4) & 0x03;

  /* Unpack Member: m_nPortState_15 */
  (pToStruct)->m_nPortState_15 =  (uint32_t)  ((pFromData)[24] >> 6) & 0x03;

  /* Unpack Member: m_nPortState_16 */
  (pToStruct)->m_nPortState_16 =  (uint32_t)  ((pFromData)[31] ) & 0x03;

  /* Unpack Member: m_nPortState_17 */
  (pToStruct)->m_nPortState_17 =  (uint32_t)  ((pFromData)[31] >> 2) & 0x03;

  /* Unpack Member: m_nPortState_18 */
  (pToStruct)->m_nPortState_18 =  (uint32_t)  ((pFromData)[31] >> 4) & 0x03;

  /* Unpack Member: m_nPortState_19 */
  (pToStruct)->m_nPortState_19 =  (uint32_t)  ((pFromData)[31] >> 6) & 0x03;

  /* Unpack Member: m_nPortState_20 */
  (pToStruct)->m_nPortState_20 =  (uint32_t)  ((pFromData)[30] ) & 0x03;

  /* Unpack Member: m_nPortState_21 */
  (pToStruct)->m_nPortState_21 =  (uint32_t)  ((pFromData)[30] >> 2) & 0x03;

  /* Unpack Member: m_nPortState_22 */
  (pToStruct)->m_nPortState_22 =  (uint32_t)  ((pFromData)[30] >> 4) & 0x03;

  /* Unpack Member: m_nPortState_23 */
  (pToStruct)->m_nPortState_23 =  (uint32_t)  ((pFromData)[30] >> 6) & 0x03;

  /* Unpack Member: m_nPortState_24 */
  (pToStruct)->m_nPortState_24 =  (uint32_t)  ((pFromData)[19] ) & 0x03;

  /* Unpack Member: m_nPortState_25 */
  (pToStruct)->m_nPortState_25 =  (uint32_t)  ((pFromData)[19] >> 2) & 0x03;

  /* Unpack Member: m_nPortState_26 */
  (pToStruct)->m_nPortState_26 =  (uint32_t)  ((pFromData)[19] >> 4) & 0x03;

  /* Unpack Member: m_nPortState_27 */
  (pToStruct)->m_nPortState_27 =  (uint32_t)  ((pFromData)[19] >> 6) & 0x03;

  /* Unpack Member: m_nPortState_28 */
  (pToStruct)->m_nPortState_28 =  (uint32_t)  ((pFromData)[18] ) & 0x03;

  /* Unpack Member: m_nPortState_29 */
  (pToStruct)->m_nPortState_29 =  (uint32_t)  ((pFromData)[18] >> 2) & 0x03;

  /* Unpack Member: m_nPortState_30 */
  (pToStruct)->m_nPortState_30 =  (uint32_t)  ((pFromData)[18] >> 4) & 0x03;

  /* Unpack Member: m_nPortState_31 */
  (pToStruct)->m_nPortState_31 =  (uint32_t)  ((pFromData)[18] >> 6) & 0x03;

  /* Unpack Member: m_nPortState_32 */
  (pToStruct)->m_nPortState_32 =  (uint32_t)  ((pFromData)[17] ) & 0x03;

  /* Unpack Member: m_nPortState_33 */
  (pToStruct)->m_nPortState_33 =  (uint32_t)  ((pFromData)[17] >> 2) & 0x03;

  /* Unpack Member: m_nPortState_34 */
  (pToStruct)->m_nPortState_34 =  (uint32_t)  ((pFromData)[17] >> 4) & 0x03;

  /* Unpack Member: m_nPortState_35 */
  (pToStruct)->m_nPortState_35 =  (uint32_t)  ((pFromData)[17] >> 6) & 0x03;

  /* Unpack Member: m_nPortState_36 */
  (pToStruct)->m_nPortState_36 =  (uint32_t)  ((pFromData)[16] ) & 0x03;

  /* Unpack Member: m_nPortState_37 */
  (pToStruct)->m_nPortState_37 =  (uint32_t)  ((pFromData)[16] >> 2) & 0x03;

  /* Unpack Member: m_nPortState_38 */
  (pToStruct)->m_nPortState_38 =  (uint32_t)  ((pFromData)[16] >> 4) & 0x03;

  /* Unpack Member: m_nPortState_39 */
  (pToStruct)->m_nPortState_39 =  (uint32_t)  ((pFromData)[16] >> 6) & 0x03;

  /* Unpack Member: m_nPortState_40 */
  (pToStruct)->m_nPortState_40 =  (uint32_t)  ((pFromData)[23] ) & 0x03;

  /* Unpack Member: m_nPortState_41 */
  (pToStruct)->m_nPortState_41 =  (uint32_t)  ((pFromData)[23] >> 2) & 0x03;

  /* Unpack Member: m_nPortState_42 */
  (pToStruct)->m_nPortState_42 =  (uint32_t)  ((pFromData)[23] >> 4) & 0x03;

  /* Unpack Member: m_nPortState_43 */
  (pToStruct)->m_nPortState_43 =  (uint32_t)  ((pFromData)[23] >> 6) & 0x03;

  /* Unpack Member: m_nPortState_44 */
  (pToStruct)->m_nPortState_44 =  (uint32_t)  ((pFromData)[22] ) & 0x03;

  /* Unpack Member: m_nPortState_45 */
  (pToStruct)->m_nPortState_45 =  (uint32_t)  ((pFromData)[22] >> 2) & 0x03;

  /* Unpack Member: m_nPortState_46 */
  (pToStruct)->m_nPortState_46 =  (uint32_t)  ((pFromData)[22] >> 4) & 0x03;

  /* Unpack Member: m_nPortState_47 */
  (pToStruct)->m_nPortState_47 =  (uint32_t)  ((pFromData)[22] >> 6) & 0x03;

  /* Unpack Member: m_nPortState_48 */
  (pToStruct)->m_nPortState_48 =  (uint32_t)  ((pFromData)[29] ) & 0x03;

  /* Unpack Member: m_nPortState_49 */
  (pToStruct)->m_nPortState_49 =  (uint32_t)  ((pFromData)[29] >> 2) & 0x03;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nVid0 */
  (pToStruct)->m_nVid0 =  (uint32_t)  ((pFromData)[30] >> 4) & 0x0f;
  (pToStruct)->m_nVid0 |=  (uint32_t)  (pFromData)[31] << 4;

  /* Unpack Member: m_nVid1 */
  (pToStruct)->m_nVid1 =  (uint32_t)  ((pFromData)[22] >> 4) & 0x0f;
  (pToStruct)->m_nVid1 |=  (uint32_t)  (pFromData)[23] << 4;

  /* Unpack Member: m_nOffset */
  (pToStruct)->m_nOffset =  (uint32_t)  ((pFromData)[22] ) & 0x0f;

  /* Unpack Member: m_nVlanPacketCounter */
  (pToStruct)->m_nVlanPacketCounter =  (uint32_t)  ((pFromData)[12] >> 3) & 0x1f;
  (pToStruct)->m_nVlanPacketCounter |=  (uint32_t)  (pFromData)[13] << 5;
  (pToStruct)->m_nVlanPacketCounter |=  (uint32_t)  (pFromData)[14] << 13;
  (pToStruct)->m_nVlanPacketCounter |=  (uint32_t)  (pFromData)[15] << 21;

  /* Unpack Member: m_nnVlanByteCounter */
  COMPILER_64_SET((pToStruct)->m_nnVlanByteCounter, 0,  (unsigned int) (pFromData)[8]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnVlanByteCounter;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[9]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnVlanByteCounter;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[10]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnVlanByteCounter;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[11]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnVlanByteCounter;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[12]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nDropPacketCounter */
  (pToStruct)->m_nDropPacketCounter =  (uint32_t)  ((pFromData)[4] >> 3) & 0x1f;
  (pToStruct)->m_nDropPacketCounter |=  (uint32_t)  (pFromData)[5] << 5;
  (pToStruct)->m_nDropPacketCounter |=  (uint32_t)  (pFromData)[6] << 13;
  (pToStruct)->m_nDropPacketCounter |=  (uint32_t)  (pFromData)[7] << 21;

  /* Unpack Member: m_nnDropByteCounter */
  COMPILER_64_SET((pToStruct)->m_nnDropByteCounter, 0,  (unsigned int) (pFromData)[0]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnDropByteCounter;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[1]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnDropByteCounter;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[2]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnDropByteCounter;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[3]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnDropByteCounter;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[4]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nPortState_0 */
  (pToStruct)->m_nPortState_0 =  (uint32_t)  ((pFromData)[24] ) & 0x03;

  /* Unpack Member: m_nPortState_1 */
  (pToStruct)->m_nPortState_1 =  (uint32_t)  ((pFromData)[24] >> 2) & 0x03;

  /* Unpack Member: m_nPortState_2 */
  (pToStruct)->m_nPortState_2 =  (uint32_t)  ((pFromData)[24] >> 4) & 0x03;

  /* Unpack Member: m_nPortState_3 */
  (pToStruct)->m_nPortState_3 =  (uint32_t)  ((pFromData)[24] >> 6) & 0x03;

  /* Unpack Member: m_nPortState_4 */
  (pToStruct)->m_nPortState_4 =  (uint32_t)  ((pFromData)[25] ) & 0x03;

  /* Unpack Member: m_nPortState_5 */
  (pToStruct)->m_nPortState_5 =  (uint32_t)  ((pFromData)[25] >> 2) & 0x03;

  /* Unpack Member: m_nPortState_6 */
  (pToStruct)->m_nPortState_6 =  (uint32_t)  ((pFromData)[25] >> 4) & 0x03;

  /* Unpack Member: m_nPortState_7 */
  (pToStruct)->m_nPortState_7 =  (uint32_t)  ((pFromData)[25] >> 6) & 0x03;

  /* Unpack Member: m_nPortState_8 */
  (pToStruct)->m_nPortState_8 =  (uint32_t)  ((pFromData)[26] ) & 0x03;

  /* Unpack Member: m_nPortState_9 */
  (pToStruct)->m_nPortState_9 =  (uint32_t)  ((pFromData)[26] >> 2) & 0x03;

  /* Unpack Member: m_nPortState_10 */
  (pToStruct)->m_nPortState_10 =  (uint32_t)  ((pFromData)[26] >> 4) & 0x03;

  /* Unpack Member: m_nPortState_11 */
  (pToStruct)->m_nPortState_11 =  (uint32_t)  ((pFromData)[26] >> 6) & 0x03;

  /* Unpack Member: m_nPortState_12 */
  (pToStruct)->m_nPortState_12 =  (uint32_t)  ((pFromData)[27] ) & 0x03;

  /* Unpack Member: m_nPortState_13 */
  (pToStruct)->m_nPortState_13 =  (uint32_t)  ((pFromData)[27] >> 2) & 0x03;

  /* Unpack Member: m_nPortState_14 */
  (pToStruct)->m_nPortState_14 =  (uint32_t)  ((pFromData)[27] >> 4) & 0x03;

  /* Unpack Member: m_nPortState_15 */
  (pToStruct)->m_nPortState_15 =  (uint32_t)  ((pFromData)[27] >> 6) & 0x03;

  /* Unpack Member: m_nPortState_16 */
  (pToStruct)->m_nPortState_16 =  (uint32_t)  ((pFromData)[28] ) & 0x03;

  /* Unpack Member: m_nPortState_17 */
  (pToStruct)->m_nPortState_17 =  (uint32_t)  ((pFromData)[28] >> 2) & 0x03;

  /* Unpack Member: m_nPortState_18 */
  (pToStruct)->m_nPortState_18 =  (uint32_t)  ((pFromData)[28] >> 4) & 0x03;

  /* Unpack Member: m_nPortState_19 */
  (pToStruct)->m_nPortState_19 =  (uint32_t)  ((pFromData)[28] >> 6) & 0x03;

  /* Unpack Member: m_nPortState_20 */
  (pToStruct)->m_nPortState_20 =  (uint32_t)  ((pFromData)[29] ) & 0x03;

  /* Unpack Member: m_nPortState_21 */
  (pToStruct)->m_nPortState_21 =  (uint32_t)  ((pFromData)[29] >> 2) & 0x03;

  /* Unpack Member: m_nPortState_22 */
  (pToStruct)->m_nPortState_22 =  (uint32_t)  ((pFromData)[29] >> 4) & 0x03;

  /* Unpack Member: m_nPortState_23 */
  (pToStruct)->m_nPortState_23 =  (uint32_t)  ((pFromData)[29] >> 6) & 0x03;

  /* Unpack Member: m_nPortState_24 */
  (pToStruct)->m_nPortState_24 =  (uint32_t)  ((pFromData)[16] ) & 0x03;

  /* Unpack Member: m_nPortState_25 */
  (pToStruct)->m_nPortState_25 =  (uint32_t)  ((pFromData)[16] >> 2) & 0x03;

  /* Unpack Member: m_nPortState_26 */
  (pToStruct)->m_nPortState_26 =  (uint32_t)  ((pFromData)[16] >> 4) & 0x03;

  /* Unpack Member: m_nPortState_27 */
  (pToStruct)->m_nPortState_27 =  (uint32_t)  ((pFromData)[16] >> 6) & 0x03;

  /* Unpack Member: m_nPortState_28 */
  (pToStruct)->m_nPortState_28 =  (uint32_t)  ((pFromData)[17] ) & 0x03;

  /* Unpack Member: m_nPortState_29 */
  (pToStruct)->m_nPortState_29 =  (uint32_t)  ((pFromData)[17] >> 2) & 0x03;

  /* Unpack Member: m_nPortState_30 */
  (pToStruct)->m_nPortState_30 =  (uint32_t)  ((pFromData)[17] >> 4) & 0x03;

  /* Unpack Member: m_nPortState_31 */
  (pToStruct)->m_nPortState_31 =  (uint32_t)  ((pFromData)[17] >> 6) & 0x03;

  /* Unpack Member: m_nPortState_32 */
  (pToStruct)->m_nPortState_32 =  (uint32_t)  ((pFromData)[18] ) & 0x03;

  /* Unpack Member: m_nPortState_33 */
  (pToStruct)->m_nPortState_33 =  (uint32_t)  ((pFromData)[18] >> 2) & 0x03;

  /* Unpack Member: m_nPortState_34 */
  (pToStruct)->m_nPortState_34 =  (uint32_t)  ((pFromData)[18] >> 4) & 0x03;

  /* Unpack Member: m_nPortState_35 */
  (pToStruct)->m_nPortState_35 =  (uint32_t)  ((pFromData)[18] >> 6) & 0x03;

  /* Unpack Member: m_nPortState_36 */
  (pToStruct)->m_nPortState_36 =  (uint32_t)  ((pFromData)[19] ) & 0x03;

  /* Unpack Member: m_nPortState_37 */
  (pToStruct)->m_nPortState_37 =  (uint32_t)  ((pFromData)[19] >> 2) & 0x03;

  /* Unpack Member: m_nPortState_38 */
  (pToStruct)->m_nPortState_38 =  (uint32_t)  ((pFromData)[19] >> 4) & 0x03;

  /* Unpack Member: m_nPortState_39 */
  (pToStruct)->m_nPortState_39 =  (uint32_t)  ((pFromData)[19] >> 6) & 0x03;

  /* Unpack Member: m_nPortState_40 */
  (pToStruct)->m_nPortState_40 =  (uint32_t)  ((pFromData)[20] ) & 0x03;

  /* Unpack Member: m_nPortState_41 */
  (pToStruct)->m_nPortState_41 =  (uint32_t)  ((pFromData)[20] >> 2) & 0x03;

  /* Unpack Member: m_nPortState_42 */
  (pToStruct)->m_nPortState_42 =  (uint32_t)  ((pFromData)[20] >> 4) & 0x03;

  /* Unpack Member: m_nPortState_43 */
  (pToStruct)->m_nPortState_43 =  (uint32_t)  ((pFromData)[20] >> 6) & 0x03;

  /* Unpack Member: m_nPortState_44 */
  (pToStruct)->m_nPortState_44 =  (uint32_t)  ((pFromData)[21] ) & 0x03;

  /* Unpack Member: m_nPortState_45 */
  (pToStruct)->m_nPortState_45 =  (uint32_t)  ((pFromData)[21] >> 2) & 0x03;

  /* Unpack Member: m_nPortState_46 */
  (pToStruct)->m_nPortState_46 =  (uint32_t)  ((pFromData)[21] >> 4) & 0x03;

  /* Unpack Member: m_nPortState_47 */
  (pToStruct)->m_nPortState_47 =  (uint32_t)  ((pFromData)[21] >> 6) & 0x03;

  /* Unpack Member: m_nPortState_48 */
  (pToStruct)->m_nPortState_48 =  (uint32_t)  ((pFromData)[30] ) & 0x03;

  /* Unpack Member: m_nPortState_49 */
  (pToStruct)->m_nPortState_49 =  (uint32_t)  ((pFromData)[30] >> 2) & 0x03;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaEpSlimVlanRecord_InitInstance(sbZfKaEpSlimVlanRecord_t *pFrame) {

  pFrame->m_nVid0 =  (unsigned int)  0;
  pFrame->m_nVid1 =  (unsigned int)  0;
  pFrame->m_nOffset =  (unsigned int)  0;
  pFrame->m_nVlanPacketCounter =  (unsigned int)  0;
  pFrame->m_nnVlanByteCounter =  (uint64_t)  0;
  pFrame->m_nDropPacketCounter =  (unsigned int)  0;
  pFrame->m_nnDropByteCounter =  (uint64_t)  0;
  pFrame->m_nPortState_0 =  (unsigned int)  0;
  pFrame->m_nPortState_1 =  (unsigned int)  0;
  pFrame->m_nPortState_2 =  (unsigned int)  0;
  pFrame->m_nPortState_3 =  (unsigned int)  0;
  pFrame->m_nPortState_4 =  (unsigned int)  0;
  pFrame->m_nPortState_5 =  (unsigned int)  0;
  pFrame->m_nPortState_6 =  (unsigned int)  0;
  pFrame->m_nPortState_7 =  (unsigned int)  0;
  pFrame->m_nPortState_8 =  (unsigned int)  0;
  pFrame->m_nPortState_9 =  (unsigned int)  0;
  pFrame->m_nPortState_10 =  (unsigned int)  0;
  pFrame->m_nPortState_11 =  (unsigned int)  0;
  pFrame->m_nPortState_12 =  (unsigned int)  0;
  pFrame->m_nPortState_13 =  (unsigned int)  0;
  pFrame->m_nPortState_14 =  (unsigned int)  0;
  pFrame->m_nPortState_15 =  (unsigned int)  0;
  pFrame->m_nPortState_16 =  (unsigned int)  0;
  pFrame->m_nPortState_17 =  (unsigned int)  0;
  pFrame->m_nPortState_18 =  (unsigned int)  0;
  pFrame->m_nPortState_19 =  (unsigned int)  0;
  pFrame->m_nPortState_20 =  (unsigned int)  0;
  pFrame->m_nPortState_21 =  (unsigned int)  0;
  pFrame->m_nPortState_22 =  (unsigned int)  0;
  pFrame->m_nPortState_23 =  (unsigned int)  0;
  pFrame->m_nPortState_24 =  (unsigned int)  0;
  pFrame->m_nPortState_25 =  (unsigned int)  0;
  pFrame->m_nPortState_26 =  (unsigned int)  0;
  pFrame->m_nPortState_27 =  (unsigned int)  0;
  pFrame->m_nPortState_28 =  (unsigned int)  0;
  pFrame->m_nPortState_29 =  (unsigned int)  0;
  pFrame->m_nPortState_30 =  (unsigned int)  0;
  pFrame->m_nPortState_31 =  (unsigned int)  0;
  pFrame->m_nPortState_32 =  (unsigned int)  0;
  pFrame->m_nPortState_33 =  (unsigned int)  0;
  pFrame->m_nPortState_34 =  (unsigned int)  0;
  pFrame->m_nPortState_35 =  (unsigned int)  0;
  pFrame->m_nPortState_36 =  (unsigned int)  0;
  pFrame->m_nPortState_37 =  (unsigned int)  0;
  pFrame->m_nPortState_38 =  (unsigned int)  0;
  pFrame->m_nPortState_39 =  (unsigned int)  0;
  pFrame->m_nPortState_40 =  (unsigned int)  0;
  pFrame->m_nPortState_41 =  (unsigned int)  0;
  pFrame->m_nPortState_42 =  (unsigned int)  0;
  pFrame->m_nPortState_43 =  (unsigned int)  0;
  pFrame->m_nPortState_44 =  (unsigned int)  0;
  pFrame->m_nPortState_45 =  (unsigned int)  0;
  pFrame->m_nPortState_46 =  (unsigned int)  0;
  pFrame->m_nPortState_47 =  (unsigned int)  0;
  pFrame->m_nPortState_48 =  (unsigned int)  0;
  pFrame->m_nPortState_49 =  (unsigned int)  0;

}
