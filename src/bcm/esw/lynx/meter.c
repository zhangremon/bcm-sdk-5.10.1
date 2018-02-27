/*
 * $Id: meter.c 1.24 Broadcom SDK $
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
 */

#include <sal/types.h>
#include <sal/core/libc.h>

#include <shared/bitop.h>

#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/cm.h>
#include <soc/counter.h>
#include <soc/drv.h>
#include <soc/register.h>
#include <soc/memory.h>

#include <bcm/filter.h>
#include <bcm/error.h>
#include <bcm/meter.h>

#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/lynx.h>
#include <bcm_int/esw/meter.h>
#include <bcm_int/esw_dispatch.h>

#ifdef BCM_METER_SUPPORT

void _bcm_lynx_ffp_packet_collect(int unit);

#define	METER_LOCK(unit) \
	soc_mem_lock(unit,  XFILTER_METERINGm)
#define	METER_UNLOCK(unit) \
	soc_mem_unlock(unit, XFILTER_METERINGm)

#define	FFPCOUNTER_LOCK(unit) \
	soc_mem_lock(unit,  XFILTER_FFPCOUNTERSm)
#define	FFPCOUNTER_UNLOCK(unit) \
	soc_mem_unlock(unit, XFILTER_FFPCOUNTERSm)

#define LYNX_FFPCOUNTERS_WIDTH              40
#define LYNX_FFPCOUNTERS_INC_LO              0
#define LYNX_FFPCOUNTERS_INC_HI              0x100

static _b_meter_t	*meter_info[BCM_MAX_NUM_UNITS];
static _b_ffpcounter_t	*ffpcounter_info[BCM_MAX_NUM_UNITS];
static uint32		meter_allocated[BCM_MAX_NUM_UNITS];
static uint32		ffpcounter_allocated[BCM_MAX_NUM_UNITS];
static uint64           ffpcounter_incrementer[BCM_MAX_NUM_UNITS];

#define LYNX_NUM_BUCKET_SIZES 16

/* For LYNX, every token in the bucket is worth one bit. */
/* For metering purposes, kbits means 1000 bits.         */
static uint32 lynx_bucket_settings[LYNX_NUM_BUCKET_SIZES] = {
  0x00007FFF/1000,  /*  32K-1 tokens */
  0x0000FFFF/1000,  /*  64K-1 tokens */
  0x0001FFFF/1000,  /* 128K-1 tokens */
  0x0003FFFF/1000,  /* 256K-1 tokens */
  0x0007FFFF/1000,  /* 512K-1 tokens */
  0x000FFFFF/1000,  /*   1M-1 tokens */
  0x001FFFFF/1000,  /*   2M-1 tokens */
  0x003FFFFF/1000,  /*   4M-1 tokens */
  0x007FFFFF/1000,  /*   8M-1 tokens */
  0x00FFFFFF/1000,  /*  16M-1 tokens */
  0x01FFFFFF/1000,  /*  32M-1 tokens */
  0x03FFFFFF/1000,  /*  64M-1 tokens */
  0x03FFFFFF/1000,  /*  64M-1 tokens */
  0x03FFFFFF/1000,  /*  64M-1 tokens */
  0x03FFFFFF/1000,  /*  64M-1 tokens */
  0x03FFFFFF/1000   /*  64M-1 tokens */
};

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
_bcm_lynx_meter_size_get(int unit, int *size)
{
    if (meter_info[unit] == NULL) {
        *size = 0;
    } else {
        *size = meter_info[unit]->m_size;
    }
}

