/*
 * $Id: cosq.c 1.25.52.1 Broadcom SDK $
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
 * BM3200 Fabric Control API
 */

#include <soc/debug.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_pt_auto.h>
#include <soc/sbx/bme3200.h>
#include <soc/sbx/bm3200_init.h>
#include <soc/sbx/fabric/sbZfFabWredParameters.hx>
#include <soc/sbx/fabric/sbZfFabBm3200WredDataTableEntry.hx>
#include <bcm_int/sbx/cosq.h>
#include <bcm_int/sbx/device_wred.h>

#include <bcm/error.h>
#include <bcm/port.h>
#include <bcm/cosq.h>
#include <bcm_int/sbx/cosq.h>
#include <bcm_int/sbx/fabric.h>
#include <bcm_int/sbx/state.h>
#include <bcm/debug.h>

static int
_bcm_bm3200_cosq_update_bag(int unit,
			    int bw_group, bcm_sbx_cosq_queue_region_type_t queue_region);

static int
_bcm_bm3200_get_nbr_node_enabled(int unit, int *nbr_node_enabled);


int
bcm_bm3200_cosq_init(int unit)
{
    int rv = BCM_E_NONE;
    int32_t bw_group;
#ifdef BCM_EASY_RELOAD_SUPPORT
    bcm_sbx_cosq_queue_state_t *p_qstate;
    int32_t num_sp_queues, num_queues_in_bag, base_queue, bag_rate_bytes_per_epoch;
    bcm_sbx_cosq_bw_group_state_t *p_bwstate;
#endif

    /* Read PRT table from the hardware if in EASY_RELOAD mode
     * write queue to bw_group_state index and write bw_group index to the base queue_state.
     */
    if (SOC_IS_RELOADING(unit)) {
#ifdef BCM_EASY_RELOAD_SUPPORT

	for (bw_group = 0; bw_group < SOC_SBX_CFG(unit)->num_bw_groups; bw_group++) {

	    p_bwstate = (bcm_sbx_cosq_bw_group_state_t*)SOC_SBX_STATE(unit)->bw_group_state;
	    p_bwstate = &p_bwstate[bw_group];

	    p_qstate = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;

	    /* Read PRT */
	    rv = soc_bm3200_prt_read(unit, bw_group, &num_sp_queues, &num_queues_in_bag,
				     &base_queue, &bag_rate_bytes_per_epoch);

	    if (rv != SOC_E_NONE) {
		BCM_ERR(("ERROR: BM3200 read of PRT table failed for bw_group(%d)\n", bw_group));
		return BCM_E_FAIL;
	    }
	    if (base_queue < SOC_SBX_CFG(unit)->num_queues) {
		p_bwstate->base_queue = base_queue;
		p_qstate[base_queue].bw_group = bw_group;
		BCM_DEBUG(BCM_DBG_COSQ, ("EASY_RELOAD bw_group(%d) base_queue(%d)\n", bw_group, base_queue));
	    }
	}
#endif
    } else {
    /* If we are not reloading, then invalidate all PRT entries base_queue during cosq init */
    /* This is how we know the entry is not in use when reloading                           */
	for (bw_group = 0; bw_group < SOC_SBX_CFG(unit)->num_bw_groups; bw_group++) {

	    rv = soc_bm3200_prt_write(unit, bw_group, 0 /* num_sp_queues */, 0 /* num_queues_in_bag */,
				      0x3fffff /* base_queue */ , 0 /* bag rate */);

	    if (rv != SOC_E_NONE) {
		BCM_ERR(("ERROR: BM3200 write of PRT table failed for bw_group(%d)\n", bw_group));
		return BCM_E_FAIL;
	    }
	}
    }


    return rv;
}

extern int
bcm_bm3200_cosq_add_queue(int unit,
                          int queue,
                          bcm_sbx_cosq_queue_region_type_t queue_region,
			  int sysport,
                          int eset,
			  int dest_node,
			  int dest_port,
			  int dest_mc,
			  int dest_cos,
			  int32 dest_type,
			  bcm_sbx_cosq_queue_params_ingress_t *p_qparams, /* unused for BM3200 */
			  bcm_sbx_cosq_bw_group_params_t *p_bwparams,     /* unused for BM3200 */
                          int inhibit_write)                              /* unused for BM3200 */

