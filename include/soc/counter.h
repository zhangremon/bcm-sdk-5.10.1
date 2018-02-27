/*
 * $Id: counter.h 1.30.2.2 Broadcom SDK $
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
 * File:        counter.h
 * Purpose:     Defines map of counter register addresses, as well
 *              as structures and routines for driver management of
 *              packet statistics counters.
 */

#ifndef _SOC_COUNTER_H
#define _SOC_COUNTER_H

#include <soc/defs.h>
#include <soc/types.h>
#include <sal/types.h>

#define SOC_REG_IS_COUNTER(unit, socreg) \
        ((SOC_REG_INFO(unit, socreg).flags & SOC_REG_FLAG_COUNTER) && \
         (SOC_REG_INFO(unit, socreg).regtype != soc_cpureg))

#define SOC_COUNTER_INVALID(unit, reg) \
        ((reg == INVALIDr) || !SOC_REG_IS_ENABLED(unit, reg))

#define SOC_COUNTER_EXTRA_CB_MAX         3

/*
 * Counter map structure.
 * Each chip has an array of these structures indexed by soc_ctr_type_t
 */

typedef struct soc_ctr_ref_s {
    soc_reg_t  reg;
    int        index;
} soc_ctr_ref_t;

typedef struct soc_cmap_s {
    soc_ctr_ref_t  *cmap_base;	/* The array of counters as they are DMA'd */
    int	            cmap_size;	/* The number of elements in the array */
} soc_cmap_t;

/*
 * Non CMIC counter DMA supported counters
 * These counters are included by show counter output
 */
typedef enum soc_counter_non_dma_id_e {
    SOC_COUNTER_NON_DMA_START = NUM_SOC_REG,
    SOC_COUNTER_NON_DMA_EGR_PERQ_XMT_PKT = SOC_COUNTER_NON_DMA_START,
    SOC_COUNTER_NON_DMA_EGR_PERQ_XMT_BYTE,
    SOC_COUNTER_NON_DMA_COSQ_DROP_PKT,
    SOC_COUNTER_NON_DMA_COSQ_DROP_BYTE,
    SOC_COUNTER_NON_DMA_COSQ_DROP_PKT_UC,
    SOC_COUNTER_NON_DMA_COSQ_DROP_BYTE_UC,
    SOC_COUNTER_NON_DMA_PORT_DROP_PKT_ING,
    SOC_COUNTER_NON_DMA_PORT_DROP_BYTE_ING,
    SOC_COUNTER_NON_DMA_PORT_DROP_PKT_IBP,
    SOC_COUNTER_NON_DMA_PORT_DROP_PKT_CFAP,
    SOC_COUNTER_NON_DMA_PORT_DROP_PKT,
    SOC_COUNTER_NON_DMA_PORT_DROP_PKT_YELLOW,
    SOC_COUNTER_NON_DMA_PORT_DROP_PKT_RED,
    SOC_COUNTER_NON_DMA_PORT_WRED_PKT_GREEN,
    SOC_COUNTER_NON_DMA_PORT_WRED_PKT_YELLOW,
    SOC_COUNTER_NON_DMA_PORT_WRED_PKT_RED,
    SOC_COUNTER_NON_DMA_POOL_PEAK,
    SOC_COUNTER_NON_DMA_POOL_CURRENT,
    SOC_COUNTER_NON_DMA_PG_MIN_PEAK,
    SOC_COUNTER_NON_DMA_PG_MIN_CURRENT,
    SOC_COUNTER_NON_DMA_PG_SHARED_PEAK,
    SOC_COUNTER_NON_DMA_PG_SHARED_CURRENT,
    SOC_COUNTER_NON_DMA_PG_HDRM_PEAK,
    SOC_COUNTER_NON_DMA_PG_HDRM_CURRENT,
    SOC_COUNTER_NON_DMA_QUEUE_PEAK,
    SOC_COUNTER_NON_DMA_QUEUE_CURRENT,
    SOC_COUNTER_NON_DMA_UC_QUEUE_PEAK,
    SOC_COUNTER_NON_DMA_UC_QUEUE_CURRENT,
    SOC_COUNTER_NON_DMA_EXT_QUEUE_PEAK,
    SOC_COUNTER_NON_DMA_EXT_QUEUE_CURRENT,
    SOC_COUNTER_NON_DMA_COSQ_WRED_PKT_RED,
    SOC_COUNTER_NON_DMA_END
} soc_counter_non_dma_id_t;

