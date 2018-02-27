/*
 * $Id: sbZfFabBm9600NmFullStatusEntry.c 1.1.16.4 Broadcom SDK $
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
#include "sbZfFabBm9600NmFullStatusEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfFabBm9600NmFullStatusEntry_Pack(sbZfFabBm9600NmFullStatusEntry_t *pFrom,
                                    uint8_t *pToData,
                                    uint32_t nMaxToDataIndex) {
#ifdef SAND_BIG_ENDIAN_HOST
  int i;
  int size = SB_ZF_FAB_BM9600_NMFULLSTATUSENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on bigword endian */

  /* Pack Member: m_FullStatus8 */
  (pToData)[35] |= ((pFrom)->m_FullStatus8) & 0xFF;
  (pToData)[34] |= ((pFrom)->m_FullStatus8 >> 8) & 0x03;

  /* Pack Member: m_FullStatus7 */
  (pToData)[31] |= ((pFrom)->m_FullStatus7) & 0xFF;
  (pToData)[30] |= ((pFrom)->m_FullStatus7 >> 8) &0xFF;
  (pToData)[29] |= ((pFrom)->m_FullStatus7 >> 16) &0xFF;
  (pToData)[28] |= ((pFrom)->m_FullStatus7 >> 24) &0xFF;

  /* Pack Member: m_FullStatus6 */
  (pToData)[27] |= ((pFrom)->m_FullStatus6) & 0xFF;
  (pToData)[26] |= ((pFrom)->m_FullStatus6 >> 8) &0xFF;
  (pToData)[25] |= ((pFrom)->m_FullStatus6 >> 16) &0xFF;
  (pToData)[24] |= ((pFrom)->m_FullStatus6 >> 24) &0xFF;

  /* Pack Member: m_FullStatus5 */
  (pToData)[23] |= ((pFrom)->m_FullStatus5) & 0xFF;
  (pToData)[22] |= ((pFrom)->m_FullStatus5 >> 8) &0xFF;
  (pToData)[21] |= ((pFrom)->m_FullStatus5 >> 16) &0xFF;
  (pToData)[20] |= ((pFrom)->m_FullStatus5 >> 24) &0xFF;

  /* Pack Member: m_FullStatus4 */
  (pToData)[19] |= ((pFrom)->m_FullStatus4) & 0xFF;
  (pToData)[18] |= ((pFrom)->m_FullStatus4 >> 8) &0xFF;
  (pToData)[17] |= ((pFrom)->m_FullStatus4 >> 16) &0xFF;
  (pToData)[16] |= ((pFrom)->m_FullStatus4 >> 24) &0xFF;

  /* Pack Member: m_FullStatus3 */
  (pToData)[15] |= ((pFrom)->m_FullStatus3) & 0xFF;
  (pToData)[14] |= ((pFrom)->m_FullStatus3 >> 8) &0xFF;
  (pToData)[13] |= ((pFrom)->m_FullStatus3 >> 16) &0xFF;
  (pToData)[12] |= ((pFrom)->m_FullStatus3 >> 24) &0xFF;

  /* Pack Member: m_FullStatus2 */
  (pToData)[11] |= ((pFrom)->m_FullStatus2) & 0xFF;
  (pToData)[10] |= ((pFrom)->m_FullStatus2 >> 8) &0xFF;
  (pToData)[9] |= ((pFrom)->m_FullStatus2 >> 16) &0xFF;
  (pToData)[8] |= ((pFrom)->m_FullStatus2 >> 24) &0xFF;

  /* Pack Member: m_FullStatus1 */
  (pToData)[7] |= ((pFrom)->m_FullStatus1) & 0xFF;
  (pToData)[6] |= ((pFrom)->m_FullStatus1 >> 8) &0xFF;
  (pToData)[5] |= ((pFrom)->m_FullStatus1 >> 16) &0xFF;
  (pToData)[4] |= ((pFrom)->m_FullStatus1 >> 24) &0xFF;

  /* Pack Member: m_FullStatus0 */
  (pToData)[3] |= ((pFrom)->m_FullStatus0) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_FullStatus0 >> 8) &0xFF;
  (pToData)[1] |= ((pFrom)->m_FullStatus0 >> 16) &0xFF;
  (pToData)[0] |= ((pFrom)->m_FullStatus0 >> 24) &0xFF;
