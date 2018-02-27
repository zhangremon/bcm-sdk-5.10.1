/*
 * $Id: l3.c 1.12.2.4 Broadcom SDK $
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
 * File:        l3.c
 * Purpose:     Trident L3 function implementations
 */


#include <soc/defs.h>

#include <assert.h>

#include <sal/core/libc.h>
#if defined(BCM_TRIDENT_SUPPORT)  && defined(INCLUDE_L3)

#include <shared/util.h>
#include <soc/mem.h>
#include <soc/cm.h>
#include <soc/drv.h>
#include <soc/register.h>
#include <soc/memory.h>
#include <soc/l3x.h>
#include <soc/lpm.h>
#include <soc/tnl_term.h>

#include <bcm/l3.h>
#include <bcm/tunnel.h>
#include <bcm/debug.h>
#include <bcm/error.h>
#include <bcm/stack.h>

#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/firebolt.h>
#include <bcm_int/esw/trx.h>
#include <bcm_int/esw/trident.h>
#if defined(BCM_TRIUMPH2_SUPPORT)
#include <bcm_int/esw/triumph2.h>
#endif /* BCM_TRIUMPH2_SUPPORT */
#include <bcm_int/esw/l3.h>
#include <bcm_int/esw/xgs3.h>

extern _bcm_l3_module_data_t *l3_module_data[BCM_MAX_NUM_UNITS];

#include <bcm_int/esw_dispatch.h>
/*
 * Function:
 *      _bcm_td_l3_ecmp_grp_get
 * Purpose:
 *      Get ecmp group next hop members by index.
 * Parameters:
 *      unit       - (IN)SOC unit number.
 *      ecmp_grp - (IN)Ecmp group id to read. 
 *      ecmp_count - (IN)Maximum number of entries to read.
 *      nh_idx     - (OUT)Next hop indexes. 
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_td_l3_ecmp_grp_get (int unit, int ecmp_grp, int ecmp_group_size, int *nh_idx)
{
    int idx; /* Iteration index. */
    int max_ent_count=0; /* Number of entries to read.*/
    uint32 hw_buf[SOC_MAX_MEM_FIELD_WORDS]; /* Buffer to read hw entry. */
    int one_entry_grp = TRUE; /* Single next hop entry group.  */ 
    int rv = BCM_E_UNAVAIL; /* Operation return status.  */
    int ecmp_idx = 0;

    /* Input parameters sanity check. */
    if ((NULL == nh_idx) || (ecmp_group_size < 1)) {
         return (BCM_E_PARAM);
    }

    /* Zero all next hop indexes first. */
    sal_memset(nh_idx, 0, ecmp_group_size * sizeof(int));
    sal_memset(hw_buf, 0, SOC_MAX_MEM_FIELD_WORDS * sizeof(uint32));

    /* Calculate Base_ptr table index. */
    if (SOC_MEM_IS_VALID(unit, L3_ECMP_COUNTm)) {

         /* Read zero based ecmp count. */
         rv = soc_mem_read(unit, L3_ECMP_COUNTm, MEM_BLOCK_ANY, ecmp_grp, hw_buf);
         if (BCM_FAILURE(rv)) {
              return rv;
         }
         if (soc_feature(unit, soc_feature_l3_ecmp_1k_groups)) {
             max_ent_count = soc_mem_field32_get(unit, L3_ECMP_COUNTm, hw_buf, COUNTf);
             ecmp_idx = soc_mem_field32_get(unit, L3_ECMP_COUNTm, hw_buf, BASE_PTRf);
         } else {
             max_ent_count = soc_mem_field32_get(unit, L3_ECMP_COUNTm, hw_buf, COUNT_0f);
             ecmp_idx = soc_mem_field32_get(unit, L3_ECMP_COUNTm, hw_buf, BASE_PTR_0f);
         }
         max_ent_count++; /* Count is zero based. */ 
    }

    /* Read all the indexes from hw. */
    for (idx = 0; idx < max_ent_count; idx++) {

         /* Read next hop index. */
         rv = soc_mem_read(unit, L3_ECMPm, MEM_BLOCK_ANY, 
                             (ecmp_idx + idx), hw_buf);
         if (rv < 0) {
              break;
         }
         nh_idx[idx] = soc_mem_field32_get(unit, L3_ECMPm, 
                                            hw_buf, NEXT_HOP_INDEXf);
         /* Check if group contains . */ 
         if (idx && (nh_idx[idx] != nh_idx[0])) { 
              one_entry_grp = FALSE;
         }

         /* Next hops popuplated in cycle,stop once you read first entry again */
         if (idx && (FALSE == one_entry_grp) && (nh_idx[idx] == nh_idx[0])) {
              nh_idx[idx] = 0;
              break;
         }
    }
    /* Reset rest of the group if only 1 next hop is present. */
    if (one_entry_grp) {
         sal_memset(nh_idx + 1, 0, (ecmp_group_size - 1) * sizeof(int)); 
    }
    return rv;
}


