/*
 * $Id: port.c 1.29 Broadcom SDK $
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
 * File:        port.c
 * Purpose:     XGS family port function implementations
 */

#include <soc/defs.h>

#ifdef BCM_XGS_SWITCH_SUPPORT

#include <assert.h>

#include <sal/core/libc.h>

#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/cm.h>
#include <soc/drv.h>
#include <soc/register.h>
#include <soc/memory.h>
#include <soc/ll.h>
#include <soc/ptable.h>
#include <soc/tucana.h>
#include <soc/l2x.h>

#include <bcm/port.h>

#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/draco.h>

#include <bcm_int/esw_dispatch.h>

/* Draco needs Strata2 metering routines. Include them */
/* if the build is for Draco-but-no-Tucana. */

#ifdef	BCM_DRACO_SUPPORT
#if (!defined(BCM_TUCANA_SUPPORT))
#include "../strata/port.c"
#endif	/* BCM_TUCANA_SUPPORT */
#endif	/* BCM_DRACO_SUPPORT */

/*
 * Function:
 *	_draco_port_tab_to_port_cfg
 * Purpose:
 *	Translate machine-dependent port table information to common
 *	structure.
 * Parameters:
 *	unit - unit #
 *	port - port number
 *	pv - Draco hardware-specific PORT_TAB entry
 *	cfg - (OUT) Hardware-independent port info structure
 * Notes:
 *	The following port configuration settings
 *	do not apply to Draco Gig ports:
 *
 *	cfg->pc_pbm
 *	cfg->pc_ut_pbm
 *	cfg->pc_stp_state
 *	cfg->pc_frame_type
 *	cfg->pc_ether_type
 *	cfg->pc_cpu
 *	cfg->pc_trunk
 *	cfg->pc_ptype
 *	cfg->pc_jumbo
 */

STATIC void
_draco_port_tab_to_port_cfg(int unit, bcm_port_t port,
			    port_tab_entry_t *pv,
                            bcm_port_cfg_t *cfg)
{
    if (!SOC_IS_TUCANA(unit) && !SOC_IS_LYNX(unit)) {
        cfg->pc_new_opri = soc_PORT_TABm_field32_get(unit, pv, NEW_PRIf);
    }
    if (SOC_IS_DRACO15(unit)) {
	int dis_tag, dis_untag;

        dis_tag = soc_PORT_TABm_field32_get(unit, pv, PORT_DIS_TAGf);
        dis_untag = soc_PORT_TABm_field32_get(unit, pv, PORT_DIS_UNTAGf);
	if (dis_tag && dis_untag) {
	    cfg->pc_disc = BCM_PORT_DISCARD_ALL;
	} else if (dis_tag) {
	    cfg->pc_disc = BCM_PORT_DISCARD_TAG;
	} else if (dis_untag) {
	    cfg->pc_disc = BCM_PORT_DISCARD_UNTAG;
	} else {
	    cfg->pc_disc = BCM_PORT_DISCARD_NONE;
        }
        cfg->pc_dse_mode = soc_PORT_TABm_field32_get(unit, pv, TRUST_DSCPf);
        cfg->pc_dscp_prio = cfg->pc_dse_mode;
	cfg->pc_dscp = -1;
    } else {	/* DRACO1, LYNX, TUCANA */
        if (soc_PORT_TABm_field32_get(unit, pv, PORT_DIS_ALLf)) {
            cfg->pc_disc = BCM_PORT_DISCARD_ALL;
        } else if (soc_PORT_TABm_field32_get(unit, pv, PORT_DIS_UNTAGf)) {
            cfg->pc_disc = BCM_PORT_DISCARD_UNTAG;
        } else {
            cfg->pc_disc = BCM_PORT_DISCARD_NONE;
        }
        cfg->pc_dse_mode = soc_PORT_TABm_field32_get(unit, pv, DSE_MODEf);
        cfg->pc_dscp = soc_PORT_TABm_field32_get(unit, pv, DSCPf);
	if (SOC_IS_TUCANA(unit)) {
	    /* Check TRUST_DSCP bit (controls DSCP-to-802.1p mapping) */
	    cfg->pc_dscp_prio = soc_PORT_TABm_field32_get(unit, pv,
							  TRUST_DSCPf);
	}
    }
    cfg->pc_tgid = soc_PORT_TABm_field32_get(unit, pv, TGIDf);
    cfg->pc_bpdu_disable = soc_PORT_TABm_field32_get(unit, pv, BPDUf);
    if (soc_PORT_TABm_field32_get(unit, pv, L3_ENABLEf))  {
        cfg->pc_l3_flags |= BCM_PORT_L3_V4_ENABLE; 
    } 
    cfg->pc_remap_pri_en = soc_PORT_TABm_field32_get(unit, pv, RPEf);
    if (soc_l2x_frozen_cml_get(unit, port, &cfg->pc_cml, &cfg->pc_cml_move) < 0) {
	cfg->pc_cml = soc_PORT_TABm_field32_get(unit, pv, CMLf);
    }
    cfg->pc_mirror_ing = soc_PORT_TABm_field32_get(unit, pv, MIRRORf);
    cfg->pc_en_ifilter = soc_PORT_TABm_field32_get(unit, pv, EN_IFILTERf);
    cfg->pc_pfm = soc_PORT_TABm_field32_get(unit, pv, PFMf);
}

