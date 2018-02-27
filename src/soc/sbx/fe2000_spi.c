
/*
 * $Id: fe2000_spi.c 1.7 Broadcom SDK $
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
 * ============================================================
 * == fe2000_spi.c - FE Initialization RESET and SPI4 Only      ==
 * ============================================================
 */
#include <soc/sbx/sbWrappers.h>
#include <soc/sbx/glue.h>
#include <soc/sbx/hal_ca_auto.h>
#include <soc/sbx/hal_c2_auto.h>
#include <soc/sbx/hal_ca_c2.h>
#include <soc/sbx/fe2000_spi.h>
#include <soc/drv.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_common.h>

#define HW_FE2000_10_USEC_K       (10000)
#define HW_FE2000_100_USEC_K     (100000)
#define HW_FE2000_1_MSEC_K      (1000000)
#define HW_FE2000_10_MSEC_K    (10000000)
#define HW_FE2000_100_MSEC_K  (100000000)
#define HW_FE2000_500_MSEC_K  (500000000)
#define HW_FE2000_1_SEC_K    (1000000000)

/* device conversion macros */
#define READ_STRIDE(_unit_, _block_, _id_, _reg_) \
        (SAND_HAL_FE2000_READ_STRIDE(_unit_, _block_, _id_, _reg_))

#define MOD_FIELD(_unit_, _block_, _field_, _regval_, _fieldval_) \
        (SAND_HAL_FE2000_MOD_FIELD(_unit_, _block_, _field_, _regval_, _fieldval_))

#define GET_FIELD(_unit_, _reg_, _field_, _regval_) \
        (SAND_HAL_FE2000_GET_FIELD(_unit_, _reg_, _field_, _regval_))

#define WRITE_STRIDE(_unit_, _block_, _id_, _regval_, _fieldval_) \
        (SAND_HAL_FE2000_WRITE_STRIDE(_unit_, _block_, _id_, _regval_, _fieldval_))

/*****************************************************************************
 * FUNCTION NAME:   hwFe2000Spi4TxForceTrainingOn()
 *
 * OVERVIEW:        Turn on training on the transmit SPI4 interface.
 *
 * ARGUMENTS:       userDeviceHandle - base address (handle) to device.
 *
 * RETURNS:
 *                  NONE
 *
 * DESCRIPTION:     This turns on tx_force_training in ST_CONFIG1 then delays
 *                  before returning.
 *
 * ASSUMPTIONS:	    NONE
 *
 * SIDE EFFECTS:    Delay.
 *
 *****************************************************************************/
void hwFe2000Spi4TxForceTrainingOn(sbhandle userDeviceHandle, uint32_t spi_ul)
{
    uint32_t reg;

    reg = READ_STRIDE(userDeviceHandle, ST, spi_ul, ST0_CONFIG0);
    reg = MOD_FIELD(userDeviceHandle, ST0_CONFIG0, TX_FORCE_TRAINING, reg, 1);
    WRITE_STRIDE(userDeviceHandle, ST, spi_ul, ST0_CONFIG0, reg);
    thin_delay(HW_FE2000_100_MSEC_K);
}

/*****************************************************************************
 * FUNCTION NAME:   hwFeSpi4TxForceTrainingOff()
 *
 * OVERVIEW:        Turn off training on the transmit SPI4 interface.
 *
 * ARGUMENTS:       userDeviceHandle - base address (handle) to device.
 *
 * RETURNS:
 *                  NONE
 *
 * DESCRIPTION:     This turns off tx_force_training in ST_CONFIG1 then delays
 *                  before returning.
 *
 * ASSUMPTIONS:	    NONE
 *
 * SIDE EFFECTS:    Delay.
 *
 *****************************************************************************/

void hwFe2000Spi4TxForceTrainingOff(sbhandle userDeviceHandle, uint32_t spi_ul)
{
    uint32_t reg;

    reg = READ_STRIDE(userDeviceHandle, ST, spi_ul, ST0_CONFIG0);
    reg = MOD_FIELD(userDeviceHandle, ST0_CONFIG0, TX_FORCE_TRAINING, reg, 0);
    WRITE_STRIDE(userDeviceHandle, ST, spi_ul, ST0_CONFIG0, reg);
    thin_delay(HW_FE2000_100_MSEC_K);
}