{
    int rv = BCM_E_NONE;
    bcm_sbx_cosq_queue_state_t *queue_state;
    int32_t bw_group;

    queue_state = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;

    bw_group = queue_state[queue].bw_group;

    rv = _bcm_bm3200_cosq_update_bag(unit, bw_group, queue_region);

    if (rv) {
	BCM_ERR(("ERROR: add_queue BM3200 configuration bw_group(%d) error(%d)\n", bw_group, rv));
	return rv;
    }
    return rv;
}



extern int
bcm_bm3200_cosq_delete_queue(int unit,
                             int queue,
                             bcm_sbx_cosq_queue_region_type_t queue_region)
{
    int rv = BCM_E_NONE;

    return rv;
}

int
bcm_bm3200_cosq_set_ingress_params(int unit,
				   int32_t queue,
                                   bcm_sbx_cosq_queue_region_type_t queue_region,
				   bcm_sbx_cosq_queue_params_ingress_t *p_newqparams,
				   bcm_sbx_cosq_queue_params_ingress_t *p_oldqparams,
				   bcm_sbx_cosq_bw_group_params_t      *p_newbwparams,
				   bcm_sbx_cosq_bw_group_params_t      *p_oldbwparams,
                                   bcm_sbx_queue_size_info_t           *p_sizeInfo) {

    int rv = BCM_E_NONE;
    bcm_sbx_cosq_queue_state_t *queue_state;
    int32_t bw_group;

    BCM_DEBUG(BCM_DBG_COSQ, ("Set ingress params for queue(%d)\n", queue));
    queue_state = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;

    bw_group = queue_state[queue].bw_group;

    /* Parameters are updated with respect to the ingress parameters in */
    /* the queue state structure and the bw_group structure, not those  */
    /* passed in above                                                  */
    rv = _bcm_bm3200_cosq_update_bag(unit, bw_group, queue_region);

    if (rv) {
	BCM_ERR(("ERROR: set ingress params BM3200 configuration bw_group(%d) error(%d)\n", bw_group, rv));
	return rv;
    }
    return rv;

}

int
bcm_bm3200_cosq_set_template_gain(int unit,
				  int queue,
				  int template,
				  int gain)
{
    int rv = BCM_E_NONE;
    int32_t read_data = 0;
    int32_t addr;
    sbZfFabBm3200WredDataTableEntry_t zfWdtEntry;
    int32_t write_value = 0;
    int status;

    BCM_DEBUG(BCM_DBG_COSQ, ("Update WRED template, queue(%d) template(%d) gain(%d) on BM3200(%d)\n",
			     queue, template, gain, unit));


    /* this is a 32 bit entry but the field is 16 bits */
    addr = queue >> 1;

    status = soc_bm3200_bw_mem_read(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_WDT,
				    0 /* nRepository */, addr, (uint32_t*)&read_data);

    BCM_DEBUG(BCM_DBG_COSQ, ("WDT entry read(0x%x)\n", read_data));

    if (status == SOC_E_NONE) {

	sbZfFabBm3200WredDataTableEntry_Unpack(&zfWdtEntry, (uint8_t*)&read_data, 4);

	if (queue & 1) {

	    zfWdtEntry.m_nTemplateOdd = template;
	    zfWdtEntry.m_nGainOdd = gain;
	    zfWdtEntry.m_nReservedOdd  = 0;

	}
	else {

	    zfWdtEntry.m_nTemplateEven = template;
	    zfWdtEntry.m_nGainEven = gain;
	    zfWdtEntry.m_nReservedEven  = 0;
	}

	sbZfFabBm3200WredDataTableEntry_Pack(&zfWdtEntry, (uint8_t*)&write_value, 4);
	BCM_DEBUG(BCM_DBG_COSQ, ("WDT entry value to write(0x%x)\n", write_value));

	status = soc_bm3200_bw_mem_write(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_WDT,
					 0 /* nRepository */, addr, write_value);
    }
    if (status != SOC_E_NONE) {
	BCM_ERR(("ERROR: %s, soc_bm3200_bw_mem_read/write,  Unit(%d)\n", FUNCTION_NAME(), unit));
	rv = BCM_E_INTERNAL;
    }

    return rv;
}

int
bcm_bm3200_cosq_get_template_gain(int unit,
				  int queue,
				  int *template,
				  int *gain)
{
    int rv = BCM_E_NONE;
    int32_t read_data;
    int32_t addr;
    sbZfFabBm3200WredDataTableEntry_t zfWdtEntry;
    int status;


    BCM_DEBUG(BCM_DBG_COSQ, ("Retreive WRED template, queue(%d) on BM3200(%d)\n", queue, unit));

    /* this is a 32 bit entry but the field is 16 bits */
    addr = queue >> 1;

    status = soc_bm3200_bw_mem_read(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_WDT,
				                       0, addr, (uint32_t*)&read_data);

    if (status == SOC_E_NONE) {

	sbZfFabBm3200WredDataTableEntry_Unpack(&zfWdtEntry, (uint8_t*)&read_data, 4);

	if (queue & 1) {
	    (*template) = zfWdtEntry.m_nTemplateOdd;
	    (*gain) = zfWdtEntry.m_nGainOdd;
	}
	else {
	    (*template) = zfWdtEntry.m_nTemplateEven;
	    (*gain) = zfWdtEntry.m_nGainEven;
	}

        BCM_DEBUG(BCM_DBG_COSQ, ("WDT entry read, template(0x%x), gain(0x%x)\n", (*template), (*gain)));
    }

    return rv;
}

int
bcm_bm3200_cosq_set_ingress_shaper(int unit,
				   int base_queue,
				   bcm_cos_queue_t cosq,
				   int num_cos_levels,
				   uint32 shape_limit_kbps,
				   int set_logical_port_shaper,
				   int enable_shaping) {
  int rv = BCM_E_UNAVAIL;
  return rv;
}


int
bcm_bm3200_cosq_gport_discard_set(int unit,
                                  bcm_gport_t gport,
                                  bcm_cos_t priority,
                                  uint32 color,
                                  uint32 template,
                                  uint32 queue_size,
				  uint32 min_queue_size,
                                  bcm_cosq_gport_discard_t *discard)
{
    int                        rv = BCM_E_NONE;
    sbZfFabWredParameters_t    chip_params;
    uint32_t                   write_value[2];
    int32_t                    addr;
    uint32_t                   status;
    int                        nbr_node_enabled;


    if (discard->drop_probability == 0) { /* disabling drop probability */
        addr = template * 8;
        addr = addr + (color * 2);

        write_value[0] = 0xFFFFFFFF;
        status = soc_bm3200_bw_mem_write(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_WCT,
                                                1 /* repository */,  addr, write_value[0]);
        if (status) {
            BCM_ERR(("ERROR: %s, soc_bm3200_bw_mem_write,  Unit(%d)\n", FUNCTION_NAME(), unit));
            rv = BCM_E_INTERNAL;
            goto err;
        }

        status = soc_bm3200_bw_mem_write(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_WCT,
                                                1 /* repository */,  addr+1, write_value[0]);
        if (status) {
            BCM_ERR(("ERROR: %s, soc_bm3200_bw_mem_write,  Unit(%d)\n", FUNCTION_NAME(), unit));
            rv = BCM_E_INTERNAL;
            goto err;
        }
    }

    else {
        rv = _bcm_bm3200_get_nbr_node_enabled(unit, &nbr_node_enabled);
        if (rv != BCM_E_NONE) {
            BCM_ERR(("ERROR: %s, _bcm_bm3200_get_nbr_node_enabled, Unit(%d)\n", FUNCTION_NAME(), unit));
            goto err;
        }

        rv = _bcm_sbx_device_wred_calc_config(unit, SOC_SBX_CFG(unit)->discard_probability_mtu,
					      queue_size, discard, &chip_params);
        if (rv) {
            BCM_ERR(("ERROR: %s, _bcm_sbx_device_wred_calc_config,  Unit(%d)\n",
                                                               FUNCTION_NAME(), unit));
            goto err;
        }

        sbZfFabWredParameters_Pack(&chip_params, (uint8_t *)write_value, SB_ZF_FAB_WRED_PARAMETERS_SIZE);

        BCM_VERB(("Enable WRED dp(%d) entry(%d) on BM3200(%d)\n", color, template, unit));
        BCM_VERB(("Write word0(0x%x) word1(0x%x)\n", write_value[0], write_value[1]));

        /* This is the starting address of the template */
        addr = template * 8;
        addr = addr + (color * 2);

        /* Note that we are writing these entries to the opposite memory          */
        /* write_value[1] is written to address 0 - the common WRED zframe        */
        /* is structured for the standalone mode define, this ordering fixes the  */
        /* issue for the BM3200 */
        status = soc_bm3200_bw_mem_write(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_WCT,
                                                1 /* repository */,  addr, write_value[1]);
        if (status) {
            BCM_ERR(("ERROR: %s, soc_bm3200_bw_mem_write,  Unit(%d)\n", FUNCTION_NAME(), unit));
            rv = BCM_E_INTERNAL;
            goto err;
        }

        status = soc_bm3200_bw_mem_write(unit, SB_FAB_DEVICE_BM3200_BANDWIDTH_TABLE_WCT,
                                                1 /* repository */,  addr+1, write_value[0]);
        if (status) {
            BCM_ERR(("ERROR: %s, soc_bm3200_bw_mem_write,  Unit(%d)\n", FUNCTION_NAME(), unit));
            rv = BCM_E_INTERNAL;
            goto err;
        }
    }

    return(rv);

err:
    return(rv);
}

int
bcm_bm3200_cosq_gport_discard_get(int unit,
                                  bcm_gport_t gport,
                                  bcm_cos_t priority,
                                  uint32 color,
                                  uint32 template,
                                  uint32 queue_size,
				  uint32 min_queue_size,
                                  bcm_cosq_gport_discard_t *discard)
{
    int rv = BCM_E_UNAVAIL;
    return rv;
}

/********************/
/* Static functions */
/********************/
static int
_bcm_bm3200_cosq_update_bag(int unit,
			    int bw_group, bcm_sbx_cosq_queue_region_type_t queue_region)

