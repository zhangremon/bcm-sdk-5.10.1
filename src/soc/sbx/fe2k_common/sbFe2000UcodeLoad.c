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
 *
 * $Id: sbFe2000UcodeLoad.c 1.16.20.1 Broadcom SDK $
 *
 * sbFe2000UcodeLoad.c : Fe2000 Ucode Initialization
 *
 *-----------------------------------------------------------------------------*/

#include "hal_ca_auto.h"
#include "hal_c2_auto.h"
#include "hal_ca_c2.h"
#include "glue.h"
#include "sbWrappers.h"
#include "sbFe2000UcodeLoad.h"
#include "sbFe2000Util.h"
#include <soc/sbx/fe2k_common/fe2k-asm2-intd.h>
#include <soc/sbx/fe2k_common/fe2k-asm2-md5.h>
#include <soc/sbx/fe2k_common/Bsc.h>
#include <bcm/error.h>
#include <soc/sbx/sbx_drv.h>

int fe2000_set_lrp_default_regs(int);

/*-----------------------------------------------------------------------------
 * @fn sbFe2000UcodeLoad()
 *
 * @brief Writting Fe2000 ucode into the LRP memory
 *
 * @param Pointer to Interface Library Control Structure
 * @param Pointer to Ucode byte array
 *
 * @return error code, SB_OK on success
 *----------------------------------------------------------------------------*/
uint32_t sbFe2000UcodeLoad(sbhandle userDeviceHandle, uint8_t * ucode){
  UINT nIndex;
  UINT uInstructionDirection;
  UINT uStream, uAddress, uInstructionIndex;
  fe2kAsm2IntD * ucodeBytes = (fe2kAsm2IntD *)ucode;
  unsigned char aucInstructionBuffer[SB_FE2000_LR_INSTRUCTION_SIZE];
  UINT uTaskReg;
  UINT uIntrData[3];
  uint32_t status = SB_OK;
  int nByte, nDataByte, nWord=0;
  uint32_t numStreams;
  uint32_t loop;

  if (SAND_HAL_IS_FE2KXT(userDeviceHandle)) {
    numStreams = SB_FE2000XT_LR_NUMBER_OF_STREAMS;
  }else{
    numStreams = SB_FE2000_LR_NUMBER_OF_STREAMS;
  }

  for (loop=0; loop<=(SAND_HAL_IS_FE2KXT(userDeviceHandle)); loop++) {
    uTaskReg = 0;
    aucInstructionBuffer[10] = 0;
    aucInstructionBuffer[11] = 0;
    for ( uStream = 0;
          uStream < numStreams; uStream++) {
      for ( uInstructionIndex = 0;
            uInstructionIndex < SB_FE2000_LR_NUMBER_OF_INSTRS;
            uInstructionIndex++) {
        uInstructionDirection = (ucodeBytes->m_tmap[uInstructionIndex/8] >> (7 - (uInstructionIndex % 8))) & 0x1;
        
        if ( uStream == 0 ) {
          uTaskReg |= uInstructionDirection << (uInstructionIndex % 32);
          if ( (uInstructionIndex % 32) == 31 ) {
            
            
            UINT uStride = SAND_HAL_FE2000_REG_OFFSET(userDeviceHandle, LR_TASK1) - SAND_HAL_FE2000_REG_OFFSET(userDeviceHandle, LR_TASK0);
            UINT addr1 =  SAND_HAL_FE2000_REG_OFFSET(userDeviceHandle, LR_TASK0) + (uStride * (uInstructionIndex/32));
            SAND_HAL_FE2000_WRITE_OFFS(userDeviceHandle, addr1, uTaskReg);
            uTaskReg = 0;
          }
        }
        
        uAddress = uStream * SB_FE2000_LR_NUMBER_OF_INSTRS + uInstructionIndex;
        
        for(nIndex=0; nIndex<10; nIndex++)
          aucInstructionBuffer[nIndex]  =
            ucodeBytes->m_code[(((uStream * 1024) + uInstructionIndex) * 10)
                               + nIndex];
        
        nWord = 0;
        for( nByte=0, nDataByte=11;
             nByte<12;
                nByte++, nDataByte--)
        {
          if(nByte%4==0) uIntrData[nWord]=0;
          
          uIntrData[nWord] |= aucInstructionBuffer[nDataByte]<<(8*(nByte%4));
          
          if(nByte%4 == 3) nWord++ ;
        }
        
        status = sbFe2000LrpInstructionMemoryWrite(userDeviceHandle,
                                                   uAddress, uIntrData[0],
                                                   uIntrData[1], uIntrData[2]);
        if (status != SB_OK) {
          return status;
        }
      }
    }

    status = fe2kAsm2IntD__initF(ucodeBytes,
                                 sbFe2000LrpInstructionMemoryWriteCallback,
                                 sbFe2000LrpInstructionMemoryReadCallback,
                                 userDeviceHandle);

    if (status != SB_OK) {
      return status;
    }

    if (SAND_HAL_IS_FE2KXT(userDeviceHandle) && (loop==0) ){
      sbFe2000SwapInstructionMemoryBank(userDeviceHandle);
    }
  }

  return status;
}

