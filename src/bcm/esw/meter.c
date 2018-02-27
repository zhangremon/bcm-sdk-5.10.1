/*
 * $Id: meter.c 1.13 Broadcom SDK $
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
 * File: 	meter.c
 * Purpose: 	API for metering, ffppacketcounters and ffpcounters.
 *
 * BCM5690A0 errata (GNATS 2997): when using BCM5690 metering to
 * rate-limit a 10Mb stream, for refresh rate values 6 or less the
 * amount of metered data will come out 10% less than expected.
 * This is fixed in BCM5690A1.
 */

#include <sal/types.h>
#include <sal/core/libc.h>

#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/cm.h>
#include <soc/drv.h>
#include <soc/register.h>
#include <soc/memory.h>

#include <bcm/filter.h>
#include <bcm/error.h>
#include <bcm/meter.h>

#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/meter.h>
#include <bcm_int/esw/lynx.h>
#include <bcm_int/esw/strata.h>
#include <bcm_int/esw_dispatch.h>

/*
 * Function:
 *	_bcm_lynx_meter_size_get
 * Description:
 *	Retrieve the number of meters for the device.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      size - (OUT) size of meter pool
 * Returns:
 *	Nothing
 * Notes:
 *	None.
 */
void
_bcm_esw_meter_size_get(int unit, int *size)
{
#ifdef BCM_METER_SUPPORT
#ifdef BCM_LYNX_SUPPORT
    if (SOC_IS_LYNX(unit)) {
        _bcm_lynx_meter_size_get(unit, size);
        return;
    }
#endif /*BCM_LYNX_SUPPORT*/

#ifdef BCM_XGS12_SWITCH_SUPPORT
    if (SOC_IS_XGS12_SWITCH(unit)) {
        _bcm_strata_meter_size_get(unit, size);
        return;
    }
#endif /*BCM_XGS12_SWITCH_SUPPORT*/
#endif /* BCM_METER_SUPPORT */
    *size = 0;
}

/*
 * Function:
 *	bcm_esw_meter_init
 * Description:
 *	Initialize the metering function.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	None.
 */
