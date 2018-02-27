/* $Id: field.c 1.169.14.1 Broadcom SDK $
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
 * File:        field.c
 * Purpose:     BCM5660x Field Processor installation functions.
 */
    
#include <soc/defs.h>
#include <soc/mem.h>
#include <soc/feature.h>

#if defined(BCM_EASYRIDER_SUPPORT) && defined(BCM_FIELD_SUPPORT)
#include <soc/easyrider.h>

#include <bcm/error.h>
#include <bcm/field.h>
#include <bcm/mirror.h>

#include <bcm_int/esw/firebolt.h>
#include <bcm_int/esw/easyrider.h>
#include <bcm_int/esw/field.h>
#include <bcm_int/esw/mirror.h>
#include <bcm_int/esw_dispatch.h>


/* FPF0 excludes the two valid bits. */
#define _FIELD_ER_FPF0_BITS(_u_) \
                       (soc_mem_field_length(_u_, FP_INTERNALm, F0_MASKf))
#define _FIELD_ER_FPF1_BITS(_u_) \
                       (soc_mem_field_length(_u_, FP_INTERNALm, F1f))
#define _FIELD_ER_FPF2_BITS   128
#define _FIELD_ER_FPF3_BITS    16

/* local/static function prototypes */
STATIC void _field_er_functions_init(_field_funct_t *functions);
STATIC int _field_er_detach(int unit, _field_control_t *fc);

STATIC int _field_er_udf_spec_set(int unit, bcm_field_udf_spec_t *udf_spec, 
                               uint32 flags, uint32 offset);
STATIC int _field_er_udf_spec_get(int unit, bcm_field_udf_spec_t *udf_spec, 
                                  uint32 *flags, uint32 *offset);

STATIC int _field_er_udf_write(int unit, bcm_field_udf_spec_t *udf_spec, 
                               uint32 udf_num, uint32 user_num);
STATIC int _field_er_udf_read(int unit, bcm_field_udf_spec_t *udf_spec, 
                              uint32 udf_num, uint32 user_num);
#ifdef BROADCOM_DEBUG
STATIC char *_field_er_action_name(bcm_field_action_t action);
#endif /* BROADCOM_DEBUG */
STATIC int _field_er_fpf0_init(int unit, _field_stage_t *stage_fc);
STATIC int _field_er_fpf1_init(int unit, _field_stage_t *stage_fc);
STATIC int _field_er_fpf2_init(int unit, _field_stage_t *stage_fc);
STATIC int _field_er_fpf3_init(int unit, _field_stage_t *stage_fc);
STATIC int _field_er_extn_init(int unit, _field_stage_t *stage_fc);

STATIC int _field_er_meter_init(int unit);

STATIC int _field_er_selcode_get(int unit, _field_stage_t *stage_fc, 
                                 bcm_field_qset_t *qset,
                                 _field_group_t *fg);
STATIC int _field_er_selcode_to_qset(int unit, _field_stage_t *stage_fc, 
                                     _field_group_t *fg, int code_id,
                                     bcm_field_qset_t *qset);
STATIC int _field_er_qual_list_get(int unit, _field_stage_t *stage_fc, 
                                    _field_group_t *group);
STATIC int _field_er_tcam_policy_clear(int unit, _field_stage_id_t stage_id,
                                       int tcam_idx);
STATIC int _field_er_tcam_policy_install(int unit, _field_entry_t *f_ent,
                                         int tcam_idx);
STATIC int _field_er_tcam_get(int unit, _field_entry_t *f_ent,
                              fp_internal_entry_t *tpp_entry);
STATIC int _field_er_action_get(int unit, _field_entry_t *f_ent, int tcam_idx,
                                _field_action_t *fa, void *tpp_entry);
STATIC int _field_er_meter_action_set(int unit, _field_entry_t *f_ent,
                                      void *tpp_entry);
STATIC int _field_er_group_install(int unit, _field_group_t *fg);
STATIC int _field_er_selcodes_install_int(int unit, _field_group_t *fg);
STATIC int _field_er_entry_move(int unit, _field_entry_t *f_ent, int parts_count, 
                     int *tcam_idx_old, int *tcam_idx_new);

STATIC int _field_er_hw_clear(int unit, _field_stage_t *stage_fc);
STATIC int _field_er_entry_remove(int unit, _field_entry_t *f_ent,
                                  int tcam_idx);
STATIC int _field_er_bucket_calc(int unit, uint32 burst, 
                                 uint32 *bucket_size, uint32 *bucket_count);

STATIC int _field_er_meter_read(int unit, int idx, soc_mem_t mem,
                                uint32 *entry);
STATIC int _field_er_meter_write(int unit, int idx, soc_mem_t mem,
                                 uint32 *entry);
STATIC int _field_er_policer_install(int unit, _field_entry_t *f_ent, 
                                     _field_policer_t *f_pl);
STATIC int _field_er_action_params_check(int unit,_field_entry_t *f_ent, 
                                         _field_action_t *fa);
#ifdef BCM_WARM_BOOT_SUPPORT

STATIC int _er_field_tbl[17][4] = {
                    {SLICE0_F0f, SLICE0_F1f, SLICE0_F2f, SLICE0_F3f},
                    {SLICE1_F0f, SLICE1_F1f, SLICE1_F2f, SLICE1_F3f},
                    {SLICE2_F0f, SLICE2_F1f, SLICE2_F2f, SLICE2_F3f},
                    {SLICE3_F0f, SLICE3_F1f, SLICE3_F2f, SLICE3_F3f},
                    {SLICE4_F0f, SLICE4_F1f, SLICE4_F2f, SLICE4_F3f},
                    {SLICE5_F0f, SLICE5_F1f, SLICE5_F2f, SLICE5_F3f},
                    {SLICE6_F0f, SLICE6_F1f, SLICE6_F2f, SLICE6_F3f},
                    {SLICE7_F0f, SLICE7_F1f, SLICE7_F2f, SLICE7_F3f},
                    {SLICE8_F0f, SLICE8_F1f, SLICE8_F2f, SLICE8_F3f},
                    {SLICE9_F0f, SLICE9_F1f, SLICE9_F2f, SLICE9_F3f},
                    {SLICE10_F0f, SLICE10_F1f, SLICE10_F2f, SLICE10_F3f},
                    {SLICE11_F0f, SLICE11_F1f, SLICE11_F2f, SLICE11_F3f},
                    {SLICE12_F0f, SLICE12_F1f, SLICE12_F2f, SLICE12_F3f},
                    {SLICE13_F0f, SLICE13_F1f, SLICE13_F2f, SLICE13_F3f},
                    {SLICE14_F0f, SLICE14_F1f, SLICE14_F2f, SLICE14_F3f},
                    {SLICE15_F0f, SLICE15_F1f, SLICE15_F2f, SLICE15_F3f},
		    {0, 0, SLICE16_F2f, SLICE16_F3f}};	

STATIC int
_field_er_slice_reinit(int unit, fp_port_field_sel_entry_t *pfs_entry, 
		       int slice_numb, _field_sel_t *selcodes)
{
    soc_field_t f0_field, f1_field, f2_field, f3_field;	
    if (slice_numb <= 15) {	    
	f0_field = _er_field_tbl[slice_numb][0];
	selcodes->fpf0 = (int8) soc_FP_PORT_FIELD_SELm_field32_get
					(unit, pfs_entry, f0_field);

        f1_field = _er_field_tbl[slice_numb][1];
	selcodes->fpf1 = (int8) soc_FP_PORT_FIELD_SELm_field32_get
			            	(unit, pfs_entry, f1_field);

        f2_field = _er_field_tbl[slice_numb][2];
        selcodes->fpf2 = (int8) soc_FP_PORT_FIELD_SELm_field32_get
			            	(unit, pfs_entry, f2_field);

        f3_field = _er_field_tbl[slice_numb][3];
        selcodes->fpf3 = (int8) soc_FP_PORT_FIELD_SELm_field32_get
			            	(unit, pfs_entry, f3_field);		   
    } else {
        f2_field = _er_field_tbl[slice_numb][2];
        selcodes->fpf2 = (int8) soc_FP_PORT_FIELD_SELm_field32_get
			            	(unit, pfs_entry, f2_field);

	f3_field = _er_field_tbl[slice_numb][3];
        selcodes->fpf3 = (int8) soc_FP_PORT_FIELD_SELm_field32_get
			            	(unit, pfs_entry, f3_field);
    }
    return BCM_E_NONE;    
}
#else
#define _field_er_slice_reinit (NULL) 
#endif /* BCM_WARM_BOOT_SUPPORT */

/*
 * Function:
 *     _bcm_field_er_init
 *
 * Purpose:
 *     Perform initializations that are specific to BCM5660x. This
 *     includes initializing the FP field select bit offset tables for FPF[1-3]
 *     and initializing the device specific function pointers.
 * Parameters:
 *     unit    - BCM device number
 *     control - FP control struct
 * Returns:
 *     BCM_E_NONE
 * Notes:
 */
int
_bcm_field_er_init(int unit, _field_control_t *fc)
{
    _field_stage_t      *stage_fc;
 
    if (NULL == fc) {
        return (BCM_E_PARAM);
    }

    stage_fc = fc->stages;
    if (NULL == stage_fc ||
        _BCM_FIELD_STAGE_INGRESS != stage_fc->stage_id) {
        return (BCM_E_PARAM);
    }
    
    /* Clear the hardware tables */
    BCM_IF_ERROR_RETURN(_field_er_hw_clear(unit, stage_fc));

    /* Initialize Qsets, bit offsets and width tables. */
    BCM_IF_ERROR_RETURN(_field_er_fpf0_init(unit, stage_fc));
    BCM_IF_ERROR_RETURN(_field_er_fpf1_init(unit, stage_fc));
    BCM_IF_ERROR_RETURN(_field_er_fpf2_init(unit, stage_fc));
    BCM_IF_ERROR_RETURN(_field_er_fpf3_init(unit, stage_fc));
    BCM_IF_ERROR_RETURN(_field_er_extn_init(unit, stage_fc));

    if (!SOC_WARM_BOOT(unit)) {
        /* Set the Filter Enable flags in the port table */
        BCM_IF_ERROR_RETURN(_field_port_filter_enable_set(unit, fc, TRUE));
    }
     
    /* Initialize the function pointers */
    _field_er_functions_init(&fc->functions);

    BCM_IF_ERROR_RETURN(_field_er_meter_init(unit));

    return (BCM_E_NONE);
}

STATIC int
_field_er_hw_clear(int unit, _field_stage_t *stage_fc)
{
    uint32      reg_values[] = {1, 1};
    soc_field_t reg_fields[] = {L2_HDR_ON_SIP_ENf, L2_HDR_ON_DIP_ENf};

    COMPILER_REFERENCE(stage_fc);

    if (SOC_WARM_BOOT(unit)) {
        return (BCM_E_NONE);
    }

    /* Clear the hardware tables used exclusively by FP */
    SOC_IF_ERROR_RETURN
        (soc_mem_clear(unit, UDF_OFFSETm, COPYNO_ALL, TRUE)); 
    SOC_IF_ERROR_RETURN
        (soc_mem_clear(unit, FP_PORT_FIELD_SELm, COPYNO_ALL, TRUE)); 
    if (SOC_MEM_IS_VALID(unit, IFP_PORT_FIELD_SELm)) {             
        SOC_IF_ERROR_RETURN
            (soc_mem_clear(unit, IFP_PORT_FIELD_SELm, COPYNO_ALL, TRUE));
    }                                                              
    SOC_IF_ERROR_RETURN
        (soc_mem_clear(unit, FP_RANGE_CHECKm, COPYNO_ALL, TRUE)); 
    SOC_IF_ERROR_RETURN
        (soc_mem_clear(unit, FP_TCP_UDP_PORT_RANGEm, COPYNO_ALL, TRUE)); 
    SOC_IF_ERROR_RETURN
        (soc_mem_clear(unit, FP_INTERNALm, COPYNO_ALL, TRUE)); 
    SOC_IF_ERROR_RETURN
        (soc_mem_clear(unit, FP_METER_TABLE_INTm, COPYNO_ALL, TRUE)); 
    SOC_IF_ERROR_RETURN
        (soc_mem_clear(unit, FP_COUNTER_INTm, COPYNO_ALL, TRUE)); 

    /* If unit uses SIP & DIP fields for non-IP packets, enable feature. */
    if (soc_feature(unit, soc_feature_field_qual_IpType)) {
        BCM_IF_ERROR_RETURN(soc_reg_fields32_modify(unit, UDF_CONFIGr, REG_PORT_ANY,
                                                    COUNTOF(reg_fields),
                                                    reg_fields, reg_values));
    }
    return (BCM_E_NONE);
}

#define BCM_FIELD_IPTYPE_BAD 0xff
/*
 * Function:
 *     _field_er_qualify_ip_type
 * Purpose:
 *     Install ip type qualifier into TCAM
 * Parameters:
 *     unit  - (IN) BCM device number
 *     f_ent - (IN) Field entry qualifier
 *     type  - (IN) Ip Type. 
 * Returns:
 *     BCM_E_XXX
 * Notes:
 */
STATIC int
_field_er_qualify_ip_type(int unit, _field_entry_t *f_ent, 
                          bcm_field_IpType_t type) 
{
    uint32  data = BCM_FIELD_IPTYPE_BAD,
            mask = BCM_FIELD_IPTYPE_BAD;

    if (NULL == f_ent) {
        return (BCM_E_PARAM);
    }

    /*
     * Devices with a separate PacketFormat and IpType use
     * different hardware encodings for IpType. 
     */
    if (soc_feature(unit, soc_feature_field_qual_IpType)) {
            switch (type) {
            case bcmFieldIpTypeAny:
                data = 0x0;
                mask = 0x0;
                break;
            case bcmFieldIpTypeNonIp:
                data = 0x0;
                mask = 0x3;
                break;
            case bcmFieldIpTypeIpv4Not:
                data = 0x0;
                mask = 0x1;
                break;
            case bcmFieldIpTypeIpv4NoOpts:
                data = 0x1;
                mask = 0x3;
                break;
            case bcmFieldIpTypeIpv4WithOpts:
                data = 0x3;
                mask = 0x3;
                break;
            case bcmFieldIpTypeIpv4Any:
                data = 0x1;
                mask = 0x1;
                break;
            case bcmFieldIpTypeIpv6:
                data = 0x2;
                mask = 0x3;
                break;
            default:
                break;
            }
    } else { /* Devices without separate IpType field. */
        switch (type) {
            case bcmFieldIpTypeAny:
                data = 0x0;
                mask = 0x0;
                break;
            case bcmFieldIpTypeNonIp:
                data = 0x0;
                mask = 0x3;
                break;
            case bcmFieldIpTypeIpv4Not:
                data = 0x0;
                mask = 0x1;
                break;
           case bcmFieldIpTypeIpv4Any:
                data = 0x1;
                mask = 0x3;
                break;
            case bcmFieldIpTypeIpv6:
                data = 0x2;
                mask = 0x3;
                break;
            case bcmFieldIpTypeIpv6Not:
                data = 0x0;
                mask = 0x2;
                break;
            default:
                break;
        }
    }

    if ((data == BCM_FIELD_IPTYPE_BAD) ||
        (mask == BCM_FIELD_IPTYPE_BAD)) {
        return (BCM_E_UNAVAIL);
    }
    return _field_qualify32(unit, f_ent->eid, 
                            bcmFieldQualifyIpType, data, mask);
}
#undef BCM_FIELD_IPTYPE_BAD
/*
 * Function:
 *     _field_er_qualify_ip_type_get
 * Purpose:
 *     Read ip type qualifier match criteria from the HW.
 * Parameters:
 *     unit  - (IN) BCM device number
 *     f_ent - (IN) Field entry qualifier
 *     type  - (OUT) Ip Type. 
 * Returns:
 *     BCM_E_XXX
 * Notes:
 */
