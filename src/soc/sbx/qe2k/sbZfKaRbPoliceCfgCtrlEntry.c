/*
 * $Id: sbZfKaRbPoliceCfgCtrlEntry.c 1.1.44.4 Broadcom SDK $
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
#include "sbZfKaRbPoliceCfgCtrlEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfKaRbPoliceCfgCtrlEntry_Pack(sbZfKaRbPoliceCfgCtrlEntry_t *pFrom,
                                uint8_t *pToData,
                                uint32_t nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKARBPOLCFGCTRLENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nReserved */
  (pToData)[0] |= ((pFrom)->m_nReserved & 0x1f) <<3;

  /* Pack Member: m_nEnable */
  (pToData)[0] |= ((pFrom)->m_nEnable & 0x01) <<2;

  /* Pack Member: m_nNotBlind */
  (pToData)[0] |= ((pFrom)->m_nNotBlind & 0x01) <<1;

  /* Pack Member: m_nDropOnRed */
  (pToData)[0] |= ((pFrom)->m_nDropOnRed & 0x01);

  /* Pack Member: m_nEnableMon */
  (pToData)[1] |= ((pFrom)->m_nEnableMon & 0x01) <<7;

  /* Pack Member: m_nMonCntId */
  (pToData)[1] |= ((pFrom)->m_nMonCntId & 0x07) <<4;

  /* Pack Member: m_nIncRate */
  (pToData)[3] |= ((pFrom)->m_nIncRate) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_nIncRate >> 8) &0xFF;
  (pToData)[1] |= ((pFrom)->m_nIncRate >> 16) & 0x0f;
#else
  int i;
  int size = SB_ZF_ZFKARBPOLCFGCTRLENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nReserved */
  (pToData)[3] |= ((pFrom)->m_nReserved & 0x1f) <<3;

  /* Pack Member: m_nEnable */
  (pToData)[3] |= ((pFrom)->m_nEnable & 0x01) <<2;

  /* Pack Member: m_nNotBlind */
  (pToData)[3] |= ((pFrom)->m_nNotBlind & 0x01) <<1;

  /* Pack Member: m_nDropOnRed */
  (pToData)[3] |= ((pFrom)->m_nDropOnRed & 0x01);

  /* Pack Member: m_nEnableMon */
  (pToData)[2] |= ((pFrom)->m_nEnableMon & 0x01) <<7;

  /* Pack Member: m_nMonCntId */
  (pToData)[2] |= ((pFrom)->m_nMonCntId & 0x07) <<4;

  /* Pack Member: m_nIncRate */
  (pToData)[0] |= ((pFrom)->m_nIncRate) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_nIncRate >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nIncRate >> 16) & 0x0f;
#endif

  return SB_ZF_ZFKARBPOLCFGCTRLENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaRbPoliceCfgCtrlEntry_Unpack(sbZfKaRbPoliceCfgCtrlEntry_t *pToStruct,
                                  uint8_t *pFromData,
                                  uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32_t)  ((pFromData)[0] >> 3) & 0x1f;

  /* Unpack Member: m_nEnable */
  (pToStruct)->m_nEnable =  (uint8_t)  ((pFromData)[0] >> 2) & 0x01;

  /* Unpack Member: m_nNotBlind */
  (pToStruct)->m_nNotBlind =  (uint8_t)  ((pFromData)[0] >> 1) & 0x01;

  /* Unpack Member: m_nDropOnRed */
  (pToStruct)->m_nDropOnRed =  (uint8_t)  ((pFromData)[0] ) & 0x01;

  /* Unpack Member: m_nEnableMon */
  (pToStruct)->m_nEnableMon =  (uint8_t)  ((pFromData)[1] >> 7) & 0x01;

  /* Unpack Member: m_nMonCntId */
  (pToStruct)->m_nMonCntId =  (uint32_t)  ((pFromData)[1] >> 4) & 0x07;

  /* Unpack Member: m_nIncRate */
  (pToStruct)->m_nIncRate =  (uint32_t)  (pFromData)[3] ;
  (pToStruct)->m_nIncRate |=  (uint32_t)  (pFromData)[2] << 8;
  (pToStruct)->m_nIncRate |=  (uint32_t)  ((pFromData)[1] & 0x0f) << 16;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nReserved */
  (pToStruct)->m_nReserved =  (uint32_t)  ((pFromData)[3] >> 3) & 0x1f;

  /* Unpack Member: m_nEnable */
  (pToStruct)->m_nEnable =  (uint8_t)  ((pFromData)[3] >> 2) & 0x01;

  /* Unpack Member: m_nNotBlind */
  (pToStruct)->m_nNotBlind =  (uint8_t)  ((pFromData)[3] >> 1) & 0x01;

  /* Unpack Member: m_nDropOnRed */
  (pToStruct)->m_nDropOnRed =  (uint8_t)  ((pFromData)[3] ) & 0x01;

  /* Unpack Member: m_nEnableMon */
  (pToStruct)->m_nEnableMon =  (uint8_t)  ((pFromData)[2] >> 7) & 0x01;

  /* Unpack Member: m_nMonCntId */
  (pToStruct)->m_nMonCntId =  (uint32_t)  ((pFromData)[2] >> 4) & 0x07;

  /* Unpack Member: m_nIncRate */
  (pToStruct)->m_nIncRate =  (uint32_t)  (pFromData)[0] ;
  (pToStruct)->m_nIncRate |=  (uint32_t)  (pFromData)[1] << 8;
  (pToStruct)->m_nIncRate |=  (uint32_t)  ((pFromData)[2] & 0x0f) << 16;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaRbPoliceCfgCtrlEntry_InitInstance(sbZfKaRbPoliceCfgCtrlEntry_t *pFrame) {

  pFrame->m_nReserved =  (unsigned int)  0;
  pFrame->m_nEnable =  (unsigned int)  0;
  pFrame->m_nNotBlind =  (unsigned int)  0;
  pFrame->m_nDropOnRed =  (unsigned int)  0;
  pFrame->m_nEnableMon =  (unsigned int)  0;
  pFrame->m_nMonCntId =  (unsigned int)  0;
  pFrame->m_nIncRate =  (unsigned int)  0;

}
