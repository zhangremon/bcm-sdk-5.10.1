/*
 * $Id: draco15.c 1.4 Broadcom SDK $
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
 * File:        draco15.c
 * Purpose:
 * Requires:    
 */


#include <sal/core/boot.h>

#include <soc/mem.h>
#include <soc/draco.h>
#include <soc/drv.h>
#include <soc/error.h>
#include <soc/hash.h>

#ifdef BCM_DRACO15_SUPPORT

/*
 * Draco chip driver functions.  
 */
soc_functions_t soc_draco15_drv_funs = {
    soc_draco_misc_init,
    soc_draco15_mmu_init,
    soc_draco_age_timer_get,
    soc_draco_age_timer_max_get,
    soc_draco_age_timer_set,
};

#define DRACO15_NUM_COS 8

int
soc_draco15_mmu_init(int unit)
{
    int		xq_per_port, cos, limit;
    soc_port_t	port;
    uint32	val;
    int         wgt[DRACO15_NUM_COS], tot_wgt = 0;
    int         inactive_cosqs = 0;

    /* 
     * Spread the XQs across all COSQs as dictated by the weight
     * properties. This will allow use of all COSQs. However, if 
     * some COSQs are later disabled, the XQs allocated here to 
     * those disabled COSQs will be unavailable for use.
     */
    for (cos = 0; cos < DRACO15_NUM_COS; cos++) {
        wgt[cos] =
            soc_property_suffix_num_get(unit, cos, spn_MMU_XQ_WEIGHT,
                                        "cos",  1);
	tot_wgt += wgt[cos];
	if (!wgt[cos]) {
	    inactive_cosqs++;
	}
    }

    /* Every inactive COSQ consumes 4 XQs; remove them from pool */
    xq_per_port = soc_mem_index_count(unit, MMU_MEMORIES2_XQ0m) -
      (4*inactive_cosqs);

    PBMP_ALL_ITER(unit, port) {
	for (cos = 0; cos < DRACO15_NUM_COS; cos++) {
	    /* 
	     * The HOLCOSPKTSETLIMITr register controls BOTH the XQ 
	     * size per cosq AND the HOL set limit for that cosq.
	     */
	    val = 0;
	    if (!wgt[cos]) {
	        limit = 4; /* Minimum value for this register */
	    }
	    else if (wgt[cos] == tot_wgt) {
	      limit = xq_per_port; /* All available XQs */ 
	    }
	    else {
	        limit = (xq_per_port * wgt[cos] / tot_wgt);
	    }
	    soc_reg_field_set(unit, HOLCOSPKTSETLIMITr, &val,
			      PKTSETLIMITf, 
			      limit);
	    SOC_IF_ERROR_RETURN
		(WRITE_HOLCOSPKTSETLIMITr(unit, port, cos, val));

	}
    }

    /*
     * MMU Port Enable.
     */
    val = 0;
    soc_reg_field_set(unit, MMUPORTENABLEr, &val, PORT_BITMAPf,
		      SOC_PBMP_WORD_GET(PBMP_ALL(unit), 0));
    SOC_IF_ERROR_RETURN(WRITE_MMUPORTENABLEr(unit, val));

    return SOC_E_NONE;
}

#define	DRACO15_VMAC_BUCKET_ENTRIES	4

/*
 * Function:
 *	soc_draco15_mac_hash
 * Purpose:
 *	Return hash bucket into which VLAN MAC entry belongs, according to HW
 * Parameters:
 *	unit - StrataSwitch unit #
 *	mac - MAC address to hash
 * Returns:
 *	If >= 0, hash bucket number
 *	If < 0, SOC_E_XXX
 */