STATIC int
_field_er_qualify_ip_type_get(int unit, _field_entry_t *f_ent, 
                              bcm_field_IpType_t *type) 
{
    uint32 hw_data;          /* HW encoded qualifier data.  */
    uint32 hw_mask;          /* HW encoding qualifier mask. */
    int rv;                  /* Operation return status.    */

    /* Input parameters checks. */
    if ((NULL == f_ent) || (NULL == type)) {
        return (BCM_E_PARAM);
    }

    /* Read qualifier match value and mask. */
    rv = _bcm_field_entry_qualifier_uint32_get(unit, f_ent->eid,
                                               bcmFieldQualifyIpType,
                                               &hw_data, &hw_mask);
    BCM_IF_ERROR_RETURN(rv);

    /*
     * Devices with a separate PacketFormat and IpType use
     * different hardware encodings for IpType. 
     */
    if (soc_feature(unit, soc_feature_field_qual_IpType)) {
        if ((hw_data == 0) && (hw_mask == 0)) {
            *type = bcmFieldIpTypeAny;
        } else if ((hw_data == 0) && (hw_mask == 3)) {
            *type = bcmFieldIpTypeNonIp;
        } else if ((hw_data == 0) && (hw_mask == 1)) {
            *type = bcmFieldIpTypeIpv4Not;
        } else if ((hw_data == 1) && (hw_mask == 3)) {
            *type = bcmFieldIpTypeIpv4NoOpts;
        } else if ((hw_data == 3) && (hw_mask == 3)) {
            *type = bcmFieldIpTypeIpv4WithOpts;
        } else if ((hw_data == 1) && (hw_mask == 1)) {
            *type  = bcmFieldIpTypeIpv4Any;
        } else if ((hw_data == 2) && (hw_mask == 3)) {
            *type = bcmFieldIpTypeIpv6;
        } else {
            return (BCM_E_INTERNAL);
        }
    } else { /* Devices without separate IpType field. */
        if ((hw_data == 0) && (hw_mask == 0)) {
            *type = bcmFieldIpTypeAny;
        } else if ((hw_data == 0) && (hw_mask == 3)) {
            *type = bcmFieldIpTypeNonIp;
        } else if ((hw_data == 0) && (hw_mask == 1)) {
            *type = bcmFieldIpTypeIpv4Not;
        } else if ((hw_data == 1) && (hw_mask == 3)) {
            *type = bcmFieldIpTypeIpv4Any;
        } else if ((hw_data == 2) && (hw_mask == 3)) {
            *type = bcmFieldIpTypeIpv6;
        } else if ((hw_data == 0) && (hw_mask == 2)) {
            *type = bcmFieldIpTypeIpv6Not;
        } else {
            return (BCM_E_INTERNAL);
        }
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *     _field_er_action_support_check
 *
 * Purpose:
 *     Check if action is supported by device.
 *
 * Parameters:
 *     unit   -(IN)BCM device number
 *     f_ent  -(IN)Field entry structure.  
 *     action -(IN) Action to check(bcmFieldActionXXX)
 *     result -(OUT)
 *               TRUE   - action is supported by device
 *               FALSE  - action is NOT supported by device
 *
 * Returns:
 *     BCM_E_XXX   
 */
STATIC int
_field_er_action_support_check(int unit, _field_entry_t *f_ent,
                               bcm_field_action_t action, int *result)
{
    /* Input parameters check */
    if ((NULL == f_ent) || (NULL == result)) {
        return (BCM_E_PARAM);
    }

    switch (action) {
      case bcmFieldActionPrioPktAndIntCopy:
      case bcmFieldActionPrioPktAndIntNew:
      case bcmFieldActionPrioPktAndIntTos:
      case bcmFieldActionPrioPktAndIntCancel:
      case bcmFieldActionPrioPktCopy:
      case bcmFieldActionPrioPktNew:
      case bcmFieldActionPrioPktTos:
      case bcmFieldActionPrioPktCancel:
      case bcmFieldActionPrioIntCopy:
      case bcmFieldActionPrioIntNew:
      case bcmFieldActionPrioIntTos:
      case bcmFieldActionPrioIntCancel:
      case bcmFieldActionTosNew:
      case bcmFieldActionTosCopy:
      case bcmFieldActionTosCancel:
      case bcmFieldActionDscpNew:
      case bcmFieldActionDscpCancel:
      case bcmFieldActionCopyToCpu:
      case bcmFieldActionCopyToCpuCancel:
      case bcmFieldActionSwitchToCpuCancel:
      case bcmFieldActionRedirect:
      case bcmFieldActionRedirectTrunk:
      case bcmFieldActionRedirectCancel:
      case bcmFieldActionRedirectPbmp:
      case bcmFieldActionEgressMask:
      case bcmFieldActionDrop:
      case bcmFieldActionDropCancel:
      case bcmFieldActionMirrorIngress:
      case bcmFieldActionMirrorEgress:
      case bcmFieldActionL3ChangeVlan:
      case bcmFieldActionL3ChangeVlanCancel:
      case bcmFieldActionL3ChangeMacDa:
      case bcmFieldActionL3ChangeMacDaCancel:
      case bcmFieldActionL3Switch:
      case bcmFieldActionL3SwitchCancel:
      case bcmFieldActionAddClassTag:
      case bcmFieldActionDropPrecedence:
      case bcmFieldActionRpDrop:
      case bcmFieldActionRpDropCancel:
      case bcmFieldActionRpDropPrecedence:
      case bcmFieldActionRpCopyToCpu:
      case bcmFieldActionRpCopyToCpuCancel:
      case bcmFieldActionRpDscpNew:
      case bcmFieldActionYpDrop:
      case bcmFieldActionYpDropCancel:
      case bcmFieldActionYpDropPrecedence:
      case bcmFieldActionYpCopyToCpu:
      case bcmFieldActionYpCopyToCpuCancel:
      case bcmFieldActionYpDscpNew:
      case bcmFieldActionUpdateCounter:
      case bcmFieldActionMeterConfig:
      case bcmFieldActionGpDrop:
      case bcmFieldActionGpDropCancel:
      case bcmFieldActionGpDropPrecedence:
      case bcmFieldActionGpCopyToCpu:
      case bcmFieldActionGpCopyToCpuCancel:
      case bcmFieldActionGpDscpNew:
          *result = TRUE;
          break;
      default:
          *result = FALSE;
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *     _field_er_action_conflict_check
 *
 * Purpose:
 *     Check if two action conflict (occupy the same field in FP policy table)
 *
 * Parameters:
 *     unit    -(IN)BCM device number
 *     f_ent   -(IN)Field entry structure.  
 *     action -(IN) Action to check(bcmFieldActionXXX)
 *     action1 -(IN) Action to check(bcmFieldActionXXX)
 *
 * Returns:
 *     BCM_E_CONFIG - if actions do conflict
 *     BCM_E_NONE   - if there is no conflict
 */
STATIC int
_field_er_action_conflict_check(int unit, _field_entry_t *f_ent,
                                 bcm_field_action_t action1,
                                 bcm_field_action_t action)
{
    /* Input parameters check */
    if (NULL == f_ent) {
        return (BCM_E_PARAM);
    }

    /* Two identical actions are forbidden. */
    _FP_ACTIONS_CONFLICT(action1);

    switch (action1) {
      case bcmFieldActionPrioPktAndIntCopy:
      case bcmFieldActionPrioPktAndIntNew:
      case bcmFieldActionPrioPktAndIntTos:
      case bcmFieldActionPrioPktAndIntCancel:
      case bcmFieldActionPrioPktCopy:
      case bcmFieldActionPrioPktNew:
      case bcmFieldActionPrioPktTos:
      case bcmFieldActionPrioPktCancel:
      case bcmFieldActionPrioIntCopy:
      case bcmFieldActionPrioIntNew:
      case bcmFieldActionPrioIntTos:
      case bcmFieldActionPrioIntCancel:
          _FP_ACTIONS_CONFLICT(bcmFieldActionPrioPktAndIntCopy);
          _FP_ACTIONS_CONFLICT(bcmFieldActionPrioPktAndIntNew);
          _FP_ACTIONS_CONFLICT(bcmFieldActionPrioPktAndIntTos);
          _FP_ACTIONS_CONFLICT(bcmFieldActionPrioPktAndIntCancel);
          _FP_ACTIONS_CONFLICT(bcmFieldActionPrioPktCopy);
          _FP_ACTIONS_CONFLICT(bcmFieldActionPrioPktNew);
          _FP_ACTIONS_CONFLICT(bcmFieldActionPrioPktTos);
          _FP_ACTIONS_CONFLICT(bcmFieldActionPrioPktCancel);
          _FP_ACTIONS_CONFLICT(bcmFieldActionPrioIntCopy);
          _FP_ACTIONS_CONFLICT(bcmFieldActionPrioIntNew);
          _FP_ACTIONS_CONFLICT(bcmFieldActionPrioIntTos);
          _FP_ACTIONS_CONFLICT(bcmFieldActionPrioIntCancel);
          break;
      case bcmFieldActionTosNew:
      case bcmFieldActionTosCopy:
      case bcmFieldActionTosCancel:
          _FP_ACTIONS_CONFLICT(bcmFieldActionTosNew);
          _FP_ACTIONS_CONFLICT(bcmFieldActionTosCopy);
          _FP_ACTIONS_CONFLICT(bcmFieldActionTosCancel);
          _FP_ACTIONS_CONFLICT(bcmFieldActionDscpNew);
          _FP_ACTIONS_CONFLICT(bcmFieldActionDscpCancel);
          _FP_ACTIONS_CONFLICT(bcmFieldActionRpDscpNew);
          _FP_ACTIONS_CONFLICT(bcmFieldActionYpDscpNew);
          _FP_ACTIONS_CONFLICT(bcmFieldActionGpDscpNew);
          break;
      case bcmFieldActionDscpNew:
      case bcmFieldActionDscpCancel:
          _FP_ACTIONS_CONFLICT(bcmFieldActionTosNew);
          _FP_ACTIONS_CONFLICT(bcmFieldActionTosCopy);
          _FP_ACTIONS_CONFLICT(bcmFieldActionTosCancel);
          _FP_ACTIONS_CONFLICT(bcmFieldActionDscpNew);
          _FP_ACTIONS_CONFLICT(bcmFieldActionDscpCancel);
          _FP_ACTIONS_CONFLICT(bcmFieldActionRpDscpNew);
          _FP_ACTIONS_CONFLICT(bcmFieldActionYpDscpNew);
          _FP_ACTIONS_CONFLICT(bcmFieldActionGpDscpNew);
          break;
      case bcmFieldActionRpDscpNew:
          _FP_ACTIONS_CONFLICT(bcmFieldActionTosNew);
          _FP_ACTIONS_CONFLICT(bcmFieldActionTosCopy);
          _FP_ACTIONS_CONFLICT(bcmFieldActionTosCancel);
          _FP_ACTIONS_CONFLICT(bcmFieldActionDscpNew);
          _FP_ACTIONS_CONFLICT(bcmFieldActionDscpCancel);
          _FP_ACTIONS_CONFLICT(bcmFieldActionRpDscpNew);
          break;
      case bcmFieldActionYpDscpNew:
          _FP_ACTIONS_CONFLICT(bcmFieldActionTosNew);
          _FP_ACTIONS_CONFLICT(bcmFieldActionTosCopy);
          _FP_ACTIONS_CONFLICT(bcmFieldActionTosCancel);
          _FP_ACTIONS_CONFLICT(bcmFieldActionDscpNew);
          _FP_ACTIONS_CONFLICT(bcmFieldActionDscpCancel);
          _FP_ACTIONS_CONFLICT(bcmFieldActionYpDscpNew);
          break;
      case bcmFieldActionGpDscpNew:
          _FP_ACTIONS_CONFLICT(bcmFieldActionTosNew);
          _FP_ACTIONS_CONFLICT(bcmFieldActionTosCopy);
          _FP_ACTIONS_CONFLICT(bcmFieldActionTosCancel);
          _FP_ACTIONS_CONFLICT(bcmFieldActionDscpNew);
          _FP_ACTIONS_CONFLICT(bcmFieldActionDscpCancel);
          _FP_ACTIONS_CONFLICT(bcmFieldActionGpDscpNew);
          break;
      case bcmFieldActionCopyToCpu:
      case bcmFieldActionCopyToCpuCancel:
      case bcmFieldActionSwitchToCpuCancel:
          _FP_ACTIONS_CONFLICT(bcmFieldActionCopyToCpu);
          _FP_ACTIONS_CONFLICT(bcmFieldActionCopyToCpuCancel);
          _FP_ACTIONS_CONFLICT(bcmFieldActionSwitchToCpuCancel);
          _FP_ACTIONS_CONFLICT(bcmFieldActionRpCopyToCpu);
          _FP_ACTIONS_CONFLICT(bcmFieldActionRpCopyToCpuCancel);
          _FP_ACTIONS_CONFLICT(bcmFieldActionYpCopyToCpu);
          _FP_ACTIONS_CONFLICT(bcmFieldActionYpCopyToCpuCancel);
          _FP_ACTIONS_CONFLICT(bcmFieldActionGpCopyToCpu);
          _FP_ACTIONS_CONFLICT(bcmFieldActionGpCopyToCpuCancel);
          break;
      case bcmFieldActionRpCopyToCpu:
      case bcmFieldActionRpCopyToCpuCancel:
          _FP_ACTIONS_CONFLICT(bcmFieldActionCopyToCpu);
          _FP_ACTIONS_CONFLICT(bcmFieldActionCopyToCpuCancel);
          _FP_ACTIONS_CONFLICT(bcmFieldActionSwitchToCpuCancel);
          _FP_ACTIONS_CONFLICT(bcmFieldActionRpCopyToCpu);
          _FP_ACTIONS_CONFLICT(bcmFieldActionRpCopyToCpuCancel);
          break;
      case bcmFieldActionYpCopyToCpu:
      case bcmFieldActionYpCopyToCpuCancel:
          _FP_ACTIONS_CONFLICT(bcmFieldActionCopyToCpu);
          _FP_ACTIONS_CONFLICT(bcmFieldActionCopyToCpuCancel);
          _FP_ACTIONS_CONFLICT(bcmFieldActionSwitchToCpuCancel);
          _FP_ACTIONS_CONFLICT(bcmFieldActionYpCopyToCpu);
          _FP_ACTIONS_CONFLICT(bcmFieldActionYpCopyToCpuCancel);
          break;
      case bcmFieldActionGpCopyToCpu:
      case bcmFieldActionGpCopyToCpuCancel:
          _FP_ACTIONS_CONFLICT(bcmFieldActionCopyToCpu);
          _FP_ACTIONS_CONFLICT(bcmFieldActionCopyToCpuCancel);
          _FP_ACTIONS_CONFLICT(bcmFieldActionSwitchToCpuCancel);
          _FP_ACTIONS_CONFLICT(bcmFieldActionGpCopyToCpu);
          _FP_ACTIONS_CONFLICT(bcmFieldActionGpCopyToCpuCancel);
          break;
      case bcmFieldActionRedirect:
      case bcmFieldActionRedirectTrunk:
      case bcmFieldActionRedirectCancel:
      case bcmFieldActionRedirectPbmp:
      case bcmFieldActionEgressMask:
          _FP_ACTIONS_CONFLICT(bcmFieldActionRedirect);
          _FP_ACTIONS_CONFLICT(bcmFieldActionRedirectTrunk);
          _FP_ACTIONS_CONFLICT(bcmFieldActionRedirectCancel);
          _FP_ACTIONS_CONFLICT(bcmFieldActionRedirectPbmp);
          _FP_ACTIONS_CONFLICT(bcmFieldActionEgressMask);
          break;
      case bcmFieldActionDrop:
      case bcmFieldActionDropCancel:
          _FP_ACTIONS_CONFLICT(bcmFieldActionDrop);
          _FP_ACTIONS_CONFLICT(bcmFieldActionDropCancel);
          _FP_ACTIONS_CONFLICT(bcmFieldActionRpDrop);
          _FP_ACTIONS_CONFLICT(bcmFieldActionRpDropCancel);
          _FP_ACTIONS_CONFLICT(bcmFieldActionYpDrop);
          _FP_ACTIONS_CONFLICT(bcmFieldActionYpDropCancel);
          _FP_ACTIONS_CONFLICT(bcmFieldActionGpDrop);
          _FP_ACTIONS_CONFLICT(bcmFieldActionGpDropCancel);
          break;
      case bcmFieldActionRpDrop:
      case bcmFieldActionRpDropCancel:
          _FP_ACTIONS_CONFLICT(bcmFieldActionDrop);
          _FP_ACTIONS_CONFLICT(bcmFieldActionDropCancel);
          _FP_ACTIONS_CONFLICT(bcmFieldActionRpDrop);
          _FP_ACTIONS_CONFLICT(bcmFieldActionRpDropCancel);
          break;
      case bcmFieldActionYpDrop:
      case bcmFieldActionYpDropCancel:
          _FP_ACTIONS_CONFLICT(bcmFieldActionDrop);
          _FP_ACTIONS_CONFLICT(bcmFieldActionDropCancel);
          _FP_ACTIONS_CONFLICT(bcmFieldActionYpDrop);
          _FP_ACTIONS_CONFLICT(bcmFieldActionYpDropCancel);
          break;
      case bcmFieldActionGpDrop:
      case bcmFieldActionGpDropCancel:
          _FP_ACTIONS_CONFLICT(bcmFieldActionDrop);
          _FP_ACTIONS_CONFLICT(bcmFieldActionDropCancel);
          _FP_ACTIONS_CONFLICT(bcmFieldActionGpDrop);
          _FP_ACTIONS_CONFLICT(bcmFieldActionGpDropCancel);
          break;
      case bcmFieldActionL3ChangeVlan:
      case bcmFieldActionL3ChangeVlanCancel:
      case bcmFieldActionL3ChangeMacDa:
      case bcmFieldActionL3ChangeMacDaCancel:
      case bcmFieldActionL3Switch:
      case bcmFieldActionL3SwitchCancel:
      case bcmFieldActionAddClassTag:
          _FP_ACTIONS_CONFLICT(bcmFieldActionAddClassTag);
          _FP_ACTIONS_CONFLICT(bcmFieldActionL3ChangeVlan);
          _FP_ACTIONS_CONFLICT(bcmFieldActionL3ChangeVlanCancel);
          _FP_ACTIONS_CONFLICT(bcmFieldActionL3ChangeMacDa);
          _FP_ACTIONS_CONFLICT(bcmFieldActionL3ChangeMacDaCancel);
          _FP_ACTIONS_CONFLICT(bcmFieldActionL3Switch);
          _FP_ACTIONS_CONFLICT(bcmFieldActionL3SwitchCancel);
          break;
      case bcmFieldActionDropPrecedence:
          _FP_ACTIONS_CONFLICT(bcmFieldActionRpDropPrecedence);
          _FP_ACTIONS_CONFLICT(bcmFieldActionYpDropPrecedence);
          _FP_ACTIONS_CONFLICT(bcmFieldActionGpDropPrecedence);
          break;
      case bcmFieldActionRpDropPrecedence:
          _FP_ACTIONS_CONFLICT(bcmFieldActionDropPrecedence);
          _FP_ACTIONS_CONFLICT(bcmFieldActionRpDropPrecedence);
          break;
      case bcmFieldActionYpDropPrecedence:
          _FP_ACTIONS_CONFLICT(bcmFieldActionDropPrecedence);
          _FP_ACTIONS_CONFLICT(bcmFieldActionYpDropPrecedence);
          break;
      case bcmFieldActionGpDropPrecedence:
          _FP_ACTIONS_CONFLICT(bcmFieldActionDropPrecedence);
          _FP_ACTIONS_CONFLICT(bcmFieldActionGpDropPrecedence);
          break;
      case bcmFieldActionMirrorIngress:
      case bcmFieldActionMirrorEgress:
      case bcmFieldActionUpdateCounter:
      case bcmFieldActionMeterConfig:
          break;
      default:
          break;
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *     _field_er_functions_init
 *
 * Purpose:
 *     Set up functions pointers 
 *
 * Parameters:
 *     functions - pointers to device specific functions
 *
 * Returns:
 *     nothing
 * Notes:
 */
STATIC void
_field_er_functions_init(_field_funct_t *functions) 
{
    assert(functions != NULL);

    functions->fp_detach              = _field_er_detach;
    functions->fp_udf_spec_set        = _field_er_udf_spec_set;
    functions->fp_udf_spec_get        = _field_er_udf_spec_get;
    functions->fp_udf_delete          = _field_er_udf_write;
    functions->fp_udf_write           = _field_er_udf_write;
    functions->fp_udf_read            = _field_er_udf_read;
    functions->fp_group_install       = _field_er_group_install;
    functions->fp_selcodes_install    = NULL;
    functions->fp_slice_clear         = NULL;
    functions->fp_entry_remove        = _field_er_entry_remove;
    functions->fp_entry_move          = _field_er_entry_move;
    functions->fp_selcode_get         = _field_er_selcode_get;
    functions->fp_selcode_to_qset     = _field_er_selcode_to_qset;
    functions->fp_qual_list_get       = _field_er_qual_list_get;
    functions->fp_tcam_policy_clear   = _field_er_tcam_policy_clear;
    functions->fp_tcam_policy_install = _field_er_tcam_policy_install;
    functions->fp_tcam_policy_reinstall = _field_er_tcam_policy_install;
    functions->fp_policer_install     = _field_er_policer_install;
    functions->fp_slice_reinit	      = _field_er_slice_reinit;
    functions->fp_action_params_check = _field_er_action_params_check;
    functions->fp_write_slice_map     = NULL;
    functions->fp_qualify_ip_type     = _field_er_qualify_ip_type;
    functions->fp_qualify_ip_type_get = _field_er_qualify_ip_type_get;
    functions->fp_action_support_check = _field_er_action_support_check;
    functions->fp_action_conflict_check = _field_er_action_conflict_check;
    functions->fp_counter_get          = _bcm_field_fb_counter_get;
    functions->fp_counter_set          = _bcm_field_fb_counter_set;
    functions->fp_stat_index_get       = _bcm_field_fb_stat_index_get;
    functions->fp_egress_key_match_type_set = NULL;
    functions->fp_external_entry_install = NULL;
    functions->fp_external_entry_reinstall = NULL;
    functions->fp_external_entry_remove = NULL;
    functions->fp_external_entry_prio_set = NULL;
    functions->fp_data_qualifier_ethertype_add =
        _bcm_field_er_data_qualifier_ethertype_add;
    functions->fp_data_qualifier_ethertype_delete=
        _bcm_field_er_data_qualifier_ethertype_delete;
    functions->fp_data_qualifier_ip_protocol_add =
        _bcm_field_er_data_qualifier_ip_protocol_add;
    functions->fp_data_qualifier_ip_protocol_delete=
        _bcm_field_er_data_qualifier_ip_protocol_delete;
    functions->fp_data_qualifier_packet_format_add=
        _bcm_field_er_data_qualifier_packet_format_add;
    functions->fp_data_qualifier_packet_format_delete=
        _bcm_field_er_data_qualifier_packet_format_delete;
}

/*  
 * Function:
 *      _bcm_field_er_data_qualifier_ethertype_add
 * Purpose:
 *      Add ethertype based offset to data qualifier object.
 * Parameters:
 *      unit       - (IN) bcm device.
 *      qual_id    - (IN) Data qualifier id.
 *      etype      - (IN) Ethertype based offset specification.
 * Returns:
 *      BCM_E_XXX
 */
int 
_bcm_field_er_data_qualifier_ethertype_add(int unit,  int qual_id,
                                 bcm_field_data_ethertype_t *etype)
{ 
    return (BCM_E_UNAVAIL);

}

/*  
 * Function:
 *      _bcm_field_er_data_qualifier_ethertype_delete
 * Purpose:
 *      Remove ethertype based offset from data qualifier object.
 * Parameters:
 *      unit       - (IN) bcm device.
 *      qual_id    - (IN) Data qualifier id.
 *      etype      - (IN) Ethertype based offset specification.
 * Returns:
 *      BCM_E_XXX
 */ 
int 
_bcm_field_er_data_qualifier_ethertype_delete(int unit, int qual_id,
                                 bcm_field_data_ethertype_t *etype)
{
    return (BCM_E_UNAVAIL);
}

/*  
 * Function:
 *      _bcm_field_er_data_qualifier_ip_protocol_add
 * Purpose:
 *      Add ipprotocol based offset to data qualifier object.
 * Parameters:
 *      unit       - (IN) bcm device.
 *      qual_id    - (IN) Data qualifier id.
 *      ip_proto   - (IN) Ip protocol based offset specification.
 * Returns:
 *      BCM_E_XXX
 */ 
int 
_bcm_field_er_data_qualifier_ip_protocol_add(int unit,  int qual_id,
                                 bcm_field_data_ip_protocol_t *ip_proto)
{
    return (BCM_E_UNAVAIL);
}

/*  
 * Function:
 *      _bcm_field_er_data_qualifier_ip_protocol_delete
 * Purpose:
 *      Remove ipprotocol based offset from data qualifier object.
 * Parameters:
 *      unit       - (IN) bcm device.
 *      qual_id    - (IN) Data qualifier id.
 *      ip_proto   - (IN) Ip protocol based offset specification.
 * Returns:
 *      BCM_E_XXX
 */ 
int 
_bcm_field_er_data_qualifier_ip_protocol_delete(int unit, int qual_id,
                                 bcm_field_data_ip_protocol_t *ip_proto)
{
    return (BCM_E_UNAVAIL);
}

/*  
 * Function:
 *      _bcm_field_er_data_qualifier_packet_format_add
 * Purpose:
 *      Add ipprotocol based offset to data qualifier object.
 * Parameters:
 *      unit       - (IN) bcm device.
 *      qual_id    - (IN) Data qualifier id.
 *      pkt_format - (IN) Packet format based offset specification.
 * Returns:
 *      BCM_E_XXX
 */ 
int 
_bcm_field_er_data_qualifier_packet_format_add(int unit,  int qual_id,
                                 bcm_field_data_packet_format_t *pkt_format)
{
    return (BCM_E_UNAVAIL);
}

/*  
 * Function:
 *      _bcm_field_er_data_qualifier_packet_format_delete
 * Purpose:
 *      Remove ipprotocol based offset from data qualifier object.
 * Parameters:
 *      unit       - (IN) bcm device.
 *      qual_id    - (IN) Data qualifier id.
 *      pkt_format - (IN) Packet format based udf offset specification.
 * Returns:
 *      BCM_E_XXX
 */ 
int 
_bcm_field_er_data_qualifier_packet_format_delete(int unit, int qual_id,
                                 bcm_field_data_packet_format_t *pkt_format)
{
    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *     _field_er_detach
 * Purpose:
 *     Deallocates unit's tables.
 * Parameters:
 *     unit - BCM device number
 * Returns:
 *     BCM_E_NONE
 * Notes:
 */
STATIC int
_field_er_detach(int unit, _field_control_t *fc) 
{
   _field_stage_t   *stage_p;

    if (NULL == fc) {
        return BCM_E_PARAM;
    }

    stage_p = fc->stages;

    if (NULL == stage_p ||
        _BCM_FIELD_STAGE_INGRESS != stage_p->stage_id) {
        return BCM_E_PARAM;
    }

    /* Destroy select value tables. */
    FP_VVERB(("FP(unit %d) vverb: BEGIN _field_er_detach(unit=%d) freeing select tables\n", unit));

    /* Clear the hardware tables */
    BCM_IF_ERROR_RETURN(_field_er_hw_clear(unit, stage_p));

    /* Clear the Filter Enable flags in the port table */
    BCM_IF_ERROR_RETURN(_field_port_filter_enable_set(unit, fc, FALSE));
    FP_VVERB(("FP(unit %d) vverb: END _field_er_detach()\n", unit));

    return BCM_E_NONE;
}

/*
 * Function:
 *     bcm_er_field_range
 * Purpose:
 *     Write FP_TCP_UDP_PORT_RANGE 
 * Parameters:
 *     
 * Returns:
 *     BCM_E_PARAM - flags or offset out of range
 *     BCM_E_NONE
 */
int
bcm_er_field_range_check_set(int unit, int range, uint32 flags,
                             bcm_l4_port_t min, bcm_l4_port_t max)
{
    fp_tcp_udp_port_range_entry_t  tbl_entry;
    uint32                         src_chk, invert;

    assert(soc_mem_index_valid(unit, FP_TCP_UDP_PORT_RANGEm, range)); 
    assert(!(flags & BCM_FIELD_RANGE_SRCPORT && 
             flags & BCM_FIELD_RANGE_DSTPORT));

    src_chk = (flags & BCM_FIELD_RANGE_SRCPORT) ? 1 : 0;
    invert  = (flags & BCM_FIELD_RANGE_INVERT)  ? 1 : 0;

    /* read/modify/write range check memory */
    SOC_IF_ERROR_RETURN(READ_FP_TCP_UDP_PORT_RANGEm(unit, MEM_BLOCK_ANY, 
                                             range, &tbl_entry));
    if (flags & BCM_FIELD_RANGE_TCP) {
        soc_FP_TCP_UDP_PORT_RANGEm_field32_set(unit, &tbl_entry, 
                                    TCP_SRCf, src_chk);
        soc_FP_TCP_UDP_PORT_RANGEm_field32_set(unit, &tbl_entry, 
                                    INVERT_TCP_RANGE_RESULTf, invert);
        soc_FP_TCP_UDP_PORT_RANGEm_field32_set(unit, &tbl_entry, 
                                    TCP_MINf, min);
        soc_FP_TCP_UDP_PORT_RANGEm_field32_set(unit, &tbl_entry, 
                                    TCP_MAXf, max);
    }
    if (flags & BCM_FIELD_RANGE_UDP) {
        assert(flags & BCM_FIELD_RANGE_UDP);
        soc_FP_TCP_UDP_PORT_RANGEm_field32_set(unit, &tbl_entry, 
                                    UDP_SRCf, src_chk);
        soc_FP_TCP_UDP_PORT_RANGEm_field32_set(unit, &tbl_entry, 
                                    INVERT_UDP_RANGE_RESULTf, invert);
        soc_FP_TCP_UDP_PORT_RANGEm_field32_set(unit, &tbl_entry, 
                                    UDP_MINf, min);
        soc_FP_TCP_UDP_PORT_RANGEm_field32_set(unit, &tbl_entry, 
                                    UDP_MAXf, max);
    }
    SOC_IF_ERROR_RETURN(WRITE_FP_TCP_UDP_PORT_RANGEm(unit, MEM_BLOCK_ALL, 
                                              range, &tbl_entry));

    return BCM_E_NONE;
}

/*
 * Function:
 *     _field_er_udf_spec_set
 * Purpose:
 *     Write user defined field configuration to udf_spec_t structure
 * Parameters:
 *     
 * Returns:
 *     BCM_E_PARAM - flags or offset out of range
 *     BCM_E_NONE
 */
STATIC int
_field_er_udf_spec_set(int unit, bcm_field_udf_spec_t *udf_spec, 
                       uint32 flags, uint32 offset)
{
    uint8               pkt_fmt_idx = 0u;     /* index into udf_spec */

    if (offset > 31) {
        FP_ERR(("FP(unit %d) Error: offset=%d out-of-range.\n", unit, offset));
        return BCM_E_PARAM;
    }
    else if ((offset >= 29) && (flags & BCM_FIELD_USER_HIGIG) &&
             !soc_feature(unit, soc_feature_field_udf_higig) ) {
        FP_ERR(("FP(unit %d) Error: device does not support UDF HiGig data.\n", unit));
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(_field_trans_flags_to_index(unit, flags, &pkt_fmt_idx));

    udf_spec->offset[pkt_fmt_idx] = offset;
    if (flags & BCM_FIELD_USER_OPTION_ADJUST) {
        udf_spec->offset[pkt_fmt_idx] |= BCM_FIELD_USER_OPTION_ADJUST;
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *     _field_er_udf_spec_get
 * Purpose:
 *     Get user defined field configuration entry from udf_spec_t structure
 * Parameters:
 *     
 * Returns:
 *     BCM_E_PARAM - flags or offset out of range
 *     BCM_E_NONE
 */
STATIC int
_field_er_udf_spec_get(int unit, bcm_field_udf_spec_t *udf_spec, 
                       uint32 *flags, uint32 *offset)
{
    uint8               pkt_fmt_idx = 0u;     /* index into udf_spec */

    BCM_IF_ERROR_RETURN(_field_trans_flags_to_index(unit, *flags,
                                                    &pkt_fmt_idx));

    if ((*offset = udf_spec->offset[pkt_fmt_idx] & ~(BCM_FIELD_USER_OPTION_ADJUST)) > 31) {
        FP_ERR(("FP(unit %d) Error: offset=%d out-of-range.\n", unit, *offset));
        return BCM_E_PARAM;
    }
    if (udf_spec->offset[pkt_fmt_idx] & BCM_FIELD_USER_OPTION_ADJUST) {
        *flags |= BCM_FIELD_USER_OPTION_ADJUST;
    }
    else {
        *flags &= ~(BCM_FIELD_USER_OPTION_ADJUST);
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *     _field_er_udf_write
 * Purpose:
 *     Write the info in udf_spec to hardware
 * Parameters:
 *     
 * Returns:
 *     BCM_E_PARAM - UDF number or User number out of range
 *     BCM_E_NONE
 * Notes:
 *     Calling function should have unit's lock.
 */
STATIC int
_field_er_udf_write(int unit, bcm_field_udf_spec_t *udf_spec, 
                        uint32 udf_num, uint32 user_num)
{
    uint8               tbl_idx = 0u;         /* index into H/W table */
    udf_offset_entry_t  tbl_entry;
    uint8               f_idx;                /* field selector index */ 
    uint32              offset;            

    soc_field_t         off_field[8] = {UDF1_OFFSET0f, UDF1_OFFSET1f, 
                                        UDF1_OFFSET2f, UDF1_OFFSET3f,
                                        UDF2_OFFSET0f, UDF2_OFFSET1f,
                                        UDF2_OFFSET2f, UDF2_OFFSET3f};

    soc_field_t         option_field[8] = {UDF1_ADD_IPV4_OPTIONS0f,
                                           UDF1_ADD_IPV4_OPTIONS1f,
                                           UDF1_ADD_IPV4_OPTIONS2f,
                                           UDF1_ADD_IPV4_OPTIONS3f,
                                           UDF2_ADD_IPV4_OPTIONS0f,
                                           UDF2_ADD_IPV4_OPTIONS1f,
                                           UDF2_ADD_IPV4_OPTIONS2f,
                                           UDF2_ADD_IPV4_OPTIONS3f};

    if (udf_num > 1 || user_num > 3) {
        FP_ERR(("FP(unit %d) Error: udf_num=%d or user_num=%d out-of-range.\n", 
                unit, udf_num, user_num));
        return BCM_E_PARAM;
    }

    /* Formulate the desired UDF_OFFSET fields */
    f_idx = (udf_num << 2) | user_num;

    for (tbl_idx = soc_mem_index_min(unit, UDF_OFFSETm); 
         tbl_idx <= soc_mem_index_max(unit, UDF_OFFSETm);
         tbl_idx++) { 
        offset = udf_spec->offset[tbl_idx] & ~(BCM_FIELD_USER_OPTION_ADJUST);
        if (offset > 31) {
            return BCM_E_PARAM;
        }
        SOC_IF_ERROR_RETURN
          (READ_UDF_OFFSETm(unit, MEM_BLOCK_ANY, tbl_idx, &tbl_entry));
        soc_UDF_OFFSETm_field32_set(unit, &tbl_entry, off_field[f_idx], offset);

        /* Set the UDFn_ADD_IPV4_OPTIONSm bit accordingly */ 
        if (udf_spec->offset[tbl_idx] & BCM_FIELD_USER_OPTION_ADJUST) {
            soc_UDF_OFFSETm_field32_set(unit, &tbl_entry, 
                                        option_field[f_idx], 1);
        }
        else {
            soc_UDF_OFFSETm_field32_set(unit, &tbl_entry, 
                                        option_field[f_idx], 0);
        }

        SOC_IF_ERROR_RETURN
          (WRITE_UDF_OFFSETm(unit, MEM_BLOCK_ALL, tbl_idx, &tbl_entry));
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     _field_er_udf_read
 * Purpose:
 *     Read info into udf_spec from hardware
 * Parameters:
 *     
 * Returns:
 *     BCM_E_PARAM - UDF number or User number out of range
 *     BCM_E_NONE
 * Notes:
 *     Calling function should have unit's lock.
 */
STATIC int
_field_er_udf_read(int unit, bcm_field_udf_spec_t *udf_spec, 
                       uint32 udf_num, uint32 user_num)
{
    uint8               tbl_idx = 0u;         /* index into H/W table */
    udf_offset_entry_t  tbl_entry;
    uint8               f_idx;                /* field selector index */ 

    soc_field_t         off_field[8] = {UDF1_OFFSET0f, UDF1_OFFSET1f, 
                                        UDF1_OFFSET2f, UDF1_OFFSET3f,
                                        UDF2_OFFSET0f, UDF2_OFFSET1f,
                                        UDF2_OFFSET2f, UDF2_OFFSET3f};

    soc_field_t         option_field[8] = {UDF1_ADD_IPV4_OPTIONS0f,
                                           UDF1_ADD_IPV4_OPTIONS1f,
                                           UDF1_ADD_IPV4_OPTIONS2f,
                                           UDF1_ADD_IPV4_OPTIONS3f,
                                           UDF2_ADD_IPV4_OPTIONS0f,
                                           UDF2_ADD_IPV4_OPTIONS1f,
                                           UDF2_ADD_IPV4_OPTIONS2f,
                                           UDF2_ADD_IPV4_OPTIONS3f};

    if (udf_num > 1 || user_num > 3) {
        FP_ERR(("FP(unit %d) Error: udf_num=%d or user_num=%d out-of-range.\n", 
                unit, udf_num, user_num));
        return BCM_E_PARAM;
    }

    /* Formulate the desired UDF_OFFSET fields */
    f_idx = (udf_num << 2) | user_num;

    for (tbl_idx = soc_mem_index_min(unit, UDF_OFFSETm); 
         tbl_idx <= soc_mem_index_max(unit, UDF_OFFSETm); 
         tbl_idx++) { 

        SOC_IF_ERROR_RETURN
          (READ_UDF_OFFSETm(unit, MEM_BLOCK_ANY, tbl_idx, &tbl_entry));
        udf_spec->offset[tbl_idx] =
          soc_UDF_OFFSETm_field32_get(unit, &tbl_entry, off_field[f_idx]); 

        /* Get the UDFn_ADD_IPV4_OPTIONSm bit */ 
        if (soc_UDF_OFFSETm_field32_get(unit, &tbl_entry, 
                                        option_field[f_idx])) { 
            udf_spec->offset[tbl_idx] |= BCM_FIELD_USER_OPTION_ADJUST;
        }
    }

    return BCM_E_NONE;
}

#ifdef BROADCOM_DEBUG
/*
 * Function:
 *     _field_er_action_name
 * Purpose:
 *     Return text name of indicated action enum value.
 */
STATIC char *
_field_er_action_name(bcm_field_action_t action)
{
    /* Text names of Actions. These are used for debugging output and CLIs.
     * Note that the order needs to match the bcm_field_action_t enum order.
     */
    static char *action_text[] = BCM_FIELD_ACTION_STRINGS;
    assert(COUNTOF(action_text)     == bcmFieldActionCount);

    return (action >= bcmFieldActionCount ? "??" : action_text[action]);
}

#endif /* BROADCOM_DEBUG */

/*
 * Function:
 *     _field_er_fpf0_init
 * Purpose:
 *     Initializes the bit offset and Qset tables for FPF0.
 * Parameters:
 *     unit - BCM device number
 *     fc   - control structure
 * Returns:
 *     BCM_E_NONE
 * Notes:
 */
STATIC int
_field_er_fpf0_init(int unit, _field_stage_t *stage_fc)
{
    int                 idx;
    _field_fpf_info_t   fpf_info;

    fpf_info.qual_table = stage_fc->_field_table_fpf0;
    fpf_info.sel_table  = stage_fc->_field_sel_f0;
    fpf_info.field      = KEYf;

    /* Fill in the table values.                                 */
    /*             (             qualifier,  offset, wdth, code) */
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyInPort,        1,    4, 0x0);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyHiGig,         0,    1, 0x0);

    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyOutPorts,      0,   14, 0x2);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyIpType,        1,    2, 0x3);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyL2Format,      3,    2, 0x3);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyL3IntfGroup,   0,    7, 0x1);


    for (idx = 0; idx < FPF_SZ_MAX; idx++) {
        _FP_MAX_FPF_BREAK(idx, stage_fc, 0);
    /*    coverity[negative_returns : FALSE]    */
        FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyStage,    21,    1, idx);
        FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyStageIngress, 0, 0, idx);
        FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyPacketFormat, 0, 0, idx);
        FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyIp4, 0, 0, idx);
        FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyIp6, 0, 0, idx);
    }

    return BCM_E_NONE;
}


/*
 * Function:
 *     _field_er_fpf1_init
 * Purpose:
 *     initializes the bit offset tables for FPF1
 * Parameters:
 *     unit       - (IN)     BCM device number
 *     stage_fc   - (IN/OUT) control structure
 * Returns:
 *     BCM_E_NONE
 * Notes:
 */
STATIC int
_field_er_fpf1_init(int unit, _field_stage_t *stage_fc)
{
    _field_fpf_info_t   fpf_info;

    fpf_info.qual_table = stage_fc->_field_table_fpf1;
    fpf_info.sel_table  = stage_fc->_field_sel_f1;
    fpf_info.field      = KEYf;

    /* Fill in the FPF1 table values. */
    /*             (             qualifier,  offset, wdth, code) */
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyInPorts,      16,   14, 0x0);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyIpType,       14,    2, 0x0);

    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifySrcModid,     18,    6, 0x1);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifySrcPortTgid,  12,    6, 0x1);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifySrcPort,      12,   12, 0x1);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifySrcTrunk,     12,    8, 0x1);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyDstModid,      6,    6, 0x1);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyDstPortTgid,   0,    6, 0x1);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyDstPort,       0,   12, 0x1);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyDstTrunk,      0,    8, 0x1);

    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyL4SrcPort,    16,   16, 0x2);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyL4DstPort,     0,   16, 0x2);

    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyOuterVlan,    16,   16, 0x3);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyOuterVlanId,  16,   12, 0x3);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyOuterVlanCfi, 28,    1, 0x3);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyOuterVlanPri, 29,    3, 0x3);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyInnerVlan,     0,   16, 0x3);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyInnerVlanId,   0,   12, 0x3);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyInnerVlanCfi, 12,    1, 0x3);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyInnerVlanPri, 13,    3, 0x3);

    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyEtherType,    16,   16, 0x4);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyOuterVlan,     0,   16, 0x4);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyOuterVlanId,   0,   12, 0x4);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyOuterVlanCfi, 12,    1, 0x4);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyOuterVlanPri, 13,    3, 0x4);

    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyEtherType,     8,   16, 0x5);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyIpProtocol,    0,    8, 0x5);

    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyInnerVlan,        16, 16, 0x6);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyInnerVlanId,      16, 12, 0x6);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyInnerVlanCfi,     28,  1, 0x6);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyInnerVlanPri,     29,  3, 0x6);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyLookupStatus,      0, 16, 0x6);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyDosAttack,        15,  1,   0x6);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyL2StationMove,    14,  1,   0x6);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyL3DestRouteHit,   13,  1,   0x6);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyIpmcStarGroupHit, 12,  1,   0x6); 
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyL3DestHostHit,    11,  1,   0x6); 
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyL3SrcHostHit,     10,  1,   0x6); 
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyL2CacheHit,        9,  1,   0x6); 
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyL3Routable,        8,  1,   0x6); 
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyL2DestHit,         7,  1,   0x6); 
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyL2SrcStatic,       6,  1,   0x6); 
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyL2SrcHit,          5,  1,   0x6); 
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyIngressStpState,   3,  2,   0x6); 
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyForwardingVlanValid, 2,  1, 0x6); 
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyVlanTranslationHit,  1,  1, 0x6); 
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyTunnelTerminated,    0,  1, 0x6); 

    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyIpInfo,       29,    3, 0x7);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyPacketRes,    25,    4, 0x7);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyMHOpcode,     22,    3, 0x7);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyIpType,       20,    2, 0x7);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyL2Format,     16,    2, 0x7);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyVlanFormat,   18,    2, 0x7);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyOuterVlan,     0,   16, 0x7);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyOuterVlanId,   0,   12, 0x7);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyOuterVlanCfi, 12,    1, 0x7);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyOuterVlanPri, 13,    3, 0x7);

    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifySrcMacGroup,   0,     5, 0x8);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyOutPorts,      0,    14, 0x8);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyIpType,        14,    2, 0x8);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyInPorts,       16,   14, 0x8);

    return BCM_E_NONE;
}

