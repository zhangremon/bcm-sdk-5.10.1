/*
 * $Id: sbZfFe2000PmGroupConfig.c 1.1.34.4 Broadcom SDK $
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
#include "sbZfFe2000PmGroupConfig.hx"
#include "sbWrappers.h"
#include <sal/types.h>



/* Pack from struct into array of bytes */
uint32_t
sbZfFe2000PmGroupConfig_Pack(sbZfFe2000PmGroupConfig_t *pFrom,
                             uint8_t *pToData,
                             uint32_t nMaxToDataIndex) {
  int i;
  int size = SB_ZF_FE2000PMGROUPCONFIG_SIZE_IN_BYTES;

  for ( i=0; i<size; i++ ) {
    (pToData)[i] = 0;
  }
  i = 0;

  /* Pack operation based on little endian */

  /* Pack Member: uEnable */
  (pToData)[15] |= ((pFrom)->uEnable & 0x01) <<7;

  /* Pack Member: uSpare2 */
  (pToData)[14] |= ((pFrom)->uSpare2 & 0x01) <<7;
  (pToData)[15] |= ((pFrom)->uSpare2 >> 1) & 0x7f;

  /* Pack Member: uMinPolicerId */
  (pToData)[12] |= ((pFrom)->uMinPolicerId) & 0xFF;
  (pToData)[13] |= ((pFrom)->uMinPolicerId >> 8) &0xFF;
  (pToData)[14] |= ((pFrom)->uMinPolicerId >> 16) & 0x7f;

  /* Pack Member: uSpare1 */
  (pToData)[10] |= ((pFrom)->uSpare1 & 0x01) <<7;
  (pToData)[11] |= ((pFrom)->uSpare1 >> 1) &0xFF;

  /* Pack Member: uMaxPolicerId */
  (pToData)[8] |= ((pFrom)->uMaxPolicerId) & 0xFF;
  (pToData)[9] |= ((pFrom)->uMaxPolicerId >> 8) &0xFF;
  (pToData)[10] |= ((pFrom)->uMaxPolicerId >> 16) & 0x7f;

  /* Pack Member: uRefreshCount */
  (pToData)[7] |= ((pFrom)->uRefreshCount & 0x0f) <<4;

  /* Pack Member: uRefreshThresh */
  (pToData)[6] |= ((pFrom)->uRefreshThresh) & 0xFF;
  (pToData)[7] |= ((pFrom)->uRefreshThresh >> 8) & 0x0f;

  /* Pack Member: uRefreshPeriod */
  (pToData)[4] |= ((pFrom)->uRefreshPeriod) & 0xFF;
  (pToData)[5] |= ((pFrom)->uRefreshPeriod >> 8) &0xFF;

  /* Pack Member: uSpare0 */
  (pToData)[3] |= ((pFrom)->uSpare0 & 0x01) <<7;

  /* Pack Member: uTimerEnable */
  (pToData)[3] |= ((pFrom)->uTimerEnable & 0x01) <<6;

  /* Pack Member: uTimestampOffset */
  (pToData)[3] |= ((pFrom)->uTimestampOffset & 0x3f);

  /* Pack Member: uTimerTickPeriod */
  (pToData)[0] |= ((pFrom)->uTimerTickPeriod) & 0xFF;
  (pToData)[1] |= ((pFrom)->uTimerTickPeriod >> 8) &0xFF;
  (pToData)[2] |= ((pFrom)->uTimerTickPeriod >> 16) &0xFF;

  return SB_ZF_FE2000PMGROUPCONFIG_SIZE_IN_BYTES;
}




