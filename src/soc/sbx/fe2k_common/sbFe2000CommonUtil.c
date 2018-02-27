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
 * $Id: sbFe2000CommonUtil.c 1.8.110.1 Broadcom SDK $
 *
 ******************************************************************************/

#include "sbTypes.h"
#include "sbWrappers.h"
#include "glue.h"
#include "soc/sbx/sbx_drv.h"
#include "hal_user.h"
#include "hal_ca_auto.h"
#include "hal_c2_auto.h"
#include "hal_ca_c2.h"
#include "sbFe2000Util.h"

int
sbFe2000UtilReadIndir(sbhandle userDeviceHandle, int raw, int descending,
                      uint32_t ctrlOffset, uint32_t address, uint32_t words,
                      uint32_t *data)
{
    int i,j;
    int offs;
    SOC_SBX_WARM_BOOT_DECLARE(int wb);

    SOC_SBX_WARM_BOOT_IGNORE((int)userDeviceHandle, wb);

    if(ctrlOffset == SAND_HAL_C2_RC0_RR_MEM_ACC_CTRL_OFFSET ||
       ctrlOffset == SAND_HAL_C2_RC1_RR_MEM_ACC_CTRL_OFFSET) {

        if(SAND_HAL_IS_FE2KXT((int) userDeviceHandle)){
            for (j = 0; j < words; j++) {
                address = address + j;
                SAND_HAL_FE2000_WRITE_OFFS(userDeviceHandle, ctrlOffset,
                                           SAND_HAL_FE2000_FIELD_MASK(userDeviceHandle, PD_COPY_IN_MEM_ACC_CTRL, ACK)
                                           | SAND_HAL_FE2000_FIELD_MASK(userDeviceHandle, PD_COPY_IN_MEM_ACC_CTRL, REQ)
                                           | SAND_HAL_FE2000_FIELD_MASK(userDeviceHandle, PD_COPY_IN_MEM_ACC_CTRL, RD_WR_N)
                                           | address);
                for (i = 0; i < 100; i++) {
                    if (SAND_HAL_FE2000_READ_OFFS(userDeviceHandle, ctrlOffset)
                        & SAND_HAL_FE2000_FIELD_MASK(userDeviceHandle, PD_COPY_IN_MEM_ACC_CTRL, ACK)) {
                        break;
                    }
                }
                if (i == 100) {
                    SOC_SBX_WARM_BOOT_OBSERVE((int)userDeviceHandle, wb);
                    return TRUE;
                }
                offs = ctrlOffset + 4;  /* Data offset = ctrlOffset + 4 */
                data[j] = SAND_HAL_READ_OFFS_RAW(userDeviceHandle, offs);
            }   
            SOC_SBX_WARM_BOOT_OBSERVE((int)userDeviceHandle, wb);
            return FALSE;
        }
    } 

    
    SAND_HAL_FE2000_WRITE_OFFS(userDeviceHandle, ctrlOffset,
                        SAND_HAL_FE2000_FIELD_MASK(userDeviceHandle, PD_COPY_IN_MEM_ACC_CTRL, ACK)
                        | SAND_HAL_FE2000_FIELD_MASK(userDeviceHandle, PD_COPY_IN_MEM_ACC_CTRL, REQ)
                        | SAND_HAL_FE2000_FIELD_MASK(userDeviceHandle, PD_COPY_IN_MEM_ACC_CTRL, RD_WR_N)
                        | address);
    for (i = 0; i < 100; i++) {
        if (SAND_HAL_FE2000_READ_OFFS(userDeviceHandle, ctrlOffset)
            & SAND_HAL_FE2000_FIELD_MASK(userDeviceHandle, PD_COPY_IN_MEM_ACC_CTRL, ACK)) {
            break;
        }
    }
    if (i == 100) {
        SOC_SBX_WARM_BOOT_OBSERVE((int)userDeviceHandle, wb);
        return TRUE;
    }

    for (i = 0; i < words; i++) {
        if (descending) {
            offs = words - i - 1;
        } else {
            offs = i;
        }
        offs = ctrlOffset + (1 + offs) * sizeof(uint32_t);
        if (raw) {
            data[i] = SAND_HAL_READ_OFFS_RAW(userDeviceHandle, offs);
        } else {
            
            data[i] = SAND_HAL_READ_OFFS(userDeviceHandle, offs);
        }
            
    }

    SOC_SBX_WARM_BOOT_OBSERVE((int)userDeviceHandle, wb);
    return FALSE;
}

int
sbFe2000UtilWriteIndir(sbhandle userDeviceHandle, int raw, int descending,
                       uint32_t ctrlOffset, uint32_t address, uint32_t words,
                       uint32_t *data)
{
    int i, j, offs;

    if (SOC_WARM_BOOT((int)userDeviceHandle)) {
        return FALSE;
    }

    if(ctrlOffset == SAND_HAL_C2_RC0_RR_MEM_ACC_CTRL_OFFSET ||
           ctrlOffset == SAND_HAL_C2_RC1_RR_MEM_ACC_CTRL_OFFSET) {
     if(SAND_HAL_IS_FE2KXT((int) userDeviceHandle)){
        for (j = 0; j < words; j++) {
            address = address + j;
            offs = ctrlOffset + 4;   /* Data offset = ctrlOffset + 4 */
            SAND_HAL_WRITE_OFFS_RAW(userDeviceHandle, offs, data[j]);

            SAND_HAL_WRITE_OFFS(userDeviceHandle, ctrlOffset,
                        SAND_HAL_FE2000_FIELD_MASK(userDeviceHandle, PD_COPY_IN_MEM_ACC_CTRL, ACK)
                        | SAND_HAL_FE2000_FIELD_MASK(userDeviceHandle, PD_COPY_IN_MEM_ACC_CTRL, REQ)
                        | address);
            for (i = 0; i < 100; i++) {
                if (SAND_HAL_READ_OFFS(userDeviceHandle, ctrlOffset)
                    & SAND_HAL_FE2000_FIELD_MASK(userDeviceHandle, PD_COPY_IN_MEM_ACC_CTRL, ACK)) {
                    break;
                }
            }
            if(i == 100) {
                return TRUE;
            }
        }
        return FALSE;
      }
    }

    for (i = 0; i < words; i++) {
        if (descending) {
            offs = words - i - 1;
        } else {
            offs = i;
        }
        offs = ctrlOffset + (1 + offs) * sizeof(uint32_t);
        if (raw) {
            SAND_HAL_WRITE_OFFS_RAW(userDeviceHandle, offs, data[i]);
        } else {
            SAND_HAL_WRITE_OFFS(userDeviceHandle, offs, data[i]);
        }
    }
    SAND_HAL_WRITE_OFFS(userDeviceHandle, ctrlOffset,
                        SAND_HAL_FE2000_FIELD_MASK(userDeviceHandle, PD_COPY_IN_MEM_ACC_CTRL, ACK)
                        | SAND_HAL_FE2000_FIELD_MASK(userDeviceHandle, PD_COPY_IN_MEM_ACC_CTRL, REQ)
                        | address);
    for (i = 0; i < 100; i++) {
        if (SAND_HAL_READ_OFFS(userDeviceHandle, ctrlOffset)
            & SAND_HAL_FE2000_FIELD_MASK(userDeviceHandle, PD_COPY_IN_MEM_ACC_CTRL, ACK)) {
            return FALSE;
        }
    }
    return TRUE;
}