/*
 * Function:
 *      _bcm_td_l3_ecmp_grp_add
 * Purpose:
 *      Add ecmp group next hop members, or reset ecmp group entry.  
 *      NOTE: Function always writes all the entries in ecmp group.
 *            If there is not enough nh indexes - next hops written
 *            in cycle. 
 * Parameters:
 *      unit       - (IN)SOC unit number.
 *      ecmp_grp   - (IN)ecmp group id to write.
 *      buf        - (IN)Next hop indexes or NULL for entry reset.
 *      max_paths - (IN) ECMP Max paths
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_td_l3_ecmp_grp_add (int unit, int ecmp_grp, void *buf, int max_paths)
{
    uint32 l3_ecmp[SOC_MAX_MEM_FIELD_WORDS];
    uint32 l3_ecmp_count[SOC_MAX_MEM_FIELD_WORDS];
    uint32 hw_buf[SOC_MAX_MEM_FIELD_WORDS];
    int max_grp_size=0;  /* Maximum ecmp group size.*/
    int ecmp_idx; /* Ecmp table entry index. */
    int *nh_idx; /* Ecmp group nh indexes.  */
    int idx=0;  /* Iteration index.  */
    int rv = BCM_E_UNAVAIL; /* Operation return value. */
    int entry_type;
    int l3_ecmp_oif_flds[8] = {  L3_OIF_0f, 
                             L3_OIF_1f, 
                             L3_OIF_2f, 
                             L3_OIF_3f, 
                             L3_OIF_4f, 
                             L3_OIF_5f, 
                             L3_OIF_6f, 
                             L3_OIF_7f }; 
    int l3_ecmp_oif_type_flds[8] = {  L3_OIF_0_TYPEf, 
                             L3_OIF_1_TYPEf, 
                             L3_OIF_2_TYPEf, 
                             L3_OIF_3_TYPEf, 
                             L3_OIF_4_TYPEf, 
                             L3_OIF_5_TYPEf, 
                             L3_OIF_6_TYPEf, 
                             L3_OIF_7_TYPEf };
    ing_l3_next_hop_entry_t ing_nh;
    uint32 reg_val, value;
    _bcm_l3_tbl_op_t data;

    /* Input parameters check. */
    if (NULL == buf) {
        return (BCM_E_PARAM);
    }

    /* Cast input buffer. */
    nh_idx = (int *) buf;
    max_grp_size = max_paths;

    /* Calculate table index. */
    data.width = max_paths;
    data.tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, ecmp); 
    data.oper_flags = _BCM_L3_SHR_TABLE_TRAVERSE_CONTROL; 
    data.entry_index = -1;
    /* Get index to the first slot in the ECMP table
     * that can accomodate max_grp_size */
    BCM_IF_ERROR_RETURN(_bcm_xgs3_tbl_free_idx_get(unit, &data));
    ecmp_idx = data.entry_index;
    BCM_XGS3_L3_ENT_REF_CNT_INC(data.tbl_ptr, data.entry_index, max_grp_size);

    if (ecmp_idx >= BCM_XGS3_L3_ECMP_TBL_SIZE(unit)) {
        return BCM_E_FULL;
    }
    
    sal_memset (hw_buf, 0, SOC_MAX_MEM_FIELD_WORDS * sizeof(uint32));

    /* Write all the indexes to hw. */
    for (idx = 0; idx < max_grp_size; idx++) {
         /* Set next hop index. */
         sal_memset (l3_ecmp, 0, SOC_MAX_MEM_FIELD_WORDS * sizeof(uint32));

         if (!nh_idx[idx]) {
              break;
         }
         soc_mem_field32_set(unit, L3_ECMPm, l3_ecmp, 
                                  NEXT_HOP_INDEXf, nh_idx[idx]);

         /* Write buffer to hw L3_ECMPm table. */
         rv = soc_mem_write(unit, L3_ECMPm, MEM_BLOCK_ALL, 
                                  (ecmp_idx + idx), l3_ecmp);

         if (BCM_FAILURE(rv)) {
              break;
         }

         /* Write buffer to hw INITIAL_L3_ECMPm table. */
         rv = soc_mem_write(unit, INITIAL_L3_ECMPm, MEM_BLOCK_ALL, 
                                  (ecmp_idx + idx), l3_ecmp);

         if (BCM_FAILURE(rv)) {
              break;
         }

         if (soc_feature(unit, soc_feature_urpf)) {
              /* Check if URPF is enabled on device */
              BCM_IF_ERROR_RETURN(
                   READ_L3_DEFIP_RPF_CONTROLr(unit, &reg_val));
              if (reg_val) {
                   if (idx < 8) {
                        BCM_IF_ERROR_RETURN (soc_mem_read(unit, 
                             ING_L3_NEXT_HOPm, MEM_BLOCK_ANY, 
                             nh_idx[idx], &ing_nh));

                        entry_type = 
                          soc_ING_L3_NEXT_HOPm_field32_get(unit, 
                               &ing_nh, ENTRY_TYPEf);

                        if (entry_type == 0x0) {
                             value = soc_ING_L3_NEXT_HOPm_field32_get(unit, 
                                            &ing_nh, VLAN_IDf);
                             soc_mem_field32_set(unit, L3_ECMP_COUNTm, hw_buf, 
                                            l3_ecmp_oif_type_flds[idx], entry_type);
                             soc_mem_field32_set(unit, L3_ECMP_COUNTm, hw_buf, 
                                            l3_ecmp_oif_flds[idx], value);
                        } else if (entry_type == 0x1) {
                             value  = soc_ING_L3_NEXT_HOPm_field32_get(unit, &ing_nh, L3_OIFf);
                             soc_mem_field32_set(unit, L3_ECMP_COUNTm, hw_buf, 
                                            l3_ecmp_oif_type_flds[idx], entry_type);
                             soc_mem_field32_set(unit, L3_ECMP_COUNTm, hw_buf, 
                                            l3_ecmp_oif_flds[idx], value);
                        }
                        soc_mem_field32_set(unit, L3_ECMP_COUNTm, hw_buf, URPF_COUNTf, idx);
                   }else {
                       /* Inorder to avoid TRAP_TO_CPU, urpf_mode on L3_IIF/PORT must be set 
                           to STRICT_MODE / LOOSE_MODE */
                        soc_mem_field32_set(unit, L3_ECMP_COUNTm, hw_buf, ECMP_GT8f , 1);
                   }
              }
         }
    }

    if (BCM_SUCCESS(rv)) {
         /* Set Max Group Size. */
         sal_memset (l3_ecmp_count, 0, SOC_MAX_MEM_FIELD_WORDS * sizeof(uint32));
         if (soc_feature(unit, soc_feature_l3_ecmp_1k_groups)) {
              soc_mem_field32_set(unit, L3_ECMP_COUNTm, l3_ecmp_count, COUNTf, max_grp_size-1);
         }  else {
              soc_mem_field32_set(unit, L3_ECMP_COUNTm, l3_ecmp_count, COUNT_0f, max_grp_size-1);
              soc_mem_field32_set(unit, L3_ECMP_COUNTm, l3_ecmp_count, COUNT_1f, max_grp_size-1);
              soc_mem_field32_set(unit, L3_ECMP_COUNTm, l3_ecmp_count, COUNT_2f, max_grp_size-1);
              soc_mem_field32_set(unit, L3_ECMP_COUNTm, l3_ecmp_count, COUNT_3f, max_grp_size-1);
         }
         rv = soc_mem_write(unit, L3_ECMP_COUNTm, MEM_BLOCK_ALL, 
                                  (ecmp_grp+1), l3_ecmp_count);
         BCM_IF_ERROR_RETURN(rv);

         if (soc_feature(unit, soc_feature_l3_ecmp_1k_groups)) {
              if (!idx) {
                  soc_mem_field32_set(unit, L3_ECMP_COUNTm, hw_buf, COUNTf, idx);
              } else {
                  soc_mem_field32_set(unit, L3_ECMP_COUNTm, hw_buf, COUNTf, idx-1);
              }
              if (SOC_MEM_FIELD_VALID(unit, L3_ECMP_COUNTm, BASE_PTRf)) {
                   soc_mem_field32_set(unit, L3_ECMP_COUNTm, hw_buf,
                                  BASE_PTRf, ecmp_idx);
              }
         }  else {
              if (!idx) {
                   soc_mem_field32_set(unit, L3_ECMP_COUNTm, hw_buf, COUNT_0f, idx );
                   soc_mem_field32_set(unit, L3_ECMP_COUNTm, hw_buf, COUNT_1f, idx );
                   soc_mem_field32_set(unit, L3_ECMP_COUNTm, hw_buf, COUNT_2f, idx );
                   soc_mem_field32_set(unit, L3_ECMP_COUNTm, hw_buf, COUNT_3f, idx );
              } else {
                   soc_mem_field32_set(unit, L3_ECMP_COUNTm, hw_buf, COUNT_0f, idx - 1);
                   soc_mem_field32_set(unit, L3_ECMP_COUNTm, hw_buf, COUNT_1f, idx - 1);
                   soc_mem_field32_set(unit, L3_ECMP_COUNTm, hw_buf, COUNT_2f, idx - 1);
                   soc_mem_field32_set(unit, L3_ECMP_COUNTm, hw_buf, COUNT_3f, idx - 1);
              }

              /* Set group base pointer. */
              if (SOC_MEM_FIELD_VALID(unit, L3_ECMP_COUNTm, BASE_PTR_0f)) {
                   soc_mem_field32_set(unit, L3_ECMP_COUNTm, hw_buf,
                                  BASE_PTR_0f, ecmp_idx);
              }
              if (SOC_MEM_FIELD_VALID(unit, L3_ECMP_COUNTm, BASE_PTR_1f)) {
                   soc_mem_field32_set(unit, L3_ECMP_COUNTm, hw_buf,
                                  BASE_PTR_1f, ecmp_idx);
              }
              if (SOC_MEM_FIELD_VALID(unit, L3_ECMP_COUNTm, BASE_PTR_2f)) {
                   soc_mem_field32_set(unit, L3_ECMP_COUNTm, hw_buf,
                                  BASE_PTR_2f, ecmp_idx);
              }
              if (SOC_MEM_FIELD_VALID(unit, L3_ECMP_COUNTm, BASE_PTR_3f)) {
                   soc_mem_field32_set(unit, L3_ECMP_COUNTm, hw_buf,
                                  BASE_PTR_3f, ecmp_idx);
              }
         }

         rv = soc_mem_write(unit, L3_ECMP_COUNTm, MEM_BLOCK_ALL,
                                  ecmp_grp, hw_buf);
         BCM_IF_ERROR_RETURN(rv);

         rv = soc_mem_write(unit, INITIAL_L3_ECMP_GROUPm,
                                  MEM_BLOCK_ALL, ecmp_grp, hw_buf);
    }
    return rv;
}


