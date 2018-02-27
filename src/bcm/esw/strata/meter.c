/*
 * $Id: meter.c 1.38 Broadcom SDK $
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
 * File:	meter.c
 * Purpose:	API for metering, ffppacketcounters and ffpcounters.
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
#include <bcm_int/esw/strata.h>
#include <bcm_int/esw/meter.h>
#include <bcm_int/esw_dispatch.h>

#ifdef BCM_METER_SUPPORT

void _bcm_strata_ffp_packet_collect(int unit);

#define	METER_LOCK(unit, pic) \
	soc_mem_lock(unit, meter_info[unit]->m_pic2meter[pic])
#define	METER_UNLOCK(unit, pic) \
	soc_mem_unlock(unit, meter_info[unit]->m_pic2meter[pic])

#define	FFPCOUNTER_LOCK(unit, pic) \
	soc_mem_lock(unit, meter_info[unit]->m_pic2fpc[pic])
#define	FFPCOUNTER_UNLOCK(unit, pic) \
	soc_mem_unlock(unit, meter_info[unit]->m_pic2fpc[pic])

#define DRACO_FFPPACKETCOUNTERS_WIDTH		17
#define DRACO_FFPPACKETCOUNTERS_WIDTH_MASK	0x1FFFF
#define DRACO15_FFPPACKETCOUNTERS_WIDTH		32
#define DRACO15_FFPPACKETCOUNTERS_WIDTH_MASK	0xFFFFFFFF
#define TUCANA_FFPPACKETCOUNTERS_WIDTH		32
#define TUCANA_FFPPACKETCOUNTERS_WIDTH_MASK	0xFFFFFFFF

#define FFPPACKETCOUNTERS_WIDTH(_u)			 \
	(SOC_IS_TUCANA(_u) ?				 \
	      TUCANA_FFPPACKETCOUNTERS_WIDTH :		 \
	      (SOC_IS_DRACO(_u) ?			 \
		DRACO_FFPPACKETCOUNTERS_WIDTH :		 \
		  DRACO15_FFPPACKETCOUNTERS_WIDTH))

#define FFPPACKETCOUNTERS_WIDTH_MASK(_u)		 \
	(SOC_IS_TUCANA(_u) ?				 \
	      TUCANA_FFPPACKETCOUNTERS_WIDTH_MASK :	 \
	      (SOC_IS_DRACO(_u) ?			 \
		DRACO_FFPPACKETCOUNTERS_WIDTH_MASK :	 \
		  DRACO15_FFPPACKETCOUNTERS_WIDTH_MASK))

#define FFPCOUNTERS_WIDTH(_u)				 \
	(SOC_IS_DRACO15(_u) ? 32 : 24)
#define FFPCOUNTERS_WIDTH_MASK(_u)			 \
	(SOC_IS_DRACO15(_u) ? 0xFFFFFFFF : 0xFFFFFF)

static _b_meter_t	*meter_info[BCM_MAX_NUM_UNITS];
static _b_ffpcounter_t	*ffpcounter_info[BCM_MAX_NUM_UNITS];
static uint32		meter_allocated[BCM_MAX_NUM_UNITS];
static uint32		ffpcounter_allocated[BCM_MAX_NUM_UNITS];


#define NUM_BUCKET_SIZES 8

/* 
 * Bucket Tables (for devices with 8 bucket sizes).
 *
 * These tables are indexed by the bucket size select field (3 bits),
 * and map each bucket size to an effective maximum burst size in 
 * kilobits (1000 bits). These tables are used to perform this
 * mapping function in both directions, i.e. burst-to-bucket_sel and
 * bucket_sel-to-burstsize.
 *
 * In general, each bucket (token counter) has a maximum possible value,
 * and a minimum "in-profile" threshold. An ethernet frame is allowed to
 * pass (either egress or ingress metering) when the token count is
 * between these two values, inclusive. Some devices have a minimum
 * in-profile threshold of zero, i.e. the token counter can go negative.
 *
 * Each token represents a certain amount of ethernet traffic. For every
 * applicable in-profile ethernet frame that is received or transmitted,
 * the token count is decremented by the appropriate amount. The token
 * count is incremented (refreshed) at a programmable rate.
 *
 * Egress burst control and ingress burst control mechanisms may differ
 * in maximum burst size and/or "traffic value" of each token. Therefore,
 * separate tables exist for egress and ingress buckets.
 *
 * The form for each bucket table entry is:
 * (max_value - min_in_profile_value + 1) * bits_per_token / 1000
 *
 */

/*
 * DRACO ports: 1 byte per token 
 *
 */

static uint32 draco_ingress_buckets[NUM_BUCKET_SIZES] = {
  (16383-12288+1)  * 8 / 1000,  /* 16K bucket */
  (20479-12288+1)  * 8 / 1000,  /* 20K bucket */
  (32767-12288+1)  * 8 / 1000,  /* 32K bucket */
  (45055-12288+1)  * 8 / 1000,  /* 44K bucket */
  (77823-12288+1)  * 8 / 1000,  /* 76K bucket */
  (143359-12288+1) * 8 / 1000,  /* 140K bucket */
  (274431-12288+1) * 8 / 1000,  /* 268K bucket */
  (524287-12288+1) * 8 / 1000   /* 512K bucket */
};

