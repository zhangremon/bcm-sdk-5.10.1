/*
 * $Id: cosq.c 1.44 Broadcom SDK $
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
 * File:        cosq.c
 */

#include <sal/core/libc.h>

#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/tucana.h>

#include <bcm/error.h>
#include <bcm/cosq.h>

#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/tucana.h>

#include <bcm_int/esw_dispatch.h>

/*
 * Ingress Back Pressure Config Settings
 * These ought to be settable through properties or a bcm API.
 */
#define	IBPCELL_PERCENT	75	/* % of fair share of cells */
#define	IBPPKT_SPREAD	4	/* pkt backpressure for up to 4 into 1 */
#define	IBPPKT_MIN	8	/* minimum pkt backpressure setting */

/* WFQMINBWCOS per port, per queue */
static int **_tucana_wfqminbwcos[SOC_MAX_NUM_DEVICES];

static int _num_cosq[SOC_MAX_NUM_DEVICES];

#define WFQMINBWCOS(_unit_, _port_, _cosq_) \
        _tucana_wfqminbwcos[_unit_][_port_][_cosq_]

/*
 * Function:
 *      bcm_tucana_cosq_init
 * Purpose:
 *      Initialize (clear) all COS schedule/mapping state.
 * Parameters:
 *      unit - StrataSwitch unit number.
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_tucana_cosq_init(int unit)
{
    bcm_port_t	port;
    int		alloc_size, num_cos;
    int		cosq;

    BCM_IF_ERROR_RETURN
        (bcm_tucana_cosq_detach(unit, 0));

    alloc_size = SOC_MAX_NUM_PORTS * sizeof (int *);
    _tucana_wfqminbwcos[unit] =
        sal_alloc(alloc_size, "COS WFQ min BW by ports");
    if (_tucana_wfqminbwcos[unit] == NULL) {
        return BCM_E_MEMORY;
    }
    sal_memset(_tucana_wfqminbwcos[unit], 0, alloc_size);

    alloc_size = NUM_COS(unit) * sizeof (int);

    PBMP_ALL_ITER(unit, port) {
        _tucana_wfqminbwcos[unit][port] =
            sal_alloc(alloc_size, "COS WFQ min BW by ports by COS queue");
        if (_tucana_wfqminbwcos[unit][port] == NULL) {
            return BCM_E_MEMORY;
        }
	sal_memset(_tucana_wfqminbwcos[unit][port], 0, alloc_size);
    }

    /* Clean WFQMINBWCOS state */
    PBMP_ALL_ITER(unit, port) {
        for (cosq = 0; cosq < NUM_COS(unit); cosq++) {
            WFQMINBWCOS(unit, port, cosq) =
                BCM_TUCANA_COSQ_PORT_BW_MIN_DFLT;
            SOC_IF_ERROR_RETURN
                (WRITE_WFQMINBWCOSr(unit, port, cosq,
                                    BCM_TUCANA_COSQ_PORT_BW_MIN_DFLT));
        }
    }

    num_cos = soc_property_get(unit, spn_BCM_NUM_COS, BCM_COS_DEFAULT);

    if (num_cos < 1) {
	num_cos = 1;
    } else if (num_cos > NUM_COS(unit)) {
	num_cos = NUM_COS(unit);
    }

    return bcm_tucana_cosq_config_set(unit, num_cos);
}

static soc_field_t _bcm_tucana_cosend_field[8] = {
    COS0_ENDADDRESSf, COS1_ENDADDRESSf, COS2_ENDADDRESSf, COS3_ENDADDRESSf,
    COS4_ENDADDRESSf, COS5_ENDADDRESSf, COS6_ENDADDRESSf, COS7_ENDADDRESSf
};

/*
 * Get weighted per-cos XQ limits
 * as previously programmed in mmu init
 */
