/*
 * $Id: l23x.c 1.5 Broadcom SDK $
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
 * XGS L2 Table Diagnoistic Routines.
 */

#include <soc/drv.h>

#include <appl/diag/l23x.h>

#ifdef BCM_XGS_SWITCH_SUPPORT

/*
 * Function:
 *	diag_l2x_check_hash
 *
 * Purpose:
 *	Verify the claimed bucket matches the entry's hash.
 *
 * Returns:
 *	SOC_E_NONE on success, SOC_E_FAIL on error.
 *
 */

int
diag_l2x_check_hash(int unit, int hash_select, int expect_bucket, 
			l2x_entry_t *entry)
{
    int hash_bucket;

    hash_bucket = soc_l2x_software_hash(unit, hash_select, entry);
    if ( hash_bucket != expect_bucket ) {
	    return SOC_E_FAIL;
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *	diag_l2x_entry_verify
 *
 * Purpose:
 *	Verify the entry at stated L2X location is correctly placed.
 *
 * Returns:
 *	SOC_E_NONE on success, SOC_E_XXX on error.
 *
 */

int
diag_l2x_entry_verify(int unit, int hash_select, int index, 
		     l2x_entry_t *entry_p)
{
    int bucket = index / SOC_L2X_BUCKET_SIZE;
    int offset = index % SOC_L2X_BUCKET_SIZE;
    int mem_read;
    int hit, stat;
    int bit_field_val;

    SOC_IF_ERROR_RETURN(soc_mem_read(unit, L2Xm, 0, index, entry_p));

    if ( !soc_L2Xm_field32_get(unit, entry_p, VALID_BITf) ) {
	/* Valid bit unset, entry position OK */
	return SOC_E_NONE;
    } 

    /* First, are we in the right place? (less chip traffic) */
    SOC_IF_ERROR_RETURN(diag_l2x_check_hash(unit, hash_select, bucket, 
					   entry_p));

    /* Next, are we consistent across tables? */
    /* Checking valid bit */
    SOC_IF_ERROR_RETURN(soc_mem_read(unit, L2X_VALIDm, 0, bucket, &mem_read));
    if ( ! ((mem_read >> offset) & 0x1 ) ) {
	return SOC_E_FAIL;
    }
 
    /* Checking hit bit */
    SOC_IF_ERROR_RETURN(soc_mem_read(unit, L2X_HITm, 0, bucket, &mem_read));
    hit = (mem_read >> offset) & 0x1;
    bit_field_val = soc_L2Xm_field32_get(unit, entry_p, HIT_BITf);
    if ( hit != bit_field_val ) {
	return SOC_E_FAIL;
    }
    
    /* Checking static bit */
    SOC_IF_ERROR_RETURN(soc_mem_read(unit, L2X_STATICm, 0, bucket, &mem_read));
    stat = (mem_read >> offset) & 0x1;
    bit_field_val = soc_L2Xm_field32_get(unit, entry_p, STATIC_BITf);
    if ( stat != bit_field_val ) {
	return SOC_E_FAIL;
    }
    
    /* Looks like a valid entry */
    return SOC_E_NONE;
}

/*
 * Function:
 *	diag_l2x_verify
 *
 * Purpose:
 *	Verify the given range of L2X entries
 *
 * Returns:
 *	SOC_E_NONE on success, SOC_E_XXX on error.
 *
 */

int
diag_l2x_verify(int unit, int hash_select, int low_index, int high_index)
{
    int	index_min, index_max, ix, jx;
    l2x_entry_t bucket_entry[SOC_L2X_BUCKET_SIZE];

    index_min = soc_mem_index_min(unit, L2Xm);
    index_max = soc_mem_index_max(unit, L2Xm);

    if ( low_index < index_min ) {
	low_index = index_min;
    }

    if ( high_index > index_max ) {
	high_index = index_max;
    }

    /* Initialize entries to invalid*/
    memset(bucket_entry, 0, sizeof(l2x_entry_t) * SOC_L2X_BUCKET_SIZE);

    for (ix = low_index; ix <= high_index; ix++) {
	int offset = ix % SOC_L2X_BUCKET_SIZE;
	SOC_IF_ERROR_RETURN( diag_l2x_entry_verify(unit, hash_select, ix, 
			       &(bucket_entry[offset]) ));

	/* Check bucket array for duplicates */
	if ( soc_L2Xm_field32_get(unit, &(bucket_entry[offset]), VALID_BITf)) {
	    for (jx = 0; jx < offset; jx++ ) {
		if ( soc_L2Xm_field32_get(unit,  
					  &(bucket_entry[jx]), VALID_BITf)) {
		    if (soc_mem_compare_key(unit, L2Xm, 
                        &(bucket_entry[offset]), &(bucket_entry[jx])) == 0) {
			/* Found a match!  Bad. */
			return SOC_E_FAIL;
			
		    }
		}	    
	    }
	}
    }

    return SOC_E_NONE;
}

#ifdef INCLUDE_L3
/*
 * Function:
 *	diag_l3x_check_hash
 *
 * Purpose:
 *	Verify the claimed bucket matches the entry's hash.
 *
 * Returns:
 *	SOC_E_NONE on success, SOC_E_FAIL on error.
 *
 */

int
diag_l3x_check_hash(int unit, int hash_select,
		   int key_sip, int expect_bucket, l3x_entry_t *entry)
{
    int hash_bucket;
    int ipmc = soc_L3Xm_field32_get(unit, entry, IPMCf);

    hash_bucket = soc_l3x_software_hash(unit, hash_select, ipmc,
					   key_sip, entry);
    if ( hash_bucket != expect_bucket ) {
	    return SOC_E_FAIL;
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *	diag_l3x_entry_verify
 *
 * Purpose:
 *	Verify the entry at stated L3X location is correctly placed.
 *
 * Returns:
 *	SOC_E_NONE on success, SOC_E_XXX on error.
 *
 */

int
diag_l3x_entry_verify(int unit, int hash_select, int ipmc_enable,
		     int key_sip, int index, l3x_entry_t *entry_p)
{
    int bucket = index / SOC_L3X_BUCKET_SIZE(unit);
    int offset = index % SOC_L3X_BUCKET_SIZE(unit);
    int mem_read;
    int hit, ipmc;
    ip_addr_t src_ip;
    int bit_field_val;

    SOC_IF_ERROR_RETURN(soc_mem_read(unit, L3Xm, 0, index, entry_p));

    if (!soc_L3Xm_field32_get(unit, entry_p, L3_VALIDf)) {
	/* Valid bit unset, entry position OK */
	return SOC_E_NONE;
    } 

    SOC_IF_ERROR_RETURN
	(diag_l3x_check_hash(unit, hash_select,  key_sip, bucket, entry_p));

    /* Next, are we consistent across tables? */
    /* Checking valid bit */
    SOC_IF_ERROR_RETURN(soc_mem_read(unit, L3X_VALIDm, 0, bucket, &mem_read));
    if (!((mem_read >> offset) & 0x1)) {
	return SOC_E_FAIL;
    }
 
    /* Checking hit bit */
    SOC_IF_ERROR_RETURN(soc_mem_read(unit, L3X_HITm, 0, bucket, &mem_read));
    hit = (mem_read >> offset) & 0x1;
    bit_field_val = soc_L3Xm_field32_get(unit, entry_p, L3_HITf);
    if (hit != bit_field_val) {
	return SOC_E_FAIL;
    }
    
    ipmc = soc_L3Xm_field32_get(unit, entry_p, IPMCf);
    if (SOC_L3X_IP_MULTICAST(soc_L3Xm_field32_get(unit, entry_p, IP_ADDRf)) != 
	 ipmc) {
	return SOC_E_FAIL;
    }

    src_ip = soc_L3Xm_field32_get(unit, entry_p, SRC_IP_ADDRf);
    if (key_sip && ipmc && ipmc_enable && (src_ip != 0)) {
	return SOC_E_FAIL;
    }

    /* Looks like a valid entry */
    return SOC_E_NONE;
}

/*
 * Function:
 *	diag_l3x_verify
 *
 * Purpose:
 *	Verify the range of L3X entries
 *
 * Returns:
 *	SOC_E_NONE on success, SOC_E_XXX on error.
 *
 */

int
diag_l3x_verify(int unit, int hash_select, int ipmc_enable,
	       int key_sip, int low_index, int high_index)
{
    int	index_min, index_max, ix, jx;
    l3x_entry_t bucket_entry[SOC_L3X_MAX_BUCKET_SIZE];

    index_min = soc_mem_index_min(unit, L3X_VALIDm);
    index_max = soc_mem_index_max(unit, L3X_VALIDm);

    if (low_index < index_min) {
	low_index = index_min;
    }

    if (high_index > index_max) {
	high_index = index_max;
    }

    /* Initialize entries to invalid*/
    memset(bucket_entry, 0, sizeof(l3x_entry_t) * SOC_L3X_MAX_BUCKET_SIZE);

    SOC_IF_ERROR_RETURN(_soc_mem_cmp_l3x_sync(unit));

    for (ix = low_index; ix <= high_index; ix++) {
	int offset = ix % SOC_L3X_BUCKET_SIZE(unit);
	SOC_IF_ERROR_RETURN( diag_l3x_entry_verify(unit, hash_select, 
						  ipmc_enable, key_sip, ix, 
						  &(bucket_entry[offset]) ));

	/* Check bucket array for duplicates */
	if ( soc_L3Xm_field32_get(unit, &(bucket_entry[offset]), VALID_BITf)) {
	    for (jx = 0; jx < offset; jx++ ) {
		if ( soc_L3Xm_field32_get(unit,  
					  &(bucket_entry[jx]), VALID_BITf)) {
		    if (soc_mem_compare_key(unit, L3Xm, 
                        &(bucket_entry[offset]), &(bucket_entry[jx])) == 0) {
			/* Found a match!  Bad. */
			return SOC_E_FAIL;
			
		    }
		}	    
	    }
	}
   }

    return SOC_E_NONE;
}
#endif /* INCLUDE_L3 */


#endif /* BCM_XGS_SWITCH_SUPPORT */