/*
 * Function:
 *	_draco_port_cfg_to_port_tab
 * Purpose:
 *	Translate common structure to machine-dependent port table
 *	information.
 * Parameters:
 *	unit - unit #
 *	port - port number
 *	pv - (OUT) Draco hardware-specific PORT_TAB entry
 *	cfg - Hardware-independent port info structure
 */

STATIC void
_draco_port_cfg_to_port_tab(int unit, bcm_port_t port,
			    port_tab_entry_t *pv,
			    bcm_port_cfg_t *cfg)
{
    int	repl_cml;

    if (!SOC_IS_TUCANA(unit) && !SOC_IS_LYNX(unit)) {
        soc_PORT_TABm_field32_set(unit, pv, NEW_PRIf, cfg->pc_new_opri);
    }

    if (SOC_IS_DRACO15(unit)) {
	switch (cfg->pc_disc) {
	case BCM_PORT_DISCARD_NONE:
            soc_PORT_TABm_field32_set(unit, pv, PORT_DIS_TAGf, 0);
            soc_PORT_TABm_field32_set(unit, pv, PORT_DIS_UNTAGf, 0);
	    break;
	case BCM_PORT_DISCARD_ALL:
            soc_PORT_TABm_field32_set(unit, pv, PORT_DIS_TAGf, 1);
            soc_PORT_TABm_field32_set(unit, pv, PORT_DIS_UNTAGf, 1);
	    break;
	case BCM_PORT_DISCARD_UNTAG:
            soc_PORT_TABm_field32_set(unit, pv, PORT_DIS_TAGf, 0);
            soc_PORT_TABm_field32_set(unit, pv, PORT_DIS_UNTAGf, 1);
	    break;
	case BCM_PORT_DISCARD_TAG:
            soc_PORT_TABm_field32_set(unit, pv, PORT_DIS_TAGf, 1);
            soc_PORT_TABm_field32_set(unit, pv, PORT_DIS_UNTAGf, 0);
	    break;
	}
	soc_PORT_TABm_field32_set(unit, pv, TRUST_DSCPf,
				  cfg->pc_dse_mode ? 1 : 0);
    } else {	/* DRACO1, LYNX, TUCANA */
	switch (cfg->pc_disc) {
	case BCM_PORT_DISCARD_NONE:
	case BCM_PORT_DISCARD_TAG:	/* cannot do this */
            soc_PORT_TABm_field32_set(unit, pv, PORT_DIS_UNTAGf, 0);
            soc_PORT_TABm_field32_set(unit, pv, PORT_DIS_ALLf, 0);
	    break;
	case BCM_PORT_DISCARD_ALL:
            soc_PORT_TABm_field32_set(unit, pv, PORT_DIS_UNTAGf, 1);
            soc_PORT_TABm_field32_set(unit, pv, PORT_DIS_ALLf, 1);
	    break;
	case BCM_PORT_DISCARD_UNTAG:
            soc_PORT_TABm_field32_set(unit, pv, PORT_DIS_UNTAGf, 1);
            soc_PORT_TABm_field32_set(unit, pv, PORT_DIS_ALLf, 0);
	    break;
	}
        soc_PORT_TABm_field32_set(unit, pv, DSE_MODEf, cfg->pc_dse_mode);
	if (SOC_IS_TUCANA(unit)) {
	    /* DSCP-to-802.1p mapping */
	    soc_PORT_TABm_field32_set(unit, pv, TRUST_DSCPf,
				      cfg->pc_dscp_prio ? 1 : 0);
	}
        soc_PORT_TABm_field32_set(unit, pv, DSCPf, cfg->pc_dscp);
    }

    soc_PORT_TABm_field32_set(unit, pv, TGIDf, cfg->pc_tgid);
    soc_PORT_TABm_field32_set(unit, pv, BPDUf, cfg->pc_bpdu_disable);
    soc_PORT_TABm_field32_set(unit, pv, L3_ENABLEf, 
                         (cfg->pc_l3_flags & BCM_PORT_L3_V4_ENABLE) ? 1 : 0);
    soc_PORT_TABm_field32_set(unit, pv, RPEf, cfg->pc_remap_pri_en);
    if (soc_l2x_frozen_cml_set(unit, port, cfg->pc_cml, cfg->pc_cml_move,  
                               &repl_cml, NULL) < 0) {
	soc_PORT_TABm_field32_set(unit, pv, CMLf, cfg->pc_cml);
    } else {
	soc_PORT_TABm_field32_set(unit, pv, CMLf, repl_cml);
    }
    soc_PORT_TABm_field32_set(unit, pv, MIRRORf, cfg->pc_mirror_ing);
    soc_PORT_TABm_field32_set(unit, pv, EN_IFILTERf, cfg->pc_en_ifilter);
    soc_PORT_TABm_field32_set(unit, pv, PFMf, cfg->pc_pfm);
}