/*-----------------------------------------------------------------------------
 * @fn sbFe2000DisableLr()
 *
 * @brief Turn off Lrp
 *
 * @param Pointer to Interface Library Control Structure
 *
 * @return error code, SB_OK on success
 *----------------------------------------------------------------------------*/
uint32_t sbFe2000DisableLr(sbhandle userDeviceHandle)
{
  /* updated way to disable */
  uint32_t timeout;
  uint32_t uData;
  uint32_t uPendingRequests;
  uint32_t uLrpOffline;

  /* 1. set lr_config2.rq_frames_per_context to zero (this stops LRP from requesting frames) */
  if (!SAND_HAL_IS_FE2KXT(userDeviceHandle)){
    SAND_HAL_RMW_FIELD(userDeviceHandle,CA,LR_CONFIG2,RQ_FRAMES_PER_CONTEXT,0);
  }else{
    SAND_HAL_RMW_FIELD(userDeviceHandle,C2,LU_CONFIG0,RQ_FRAMES_PER_CONTEXT,0);
  }
  /* 2. Wait for pending requests to go to zero */
  timeout = 200000;
  while(timeout) {
    uData = SAND_HAL_FE2000_READ(userDeviceHandle,LR_STATUS);
    uPendingRequests = SAND_HAL_FE2000_GET_FIELD(userDeviceHandle,LR_STATUS,PENDING_REQS,uData);
    if (uPendingRequests == 0)
      break;
    thin_delay(500);
    timeout--;
  }

  if (timeout == 0) {
    SB_LOG("ERROR Timed out waiting for lrp pending requests to finish\n");
    return (-1);
  }

  /* 3. Shut down context loading */
  SAND_HAL_FE2000_RMW_FIELD(userDeviceHandle,LR_CONFIG1,FRAMES_PER_CONTEXT,0);

  /* 4. Request shutdown */
  SAND_HAL_FE2000_RMW_FIELD(userDeviceHandle,LR_CONFIG0,SHUTDOWN,1);

  /* 5. Wait for LRP to go offline */
  timeout = 20000000;
  while(timeout) {
    uData = SAND_HAL_FE2000_READ(userDeviceHandle,LR_EVENT);
    uLrpOffline = SAND_HAL_FE2000_GET_FIELD(userDeviceHandle,LR_EVENT,OFFLINE,uData);
    if (uLrpOffline == 1)
      break;
    thin_delay(500);
    timeout--;
  }

  if (timeout == 0) {
    SB_LOG("ERROR Timed out waiting for lrp to go offline\n");
    return (-1);
  }

  /* 6. Clear offline status */
  SAND_HAL_FE2000_RMW_FIELD(userDeviceHandle,LR_EVENT,OFFLINE,1);

  /* 7. Reset Control/Config */
  SAND_HAL_FE2000_WRITE(userDeviceHandle,LR_EPOCH_CNTRL,0);

  /* restore reset state to lr_config0/1/2 */
  SAND_HAL_FE2000_WRITE(userDeviceHandle,LR_CONFIG0,0);

  uData = 0;
  uData = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle,LR_CONFIG1,DONE_DELAY,uData,0);
  uData = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle,LR_CONFIG1,LOAD_COUNT,uData,1);
  uData = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle,LR_CONFIG1,WORDS1,uData,12);
  uData = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle,LR_CONFIG1,WORDS0,uData,12);
  uData = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle,LR_CONFIG1,PPE_PACE,uData,2);
  uData = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle,LR_CONFIG1,FRAMES_PER_CONTEXT,uData,48);
  SAND_HAL_FE2000_WRITE(userDeviceHandle,LR_CONFIG1,uData);


  uData = 0;
  if (!SAND_HAL_IS_FE2KXT(userDeviceHandle)){
    uData = SAND_HAL_MOD_FIELD(CA,LR_CONFIG2,RQ_THRESHOLD,uData,85);
    uData = SAND_HAL_MOD_FIELD(CA,LR_CONFIG2,RQ_FRAMES_PER_CONTEXT,uData,48);
    SAND_HAL_WRITE(userDeviceHandle,CA,LR_CONFIG2,uData);
  }else{
    uData = SAND_HAL_MOD_FIELD(C2,LU_CONFIG0,RQ_THRESHOLD,uData,85);
    uData = SAND_HAL_MOD_FIELD(C2,LU_CONFIG0,RQ_FRAMES_PER_CONTEXT,uData,48);
    SAND_HAL_WRITE(userDeviceHandle,C2,LU_CONFIG0,uData);
  }

  return SB_OK;
}