int
soc_draco15_mac_hash(int unit, uint8 *mac)
{
    hashinput_entry_t		hent;
    uint32			key[SOC_MAX_MEM_WORDS];
    uint32			result;
    int                         hash_sel;

    sal_memset(&hent, 0, sizeof(hashinput_entry_t));
    sal_memset(key, 0, sizeof(key));
    SAL_MAC_ADDR_TO_UINT32(mac, key);

    soc_HASHINPUTm_field32_set(unit, &hent, KEY_TYPEf,
			       XGS_HASH_KEY_TYPE_L2);

    soc_HASHINPUTm_field_set(unit, &hent, KEYf, key);

    SOC_IF_ERROR_RETURN(soc_mem_write(unit, HASHINPUTm, MEM_BLOCK_ALL,
				      0, &hent));

    SOC_IF_ERROR_RETURN(READ_HASH_OUTPUTr(unit, &result));

    /*
     * For an XGS_HASH_KEY_TYPE_L2 hash, the hardware returns
     * an 11-bit result. We need to appropriately trim this
     * down to an 8-bit number for VLAN_MAC table operations.
     */

    soc_draco_hash_get(unit, &hash_sel);

    switch (hash_sel) {
    case XGS_HASH_CRC16_UPPER:
    case XGS_HASH_CRC32_UPPER:
	return result >> 3;
    case XGS_HASH_CRC16_LOWER:
    case XGS_HASH_CRC32_LOWER:
    case XGS_HASH_LSB:
    case XGS_HASH_ZERO:
	return result & 0xff;
    default:
	return SOC_E_PARAM;
    }
}

/*
 * Function:
 *	soc_draco15_mac_software_hash
 * Purpose:
 *	Return hash bucket into which VLAN MAC entry belongs, according to SW
 * Parameters:
 *	unit - StrataSwitch unit #
 *      hash_sel - hash type to use, or <0 to use current H/W hash type
 *	mac - MAC address to hash
 * Returns:
 *	If >= 0, hash bucket number
 *	If < 0, SOC_E_XXX
 */

int
soc_draco15_mac_software_hash(int unit, int hash_sel, uint8 *key)
{
    uint8 key60[XGS_HASH_KEY_SIZE];

    /* The 5695 actually does a 60-bit hash of 0x000:MAC_ADDR */

    if (hash_sel < 0) {
	soc_draco_hash_get(unit, &hash_sel);
    }

    sal_memset(key60, 0, sizeof(key60));

    soc_draco_l2x_param_to_key(key, 0, key60);

    switch (hash_sel) {
    case XGS_HASH_CRC16_UPPER:
	return soc_draco_crc16(key60, 8) >> 8;

    case XGS_HASH_CRC16_LOWER:
	return soc_draco_crc16(key60, 8) & 0xff;

    case XGS_HASH_LSB:
	return key[5];

    case XGS_HASH_ZERO:
	return 0;

    case XGS_HASH_CRC32_UPPER:
	return soc_draco_crc32(key60, 8) >> 24;

    case XGS_HASH_CRC32_LOWER:
	return soc_draco_crc32(key60, 8) & 0xff;

    default:
	return SOC_E_PARAM;
    }
}


/*
 * Function:
 *      soc_mem_draco15_vlan_mac_search
 * Purpose:
 *      Searches the Draco 1.5 VLAN_MAC table set for the indicated entry.
 * Parameters:
 *      unit                 Draco 1.5  PCI device unit number 
 *                           (driver internal).
 *      index_ptr            OUT:
 *                           If entry found gets the location of the entry.
 *                           If not found, gets table location where a
 *                           new entry could be written. If table (bucket) 
 *                           is already full, index_ptr gets -1.
 *                           Assumed to be non-NULL.
 *      entry_data    	     IN: Entry containing search (hash) key. 
 *                           OUT: If found, target entry data written here.
 *                           Assumed to be non-NULL.
 *                           Assumed to be a VLAN_MAC_ENTRYm table entry.
 * Returns:
 *	SOC_E_NOT_FOUND
 *			     Entry not found in table. The index_ptr
 *                           parameter qualifies this result (see above). 
 *	SOC_E_NONE	     Entry is found:  index_ptr gets location
 *	SOC_E_XXX	     If internal error occurs
 *
 * Notes:
 *   A search is performed for a matching table entry.
 *      The appropriate field(s) of entry_data are used for the key.
 *      All other fields in entry_data are ignored.
 *
 *   If found, the index is stored in index_ptr, and if entry_data is
 *      non-NULL, the contents of the found entry are written into it.
 *   If not found AND there is room for a new entry, the index of the 
 *      correct insertion point for the new entry is stored in index_ptr.
 *      If no room in the table, index_ptr gets -1.
 *   If a table read error occurs, SOC_E_XXX is returned.
 *   The VLAN_MAC_ENTRYm table does not have to be locked.
 */

