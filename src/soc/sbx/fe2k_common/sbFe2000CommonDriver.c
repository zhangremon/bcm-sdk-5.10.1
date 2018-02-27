/*****************************************************************************
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
 * $Id: sbFe2000CommonDriver.c 1.5.112.1 Broadcom SDK $
 * ******************************************************************************/
#include "sbFe2000CommonDriver.h"
#include "hal_ca_auto.h"
#include "hal_c2_auto.h"
#include "hal_ca_c2.h"
#include "sbx_drv.h"

#define SB_FE2000_PM_INDIR_TIMEOUT           100
#define SB_FE2000_PP_INDIR_TIMEOUT           100

uint32_t
sbFe2000PmPolicerSuppressControl(sbhandle userDeviceHandle,
                                 uint32_t uPolicerId,
                                 uint8_t bEnable)
{
  uint32_t uValue=0;
  uValue = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PM_SUPPRESS_WRITE_CONFIG, POLID, uValue, uPolicerId);
  uValue = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PM_SUPPRESS_WRITE_CONFIG, SUPPRESS, uValue, bEnable ? 1 : 0);
  SAND_HAL_FE2000_WRITE(userDeviceHandle, PM_SUPPRESS_WRITE_CONFIG, uValue);
  return SB_OK;
}


uint32_t 
sbFe2000PmGroupConfigSet(sbhandle userDeviceHandle,
                         uint32_t uGroupId, 
                         sbZfFe2000PmGroupConfig_t *pConfig)
{
  uint32_t uConfig = 0;
  uint32_t uAddress =0;

  uAddress = SAND_HAL_FE2000_REG_OFFSET(userDeviceHandle,PM_GROUP0_CONFIG0) + uGroupId*16;
  uConfig = SAND_HAL_FE2000_READ_OFFS(userDeviceHandle, uAddress);

  /* First disable the group before touching anything */
  uConfig = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PM_GROUP0_CONFIG0, ENABLE, uConfig, 0);
  SAND_HAL_FE2000_WRITE_OFFS(userDeviceHandle, uAddress, uConfig);

  /* configure the min policer id */
  uConfig = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PM_GROUP0_CONFIG0, POLID_MIN, uConfig, pConfig->uMinPolicerId);
  SAND_HAL_FE2000_WRITE_OFFS(userDeviceHandle, uAddress, uConfig);

  uAddress += 4;
  /* configure the max policer id */
  uConfig = SAND_HAL_FE2000_READ_OFFS(userDeviceHandle, uAddress);
  uConfig = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PM_GROUP0_CONFIG1,POLID_MAX, uConfig, pConfig->uMaxPolicerId);
  SAND_HAL_FE2000_WRITE_OFFS(userDeviceHandle, uAddress, uConfig);

  /* configure the refresh counts and periods */
  uAddress += 4;
  uConfig = SAND_HAL_FE2000_READ_OFFS(userDeviceHandle, uAddress);
  uConfig = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PM_GROUP0_CONFIG2,REFRESH_COUNT, uConfig,pConfig->uRefreshCount);
  uConfig = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PM_GROUP0_CONFIG2,REFRESH_THRESH, uConfig,pConfig->uRefreshThresh);
  uConfig = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PM_GROUP0_CONFIG2,REFRESH_PERIOD, uConfig,pConfig->uRefreshPeriod);
  SAND_HAL_FE2000_WRITE_OFFS(userDeviceHandle, uAddress, uConfig);

  /* configure the timer tick period */
  uAddress += 4;
  uConfig = SAND_HAL_FE2000_READ_OFFS(userDeviceHandle, uAddress);
  uConfig = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PM_GROUP0_CONFIG3, TIMER_ENABLE, uConfig, pConfig->uTimerEnable ? 1 : 0);
  uConfig = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PM_GROUP0_CONFIG3, TIMESTAMP_OFFSET, uConfig, pConfig->uTimestampOffset);
  uConfig = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PM_GROUP0_CONFIG3, TIMER_TICK_PERIOD, uConfig, pConfig->uTimerTickPeriod);
  SAND_HAL_FE2000_WRITE_OFFS(userDeviceHandle, uAddress, uConfig);

  if(pConfig->uEnable) {
    uAddress = SAND_HAL_FE2000_REG_OFFSET(userDeviceHandle,PM_GROUP0_CONFIG0) + uGroupId*16;
    uConfig = SAND_HAL_FE2000_READ_OFFS(userDeviceHandle, uAddress);
    uConfig = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PM_GROUP0_CONFIG0, ENABLE, uConfig, 1);
    SAND_HAL_FE2000_WRITE_OFFS(userDeviceHandle, uAddress, uConfig);
  }
  return SB_OK;
}

