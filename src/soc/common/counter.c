/*
 * $Id: counter.c 1.62.2.9 Broadcom SDK $
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
 * Packet Statistics Counter Management
 *
 * Strata:      256 bytes per port (64 * uint32)
 * Draco:       512 bytes per port (128 * uint32, 64 * uint64)
 *              draco doesn't use dma for uint64 counters because there
 *              are more than 64 of them
 * Easyrider    1024 bytes per port (128 * uint64)
 * Firebolt     1024 bytes per port (128 * uint64)
 * Helix        1024 bytes per port (128 * uint64)
 * Felix        1024 bytes per port (128 * uint64)
 * Raptor       1024 bytes per port (128 * uint64)
 * Draco15:     1024 bytes per port (256 * uint32, 128 * uint64)
 * Hercules:    1024 bytes per port (128 * uint64)
 * Lynx:        1024 bytes per port (128 * uint64)
 * Tucana:      2048 bytes per port (256 * uint64)
 * HUMV:        2048 bytes per port (256 * uint64)
 * Bradley:     2048 bytes per port (256 * uint64)
 * Goldwing:    2048 bytes per port (256 * uint64)
 * Triumph:     4096 bytes per port (512 * uint64)
 *
 * BCM5670 Endian: on Hercules, the well-intentioned ES_BIG_ENDIAN_OTHER
 * causes counter DMA to write the most significant word first in the
 * DMA buffer.  This is wrong because not all big-endian hosts have the
 * same setting for ES_BIG_ENDIAN_OTHER (e.g. mousse and idtrp334).
 * This problem makes it necessary to check the endian select to
 * determine whether to swap words.
 */

#include <sal/core/libc.h>
#include <shared/alloc.h>
#include <sal/core/spl.h>
#include <sal/core/sync.h>
#include <sal/core/time.h>

#include <soc/drv.h>
#include <soc/counter.h>
#include <soc/ll.h>
#include <soc/debug.h>
#include <soc/mem.h>
#ifdef BCM_TRIDENT_SUPPORT
#include <soc/trident.h>
#endif
#ifdef BCM_CMICM_SUPPORT
#include <soc/cmicm.h>
#endif

#if defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT)
#define COUNTER_IDX_PORTBASE(unit, port) \
        ((port) * SOC_CONTROL(unit)->counter_perport)

#define COUNTER_IDX_GET(unit, ctr_ref, port) \
        (COUNTER_IDX_PORTBASE(unit, port) + \
         SOC_REG_CTR_IDX(unit, (ctr_ref)->reg) + (ctr_ref)->index)

#define COUNTER_IDX_OFFSET(unit)        \
        (SOC_IS_HERCULES1(unit) ? COUNTER_OFF_MIN_HERC : 0)

#define COUNTER_MIN_IDX_GET(unit, port) \
        (COUNTER_IDX_PORTBASE(unit, port) + COUNTER_IDX_OFFSET(unit))

static soc_counter_extra_f soc_counter_extra[SOC_MAX_NUM_DEVICES]
                                            [SOC_COUNTER_EXTRA_CB_MAX];
/* Number of cosq per unit per port */
static int num_cosq[SOC_MAX_NUM_DEVICES][SOC_MAX_NUM_PORTS] = {{ 0 }};

/*
 * Turn on COUNTER_BENCH and "debug +verbose" to benchmark the CPU time
 * spent on counter activity.  NOTE: most useful on platforms where
 * sal_time_usecs() has usec precision instead of clock tick precision.
 */

#undef COUNTER_BENCH

/* Per port mapping to counter map structures */
#define PORT_CTR_REG(unit, port, idx) \
        (&SOC_CONTROL(unit)->counter_map[port]->cmap_base[idx])
#define PORT_CTR_NUM(unit, port) \
        (SOC_CONTROL(unit)->counter_map[port]->cmap_size)

#if defined(BCM_BRADLEY_SUPPORT) || defined(BCM_TRX_SUPPORT)

static uint64 *soc_counter_tbuf[SOC_MAX_NUM_DEVICES];
#define SOC_COUNTER_TBUF_SIZE(unit) \
    (SOC_CONTROL(unit)->counter_perport * sizeof(uint64))

#ifdef BCM_TRIUMPH2_SUPPORT
#define COUNTER_TIMESTAMP_FIFO_SIZE     4
#endif /* BCM_TRIUMPH2_SUPPORT */

int
is_xaui_rx_counter(soc_reg_t ctr_reg)
{
    switch (ctr_reg) {
    case IR64r:
    case IR127r:
    case IR255r:
    case IR511r:
    case IR1023r:
    case IR1518r:
    case IR2047r:
    case IR4095r:
    case IR9216r:
    case IR16383r:
    case IRBCAr:
    case IRBYTr:
    case IRERBYTr:
    case IRERPKTr:
    case IRFCSr:
    case IRFLRr:
    case IRFRGr:
    case IRJBRr:
    case IRJUNKr:
    case IRMAXr:
    case IRMCAr:
    case IRMEBr:
    case IRMEGr:
    case IROVRr:
    case IRPKTr:
    case IRUNDr:
    case IRXCFr:
    case IRXPFr:
    case IRXUOr:
#ifdef BCM_TRX_SUPPORT
    case IRUCr:
#ifdef BCM_ENDURO_SUPPORT
    case IRUCAr:
#endif /* BCM_ENDURO_SUPPORT */
    case IRPOKr:
    case MAC_RXLLFCMSGCNTr:
#endif
#if defined(BCM_SCORPION_SUPPORT) || defined(BCM_TRIUMPH2_SUPPORT)
    case IRXPPr:
#endif
        return 1;
    default:
        break;
    }
    return 0;
}
#endif /* BCM_BRADLEY_SUPPORT || BCM_TRX_SUPPORT */

#ifdef BCM_TRIDENT_SUPPORT
STATIC int
_soc_counter_trident_get_info(int unit, soc_port_t port, soc_reg_t id,
                              int *base_index, int *num_entries)
{
    soc_control_t *soc;
    soc_info_t *si;
    soc_counter_non_dma_t *non_dma;
    soc_port_t phy_port, mmu_port;
    soc_port_t mmu_cmic_port, mmu_lb_port;
    egr_perq_xmt_counters_base_addr_entry_t base_addr_entry;

    soc = SOC_CONTROL(unit);
    non_dma = &soc->counter_non_dma[id - SOC_COUNTER_NON_DMA_START];

    if (!(non_dma->flags & _SOC_COUNTER_NON_DMA_VALID)) {
        return SOC_E_UNAVAIL;
    }

    si = &SOC_INFO(unit);
    mmu_cmic_port = si->port_p2m_mapping[si->port_l2p_mapping[si->cmic_port]];
    mmu_lb_port = si->port_p2m_mapping[si->port_l2p_mapping[si->lb_port]];

    phy_port = si->port_l2p_mapping[port];
    mmu_port = si->port_p2m_mapping[phy_port];

    switch (id) {
    case SOC_COUNTER_NON_DMA_EGR_PERQ_XMT_PKT:
    case SOC_COUNTER_NON_DMA_EGR_PERQ_XMT_BYTE:
        SOC_IF_ERROR_RETURN
            (READ_EGR_PERQ_XMT_COUNTERS_BASE_ADDRm(unit, MEM_BLOCK_ANY,
                                                   port << 2,
                                                   &base_addr_entry));
        *base_index = soc_mem_field32_get(unit,
                                          EGR_PERQ_XMT_COUNTERS_BASE_ADDRm,
                                          &base_addr_entry, BASEf);
        if (mmu_port >= mmu_lb_port) { /* in Y pipe */
            *base_index += non_dma->dma_index_max[0] + 1;
        }

        if (mmu_port < mmu_lb_port) { /* in X pipe */
            if (mmu_port == mmu_cmic_port) {
                *num_entries = 48;
            } else if (mmu_port <= mmu_cmic_port + 4) {
                *num_entries = 79; /* 10 ucast + 5 mcast + 64 ext ucast */
            } else {
                *num_entries = 15; /* 10 ucast + 5 mcast */
            }
        } else { /* in Y pipe */
            if (mmu_port == mmu_lb_port) {
                *num_entries = 9;
            } else if (mmu_port <= mmu_lb_port + 4) { /* extended ports */
                *num_entries = 79; /* 10 ucast + 5 mcast + 64 ext ucast */
            } else {
                *num_entries = 15; /* 10 ucast + 5 mcast */
            }
        }
        break;
    case SOC_COUNTER_NON_DMA_COSQ_DROP_PKT:
    case SOC_COUNTER_NON_DMA_COSQ_DROP_BYTE:
        if (mmu_port == mmu_cmic_port) {
            *base_index = 0;
            *num_entries = 48;
        } else {
            *base_index = 48 + (mmu_port - mmu_cmic_port - 1) * 5;
            *num_entries = 5;
        }
        break;
    case SOC_COUNTER_NON_DMA_COSQ_DROP_PKT_UC:
    case SOC_COUNTER_NON_DMA_COSQ_DROP_BYTE_UC:
        if (mmu_port < mmu_lb_port) { /* in X pipe */
            if (mmu_port == mmu_cmic_port) {
                *base_index = 0;
                *num_entries = 0;
            } else if (mmu_port <= mmu_cmic_port + 4) {
                *base_index = (mmu_port - mmu_cmic_port - 1) * 74;
                *num_entries = 74; /* 10 ucast + 64 ext ucast */
            } else {
                *base_index = 74 * 4 + (mmu_port - mmu_cmic_port - 5) * 10;
                *num_entries = 10;
            }
        } else { /* in Y pipe */
            if (mmu_port == mmu_lb_port) {
                *base_index = 0;
                *num_entries = 0;
            } else if (mmu_port <= mmu_lb_port + 4) {
                *base_index = 576 + (mmu_port - mmu_lb_port - 1) * 74;
                *num_entries = 74; /* 10 ucast + 64 ext ucast */
            } else {
                *base_index = 576 + 74 * 4 + (mmu_port - mmu_lb_port - 5) * 10;
                *num_entries = 10;
            }
        }
        break;
    case SOC_COUNTER_NON_DMA_PORT_DROP_PKT_ING:
    case SOC_COUNTER_NON_DMA_PORT_DROP_BYTE_ING:
        *num_entries = 1;
        *base_index = port;
        break;
    case SOC_COUNTER_NON_DMA_PORT_DROP_PKT_YELLOW:
    case SOC_COUNTER_NON_DMA_PORT_DROP_PKT_RED:
    case SOC_COUNTER_NON_DMA_PORT_WRED_PKT_GREEN:
    case SOC_COUNTER_NON_DMA_PORT_WRED_PKT_YELLOW:
    case SOC_COUNTER_NON_DMA_PORT_WRED_PKT_RED:
        *num_entries = 1;
        *base_index = mmu_port;
        break;
    case SOC_COUNTER_NON_DMA_PG_MIN_PEAK:
    case SOC_COUNTER_NON_DMA_PG_MIN_CURRENT:
    case SOC_COUNTER_NON_DMA_PG_SHARED_PEAK:
    case SOC_COUNTER_NON_DMA_PG_SHARED_CURRENT:
    case SOC_COUNTER_NON_DMA_PG_HDRM_PEAK:
    case SOC_COUNTER_NON_DMA_PG_HDRM_CURRENT:
        *num_entries = 8;
        *base_index = port * (*num_entries);
        break;
    case SOC_COUNTER_NON_DMA_QUEUE_PEAK:
    case SOC_COUNTER_NON_DMA_QUEUE_CURRENT:
        *num_entries = 5;
        *base_index = port * (*num_entries);
        break;
    case SOC_COUNTER_NON_DMA_UC_QUEUE_PEAK:
    case SOC_COUNTER_NON_DMA_UC_QUEUE_CURRENT:
        *num_entries = 10;
        *base_index = port * (*num_entries);
        break;
    case SOC_COUNTER_NON_DMA_EXT_QUEUE_PEAK:
    case SOC_COUNTER_NON_DMA_EXT_QUEUE_CURRENT:
        *num_entries = 64;
        *base_index = port * (*num_entries);
        break;
    default:
        return SOC_E_INTERNAL;
    }
    *base_index += non_dma->base_index;

    return SOC_E_NONE;
}
#endif /* BCM_TRIDENT_SUPPORT */

#ifdef BCM_KATANA_SUPPORT
STATIC int
_soc_counter_katana_get_info(int unit, soc_port_t port, soc_reg_t id,
                              int *base_index, int *num_entries)
{
    soc_control_t *soc;
    soc_counter_non_dma_t *non_dma;
    
    soc = SOC_CONTROL(unit);
    non_dma = &soc->counter_non_dma[id - SOC_COUNTER_NON_DMA_START];

    if (!(non_dma->flags & _SOC_COUNTER_NON_DMA_VALID)) {
        return SOC_E_UNAVAIL;
    }

    *base_index = 0;
    *num_entries = port * soc_mem_index_count(unit, CTR_FLEX_COUNT_0m);

    *base_index += non_dma->base_index;

    return SOC_E_NONE;
}
#endif /* BCM_KATANA_SUPPORT */

#ifdef BCM_FIREBOLT_SUPPORT
STATIC int
_soc_counter_fb_get_info(int unit, soc_port_t port, soc_reg_t id,
                         int *base_index, int *num_entries)
{
    soc_control_t *soc;
    soc_counter_non_dma_t *non_dma;
    int i;

    soc = SOC_CONTROL(unit);
    non_dma = &soc->counter_non_dma[id - SOC_COUNTER_NON_DMA_START];

    if (!(non_dma->flags & _SOC_COUNTER_NON_DMA_VALID)) {
        return SOC_E_UNAVAIL;
    }

    if (non_dma->entries_per_port == 1) {
        /* This non-dma counter is per port */
        *base_index = non_dma->base_index + port;
        *num_entries = 1;
    } else {
        /* This non-dma counter is per cosq */
        *base_index = non_dma->base_index;
        for (i = 0; i < SOC_MAX_NUM_PORTS; i++) {
            if (i == port) {
                break;
            }
            *base_index += num_cosq[unit][i];
        }
        *num_entries = num_cosq[unit][port];
    }

    return SOC_E_NONE;
}
#endif /* BCM_FIREBOLT_SUPPORT */

#ifdef BCM_SHADOW_SUPPORT
STATIC int
_soc_counter_shadow_get_info(int unit, soc_port_t port, soc_reg_t id,
                         int *base_index, int *num_entries)
{
    soc_control_t *soc;
    soc_counter_non_dma_t *non_dma;

    soc = SOC_CONTROL(unit);
    non_dma = &soc->counter_non_dma[id - SOC_COUNTER_NON_DMA_START];

    if (!(non_dma->flags & _SOC_COUNTER_NON_DMA_VALID)) {
        return SOC_E_UNAVAIL;
    }

    *base_index = non_dma->base_index;
    *num_entries = 1;

    return SOC_E_NONE;
}
#endif /* BCM_SHADOW_SUPPORT */


STATIC int
_soc_counter_get_info(int unit, soc_port_t port, soc_reg_t ctr_reg,
                      int *base_index, int *num_entries, char **cname)
{
    int rv;

    if (ctr_reg >= NUM_SOC_REG) {
        if (ctr_reg >= SOC_COUNTER_NON_DMA_END) {
            return SOC_E_PARAM;
        }

#ifdef BCM_TRIDENT_SUPPORT
        if (SOC_IS_TD_TT(unit)) {
            rv = _soc_counter_trident_get_info(unit, port, ctr_reg, base_index,
                                               num_entries);
        } else
#endif /* BCM_TRIDENT_SUPPORT */
#ifdef BCM_KATANA_SUPPORT
        if (SOC_IS_KATANA(unit)) {
            rv = _soc_counter_katana_get_info(unit, port, ctr_reg, base_index,
                                               num_entries);
        } else
#endif /* BCM_KATANA_SUPPORT */
#ifdef BCM_FIREBOLT_SUPPORT
        if (SOC_IS_FBX(unit) && !(SOC_IS_SHADOW(unit))) {
            rv = _soc_counter_fb_get_info(unit, port, ctr_reg, base_index,
                                          num_entries);
        } else 
#endif /* BCM_FIREBOLT_SUPPORT */
#ifdef BCM_SHADOW_SUPPORT
        if (SOC_IS_SHADOW(unit)) {
            rv = _soc_counter_shadow_get_info(unit, port, ctr_reg, base_index,
                                          num_entries);
        } else 
#endif /* BCM_SHADOW_SUPPORT */
        {
            rv = SOC_E_UNAVAIL;
        }

        if (rv < 0) {
            return rv;
        }
        if (cname) {
            *cname = SOC_CONTROL(unit)->
                counter_non_dma[ctr_reg - SOC_COUNTER_NON_DMA_START].cname;
        }
    } else {
        if (!SOC_REG_IS_ENABLED(unit, ctr_reg)) {
            return SOC_E_PARAM;
        }
#ifdef BCM_SHADOW_SUPPORT
        if (SOC_IS_SHADOW(unit) && IS_IL_PORT(unit, port)) {
            return SOC_E_PORT;
        }
#endif /* BCM_SHADOW_SUPPORT */

        *base_index = COUNTER_IDX_PORTBASE(unit, port) +
            SOC_REG_CTR_IDX(unit, ctr_reg);
        *num_entries = SOC_REG_NUMELS(unit, ctr_reg);
        if (cname) {
            *cname = SOC_REG_NAME(unit, ctr_reg);
        }
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_counter_idx_get
 * Purpose:
 *      Get the index of a counter given the counter and port
 * Parameters:
 *      unit - The SOC unit number
 *      reg - The register number
 *      port - The port for which index is being calculated
 * Returns:
 *      BCM_E_XXX, no it is not
 * Notes:
 */

int
soc_counter_idx_get(int unit, soc_reg_t reg, int ar_idx, int port)
{
    int base_index, num_entries;
    if (_soc_counter_get_info(unit, port, reg, &base_index, &num_entries,
                              NULL) < 0) {
        return -1;
    }

    if (ar_idx < 0) {
        return base_index;
    } else if (ar_idx < num_entries) {
        return base_index + ar_idx;
    } else {
        return -1;
    }
}

/*
 * Function:
 *      _soc_counter_num_cosq_init
 * Purpose:
 *      Initialize the number of COSQ per port.
 * Parameters:
 *      unit - The SOC unit number
 *      nports - Number of ports the unit has.
 * Returns:
 *      None.
 */
STATIC void
_soc_counter_num_cosq_init(int unit, int nports)
{

#ifdef BCM_TRIUMPH_SUPPORT
#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_IS_TD_TT(unit)) {
        return;
    }
#endif /* BCM_TRIDENT_SUPPORT */
#ifdef BCM_KATANA_SUPPORT
    if (SOC_IS_KATANA(unit)) {
        int port;

        for (port = 0; port < nports; port++) {
            if (IS_CPU_PORT(unit, port)) {
                num_cosq[unit][port] = NUM_CPU_COSQ(unit);
            } else { 
                num_cosq[unit][port] = 8;
            }
        }
    } else
#endif /* BCM_KATANA_SUPPORT */
#ifdef BCM_SHADOW_SUPPORT
    if (SOC_IS_SHADOW(unit)) {
        return;
    }
#endif /* BCM_SHADOW_SUPPORT */
#ifdef BCM_HURRICANE_SUPPORT
    if (SOC_IS_HURRICANE(unit)) {
        int port;
    
        /* all ports 8Q, including G, HG, CMIC and the reserved port 1 */
        for (port = 0; port < nports; port++) {
            num_cosq[unit][port] = 8;
        }

    } else
#endif /* BCM_HURRICANE_SUPPORT */
#ifdef BCM_TRIUMPH2_SUPPORT
    if (SOC_IS_TRIUMPH2(unit) || SOC_IS_APOLLO(unit) || SOC_IS_VALKYRIE2(unit)){
        const int port_24q[] = {26, 27, 28, 29, 30, 31, 34, 38, 39, 42, 43, 46, 50, 51};
        const int port_16q = 54;
        int port, i;

        for (port = 0; port < nports; port++) {
            if (IS_CPU_PORT(unit, port)) {
                num_cosq[unit][port] = NUM_CPU_COSQ(unit);
            } else { 
                num_cosq[unit][port] = 8;
                for (i = 0; i < sizeof(port_24q) / sizeof(port_24q[1]); i++) {
                    if (port == port_24q[i]) {
                        num_cosq[unit][port] = 24;
                        break;
                    }
                }
                if (port == port_16q) {
                    num_cosq[unit][port] = 16;
                }
            }
        }

    } else
#endif /* BCM_TRIUMPH2_SUPPORT */
#ifdef BCM_ENDURO_SUPPORT
    if (SOC_IS_ENDURO(unit)){
        const int port_24q[] = {26, 27, 28, 29};
        int port, i;

        for (port = 0; port < nports; port++) {
            if (IS_CPU_PORT(unit, port)) {
                num_cosq[unit][port] = NUM_CPU_COSQ(unit);
            } else { 
                num_cosq[unit][port] = 8;
                for (i = 0; i < sizeof(port_24q) / sizeof(port_24q[1]); i++) {
                    if (port == port_24q[i]) {
                        num_cosq[unit][port] = 24;
                        break;
                    }
                }
            }
        }

    } else
#endif /* BCM_ENDURO_SUPPORT */
    if (SOC_IS_TR_VL(unit)) { 
        const int port_24q[] = {2, 3, 14, 15, 26, 27, 28, 29, 30, 31, 32, 43};
        int port, i;

        for (port = 0; port < nports; port++) {
            if (IS_CPU_PORT(unit, port)) {
                num_cosq[unit][port] = NUM_CPU_COSQ(unit);
            } else { 
                num_cosq[unit][port] = 8;
                for (i = 0; i < sizeof(port_24q) / sizeof(port_24q[1]); i++) {
                    if (port == port_24q[i]) {
                        num_cosq[unit][port] = 24;
                        break;
                    }
                }
            }
        }
    } else 
#endif /* BCM_TRIUMPH_SUPPORT */
#ifdef BCM_FIREBOLT_SUPPORT
    if (SOC_IS_SC_CQ(unit) || SOC_IS_HB_GW(unit) || SOC_IS_FB_FX_HX(unit)) {
        int port;

        for (port = 0; port < nports; port++) {
            if (IS_CPU_PORT(unit, port)) {
                num_cosq[unit][port] = NUM_CPU_COSQ(unit);
            } else { 
                num_cosq[unit][port] = 8;
            }
        }
    } else
#endif /* BCM_FIREBOLT_SUPPORT */
    {
        return;
    }

    return;
}

#ifdef BCM_TRIDENT_SUPPORT
/*
 * Function:
 *      _soc_counter_trident_non_dma_init
 * Purpose:
 *      Initialize Trident's non-DMA counters.
 * Parameters:
 *      unit - The SOC unit number
 *      nports - Number of ports.
 *      non_dma_start_index - The starting index of non-DMA counter entries.
 *      non_dma_entries - (OUT) The number of non-DMA counter entries.
 * Returns:
 *      SOC_E_NONE
 *      SOC_E_MEMORY
 */
STATIC int
_soc_counter_trident_non_dma_init(int unit, int nports, int non_dma_start_index, 
        int *non_dma_entries)
{
    soc_control_t *soc;
    soc_counter_non_dma_t *non_dma0, *non_dma1, *non_dma2;
    int entry_words, num_entries[2], alloc_size, table_size;
    uint32 *buf;

    soc = SOC_CONTROL(unit);
    *non_dma_entries = 0;

    soc_trident_get_egr_perq_xmt_counters_size(unit, &num_entries[0],
                                               &num_entries[1]);

    /* EGR_PERQ_XMT_COUNTERS size depends on user's portmap config */
    alloc_size = (num_entries[0] + num_entries[1]) *
        soc_mem_entry_words(unit, EGR_PERQ_XMT_COUNTERSm) * sizeof(uint32);

    /* MMU_CTR_UC_DROP_MEM is the largest among MMU_CTR_*_DROP_MEM tables */
    table_size = soc_mem_index_count(unit, MMU_CTR_UC_DROP_MEMm) *
        soc_mem_entry_words(unit, MMU_CTR_UC_DROP_MEMm) * sizeof(uint32);
    if (alloc_size < table_size) {
        alloc_size = table_size;
    }

    buf = soc_cm_salloc(unit, alloc_size, "non_dma_counter");
    if (buf == NULL) {
        return SOC_E_MEMORY;
    }
    sal_memset(buf, 0, alloc_size);

    entry_words = soc_mem_entry_words(unit, EGR_PERQ_XMT_COUNTERSm);
    non_dma0 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_EGR_PERQ_XMT_PKT -
                                     SOC_COUNTER_NON_DMA_START];
    non_dma0->flags = _SOC_COUNTER_NON_DMA_VALID |
        _SOC_COUNTER_NON_DMA_DO_DMA | _SOC_COUNTER_NON_DMA_ALLOC;
    non_dma0->pbmp = PBMP_ALL(unit);
    non_dma0->base_index = non_dma_start_index + *non_dma_entries;
    non_dma0->entries_per_port = 79; /* 10 ucast + 5 mcast + 64 ext ucast */
    non_dma0->num_entries = num_entries[0] + num_entries[1];
    non_dma0->mem = EGR_PERQ_XMT_COUNTERSm;
    non_dma0->reg = INVALIDr;
    non_dma0->field = PACKET_COUNTERf;
    non_dma0->cname = "PERQ_PKT";
    non_dma0->dma_buf[0] = buf;
    non_dma0->dma_buf[1] = &buf[num_entries[0] * entry_words];
    non_dma0->dma_index_max[0] = num_entries[0] - 1;
    non_dma0->dma_index_max[1] = num_entries[1] - 1;
    non_dma0->dma_mem[0] = EGR_PERQ_XMT_COUNTERS_Xm;
    non_dma0->dma_mem[1] = EGR_PERQ_XMT_COUNTERS_Ym;
    *non_dma_entries += non_dma0->num_entries;

    non_dma1 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_EGR_PERQ_XMT_BYTE -
                                    SOC_COUNTER_NON_DMA_START];
    sal_memcpy(non_dma1, non_dma0, sizeof(soc_counter_non_dma_t));
    non_dma1->flags = _SOC_COUNTER_NON_DMA_VALID;
    non_dma1->base_index = non_dma_start_index + *non_dma_entries;
    non_dma1->field = BYTE_COUNTERf;
    non_dma1->cname = "PERQ_BYTE";
    *non_dma_entries += non_dma1->num_entries;

    non_dma0 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_COSQ_DROP_PKT -
                                     SOC_COUNTER_NON_DMA_START];
    non_dma0->flags = _SOC_COUNTER_NON_DMA_VALID |
        _SOC_COUNTER_NON_DMA_DO_DMA;
    non_dma0->pbmp = PBMP_ALL(unit);
    non_dma0->base_index = non_dma_start_index + *non_dma_entries;
    non_dma0->entries_per_port = 48; /* 48 for cpu port */
    non_dma0->num_entries = soc_mem_index_count(unit, MMU_CTR_MC_DROP_MEMm);
    non_dma0->mem = MMU_CTR_MC_DROP_MEMm;
    non_dma0->reg = INVALIDr;
    non_dma0->field = PKT_CNTf;
    non_dma0->cname = "MCQ_DROP_PKT";
    non_dma0->dma_buf[0] = buf;
    non_dma0->dma_index_max[0] = non_dma0->num_entries - 1;
    non_dma0->dma_mem[0] = non_dma0->mem;
    *non_dma_entries += non_dma0->num_entries;

    non_dma1 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_COSQ_DROP_BYTE -
                                     SOC_COUNTER_NON_DMA_START];
    sal_memcpy(non_dma1, non_dma0, sizeof(soc_counter_non_dma_t));
    non_dma1->flags = _SOC_COUNTER_NON_DMA_VALID;
    non_dma1->base_index = non_dma_start_index + *non_dma_entries;
    non_dma1->field = BYTE_COUNTf;
    non_dma1->cname = "MCQ_DROP_BYTE";
    *non_dma_entries += non_dma1->num_entries;

    non_dma0 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_COSQ_DROP_PKT_UC -
                                     SOC_COUNTER_NON_DMA_START];
    non_dma0->flags = _SOC_COUNTER_NON_DMA_VALID |
        _SOC_COUNTER_NON_DMA_DO_DMA;
    non_dma0->pbmp = PBMP_PORT_ALL(unit);
    non_dma0->base_index = non_dma_start_index + *non_dma_entries;
    non_dma0->entries_per_port = 74; /*  10 ucast + 64 ext ucast */
    non_dma0->num_entries = soc_mem_index_count(unit, MMU_CTR_UC_DROP_MEMm);
    non_dma0->mem = MMU_CTR_UC_DROP_MEMm;
    non_dma0->reg = INVALIDr;
    non_dma0->field = PKT_CNTf;
    non_dma0->cname = "UCQ_DROP_PKT";
    non_dma0->dma_buf[0] = buf;
    non_dma0->dma_index_max[0] = non_dma0->num_entries - 1;
    non_dma0->dma_mem[0] = non_dma0->mem;
    *non_dma_entries += non_dma0->num_entries;

    non_dma1 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_COSQ_DROP_BYTE_UC -
                                     SOC_COUNTER_NON_DMA_START];
    sal_memcpy(non_dma1, non_dma0, sizeof(soc_counter_non_dma_t));
    non_dma1->flags = _SOC_COUNTER_NON_DMA_VALID;
    non_dma1->base_index = non_dma_start_index + *non_dma_entries;
    non_dma1->field = BYTE_COUNTf;
    non_dma1->cname = "UCQ_DROP_BYTE";
    *non_dma_entries += non_dma1->num_entries;

    non_dma0 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_PORT_DROP_PKT_ING -
                                     SOC_COUNTER_NON_DMA_START];
    non_dma0->flags = _SOC_COUNTER_NON_DMA_VALID;
    non_dma0->pbmp = PBMP_ALL(unit);
    non_dma0->base_index = non_dma_start_index + *non_dma_entries;
    non_dma0->entries_per_port = 1;
    non_dma0->num_entries = nports;
    non_dma0->mem = INVALIDm;
    non_dma0->reg = DROP_PKT_CNT_INGr;
    non_dma0->field = COUNTf;
    non_dma0->cname = "DROP_PKT_ING";
    *non_dma_entries += non_dma0->num_entries;

    non_dma1 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_PORT_DROP_BYTE_ING -
                                     SOC_COUNTER_NON_DMA_START];
    sal_memcpy(non_dma1, non_dma0, sizeof(soc_counter_non_dma_t));
    non_dma1->base_index = non_dma_start_index + *non_dma_entries;
    non_dma1->reg = DROP_BYTE_CNT_ING_64r;
    non_dma1->cname = "DROP_BYTE_ING";
    *non_dma_entries += non_dma1->num_entries;

    non_dma0 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_PORT_DROP_PKT_YELLOW -
                                     SOC_COUNTER_NON_DMA_START];
    non_dma0->flags = _SOC_COUNTER_NON_DMA_VALID |
        _SOC_COUNTER_NON_DMA_DO_DMA;
    non_dma0->pbmp = PBMP_PORT_ALL(unit);
    non_dma0->base_index = non_dma_start_index + *non_dma_entries;
    non_dma0->entries_per_port = 1;
    non_dma0->num_entries = 66; /* one per port */
    non_dma0->mem = MMU_CTR_COLOR_DROP_MEMm;
    non_dma0->reg = INVALIDr;
    non_dma0->field = PKT_CNTf;
    non_dma0->cname = "DROP_PKT_YEL";
    non_dma0->dma_buf[0] = buf;
    non_dma0->dma_index_min[0] = 66 * 4;
    non_dma0->dma_index_max[0] = 66 * 4 + 65;
    non_dma0->dma_mem[0] = non_dma0->mem;
    *non_dma_entries += non_dma0->num_entries;

    non_dma1 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_PORT_DROP_PKT_RED -
                                     SOC_COUNTER_NON_DMA_START];
    sal_memcpy(non_dma1, non_dma0, sizeof(soc_counter_non_dma_t));
    non_dma1->base_index = non_dma_start_index + *non_dma_entries;
    non_dma1->cname = "DROP_PKT_RED";
    non_dma1->dma_index_min[0] = 66 * 3;
    non_dma1->dma_index_max[0] = 66 * 3 + 65;
    *non_dma_entries += non_dma1->num_entries;

    non_dma1 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_PORT_WRED_PKT_GREEN -
                                     SOC_COUNTER_NON_DMA_START];
    sal_memcpy(non_dma1, non_dma0, sizeof(soc_counter_non_dma_t));
    non_dma1->base_index = non_dma_start_index + *non_dma_entries;
    non_dma1->cname = "WRED_PKT_GRE";
    non_dma1->dma_index_min[0] = 66 * 2;
    non_dma1->dma_index_max[0] = 66 * 2 + 65;
    *non_dma_entries += non_dma1->num_entries;

    non_dma1 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_PORT_WRED_PKT_YELLOW -
                                     SOC_COUNTER_NON_DMA_START];
    sal_memcpy(non_dma1, non_dma0, sizeof(soc_counter_non_dma_t));
    non_dma1->base_index = non_dma_start_index + *non_dma_entries;
    non_dma1->cname = "WRED_PKT_YEL";
    non_dma1->dma_index_min[0] = 66 * 1;
    non_dma1->dma_index_max[0] = 66 * 1 + 65;
    *non_dma_entries += non_dma1->num_entries;

    non_dma1 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_PORT_WRED_PKT_RED -
                                     SOC_COUNTER_NON_DMA_START];
    sal_memcpy(non_dma1, non_dma0, sizeof(soc_counter_non_dma_t));
    non_dma1->base_index = non_dma_start_index + *non_dma_entries;
    non_dma1->cname = "WRED_PKT_RED";
    non_dma1->dma_index_min[0] = 0;
    non_dma1->dma_index_max[0] = 65;
    *non_dma_entries += non_dma1->num_entries;

    non_dma0 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_POOL_PEAK -
                                     SOC_COUNTER_NON_DMA_START];
    non_dma0->flags = _SOC_COUNTER_NON_DMA_VALID |
        _SOC_COUNTER_NON_DMA_PEAK;
    SOC_PBMP_CLEAR(non_dma0->pbmp);
    non_dma0->base_index = non_dma_start_index + *non_dma_entries;
    non_dma0->entries_per_port = 0;
    non_dma0->num_entries = 4;
    non_dma0->mem = INVALIDm;
    non_dma0->reg = TOTAL_BUFFER_COUNT_CELL_SPr;
    non_dma0->field = TOTAL_BUFFER_COUNTf;
    non_dma0->cname = "POOL_PEAK";
    *non_dma_entries += non_dma0->num_entries;

    non_dma2 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_POOL_CURRENT -
                                     SOC_COUNTER_NON_DMA_START];
    sal_memcpy(non_dma2, non_dma0, sizeof(soc_counter_non_dma_t));
    non_dma2->flags = _SOC_COUNTER_NON_DMA_VALID |
        _SOC_COUNTER_NON_DMA_CURRENT;
    non_dma2->cname = "POOL_CUR";

    non_dma0 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_PG_MIN_PEAK -
                                     SOC_COUNTER_NON_DMA_START];
    non_dma0->flags = _SOC_COUNTER_NON_DMA_VALID |
        _SOC_COUNTER_NON_DMA_PEAK;
    non_dma0->pbmp = PBMP_PORT_ALL(unit);
    non_dma0->base_index = non_dma_start_index + *non_dma_entries;
    non_dma0->entries_per_port = 8;
    non_dma0->num_entries = nports * non_dma0->entries_per_port;
    non_dma0->mem = INVALIDm;
    non_dma0->reg = PG_MIN_COUNT_CELLr;
    non_dma0->field = PG_MIN_COUNTf;
    non_dma0->cname = "PG_MIN_PEAK";
    *non_dma_entries += non_dma0->num_entries;

    non_dma2 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_PG_MIN_CURRENT -
                                     SOC_COUNTER_NON_DMA_START];
    sal_memcpy(non_dma2, non_dma0, sizeof(soc_counter_non_dma_t));
    non_dma2->flags = _SOC_COUNTER_NON_DMA_VALID |
        _SOC_COUNTER_NON_DMA_CURRENT;
    non_dma2->cname = "PG_MIN_CUR";

    non_dma1 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_PG_SHARED_PEAK -
                                     SOC_COUNTER_NON_DMA_START];
    sal_memcpy(non_dma1, non_dma0, sizeof(soc_counter_non_dma_t));
    non_dma1->base_index = non_dma_start_index + *non_dma_entries;
    non_dma1->reg = PG_SHARED_COUNT_CELLr;
    non_dma1->field = PG_SHARED_COUNTf;
    non_dma1->cname = "PG_SHARED_PEAK";
    *non_dma_entries += non_dma1->num_entries;

    non_dma2 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_PG_SHARED_CURRENT -
                                     SOC_COUNTER_NON_DMA_START];
    sal_memcpy(non_dma2, non_dma1, sizeof(soc_counter_non_dma_t));
    non_dma2->flags = _SOC_COUNTER_NON_DMA_VALID |
        _SOC_COUNTER_NON_DMA_CURRENT;
    non_dma2->cname = "PG_SHARED_CUR";

    non_dma1 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_PG_HDRM_PEAK -
                                     SOC_COUNTER_NON_DMA_START];
    sal_memcpy(non_dma1, non_dma0, sizeof(soc_counter_non_dma_t));
    non_dma1->base_index = non_dma_start_index + *non_dma_entries;
    non_dma1->reg = PG_HDRM_COUNT_CELLr;
    non_dma1->field = PG_HDRM_COUNTf;
    non_dma1->cname = "PG_HDRM_PEAK";
    *non_dma_entries += non_dma1->num_entries;

    non_dma2 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_PG_HDRM_CURRENT -
                                     SOC_COUNTER_NON_DMA_START];
    sal_memcpy(non_dma2, non_dma1, sizeof(soc_counter_non_dma_t));
    non_dma2->flags = _SOC_COUNTER_NON_DMA_VALID |
        _SOC_COUNTER_NON_DMA_CURRENT;
    non_dma2->cname = "PG_HDRM_CUR";

    non_dma1 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_QUEUE_PEAK -
                                     SOC_COUNTER_NON_DMA_START];
    sal_memcpy(non_dma1, non_dma0, sizeof(soc_counter_non_dma_t));
    non_dma1->pbmp = PBMP_ALL(unit);
    non_dma1->base_index = non_dma_start_index + *non_dma_entries;
    non_dma1->entries_per_port = 5;
    non_dma1->num_entries = nports * non_dma0->entries_per_port;
    non_dma1->reg = OP_QUEUE_TOTAL_COUNT_CELLr;
    non_dma1->field = Q_TOTAL_COUNT_CELLf;
    non_dma1->cname = "MC_QUEUE_PEAK";
    *non_dma_entries += non_dma1->num_entries;

    non_dma2 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_QUEUE_CURRENT -
                                     SOC_COUNTER_NON_DMA_START];
    sal_memcpy(non_dma2, non_dma1, sizeof(soc_counter_non_dma_t));
    non_dma2->flags = _SOC_COUNTER_NON_DMA_VALID |
        _SOC_COUNTER_NON_DMA_CURRENT;
    non_dma2->cname = "MC_QUEUE_CUR";

    non_dma1 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_UC_QUEUE_PEAK -
                                     SOC_COUNTER_NON_DMA_START];
    sal_memcpy(non_dma1, non_dma0, sizeof(soc_counter_non_dma_t));
    non_dma1->pbmp = PBMP_PORT_ALL(unit);
    non_dma1->base_index = non_dma_start_index + *non_dma_entries;
    non_dma1->entries_per_port = 10;
    non_dma1->num_entries = nports * non_dma1->entries_per_port;
    non_dma1->reg = OP_UC_QUEUE_TOTAL_COUNT_CELLr;
    non_dma1->field =  Q_TOTAL_COUNT_CELLf;
    non_dma1->cname = "UC_QUEUE_PEAK";
    *non_dma_entries += non_dma1->num_entries;

    non_dma2 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_UC_QUEUE_CURRENT -
                                     SOC_COUNTER_NON_DMA_START];
    sal_memcpy(non_dma2, non_dma1, sizeof(soc_counter_non_dma_t));
    non_dma2->flags = _SOC_COUNTER_NON_DMA_VALID |
        _SOC_COUNTER_NON_DMA_CURRENT;
    non_dma2->cname = "UC_QUEUE_CUR";

    non_dma1 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_EXT_QUEUE_PEAK -
                                     SOC_COUNTER_NON_DMA_START];
    sal_memcpy(non_dma1, non_dma0, sizeof(soc_counter_non_dma_t));
    non_dma1->pbmp = PBMP_EQ(unit);
    non_dma1->base_index = non_dma_start_index + *non_dma_entries;
    non_dma1->entries_per_port = 64;
    non_dma1->num_entries = nports * non_dma1->entries_per_port;
    non_dma1->reg = OP_EX_QUEUE_TOTAL_COUNT_CELLr;
    non_dma1->cname = "EXT_QUEUE_PEAK";
    non_dma1->field =  Q_TOTAL_COUNT_CELLf;
    *non_dma_entries += non_dma1->num_entries;

    non_dma2 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_EXT_QUEUE_CURRENT -
                                     SOC_COUNTER_NON_DMA_START];
    sal_memcpy(non_dma2, non_dma1, sizeof(soc_counter_non_dma_t));
    non_dma2->flags = _SOC_COUNTER_NON_DMA_VALID |
        _SOC_COUNTER_NON_DMA_CURRENT;
    non_dma2->cname = "EXT_QUEUE_CUR";

    return SOC_E_NONE;
}
#endif /* BCM_TRIDENT_SUPPORT */