/*
 * Function:
 *	bcm_lynx_meter_init
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
bcm_lynx_meter_init(int unit)
{
    int pic;
    _b_meter_t *mi;

    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    soc_counter_extra_unregister(unit, _bcm_lynx_ffp_packet_collect);

    BCM_IF_ERROR_RETURN(bcm_lynx_ffpcounter_init(unit));

    if (meter_info[unit] == NULL) {
        meter_info[unit] = sal_alloc(sizeof(*mi), "meter");
        if (meter_info[unit] == NULL) {
            return (BCM_E_MEMORY);
        }
        sal_memset(meter_info[unit], 0, sizeof(*mi));
        meter_allocated[unit] = 0;
    }

    mi = meter_info[unit];

    SOC_BLOCK_ITER(unit, pic, SOC_BLK_ETHER) {
        mi->m_pic2meter[pic] = XFILTER_METERINGm;
        mi->m_pic2fpc[pic] = XFILTER_FFPOPPACKETCOUNTERSm;
    }

    mi->m_size = soc_mem_index_count(unit, XFILTER_METERINGm);

    if (NUM_XE_PORT(unit) > 0) {
        if (soc_property_get(unit, spn_BCM_METER_CLEAR_XE, 1)) {
            SOC_IF_ERROR_RETURN(soc_mem_clear(unit,
                                              XFILTER_METERINGm,
                                              COPYNO_ALL,
                                              TRUE));
            SOC_IF_ERROR_RETURN(soc_mem_clear(unit,
                                              XFILTER_FFPOPPACKETCOUNTERSm,
                                              COPYNO_ALL,
                                              TRUE));
        }
    }

    /* Serialize these so they appear as blocks to sal_alloc_stat */
    SOC_BLOCK_ITER(unit, pic, SOC_BLK_ETHER) {
        if (NULL == mi->m_allocated[pic]) {
            mi->m_allocated[pic] = 
                sal_alloc(SHR_BITALLOCSIZE(mi->m_size), "METER_ALLOC");
        }
        if (NULL == mi->m_allocated[pic]) {
            return (BCM_E_MEMORY);
        }
        METER_ZERO(unit, pic);
        METER_SET(unit, pic, 0);	/* Meter 0 should not be used */
    }

    SOC_BLOCK_ITER(unit, pic, SOC_BLK_ETHER) {
        if (NULL == mi->m_counters[pic]) {
            mi->m_counters[pic] = 
                sal_alloc(mi->m_size * sizeof (_counter_t), "METER_CTRS");
        }
        if (NULL == mi->m_counters[pic]) {
            return (BCM_E_MEMORY);
        }
        sal_memset(mi->m_counters[pic], 0,
                   mi->m_size * sizeof (_counter_t));
    }

    soc_counter_extra_register(unit, _bcm_lynx_ffp_packet_collect);

    COMPILER_64_SET(ffpcounter_incrementer[unit], LYNX_FFPCOUNTERS_INC_HI,
                    LYNX_FFPCOUNTERS_INC_LO);

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_lynx_meter_create
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
bcm_lynx_meter_create(int unit, bcm_port_t port, int *mid)
{
    uint64 tmp64;
    int pic, i, rv;
    xmetering_entry_t	meter;

    COMPILER_64_SET(tmp64, 0, 0);

    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port, &port));
    }

    METER_INIT(unit);
    pic = PIC(unit, port);
    METER_PIC(unit, pic);
    if (!mid) {
        return BCM_E_PARAM;
    }

    METER_LOCK(unit);

    for (i = 1; i < meter_info[unit]->m_size; i++) {
        if (!METER_ISSET(unit, pic, i)) {

            METER_SET(unit, pic, i);
            rv = soc_mem_read(unit,
			      meter_info[unit]->m_pic2meter[pic],
			      pic,
			      i,
			      &meter);
            if (rv < 0) {
                METER_CLR(unit, pic, i);
                METER_UNLOCK(unit);
                return (rv);
            }
            soc_XFILTER_METERINGm_field32_set(unit, &meter,
                                              BUCKETSIZEf, 
                                              0);
            soc_XFILTER_METERINGm_field32_set(unit, &meter,
                                              REFRESHCOUNTf, 
                                              0);
            rv = soc_mem_write(unit,
                               meter_info[unit]->m_pic2meter[pic],
                               pic,
                               i,
                               &meter);
            rv = bcm_lynx_ffppacketcounter_set(unit, port, i, tmp64);
            if (rv >= 0) {
                meter_info[unit]->m_alloc_count++;
                meter_allocated[unit]++;
                *mid = i;
                soc_cm_debug(DK_VERBOSE,
                             "bcm_meter_create: pic=%d id=%d\n",
                             pic, *mid);
            }
            METER_UNLOCK(unit);
            return (rv);
        }
    }

    METER_UNLOCK(unit);

    return (BCM_E_FULL);
}