static uint32 draco_egress_buckets[NUM_BUCKET_SIZES] = {
  (16384-12288+1)  * 8 / 1000,  /* 16K bucket */
  (20480-12288+1)  * 8 / 1000,  /* 20K bucket */
  (28672-12288+1)  * 8 / 1000,  /* 28K bucket */
  (45056-12288+1)  * 8 / 1000,  /* 44K bucket */
  (77824-12288+1)  * 8 / 1000,  /* 76K bucket */
  (143360-12288+1) * 8 / 1000,  /* 140K bucket */
  (274432-12288+1) * 8 / 1000,  /* 268K bucket */
  (524288-12288+1) * 8 / 1000   /* 512K bucket */
};

/*
 * DRACO 1.5 ports: 1/16th byte per token (16 tokens per byte)
 * Ingress meter (FFP) and egress buckets have identical values.  
 *
 */

static uint32 draco15_buckets[NUM_BUCKET_SIZES] = {
  (65535-0+1)  * 8 / 16 / 1000,  
  (131071-0+1) * 8 / 16 / 1000,  
  (262143-0+1) * 8 / 16 / 1000,  
  (524287-0+1) * 8 / 16 / 1000,  
  (1048575-0+1) * 8 / 16 / 1000,  
  (2097151-0+1) * 8 / 16 / 1000,  
  (4194303-0+1) * 8 / 16 / 1000,  
  (8388607-0+1) * 8 / 16 / 1000   
};

/*
 * TUCANA ports: 1/125th byte per token (125 tokens per byte)
 * Ingress and egress buckets have identical values.  
 *
 */

static uint32 tucana_buckets[NUM_BUCKET_SIZES] = {
  (512000-0+1)   * 8 / 125 / 1000,
  (1024000-0+1)  * 8 / 125 / 1000,
  (2048000-0+1)  * 8 / 125 / 1000,
  (4096000-0+1)  * 8 / 125 / 1000,
  (8192000-0+1)  * 8 / 125 / 1000,
  (16384000-0+1) * 8 / 125 / 1000,
  (32768000-0+1) * 8 / 125 / 1000,
  (65536000-0+1) * 8 / 125 / 1000
};


/*
 * Function:
 *	_bcm_strata_meter_size_get
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
_bcm_strata_meter_size_get(int unit, int *size)
{
    if (meter_info[unit] == NULL) {
        *size = 0;
    } else {
        *size = meter_info[unit]->m_size;
    }
}

/*
 * Function:
 *	bcm_strata_meter_init
 * Description:
 *	Initialize the metering function.
 * Parameters:
 *	unit - StrataSwitch PCI device unit number.
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	None.
 */
