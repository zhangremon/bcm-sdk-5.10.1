/*
 * $Id: l2.c 1.88.2.1 Broadcom SDK $
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
 * File:        l2.c
 * Purpose:     BCM layer function driver
 */

#include <sal/core/libc.h>

#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/drv.h>
#include <soc/l2x.h>

#include <bcm/error.h>
#include <bcm/l2.h>

#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/stack.h>
#include <bcm_int/esw/draco.h>
#include <bcm_int/esw/l2.h>
#include <bcm_int/esw/mpls.h>

#include <bcm_int/esw_dispatch.h>

typedef struct _bcm_mac_block_info_s {
    bcm_pbmp_t mb_pbmp;
    int ref_count;
} _bcm_mac_block_info_t;

static _bcm_mac_block_info_t *_mbi_entries[BCM_MAX_NUM_UNITS];
#if defined(BCM_XGS12_SWITCH_SUPPORT)
static int _mbi_num[BCM_MAX_NUM_UNITS];
#endif /* BCM_XGS12_SWITCH_SUPPORT */

/*
 * Function:
 *	_bcm_l2_from_l2x
 * Purpose:
 *	Convert a Draco L2X entry to a hardware-independent L2 entry
 * Parameters:
 *	unit - Unit number
 *	l2addr - (OUT) Hardware-independent L2 entry
 *	l2x_entry - Draco L2X entry
 */

