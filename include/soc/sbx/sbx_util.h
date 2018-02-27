/*
 * $Id: sbx_util.h 1.3 Broadcom SDK $
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
 * SBX utility macros and other useful accessor routines
 */

#ifndef _SBX_UTIL_H_
#define _SBX_UTIL_H_

#include <sal/core/thread.h> /* for sal_usleep */

#define SOC_SBX_UTIL_READ_REQUEST(unit, device, reg, otherAddr) \
SAND_HAL_WRITE(unit, device, reg##_ACC_CTRL, \
SAND_HAL_SET_FIELD(device, reg##_ACC_CTRL, REQ,1)|\
SAND_HAL_SET_FIELD(device, reg##_ACC_CTRL, ACK,1)| \
SAND_HAL_SET_FIELD(device, reg##_ACC_CTRL, RD_WR_N, 1) | otherAddr )

#define SOC_SBX_UTIL_READ_REQUEST_CLR_ON_RD(unit, device, reg, otherAddr) \
SAND_HAL_WRITE(unit, device, reg##_ACC_CTRL, \
SAND_HAL_SET_FIELD(device, reg##_ACC_CTRL, REQ,1)|\
SAND_HAL_SET_FIELD(device, reg##_ACC_CTRL, ACK,1)| \
SAND_HAL_SET_FIELD(device, reg##_ACC_CTRL, CLR_ON_RD,1)| \
SAND_HAL_SET_FIELD(device, reg##_ACC_CTRL, RD_WR_N, 1) | otherAddr )

#define SOC_SBX_UTIL_WRITE_REQUEST(unit, device, reg, otherAddr)\
SAND_HAL_WRITE(unit, device, reg##_ACC_CTRL, \
SAND_HAL_SET_FIELD(device, reg##_ACC_CTRL, REQ,1)|\
SAND_HAL_SET_FIELD(device, reg##_ACC_CTRL, ACK,1)| \
SAND_HAL_SET_FIELD(device, reg##_ACC_CTRL, RD_WR_N, 0) | otherAddr )

#define SOC_SBX_UTIL_WRITE_REQUEST_WITH_PARITY(unit, device, otherAddr, parity) \
SAND_HAL_WRITE(unit, device, reg##_ACC_CTRL, \
SAND_HAL_SET_FIELD(device, reg##_ACC_CTRL, REQ,1)|\
SAND_HAL_SET_FIELD(device, reg##_ACC_CTRL, ACK,1)| \
SAND_HAL_SET_FIELD(device, reg##_ACC_CTRL, CORRUPT_PARITY, parity)| \
SAND_HAL_SET_FIELD(device, reg##_ACC_CTRL, RD_WR_N, 0) | otherAddr )

#ifdef VXWORKS

#define SOC_SBX_UTIL_WAIT_FOR_ACK(unit, device, reg, nTimeoutInMs, nStatus) \
{ \
int32_t nTimeCnt = 0;\
while (1) {\
  uint32_t uDataWaitForAck = SAND_HAL_READ(unit, device, reg##_ACC_CTRL); \
  nStatus = 0;\
  if (SAND_HAL_GET_FIELD(device, reg##_ACC_CTRL, ACK, uDataWaitForAck)==1){ \
    uDataWaitForAck = SAND_HAL_MOD_FIELD(device, reg##_ACC_CTRL, REQ, uDataWaitForAck, 0); \
    uDataWaitForAck = SAND_HAL_MOD_FIELD(device, reg##_ACC_CTRL, ACK, uDataWaitForAck, 1); \
    SAND_HAL_WRITE(unit, device, reg##_ACC_CTRL, uDataWaitForAck);\
    break; \
  }\
  if (nTimeCnt >= nTimeoutInMs){ \
   nStatus = -1; \
   break; \
  } \
  sal_udelay(1000); \
  nTimeCnt++; \
} \
}

#else /* VXWORKS */

#define SOC_SBX_UTIL_WAIT_FOR_ACK(unit, device, reg, nTimeoutInMs, nStatus) \
{ \
int32_t nTimeCnt = 0;\
while (1) {\
  uint32_t uDataWaitForAck = SAND_HAL_READ(unit, device, reg##_ACC_CTRL); \
  nStatus = 0;\
  if (SAND_HAL_GET_FIELD(device, reg##_ACC_CTRL, ACK, uDataWaitForAck)==1){ \
    uDataWaitForAck = SAND_HAL_MOD_FIELD(device, reg##_ACC_CTRL, REQ, uDataWaitForAck, 0); \
    uDataWaitForAck = SAND_HAL_MOD_FIELD(device, reg##_ACC_CTRL, ACK, uDataWaitForAck, 1); \
    SAND_HAL_WRITE(unit, device, reg##_ACC_CTRL, uDataWaitForAck);\
    break; \
  }\
  if (nTimeCnt >= nTimeoutInMs){ \
   nStatus = -1; \
   break; \
  } \
  sal_usleep(1000); \
  nTimeCnt++; \
} \
}

#endif /* VXWORKS */

/* handle CA vs C2 differences */
#define SOC_SBX_UTIL_FE2000_WAIT_FOR_ACK(unit, reg, nTimeoutInMs, nStatus) \
    if (SOC_SBX_CONTROL(unit)->fetype == SOC_SBX_FETYPE_FE2KXT) {\
        SOC_SBX_UTIL_WAIT_FOR_ACK(unit, C2, reg, nTimeoutInMs, nStatus) \
    }else{ \
        SOC_SBX_UTIL_WAIT_FOR_ACK(unit, CA, reg, nTimeoutInMs, nStatus) \
    }

#define SOC_SBX_UTIL_FE2000_WRITE_REQUEST(unit, reg, otherAddr)\
    (SOC_SBX_CONTROL(unit)->fetype == SOC_SBX_FETYPE_FE2KXT) \
    ? SOC_SBX_UTIL_WRITE_REQUEST(unit, C2, reg, otherAddr)\
    : SOC_SBX_UTIL_WRITE_REQUEST(unit, CA, reg, otherAddr)

#define SOC_SBX_UTIL_FE2000_READ_REQUEST(unit, reg, otherAddr) \
    (SOC_SBX_CONTROL(unit)->fetype == SOC_SBX_FETYPE_FE2KXT) \
    ? SOC_SBX_UTIL_READ_REQUEST(unit, C2, reg, otherAddr) \
    : SOC_SBX_UTIL_READ_REQUEST(unit, CA, reg, otherAddr)
#endif /* _SBX_UTIL_H_ */
