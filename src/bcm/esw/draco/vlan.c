/*
 * $Id: vlan.c 1.25 Broadcom SDK $
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
 * File:        vlan.c
 * Purpose:	Provide low-level access to Draco VLAN resources
 */

#include <sal/core/boot.h>

#include <soc/drv.h>
#include <soc/mem.h>

#include <bcm/error.h>
#include <bcm/vlan.h>

#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/draco.h>

#include <bcm_int/esw_dispatch.h>

static uint16 *bcm_draco_vlan_port_bmp[BCM_MAX_NUM_UNITS];

STATIC int
bcm_draco_vlan_pbmp_clear(int unit, bcm_stg_t stg, bcm_pbmp_t *pbmp)
{
    bcm_pbmp_t fixed_pbmp;
    bcm_port_t port;
    int        stp_state;
    
    BCM_PBMP_ASSIGN(fixed_pbmp, *pbmp);
    
    BCM_PBMP_ITER(*pbmp, port) {
        if (port != CMIC_PORT(unit)) {
	    BCM_IF_ERROR_RETURN(bcm_esw_stg_stp_get(unit, stg, port, 
                                                    &stp_state));
 	    if (stp_state != BCM_STG_STP_FORWARD) {
	        BCM_PBMP_PORT_REMOVE(fixed_pbmp, port);
 	    }
 	}
    }
    
    BCM_PBMP_ASSIGN(*pbmp, fixed_pbmp);
    
    return BCM_E_NONE;
}
  
int
bcm_draco_vlan_stg_update(int unit, bcm_vlan_t vid, bcm_pbmp_t pbmp, int add)
{
    vlan_tab_entry_t vt;
    int              rv = BCM_E_NONE;
    uint32           bmval;
    bcm_pbmp_t       pbmp16;
    
    BCM_PBMP_CLEAR(pbmp16);
    SOC_PBMP_WORD_SET(pbmp16, 0, bcm_draco_vlan_port_bmp[unit][vid]); 
    BCM_PBMP_AND(pbmp, pbmp16);
    
    soc_mem_lock(unit, VLAN_TABm);
    
    if ((rv = READ_VLAN_TABm(unit, MEM_BLOCK_ANY, (int) vid, &vt)) < 0) {
        soc_mem_unlock(unit, VLAN_TABm);
 	return rv;
    }
    
    if (!soc_VLAN_TABm_field32_get(unit, &vt, VALIDf)) {
        soc_mem_unlock(unit, VLAN_TABm);
 	return BCM_E_NOT_FOUND;
    }
    
    soc_VLAN_TABm_field_get(unit, &vt, PORT_BITMAPf, &bmval);
    if (add) {
        bmval |= SOC_PBMP_WORD_GET(pbmp, 0);
    } else {
        bmval &= ~SOC_PBMP_WORD_GET(pbmp, 0);
    }
    soc_VLAN_TABm_field_set(unit, &vt, PORT_BITMAPf, &bmval);
    
    rv = WRITE_VLAN_TABm(unit, MEM_BLOCK_ALL, (int) vid, &vt);
    
    soc_mem_unlock(unit, VLAN_TABm);
    
    return rv;
}

/*
 * Function:
 *	bcm_draco_vlan_init
 * Purpose:
 *	Initialize the VLAN tables with the default entry in it.
 */