/*
 * Function:
 *	bcm_lynx_meter_delete
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
bcm_lynx_meter_delete(int unit, bcm_port_t port, int mid)
{
    int pic;

    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port, &port));
    }

    soc_cm_debug(DK_VERBOSE,
                 "bcm_meter_delete: id=%d\n", mid);

    METER_INIT(unit);
    pic = PIC(unit, port);
    METER_PIC(unit, pic);
    METER_ID(unit, mid);
    METER_LOCK(unit);
    METER_CLR(unit, pic, mid);
    meter_info[unit]->m_alloc_count--;
    meter_allocated[unit]--;
    METER_UNLOCK(unit);

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_lynx_meter_delete_all
 * Description:
 *	Delete all meters and ffppacketcounters.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	None.
 */

int
bcm_lynx_meter_delete_all(int unit)
{
    int         pic;
    _b_meter_t *mi;

    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    soc_cm_debug(DK_VERBOSE, "bcm_meter_delete_all\n");

    METER_INIT(unit);

    METER_LOCK(unit);

    mi = meter_info[unit];

    SOC_BLOCK_ITER(unit, pic, SOC_BLK_ETHER) {
        if (mi->m_pic2meter[pic] == 0 ||
            mi->m_pic2fpc[pic] == 0 ||
            mi->m_allocated[pic] == NULL ||
            mi->m_counters[pic] == NULL) {
            continue;
        }

        METER_ZERO(unit, pic);
    }

    meter_allocated[unit] -= mi->m_alloc_count;
    mi->m_alloc_count = 0;

    METER_UNLOCK(unit);

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_lynx_meter_set
 * Description:
 *	Set the metering parameter.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      port - port number.
 *      mid - Meter ID.
 *      kbits_sec - meter rate.
 *      kbits_burst - meter max burst.
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	None.
 */

int
bcm_lynx_meter_set(int unit, bcm_port_t port, int mid, 
		   uint32 kbits_sec, uint32 kbits_burst)
{
    int rv = BCM_E_NONE;
    int pic;
    uint32 regval;
    xmetering_entry_t	meter;

    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port, &port));
    }

    pic = PIC(unit, port);

    soc_cm_debug(DK_VERBOSE,
                 "bcm_meter_set: pic=%d id=%d\n", pic, mid);

    METER_INIT(unit);
    METER_PIC(unit, pic);
    METER_ID(unit, mid);

    METER_LOCK(unit);
    rv = soc_mem_read(unit,
                      meter_info[unit]->m_pic2meter[pic],
                      pic,
                      mid,
                      &meter);
    if (rv < 0) {
        METER_UNLOCK(unit);
        return (rv);
    }

    soc_XFILTER_METERINGm_field32_set(unit, &meter, BUCKETSIZEf, 
				      _bcm_lynx_kbits_to_bucketsize(kbits_burst));

    /* Check kbits_sec upper limit prior to "granularization" */
    if (kbits_sec > (0xFFFFFFFF-999))
        kbits_sec = (0xFFFFFFFF-999);

    /* The granularity for the meter rate is 1,000,000 bits per second; */
    /* therefore the granularity of the kbits_sec parameter is 1000.    */
    /* The REFRESHCOUNT field is a 14-bit field, so check regval.       */
    if ((regval = (kbits_sec+999)/1000) > 0x3FFF)
        regval = 0x3FFF;
    soc_XFILTER_METERINGm_field32_set(unit, &meter, REFRESHCOUNTf,
                                      regval);
    rv = soc_mem_write(unit,
                       meter_info[unit]->m_pic2meter[pic],
                       pic,
                       mid,
                       &meter);

    METER_UNLOCK(unit);

    return (rv);
}

