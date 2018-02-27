/*
 * $Id: sbZfFe2000PmPolicerConfig.c 1.1.34.4 Broadcom SDK $
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
#include "sbZfFe2000PmPolicerConfig.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfFe2000PmPolicerConfig_Pack(sbZfFe2000PmPolicerConfig_t *pFrom,
                               uint8_t *pToData,
                               uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_FE_2000_PM_POLICERCFG_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: uRfcMode */
  (pToData)[21] |= ((pFrom)->uRfcMode & 0x03) <<1;

  /* Pack Member: uLenShift */
  (pToData)[20] |= ((pFrom)->uLenShift & 0x03) <<6;
  (pToData)[21] |= ((pFrom)->uLenShift >> 2) & 0x01;

  /* Pack Member: uRate */
  (pToData)[16] |= ((pFrom)->uRate & 0x03) <<6;
  (pToData)[17] |= ((pFrom)->uRate >> 2) &0xFF;
  (pToData)[18] |= ((pFrom)->uRate >> 10) &0xFF;
  (pToData)[19] |= ((pFrom)->uRate >> 18) &0xFF;
  (pToData)[20] |= ((pFrom)->uRate >> 26) & 0x3f;

  /* Pack Member: uCBS */
  (pToData)[12] |= ((pFrom)->uCBS & 0x03) <<6;
  (pToData)[13] |= ((pFrom)->uCBS >> 2) &0xFF;
  (pToData)[14] |= ((pFrom)->uCBS >> 10) &0xFF;
  (pToData)[15] |= ((pFrom)->uCBS >> 18) &0xFF;
  (pToData)[16] |= ((pFrom)->uCBS >> 26) & 0x3f;

  /* Pack Member: uCIR */
  (pToData)[8] |= ((pFrom)->uCIR & 0x03) <<6;
  (pToData)[9] |= ((pFrom)->uCIR >> 2) &0xFF;
  (pToData)[10] |= ((pFrom)->uCIR >> 10) &0xFF;
  (pToData)[11] |= ((pFrom)->uCIR >> 18) &0xFF;
  (pToData)[12] |= ((pFrom)->uCIR >> 26) & 0x3f;

  /* Pack Member: uEBS */
  (pToData)[4] |= ((pFrom)->uEBS & 0x03) <<6;
  (pToData)[5] |= ((pFrom)->uEBS >> 2) &0xFF;
  (pToData)[6] |= ((pFrom)->uEBS >> 10) &0xFF;
  (pToData)[7] |= ((pFrom)->uEBS >> 18) &0xFF;
  (pToData)[8] |= ((pFrom)->uEBS >> 26) & 0x3f;

  /* Pack Member: uEIR */
  (pToData)[0] |= ((pFrom)->uEIR & 0x03) <<6;
  (pToData)[1] |= ((pFrom)->uEIR >> 2) &0xFF;
  (pToData)[2] |= ((pFrom)->uEIR >> 10) &0xFF;
  (pToData)[3] |= ((pFrom)->uEIR >> 18) &0xFF;
  (pToData)[4] |= ((pFrom)->uEIR >> 26) & 0x3f;

  /* Pack Member: uRsvd */
  (pToData)[0] |= ((pFrom)->uRsvd & 0x01) <<5;

  /* Pack Member: bBlindMode */
  (pToData)[0] |= ((pFrom)->bBlindMode & 0x01) <<4;

  /* Pack Member: bDropOnRed */
  (pToData)[0] |= ((pFrom)->bDropOnRed & 0x01) <<3;

  /* Pack Member: bCoupling */
  (pToData)[0] |= ((pFrom)->bCoupling & 0x01) <<2;

  /* Pack Member: bCBSNoDecrement */
  (pToData)[0] |= ((pFrom)->bCBSNoDecrement & 0x01) <<1;

  /* Pack Member: bEBSNoDecrement */
  (pToData)[0] |= ((pFrom)->bEBSNoDecrement & 0x01);

  return SB_ZF_FE_2000_PM_POLICERCFG_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFe2000PmPolicerConfig_Unpack(sbZfFe2000PmPolicerConfig_t *pToStruct,
                                 uint8_t *pFromData,
                                 uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: uRfcMode */
  (pToStruct)->uRfcMode =  (uint32_t)  ((pFromData)[21] >> 1) & 0x03;

  /* Unpack Member: uLenShift */
  (pToStruct)->uLenShift =  (uint32_t)  ((pFromData)[20] >> 6) & 0x03;
  (pToStruct)->uLenShift |=  (uint32_t)  ((pFromData)[21] & 0x01) << 2;

  /* Unpack Member: uRate */
  (pToStruct)->uRate =  (uint32_t)  ((pFromData)[16] >> 6) & 0x03;
  (pToStruct)->uRate |=  (uint32_t)  (pFromData)[17] << 2;
  (pToStruct)->uRate |=  (uint32_t)  (pFromData)[18] << 10;
  (pToStruct)->uRate |=  (uint32_t)  (pFromData)[19] << 18;
  (pToStruct)->uRate |=  (uint32_t)  ((pFromData)[20] & 0x3f) << 26;

  /* Unpack Member: uCBS */
  (pToStruct)->uCBS =  (uint32_t)  ((pFromData)[12] >> 6) & 0x03;
  (pToStruct)->uCBS |=  (uint32_t)  (pFromData)[13] << 2;
  (pToStruct)->uCBS |=  (uint32_t)  (pFromData)[14] << 10;
  (pToStruct)->uCBS |=  (uint32_t)  (pFromData)[15] << 18;
  (pToStruct)->uCBS |=  (uint32_t)  ((pFromData)[16] & 0x3f) << 26;

  /* Unpack Member: uCIR */
  (pToStruct)->uCIR =  (uint32_t)  ((pFromData)[8] >> 6) & 0x03;
  (pToStruct)->uCIR |=  (uint32_t)  (pFromData)[9] << 2;
  (pToStruct)->uCIR |=  (uint32_t)  (pFromData)[10] << 10;
  (pToStruct)->uCIR |=  (uint32_t)  (pFromData)[11] << 18;
  (pToStruct)->uCIR |=  (uint32_t)  ((pFromData)[12] & 0x3f) << 26;

  /* Unpack Member: uEBS */
  (pToStruct)->uEBS =  (uint32_t)  ((pFromData)[4] >> 6) & 0x03;
  (pToStruct)->uEBS |=  (uint32_t)  (pFromData)[5] << 2;
  (pToStruct)->uEBS |=  (uint32_t)  (pFromData)[6] << 10;
  (pToStruct)->uEBS |=  (uint32_t)  (pFromData)[7] << 18;
  (pToStruct)->uEBS |=  (uint32_t)  ((pFromData)[8] & 0x3f) << 26;

  /* Unpack Member: uEIR */
  (pToStruct)->uEIR =  (uint32_t)  ((pFromData)[0] >> 6) & 0x03;
  (pToStruct)->uEIR |=  (uint32_t)  (pFromData)[1] << 2;
  (pToStruct)->uEIR |=  (uint32_t)  (pFromData)[2] << 10;
  (pToStruct)->uEIR |=  (uint32_t)  (pFromData)[3] << 18;
  (pToStruct)->uEIR |=  (uint32_t)  ((pFromData)[4] & 0x3f) << 26;

  /* Unpack Member: uRsvd */
  (pToStruct)->uRsvd =  (uint32_t)  ((pFromData)[0] >> 5) & 0x01;

  /* Unpack Member: bBlindMode */
  (pToStruct)->bBlindMode =  (uint32_t)  ((pFromData)[0] >> 4) & 0x01;

  /* Unpack Member: bDropOnRed */
  (pToStruct)->bDropOnRed =  (uint32_t)  ((pFromData)[0] >> 3) & 0x01;

  /* Unpack Member: bCoupling */
  (pToStruct)->bCoupling =  (uint32_t)  ((pFromData)[0] >> 2) & 0x01;

  /* Unpack Member: bCBSNoDecrement */
  (pToStruct)->bCBSNoDecrement =  (uint32_t)  ((pFromData)[0] >> 1) & 0x01;

  /* Unpack Member: bEBSNoDecrement */
  (pToStruct)->bEBSNoDecrement =  (uint32_t)  ((pFromData)[0] ) & 0x01;

}



/* initialize an instance of this zframe */
void
sbZfFe2000PmPolicerConfig_InitInstance(sbZfFe2000PmPolicerConfig_t *pFrame) {

  pFrame->uRfcMode =  (unsigned int)  0;
  pFrame->uLenShift =  (unsigned int)  0;
  pFrame->uRate =  (unsigned int)  0;
  pFrame->uCBS =  (unsigned int)  0;
  pFrame->uCIR =  (unsigned int)  0;
  pFrame->uEBS =  (unsigned int)  0;
  pFrame->uEIR =  (unsigned int)  0;
  pFrame->uRsvd =  (unsigned int)  0;
  pFrame->bBlindMode =  (unsigned int)  0;
  pFrame->bDropOnRed =  (unsigned int)  0;
  pFrame->bCoupling =  (unsigned int)  0;
  pFrame->bCBSNoDecrement =  (unsigned int)  0;
  pFrame->bEBSNoDecrement =  (unsigned int)  0;

}