int
_bcm_l2_from_l2x(int unit, bcm_l2_addr_t *l2addr,
		 l2x_entry_t *l2x_entry)
{
    int mb_index; 

    sal_memset(l2addr, 0, sizeof (*l2addr));

    /* Valid bit is ignored here; entry is assumed valid */

    soc_L2Xm_mac_addr_get(unit, l2x_entry, MAC_ADDRf, l2addr->mac);

    l2addr->vid = soc_L2Xm_field32_get(unit, l2x_entry, VLAN_IDf);
    l2addr->cos_dst = soc_L2Xm_field32_get(unit, l2x_entry, COSf);
    l2addr->cos_src = soc_L2Xm_field32_get(unit, l2x_entry, COSf);

    if (soc_L2Xm_field32_get(unit, l2x_entry, CPUf)) {
	l2addr->flags |= BCM_L2_COPY_TO_CPU;
    }

    if (soc_L2Xm_field32_get(unit, l2x_entry, DST_DISCARDf)) {
	l2addr->flags |= BCM_L2_DISCARD_DST;
    }

    if (soc_L2Xm_field32_get(unit, l2x_entry, SRC_DISCARDf)) {
	l2addr->flags |= BCM_L2_DISCARD_SRC;
    }

    if (soc_L2Xm_field32_get(unit, l2x_entry, SCPf)) {
	l2addr->flags |= BCM_L2_COS_SRC_PRI;
    }

    if (soc_L2Xm_field32_get(unit, l2x_entry, L2MCf)) {
	l2addr->flags |= BCM_L2_MCAST;
	l2addr->l2mc_index =
	    soc_L2Xm_field32_get(unit, l2x_entry, L2MC_PTRf);
    } else {
    	uint32		            tgid_port;
        _bcm_gport_dest_t       dest;
        int                     isGport, rv;

        _bcm_gport_dest_t_init(&dest);
    	tgid_port = soc_L2Xm_field32_get(unit, l2x_entry, TGID_PORTf);
    
    	if (tgid_port & BCM_TGID_TRUNK_INDICATOR(unit)) {
    	    l2addr->flags |= BCM_L2_TRUNK_MEMBER;
    	    l2addr->tgid = tgid_port & BCM_TGID_PORT_TRUNK_MASK(unit);
            if (SOC_IS_DRACO15(unit)) {
                tgid_port = soc_L2Xm_field32_get(unit, l2x_entry, TGID_HIf);
                l2addr->tgid |= (tgid_port << 5);
            }
            bcm_esw_trunk_psc_get(unit, l2addr->tgid, &(l2addr->rtag));
            dest.tgid = l2addr->tgid;
            dest.gport_type = _SHR_GPORT_TYPE_TRUNK;
    
    	} else {
    	    bcm_module_t	mod_in, mod_out;
    	    bcm_port_t		port_in, port_out;
    
    	    mod_in = soc_L2Xm_field32_get(unit, l2x_entry, MODULE_IDf);
    	    port_in = tgid_port;
    	    BCM_IF_ERROR_RETURN
    		(_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_GET,
                                        mod_in, port_in,
                                        &mod_out, &port_out));
            l2addr->modid = mod_out;
    	    l2addr->port = port_out;

            dest.port = l2addr->port;
            dest.modid = l2addr->modid;
            dest.gport_type = _SHR_GPORT_TYPE_MODPORT;
    	}
        rv = bcm_esw_switch_control_get(unit, bcmSwitchUseGport, &isGport);

        if (BCM_SUCCESS(rv) && isGport) {
            BCM_IF_ERROR_RETURN(
                _bcm_esw_gport_construct(unit, &dest, &(l2addr->port)));
        }
    }

    if (soc_L2Xm_field32_get(unit, l2x_entry, L3f)) {
	l2addr->flags |= BCM_L2_L3LOOKUP;
    }

    mb_index = soc_L2Xm_field32_get(unit, l2x_entry, MAC_BLOCK_INDEXf);
    if (mb_index) {
        BCM_PBMP_ASSIGN(l2addr->block_bitmap,
                        _mbi_entries[unit][mb_index].mb_pbmp);
    }


    if (soc_L2Xm_field32_get(unit, l2x_entry, STATIC_BITf)) {
	l2addr->flags |= BCM_L2_STATIC;
    }

    if (soc_L2Xm_field32_get(unit, l2x_entry, HIT_BITf)) {
	l2addr->flags |= BCM_L2_HIT;
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *	_bcm_l2_to_l2x
 * Purpose:
 *	Convert a hardware-independent L2 entry to a Draco L2X entry
 * Parameters:
 *	unit - Unit number
 *	l2x_entry - (OUT) Draco L2X entry
 *	l2addr - Hardware-independent L2 entry
 */

int
_bcm_l2_to_l2x(int unit, l2x_entry_t *l2x_entry,
	       bcm_l2_addr_t *l2addr)
{
    VLAN_CHK_ID(unit, l2addr->vid);
    VLAN_CHK_PRIO(unit, l2addr->cos_dst);
    if (l2addr->flags & BCM_L2_PENDING) {
        return BCM_E_PARAM;
    }

    sal_memset(l2x_entry, 0, sizeof (*l2x_entry));
    soc_L2Xm_field32_set(unit, l2x_entry, VALID_BITf, 1);
    soc_L2Xm_mac_addr_set(unit, l2x_entry, MAC_ADDRf, l2addr->mac);
    soc_L2Xm_field32_set(unit, l2x_entry, VLAN_IDf, l2addr->vid);
    soc_L2Xm_field32_set(unit, l2x_entry, COSf, l2addr->cos_dst);

    if (l2addr->flags & BCM_L2_COPY_TO_CPU) {
        soc_L2Xm_field32_set(unit, l2x_entry, CPUf, 1);
    }

    if (l2addr->flags & BCM_L2_DISCARD_DST) {
        soc_L2Xm_field32_set(unit, l2x_entry, DST_DISCARDf, 1);
    }

    if (l2addr->flags & BCM_L2_DISCARD_SRC) {
        soc_L2Xm_field32_set(unit, l2x_entry, SRC_DISCARDf, 1);
    }

    if (l2addr->flags & BCM_L2_COS_SRC_PRI) {
        soc_L2Xm_field32_set(unit, l2x_entry, SCPf, 1);
    }

    if (l2addr->flags & BCM_L2_MCAST) {
        soc_L2Xm_field32_set(unit, l2x_entry, L2MCf, 1);
        soc_L2Xm_field32_set(unit, l2x_entry, L2MC_PTRf,
			     l2addr->l2mc_index);
    } else {
        bcm_port_t      port = -1;
        bcm_trunk_t     tgid = BCM_TRUNK_INVALID;
        bcm_module_t    modid = -1;

        if (BCM_GPORT_IS_SET(l2addr->port)) {
            _bcm_l2_gport_params_t  g_params;

            if (BCM_GPORT_IS_BLACK_HOLE(l2addr->port)) {
                soc_L2Xm_field32_set(unit, l2x_entry, SRC_DISCARDf, 1);
            } else {
                if (!(l2addr->flags & BCM_L2_DISCARD_SRC)) {
                    soc_L2Xm_field32_set(unit, l2x_entry, SRC_DISCARDf, 0);
                }

                BCM_IF_ERROR_RETURN(
                    _bcm_esw_l2_gport_parse(unit, l2addr, &g_params));
    
                switch (g_params.type) {
                    case _SHR_GPORT_TYPE_TRUNK: 
                        tgid = g_params.param0;
                        break;
                    case  _SHR_GPORT_TYPE_MODPORT:
                        port = g_params.param0;
                        modid = g_params.param1;
                        break;
                    case _SHR_GPORT_TYPE_LOCAL_CPU:
                        port = g_params.param0;
                        BCM_IF_ERROR_RETURN(
                            bcm_esw_stk_my_modid_get(unit, &modid));
                        break;
                    default:
                        return BCM_E_PORT;
                }
            }
        } else if (l2addr->flags & BCM_L2_TRUNK_MEMBER) {
            tgid = l2addr->tgid;

        } else {
            BCM_IF_ERROR_RETURN(
                _bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_SET,
                                       l2addr->modid, l2addr->port, 
                                       &modid, &port));
            /* Check parameters */
            if (!SOC_MODID_ADDRESSABLE(unit, modid)) {
                return BCM_E_BADID;
            }
            if (!SOC_PORT_ADDRESSABLE(unit, port)) {
                return BCM_E_PORT;
            }
        }

        /* Setting l2x_entry fields according to parameters */
        if ( BCM_TRUNK_INVALID != tgid) {
            if (SOC_IS_DRACO15(unit)) {
                soc_L2Xm_field32_set(unit, l2x_entry, Tf, 1);
                soc_L2Xm_field32_set(unit, l2x_entry, TGID_LOf,
                                     tgid & BCM_TGID_PORT_TRUNK_MASK(unit));
                soc_L2Xm_field32_set(unit, l2x_entry, TGID_HIf,
                                     (tgid & BCM_TGID_PORT_TRUNK_MASK_HI(unit)) >> 5);
            } else {
                soc_L2Xm_field32_set(unit, l2x_entry, TGID_PORTf,
                                     tgid | BCM_TGID_TRUNK_INDICATOR(unit));
            }
            /*
             * Note:  RTAG is ignored here.  Use bcm_trunk_psc_set to
             * to set for a given trunk.
             */
        } else if (-1 != port) {
            soc_L2Xm_field32_set(unit, l2x_entry, MODULE_IDf, modid);
            soc_L2Xm_field32_set(unit, l2x_entry, TGID_PORTf, port);
        }
    }
    if (l2addr->flags & BCM_L2_L3LOOKUP) {
        soc_L2Xm_field32_set(unit, l2x_entry, L3f, 1);
    }

    /* MAC_BLOCK_INDEXf is handled in the add/remove functions below */

    if (l2addr->flags & BCM_L2_STATIC) {
        soc_L2Xm_field32_set(unit, l2x_entry, STATIC_BITf, 1);
    }

    if (l2addr->flags & BCM_L2_HIT) {
        soc_L2Xm_field32_set(unit, l2x_entry, HIT_BITf, 1);
    }

    return BCM_E_NONE;
}

