/*
 * $Id: stg.h 1.2 Broadcom SDK $
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
 * This file contains STG definitions internal to the BCM library.
 */

#ifndef _BCM_INT_SBX_STG_H_
#define _BCM_INT_SBX_STG_H_

#define BCM_STG_MAX     BCM_VLAN_COUNT

/*
 * Function:
 *      _bcm_fe2000_stg_map_get
 * Purpose:
 *      Get STG that a VLAN is mapped to.
 * Parameters:
 *      unit - device unit number.
 *      vid  - VLAN id to search for
 *      *stg - Spanning tree group id if found
 * Returns: 
 *      TRUE if the VLAN is mapped to an STG or FALSE if not
 */
extern int _bcm_fe2000_stg_map_get(int unit, bcm_vlan_t vid, bcm_stg_t *stg);

/*
 * Function:
 *      bcm_fe2000_stg_default_get
 * Purpose:
 *      Returns the default STG for the device.
 * Parameters:
 *      unit    - device unit number.
 *      stg_ptr - STG ID for default.
 * Returns:
 *      BCM_E_XXX
 */
extern int bcm_fe2000_stg_default_get(int unit, bcm_stg_t *stg_ptr);

/*
 * Function:
 *      bcm_fe2000_stg_vlan_add
 * Purpose:
 *      Add a VLAN to a spanning tree group.
 * Parameters:
 *      unit - device unit number
 *      stg  - STG ID to use
 *      vid  - VLAN id to be added to STG
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Spanning tree group ID must have already been created. The 
 *      VLAN is removed from the STG it is currently in.
 */
extern int bcm_fe2000_stg_vlan_add(int unit, bcm_stg_t stg, bcm_vlan_t vid);

/*
 * Function:
 *      bcm_fe2000_stg_stp_set
 * Purpose:
 *      Set the Spanning tree state for a port in specified STG.
 * Parameters:
 *      unit      - device unit number.
 *      stg       - STG ID.
 *      port      - device port number.
 *      stp_state - Spanning Tree State of port.
 * Returns:
 *      BCM_E_XXX
 */
extern int bcm_fe2000_stg_stp_set(int unit, bcm_stg_t stg, bcm_port_t port, int stp_state);

/*
 * Function:
 *      bcm_fe2000_stg_stp_get
 * Purpose:
 *      Get the Spanning tree state for a port in specified STG.
 * Parameters:
 *      unit      - device unit number.
 *      stg       - STG ID.
 *      port      - device port number.
 *      stp_state - (Out) Pointer to where Spanning Tree State is stored.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_INTERNAL
 */
extern int bcm_fe2000_stg_stp_get(int unit, bcm_stg_t stg, bcm_port_t port, int *stp_state);

#endif	/* _BCM_INT_SBX_STG_H_ */
