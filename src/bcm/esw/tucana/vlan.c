/*
 * $Id: vlan.c 1.13 Broadcom SDK $
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
 * Purpose:
 * Requires:    
 */

#include <sal/core/boot.h>

#include <soc/drv.h>
#include <soc/mem.h>

#include <bcm/error.h>
#include <bcm/vlan.h>
#include <bcm/stg.h>
#include <bcm/port.h>
#include <bcm/trunk.h>
#include <bcm/stack.h>

#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/tucana.h>

/*
 * Function:
 *	bcm_tucana_vlan_init
 * Purpose:
 *	Initialize the VLAN tables with the default entry in it.
 */

int
bcm_tucana_vlan_init(int unit, bcm_vlan_data_t *vd)
{
    vlan_tab_entry_t	ve;
    bcm_pbmp_t pbm;

    if (SAL_BOOT_QUICKTURN || SAL_BOOT_PLISIM) {
	soc_cm_print("SIMULATION: skipped VLAN table clear "
		     "(assuming hardware did it)\n");
    } else {
	SOC_IF_ERROR_RETURN
	    (soc_mem_clear(unit, VLAN_TABm, MEM_BLOCK_ALL, TRUE));
    }

    sal_memcpy(&ve, soc_mem_entry_null(unit, VLAN_TABm), sizeof (ve));

    BCM_PBMP_ASSIGN(pbm, vd->ut_port_bitmap);
    BCM_PBMP_AND(pbm, PBMP_E_ALL(unit));
    soc_VLAN_TABm_field32_set(unit, &ve, PORT_BITMAP_M0f,
			      SOC_PBMP_WORD_GET(vd->port_bitmap, 0));
    soc_VLAN_TABm_field32_set(unit, &ve, PORT_BITMAP_M1f,
			      SOC_PBMP_WORD_GET(vd->port_bitmap, 1));
    soc_VLAN_TABm_field32_set(unit, &ve, UT_BITMAP_M0f,
			      SOC_PBMP_WORD_GET(pbm, 0));
    soc_VLAN_TABm_field32_set(unit, &ve, UT_BITMAP_M1f,
			      SOC_PBMP_WORD_GET(pbm, 1));
    soc_VLAN_TABm_field32_set(unit, &ve, STGf, BCM_STG_DEFAULT);
    soc_VLAN_TABm_field32_set(unit, &ve, VALIDf, 1);

    SOC_IF_ERROR_RETURN
	(WRITE_VLAN_TABm(unit, MEM_BLOCK_ALL, vd->vlan_tag, &ve));

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_tucana_vlan_create
 * Purpose:
 *	Create a VLAN entry in VLAN_TAB
 */

int
bcm_tucana_vlan_create(int unit, bcm_vlan_t vid)
{
    vlan_tab_entry_t		vt;
    int				rv;

    /* Upper layer already checks that vid is valid */

    if (SOC_IS_RELOADING(unit)) {
        return BCM_E_NONE;	  
    }

    soc_mem_lock(unit, VLAN_TABm);

    if ((rv = READ_VLAN_TABm(unit, MEM_BLOCK_ANY, (int)vid, &vt)) < 0) {
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

    soc_VLAN_TABm_field32_set(unit, &vt, UT_BITMAP_M0f, 0);
    soc_VLAN_TABm_field32_set(unit, &vt, UT_BITMAP_M1f, 0);
    soc_VLAN_TABm_field32_set(unit, &vt, STGf, 0);
    soc_VLAN_TABm_field32_set(unit, &vt, VALIDf, 1);

    rv = WRITE_VLAN_TABm(unit, MEM_BLOCK_ALL, (int) vid, &vt);

    soc_mem_unlock(unit, VLAN_TABm);

    return rv;
}

/*
 * Function:
 *	bcm_tucana_vlan_port_add
 * Purpose:
 *	Add ports to the VLAN bitmap in a VLAN_TAB entry.
 */

int
bcm_tucana_vlan_port_add(int unit, bcm_vlan_t vid, pbmp_t pbmp, pbmp_t ubmp,
                         pbmp_t ing_pbmp)
{
    vlan_tab_entry_t		vt;
    uint32			bmval;
    int				rv;

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

    /*
     * Update the entry for this VLAN.
     */
    soc_VLAN_TABm_field_get(unit, &vt, PORT_BITMAP_M0f, &bmval);
    bmval |= SOC_PBMP_WORD_GET(pbmp, 0);
    soc_VLAN_TABm_field_set(unit, &vt, PORT_BITMAP_M0f, &bmval);
    soc_VLAN_TABm_field_get(unit, &vt, PORT_BITMAP_M1f, &bmval);
    bmval |= SOC_PBMP_WORD_GET(pbmp, 1);
    soc_VLAN_TABm_field_set(unit, &vt, PORT_BITMAP_M1f, &bmval);

    soc_VLAN_TABm_field_get(unit, &vt, UT_BITMAP_M0f, &bmval);
    bmval &= ~SOC_PBMP_WORD_GET(pbmp, 0);
    bmval |= SOC_PBMP_WORD_GET(ubmp, 0);
    soc_VLAN_TABm_field_set(unit, &vt, UT_BITMAP_M0f, &bmval);
    soc_VLAN_TABm_field_get(unit, &vt, UT_BITMAP_M1f, &bmval);
    bmval &= ~SOC_PBMP_WORD_GET(pbmp, 1);
    bmval |= SOC_PBMP_WORD_GET(ubmp, 1);
    soc_VLAN_TABm_field_set(unit, &vt, UT_BITMAP_M1f, &bmval);

    rv = WRITE_VLAN_TABm(unit, MEM_BLOCK_ALL, (int) vid, &vt);

    soc_mem_unlock(unit, VLAN_TABm);

    return rv;
}

/*
 * Function:
 *	bcm_tucana_vlan_port_remove
 * Purpose:
 *	Remove ports from the VLAN bitmap in a VLAN_TAB entry.
 */

int
bcm_tucana_vlan_port_remove(int unit, bcm_vlan_t vid, pbmp_t pbmp)
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

    soc_VLAN_TABm_field_get(unit, &vt, PORT_BITMAP_M0f, &bmval);
    bmval &= ~SOC_PBMP_WORD_GET(pbmp, 0);
    soc_VLAN_TABm_field_set(unit, &vt, PORT_BITMAP_M0f, &bmval);
    soc_VLAN_TABm_field_get(unit, &vt, PORT_BITMAP_M1f, &bmval);
    bmval &= ~SOC_PBMP_WORD_GET(pbmp, 1);
    soc_VLAN_TABm_field_set(unit, &vt, PORT_BITMAP_M1f, &bmval);

    soc_VLAN_TABm_field_get(unit, &vt, UT_BITMAP_M0f, &bmval);
    bmval &= ~SOC_PBMP_WORD_GET(pbmp, 0);
    soc_VLAN_TABm_field_set(unit, &vt, UT_BITMAP_M0f, &bmval);
    soc_VLAN_TABm_field_get(unit, &vt, UT_BITMAP_M1f, &bmval);
    bmval &= ~SOC_PBMP_WORD_GET(pbmp, 1);
    soc_VLAN_TABm_field_set(unit, &vt, UT_BITMAP_M1f, &bmval);

    rv = WRITE_VLAN_TABm(unit, MEM_BLOCK_ALL, (int) vid, &vt);

    soc_mem_unlock(unit, VLAN_TABm);

    return rv;
}

/*
 * Function:
 *	bcm_tucana_vlan_port_get
 * Purpose:
 *	Read the port bitmap from a VLAN_TAB entry.
 */

int
bcm_tucana_vlan_port_get(int unit, bcm_vlan_t vid, pbmp_t *pbmp, pbmp_t *ubmp,
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
                      soc_VLAN_TABm_field32_get(unit, &vt, PORT_BITMAP_M0f));
    SOC_PBMP_WORD_SET(local_pbmp, 1,
                      soc_VLAN_TABm_field32_get(unit, &vt, PORT_BITMAP_M1f));

    if (pbmp != NULL) {
        BCM_PBMP_ASSIGN(*pbmp, local_pbmp);
    }

    if (ubmp != NULL) {
	SOC_PBMP_CLEAR(*ubmp);
	SOC_PBMP_WORD_SET(*ubmp, 0, soc_VLAN_TABm_field32_get(unit,
                                  &vt, UT_BITMAP_M0f));
	SOC_PBMP_WORD_SET(*ubmp, 1, soc_VLAN_TABm_field32_get(unit,
                                  &vt, UT_BITMAP_M1f));
    }

    if (ing_pbmp != NULL) {
        BCM_PBMP_ASSIGN(*ing_pbmp, local_pbmp);
    }

    return BCM_E_NONE;
}
