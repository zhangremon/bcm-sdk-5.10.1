/*
 * $Id: fe2000.h 1.15 Broadcom SDK $
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
 * FE2000 (BCM88020) SOC Initialization
 */
#ifndef _FE2000_H
#define _FE2000_H
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/fe2k_common/sbFe2000UcodeLoad.h>
#include <soc/cmtypes.h>

typedef enum soc_sbx_fe2000_isr_type_e {
    /* 0-12 correspond to pc_interrupt bits */
    SOC_SBX_FE2000_ISR_TYPE_LAST_IN_PC = 13,
    SOC_SBX_FE2000_ISR_TYPE_PMU_WATCHDOG = 14,
    SOC_SBX_FE2000_ISR_TYPE_CMU = 15,
    SOC_SBX_FE2000_ISR_TYPE_MAX
} soc_sbx_fe2000_isr_type_t;

#define L2_AGE_CYCLES_INTERVAL_DEFAULT  1

/* 0xff -> dmac byte...and 0xf is sub types */
#define L2CACHE_MAX_L2CP_IDX (SBX_MAX_PORTS * ((0xff +1) + (0xf + 1)))

/* first half is for L2CP entries, second half is for non-L2CP entries
 * that are used to create encap_ids (remote tunneling, encap sharing etc...)
 */
#define L2CACHE_MAX_IDX_DEFAULT (2 * L2CACHE_MAX_L2CP_IDX)

#define OAM_EXPIRED_TIMER_MAX_DEPTH_DFLT     4096
#define OAM_THREAD_PRI_DFLT   200

extern sbFe2000ResetSramDllFunc_t soc_sbx_fe2000_reset_sram_dll;

extern int soc_sbx_fe2000_check_mem(int unit);
extern void soc_sbx_fe2000_check_mem_and_die(int unit);

extern int
soc_sbx_fe2000_init(int unit, soc_sbx_config_t *cfg);
extern int
soc_sbx_fe2000xt_init(int unit, soc_sbx_config_t *cfg);
extern int
soc_sbx_fe2000_detach(int unit);

extern void
soc_sbx_fe2000_register_isr(int unit, soc_sbx_fe2000_isr_type_t isrnum,
                            soc_cm_isr_func_t isr);
extern soc_cm_isr_func_t 
soc_sbx_fe2000_unregister_isr(int unit,
                              soc_sbx_fe2000_isr_type_t isrtype);

extern void
soc_sbx_fe2000_isr(void *_unit);

extern int
soc_sbx_fe2000_mac_probe(int unit, soc_port_t port, mac_driver_t **macdp);

extern int
soc_sbx_fe2000_port_info_config(int unit);

extern soc_driver_t soc_driver_bcm88020_a0;
extern soc_driver_t soc_driver_bcm88025_a0;

/*
 * Defined in 'fe2000_xmac.c'
 */
extern int
soc_fe2000_fusioncore_reset(int unit, soc_port_t port, int speed);


/*
 * Routines to read/write AM and XM block registers
 * with mutex lock protection.
 */
extern int
soc_sbx_fe2000_am_read(int unit, int32 block_num, int32 block_offset,
                       uint32 reg, uint32 *data);
extern int
soc_sbx_fe2000_am_write(int unit, int32 block_num, int32 block_offset,
                        uint32 reg, uint32 data);
extern int
soc_sbx_fe2000_xm_read(int unit, int32 block_num,
                       uint32 reg, uint32 *data_hi, uint32 *data_lo);
extern int
soc_sbx_fe2000_xm_write(int unit, int32 block_num,
                        uint32 reg, uint32 data_hi, uint32 data_lo);
extern int
soc_sbx_fe2000_xm_rxtx(int unit, int port);

extern int
soc_sbx_fe2000_ddr_train(int unit);

extern int
soc_sbx_fe2000_ddr_tune(int unit, unsigned int );

extern int
soc_sbx_fe2000_ucode_value_set(int unit, char *a_n, unsigned int value);

extern int
soc_sbx_fe2000_ucode_value_get(int unit, char *a_n, unsigned int *value);

extern int
soc_sbx_fe2000_ucode_load(int unit, void *ucode);

extern int
soc_sbx_fe2000_ucode_load_from_buffer(int unit, void *buf, unsigned int len);

extern int
soc_sbx_fe2000_egress_pages_set(int unit, int port, int mbps, int mtu);

#endif /* _FE2000_H */