int
bcm_esw_meter_init(int unit)
{
    if (!SOC_UNIT_VALID(unit)) {
	return BCM_E_UNIT;
    }

    if (!soc_feature(unit, soc_feature_filter_metering)) {
        return (BCM_E_UNAVAIL);
    }

#ifdef BCM_METER_SUPPORT
    else {
        return (mbcm_driver[unit]->mbcm_meter_init(unit));
    }
#endif

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_esw_meter_create
 * Description:
 *	Create a meter and ffppacketcounter.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      port - port number.
 *      mid - (OUT) The meter ID.
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	None.
 */

int
bcm_esw_meter_create(int unit, bcm_port_t port, int *mid)
{

    if (!SOC_UNIT_VALID(unit)) {
	return BCM_E_UNIT;
    }

    if (!soc_feature(unit, soc_feature_filter_metering)) {
        return (BCM_E_UNAVAIL);
    }

#ifdef BCM_METER_SUPPORT
    else {
        return (mbcm_driver[unit]->mbcm_meter_create(unit, port, mid));
    }
#endif

    return (BCM_E_FULL);
}

/*
 * Function:
 *	bcm_esw_meter_delete
 * Description:
 *	Delete a meter and ffppacketcounter.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      port - port number.
 *      mid - The meter ID.
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	None.
 */

int
bcm_esw_meter_delete(int unit, bcm_port_t port, int mid)
{
    if (!SOC_UNIT_VALID(unit)) {
	return BCM_E_UNIT;
    }

    if (!soc_feature(unit, soc_feature_filter_metering)) {
        return (BCM_E_UNAVAIL);
    }

#ifdef BCM_METER_SUPPORT
    else {
        return (mbcm_driver[unit]->mbcm_meter_delete(unit, port, mid));
    }
#endif

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_esw_meter_delete_all
 * Description:
 *	Delete all meters and ffppacketcounters on the selected unit.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	None.
 */

int
bcm_esw_meter_delete_all(int unit)
{
    if (!SOC_UNIT_VALID(unit)) {
	return BCM_E_UNIT;
    }

    if (!soc_feature(unit, soc_feature_filter_metering)) {
        return (BCM_E_UNAVAIL);
    }

#ifdef BCM_METER_SUPPORT
    else {
        return (mbcm_driver[unit]->mbcm_meter_delete_all(unit));
    }
#endif

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_esw_meter_set
 * Description:
 *	Set the metering parameter.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      port - port number.
 *      mid - Meter ID.
 *      kbits_sec - Data rate to associate with meter.
 *      kbits_burst - Max burst to associate with meter.
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	None.
 */

int
bcm_esw_meter_set(int unit, bcm_port_t port, int mid, 
	      uint32 kbits_sec, uint32 kbits_burst)
{
    if (!SOC_UNIT_VALID(unit)) {
	return BCM_E_UNIT;
    }

    if (!soc_feature(unit, soc_feature_filter_metering)) {
        return (BCM_E_UNAVAIL);
    }

#ifdef BCM_METER_SUPPORT
    else {
        return (mbcm_driver[unit]->mbcm_meter_set(unit, port, mid, 
						  kbits_sec, kbits_burst));
    }
#endif

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_esw_meter_get
 * Description:
 *	Get the metering parameter.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      port - port number.
 *      mid - The meter ID.
 *      kbits_sec - (OUT) Data rate setting of meter.
 *      kbits_burst - (OUT) Max burst setting of meter.
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	None.
 */

int
bcm_esw_meter_get(int unit, bcm_port_t port, int mid, 
	      uint32 *kbits_sec, uint32 *kbits_burst)
{
    if (!SOC_UNIT_VALID(unit)) {
	return BCM_E_UNIT;
    }

    if (!soc_feature(unit, soc_feature_filter_metering)) {
        return (BCM_E_UNAVAIL);
    }

#ifdef BCM_METER_SUPPORT
    else {
        return (mbcm_driver[unit]->mbcm_meter_get(unit, port, mid, 
						  kbits_sec, kbits_burst));
    }
#endif

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_esw_ffppacketcounter_set
 * Description:
 *	Given a port number, ffppacketcounter index number, the same as
 *	meter index, and a counter value, set both the hardware and
 *	software accumulated counters to the value.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      port - port number.
 *      mid - The meter ID.
 *      val - ffppacketcounter value.
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	None.
 */

int
bcm_esw_ffppacketcounter_set(int unit, bcm_port_t port, int mid, uint64 val)
{
    if (!SOC_UNIT_VALID(unit)) {
	return BCM_E_UNIT;
    }

    if (!soc_feature(unit, soc_feature_filter_metering)) {
        return (BCM_E_UNAVAIL);
    }

#ifdef BCM_METER_SUPPORT
    else {
        return (mbcm_driver[unit]->mbcm_ffppacketcounter_set(unit, port, 
                                                             mid, val));
    }
#endif

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_esw_ffppacketcounter_get
 * Description:
 *	Given a port number and ffppacketcounter index (same as meter index),
 *	fetch the 64-bit software-accumulated counter value.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      port - port number.
 *      mid - The meter ID.
 *      val - (OUT) Pointer to store ffppacketcounter for return.
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	None.
 */

int
bcm_esw_ffppacketcounter_get(int unit, bcm_port_t port, int mid, uint64 *val)
{
    if (!SOC_UNIT_VALID(unit)) {
	return BCM_E_UNIT;
    }

    if (!soc_feature(unit, soc_feature_filter_metering)) {
        return (BCM_E_UNAVAIL);
    }

#ifdef BCM_METER_SUPPORT
    else {
        return (mbcm_driver[unit]->mbcm_ffppacketcounter_get(unit, port, 
                                                             mid, val));
    }
#endif

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_esw_ffpcounter_init
 * Description:
 *	Initialize the ffpcounter function.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	None.
 */

int
bcm_esw_ffpcounter_init(int unit)
{
    if (!SOC_UNIT_VALID(unit)) {
	return BCM_E_UNIT;
    }

    if (!soc_feature(unit, soc_feature_filter_metering)) {
        return (BCM_E_UNAVAIL);
    }

#ifdef BCM_METER_SUPPORT
    else {
        return (mbcm_driver[unit]->mbcm_ffpcounter_init(unit));
    }
#endif

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_esw_ffpcounter_create
 * Description:
 *	Create a ffpcounter.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      port - port number.
 *      ffpcounterid - (OUT) The ffpcounter ID.
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	None.
 */

int
bcm_esw_ffpcounter_create(int unit, bcm_port_t port, int *ffpcounterid)
{
    if (!SOC_UNIT_VALID(unit)) {
	return BCM_E_UNIT;
    }

    if (!soc_feature(unit, soc_feature_filter_metering)) {
        return (BCM_E_UNAVAIL);
    }

#ifdef BCM_METER_SUPPORT
    else {
        return (mbcm_driver[unit]->mbcm_ffpcounter_create(unit, port,
                                                          ffpcounterid));
    }
#endif

    return (BCM_E_FULL);
}

/*
 * Function:
 *	bcm_esw_ffpcounter_delete
 * Description:
 *	Delete a ffpcounter.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      port - port number.
 *      ffpcounterid - The ffpcounter ID to be deleted.
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	None.
 */

int
bcm_esw_ffpcounter_delete(int unit, bcm_port_t port, int ffpcounterid)
{
    if (!SOC_UNIT_VALID(unit)) {
	return BCM_E_UNIT;
    }

    if (!soc_feature(unit, soc_feature_filter_metering)) {
        return (BCM_E_UNAVAIL);
    }

#ifdef BCM_METER_SUPPORT
    else {
        return (mbcm_driver[unit]->mbcm_ffpcounter_delete(unit, port,
                                                          ffpcounterid));
    }
#endif

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_esw_ffpcounter_delete_all
 * Description:
 *	Delete all ffpcounters on a unit.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	None.
 */

int
bcm_esw_ffpcounter_delete_all(int unit)
{
    if (!SOC_UNIT_VALID(unit)) {
	return BCM_E_UNIT;
    }

    if (!soc_feature(unit, soc_feature_filter_metering)) {
        return (BCM_E_UNAVAIL);
    }

#ifdef BCM_METER_SUPPORT
    else {
        return (mbcm_driver[unit]->mbcm_ffpcounter_delete_all(unit));
    }
#endif

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_esw_ffpcounter_set
 * Description:
 *	Given a port number, ffpcounter index number, and a counter
 *	value, set both the hardware and software-accumulated counters
 *	to the value.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      port - port number.
 *      ffpcounterid - Ffpcounter ID.
 *      val - The ffpcounter value to be set.
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	None.
 */

int
bcm_esw_ffpcounter_set(int unit, bcm_port_t port, int ffpcounterid, uint64 val)
{
    if (!SOC_UNIT_VALID(unit)) {
	return BCM_E_UNIT;
    }

    if (!soc_feature(unit, soc_feature_filter_metering)) {
        return (BCM_E_UNAVAIL);
    }

#ifdef BCM_METER_SUPPORT
    else {
        return (mbcm_driver[unit]->mbcm_ffpcounter_set(unit, port,
                                                       ffpcounterid, val));
    }
#endif

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_esw_ffpcounter_get
 * Description:
 *	Given a port number and ffpcounter index,
 *	fetch the 64-bit software-accumulated counter value.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      port - port number.
 *      ffpcounterid - The ffpcounter ID.
 *      val - (OUT) Pointer to store ffppacketcounter for return.
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	None.
 */

int
bcm_esw_ffpcounter_get(int unit, bcm_port_t port, int ffpcounterid, uint64 *val)
{
    if (!SOC_UNIT_VALID(unit)) {
	return BCM_E_UNIT;
    }

    if (!soc_feature(unit, soc_feature_filter_metering)) {
        return (BCM_E_UNAVAIL);
    }

#ifdef BCM_METER_SUPPORT
    else {
        return (mbcm_driver[unit]->mbcm_ffpcounter_get(unit, port,
                                                       ffpcounterid, val));
    }
#endif

    return BCM_E_NONE;
}
