/*
 * $Id: sbZfKaPmLastLine.c 1.1.44.4 Broadcom SDK $
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
#include "sbZfKaPmLastLine.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfKaPmLastLine_Pack(sbZfKaPmLastLine_t *pFrom,
                      uint8_t *pToData,
                      uint32_t nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAPMLASTLINE_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nReserved */
  (pToData)[11] |= ((pFrom)->m_nReserved & 0x0f) <<4;
  (pToData)[10] |= ((pFrom)->m_nReserved >> 4) &0xFF;
  (pToData)[9] |= ((pFrom)->m_nReserved >> 12) &0xFF;
  (pToData)[8] |= ((pFrom)->m_nReserved >> 20) &0xFF;
  (pToData)[15] |= ((pFrom)->m_nReserved >> 28) &0xFF;
  (pToData)[14] |= ((pFrom)->m_nReserved >> 36) &0xFF;
  (pToData)[13] |= ((pFrom)->m_nReserved >> 44) &0xFF;
  (pToData)[12] |= ((pFrom)->m_nReserved >> 52) &0xFF;

  /* Pack Member: m_nHec */
  (pToData)[4] |= ((pFrom)->m_nHec & 0x0f) <<4;
  (pToData)[11] |= ((pFrom)->m_nHec >> 4) & 0x0f;

  /* Pack Member: m_nZero */
  (pToData)[5] |= ((pFrom)->m_nZero & 0x7f) <<1;
  (pToData)[4] |= ((pFrom)->m_nZero >> 7) & 0x0f;

  /* Pack Member: m_nNextBuffer */
  (pToData)[7] |= ((pFrom)->m_nNextBuffer) & 0xFF;
  (pToData)[6] |= ((pFrom)->m_nNextBuffer >> 8) &0xFF;
  (pToData)[5] |= ((pFrom)->m_nNextBuffer >> 16) & 0x01;

  /* Pack Member: m_nTimestamp */
  (pToData)[3] |= ((pFrom)->m_nTimestamp) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nTimestamp >> 8) &0xFF;
  (pToData)[1] |= ((pFrom)->m_nTimestamp >> 16) &0xFF;
  (pToData)[0] |= ((pFrom)->m_nTimestamp >> 24) &0xFF;
#else
  int i;
  int size = SB_ZF_ZFKAPMLASTLINE_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nReserved */
  (pToData)[8] |= ((pFrom)->m_nReserved & 0x0f) <<4;
  (pToData)[9] |= ((pFrom)->m_nReserved >> 4) &0xFF;
  (pToData)[10] |= ((pFrom)->m_nReserved >> 12) &0xFF;
  (pToData)[11] |= ((pFrom)->m_nReserved >> 20) &0xFF;
  (pToData)[12] |= ((pFrom)->m_nReserved >> 28) &0xFF;
  (pToData)[13] |= ((pFrom)->m_nReserved >> 36) &0xFF;
  (pToData)[14] |= ((pFrom)->m_nReserved >> 44) &0xFF;
  (pToData)[15] |= ((pFrom)->m_nReserved >> 52) &0xFF;

  /* Pack Member: m_nHec */
  (pToData)[7] |= ((pFrom)->m_nHec & 0x0f) <<4;
  (pToData)[8] |= ((pFrom)->m_nHec >> 4) & 0x0f;

  /* Pack Member: m_nZero */
  (pToData)[6] |= ((pFrom)->m_nZero & 0x7f) <<1;
  (pToData)[7] |= ((pFrom)->m_nZero >> 7) & 0x0f;

  /* Pack Member: m_nNextBuffer */
  (pToData)[4] |= ((pFrom)->m_nNextBuffer) & 0xFF;
  (pToData)[5] |= ((pFrom)->m_nNextBuffer >> 8) &0xFF;
  (pToData)[6] |= ((pFrom)->m_nNextBuffer >> 16) & 0x01;

  /* Pack Member: m_nTimestamp */
  (pToData)[0] |= ((pFrom)->m_nTimestamp) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nTimestamp >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nTimestamp >> 16) &0xFF;
  (pToData)[3] |= ((pFrom)->m_nTimestamp >> 24) &0xFF;