/*-----------------------------------------------------------------------------
 * @fn sbFe2000EnableLr()
 *
 * @brief Turn on Lrp
 *
 * @param Pointer to Interface Library Control Structure
 *
 * @return error code, SB_OK on success
 *----------------------------------------------------------------------------*/
uint32_t sbFe2000EnableLr(sbhandle userDeviceHandle, uint8_t *ucode)
{
  uint32_t status = SB_OK;
  int words0;
  int words1;
  UINT uConfig0, uConfig1, elen, num_switches;
  UINT uStreamIndex;
  UINT uStreamOnline;
  UINT nRequestThreshold;
  UINT uNumStreams;
  fe2kAsm2IntD *ucodeBytes = (fe2kAsm2IntD *)ucode;
  UINT uLoadCount,uContexts;

  uConfig0 = SAND_HAL_FE2000_READ(userDeviceHandle, LR_CONFIG0);
  if (SAND_HAL_FE2000_GET_FIELD(userDeviceHandle, LR_CONFIG0, BYPASS, uConfig0)) {
    return status;
  }

  
  if (SAND_HAL_IS_FE2KXT(userDeviceHandle)) {
    words0 = words1 = 10;
  }else{
    words0 = words1 = 8;
  }

  /* NOTE: assumes even split */
  nRequestThreshold =(1024/words0);
  if (!SAND_HAL_IS_FE2KXT(userDeviceHandle)){
    uNumStreams = SB_FE2000_LR_NUMBER_OF_STREAMS;
    SAND_HAL_RMW_FIELD(userDeviceHandle, CA, LR_CONFIG2, RQ_THRESHOLD, nRequestThreshold);
    uLoadCount = 3;
  }else{
    uNumStreams = SB_FE2000XT_LR_NUMBER_OF_STREAMS;
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, LU_CONFIG0, RQ_THRESHOLD, nRequestThreshold);
    uLoadCount = 1;
  }

  uConfig1 = 0;
  uConfig1 = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, LR_CONFIG1, DONE_DELAY, uConfig1, 0);
  uConfig1 = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, LR_CONFIG1, LOAD_COUNT, uConfig1, uLoadCount);
  uConfig1 = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, LR_CONFIG1, WORDS1, uConfig1, words1);
  uConfig1 = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, LR_CONFIG1, WORDS0, uConfig1, words0);
  uConfig1 = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, LR_CONFIG1, PPE_PACE, uConfig1, 1);
  uConfig1 = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, LR_CONFIG1, FRAMES_PER_CONTEXT, uConfig1,
                                0x30);
  SAND_HAL_FE2000_WRITE(userDeviceHandle, LR_CONFIG1, uConfig1);

  uStreamOnline = 0;

  for( uStreamIndex = 0; uStreamIndex < uNumStreams;
       uStreamIndex++ ) {
    uStreamOnline |= (1 << uStreamIndex);
  }

  uConfig0 = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, LR_CONFIG0, ONLINE, uConfig0,
                                uStreamOnline);
  uConfig0 = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, LR_CONFIG0, ENABLE, uConfig0, 1);
  uConfig0 = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, LR_CONFIG0, LOAD_ENABLE, uConfig0, 1);
  /* VxWorks build seems to have defined UPDATE */