#if defined(BCM_XGS12_SWITCH_SUPPORT)
/*
 * Function:
 *	_bcm_mac_block_insert
 * Purpose:
 *	Find or create a MAC_BLOCK table entry matching the given bitmap.
 * Parameters:
 *	unit - Unit number
 *	mb_pbmp - egress port bitmap for source MAC blocking
 *	mb_index - (OUT) Index of MAC_BLOCK table with bitmap.
 * Returns:
 *	BCM_E_NONE		Success
 *	BCM_E_INTERNAL		Chip access failure
 *	BCM_E_RESOURCE		No more MAC_BLOCK entries available
 *	BCM_E_PARAM		Bad bitmap supplied
 */
STATIC int
_bcm_mac_block_insert(int unit, bcm_pbmp_t mb_pbmp, int *mb_index)
{
    int cur_index = 0;
    _bcm_mac_block_info_t *mbi = _mbi_entries[unit];
    mac_block_entry_t mbe;
    bcm_pbmp_t temp_pbmp;

    /* Check for reasonable pbmp */
    BCM_PBMP_ASSIGN(temp_pbmp, mb_pbmp);
    BCM_PBMP_AND(temp_pbmp, PBMP_ALL(unit));
    if (BCM_PBMP_NEQ(mb_pbmp, temp_pbmp)) {
        return BCM_E_PARAM;
    }

    for (cur_index = 0; cur_index < _mbi_num[unit]; cur_index++) {
        if (BCM_PBMP_EQ(mbi[cur_index].mb_pbmp, mb_pbmp)) {
            mbi[cur_index].ref_count++;
            *mb_index = cur_index;
            return BCM_E_NONE;
        }
    }

    /* Not in table already, see if any space free */
    for (cur_index = 1; cur_index < _mbi_num[unit]; cur_index++) {
        if (mbi[cur_index].ref_count == 0) {
            /* Attempt insert */
            sal_memset(&mbe, 0, sizeof(mac_block_entry_t));
            if (SOC_IS_TUCANA(unit)) {
                soc_MAC_BLOCKm_field32_set(unit, &mbe, MAC_BLOCK_MASK_M0f,
                                           SOC_PBMP_WORD_GET(mb_pbmp, 0));
                soc_MAC_BLOCKm_field32_set(unit, &mbe, MAC_BLOCK_MASK_M1f,
                                           SOC_PBMP_WORD_GET(mb_pbmp, 1));
            } else {
                soc_MAC_BLOCKm_field32_set(unit, &mbe, MAC_BLOCK_MASKf,
                                           SOC_PBMP_WORD_GET(mb_pbmp, 0));
            }
            SOC_IF_ERROR_RETURN(WRITE_MAC_BLOCKm(unit, MEM_BLOCK_ALL,
                                                 cur_index, &mbe));
            mbi[cur_index].ref_count++;
            BCM_PBMP_ASSIGN(mbi[cur_index].mb_pbmp, mb_pbmp);
            *mb_index = cur_index;
            return BCM_E_NONE;
        }
    }

    /* Didn't find a free slot, out of table space */
    return BCM_E_RESOURCE;
}

/*
 * Function:
 *	_bcm_mac_block_delete
 * Purpose:
 *	Remove reference to MAC_BLOCK table entry matching the given bitmap.
 * Parameters:
 *	unit - Unit number
 *	mb_index - Index of MAC_BLOCK table with bitmap.
 */
STATIC void
_bcm_mac_block_delete(int unit, int mb_index)
{
    if (_mbi_entries[unit][mb_index].ref_count > 0) {
        _mbi_entries[unit][mb_index].ref_count--;
    } else if (mb_index) {
        
        /* Someone reran init without flushing the L2 table */
    } /* else mb_index = 0, as expected for learning */
}

/*
 * Function:
 *	bcm_draco_l2_addr_add
 * Description:
 *	Add a MAC address to the Switch Address Resolution Logic (ARL)
 *	port with the given VLAN ID and parameters.
 * Parameters:
 *	unit - StrataSwitch PCI device unit number (driver internal).
 *	l2addr - Pointer to bcm_l2_addr_t containing all valid fields
 * Returns:
 *	BCM_E_NONE		Success
 *	BCM_E_INTERNAL		Chip access failure
 *	BCM_E_RESOURCE		No MAC_BLOCK entries available
 * Notes:
 *	Use CMIC_PORT(unit) to associate the entry with the CPU.
 *	Use flag of BCM_L2_COPY_TO_CPU to send a copy to the CPU.
 *      Use flag of BCM_L2_TRUNK_MEMBER to set trunking (TGID must be
 *      passed as well with non-zero trunk group ID)
 */

