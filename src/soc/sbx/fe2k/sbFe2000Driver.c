/* -*- mode:c++; c-style:k&r; c-basic-offset:2; indent-tabs-mode: nil; -*- */
/* vi:set expandtab cindent shiftwidth=2 cinoptions=\:0l1(0t0g0: */
/*
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
 * $Id: sbFe2000Driver.c 1.8.52.1 Broadcom SDK $
 *
 * sbFe2000Dma.h : FE2000 DMA Engine Driver
 *
 *-----------------------------------------------------------------------------*/

#include "glue.h"
#include "hal_user.h"
#include "sbFe2000Driver.h"
#include "sbWrappers.h"
#include "sbx_drv.h"
#include "hal_ca_c2.h"

typedef enum 
{
  SB_FE2000_RC_PROGRAM0 =0,
  SB_FE2000_RC_PROGRAM1,
  SB_FE2000_RC_PROGRAM2,
  SB_FE2000_RC_PROGRAM3,
  SB_FE2000_RC_PROGRAM4,
  SB_FE2000_RC_PROGRAM5,
  SB_FE2000_RC_PROGRAM6,
  SB_FE2000_RC_PROGRAM7,
  SB_FE2000_RC_PROGRAM8,
  SB_FE2000_RC_PROGRAM9,
  SB_FE2000_RC_PROGRAM10,
  SB_FE2000_RC_PROGRAM11,
  SB_FE2000_RC_PROGRAM12,
  SB_FE2000_RC_PROGRAM13,
  SB_FE2000_RC_PROGRAM14,
  SB_FE2000_RC_PROGRAM15,
} sbFe2000RcProgramId_t;



uint32_t
sbFe2000RcProgramControlSet(sbhandle userDeviceHandle, 
                         uint32_t uRcInstance, 
                         uint32_t uProgramId, 
                         uint32_t bEnable, 
                         uint32_t uLength, 
                         uint32_t uBaseAddress)
{
  uint32_t uProgramConfig = 0;
  uint32_t uAddress  =0;
  uint32_t uMultiportMask = 0xFFFFFFC0; /* Support 6bit port field */

  if (!SAND_HAL_IS_FE2KXT(userDeviceHandle)) {
    /* 
     * The RC Program configuration register at an offset of 4 bytes 
     * from each other. And there are total of 16 of them. All of them
     * use the same register format.
     */
    uAddress = (uRcInstance == 0) ? (SAND_HAL_REG_OFFSET(CA,RC0_CONFIG_PROGRAM0) + (uProgramId*4)) :
                          (SAND_HAL_REG_OFFSET(CA,RC1_CONFIG_PROGRAM0) + (uProgramId*4))  ;
    uProgramConfig = SAND_HAL_MOD_FIELD(CA, RC_CONFIG_PROGRAM0, EN, uProgramConfig, bEnable);
    
    uProgramConfig = SAND_HAL_MOD_FIELD(CA, RC_CONFIG_PROGRAM0, LENGTH, uProgramConfig, uLength);
    uProgramConfig = SAND_HAL_MOD_FIELD(CA, RC_CONFIG_PROGRAM0, BASE_ADDR, uProgramConfig, uBaseAddress);

    SAND_HAL_WRITE_OFFS(userDeviceHandle, uAddress, uProgramConfig);
  }else{
    /* Index is * 3 since two cascade registers are in the way */

    if(uRcInstance == 0) {
      uProgramConfig = SAND_HAL_MOD_FIELD(C2, RC0_CONFIG_PROGRAM0, EN, uProgramConfig, bEnable); 
      uProgramConfig = SAND_HAL_MOD_FIELD(C2, RC0_CONFIG_PROGRAM0, MULTIPORT_MASK, uProgramConfig, uMultiportMask);
      uProgramConfig = SAND_HAL_MOD_FIELD(C2, RC0_CONFIG_PROGRAM0, BASE_ADDR, uProgramConfig, uBaseAddress);
      SAND_HAL_WRITE_INDEX(userDeviceHandle, C2, RC0_CONFIG_PROGRAM0, uProgramId * 3, uProgramConfig);
    } else {
      uProgramConfig = SAND_HAL_MOD_FIELD(C2, RC1_CONFIG_PROGRAM0, EN, uProgramConfig, bEnable); 
      uProgramConfig = SAND_HAL_MOD_FIELD(C2, RC1_CONFIG_PROGRAM0,  MULTIPORT_MASK, uProgramConfig,uMultiportMask); 
      uProgramConfig = SAND_HAL_MOD_FIELD(C2, RC1_CONFIG_PROGRAM0, BASE_ADDR, uProgramConfig, uBaseAddress);
      SAND_HAL_WRITE_INDEX(userDeviceHandle, C2, RC1_CONFIG_PROGRAM0, uProgramId * 3, uProgramConfig);
    }
  }

  return SB_OK;
}