void
sbFe2000UtilApplySoftReset(sbhandle userDeviceHandle)
{
  uint32_t uData;

  /* apply global soft reset, disable CII (Core Initiator Interface) */
  DENTER();
  SAND_HAL_FE2000_WRITE(userDeviceHandle, PC_INTERRUPT_MASK,
                 SAND_HAL_FE2000_REG_MASK(userDeviceHandle, PC_INTERRUPT_MASK));
  SAND_HAL_FE2000_RMW_FIELD(userDeviceHandle,PC_CONFIG, CII_ENABLE, 0x0);
  uData = SAND_HAL_FE2000_READ(userDeviceHandle, PC_RESET);
  uData = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PC_RESET, SOFT_RESET,uData, 1);
  uData = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PC_RESET, PC_CORE_RESET,uData, 1);
  uData = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PC_RESET, RESET_RXBUF,uData, 1);
  SAND_HAL_FE2000_WRITE(userDeviceHandle, PC_RESET, uData);
  SAND_HAL_FE2000_WRITE(userDeviceHandle, PC_CORE_RESET, 0xffffffff);
  /* Must `manually' reset some PCI block registers */
  SAND_HAL_FE2000_WRITE(userDeviceHandle, PC_DMA_CTRL0,
                 SAND_HAL_FE2000_FIELD_MASK(userDeviceHandle, PC_DMA_CTRL0, ACK));
  SAND_HAL_FE2000_WRITE(userDeviceHandle, PC_DMA_CTRL1,
                 SAND_HAL_FE2000_FIELD_MASK(userDeviceHandle, PC_DMA_CTRL1, ACK));
  SAND_HAL_FE2000_WRITE(userDeviceHandle, PC_DMA_CTRL2,
                 SAND_HAL_FE2000_FIELD_MASK(userDeviceHandle, PC_DMA_CTRL2, ACK));
  SAND_HAL_FE2000_WRITE(userDeviceHandle, PC_DMA_CTRL3,
                 SAND_HAL_FE2000_FIELD_MASK(userDeviceHandle, PC_DMA_CTRL3, ACK));
  SAND_HAL_FE2000_WRITE(userDeviceHandle, PC_TX_RING_PTR, 0);
  SAND_HAL_FE2000_WRITE(userDeviceHandle, PC_TX_RING_PRODUCER, 0);
  SAND_HAL_FE2000_WRITE(userDeviceHandle, PC_COMPLETION_RING_PTR, 0);
  SAND_HAL_FE2000_WRITE(userDeviceHandle, PC_COMPLETION_RING_CONSUMER, 0);

  SAND_HAL_FE2000_READ(userDeviceHandle, PC_TX_BYTE_CNT);
  SAND_HAL_FE2000_READ(userDeviceHandle, PC_TX_PKT_CNT);
  SAND_HAL_FE2000_READ(userDeviceHandle, PC_RX_BYTE_CNT);
  SAND_HAL_FE2000_READ(userDeviceHandle, PC_RX_PKT_CNT);
  SAND_HAL_FE2000_READ(userDeviceHandle, PC_TX_TEST_BYTE_CNT);
  SAND_HAL_FE2000_READ(userDeviceHandle, PC_TX_TEST_PKT_CNT);
  SAND_HAL_FE2000_READ(userDeviceHandle, PC_RX_TEST_BYTE_CNT);
  SAND_HAL_FE2000_READ(userDeviceHandle, PC_RX_TEST_PKT_CNT);

  SAND_HAL_FE2000_READ(userDeviceHandle, PC_ERROR0);
  SAND_HAL_FE2000_WRITE(userDeviceHandle, PC_ERROR0, uData);
  uData = SAND_HAL_FE2000_READ(userDeviceHandle, PC_ERROR1);
  SAND_HAL_FE2000_WRITE(userDeviceHandle, PC_ERROR1, uData);
  uData = SAND_HAL_FE2000_READ(userDeviceHandle, PC_INTERRUPT);
  SAND_HAL_FE2000_WRITE(userDeviceHandle, PC_INTERRUPT, uData);
  DEXIT();

}

void 
sbFe2000UtilReleaseSoftReset(sbhandle userDeviceHandle)
{
  uint32_t uData = 0;
  /* bring out of soft reset */
  DENTER();
  /* TODO: BOGUS */
  uData = SAND_HAL_FE2000_READ(userDeviceHandle, PC_RESET);
  uData = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PC_RESET, SOFT_RESET,uData, 0);
  uData = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PC_RESET, PC_CORE_RESET,uData, 0);
  uData = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PC_RESET, RESET_RXBUF,uData, 0);
  SAND_HAL_FE2000_WRITE(userDeviceHandle, PC_RESET, uData);
  SAND_HAL_FE2000_RMW_FIELD(userDeviceHandle, PC_CONFIG, CII_ENABLE, 0x1);
  SAND_HAL_FE2000_RMW_FIELD(userDeviceHandle, PC_CONFIG, CII_DISABLED_ACK, 0x1);
  DEXIT();
}



uint32_t
sbFe2000UtilAgmWrite(sbhandle userDeviceHandle, int32_t nAgmNum, int32_t nPort,
                 uint32_t uRegAddr, uint32_t uData)
{
  uint32_t uMemAccCtrl;
  uint32_t uAck;
  uint32_t uTimeOut;


  SAND_HAL_FE2000_WRITE_STRIDE(userDeviceHandle, AG, nAgmNum,
			AG_GPORT_MEM_ACC_DATA, uData);

  if ( nPort < SB_FE2000_MAX_AG_PORTS ) {
    uRegAddr += nPort << 12; /* PORT_REGTYPE */
  } else {
    uRegAddr |= 0x00080000;  /* GEN_REGTYPE */
  }

  uMemAccCtrl = 0;
  uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, AG_GPORT_MEM_ACC_CTRL,
				   ACK, uMemAccCtrl, 0x1);
  uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, AG_GPORT_MEM_ACC_CTRL,
				   REQ, uMemAccCtrl, 0x1);
  uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, AG_GPORT_MEM_ACC_CTRL,
				   RD_WR_N, uMemAccCtrl, 0x0);
  uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, AG_GPORT_MEM_ACC_CTRL,
				   ADDRESS, uMemAccCtrl, uRegAddr);
  SAND_HAL_FE2000_WRITE_STRIDE(userDeviceHandle, AG, nAgmNum,
			AG_GPORT_MEM_ACC_CTRL, uMemAccCtrl);

  uAck = 0;
  uTimeOut = 0;
  while ( !uAck && (100 > uTimeOut) ) {
    uAck=SAND_HAL_FE2000_GET_FIELD(userDeviceHandle, AG_GPORT_MEM_ACC_CTRL, ACK,
			    SAND_HAL_FE2000_READ_STRIDE(userDeviceHandle, AG,
						 nAgmNum, AG_GPORT_MEM_ACC_CTRL));
    thin_delay(500);
    uTimeOut++;
  }

  if ( uTimeOut == 100 ) {
    return SB_FE2000_STS_INIT_AGM_WRITE_TIMEOUT_ERR_K;
  }

  return SB_FE2000_STS_INIT_OK_K;
}

uint32_t
sbFe2000UtilAgmRead(sbhandle userDeviceHandle, int32_t nAgmNum, int32_t nPort,
                uint32_t uRegAddr, uint32_t *puData)
{

  uint32_t uMemAccCtrl;
  uint32_t uAck;
  uint32_t uTimeOut;

  if ( nPort < SB_FE2000_MAX_AG_PORTS ) {
    uRegAddr += nPort << 12; /* PORT_REGTYPE */
  } else {
    uRegAddr |= 0x00080000;  /* GEN_REGTYPE */
  }
  uMemAccCtrl = 0;
  uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, AG_GPORT_MEM_ACC_CTRL, ACK,
				   uMemAccCtrl, 0x1);
  uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, AG_GPORT_MEM_ACC_CTRL, REQ,
				   uMemAccCtrl, 0x1);
  uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, AG_GPORT_MEM_ACC_CTRL, RD_WR_N,
				   uMemAccCtrl, 0x1);
  uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, AG_GPORT_MEM_ACC_CTRL, ADDRESS,
				   uMemAccCtrl, uRegAddr);
  SAND_HAL_FE2000_WRITE_STRIDE(userDeviceHandle, AG, nAgmNum,
			AG_GPORT_MEM_ACC_CTRL, uMemAccCtrl);

  uAck = 0;
  uTimeOut = 0;
  while ( !uAck && (100 > uTimeOut) ) {
    uAck=SAND_HAL_FE2000_GET_FIELD(userDeviceHandle, AG_GPORT_MEM_ACC_CTRL, ACK,
			    SAND_HAL_FE2000_READ_STRIDE(userDeviceHandle, AG,
						 nAgmNum, AG_GPORT_MEM_ACC_CTRL));
    thin_delay(500);
    uTimeOut++;
  }

  if ( uTimeOut == 100 ) {
    return SB_FE2000_STS_INIT_AGM_READ_TIMEOUT_ERR_K;
  } else {
    *puData = SAND_HAL_FE2000_READ_STRIDE(userDeviceHandle, AG, nAgmNum,
				   AG_GPORT_MEM_ACC_DATA);
  }

  return SB_FE2000_STS_INIT_OK_K;
}