#else
  int i;
  int size = SB_ZF_FAB_BM9600_NMFULLSTATUSENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_FullStatus8 */
  (pToData)[32] |= ((pFrom)->m_FullStatus8) & 0xFF;
  (pToData)[33] |= ((pFrom)->m_FullStatus8 >> 8) & 0x03;

  /* Pack Member: m_FullStatus7 */
  (pToData)[28] |= ((pFrom)->m_FullStatus7) & 0xFF;
  (pToData)[29] |= ((pFrom)->m_FullStatus7 >> 8) &0xFF;
  (pToData)[30] |= ((pFrom)->m_FullStatus7 >> 16) &0xFF;
  (pToData)[31] |= ((pFrom)->m_FullStatus7 >> 24) &0xFF;

  /* Pack Member: m_FullStatus6 */
  (pToData)[24] |= ((pFrom)->m_FullStatus6) & 0xFF;
  (pToData)[25] |= ((pFrom)->m_FullStatus6 >> 8) &0xFF;
  (pToData)[26] |= ((pFrom)->m_FullStatus6 >> 16) &0xFF;
  (pToData)[27] |= ((pFrom)->m_FullStatus6 >> 24) &0xFF;

  /* Pack Member: m_FullStatus5 */
  (pToData)[20] |= ((pFrom)->m_FullStatus5) & 0xFF;
  (pToData)[21] |= ((pFrom)->m_FullStatus5 >> 8) &0xFF;
  (pToData)[22] |= ((pFrom)->m_FullStatus5 >> 16) &0xFF;
  (pToData)[23] |= ((pFrom)->m_FullStatus5 >> 24) &0xFF;

  /* Pack Member: m_FullStatus4 */
  (pToData)[16] |= ((pFrom)->m_FullStatus4) & 0xFF;
  (pToData)[17] |= ((pFrom)->m_FullStatus4 >> 8) &0xFF;
  (pToData)[18] |= ((pFrom)->m_FullStatus4 >> 16) &0xFF;
  (pToData)[19] |= ((pFrom)->m_FullStatus4 >> 24) &0xFF;

  /* Pack Member: m_FullStatus3 */
  (pToData)[12] |= ((pFrom)->m_FullStatus3) & 0xFF;
  (pToData)[13] |= ((pFrom)->m_FullStatus3 >> 8) &0xFF;
  (pToData)[14] |= ((pFrom)->m_FullStatus3 >> 16) &0xFF;
  (pToData)[15] |= ((pFrom)->m_FullStatus3 >> 24) &0xFF;

  /* Pack Member: m_FullStatus2 */
  (pToData)[8] |= ((pFrom)->m_FullStatus2) & 0xFF;
  (pToData)[9] |= ((pFrom)->m_FullStatus2 >> 8) &0xFF;
  (pToData)[10] |= ((pFrom)->m_FullStatus2 >> 16) &0xFF;
  (pToData)[11] |= ((pFrom)->m_FullStatus2 >> 24) &0xFF;

  /* Pack Member: m_FullStatus1 */
  (pToData)[4] |= ((pFrom)->m_FullStatus1) & 0xFF;
  (pToData)[5] |= ((pFrom)->m_FullStatus1 >> 8) &0xFF;
  (pToData)[6] |= ((pFrom)->m_FullStatus1 >> 16) &0xFF;
  (pToData)[7] |= ((pFrom)->m_FullStatus1 >> 24) &0xFF;

  /* Pack Member: m_FullStatus0 */
  (pToData)[0] |= ((pFrom)->m_FullStatus0) & 0xFF;
  (pToData)[1] |= ((pFrom)->m_FullStatus0 >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->m_FullStatus0 >> 16) &0xFF;
  (pToData)[3] |= ((pFrom)->m_FullStatus0 >> 24) &0xFF;