/*
 * Function:
 *	bcm_lynx_meter_get
 * Description:
 *	Get the metering parameter.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number.
 *      port - port number.
 *      mid - The meter ID.
 *      kbits_sec - (OUT) Pointer to meter rate.
 *      kbits_burst - (OUT) Pointer to meter max burst.
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	None.
 */

int
bcm_lynx_meter_get(int unit, bcm_port_t port, int mid, 
		   uint32 *kbits_sec, uint32 *kbits_burst)
{
    int rv = BCM_E_NONE;
    int pic;
    uint8 regval;
    xmetering_entry_t	meter;

    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port, &port));
    }

    pic = PIC(unit, port);

    soc_cm_debug(DK_VERBOSE,
                 "bcm_meter_get: pic=%d id=%d\n", pic, mid);

    METER_INIT(unit);
    METER_PIC(unit, pic);
    METER_ID(unit, mid);

    if (!kbits_sec || !kbits_burst) {
        return (BCM_E_PARAM);
    }

    METER_LOCK(unit);
    rv = soc_mem_read(unit,
                      meter_info[unit]->m_pic2meter[pic],
                      pic,
                      mid,
                      &meter);
    if (rv < 0) {
        METER_UNLOCK(unit);
        return (rv);
    }

    /* Convert the REFRESHCOUNT field in Mbits/sec (1,000,000 bits/sec) to */
    /* kbits/sec (1000 bits/sec) */
    *kbits_sec = 1000 *
        soc_XFILTER_METERINGm_field32_get(unit, &meter, REFRESHCOUNTf);
    /* Use table to convert BUCKETSIZE field to burst size in kbits. */
    regval =
        soc_XFILTER_METERINGm_field32_get(unit, &meter, BUCKETSIZEf);
    *kbits_burst = _bcm_lynx_bucketsize_to_kbits(regval);
    METER_UNLOCK(unit);

    return (rv);
}

/*
 * Function:
 *	bcm_lynx_ffppacketcounter_set
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
bcm_lynx_ffppacketcounter_set(int unit, bcm_port_t port, int mid, uint64 val)
{
    int rv = BCM_E_NONE;
    xffpoppacketcounters_entry_t	counter;
    int pic;
    uint64 new;
    uint32 buf[2];
    COUNTER_ATOMIC_DEF	s;

    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port, &port));
    }

    pic = PIC(unit, port);

    soc_cm_debug(DK_VERBOSE,
                 "bcm_ffppacketcounterr_set: pic=%d, id=%d\n", pic, mid);
    METER_INIT(unit);
    METER_PIC(unit, pic);
    METER_ID(unit, mid);

    COMPILER_64_MASK_CREATE(new, LYNX_FFPCOUNTERS_WIDTH, 0);
    COMPILER_64_AND(new, val);
    COMPILER_64_TO_32_LO(buf[0], new);
    COMPILER_64_TO_32_HI(buf[1], new);

    METER_LOCK(unit);
    rv = soc_mem_read(unit,
                      meter_info[unit]->m_pic2fpc[pic],
                      pic,
                      mid,
                      &counter);
    if (rv < 0) {
        METER_UNLOCK(unit);
        return (rv);
    }
    soc_XFILTER_FFPOPPACKETCOUNTERSm_field_set(unit, &counter, COUNTf, buf);
    rv = soc_mem_write(unit,
                       meter_info[unit]->m_pic2fpc[pic],
                       pic,
                       mid,
                       &counter);
    METER_UNLOCK(unit);

    COUNTER_ATOMIC_BEGIN(s);
    meter_info[unit]->m_counters[pic][mid].ctr_val = val;
    meter_info[unit]->m_counters[pic][mid].ctr64_prev = new;
    COUNTER_ATOMIC_END(s);

    return (rv);
}

/*
 * Function:
 *	_hw_ffppacketcounter_get
 * Description:
 *	Get the hardware ffppacketcounter value.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      pic - epic/gpic number.
 *      mid - The meter ID.
 *      val - (OUT) Pointer to store ffppacketcounter for return.
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	None.
 */