uint32_t 
sbFe2000PmGroupConfigGet(sbhandle userHandle,
                         uint32_t uGroup, 
                         sbZfFe2000PmGroupConfig_t *pGroupConfig)
{
  uint32_t uConfig = 0;
  uint32_t uAddress =0;

  uAddress = SAND_HAL_FE2000_REG_OFFSET(userHandle,PM_GROUP0_CONFIG0) + uGroup*16;

  /* configure the min policer id */
  uConfig = SAND_HAL_FE2000_READ_OFFS(userHandle, uAddress);
  pGroupConfig->uMinPolicerId = SAND_HAL_FE2000_GET_FIELD(userHandle, PM_GROUP0_CONFIG0, POLID_MIN, uConfig);

  uAddress += 4;
  /* configure the max policer id */
  uConfig = SAND_HAL_FE2000_READ_OFFS(userHandle, uAddress);
  pGroupConfig->uMaxPolicerId = SAND_HAL_FE2000_GET_FIELD(userHandle, PM_GROUP0_CONFIG1,POLID_MAX, uConfig);

  /* configure the refresh counts and periods */
  uAddress += 4;
  uConfig = SAND_HAL_FE2000_READ_OFFS(userHandle, uAddress);
  pGroupConfig->uRefreshCount = SAND_HAL_FE2000_GET_FIELD(userHandle, PM_GROUP0_CONFIG2,REFRESH_COUNT, uConfig);
  pGroupConfig->uRefreshThresh = SAND_HAL_FE2000_GET_FIELD(userHandle, PM_GROUP0_CONFIG2,REFRESH_THRESH, uConfig);
  pGroupConfig->uRefreshPeriod = SAND_HAL_FE2000_GET_FIELD(userHandle, PM_GROUP0_CONFIG2,REFRESH_PERIOD, uConfig);

  /* configure the timer tick period */
  uAddress += 4;
  uConfig = SAND_HAL_FE2000_READ_OFFS(userHandle, uAddress);
  pGroupConfig->uTimerEnable = SAND_HAL_FE2000_GET_FIELD(userHandle, PM_GROUP0_CONFIG3, TIMER_ENABLE, uConfig);
  pGroupConfig->uTimestampOffset = SAND_HAL_FE2000_GET_FIELD(userHandle, PM_GROUP0_CONFIG3, TIMESTAMP_OFFSET, uConfig);
  pGroupConfig->uTimerTickPeriod = SAND_HAL_FE2000_GET_FIELD(userHandle, PM_GROUP0_CONFIG3, TIMER_TICK_PERIOD, uConfig);

  return SB_OK;
}

