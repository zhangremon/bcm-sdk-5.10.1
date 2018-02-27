/*
 * $Id: g2p3.h 1.38 Broadcom SDK $
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
 * This file contains aggregated definitions for Guadalupe 2.x microcode
 */

#ifndef _SOC_SBX_G2P3_H
#define _SOC_SBX_G2P3_H

#ifdef BCM_FE2000_SUPPORT

typedef struct soc_sbx_g2p3_state_s soc_sbx_g2p3_state_t;
#ifdef BCM_SBX_MPLSTP_SUPPORT
#include <soc/sbx/g2p3/c2_g2p3_mplstp_defs.h>
#else
#include <soc/sbx/g2p3/g2p3_defs.h>
#endif
#include <soc/sbx/g2p3/g2p3_soc_cls.h>
#include <soc/sbx/g2p3/g2p3_ace.h>
#include <soc/sbx/g2p3/g2p3_cls.h>
#include <soc/sbx/g2p3/g2p3_pp.h>

#define SOC_SBX_G2P3_NOP_POLICER_ID 0

/* for creating unique SCACHE handles */
typedef enum {
    SOC_SBX_G2P3_WB_COMMON = 0,
    SOC_SBX_G2P3_WB_ACE,
    SOC_SBX_G2P3_WB_CLS
}soc_sbx_g2p3_wb_id_t;

extern int soc_sbx_g2p3_drv_hw_init(int unit);
extern int soc_sbx_g2p3_drv_init(int unit);
extern int soc_sbx_g2p3_ilib_recover(int unit);
extern int soc_sbx_g2p3_ilib_init(int unit);
extern int soc_sbx_g2p3_ilib_uninit(int unit);
extern int soc_sbx_g2p3_is_fe2kxt(int unit);
extern int soc_sbx_g2p3_v4_ena(int unit);
extern int soc_sbx_g2p3_v6_ena(int unit);
extern int soc_sbx_g2p3_mplstp_ena(int unit);
extern int soc_sbx_g2p3_mim_ena(int unit);
extern int soc_sbx_g2p3_dscp_ena(int unit);
extern int soc_sbx_g2p3_topci_qid_get(int unit, int *qid);
extern int soc_sbx_g2p3_frompci_qid_get(int unit, int *qid);
extern soc_sbx_g2p3_state_t *soc_sbx_g2p3_unit2fe(int unit);

extern int soc_sbx_g2p3_cuckoo_init(int unit);
extern int
soc_sbx_g2p3_pvv2e_keys_get(int unit, int ivid, int ovid, int port,
                            uint32 *left, uint32 *right, uint32 *seed,
                            int *b);

extern int soc_sbx_g2p3_is_oam_rx_enabled(int unit);
extern int soc_sbx_g2p3_is_v4mc_str_sel(int unit);
extern int soc_sbx_g2p3_is_v4uc_str_sel(int unit);

extern int soc_sbx_g2p3_lpm_stats_clear(int unit, void *cstate);
extern int soc_sbx_g2p3_lpm_stats_dump(int unit, void *cstate, int verbose);
extern int soc_sbx_g2p3_lpm_singleton_set(int unit, void *cstate, int flag);

extern int soc_sbx_g2p3_ipv4da_singleton_set(int unit, int flag);

typedef void (*soc_sbx_g2p3_eml_old_f_t)(int unit, uint8 *index);

extern int soc_sbx_g2p3_eml_run_aging(int unit, void *cstate,
                                      int now, int old, uint32 ecount,
                                      soc_sbx_g2p3_eml_old_f_t oldcb);

extern int soc_sbx_g2p3_emirror_init(int unit);

extern int gu2_eplib_init(int unit);


extern int soc_sbx_g2p3_mim_lsm_get(int unit, int mim_port_id,int  *lsm);

extern int soc_sbx_g2p3_rtc_get(int unit, uint32 *sec, uint *nsec);

#endif /* BCM_FE2000_SUPPORT */
#endif