static int
_hw_ffppacketcounter_get(int unit, int pic, int mid, uint64 *val)
{
    int rv;
    xffpoppacketcounters_entry_t	counter;
    uint32 buf[2];

    rv = soc_mem_read(unit,
                      meter_info[unit]->m_pic2fpc[pic],
		      pic,
                      mid,
                      &counter);
    if (rv >= 0) {
        soc_XFILTER_FFPOPPACKETCOUNTERSm_field_get(unit, &counter, 
                                                   COUNTf, buf);
        COMPILER_64_SET(*val, buf[1], buf[0]);
    }

    return (rv);
}

/*
 * Function:
 *	bcm_lynx_ffppacketcounter_get
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
bcm_lynx_ffppacketcounter_get(int unit, bcm_port_t port, int mid, uint64 *val)
{
    int pic;
    COUNTER_ATOMIC_DEF	s;
    uint64 value;

    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port, &port));
    }

    pic = PIC(unit, port);

    soc_cm_debug(DK_VERBOSE,
                 "bcm_ffppacketcounterr_get: pic=%d, id=%d\n",
                 pic, mid);
    METER_INIT(unit);
    METER_PIC(unit, pic);
    METER_ID(unit, mid);

    if (!val) {
        return (BCM_E_PARAM);
    }

    COUNTER_ATOMIC_BEGIN(s);
    value = meter_info[unit]->m_counters[pic][mid].ctr_val;
    COUNTER_ATOMIC_END(s);

    *val = value;

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_lynx_ffpcounter_init
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
bcm_lynx_ffpcounter_init(int unit)
{
    int pic;
    _b_ffpcounter_t *fi;

    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    if (ffpcounter_info[unit] == NULL) {
        ffpcounter_info[unit] = sal_alloc(sizeof(*fi), "ffpctr");
        if (ffpcounter_info[unit] == NULL) {
            return BCM_E_MEMORY;
        }
        sal_memset(ffpcounter_info[unit], 0, sizeof(*fi));
        ffpcounter_allocated[unit] = 0;
    }

    fi = ffpcounter_info[unit];

    SOC_BLOCK_ITER(unit, pic, SOC_BLK_ETHER) {
        fi->fc_pic2fc[pic] = XFILTER_FFPCOUNTERSm;
    }

    fi->fc_size = soc_mem_index_count(unit, XFILTER_FFPCOUNTERSm);
    if (soc_property_get(unit, spn_BCM_METER_CLEAR_XE, 1)) {
        SOC_IF_ERROR_RETURN(soc_mem_clear(unit,
                                          XFILTER_FFPCOUNTERSm,
                                          COPYNO_ALL,
                                          TRUE));
    }

    /* Serialize these so they appear as blocks to sal_alloc_stat */
    SOC_BLOCK_ITER(unit, pic, SOC_BLK_ETHER) {
        if (NULL == fi->fc_allocated[pic]) {
            fi->fc_allocated[pic] = 
                sal_alloc(SHR_BITALLOCSIZE(fi->fc_size), "FFPCTR_ALLOC");
        }
        if (NULL == fi->fc_allocated[pic]) {
            return (BCM_E_MEMORY);
        }
        FFPCOUNTER_ZERO(unit, pic);
    }

    SOC_BLOCK_ITER(unit, pic, SOC_BLK_ETHER) {
        if (NULL == fi->fc_counters[pic]) {
            fi->fc_counters[pic] = 
                sal_alloc(fi->fc_size * sizeof (_counter_t), "FFPCTR_CTRS");
        }
        if (NULL == fi->fc_counters[pic]) {
            return (BCM_E_MEMORY);
        }
        sal_memset(fi->fc_counters[pic], 0,
                   fi->fc_size * sizeof (_counter_t));
    }

    return BCM_E_NONE;
}
/*
 * Function:
 *	bcm_lynx_ffpcounter_create
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
bcm_lynx_ffpcounter_create(int unit, bcm_port_t port, int *ffpcounterid)
{
    uint64 tmp64;
    int i, rv, pic;

    COMPILER_64_SET(tmp64, 0, 0);

    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port, &port));
    }

    FFPCOUNTER_INIT(unit);
    pic = PIC(unit, port);
    FFPCOUNTER_PIC(unit, pic);

    if (!ffpcounterid) {
        return BCM_E_PARAM;
    }

    FFPCOUNTER_LOCK(unit);

    for (i = 0; i < ffpcounter_info[unit]->fc_size; i++) {
        if (!FFPCOUNTER_ISSET(unit, pic, i)) {
            FFPCOUNTER_SET(unit, pic, i);
            rv = bcm_lynx_ffpcounter_set(unit, port, i, tmp64);
            if (rv >= 0) {
                ffpcounter_info[unit]->fc_alloc_count++;
                ffpcounter_allocated[unit]++;
                *ffpcounterid = i;
                soc_cm_debug(DK_VERBOSE,
                             "bcm_ffpcounter_create: pic=%d, id=%d\n",
                             pic, *ffpcounterid);
            }
            FFPCOUNTER_UNLOCK(unit);
            return (rv);
        }
    }

    FFPCOUNTER_UNLOCK(unit);

    return (BCM_E_FULL);
}

/*
 * Function:
 *	bcm_lynx_ffpcounter_delete
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
bcm_lynx_ffpcounter_delete(int unit, bcm_port_t port, int ffpcounterid)
{
    int pic;

    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port, &port));
    }

    pic = PIC(unit, port);

    soc_cm_debug(DK_VERBOSE,
                 "bcm_ffpcounter_delete: pic=%d, id=%d\n",
                 pic, ffpcounterid);
    FFPCOUNTER_INIT(unit);
    FFPCOUNTER_PIC(unit, pic);
    FFPCOUNTER_ID(ffpcounterid);
    FFPCOUNTER_LOCK(unit);
    FFPCOUNTER_CLR(unit, pic, ffpcounterid);
    ffpcounter_info[unit]->fc_alloc_count--;
    ffpcounter_allocated[unit]--;
    FFPCOUNTER_UNLOCK(unit);

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_strata_ffpcounter_delete_all
 * Description:
 *	Delete all ffpcounters.
 * Parameterss:
 *      unit - StrataSwitch PCI device unit number.
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	None.
 */

