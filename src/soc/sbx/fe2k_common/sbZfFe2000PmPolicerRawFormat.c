/*
 * $Id: sbZfFe2000PmPolicerRawFormat.c 1.1.34.4 Broadcom SDK $
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
#include "sbZfFe2000PmPolicerRawFormat.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfFe2000PmPolicerRawFormat_Pack(sbZfFe2000PmPolicerRawFormat_t *pFrom,
                                  uint8_t *pToData,
                                  uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_FE_2000_PM_POLICERRAWFORMAT_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: uProfileId */
  (pToData)[6] |= ((pFrom)->uProfileId & 0x07) <<5;
  (pToData)[7] |= ((pFrom)->uProfileId >> 3) &0xFF;

  /* Pack Member: uMeterState */
  (pToData)[0] |= ((pFrom)->uMeterState) & 0xFF;
  (pToData)[1] |= ((pFrom)->uMeterState >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->uMeterState >> 16) &0xFF;
  (pToData)[3] |= ((pFrom)->uMeterState >> 24) &0xFF;
  (pToData)[4] |= ((pFrom)->uMeterState >> 32) &0xFF;
  (pToData)[5] |= ((pFrom)->uMeterState >> 40) &0xFF;
  (pToData)[6] |= ((pFrom)->uMeterState >> 48) & 0x1f;

  return SB_ZF_FE_2000_PM_POLICERRAWFORMAT_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFe2000PmPolicerRawFormat_Unpack(sbZfFe2000PmPolicerRawFormat_t *pToStruct,
                                    uint8_t *pFromData,
                                    uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: uProfileId */
  (pToStruct)->uProfileId =  (uint32_t)  ((pFromData)[6] >> 5) & 0x07;
  (pToStruct)->uProfileId |=  (uint32_t)  (pFromData)[7] << 3;

  /* Unpack Member: uMeterState */
  COMPILER_64_SET((pToStruct)->uMeterState, 0,  (unsigned int) (pFromData)[0]);
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->uMeterState;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[1]);
    COMPILER_64_SHL(tmp, 8);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->uMeterState;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[2]);
    COMPILER_64_SHL(tmp, 16);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->uMeterState;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[3]);
    COMPILER_64_SHL(tmp, 24);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->uMeterState;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[4]);
    COMPILER_64_SHL(tmp, 32);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->uMeterState;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[5]);
    COMPILER_64_SHL(tmp, 40);
    COMPILER_64_OR(*tmp0, tmp);
  };
  {
    VOL COMPILER_UINT64 *tmp0 = &(pToStruct)->uMeterState;
    COMPILER_64_SET(tmp, 0,  (unsigned int) (pFromData)[6]);
    COMPILER_64_SHL(tmp, 48);
    COMPILER_64_OR(*tmp0, tmp);
  };

}



/* initialize an instance of this zframe */
void
sbZfFe2000PmPolicerRawFormat_InitInstance(sbZfFe2000PmPolicerRawFormat_t *pFrame) {

  pFrame->uProfileId =  (unsigned int)  0;
  pFrame->uMeterState =  (uint64_t)  0;

}