uint32_t 
sbFe2000PmProfileMemoryWrite(sbhandle userDeviceHandle, uint32_t uProfileId, uint32_t *puData)
{
  uint32_t uTemp =0;
  uint32_t bWriteDone = 0;
  uint32_t uTimeout =0;
  uint32_t uCtrl =0;


  SAND_HAL_FE2000_WRITE(userDeviceHandle,  PM_PROFILE_MEM_ACC_DATA0, puData[0]);
  SAND_HAL_FE2000_WRITE(userDeviceHandle,  PM_PROFILE_MEM_ACC_DATA1, puData[1]);
  if (SAND_HAL_IS_FE2KXT(userDeviceHandle)) {
    SAND_HAL_WRITE(userDeviceHandle,  C2, PM_PROFILE_MEM_ACC_DATA2, puData[2]);
  }

  uCtrl = (SAND_HAL_FE2000_SET_FIELD(userDeviceHandle, PM_PROFILE_MEM_ACC_CTRL,REQ, 1) |
           SAND_HAL_FE2000_SET_FIELD(userDeviceHandle, PM_PROFILE_MEM_ACC_CTRL,ACK, 1) |
           SAND_HAL_FE2000_SET_FIELD(userDeviceHandle, PM_PROFILE_MEM_ACC_CTRL,RD_WR_N, 0) |
           SAND_HAL_FE2000_SET_FIELD(userDeviceHandle, PM_PROFILE_MEM_ACC_CTRL,ADDRESS, uProfileId) );
  SAND_HAL_FE2000_WRITE(userDeviceHandle,  PM_PROFILE_MEM_ACC_CTRL, uCtrl);

  bWriteDone = FALSE;
  uTimeout = 0;
  while(!bWriteDone) {
    uTemp = SAND_HAL_FE2000_READ(userDeviceHandle,  PM_PROFILE_MEM_ACC_CTRL);
    if(SAND_HAL_FE2000_GET_FIELD(userDeviceHandle,PM_PROFILE_MEM_ACC_CTRL,ACK, uTemp) == 1) {
      bWriteDone = TRUE;
    }
    uTimeout++;
    if(uTimeout > SB_FE2000_PM_INDIR_TIMEOUT) {
      break;
    }
  }

  if(!bWriteDone)  {
    return SB_TIMEOUT_ERR_CODE;
  }
  return SB_OK;
}

uint32_t 
sbFe2000PmProfileMemoryRead(sbhandle userDeviceHandle, uint32_t uProfileId, uint32_t *puData)
{
  uint32_t uTemp =0;
  uint32_t bWriteDone = 0;
  uint32_t uTimeout =0;
  uint32_t uCtrl =0;
  SOC_SBX_WARM_BOOT_DECLARE(int wb);

  /* disable warmboot register bypass for indirect memory reads */
  SOC_SBX_WARM_BOOT_IGNORE((int)userDeviceHandle, wb);
  
  uCtrl = (SAND_HAL_FE2000_SET_FIELD(userDeviceHandle, PM_PROFILE_MEM_ACC_CTRL,REQ, 1) |
           SAND_HAL_FE2000_SET_FIELD(userDeviceHandle, PM_PROFILE_MEM_ACC_CTRL,ACK, 1) |
           SAND_HAL_FE2000_SET_FIELD(userDeviceHandle, PM_PROFILE_MEM_ACC_CTRL,RD_WR_N, 1) |
           SAND_HAL_FE2000_SET_FIELD(userDeviceHandle, PM_PROFILE_MEM_ACC_CTRL,ADDRESS, uProfileId) );
  SAND_HAL_FE2000_WRITE(userDeviceHandle,  PM_PROFILE_MEM_ACC_CTRL, uCtrl);

  bWriteDone = FALSE;
  uTimeout = 0;
  while(!bWriteDone) {
    uTemp = SAND_HAL_FE2000_READ(userDeviceHandle,  PM_PROFILE_MEM_ACC_CTRL);
    if(SAND_HAL_FE2000_GET_FIELD(userDeviceHandle,PM_PROFILE_MEM_ACC_CTRL,ACK, uTemp) == 1) {
      bWriteDone = TRUE;
    }
    uTimeout++;
    if(uTimeout > SB_FE2000_PM_INDIR_TIMEOUT) {
      break;
    }
  }

  SOC_SBX_WARM_BOOT_OBSERVE((int)userDeviceHandle, wb);

  if(!bWriteDone)  {
    return SB_TIMEOUT_ERR_CODE;
  }

  puData[0] = SAND_HAL_FE2000_READ(userDeviceHandle,  PM_PROFILE_MEM_ACC_DATA0);
  puData[1] = SAND_HAL_FE2000_READ(userDeviceHandle,  PM_PROFILE_MEM_ACC_DATA1);
  if (SAND_HAL_IS_FE2KXT(userDeviceHandle)) {
    puData[2] = SAND_HAL_READ(userDeviceHandle,  C2, PM_PROFILE_MEM_ACC_DATA2);
  }

  return SB_OK;
}