uint32_t
sbFe2000UtilAgmGportRead(sbhandle userDeviceHandle, uint32_t uAgmNum,
		     uint32_t uPort, uint32_t uRegAddr)
{

 uint32_t uReadData;
 uint32_t uMemAccCtrl;
 uint32_t uAck;
 uint32_t uTimeOut;

 if ( uPort < SB_FE2000_MAX_AG_PORTS ) {
   uRegAddr += uPort << 12; /* PORT_REGTYPE */
 } else {
   uRegAddr |= 0x00080000;  /* GEN_REGTYPE */
 }
 uMemAccCtrl = 0;
 uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, AG_GPORT_MEM_ACC_CTRL, ACK, uMemAccCtrl, 0x1);
 uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, AG_GPORT_MEM_ACC_CTRL, REQ, uMemAccCtrl, 0x1);
 uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, AG_GPORT_MEM_ACC_CTRL, RD_WR_N, uMemAccCtrl, 0x1);
 uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, AG_GPORT_MEM_ACC_CTRL, ADDRESS, uMemAccCtrl, uRegAddr);
 SAND_HAL_FE2000_WRITE_STRIDE(userDeviceHandle, AG, uAgmNum, AG_GPORT_MEM_ACC_CTRL,
                       uMemAccCtrl);

 uAck = 0;
 uTimeOut = 0;
 while ( !uAck && (100 > uTimeOut) ) {
   uAck=SAND_HAL_FE2000_GET_FIELD(userDeviceHandle, AG_GPORT_MEM_ACC_CTRL, ACK, 
			   SAND_HAL_FE2000_READ_STRIDE(userDeviceHandle,AG,
						uAgmNum,AG_GPORT_MEM_ACC_CTRL));
   uTimeOut++;
 }

 if ( uTimeOut == 100 ) {
   SB_ERROR("%s AG_GPORT_MEM_ACC_CTRL, timeout waiting for ACK on GPORT read.\n",__PRETTY_FUNCTION__);
   return (-1);
 } else {
   uReadData = SAND_HAL_FE2000_READ_STRIDE(userDeviceHandle,AG,uAgmNum,AG_GPORT_MEM_ACC_DATA);
   SB_LOGV3("%s AGM%d PORT%d Addr 0x%08x -- Data 0x%08x\n",__PRETTY_FUNCTION__,uAgmNum, uPort, uRegAddr, uReadData);
 }

 return uReadData;
}

uint32_t
sbFe2000UtilAgmGportWrite(sbhandle userDeviceHandle, uint32_t uPort, uint32_t nAgmNum, uint32_t uRegAddr,
		      uint32_t uData)
{
  uint32_t uMemAccCtrl;
  uint32_t uAck;
  uint32_t uTimeOut;

  DENTER();

  SAND_HAL_FE2000_WRITE_STRIDE(userDeviceHandle, AG, nAgmNum,AG_GPORT_MEM_ACC_DATA, uData);

 if ( uPort < SB_FE2000_MAX_AG_PORTS ) {
   uRegAddr += uPort << 12; /*  PORT_REGTYPE */
 } else {
   uRegAddr |= 0x00080000;  /* GEN_REGTYPE */
 }
 uMemAccCtrl = 0;
 uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, AG_GPORT_MEM_ACC_CTRL, ACK, uMemAccCtrl, 0x1);
 uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, AG_GPORT_MEM_ACC_CTRL, REQ, uMemAccCtrl, 0x1);
 uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, AG_GPORT_MEM_ACC_CTRL, RD_WR_N, uMemAccCtrl, 0x0);
 uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, AG_GPORT_MEM_ACC_CTRL, ADDRESS, uMemAccCtrl, uRegAddr);
 SAND_HAL_FE2000_WRITE_STRIDE(userDeviceHandle, AG, nAgmNum,AG_GPORT_MEM_ACC_CTRL, uMemAccCtrl);

 uAck = 0;
 uTimeOut = 0;
 while ( !uAck && (100 > uTimeOut) ) {
    uAck = SAND_HAL_FE2000_GET_FIELD(userDeviceHandle, AG_GPORT_MEM_ACC_CTRL, ACK,
			      SAND_HAL_FE2000_READ_STRIDE(userDeviceHandle, AG,
						   nAgmNum, AG_GPORT_MEM_ACC_CTRL));
   uTimeOut++;
 }
 if ( uTimeOut == 100 ) {
   SB_ERROR("%s:AG_GPORT_MEM_ACC_CTRL, timeout waiting for ACK on GPORT write.\n",__PRETTY_FUNCTION__);
   return (-1);
 }

 SB_LOGV3("AGM%d PORT%d Addr 0x%08x -- Data 0x%08x\n",nAgmNum, uPort, uRegAddr, uData);
 DEXIT();
 return 0;
}

uint32_t
sbFe2000UtilAgmMiiWrite(sbhandle userDeviceHandle, uint8_t bInternalPhy,uint32_t uPhyAddr, 
			uint32_t uRegAddr, uint32_t uData)
{
  uint32_t uMemAccCtrl;
  uint32_t uAck;
  uint32_t uTimeOut;

  uMemAccCtrl = 0;
  uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PC_MIIM_ADDRESS, MIIM_ADDRESS, uMemAccCtrl, uRegAddr);
  SAND_HAL_FE2000_WRITE(userDeviceHandle, PC_MIIM_ADDRESS, uMemAccCtrl);

  uMemAccCtrl = SAND_HAL_FE2000_READ(userDeviceHandle, PC_MIIM_CONTROL1);
  uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PC_MIIM_CONTROL1, MIIM_INT_PHY_SEL, uMemAccCtrl, bInternalPhy);
  uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PC_MIIM_CONTROL1, MIIM_PORT_NUM, uMemAccCtrl, uPhyAddr);
  uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PC_MIIM_CONTROL1, MIIM_PHY_WR_DATA, uMemAccCtrl, uData);
  SAND_HAL_FE2000_WRITE(userDeviceHandle, PC_MIIM_CONTROL1, uMemAccCtrl);

  uMemAccCtrl = SAND_HAL_FE2000_READ(userDeviceHandle, PC_MIIM_CONTROL0);
  uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PC_MIIM_CONTROL0, MIIM_WR_CMD_EN, uMemAccCtrl, 0x1);
  uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PC_MIIM_CONTROL0, OVER_RIDE_EXT_MDIO, uMemAccCtrl, 0x1);
  SAND_HAL_FE2000_WRITE(userDeviceHandle, PC_MIIM_CONTROL0, uMemAccCtrl);

  uAck = 0;
  uTimeOut = 0;
  while ( !uAck && (150 > uTimeOut) ) {
    uAck = SAND_HAL_FE2000_GET_FIELD(userDeviceHandle, PC_INTERRUPT, MIIM_OP_COMPLETE, SAND_HAL_FE2000_READ(userDeviceHandle, PC_INTERRUPT));
    thin_delay(250);
    uTimeOut++;
  }
  if ( uTimeOut == 150 ) {
    return SB_FE2000_STS_INIT_MII_WRITE_TIMEOUT_ERR;
  } else {
    /* clear interrupt status (W1TC) */
    SAND_HAL_FE2000_WRITE(userDeviceHandle, PC_INTERRUPT, (1<<SAND_HAL_FE2000_FIELD_SHIFT(userDeviceHandle, PC_INTERRUPT_MIIM, OP_COMPLETE)));
  }

  return SB_FE2000_STS_INIT_OK_K;
}