#ifdef UPDATE
#define _SB_UPDATE_SAVE UPDATE
#undef UPDATE
#endif
  uConfig0 = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, LR_CONFIG0, UPDATE, uConfig0, 1);
#ifdef _SB_UPDATE_SAVE
#define UPDATE _SB_UPDATE_SAVE
#endif

  /* TODO: need to put this in network byte order in the package & reconcile */
  /* should bubble up paired mode */
  if(fe2kAsm2IntD__getNc(ucodeBytes, "elen", &elen) > 0){
    SB_ASSERT(0);
  }
  if(fe2kAsm2IntD__getNc(ucodeBytes, "num_switches", &num_switches) > 0){
    /* Set num_switches to max value */
    if (!SAND_HAL_IS_FE2KXT(userDeviceHandle)){
      num_switches = 5;
    } else {
      num_switches = 3;
    }
  }

  /* context value is calculated as follows:
   *   number of contexts = (num_switches + 1) + load_context + unload_context 
   *   and this number is doubled for paired mode (multiply by 2).
   *   finally, the value is zero based (need to subtract 1).
   */
  uContexts = ((num_switches + 3) * 2) - 1;

  uConfig0 = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, LR_CONFIG0, EPOCH, uConfig0,
                                elen - 11);
  uConfig0 = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, LR_CONFIG0, PAIRED, uConfig0, 1);
  uConfig0 = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, LR_CONFIG0, CONTEXTS, uConfig0, uContexts);
  uConfig1 = SAND_HAL_FE2000_READ(userDeviceHandle, LR_SW_ASSIST);
  SAND_HAL_FE2000_WRITE(userDeviceHandle, LR_CONFIG0, uConfig0);

  return status;
}

/*-----------------------------------------------------------------------------
 * @fn sbFe2000BringUpLr()
 *
 * @brief The main function loading the Fe2000 ucode
 *
 * @param Pointer to Interface Library Control Structure
 * @param Pointer to Ucode byte array
 *
 * @return error code, SB_OK on success
 *----------------------------------------------------------------------------*/
uint32_t sbFe2000BringUpLr(sbhandle userDeviceHandle, uint8_t * ucode)
{
  uint32_t status = SB_OK;
  UINT uConfig0;

  uConfig0 = SAND_HAL_FE2000_READ(userDeviceHandle, LR_CONFIG0);
  if (SAND_HAL_FE2000_GET_FIELD(userDeviceHandle, LR_CONFIG0, BYPASS, uConfig0)) {
    return status;
  }

  status = sbFe2000UcodeLoad(userDeviceHandle, ucode);
  if (status != SB_OK) {
    return status;
  }

  status = sbFe2000EnableLr(userDeviceHandle, ucode);
  return status;
}


uint32_t
sbFe2000ReloadUcode(sbhandle userDeviceHandle, uint8_t * ucode)
{
  int status;

  status = sbFe2000DisableLr(userDeviceHandle);
  if(status != SB_OK)
    return status;

  status = sbFe2000UcodeLoad(userDeviceHandle, ucode);
  if(status != SB_OK)
    return status;

  status = sbFe2000EnableLr(userDeviceHandle, ucode);
  if(status != SB_OK)
    return status;

  return SB_OK;
}

