/*
 * $Id: qos.h 1.2 Broadcom SDK $
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
 * File:        qos.h
 * Purpose:     qos internally exported interface
 *              specific to FE2000 BCM API implementation
 */

#ifndef _BCM_INT_SBX_FE2000_QOS_H_
#define _BCM_INT_SBX_FE2000_QOS_H_

#include <bcm/qos.h>

/*
 *  We must differentiate by ID the ingress versus egress QoS maps.  This is
 *  because they are different, and the settings are managed by ID only.
 *
 *  This value is added to the ID on an egress QoS map.
 */
#define QOS_MAP_ID_EGRESS_OFFSET 0x00001000

/*
 *   Function
 *      _bcm_fe2000_qos_map_id_to_hw_id
 *   Purpose
 *      temporary translation api for use by port while its qos
 *      api is still suppported.
 *   Parameters
 *      (in)  flags          - BCM_QOS_MAP_[INGRESS|EGRESS]
 *      (in)  map_id         - qos map id to be translated.
 *   Returns
 *       hardware index for given resource
 */
int _bcm_fe2000_qos_map_id_to_hw_id(int flags, int map_id);

/*
 *   Function
 *      _bcm_fe2000_qos_hw_id_to_map_id
 *   Purpose
 *      temporary translation api for use by port while its qos
 *      api is still suppported.
 *   Parameters
 *      (in)  flags          - BCM_QOS_MAP_[INGRESS|EGRESS]
 *      (in)  hw_id          - hardware id to be translated.
 *   Returns
 *       qos map id for given resource
 */
int _bcm_fe2000_qos_hw_id_to_map_id(int flags, int hw_id);

#endif
