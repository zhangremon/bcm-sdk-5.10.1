
/*
 * $Id: field.c 1.8.232.1 Broadcom SDK $
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
 * Field - Broadcom StrataSwitch Field Processor switch common API.
 */
#include <soc/types.h>
#include <soc/ll.h>

#include <bcm/field.h>

/*
 * Function:
 *      bcm_field_udf_spec_t_init
 * Purpose:
 *      Initialize a UDF data type.
 */

void 
bcm_field_udf_spec_t_init(bcm_field_udf_spec_t *udf_spec)
{
    if (NULL != udf_spec) {
        sal_memset(udf_spec, 0, sizeof(bcm_field_udf_spec_t));
    }
    return;
}

/*
 * Function:
 *      bcm_field_llc_header_t_init
 * Purpose:
 *      Initialize the bcm_field_llc_header_t structure.
 * Parameters:
 *      llc_header - Pointer to llc header structure.
 * Returns:
 *      NONE
 */
void
bcm_field_llc_header_t_init(bcm_field_llc_header_t *llc_header)
{
    if (llc_header != NULL) {
        sal_memset(llc_header, 0, sizeof (*llc_header));
    }
    return;
}

/*
 * Function:
 *      bcm_field_snap_header_t_init
 * Purpose:
 *      Initialize the bcm_field_snap_header_t structure.
 * Parameters:
 *      snap_header - Pointer to snap header structure.
 * Returns:
 *      NONE 
 */
void
bcm_field_snap_header_t_init(bcm_field_snap_header_t *snap_header)
{
    if (snap_header != NULL) {
        sal_memset(snap_header, 0, sizeof (*snap_header));
    }
    return;
}

/*
 * Function:
 *      bcm_field_qset_t_init
 * Purpose:
 *      Initialize the bcm_field_qset_t structure.
 * Parameters:
 *      qset - Pointer to field qset structure.
 * Returns:
 *      NONE
 */
void
bcm_field_qset_t_init(bcm_field_qset_t *qset)
{
    if (qset != NULL) {
        sal_memset(qset, 0, sizeof (*qset));
    }
    return;
}

/*
 * Function:
 *      bcm_field_status_t_init
 * Purpose:
 *      Initialize the Field Status structure.
 * Parameters:
 *      fstatus - Pointer to field status structure.
 * Returns:
 *      NONE
 */
void
bcm_field_status_t_init(bcm_field_status_t *fstatus)
{
    if (fstatus != NULL) {
        sal_memset(fstatus, 0, sizeof (*fstatus));
    }
    return;
}

/*
 * Function:
 *      bcm_field_group_status_t_init
 * Purpose:
 *      Initialize the Field Group Status structure.
 * Parameters:
 *      fgroup - Pointer to Field Group Status structure.
 * Returns:
 *      NONE
 */
void
bcm_field_group_status_t_init(bcm_field_group_status_t *fgroup)
{
    if (fgroup != NULL) {
        sal_memset(fgroup, 0, sizeof (*fgroup));
    }
    return;
}

/*
 * Function:
 *      bcm_field_data_qualifier_t
 * Purpose:
 *      Initialize the Field Data Qualifier structure.
 * Parameters:
 *      data_qual - Pointer to field data qualifier structure.
 * Returns:
 *      NONE
 */
void
bcm_field_data_qualifier_t_init(bcm_field_data_qualifier_t *data_qual)
{
    if (data_qual != NULL) {
        sal_memset(data_qual, 0, sizeof (bcm_field_data_qualifier_t));
    }
    return;
}

/*
 * Function:
 *      bcm_field_data_ethertype_t_init
 * Purpose:
 *      Initialize ethertype based field data qualifier. 
 * Parameters:
 *      etype - Pointer to ethertype based data qualifier structure.
 * Returns:
 *      NONE
 */
void
bcm_field_data_ethertype_t_init (bcm_field_data_ethertype_t *etype)
{
    if (etype != NULL) {
        sal_memset(etype, 0, sizeof (bcm_field_data_ethertype_t));
    }
    return;
}

/*
 * Function:
 *      bcm_field_data_ip_protocol_t_init
 * Purpose:
 *      Initialize ip protocol based field data qualifier. 
 * Parameters:
 *      etype - Pointer to ip_protocol based data qualifier structure.
 * Returns:
 *      NONE
 */
void
bcm_field_data_ip_protocol_t_init (bcm_field_data_ip_protocol_t *ip_protocol)
{
    if (ip_protocol != NULL) {
        sal_memset(ip_protocol, 0, sizeof (bcm_field_data_ip_protocol_t));
    }
    return;
}

/*
 * Function:
 *      bcm_field_data_packet_format_t_init
 * Purpose:
 *      Initialize packet format based field data qualifier. 
 * Parameters:
 *      packet_format - Pointer to packet_format based data qualifier structure.
 * Returns:
 *      NONE
 */
void
bcm_field_data_packet_format_t_init (bcm_field_data_packet_format_t *packet_format)
{
    if (packet_format != NULL) {
        sal_memset(packet_format, 0, sizeof (bcm_field_data_packet_format_t));
    }
    return;
}

/*
 * Function:
 *      bcm_field_group_config_t_init
 * Purpose:
 *      Initialize the Field Group Config structure.
 * Parameters:
 *      data_qual - Pointer to field group config structure.
 * Returns:
 *      NONE
 */
void
bcm_field_group_config_t_init(bcm_field_group_config_t *group_config)
{
    if (group_config != NULL) {
        sal_memset(group_config, 0, sizeof (bcm_field_group_config_t));
    }
    return;
}
