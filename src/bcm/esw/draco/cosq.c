/*
 * $Id: cosq.c 1.33 Broadcom SDK $
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
#include <bcm_int/esw/draco.h>

#define	IBPCELL_PERCENT	90

static int _num_cosq[SOC_MAX_NUM_DEVICES];

/*
 * Function:
 *      bcm_draco_cosq_init
 * Purpose:
 *      Initialize (clear) all COS schedule/mapping state.
 * Parameters:
 *      unit - StrataSwitch unit number.
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_draco_cosq_init(int unit)
{
    int		num_cos;

    num_cos = soc_property_get(unit, spn_BCM_NUM_COS, BCM_COS_DEFAULT);

    if (num_cos < 1) {
	num_cos = 1;
    } else if (num_cos > NUM_COS(unit)) {
	num_cos = NUM_COS(unit);
    }

    return bcm_draco_cosq_config_set(unit, num_cos);
}

STATIC int
_bcm_draco1_cosq_config_set(int unit, int numq)
{
    int		ncells, nports, xq_per_port, cos, n;
    soc_port_t	port;
    uint32	val, limit;
    int         prev_end;

    soc_reg_t _cos_range_reg[2] = {
      XQCOSRANGE3_0r, XQCOSRANGE7_4r
    };
    soc_field_t _cos_endaddress_field[8] = {
      COS0_ENDADDRESSf, COS1_ENDADDRESSf, COS2_ENDADDRESSf, COS3_ENDADDRESSf, 
      COS4_ENDADDRESSf, COS5_ENDADDRESSf, COS6_ENDADDRESSf, COS7_ENDADDRESSf
    };
    int cos_weight[8];

    ncells = (1024*1024) / 128;	/* 1MB split into 128 bytes cells: 8192 */
    nports = NUM_ALL_PORT(unit);

    /*
     * Number of transactions allocated to each port,
     * shared between that port's active cosqs.
     */
    xq_per_port = soc_mem_index_count(unit, MMU_MEMORIES2_XQ0m);

    /* Draco MMU setup for (static) CBP allocation */

    /* Get the cosq XQ weights (in sixteenths of all cosq's XQs) */
    prev_end = 0;
    for (cos = 0; cos < 8; cos++) {
        uint32 addr_end;
        SOC_IF_ERROR_RETURN
	  (soc_reg32_get(unit, _cos_range_reg[cos/4],
				       REG_PORT_ANY, cos,
			  &val));
	addr_end = soc_reg_field_get(unit, _cos_range_reg[cos/4], 
				     val,
				     _cos_endaddress_field[cos]);
	cos_weight[cos] = addr_end - prev_end;
	prev_end = addr_end;

	/* Make sure numq will work with the cosq weighting. */
	if ((cos < numq) && !cos_weight[cos]) {
	    return BCM_E_PARAM;
	}
    }

    PBMP_ALL_ITER(unit, port) {
	for (cos = 0; cos < 8; cos++) {
	    /*
	     * Set to "fair share" value, so no single (low priority) 
	     * cosq can starve another (higher priority) cosq.
	     */
	    if (cos < numq) {
	        limit = ncells / numq / nports;
	    }
	    else {
	        limit = ncells - 1; /* Max limit (disabled) */
	    }
	    val = 0;
	    soc_reg_field_set(unit, HOLCOSCELLSETLIMITr, &val,
			      CELLSETLIMITf, limit);
	    SOC_IF_ERROR_RETURN
		(WRITE_HOLCOSCELLSETLIMITr(unit, port, cos, val));

	    /*
	     * HOLCELLRESETLIMIT left at default
	     * (50% of HOLCOSCELLSETLIMIT)
	     */
	    val = 0;
	    soc_reg_field_set(unit, IBPCOSPKTSETLIMITr, &val,
			      PKTSETLIMITf, xq_per_port-1);
	    SOC_IF_ERROR_RETURN
		(WRITE_IBPCOSPKTSETLIMITr(unit, port, cos, val));

	    /* 
	     * Set this limit to match how the XQs are split
	     * between the cosqs. 
	     * Must be 0x40 less than the XQ size for the cosq
	     * in order to enable proper HOL functionality.
	     * Also, must be greater than CNGCOSPKTLIMITr[cos].
	     */
	    if (cos < numq) {
	        limit = (xq_per_port*cos_weight[cos]/16) - 0x40;
	    }
	    else {
	        limit = xq_per_port - 1; /* Max limit (disabled) */
	    }
	    val = 0;
	    soc_reg_field_set(unit, HOLCOSPKTSETLIMITr, &val,
			      PKTSETLIMITf, limit); 
	    SOC_IF_ERROR_RETURN
		(WRITE_HOLCOSPKTSETLIMITr(unit, port, cos, val));

	}

	/*
	 * set ingress back pressure to a percentage of fair share of cells
	 */
	if (port == CMIC_PORT(unit)) {
	    n = ncells-1;
	} else {
	    n = (ncells / nports / numq) * IBPCELL_PERCENT / 100;
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

    /* CNGCOSPKTLIMIT is a genreg on 5690, portreg on 5695 */
    for (cos = 0; cos < 8; cos++) {
        /* 
	 * Set this limit based on how the XQs are split between 
	 * the COSQs. 
	 * Must be smaller than the HOLCOSPKTSETLIMITr value.
	 * For active ports, use half of the HOLCOSPKTSETLIMITr 
	 * value for this cosq (all ports are programmed identically).
	 * Set inactive ports with the maximum limit.
	 */

        if (cos < numq) {
	    limit = (xq_per_port * cos_weight[cos] / 32) - 0x20;
	}
	else {
	    limit = xq_per_port - 1; /* Max limit (disabled) */
	}

        val = 0;
        soc_reg_field_set(unit, CNGCOSPKTLIMITr, &val, CNGPKTSETLIMITf,
			  limit);
        SOC_IF_ERROR_RETURN(WRITE_CNGCOSPKTLIMITr(unit, cos, val));
    }

    return BCM_E_NONE;
}

/*
 * Initial mmu programming for the 5695.
 * Always run the 5695 in dynamic mode, with DYNCELL_PERCENT of
 * the the cells in the dynamic pool.  The higig port (if any)
 * gets a few more resources.
 */
#define	DYNCELL_PERCENT	50

STATIC int
_bcm_draco15_cosq_config_set(int unit, int numq)
{
    int		ncells, nports, xq_per_port, cos;
    int		n, used_cells;
    soc_port_t	port;
    uint32	val, regval, limit;

    ncells = (1024*1024) / 128;	/* 1MB split into 128 bytes cells: 8192 */
    nports = NUM_ALL_PORT(unit);

    /*
     * Number of transactions allocated to each port,
     * shared between that port's active cosqs.
     */
    xq_per_port = soc_mem_index_count(unit, MMU_MEMORIES2_XQ0m);

    /* set static limits first */
    used_cells = 0;
    PBMP_ALL_ITER(unit, port) {
	n = ncells / nports / numq;
	/* part of each non-higig port's cells go into the dynamic pool */
	if (!IS_HG_PORT(unit, port)) {
	    n = n * (100 - DYNCELL_PERCENT) / 100;
	}
	val = 0;
	soc_reg_field_set(unit, LWMCOSCELLSETLIMITr, &val,
			  CELLSETLIMITf, n);

	for (cos = 0; cos < numq; cos++) {
	    SOC_IF_ERROR_RETURN
		(WRITE_LWMCOSCELLSETLIMITr(unit, port, cos, val));
	    used_cells += n;
	}
    }

    /* remaining cells are dynamic */
    val = 0;
    soc_reg_field_set(unit, TOTALDYNCELLLIMITr,
		      &val, TOTALDYNCELLLIMITf, ncells - used_cells);
    SOC_IF_ERROR_RETURN(WRITE_TOTALDYNCELLLIMITr(unit, val));

    PBMP_ALL_ITER(unit, port) {
	n = (ncells - used_cells) / (nports + 1);
	if (IS_HG_PORT(unit, port)) {	/* higig gets an extra helping */
	    n *= 2;
	}

	val = 0;
	soc_reg_field_set(unit, DYNCELLLIMITr, &val,
			  DYNCELLRESETLIMITSELf, 4);	/* 100% of LWM */
	soc_reg_field_set(unit, DYNCELLLIMITr, &val,
			  DYNCELLLIMITf, n);
	SOC_IF_ERROR_RETURN(WRITE_DYNCELLLIMITr(unit, port, val));

	/*
	 * Divide weighted per-port packet limits into each cos.
	 * Initialize all COSQs, even the inactive ones.
	 */
	for (cos = 0; cos < 8; cos++) {

	    /* 
	     * Set this limit based on how the XQs are split between 
	     * the COSQs. 
	     * Must be smaller than the HOLCOSPKTSETLIMITr value.
	     * Use half of the HOLCOSPKTSETLIMITr value.
	     */

	    SOC_IF_ERROR_RETURN
	      (READ_HOLCOSPKTSETLIMITr(unit, port, cos, &regval));
	    val = soc_reg_field_get(unit, HOLCOSPKTSETLIMITr, regval,
				    PKTSETLIMITf);

	    /* Make sure numq will work with the cosq weighting. */
	    if ((cos < numq) && (val == 4)) {
	        /* 
		 * This means that this COSQ has been 
		 * disabled due to XQ weighting, but
		 * has been selected as active. Config error. 
		 */
		return BCM_E_PARAM;
	    }

	    if (cos < numq) {
	        limit = val/2;
	    }
	    else {
	        limit = xq_per_port - 1; /* Max limit (disabled) */
	    }

	    regval = 0;
	    soc_reg_field_set(unit, CNGCOSPKTLIMITr, &regval,
			      CNGPKTSETLIMITf, limit);
	    /*
	     * should be
	     *	WRITE_CNGCOSPKTLIMITr(unit, port, cos, regval)
	     * but the 5690 genreg definition takes precedence
	     */
	    SOC_IF_ERROR_RETURN
		(soc_reg32_set(unit,
				 CNGCOSPKTLIMITr,
					      port, cos,
				 regval));
	}

	/*
	 * set ingress back pressure to a percentage of fair share of cells
	 */
	if (port == CMIC_PORT(unit)) {
	    n = ncells-1;
	} else {
	    n = (ncells / nports / numq) * IBPCELL_PERCENT / 100;
	}
	
        val = 0;
        soc_reg_field_set(unit, IBPCELLSETLIMITr, &val,
                          CELLSETLIMITf, n);
        SOC_IF_ERROR_RETURN(WRITE_IBPCELLSETLIMITr(unit, port, val));

        val = 0;
        soc_reg_field_set(unit, IBPDISCARDSETLIMITr, &val,
                          DISCARDSETLIMITf, ncells-1);
        SOC_IF_ERROR_RETURN(WRITE_IBPDISCARDSETLIMITr(unit, port, val));

	val = 0;
	soc_reg_field_set(unit, IBPPKTSETLIMITr, &val,
			  PKTSETLIMITf, xq_per_port-1);
	SOC_IF_ERROR_RETURN(WRITE_IBPPKTSETLIMITr(unit, port, val));

    }

    /* flip into dynamic mode */
    SOC_IF_ERROR_RETURN(READ_MISCCONFIGr(unit, &val));
    soc_reg_field_set(unit, MISCCONFIGr, &val, DRACOMODEENf, 0);
    SOC_IF_ERROR_RETURN(WRITE_MISCCONFIGr(unit, val));

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_draco_cosq_config_set
 * Purpose:
 *      Set the number of COS queues
 * Parameters:
 *      unit - Draco unit number.
 *      numq - number of COS queues (1-8).
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_draco_cosq_config_set(int unit, int numq)
{
    int		cos, prio, ratio, remain;

    if (SOC_IS_DRACO15(unit)) {
	SOC_IF_ERROR_RETURN(_bcm_draco15_cosq_config_set(unit, numq));
    } else {
	SOC_IF_ERROR_RETURN(_bcm_draco1_cosq_config_set(unit, numq));
    }

    /* Map the eight 802.1 priority levels to the active cosqs */
    ratio = 8 / numq;
    remain = 8 % numq;
    cos = 0;
    for (prio = 0; prio < 8; prio++) {
        BCM_IF_ERROR_RETURN
            (bcm_draco_cosq_mapping_set(unit, -1, prio, cos));
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
 *      bcm_draco_cosq_mapping_set
 * Purpose:
 *      Set which cosq a given priority should fall into
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      priority - Priority value to map
 *      cosq - COS queue to map to
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_draco_cosq_mapping_set(int unit, bcm_port_t port,
			   bcm_cos_t priority, bcm_cos_queue_t cosq)
{
    uint64 cval64, oval64;
    soc_field_t f;
    bcm_pbmp_t ports;
    
    switch (priority) {
    case 0:	f = COS0f; break;
    case 1:	f = COS1f; break;
    case 2:	f = COS2f; break;
    case 3:	f = COS3f; break;
    case 4:	f = COS4f; break;
    case 5:	f = COS5f; break;
    case 6:	f = COS6f; break;
    case 7:	f = COS7f; break;
    default:	return BCM_E_PARAM;
    }

    if (cosq < 0 || cosq >= NUM_COS(unit)) {
        return (BCM_E_PARAM);
    }

    if (port == -1) {	/* all ports */
        BCM_PBMP_ASSIGN(ports, PBMP_PORT_ALL(unit));
    } else if (SOC_PORT_VALID(unit, port) && IS_PORT(unit, port)) {
        BCM_PBMP_CLEAR(ports);
	    BCM_PBMP_PORT_ADD(ports, port);
    } else {
        return BCM_E_PORT;
    }

    PBMP_ITER(ports, port) {
    	SOC_IF_ERROR_RETURN(READ_COS_SELr(unit, port, &cval64));
    	oval64 = cval64;
    	soc_reg64_field32_set(unit, COS_SELr, &cval64, f, cosq);
    	if (COMPILER_64_NE(cval64, oval64)) {
    	    SOC_IF_ERROR_RETURN(WRITE_COS_SELr(unit, port, cval64));
    	}
#ifndef BCM_COSQ_HIGIG_MAP_DISABLE
    	if (IS_HG_PORT(unit, port)) {
    	    SOC_IF_ERROR_RETURN(READ_ICOS_SELr(unit, port, &cval64));
    	    oval64 = cval64;
    	    soc_reg64_field32_set(unit, ICOS_SELr, &cval64, f, cosq);
    	    if (COMPILER_64_NE(cval64, oval64)) {
                SOC_IF_ERROR_RETURN(WRITE_ICOS_SELr(unit, port, cval64));
    	    }
    	}
#endif
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_draco_cosq_mapping_get
 * Purpose:
 *      Determine which COS queue a given priority currently maps to.
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      priority - Priority value
 *      cosq - (Output) COS queue number
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_draco_cosq_mapping_get(int unit, bcm_port_t port,
			   bcm_cos_t priority, bcm_cos_queue_t *cosq)
{
    uint64 cval64;
    soc_field_t f;

    switch (priority) {
    case 0:	f = COS0f; break;
    case 1:	f = COS1f; break;
    case 2:	f = COS2f; break;
    case 3:	f = COS3f; break;
    case 4:	f = COS4f; break;
    case 5:	f = COS5f; break;
    case 6:	f = COS6f; break;
    case 7:	f = COS7f; break;
    default:	return BCM_E_PARAM;
    }

    if (port == -1) {
	port = REG_PORT_ANY;
    } else if (!SOC_PORT_VALID(unit, port) || !IS_E_PORT(unit, port)) {
	return BCM_E_PORT;
    }

    SOC_IF_ERROR_RETURN(READ_COS_SELr(unit, port, &cval64));
    *cosq = soc_reg64_field32_get(unit, COS_SELr, cval64, f);

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_draco_cosq_config_get
 * Purpose:
 *      Get the number of cos queues
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      numq - (Output) number of cosq
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_draco_cosq_config_get(int unit, int *numq)
{
    if (_num_cosq[unit] == 0) {
	return BCM_E_INIT;
    }

    if (numq != NULL) {
        *numq = _num_cosq[unit];
    }

    return (BCM_E_NONE);
}

static soc_field_t _draco_cosarb_port2field[] = {
    PORT0_COSARBf,	/* 0 - ge0 */
    PORT1_COSARBf,	/* 1 - ge1 */
    PORT2_COSARBf,	/* 2 - ge2 */
    PORT3_COSARBf,	/* 3 - ge3 */
    PORT4_COSARBf,	/* 4 - ge4 */
    PORT5_COSARBf,	/* 5 - ge5 */
    PORT6_COSARBf,	/* 6 - ge6 */
    PORT7_COSARBf,	/* 7 - ge7 */
    PORT8_COSARBf,	/* 8 - ge8 */
    PORT9_COSARBf,	/* 9 - ge9 */
    PORT10_COSARBf,	/* 10 - ge10 */
    PORT11_COSARBf,	/* 11 - ge11 */
    PORTIPIC_COSARBf,	/* 12 - hg0 */
    PORTCPU_COSARBf,	/* 13 - cpu0 */
};

/*
 * Function:
 *      bcm_draco_cosq_port_sched_set
 * Purpose:
 *      Set up class-of-service policy and corresponding weights and delay
 * Parameters:
 *      unit - StrataSwitch unit number.
 *	pbm - port bitmap
 *      mode - Scheduling mode, one of BCM_COSQ_xxx
 *	weights - Weights for each COS queue
 *		  Only for BCM_COSQ_WEIGHTED_FAIR_ROUND_ROBIN mode.
 *	delay - This parameter is not used in 5690
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_draco_cosq_port_sched_set(int unit, bcm_pbmp_t pbm,
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
	if (port >= COUNTOF(_draco_cosarb_port2field)) {
	    return BCM_E_PARAM;
	}
	soc_reg_field_set(unit, COSARBSELr, &cosarbsel,
			  _draco_cosarb_port2field[port], mbits);
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
 *      bcm_draco_cosq_port_sched_get
 * Purpose:
 *      Retrieve class-of-service policy and corresponding weights and delay
 * Parameters:
 *      unit     - StrataSwitch unit number.
 *	pbm	 - port bitmap
 *      mode     - (output) Scheduling mode, one of BCM_COSQ_xxx
 *	weights  - (output) Weights for each COS queue
 *		            Only for BCM_COSQ_WEIGHTED_ROUND_ROBIN mode.
 *	delay    - This parameter is not used in 5690
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *	Actually just returns data for the first port in the bitmap
 */

int
bcm_draco_cosq_port_sched_get(int unit, bcm_pbmp_t pbm,
			      int *mode, int weights[], int *delay)
{
    uint32		cosarbsel, wfq;
    int			mbits, port;

    mbits = -1;
    SOC_IF_ERROR_RETURN(READ_COSARBSELr(unit, &cosarbsel));
    PBMP_ITER(pbm, port) {
	if (port >= COUNTOF(_draco_cosarb_port2field)) {
	    return BCM_E_PARAM;
	}
	mbits = soc_reg_field_get(unit, COSARBSELr, cosarbsel,
				  _draco_cosarb_port2field[port]);
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

/*
 * Function:
 *      bcm_draco_cosq_sched_weight_max_get
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
bcm_draco_cosq_sched_weight_max_get(int unit, int mode, int *weight_max)
{
    switch (mode) {
    case BCM_COSQ_STRICT:
	*weight_max = BCM_COSQ_WEIGHT_STRICT;
	break;
    case BCM_COSQ_ROUND_ROBIN:
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

int
bcm_draco_cosq_port_bandwidth_set(int unit, bcm_port_t port,
                                  bcm_cos_queue_t cosq,
                                  uint32 kbits_sec_min,
                                  uint32 kbits_sec_max,
                                  uint32 kbits_sec_burst,
                                  uint32 flags)
{
    return BCM_E_UNAVAIL;
}

int
bcm_draco_cosq_port_bandwidth_get(int unit, bcm_port_t port,
                                  bcm_cos_queue_t cosq,
                                  uint32 *kbits_sec_min,
                                  uint32 *kbits_sec_max,
                                  uint32 *kbits_sec_burst,
                                  uint32 *flags)
{
    return BCM_E_UNAVAIL;
}

int     
bcm_draco_cosq_discard_set(int unit, uint32 flags)
{
    return BCM_E_UNAVAIL;
}

int
bcm_draco_cosq_discard_get(int unit, uint32 *flags)
{
    return BCM_E_UNAVAIL;
}

int
bcm_draco_cosq_discard_port_set(int unit, bcm_port_t port,
                                bcm_cos_queue_t cosq,
                                uint32 color,
                                int drop_start,
                                int drop_slope,
                                int average_time)
{
    return BCM_E_UNAVAIL;
}

int
bcm_draco_cosq_discard_port_get(int unit, bcm_port_t port,
                                bcm_cos_queue_t cosq,
                                uint32 color,
                                int *drop_start,
                                int *drop_slope,
                                int *average_time)
{
    return BCM_E_UNAVAIL;
}

int
bcm_draco_cosq_detach(int unit, int software_state_only)
{
    return BCM_E_NONE;
}

#ifdef BCM_WARM_BOOT_SUPPORT
int
bcm_draco_cosq_sync(int unit)
{
    return BCM_E_UNAVAIL;
}
#endif /* BCM_WARM_BOOT_SUPPORT */

#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
void
bcm_draco_cosq_sw_dump(int unit)
{
    soc_cm_print("\nSW Information COSQ - Unit %d\n", unit);
    soc_cm_print("    Number: %d\n", _num_cosq[unit]);

    return;
}
#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */
