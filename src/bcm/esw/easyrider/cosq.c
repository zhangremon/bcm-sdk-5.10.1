/*
 * $Id: cosq.c 1.32 Broadcom SDK $
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
    
#include <soc/defs.h>
#if defined(BCM_EASYRIDER_SUPPORT)
#include <sal/core/libc.h>

#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/easyrider.h>

#include <bcm/error.h>
#include <bcm/cosq.h> 
 
#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/easyrider.h>
#include <bcm_int/esw/firebolt.h>

#define ER_NUM_COS  8

/* WFQMINBWCOS per port, per queue */
static int **_er_wfqminbwcos[SOC_MAX_NUM_DEVICES];

static int _num_cosq[SOC_MAX_NUM_DEVICES];
static int _xq_weights[ER_NUM_COS];

#define WFQMINBWCOS(_unit_, _port_, _cosq_) \
        _er_wfqminbwcos[_unit_][_port_][_cosq_]

/*
 * Ingress Back Pressure Config Settings
 * These ought to be settable through properties or a bcm API.
 */
#define ER_IBP_PKT_SPREAD     4  /* pkt backpressure for up to 4 into 1 */
#define ER_IBP_RESET_PERCENT  50 /* % of IBP set limit for IBP reset limit */
#define ER_IBP_MAX_LIMIT      0x3ffff /* max IBP limit value */
#define ER_XQ_LIMIT_MAX       0xff


/*
 * Function:
 *      bcm_er_cosq_init
 * Purpose:
 *      Initialize (clear) all COS schedule/mapping state.
 * Parameters:
 *      unit - StrataSwitch unit number.
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_er_cosq_init(int unit)
{
    bcm_port_t  port;
    int         alloc_size, num_cos, cosq;
    uint32      val;

    BCM_IF_ERROR_RETURN
        (bcm_er_cosq_detach(unit, 0));

    for (cosq = 0; cosq < ER_NUM_COS; cosq++) {
        _xq_weights[cosq] = 
            soc_property_suffix_num_get(unit, cosq, spn_MMU_XQ_WEIGHT,
                                        "cos",  1);
    }

    _er_wfqminbwcos[unit] =
        sal_alloc(SOC_MAX_NUM_PORTS * sizeof (int *),
                  "COS WFQ min BW by ports");
    if (_er_wfqminbwcos[unit] == NULL) {
        return BCM_E_MEMORY;
    }

    alloc_size = NUM_COS(unit) * sizeof (int);

    PBMP_ALL_ITER(unit, port) {
        _er_wfqminbwcos[unit][port] =
            sal_alloc(alloc_size, "COS WFQ min BW by ports by COS queue");
        if (_er_wfqminbwcos[unit][port] == NULL) {
            return BCM_E_MEMORY;
        }
        sal_memset(_er_wfqminbwcos[unit][port], 0, alloc_size);
    }

    /* Clean WFQMINBWCOS state */
    PBMP_ALL_ITER(unit, port) {
        for (cosq = 0; cosq < NUM_COS(unit); cosq++) {
            WFQMINBWCOS(unit, port, cosq) =
                BCM_ER_COSQ_PORT_BW_MIN_DFLT;
            SOC_IF_ERROR_RETURN
                (READ_WFQMINBWCOSr(unit, port, cosq, &val));
            soc_reg_field_set(unit, WFQMINBWCOSr, &val,
                              MIN_BWf, BCM_ER_COSQ_PORT_BW_MIN_DFLT);
            SOC_IF_ERROR_RETURN
                (WRITE_WFQMINBWCOSr(unit, port, cosq, val));
        }
    }

    /* Clean Discard state */
    PBMP_ALL_ITER(unit, port) {
        for (cosq = 0; cosq < NUM_COS(unit); cosq++) {
            SOC_IF_ERROR_RETURN
                (READ_WREDPARAMYELCOSr(unit, port, cosq, &val));
            soc_reg_field_set(unit, WREDPARAMYELCOSr, &val, DROPSTARTPOINTf, 0);
            soc_reg_field_set(unit, WREDPARAMYELCOSr, &val, SLOPEf, 0);
            SOC_IF_ERROR_RETURN
                (WRITE_WREDPARAMYELCOSr(unit, port, cosq, val));

            SOC_IF_ERROR_RETURN
                (READ_WREDPARAMREDCOSr(unit, port, cosq, &val));
            soc_reg_field_set(unit, WREDPARAMREDCOSr, &val, DROPSTARTPOINTf, 0);
            soc_reg_field_set(unit, WREDPARAMREDCOSr, &val, SLOPEf, 0);
            SOC_IF_ERROR_RETURN
                (WRITE_WREDPARAMREDCOSr(unit, port, cosq, val));

            SOC_IF_ERROR_RETURN
                (READ_WREDPARAMCOSr(unit, port, cosq, &val));
            soc_reg_field_set(unit, WREDPARAMCOSr, &val, DROPSTARTPOINTf, 0);
            soc_reg_field_set(unit, WREDPARAMCOSr, &val, SLOPEf, 0);
            SOC_IF_ERROR_RETURN
                (WRITE_WREDPARAMCOSr(unit, port, cosq, val));
        }
    }

    num_cos = soc_property_get(unit, spn_BCM_NUM_COS, BCM_COS_DEFAULT);

    if (num_cos < 1) {
        num_cos = 1;
    } else if (num_cos > NUM_COS(unit)) {
        num_cos = NUM_COS(unit);
    }

    return bcm_er_cosq_config_set(unit, num_cos);
}