/*
 * Function:
 *     _field_er_fpf2_init
 * Purpose:
 * 
 * Parameters:
 *     unit       - (IN)     BCM device number
 *     stage_fc   - (IN/OUT) control structure
 * Returns:
 *     BCM_E_NONE
 */
STATIC int
_field_er_fpf2_init(int unit, _field_stage_t *stage_fc)
{
    _field_fpf_info_t   fpf_info;

    fpf_info.qual_table = stage_fc->_field_table_fpf2;
    fpf_info.sel_table  = stage_fc->_field_sel_f2;
    fpf_info.field      = KEYf;

    /* Fill in the FPF2 table values. */
    /*             (             qualifier, offset, wdth, code) */
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifySrcIp,          96,  32, 0x0);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyDstIp,          64,  32, 0x0);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyIpProtocol,     56,   8, 0x0);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyL4SrcPort,      40,  16, 0x0);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyL4DstPort,      24,  16, 0x0);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyDSCP,           16,   6, 0x0);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyIpFlags,        14,   2, 0x0);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyTcpControl,      8,   6, 0x0);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyTtl,             0,   8, 0x0);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyIp6HopLimit,     0,   8, 0x0);

    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifySrcIp,          96,  32, 0x1);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyDstIp,          64,  32, 0x1);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyIpProtocol,     56,   8, 0x1);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyRangeCheck,     40,  16, 0x1);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyL4DstPort,      24,  16, 0x1);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyDSCP,           16,   6, 0x1);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyIpFlags,        14,   2, 0x1);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyTcpControl,      8,   6, 0x1);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyTtl,             0,   8, 0x1);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyIp6HopLimit,     0,   8, 0x1);

    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifySrcIp,          96,  32, 0x2);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyDstIp,          64,  32, 0x2);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyIpProtocol,     56,   8, 0x2);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyRangeCheck,     40,  16, 0x2);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyL4SrcPort,      24,  16, 0x2);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyDSCP,           16,   6, 0x2);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyIpFlags,        14,   2, 0x2);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyTcpControl,      8,   6, 0x2);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyTtl,             0,   8, 0x2);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyIp6HopLimit,     0,   8, 0x2);

    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifySrcIp6,          0, 128, 0x3);

    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyDstIp6,          0, 128, 0x4);

    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyDstIp6High,      50, 64, 0x5);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyIp6NextHeader,   42,  8, 0x5);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyTos,             34,  8, 0x5);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyIp6TrafficClass, 34,  8, 0x5);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyIp6FlowLabel,    14, 20, 0x5);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyTtl,              6,  8, 0x5);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyIp6HopLimit,      6,  8, 0x5);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyTcpControl,       0,  6, 0x5);

    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyDstMac,          80, 48, 0x6);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifySrcMac,          32, 48, 0x6);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyEtherType,       16, 16, 0x6);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyOuterVlan,        0, 16, 0x6);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyOuterVlanId,      0, 12, 0x6);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyOuterVlanCfi,    12,  1, 0x6);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyOuterVlanPri,    13,  3, 0x6);

    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifySrcMac,          64, 48, 0x7);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifySrcIp,           32, 32, 0x7);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyEtherType,       16, 16, 0x7);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyOuterVlan,        0, 16, 0x7);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyOuterVlanId,      0, 12, 0x7);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyOuterVlanCfi,    12,  1, 0x7);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyOuterVlanPri,    13,  3, 0x7);

    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyDstMac,          64, 48, 0x8);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyDstIp,           32, 32, 0x8);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyEtherType,       16, 16, 0x8);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyOuterVlan,        0, 16, 0x8);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyOuterVlanId,      0, 12, 0x8);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyOuterVlanCfi,    12,  1, 0x8);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyOuterVlanPri,    13,  3, 0x8);

    FILL_FPF_TABLE(&fpf_info, _bcmFieldQualifyData0,    0, 128, 0x9);

    FILL_FPF_TABLE(&fpf_info, _bcmFieldQualifyData1,    0, 128, 0xA);

    return BCM_E_NONE;
}