int
bcm_lynx_ffpcounter_delete_all(int unit)
{
    int              pic;
    _b_ffpcounter_t *fi;

    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    soc_cm_debug(DK_VERBOSE, "bcm_ffpcounter_delete_all\n");

    FFPCOUNTER_INIT(unit);

    FFPCOUNTER_LOCK(unit);

    fi = ffpcounter_info[unit];

    SOC_BLOCK_ITER(unit, pic, SOC_BLK_ETHER) {
        if (fi->fc_pic2fc[pic] == 0 ||
            fi->fc_allocated[pic] == NULL ||
            fi->fc_counters[pic] == NULL) {
            continue;
        }

        FFPCOUNTER_ZERO(unit, pic);
    }

    ffpcounter_allocated[unit] -= fi->fc_alloc_count;
    fi->fc_alloc_count = 0;

    FFPCOUNTER_UNLOCK(unit);

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_lynx_ffpcounter_set
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
bcm_lynx_ffpcounter_set(int unit, bcm_port_t port, int ffpcounterid, uint64 val)
{
    int rv = BCM_E_NONE;
    xffpcounters_entry_t	counter;
    int pic;
    uint64 new;
    uint32 buf[2];
    COUNTER_ATOMIC_DEF	s;

    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port, &port));
    }

    pic = PIC(unit, port);

    soc_cm_debug(DK_VERBOSE,
                 "bcm_ffpcounter_set: pic=%d, id=%d\n",
                 pic, ffpcounterid);
    FFPCOUNTER_INIT(unit);
    FFPCOUNTER_PIC(unit, pic);
    FFPCOUNTER_ID(ffpcounterid);

    COMPILER_64_MASK_CREATE(new, LYNX_FFPCOUNTERS_WIDTH, 0);
    COMPILER_64_AND(new, val);
    COMPILER_64_TO_32_LO(buf[0], new);
    COMPILER_64_TO_32_HI(buf[1], new);

    FFPCOUNTER_LOCK(unit);
    rv = soc_mem_read(unit,
                      ffpcounter_info[unit]->fc_pic2fc[pic],
                      pic,
                      ffpcounterid,
                      &counter);
    if (rv < 0) {
        FFPCOUNTER_UNLOCK(unit);
        return (rv);
    }
    soc_XFILTER_FFPCOUNTERSm_field_set(unit, &counter,
                                         COUNTf, buf);
    rv = soc_mem_write(unit,
                       ffpcounter_info[unit]->fc_pic2fc[pic],
                       pic,
                       ffpcounterid,
                       &counter);
    FFPCOUNTER_UNLOCK(unit);

    COUNTER_ATOMIC_BEGIN(s);
    ffpcounter_info[unit]->fc_counters[pic][ffpcounterid].ctr_val = val;
    ffpcounter_info[unit]->fc_counters[pic][ffpcounterid].ctr64_prev = new;
    COUNTER_ATOMIC_END(s);

    return (rv);
}