#endif

  return SB_ZF_FAB_BM9600_NMFULLSTATUSENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFabBm9600NmFullStatusEntry_Unpack(sbZfFabBm9600NmFullStatusEntry_t *pToStruct,
                                      uint8_t *pFromData,
                                      uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;

#ifdef SAND_BIG_ENDIAN_HOST

  /* Unpack operation based on bigword endian */

  /* Unpack Member: m_FullStatus8 */
  (pToStruct)->m_FullStatus8 =  (uint32_t)  (pFromData)[35] ;
  (pToStruct)->m_FullStatus8 |=  (uint32_t)  ((pFromData)[34] & 0x03) << 8;

  /* Unpack Member: m_FullStatus7 */
  (pToStruct)->m_FullStatus7 =  (uint32_t)  (pFromData)[31] ;
  (pToStruct)->m_FullStatus7 |=  (uint32_t)  (pFromData)[30] << 8;
  (pToStruct)->m_FullStatus7 |=  (uint32_t)  (pFromData)[29] << 16;
  (pToStruct)->m_FullStatus7 |=  (uint32_t)  (pFromData)[28] << 24;

  /* Unpack Member: m_FullStatus6 */
  (pToStruct)->m_FullStatus6 =  (uint32_t)  (pFromData)[27] ;
  (pToStruct)->m_FullStatus6 |=  (uint32_t)  (pFromData)[26] << 8;
  (pToStruct)->m_FullStatus6 |=  (uint32_t)  (pFromData)[25] << 16;
  (pToStruct)->m_FullStatus6 |=  (uint32_t)  (pFromData)[24] << 24;

  /* Unpack Member: m_FullStatus5 */
  (pToStruct)->m_FullStatus5 =  (uint32_t)  (pFromData)[23] ;
  (pToStruct)->m_FullStatus5 |=  (uint32_t)  (pFromData)[22] << 8;
  (pToStruct)->m_FullStatus5 |=  (uint32_t)  (pFromData)[21] << 16;
  (pToStruct)->m_FullStatus5 |=  (uint32_t)  (pFromData)[20] << 24;

  /* Unpack Member: m_FullStatus4 */
  (pToStruct)->m_FullStatus4 =  (uint32_t)  (pFromData)[19] ;
  (pToStruct)->m_FullStatus4 |=  (uint32_t)  (pFromData)[18] << 8;
  (pToStruct)->m_FullStatus4 |=  (uint32_t)  (pFromData)[17] << 16;
  (pToStruct)->m_FullStatus4 |=  (uint32_t)  (pFromData)[16] << 24;

  /* Unpack Member: m_FullStatus3 */
  (pToStruct)->m_FullStatus3 =  (uint32_t)  (pFromData)[15] ;
  (pToStruct)->m_FullStatus3 |=  (uint32_t)  (pFromData)[14] << 8;
  (pToStruct)->m_FullStatus3 |=  (uint32_t)  (pFromData)[13] << 16;
  (pToStruct)->m_FullStatus3 |=  (uint32_t)  (pFromData)[12] << 24;

  /* Unpack Member: m_FullStatus2 */
  (pToStruct)->m_FullStatus2 =  (uint32_t)  (pFromData)[11] ;
  (pToStruct)->m_FullStatus2 |=  (uint32_t)  (pFromData)[10] << 8;
  (pToStruct)->m_FullStatus2 |=  (uint32_t)  (pFromData)[9] << 16;
  (pToStruct)->m_FullStatus2 |=  (uint32_t)  (pFromData)[8] << 24;

  /* Unpack Member: m_FullStatus1 */
  (pToStruct)->m_FullStatus1 =  (uint32_t)  (pFromData)[7] ;
  (pToStruct)->m_FullStatus1 |=  (uint32_t)  (pFromData)[6] << 8;
  (pToStruct)->m_FullStatus1 |=  (uint32_t)  (pFromData)[5] << 16;
  (pToStruct)->m_FullStatus1 |=  (uint32_t)  (pFromData)[4] << 24;

  /* Unpack Member: m_FullStatus0 */
  (pToStruct)->m_FullStatus0 =  (uint32_t)  (pFromData)[3] ;
  (pToStruct)->m_FullStatus0 |=  (uint32_t)  (pFromData)[2] << 8;
  (pToStruct)->m_FullStatus0 |=  (uint32_t)  (pFromData)[1] << 16;
  (pToStruct)->m_FullStatus0 |=  (uint32_t)  (pFromData)[0] << 24;
#else

  /* Unpack operation based on little endian */

  /* Unpack Member: m_FullStatus8 */
  (pToStruct)->m_FullStatus8 =  (uint32_t)  (pFromData)[32] ;
  (pToStruct)->m_FullStatus8 |=  (uint32_t)  ((pFromData)[33] & 0x03) << 8;

  /* Unpack Member: m_FullStatus7 */
  (pToStruct)->m_FullStatus7 =  (uint32_t)  (pFromData)[28] ;
  (pToStruct)->m_FullStatus7 |=  (uint32_t)  (pFromData)[29] << 8;
  (pToStruct)->m_FullStatus7 |=  (uint32_t)  (pFromData)[30] << 16;
  (pToStruct)->m_FullStatus7 |=  (uint32_t)  (pFromData)[31] << 24;

  /* Unpack Member: m_FullStatus6 */
  (pToStruct)->m_FullStatus6 =  (uint32_t)  (pFromData)[24] ;
  (pToStruct)->m_FullStatus6 |=  (uint32_t)  (pFromData)[25] << 8;
  (pToStruct)->m_FullStatus6 |=  (uint32_t)  (pFromData)[26] << 16;
  (pToStruct)->m_FullStatus6 |=  (uint32_t)  (pFromData)[27] << 24;

  /* Unpack Member: m_FullStatus5 */
  (pToStruct)->m_FullStatus5 =  (uint32_t)  (pFromData)[20] ;
  (pToStruct)->m_FullStatus5 |=  (uint32_t)  (pFromData)[21] << 8;
  (pToStruct)->m_FullStatus5 |=  (uint32_t)  (pFromData)[22] << 16;
  (pToStruct)->m_FullStatus5 |=  (uint32_t)  (pFromData)[23] << 24;

  /* Unpack Member: m_FullStatus4 */
  (pToStruct)->m_FullStatus4 =  (uint32_t)  (pFromData)[16] ;
  (pToStruct)->m_FullStatus4 |=  (uint32_t)  (pFromData)[17] << 8;
  (pToStruct)->m_FullStatus4 |=  (uint32_t)  (pFromData)[18] << 16;
  (pToStruct)->m_FullStatus4 |=  (uint32_t)  (pFromData)[19] << 24;

  /* Unpack Member: m_FullStatus3 */
  (pToStruct)->m_FullStatus3 =  (uint32_t)  (pFromData)[12] ;
  (pToStruct)->m_FullStatus3 |=  (uint32_t)  (pFromData)[13] << 8;
  (pToStruct)->m_FullStatus3 |=  (uint32_t)  (pFromData)[14] << 16;
  (pToStruct)->m_FullStatus3 |=  (uint32_t)  (pFromData)[15] << 24;

  /* Unpack Member: m_FullStatus2 */
  (pToStruct)->m_FullStatus2 =  (uint32_t)  (pFromData)[8] ;
  (pToStruct)->m_FullStatus2 |=  (uint32_t)  (pFromData)[9] << 8;
  (pToStruct)->m_FullStatus2 |=  (uint32_t)  (pFromData)[10] << 16;
  (pToStruct)->m_FullStatus2 |=  (uint32_t)  (pFromData)[11] << 24;

  /* Unpack Member: m_FullStatus1 */
  (pToStruct)->m_FullStatus1 =  (uint32_t)  (pFromData)[4] ;
  (pToStruct)->m_FullStatus1 |=  (uint32_t)  (pFromData)[5] << 8;
  (pToStruct)->m_FullStatus1 |=  (uint32_t)  (pFromData)[6] << 16;
  (pToStruct)->m_FullStatus1 |=  (uint32_t)  (pFromData)[7] << 24;

  /* Unpack Member: m_FullStatus0 */
  (pToStruct)->m_FullStatus0 =  (uint32_t)  (pFromData)[0] ;
  (pToStruct)->m_FullStatus0 |=  (uint32_t)  (pFromData)[1] << 8;
  (pToStruct)->m_FullStatus0 |=  (uint32_t)  (pFromData)[2] << 16;
  (pToStruct)->m_FullStatus0 |=  (uint32_t)  (pFromData)[3] << 24;
#endif

}



/* initialize an instance of this zframe */
void
sbZfFabBm9600NmFullStatusEntry_InitInstance(sbZfFabBm9600NmFullStatusEntry_t *pFrame) {

  pFrame->m_FullStatus8 =  (unsigned int)  0;
  pFrame->m_FullStatus7 =  (unsigned int)  0;
  pFrame->m_FullStatus6 =  (unsigned int)  0;
  pFrame->m_FullStatus5 =  (unsigned int)  0;
  pFrame->m_FullStatus4 =  (unsigned int)  0;
  pFrame->m_FullStatus3 =  (unsigned int)  0;
  pFrame->m_FullStatus2 =  (unsigned int)  0;
  pFrame->m_FullStatus1 =  (unsigned int)  0;
  pFrame->m_FullStatus0 =  (unsigned int)  0;

}
