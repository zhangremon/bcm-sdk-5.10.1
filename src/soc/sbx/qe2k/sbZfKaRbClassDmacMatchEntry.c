/*
 * $Id: sbZfKaRbClassDmacMatchEntry.c 1.1.44.4 Broadcom SDK $
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
#include "sbZfKaRbClassDmacMatchEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfKaRbClassDmacMatchEntry_Pack(sbZfKaRbClassDmacMatchEntry_t *pFrom,
                                 uint8_t *pToData,
                                 uint32_t nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_ZFKARBCLASSDMACMATCHENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_nDmacDataLsb */
  (pToData)[19] |= ((pFrom)->m_nDmacDataLsb) & 0xFF;
  (pToData)[18] |= ((pFrom)->m_nDmacDataLsb >> 8) &0xFF;
  (pToData)[17] |= ((pFrom)->m_nDmacDataLsb >> 16) &0xFF;
  (pToData)[16] |= ((pFrom)->m_nDmacDataLsb >> 24) &0xFF;

  /* Pack Member: m_nDmacDataRsv */
  (pToData)[13] |= ((pFrom)->m_nDmacDataRsv) & 0xFF;
  (pToData)[12] |= ((pFrom)->m_nDmacDataRsv >> 8) &0xFF;

  /* Pack Member: m_nDmacDataMsb */
  (pToData)[15] |= ((pFrom)->m_nDmacDataMsb) & 0xFF;
  (pToData)[14] |= ((pFrom)->m_nDmacDataMsb >> 8) &0xFF;

  /* Pack Member: m_nDmacMaskLsb */
  (pToData)[11] |= ((pFrom)->m_nDmacMaskLsb) & 0xFF;
  (pToData)[10] |= ((pFrom)->m_nDmacMaskLsb >> 8) &0xFF;
  (pToData)[9] |= ((pFrom)->m_nDmacMaskLsb >> 16) &0xFF;
  (pToData)[8] |= ((pFrom)->m_nDmacMaskLsb >> 24) &0xFF;

  /* Pack Member: m_nDmacMaskRsv */
  (pToData)[5] |= ((pFrom)->m_nDmacMaskRsv) & 0xFF;
  (pToData)[4] |= ((pFrom)->m_nDmacMaskRsv >> 8) &0xFF;

  /* Pack Member: m_nDmacMaskMsb */
  (pToData)[7] |= ((pFrom)->m_nDmacMaskMsb) & 0xFF;
  (pToData)[6] |= ((pFrom)->m_nDmacMaskMsb >> 8) &0xFF;

  /* Pack Member: m_nDmacReserve */
  (pToData)[3] |= ((pFrom)->m_nDmacReserve & 0x01) <<7;
  (pToData)[2] |= ((pFrom)->m_nDmacReserve >> 1) &0xFF;
  (pToData)[1] |= ((pFrom)->m_nDmacReserve >> 9) &0xFF;
  (pToData)[0] |= ((pFrom)->m_nDmacReserve >> 17) &0xFF;

  /* Pack Member: m_nDmacEnable */
  (pToData)[3] |= ((pFrom)->m_nDmacEnable & 0x01) <<6;

  /* Pack Member: m_nDmacDp */
  (pToData)[3] |= ((pFrom)->m_nDmacDp & 0x03) <<4;

  /* Pack Member: m_nDmacLsb */
  (pToData)[3] |= ((pFrom)->m_nDmacLsb & 0x0f);