{
    int rv = BCM_E_NONE;
    bcm_sbx_cosq_queue_state_t *p_qstate;
    bcm_sbx_fabric_state_t *p_fabricstate;
    bcm_sbx_cosq_bw_group_state_t *p_bwstate;
    int gamma = 0, sigma = 0;
    uint32 guarantee_in_kbps = 0;
    uint64_t uu_epoch_length_in_ns = 0;
    uint64_t uu_guarantee_in_bytes_per_sec;
    uint32_t guarantee_in_bytes_per_epoch;
    int32_t bag_rate_kbps;
    uint32_t bag_rate_bytes_per_epoch;
    uint64_t uu_bag_rate_in_bytes_per_sec;
    int32 queue;
    int32 num_sp_queues;
    int32 num_queues_in_bag;
    int32 base_queue = 0;
    int num_queues, start_queue;

    BCM_DEBUG(BCM_DBG_COSQ, ("update bw_group(%d)\n",bw_group));

    p_qstate = (bcm_sbx_cosq_queue_state_t*)SOC_SBX_STATE(unit)->queue_state;
    p_fabricstate = (bcm_sbx_fabric_state_t*)SOC_SBX_STATE(unit)->fabric_state;

    p_bwstate = (bcm_sbx_cosq_bw_group_state_t*)SOC_SBX_STATE(unit)->bw_group_state;
    p_bwstate = &p_bwstate[bw_group];

    rv = bcm_sbx_cosq_bw_group_verify_queues(unit, bw_group);

    if (rv) {
	BCM_ERR(("ERROR: verifying queue setup for bw_group(%d) error(%d)\n", bw_group, rv));
	return rv;
    }

    rv = bcm_sbx_cosq_update_given_weights(unit, bw_group);
    if (rv) {
	BCM_ERR(("ERROR: update WFQ weights failed\n"));
	return rv;
    }

    uu_epoch_length_in_ns = ((SOC_SBX_CFG(unit)->epoch_length_in_timeslots) * (SOC_SBX_STATE(unit)->fabric_state->timeslot_size));

    base_queue = p_bwstate->base_queue;

    /* write queue indexed BWP table */
    for (queue=base_queue; queue < (base_queue + p_bwstate->num_cos); queue++) {

	gamma = 0;
	sigma = 0;

	if (p_qstate[queue].ingress.bw_mode == BCM_COSQ_WEIGHTED_FAIR_QUEUING) {
	    gamma = p_qstate[queue].ingress.given_weight;
	}

	if (p_qstate[queue].ingress.bw_mode== BCM_COSQ_AF) {

	    guarantee_in_kbps = bcm_sbx_cosq_get_bw_guarantee(unit, queue);
	    uu_guarantee_in_bytes_per_sec = guarantee_in_kbps /8;
	    
	    if (soc_sbx_div64((uu_guarantee_in_bytes_per_sec * uu_epoch_length_in_ns), 1000000, &guarantee_in_bytes_per_epoch) == -1) {
		BCM_ERR(("ERROR: update Guarantee failed\n"));
		return BCM_E_INTERNAL;
	    }
            guarantee_in_bytes_per_epoch >>= SOC_SBX_CFG(unit)->demand_scale;

	    sigma = (int32_t) guarantee_in_bytes_per_epoch;

	    BCM_DEBUG(BCM_DBG_COSQ, ("queue(%d) guarantee in bytes/epoch(%d) uu_guarantee_in_bytes_per_sec(%lld) uu_epoch_length_in_ns(%lld)\n",
				     queue, guarantee_in_bytes_per_epoch, uu_guarantee_in_bytes_per_sec, uu_epoch_length_in_ns));

	}

	BCM_DEBUG(BCM_DBG_COSQ, ("queue(%d) guarantee in kbps(%d) sigma(%d) gamma(%d)\n", queue, guarantee_in_kbps, sigma, gamma));

	rv = soc_bm3200_bwp_write(unit, queue, gamma, sigma);

	if (rv != SOC_E_NONE) {
	    BCM_ERR(("ERROR: BM3200 Write to BWP table failed for queue(%d)\n", queue));
	    return BCM_E_FAIL;
	}
    }

    num_sp_queues = bcm_sbx_cosq_get_num_sp_queues(unit, bw_group);
    num_queues_in_bag = bcm_sbx_cosq_get_num_queues_in_bag(unit, bw_group, queue_region, &num_queues, &start_queue);

    BCM_DEBUG(BCM_DBG_COSQ, ("num_sp_queues(%d) num_queues_in_bag(%d)\n",
			     num_sp_queues, num_queues_in_bag));

    bag_rate_kbps = p_bwstate->path.bag_rate_kbps;
    uu_bag_rate_in_bytes_per_sec = bag_rate_kbps / 8;
    
    if (soc_sbx_div64((uu_bag_rate_in_bytes_per_sec * uu_epoch_length_in_ns), 1000000, &bag_rate_bytes_per_epoch) == -1) {
	BCM_ERR(("ERROR: update BAG rate per epoch failed\n"));
	return BCM_E_INTERNAL;
    }
    bag_rate_bytes_per_epoch >>= SOC_SBX_CFG(unit)->demand_scale;

    BCM_DEBUG(BCM_DBG_COSQ, ("bag_rate_kbps=(%d) bag_rate_bytes_per_epoch(%d)\n",
			     bag_rate_kbps, bag_rate_bytes_per_epoch));

    base_queue = p_bwstate->base_queue;

    /* If there is nothing in the bag, clear the PRT entry information, except for the base_queue, we need */
    /* this info for easy reload                                                                           */
    if (num_queues_in_bag == 0) {
	bag_rate_bytes_per_epoch = 0;
	num_sp_queues = 0;
    }

    /* write PRT table used always */
    rv = soc_bm3200_prt_write(unit, bw_group, num_sp_queues, num_queues_in_bag,
			      base_queue, bag_rate_bytes_per_epoch);

    if (rv != SOC_E_NONE) {
	BCM_ERR(("ERROR: BM3200 Write to PRT table failed for bw_group(%d)\n", bw_group));
	return BCM_E_FAIL;
    }
    BCM_DEBUG(BCM_DBG_COSQ, ("Bag update successful for bw_group(%d)\n",bw_group));

    return BCM_E_NONE;
}