/*
 * Function:
 *     _field_er_fpf3_init
 * Purpose:
 *     Fill in the stage_fc->_field_table_fpf3 with qualifier info for FPF3.
 * Parameters:
 *     unit       - (IN)     BCM device number
 *     stage_fc   - (IN/OUT) control structure
 * Returns:
 *     BCM_E_NONE
 */
STATIC int
_field_er_fpf3_init(int unit, _field_stage_t *stage_fc)
{
    _field_fpf_info_t   fpf_info;

    fpf_info.qual_table = stage_fc->_field_table_fpf3;
    fpf_info.sel_table  = stage_fc->_field_sel_f3;
    fpf_info.field      = KEYf;

    /* Fill in the FPF3 table values. */
    /*             (               qualifier, offset, wdth, code) */
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyIpInfo,        13,    3, 0x0);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyPacketRes,      9,    4, 0x0);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyMHOpcode,       6,    3, 0x0);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyL2Format,       0,    2, 0x0);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyVlanFormat,     2,    2, 0x0);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyIpType,         4,    2, 0x0);
    
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifySrcModid,       6,    6, 0x1);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifySrcPortTgid,    0,    6, 0x1);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifySrcPort,        0,   12, 0x1);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifySrcTrunk,       0,    8, 0x1);
    
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyDstModid,       6,    6, 0x2);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyDstPortTgid,    0,    6, 0x2);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyDstPort,        0,   12, 0x2);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyDstTrunk,       0,    8, 0x2);
    
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyLookupStatus,   0,   16, 0x3);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyDosAttack,        15,  1,   0x3); 
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyL2StationMove,    14,  1,   0x3); 
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyL3DestRouteHit,   13,  1,   0x3); 
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyIpmcStarGroupHit, 12,  1,   0x3); 
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyL3DestHostHit,    11,  1,   0x3); 
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyL3SrcHostHit,     10,  1,   0x3); 
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyL2CacheHit,        9,  1,   0x3); 
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyL3Routable,        8,  1,   0x3); 
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyL2DestHit,         7,  1,   0x3); 
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyL2SrcStatic,       6,  1,   0x3); 
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyL2SrcHit,          5,  1,   0x3); 
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyIngressStpState,   3,  2,   0x3); 
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyForwardingVlanValid, 2,  1, 0x3); 
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyVlanTranslationHit,  1,  1, 0x3); 
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyTunnelTerminated,    0,  1, 0x3); 

    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyDSCP,           8,    8, 0x4);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyTcpControl,     0,    6, 0x4);
    
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyOuterVlan,      0,   16, 0x5);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyOuterVlanId,      0, 12, 0x5);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyOuterVlanCfi,    12,  1, 0x5);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyOuterVlanPri,    13,  3, 0x5);


    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyOutPorts,       0,   14, 0x9);

    return BCM_E_NONE;
}

/*
 * Function:
 *     _field_er_extn_init
 *
 * Purpose:
 *     Fill in the stage_fc->_field_table_extn with qualifier 
 *     info for External TCAM.
 *
 * Parameters:
 *     unit       - (IN)     BCM device number
 *     stage_fc   - (IN/OUT) control structure
 *
 * Returns:
 *     BCM_E_NONE  - Success
 *     BCM_E_XXX   - error from _field_qual_info_create()
 */
STATIC int
_field_er_extn_init(int unit, _field_stage_t *stage_fc) 
{
    _field_fpf_info_t    fpf_info;

    fpf_info.qual_table = stage_fc->_field_table_extn;
    fpf_info.sel_table  = stage_fc->_field_sel_extn;
    fpf_info.field      = KEYf;

    /* Fill in the IPv4 values. */
    /*                                        offset, wth, idx */
    /*  IPv4=2'00 */
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyIpType,       141,   2, 0);
    /* [128:126] */
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyDecap,        126,   3, 0); 

    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifySrcIp,         94,  32, 0);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyDstIp,         62,  32, 0);

    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyIpFlags,       54,   1, 0);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyRangeCheck,    50,   4, 0);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifySrcPortTgid,   34,  16, 0);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifySrcPort,       34,  16, 0);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifySrcTrunk,      34,  16, 0);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyDstPortTgid,   18,  16, 0);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyDstPort,       18,  16, 0);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyDstTrunk,      18,  16, 0);

    /* TOS */
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyDSCP,           9,   8, 0); 
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyIpProtocol,     1,   8, 0);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyTcpControl,     1,   8, 0);

    /* Fill in the IPv6 values. */
    /* IPv6=2'01 */
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyIpType,       141,   2, 1);
    /* [130:141] */
    /* [127:125] */
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyDecap,        124,   3, 1); 
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifySrcIp6,         6, 118, 1);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyIpFlags,        6,   1, 1);
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyRangeCheck,     2,   4, 1);

    /* Fill in the non-IP values. */
    /*non-IP=2'10 */
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyIpType,       142,   2, 2);
    /* [130:128]  */
    FILL_FPF_TABLE(&fpf_info, bcmFieldQualifyDecap,        128,   3, 2); 
    FILL_FPF_TABLE(&fpf_info, _bcmFieldQualifyData1,   0, 128, 2);

    return BCM_E_NONE;
}

/*
 * Register encoding for three-color metering.
 */
#define _FP_METER_COLOR_GREEN    0x0
#define _FP_METER_COLOR_YELLOW   0x3
#define _FP_METER_COLOR_RED      0x1

typedef struct _field_meter_config_s {
    int8    cout_pin_g_dec_c;
    int8    cout_pin_g_dec_p;
    int8    cout_pin_g_color;

    int8    cout_pin_y_dec_c;
    int8    cout_pin_y_dec_p;
    int8    cout_pin_y_color;

    int8    cout_pin_r_dec_c;
    int8    cout_pin_r_dec_p;
    int8    cout_pin_r_color;

    int8    cout_pout_g_dec_c;
    int8    cout_pout_g_dec_p;
    int8    cout_pout_g_color;

    int8    cout_pout_y_dec_c;
    int8    cout_pout_y_dec_p;
    int8    cout_pout_y_color;

    int8    cout_pout_r_dec_c;
    int8    cout_pout_r_dec_p;
    int8    cout_pout_r_color;

    int8    cin_pin_g_dec_c;
    int8    cin_pin_g_dec_p;
    int8    cin_pin_g_color;

    int8    cin_pin_y_dec_c;
    int8    cin_pin_y_dec_p;
    int8    cin_pin_y_color;

    int8    cin_pin_r_dec_c;
    int8    cin_pin_r_dec_p;
    int8    cin_pin_r_color;

    int8    cin_pout_g_dec_c;
    int8    cin_pout_g_dec_p;
    int8    cin_pout_g_color;

    int8    cin_pout_y_dec_c;
    int8    cin_pout_y_dec_p;
    int8    cin_pout_y_color;

    int8    cin_pout_r_dec_c;
    int8    cin_pout_r_dec_p;
    int8    cin_pout_r_color;
}_field_meter_config_t;

/* 
 * Macro:
 *     _FIELD_METER_CONFIG
 *
 * Purpose:
 *     Write triplet decrement commited meter, decrement peak meter, color
 *     into fields of meter configuration entry.
 *
 * Parameters:
 *     reg          - destination register (METER_DEF[0-5]_[01])
 *     commit_field - decrement committed meter field to write
 *     commit_val   - decrement (1) or don't decrement (0) committed meter
 *     peak_field   - decrement peak meter field to write
 *     peak_val     - decrement (1) or don't decrement (0) peak meter
 *     color_field  - resulting color field
 *     color_val    - desired color for conditions
 *
 * Returns:
 *     N/A
 */
#define _FIELD_METER_CONFIG(reg, reg_val_p, commit_field, commit_val,         \
                            peak_field, peak_val, color_field, color_val)     \
    soc_reg_field_set(unit, (reg), (reg_val_p), (commit_field), (commit_val));\
    soc_reg_field_set(unit, (reg), (reg_val_p), (peak_field),   (peak_val));  \
    soc_reg_field_set(unit, (reg), (reg_val_p), (color_field),  (color_val));
 
STATIC void
_field_meter_config_entry(int unit, _field_meter_config_t *cf, 
                          soc_reg_t reg0, soc_reg_t reg1,
                          uint32 *reg0_val, uint32 *reg1_val)
{
    _FIELD_METER_CONFIG(reg0, reg0_val,
                        COUT_PIN_G_DEC_Cf, cf->cout_pin_g_dec_c,
                        COUT_PIN_G_DEC_Pf, cf->cout_pin_g_dec_p,
                        COUT_PIN_G_COLORf, cf->cout_pin_g_color);
    _FIELD_METER_CONFIG(reg0, reg0_val,
                        COUT_PIN_Y_DEC_Cf, cf->cout_pin_y_dec_c,
                        COUT_PIN_Y_DEC_Pf, cf->cout_pin_y_dec_p,
                        COUT_PIN_Y_COLORf, cf->cout_pin_y_color);
    _FIELD_METER_CONFIG(reg0, reg0_val,
                        COUT_PIN_R_DEC_Cf, cf->cout_pin_r_dec_c,
                        COUT_PIN_R_DEC_Pf, cf->cout_pin_r_dec_p,
                        COUT_PIN_R_COLORf, cf->cout_pin_r_color);

    _FIELD_METER_CONFIG(reg0, reg0_val,
                        COUT_POUT_G_DEC_Cf, cf->cout_pout_g_dec_c,
                        COUT_POUT_G_DEC_Pf, cf->cout_pout_g_dec_p,
                        COUT_POUT_G_COLORf, cf->cout_pout_g_color);
    _FIELD_METER_CONFIG(reg0, reg0_val,
                        COUT_POUT_Y_DEC_Cf, cf->cout_pout_y_dec_c,
                        COUT_POUT_Y_DEC_Pf, cf->cout_pout_y_dec_p,
                        COUT_POUT_Y_COLORf, cf->cout_pout_y_color);
    _FIELD_METER_CONFIG(reg0, reg0_val,
                        COUT_POUT_R_DEC_Cf, cf->cout_pout_r_dec_c,
                        COUT_POUT_R_DEC_Pf, cf->cout_pout_r_dec_p,
                        COUT_POUT_R_COLORf, cf->cout_pout_r_color);

    _FIELD_METER_CONFIG(reg1, reg1_val,
                        CIN_PIN_G_DEC_Cf, cf->cin_pin_g_dec_c,
                        CIN_PIN_G_DEC_Pf, cf->cin_pin_g_dec_p,
                        CIN_PIN_G_COLORf, cf->cin_pin_g_color);
    _FIELD_METER_CONFIG(reg1, reg1_val,
                        CIN_PIN_Y_DEC_Cf, cf->cin_pin_y_dec_c,
                        CIN_PIN_Y_DEC_Pf, cf->cin_pin_y_dec_p,
                        CIN_PIN_Y_COLORf, cf->cin_pin_y_color);
    _FIELD_METER_CONFIG(reg1, reg1_val,
                        CIN_PIN_R_DEC_Cf, cf->cin_pin_r_dec_c,
                        CIN_PIN_R_DEC_Pf, cf->cin_pin_r_dec_p,
                        CIN_PIN_R_COLORf, cf->cin_pin_r_color);

    _FIELD_METER_CONFIG(reg1, reg1_val,
                        CIN_POUT_G_DEC_Cf, cf->cin_pout_g_dec_c,
                        CIN_POUT_G_DEC_Pf, cf->cin_pout_g_dec_p,
                        CIN_POUT_G_COLORf, cf->cin_pout_g_color);
    _FIELD_METER_CONFIG(reg1, reg1_val,
                        CIN_POUT_Y_DEC_Cf, cf->cin_pout_y_dec_c,
                        CIN_POUT_Y_DEC_Pf, cf->cin_pout_y_dec_p,
                        CIN_POUT_Y_COLORf, cf->cin_pout_y_color);
    _FIELD_METER_CONFIG(reg1, reg1_val,
                        CIN_POUT_R_DEC_Cf, cf->cin_pout_r_dec_c,
                        CIN_POUT_R_DEC_Pf, cf->cin_pout_r_dec_p,
                        CIN_POUT_R_COLORf, cf->cin_pout_r_color);
}

