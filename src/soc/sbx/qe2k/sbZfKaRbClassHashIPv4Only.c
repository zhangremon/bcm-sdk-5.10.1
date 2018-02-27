/*
 * $Id: sbZfKaRbClassHashIPv4Only.c 1.1.44.4 Broadcom SDK $
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
#include "sbZfKaRbClassHashIPv4Only.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfKaRbClassHashIPv4Only_Pack(sbZfKaRbClassHashIPv4Only_t *pFrom,
                               uint8_t *pToData,
                               uint32_t nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKARBCLASSHASHIPV4ONLY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nProtocol */
  (pToData)[44] |= ((pFrom)->m_nProtocol) & 0xFF;

  /* Pack Member: m_nPadWord1 */
  (pToData)[45] |= ((pFrom)->m_nPadWord1) & 0xFF;

  /* Pack Member: m_nPadWord0 */
  (pToData)[37] |= ((pFrom)->m_nPadWord0) & 0xFF;
  (pToData)[36] |= ((pFrom)->m_nPadWord0 >> 8) &0xFF;
  (pToData)[43] |= ((pFrom)->m_nPadWord0 >> 16) &0xFF;
  (pToData)[42] |= ((pFrom)->m_nPadWord0 >> 24) &0xFF;
  (pToData)[41] |= ((pFrom)->m_nPadWord0 >> 32) &0xFF;
  (pToData)[40] |= ((pFrom)->m_nPadWord0 >> 40) &0xFF;
  (pToData)[47] |= ((pFrom)->m_nPadWord0 >> 48) &0xFF;
  (pToData)[46] |= ((pFrom)->m_nPadWord0 >> 56) &0xFF;

  /* Pack Member: m_nIpSa */
  (pToData)[33] |= ((pFrom)->m_nIpSa) & 0xFF;
  (pToData)[32] |= ((pFrom)->m_nIpSa >> 8) &0xFF;
  (pToData)[39] |= ((pFrom)->m_nIpSa >> 16) &0xFF;
  (pToData)[38] |= ((pFrom)->m_nIpSa >> 24) &0xFF;

  /* Pack Member: m_nIpDa */
  (pToData)[29] |= ((pFrom)->m_nIpDa) & 0xFF;
  (pToData)[28] |= ((pFrom)->m_nIpDa >> 8) &0xFF;
  (pToData)[35] |= ((pFrom)->m_nIpDa >> 16) &0xFF;
  (pToData)[34] |= ((pFrom)->m_nIpDa >> 24) &0xFF;

  /* Pack Member: m_nSocket */
  (pToData)[25] |= ((pFrom)->m_nSocket) & 0xFF;
  (pToData)[24] |= ((pFrom)->m_nSocket >> 8) &0xFF;
  (pToData)[31] |= ((pFrom)->m_nSocket >> 16) &0xFF;
  (pToData)[30] |= ((pFrom)->m_nSocket >> 24) &0xFF;

  /* Pack Member: m_nPadWord2 */
  (pToData)[21] |= ((pFrom)->m_nPadWord2 & 0x07) <<5;
  (pToData)[20] |= ((pFrom)->m_nPadWord2 >> 3) &0xFF;
  (pToData)[27] |= ((pFrom)->m_nPadWord2 >> 11) &0xFF;
  (pToData)[26] |= ((pFrom)->m_nPadWord2 >> 19) &0xFF;

  /* Pack Member: m_nPadWord3 */
  (pToData)[19] |= ((pFrom)->m_nPadWord3) & 0xFF;
  (pToData)[18] |= ((pFrom)->m_nPadWord3 >> 8) &0xFF;
  (pToData)[17] |= ((pFrom)->m_nPadWord3 >> 16) &0xFF;
  (pToData)[16] |= ((pFrom)->m_nPadWord3 >> 24) &0xFF;
  (pToData)[23] |= ((pFrom)->m_nPadWord3 >> 32) &0xFF;
  (pToData)[22] |= ((pFrom)->m_nPadWord3 >> 40) &0xFF;
  (pToData)[21] |= ((pFrom)->m_nPadWord3 >> 48) & 0x1f;

  /* Pack Member: m_nSpareWord1 */
  (pToData)[11] |= ((pFrom)->m_nSpareWord1) & 0xFF;
  (pToData)[10] |= ((pFrom)->m_nSpareWord1 >> 8) &0xFF;
  (pToData)[9] |= ((pFrom)->m_nSpareWord1 >> 16) &0xFF;
  (pToData)[8] |= ((pFrom)->m_nSpareWord1 >> 24) &0xFF;
  (pToData)[15] |= ((pFrom)->m_nSpareWord1 >> 32) &0xFF;
  (pToData)[14] |= ((pFrom)->m_nSpareWord1 >> 40) &0xFF;
  (pToData)[13] |= ((pFrom)->m_nSpareWord1 >> 48) &0xFF;
  (pToData)[12] |= ((pFrom)->m_nSpareWord1 >> 56) &0xFF;

  /* Pack Member: m_nSpareWord0 */
  (pToData)[3] |= ((pFrom)->m_nSpareWord0) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nSpareWord0 >> 8) &0xFF;
  (pToData)[1] |= ((pFrom)->m_nSpareWord0 >> 16) &0xFF;
  (pToData)[0] |= ((pFrom)->m_nSpareWord0 >> 24) &0xFF;
  (pToData)[7] |= ((pFrom)->m_nSpareWord0 >> 32) &0xFF;
  (pToData)[6] |= ((pFrom)->m_nSpareWord0 >> 40) &0xFF;
  (pToData)[5] |= ((pFrom)->m_nSpareWord0 >> 48) &0xFF;
  (pToData)[4] |= ((pFrom)->m_nSpareWord0 >> 56) &0xFF;