#define ER_XQ_LIMIT_OVERSUB     2
#define ER_XQ_MINENTRY_PERCENT  75
#define ER_XQ_YELLOW_PERCENT    75
#define ER_XQ_RED_PERCENT       50

/*
 * Initial mmu programming for the 5660x
 * The higig port gets a few more resources. 
 */
STATIC int
_bcm_er_cosq_config_set(int unit, int numq)
{
    int         ncells, nports, xq_per_port, cos;
    int         ncells_per_port, xq_factor, total_weights;
    int         my_mod_id, mod_64_mode, module, num_mod_ports, num_mods;
    int         xq_pool, xq_min_unit, xq_max_unit;
    uint32      val, limit, smallest_xq_limit=0;
    uint32      cos_xq_set_limit, cos_xq_min_entry;
    mmu_ingpktcelllimitibp_entry_t   ipclibp;
    soc_port_t  port;
    soc_mem_t   xq_mem[] = {    MMU_XQ0m,
                                MMU_XQ1m,
                                MMU_XQ2m,
                                MMU_XQ3m,
                                MMU_XQ4m,
                                MMU_XQ5m,
                                MMU_XQ6m,
                                MMU_XQ7m,
                                MMU_XQ8m,
                                MMU_XQ9m,
                                MMU_XQ10m,
                                MMU_XQ11m,
                                MMU_XQ12m,
                                MMU_XQ13m
                            };

    SOC_IF_ERROR_RETURN
        (soc_er_num_cells(unit, &ncells));
    /* 
     * Assume only 90% of CBP is actually available
     */
    
    ncells = (ncells * 90) / 100;
    nports = NUM_ALL_PORT(unit);

    SOC_IF_ERROR_RETURN
        (READ_E2E_MODULE_CONFIGr(unit, &val));
    my_mod_id = soc_reg_field_get(unit, E2E_MODULE_CONFIGr, 
                                  val, MY_MODULE_IDf); 
    mod_64_mode = soc_reg_field_get(unit, E2E_MODULE_CONFIGr, 
                                    val, MOD_64_MODEf); 
    num_mods = mod_64_mode ? 64 : 32;
    num_mod_ports = mod_64_mode ? 16 : 32;

    total_weights = 0;
    for (cos = 0; cos < numq; cos++) {
        total_weights += _xq_weights[cos];
    }

    PBMP_ALL_ITER(unit, port) {
        ncells_per_port = ncells / (nports + 1); /* add 1 - higig gets extra */
        if (IS_HG_PORT(unit, port)) {   /* higig gets an extra helping */
            ncells_per_port *= 2;
        }

        /*
         * Set port HOL cell reset limits to 50%.
         */
        SOC_IF_ERROR_RETURN
            (READ_RESETLIMITSr(unit, port, &val));
        soc_reg_field_set(unit, RESETLIMITSr, &val,
                          EGRCELLRESETLIMITf, 1); /* 50% of cell limit */
        SOC_IF_ERROR_RETURN
            (WRITE_RESETLIMITSr(unit, port, val));

        /* 
         * The limits are programmed as units of XQ multiples.
         * A unit equals 'xq_factor' XQ entries.
         */
        if (IS_HG_PORT(unit, port)) {
            xq_factor = 32;
        } else {
            xq_factor = 16;
        }

        /*
         * Number of transactions allocated to each port,
         * shared between that port's active cosqs.
         */
        xq_per_port = soc_mem_index_count(unit, xq_mem[port]);

        /*
         * ER_XQ_MINENTRY_PERCENT of the total port XQ's are divided
         * among each cosq for minimum XQ guarantees. The remaining
         * are available in a dynamic pool of XQ's. To improve the
         * chance that cosq's may use more of the dynamic pool, the 
         * cos XQ limits are calculated with an over-subscription value, 
         * ER_XQ_LIMIT_OVERSUB.
         */
        xq_pool = (xq_per_port * ER_XQ_MINENTRY_PERCENT) / 100;
        xq_min_unit = xq_pool / total_weights;

        xq_pool = xq_per_port * ER_XQ_LIMIT_OVERSUB;
        xq_max_unit = xq_pool / total_weights;
        
        /*
         * Initialize all COSQs, even the inactive ones.
         */
        for (cos = 0; cos < 8; cos++) {
            /*
             * Set the cell limits.
             */
            SOC_IF_ERROR_RETURN
                (READ_EGRCELLLIMITCOSr(unit, port, cos, &val));
            if (cos < numq) {
                /* 
                 * Divide the cells evenly among each cosq. 
                 */
                limit = ncells_per_port / numq;
            } else {
                limit = ncells_per_port - 1; /* Max limit (disabled) */
            }
            soc_reg_field_set(unit, EGRCELLLIMITCOSr, &val,
                              CELLSETLIMITf, limit);
            SOC_IF_ERROR_RETURN
                (WRITE_EGRCELLLIMITCOSr(unit, port, cos, val));

            
            if (numq == 1) {
                /*
                 * Only one cosq, use max values.
                 */
                cos_xq_min_entry = xq_per_port;
                cos_xq_set_limit = xq_per_port;
            } else {
                cos_xq_min_entry = xq_min_unit * _xq_weights[cos];
                cos_xq_set_limit = xq_max_unit * _xq_weights[cos];
            }
    
            /*
             * Keep track of the smallest egress port allocation (dynamic plus 
             * static) of XQ's. Later this will be used for the IBP calculations.
             */
            if (!smallest_xq_limit || (cos_xq_set_limit < smallest_xq_limit)) {
                smallest_xq_limit = cos_xq_set_limit;
            }

            /*
             * Set the packet limits.
             */
            SOC_IF_ERROR_RETURN
                (READ_EGRPKTLIMITCOSr(unit, port, cos, &val));
            if (cos < numq) {
                limit = cos_xq_set_limit / xq_factor;
                if (limit > ER_XQ_LIMIT_MAX) {
                    limit = ER_XQ_LIMIT_MAX;
                }
                soc_reg_field_set(unit, EGRPKTLIMITCOSr, &val,
                                  XQSETLIMITf, limit);

                limit = cos_xq_min_entry / xq_factor;
                if (limit > ER_XQ_LIMIT_MAX) {
                    limit = ER_XQ_LIMIT_MAX;
                }
                soc_reg_field_set(unit, EGRPKTLIMITCOSr, &val,
                                  XQMINENTRYf, limit);
                /*
                 * Calculate the Yellow and Red XQ limits. Calculate as 
                 * a fraction of 'cos_xq_min_entry' so that only the "green"
                 * colored packets make use of the dynamically available XQ's.
                 */
                limit = (cos_xq_min_entry * ER_XQ_YELLOW_PERCENT) / 100;
                limit /= xq_factor;
                if (limit > ER_XQ_LIMIT_MAX) {
                    limit = ER_XQ_LIMIT_MAX;
                }
                soc_reg_field_set(unit, EGRPKTLIMITCOSr, &val,
                                  XQYELLIMITf, limit);

                limit = (cos_xq_min_entry * ER_XQ_RED_PERCENT) / 100;
                limit /= xq_factor;
                if (limit > ER_XQ_LIMIT_MAX) {
                    limit = ER_XQ_LIMIT_MAX;
                }
                soc_reg_field_set(unit, EGRPKTLIMITCOSr, &val,
                                  XQREDLIMITf, limit);
            } else {
                limit = ER_XQ_LIMIT_MAX; /* Max limit (disabled) */
                soc_reg_field_set(unit, EGRPKTLIMITCOSr, &val,
                                  XQSETLIMITf, limit);
                soc_reg_field_set(unit, EGRPKTLIMITCOSr, &val,
                                  XQMINENTRYf, limit);
                soc_reg_field_set(unit, EGRPKTLIMITCOSr, &val,
                                  XQYELLIMITf, limit);
                soc_reg_field_set(unit, EGRPKTLIMITCOSr, &val,
                                  XQREDLIMITf, limit);
            }
            SOC_IF_ERROR_RETURN
                (WRITE_EGRPKTLIMITCOSr(unit, port, cos, val));
        }
    }

    /*
     * Set the IBP limits for the local ports on this module.
     */
    SOC_IF_ERROR_RETURN
        (READ_INGLIMITDISCARDr(unit, SOC_ER_MMU_LIMITS_PTR_LOCAL, &val));
    limit = ER_IBP_MAX_LIMIT; /* max discard limit value */
    soc_reg_field_set(unit, INGLIMITDISCARDr, &val, DISCARDLIMITf, limit);
    SOC_IF_ERROR_RETURN
        (WRITE_INGLIMITDISCARDr(unit, SOC_ER_MMU_LIMITS_PTR_LOCAL, val));

    SOC_IF_ERROR_RETURN
        (READ_INGLIMITr(unit, SOC_ER_MMU_LIMITS_PTR_LOCAL, &val));
    limit = smallest_xq_limit / ER_IBP_PKT_SPREAD;
    soc_reg_field_set(unit, INGLIMITr, &val, IBPLIMITf, limit);
    SOC_IF_ERROR_RETURN
        (WRITE_INGLIMITr(unit, SOC_ER_MMU_LIMITS_PTR_LOCAL, val));

    SOC_IF_ERROR_RETURN
        (READ_INGLIMITRESETr(unit, SOC_ER_MMU_LIMITS_PTR_LOCAL, &val));
    limit = (limit * ER_IBP_RESET_PERCENT) / 100;
    soc_reg_field_set(unit, INGLIMITRESETr, &val, RESETLIMITf, limit);
    SOC_IF_ERROR_RETURN
        (WRITE_INGLIMITRESETr(unit, SOC_ER_MMU_LIMITS_PTR_LOCAL, val));

    module = my_mod_id;
    PBMP_ALL_ITER(unit, port) {
        sal_memset(&ipclibp, 0, sizeof(ipclibp));
        if (port == CMIC_PORT(unit)) {
            soc_mem_field32_set(unit, MMU_INGPKTCELLLIMITIBPm, &ipclibp, PTRf,
                                SOC_ER_MMU_LIMITS_PTR_CMIC);
        } else {
            soc_mem_field32_set(unit, MMU_INGPKTCELLLIMITIBPm, &ipclibp, PTRf,
                                SOC_ER_MMU_LIMITS_PTR_LOCAL);
        }
        SOC_IF_ERROR_RETURN
            (WRITE_MMU_INGPKTCELLLIMITIBPm(unit, MEM_BLOCK_ALL,
                       port + (module * num_mod_ports), &ipclibp));
    }

    /*
     * Set the IBP limits for the remote ports on all other modules.
     */
    SOC_IF_ERROR_RETURN
        (READ_INGLIMITDISCARDr(unit, SOC_ER_MMU_LIMITS_PTR_REMOTE, &val));
    limit = ER_IBP_MAX_LIMIT; /* max discard limit value */
    soc_reg_field_set(unit, INGLIMITDISCARDr, &val, DISCARDLIMITf, limit);
    SOC_IF_ERROR_RETURN
        (WRITE_INGLIMITDISCARDr(unit, SOC_ER_MMU_LIMITS_PTR_REMOTE, val));

    SOC_IF_ERROR_RETURN
        (READ_INGLIMITr(unit, SOC_ER_MMU_LIMITS_PTR_REMOTE, &val));
    limit = smallest_xq_limit / ER_IBP_PKT_SPREAD;
    soc_reg_field_set(unit, INGLIMITr, &val, IBPLIMITf, limit);
    SOC_IF_ERROR_RETURN
        (WRITE_INGLIMITr(unit, SOC_ER_MMU_LIMITS_PTR_REMOTE, val));

    SOC_IF_ERROR_RETURN
        (READ_INGLIMITRESETr(unit, SOC_ER_MMU_LIMITS_PTR_REMOTE, &val));
    limit = (limit * ER_IBP_RESET_PERCENT) / 100;
    soc_reg_field_set(unit, INGLIMITRESETr, &val, RESETLIMITf, limit);
    SOC_IF_ERROR_RETURN
        (WRITE_INGLIMITRESETr(unit, SOC_ER_MMU_LIMITS_PTR_REMOTE, val));

    for (module = 0; module < num_mods; module++) {
        if (module == my_mod_id) {
            continue;
        }
        sal_memset(&ipclibp, 0, sizeof(ipclibp));
        soc_mem_field32_set(unit, MMU_INGPKTCELLLIMITIBPm, &ipclibp, PTRf,
                            SOC_ER_MMU_LIMITS_PTR_REMOTE);
        for (port = 0; port < num_mod_ports; port++) {
            SOC_IF_ERROR_RETURN
                (WRITE_MMU_INGPKTCELLLIMITIBPm(unit, MEM_BLOCK_ALL,
                           port + (module * num_mod_ports), &ipclibp));
        }
    }

    return BCM_E_NONE;
}


