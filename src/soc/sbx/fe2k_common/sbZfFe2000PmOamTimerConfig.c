/*
 * $Id: sbZfFe2000PmOamTimerConfig.c 1.1.34.4 Broadcom SDK $
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
#include "sbZfFe2000PmOamTimerConfig.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfFe2000PmOamTimerConfig_Pack(sbZfFe2000PmOamTimerConfig_t *pFrom,
                                uint8_t *pToData,
                                uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_FE_2000_PM_OAMTIMERCFG_SIZE_IN_BYTES;

  if (size % 4) {
    size += (4 - size %4);
  }

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: uRate */
  (pToData)[4] |= ((pFrom)->uRate & 0x0f) <<4;

  /* Pack Member: uDeadline */
  (pToData)[0] |= ((pFrom)->uDeadline & 0x0f) <<4;
  (pToData)[1] |= ((pFrom)->uDeadline >> 4) &0xFF;
  (pToData)[2] |= ((pFrom)->uDeadline >> 12) &0xFF;
  (pToData)[3] |= ((pFrom)->uDeadline >> 20) &0xFF;
  (pToData)[4] |= ((pFrom)->uDeadline >> 28) & 0x0f;

  /* Pack Member: bStarted */
  (pToData)[0] |= ((pFrom)->bStarted & 0x01) <<3;

  /* Pack Member: bStrict */
  (pToData)[0] |= ((pFrom)->bStrict & 0x01) <<2;

  /* Pack Member: bReset */
  (pToData)[0] |= ((pFrom)->bReset & 0x01) <<1;

  /* Pack Member: bInterrupt */
  (pToData)[0] |= ((pFrom)->bInterrupt & 0x01);

  return SB_ZF_FE_2000_PM_OAMTIMERCFG_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFe2000PmOamTimerConfig_Unpack(sbZfFe2000PmOamTimerConfig_t *pToStruct,
                                  uint8_t *pFromData,
                                  uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: uRate */
  (pToStruct)->uRate =  (uint32_t)  ((pFromData)[4] >> 4) & 0x0f;

  /* Unpack Member: uDeadline */
  (pToStruct)->uDeadline =  (uint32_t)  ((pFromData)[0] >> 4) & 0x0f;
  (pToStruct)->uDeadline |=  (uint32_t)  (pFromData)[1] << 4;
  (pToStruct)->uDeadline |=  (uint32_t)  (pFromData)[2] << 12;
  (pToStruct)->uDeadline |=  (uint32_t)  (pFromData)[3] << 20;
  (pToStruct)->uDeadline |=  (uint32_t)  ((pFromData)[4] & 0x0f) << 28;

  /* Unpack Member: bStarted */
  (pToStruct)->bStarted =  (uint32_t)  ((pFromData)[0] >> 3) & 0x01;

  /* Unpack Member: bStrict */
  (pToStruct)->bStrict =  (uint32_t)  ((pFromData)[0] >> 2) & 0x01;

  /* Unpack Member: bReset */
  (pToStruct)->bReset =  (uint32_t)  ((pFromData)[0] >> 1) & 0x01;

  /* Unpack Member: bInterrupt */
  (pToStruct)->bInterrupt =  (uint32_t)  ((pFromData)[0] ) & 0x01;

}



/* initialize an instance of this zframe */
void
sbZfFe2000PmOamTimerConfig_InitInstance(sbZfFe2000PmOamTimerConfig_t *pFrame) {

  pFrame->uRate =  (unsigned int)  0;
  pFrame->uDeadline =  (unsigned int)  0;
  pFrame->bStarted =  (unsigned int)  0;
  pFrame->bStrict =  (unsigned int)  0;
  pFrame->bReset =  (unsigned int)  0;
  pFrame->bInterrupt =  (unsigned int)  0;

}