/*
 * Function:
 *      _bcm_td_l3_ecmp_grp_del
 * Purpose:
 *      Reset ecmp group next hop members
 * Parameters:
 *      unit       - (IN)SOC unit number.
 *      ecmp_grp   - (IN)ecmp group id to write.
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_td_l3_ecmp_grp_del (int unit, int ecmp_grp, int max_grp_size)
{
    uint32 hw_buf[SOC_MAX_MEM_FIELD_WORDS]; /* Buffer to read hw entry.*/
    int ecmp_idx = 0; /* Ecmp table entry index. */
    int idx; /* Iteration index. */
    int rv = BCM_E_UNAVAIL; /* Operation return value. */
    _bcm_l3_tbl_op_t data;
    data.tbl_ptr = BCM_XGS3_L3_TBL_PTR(unit, ecmp); 

    /* Initialize ecmp entry. */
    sal_memset (hw_buf, 0, SOC_MAX_MEM_FIELD_WORDS * sizeof(uint32));

    /* Calculate table index. */
    if (SOC_MEM_IS_VALID(unit, L3_ECMP_COUNTm)) {
        BCM_IF_ERROR_RETURN(soc_mem_read(unit, L3_ECMP_COUNTm,
                                         MEM_BLOCK_ANY, ecmp_grp, hw_buf));
        if (soc_feature(unit, soc_feature_l3_ecmp_1k_groups)) {
            ecmp_idx = soc_mem_field32_get(unit, L3_ECMP_COUNTm, hw_buf, BASE_PTRf);
        } else {
            ecmp_idx = soc_mem_field32_get(unit, L3_ECMP_COUNTm, hw_buf, BASE_PTR_0f);
        }
    }

    /* Write all the indexes to hw. */
    for (idx = 0; idx < max_grp_size; idx++) {
         /* Write buffer to hw. */
         rv = soc_mem_write(unit, L3_ECMPm, MEM_BLOCK_ALL, 
                                  (ecmp_idx + idx), hw_buf);
         if (BCM_FAILURE(rv)) {
              return rv;
         }
         /* Write buffer to hw INITIAL_L3_ECMPm table. */
         rv = soc_mem_write(unit, INITIAL_L3_ECMPm, MEM_BLOCK_ALL, 
                                  (ecmp_idx + idx), hw_buf);
         if (BCM_FAILURE(rv)) {
              return rv;
         }
    }

    /* Decrement ref count for the entries in ecmp table
     * Ref count for ecmp_group table is decremented in common table del func. */
    BCM_XGS3_L3_ENT_REF_CNT_DEC(data.tbl_ptr, ecmp_idx, max_grp_size);

    if (SOC_MEM_IS_VALID(unit, L3_ECMP_COUNTm)) {
         /* Set group base pointer. */
         ecmp_idx = ecmp_grp;

         rv = soc_mem_write(unit, L3_ECMP_COUNTm,
                                  MEM_BLOCK_ALL, ecmp_idx, hw_buf);
         BCM_IF_ERROR_RETURN(rv);
         rv = soc_mem_write(unit, L3_ECMP_COUNTm,
                                  MEM_BLOCK_ALL, (ecmp_idx+1), hw_buf);
         BCM_IF_ERROR_RETURN(rv);

         if (SOC_MEM_IS_VALID(unit, INITIAL_L3_ECMP_GROUPm)) {
              rv = soc_mem_write(unit, INITIAL_L3_ECMP_GROUPm,
                                  MEM_BLOCK_ALL, ecmp_idx, hw_buf);
         }
    }
    return rv;
}

