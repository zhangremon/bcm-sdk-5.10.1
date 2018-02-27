/* -*-  Mode:C; c-basic-offset:4 -*- */
/******************************************************************************
 *
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
 *
 * $Id: sbFe2000Util.c 1.6.214.1 Broadcom SDK $
 *
 ******************************************************************************/

#include "sbTypes.h"
#include "sbWrappers.h"
#include "glue.h"
#include <soc/drv.h>
#include "hal_user.h"
#include "hal_ca_auto.h"
#include "sbFe2000Util.h"

uint8_t
sbFe2000UtilPvtCompensation(sbhandle userDeviceHandle, uint32_t uMmInstance)
{

  uint32_t uData;
  uint8_t bError = FALSE;
  uint32_t uDone;
  uint32_t uRetry = 100;
  uint8_t bRevA0Chip;
  uint32_t uScomp_control;

  DENTER();

  uData = SAND_HAL_READ(userDeviceHandle, CA, PC_REVISION);
  bRevA0Chip = SAND_HAL_GET_FIELD(CA, PC_REVISION, REVISION, uData)==0;

  SB_LOGV1("Calibrating MM%d PVT Compensation\n",uMmInstance);
  /* dwl - Apr 12 2007 - Enable PVT compensation logic, use default sequence
   *                   - (1) reset controller
   *                   - (2) setups up compensation paramters
   *                   - (3) Enable compensation
   *                   - (4) Wait for completion and check for error
   */

  SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,CA,MM,uMmInstance
                            ,MM_PVT_CONTROL0,PVT_RESETB,0);
  SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,CA,MM,uMmInstance,
                            MM_PVT_CONTROL1,PVT_COMP_EN,0);
  SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,CA,MM,uMmInstance,
                            MM_PVT_CONTROL0,PVT_RESETB,1);

  uData = SAND_HAL_READ_STRIDE(userDeviceHandle,CA,MM,uMmInstance,
                               MM_PVT_CONTROL2 );
  uData = SAND_HAL_MOD_FIELD(CA, MM_PVT_CONTROL2, PVT_OVERRIDE_EN,
                             uData, 0);
  uData = SAND_HAL_MOD_FIELD(CA, MM_PVT_CONTROL2, PVT_RT75, uData, 1);

  /* dwl - Jul 26 2007 - The default value of PVT_SCOMP_CONTROL for A0 part is 7
   *                   - For rev A1 part, the MSB of the register value is inverted
   *                   - such that the default of 4'b0000(0) is effectively 4'b1000(8).
   *                   - To setup PVT_SCOMP_CONTROL to 7, the value of 4'b1111 (15)
   *                   - should be used.
   */

  uScomp_control = bRevA0Chip ? 0x7 : 0xf;
  uData = SAND_HAL_MOD_FIELD(CA, MM_PVT_CONTROL2, PVT_SCOMP_CONTROL, uData, uScomp_control);
  SAND_HAL_WRITE_STRIDE(userDeviceHandle,CA,MM,uMmInstance,MM_PVT_CONTROL2,
                        uData);
  
  /* enable power_save, no offset is used */
  uData = SAND_HAL_READ_STRIDE(userDeviceHandle,CA,MM,uMmInstance,
                               MM_PVT_CONTROL3);
  uData = SAND_HAL_MOD_FIELD(CA, MM_PVT_CONTROL3, PVT_POWER_SAVE_EN, uData, 1);
  uData = SAND_HAL_MOD_FIELD(CA, MM_PVT_CONTROL3, PVT_COMP_OFFSET_EN, uData,
                             0);
  SAND_HAL_WRITE_STRIDE(userDeviceHandle,CA,MM,uMmInstance,MM_PVT_CONTROL3,
                        uData);
  
  SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,CA,MM,uMmInstance,
                            MM_PVT_CONTROL1,PVT_COMP_EN,1);
  
  uData = SAND_HAL_READ_STRIDE(userDeviceHandle,CA,MM,uMmInstance,
                               MM_PVT_STATUS);
  uDone = SAND_HAL_GET_FIELD(CA, MM_PVT_STATUS, PVT_COMP_DONE, uData);
  uRetry = 100;
  while ( uRetry && !uDone ) {
    uData = SAND_HAL_READ_STRIDE(userDeviceHandle,CA,MM,uMmInstance,
                                 MM_PVT_STATUS);
    uDone = SAND_HAL_GET_FIELD(CA, MM_PVT_STATUS, PVT_COMP_DONE, uData) ||
      SAND_HAL_GET_FIELD(CA, MM_PVT_STATUS, PVT_COMP_ERROR, uData);
    uRetry--;
  }
  
  if ((uRetry == 0) && !uDone) {
    SB_LOGV1("MM%d PVT compensation timeout\n", uMmInstance);
    bError = TRUE;
  }
  
  if (SAND_HAL_GET_FIELD(CA, MM_PVT_STATUS, PVT_COMP_ERROR, uData)) {
    SB_LOGV1("MM%d PVT compensation error, MM_PVT_STATUS=0x%x\n", uMmInstance,
           uData);
    uData = SAND_HAL_GET_FIELD(CA, MM_PVT_STATUS, PVT_COMP_ERROR, uData);
    if (uData & 0x1)
      SB_LOGV1( " Filter expired during PMOS compensation\n");
    if (uData & 0x2)
      SB_LOGV1( " Filter expired during NMOS compensation\n");
    if (uData & 0x4)
      SB_LOGV1( " Filter expired during ODT resistor compensation\n");
    if (uData & 0x8)
      SB_LOGV1( " P-code N-code invalid, their delta exceed limits\n");
    if (uData & 0x10)
      SB_LOGV1( " Watchdog timer expired\n");
  }
  
  if (SAND_HAL_GET_FIELD(CA, MM_PVT_STATUS, PVT_COMP_DONE, uData)) {
    SB_LOGV1("MM%d PVT compensation done\n", uMmInstance);
    SB_LOGV1("MM%d PVT compensation P,N,R code = {%d, %d, %d}\n", 
	       uMmInstance,
	       SAND_HAL_GET_FIELD(CA, MM_PVT_STATUS, PVT_PCOMP_CODE_2CORE,
                                  uData),
	       SAND_HAL_GET_FIELD(CA, MM_PVT_STATUS, PVT_NCOMP_CODE_2CORE,
                                  uData),
	       SAND_HAL_GET_FIELD(CA, MM_PVT_STATUS, PVT_RCOMP_CODE_2CORE,
                                  uData)
	       );
    SB_LOGV1("MM%d PVT compensation P,N,R comparator outputs = {%d, %d, %d}\n",
	       uMmInstance,
	       SAND_HAL_GET_FIELD(CA, MM_PVT_STATUS, PVT_PCMPR_VAL_2CORE,
                                  uData),
	       SAND_HAL_GET_FIELD(CA, MM_PVT_STATUS, PVT_NCMPR_VAL_2CORE,
                                  uData),
	       SAND_HAL_GET_FIELD(CA, MM_PVT_STATUS, PVT_RCMPR_VAL_2CORE,
                                  uData)
	       );
    if (SAND_HAL_GET_FIELD(CA, MM_PVT_STATUS, PVT_PCOMP_CODE_2CORE, uData)==0 ||
	SAND_HAL_GET_FIELD(CA, MM_PVT_STATUS, PVT_PCOMP_CODE_2CORE, uData)==15)
      SB_LOGV1("Caution...:  MM%d PVT PCOMP_CODE is at extreme value of %d\n",
		 uMmInstance,
		 SAND_HAL_GET_FIELD(CA, MM_PVT_STATUS, PVT_PCOMP_CODE_2CORE, uData));

    if (SAND_HAL_GET_FIELD(CA, MM_PVT_STATUS, PVT_NCOMP_CODE_2CORE, uData)==0 ||
	SAND_HAL_GET_FIELD(CA, MM_PVT_STATUS, PVT_NCOMP_CODE_2CORE, uData)==15)
      SB_LOGV1("Caution...:  MM%d PVT NCOMP_CODE is at extreme value of %d\n",
		 uMmInstance,
		 SAND_HAL_GET_FIELD(CA, MM_PVT_STATUS, PVT_NCOMP_CODE_2CORE, uData));

    if (SAND_HAL_GET_FIELD(CA, MM_PVT_STATUS, PVT_RCOMP_CODE_2CORE, uData)==0 ||
	SAND_HAL_GET_FIELD(CA, MM_PVT_STATUS, PVT_RCOMP_CODE_2CORE, uData)==15)
      SB_LOGV1("Caution...:  MM%d PVT RCOMP_CODE is at extreme value of %d\n",
		 uMmInstance,
		 SAND_HAL_GET_FIELD(CA, MM_PVT_STATUS, PVT_RCOMP_CODE_2CORE, uData));
      
  }
  SAND_HAL_RMW_FIELD_STRIDE(userDeviceHandle,CA,MM,uMmInstance,MM_PVT_CONTROL1,PVT_COMP_EN,0);
  DEXIT();
  return bError;
}
