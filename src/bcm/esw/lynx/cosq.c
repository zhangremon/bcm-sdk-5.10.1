/*
 * $Id: cosq.c 1.20 Broadcom SDK $
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
 * COS Queue Management
 * Purpose: API to set different cosq, priorities, and scheduler registers.
 */

#include <sal/core/libc.h>

#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/debug.h>

#include <bcm/error.h>
#include <bcm/cosq.h>

#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/lynx.h>
#include <bcm_int/esw/strata.h>

#include <bcm_int/esw_dispatch.h>

#define	IBPCELL_PERCENT	90

static int _num_cosq[SOC_MAX_NUM_DEVICES];

/*
 * Function:
 *      bcm_lynx_cosq_init
 * Purpose:
 *      Initialize (clear) all COS schedule/mapping state.
 * Parameters:
 *      unit - StrataSwitch unit number.
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_lynx_cosq_init(int unit)
{
    int		num_cos;

    num_cos = soc_property_get(unit, spn_BCM_NUM_COS, BCM_COS_DEFAULT);

    if (num_cos < 1) {
	num_cos = 1;
    } else if (num_cos > NUM_COS(unit)) {
	num_cos = NUM_COS(unit);
    }

    return bcm_lynx_cosq_config_set(unit, num_cos);
}

int
bcm_lynx_cosq_config_set(int unit, int numq)
{
    uint32	val, val10, val32, val54, val76;
    soc_port_t	port;
    int		cos, num_ports, prio, ratio, remain;
    int		xqmax, n, ncells;
 
    xqmax = soc_mem_index_max(unit, MMU_MEMORIES2_XQ0m);
    ncells = (512*1024) / 128;	/* 512KB split into 128 bytes cells: 4096 */
    num_ports = NUM_ALL_PORT(unit);

    /*
     * Acitve cosqs (more or less) equally share the xqmax entries of the
     * XQ memory associated with each port.
     */
#define	SETVAL(_n)	val = _n >= numq ? xqmax : (_n * xqmax / numq)
    val10 = val32 = val54 = val76 = 0;
    SETVAL(1);
    soc_reg_field_set(unit, XQCOSRANGE1_0r, &val10, COS0_ENDADDRESSf, val);
    SETVAL(2);
    soc_reg_field_set(unit, XQCOSRANGE1_0r, &val10, COS1_ENDADDRESSf, val);
    SETVAL(3);
    soc_reg_field_set(unit, XQCOSRANGE3_2r, &val32, COS2_ENDADDRESSf, val);
    SETVAL(4);
    soc_reg_field_set(unit, XQCOSRANGE3_2r, &val32, COS3_ENDADDRESSf, val);
    SETVAL(5);
    soc_reg_field_set(unit, XQCOSRANGE5_4r, &val54, COS4_ENDADDRESSf, val);
    SETVAL(6);
    soc_reg_field_set(unit, XQCOSRANGE5_4r, &val54, COS5_ENDADDRESSf, val);
    SETVAL(7);
    soc_reg_field_set(unit, XQCOSRANGE7_6r, &val76, COS6_ENDADDRESSf, val);
    SETVAL(8);
    soc_reg_field_set(unit, XQCOSRANGE7_6r, &val76, COS7_ENDADDRESSf, val);
#undef	SETVAL

    PBMP_ALL_ITER(unit, port) {
        SOC_IF_ERROR_RETURN(WRITE_XQCOSRANGE1_0r(unit, port, val10));
        SOC_IF_ERROR_RETURN(WRITE_XQCOSRANGE3_2r(unit, port, val32));
        SOC_IF_ERROR_RETURN(WRITE_XQCOSRANGE5_4r(unit, port, val54));
        SOC_IF_ERROR_RETURN(WRITE_XQCOSRANGE7_6r(unit, port, val76));

        for (cos = 0; cos < numq; cos++) {
            val = 0;
            soc_reg_field_set(unit, IBPCOSPKTSETLIMITr, &val, 
                              PKTSETLIMITf, xqmax);
            SOC_IF_ERROR_RETURN
		(WRITE_IBPCOSPKTSETLIMITr(unit, port, cos, val));

            val = 0;
            soc_reg_field_set(unit, HOLCOSPKTSETLIMITr, &val,
			      PKTSETLIMITf, xqmax / numq);
            SOC_IF_ERROR_RETURN
		(WRITE_HOLCOSPKTSETLIMITr(unit, port, cos, val));

            val = 0;
            soc_reg_field_set(unit, CNGCOSPKTLIMITr, &val,
			      CNGPKTSETLIMITf, xqmax / numq);
            
            SOC_IF_ERROR_RETURN
		(WRITE_CNGCOSPKTLIMITr(unit, cos, val));

            val = 0;
            soc_reg_field_set(unit, HOLCOSCELLSETLIMITr, &val,
			      CELLSETLIMITf, ncells / numq / num_ports);
            SOC_IF_ERROR_RETURN
		(WRITE_HOLCOSCELLSETLIMITr(unit, port, cos, val));
        }

	/*
	 * set ingress back pressure to a percentage of fair share of cells
	 */
	if (port == CMIC_PORT(unit)) {
	    n = ncells-1;
	} else {
	    n = (ncells / num_ports / numq) * IBPCELL_PERCENT / 100;
	}
	
        val = 0;
        soc_reg_field_set(unit, IBPCELLSETLIMITr, &val,
                          CELLSETLIMITf, n);
        SOC_IF_ERROR_RETURN(WRITE_IBPCELLSETLIMITr(unit, port, val));

        val = 0;
        soc_reg_field_set(unit, IBPDISCARDSETLIMITr, &val,
                          DISCARDSETLIMITf, ncells-1);
        SOC_IF_ERROR_RETURN(WRITE_IBPDISCARDSETLIMITr(unit, port, val));
    }

    ratio = 8 / numq;
    remain = 8 % numq;
    cos = 0;
    for (prio = 0; prio < 8; prio++) {
        BCM_IF_ERROR_RETURN
            (bcm_esw_cosq_mapping_set(unit, prio, cos));
        if ((prio + 1) == (((cos + 1) * ratio) + 
                           ((remain < (numq - cos)) ? 0 : 
                            (remain - (numq - cos) + 1)))) {
            cos++;
        }
    }

#ifdef BCM_COSQ_HIGIG_MAP_DISABLE
    SOC_IF_ERROR_RETURN(soc_cosq_stack_port_map_disable(unit));
#endif

    _num_cosq[unit] = numq;

    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_lynx_cosq_config_get
 * Purpose:
 *      Get the number of cos queues
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      numq - (Output) number of cosq
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_lynx_cosq_config_get(int unit, int *numq)
{
    if (_num_cosq[unit] == 0) {
	return BCM_E_INIT;
    }

    if (numq != NULL) {
        *numq = _num_cosq[unit];
    }

    return (BCM_E_NONE);
}

static soc_field_t _lynx_cosarb_port2field[] = {
    PORT0_COSARBf,	/* 0 - xe0 */
    PORTIPIC_COSARBf,	/* 1 - hg0 */
    PORTCPU_COSARBf,	/* 2 - cpu0 */
};

/*
 * Function:
 *      bcm_lynx_cosq_port_sched_set
 * Purpose:
 *      Set up class-of-service policy and corresponding weights and delay
 * Parameters:
 *      unit - StrataSwitch unit number.
 *	pbm - port bitmap
 *      mode - Scheduling mode, one of BCM_COSQ_xxx
 *	weights - Weights for each COS queue
 *		Unused if mode is BCM_COSQ_STRICT.
 *		Indicates number of packets sent before going on to
 *		the next COS queue.
 *	delay - Maximum delay in microseconds before returning the
 *		round-robin to the highest priority COS queue
 *		(Unused if mode other than BCM_COSQ_BOUNDED_DELAY)
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_lynx_cosq_port_sched_set(int unit, bcm_pbmp_t pbm,
			     int mode, const int weights[], int delay)
{
    uint32		wfq;
    int			port, t;
    uint32		cosarbsel;
    int			mbits;

    COMPILER_REFERENCE(delay);

    switch (mode) {
    case BCM_COSQ_STRICT:
	mbits = 0;
	break;
    case BCM_COSQ_ROUND_ROBIN:
	mbits = 1;
	break;
    case BCM_COSQ_WEIGHTED_ROUND_ROBIN:
	mbits = 2;
	/*
	 * All weight values must fit within 4 bits.
	 * If weight is 0, this queue is run in strict mode,
	 * others run in WFQ mode.
	 */

	t = weights[0] | weights[1] | weights[2] | weights[3] |
	    weights[4] | weights[5] | weights[6] | weights[7];

	if ((t & ~0xf) != 0) {
	    return BCM_E_PARAM;
	}
	break;
    case BCM_COSQ_BOUNDED_DELAY:	/* not supported in xgs */
    default:
	return BCM_E_PARAM;
    }

    SOC_IF_ERROR_RETURN(READ_COSARBSELr(unit, &cosarbsel));
    PBMP_ITER(pbm, port) {
	if (port >= COUNTOF(_lynx_cosarb_port2field)) {
	    return BCM_E_PARAM;
	}
	soc_reg_field_set(unit, COSARBSELr, &cosarbsel,
			  _lynx_cosarb_port2field[port], mbits);
    }
    SOC_IF_ERROR_RETURN(WRITE_COSARBSELr(unit, cosarbsel));

    if (mode == BCM_COSQ_WEIGHTED_ROUND_ROBIN) {
	/*
	 * Weighted Fair Queueing scheduling among vaild COSs
	 */
	PBMP_ITER(pbm, port) {
	    SOC_IF_ERROR_RETURN(READ_WFQWEIGHTSr(unit, port, &wfq));
	    soc_reg_field_set(unit, WFQWEIGHTSr, &wfq,
			      COS0WEIGHTf, weights[0]);
	    soc_reg_field_set(unit, WFQWEIGHTSr, &wfq,
			      COS1WEIGHTf, weights[1]);
	    soc_reg_field_set(unit, WFQWEIGHTSr, &wfq,
			      COS2WEIGHTf, weights[2]);
	    soc_reg_field_set(unit, WFQWEIGHTSr, &wfq,
			      COS3WEIGHTf, weights[3]);
	    soc_reg_field_set(unit, WFQWEIGHTSr, &wfq,
			      COS4WEIGHTf, weights[4]);
	    soc_reg_field_set(unit, WFQWEIGHTSr, &wfq,
			      COS5WEIGHTf, weights[5]);
	    soc_reg_field_set(unit, WFQWEIGHTSr, &wfq,
			      COS6WEIGHTf, weights[6]);
	    soc_reg_field_set(unit, WFQWEIGHTSr, &wfq,
			      COS7WEIGHTf, weights[7]);
	    SOC_IF_ERROR_RETURN(WRITE_WFQWEIGHTSr(unit, port, wfq));
	}
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_lynx_cosq_port_sched_get
 * Purpose:
 *      Retrieve class-of-service policy and corresponding weights and delay
 * Parameters:
 *      unit - StrataSwitch unit number.
 *	pbm - port bitmap
 *      mode_ptr - (output) Scheduling mode, one of BCM_COSQ_xxx
 *	weights - (output) Weights for each COS queue
 *		Unused if mode is BCM_COSQ_STRICT.
 *	delay - (output) Maximum delay in microseconds before returning
 *		the round-robin to the highest priority COS queue
 *		Unused if mode other than BCM_COSQ_BOUNDED_DELAY.
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_lynx_cosq_port_sched_get(int unit, bcm_pbmp_t pbm,
			     int *mode, int weights[], int *delay)
{
    uint32		cosarbsel, wfq;
    int			mbits, port;

    mbits = -1;
    SOC_IF_ERROR_RETURN(READ_COSARBSELr(unit, &cosarbsel));
    PBMP_ITER(pbm, port) {
	if (port >= COUNTOF(_lynx_cosarb_port2field)) {
	    return BCM_E_PARAM;
	}
	mbits = soc_reg_field_get(unit, COSARBSELr, cosarbsel,
				  _lynx_cosarb_port2field[port]);
	break;
    }

    switch (mbits) {
    case 0:
        *mode = BCM_COSQ_STRICT;
	break;
    case 1:
        *mode = BCM_COSQ_ROUND_ROBIN;
	break;
    case 2:
	*mode = BCM_COSQ_WEIGHTED_ROUND_ROBIN;
	break;
    default:
	return BCM_E_INTERNAL;
    }

    if (mbits == 2) {
	wfq = 0;
	PBMP_ITER(pbm, port) {
	    SOC_IF_ERROR_RETURN(READ_WFQWEIGHTSr(unit, port, &wfq));
	    break;
	}
    	weights[0] = soc_reg_field_get(unit, WFQWEIGHTSr, wfq, COS0WEIGHTf);
    	weights[1] = soc_reg_field_get(unit, WFQWEIGHTSr, wfq, COS1WEIGHTf);
    	weights[2] = soc_reg_field_get(unit, WFQWEIGHTSr, wfq, COS2WEIGHTf);
    	weights[3] = soc_reg_field_get(unit, WFQWEIGHTSr, wfq, COS3WEIGHTf);
    	weights[4] = soc_reg_field_get(unit, WFQWEIGHTSr, wfq, COS4WEIGHTf);
    	weights[5] = soc_reg_field_get(unit, WFQWEIGHTSr, wfq, COS5WEIGHTf);
    	weights[6] = soc_reg_field_get(unit, WFQWEIGHTSr, wfq, COS6WEIGHTf);
    	weights[7] = soc_reg_field_get(unit, WFQWEIGHTSr, wfq, COS7WEIGHTf);
    }

    if (delay) {
	*delay = 0;
    }
    return BCM_E_NONE;
}

int
bcm_lynx_cosq_discard_set(int unit, uint32 flags)
{
    return BCM_E_UNAVAIL;
}

int
bcm_lynx_cosq_discard_get(int unit, uint32 *flags)
{
    return BCM_E_UNAVAIL;
}

int
bcm_lynx_cosq_discard_port_set(int unit, bcm_port_t port,
                               bcm_cos_queue_t cosq,
                               uint32 color,
                               int drop_start,
                               int drop_slope,
                               int average_time)
{
    return BCM_E_UNAVAIL;
}

int
bcm_lynx_cosq_discard_port_get(int unit, bcm_port_t port,
                               bcm_cos_queue_t cosq,
                               uint32 color,
                               int *drop_start,
                               int *drop_slope,
                               int *average_time)
{
    return BCM_E_UNAVAIL;
}

#ifdef BCM_WARM_BOOT_SUPPORT
int
bcm_lynx_cosq_sync(int unit)
{
    return BCM_E_UNAVAIL;
}
#endif /* BCM_WARM_BOOT_SUPPORT */

#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
void
bcm_lynx_cosq_sw_dump(int unit)
{
    soc_cm_print("\nSW Information COSQ - Unit %d\n", unit);
    soc_cm_print("    Number: %d\n", _num_cosq[unit]);

    return;
}
#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */
