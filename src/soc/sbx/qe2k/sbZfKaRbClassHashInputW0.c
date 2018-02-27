/*
 * $Id: sbZfKaRbClassHashInputW0.c 1.1.44.4 Broadcom SDK $
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
#include "sbZfKaRbClassHashInputW0.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfKaRbClassHashInputW0_Pack(sbZfKaRbClassHashInputW0_t *pFrom,
                              uint8_t *pToData,
                              uint32_t nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKARBCLASSHASHINPUTW0_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nnDmac */
  (pToData)[9] |= ((pFrom)->m_nnDmac) & 0xFF;
  (pToData)[8] |= ((pFrom)->m_nnDmac >> 8) &0xFF;
  (pToData)[15] |= ((pFrom)->m_nnDmac >> 16) &0xFF;
  (pToData)[14] |= ((pFrom)->m_nnDmac >> 24) &0xFF;
  (pToData)[13] |= ((pFrom)->m_nnDmac >> 32) &0xFF;
  (pToData)[12] |= ((pFrom)->m_nnDmac >> 40) &0xFF;

  /* Pack Member: m_nnSmac */
  (pToData)[7] |= ((pFrom)->m_nnSmac) & 0xFF;
  (pToData)[6] |= ((pFrom)->m_nnSmac >> 8) &0xFF;
  (pToData)[5] |= ((pFrom)->m_nnSmac >> 16) &0xFF;
  (pToData)[4] |= ((pFrom)->m_nnSmac >> 24) &0xFF;
  (pToData)[11] |= ((pFrom)->m_nnSmac >> 32) &0xFF;
  (pToData)[10] |= ((pFrom)->m_nnSmac >> 40) &0xFF;

  /* Pack Member: m_nSpare1 */
  (pToData)[1] |= ((pFrom)->m_nSpare1 & 0x3f) <<2;
  (pToData)[0] |= ((pFrom)->m_nSpare1 >> 6) &0xFF;

  /* Pack Member: m_nIPort */
  (pToData)[2] |= ((pFrom)->m_nIPort & 0x0f) <<4;
  (pToData)[1] |= ((pFrom)->m_nIPort >> 4) & 0x03;

  /* Pack Member: m_nVlanId */
  (pToData)[3] |= ((pFrom)->m_nVlanId) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nVlanId >> 8) & 0x0f;
#else
  int i;
  int size = SB_ZF_ZFKARBCLASSHASHINPUTW0_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nnDmac */
  (pToData)[10] |= ((pFrom)->m_nnDmac) & 0xFF;
  (pToData)[11] |= ((pFrom)->m_nnDmac >> 8) &0xFF;
  (pToData)[12] |= ((pFrom)->m_nnDmac >> 16) &0xFF;
  (pToData)[13] |= ((pFrom)->m_nnDmac >> 24) &0xFF;
  (pToData)[14] |= ((pFrom)->m_nnDmac >> 32) &0xFF;
  (pToData)[15] |= ((pFrom)->m_nnDmac >> 40) &0xFF;

  /* Pack Member: m_nnSmac */
  (pToData)[4] |= ((pFrom)->m_nnSmac) & 0xFF;
  (pToData)[5] |= ((pFrom)->m_nnSmac >> 8) &0xFF;
  (pToData)[6] |= ((pFrom)->m_nnSmac >> 16) &0xFF;
  (pToData)[7] |= ((pFrom)->m_nnSmac >> 24) &0xFF;
  (pToData)[8] |= ((pFrom)->m_nnSmac >> 32) &0xFF;
  (pToData)[9] |= ((pFrom)->m_nnSmac >> 40) &0xFF;

  /* Pack Member: m_nSpare1 */
  (pToData)[2] |= ((pFrom)->m_nSpare1 & 0x3f) <<2;
  (pToData)[3] |= ((pFrom)->m_nSpare1 >> 6) &0xFF;

  /* Pack Member: m_nIPort */
  (pToData)[1] |= ((pFrom)->m_nIPort & 0x0f) <<4;
  (pToData)[2] |= ((pFrom)->m_nIPort >> 4) & 0x03;

  /* Pack Member: m_nVlanId */
  (pToData)[0] |= ((pFrom)->m_nVlanId) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nVlanId >> 8) & 0x0f;
#endif

  return SB_ZF_ZFKARBCLASSHASHINPUTW0_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaRbClassHashInputW0_Unpack(sbZfKaRbClassHashInputW0_t *pToStruct,
                                uint8_t *pFromData,
                                uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nnDmac */
  COMPILER_64_SET((pToStruct)->m_nnDmac, 0,  (unsigned int) (pFromData)[9]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnDmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[8]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnDmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[15]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnDmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[14]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnDmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[13]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnDmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[12]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nnSmac */
  COMPILER_64_SET((pToStruct)->m_nnSmac, 0,  (unsigned int) (pFromData)[7]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnSmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[6]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnSmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[5]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnSmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[4]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnSmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[11]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnSmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[10]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nSpare1 */
  (pToStruct)->m_nSpare1 =  (uint32_t)  ((pFromData)[1] >> 2) & 0x3f;
  (pToStruct)->m_nSpare1 |=  (uint32_t)  (pFromData)[0] << 6;

  /* Unpack Member: m_nIPort */
  (pToStruct)->m_nIPort =  (uint32_t)  ((pFromData)[2] >> 4) & 0x0f;
  (pToStruct)->m_nIPort |=  (uint32_t)  ((pFromData)[1] & 0x03) << 4;

  /* Unpack Member: m_nVlanId */
  (pToStruct)->m_nVlanId =  (uint32_t)  (pFromData)[3] ;
  (pToStruct)->m_nVlanId |=  (uint32_t)  ((pFromData)[2] & 0x0f) << 8;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nnDmac */
  COMPILER_64_SET((pToStruct)->m_nnDmac, 0,  (unsigned int) (pFromData)[10]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnDmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[11]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnDmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[12]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnDmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[13]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnDmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[14]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnDmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[15]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nnSmac */
  COMPILER_64_SET((pToStruct)->m_nnSmac, 0,  (unsigned int) (pFromData)[4]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnSmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[5]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnSmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[6]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnSmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[7]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnSmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[8]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnSmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[9]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nSpare1 */
  (pToStruct)->m_nSpare1 =  (uint32_t)  ((pFromData)[2] >> 2) & 0x3f;
  (pToStruct)->m_nSpare1 |=  (uint32_t)  (pFromData)[3] << 6;

  /* Unpack Member: m_nIPort */
  (pToStruct)->m_nIPort =  (uint32_t)  ((pFromData)[1] >> 4) & 0x0f;
  (pToStruct)->m_nIPort |=  (uint32_t)  ((pFromData)[2] & 0x03) << 4;

  /* Unpack Member: m_nVlanId */
  (pToStruct)->m_nVlanId =  (uint32_t)  (pFromData)[0] ;
  (pToStruct)->m_nVlanId |=  (uint32_t)  ((pFromData)[1] & 0x0f) << 8;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaRbClassHashInputW0_InitInstance(sbZfKaRbClassHashInputW0_t *pFrame) {

  pFrame->m_nnDmac =  (uint64_t)  0;
  pFrame->m_nnSmac =  (uint64_t)  0;
  pFrame->m_nSpare1 =  (unsigned int)  0;
  pFrame->m_nIPort =  (unsigned int)  0;
  pFrame->m_nVlanId =  (unsigned int)  0;

}