/*
 * Function:
 *	_hw_ffpcounter_get
 * Description:
 *	Get the ffpcounter value.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      pic - epic/gpict number.
 *      ffpcounterid - The ffpcounter ID.
 *      val - (OUT) Pointer to store ffppacketcounter for return.
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	None.
 */

static int
_hw_ffpcounter_get(int unit, int pic, int ffpcounterid, uint64 *val)
{
    int rv;
    xffpcounters_entry_t	counter;
    uint32 buf[2];

    rv = soc_mem_read(unit,
		      ffpcounter_info[unit]->fc_pic2fc[pic],
		      pic,
		      ffpcounterid,
		      &counter);
    if (rv >= 0) {
	soc_XFILTER_FFPCOUNTERSm_field_get(unit, &counter, COUNTf, buf);
        COMPILER_64_SET(*val, buf[1], buf[0]);
    }
    return (rv);
}

/*
 * Function:
 *	bcm_lynx_ffpcounter_get
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
bcm_lynx_ffpcounter_get(int unit, bcm_port_t port, int ffpcounterid, uint64 *val)
{
    COUNTER_ATOMIC_DEF	s;
    uint64 value;
    int pic;

    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port, &port));
    }

    pic = PIC(unit, port);

    FFPCOUNTER_INIT(unit);
    FFPCOUNTER_PIC(unit, pic);
    FFPCOUNTER_ID(ffpcounterid);
    if (!val) {
        return (BCM_E_PARAM);
    }

    COUNTER_ATOMIC_BEGIN(s);
    value = ffpcounter_info[unit]->fc_counters[pic][ffpcounterid].ctr_val;
    COUNTER_ATOMIC_END(s);

    *val = value;

    return BCM_E_NONE;
}

/*
 * Function:
 *	_bcm_lynx_ffp_packet_collect
 * Description:
 *	Callback routine executed each counter cycle to add up the
 *	64-bit software accumulated counter values.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	None.
 */