/*****************************************************************************
 * FUNCTION NAME:   hwFe2000Spi4RxStatus()
 *
 * OVERVIEW:        Determine health of SPI4 rx.
 *
 * ARGUMENTS:       userDeviceHandle - handle to device.
 *
 * RETURNS:
 *                  -1 if not trained or insync in SR_STATUS
 *                  SR_ERROR1 register value if non-zero (error)
 *                  0 if good.
 *
 * DESCRIPTION:     Checks SR_STATUS and SR_ERROR1 for SPI4 rx status.
 *                  Clears out SR_ERROR1 and rechecks after delay.
 *
 * ASSUMPTIONS:	    SPI4 interface is enabled.
 *
 * SIDE EFFECTS:    Delay
 *
 *****************************************************************************/

uint32_t hwFe2000Spi4RxStatus(sbhandle userDeviceHandle, uint32_t spi_ul)
{
    uint32_t reg, rxsync;

    /*
    ** Read regs  and clear out
    */
    reg = READ_STRIDE(userDeviceHandle, SR, spi_ul, SR0_STATUS);
    WRITE_STRIDE(userDeviceHandle, SR, spi_ul, SR0_STATUS, reg);

    reg = READ_STRIDE(userDeviceHandle, SR, spi_ul, SR0_ERROR);
    WRITE_STRIDE(userDeviceHandle, SR, spi_ul, SR0_ERROR, reg);

    /*
    ** delay
    */
    thin_delay(HW_FE2000_500_MSEC_K);

    /*
    ** Check sr_status to see whether trained and insync
    */
    reg = READ_STRIDE(userDeviceHandle, SR, spi_ul, SR0_STATUS);
    rxsync = GET_FIELD(userDeviceHandle, SR0_STATUS, RX_INSYNC, reg);

    if (rxsync == 0) {
	return(0xffffffff); /* to indicate an error */
    }

    reg = READ_STRIDE(userDeviceHandle, SR, spi_ul, SR0_ERROR);

    if (reg  != 0) {
      return(reg);
    }
    return(0);
}

/*****************************************************************************
 * FUNCTION NAME:   hwFe2000Spi4TxStatus()
 *
 * OVERVIEW:        Determine health of SPI4 tx.
 *
 * ARGUMENTS:       userDeviceHandle - handle to device.
 *
 * RETURNS:
 *                  -1 if not trained or insync in ST_STATUS
 *                  ST_ERROR register value if non-zero (error)
 *                  0 if good.
 *
 * DESCRIPTION:     Checks ST_STATUS and ST_ERROR for SPI4 tx status.
 *                  Clears out ST_ERROR and rechecks after delay.
 *
 * ASSUMPTIONS:	    SPI4 interface is enabled.
 *
 * SIDE EFFECTS:    3 second delay
 *
 *****************************************************************************/
uint32_t hwFe2000Spi4TxStatus(sbhandle userDeviceHandle, uint32_t spi_ul)
{
  uint32_t reg, txsync, locktx;

  reg = READ_STRIDE(userDeviceHandle, ST, spi_ul, ST0_STATUS);
  txsync = GET_FIELD(userDeviceHandle, ST0_STATUS, TX_INSYNC, reg);
  locktx = GET_FIELD(userDeviceHandle, ST0_STATUS, LOCKTX, reg);

  if (0 == txsync || 0 == locktx) {
    return(-1);
  }

  reg = READ_STRIDE(userDeviceHandle, ST, spi_ul, ST0_ERROR);
  WRITE_STRIDE(userDeviceHandle, ST, spi_ul, ST0_ERROR, reg);

  /*
  ** Re-read register to see if any new errors cropped up after delaying
  */
  thin_delay(HW_FE2000_500_MSEC_K);

  reg = READ_STRIDE(userDeviceHandle, ST, spi_ul, ST0_ERROR);

  if (reg != 0) {
    return(reg);
  }
  return(0);
}