#endif

  return SB_ZF_ZFKAPMLASTLINE_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaPmLastLine_Unpack(sbZfKaPmLastLine_t *pToStruct,
                        uint8_t *pFromData,
                        uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nReserved */
  COMPILER_64_SET((pToStruct)->m_nReserved, 0,  (unsigned int) (pFromData)[11]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nReserved;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[10]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nReserved;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[9]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nReserved;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[8]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nReserved;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[15]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nReserved;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[14]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nReserved;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[13]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nReserved;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[12]);
    COMPILER_64_SHL(tmp, 56);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nHec */
  (pToStruct)->m_nHec =  (uint32_t)  ((pFromData)[4] >> 4) & 0x0f;
  (pToStruct)->m_nHec |=  (uint32_t)  ((pFromData)[11] & 0x0f) << 4;

  /* Unpack Member: m_nZero */
  (pToStruct)->m_nZero =  (uint32_t)  ((pFromData)[5] >> 1) & 0x7f;
  (pToStruct)->m_nZero |=  (uint32_t)  ((pFromData)[4] & 0x0f) << 7;

  /* Unpack Member: m_nNextBuffer */
  (pToStruct)->m_nNextBuffer =  (uint32_t)  (pFromData)[7] ;
  (pToStruct)->m_nNextBuffer |=  (uint32_t)  (pFromData)[6] << 8;
  (pToStruct)->m_nNextBuffer |=  (uint32_t)  ((pFromData)[5] & 0x01) << 16;

  /* Unpack Member: m_nTimestamp */
  (pToStruct)->m_nTimestamp =  (uint32_t)  (pFromData)[3] ;
  (pToStruct)->m_nTimestamp |=  (uint32_t)  (pFromData)[2] << 8;
  (pToStruct)->m_nTimestamp |=  (uint32_t)  (pFromData)[1] << 16;
  (pToStruct)->m_nTimestamp |=  (uint32_t)  (pFromData)[0] << 24;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nReserved */
  COMPILER_64_SET((pToStruct)->m_nReserved, 0,  (unsigned int) (pFromData)[8]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nReserved;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[9]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nReserved;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[10]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nReserved;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[11]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nReserved;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[12]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nReserved;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[13]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nReserved;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[14]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nReserved;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[15]);
    COMPILER_64_SHL(tmp, 56);
    COMPILER_64_OR(*tmp0, tmp);
  };

  /* Unpack Member: m_nHec */
  (pToStruct)->m_nHec =  (uint32_t)  ((pFromData)[7] >> 4) & 0x0f;
  (pToStruct)->m_nHec |=  (uint32_t)  ((pFromData)[8] & 0x0f) << 4;

  /* Unpack Member: m_nZero */
  (pToStruct)->m_nZero =  (uint32_t)  ((pFromData)[6] >> 1) & 0x7f;
  (pToStruct)->m_nZero |=  (uint32_t)  ((pFromData)[7] & 0x0f) << 7;

  /* Unpack Member: m_nNextBuffer */
  (pToStruct)->m_nNextBuffer =  (uint32_t)  (pFromData)[4] ;
  (pToStruct)->m_nNextBuffer |=  (uint32_t)  (pFromData)[5] << 8;
  (pToStruct)->m_nNextBuffer |=  (uint32_t)  ((pFromData)[6] & 0x01) << 16;

  /* Unpack Member: m_nTimestamp */
  (pToStruct)->m_nTimestamp =  (uint32_t)  (pFromData)[0] ;
  (pToStruct)->m_nTimestamp |=  (uint32_t)  (pFromData)[1] << 8;
  (pToStruct)->m_nTimestamp |=  (uint32_t)  (pFromData)[2] << 16;
  (pToStruct)->m_nTimestamp |=  (uint32_t)  (pFromData)[3] << 24;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaPmLastLine_InitInstance(sbZfKaPmLastLine_t *pFrame) {

  pFrame->m_nReserved =  (uint64_t)  0;
  pFrame->m_nHec =  (unsigned int)  0;
  pFrame->m_nZero =  (unsigned int)  0;
  pFrame->m_nNextBuffer =  (unsigned int)  0;
  pFrame->m_nTimestamp =  (unsigned int)  0;

}
