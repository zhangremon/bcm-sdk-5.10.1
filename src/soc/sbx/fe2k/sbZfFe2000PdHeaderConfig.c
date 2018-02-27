/*
 * $Id: sbZfFe2000PdHeaderConfig.c 1.3.36.4 Broadcom SDK $
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
#include "sbZfFe2000PdHeaderConfig.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfFe2000PdHeaderConfig_Pack(sbZfFe2000PdHeaderConfig_t *pFrom,
                              uint8_t *pToData,
                              uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_FE2000PDHEADERCONFIG_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: m_uSpare0 */
  (pToData)[3] |= ((pFrom)->m_uSpare0 & 0x3f) <<2;

  /* Pack Member: m_uLengthUnits */
  (pToData)[3] |= ((pFrom)->m_uLengthUnits & 0x03);

  /* Pack Member: m_uSpare1 */
  (pToData)[2] |= ((pFrom)->m_uSpare1 & 0x01) <<7;

  /* Pack Member: m_uLengthSize */
  (pToData)[2] |= ((pFrom)->m_uLengthSize & 0x07) <<4;

  /* Pack Member: m_uSpare2 */
  (pToData)[2] |= ((pFrom)->m_uSpare2 & 0x07) <<1;

  /* Pack Member: m_uLengthPosition */
  (pToData)[1] |= ((pFrom)->m_uLengthPosition) & 0xFF;
  (pToData)[2] |= ((pFrom)->m_uLengthPosition >> 8) & 0x01;

  /* Pack Member: m_uSpare3 */
  (pToData)[0] |= ((pFrom)->m_uSpare3 & 0x03) <<6;

  /* Pack Member: m_uBaseLength */
  (pToData)[0] |= ((pFrom)->m_uBaseLength & 0x3f);

  return SB_ZF_FE2000PDHEADERCONFIG_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFe2000PdHeaderConfig_Unpack(sbZfFe2000PdHeaderConfig_t *pToStruct,
                                uint8_t *pFromData,
                                uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: m_uSpare0 */
  (pToStruct)->m_uSpare0 =  (uint32_t)  ((pFromData)[3] >> 2) & 0x3f;

  /* Unpack Member: m_uLengthUnits */
  (pToStruct)->m_uLengthUnits =  (uint32_t)  ((pFromData)[3] ) & 0x03;

  /* Unpack Member: m_uSpare1 */
  (pToStruct)->m_uSpare1 =  (uint32_t)  ((pFromData)[2] >> 7) & 0x01;

  /* Unpack Member: m_uLengthSize */
  (pToStruct)->m_uLengthSize =  (uint32_t)  ((pFromData)[2] >> 4) & 0x07;

  /* Unpack Member: m_uSpare2 */
  (pToStruct)->m_uSpare2 =  (uint32_t)  ((pFromData)[2] >> 1) & 0x07;

  /* Unpack Member: m_uLengthPosition */
  (pToStruct)->m_uLengthPosition =  (uint32_t)  (pFromData)[1] ;
  (pToStruct)->m_uLengthPosition |=  (uint32_t)  ((pFromData)[2] & 0x01) << 8;

  /* Unpack Member: m_uSpare3 */
  (pToStruct)->m_uSpare3 =  (uint32_t)  ((pFromData)[0] >> 6) & 0x03;

  /* Unpack Member: m_uBaseLength */
  (pToStruct)->m_uBaseLength =  (uint32_t)  ((pFromData)[0] ) & 0x3f;

}



/* initialize an instance of this zframe */
void
sbZfFe2000PdHeaderConfig_InitInstance(sbZfFe2000PdHeaderConfig_t *pFrame) {

  pFrame->m_uSpare0 =  (unsigned int)  0;
  pFrame->m_uLengthUnits =  (unsigned int)  0;
  pFrame->m_uSpare1 =  (unsigned int)  0;
  pFrame->m_uLengthSize =  (unsigned int)  0;
  pFrame->m_uSpare2 =  (unsigned int)  0;
  pFrame->m_uLengthPosition =  (unsigned int)  0;
  pFrame->m_uSpare3 =  (unsigned int)  0;
  pFrame->m_uBaseLength =  (unsigned int)  0;

}