#ifdef BCM_HURRICANE_SUPPORT
/*
 * Function:
 *      _soc_counter_hu_non_dma_init
 * Purpose:
 *      Initialize Hurricane's non-DMA counters.
 * Parameters:
 *      unit - The SOC unit number
 *      nports - Number of ports.
 *      non_dma_start_index - The starting index of non-DMA counter entries.
 *      non_dma_entries - (OUT) The number of non-DMA counter entries.
 * Returns:
 *      SOC_E_NONE
 *      SOC_E_MEMORY
 */
STATIC int
_soc_counter_hu_non_dma_init(int unit, int nports, int non_dma_start_index, 
        int *non_dma_entries)
{
    soc_control_t *soc;
    soc_counter_non_dma_t *non_dma0, *non_dma1;
    int max_cosq_per_port, total_num_cosq, port;
    int num_entries, alloc_size;
    uint32 *buf;

    soc = SOC_CONTROL(unit);
    *non_dma_entries = 0;

    max_cosq_per_port = 0;
    total_num_cosq = 0;
    for (port = 0; port < nports; port++) {
        if (num_cosq[unit][port] > max_cosq_per_port) {
            max_cosq_per_port = num_cosq[unit][port];
        }
        total_num_cosq += num_cosq[unit][port];
    }

    num_entries = soc_mem_index_count(unit, EGR_PERQ_XMT_COUNTERSm);
    alloc_size = num_entries *
        soc_mem_entry_words(unit, EGR_PERQ_XMT_COUNTERSm) * sizeof(uint32);

    buf = soc_cm_salloc(unit, alloc_size, "non_dma_counter");
    if (buf == NULL) {
        return SOC_E_MEMORY;
    }
    sal_memset(buf, 0, alloc_size);

    non_dma0 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_EGR_PERQ_XMT_PKT -
                                     SOC_COUNTER_NON_DMA_START];
    non_dma0->flags = _SOC_COUNTER_NON_DMA_VALID |
        _SOC_COUNTER_NON_DMA_DO_DMA | _SOC_COUNTER_NON_DMA_ALLOC;
    non_dma0->pbmp = PBMP_ALL(unit);
    non_dma0->base_index = non_dma_start_index + *non_dma_entries;
    non_dma0->entries_per_port = max_cosq_per_port;
    non_dma0->num_entries = num_entries;
    non_dma0->mem = EGR_PERQ_XMT_COUNTERSm;
    non_dma0->reg = INVALIDr;
    non_dma0->field = PACKET_COUNTERf;
    non_dma0->cname = "PERQ_PKT";
    non_dma0->dma_buf[0] = buf;
    non_dma0->dma_index_max[0] = num_entries - 1;
    non_dma0->dma_mem[0] = non_dma0->mem;
    *non_dma_entries += non_dma0->num_entries;

    non_dma1 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_EGR_PERQ_XMT_BYTE -
                                     SOC_COUNTER_NON_DMA_START];
    sal_memcpy(non_dma1, non_dma0, sizeof(soc_counter_non_dma_t));
    non_dma1->flags = _SOC_COUNTER_NON_DMA_VALID;
    non_dma1->base_index = non_dma_start_index + *non_dma_entries;
    non_dma1->field = BYTE_COUNTERf;
    non_dma1->cname = "PERQ_BYTE";
    *non_dma_entries += non_dma1->num_entries;

    non_dma0 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_PORT_DROP_PKT_YELLOW -
                                     SOC_COUNTER_NON_DMA_START];
    non_dma0->flags = _SOC_COUNTER_NON_DMA_VALID;
    non_dma0->pbmp = PBMP_ALL(unit);
    non_dma0->base_index = non_dma_start_index + *non_dma_entries;
    non_dma0->entries_per_port = 1;
    non_dma0->num_entries = nports;
    non_dma0->mem = INVALIDm;
    non_dma0->reg = CNGDROPCOUNT1r;
    non_dma0->field = DROPPKTCOUNTf;
    non_dma0->cname = "DROP_PKT_YEL";
    *non_dma_entries += non_dma0->num_entries;

    non_dma0 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_PORT_DROP_PKT_RED -
                                     SOC_COUNTER_NON_DMA_START];
    non_dma0->flags = _SOC_COUNTER_NON_DMA_VALID;
    non_dma0->pbmp = PBMP_ALL(unit);
    non_dma0->base_index = non_dma_start_index + *non_dma_entries;
    non_dma0->entries_per_port = 1;
    non_dma0->num_entries = nports;
    non_dma0->mem = INVALIDm;
    non_dma0->reg = CNGDROPCOUNT0r;
    non_dma0->field = DROPPKTCOUNTf;
    non_dma0->cname = "DROP_PKT_RED";
    *non_dma_entries += non_dma0->num_entries;

    return SOC_E_NONE;
}
#endif /* BCM_HURRICANE_SUPPORT */

#ifdef BCM_KATANA_SUPPORT
/*
 * Function:
 *      _soc_counter_katana_non_dma_init
 * Purpose:
 *      Initialize Katana's non-DMA counters.
 * Parameters:
 *      unit - The SOC unit number
 *      nports - Number of ports.
 *      non_dma_start_index - The starting index of non-DMA counter entries.
 *      non_dma_entries - (OUT) The number of non-DMA counter entries.
 * Returns:
 *      SOC_E_NONE
 *      SOC_E_MEMORY
 */
STATIC int
_soc_counter_katana_non_dma_init(int unit, int nports, int non_dma_start_index, 
        int *non_dma_entries)
{
#if 0
    soc_control_t *soc;
    soc_counter_non_dma_t *non_dma0, *non_dma1;
    int table_size;
    uint32 *buf;

    soc = SOC_CONTROL(unit);
    *non_dma_entries = 0;

    table_size = soc_mem_index_count(unit, CTR_FLEX_COUNT_0m) *
        soc_mem_entry_words(unit, CTR_FLEX_COUNT_0m) * sizeof(uint32);

    buf = soc_cm_salloc(unit, table_size, "non_dma_counter");
    if (buf == NULL) {
        return SOC_E_MEMORY;
    }
    sal_memset(buf, 0, table_size);

    non_dma0 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_EGR_PERQ_XMT_PKT -
                                     SOC_COUNTER_NON_DMA_START];
    non_dma0->flags = _SOC_COUNTER_NON_DMA_VALID |
        _SOC_COUNTER_NON_DMA_DO_DMA | _SOC_COUNTER_NON_DMA_ALLOC;
    non_dma0->pbmp = PBMP_ALL(unit);
    non_dma0->base_index = non_dma_start_index + *non_dma_entries;
    non_dma0->entries_per_port = 1;
    non_dma0->num_entries = soc_mem_index_count(unit, CTR_FLEX_COUNT_0m);
    non_dma0->mem = CTR_FLEX_COUNT_0m;
    non_dma0->reg = INVALIDr;
    non_dma0->field = COUNTf;
    non_dma0->cname = "PERQ_PKT";
    non_dma0->dma_buf[0] = buf;
    non_dma0->dma_index_max[0] = 1023;
    *non_dma_entries += non_dma0->num_entries;

    non_dma1 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_EGR_PERQ_XMT_BYTE -
                                    SOC_COUNTER_NON_DMA_START];
    sal_memcpy(non_dma1, non_dma0, sizeof(soc_counter_non_dma_t));
    non_dma1->flags = _SOC_COUNTER_NON_DMA_VALID;
    non_dma1->base_index = non_dma_start_index + *non_dma_entries;
    non_dma1->field = BYTE_COUNTf;
    non_dma1->cname = "PERQ_BYTE";
    *non_dma_entries += non_dma1->num_entries;

    non_dma0 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_COSQ_DROP_PKT_UC -
                                     SOC_COUNTER_NON_DMA_START];
    non_dma0->flags = _SOC_COUNTER_NON_DMA_VALID |
        _SOC_COUNTER_NON_DMA_DO_DMA;
    non_dma0->pbmp = PBMP_ALL(unit);
    non_dma0->base_index = non_dma_start_index + *non_dma_entries;
    non_dma0->entries_per_port = 1;
    non_dma0->num_entries = soc_mem_index_count(unit, CTR_FLEX_COUNT_4m);
    non_dma0->mem = CTR_FLEX_COUNT_4m;
    non_dma0->reg = INVALIDr;
    non_dma0->field = COUNTf;
    non_dma0->cname = "PERQ_DROP_PKT";
    non_dma0->dma_buf[0] = buf;
    non_dma0->dma_index_max[0] = non_dma0->num_entries - 1;
    non_dma0->dma_mem[0] = non_dma0->mem;
    *non_dma_entries += non_dma0->num_entries;

    non_dma1 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_COSQ_DROP_BYTE_UC -
                                    SOC_COUNTER_NON_DMA_START];
    sal_memcpy(non_dma1, non_dma0, sizeof(soc_counter_non_dma_t));
    non_dma1->flags = _SOC_COUNTER_NON_DMA_VALID;
    non_dma1->base_index = non_dma_start_index + *non_dma_entries;
    non_dma1->field = BYTE_COUNTf;
    non_dma1->cname = "PERQ_DROP_BYTE";
    *non_dma_entries += non_dma1->num_entries;

    non_dma1 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_COSQ_WRED_PKT_RED -
                                     SOC_COUNTER_NON_DMA_START];
    sal_memcpy(non_dma1, non_dma0, sizeof(soc_counter_non_dma_t));
    non_dma1->base_index = non_dma_start_index + *non_dma_entries;
    non_dma0->num_entries = soc_mem_index_count(unit, CTR_FLEX_COUNT_8m);
    non_dma1->mem = CTR_FLEX_COUNT_8m;
    non_dma1->cname = "COSQ_WRED_PKT_RED";
    *non_dma_entries += non_dma1->num_entries;
#endif
    return SOC_E_NONE;
}
#endif /* BCM_KATANA_SUPPORT */

#ifdef BCM_TRIUMPH_SUPPORT
/*
 * Function:
 *      _soc_counter_tr_non_dma_init
 * Purpose:
 *      Initialize Triumph's non-DMA counters.
 * Parameters:
 *      unit - The SOC unit number
 *      nports - Number of ports.
 *      non_dma_start_index - The starting index of non-DMA counter entries.
 *      non_dma_entries - (OUT) The number of non-DMA counter entries.
 * Returns:
 *      SOC_E_NONE
 *      SOC_E_MEMORY
 */
STATIC int
_soc_counter_tr_non_dma_init(int unit, int nports, int non_dma_start_index, 
        int *non_dma_entries)
{
    soc_control_t *soc;
    soc_counter_non_dma_t *non_dma0, *non_dma1;
    int max_cosq_per_port, total_num_cosq, port;
    int num_entries, alloc_size;
    uint32 *buf;

    soc = SOC_CONTROL(unit);
    *non_dma_entries = 0;

    max_cosq_per_port = 0;
    total_num_cosq = 0;
    for (port = 0; port < nports; port++) {
        if (num_cosq[unit][port] > max_cosq_per_port) {
            max_cosq_per_port = num_cosq[unit][port];
        }
        total_num_cosq += num_cosq[unit][port];
    }

    num_entries = soc_mem_index_count(unit, EGR_PERQ_XMT_COUNTERSm);
    alloc_size = num_entries *
        soc_mem_entry_words(unit, EGR_PERQ_XMT_COUNTERSm) * sizeof(uint32);

    buf = soc_cm_salloc(unit, alloc_size, "non_dma_counter");
    if (buf == NULL) {
        return SOC_E_MEMORY;
    }
    sal_memset(buf, 0, alloc_size);

    non_dma0 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_EGR_PERQ_XMT_PKT -
                                     SOC_COUNTER_NON_DMA_START];
    non_dma0->flags = _SOC_COUNTER_NON_DMA_VALID |
        _SOC_COUNTER_NON_DMA_DO_DMA | _SOC_COUNTER_NON_DMA_ALLOC;
    non_dma0->pbmp = PBMP_ALL(unit);
    non_dma0->base_index = non_dma_start_index + *non_dma_entries;
    non_dma0->entries_per_port = max_cosq_per_port;
    non_dma0->num_entries = num_entries;
    non_dma0->mem = EGR_PERQ_XMT_COUNTERSm;
    non_dma0->reg = INVALIDr;
    non_dma0->field = PACKET_COUNTERf;
    non_dma0->cname = "PERQ_PKT";
    non_dma0->dma_buf[0] = buf;
    non_dma0->dma_index_max[0] = num_entries - 1;
    non_dma0->dma_mem[0] = non_dma0->mem;
    *non_dma_entries += non_dma0->num_entries;

    non_dma1 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_EGR_PERQ_XMT_BYTE -
                                     SOC_COUNTER_NON_DMA_START];
    sal_memcpy(non_dma1, non_dma0, sizeof(soc_counter_non_dma_t));
    non_dma1->flags = _SOC_COUNTER_NON_DMA_VALID;
    non_dma1->base_index = non_dma_start_index + *non_dma_entries;
    non_dma1->field = BYTE_COUNTERf;
    non_dma1->cname = "PERQ_BYTE";
    *non_dma_entries += non_dma1->num_entries;

    non_dma0 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_COSQ_DROP_PKT -
                                     SOC_COUNTER_NON_DMA_START];
    non_dma0->flags = _SOC_COUNTER_NON_DMA_VALID |
        _SOC_COUNTER_NON_DMA_PERQ_REG;
    non_dma0->pbmp = PBMP_ALL(unit);
    non_dma0->base_index = non_dma_start_index + *non_dma_entries;
    non_dma0->entries_per_port = max_cosq_per_port;
    non_dma0->num_entries = total_num_cosq;
    non_dma0->mem = INVALIDm;
    non_dma0->reg = DROP_PKT_CNTr;
    non_dma0->field = COUNTf;
    non_dma0->cname = "PERQ_DROP_PKT";
    *non_dma_entries += non_dma0->num_entries;

    non_dma1 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_COSQ_DROP_BYTE -
                                     SOC_COUNTER_NON_DMA_START];
    sal_memcpy(non_dma1, non_dma0, sizeof(soc_counter_non_dma_t));
    non_dma1->base_index = non_dma_start_index + *non_dma_entries;
    non_dma1->reg = DROP_BYTE_CNTr;
    non_dma1->cname = "PERQ_DROP_BYTE";
    *non_dma_entries += non_dma1->num_entries;

    non_dma0 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_PORT_DROP_PKT_ING -
                                     SOC_COUNTER_NON_DMA_START];
    non_dma0->flags = _SOC_COUNTER_NON_DMA_VALID;
    non_dma0->pbmp = PBMP_ALL(unit);
    non_dma0->base_index = non_dma_start_index + *non_dma_entries;
    non_dma0->entries_per_port = 1;
    non_dma0->num_entries = nports;
    non_dma0->mem = INVALIDm;
    non_dma0->reg = DROP_PKT_CNT_INGr;
    non_dma0->field = COUNTf;
    non_dma0->cname = "DROP_PKT_ING";
    *non_dma_entries += non_dma0->num_entries;

    non_dma1 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_PORT_DROP_BYTE_ING -
                                     SOC_COUNTER_NON_DMA_START];
    sal_memcpy(non_dma1, non_dma0, sizeof(soc_counter_non_dma_t));
    non_dma1->base_index = non_dma_start_index + *non_dma_entries;
    non_dma1->reg = DROP_BYTE_CNT_INGr;
    non_dma1->cname = "DROP_BYTE_ING";
    *non_dma_entries += non_dma1->num_entries;

    non_dma1 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_PORT_DROP_PKT_YELLOW -
                                     SOC_COUNTER_NON_DMA_START];
    sal_memcpy(non_dma1, non_dma0, sizeof(soc_counter_non_dma_t));
    non_dma1->base_index = non_dma_start_index + *non_dma_entries;
    non_dma1->reg = DROP_PKT_CNT_YELr;
    non_dma1->cname = "DROP_PKT_YEL";
    *non_dma_entries += non_dma1->num_entries;

    non_dma1 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_PORT_DROP_PKT_RED -
                                     SOC_COUNTER_NON_DMA_START];
    sal_memcpy(non_dma1, non_dma0, sizeof(soc_counter_non_dma_t));
    non_dma1->base_index = non_dma_start_index + *non_dma_entries;
    non_dma1->reg = DROP_PKT_CNT_REDr;
    non_dma1->cname = "DROP_PKT_RED";
    *non_dma_entries += non_dma1->num_entries;

    return SOC_E_NONE;
}
#endif /* BCM_TRIUMPH_SUPPORT */