#else
  int i;
  int size = SB_ZF_ZFKARBCLASSDMACMATCHENTRY_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_nDmacDataLsb */
  (pToData)[16] |= ((pFrom)->m_nDmacDataLsb) & 0xFF;
  (pToData)[17] |= ((pFrom)->m_nDmacDataLsb >> 8) &0xFF;
  (pToData)[18] |= ((pFrom)->m_nDmacDataLsb >> 16) &0xFF;
  (pToData)[19] |= ((pFrom)->m_nDmacDataLsb >> 24) &0xFF;

  /* Pack Member: m_nDmacDataRsv */
  (pToData)[14] |= ((pFrom)->m_nDmacDataRsv) & 0xFF;
  (pToData)[15] |= ((pFrom)->m_nDmacDataRsv >> 8) &0xFF;

  /* Pack Member: m_nDmacDataMsb */
  (pToData)[12] |= ((pFrom)->m_nDmacDataMsb) & 0xFF;
  (pToData)[13] |= ((pFrom)->m_nDmacDataMsb >> 8) &0xFF;

  /* Pack Member: m_nDmacMaskLsb */
  (pToData)[8] |= ((pFrom)->m_nDmacMaskLsb) & 0xFF;
  (pToData)[9] |= ((pFrom)->m_nDmacMaskLsb >> 8) &0xFF;
  (pToData)[10] |= ((pFrom)->m_nDmacMaskLsb >> 16) &0xFF;
  (pToData)[11] |= ((pFrom)->m_nDmacMaskLsb >> 24) &0xFF;

  /* Pack Member: m_nDmacMaskRsv */
  (pToData)[6] |= ((pFrom)->m_nDmacMaskRsv) & 0xFF;
  (pToData)[7] |= ((pFrom)->m_nDmacMaskRsv >> 8) &0xFF;

  /* Pack Member: m_nDmacMaskMsb */
  (pToData)[4] |= ((pFrom)->m_nDmacMaskMsb) & 0xFF;
  (pToData)[5] |= ((pFrom)->m_nDmacMaskMsb >> 8) &0xFF;

  /* Pack Member: m_nDmacReserve */
  (pToData)[0] |= ((pFrom)->m_nDmacReserve & 0x01) <<7;
  (pToData)[1] |= ((pFrom)->m_nDmacReserve >> 1) &0xFF;
  (pToData)[2] |= ((pFrom)->m_nDmacReserve >> 9) &0xFF;
  (pToData)[3] |= ((pFrom)->m_nDmacReserve >> 17) &0xFF;

  /* Pack Member: m_nDmacEnable */
  (pToData)[0] |= ((pFrom)->m_nDmacEnable & 0x01) <<6;

  /* Pack Member: m_nDmacDp */
  (pToData)[0] |= ((pFrom)->m_nDmacDp & 0x03) <<4;

  /* Pack Member: m_nDmacLsb */
  (pToData)[0] |= ((pFrom)->m_nDmacLsb & 0x0f);