int
bcm_draco_l2_addr_add(int unit, bcm_l2_addr_t *l2addr)
{
    l2x_entry_t		l2x_entry;
    int                 rv;
    l2x_entry_t		l2x_lookup;
    int                 l2_index, mb_index = 0;

    BCM_IF_ERROR_RETURN(_bcm_l2_to_l2x(unit, &l2x_entry, l2addr));
    
    rv = soc_l2x_lookup(unit, &l2x_entry, &l2x_lookup, &l2_index);
    if (rv < 0) {
         if (rv != BCM_E_NOT_FOUND) {
             return rv;
         }
    } else {
        /* Only here if found */
        mb_index = soc_L2Xm_field32_get(unit, &l2x_lookup, MAC_BLOCK_INDEXf);
        _bcm_mac_block_delete(unit, mb_index);
    }

    /* Mac blocking, attempt to associate with bitmap entry */
    BCM_IF_ERROR_RETURN(_bcm_mac_block_insert(unit, l2addr->block_bitmap,
                                              &mb_index));

    soc_L2Xm_field32_set(unit, &l2x_entry, MAC_BLOCK_INDEXf, mb_index);

    rv = soc_l2x_insert(unit, &l2x_entry);

    if ((rv == BCM_E_FULL) && (l2addr->flags & BCM_L2_REPLACE_DYNAMIC)) {
        l2x_valid_entry_t  vbits;
        l2x_static_entry_t sbits;
        l2x_hit_entry_t    hbits;
        l2x_entry_t        l2ent;
        uint8              key[XGS_HASH_KEY_SIZE];
        int                hash_select, bucket, slot;
        int                cf_hit, cf_unhit;

        if ((rv = soc_draco_hash_get(unit, &hash_select) < 0)) {
            goto done;
        }

        soc_draco_l2x_param_to_key(l2addr->mac, l2addr->vid, key);
        bucket = soc_draco_l2_hash(unit, hash_select, key);

        if ((rv = soc_l2x_freeze(unit)) < 0) {
            goto done;
        }

        if ((rv = soc_mem_read(unit, L2X_VALIDm, MEM_BLOCK_ANY,
                               bucket, &vbits) < 0)) {
            (void) soc_l2x_thaw(unit);
            goto done;
        }

        cf_hit = cf_unhit = -1;
        for (slot = 0; slot < SOC_L2X_BUCKET_SIZE; slot++) {
            if (vbits.entry_data[0] & (1 << slot)) {
                if ((rv = soc_mem_read(unit, L2Xm, MEM_BLOCK_ANY,
                                  bucket * SOC_L2X_BUCKET_SIZE + slot,
                                  &l2ent) < 0)) {
                    (void) soc_l2x_thaw(unit);
                    goto done;
                }
                if ((soc_L2Xm_field32_get(unit, &l2ent, STATIC_BITf)) ||
                    (soc_L2Xm_field32_get(unit, &l2ent, L2MCf)) ||
                    (soc_L2Xm_field32_get(unit, &l2ent, L3f))) {
                    continue;
                }
                if (soc_L2Xm_field32_get(unit, &l2ent, HIT_BITf)) {
                    cf_hit =  slot;
                } else {
                    cf_unhit = slot;
                }
            }
        }

        if (cf_unhit >= 0) {
            slot = cf_unhit;   /* take last unhit dynamic */
        } else if (cf_hit >= 0) {
            slot = cf_hit;     /* or last hit dynamic */
        } else {
          rv = BCM_E_FULL;     /* no dynamics to delete */
          (void) soc_l2x_thaw(unit);
          goto done;
        }

        if ((rv = soc_mem_write(unit, L2X_BASEm, MEM_BLOCK_ALL,
                       bucket * SOC_L2X_BUCKET_SIZE + slot, &l2x_entry)) < 0) {
            (void) soc_l2x_thaw(unit);
            goto done;
        }
        if (l2addr->flags & BCM_L2_STATIC) {
            if ((rv = soc_mem_read(unit, L2X_STATICm, MEM_BLOCK_ANY,
                                               bucket, &sbits)) < 0) {
                (void) soc_l2x_thaw(unit);
                goto done;
            }
            sbits.entry_data[0] |= (1 << slot);
            if ((rv = soc_mem_write(unit, L2X_STATICm, MEM_BLOCK_ALL,
                                                bucket, &sbits)) < 0) {
                (void) soc_l2x_thaw(unit);
                goto done;
            }
        }
        if (l2addr->flags & BCM_L2_HIT) {
            if ((rv = soc_mem_read(unit, L2X_HITm, MEM_BLOCK_ANY,
                                            bucket, &hbits)) < 0) {
                (void) soc_l2x_thaw(unit);
                goto done;
            }
            hbits.entry_data[0] |= (1 << slot);
            if ((rv = soc_mem_write(unit, L2X_HITm, MEM_BLOCK_ALL,
                                             bucket, &hbits)) < 0) {
                (void) soc_l2x_thaw(unit);
                goto done;
            }
        }
        rv = soc_l2x_thaw(unit);
    }

done:
    if (rv < 0) {
        _bcm_mac_block_delete(unit, mb_index);
    }

    return rv;
}

/*
 * Function:
 *	bcm_draco_l2_addr_delete
 * Description:
 *	Delete an L2 address (MAC+VLAN) from the device
 * Parameters:
 *	unit - device unit
 *	mac  - MAC address to delete
 *	vid  - VLAN id
 * Returns:
 *	BCM_E_XXX
 */