#ifdef BCM_SCORPION_SUPPORT
/*
 * Function:
 *      _soc_counter_sc_non_dma_init
 * Purpose:
 *      Initialize Scorpion's non-DMA counters.
 * Parameters:
 *      unit - The SOC unit number
 *      nports - Number of ports
 *      non_dma_start_index - The starting index of non-DMA counter entries.
 *      non_dma_entries - (OUT) The number of non-DMA counter entries.
 * Returns:
 *      SOC_E_NONE
 */
STATIC int
_soc_counter_sc_non_dma_init(int unit, int nports, int non_dma_start_index, 
        int *non_dma_entries)
{
    soc_control_t *soc;
    soc_counter_non_dma_t *non_dma0, *non_dma1;
    int max_cosq_per_port, total_num_cosq, port;

    soc = SOC_CONTROL(unit);
    *non_dma_entries = 0;

    max_cosq_per_port = 0;
    total_num_cosq = 0;
    for (port = 0; port < nports; port++) {
        if (num_cosq[unit][port] > max_cosq_per_port) {
            max_cosq_per_port = num_cosq[unit][port];
        }
        total_num_cosq += num_cosq[unit][port];
    }

    non_dma0 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_COSQ_DROP_PKT -
                                     SOC_COUNTER_NON_DMA_START];
    non_dma0->flags = _SOC_COUNTER_NON_DMA_VALID |
        _SOC_COUNTER_NON_DMA_PERQ_REG;
    non_dma0->pbmp = PBMP_ALL(unit);
    non_dma0->base_index = non_dma_start_index + *non_dma_entries;
    non_dma0->entries_per_port = max_cosq_per_port;
    non_dma0->num_entries = total_num_cosq;
    non_dma0->mem = INVALIDm;
    non_dma0->reg = HOLDROP_PKT_CNTr;
    non_dma0->field = COUNTf;
    non_dma0->cname = "PERQ_DROP_PKT";
    *non_dma_entries += non_dma0->num_entries;

    non_dma0 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_PORT_DROP_PKT_IBP -
                                     SOC_COUNTER_NON_DMA_START];
    non_dma0->flags = _SOC_COUNTER_NON_DMA_VALID;
    non_dma0->pbmp = PBMP_ALL(unit);
    non_dma0->base_index = non_dma_start_index + *non_dma_entries;
    non_dma0->entries_per_port = 1;
    non_dma0->num_entries = nports;
    non_dma0->mem = INVALIDm;
    non_dma0->reg = IBP_DROP_PKT_CNTr;
    non_dma0->field = COUNTf;
    non_dma0->cname = "DROP_PKT_IBP";
    *non_dma_entries += non_dma0->num_entries;

    non_dma1 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_PORT_DROP_PKT_CFAP -
                                     SOC_COUNTER_NON_DMA_START];
    sal_memcpy(non_dma1, non_dma0, sizeof(soc_counter_non_dma_t));
    non_dma1->base_index = non_dma_start_index + *non_dma_entries;
    non_dma1->reg = CFAP_DROP_PKT_CNTr;
    non_dma1->cname = "DROP_PKT_CFAP";
    *non_dma_entries += non_dma1->num_entries;

    non_dma1 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_PORT_DROP_PKT_YELLOW -
                                     SOC_COUNTER_NON_DMA_START];
    sal_memcpy(non_dma1, non_dma0, sizeof(soc_counter_non_dma_t));
    non_dma1->base_index = non_dma_start_index + *non_dma_entries;
    non_dma1->reg = YELLOW_CNG_DROP_CNTr;
    non_dma1->cname = "DROP_PKT_YEL";
    *non_dma_entries += non_dma1->num_entries;

    non_dma1 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_PORT_DROP_PKT_RED -
                                     SOC_COUNTER_NON_DMA_START];
    sal_memcpy(non_dma1, non_dma0, sizeof(soc_counter_non_dma_t));
    non_dma1->base_index = non_dma_start_index + *non_dma_entries;
    non_dma1->reg = RED_CNG_DROP_CNTr;
    non_dma1->cname = "DROP_PKT_RED";
    *non_dma_entries += non_dma1->num_entries;

    return SOC_E_NONE;
}
#endif /* BCM_SCORPION_SUPPORT */

#ifdef BCM_SHADOW_SUPPORT
/*
 * Function:
 *      _soc_counter_shadow_non_dma_init
 * Purpose:
 *      Initialize Shadow's non-DMA counters.
 * Parameters:
 *      unit - The SOC unit number
 *      nports - Number of ports.
 *      non_dma_start_index - The starting index of non-DMA counter entries.
 *      non_dma_entries - (OUT) The number of non-DMA counter entries.
 * Returns:
 *      SOC_E_NONE
 *      SOC_E_MEMORY
 */
STATIC int
_soc_counter_shadow_non_dma_init(int unit, int nports, int non_dma_start_index, 
        int *non_dma_entries)
{
    soc_control_t *soc;
    soc_counter_non_dma_t *non_dma0,*non_dma1;
    int num_entries,alloc_size;
    uint32 *buf;

    soc = SOC_CONTROL(unit);
    *non_dma_entries = 0;
    
    /* Reusing non_dma id for interlaken counters */

    /* IL_STAT_MEM_0 Entries */
    num_entries = soc_mem_index_count(unit, IL_STAT_MEM_0m);
    alloc_size = num_entries *
        soc_mem_entry_words(unit, IL_STAT_MEM_0m) * sizeof(uint32);

    buf = soc_cm_salloc(unit, alloc_size, "non_dma_counter");
    if (buf == NULL) {
        return SOC_E_MEMORY;
    }
    sal_memset(buf, 0, alloc_size);


    /* IL_STAT_MEM_0: RX_STAT_PKT_COUNTf */
    /* Overload SOC_COUNTER_NON_DMA_EGR_PERQ_XMT_PKT */ 
    non_dma0 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_EGR_PERQ_XMT_PKT -
                                     SOC_COUNTER_NON_DMA_START];
    non_dma0->flags = _SOC_COUNTER_NON_DMA_VALID |
        _SOC_COUNTER_NON_DMA_DO_DMA | _SOC_COUNTER_NON_DMA_ALLOC;
    non_dma0->pbmp = PBMP_IL_ALL(unit);
    non_dma0->base_index = non_dma_start_index + *non_dma_entries;
    non_dma0->entries_per_port = 1;
    non_dma0->num_entries = 1;
    non_dma0->mem = IL_STAT_MEM_0m;
    non_dma0->reg = INVALIDr;
    non_dma0->field = RX_STAT_PKT_COUNTf;
    non_dma0->cname = "IL_RX_PKTCNT";
    non_dma0->dma_buf[0] = buf;
    non_dma0->dma_index_min[0] = 0;
    non_dma0->dma_index_max[0] = num_entries - 1;
    non_dma0->dma_mem[0] = non_dma0->mem;
    *non_dma_entries += non_dma0->num_entries;

    /* IL_STAT_MEM_0: RX_STAT_BYTE_COUNTf */
    /* Overload SOC_COUNTER_NON_DMA_EGR_PERQ_XMT_BYTE */ 
    non_dma1 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_EGR_PERQ_XMT_BYTE -
                                     SOC_COUNTER_NON_DMA_START];
    sal_memcpy(non_dma1, non_dma0, sizeof(soc_counter_non_dma_t));
    non_dma1->flags = _SOC_COUNTER_NON_DMA_VALID;
    non_dma1->base_index = non_dma_start_index + *non_dma_entries;
    non_dma1->field = RX_STAT_BYTE_COUNTf;
    non_dma1->cname = "IL_RX_BYTECNT";
    *non_dma_entries += non_dma1->num_entries;

    /* IL_STAT_MEM_0: RX_STAT_BAD_PKT_ILERR_COUNTf */
    /* Overload SOC_COUNTER_NON_DMA_COSQ_DROP_PKT */ 
    non_dma1 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_COSQ_DROP_PKT -
                                     SOC_COUNTER_NON_DMA_START];
    sal_memcpy(non_dma1, non_dma0, sizeof(soc_counter_non_dma_t));
    non_dma1->flags = _SOC_COUNTER_NON_DMA_VALID;
    non_dma1->base_index = non_dma_start_index + *non_dma_entries;
    non_dma1->field = RX_STAT_BAD_PKT_ILERR_COUNTf;
    non_dma1->cname = "IL_RX_ERRCNT";
    *non_dma_entries += non_dma1->num_entries;


    /* IL_STAT_MEM_1 Entries */
    num_entries = soc_mem_index_count(unit, IL_STAT_MEM_1m);
    alloc_size = num_entries *
        soc_mem_entry_words(unit, IL_STAT_MEM_1m) * sizeof(uint32);

    buf = soc_cm_salloc(unit, alloc_size, "non_dma_counter");
    if (buf == NULL) {
        return SOC_E_MEMORY;
    }
    sal_memset(buf, 0, alloc_size);


    /* IL_STAT_MEM_1:  RX_STAT_GTMTU_PKT_COUNTf Greater that MTU size*/
    /* Overload SOC_COUNTER_NON_DMA_COSQ_DROP_BYTE */ 
    non_dma0 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_COSQ_DROP_BYTE -
                                     SOC_COUNTER_NON_DMA_START];
    non_dma0->flags = _SOC_COUNTER_NON_DMA_VALID |
        _SOC_COUNTER_NON_DMA_DO_DMA | _SOC_COUNTER_NON_DMA_ALLOC;
    non_dma0->pbmp = PBMP_IL_ALL(unit);
    non_dma0->base_index = non_dma_start_index + *non_dma_entries;
    non_dma0->entries_per_port = 1;
    non_dma0->num_entries = 1;
    non_dma0->mem = IL_STAT_MEM_1m;
    non_dma0->reg = INVALIDr;
    non_dma0->field =  RX_STAT_GTMTU_PKT_COUNTf;
    non_dma0->cname = "IL_RX_GTMTUCNT";
    non_dma0->dma_buf[0] = buf;
    non_dma0->dma_index_min[0] = 0;
    non_dma0->dma_index_max[0] = num_entries - 1;
    non_dma0->dma_mem[0] = non_dma0->mem;
    *non_dma_entries += non_dma0->num_entries;

    /* IL_STAT_MEM_1: RX_STAT_EQMTU_PKT_COUNTf - equal to MTU Size*/
    /* Overload SOC_COUNTER_NON_DMA_COSQ_DROP_PKT_UC */ 
    non_dma1 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_COSQ_DROP_PKT_UC -
                                     SOC_COUNTER_NON_DMA_START];
    sal_memcpy(non_dma1, non_dma0, sizeof(soc_counter_non_dma_t));
    non_dma1->flags = _SOC_COUNTER_NON_DMA_VALID;
    non_dma1->base_index = non_dma_start_index + *non_dma_entries;
    non_dma1->field = RX_STAT_EQMTU_PKT_COUNTf;
    non_dma1->cname = "IL_RX_EQMTUCNT";
    *non_dma_entries += non_dma1->num_entries;


    /* IL_STAT_MEM_1: RX_STAT_IEEE_CRCERR_PKT_COUNTf - IEEE CRC err count*/
    /* Overload SOC_COUNTER_NON_DMA_COSQ_DROP_PKT_UC */ 
    non_dma1 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_COSQ_DROP_BYTE_UC -
                                     SOC_COUNTER_NON_DMA_START];
    sal_memcpy(non_dma1, non_dma0, sizeof(soc_counter_non_dma_t));
    non_dma1->flags = _SOC_COUNTER_NON_DMA_VALID;
    non_dma1->base_index = non_dma_start_index + *non_dma_entries;
    non_dma1->field = RX_STAT_IEEE_CRCERR_PKT_COUNTf;
    non_dma1->cname = "IL_RX_CRCERRCNT";
    *non_dma_entries += non_dma1->num_entries;

    /* IL_STAT_MEM_2 Entries */
    num_entries = soc_mem_index_count(unit, IL_STAT_MEM_2m);
    alloc_size = num_entries *
        soc_mem_entry_words(unit, IL_STAT_MEM_2m) * sizeof(uint32);

    buf = soc_cm_salloc(unit, alloc_size, "non_dma_counter");
    if (buf == NULL) {
        return SOC_E_MEMORY;
    }
    sal_memset(buf, 0, alloc_size);

    /* IL_STAT_MEM_2:  RX_STAT_PKT_COUNTf Rx stat packet count*/
    /* Overload SOC_COUNTER_NON_DMA_PORT_DROP_PKT_ING */ 
    non_dma0 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_PORT_DROP_PKT_ING -
                                     SOC_COUNTER_NON_DMA_START];
    non_dma0->flags = _SOC_COUNTER_NON_DMA_VALID |
        _SOC_COUNTER_NON_DMA_DO_DMA | _SOC_COUNTER_NON_DMA_ALLOC;
    non_dma0->pbmp = PBMP_IL_ALL(unit);
    non_dma0->base_index = non_dma_start_index + *non_dma_entries;
    non_dma0->entries_per_port = 1;
    non_dma0->num_entries = 1;
    non_dma0->mem = IL_STAT_MEM_2m;
    non_dma0->reg = INVALIDr;
    non_dma0->field =  RX_STAT_PKT_COUNTf;
    non_dma0->cname = "IL_RX_STATCNT";
    non_dma0->dma_buf[0] = buf;
    non_dma0->dma_index_min[0] = 0;
    non_dma0->dma_index_max[0] = num_entries - 1;
    non_dma0->dma_mem[0] = non_dma0->mem;
    *non_dma_entries += non_dma0->num_entries;

    /* IL_STAT_MEM_3 Entries */
    num_entries = soc_mem_index_count(unit, IL_STAT_MEM_3m);
    alloc_size = num_entries *
        soc_mem_entry_words(unit, IL_STAT_MEM_3m) * sizeof(uint32);

    buf = soc_cm_salloc(unit, alloc_size, "non_dma_counter");
    if (buf == NULL) {
        return SOC_E_MEMORY;
    }
    sal_memset(buf, 0, alloc_size);

    /* IL_STAT_MEM_3:  TX_STAT_PKT_COUNT */
    /* Overload SOC_COUNTER_NON_DMA_PORT_DROP_BYTE_ING */ 
    non_dma0 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_PORT_DROP_BYTE_ING -
                                     SOC_COUNTER_NON_DMA_START];
    non_dma0->flags = _SOC_COUNTER_NON_DMA_VALID |
        _SOC_COUNTER_NON_DMA_DO_DMA | _SOC_COUNTER_NON_DMA_ALLOC;
    non_dma0->pbmp = PBMP_IL_ALL(unit);
    non_dma0->base_index = non_dma_start_index + *non_dma_entries;
    non_dma0->entries_per_port = 1;
    non_dma0->num_entries = 1;
    non_dma0->mem = IL_STAT_MEM_3m;
    non_dma0->reg = INVALIDr;
    non_dma0->field =  TX_STAT_PKT_COUNTf;
    non_dma0->cname = "IL_TX_PKTCNT";
    non_dma0->dma_buf[0] = buf;
    non_dma0->dma_index_min[0] = 0;
    non_dma0->dma_index_max[0] = 1;
    non_dma0->dma_mem[0] = non_dma0->mem;
    *non_dma_entries += non_dma0->num_entries;

    /* IL_STAT_MEM_3: TX_STAT_BYTE_COUNTf */
    /* Overload SOC_COUNTER_NON_DMA_PORT_DROP_PKT_IBP */ 
    non_dma1 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_PORT_DROP_PKT_IBP -
                                     SOC_COUNTER_NON_DMA_START];
    sal_memcpy(non_dma1, non_dma0, sizeof(soc_counter_non_dma_t));
    non_dma1->flags = _SOC_COUNTER_NON_DMA_VALID;
    non_dma1->base_index = non_dma_start_index + *non_dma_entries;
    non_dma1->field = TX_STAT_BYTE_COUNTf;
    non_dma1->cname = "IL_TX_BYTECNT";
    *non_dma_entries += non_dma1->num_entries;

    /* IL_STAT_MEM_3: TX_STAT_BAD_PKT_PERR_COUNTf */
    /* Overload SOC_COUNTER_NON_DMA_PORT_DROP_PKT_CFAP */ 
    non_dma1 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_PORT_DROP_PKT_CFAP -
                                     SOC_COUNTER_NON_DMA_START];
    sal_memcpy(non_dma1, non_dma0, sizeof(soc_counter_non_dma_t));
    non_dma1->flags = _SOC_COUNTER_NON_DMA_VALID;
    non_dma1->base_index = non_dma_start_index + *non_dma_entries;
    non_dma1->field = TX_STAT_BAD_PKT_PERR_COUNTf;
    non_dma1->cname = "IL_TX_ERRCNT";
    *non_dma_entries += non_dma1->num_entries;

    /* IL_STAT_MEM_3: TX_STAT_GTMTU_PKT_COUNTf */
    /* Overload SOC_COUNTER_NON_DMA_PORT_DROP_PKT */ 
    non_dma1 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_PORT_DROP_PKT -
                                     SOC_COUNTER_NON_DMA_START];
    sal_memcpy(non_dma1, non_dma0, sizeof(soc_counter_non_dma_t));
    non_dma1->flags = _SOC_COUNTER_NON_DMA_VALID;
    non_dma1->base_index = non_dma_start_index + *non_dma_entries;
    non_dma1->field = TX_STAT_GTMTU_PKT_COUNTf;
    non_dma1->cname = "IL_TX_GTMTUCNT";
    *non_dma_entries += non_dma1->num_entries;

    /* IL_STAT_MEM_3: TX_STAT_EQMTU_PKT_COUNT - Greater than mtu size */
    /* Overload SOC_COUNTER_NON_DMA_PORT_DROP_PKT_YELLOW */ 
    non_dma1 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_PORT_DROP_PKT_YELLOW -
                                     SOC_COUNTER_NON_DMA_START];
    sal_memcpy(non_dma1, non_dma0, sizeof(soc_counter_non_dma_t));
    non_dma1->flags = _SOC_COUNTER_NON_DMA_VALID;
    non_dma1->base_index = non_dma_start_index + *non_dma_entries;
    non_dma1->field = TX_STAT_EQMTU_PKT_COUNTf;
    non_dma1->cname = "IL_TX_EQMTUCNT";
    *non_dma_entries += non_dma1->num_entries;

    /* IL_STAT_MEM_4 Entries */
    /* IL_STAT_MEM_4 overloaded on SOC_COUNTER_NON_DMA_EGR_PERQ_XMT_PKT */ 
    num_entries = soc_mem_index_count(unit, IL_STAT_MEM_4m);
    alloc_size = num_entries *
        soc_mem_entry_words(unit, IL_STAT_MEM_4m) * sizeof(uint32);

    buf = soc_cm_salloc(unit, alloc_size, "non_dma_counter");
    if (buf == NULL) {
        return SOC_E_MEMORY;
    }
    sal_memset(buf, 0, alloc_size);

    /* TX_STAT_PKT_COUNT */ 
    non_dma1 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_PORT_DROP_PKT_RED -
                                     SOC_COUNTER_NON_DMA_START];
    non_dma1->flags = _SOC_COUNTER_NON_DMA_VALID |
        _SOC_COUNTER_NON_DMA_DO_DMA | _SOC_COUNTER_NON_DMA_ALLOC;
    non_dma1->pbmp = PBMP_IL_ALL(unit);
    non_dma1->base_index = non_dma_start_index + *non_dma_entries;
    non_dma1->entries_per_port = 1;
    non_dma1->num_entries = 1;
    non_dma1->mem = IL_STAT_MEM_4m;
    non_dma1->reg = INVALIDr;
    non_dma1->field = TX_STAT_PKT_COUNTf;
    non_dma1->cname = "IL_TX_STATCNT";
    non_dma1->dma_buf[0] = buf;
    non_dma1->dma_index_min[0] = 0;
    non_dma1->dma_index_max[0] = num_entries - 1;
    non_dma1->dma_mem[0] = non_dma1->mem;
    *non_dma_entries += non_dma1->num_entries;

    return SOC_E_NONE;
}
#endif /* BCM_SHADOW_SUPPORT */

#ifdef BCM_BRADLEY_SUPPORT
/*
 * Function:
 *      _soc_counter_hb_non_dma_init
 * Purpose:
 *      Initialize Bradley's non-DMA counters.
 * Parameters:
 *      unit - The SOC unit number
 *      nports - Number of ports
 *      non_dma_start_index - The starting index of non-DMA counter entries.
 *      non_dma_entries - (OUT) The number of non-DMA counter entries.
 * Returns:
 *      SOC_E_NONE
 */
STATIC int 
_soc_counter_hb_non_dma_init(int unit, int nports, int non_dma_start_index, 
        int *non_dma_entries)
{
    soc_control_t *soc;
    soc_counter_non_dma_t *non_dma;

    soc = SOC_CONTROL(unit);
    *non_dma_entries = 0;

    non_dma = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_PORT_DROP_PKT -
                                    SOC_COUNTER_NON_DMA_START];
    non_dma->flags = _SOC_COUNTER_NON_DMA_VALID;
    non_dma->pbmp = PBMP_ALL(unit);
    non_dma->base_index = non_dma_start_index + *non_dma_entries;
    non_dma->entries_per_port = 1;
    non_dma->num_entries = nports;
    non_dma->mem = INVALIDm;
    non_dma->reg = DROP_PKT_CNTr;
    non_dma->field = COUNTf;
    non_dma->cname = "DROP_PKT_MMU";
    *non_dma_entries += non_dma->num_entries;

    return SOC_E_NONE;
}
#endif /* BCM_BRADLEY_SUPPORT */

#ifdef BCM_FIREBOLT_SUPPORT
/*
 * Function:
 *      _soc_counter_fb_non_dma_init
 * Purpose:
 *      Initialize Firebolt's non-DMA counters.
 * Parameters:
 *      unit - The SOC unit number
 *      nports - Number of ports
 *      non_dma_start_index - The starting index of non-DMA counter entries.
 *      non_dma_entries - (OUT) The number of non-DMA counter entries.
 * Returns:
 *      SOC_E_NONE
 */
STATIC int
_soc_counter_fb_non_dma_init(int unit, int nports, int non_dma_start_index,
        int *non_dma_entries)
{
    soc_control_t *soc;
    soc_counter_non_dma_t *non_dma0, *non_dma1;

    soc = SOC_CONTROL(unit);
    *non_dma_entries = 0;

    non_dma0 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_PORT_DROP_PKT_YELLOW -
                                     SOC_COUNTER_NON_DMA_START];
    non_dma0->flags = _SOC_COUNTER_NON_DMA_VALID;
    non_dma0->pbmp = PBMP_ALL(unit);
    non_dma0->base_index = non_dma_start_index + *non_dma_entries;
    non_dma0->entries_per_port = 1;
    non_dma0->num_entries = nports;
    non_dma0->mem = INVALIDm;
    non_dma0->reg = CNGDROPCOUNT1r;
    non_dma0->field = DROPPKTCOUNTf;
    non_dma0->cname = "DROP_PKT_YEL";
    *non_dma_entries += non_dma0->num_entries;

    non_dma1 = &soc->counter_non_dma[SOC_COUNTER_NON_DMA_PORT_DROP_PKT_RED -
                                     SOC_COUNTER_NON_DMA_START];
    sal_memcpy(non_dma1, non_dma0, sizeof(soc_counter_non_dma_t));
    non_dma1->base_index = non_dma_start_index + *non_dma_entries;
    non_dma1->reg = CNGDROPCOUNT0r;
    non_dma1->cname = "DROP_PKT_RED";
    *non_dma_entries += non_dma1->num_entries;

    return SOC_E_NONE;
}
#endif /* BCM_FIREBOLT_SUPPORT */

/*
 * Function:
 *      soc_counter_attach
 * Purpose:
 *      Initialize counter module.
 * Notes:
 *      Allocates counter collection buffers.
 *      We need to work with the true data for the chip, not the current
 *      pbmp_valid settings.  They may be changed after attach, but the
 *      memory won't be reallocated at that time.
 */

