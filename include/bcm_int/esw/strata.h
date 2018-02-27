/*
 * $Id: strata.h 1.40 Broadcom SDK $
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
 * File:        strata.h
 * Purpose:     Function declarations for Strata bcm functions
 */

#ifndef _BCM_INT_STRATA_H_
#define _BCM_INT_STRATA_H_

#include <bcm_int/esw/mbcm.h>

/* Utility routines for metering and egress rate control */

extern uint8 _bcm_strata_kbits_to_bucketsize(int unit, bcm_port_t port, 
				      int ingress, uint32 kbits_burst);
extern uint32 _bcm_strata_bucketsize_to_kbits(int unit, bcm_port_t port,
				       int ingress, uint8 reg_val);
extern uint16 _bcm_strata_kbps_to_refreshcount(int unit, bcm_port_t port,
					int ingress, uint32 kbits_sec);
extern uint32 _bcm_strata_refreshcount_to_kbps(int unit, bcm_port_t port,
					int ingress, uint16 reg_val);

/****************************************************************
 *
 * Strata BCM functions
 *
 ****************************************************************/


extern int bcm_strata_port_cfg_init(int, bcm_port_t port, bcm_vlan_data_t *);
extern int bcm_strata_port_cfg_get(int, bcm_port_t, bcm_port_cfg_t *);
extern int bcm_strata_port_cfg_set(int, bcm_port_t, bcm_port_cfg_t *);

/* Meter functions */
extern int bcm_strata_meter_init(int unit);
extern int bcm_strata_meter_create(int unit, int port, int *mid);
extern int bcm_strata_meter_delete(int unit, int port, int mid);
extern int bcm_strata_meter_delete_all(int unit);
extern int bcm_strata_meter_get(int unit, int port, int mid, 
			 uint32 *kbits_sec, uint32 *kbits_burst);
extern int bcm_strata_meter_set(int unit, int port, int mid, 
			 uint32 kbits_sec, uint32 kbits_burst);
extern void _bcm_strata_meter_size_get(int unit, int *size);
extern int bcm_strata_ffppacketcounter_set(int unit, int port, int mid, 
				    uint64 val);
extern int bcm_strata_ffppacketcounter_get(int unit, int port, int mid, 
				    uint64 *val);
extern int bcm_strata_ffpcounter_init(int unit);
extern int bcm_strata_ffpcounter_create(int unit, int port, int *ffpcounterid);
extern int bcm_strata_ffpcounter_delete(int unit, int port, int ffpcounterid);
extern int bcm_strata_ffpcounter_delete_all(int unit);
extern int bcm_strata_ffpcounter_set(int unit, int port, int ffpcounterid,
			      uint64 val);
extern int bcm_strata_ffpcounter_get(int unit, int port, int ffpcounterid,
			      uint64 *val);
extern int bcm_strata_port_rate_egress_set(int unit, bcm_port_t port,
				    uint32 kbits_sec,
				    uint32 kbits_burst);
extern int bcm_strata_port_rate_egress_get(int unit, bcm_port_t port,
				    uint32 *kbits_sec,
				    uint32 *kbits_burst);
#endif	/* !_BCM_INT_STRATA_H_ */