int
bcm_draco_port_cfg_get(int unit, bcm_port_t port, bcm_port_cfg_t *cfg)
{
    uint64		prreg;
    uint32		vlreg;
    port_tab_entry_t	ptab;
    soc_mem_t		mem;
    uint64		iconfig;
    int         rv;

    sal_memset(cfg, 0, sizeof(*cfg));

    /* higig port */
    if (IS_HG_PORT(unit, port)) {
	BCM_IF_ERROR_RETURN(READ_ICONFIGr(unit, port, &iconfig));

	cfg->pc_cml = soc_reg64_field32_get(unit, ICONFIGr, iconfig, CMLf);
	if (soc_reg64_field32_get(unit, ICONFIGr, iconfig, PORT_DIS_ALLf)) {
	    cfg->pc_disc = BCM_PORT_DISCARD_ALL;
	} else {
	    cfg->pc_disc = BCM_PORT_DISCARD_NONE;
	}
	return BCM_E_NONE;
    }

    /* ether ports */
    if (IS_E_PORT(unit, port)) {
        mem = SOC_PORT_MEM_TAB(unit, port);

        /* Get the default VLAN (and/or priority) for this port */
        if (SOC_IS_DRACO15(unit)) {
            BCM_IF_ERROR_RETURN(READ_VLAN_DEFAULTr(unit, port, &vlreg));

            cfg->pc_vlan = soc_reg_field_get(unit, VLAN_DEFAULTr,
                                             vlreg, VLAN_IDf);
        } else {
            BCM_IF_ERROR_RETURN(READ_PRTABLE_DEFAULTr(unit, port, &prreg));

            cfg->pc_vlan = soc_reg64_field32_get(unit, PRTABLE_DEFAULTr,
                                                 prreg, VLAN_IDf);
#if defined(BCM_TUCANA_SUPPORT) || defined(BCM_LYNX_SUPPORT)
            if (SOC_IS_TUCANA(unit) || SOC_IS_LYNX(unit)) {
                cfg->pc_new_opri = soc_reg64_field32_get(unit, PRTABLE_DEFAULTr,
                                                         prreg, PRIf);
            }
#endif
        }

        /* Get the port table entry and convert */
        soc_mem_lock(unit, mem);

        rv = soc_mem_read(unit, mem, MEM_BLOCK_ANY, SOC_PORT_MOD_OFFSET(unit, port), &ptab);
        if (BCM_FAILURE(rv)) {
            soc_mem_unlock(unit, mem);
            return (rv);
        }
        _draco_port_tab_to_port_cfg(unit, port, &ptab, cfg);
        soc_mem_unlock(unit, mem);

        return BCM_E_NONE;
    }

    assert(0 && "port cfg get");
    return BCM_E_FAIL;
}

int
bcm_draco_port_cfg_set(int unit, bcm_port_t port, bcm_port_cfg_t *cfg)
{
    port_tab_entry_t	ptab;
    uint64		prreg;
    uint32		vlreg;
    soc_mem_t		mem;
    uint64		iconfig, oiconfig;
    int                 rv = BCM_E_NONE;

    /* higig port */
    if (IS_HG_PORT(unit, port)) {
	BCM_IF_ERROR_RETURN(READ_ICONFIGr(unit, port, &iconfig));
	oiconfig = iconfig;
	soc_reg64_field32_set(unit, ICONFIGr, &iconfig, CMLf, cfg->pc_cml);
	soc_reg64_field32_set(unit, ICONFIGr, &iconfig, PORT_DIS_ALLf,
			      cfg->pc_disc == BCM_PORT_DISCARD_ALL ? 1 : 0);
	if (COMPILER_64_NE(iconfig, oiconfig)) {
	    BCM_IF_ERROR_RETURN(WRITE_ICONFIGr(unit, port, iconfig));
	}
	return BCM_E_NONE;
    }

    /* ether ports */
    if (IS_E_PORT(unit, port)) {
        /* Set the default VLAN (and/or priority) for this port */
	if (SOC_IS_DRACO15(unit)) {
	    vlreg = 0;
	    soc_reg_field_set(unit, VLAN_DEFAULTr, &vlreg,
			      VLAN_IDf, cfg->pc_vlan);

	    BCM_IF_ERROR_RETURN(WRITE_VLAN_DEFAULTr(unit, port, vlreg));
	} else {
	    COMPILER_64_ZERO(prreg);
	    soc_reg64_field32_set(unit, PRTABLE_DEFAULTr, &prreg,
				  VLAN_IDf, cfg->pc_vlan);
#if defined(BCM_TUCANA_SUPPORT) || defined(BCM_LYNX_SUPPORT)
	    if (SOC_IS_TUCANA(unit) || SOC_IS_LYNX(unit)) {
		soc_reg64_field32_set(unit, PRTABLE_DEFAULTr, &prreg,
				      PRIf, cfg->pc_new_opri);
	    }
#endif
	    BCM_IF_ERROR_RETURN(WRITE_PRTABLE_DEFAULTr(unit, port, prreg));
	}

	mem = SOC_PORT_MEM_TAB(unit, port);
        soc_mem_lock(unit, mem);

        if ((rv = soc_mem_read(unit, mem, MEM_BLOCK_ANY,
                               SOC_PORT_MOD_OFFSET(unit, port), &ptab)) < 0) {
            soc_mem_unlock(unit, mem);
            return rv;
        }

        _draco_port_cfg_to_port_tab(unit, port, &ptab, cfg);

        rv = soc_mem_write(unit, mem, MEM_BLOCK_ALL,
                           SOC_PORT_MOD_OFFSET(unit, port), &ptab);

        soc_mem_unlock(unit, mem);
	return rv;
    }

    assert(0 && "port cfg set");
    return BCM_E_FAIL;
}