/* flags for soc_counter_non_dma_t.flags */
#define _SOC_COUNTER_NON_DMA_VALID        0x0001
#define _SOC_COUNTER_NON_DMA_DO_DMA       0x0002
#define _SOC_COUNTER_NON_DMA_ALLOC        0x0004
#define _SOC_COUNTER_NON_DMA_PEAK         0x0008
#define _SOC_COUNTER_NON_DMA_CURRENT      0x0010
#define _SOC_COUNTER_NON_DMA_PERQ_REG     0x0020 /* for register with variable
                                                  * number of queue per port */

/* one structure per each soc_counter_non_dma_id_t */
typedef struct soc_counter_non_dma_s {
    uint32 flags;   /* _SOC_COUNTER_NON_DMA_XXX */
    soc_pbmp_t pbmp;
    int base_index; /* first index into soc_control_t.counter_val[] and etc. */
    int entries_per_port; /* max possible number of entries per port */
    int num_entries;
    soc_mem_t mem;
    soc_reg_t reg;
    soc_field_t field;
    char *cname;
    uint32 *dma_buf[2];
    int dma_index_min[2];
    int dma_index_max[2];
    soc_mem_t dma_mem[2];
} soc_counter_non_dma_t;

/*
 * Nomenclature:
 *
 *   Counter register: enum value such as RPKTr
 *   Counter port_offset: Physical S-Channel address with port included
 *   Counter offset: Physical S-Channel address, lower 8 bits only
 *   Counter port_index: Index of counter in DMA buffer given port
 *   Counter index: Index of counter in counter map
 */

#define COUNTER_OFF_MIN_STRATA          0x20   /* Strata stat reg offset */
#define COUNTER_OFF_MIN_DRACO           0x20   /* Draco stat offs, 0x20-0x75 */
#define COUNTER_OFF_MIN_TUCANA          0x20
#define COUNTER_OFF_MIN_DEFAULT         0x20   /* Default matches all above */
#define COUNTER_OFF_MIN_HERC            0x09   /* Herc stat offs, 0x09-0x49 */
#define COUNTER_OFF_MIN_LYNX            0x1d0  /* Lynx stat offs, 0x1d0-0x239 */
#define COUNTER_OFF_MIN_HERC15          0x1d0  /* Herc15 stat, 0x1d0-0x23d */


/*
 * Counter thread control
 */
#define SOC_COUNTER_F_DMA               0x00000001
#define SOC_COUNTER_F_SWAP64		0x00010000	/* DMA buf 64 bit vals
							 * must be word swapped
							 * (internal only)
							 */
#define SOC_COUNTER_F_HOLD              0x00020000      /* Special handling of
                                                         * HOLD counter
							 * (internal only)
                                                         */

typedef enum soc_ctr_type_e {   
    SOC_CTR_TYPE_FE,
    SOC_CTR_TYPE_GE,
    SOC_CTR_TYPE_GFE,
    SOC_CTR_TYPE_HG,
    SOC_CTR_TYPE_XE,
    SOC_CTR_TYPE_CPU,
    SOC_CTR_TYPE_RX,
    SOC_CTR_TYPE_TX,    
    SOC_CTR_NUM_TYPES   /* Last please */
} soc_ctr_type_t;

#define	SOC_CTR_TYPE_NAMES_INITIALIZER { \
	"FE",	\
	"GE",	\
	"GFE",	\
	"HG",	\
	"XE",	\
	"CPU",	\
	"RX",   \
	"TX",   \
	"XGE",	\
	}

extern int soc_counter_attach(int unit);
extern int soc_counter_detach(int unit);
extern int soc_counter_start(int unit, uint32 flags,
			     int interval, pbmp_t pbmp);
extern int soc_counter_status(int unit, uint32 *flags,
			      int *interval, pbmp_t *pbmp);
extern int soc_counter_sync(int unit);
extern int soc_counter_stop(int unit);

/*
 * Routines to fetch counter values.
 * These return non-zero if the counter changed since the last 'get'.
 */

extern int soc_counter_get(int unit, soc_port_t port, soc_reg_t ctr_reg,
                           int ar_idx, uint64 *val);
extern int soc_counter_get32(int unit, soc_port_t port, soc_reg_t ctr_reg,
                             int ar_idx, uint32 *val);
extern int soc_counter_get_zero(int unit, soc_port_t port, soc_reg_t ctr_reg, 
                                int ar_idx, uint64 *val);
extern int soc_counter_get32_zero(int unit, soc_port_t port, soc_reg_t ctr_reg, 
                                  int ar_idx, uint32 *val);
extern int soc_counter_get_rate(int unit, soc_port_t port, soc_reg_t ctr_reg,
                                int ar_idx, uint64 *rate);
extern int soc_counter_get32_rate(int unit, soc_port_t port, soc_reg_t ctr_reg,
                                  int ar_idx, uint32 *rate);

/*
 * Routines to set counter values, normally used to clear them to zero.
 * These work by reading the counters to synchronize the software and
 * hardware counters, then set the software counters.
 */

extern int soc_counter_set(int unit, soc_port_t port, soc_reg_t ctr_reg,
                           int ar_idx, uint64 val);
extern int soc_counter_set32(int unit, soc_port_t port, soc_reg_t ctr_reg,
                             int ar_idx, uint32 val);
extern int soc_counter_set_by_port(int unit, pbmp_t pbmp, uint64 val);
extern int soc_counter_set32_by_port(int unit, pbmp_t pbmp, uint32 val);
extern int soc_counter_set_by_reg(int unit, soc_reg_t ctr_reg,
                                  int ar_idx, uint64 val);
extern int soc_counter_set32_by_reg(int unit, soc_reg_t ctr_reg,
                                    int ar_idx, uint32 val);

extern int soc_port_cmap_set(int unit, soc_port_t port, soc_ctr_type_t ctype);
extern soc_cmap_t *soc_port_cmap_get(int unit, soc_port_t port);

extern int soc_counter_idx_get(int unit, soc_reg_t reg, int ar_idx, int port);

/*
 * Driver internal and debugging use only
 */

extern int soc_counter_autoz(int unit, int enable);
extern int soc_counter_timestamp_get(int unit, soc_port_t port,
                                     uint32 *timestamp);

/* For registering additional counter collection */
typedef void (*soc_counter_extra_f)(int unit);

extern int soc_counter_extra_register(int unit, soc_counter_extra_f fn);
extern int soc_counter_extra_unregister(int unit, soc_counter_extra_f fn);

#ifdef BROADCOM_DEBUG
extern void _soc_counter_verify(int unit); /* Verify counter map consistent */
#endif /* BROADCOM_DEBUG */

/* bcm5396 MIB group */
#define SOC_COUNTER_BCM5396_GP0     0
#define SOC_COUNTER_BCM5396_GP1     1
#define SOC_COUNTER_BCM5396_GP2     2

extern int soc_robo_port_cmap_set(int unit, soc_port_t port, soc_ctr_type_t ctype);
extern soc_cmap_t *soc_robo_port_cmap_get(int unit, soc_port_t port);

#ifdef BCM_TB_SUPPORT
extern int soc_tb_cmap_set(int unit, soc_ctr_type_t ctype);
extern soc_cmap_t *soc_tb_cmap_get(int unit, int type);
#endif

extern int soc_robo_counter_get(int unit, soc_port_t port, soc_reg_t ctr_reg, uint64 *val);
extern int soc_robo_counter_get32(int unit, soc_port_t port, soc_reg_t ctr_reg,
                  uint32 *val);
extern int soc_robo_counter_get_zero(int unit, soc_port_t port,
		     soc_reg_t ctr_reg, uint64 *val);
extern int soc_robo_counter_get32_zero(int unit, soc_port_t port,
		       soc_reg_t ctr_reg, uint32 *val);
extern int soc_robo_counter_get_rate(int unit, soc_port_t port,
		     soc_reg_t ctr_reg, uint64 *rate);
extern int soc_robo_counter_get32_rate(int unit, soc_port_t port,
		       soc_reg_t ctr_reg, uint32 *rate);
extern int soc_robo_counter_set(int unit, soc_port_t port, soc_reg_t ctr_reg,
                uint64 val);
extern int soc_robo_counter_set32(int unit, soc_port_t port, soc_reg_t ctr_reg,
                  uint32 val);
extern int soc_robo_counter_set_by_port(int unit, pbmp_t pbmp, uint64 val);
extern int soc_robo_counter_set32_by_port(int unit, pbmp_t pbmp, uint32 val);
extern int soc_robo_counter_set_by_reg(int unit, soc_reg_t ctr_reg, uint64 val);
extern int soc_robo_counter_set32_by_reg(int unit, soc_reg_t ctr_reg, uint32 val);
extern int soc_robo_counter_start(int unit, uint32 flags, int interval, pbmp_t pbmp);
extern int soc_robo_counter_sync(int unit);
extern int soc_robo_counter_stop(int unit);

extern int soc_robo_counter_attach(int unit);
extern int soc_robo_counter_detach(int unit);

extern int soc_robo_counter_idx_get(int unit, soc_reg_t reg, int port);
extern int soc_robo_counter_status(int unit, uint32 *flags,
			      int *interval, pbmp_t *pbmp);

extern int soc_robo_counter_prev_get(int unit, int port, soc_reg_t ctr_reg, uint64 *val);
extern int soc_robo_counter_prev_set(int unit, int port, soc_reg_t ctr_reg, uint64 val);

#endif  /* !_SOC_COUNTER_H */