/*-----------------------------------------------------------------------------
 * @fn sbFe2000GetNamedConstant
 *
 * @brief Get Named Constant value
 *
 * @param Pointer to Interface Library Control Structure
 * @param Name of the "Named Constant"
 *
 * @return Named Constant value
 *----------------------------------------------------------------------------*/
uint32_t sbFe2000GetNamedConstant(sbhandle userDeviceHandle, fe2kAsm2IntD *ucode, char *nc, uint32_t *val)
{
  int status = SB_OK;

  status = fe2kAsm2IntD__getNc(ucode, nc, val);

  return status;
}

/*-----------------------------------------------------------------------------
 * @fn sbFe2000SetNamedConstant
 *
 * @brief Set Named Constant value
 *
 * @param Pointer to Interface Library Control Structure
 * @param Name of the "Named Constant"
 *
 * @return Named Constant value
 *----------------------------------------------------------------------------*/
uint32_t sbFe2000SetNamedConstant(sbhandle userDeviceHandle, fe2kAsm2IntD *ucode, char *nc, uint32 val)
{
  int status = SB_OK;

  status = fe2kAsm2IntD__putNc(ucode, nc, &val);
  if(status != SB_OK)
    return -1;

  /* If the soc type is Caladan 2 then swap the bank and rewrite the same value in 
     the mirror RAM bank */

  if (SAND_HAL_IS_FE2KXT(userDeviceHandle)){

     status=  sbFe2000SwapInstructionMemoryBank(userDeviceHandle);
     if( status != SB_OK)
       return status;

     status = fe2kAsm2IntD__putNc(ucode, nc, (unsigned int *)&val);
     if(status != SB_OK)
       return -1;
     
  }

  return status;
}

/*-----------------------------------------------------------------------------
 * @fn sbFe2000LrpInstructionMemoryRead()
 *
 * @brief Reading one Ucode instruction
 *
 * @param Pointer to Interface Library Control Structure
 * @param Memory address of the instruction to be written to
 * @param First 32-bit instruction word
 * @param Second 32-bit instruction word
 * @param Third 32-bit instruction word
 *
 * @return error code, SB_OK on success
 *----------------------------------------------------------------------------*/
uint32_t sbFe2000LrpInstructionMemoryRead(sbhandle userDeviceHandle,
                                          UINT uAddress,
                                          UINT *pData0,
                                          UINT *pData1,
                                          UINT *pData2)
{
  /* pls sync with system_sim/verify/caladan/src/CaladanDriver.c */
  uint32_t rv = SB_OK;
  uint32_t nnCutoffTime = 0;
  SOC_SBX_WARM_BOOT_DECLARE(int wb);

  SOC_SBX_WARM_BOOT_IGNORE((int)userDeviceHandle, wb);
  SAND_HAL_FE2000_WRITE(userDeviceHandle, LR_IMEM_ACC_CTRL,
                 SAND_HAL_FE2000_SET_FIELD(userDeviceHandle, LR_IMEM_ACC_CTRL, REQ, 1) |
                 SAND_HAL_FE2000_SET_FIELD(userDeviceHandle, LR_IMEM_ACC_CTRL, ACK, 1) |
                 SAND_HAL_FE2000_SET_FIELD(userDeviceHandle, LR_IMEM_ACC_CTRL, RD_WR_N, 1) |
                 SAND_HAL_FE2000_SET_FIELD(userDeviceHandle, LR_IMEM_ACC_CTRL, ADDRESS, uAddress)
                 );

  while (1) {
    UINT nData = SAND_HAL_FE2000_READ(userDeviceHandle, LR_IMEM_ACC_CTRL);
    if (SAND_HAL_FE2000_GET_FIELD(userDeviceHandle, LR_IMEM_ACC_CTRL, ACK, nData) == 1 ) {
      nData = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, LR_IMEM_ACC_CTRL, REQ, nData, 0);
      nData = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, LR_IMEM_ACC_CTRL, ACK, nData, 1);
      SAND_HAL_FE2000_WRITE(userDeviceHandle, LR_IMEM_ACC_CTRL,  nData);
      break;
    }
    nnCutoffTime++;
    if( nnCutoffTime > 100 ) {
      rv = SB_FE_MEM_ACC_TIMEOUT_ERR_CODE;
      break;
    }
  }
  
  SOC_SBX_WARM_BOOT_OBSERVE((int)userDeviceHandle, wb);

  if (rv == SB_OK) {
    *pData0 = SAND_HAL_FE2000_READ(userDeviceHandle, LR_IMEM_ACC_DATA0);
    *pData1 = SAND_HAL_FE2000_READ(userDeviceHandle, LR_IMEM_ACC_DATA1);
    *pData2 = SAND_HAL_FE2000_READ(userDeviceHandle, LR_IMEM_ACC_DATA2);
  }

  return rv;
}