int
bcm_draco_l2_addr_delete(int unit, bcm_mac_t mac, bcm_vlan_t vid)
{
    bcm_l2_addr_t	l2addr;
    l2x_entry_t		l2x_entry, l2x_lookup;
    int                 l2_index, mb_index, rv;
    soc_control_t	*soc = SOC_CONTROL(unit);

    bcm_l2_addr_t_init(&l2addr, mac, vid);

    BCM_IF_ERROR_RETURN(_bcm_l2_to_l2x(unit, &l2x_entry, &l2addr));

    /* Lock L2X table before using SOC_L2_DEL_SYNC_LOCK */
    soc_mem_lock(unit, L2Xm);   
    rv = soc_l2x_lookup(unit, &l2x_entry, &l2x_lookup, &l2_index);
    if (BCM_E_NONE != rv) {
        soc_mem_unlock(unit, L2Xm);
        return rv;
    }
    mb_index = soc_L2Xm_field32_get(unit, &l2x_lookup, MAC_BLOCK_INDEXf);
    _bcm_mac_block_delete(unit, mb_index);

    if (SOC_L2_DEL_SYNC_LOCK(soc) < 0) {
        soc_mem_unlock(unit, L2Xm);
        return BCM_E_RESOURCE;
    }
    if ((rv = soc_l2x_delete(unit, &l2x_entry)) >= 0) {
        rv = soc_l2x_sync_delete(unit, (uint32 *) &l2x_lookup, l2_index, 0);
    }
    SOC_L2_DEL_SYNC_UNLOCK(soc);
    soc_mem_unlock(unit, L2Xm);
    return(rv);
}
#endif /* BCM_XGS12_SWITCH_SUPPORT */

#define DEFAULT_L2DELETE_CHUNKS		64	/* 16k entries / 64 = 256 */

/*
 * Function:
 *     _bcm_draco_age_reg_config
 * Description:
 *     Helper function to _bcm_fb_l2_addr_replace_by_XXX functions to 
 *     configure PER_PORT_AGE_CONTROL register for Draco
 * Parameters:
 *     unit         device number
 *     flags        flags BCM_L2_REPLACE_* 
 *     rep_st       structure with information of what to replace 
 * Return:
 *     BCM_E_XXX
 */
STATIC int 
_bcm_draco_age_reg_config(int unit, uint32 flags, _bcm_l2_replace_t *rep_st)
{
    uint32          age_val, op, match_mod, match_port;
    uint32          soc_flags, sync_op;
    soc_control_t   *soc = SOC_CONTROL(unit);
    int             rv;

    age_val = 0;
    op = 0;
    sync_op = 0;

    BCM_IF_ERROR_RETURN(
        _bcm_get_op_from_flags(flags, &op, &sync_op));

    if (rep_st->match_dest.trunk != -1) {
        match_mod = BCM_TRUNK_TO_MODIDf(unit, rep_st->match_dest.trunk);
        match_port = BCM_TRUNK_TO_TGIDf(unit, rep_st->match_dest.trunk);
        if (sync_op == SOC_L2X_PORTMOD_DEL) {
            sync_op = SOC_L2X_TRUNK_DEL;
        }
    } else {
        match_mod = rep_st->match_dest.module;
        match_port = rep_st->match_dest.port;
    }

    if (soc_reg_field_valid(unit, PER_PORT_AGE_CONTROLr, VID_OR_PORTf)) {
        soc_reg_field_set(unit, PER_PORT_AGE_CONTROLr, &age_val, 
                          VID_OR_PORTf, 
                          (flags & BCM_L2_REPLACE_MATCH_VLAN) ? 1 : 0);
    } else {
        soc_reg_field_set(unit, PER_PORT_AGE_CONTROLr, &age_val,
                  EXCL_STATICf,
                  (flags & BCM_L2_REPLACE_MATCH_STATIC) ? 0 : 1);
        soc_reg_field_set(unit, PER_PORT_AGE_CONTROLr, &age_val, 
                          PPA_MODEf, op);
    }
    soc_reg_field_set(unit, PER_PORT_AGE_CONTROLr, &age_val, 
                      VLAN_IDf, rep_st->key_vlan);
    soc_reg_field_set(unit, PER_PORT_AGE_CONTROLr, &age_val,
                      MODULE_IDf, match_mod);
    soc_reg_field_set(unit, PER_PORT_AGE_CONTROLr, &age_val,
                      TGID_PORTf, match_port);
    BCM_IF_ERROR_RETURN(WRITE_PER_PORT_AGE_CONTROLr(unit, age_val));

    /* OK not to lock L2Xm before since it is locked by upper layer */
    if (SOC_L2_DEL_SYNC_LOCK(soc) < 0) {
        return BCM_E_RESOURCE;
    }
    rv = soc_l2x_port_age(unit, PER_PORT_AGE_CONTROLr, INVALIDr);
    if (BCM_FAILURE(rv)) {
        SOC_L2_DEL_SYNC_UNLOCK(soc);
        return rv;
    }
    if ((flags & BCM_L2_REPLACE_DELETE)) {
        soc_flags = (flags & BCM_L2_REPLACE_MATCH_STATIC) ? SOC_L2X_INC_STATIC : 0;
        if (flags & BCM_L2_REPLACE_NO_CALLBACKS) {
            soc_flags |= SOC_L2X_NO_CALLBACKS;
        }
        rv = _soc_l2x_sync_delete_by(unit, rep_st->match_dest.module,
                                     rep_st->match_dest.port,
                                     rep_st->key_vlan,
                                     rep_st->match_dest.trunk, 0,
                                     soc_flags, sync_op);
    }
    SOC_L2_DEL_SYNC_UNLOCK(soc);
    return rv;
}