#else
  int i;
  int size = SB_ZF_ZFKARBCLASSHASHIPV4ONLY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nProtocol */
  (pToData)[47] |= ((pFrom)->m_nProtocol) & 0xFF;

  /* Pack Member: m_nPadWord1 */
  (pToData)[46] |= ((pFrom)->m_nPadWord1) & 0xFF;

  /* Pack Member: m_nPadWord0 */
  (pToData)[38] |= ((pFrom)->m_nPadWord0) & 0xFF;
  (pToData)[39] |= ((pFrom)->m_nPadWord0 >> 8) &0xFF;
  (pToData)[40] |= ((pFrom)->m_nPadWord0 >> 16) &0xFF;
  (pToData)[41] |= ((pFrom)->m_nPadWord0 >> 24) &0xFF;
  (pToData)[42] |= ((pFrom)->m_nPadWord0 >> 32) &0xFF;
  (pToData)[43] |= ((pFrom)->m_nPadWord0 >> 40) &0xFF;
  (pToData)[44] |= ((pFrom)->m_nPadWord0 >> 48) &0xFF;
  (pToData)[45] |= ((pFrom)->m_nPadWord0 >> 56) &0xFF;

  /* Pack Member: m_nIpSa */
  (pToData)[34] |= ((pFrom)->m_nIpSa) & 0xFF;
  (pToData)[35] |= ((pFrom)->m_nIpSa >> 8) &0xFF;
  (pToData)[36] |= ((pFrom)->m_nIpSa >> 16) &0xFF;
  (pToData)[37] |= ((pFrom)->m_nIpSa >> 24) &0xFF;

  /* Pack Member: m_nIpDa */
  (pToData)[30] |= ((pFrom)->m_nIpDa) & 0xFF;
  (pToData)[31] |= ((pFrom)->m_nIpDa >> 8) &0xFF;
  (pToData)[32] |= ((pFrom)->m_nIpDa >> 16) &0xFF;
  (pToData)[33] |= ((pFrom)->m_nIpDa >> 24) &0xFF;

  /* Pack Member: m_nSocket */
  (pToData)[26] |= ((pFrom)->m_nSocket) & 0xFF;
  (pToData)[27] |= ((pFrom)->m_nSocket >> 8) &0xFF;
  (pToData)[28] |= ((pFrom)->m_nSocket >> 16) &0xFF;
  (pToData)[29] |= ((pFrom)->m_nSocket >> 24) &0xFF;

  /* Pack Member: m_nPadWord2 */
  (pToData)[22] |= ((pFrom)->m_nPadWord2 & 0x07) <<5;
  (pToData)[23] |= ((pFrom)->m_nPadWord2 >> 3) &0xFF;
  (pToData)[24] |= ((pFrom)->m_nPadWord2 >> 11) &0xFF;
  (pToData)[25] |= ((pFrom)->m_nPadWord2 >> 19) &0xFF;

  /* Pack Member: m_nPadWord3 */
  (pToData)[16] |= ((pFrom)->m_nPadWord3) & 0xFF;
  (pToData)[17] |= ((pFrom)->m_nPadWord3 >> 8) &0xFF;
  (pToData)[18] |= ((pFrom)->m_nPadWord3 >> 16) &0xFF;
  (pToData)[19] |= ((pFrom)->m_nPadWord3 >> 24) &0xFF;
  (pToData)[20] |= ((pFrom)->m_nPadWord3 >> 32) &0xFF;
  (pToData)[21] |= ((pFrom)->m_nPadWord3 >> 40) &0xFF;
  (pToData)[22] |= ((pFrom)->m_nPadWord3 >> 48) & 0x1f;

  /* Pack Member: m_nSpareWord1 */
  (pToData)[8] |= ((pFrom)->m_nSpareWord1) & 0xFF;
  (pToData)[9] |= ((pFrom)->m_nSpareWord1 >> 8) &0xFF;
  (pToData)[10] |= ((pFrom)->m_nSpareWord1 >> 16) &0xFF;
  (pToData)[11] |= ((pFrom)->m_nSpareWord1 >> 24) &0xFF;
  (pToData)[12] |= ((pFrom)->m_nSpareWord1 >> 32) &0xFF;
  (pToData)[13] |= ((pFrom)->m_nSpareWord1 >> 40) &0xFF;
  (pToData)[14] |= ((pFrom)->m_nSpareWord1 >> 48) &0xFF;
  (pToData)[15] |= ((pFrom)->m_nSpareWord1 >> 56) &0xFF;

  /* Pack Member: m_nSpareWord0 */
  (pToData)[0] |= ((pFrom)->m_nSpareWord0) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nSpareWord0 >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nSpareWord0 >> 16) &0xFF;
  (pToData)[3] |= ((pFrom)->m_nSpareWord0 >> 24) &0xFF;
  (pToData)[4] |= ((pFrom)->m_nSpareWord0 >> 32) &0xFF;
  (pToData)[5] |= ((pFrom)->m_nSpareWord0 >> 40) &0xFF;
  (pToData)[6] |= ((pFrom)->m_nSpareWord0 >> 48) &0xFF;
  (pToData)[7] |= ((pFrom)->m_nSpareWord0 >> 56) &0xFF;