int
soc_counter_attach(int unit)
{
    soc_control_t       *soc;
    int                 n_entries, n_bytes;
    int                 non_dma_entries = 0;
    int                 portsize, nports, ports32, ports64;
    int                 blk, bindex, port, phy_port;
    int                 blktype, ctype, idx;

    assert(SOC_UNIT_VALID(unit));

    soc = SOC_CONTROL(unit);

    soc->counter_pid = SAL_THREAD_ERROR;
    soc->counter_interval = 0;
    SOC_PBMP_CLEAR(soc->counter_pbmp);
    soc->counter_trigger = NULL;
    soc->counter_intr = NULL;
    /* Note that flags will be reinitialized in soc_counter_start */
    if (soc_feature(unit, soc_feature_stat_dma)) {
        soc->counter_flags = SOC_COUNTER_F_DMA;
    } else {
        soc->counter_flags = 0;
    }
    soc->counter_coll_prev = soc->counter_coll_cur = sal_time_usecs();

    portsize = 0;
    port = 0;
    /* We can't use pbmp_valid calculations, so we must do this manually. */
    for (phy_port = 0; ; phy_port++) {
        blk = SOC_PORT_INFO(unit, phy_port).blk;
        bindex = SOC_PORT_INFO(unit, phy_port).bindex;
        if (blk < 0 && bindex < 0) {                    /* end of list */
            break;
        }
        if (soc_feature(unit, soc_feature_logical_port_num)) {
            if (port < SOC_INFO(unit).port_p2l_mapping[phy_port]) {
                port = SOC_INFO(unit).port_p2l_mapping[phy_port];
            }
        } else {
            port = phy_port;
        }
    }
    port++;

    /* Don't count last port if CMIC */
    if (CMIC_PORT(unit) == (port - 1)) {
        /*
         * On XGS3 switch CMIC port stats are DMAable. Allocate space
         * in case these stats need to be DMAed as well.
         */
        if (!soc_feature(unit, soc_feature_cpuport_stat_dma)) {
            port--;      
        }
    }
    nports = port;      /* 0..max_port_num */
    ports32 = ports64 = 0;

    if (SOC_IS_DRACO1(unit)) {
        portsize = 512;
        if (IPIC_PORT(unit) >= 0) {
            ports64 = 1;
            ports32 = nports - 1;
        } else {
            ports32 = nports;
        }
    } else if (SOC_IS_HB_GW(unit)) {
        portsize = 2048;
        ports64 = nports;
    } else if (SOC_IS_TRX(unit)) {
        portsize = 4096;
        ports64 = nports;
    } else if ((SOC_IS_HERCULES(unit))
                || (SOC_IS_XGS3_SWITCH(unit))
                || (SOC_IS_LYNX(unit))) {
        portsize = 1024;
        ports64 = nports;
    } else if (SOC_IS_DRACO15(unit)) {
        portsize = 1024;
        if (IPIC_PORT(unit) >= 0) {
            ports64 = 1;
            ports32 = nports - 1;
        } else {
            ports32 = nports;
        }
    } else if (SOC_IS_TUCANA(unit)) {
        portsize = 2048;
        ports64 = nports;
#if defined(BCM_SIRIUS_SUPPORT)
    } else if (SOC_IS_SIRIUS(unit)) {
        portsize = 2048;
        ports64 = nports;
#endif
#if defined(BCM_CALADAN3_SUPPORT)
        
    } else if (SOC_IS_CALADAN3(unit)) {
        portsize = 2048;
        ports64 = nports;
#endif
    } else {
        soc_cm_debug(DK_ERR,
                     "soc_counter_attach: unit %d: unexpected chip type\n",
                     unit);
        return SOC_E_FAIL;
    }

    if (ports32) {
        soc->counter_perport = portsize / sizeof(uint32);
    } else {
        soc->counter_perport = portsize / sizeof(uint64);
    }
    soc->counter_n32 = (portsize / sizeof(uint32)) * ports32;
    soc->counter_n64 = (portsize / sizeof(uint64)) * ports64;
    soc->counter_ports32 = ports32;
    soc->counter_ports64 = ports64;
    n_bytes = portsize * nports;

    /*
     * allocate extra space since draco1 has more counters than
     * will fit in the normal dma block
     */
    if (SOC_IS_DRACO1(unit)) {
        soc->counter_n64 += portsize / sizeof(uint64);
        n_bytes += portsize;
    }

    n_entries = soc->counter_n32 + soc->counter_n64;
    soc->counter_portsize = portsize;
    soc->counter_bufsize = n_bytes;

    soc_cm_debug(DK_VERBOSE,
                 "soc_counter_attach: %d bytes/port, %d ports, %d ctrs/port, "
                 "%d ports with %d 32bit ctrs, "
                 "%d ports with %d 64bit ctrs\n",
                 portsize, nports, soc->counter_perport,
                 ports32, soc->counter_n32,
                 ports64, soc->counter_n64);

    /* Initialize Non-DMA counters */

    if (soc->counter_non_dma == NULL) {
        soc->counter_non_dma = sal_alloc((SOC_COUNTER_NON_DMA_END -
                                         SOC_COUNTER_NON_DMA_START) *
                                         sizeof(soc_counter_non_dma_t),
                                         "cntr_non_dma");
        if (soc->counter_non_dma == NULL) {
            goto error;
        }
    } else {
        for (idx = 0; idx < SOC_COUNTER_NON_DMA_END - SOC_COUNTER_NON_DMA_START;
             idx++) {
            if (soc->counter_non_dma[idx].flags & _SOC_COUNTER_NON_DMA_ALLOC) {
                soc_cm_sfree(unit, soc->counter_non_dma[idx].dma_buf[0]);
            }
        }
    }
    sal_memset(soc->counter_non_dma, 0,
               (SOC_COUNTER_NON_DMA_END - SOC_COUNTER_NON_DMA_START) *
               sizeof(soc_counter_non_dma_t));

    /* Initialize the number of cosq counters per port */
    _soc_counter_num_cosq_init(unit, nports);


#ifdef BCM_TRIUMPH_SUPPORT
#ifdef BCM_TRIDENT_SUPPORT
    if (SOC_IS_TD_TT(unit)) {
        if (SOC_FAILURE(_soc_counter_trident_non_dma_init(unit, nports, n_entries, 
                        &non_dma_entries))) {
            goto error;
        }
    } else
#endif /* BCM_TRIDENT_SUPPORT */
#ifdef BCM_HURRICANE_SUPPORT
    if (SOC_IS_HURRICANE(unit)) {
        if (SOC_FAILURE(_soc_counter_hu_non_dma_init(unit, nports, n_entries, 
                        &non_dma_entries))) {
            goto error;
        }
    } else
#endif /* BCM_HURRICANE_SUPPORT */
#ifdef BCM_KATANA_SUPPORT
    if (SOC_IS_KATANA(unit)) {
        if (SOC_FAILURE(_soc_counter_katana_non_dma_init(unit, nports, n_entries, 
                        &non_dma_entries))) {
            goto error;
        }
    } else
#endif /* BCM_KATANA_SUPPORT */
    if (SOC_IS_TR_VL(unit)) {
        if (SOC_FAILURE(_soc_counter_tr_non_dma_init(unit, nports, n_entries, 
                        &non_dma_entries))) {
            goto error;
        }
    } else
#endif /* BCM_TRIUMPH_SUPPORT */
#ifdef BCM_SCORPION_SUPPORT
    if (SOC_IS_SC_CQ(unit) && (!(SOC_IS_SHADOW(unit)))) {
        if (SOC_FAILURE(_soc_counter_sc_non_dma_init(unit, nports, n_entries, 
                        &non_dma_entries))) {
            goto error;
        }
    } else
#endif /* BCM_SCORPION_SUPPORT */
#ifdef BCM_SHADOW_SUPPORT
    if (SOC_IS_SHADOW(unit)) {
        if (!SOC_PBMP_IS_NULL(PBMP_IL_ALL(unit))) {
            if (SOC_FAILURE(_soc_counter_shadow_non_dma_init(unit, nports, 
                            n_entries, &non_dma_entries))) {
                goto error;
            }
        }
    } else
#endif /* BCM_SHADOW_SUPPORT */
#ifdef BCM_BRADLEY_SUPPORT
    if (SOC_IS_HB_GW(unit)) {
        if (SOC_FAILURE(_soc_counter_hb_non_dma_init(unit, nports, n_entries, 
                        &non_dma_entries))) {
            goto error;
        }
    } else
#endif /* BCM_BRADLEY_SUPPORT */
#ifdef BCM_FIREBOLT_SUPPORT
    if (SOC_IS_FB_FX_HX(unit)) {
        if (SOC_FAILURE(_soc_counter_fb_non_dma_init(unit, nports, n_entries, 
                        &non_dma_entries))) {
            goto error;
        }
    } else
#endif /* BCM_FIREBOLT_SUPPORT */
    {
        non_dma_entries = 0;
    }

    soc->counter_n64_non_dma = non_dma_entries;

    /*
     * Counter DMA buffer allocation (32 and 64 bit allocated together)
     */

    assert(n_bytes > 0);

    /* Hardware DMA buf */
    if (soc->counter_buf32 == NULL) {
        soc->counter_buf32 = soc_cm_salloc(unit, n_bytes, "cntr_dma_buf");
        if (soc->counter_buf32 == NULL) {
            goto error;
        }
        soc->counter_buf64 = (uint64 *)&soc->counter_buf32[soc->counter_n32];
    }
    sal_memset(soc->counter_buf32, 0, n_bytes);

    n_entries += non_dma_entries;

    /* Hardware value buf */
    if (soc->counter_hw_val == NULL) {
        soc->counter_hw_val = sal_alloc(n_entries * sizeof(uint64),
                                        "cntr_hw_val");
        if (soc->counter_hw_val == NULL) {
            goto error;
        }
    }
    sal_memset(soc->counter_hw_val, 0, n_entries * sizeof(uint64));

    /* Software value buf */
    if (soc->counter_sw_val == NULL) {
        soc->counter_sw_val = sal_alloc(n_entries * sizeof(uint64),
                                        "cntr_sw_val");
        if (soc->counter_sw_val == NULL) {
            goto error;
        }
    }
    sal_memset(soc->counter_sw_val, 0, n_entries * sizeof(uint64));

    /* Delta buf */
    if (soc->counter_delta == NULL) {
        soc->counter_delta = sal_alloc(n_entries * sizeof(uint64),
                                       "cntr_delta");
        if (soc->counter_delta == NULL) {
            goto error;
        }
    }
    sal_memset(soc->counter_delta, 0, n_entries * sizeof(uint64));

    /* Set up and install counter maps */

    /* We can't use pbmp_valid calculations, so we must do this manually. */
    for (phy_port = 0; ; phy_port++) {
	blk = SOC_PORT_INFO(unit, phy_port).blk;
	bindex = SOC_PORT_INFO(unit, phy_port).bindex;
        if (blk < 0 && bindex < 0) {                    /* end of list */
            break;
        }
	if (blk < 0) {                                  /* empty slot */
	    continue;       
	}
        if (soc_feature(unit, soc_feature_logical_port_num)) {
            port = SOC_INFO(unit).port_p2l_mapping[phy_port];
            if (port < 0) {
                continue;
            }
        } else {
            port = phy_port;
            if (port >= nports) {
                continue;
            }
        }
	blktype = SOC_BLOCK_INFO(unit, blk).type;
	switch (blktype) {
	    case SOC_BLK_EPIC:
		ctype = SOC_CTR_TYPE_FE;
		break;
	    case SOC_BLK_GPIC:
	    case SOC_BLK_GPORT:
	    case SOC_BLK_QGPORT:
	    case SOC_BLK_SPORT:
		ctype = SOC_CTR_TYPE_GE;
		break;
	    case SOC_BLK_XQPORT:
	    case SOC_BLK_XGPORT:
                if (IS_XE_PORT(unit, port) || IS_HG_PORT(unit, port)) {
                    ctype = SOC_CTR_TYPE_XE;
                } else {
                    ctype = SOC_CTR_TYPE_GE;
                }
                break;
	    case SOC_BLK_IPIC:
	    case SOC_BLK_HPIC:
		ctype = SOC_CTR_TYPE_HG;
		break;
	    case SOC_BLK_XPIC:
	    case SOC_BLK_XPORT:
            case SOC_BLK_GXPORT:
            case SOC_BLK_XLPORT:
            case SOC_BLK_XTPORT:
            case SOC_BLK_CLPORT:
            case SOC_BLK_MXQPORT:
		ctype = SOC_CTR_TYPE_XE;
		break;
	    case SOC_BLK_CMIC:
		if (soc_feature(unit, soc_feature_cpuport_stat_dma)) {
		    ctype = SOC_CTR_TYPE_CPU;
		    break;
		}
	    case SOC_BLK_CPIC:
	    default:
		continue;
	}

        if (!SOC_CONTROL(unit)->counter_map[port]) {
            SOC_CONTROL(unit)->counter_map[port] =
                &SOC_CTR_DMA_MAP(unit, ctype);
            assert(SOC_CONTROL(unit)->counter_map[port]);
        }
        assert(SOC_CONTROL(unit)->counter_map[port]->cmap_base);

#ifdef BCM_TRIUMPH2_SUPPORT
        /* We need this done in a port loop like this one, so we're
         * piggybacking on it. */

        if (soc_feature(unit, soc_feature_timestamp_counter)) {
            /* A HW register implemented as a FIFO-pop on read is in the
             * counter collection space.  We must copy the DMA'd values 
             * into a SW FIFO and make them available to the application
             * on request.
             */
            if (soc->counter_timestamp_fifo[port] == NULL) {
                soc->counter_timestamp_fifo[port] =
                    sal_alloc(sizeof(shr_fifo_t), "Timestamp counter FIFO");
                if (soc->counter_timestamp_fifo[port] == NULL) {
                    goto error;
                }
            } else {
                SHR_FIFO_FREE(soc->counter_timestamp_fifo[port]);
            }
            sal_memset(soc->counter_timestamp_fifo[port], 0,
                       sizeof(shr_fifo_t));
            SHR_FIFO_ALLOC(soc->counter_timestamp_fifo[port],
                           COUNTER_TIMESTAMP_FIFO_SIZE, sizeof(uint32),
                           SHR_FIFO_FLAG_DO_NOT_PUSH_DUPLICATE);
        }
#endif /* BCM_TRIUMPH2_SUPPORT */
    }

    return SOC_E_NONE;

 error:
#ifdef BCM_TRIUMPH2_SUPPORT
    for (port = 0; port < SOC_MAX_NUM_PORTS; port++) {
        if (soc->counter_timestamp_fifo[port] != NULL) {
            SHR_FIFO_FREE(soc->counter_timestamp_fifo[port]);
            sal_free(soc->counter_timestamp_fifo[port]);
            soc->counter_timestamp_fifo[port] = NULL;
        }
    }
#endif /* BCM_TRIUMPH2_SUPPORT */

    if (soc->counter_non_dma != NULL) {
        for (idx = 0;
             idx < SOC_COUNTER_NON_DMA_END - SOC_COUNTER_NON_DMA_START;
             idx++) {
            if (soc->counter_non_dma[idx].flags & _SOC_COUNTER_NON_DMA_ALLOC) {
                soc_cm_sfree(unit, soc->counter_non_dma[idx].dma_buf[0]);
            }
        }
        sal_free(soc->counter_non_dma);
        soc->counter_non_dma = NULL;
    }

    if (soc->counter_buf32 != NULL) {
        soc_cm_sfree(unit, soc->counter_buf32);
        soc->counter_buf32 = NULL;
        soc->counter_buf64 = NULL;
    }

    if (soc->counter_hw_val != NULL) {
        sal_free(soc->counter_hw_val);
        soc->counter_hw_val = NULL;
    }

    if (soc->counter_sw_val != NULL) {
        sal_free(soc->counter_sw_val);
        soc->counter_sw_val = NULL;
    }

    if (soc->counter_delta != NULL) {
        sal_free(soc->counter_delta);
        soc->counter_delta = NULL;
    }

    return SOC_E_MEMORY;
}

/*
 * Function:
 *      soc_port_cmap_get/set
 * Purpose:
 *      Access the counter map structure per port
 */

soc_cmap_t *
soc_port_cmap_get(int unit, soc_port_t port)
{
    if (!SOC_UNIT_VALID(unit)) {
        return NULL;
    }
    if (!SOC_PORT_VALID(unit, port) || 
        (!IS_PORT(unit, port) &&
         (!soc_feature(unit, soc_feature_cpuport_stat_dma)))) {
        return NULL;
    }
    return SOC_CONTROL(unit)->counter_map[port];
}