/*
 * Function:
 *     _field_er_meter_init
 *
 * Purpose:
 *     Configure the device's meter registers. Meters 0-5 are configured as
 *     follows:
 *          METER_DEF0 = Default mode
 *          METER_DEF1 = Flow mode
 *          METER_DEF2 = trTCM Color-Blind
 *          METER_DEF3 = trTCM Color-Aware
 *          METER_DEF6 = srTCM Color-blind
 *          METER_DEF7 = srTCM Color-Aware
 *     This ordering is chosen strategically to match the METER_PAIR_MODE
 *     values used by BCM5650x.
 * Parameters:
 *     unit - BCM device number
 *
 * Returns:
 *     BCM_E_NONE
 *
 * Note:
 *     The values written into the METER_DEFx_y registers are taken from
 *     Application Note 5660X_AN200_R.
 */
STATIC int
_field_er_meter_init(int unit)
{
    uint32              reg0_val = 0, reg1_val = 0;
    _field_meter_config_t default_mode = {0, 0, _FP_METER_COLOR_GREEN,
                                          0, 0, _FP_METER_COLOR_GREEN,
                                          0, 0, _FP_METER_COLOR_GREEN,
                                          0, 0, _FP_METER_COLOR_GREEN,
                                          0, 0, _FP_METER_COLOR_GREEN,
                                          0, 0, _FP_METER_COLOR_GREEN,
                                          0, 0, _FP_METER_COLOR_GREEN,
                                          0, 0, _FP_METER_COLOR_GREEN,
                                          0, 0, _FP_METER_COLOR_GREEN,
                                          0, 0, _FP_METER_COLOR_GREEN,
                                          0, 0, _FP_METER_COLOR_GREEN,
                                          0, 0, _FP_METER_COLOR_GREEN
    };

    _field_meter_config_t flow_mode    = {0, 0, _FP_METER_COLOR_RED,
                                          0, 0, 0,              /* N/A */
                                          0, 0, 0,              /* N/A */
                                          0, 0, _FP_METER_COLOR_RED,
                                          0, 0, 0,              /* N/A */
                                          0, 0, 0,              /* N/A */
                                          1, 1, _FP_METER_COLOR_GREEN,
                                          0, 0, 0,              /* N/A */
                                          0, 0, 0,              /* N/A */
                                          1, 0, _FP_METER_COLOR_GREEN,
                                          0, 0, 0,              /* N/A */
                                          0, 0, 0               /* N/A */
    };

    _field_meter_config_t srTCM_Blind  = {0, 1, _FP_METER_COLOR_YELLOW,
                                          0, 0, 0,              /* N/A */
                                          0, 0, 0,              /* N/A */
                                          0, 0, _FP_METER_COLOR_RED,
                                          0, 0, 0,              /* N/A */
                                          0, 0, 0,              /* N/A */
                                          1, 0, _FP_METER_COLOR_GREEN,
                                          0, 0, 0,              /* N/A */
                                          0, 0, 0,              /* N/A */
                                          1, 0, _FP_METER_COLOR_GREEN,
                                          0, 0, 0,              /* N/A */
                                          0, 0, 0               /* N/A */
    };

    _field_meter_config_t srTCM_Aware  = {0, 1, _FP_METER_COLOR_YELLOW,
                                          0, 1, _FP_METER_COLOR_YELLOW,
                                          0, 0, _FP_METER_COLOR_RED,
                                          0, 0, _FP_METER_COLOR_RED,
                                          0, 0, _FP_METER_COLOR_RED,
                                          0, 0, _FP_METER_COLOR_RED,
                                          1, 0, _FP_METER_COLOR_GREEN,
                                          0, 1, _FP_METER_COLOR_YELLOW,
                                          0, 0, _FP_METER_COLOR_RED,
                                          1, 0, _FP_METER_COLOR_GREEN,
                                          0, 0, _FP_METER_COLOR_RED,
                                          0, 0, _FP_METER_COLOR_RED
    };

    _field_meter_config_t trTCM_Blind  = {0, 1, _FP_METER_COLOR_YELLOW,
                                          0, 0, 0,              /* N/A */
                                          0, 0, 0,              /* N/A */
                                          0, 0, _FP_METER_COLOR_RED,
                                          0, 0, 0,              /* N/A */
                                          0, 0, 0,              /* N/A */
                                          1, 1, _FP_METER_COLOR_GREEN,
                                          0, 0, 0,              /* N/A */
                                          0, 0, 0,              /* N/A */
                                          1, 1, _FP_METER_COLOR_RED,
                                          0, 0, 0,              /* N/A */
                                          0, 0, 0               /* N/A */
    };

    _field_meter_config_t trTCM_Aware  = {0, 1, _FP_METER_COLOR_YELLOW,
                                          0, 1, _FP_METER_COLOR_YELLOW,
                                          0, 0, _FP_METER_COLOR_RED,
                                          0, 0, _FP_METER_COLOR_RED,
                                          0, 0, _FP_METER_COLOR_RED,
                                          0, 0, _FP_METER_COLOR_RED,
                                          1, 1, _FP_METER_COLOR_GREEN,
                                          0, 1, _FP_METER_COLOR_YELLOW,
                                          0, 0, _FP_METER_COLOR_RED,
                                          1, 1, _FP_METER_COLOR_RED,
                                          0, 0, _FP_METER_COLOR_RED,
                                          0, 0, _FP_METER_COLOR_RED
    };

    /* METER_DEF0 = default mode */
    _field_meter_config_entry(unit, &default_mode, METER_DEF0_0r, METER_DEF0_1r,
                             &reg0_val, &reg1_val);
    SOC_IF_ERROR_RETURN(WRITE_METER_DEF0_0r(unit, reg0_val));
    SOC_IF_ERROR_RETURN(WRITE_METER_DEF0_1r(unit, reg1_val));
    FP_VVERB(("METER_DEF0_0r (default)     = 0x%08x\n", reg0_val));
    FP_VVERB(("METER_DEF0_1r (default)     = 0x%08x\n", reg1_val));

    /* METER_DEF1 = Flow mode */
    _field_meter_config_entry(unit, &flow_mode,   METER_DEF1_0r, METER_DEF1_1r,
                             &reg0_val, &reg1_val);
    SOC_IF_ERROR_RETURN(WRITE_METER_DEF1_0r(unit, reg0_val));
    SOC_IF_ERROR_RETURN(WRITE_METER_DEF1_1r(unit, reg1_val));
    FP_VVERB(("METER_DEF1_0r (flow)        = 0x%08x\n", reg0_val));
    FP_VVERB(("METER_DEF1_1r (flow)        = 0x%08x\n", reg1_val));

    /* METER_DEF2 = trTCM Color-Blind */
    _field_meter_config_entry(unit, &trTCM_Blind, METER_DEF2_0r, METER_DEF2_1r,
                             &reg0_val, &reg1_val);
    SOC_IF_ERROR_RETURN(WRITE_METER_DEF2_0r(unit, reg0_val));
    SOC_IF_ERROR_RETURN(WRITE_METER_DEF2_1r(unit, reg1_val));
    FP_VVERB(("METER_DEF2_0r (trTCM Blind) = 0x%08x\n", reg0_val));
    FP_VVERB(("METER_DEF2_1r (trTCM Blind) = 0x%08x\n", reg1_val));

    /* METER_DEF3 = trTCM Color-Aware */
    _field_meter_config_entry(unit, &trTCM_Aware, METER_DEF3_0r, METER_DEF3_1r,
                             &reg0_val, &reg1_val);
    SOC_IF_ERROR_RETURN(WRITE_METER_DEF3_0r(unit, reg0_val));
    SOC_IF_ERROR_RETURN(WRITE_METER_DEF3_1r(unit, reg1_val));
    FP_VVERB(("METER_DEF3_0r (trTCM Aware) = 0x%08x\n", reg0_val));
    FP_VVERB(("METER_DEF3_1r (trTCM Aware) = 0x%08x\n", reg1_val));

    /* METER_DEF6 = srTCM Color-Blind */
    _field_meter_config_entry(unit, &srTCM_Blind, METER_DEF6_0r, METER_DEF6_1r,
                             &reg0_val, &reg1_val);
    SOC_IF_ERROR_RETURN(WRITE_METER_DEF6_0r(unit, reg0_val));
    SOC_IF_ERROR_RETURN(WRITE_METER_DEF6_1r(unit, reg1_val));
    FP_VVERB(("METER_DEF6_0r (srTCM Blind) = 0x%08x\n", reg0_val));
    FP_VVERB(("METER_DEF6_1r (srTCM Blind) = 0x%08x\n", reg1_val));

    /* METER_DEF7 = srTCM Color-Aware */
    _field_meter_config_entry(unit, &srTCM_Aware, METER_DEF7_0r, METER_DEF7_1r,
                             &reg0_val, &reg1_val);
    SOC_IF_ERROR_RETURN(WRITE_METER_DEF7_0r(unit, reg0_val));
    SOC_IF_ERROR_RETURN(WRITE_METER_DEF7_1r(unit, reg1_val));
    FP_VVERB(("METER_DEF7_0r (srTCM Aware) = 0x%08x\n", reg0_val));
    FP_VVERB(("METER_DEF7_1r (srTCM Aware) = 0x%08x\n", reg1_val));

    return BCM_E_NONE;
}
#undef _FP_METER_COLOR_GREEN
#undef _FP_METER_COLOR_YELLOW
#undef _FP_METER_COLOR_RED



/*
 * Function:
 *     _field_er_selcode_get
 *
 * Purpose:
 *     Finds a triplet of FPF select encodings that will satisfy the
 *     requested qualifier set (qset).
 *
 * Parameters:
 *     unit      - (IN) BCM unit number.
 *     stage_fc  - (IN) Stage Field control structure.
 *     qset      - (IN) Client qualifier set.
 *     sel_codes - (OUT)Select code info.  
 *
 * Returns:
 *     BCM_E_RESOURCE - No select code will satisfy qualifier set
 *     BCM_E_NONE     - Success
 *
 */
STATIC int
_field_er_selcode_get(int unit, _field_stage_t *stage_fc, 
                      bcm_field_qset_t *qset,
                      _field_group_t *fg)
{
    bcm_field_qset_t    qset_test;
    _field_sel_t        *selcode_test;
    _field_group_t      fg_temp;
    int                 fpf0, fpf1, fpf2, fpf3;
    _field_sel_t        *selcode;

    /* Input parameters check. */
    if ((NULL == stage_fc) || (NULL == qset) || (NULL == fg)) {
        return (BCM_E_PARAM); 
    }

    if (_FP_GROUP_SPAN_SINGLE_SLICE != (fg->flags & _FP_GROUP_STATUS_MASK)) {
        return (BCM_E_RESOURCE);
    }

    selcode = fg->sel_codes;
    fg_temp.flags = fg->flags;
    selcode_test = fg_temp.sel_codes;

    _FP_SELCODE_SET(fg_temp.sel_codes, _FP_SELCODE_DONT_CARE);
    BCM_FIELD_QSET_INIT(qset_test);

    for (fpf3 = _FP_SELCODE_DONT_CARE; fpf3 < FPF_SZ_MAX; fpf3++) { 
        _FP_MAX_FPF_BREAK(fpf3, stage_fc, 3);
        for (fpf2 = _FP_SELCODE_DONT_CARE; fpf2 < FPF_SZ_MAX; fpf2++) {
            if(_BCM_FIELD_STAGE_INGRESS != stage_fc->stage_id) {
                _FP_MAX_FPF_BREAK(fpf2, stage_fc, 2);
            } else if (_FP_SELCODE_IS_VALID(fpf2))  {
                if (NULL == (stage_fc->_field_table_fpf2[fpf2])) {
                    break; 
                }
            }
            for (fpf1 = _FP_SELCODE_DONT_CARE; fpf1 < FPF_SZ_MAX; fpf1++) {
                _FP_MAX_FPF_BREAK(fpf1, stage_fc, 1);
                for (fpf0 = _FP_SELCODE_DONT_CARE; fpf0 < FPF_SZ_MAX; fpf0++) {
                    _FP_MAX_FPF_BREAK(fpf0, stage_fc, 0);

                    selcode_test->fpf0 =
                        (selcode->fpf0 == _FP_SELCODE_DONT_CARE) ?  fpf0 :
                        selcode->fpf0;
                    selcode_test->fpf1 =
                        (selcode->fpf1 == _FP_SELCODE_DONT_CARE) ?  fpf1 :
                        selcode->fpf1;
                    selcode_test->fpf2 =
                        (selcode->fpf2 == _FP_SELCODE_DONT_CARE) ?  fpf2 :
                        selcode->fpf2;
                    selcode_test->fpf3 =
                        (selcode->fpf3 == _FP_SELCODE_DONT_CARE) ?  fpf3 :
                        selcode->fpf3;
                    BCM_IF_ERROR_RETURN
                        (_field_er_selcode_to_qset(unit, stage_fc, &fg_temp, 0, &qset_test));
                    if (_field_qset_is_subset(qset, &qset_test)) {
                        FP_VVERB(("FP(unit %d) vverb: sel={f0=%d,f1=%d,f2=%d,f3=%d} found\n",
                                  unit, selcode_test->fpf0, selcode_test->fpf1,
                                  selcode_test->fpf2, selcode_test->fpf3));
                        *selcode = *selcode_test;
                        return BCM_E_NONE;
                    }
                }
            }
        }
    }

    FP_ERR(("FP(unit %d) Error: No ER select code can satisfy Qset.\n", unit));
    return BCM_E_RESOURCE;
}

/*
 * Function:
 *     _field_er_selcode_to_qset
 * Purpose:
 *     Find the qset that corresponds to all the possible qualifiers
 *     supported by the field selection codes (FPF0, FPF1, FPF2, FPF3).
 * Parameters:
 *     unit      - (IN)  unit
 *     stage_fc  - (IN)  Stage Field control structure.
 *     selcodes  - (IN)  Select codes to convert to a qset.
 *     qset      - (OUT) Client qualifier set.
 * Returns:
 *     BCM_E_XXX
 *     BCM_E_NONE
 * Note:
 */