/*
 * Function:
 *     _bcm_draco_repl_reg_config
 * Description:
 *     Helper function to _bcm_fb_l2_addr_replace_by_XXX functions to 
 *     configure PER_PORT_REPL_CONTROL register for Draco
 * Parameters:
 *     unit         device number
 *     rep_st       structure with information of what to replace 
 * Return:
 *     BCM_E_XXX
 */
STATIC int 
_bcm_draco_repl_reg_config(int unit, _bcm_l2_replace_t *rep_st)
{
    uint32 repl_val = 0, mod_val, port_val;

    if (rep_st->new_dest.trunk != -1) {
        mod_val = BCM_TRUNK_TO_MODIDf(unit, rep_st->new_dest.trunk);
        port_val = BCM_TRUNK_TO_TGIDf(unit, rep_st->new_dest.trunk);
    } else {
        mod_val = rep_st->new_dest.module;
        port_val = rep_st->new_dest.port;
    }
    soc_reg_field_set(unit, PER_PORT_REPL_CONTROLr, &repl_val,
                      MODULE_IDf, mod_val);
    soc_reg_field_set(unit, PER_PORT_REPL_CONTROLr, &repl_val,
                      TGID_PORTf, port_val);
    SOC_IF_ERROR_RETURN(WRITE_PER_PORT_REPL_CONTROLr(unit, repl_val));

    return BCM_E_NONE;
}

/*
 * Functions:
 *      _bcm_draco_l2_delete_vlan_dest
 * Description:
 *      Helper function to _bcm_draco_l2_addr_replace_by_vlan_dest to
 *      delete all non-static entries via DMA 
 * Parameters:
 *     unit         device number
 *     flags        flag
 *     rep_st       structure with info of what to match 
 * Return:
 *     BCM_E_XXX
 */
STATIC int
_bcm_draco_l2_delete_vlan_dest(int unit, uint32 flags, _bcm_l2_replace_t *rep_st)
{
    int           ix, rv;
    int           chunksize, nchunks, chunk;
    l2x_entry_t   *l2xe, *l2xep;
    soc_control_t *soc = SOC_CONTROL(unit);
    int           valid_bit = SOC_IS_FBX(unit)? VALIDf : VALID_BITf;
    int           port_tgid;
    int           dest_match = (flags & BCM_L2_REPLACE_MATCH_DEST);
    int           vlan_match = (flags & BCM_L2_REPLACE_MATCH_VLAN);
    int           dyn_only = (flags & BCM_L2_REPLACE_MATCH_STATIC) ? 0 : 1;
    uint32        soc_flags = (flags & BCM_L2_REPLACE_NO_CALLBACKS) ? 
                                SOC_L2X_NO_CALLBACKS : 0;

    if (vlan_match == 0 && dest_match == 0) {
        return BCM_E_PARAM;
    } 

    chunksize = soc_property_get(unit, spn_L2DELETE_CHUNKS,
                                 DEFAULT_L2DELETE_CHUNKS);

    l2xe = soc_cm_salloc(unit, chunksize * sizeof(*l2xe), "l2del_chunk");
    if (l2xe == NULL) {
        return BCM_E_MEMORY;
    }

    nchunks = soc_mem_index_count(unit, L2Xm) / chunksize;
    rv = BCM_E_NONE;

    soc_mem_lock(unit, L2Xm);
    for (chunk = 0; chunk < nchunks; chunk++) {
        rv = soc_mem_read_range(unit, L2Xm, MEM_BLOCK_ANY,
                                chunk * chunksize,
                                (chunk + 1) * chunksize - 1,
                                l2xe);
        if (rv < 0) {
            break;
        }
        for (ix = 0; ix < chunksize; ix++) {
            l2xep = soc_mem_table_idx_to_pointer(unit, L2Xm,
                                                 l2x_entry_t *, l2xe, ix);
            if (!soc_L2Xm_field32_get(unit, l2xep, valid_bit)) {
                continue;
            }
            if (dyn_only && soc_L2Xm_field32_get(unit, l2xep, STATIC_BITf)) {
                continue;
            }
            if (vlan_match) {
                 /* Match the vlan id */
                if (rep_st->key_vlan != 
                             soc_L2Xm_field32_get(unit, l2xep, VLAN_IDf)) {
                    continue;
                }
            }
            if (dest_match) {
                port_tgid = soc_L2Xm_field32_get(unit, l2xep, TGID_PORTf);
                if (rep_st->match_dest.trunk != -1) {
                    if (!(port_tgid & BCM_TGID_TRUNK_INDICATOR(unit))) {
                        continue;
                    } 
                    /* trunk128 always false for chips using this function */
                    if (rep_st->match_dest.trunk != 
                           (port_tgid & ~BCM_TGID_TRUNK_INDICATOR(unit))) {
                        continue;
                    }
                } else { /* modid/port */
                    if (port_tgid & BCM_TGID_TRUNK_INDICATOR(unit)) {
                        continue;
                    } 
                    if (rep_st->match_dest.module != 
                           (int)soc_L2Xm_field32_get(unit, l2xep, MODULE_IDf)) {
                        continue;
                    }
                    if (port_tgid != rep_st->match_dest.port) {
                        continue;
                    }
                }
            }
            if (SOC_L2_DEL_SYNC_LOCK(soc) < 0) {
                rv = BCM_E_RESOURCE;
                break;
            }
            if (((rv = soc_mem_delete(unit, L2Xm, MEM_BLOCK_ALL, l2xep)) < 0) ||
               ((rv = soc_l2x_sync_delete(unit, (uint32 *) l2xep,
                                          ((chunk * chunksize) + ix),
                                          soc_flags)) < 0)) {
                SOC_L2_DEL_SYNC_UNLOCK(soc);
                break;
            }
            SOC_L2_DEL_SYNC_UNLOCK(soc);
        }
        if (rv < 0) {
            break;
        }
    }
    soc_mem_unlock(unit, L2Xm);
    soc_cm_sfree(unit, l2xe);

    return rv;
}