uint32_t
sbFe2000UtilAgmMiiRead(sbhandle userDeviceHandle, uint8_t bInternalPhy, uint32_t uPhyAddr, 
		       uint32_t uRegAddr, uint32_t *puData)
{
  uint32_t uMemAccCtrl;
  uint32_t uAck;
  uint32_t uTimeOut;


  uMemAccCtrl = 0;
  uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PC_MIIM_ADDRESS, MIIM_ADDRESS, uMemAccCtrl, uRegAddr);
  SAND_HAL_FE2000_WRITE(userDeviceHandle, PC_MIIM_ADDRESS, uMemAccCtrl);

  uMemAccCtrl = SAND_HAL_FE2000_READ(userDeviceHandle, PC_MIIM_CONTROL1);
  uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PC_MIIM_CONTROL1, MIIM_INT_PHY_SEL, uMemAccCtrl, bInternalPhy);
  uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PC_MIIM_CONTROL1, MIIM_PORT_NUM, uMemAccCtrl, uPhyAddr);
  SAND_HAL_FE2000_WRITE(userDeviceHandle, PC_MIIM_CONTROL1, uMemAccCtrl);

  uMemAccCtrl = SAND_HAL_FE2000_READ(userDeviceHandle, PC_MIIM_CONTROL0);
  uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PC_MIIM_CONTROL0, MIIM_RD_CMD_EN, uMemAccCtrl, 0x1);
  uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PC_MIIM_CONTROL0, OVER_RIDE_EXT_MDIO, uMemAccCtrl, 0x1);
  SAND_HAL_FE2000_WRITE(userDeviceHandle, PC_MIIM_CONTROL0, uMemAccCtrl);

  uAck = 0;
  uTimeOut = 0;
  while ( !uAck && (150 > uTimeOut) ) {
    uAck = SAND_HAL_FE2000_GET_FIELD(userDeviceHandle, PC_INTERRUPT, MIIM_OP_COMPLETE, SAND_HAL_FE2000_READ(userDeviceHandle, PC_INTERRUPT));
    thin_delay(250);
    uTimeOut++;
  }

  if ( uTimeOut == 150 ) {
    return SB_FE2000_STS_INIT_MII_READ_TIMEOUT_ERR;
  } else {
    /* clear interrupt status (W1TC) */
    SAND_HAL_FE2000_WRITE(userDeviceHandle, PC_INTERRUPT, (1<<SAND_HAL_FE2000_FIELD_SHIFT(userDeviceHandle, PC_INTERRUPT_MIIM, OP_COMPLETE)));

    *puData = SAND_HAL_FE2000_READ(userDeviceHandle, PC_MIIM_READ_DATA);
  }

  return SB_FE2000_STS_INIT_OK_K;
}

/* Canned routine to read from PCI interface to IIC on chip port */
unsigned int
sbFe2000UtilPciIICRead(int unit, unsigned int uRegAddr, unsigned int *puData)
{
  uint32_t uTimeOut;
  uint32_t uAck = 0;
  uint32_t uRegCtrl;

  /* Set Address in IIC */
  uRegCtrl = 0;
  uRegCtrl = SAND_HAL_FE2000_MOD_FIELD((sbhandle)unit, PC_IIC_ACC_CTRL, ACK, uRegCtrl, 0x1); /* Clear ACK */
  uRegCtrl = SAND_HAL_FE2000_MOD_FIELD((sbhandle)unit, PC_IIC_ACC_CTRL, REQ, uRegCtrl, 0x1);
  uRegCtrl = SAND_HAL_FE2000_MOD_FIELD((sbhandle)unit, PC_IIC_ACC_CTRL, RD_WR_N, uRegCtrl, 0x1);
  uRegCtrl = SAND_HAL_FE2000_MOD_FIELD((sbhandle)unit, PC_IIC_ACC_CTRL, ADDRESS, uRegCtrl, uRegAddr);

  SAND_HAL_FE2000_WRITE((sbhandle)unit, PC_IIC_ACC_CTRL, uRegCtrl);

  /* Wait for port ack */
  uTimeOut = 0;
  while ( !uAck && (SB_FE2000_IIC_WAIT_TIMEOUT > uTimeOut) ) {
    uAck = SAND_HAL_FE2000_GET_FIELD((sbhandle)unit, PC_IIC_ACC_CTRL, ACK, SAND_HAL_FE2000_READ((sbhandle)unit, PC_IIC_ACC_CTRL));
    thin_delay(250);
    uTimeOut++;
  }

  if ( uTimeOut == SB_FE2000_IIC_WAIT_TIMEOUT ) {
    return SB_FE2000_STS_INIT_IIC_READ_TIMEOUT_ERR;
  }

  *puData = SAND_HAL_FE2000_READ((sbhandle)unit, PC_IIC_ACC_DATA);

  return SB_FE2000_STS_INIT_OK_K;
};

/* Canned routine to write from PCI interface to IIC on chip port */
unsigned int
sbFe2000UtilPciIICWrite(int unit, unsigned int uRegAddr, unsigned int uData)
{
  uint32_t uTimeOut;
  uint32_t uAck = 0;
  uint32_t uRegCtrl;

  /* Set Address in IIC */
  uRegCtrl = 0;
  uRegCtrl = SAND_HAL_FE2000_MOD_FIELD((sbhandle)unit, PC_IIC_ACC_CTRL, ACK, uRegCtrl, 0x1); /* Clear ACK */
  uRegCtrl = SAND_HAL_FE2000_MOD_FIELD((sbhandle)unit, PC_IIC_ACC_CTRL, REQ, uRegCtrl, 0x1);
  uRegCtrl = SAND_HAL_FE2000_MOD_FIELD((sbhandle)unit, PC_IIC_ACC_CTRL, RD_WR_N, uRegCtrl, 0x0);
  uRegCtrl = SAND_HAL_FE2000_MOD_FIELD((sbhandle)unit, PC_IIC_ACC_CTRL, ADDRESS, uRegCtrl, uRegAddr);
  SAND_HAL_FE2000_WRITE((sbhandle)unit, PC_IIC_ACC_DATA, uData);
  SAND_HAL_FE2000_WRITE((sbhandle)unit, PC_IIC_ACC_CTRL, uRegCtrl);

  /* Wait for port ack */
  uTimeOut = 0;
  while ( !uAck && (SB_FE2000_IIC_WAIT_TIMEOUT > uTimeOut) ) {
    uAck = SAND_HAL_FE2000_GET_FIELD((sbhandle)unit, PC_IIC_ACC_CTRL, ACK, SAND_HAL_FE2000_READ((sbhandle)unit, PC_IIC_ACC_CTRL));
    thin_delay(250);
    uTimeOut++;
  }

  if ( uTimeOut == SB_FE2000_IIC_WAIT_TIMEOUT ) {
    return SB_FE2000_STS_INIT_IIC_WRITE_TIMEOUT_ERR;
  }

  return SB_FE2000_STS_INIT_OK_K;
}