STATIC int
_field_er_selcode_to_qset(int unit, _field_stage_t *stage_fc, _field_group_t *fg,
                              int code_id, bcm_field_qset_t *qset)
{
    _field_sel_t *sel_codes;    /* Group select code selection. */ 
    int rv;                     /* Operation return status.     */

    /* Input parameters check. */
    if ((NULL == qset) || (NULL == stage_fc) ||
        (NULL == fg)   || (code_id < 0) || (code_id >= _FP_MAX_ENTRY_WIDTH)) {
        return (BCM_E_PARAM); 
    }

    BCM_FIELD_QSET_INIT(*qset);
    sel_codes = &fg->sel_codes[code_id];

    if (_FP_SELCODE_IS_VALID(sel_codes->fpf0)) {
        rv = _field_qset_union(qset, &stage_fc->_field_sel_f0[sel_codes->fpf0],
                               qset);
        BCM_IF_ERROR_RETURN(rv);
    }

    if (_FP_SELCODE_IS_VALID(sel_codes->fpf1)) {

        rv = _field_qset_union(qset, &stage_fc->_field_sel_f1[sel_codes->fpf1], 
                               qset);
        BCM_IF_ERROR_RETURN(rv);
    }

    if (_FP_SELCODE_IS_VALID(sel_codes->fpf2)) {

        rv = _field_qset_union(qset, &stage_fc->_field_sel_f2[sel_codes->fpf2],
                               qset);
        BCM_IF_ERROR_RETURN(rv);
    }

    if (_FP_SELCODE_IS_VALID(sel_codes->fpf3)) {
        rv = _field_qset_union(qset, &stage_fc->_field_sel_f3[sel_codes->fpf3],
                               qset);
        BCM_IF_ERROR_RETURN(rv);
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     _field_er_qual_list_get
 * Purpose:
 *     Build a qual_info_t linked list by assembling linked-lists from each
 *     select code.
 * Parameters:
 *     unit  - BCM device number
 * Returns:
 *     BCM_E_MEMORY
 *     BCM_E_NONE
 * Note:
 */
STATIC int
_field_er_qual_list_get(int unit, _field_stage_t *stage_fc, _field_group_t *fg)
{
    int                 offset = 0;
    _field_sel_t        *sel_codes;
    _qual_info_t        *qlist;          /* HW qualifier list pointer.*/
    int                 rv;

    if ((NULL == stage_fc) || (NULL == fg)) {
        return (BCM_E_PARAM);
    }
    sel_codes = &fg->sel_codes[0];

    /* If qualifier set was updated free original qualifiers array. */ 
    BCM_IF_ERROR_RETURN(_bcm_field_group_qualifiers_free(fg, 0));

    if (_FP_SELCODE_IS_VALID(sel_codes->fpf0)) {
        qlist =  stage_fc->_field_table_fpf0[sel_codes->fpf0];
        rv = _bcm_field_qual_list_append(fg, 0, qlist, offset);
        BCM_IF_ERROR_RETURN(rv);
    }
    if (0 == (fg->slices[0].slice_flags & _BCM_FIELD_SLICE_EXTERNAL)) {
        offset += _FIELD_ER_FPF0_BITS(unit);
    }

    if (_FP_SELCODE_IS_VALID(sel_codes->fpf3)) {
        qlist =  stage_fc->_field_table_fpf3[sel_codes->fpf3];
        rv = _bcm_field_qual_list_append(fg, 0, qlist, offset);
        BCM_IF_ERROR_RETURN(rv);
    }

    offset += _FIELD_ER_FPF3_BITS;

    if (_FP_SELCODE_IS_VALID(sel_codes->fpf2))  {
        qlist =  stage_fc->_field_table_fpf2[sel_codes->fpf2];
        rv = _bcm_field_qual_list_append(fg, 0, qlist, offset);
        BCM_IF_ERROR_RETURN(rv);
    }

    offset += _FIELD_ER_FPF2_BITS;

    if (_FP_SELCODE_IS_VALID(sel_codes->fpf1)) {
        qlist =  stage_fc->_field_table_fpf1[sel_codes->fpf1];
        rv = _bcm_field_qual_list_append(fg, 0, qlist, offset);
        BCM_IF_ERROR_RETURN(rv);
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *     _field_er_tcam_policy_clear
 * Purpose:
 *
 * Parameters:
 *     unit      -   (IN)BCM device number.
 *     stage_id  -   (IN)FP stage pipeline id. 
 *     tcam_idx  -   (IN)Entry tcam index.
 *
 * Returns:
 *     BCM_E_XXX
 * Note:
 */
int
_field_er_tcam_policy_clear(int unit, _field_stage_id_t stage_id, int tcam_idx)
{
    fp_internal_entry_t         entry_int;

    if (_BCM_FIELD_STAGE_INGRESS != stage_id) {
        return (BCM_E_PARAM);
    }
    
    /* Memory index sanity check. */
    if (tcam_idx > soc_mem_index_max(unit, FP_INTERNALm)) {
        return (BCM_E_PARAM);
    }

    /* Clear Entry. */
    sal_memset(&entry_int, 0, sizeof(fp_internal_entry_t));

    /* Write */
    SOC_IF_ERROR_RETURN
        (WRITE_FP_INTERNALm(unit, MEM_BLOCK_ALL, tcam_idx, &entry_int));

    return BCM_E_NONE;
}



/*
 * Function:
 *     _field_er_stat_action_set
 *
 * Purpose:
 *     Get counter portion of Policy Table.
 *
 * Parameters:
 *     unit      - (IN)BCM device number. 
 *     f_ent     - (IN)Software entry structure to get tcam info from.
 *     mem       - (IN)Policy table memory. 
 *     buf       - (IN/OUT)Hardware policy entry
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_field_er_stat_action_set(int unit, _field_entry_t *f_ent, void *tpp_entry)
{
    _field_stat_t  *f_st;  /* Field policer descriptor. */  
    int mode;              /* Counter hw mode.          */                    
    int idx;               /* Counter index.            */
    int rv;                /* Operation return status.  */

    /* Input parameter check. */
    if ((NULL == f_ent) || (NULL == tpp_entry))  {
        return (BCM_E_PARAM);
    }

    /* Disable counting if counter was not attached to the entry. */
    if ((0 == (f_ent->statistic.flags & _FP_POLICER_INSTALLED)) ||
        ((f_ent->flags & _FP_ENTRY_PRIMARY) && 
         (f_ent->flags & _FP_ENTRY_STAT_IN_SECONDARY_SLICE))) {
        idx = 0;
        mode = 0;
    } else {
        /* Get statistics entity description structure. */
        rv = _bcm_field_stat_get(unit, f_ent->statistic.sid, &f_st);
        BCM_IF_ERROR_RETURN(rv);
        idx = f_st->hw_index;
        mode = f_st->hw_mode;
        /* Adjust counter hw mode for COUNTER_MODE_YES_NO/NO_YES */
        if (f_ent->statistic.flags & _FP_ENTRY_STAT_USE_ODD) {
            mode++;
        }
    }

    PolicySet(unit, FP_INTERNALm, tpp_entry, TRAFFIC_COUNTERf, idx);
    PolicySet(unit, FP_INTERNALm, tpp_entry, TRAFFIC_COUNTER_MODEf, mode);
    return (BCM_E_NONE);
}

/*
 * Function:
 *     _field_er_tcam_policy_install
 * Purpose:
 *     Install a physical entry into the TCAM and policy table.
 * Parameters:
 *     unit      - BCM unit
 *     f_ent     - entry to install
 *     tcam_idx  - Index into the FP_INTERNAL table
 *
 * Returns:
 *     BCM_E_PARAM
 *     BCM_E_NONE
 *
 * Note:
 */
STATIC int
_field_er_tcam_policy_install(int unit, _field_entry_t *f_ent, int tcam_idx)
{
    _bcm_field_qual_offset_t q_offset;
    uint32                   zero = 0;
    uint32                   one = 1;
    fp_internal_entry_t      entry_int;
    _field_action_t          *fa = NULL;
    _field_tcam_t            *tcam = NULL;
    int                      fpf2;

    if (NULL == f_ent) {
        return (BCM_E_PARAM);
    }

    if ((tcam_idx < soc_mem_index_min(unit, FP_INTERNALm)) ||
        (tcam_idx > soc_mem_index_max(unit, FP_INTERNALm))) {
        return (BCM_E_PARAM);
    }

    tcam = &f_ent->tcam;

    /* Set TCP Protocol Flag bit */
    if (f_ent->flags | _FP_ENTRY_PRIMARY) {
        fpf2 = f_ent->group->sel_codes[0].fpf2;
    } else if (f_ent->flags | _FP_ENTRY_SECONDARY) {
        fpf2 = f_ent->group->sel_codes[1].fpf2;
    } else if (f_ent->flags | _FP_ENTRY_TERTIARY) {
        fpf2 = f_ent->group->sel_codes[2].fpf2;
    } else {
        return BCM_E_INTERNAL;
    }

    FP_VVERB(("FP(unit %d) vverb: installing entry=%d in Internal TCAM\n",
              unit, f_ent->eid));

    /* If needed, set the IPBM_SEL bit in F0 */
    sal_memset(&q_offset, 0, sizeof(_bcm_field_qual_offset_t));
    q_offset.field = KEYf;
    q_offset.offset = 19;
    q_offset.width = 1;
    if (BCM_FIELD_QSET_TEST(f_ent->group->qset, bcmFieldQualifyInPorts)) {
        _bcm_field_qual_value_set(unit, &q_offset, f_ent, &one, &one);
    } else {
        _bcm_field_qual_value_set(unit, &q_offset, f_ent, &zero, &zero);
    }

    /* Start with an empty TCAM plus POLICY entry. */
    sal_memset(&entry_int, 0, sizeof(fp_internal_entry_t));

    /* Extract the qualifier info from the entry structure. */
    BCM_IF_ERROR_RETURN(_field_er_tcam_get(unit, f_ent, &entry_int));

    /* Extract the policy info from the entry structure. */
    for (fa = f_ent->actions; fa != NULL; fa = fa->next) {
        BCM_IF_ERROR_RETURN(
            _field_er_action_get(unit, f_ent, tcam_idx, fa, &entry_int));
    }

    /* If the entry has a meter, get its index. */
    if (f_ent->policer[0].flags & _FP_POLICER_INSTALLED) {
        BCM_IF_ERROR_RETURN(_field_er_meter_action_set(unit, f_ent, &entry_int));
    }

    /* Set counter mode and counter index. */
    BCM_IF_ERROR_RETURN(_field_er_stat_action_set(unit, f_ent, &entry_int));

    /* Write the TCAM & Policy Tables */
    SOC_IF_ERROR_RETURN
        (WRITE_FP_INTERNALm(unit, MEM_BLOCK_ALL, tcam_idx, &entry_int));
    FP_VVERB(("FP(unit %d) vverb: END _field_er_tcam_policy_install(entry_int=0x%x,"
              "tcam_idx=%d)\n", unit, tcam_idx, entry_int));

    return BCM_E_NONE;
}

/*
 * Function:
 *     _field_er_tcam_get
 * Purpose:
 *     Extracts TCAM data from Software entry and puts it in HW fields.
 *
 * Parameters:
 *     unit       - BCM unit
 *     f_ent      - entry to extract info from
 *     tpp_entry  - (OUT) 
 *
 * Returns:
 *     BCM_E_NONE
 *
 * Note:
 */
STATIC int
_field_er_tcam_get(int unit, _field_entry_t *f_ent,
                              fp_internal_entry_t  *entry_int)
{
    _field_tcam_t       *tcam = NULL;

    tcam = &f_ent->tcam;
    assert(tcam != NULL);

    FP_VVERB(("FP(unit %d) vverb: BEGIN _field_er_tcam_get(eid=%d, tpp=0x%x)\n",
         unit, f_ent->eid, *entry_int));

    soc_mem_field_set(unit, FP_INTERNALm, (uint32 *)entry_int, KEYf,
        	      tcam->key);
    soc_mem_field_set(unit, FP_INTERNALm, (uint32 *)entry_int, MASKf,
                      tcam->mask);

    /* Note: These must be after setting F0f or they will be overwritten. */
    soc_mem_field32_set(unit, FP_INTERNALm, entry_int, VALID_0f, 1);
    soc_mem_field32_set(unit, FP_INTERNALm, entry_int, VALID_1f, 1);

    return BCM_E_NONE;
}

/*
 * Function:
 *     _field_er_group_install
 * Purpose:
 *
 * Parameters:
 *     unit      - BCM device number
 *     fg        - Group to install
 * Returns:
 *     BCM_E_NONE
 * Note:
 */
STATIC int
_field_er_group_install(int unit, _field_group_t *fg) 
{
    assert(fg != NULL);

    return _field_er_selcodes_install_int(unit, fg);
}

/*
 * Function:
 *     _field_er_selcodes_install_int
 * Purpose:
 *
 * Parameters:
 *     unit      - BCM device number
 * Returns:
 *     BCM_E_NONE
 * Note:
 */
STATIC int
_field_er_selcodes_install_int(int unit, _field_group_t *fg) 
{
    bcm_port_t                  port;
    _field_sel_t                *sel = NULL;
    fp_port_field_sel_entry_t   pfs_entry;
    ifp_port_field_sel_entry_t  ipfs_entry;
    soc_field_t                 f0_field, f1_field, f2_field, f3_field;
    int                         field_tbl[16][4] = {
        {SLICE0_F0f,  SLICE0_F1f,  SLICE0_F2f,  SLICE0_F3f},
        {SLICE1_F0f,  SLICE1_F1f,  SLICE1_F2f,  SLICE1_F3f},
        {SLICE2_F0f,  SLICE2_F1f,  SLICE2_F2f,  SLICE2_F3f},
        {SLICE3_F0f,  SLICE3_F1f,  SLICE3_F2f,  SLICE3_F3f},
        {SLICE4_F0f,  SLICE4_F1f,  SLICE4_F2f,  SLICE4_F3f},
        {SLICE5_F0f,  SLICE5_F1f,  SLICE5_F2f,  SLICE5_F3f},
        {SLICE6_F0f,  SLICE6_F1f,  SLICE6_F2f,  SLICE6_F3f},
        {SLICE7_F0f,  SLICE7_F1f,  SLICE7_F2f,  SLICE7_F3f},
        {SLICE8_F0f,  SLICE8_F1f,  SLICE8_F2f,  SLICE8_F3f},
        {SLICE9_F0f,  SLICE9_F1f,  SLICE9_F2f,  SLICE9_F3f},
        {SLICE10_F0f, SLICE10_F1f, SLICE10_F2f, SLICE10_F3f},
        {SLICE11_F0f, SLICE11_F1f, SLICE11_F2f, SLICE11_F3f},
        {SLICE12_F0f, SLICE12_F1f, SLICE12_F2f, SLICE12_F3f},
        {SLICE13_F0f, SLICE13_F1f, SLICE13_F2f, SLICE13_F3f},
        {SLICE14_F0f, SLICE14_F1f, SLICE14_F2f, SLICE14_F3f},
        {SLICE15_F0f, SLICE15_F1f, SLICE15_F2f, SLICE15_F3f}};
    int i_write_flag;

    assert(fg != NULL);

    sel  = &fg->sel_codes[0];

    /* Determine which 4 fields can be modified. */
    f0_field = field_tbl[fg->slices[0].slice_number][0];
    f1_field = field_tbl[fg->slices[0].slice_number][1];
    f2_field = field_tbl[fg->slices[0].slice_number][2];
    f3_field = field_tbl[fg->slices[0].slice_number][3];

    /* Iterate over Group's ports. */
    PBMP_ITER(fg->pbmp, port) {
        i_write_flag = _FIELD_NEED_I_WRITE(unit, port, IFP_PORT_FIELD_SELm);
        /* Read Port's current entry in FP_PORT_FIELD_SEL table */
        SOC_IF_ERROR_RETURN(READ_FP_PORT_FIELD_SELm(unit, MEM_BLOCK_ALL, port,
                                                    &pfs_entry));
        if (i_write_flag) {
            SOC_IF_ERROR_RETURN(READ_IFP_PORT_FIELD_SELm(unit, MEM_BLOCK_ALL,
                                                        port, &ipfs_entry));
        }

        /* modify 0-4 fields depending on state of SELCODE_INVALID */
        if (sel->fpf0 != _FP_SELCODE_DONT_CARE &&
            sel->fpf0 != _FP_SELCODE_DONT_USE) {
            soc_FP_PORT_FIELD_SELm_field32_set(unit, &pfs_entry, f0_field,
                                               sel->fpf0);
            if (i_write_flag) {
                soc_IFP_PORT_FIELD_SELm_field32_set(unit, &ipfs_entry, f0_field,
                                                    sel->fpf0);
            }
        }
        if (sel->fpf1 != _FP_SELCODE_DONT_CARE &&
            sel->fpf1 != _FP_SELCODE_DONT_USE) {
            soc_FP_PORT_FIELD_SELm_field32_set(unit, &pfs_entry, f1_field,
                                               sel->fpf1);
            if (i_write_flag) {
                soc_IFP_PORT_FIELD_SELm_field32_set(unit, &ipfs_entry, f1_field,
                                                    sel->fpf1);
            }
	}
        if (!(sel->fpf2 == _FP_SELCODE_DONT_CARE ||
              sel->fpf2 == _FP_SELCODE_DONT_USE)) {
            soc_FP_PORT_FIELD_SELm_field32_set(unit, &pfs_entry, f2_field,
                                               sel->fpf2);
            if (i_write_flag) {
                soc_IFP_PORT_FIELD_SELm_field32_set(unit, &ipfs_entry, f2_field,
                                                    sel->fpf2);
            }
        }
        if (sel->fpf3 != _FP_SELCODE_DONT_CARE &&
            sel->fpf3 != _FP_SELCODE_DONT_USE) {
            soc_FP_PORT_FIELD_SELm_field32_set(unit, &pfs_entry, f3_field,
                                               sel->fpf3);
            if (i_write_flag) {
                soc_IFP_PORT_FIELD_SELm_field32_set(unit, &ipfs_entry, f3_field,
                                                    sel->fpf3);
            }
	}

        /* Write each port's new entry. */
        SOC_IF_ERROR_RETURN(WRITE_FP_PORT_FIELD_SELm(unit, MEM_BLOCK_ALL,
                                                     port, &pfs_entry));
        if (i_write_flag) {
            SOC_IF_ERROR_RETURN(WRITE_IFP_PORT_FIELD_SELm(unit, MEM_BLOCK_ALL,
                                                         port, &ipfs_entry));
 
	}
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     _field_er_entry_move
 * Purpose:
 *     Moves an entry (rules + policies) from one index to another. 
 *     It copies the values in hardware from the old index 
 *     to the new index. The old index is then cleared and set invalid.
 * Parameters:
 *     unit           - BCM device number.
 *     f_ent          - Field entry pointer. 
 *     parts_count    - (IN) Field entry parts count.
 *     tcam_idx_old   - (IN) Source entry tcam index.
 *     tcam_idx_new   - (IN) Destination entry tcam index.
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_field_er_entry_move(int unit, _field_entry_t *f_ent, int parts_count, 
                     int *tcam_idx_old, int *tcam_idx_new)
{
    fp_internal_entry_t         entry_int;
    int const tcam_idx_max = soc_mem_index_max(unit, FP_INTERNALm);

    if ((NULL == f_ent) || (NULL == tcam_idx_old) || (NULL == tcam_idx_new)) {
        return (BCM_E_PARAM);
    }

    /* Calculate the new and old TCAM indexes. */
    /* Index sanity check. */
    if ((tcam_idx_old[0] < 0) || (tcam_idx_old[0] > tcam_idx_max) ||
        (tcam_idx_new[0] < 0) || (tcam_idx_new[0] > tcam_idx_max)) {
        FP_VVERB(("FP(unit %d) vverb: FP: Invalid index range for _field_er_entry_move \n \
                  from %d to %d", unit, tcam_idx_old[0], tcam_idx_new[0]));
        return (BCM_E_PARAM);
    }


    /* Read the entry from current tcam index. */
    SOC_IF_ERROR_RETURN
        (READ_FP_INTERNALm(unit, MEM_BLOCK_ANY, tcam_idx_old[0], &entry_int));

    /* Write duplicate entry to new tcam index. */
    SOC_IF_ERROR_RETURN
        (WRITE_FP_INTERNALm(unit, MEM_BLOCK_ALL, tcam_idx_new[0], &entry_int));

    /* Invalidate old tcam index entry. */
    sal_memset(&entry_int, 0, sizeof(fp_internal_entry_t));
    soc_mem_field32_set(unit, FP_INTERNALm, &entry_int, VALID_0f, 0);
    soc_mem_field32_set(unit, FP_INTERNALm, &entry_int, VALID_1f, 0);
    SOC_IF_ERROR_RETURN
        (WRITE_FP_INTERNALm(unit, MEM_BLOCK_ALL, tcam_idx_old[0], &entry_int));

    return BCM_E_NONE;
}


/*
 * Function:
 *     _field_er_entry_remove
 * Purpose:
 *     Remove a previously installed entry.
 * Parameters:
 *     unit     - BCM device number
 *     f_ent    - Entry to be removed
 *     tcam_idx - HW index to remove from
 * Returns:
 *     BCM_E_XXX
 *     BCM_E_NONE
 * Notes:
 *     FP unit lock should be held by calling function.
 */
STATIC int
_field_er_entry_remove(int unit, _field_entry_t *f_ent, int tcam_idx)
{
    fp_internal_entry_t          entry_int;

    assert(f_ent != NULL);

    /* Set FP_TCAM to default value.  */
    sal_memset(&entry_int, 0, sizeof(fp_internal_entry_t));
    SOC_IF_ERROR_RETURN(WRITE_FP_INTERNALm(unit, MEM_BLOCK_ALL,
                                           tcam_idx, &entry_int));
    f_ent->flags |= _FP_ENTRY_DIRTY; /* Mark entry as not installed */

    return BCM_E_NONE;
}


#ifdef INCLUDE_L3
/*
 * Function:
 *     _field_er_policy_set_l3_info
 * Purpose:
 *     Install l3 forwarding policy entry.  
 * Parameters:
 *     unit      - (IN) BCM device number
 *     value     - (IN) Egress object id or combined next hop information.
 *     tpp_entry - (IN/OUT) Hw entry buffer to write.
 * Returns:
 *     BCM_E_XXX
 */

STATIC int
_field_er_policy_set_l3_info(int unit, int value, void *tpp_entry)
{
    uint32 flags;         /* L3 forwarding flags             */ 
    int nh_ecmp_id;       /* Next hop/Ecmp group id.         */
    int max_ecmp_paths;   /* Maximum number of ecmp paths.   */
    int ecmp_val;         /* Ecmp group field value.         */    
    int retval;           /* Operation return value.         */ 

    /* Resove next hop /ecmp group id. */
    retval = _bcm_field_policy_set_l3_nh_resolve(unit,  value,
                                                 &flags, &nh_ecmp_id);
    BCM_IF_ERROR_RETURN(retval);

    if (flags & BCM_L3_MULTIPATH) {              
        FP_VVERB(("FP(unit %d) vverb: Install mpath L3 policy (Ecmp_group: %d)",
                  unit, nh_ecmp_id));
        ecmp_val = (1 << 16);
        BCM_IF_ERROR_RETURN(bcm_xgs3_max_ecmp_get(unit, &max_ecmp_paths));
        ecmp_val |= (((max_ecmp_paths - 1) << 11) | nh_ecmp_id);
        PolicySet(unit, FP_INTERNALm, tpp_entry, NEXT_HOP_INDEXf, ecmp_val);
    } else {
        FP_VVERB(("FP(unit %d) vverb: Install unipath L3 policy(Next hop id: %d)",
                  unit, nh_ecmp_id));
        /* PolicySet(tpp_entry, ECMPf, 0); */
        PolicySet(unit, FP_INTERNALm, tpp_entry, NEXT_HOP_INDEXf, (uint32)nh_ecmp_id);
    }
    return (BCM_E_NONE);
}
#endif /* INCLUDE_L3 */

/*
 * Function:
 *     _field_er_action_get
 * Purpose:
 *     Install an action into the hardware tables.
 * Parameters:
 *     unit     - BCM device number
 *     tcam_idx - index into TCAM
 *     fa       - field action 
 * Returns:
 *     BCM_E_NONE  - Success
 *     BCM_E_PARAM - Action parameter out-of-range or unrecognized action.
 * Notes:
 *     This is a simple read/modify/write pattern.
 *     FP unit lock should be held by calling function.
 */
STATIC int
_field_er_action_get(int unit, _field_entry_t *f_ent, int tcam_idx,
                                _field_action_t *fa,
                                void *tpp_entry)
{
    _field_control_t     *fc;
    _field_group_t       *fg;
    uint32               reg_val;

    BCM_IF_ERROR_RETURN(_field_control_get(unit, &fc));
    fg = f_ent->group;
    assert(fg != NULL);

    FP_VVERB(("FP(unit %d) vverb: BEGIN _field_er_action_get(eid=%d, action=%s) ", 
              unit, f_ent->eid, _field_er_action_name(fa->action)));
    FP_VVERB(("FP(unit %d) vverb: Internal TCAM, at %idx=0x%x\n",
              unit, tcam_idx));

    switch (fa->action) {
    case bcmFieldActionPrioPktAndIntCopy:
        PolicySet(unit, FP_INTERNALm, tpp_entry, CHANGE_PKT_PRIORITYf, 4);
        break;
    case bcmFieldActionPrioPktAndIntNew:
        PolicySet(unit, FP_INTERNALm, tpp_entry, CHANGE_PKT_PRIORITYf, 5);
        PolicySet(unit, FP_INTERNALm, tpp_entry, NEWPRIf, fa->param[0]);
        break;
    case bcmFieldActionPrioPktAndIntTos:
        PolicySet(unit, FP_INTERNALm, tpp_entry, CHANGE_PKT_PRIORITYf, 6);
        break;
    case bcmFieldActionPrioPktAndIntCancel:
        PolicySet(unit, FP_INTERNALm, tpp_entry, CHANGE_PKT_PRIORITYf, 7);
        break;
    case bcmFieldActionPrioPktCopy:
        PolicySet(unit, FP_INTERNALm, tpp_entry, CHANGE_PKT_PRIORITYf, 8);
        break;
    case bcmFieldActionPrioPktNew:
        PolicySet(unit, FP_INTERNALm, tpp_entry, CHANGE_PKT_PRIORITYf, 9);
        PolicySet(unit, FP_INTERNALm, tpp_entry, NEWPRIf, fa->param[0]);
        break;
    case bcmFieldActionPrioPktTos:
        PolicySet(unit, FP_INTERNALm, tpp_entry, CHANGE_PKT_PRIORITYf, 10);
        break;
    case bcmFieldActionPrioPktCancel:
        PolicySet(unit, FP_INTERNALm, tpp_entry, CHANGE_PKT_PRIORITYf, 11);
        break;
    case bcmFieldActionPrioIntCopy:
        PolicySet(unit, FP_INTERNALm, tpp_entry, CHANGE_PKT_PRIORITYf, 12);
        break;
    case bcmFieldActionPrioIntNew:
        PolicySet(unit, FP_INTERNALm, tpp_entry, CHANGE_PKT_PRIORITYf, 13);
        PolicySet(unit, FP_INTERNALm, tpp_entry, NEWPRIf, fa->param[0]);
        break;
    case bcmFieldActionPrioIntTos:
        PolicySet(unit, FP_INTERNALm, tpp_entry, CHANGE_PKT_PRIORITYf, 14);
        break;
    case bcmFieldActionPrioIntCancel:
        PolicySet(unit, FP_INTERNALm, tpp_entry, CHANGE_PKT_PRIORITYf, 15);
        break;
    case bcmFieldActionTosNew:
        PolicySet(unit, FP_INTERNALm, tpp_entry, RP_CHANGE_DSCPf, 1);
        PolicySet(unit, FP_INTERNALm, tpp_entry, RP_DSCPf, fa->param[0]);
        PolicySet(unit, FP_INTERNALm, tpp_entry, YP_CHANGE_DSCPf, 1);
        PolicySet(unit, FP_INTERNALm, tpp_entry, YP_DSCPf, fa->param[0]);
        PolicySet(unit, FP_INTERNALm, tpp_entry, CHANGE_DSCP_TOSf, 1);
        PolicySet(unit, FP_INTERNALm, tpp_entry, NEW_DSCPf, fa->param[0]);
        break;
    case bcmFieldActionTosCopy:
        PolicySet(unit, FP_INTERNALm, tpp_entry, CHANGE_DSCP_TOSf, 2);
        break;
    case bcmFieldActionDscpNew:
        PolicySet(unit, FP_INTERNALm, tpp_entry, CHANGE_DSCP_TOSf, 3);
        PolicySet(unit, FP_INTERNALm, tpp_entry, NEW_DSCPf, fa->param[0]);
        break;
    case bcmFieldActionTosCancel:
        PolicySet(unit, FP_INTERNALm, tpp_entry, CHANGE_DSCP_TOSf, 4);
        break;
    case bcmFieldActionDscpCancel:
        PolicySet(unit, FP_INTERNALm, tpp_entry, CHANGE_DSCP_TOSf, 4);
        break;
    case bcmFieldActionCopyToCpu:
        /* if the action is CopyToCpu, and param0 is nonzero, which means the user
         * wants to install the value of param1 as the MATCHED_RULEf, we want to
         * make sure that param1 can fit into 8 bits
         */
        PolicySet(unit, FP_INTERNALm, tpp_entry, RP_COPYTOCPUf, 1);
        PolicySet(unit, FP_INTERNALm, tpp_entry, YP_COPYTOCPUf, 1);
        PolicySet(unit, FP_INTERNALm, tpp_entry, COPYTO_CPUf, 1);
        if(fa->param[0] != 0) {
            PolicySet(unit, FP_INTERNALm, tpp_entry, MATCHED_RULEf, fa->param[1]);
        }

        break;
    case bcmFieldActionCopyToCpuCancel:
        PolicySet(unit, FP_INTERNALm, tpp_entry, RP_COPYTOCPUf, 2);
        PolicySet(unit, FP_INTERNALm, tpp_entry, YP_COPYTOCPUf, 2);
        PolicySet(unit, FP_INTERNALm, tpp_entry, COPYTO_CPUf, 2);
        break;
    case bcmFieldActionSwitchToCpuCancel:
        PolicySet(unit, FP_INTERNALm, tpp_entry, COPYTO_CPUf, 3);
        break;
    case bcmFieldActionRedirect:    /* param0=modid, param1=port/tgid */
        PolicySet(unit, FP_INTERNALm, tpp_entry, PACKET_REDIRECTIONf, 1);
        PolicySet(unit, FP_INTERNALm, tpp_entry, REDIRECTIONf, (fa->param[0] << 6) | fa->param[1]);
        break;
    case bcmFieldActionRedirectTrunk:    /* param0 = trunk ID */
        PolicySet(unit, FP_INTERNALm, tpp_entry, PACKET_REDIRECTIONf, 1);
        PolicySet(unit, FP_INTERNALm, tpp_entry, REDIRECTIONf, (fa->param[0] & 0x1f) | 0x20);
        break;
    case bcmFieldActionRedirectCancel:
        PolicySet(unit, FP_INTERNALm, tpp_entry, PACKET_REDIRECTIONf, 2);
        break;
    case bcmFieldActionRedirectPbmp:
        PolicySet(unit, FP_INTERNALm, tpp_entry, PACKET_REDIRECTIONf, 3);
        PolicySet(unit, FP_INTERNALm, tpp_entry, REDIRECTIONf, fa->param[0]);
        break;
    case bcmFieldActionEgressMask:
        PolicySet(unit, FP_INTERNALm, tpp_entry, PACKET_REDIRECTIONf, 4);
        PolicySet(unit, FP_INTERNALm, tpp_entry, REDIRECTIONf, fa->param[0]);
        break;
    case bcmFieldActionDrop:
        PolicySet(unit, FP_INTERNALm, tpp_entry, RP_DROPf, 1);
        PolicySet(unit, FP_INTERNALm, tpp_entry, YP_DROPf, 1);
        PolicySet(unit, FP_INTERNALm, tpp_entry, DROPf, 1);
        break;
    case bcmFieldActionDropCancel:
        PolicySet(unit, FP_INTERNALm, tpp_entry, RP_DROPf, 2);
        PolicySet(unit, FP_INTERNALm, tpp_entry, YP_DROPf, 2);
        PolicySet(unit, FP_INTERNALm, tpp_entry, DROPf, 2);
        break;
    case bcmFieldActionMirrorIngress:    /* param0=modid, param1=port/tgid */
        PolicySet(unit, FP_INTERNALm, tpp_entry, IM_MTP_INDEXf, (uint32)fa->hw_index);
        PolicySet(unit, FP_INTERNALm, tpp_entry, MIRRORf,
                  PolicyGet(unit, FP_INTERNALm,  tpp_entry, MIRRORf) | 1);
        SOC_IF_ERROR_RETURN(READ_SEER_CONFIGr(unit, &reg_val));
        soc_reg_field_set(unit, SEER_CONFIGr, &reg_val,
                          ENABLE_FP_FOR_MIRROR_PKTSf, 1 );
        SOC_IF_ERROR_RETURN(WRITE_SEER_CONFIGr(unit, reg_val));
        break;
    case bcmFieldActionMirrorEgress:     /* param0=modid, param1=port/tgid */
        PolicySet(unit, FP_INTERNALm, tpp_entry, EM_MTP_INDEXf, (uint32)fa->hw_index);
        PolicySet(unit, FP_INTERNALm, tpp_entry, MIRRORf,
                  PolicyGet(unit, FP_INTERNALm, tpp_entry, MIRRORf) | 2);
        SOC_IF_ERROR_RETURN(READ_SEER_CONFIGr(unit, &reg_val));
        soc_reg_field_set(unit, SEER_CONFIGr, &reg_val,
                          ENABLE_FP_FOR_MIRROR_PKTSf, 1 );
        SOC_IF_ERROR_RETURN(WRITE_SEER_CONFIGr(unit, reg_val));
        break;
#ifdef INCLUDE_L3
    case bcmFieldActionL3ChangeVlan:
        PolicySet(unit, FP_INTERNALm, tpp_entry, L3SW_CHANGE_MACDA_OR_VLANf, 1);
        BCM_IF_ERROR_RETURN
            (_field_er_policy_set_l3_info(unit, fa->param[0], tpp_entry));
        break;
    case bcmFieldActionL3ChangeVlanCancel:
        PolicySet(unit, FP_INTERNALm, tpp_entry, L3SW_CHANGE_MACDA_OR_VLANf, 2);
        break;
    case bcmFieldActionL3ChangeMacDa:
        PolicySet(unit, FP_INTERNALm, tpp_entry, L3SW_CHANGE_MACDA_OR_VLANf, 4);
        BCM_IF_ERROR_RETURN
            (_field_er_policy_set_l3_info(unit, fa->param[0], tpp_entry));
        break;
    case bcmFieldActionL3ChangeMacDaCancel:
        PolicySet(unit, FP_INTERNALm, tpp_entry, L3SW_CHANGE_MACDA_OR_VLANf, 5);
        break;
    case bcmFieldActionL3Switch:
        PolicySet(unit, FP_INTERNALm, tpp_entry, L3SW_CHANGE_MACDA_OR_VLANf, 6);
        BCM_IF_ERROR_RETURN
            (_field_er_policy_set_l3_info(unit, fa->param[0], tpp_entry));
        break;
    case bcmFieldActionL3SwitchCancel:
        PolicySet(unit, FP_INTERNALm, tpp_entry, L3SW_CHANGE_MACDA_OR_VLANf, 7);
        break;
#endif /* INCLUDE_L3 */
    case bcmFieldActionAddClassTag:
        PolicySet(unit, FP_INTERNALm, tpp_entry, L3SW_CHANGE_MACDA_OR_VLANf, 3);
        PolicySet(unit, FP_INTERNALm, tpp_entry, CLASSIFICATION_TAGf, fa->param[0]);
        break;
    case bcmFieldActionDropPrecedence:
        PolicySet(unit, FP_INTERNALm, tpp_entry, RP_DROP_PRECEDENCEf, fa->param[0]);
        PolicySet(unit, FP_INTERNALm, tpp_entry, YP_DROP_PRECEDENCEf, fa->param[0]);
        PolicySet(unit, FP_INTERNALm, tpp_entry, DROP_PRECEDENCEf, fa->param[0]);
        break;
    case bcmFieldActionRpDrop:
        PolicySet(unit, FP_INTERNALm, tpp_entry, RP_DROPf, 1);
        break;
    case bcmFieldActionRpDropCancel:
        PolicySet(unit, FP_INTERNALm, tpp_entry, RP_DROPf, 2);
        break;
    case bcmFieldActionRpDropPrecedence:
        PolicySet(unit, FP_INTERNALm, tpp_entry, RP_DROP_PRECEDENCEf, fa->param[0]);
        break;
    case bcmFieldActionRpCopyToCpu:
        PolicySet(unit, FP_INTERNALm, tpp_entry, RP_COPYTOCPUf, 1);
        break;
    case bcmFieldActionRpCopyToCpuCancel:
        PolicySet(unit, FP_INTERNALm, tpp_entry, RP_COPYTOCPUf, 2);
        break;
    case bcmFieldActionRpDscpNew:
        PolicySet(unit, FP_INTERNALm, tpp_entry, RP_CHANGE_DSCPf, 1);
        PolicySet(unit, FP_INTERNALm, tpp_entry, RP_DSCPf, fa->param[0]);
        break;
    case bcmFieldActionYpDrop:
        PolicySet(unit, FP_INTERNALm, tpp_entry, YP_DROPf, 1);
        break;
    case bcmFieldActionYpDropCancel:
        PolicySet(unit, FP_INTERNALm, tpp_entry, YP_DROPf, 2);
        break;
    case bcmFieldActionYpDropPrecedence:
        PolicySet(unit, FP_INTERNALm, tpp_entry, YP_DROP_PRECEDENCEf, fa->param[0]);
        break;
    case bcmFieldActionYpCopyToCpu:
        PolicySet(unit, FP_INTERNALm, tpp_entry, YP_COPYTOCPUf, 1);
        break;
    case bcmFieldActionYpCopyToCpuCancel:
        PolicySet(unit, FP_INTERNALm, tpp_entry, YP_COPYTOCPUf, 2);
        break;
    case bcmFieldActionYpDscpNew:
        PolicySet(unit, FP_INTERNALm, tpp_entry, YP_CHANGE_DSCPf, 1);
        PolicySet(unit, FP_INTERNALm, tpp_entry, YP_DSCPf, fa->param[0]);
        break;
    case bcmFieldActionGpDrop:
        PolicySet(unit, FP_INTERNALm, tpp_entry, DROPf, 1);
        break;
    case bcmFieldActionGpDropCancel:
        PolicySet(unit, FP_INTERNALm, tpp_entry, DROPf, 2);
        break;
    case bcmFieldActionGpDropPrecedence:
        PolicySet(unit, FP_INTERNALm, tpp_entry,DROP_PRECEDENCEf, fa->param[0]);
        break;
    case bcmFieldActionGpCopyToCpu:
        PolicySet(unit, FP_INTERNALm, tpp_entry, COPYTO_CPUf, 1);
        break;
    case bcmFieldActionGpCopyToCpuCancel:
        PolicySet(unit, FP_INTERNALm, tpp_entry, COPYTO_CPUf, 2);
        break;
    case bcmFieldActionGpDscpNew:
        PolicySet(unit, FP_INTERNALm, tpp_entry, CHANGE_DSCP_TOSf, 1);
        PolicySet(unit, FP_INTERNALm, tpp_entry, NEW_DSCPf, fa->param[0]);
        break;
    case bcmFieldActionUpdateCounter:
    case bcmFieldActionMeterConfig:
        /*
         * This action is handled by the calling routine, not by this
         * routine.
         */
        return (BCM_E_INTERNAL);
    default:
        FP_ERR(("FP(unit %d) Error: Can't install unknown action=%d\n", unit, (uint32)fa->action));
        return BCM_E_PARAM;
    }

    fa->flags &= ~_FP_ACTION_DIRTY; /* Mark action as installed. */

    return BCM_E_NONE;
}

/*
 * Function:
 *     _field_er_action_params_check
 * Purpose:
 *     Check field action parameters.
 * Parameters:
 *     unit     - BCM device number
 *     f_ent    - Field entry structure.
 *     fa       - field action 
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_field_er_action_params_check(int unit,_field_entry_t *f_ent, 
                             _field_action_t *fa)
{
#if defined(INCLUDE_L3)
    uint32 flags;            /* L3 forwarding flags.     */ 
    int nh_ecmp_id;          /* Next hop/Ecmp group id.  */
    int max_ecmp_paths;      /* Maximum number of ecmp paths.   */
    int ecmp_val;            /* Ecmp group field value.         */    
#endif /* INCLUDE_L3 */
    int rv;                  /* Operation return status. */

    switch (fa->action) {
      case bcmFieldActionPrioPktAndIntNew:
          PolicyCheck(unit, FP_INTERNALm, NEWPRIf, fa->param[0]);
          break;
      case bcmFieldActionPrioPktNew:
          PolicyCheck(unit, FP_INTERNALm, NEWPRIf, fa->param[0]);
          break;
      case bcmFieldActionPrioIntNew:
          PolicyCheck(unit, FP_INTERNALm, NEWPRIf, fa->param[0]);
          break;
      case bcmFieldActionTosNew:
          PolicyCheck(unit, FP_INTERNALm, RP_DSCPf, fa->param[0]);
          PolicyCheck(unit, FP_INTERNALm, YP_DSCPf, fa->param[0]);
          PolicyCheck(unit, FP_INTERNALm, NEW_DSCPf, fa->param[0]);
          break;
      case bcmFieldActionDscpNew:
          PolicyCheck(unit, FP_INTERNALm, NEW_DSCPf, fa->param[0]);
          break;
      case bcmFieldActionCopyToCpu:
          if ((fa->param[0] != 0) && (fa->param[1] >= (1 << 8))) {
              return BCM_E_PARAM;
          }
          if(fa->param[0] != 0) {
              PolicyCheck(unit, FP_INTERNALm, MATCHED_RULEf, fa->param[1]);
          }
          break;
      case bcmFieldActionRedirect:    /* param0=modid, param1=port/tgid */
          rv = _bcm_field_action_dest_check(unit, fa);
          BCM_IF_ERROR_RETURN(rv);
          break;
      case bcmFieldActionRedirectTrunk:    /* param0 = trunk ID */
          PolicyCheck(unit, FP_INTERNALm, REDIRECTIONf, (fa->param[0] & 0x1f) | 0x20);
          break;
      case bcmFieldActionRedirectPbmp:
          PolicyCheck(unit, FP_INTERNALm, REDIRECTIONf, fa->param[0]);
          break;
      case bcmFieldActionEgressMask:
          PolicyCheck(unit, FP_INTERNALm, REDIRECTIONf, fa->param[0]);
          break;
      case bcmFieldActionMirrorIngress:
      case bcmFieldActionMirrorEgress:
          rv = _bcm_field_action_dest_check(unit, fa);
          BCM_IF_ERROR_RETURN(rv);
          break;
#ifdef INCLUDE_L3
      case bcmFieldActionL3ChangeVlan:
      case bcmFieldActionL3ChangeMacDa:
      case bcmFieldActionL3Switch:
          rv = _bcm_field_policy_set_l3_nh_resolve(unit,  fa->param[0],
                                                   &flags, &nh_ecmp_id);

          BCM_IF_ERROR_RETURN(rv);
          if (flags & BCM_L3_MULTIPATH) {
              ecmp_val = (1 << 16);
              BCM_IF_ERROR_RETURN(bcm_xgs3_max_ecmp_get(unit, &max_ecmp_paths));
              ecmp_val |= (((max_ecmp_paths - 1) << 11) | nh_ecmp_id);
              PolicyCheck(unit, FP_INTERNALm, NEXT_HOP_INDEXf, ecmp_val);
          } else {
              PolicyCheck(unit, FP_INTERNALm, NEXT_HOP_INDEXf, nh_ecmp_id);
          }
          break;
#endif /* INCLUDE_L3 */
      case bcmFieldActionAddClassTag:
          PolicyCheck(unit, FP_INTERNALm, CLASSIFICATION_TAGf, fa->param[0]);
          break;
      case bcmFieldActionDropPrecedence:
          PolicyCheck(unit, FP_INTERNALm, RP_DROP_PRECEDENCEf, fa->param[0]);
          PolicyCheck(unit, FP_INTERNALm, YP_DROP_PRECEDENCEf, fa->param[0]);
          PolicyCheck(unit, FP_INTERNALm, DROP_PRECEDENCEf, fa->param[0]);
          break;
      case bcmFieldActionRpDropPrecedence:
          PolicyCheck(unit, FP_INTERNALm, RP_DROP_PRECEDENCEf, fa->param[0]);
          break;
      case bcmFieldActionRpDscpNew:
          PolicyCheck(unit, FP_INTERNALm, RP_DSCPf, fa->param[0]);
          break;
      case bcmFieldActionYpDropPrecedence:
          PolicyCheck(unit, FP_INTERNALm, YP_DROP_PRECEDENCEf, fa->param[0]);
          break;
      case bcmFieldActionYpDscpNew:
          PolicyCheck(unit, FP_INTERNALm, YP_DSCPf, fa->param[0]);
          break;
      case bcmFieldActionGpDropPrecedence:
          PolicyCheck(unit, FP_INTERNALm, DROP_PRECEDENCEf, fa->param[0]);
          break;
      case bcmFieldActionGpDscpNew:
          PolicyCheck(unit, FP_INTERNALm, NEW_DSCPf, fa->param[0]);
          break;
      default:
          return (BCM_E_NONE);
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *     _field_er_meter_action_set
 *
 * Purpose:
 *     Extract the meter info from the software entry to the hardware entry.
 *
 * Parameters:
 *     unit       - BCM device number
 *     f_ent      - Software entry to extract meter info
 *     tpp_entry  - (OUT) Hardware entry
 *
 * Returns:
 *     BCM_E_NONE
 *
 * Notes:
 *     FP unit lock should be held by calling function.
 */
STATIC int
_field_er_meter_action_set(int unit, _field_entry_t *f_ent, void *tpp_entry) 
{
    _field_policer_t *f_pl;
    uint32 meter_pair_mode; 

    if ((NULL == f_ent) || (NULL == tpp_entry)) {
        return (BCM_E_PARAM);
    }

    FP_VVERB(("FP(unit %d) vverb: BEGIN _field_er_meter_action_set(eid=%d, tpp=0x%x)\n",
         unit, f_ent->eid, *(fp_internal_entry_t*)tpp_entry));

    BCM_IF_ERROR_RETURN
        (_bcm_field_policer_get(unit, f_ent->policer[0].pid, &f_pl));

    /* Get hw encoding for meter mode. */
    BCM_IF_ERROR_RETURN
        (_bcm_field_meter_pair_mode_get(unit, f_pl, &meter_pair_mode));

    soc_mem_field32_set(unit, FP_INTERNALm, tpp_entry, METER_PAIR_MODEf,
                        meter_pair_mode);
    soc_mem_field32_set(unit, FP_INTERNALm, tpp_entry, METER_INDEX_EVENf,
                        f_pl->hw_index);
    soc_mem_field32_set(unit, FP_INTERNALm, tpp_entry, METER_INDEX_ODDf,
                        f_pl->hw_index);

    return BCM_E_NONE;
}

#define MeterSet(field, value) \
    soc_mem_field32_set(unit, mem, meter_entry, (field), (value))

#define MeterSetCheck(field, value)                                \
    if (SOC_MEM_FIELD32_VALUE_FIT(unit, mem, (field), (value))) {  \
        MeterSet(field, value);                                    \
    } else {                                                       \
        FP_ERR(("FP(unit %d) Error: Meter value %d > %d (max).\n", unit,          \
               (value), SOC_MEM_FIELD32_VALUE_MAX(unit, mem, (field))));  \
        return BCM_E_PARAM;                                        \
    }

/*
 * Function:
 *     _field_er_policer_install
 *
 * Purpose:
 *     Install a policer pair into the hardware tables.
 *
 * Parameters:
 *     unit   - BCM device number
 *     f_ent  - field entry 
 *
 * Returns:
 *     BCM_E_NONE
 *
 * Notes:
 *     This is a simple read/modify/write pattern.
 *     Unit lock is acquired by caller, bcm_field_entry_install()
 *     If meter is in FLOW mode, 
 *         only the corresponding meter in the pair will be installed.
 */
/* This reflects the 64,000bps granularity */
#define _ER_METER_REFRESH_RATE 64
STATIC int
_field_er_policer_install(int unit, _field_entry_t *f_ent, 
                          _field_policer_t *f_pl)
{
    fp_internal_entry_t         entry_int;
    fp_external_entry_t         entry_ext;
    uint32                      meter_entry[SOC_MAX_MEM_FIELD_WORDS];
    int                         slice_indexes;
    int                         slice_numb;
    uint32                      meter_pair_mode;   /* Meter usage. */ 
    int                         fmt_idx, /* Field Meter Table index */
                                fpt_idx; /* Field Policy Table index */
    soc_mem_t                   mem;
    uint32                      bucketsize;
    uint32                      refreshcount;
    uint32                      bucketcount;


    if ((NULL == f_ent) || (NULL == f_pl)) {
        return (BCM_E_PARAM);
    }

    if (NULL == f_ent->group || NULL == f_ent->fs) {
        return (BCM_E_INTERNAL);
    }

    if (0 == (f_pl->hw_flags & _FP_POLICER_DIRTY)) {
        return (BCM_E_NONE);
    }

    slice_indexes = f_ent->fs->entry_count;
    slice_numb = f_ent->fs->slice_number;

    fmt_idx = 2 * f_pl->hw_index + slice_numb * slice_indexes;
    fpt_idx = slice_numb * slice_indexes + f_ent->slice_idx;
 
    mem = FP_METER_TABLE_INTm;


    if (f_pl->hw_flags & _FP_POLICER_PEAK_DIRTY) {
        BCM_IF_ERROR_RETURN(_field_er_bucket_calc (unit, f_pl->cfg.pkbits_burst, 
                                                   &bucketsize,
                                                   &bucketcount));
        refreshcount = f_pl->cfg.pkbits_sec / _ER_METER_REFRESH_RATE;
        /*
         * Read/modify/write the FP_METER_TABLE fields for Commit meter
         */
        BCM_IF_ERROR_RETURN(_field_er_meter_read(unit, fmt_idx + 1, mem,
                                                 meter_entry));

        MeterSetCheck(REFRESHCOUNTf, refreshcount);
        MeterSetCheck(BUCKETSIZEf,  bucketsize);
        MeterSetCheck(BUCKETCOUNTf, bucketcount);
        FP_VVERB(("FP(unit %d) vverb: refresh=%d, b_size =%d b_count %d\n", 
                  unit, refreshcount, bucketsize, bucketcount));

        /* Refresh mode is only set to 1 for Single Rate. Other modes get 0 */
        if (f_pl->cfg.mode  == bcmPolicerModeSrTcm) {
            MeterSet(REFRESH_MODEf, 1);
        } else {
            MeterSet(REFRESH_MODEf, 0);
        }

        BCM_IF_ERROR_RETURN
            (_field_er_meter_write(unit, fmt_idx + 1, mem, meter_entry));

         f_pl->hw_flags &=  ~_FP_POLICER_PEAK_DIRTY;
    }

    if (f_pl->hw_flags & _FP_POLICER_COMMITTED_DIRTY) {
        BCM_IF_ERROR_RETURN(_field_er_bucket_calc (unit, f_pl->cfg.ckbits_burst, 
                                                   &bucketsize,
                                                   &bucketcount));

        refreshcount = f_pl->cfg.ckbits_sec / _ER_METER_REFRESH_RATE;
        /*
         * Read/modify/write the FP_METER_TABLE fields for Peak meter
         */
        BCM_IF_ERROR_RETURN(_field_er_meter_read(unit, fmt_idx, mem,
                                                 meter_entry));

        MeterSetCheck(REFRESHCOUNTf, refreshcount);
        MeterSetCheck(BUCKETSIZEf,   bucketsize);
        MeterSetCheck(BUCKETCOUNTf, bucketcount);

        FP_VVERB(("FP(unit %d) vverb: refresh=%d, b_size =%d b_count %d\n", 
                  unit, refreshcount, bucketsize, bucketcount));

        /* Refresh mode is only set to 1 for Single Rate. Other modes get 0 */
        if (f_pl->cfg.mode  == bcmPolicerModeSrTcm) {
            MeterSet(REFRESH_MODEf, 1);
        } else {
            MeterSet(REFRESH_MODEf, 0);
        }

        BCM_IF_ERROR_RETURN
            (_field_er_meter_write(unit, fmt_idx, mem, meter_entry));
         f_pl->hw_flags &=  ~_FP_POLICER_COMMITTED_DIRTY;
    }

    /* Get hw encoding for meter mode. */
    BCM_IF_ERROR_RETURN
        (_bcm_field_meter_pair_mode_get(unit, f_pl, &meter_pair_mode));

    if (mem == FP_METER_TABLE_EXTm) {
        /* Read/Modify/Write the FP_EXTERNAL fields */
        BCM_IF_ERROR_RETURN(READ_FP_EXTERNALm(unit, MEM_BLOCK_ANY, fpt_idx,
                                              &entry_ext));

        soc_mem_field32_set(unit, FP_EXTERNALm, &entry_ext, METER_PAIR_MODEf,
                            meter_pair_mode);

        BCM_IF_ERROR_RETURN(WRITE_FP_EXTERNALm(unit, MEM_BLOCK_ANY, fpt_idx,
                                               &entry_ext));
    } else {
        /* Read/Modify/Write the FP_INTERNAL fields */
        BCM_IF_ERROR_RETURN(READ_FP_INTERNALm(unit, MEM_BLOCK_ANY, fpt_idx,
                                              &entry_int));

        soc_mem_field32_set(unit, FP_INTERNALm, &entry_int, METER_INDEX_EVENf,
                            f_pl->hw_index);
        soc_mem_field32_set(unit, FP_INTERNALm, &entry_int, METER_INDEX_ODDf,
                            f_pl->hw_index);
        soc_mem_field32_set(unit, FP_INTERNALm, &entry_int, METER_PAIR_MODEf,
                            meter_pair_mode);

        BCM_IF_ERROR_RETURN(WRITE_FP_INTERNALm(unit, MEM_BLOCK_ANY, fpt_idx,
                                               &entry_int));
    }
    return BCM_E_NONE;
}
#undef MeterSet
#undef MeterSetCheck

#define BCM_ER_BITS_IN_32_KBYTES   (1 << 18) /* (32 * 1024 * 8) 32K bytes */
#define BCM_ER_BUCKET_SIZE_MAX          (12) /* Maximum bucket size encoding.*/
/*
 * Function: 
 *     _field_er_bucket_calc
 * Purpose:
 *     Lookup the proper meter bucket size encoding.
 */

STATIC int 
_field_er_bucket_calc(int unit, uint32 burst, 
                      uint32 *bucket_size, uint32 *bucket_count)
{
    uint32  b_size;
    uint8   b_code;

    /* Input  parameters check. */  
    if ((NULL == bucket_size) || (NULL == bucket_count)) {
        return (BCM_E_PARAM);
    }

    /* If requested rate is 0 -> set size & count to 0 */
    if (0 == burst) {
        *bucket_size = *bucket_count = 0;
        return (BCM_E_NONE);
    }

    /* Bucket size is encoded as follow based on Mode 3 
     *            Bucket Size
     * 4'd0     : 32 K - 1
     * 4'd1     : 64 K - 1
     * 4'd2     : 128 K - 1
     * 4'd3     : 256 K - 1
     * 4'd4     : 512 K - 1
     * 4'd5     : 1 M - 1
     * 4'd6     : 2 M - 1
     * 4'd7     : 4 M - 1
     * 4'd8     : 8 M - 1
     * 4'd9     : 16 M - 1
     * 4'd10    : 32 M - 1
     * 4'd11    : 64 M - 1
     * 4'd12    : 128 M - 1
     */
    b_size = BCM_ER_BITS_IN_32_KBYTES;
    burst = burst * 1000; /* Convert KBits to bits */
    for (b_code = 0; b_code <= BCM_ER_BUCKET_SIZE_MAX; b_code++) {
        if (burst < b_size) {
            break;
        }
        b_size *= 2;
    }
    *bucket_size  = b_code;
    if (*bucket_size >  BCM_ER_BUCKET_SIZE_MAX) {
        return (BCM_E_PARAM);
    }

    /* Bucket count is in 1/2 of a bit granularity */
    *bucket_count = 2 * b_size;
    FP_VVERB(("FP(unit %d) vverb: _field_er_bucket_calc  rate=%d, bucket_count=%d, "
             "bucket_size=%d\n", unit, burst, *bucket_count, *bucket_size));
    return (BCM_E_NONE);
}
#undef BCM_ER_BITS_IN_32_KBYTES
#undef BCM_ER_BUCKET_SIZE_MAX 

/*
 * Function: 
 *     _field_er_meter_read
 *
 * Purpose:
 *     Unified method of reading a meter table entry to hardware.
 *
 * Parameters:
 *     unit   - BCM device number
 *     idx    - Index into meter table.
 *     entry  - (OUT) Meter table entry
 *
 * Returns:
 *     BCM_E_NONE     - success
 *     BCM_E_PARAM    - meter table index out of range
 *     BCM_E_INTERNAL - Memory read failure
 * Notes:
 */
STATIC int
_field_er_meter_read(int unit, int idx, soc_mem_t mem, uint32 *entry)
{
    assert(entry != NULL);

    FP_VVERB(("FP(unit %d) vverb: _field_er_meter_read(idx=%d(%#x) int/ext=%s\n", unit,
             idx, idx, (mem == FP_METER_TABLE_INTm) ? "int" : "ext"));

    /* Range check meter table index. */
    if (idx < soc_mem_index_min(unit, mem) ||
        idx > soc_mem_index_max(unit, mem)) {
        FP_ERR(("FP(unit %d) Error: Trying to read from invalid meter table index=%#x\n", unit,
                idx));
        return BCM_E_PARAM;
    }

    /* Internal TCAM read. */
    if (mem == FP_METER_TABLE_INTm) {
        return READ_FP_METER_TABLE_INTm(unit, MEM_BLOCK_ANY, idx ,
                                       (fp_meter_table_int_entry_t *)entry);
    }

    /* External TCAM read. */
    if (mem == FP_METER_TABLE_EXTm) {
        return READ_FP_METER_TABLE_EXTm(unit, MEM_BLOCK_ANY, idx ,
                                       (fp_meter_table_ext_entry_t *)entry);
    }

    return BCM_E_PARAM;
}

/*
 * Function: 
 *     _field_er_meter_write
 *
 * Purpose:
 *     Unified method of writing a meter table entry to hardware.
 *
 * Parameters:
 *     unit   - BCM device number
 *     idx    - Index into meter table.
 *     entry  - Meter table entry
 *
 * Returns:
 *     BCM_E_NONE     - success
 *     BCM_E_PARAM    - meter table index out of range
 *     BCM_E_INTERNAL - Memory read failure
 * Notes:
 */
STATIC int
_field_er_meter_write(int unit, int idx, soc_mem_t mem, uint32 *entry)
{
    assert(entry != NULL);

    FP_VVERB(("FP(unit %d) vverb: _field_er_meter_write(unit=%d, idx=%d(%#x)\n", unit, idx, idx));

    /* Range check meter table index. */
    if (idx < soc_mem_index_min(unit, mem) ||
        idx > soc_mem_index_max(unit, mem)) {
        FP_ERR(("FP(unit %d) Error: Trying to read from invalid meter table index=%#x\n", unit,
                idx));
        return BCM_E_PARAM;
    }

    /* Internal TCAM write. */
    if (mem == FP_METER_TABLE_INTm) {
        return WRITE_FP_METER_TABLE_INTm(unit, MEM_BLOCK_ANY, idx ,
                                       (fp_meter_table_int_entry_t *)entry);
    }

    /* External TCAM read. */
    if (mem == FP_METER_TABLE_EXTm) {
        return WRITE_FP_METER_TABLE_EXTm(unit, MEM_BLOCK_ANY, idx ,
                                       (fp_meter_table_ext_entry_t *)entry);
    }

    return BCM_E_PARAM;
}

#else /* BCM_EASYRIDER_SUPPORT && BCM_FIELD_SUPPORT */
int _easyrider_field_not_empty;
#endif  /* BCM_EASYRIDER_SUPPORT && BCM_FIELD_SUPPORT */