int 
soc_mem_draco15_vlan_mac_search(int unit, int *index_ptr, void *entry_data)
{
    uint32			bucket, ebase;
    sal_mac_addr_t              hash_key, target_key;
    vlan_mac_entry_entry_t	*vment, *vmentp;
    int			        i, rv;

    /* The hash key for this table is always the 48-bit MAC_ADDRf,     */
    /* even if configured for VLAN Translation Mode (overlaid fields). */
    soc_VLAN_MAC_ENTRYm_mac_addr_get(unit, entry_data, MAC_ADDRf, 
				     hash_key);
    bucket = soc_draco15_mac_hash(unit, hash_key);

    vment = soc_cm_salloc(unit,
			  DRACO15_VMAC_BUCKET_ENTRIES * sizeof(*vment),
			  "vlan_translation_entry");
    if (vment == NULL) {
        return SOC_E_MEMORY;
    }
    
    ebase = bucket * DRACO15_VMAC_BUCKET_ENTRIES;
    rv = soc_mem_read_range(unit, VLAN_MAC_ENTRYm, MEM_BLOCK_ANY,
			    ebase, ebase + DRACO15_VMAC_BUCKET_ENTRIES - 1,
			    vment);
    if (rv >= 0) {
        rv = SOC_E_NOT_FOUND;
	*index_ptr = -1;
	for (i = 0; i < DRACO15_VMAC_BUCKET_ENTRIES; i++) {
            vmentp = soc_mem_table_idx_to_pointer(unit,
                         VLAN_MAC_ENTRYm, vlan_mac_entry_entry_t*, 
                         vment, i);
	    if (soc_VLAN_MAC_ENTRYm_field32_get(unit, vmentp,
						VALID_BITf)) {
	        soc_VLAN_MAC_ENTRYm_mac_addr_get(unit, vmentp, 
						 MAC_ADDRf, target_key);
		if (sal_memcmp(hash_key, target_key,
			       sizeof(hash_key)) == 0) { 
		    *index_ptr = ebase + i;
		    sal_memcpy(entry_data, vmentp, 
			       sizeof(vlan_mac_entry_entry_t));
		    rv = SOC_E_NONE;
		    break;
		}
	    }
	    else if ((*index_ptr) < 0) {
	        *index_ptr = ebase + i;
	    }
	}
    }
    soc_cm_sfree(unit, vment);
    return rv;
} /* end soc_mem_draco15_vlan_mac_search() */


/*
 * Function:
 *	soc_mem_draco15_vlan_mac_insert
 * Purpose:
 *	Insert an entry into the hashed Draco 1.5 VLAN_MAC table set.
 * Parameters:
 *      unit                 Draco 1.5  PCI device unit number 
 *                           (driver internal).
 *      entry_data           Pointer to VLAN_MAC_ENTRYm table entry.
 *                           Assumed to be non-NULL.
 * Returns:
 *	SOC_E_NONE - insertion succeeded
 *	SOC_E_FULL - table full
 *	SOC_E_XXX - other error
 * Notes:
 *	A search is made for the entry, and if a matching key is
 *	found, the entry is overwritten with new data, and SOC_E_NONE
 *      is returned.
 *	If key is not found, new entry is added to table set, or
 *	fails if the bucket is already full.
 * Note:
 *      Assumes VLAN_MAC_ENTRYm table is already locked
 */

