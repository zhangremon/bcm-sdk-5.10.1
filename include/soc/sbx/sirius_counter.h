/* 
 * $Id: sirius_counter.h 1.7.2.1 Broadcom SDK $
 *
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
 * File:        sirius_counter.h
 * Purpose:     SBX Software Counter Collection module definitions for SIRIUS.
 */

#ifndef   _SOC_SBX_SIRIUS_COUNTER_H_
#define   _SOC_SBX_SIRIUS_COUNTER_H_

#include <soc/sbx/counter.h>

#define CS_DMA_CNTR_MSG_LAST          0x1
#define CS_DMA_CNTR_MSG_DUMMY         0x2
#define CS_DMA_CNTR_MSG_UNUSED        0xC
#define CS_DMA_CNTR_ADDR_MASK        ~(CS_DMA_CNTR_MSG_LAST  | \
                                       CS_DMA_CNTR_MSG_DUMMY | \
				       CS_DMA_CNTR_MSG_UNUSED)

#define CS_DMA_FIFO_MSG_SIZE           (sizeof(soc_sbx_cs_rbuf_entry_t)/4)
#define CS_DMA_FIFO_MEM_ADDR(mem_addr) ((mem_addr) & CS_DMA_CNTR_ADDR_MASK)
#define CS_CFG_GRP_MASK                0xFFFFFC00
#define CS_CFG_GRP_SHIFT               10
#define CS_GRP_SIZE                    (1 << CS_CFG_GRP_SHIFT)
#define CS_SLQ_ENABLE                  0x02
#define CS_GBL_ENABLE                  0x04
#define CS_FLUSHING                    0x08

typedef enum cs_grp_state_s {
  CS_GROUP_FREE,
  CS_GROUP_PROVISIONING,
  CS_GROUP_ACTIVE
} cs_grp_state_t;

typedef enum gbl_stats_s {
  gbl_accepted_dp0,
  gbl_accepted_dp1,
  gbl_accepted_dp2,
  gbl_accepted_dp3,
  gbl_wred_drop_dp0,
  gbl_wred_drop_dp1,
  gbl_wred_drop_dp2,
  gbl_wred_drop_dp3,
  gbl_wred_mark_dp0,
  gbl_wred_mark_dp1,
  gbl_wred_mark_dp2,
  gbl_wred_mark_dp3,
  gbl_nwred_drop_dp0,
  gbl_nwred_drop_dp1,
  gbl_nwred_drop_dp2,
  gbl_nwred_drop_dp3,
  gbl_dequeue
} gbl_stats_t;

typedef enum slq_stats_s {
  slq_accepted_dp0,
  slq_marked_dp0,
  slq_dropped_dp0,
  slq_accepted_dp1,
  slq_marked_dp1,
  slq_dropped_dp1,
  slq_accepted_dp2,
  slq_marked_dp2,
  slq_dropped_dp2,
  slq_accepted_dp3,
  slq_marked_dp3,
  slq_dropped_dp3,
  slq_tail_drop,
  slq_oversubscribetotal,
  slq_oversubscriptguarantee,
  slq_dequeue
} slq_stats_t;

typedef enum dp_color_s {
  ADD_GREEN,
  ADD_YELLOW,
  ADD_RED,
  ADD_BLACK,
  DROP_GREEN,
  DROP_YELLOW,
  DROP_RED,
  DROP_BLACK
} dp_color_t;

typedef struct soc_sbx_cs_rbuf_entry_s {
    uint32           entry_pkts;
    uint32           entry_bytes;
    uint32           mem_addr;
} soc_sbx_cs_rbuf_entry_t;

typedef struct soc_sbx_sirius_cs_update_s {
    uint64           bytes64;
    uint64           pkts64;
} soc_sbx_sirius_cs_update_t;

extern int soc_sbx_sirius_counter_init(int unit, 
				       uint32 flags, 
				       int interval,
				       pbmp_t pbmp);
extern int soc_sbx_process_custom_stats(int unit, int links);
extern int soc_sbx_sirius_process_slq_stats(int unit);
extern int soc_sbx_sirius_process_global_stats(int unit);
extern int soc_sbx_sirius_process_fd_drop_stats(int unit, int clear);
extern int soc_sbx_sirius_get_segment(uint32 unit, 
				      uint8 cu,
				      int32_t *rSegment);
extern int soc_sbx_sirius_free_segment(uint32 unit, 
				       uint32 segment);
extern int soc_sbx_sirius_create_group(uint32 unit, 
					 uint32 segment, 
					 uint32 cu_num, 
					 uint32 cntrId, 
					 uint32 size);
extern int soc_sbx_sirius_remove_group(uint32 unit, 
				       uint32 segment, 
				       uint32 cu_num, 
				       uint32 cntrId, 
				       uint32 size);
extern int soc_sbx_sirius_provision_group(uint32 unit, 
					    uint32 segment, 
					    uint64 *ullCntAddr);
extern int soc_sbx_sirius_init_group(uint32 unit, 
				     uint32 segment, 
				     uint32 cntr_id, 
				     uint32 num_cntrs);
extern int soc_sbx_sirius_flush_segment(uint32 unit, 
					uint32 segment, 
					uint32 cntrId, 
					uint32 num_cntrs);

#define    SIRIUS_COUNTER_QM_COUNT 32

int soc_sbx_sirius_test(int unit);

extern uint32 soc_sirius_qm_counter_base_set(int unit,
					     int32 nBaseQueue,
					     int32 enable);

extern uint32 soc_sirius_qm_counter_base_get(int unit, int32 *nBaseQueue);

extern uint32 soc_sirius_qm_counter_read(int unit, int32 set, uint32 *puCounterBase);

extern int soc_sbx_sirius_counter_port_get(int unit, int port, int block, int set,
                                           int counter, uint64 *val);

extern int soc_sbx_sirius_counter_port_set(int unit, int port, int block , int set,
                                           int counter, uint64 val);

extern int soc_sbx_sirius_counter_bset_collect( int unit, int discard, int block, int set);

extern int soc_sirius_counter_enable_get(int unit, int *base, int *result);

extern int soc_sirius_counter_enable_set(int unit, int base );

extern int soc_sirius_counter_enable_clear(int unit);

#endif /* _SOC_SBX_SIRIUS_COUNTER_H_ */