int
bcm_strata_meter_init(int unit)
{
    int			pic;
    _b_meter_t		*mi;

    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    soc_counter_extra_unregister(unit, _bcm_strata_ffp_packet_collect);

    BCM_IF_ERROR_RETURN(bcm_strata_ffpcounter_init(unit));
    /* Allocate memory for metering module */
    if (meter_info[unit] == NULL) {
        meter_info[unit] = sal_alloc(sizeof(*mi), "meter");
        if (meter_info[unit] == NULL) {
            return BCM_E_MEMORY;
        }
        sal_memset(meter_info[unit], 0, sizeof(*mi));
        meter_allocated[unit] = 0;
    }

    mi = meter_info[unit];
    SOC_BLOCK_ITER(unit, pic, SOC_BLK_ETHER) {
        if (SOC_IS_TUCANA(unit)) {
#if defined(BCM_5665)
            mi->m_pic2meter[pic] = GFILTER_METERINGm;
            mi->m_pic2fpc[pic] = GFILTER_METERINGm;
#endif
#if defined(BCM_5695)
        } else if (SOC_IS_DRACO15(unit)) {
            mi->m_pic2meter[pic] = GFILTER_METERINGm;
            mi->m_pic2fpc[pic] = GFILTER_FFP_OUT_PROFILE_COUNTERSm;
#endif
        } else {
            switch (SOC_BLOCK_INFO(unit, pic).type) {
                case SOC_BLK_GPIC:
                    mi->m_pic2meter[pic] = GFILTER_METERINGm;
                    mi->m_pic2fpc[pic] = GFILTER_FFPPACKETCOUNTERSm;
                    break;
                case SOC_BLK_XPIC:
                default:
                    soc_cm_debug(DK_ERR,
                        "unit %d: metering not supported on %s ports",
                        unit, SOC_BLOCK_NAME(unit, pic));
                    return BCM_E_UNAVAIL;
            }
        }
    }
    
    mi->m_size = soc_mem_index_count(unit, GFILTER_METERINGm);
    
    if (SOC_IS_TUCANA(unit)) {
        
        if (((NUM_FE_PORT(unit) > 0) && 
            soc_property_get(unit, spn_BCM_METER_CLEAR_FE, 1)) ||
            ((NUM_GE_PORT(unit) > 0) && 
            soc_property_get(unit, spn_BCM_METER_CLEAR_GE, 1))) {
                SOC_IF_ERROR_RETURN
                    (soc_mem_clear(unit,
                    GFILTER_METERINGm,
                    COPYNO_ALL,
                    TRUE));
        }
    } else if (SOC_IS_DRACO15(unit)) { 
        if (soc_property_get(unit, spn_BCM_METER_CLEAR_GE, 1)) {
            SOC_IF_ERROR_RETURN
                (soc_mem_clear(unit,
                GFILTER_METERINGm,
                COPYNO_ALL,
                TRUE));
            SOC_IF_ERROR_RETURN
                (soc_mem_clear(unit,
                GFILTER_FFP_OUT_PROFILE_COUNTERSm,
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
    soc_counter_extra_register(unit, _bcm_strata_ffp_packet_collect);

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_strata_meter_create
 * Description:
 *	Create a meter and ffppacketcounter.
 * Parameters:
 *	unit - StrataSwitch PCI device unit number.
 *	port - port number.
 *	mid - (OUT) The meter ID.
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	None.
 */

int
bcm_strata_meter_create(int unit, bcm_port_t port, int *mid)
{
    uint64		tmp64;
    int			pic, i, rv;
    metering_entry_t	meter;

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
    if (mid == NULL) {
	return BCM_E_PARAM;
    }

    METER_LOCK(unit, pic);

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
		METER_UNLOCK(unit, pic);
		return (rv);
	    }
	    soc_GFILTER_METERINGm_field32_set(unit, &meter,
					      BUCKETSIZEf, 
					      0);
	    soc_GFILTER_METERINGm_field32_set(unit, &meter,
					      REFRESHCOUNTf, 
					      0);
	    rv = soc_mem_write(unit,
			       meter_info[unit]->m_pic2meter[pic],
			       pic,
			       i,
			       &meter);
	    rv = bcm_strata_ffppacketcounter_set(unit, port, i, tmp64);
	    if (rv >= 0) {
		meter_info[unit]->m_alloc_count++;
		meter_allocated[unit]++;
		*mid = i;
		soc_cm_debug(DK_VERBOSE,
			     "bcm_meter_create: pic=%d id=%d\n",
			     pic, *mid);
	    } else {
            METER_CLR(unit, pic, i);  
        }
	    METER_UNLOCK(unit, pic);
	    return (rv);
	}
    }

    METER_UNLOCK(unit, pic);

    return (BCM_E_FULL);
}

/*
 * Function:
 *	bcm_strata_meter_delete
 * Description:
 *	Delete a meter and ffppacketcounter.
 * Parameters:
 *	unit - StrataSwitch PCI device unit number.
 *	port - port number.
 *	mid - The meter ID.
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	None.
 */

int
bcm_strata_meter_delete(int unit, bcm_port_t port, int mid)
{
    int			pic;

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
    METER_LOCK(unit, pic);
    METER_CLR(unit, pic, mid);
    meter_info[unit]->m_alloc_count--;
    meter_allocated[unit]--;
    METER_UNLOCK(unit, pic);

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_strata_meter_delete_all
 * Description:
 *	Delete all meters and ffppacketcounters.
 * Parameters:
 *	unit - StrataSwitch PCI device unit number.
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	None.
 */

int
bcm_strata_meter_delete_all(int unit)
{
    int			pic;
    _b_meter_t		*mi;

    if (!SOC_UNIT_VALID(unit)) {
	return BCM_E_UNIT;
    }

    soc_cm_debug(DK_VERBOSE, "bcm_meter_delete_all\n");

    METER_INIT(unit);

    mi = meter_info[unit];

    SOC_BLOCK_ITER(unit, pic, SOC_BLK_ETHER) {
	METER_LOCK(unit, pic);

	if (mi->m_pic2meter[pic] == 0 ||
	    mi->m_pic2fpc[pic] == 0 ||
	    mi->m_allocated[pic] == NULL ||
	    mi->m_counters[pic] == NULL) {
	    continue;
	}

	METER_ZERO(unit, pic);

	METER_UNLOCK(unit, pic);
    }

    meter_allocated[unit] -= mi->m_alloc_count;
    mi->m_alloc_count = 0;

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_strata_meter_set
 * Description:
 *	Set the metering parameter.
 * Parameters:
 *	unit - StrataSwitch PCI device unit number.
 *	port - port number.
 *	mid - Meter ID.
 *	minfo - Metering parameter.
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	None.
 */

int
bcm_strata_meter_set(int unit, bcm_port_t port, int mid, 
		     uint32 kbits_sec, uint32 kbits_burst)
{
    int			rv = BCM_E_NONE;
    int			pic;
    metering_entry_t	meter;
    uint8		bucketsize;
    uint16		refreshcount;

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

    METER_LOCK(unit, pic);
    rv = soc_mem_read(unit,
		      meter_info[unit]->m_pic2meter[pic],
		      pic,
		      mid,
		      &meter);
    if (rv < 0) {
	METER_UNLOCK(unit, pic);
	return (rv);
    }

    bucketsize = _bcm_strata_kbits_to_bucketsize(unit, port, 
						 1, kbits_burst);

    refreshcount = _bcm_strata_kbps_to_refreshcount(unit, port, 
						    1, kbits_sec);

    /*
     * On Strata, the FE and GE metering fields are identical,
     * so the GFILTER_METERING macro works for both FE and GE.
     * On Draco, there is only GE metering.
     */

    soc_GFILTER_METERINGm_field32_set(unit, &meter, BUCKETSIZEf, bucketsize);

    soc_GFILTER_METERINGm_field32_set(unit, &meter, REFRESHCOUNTf,
				      refreshcount);
    rv = soc_mem_write(unit,
		       meter_info[unit]->m_pic2meter[pic],
		       pic,
		       mid,
		       &meter);

    METER_UNLOCK(unit, pic);

    return (rv);
}

/*
 * Function:
 *	bcm_strata_meter_get
 * Description:
 *	Get the metering parameter.
 * Parameters:
 *	unit - StrataSwitch PCI device unit number.
 *	port - port number.
 *	mid - The meter ID.
 *	minfo - (OUT) Pointer to store metering parameter.
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	None.
 */

int
bcm_strata_meter_get(int unit, bcm_port_t port, int mid, 
		     uint32 *kbits_sec, uint32 *kbits_burst)
{
    int			rv = BCM_E_NONE;
    int			pic;
    uint8		bucketsize;
    uint16		refreshcount;
    metering_entry_t	meter;

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

    if (!IS_FE_PORT(unit, port) && !IS_GE_PORT(unit, port))
	return BCM_E_UNAVAIL;

    METER_LOCK(unit, pic);
    rv = soc_mem_read(unit,
		      meter_info[unit]->m_pic2meter[pic],
		      pic,
		      mid,
		      &meter);
    if (rv < 0) {
	METER_UNLOCK(unit, pic);
	return (rv);
    }

    /* Convert the REFRESHCOUNT field to kbits/sec (1000 bits/sec). */
    refreshcount = soc_GFILTER_METERINGm_field32_get(unit, &meter, 
						     REFRESHCOUNTf);
    *kbits_sec = _bcm_strata_refreshcount_to_kbps(unit, port, 
						  1, refreshcount);
    /* Convert the BUCKETSIZE field to burst size in kbits. */
    bucketsize =
	soc_GFILTER_METERINGm_field32_get(unit, &meter, BUCKETSIZEf);
    *kbits_burst = _bcm_strata_bucketsize_to_kbits(unit, port, 
						   1, bucketsize);
    METER_UNLOCK(unit, pic);

    return (rv);
}

/*
 * Function:
 *	bcm_strata_ffppacketcounter_set
 * Description:
 *	Given a port number, ffppacketcounter index number, the same as
 *	meter index, and a counter value, set both the hardware and
 *	software accumulated counters to the value.
 * Parameters:
 *	unit - StrataSwitch PCI device unit number.
 *	port - port number.
 *	mid - The meter ID.
 *	val - ffppacketcounter value.
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	None.
 */

int
bcm_strata_ffppacketcounter_set(int unit, bcm_port_t port, int mid, uint64 val)
{
    int			rv = BCM_E_NONE;
    ffppacketcounters_entry_t counter;
    metering_entry_t	meter;
    int			pic, new;

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

    COMPILER_64_TO_32_LO(new, val);
    new &= FFPPACKETCOUNTERS_WIDTH_MASK(unit);

    METER_LOCK(unit, pic);

    if (SOC_IS_TUCANA(unit)) {
	rv = soc_mem_read(unit,
			  meter_info[unit]->m_pic2fpc[pic],
			  pic,
			  mid,
			  &meter);
	if (rv < 0) {
	    METER_UNLOCK(unit, pic);
	    return (rv);
	}
	soc_GFILTER_METERINGm_field32_set(unit, &meter,
					  OUTCOUNTERf,
					  new);
	rv = soc_mem_write(unit,
			   meter_info[unit]->m_pic2fpc[pic],
			   pic,
			   mid,
			   &meter);
    } else if (SOC_IS_DRACO15(unit)) {
	rv = soc_mem_read(unit,
			  meter_info[unit]->m_pic2fpc[pic],
			  pic,
			  mid,
			  &counter);
	if (rv < 0) {
	    METER_UNLOCK(unit, pic);
	    return (rv);
	}
	soc_GFILTER_FFP_OUT_PROFILE_COUNTERSm_field32_set(unit, &counter,
					  PACKET_COUNTf,
					  new);
	rv = soc_mem_write(unit,
			   meter_info[unit]->m_pic2fpc[pic],
			   pic,
			   mid,
			   &counter);
    } else {
	rv = soc_mem_read(unit,
			  meter_info[unit]->m_pic2fpc[pic],
			  pic,
			  mid,
			  &counter);
	if (rv < 0) {
	    METER_UNLOCK(unit, pic);
	    return (rv);
	}
	soc_GFILTER_FFPPACKETCOUNTERSm_field32_set(unit, &counter,
						   PACKET_COUNT_LOWf,
						   new);
	rv = soc_mem_write(unit,
			   meter_info[unit]->m_pic2fpc[pic],
			   pic,
			   mid,
			   &counter);
    }

    METER_UNLOCK(unit, pic);

    COUNTER_ATOMIC_BEGIN(s);
    meter_info[unit]->m_counters[pic][mid].ctr_val = val;
    meter_info[unit]->m_counters[pic][mid].ctr_prev = new;
    COUNTER_ATOMIC_END(s);

    return (rv);
}

/*
 * Function:
 *	_hw_ffppacketcounter_get
 * Description:
 *	Get the hardware ffppacketcounter value.
 * Parameters:
 *	unit - StrataSwitch PCI device unit number (driver internal).
 *	pic - epic/gpic number.
 *	mid - The meter ID.
 *	val - (OUT) Pointer to store ffppacketcounter for return.
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	None.
 */

static int
_hw_ffppacketcounter_get(int unit, int pic, int mid, uint32 *val)
{
    int			rv;
    ffppacketcounters_entry_t counter;
    metering_entry_t	meter;

    *val = 0;
    if (SOC_IS_TUCANA(unit)) {
	rv = soc_mem_read(unit,
			  meter_info[unit]->m_pic2fpc[pic],
			  pic,
			  mid,
			  &meter);
	if (rv >= 0) {
	    *val = soc_GFILTER_METERINGm_field32_get(unit, &meter,
						     OUTCOUNTERf);
	}
    } else if (SOC_IS_DRACO15(unit)) {
	rv = soc_mem_read(unit,
			  meter_info[unit]->m_pic2fpc[pic],
			  pic,
			  mid,
			  &counter);
	if (rv >= 0) {
	    *val = 
	      soc_GFILTER_FFP_OUT_PROFILE_COUNTERSm_field32_get(unit, 
								&counter,
								PACKET_COUNTf);
	}
    } else {
	rv = soc_mem_read(unit,
			  meter_info[unit]->m_pic2fpc[pic],
			  pic,
			  mid,
			  &counter);
	if (rv >= 0) {
	    *val = soc_GFILTER_FFPPACKETCOUNTERSm_field32_get(unit, &counter,
							  PACKET_COUNT_LOWf);
	}
    }
    return (rv);
}

/*
 * Function:
 *	bcm_strata_ffppacketcounter_get
 * Description:
 *	Given a port number and ffppacketcounter index (same as meter index),
 *	fetch the 64-bit software-accumulated counter value.
 * Parameters:
 *	unit - StrataSwitch PCI device unit number (driver internal).
 *	port - port number.
 *	mid - The meter ID.
 *	val - (OUT) Pointer to store ffppacketcounter for return.
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	None.
 */

int
bcm_strata_ffppacketcounter_get(int unit, bcm_port_t port,
				int mid, uint64 *val)
{
    int			pic;
    COUNTER_ATOMIC_DEF	s;
    uint64		value;

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

    if (val == NULL) {
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
 *	bcm_strata_ffpcounter_init
 * Description:
 *	Initialize the ffpcounter function.
 * Parameters:
 *	unit - StrataSwitch PCI device unit number.
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	None.
 */

int
bcm_strata_ffpcounter_init(int unit)
{
    int			pic;
    _b_ffpcounter_t	*fi;
    soc_mem_t		epic_fc_memory, gpic_fc_memory;

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

    /* Figure out the memory names here. */
    if (SOC_MEM_IS_VALID(unit, GFILTER_FFP_IN_PROFILE_COUNTERSm)) {
        epic_fc_memory = INVALIDm; /* Not used */ 
        gpic_fc_memory = GFILTER_FFP_IN_PROFILE_COUNTERSm;
    }
    else
    {
        epic_fc_memory = GFILTER_FFPCOUNTERSm;
        gpic_fc_memory = GFILTER_FFPCOUNTERSm;
    }

    SOC_BLOCK_ITER(unit, pic, SOC_BLK_ETHER) {
        switch (SOC_BLOCK_INFO(unit, pic).type) {
            case SOC_BLK_EPIC:
                fi->fc_pic2fc[pic] = epic_fc_memory;
                break;
            case SOC_BLK_GPIC:
                fi->fc_pic2fc[pic] = gpic_fc_memory;
                break;
            case SOC_BLK_XPIC:
            default:
                soc_cm_debug(DK_ERR,
                    "unit %d: ffp metering not supported on %s ports",
                    unit, SOC_BLOCK_NAME(unit, pic));
                return BCM_E_UNAVAIL;
        }
    }

    fi->fc_size = soc_mem_index_count(unit, gpic_fc_memory);

    if (NUM_FE_PORT(unit) > 0) {
        if (soc_property_get(unit, spn_BCM_METER_CLEAR_GE, 1) && 
            SOC_MEM_IS_VALID(unit, epic_fc_memory)) {
                SOC_IF_ERROR_RETURN(soc_mem_clear(unit, epic_fc_memory,
                    COPYNO_ALL, TRUE));
        }
    }
    if (NUM_GE_PORT(unit) > 0) {
        if (soc_property_get(unit, spn_BCM_METER_CLEAR_GE, 1) &&
            SOC_MEM_IS_VALID(unit, gpic_fc_memory)) {
                SOC_IF_ERROR_RETURN(soc_mem_clear(unit, gpic_fc_memory,
                    COPYNO_ALL, TRUE));
        }
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
        if (SOC_IS_DRACO15(unit)) {
            /* index 0 is reserved, defined as no-counter index */
            FFPCOUNTER_SET(unit, pic, 0);
        }
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
 *	bcm_strata_ffpcounter_create
 * Description:
 *	Create a ffpcounter.
 * Parameters:
 *	unit - StrataSwitch PCI device unit number.
 *	port - port number.
 *	ffpcounterid - (OUT) The ffpcounter ID.
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	None.
 */

int
bcm_strata_ffpcounter_create(int unit, bcm_port_t port, int *ffpcounterid)
{
    uint64		tmp64;
    int			i, rv, pic;

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

    pic = PIC(unit, port);
       
    FFPCOUNTER_LOCK(unit, pic);

    for (i = 0; i < ffpcounter_info[unit]->fc_size; i++) {
	if (!FFPCOUNTER_ISSET(unit, pic, i)) {
	    FFPCOUNTER_SET(unit, pic, i);
	    rv = bcm_strata_ffpcounter_set(unit, port, i, tmp64);
	    if (rv >= 0) {
		ffpcounter_info[unit]->fc_alloc_count++;
		ffpcounter_allocated[unit]++;
		*ffpcounterid = i;
		soc_cm_debug(DK_VERBOSE,
			     "bcm_ffpcounter_create: pic=%d, id=%d\n",
			     pic, *ffpcounterid);
	    }
	    FFPCOUNTER_UNLOCK(unit, pic);
	    return (rv);
	}
    }

    FFPCOUNTER_UNLOCK(unit, pic);

    return (BCM_E_FULL);
}

/*
 * Function:
 *	bcm_strata_ffpcounter_delete
 * Description:
 *	Delete a ffpcounter.
 * Parameters:
 *	unit - StrataSwitch PCI device unit number.
 *	port - port number.
 *	ffpcounterid - The ffpcounter ID to be deleted.
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	None.
 */

int
bcm_strata_ffpcounter_delete(int unit, bcm_port_t port, int ffpcounterid)
{
    int			pic;

    if (!SOC_UNIT_VALID(unit)) {
	return BCM_E_UNIT;
    }
    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port, &port));
    }

    if (!ffpcounterid && SOC_IS_DRACO15(unit)) {
	/* FFPCOUNTER index 0 is reserved; never make it available */
	return BCM_E_NONE;
    }

    pic = PIC(unit, port);

    soc_cm_debug(DK_VERBOSE,
		 "bcm_ffpcounter_delete: pic=%d, id=%d\n",
		 pic, ffpcounterid);
    FFPCOUNTER_INIT(unit);
    FFPCOUNTER_PIC(unit, pic);
    FFPCOUNTER_ID(ffpcounterid);
    FFPCOUNTER_LOCK(unit, pic);
    FFPCOUNTER_CLR(unit, pic, ffpcounterid);
    ffpcounter_info[unit]->fc_alloc_count--;
    ffpcounter_allocated[unit]--;
    FFPCOUNTER_UNLOCK(unit, pic);

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_strata_ffpcounter_delete_all
 * Description:
 *	Delete all ffpcounters.
 * Parameterss:
 *	unit - StrataSwitch PCI device unit number.
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	None.
 */

int
bcm_strata_ffpcounter_delete_all(int unit)
{
    int			pic;
    _b_ffpcounter_t	*fi;

    if (!SOC_UNIT_VALID(unit)) {
	return BCM_E_UNIT;
    }

    soc_cm_debug(DK_VERBOSE, "bcm_ffpcounter_delete_all\n");

    FFPCOUNTER_INIT(unit);

    fi = ffpcounter_info[unit];

    SOC_BLOCK_ITER(unit, pic, SOC_BLK_ETHER) {
	FFPCOUNTER_LOCK(unit, pic);

	if (fi->fc_pic2fc[pic] == 0 ||
	    fi->fc_allocated[pic] == NULL ||
	    fi->fc_counters[pic] == NULL) {
	    continue;
	}

	FFPCOUNTER_ZERO(unit, pic);
	if (SOC_IS_DRACO15(unit)) {
	    /* index 0 is reserved, defined as no-counter index */
	    FFPCOUNTER_SET(unit, pic, 0);
	}

	FFPCOUNTER_UNLOCK(unit, pic);
    }

    ffpcounter_allocated[unit] -= fi->fc_alloc_count;
    fi->fc_alloc_count = 0;

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_strata_ffpcounter_set
 * Description:
 *	Given a port number, ffpcounter index number, and a counter
 *	value, set both the hardware and software-accumulated counters
 *	to the value.
 * Parameters:
 *	unit - StrataSwitch PCI device unit number.
 *	port - port number.
 *	ffpcounterid - Ffpcounter ID.
 *	val - The ffpcounter value to be set.
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	None.
 */

int
bcm_strata_ffpcounter_set(int unit, bcm_port_t port,
			  int ffpcounterid, uint64 val)
{
    int			rv = BCM_E_NONE;
    ffpcounters_entry_t	counter;
    int			new, pic;
    COUNTER_ATOMIC_DEF	s;

    if (!SOC_UNIT_VALID(unit)) {
	return BCM_E_UNIT;
    }

    if (!ffpcounterid && SOC_IS_DRACO15(unit)) {
	/* BCM5695 FFPCOUNTER index 0 is reserved */
	return BCM_E_BADID;
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

    COMPILER_64_TO_32_LO(new, val);
    new &= FFPCOUNTERS_WIDTH_MASK(unit);

    FFPCOUNTER_LOCK(unit, pic);
    rv = soc_mem_read(unit,
		      ffpcounter_info[unit]->fc_pic2fc[pic],
		      pic,
		      ffpcounterid,
		      &counter);
    if (rv < 0) {
	FFPCOUNTER_UNLOCK(unit, pic);
	return (rv);
    }
    soc_mem_field32_set(unit, ffpcounter_info[unit]->fc_pic2fc[pic], 
			&counter, COUNT_VALUEf, new);
    rv = soc_mem_write(unit,
		       ffpcounter_info[unit]->fc_pic2fc[pic],
		       pic,
		       ffpcounterid,
		       &counter);
    FFPCOUNTER_UNLOCK(unit, pic);

    COUNTER_ATOMIC_BEGIN(s);
    ffpcounter_info[unit]->fc_counters[pic][ffpcounterid].ctr_val = val;
    ffpcounter_info[unit]->fc_counters[pic][ffpcounterid].ctr_prev = new;
    COUNTER_ATOMIC_END(s);

    return (rv);
}

/*
 * Function:
 *	_hw_ffpcounter_get
 * Description:
 *	Get the ffpcounter value.
 * Parameters:
 *	unit - StrataSwitch PCI device unit number (driver internal).
 *	pic - epic/gpict number.
 *	ffpcounterid - The ffpcounter ID.
 *	val - (OUT) Pointer to store ffppacketcounter for return.
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	None.
 */

static int
_hw_ffpcounter_get(int unit, int pic, int ffpcounterid, uint32 *val)
{
    int			rv;
    ffpcounters_entry_t	counter;

    rv = soc_mem_read(unit,
		      ffpcounter_info[unit]->fc_pic2fc[pic],
		      pic,
		      ffpcounterid,
		      &counter);
    if (rv >= 0) {
	*val = soc_mem_field32_get(unit, 
				   ffpcounter_info[unit]->fc_pic2fc[pic], 
				   &counter, COUNT_VALUEf);
    } else {
        *val = 0;
    }

    return (rv);
}

/*
 * Function:
 *	bcm_strata_ffpcounter_get
 * Description:
 *	Given a port number and ffpcounter index,
 *	fetch the 64-bit software-accumulated counter value.
 * Parameters:
 *	unit - StrataSwitch PCI device unit number (driver internal).
 *	port - port number.
 *	ffpcounterid - The ffpcounter ID.
 *	val - (OUT) Pointer to store ffppacketcounter for return.
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	None.
 */

int
bcm_strata_ffpcounter_get(int unit, bcm_port_t port,
			  int ffpcounterid, uint64 *val)
{
    COUNTER_ATOMIC_DEF	s;
    uint64		value;
    int			pic;

    if (!SOC_UNIT_VALID(unit)) {
	return BCM_E_UNIT;
    }

    if (!ffpcounterid && SOC_IS_DRACO15(unit)) {
	/* BCM5695 FFPCOUNTER index 0 is reserved */
	return BCM_E_BADID;
    }
    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_port_local_get(unit, port, &port));
    }

    pic = PIC(unit, port);

    FFPCOUNTER_INIT(unit);
    FFPCOUNTER_PIC(unit, pic);
    FFPCOUNTER_ID(ffpcounterid);
    if (val == NULL) {
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
 *	_bcm_strata_ffp_packet_collect
 * Description:
 *	Callback routine executed each counter cycle to add up the
 *	64-bit software accumulated counter values.
 * Parameters:
 *	unit - StrataSwitch PCI device unit number (driver internal).
 * Returns:
 *	BCM_E_XXX
 * Notes:
 *	None.
 */

void
_bcm_strata_ffp_packet_collect(int unit)
{
    int			pic, i;
    uint32              new, prev;
    uint64		*vptr, diff64, tmp64;
    COUNTER_ATOMIC_DEF	s;
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
                    /* Convenience variables */
                    prev = mi->m_counters[pic][i].ctr_prev;
                    vptr = &mi->m_counters[pic][i].ctr_val;
                    if (new != prev) {
                        /* Prepare 64-bit counter diff */
                        COMPILER_64_SET(diff64, 0, new);
                        /* If hardware wrapped, add counter size */
                        if (new < prev) {
                            COMPILER_64_SET(tmp64, 0, 1);
                            COMPILER_64_SHL(tmp64, FFPPACKETCOUNTERS_WIDTH(unit));
                            COMPILER_64_ADD_64(diff64, tmp64);
                        }
                        /* Subtract previous hardware counter value */
                        COMPILER_64_SUB_32(diff64, prev);

                        /* Add 64-bit hardware counter diff */
                        COUNTER_ATOMIC_BEGIN(s);
                        COMPILER_64_ADD_64(*vptr, diff64);
                        COUNTER_ATOMIC_END(s);

                        /* Update cached hardware counter */
                        mi->m_counters[pic][i].ctr_prev = new;
                    }
                }
            }
        }

        i = SOC_IS_DRACO15(unit) ? 1 : 0; /* 5695 index 0 reserved */
        for ( ; i < fi->fc_size; i++) {
            if (FFPCOUNTER_ISSET(unit, pic, i)) {
                if (_hw_ffpcounter_get(unit, pic, i, &new) >= 0) {
                    /* Convenience variables */
                    prev = fi->fc_counters[pic][i].ctr_prev;
                    vptr = &fi->fc_counters[pic][i].ctr_val;
                    if (new != prev) {
                        /* Prepare 64-bit counter diff */
                        COMPILER_64_SET(diff64, 0, new);
                        /* If hardware wrapped, add counter size */
                        if (new < prev) {
                            COMPILER_64_SET(tmp64, 0, 1);
                            COMPILER_64_SHL(tmp64, FFPCOUNTERS_WIDTH(unit));
                            COMPILER_64_ADD_64(diff64, tmp64);
                        }
                        /* Subtract previous hardware counter value */
                        COMPILER_64_SUB_32(diff64, prev);

                        /* Add 64-bit hardware counter diff */
                        COUNTER_ATOMIC_BEGIN(s);
                        COMPILER_64_ADD_64(*vptr, diff64);
                        COUNTER_ATOMIC_END(s);

                        /* Update cached hardware counter */
                        fi->fc_counters[pic][i].ctr_prev = new;
                    }
                }
            }
        }
    }
}

/*
 * Function:
 *	_bcm_strata_kbits_to_bucketsize
 * Description:
 *	Routine that converts desired burst size (in kbits) 
 *	into bucketsize select field value.
 *	Used for Draco, Tucana, and Draco 1.5
 * Parameters:
 *	unit - StrataSwitch PCI device unit number (driver internal).
 *	port - port number.
 *      ingress - set for ffp metering, cleared otherwise
 *	kbits_burst - Desired size of maximum burst traffic
 * Returns:
 *	Register bits for bucket size select field (3-bit field). 
 * Notes:
 *	None.
 */

uint8
_bcm_strata_kbits_to_bucketsize(int unit, bcm_port_t port, 
                                int ingress, uint32 kbits_burst)
{
    uint8		i;
    uint32		*bucket_settings;

    if (SOC_IS_TUCANA(unit)) {
        bucket_settings = tucana_buckets;
    } else if (SOC_IS_DRACO15(unit)) {
        bucket_settings = draco15_buckets;
    }
    else
    {
        /* Is Draco1 */

        if (ingress) {
            bucket_settings = draco_ingress_buckets;
        }
        else {
            bucket_settings = draco_egress_buckets;
        }
    }

    for (i = 0; i < NUM_BUCKET_SIZES; i++) {
        if (kbits_burst <= bucket_settings[i])
            break; 
    }

    if (i == NUM_BUCKET_SIZES) {
        i--;
    }

    return i;
}

/*
 * Function:
 *	_bcm_strata_bucketsize_to_kbits
 * Description:
 *	Routine that converts 3-bit bucketsize select 
 *	field value into its effective burst size (in kbits).
 *	Used for Draco, Tucana, and Draco 1.5
 * Parameters:
 *	unit - StrataSwitch PCI device unit number (driver internal).
 *	port - port number.
 *      ingress - set for ingress metering, cleared otherwise
 *	reg_val - BUCKETSIZEf field value
 * Returns:
 *	Max size burst traffic in kbits. 
 * Notes:
 *	None.
 */

uint32
_bcm_strata_bucketsize_to_kbits(int unit, bcm_port_t port, 
                                int ingress, uint8 reg_val)
{
    if (SOC_IS_TUCANA(unit))
    {
        return tucana_buckets[reg_val];
    }
    else if (SOC_IS_DRACO15(unit))
    {
        return draco15_buckets[reg_val];
    }
    else
    {
        if (ingress)
        {
            return draco_ingress_buckets[reg_val];
        }
        else
        {
            return draco_egress_buckets[reg_val];
        }
    }
}

/*
 * Function:
 *	_bcm_strata_kbps_to_refreshcount
 * Description:
 *	Routine that converts desired meter rate into Strata style
 *	leaky bucket refresh count register value.
 *	Used for Draco, Tucana, and Draco 1.5
 * Parameters:
 *	unit - StrataSwitch PCI device unit number (driver internal).
 *	port - port number.
 *      ingress - set for ingress metering, cleared otherwise
 *	kbits_sec - Desired traffic meter rate
 * Returns:
 *	Register bits for REFCOUNTf field.
 */

uint16
_bcm_strata_kbps_to_refreshcount(int unit, bcm_port_t port, 
                                 int ingress, uint32 kbits_sec)
{
    uint32 refreshcount; /* must be 32 bits to handle overflows */

    if (!kbits_sec) {
        return 0;
    }

    if (SOC_IS_TUCANA(unit)) {
        /* Check kbits_sec upper limit */
        if (kbits_sec > (0xFFFFFFFF-63))
            kbits_sec = (0xFFFFFFFF-63);
        refreshcount = (kbits_sec + 63) / 64;
        /* The REFRESHCOUNT field is limited to 16 bits */
        if (refreshcount > 0xFFFF) {
            refreshcount = 0xFFFF; 
        }
    } else if (SOC_IS_DRACO15(unit)) {
        /* Check kbits_sec upper limit */
        if (kbits_sec > (0xFFFFFFFF-63)) {
            kbits_sec = (0xFFFFFFFF-63);
        }
        refreshcount = (kbits_sec + 63) / 64;
        /* The REFRESHCOUNT field is limited to 14 bits */
        if (refreshcount > 0x3FFF) {
            refreshcount = 0x3FFF; 
        }
    }
    else
    {
        /* Is Draco1 */
        /* Check kbits_sec upper limit */

        if (kbits_sec > (0xFFFFFFFF-999)) {
            kbits_sec = (0xFFFFFFFF-999);
        }
        refreshcount = (kbits_sec + 999) / 1000;
        /* The REFRESHCOUNT field is limited to 10 bits */
        if (refreshcount > 0x3FF) {
            refreshcount = 0x3FF; 
        }
    }

    return (uint16)refreshcount;
}

/*
 * Function:
 *	_bcm_strata_refreshcount_to_kbps
 * Description:
 *	Routine that converts Strata style leaky bucket 
 *	register value into its associated meter rate.
 *	Used for Draco, Tucana, and Draco 1.5
 * Parameters:
 *	unit - StrataSwitch PCI device unit number (driver internal).
 *	port - port number.
 *      ingress - set for ingress metering, cleared otherwise
 *	reg_val - REFCOUNTf field value
 * Returns:
 *	Traffic meter rate in kbits per second. 
 */

uint32
_bcm_strata_refreshcount_to_kbps(int unit, bcm_port_t port, 
                                 int ingress, uint16 reg_val)
{
    if (SOC_IS_TUCANA(unit) || SOC_IS_DRACO15(unit))
    {
        return reg_val * 64;
    }
    else
    {
        return reg_val * 1000;
    }  
}

#endif /* BCM_METER_SUPPORT */