/*
 * Function:
 *     _bcm_draco_l2_addr_replace_by_vlan_dest
 * Description:
 *     Helper function to _bcm_l2_addr_replace_by_vlan API to replace l2 entries 
 *      by destination for Draco family 
 * Parameters:
 *     unit         device number
 *     flags        flags BCM_L2_REPLACE_* 
 *     rep_st       structure with information of what to replace
 * Return:
 *     BCM_E_XXX
 */
int 
_bcm_draco_l2_addr_replace_by_vlan_dest(int unit, uint32 flags, _bcm_l2_replace_t *rep_st)
{
    int hw_aging = 0;

    if (NULL == rep_st) {
        return BCM_E_PARAM;
    }
    if (flags & BCM_L2_REPLACE_MATCH_VLAN) {
        VLAN_CHK_ID(unit, rep_st->key_vlan);
    }
  
    if (0 == (flags & BCM_L2_REPLACE_DELETE)) {
        if (SOC_IS_DRACO15(unit)) {
            BCM_IF_ERROR_RETURN(
                _bcm_draco_repl_reg_config(unit, rep_st));
        } else {
            return BCM_E_UNAVAIL;
        }
    }

    /* extended aging lets us remove dest+vlan entries and non-static only */
    if (soc_feature(unit, soc_feature_aging_extended)) {
        hw_aging = 1;
    } else if (flags & BCM_L2_REPLACE_MATCH_STATIC) {
        /* normal aging supports either dest or vlan and must include static */
        switch (flags & (BCM_L2_REPLACE_MATCH_DEST | BCM_L2_REPLACE_MATCH_VLAN)) {
        case BCM_L2_REPLACE_MATCH_DEST:
        case BCM_L2_REPLACE_MATCH_VLAN:
            hw_aging = 1;
            break;
        default:
            break;
        }
    }

    if (hw_aging) {
         BCM_IF_ERROR_RETURN(
             _bcm_draco_age_reg_config(unit, flags, rep_st));
    } else {
         BCM_IF_ERROR_RETURN(
              _bcm_draco_l2_delete_vlan_dest(unit, flags, rep_st));
    } 

    return BCM_E_NONE;
}

/*
 * Function:
 *	bcm_draco_l2_addr_delete_mcast
 * Description
 *	Delete L2 multicast entries
 * Parameters:
 *	unit  - device unit
 *	flags - BCM_L2_DELETE_XXX
 * Returns:
 *	BCM_E_XXX
 */
int
bcm_draco_l2_addr_delete_mcast(int unit, uint32 flags)
{
    l2x_entry_t *l2xe, *l2xep;
    int		rv, chunksize, nchunks, chunk, i;
    int		dyn_only;

    chunksize = soc_property_get(unit, spn_L2DELETE_CHUNKS,
				 DEFAULT_L2DELETE_CHUNKS);

    l2xe = soc_cm_salloc(unit, chunksize * sizeof(*l2xe), "l2del_chunk");
    if (l2xe == NULL) {
	return BCM_E_MEMORY;
    }

    nchunks = soc_mem_index_count(unit, L2Xm) / chunksize;
    rv = BCM_E_NONE;

    dyn_only = !(flags & BCM_L2_DELETE_STATIC);

    for (chunk = 0; chunk < nchunks; chunk++) {
	rv = soc_mem_read_range(unit, L2Xm, MEM_BLOCK_ANY,
				chunk * chunksize,
				(chunk + 1) * chunksize - 1,
				l2xe);
	if (rv < 0) {
	    break;
	}

	for (i = 0; i < chunksize; i++) {
            int valid_bit = SOC_IS_FBX(unit) ? VALIDf : VALID_BITf;

	    l2xep = soc_mem_table_idx_to_pointer(unit, L2Xm,
                                                 l2x_entry_t *, l2xe, i);

            if (!soc_L2Xm_field32_get(unit, l2xep, valid_bit)) {
                continue;
	    }

#ifdef BCM_FIREBOLT_SUPPORT
            if (SOC_IS_FBX(unit)) {
                bcm_mac_t mac;
                soc_L2Xm_mac_addr_get(unit, l2xep, MAC_ADDRf, mac);
                if (!BCM_MAC_IS_MCAST(mac)) {
	            continue;
	        }
            } else
#endif /* BCM_FIREBOLT_SUPPORT */
            {
	        if (!soc_L2Xm_field32_get(unit, l2xep, L2MCf)) {
	            continue;
	        }
	    }

	    if (dyn_only &&
		soc_L2Xm_field32_get(unit, l2xep, STATIC_BITf)) {
		continue;
	    }
	    rv = soc_mem_delete(unit, L2Xm, MEM_BLOCK_ALL, l2xep);
	    if (rv < 0) {
	        break;
	    }
	}

	if (rv < 0) {
	    break;
	}
    }

    soc_cm_sfree(unit, l2xe);

    return rv;
}