/*
 * Function:
 *      bcm_er_cosq_config_set
 * Purpose:
 *      Set the number of COS queues
 * Parameters:
 *      unit - Draco unit number.
 *      numq - number of COS queues (1-8).
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_er_cosq_config_set(int unit, int numq)
{
    int         cos, prio, ratio, remain;

    SOC_IF_ERROR_RETURN(_bcm_er_cosq_config_set(unit, numq));

    /* Map the eight 802.1 priority levels to the active cosqs */
    ratio = 8 / numq;
    remain = 8 % numq;
    cos = 0;
    for (prio = 0; prio < 8; prio++) {
        BCM_IF_ERROR_RETURN
            (bcm_fb_er_cosq_mapping_set(unit, -1, prio, cos));
        if ((prio + 1) == (((cos + 1) * ratio) +
                           ((remain < (numq - cos)) ? 0 :
                            (remain - (numq - cos) + 1)))) {
            cos++;
        }
    }

#ifdef BCM_COSQ_HIGIG_MAP_DISABLE
    SOC_IF_ERROR_RETURN(soc_cosq_stack_port_map_disable(unit));
#endif

    /* use identity mapping for CPU priority mapping */
    SOC_IF_ERROR_RETURN(soc_cpu_priority_mapping_init(unit));

    _num_cosq[unit] = numq;

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_er_cosq_config_get
 * Purpose:
 *      Get the number of cos queues
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      numq - (Output) number of cosq
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_er_cosq_config_get(int unit, int *numq)
{
    if (_num_cosq[unit] == 0) {
        return BCM_E_INIT;
    }

    if (numq != NULL) {
        *numq = _num_cosq[unit];
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *      bcm_er_cosq_port_sched_set
 * Purpose:
 *      Set up class-of-service policy and corresponding weights and delay
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      pbm - port bitmap
 *      mode - Scheduling mode, one of BCM_COSQ_xxx
 *      weights - Weights for each COS queue
 *                Unused if mode is BCM_COSQ_STRICT.
 *      delay - This parameter is not used in 5660x
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_er_cosq_port_sched_set(int unit, bcm_pbmp_t pbm,
                           int mode, const int weights[], int delay)
{
    int                 port, t, cos;
    uint32              val;
    int                 mbits = 0;
    int                 rbit = 0;
    int                 wfq_sp_mask = 0;

    COMPILER_REFERENCE(delay);

    switch (mode) {
    case BCM_COSQ_STRICT:
        mbits = 0;
        wfq_sp_mask = 0xff;
        break;
    case BCM_COSQ_ROUND_ROBIN:
        mbits = 1;
        break;
    case BCM_COSQ_WEIGHTED_ROUND_ROBIN:
        mbits = 3;
        /*
         * All weight values must fit within 4 bits.
         * If weight is 0, this queue is run in strict mode,
         * others run in WRR mode.
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
            }
        }
        break;
    case BCM_COSQ_DEFICIT_ROUND_ROBIN:  /* not supported in 5660x */
    case BCM_COSQ_BOUNDED_DELAY:        /* not supported in xgs */
    default:
        return BCM_E_PARAM;
    }

    PBMP_ITER(pbm, port) {
        if (mbits == 0) {
            for (cos = 0; cos < 8; cos++) {
                SOC_IF_ERROR_RETURN
                    (READ_WFQMINBWCOSr(unit, port, cos, &val));
                if (mode == BCM_COSQ_WEIGHTED_FAIR_QUEUING) {
                    soc_reg_field_set(unit, WFQMINBWCOSr, &val,
                                      MIN_BWf, weights[cos]);
                } else {
                    soc_reg_field_set(unit, WFQMINBWCOSr, &val,
                                      MIN_BWf, WFQMINBWCOS(unit, port, cos));
                }
                SOC_IF_ERROR_RETURN
                    (WRITE_WFQMINBWCOSr(unit, port, cos, val));
            }
        } else if (mode == BCM_COSQ_WEIGHTED_ROUND_ROBIN) {
            for (cos = 0; cos < 8; cos++) {
                SOC_IF_ERROR_RETURN
                    (READ_WFQMINBWCOSr(unit, port, cos, &val));
                soc_reg_field_set(unit, WFQMINBWCOSr, &val,
                                  MIN_BWf, weights[cos]);
                SOC_IF_ERROR_RETURN
                    (WRITE_WFQMINBWCOSr(unit, port, cos, val));
            }
        }

        /* Set scheduler mode */
        SOC_IF_ERROR_RETURN(READ_WFQCONFIG_GENERALr(unit, port, &val));
        soc_reg_field_set(unit, WFQCONFIG_GENERALr, &val, 
                          SCHEDULER_MODEf, mbits);
        soc_reg_field_set(unit, WFQCONFIG_GENERALr, &val, 
                          RELATIVE_WFQf, rbit);

        soc_reg_field_set(unit, WFQCONFIG_GENERALr, &val, 
                          BUCKET_MAXf, 0x6);
        soc_reg_field_set(unit, WFQCONFIG_GENERALr, &val, 
                          BUCKET_MIN_HIGHf, 0x6);
        soc_reg_field_set(unit, WFQCONFIG_GENERALr, &val, 
                          BUCKET_MIN_LOWf, 0x5);
        SOC_IF_ERROR_RETURN(WRITE_WFQCONFIG_GENERALr(unit, port, val));

        /* Set WFQ_SP_MASKs in WFQ+SP mode */
        if (mbits == 0) {
            SOC_IF_ERROR_RETURN(READ_WFQCONFIG_MASKSr(unit, port, &val));
            soc_reg_field_set(unit, WFQCONFIG_MASKSr, &val, 
                              WFQ_SP_MASKf, wfq_sp_mask);
            SOC_IF_ERROR_RETURN(WRITE_WFQCONFIG_MASKSr(unit, port, val));
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_er_cosq_port_sched_get
 * Purpose:
 *      Retrieve class-of-service policy and corresponding weights and delay
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      pbm - port bitmap
 *      mode_ptr - (output) Scheduling mode, one of BCM_COSQ_xxx
 *      weights - (output) Weights for each COS queue
 *              Unused if mode is BCM_COSQ_STRICT.
 *      delay - This parameter is not used in 5660x
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_er_cosq_port_sched_get(int unit, bcm_pbmp_t pbm,
                           int *mode, int weights[], int *delay)
{
    uint32              sched_config, wfq, wfq_config;
    int                 port, cos, mbits = -1;
    int                 wfq_sp_mask;

    PBMP_ITER(pbm, port) {
        /* Set scheduler mode */
        SOC_IF_ERROR_RETURN(READ_WFQCONFIG_GENERALr(unit, port, &sched_config));
        mbits = soc_reg_field_get(unit, WFQCONFIG_GENERALr,
                                  sched_config, SCHEDULER_MODEf); 
        break;
    }

    switch (mbits) {
    case 0:
        /* port is still valid from the loop above */
        SOC_IF_ERROR_RETURN(READ_WFQCONFIG_MASKSr(unit, port, &wfq_config));
        wfq_sp_mask = soc_reg_field_get(unit, WFQCONFIG_MASKSr,
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
 *      bcm_er_cosq_sched_weight_max_get
 * Purpose:
 *      Retrieve maximum weights for given COS policy.
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      mode - Scheduling mode, one of BCM_COSQ_xxx
 *      weight_max - (output) Maximum weight for COS queue.
 *              0 if mode is BCM_COSQ_STRICT.
 *              1 if mode is BCM_COSQ_ROUND_ROBIN.
 *              -1 if not applicable to mode.
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_er_cosq_sched_weight_max_get(int unit, int mode,
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

#define ER_BW_GRANULARITY        64
#define ER_BW_FIELD_MAX          0x3ffff

/*
 * Function:
 *      bcm_er_cosq_port_bandwidth_set
 * Purpose:
 *      Set bandwidth values for given COS policy.
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      port - port to configure.
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
bcm_er_cosq_port_bandwidth_set(int unit, bcm_port_t port,
                               bcm_cos_queue_t cosq,
                               uint32 kbits_sec_min,
                               uint32 kbits_sec_max,
                               uint32 kbits_sec_burst,
                               uint32 flags)
{
    uint32              sched_config, wfq_config, wfqmin, shapermax;
    int                 mbits = -1;
    uint32              field_work;

    if (kbits_sec_burst != kbits_sec_max) {
        return BCM_E_PARAM;
    }

    /* WFQCONFIG_MASKS settings from flags */
    SOC_IF_ERROR_RETURN(
        READ_WFQCONFIG_MASKSr(unit, port, &wfq_config));

    field_work = soc_reg_field_get(unit, WFQCONFIG_MASKSr, 
                                   wfq_config, WFQ_PRIOS_MIN_BW_MASKf);
    if (flags & BCM_COSQ_BW_MINIMUM_PREF) {
        field_work |= 1 << cosq;
    } else {
        field_work &= ~(1 << cosq);
    }
    soc_reg_field_set(unit, WFQCONFIG_MASKSr, &wfq_config, 
                      WFQ_PRIOS_MIN_BW_MASKf, field_work);

    field_work = soc_reg_field_get(unit, WFQCONFIG_MASKSr, 
                                   wfq_config, WFQ_PRIOS_MAX_BW_MASKf);
    if (flags & BCM_COSQ_BW_EXCESS_PREF) {
        field_work |= 1 << cosq;
    } else {
        field_work &= ~(1 << cosq);
    }
    soc_reg_field_set(unit, WFQCONFIG_MASKSr, &wfq_config, 
                      WFQ_PRIOS_MAX_BW_MASKf, field_work);
    SOC_IF_ERROR_RETURN(
        WRITE_WFQCONFIG_MASKSr(unit, port, wfq_config));

    if (kbits_sec_max > (ER_BW_FIELD_MAX * ER_BW_GRANULARITY)) {
        field_work = ER_BW_FIELD_MAX;
    } else {
        field_work =
            (kbits_sec_max + ER_BW_GRANULARITY - 1) / ER_BW_GRANULARITY;
    }
    /* SHAPERMAXBWCOS setting from kbits_sec_max */
    SOC_IF_ERROR_RETURN
        (READ_SHAPERMAXBWCOSr(unit, port, cosq, &shapermax));
    soc_reg_field_set(unit, SHAPERMAXBWCOSr, &shapermax, MAX_BWf, field_work);
    SOC_IF_ERROR_RETURN
        (WRITE_SHAPERMAXBWCOSr(unit, port, cosq, shapermax));

    SOC_IF_ERROR_RETURN(READ_WFQCONFIG_GENERALr(unit, port, &sched_config));
    mbits = soc_reg_field_get(unit, WFQCONFIG_GENERALr,
                              sched_config, SCHEDULER_MODEf); 
    if (kbits_sec_min > (ER_BW_FIELD_MAX * ER_BW_GRANULARITY)) {
        field_work = ER_BW_FIELD_MAX;
    } else {
        field_work =
            (kbits_sec_min + ER_BW_GRANULARITY - 1) / ER_BW_GRANULARITY;
    }
    if (mbits == 0) {
        /* WFQ mode, write WFQMINBWCOS value */
        SOC_IF_ERROR_RETURN
            (READ_WFQMINBWCOSr(unit, port, cosq, &wfqmin));
        soc_reg_field_set(unit, WFQMINBWCOSr, &wfqmin, MIN_BWf, field_work);
        SOC_IF_ERROR_RETURN
            (WRITE_WFQMINBWCOSr(unit, port, cosq, wfqmin));
    }

    WFQMINBWCOS(unit, port, cosq) = field_work;

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_er_cosq_port_bandwidth_get
 * Purpose:
 *      Retrieve bandwidth values for given COS policy.
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      port - port to retrieve.
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
bcm_er_cosq_port_bandwidth_get(int unit, bcm_port_t port,
                               bcm_cos_queue_t cosq,
                               uint32 *kbits_sec_min,
                               uint32 *kbits_sec_max,
                               uint32 *kbits_sec_burst,
                               uint32 *flags)
{
    uint32              wfq_config, shapermax;
    uint32              field_work, tmp_flags = 0;

    COMPILER_REFERENCE(kbits_sec_burst);

    /* WFQCONFIG_MASKS settings for flags */
    SOC_IF_ERROR_RETURN(
        READ_WFQCONFIG_MASKSr(unit, port, &wfq_config));

    field_work = soc_reg_field_get(unit, WFQCONFIG_MASKSr,
                                   wfq_config, WFQ_PRIOS_MIN_BW_MASKf);
    if (field_work & (1 << cosq)) {
        tmp_flags |= BCM_COSQ_BW_MINIMUM_PREF;
    }

    field_work = soc_reg_field_get(unit, WFQCONFIG_MASKSr,
                                   wfq_config, WFQ_PRIOS_MAX_BW_MASKf);
    if (field_work & (1 << cosq)) {
        tmp_flags |= BCM_COSQ_BW_EXCESS_PREF;
    }

    SOC_IF_ERROR_RETURN
        (READ_SHAPERMAXBWCOSr(unit, port, cosq, &shapermax));
    field_work = soc_reg_field_get(unit, SHAPERMAXBWCOSr, shapermax, MAX_BWf);

    *flags = tmp_flags;
    *kbits_sec_burst = *kbits_sec_max = field_work * ER_BW_GRANULARITY;
    *kbits_sec_min = WFQMINBWCOS(unit, port, cosq) * ER_BW_GRANULARITY;

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_er_cosq_discard_set
 * Purpose:
 *      Set the COS queue WRED parameters
 * Parameters:
 *      unit  - EasyRider unit number.
 *      flags - BCM_COSQ_DISCARD_*
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_er_cosq_discard_set(int unit, uint32 flags)
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
 *      bcm_er_cosq_discard_get
 * Purpose:
 *      Get the COS queue WRED parameters
 * Parameters:
 *      unit  - EasyRider unit number.
 *      flags - (OUT) BCM_COSQ_DISCARD_*
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_er_cosq_discard_get(int unit, uint32 *flags)
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
_bcm_er_hw_exponent_to_usec[] = {
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
_bcm_er_usec_to_exponent(int usec) {
   int i;

   for (i=15; i > 0 ; i--) {
      if (_bcm_er_hw_exponent_to_usec[i] <= usec) break;
   }
   return i;
}

STATIC int
_bcm_er_exponent_to_usec(int exp) {
   return (_bcm_er_hw_exponent_to_usec[exp]);
}

/*
 * Each angle value from 0..90 degrees is approximated
 * by mapping to one of the 15 possible HW slope values. 
 * This array contains the lowest angle value which has a
 * slope greater than or equal to the each HW slope value.
 * Used by _bcm_er_angle_to_slope()
 */
STATIC int
_bcm_er_hw_slope_to_angle[] = {
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
_bcm_er_angle_to_slope(int angle) {
    int i;

    for (i=15; i > 0 ; i--) {
        if (_bcm_er_hw_slope_to_angle[i] <= angle) break;
    }
    return i;
}

STATIC int
_bcm_er_slope_to_angle(int slope) {
   return (_bcm_er_hw_slope_to_angle[slope]);
}

/*
 * Function:
 *      bcm_er_cosq_discard_port_set
 * Purpose:
 *      Set the COS queue WRED parameters
 * Parameters:
 *      unit  - EasyRider unit number.
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
bcm_er_cosq_discard_port_set(int unit, bcm_port_t port,
                             bcm_cos_queue_t cosq,
                             uint32 color,
                             int drop_start,
                             int drop_slope,
                             int average_time)
{
    bcm_port_t          local_port;
    bcm_pbmp_t          pbmp;
    uint32              val, start;
    uint32              pkt_limit, limit;
    int                 exp, slope, xq_factor;
    int                 idx, cosq_start, num_cosq;

    if ((drop_start < 0) || (drop_start > 100) ||
        (drop_slope < 0) || (drop_slope > 90) ||
        (average_time < 0)) {
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

    exp = _bcm_er_usec_to_exponent(average_time);
    slope = _bcm_er_angle_to_slope(drop_slope);

    BCM_PBMP_ITER(pbmp, local_port) {
        SOC_IF_ERROR_RETURN(READ_WREDAVERAGINGTIMEr(unit, local_port, &val));
        soc_reg_field_set(unit, WREDAVERAGINGTIMEr, &val, EXPf, exp);
        SOC_IF_ERROR_RETURN(WRITE_WREDAVERAGINGTIMEr(unit, local_port, val));

        /*
         * The limits are programmed as units of XQ multiples.
         * A unit equals 'xq_factor' XQ entries.
         */
        if (IS_HG_PORT(unit, local_port)) {
            xq_factor = 32;
        } else {
            xq_factor = 16;
        }

        for (idx = cosq_start; idx < cosq_start + num_cosq; idx++) {
            /* Read the packet limits for this cos */
            SOC_IF_ERROR_RETURN
                (READ_EGRPKTLIMITCOSr(unit, local_port, idx, &pkt_limit));
            if (color & BCM_COSQ_DISCARD_COLOR_YELLOW) {
                limit = soc_reg_field_get(unit, EGRPKTLIMITCOSr, pkt_limit,
                                          XQYELLIMITf);
                start = (limit * xq_factor * drop_start) / 100 / 4;
                SOC_IF_ERROR_RETURN
                    (READ_WREDPARAMYELCOSr(unit, local_port, idx, &val));
                soc_reg_field_set(unit, WREDPARAMYELCOSr, &val,
                                  DROPSTARTPOINTf, start);
                soc_reg_field_set(unit, WREDPARAMYELCOSr, &val, SLOPEf, slope);
                SOC_IF_ERROR_RETURN
                    (WRITE_WREDPARAMYELCOSr(unit, local_port, idx, val));
            }
            if (color & BCM_COSQ_DISCARD_COLOR_RED) {
                limit = soc_reg_field_get(unit, EGRPKTLIMITCOSr, pkt_limit,
                                          XQREDLIMITf);
                start = (limit * xq_factor * drop_start) / 100 / 4;
                SOC_IF_ERROR_RETURN
                    (READ_WREDPARAMREDCOSr(unit, local_port, idx, &val));
                soc_reg_field_set(unit, WREDPARAMREDCOSr, &val,
                                  DROPSTARTPOINTf, start);
                soc_reg_field_set(unit, WREDPARAMREDCOSr, &val, SLOPEf, slope);
                SOC_IF_ERROR_RETURN
                    (WRITE_WREDPARAMREDCOSr(unit, local_port, idx, val));
            }
            if (!color || (color & BCM_COSQ_DISCARD_COLOR_GREEN)) {
                limit = soc_reg_field_get(unit, EGRPKTLIMITCOSr, pkt_limit,
                                          XQMINENTRYf);
                start = (limit * xq_factor * drop_start) / 100 / 4;
                SOC_IF_ERROR_RETURN
                    (READ_WREDPARAMCOSr(unit, local_port, idx, &val));
                soc_reg_field_set(unit, WREDPARAMCOSr, &val, DROPSTARTPOINTf,
                                  start);
                soc_reg_field_set(unit, WREDPARAMCOSr, &val, SLOPEf, slope);
                SOC_IF_ERROR_RETURN
                    (WRITE_WREDPARAMCOSr(unit, local_port, idx, val));
            }
        }
    }
 
    return BCM_E_NONE;
}
 
/*
 * Function:
 *      bcm_er_cosq_discard_port_get
 * Purpose:
 *      Set the COS queue WRED parameters
 * Parameters:
 *      unit  - EasyRider unit number.
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
bcm_er_cosq_discard_port_get(int unit, bcm_port_t port,
                             bcm_cos_queue_t cosq,
                             uint32 color,
                             int *drop_start,
                             int *drop_slope,
                             int *average_time)
{
    bcm_port_t          local_port;
    bcm_pbmp_t          pbmp;
    uint32              val, start;
    uint32              pkt_limit, limit;
    int                 exp, slope, xq_factor;

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

        /* Read the packet limits for this cos */
        SOC_IF_ERROR_RETURN
            (READ_EGRPKTLIMITCOSr(unit, local_port, cosq, &pkt_limit));
        /*
         * The limits are programmed as units of XQ multiples.
         * A unit equals 'xq_factor' XQ entries.
         */
        if (IS_HG_PORT(unit, port)) {
            xq_factor = 32;
        } else {
            xq_factor = 16;
        }

        val = 0;
        switch (color) {
        case BCM_COSQ_DISCARD_COLOR_YELLOW:
            limit = soc_reg_field_get(unit, EGRPKTLIMITCOSr,
                                      pkt_limit, XQYELLIMITf);
            limit *= xq_factor;
            SOC_IF_ERROR_RETURN
                    (READ_WREDPARAMYELCOSr(unit, local_port, cosq, &val));
            start = soc_reg_field_get(unit, WREDPARAMYELCOSr,
                                      val, DROPSTARTPOINTf);
            slope = soc_reg_field_get(unit, WREDPARAMYELCOSr,
                                      val, SLOPEf);
            break;
        case BCM_COSQ_DISCARD_COLOR_RED:
            limit = soc_reg_field_get(unit, EGRPKTLIMITCOSr,
                                      pkt_limit, XQREDLIMITf);
            limit *= xq_factor;
            SOC_IF_ERROR_RETURN
                    (READ_WREDPARAMREDCOSr(unit, local_port, cosq, &val));
            start = soc_reg_field_get(unit, WREDPARAMREDCOSr,
                                      val, DROPSTARTPOINTf);
            slope = soc_reg_field_get(unit, WREDPARAMREDCOSr,
                                      val, SLOPEf);
            break;
        default: /* BCM_COSQ_DISCARD_COLOR_GREEN */
            limit = soc_reg_field_get(unit, EGRPKTLIMITCOSr,
                                      pkt_limit, XQMINENTRYf);
            limit *= xq_factor;
            SOC_IF_ERROR_RETURN
                    (READ_WREDPARAMCOSr(unit, local_port, cosq, &val));
            start = soc_reg_field_get(unit, WREDPARAMCOSr,
                                      val, DROPSTARTPOINTf);
            slope = soc_reg_field_get(unit, WREDPARAMCOSr,
                                      val, SLOPEf);
            break;
        }

        *drop_start = (start * 100 * 4) / limit;
        *drop_slope = _bcm_er_slope_to_angle(slope);
        *average_time = _bcm_er_exponent_to_usec(exp);
        break;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_er_cosq_detach
 * Purpose:
 *      Discard all COS schedule/mapping state.
 * Parameters:
 *      unit - StrataSwitch unit number.
 * Returns:
 *      BCM_E_XXX
 */

int
bcm_er_cosq_detach(int unit, int software_state_only)
{
    bcm_port_t         port;

    if (_er_wfqminbwcos[unit] != NULL) {
        PBMP_ALL_ITER(unit, port) {
            if (_er_wfqminbwcos[unit][port] != NULL) {
                sal_free(_er_wfqminbwcos[unit][port]);
            }
        }
        sal_free(_er_wfqminbwcos[unit]);
        _er_wfqminbwcos[unit] = NULL;
    }

    return BCM_E_NONE;
}

#ifdef BCM_WARM_BOOT_SUPPORT
int
bcm_er_cosq_sync(int unit)
{
    return BCM_E_UNAVAIL;
}
#endif /* BCM_WARM_BOOT_SUPPORT */

#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
void
bcm_er_cosq_sw_dump(int unit)
{
    soc_cm_print("\nSW Information COSQ - Unit %d\n", unit);
    soc_cm_print("    Number: %d\n", _num_cosq[unit]);

    return;
}
#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */

#else  /* BCM_EASYRIDER_SUPPORT */
int bcm_esw_easyrider_cosq_not_empty;
#endif /* BCM_EASYRIDER_SUPPORT */