static int
_bcm_tucana_cosq_get_cos_xq(int unit, bcm_port_t port, int cos_xq[])
{
    uint32      val;
    int         cos, cos_end, prev_end;

    prev_end = 0;
    for (cos = 0; cos < 8; cos++) {
        if (cos == 0) {
            SOC_IF_ERROR_RETURN
                (READ_XQCOSENTRIES0_3r(unit, REG_PORT_ANY, &val));
        } else if (cos == 4) {
            SOC_IF_ERROR_RETURN
                (READ_XQCOSENTRIES4_7r(unit, REG_PORT_ANY, &val));
        }
        if (cos < 4) {
            cos_end = soc_reg_field_get(unit, XQCOSENTRIES0_3r,
                                        val, _bcm_tucana_cosend_field[cos]);
        } else {
            cos_end = soc_reg_field_get(unit, XQCOSENTRIES4_7r,
                                        val, _bcm_tucana_cosend_field[cos]);
        }
        cos_xq[cos] = (cos_end - prev_end) * 16;
        prev_end = cos_end;
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_tucana_cosq_config_set
 * Purpose:
 *      Set the number of COS queues
 * Parameters:
 *      unit - Tucana unit number.
 *      numq - number of COS queues (1-8).
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_tucana_cosq_config_set(int unit, int numq)
{
    uint32	val;
    uint64      val64;
    int		cos, num_ports, prio, ratio, remain;
    int		num_cells, n, rc;
    soc_port_t	port;
    int         has_ipic = (NUM_HG_PORT(unit) != 0);
    int		cos_xq[8], port_xq;

    SOC_IF_ERROR_RETURN
        (soc_tucana_num_cells(unit, &num_cells));
    num_ports = NUM_ALL_PORT(unit);
    port_xq = soc_mem_index_count(unit, MMU_XQ0m);

    /*
     * Get weighted per-cos XQ limits
     * as previously programmed in mmu init
     */
    rc = _bcm_tucana_cosq_get_cos_xq(unit, REG_PORT_ANY, cos_xq);
    if (rc != BCM_E_NONE) {
        return rc;
    }

    /* Tucana MMU setup for CBP allocation */

    PBMP_ALL_ITER(unit, port) {
        /*
         * RESETLIMIT left at default
         */

        for (cos = 0; cos < numq; cos++) {
            /*
             * Assume wire-speed mode, set to "fair share"
             * value, so no single (low priority) cosq can
             * starve another (higher priority) cosq.
             */
            val = 0;
            soc_reg_field_set(unit, EGRCELLLIMITCG0COSr, &val,
                              CELLSETLIMITf, num_cells / numq / num_ports);
            SOC_IF_ERROR_RETURN
                (WRITE_EGRCELLLIMITCG0COSr(unit, port, cos, val));
            if (has_ipic) {
                SOC_IF_ERROR_RETURN
                    (WRITE_EGRCELLLIMITCG1COSr(unit, port, cos, val));
            }

	    /*
	     * Packet Ingress Backpressure
	     * Each cosq has a share of the port total XQs.
	     * Since an egress does not tell the ingresses it is getting
	     * full, we split the ingress packet limits so that multiple
	     * ingresses can send to a single egress and assert backpressure
	     * as things fill up.
	     */
	    if (port == CMIC_PORT(unit)) {
		n = port_xq-1;
	    } else {
		n = cos_xq[cos] / IBPPKT_SPREAD;
		if (n < IBPPKT_MIN) {
		    n = IBPPKT_MIN;
		}
	    }
            COMPILER_64_ZERO(val64);
            soc_reg64_field32_set(unit, INGPKTLIMITSCOSr, &val64,
                                  IBP_PKTLIMITf, n);
            soc_reg64_field32_set(unit, INGPKTLIMITSCOSr, &val64,
                                  DISCARD_PKTLIMITf, num_cells-1);
            SOC_IF_ERROR_RETURN
                (WRITE_INGPKTLIMITSCOSr(unit, port, cos, val64));

            /* No analogue to HOLCOSPKTSETLIMITr in Tucana */
        }

	/*
	 * set ingress back pressure to a percentage of fair share of cells
	 * add an extra divide by 2 to offset the larger number of cells
	 * versus packets
	 */
	if (port == CMIC_PORT(unit)) {
	    n = num_cells-1;
	} else {
	    n = num_cells * IBPCELL_PERCENT / (num_ports * numq * 2 * 100);
	}

        val = 0;
        soc_reg_field_set(unit, INGCELLLIMITIBPCG0r, &val,
                          IBPLIMITf, n);
        SOC_IF_ERROR_RETURN
            (WRITE_INGCELLLIMITIBPCG0r(unit, port, val));
        if (has_ipic) {
            SOC_IF_ERROR_RETURN
                (WRITE_INGCELLLIMITIBPCG1r(unit, port, val));
        }

        val = 0;
        soc_reg_field_set(unit, INGCELLLIMITDISCARDCG0r, &val,
                          DISCARDLIMITf, num_cells-1);
        SOC_IF_ERROR_RETURN
            (WRITE_INGCELLLIMITDISCARDCG0r(unit, port, val));
        if (has_ipic) {
            SOC_IF_ERROR_RETURN
                (WRITE_INGCELLLIMITDISCARDCG1r(unit, port, val));
        }
    }

    /* Leave EGRPKTLIMITCNGCOS at default of 75% of HOL */

    /* Map the eight 802.1 priority levels to the active cosqs */
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

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_tucana_cosq_config_get
 * Purpose:
 *      Get the number of cos queues
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      numq - (Output) number of cosq
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_tucana_cosq_config_get(int unit, int *numq)
{
    if (_num_cosq[unit] == 0) {
	return BCM_E_INIT;
    }

    if (numq != NULL) {
        *numq = _num_cosq[unit];
    }

    return (BCM_E_NONE);
}

static soc_reg_t _tucana_sched_conf_port2reg[] = {
    SCHEDULERCONFIGMOD0_P0_7r,          /* fe0 - fe7 */
    SCHEDULERCONFIGMOD0_P0_7r,          /* fe0 - fe7 */
    SCHEDULERCONFIGMOD0_P0_7r,          /* fe0 - fe7 */
    SCHEDULERCONFIGMOD0_P0_7r,          /* fe0 - fe7 */
    SCHEDULERCONFIGMOD0_P0_7r,          /* fe0 - fe7 */
    SCHEDULERCONFIGMOD0_P0_7r,          /* fe0 - fe7 */
    SCHEDULERCONFIGMOD0_P0_7r,          /* fe0 - fe7 */
    SCHEDULERCONFIGMOD0_P0_7r,          /* fe0 - fe7 */
    SCHEDULERCONFIGMOD0_P8_15r,         /* fe8 - fe15 */
    SCHEDULERCONFIGMOD0_P8_15r,         /* fe8 - fe15 */
    SCHEDULERCONFIGMOD0_P8_15r,         /* fe8 - fe15 */
    SCHEDULERCONFIGMOD0_P8_15r,         /* fe8 - fe15 */
    SCHEDULERCONFIGMOD0_P8_15r,         /* fe8 - fe15 */
    SCHEDULERCONFIGMOD0_P8_15r,         /* fe8 - fe15 */
    SCHEDULERCONFIGMOD0_P8_15r,         /* fe8 - fe15 */
    SCHEDULERCONFIGMOD0_P8_15r,         /* fe8 - fe15 */
    SCHEDULERCONFIGMOD0_P16_23r,        /* fe16 - fe23 */
    SCHEDULERCONFIGMOD0_P16_23r,        /* fe16 - fe23 */
    SCHEDULERCONFIGMOD0_P16_23r,        /* fe16 - fe23 */
    SCHEDULERCONFIGMOD0_P16_23r,        /* fe16 - fe23 */
    SCHEDULERCONFIGMOD0_P16_23r,        /* fe16 - fe23 */
    SCHEDULERCONFIGMOD0_P16_23r,        /* fe16 - fe23 */
    SCHEDULERCONFIGMOD0_P16_23r,        /* fe16 - fe23 */
    SCHEDULERCONFIGMOD0_P16_23r,        /* fe16 - fe23 */
    SCHEDULERCONFIGMOD0_P24r,	        /* ge0 */
    SCHEDULERCONFIGMOD0_P25r,           /* ge1 */
    SCHEDULERCONFIGMOD0_P26r,           /* ge2 */
    SCHEDULERCONFIGMOD0_P27r,           /* ge3 */
    SCHEDULERCONFIGMOD0_P28r,           /* cmic */
    INVALIDr,
    INVALIDr,
    INVALIDr,
    SCHEDULERCONFIGMOD1_P0_7r,          /* fe24 - fe31 */
    SCHEDULERCONFIGMOD1_P0_7r,          /* fe24 - fe31 */
    SCHEDULERCONFIGMOD1_P0_7r,          /* fe24 - fe31 */
    SCHEDULERCONFIGMOD1_P0_7r,          /* fe24 - fe31 */
    SCHEDULERCONFIGMOD1_P0_7r,          /* fe24 - fe31 */
    SCHEDULERCONFIGMOD1_P0_7r,          /* fe24 - fe31 */
    SCHEDULERCONFIGMOD1_P0_7r,          /* fe24 - fe31 */
    SCHEDULERCONFIGMOD1_P0_7r,          /* fe24 - fe31 */
    SCHEDULERCONFIGMOD1_P8_15r,         /* fe32 - fe39 */
    SCHEDULERCONFIGMOD1_P8_15r,         /* fe32 - fe39 */
    SCHEDULERCONFIGMOD1_P8_15r,         /* fe32 - fe39 */
    SCHEDULERCONFIGMOD1_P8_15r,         /* fe32 - fe39 */
    SCHEDULERCONFIGMOD1_P8_15r,         /* fe32 - fe39 */
    SCHEDULERCONFIGMOD1_P8_15r,         /* fe32 - fe39 */
    SCHEDULERCONFIGMOD1_P8_15r,         /* fe32 - fe39 */
    SCHEDULERCONFIGMOD1_P8_15r,         /* fe32 - fe39 */
    SCHEDULERCONFIGMOD1_P16_23r,        /* fe40 - fe47 */
    SCHEDULERCONFIGMOD1_P16_23r,        /* fe40 - fe47 */
    SCHEDULERCONFIGMOD1_P16_23r,        /* fe40 - fe47 */
    SCHEDULERCONFIGMOD1_P16_23r,        /* fe40 - fe47 */
    SCHEDULERCONFIGMOD1_P16_23r,        /* fe40 - fe47 */
    SCHEDULERCONFIGMOD1_P16_23r,        /* fe40 - fe47 */
    SCHEDULERCONFIGMOD1_P16_23r,        /* fe40 - fe47 */
    SCHEDULERCONFIGMOD1_P16_23r,        /* fe40 - fe47 */
    SCHEDULERCONFIGMOD1_P24r,           /* ipic */
};

static soc_reg_t _tucana_wfq_conf_port2reg[] = {
    WFQCONFIGMOD0_P0_7r,          /* fe0 - fe7 */
    WFQCONFIGMOD0_P0_7r,          /* fe0 - fe7 */
    WFQCONFIGMOD0_P0_7r,          /* fe0 - fe7 */
    WFQCONFIGMOD0_P0_7r,          /* fe0 - fe7 */
    WFQCONFIGMOD0_P0_7r,          /* fe0 - fe7 */
    WFQCONFIGMOD0_P0_7r,          /* fe0 - fe7 */
    WFQCONFIGMOD0_P0_7r,          /* fe0 - fe7 */
    WFQCONFIGMOD0_P0_7r,          /* fe0 - fe7 */
    WFQCONFIGMOD0_P8_15r,         /* fe8 - fe15 */
    WFQCONFIGMOD0_P8_15r,         /* fe8 - fe15 */
    WFQCONFIGMOD0_P8_15r,         /* fe8 - fe15 */
    WFQCONFIGMOD0_P8_15r,         /* fe8 - fe15 */
    WFQCONFIGMOD0_P8_15r,         /* fe8 - fe15 */
    WFQCONFIGMOD0_P8_15r,         /* fe8 - fe15 */
    WFQCONFIGMOD0_P8_15r,         /* fe8 - fe15 */
    WFQCONFIGMOD0_P8_15r,         /* fe8 - fe15 */
    WFQCONFIGMOD0_P16_23r,        /* fe16 - fe23 */
    WFQCONFIGMOD0_P16_23r,        /* fe16 - fe23 */
    WFQCONFIGMOD0_P16_23r,        /* fe16 - fe23 */
    WFQCONFIGMOD0_P16_23r,        /* fe16 - fe23 */
    WFQCONFIGMOD0_P16_23r,        /* fe16 - fe23 */
    WFQCONFIGMOD0_P16_23r,        /* fe16 - fe23 */
    WFQCONFIGMOD0_P16_23r,        /* fe16 - fe23 */
    WFQCONFIGMOD0_P16_23r,        /* fe16 - fe23 */
    WFQCONFIGMOD0_P24r,	          /* ge0 */
    WFQCONFIGMOD0_P25r,           /* ge1 */
    WFQCONFIGMOD0_P26r,           /* ge2 */
    WFQCONFIGMOD0_P27r,           /* ge3 */
    WFQCONFIGMOD0_P28r,           /* cmic */
    INVALIDr,
    INVALIDr,
    INVALIDr,
    WFQCONFIGMOD1_P0_7r,          /* fe24 - fe31 */
    WFQCONFIGMOD1_P0_7r,          /* fe24 - fe31 */
    WFQCONFIGMOD1_P0_7r,          /* fe24 - fe31 */
    WFQCONFIGMOD1_P0_7r,          /* fe24 - fe31 */
    WFQCONFIGMOD1_P0_7r,          /* fe24 - fe31 */
    WFQCONFIGMOD1_P0_7r,          /* fe24 - fe31 */
    WFQCONFIGMOD1_P0_7r,          /* fe24 - fe31 */
    WFQCONFIGMOD1_P0_7r,          /* fe24 - fe31 */
    WFQCONFIGMOD1_P8_15r,         /* fe32 - fe39 */
    WFQCONFIGMOD1_P8_15r,         /* fe32 - fe39 */
    WFQCONFIGMOD1_P8_15r,         /* fe32 - fe39 */
    WFQCONFIGMOD1_P8_15r,         /* fe32 - fe39 */
    WFQCONFIGMOD1_P8_15r,         /* fe32 - fe39 */
    WFQCONFIGMOD1_P8_15r,         /* fe32 - fe39 */
    WFQCONFIGMOD1_P8_15r,         /* fe32 - fe39 */
    WFQCONFIGMOD1_P8_15r,         /* fe32 - fe39 */
    WFQCONFIGMOD1_P16_23r,        /* fe40 - fe47 */
    WFQCONFIGMOD1_P16_23r,        /* fe40 - fe47 */
    WFQCONFIGMOD1_P16_23r,        /* fe40 - fe47 */
    WFQCONFIGMOD1_P16_23r,        /* fe40 - fe47 */
    WFQCONFIGMOD1_P16_23r,        /* fe40 - fe47 */
    WFQCONFIGMOD1_P16_23r,        /* fe40 - fe47 */
    WFQCONFIGMOD1_P16_23r,        /* fe40 - fe47 */
    WFQCONFIGMOD1_P16_23r,        /* fe40 - fe47 */
    WFQCONFIGMOD1_P24r,           /* ipic */
};

/*
 * Function:
 *      bcm_tucana_cosq_port_sched_set
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
bcm_tucana_cosq_port_sched_set(int unit, bcm_pbmp_t pbm,
			       int mode, const int weights[], int delay)
{
    uint32		sched_config, wfq, wfq_config;
    int			port, t, cos, mbits, last_blk = -1;
    bcm_pbmp_t          blk_bitmap, test_bitmap;
    int                 wfq_sp_mask = 0;
    int 		wfq_max_bw_mask = 0;

    COMPILER_REFERENCE(delay);

    switch (mode) {
    case BCM_COSQ_STRICT:
	mbits = 0;
        wfq_sp_mask = 0xff;
	wfq_max_bw_mask = 0xff;
	break;
    case BCM_COSQ_ROUND_ROBIN:
	mbits = 1;
	break;
    case BCM_COSQ_WEIGHTED_ROUND_ROBIN:
	mbits = 3;
	/*
	 * All weight values must fit within 4 bits (per HW).
	 * If weight is 0, this queue is run in strict mode (Draco compat),
	 * others run in WFQ mode.
	 */

	t = weights[0] | weights[1] | weights[2] | weights[3] |
	    weights[4] | weights[5] | weights[6] | weights[7];

	if ((t & ~0xf) != 0) {
	    return BCM_E_PARAM;
	}
	break;
    case BCM_COSQ_WEIGHTED_FAIR_QUEUING:
	mbits = 0;
        wfq_sp_mask = 0;
	/*
	 * All weight values are either 0 or 1.
	 * If weight is 0, this queue is run in strict mode (Draco compat),
	 * others run in WFQ mode.
	 */

	t = weights[0] | weights[1] | weights[2] | weights[3] |
	    weights[4] | weights[5] | weights[6] | weights[7];

	if ((t & ~0x1) != 0) {
	    return BCM_E_PARAM;
	}
        for (cos = 0; cos < 8; cos++) {
            if (weights[cos] == 0) { /* Strict for this COS */
                wfq_sp_mask |= (1 << cos);
		wfq_max_bw_mask |= (1 << cos);
            }
        }
	break;
    case BCM_COSQ_BOUNDED_DELAY:	/* not supported in xgs */
    default:
	return BCM_E_PARAM;
    }

    PBMP_ITER(pbm, port) {
        /* This register is per port */
        if (mbits == 0) {
            for (cos = 0; cos < 8; cos++) {
                SOC_IF_ERROR_RETURN
                    (READ_WFQMINBWCOSr(unit, port, cos, &wfq));
                if (mode == BCM_COSQ_WEIGHTED_FAIR_QUEUING) {
                    soc_reg_field_set(unit, WFQMINBWCOSr, &wfq,
                                      MIN_BWf, weights[cos]);
                } else if (mode == BCM_COSQ_STRICT) {
		    soc_reg_field_set(unit, WFQMINBWCOSr, &wfq,
			              MIN_BWf, 0);
		} else {
                    soc_reg_field_set(unit, WFQMINBWCOSr, &wfq,
                                      MIN_BWf, WFQMINBWCOS(unit, port, cos));
                }
                SOC_IF_ERROR_RETURN
                    (WRITE_WFQMINBWCOSr(unit, port, cos, wfq));
            }
        }

        /* Other registers are per block */
        if (last_blk == SOC_PORT_BLOCK(unit, port)) {
            continue;
        }
        last_blk = SOC_PORT_BLOCK(unit, port);
        blk_bitmap = SOC_BLOCK_BITMAP(unit, last_blk);
        BCM_PBMP_ASSIGN(test_bitmap, blk_bitmap);
        BCM_PBMP_AND(test_bitmap, pbm);
        if (BCM_PBMP_NEQ(test_bitmap, blk_bitmap)) {
            /* Must configure all ports in PIC */
            return BCM_E_PARAM;
        }
        /*    coverity[overrun-local]    */
        SOC_IF_ERROR_RETURN
            (soc_reg32_get(unit, 
                 _tucana_sched_conf_port2reg[port], REG_PORT_ANY, 0,
                            &sched_config));
	soc_reg_field_set(unit, _tucana_sched_conf_port2reg[port],
                          &sched_config, SCHEDULER_MODEf, mbits);
        SOC_IF_ERROR_RETURN
            (soc_reg32_set(unit, 
                 _tucana_sched_conf_port2reg[port], REG_PORT_ANY, 0,
                            sched_config));

        /* Set WFQ_SP_MASKs in WFQ+SP mode */
        if (mbits == 0) {
            SOC_IF_ERROR_RETURN
                (soc_reg32_get(unit, 
                     _tucana_wfq_conf_port2reg[port], REG_PORT_ANY, 0,
                                &wfq_config));
            soc_reg_field_set(unit, _tucana_wfq_conf_port2reg[port],
                              &wfq_config, WFQ_SP_MASKf, wfq_sp_mask);
	    soc_reg_field_set(unit, _tucana_wfq_conf_port2reg[port],
		              &wfq_config, WFQ_PRIOS_MAX_BW_MASKf, wfq_max_bw_mask);
            SOC_IF_ERROR_RETURN
                (soc_reg32_set(unit, 
                     _tucana_wfq_conf_port2reg[port], REG_PORT_ANY, 0,
                                 wfq_config));

       }
    }

    if (mode == BCM_COSQ_WEIGHTED_ROUND_ROBIN) {
	PBMP_ITER(pbm, port) {
            for (cos = 0; cos < NUM_COS(unit); cos++) {
                wfq = 0;
                soc_reg_field_set(unit, WFQMINBWCOSr, &wfq,
                                  MIN_BWf, weights[cos]);
                SOC_IF_ERROR_RETURN(WRITE_WFQMINBWCOSr(unit, port, cos, wfq));
            }
	}
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_tucana_cosq_port_sched_get
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
bcm_tucana_cosq_port_sched_get(int unit, bcm_pbmp_t pbm,
			       int *mode, int weights[], int *delay)
{
    uint32		sched_config, wfq, wfq_config;
    int			port, cos, mbits = -1;
    int                 wfq_sp_mask;

    PBMP_ITER(pbm, port) {
        /* coverity[overrun-local] : FALSE */
        SOC_IF_ERROR_RETURN
            (soc_reg32_get(unit, 
                 _tucana_sched_conf_port2reg[port], REG_PORT_ANY, 0,
                            &sched_config));
	mbits = soc_reg_field_get(unit, _tucana_sched_conf_port2reg[port],
                                  sched_config, SCHEDULER_MODEf);
        break;
    }

    switch (mbits) {
    case 0:
        SOC_IF_ERROR_RETURN
            (soc_reg32_get(unit, 
                 _tucana_wfq_conf_port2reg[port], REG_PORT_ANY, 0,
                            &wfq_config));
        wfq_sp_mask = soc_reg_field_get(unit, _tucana_wfq_conf_port2reg[port],
                                        wfq_config, WFQ_SP_MASKf);

        if (wfq_sp_mask == 0xff) {
            *mode = BCM_COSQ_STRICT;
        } else {
            for (cos = 0; cos < NUM_COS(unit); cos++) {
                if (wfq_sp_mask & (1 << cos)) {
                    weights[cos] = 0;
                } else {
                    SOC_IF_ERROR_RETURN(READ_WFQMINBWCOSr(unit, port, cos,
                                                          &wfq));
                    weights[cos] =
                        soc_reg_field_get(unit, WFQMINBWCOSr, wfq, MIN_BWf);
                }
            }
            *mode = BCM_COSQ_WEIGHTED_FAIR_QUEUING;
        }
	break;
    case 1:
        *mode = BCM_COSQ_ROUND_ROBIN;
	break;
    case 3:
	*mode = BCM_COSQ_WEIGHTED_ROUND_ROBIN;
        /* port is still valid from the loop above */
        for (cos = 0; cos < NUM_COS(unit); cos++) {
            SOC_IF_ERROR_RETURN(READ_WFQMINBWCOSr(unit, port, cos, &wfq));
            weights[cos] =
                soc_reg_field_get(unit, WFQMINBWCOSr, wfq, MIN_BWf);
        }
	break;
    default:
	return BCM_E_INTERNAL;
    }

    if (delay) {
	*delay = 0;
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_tucana_cosq_sched_weight_max_get
 * Purpose:
 *      Retrieve maximum weights for given COS policy.
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      mode - Scheduling mode, one of BCM_COSQ_xxx
 *	weight_max - (output) Maximum weight for COS queue.
 *		0 if mode is BCM_COSQ_STRICT.
 *		1 if mode is BCM_COSQ_ROUND_ROBIN.
 *		-1 if not applicable to mode.
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_tucana_cosq_sched_weight_max_get(int unit, int mode,
				     int *weight_max)
{
    switch (mode) {
    case BCM_COSQ_STRICT:
	*weight_max = BCM_COSQ_WEIGHT_STRICT;
	break;
    case BCM_COSQ_ROUND_ROBIN:
    case BCM_COSQ_WEIGHTED_FAIR_QUEUING:
	*weight_max = BCM_COSQ_WEIGHT_MIN;
	break;
    case BCM_COSQ_WEIGHTED_ROUND_ROBIN:
	*weight_max = 0xf;
	break;
    default:
	*weight_max = BCM_COSQ_WEIGHT_UNLIMITED;
	return BCM_E_PARAM;
    }

    return BCM_E_NONE;
}

#define TUCANA_BW_GRANULARITY   64
#define TUCANA_BW_FIELD_MAX_IPIC     0x3ffff
#define TUCANA_BW_FIELD_MAX_GPIC     0xffff
#define TUCANA_BW_FIELD_MAX_EPIC     0x7ff

/*
 * Function:
 *      bcm_tucana_cosq_port_bandwidth_set
 * Purpose:
 *      Set bandwidth values for given COS policy.
 * Parameters:
 *      unit - StrataSwitch unit number.
 *	port - port to configure.
 *      cosq - COS queue to configure
 *      kbits_sec_min - minimum bandwidth, kbits/sec.
 *      kbits_sec_max - maximum bandwidth, kbits/sec.
 *      flags - may include:
 *              BCM_COSQ_BW_EXCESS_PREF
 *              BCM_COSQ_BW_MINIMUM_PREF
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_tucana_cosq_port_bandwidth_set(int unit, bcm_port_t port,
                                   bcm_cos_queue_t cosq,
                                   uint32 kbits_sec_min,
                                   uint32 kbits_sec_max,
                                   uint32 kbits_sec_burst,
                                   uint32 flags)
{
    uint32		sched_config, wfq_config, wfqmin, shapermax;
    int			mbits = -1;
    uint32              field_work, bw_field_max;

    if (kbits_sec_burst != kbits_sec_max) {
        return BCM_E_PARAM;
    }

    bw_field_max = IS_HG_PORT(unit, port) ? TUCANA_BW_FIELD_MAX_IPIC : 
        IS_GE_PORT(unit, port) ? TUCANA_BW_FIELD_MAX_GPIC : 
        TUCANA_BW_FIELD_MAX_EPIC;

    /* WFQCONFIGMOD settings from flags */
    SOC_IF_ERROR_RETURN
        (soc_reg32_get(unit, 
             _tucana_wfq_conf_port2reg[port], REG_PORT_ANY, 0,
                        &wfq_config));

    field_work = soc_reg_field_get(unit, _tucana_wfq_conf_port2reg[port],
                                   wfq_config, WFQ_PRIOS_MIN_BW_MASKf);
    if (flags & BCM_COSQ_BW_MINIMUM_PREF) {
        field_work |= 1 << cosq;
    } else {
        field_work &= ~(1 << cosq);
    }
    soc_reg_field_set(unit, _tucana_wfq_conf_port2reg[port],
                      &wfq_config, WFQ_PRIOS_MIN_BW_MASKf, field_work);

    field_work = soc_reg_field_get(unit, _tucana_wfq_conf_port2reg[port],
                                   wfq_config, WFQ_PRIOS_MAX_BW_MASKf);
    if (flags & BCM_COSQ_BW_EXCESS_PREF) {
        field_work |= 1 << cosq;
    } else {
        field_work &= ~(1 << cosq);
    }
    soc_reg_field_set(unit, _tucana_wfq_conf_port2reg[port],
                      &wfq_config, WFQ_PRIOS_MAX_BW_MASKf, field_work);

    SOC_IF_ERROR_RETURN
        (soc_reg32_set(unit, 
             _tucana_wfq_conf_port2reg[port], REG_PORT_ANY, 0,
                         wfq_config));

    /* SHAPERMAXBWCOS setting from kbits_sec_max */
    shapermax = 0;
    if (kbits_sec_max > (bw_field_max * TUCANA_BW_GRANULARITY)) {
        field_work = bw_field_max;
    } else {
        field_work =
            (kbits_sec_max + TUCANA_BW_GRANULARITY - 1) / TUCANA_BW_GRANULARITY;
        if (field_work > bw_field_max) {
            field_work = bw_field_max;
        }
    }
    soc_reg_field_set(unit, SHAPERMAXBWCOSr, &shapermax, MAX_BWf, field_work);
    SOC_IF_ERROR_RETURN
        (WRITE_SHAPERMAXBWCOSr(unit, port, cosq, shapermax));

    SOC_IF_ERROR_RETURN
        (soc_reg32_get(unit,
                 _tucana_sched_conf_port2reg[port], REG_PORT_ANY, 0,
                            &sched_config));
    mbits = soc_reg_field_get(unit, _tucana_sched_conf_port2reg[port],
                              sched_config, SCHEDULER_MODEf);

    if (kbits_sec_min > (bw_field_max * TUCANA_BW_GRANULARITY)) {
        field_work = bw_field_max;
    } else {
        field_work =
            (kbits_sec_min + TUCANA_BW_GRANULARITY - 1) / TUCANA_BW_GRANULARITY;
        if (field_work > bw_field_max) {
            field_work = bw_field_max;
        }
    }
    if (mbits == 0) {
        /* WFQ mode, write WFQMINBWCOS value */
        wfqmin = 0;
        soc_reg_field_set(unit, WFQMINBWCOSr, &wfqmin, MIN_BWf, field_work);
        SOC_IF_ERROR_RETURN
            (WRITE_WFQMINBWCOSr(unit, port, cosq, wfqmin));
    }

    WFQMINBWCOS(unit, port, cosq) = field_work;

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_tucana_cosq_port_bandwidth_get
 * Purpose:
 *      Retrieve bandwidth values for given COS policy.
 * Parameters:
 *      unit - StrataSwitch unit number.
 *	port - port to retrieve.
 *      cosq - COS queue to retrieve
 *      kbits_sec_min - (OUT) minimum bandwidth, kbits/sec.
 *      kbits_sec_max - (OUT) maximum bandwidth, kbits/sec.
 *      flags - (OUT) may include:
 *              BCM_COSQ_BW_EXCESS_PREF
 *              BCM_COSQ_BW_MINIMUM_PREF
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_tucana_cosq_port_bandwidth_get(int unit, bcm_port_t port,
                                   bcm_cos_queue_t cosq,
                                   uint32 *kbits_sec_min,
                                   uint32 *kbits_sec_max,
                                   uint32 *kbits_sec_burst,
                                   uint32 *flags)
{
    uint32		wfq_config, shapermax;
    uint32              field_work, tmp_flags = 0;

    /* WFQCONFIGMOD settings for flags */
    SOC_IF_ERROR_RETURN
        (soc_reg32_get(unit, 
             _tucana_wfq_conf_port2reg[port], REG_PORT_ANY, 0,
                        &wfq_config));

    field_work = soc_reg_field_get(unit, _tucana_wfq_conf_port2reg[port],
                                   wfq_config, WFQ_PRIOS_MIN_BW_MASKf);
    if (field_work & (1 << cosq)) {
        tmp_flags |= BCM_COSQ_BW_MINIMUM_PREF;
    }

    field_work = soc_reg_field_get(unit, _tucana_wfq_conf_port2reg[port],
                                   wfq_config, WFQ_PRIOS_MAX_BW_MASKf);
    if (field_work & (1 << cosq)) {
        tmp_flags |= BCM_COSQ_BW_EXCESS_PREF;
    }

    SOC_IF_ERROR_RETURN
        (READ_SHAPERMAXBWCOSr(unit, port, cosq, &shapermax));
    field_work = soc_reg_field_get(unit, SHAPERMAXBWCOSr, shapermax, MAX_BWf);

    *flags = tmp_flags;
    *kbits_sec_burst = *kbits_sec_max = field_work * TUCANA_BW_GRANULARITY;
    *kbits_sec_min = WFQMINBWCOS(unit, port, cosq) * TUCANA_BW_GRANULARITY;

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_tucana_cosq_discard_set
 * Purpose:
 *      Set the COS queue WRED parameters
 * Parameters:
 *      unit  - Tucana unit number.
 *      flags - BCM_COSQ_DISCARD_*
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_tucana_cosq_discard_set(int unit, uint32 flags)
{
    uint32 val;

    SOC_IF_ERROR_RETURN(
        READ_WREDFUNCTIONr(unit, &val));

    if (flags & BCM_COSQ_DISCARD_CAP_AVERAGE) {
        soc_reg_field_set(unit, WREDFUNCTIONr, &val, CAP_AVERAGEf, 1);
    } else {
        soc_reg_field_set(unit, WREDFUNCTIONr, &val, CAP_AVERAGEf, 0);
    }

    if (flags & BCM_COSQ_DISCARD_ENABLE) {
        soc_reg_field_set(unit, WREDFUNCTIONr, &val, ENABLEf, 1);
    } else {
        soc_reg_field_set(unit, WREDFUNCTIONr, &val, ENABLEf, 0);
    }

    SOC_IF_ERROR_RETURN
        (WRITE_WREDFUNCTIONr(unit, val));

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_tucana_cosq_discard_get
 * Purpose:
 *      Get the COS queue WRED parameters
 * Parameters:
 *      unit  - Tucana unit number.
 *      flags - (OUT) BCM_COSQ_DISCARD_*
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_tucana_cosq_discard_get(int unit, uint32 *flags)
{
    uint32 val;
    int enable, cap_avg;

    *flags = 0;
    SOC_IF_ERROR_RETURN
        (READ_WREDFUNCTIONr(unit, &val));
    enable = soc_reg_field_get(unit, WREDFUNCTIONr, val, ENABLEf);
    cap_avg = soc_reg_field_get(unit, WREDFUNCTIONr, val, CAP_AVERAGEf);

    if (enable) {
        *flags |= BCM_COSQ_DISCARD_ENABLE;
    }
    if (cap_avg) {
        *flags |= BCM_COSQ_DISCARD_CAP_AVERAGE;
    }

    return BCM_E_NONE;
}

/*
 *  Convert exponent 0..15 to usec using:
 *   value = 1usec * 2 ^ exponent
 */
STATIC int 
_bcm_tucana_hw_exponent_to_usec[] = {
    1,     /* 0 */
    2,     /* 1 */
    4,     /* 2 */
    8,     /* 3 */
    16,    /* 4 */
    32,    /* 5 */
    64,    /* 6 */
    128,   /* 7 */
    256,   /* 8 */
    512,   /* 9 */
    1024,  /* 10 */
    2048,  /* 11 */
    4096,  /* 12 */
    8192,  /* 13 */ 
    16384, /* 14 */
    32768, /* 15 */
};      
 
STATIC int
_bcm_tucana_usec_to_exponent(int usec) {
   int i;
 
   for (i=15; i > 0 ; i--) {
      if (_bcm_tucana_hw_exponent_to_usec[i] <= usec) break;
   }
   return i;
}

STATIC int
_bcm_tucana_exponent_to_usec(int exp) {
   return (_bcm_tucana_hw_exponent_to_usec[exp]);
}

/*
 * Each angle value from 0..90 degrees is approximated
 * by mapping to one of the 15 possible HW slope values.
 * This array contains the lowest angle value which has a
 * slope greater than or equal to the each HW slope value.
 * Used by _bcm_tucana_angle_to_slope()
 */
STATIC int
_bcm_tucana_hw_slope_to_angle[] = {
    0,  /* 0.000000 */
    4,  /* 0.062500 */
    6,  /* 0.093750 */
    8,  /* 0.125000 */
    11, /* 0.187500 */
    15, /* 0.250000 */
    21, /* 0.375000 */
    27, /* 0.500000 */
    37, /* 0.750000 */
    45, /* 1.000000 */
    57, /* 1.500000 */
    64, /* 2.000000 */
    72, /* 3.000000 */
    76, /* 4.000000 */
    81, /* 6.000000 */
    83, /* 8.000000 */
};

STATIC int
_bcm_tucana_angle_to_slope(int angle) {
    int i;

    for (i=15; i > 0 ; i--) {
        if (_bcm_tucana_hw_slope_to_angle[i] <= angle) break;
    }
    return i;
}

STATIC int
_bcm_tucana_slope_to_angle(int slope) {
   return (_bcm_tucana_hw_slope_to_angle[slope]);
}

/*
 * Function:
 *      bcm_tucana_cosq_discard_port_set
 * Purpose:
 *      Set the COS queue WRED parameters
 * Parameters:
 *      unit  - Tucana unit number.
 *      port  - port to configure (-1 for all ports).
 *      cosq  - COS queue to configure (-1 for all queues).
 *      color - BCM_COSQ_DISCARD_COLOR_*
 *      drop_start -  percentage of queue
 *      drop_slope -  degress 0..90
 *      average_time - in microseconds
 *
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_tucana_cosq_discard_port_set(int unit, bcm_port_t port,
                                 bcm_cos_queue_t cosq,
                                 uint32 color,
                                 int drop_start,
                                 int drop_slope,
                                 int average_time)
{
    bcm_port_t          local_port;
    bcm_pbmp_t          pbmp;
    uint32              val, start;
    uint32              limit;
    int                 exp, slope, rc; 
    int                 cng_percent, cos_xq[8];
    int                 idx, cosq_start, num_cosq;
 
    if ((drop_start < 0) || (drop_start > 100) ||
        (drop_slope < 0) || (drop_slope > 90) ||
        (average_time < 0) ||
        (color & BCM_COSQ_DISCARD_COLOR_YELLOW)) {
        return BCM_E_PARAM;
    }
    
    if (port == -1) {
        BCM_PBMP_ASSIGN(pbmp, PBMP_PORT_ALL(unit));
    } else if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    } else {
        BCM_PBMP_PORT_SET(pbmp, port);
    }

    if (cosq < -1 || cosq >= _num_cosq[unit]) {
        return BCM_E_PARAM;
    } else if (cosq == -1) {
        cosq_start = 0;
        num_cosq = _num_cosq[unit];
    } else {
        cosq_start = cosq;
        num_cosq =1;
    }

    exp = _bcm_tucana_usec_to_exponent(average_time);
    slope = _bcm_tucana_angle_to_slope(drop_slope);

    BCM_PBMP_ITER(pbmp, local_port) {
        SOC_IF_ERROR_RETURN(READ_WREDAVERAGINGTIMEr(unit, local_port, &val));
        soc_reg_field_set(unit, WREDAVERAGINGTIMEr, &val, EXPf, exp);
        SOC_IF_ERROR_RETURN(WRITE_WREDAVERAGINGTIMEr(unit, local_port, val));

        /*
         * Get weighted per-cos XQ limits
         * as previously programmed in mmu init
         */
        rc = _bcm_tucana_cosq_get_cos_xq(unit, local_port, cos_xq);
        if (rc != BCM_E_NONE) {
            return rc;
        }

        for (idx = cosq_start; idx < cosq_start + num_cosq; idx++) {
            limit = cos_xq[idx];
            if (color & BCM_COSQ_DISCARD_COLOR_RED) {
                SOC_IF_ERROR_RETURN
                    (READ_EGRPKTLIMITCNGCOSr(unit, local_port, idx, &val));
                cng_percent = soc_reg_field_get(unit, EGRPKTLIMITCNGCOSr,
                                                val, PKTSETLIMITf);
                start = drop_start * limit / 100;
                if (cng_percent == 0x2) {
                    /* 75% of XQ limit */
                    start = (start * 3) / 4;
                } else if (cng_percent == 0x1) {
                    /* 50% of XQ limit */
                    start /= 2;
                } else if (cng_percent == 0x0) {
                    /* 25% of XQ limit */
                    start /= 4;
                }
                SOC_IF_ERROR_RETURN
                    (READ_WREDCNGPARAMETERCOSr(unit, local_port, idx, &val));
                soc_reg_field_set(unit, WREDCNGPARAMETERCOSr, &val,
                                  DROPSTARTPOINTf, start);
                soc_reg_field_set(unit, WREDCNGPARAMETERCOSr, &val, SLOPEf,
                                  slope);
                SOC_IF_ERROR_RETURN
                    (WRITE_WREDCNGPARAMETERCOSr(unit, local_port, idx, val));
            }
            if (!color || (color & BCM_COSQ_DISCARD_COLOR_GREEN)) {
                start = (limit * drop_start) / 100;
                SOC_IF_ERROR_RETURN
                    (READ_WREDPARAMETERCOSr(unit, local_port, idx, &val));
                soc_reg_field_set(unit, WREDPARAMETERCOSr, &val,
                                  DROPSTARTPOINTf, start);
                soc_reg_field_set(unit, WREDPARAMETERCOSr, &val, SLOPEf,
                                  slope);
                SOC_IF_ERROR_RETURN
                    (WRITE_WREDPARAMETERCOSr(unit, local_port, idx, val));
            }
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_tucana_cosq_discard_port_get
 * Purpose:
 *      Set the COS queue WRED parameters
 * Parameters:
 *      unit  - Tucana unit number.
 *      port  - port to get (-1 for any).
 *      cosq  - COS queue to get (-1 for any).
 *      color - BCM_COSQ_DISCARD_COLOR_*
 *      drop_start - (OUT) percentage of queue
 *      drop_slope - (OUT) degress 0..90
 *      average_time - (OUT) in microseconds
 *
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_tucana_cosq_discard_port_get(int unit, bcm_port_t port,
                                 bcm_cos_queue_t cosq,
                                 uint32 color,
                                 int *drop_start,
                                 int *drop_slope,
                                 int *average_time)
{
    bcm_port_t          local_port;
    bcm_pbmp_t          pbmp;
    uint32              val, start, limit;
    int                 exp, slope, cng_percent;
    int                 rc, cos_xq[8];

    if (port == -1) {
        BCM_PBMP_ASSIGN(pbmp, PBMP_PORT_ALL(unit));
    } else if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    } else {
        BCM_PBMP_PORT_SET(pbmp, port);
    }

    if (cosq < -1 || cosq >= _num_cosq[unit]) {
        return BCM_E_PARAM;
    } else if (cosq == -1) {
        cosq = 0; 
    }   
 
    BCM_PBMP_ITER(pbmp, local_port) {
        SOC_IF_ERROR_RETURN(READ_WREDAVERAGINGTIMEr(unit, local_port, &val));
        exp = soc_reg_field_get(unit, WREDAVERAGINGTIMEr, val, EXPf);
 
        /*
         * Get weighted per-cos XQ limits
         * as previously programmed in mmu init
         */
        rc = _bcm_tucana_cosq_get_cos_xq(unit, local_port, cos_xq);
        if (rc != BCM_E_NONE) {
            return rc;
        }
        limit = cos_xq[cosq];

        val = 0;
        switch (color) {
        case BCM_COSQ_DISCARD_COLOR_YELLOW:
            return BCM_E_UNAVAIL;
            break;
        case BCM_COSQ_DISCARD_COLOR_RED:
            SOC_IF_ERROR_RETURN
                (READ_EGRPKTLIMITCNGCOSr(unit, local_port, cosq, &val));
            cng_percent = soc_reg_field_get(unit, EGRPKTLIMITCNGCOSr,
                                            val, PKTSETLIMITf);
            if (cng_percent == 0x2) {
               /* 75% of XQ limit */
               limit = (limit * 3) / 4;
            } else if (cng_percent == 0x1) {
               /* 50% of XQ limit */
               limit /= 2;
            } else if (cng_percent == 0x0) {
               /* 25% of XQ limit */
               limit /= 4;
            }

            SOC_IF_ERROR_RETURN
                    (READ_WREDCNGPARAMETERCOSr(unit, local_port, cosq, &val));
            start = soc_reg_field_get(unit, WREDCNGPARAMETERCOSr,
                                      val, DROPSTARTPOINTf);
            slope = soc_reg_field_get(unit, WREDCNGPARAMETERCOSr, 
                                      val, SLOPEf);
            break;
        default: /* BCM_COSQ_DISCARD_COLOR_GREEN */
            SOC_IF_ERROR_RETURN
                    (READ_WREDPARAMETERCOSr(unit, local_port, cosq, &val));
            start = soc_reg_field_get(unit, WREDPARAMETERCOSr, 
                                      val, DROPSTARTPOINTf);
            slope = soc_reg_field_get(unit, WREDPARAMETERCOSr, 
                                      val, SLOPEf);
            break;
        }

        *drop_start = (start * 100) / limit;
        *drop_slope = _bcm_tucana_slope_to_angle(slope);
        *average_time = _bcm_tucana_exponent_to_usec(exp);
        break;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_tucana_cosq_detach
 * Purpose:
 *      Discard all COS schedule/mapping state.
 * Parameters:
 *      unit - StrataSwitch unit number.
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_tucana_cosq_detach(int unit, int software_state_only)
{
    bcm_port_t         port;

    if (_tucana_wfqminbwcos[unit] != NULL) {
        /* Port bitmap may have changed since alloc,
         * iterate over all possible */
        for (port = 0; port < SOC_MAX_NUM_PORTS; port++) {
            if (_tucana_wfqminbwcos[unit][port] != NULL) {
                sal_free(_tucana_wfqminbwcos[unit][port]);
            }
        }
        sal_free(_tucana_wfqminbwcos[unit]);
        _tucana_wfqminbwcos[unit] = NULL;
    }

    return BCM_E_NONE;
}

#ifdef BCM_WARM_BOOT_SUPPORT
int
bcm_tucana_cosq_sync(int unit)
{
    return BCM_E_UNAVAIL;
}
#endif /* BCM_WARM_BOOT_SUPPORT */

#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
void
bcm_tucana_cosq_sw_dump(int unit)
{
    soc_cm_print("\nSW Information COSQ - Unit %d\n", unit);
    soc_cm_print("    Number: %d\n", _num_cosq[unit]);

    return;
}
#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */
