/*
 * $Id: trunk.c 1.31.52.1 Broadcom SDK $
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
 */


#include <sal/types.h>

#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/drv.h>
#include <soc/tucana.h>

#include <bcm/error.h>
#include <bcm/trunk.h>

#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/stack.h>
#include <bcm_int/esw/trunk.h>

#include <bcm_int/esw_dispatch.h>

STATIC pbmp_t
_xgs_getr_pbm(int unit, uint64 data,
	     soc_reg_t reg, soc_field_t fa, soc_field_t f0, soc_field_t f1)
{
    pbmp_t	pbm;

    SOC_PBMP_CLEAR(pbm);
    if (SOC_IS_TUCANA(unit)) {
	SOC_PBMP_WORD_SET(pbm, 0, soc_reg64_field32_get(unit, reg, data, f0));
	SOC_PBMP_WORD_SET(pbm, 1, soc_reg64_field32_get(unit, reg, data, f1));
    } else {
	SOC_PBMP_WORD_SET(pbm, 0, soc_reg64_field32_get(unit, reg, data, fa));
    }
    return pbm;
}

STATIC void
_xgs_setr_pbm(int unit, uint64 *datap, pbmp_t pbm,
	     soc_reg_t reg, soc_field_t fa, soc_field_t f0, soc_field_t f1)
{
    if (SOC_IS_TUCANA(unit)) {
	soc_reg64_field32_set(unit, reg, datap, f0, SOC_PBMP_WORD_GET(pbm, 0));
	soc_reg64_field32_set(unit, reg, datap, f1, SOC_PBMP_WORD_GET(pbm, 1));
    } else {
	soc_reg64_field32_set(unit, reg, datap, fa, SOC_PBMP_WORD_GET(pbm, 0));
    }
}

/*
 * Purpose:
 *        Set up PTABLE for for trunk ports in tpbmp
 * Parameters:
 *        action - 1 to set trunking, 0 - to remove trunking
 */