/*
 * Function:
 *      _bcm_td_l3_intf_urpf_default_route_set
 * Purpose:
 *      Set urpf_default_route enable flag for the L3 interface. 
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      vid       - (IN)Vlan id. 
 *      enable    - (IN)Enable. 
 * Returns:
 *      BCM_E_XXX.
 */
int
_bcm_td_l3_intf_urpf_default_route_set(int unit, bcm_vlan_t vid, int enable)  
{
    _bcm_l3_ingress_intf_t iif; /* Ingress interface config.*/
    int ret_val;                /* Operation return value.  */

    /* Input parameters sanity check. */
    if ((vid > soc_mem_index_max(unit, L3_IIFm)) || 
        (vid < soc_mem_index_min(unit, L3_IIFm))) {
        return (BCM_E_PARAM);
    }

    iif.intf_id = vid;
    soc_mem_lock(unit, L3_IIFm);

    ret_val = _bcm_tr_l3_ingress_interface_get(unit, &iif);
    if (BCM_FAILURE(ret_val)) {
        soc_mem_unlock(unit, L3_IIFm);
        return (ret_val);
    }

    if (enable) {
       iif.flags &= ~BCM_VLAN_L3_URPF_DEFAULT_ROUTE_CHECK_DISABLE;
    } else {
       iif.flags |= BCM_VLAN_L3_URPF_DEFAULT_ROUTE_CHECK_DISABLE;
    }

    ret_val = _bcm_tr_l3_ingress_interface_set(unit, &iif);

    soc_mem_unlock(unit, L3_IIFm);

    return (ret_val);
}