uint32_t
sbFe2000UtilIICRead(	int unit,
			unsigned int slave_dev_addr,
			unsigned int reg_index,
			unsigned int *data)
{
	uint32_t status;
	uint32_t timeout;
	uint32_t ack;

	/* Clear Enable Reg */
	status = sbFe2000UtilPciIICWrite(unit, SB_FE2000_IIC_ENABLE_REG, 0x0);
	if( SB_FE2000_STS_INIT_OK_K != status ) {
		return status;
	}

	/* Set slave device address */
	status = sbFe2000UtilPciIICWrite(unit, SB_FE2000_IIC_CHIP_ADDR_REG,
						(slave_dev_addr << 1) & 0xff);
	if( SB_FE2000_STS_INIT_OK_K != status ) {
		return status;
	}

	/* Set register index */
	status = sbFe2000UtilPciIICWrite(unit, SB_FE2000_IIC_DATA_IN0_REG, reg_index);
	if( SB_FE2000_STS_INIT_OK_K != status ) {
		return status;
	}

	/* Set CNT Reg to transmit 1 byte and receive 1 byte */
	status = sbFe2000UtilPciIICWrite(unit, SB_FE2000_IIC_CNT_REG, 0x11);
	if( SB_FE2000_STS_INIT_OK_K != status ) {
		return status;
	}

	/* Set CTRL Reg for read transfer */
	status = sbFe2000UtilPciIICWrite(unit, SB_FE2000_IIC_CTRL_REG, 0x03);
	if( SB_FE2000_STS_INIT_OK_K != status ) {
		return status;
	}

	/* Set Enable Reg to start transfer */
	status = sbFe2000UtilPciIICWrite(unit, SB_FE2000_IIC_ENABLE_REG, 0x3);
	if( SB_FE2000_STS_INIT_OK_K != status ) {
		return status;
	}

	/*Wait for transfer complete */
	ack = 0;
	timeout = 0;
	while ( ((ack & 0x2) != 2) && (SB_FE2000_IIC_SLAVE_WAIT_TIMEOUT > timeout) ) {
		status = sbFe2000UtilPciIICRead(unit, SB_FE2000_IIC_ENABLE_REG, &ack);
		if( SB_FE2000_STS_INIT_OK_K != status ) {
			return status;
		}
		thin_delay(250);
		timeout++;
	}

	if ( timeout ==  SB_FE2000_IIC_SLAVE_WAIT_TIMEOUT) {
		return SB_FE2000_STS_INIT_IIC_READ_TIMEOUT_ERR;
	}

	status = sbFe2000UtilPciIICRead(unit, SB_FE2000_IIC_DATA_OUT0_REG, data);
	if( SB_FE2000_STS_INIT_OK_K != status ) {
		return status;
	}

	/* Clear Enable Reg */
	status = sbFe2000UtilPciIICWrite(unit, SB_FE2000_IIC_ENABLE_REG, 0x0);
	if( SB_FE2000_STS_INIT_OK_K != status ) {
		return status;
	}

	return SB_FE2000_STS_INIT_OK_K;
}

uint32_t
sbFe2000UtilIICWrite(	int unit,
			unsigned int slave_dev_addr,
			unsigned int reg_index,
			unsigned int data)
{
	uint32_t status;
	uint32_t timeout;
	uint32_t ack;

	/* Clear Enable Reg */
	status = sbFe2000UtilPciIICWrite(unit, SB_FE2000_IIC_ENABLE_REG, 0x0);
	if( SB_FE2000_STS_INIT_OK_K != status ) {
		return status;
	}

	/* Set slave device address */
	status = sbFe2000UtilPciIICWrite(unit, SB_FE2000_IIC_CHIP_ADDR_REG,
						(slave_dev_addr << 1) & 0xff);
	if( SB_FE2000_STS_INIT_OK_K != status ) {
		return status;
	}

	/* Set register index */
	status = sbFe2000UtilPciIICWrite(unit, SB_FE2000_IIC_DATA_IN0_REG, reg_index);
	if( SB_FE2000_STS_INIT_OK_K != status ) {
		return status;
	}

	/* Set data register */
	status = sbFe2000UtilPciIICWrite(unit, SB_FE2000_IIC_DATA_IN1_REG, data);
	if( SB_FE2000_STS_INIT_OK_K != status ) {
		return status;
	}

	/* Set CNT Reg to transfer 2 byte */
	status = sbFe2000UtilPciIICWrite(unit, SB_FE2000_IIC_CNT_REG, 2);
	if( SB_FE2000_STS_INIT_OK_K != status ) {
		return status;
	}

	/* Set CTRL Reg for write transfer */
	status = sbFe2000UtilPciIICWrite(unit, SB_FE2000_IIC_CTRL_REG, 0);
	if( SB_FE2000_STS_INIT_OK_K != status ) {
		return status;
	}

	/* Set Enable Reg to start transfer */
	status = sbFe2000UtilPciIICWrite(unit, SB_FE2000_IIC_ENABLE_REG, 0x3);
	if( SB_FE2000_STS_INIT_OK_K != status ) {
		return status;
	}

	/* Wait for transfer complete */
	ack = 0;
	timeout = 0;
	while ( ((ack & 0x2) != 2) && (timeout < SB_FE2000_IIC_SLAVE_WAIT_TIMEOUT) ) {
		status = sbFe2000UtilPciIICRead(unit, SB_FE2000_IIC_ENABLE_REG, &ack);
		if( SB_FE2000_STS_INIT_OK_K != status ) {
			return status;
		}
		timeout++;
	}

	if (timeout == SB_FE2000_IIC_SLAVE_WAIT_TIMEOUT) {
		SB_ERROR(",IIC Master timeout waiting for ACK on IIC Master read.\n");
		return SB_FE2000_STS_INIT_IIC_READ_TIMEOUT_ERR;
	}
	return SB_FE2000_STS_INIT_OK_K;
}

uint32_t 
sbFe2000UtilAgmMacErrorMaskCalcAddr(uint16_t nNum, uint16_t nPort,sbhandle userDeviceHandle)
{
  /* kpl fix me need below equivalent using userDeviceHandle */
  /* uint32_t uBaseAg = CA_BASE_AG(CA_BASE_ADDR,nNum);*/
  uint32_t uBaseAg = 0;
  uint32_t uStride = SAND_HAL_FE2000_REG_OFFSET(userDeviceHandle, AG_MAC1_ERROR_MASK) - SAND_HAL_FE2000_REG_OFFSET(userDeviceHandle, AG_MAC0_ERROR_MASK);
  uint32_t uAddr = uBaseAg + SAND_HAL_FE2000_REG_OFFSET(userDeviceHandle, AG_MAC0_ERROR_MASK) + (uStride * nPort);
  return uAddr;
}