/* Unpack from array of bytes into struct */
void
sbZfFe2000PmGroupConfig_Unpack(sbZfFe2000PmGroupConfig_t *pToStruct,
                               uint8_t *pFromData,
                               uint32_t nMaxToDataIndex) {
  COMPILER_UINT64 tmp;

  (void) tmp;


  /* Unpack operation based on little endian */

  /* Unpack Member: uEnable */
  (pToStruct)->uEnable =  (uint32_t)  ((pFromData)[15] >> 7) & 0x01;

  /* Unpack Member: uSpare2 */
  (pToStruct)->uSpare2 =  (uint32_t)  ((pFromData)[14] >> 7) & 0x01;
  (pToStruct)->uSpare2 |=  (uint32_t)  ((pFromData)[15] & 0x7f) << 1;

  /* Unpack Member: uMinPolicerId */
  (pToStruct)->uMinPolicerId =  (uint32_t)  (pFromData)[12] ;
  (pToStruct)->uMinPolicerId |=  (uint32_t)  (pFromData)[13] << 8;
  (pToStruct)->uMinPolicerId |=  (uint32_t)  ((pFromData)[14] & 0x7f) << 16;

  /* Unpack Member: uSpare1 */
  (pToStruct)->uSpare1 =  (uint32_t)  ((pFromData)[10] >> 7) & 0x01;
  (pToStruct)->uSpare1 |=  (uint32_t)  (pFromData)[11] << 1;

  /* Unpack Member: uMaxPolicerId */
  (pToStruct)->uMaxPolicerId =  (uint32_t)  (pFromData)[8] ;
  (pToStruct)->uMaxPolicerId |=  (uint32_t)  (pFromData)[9] << 8;
  (pToStruct)->uMaxPolicerId |=  (uint32_t)  ((pFromData)[10] & 0x7f) << 16;

  /* Unpack Member: uRefreshCount */
  (pToStruct)->uRefreshCount =  (uint32_t)  ((pFromData)[7] >> 4) & 0x0f;

  /* Unpack Member: uRefreshThresh */
  (pToStruct)->uRefreshThresh =  (uint32_t)  (pFromData)[6] ;
  (pToStruct)->uRefreshThresh |=  (uint32_t)  ((pFromData)[7] & 0x0f) << 8;

  /* Unpack Member: uRefreshPeriod */
  (pToStruct)->uRefreshPeriod =  (uint32_t)  (pFromData)[4] ;
  (pToStruct)->uRefreshPeriod |=  (uint32_t)  (pFromData)[5] << 8;

  /* Unpack Member: uSpare0 */
  (pToStruct)->uSpare0 =  (uint32_t)  ((pFromData)[3] >> 7) & 0x01;

  /* Unpack Member: uTimerEnable */
  (pToStruct)->uTimerEnable =  (uint32_t)  ((pFromData)[3] >> 6) & 0x01;

  /* Unpack Member: uTimestampOffset */
  (pToStruct)->uTimestampOffset =  (uint32_t)  ((pFromData)[3] ) & 0x3f;

  /* Unpack Member: uTimerTickPeriod */
  (pToStruct)->uTimerTickPeriod =  (uint32_t)  (pFromData)[0] ;
  (pToStruct)->uTimerTickPeriod |=  (uint32_t)  (pFromData)[1] << 8;
  (pToStruct)->uTimerTickPeriod |=  (uint32_t)  (pFromData)[2] << 16;

}



/* initialize an instance of this zframe */
void
sbZfFe2000PmGroupConfig_InitInstance(sbZfFe2000PmGroupConfig_t *pFrame) {

  pFrame->uEnable =  (unsigned int)  0;
  pFrame->uSpare2 =  (unsigned int)  0;
  pFrame->uMinPolicerId =  (unsigned int)  0;
  pFrame->uSpare1 =  (unsigned int)  0;
  pFrame->uMaxPolicerId =  (unsigned int)  0;
  pFrame->uRefreshCount =  (unsigned int)  0;
  pFrame->uRefreshThresh =  (unsigned int)  0;
  pFrame->uRefreshPeriod =  (unsigned int)  0;
  pFrame->uSpare0 =  (unsigned int)  0;
  pFrame->uTimerEnable =  (unsigned int)  0;
  pFrame->uTimestampOffset =  (unsigned int)  0;
  pFrame->uTimerTickPeriod =  (unsigned int)  0;

}