#if defined(BCM_DRACO15_SUPPORT)
/*
 * Function:
 *      _bcm_draco15_port_dtag_mode_set
 * Description:
 *      Set the double-tagging mode of a port.
 * Parameters:
 *      unit - (IN) BCM device number
 *      port - (IN) Port number
 *      mode - (IN) Double-tagging mode, one of:
 *              BCM_PORT_DTAG_MODE_NONE            No double tagging
 *              BCM_PORT_DTAG_MODE_INTERNAL        Service Provider port
 *              BCM_PORT_DTAG_MODE_EXTERNAL        Customer port
 *              BCM_PORT_DTAG_REMOVE_EXTERNAL_TAG  Remove customer tag
 *              BCM_PORT_DTAG_ADD_EXTERNAL_TAG     Add customer tag
 *      dt_mode - (IN) True/False double tagged port.
 *      ignore_tag - (IN) Ignore outer tag. 
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *      BCM_PORT_DTAG_MODE_INTERNAL is for service provider ports.
 *              A tag will be added if the packet does not already
 *              have the internal TPID (see bcm_port_tpid_set below).
 *              Internally this sets DT_MODE and clears IGNORE_TAG.
 *      BCM_PORT_DTAG_MODE_EXTERNAL is for customer ports.
 *              The service provider TPID will always be added
 *              (see bcm_port_tpid_set below).
 *              Internally this sets DT_MODE and sets IGNORE_TAG.
 */