int
soc_port_cmap_set(int unit, soc_port_t port, soc_ctr_type_t ctype)
{
    if (!SOC_UNIT_VALID(unit)) {
        return SOC_E_UNIT;
    }
    if (!SOC_PORT_VALID(unit, port) ||
        (!IS_PORT(unit, port) &&
         (!soc_feature(unit, soc_feature_cpuport_stat_dma)))) {
        return SOC_E_PARAM;
    }
    SOC_CONTROL(unit)->counter_map[port] = &SOC_CTR_DMA_MAP(unit, ctype);
    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_counter_detach
 * Purpose:
 *      Finalize counter module.
 * Notes:
 *      Stops counter task if running.
 *      Deallocates counter collection buffers.
 */

int
soc_counter_detach(int unit)
{
    soc_control_t       *soc;
    int                 i;
#ifdef BCM_TRIUMPH2_SUPPORT
    soc_port_t          port;
#endif /* BCM_TRIUMPH2_SUPPORT */

    assert(SOC_UNIT_VALID(unit));

    soc = SOC_CONTROL(unit);

    SOC_IF_ERROR_RETURN(soc_counter_stop(unit));

#ifdef BCM_TRIUMPH2_SUPPORT
    for (port = 0; port < SOC_MAX_NUM_PORTS; port++) {
        if (soc->counter_timestamp_fifo[port] != NULL) {
            SHR_FIFO_FREE(soc->counter_timestamp_fifo[port]);
            sal_free(soc->counter_timestamp_fifo[port]);
            soc->counter_timestamp_fifo[port] = NULL;
        }
    }
#endif /* BCM_TRIUMPH2_SUPPORT */

    if (soc->counter_non_dma != NULL) {
        for (i = 0; i < SOC_COUNTER_NON_DMA_END - SOC_COUNTER_NON_DMA_START;
             i++) {
            if (soc->counter_non_dma[i].flags & _SOC_COUNTER_NON_DMA_ALLOC) {
                soc_cm_sfree(unit, soc->counter_non_dma[i].dma_buf[0]);
            }
        }
        sal_free(soc->counter_non_dma);
        soc->counter_non_dma = NULL;
    }

    if (soc->counter_buf32 != NULL) {
        soc_cm_sfree(unit, soc->counter_buf32);
        soc->counter_buf32 = NULL;
        soc->counter_buf64 = NULL;
    }

    if (soc->counter_hw_val != NULL) {
        sal_free(soc->counter_hw_val);
        soc->counter_hw_val = NULL;
    }

    if (soc->counter_sw_val != NULL) {
        sal_free(soc->counter_sw_val);
        soc->counter_sw_val = NULL;
    }

    if (soc->counter_delta != NULL) {
        sal_free(soc->counter_delta);
        soc->counter_delta = NULL;
    }

    return SOC_E_NONE;
}

/*
 * StrataSwitch counter register map
 *
 * NOTE: soc_attach verifies this map is correct and prints warnings if
 * not.  The map should contain only registers with a single field named
 * COUNT, and all such registers should be in the map.
 *
 * The soc_counter_map[] array is a list of counter registers in the
 * order found in the internal address map and counter DMA buffer.
 * soc_counter_map[0] corresponds to S-Channel address
 * COUNTER_OFF_MIN, and contains SOC_CTR_MAP_SIZE(unit) entries.
 *
 * This map structure, contents, and size are exposed only to provide a
 * convenient way to loop through all available counters.
 *
 * Accumulated counters are stored as 64-bit values and may be written
 * and read by multiple tasks on a 32-bit processor.  This requires
 * atomic operation to get correct values.  Also, to fetch-and-clear a
 * counter requires atomic operation.
 *
 * These atomic operations are very brief, so rather than using an
 * inefficient mutex we use splhi to lock out interrupts and task
 * switches.  In theory, splhi is only a few instructions on most
 * processors.
 */

#define COUNTER_ATOMIC_DEF              int
#define COUNTER_ATOMIC_BEGIN(s)         ((s) = sal_splhi())
#define COUNTER_ATOMIC_END(s)           (sal_spl(s))

#ifdef BROADCOM_DEBUG

char *soc_ctr_type_names[] = SOC_CTR_TYPE_NAMES_INITIALIZER;

/*
 * Function:
 *      _soc_counter_verify (internal)
 * Purpose:
 *      Verify contents of soc_counter_map[] array
 */
void
_soc_counter_verify(int unit)
{
    int                 i, errors, num_ctrs, found;
    soc_ctr_type_t      ctype;
    soc_reg_t           reg;
    int                 ar_idx;

    soc_cm_debug(DK_VERBOSE, "soc_counter_verify: unit %d begins\n", unit);

    errors = 0;

    for (ctype = SOC_CTR_TYPE_FE; ctype < SOC_CTR_NUM_TYPES; ctype++) {
        if (SOC_HAS_CTR_TYPE(unit, ctype)) {
            num_ctrs = SOC_CTR_MAP_SIZE(unit, ctype);
            for (i = 0; i < num_ctrs; i++) {
                reg = SOC_CTR_TO_REG(unit, ctype, i);
                ar_idx = SOC_CTR_TO_REG_IDX(unit, ctype, i);
                if (SOC_COUNTER_INVALID(unit, reg)) {
                    continue;
                }
                if (!SOC_REG_IS_COUNTER(unit, reg)) {
                    soc_cm_debug(DK_VERBOSE,
                                 "soc_counter_verify: "
                                 "%s cntr %s (%d) index %d "
                                 "is not a counter\n",
                                 soc_ctr_type_names[ctype],
                                 SOC_REG_NAME(unit, reg), reg, i);
                    errors = 1;
                }
                if (((SOC_REG_CTR_IDX(unit, reg) + ar_idx) -
                     COUNTER_IDX_OFFSET(unit))
                    != i) {
                    soc_cm_debug(DK_VERBOSE, "soc_counter_verify: "
                         "%s cntr %s (%d) index mismatch.\n"
                         "    (ctr_idx %d + ar_idx %d) - offset %d != "
                         "index in ctr array %d\n",
                                 soc_ctr_type_names[ctype],
                                 SOC_REG_NAME(unit, reg), reg,
                                 SOC_REG_CTR_IDX(unit, reg),
                                 ar_idx,
                                 COUNTER_IDX_OFFSET(unit),
                                 i);
                    errors = 1;
                }
            }
        }
    }

    for (reg = 0; reg < NUM_SOC_REG; reg++) {
        if (SOC_REG_IS_VALID(unit, reg) && SOC_REG_IS_COUNTER(unit, reg)) {
            found = FALSE;
            i = -1;
            for (ctype = SOC_CTR_TYPE_FE; ctype < SOC_CTR_NUM_TYPES; ctype++) {
                if (SOC_HAS_CTR_TYPE(unit, ctype)) {
                    num_ctrs = SOC_CTR_MAP_SIZE(unit, ctype);
                    for (i = 0; i < num_ctrs; i++) {
                        if (SOC_CTR_TO_REG(unit, ctype, i) == reg) {
                            if ((SOC_REG_CTR_IDX(unit, reg) -
                                 COUNTER_IDX_OFFSET(unit)) != i) {
                                soc_cm_debug(DK_VERBOSE,
                                   "soc_counter_verify: "
                                   "%s cntr %s (%d) index mismatch.\n"
                                   "    (ctr_idx %d - offset %d) != "
                                   "index in ctr array %d\n",
                                             soc_ctr_type_names[ctype],
                                             SOC_REG_NAME(unit, reg), reg,
                                             SOC_REG_CTR_IDX(unit, reg),
                                             COUNTER_IDX_OFFSET(unit),
                                             i);
                                errors = 1;
                            }
                            found = TRUE;
                            break;
                        }
                    }
                }
                if (found) {
                    break;
                }
            }

#if defined(BCM_ENDURO_SUPPORT) || defined(BCM_KATANA_SUPPORT)
            if(SOC_IS_ENDURO(unit) || SOC_IS_HURRICANE(unit) || SOC_IS_KATANA(unit)) {
                /* OAM_SEC_NS_COUNTER_64 is a general purpose counter 
                   which counts time for entire chip */    
                if ((!found) && (OAM_SEC_NS_COUNTER_64r == reg)) {
                    found = TRUE;
                }
            }
#endif
#ifdef BCM_HURRICANE_SUPPORT
			if(SOC_IS_HURRICANE(unit)) {
				if( (!found) && (
					(HOLD_COS0r == reg) ||
					(HOLD_COS1r == reg) ||
					(HOLD_COS2r == reg) ||
					(HOLD_COS3r == reg) ||
					(HOLD_COS4r == reg) ||
					(HOLD_COS5r == reg) ||
					(HOLD_COS6r == reg) ||
					(HOLD_COS7r == reg) ) ) {
                    found = TRUE;
				}
			}
#endif

            if (!found) {
                soc_cm_debug(DK_VERBOSE,
                             "soc_counter_verify: "
                             "counter %d %s is missing (i=%d, 0x%x)\n",
                             (int)reg, SOC_REG_NAME(unit, reg), i, i);
                errors = 1;
            }
        }
    }

    if (errors) {
        soc_cm_print("\nERRORS found during counter initialization.  "
                     "Set debug verbose for more info.\n\n");
    }

    soc_cm_debug(DK_VERBOSE, "soc_counter_verify: unit %d ends\n", unit);
}

/*
 * Function:
 *      _soc_counter_illegal
 * Purpose:
 *      Routine to display info about bad counter and halt.
 */

void
_soc_counter_illegal(int unit, soc_reg_t ctr_reg)
{
    soc_cm_print("soc_counter_get: unit %d: "
                 "ctr_reg %d (%s) is not a counter\n",
                 unit, ctr_reg, SOC_REG_NAME(unit, ctr_reg));
    /* assert(0); */
}

#endif /* BROADCOM_DEBUG */

/*
 * Function:
 *      soc_counter_autoz
 * Purpose:
 *      Set or clear AUTOZ mode for all MAC counter registers (all ports)
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      enable - if TRUE, enable, otherwise disable AUTOZ mode
 * Returns:
 *      SOC_E_XXX.
 * Notes:
 *      This module requires AUTOZ is off to accumulate counters properly.
 *      On 5665, this also turns on/off the MMU counters AUTOZ mode.
 */

int
soc_counter_autoz(int unit, int enable)
{
    soc_port_t          port;

    soc_cm_debug(DK_COUNTER,
                 "soc_counter_autoz: unit=%d enable=%d\n", unit, enable);

    PBMP_PORT_ITER(unit, port) {
#if defined(BCM_ESW_SUPPORT)
        SOC_IF_ERROR_RETURN(soc_autoz_set(unit, port, enable));
#endif
    }

    if (SOC_IS_TUCANA(unit)) {
        uint32         regval;
        SOC_IF_ERROR_RETURN(READ_MISCCONFIGr(unit, &regval));
        soc_reg_field_set(unit, MISCCONFIGr, &regval, CTRREADCLEARENf, 0);
        SOC_IF_ERROR_RETURN(WRITE_MISCCONFIGr(unit, regval));
    }

    return (SOC_E_NONE);
}

/*
 * Function:
 *      _soc_counter_get
 * Purpose:
 *      Given a counter register number and port number, fetch the
 *      64-bit software-accumulated counter value.  The software-
 *      accumulated counter value is zeroed if requested.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      ctr_ref - ref structure for counter register.
 *      zero - if TRUE, current counter is zeroed after reading.
 *      val - (OUT) 64-bit counter value.
 * Returns:
 *      SOC_E_XXX.
 * Notes:
 *      Returns 0 if ctr_reg is INVALIDr or not enabled.
 */

STATIC INLINE int
_soc_counter_get(int unit, soc_port_t port, soc_reg_t ctr_reg, int ar_idx,
                 int zero, uint64 *val)
{
    soc_control_t       *soc = SOC_CONTROL(unit);
    int                 port_index, num_entries;
    char                *cname;
    uint64              *vptr;
    uint64              value;
    COUNTER_ATOMIC_DEF  s;

    SOC_IF_ERROR_RETURN(_soc_counter_get_info(unit, port, ctr_reg,
                                              &port_index, &num_entries,
                                              &cname));
    if (ar_idx >= num_entries) {
        return SOC_E_PARAM;
    }

    if (ar_idx > 0 && ar_idx < num_entries) {
        port_index += ar_idx;
    }

    /* Try to minimize the atomic section as much as possible */
    if (ctr_reg >= NUM_SOC_REG &&
        (soc->counter_non_dma[ctr_reg - SOC_COUNTER_NON_DMA_START].flags &
         _SOC_COUNTER_NON_DMA_CURRENT)) {
        vptr = &soc->counter_hw_val[port_index];
    } else {
        vptr = &soc->counter_sw_val[port_index];
    }
    if (zero) {
        COUNTER_ATOMIC_BEGIN(s);
        value = *vptr;
        COMPILER_64_ZERO(*vptr);
        COUNTER_ATOMIC_END(s);
    } else {
        COUNTER_ATOMIC_BEGIN(s);
        value = *vptr;
        COUNTER_ATOMIC_END(s);
    }

    soc_cm_debug(DK_COUNTER + DK_VERBOSE,
                 "cntr get %s port=%d "
                 "port_index=%d vptr=%p val=0x%08x_%08x\n",
                 cname,
                 port,
                 port_index,
                 (void *)vptr, COMPILER_64_HI(value), COMPILER_64_LO(value));

    *val = value;

    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_counter_get, soc_counter_get32
 *      soc_counter_get_zero, soc_counter_get32_zero
 * Purpose:
 *      Given a counter register number and port number, fetch the
 *      64-bit software-accumulated counter value (normal versions)
 *      or the 64-bit software-accumulated counter value truncated to
 *      32 bits (32-bit versions).  The software-accumulated counter
 *      value is zeroed if requested.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      ctr_reg - counter register to retrieve.
 *      val - (OUT) 64/32-bit counter value.
 * Returns:
 *      SOC_E_XXX.
 * Notes:
 *      The 32-bit version returns only the lower 32 bits of the 64-bit
 *      counter value.
 */

int
soc_counter_get(int unit, soc_port_t port, soc_reg_t ctr_reg,
                int ar_idx, uint64 *val)
{
    return _soc_counter_get(unit, port, ctr_reg, ar_idx, FALSE, val);
}

int
soc_counter_get32(int unit, soc_port_t port, soc_reg_t ctr_reg,
                  int ar_idx, uint32 *val)
{
    uint64              val64;
    int                 rv;

    rv = _soc_counter_get(unit, port, ctr_reg, ar_idx, FALSE, &val64);

    if (rv >= 0) {
        COMPILER_64_TO_32_LO(*val, val64);
    }

    return rv;
}

int
soc_counter_get_zero(int unit, soc_port_t port,
                     soc_reg_t ctr_reg, int ar_idx, uint64 *val)
{
    return _soc_counter_get(unit, port, ctr_reg, ar_idx, TRUE, val);
}

int
soc_counter_get32_zero(int unit, soc_port_t port,
                       soc_reg_t ctr_reg, int ar_idx, uint32 *val)
{
    uint64              val64;
    int                 rv;

    rv = _soc_counter_get(unit, port, ctr_reg, ar_idx, TRUE, &val64);

    if (rv >= 0) {
        COMPILER_64_TO_32_LO(*val, val64);
    }

    return rv;
}

/*
 * Function:
 *      soc_counter_get_rate
 * Purpose:
 *      Returns the counter incrementing rate in units of 1/sec.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      ctr_reg - counter register to retrieve.
 *      rate - (OUT) 64-bit rate value.
 * Returns:
 *      SOC_E_XXX.
 */

int
soc_counter_get_rate(int unit, soc_port_t port,
                     soc_reg_t ctr_reg, int ar_idx, uint64 *rate)
{
    soc_control_t       *soc = SOC_CONTROL(unit);
    int                 port_index, num_entries;

    SOC_IF_ERROR_RETURN(_soc_counter_get_info(unit, port, ctr_reg,
                                              &port_index, &num_entries,
                                              NULL));
    if (ar_idx >= num_entries) {
        return SOC_E_PARAM;
    }

    if (ar_idx > 0 && ar_idx < num_entries) {
        port_index += ar_idx;
    }

    if (soc->counter_interval == 0) {
        COMPILER_64_ZERO(*rate);
    } else {
#ifdef  COMPILER_HAS_DOUBLE
        /*
         * This uses floating point right now because uint64 multiply/divide
         * support is missing from many compiler libraries.
         */

        uint32          delta32, rate32;
        double          interval;

        COMPILER_64_TO_32_LO(delta32, soc->counter_delta[port_index]);

        interval = SAL_USECS_SUB(soc->counter_coll_cur,
                                 soc->counter_coll_prev) / 1000000.0;

        if (interval < 0.0001) {
            rate32 = 0;
        } else {
            rate32 = (uint32) (delta32 / interval + 0.5);
        }

        COMPILER_64_SET(*rate, 0, rate32);
#else   /* !COMPILER_HAS_DOUBLE */
        uint32          delta32, rate32;
        int             interval;

        COMPILER_64_TO_32_LO(delta32, soc->counter_delta[port_index]);

        interval = SAL_USECS_SUB(soc->counter_coll_cur, soc->counter_coll_prev);

        if (interval < 100) {
            rate32 = 0;
        } else {
            rate32 = _shr_div_exp10(delta32, interval, 6);
        }

        COMPILER_64_SET(*rate, 0, rate32);
#endif  /* !COMPILER_HAS_DOUBLE */
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_counter_get32_rate
 * Purpose:
 *      Returns the counter incrementing rate in units of 1/sec.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      ctr_reg - counter register to retrieve.
 *      rate - (OUT) 32-bit delta value.
 * Returns:
 *      SOC_E_XXX.
 */

int
soc_counter_get32_rate(int unit, soc_port_t port,
                       soc_reg_t ctr_reg, int ar_idx, uint32 *rate)
{
    uint64              rate64;

    SOC_IF_ERROR_RETURN(soc_counter_get_rate(unit, port, ctr_reg,
                                             ar_idx, &rate64));

    COMPILER_64_TO_32_LO(*rate, rate64);

    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_counter_set, soc_counter_set32
 * Purpose:
 *      Given a counter register number, port number, and a counter
 *      value, set both the hardware and software-accumulated counters
 *      to the value.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port #.
 *      ctr_reg - counter register to retrieve.
 *      val - 64/32-bit counter value.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      The 32-bit version sets the upper 32-bits of the 64-bit
 *      software counter to zero.  The specified value is truncated
 *      to the width of the hardware register when storing to hardware.
 *      Use the value 0 to clear counters.
 */

STATIC int
_soc_counter_set(int unit, soc_port_t port, soc_reg_t ctr_reg,
                 int ar_idx, uint64 val64)
{
    soc_control_t       *soc = SOC_CONTROL(unit);
    int                 port_index, num_entries;
    int                 index_min, index_max, index;
    soc_counter_non_dma_t *non_dma;
    uint32              entry[SOC_MAX_MEM_FIELD_WORDS], fval[2];
    int                 dma_base_index;
    soc_mem_t           dma_mem;
    COUNTER_ATOMIC_DEF  s;
    int 		rv = SOC_E_NONE;
    int                 prime_ctr_write;
    uint32              val32;

    SOC_IF_ERROR_RETURN(_soc_counter_get_info(unit, port, ctr_reg,
                                              &port_index, &num_entries,
                                              NULL));
    if (ar_idx >= num_entries) {
        return SOC_E_PARAM;
    }

    if (ar_idx < 0) {
        index_min = 0;
        index_max = num_entries - 1;
    } else {
        index_min = index_max = ar_idx;
    }

    COUNTER_LOCK(unit);
    for (index = index_min; index <= index_max; index++) {
        if (ctr_reg >= NUM_SOC_REG) {
            non_dma = &soc->counter_non_dma[ctr_reg - SOC_COUNTER_NON_DMA_START];
            if (non_dma->mem != INVALIDm) {
                dma_mem = non_dma->dma_mem[0];
                dma_base_index = port_index - non_dma->base_index +
                    non_dma->dma_index_min[0];
                if (non_dma->dma_buf[1] != NULL &&
                    dma_base_index > non_dma->dma_index_max[0]) {
                    dma_mem = non_dma->dma_mem[1];
                    dma_base_index -= non_dma->dma_index_max[0] + 1;
                    dma_base_index += non_dma->dma_index_min[1];
                }
                rv = soc_mem_read(unit, dma_mem, MEM_BLOCK_ANY,
                        dma_base_index + index, entry);
                if (SOC_FAILURE(rv)) {
                    goto counter_set_done;
                }
                fval[0] = COMPILER_64_LO(val64);
                fval[1] = COMPILER_64_HI(val64);
                soc_mem_field_set(unit, dma_mem, entry,
                        non_dma->field, fval);
                rv = soc_mem_write(unit, dma_mem, MEM_BLOCK_ALL,
                        dma_base_index + index, entry);
                if (SOC_FAILURE(rv)) {
                    goto counter_set_done;
                }
            } else if (non_dma->reg != INVALIDr) {
                if (PBMP_MEMBER(non_dma->pbmp, port) &&
                    !(non_dma->flags & (_SOC_COUNTER_NON_DMA_PEAK |
                                        _SOC_COUNTER_NON_DMA_CURRENT))) {
                    uint64 regval64;
                    uint32 regval32;

                    rv = soc_reg_get(unit, non_dma->reg, 
                                     port, index, &regval64);
                    if (SOC_FAILURE(rv)) {
                        goto counter_set_done;
                    }

                    if (SOC_REG_IS_64(unit, non_dma->reg)) {
                        soc_reg64_field_set(unit, non_dma->reg, &regval64,
                                non_dma->field, val64);
                    } else {
                        regval32 = COMPILER_64_LO(regval64);
                        soc_reg_field_set(unit, non_dma->reg, &regval32,
                                non_dma->field, COMPILER_64_LO(val64));
                        COMPILER_64_SET(regval64, 0, regval32);
                    }

                    rv = soc_reg_set(unit, non_dma->reg, 
                                     port, index, regval64);
                    if (SOC_FAILURE(rv)) {
                        goto counter_set_done;
                    }
                }
            }
        } else {
            if (!(SOC_REG_INFO(unit, ctr_reg).flags & SOC_REG_FLAG_64_BITS) &&
                !SOC_IS_XGS3_SWITCH(unit)) {
                val32 = COMPILER_64_LO(val64);
                rv = soc_reg32_set(unit, ctr_reg, port, index, val32);
                if (SOC_FAILURE(rv)) {
                    goto counter_set_done;
                }
            } else {
                if (soc_feature(unit, soc_feature_prime_ctr_writes) &&
                    /*SOC_REG_INFO(unit, ctr_reg).block == SOC_BLK_EGR) {*/
                    SOC_BLOCK_IS(SOC_REG_INFO(unit, ctr_reg).block, SOC_BLK_EGR)) {
                    prime_ctr_write = TRUE;
                } else {
                    prime_ctr_write = FALSE;
                }

                if (prime_ctr_write) {
                    rv = soc_reg32_get(unit, ctr_reg, port, index, &val32);
                    if (SOC_FAILURE(rv)) {
                        goto counter_set_done;
                    }
                }
                rv = soc_reg_set(unit, ctr_reg, port, index, val64);
                if (SOC_FAILURE(rv)) {
                    goto counter_set_done;
                }
                if (prime_ctr_write) {
                    rv = soc_reg32_get(unit, ctr_reg, port, index, &val32);
                    if (SOC_FAILURE(rv)) {
                        goto counter_set_done;
                    }
                }
            }
        }

        /* The following section updates 64-bit values and must be atomic */
        COUNTER_ATOMIC_BEGIN(s);
        soc->counter_sw_val[port_index + index] = val64;
        soc->counter_hw_val[port_index + index] = val64;
        COMPILER_64_SET(soc->counter_delta[port_index + index], 0, 0);
        COUNTER_ATOMIC_END(s);
    }

 counter_set_done:
    COUNTER_UNLOCK(unit);
    return rv;
}

int
soc_counter_set(int unit, soc_port_t port, soc_reg_t ctr_reg,
                int ar_idx, uint64 val)
{
    return _soc_counter_set(unit, port, ctr_reg, ar_idx, val);
}

int
soc_counter_set32(int unit, soc_port_t port, soc_reg_t ctr_reg,
                  int ar_idx, uint32 val)
{
    uint64              val64;

    COMPILER_64_SET(val64, 0, val);

    return _soc_counter_set(unit, port, ctr_reg, ar_idx, val64);
}

/*
 * Function:
 *      soc_counter_set_by_port, soc_counter_set32_by_port
 * Purpose:
 *      Given a port bit map, set all counters for the specified ports to
 *      the requested value.  Useful mainly to clear all the counters.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      pbm  - Bit map of all ports to set counters on.
 *      val  - 64/32-bit counter value.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      The 32-bit version sets the upper 32-bits of the 64-bit
 *      software counters to zero.  The specified value is truncated
 *      to the size of the hardware register when storing to hardware.
 */

int
soc_counter_set_by_port(int unit, pbmp_t pbmp, uint64 val)
{
    int                 i;
    soc_port_t          port;
    soc_ctr_ref_t       *ctr_ref;

    PBMP_ITER(pbmp, port) {
        if (!IS_LB_PORT(unit, port)
#ifdef BCM_SHADOW_SUPPORT
            && !IS_IL_PORT(unit, port)
#endif
        ) {
            /* No DMA counters for the loopback port */
            for (i = 0; i < PORT_CTR_NUM(unit, port); i++) {
                ctr_ref = PORT_CTR_REG(unit, port, i);
                if (!SOC_COUNTER_INVALID(unit, ctr_ref->reg)) {
                    SOC_IF_ERROR_RETURN
                        (_soc_counter_set(unit, port, ctr_ref->reg,
                                          ctr_ref->index, val));
                }
            }
        }
        for (i = 0; i < SOC_COUNTER_NON_DMA_END - SOC_COUNTER_NON_DMA_START;
             i++) {
            _soc_counter_set(unit, port, SOC_COUNTER_NON_DMA_START + i, -1,
                             val);
        }
#ifdef BCM_BRADLEY_SUPPORT
        /*
         * The HOLD register is not supported by DMA on HUMV/Bradley, 
         * so we cannot use the standard counter_set function.
         */
        if (SOC_IS_HBX(unit)) {
            if (SOC_REG_IS_VALID(unit, HOLD_Xr) && 
                SOC_REG_IS_VALID(unit, HOLD_Yr)) {
            SOC_IF_ERROR_RETURN
                (WRITE_HOLD_Xr(unit, port, COMPILER_64_LO(val) / 2));
            SOC_IF_ERROR_RETURN
                (WRITE_HOLD_Yr(unit, port, COMPILER_64_LO(val) / 2));
            }
        }
#endif
    }

    return SOC_E_NONE;
}

int
soc_counter_set32_by_port(int unit, pbmp_t pbmp, uint32 val)
{
    uint64              val64;

    COMPILER_64_SET(val64, 0, val);

    return soc_counter_set_by_port(unit, pbmp, val64);
}

/*
 * Function:
 *      soc_counter_set_by_reg, soc_counter_set32_by_reg
 * Purpose:
 *      Given a counter register number and a counter value, set both
 *      the hardware and software-accumulated counters to the value
 *      for all ports.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      ctr_reg - counter register to set.
 *      val - 64/32-bit counter value.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      The 32-bit version sets the upper 32-bits of the 64-bit
 *      software counters to zero.  The specified value is truncated
 *      to the size of the hardware register when storing to hardware.
 */

int
soc_counter_set_by_reg(int unit, soc_reg_t ctr_reg, int ar_idx, uint64 val)
{
    soc_port_t          port;

    if (SOC_COUNTER_INVALID(unit, ctr_reg)) {
        return SOC_E_NONE;
    }

#ifdef BROADCOM_DEBUG
    if (!SOC_REG_IS_COUNTER(unit, ctr_reg)) {
        _soc_counter_illegal(unit, ctr_reg);
        return SOC_E_NONE;
    }
#endif /* BROADCOM_DEBUG */


    PBMP_PORT_ITER(unit, port) {
        SOC_IF_ERROR_RETURN(_soc_counter_set(unit, port, ctr_reg,
                                             ar_idx, val));
    }

    return SOC_E_NONE;
}

int
soc_counter_set32_by_reg(int unit, soc_reg_t ctr_reg, int ar_idx, uint32 val)
{
    uint64              val64;

    COMPILER_64_SET(val64, 0, val);

    return soc_counter_set_by_reg(unit, ctr_reg, ar_idx, val64);
}

/*
 * Function:
 *      soc_counter_collect32
 * Purpose:
 *      This routine gets called each time the counter transfer has
 *      completed a cycle.  It collects the 32-bit counters.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      discard - If TRUE, the software counters are not updated; this
 *              results in only synchronizing the previous hardware
 *              count buffer.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      Computes the deltas in the hardware counters from the last
 *      time it was called and adds them to the high resolution (64-bit)
 *      software counters in counter_sw_val[].  It takes wrapping into
 *      account for counters that are less than 32 bits wide.
 *      It also computes counter_delta[].
 *
 *      Collects FE+GE counters on StrataSwitches.
 *      Collects GE counters on Draco, but not IPIC.
 */

STATIC int
soc_counter_collect32(int unit, int discard)
{
    soc_control_t       *soc = SOC_CONTROL(unit);
    soc_port_t          port;
    soc_reg_t           ctr_reg;
    soc_ctr_ref_t       *ctr_ref;
    uint32              ctr_new, ctr_prev, ctr_diff;
    int                 index;
    int                 port_index;
    int                 dma;
    int                 recheck_cntrs;
    int                 ar_idx;

    soc_cm_debug(DK_COUNTER + DK_VERBOSE,
                 "soc_counter_collect32: unit=%d discard=%d\n",
                 unit, discard);

    dma = ((soc->counter_flags & SOC_COUNTER_F_DMA) &&
           !discard);

    recheck_cntrs = soc_feature(unit, soc_feature_recheck_cntrs);

    PBMP_ITER(soc->counter_pbmp, port) {
        if ((IS_HG_PORT(unit, port) || IS_XE_PORT(unit, port)) &&
            (!SOC_IS_XGS3_SWITCH(unit))) {
                continue;
        }
#if defined(BCM_SHADOW_SUPPORT)
        if (SOC_IS_SHADOW(unit) && IS_IL_PORT(unit, port)) {
                continue;
        }
#endif /* BCM_SHADOW_SUPPORT */


        port_index = COUNTER_MIN_IDX_GET(unit, port);

        for (index = 0; index < PORT_CTR_NUM(unit, port);
             index++, port_index++) {
            volatile uint64 *vptr;
            COUNTER_ATOMIC_DEF s;

            ctr_ref = PORT_CTR_REG(unit, port, index);
            ctr_reg = ctr_ref->reg;
            ar_idx = ctr_ref->index;

            if (SOC_COUNTER_INVALID(unit, ctr_reg)) {
                continue;
            }

            ctr_prev = COMPILER_64_LO(soc->counter_hw_val[port_index]);

            if (dma) {
                ctr_new = soc->counter_buf32[port_index];
            } else {                   /* Not DMA.  Just read register */
                SOC_IF_ERROR_RETURN
                    (soc_reg32_get(unit, ctr_reg, port, ar_idx,
                                   &ctr_new));
            }

            if ( (recheck_cntrs == TRUE) && (ctr_new != ctr_prev) ) {
                /* Seeds of doubt, double-check */
                uint32 ctr_new2;
                int suspicious = 0;
                SOC_IF_ERROR_RETURN
                  (soc_reg32_get(unit, ctr_reg, port, 0, &ctr_new2));

                /* Check for partial ordering, with wrap */
                if (ctr_new < ctr_prev) {
                    if ((ctr_new2 < ctr_new) || (ctr_new2 > ctr_prev)) {
                        /* prev < new2 < new, bad data */
                        /* Try again next time */
                        ctr_new = ctr_prev;
                        suspicious = 1;
                    }
                } else {
                    if ((ctr_new2 < ctr_new) && (ctr_new2 > ctr_prev)) {
                        /* prev < new2 < new, bad data */
                        /* Try again next time */
                        ctr_new = ctr_prev;
                        suspicious = 1;
                    }
                }
                /* Otherwise believe ctr_new */

                if (suspicious) {
#if !defined(SOC_NO_NAMES)
                    soc_cm_debug(DK_COUNTER, "soc_counter_collect32: "
                                 "unit %d, port%d: suspicious %s "
                                 "counter read (%s)\n",
                                 unit, port, 
                                 dma?"DMA":"manual",
                                 SOC_REG_NAME(unit, ctr_reg));
#else
                    soc_cm_debug(DK_COUNTER, "soc_counter_collect32: "
                                 "unit %d, port%d: suspicious %s "
                                 "counter read (Counter%d)\n",
                                 unit, port, 
                                 dma?"DMA":"manual",
                                 ctr_reg);
#endif /* SOC_NO_NAMES */
                } /* Suspicious counter change */
            } /* recheck_cntrs == TRUE */

            if (ctr_new == ctr_prev) {
                COUNTER_ATOMIC_BEGIN(s);
                COMPILER_64_SET(soc->counter_delta[port_index], 0, 0);
                COUNTER_ATOMIC_END(s);
                continue;
            }

            if (discard) {
                COUNTER_ATOMIC_BEGIN(s);
                /* Update the DMA location */
                soc->counter_buf32[port_index] = ctr_new;
                /* Update the previous value buffer */
                COMPILER_64_SET(soc->counter_hw_val[port_index], 0, ctr_new);
                COMPILER_64_SET(soc->counter_delta[port_index], 0, 0);
                COUNTER_ATOMIC_END(s);
                continue;
            }

            soc_cm_debug(DK_COUNTER,
                         "soc_counter_collect32: ctr %d => %u\n",
                         port_index, ctr_new);

            vptr = &soc->counter_sw_val[port_index];

            ctr_diff = ctr_new;

            if (ctr_diff < ctr_prev) {
                int             width, i = 0;

                /*
                 * Counter must have wrapped around.  Add the proper
                 * wrap-around amount.  Full 32-bit wide counters do not
                 * need anything added.
                 */
                width = SOC_REG_INFO(unit, ctr_reg).fields[0].len;
                if (soc_feature(unit, soc_feature_counter_parity)) {
                    while((SOC_REG_INFO(unit, ctr_reg).fields + i) != NULL) { 
                        if (SOC_REG_INFO(unit, ctr_reg).fields[i].field == 
                            COUNTf) {
                            width = SOC_REG_INFO(unit, ctr_reg).fields[i].len;
                            break;
                        }
                        i++;
                    }
                }
                if (width < 32) {
                    ctr_diff += (1UL << width);
                }
            }

            ctr_diff -= ctr_prev;

            COUNTER_ATOMIC_BEGIN(s);
            COMPILER_64_ADD_32(*vptr, ctr_diff);
            COMPILER_64_SET(soc->counter_delta[port_index], 0, ctr_diff);
            COMPILER_64_SET(soc->counter_hw_val[port_index], 0, ctr_new);
            COUNTER_ATOMIC_END(s);

        }

        /* If signaled to exit then return  */
        if (!soc->counter_interval) {
            return SOC_E_NONE;
        }
        
        /*
         * Allow other tasks to run between processing each port.
         */

        sal_thread_yield();
    }

    return SOC_E_NONE;
}

#ifdef BCM_XGS_SUPPORT

#if defined(BCM_BRADLEY_SUPPORT) || defined(BCM_TRIDENT_SUPPORT)
/*
 * Function:
 *      _soc_xgs3_update_link_activity
 * Purpose:
 *      Report the link activities to LED processor. 
 * Parameters:
 *      unit     - switch unit
 *      port     - port number
 *      tx_byte  - transmitted byte during counter DMA interval
 *      rx_byte  - received byte during counter DMA interval
 *
 * Returns:
 *      SOC_E_*
 */
static int
_soc_xgs3_update_link_activity(int unit, soc_port_t port, 
                               int tx_byte, int rx_byte)
{
    soc_control_t       *soc = SOC_CONTROL(unit);
    int                  port_speed, interval, act_byte, byte;
    uint32               portdata;

    SOC_IF_ERROR_RETURN(soc_mac_speed_get(unit, port, &port_speed));
    portdata = 0;
    interval = soc->counter_interval;

    if (tx_byte < rx_byte) {
        act_byte = rx_byte;
    } else {
        act_byte = tx_byte;
    }

    if (act_byte > (((port_speed / 8) * interval) / 2)) {
        /* transmitting or receiving more than 50% */
        portdata |= 0x2;
    } else if (act_byte > (((port_speed / 8) * interval) / 4)) {
        /* transmitting or receiving more than 25% */
        portdata |= 0x4;
    } else if (act_byte > 0) { 
        /* some transmitting or receiving activity on the link */
        portdata |= 0x8;
    }

    if (tx_byte) {
        /* Indicate TX activity */
        portdata |= 0x10;
    }
    if (rx_byte) {
        /* Indicate RX activity */
        portdata |= 0x20;
    }

#ifdef BCM_TRIDENT_SUPPORT
#define TRIDENT_LEDUP0_PORT_MAX        36

    if (SOC_IS_TD_TT(unit)) {
        soc_info_t *si;
        int phy_port;  /* physical port number */
        uint32 dram_base;
        uint32 prev_data;

        si = &SOC_INFO(unit);
        phy_port = si->port_l2p_mapping[port];

        /* trident first 36 ports in ledproc0, the other 36 ports in ledproc1*/
        if (phy_port > TRIDENT_LEDUP0_PORT_MAX) {
            phy_port -= TRIDENT_LEDUP0_PORT_MAX;
            dram_base = CMICE_LEDUP1_DATA_RAM_BASE;
        } else {
            dram_base = CMICE_LEDUP0_DATA_RAM_BASE;
        }

        byte = LS_LED_DATA_OFFSET + phy_port;

        prev_data = soc_pci_read(unit, dram_base + CMIC_LED_REG_SIZE * byte);
        prev_data &= ~0x3e;
        portdata |= prev_data;
        soc_pci_write(unit, dram_base + CMIC_LED_REG_SIZE * byte, portdata);

    } else
#endif
    {
    /* 20 bytes from LS_LED_DATA_OFFSET is used by linkscan callback */
    byte = LS_LED_DATA_OFFSET + 20 + port;
    soc_pci_write(unit, CMIC_LED_DATA_RAM(byte), portdata);
    }
    return SOC_E_NONE;
}
#endif

/*
 * Function:
 *      soc_counter_collect64
 * Purpose:
 *      This routine gets called each time the counter transfer has
 *      completed a cycle.  It collects the 64-bit counters.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      discard - If true, the software counters are not updated; this
 *              results in only synchronizing the previous hardware
 *              count buffer.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      It computes the deltas in the hardware counters from the last
 *      time it was called and adds them to the high resolution (64-bit)
 *      software counters in counter_sw_val[].  It takes wrapping into
 *      account for counters that are less than 64 bits wide.
 *      It also computes counter_delta[].
 *
 *      Collects IPIC counters on Draco.
 *              Currently does not use DMA for IPIC counters since the
 *              hardware does not transfer all of them (offsets >= 0x60).
 *      Collects IPIC counters on Draco 1.5, using DMA
 *      Collects all counters on Hercules, Lynx, Tucana
 */

STATIC int
soc_counter_collect64(int unit, int discard)
{
    soc_control_t       *soc = SOC_CONTROL(unit);
    soc_port_t          port;
    soc_reg_t           ctr_reg;
    uint64              ctr_new, ctr_prev, ctr_diff;
    int                 index;
    int                 port_base, port_base_dma;
    int                 dma;
    int                 recheck_cntrs, prime_cntrs;
    int                 ar_idx;
#if defined(BCM_BRADLEY_SUPPORT) || defined(BCM_TRX_SUPPORT)
    int                 pio_hold, pio_hold_enable;
    uint64              *counter_tmp = NULL;
    int                 tx_byt_to_led = 0;
    int                 rx_byt_to_led = 0;
    int                 rxcnt_xaui = 0;
    int                 skip_xaui_rx_counters;
    soc_reg_t           reg_1a = INVALIDr;
    soc_reg_t           reg_1b = INVALIDr;
    soc_reg_t           reg_2a = INVALIDr;
    soc_reg_t           reg_2b = INVALIDr;
#endif /* BCM_BRADLEY_SUPPORT || BCM_TRX_SUPPORT */

    soc_cm_debug(DK_COUNTER + DK_VERBOSE,
                 "soc_counter_collect64: unit=%d discard=%d\n",
                 unit, discard);

    dma = ((soc->counter_flags & SOC_COUNTER_F_DMA) &&
           !SOC_IS_DRACO(unit) &&
           !discard);
    recheck_cntrs = soc_feature(unit, soc_feature_recheck_cntrs);
    prime_cntrs = soc_feature(unit, soc_feature_prime_ctr_writes);

#ifdef BCM_BRADLEY_SUPPORT
    pio_hold = SOC_IS_HBX(unit);
    pio_hold_enable = (soc->counter_flags & SOC_COUNTER_F_HOLD);

    if (SOC_IS_HBX(unit) && soc_feature(unit, soc_feature_bigmac_rxcnt_bug)) {
        
        rxcnt_xaui = 1;
        reg_1a = MAC_TXPSETHRr;
        reg_1b = IRFCSr;
        reg_2a = MAC_TXMAXSZr;
        reg_2b = IROVRr;
        if (dma) {
            counter_tmp = soc_counter_tbuf[unit];
        }
    }
#endif /* BCM_BRADLEY_SUPPORT */

#ifdef BCM_TRX_SUPPORT
    if (SOC_IS_TRX(unit) && soc_feature(unit, soc_feature_bigmac_rxcnt_bug)) {
        
        rxcnt_xaui = 1;
        reg_1a = MAC_TXPSETHRr;
#ifdef BCM_ENDURO_SUPPORT
        if(SOC_IS_ENDURO(unit) || SOC_IS_HURRICANE(unit)) {
            reg_1b = IRUCAr;
        } else
#endif /* BCM_ENDURO_SUPPORT */
        {
            reg_1b = IRUCr;
        }
        reg_2a = MAC_TXMAXSZr;
        reg_2b = IRFCSr;
        if (dma) {
            counter_tmp = soc_counter_tbuf[unit];
        }
    }
#endif /* BCM_TRX_SUPPORT */

    PBMP_ITER(soc->counter_pbmp, port) {
        if (!SOC_IS_XGS3_SWITCH(unit) && !SOC_IS_TUCANA(unit) &&
            !IS_HG_PORT(unit, port) && !IS_XE_PORT(unit, port)) {
                continue;
        }

        port_base = COUNTER_MIN_IDX_GET(unit, port);
        port_base_dma = COUNTER_MIN_IDX_GET(unit, port - soc->counter_ports32);

#if defined(BCM_BRADLEY_SUPPORT) || defined(BCM_TRX_SUPPORT)
        /*
         * Handle XAUI Rx counter bug.
         * 
         * If the IRFCS counter returns the contents of the
         * MAC_TXPSETHR register, and the IROVR counter returns the
         * contents of the MAC_TXMAXSZ register, we assume that the
         * counters are bogus.
         *
         * By using two counters we significantly reduce the chance of
         * getting stuck if the real counter value matches the bogus
         * reference.
         *
         * To protect ourselves from the error condition happening
         * while the counter DMA is in progress, we manually check the
         * counters before and after we copy the DMA buffer contents.
         */
        skip_xaui_rx_counters = 1;
        if (rxcnt_xaui &&
            (IS_GX_PORT(unit, port) ||
             (IS_XG_PORT(unit, port) &&
              (IS_XE_PORT(unit, port) || IS_HG_PORT(unit, port))))) {
            uint64 val_1a, val_1b, val_2a, val_2b;
            uint32 adr_1a, adr_1b, adr_2a, adr_2b;
            uint32 *p1, *p2;
            int rxcnt_appear_valid = 0;

            adr_1a = soc_reg_addr(unit, reg_1a, port, 0);
            adr_1b = soc_reg_addr(unit, reg_1b, port, 0);
            adr_2a = soc_reg_addr(unit, reg_2a, port, 0);
            adr_2b = soc_reg_addr(unit, reg_2b, port, 0);

            /* Read reference values values and counter values */
            if (soc_reg_get(unit, reg_1a, port, 0, &val_1a) == 0 &&
                soc_reg_get(unit, reg_1b, port, 0, &val_1b) == 0 &&
                soc_reg_get(unit, reg_2a, port, 0, &val_2a) == 0 &&
                soc_reg_get(unit, reg_2b, port, 0, &val_2b) == 0) {

                /* Check for bogus values */
                if (COMPILER_64_NE(val_1a, val_1b) || 
                    COMPILER_64_NE(val_2a, val_2b)) {
                    rxcnt_appear_valid = 1;
                }
                if (rxcnt_appear_valid && counter_tmp) {
                    /* Copy DMA buffer */
                    sal_memcpy(counter_tmp, &soc->counter_buf64[port_base_dma], 
                               SOC_COUNTER_TBUF_SIZE(unit));
                    p1 = (uint32 *)&counter_tmp[SOC_REG_CTR_IDX(unit, reg_1b)];
                    p2 = (uint32 *)&counter_tmp[SOC_REG_CTR_IDX(unit, reg_2b)];
                    /* Read counter values again */
                    if (soc_reg_get(unit, reg_1b, port, 0, &val_1b) == 0 &&
                        soc_reg_get(unit, reg_2b, port, 0, &val_2b) == 0) {
                        /* Check for bogus value again, including DMA buffer */
                        if ((COMPILER_64_NE(val_1a, val_1b) || 
                             COMPILER_64_NE(val_2a, val_2b)) && 
                            (p1[0] != COMPILER_64_LO(val_1a) ||
                             p2[0] != COMPILER_64_LO(val_2a))) {
                            /* Counters appears to be valid */
                            skip_xaui_rx_counters = 0;
                        }
                    }
                } else if (rxcnt_appear_valid && !dma) {
                    /* If not DMA we take the chance and read the counters */
                    skip_xaui_rx_counters = 0;
                }
            }
        }
#endif /* BCM_BRADLEY_SUPPORT || BCM_TRX_SUPPORT */

        for (index = 0; index < PORT_CTR_NUM(unit, port); index++) {
            volatile uint64 *vptr;
            COUNTER_ATOMIC_DEF s;

            ctr_reg = PORT_CTR_REG(unit, port, index)->reg;
            ar_idx = PORT_CTR_REG(unit, port, index)->index;

            if (SOC_COUNTER_INVALID(unit, ctr_reg)) {
                continue;
            }

#ifdef BCM_TRIUMPH2_SUPPORT
            /* Trap for BigMAC TX Timestamp FIFO reads
             *
             * MAC_TXTIMESTAMPFIFOREADr is immediately after
             * MAC_RXLLFCMSGCNTr.  It is a single register exposing
             * a HW FIFO which pops on read.  And it is in the counter
             * DMA range.  So we detect MAC_RXLLFCMSGCNTr and record
             * the value of the next piece of data in a SW FIFO for
             * use by the API.
             */
            if (soc_feature(unit, soc_feature_timestamp_counter) &&
                dma && (ctr_reg == MAC_RXLLFCMSGCNTr) &&
                (IS_XE_PORT(unit, port) || IS_HG_PORT(unit, port)) &&
                (soc->counter_timestamp_fifo[port] != NULL) &&
                !SHR_FIFO_IS_FULL(soc->counter_timestamp_fifo[port])) {
                uint32 *ptr =
                    (uint32 *)&soc->counter_buf64[port_base_dma + index + 1];

                if (ptr[0] != 0) { /* Else, HW FIFO empty */
                    SHR_FIFO_PUSH(soc->counter_timestamp_fifo[port], &(ptr[0]));
                }
            }
#endif /* BCM_TRIUMPH2_SUPPORT */

            /* Atomic because soc_counter_set may update 64-bit value */
            COUNTER_ATOMIC_BEGIN(s);
            ctr_prev = soc->counter_hw_val[port_base + index];
            COUNTER_ATOMIC_END(s);

#if defined(BCM_BRADLEY_SUPPORT) || defined(BCM_TRX_SUPPORT)
            if (pio_hold && ctr_reg == HOLDr) {
                if (pio_hold_enable) {
                    uint64 ctr_prev_x, ctr_prev_y;
                    uint32 ctr_x, ctr_y;
                    int idx_x, idx_y;
                    /*
                     * We need to account for the two values of HOLD
                     * separately to get correct rollover behavior.
                     * Use holes in counter space to store HOLD_X/Y values
                     * HOLD_X at RDBGC4r, hole, IUNHGIr
                     * HOLD_Y at IUNKOPCr, hole, RDBGC5r
                     */

                    idx_x = SOC_REG_CTR_IDX(unit, RDBGC4r) + 1;
                    idx_y = SOC_REG_CTR_IDX(unit, IUNKOPCr) + 1;

                    COUNTER_ATOMIC_BEGIN(s);
                    ctr_prev_x = soc->counter_hw_val[port_base + idx_x];
                    ctr_prev_y = soc->counter_hw_val[port_base + idx_y];
                    COUNTER_ATOMIC_END(s);

                    SOC_IF_ERROR_RETURN
                        (soc_reg32_get(unit, HOLD_Xr, port, ar_idx,
                                       &ctr_x));
                    SOC_IF_ERROR_RETURN
                        (soc_reg32_get(unit, HOLD_Yr, port, ar_idx,
                                       &ctr_y));

                    if (discard) {
                        /* Just save the new sum to be used below */
                        COMPILER_64_SET(ctr_new, 0, ctr_x + ctr_y);
                    } else {
                        int             width = 0;
                        uint64          wrap_amt;

                        COMPILER_64_SET(ctr_diff, 0, ctr_x);
                        /* We know the width of HOLDr is < 32 */
                        if (COMPILER_64_LT(ctr_diff, ctr_prev_x)) {
                            width =
                                SOC_REG_INFO(unit, ctr_reg).fields[0].len;
                            COMPILER_64_SET(wrap_amt, 0, 1UL << width);
                            COMPILER_64_ADD_64(ctr_diff, wrap_amt);
                        }
                        /* Calculate HOLD_X diff from previous */
                        COMPILER_64_SUB_64(ctr_diff, ctr_prev_x);
                        ctr_new = ctr_diff;

                        COMPILER_64_SET(ctr_diff, 0, ctr_y);
                        if (COMPILER_64_LT(ctr_diff, ctr_prev_y)) {
                            width =
                                SOC_REG_INFO(unit, ctr_reg).fields[0].len;
                            COMPILER_64_SET(wrap_amt, 0, 1UL << width);
                            COMPILER_64_ADD_64(ctr_diff, wrap_amt);
                        }
                        /* Calculate HOLD_Y diff from previous */
                        COMPILER_64_SUB_64(ctr_diff, ctr_prev_y);

                        /* Combine diffs */
                        COMPILER_64_ADD_64(ctr_new, ctr_diff);
                        /* Add previous value so logic below handles
                         * all of the other updates */
                        COMPILER_64_ADD_64(ctr_new, ctr_prev);
                    }
                        
                    /* Update previous values with new values.
                     * Since these are INVALID registers, the other
                     * counter logic will not touch them. */
                    COMPILER_64_SET(soc->counter_hw_val[port_base + idx_x], 0, ctr_x);
                    COMPILER_64_SET(soc->counter_hw_val[port_base + idx_y], 0, ctr_y);
                } else {
                    /* 
                     * The counter collected by DMA is not reliable 
                     * due to incorrect access type, so force zero.
                     */
                    COMPILER_64_ZERO(ctr_new);
                }
            } else if (IS_HYPLITE_PORT(unit, port) && is_xaui_rx_counter(ctr_reg)) {
                SOC_IF_ERROR_RETURN
                    (soc_reg_read(unit, ctr_reg,
                                    soc_reg_addr(unit, ctr_reg, port, ar_idx),
                                    &ctr_new));
            } else if (rxcnt_xaui && is_xaui_rx_counter(ctr_reg)) {
                if (skip_xaui_rx_counters) {
                    ctr_new = ctr_prev;
                } else if (counter_tmp) {
                    uint32 *ptr = (uint32 *)&counter_tmp[index];
                    /* No need to check for SWAP64 flag */
                    COMPILER_64_SET(ctr_new, ptr[1], ptr[0]);
                } else {
                    SOC_IF_ERROR_RETURN
                        (soc_reg_get(unit, ctr_reg, port, ar_idx,
                                     &ctr_new));
                }
            } else 
#endif /* BCM_BRADLEY_SUPPORT || BCM_TRX_SUPPORT */
            if (dma) {
                uint32 *ptr =
                    (uint32 *)&soc->counter_buf64[port_base_dma + index];
                if (soc->counter_flags & SOC_COUNTER_F_SWAP64) {
                    COMPILER_64_SET(ctr_new, ptr[0], ptr[1]);
                } else {
                    COMPILER_64_SET(ctr_new, ptr[1], ptr[0]);
                }
            } else {
                SOC_IF_ERROR_RETURN
                    (soc_reg_get(unit, ctr_reg, port, ar_idx,
                                 &ctr_new));

            }
            if (soc_feature(unit, soc_feature_counter_parity)) {
                uint32 temp;
                if (soc_reg_field_valid(unit, ctr_reg, EVEN_PARITYf) || 
                    soc_reg_field_valid(unit, ctr_reg, PARITYf)) {
                    temp = COMPILER_64_LO(ctr_new) & ((1 << soc_reg_field_length
                           (unit, ctr_reg, COUNTf)) - 1);
                    COMPILER_64_SET(ctr_new, COMPILER_64_HI(ctr_new), temp);
                }
            }

            if ( (recheck_cntrs == TRUE) && 
                 COMPILER_64_NE(ctr_new, ctr_prev) ) {
                /* Seeds of doubt, double-check */
                uint64 ctr_new2;
                int suspicious = 0;
                SOC_IF_ERROR_RETURN
                  (soc_reg_get(unit, ctr_reg, port, 0,
                                &ctr_new2));
                
                /* Check for partial ordering, with wrap */
                if (COMPILER_64_LT(ctr_new, ctr_prev)) {
                    if (COMPILER_64_LT(ctr_new2, ctr_new) || 
                        COMPILER_64_GE(ctr_new2, ctr_prev)) {
                        /* prev < new2 < new, bad data */
                        /* Try again next time */
                        ctr_new = ctr_prev;
                        suspicious = 1;
                    }
                } else {
                    if (COMPILER_64_LT(ctr_new2, ctr_new) && 
                        COMPILER_64_GE(ctr_new2, ctr_prev)) {
                        /* prev < new2 < new, bad data */
                        /* Try again next time */
                        ctr_new = ctr_prev;
                        suspicious = 1;
                    }
                }
                /* Otherwise believe ctr_new */

                if (suspicious) {
#if !defined(SOC_NO_NAMES)
                    soc_cm_debug(DK_COUNTER, "soc_counter_collect64: "
                                 "unit %d, port%d: suspicious %s "
                                 "counter read (%s)\n",
                                 unit, port,
                                 dma?"DMA":"manual",
                                 SOC_REG_NAME(unit, ctr_reg));
#else
                    soc_cm_debug(DK_COUNTER, "soc_counter_collect64: "
                                 "unit %d, port%d: suspicious %s "
                                 "counter read (Counter%d)\n",
                                 unit, port,
                                 dma?"DMA":"manual",
                                 ctr_reg);
#endif /* SOC_NO_NAMES */
                } /* Suspicious counter change */
            } /* recheck_cntrs == TRUE */

#ifdef BCM_EASYRIDER_SUPPORT
            if (prime_cntrs && IS_HG_PORT(unit, port)) {
                /* Get counter in trustworthy method */
                SOC_IF_ERROR_RETURN
                    (soc_reg_get(unit, ctr_reg, port, 0,
                                  &ctr_new));
                SOC_IF_ERROR_RETURN
                    (soc_reg_get(unit, ctr_reg, port, 0,
                                  &ctr_new));
               /* Now ctr_new is reliable */
            }
#endif

            if (COMPILER_64_EQ(ctr_new, ctr_prev)) {
                COUNTER_ATOMIC_BEGIN(s);
                COMPILER_64_ZERO(soc->counter_delta[port_base + index]);
                COUNTER_ATOMIC_END(s);
                continue;
            }

            if (discard) {
                uint32 *ptr =
                    (uint32 *)&soc->counter_buf64[port_base_dma + index];
                COUNTER_ATOMIC_BEGIN(s);
                /* Update the DMA location */
                if (soc->counter_flags & SOC_COUNTER_F_SWAP64) {
                    COMPILER_64_TO_32_HI(ptr[0], ctr_new);
                    COMPILER_64_TO_32_LO(ptr[1], ctr_new);
                } else {
                    COMPILER_64_TO_32_LO(ptr[0], ctr_new);
                    COMPILER_64_TO_32_HI(ptr[1], ctr_new);
                }
                /* Update the previous value buffer */
                soc->counter_hw_val[port_base + index] = ctr_new;
                COMPILER_64_ZERO(soc->counter_delta[port_base + index]);
                COUNTER_ATOMIC_END(s);
                continue;
            }

            soc_cm_debug(DK_COUNTER,
                         "soc_counter_collect64: ctr %d => 0x%08x_%08x\n",
                         port_base + index,
                         COMPILER_64_HI(ctr_new), COMPILER_64_LO(ctr_new));

            vptr = &soc->counter_sw_val[port_base + index];

            ctr_diff = ctr_new;

            if (COMPILER_64_LT(ctr_diff, ctr_prev)) {
                int             width, i = 0;
                uint64          wrap_amt;

                /*
                 * Counter must have wrapped around.
                 * Add the proper wrap-around amount.
                 */
                width = SOC_REG_INFO(unit, ctr_reg).fields[0].len;
                if (soc_feature(unit, soc_feature_counter_parity)) {
                    while((SOC_REG_INFO(unit, ctr_reg).fields + i) != NULL) { 
                        if (SOC_REG_INFO(unit, ctr_reg).fields[i].field == 
                            COUNTf) {
                            width = SOC_REG_INFO(unit, ctr_reg).fields[i].len;
                            break;
                        }
                        i++;
                    }
                }
                if (width < 32) {
                    COMPILER_64_SET(wrap_amt, 0, 1UL << width);
                    COMPILER_64_ADD_64(ctr_diff, wrap_amt);
                } else if (width < 64) {
                    COMPILER_64_SET(wrap_amt, 1UL << (width - 32), 0);
                    COMPILER_64_ADD_64(ctr_diff, wrap_amt);
                }
            }

            COMPILER_64_SUB_64(ctr_diff, ctr_prev);

            COUNTER_ATOMIC_BEGIN(s);
            COMPILER_64_ADD_64(*vptr, ctr_diff);
            soc->counter_delta[port_base + index] = ctr_diff;
            soc->counter_hw_val[port_base + index] = ctr_new;
            COUNTER_ATOMIC_END(s);

#if defined(BCM_BRADLEY_SUPPORT) || defined(BCM_TRIDENT_SUPPORT)
            if (soc_feature(unit, soc_feature_ctr_xaui_activity)) {
#ifdef BCM_TRIDENT_SUPPORT
                if (SOC_IS_TD_TT(unit)) {
                    if (ctr_reg == RPKTr) {
                        COMPILER_64_TO_32_LO(rx_byt_to_led, ctr_diff); 
                    } else if (ctr_reg == TPKTr) {
                        COMPILER_64_TO_32_LO(tx_byt_to_led, ctr_diff); 
                    }
                } else
#endif
                {
#ifdef BCM_BRADLEY_SUPPORT
                    if (ctr_reg == IRBYTr) {
                        COMPILER_64_TO_32_LO(rx_byt_to_led, ctr_diff); 
                    } else if (ctr_reg == ITBYTr) {
                        COMPILER_64_TO_32_LO(tx_byt_to_led, ctr_diff); 
                    }
#endif
                }
            }
#endif
        }

#if defined(BCM_BRADLEY_SUPPORT) || defined(BCM_TRIDENT_SUPPORT)
        if (soc_feature(unit, soc_feature_ctr_xaui_activity)) {
            _soc_xgs3_update_link_activity(unit, port, tx_byt_to_led,
                                           rx_byt_to_led);
            tx_byt_to_led = 0;
            rx_byt_to_led = 0;
        }
#endif
        /* If signaled to exit then return  */
        if (!soc->counter_interval) {
            return SOC_E_NONE;
        }

        /*
         * Allow other tasks to run between processing each port.
         */

        sal_thread_yield();
    }

    return SOC_E_NONE;
}
#endif /* BCM_XGS_SUPPORT */

STATIC void
soc_counter_collect_non_dma_entries(int unit)
{
    soc_control_t *soc;
    soc_counter_non_dma_t *non_dma;
    int base_index, id, width, i;
    int port;
    uint32 fval[2];
    uint64 ctr_new, ctr_prev, ctr_diff, wrap_amt;
    int rv, count, entry_words;
    COUNTER_ATOMIC_DEF s;

    soc = SOC_CONTROL(unit);

    if (soc->counter_non_dma == NULL) {
        return;
    }

    for (id = 0; id < SOC_COUNTER_NON_DMA_END - SOC_COUNTER_NON_DMA_START;
         id++) {
        non_dma = &soc->counter_non_dma[id];

        if (!(non_dma->flags & _SOC_COUNTER_NON_DMA_VALID)) {
            continue;
        }

        if (non_dma->flags & _SOC_COUNTER_NON_DMA_CURRENT) {
            continue;
        }

        /* If signaled to exit then return  */
        if (!soc->counter_interval) {
            return;
        }

        width = entry_words = 0;

        if (non_dma->mem != INVALIDm) {
            entry_words = soc_mem_entry_words(unit, non_dma->mem);
            if (non_dma->field != INVALIDf) {
                width = soc_mem_field_length(unit, non_dma->mem,
                                             non_dma->field);
            } else {
                width = 32;
            }

            for (i = 0; i < 2; i++) {
                if (non_dma->flags & _SOC_COUNTER_NON_DMA_DO_DMA) {
                    if (non_dma->dma_buf[i] == NULL) {
                        continue;
                    }
                    rv = soc_mem_read_range(unit, non_dma->dma_mem[i],
                                            MEM_BLOCK_ANY,
                                            non_dma->dma_index_min[i],
                                            non_dma->dma_index_max[i],
                                            non_dma->dma_buf[i]);
                    if (SOC_FAILURE(rv)) {
                        return;
                    }
                }
            }

            base_index = non_dma->base_index;
            for (i = 0; i < non_dma->num_entries; i++) {
                soc_mem_field_get(unit, non_dma->mem,
                                  &non_dma->dma_buf[0][entry_words * i],
                                  non_dma->field, fval);
                if (width <= 32) {
                    COMPILER_64_SET(ctr_new, 0, fval[0]);
                } else {
                    COMPILER_64_SET(ctr_new, fval[1], fval[0]);
                }

                COUNTER_ATOMIC_BEGIN(s);
                ctr_prev = soc->counter_hw_val[base_index + i];
                COUNTER_ATOMIC_END(s);

                if (COMPILER_64_EQ(ctr_new, ctr_prev)) {
                    COUNTER_ATOMIC_BEGIN(s);
                    COMPILER_64_ZERO(soc->counter_delta[base_index + i]);
                    COUNTER_ATOMIC_END(s);
                    continue;
                }

                ctr_diff = ctr_new;

                if (COMPILER_64_LT(ctr_diff, ctr_prev)) {
                    if (width < 32) {
                        COMPILER_64_ADD_32(ctr_diff, 1 << width);
                    } else if (width < 64) {
                        COMPILER_64_SET(wrap_amt, 1 << (width - 32), 0);
                        COMPILER_64_ADD_64(ctr_diff, wrap_amt);
                    }
                }

                COMPILER_64_SUB_64(ctr_diff, ctr_prev);

                COUNTER_ATOMIC_BEGIN(s);
                COMPILER_64_ADD_64(soc->counter_sw_val[base_index + i],
                                   ctr_diff);
                soc->counter_hw_val[base_index + i] = ctr_new;
                soc->counter_delta[base_index + i] = ctr_diff;
                COUNTER_ATOMIC_END(s);
            }
        } else if (non_dma->reg != INVALIDr) {
            if (non_dma->field != INVALIDf) {
                width = soc_reg_field_length(unit, non_dma->reg, 
                                             non_dma->field);
            } else {
                width = 32;
            }

            PBMP_ITER(soc->counter_pbmp, port) {
                if (non_dma->entries_per_port == 0) {
                    port = REG_PORT_ANY;
                    count = non_dma->num_entries;
                    base_index = non_dma->base_index;
                } else {
                    if (!SOC_PBMP_MEMBER(non_dma->pbmp, port)) {
                        continue;
                    }

                    if (non_dma->flags & _SOC_COUNTER_NON_DMA_PERQ_REG) {
                        count = num_cosq[unit][port];
                        base_index = non_dma->base_index;
                        for (i = 0; i < port; i++) {
                            base_index += num_cosq[unit][i];
                        }
                    } else {
                        count = non_dma->entries_per_port;
                        base_index = non_dma->base_index + port * count;
                    }
                }

                for (i = 0; i < count; i++) {
                    rv = soc_reg_get(unit, non_dma->reg, port, i, &ctr_new);
                    if (SOC_FAILURE(rv)) {
                        return;
                    }
                    COUNTER_ATOMIC_BEGIN(s);
                    ctr_prev = soc->counter_sw_val[base_index + i];
                    COUNTER_ATOMIC_END(s);

                    if (non_dma->flags & _SOC_COUNTER_NON_DMA_PEAK) {
                        COUNTER_ATOMIC_BEGIN(s);
                        if (COMPILER_64_GT(ctr_new, ctr_prev)) {
                            soc->counter_sw_val[base_index + i] = ctr_new;
                        }
                        soc->counter_hw_val[base_index + i] = ctr_new;
                        COMPILER_64_ZERO(soc->counter_delta[base_index + i]);
                        COUNTER_ATOMIC_END(s);
                        continue;
                    }

                    if (COMPILER_64_EQ(ctr_new, ctr_prev)) {
                        COUNTER_ATOMIC_BEGIN(s);
                        COMPILER_64_ZERO(soc->counter_delta[base_index + i]);
                        COUNTER_ATOMIC_END(s);
                        continue;
                    }

                    ctr_diff = ctr_new;

                    if (COMPILER_64_LT(ctr_diff, ctr_prev)) {
                        if (width < 32) {
                            COMPILER_64_ADD_32(ctr_diff, 1 << width);
                        } else if (width < 64) {
                            COMPILER_64_SET(wrap_amt, 1 << (width - 32), 0);
                            COMPILER_64_ADD_64(ctr_diff, wrap_amt);
                        }
                    }

                    COMPILER_64_SUB_64(ctr_diff, ctr_prev);

                    COUNTER_ATOMIC_BEGIN(s);
                    COMPILER_64_ADD_64(soc->counter_sw_val[base_index + i],
                                       ctr_diff);
                    soc->counter_hw_val[base_index + i] = ctr_new;
                    soc->counter_delta[base_index + i] = ctr_diff;
                    COUNTER_ATOMIC_END(s);
                }
                if (non_dma->entries_per_port == 0) {
                    break;
                }
            }
        } else {
            continue;
        }
    }
}

#ifdef BCM_XGS3_SWITCH_SUPPORT
/*
 * Function:
 *      _soc_xgs3_counter_dma_setup
 * Purpose:
 *      Configure hardware registers for counter collection.  Used during
 *      soc_counter_thread initialization.
 * Parameters:
 *      unit    - switch unit
 * Returns:
 *      SOC_E_*
 */
STATIC int
_soc_xgs3_counter_dma_setup(int unit)
{
    soc_control_t       *soc;
    int                 csize;
    pbmp_t              pbmp;
    soc_reg_t           reg;
    soc_ctr_type_t      ctype;
    uint32              val;
    uint32              ing_blk, ing_blkoff, ing_pstage, ing_c_cnt;
    uint32              egr_blk, egr_blkoff, egr_pstage, egr_c_cnt;
#ifdef BCM_SHADOW_SUPPORT
    uint32              inv_c_cnt = 0;
#endif
    uint32              gmac_c_cnt, xmac_c_cnt;
    int                 blk, bindex, blk_num, port, phy_port;
    soc_reg_t           blknum_reg, portnum_reg;
    int                 num_blknum, num_portnum, blknum_offset, portnum_offset;
    uint32              blknum_mask, portnum_mask;
    uint32              blknum_map[20];
    uint32              portnum_map[24];
#ifdef BCM_CMICM_SUPPORT
    int cmc = SOC_PCI_CMC(unit);
#endif
    ing_blk = ing_blkoff = ing_pstage = 0;
    egr_blk = egr_blkoff = egr_pstage = 0;

#ifdef BCM_FIREBOLT_SUPPORT
    if (SOC_IS_FBX(unit)) {
        ing_blk = SOC_BLK_IPIPE;
        egr_blk = SOC_BLK_EPIPE;
        ing_blkoff = SOC_BLOCK2OFFSET(unit, IPIPE_BLOCK(unit));
        egr_blkoff = SOC_BLOCK2OFFSET(unit, EPIPE_BLOCK(unit));
    }
#endif

#ifdef BCM_EASYRIDER_SUPPORT
    if (SOC_IS_EASYRIDER(unit)) {
        ing_blk = SOC_BLK_IGR;
        egr_blk = SOC_BLK_EGR;
        ing_blkoff = SOC_BLOCK2OFFSET(unit, IGR_BLOCK(unit));
        egr_blkoff = SOC_BLOCK2OFFSET(unit, EGR_BLOCK(unit));
    }
#endif

    soc = SOC_CONTROL(unit);
    SOC_PBMP_ASSIGN(pbmp, soc->counter_pbmp);
    /*
     * Calculate the number of counters from each block
     */
    ing_c_cnt = egr_c_cnt = gmac_c_cnt = xmac_c_cnt = 0;
#ifdef BCM_SHADOW_SUPPORT
    if (SOC_IS_SHADOW(unit)) {
        inv_c_cnt = ing_c_cnt;
    }
#endif
    for (ctype = 0; ctype < SOC_CTR_NUM_TYPES; ctype++) {
        if (!SOC_HAS_CTR_TYPE(unit, ctype)) {
            continue;
        }
        for (csize = SOC_CTR_MAP_SIZE(unit, ctype) - 1; csize > 0; csize--) {
            reg = SOC_CTR_TO_REG(unit, ctype, csize);
            if (SOC_COUNTER_INVALID(unit, reg)) {
                continue;                       /* skip trailing invalids */
            }
            if (SOC_REG_BLOCK_IS(unit, reg, ing_blk) &&
                                 (ing_c_cnt == 0)) {
                ing_c_cnt = csize + 1;
                if (soc_feature(unit, soc_feature_new_sbus_format)) {
                    ing_pstage = (SOC_REG_INFO(unit, reg).offset >> 26) & 0x3F;
                } else {
                    ing_pstage = (SOC_REG_INFO(unit, reg).offset >> 24) & 0xFF;
                }
            }
            if (SOC_REG_BLOCK_IS(unit, reg, egr_blk) &&
                                 (egr_c_cnt == 0)) {
                egr_c_cnt = csize + 1;
                if (soc_feature(unit, soc_feature_new_sbus_format)) {
                    egr_pstage = (SOC_REG_INFO(unit, reg).offset >> 26) & 0x3F;
                } else {
                    egr_pstage = (SOC_REG_INFO(unit, reg).offset >> 24) & 0xFF;
                }
            }
            if ((SOC_REG_BLOCK_IS(unit, reg, SOC_BLK_GPORT) ||
                 SOC_REG_BLOCK_IS(unit, reg, SOC_BLK_QGPORT) ||
                 SOC_REG_BLOCK_IS(unit, reg, SOC_BLK_SPORT) ||
                 SOC_REG_BLOCK_IS(unit, reg, SOC_BLK_XGPORT) ||
                 SOC_REG_BLOCK_IS(unit, reg, SOC_BLK_XQPORT) ||
                 SOC_REG_BLOCK_IS(unit, reg, SOC_BLK_GXPORT) ||
                 SOC_REG_BLOCK_IS(unit, reg, SOC_BLK_XLPORT) ||
                 SOC_REG_BLOCK_IS(unit, reg, SOC_BLK_XTPORT) ||
                 SOC_REG_BLOCK_IS(unit, reg, SOC_BLK_CLPORT)) &&
                (gmac_c_cnt == 0)) {
                gmac_c_cnt = csize + 1;
            }
            if ((SOC_REG_BLOCK_IS(unit, reg, SOC_BLK_XPORT) ||
                 SOC_REG_BLOCK_IS(unit, reg, SOC_BLK_XGPORT) ||
                 SOC_REG_BLOCK_IS(unit, reg, SOC_BLK_XQPORT) ||
                 SOC_REG_BLOCK_IS(unit, reg, SOC_BLK_GXPORT) ||
                 SOC_REG_BLOCK_IS(unit, reg, SOC_BLK_XLPORT) ||
                 SOC_REG_BLOCK_IS(unit, reg, SOC_BLK_XTPORT) ||
                 SOC_REG_BLOCK_IS(unit, reg, SOC_BLK_MXQPORT) ||
                 SOC_REG_BLOCK_IS(unit, reg, SOC_BLK_CLPORT)) &&
                (xmac_c_cnt == 0)) {
                xmac_c_cnt = csize + 1;
            }
        }
    }
#ifdef BCM_SHADOW_SUPPORT
    if (SOC_IS_SHADOW(unit)) {
        /* Remove leading invalids (Shadow) */
        for (csize = 0; csize < SOC_CTR_MAP_SIZE(unit, SOC_CTR_TYPE_XE); csize++) {
            reg = SOC_CTR_TO_REG(unit, SOC_CTR_TYPE_XE, csize);
            if (SOC_COUNTER_INVALID(unit, reg)) {
                inv_c_cnt++;
            } else {
                break;
            }
        }
    }
#endif /* BCM_SHADOW_SUPPORT */

    egr_c_cnt = (egr_c_cnt) ? (egr_c_cnt - ing_c_cnt) : 0;
    gmac_c_cnt = (gmac_c_cnt) ? (gmac_c_cnt - egr_c_cnt - ing_c_cnt) : 0;
    xmac_c_cnt = (xmac_c_cnt) ? (xmac_c_cnt - egr_c_cnt - ing_c_cnt) : 0;
#ifdef BCM_SHADOW_SUPPORT
    if (SOC_IS_SHADOW(unit)) {
        ing_c_cnt -= inv_c_cnt;
    }
#endif

    soc_cm_debug(DK_COUNTER,
                 "ing_c_cnt = %d egr_c_cnt = %d "
                 "gmac_c_cnt = %d xmac_c_cnt = %d\n"
                 "ing_pstage = %d egr_pstage = %d "
                 "ing_blkoff = %d egr_blkoff = %d\n",
                 ing_c_cnt, egr_c_cnt, gmac_c_cnt, xmac_c_cnt,
                 ing_pstage, egr_pstage, ing_blkoff, egr_blkoff);

    /* Prepare for CMIC_STAT_DMA_PORTNUM_MAP and BLKNUM_MAP */
    if (SOC_REG_IS_VALID(unit, CMIC_STAT_DMA_PORTNUM_MAP_3_0r)) {
        portnum_reg = CMIC_STAT_DMA_PORTNUM_MAP_3_0r;
        num_portnum = 4;
        portnum_offset = 8;
    } else if (SOC_REG_IS_VALID(unit, CMIC_STAT_DMA_PORTNUM_MAP_5_0r)) {
        portnum_reg = CMIC_STAT_DMA_PORTNUM_MAP_5_0r;
        num_portnum = 6;
        portnum_offset = 5;
    } else if (SOC_REG_IS_VALID(unit, CMIC_STAT_DMA_PORTNUM_MAP_7_0r)) {
        portnum_reg = CMIC_STAT_DMA_PORTNUM_MAP_7_0r;
        num_portnum = 8;
        portnum_offset = 4;
    } else {
        return SOC_E_INTERNAL;
    }
    portnum_mask =
        (1 << soc_reg_field_length(unit, portnum_reg, SBUS_PORTNUM_0f)) - 1;

    if (SOC_REG_IS_VALID(unit, CMIC_STAT_DMA_BLKNUM_MAP_4_0r)) {
        blknum_reg = CMIC_STAT_DMA_BLKNUM_MAP_4_0r;
        num_blknum = 5;
        blknum_offset = 6;
    } else if (SOC_REG_IS_VALID(unit, CMIC_STAT_DMA_BLKNUM_MAP_7_0r)) {
        blknum_reg = CMIC_STAT_DMA_BLKNUM_MAP_7_0r;
        num_blknum = 8;
        blknum_offset = 4;
    } else {
        return SOC_E_INTERNAL;
    }
    blknum_mask =
        (1 << soc_reg_field_length(unit, blknum_reg, SBUS_BLKNUM_0f)) - 1;

    sal_memset(portnum_map, 0, sizeof(portnum_map));
    sal_memset(blknum_map, 0, sizeof(blknum_map));
    PBMP_ITER(pbmp, port) {
        if (soc_feature(unit, soc_feature_logical_port_num)) {
            phy_port = SOC_INFO(unit).port_l2p_mapping[port];
        } else {
            phy_port = port;
        }
        blk = SOC_PORT_BLOCK(unit, phy_port);
        bindex = SOC_PORT_BINDEX(unit, phy_port);
        blk_num = SOC_BLOCK2OFFSET(unit, blk);
        blknum_map[port / num_blknum] |=
            (blk_num & blknum_mask) << (blknum_offset * (port % num_blknum));
        portnum_map[port / num_portnum] |=
            (bindex & portnum_mask) << (portnum_offset * (port % num_portnum));
    }

    switch (portnum_reg) {
    case CMIC_STAT_DMA_PORTNUM_MAP_3_0r:
        WRITE_CMIC_STAT_DMA_PORTNUM_MAP_3_0r(unit, portnum_map[0]);
        WRITE_CMIC_STAT_DMA_PORTNUM_MAP_7_4r(unit, portnum_map[1]);
        WRITE_CMIC_STAT_DMA_PORTNUM_MAP_11_8r(unit, portnum_map[2]);
        WRITE_CMIC_STAT_DMA_PORTNUM_MAP_15_12r(unit, portnum_map[3]);
        WRITE_CMIC_STAT_DMA_PORTNUM_MAP_19_16r(unit, portnum_map[4]);
        WRITE_CMIC_STAT_DMA_PORTNUM_MAP_23_20r(unit, portnum_map[5]);
        WRITE_CMIC_STAT_DMA_PORTNUM_MAP_27_24r(unit, portnum_map[6]);
        WRITE_CMIC_STAT_DMA_PORTNUM_MAP_31_28r(unit, portnum_map[7]);
        WRITE_CMIC_STAT_DMA_PORTNUM_MAP_35_32r(unit, portnum_map[8]);
        WRITE_CMIC_STAT_DMA_PORTNUM_MAP_39_36r(unit, portnum_map[9]);
        WRITE_CMIC_STAT_DMA_PORTNUM_MAP_43_40r(unit, portnum_map[10]);
        WRITE_CMIC_STAT_DMA_PORTNUM_MAP_47_44r(unit, portnum_map[11]);
        WRITE_CMIC_STAT_DMA_PORTNUM_MAP_51_48r(unit, portnum_map[12]);
        WRITE_CMIC_STAT_DMA_PORTNUM_MAP_55_52r(unit, portnum_map[13]);
        WRITE_CMIC_STAT_DMA_PORTNUM_MAP_59_56r(unit, portnum_map[14]);
        WRITE_CMIC_STAT_DMA_PORTNUM_MAP_63_60r(unit, portnum_map[15]);
        if (!SOC_REG_IS_VALID(unit, CMIC_STAT_DMA_PORTNUM_MAP_67_64r)) {
            break;
        }
        WRITE_CMIC_STAT_DMA_PORTNUM_MAP_67_64r(unit, portnum_map[16]);
        WRITE_CMIC_STAT_DMA_PORTNUM_MAP_71_68r(unit, portnum_map[17]);
        WRITE_CMIC_STAT_DMA_PORTNUM_MAP_75_72r(unit, portnum_map[18]);
        WRITE_CMIC_STAT_DMA_PORTNUM_MAP_79_76r(unit, portnum_map[19]);
        WRITE_CMIC_STAT_DMA_PORTNUM_MAP_83_80r(unit, portnum_map[20]);
        WRITE_CMIC_STAT_DMA_PORTNUM_MAP_87_84r(unit, portnum_map[21]);
        WRITE_CMIC_STAT_DMA_PORTNUM_MAP_91_88r(unit, portnum_map[22]);
        WRITE_CMIC_STAT_DMA_PORTNUM_MAP_95_92r(unit, portnum_map[23]);
        break;
    case CMIC_STAT_DMA_PORTNUM_MAP_5_0r:
        WRITE_CMIC_STAT_DMA_PORTNUM_MAP_5_0r(unit, portnum_map[0]);
        WRITE_CMIC_STAT_DMA_PORTNUM_MAP_11_6r(unit, portnum_map[1]);
        WRITE_CMIC_STAT_DMA_PORTNUM_MAP_17_12r(unit, portnum_map[2]);
        WRITE_CMIC_STAT_DMA_PORTNUM_MAP_23_18r(unit, portnum_map[3]);
        break;
    case CMIC_STAT_DMA_PORTNUM_MAP_7_0r:
        WRITE_CMIC_STAT_DMA_PORTNUM_MAP_7_0r(unit, portnum_map[0]);
        WRITE_CMIC_STAT_DMA_PORTNUM_MAP_15_8r(unit, portnum_map[1]);
        WRITE_CMIC_STAT_DMA_PORTNUM_MAP_23_16r(unit, portnum_map[2]);
        WRITE_CMIC_STAT_DMA_PORTNUM_MAP_31_24r(unit, portnum_map[3]);
        break;
    }

    switch (blknum_reg) {
    case CMIC_STAT_DMA_BLKNUM_MAP_4_0r:
        WRITE_CMIC_STAT_DMA_BLKNUM_MAP_4_0r(unit, blknum_map[0]);
        WRITE_CMIC_STAT_DMA_BLKNUM_MAP_9_5r(unit, blknum_map[1]);
        WRITE_CMIC_STAT_DMA_BLKNUM_MAP_14_10r(unit, blknum_map[2]);
        WRITE_CMIC_STAT_DMA_BLKNUM_MAP_19_15r(unit, blknum_map[3]);
        WRITE_CMIC_STAT_DMA_BLKNUM_MAP_24_20r(unit, blknum_map[4]);
        WRITE_CMIC_STAT_DMA_BLKNUM_MAP_29_25r(unit, blknum_map[5]);
        WRITE_CMIC_STAT_DMA_BLKNUM_MAP_34_30r(unit, blknum_map[6]);
        WRITE_CMIC_STAT_DMA_BLKNUM_MAP_39_35r(unit, blknum_map[7]);
        WRITE_CMIC_STAT_DMA_BLKNUM_MAP_44_40r(unit, blknum_map[8]);
        WRITE_CMIC_STAT_DMA_BLKNUM_MAP_49_45r(unit, blknum_map[9]);
        WRITE_CMIC_STAT_DMA_BLKNUM_MAP_54_50r(unit, blknum_map[10]);
        WRITE_CMIC_STAT_DMA_BLKNUM_MAP_59_55r(unit, blknum_map[11]);
        if (SOC_REG_IS_VALID(unit, CMIC_STAT_DMA_BLKNUM_MAP_63_60r)) {
            WRITE_CMIC_STAT_DMA_BLKNUM_MAP_63_60r(unit, blknum_map[12]);
            break;
        }
        WRITE_CMIC_STAT_DMA_BLKNUM_MAP_64_60r(unit, blknum_map[12]);
        WRITE_CMIC_STAT_DMA_BLKNUM_MAP_69_65r(unit, blknum_map[13]);
        WRITE_CMIC_STAT_DMA_BLKNUM_MAP_74_70r(unit, blknum_map[14]);
        WRITE_CMIC_STAT_DMA_BLKNUM_MAP_79_75r(unit, blknum_map[15]);
        WRITE_CMIC_STAT_DMA_BLKNUM_MAP_84_80r(unit, blknum_map[16]);
        WRITE_CMIC_STAT_DMA_BLKNUM_MAP_89_85r(unit, blknum_map[17]);
        WRITE_CMIC_STAT_DMA_BLKNUM_MAP_94_90r(unit, blknum_map[18]);
        WRITE_CMIC_STAT_DMA_BLKNUM_MAP_95r(unit, blknum_map[19]);
        break;
    case CMIC_STAT_DMA_BLKNUM_MAP_7_0r:
        WRITE_CMIC_STAT_DMA_BLKNUM_MAP_7_0r(unit, blknum_map[0]);
        WRITE_CMIC_STAT_DMA_BLKNUM_MAP_15_8r(unit, blknum_map[1]);
        WRITE_CMIC_STAT_DMA_BLKNUM_MAP_23_16r(unit, blknum_map[2]);
        WRITE_CMIC_STAT_DMA_BLKNUM_MAP_31_24r(unit, blknum_map[3]);
        if (!SOC_REG_IS_VALID(unit, CMIC_STAT_DMA_BLKNUM_MAP_39_32r)) {
            break;
        }
        WRITE_CMIC_STAT_DMA_BLKNUM_MAP_39_32r(unit, blknum_map[4]);
        WRITE_CMIC_STAT_DMA_BLKNUM_MAP_47_40r(unit, blknum_map[5]);
        WRITE_CMIC_STAT_DMA_BLKNUM_MAP_55_48r(unit, blknum_map[6]);

        break;
    }

    /*
     * Reset value in CMIC_STAT_DMA_SETUP register is good. No
     * additional setup necessary for FB/ER
     */
#ifdef BCM_CMICM_SUPPORT
    if(soc_feature(unit, soc_feature_cmicm)) {
        soc_pci_write(unit,CMIC_CMCx_STAT_DMA_ADDR_OFFSET(cmc),soc_cm_l2p(unit, soc->counter_buf64));
        soc_pci_write(unit,CMIC_CMCx_STAT_DMA_PORTS_0_OFFSET(cmc),SOC_PBMP_WORD_GET(pbmp, 0));
        if (SOC_REG_IS_VALID(unit, CMIC_CMC0_STAT_DMA_PORTS_1r)) { 
            soc_pci_write(unit,CMIC_CMCx_STAT_DMA_PORTS_1_OFFSET(cmc),SOC_PBMP_WORD_GET(pbmp, 1));
        }
        if (SOC_REG_IS_VALID(unit, CMIC_CMC0_STAT_DMA_PORTS_2r)) {  
            soc_pci_write(unit,CMIC_CMCx_STAT_DMA_PORTS_2_OFFSET(cmc),SOC_PBMP_WORD_GET(pbmp, 2));
        }
        SOC_PBMP_ASSIGN(pbmp, PBMP_HG_ALL(unit));
        SOC_PBMP_OR(pbmp, PBMP_XE_ALL(unit));
        SOC_PBMP_AND(pbmp, soc->counter_pbmp);

        WRITE_CMIC_STAT_DMA_PORT_TYPE_MAP_0r(unit, SOC_PBMP_WORD_GET(pbmp, 0));
        if (SOC_REG_IS_VALID(unit, CMIC_STAT_DMA_PORT_TYPE_MAP_1r)) {
            WRITE_CMIC_STAT_DMA_PORT_TYPE_MAP_1r
                (unit, SOC_PBMP_WORD_GET(pbmp, 1));
        }
        if (SOC_REG_IS_VALID(unit, CMIC_STAT_DMA_PORT_TYPE_MAP_2r)) {
            WRITE_CMIC_STAT_DMA_PORT_TYPE_MAP_2r
                (unit, SOC_PBMP_WORD_GET(pbmp, 2));
        }
    } else
#endif /* CMICM Support */
    {
        WRITE_CMIC_STAT_DMA_ADDRr(unit, soc_cm_l2p(unit, soc->counter_buf64));

        WRITE_CMIC_STAT_DMA_PORTSr(unit, SOC_PBMP_WORD_GET(pbmp, 0));
        if (SOC_REG_IS_VALID(unit, CMIC_STAT_DMA_PORTS_HIr)) {
            WRITE_CMIC_STAT_DMA_PORTS_HIr(unit, SOC_PBMP_WORD_GET(pbmp, 1));
        }
        if (SOC_REG_IS_VALID(unit, CMIC_STAT_DMA_PORTS_HI_2r)) {
            WRITE_CMIC_STAT_DMA_PORTS_HI_2r(unit, SOC_PBMP_WORD_GET(pbmp, 2));
        }

        if (SOC_IS_HB_GW(unit)) {
            SOC_PBMP_ASSIGN(pbmp, PBMP_PORT_ALL(unit));
        } else {
            SOC_PBMP_ASSIGN(pbmp, PBMP_HG_ALL(unit));
            SOC_PBMP_OR(pbmp, PBMP_XE_ALL(unit));
        }
        SOC_PBMP_AND(pbmp, soc->counter_pbmp);

        WRITE_CMIC_STAT_DMA_PORT_TYPE_MAPr(unit, SOC_PBMP_WORD_GET(pbmp, 0));
        if (SOC_REG_IS_VALID(unit, CMIC_STAT_DMA_PORT_TYPE_MAP_HIr)) {
            WRITE_CMIC_STAT_DMA_PORT_TYPE_MAP_HIr
                (unit, SOC_PBMP_WORD_GET(pbmp, 1));
        }
        if (SOC_REG_IS_VALID(unit, CMIC_STAT_DMA_PORT_TYPE_MAP_HI_2r)) {
            WRITE_CMIC_STAT_DMA_PORT_TYPE_MAP_HI_2r
                (unit, SOC_PBMP_WORD_GET(pbmp, 2));
        }
    }
    val = 0;
    soc_reg_field_set(unit, CMIC_STAT_DMA_ING_STATS_CFGr,
                      &val, ING_ETH_BLK_NUMf, ing_blkoff);
    soc_reg_field_set(unit, CMIC_STAT_DMA_ING_STATS_CFGr,
                      &val, ING_STAT_COUNTERS_NUMf, ing_c_cnt);
    soc_reg_field_set(unit, CMIC_STAT_DMA_ING_STATS_CFGr,
                      &val, ING_STATS_PIPELINE_STAGE_NUMf, ing_pstage);
    WRITE_CMIC_STAT_DMA_ING_STATS_CFGr(unit, val);

    val = 0;
    soc_reg_field_set(unit, CMIC_STAT_DMA_EGR_STATS_CFGr,
                      &val, EGR_ETH_BLK_NUMf, egr_blkoff);
    soc_reg_field_set(unit, CMIC_STAT_DMA_EGR_STATS_CFGr,
                      &val, EGR_STAT_COUNTERS_NUMf, egr_c_cnt);
    soc_reg_field_set(unit, CMIC_STAT_DMA_EGR_STATS_CFGr,
                      &val, EGR_STATS_PIPELINE_STAGE_NUMf, egr_pstage);
    WRITE_CMIC_STAT_DMA_EGR_STATS_CFGr(unit, val);

    val = 0;
    soc_reg_field_set(unit, CMIC_STAT_DMA_MAC_STATS_CFGr,
                      &val, MAC_G_STAT_COUNTERS_NUMf, gmac_c_cnt);
    soc_reg_field_set(unit, CMIC_STAT_DMA_MAC_STATS_CFGr,
                      &val, MAC_X_STAT_COUNTERS_NUMf, xmac_c_cnt);
    soc_reg_field_set(unit, CMIC_STAT_DMA_MAC_STATS_CFGr,
                      &val, CPU_STATS_PORT_NUMf, CMIC_PORT(unit));
    WRITE_CMIC_STAT_DMA_MAC_STATS_CFGr(unit, val);

#ifdef BCM_CMICM_SUPPORT
    if(soc_feature(unit, soc_feature_cmicm)) {
        val = soc_pci_read(unit,CMIC_CMCx_STAT_DMA_CFG_OFFSET(cmc));
        soc_reg_field_set(unit, CMIC_CMC0_STAT_DMA_CFGr, &val,
                          ST_DMA_ITER_DONE_CLRf, 1);
        if (soc_feature(unit, soc_feature_new_sbus_format)) {
            soc_reg_field_set(unit, CMIC_CMC0_STAT_DMA_CFGr, &val,
                              EN_TR3_SBUS_STYLEf, 1);
        }
        soc_pci_write(unit,CMIC_CMCx_STAT_DMA_CFG_OFFSET(cmc),val);
        soc_cmicm_intr0_enable(unit, IRQ_CMCx_STAT_ITER_DONE);
    } else
#endif
    {
        WRITE_CMIC_DMA_STATr(unit, DS_STAT_DMA_ITER_DONE_CLR);
        soc_intr_enable(unit, IRQ_STAT_ITER_DONE);
    }
    return SOC_E_NONE;
}
#endif /* BCM_XGS3_SWITCH_SUPPORT */

/*
 * Function:
 *      _soc_counter_dma_setup
 * Purpose:
 *      Configure hardware registers for counter collection.  Used during
 *      soc_counter_thread initialization.
 * Parameters:
 *      unit    - switch unit
 * Returns:
 *      SOC_E_*
 */

static int
_soc_counter_dma_setup(int unit)
{
    soc_control_t       *soc;
    uint32              creg_first, creg_last, cireg_first, cireg_last;
    int                 csize;
    pbmp_t              pbmp;
    soc_reg_t           reg;
    soc_ctr_type_t      ctype;
    uint32              val, offset;

#ifdef BCM_XGS3_SWITCH_SUPPORT
    if (SOC_IS_XGS3_SWITCH(unit)) {
        return(_soc_xgs3_counter_dma_setup(unit));
    }
#endif

    soc = SOC_CONTROL(unit);
    SOC_PBMP_ASSIGN(pbmp, soc->counter_pbmp);

    if (SOC_IS_DRACO(unit)) {
        /* For BCM5690/95, do not use DMA for HiGig counter collection */
        SOC_PBMP_REMOVE(pbmp, PBMP_HG_ALL(unit));
    }

    WRITE_CMIC_STAT_DMA_ADDRr(unit, soc_cm_l2p(unit, soc->counter_buf32));
    if (SOC_IS_TUCANA(unit)) {
        WRITE_CMIC_STAT_DMA_PORTS_MOD0r(unit, SOC_PBMP_WORD_GET(pbmp, 0));
        WRITE_CMIC_STAT_DMA_PORTS_MOD1r(unit, SOC_PBMP_WORD_GET(pbmp, 1));
        WRITE_CMIC_STAT_DMA_MMU_PORTS0r(unit, SOC_PBMP_WORD_GET(pbmp, 0));
        WRITE_CMIC_STAT_DMA_MMU_PORTS1r(unit, SOC_PBMP_WORD_GET(pbmp, 1));
    } else {
        WRITE_CMIC_STAT_DMA_PORTSr(unit, SOC_PBMP_WORD_GET(pbmp, 0));
    }

    /*
     * Calculate the first and last register offsets for collection
     * Higig offsets are kept separately
     */
    creg_first = creg_last = cireg_first = cireg_last = 0;
    for (ctype = 0; ctype < SOC_CTR_NUM_TYPES; ctype++) {
        if (!SOC_HAS_CTR_TYPE(unit, ctype)) {
            continue;
        }
        if (ctype == SOC_CTR_TYPE_GFE) {        /* offsets screwed up */
            continue;
        }
        reg = SOC_CTR_TO_REG(unit, ctype, 0);
        if (!SOC_COUNTER_INVALID(unit, reg)) {
            if (ctype == SOC_CTR_TYPE_HG) {
                cireg_first = SOC_REG_INFO(unit, reg).offset;
            } else {
                creg_first = SOC_REG_INFO(unit, reg).offset;
                if (cireg_first == 0) {
                    cireg_first = creg_first;
                }
            }
        }
        csize = SOC_CTR_MAP_SIZE(unit, ctype) - 1;
        for (; csize > 0; csize--) {
            reg = SOC_CTR_TO_REG(unit, ctype, csize);
            if (SOC_COUNTER_INVALID(unit, reg)) {
                continue;                       /* skip trailing invalids */
            }
            if (SOC_REG_BLOCK_IS(unit, reg, SOC_BLK_MMU)) {
                continue;                       /* skip mmu regs for Tucana */
            }
            offset = SOC_REG_INFO(unit, reg).offset;
            csize = SOC_REG_IS_64(unit, reg) ?
                sizeof(uint64) : sizeof(uint32);
            csize = (soc->counter_portsize / csize) - 1;
            if (ctype == SOC_CTR_TYPE_HG) {
                if (offset > cireg_last) {
                    cireg_last = offset;
                    if (cireg_last > cireg_first + csize) {
                        cireg_last = cireg_first + csize;
                    }
                }
            } else {
                if (offset > creg_last) {
                    creg_last = offset;
                    if (creg_last > creg_first + csize) {
                        creg_last = creg_first + csize;
                    }
                }
            }
            break;
        }
    }

    /*
     * Set the various configuration registers
     */
    if (SOC_IS_HERCULES(unit)) {
        creg_first = cireg_first;
        creg_last = cireg_last;
    }
    if (SOC_IS_LYNX(unit) || SOC_IS_HERCULES15(unit)) {
        val = 0;
        soc_reg_field_set(unit, CMIC_64BIT_STATS_CFGr, &val,
                          L_STAT_REGf, creg_last);
        soc_reg_field_set(unit, CMIC_64BIT_STATS_CFGr, &val,
                          F_STAT_REGf, creg_first);
        WRITE_CMIC_64BIT_STATS_CFGr(unit, val);
    } else {
        val = 0;
        soc_reg_field_set(unit, CMIC_STAT_DMA_SETUPr, &val,
                          L_STAT_REGf, creg_last);
        soc_reg_field_set(unit, CMIC_STAT_DMA_SETUPr, &val,
                          F_STAT_REGf, creg_first);
        WRITE_CMIC_STAT_DMA_SETUPr(unit, val);
    }

    if (SOC_IS_DRACO1(unit)) {
        /* set for 512 bytes/port */
        val = 0;
        soc_reg_field_set(unit, CMIC_64BIT_STATS_CFGr, &val,
                          SIZE_SELECTf, 1);
        soc_reg_field_set(unit, CMIC_64BIT_STATS_CFGr, &val,
                          LAST_64_REGf, cireg_last);
        WRITE_CMIC_64BIT_STATS_CFGr(unit, val);
    }
    if (SOC_IS_DRACO15(unit)) {
        /* set for 1024 bytes/port */
        val = 0;
        soc_reg_field_set(unit, CMIC_64BIT_STATS_CFGr, &val,
                          SIZE_SELECTf, 0);
#if defined(BCM_5695)
        soc_reg_field_set(unit, CMIC_64BIT_STATS_CFGr, &val,
                          KSIZE_SELECTf, 1);
#endif
        soc_reg_field_set(unit, CMIC_64BIT_STATS_CFGr, &val,
                          LAST_64_REGf, cireg_last);
        WRITE_CMIC_64BIT_STATS_CFGr(unit, val);
    }
    if (SOC_IS_TUCANA(unit)) {
        val = 0;
        soc_reg_field_set(unit, CMIC_IPIC_STATS_CFGr, &val,
                          F_STAT_REG_64Bf, cireg_first);
        soc_reg_field_set(unit, CMIC_IPIC_STATS_CFGr, &val,
                          L_STAT_REG_64Bf, cireg_last);
        WRITE_CMIC_IPIC_STATS_CFGr(unit, val);

#define TUC_COS         0xff    /* COSes to collect on */
#define TUC_PERCOS      0x01    /* per-COS regs to collect */
#define TUC_NONPERCOS   0x03    /* non-per-COS regs to collect */

        val = 0;
        soc_reg_field_set(unit, CMIC_STAT_DMA_MMU_SETUPr, &val,
                          MMU_PP_PC_STATS_REGS_BMAPf, TUC_PERCOS);
        soc_reg_field_set(unit, CMIC_STAT_DMA_MMU_SETUPr, &val,
                          MMU_PP_NONPC_STATS_REGS_BMAPf, TUC_NONPERCOS);
        soc_reg_field_set(unit, CMIC_STAT_DMA_MMU_SETUPr, &val,
                          MMU_STATS_COS_BMAPf, TUC_COS);
        WRITE_CMIC_STAT_DMA_MMU_SETUPr(unit, val);
    }

    WRITE_CMIC_DMA_STATr(unit, DS_STAT_DMA_ITER_DONE_CLR);
    soc_intr_enable(unit, IRQ_STAT_ITER_DONE);
    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_counter_thread
 * Purpose:
 *      Master counter collection and accumulation thread.
 * Parameters:
 *      unit_vp - StrataSwitch unit # (as a void *).
 * Returns:
 *      Nothing, does not return.
 */

void
soc_counter_thread(void *unit_vp)
{
    int                 unit = PTR_TO_INT(unit_vp);
    soc_control_t       *soc = SOC_CONTROL(unit);
    int                 rv;
    int                 interval;
    uint32              val;
    int                 dma;
    sal_usecs_t         cdma_timeout, now;
    COUNTER_ATOMIC_DEF  s;
    int                 sync_gnt = FALSE;
    int                 i;
#ifdef BCM_CMICM_SUPPORT
    int cmc = SOC_PCI_CMC(unit);
#endif

    soc_cm_debug(DK_COUNTER, "soc_counter_thread: unit=%d\n", unit);

    /*
     * Create a semaphore used to time the trigger scans, and if DMA is
     * used, monitor for the Stats DMA Iteration Done interrupt.
     */
    cdma_timeout = soc_property_get(unit, spn_CDMA_TIMEOUT_USEC, 1000000);

    dma = ((soc->counter_flags & SOC_COUNTER_F_DMA) != 0);

    if (dma) {
        rv = _soc_counter_dma_setup(unit);
        if (rv < 0) {
            goto done;
        }
    }

    /*
     * The hardware timer can only be used for intervals up to about
     * 1.048 seconds.  This implementation uses a software timer (via
     * semaphore timeout) instead of the hardware timer.
     */

    while ((interval = soc->counter_interval) != 0) {
#ifdef COUNTER_BENCH
        sal_usecs_t     start_time;
#endif
        int             err = 0;

        /*
         * Use a semaphore timeout instead of a sleep to achieve the
         * desired delay between scans.  This allows this thread to exit
         * immediately when soc_counter_stop wants it to.
         */

        soc_cm_debug(DK_COUNTER + DK_VERBOSE,
                     "soc_counter_thread: sleep %d\n", interval);

        (void)sal_sem_take(soc->counter_trigger, interval);

        if (soc->counter_interval == 0) {       /* Exit signaled */
            break;
        }

        if (soc->counter_sync_req) {
            sync_gnt = TRUE;
        }

#ifdef COUNTER_BENCH
        start_time = sal_time_usecs();
#endif

        /*
         * If in DMA mode, use DMA to transfer the counters into
         * memory.  Start a DMA pass by enabling DMA and clearing
         * STAT_DMA_DONE bit.  Wait for the pass to finish.
         */
        COUNTER_LOCK(unit);
        if (dma) {
            soc_cm_debug(DK_COUNTER + DK_VERBOSE,
                         "soc_counter_thread: trigger DMA\n");

#ifdef BCM_CMICM_SUPPORT
            if(soc_feature(unit, soc_feature_cmicm)) {
                /* Clear Status */
                val = soc_pci_read(unit, CMIC_CMCx_STAT_DMA_CFG_OFFSET(cmc));
                soc_reg_field_set(unit, CMIC_CMC0_STAT_DMA_CFGr, &val,
                                  ENf, 0);
                soc_reg_field_set(unit, CMIC_CMC0_STAT_DMA_CFGr, &val,
                                  ST_DMA_ITER_DONE_CLRf, 0);
                soc_pci_write(unit, CMIC_CMCx_STAT_DMA_CFG_OFFSET(cmc), val);
                /* start DMA */
                soc_reg_field_set(unit, CMIC_CMC0_STAT_DMA_CFGr, &val,
                                  ENf, 1);
                soc_pci_write(unit, CMIC_CMCx_STAT_DMA_CFG_OFFSET(cmc), val);
            } else 
#endif
            {
                /* Clear Status and start DMA */
                WRITE_CMIC_DMA_STATr(unit, DS_STAT_DMA_DONE_CLR);
                READ_CMIC_STAT_DMA_SETUPr(unit, &val);
                soc_reg_field_set(unit, CMIC_STAT_DMA_SETUPr, &val,
                                  ENf, 1);
                WRITE_CMIC_STAT_DMA_SETUPr(unit, val);
            }

            if (SOC_IS_TUCANA(unit)) {  /* enable mmu stats dma */
                READ_CMIC_STAT_DMA_MMU_SETUPr(unit, &val);
                soc_reg_field_set(unit, CMIC_STAT_DMA_MMU_SETUPr, &val,
                                  MMU_STATS_ENf, 1);
                WRITE_CMIC_STAT_DMA_MMU_SETUPr(unit, val);
            }

            /* Wait for ISR to wake semaphore */
#ifdef BCM_CMICM_SUPPORT
            if(soc_feature(unit, soc_feature_cmicm)) {
                if (sal_sem_take(soc->counter_intr, cdma_timeout) >= 0) {
                    soc_cm_sinval(unit,
                                  (void *)soc->counter_buf32,
                                  soc->counter_bufsize);

                    soc_cm_debug(DK_COUNTER + DK_VERBOSE,
                                 "soc_counter_thread: "
                                 "DMA iter done\n");

#ifdef COUNTER_BENCH
                    soc_cm_debug(DK_VERBOSE,
                                 "Time taken for dma: %d usec\n",
                                 SAL_USECS_SUB(sal_time_usecs(), start_time));
#endif
                } else {
                    soc_cm_debug(DK_ERR,
                                 "soc_counter_thread: "
                                 "DMA did not finish buf32=%p\n",
                                 (void *)soc->counter_buf32);
                    err = 1;
                }
                if (soc_pci_read(unit, CMIC_CMCx_STAT_DMA_STAT_OFFSET(cmc)) & ST_CMCx_DMA_ERR) {
                    soc_cm_debug(DK_ERR,
                                 "soc_counter_thread: unit = %d DMA Error\n",
                                 unit);
                    /* ToDo : Clear Error */
                    err = 1;
                }
            } else
#endif /* CMICm Support */
            {
                if (sal_sem_take(soc->counter_intr, cdma_timeout) >= 0) {
                    soc_cm_sinval(unit,
                                  (void *)soc->counter_buf32,
                                  soc->counter_bufsize);

                    soc_cm_debug(DK_COUNTER + DK_VERBOSE,
                                 "soc_counter_thread: "
                                 "DMA iter done\n");

#ifdef COUNTER_BENCH
                    soc_cm_debug(DK_VERBOSE,
                                 "Time taken for dma: %d usec\n",
                                 SAL_USECS_SUB(sal_time_usecs(), start_time));
#endif
                } else {
                    soc_cm_debug(DK_ERR,
                                 "soc_counter_thread: "
                                 "DMA did not finish buf32=%p\n",
                                 (void *)soc->counter_buf32);
                    err = 1;
                }
#ifdef BCM_XGS3_SWITCH_SUPPORT
                if (SOC_IS_XGS3_SWITCH(unit)) {
                    if (soc_pci_read(unit, CMIC_DMA_STAT) & DS_STAT_DMA_ERROR) {
                        if (soc_feature(unit, soc_feature_stat_dma_error_ack)) {
                            WRITE_CMIC_DMA_STATr(unit, DS_STAT_DMA_ERROR_CLR - 1);
                        } else {
                        WRITE_CMIC_DMA_STATr(unit, DS_STAT_DMA_ERROR_CLR);
                        }
                        soc_cm_debug(DK_ERR,
                                     "soc_counter_thread: unit = %d DMA Error\n",
                                     unit);
                        err = 1;
                    }
                }
#endif
            }
            if (soc->counter_interval == 0) {   /* Exit signaled */
                COUNTER_UNLOCK(unit);
                break;
            }
        }

        /*
         * Add up changes to counter values.
         */

        now = sal_time_usecs();
        COUNTER_ATOMIC_BEGIN(s);
        soc->counter_coll_prev = soc->counter_coll_cur;
        soc->counter_coll_cur = now;
        COUNTER_ATOMIC_END(s);

        if ( (!err) && (soc->counter_n32 > 0) && (soc->counter_interval) ) {
	    rv = soc_counter_collect32(unit, FALSE);
            if (rv < 0) {
                soc_cm_debug(DK_ERR,
                             "soc_counter_thread: collect32 failed: %s\n",
                             soc_errmsg(rv));
                err = 1;
            }
        }

#ifdef BCM_XGS_SUPPORT
        if ((!err) && (soc->counter_n64 > 0) && (soc->counter_interval) ) {
	    rv = soc_counter_collect64(unit, FALSE);
            if (rv < 0) {
                soc_cm_debug(DK_ERR,
                             "soc_counter_thread: collect64 failed: %s\n",
                             soc_errmsg(rv));
                err = 1;
            }
        }
#endif

        /*
         * Check optional non CMIC counter DMA support entries
         * These counters are included in "show counter" output
         */
        if ((soc->counter_interval)) {
            soc_counter_collect_non_dma_entries(unit);
        }
        
	COUNTER_UNLOCK(unit);

        /*
         * Callback for additional work
         * These counters are not included in "show counter" output
         */
        for (i = 0; i < SOC_COUNTER_EXTRA_CB_MAX; i++) {
            if (soc_counter_extra[unit][i] != NULL) {
                soc_counter_extra[unit][i](unit);
            }
        }

        /*
         * Forgive spurious errors
         */

        if (err) {
                soc_cm_debug(DK_ERR, "soc_counter_thread: Too many errors\n");
                rv = SOC_E_INTERNAL;
                goto done;
            }

#ifdef COUNTER_BENCH
        soc_cm_debug(DK_VERBOSE,
                     "Iteration time: %d usec\n",
                     SAL_USECS_SUB(sal_time_usecs(), start_time));
#endif

        if (sync_gnt) {
            soc->counter_sync_req = 0;
            sync_gnt = 0;
        }
    }

    rv = SOC_E_NONE;

 done:
    if (rv < 0) {
        soc_cm_debug(DK_ERR,
                     "soc_counter_thread: Operation failed; exiting\n");
        soc_event_generate(unit, SOC_SWITCH_EVENT_THREAD_ERROR, 
                           SOC_SWITCH_EVENT_THREAD_COUNTER, __LINE__, rv);
    }

    soc_cm_debug(DK_COUNTER, "soc_counter_thread: exiting\n");

    soc->counter_pid = SAL_THREAD_ERROR;
    soc->counter_interval = 0;

    sal_thread_exit(0);
}

/*
 * Function:
 *      soc_counter_start
 * Purpose:
 *      Start the counter collection, S/W accumulation process.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      flags - SOC_COUNTER_F_xxx flags.
 *      interval - collection period in micro-seconds.
 *              Using 0 is the same as calling soc_counter_stop().
 *      pbmp - bit map of ports to collact counters on.
 * Returns:
 *      SOC_E_XXX
 */

int
soc_counter_start(int unit, uint32 flags, int interval, pbmp_t pbmp)
{
    soc_control_t       *soc = SOC_CONTROL(unit);
    char                pfmt[SOC_PBMP_FMT_LEN];
    sal_sem_t           sem;
    int 		rv;
    soc_port_t          p;

    soc_cm_debug(DK_COUNTER,
                 "soc_counter_start: unit=%d flags=0x%x "
                 "interval=%d pbmp=%s\n",
                 unit, flags, interval, SOC_PBMP_FMT(pbmp, pfmt));

    /* Stop if already running */

    if (soc->counter_interval != 0) {
        SOC_IF_ERROR_RETURN(soc_counter_stop(unit));
    }

    if (interval == 0) {
        return SOC_E_NONE;
    }

    /* Create fresh semaphores */

    if ((sem = soc->counter_trigger) != NULL) {
        soc->counter_trigger = NULL;    /* Stop others from waking sem */
        sal_sem_destroy(sem);           /* Then destroy it */
    }

    soc->counter_trigger =
        sal_sem_create("counter_trigger", sal_sem_BINARY, 0);

    if ((sem = soc->counter_intr) != NULL) {
        soc->counter_intr = NULL;       /* Stop intr from waking sem */
        sal_sem_destroy(sem);           /* Then destroy it */
    }

    soc->counter_intr =
        sal_sem_create("counter_intr", sal_sem_BINARY, 0);

    if (soc->counter_trigger == NULL || soc->counter_intr == NULL) {
        soc_cm_debug(DK_ERR, "soc_counter_start: sem create failed\n");
        return SOC_E_INTERNAL;
    }

    sal_snprintf(soc->counter_name,
                 sizeof(soc->counter_name),
                 "bcmCNTR.%d", unit);

    SOC_PBMP_ASSIGN(soc->counter_pbmp, pbmp);
    PBMP_ITER(soc->counter_pbmp, p) {
        if ((SOC_PBMP_MEMBER(SOC_PORT_DISABLED_BITMAP(unit,all), p))) {
            SOC_PBMP_PORT_REMOVE(soc->counter_pbmp, p);
        }
        if (IS_LB_PORT(unit, p)) {
            SOC_PBMP_PORT_REMOVE(soc->counter_pbmp, p);
        }
    }
    soc->counter_flags = flags;

    soc->counter_flags &= ~SOC_COUNTER_F_SWAP64; /* HW takes care of this */

    if (!soc_feature(unit, soc_feature_stat_dma) || SOC_IS_RCPU_ONLY(unit)) {
        soc->counter_flags &= ~SOC_COUNTER_F_DMA;
    }

    if (SOC_IS_HERCULES1(unit)) {
        uint32          esel;

        esel = soc_pci_read(unit, CMIC_ENDIAN_SELECT);
        if (esel & ES_BIG_ENDIAN_DMA_OTHER) {
            soc->counter_flags |= SOC_COUNTER_F_SWAP64;
        }
    }

    /*
     * The HOLD register is not supported by DMA on HUMV/Bradley, 
     * but in order to allow certain test scripts to pass, we 
     * optionally collect this register manually.
     */

#ifdef BCM_BRADLEY_SUPPORT
    soc->counter_flags &= ~SOC_COUNTER_F_HOLD;

    if (SOC_IS_HBX(unit)) {
        if (soc_property_get(unit, spn_CDMA_PIO_HOLD_ENABLE, 1)) {
            soc->counter_flags |= SOC_COUNTER_F_HOLD;
        }
    }
#endif /* BCM_BRADLEY_SUPPORT */

#if defined(BCM_BRADLEY_SUPPORT) || defined(BCM_TRX_SUPPORT)
    if (soc_feature(unit, soc_feature_bigmac_rxcnt_bug)) {
        /* Allocate buffer for DMA counter validation */
        soc_counter_tbuf[unit] = sal_alloc(SOC_COUNTER_TBUF_SIZE(unit), 
                                           "counter_tbuf");
        if (soc_counter_tbuf[unit] == NULL) {
            soc_cm_debug(DK_ERR,
                         "soc_counter_thread: unit %d: "
                         "failed to allocate temp counter buffer\n",
                         unit);
        }
    }
#endif /* BCM_BRADLEY_SUPPORT || BCM_TRX_SUPPORT */

    SOC_IF_ERROR_RETURN(soc_counter_autoz(unit, 0));

    /* Synchronize counter 'prev' values with current hardware counters */

    soc->counter_coll_prev = soc->counter_coll_cur = sal_time_usecs();

    if (soc->counter_n32 > 0) {
        COUNTER_LOCK(unit);
	rv = soc_counter_collect32(unit, TRUE);
        COUNTER_UNLOCK(unit);
        SOC_IF_ERROR_RETURN(rv);
    }

#if defined(BCM_XGS_SUPPORT)
    if (soc->counter_n64 > 0) {
        COUNTER_LOCK(unit);
	rv = soc_counter_collect64(unit, TRUE);
        COUNTER_UNLOCK(unit);
        SOC_IF_ERROR_RETURN(rv);
    }
#endif

    /* Start the thread */

    if (interval != 0) {
        soc->counter_interval = interval;

        soc->counter_pid =
            sal_thread_create(soc->counter_name,
                              SAL_THREAD_STKSZ,
                              soc_property_get(unit,
                                               spn_COUNTER_THREAD_PRI,
                                               50),
                              soc_counter_thread, INT_TO_PTR(unit));

        if (soc->counter_pid == SAL_THREAD_ERROR) {
            soc->counter_interval = 0;
            soc_cm_debug(DK_ERR, "soc_counter_start: thread create failed\n");
            return (SOC_E_INTERNAL);
        }

        soc_cm_debug(DK_COUNTER, "soc_counter_start: complete\n");
    }

    return (SOC_E_NONE);
}

/*
 * Function:
 *      soc_counter_status
 * Purpose:
 *      Get the status of counter collection, S/W accumulation process.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      flags - SOC_COUNTER_F_xxx flags.
 *      interval - collection period in micro-seconds.
 *      pbmp - bit map of ports to collact counters on.
 * Returns:
 *      SOC_E_XXX
 */

int
soc_counter_status(int unit, uint32 *flags, int *interval, pbmp_t *pbmp)
{
    soc_control_t       *soc = SOC_CONTROL(unit);
 
    soc_cm_debug(DK_COUNTER, "soc_counter_status: unit=%d\n", unit);

    *interval = soc->counter_interval;
    *flags = soc->counter_flags;
    SOC_PBMP_ASSIGN(*pbmp, soc->counter_pbmp);

    return (SOC_E_NONE);
}

/*
 * Function:
 *      soc_counter_sync
 * Purpose:
 *      Force an immediate counter update
 * Parameters:
 *      unit - StrataSwitch unit #.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      Ensures that ALL counter activity that occurred before the sync
 *      is reflected in the results of any soc_counter_get()-type
 *      routine that is called after the sync.
 */

int
soc_counter_sync(int unit)
{
    soc_control_t       *soc = SOC_CONTROL(unit);
    soc_timeout_t       to;
    uint32              stat_sync_timeout;

    if (soc->counter_interval == 0) {
        return SOC_E_DISABLED;
    }

    /* Trigger a collection */

    soc->counter_sync_req = TRUE;

    sal_sem_give(soc->counter_trigger);

    if (SAL_BOOT_QUICKTURN) {
        stat_sync_timeout = STAT_SYNC_TIMEOUT_QT;
    } else if (SAL_BOOT_BCMSIM) {
        stat_sync_timeout = STAT_SYNC_TIMEOUT_BCMSIM;
    } else {
        stat_sync_timeout = STAT_SYNC_TIMEOUT;
    }
    stat_sync_timeout = soc_property_get(unit,
                                         spn_BCM_STAT_SYNC_TIMEOUT,
                                         stat_sync_timeout);
    soc_timeout_init(&to, stat_sync_timeout, 0);
    while (soc->counter_sync_req) {
        if (soc_timeout_check(&to)) {
            if (soc->counter_sync_req) {
                soc_cm_debug(DK_ERR,
                             "soc_counter_sync: counter thread not responding\n");
                soc->counter_sync_req = FALSE;
                return SOC_E_TIMEOUT;
            }
        }

        sal_usleep(10000);
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_counter_stop
 * Purpose:
 *      Terminate the counter collection, S/W accumulation process.
 * Parameters:
 *      unit - StrataSwitch unit #.
 * Returns:
 *      SOC_E_XXX
 */

int
soc_counter_stop(int unit)
{
    soc_control_t       *soc = SOC_CONTROL(unit);
    int                 rv = SOC_E_NONE;
    soc_timeout_t       to;
    sal_usecs_t         cdma_timeout;
#ifdef BCM_CMICM_SUPPORT
    int cmc = SOC_PCI_CMC(unit);
#endif

    soc_cm_debug(DK_COUNTER, "soc_counter_stop: unit=%d\n", unit);

    if (SAL_BOOT_QUICKTURN) {
        cdma_timeout = CDMA_TIMEOUT_QT;
    } else if (SAL_BOOT_BCMSIM) {
        cdma_timeout = CDMA_TIMEOUT_BCMSIM;
    } else {
        cdma_timeout = CDMA_TIMEOUT;
    }
    cdma_timeout = soc_property_get(unit, spn_CDMA_TIMEOUT_USEC, cdma_timeout);

    /* Stop thread if present. */

    if (soc->counter_interval != 0) {
        sal_thread_t    sample_pid;

        /*
         * Signal by setting interval to 0, and wake up thread to speed
         * its exit.  It may also be waiting for the hardware interrupt
         * semaphore.  Wait a limited amount of time for it to exit.
         */

        soc->counter_interval = 0;

        sal_sem_give(soc->counter_intr);
        sal_sem_give(soc->counter_trigger);

        soc_timeout_init(&to, cdma_timeout, 0);

        while ((sample_pid = soc->counter_pid) != SAL_THREAD_ERROR) {
            if (soc_timeout_check(&to)) {
                soc_cm_debug(DK_ERR,
                             "soc_counter_stop: thread did not exit\n");
                soc->counter_pid = SAL_THREAD_ERROR;
                rv = SOC_E_TIMEOUT;
                break;
            }

            sal_usleep(10000);
        }
    }

    if (soc->counter_flags & SOC_COUNTER_F_DMA) {
        uint32          val;

        /*
         * Disable hardware counter scanning.
         * Turn off all ports to speed up final scan cycle.
         *
         * This cleanup is done here instead of at the end of the
         * counter thread so counter DMA will turn off even if the
         * thread is non responsive.
         */

#ifdef BCM_CMICM_SUPPORT
        if(soc_feature(unit, soc_feature_cmicm)) {

            soc_cmicm_intr0_disable(unit, IRQ_CMCx_STAT_ITER_DONE);

            val = soc_pci_read(unit, CMIC_CMCx_STAT_DMA_CFG_OFFSET(cmc));
            soc_reg_field_set(unit, CMIC_CMC0_STAT_DMA_CFGr, &val, ENf, 0);
            soc_reg_field_set(unit, CMIC_CMC0_STAT_DMA_CFGr, &val, E_Tf, 0);
            soc_pci_write(unit, CMIC_CMCx_STAT_DMA_CFG_OFFSET(cmc), val);
            soc_pci_write(unit, CMIC_CMCx_STAT_DMA_PORTS_0_OFFSET(cmc), 0);
            if (SOC_REG_IS_VALID(unit, CMIC_CMC0_STAT_DMA_PORTS_1r)) { 
                soc_pci_write(unit, CMIC_CMCx_STAT_DMA_PORTS_1_OFFSET(cmc), 0);
            }
            if (SOC_REG_IS_VALID(unit, CMIC_CMC0_STAT_DMA_PORTS_2r)) { 
                soc_pci_write(unit, CMIC_CMCx_STAT_DMA_PORTS_2_OFFSET(cmc), 0);
            }
        } else
#endif
        {

            soc_intr_disable(unit, IRQ_STAT_ITER_DONE);

            READ_CMIC_STAT_DMA_SETUPr(unit, &val);
            soc_reg_field_set(unit, CMIC_STAT_DMA_SETUPr, &val, ENf, 0);
            soc_reg_field_set(unit, CMIC_STAT_DMA_SETUPr, &val, E_Tf, 0);
            WRITE_CMIC_STAT_DMA_SETUPr(unit, val);
            if (SOC_IS_TUCANA(unit)) {
#ifdef  BCM_TUCANA_SUPPORT
                /* disable mmu stats collection */
                READ_CMIC_STAT_DMA_MMU_SETUPr(unit, &val);
                soc_reg_field_set(unit, CMIC_STAT_DMA_MMU_SETUPr, &val,
                                  MMU_STATS_ENf, 0);
                WRITE_CMIC_STAT_DMA_MMU_SETUPr(unit, val);
                WRITE_CMIC_STAT_DMA_MMU_PORTS0r(unit, 0);
                WRITE_CMIC_STAT_DMA_MMU_PORTS1r(unit, 0);

                WRITE_CMIC_STAT_DMA_PORTS_MOD0r(unit, 0);
                WRITE_CMIC_STAT_DMA_PORTS_MOD1r(unit, 0);
#endif  /* BCM_TUCANA_SUPPORT */
            } else {
                WRITE_CMIC_STAT_DMA_PORTSr(unit, 0);
#if defined(BCM_RAPTOR_SUPPORT)
                if (soc_feature(unit, soc_feature_register_hi)) {
                    WRITE_CMIC_STAT_DMA_PORTS_HIr(unit, 0);
                }
#endif /* BCM_RAPTOR_SUPPORT */
#if defined(BCM_TRIUMPH_SUPPORT)
                if (SOC_IS_TR_VL(unit)) {
                    WRITE_CMIC_STAT_DMA_PORTS_HIr(unit, 0);
                }
#endif /* BCM_TRIUMPH_SUPPORT */
            }
        }

        /*
         * Wait for STAT_DMA_ACTIVE to go away, with a timeout in case
         * it never does.
         */

        soc_cm_debug(DK_COUNTER, "soc_counter_stop: waiting for idle\n");

        soc_timeout_init(&to, cdma_timeout, 0);
#ifdef BCM_CMICM_SUPPORT
        if(soc_feature(unit, soc_feature_cmicm)) {
            while (soc_pci_read(unit, CMIC_CMCx_STAT_DMA_STAT_OFFSET(cmc)) & ST_CMCx_DMA_ACTIVE) {
                if (soc_timeout_check(&to)) {
                    rv = SOC_E_INTERNAL;
                    break;
                }
            }
        } else
#endif
        {
            while (soc_pci_read(unit, CMIC_DMA_STAT) & DS_STAT_DMA_ACTIVE) {
                if (soc_timeout_check(&to)) {
                    rv = SOC_E_TIMEOUT;
                    break;
                }
            }
        }
    }

#ifdef BCM_BRADLEY_SUPPORT
    if (soc_counter_tbuf[unit]) {
        sal_free(soc_counter_tbuf[unit]);
        soc_counter_tbuf[unit] = NULL;
    }
#endif /* BCM_BRADLEY_SUPPORT */

    if (NULL != soc->counter_intr) {
        sal_sem_destroy(soc->counter_intr);
        soc->counter_intr = NULL;
    }
    if (NULL != soc->counter_trigger) {
        sal_sem_destroy(soc->counter_trigger);
        soc->counter_trigger = NULL;
    }

    soc_cm_debug(DK_COUNTER, "soc_counter_stop: stopped\n");

    return (rv);
}

/*
 * Function:
 *      soc_counter_extra_register
 * Purpose:
 *      Register callback for additional counter collection.
 * Parameters:
 *      unit - The SOC unit number
 *      fn   - Callback function.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 */
int 
soc_counter_extra_register(int unit, soc_counter_extra_f fn)
{
    int i;

    if (fn == NULL) {
        return SOC_E_PARAM;
    }

    for (i = 0; i < SOC_COUNTER_EXTRA_CB_MAX; i++) {
        if (soc_counter_extra[unit][i] == fn) {
            return SOC_E_NONE;
        }
    }

    for (i = 0; i < SOC_COUNTER_EXTRA_CB_MAX; i++) {
        if (soc_counter_extra[unit][i] == NULL) {
            soc_counter_extra[unit][i] = fn;
            return SOC_E_NONE;
        }
    }

    return SOC_E_FULL;
}

/*
 * Function:
 *      soc_counter_extra_unregister
 * Purpose:
 *      Unregister callback for additional counter collection.
 * Parameters:
 *      unit - The SOC unit number
 *      fn   - Callback function.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 */
int
soc_counter_extra_unregister(int unit, soc_counter_extra_f fn)
{
    int i;

    if (fn == NULL) {
        return SOC_E_PARAM;
    }

    for (i = 0; i < SOC_COUNTER_EXTRA_CB_MAX; i++) {
        if (soc_counter_extra[unit][i] == fn) {
            soc_counter_extra[unit][i] = NULL;
            return SOC_E_NONE;
        }
    }

    return SOC_E_NOT_FOUND;
}

int
soc_counter_timestamp_get(int unit, soc_port_t port,
                          uint32 *timestamp)
{
    soc_control_t       *soc = SOC_CONTROL(unit);
    int rv = SOC_E_NOT_FOUND;

    if (!soc_feature(unit, soc_feature_timestamp_counter)) {
        return rv;
    }

    if (soc->counter_timestamp_fifo[port] == NULL) {
        return rv;
    }

    COUNTER_LOCK(unit);
    if (!SHR_FIFO_IS_EMPTY(soc->counter_timestamp_fifo[port])) {
        SHR_FIFO_POP(soc->counter_timestamp_fifo[port], timestamp);
        rv = SOC_E_NONE;
    }
    COUNTER_UNLOCK(unit);
    return rv;
}

#endif /* defined(BCM_ESW_SUPPORT) || defined(BCM_SIRIUS_SUPPORT)*/