uint32_t
sbFe2000LrpGetInstruction(sbhandle userDeviceHandle, int stream, int pc,
                          UINT *pData0, UINT *pData1, UINT *pData2, int *task)
{
  UINT taskstride =
    SAND_HAL_FE2000_REG_OFFSET(userDeviceHandle, LR_TASK1) - SAND_HAL_FE2000_REG_OFFSET(userDeviceHandle, LR_TASK0);
  UINT taskaddr = SAND_HAL_FE2000_REG_OFFSET(userDeviceHandle, LR_TASK0) + (taskstride * (pc / 32));
  UINT taskreg;
  UINT status;
  UINT instaddr;

  if (SAND_HAL_IS_FE2KXT(userDeviceHandle)) {
    if (stream >=  SB_FE2000XT_LR_NUMBER_OF_STREAMS) {
      return SB_BAD_ARGUMENT_ERR_CODE;
    }
  } else {
    if (stream >= SB_FE2000_LR_NUMBER_OF_STREAMS) {
      return SB_BAD_ARGUMENT_ERR_CODE;
    }
  }

  if (pc >= SB_FE2000_LR_NUMBER_OF_INSTRS) {
    return SB_BAD_ARGUMENT_ERR_CODE;
  }

  taskreg = SAND_HAL_FE2000_READ_OFFS(userDeviceHandle, taskaddr);
  *task = (taskreg >> (pc % 32)) & 1;

  instaddr = stream * SB_FE2000_LR_NUMBER_OF_INSTRS + pc;

  status = sbFe2000LrpInstructionMemoryRead(userDeviceHandle, instaddr,
                                            pData0, pData1, pData2);

  return status;
}

/*-----------------------------------------------------------------------------
 * @fn sbFe2000LrpInstructionMemoryWrite()
 *
 * @brief Writting one Ucode instruction into the FE2000 memory
 *
 * @param Pointer to Interface Library Control Structure
 * @param Memory address of the instruction to be written to
 * @param First 32-bit instruction word
 * @param Second 32-bit instruction word
 * @param Third 32-bit instruction word
 *
 * @return error code, SB_OK on success
 *----------------------------------------------------------------------------*/