void
_bcm_lynx_ffp_packet_collect(int unit)
{
    int pic, i;
    uint64 new, prev, diff;
    uint64	*vptr;
    COUNTER_ATOMIC_DEF s;
    _b_meter_t		*mi;
    _b_ffpcounter_t	*fi;

    if (!ffpcounter_allocated[unit] && !meter_allocated[unit]) {
        return;
    }

    mi = meter_info[unit];
    fi = ffpcounter_info[unit];

    SOC_BLOCK_ITER(unit, pic, SOC_BLK_ETHER) {
        for (i = 1; i < mi->m_size; i++) {
            if (METER_ISSET(unit, pic, i)) {
                if (_hw_ffppacketcounter_get(unit, pic, i, &new) >= 0) {
                    prev = mi->m_counters[pic][i].ctr64_prev;
                    if (COMPILER_64_NE(new, prev)) {
                        vptr = &mi->m_counters[pic][i].ctr_val;
                        if (COMPILER_64_LT(new, prev)) {
                            COMPILER_64_ADD_64(new,
                                               ffpcounter_incrementer[unit]);
                        }
                        diff = new;
                        COMPILER_64_SUB_64(diff, prev);

                        COUNTER_ATOMIC_BEGIN(s);
                        COMPILER_64_ADD_64(*vptr, diff);
                        COUNTER_ATOMIC_END(s);
                        mi->m_counters[pic][i].ctr64_prev = new;
                    }
                }
            }
        }

        for (i = 0; i < fi->fc_size; i++) {
            if (FFPCOUNTER_ISSET(unit, pic, i)) {
                if (_hw_ffpcounter_get(unit, pic, i, &new) >= 0) {
                    prev = fi->fc_counters[pic][i].ctr64_prev;
                    if (COMPILER_64_NE(new, prev)) {
                        vptr = &fi->fc_counters[pic][i].ctr_val;
                        if (COMPILER_64_LT(new, prev)) {
                            COMPILER_64_ADD_64(new,
                                               ffpcounter_incrementer[unit]);
                        }
                        diff = new;
                        COMPILER_64_SUB_64(diff, prev);

                        COUNTER_ATOMIC_BEGIN(s);
                        COMPILER_64_ADD_64(*vptr, diff);
                        COUNTER_ATOMIC_END(s);
                        fi->fc_counters[pic][i].ctr64_prev = new;
                    }
                }
            }
        }
    }
}

/*
 * Function:
 *	_bcm_lynx_kbits_to_bucketsize
 * Description:
 *      Routine that converts desired burst size into Lynx style
 *      leaky bucketsize register value.
 * Parameters:
 *      kbits_burst - Desired size of maximum burst traffic
 * Returns:
 *	Register bits for BUCKETSIZEf field. 
 * Notes:
 *	None.
 */

uint8
_bcm_lynx_kbits_to_bucketsize(uint32 kbits_burst)
{
    uint8 i;
    for (i=0; i< LYNX_NUM_BUCKET_SIZES; i++) {
        if (kbits_burst <= lynx_bucket_settings[i])
	    break; 
    }
    if (i == LYNX_NUM_BUCKET_SIZES)
        i--;
    return i;
}

/*
 * Function:
 *	_bcm_lynx_kbits_to_bucketsize
 * Description:
 *      Routine that converts Lynx style leaky bucketsize 
 *      register value into its associated burst size.
 * Parameters:
 *      reg_val - BUCKETSIZEf field value
 * Returns:
 *	Max size burst traffic in kbits. 
 * Notes:
 *	None.
 */

uint32
_bcm_lynx_bucketsize_to_kbits(uint8 reg_val)
{
    return lynx_bucket_settings[reg_val];
}

#endif /* BCM_METER_SUPPORT */