int
_bcm_draco15_port_dtag_mode_set(int unit, bcm_port_t port, int mode, 
                           int dt_mode, int ignore_tag)
{
    uint32  config, oconfig;
    uint64  vctrl, ovctrl;

    SOC_IF_ERROR_RETURN(READ_CONFIGr(unit, port, &config));
    oconfig = config;
    SOC_IF_ERROR_RETURN(READ_VLAN_CONTROLr(unit, port, &vctrl));
    ovctrl = vctrl;
    soc_reg_field_set(unit, CONFIGr, &config, DT_MODEf, dt_mode);
    soc_reg64_field32_set(unit, VLAN_CONTROLr, &vctrl, IGNORE_TAGf, ignore_tag);
    if (COMPILER_64_NE(vctrl, ovctrl)) {
        SOC_IF_ERROR_RETURN(WRITE_VLAN_CONTROLr(unit, port, vctrl));
    }
    if (config != oconfig) {
        SOC_IF_ERROR_RETURN(WRITE_CONFIGr(unit, port, config));
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_draco15_port_dtag_mode_get
 * Description:
 *      Return the current double-tagging mode of a port.
 * Parameters:
 *      unit - (IN) BCM device number
 *      port - (IN) Port number
 *      mode - (OUT) Double-tagging mode
 * Return Value:
 *      BCM_E_XXX
 */

int
_bcm_draco15_port_dtag_mode_get(int unit, bcm_port_t port, int *mode)
{
    uint32  config;
    uint64  vctrl;

    SOC_IF_ERROR_RETURN(READ_CONFIGr(unit, port, &config));
    if (soc_reg_field_get(unit, CONFIGr, config, DT_MODEf) == 0) {
        *mode = BCM_PORT_DTAG_MODE_NONE;
    } else {
        SOC_IF_ERROR_RETURN(READ_VLAN_CONTROLr(unit, port, &vctrl));
        if (soc_reg64_field32_get(unit, VLAN_CONTROLr, vctrl, IGNORE_TAGf)) {
            *mode = BCM_PORT_DTAG_MODE_EXTERNAL;
        } else {
            *mode = BCM_PORT_DTAG_MODE_INTERNAL;
        }
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_draco15_port_protocol_vlan_add
 * Purpose:
 *      Adds a protocol based vlan to a port.  The protocol
 *      is matched by frame type and ether type.  
 * Parameters:
 *      unit  - (IN) BCM device number
 *      port  - (IN) Port number
 *      frame - (IN) one of BCM_PORT_FRAMETYPE_{ETHER2,8023,LLC}
 *      ether - (IN) 16 bit Ethernet type field
 *      vid   - (IN)VLAN ID
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_draco15_port_protocol_vlan_add(int unit,
                           bcm_port_t port,
                           bcm_port_frametype_t frame,
                           bcm_port_ethertype_t ether,
                           bcm_vlan_t vid)
{
    bcm_port_t              p;
    vlan_protocol_entry_t   vpe;
    vlan_data_entry_t       vde;
    bcm_port_frametype_t    ft;
    bcm_port_ethertype_t    et;
    _bcm_port_info_t        *pinfo;
    int                     idxmin, idxmax;
    int                     vlan_prot_entries;
    int                     vlan_data_prot_start;
    int                     vpentry;
    int                     vdentry;
    int                     i, empty;

    idxmin = soc_mem_index_min(unit, VLAN_PROTOCOLm);
    idxmax = soc_mem_index_max(unit, VLAN_PROTOCOLm);
    vlan_prot_entries = soc_mem_index_count(unit, VLAN_PROTOCOLm);
    vlan_data_prot_start = soc_mem_index_max(unit, VLAN_SUBNETm) + 1;
    vpentry = empty = -1;
    for (i = idxmin; i <= idxmax; i++) {
        SOC_IF_ERROR_RETURN
            (READ_VLAN_PROTOCOLm(unit, MEM_BLOCK_ANY, i, &vpe));
        ft = 0;
        if (soc_VLAN_PROTOCOLm_field32_get(unit, &vpe, ETHERIIf)) {
            ft |= BCM_PORT_FRAMETYPE_ETHER2;
        }
        if (soc_VLAN_PROTOCOLm_field32_get(unit, &vpe, SNAPf)) {
            ft |= BCM_PORT_FRAMETYPE_8023;
        }
        if (soc_VLAN_PROTOCOLm_field32_get(unit, &vpe, LLCf)) {
            ft |= BCM_PORT_FRAMETYPE_LLC;
        }
        et = soc_VLAN_PROTOCOLm_field32_get(unit, &vpe, ETHERTYPEf);
        if (empty < 0 && ft == 0) {
            empty = i;
        }
        if (ft == frame && et == ether) {
            vpentry = i;
            break;
        }
    }
    if (vpentry < 0 && empty < 0) {
        return BCM_E_FULL;
    }
    if (vpentry < 0) {
        sal_memset(&vpe, 0, sizeof(vpe));
        soc_VLAN_PROTOCOLm_field32_set(unit, &vpe, ETHERTYPEf, ether);
        if (frame & BCM_PORT_FRAMETYPE_ETHER2) {
            soc_VLAN_PROTOCOLm_field32_set(unit, &vpe, ETHERIIf, 1);
        }
        if (frame & BCM_PORT_FRAMETYPE_8023) {
            soc_VLAN_PROTOCOLm_field32_set(unit, &vpe, SNAPf, 1);
        }
        if (frame & BCM_PORT_FRAMETYPE_LLC) {
            soc_VLAN_PROTOCOLm_field32_set(unit, &vpe, LLCf, 1);
        }
        soc_VLAN_PROTOCOLm_field32_set(unit, &vpe, MATCHUPPERf, 1);
        soc_VLAN_PROTOCOLm_field32_set(unit, &vpe, MATCHLOWERf, 1);
        SOC_IF_ERROR_RETURN
            (WRITE_VLAN_PROTOCOLm(unit, MEM_BLOCK_ALL, empty, &vpe));
        vpentry = empty;
    }

    /*
     * Set VLAN ID for target port. For all other GE ports,
     * set default VLAN ID in entries indexed by the matched entry in
     * VLAN_PROTOCOL.
     */
    PBMP_E_ITER(unit, p) {
        bcm_vlan_t  cvid, defvid, wvid;
        vdentry = vlan_data_prot_start
            + (p * vlan_prot_entries)
            + vpentry;
        SOC_IF_ERROR_RETURN
            (READ_VLAN_DATAm(unit, MEM_BLOCK_ANY, vdentry, &vde));
        wvid = vid;
        cvid = soc_VLAN_DATAm_field32_get(unit, &vde, VLAN_IDf);
        BCM_IF_ERROR_RETURN
            (bcm_esw_port_untagged_vlan_get(unit, p, &defvid));

        BCM_IF_ERROR_RETURN(_bcm_port_info_get(unit, p, &pinfo));
        if (p == port) {
            if (_BCM_PORT_VD_PBVL_IS_SET(pinfo, vpentry)) {
                return BCM_E_EXISTS;
            }
            _BCM_PORT_VD_PBVL_SET(pinfo, vpentry);
            if (cvid == vid) {
                continue;
            }
        } else {
            /*
             * For all other ports, if no VLAN ID already set,
             * set to default VLAN ID.
             */
            if (cvid != 0) {
                continue;
            }
            wvid = defvid;
            _BCM_PORT_VD_PBVL_CLEAR(pinfo, vpentry);
        }

        sal_memset(&vde, 0, sizeof(vde));
        soc_VLAN_DATAm_field32_set(unit, &vde, VLAN_IDf, wvid);
        soc_VLAN_DATAm_field32_set(unit, &vde, PRIf, pinfo->p_ut_prio);
        soc_VLAN_DATAm_field32_set(unit, &vde, CNGf, 0);
        SOC_IF_ERROR_RETURN
            (WRITE_VLAN_DATAm(unit, MEM_BLOCK_ALL, vdentry, &vde));
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_draco15_port_protocol_vlan_delete
 * Purpose:
 *      Remove an already created proto protocol based vlan
 *      on a port.
 * Parameters:
 *      unit - (IN) BCM device number
 *      port - (IN) Port number
 *      frame - (IN) one of BCM_PORT_FRAMETYPE_{ETHER2,8023,LLC}
 *      ether - (IN) 16 bit Ethernet type field
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_draco15_port_protocol_vlan_delete(int unit,
                                       bcm_port_t port,
                                       bcm_port_frametype_t frame,
                                       bcm_port_ethertype_t ether)
{
    bcm_port_frametype_t    ft;
    bcm_port_ethertype_t    et;
    vlan_protocol_entry_t   vpe;
    vlan_data_entry_t       vde;
    bcm_vlan_t              cvid, defvid;
    _bcm_port_info_t        *pinfo;
    int                     idxmin, idxmax;
    int                     vlan_prot_entries, vlan_data_prot_start;
    int                     vpentry, vdentry, p, valid;
    int                     i;

    idxmin = soc_mem_index_min(unit, VLAN_PROTOCOLm);
    idxmax = soc_mem_index_max(unit, VLAN_PROTOCOLm);
    vlan_prot_entries = soc_mem_index_count(unit, VLAN_PROTOCOLm);
    vlan_data_prot_start = soc_mem_index_max(unit, VLAN_SUBNETm) + 1;
    vpentry = -1;
    for (i = idxmin; i <= idxmax; i++) {
        SOC_IF_ERROR_RETURN
            (READ_VLAN_PROTOCOLm(unit, MEM_BLOCK_ANY, i, &vpe));
        ft = 0;
        if (soc_VLAN_PROTOCOLm_field32_get(unit, &vpe, ETHERIIf)) {
            ft |= BCM_PORT_FRAMETYPE_ETHER2;
        }
        if (soc_VLAN_PROTOCOLm_field32_get(unit, &vpe, SNAPf)) {
            ft |= BCM_PORT_FRAMETYPE_8023;
        }
        if (soc_VLAN_PROTOCOLm_field32_get(unit, &vpe, LLCf)) {
            ft |= BCM_PORT_FRAMETYPE_LLC;
        }
        et = soc_VLAN_PROTOCOLm_field32_get(unit, &vpe, ETHERTYPEf);
        if (ft == frame && et == ether) {
            vpentry = i;
            break;
        }
    }
    if (vpentry < 0) {
        return BCM_E_NOT_FOUND;
    }
    vdentry = vlan_data_prot_start + (port * vlan_prot_entries) + vpentry;
    SOC_IF_ERROR_RETURN
        (READ_VLAN_DATAm(unit, MEM_BLOCK_ANY, vdentry, &vde));
    cvid = soc_VLAN_DATAm_field32_get(unit, &vde, VLAN_IDf);
    BCM_IF_ERROR_RETURN
        (bcm_esw_port_untagged_vlan_get(unit, port, &defvid));
    /* entry not valid VLAN ID is 0 or has default id but not programmed */
    BCM_IF_ERROR_RETURN(_bcm_port_info_get(unit, port, &pinfo));
    if (cvid == 0 ||
        (cvid == defvid && (!_BCM_PORT_VD_PBVL_IS_SET(pinfo, vpentry)))) {
        return BCM_E_NOT_FOUND;
    }
    sal_memset(&vde, 0, sizeof(vde));
    /* set to default VLAN ID and mark it as not programmed   */
    soc_VLAN_DATAm_field32_set(unit, &vde, VLAN_IDf, defvid);
    _BCM_PORT_VD_PBVL_CLEAR(pinfo, vpentry);
    SOC_IF_ERROR_RETURN
        (WRITE_VLAN_DATAm(unit, MEM_BLOCK_ALL, vdentry, &vde));
    /* see if any vlan_data entries are still valid */
    valid = 0;
    PBMP_E_ITER(unit, p) {
        if (p == port) {    /* skip the entry we just wrote */
            continue;
        }
        BCM_IF_ERROR_RETURN(_bcm_port_info_get(unit, p, &pinfo));
        /* entry valid if programmed VLAN ID for the port */
        if (_BCM_PORT_VD_PBVL_IS_SET(pinfo, vpentry)) {
            valid = 1;
            break;
        }
    }
    if (!valid) {
        /* clear all VLAN_DATA entries associated with this protocol */
        PBMP_E_ITER(unit, p) {
            vdentry = vlan_data_prot_start + (p * vlan_prot_entries)
                + vpentry;
            sal_memset(&vde, 0, sizeof(vde));
            SOC_IF_ERROR_RETURN
                (WRITE_VLAN_DATAm(unit, MEM_BLOCK_ALL, vdentry, &vde));
            BCM_IF_ERROR_RETURN(_bcm_port_info_get(unit, p, &pinfo));
            _BCM_PORT_VD_PBVL_CLEAR(pinfo, vpentry);
        }
        /* clear VLAN_PROTOCOL entry */
        sal_memset(&vpe, 0, sizeof(vpe));
        SOC_IF_ERROR_RETURN
            (WRITE_VLAN_PROTOCOLm(unit, MEM_BLOCK_ALL, vpentry, &vpe));
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_d15_port_untagged_vlan_data_update
 * Purpose:
 *      Update VLAN_DATA with new default VLAN ID for port
 * Parameters:
 *      unit - StrataSwitch unit number.
 *      port - StrataSwitch port number of port to get info for
 *      pdvid- Previous default VLAN ID
 *      ndvid- New default VLAN ID
 * Returns:
 *      BCM_XX_NONE
 */
int
_bcm_d15_port_untagged_vlan_data_update(int unit, bcm_port_t port,
                                        bcm_vlan_t pdvid, bcm_vlan_t ndvid)
{
    _bcm_port_info_t    *pinfo;
    vlan_data_entry_t   vde;
    int                 vdentry, vdend, idx;
    int                 vlan_prot_entries, vlan_data_prot_start;

    vlan_prot_entries = soc_mem_index_count(unit, VLAN_PROTOCOLm);
    vlan_data_prot_start = soc_mem_index_max(unit, VLAN_SUBNETm) + 1;
    for (vdentry   = vlan_data_prot_start + (port * vlan_prot_entries),
         vdend = vdentry + vlan_prot_entries, idx = 0;
         vdentry < vdend; vdentry++, idx++) {
        SOC_IF_ERROR_RETURN
            (READ_VLAN_DATAm(unit, MEM_BLOCK_ANY, vdentry, &vde));
        /*
         * Update entry with new default VLAN ID if not a programmed
         * VLAN ID and entry has previous default VLAN ID
         */
        BCM_IF_ERROR_RETURN(_bcm_port_info_get(unit, port, &pinfo));

        if (!(_BCM_PORT_VD_PBVL_IS_SET(pinfo, idx)) &&
            soc_VLAN_DATAm_field32_get(unit, &vde, VLAN_IDf) == pdvid) {
            soc_VLAN_DATAm_field32_set(unit, &vde, VLAN_IDf, ndvid);
            SOC_IF_ERROR_RETURN
                (WRITE_VLAN_DATAm(unit, MEM_BLOCK_ALL, vdentry, &vde));
        }
    }
    return BCM_E_NONE;
}
#endif /* BCM_DRACO15_SUPPORT */

#if defined(BCM_DRACO1_SUPPORT) || defined(BCM_TUCANA_SUPPORT)
/*
 * Function:
 *      _bcm_draco_port_protocol_vlan_add
 * Purpose:
 *      Adds a protocol based vlan to a port.  The protocol
 *      is matched by frame type and ether type.  
 * Parameters:
 *      unit  - (IN) BCM device number
 *      port  - (IN) Port number
 *      frame - (IN) one of BCM_PORT_FRAMETYPE_{ETHER2,8023,LLC}
 *      ether - (IN) 16 bit Ethernet type field
 *      vid   - (IN)VLAN ID
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_draco_port_protocol_vlan_add(int unit,
                           bcm_port_t port,
                           bcm_port_frametype_t frame,
                           bcm_port_ethertype_t ether,
                           bcm_vlan_t vid)
{
    bcm_port_frametype_t    ft;
    bcm_port_ethertype_t    et;
    _bcm_port_info_t        *pinfo;
    uint64                  regval;
    int                     i;
    int                     empty;

    empty = -1;

    for (i = 0; i < SOC_REG_NUMELS(unit, PRTABLE_ENTRYr); i++) {
        SOC_IF_ERROR_RETURN(READ_PRTABLE_ENTRYr(unit, port, i, &regval));
        ft = soc_reg64_field32_get(unit, PRTABLE_ENTRYr, regval,
                                   FRAMETYPEf);
        et = soc_reg64_field32_get(unit, PRTABLE_ENTRYr, regval,
                                   ETHERTYPEf);
        if (empty < 0 && ft == 0) {
            empty = i;
        }
        if (ft == frame && et == ether) {
            return BCM_E_EXISTS;
        }
    }
    if (empty < 0) {
        return BCM_E_FULL;
    }
    soc_reg64_field32_set(unit, PRTABLE_ENTRYr, &regval,
                          FRAMETYPEf, frame);
    soc_reg64_field32_set(unit, PRTABLE_ENTRYr, &regval,
                          ETHERTYPEf, ether);
    soc_reg64_field32_set(unit, PRTABLE_ENTRYr, &regval,
                          VLAN_IDf, vid);
    if (SOC_IS_TUCANA(unit)) {
        BCM_IF_ERROR_RETURN(_bcm_port_info_get(unit, port, &pinfo));
        soc_reg64_field32_set(unit, PRTABLE_ENTRYr, &regval,
                              PRIf, pinfo->p_ut_prio);
    }
    SOC_IF_ERROR_RETURN(WRITE_PRTABLE_ENTRYr(unit, port, empty, regval));
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_draco_port_protocol_vlan_delete
 * Purpose:
 *      Remove an already created proto protocol based vlan
 *      on a port.
 * Parameters:
 *      unit - (IN) BCM device number
 *      port - (IN) Port number
 *      frame - (IN) one of BCM_PORT_FRAMETYPE_{ETHER2,8023,LLC}
 *      ether - (IN) 16 bit Ethernet type field
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_draco_port_protocol_vlan_delete(int unit,
                              bcm_port_t port,
                              bcm_port_frametype_t frame,
                              bcm_port_ethertype_t ether)
{
    int                     i;
    uint64                  regval;
    bcm_port_frametype_t    ft;
    bcm_port_ethertype_t    et;

    for (i = 0; i < SOC_REG_NUMELS(unit, PRTABLE_ENTRYr); i++) {
        SOC_IF_ERROR_RETURN(READ_PRTABLE_ENTRYr(unit, port, i, &regval));
        ft = soc_reg64_field32_get(unit, PRTABLE_ENTRYr, regval,
                                   FRAMETYPEf);
        et = soc_reg64_field32_get(unit, PRTABLE_ENTRYr, regval,
                                   ETHERTYPEf);
        if (ft == frame && et == ether) {
            COMPILER_64_ZERO(regval);
            SOC_IF_ERROR_RETURN
                (WRITE_PRTABLE_ENTRYr(unit, port, i, regval));
            return BCM_E_NONE;
        }
    }
    return BCM_E_NOT_FOUND;
}
#endif /* BCM_DRACO1_SUPPORT || BCM_TUCANA_SUPPORT */

#if (defined(BCM_DRACO15_SUPPORT) || defined(BCM_LYNX_SUPPORT) || \
     defined(BCM_TUCANA_SUPPORT))
/*
 * Function:
 *      _bcm_draco_port_tpid_get
 * Description:
 *      Retrieve the default Tag Protocol ID for a port.
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      tpid - (OUT) Tag Protocol ID
 * Return Value:
 *      BCM_E_XXX
 */
int
_bcm_draco_port_tpid_get(int unit, bcm_port_t port, uint16 *tpid)
{
    uint64  vctrl;

    /* Input parameters check. */
    if (tpid == NULL) {
        return BCM_E_PARAM;
    }

    if (IS_HG_PORT(unit, port)) {
        if (SOC_IS_LYNX(unit)) {
            *tpid = 0x8100;
            return BCM_E_NONE;
        }
        SOC_IF_ERROR_RETURN(READ_IVLAN_CONTROLr(unit, port, &vctrl));
        *tpid = soc_reg64_field32_get(unit, IVLAN_CONTROLr, vctrl, TPIDf);
    } else {
        SOC_IF_ERROR_RETURN(READ_VLAN_CONTROLr(unit, port, &vctrl));
        *tpid = soc_reg64_field32_get(unit, VLAN_CONTROLr, vctrl, TPIDf);
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_draco_port_tpid_set
 * Description:
 *      Set the default Tag Protocol ID for a port.
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      tpid - Tag Protocol ID
 * Return Value:
 *      BCM_E_XXX
 * Notes:
 *      This API is not specifically double-tagging-related, but
 *      the port TPID becomes the service provider TPID when double-tagging
 *      is enabled on a port.  The default TPID is 0x8100.
 *      On BCM5665, only 0x8100 is allowed for the inner (customer) tag.
 */
int
_bcm_draco_port_tpid_set(int unit, bcm_port_t port, uint16 tpid)
{
    uint64  vctrl, ovctrl;

    if (IS_HG_PORT(unit, port)) {
        if (SOC_IS_LYNX(unit)) {
            if (tpid == 0x8100) {
                return BCM_E_NONE;
            } else {
                return BCM_E_UNAVAIL;
            }
        }
        SOC_IF_ERROR_RETURN(READ_IVLAN_CONTROLr(unit, port, &vctrl));
        ovctrl = vctrl;
        soc_reg64_field32_set(unit, IVLAN_CONTROLr, &vctrl, TPIDf, tpid);
        if (COMPILER_64_NE(vctrl, ovctrl)) {
            SOC_IF_ERROR_RETURN(WRITE_IVLAN_CONTROLr(unit, port, vctrl));
        }
    } else {
        SOC_IF_ERROR_RETURN(READ_VLAN_CONTROLr(unit, port, &vctrl));
        ovctrl = vctrl;
        soc_reg64_field32_set(unit, VLAN_CONTROLr, &vctrl, TPIDf, tpid);
        if (COMPILER_64_NE(vctrl, ovctrl)) {
            SOC_IF_ERROR_RETURN(WRITE_VLAN_CONTROLr(unit, port, vctrl));
        }
    }
    return BCM_E_NONE;
}
#endif /* BCM_DRACO15_SUPPORT || BCM_TUCANA_SUPPORT || BCM_LYNX_SUPPORT */

/*
 * Function:
 *	bcm_draco_port_cfg_init
 * Purpose:
 *	Initialize the port configuration according to Initial System
 *	Configuration (see init.c)
 * Parameters:
 *	unit - StrataSwitch unit number.
 *	port - Port number
 *	vd - Initial VLAN data information
 * Returns:
 *	BCM_E_XXX
 */

int
bcm_draco_port_cfg_init(int unit, bcm_port_t port, bcm_vlan_data_t *vd)
{
    uint64	     prreg;
    uint32	     vlreg;
    soc_mem_t	     mem;
    port_tab_entry_t ptab;

    /* ether ports */
    if (IS_E_PORT(unit, port)) {
	if (SOC_IS_DRACO15(unit)) {
	    vlreg = 0;
	    soc_reg_field_set(unit, VLAN_DEFAULTr, &vlreg,
			      VLAN_IDf, vd->vlan_tag);
	    BCM_IF_ERROR_RETURN(WRITE_VLAN_DEFAULTr(unit, port, vlreg));
	} else {
	    COMPILER_64_ZERO(prreg);
	    soc_reg64_field32_set(unit, PRTABLE_DEFAULTr, &prreg,
				  VLAN_IDf, vd->vlan_tag);
	    BCM_IF_ERROR_RETURN(WRITE_PRTABLE_DEFAULTr(unit, port, prreg));
	}

	mem = SOC_PORT_MEM_TAB(unit, port);
	sal_memcpy(&ptab, soc_mem_entry_null(unit, PORT_TABm), sizeof (ptab));

	/* For XGS switch chips, RPE=1 technically prevents any ARL hits  */
	/* from reassigning internal priority. This differs from STRATA2  */
	/* chips where RPE=1 forces PORT.PRI as the assigned priority.    */
	/* The default setting here ignores any ARL priority assignment.  */
	soc_mem_field32_set(unit, mem, &ptab, RPEf, 1);

        BCM_IF_ERROR_RETURN
	    (soc_mem_write(unit, mem, MEM_BLOCK_ALL,
			   SOC_PORT_MOD_OFFSET(unit, port),
			   &ptab));
    }

    return BCM_E_NONE;
}


#endif	/* BCM_XGS_SWITCH_SUPPORT */