uint32_t 
sbFe2000ByteHashConfigAccess(sbhandle userDeviceHandle, uint32_t uTemplateId, uint32_t uRead)
{
    uint32_t uCtrl, uTimeout, bAccessDone, uTemp;
    
    uCtrl = (SAND_HAL_FE2000_SET_FIELD(userDeviceHandle, PP_HASH_BYTE_TEMPLATE_MEM_ACC_CTRL,REQ, 1) |
             SAND_HAL_FE2000_SET_FIELD(userDeviceHandle, PP_HASH_BYTE_TEMPLATE_MEM_ACC_CTRL,ACK, 1) |
             SAND_HAL_FE2000_SET_FIELD(userDeviceHandle, PP_HASH_BYTE_TEMPLATE_MEM_ACC_CTRL,RD_WR_N, uRead) |
             SAND_HAL_FE2000_SET_FIELD(userDeviceHandle, PP_HASH_BYTE_TEMPLATE_MEM_ACC_CTRL,ADDRESS, uTemplateId) );
    
    SAND_HAL_FE2000_WRITE(userDeviceHandle,  PP_HASH_BYTE_TEMPLATE_MEM_ACC_CTRL, uCtrl);

    bAccessDone = FALSE;
    uTimeout = 0;

    while (!bAccessDone) {
        uTemp = SAND_HAL_FE2000_READ(userDeviceHandle,  PP_HASH_BYTE_TEMPLATE_MEM_ACC_CTRL);
        if(SAND_HAL_FE2000_GET_FIELD(userDeviceHandle, PP_HASH_BYTE_TEMPLATE_MEM_ACC_CTRL, ACK, uTemp) == 1) {
            bAccessDone = TRUE;
        }

        uTimeout++;
        if(uTimeout > SB_FE2000_PP_INDIR_TIMEOUT) {
            break;
        }
    }

    if(!bAccessDone)  {
        return SB_TIMEOUT_ERR_CODE;
    }
    return SB_OK;
}


uint32_t 
sbFe2000ByteHashConfigWrite(sbhandle userDeviceHandle, uint32_t uTemplateId, uint32_t *puData)
{
    SAND_HAL_FE2000_WRITE(userDeviceHandle, PP_HASH_BYTE_TEMPLATE_MEM_ACC_DATA1, puData[0]);
    SAND_HAL_FE2000_WRITE(userDeviceHandle, PP_HASH_BYTE_TEMPLATE_MEM_ACC_DATA0, puData[1]);
    return sbFe2000ByteHashConfigAccess(userDeviceHandle, uTemplateId, 0);
}

uint32_t 
sbFe2000ByteHashConfigRead(sbhandle userDeviceHandle, uint32_t uTemplateId, uint32_t *puData)
{
    int rv;
    SOC_SBX_WARM_BOOT_DECLARE(int wb);

    SOC_SBX_WARM_BOOT_IGNORE((int)userDeviceHandle, wb);

    rv = sbFe2000ByteHashConfigAccess(userDeviceHandle, uTemplateId, 1);
    if (rv == SB_OK) {
        puData[0] = SAND_HAL_FE2000_READ(userDeviceHandle, PP_HASH_BYTE_TEMPLATE_MEM_ACC_DATA1);
        puData[1] = SAND_HAL_FE2000_READ(userDeviceHandle, PP_HASH_BYTE_TEMPLATE_MEM_ACC_DATA0);
    }

    SOC_SBX_WARM_BOOT_OBSERVE((int)userDeviceHandle, wb);

    return rv;
}



