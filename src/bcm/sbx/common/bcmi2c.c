/*
 * $Id: bcmi2c.c 1.5 Broadcom SDK $
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
 * File:	bcmi2c.c
 * Purpose:	BCM I2C API
 */

#ifdef INCLUDE_I2C

#include <soc/drv.h>
#include <soc/i2c.h>

#include <bcm/error.h>
#include <bcm/bcmi2c.h>
#include <soc/bsc.h>
#include <sal/compiler.h>

int
bcm_sbx_i2c_open(int unit,
                 char *devname,
                 uint32 flags,
                 int speed)
{
#ifdef BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(unit)) {
        return soc_i2c_devopen(unit, devname, flags, speed);
    }
#endif
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_i2c_write(int unit,
                  int fd,
                  uint32 addr,
                  uint8 *data,
                  uint32 nbytes)
{
#ifdef BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(unit)) {
        if (soc_i2c_device(unit, fd)->driver == NULL) {
            return BCM_E_PARAM;
        }
        
        return soc_i2c_device(unit, fd)->driver->write(unit, fd,
                                                       addr, data, nbytes);
    }
#endif
    return BCM_E_UNAVAIL;

}

int
bcm_sbx_i2c_read(int unit,
                 int fd,
                 uint32 addr,
                 uint8 *data,
                 uint32 * nbytes)
{
#ifdef BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(unit)) {
        if (soc_i2c_device(unit, fd)->driver == NULL) {
            return BCM_E_PARAM;
        }
        
        return soc_i2c_device(unit, fd)->driver->read(unit, fd,
                                                      addr, data, nbytes);
    }
#endif
    return BCM_E_UNAVAIL;

}

int
bcm_sbx_i2c_ioctl(int unit,
                  int fd,
                  int opcode,
                  void *data,
                  int len)
{
#ifdef BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(unit)) {

        if (soc_i2c_device(unit, fd)->driver == NULL) {
            return BCM_E_PARAM;
        }
        
        return soc_i2c_device(unit, fd)->driver->ioctl(unit, fd, opcode,
                                                       data, len);
    }
#endif
    return BCM_E_UNAVAIL;

}

#ifdef BCM_FE2000_SUPPORT

int
bcm_fe2000_i2c_open(int unit, char *devname, uint32 flags, int speed)
{
	COMPILER_REFERENCE(flags);
	COMPILER_REFERENCE(speed);

	return soc_bsc_devopen(unit, devname);
}

int
bcm_fe2000_i2c_write(int unit, int fd, uint32 addr, uint8 *data, uint32 nbytes)
{
	/* nbytes is not used.  One byte a time */
	COMPILER_REFERENCE(nbytes);

	if ((soc_bsc_device(unit, fd) == NULL) || (soc_bsc_device(unit, fd)->driver == NULL)) {
		return BCM_E_PARAM;
	}
	return(soc_bsc_device(unit, fd)->driver->write(unit, fd, addr, (uint32)*data));
}

int
bcm_fe2000_i2c_read(int unit, int fd, uint32 addr, uint8 *data, uint32 *nbytes)
{
	uint32 tmp;
	int retv;

	/* nbytes is not used.  One byte a time */
	COMPILER_REFERENCE(nbytes);

	if ((soc_bsc_device(unit, fd) == NULL) || (soc_bsc_device(unit, fd)->driver == NULL)) {
		return BCM_E_PARAM;
	}
	retv = soc_bsc_device(unit, fd)->driver->read(unit, fd, addr, &tmp);
	if (retv < 0) {
		return retv;
	}
	*data = tmp & 0xff;
	return SOC_E_NONE;
}

int
bcm_fe2000_i2c_ioctl(int unit, int fd, int opcode, void *data, int len)
{
	if ((soc_bsc_device(unit, fd) == NULL) || (soc_bsc_device(unit, fd)->driver == NULL)) {
		return BCM_E_PARAM;
	}

	return soc_bsc_device(unit, fd)->driver->ioctl(unit, fd, opcode, data, len);
}
#endif /* BCM_FE2000_SUPPORT */
#endif /* INCLUDE_I2C */