uint32_t sbFe2000LrpInstructionMemoryWrite(sbhandle userDeviceHandle,
                                           UINT uAddress,
                                           UINT uData0,
                                           UINT uData1,
                                           UINT uData2)
{
  /* pls sync with system_sim/verify/caladan/src/CaladanDriver.c */

  uint32_t nnCutoffTime = 0;
  if (SOC_WARM_BOOT((int)userDeviceHandle)) {
    return SB_OK;
  }

  SAND_HAL_FE2000_WRITE(userDeviceHandle, LR_IMEM_ACC_DATA0, uData0);
  SAND_HAL_FE2000_WRITE(userDeviceHandle, LR_IMEM_ACC_DATA1, uData1);
  SAND_HAL_FE2000_WRITE(userDeviceHandle, LR_IMEM_ACC_DATA2, uData2);

  SAND_HAL_FE2000_WRITE(userDeviceHandle, LR_IMEM_ACC_CTRL,
                 SAND_HAL_FE2000_SET_FIELD(userDeviceHandle, LR_IMEM_ACC_CTRL, REQ, 1) |
                 SAND_HAL_FE2000_SET_FIELD(userDeviceHandle, LR_IMEM_ACC_CTRL, ACK, 1) |
                 SAND_HAL_FE2000_SET_FIELD(userDeviceHandle, LR_IMEM_ACC_CTRL, RD_WR_N, 0) |
                 SAND_HAL_FE2000_SET_FIELD(userDeviceHandle, LR_IMEM_ACC_CTRL, ADDRESS, uAddress)
                 );

  while (1) {
    UINT nData = SAND_HAL_FE2000_READ(userDeviceHandle, LR_IMEM_ACC_CTRL);
    if (SAND_HAL_FE2000_GET_FIELD(userDeviceHandle, LR_IMEM_ACC_CTRL, ACK, nData) == 1 ) {
      nData = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, LR_IMEM_ACC_CTRL, REQ, nData, 0);
      nData = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, LR_IMEM_ACC_CTRL, ACK, nData, 1);
      SAND_HAL_FE2000_WRITE(userDeviceHandle, LR_IMEM_ACC_CTRL,  nData);
      break;
    }
    nnCutoffTime++;
    if( nnCutoffTime > 100 ) {
      return SB_FE_MEM_ACC_TIMEOUT_ERR_CODE;
    }
  }
  return SB_OK;
}

/*-----------------------------------------------------------------------------
 * @fn sbFe2000LrpInstructionMemoryWriteCallback()
 *
 * @brief Callback to write one pushdown value from the FE2000 memory
 *
 * @return error code, SB_OK on success
 *----------------------------------------------------------------------------*/
int32_t sbFe2000LrpInstructionMemoryWriteCallback(void *a_pv,
                                                  unsigned int a_sn,
                                                  unsigned int a_in,
                                                  unsigned char *a_n)
{
  unsigned int status = SB_OK, uAddress;
  sbhandle userDeviceHandle = (sbhandle)a_pv;
  unsigned int uIntrData[3], nIndex;
  int nByte, nDataByte, nWord=0;
  unsigned char a_b[12];

  uAddress = a_sn * SB_FE2000_LR_NUMBER_OF_INSTRS + a_in;

  for(nIndex=0; nIndex<10; nIndex++)
            a_b[nIndex] = a_n[nIndex];
  a_b[10] = 0;
  a_b[11] = 0;

  nWord = 0;
  for( nByte=0, nDataByte=11;
       nByte<12;
       nByte++, nDataByte--) {

    if(nByte%4==0) uIntrData[nWord]=0;

    uIntrData[nWord] |= a_b[nDataByte]<<(8*(nByte%4));

    if(nByte%4 == 3) nWord++ ;
  }

  status = sbFe2000LrpInstructionMemoryWrite(userDeviceHandle,
                                             uAddress, uIntrData[0],
                                             uIntrData[1], uIntrData[2]);
  if (status != SB_OK) {
    return status;
  }

  return status;
}


/*-----------------------------------------------------------------------------
 * @fn sbFe2000LrpInstructionMemoryReadCallback()
 *
 * @brief Callback to read one pushdown value from the FE2000 memory
 *
 * @return error code, SB_OK on success
 *----------------------------------------------------------------------------*/