#endif

  return SB_ZF_ZFKARBCLASSDMACMATCHENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfKaRbClassDmacMatchEntry_Unpack(sbZfKaRbClassDmacMatchEntry_t *pToStruct,
                                   uint8_t *pFromData,
                                   uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_nDmacDataLsb */
  (pToStruct)->m_nDmacDataLsb =  (uint32_t)  (pFromData)[19] ;
  (pToStruct)->m_nDmacDataLsb |=  (uint32_t)  (pFromData)[18] << 8;
  (pToStruct)->m_nDmacDataLsb |=  (uint32_t)  (pFromData)[17] << 16;
  (pToStruct)->m_nDmacDataLsb |=  (uint32_t)  (pFromData)[16] << 24;

  /* Unpack Member: m_nDmacDataRsv */
  (pToStruct)->m_nDmacDataRsv =  (uint32_t)  (pFromData)[13] ;
  (pToStruct)->m_nDmacDataRsv |=  (uint32_t)  (pFromData)[12] << 8;

  /* Unpack Member: m_nDmacDataMsb */
  (pToStruct)->m_nDmacDataMsb =  (uint32_t)  (pFromData)[15] ;
  (pToStruct)->m_nDmacDataMsb |=  (uint32_t)  (pFromData)[14] << 8;

  /* Unpack Member: m_nDmacMaskLsb */
  (pToStruct)->m_nDmacMaskLsb =  (uint32_t)  (pFromData)[11] ;
  (pToStruct)->m_nDmacMaskLsb |=  (uint32_t)  (pFromData)[10] << 8;
  (pToStruct)->m_nDmacMaskLsb |=  (uint32_t)  (pFromData)[9] << 16;
  (pToStruct)->m_nDmacMaskLsb |=  (uint32_t)  (pFromData)[8] << 24;

  /* Unpack Member: m_nDmacMaskRsv */
  (pToStruct)->m_nDmacMaskRsv =  (uint32_t)  (pFromData)[5] ;
  (pToStruct)->m_nDmacMaskRsv |=  (uint32_t)  (pFromData)[4] << 8;

  /* Unpack Member: m_nDmacMaskMsb */
  (pToStruct)->m_nDmacMaskMsb =  (uint32_t)  (pFromData)[7] ;
  (pToStruct)->m_nDmacMaskMsb |=  (uint32_t)  (pFromData)[6] << 8;

  /* Unpack Member: m_nDmacReserve */
  (pToStruct)->m_nDmacReserve =  (uint32_t)  ((pFromData)[3] >> 7) & 0x01;
  (pToStruct)->m_nDmacReserve |=  (uint32_t)  (pFromData)[2] << 1;
  (pToStruct)->m_nDmacReserve |=  (uint32_t)  (pFromData)[1] << 9;
  (pToStruct)->m_nDmacReserve |=  (uint32_t)  (pFromData)[0] << 17;

  /* Unpack Member: m_nDmacEnable */
  (pToStruct)->m_nDmacEnable =  (uint8_t)  ((pFromData)[3] >> 6) & 0x01;

  /* Unpack Member: m_nDmacDp */
  (pToStruct)->m_nDmacDp =  (uint32_t)  ((pFromData)[3] >> 4) & 0x03;

  /* Unpack Member: m_nDmacLsb */
  (pToStruct)->m_nDmacLsb =  (uint32_t)  ((pFromData)[3] ) & 0x0f;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_nDmacDataLsb */
  (pToStruct)->m_nDmacDataLsb =  (uint32_t)  (pFromData)[16] ;
  (pToStruct)->m_nDmacDataLsb |=  (uint32_t)  (pFromData)[17] << 8;
  (pToStruct)->m_nDmacDataLsb |=  (uint32_t)  (pFromData)[18] << 16;
  (pToStruct)->m_nDmacDataLsb |=  (uint32_t)  (pFromData)[19] << 24;

  /* Unpack Member: m_nDmacDataRsv */
  (pToStruct)->m_nDmacDataRsv =  (uint32_t)  (pFromData)[14] ;
  (pToStruct)->m_nDmacDataRsv |=  (uint32_t)  (pFromData)[15] << 8;

  /* Unpack Member: m_nDmacDataMsb */
  (pToStruct)->m_nDmacDataMsb =  (uint32_t)  (pFromData)[12] ;
  (pToStruct)->m_nDmacDataMsb |=  (uint32_t)  (pFromData)[13] << 8;

  /* Unpack Member: m_nDmacMaskLsb */
  (pToStruct)->m_nDmacMaskLsb =  (uint32_t)  (pFromData)[8] ;
  (pToStruct)->m_nDmacMaskLsb |=  (uint32_t)  (pFromData)[9] << 8;
  (pToStruct)->m_nDmacMaskLsb |=  (uint32_t)  (pFromData)[10] << 16;
  (pToStruct)->m_nDmacMaskLsb |=  (uint32_t)  (pFromData)[11] << 24;

  /* Unpack Member: m_nDmacMaskRsv */
  (pToStruct)->m_nDmacMaskRsv =  (uint32_t)  (pFromData)[6] ;
  (pToStruct)->m_nDmacMaskRsv |=  (uint32_t)  (pFromData)[7] << 8;

  /* Unpack Member: m_nDmacMaskMsb */
  (pToStruct)->m_nDmacMaskMsb =  (uint32_t)  (pFromData)[4] ;
  (pToStruct)->m_nDmacMaskMsb |=  (uint32_t)  (pFromData)[5] << 8;

  /* Unpack Member: m_nDmacReserve */
  (pToStruct)->m_nDmacReserve =  (uint32_t)  ((pFromData)[0] >> 7) & 0x01;
  (pToStruct)->m_nDmacReserve |=  (uint32_t)  (pFromData)[1] << 1;
  (pToStruct)->m_nDmacReserve |=  (uint32_t)  (pFromData)[2] << 9;
  (pToStruct)->m_nDmacReserve |=  (uint32_t)  (pFromData)[3] << 17;

  /* Unpack Member: m_nDmacEnable */
  (pToStruct)->m_nDmacEnable =  (uint8_t)  ((pFromData)[0] >> 6) & 0x01;

  /* Unpack Member: m_nDmacDp */
  (pToStruct)->m_nDmacDp =  (uint32_t)  ((pFromData)[0] >> 4) & 0x03;

  /* Unpack Member: m_nDmacLsb */
  (pToStruct)->m_nDmacLsb =  (uint32_t)  ((pFromData)[0] ) & 0x0f;
#endif

}



/* initialize an instance of this zframe */
void
sbZfKaRbClassDmacMatchEntry_InitInstance(sbZfKaRbClassDmacMatchEntry_t *pFrame) {

  pFrame->m_nDmacDataLsb =  (unsigned int)  0;
  pFrame->m_nDmacDataRsv =  (unsigned int)  0;
  pFrame->m_nDmacDataMsb =  (unsigned int)  0;
  pFrame->m_nDmacMaskLsb =  (unsigned int)  0;
  pFrame->m_nDmacMaskRsv =  (unsigned int)  0;
  pFrame->m_nDmacMaskMsb =  (unsigned int)  0;
  pFrame->m_nDmacReserve =  (unsigned int)  0;
  pFrame->m_nDmacEnable =  (unsigned int)  0;
  pFrame->m_nDmacDp =  (unsigned int)  0;
  pFrame->m_nDmacLsb =  (unsigned int)  0;

}