/*
 * Function:
 *      _bcm_td_l3_intf_urpf_default_route_get
 * Purpose:
 *      Get the urpf_default_route_check enable flag for the specified L3 interface
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      vid       - (IN)Vlan id. 
 *      enable    - (OUT)enable. 
 * Returns:
 *      BCM_E_XXX.
 */
int
_bcm_td_l3_intf_urpf_default_route_get (int unit, bcm_vlan_t vid, int *enable)
{
    _bcm_l3_ingress_intf_t iif; /* Ingress interface config.*/

    /* Input parameters sanity check. */
    if ((vid > soc_mem_index_max(unit, L3_IIFm)) || 
        (vid < soc_mem_index_min(unit, L3_IIFm))) {
        return (BCM_E_PARAM);
    }

    iif.intf_id = vid;

    BCM_IF_ERROR_RETURN(_bcm_tr_l3_ingress_interface_get(unit, &iif));

    *enable = (iif.flags & BCM_VLAN_L3_URPF_DEFAULT_ROUTE_CHECK_DISABLE) ? 0 : 1;
    return (BCM_E_NONE);
}
/*
 * Function:
 *      _bcm_td_l3_intf_urpf_mode_set
 * Purpose:
 *      Set the urpf_mode info for the specified L3 interface
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      vid       - (IN)Vlan id. 
 *      urpf_mode       - (IN)urpf_mode. 
 * Returns:
 *      BCM_E_XXX.
 */
int
_bcm_td_l3_intf_urpf_mode_set(int unit, bcm_vlan_t vid, bcm_vlan_urpf_mode_t urpf_mode)  
{
    _bcm_l3_ingress_intf_t iif; /* Ingress interface config.*/
    int ret_val;                /* Operation return value.  */

    /* Input parameters sanity check. */
    if ((vid > soc_mem_index_max(unit, L3_IIFm)) || 
        (vid < soc_mem_index_min(unit, L3_IIFm))) {
        return (BCM_E_PARAM);
    }
   
    iif.intf_id = vid;
    soc_mem_lock(unit, L3_IIFm);

    ret_val = _bcm_tr_l3_ingress_interface_get(unit, &iif);
    if (BCM_FAILURE(ret_val)) {
        soc_mem_unlock(unit, L3_IIFm);
        return (ret_val);
    }

    iif.urpf_mode = urpf_mode;

    ret_val = _bcm_tr_l3_ingress_interface_set(unit, &iif);

    soc_mem_unlock(unit, L3_IIFm);

    return (ret_val);
}

/*
 * Function:
 *      _bcm_td_l3_intf_urpf_mode_get
 * Purpose:
 *      Get the urpf_mode info for the specified L3 interface
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      vid       - (IN)Vlan id. 
 *      urpf_mode       - (OUT) urpf_mode
 * Returns:
 *      BCM_E_XXX.
 */
