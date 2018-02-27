/*
 * $Id: field.c 1.42.6.2 Broadcom SDK $
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
 * Module: Field Processor APIs
 *
 * Purpose:
 *     API for Field Processor (FP) for XGS3 family and later.
 */

#include <bcm/types.h>
#include <bcm/error.h>
#include <bcm/field.h>

int
bcm_sbx_field_init(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_detach(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_status_get(int unit,
                         bcm_field_status_t *status)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_control_get(int unit,
                          bcm_field_control_t control,
                          uint32 *state)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_control_set(int unit,
                          bcm_field_control_t control,
                          uint32 state)
{
    return BCM_E_UNAVAIL;
}

/* Group Management */

int
bcm_sbx_field_group_create(int unit,
                           bcm_field_qset_t qset,
                           int pri,
                           bcm_field_group_t *group)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_group_create_id(int unit,
                              bcm_field_qset_t qset,
                              int pri,
                              bcm_field_group_t group)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_group_create_mode(int unit,
                                bcm_field_qset_t qset,
                                int pri,
                                bcm_field_group_mode_t mode,
                                bcm_field_group_t *group)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_group_create_mode_id(int unit,
                                   bcm_field_qset_t qset,
                                   int pri,
                                   bcm_field_group_mode_t mode,
                                   bcm_field_group_t group)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_group_port_create_mode(int unit,
                                     bcm_port_t port,
                                     bcm_field_qset_t qset,
                                     int pri,
                                     bcm_field_group_mode_t mode,
                                     bcm_field_group_t *group)
{
    return BCM_E_UNAVAIL;
}
int
bcm_sbx_field_group_port_create_mode_id(int unit,
                                        bcm_port_t port,
                                        bcm_field_qset_t qset,
                                        int pri,
                                        bcm_field_group_mode_t mode,
                                        bcm_field_group_t group)
{
    return BCM_E_UNAVAIL;
}
int
bcm_sbx_field_group_ports_create_mode(int unit,
                                      bcm_pbmp_t pbmp,
                                      bcm_field_qset_t qset,
                                      int pri,
                                      bcm_field_group_mode_t mode,
                                      bcm_field_group_t *group)
{
    return BCM_E_UNAVAIL;
}
int
bcm_sbx_field_group_ports_create_mode_id(int unit,
                                         bcm_pbmp_t pbmp,
                                         bcm_field_qset_t qset,
                                         int pri,
                                         bcm_field_group_mode_t mode,
                                         bcm_field_group_t group)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_group_mode_get(int unit,
                             bcm_field_group_t group,
                             bcm_field_group_mode_t *mode)
{
    return BCM_E_UNAVAIL;
}


int
bcm_sbx_field_group_flush(int unit,
                          bcm_field_group_t group)
{
    return BCM_E_UNAVAIL;
}


int
bcm_sbx_field_group_set(int unit,
                        bcm_field_group_t group,
                        bcm_field_qset_t qset)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_group_get(int unit,
                        bcm_field_group_t group,
                        bcm_field_qset_t *qset)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_group_destroy(int unit,
                            bcm_field_group_t group)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_group_status_get(int unit,
                               bcm_field_group_t group,
                               bcm_field_group_status_t *status)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_group_enable_set(int unit,
                               bcm_field_group_t group,
                               int enable)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_group_enable_get(int unit,
                               bcm_field_group_t group,
                               int *enable)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_range_create(int unit,
                           bcm_field_range_t *range,
                           uint32 flags,
                           bcm_l4_port_t min,
                           bcm_l4_port_t max)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_range_create_id(int unit,
                              bcm_field_range_t range,
                              uint32 flags,
                              bcm_l4_port_t min,
                              bcm_l4_port_t max)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_range_group_create(int unit,
                                 bcm_field_range_t *range,
                                 uint32 flags,
                                 bcm_l4_port_t min,
                                 bcm_l4_port_t max, 
                                 bcm_if_group_t group)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_range_group_create_id(int unit,
                                    bcm_field_range_t range,
                                    uint32 flags,
                                    bcm_l4_port_t min,
                                    bcm_l4_port_t max,
                                    bcm_if_group_t group)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_range_get(int unit,
                        bcm_field_range_t range,
                        uint32 *flags,
                        bcm_l4_port_t *min,
                        bcm_l4_port_t *max)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_range_destroy(int unit,
                            bcm_field_range_t range)
{
    return BCM_E_UNAVAIL;
}

/* User-Defined Fields */

int
bcm_sbx_field_udf_spec_set(int unit,
                           bcm_field_udf_spec_t *udf_spec,
                           uint32 flags,
                           uint32 offset)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_udf_spec_get(int unit,
                           bcm_field_udf_spec_t *udf_spec,
                           uint32 *flags,
                           uint32 *offset)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_udf_create(int unit,
                         bcm_field_udf_spec_t *udf_spec,
                         bcm_field_udf_t *udf_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_udf_create_id(int unit,
                            bcm_field_udf_spec_t *udf_spec,
                            bcm_field_udf_t udf_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_udf_ethertype_set(int unit,
                                int index,
                                bcm_port_frametype_t frametype,
                                bcm_port_ethertype_t ethertype)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_udf_ethertype_get(int unit,
                                int index,
                                bcm_port_frametype_t *frametype,
                                bcm_port_ethertype_t *ethertype)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_udf_ipprotocol_set(int unit,
                                 int index,
                                 uint32 flags,
                                 uint8 proto)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_udf_ipprotocol_get(int unit,
                                 int index,
                                 uint32 *flags,
                                 uint8 *proto)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_udf_get(int unit,
                      bcm_field_udf_spec_t *udf_spec,
                      bcm_field_udf_t udf_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_udf_destroy(int unit,
                          bcm_field_udf_t udf_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qset_add_udf(int unit,
                           bcm_field_qset_t *qset,
                           bcm_field_udf_t udf_id)
{
    return BCM_E_UNAVAIL;
}

/* Entry Management */

int
bcm_sbx_field_entry_create(int unit,
                           bcm_field_group_t group,
                           bcm_field_entry_t *entry)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_entry_create_id(int unit,
                              bcm_field_group_t group,
                              bcm_field_entry_t entry)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_entry_multi_get(int unit, bcm_field_group_t group,
    int entry_size, bcm_field_entry_t *entry_array, int *entry_count)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_entry_destroy(int unit,
                            bcm_field_entry_t entry)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_entry_destroy_all(int unit)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_entry_copy(int unit,
                         bcm_field_entry_t src_entry,
                         bcm_field_entry_t *dst_entry)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_entry_copy_id(int unit,
                            bcm_field_entry_t src_entry,
                            bcm_field_entry_t dst_entry)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_entry_install(int unit,
                            bcm_field_entry_t entry)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_entry_reinstall(int unit,
                              bcm_field_entry_t entry)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_entry_remove(int unit,
                           bcm_field_entry_t entry)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_entry_policer_attach(int unit, bcm_field_entry_t entry_id,
                                   int level, bcm_policer_t policer_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_entry_policer_detach(int unit, bcm_field_entry_t entry_id,
                                   int level)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_entry_policer_detach_all(int unit, bcm_field_entry_t entry_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_entry_policer_get(int unit, bcm_field_entry_t entry_id,
                                int level, bcm_policer_t *policer_id)
{   
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_entry_prio_get(int unit,
                             bcm_field_entry_t entry,
                             int *prio)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_entry_prio_set(int unit,
                             bcm_field_entry_t entry,
                             int prio)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_resync(int unit)
{
    return BCM_E_UNAVAIL;
}


/*
 * Entry Field Qualification
 *
 *   The set of qualify functions are named to match the
 *   bcm_field_qualify_t enumeration names.
 */
int
bcm_sbx_field_qualify_clear(int unit,
                            bcm_field_entry_t entry)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_InPort(int unit,
                             bcm_field_entry_t entry,
                             bcm_port_t data,
                             bcm_port_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_OutPort(int unit,
                              bcm_field_entry_t entry,
                              bcm_port_t data,
                              bcm_port_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_InPorts(int unit,
                              bcm_field_entry_t entry,
                              bcm_pbmp_t data,
                              bcm_pbmp_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_OutPorts(int unit,
                               bcm_field_entry_t entry,
                               bcm_pbmp_t data,
                               bcm_pbmp_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_Drop(int unit,
                           bcm_field_entry_t entry,
                           uint8 data,
                           uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_SrcModid(int unit,
                               bcm_field_entry_t entry,
                               bcm_module_t data,
                               bcm_module_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_SrcPortTgid(int unit,
                                  bcm_field_entry_t entry,
                                  bcm_port_t data,
                                  bcm_port_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_SrcPort(int unit,
                              bcm_field_entry_t entry,
                              bcm_module_t data_modid,
                              bcm_module_t mask_modid,
                              bcm_port_t   data_port,
                              bcm_port_t   mask_port)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_SrcTrunk(int unit,
                               bcm_field_entry_t entry,
                               bcm_trunk_t data,
                               bcm_trunk_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_DstModid(int unit,
                               bcm_field_entry_t entry,
                               bcm_module_t data,
                               bcm_module_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_DstPortTgid(int unit,
                                  bcm_field_entry_t entry,
                                  bcm_port_t data,
                                  bcm_port_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_DstPort(int unit,
                              bcm_field_entry_t entry,
                              bcm_module_t data_modid,
                              bcm_module_t mask_modid,
                              bcm_port_t   data_port,
                              bcm_port_t   mask_port)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_DstTrunk(int unit,
                               bcm_field_entry_t entry,
                               bcm_trunk_t data,
                               bcm_trunk_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_L4SrcPort(int unit,
                                bcm_field_entry_t entry,
                                bcm_l4_port_t data,
                                bcm_l4_port_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_L4DstPort(int unit,
                                bcm_field_entry_t entry,
                                bcm_l4_port_t data,
                                bcm_l4_port_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_OuterVlan(int unit,
                                bcm_field_entry_t entry,
                                bcm_vlan_t data,
                                bcm_vlan_t mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_sbx_field_qualify_OuterVlanId
 * Purpose:
 *       Set match criteria for bcmFieildQualifyOuterVlanId
 *                       qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (IN) Qualifier match data.
 *      mask - (IN) Qualifier match data.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_OuterVlanId(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_vlan_t data, 
    bcm_vlan_t mask)
{
#ifdef UNFINISHED_DRIVER_CHECK
#error DRIVER UNFINISHED
#endif
    return BCM_E_UNAVAIL; 
}


/*
 * Function:
 *      bcm_sbx_field_qualify_OuterVlanPri
 * Purpose:
 *       Set match criteria for bcmFieildQualifyOuterVlanPri
 *                       qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (IN) Qualifier match data.
 *      mask - (IN) Qualifier match data.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_OuterVlanPri(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 data, 
    uint8 mask)
{
#ifdef UNFINISHED_DRIVER_CHECK
#error DRIVER UNFINISHED
#endif
    return BCM_E_UNAVAIL; 
}


/*
 * Function:
 *      bcm_sbx_field_qualify_OuterVlanCfi
 * Purpose:
 *       Set match criteria for bcmFieildQualifyOuterVlanCfi
 *                       qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (IN) Qualifier match data.
 *      mask - (IN) Qualifier match data.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_OuterVlanCfi(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 data, 
    uint8 mask)
{
#ifdef UNFINISHED_DRIVER_CHECK
#error DRIVER UNFINISHED
#endif
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_InnerVlanId
 * Purpose:
 *       Set match criteria for bcmFieildQualifyInnerVlanId
 *                       qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (IN) Qualifier match data.
 *      mask - (IN) Qualifier match data.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_InnerVlanId(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_vlan_t data, 
    bcm_vlan_t mask)
{
#ifdef UNFINISHED_DRIVER_CHECK
#error DRIVER UNFINISHED
#endif
    return BCM_E_UNAVAIL; 
}


/*
 * Function:
 *      bcm_sbx_field_qualify_InnerVlanPri
 * Purpose:
 *       Set match criteria for bcmFieildQualifyInnerVlanPri
 *                       qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (IN) Qualifier match data.
 *      mask - (IN) Qualifier match data.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_InnerVlanPri(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 data, 
    uint8 mask)
{
#ifdef UNFINISHED_DRIVER_CHECK
#error DRIVER UNFINISHED
#endif
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_InnerVlanCfi
 * Purpose:
 *       Set match criteria for bcmFieildQualifyInnerVlanCfi
 *                       qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (IN) Qualifier match data.
 *      mask - (IN) Qualifier match data.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_InnerVlanCfi(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 data, 
    uint8 mask)
{
#ifdef UNFINISHED_DRIVER_CHECK
#error DRIVER UNFINISHED
#endif
    return BCM_E_UNAVAIL; 
}


/*
 * Function:
 *      bcm_sbx_field_qualify_OuterVlanId_get
 * Purpose:
 *       Get match criteria for bcmFieildQualifyOuterVlanId
 *                       qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match data.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_OuterVlanId_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_vlan_t *data, 
    bcm_vlan_t *mask)
{
#ifdef UNFINISHED_DRIVER_CHECK
#error DRIVER UNFINISHED
#endif
    return BCM_E_UNAVAIL; 
}


/*
 * Function:
 *      bcm_sbx_field_qualify_OuterVlanPri_get
 * Purpose:
 *       Get match criteria for bcmFieildQualifyOuterVlanPri
 *                       qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_OuterVlanPri_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
#ifdef UNFINISHED_DRIVER_CHECK
#error DRIVER UNFINISHED
#endif
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_OuterVlanCfi_get
 * Purpose:
 *       Get match criteria for bcmFieildQualifyOuterVlanCfi_get
 *                       qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_OuterVlanCfi_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
#ifdef UNFINISHED_DRIVER_CHECK
#error DRIVER UNFINISHED
#endif
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_InnerVlanId_get
 * Purpose:
 *       Get match criteria for bcmFieildQualifyInnerVlanId
 *                       qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_InnerVlanId_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_vlan_t *data, 
    bcm_vlan_t *mask)
{
#ifdef UNFINISHED_DRIVER_CHECK
#error DRIVER UNFINISHED
#endif
    return BCM_E_UNAVAIL; 
}


/*
 * Function:
 *      bcm_sbx_field_qualify_InnerVlanPri_get
 * Purpose:
 *       Get match criteria for bcmFieildQualifyInnerVlanPri
 *                       qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_InnerVlanPri_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
#ifdef UNFINISHED_DRIVER_CHECK
#error DRIVER UNFINISHED
#endif
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_InnerVlanCfi_get
 * Purpose:
 *       Get match criteria for bcmFieildQualifyInnerVlanCfi_get
 *                       qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_InnerVlanCfi_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
#ifdef UNFINISHED_DRIVER_CHECK
#error DRIVER UNFINISHED
#endif
    return BCM_E_UNAVAIL; 
}


int
bcm_sbx_field_qualify_InnerVlan(int unit,
                                bcm_field_entry_t entry,
                                bcm_vlan_t data,
                                bcm_vlan_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_EtherType(int unit,
                                bcm_field_entry_t entry,
                                uint16 data,
                                uint16 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_ExtensionHeaderSubCode(int unit,
                                             bcm_field_entry_t entry,
                                             uint8 i,uint8 j)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_ExtensionHeaderType(int unit,
                                          bcm_field_entry_t entry,
                                          uint8 i,uint8 j)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_ExtensionHeader2Type(int unit,
                                          bcm_field_entry_t entry,
                                          uint8 i,uint8 j)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_InnerSrcIp(int unit,
                                 bcm_field_entry_t entry,
                                 bcm_ip_t data,
                                 bcm_ip_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_InnerDstIp(int unit,
                                 bcm_field_entry_t entry,
                                 bcm_ip_t data,
                                 bcm_ip_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_InnerSrcIp6(int unit,
                                  bcm_field_entry_t entry,
                                  bcm_ip6_t data,
                                  bcm_ip6_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_InnerDstIp6(int unit,
                                  bcm_field_entry_t entry,
                                  bcm_ip6_t data,
                                  bcm_ip6_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_InnerSrcIp6High(int unit,
                                      bcm_field_entry_t entry,
                                      bcm_ip6_t data,
                                      bcm_ip6_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_InnerDstIp6High(int unit,
                                      bcm_field_entry_t entry,
                                      bcm_ip6_t data,
                                      bcm_ip6_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_InnerTos(int unit,
                                bcm_field_entry_t entry,
                                uint8 data,
                                uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_InnerDSCP(int unit,
                                bcm_field_entry_t entry,
                                uint8 data,
                                uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_InnerIpProtocol(int unit,
                                      bcm_field_entry_t entry,
                                      uint8 data,
                                      uint8 mask)
{
    return BCM_E_UNAVAIL;
}


int
bcm_sbx_field_qualify_InnerIpFrag(int unit,
                                  bcm_field_entry_t entry,
                                  bcm_field_IpFrag_t frag_info)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_InnerTtl(int unit,
                               bcm_field_entry_t entry,
                               uint8 data,
                               uint8 mask)
{
    return BCM_E_UNAVAIL;
}


int
bcm_sbx_field_qualify_EqualL4Port(int unit,
                                  bcm_field_entry_t entry,
                                  uint32 flag)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_IpProtocol(int unit,
                                 bcm_field_entry_t entry,
                                 uint8 data,
                                 uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_LookupStatus(int unit,
                                   bcm_field_entry_t entry,
                                   uint32 data,
                                   uint32 mask)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_field_qualify_DosAttack(int unit,
                                bcm_field_entry_t entry, 
                                uint8 data,
                                uint8 mask) 
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_field_qualify_IpmcStarGroupHit(int unit,
                                       bcm_field_entry_t entry, 
                                       uint8 data,
                                       uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_field_qualify_MyStationHit(int unit,
                                   bcm_field_entry_t entry, 
                                   uint8 data,
                                   uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_L2PayloadFirstEightBytes(int unit, bcm_field_entry_t entry,
                                               uint32 data1, uint32 data2,
                                               uint32 mask1, uint32 mask2)
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_sbx_field_qualify_L3DestRouteHit(int unit,
                                     bcm_field_entry_t entry, 
                                     uint8 data,
                                     uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_field_qualify_L3DestHostHit(int unit,
                                    bcm_field_entry_t entry, 
                                    uint8 data,
                                    uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_field_qualify_L3SrcHostHit(int unit,
                                   bcm_field_entry_t entry, 
                                   uint8 data,
                                   uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_field_qualify_L2CacheHit(int unit,
                                 bcm_field_entry_t entry, 
                                 uint8 data,
                                 uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_field_qualify_L2StationMove(int unit,
                                    bcm_field_entry_t entry, 
                                    uint8 data,
                                    uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_field_qualify_L2DestHit(int unit,
                                bcm_field_entry_t entry, 
                                uint8 data,
                                uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_field_qualify_L2SrcStatic(int unit,
                                  bcm_field_entry_t entry, 
                                  uint8 data,
                                  uint8 mask) 
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_field_qualify_L2SrcHit(int unit,
                               bcm_field_entry_t entry, 
                               uint8 data,
                               uint8 mask) 
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_field_qualify_IngressStpState(int unit,
                                      bcm_field_entry_t entry, 
                                      uint8 data,
                                      uint8 mask) 
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_field_qualify_ForwardingVlanValid(int unit,
                                          bcm_field_entry_t entry, 
                                          uint8 data,
                                          uint8 mask) 
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_field_qualify_SrcVirtualPortValid(int unit,
                                          bcm_field_entry_t entry, 
                                          uint8 data,
                                          uint8 mask) 
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_DstL3EgressNextHops(int unit,
                                          bcm_field_entry_t entry,
                                          uint32 data,
                                          uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_sbx_field_qualify_VlanTranslationHit(int unit,
                                         bcm_field_entry_t entry, 
                                         uint8 data,
                                         uint8 mask) 
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_IpInfo(int unit,
                             bcm_field_entry_t entry,
                             uint32 data,
                             uint32 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_PacketRes(int unit,
                                bcm_field_entry_t entry,
                                uint32 data,
                                uint32 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_SrcIp(int unit,
                            bcm_field_entry_t entry,
                            bcm_ip_t data,
                            bcm_ip_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_DstIp(int unit,
                            bcm_field_entry_t entry,
                            bcm_ip_t data,
                            bcm_ip_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_DSCP(int unit,
                           bcm_field_entry_t entry,
                           uint8 data,
                           uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_Tos(int unit,
                          bcm_field_entry_t entry,
                          uint8 data,
                          uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_IpFlags(int unit,
                              bcm_field_entry_t entry,
                              uint8 data,
                              uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_TcpControl(int unit,
                                 bcm_field_entry_t entry,
                                 uint8 data,
                                 uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_TcpSequenceZero(int unit,
                                      bcm_field_entry_t entry,
                                      uint32 flag)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_TcpHeaderSize(int unit,
                                    bcm_field_entry_t entry,
                                    uint8 data,
                                    uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_Ttl(int unit,
                          bcm_field_entry_t entry,
                          uint8 data,
                          uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int bcm_sbx_field_qualify_IpAuth(int unit, 
				 bcm_field_entry_t entry ,
				 uint8 a,
				 uint8 b)
{
  return BCM_E_UNAVAIL;
}


int
bcm_sbx_field_qualify_RangeCheck(int unit,
                                 bcm_field_entry_t entry,
                                 bcm_field_range_t range,
                                 int invert)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_SrcIp6(int unit,
                             bcm_field_entry_t entry,
                             bcm_ip6_t data,
                             bcm_ip6_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_DstIp6(int unit,
                             bcm_field_entry_t entry,
                             bcm_ip6_t data,
                             bcm_ip6_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_SrcIp6High(int unit,
                                 bcm_field_entry_t entry,
                                 bcm_ip6_t data,
                                 bcm_ip6_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_SrcIpEqualDstIp(int unit,
                                      bcm_field_entry_t entry,
                                      uint32 flag)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_DstIp6High(int unit,
                                 bcm_field_entry_t entry,
                                 bcm_ip6_t data,
                                 bcm_ip6_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_Ip6NextHeader(int unit,
                                    bcm_field_entry_t entry,
                                    uint8 data,
                                    uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_Ip6TrafficClass(int unit,
                                      bcm_field_entry_t entry,
                                      uint8 data,
                                      uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_Ip6FlowLabel(int unit,
                                   bcm_field_entry_t entry,
                                   uint32 data,
                                   uint32 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_Ip6HopLimit(int unit,
                                  bcm_field_entry_t entry,
                                  uint8 data,
                                  uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_SrcMac(int unit,
                             bcm_field_entry_t entry,
                             bcm_mac_t data,
                             bcm_mac_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_DstMac(int unit,
                             bcm_field_entry_t entry,
                             bcm_mac_t data,
                             bcm_mac_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_PacketFormat(int unit,
                                   bcm_field_entry_t entry,
                                   uint32 data,
                                   uint32 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_SrcMacGroup(int unit,
                                  bcm_field_entry_t entry,
                                  uint32 data,
                                  uint32 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_ForwardingType(int unit, bcm_field_entry_t entry,
                                     bcm_field_ForwardingType_t data)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_sbx_field_qualify_IpType(int unit,
                             bcm_field_entry_t entry,
                             bcm_field_IpType_t type)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_L2Format(int unit,
                               bcm_field_entry_t entry,
                               bcm_field_L2Format_t type)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_VlanFormat(int unit,
                                 bcm_field_entry_t entry,
                                 uint8 data,
                                 uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_MHOpcode(int unit,
                               bcm_field_entry_t entry,
                               uint8 data,
                               uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_UserDefined(int unit,
                                  bcm_field_entry_t entry,
                                  bcm_field_udf_t udf_id, 
                                  uint8 data[],
                                  uint8 mask[])
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_Decap(int unit,
                            bcm_field_entry_t entry,
                            bcm_field_decap_t decap)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_HiGig(int unit,
                            bcm_field_entry_t entry,
                            uint8 data,
                            uint8 mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_sbx_field_qualify_DstHiGig
 * Purpose:
 *      Qualify on HiGig destination packets.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (IN) Qualifier match data.
 *      mask - (IN) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_DstHiGig(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 data, 
    uint8 mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_DstHiGig_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstHiGig
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_DstHiGig_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}


int
bcm_sbx_field_qualify_Stage(int unit,
                            bcm_field_entry_t entry,
                            bcm_field_stage_t stage)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_L3IntfGroup(int unit,
                                  bcm_field_entry_t entry,
                                  bcm_if_group_t data,
                                  bcm_if_group_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_InterfaceClassL2(int unit, bcm_field_entry_t entry,
                                        uint32 data, uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_sbx_field_qualify_InterfaceClassL3(int unit, bcm_field_entry_t entry,
                                        uint32 data, uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_sbx_field_qualify_InterfaceClassPort(int unit, bcm_field_entry_t entry,
                                        uint32 data, uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_sbx_field_qualify_SrcClassL2(int unit, bcm_field_entry_t entry,
                                  uint32 data, uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_sbx_field_qualify_SrcClassL3(int unit, bcm_field_entry_t entry,
                                  uint32 data, uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_sbx_field_qualify_SrcClassField(int unit, bcm_field_entry_t entry,
                                  uint32 data, uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_sbx_field_qualify_DstClassL2(int unit, bcm_field_entry_t entry,
                                  uint32 data, uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_sbx_field_qualify_DstClassL3(int unit, bcm_field_entry_t entry,
                                  uint32 data, uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_sbx_field_qualify_DstClassField(int unit, bcm_field_entry_t entry,
                                  uint32 data, uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_sbx_field_qualify_IpProtocolCommon(int unit, 
                                       bcm_field_entry_t entry,
                                       bcm_field_IpProtocolCommon_t protocol)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_Snap(int unit,
                           bcm_field_entry_t entry,
                           bcm_field_snap_header_t data, 
                           bcm_field_snap_header_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_Llc(int unit,
                          bcm_field_entry_t entry,
                          bcm_field_llc_header_t data, 
                          bcm_field_llc_header_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_InnerTpid(int unit,
                                bcm_field_entry_t entry,
                                uint16 tpid)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_OuterTpid(int unit,
                                bcm_field_entry_t entry,
                                uint16 tpid)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_PortClass(int unit,
                                bcm_field_entry_t entry,
                                uint32 data,
                                uint32 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_L3Routable(int unit,
                                 bcm_field_entry_t entry,
                                 uint8 data,
                                 uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_IpFrag(int unit,
                             bcm_field_entry_t entry,
                             bcm_field_IpFrag_t frag_info)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_LookupClass0(int unit,
                                   bcm_field_entry_t entry,
                                   uint32 data,
                                   uint32 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_Vrf(int unit,
                          bcm_field_entry_t entry,
                          uint32 data,
                          uint32 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_L3Ingress(int unit,
                          bcm_field_entry_t entry,
                          uint32 data,
                          uint32 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_L4Ports(int unit,
                              bcm_field_entry_t entry,
                              uint8 data,
                              uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_MirrorCopy(int unit,
                                 bcm_field_entry_t entry,
                                 uint8 data,
                                 uint8 mask)
{
    return BCM_E_UNAVAIL;
}


int
bcm_sbx_field_qualify_TunnelTerminated(int unit,
                                       bcm_field_entry_t entry,
                                       uint8 data,
                                       uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_MplsTerminated(int unit,
                                     bcm_field_entry_t entry,
                                     uint8 data,
                                     uint8 mask)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_field_qualify_FlowId(int unit, bcm_field_entry_t entry,
                 uint16 data, uint16 mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_sbx_field_qualify_InVPort(int unit, bcm_field_entry_t entry,
                 uint8 data, uint8 mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_sbx_field_qualify_OutVPort(int unit, bcm_field_entry_t entry,
                 uint8 data, uint8 mask)
{
    return BCM_E_UNAVAIL; 
}

int 
bcm_sbx_field_qualify_FlowId_get(int unit, bcm_field_entry_t entry,
                 uint16 *data, uint16 *mask)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_field_qualify_InVPort_get(int unit, bcm_field_entry_t entry,
                 uint8 *data, uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_field_qualify_OutVPort_get(int unit, bcm_field_entry_t entry,
                 uint8 *data, uint8 *mask)
{
    return BCM_E_UNAVAIL;
}

/* Entry Action Management */

int
bcm_sbx_field_action_add(int unit,
                         bcm_field_entry_t entry,
                         bcm_field_action_t action,
                         uint32 param0,
                         uint32 param1)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_action_ports_add(int unit,
                               bcm_field_entry_t entry,
                               bcm_field_action_t action,
                               bcm_pbmp_t pbmp)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_action_get(int unit,
                         bcm_field_entry_t entry,
                         bcm_field_action_t action,
                         uint32 *param0,
                         uint32 *param1)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_action_ports_get(int unit,
                               bcm_field_entry_t entry,
                               bcm_field_action_t action,
                               bcm_pbmp_t *pbmp)
{
    return BCM_E_UNAVAIL;
}

int 
bcm_sbx_field_action_delete(int unit,
                            bcm_field_entry_t entry, 
                            bcm_field_action_t action, 
                            uint32 param0,
                            uint32 param1)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_action_remove(int unit,
                            bcm_field_entry_t entry,
                            bcm_field_action_t action)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_action_remove_all(int unit,
                                bcm_field_entry_t entry)
{
    return BCM_E_UNAVAIL;
}

/* Counter Management */

int
bcm_sbx_field_counter_create(int unit,
                             bcm_field_entry_t entry)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_counter_share(int unit,
                            bcm_field_entry_t src_entry,
                            bcm_field_entry_t dst_entry)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_counter_destroy(int unit,
                              bcm_field_entry_t entry)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_counter_set(int unit,
                          bcm_field_entry_t entry,
                          int counter_num,
                          uint64 val)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_counter_set32(int unit,
                            bcm_field_entry_t entry,
                            int counter_num,
                            uint32 val)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_counter_get(int unit,
                          bcm_field_entry_t entry,
                          int counter_num,
                          uint64 *valp)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_counter_get32(int unit,
                            bcm_field_entry_t entry,
                            int counter_num,
                            uint32 *valp)
{
    return BCM_E_UNAVAIL;
}

/* Meter Management */

int
bcm_sbx_field_meter_create(int unit,
                           bcm_field_entry_t entry)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_meter_share(int unit,
                          bcm_field_entry_t src_entry,
                          bcm_field_entry_t dst_entry)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_meter_destroy(int unit,
                            bcm_field_entry_t entry)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_meter_set(int unit,
                        bcm_field_entry_t entry,
                        int meter_num,
                        uint32 kbits_sec,
                        uint32 kbits_burst)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_meter_get(int unit,
                        bcm_field_entry_t entry,
                        int meter_num,
                        uint32 *kbits_sec,
                        uint32 *kbits_burst)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_group_compress(int unit, bcm_field_group_t group)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_sbx_field_group_priority_set(int unit, bcm_field_group_t group,
                                 int priority)
{
    return (BCM_E_UNAVAIL);
}

int
bcm_sbx_field_group_priority_get(int unit, bcm_field_group_t group,
                                 int *priority)
{
    return (BCM_E_UNAVAIL);
}


int 
bcm_sbx_field_qualify_BigIcmpCheck(int unit, 
                                   bcm_field_entry_t entry, 
                                   uint32 flag, 
                                   uint32 size)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_IcmpTypeCode(int unit, 
                                   bcm_field_entry_t entry, 
                                   uint16 data, 
                                   uint16 mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_IgmpTypeMaxRespTime(int unit, 
                                          bcm_field_entry_t entry, 
                                          uint16 data, 
                                          uint16 mask)
{
    return BCM_E_UNAVAIL;
}


int
bcm_sbx_field_entry_stat_attach(int unit, 
                                bcm_field_entry_t entry, 
                                int stat_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_entry_stat_detach(int unit, 
                                bcm_field_entry_t entry, 
                                int stat_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_entry_stat_get(int unit, 
                             bcm_field_entry_t entry, 
                             int *stat_id)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_InnerL4DstPort(int unit, bcm_field_entry_t entry,
                                bcm_l4_port_t data, bcm_l4_port_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_InnerL4SrcPort(int unit, bcm_field_entry_t entry,
                                bcm_l4_port_t data, bcm_l4_port_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_InnerIpType(int unit, bcm_field_entry_t entry,
                             bcm_field_IpType_t type)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_InnerIpProtocolCommon(int unit, bcm_field_entry_t entry,
                                       bcm_field_IpProtocolCommon_t protocol)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_qualify_InnerIp6FlowLabel(int unit, bcm_field_entry_t entry,
                                   uint32 data, uint32 mask)
{
    return (BCM_E_UNAVAIL);
}

int 
bcm_sbx_field_qualify_DstL3Egress(int unit, 
                               bcm_field_entry_t entry, 
                               bcm_if_t if_id)
{
    return BCM_E_UNAVAIL; 
}


int 
bcm_sbx_field_qualify_DstMulticastGroup(int unit, 
                                 bcm_field_entry_t entry, 
                                 bcm_gport_t group)
{
    return BCM_E_UNAVAIL; 
}


int 
bcm_sbx_field_qualify_SrcMplsGport(int unit, 
                               bcm_field_entry_t entry, 
                               bcm_gport_t mpls_port_id)
{
    return BCM_E_UNAVAIL; 
}


int 
bcm_sbx_field_qualify_DstMplsGport(int unit, 
                               bcm_field_entry_t entry, 
                               bcm_gport_t mpls_port_id)
{
    return BCM_E_UNAVAIL; 
}


int 
bcm_sbx_field_qualify_SrcMimGport(int unit, 
                              bcm_field_entry_t entry, 
                              bcm_gport_t mim_port_id)
{
    return BCM_E_UNAVAIL; 
}


int 
bcm_sbx_field_qualify_DstMimGport(int unit, 
                              bcm_field_entry_t entry, 
                              bcm_gport_t mim_port_id)
{
    return BCM_E_UNAVAIL; 
}



int 
bcm_sbx_field_qualify_SrcWlanGport(int unit, 
                               bcm_field_entry_t entry, 
                               bcm_gport_t wlan_port_id)
{
    return BCM_E_UNAVAIL; 
}


int 
bcm_sbx_field_qualify_DstWlanGport(int unit, 
                               bcm_field_entry_t entry, 
                               bcm_gport_t wlan_port_id)
{
    return BCM_E_UNAVAIL; 
}


int 
bcm_sbx_field_qualify_Loopback(int unit, 
                               bcm_field_entry_t entry, 
                               uint8 data, 
                               uint8 mask)
{
    return BCM_E_UNAVAIL; 
}



int 
bcm_sbx_field_qualify_LoopbackType(int unit, 
                                   bcm_field_entry_t entry, 
                                   bcm_field_LoopbackType_t loopback_type)
{
    return BCM_E_UNAVAIL; 
}


int 
bcm_sbx_field_qualify_TunnelType(int unit, 
                                 bcm_field_entry_t entry, 
                                 bcm_field_TunnelType_t tunnel_type)
{
    return BCM_E_UNAVAIL; 
}



/*
 * Function:
 *      bcm_sbx_field_qualify_LoopbackType_get
 * Purpose:
 *      Get loopback type field qualification from a field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) Field entry id.
 *      loopback_type - (OUT) Loopback type.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_LoopbackType_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_LoopbackType_t *loopback_type)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_TunnelType_get
 * Purpose:
 *      Get tunnel type field qualification from a field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) Field entry id.
 *      tunnel_type - (OUT) Tunnel type.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_TunnelType_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_TunnelType_t *tunnel_type)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_DstL3Egress_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstL3Egress
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      if_id - (OUT) L3 forwarding object.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_DstL3Egress_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_if_t *if_id)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_DstMulticastGroup_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstMulticastGroup
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      group - (OUT) Multicast group id.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_DstMulticastGroup_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_gport_t *group)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_SrcMplsGport_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcMplsGport
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      mpls_port_id - (OUT) Mpls port id.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_SrcMplsGport_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_gport_t *mpls_port_id)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_DstMplsGport_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstMplsGport
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      mpls_port_id - (OUT) Mpls port id.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_DstMplsGport_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_gport_t *mpls_port_id)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_SrcMimGport_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcMimGport
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      mim_port_id - (OUT) Mim port id.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_SrcMimGport_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_gport_t *mim_port_id)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_DstMimGport_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstMimGport
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      mim_port_id - (OUT) Mim port id.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_DstMimGport_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_gport_t *mim_port_id)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_SrcWlanGport_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcWlanGport
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      wlan_port_id - (OUT) Wlan port id.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_SrcWlanGport_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_gport_t *wlan_port_id)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_DstWlanGport_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstWlanGport
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      wlan_port_id - (OUT) Wlan port id.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_DstWlanGport_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_gport_t *wlan_port_id)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_Loopback_get
 * Purpose:
 *      Get loopback field qualification from  a field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) Field entry id.
 *      data - (OUT) Data to qualify with.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_Loopback_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_InPort_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInPort
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_InPort_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_port_t *data, 
    bcm_port_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_OutPort_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyOutPort
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_OutPort_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_port_t *data, 
    bcm_port_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_InPorts_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInPorts
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_InPorts_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_pbmp_t *data, 
    bcm_pbmp_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_OutPorts_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyOutPorts
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_OutPorts_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_pbmp_t *data, 
    bcm_pbmp_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_Drop_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDrop
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_Drop_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_SrcModid_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcModid
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_SrcModid_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_module_t *data, 
    bcm_module_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_SrcPortTgid_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcPortTgid
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_SrcPortTgid_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_port_t *data, 
    bcm_port_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_SrcPort_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcPort
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data_modid - (OUT) Qualifier module id.
 *      mask_modid - (OUT) Qualifier module id mask.
 *      data_port - (OUT) Qualifier port id.
 *      mask_port - (OUT) Qualifier port id mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_SrcPort_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_module_t *data_modid, 
    bcm_module_t *mask_modid, 
    bcm_port_t *data_port, 
    bcm_port_t *mask_port)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_SrcTrunk_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcTrunk
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_SrcTrunk_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_trunk_t *data, 
    bcm_trunk_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_DstModid_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstModid
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_DstModid_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_module_t *data, 
    bcm_module_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_DstPortTgid_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstPortTgid
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_DstPortTgid_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_port_t *data, 
    bcm_port_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_DstPort_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstPort
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data_modid - (OUT) Qualifier module id.
 *      mask_modid - (OUT) Qualifier module id mask.
 *      data_port - (OUT) Qualifier port id.
 *      mask_port - (OUT) Qualifier port id mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_DstPort_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_module_t *data_modid, 
    bcm_module_t *mask_modid, 
    bcm_port_t *data_port, 
    bcm_port_t *mask_port)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_DstTrunk_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstTrunk
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_DstTrunk_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_trunk_t *data, 
    bcm_trunk_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_InnerL4SrcPort_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerL4SrcPort
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_InnerL4SrcPort_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_l4_port_t *data, 
    bcm_l4_port_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_InnerL4DstPort_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerL4DstPort
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_InnerL4DstPort_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_l4_port_t *data, 
    bcm_l4_port_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_L4SrcPort_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL4SrcPort
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_L4SrcPort_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_l4_port_t *data, 
    bcm_l4_port_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_L4DstPort_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL4DstPort
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_L4DstPort_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_l4_port_t *data, 
    bcm_l4_port_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_OuterVlan_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyOuterVlan
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_OuterVlan_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_vlan_t *data, 
    bcm_vlan_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_InnerVlan_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerVlan
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_InnerVlan_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_vlan_t *data, 
    bcm_vlan_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_EtherType_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyEtherType
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_EtherType_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint16 *data, 
    uint16 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_EqualL4Port_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyEqualL4Port
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      flag - (OUT) Qualifier flags.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_EqualL4Port_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *flag)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_IpProtocol_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIpProtocol
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_IpProtocol_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}


/*
 * Function:
 *      bcm_sbx_field_qualify_IpInfo_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIpInfo
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_IpInfo_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_PacketRes_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyPacketRes
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_PacketRes_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_SrcIp_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcIp
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_SrcIp_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip_t *data, 
    bcm_ip_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_DstIp_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstIp
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_DstIp_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip_t *data, 
    bcm_ip_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_DSCP_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDSCP
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_DSCP_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_Tos_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyTos
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_Tos_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_IpFlags_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIpFlags
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_IpFlags_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_TcpControl_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyTcpControl
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_TcpControl_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_TcpSequenceZero_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyTcpSequenceZero
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      flag - (OUT) Qualifier match flags.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_TcpSequenceZero_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *flag)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_TcpHeaderSize_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyTcpHeaderSize
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_TcpHeaderSize_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_Ttl_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyTtl
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_Ttl_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_RangeCheck_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyRangeCheck
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      max_count - (IN) Max entries to fill.
 *      range - (OUT) Range checkers array.
 *      invert - (OUT) Range checkers invert array.
 *      count - (OUT) Number of filled range checkers.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_RangeCheck_get(
    int unit, 
    bcm_field_entry_t entry, 
    int max_count, 
    bcm_field_range_t *range, 
    int *invert, 
    int *count)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_SrcIp6_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcIp6
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_SrcIp6_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip6_t *data, 
    bcm_ip6_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_DstIp6_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstIp6
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_DstIp6_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip6_t *data, 
    bcm_ip6_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_SrcIp6High_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcIp6High
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_SrcIp6High_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip6_t *data, 
    bcm_ip6_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_SrcIpEqualDstIp_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcIpEqualDstIp
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      flag - (OUT) Qualifier match flags.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_SrcIpEqualDstIp_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *flag)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_DstIp6High_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstIp6High
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_DstIp6High_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip6_t *data, 
    bcm_ip6_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_Ip6NextHeader_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIp6NextHeader
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_Ip6NextHeader_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_Ip6TrafficClass_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIp6TrafficClass
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_Ip6TrafficClass_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_InnerIp6FlowLabel_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerIp6FlowLabel
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_InnerIp6FlowLabel_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_Ip6FlowLabel_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIp6FlowLabel
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_Ip6FlowLabel_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_Ip6HopLimit_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIp6HopLimit
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_Ip6HopLimit_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_SrcMac_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcMac
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_SrcMac_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_mac_t *data, 
    bcm_mac_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_DstMac_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstMac
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_DstMac_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_mac_t *data, 
    bcm_mac_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_PacketFormat_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyPacketFormat
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_PacketFormat_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_SrcMacGroup_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcMacGroup
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_SrcMacGroup_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_InnerIpType_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerIpType
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      type - (OUT) Inner ip header ip type.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_InnerIpType_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_IpType_t *type)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_ForwardingType_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyForwardingType
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      type - (OUT) Qualifier match forwarding type.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_ForwardingType_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_ForwardingType_t *type)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_IpType_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIpType
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      type - (OUT) Qualifier match ip type.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_IpType_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_IpType_t *type)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_L2Format_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL2Format
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      type - (OUT) Qualifier match l2 format.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_L2Format_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_L2Format_t *type)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_VlanFormat_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyVlanFormat
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_VlanFormat_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_MHOpcode_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyMHOpcode
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_MHOpcode_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_UserDefined_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyUserDefined
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      udf_id - (IN) Udf spec id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_UserDefined_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_udf_t udf_id, 
    uint8 data[BCM_FIELD_USER_FIELD_SIZE], 
    uint8 mask[BCM_FIELD_USER_FIELD_SIZE])
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_HiGig_get
 * Purpose:
 *      Qualify on HiGig packets.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_HiGig_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_InterfaceClassPort_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInterfaceClassPort
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_InterfaceClassPort_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_InterfaceClassL2_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInterfaceClassL2
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_InterfaceClassL2_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_InterfaceClassL3_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInterfaceClassL3
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_InterfaceClassL3_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_SrcClassL2_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcClassL2
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_SrcClassL2_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_SrcClassL3_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcClassL3
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_SrcClassL3_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_SrcClassField_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcClassField
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_SrcClassField_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_DstClassL2_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstClassL2
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_DstClassL2_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_DstClassL3_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstClassL3
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_DstClassL3_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_DstClassField_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstClassField
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_DstClassField_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_L3IntfGroup_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL3IntfGroup
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_L3IntfGroup_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_if_group_t *data, 
    bcm_if_group_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_IpProtocolCommon_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIpProtocolCommon
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      protocol - (OUT) Qualifier protocol encoding.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_IpProtocolCommon_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_IpProtocolCommon_t *protocol)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_InnerIpProtocolCommon_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerIpProtocolCommon
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      protocol - (OUT) Qualifier inner ip protocol encodnig.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_InnerIpProtocolCommon_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_IpProtocolCommon_t *protocol)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_Snap_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySnap
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_Snap_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_snap_header_t *data, 
    bcm_field_snap_header_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_Llc_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyLlc
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_Llc_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_llc_header_t *data, 
    bcm_field_llc_header_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_InnerTpid_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerTpid
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      tpid - (OUT) Qualifier tpid.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_InnerTpid_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint16 *tpid)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_OuterTpid_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyOuterTpid
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      tpid - (OUT) Qualifier tpid.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_OuterTpid_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint16 *tpid)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_PortClass_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyPortClass
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_PortClass_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_L3Routable_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL3Routable
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_L3Routable_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_IpFrag_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIpFrag
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      frag_info - (OUT) Qualifier ip framentation encoding.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_IpFrag_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_IpFrag_t *frag_info)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_LookupClass0_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyLookupClass0
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_LookupClass0_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_Vrf_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyVrf
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_Vrf_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_L3Ingress_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL3Ingress
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_L3Ingress_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_ExtensionHeaderType_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyExtensionHeaderType
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_ExtensionHeaderType_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_ExtensionHeaderSubCode_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyExtensionHeaderSubCode
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_ExtensionHeaderSubCode_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_ExtensionHeader2Type_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyExtensionHeader2Type
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_ExtensionHeader2Type_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_L4Ports_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL4Ports
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_L4Ports_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_MirrorCopy_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyMirrorCopy
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_MirrorCopy_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_TunnelTerminated_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyTunnelTerminated
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_TunnelTerminated_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_MplsTerminated_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyMplsTerminated
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_MplsTerminated_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_InnerSrcIp_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerSrcIp
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_InnerSrcIp_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip_t *data, 
    bcm_ip_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_InnerDstIp_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerDstIp
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_InnerDstIp_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip_t *data, 
    bcm_ip_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_InnerSrcIp6_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerSrcIp6
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_InnerSrcIp6_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip6_t *data, 
    bcm_ip6_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_InnerDstIp6_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerDstIp6
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_InnerDstIp6_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip6_t *data, 
    bcm_ip6_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_InnerSrcIp6High_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerSrcIp6High
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_InnerSrcIp6High_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip6_t *data, 
    bcm_ip6_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_InnerDstIp6High_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerDstIp6High
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_InnerDstIp6High_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip6_t *data, 
    bcm_ip6_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_InnerTtl_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerTtl
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_InnerTtl_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_InnerTos_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerDSCP
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_InnerTos_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_InnerDSCP_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerDSCP
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_InnerDSCP_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_InnerIpProtocol_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerIpProtocol
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_InnerIpProtocol_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_InnerIpFrag_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInnerIpFrag
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      frag_info - (OUT) Inner ip header fragmentation info.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_InnerIpFrag_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_IpFrag_t *frag_info)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_DosAttack_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDosAttack
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_DosAttack_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_IpmcStarGroupHit_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIpmcStarGroupHit
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_IpmcStarGroupHit_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_MyStationHit_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyMyStationHit
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_MyStationHit_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_L2PayloadFirstEightBytes_get
 * Purpose:
 *      Get match criteria for bcmFieldQualifyL2PayloadFirstEightBytes
 *          qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data1 - (OUT) Qualifier first four bytes of match data.
 *      data2 - (OUT) Qualifier last four bytes of match data.
 *      mask1 - (OUT) Qualifier first four bytes of match mask.
 *      mask2 - (OUT) Qualifier last four bytes of match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_sbx_field_qualify_L2PayloadFirstEightBytes_get(
    int unit,
    bcm_field_entry_t entry,
    uint32 *data1,
    uint32 *data2,
    uint32 *mask1,
    uint32 *mask2)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_sbx_field_qualify_L3DestRouteHit_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL3DestRouteHit
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_L3DestRouteHit_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_L3DestHostHit_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL3DestHostHit
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_L3DestHostHit_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_L3SrcHostHit_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL3SrcHostHit
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_L3SrcHostHit_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_L2CacheHit_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL2CacheHit
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_L2CacheHit_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_L2StationMove_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL2StationMove
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_L2StationMove_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_L2DestHit_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL2DestHit
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_L2DestHit_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_L2SrcStatic_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL2SrcStatic
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_L2SrcStatic_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_L2SrcHit_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL2SrcHit
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_L2SrcHit_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_IngressStpState_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIngressStpState
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_IngressStpState_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_ForwardingVlanValid_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyForwardingVlanValid
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_ForwardingVlanValid_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/* Function: bcm_sbx_field_group_config_create
 *
 * Purpose:
 *     Create a group with a mode (single, double, etc.), a port bitmap,
 *     group size and a Group ID. 
 * Parameters:
 *     unit - BCM device number.
 *     group_config - Group create attributes namely:
 *          flags       - (IN) Bits indicate which parameters have been
 *                             passed to API and should be used during group
 *                             creation.
 *          qset        - (IN) Field qualifier set
 *          priority    - (IN) Priority within allowable range,
 *                             or BCM_FIELD_GROUP_PRIO_ANY to automatically
 *                             assign a priority; each priority value may be
 *                             used only once
 *          mode        - (IN) Group mode (single, double, triple or Auto-wide)
 *          ports       - (IN) Ports where group is defined
 *          group       - (IN/OUT) Requested Group ID. If Group ID is not set,
 *                              then API allocates and returns the created
 *                              Group ID.
 * Returns:
 *     BCM_E_INIT      - BCM unit not initialized
 *     BCM_E_RESOURCE  - no select codes will satisfy qualifier set
 *     BCM_E_NONE      - Success
 */
int
bcm_sbx_field_group_config_create(int unit,
    bcm_field_group_config_t *group_config)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_sbx_field_qualify_SrcVirtualPortValid_get
 * Purpose:
 *      Get match criteria for bcmFieldQualifySrcVirtualPortValid
 *      qualifier from the field entry.
 * Parameters:
 *      unit    - (IN) Unit number.
 *      entry   - (IN) BCM field entry id.
 *      data    - (OUT) Qualifier match data.
 *      mask    - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_SrcVirtualPortValid_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_DstL3EgressNextHops_get
 * Purpose:
 *      Get match criteria for bcmFieldQualifyDstL3EgressNextHops
 *      qualifier from the field entry.
 * Parameters:
 *      unit    - (IN) Unit number.
 *      entry   - (IN) BCM field entry id.
 *      data    - (OUT) Qualifier match data.
 *      mask    - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_sbx_field_qualify_DstL3EgressNextHops_get(int unit,
    bcm_field_entry_t entry,
    uint32 *data,
    uint32 *mask)
{
    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *      bcm_sbx_field_qualify_VlanTranslationHit_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyVlanTranslationHit
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_VlanTranslationHit_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_IpAuth_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIpAuth
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_IpAuth_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_BigIcmpCheck_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyBigIcmpCheck
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      flag - (OUT) Flag.
 *      size - (OUT) Size.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_BigIcmpCheck_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *flag, 
    uint32 *size)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_IcmpTypeCode_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIcmpTypeCode
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_IcmpTypeCode_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint16 *data, 
    uint16 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_IgmpTypeMaxRespTime_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIgmpTypeMaxRespTime
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_IgmpTypeMaxRespTime_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint16 *data, 
    uint16 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualifier_delete
 * Purpose:
 *      Remove match criteria from a field processor entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      qual_id - (IN) BCM field qualifier id.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualifier_delete(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_qualify_t qual_id)
{
#ifdef UNFINISHED_DRIVER_CHECK
#error DRIVER UNFINISHED
#endif
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_TranslatedVlanFormat
 * Purpose:
 *      Set match criteria for bcmFieildQualifyTranslatedVlanFormat
 *                     qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (IN) Qualifier match data.
 *      mask - (IN) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_TranslatedVlanFormat(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 data, 
    uint8 mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_IntPriority
 * Purpose:
 *      Set match criteria for bcmFieildQualifyIntPriority
 *                     qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (IN) Qualifier match data.
 *      mask - (IN) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_IntPriority(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 data, 
    uint8 mask)
{
    return BCM_E_UNAVAIL; 
}


/*
 * Function:
 *      bcm_sbx_field_qualify_Color
 * Purpose:
 *      Set match criteria for bcmFieildQualifyColor
 *                     qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      color - (IN) Qualifier match color.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_Color(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 color)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_FibreChanOuter
 * Purpose:
 *      Add Fibre Channel outer header type field qualification to a field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) Field entry id.
 *      fibre_chan_type - (IN) Data to qualify with.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_FibreChanOuter(int unit, 
                             bcm_field_entry_t entry, 
                             bcm_field_FibreChan_t fibre_chan_type)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_sbx_field_qualify_FibreChanInner
 * Purpose:
 *      Add Fibre Channel inner header type field qualification to a field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) Field entry id.
 *      fibre_chan_type - (IN) Data to qualify with.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_FibreChanInner(int unit, 
                             bcm_field_entry_t entry, 
                             bcm_field_FibreChan_t fibre_chan_type)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_sbx_field_qualify_VnTag
 * Purpose:
 *      Add NIV VN tag field qualification to a field entry.
 * Parameters:
 *      unit  - (IN) Unit number.
 *      entry - (IN) Field entry id.
 *      data  - (IN) Qualifier match data.
 *      mask  - (IN) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_sbx_field_qualify_VnTag(int unit, bcm_field_entry_t entry,
                            uint32 data, uint32 mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_sbx_field_qualify_CnTag
 * Purpose:
 *      Add QCN CN tag field qualification to a field entry.
 * Parameters:
 *      unit  - (IN) Unit number.
 *      entry - (IN) Field entry id.
 *      data  - (IN) Qualifier match data.
 *      mask  - (IN) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_sbx_field_qualify_CnTag(int unit, bcm_field_entry_t entry,
                            uint32 data, uint32 mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_sbx_field_qualify_FabricQueueTag
 * Purpose:
 *      Add Fabric Queue tag field qualification to a field entry.
 * Parameters:
 *      unit  - (IN) Unit number.
 *      entry - (IN) Field entry id.
 *      data  - (IN) Qualifier match data.
 *      mask  - (IN) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_sbx_field_qualify_FabricQueueTag(int unit, bcm_field_entry_t entry,
                                     uint32 data, uint32 mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_sbx_field_qualify_SrcModPortGport
 * Purpose:
 *      Set match criteria for bcmFieildQualifySrcModPortGport
 *                     qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (IN) Qualifier match gport.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_SrcModPortGport(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_gport_t data)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_SrcModuleGport
 * Purpose:
 *      Set match criteria for bcmFieildQualifySrcModuleGport
 *                     qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (IN) Qualifier match gport.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_SrcModuleGport(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_gport_t data)
{
    return BCM_E_UNAVAIL; 
}


/*
 * Function:
 *      bcm_sbx_field_qualify_TranslatedVlanFormat_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyTranslatedVlanFormat
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_TranslatedVlanFormat_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_IntPriority_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIntPriority
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_IntPriority_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *data, 
    uint8 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_Color_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyColor
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      color - (OUT) Qualifier match color.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_Color_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint8 *color)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_FibreChannOuter_get
 * Purpose:
 *      Get Fibre Channel outer header type field qualification from a field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) Field entry id.
 *      fibre_chan_type - (OUT) Fibre Channel header type.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_FibreChanOuter_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_FibreChan_t *fibre_chan_type)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_FibreChannInner_get
 * Purpose:
 *      Get Fibre Channel inner header type field qualification from a field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) Field entry id.
 *      fibre_chan_type - (OUT) Fibre Channel header type.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_FibreChanInner_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_FibreChan_t *fibre_chan_type)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_VnTag_get
 * Purpose:
 *      Get match criteria for bcmFieldQualifyVnTag
 *      qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_VnTag_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_CnTag_get
 * Purpose:
 *      Get match criteria for bcmFieldQualifyCnTag
 *      qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_CnTag_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_FabricQueueTag_get
 * Purpose:
 *      Get match criteria for bcmFieldQualifyFabricQueueTag
 *      qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_FabricQueueTag_get(
    int unit, 
    bcm_field_entry_t entry, 
    uint32 *data, 
    uint32 *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_SrcModPortGport_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcModPortGport
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match gport.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_SrcModPortGport_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_gport_t *data)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_SrcModuleGport_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcModuleGport
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match gport.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_SrcModuleGport_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_gport_t *data)
{
    return BCM_E_UNAVAIL; 
}


/* Function: bcm_sbx_field_group_wlan_create_mode
 *     
 * Purpose:
 *     Create a wlan field group with a mode (single, double, etc.).
 *
 * Parameters:
 *     unit - BCM device number.
 *     qset - Field qualifier set
 *     pri  - Priority within allowable range,
 *            or BCM_FIELD_GROUP_PRIO_ANY to automatically assign a
 *            priority; each priority value may be used only once
 *    mode  - Group mode (single, double, triple or Auto-wide)
 *    group - (OUT) field Group ID
 *
 * Returns:
 *     BCM_E_INIT      - BCM unit not initialized
 *     BCM_E_RESOURCE  - no select codes will satisfy qualifier set
 *     BCM_E_NONE      - Success
 */
int
bcm_sbx_field_group_wlan_create_mode(int unit, bcm_field_qset_t qset, int pri,
                                     bcm_field_group_mode_t mode,
                                     bcm_field_group_t *group)
{
    return BCM_E_UNAVAIL; 
}

/* Function: bcm_sbx_field_group_wlan_create_mode_id
 *     
 * Purpose:
 *     Create a wlan field group with a mode (single, double, etc.).
 *
 * Parameters:
 *     unit - BCM device number.
 *     qset - Field qualifier set
 *     pri  - Priority within allowable range,
 *            or BCM_FIELD_GROUP_PRIO_ANY to automatically assign a
 *            priority; each priority value may be used only once
 *    mode  - Group mode (single, double, triple or Auto-wide)
 *    group - (OUT) field Group ID
 *
 * Returns:
 *     BCM_E_INIT      - BCM unit not initialized
 *     BCM_E_RESOURCE  - no select codes will satisfy qualifier set
 *     BCM_E_NONE      - Success
 */
int
bcm_sbx_field_group_wlan_create_mode_id(int unit, bcm_field_qset_t qset, int pri,
                                        bcm_field_group_mode_t mode,
                                        bcm_field_group_t group)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_action_mac_add
 * Purpose:
 *      Add an action to a field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) Field entry Id.
 *      action - (IN) Field action id.
 *      mac - (IN) Field action parameter.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_action_mac_add(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_action_t action, 
    bcm_mac_t mac)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_action_mac_get
 * Purpose:
 *      Retrieve the parameters for an action previously added to a
 *      field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) Field entry Id.
 *      action - (IN) Field action id.
 *      mac - (IN) Field action argument.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_action_mac_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_field_action_t action, 
    bcm_mac_t *mac)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_group_traverse
 * Purpose:
 *      Traverse all the fp groups in the system, calling a specified
 *      callback for each one
 * Parameters:
 *      unit - (IN) Unit number.
 *      callback - (IN) A pointer to the callback function to call for each fp group
 *      user_data - (IN) Pointer to user data to supply in the callback
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_sbx_field_group_traverse(int unit, bcm_field_group_traverse_cb callback,
                             void *user_data)
{
    return BCM_E_UNAVAIL; 
}


/*
 * Function:
 *      bcm_sbx_field_qualify_ForwardingVlanId_get
 * Purpose:
 *       Get match criteria for bcmFieildQualifyForwardingVlanId
 *                       qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match data.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_ForwardingVlanId_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_vlan_t *data, 
    bcm_vlan_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_Vpn_get
 * Purpose:
 *       Get match criteria for bcmFieildQualifyVpn
 *                       qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match data.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_Vpn_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_vpn_t *data, 
    bcm_vpn_t *mask)
{
    return BCM_E_UNAVAIL; 
}


/*
 * Function:
 *      bcm_sbx_field_qualify_ForwardingVlanId
 * Purpose:
 *      Set match criteria for bcmFieildQualifyForwardingVlanId
 *      qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (IN) Qualifier match data.
 *      mask - (IN) Qualifier match data.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_ForwardingVlanId(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_vlan_t data, 
    bcm_vlan_t mask)
{
    return BCM_E_UNAVAIL; 
}


/*
 * Function:
 *      bcm_sbx_field_qualify_Vpn
 * Purpose:
 *      Set match criteria for bcmFieildQualifyVpn
 *      qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (IN) Qualifier match data.
 *      mask - (IN) Qualifier match data.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_Vpn(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_vpn_t data, 
    bcm_vpn_t mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_SrcIp6Low
 * Purpose:
 *      Set match criteria for bcmFieildQualifySrcIp6Low
 *                     qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_sbx_field_qualify_SrcIp6Low(int unit, bcm_field_entry_t entry,
                                 bcm_ip6_t data, bcm_ip6_t mask)
{
    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *      bcm_sbx_field_qualify_DstIp6Low
 * Purpose:
 *      Set match criteria for bcmFieildQualifyDstIp6Low
 *                     qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_sbx_field_qualify_DstIp6Low(int unit, bcm_field_entry_t entry,
                                 bcm_ip6_t data, bcm_ip6_t mask)
{
    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *      bcm_sbx_field_qualify_SrcIp6Low_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcIp6Low
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_SrcIp6Low_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip6_t *data, 
    bcm_ip6_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_sbx_field_qualify_DstIp6Low_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstIp6Low
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_sbx_field_qualify_DstIp6Low_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip6_t *data, 
    bcm_ip6_t *mask)
{
    return BCM_E_UNAVAIL; 
}


#ifdef BROADCOM_DEBUG

/*
 * Field debugging
 */

int
bcm_sbx_field_show(int unit,
               const char *pfx)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_entry_dump(int unit,
                         bcm_field_entry_t entry)
{
    return BCM_E_UNAVAIL;
}

int
bcm_sbx_field_group_dump(int unit,
                         bcm_field_group_t group)
{
    return BCM_E_UNAVAIL;
}

#endif /* BROADCOM_DEBUG */