uint32_t 
sbFe2000BitHashConfigAccess(sbhandle userDeviceHandle, uint32_t uTemplateId, uint32_t uRead)
{
    uint32_t uTemp, uCtrl, bAccessDone, uTimeout;

    uCtrl = (SAND_HAL_FE2000_SET_FIELD(userDeviceHandle, PP_HASH_BIT_TEMPLATE_MEM_ACC_CTRL,REQ, 1) |
             SAND_HAL_FE2000_SET_FIELD(userDeviceHandle, PP_HASH_BIT_TEMPLATE_MEM_ACC_CTRL,ACK, 1) |
             SAND_HAL_FE2000_SET_FIELD(userDeviceHandle, PP_HASH_BIT_TEMPLATE_MEM_ACC_CTRL,RD_WR_N, uRead) |
             SAND_HAL_FE2000_SET_FIELD(userDeviceHandle, PP_HASH_BIT_TEMPLATE_MEM_ACC_CTRL,ADDRESS, uTemplateId) );
    
    SAND_HAL_FE2000_WRITE(userDeviceHandle,  PP_HASH_BIT_TEMPLATE_MEM_ACC_CTRL, uCtrl);
    
    bAccessDone = FALSE;
    uTimeout = 0;

    while (!bAccessDone) {
        uTemp = SAND_HAL_FE2000_READ(userDeviceHandle,  PP_HASH_BIT_TEMPLATE_MEM_ACC_CTRL);
        if (SAND_HAL_FE2000_GET_FIELD(userDeviceHandle, PP_HASH_BIT_TEMPLATE_MEM_ACC_CTRL, ACK, uTemp) == 1) {
            bAccessDone = TRUE;
        }
        uTimeout++;
        if (uTimeout > SB_FE2000_PP_INDIR_TIMEOUT) {
            break;
        }
    }

    if(!bAccessDone)  {
        return SB_TIMEOUT_ERR_CODE;
    }
    return SB_OK;
}

uint32_t 
sbFe2000BitHashConfigWrite(sbhandle userDeviceHandle, uint32_t uTemplateId, uint32_t *puData)
{

  /* First DATA register is number 9... */
  SAND_HAL_FE2000_WRITE(userDeviceHandle, PP_HASH_BIT_TEMPLATE_MEM_ACC_DATA9, puData[0]);
  SAND_HAL_FE2000_WRITE(userDeviceHandle, PP_HASH_BIT_TEMPLATE_MEM_ACC_DATA8, puData[1]);
  SAND_HAL_FE2000_WRITE(userDeviceHandle, PP_HASH_BIT_TEMPLATE_MEM_ACC_DATA7, puData[2]);
  SAND_HAL_FE2000_WRITE(userDeviceHandle, PP_HASH_BIT_TEMPLATE_MEM_ACC_DATA6, puData[3]);
  SAND_HAL_FE2000_WRITE(userDeviceHandle, PP_HASH_BIT_TEMPLATE_MEM_ACC_DATA5, puData[4]);
  SAND_HAL_FE2000_WRITE(userDeviceHandle, PP_HASH_BIT_TEMPLATE_MEM_ACC_DATA4, puData[5]);
  SAND_HAL_FE2000_WRITE(userDeviceHandle, PP_HASH_BIT_TEMPLATE_MEM_ACC_DATA3, puData[6]);
  SAND_HAL_FE2000_WRITE(userDeviceHandle, PP_HASH_BIT_TEMPLATE_MEM_ACC_DATA2, puData[7]);
  SAND_HAL_FE2000_WRITE(userDeviceHandle, PP_HASH_BIT_TEMPLATE_MEM_ACC_DATA1, puData[8]);
  SAND_HAL_FE2000_WRITE(userDeviceHandle, PP_HASH_BIT_TEMPLATE_MEM_ACC_DATA0, puData[9]);

  return sbFe2000BitHashConfigAccess(userDeviceHandle, uTemplateId, 0);
}