#endif

  return SB_ZF_ZFKARBCLASSHASHIPV4ONLY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaRbClassHashIPv4Only_Unpack(sbZfKaRbClassHashIPv4Only_t *pToStruct,
                                 uint8_t *pFromData,
                                 uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nProtocol */
  (pToStruct)->m_nProtocol =  (uint32_t)  (pFromData)[44] ;

  /* Unpack Member: m_nPadWord1 */
  COMPILER_64_SET((pToStruct)->m_nPadWord1, 0,  (unsigned int) (pFromData)[45]);

  /* Unpack Member: m_nPadWord0 */
  COMPILER_64_SET((pToStruct)->m_nPadWord0, 0,  (unsigned int) (pFromData)[37]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[36]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[43]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[42]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[41]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[40]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[47]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[46]);
    COMPILER_64_SHL(tmp, 56);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nIpSa */
  (pToStruct)->m_nIpSa =  (uint32_t)  (pFromData)[33] ;
  (pToStruct)->m_nIpSa |=  (uint32_t)  (pFromData)[32] << 8;
  (pToStruct)->m_nIpSa |=  (uint32_t)  (pFromData)[39] << 16;
  (pToStruct)->m_nIpSa |=  (uint32_t)  (pFromData)[38] << 24;

  /* Unpack Member: m_nIpDa */
  (pToStruct)->m_nIpDa =  (uint32_t)  (pFromData)[29] ;
  (pToStruct)->m_nIpDa |=  (uint32_t)  (pFromData)[28] << 8;
  (pToStruct)->m_nIpDa |=  (uint32_t)  (pFromData)[35] << 16;
  (pToStruct)->m_nIpDa |=  (uint32_t)  (pFromData)[34] << 24;

  /* Unpack Member: m_nSocket */
  (pToStruct)->m_nSocket =  (uint32_t)  (pFromData)[25] ;
  (pToStruct)->m_nSocket |=  (uint32_t)  (pFromData)[24] << 8;
  (pToStruct)->m_nSocket |=  (uint32_t)  (pFromData)[31] << 16;
  (pToStruct)->m_nSocket |=  (uint32_t)  (pFromData)[30] << 24;

  /* Unpack Member: m_nPadWord2 */
  COMPILER_64_SET((pToStruct)->m_nPadWord2, 0,  (unsigned int) (pFromData)[21]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord2;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[20]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord2;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[27]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord2;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[26]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nPadWord3 */
  COMPILER_64_SET((pToStruct)->m_nPadWord3, 0,  (unsigned int) (pFromData)[19]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[18]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[17]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[16]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[23]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[22]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[21]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nSpareWord1 */
  COMPILER_64_SET((pToStruct)->m_nSpareWord1, 0,  (unsigned int) (pFromData)[11]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nSpareWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[10]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nSpareWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[9]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nSpareWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[8]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nSpareWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[15]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nSpareWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[14]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nSpareWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[13]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nSpareWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[12]);
    COMPILER_64_SHL(tmp, 56);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nSpareWord0 */
  COMPILER_64_SET((pToStruct)->m_nSpareWord0, 0,  (unsigned int) (pFromData)[3]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nSpareWord0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[2]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nSpareWord0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[1]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nSpareWord0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[0]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nSpareWord0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[7]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nSpareWord0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[6]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nSpareWord0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[5]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nSpareWord0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[4]);
    COMPILER_64_SHL(tmp, 56);
    COMPILER_64_OR(*tmp0, tmp);
  };
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nProtocol */
  (pToStruct)->m_nProtocol =  (uint32_t)  (pFromData)[47] ;

  /* Unpack Member: m_nPadWord1 */
  COMPILER_64_SET((pToStruct)->m_nPadWord1, 0,  (unsigned int) (pFromData)[46]);

  /* Unpack Member: m_nPadWord0 */
  COMPILER_64_SET((pToStruct)->m_nPadWord0, 0,  (unsigned int) (pFromData)[38]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[39]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[40]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[41]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[42]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[43]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[44]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[45]);
    COMPILER_64_SHL(tmp, 56);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nIpSa */
  (pToStruct)->m_nIpSa =  (uint32_t)  (pFromData)[34] ;
  (pToStruct)->m_nIpSa |=  (uint32_t)  (pFromData)[35] << 8;
  (pToStruct)->m_nIpSa |=  (uint32_t)  (pFromData)[36] << 16;
  (pToStruct)->m_nIpSa |=  (uint32_t)  (pFromData)[37] << 24;

  /* Unpack Member: m_nIpDa */
  (pToStruct)->m_nIpDa =  (uint32_t)  (pFromData)[30] ;
  (pToStruct)->m_nIpDa |=  (uint32_t)  (pFromData)[31] << 8;
  (pToStruct)->m_nIpDa |=  (uint32_t)  (pFromData)[32] << 16;
  (pToStruct)->m_nIpDa |=  (uint32_t)  (pFromData)[33] << 24;

  /* Unpack Member: m_nSocket */
  (pToStruct)->m_nSocket =  (uint32_t)  (pFromData)[26] ;
  (pToStruct)->m_nSocket |=  (uint32_t)  (pFromData)[27] << 8;
  (pToStruct)->m_nSocket |=  (uint32_t)  (pFromData)[28] << 16;
  (pToStruct)->m_nSocket |=  (uint32_t)  (pFromData)[29] << 24;

  /* Unpack Member: m_nPadWord2 */
  COMPILER_64_SET((pToStruct)->m_nPadWord2, 0,  (unsigned int) (pFromData)[22]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord2;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[23]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord2;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[24]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord2;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[25]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nPadWord3 */
  COMPILER_64_SET((pToStruct)->m_nPadWord3, 0,  (unsigned int) (pFromData)[16]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[17]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[18]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[19]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[20]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[21]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nPadWord3;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[22]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nSpareWord1 */
  COMPILER_64_SET((pToStruct)->m_nSpareWord1, 0,  (unsigned int) (pFromData)[8]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nSpareWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[9]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nSpareWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[10]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nSpareWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[11]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nSpareWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[12]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nSpareWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[13]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nSpareWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[14]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nSpareWord1;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[15]);
    COMPILER_64_SHL(tmp, 56);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nSpareWord0 */
  COMPILER_64_SET((pToStruct)->m_nSpareWord0, 0,  (unsigned int) (pFromData)[0]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nSpareWord0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[1]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nSpareWord0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[2]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nSpareWord0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[3]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nSpareWord0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[4]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nSpareWord0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[5]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nSpareWord0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[6]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nSpareWord0;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[7]);
    COMPILER_64_SHL(tmp, 56);
    COMPILER_64_OR(*tmp0, tmp);
  };
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaRbClassHashIPv4Only_InitInstance(sbZfKaRbClassHashIPv4Only_t *pFrame) {

  pFrame->m_nProtocol =  (unsigned int)  0;
  pFrame->m_nPadWord1 =  (unsigned int)  0;
  pFrame->m_nPadWord0 =  (uint64_t)  0;
  pFrame->m_nIpSa =  (unsigned int)  0;
  pFrame->m_nIpDa =  (unsigned int)  0;
  pFrame->m_nSocket =  (unsigned int)  0;
  pFrame->m_nPadWord2 =  (unsigned int)  0;
  pFrame->m_nPadWord3 =  (uint64_t)  0;
  pFrame->m_nSpareWord1 =  (uint64_t)  0;
  pFrame->m_nSpareWord0 =  (uint64_t)  0;

}