static int
_bcm_bm3200_get_nbr_node_enabled(int unit, int *nbr_node_enabled)
{
    int rc = BCM_E_NONE;
    uint32 uData, ina_enable;
    int i;


    (*nbr_node_enabled) = 0;

    uData = SAND_HAL_READ(unit, PT, FO_CONFIG1);
    ina_enable = SAND_HAL_GET_FIELD(PT, FO_CONFIG1, INA_ENABLE, uData);

    for (i = 0; i < 32; i++) {
        if (ina_enable & (1 << i)) {
            (*nbr_node_enabled)++;
        }
    }

    return(rc);
}

int
bcm_bm3200_cosq_gport_sched_config_set(int unit, bcm_gport_t gport,
                                                      int sched_mode, int int_pri, uint32 flags)
{
    int rv = BCM_E_NONE;
    int queue_type, priority, priority2;
    uint32 uData;
    int port;


    rv = soc_sbx_sched_config_set_params_verify(unit, sched_mode, int_pri);
    if (rv != BCM_E_NONE) {
        return(rv);
    }

    rv = soc_sbx_sched_get_internal_state(unit, sched_mode, int_pri,
                                                        &queue_type, &priority, &priority2);
    if (rv != BCM_E_NONE) {
        return(rv);
    }

    for (port = 0; port < SB_FAB_DEVICE_BM3200_NUM_AI_PORTS; port++) {
        uData = SAND_HAL_READ_STRIDE(unit, PT, INA, port, INA0_PRI_FULL_THRESH);
        if (flags & BCM_COSQ_SCHED_CONFIG_EXPEDITE) {
            uData &= ~(0x3 << ((priority - 1) * 2));
            uData |= (0x2 << ((priority - 1) * 2));
        }
        else {
            uData &= ~(0x3 << ((priority - 1) * 2));
            uData |= (0x1 << ((priority - 1) * 2));
        }
        SAND_HAL_WRITE_STRIDE(unit, PT, INA, port, INA0_PRI_FULL_THRESH, uData);
    }

    return(rv);
}

int
bcm_bm3200_cosq_gport_sched_config_get(int unit, bcm_gport_t gport,
                                                      int sched_mode, int int_pri, uint32 *flags)
{
    int rv = BCM_E_NONE;
    int queue_type, priority, priority2;
    uint32 uData;


    rv = soc_sbx_sched_config_params_verify(unit, sched_mode, int_pri);
    if (rv != BCM_E_NONE) {
        return(rv);
    }

    rv = soc_sbx_sched_get_internal_state(unit, sched_mode, int_pri,
                                                        &queue_type, &priority, &priority2);
    if (rv != BCM_E_NONE) {
        return(rv);
    }

    (*flags) = 0;
    uData = SAND_HAL_READ_STRIDE(unit, PT, INA, 0, INA0_PRI_FULL_THRESH);
    if ((uData & (0x3 << ((priority - 1) * 2))) == (0x2 << ((priority - 1) * 2))) {
        (*flags) |= BCM_COSQ_SCHED_CONFIG_EXPEDITE;
    }

    return(rv);
}