int32_t sbFe2000LrpInstructionMemoryReadCallback(void *a_pv,
                                                 unsigned int a_sn,
                                                 unsigned int a_in,
                                                 unsigned char *a_n)
{
  unsigned int status = SB_OK, uAddress;
  sbhandle userDeviceHandle = (sbhandle)a_pv;
  unsigned int uIntrData[3];

  uAddress = a_sn * SB_FE2000_LR_NUMBER_OF_INSTRS + a_in;
  status = sbFe2000LrpInstructionMemoryRead(userDeviceHandle,
                                            uAddress, uIntrData,
                                            uIntrData+1, uIntrData+2);
  if (status != SB_OK) {
    return status;
  }

  /* do not update a_n[10] and a_n[11] */

  a_n[9] = (uIntrData[0] & 0xff0000 ) >> 16;
  a_n[8] = (uIntrData[0] & 0xff000000 ) >> 24;

  a_n[7] = uIntrData[1] & 0xff;
  a_n[6] = (uIntrData[1] & 0xff00 ) >> 8;
  a_n[5] = (uIntrData[1] & 0xff0000 ) >> 16;
  a_n[4] = (uIntrData[1] & 0xff000000 ) >> 24;

  a_n[3] = uIntrData[2] & 0xff;
  a_n[2] = (uIntrData[2] & 0xff00 ) >> 8;
  a_n[1] = (uIntrData[2] & 0xff0000 ) >> 16;
  a_n[0] = (uIntrData[2] & 0xff000000 ) >> 24;

  return status;
}

/*-----------------------------------------------------------------------------
 * @fn sbFe2000LrpUcodeLoad()
 *
 * @brief Load ucode from buffer
 *
 * @param Pointer to Interface Library Control Structure
 * @param Pointer to the buffer which holds the ucode
 * @param Size of the buffer
 *
 * @return error code, SB_OK on success
 *----------------------------------------------------------------------------*/
unsigned int sbFe2000UcodeLoadFromBuffer(sbhandle userDeviceHandle,
                                         fe2kAsm2IntD *a_p,
                                         unsigned char *a_b,
                                         unsigned int a_l)
{
  unsigned int ret = SB_OK;

#if 1
  /* fe2kAsm2IntD__init(a_p) is called inside destroy */
  if((ret = fe2kAsm2IntD__destroy(a_p)) != 0)
    return ret;
#else
  if((ret = fe2kAsm2IntD__initF(a_p, sbFe2000LrpInstructionMemoryWrite1, sbFe2000LrpInstructionMemoryRead1, (void *)userDeviceHandle)) != 0)
    return ret;
#endif

  return fe2kAsm2IntD__readBuf(a_p, a_b, a_l);
}

/*-----------------------------------------------------------------------------
 * @fn sbFe2000SwapInstructionMemoryBank()
 *
 * @brief Swap the instruction memory to use the corresponding bank
 *
 * @param Pointer to Interface Library Control Structure
 *
 * @return error code, SB_OK on success
 *----------------------------------------------------------------------------*/


uint32_t sbFe2000SwapInstructionMemoryBank(sbhandle userDeviceHandle)
{
  uint32_t uBankSwapEvent, uTimeOut;

  if (SOC_WARM_BOOT((int)userDeviceHandle)) {
    return SB_OK;
  }

  if (SAND_HAL_IS_FE2KXT(userDeviceHandle)){
    /* clear bank_swap_event completion and request another bank swap */
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, LR_EVENT, BANK_SWAP_DONE, 1);
    SAND_HAL_RMW_FIELD(userDeviceHandle, C2, LR_CONFIG0, BANK_SWAP, 1);

    /* poll for completion */
    uBankSwapEvent = 0;
    uTimeOut = 10000;
    while ( !uBankSwapEvent && uTimeOut ) {
      uBankSwapEvent = SAND_HAL_GET_FIELD(C2, LR_EVENT, BANK_SWAP_DONE, SAND_HAL_READ(userDeviceHandle, C2, LR_EVENT));
      --uTimeOut;
    }
    if ( !uTimeOut ) {
      SB_ERROR("Timeout waiting for BANK_SWAP event from the LRP.\n");
      return SB_FE2000_STS_INIT_LR_UCODESWAP_TIMEOUT_ERR;

    } else {
      SAND_HAL_RMW_FIELD(userDeviceHandle, C2, LR_EVENT, BANK_SWAP_DONE, 1);
      return SB_OK;
    }

  }
  else
    return BCM_E_UNAVAIL;


}
