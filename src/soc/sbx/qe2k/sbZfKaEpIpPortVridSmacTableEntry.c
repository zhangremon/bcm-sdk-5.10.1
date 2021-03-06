/*
 * $Id: sbZfKaEpIpPortVridSmacTableEntry.c 1.1.44.4 Broadcom SDK $
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
#include "sbZfKaEpIpPortVridSmacTableEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfKaEpIpPortVridSmacTableEntry_Pack(sbZfKaEpIpPortVridSmacTableEntry_t *pFrom,
                                      uint8_t *pToData,
                                      uint32_t nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKAEPIP_PORT_VRID_SMAC_TABLEENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nReserved */
  (pToData)[5] |= ((pFrom)->m_nReserved) & 0xFF;
  (pToData)[4] |= ((pFrom)->m_nReserved >> 8) &0xFF;

  /* Pack Member: m_nnSmac */
  (pToData)[3] |= ((pFrom)->m_nnSmac) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nnSmac >> 8) &0xFF;
  (pToData)[1] |= ((pFrom)->m_nnSmac >> 16) &0xFF;
  (pToData)[0] |= ((pFrom)->m_nnSmac >> 24) &0xFF;
  (pToData)[7] |= ((pFrom)->m_nnSmac >> 32) &0xFF;
  (pToData)[6] |= ((pFrom)->m_nnSmac >> 40) &0xFF;
#else
  int i;
  int size = SB_ZF_ZFKAEPIP_PORT_VRID_SMAC_TABLEENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nReserved */
  (pToData)[6] |= ((pFrom)->m_nReserved) & 0xFF;
  (pToData)[7] |= ((pFrom)->m_nReserved >> 8) &0xFF;

  /* Pack Member: m_nnSmac */
  (pToData)[0] |= ((pFrom)->m_nnSmac) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nnSmac >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nnSmac >> 16) &0xFF;
  (pToData)[3] |= ((pFrom)->m_nnSmac >> 24) &0xFF;
  (pToData)[4] |= ((pFrom)->m_nnSmac >> 32) &0xFF;
  (pToData)[5] |= ((pFrom)->m_nnSmac >> 40) &0xFF;
#endif

  return SB_ZF_ZFKAEPIP_PORT_VRID_SMAC_TABLEENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaEpIpPortVridSmacTableEntry_Unpack(sbZfKaEpIpPortVridSmacTableEntry_t *pToStruct,
                                        uint8_t *pFromData,
                                        uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32_t)  (pFromData)[5] ;
  (pToStruct)->m_nReserved |=  (uint32_t)  (pFromData)[4] << 8;

  /* Unpack Member: m_nnSmac */
  COMPILER_64_SET((pToStruct)->m_nnSmac, 0,  (unsigned int) (pFromData)[3]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnSmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[2]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnSmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[1]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnSmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[0]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnSmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[7]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnSmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[6]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32_t)  (pFromData)[6] ;
  (pToStruct)->m_nReserved |=  (uint32_t)  (pFromData)[7] << 8;

  /* Unpack Member: m_nnSmac */
  COMPILER_64_SET((pToStruct)->m_nnSmac, 0,  (unsigned int) (pFromData)[0]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnSmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[1]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnSmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[2]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnSmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[3]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnSmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[4]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->m_nnSmac;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[5]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaEpIpPortVridSmacTableEntry_InitInstance(sbZfKaEpIpPortVridSmacTableEntry_t *pFrame) {

  pFrame->m_nReserved =  (unsigned int)  0;
  pFrame->m_nnSmac =  (uint64_t)  0;

}