int 
soc_mem_draco15_vlan_mac_insert(int unit, void *entry_data)
{
    vlan_mac_entry_entry_t	vment;
    vlan_mac_valid_entry_t	vvent;
    int                         rv, index = 0;
    uint32                      bucket, bvalid;

    /* Preserve the caller's entry data, in case of an existing entry */
    sal_memcpy(&vment, entry_data, sizeof(vlan_mac_entry_entry_t));

    rv = soc_mem_draco15_vlan_mac_search(unit, &index, &vment);

    if (index < 0)
        return SOC_E_FULL;
    else if ((rv < 0) && (rv != SOC_E_NOT_FOUND))
        return rv;

    /* Write the new entry, either to an empty entry or existing entry */
    SOC_IF_ERROR_RETURN(WRITE_VLAN_MACm(unit, MEM_BLOCK_ALL, index, 
					entry_data));

    /* Make sure the corresponding valid bit for the new entry is set */
    bucket = index / DRACO15_VMAC_BUCKET_ENTRIES;
    SOC_IF_ERROR_RETURN(READ_VLAN_MAC_VALIDm(unit, MEM_BLOCK_ANY,
					     bucket, &vvent));
    bvalid = soc_VLAN_MAC_VALIDm_field32_get(unit, &vvent, BUCKET_BITMAPf);
    bvalid |= (1<<(index % DRACO15_VMAC_BUCKET_ENTRIES));
    soc_VLAN_MAC_VALIDm_field32_set(unit, &vvent, BUCKET_BITMAPf, bvalid);
    SOC_IF_ERROR_RETURN(WRITE_VLAN_MAC_VALIDm(unit, MEM_BLOCK_ALL,
					      bucket, &vvent));
    return SOC_E_NONE;
} /* end soc_mem_draco15_vlan_mac_insert() */


/*
 * Function:
 *	soc_mem_draco15_vlan_mac_delete
 * Purpose:
 *	Delete an entry from the Draco 1.5 VLAN_MAC table set.
 * Parameters:
 *      unit                 Draco 1.5  PCI device unit number 
 *                           (driver internal).
 *      key_data             Pointer to 48-bit hash key_data.
 *                           Assumed to be non-NULL.
 * Returns:
 *	SOC_E_NONE on success (whether or not entry existed),
 *	SOC_E_XXX on read or write error.
 * Note:
 *      Assumes VLAN_MAC_ENTRYm table is already locked
 */

int 
soc_mem_draco15_vlan_mac_delete(int unit, void *key_data) 
{
    vlan_mac_entry_entry_t	vment;
    vlan_mac_valid_entry_t	vvent;
    int                         rv, index = 0;
    uint32                      bucket, bvalid;

    memset(&vment, 0, sizeof(vment));

    soc_VLAN_MAC_ENTRYm_mac_addr_set(unit, &vment, MAC_ADDRf, key_data);

    rv = soc_mem_draco15_vlan_mac_search(unit, &index, &vment);

    if ((index < 0) || (rv == SOC_E_NOT_FOUND)) 
        return SOC_E_NONE; /* No matching entry to delete */
    else if (rv < 0)
        return rv;

    bucket = index / DRACO15_VMAC_BUCKET_ENTRIES;

    SOC_IF_ERROR_RETURN(READ_VLAN_MAC_VALIDm(unit, MEM_BLOCK_ANY,
					     bucket, &vvent));
    bvalid = soc_VLAN_MAC_VALIDm_field32_get(unit, &vvent, BUCKET_BITMAPf);
    bvalid &= ~(1<<(index % DRACO15_VMAC_BUCKET_ENTRIES));
    soc_VLAN_MAC_VALIDm_field32_set(unit, &vvent, BUCKET_BITMAPf, bvalid);
    SOC_IF_ERROR_RETURN(WRITE_VLAN_MAC_VALIDm(unit, MEM_BLOCK_ALL,
					      bucket, &vvent));
    return SOC_E_NONE;
} /* end soc_mem_draco15_vlan_mac_delete() */

#endif /* BCM_DRACO15_SUPPORT */