STATIC int
_draco_trunk_set_port_property(int unit, pbmp_t tpbmp, int tid,
			       int psc, int action)
{
    bcm_port_t 		port;
    uint32     		config;

    PBMP_ITER(tpbmp, port) {
        /* Update PTABLE to reflect port trunk ID */
        SOC_IF_ERROR_RETURN
            (bcm_esw_port_tgid_set(unit, port, action ? tid : BCM_TRUNK_INVALID, 0));

	if (SOC_IS_TUCANA(unit)) {
	    port_tab_entry_t	ptab;

	    SOC_IF_ERROR_RETURN
		(READ_PORT_TAB_MOD_CHK(unit, MEM_BLOCK_ANY, port, &ptab));
	    soc_mem_field32_set(unit, SOC_PORT_MEM_TAB(unit, port),
				&ptab, PORT_TYPEf, action ? 1 : 0);
	    SOC_IF_ERROR_RETURN
		(WRITE_PORT_TAB_MOD_CHK(unit, MEM_BLOCK_ALL, port, &ptab));
	} else {
	    /* Update CONFIG Register to reflect port trunk status */
	    SOC_IF_ERROR_RETURN(READ_CONFIGr(unit, port, &config));
	    soc_reg_field_set(unit, CONFIGr, &config, PORT_TYPEf,
			      action ? 1 : 0);
	    SOC_IF_ERROR_RETURN(WRITE_CONFIGr(unit, port, config));
	}
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *        _draco_trunk_remove_ports
 * Purpose:
 *        De-trunk the ports in del_pbmp in ptable,
 *        xDLF_TRUNK_BLOCK_MASKr, xMC_TRUNK_BLOCK_MASKr and
 *        xIPMC_TRUNK_BLOCK_MASKr.
 * NOTE:  
 *        TRUNK_BITMAPm and TRUNK_GROUPm are not configured here
 */

STATIC int
_draco_trunk_remove_ports(int unit, bcm_trunk_t tid, pbmp_t del_pbmp)
{
    uint64          		trunk_mask;
    pbmp_t			mask_pbmp;
    soc_port_t			port;

    /*
     * Set the ports to non-trunking in ptable.
     */
    _draco_trunk_set_port_property(unit, del_pbmp, 0, 0, FALSE);

    /* Set DLF trunk block mask register */
    SOC_IF_ERROR_RETURN
	(soc_reg64_read_any_block(unit, DLF_TRUNK_BLOCK_MASKr,
				  &trunk_mask));
    mask_pbmp = _xgs_getr_pbm(unit, trunk_mask, DLF_TRUNK_BLOCK_MASKr,
			      BLOCK_MASKf, BLOCK_MASK_0f, BLOCK_MASK_1f);
    SOC_PBMP_REMOVE(mask_pbmp, del_pbmp);
    _xgs_setr_pbm(unit, &trunk_mask, mask_pbmp, DLF_TRUNK_BLOCK_MASKr,
		  BLOCK_MASKf, BLOCK_MASK_0f, BLOCK_MASK_1f);
    SOC_IF_ERROR_RETURN
        (soc_reg64_write_all_blocks(unit, DLF_TRUNK_BLOCK_MASKr,
                                    trunk_mask));
    PBMP_HG_ITER(unit, port) {
	SOC_IF_ERROR_RETURN(WRITE_IDLF_TRUNK_BLOCK_MASKr(unit, port,
							 trunk_mask));
    }

    /* Set MC trunk block mask register */
    SOC_IF_ERROR_RETURN
	(soc_reg64_read_any_block(unit, MC_TRUNK_BLOCK_MASKr,
				  &trunk_mask));
    mask_pbmp = _xgs_getr_pbm(unit, trunk_mask, MC_TRUNK_BLOCK_MASKr,
			      BLOCK_MASKf, BLOCK_MASK_0f, BLOCK_MASK_1f);
    SOC_PBMP_REMOVE(mask_pbmp, del_pbmp);
    _xgs_setr_pbm(unit, &trunk_mask, mask_pbmp, MC_TRUNK_BLOCK_MASKr,
		  BLOCK_MASKf, BLOCK_MASK_0f, BLOCK_MASK_1f);
    SOC_IF_ERROR_RETURN
        (soc_reg64_write_all_blocks(unit, MC_TRUNK_BLOCK_MASKr,
                                    trunk_mask));
    PBMP_HG_ITER(unit, port) {
	SOC_IF_ERROR_RETURN(WRITE_IMC_TRUNK_BLOCK_MASKr(unit, port,
							trunk_mask));
    }

    /* Set IPMC trunk block mask register */
    SOC_IF_ERROR_RETURN
	(soc_reg64_read_any_block(unit, IPMC_TRUNK_BLOCK_MASKr,
				  &trunk_mask));
    mask_pbmp = _xgs_getr_pbm(unit, trunk_mask, IPMC_TRUNK_BLOCK_MASKr,
			      BLOCK_MASKf, BLOCK_MASK_0f, BLOCK_MASK_1f);
    SOC_PBMP_REMOVE(mask_pbmp, del_pbmp);
    _xgs_setr_pbm(unit, &trunk_mask, mask_pbmp, IPMC_TRUNK_BLOCK_MASKr,
		  BLOCK_MASKf, BLOCK_MASK_0f, BLOCK_MASK_1f);
    SOC_IF_ERROR_RETURN
	(soc_reg64_write_all_blocks(unit, IPMC_TRUNK_BLOCK_MASKr,
				    trunk_mask));
    PBMP_HG_ITER(unit, port) {
	SOC_IF_ERROR_RETURN(WRITE_IIPMC_TRUNK_BLOCK_MASKr(unit, port,
							  trunk_mask));
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *        _draco15_trunk_configure
 * Purpose:
 *        Helper routine for BCM5695 trunk configuration.
 *        EGR_MASKm must be configured for modid/port->tgid lookup.
 *        MMU_MEMORIES2_SRC_TRUNK_MAPm and
 *        MMU_MEMORIES2_EGR_TRUNK_MAPm tables must be configured
 *        for IPMC echo packet suppression.
 */

STATIC int
_draco15_trunk_configure(int unit, bcm_module_t portmod, bcm_port_t port,
			 int trunk_enable, bcm_trunk_t tid)
{
    egr_mask_entry_t egr_mask_entry;
    int rv = BCM_E_NONE;
    int mp_index = portmod*32 + port;
    int trunk = (trunk_enable != 0);

    /*
     * Setup EGR_MASKm to do modid/port -> tgid lookup
     * Also clear EGR_MASK.EGRESS_MASK if de-trunking
     */
    soc_mem_lock(unit, EGR_MASKm);
    rv = soc_mem_read(unit, EGR_MASKm, MEM_BLOCK_ALL, 
		      mp_index, &egr_mask_entry);
    if (BCM_SUCCESS(rv)) {
	if (!trunk &&
	    soc_EGR_MASKm_field32_get(unit, &egr_mask_entry, Tf)) {
	    /* 
	     * De-trunking a trunk port; clear its EGR_MASK.EGRESS_MASK
	     */
	    soc_EGR_MASKm_field32_set(unit, &egr_mask_entry, 
				      EGRESS_MASKf, 0);
	}
        soc_EGR_MASKm_field32_set(unit, &egr_mask_entry, 
				  Tf, 
				  trunk);
	soc_EGR_MASKm_field32_set(unit, &egr_mask_entry, 
				  TGIDf, 
				  (trunk ? tid : 0));
	rv = soc_mem_write(unit, EGR_MASKm, MEM_BLOCK_ALL, 
			   mp_index, &egr_mask_entry);
    }
    soc_mem_unlock(unit, EGR_MASKm);
    if (!BCM_SUCCESS(rv)) {
        return rv;
    }

    if (soc_feature(unit, soc_feature_ipmc_lookup)) {
        mmu_memories2_src_trunk_map_entry_t stm_entry;
        bcm_module_t my_modid;

        /* 
	 * Setup SRC_TRUNK_MAPm with modid/port -> tgid lookup information.
	 */
	sal_memset(&stm_entry, 0, sizeof(stm_entry));
	soc_MMU_MEMORIES2_SRC_TRUNK_MAPm_field32_set(unit, 
						     &stm_entry, 
						     SRC_Tf, 
						     trunk);
	soc_MMU_MEMORIES2_SRC_TRUNK_MAPm_field32_set(unit, 
						     &stm_entry, 
						     SRC_TGIDf, 
						     (trunk ? tid : 0));
	SOC_IF_ERROR_RETURN
	  (WRITE_MMU_MEMORIES2_SRC_TRUNK_MAPm(unit, MEM_BLOCK_ALL, 
					      mp_index, &stm_entry));

    BCM_IF_ERROR_RETURN(
        bcm_esw_stk_my_modid_get(unit, &my_modid));

	if (portmod == my_modid) {
	    /* 
	     * Local port, setup MMU_MEMORIES2_EGR_TRUNK_MAPm  with
	     * local_port -> tgid lookup info (IPMC echo suppression).
	     */
	    mmu_memories2_egr_trunk_map_entry_t etm_entry;
	    soc_mem_lock(unit, MMU_MEMORIES2_EGR_TRUNK_MAPm);
	    rv = READ_MMU_MEMORIES2_EGR_TRUNK_MAPm(unit, 
						   MEM_BLOCK_ALL, 
						   port, &etm_entry);
	    if (BCM_SUCCESS(rv)) {
	        soc_MMU_MEMORIES2_EGR_TRUNK_MAPm_field32_set(unit, 
							     &etm_entry, 
							     TGIDf, 
							     (trunk ? tid : 0));
		soc_MMU_MEMORIES2_EGR_TRUNK_MAPm_field32_set(unit, 
							     &etm_entry, 
							     TRUNKf, 
							     trunk);
		rv = WRITE_MMU_MEMORIES2_EGR_TRUNK_MAPm(unit, 
							MEM_BLOCK_ALL, 
							port,
							&etm_entry);
	    }
	    soc_mem_unlock(unit, MMU_MEMORIES2_EGR_TRUNK_MAPm);
	} /* Local port */ 
    } /* soc_feature_ipmc_lookup */
    return rv;
}


/* fields from TRUNK_GROUPm */
static soc_field_t	_xgs_portf[8] = {
    PORT0f, PORT1f, PORT2f, PORT3f,
    PORT4f, PORT5f, PORT6f, PORT7f
};

static soc_field_t	_xgs_modulef[8] = {
    MODULE0f, MODULE1f, MODULE2f, MODULE3f,
    MODULE4f, MODULE5f, MODULE6f, MODULE7f
};

/*
 * Function:
 *	bcm_draco_trunk_set
 * Purpose:
 *	Multiplexed trunking function for Draco
 */
int
bcm_draco_trunk_set(int unit, bcm_trunk_t tid, bcm_trunk_add_info_t *add_info,
                     trunk_private_t *t_info)
{
    trunk_bitmap_entry_t 	trunk_bitmap_entry;
    trunk_group_entry_t 	tg_entry;
    pbmp_t     			old_trunk_pbmp;
    pbmp_t     			new_trunk_pbmp;
    pbmp_t     			diff_pbmp;
    pbmp_t			nonge_pbmp;
    pbmp_t			mask_pbmp;
    bcm_port_t 			port;
    int				dlf_index, mc_index, ipmc_index;
    bcm_module_t 		portmod, my_modid, my_modid_hi = -1;
    int        			i, size;
    uint64     			trunk_mask;
    uint32                      nuc_tpbm = 0;

    BCM_IF_ERROR_RETURN(
        bcm_esw_stk_my_modid_get(unit, &my_modid));
    if (SOC_IS_TUCANA(unit)) {
        my_modid_hi = my_modid + 1;
    }

    /*
     * if special ports were previously chosen by code,
     * make them dynamic this time as well
     */
    if (t_info->in_use) {
	if (t_info->dlf_index_spec < 0 &&
	    add_info->dlf_index == t_info->dlf_index_used) {
	    add_info->dlf_index = -1;
	}
	if (t_info->mc_index_spec < 0 &&
	    add_info->mc_index == t_info->mc_index_used) {
	    add_info->mc_index = -1;
	}
	if (t_info->ipmc_index_spec < 0 &&
	    add_info->ipmc_index == t_info->ipmc_index_used) {
	    add_info->ipmc_index = -1;
	}
    }

    if (add_info->dlf_index >= add_info->num_ports) {
	return BCM_E_PARAM;
    }
    if (add_info->mc_index >= add_info->num_ports) {
	return BCM_E_PARAM;
    }
    if (add_info->ipmc_index >= add_info->num_ports) {
	return BCM_E_PARAM;
    }

    if (add_info->psc <= 0) {
	add_info->psc = BCM_TRUNK_PSC_DEFAULT;
    }
    switch (add_info->psc) {
    case BCM_TRUNK_PSC_SRCMAC:
    case BCM_TRUNK_PSC_DSTMAC:
    case BCM_TRUNK_PSC_SRCDSTMAC:
    case BCM_TRUNK_PSC_SRCIP:
    case BCM_TRUNK_PSC_DSTIP:
    case BCM_TRUNK_PSC_SRCDSTIP:
	break;
    default:
	return BCM_E_PARAM;
    }

    /*
     * Read the old TRUNK_GROUP entry
     */
    if (t_info->in_use) {
        BCM_IF_ERROR_RETURN
            (soc_mem_read(unit, TRUNK_GROUPm, MEM_BLOCK_ANY, tid, &tg_entry));
	size = soc_TRUNK_GROUPm_field32_get(unit, &tg_entry, TG_SIZEf);
    } else {
        size = -1;
    }

    /*
     * BUGwatch : Make sure that the ports are for this particular module
     * IN PARTICULAR, for Tucana, may need to check port % 32.
     */

    /*
     * TG_SIZEf runs 7..0 for trunk sizes 8..1
     */
    SOC_PBMP_CLEAR(old_trunk_pbmp);
    for (; size >= 0; size--) {
        port = soc_TRUNK_GROUPm_field32_get(unit, &tg_entry,
					    _xgs_portf[size]);
        portmod = soc_TRUNK_GROUPm_field32_get(unit, &tg_entry,
					       _xgs_modulef[size]);
        if (portmod == my_modid) {
	    SOC_PBMP_PORT_ADD(old_trunk_pbmp, port);
        } else if (portmod == my_modid_hi) {
	    SOC_PBMP_PORT_ADD(old_trunk_pbmp, SOC_PORT_IN_MOD1(unit, port));
        }

	if (SOC_IS_DRACO15(unit)) {
	    /* 
	     * De-trunk this module/port from DRACO15-specific trunk tables 
	     */
	    BCM_IF_ERROR_RETURN
		(_draco15_trunk_configure(unit, portmod, port, 0, 0));
	} 
    }

    /* New trunk group size */
    size = add_info->num_ports - 1;

    /*
     * Write new TTR and TBMAP entry
     */

    /* Will overwrite existing TRUNK_BITMAP_ENTRYm[tid], TRUNK_GROUPm[tid] */
    SOC_PBMP_CLEAR(new_trunk_pbmp);
    sal_memset(&tg_entry, 0, sizeof(tg_entry));
    for (i = 0; i <= size; i++) {
	bcm_module_t	mod_in, mod_out;
	bcm_port_t	port_in, port_out;

	mod_in = add_info->tm[i];
	port_in = add_info->tp[i];
	BCM_IF_ERROR_RETURN
	    (_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_SET,
                                    mod_in, port_in,
                                    &mod_out, &port_out));

    if (!SOC_MODID_ADDRESSABLE(unit, mod_out)) {
        return BCM_E_BADID;
    }
    if (!SOC_PORT_ADDRESSABLE(unit, port_out)) {
        return BCM_E_PORT;
    }

    if (my_modid == mod_out) {
        if (SOC_PORT_MOD1(unit, port_out)) {
            mod_out = my_modid_hi;
        }
        SOC_PBMP_PORT_ADD(new_trunk_pbmp, port_out);
    } else if (my_modid_hi == mod_out) {
        /* Force to hi port number */
        port_out = SOC_PORT_IN_MOD1(unit, port_out);
        SOC_PBMP_PORT_ADD(new_trunk_pbmp, port_out);
    }

	soc_TRUNK_GROUPm_field32_set(unit, &tg_entry,
				     _xgs_portf[i], port_out & 0x1f);
	soc_TRUNK_GROUPm_field32_set(unit, &tg_entry,
				     _xgs_modulef[i], mod_out);
	if (SOC_IS_DRACO15(unit)) {
	    /* 
	     * Add this module/port to DRACO15-specific trunk tables 
	     */
	    BCM_IF_ERROR_RETURN
		(_draco15_trunk_configure(unit, mod_out, (port_out & 0x1f), 
					  1, tid));
	} 
    }

    soc_TRUNK_GROUPm_field32_set(unit, &tg_entry, RTAGf, add_info->psc);
    soc_TRUNK_GROUPm_field32_set(unit, &tg_entry, TG_SIZEf, size);

    /*
     * The stack ports should not be in TBMAP entry, and should be
     * kept in vtable or ptable pbmap entries.
     */
    SOC_PBMP_REMOVE(new_trunk_pbmp, SOC_PBMP_STACK_CURRENT(unit));
    SOC_PBMP_REMOVE(old_trunk_pbmp, SOC_PBMP_STACK_CURRENT(unit));

    /*
     * make sure all ports are all GE(except for stack ports)
     */
    SOC_PBMP_ASSIGN(nonge_pbmp, PBMP_HG_ALL(unit));
    SOC_PBMP_OR(nonge_pbmp, PBMP_CMIC(unit));
    SOC_PBMP_AND(nonge_pbmp, new_trunk_pbmp);
    if (SOC_PBMP_NOT_NULL(nonge_pbmp)) {
        return (BCM_E_CONFIG);
    }

    SOC_PBMP_ASSIGN(diff_pbmp, new_trunk_pbmp);
    SOC_PBMP_OR(diff_pbmp, old_trunk_pbmp);
    SOC_PBMP_XOR(diff_pbmp, new_trunk_pbmp);
    if (SOC_PBMP_NOT_NULL(diff_pbmp)) {
        BCM_IF_ERROR_RETURN
            (_draco_trunk_remove_ports(unit, tid, diff_pbmp));
    }

    /*
     * Update TRUNK_BITMAP table
     */
    SOC_IF_ERROR_RETURN
        (soc_mem_read(unit, TRUNK_BITMAPm, MEM_BLOCK_ANY, tid,
		      &trunk_bitmap_entry));
    if (SOC_IS_TUCANA(unit)) {
	soc_TRUNK_BITMAPm_field32_set(unit, &trunk_bitmap_entry,
				      TRUNK_BITMAP_M0f,
				      SOC_PBMP_WORD_GET(new_trunk_pbmp, 0));
	soc_TRUNK_BITMAPm_field32_set(unit, &trunk_bitmap_entry,
				      TRUNK_BITMAP_M1f,
				      SOC_PBMP_WORD_GET(new_trunk_pbmp, 1));
    } else {
	soc_TRUNK_BITMAPm_field32_set(unit, &trunk_bitmap_entry,
				      TRUNK_BITMAPf,
				      SOC_PBMP_WORD_GET(new_trunk_pbmp, 0));
    }
    soc_TRUNK_BITMAPm_field32_set(unit, &trunk_bitmap_entry,
                                  RTAGf, add_info->psc);
    SOC_IF_ERROR_RETURN
        (soc_mem_write(unit, TRUNK_BITMAPm, MEM_BLOCK_ALL, tid,
		       &trunk_bitmap_entry));

    /*
     * Update TRUNK_GROUP table
     */
    SOC_IF_ERROR_RETURN
        (soc_mem_write(unit, TRUNK_GROUPm, MEM_BLOCK_ALL, tid, &tg_entry));

    if (SOC_PBMP_NOT_NULL(new_trunk_pbmp)) {
	bcm_module_t	mod_in, mod_out;
	bcm_port_t	port_in, port_out;

        t_info->dlf_index_spec = add_info->dlf_index;
        t_info->mc_index_spec = add_info->mc_index;
        t_info->ipmc_index_spec = add_info->ipmc_index;

        /* determine the dlf/broadcast port */
	dlf_index = t_info->dlf_index_spec;
	if (dlf_index < 0) {
	    dlf_index = 0;
            if ((add_info->dlf_index != add_info->mc_index) ||
                (add_info->dlf_index != add_info->ipmc_index) ||
                (add_info->mc_index != add_info->ipmc_index)) {
                return BCM_E_PARAM;
            }
            BCM_IF_ERROR_RETURN(_bcm_nuc_tpbm_get(unit,
                                                  add_info->num_ports,
                                                  add_info->tm,
                                                  &nuc_tpbm));
	} else {
	    nuc_tpbm = (1 << dlf_index);
	}
	t_info->dlf_index_used = dlf_index;
	mod_in = add_info->tm[dlf_index];
	port_in = add_info->tp[dlf_index];
	BCM_IF_ERROR_RETURN
	    (_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_SET,
                                    mod_in, port_in,
                                    &mod_out, &port_out));
        if (mod_out == my_modid || mod_out == my_modid_hi) {
            if (mod_out == my_modid_hi) {
                port_out = SOC_PORT_IN_MOD1(unit, port_out);
            }
	    t_info->dlf_port_used = port_out;
	} else {
	    t_info->dlf_port_used = -1;
	}

	mc_index = t_info->mc_index_spec;
	if (mc_index < 0) {
	    mc_index = 0;
	}
	t_info->mc_index_used = mc_index;
	if (mc_index != dlf_index) {
	    mod_in = add_info->tm[mc_index];
	    port_in = add_info->tp[mc_index];
	    BCM_IF_ERROR_RETURN
		(_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_SET,
                                        mod_in, port_in,
                                        &mod_out, &port_out));
	}
        if (mod_out == my_modid || mod_out == my_modid_hi) {
            if (mod_out == my_modid_hi) {
                port_out = SOC_PORT_IN_MOD1(unit, port_out);
            }
	    t_info->mc_port_used = port_out;
	} else {
	    t_info->mc_port_used = -1;
	}

	ipmc_index = t_info->ipmc_index_spec;
	if (ipmc_index < 0) {
	    ipmc_index = 0;
	}
	t_info->ipmc_index_used = ipmc_index;
	if (ipmc_index != mc_index) {
	    mod_in = add_info->tm[ipmc_index];
	    port_in = add_info->tp[ipmc_index];
	    BCM_IF_ERROR_RETURN
		(_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_SET,
                                        mod_in, port_in,
                                        &mod_out, &port_out));
	}
        if (mod_out == my_modid || mod_out == my_modid_hi) {
            if (mod_out == my_modid_hi) {
                port_out = SOC_PORT_IN_MOD1(unit, port_out);
            }
	    t_info->ipmc_port_used = port_out;
	} else {
	    t_info->ipmc_port_used = -1;
	}

        if (!((nuc_tpbm & (1 << t_info->dlf_index_used)) &&
            (nuc_tpbm & (1 << t_info->ipmc_index_used)) &&
            (nuc_tpbm & (1 << t_info->mc_index_used)))) {
            /*
             *  No trunk ports Eligible for Unblocking for
             *  DLF/MC/IPMC packet forwarding.
             */
            t_info->dlf_port_used = -1;
            t_info->ipmc_port_used = -1;
            t_info->mc_port_used = -1;
        }

        /*
         * Now set the DLF, MC and IPMC trunk block mask registers
         */
        SOC_IF_ERROR_RETURN
            (soc_reg64_read_any_block(unit, DLF_TRUNK_BLOCK_MASKr,
				      &trunk_mask));
	mask_pbmp = _xgs_getr_pbm(unit, trunk_mask, DLF_TRUNK_BLOCK_MASKr,
				  BLOCK_MASKf, BLOCK_MASK_0f, BLOCK_MASK_1f);
	SOC_PBMP_OR(mask_pbmp, new_trunk_pbmp);
	if (t_info->dlf_port_used >= 0) {
	    SOC_PBMP_PORT_REMOVE(mask_pbmp, t_info->dlf_port_used);
	}
	_xgs_setr_pbm(unit, &trunk_mask, mask_pbmp, DLF_TRUNK_BLOCK_MASKr,
		      BLOCK_MASKf, BLOCK_MASK_0f, BLOCK_MASK_1f);
        SOC_IF_ERROR_RETURN
            (soc_reg64_write_all_blocks(unit, DLF_TRUNK_BLOCK_MASKr,
                                        trunk_mask));
	PBMP_HG_ITER(unit, port) {
	    SOC_IF_ERROR_RETURN(WRITE_IDLF_TRUNK_BLOCK_MASKr(unit, port,
							     trunk_mask));
        }

        SOC_IF_ERROR_RETURN
	    (soc_reg64_read_any_block(unit, MC_TRUNK_BLOCK_MASKr,
				      &trunk_mask));
	mask_pbmp = _xgs_getr_pbm(unit, trunk_mask, MC_TRUNK_BLOCK_MASKr,
				  BLOCK_MASKf, BLOCK_MASK_0f, BLOCK_MASK_1f);
	SOC_PBMP_OR(mask_pbmp, new_trunk_pbmp);
	if (t_info->mc_port_used >= 0) {
	    SOC_PBMP_PORT_REMOVE(mask_pbmp, t_info->mc_port_used);
	}
	_xgs_setr_pbm(unit, &trunk_mask, mask_pbmp, MC_TRUNK_BLOCK_MASKr,
		      BLOCK_MASKf, BLOCK_MASK_0f, BLOCK_MASK_1f);
        SOC_IF_ERROR_RETURN
	    (soc_reg64_write_all_blocks(unit, MC_TRUNK_BLOCK_MASKr,
                                        trunk_mask));
	PBMP_HG_ITER(unit, port) {
	    SOC_IF_ERROR_RETURN(WRITE_IMC_TRUNK_BLOCK_MASKr(unit, port,
							    trunk_mask));
        }

        SOC_IF_ERROR_RETURN
	    (soc_reg64_read_any_block(unit, IPMC_TRUNK_BLOCK_MASKr,
				      &trunk_mask));
	mask_pbmp = _xgs_getr_pbm(unit, trunk_mask, IPMC_TRUNK_BLOCK_MASKr,
				  BLOCK_MASKf, BLOCK_MASK_0f, BLOCK_MASK_1f);
	SOC_PBMP_OR(mask_pbmp, new_trunk_pbmp);
	if (t_info->ipmc_port_used >= 0) {
	    SOC_PBMP_PORT_REMOVE(mask_pbmp, t_info->ipmc_port_used);
	}
	_xgs_setr_pbm(unit, &trunk_mask, mask_pbmp, IPMC_TRUNK_BLOCK_MASKr,
		      BLOCK_MASKf, BLOCK_MASK_0f, BLOCK_MASK_1f);
        SOC_IF_ERROR_RETURN
	    (soc_reg64_write_all_blocks(unit, IPMC_TRUNK_BLOCK_MASKr,
                                        trunk_mask));
	PBMP_HG_ITER(unit, port) {
	    SOC_IF_ERROR_RETURN(WRITE_IIPMC_TRUNK_BLOCK_MASKr(unit, port,
							      trunk_mask));
        }

        _draco_trunk_set_port_property(unit, new_trunk_pbmp,
				       tid, add_info->psc, TRUE);
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *	bcm_draco_trunk_destroy
 * Purpose:
 *	Multiplexed trunking function for Draco
 */
int
bcm_draco_trunk_destroy(int unit, bcm_trunk_t tid, trunk_private_t *t_info)
{
    trunk_group_entry_t 	tg_entry;
    bcm_port_t 			port;
    pbmp_t 			tr_pbmp;
    bcm_module_t 		portmod, my_modid, my_modid_hi = -1;
    int 			size;

    BCM_IF_ERROR_RETURN(
        bcm_esw_stk_my_modid_get(unit, &my_modid));
    if (SOC_IS_TUCANA(unit)) {
	my_modid_hi = my_modid + 1;
    }

    SOC_IF_ERROR_RETURN
	(soc_mem_read(unit, TRUNK_GROUPm, MEM_BLOCK_ANY, tid, &tg_entry));

    size = soc_TRUNK_GROUPm_field32_get(unit, &tg_entry, TG_SIZEf);
    SOC_PBMP_CLEAR(tr_pbmp);

    for (; size >= 0; size--) {
        port = soc_TRUNK_GROUPm_field32_get(unit, &tg_entry,
					    _xgs_portf[size]);
        portmod = soc_TRUNK_GROUPm_field32_get(unit, &tg_entry,
					       _xgs_modulef[size]);
        if (portmod == my_modid) {
            SOC_PBMP_PORT_ADD(tr_pbmp, port);
        } else if (portmod == my_modid_hi) {
            SOC_PBMP_PORT_ADD(tr_pbmp, SOC_PORT_IN_MOD1(unit, port));
        }

	if (SOC_IS_DRACO15(unit)) {
	    /* 
	     * De-trunk this module/port from DRACO15-specific trunk tables 
	     */
	    BCM_IF_ERROR_RETURN
	      (_draco15_trunk_configure(unit, portmod, port, 0, 0));
	}
    }

    BCM_IF_ERROR_RETURN(_draco_trunk_remove_ports(unit, tid, tr_pbmp));

    /*
     * Reset TRUNK_EGR_MASK table entry
     */
    SOC_IF_ERROR_RETURN
        (soc_mem_write(unit, TRUNK_EGR_MASKm, MEM_BLOCK_ALL, tid,
		       soc_mem_entry_null(unit, TRUNK_EGR_MASKm)));

    /*
     * Reset TRUNK_BITMAP table entry
     */

    SOC_IF_ERROR_RETURN
        (soc_mem_write(unit, TRUNK_BITMAPm, MEM_BLOCK_ALL, tid,
		       soc_mem_entry_null(unit, TRUNK_BITMAPm)));

    /*
     * Reset TRUNK_GROUP table entry
     */
    SOC_IF_ERROR_RETURN
	(soc_mem_write(unit, TRUNK_GROUPm, MEM_BLOCK_ALL, tid,
		       soc_mem_entry_null(unit, TRUNK_GROUPm)));

    t_info->in_use = FALSE;

    return (BCM_E_NONE);
}

/*
 * Function:
 *	bcm_draco_trunk_get
 * Purpose:
 *	Multiplexed trunking function for Draco
 */
int
bcm_draco_trunk_get(int unit, bcm_trunk_t tid, bcm_trunk_add_info_t *t_data,
		    trunk_private_t *t_info)
{
    trunk_group_entry_t 	tg_entry;
    bcm_module_t 		modid_hi;
    int        			size;

    if (SOC_IS_TUCANA(unit)) {
        BCM_IF_ERROR_RETURN(
            bcm_esw_stk_my_modid_get(unit, &modid_hi));
        modid_hi += 1;
    } else {
	modid_hi = -1;
    }

    if (!t_info->in_use) {
	t_data->num_ports = 0;
    } else {
        SOC_IF_ERROR_RETURN
	    (soc_mem_read(unit, TRUNK_GROUPm, MEM_BLOCK_ANY, tid, &tg_entry));

        size = soc_TRUNK_GROUPm_field32_get(unit, &tg_entry, TG_SIZEf);
        t_data->num_ports = size + 1;

	for (; size >= 0; size--) {
	    bcm_module_t	mod_in, mod_out;
	    bcm_port_t		port_in, port_out;

            mod_in = soc_TRUNK_GROUPm_field32_get(unit, &tg_entry,
						  _xgs_modulef[size]);
            port_in = soc_TRUNK_GROUPm_field32_get(unit, &tg_entry,
						   _xgs_portf[size]);
            if (modid_hi == mod_in) {
		port_in = SOC_PORT_IN_MOD1(unit, port_in);
                /* Force to hi mod port numbering */
            }
	    BCM_IF_ERROR_RETURN
		(_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_GET,
                                        mod_in, port_in,
                                        &mod_out, &port_out));
            t_data->tm[size] = mod_out;
            t_data->tp[size] = port_out;
	}
    }

    t_data->psc = t_info->psc;
    t_data->dlf_index = t_info->dlf_index_used;
    t_data->mc_index = t_info->mc_index_used;
    t_data->ipmc_index = t_info->ipmc_index_used;

    return (BCM_E_NONE);
}

/*
 * Function:
 *	bcm_draco_trunk_mcast_join
 * Purpose:
 *	Multiplexed function for Draco
 */

int
bcm_draco_trunk_mcast_join(int unit, bcm_trunk_t tid, bcm_vlan_t vid,
			   sal_mac_addr_t mac, trunk_private_t *t_info)
{
    trunk_bitmap_entry_t	trunk_bitmap_entry;
    bcm_mcast_addr_t  		mc_addr;
    pbmp_t  			pbmp, m_pbmp;

    /*
     * No need for all this in Draco. You can just set the MC trunk block
     * mask register ???
     */

    /* get the ports in the trunk group */
    SOC_IF_ERROR_RETURN
        (soc_mem_read(unit, TRUNK_BITMAPm, MEM_BLOCK_ANY, tid,
		      &trunk_bitmap_entry));
    SOC_PBMP_CLEAR(pbmp);
    if (SOC_IS_TUCANA(unit)) {
	SOC_PBMP_WORD_SET(pbmp, 0,
			  soc_TRUNK_BITMAPm_field32_get(unit,
							&trunk_bitmap_entry,
							TRUNK_BITMAP_M0f));
	SOC_PBMP_WORD_SET(pbmp, 1,
			  soc_TRUNK_BITMAPm_field32_get(unit,
							&trunk_bitmap_entry,
							TRUNK_BITMAP_M1f));
    } else {
	SOC_PBMP_WORD_SET(pbmp, 0,
			  soc_TRUNK_BITMAPm_field32_get(unit,
							&trunk_bitmap_entry,
							TRUNK_BITMAPf));
    }

    BCM_IF_ERROR_RETURN(
        bcm_esw_mcast_port_get(unit, mac, vid, &mc_addr));
    /* Coverity: mc_addr.pbmp is initialized above by bcm_esw_mcast_port_get. */
    /* coverity[uninit_use : FALSE] */
    SOC_PBMP_ASSIGN(m_pbmp, mc_addr.pbmp);
    SOC_PBMP_XOR(m_pbmp, pbmp);
    SOC_PBMP_AND(m_pbmp, pbmp);
    if (SOC_PBMP_IS_NULL(m_pbmp)) {
        return (BCM_E_CONFIG);
    }

    BCM_IF_ERROR_RETURN(
        bcm_esw_mcast_addr_remove(unit, mac, vid));
    SOC_PBMP_REMOVE(mc_addr.pbmp, pbmp);
    if (t_info->mc_port_used >= 0) {
        SOC_PBMP_PORT_ADD(mc_addr.pbmp, t_info->mc_port_used);
    }

    return bcm_esw_mcast_addr_add(unit, &mc_addr);
}