int
_bcm_td_l3_intf_urpf_mode_get(int unit, bcm_vlan_t vid, bcm_vlan_urpf_mode_t *urpf_mode)
{
    _bcm_l3_ingress_intf_t iif; /* Ingress interface config.*/

    /* Input parameters sanity check. */
    if ((vid > soc_mem_index_max(unit, L3_IIFm)) || 
        (vid < soc_mem_index_min(unit, L3_IIFm))) {
        return (BCM_E_PARAM);
    }

    iif.intf_id = vid;

    BCM_IF_ERROR_RETURN(_bcm_tr_l3_ingress_interface_get(unit, &iif));

    *urpf_mode = iif.urpf_mode;
    return (BCM_E_NONE);
}

/*
 * Function:
 *      _bcm_td_l3_intf_qos_set
 * Purpose:
 *      Set L3 Interface QoS parameters
 * Parameters:
 *      unit      - (IN)SOC unit number.
 *      l3_if_entry_p - (IN) Pointer to buffer
 *      vid       - (IN) Intf_info
 * Returns:
 *      BCM_E_XXX.
 */
int
_bcm_td_l3_intf_qos_set(int unit, uint32 *l3_if_entry_p, _bcm_l3_intf_cfg_t *intf_info)
{
    int hw_map_idx=0;

    /* Input parameters check */
    if (NULL == intf_info) {
        return (BCM_E_PARAM);
    }

    if (intf_info->vlan_qos.flags & BCM_L3_INTF_QOS_OUTER_VLAN_PRI_COPY) {
         if (SOC_MEM_FIELD_VALID(unit, EGR_L3_INTFm, OPRI_OCFI_SELf)) {
              soc_mem_field32_set(unit, EGR_L3_INTFm, l3_if_entry_p, OPRI_OCFI_SELf, 0x0);
         }
    } else if (intf_info->vlan_qos.flags & BCM_L3_INTF_QOS_OUTER_VLAN_PRI_SET) {
         if (SOC_MEM_FIELD_VALID(unit, EGR_L3_INTFm, OPRI_OCFI_SELf)) {
              soc_mem_field32_set(unit, EGR_L3_INTFm, l3_if_entry_p, OPRI_OCFI_SELf, 0x1);
              soc_mem_field32_set(unit, EGR_L3_INTFm, l3_if_entry_p, OPRIf,
                                            intf_info->vlan_qos.pri);
              soc_mem_field32_set(unit, EGR_L3_INTFm, l3_if_entry_p, OCFIf,
                                            intf_info->vlan_qos.cfi);
         }
    } else if (intf_info->vlan_qos.flags & BCM_L3_INTF_QOS_OUTER_VLAN_PRI_REMARK) {

         if (SOC_MEM_FIELD_VALID(unit, EGR_L3_INTFm, OPRI_OCFI_SELf)) {
              soc_mem_field32_set(unit, EGR_L3_INTFm, l3_if_entry_p, OPRI_OCFI_SELf, 0x2);
         }
         if (SOC_MEM_FIELD_VALID(unit, EGR_L3_INTFm, OPRI_OCFI_MAPPING_PROFILEf)) {
              BCM_IF_ERROR_RETURN
               (_bcm_tr2_qos_id2idx(unit, intf_info->vlan_qos.qos_map_id, &hw_map_idx));
              soc_mem_field32_set(unit, EGR_L3_INTFm, l3_if_entry_p, 
                             OPRI_OCFI_MAPPING_PROFILEf, hw_map_idx);
         }
    }

    if (intf_info->inner_vlan_qos.flags & BCM_L3_INTF_QOS_INNER_VLAN_PRI_COPY) {
       if (SOC_MEM_FIELD_VALID(unit, EGR_L3_INTFm, IPRI_ICFI_SELf)) {
           soc_mem_field32_set(unit, EGR_L3_INTFm, l3_if_entry_p, IPRI_ICFI_SELf, 0x0);
       }
   } else if (intf_info->inner_vlan_qos.flags & BCM_L3_INTF_QOS_INNER_VLAN_PRI_SET) {
       if (SOC_MEM_FIELD_VALID(unit, EGR_L3_INTFm, IPRI_ICFI_SELf)) {
           soc_mem_field32_set(unit, EGR_L3_INTFm, l3_if_entry_p, IPRI_ICFI_SELf, 0x1);
           soc_mem_field32_set(unit, EGR_L3_INTFm, l3_if_entry_p, IPRIf,
                        intf_info->inner_vlan_qos.pri);
           soc_mem_field32_set(unit, EGR_L3_INTFm, l3_if_entry_p, ICFIf,
                        intf_info->inner_vlan_qos.cfi);
       }
   } else if (intf_info->vlan_qos.flags & BCM_L3_INTF_QOS_INNER_VLAN_PRI_REMARK) {
       if (SOC_MEM_FIELD_VALID(unit, EGR_L3_INTFm, IPRI_ICFI_SELf)) {
           soc_mem_field32_set(unit, EGR_L3_INTFm, l3_if_entry_p, IPRI_ICFI_SELf, 0x2);
       }
       if (SOC_MEM_FIELD_VALID(unit, EGR_L3_INTFm, IPRI_ICFI_MAPPING_PROFILEf)) {
           BCM_IF_ERROR_RETURN
              (_bcm_tr2_qos_id2idx(unit, intf_info->inner_vlan_qos.qos_map_id, &hw_map_idx));
           soc_mem_field32_set(unit, EGR_L3_INTFm, l3_if_entry_p, 
                        IPRI_ICFI_MAPPING_PROFILEf, hw_map_idx);
       }
   }

    if (intf_info->dscp_qos.flags & BCM_L3_INTF_QOS_DSCP_COPY) {
         if (SOC_MEM_FIELD_VALID(unit, EGR_L3_INTFm, DSCP_SELf)) {
              soc_mem_field32_set(unit, EGR_L3_INTFm, l3_if_entry_p, DSCP_SELf, 0x0);
         }
    } else if (intf_info->dscp_qos.flags & BCM_L3_INTF_QOS_DSCP_SET) {
         if (SOC_MEM_FIELD_VALID(unit, EGR_L3_INTFm, DSCP_SELf)) {
              soc_mem_field32_set(unit, EGR_L3_INTFm, l3_if_entry_p, DSCP_SELf, 0x1);
              soc_mem_field32_set(unit, EGR_L3_INTFm, l3_if_entry_p, DSCPf,
                                            intf_info->dscp_qos.dscp);
         }
    } else if (intf_info->dscp_qos.flags & BCM_L3_INTF_QOS_DSCP_REMARK) {
         if (SOC_MEM_FIELD_VALID(unit, EGR_L3_INTFm, DSCP_SELf)) {
              soc_mem_field32_set(unit, EGR_L3_INTFm, l3_if_entry_p, DSCP_SELf, 0x2);
         }
         if (SOC_MEM_FIELD_VALID(unit, EGR_L3_INTFm, DSCP_MAPPING_PTRf)) {
              BCM_IF_ERROR_RETURN
               (_bcm_tr2_qos_id2idx(unit, intf_info->dscp_qos.qos_map_id, &hw_map_idx));
              soc_mem_field32_set(unit, EGR_L3_INTFm, l3_if_entry_p, 
                             DSCP_MAPPING_PTRf, hw_map_idx);
         }
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_td_l3_intf_qos_get
 * Purpose:
 *      Get L3 Interface QoS parameters
 * Parameters:
 *      unit      - (IN)SOC unit number.
*       l3_if_entry_p - entry buffer
 *      vid       - (IN) Intf_info
 * Returns:
 *      BCM_E_XXX.
 */
int
_bcm_td_l3_intf_qos_get(int unit, uint32 *l3_if_entry_p, _bcm_l3_intf_cfg_t *intf_info)
{

    /* Input parameters check */
    if (NULL == intf_info) {
        return (BCM_E_PARAM);
    }

    if (SOC_MEM_FIELD_VALID(unit, EGR_L3_INTFm, OPRI_OCFI_SELf)) {
       if (soc_mem_field32_get(unit, EGR_L3_INTFm,
                l3_if_entry_p, OPRI_OCFI_SELf) == 0x0) {
              intf_info->vlan_qos.flags |= BCM_L3_INTF_QOS_OUTER_VLAN_PRI_COPY;
       } else if (soc_mem_field32_get(unit, EGR_L3_INTFm,
                l3_if_entry_p, OPRI_OCFI_SELf) == 0x1) {
              intf_info->vlan_qos.flags |= BCM_L3_INTF_QOS_OUTER_VLAN_PRI_SET;
              intf_info->vlan_qos.pri = 
                   soc_mem_field32_get(unit, EGR_L3_INTFm, l3_if_entry_p, OPRIf);
              intf_info->vlan_qos.cfi = 
                   soc_mem_field32_get(unit, EGR_L3_INTFm, l3_if_entry_p, OCFIf);
       } else if (soc_mem_field32_get(unit, EGR_L3_INTFm,
                l3_if_entry_p, OPRI_OCFI_SELf) == 0x2) {
              intf_info->vlan_qos.flags |= BCM_L3_INTF_QOS_OUTER_VLAN_PRI_REMARK;
              BCM_IF_ERROR_RETURN
                   (_bcm_tr2_qos_idx2id(unit, 
                        soc_mem_field32_get(unit, EGR_L3_INTFm, l3_if_entry_p,
                                OPRI_OCFI_MAPPING_PROFILEf), 0x2,
                        &intf_info->vlan_qos.qos_map_id));
        }
    }

    if (SOC_MEM_FIELD_VALID(unit, EGR_L3_INTFm, IPRI_ICFI_SELf)) {
       if (soc_mem_field32_get(unit, EGR_L3_INTFm,
                l3_if_entry_p, IPRI_ICFI_SELf) == 0x0) {
              intf_info->inner_vlan_qos.flags |= BCM_L3_INTF_QOS_INNER_VLAN_PRI_COPY;
       } else if (soc_mem_field32_get(unit, EGR_L3_INTFm,
                l3_if_entry_p, IPRI_ICFI_SELf) == 0x1) {
              intf_info->inner_vlan_qos.flags |= BCM_L3_INTF_QOS_INNER_VLAN_PRI_SET;
              intf_info->inner_vlan_qos.pri = 
                   soc_mem_field32_get(unit, EGR_L3_INTFm, l3_if_entry_p, IPRIf);
              intf_info->inner_vlan_qos.cfi = 
                   soc_mem_field32_get(unit, EGR_L3_INTFm, l3_if_entry_p, ICFIf);
       } else if (soc_mem_field32_get(unit, EGR_L3_INTFm,
                l3_if_entry_p, IPRI_ICFI_SELf) == 0x2) {
              intf_info->inner_vlan_qos.flags |= BCM_L3_INTF_QOS_INNER_VLAN_PRI_REMARK;
              BCM_IF_ERROR_RETURN
                   (_bcm_tr2_qos_idx2id(unit, 
                        soc_mem_field32_get(unit, EGR_L3_INTFm, l3_if_entry_p,
                                IPRI_ICFI_MAPPING_PROFILEf), 0x2,
                        &intf_info->inner_vlan_qos.qos_map_id));
        }
    }


    if (SOC_MEM_FIELD_VALID(unit, EGR_L3_INTFm, DSCP_SELf)) {
         if (soc_mem_field32_get(unit, EGR_L3_INTFm,
                  l3_if_entry_p, DSCP_SELf) == 0x0) {
              intf_info->dscp_qos.flags |= BCM_L3_INTF_QOS_DSCP_COPY;
         } else if (soc_mem_field32_get(unit, EGR_L3_INTFm,
                  l3_if_entry_p, DSCP_SELf) == 0x1) {
              intf_info->dscp_qos.flags |= BCM_L3_INTF_QOS_DSCP_SET;
              intf_info->dscp_qos.dscp = 
                   soc_mem_field32_get(unit, EGR_L3_INTFm, l3_if_entry_p, DSCPf);
         } else if (soc_mem_field32_get(unit, EGR_L3_INTFm,
                  l3_if_entry_p, DSCP_SELf) == 0x2) {
              intf_info->dscp_qos.flags |= BCM_L3_INTF_QOS_DSCP_REMARK;
              BCM_IF_ERROR_RETURN
                   (_bcm_tr2_qos_idx2id(unit, 
                        soc_mem_field32_get(unit, EGR_L3_INTFm, l3_if_entry_p,
                                DSCP_MAPPING_PTRf), 0x4,
                        &intf_info->dscp_qos.qos_map_id));
         }
    } 
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_td_l3_routed_int_pri_init
 * Purpose:
 *      Set L3 Routed Internal Priority
 * Parameters:
 *      unit      - (IN)SOC unit number.
 * Returns:
 *      BCM_E_XXX.
 */

int
_bcm_td_l3_routed_int_pri_init (int unit)
{
   int rv=BCM_E_NONE;
   int idx; /* Iteration index. */
   int max_ent_count=0; /* Number of entries to read.*/
   ing_routed_int_pri_mapping_entry_t   int_pri_entry;


    if(soc_mem_is_valid(unit, ING_ROUTED_INT_PRI_MAPPINGm)) {
       max_ent_count = soc_mem_index_count(unit, ING_ROUTED_INT_PRI_MAPPINGm);

       for(idx=0; idx<max_ent_count; idx++) {

            sal_memset(&int_pri_entry, 0, sizeof(ing_routed_int_pri_mapping_entry_t));

            soc_mem_field32_set(unit, ING_ROUTED_INT_PRI_MAPPINGm, &int_pri_entry,
                       NEW_INT_PRIf, idx);

            rv = soc_mem_write(unit, ING_ROUTED_INT_PRI_MAPPINGm,
                       MEM_BLOCK_ALL, idx, &int_pri_entry);

            if (rv < 0) {
                return rv;
            }
       }
    }
    return rv;
}

#else /* BCM_TRIDENT_SUPPORT && INCLUDE_L3 */
int bcm_esw_trident_l3_not_empty;
#endif /* BCM_TRIDENT_SUPPORT && INCLUDE_L3 */
