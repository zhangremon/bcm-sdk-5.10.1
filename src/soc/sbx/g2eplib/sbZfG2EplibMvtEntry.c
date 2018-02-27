/*
 * $Id: sbZfG2EplibMvtEntry.c 1.5.12.4 Broadcom SDK $
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
#include "sbZfG2EplibMvtEntry.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfG2EplibMvtEntry_Pack(sbZfG2EplibMvtEntry_t *pFrom,
                         uint8_t *pToData,
                         uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_G2_EPLIB_MVTENTRY_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on big endian */

  /* Pack Member: ullPortMask */
  (pToData)[0] |= ((pFrom)->ullPortMask & 0x01);

  /* Pack Member: nType */
  (pToData)[0] |= ((pFrom)->nType & 0x01);

  /* Pack Member: ulMvtdA */
  (pToData)[0] |= ((pFrom)->ulMvtdA & 0x01);

  /* Pack Member: ulMvtdB */
  (pToData)[0] |= ((pFrom)->ulMvtdB & 0x01);

  /* Pack Member: bSourceKnockout */
  (pToData)[0] |= ((pFrom)->bSourceKnockout & 0x01);

  /* Pack Member: bEnableChaining */
  (pToData)[0] |= ((pFrom)->bEnableChaining & 0x01);

  /* Pack Member: ulNextMcGroup */
  (pToData)[0] |= ((pFrom)->ulNextMcGroup & 0x01);

  return SB_ZF_G2_EPLIB_MVTENTRY_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfG2EplibMvtEntry_Unpack(sbZfG2EplibMvtEntry_t *pToStruct,
                           uint8_t *pFromData,
                           uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on big endian */

  /* Unpack Member: ullPortMask */
  COMPILER_64_SET((pToStruct)->ullPortMask, 0,  (unsigned int) (pFromData)[0]);

  /* Unpack Member: nType */
  (pToStruct)->nType =  (uint32_t)  ((pFromData)[0] ) & 0x01;

  /* Unpack Member: ulMvtdA */
  (pToStruct)->ulMvtdA =  (uint32_t)  ((pFromData)[0] ) & 0x01;

  /* Unpack Member: ulMvtdB */
  (pToStruct)->ulMvtdB =  (uint32_t)  ((pFromData)[0] ) & 0x01;

  /* Unpack Member: bSourceKnockout */
  (pToStruct)->bSourceKnockout =  (uint8_t)  ((pFromData)[0] ) & 0x01;

  /* Unpack Member: bEnableChaining */
  (pToStruct)->bEnableChaining =  (uint8_t)  ((pFromData)[0] ) & 0x01;

  /* Unpack Member: ulNextMcGroup */
  (pToStruct)->ulNextMcGroup =  (uint32_t)  ((pFromData)[0] ) & 0x01;

}



/* initialize an instance of this zframe */
void
sbZfG2EplibMvtEntry_InitInstance(sbZfG2EplibMvtEntry_t *pFrame) {

  pFrame->ullPortMask =  (unsigned int)  0;
  pFrame->nType =  (unsigned int)  0;
  pFrame->ulMvtdA =  (unsigned int)  0;
  pFrame->ulMvtdB =  (unsigned int)  0;
  pFrame->bSourceKnockout =  (unsigned int)  0;
  pFrame->bEnableChaining =  (unsigned int)  0;
  pFrame->ulNextMcGroup =  (unsigned int)  0;

}