uint32_t 
sbFe2000BitHashConfigRead(sbhandle userDeviceHandle, uint32_t uTemplateId, uint32_t *puData)
{

    int rv;
    SOC_SBX_WARM_BOOT_DECLARE(int wb);

    SOC_SBX_WARM_BOOT_IGNORE((int)userDeviceHandle, wb);

    rv = sbFe2000BitHashConfigAccess(userDeviceHandle, uTemplateId, 1);

    /* First DATA register is number 9... */
    puData[0] = SAND_HAL_FE2000_READ(userDeviceHandle, PP_HASH_BIT_TEMPLATE_MEM_ACC_DATA9);
    puData[1] = SAND_HAL_FE2000_READ(userDeviceHandle, PP_HASH_BIT_TEMPLATE_MEM_ACC_DATA8);
    puData[2] = SAND_HAL_FE2000_READ(userDeviceHandle, PP_HASH_BIT_TEMPLATE_MEM_ACC_DATA7);
    puData[3] = SAND_HAL_FE2000_READ(userDeviceHandle, PP_HASH_BIT_TEMPLATE_MEM_ACC_DATA6);
    puData[4] = SAND_HAL_FE2000_READ(userDeviceHandle, PP_HASH_BIT_TEMPLATE_MEM_ACC_DATA5);
    puData[5] = SAND_HAL_FE2000_READ(userDeviceHandle, PP_HASH_BIT_TEMPLATE_MEM_ACC_DATA4);
    puData[6] = SAND_HAL_FE2000_READ(userDeviceHandle, PP_HASH_BIT_TEMPLATE_MEM_ACC_DATA3);
    puData[7] = SAND_HAL_FE2000_READ(userDeviceHandle, PP_HASH_BIT_TEMPLATE_MEM_ACC_DATA2);
    puData[8] = SAND_HAL_FE2000_READ(userDeviceHandle, PP_HASH_BIT_TEMPLATE_MEM_ACC_DATA1);
    puData[9] = SAND_HAL_FE2000_READ(userDeviceHandle, PP_HASH_BIT_TEMPLATE_MEM_ACC_DATA0);
    
    SOC_SBX_WARM_BOOT_OBSERVE((int)userDeviceHandle, wb);

    return rv;
}

uint32_t 
sbFe2000BatchGroupRecordSizeWrite(sbhandle userDeviceHandle, uint32_t uBatchGroup, uint32_t uRecordSize)
{
  uint32_t uTemp =0;
  uint32_t uCtrl =0;
  uint32_t bWriteDone = 0;
  uint32_t uTimeout =0;


  uTemp = SAND_HAL_FE2000_SET_FIELD(userDeviceHandle,PP_HEADER_RECORD_SIZE_MEM_ACC_DATA, DATA, uRecordSize);
  SAND_HAL_FE2000_WRITE(userDeviceHandle, PP_HEADER_RECORD_SIZE_MEM_ACC_DATA, uTemp);

  uCtrl =0;
  uCtrl = (SAND_HAL_FE2000_SET_FIELD(userDeviceHandle, PP_HEADER_RECORD_SIZE_MEM_ACC_CTRL,REQ, 1) |
           SAND_HAL_FE2000_SET_FIELD(userDeviceHandle, PP_HEADER_RECORD_SIZE_MEM_ACC_CTRL,ACK, 1) |
           SAND_HAL_FE2000_SET_FIELD(userDeviceHandle, PP_HEADER_RECORD_SIZE_MEM_ACC_CTRL,RD_WR_N, 0) |
           SAND_HAL_FE2000_SET_FIELD(userDeviceHandle, PP_HEADER_RECORD_SIZE_MEM_ACC_CTRL,ADDRESS, uBatchGroup) );

  SAND_HAL_FE2000_WRITE(userDeviceHandle,  PP_HEADER_RECORD_SIZE_MEM_ACC_CTRL, uCtrl);

  bWriteDone = FALSE;
  uTimeout = 0;
  while(!bWriteDone) {
    uTemp = SAND_HAL_FE2000_READ(userDeviceHandle,  PP_HEADER_RECORD_SIZE_MEM_ACC_CTRL);
    if(SAND_HAL_FE2000_GET_FIELD(userDeviceHandle,PP_HEADER_RECORD_SIZE_MEM_ACC_CTRL,ACK, uTemp) == 1) {
      bWriteDone = TRUE;
    }
    uTimeout++;
    if(uTimeout > SB_FE2000_PP_INDIR_TIMEOUT) {
      break;
    }
  }

  if(!bWriteDone)  {
    return SB_TIMEOUT_ERR_CODE;
  }
  return SB_OK;
}