/*
 * Function:
 *	bcm_draco_l2_addr_get
 * Description:
 *	Given a MAC address and VLAN ID, check if the entry is present
 *	in the L2 table, and if so, return all associated information.
 * Parameters:
 *	unit - StrataSwitch PCI device unit number (driver internal).
 *	mac - input MAC address to search
 *	vid - input VLAN ID to search
 *	l2addr - Pointer to bcm_l2_addr_t structure to receive results
 * Returns:
 *	BCM_E_NONE		Success (l2addr filled in)
 *	BCM_E_PARAM		Illegal parameter (NULL pointer)
 *	BCM_E_INTERNAL		Chip access failure
 *	BCM_E_NOT_FOUND	Address not found (l2addr not filled in)
 */

int
bcm_draco_l2_addr_get(int unit, sal_mac_addr_t mac, bcm_vlan_t vid,
                      bcm_l2_addr_t *l2addr)
{
    l2x_entry_t	l2x_search, l2x_entry;
    int         index, rv;

    VLAN_CHK_ID(unit, vid);

    /* Set up entry for query */
    soc_L2Xm_mac_addr_set(unit, &l2x_search, MAC_ADDRf, mac);
    soc_L2Xm_field32_set(unit, &l2x_search, VLAN_IDf, vid);

    rv = soc_mem_search(unit, L2Xm, MEM_BLOCK_ANY, &index,
			&l2x_search, &l2x_entry, 0);

    BCM_IF_ERROR_RETURN(rv);

    BCM_IF_ERROR_RETURN(_bcm_l2_from_l2x(unit, l2addr, &l2x_entry));

    return BCM_E_NONE;
}

#if defined(BCM_XGS12_SWITCH_SUPPORT)
/*
 * Function:
 *	bcm_draco_l2_init
 * Description:
 *	Initialize chip-dependent parts of L2 module
 * Parameters:
 *	unit - StrataSwitch unit number.
 */

int
bcm_draco_l2_init(int unit)
{
    int         was_running = FALSE;
    uint32      flags;
    sal_usecs_t interval;

    if (soc_l2x_running(unit, &flags, &interval)) { 	 
        was_running = TRUE; 	 
        BCM_IF_ERROR_RETURN(soc_l2x_stop(unit)); 	 
    }
    soc_l2x_delete_all(unit, TRUE);

    if (_mbi_entries[unit] != NULL) {
        sal_free(_mbi_entries[unit]);
        _mbi_entries[unit] = NULL;
    }

    _mbi_num[unit] = (SOC_MEM_INFO(unit, MAC_BLOCKm).index_max -
		      SOC_MEM_INFO(unit, MAC_BLOCKm).index_min + 1);
    _mbi_entries[unit] = sal_alloc(_mbi_num[unit] *
                                   sizeof(_bcm_mac_block_info_t),
                                   "BCM L2X MAC blocking info");
    if (!_mbi_entries[unit]) {
        return BCM_E_MEMORY;
    }

    sal_memset(_mbi_entries[unit], 0,
	       _mbi_num[unit] * sizeof(_bcm_mac_block_info_t));

    /* bcm_l2_register clients */
    soc_l2x_register(unit, _bcm_l2_register_callback, NULL);

    if (was_running || SAL_BOOT_BCMSIM) {
        interval = (SAL_BOOT_BCMSIM)? BCMSIM_L2XMSG_INTERVAL : interval;
        soc_l2x_start(unit, flags, interval);
    }

    return SOC_E_NONE;
}
#endif /* BCM_XGS12_SWITCH_SUPPORT */

/*
 * Function:
 *	bcm_draco_l2_term
 * Description:
 *	Finalize chip-dependent parts of L2 module
 * Parameters:
 *	unit - StrataSwitch unit number.
 */

int
bcm_draco_l2_term(int unit)
{
    if (_mbi_entries[unit] != NULL) {
        sal_free(_mbi_entries[unit]);
        _mbi_entries[unit] = NULL;
    }

    return SOC_E_NONE;
}

int
bcm_draco_l2_conflict_get(int unit, bcm_l2_addr_t *addr,
			  bcm_l2_addr_t *cf_array, int cf_max,
			  int *cf_count)
{

    l2x_valid_entry_t	vbits;
    l2x_entry_t		l2ent;
    uint8		key[XGS_HASH_KEY_SIZE];
    int			hash_select, bucket, slot;

    *cf_count = 0;

    SOC_IF_ERROR_RETURN(soc_draco_hash_get(unit, &hash_select));

    soc_draco_l2x_param_to_key(addr->mac, addr->vid, key);
    bucket = soc_draco_l2_hash(unit, hash_select, key);

    SOC_IF_ERROR_RETURN
	(soc_mem_read(unit, L2X_VALIDm, MEM_BLOCK_ANY, bucket, &vbits));

    for (slot = 0;
	 slot < SOC_L2X_BUCKET_SIZE && *cf_count < cf_max;
	 slot++) {
	if (vbits.entry_data[0] & (1 << slot)) {
	    SOC_IF_ERROR_RETURN
		(soc_mem_read(unit, L2Xm, MEM_BLOCK_ANY,
			      bucket * SOC_L2X_BUCKET_SIZE + slot,
			      &l2ent));
	    BCM_IF_ERROR_RETURN
		(_bcm_l2_from_l2x(unit, &cf_array[*cf_count], &l2ent));
	    *cf_count += 1;
	}
    }
    return BCM_E_NONE;
}