/* gma - Oct 09 2006 - */
/* gma - Oct 09 2006 - Purpose:  This function will properly configure the CMU segment selected by the caller. */
/* gma - Oct 09 2006 -           The CMU contains 32 segments.  Each segment has a config, mmu segment, pci segment */
/* gma - Oct 09 2006 -           and limit associated with it that must be setup as well. */
/* gma - Oct 09 2006 - */
void
sbFe2000UtilSetupCmSegment(  sbhandle userDeviceHandle, uint32_t uCmSegment, uint32_t uCntrType, uint32_t uMmuDestBank, uint32_t uEjectLocation,
		 uint32_t uMmuBankBase, uint32_t uLimit, uint32_t uMemAddrConsumed ){


  uint8_t bEnabledCmSegments[SB_FE2000_MAX_CM_SEGMENTS] = {0};

  /* kpl - uncomment when below fixes are in place */
#if 0
  uint32_t uCmSegCfgStride = SAND_HAL_FE2000_REG_OFFSET( userDeviceHandle, CM_SEGMENT_TABLE_CONFIG1 ) - SAND_HAL_FE2000_REG_OFFSET( userDeviceHandle, CM_SEGMENT_TABLE_CONFIG0 );
  uint32_t uCmSegMmuStride = SAND_HAL_FE2000_REG_OFFSET( userDeviceHandle, CM_SEGMENT_TABLE_MMU1 ) - SAND_HAL_FE2000_REG_OFFSET( userDeviceHandle, CM_SEGMENT_TABLE_MMU0 );
  uint32_t uCmSegPciStride = SAND_HAL_FE2000_REG_OFFSET( userDeviceHandle, CM_SEGMENT_TABLE_PCI1 ) - SAND_HAL_FE2000_REG_OFFSET( userDeviceHandle, CM_SEGMENT_TABLE_PCI0 );
  uint32_t uCmSegLimStride = SAND_HAL_FE2000_REG_OFFSET( userDeviceHandle, CM_SEGMENT_TABLE_LIMIT1 ) - SAND_HAL_FE2000_REG_OFFSET( userDeviceHandle, CM_SEGMENT_TABLE_LIMIT0 );
  uint32_t uPciBase = 0;
#endif

  uint32_t uCmSegCfg = 0;
  uint32_t uMmSegCfg = 0;
  uint32_t uSegEnable = 0;

  uint8_t bOk = TRUE;
  uint32_t uIndex = 0;

  uint32_t uMaxAllowedVal[8] = { SB_FE2000_MAX_CM_SEGMENTS-1, SB_FE2000_NUM_TYPES_COUNTERS-1, 0x3, 0x1, 0x7fffff, 0x7fffff };
  uint32_t uParamValCheck[8] = { uCmSegment, uCntrType, uMmuDestBank, uEjectLocation, uMmuBankBase, uLimit };

  DENTER();
  for( uIndex = 0; (( uIndex < 8 ) && bOk ); uIndex++ ){
    if (uParamValCheck[uIndex] > uMaxAllowedVal[uIndex]) {
      bOk &= FALSE;
    }
  }
  uIndex--;

  /* gma - Oct 09 2006 - Ensure that all parameters contain a valid value. */
  if( bOk == FALSE ){
    SB_LOG("Function Parameter %d is invalid: 0x%x.", uIndex+1, uParamValCheck[uIndex] );
    SB_ASSERT( 0 );
  } else if( bEnabledCmSegments[uCmSegment] == FALSE ){ /*  gma - Oct 09 2006 - Is this segment enabled already? */
    SB_LOGV3("Creating a new segment - Id: %d\n", uCmSegment );

    /* gma - Oct 09 2006 - Setup the type and bank */
    uCmSegCfg = SAND_HAL_FE2000_MOD_FIELD( userDeviceHandle, CM_SEGMENT_TABLE_CONFIG0, BANK, uCmSegCfg, uMmuDestBank );
    uCmSegCfg = SAND_HAL_FE2000_MOD_FIELD( userDeviceHandle, CM_SEGMENT_TABLE_CONFIG0, CNTR_TYPE, uCmSegCfg, uCntrType );

    SB_LOGV3("CM_SEGMENT_TABLE_CONFIG%d Contents: 0x%x", uCmSegment, uCmSegCfg );
    SB_LOGV3("Intended contents: Bank( 0x%x )  -  Cntr_Type( 0x%x )  -  Eject( 0x%x )\n", uMmuDestBank, uCntrType, uEjectLocation );

    /* gma - Oct 09 2006 - Can only eject to MMU RAM if it is of type TURBO Counter */
    if(( uEjectLocation == SB_FE2000_CM_SRAM_EJECT ) && ( uCntrType != 2 /*SB_FE2000_CM_TURBO_COUNTER*/ )){
      SB_LOG("Only Turbo Counters can be ejected to MMU RAM. Selected counter type: %d", uCntrType );
      SB_ASSERT( 0 );
    } else {
      uCmSegCfg = SAND_HAL_FE2000_MOD_FIELD( userDeviceHandle, CM_SEGMENT_TABLE_CONFIG0, EJECT, uCmSegCfg, uEjectLocation );
    }

    /* gma - Oct 09 2006 - Write the config */
    /* kpl - needs fix to work with userDeviceHandle */
    /* SAND_HAL_FE2000_WRITE( CA_BASE_ADDR+(uCmSegment * uCmSegCfgStride), CA, CM_SEGMENT_TABLE_CONFIG0, uCmSegCfg ); */

    /* gma - Oct 09 2006 - Setup the Mmu segment config */
    /*  gma - Oct 26 2006 - No more segments in mmu */
    /*    uMmSegCfg = SAND_HAL_FE2000_MOD_FIELD( CA, CM_SEGMENT_TABLE_MMU0, MMU_SEGMENT, uMmSegCfg, uMmuSegment ); */
    uMmSegCfg = SAND_HAL_FE2000_MOD_FIELD( userDeviceHandle, CM_SEGMENT_TABLE_MMU0, BANK_BASE, uMmSegCfg, uMmuBankBase );

    SB_LOGV3("CM_SEGMENT_TABLE_MMU%d Contents: 0x%x", uCmSegment, uMmSegCfg );
    SB_LOGV3("Intended contents: Bank_Base( 0x%x )\n", uMmuBankBase );

    /* kpl - needs fix to work wtih userDeviceHandle */
    /* SAND_HAL_FE2000_WRITE( CA_BASE_ADDR+(uCmSegment * uCmSegMmuStride), CA, CM_SEGMENT_TABLE_MMU0, uMmSegCfg ); */

    /* ZSIM_UPGRADE_ONCE( this, "pci_base_addr is supposed to be 8-byte aligned so how does this afect the way memory is allocated?.\n" ); */
    SB_LOGV3("Allocating PCI memory of %d Bytes for segment %d.", (( uMemAddrConsumed + 1 ) * 8 ), uCmSegment );
    /* gma - Nov 27 2006 - Need x 8B addressable slots.  In the case where the base address is not */
    /* gma - Nov 27 2006 - 8B addressable, then need x+1 8B addressable slots. */

    /* kpl - needs fix */
    /* uint32_t uPciBaseAddr = (uint32_t) HalMalloc( (void*)this, (( uMemAddrConsumed + 1 ) * 8 )); */
    /* m_listCmPciBaseAddr.AddTail( (void*)uPciBaseAddr ); */
    /* uPciBase = uPciBaseAddr & 0xfffffff8; */
    /* SB_LOGV3("CM_SEGMENT_TABLE_PCI%d Contents: 0x%x", uCmSegment, uPciBase ); */

    /* kpl - needs fix to work with userDeviceHandle */
    /* SAND_HAL_FE2000_WRITE( CA_BASE_ADDR+(uCmSegment * uCmSegPciStride), CA, CM_SEGMENT_TABLE_PCI0, */
    /*		    SAND_HAL_FE2000_SET_FIELD( CA, CM_SEGMENT_TABLE_PCI0, PCI_BASE, ( uPciBase >> 3 ))); */

    /* gma - Nov 27 2006 - 0-based so need to subtract 1. */
    uLimit--;
    SB_LOGV3("CM_SEGMENT_TABLE_LIMIT%d Contents: 0x%x", uCmSegment, uLimit );
    /* gma - Oct 09 2006 - Setup the counterId limit for this segment */

    /* kpl - needs fix to work with userDeviceHandle */
    /* SAND_HAL_FE2000_WRITE( CA_BASE_ADDR+(uCmSegment * uCmSegLimStride), CA, CM_SEGMENT_TABLE_LIMIT0, uLimit ); */

    /* gma - Oct 09 2006 - Mark segment as enabled */
    bEnabledCmSegments[uCmSegment] = TRUE;

    /* gma - Oct 09 2006 - Enable the segment */
    uSegEnable = SAND_HAL_FE2000_READ( userDeviceHandle, CM_SEGMENT_ENABLE );
    SB_LOGV3("Existing CM_SEGMENT_ENABLE: 0x%x", uSegEnable );

    uSegEnable = uSegEnable | ( 0x1 << uCmSegment );
    SB_LOGV3("New CM_SEGMENT_ENABLE: 0x%x", uSegEnable );

    SAND_HAL_FE2000_WRITE( userDeviceHandle, CM_SEGMENT_ENABLE, uSegEnable );

  } else {
    SB_LOG("Invalid CMU Segment ID: %d.  Segment already configured & enabled.\n", uCmSegment );
    SB_ASSERT( 0 );
  }
}