int
bcm_draco_vlan_init(int unit, bcm_vlan_data_t *vd)
{
    vlan_tab_entry_t	ve;
    bcm_pbmp_t pbm;

    if (soc_feature(unit, soc_feature_mstp_uipmc)) {
        if ( bcm_draco_vlan_port_bmp[unit] == NULL) {
            bcm_draco_vlan_port_bmp[unit] =
                sal_alloc(sizeof(bcm_vlan_t) * BCM_VLAN_COUNT, "VLAN cache table");
            if (NULL == bcm_draco_vlan_port_bmp[unit]) {
                return BCM_E_MEMORY;
            }
        }
        sal_memset(bcm_draco_vlan_port_bmp[unit], 0,
                   sizeof(bcm_vlan_t) * BCM_VLAN_COUNT);
    }
  
    if (SAL_BOOT_QUICKTURN || SAL_BOOT_PLISIM) {
	soc_cm_print("SIMULATION: skipped VLAN table clear "
		     "(assuming hardware did it)\n");
    } else {
	SOC_IF_ERROR_RETURN
	    (soc_mem_clear(unit, VLAN_TABm, MEM_BLOCK_ALL, TRUE));

	if (SOC_IS_DRACO15(unit)) {
	    /* Default to MAC based VLAN mode (VLAN translate off) */
	    bcm_esw_vlan_control_set(unit, bcmVlanTranslate, FALSE);
	    (void)bcm_esw_vlan_mac_delete_all(unit);
	    (void)bcm_esw_vlan_ip4_delete_all(unit);
	}
    }

    sal_memcpy(&ve, soc_mem_entry_null(unit, VLAN_TABm), sizeof (ve));

    BCM_PBMP_ASSIGN(pbm, vd->ut_port_bitmap);
    BCM_PBMP_AND(pbm, PBMP_E_ALL(unit));
    soc_VLAN_TABm_field32_set(unit, &ve, PORT_BITMAPf,
			      SOC_PBMP_WORD_GET(vd->port_bitmap, 0));
    soc_VLAN_TABm_field32_set(unit, &ve, UT_BITMAPf,
			      SOC_PBMP_WORD_GET(pbm, 0));
    soc_VLAN_TABm_field32_set(unit, &ve, STGf, BCM_STG_DEFAULT);
    soc_VLAN_TABm_field32_set(unit, &ve, VALIDf, 1);

    SOC_IF_ERROR_RETURN
	(WRITE_VLAN_TABm(unit, MEM_BLOCK_ALL, vd->vlan_tag, &ve));

    if (soc_feature(unit, soc_feature_mstp_uipmc)) {
        bcm_draco_vlan_port_bmp[unit][vd->vlan_tag] =
	  SOC_PBMP_WORD_GET(vd->port_bitmap, 0);
    }
    
    if (soc_feature(unit, soc_feature_mstp_lookup)) {
        /* 
	 * For 5695_b+, MSTP_TBLm does a VLAN -> STG lookup.
	 * Copy BCM_STG_DEFAULT to MSTP_TBLm[vd->vlan_tag]. 
	 */
        mmu_memories1_mstp_tbl_entry_t mstp_entry;
	sal_memset(&mstp_entry, 0, sizeof(mstp_entry));
	soc_MMU_MEMORIES1_MSTP_TBLm_field32_set(unit, &mstp_entry, 
						MSTP_TBLf, 
						BCM_STG_DEFAULT);
	BCM_IF_ERROR_RETURN(WRITE_MMU_MEMORIES1_MSTP_TBLm(unit, 
							  MEM_BLOCK_ALL, 
							  vd->vlan_tag, 
							  &mstp_entry));
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_draco_vlan_reload
 * Purpose:
 *	Return a bitmap of VLANs currently installed in the hardware.
 */

int
bcm_draco_vlan_reload(int unit, vbmp_t *bmp, int *count)
{
    vlan_tab_entry_t	vt;
    int 		index;
    int 		rv;

    (*count) = 0;

    for (index = soc_mem_index_min(unit, VLAN_TABm);
         index <= soc_mem_index_max(unit, VLAN_TABm);
	 index++) {
        if ((rv = READ_VLAN_TABm(unit, MEM_BLOCK_ANY, index, &vt)) < 0) {
	    return rv;
        }

        if (soc_VLAN_TABm_field32_get(unit, &vt, VALIDf)) {
            SHR_BITSET(bmp->w, index);
            (*count)++;
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_draco_vlan_create
 * Purpose:
 *	Create a VLAN entry in VLAN_TAB
 */

int
bcm_draco_vlan_create(int unit, bcm_vlan_t vid)
{
    vlan_tab_entry_t		vt;
    int				rv;

    /* Upper layer already checks that vid is valid */

    if (SOC_IS_RELOADING(unit)) {
        return BCM_E_NONE;	  
    }

    soc_mem_lock(unit, VLAN_TABm);

    if ((rv = READ_VLAN_TABm(unit, MEM_BLOCK_ANY, (int) vid, &vt)) < 0) {
	soc_mem_unlock(unit, VLAN_TABm);
	return rv;
    }

    if (soc_VLAN_TABm_field32_get(unit, &vt, VALIDf)) {
	soc_mem_unlock(unit, VLAN_TABm);
	return BCM_E_EXISTS;
    }

    /*
     * The STG value doesn't matter much; it will get changed
     * immediately after this routine is called.
     */

    soc_VLAN_TABm_field32_set(unit, &vt, UT_BITMAPf, 0);
    soc_VLAN_TABm_field32_set(unit, &vt, STGf, 0);
    soc_VLAN_TABm_field32_set(unit, &vt, VALIDf, 1);

    rv = WRITE_VLAN_TABm(unit, MEM_BLOCK_ALL, (int) vid, &vt);

    if (soc_feature(unit, soc_feature_mstp_uipmc)) {
        /* Clear the VLAN's port bitmap */
        bcm_draco_vlan_port_bmp[unit][vid] = 0;
    }
     
    soc_mem_unlock(unit, VLAN_TABm);

    if ( BCM_SUCCESS(rv) &&
	 soc_feature(unit, soc_feature_mstp_lookup) ) {
        /* 
	 * For 5695_b+, MSTP_TBLm does a VLAN -> STG lookup.
	 * Zero the MSTP_TBLm[vd->vlan_tag] entry. 
	 */
        mmu_memories1_mstp_tbl_entry_t mstp_entry;
	sal_memset(&mstp_entry, 0, sizeof(mstp_entry));
	rv = WRITE_MMU_MEMORIES1_MSTP_TBLm(unit, MEM_BLOCK_ALL, 
					   vid, &mstp_entry);
    }

    return rv;
}

/*
 * Function:
 *	bcm_draco_vlan_destroy
 * Purpose:
 *	Clear a VLAN entry in VLAN_TAB
 */

int
bcm_draco_vlan_destroy(int unit, bcm_vlan_t vid)
{
    vlan_tab_entry_t		vt;
    int				rv;

    /* Upper layer already checks that vid is valid */

    soc_mem_lock(unit, VLAN_TABm);

    if ((rv = READ_VLAN_TABm(unit, MEM_BLOCK_ANY, (int) vid, &vt)) < 0) {
	soc_mem_unlock(unit, VLAN_TABm);
	return rv;
    }

    if (!soc_VLAN_TABm_field32_get(unit, &vt, VALIDf)) {
	soc_mem_unlock(unit, VLAN_TABm);
	return BCM_E_NOT_FOUND;
    }

    rv = WRITE_VLAN_TABm(unit, MEM_BLOCK_ALL, (int) vid,
			 soc_mem_entry_null(unit, VLAN_TABm));

    if (soc_feature(unit, soc_feature_mstp_uipmc)) {
        /* Clear the VLAN's port bitmap */
        bcm_draco_vlan_port_bmp[unit][vid] = 0;
    }
    
    soc_mem_unlock(unit, VLAN_TABm);

    if ( BCM_SUCCESS(rv) &&
	 soc_feature(unit, soc_feature_mstp_lookup) ) {
        /* 
	 * For 5695_b+, MSTP_TBLm does a VLAN -> STG lookup.
	 * Null the MSTP_TBLm[vd->vlan_tag] entry. 
	 */
        mmu_memories1_mstp_tbl_entry_t mstp_entry;
	uint32 stg_val;
	sal_memset(&mstp_entry, 0, sizeof(mstp_entry));
	stg_val = 
	  soc_VLAN_TABm_field32_get(unit, 
				    soc_mem_entry_null(unit, VLAN_TABm), 
				    STGf);
	soc_MMU_MEMORIES1_MSTP_TBLm_field32_set(unit, &mstp_entry, 
						MSTP_TBLf, 
						stg_val);
	rv = WRITE_MMU_MEMORIES1_MSTP_TBLm(unit, 
					   MEM_BLOCK_ALL, 
					   vid, &mstp_entry);
    }

    return rv;
}

/*
 * Function:
 *	bcm_draco_vlan_port_add
 * Purpose:
 *	Add ports to the VLAN bitmap in a VLAN_TAB entry.
 */

int
bcm_draco_vlan_port_add(int unit, bcm_vlan_t vid, pbmp_t pbmp, pbmp_t ubmp,
                        pbmp_t ing_pbmp)
{
    vlan_tab_entry_t		vt;
    int				rv;
    bcm_pbmp_t                  hw_pbmp;
    uint32			bmval;

    BCM_PBMP_OR(pbmp, ing_pbmp);

    /* Only allow untagged ports on the vlan. */
    SOC_PBMP_AND(ubmp, pbmp);

    /* Draco does not have room for CMIC/IPIC in ubmp */
    SOC_PBMP_REMOVE(ubmp, PBMP_CMIC(unit));
    SOC_PBMP_REMOVE(ubmp, PBMP_HG_ALL(unit));

    /* Upper layer already checks that vid is valid */

    soc_mem_lock(unit, VLAN_TABm);

    if ((rv = READ_VLAN_TABm(unit, MEM_BLOCK_ANY, (int) vid, &vt)) < 0) {
	soc_mem_unlock(unit, VLAN_TABm);
	return rv;
    }

    if (!soc_VLAN_TABm_field32_get(unit, &vt, VALIDf)) {
	soc_mem_unlock(unit, VLAN_TABm);
	return BCM_E_NOT_FOUND;
    }

    BCM_PBMP_ASSIGN(hw_pbmp, pbmp);
  
    if (soc_feature(unit, soc_feature_mstp_uipmc)) {
        bcm_stg_t  stg;
 	int        rv;
  
 	/*
 	 * Find out which STG this VLAN belongs to
 	 */
 	stg = soc_VLAN_TABm_field32_get(unit, &vt, STGf);
 	
 	rv = bcm_draco_vlan_pbmp_clear(unit, stg, &hw_pbmp);
 	if (!BCM_SUCCESS(rv)) {
	    soc_mem_unlock(unit, VLAN_TABm);
 	    return rv;
 	}
 	
 	bcm_draco_vlan_port_bmp[unit][vid] |=
 	  SOC_PBMP_WORD_GET(pbmp, 0); 
    }
    
    /*
     * Update the entry for this VLAN.
     */
    soc_VLAN_TABm_field_get(unit, &vt, PORT_BITMAPf, &bmval);
    bmval |= SOC_PBMP_WORD_GET(hw_pbmp, 0);
    soc_VLAN_TABm_field_set(unit, &vt, PORT_BITMAPf, &bmval);

    soc_VLAN_TABm_field_get(unit, &vt, UT_BITMAPf, &bmval);
    bmval &= ~SOC_PBMP_WORD_GET(pbmp, 0);
    bmval |= SOC_PBMP_WORD_GET(ubmp, 0);

    soc_VLAN_TABm_field_set(unit, &vt, UT_BITMAPf, &bmval);

    rv = WRITE_VLAN_TABm(unit, MEM_BLOCK_ALL, (int) vid, &vt);

    soc_mem_unlock(unit, VLAN_TABm);

    return rv;
}

/*
 * Function:
 *	bcm_draco_vlan_port_remove
 * Purpose:
 *	Remove ports from the VLAN bitmap in a VLAN_TAB entry.
 */

int
bcm_draco_vlan_port_remove(int unit, bcm_vlan_t vid, pbmp_t pbmp)
{
    vlan_tab_entry_t		vt;
    int				rv;
    uint32			bmval;

    /* Upper layer already checks that vid is valid */

    soc_mem_lock(unit, VLAN_TABm);

    if ((rv = READ_VLAN_TABm(unit, MEM_BLOCK_ANY, (int) vid, &vt)) < 0) {
	soc_mem_unlock(unit, VLAN_TABm);
	return rv;
    }

    if (!soc_VLAN_TABm_field32_get(unit, &vt, VALIDf)) {
	soc_mem_unlock(unit, VLAN_TABm);
	return BCM_E_NOT_FOUND;
    }

    if (soc_feature(unit, soc_feature_mstp_uipmc)) {
        bcm_draco_vlan_port_bmp[unit][vid] &=
 	  (~SOC_PBMP_WORD_GET(pbmp, 0) & 0xffff);
    }
    
    soc_VLAN_TABm_field_get(unit, &vt, PORT_BITMAPf, &bmval);
    bmval &= ~SOC_PBMP_WORD_GET(pbmp, 0);
    soc_VLAN_TABm_field_set(unit, &vt, PORT_BITMAPf, &bmval);

    soc_VLAN_TABm_field_get(unit, &vt, UT_BITMAPf, &bmval);
    bmval &= ~SOC_PBMP_WORD_GET(pbmp, 0);
    soc_VLAN_TABm_field_set(unit, &vt, UT_BITMAPf, &bmval);

    rv = WRITE_VLAN_TABm(unit, MEM_BLOCK_ALL, (int) vid, &vt);

    soc_mem_unlock(unit, VLAN_TABm);

    return rv;
}

/*
 * Function:
 *	bcm_draco_vlan_port_get
 * Purpose:
 *	Read the port bitmap from a VLAN_TAB entry.
 */

int
bcm_draco_vlan_port_get(int unit, bcm_vlan_t vid, pbmp_t *pbmp, pbmp_t *ubmp,
                        pbmp_t *ing_pbmp)
{
    vlan_tab_entry_t		vt;
    pbmp_t                      local_pbmp;

    /* Upper layer already checks that vid is valid */

    BCM_IF_ERROR_RETURN
	(READ_VLAN_TABm(unit, MEM_BLOCK_ANY, (int) vid, &vt));

    if (!soc_VLAN_TABm_field32_get(unit, &vt, VALIDf)) {
	return BCM_E_NOT_FOUND;
    }

    SOC_PBMP_CLEAR(local_pbmp);
    SOC_PBMP_WORD_SET(local_pbmp, 0,
                      soc_VLAN_TABm_field32_get(unit, &vt, PORT_BITMAPf));
    if (soc_feature(unit, soc_feature_mstp_uipmc)) {
        bcm_pbmp_t pbmp16;
        BCM_PBMP_CLEAR(pbmp16);
        SOC_PBMP_WORD_SET(pbmp16, 0, bcm_draco_vlan_port_bmp[unit][vid]);
        BCM_PBMP_ASSIGN(local_pbmp, pbmp16);
    }

    if (pbmp != NULL) {
        BCM_PBMP_ASSIGN(*pbmp, local_pbmp);
    }

    if (ubmp != NULL) {
	SOC_PBMP_CLEAR(*ubmp);
	SOC_PBMP_WORD_SET(*ubmp, 0,
			  soc_VLAN_TABm_field32_get(unit, &vt, UT_BITMAPf));
    }

    if (ing_pbmp != NULL) {
        BCM_PBMP_ASSIGN(*ing_pbmp, local_pbmp);
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_draco_vlan_stg_get
 * Purpose:
 *	Read the STG number from a VLAN_TAB entry.
 */

int
bcm_draco_vlan_stg_get(int unit, bcm_vlan_t vid, bcm_stg_t *stg_ptr)
{
    vlan_tab_entry_t	vt;

    /* Upper layer already checks that vid is valid */

    BCM_IF_ERROR_RETURN(READ_VLAN_TABm(unit, MEM_BLOCK_ANY, (int) vid, &vt));

    if (!soc_VLAN_TABm_field32_get(unit, &vt, VALIDf)) {
	return BCM_E_NOT_FOUND;
    }

    *stg_ptr = soc_VLAN_TABm_field32_get(unit, &vt, STGf);

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_draco_vlan_stg_set
 * Purpose:
 *	Update the STG number for a VLAN_TAB entry.
 */

int
bcm_draco_vlan_stg_set(int unit, bcm_vlan_t vid, bcm_stg_t stg)
{
    vlan_tab_entry_t	vt;
    int			rv;

    /* Upper layer already checks that vid is valid */

    soc_mem_lock(unit, VLAN_TABm);

    rv = READ_VLAN_TABm(unit, MEM_BLOCK_ANY, (int) vid, &vt);

    if (BCM_SUCCESS(rv) && !soc_VLAN_TABm_field32_get(unit, &vt, VALIDf)) {
	rv = BCM_E_NOT_FOUND;
    }

    if (BCM_SUCCESS(rv)) {
        if (soc_feature(unit, soc_feature_mstp_uipmc)) {
	    bcm_pbmp_t pbmp;
 	    
 	    BCM_PBMP_CLEAR(pbmp);
 	    SOC_PBMP_WORD_SET(pbmp, 0, bcm_draco_vlan_port_bmp[unit][vid]);
 	    
 	    rv =  bcm_draco_vlan_pbmp_clear(unit, stg, &pbmp);
 	    if (!BCM_SUCCESS(rv)) {
	        soc_mem_unlock(unit, VLAN_TABm);
 		return(rv);
 	    }
 	    
 	    soc_VLAN_TABm_field32_set(unit, &vt, PORT_BITMAPf,
 				      SOC_PBMP_WORD_GET(pbmp, 0));
 	}
  
	soc_VLAN_TABm_field32_set(unit, &vt, STGf, stg);

	rv = WRITE_VLAN_TABm(unit, MEM_BLOCK_ALL, (int) vid, &vt);
    }

    soc_mem_unlock(unit, VLAN_TABm);

    if ( BCM_SUCCESS(rv) &&
	 soc_feature(unit, soc_feature_mstp_lookup)) {
        /* 
	 * For 5695_b+, MSTP_TBLm does a VLAN -> STG lookup.
	 * Copy the new STG value to MSTP_TBLm[vd->vlan_tag]. 
	 */
        mmu_memories1_mstp_tbl_entry_t mstp_entry;
	sal_memset(&mstp_entry, 0, sizeof(mstp_entry));
	soc_MMU_MEMORIES1_MSTP_TBLm_field32_set(unit, &mstp_entry, 
						MSTP_TBLf, stg);
	rv = WRITE_MMU_MEMORIES1_MSTP_TBLm(unit, 
					   MEM_BLOCK_ALL, 
					   vid, &mstp_entry);
    }

    return rv;
}

/*
 * Function:
 *      _bcm_d15_vlan_translate_delete
 * Purpose:
 *      Helper function to bcm_esw_vlan_translate_delete to delete vlan 
 *      translation for Draco1.5 family
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      port - port numebr can be wildcarded (port = -1)
 *      old_vid - Old VLAN ID to delete translation for can be wildcarded (old_vid = BCM_VLAN_NONE)
 *      xtable  - Memory table
 * Returns:
 *      BCM_E_NONE - Translation found, new_vid nad prio will have the values.
 *      BCM_E_NOT_FOUND - Translation does not exist
 *      BCM_E_XXX  - Other error
 * Notes:
 *      None.
 */
STATIC int 
_bcm_d15_vlan_translate_get_cb(int unit, void *trvs_info, int *stop)
{

    _bcm_vlan_translate_data_t      *get_info;
    _bcm_vlan_translate_traverse_t  *trvs_str;
    bcm_vlan_t                      old_vlan, new_vlan;
    bcm_port_t                      port;
    bcm_module_t                    modid;
    bcm_trunk_t                     trunk;
    int                             id;

    if ((NULL == trvs_info) || (NULL == stop)){
        return BCM_E_PARAM;
    }
    trvs_str = (_bcm_vlan_translate_traverse_t *)trvs_info;
    if (BCM_VLAN_INVALID != trvs_str->outer_vlan) {
        old_vlan = trvs_str->outer_vlan;
    } else {
        old_vlan = trvs_str->inner_vlan;
    }

    if (BCM_VLAN_INVALID != trvs_str->action->new_outer_vlan) {
        new_vlan = trvs_str->action->new_outer_vlan;
    } else {
        new_vlan = trvs_str->action->new_inner_vlan;
    }

    get_info = (_bcm_vlan_translate_data_t *)trvs_str->user_data;

    BCM_IF_ERROR_RETURN(
        _bcm_esw_gport_resolve(unit, trvs_str->gport, &modid, &port, 
                               &trunk, &id));

    if ((port == get_info->port) && (old_vlan == get_info->old_vlan)) {
        *(get_info->new_vlan) = new_vlan;
        *(get_info->prio) = trvs_str->action->priority;
        *stop = TRUE;
    } else {
        *stop = FALSE;
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_d15_vlan_translate_add
 * Purpose:
 *      Helper function to bcm_esw_vlan_translate_add to add vlan translation 
 *      for DRACO 1.5
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      port - port numebr
 *      old_vid - Old VLAN ID to has translation for
 *      new_vid - New VLAN ID that packet will get
 *      prio    - Priority
 * Returns:
 *      BCM_E_NONE - Translation found, new_vid nad prio will have the values.
 *      BCM_E_NOT_FOUND - Translation does not exist
 *      BCM_E_XXX  - Other error
 * Notes:
 *      None.
 */
int
_bcm_d15_vlan_translate_add(int unit, int port, bcm_vlan_t old_vid,
                            bcm_vlan_t new_vid, int prio)
{
    vlan_mac_entry_entry_t  vtent;
    int                     cng, vt_mode;

    bcm_esw_vlan_control_get(unit, bcmVlanTranslate, &vt_mode);
    if (!vt_mode) {
        return BCM_E_CONFIG; /* Not in VLAN translation mode */
    }

    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    }
    if (old_vid == BCM_VLAN_NONE) {
        return BCM_E_PARAM;
    }

    cng = 0;
    if (prio & BCM_PRIO_DROP_FIRST) {
        cng = 1;
        prio &= ~BCM_PRIO_DROP_FIRST;
    }
    if ((prio & ~BCM_PRIO_MASK) != 0) {
        return BCM_E_PARAM;
    }

    sal_memset(&vtent, 0, sizeof(vtent));

    soc_VLAN_MAC_ENTRYm_field32_set(unit, &vtent, SRC_PORTf, port);
    soc_VLAN_MAC_ENTRYm_field32_set(unit, &vtent, OLD_VLAN_IDf, old_vid);
    soc_VLAN_MAC_ENTRYm_field32_set(unit, &vtent, VLAN_IDf, new_vid);
    soc_VLAN_MAC_ENTRYm_field32_set(unit, &vtent, PRIf, prio);
    soc_VLAN_MAC_ENTRYm_field32_set(unit, &vtent, CNGf, cng);

    return soc_mem_insert(unit, VLAN_MAC_ENTRYm, MEM_BLOCK_ALL, &vtent);
}


/*
 * Function:
 *      _bcm_d15_vlan_translate_delete
 * Purpose:
 *      Helper function to bcm_esw_vlan_translate_delete to delete vlan 
 *      translation for DRACO 1.5 family
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      port - port numebr can be wildcarded (port = -1)
 *      old_vid - Old VLAN ID to delete translation for can be wildcarded (old_vid = BCM_VLAN_NONE)
 * Returns:
 *      BCM_E_NONE - Translation found, new_vid nad prio will have the values.
 *      BCM_E_NOT_FOUND - Translation does not exist
 *      BCM_E_XXX  - Other error
 * Notes:
 *      None.
 */
int
_bcm_d15_vlan_translate_delete(int unit, int port, bcm_vlan_t old_vid)
{
    uint32                  bucket, bvalid;
    vlan_mac_entry_entry_t  *vtent, *vtentp;
    vlan_mac_valid_entry_t  vvent;
    uint32                  ve_valid, ve_port, ve_vid;
    int                     i, rv, num_entries;
    int                     vt_mode;
    uint32                  imax, imin;

#define DRACO15_VMAC_BUCKET_ENTRIES     4

    /* Make sure the chip is configured for VLAN translation */
    bcm_esw_vlan_control_get(unit, bcmVlanTranslate, &vt_mode);
    if (!vt_mode) {
        return BCM_E_CONFIG; /* Not in VLAN translation mode */
    }

    /* Double wildcard, clear the table */
    if ((port < 0) && (old_vid == BCM_VLAN_NONE)) {
        return soc_mem_clear(unit, VLAN_MAC_VALIDm, MEM_BLOCK_ALL, 0);
    }

    if (port >= 0 && !SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    }

    /* Wildcard search and delete... */
    if ((port < 0) || (old_vid == BCM_VLAN_NONE)) {
        num_entries = soc_mem_index_count(unit, VLAN_MAC_ENTRYm);
        imin = soc_mem_index_min(unit, VLAN_MAC_ENTRYm);
        imax = soc_mem_index_max(unit, VLAN_MAC_ENTRYm);

        rv = BCM_E_MEMORY;
        vtent = soc_cm_salloc(unit,
                              num_entries * sizeof(*vtent),
                              "vlan_translation_entry");
        if (vtent == NULL) {
            return BCM_E_MEMORY;
        }

        soc_mem_lock(unit, VLAN_MAC_ENTRYm);
        rv = soc_mem_read_range(unit, VLAN_MAC_ENTRYm, MEM_BLOCK_ANY,
                                imin, imax, vtent);
        if (rv >= 0) {
            for (i = 0; i < num_entries; i++) {
                vtentp = soc_mem_table_idx_to_pointer(unit,
                                                      VLAN_MAC_ENTRYm,
                                                      vlan_mac_entry_entry_t *,
                                                      vtent, i);
                ve_valid = soc_VLAN_MAC_ENTRYm_field32_get(unit,
                                                           vtentp,
                                                           VALID_BITf);
                if (!ve_valid) {
                    continue;
                }
                if (port >= 0) {
                    ve_port = soc_VLAN_MAC_ENTRYm_field32_get(unit, vtentp,
                                                              SRC_PORTf);
                    if (ve_port != (uint32)port) {
                        continue;
                    }
                }
                if (old_vid != BCM_VLAN_NONE) {
                    ve_vid = soc_VLAN_MAC_ENTRYm_field32_get(unit, vtentp,
                                                             OLD_VLAN_IDf);
                    if (ve_vid != old_vid) {
                        continue;
                    }
                }

                /* matched an entry, delete it */
                bucket = i / DRACO15_VMAC_BUCKET_ENTRIES;
                rv = READ_VLAN_MAC_VALIDm(unit, MEM_BLOCK_ANY,
                                          bucket, &vvent);
                if (rv < 0) {
                    break;
                }
                bvalid = soc_VLAN_MAC_VALIDm_field32_get(unit, &vvent,
                                                         BUCKET_BITMAPf);
                bvalid &= ~(1<<(i % DRACO15_VMAC_BUCKET_ENTRIES));
                soc_VLAN_MAC_VALIDm_field32_set(unit, &vvent,
                                                BUCKET_BITMAPf, bvalid);
                rv = WRITE_VLAN_MAC_VALIDm(unit, MEM_BLOCK_ALL,
                                           bucket, &vvent);
                if (rv < 0) {
                    break;
                }
            } /* For each entry[i] */
        } /* Successful table read */
        soc_mem_unlock(unit, VLAN_MAC_ENTRYm);
        soc_cm_sfree(unit, vtent);
        return rv;
    } /* Wildcard search and delete */

    /* Find and delete a specific entry... */
    else {
        bcm_mac_t              key_data;
        vlan_mac_entry_entry_t vtent;

        sal_memset(&vtent, 0, sizeof(vtent));

        /* Overlaid fields; set as SRC_PORT and OLD_VLAN... */
        soc_VLAN_MAC_ENTRYm_field32_set(unit, &vtent, SRC_PORTf, port);
        soc_VLAN_MAC_ENTRYm_field32_set(unit, &vtent, OLD_VLAN_IDf,
                                        old_vid);
        /* ...extract as the 48-bit MAC_ADDR (key_data) */
        soc_VLAN_MAC_ENTRYm_mac_addr_get(unit, &vtent, MAC_ADDRf,
                                         key_data);

        return soc_mem_delete(unit, VLAN_MAC_ENTRYm, MEM_BLOCK_ALL,
                              key_data);
    } /* Find and delete specific entry */
}

/*
 * Function:
 *      _bcm_d15_vlan_translate_get
 * Purpose:
 *      Helper function to bcm_esw_vlan_translate_get to get vlan translation 
 *      for DRACO 1.5
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 *      port - port numebr
 *      old_vid - Old VLAN ID that has translation for
 *      new_vid - (OUT)New VLAN ID that packet will get
 *      prio    - (OUT)Priority
 * Returns:
 *      BCM_E_NONE - Translation found, new_vid nad prio will have the values.
 *      BCM_E_NOT_FOUND - Translation does not exist
 *      BCM_E_XXX  - Other error
 * Notes:
 *      None.
 */
int
_bcm_d15_vlan_translate_get(int unit, int port, bcm_vlan_t old_vid,
                            bcm_vlan_t *new_vid, int *prio)
{
    _bcm_vlan_translate_data_t          get_info;
    _bcm_vlan_translate_traverse_t      trvs_st;
    bcm_vlan_action_set_t               action;
    soc_mem_t                           mem;
    bcm_vlan_t                          new_vlan;
    int                                 priority, vt_mode;

    if (!SOC_PORT_ADDRESSABLE(unit, port)) {
        return BCM_E_PORT;
    }
    /* Make sure the chip is configured for VLAN translation */
    bcm_esw_vlan_control_get(unit, bcmVlanTranslate, &vt_mode);
    if (!vt_mode) {
        return BCM_E_CONFIG; /* Not in VLAN translation mode */
    }

    if ((NULL == new_vid) || (NULL == prio)){
        return BCM_E_PARAM;
    }
    
    sal_memset(&get_info, 0, sizeof(_bcm_vlan_translate_data_t));
    sal_memset(&trvs_st, 0, sizeof(_bcm_vlan_translate_traverse_t));
    sal_memset(&action, 0, sizeof(bcm_vlan_action_set_t));
    
    new_vlan = BCM_VLAN_INVALID;
    priority = -1;
    get_info.old_vlan = old_vid;
    get_info.port = port;
    get_info.new_vlan = &new_vlan;
    get_info.prio = &priority;

    trvs_st.user_data = (void *)&get_info;
    trvs_st.action = &action;
    trvs_st.user_cb_st = NULL;
    trvs_st.int_cb = _bcm_d15_vlan_translate_get_cb;

    mem = VLAN_MAC_ENTRYm;

    BCM_IF_ERROR_RETURN(
    _bcm_esw_vlan_translate_traverse_mem(unit, mem, &trvs_st));
    if ((BCM_VLAN_INVALID != new_vlan) && (-1 != priority) ){
        *new_vid = new_vlan;
        *prio = priority;
        return BCM_E_NONE;
    }

    return BCM_E_NOT_FOUND;
}

/*
 * Function   :
 *      _bcm_d15_vlan_translate_parse
 * Description   :
 *      Helper function for an API to parse a vlan translate 
 *      entry for Draco1.5 
 * Parameters   :
 *      unit            (IN) BCM unit number
 *      trvs_info       (IN/OUT) Traverse structure that contain all relevant info
 */
int 
_bcm_d15_vlan_translate_parse(int unit, soc_mem_t mem, uint32* vent,
                             _bcm_vlan_translate_traverse_t *trvs_info)
{
    int         vt_mode = 0;
    bcm_port_t  port;

    /* Make sure the chip is configured for VLAN translation */
    bcm_esw_vlan_control_get(unit, bcmVlanTranslate, &vt_mode);
    if (!vt_mode) {
        return BCM_E_CONFIG; /* Not in VLAN translation mode */
    }

    port =  soc_mem_field32_get(unit,mem , vent, SRC_PORTf);
    BCM_IF_ERROR_RETURN(
        bcm_esw_port_gport_get(unit, port, &(trvs_info->gport)));

    trvs_info->outer_vlan = soc_mem_field32_get(unit,mem , vent, OLD_VLAN_IDf);
    trvs_info->action->new_outer_vlan = soc_mem_field32_get(unit, mem, vent, 
                                                            VLAN_IDf);
    trvs_info->action->priority = soc_mem_field32_get(unit,mem , vent, 
                                                            PRIf);
    return (BCM_E_NONE);
}