void hwFe2000Spi4RxEnable(sbhandle userDeviceHandle, uint32_t enable_ul, uint32_t spi_ul)
{
    uint32_t sr_cfg0;

    sr_cfg0 = READ_STRIDE(userDeviceHandle, SR, spi_ul, SR0_CONFIG0);

    if (enable_ul){

        sr_cfg0 = MOD_FIELD(userDeviceHandle, SR0_CONFIG0, PC_RESET, sr_cfg0, 0);
	WRITE_STRIDE(userDeviceHandle, SR, spi_ul, SR0_CONFIG0, sr_cfg0);
	thin_delay(HW_FE2000_100_MSEC_K);

	sr_cfg0 = MOD_FIELD(userDeviceHandle, SR0_CONFIG0, RX_ENABLE, sr_cfg0, 1);
	WRITE_STRIDE(userDeviceHandle, SR, spi_ul, SR0_CONFIG0, sr_cfg0);
	thin_delay(HW_FE2000_100_MSEC_K);

    }
    else {
	sr_cfg0 = MOD_FIELD(userDeviceHandle, SR0_CONFIG0, RX_ENABLE, sr_cfg0, 0);
	WRITE_STRIDE(userDeviceHandle, SR, spi_ul, SR0_CONFIG0, sr_cfg0);
	thin_delay(HW_FE2000_100_MSEC_K);

	sr_cfg0 = MOD_FIELD(userDeviceHandle, SR0_CONFIG0, PC_RESET, sr_cfg0, 1);
	WRITE_STRIDE(userDeviceHandle, SR, spi_ul, SR0_CONFIG0, sr_cfg0);
	thin_delay(HW_FE2000_100_MSEC_K);

    }

}

void hwFe2000Spi4TxEnable(sbhandle userDeviceHandle, uint32_t enable_ul, uint32_t spi_ul)
{
   uint32_t reg, st_cfg1;

   st_cfg1 = READ_STRIDE(userDeviceHandle, ST, spi_ul, ST0_CONFIG0);

   if (enable_ul) {
       reg = MOD_FIELD(userDeviceHandle, ST0_CONFIG0, TX_ENABLE, st_cfg1, 1);
   }
   else {
     reg = MOD_FIELD(userDeviceHandle, ST0_CONFIG0, TX_ENABLE, st_cfg1, 0);
   }
   WRITE_STRIDE(userDeviceHandle, ST, spi_ul, ST0_CONFIG0, reg);
   thin_delay(HW_FE2000_100_MSEC_K);
}


/*****************************************************************************
 * FUNCTION NAME:   hwFe2000Spi4RxStatusNoDebounce()
 *
 * OVERVIEW:        Determine health of SPI4 rx.
 *
 * ARGUMENTS:       userDeviceHandle - handle to device
 *                  spi_ul           - SPI bus
 *
 * RETURNS:         -1 if not trained or insync in SR_STATUS
 *                  SR_ERROR register value if non-zero (error)
 *                  0 if good.
 *
 * DESCRIPTION:     Checks and clears SR_STATUS and SR_ERROR for SPI4 rx status.
 *
 * ASSUMPTIONS:	    SPI4 interface is enabled.
 *
 *****************************************************************************/
uint32_t hwFe2000Spi4RxStatusNoDebounce(sbhandle userDeviceHandle, uint32_t spi_ul)
{
    uint32_t sr_status, sr_error, rxsync;

    /*
     * Read regs and clear out
     * Need 2nd read in sr_error to clear any previous error.
     */
    sr_status = READ_STRIDE(userDeviceHandle, SR, spi_ul, SR0_STATUS);
    WRITE_STRIDE(userDeviceHandle, SR, spi_ul, SR0_STATUS, sr_status);

    sr_error = READ_STRIDE(userDeviceHandle, SR, spi_ul, SR0_ERROR);
    WRITE_STRIDE(userDeviceHandle, SR, spi_ul, SR0_ERROR, sr_error);
    sr_error = READ_STRIDE(userDeviceHandle, SR, spi_ul, SR0_ERROR);

    rxsync = GET_FIELD(userDeviceHandle, SR0_STATUS, RX_INSYNC, sr_status);

    if (rxsync == 0) {
        return(0xffffffff); /* to indicate an error */
    }

    if (sr_error != 0) {
      return(sr_error);
    }

    return(0);
}