uint32_t
sbFe2000UtilXgmWrite(sbhandle userDeviceHandle, int32_t nXgmNum, uint32_t uRegAddr,
                 uint32_t uDataHi, uint32_t uDataLo)
{

  uint32_t uMemAccCtrl;
  uint32_t uAck;
  uint32_t uTimeOut;

  SAND_HAL_FE2000_WRITE_STRIDE(userDeviceHandle, XG, nXgmNum,
			XG0_MAC_ACC_DATA_LO, uDataLo);
  SAND_HAL_FE2000_WRITE_STRIDE(userDeviceHandle, XG, nXgmNum,
			XG0_MAC_ACC_DATA_HI, uDataHi);

  uMemAccCtrl = 0;
  uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, XG0_MAC_ACC_CTRL, ACK,
				   uMemAccCtrl, 0x1);
  uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, XG0_MAC_ACC_CTRL, REQ,
				   uMemAccCtrl, 0x1);
  uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, XG0_MAC_ACC_CTRL, RD_WR_N,
				   uMemAccCtrl, 0x0);
  uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, XG0_MAC_ACC_CTRL, ADDRESS,
				   uMemAccCtrl, uRegAddr);
  SAND_HAL_FE2000_WRITE_STRIDE(userDeviceHandle, XG, nXgmNum, XG0_MAC_ACC_CTRL,
			uMemAccCtrl);

  uAck = 0;
  uTimeOut = 0;
  while ( !uAck && (100 > uTimeOut) ) {
    uAck = SAND_HAL_FE2000_GET_FIELD(userDeviceHandle, XG0_MAC_ACC_CTRL, ACK,
			      SAND_HAL_FE2000_READ_STRIDE(userDeviceHandle, XG,
						   nXgmNum, XG0_MAC_ACC_CTRL));
    thin_delay(500);
    uTimeOut++;
  }

  if ( uTimeOut == 100 ) {
    return SB_FE2000_STS_INIT_XGM_WRITE_TIMEOUT_ERR_K;
  }

  return SB_FE2000_STS_INIT_OK_K;
}

uint32_t
sbFe2000UtilXgmRead(sbhandle userDeviceHandle, int32_t nXgmNum,
                uint32_t uRegAddr, uint32_t *puDataHi, uint32_t *puDataLo)
{

  uint32_t uMemAccCtrl;
  uint32_t uAck;
  uint32_t uTimeOut;

  uMemAccCtrl = 0;
  uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, XG0_MAC_ACC_CTRL, ACK,
				   uMemAccCtrl, 0x1);
  uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, XG0_MAC_ACC_CTRL, REQ,
				   uMemAccCtrl, 0x1);
  uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, XG0_MAC_ACC_CTRL, RD_WR_N,
				   uMemAccCtrl, 0x1);
  uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, XG0_MAC_ACC_CTRL, ADDRESS,
				   uMemAccCtrl, uRegAddr);
  SAND_HAL_FE2000_WRITE_STRIDE(userDeviceHandle, XG, nXgmNum, XG0_MAC_ACC_CTRL,
			uMemAccCtrl);

  uAck = 0;
  uTimeOut = 0;
  while ( !uAck && (100 > uTimeOut) ) {
    uAck = SAND_HAL_FE2000_GET_FIELD(userDeviceHandle, XG0_MAC_ACC_CTRL, ACK,
			      SAND_HAL_FE2000_READ_STRIDE(userDeviceHandle, XG,
						   nXgmNum, XG0_MAC_ACC_CTRL));
    thin_delay(500);
    uTimeOut++;
  }

  if ( uTimeOut == 100 ) {
    return SB_FE2000_STS_INIT_XGM_READ_TIMEOUT_ERR_K;
  } else {
    *puDataLo = SAND_HAL_FE2000_READ_STRIDE(userDeviceHandle, XG, nXgmNum,
				     XG0_MAC_ACC_DATA_LO);
    *puDataHi = SAND_HAL_FE2000_READ_STRIDE(userDeviceHandle, XG, nXgmNum,
				     XG0_MAC_ACC_DATA_HI);
  }

  return SB_FE2000_STS_INIT_OK_K;
}

uint32_t
sbFe2000UtilXgmMiimRead(sbhandle userDeviceHandle, uint8_t bInternalPhy, uint8_t bClause45,
			uint32_t uDevAddr, uint32_t uPhyOrPortAddr, uint32_t uRegAddr,
			uint32_t *pData)
{
  uint32_t uMemAccCtrl;
  uint32_t uAck;
  uint32_t uTimeOut;

  DENTER();

  uMemAccCtrl = 0;
  uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PC_MIIM_ADDRESS, MIIM_ADDRESS, uMemAccCtrl, (bClause45) ? ((uDevAddr<<16) | uRegAddr) : uRegAddr);
  SAND_HAL_FE2000_WRITE(userDeviceHandle, PC_MIIM_ADDRESS, uMemAccCtrl);

  uMemAccCtrl = SAND_HAL_FE2000_READ(userDeviceHandle, PC_MIIM_CONTROL1);
  uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PC_MIIM_CONTROL1, MIIM_INT_PHY_SEL, uMemAccCtrl, bInternalPhy);
  uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PC_MIIM_CONTROL1, MIIM_C45_PHY_SEL, uMemAccCtrl, bClause45);
  uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PC_MIIM_CONTROL1, MIIM_HG_PHY_SEL, uMemAccCtrl, bClause45);
  uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PC_MIIM_CONTROL1, MIIM_PORT_NUM, uMemAccCtrl, uPhyOrPortAddr);
  SAND_HAL_FE2000_WRITE(userDeviceHandle, PC_MIIM_CONTROL1, uMemAccCtrl);

  uMemAccCtrl = SAND_HAL_FE2000_READ(userDeviceHandle, PC_MIIM_CONTROL0);
  uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PC_MIIM_CONTROL0, MIIM_WR_CMD_EN, uMemAccCtrl, 0x0);
  uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PC_MIIM_CONTROL0, MIIM_RD_CMD_EN, uMemAccCtrl, 0x1);
  uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PC_MIIM_CONTROL0, OVER_RIDE_EXT_MDIO, uMemAccCtrl, 0x1);
  SAND_HAL_FE2000_WRITE(userDeviceHandle, PC_MIIM_CONTROL0, uMemAccCtrl);

  uAck = 0;
  uTimeOut = 0;
  while ( !uAck && (250 > uTimeOut) ) {
    uAck = SAND_HAL_FE2000_GET_FIELD(userDeviceHandle, PC_INTERRUPT, MIIM_OP_COMPLETE, SAND_HAL_FE2000_READ(userDeviceHandle, PC_INTERRUPT));
    uTimeOut++;
  }
  if ( uTimeOut == 250 ) {
      SB_ERROR("%s PC_INTERRUPT-MIIM_OP_COMPLETE, timeout waiting for ACK on MDIO read.\n",__PRETTY_FUNCTION__);
      return (-1);
  } else {
      /* clear interrupt status (W1TC) */
    SAND_HAL_FE2000_WRITE(userDeviceHandle, PC_INTERRUPT, (1<<SAND_HAL_FE2000_FIELD_SHIFT(userDeviceHandle, PC_INTERRUPT, MIIM_OP_COMPLETE)));

    *pData = SAND_HAL_FE2000_READ(userDeviceHandle, PC_MIIM_READ_DATA);

    if (bClause45) {
	SB_LOGV3("%s MDIO read to Port%d Dev %d Addr 0x%08x -- Data 0x%08x\n",__PRETTY_FUNCTION__,uPhyOrPortAddr, uDevAddr, uRegAddr, *pData);
    } else {
	SB_LOGV3("%s MDIO read to Phy%d Addr 0x%08x -- Data 0x%08x\n",__PRETTY_FUNCTION__,uPhyOrPortAddr, uRegAddr, *pData);
    }
  }

  return 0;
}