uint32_t
sbFe2000RcProgramControlGet(sbhandle userDeviceHandle, 
                         uint32_t uRcInstance, 
                         uint32_t uProgramId, 
                         uint8_t *bEnable, 
                         uint32_t *uLength, 
                         uint32_t *uBaseAddress)
{
  uint32_t uProgramConfig = 0;
  uint32_t uAddress  =0;

  if (!SAND_HAL_IS_FE2KXT(userDeviceHandle)) {
    /* 
     * The RC Program configuration register at an offset of 4 bytes 
     * from each other. And there are total of 16 of them. All of them
     * use the same register format.
     */
    uAddress = (uRcInstance == 0) ? (SAND_HAL_REG_OFFSET(CA,RC0_CONFIG_PROGRAM0) + (uProgramId*4)) :
                          (SAND_HAL_REG_OFFSET(CA,RC1_CONFIG_PROGRAM0) + (uProgramId*4))  ;
    uProgramConfig = SAND_HAL_READ_OFFS(userDeviceHandle, uAddress);

    *bEnable = SAND_HAL_GET_FIELD(CA, RC_CONFIG_PROGRAM0, EN, uProgramConfig);
    *uLength = SAND_HAL_GET_FIELD(CA, RC_CONFIG_PROGRAM0, LENGTH, uProgramConfig);
    *uBaseAddress = SAND_HAL_GET_FIELD(CA, RC_CONFIG_PROGRAM0, BASE_ADDR, uProgramConfig);
  }else{
    *uLength  = 0;
    /* Index is * 3 since two cascade registers are in the way */
    if(uRcInstance == 0) {
      uProgramConfig =  SAND_HAL_READ_INDEX(userDeviceHandle, C2, RC0_CONFIG_PROGRAM0, uProgramId*3);
      *bEnable = SAND_HAL_GET_FIELD(C2, RC0_CONFIG_PROGRAM0, EN, uProgramConfig); 
      *uBaseAddress = SAND_HAL_GET_FIELD(C2, RC0_CONFIG_PROGRAM0, BASE_ADDR, uProgramConfig);
    } else {
      uProgramConfig =  SAND_HAL_READ_INDEX(userDeviceHandle, C2, RC1_CONFIG_PROGRAM0, uProgramId*3);
      *bEnable = SAND_HAL_GET_FIELD(C2, RC1_CONFIG_PROGRAM0, EN, uProgramConfig); 
      *uBaseAddress = SAND_HAL_GET_FIELD(C2, RC1_CONFIG_PROGRAM0, BASE_ADDR, uProgramConfig);
    }
  }

  return SB_OK;
}

uint32_t
sbFe2000RcMapLogicaltoPhysicalProgram(sbhandle userDeviceHandle, 
                                      uint32_t uBatchGroup, 
                                      uint32_t uLogicalProgramNumber,
                                      uint32_t uPhysicalProgramNumber)
{

  uint32_t uProgram=0;

  if(uLogicalProgramNumber == 0 ) {
    uProgram = SAND_HAL_FE2000_READ(userDeviceHandle, LU_RCE_PROGRAM0);
    uProgram &= ~(0xF << uBatchGroup*4);
    uProgram |= (uPhysicalProgramNumber << uBatchGroup*4);
    SAND_HAL_FE2000_WRITE(userDeviceHandle,  LU_RCE_PROGRAM0, uProgram);
  } else if(uLogicalProgramNumber == 1 ) {
    uProgram = SAND_HAL_FE2000_READ(userDeviceHandle, LU_RCE_PROGRAM1);
    uProgram &= ~(0xF << uBatchGroup*4);
    uProgram |= (uPhysicalProgramNumber << uBatchGroup*4);
    SAND_HAL_FE2000_WRITE(userDeviceHandle,  LU_RCE_PROGRAM1, uProgram);
  } else if(uLogicalProgramNumber == 2 ) {
    uProgram = SAND_HAL_FE2000_READ(userDeviceHandle, LU_RCE_PROGRAM2);
    uProgram &= ~(0xF << uBatchGroup*4);
    uProgram |= (uPhysicalProgramNumber << uBatchGroup*4);
    SAND_HAL_FE2000_WRITE(userDeviceHandle,  LU_RCE_PROGRAM2, uProgram);
  } else if(uLogicalProgramNumber == 3 ) {
    uProgram = SAND_HAL_FE2000_READ(userDeviceHandle, LU_RCE_PROGRAM3);
    uProgram &= ~(0xF << uBatchGroup*4);
    uProgram |= (uPhysicalProgramNumber << uBatchGroup*4);
    SAND_HAL_FE2000_WRITE(userDeviceHandle,  LU_RCE_PROGRAM3, uProgram);
  } else if(uLogicalProgramNumber == 4 ) {
    uProgram = SAND_HAL_FE2000_READ(userDeviceHandle, LU_RCE_PROGRAM4);
    uProgram &= ~(0xF << uBatchGroup*4);
    uProgram |= (uPhysicalProgramNumber << uBatchGroup*4);
    SAND_HAL_FE2000_WRITE(userDeviceHandle,  LU_RCE_PROGRAM4, uProgram);
  } else if(uLogicalProgramNumber == 5 ) {
    uProgram = SAND_HAL_FE2000_READ(userDeviceHandle, LU_RCE_PROGRAM5);
    uProgram &= ~(0xF << uBatchGroup*4);
    uProgram |= (uPhysicalProgramNumber << uBatchGroup*4);
    SAND_HAL_FE2000_WRITE(userDeviceHandle,  LU_RCE_PROGRAM5, uProgram);
  } else if(uLogicalProgramNumber == 6 ) {
    uProgram = SAND_HAL_FE2000_READ(userDeviceHandle, LU_RCE_PROGRAM6);
    uProgram &= ~(0xF << uBatchGroup*4);
    uProgram |= (uPhysicalProgramNumber << uBatchGroup*4);
    SAND_HAL_FE2000_WRITE(userDeviceHandle,  LU_RCE_PROGRAM6, uProgram);
  } else if(uLogicalProgramNumber == 7 ) {
    uProgram = SAND_HAL_FE2000_READ(userDeviceHandle, LU_RCE_PROGRAM7);
    uProgram &= ~(0xF << uBatchGroup*4);
    uProgram |= (uPhysicalProgramNumber << uBatchGroup*4);
    SAND_HAL_FE2000_WRITE(userDeviceHandle,  LU_RCE_PROGRAM7, uProgram);
  } else {
    /* error */
  }
  return SB_OK;
}