uint32_t
sbFe2000UtilXgmMiimWrite(sbhandle userDeviceHandle,uint8_t bInternalPhy, uint8_t bClause45,
			 uint32_t uDevAddr, uint32_t uPhyOrPortAddr, uint32_t uRegAddr, 
			 uint32_t uData) 
{

    uint32_t uMemAccCtrl;
    uint32_t uAck;
    uint32_t uTimeOut;

    DENTER();
    uMemAccCtrl = 0;
    uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PC_MIIM_ADDRESS, MIIM_ADDRESS, uMemAccCtrl, (bClause45) ? ((uDevAddr<<16) | uRegAddr) : uRegAddr);
    SAND_HAL_FE2000_WRITE(userDeviceHandle, PC_MIIM_ADDRESS, uMemAccCtrl);

    uMemAccCtrl = SAND_HAL_FE2000_READ(userDeviceHandle, PC_MIIM_CONTROL1);
    uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PC_MIIM_CONTROL1, MIIM_INT_PHY_SEL, uMemAccCtrl, bInternalPhy);
    uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PC_MIIM_CONTROL1, MIIM_C45_PHY_SEL, uMemAccCtrl, bClause45);
    uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PC_MIIM_CONTROL1, MIIM_HG_PHY_SEL, uMemAccCtrl, bClause45);
    uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PC_MIIM_CONTROL1, MIIM_PORT_NUM, uMemAccCtrl, uPhyOrPortAddr);
    uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PC_MIIM_CONTROL1, MIIM_PHY_WR_DATA, uMemAccCtrl, uData);
    SAND_HAL_FE2000_WRITE(userDeviceHandle, PC_MIIM_CONTROL1, uMemAccCtrl);

    uMemAccCtrl = SAND_HAL_FE2000_READ(userDeviceHandle, PC_MIIM_CONTROL0);
    uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PC_MIIM_CONTROL0, MIIM_WR_CMD_EN, uMemAccCtrl, 0x1);
    uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PC_MIIM_CONTROL0, MIIM_RD_CMD_EN, uMemAccCtrl, 0x0);
    uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, PC_MIIM_CONTROL0, OVER_RIDE_EXT_MDIO, uMemAccCtrl, 0x1); /* Always 1 for normal operation */
    SAND_HAL_FE2000_WRITE(userDeviceHandle, PC_MIIM_CONTROL0, uMemAccCtrl);

    uAck = 0;
    uTimeOut = 0;
    while ( !uAck && (250 > uTimeOut) ) {
	uAck = SAND_HAL_FE2000_GET_FIELD(userDeviceHandle, PC_INTERRUPT, MIIM_OP_COMPLETE, SAND_HAL_FE2000_READ(userDeviceHandle, PC_INTERRUPT));
	uTimeOut++;
    }
    if ( uTimeOut == 250 ) {
	SB_ERROR("%s PC_INTERRUPT-MIIM_OP_COMPLETE, timeout waiting for ACK on MDIO write.\n",__PRETTY_FUNCTION__);
	return(-1);
    } else {
	/* clear interrupt status (W1TC) */
	SAND_HAL_FE2000_WRITE(userDeviceHandle, PC_INTERRUPT, (1<<SAND_HAL_FE2000_FIELD_SHIFT(userDeviceHandle, PC_INTERRUPT, MIIM_OP_COMPLETE)));
    }

    if (bClause45) {
	SB_LOGV3("%s MDIO write to Port%d Dev %d Addr 0x%08x -- Data 0x%08x\n",__PRETTY_FUNCTION__,uPhyOrPortAddr, uDevAddr, uRegAddr, uData);
    } else {
	SB_LOGV3("%s MDIO write to Phy%d Addr 0x%08x -- Data 0x%08x\n",__PRETTY_FUNCTION__,uPhyOrPortAddr, uRegAddr, uData);
    }

    return 0;
    DEXIT();
}

uint32_t
sbFe2000UtilXgmBigMacRead(sbhandle userDeviceHandle,uint32_t uXgmNum, uint32_t uRegAddr,
		      uint32_t *pDataHi,uint32_t *pDataLo)
{
  uint32_t uMemAccCtrl;
  uint32_t uAck;
  uint32_t uTimeOut;

  DENTER();

  uMemAccCtrl = 0;
  uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, XG0_MAC_ACC_CTRL, ACK, uMemAccCtrl, 0x1);
  uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, XG0_MAC_ACC_CTRL, REQ, uMemAccCtrl, 0x1);
  uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, XG0_MAC_ACC_CTRL, RD_WR_N, uMemAccCtrl, 0x1);
  uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, XG0_MAC_ACC_CTRL, ADDRESS, uMemAccCtrl, uRegAddr);
  SAND_HAL_FE2000_WRITE_STRIDE(userDeviceHandle, XG, uXgmNum,XG_MAC_ACC_CTRL, uMemAccCtrl);

  uAck = 0;
  uTimeOut = 0;
  while (!uAck && (100 > uTimeOut)) {
    uAck=SAND_HAL_FE2000_GET_FIELD(userDeviceHandle, XG_MAC_ACC_CTRL, ACK,
			    SAND_HAL_FE2000_READ_STRIDE(userDeviceHandle, XG,
						 uXgmNum, XG_MAC_ACC_CTRL));
    thin_delay(500);
    uTimeOut++;
  }

  if ( uTimeOut == 100 ) {
    SB_ERROR("%s XG%d_MAC_ACC_CTRL, timeout waiting for ACK\n", __PRETTY_FUNCTION__,uXgmNum);
    return (-1);
  } else {
    *pDataLo = SAND_HAL_FE2000_READ_STRIDE(userDeviceHandle, XG, uXgmNum,
				     XG_MAC_ACC_DATA_LO);
    *pDataHi = SAND_HAL_FE2000_READ_STRIDE(userDeviceHandle, XG, uXgmNum,
				     XG_MAC_ACC_DATA_HI);
    SB_LOGV3("%s:XGM%d Addr 0x%08x -- DataHi 0x%08x DataLo 0x%08x\n",__PRETTY_FUNCTION__,uXgmNum, uRegAddr,
	     *pDataHi, *pDataLo);
  }
  return 0;
  DEXIT();
}

uint32_t
sbFe2000UtilXgmBigMacWrite(sbhandle userDeviceHandle,uint32_t uXgmNum, uint32_t uRegAddr, 
		       uint32_t uDataHi, uint32_t uDataLo) 
{
  uint32_t uMemAccCtrl;
  uint32_t uAck;
  uint32_t uTimeOut;

  DENTER();

  SAND_HAL_FE2000_WRITE_STRIDE(userDeviceHandle, XG, uXgmNum,
			XG_MAC_ACC_DATA_LO, uDataLo);
  SAND_HAL_FE2000_WRITE_STRIDE(userDeviceHandle, XG, uXgmNum,
			XG_MAC_ACC_DATA_HI, uDataHi);

  uMemAccCtrl = 0;
  uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, XG_MAC_ACC_CTRL, ACK, uMemAccCtrl, 0x1);
  uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, XG_MAC_ACC_CTRL, REQ, uMemAccCtrl, 0x1);
  uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, XG_MAC_ACC_CTRL, RD_WR_N, uMemAccCtrl, 0x0);
  uMemAccCtrl = SAND_HAL_FE2000_MOD_FIELD(userDeviceHandle, XG_MAC_ACC_CTRL, ADDRESS, uMemAccCtrl, uRegAddr);
  SAND_HAL_FE2000_WRITE_STRIDE(userDeviceHandle,XG,uXgmNum,XG_MAC_ACC_CTRL,uMemAccCtrl);

  uAck = 0;
  uTimeOut = 0;
  while (!uAck && (100 > uTimeOut)) {
    uAck=SAND_HAL_FE2000_GET_FIELD(userDeviceHandle, XG_MAC_ACC_CTRL, ACK,
			    SAND_HAL_FE2000_READ_STRIDE(userDeviceHandle, XG,
						 uXgmNum, XG_MAC_ACC_CTRL));
    thin_delay(500);
    uTimeOut++;
  }

  if ( uTimeOut == 100 ) {
    SB_ERROR("%s XG%d_MAC_ACC_CTRL, timeout waiting for ACK\n", __PRETTY_FUNCTION__,uXgmNum);
    return (-1);
  } else {
    SB_LOGV3("%s XGM%d Addr 0x%08x -- DataHi 0x%08x DataLo 0x%08x\n",__PRETTY_FUNCTION__,uXgmNum, uRegAddr, uDataHi, uDataLo);
  }

  return 0;

  DEXIT();
}

/* temp helper routines */
void CaSetDataWord( uint32_t *pWord, uint8_t *pBytes) {
  *pWord = pBytes[0] | pBytes[1] << 8 | pBytes[2] << 16 | pBytes[3] << 24;
}

void CaSetDataBytes( uint32_t uData, uint8_t *pBytes) {
  pBytes[0]  = uData & 0xFF;
  pBytes[1] = (uData >> 8 ) & 0xFF;
  pBytes[2] = (uData >> 16 ) & 0xFF;
  pBytes[3] = (uData >> 24 ) & 0xFF;
}
