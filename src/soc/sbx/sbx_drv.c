/*
 * $Id: sbx_drv.c 1.261.2.3 Broadcom SDK $
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
 * NOTE:
 * SOC driver infrastructure cleanup pending.
 */

#include <soc/debug.h>
#include <soc/error.h>
#include <soc/types.h>
#include <soc/property.h>
#include <soc/drv.h>
#include <soc/ipoll.h>
#include <soc/i2c.h>
#include <soc/mcm/driver.h>     /* soc_base_driver_table */
#include <soc/mem.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/phyctrl.h>
#include <soc/sbx/counter.h>

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/qe2000.h>
#include <soc/sbx/qe2000_init.h>
#include <soc/sbx/bme3200.h>
#include <soc/sbx/bm3200_init.h>
#include <soc/sbx/bm9600.h>
#include <soc/sbx/bm9600_soc_init.h>
#include <soc/sbx/bm9600_init.h>
#include <soc/sbx/fe2000.h>
#include <soc/sbx/sirius.h>
#include <soc/sbx/hal_ka_auto.h>
#include <soc/sbx/hal_ca_auto.h>
#include <soc/sbx/hal_c2_auto.h>
#include <soc/sbx/hal_ca_c2.h>
#include <soc/sbx/fe2k_common/sbFe2000Common.h>
#include <soc/sbx/qe2000_mvt.h>

#include <bcm/stack.h>
#include <bcm/cosq.h>
#include <sal/types.h>
#include <sal/compiler.h>
#include <sal/appl/io.h>
#include <sal/core/dpc.h>

#ifdef BCM_CMICM_SUPPORT
#include <soc/cmicm.h>
#endif

/*
 * The table of all known drivers
 * Indexed by supported chips
 */

#if  !defined(BCM_QE2000_A1) || !defined(BCM_QE2000_A2) \
   || !defined(BCM_QE2000_A3) || !defined(BCM_BME3200_B0) \
   || !defined(BCM_FE2000_A0) || !defined(BCM_BME3200_A0) \
   || !defined(BCM_BM9600_A0) || !defined(BCM_BM9600_A1) \
   || !defined(BCM_BM9600_B0) || !defined(BCM_88230_A0) \
   || !defined(BCM_88230_B0)  || !defined(BCM_88230_C0)
static soc_driver_t soc_sbx_driver_none;
#endif

soc_driver_t *soc_sbx_driver_table[] = {
#if defined(BCM_QE2000_A1) || defined(BCM_QE2000_A2) || defined(BCM_QE2000_A3)
    &soc_driver_bcm83200_a3,
#else
    &soc_sbx_driver_none,
#endif

#if defined(BCM_BME3200_A0)
    &soc_driver_bcm83332_a0,
#else
    &soc_sbx_driver_none,
#endif

#if defined(BCM_BME3200_B0)
    &soc_driver_bcm83332_b0,
#else
    &soc_sbx_driver_none,
#endif

#if defined(BCM_FE2000_A0)
    &soc_driver_bcm88020_a0,
#else
    &soc_sbx_driver_none,
#endif

#if defined(BCM_FE2000_A0)
    &soc_driver_bcm88025_a0,
#else
    &soc_sbx_driver_none,
#endif

#if defined(BCM_BM9600_A0) || defined(BCM_BM9600_A1)
    &soc_driver_bm9600_a0,
#else
    &soc_sbx_driver_none,
#endif

#if defined(BCM_BM9600_B0)
    &soc_driver_bm9600_b0,
#else
    &soc_sbx_driver_none,
#endif

#if defined(BCM_88230_A0)
    &soc_driver_bcm88230_a0,
#else
    &soc_sbx_driver_none,
#endif

#if defined(BCM_88230_B0)
    &soc_driver_bcm88230_b0,
#else
    &soc_sbx_driver_none,
#endif

#if defined(BCM_88230_C0)
    &soc_driver_bcm88230_c0,
#else
    &soc_sbx_driver_none,
#endif

};

int
soc_sbx_misc_init(int unit)
{
    return SOC_E_NONE;
}

int
soc_sbx_mmu_init(int unit)
{
    return SOC_E_NONE;
}

soc_functions_t soc_sbx_drv_funs = {
    soc_sbx_misc_init,
    soc_sbx_mmu_init,
    NULL,
    NULL,
    NULL,
};

int gu2_verbose = 0;

/* logging function used by the ilib and fabric */
void _sb_log_function(int32_t uLevel, const char *format, ...)
{
  if( gu2_verbose >= uLevel) {
    va_list argList;
    va_start(argList, format);
    soc_cm_vprint(format, argList);
    va_end(argList);
  }
}

volatile int __soc_sbx_sirius_state_init = FALSE;

/*
 * Microcode version strings
 */
char *soc_sbx_ucode_versions[SOC_SBX_MAX_UCODE_TYPE] =
    {NULL, NULL, NULL};

static int soc_sbx_connect_min_util[SBX_MAX_FABRIC_COS] =
{
  100, 100, 100, 100, 100, 100, 100, 100,
  100, 100, 100, 100, 100, 100, 100, 100
};

static int soc_sbx_connect_max_age_time[SBX_MAX_FABRIC_COS] =
{
   32, 240, 240, 240, 240, 240, 240, 240,
  240, 240, 240, 240, 240, 240, 240, 240
};

extern void soc_sbx_intr(void *_unit);

#define UPPER32(x) ((uint32_t)((((uint64_t)(x))>>32) & 0xffffffffU))
#define LOWER32(x) ((uint32_t)(((uint64_t)(x)) & 0xffffffffU))

int soc_sbx_div64(uint64_t x, uint32_t y, uint32_t *result)
{
    uint64_t rem;
    uint64_t b;
    uint64_t res, d;
    uint32_t high;
    uint32_t low;

    COMPILER_64_SET(rem, COMPILER_64_HI(x), COMPILER_64_LO(x));
    COMPILER_64_SET(b, 0, y);
    COMPILER_64_SET(d, 0, 1);

    high = COMPILER_64_HI(rem);
    low = COMPILER_64_LO(rem);

    COMPILER_64_ZERO(res);
    if (high >= y) {
        /* NOTE: Follow code is used to handle 64bits result
         *  high /= y;
         *  res = (uint64_t) (high << 32);
         *  rem -= (uint64_t)((high * y) << 32);
         */
        SOC_ERROR_PRINT((DK_ERR,"soc_sbx_div64: result > 32bits\n"));
        return SOC_E_PARAM;
    }

    while ((!COMPILER_64_BITTEST(b, 63)) &&
	   (COMPILER_64_LT(b, rem)) ) {
	COMPILER_64_ADD_64(b,b);
	COMPILER_64_ADD_64(d,d);
    }

    do {
	if (COMPILER_64_GE(rem, b)) {
	    COMPILER_64_SUB_64(rem, b);
	    COMPILER_64_ADD_64(res, d);
	}
	COMPILER_64_SHR(b, 1);
	COMPILER_64_SHR(d, 1);
    } while (!COMPILER_64_IS_ZERO(d));

    *result = COMPILER_64_LO(res);

    /*
    SOC_DEBUG_PRINT((DK_VERBOSE,
                     "%s: divisor 0x%x%8.8x dividor 0x%x result 0x%x\n",
                     FUNCTION_NAME(), high, low, y, *result));
    */
    return 0;
}

#if defined(BCM_FE2000_A0)
char* soc_sbx_fte_segment_names[] = SOC_SBX_FTE_SEGMENT_STRINGS;
#endif

/* logic for node/port to qid macros added to ease testing config settings */

int map_np_to_qid(int unit, int node, int port, int numcos) {
   int qid = 0;
   char *ucodestr = NULL;

   /* NOTE: standalone feature must be global, so must be consistent for all units */
    if (soc_feature(0, soc_feature_standalone)) {
      if (SOC_IS_SBX_QE2000(unit)) {
	/* only QE2000 TME forced to be 16 queues per logical port */
	qid = (SBX_UC_QID_BASE + (((node) * SBX_MAX_PORTS + (port)) << 4 ));
      } else if (SOC_IS_SBX_FE2KXT(unit)) {
	/* special case for c2-sirius interop */
	ucodestr = soc_property_get_str(unit,spn_BCM88025_UCODE);
	if (ucodestr != NULL && (sal_strcmp(ucodestr, "c2ss_g2p3") == 0)) {
          qid = (SBX_UC_QID_BASE + (((node) * SBX_MAX_PORTS + (port)) << (_BITS(numcos))));
        } else {
	  qid = (SBX_UC_QID_BASE + (((node) * SBX_MAX_PORTS + (port)) << 4 ));
	}
      } else {
	qid = (SBX_UC_QID_BASE + (((node) * SBX_MAX_PORTS + (port)) << (_BITS(numcos))));
      }
    }
    else {
      /* for Polaris chasses - should be no-op otherwise */
       node = (node % 32);
           qid = (SBX_UC_QID_BASE + (((node) * SBX_MAX_PORTS + (port)) << (_BITS(numcos))));
       }
   return qid;
}

int map_qid_to_np(int unit, int qid, int *node, int *port, int numcos) {
   int qidoffs=0;
   char *ucodestr = NULL;

   /* NOTE: standalone feature must be global, so must be consistent for all units */
    if (soc_feature(0, soc_feature_standalone)) {
      if (SOC_IS_SBX_QE2000(unit)) {
	/* only QE2000 TME forced to be 16 queues per logical port */
	qidoffs =  (((qid) - SBX_UC_QID_BASE) >> 4);
      } else if (SOC_IS_SBX_FE2KXT(unit)) {
	/* special case for c2-sirius interop */
	ucodestr = soc_property_get_str(unit,spn_BCM88025_UCODE);
	if (ucodestr != NULL && (sal_strcmp(ucodestr, "c2ss_g2p3") == 0)) {
	  qidoffs =  (((qid) - SBX_UC_QID_BASE) >> (_BITS(numcos)));
        } else {
	  qidoffs =  (((qid) - SBX_UC_QID_BASE) >> 4);
	}
      } else {
	qidoffs =  (((qid) - SBX_UC_QID_BASE) >> (_BITS(numcos)));
      }
    }
    else {
      qidoffs =  (((qid) - SBX_UC_QID_BASE) >> (_BITS(numcos)));
    }
    *node = qidoffs / SBX_MAX_PORTS;
    *port = qidoffs % SBX_MAX_PORTS;
   return 0;
}

int map_ds_id_to_qid(int unit, int ds_id, int numcos) {
    int qid;

    if (soc_feature(0, soc_feature_standalone)) {
        qid = SBX_MC_QID_BASE + (ds_id << 4);
    }
    else {
        qid = SBX_MC_QID_BASE + (ds_id << ((_BITS(numcos) > _BITS(SBX_MAX_COS) ? 3 : _BITS(numcos))));
    }
    return qid;
}

int map_qid_to_ds_id(int unit, int qid, int numcos) {
    int ds_id;

    if (soc_feature(0, soc_feature_standalone)) {
        ds_id = (qid >> 4) - SBX_MC_QID_BASE;
    }
    else {
        ds_id = (qid >> ((_BITS(numcos) > _BITS(SBX_MAX_COS) ? 3 : _BITS(numcos)))) - SBX_MC_QID_BASE;
    }
    return ds_id;
}

int
soc_sbx_hybrid_demarcation_qid_get(int unit)
{
    int qid;


    qid = (SOC_SBX_CFG(unit)->nMaxFabricPortsOnModule + 1) *
                           (SOC_SBX_CFG(unit)->cfg_num_nodes) *
                           soc_property_get(unit, spn_BCM_NUM_COS, BCM_COS_COUNT);

    return(qid);
}

int
soc_sbx_translate_status(sbStatus_t s)
{
    int rv;

    switch (s) {
    case SB_OK:
        rv = SOC_E_NONE;
        break;
    case SB_MAC_DUP:
    case SB_SVID_DUP:
    case SB_LPM_DUPLICATE_ADDRESS:
        rv = SOC_E_EXISTS;
        break;
    case SB_MAC_FULL:
    case SB_MAC_NO_MEM:
    case SB_MAC_COL:
    case SB_SVID_TOO_MANY_ENTRIES:
    case SB_SVID_COL:
    case SB_LPM_OUT_OF_HOST_MEMORY:
    case SB_LPM_OUT_OF_DEVICE_MEMORY:
        rv = SOC_E_FULL;
        break;
    case SB_MAC_NOT_FOUND:
    case SB_SVID_KEY_NOT_FOUND:
    case SB_LPM_ADDRESS_NOT_FOUND:
        rv = SOC_E_NOT_FOUND;
        break;
    case SB_TIMEOUT_ERR_CODE:
        rv = SOC_E_TIMEOUT;
        break;
    default:
        rv = SOC_E_FAIL;
    }

    return rv;
}


int
soc_sbx_info_config(int unit, int dev_id, int drv_dev_id)
{
    soc_info_t          *si;
    soc_sbx_control_t   *sbx;
    soc_control_t       *soc;
#if defined(BCM_SIRIUS_SUPPORT)
    int                 blktype, blk, mem;
    int                 instance;
#endif

    sbx = SOC_SBX_CONTROL(unit);
    soc = SOC_CONTROL(unit);

    si  = &SOC_INFO(unit);
    si->driver_type = soc->chip_driver->type;
    si->driver_group = soc_chip_type_map[si->driver_type];

    si->fe.min          = si->fe.max          = -1;
    si->ge.min          = si->ge.max          = -1;
    si->xe.min          = si->xe.max          = -1;
    si->hg.min          = si->hg.max          = -1;
    si->hg_subport.min  = si->hg_subport.max  = -1;
    si->hl.min          = si->hl.max          = -1;
    si->st.min          = si->st.max          = -1;
    si->gx.min          = si->gx.max          = -1;
    si->xg.min          = si->xg.max          = -1;
    si->spi.min         = si->spi.max         = -1;
    si->spi_subport.min = si->spi_subport.max = -1;
    si->sci.min         = si->sci.max         = -1;
    si->sfi.min         = si->sfi.max         = -1;
    si->port.min        = si->port.max        = -1;
    si->ether.min       = si->ether.max       = -1;
    si->all.min         = si->all.max         = -1;

    SOC_PBMP_CLEAR(si->cmic_bitmap);

    /*
     * Used to implement the SOC_IS_*(unit) macros
     */
    switch (drv_dev_id) {
#if defined(BCM_QE2000_SUPPORT)
    case QE2000_DEVICE_ID:
        si->chip_type = SOC_INFO_QE2000;
        si->modid_max = 10031;
        sbx->fabtype = SB_FAB_DEVICE_QE2000;
        SOC_CHIP_STRING(unit) = "qe2000";
        sbx->modid_count = 1;
        sbx->init_func = soc_qe2000_init;
        sbx->detach = soc_sbx_qe2000_detach;
        sbx->isr = soc_qe2000_isr;

        soc_qe2000_port_info_config(unit);

        sbx->tx_ring_reg =
            SAND_HAL_KA_PC_TX_RING_PTR_OFFSET;
        sbx->tx_ring_size_reg =
            SAND_HAL_KA_PC_TX_RING_SIZE_OFFSET;
        sbx->tx_ring_producer_reg =
            SAND_HAL_KA_PC_TX_RING_PRODUCER_OFFSET;
        sbx->tx_ring_consumer_reg =
            SAND_HAL_KA_PC_TX_RING_CONSUMER_OFFSET;
        sbx->completion_ring_reg =
            SAND_HAL_KA_PC_COMPLETION_RING_PTR_OFFSET;
        sbx->completion_ring_size_reg =
            SAND_HAL_KA_PC_COMPLETION_RING_SIZE_OFFSET;
        sbx->completion_ring_producer_reg =
            SAND_HAL_KA_PC_COMPLETION_RING_PRODUCER_OFFSET;
        sbx->completion_ring_consumer_reg =
            SAND_HAL_KA_PC_COMPLETION_RING_CONSUMER_OFFSET;
        sbx->rxbuf_size_reg =
            SAND_HAL_KA_PC_RXBUF_SIZE_OFFSET;
        sbx->rxbuf_load_reg =
            SAND_HAL_KA_PC_RXBUF_LOAD0_OFFSET;
        sbx->rxbufs_pop_reg =
            SAND_HAL_KA_PC_RXBUF_FIFO_DEBUG_OFFSET;
        sbx->rxbufs_pop_bit =
            SAND_HAL_KA_PC_RXBUF_FIFO_DEBUG_POP_FIFO_MASK;
        break;
#endif /* BCM_QE2000_SUPPORT */

#if defined(BCM_BME3200_SUPPORT)
    case BME3200_DEVICE_ID:
        si->chip_type = SOC_INFO_BME3200;
        sbx->fabtype = SB_FAB_DEVICE_BM3200;
        SOC_CHIP_STRING(unit) = "bme3200";
        sbx->modid_count = 0;
        sbx->init_func = soc_bm3200_init;
        sbx->isr = soc_bm3200_isr;
        sbx->detach = NULL;

        soc_bm3200_port_info_config(unit);

        break;
#endif /* BCM_BME3200_SUPPORT */
/* MCM added bm9600 section */
#if defined(BCM_BM9600_SUPPORT)
    case BM9600_DEVICE_ID:
        si->chip_type = SOC_INFO_BM9600;
        sbx->fabtype = SB_FAB_DEVICE_BM9600;
        SOC_CHIP_STRING(unit) = "bm9600";
        sbx->modid_count = 0;
        sbx->init_func = soc_bm9600_init;
        sbx->isr = soc_bm9600_isr;
        sbx->detach = NULL;

        soc_bm9600_port_info_config(unit);

        break;
#endif /* BCM_BM9600_SUPPORT */

#if defined(BCM_FE2000_SUPPORT)
    case BCM88020_DEVICE_ID:
        si->chip_type = SOC_INFO_FE2000;
        si->modid_max = 31;
        SOC_CHIP_STRING(unit) = "fe2000";
        sbx->modid_count = 1;
        sbx->fetype = SOC_SBX_FETYPE_FE2K;

        sbx->init_func = soc_sbx_fe2000_init;
        sbx->detach = soc_sbx_fe2000_detach;
        sbx->isr = soc_sbx_fe2000_isr;
        si->sci.num = si->sfi.num = 0;

        soc_sbx_fe2000_port_info_config(unit);

        sbx->tx_ring_reg =
            SAND_HAL_FE2000_REG_OFFSET(unit, PC_TX_RING_PTR);
        sbx->tx_ring_size_reg =
            SAND_HAL_FE2000_REG_OFFSET(unit, PC_TX_RING_SIZE);
        sbx->tx_ring_producer_reg =
            SAND_HAL_FE2000_REG_OFFSET(unit, PC_TX_RING_PRODUCER);
        sbx->tx_ring_consumer_reg =
            SAND_HAL_FE2000_REG_OFFSET(unit, PC_TX_RING_CONSUMER);
        sbx->completion_ring_reg =
            SAND_HAL_FE2000_REG_OFFSET(unit, PC_COMPLETION_RING_PTR);
        sbx->completion_ring_size_reg =
            SAND_HAL_FE2000_REG_OFFSET(unit, PC_COMPLETION_RING_SIZE);
        sbx->completion_ring_producer_reg =
            SAND_HAL_FE2000_REG_OFFSET(unit, PC_COMPLETION_RING_PRODUCER);
        sbx->completion_ring_consumer_reg =
            SAND_HAL_FE2000_REG_OFFSET(unit, PC_COMPLETION_RING_CONSUMER);
        sbx->rxbuf_size_reg =
            SAND_HAL_FE2000_REG_OFFSET(unit, PC_RXBUF_SIZE);
        sbx->rxbuf_load_reg =
            SAND_HAL_FE2000_REG_OFFSET(unit, PC_RXBUF_LOAD0);
        sbx->rxbufs_pop_reg =
            SAND_HAL_FE2000_REG_OFFSET(unit, PC_RXBUF_FIFO_DEBUG);
        sbx->rxbufs_pop_bit =
            SAND_HAL_FE2000_FIELD_MASK(unit, PC_RXBUF_FIFO_DEBUG, POP_FIFO);

        break;
    case BCM88025_DEVICE_ID:
        si->chip_type = SOC_INFO_FE2000XT;
        si->modid_max = 31;
        /* the string description of the FE2000XT must be kept the same as the
         * description of the FE2000 as these share some BCM layer data structures
         * which are located uisng this information.*/
        SOC_CHIP_STRING(unit) = "fe2000";
        sbx->modid_count = 1;
        sbx->fetype = SOC_SBX_FETYPE_FE2KXT;

        sbx->init_func = soc_sbx_fe2000xt_init;
        sbx->detach = soc_sbx_fe2000_detach;
        sbx->isr = soc_sbx_fe2000_isr;
        si->sci.num = si->sfi.num = 0;

        soc_sbx_fe2000_port_info_config(unit);

        sbx->tx_ring_reg =
            SAND_HAL_FE2000_REG_OFFSET(unit, PC_TX_RING_PTR);
        sbx->tx_ring_size_reg =
            SAND_HAL_FE2000_REG_OFFSET(unit, PC_TX_RING_SIZE);
        sbx->tx_ring_producer_reg =
            SAND_HAL_FE2000_REG_OFFSET(unit, PC_TX_RING_PRODUCER);
        sbx->tx_ring_consumer_reg =
            SAND_HAL_FE2000_REG_OFFSET(unit, PC_TX_RING_CONSUMER);
        sbx->completion_ring_reg =
            SAND_HAL_FE2000_REG_OFFSET(unit, PC_COMPLETION_RING_PTR);
        sbx->completion_ring_size_reg =
            SAND_HAL_FE2000_REG_OFFSET(unit, PC_COMPLETION_RING_SIZE);
        sbx->completion_ring_producer_reg =
            SAND_HAL_FE2000_REG_OFFSET(unit, PC_COMPLETION_RING_PRODUCER);
        sbx->completion_ring_consumer_reg =
            SAND_HAL_FE2000_REG_OFFSET(unit, PC_COMPLETION_RING_CONSUMER);
        sbx->rxbuf_size_reg =
            SAND_HAL_FE2000_REG_OFFSET(unit, PC_RXBUF_SIZE);
        sbx->rxbuf_load_reg =
            SAND_HAL_FE2000_REG_OFFSET(unit, PC_RXBUF_LOAD0);
        sbx->rxbufs_pop_reg =
            SAND_HAL_FE2000_REG_OFFSET(unit, PC_RXBUF_FIFO_DEBUG);
        sbx->rxbufs_pop_bit =
            SAND_HAL_FE2000_FIELD_MASK(unit, PC_RXBUF_FIFO_DEBUG, POP_FIFO);

        break;
#endif /* BCM_FE2000_SUPPORT */


#if defined(BCM_SIRIUS_SUPPORT)
    case BCM88230_DEVICE_ID:
        si->chip_type = SOC_INFO_SIRIUS;
        si->chip = SOC_INFO_SIRIUS;
        si->modid_max = 10071;
        si->num_cpu_cosq = 48;
        sbx->fabtype = SB_FAB_DEVICE_SIRIUS;
        SOC_CHIP_STRING(unit) = "sirius";
        sbx->modid_count = 1;
        sbx->init_func = soc_sirius_init;
        sbx->detach = soc_sirius_detach;
        sbx->isr = soc_intr;

        
        switch (dev_id) {
        case BCM88235_DEVICE_ID:
          /* 80Gps/120Mps device, 4HG */
          break;
        case BCM88230_DEVICE_ID:
          /* 50Gps/75Mps device, 4HG */
          break;
        case BCM88239_DEVICE_ID:
          /* 20Gps/30Mps device, 2HG */
          break;
        case BCM56613_DEVICE_ID:
          /* 80Gps/120Mps device, 4HG with limited queue/hqos/buffer etc */
          break;
        default:
          break;
        }
        soc_sirius_port_info_config(unit, drv_dev_id, dev_id);

        break;
#endif /* BCM_SIRIUS_SUPPORT */

    default:
        si->chip_type = 0;
        SOC_CHIP_STRING(unit) = "???";
        sbx->modid_count = 0;
        SOC_ERROR_PRINT((DK_WARN,
                         "soc_sbx_info_config: driver device %04x unexpected\n",
                         drv_dev_id));
        break;
    }

#if defined(BCM_SIRIUS_SUPPORT)
    if (SOC_IS_SBX_SIRIUS(unit)) {
        /* config for reg/mem access through cmic interface */
        si->arl_block = -1;
        si->mmu_block = -1;
        si->mcu_block = -1;

        si->ipipe_block = -1;
        si->ipipe_hi_block = -1;
        si->epipe_block = -1;
        si->epipe_hi_block = -1;
        si->bsafe_block = -1;
        si->esm_block = -1;
        si->otpc_block = -1;

        si->igr_block = -1;
        si->egr_block = -1;
        si->bse_block = -1;
        si->cse_block = -1;
        si->hse_block = -1;

        si->qma_block = -1;
        si->qmb_block = -1;
        si->qmc_block = -1;
        si->bp_block = -1;
        si->cs_block = -1;
        si->eb_block = -1;
        si->ep_block = -1;
        si->es_block = -1;
        si->fd_block = -1;
        si->ff_block = -1;
        si->fr_block = -1;
        si->tx_block = -1;
        si->qsa_block = -1;
        si->qsb_block = -1;
        si->rb_block = -1;
        si->sc_top_block = -1;
        si->sf_top_block = -1;
        si->ts_block = -1;
        si->otpc_block = -1;

        sal_memset(si->has_block, 0, sizeof(soc_block_t) * COUNTOF(si->has_block));

        for (blk = 0; blk < SOC_MAX_NUM_CI_BLKS; blk++) {
            si->ci_block[blk] = -1;
        }

        for (blk = 0; blk < SOC_MAX_NUM_XPORT_BLKS; blk++) {
            si->xport_block[blk] = -1;
        }

        for (blk = 0; blk < SOC_MAX_NUM_CLPORT_BLKS; blk++) {
            si->clport_block[blk] = -1;
        }

        si->xlport_block = -1;

        for (blk = 0; blk < SOC_MAX_NUM_ILPORT_BLKS; blk++) {
            si->il_block[blk] = -1;
        }

        for (blk = 0; blk < SOC_MAX_NUM_TMU_BLKS; blk++) {
            si->tm_block[blk] = -1;
        }

        for (blk = 0; blk < SOC_MAX_NUM_COP_BLKS; blk++) {
            si->co_block[blk] = -1;
        }

        for (blk = 0; blk < SOC_MAX_NUM_PR_BLKS; blk++) {
            si->pr_block[blk] = -1;
        }

        for (blk = 0; blk < SOC_MAX_NUM_PT_BLKS; blk++) {
            si->pt_block[blk] = -1;
        }


        for (blk = 0; SOC_BLOCK_INFO(unit, blk).type >= 0; blk++) {
            blktype = SOC_BLOCK_INFO(unit, blk).type;
            instance = SOC_BLOCK_INFO(unit, blk).number;
            si->has_block[blk] = blktype;
            switch (blktype) {
                case SOC_BLK_CMIC:
                    si->cmic_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_BP:
                    si->bp_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_CI:
                    si->ci_block[instance] = blk;
                    si->block_valid[blk] += 1;
                    si->block_port[blk] = (SOC_REG_ADDR_INSTANCE_MASK | instance);
                    SOC_PBMP_PORT_ADD(si->block_bitmap[blk], instance);
                    break;
                case SOC_BLK_CS:
                    si->cs_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_EB:
                    si->eb_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_EP:
                    si->ep_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_ES:
                    si->es_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_FD:
                    si->fd_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_FF:
                    si->ff_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_FR:
                    si->fr_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_GXPORT:
                    si->xport_block[instance] = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_TX:
                    si->tx_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_QMA:
                    si->qma_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_QMB:
                    si->qmb_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_QMC:
                    si->qmc_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_QSA:
                    si->qsa_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_QSB:
                    si->qsb_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_RB:
                    si->rb_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_SC_TOP:
                    si->sc_top_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_SF_TOP:
                    si->sf_top_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_TS:
                    si->ts_block = blk;
                    si->block_valid[blk] += 1;
                    break;
                case SOC_BLK_OTPC:
                    si->otpc_block = blk;
                    si->block_valid[blk] += 1;
                    break;
            }
            sal_snprintf(si->block_name[blk], sizeof(si->block_name[blk]),
                         "%s%d",
                         soc_block_name_lookup_ext(blktype, unit),
                         SOC_BLOCK_INFO(unit, blk).number);
        }
        si->block_num = blk;

        /*
         * Calculate the mem_block_any array for this configuration
         * The "any" block is just the first one enabled
         */
        for (mem = 0; mem < NUM_SOC_MEM; mem++) {
            si->mem_block_any[mem] = -1;
            if (SOC_MEM_IS_VALID(unit, mem)) {
                SOC_MEM_BLOCK_ITER(unit, mem, blk) {
                    si->mem_block_any[mem] = blk;
                    break;
                }
            }
        }


    }
#endif /* defined(BCM_SIRIUS_SUPPORT)*/

    return SOC_E_NONE;
}


soc_driver_t *
soc_sbx_chip_driver_find(uint16 pci_dev_id, uint8 pci_rev_id)
{
    int                 i;
    soc_driver_t        *d;

    /*
     * Find driver in table.  In theory any IDs returned by
     * soc_cm_id_to_driver_id() should have a driver in the table.
     */
    for (i = 0; i < SOC_SBX_NUM_SUPPORTED_CHIPS; i++) {
         d = soc_sbx_driver_table[i];
         if ((d != NULL) &&
             (d->pci_device == pci_dev_id) &&
             (d->pci_revision == pci_rev_id)) {
             return d;
         }
    }

    SOC_ERROR_PRINT((DK_ERR,
                     "soc_sbx_chip_driver_find: driver in devid table "
                     "not in soc_sbx_driver_table\n"));

    return NULL;
}

void
soc_sbx_get_default(int unit, soc_sbx_config_t *cfg)
{
    int                   spi, port=0, node, hg, intf;
    soc_sbx_control_t    *sbx;
    int                   cos;
    int                   level;
    int16                 fifo=0, group=0;

    assert(SOC_CONTROL(unit) != NULL);
    assert(SOC_SBX_CONTROL(unit) != NULL);
    assert(SOC_SBX_CFG(unit) != NULL);

    sbx = SOC_SBX_CONTROL(unit);

    /* Global config */
    SOC_SBX_CFG(unit)->DeviceHandle        = sbx->sbhdl;
    SOC_SBX_CFG(unit)->reset_ul            = 1;           /* default to reset */
    SOC_SBX_CFG(unit)->uClockSpeedInMHz    = 0;           /* default to 0 */
    SOC_SBX_CFG(unit)->uFabricConfig       = SOC_SBX_SYSTEM_CFG_INVALID;
                                                          /* default to invalid */
    SOC_SBX_CFG(unit)->bHalfBus            = FALSE;       /* default to full bus mode */
    SOC_SBX_CFG(unit)->bRunSelfTest        = TRUE;        /* default to run self test */
    SOC_SBX_CFG(unit)->uActiveScId         = 0;           /* default to 0 ACTIVE_SWITCH_CONTROLLER */
    SOC_SBX_CFG(unit)->uLinkThresholdIndex = 89;
    SOC_SBX_CFG(unit)->uRedMode            = 0;           /* default to be manual */
    SOC_SBX_CFG(unit)->uMaxFailedLinks     = 2;           /* default to allow 2 links fail */
    SOC_SBX_CFG(unit)->bHybridMode         = FALSE;       /* default to non-hybrid mode */
    SOC_SBX_CFG(unit)->bUcqResourceAllocationMode = FALSE;/* default resource creation mode */
    SOC_SBX_CFG(unit)->bEgressFifoIndependentFlowControl = FALSE;
    SOC_SBX_CFG(unit)->bEgressMulticastFifoIndependentFlowControl = FALSE;
    SOC_SBX_CFG(unit)->nDiscardTemplates   = 0;
    SOC_SBX_CFG(unit)->discard_probability_mtu = SOC_SBX_DISCARD_PROBABILITY_MTU_SZ;
    SOC_SBX_CFG(unit)->discard_queue_size = SOC_SBX_DISCARD_QUEUE_SZ;
    SOC_SBX_CFG(unit)->nShaperCount        = 0;           /* default to no Shapers, override if available */
    SOC_SBX_CFG(unit)->bcm_cosq_init       = FALSE;        /* default to initialize VOQs */
    SOC_SBX_CFG(unit)->bTmeMode = FALSE;                  /* Non-TME by default */
    SOC_SBX_CFG(unit)->fabric_egress_setup = TRUE;        /* default creation of egress groups to True */
    SOC_SBX_CFG(unit)->erh_type = 0;
    SOC_SBX_CFG(unit)->epoch_length_in_timeslots = SB_FAB_DMODE_EPOCH_IN_TIMESLOTS;
    SOC_SBX_CFG(unit)->xbar_link_en        = 0; /* fabric disabled */
    SOC_SBX_CFG(unit)->mcgroup_local_start_index = 0;
    SOC_SBX_CFG(unit)->max_pkt_len_adj_sel = 0;
    SOC_SBX_CFG(unit)->max_pkt_len_adj_value = 0;
    SOC_SBX_CFG(unit)->enable_all_egress_nodes = FALSE;
    SOC_SBX_CFG(unit)->nMaxFabricPortsOnModule = 32;     /* hybrid mode & FE FTE configuration */
    SOC_SBX_CFG(unit)->is_demand_scale_fixed = FALSE;    /* default no SOC property */
    SOC_SBX_CFG(unit)->fixed_demand_scale = -1;
    SOC_SBX_CFG(unit)->demand_scale = 0;                 /* default to demand scale 0 bits */
    SOC_SBX_CFG(unit)->sp_mode = SOC_SBX_SP_MODE_IN_BAG;
    SOC_SBX_CFG(unit)->local_template_id = SOC_SBX_QOS_TEMPLATE_TYPE0;
    SOC_SBX_CFG(unit)->node_template_id = SOC_SBX_NODE_QOS_TEMPLATE_TYPE0;
    SOC_SBX_CFG(unit)->v4_ena = 1;
    SOC_SBX_CFG(unit)->oam_rx_ena = 0;
    SOC_SBX_CFG(unit)->oam_tx_ena = 0;
    SOC_SBX_CFG(unit)->oam_spi_lb_port = 0;
    SOC_SBX_CFG(unit)->oam_spi_lb_queue = 0;
    SOC_SBX_CFG(unit)->v4mc_str_sel = 0;
    SOC_SBX_CFG(unit)->v4uc_str_sel = 0;
    SOC_SBX_CFG(unit)->uInterfaceProtocol = SOC_SBX_IF_PROTOCOL_XGS;
    SOC_SBX_CFG(unit)->arbitration_port_allocation = SOC_SBX_SYSTEM_ARBITRATION_PORT_ALLOCATION1;


    /* These values are system maximums for fabric devices.
     * Default this maximums to those in DMode. Later on,
     * these maximums will be adjusted based on the system mode
     */
    SOC_SBX_CFG(unit)->num_nodes = SB_FAB_DEVICE_BM3200_MAX_NODES;
    SOC_SBX_CFG(unit)->num_ds_ids = SB_FAB_DEVICE_BM3200_MAX_DS_IDS;
    SOC_SBX_CFG(unit)->num_internal_ds_ids = SB_FAB_DEVICE_BM3200_MAX_DS_IDS;
    SOC_SBX_CFG(unit)->num_queues = HW_QE2000_MAX_QUEUES;
    SOC_SBX_CFG(unit)->num_bw_groups = HW_BM3200_PT_MAX_DMODE_VIRTUAL_PORTS;
    SOC_SBX_CFG(unit)->num_sysports = 0;
    SOC_SBX_CFG(unit)->use_extended_esets = 0;
    SOC_SBX_CFG(unit)->max_ports = SB_FAB_DEVICE_QE2000_MAX_PORT;

    for (cos=0; cos<SBX_MAX_FABRIC_COS; cos++) {
        SOC_SBX_CFG(unit)->bcm_cosq_priority_mode[cos] = BCM_COSQ_BE; /* best effort */
        SOC_SBX_CFG(unit)->bcm_cosq_priority_min_bw_kbps[cos] = 0; /* no bw_group (bag) rate */
        SOC_SBX_CFG(unit)->bcm_cosq_priority_max_bw_kbps[cos] = 0; /* no shape rate */
        SOC_SBX_CFG(unit)->bcm_cosq_priority_weight[cos] = 0; /* no weight */
        SOC_SBX_CFG(unit)->bcm_cosq_priority_min_depth_bytes[cos] = (25*1024);  /* 25kbytes */
        SOC_SBX_CFG(unit)->bcm_cosq_priority_max_depth_bytes[cos] = (1024*1024); /* 1Mbyte */
	SOC_SBX_CFG(unit)->bcm_cosq_priority_group[cos] = 0; /* default mapped to pg 0 */
    }
    SOC_SBX_CFG(unit)->pfc_cos_enable = 0;
    SOC_SBX_CFG(unit)->bcm_cosq_all_min_bw_kbps = 1000000; /* 1G bag rate */

    SOC_SBX_CFG(unit)->hold_pri_num_timeslots = -1;  /* default hold depends on queue region */

    SOC_SBX_CFG(unit)->num_ingress_scheduler = 0;
    SOC_SBX_CFG(unit)->num_egress_scheduler = 0;
    SOC_SBX_CFG(unit)->num_ingress_multipath = 0;
    SOC_SBX_CFG(unit)->num_egress_multipath = 0;

    for (cos = 0; cos < SBX_MAX_FABRIC_COS; cos++) {
        SOC_SBX_CFG(unit)->connect_min_util[cos] = soc_sbx_connect_min_util[cos];
	if (SOC_IS_SIRIUS(unit)) {
	    SOC_SBX_CFG(unit)->connect_min_util[cos] = SOC_SBX_CFG(unit)->connect_min_util[cos] * 2;
	}
        SOC_SBX_CFG(unit)->connect_max_age_time[cos] = soc_sbx_connect_max_age_time[cos];
    }

    SOC_SBX_CFG(unit)->diag_qe_revid = -1;

    if (SOC_IS_SBX_FE2000(unit)) {
        /* FE2000 only config */
        SOC_SBX_CFG_FE2000(unit)->mm0wx18 =
            soc_property_get(unit, spn_WIDE_SRAM0_X18, 0);
        SOC_SBX_CFG_FE2000(unit)->mm0n0awidth =
            soc_property_get(unit, spn_NP0_ADDR_WIDTH, 21);
        SOC_SBX_CFG_FE2000(unit)->mm0n0dwidth =
            soc_property_get(unit, spn_NP0_DATA_WIDTH, 36);
        SOC_SBX_CFG_FE2000(unit)->mm0n1awidth =
            soc_property_get(unit, spn_NP1_ADDR_WIDTH, 21);
        SOC_SBX_CFG_FE2000(unit)->mm0n1dwidth =
            soc_property_get(unit, spn_NP1_DATA_WIDTH, 36);
        SOC_SBX_CFG_FE2000(unit)->mm0wawidth =
            soc_property_get(unit, spn_WP_ADDR_WIDTH, 21);
        SOC_SBX_CFG_FE2000(unit)->mm0wdwidth =
            soc_property_get(unit, spn_WP_DATA_WIDTH, 36);

        SOC_SBX_CFG_FE2000(unit)->mm1wx18 =
            soc_property_get(unit, spn_WIDE_SRAM1_X18, 0);
        SOC_SBX_CFG_FE2000(unit)->mm1n0awidth =
            soc_property_get(unit, spn_NP0_ADDR_WIDTH, 21);
        SOC_SBX_CFG_FE2000(unit)->mm1n0dwidth =
            soc_property_get(unit, spn_NP0_DATA_WIDTH, 36);
        SOC_SBX_CFG_FE2000(unit)->mm1n1awidth =
            soc_property_get(unit, spn_NP1_ADDR_WIDTH, 21);
        SOC_SBX_CFG_FE2000(unit)->mm1n1dwidth =
            soc_property_get(unit, spn_NP1_DATA_WIDTH, 36);
        SOC_SBX_CFG_FE2000(unit)->mm1wawidth =
            soc_property_get(unit, spn_WP_ADDR_WIDTH, 21);
        SOC_SBX_CFG_FE2000(unit)->mm1wdwidth =
            soc_property_get(unit, spn_WP_DATA_WIDTH, 36);

    } else if (SOC_IS_SBX_QE2000(unit)) {

        SOC_SBX_CFG(unit)->uSerdesSpeed = 3125;        /* 3.125G by default */
        SOC_SBX_CFG(unit)->bSerdesEncoding = TRUE;       /* 8B10B by default */

        /* QE2000 only config */
        SOC_SBX_CFG_QE2000(unit)->n2one_ul = 0;
        SOC_SBX_CFG(unit)->uClockSpeedInMHz = 250;  /* full speed by default */
        SOC_SBX_CFG_QE2000(unit)->bHalfBus = FALSE;        /* half bus mode for the qe device only */
        SOC_SBX_CFG_QE2000(unit)->bRunLongDdrMemoryTest = FALSE;
        SOC_SBX_CFG_QE2000(unit)->bQm512MbDdr2 = TRUE;
        SOC_SBX_CFG_QE2000(unit)->bSv2_5GbpsLinks = 0;
        SOC_SBX_CFG_QE2000(unit)->uEgMVTSize = 2;          /* By default 48k mvt entries */
        SOC_SBX_CFG_QE2000(unit)->uEgMcDropOnFull = TRUE;
        SOC_SBX_CFG_QE2000(unit)->uEgMvtFormat = SBX_MVT_FORMAT0;
        SOC_SBX_CFG_QE2000(unit)->uEgMcEfTimeout = SB_FAB_DEVICE_QE2000_MC_PORT_TIMEOUT;
        SOC_SBX_CFG_QE2000(unit)->uEgMcNefTimeout = SB_FAB_DEVICE_QE2000_MC_PORT_TIMEOUT;
        SOC_SBX_CFG_QE2000(unit)->uEiPortInactiveTimeout = 500;
        SOC_SBX_CFG_QE2000(unit)->uScGrantoffset = 0xE;
        SOC_SBX_CFG_QE2000(unit)->nGlobalShapingAdjustInBytes = 20;  /* by default, adjust length for ethernet IPG */
        SOC_SBX_CFG_QE2000(unit)->uQmMaxArrivalRateMbs = 20000;
        SOC_SBX_CFG_QE2000(unit)->bMixHighAndLowRateFlows = TRUE;
        SOC_SBX_CFG_QE2000(unit)->uSfiTimeslotOffsetInClocks = 0x96;
        SOC_SBX_CFG_QE2000(unit)->uScTxdmaSotDelayInClocks = -1;
        SOC_SBX_CFG_QE2000(unit)->bEiSpiFullPacketMode[0] = SOC_SBX_PORT_MODE_BURST_IL;
        SOC_SBX_CFG_QE2000(unit)->bEiSpiFullPacketMode[1] = SOC_SBX_PORT_MODE_BURST_IL;
        SOC_SBX_CFG_QE2000(unit)->uEiLines[0] = 768;
        SOC_SBX_CFG_QE2000(unit)->uEiLines[1] = 768;
        SOC_SBX_CFG_QE2000(unit)->nodeNum_ul = 0;          /* Node 0 by default */
        SOC_SBX_CFG_QE2000(unit)->uQsMaxNodes = SB_FAB_USER_MAX_NUM_NODES;
        SOC_SBX_CFG_QE2000(unit)->nQueuesPerShaperIngress = 1;  /* Default to be per queue shaping */
        SOC_SBX_CFG_QE2000(unit)->uSfiDataLinkInitMask = 0;     /* No QE1k links supported */
        SOC_SBX_CFG_QE2000(unit)->bEpDisable = FALSE;
        SOC_SBX_CFG_QE2000(unit)->SpiRefClockSpeed[0] = 0;
        SOC_SBX_CFG_QE2000(unit)->SpiClockSpeed[0] = 0;         /* Default, Power On Reset value */
        SOC_SBX_CFG_QE2000(unit)->SpiRefClockSpeed[1] = 0;
        SOC_SBX_CFG_QE2000(unit)->SpiClockSpeed[1] = 0;         /* Default, Power on Reset value */
        SOC_SBX_CFG_QE2000(unit)->uNumPhySpiPorts[0] = 0;
        SOC_SBX_CFG_QE2000(unit)->uNumPhySpiPorts[1] = 0;

        for (port=0; port < SB_FAB_DEVICE_QE2000_MAX_PORT; port++) {

            SOC_SBX_CFG_QE2000(unit)->uEgressMcastEfDescFifoSize[port] = -1;
            SOC_SBX_CFG_QE2000(unit)->uEgressMcastNefDescFifoSize[port] = -1;
            SOC_SBX_CFG_QE2000(unit)->bEgressMcastEfDescFifoInUse[port] = FALSE;
            SOC_SBX_CFG_QE2000(unit)->bEgressMcastNefDescFifoInUse[port] = FALSE;
        }

        SOC_SBX_CFG_QE2000(unit)->uNumSpiTxPorts[0] = 0;
        SOC_SBX_CFG_QE2000(unit)->uNumSpiTxPorts[1] = 0;
        SOC_SBX_CFG_QE2000(unit)->uNumSpiRxPorts[0] = 0;
        SOC_SBX_CFG_QE2000(unit)->uNumSpiRxPorts[1] = 0;
        SOC_SBX_CFG_QE2000(unit)->uNumSpiTxStatusRepCnt[0] = 0;
        SOC_SBX_CFG_QE2000(unit)->uNumSpiTxStatusRepCnt[1] = 0;
        SOC_SBX_CFG_QE2000(unit)->uNumSpiRxStatusRepCnt[0] = 0;
        SOC_SBX_CFG_QE2000(unit)->uNumSpiRxStatusRepCnt[1] = 0;
        SOC_SBX_CFG_QE2000(unit)->uuRequeuePortsMask[0] = 0;
        SOC_SBX_CFG_QE2000(unit)->uuRequeuePortsMask[1] = 0;


        SOC_SBX_CFG(unit)->max_pkt_len_adj_sel = SB_FAB_DEVICE_QE2000_MAX_PKT_LEN_ADJ_SEL;
        SOC_SBX_CFG(unit)->max_pkt_len_adj_value = SB_FAB_DEVICE_QE2000_MAX_PKT_LEN_ADJ_VALUE;

        for (spi=0; spi<SB_FAB_DEVICE_QE2000_NUM_SPI_INTERFACES; spi++) {
            for(port=0; port<SB_FAB_DEVICE_QE2000_MAX_SPI_SUBPORTs; port++){
                SOC_SBX_CFG_QE2000(unit)->uSpiSubportSpeed[spi][port] = 0;
            }
        }

        for(port=0; port<SB_FAB_DEVICE_QE2000_NUM_SERIALIZERS; port++){
            GetDefaultLinkDriverConfig((sbFabUserDeviceHandle_t)sbx->sbhdl, port, &(SOC_SBX_CFG_QE2000(unit)->linkDriverConfig[port]));
        }

        SOC_SBX_CFG(unit)->nDiscardTemplates = SB_FAB_DEVICE_QE2000_MAX_WRED_TEMPLATES;
        SOC_SBX_CFG(unit)->mcgroup_local_start_index = SBX_MVT_ID_LOCAL_BASE;
        SOC_SBX_CFG(unit)->nShaperCount = SB_FAB_DEVICE_QE2000_MAX_EGRESS_SHAPERS;


        SOC_SBX_CFG_QE2000(unit)->bVirtualPortFairness = 1;

        SOC_SBX_CFG_QE2000(unit)->uPacketAdjustFormat = 0;

    } else if (SOC_IS_SBX_BME3200(unit)) {
        /* BM3200 only config */
        SOC_SBX_CFG(unit)->uClockSpeedInMHz = 200;  
        SOC_SBX_CFG_BM3200(unit)->uDeviceMode = 0;
        SOC_SBX_CFG_BM3200(unit)->uBmLocalBmId = 0;
        SOC_SBX_CFG_BM3200(unit)->uBmDefaultBmId = 0;
        SOC_SBX_CFG_BM3200(unit)->bSv2_5GbpsLinks = 0;
        SOC_SBX_CFG(unit)->uSerdesSpeed = 3125;          /* 3.125G by default */
        SOC_SBX_CFG(unit)->bSerdesEncoding = TRUE;       /* 8B10B by default */

        for(port=0; port<SB_FAB_DEVICE_BM3200_NUM_SERIALIZERS; port++){
            GetDefaultLinkDriverConfig((sbFabUserDeviceHandle_t)sbx->sbhdl, port, &(SOC_SBX_CFG_BM3200(unit)->linkDriverConfig[port]));
            SOC_SBX_CFG_BM3200(unit)->linkState[port].nState = SB_FAB_DEVICE_SERIALIZER_STATE_RESERVED;
        }

        /*
         * Default to SCIs on 0 & 1 because the BME3200 requires SCI ports
         * starting at port 0
         */
        SOC_SBX_CFG_BM3200(unit)->nNumLinks = 0;
        SOC_SBX_CFG_BM3200(unit)->uSerializerMask = 0;
        SOC_SBX_CFG_BM3200(unit)->uLcmSerializerMaskHi = 0;          /* Unused for BM3200 */
        SOC_SBX_CFG_BM3200(unit)->uLcmSerializerMaskLo = 0;          /* Unused for BM3200 */
        SOC_SBX_CFG_BM3200(unit)->uLcmPlaneValid[0] = 0;             /* Unused for BM3200 */
        SOC_SBX_CFG_BM3200(unit)->uLcmPlaneValid[1] = 0;             /* Unused for BM3200 */
        SOC_SBX_CFG_BM3200(unit)->bLcmXcfgABInputPolarityReversed = FALSE;
        sal_memset(SOC_SBX_CFG_BM3200(unit)->uLcmXcfg, 0,            /* Unused for BM3200 */
                   (size_t)(SB_FAB_MAX_NUM_DATA_PLANES *  \
                            SB_FAB_DEVICE_BM3200_NUM_SERIALIZERS *      \
                            sizeof(uint32_t)));

        SOC_SBX_CFG(unit)->nDiscardTemplates = SB_FAB_DEVICE_BM3200_MAX_WRED_TEMPLATES;
    } else if (SOC_IS_SBX_BM9600(unit)) {
        /* BM9600 only config */
        SOC_SBX_CFG(unit)->uClockSpeedInMHz = 200;  /* full speed by default */
        SOC_SBX_CFG_BM9600(unit)->uDeviceMode = 0;
        SOC_SBX_CFG_BM9600(unit)->bElectArbiterReconfig = FALSE;
        SOC_SBX_CFG_BM9600(unit)->uBmLocalBmId = 0;
        SOC_SBX_CFG_BM9600(unit)->uBmDefaultBmId = 0;
        SOC_SBX_CFG_BM9600(unit)->bSv2_5GbpsLinks = 0;
        SOC_SBX_CFG(unit)->uSerdesSpeed = 6250;         /* 6.250G by default */
        SOC_SBX_CFG(unit)->bSerdesEncoding = TRUE;      /* 8b10b by default */
        SOC_SBX_CFG_BM9600(unit)->BackupDeviceUnit = -1;      /* 8b10b by default */

        for (port=0; port< SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS; port++) {
            SOC_SBX_CFG_BM9600(unit)->uSerdesAbility[port] = SOC_PORT_ABILITY_SFI;
        }

        for(port=0; port<SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS; port++){
            GetDefaultLinkDriverConfig((sbFabUserDeviceHandle_t)sbx->sbhdl, port, &(SOC_SBX_CFG_BM9600(unit)->linkDriverConfig[port]));
            SOC_SBX_CFG_BM9600(unit)->linkState[port].nState = SB_FAB_DEVICE_SERIALIZER_STATE_RESERVED;
        }


        /*
         * Default to SCIs on 0 & 1 because the BME3200 requires SCI ports
         * starting at port 0
         */
        SOC_SBX_CFG_BM9600(unit)->nNumLinks = 0;
        SOC_SBX_CFG_BM9600(unit)->uLcmSerializerMaskHi = 0;
        SOC_SBX_CFG_BM9600(unit)->uLcmSerializerMaskLo = 0;
        SOC_SBX_CFG_BM9600(unit)->uLcmPlaneValid[0] = 0;
        SOC_SBX_CFG_BM9600(unit)->uLcmPlaneValid[1] = 0;
        SOC_SBX_CFG_BM9600(unit)->bLcmXcfgABInputPolarityReversed = FALSE;
        sal_memset(SOC_SBX_CFG_BM9600(unit)->uLcmXcfg, 0,
                   (size_t)(SB_FAB_MAX_NUM_DATA_PLANES *  \
                            SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS *      \
                            sizeof(uint32_t)));

        SOC_SBX_CFG(unit)->nDiscardTemplates = SB_FAB_DEVICE_BM9600_MAX_WRED_TEMPLATES;
        SOC_SBX_CFG_BM9600(unit)->cached_ina = -1;
    } else if (SOC_IS_SBX_SIRIUS(unit)) {
        SOC_SBX_CFG(unit)->uClockSpeedInMHz = 405;        /* default to 405MhZ for 80G */

        /* default to 1k col, 8k rol, 1G memory */
        SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns = 1024;
        SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows = 8192;
        SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumBanks = 8;
        SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumMemories = 10; /* default to 10 ddr devices */
        SOC_SBX_CFG_SIRIUS(unit)->uDdr3ClockMhz = 667;   /* 667 Mhz */
        SOC_SBX_CFG_SIRIUS(unit)->uDdr3MemGrade = 0;     /* 9-9-9 grade */
        SOC_SBX_CFG_SIRIUS(unit)->uMaxBuffers = 256 * 1024;     /* match other ddr settings */

        SOC_SBX_CFG_SIRIUS(unit)->ucast_cos_map = _SIRIUS_I_COS_PROFILE_GENERAL;
        SOC_SBX_CFG_SIRIUS(unit)->mcast_cos_map = _SIRIUS_I_COS_PROFILE_GENERAL;

        SOC_SBX_CFG(unit)->uSerdesSpeed = 6250;           /* 6.250G by default */
        SOC_SBX_CFG(unit)->bSerdesEncoding = TRUE;        /* 8b10b by default */

        SOC_SBX_CFG(unit)->max_pkt_len_adj_sel = SB_FAB_DEVICE_SIRIUS_MAX_PKT_LEN_ADJ_SEL;
        SOC_SBX_CFG(unit)->max_pkt_len_adj_value = SB_FAB_DEVICE_SIRIUS_MAX_PKT_LEN_ADJ_VALUE;

        for (port=0; port< SB_FAB_DEVICE_SIRIUS_NUM_SERIALIZERS; port++) {
            SOC_SBX_CFG_SIRIUS(unit)->uSerdesAbility[port] = SOC_PORT_ABILITY_DUAL_SFI;
        }

        for(port=0; port<SB_FAB_DEVICE_SIRIUS_NUM_SERIALIZERS; port++){
            GetDefaultLinkDriverConfig((sbFabUserDeviceHandle_t)sbx->sbhdl, port, &(SOC_SBX_CFG_SIRIUS(unit)->linkDriverConfig[port]));
        }

        /* Limited by the SIRIUS to 16K for now, sirius max is 32k VOQs */
        SOC_SBX_CFG_SIRIUS(unit)->nMaxVoq = BM9600_BW_MAX_VOQ_NUM;
        /* In BCMSIM, DualLocalGrants is not supported */
        SOC_SBX_CFG_SIRIUS(unit)->bDualLocalGrants = (SAL_BOOT_BCMSIM? FALSE : TRUE);

        /* default to 4 fifos for each fifo group as what hardware naturally supports */
        SOC_SBX_CFG_SIRIUS(unit)->uFifoGroupSize =  SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_HW_SIZE;

        for(hg = 0; hg < SB_FAB_DEVICE_SIRIUS_NUM_HG_PORTS-1; hg++) {
            intf = SB_FAB_DEVICE_SIRIUS_HG0_INTF + hg;
            SOC_SBX_SIRIUS_STATE(unit)->uNumExternalSubports[intf] = 0;
            SOC_SBX_SIRIUS_STATE(unit)->uNumInternalSubports[intf] = 0;
            for(port = 0; port < SB_FAB_DEVICE_SIRIUS_MAX_PER_HG_SUBPORTS; port++) {
                for (group = 0; group < SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX; group++) {
                    SOC_SBX_CFG_SIRIUS(unit)->uSubportSpeed[intf][port][group][SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX_SIZE] = 0;
                    for(fifo = 0; fifo < SOC_SBX_CFG_SIRIUS(unit)->uFifoGroupSize; fifo++) {
                        SOC_SBX_CFG_SIRIUS(unit)->uSubportSpeed[intf][port][group][fifo] = 0;
                    }
                }
            }
        }
        SOC_SBX_SIRIUS_STATE(unit)->nMaxFabricPorts = 0;
	SOC_SBX_CFG_SIRIUS(unit)->bExtendedPortMode = FALSE;
        SOC_SBX_CFG_SIRIUS(unit)->thresh_drop_limit = -1;

        /*
         * Initialize speed for CPU port
         */
        for(port = 0; port < SB_FAB_DEVICE_SIRIUS_MAX_PER_HG_SUBPORTS; port++) {
            for (group = 0; group < SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX; group++) {
                SOC_SBX_CFG_SIRIUS(unit)->uSubportSpeed[SB_FAB_DEVICE_SIRIUS_CPU_INTF][port][group][SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX_SIZE] = 1000;
                for(fifo = 0; fifo < SOC_SBX_CFG_SIRIUS(unit)->uFifoGroupSize; fifo++) {
                    SOC_SBX_CFG_SIRIUS(unit)->uSubportSpeed[SB_FAB_DEVICE_SIRIUS_CPU_INTF][port][group][fifo] = 1000;
                }
            }
        }

        SOC_SBX_SIRIUS_STATE(unit)->uTotalInternalSubports = 0;
        SOC_SBX_CFG_SIRIUS(unit)->ucast_ef_fifo = SB_FAB_XCORE_COS_FIFO_UNICAST_EF;
        SOC_SBX_CFG_SIRIUS(unit)->mcast_ef_fifo = SB_FAB_XCORE_COS_FIFO_MULTICAST_EF;
        SOC_SBX_CFG_SIRIUS(unit)->mcast_nef_fifo = SB_FAB_XCORE_COS_FIFO_MULTICAST_NEF;

        /* by default no ingress scheduler is used, all egress schedulers are used */
        for(level = 0; level < SB_FAB_DEVICE_SIRIUS_NUM_TS_LEVELS; level++) {
            SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level] = 0;
        }
        SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level] = 1; /* root only has 1 scheduler */

        SOC_SBX_CFG_SIRIUS(unit)->uNumEsNode[0] = SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L0;
        SOC_SBX_CFG_SIRIUS(unit)->uNumEsNode[1] = SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L1;
        SOC_SBX_CFG_SIRIUS(unit)->uNumEsNode[2] = SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L2;
        SOC_SBX_CFG_SIRIUS(unit)->uNumEsNode[3] = SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L3;
        SOC_SBX_CFG_SIRIUS(unit)->uNumEsNode[4] = SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L4;
        /* support level 1/2 egress schedulers need to be allocated */
        SOC_SBX_CFG(unit)->num_egress_scheduler = SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L1 +
	    SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_SCHEDULER_L2;
        SOC_SBX_CFG(unit)->num_egress_multipath = SB_FAB_DEVICE_SIRIUS_NUM_EGRESS_GROUP_SHAPER;

	SOC_SBX_CFG_SIRIUS(unit)->nNodeUserManagementMode = -1; /* unknown mode at default */

        SOC_SBX_CFG_SIRIUS(unit)->nNodeUserManagementMode = -1; /* unknown mode at default */

        /* TODO: Restrict sirius chip capability based on chip ID */

        SOC_SBX_CFG_SIRIUS(unit)->uQmMaxArrivalRateMbs = 80000;
        SOC_SBX_CFG_SIRIUS(unit)->uSubscriberMaxCos = 8;            /* default to 8 cos levels */
        SOC_SBX_CFG_SIRIUS(unit)->bSubscriberNodeOptimize = FALSE;  /* default to optimize level usage */

        SOC_SBX_CFG(unit)->nDiscardTemplates = SB_FAB_DEVICE_SIRIUS_MAX_WRED_TEMPLATES;

        for (port = 0; port < SB_FAB_DEVICE_SIRIUS_MAX_SCHED_INTERFACES; port++) {
            
            /* by default, adjust length for ethernet interframe gap */
	    if (soc_property_get(unit, spn_IF_PROTOCOL, 1) == 1) {
		SOC_SBX_CFG_SIRIUS(unit)->shapingBusLengthAdj[port] = 10;
	    } else { /* in XGS mode, default shapingBusLengthAdj to 0 */
		SOC_SBX_CFG_SIRIUS(unit)->shapingBusLengthAdj[port] = 5;
	    }
        }

        level = soc_property_get(unit, spn_QE_MVR_MAX_SIZE, 5);
        if ((1 > level) || (5 < level)) {
            
            level = 5;
        }
        SOC_SBX_CFG_SIRIUS(unit)->mvrMaxSize = level;

        level = soc_property_get(unit, spn_QE_MC_DUAL_LOOKUP, 0);
        if (level) {
            SOC_SBX_CFG_SIRIUS(unit)->dualLookup = TRUE;
        } else {
            SOC_SBX_CFG_SIRIUS(unit)->dualLookup = FALSE;
        }

        level = soc_property_get(unit, spn_QE_LAG_UC_REDIST, 0);
        if (level) {
            SOC_SBX_CFG_SIRIUS(unit)->redirectUcLag = TRUE;
        } else {
            SOC_SBX_CFG_SIRIUS(unit)->redirectUcLag = FALSE;
        }
    }

    for (node = 0; node < SBX_MAXIMUM_NODES; node++) {
        /* setup one to one mapping for logical node to physical node */
        SOC_SBX_CFG(unit)->l2p_node[node] = node;
        SOC_SBX_CFG(unit)->p2l_node[node] = node;
    }

    SOC_SBX_CFG(unit)->uRateClockSpeed = SIRIUS_RATE_CLOCK;
    SOC_SBX_CFG(unit)->uMaxClocksInEpoch = SIRIUS_MAX_CLOCKS_PER_EPOCH;

    /* all lxbars below 18 must be the interop xbars shared between QE and SS */
    SOC_SBX_CFG(unit)->max_interop_xbar = SB_FAB_DEVICE_QE2000_SFI_LINKS;
}

static void
soc_sbx_intr_disable(int unit, uint16 dev_id, uint32 mask)
{
    int                s;
    soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);

    if ( (dev_id == BCM88020_DEVICE_ID) ||
         (dev_id == BCM88025_DEVICE_ID) ){
        s = sal_splhi();
        SAND_HAL_FE2000_WRITE(sbx->sbhdl, PC_INTERRUPT_MASK, mask);
        SAND_HAL_FE2000_WRITE(sbx->sbhdl, PC_ERROR0_MASK, mask);
        SAND_HAL_FE2000_WRITE(sbx->sbhdl, PC_ERROR1_MASK, mask);
        SAND_HAL_FE2000_WRITE(sbx->sbhdl, PC_UNIT_INTERRUPT_MASK, mask);
        sal_spl(s);
    }
}

int
soc_sbx_attach(int unit)
{
    soc_sbx_control_t   *sbx;
    uint16              dev_id, dev_id_driver;
    uint8               rev_id, rev_id_driver;
    uint32              dev_type;
    soc_control_t       *soc;
    soc_persist_t       *sop;
    soc_port_t          port;
#if defined(BCM_SIRIUS_SUPPORT)
    int                 mem;
    int                 ix;
    int                 rv = SOC_E_MEMORY;
#endif

    SOC_DEBUG_PRINT((DK_PCI, "soc_attach: unit %d\n", unit));

    /*
     * Allocate soc_control and soc_persist if not already.
     */

    soc = SOC_CONTROL(unit);
    if (SOC_CONTROL(unit) == NULL) {
        SOC_CONTROL(unit) =
            sal_alloc(sizeof (soc_control_t), "soc_control");
        if (SOC_CONTROL(unit) == NULL) {
            return SOC_E_MEMORY;
        }
        sal_memset(SOC_CONTROL(unit), 0, sizeof (soc_control_t));
    } else {
        if (soc->soc_flags & SOC_F_ATTACHED) {
            return(SOC_E_NONE);
        }
    }

    SOC_PERSIST(unit) = sal_alloc(sizeof (soc_persist_t), "soc_persist");
    if (NULL == SOC_PERSIST(unit)) {
        return SOC_E_MEMORY;
    }

    sal_memset(SOC_PERSIST(unit), 0, sizeof (soc_persist_t));

    soc = SOC_CONTROL(unit);
    sop = SOC_PERSIST(unit);
    sop->version = 1;

    /* Sbx control structure
     */
    if (SOC_SBX_CONTROL(unit) == NULL) {
        SOC_CONTROL(unit)->drv = (soc_sbx_control_t *)
                                 sal_alloc(sizeof(soc_sbx_control_t),
                                           "soc_sbx_control");

        if (SOC_SBX_CONTROL(unit) == NULL) {
            return SOC_E_MEMORY;
        }
        sal_memset(SOC_SBX_CONTROL(unit), 0, sizeof (soc_sbx_control_t));
        sbx = SOC_SBX_CONTROL(unit);
        sbx->cfg = NULL;
    }

#if defined(BCM_SIRIUS_SUPPORT)
    /* make sure state information is correctly initialised */
    if (!__soc_sbx_sirius_state_init) {
        sal_memset(&(_soc_sbx_sirius_state[0]),
                   0x00,
                   sizeof(_soc_sbx_sirius_state[0]) * SOC_MAX_NUM_DEVICES);
        __soc_sbx_sirius_state_init = TRUE;
    }
#endif /* defined(BCM_SIRIUS_SUPPORT) */

    sbx = SOC_SBX_CONTROL(unit);
    sbx->sbhdl = (sbhandle)unit;

    soc_cm_get_id(unit, &dev_id, &rev_id);
    soc_cm_get_id_driver(dev_id, rev_id, &dev_id_driver, &rev_id_driver);

    soc->chip_driver = soc_sbx_chip_driver_find(dev_id_driver, rev_id_driver);

    /* TODO: do we want to call detach if we fail? esw/drv.c does but it seems
     * a bit haphazard in that it does much initialization with possible
     * failures before it starts calling detach on failure
     */
    if (soc->chip_driver == NULL) {
        SOC_ERROR_PRINT((DK_ERR, "soc_attach: unit %d has no driver "
                         "(device 0x%04x rev 0x%02x)\n",
                         unit, dev_id, rev_id));
        return SOC_E_UNAVAIL;
    }

    soc->soc_functions = &soc_sbx_drv_funs;

#if defined(BCM_SIRIUS_SUPPORT)
    /* TODO: do we need to provide soc_functions here for sirius
     */
    sbx->sbx_functions.sirius_ddr_clear = soc_sbx_sirius_ddr23_clear;
#endif

    dev_type = soc_cm_get_dev_type(unit);

    /*
     * Default config
     */
    if (sbx->cfg == NULL) {
        sbx->cfg = (soc_sbx_config_t *)
                   sal_alloc(sizeof(soc_sbx_config_t),
                             "soc_sbx_config");
        if (sbx->cfg == NULL) {
             return SOC_E_MEMORY;
        }
        sal_memset(sbx->cfg, 0, sizeof (soc_sbx_config_t));
    }

    SOC_SBX_CFG(unit)->bTmeMode = soc_property_get(unit, spn_QE_TME_MODE, 0);

    /* Set feature cache, since used by mutex creation */
    if (SOC_DRIVER(unit)->feature) {
        soc_feature_init(unit);
    }

    /* Create MIIM mutex */
    if ((soc->miimMutex = sal_mutex_create("MIIM")) == NULL) {
        SOC_ERROR_PRINT((DK_ERR,
                         "soc_attach: unit %d unable to create miimMutex\n",
                         unit));
        return SOC_E_MEMORY;
    }

    if (soc_sbx_info_config(unit, dev_id, dev_id_driver) != SOC_E_NONE) {
        SOC_ERROR_PRINT((DK_ERR,
                         "soc_attach: unit %d unable to init config\n",
                         unit));
        return SOC_E_INTERNAL;
    }

    sbx->dma_sem = sal_sem_create("DMA done semaphore", sal_sem_BINARY, 0);
    if (!sbx->dma_sem) {
        SOC_ERROR_PRINT((DK_ERR,
                         "soc_attach: unit %d unable to create dma_sem\n",
                         unit));
        return SOC_E_MEMORY;
    }

#if defined(BCM_SIRIUS_SUPPORT)
    /* TODO: do we need to provide polled interrupt here for sirius
     */
    if (SOC_IS_SBX_SIRIUS(unit)) {
        soc_intr_disable(unit, ~0);

        /* allocate local state information if needed */
        if (!SOC_SBX_SIRIUS_STATE(unit)) {
            SOC_SBX_SIRIUS_STATE(unit) = sal_alloc(sizeof(*SOC_SBX_SIRIUS_STATE(unit)),
                                                   "Sirius SOC state");
        }
        /* reset local state (fail now if not allocated) */
        
        if (SOC_SBX_SIRIUS_STATE(unit)) {
            sal_memset(SOC_SBX_SIRIUS_STATE(unit),
                       0x00,
                       sizeof(*SOC_SBX_SIRIUS_STATE(unit)));
        } else {
            return SOC_E_MEMORY;
        }
    }
#endif


    /* Install the Interrupt Handler */
    /* Make sure interrupts are masked before connecting line. */
#ifdef BCM_CMICM_SUPPORT
    if (soc_feature(unit, soc_feature_cmicm)) {
        soc_cmicm_intr0_disable(unit, ~0);
        soc_cmicm_intr1_disable(unit, ~0);
        SOC_PCI_CMC(unit) = soc_property_uc_get(unit, 0, spn_CMC, 0);
    } else
#endif
    {
        /* disable interrupts before connecting isr */
        soc_sbx_intr_disable(unit, dev_id, ~0);
    }

    if (sbx->isr != NULL) {
#if defined(BCM_SIRIUS_SUPPORT)
        if (soc_property_get(unit, spn_POLLED_IRQ_MODE, 0)) {
            if (soc_ipoll_connect(unit, sbx->isr, INT_TO_PTR(unit)) < 0) {
                SOC_ERROR_PRINT((DK_ERR, "soc_attach: soc_attach: unit %d"
                                 " polled interrupt connect failed\n",
                                 unit));
                return SOC_E_INTERNAL;
            }
            soc->soc_flags |= SOC_F_POLLED;
        }
        else
#endif
        {
            /* unit # is ISR arg */
            if (soc_cm_interrupt_connect(unit, sbx->isr,
                                         INT_TO_PTR(unit)) < 0) {
                SOC_ERROR_PRINT((DK_ERR, "soc_attach: unit %d interrupt"
                                 " connect failed\n", unit));
                return SOC_E_INTERNAL;
            }
        }
    }

    if (soc->soc_flags & SOC_F_ATTACHED) {
        SOC_ERROR_PRINT((DK_ERR, "soc_attach: unit %d already attached\n",
                         unit));
        return(SOC_E_NONE);
    }

    /* Initialize information in soc persistent structure */
    sop = SOC_PERSIST(unit);
    SOC_PBMP_CLEAR(sop->link_fwd);

    /*
     * Configure nominal IPG register settings.
     * By default the IPG should be 96 bit-times.
     */
    PBMP_ALL_ITER(unit, port) {
        sop->ipg[port].hd_10    = 96;
        sop->ipg[port].hd_100   = 96;
        sop->ipg[port].hd_1000  = 96;
        sop->ipg[port].hd_2500  = 96;

        sop->ipg[port].fd_10    = 96;
        sop->ipg[port].fd_100   = 96;
        sop->ipg[port].fd_1000  = 96;
        sop->ipg[port].fd_2500  = 96;
        sop->ipg[port].fd_10000 = 96;
    }

    soc_sbx_get_default(unit, sbx->cfg);

#if defined(BCM_SIRIUS_SUPPORT)
    if (SOC_IS_SBX_SIRIUS(unit)) {
        if ((soc->socControlMutex = sal_mutex_create("SOC_CONTROL")) == NULL) {
            SOC_ERROR_PRINT((DK_ERR,
                             "soc_attach: unit %d unable to create schanMutex\n",
                             unit));
            return SOC_E_MEMORY;
        }
 
        if ((soc->schanMutex = sal_mutex_create("SCHAN")) == NULL) {
            SOC_ERROR_PRINT((DK_ERR,
                             "soc_attach: unit %d unable to create schanMutex\n",
                             unit));
            return SOC_E_MEMORY;
        }

#ifdef BCM_CMICM_SUPPORT
    if (soc_property_get(unit, spn_FSCHAN_ENABLE, 0)) {
        if ((soc->fschanMutex = sal_mutex_create("FSCHAN")) == NULL) {
            return SOC_E_MEMORY;
        }
    } else {
        soc->fschanMutex = NULL;
    }
#endif

        /* create mutex for memory access */
        for (mem = 0; mem < NUM_SOC_MEM; mem++) {
            /*
             * should only create mutexes for valid memories, but at least
             * PTABLE's mutex is used (in PORT_LOCK)
             */
            if ((soc->memState[mem].lock =
                 sal_mutex_create(SOC_MEM_NAME(unit, mem))) == NULL) {
                SOC_ERROR_PRINT((DK_ERR,
                                 "soc_attach: unit %d unable to create Mutex for mem %s\n",
                                 unit, SOC_MEM_NAME(unit, mem)));
            }

            if (!SOC_MEM_IS_VALID(unit, mem)) {
                continue;
            }

            /* Set cache copy pointers to NULL */

            sal_memset(soc->memState[mem].cache,
                       0,
                       sizeof (soc->memState[mem].cache));
        }

#ifdef INCLUDE_MEM_SCAN
        if ((soc->mem_scan_notify =
             sal_sem_create("memscan timer", sal_sem_BINARY, 0)) == NULL) {
            SOC_ERROR_PRINT((DK_ERR, "soc_attach: unit %d failed\n", unit));
            soc_sbx_detach(unit);           /* Perform necessary clean-ups on error */
            return rv;
        }

        soc->mem_scan_pid = SAL_THREAD_ERROR;
        soc->mem_scan_interval = 0;
#endif

        /* create mutex for table dma */
        soc->tableDmaMutex = NULL; /* marker for soc_mem_dmaable */
        soc->tableDmaIntr = NULL;
        if (soc_feature(unit, soc_feature_table_dma) &&
            soc_property_get(unit, spn_TABLE_DMA_ENABLE, 1)) {
            if (SAL_BOOT_QUICKTURN) {
                soc->tableDmaTimeout = TDMA_TIMEOUT_QT;
            } else {
                soc->tableDmaTimeout = TDMA_TIMEOUT;
            }
            soc->tableDmaTimeout = soc_property_get(unit, spn_TDMA_TIMEOUT_USEC,
                                                    soc->tableDmaTimeout);
            if (soc->tableDmaTimeout) {
                soc->tableDmaMutex = sal_mutex_create("TblDMA");
                if (soc->tableDmaMutex == NULL) {
                    SOC_ERROR_PRINT((DK_ERR, "soc_attach: unit %d failed\n", unit));
                    soc_sbx_detach(unit);           /* Perform necessary clean-ups on error */
                    return rv;
                }
                soc->tableDmaIntr = sal_sem_create("TDMA interrupt",
                                                   sal_sem_BINARY, 0);
                if (soc->tableDmaIntr == NULL) {
                    SOC_ERROR_PRINT((DK_ERR, "soc_attach: unit %d failed\n", unit));
                    soc_sbx_detach(unit);           /* Perform necessary clean-ups on error */
                    return rv;
                }
                soc->tableDmaIntrEnb = soc_property_get(unit,
                                                        spn_TDMA_INTR_ENABLE, 1);
            }
        }

        /* create mutex for slam dma */
        soc->tslamDmaMutex = NULL; /* marker for soc_mem_write_dmaable */
        soc->tslamDmaIntr = NULL;
        if (soc_feature(unit, soc_feature_tslam_dma) &&
            soc_property_get(unit, spn_TSLAM_DMA_ENABLE, 1)) {
            if (SAL_BOOT_QUICKTURN) {
                soc->tslamDmaTimeout = TSLAM_TIMEOUT_QT;
            } else {
                soc->tslamDmaTimeout = TSLAM_TIMEOUT;
            }
            soc->tslamDmaTimeout = soc_property_get(unit, spn_TSLAM_TIMEOUT_USEC,
                                                    soc->tslamDmaTimeout);
            if (soc->tslamDmaTimeout) {
                soc->tslamDmaMutex = sal_mutex_create("TSlamDMA");
                if (soc->tslamDmaMutex == NULL) {
                    SOC_ERROR_PRINT((DK_ERR, "soc_attach: unit %d failed\n", unit));
                    soc_sbx_detach(unit);           /* Perform necessary clean-ups on error */
                    return rv;
                }
                soc->tslamDmaIntr = sal_sem_create("TSLAM interrupt",
                                                   sal_sem_BINARY, 0);
                if (soc->tslamDmaIntr == NULL) {
                    SOC_ERROR_PRINT((DK_ERR, "soc_attach: unit %d failed\n", unit));
                    soc_sbx_detach(unit);           /* Perform necessary clean-ups on error */
                    return rv;
                }
                soc->tslamDmaIntrEnb = soc_property_get(unit,
                                                        spn_TSLAM_INTR_ENABLE, 1);
            }
        }

#ifdef BCM_CMICM_SUPPORT
        soc->ccmDmaMutex = NULL;
        soc->ccmDmaIntr = NULL;
        if (soc_feature(unit, soc_feature_cmicm) &&
            soc_property_get(unit, spn_CCM_DMA_ENABLE, 1)) {
            if (SAL_BOOT_QUICKTURN) {
                soc->ccmDmaTimeout = CCMDMA_TIMEOUT_QT;
            } else {
                soc->ccmDmaTimeout = CCMDMA_TIMEOUT;
            }
            soc->ccmDmaTimeout = soc_property_get(unit, spn_CCMDMA_TIMEOUT_USEC,
                                                  soc->ccmDmaTimeout);
            if (soc->ccmDmaTimeout) {
                soc->ccmDmaMutex = sal_mutex_create("ccmDMA");
                if (soc->ccmDmaMutex == NULL) {
                    return SOC_E_MEMORY;
                }
                soc->ccmDmaIntr = sal_sem_create("CCMDMA interrupt",
                                                 sal_sem_BINARY, 0);
                if (soc->ccmDmaIntr == NULL) {
                    return SOC_E_MEMORY;
                }
                soc->ccmDmaIntrEnb = soc_property_get(unit,
                                                      spn_CCMDMA_INTR_ENABLE, 1);
            }
        }
#endif /* CMICM Support */

        /* mem command */
        if (soc_feature(unit, soc_feature_mem_cmd)) {
            soc->memCmdTimeout = soc_property_get(unit,
                                                  spn_MEMCMD_TIMEOUT_USEC,
                                                  1000000);
            soc->memCmdIntrEnb = soc_property_get(unit,
                                                  spn_MEMCMD_INTR_ENABLE, 0);
            for (ix = 0; ix < 3; ix++) {
                if ((soc->memCmdIntr[ix] =
                     sal_sem_create("MemCmd interrupt", sal_sem_BINARY, 0)) == NULL) {
                    SOC_ERROR_PRINT((DK_ERR, "soc_attach: unit %d failed\n", unit));
                    soc_sbx_detach(unit);           /* Perform necessary clean-ups on error */
                    return rv;
                }
            }
        } else {
            for (ix = 0; ix < 3; ix++) {
                soc->memCmdIntr[ix] = 0;
            }
        }

        /* fifo pop dma */
        /* TODO: rename it for CS block */
        if (soc_feature(unit, soc_feature_fifo_dma)) {
            soc->l2modDmaIntrEnb =
                soc_property_get(unit, spn_L2MOD_DMA_INTR_ENABLE, 0);
        }

        if ((soc->counterMutex = sal_mutex_create("Counter")) == NULL) {
            SOC_ERROR_PRINT((DK_ERR, "soc_attach: unit %d failed\n", unit));
            soc_sbx_detach(unit);           /* Perform necessary clean-ups on error */
            return rv;
        }

        soc->schan_override = 0;

        if (soc_ndev_attached++ == 0) {
            int                     chip;

            /* Work to be done before the first SOC device is attached. */
            for (chip = 0; chip < SOC_NUM_SUPPORTED_CHIPS; chip++) {
                /* Call each chip driver's init function */
                if (soc_base_driver_table[chip]->init) {
                    (soc_base_driver_table[chip]->init)();
                }
            }
        }

        /*
         * Set up port bitmaps.  They are also set up on each soc_init so
         * they can be changed from a CLI without rebooting.
         */

        soc_sbx_info_config(unit, dev_id, dev_id_driver);

        soc_dcb_unit_init(unit);

        /*
         * Initialize memory index_maxes. Chip specific overrides follow.
         */
        for (mem = 0; mem < NUM_SOC_MEM; mem++) {
            if (SOC_MEM_IS_VALID(unit, mem)) {
                sop->memState[mem].index_max = SOC_MEM_INFO(unit, mem).index_max;
            } else {
                sop->memState[mem].index_max = -1;
            }
        }

        /* TODO: soc_sirius_mem_config(unit);
         *       basically fix up memState if required
         */

        for (mem = 0; mem < NUM_SOC_MEM; mem++) {
            if (SOC_MEM_IS_VALID(unit, mem)) {
                uint32      max;
                uint32      max_adj;
                char        mem_name[100];
                char        *mptr;

                max = sop->memState[mem].index_max;

                sal_strcpy(mem_name, "memmax_");
                mptr = &mem_name[sal_strlen(mem_name)];
                sal_strcpy(mptr, SOC_MEM_NAME(unit, mem));
                max_adj = soc_property_get(unit, mem_name, max);
                if (max_adj == max) {
                    sal_strcpy(mptr, SOC_MEM_UFNAME(unit, mem));
                    max_adj = soc_property_get(unit, mem_name, max);
                }
                if (max_adj == max) {
                    sal_strcpy(mptr, SOC_MEM_UFALIAS(unit, mem));
                    max_adj = soc_property_get(unit, mem_name, max);
                }
                sop->memState[mem].index_max = max_adj;
            }
        }

#if defined(BCM_SIRIUS_SUPPORT)
        /* Fixme enable for C3 after port mapping is initialized */
        if (SOC_IS_SBX_SIRIUS(unit)) {
            /* Allocate counter module resources */
            if (soc_counter_attach(unit)) {
                SOC_ERROR_PRINT((DK_ERR, "soc_attach: unit %d failed\n", unit));
                soc_sbx_detach(unit);           /* Perform necessary clean-ups on error */
                return rv;
            }
        }
#endif
        
        /*
         * Create binary semaphores for interrupt signals, initially empty
         * making us block when we try to "take" on them.  In soc_intr(),
         * when we receive the interrupt, a "give" is performed, which will
         * wake us back up.
         */

        if ((soc->schanIntr =
             sal_sem_create("SCHAN interrupt", sal_sem_BINARY, 0)) == NULL) {
            SOC_ERROR_PRINT((DK_ERR, "soc_attach: unit %d failed\n", unit));
            soc_sbx_detach(unit);           /* Perform necessary clean-ups on error */
            return rv;
        }

        if ((soc->miimIntr =
             sal_sem_create("MIIM interrupt", sal_sem_BINARY, 0)) == NULL) {
            SOC_ERROR_PRINT((DK_ERR, "soc_attach: unit %d failed\n", unit));
            soc_sbx_detach(unit);           /* Perform necessary clean-ups on error */
            return rv;
        }

        /* Initialize target device */
        if (SAL_BOOT_QUICKTURN) {
            soc->schanTimeout = SCHAN_TIMEOUT_QT;
            soc->miimTimeout = MIIM_TIMEOUT_QT;
            soc->bistTimeout = BIST_TIMEOUT_QT;
        } else if (SAL_BOOT_PLISIM) {
            soc->schanTimeout = SCHAN_TIMEOUT_PLI;
            soc->miimTimeout = MIIM_TIMEOUT_PLI;
            soc->bistTimeout = BIST_TIMEOUT_PLI;
        } else {
            soc->schanTimeout = SCHAN_TIMEOUT;
            soc->miimTimeout = MIIM_TIMEOUT;
            soc->bistTimeout = BIST_TIMEOUT;
        }

        soc->schanTimeout = soc_property_get(unit, spn_SCHAN_TIMEOUT_USEC,
                                             soc->schanTimeout);
        soc->miimTimeout = soc_property_get(unit, spn_MIIM_TIMEOUT_USEC,
                                            soc->miimTimeout);
        soc->bistTimeout = soc_property_get(unit, spn_BIST_TIMEOUT_MSEC,
                                            soc->bistTimeout);

        soc->schanIntrEnb = soc_property_get(unit, spn_SCHAN_INTR_ENABLE, 1);
        soc->schanIntrBlk = soc_property_get(unit, spn_SCHAN_ERROR_BLOCK_USEC,
                                             250000);

        /* soc->miimIntrEnb = soc_property_get(unit, spn_MIIM_INTR_ENABLE, 1); */
        soc->miimIntrEnb = soc_property_get(unit, spn_MIIM_INTR_ENABLE, 0);
    }
#endif /* defined(BCM_SIRIUS_SUPPORT)*/

    soc->soc_flags |= SOC_F_ATTACHED;

    return SOC_E_NONE;
}


int
soc_sbx_detach(int unit)
{
    soc_control_t       *soc;
    soc_sbx_control_t   *sbx;
    uint32              dev_type;
    soc_mem_t           mem;
    int                 ix;
    uint16              dev_id, dev_id_driver;
    uint8               rev_id, rev_id_driver;

    if (!SOC_UNIT_VALID(unit)) {
        return SOC_E_UNIT;
    }

    soc = SOC_CONTROL(unit);
    if (NULL == soc) {
        return SOC_E_NONE;
    }

    if (!(soc->soc_flags & SOC_F_ATTACHED)) {
        return SOC_E_UNIT;
    }

    soc_cm_get_id(unit, &dev_id, &rev_id);
    soc_cm_get_id_driver(dev_id, rev_id, &dev_id_driver, &rev_id_driver);

    if (SOC_IS_SIRIUS(unit)) {
        

        if (0 == (soc->soc_flags & SOC_F_ATTACHED)) {
            return SOC_E_NONE;
        }

        /* Free up any memory used by the I2C driver */

#ifdef  INCLUDE_I2C
        (void)soc_i2c_detach(unit);
#endif

        /* Clear all outstanding DPCs owned by this unit */
        sal_dpc_cancel(INT_TO_PTR(unit));

        /*
         * Call soc_init to cancel link scan task, counter DMA task,
         * outstanding DMAs, interrupt generation, and anything else the
         * driver or chip may be doing.
         */

#if !defined(PLISIM)
        soc_sbx_reset_init(unit);
#endif

        /*
         * PHY drivers and ID map
         */
        SOC_IF_ERROR_RETURN(soc_phyctrl_software_deinit(unit));

        /* Free up DMA memory */
        (void)soc_dma_detach(unit);

        /* Shutdown polled interrupt mode if active */
        soc_ipoll_disconnect(unit);
        soc->soc_flags &= ~SOC_F_POLLED;

        /* Detach interrupt handler, if we installed one */
        /* unit # is ISR arg */
        if (soc_cm_interrupt_disconnect(unit) < 0) {
            SOC_ERROR_PRINT((DK_ERR,
                             "soc_detach: could not disconnect interrupt line\n"));
            return SOC_E_INTERNAL;
        }

        /* Terminate counter module; frees allocated space */
        soc_counter_detach(unit);

        if (soc->counterMutex) {
            sal_mutex_destroy(soc->counterMutex);
            soc->counterMutex = NULL;
        }

        for (mem = 0; mem < NUM_SOC_MEM; mem++) {
            if (SOC_MEM_IS_VALID(unit, mem)) {
                /* Deallocate table cache memory, if caching enabled */
                (void)soc_mem_cache_set(unit, mem, COPYNO_ALL, FALSE);
            }
            if (soc->memState[mem].lock != NULL) {
                sal_mutex_destroy(soc->memState[mem].lock);
                soc->memState[mem].lock = NULL;
            }
        }

#ifdef INCLUDE_MEM_SCAN
        (void)soc_mem_scan_stop(unit);         /* Stop memory scanner */
        if (soc->mem_scan_notify) {
            sal_sem_destroy(soc->mem_scan_notify);
        }
#endif

        if (soc->schanMutex) {
            sal_mutex_destroy(soc->schanMutex);
            soc->schanMutex = NULL;
        }

        if (soc->schanIntr) {
            sal_sem_destroy(soc->schanIntr);
            soc->schanIntr = NULL;
        }

        if (soc->miimMutex) {
            sal_mutex_destroy(soc->miimMutex);
            soc->miimMutex = NULL;
        }

        if (soc->tableDmaMutex) {
            sal_mutex_destroy(soc->tableDmaMutex);
            soc->tableDmaMutex = NULL;
        }

        if (soc->tslamDmaMutex) {
            sal_mutex_destroy(soc->tslamDmaMutex);
            soc->tslamDmaMutex = NULL;
        }

        if (soc->tableDmaIntr) {
            sal_sem_destroy(soc->tableDmaIntr);
            soc->tableDmaIntr = NULL;
        }

        if (soc->tslamDmaIntr) {
            sal_sem_destroy(soc->tslamDmaIntr);
            soc->tslamDmaIntr = NULL;
        }

        if (soc->miimIntr) {
            sal_sem_destroy(soc->miimIntr);
            soc->miimIntr = NULL;
        }

        for (ix = 0; ix < 3; ix++) {
            if (soc->memCmdIntr[ix]) {
                sal_sem_destroy(soc->memCmdIntr[ix]);
                soc->memCmdIntr[ix] = NULL;
            }
        }

        if (soc->ipfixIntr) {
            sal_sem_destroy(soc->ipfixIntr);
            soc->ipfixIntr = NULL;
        }

        if (soc->socControlMutex) {
            sal_mutex_destroy(soc->socControlMutex);
            soc->socControlMutex = NULL;
        }

        if (soc->egressMeteringMutex) {
            sal_mutex_destroy(soc->egressMeteringMutex);
            soc->egressMeteringMutex = NULL;
        }

        sal_free(SOC_PERSIST(unit));
        SOC_PERSIST(unit) = NULL;

        sal_free(SOC_CONTROL(unit));
        SOC_CONTROL(unit) = NULL;

        if (--soc_ndev_attached == 0) {
            /* Work done after the last SOC device is detached. */
            /* (currently nothing) */
        }
    } else {

        sbx = SOC_SBX_CONTROL(unit);

        /* disable interrupts before detaching */
        soc_sbx_intr_disable(unit, dev_id, ~0);

        /*
         * Detach interrupt handler
         */
        dev_type = soc_cm_get_dev_type(unit);
        if (!(dev_type & SAL_EB_DEV_TYPE)) {
            /* TODO: Not supporting local bus interrupts yet */
            if (soc_cm_interrupt_disconnect(unit) < 0) {
                return SOC_E_INTERNAL;
            }
        }

        /* Terminate software counter collection module; free allocated space */
        soc_sbx_counter_detach(unit);

        /* Destroy MIIM mutex */
        if (soc->miimMutex) {
            sal_mutex_destroy(soc->miimMutex);
            soc->miimMutex = NULL;
        }

        if (sbx != NULL) {
            /*
             * Detach resources allocated by device specific initialization
             */
            if (sbx->dma_sem) {
                sal_sem_destroy(sbx->dma_sem);
                sbx->dma_sem = 0;
            }

            if (sbx->detach) {
                sbx->detach(unit);
            }

            if (sbx->cfg) {
                sal_free(sbx->cfg);
                sbx->cfg = NULL;
            }

            sal_free(sbx);
            sbx = soc->drv = NULL;

            sal_free(SOC_PERSIST(unit));
            SOC_PERSIST(unit) = NULL;

            sal_free(SOC_CONTROL(unit));
            soc = SOC_CONTROL(unit) = NULL;
         }

        if (soc) {
            soc->soc_flags &= ~SOC_F_ATTACHED;
        }
    }


    return SOC_E_NONE;
}



#ifdef BCM_WARM_BOOT_SUPPORT

/*
 * Function:
 *      soc_wb_state_alloc_and_check
 * Purpose:
 *      Allocate an scache handle during a cold boot, or check
 *      version during a warm boot
 * Parameters:
 *     (in)     unit      - bcm device number
 *     (in)     hdl       - scache handle to alloc/validate
 *     (in/out) size      - size to allocate for cold boot
 *                          or size validated for warm boot
 *     (in)     current_version  - current version to store on cold boot
 *                                 or version to verify
 *     (out)    upgrade   - during a warm boot, returns upgrade status
 *                          == 0 : versions match
 *                          >  0 : upgrade detected
 *                          <  0 : downgrade detected
 * Returns:
 *      SOC_E_XXX
 */
int
soc_wb_state_alloc_and_check(int unit, soc_scache_handle_t hdl,
                             uint32 *size, uint32 current_version, 
                             int *upgrade)
{
    int                rv = BCM_E_NONE;
    int                scache_size;
    soc_wb_cache_t    *wbc;

    /* Is Level 2 recovery even available? */
    rv = soc_stable_size_get(unit, &scache_size);
    if (scache_size == 0) {
        return SOC_E_UNAVAIL;
    }
    if (SOC_FAILURE(rv)) {
        return rv;
    }

    /* Allocate a new chunk of scache during a cold boot */
    if (SOC_WARM_BOOT(unit) == FALSE) {
        SOC_IF_ERROR_RETURN(
            soc_scache_alloc(unit, hdl, *size));
    }

    /* Get the pointer for the Level 2 cache */
    wbc = NULL;
    SOC_IF_ERROR_RETURN(
        soc_scache_ptr_get(unit, hdl, (uint8**)&wbc, size));

    if (wbc == NULL) {
        return SOC_E_NOT_FOUND;
    }

    if (SOC_WARM_BOOT(unit) == FALSE) {
        soc_scache_handle_lock(unit, hdl);
        wbc->version = current_version;
        soc_scache_handle_unlock(unit, hdl);
    }

    if (upgrade) {
        *upgrade = current_version - wbc->version;
    }

    return rv;
}


/*
 * Function:
 *      soc_sbx_shutdown
 * Purpose:
 *      Free up SOC resources without touching hardware
 * Parameters:
 *      unit - SBX unit #
 * Returns:
 *      SOC_E_XXX
 */
int
soc_sbx_shutdown(int unit)
{
    return soc_sbx_detach(unit);
}
#endif /* BCM_WARM_BOOT_SUPPORT */


#ifdef BCM_WARM_BOOT_SUPPORT

#define __WB_CHECK_OVERRUN(z,str)					\
    if (ptr+z>end_ptr){							\
      SOC_ERROR_PRINT((DK_ERR, "%s: Incoherent state of scache (%s is not" \
	       "stored correctly). \n", FUNCTION_NAME(), str));		\
      return SOC_E_INTERNAL;						\
    }

/* Following macro used for warm and cold boot */
#define __WB_DECOMPRESS_SCALAR(type, var) do {\
    if (SOC_WARM_BOOT(unit)){				\
      __WB_CHECK_OVERRUN(sizeof(type), #var);		\
      (var) = *(type*)ptr; ptr+=sizeof(type);		\
    }else{						\
      scache_len += sizeof(type);			\
    }							\
  } while(0)

#define __WB_COMPRESS_SCALAR(type, var) do {\
    __WB_CHECK_OVERRUN(sizeof(type), #var);		\
    *(type*)ptr = (var); ptr+=sizeof(type); } while(0)


int
soc_sbx_wb_init(int unit){
  int                 rv = SOC_E_NONE;
  soc_scache_handle_t handle;
  uint32              scache_len;
  uint32              alloc_get;
  uint8               *scache_ptr=0;
  uint8                *ptr=0, *end_ptr=0;
  int a,b,stable_size;

  /* check to see if an scache table has been configured */
  rv = soc_stable_size_get(unit, &stable_size);
  if (SOC_FAILURE(rv) || stable_size <= 0) {
      return rv;
  }

  scache_len = 0;
  SOC_SCACHE_HANDLE_SET(handle, unit, SOC_SBX_WB_MODULE_SOC, 0);
  if (SOC_WARM_BOOT(unit)) {
    /* If device is during warm-boot, recover the state from scache */
    rv = soc_scache_ptr_get(unit, handle, 
			    &scache_ptr, &scache_len);
    if ((rv != SOC_E_NONE) || (scache_ptr == NULL) || (scache_len == 0)) {
      SOC_ERROR_PRINT((DK_ERR, "%s: Error(%s) reading scache. scache_ptr:0x%lx and len:%d\n",
		       FUNCTION_NAME(), soc_errmsg(rv), (unsigned long) scache_ptr, scache_len));
      return rv;
    }
    ptr = scache_ptr;
    end_ptr = scache_ptr + scache_len; /* used for overrun checks*/
  }
  
  /* now decompress */
  __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CONTROL(unit)->module_id0);
  __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CONTROL(unit)->node_id);
  for (a=0;a<SBX_MAX_MODIDS;a++)
    for(b=0;b<SBX_MAX_PORTS;b++)
      __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CONTROL(unit)->modport[a][b]);
  for (a=0;a<SBX_MAX_NODES;a++)
    for(b=0;b<SBX_MAX_PORTS;b++)
      __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CONTROL(unit)->fabnodeport2feport[a][b]);
  for (a=0;a<SBX_MAX_PORTS;a++)
    __WB_DECOMPRESS_SCALAR(sbhandle, SOC_SBX_CONTROL(unit)->fabric_units[a]);
  for (a=0;a<SBX_MAX_PORTS;a++)
    __WB_DECOMPRESS_SCALAR(sbhandle, SOC_SBX_CONTROL(unit)->forwarding_units[a]);

  __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->reset_ul);
  __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->uRedMode);
  __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->uMaxFailedLinks);

  __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->epoch_length_in_timeslots);
  __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->xbar_link_en);
  __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->demand_scale);
  __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->num_queues);
  if (SOC_IS_SBX_QE2000(unit)){
#ifdef BCM_QE2000_SUPPORT
    for (a=0; a<SB_FAB_DEVICE_QE2000_NUM_SERIALIZERS;a++){
      __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG_QE2000(unit)->linkDriverConfig[a].uDriverStrength);
      __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG_QE2000(unit)->linkDriverConfig[a].uDriverEqualization);
    }
#endif
  }else if (SOC_IS_SBX_BME3200(unit)){
#ifdef BCM_BME3200_SUPPORT
    for (a=0; a<SB_FAB_DEVICE_BM3200_NUM_SERIALIZERS;a++){
      __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG_BM3200(unit)->linkDriverConfig[a].uDriverStrength);
      __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG_BM3200(unit)->linkDriverConfig[a].uDriverEqualization);
    }
#endif
  }else if (SOC_IS_SBX_BM9600(unit)){
#ifdef BCM_BM9600_SUPPORT
    for (a=0; a<SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS;a++){
      __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG_BM9600(unit)->linkDriverConfig[a].uDriverStrength);
      __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG_BM9600(unit)->linkDriverConfig[a].uDriverEqualization);
    }
    __WB_DECOMPRESS_SCALAR(uint32, SOC_SBX_CFG_BM9600(unit)->BackupDeviceUnit);
#endif
  }else if (SOC_IS_SBX_SIRIUS(unit)){
#ifdef BCM_SIRIUS_SUPPORT
	
#endif
  }else if (SOC_IS_SBX_FE2000(unit)){

  }else {
    SOC_ERROR_PRINT((DK_ERR, "%s: Missing SOC WB init code\n", FUNCTION_NAME()));
  }
  if (!SOC_WARM_BOOT(unit)){
    /* scache_len now has the size needed */
    SOC_IF_ERROR_RETURN(soc_scache_alloc(unit, handle, scache_len));
    SOC_IF_ERROR_RETURN(soc_scache_ptr_get(unit, handle, 
					   &scache_ptr, &alloc_get));
    if (alloc_get != scache_len) {
      SOC_ERROR_PRINT((DK_ERR, "Size mismatch in requested vs allocated "
		       "Warmboot state for SOC_SBX_WB_MODULE_SOC\n"));
      return SOC_E_INIT;
    }
    if (!scache_ptr) {
      SOC_ERROR_PRINT((DK_ERR, "Invalid Warmboot scache handle "
		       "allocated for SOC_SBX_WB_MODULE_SOC \n"));
      return SOC_E_MEMORY;
    }
  }
  return rv;
}

int
soc_sbx_wb_sync(int unit, int sync)
{
  uint8                   *scache_ptr = NULL;
  uint8                   *ptr, *end_ptr;
  uint32                  scache_len=0;
  int                     rv;
  soc_scache_handle_t     handle;
  int a,b;

  if (SOC_WARM_BOOT(unit)){
    SOC_ERROR_PRINT((DK_ERR, "Cannot write to scache during WarmBoot\n"));
    return SOC_E_INTERNAL;
  }
  SOC_SCACHE_HANDLE_SET(handle, unit, SOC_SBX_WB_MODULE_SOC, 0);
  scache_len = 0;
  rv = soc_scache_ptr_get(unit, handle, &scache_ptr, &scache_len);
  if ((rv != SOC_E_NONE) || (scache_ptr == NULL) || (scache_len == 0)) {
    SOC_ERROR_PRINT((DK_ERR, "%s: Error(%s) reading scache. scache_ptr:0x%lx and len:%d\n",
		     FUNCTION_NAME(), soc_errmsg(rv), (unsigned long)scache_ptr, scache_len));
        return rv;
  }
  ptr = scache_ptr;
  end_ptr = scache_ptr+scache_len;

  /* now compress */
  __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CONTROL(unit)->module_id0);
  __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CONTROL(unit)->node_id);
  for (a=0;a<SBX_MAX_MODIDS;a++)
    for(b=0;b<SBX_MAX_PORTS;b++)
      __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CONTROL(unit)->modport[a][b]);
  for (a=0;a<SBX_MAX_NODES;a++)
    for(b=0;b<SBX_MAX_PORTS;b++)
      __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CONTROL(unit)->fabnodeport2feport[a][b]);
  for (a=0;a<SBX_MAX_PORTS;a++)
    __WB_COMPRESS_SCALAR(sbhandle, SOC_SBX_CONTROL(unit)->fabric_units[a]);
  for (a=0;a<SBX_MAX_PORTS;a++)
    __WB_COMPRESS_SCALAR(sbhandle, SOC_SBX_CONTROL(unit)->forwarding_units[a]);

  __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->reset_ul);
  __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->uRedMode);
  __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->uMaxFailedLinks);

  __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->epoch_length_in_timeslots);
  __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->xbar_link_en);
  __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->demand_scale);
  __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG(unit)->num_queues);
  if (SOC_IS_SBX_QE2000(unit)){
#ifdef BCM_QE2000_SUPPORT
    for (a=0; a<SB_FAB_DEVICE_QE2000_NUM_SERIALIZERS;a++){
      __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG_QE2000(unit)->linkDriverConfig[a].uDriverStrength);
      __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG_QE2000(unit)->linkDriverConfig[a].uDriverEqualization);
    }
#endif
  }else if (SOC_IS_SBX_BME3200(unit)){
#ifdef BCM_BME3200_SUPPORT
    for (a=0; a<SB_FAB_DEVICE_BM3200_NUM_SERIALIZERS;a++){
      __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG_BM3200(unit)->linkDriverConfig[a].uDriverStrength);
      __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG_BM3200(unit)->linkDriverConfig[a].uDriverEqualization);
    }
#endif
  }else if (SOC_IS_SBX_BM9600(unit)){
#ifdef BCM_BM9600_SUPPORT
    for (a=0; a<SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS;a++){
      __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG_BM9600(unit)->linkDriverConfig[a].uDriverStrength);
      __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG_BM9600(unit)->linkDriverConfig[a].uDriverEqualization);
    }
    __WB_COMPRESS_SCALAR(uint32, SOC_SBX_CFG_BM9600(unit)->BackupDeviceUnit);
#endif
  }else if (SOC_IS_SBX_SIRIUS(unit)){
#ifdef BCM_SIRIUS_SUPPORT
	
#endif
  }else {
    SOC_ERROR_PRINT((DK_ERR, "%s: Missing SOC WB init code\n", FUNCTION_NAME()));
  }
  
  if (sync) {
    rv = soc_scache_commit(unit);
    if (rv != SOC_E_NONE) {
      SOC_ERROR_PRINT((DK_ERR, "%s: Error(%s) sync'ing scache to Persistent memory. \n",
		       FUNCTION_NAME(), soc_errmsg(rv)));
      return rv;
    }
  }
  return BCM_E_NONE;
}

#endif /* WARMBOOT */



int
soc_sbx_init(int unit)
{
    soc_control_t        *soc;
    soc_sbx_control_t    *sbx;
    int                   rv;
    uint32                property = 0;
    uint32                port_default_speed = 0, port_speed = 0;
    uint32                max_nodes = 0;
    int                   cos;
    int                   i;
    uint32                requeue_mode;
    uint32                node, modid;
    int                   port=0, spi, hg, intf, level;
    int                   fifoset;
    char                 *ucodestr = NULL;
    int                   nbr_nodes_arbitration_port_allocation;
    int                   flow_control_mode;

    if (!SOC_UNIT_VALID(unit)) {
        SOC_ERROR_PRINT((DK_ERR, "soc_sbx_init: unit %d not valid\n", unit));
        return SOC_E_UNIT;
    }

    if (SOC_WARM_BOOT(unit)) {
        SOC_DEBUG_PRINT((DK_VERBOSE, "Unit %d: warm booting.\n", unit));
    }

    soc = SOC_CONTROL(unit);
    sbx = SOC_SBX_CONTROL(unit);
    assert(soc != NULL);
    assert(sbx != NULL);
    assert(SOC_SBX_CFG(unit) != NULL);

    if (!(soc->soc_flags & SOC_F_ATTACHED)) {
        SOC_ERROR_PRINT((DK_ERR, "soc_sbx_init: unit %d not attached\n", unit));
        return SOC_E_UNIT;
    }

    if (sbx->sbhdl != (sbhandle)unit) {
        sbx->sbhdl = (sbhandle)unit;
    }

#ifdef BCM_WARM_BOOT_SUPPORT
    /* Recover stored Level 2 Warm Boot cache */
    /* The stable and stable size must be selected first */
    if (SOC_WARM_BOOT(unit)) {
        rv = soc_scache_recover(unit);
        if (SOC_FAILURE(rv)) {
            SOC_ERROR_PRINT((DK_ERR, "scache recover failed: %s\n",
                             soc_errmsg(rv)));
            return rv;
        }
    }
#endif

    if (SOC_IS_SIRIUS(unit)) {
        /* reset local state (fail now if not allocated) */
        
        if (SOC_SBX_SIRIUS_STATE(unit)) {
            sal_memset(SOC_SBX_SIRIUS_STATE(unit),
                       0x00,
                       sizeof(*SOC_SBX_SIRIUS_STATE(unit)));
        } else {
            return SOC_E_MEMORY;
        }
    }

    SOC_SBX_CFG(unit)->parse_rx_erh = soc_property_get(unit, spn_RX_PARSE_ERH, 1);
    NUM_COS(unit) = soc_property_get(unit, spn_BCM_NUM_COS, BCM_COS_COUNT);

    /* Default cfg was setup when device was attached
     * here process user defines to modify the default cfg
     */
    SOC_SBX_CFG(unit)->uFabricConfig       = soc_property_get(unit, spn_FABRIC_CONFIGURATION,
                                                              SOC_SBX_CFG(unit)->uFabricConfig);
    SOC_SBX_CFG(unit)->bHalfBus            = soc_property_get(unit, spn_HALF_BUS_MODE,
                                                              SOC_SBX_CFG(unit)->bHalfBus);
    SOC_SBX_CFG(unit)->bRunSelfTest        = soc_property_get(unit, spn_BIST_ENABLE,
                                                              SOC_SBX_CFG(unit)->bRunSelfTest);
    SOC_SBX_CFG(unit)->uLinkThresholdIndex = soc_property_get(unit, spn_LINK_THRESHOLD,
                                                              SOC_SBX_CFG(unit)->uLinkThresholdIndex);
    SOC_SBX_CFG(unit)->uRedMode            = soc_property_get(unit, spn_REDUNDANCY_MODE,
                                                              SOC_SBX_CFG(unit)->uRedMode);
    SOC_SBX_CFG(unit)->uMaxFailedLinks     = soc_property_get(unit, spn_MAX_FAILED_LINKS,
                                                              SOC_SBX_CFG(unit)->uMaxFailedLinks);
    SOC_SBX_CFG(unit)->bHybridMode         = soc_property_get(unit, spn_HYBRID_MODE,
                                                              SOC_SBX_CFG(unit)->bHybridMode);
    SOC_SBX_CFG(unit)->bUcqResourceAllocationMode = soc_property_get(unit, spn_UNICAST_QUEUE_RESOURCE_ALLOCATION_MODE,
								     SOC_SBX_CFG(unit)->bUcqResourceAllocationMode);

    /* flow control mode */
    flow_control_mode = soc_property_get(unit, spn_EGRESS_FIFO_INDEPENDENT_FC, FALSE);

    /* additional consistency checks - flow control mode */
    if (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_DMODE) {
        if (flow_control_mode != SOC_SBX_SYSTEM_UNICAST_NIFC_MULTICAST_NIFC) {
            SOC_ERROR_PRINT((DK_ERR, "Invalid FlowCtrlMode %d unit %d fabricMode:%d, setting it to default-0\n",
                                       flow_control_mode, unit, SOC_SBX_CFG(unit)->uFabricConfig));
            flow_control_mode = SOC_SBX_SYSTEM_UNICAST_NIFC_MULTICAST_NIFC;
        }
    }
    else if ((SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_LEGACY) ||
                       (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_MIX)) {
        if ((flow_control_mode != SOC_SBX_SYSTEM_UNICAST_NIFC_MULTICAST_NIFC) &&
                          (flow_control_mode != SOC_SBX_SYSTEM_UNICAST_IFC_MULTICAST_IFC)) {
            SOC_ERROR_PRINT((DK_ERR, "Invalid FlowCtrlMode %d unit %d fabricMode:%d, setting it to default-0\n",
                                       flow_control_mode, unit, SOC_SBX_CFG(unit)->uFabricConfig));
            flow_control_mode = SOC_SBX_SYSTEM_UNICAST_NIFC_MULTICAST_NIFC;
        }

        
        if (flow_control_mode == SOC_SBX_SYSTEM_UNICAST_IFC_MULTICAST_IFC) {
            flow_control_mode = SOC_SBX_SYSTEM_UNICAST_IFC_MULTICAST_NIFC;
        }
    }

    /* flow control mode - setting */
    switch (flow_control_mode) {
        case SOC_SBX_SYSTEM_UNICAST_NIFC_MULTICAST_NIFC:
            SOC_SBX_CFG(unit)->bEgressFifoIndependentFlowControl = FALSE;
            SOC_SBX_CFG(unit)->bEgressMulticastFifoIndependentFlowControl = FALSE;
            break;
        case SOC_SBX_SYSTEM_UNICAST_IFC_MULTICAST_IFC:
            SOC_SBX_CFG(unit)->bEgressFifoIndependentFlowControl = TRUE;
            SOC_SBX_CFG(unit)->bEgressMulticastFifoIndependentFlowControl = TRUE;
            break;
        case SOC_SBX_SYSTEM_UNICAST_NIFC_MULTICAST_IFC:
            SOC_SBX_CFG(unit)->bEgressFifoIndependentFlowControl = FALSE;
            SOC_SBX_CFG(unit)->bEgressMulticastFifoIndependentFlowControl = TRUE;
            break;
        case SOC_SBX_SYSTEM_UNICAST_IFC_MULTICAST_NIFC:
            SOC_SBX_CFG(unit)->bEgressFifoIndependentFlowControl = TRUE;
            SOC_SBX_CFG(unit)->bEgressMulticastFifoIndependentFlowControl = FALSE;
            break;
        default:
            SOC_ERROR_PRINT((DK_ERR, "Invalid FlowCtrlMode %d unit %d, setting it to default-0\n",
                                       flow_control_mode, unit));
            SOC_SBX_CFG(unit)->bEgressFifoIndependentFlowControl = FALSE;
            SOC_SBX_CFG(unit)->bEgressMulticastFifoIndependentFlowControl = FALSE;
            break;
    }

    SOC_SBX_CFG(unit)->bTmeMode            = soc_property_get(unit, spn_QE_TME_MODE,
                                                              SOC_SBX_CFG(unit)->bTmeMode);
    SOC_SBX_CFG(unit)->fabric_egress_setup = soc_property_get(unit, spn_FABRIC_EGRESS_SETUP,
                                                              SOC_SBX_CFG(unit)->fabric_egress_setup);
    SOC_SBX_CFG(unit)->sp_mode             = soc_property_get(unit, spn_BCM_COSQ_SP_MODE,
							      SOC_SBX_CFG(unit)->sp_mode);
    SOC_SBX_CFG(unit)->local_template_id   = soc_property_get(unit, spn_TM_LOCAL_SCHED_DISCIPLINE_TEMPLATE,
							      SOC_SBX_CFG(unit)->local_template_id);
    SOC_SBX_CFG(unit)->node_template_id    = soc_property_get(unit, spn_TM_SCHED_DISCIPLINE_TEMPLATE,
							      SOC_SBX_CFG(unit)->node_template_id);

    SOC_SBX_CFG(unit)->erh_type = soc_property_get(unit, spn_QE_ERH_TYPE, 0);

    if ( (SOC_SBX_CFG(unit)->sp_mode != SOC_SBX_SP_MODE_IN_BAG) &&
         (SOC_SBX_CFG(unit)->sp_mode != SOC_SBX_SP_MODE_ACCOUNT_IN_BAG) ) {
        SOC_ERROR_PRINT((DK_ERR, "Invalid SP mode %d unit %d\n",
                         SOC_SBX_CFG(unit)->sp_mode, unit));

        /* set it to a default value */
        SOC_SBX_CFG(unit)->sp_mode = SOC_SBX_SP_MODE_IN_BAG;
    }

    SOC_SBX_CFG(unit)->nMaxFabricPortsOnModule = soc_property_get(unit,
                 (spn_NUM_MAX_FABRIC_PORTS_ON_MODULE), SOC_SBX_CFG(unit)->nMaxFabricPortsOnModule);

    SOC_SBX_CFG(unit)->fixed_demand_scale = soc_property_get(unit,
                           (spn_DEMAND_SCALE), SOC_SBX_CFG(unit)->fixed_demand_scale);
    if (SOC_SBX_CFG(unit)->fixed_demand_scale != -1) {
        SOC_SBX_CFG(unit)->is_demand_scale_fixed = TRUE;

        if (SOC_SBX_CFG(unit)->fixed_demand_scale > 7) {
            SOC_ERROR_PRINT((DK_ERR, "Invalid Demand Scale %d unit %d\n",
                                       SOC_SBX_CFG(unit)->fixed_demand_scale, unit));
            /* set it to a default value */
            SOC_SBX_CFG(unit)->fixed_demand_scale = 3;
        }

        SOC_SBX_CFG(unit)->demand_scale = SOC_SBX_CFG(unit)->fixed_demand_scale;
    }

    if (SOC_IS_SBX_FE2KXT(unit) || SOC_IS_SBX_FE2000(unit)) {
      SOC_SBX_CFG_FE2000(unit)->l2_age_cycles =  soc_property_get(unit, spn_L2_AGE_CYCLES,
								  L2_AGE_CYCLES_INTERVAL_DEFAULT);
      
      SOC_SBX_CFG_FE2000(unit)->l2_cache_max_idx = soc_property_get(unit, spn_L2CACHE_MAX,
								    L2CACHE_MAX_IDX_DEFAULT);
      
      SOC_SBX_CFG_FE2000(unit)->oam_q_depth =  soc_property_get(unit,
								"bcm_oam_expired_timer_q_depth",
								OAM_EXPIRED_TIMER_MAX_DEPTH_DFLT);
      
      SOC_SBX_CFG_FE2000(unit)->oam_thread_pri =  soc_property_get(unit, spn_BCM_OAM_THREAD_PRI,
                                                                 OAM_THREAD_PRI_DFLT);
    }

    /* Default support V4 and MiM on FE2KXT device */
    /* Only support MiM and V6 on BCM88025  FE2KXT device */
    if (SOC_IS_SBX_FE2KXT(unit)) {
      SOC_SBX_CFG(unit)->v4_ena = soc_property_get(unit, spn_L3_ENABLE, 1);
      SOC_SBX_CFG(unit)->v6_ena = soc_property_get(unit, spn_IPV6_ENABLE, 0);
      SOC_SBX_CFG(unit)->mim_ena =  soc_property_get(unit, spn_MIM_ENABLE, 1);
      SOC_SBX_CFG(unit)->dscp_ena = 1;
      ucodestr = soc_property_get_str(unit,spn_BCM88025_UCODE);
      if (ucodestr != NULL && (sal_strcmp(ucodestr, "c2mplstp") == 0)) {
        SOC_SBX_CFG(unit)->mplstp_ena = 1;
      } else {
        SOC_SBX_CFG(unit)->mplstp_ena = 0;
      }
    } else {
      SOC_SBX_CFG(unit)->v4_ena = soc_property_get(unit, spn_L3_ENABLE, 0);
      SOC_SBX_CFG(unit)->v6_ena = 0;
      SOC_SBX_CFG(unit)->mim_ena = 0;
      ucodestr = soc_property_get_str(unit,spn_BCM88020_UCODE);
      if (ucodestr != NULL && (sal_strcmp(ucodestr, "g2p3_dscp") == 0)) {
        SOC_SBX_CFG(unit)->dscp_ena = 1;
      } else {
        SOC_SBX_CFG(unit)->dscp_ena = 0;
      }
    }

    SOC_SBX_CFG(unit)->oam_rx_ena = soc_property_get(unit, spn_OAM_RX_ENABLE, 1);
    SOC_SBX_CFG(unit)->oam_tx_ena = soc_property_get(unit, spn_OAM_TX_ENABLE, 1);
    SOC_SBX_CFG(unit)->oam_spi_lb_port = soc_property_get(unit, spn_OAM_SPI_LB_PORT,
                                           SBX_OAM_SPI_LB_PORT);
    SOC_SBX_CFG(unit)->oam_spi_lb_queue = soc_property_get(unit, spn_OAM_SPI_LB_QUEUE,
                                            SBX_OAM_SPI_LB_QUEUE);
    SOC_SBX_CFG(unit)->v4mc_str_sel = soc_property_get(unit, spn_V4MC_STR_SEL, 0);
    SOC_SBX_CFG(unit)->v4uc_str_sel = soc_property_get(unit, spn_V4UC_STR_SEL, 0);

    /* These values are system maximums for fabric devices.
     * Initialize these on all devices to the maximum system wide value.
     */
    switch (SOC_SBX_CFG(unit)->uFabricConfig) {
        case SOC_SBX_SYSTEM_CFG_DMODE: /* DMode (Default) - Bm3200 + Qe2000 */
        default:
            SOC_SBX_CFG(unit)->num_nodes = SB_FAB_DEVICE_BM3200_MAX_NODES;
            SOC_SBX_CFG(unit)->num_ds_ids = SB_FAB_DEVICE_BM3200_MAX_DS_IDS;
            SOC_SBX_CFG(unit)->num_queues = HW_QE2000_MAX_QUEUES;
            SOC_SBX_CFG(unit)->num_bw_groups = HW_BM3200_PT_MAX_DMODE_VIRTUAL_PORTS;
            if (SOC_SBX_CFG(unit)->bHybridMode == FALSE) {
                SOC_SBX_CFG(unit)->num_sysports = 0;
            } else if (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_DMODE) {
                SOC_SBX_CFG(unit)->num_sysports =
                         SB_FAB_DEVICE_BM3200_MAX_NODES * SB_FAB_DEVICE_QE2000_MAX_PORT;
            }
            max_nodes = SB_FAB_DEVICE_BM3200_MAX_NODES;

            /* update num_ds_ids to account for epoch optimizations */
            SOC_SBX_CFG(unit)->num_ds_ids = soc_property_get(unit, spn_BME_NUM_ESETS, SOC_SBX_CFG(unit)->num_ds_ids);
            if (SOC_SBX_CFG(unit)->num_ds_ids > SB_FAB_DEVICE_BM3200_MAX_DS_IDS) {
                SOC_SBX_CFG(unit)->num_ds_ids = SB_FAB_DEVICE_BM3200_MAX_DS_IDS;
            }
            SOC_SBX_CFG(unit)->num_internal_ds_ids = SOC_SBX_CFG(unit)->num_ds_ids;

            break;

        case SOC_SBX_SYSTEM_CFG_VPORT:        /* Vport - Bm9600 + Qe4000 */
            /* TODO: Revisit system maximums when Qe4000 support are added.
             *   For now, assuming it's same as Legacy mode
             */
            SOC_SBX_CFG(unit)->num_nodes = SB_FAB_DEVICE_BM9600_MAX_NODES;
            SOC_SBX_CFG(unit)->num_ds_ids = soc_property_get(unit, spn_BME_NUM_ESETS, SB_FAB_DEVICE_BM9600_MAX_DS_IDS);
            if (SOC_SBX_CFG(unit)->bEgressMulticastFifoIndependentFlowControl == FALSE) {
                if (SOC_SBX_CFG(unit)->num_ds_ids > BM9600_MAX_MULTICAST_ESETS) {
                    SOC_SBX_CFG(unit)->use_extended_esets = 0;
                }
                SOC_SBX_CFG(unit)->num_internal_ds_ids = SOC_SBX_CFG(unit)->num_ds_ids;
            }
            else if (SOC_SBX_CFG(unit)->bEgressMulticastFifoIndependentFlowControl == TRUE) {
                if (SOC_SBX_CFG(unit)->num_ds_ids > (BM9600_MAX_MULTICAST_EXTENDED_ESETS / 2)) {
		    if (soc_property_get(unit, spn_BME_NUM_ESETS, 0) > (BM9600_MAX_MULTICAST_EXTENDED_ESETS / 2)) {
			SOC_ERROR_PRINT((DK_ERR, "Invalid ds_ids %d unit %d, setting it to %d\n",
					 SOC_SBX_CFG(unit)->num_ds_ids, unit, BM9600_MAX_MULTICAST_EXTENDED_ESETS/2));
		    }
                    SOC_SBX_CFG(unit)->num_ds_ids = BM9600_MAX_MULTICAST_EXTENDED_ESETS / 2;
                }
                if ((SOC_SBX_CFG(unit)->num_ds_ids * 2) > BM9600_MAX_MULTICAST_ESETS) {
                    SOC_SBX_CFG(unit)->use_extended_esets = 0;
                }
                SOC_SBX_CFG(unit)->num_internal_ds_ids = SOC_SBX_CFG(unit)->num_ds_ids * 2;
            }
            SOC_SBX_CFG(unit)->num_sysports = BM9600_MAX_NUM_SYSPORTS;
            if ((SOC_SBX_CFG(unit)->bHybridMode) ||
		(SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME_BYPASS) ||
		(SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME)) {
                SOC_SBX_CFG(unit)->num_queues = SB_FAB_DEVICE_SIRIUS_NUM_QUEUES;
            } else {
                SOC_SBX_CFG(unit)->num_queues = BM9600_BW_MAX_VOQ_NUM;
            }

            if ((SOC_SBX_CFG(unit)->bHybridMode) && (SOC_IS_SIRIUS(unit))) {
                SOC_SBX_CFG(unit)->num_sysports = 4096; /* SIRIUS_TS_LOCAL_SYSPORT_BASE + 512 */
            }
            SOC_SBX_CFG(unit)->num_bw_groups = BM9600_BW_MAX_BAG_NUM;
            max_nodes = SB_FAB_DEVICE_BM9600_MAX_NODES;
            break;
        case SOC_SBX_SYSTEM_CFG_VPORT_LEGACY: /* Vport Legacy - Bm9600 + Qe2000 */
            SOC_SBX_CFG(unit)->num_nodes = SB_FAB_DEVICE_BM9600_MAX_NODES;
            SOC_SBX_CFG(unit)->num_ds_ids = soc_property_get(unit, spn_BME_NUM_ESETS, SB_FAB_DEVICE_BM9600_MAX_DS_IDS);
            if (SOC_SBX_CFG(unit)->bEgressMulticastFifoIndependentFlowControl == FALSE) {
                if (SOC_SBX_CFG(unit)->num_ds_ids > BM9600_MAX_MULTICAST_ESETS) {
                    SOC_SBX_CFG(unit)->use_extended_esets = 1;
                }
                SOC_SBX_CFG(unit)->num_internal_ds_ids = SOC_SBX_CFG(unit)->num_ds_ids;
            }
            else if (SOC_SBX_CFG(unit)->bEgressMulticastFifoIndependentFlowControl == TRUE) {
                if (SOC_SBX_CFG(unit)->num_ds_ids > (BM9600_MAX_MULTICAST_EXTENDED_ESETS / 2)) {
		    if (soc_property_get(unit, spn_BME_NUM_ESETS, 0) > (BM9600_MAX_MULTICAST_EXTENDED_ESETS / 2)) {
			SOC_ERROR_PRINT((DK_ERR, "Invalid ds_ids %d unit %d, setting it to %d\n",
					 SOC_SBX_CFG(unit)->num_ds_ids, unit, BM9600_MAX_MULTICAST_EXTENDED_ESETS/2));
		    }
                    SOC_SBX_CFG(unit)->num_ds_ids = BM9600_MAX_MULTICAST_EXTENDED_ESETS / 2;
                }
                if ((SOC_SBX_CFG(unit)->num_ds_ids * 2) > BM9600_MAX_MULTICAST_ESETS) {
                    SOC_SBX_CFG(unit)->use_extended_esets = 1;
                }
                SOC_SBX_CFG(unit)->num_internal_ds_ids = SOC_SBX_CFG(unit)->num_ds_ids * 2;
            }
            SOC_SBX_CFG(unit)->num_sysports = BM9600_MAX_NUM_SYSPORTS;
            if ((SOC_SBX_CFG(unit)->bHybridMode) ||
		(SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME_BYPASS) ||
		(SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME)) {
                if (SOC_IS_SBX_QE2000(unit)) {
                    SOC_SBX_CFG(unit)->num_queues = HW_QE2000_MAX_QUEUES;
                } else {
		    SOC_SBX_CFG(unit)->num_queues = BM9600_BW_MAX_VOQ_NUM;
		}
            } else {
                SOC_SBX_CFG(unit)->num_queues = BM9600_BW_MAX_VOQ_NUM;
            }
            if (SOC_SBX_CFG(unit)->bHybridMode) {
                SOC_SBX_CFG(unit)->num_sysports = 4096; /* SIRIUS_TS_LOCAL_SYSPORT_BASE + 512 */
            }
            SOC_SBX_CFG(unit)->num_bw_groups = BM9600_BW_MAX_BAG_NUM;
            max_nodes = SB_FAB_DEVICE_BM3200_MAX_NODES;
            break;
        case SOC_SBX_SYSTEM_CFG_VPORT_MIX: /* Vport Legacy - Bm9600 + Qe2000 + Qe4000 */
            SOC_SBX_CFG(unit)->num_nodes = SB_FAB_DEVICE_BM9600_MAX_NODES;
            SOC_SBX_CFG(unit)->num_ds_ids = soc_property_get(unit, spn_BME_NUM_ESETS, SB_FAB_DEVICE_BM9600_MAX_DS_IDS);
            if (SOC_SBX_CFG(unit)->bEgressMulticastFifoIndependentFlowControl == FALSE) {
                if (SOC_SBX_CFG(unit)->num_ds_ids > BM9600_MAX_MULTICAST_ESETS) {
                    SOC_SBX_CFG(unit)->use_extended_esets = 1;
                }
                SOC_SBX_CFG(unit)->num_internal_ds_ids = SOC_SBX_CFG(unit)->num_ds_ids;
            }
            else if (SOC_SBX_CFG(unit)->bEgressMulticastFifoIndependentFlowControl == TRUE) {
                if (SOC_SBX_CFG(unit)->num_ds_ids > (BM9600_MAX_MULTICAST_EXTENDED_ESETS / 2)) {
		    if (soc_property_get(unit, spn_BME_NUM_ESETS, 0) > (BM9600_MAX_MULTICAST_EXTENDED_ESETS / 2)) {
			/* only complain when user is tring to config this way, otherwise quietly change it */
			SOC_ERROR_PRINT((DK_ERR, "Invalid ds_ids %d unit %d, setting it to %d\n",
					 SOC_SBX_CFG(unit)->num_ds_ids, unit, BM9600_MAX_MULTICAST_EXTENDED_ESETS/2));
		    }
                    SOC_SBX_CFG(unit)->num_ds_ids = BM9600_MAX_MULTICAST_EXTENDED_ESETS / 2;
                }
                if ((SOC_SBX_CFG(unit)->num_ds_ids * 2) > BM9600_MAX_MULTICAST_ESETS) {
                    SOC_SBX_CFG(unit)->use_extended_esets = 1;
                }
                SOC_SBX_CFG(unit)->num_internal_ds_ids = SOC_SBX_CFG(unit)->num_ds_ids * 2;
            }
            if ((SOC_SBX_CFG(unit)->bHybridMode) ||
		(SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME_BYPASS) ||
		(SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME)) {
                if (SOC_IS_SBX_QE2000(unit)) {
                    SOC_SBX_CFG(unit)->num_queues = HW_QE2000_MAX_QUEUES;
                } else if (SOC_IS_SIRIUS(unit)) {
                    SOC_SBX_CFG(unit)->num_queues = SB_FAB_DEVICE_SIRIUS_NUM_QUEUES;
                } else {
		    SOC_SBX_CFG(unit)->num_queues = BM9600_BW_MAX_VOQ_NUM;
		}
            } else {
                SOC_SBX_CFG(unit)->num_queues = BM9600_BW_MAX_VOQ_NUM;
            }
            SOC_SBX_CFG(unit)->num_bw_groups = BM9600_BW_MAX_BAG_NUM;
            SOC_SBX_CFG(unit)->num_sysports = BM9600_MAX_NUM_SYSPORTS;
            max_nodes = SB_FAB_DEVICE_BM3200_MAX_NODES;
            if ((SOC_SBX_CFG(unit)->bHybridMode) && (SOC_IS_SIRIUS(unit))) {
                SOC_SBX_CFG(unit)->num_sysports = 4096; /* SIRIUS_TS_LOCAL_SYSPORT_BASE + 512 */
            }
            break;
    }

    if (SOC_IS_SBX_SIRIUS(unit) && soc_feature(unit, soc_feature_standalone)) {

        SOC_SBX_CFG(unit)->num_nodes = SB_FAB_DEVICE_BM9600_MAX_NODES;
        SOC_SBX_CFG(unit)->num_ds_ids = soc_property_get(unit,
                                               spn_BME_NUM_ESETS,
                                               SB_FAB_DEVICE_BM9600_MAX_DS_IDS);

        
        if (SOC_SBX_CFG(unit)->num_ds_ids > BM9600_MAX_MULTICAST_ESETS) {
            SOC_SBX_CFG(unit)->use_extended_esets = 1;
            SOC_SBX_CFG(unit)->num_internal_ds_ids = SOC_SBX_CFG(unit)->num_ds_ids;
        }
        if ((SOC_SBX_CFG(unit)->bHybridMode) ||
	    (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME_BYPASS) ||
	    (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME)) {
            SOC_SBX_CFG(unit)->num_queues = SB_FAB_DEVICE_SIRIUS_NUM_QUEUES;
	    SOC_SBX_CFG(unit)->num_bw_groups = SOC_SBX_CFG(unit)->num_queues / NUM_COS(unit);
        } else {
            SOC_SBX_CFG(unit)->num_queues = BM9600_BW_MAX_VOQ_NUM;
	    SOC_SBX_CFG(unit)->num_bw_groups = BM9600_BW_MAX_BAG_NUM;
        }

        SOC_SBX_CFG(unit)->num_sysports = BM9600_MAX_NUM_SYSPORTS;
        max_nodes = SB_FAB_DEVICE_BM9600_MAX_NODES;
    }

    SOC_SBX_CFG(unit)->cfg_num_nodes = soc_property_get(unit, spn_NUM_MODULES, max_nodes);

    /* fill in the default node mask */
    SOC_SBX_CFG(unit)->cfg_node_00_31_mask = SOC_SBX_CFG(unit)->cfg_node_32_63_mask =
                                                 SOC_SBX_CFG(unit)->cfg_node_64_95_mask = 0;
    for (i = 0; i < SOC_SBX_CFG(unit)->cfg_num_nodes; i++) {
        if (i < (sizeof(uint32) * 8)) {
            SOC_SBX_CFG(unit)->cfg_node_00_31_mask |= (1 << i);
        }
        else if (i < ((sizeof(uint32) * 8) * 2)) {
            SOC_SBX_CFG(unit)->cfg_node_32_63_mask |= (1 << (i - (sizeof(uint32) * 8)));
        }
        else {
            SOC_SBX_CFG(unit)->cfg_node_64_95_mask |= (1 << (i - ((sizeof(uint32) * 8) * 2)));
        }
    }
    /* update node mask from the configuration file */
    /* NOTE: currently no consistency checks are done */
    SOC_SBX_CFG(unit)->cfg_node_00_31_mask = soc_property_get(unit, spn_NUM_MODULES_00_31_MASK,
                                                        SOC_SBX_CFG(unit)->cfg_node_00_31_mask);
    SOC_SBX_CFG(unit)->cfg_node_32_63_mask = soc_property_get(unit, spn_NUM_MODULES_32_63_MASK,
                                                        SOC_SBX_CFG(unit)->cfg_node_32_63_mask);
    SOC_SBX_CFG(unit)->cfg_node_64_95_mask = soc_property_get(unit, spn_NUM_MODULES_64_95_MASK,
                                                        SOC_SBX_CFG(unit)->cfg_node_64_95_mask);

    SOC_SBX_CFG(unit)->uActiveScId         = soc_property_get(unit, spn_ACTIVE_SWITCH_CONTROLLER_ID,
                                                              SOC_SBX_CFG(unit)->uActiveScId);
    /* discard configuration parameters */
    SOC_SBX_CFG(unit)->discard_probability_mtu = soc_property_get(unit, spn_DISCARD_MTU_SIZE,
                                                      SOC_SBX_CFG(unit)->discard_probability_mtu);
    SOC_SBX_CFG(unit)->discard_queue_size = soc_property_get(unit, spn_DISCARD_QUEUE_SIZE,
                                                      SOC_SBX_CFG(unit)->discard_queue_size);

    soc_sbx_set_epoch_length(unit);

    SOC_SBX_CFG(unit)->hold_pri_num_timeslots = soc_property_get(unit, spn_HOLD_PRI_NUM_TIMESLOTS, SOC_SBX_CFG(unit)->hold_pri_num_timeslots);

    SOC_SBX_CFG(unit)->uSerdesSpeed           = soc_property_get(unit, spn_BACKPLANE_SERDES_SPEED,
                                                                 SOC_SBX_CFG(unit)->uSerdesSpeed);
    SOC_SBX_CFG(unit)->bSerdesEncoding        = soc_property_get(unit, spn_BACKPLANE_SERDES_ENCODING,
                                                                 SOC_SBX_CFG(unit)->bSerdesEncoding);

    nbr_nodes_arbitration_port_allocation = soc_property_get(unit, spn_ARBITRATION_PORT_MAX_NODES, -1);
    if (nbr_nodes_arbitration_port_allocation != -1) {
        if ( (nbr_nodes_arbitration_port_allocation != SOC_SBX_SYSTEM_NBR_NODES_ARBITRATION_PORT_ALLOCATION1) &&
                (nbr_nodes_arbitration_port_allocation != SOC_SBX_SYSTEM_NBR_NODES_ARBITRATION_PORT_ALLOCATION2) ) {
            SOC_ERROR_PRINT((DK_ERR, "Invalid Max Nodes for Arbitration Port Allocation Scheme %d unit %d, resetting it to default\n",
                                       nbr_nodes_arbitration_port_allocation, unit));
            nbr_nodes_arbitration_port_allocation = -1;
        }
        else {
            SOC_SBX_CFG(unit)->arbitration_port_allocation =
              (nbr_nodes_arbitration_port_allocation ==
                             SOC_SBX_SYSTEM_NBR_NODES_ARBITRATION_PORT_ALLOCATION1) ?
                               SOC_SBX_SYSTEM_ARBITRATION_PORT_ALLOCATION1 :
                               SOC_SBX_SYSTEM_ARBITRATION_PORT_ALLOCATION2;
        }
    }
    if (nbr_nodes_arbitration_port_allocation == -1) {
        /* currently the default is SOC_SBX_SYSTEM_ARBITRATION_PORT_ALLOCATION1 */
#if 0
        if (SOC_SBX_CFG(unit)->cfg_num_nodes > 32) {
            SOC_SBX_CFG(unit)->arbitration_port_allocation =
                                                SOC_SBX_SYSTEM_ARBITRATION_PORT_ALLOCATION1;
        }
        else {
            SOC_SBX_CFG(unit)->arbitration_port_allocation =
                                                SOC_SBX_SYSTEM_ARBITRATION_PORT_ALLOCATION2;
        }
#endif /* 0 */
    }

    SOC_SBX_CFG(unit)->diag_qe_revid = soc_property_get(unit, "diag_qe_revid",
							SOC_SBX_CFG(unit)->diag_qe_revid);

    if ( SOC_IS_SBX_QE2000(unit) || SOC_IS_SBX_SIRIUS(unit) ) {
        for (cos = 0; cos < SBX_MAX_FABRIC_COS; cos++) {
            SOC_SBX_CFG(unit)->connect_min_util[cos] = soc_property_cos_get(unit, cos,
                  spn_FABRIC_CONNECT_MIN_UTILIZATION, SOC_SBX_CFG(unit)->connect_min_util[cos]);
            SOC_SBX_CFG(unit)->connect_max_age_time[cos] = soc_property_cos_get(unit, cos,
                  spn_FABRIC_CONNECT_MAX_AGE_TIME, SOC_SBX_CFG(unit)->connect_max_age_time[cos]);
        }
    }

    if (SOC_IS_SBX_QE2000(unit)) {
        /* Fill in QE2000 specific configuration */
        /* modify the configuration set earlier */
        if (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_DMODE) {
            if (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME) {
                SOC_SBX_CFG(unit)->num_sysports = 0;
            } else if (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_HYBRID) {
                SOC_SBX_CFG_QE2000(unit)->uSfiTimeslotOffsetInClocks = 0xc0;
            }
        } else if (SOC_SBX_CFG(unit)->uFabricConfig == SOC_SBX_SYSTEM_CFG_VPORT_MIX) {
	    SOC_SBX_CFG_QE2000(unit)->uSfiTimeslotOffsetInClocks = 0xfa;
	}

        SOC_SBX_CFG(unit)->uClockSpeedInMHz              = soc_property_get(unit, spn_QE_CLOCK_SPEED,
                                                                            SOC_SBX_CFG(unit)->uClockSpeedInMHz);
        SOC_SBX_CFG_QE2000(unit)->bHalfBus               = soc_property_get(unit, spn_QE_HALF_BUS_MODE,
                                                                            SOC_SBX_CFG_QE2000(unit)->bHalfBus);
        SOC_SBX_CFG_QE2000(unit)->bRunLongDdrMemoryTest  = soc_property_get(unit, spn_QE_LONG_DDR_MEMTEST,
                                                                            SOC_SBX_CFG_QE2000(unit)->bRunLongDdrMemoryTest);
        SOC_SBX_CFG_QE2000(unit)->bQm512MbDdr2           = soc_property_get(unit, spn_QE_MEMORY_PART,
                                                                            SOC_SBX_CFG_QE2000(unit)->bQm512MbDdr2);
        SOC_SBX_CFG_QE2000(unit)->bSv2_5GbpsLinks        = soc_property_get(unit, spn_QE_2_5GBPS_LINKS,
                                                                            SOC_SBX_CFG_QE2000(unit)->bSv2_5GbpsLinks);
        SOC_SBX_CFG_QE2000(unit)->uEgMVTSize             = soc_property_get(unit, spn_QE_EG_MVT_SIZE,
                                                                            SOC_SBX_CFG_QE2000(unit)->uEgMVTSize);

        switch (SOC_SBX_CFG_QE2000(unit)->uEgMVTSize) {
            case 0:
                SOC_SBX_CFG(unit)->mcgroup_local_start_index = SBX_MVT_ID_12K_LOCAL_BASE;
                break;

            case 1:
                SOC_SBX_CFG(unit)->mcgroup_local_start_index = SBX_MVT_ID_24K_LOCAL_BASE;
                break;

            case 2:
                SOC_SBX_CFG(unit)->mcgroup_local_start_index = SBX_MVT_ID_LOCAL_BASE;
                break;

            default:
                SOC_ERROR_PRINT((DK_ERR, "Invalid MVT size %d unit %d\n",
                                        SOC_SBX_CFG_QE2000(unit)->uEgMVTSize, unit));
                SOC_SBX_CFG(unit)->mcgroup_local_start_index = SBX_MVT_ID_LOCAL_BASE;
                SOC_SBX_CFG_QE2000(unit)->uEgMVTSize = 2;
                break;
        }

        SOC_SBX_CFG_QE2000(unit)->bVirtualPortFairness = soc_property_get(unit,"subscriber_qos_excess_mode", SOC_SBX_CFG_QE2000(unit)->bVirtualPortFairness);


        SOC_SBX_CFG_QE2000(unit)->uEgMcDropOnFull        = soc_property_get(unit, spn_QE_EG_MC_DROP_ON_FULL,
                                                                            SOC_SBX_CFG_QE2000(unit)->uEgMcDropOnFull);

        SOC_SBX_CFG_QE2000(unit)->uEgMvtFormat           = soc_property_get(unit, spn_QE_MVT_FORMAT,
                                                                            SOC_SBX_CFG_QE2000(unit)->uEgMvtFormat);
        if ((SOC_SBX_CFG_QE2000(unit)->uEgMvtFormat != SBX_MVT_FORMAT0) && (SOC_SBX_CFG_QE2000(unit)->uEgMvtFormat != SBX_MVT_FORMAT1) ) {
            SOC_SBX_CFG_QE2000(unit)->uEgMvtFormat = SBX_MVT_FORMAT0;
        }

        SOC_SBX_CFG_QE2000(unit)->uEgMcEfTimeout = soc_property_get(unit, spn_EGRESS_MC_EF_TIMEOUT,
                                                                       SOC_SBX_CFG_QE2000(unit)->uEgMcEfTimeout);
        SOC_SBX_CFG_QE2000(unit)->uEgMcNefTimeout = soc_property_get(unit, spn_EGRESS_MC_NEF_TIMEOUT,
                                                                       SOC_SBX_CFG_QE2000(unit)->uEgMcNefTimeout);

        SOC_SBX_CFG_QE2000(unit)->uEiPortInactiveTimeout = soc_property_get(unit, spn_QE_EI_PORT_TIMEOUT,
                                                                            SOC_SBX_CFG_QE2000(unit)->uEiPortInactiveTimeout);
        SOC_SBX_CFG_QE2000(unit)->uScGrantoffset          = soc_property_get(unit, spn_QE_GRANT_OFFSET,
                                                                            SOC_SBX_CFG_QE2000(unit)->uScGrantoffset);
        SOC_SBX_CFG_QE2000(unit)->nGlobalShapingAdjustInBytes = soc_property_get(unit, spn_QE_GLOBAL_SHAPING_ADJUST,
                                                                                 SOC_SBX_CFG_QE2000(unit)->nGlobalShapingAdjustInBytes);
        SOC_SBX_CFG_QE2000(unit)->uQmMaxArrivalRateMbs   = soc_property_get(unit, spn_QE_MAX_ARRIVAL_RATE,
                                                                            SOC_SBX_CFG_QE2000(unit)->uQmMaxArrivalRateMbs);
        SOC_SBX_CFG_QE2000(unit)->bMixHighAndLowRateFlows = soc_property_get(unit, spn_QE_MIX_HIGH_LOW_RATE_FLOWS,
                                                                             SOC_SBX_CFG_QE2000(unit)->bMixHighAndLowRateFlows);
        SOC_SBX_CFG_QE2000(unit)->bEiSpiFullPacketMode[0] = soc_property_get(unit, spn_QE_SPI_0_FULL_PACKET_MODE,
                                                                             SOC_SBX_CFG_QE2000(unit)->bEiSpiFullPacketMode[0]);
        SOC_SBX_CFG_QE2000(unit)->bEiSpiFullPacketMode[1] = soc_property_get(unit, spn_QE_SPI_1_FULL_PACKET_MODE,
                                                                             SOC_SBX_CFG_QE2000(unit)->bEiSpiFullPacketMode[1]);
        SOC_SBX_CFG_QE2000(unit)->uEiLines[0]
            = soc_property_get(unit, spn_QE_SPI_0_EI_LINES,
                               SOC_SBX_CFG_QE2000(unit)->uEiLines[0]);
        SOC_SBX_CFG_QE2000(unit)->uEiLines[1]
            = soc_property_get(unit, spn_QE_SPI_1_EI_LINES,
                               SOC_SBX_CFG_QE2000(unit)->uEiLines[1]);

        SOC_SBX_CFG_QE2000(unit)->uQsMaxNodes             = soc_property_get(unit, spn_NUM_MODULES,
                                                                             SOC_SBX_CFG_QE2000(unit)->uQsMaxNodes);
        SOC_SBX_CFG_QE2000(unit)->nQueuesPerShaperIngress = soc_property_get(unit, spn_QE_QUEUES_PER_INGRESS_SHAPER,
                                                                             SOC_SBX_CFG_QE2000(unit)->nQueuesPerShaperIngress);

        SOC_SBX_CFG_QE2000(unit)->bEpDisable = soc_property_get(unit, spn_QE_EP_DISABLE, SOC_SBX_CFG_QE2000(unit)->bEpDisable);
        if ((SOC_SBX_CFG_QE2000(unit)->bEpDisable != FALSE) && (SOC_SBX_CFG_QE2000(unit)->bEpDisable != TRUE)) {
            SOC_SBX_CFG_QE2000(unit)->bEpDisable = FALSE;
        }

        SOC_SBX_CFG_QE2000(unit)->SpiRefClockSpeed[0] = soc_property_get(unit, spn_SPI_0_REF_CLOCK_SPEED, SOC_SBX_CFG_QE2000(unit)->SpiRefClockSpeed[0]);
        SOC_SBX_CFG_QE2000(unit)->SpiClockSpeed[0] = soc_property_get(unit, spn_SPI_0_CLOCK_SPEED, SOC_SBX_CFG_QE2000(unit)->SpiClockSpeed[0]);
        SOC_SBX_CFG_QE2000(unit)->SpiRefClockSpeed[1] = soc_property_get(unit, spn_SPI_1_REF_CLOCK_SPEED, SOC_SBX_CFG_QE2000(unit)->SpiRefClockSpeed[1]);
        SOC_SBX_CFG_QE2000(unit)->SpiClockSpeed[1] = soc_property_get(unit, spn_SPI_1_CLOCK_SPEED, SOC_SBX_CFG_QE2000(unit)->SpiClockSpeed[1]);
        SOC_SBX_CFG_QE2000(unit)->uNumPhySpiPorts[0]      = soc_property_get(unit, spn_QE_SPI_0_SUBPORTS,
                                                                             SOC_SBX_CFG_QE2000(unit)->uNumPhySpiPorts[0]);
        SOC_SBX_CFG_QE2000(unit)->uNumPhySpiPorts[1]      = soc_property_get(unit, spn_QE_SPI_1_SUBPORTS,
                                                                             SOC_SBX_CFG_QE2000(unit)->uNumPhySpiPorts[1]);

        /* NOTE: currently it is expeted that spn_QE_SPI_0_SUBPORTS/spn_QE_SPI_1_SUBPORTS will  */
        /*       always be specified when Tx/Rx properties are specified individually. The      */
        /*       alternative is to set it from the Tx properties. The port bitmap is populated  */
        /*       with spn_QE_SPI_0_SUBPORTS/spn_QE_SPI_1_SUBPORTS values and there is no        */
        /*       consistency check currently present when a queue group is added for a physical */
        /*       port. These checks can be later added.                                         */
        SOC_SBX_CFG_QE2000(unit)->uNumSpiTxPorts[0] = soc_property_get(unit,
             spn_SPI_0_NUM_TX_SUBPORTS, SOC_SBX_CFG_QE2000(unit)->uNumPhySpiPorts[0]);
        SOC_SBX_CFG_QE2000(unit)->uNumSpiRxPorts[0] = soc_property_get(unit,
             spn_SPI_0_NUM_RX_SUBPORTS, SOC_SBX_CFG_QE2000(unit)->uNumPhySpiPorts[0]);
        SOC_SBX_CFG_QE2000(unit)->uNumSpiTxPorts[1] = soc_property_get(unit,
             spn_SPI_1_NUM_TX_SUBPORTS, SOC_SBX_CFG_QE2000(unit)->uNumPhySpiPorts[1]);
        SOC_SBX_CFG_QE2000(unit)->uNumSpiRxPorts[1] = soc_property_get(unit,
             spn_SPI_1_NUM_RX_SUBPORTS, SOC_SBX_CFG_QE2000(unit)->uNumPhySpiPorts[1]);

        SOC_SBX_CFG_QE2000(unit)->uNumSpiTxStatusRepCnt[0] = soc_property_get(unit,
             spn_SPI_0_TX_CAL_STATUS_REP_CNT, SOC_SBX_CFG_QE2000(unit)->uNumSpiTxStatusRepCnt[0]);
        SOC_SBX_CFG_QE2000(unit)->uNumSpiRxStatusRepCnt[0] = soc_property_get(unit,
             spn_SPI_0_RX_CAL_STATUS_REP_CNT, SOC_SBX_CFG_QE2000(unit)->uNumSpiRxStatusRepCnt[0]);
        SOC_SBX_CFG_QE2000(unit)->uNumSpiTxStatusRepCnt[1] = soc_property_get(unit,
             spn_SPI_1_TX_CAL_STATUS_REP_CNT, SOC_SBX_CFG_QE2000(unit)->uNumSpiTxStatusRepCnt[1]);
        SOC_SBX_CFG_QE2000(unit)->uNumSpiRxStatusRepCnt[1] = soc_property_get(unit,
             spn_SPI_1_RX_CAL_STATUS_REP_CNT, SOC_SBX_CFG_QE2000(unit)->uNumSpiRxStatusRepCnt[1]);

        SOC_SBX_CFG_QE2000(unit)->uScTxdmaSotDelayInClocks = soc_property_get(unit, spn_QE_SC_TXDMA_SOT_DELAY_CLOCKS,
                                                                              SOC_SBX_CFG_QE2000(unit)->uScTxdmaSotDelayInClocks);
        SOC_SBX_CFG_QE2000(unit)->uSfiTimeslotOffsetInClocks = soc_property_get(unit, spn_QE_SFI_TIMESLOT_OFFSET_CLOCKS,
                                                                                SOC_SBX_CFG_QE2000(unit)->uSfiTimeslotOffsetInClocks);

        SOC_INFO(unit).modid_max = BCM_MODULE_FABRIC_BASE + SOC_SBX_CFG(unit)->num_nodes - 1;

        /* init default spi subport speed based on number of ports on the spi interface */
        for (spi=0; spi<SB_FAB_DEVICE_QE2000_NUM_SPI_INTERFACES; spi++) {
            SOC_SBX_CFG_QE2000(unit)->uInterleaveBurstSize[spi] =
                soc_property_port_get(unit, spi, spn_SPI_INTERLEAVE_BURST_SIZE, HW_QE2000_PORT_MODE_PKT_IL_BURST_SIZE);
            if (SOC_SBX_CFG_QE2000(unit)->uNumPhySpiPorts[spi] > 1) {
                /* default to 1 Gbps */
                port_default_speed = 1000;
            } else {
                /* default to 10 Gbps */
                port_default_speed = 10000;
            }
            for(port=0; port<(SOC_SBX_CFG_QE2000(unit)->uNumPhySpiPorts[spi]); port++){
                if (spi == 0) {
                    port_speed = soc_property_port_get(unit, port, spn_QE_SPI_0_SUBPORT_SPEED, port_default_speed);
                } else {
                    port_speed = soc_property_port_get(unit, port, spn_QE_SPI_1_SUBPORT_SPEED, port_default_speed);
                }
                SOC_SBX_CFG_QE2000(unit)->uSpiSubportSpeed[spi][port] = port_speed;
                if (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_HYBRID) {
                    if (spi == 0) {
                        requeue_mode = soc_property_port_get(unit, port, spn_QE_SPI_0_SUBPORT_IS_REQUEUE, 0);
                    } else {
                        requeue_mode = soc_property_port_get(unit, port, spn_QE_SPI_1_SUBPORT_IS_REQUEUE, 0);
                    }
                    SOC_SBX_CFG_QE2000(unit)->uuRequeuePortsMask[spi] |= (((uint64)requeue_mode) << port);
                }
            }
        }

        for (port=0; port < SB_FAB_DEVICE_QE2000_MAX_PORT; port++) {
            SOC_SBX_CFG_QE2000(unit)->uEgressMcastEfDescFifoSize[port] = soc_property_port_get(unit, port, spn_EGRESS_MCAST_EF_DESC_SZ,
                                                                                               SOC_SBX_CFG_QE2000(unit)->uEgressMcastEfDescFifoSize[port]);
            SOC_SBX_CFG_QE2000(unit)->uEgressMcastNefDescFifoSize[port] = soc_property_port_get(unit, port, spn_EGRESS_MCAST_NEF_DESC_SZ,
                                                                                               SOC_SBX_CFG_QE2000(unit)->uEgressMcastNefDescFifoSize[port]);
        }

        fifoset=0;
        for (spi=0; spi<SB_FAB_DEVICE_QE2000_NUM_SPI_INTERFACES; spi++) {
            for (port=0; port <(SOC_SBX_CFG_QE2000(unit)->uNumPhySpiPorts[spi]); port++) {
                /* 2 multicast FIFOs per port, EF, non EF */
                /* EF is 0-49, with PCI at 49, NEF is 50-99 with PCI at 99 */
                SOC_SBX_CFG_QE2000(unit)->bEgressMcastEfDescFifoInUse[fifoset] = TRUE;
                SOC_SBX_CFG_QE2000(unit)->bEgressMcastNefDescFifoInUse[fifoset] = TRUE;
                fifoset++;
            }
        }
        /* Reserve last 2 FIFOs after all SPI for PCI port */
        SOC_SBX_CFG_QE2000(unit)->bEgressMcastEfDescFifoInUse[49] = TRUE;
        SOC_SBX_CFG_QE2000(unit)->bEgressMcastNefDescFifoInUse[49] = TRUE;

        for(port=0; port<SB_FAB_DEVICE_QE2000_NUM_SERIALIZERS; port++){
            SOC_SBX_CFG_QE2000(unit)->linkDriverConfig[port].uDriverStrength =
                soc_property_port_get(unit, port, spn_LINK_DRIVER_STRENGTH, SOC_SBX_CFG_QE2000(unit)->linkDriverConfig[port].uDriverStrength);
            SOC_SBX_CFG_QE2000(unit)->linkDriverConfig[port].uDriverEqualization =
                soc_property_port_get(unit, port, spn_LINK_DRIVER_EQUALIZATION, SOC_SBX_CFG_QE2000(unit)->linkDriverConfig[port].uDriverEqualization);
        }
        /* Following config's are either hardcoded or configured after init:
           SOC_SBX_CFG_QE2000(unit)->uSfiDataLinkInitMask  = 0; QE1000 related, no use
           SOC_SBX_CFG_QE2000(unit)->uuRequeuePortsMask[0] = 0; requeue configuration done by API
           SOC_SBX_CFG_QE2000(unit)->uuRequeuePortsMask[1] = 0; requeue configuration done by API
           SOC_SBX_CFG_QE2000(unit)->nodeNum_ul            = 0; node id is unknown at init time, configured by API
        */

        SOC_SBX_CFG(unit)->mcgroup_local_start_index =
                soc_property_get(unit, spn_MC_GROUP_LOCAL_START_INDEX, SOC_SBX_CFG(unit)->mcgroup_local_start_index);

        switch (SOC_SBX_CFG_QE2000(unit)->uEgMVTSize) {
            case 0:
                if ((SBX_MVT_ID_12K_DYNAMIC_END + 1) == SOC_SBX_CFG(unit)->mcgroup_local_start_index) {
                    SOC_ERROR_PRINT((DK_WARN, "Disabling local MVT entries on unit %d\n",
                                     unit));
                } else if (SOC_SBX_CFG(unit)->mcgroup_local_start_index > SBX_MVT_ID_12K_DYNAMIC_END) {
                    SOC_ERROR_PRINT((DK_ERR, "Invalid MVT local Index %d unit %d\n",
                                        SOC_SBX_CFG(unit)->mcgroup_local_start_index, unit));
                    SOC_SBX_CFG(unit)->mcgroup_local_start_index = SBX_MVT_ID_12K_LOCAL_BASE;
                }
                break;

            case 1:
                if ((SBX_MVT_ID_24K_DYNAMIC_END + 1) == SOC_SBX_CFG(unit)->mcgroup_local_start_index) {
                    SOC_ERROR_PRINT((DK_WARN, "Disabling local MVT entries on unit %d\n",
                                     unit));
                } else if (SOC_SBX_CFG(unit)->mcgroup_local_start_index > SBX_MVT_ID_24K_DYNAMIC_END) {
                    SOC_ERROR_PRINT((DK_ERR, "Invalid MVT local Index %d unit %d\n",
                                        SOC_SBX_CFG(unit)->mcgroup_local_start_index, unit));
                    SOC_SBX_CFG(unit)->mcgroup_local_start_index = SBX_MVT_ID_24K_LOCAL_BASE;
                }
                break;

            case 2:
                if ((SBX_MVT_ID_DYNAMIC_END + 1) == SOC_SBX_CFG(unit)->mcgroup_local_start_index) {
                    SOC_ERROR_PRINT((DK_WARN, "Disabling local MVT entries on unit %d\n",
                                     unit));
                } else if (SOC_SBX_CFG(unit)->mcgroup_local_start_index > SBX_MVT_ID_DYNAMIC_END) {
                    SOC_ERROR_PRINT((DK_ERR, "Invalid MVT local Index %d unit %d\n",
                                        SOC_SBX_CFG(unit)->mcgroup_local_start_index, unit));
                    SOC_SBX_CFG(unit)->mcgroup_local_start_index = SBX_MVT_ID_LOCAL_BASE;
                }
                break;

            default:
                SOC_ERROR_PRINT((DK_ERR, "Invalid MVT size %d unit %d\n",
                                        SOC_SBX_CFG_QE2000(unit)->uEgMVTSize, unit));
                if ((SBX_MVT_ID_DYNAMIC_END + 1) == SOC_SBX_CFG(unit)->mcgroup_local_start_index) {
                    SOC_ERROR_PRINT((DK_WARN, "Disabling local MVT entries on unit %d\n",
                                     unit));
                } else if (SOC_SBX_CFG(unit)->mcgroup_local_start_index > SBX_MVT_ID_DYNAMIC_END) {
                    SOC_ERROR_PRINT((DK_ERR, "Invalid MVT local Index %d unit %d\n",
                                        SOC_SBX_CFG(unit)->mcgroup_local_start_index, unit));
                    SOC_SBX_CFG(unit)->mcgroup_local_start_index = SBX_MVT_ID_LOCAL_BASE;
                }
                SOC_SBX_CFG_QE2000(unit)->uEgMVTSize = 2;
                break;
        }

        SOC_SBX_CFG_QE2000(unit)->uPacketAdjustFormat = soc_property_get(unit, spn_PACKET_ADJUST_FORMAT, SOC_SBX_CFG_QE2000(unit)->uPacketAdjustFormat);
        if ( (SOC_SBX_CFG_QE2000(unit)->uPacketAdjustFormat != 0) &&
                             (SOC_SBX_CFG_QE2000(unit)->uPacketAdjustFormat != 1) ) {
            SOC_SBX_CFG_QE2000(unit)->uPacketAdjustFormat = 0;
        }

    } else if (SOC_IS_SBX_BME3200(unit)) {
        /* Fill in BM3200 specific configuration */
        SOC_SBX_CFG(unit)->uClockSpeedInMHz              = soc_property_get(unit, spn_QE_CLOCK_SPEED,
                                                                            SOC_SBX_CFG(unit)->uClockSpeedInMHz);
        SOC_SBX_CFG_BM3200(unit)->uBmLocalBmId           = soc_property_get(unit, spn_BME_SWITCH_CONTROLLER_ID,
                                                                             SOC_SBX_CFG_BM3200(unit)->uBmLocalBmId);
        SOC_SBX_CFG_BM3200(unit)->uBmDefaultBmId         = soc_property_get(unit, spn_ACTIVE_SWITCH_CONTROLLER_ID,
                                                                             SOC_SBX_CFG_BM3200(unit)->uBmDefaultBmId);
        SOC_SBX_CFG_BM3200(unit)->bSv2_5GbpsLinks        = soc_property_get(unit, spn_QE_2_5GBPS_LINKS,
                                                                            SOC_SBX_CFG_BM3200(unit)->bSv2_5GbpsLinks);

        /* Bm device mode
         */
        switch (SOC_SBX_CFG_BM3200(unit)->uDeviceMode = soc_property_get(unit, spn_BM_DEVICE_MODE, SOC_SBX_CFG_BM3200(unit)->uDeviceMode)) {
            case SOC_SBX_BME_ARBITER_MODE:
                /* BME only */
                SOC_SBX_CFG_BM3200(unit)->nNumLinks = SB_FAB_USER_MAX_NUM_NODES;
                SOC_SBX_CFG_BM3200(unit)->uLcmPlaneValid[0] = 0;    /* not used */
                SOC_SBX_CFG_BM3200(unit)->uLcmPlaneValid[1] = 0;    /* not used */
                SOC_SBX_CFG_BM3200(unit)->uLcmSerializerMaskHi = 0; /* not used */
                SOC_SBX_CFG_BM3200(unit)->uLcmSerializerMaskLo = 0; /* not used */
                break;
            case SOC_SBX_BME_XBAR_MODE:
                /* SE only */
                SOC_SBX_CFG_BM3200(unit)->nNumLinks = SB_FAB_DEVICE_BM3200_NUM_SERIALIZERS;
                SOC_SBX_CFG_BM3200(unit)->uLcmPlaneValid[0] = 0;    /* not used */
                SOC_SBX_CFG_BM3200(unit)->uLcmPlaneValid[1] = 0;    /* not used */
                SOC_SBX_CFG_BM3200(unit)->uLcmSerializerMaskHi = 0; /* not used */
                SOC_SBX_CFG_BM3200(unit)->uLcmSerializerMaskLo = 0; /* not used */
                break;
            case SOC_SBX_BME_ARBITER_XBAR_MODE:
                /* BME+SE */
                SOC_SBX_CFG_BM3200(unit)->nNumLinks = SB_FAB_DEVICE_BM3200_NUM_SERIALIZERS;
                SOC_SBX_CFG_BM3200(unit)->uLcmPlaneValid[0] = 0;    /* not used */
                SOC_SBX_CFG_BM3200(unit)->uLcmPlaneValid[1] = 0;    /* not used */
                SOC_SBX_CFG_BM3200(unit)->uLcmSerializerMaskHi = 0; /* not used */
                SOC_SBX_CFG_BM3200(unit)->uLcmSerializerMaskLo = 0; /* not used */
                break;
            case SOC_SBX_BME_LCM_MODE:
                /* LCM only */
                SOC_SBX_CFG_BM3200(unit)->nNumLinks = SB_FAB_DEVICE_BM3200_NUM_SERIALIZERS;
                /*
                 * Find out LCM Xcfg and link usage from the user defined lcm_dataplane_x_map
                 */
                if( soc_property_get(unit, spn_LCM_PASSTHROUGH_MODE, 0)) {
                    /* Default to be non-passthrough */
                    SOC_SBX_CFG_BM3200(unit)->uLcmPlaneValid[0] = 1;
                    SOC_SBX_CFG_BM3200(unit)->uLcmPlaneValid[1] = 0;
                    for(port=0; port<SB_FAB_DEVICE_BM3200_NUM_SERIALIZERS; port++){
                        if( (property = soc_property_port_get(unit, port, spn_LCM_DATAPLANE_0_MAP, 0xFF)) != 0xFF) {
                            if (property < SOC_SBX_CFG_BM3200(unit)->nNumLinks) {
                                SOC_SBX_CFG_BM3200(unit)->uLcmXcfg[0][port] = property;
                                SOC_SBX_CFG_BM3200(unit)->uLcmXcfg[1][port] = property;
                                SOC_SBX_CFG_BM3200(unit)->linkState[port].nState = SB_FAB_DEVICE_SERIALIZER_STATE_DATA;
                                SOC_SBX_CFG_BM3200(unit)->linkState[property].nState = SB_FAB_DEVICE_SERIALIZER_STATE_DATA;
                            } else {
                                SOC_ERROR_PRINT((DK_ERR, "soc_sbx_init: unit %d lcm_dataplane0_map_port%d value %d out of range [0, %d]",
                                                 unit, port, property, SOC_SBX_CFG_BM3200(unit)->nNumLinks));
                                return SOC_E_PARAM;
                            }
                        }
                    }
                } else {
                    SOC_SBX_CFG_BM3200(unit)->uLcmPlaneValid[0] = 1;
                    SOC_SBX_CFG_BM3200(unit)->uLcmPlaneValid[1] = 1;
                    for(port=0; port<SB_FAB_DEVICE_BM3200_NUM_SERIALIZERS; port++){
                        if( (property = soc_property_port_get(unit, port, spn_LCM_DATAPLANE_0_MAP, 0xFF)) != 0xFF) {
                            if (property < SOC_SBX_CFG_BM3200(unit)->nNumLinks) {
                                SOC_SBX_CFG_BM3200(unit)->uLcmXcfg[0][port] = property;
                                SOC_SBX_CFG_BM3200(unit)->linkState[port].nState = SB_FAB_DEVICE_SERIALIZER_STATE_DATA;
                                SOC_SBX_CFG_BM3200(unit)->linkState[property].nState = SB_FAB_DEVICE_SERIALIZER_STATE_DATA;
                            } else {
                                SOC_ERROR_PRINT((DK_ERR, "soc_sbx_init: unit %d lcm_dataplane0_map_port%d value %d out of range [0, %d]",
                                                 unit, port, property, SOC_SBX_CFG_BM3200(unit)->nNumLinks));
                                return SOC_E_PARAM;
                            }
                        }
                        if( (property = soc_property_port_get(unit, port, spn_LCM_DATAPLANE_1_MAP, 0xFF)) != 0xFF) {
                            if (property < SOC_SBX_CFG_BM3200(unit)->nNumLinks) {
                                SOC_SBX_CFG_BM3200(unit)->uLcmXcfg[1][port] = property;
                                SOC_SBX_CFG_BM3200(unit)->linkState[port].nState = SB_FAB_DEVICE_SERIALIZER_STATE_DATA;
                                SOC_SBX_CFG_BM3200(unit)->linkState[property].nState = SB_FAB_DEVICE_SERIALIZER_STATE_DATA;
                            } else {
                                SOC_ERROR_PRINT((DK_ERR, "soc_sbx_init: unit %d lcm_dataplane1_map_port%d value %d out of range [0, %d]",
                                                 unit, port, property, SOC_SBX_CFG_BM3200(unit)->nNumLinks));
                                return SOC_E_PARAM;
                            }
                        }
                    }
                }
                /* Serializer usage mask */
                for(port=0; port<SB_FAB_DEVICE_BM3200_NUM_SERIALIZERS; port++){
                    if ( SOC_SBX_CFG_BM3200(unit)->linkState[port].nState == SB_FAB_DEVICE_SERIALIZER_STATE_DATA ) {
                        if (port < 32) {
                            SOC_SBX_CFG_BM3200(unit)->uLcmSerializerMaskLo |= 0x1 << port;
                        } else {
                            SOC_SBX_CFG_BM3200(unit)->uLcmSerializerMaskHi |= 0x1 << (port-32);
                        }
                    }
                }
                /* bLcmXcfgABInputPolarityReversed */
                SOC_SBX_CFG_BM3200(unit)->bLcmXcfgABInputPolarityReversed = soc_property_get(unit, spn_LCM_XCFG_AB_INPUT_POLARITY_REVERSED,
                                                                              SOC_SBX_CFG_BM3200(unit)->bLcmXcfgABInputPolarityReversed );
                break;
            default :
                /* Default to BME+SE */
                SOC_SBX_CFG_BM3200(unit)->nNumLinks = SB_FAB_DEVICE_BM3200_NUM_SERIALIZERS;
                SOC_SBX_CFG_BM3200(unit)->uLcmPlaneValid[0] = 0;    /* not used */
                SOC_SBX_CFG_BM3200(unit)->uLcmPlaneValid[1] = 0;    /* not used */
                SOC_SBX_CFG_BM3200(unit)->uLcmSerializerMaskHi = 0; /* not used */
                SOC_SBX_CFG_BM3200(unit)->uLcmSerializerMaskLo = 0; /* not used */
                break;
        }

        /* Get Link config from user defined config
         */
        for(port=0; port<SB_FAB_DEVICE_BM3200_NUM_SERIALIZERS; port++){
            /* User defines for link driver config */
            SOC_SBX_CFG_BM3200(unit)->linkDriverConfig[port].uDriverStrength =
                soc_property_port_get(unit, port, spn_LINK_DRIVER_STRENGTH, SOC_SBX_CFG_BM3200(unit)->linkDriverConfig[port].uDriverStrength);

            SOC_SBX_CFG_BM3200(unit)->linkDriverConfig[port].uDriverEqualization =
                soc_property_port_get(unit, port, spn_LINK_DRIVER_EQUALIZATION, SOC_SBX_CFG_BM3200(unit)->linkDriverConfig[port].uDriverEqualization);

            if (port < SOC_SBX_CFG_BM3200(unit)->nNumLinks) {
                /* User defines for SCIs */
                if(port < 32 && soc_property_port_get(unit, port, spn_PORT_IS_SCI, 0) ){
                    /* Links for bandwidth manager */
                    SOC_SBX_CFG_BM3200(unit)->uSerializerMask |= 1 << port;
                    SOC_SBX_CFG_BM3200(unit)->linkState[port].nState = SB_FAB_DEVICE_SERIALIZER_STATE_CONTROL;
                } else {
                    /* Links for crossbar */
                    SOC_SBX_CFG_BM3200(unit)->linkState[port].nState = SB_FAB_DEVICE_SERIALIZER_STATE_DATA;
                }
            } else {
                /* Reserved links */
                SOC_SBX_CFG_BM3200(unit)->linkState[port].nState = SB_FAB_DEVICE_SERIALIZER_STATE_RESERVED;
            }
        }
    } else if (SOC_IS_SBX_BM9600(unit)) {
        /* Fill in Bm9600 specific configuration */
        SOC_SBX_CFG(unit)->uClockSpeedInMHz              = soc_property_get(unit, spn_QE_CLOCK_SPEED,
                                                                            SOC_SBX_CFG(unit)->uClockSpeedInMHz);
        SOC_SBX_CFG_BM9600(unit)->uBmLocalBmId           = soc_property_get(unit, spn_BME_SWITCH_CONTROLLER_ID,
                                                                             SOC_SBX_CFG_BM9600(unit)->uBmLocalBmId);
        SOC_SBX_CFG_BM9600(unit)->uBmDefaultBmId         = soc_property_get(unit, spn_ACTIVE_SWITCH_CONTROLLER_ID,
                                                                             SOC_SBX_CFG_BM9600(unit)->uBmDefaultBmId);
        SOC_SBX_CFG_BM9600(unit)->bSv2_5GbpsLinks        = soc_property_get(unit, spn_QE_2_5GBPS_LINKS,
                                                                            SOC_SBX_CFG_BM9600(unit)->bSv2_5GbpsLinks);

        SOC_SBX_CFG(unit)->enable_all_egress_nodes = TRUE;
        SOC_SBX_CFG(unit)->enable_all_egress_nodes = soc_property_get(unit, spn_ENABLE_ALL_MODULE_ARBITRATION,
                                                                            SOC_SBX_CFG(unit)->enable_all_egress_nodes);
        SOC_SBX_CFG(unit)->enable_all_egress_nodes =
               (SOC_SBX_CFG(unit)->enable_all_egress_nodes != FALSE) ? TRUE : FALSE;

	SOC_SBX_CFG_BM9600(unit)->bElectArbiterReconfig = FALSE;

        /* Bm device mode
         */
        switch (SOC_SBX_CFG_BM9600(unit)->uDeviceMode = soc_property_get(unit, spn_BM_DEVICE_MODE, SOC_SBX_CFG_BM9600(unit)->uDeviceMode)) {
            case SOC_SBX_BME_ARBITER_MODE:
                /* BME only */
                SOC_SBX_CFG_BM9600(unit)->nNumLinks = SB_FAB_USER_MAX_NUM_NODES;
                SOC_SBX_CFG_BM9600(unit)->uLcmPlaneValid[0] = 0;    /* not used */
                SOC_SBX_CFG_BM9600(unit)->uLcmPlaneValid[1] = 0;    /* not used */
                SOC_SBX_CFG_BM9600(unit)->uLcmSerializerMaskHi = 0; /* not used */
                SOC_SBX_CFG_BM9600(unit)->uLcmSerializerMaskLo = 0; /* not used */
                break;
            case SOC_SBX_BME_XBAR_MODE:
                /* SE only */
                SOC_SBX_CFG_BM9600(unit)->nNumLinks = SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS;
                SOC_SBX_CFG_BM9600(unit)->uLcmPlaneValid[0] = 0;    /* not used */
                SOC_SBX_CFG_BM9600(unit)->uLcmPlaneValid[1] = 0;    /* not used */
                SOC_SBX_CFG_BM9600(unit)->uLcmSerializerMaskHi = 0; /* not used */
                SOC_SBX_CFG_BM9600(unit)->uLcmSerializerMaskLo = 0; /* not used */
                break;
            case SOC_SBX_BME_ARBITER_XBAR_MODE:
                /* BME+SE */
                SOC_SBX_CFG_BM9600(unit)->nNumLinks = SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS;
                SOC_SBX_CFG_BM9600(unit)->uLcmPlaneValid[0] = 0;    /* not used */
                SOC_SBX_CFG_BM9600(unit)->uLcmPlaneValid[1] = 0;    /* not used */
                SOC_SBX_CFG_BM9600(unit)->uLcmSerializerMaskHi = 0; /* not used */
                SOC_SBX_CFG_BM9600(unit)->uLcmSerializerMaskLo = 0; /* not used */
                break;
            case SOC_SBX_BME_LCM_MODE:
                /* LCM only */
                SOC_SBX_CFG_BM9600(unit)->nNumLinks = SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS;
                /*
                 * Find out LCM Xcfg and link usage from the user defined lcm_dataplane_x_map
                 */
                if( soc_property_get(unit, spn_LCM_PASSTHROUGH_MODE, 0)) {
                    /* Default to be non-passthrough */
                    SOC_SBX_CFG_BM9600(unit)->uLcmPlaneValid[0] = 1;
                    SOC_SBX_CFG_BM9600(unit)->uLcmPlaneValid[1] = 0;
                    for(port=0; port<SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS; port++){
                        if( (property = soc_property_port_get(unit, port, spn_LCM_DATAPLANE_0_MAP, 0xFF)) != 0xFF) {
                            if (property < SOC_SBX_CFG_BM9600(unit)->nNumLinks) {
                                SOC_SBX_CFG_BM9600(unit)->uLcmXcfg[0][port] = property;
                                SOC_SBX_CFG_BM9600(unit)->uLcmXcfg[1][port] = property;
                                SOC_SBX_CFG_BM9600(unit)->linkState[port].nState = SB_FAB_DEVICE_SERIALIZER_STATE_DATA;
                                SOC_SBX_CFG_BM9600(unit)->linkState[property].nState = SB_FAB_DEVICE_SERIALIZER_STATE_DATA;
                            } else {
                                SOC_ERROR_PRINT((DK_ERR, "soc_sbx_init: unit %d lcm_dataplane0_map_port%d value %d out of range [0, %d]",
                                                 unit, port, property, SOC_SBX_CFG_BM9600(unit)->nNumLinks));
                                return SOC_E_PARAM;
                            }
                        }
                    }
                } else {
		    SOC_SBX_CFG_BM9600(unit)->uLcmPlaneValid[0] = 1;
		    SOC_SBX_CFG_BM9600(unit)->uLcmPlaneValid[1] = 1;
		    for(port=0; port<SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS; port++){
			if( (property = soc_property_port_get(unit, port, spn_LCM_DATAPLANE_0_MAP, 0xFF)) != 0xFF) {
			    if (property < SOC_SBX_CFG_BM9600(unit)->nNumLinks) {
				SOC_SBX_CFG_BM9600(unit)->uLcmXcfg[0][port] = property;
				SOC_SBX_CFG_BM9600(unit)->linkState[port].nState = SB_FAB_DEVICE_SERIALIZER_STATE_DATA;
				SOC_SBX_CFG_BM9600(unit)->linkState[property].nState = SB_FAB_DEVICE_SERIALIZER_STATE_DATA;
			    } else {
                                SOC_ERROR_PRINT((DK_ERR, "soc_sbx_init: unit %d lcm_dataplane0_map_port%d value %d out of range [0, %d]",
                                                 unit, port, property, SOC_SBX_CFG_BM9600(unit)->nNumLinks));
                                return SOC_E_PARAM;
                            }
                        }
                        if( (property = soc_property_port_get(unit, port, spn_LCM_DATAPLANE_1_MAP, 0xFF)) != 0xFF) {
                            if (property < SOC_SBX_CFG_BM9600(unit)->nNumLinks) {
                                SOC_SBX_CFG_BM9600(unit)->uLcmXcfg[1][port] = property;
                                SOC_SBX_CFG_BM9600(unit)->linkState[port].nState = SB_FAB_DEVICE_SERIALIZER_STATE_DATA;
                                SOC_SBX_CFG_BM9600(unit)->linkState[property].nState = SB_FAB_DEVICE_SERIALIZER_STATE_DATA;
                            } else {
                                SOC_ERROR_PRINT((DK_ERR, "soc_sbx_init: unit %d lcm_dataplane1_map_port%d value %d out of range [0, %d]",
                                                 unit, port, property, SOC_SBX_CFG_BM9600(unit)->nNumLinks));
                                return SOC_E_PARAM;
                            }
                        }
                    }
                }
                /* Serializer usage mask */
                for(port=0; port<SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS; port++){
                    if ( SOC_SBX_CFG_BM9600(unit)->linkState[port].nState == SB_FAB_DEVICE_SERIALIZER_STATE_DATA ) {
                        if (port < 32) {
                            SOC_SBX_CFG_BM9600(unit)->uLcmSerializerMaskLo |= 0x1 << port;
                        } else {
                            SOC_SBX_CFG_BM9600(unit)->uLcmSerializerMaskHi |= 0x1 << (port-32);
                        }
                    }
                }
                /* bLcmXcfgABInputPolarityReversed */
                SOC_SBX_CFG_BM9600(unit)->bLcmXcfgABInputPolarityReversed = soc_property_get(unit, spn_LCM_XCFG_AB_INPUT_POLARITY_REVERSED,
                                                                              SOC_SBX_CFG_BM9600(unit)->bLcmXcfgABInputPolarityReversed );
                break;
            default :
                /* Default to BME+SE */
                SOC_SBX_CFG_BM9600(unit)->nNumLinks = SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS;
                SOC_SBX_CFG_BM9600(unit)->uLcmPlaneValid[0] = 0;    /* not used */
                SOC_SBX_CFG_BM9600(unit)->uLcmPlaneValid[1] = 0;    /* not used */
                SOC_SBX_CFG_BM9600(unit)->uLcmSerializerMaskHi = 0; /* not used */
                SOC_SBX_CFG_BM9600(unit)->uLcmSerializerMaskLo = 0; /* not used */
                break;
        }

        /* Get Link config from user defined config
         */
        for(port=0; port<SB_FAB_DEVICE_BM9600_NUM_SERIALIZERS; port++){
            /* User defines for link driver config */
            SOC_SBX_CFG_BM9600(unit)->linkDriverConfig[port].uDriverStrength =
                soc_property_port_get(unit, port, spn_LINK_DRIVER_STRENGTH, SOC_SBX_CFG_BM9600(unit)->linkDriverConfig[port].uDriverStrength);

            SOC_SBX_CFG_BM9600(unit)->linkDriverConfig[port].uDriverEqualization =
                soc_property_port_get(unit, port, spn_LINK_DRIVER_EQUALIZATION, SOC_SBX_CFG_BM9600(unit)->linkDriverConfig[port].uDriverEqualization);

            if (port < SOC_SBX_CFG_BM9600(unit)->nNumLinks) {
		  if ((SOC_SBX_CFG_BM9600(unit)->uDeviceMode == SOC_SBX_BME_ARBITER_MODE) ||
		      (SOC_SBX_CFG_BM9600(unit)->uDeviceMode == SOC_SBX_BME_ARBITER_XBAR_MODE)) {

		      /* User defines for SCIs */
		      if( soc_property_port_get(unit, port, spn_PORT_IS_SCI, 0) ){
			  /* Links for bandwidth manager */
			  SOC_SBX_CFG_BM9600(unit)->uSerializerMask |= 1 << port;
			  SOC_SBX_CFG_BM9600(unit)->linkState[port].nState = SB_FAB_DEVICE_SERIALIZER_STATE_CONTROL;
		      }
		  } else {
		      /* Links for crossbar */
		      SOC_SBX_CFG_BM9600(unit)->linkState[port].nState = SB_FAB_DEVICE_SERIALIZER_STATE_DATA;
		  }
            } else {
                /* Reserved links */
                SOC_SBX_CFG_BM9600(unit)->linkState[port].nState = SB_FAB_DEVICE_SERIALIZER_STATE_RESERVED;
            }
	}
        for (node = 0; node < SBX_MAXIMUM_NODES; node++) {

            modid = soc_property_port_get(unit, node, spn_SCI_PORT_MODID,
                            (BCM_MODULE_FABRIC_BASE + SOC_SBX_CFG(unit)->p2l_node[node]));

            if (modid != (BCM_MODULE_FABRIC_BASE + SOC_SBX_CFG(unit)->p2l_node[node])) {

                SOC_SBX_CFG(unit)->l2p_node[BCM_STK_MOD_TO_NODE(modid)] = node;
                SOC_SBX_CFG(unit)->p2l_node[node] = BCM_STK_MOD_TO_NODE(modid);

                if (SOC_SBX_CFG(unit)->p2l_node[BCM_STK_MOD_TO_NODE(modid)]
                                            == BCM_STK_MOD_TO_NODE(modid)) {
                    SOC_SBX_CFG(unit)->p2l_node[BCM_STK_MOD_TO_NODE(modid)] = SBX_MAXIMUM_NODES - 1;
                }
            }
        }

	SOC_SBX_CFG(unit)->uInterfaceProtocol = soc_property_get(unit, spn_IF_PROTOCOL, SOC_SBX_CFG(unit)->uInterfaceProtocol);
  	if (SOC_SBX_CFG(unit)->uInterfaceProtocol >= 2) {
	  SOC_ERROR_PRINT((DK_ERR, "soc_sbx_init: unit %d if_protocol value %d out of range [0, 1]",
			   unit, soc_property_get(unit, spn_IF_PROTOCOL, SOC_SBX_CFG(unit)->uInterfaceProtocol)));
	  return SOC_E_PARAM;
	}

	/* All levels and all nodes in each level are used for egress scheduler */
	SOC_SBX_CFG(unit)->num_egress_group = SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS;

    } else if (SOC_IS_SBX_SIRIUS(unit)) {
        /* Fill in Sirius specific configuration */
        if (soc_property_get(unit, spn_IF_SUBPORTS_CREATE, 0)) {
            SHR_BITSET(SOC_SBX_CFG_SIRIUS(unit)->property, IF_SUBPORTS_CREATE);
        }
        if (soc_property_get(unit, spn_TM_FABRIC_PORT_HIERARCHY_SETUP, 1)) {
            SHR_BITSET(SOC_SBX_CFG_SIRIUS(unit)->property, TM_FABRIC_PORT_HIERARCHY_SETUP);
        }
        if (soc_property_get(unit, spn_ES_FABRIC_PORT_HIERARCHY_SETUP, 1)) {
            SHR_BITSET(SOC_SBX_CFG_SIRIUS(unit)->property, ES_FABRIC_PORT_HIERARCHY_SETUP);
        } else {
	    /* when fabric port (level 2) was not setup, fabric egress (level 1/0) setup could not be done */
	    SOC_SBX_CFG(unit)->fabric_egress_setup = FALSE;
	}

        SOC_SBX_CFG(unit)->uClockSpeedInMHz = soc_property_get(unit, spn_CORE_CLOCK_SPEED,
							       SOC_SBX_CFG(unit)->uClockSpeedInMHz);
        SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns = soc_property_get(unit,spn_EXT_RAM_COLUMNS,
								  SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns);
        SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows = soc_property_get(unit,spn_EXT_RAM_ROWS,
							       SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows);
        SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumBanks = soc_property_get(unit,spn_EXT_RAM_BANKS,
								SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumBanks);
        SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumMemories = soc_property_get(unit,spn_EXT_RAM_PRESENT,
								       SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumMemories);
        SOC_SBX_CFG_SIRIUS(unit)->uDdr3ClockMhz = soc_property_get(unit, spn_SIRIUS_DDR3_CLOCK_MHZ,
								    SOC_SBX_CFG_SIRIUS(unit)->uDdr3ClockMhz);
        SOC_SBX_CFG_SIRIUS(unit)->uDdr3MemGrade = soc_property_get(unit, spn_SIRIUS_DDR3_MEM_GRADE,
								    SOC_SBX_CFG_SIRIUS(unit)->uDdr3MemGrade);

	/* Sanity check DDR config */
	if (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumBanks != 8) {
	    SOC_ERROR_PRINT((DK_ERR, "soc_sbx_init: unit %d ext_mem_banks value %d unsupported",
			     unit, SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumBanks));
	    return SOC_E_PARAM;
	}
	switch (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumMemories) {
	    case 10:
		if ((SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns == 1024) &&
		    (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows == 8192)) {
		} else if ((SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns == 1024) &&
			   (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows == 16384)) {
		} else if ((SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns == 2048) &&
			   (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows == 8192)) {
		} else {
		    SOC_ERROR_PRINT((DK_ERR, "soc_sbx_init: unit %d invalid ext mem column/row config", unit));
		    return SOC_E_PARAM;
		}
		break;
	    case 8:
		if ((SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns == 1024) &&
		    (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows == 8192)) {
		} else if ((SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns == 1024) &&
			   (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows == 16384)) {
		} else if ((SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns == 2048) &&
			   (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows == 16384)) {
		} else {
		    SOC_ERROR_PRINT((DK_ERR, "soc_sbx_init: unit %d invalid ext mem column/row config", unit));
		    return SOC_E_PARAM;
		}
		break;
	    case 6:
		if ((SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns == 1024) &&
		    (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows == 8192)) {
		} else if ((SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns == 1024) &&
			   (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows == 16384)) {
		} else {
		    SOC_ERROR_PRINT((DK_ERR, "soc_sbx_init: unit %d invalid ext mem column/row config", unit));
		    return SOC_E_PARAM;
		}
		break;
	    case 5:
		if ((SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns == 1024) &&
		    (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows == 16384)) {
		} else {
		    SOC_ERROR_PRINT((DK_ERR, "soc_sbx_init: unit %d invalid ext mem column/row config", unit));
		    return SOC_E_PARAM;
		}
		break;
	    case 4:
		if ((SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns == 1024) &&
		    (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows == 8192)) {
		} else if ((SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns == 1024) &&
			   (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows == 16384)) {
		} else if ((SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns == 2048) &&
			   (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows == 8192)) {
		} else if ((SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns == 4096) &&
			   (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows == 4096)) {
		} else {
		    SOC_ERROR_PRINT((DK_ERR, "soc_sbx_init: unit %d invalid ext mem column/row config", unit));
		    return SOC_E_PARAM;
		}
		break;
	    case 3:
		if ((SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns == 2048) &&
		    (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows == 8192)) {
		} else if ((SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns == 1024) &&
			   (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows == 16384)) {
		} else if ((SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns == 4096) &&
			   (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows == 4096)) {
		} else {
		    SOC_ERROR_PRINT((DK_ERR, "soc_sbx_init: unit %d invalid ext mem column/row config", unit));
		    return SOC_E_PARAM;
		}
		break;
	    case 2:
		if ((SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns == 1024) &&
		    (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows == 8192)) {
		} else if ((SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns == 1024) &&
			   (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows == 16384)) {
		} else if ((SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns == 2048) &&
			   (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows == 8192)) {
		} else if ((SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns == 4096) &&
			   (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows == 4096)) {
		} else if ((SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumColumns == 8192) &&
			   (SOC_SBX_CFG_SIRIUS(unit)->uDdr3NumRows == 2048)) {
		} else {
		    SOC_ERROR_PRINT((DK_ERR, "soc_sbx_init: unit %d invalid ext mem column/row config", unit));
		    return SOC_E_PARAM;
		}
		break;
	    default:
		SOC_ERROR_PRINT((DK_ERR, "soc_sbx_init: unit %d invalid ext mem column/row config", unit));
		return SOC_E_PARAM;
	}

        SOC_SBX_CFG_SIRIUS(unit)->nMaxVoq = BM9600_BW_MAX_VOQ_NUM;

        if (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME ||
            SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME_BYPASS) {
            SOC_SBX_CFG_SIRIUS(unit)->nMaxVoq = 0;
        }

        if ((SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME) || 
            (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME_BYPASS) || 
            SOC_SBX_CFG(unit)->bHybridMode) {
            /* Local grants required, config TS */

            /* All levels and all nodes in each level are used for ingress scheduler */
            SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[0] = SB_FAB_DEVICE_SIRIUS_NUM_INGRESS_SCHEDULER_L0;          /* 64K leaf nodes */
            if ( soc_property_get(unit, spn_BCM_NUM_COS, BCM_COS_COUNT) <= 4 ) {
                /* each level 1 node has 4 children */
                SOC_SBX_CFG_SIRIUS(unit)->b8kNodes = FALSE;
                SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[1] = SB_FAB_DEVICE_SIRIUS_NUM_INGRESS_SCHEDULER_L1;      /* 16K level 1 nodes */
            } else {
                /* each level 1 node has 8 children */
                SOC_SBX_CFG_SIRIUS(unit)->b8kNodes = TRUE;
                SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[1] = SB_FAB_DEVICE_SIRIUS_NUM_INGRESS_SCHEDULER_L1/2;    /* 8K level 1 nodes */
            }

            SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[2] = SB_FAB_DEVICE_SIRIUS_NUM_INGRESS_SCHEDULER_L2;          /* 4K level 2 nodes */
            SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[3] = SB_FAB_DEVICE_SIRIUS_NUM_INGRESS_SCHEDULER_L3;          /* 1K level 3 nodes */
            SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[4] = SB_FAB_DEVICE_SIRIUS_NUM_INGRESS_SCHEDULER_L4;          /* 264 level 4 nodes */
            SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[5] = SB_FAB_DEVICE_SIRIUS_NUM_INGRESS_SCHEDULER_L5;          /* 132 level 5 nodes */
            SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[6] = SB_FAB_DEVICE_SIRIUS_NUM_INGRESS_SCHEDULER_L6;          /* 56 level 6 nodes */
            SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[7] = SB_FAB_DEVICE_SIRIUS_NUM_INGRESS_SCHEDULER_L7;          /* 7 level 7nodes */

            SOC_SBX_CFG(unit)->num_ingress_scheduler = 0;
            SOC_SBX_CFG(unit)->num_ingress_multipath = 0;
            for (level = 0; level < SB_FAB_DEVICE_SIRIUS_NUM_TS_LEVELS; level++) {
                SOC_SBX_CFG(unit)->num_ingress_scheduler += SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level];
		if ((level >= 2) && (level <= 6)) {
		    /* only level 2 to 6 support ingress multipath shaper */
		    SOC_SBX_CFG(unit)->num_ingress_multipath += SOC_SBX_CFG_SIRIUS(unit)->uNumTsNode[level];
		}
            }
        }

	SOC_SBX_CFG(unit)->uInterfaceProtocol = soc_property_get(unit, spn_IF_PROTOCOL, SOC_SBX_CFG(unit)->uInterfaceProtocol);
	if (SOC_SBX_CFG(unit)->uInterfaceProtocol >= 2) {
	  SOC_ERROR_PRINT((DK_ERR, "soc_sbx_init: unit %d if_protocol value %d out of range [0, 1]",
			   unit, soc_property_get(unit, spn_IF_PROTOCOL, SOC_SBX_CFG(unit)->uInterfaceProtocol)));
	  return SOC_E_PARAM;
	}

        /*
         *  Set split point in ep_oi2qb_map table.  Below this point is space
         *  multicast will use for repId to OI translation.  At and above this
         *  point is space that will be used for requeueing.
         *
         *  Valid range for this is 0x080..0x100 in XGS mode,
         *  and 0x080..0x0F0 in SBX mode.  Values below 0x080 may cause some
         *  issues with multicast.  Values above 0x0F0 may cause problems with
         *  SBX mode requeue behaviour.  The value 0x100 indicates that the
         *  entire space goes to multicast, and can only be used in XGS mode.
         */
        if (SOC_SBX_CFG(unit)->uInterfaceProtocol == SOC_SBX_IF_PROTOCOL_XGS) {
            /* Use all of ep_oi2qb_map for multicast in XGS mode */
            SOC_SBX_CFG_SIRIUS(unit)->requeueMinPage = 0x100;
        } else {
            /* Use high 8K of ep_oi2qb_map for requeue in SBX mode */
            SOC_SBX_CFG_SIRIUS(unit)->requeueMinPage = 0x0F0;
        }

	/* All levels and all nodes in each level are used for egress scheduler */
	SOC_SBX_CFG(unit)->num_egress_group = SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS;

        for(port=0; port<SB_FAB_DEVICE_SIRIUS_NUM_SERIALIZERS; port++){
            /* User defines for link driver config */
            SOC_SBX_CFG_SIRIUS(unit)->linkDriverConfig[port].uDriverStrength =
                soc_property_port_get(unit, port, spn_LINK_DRIVER_STRENGTH, SOC_SBX_CFG_SIRIUS(unit)->linkDriverConfig[port].uDriverStrength);

            SOC_SBX_CFG_SIRIUS(unit)->linkDriverConfig[port].uDriverEqualization =
                soc_property_port_get(unit, port, spn_LINK_DRIVER_EQUALIZATION, SOC_SBX_CFG_SIRIUS(unit)->linkDriverConfig[port].uDriverEqualization);
        }

        /* Local grant config */
        if (SOC_SBX_CFG_SIRIUS(unit)->bDualLocalGrants && !SOC_SBX_CFG(unit)->bHybridMode) {
            /*
             * when dual grant, 1st grant goes to plane A interfaces while 2nd grant goes to
             * plane B interfaces. Evenly distribute mc/higig interfaces on plane A and plane B
             * assign cpu interface to plane A. When a port is disabled, set the child nodes
             * of the interface to always report priority 0
	     plane = SB_FAB_DEVICE_SIRIUS_LOCAL_PLANE_A;
	     for(hg = 0; hg < SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS; hg++) {
	       intf = SB_FAB_DEVICE_SIRIUS_HG0_INTF + hg;
	       if (soc_property_port_get(unit, hg, spn_IF_SUBPORTS, SOC_SBX_SIRIUS_STATE(unit)->uNumExternalSubports[intf])) {
	         SOC_SBX_CFG_SIRIUS(unit)->uInterfacePlane[intf] = plane;
	         if (plane == SB_FAB_DEVICE_SIRIUS_LOCAL_PLANE_A) {
	           plane = SB_FAB_DEVICE_SIRIUS_LOCAL_PLANE_B;
	         } else {
	           plane = SB_FAB_DEVICE_SIRIUS_LOCAL_PLANE_A;
	         }
	       }
	     }
             SOC_SBX_CFG_SIRIUS(unit)->uInterfacePlane[SB_FAB_DEVICE_SIRIUS_RQ0_INTF] = SB_FAB_DEVICE_SIRIUS_LOCAL_PLANE_A;
             SOC_SBX_CFG_SIRIUS(unit)->uInterfacePlane[SB_FAB_DEVICE_SIRIUS_RQ1_INTF] = SB_FAB_DEVICE_SIRIUS_LOCAL_PLANE_B;
             SOC_SBX_CFG_SIRIUS(unit)->uInterfacePlane[SB_FAB_DEVICE_SIRIUS_CPU_INTF] = SB_FAB_DEVICE_SIRIUS_LOCAL_PLANE_A;
	    */
	    /* hardcode plane for higig interfaces for easier NULL grants management */
            SOC_SBX_CFG_SIRIUS(unit)->uInterfacePlane[SB_FAB_DEVICE_SIRIUS_CPU_INTF] = SB_FAB_DEVICE_SIRIUS_LOCAL_PLANE_A;
	    SOC_SBX_CFG_SIRIUS(unit)->uInterfacePlane[SB_FAB_DEVICE_SIRIUS_HG0_INTF] = SB_FAB_DEVICE_SIRIUS_LOCAL_PLANE_B;
	    SOC_SBX_CFG_SIRIUS(unit)->uInterfacePlane[SB_FAB_DEVICE_SIRIUS_HG1_INTF] = SB_FAB_DEVICE_SIRIUS_LOCAL_PLANE_A;
	    SOC_SBX_CFG_SIRIUS(unit)->uInterfacePlane[SB_FAB_DEVICE_SIRIUS_HG2_INTF] = SB_FAB_DEVICE_SIRIUS_LOCAL_PLANE_B;
	    SOC_SBX_CFG_SIRIUS(unit)->uInterfacePlane[SB_FAB_DEVICE_SIRIUS_HG3_INTF] = SB_FAB_DEVICE_SIRIUS_LOCAL_PLANE_A;	    
            SOC_SBX_CFG_SIRIUS(unit)->uInterfacePlane[SB_FAB_DEVICE_SIRIUS_RQ0_INTF] = SB_FAB_DEVICE_SIRIUS_LOCAL_PLANE_B;
            SOC_SBX_CFG_SIRIUS(unit)->uInterfacePlane[SB_FAB_DEVICE_SIRIUS_RQ1_INTF] = SB_FAB_DEVICE_SIRIUS_LOCAL_PLANE_A;
        } else {
            /*
             * When not dual grant, all enabled interfaces are at plane A.
             * all disabled interfaces are at plane B
             */
            for(hg = 0; hg < SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS; hg++) {
                intf = SB_FAB_DEVICE_SIRIUS_HG0_INTF + hg;
                if (soc_property_port_get(unit, hg, spn_IF_SUBPORTS, SOC_SBX_SIRIUS_STATE(unit)->uNumExternalSubports[intf])) {
                    SOC_SBX_CFG_SIRIUS(unit)->uInterfacePlane[intf] = SB_FAB_DEVICE_SIRIUS_LOCAL_PLANE_A;
                } else {
                    SOC_SBX_CFG_SIRIUS(unit)->uInterfacePlane[intf] = SB_FAB_DEVICE_SIRIUS_LOCAL_PLANE_B;
                }
            }
            /* MC0, MC1, CPU are assumed to be always enabled and on plane A */
            SOC_SBX_CFG_SIRIUS(unit)->uInterfacePlane[SB_FAB_DEVICE_SIRIUS_RQ0_INTF] = SB_FAB_DEVICE_SIRIUS_LOCAL_PLANE_A;
            SOC_SBX_CFG_SIRIUS(unit)->uInterfacePlane[SB_FAB_DEVICE_SIRIUS_RQ1_INTF] = SB_FAB_DEVICE_SIRIUS_LOCAL_PLANE_A;
            SOC_SBX_CFG_SIRIUS(unit)->uInterfacePlane[SB_FAB_DEVICE_SIRIUS_CPU_INTF] = SB_FAB_DEVICE_SIRIUS_LOCAL_PLANE_A;
        }

	/* count total number of configed higig subports */
	for(hg = 0; hg < SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS; hg++) {
	    intf = SB_FAB_DEVICE_SIRIUS_HG0_INTF + hg;
            SOC_SBX_SIRIUS_STATE(unit)->nMaxFabricPorts += soc_property_port_get(unit, hg, spn_IF_SUBPORTS,
                                                                                 SOC_SBX_SIRIUS_STATE(unit)->uNumExternalSubports[intf]);
	}
        if (SOC_SBX_SIRIUS_STATE(unit)->nMaxFabricPorts > 128) {
	    /* support this feature only under TME or INLINE mode for now */
	    if ((SOC_SBX_CFG(unit)->bTmeMode != SOC_SBX_QE_MODE_TME) &&
		(SOC_SBX_CFG(unit)->bTmeMode != SOC_SBX_QE_MODE_TME_BYPASS)) {
		SOC_ERROR_PRINT((DK_ERR, "soc_sbx_init: unit %d only TME or TME_BYPASS mode supports more than 128 ports", unit));
		return SOC_E_PARAM;
	    }

	    /* when more than 128 subports, has to use independent fc mode */
	    SOC_SBX_CFG(unit)->bEgressFifoIndependentFlowControl = TRUE;
	    SOC_SBX_CFG_SIRIUS(unit)->bExtendedPortMode = TRUE;
	} else {
	    SOC_SBX_CFG_SIRIUS(unit)->bExtendedPortMode = FALSE;
	}

        for (intf = 0; intf < SB_FAB_DEVICE_SIRIUS_MAX_SCHED_INTERFACES; intf++) {
            /* don't assign weight to interfaces */
            SOC_SBX_CFG_SIRIUS(unit)->uInterfaceWeight[intf] = 0;

            /* try to configure the interface length adjustment */
            port = soc_property_port_get(unit, intf, spn_QE_EGR_SHAPING_ADJUST, -1);
            if ((0 > port) || (31 < port)) {
                port = soc_property_get(unit, spn_QE_EGR_SHAPING_ADJUST, -1);
            }
            if ((0 <= port) && (31 >= port)) {
                SOC_SBX_CFG_SIRIUS(unit)->shapingBusLengthAdj[intf] = port;
            }
        }

        SOC_SBX_CFG_SIRIUS(unit)->uQmMaxArrivalRateMbs = soc_property_get(unit,
                        spn_QE_MAX_ARRIVAL_RATE, SOC_SBX_CFG_SIRIUS(unit)->uQmMaxArrivalRateMbs);
        SOC_SBX_CFG_SIRIUS(unit)->thresh_drop_limit = soc_property_get(unit,
                        spn_QE_THRESH_DROP_LIMIT, SOC_SBX_CFG_SIRIUS(unit)->thresh_drop_limit);
    } else if (SOC_IS_SBX_FE2000(unit)) {

        if ( (SOC_SBX_CFG(unit)->num_nodes < SOC_INFO(unit).modid_max) &&
                                                 (SOC_SBX_CFG(unit)->num_nodes > 1) ) {
            SOC_INFO(unit).modid_max = SOC_SBX_CFG(unit)->num_nodes - 1;
        }
    }


#ifdef BCM_WARM_BOOT_SUPPORT
    /*
     * Currently, Warmboot not fully supported on sirius, or LCMs
     */
    if ((SOC_IS_SBX_BME3200(unit) && 
         (SOC_SBX_CFG_BM3200(unit)->uDeviceMode != SOC_SBX_BME_LCM_MODE)) ||
        SOC_IS_SIRIUS(unit) == FALSE)
    {
 	SOC_IF_ERROR_RETURN(soc_sbx_wb_init(unit));
    }
#endif


    /* Set feature cache */
    if (SOC_DRIVER(unit)->feature) {
        soc_feature_init(unit);
    }

    /*
     * PHY drivers and ID map
     */
    SOC_IF_ERROR_RETURN(soc_phyctrl_software_init(unit));

    if (SOC_IS_SBX_FE2000(unit) &&
        SOC_CONTROL(unit)->soc_flags & SOC_F_INITED) {
        /* stop the counter thread */
        soc_sbx_counter_stop(unit);
    }

    rv = sbx->init_func(unit, SOC_SBX_CFG(unit));
    if (rv != SOC_E_NONE) {
        return rv;
    }

    /*
     * Reset fusion core on HG XE
     */

    if (SOC_IS_SBX_FE2000(unit)) {
        soc_port_t  port;
        int         speed;

        PBMP_HG_ITER(unit, port) {
            speed = 10000;
            if (soc_feature(unit, soc_feature_xgxs_lcpll)) {
                if (soc_property_port_get(unit, port,
                                          spn_XGXS_LCPLL_12GBPS, 0)) {
                    speed = 12000; /* 12 Gbps */
                }
            }
            SOC_IF_ERROR_RETURN(soc_fe2000_fusioncore_reset(unit,
                                                            port, speed));
        }
        PBMP_XE_ITER(unit, port) {
            SOC_IF_ERROR_RETURN(soc_fe2000_fusioncore_reset(unit,
                                                            port, 10000));
        }
        PBMP_XE_ITER(unit, port) {
	  SOC_IF_ERROR_RETURN(soc_sbx_fe2000_xm_rxtx(unit, port));
	}
    }


    sbx->init = 1;  /* HW Initialized */
    if (soc->soc_flags & SOC_F_INITED) {
        soc->soc_flags |= SOC_F_RE_INITED;
    }
    soc->soc_flags |= SOC_F_INITED;

    return rv;
}


int
soc_sbx_reset_init(int unit) {
    /* force the reset */
    SOC_SBX_CFG(unit)->reset_ul = 1;

    return soc_sbx_init(unit);
}


/*
 * Function:
 *     soc_sbx_block_find
 * Purpose:
 *     Find block and returns index to SOC block info array.
 * Parameters:
 *     unit   - Device number
 *     type   - Block type to match SOC_BLK_xxx
 *     number - Block instance to match
 * Returns:
 *     >= 0  Index to soc_block_info array in SOC driver
 *     -1    If block type/instance was not found
 */
int
soc_sbx_block_find(int unit, int type, int number)
{
    int blk;

    for (blk = 0; ; blk++) {

        if (SOC_BLOCK_INFO(unit, blk).type < 0) { /* End of list */
            return -1;
        }

        if ((SOC_BLOCK_INFO(unit, blk).type   == type) &&
            (SOC_BLOCK_INFO(unit, blk).number == number)) { /* Found */
            return blk;
        }
    }

    return -1;
}

int
soc_sbx_node_port_get(int unit, int module_id, int port,
                      int *fab_unit, int *node_id, int *fabric_port)
{
    int modport;
    int mymod;
    int node;

    if (SOC_IS_SBX_FE(unit)) {
        mymod = SOC_SBX_CONTROL(unit)->module_id0;
    } else if (SOC_IS_SBX_QE(unit)) {
        mymod = BCM_STK_NODE_TO_MOD(SOC_SBX_CONTROL(unit)->node_id);
    } else {
        return SOC_E_UNIT;
    }

    if (port >= SBX_MAX_PORTS) {
        return SOC_E_PARAM;
    }

    modport = SOC_SBX_CONTROL(unit)->modport[module_id][port];
    if (BCM_GPORT_IS_CHILD(modport)) {
        node = BCM_GPORT_CHILD_MODID_GET(modport) - SBX_QE_BASE_MODID;
        *fabric_port = BCM_GPORT_CHILD_PORT_GET(modport);
    } else {
        node = ((modport >> 16) & 0xffff) - SBX_QE_BASE_MODID;
        *fabric_port = modport & 0xffff;
    }

    if (node < 0 || node > SBX_MAX_NODES) {
        SOC_ERROR_PRINT((DK_ERR, "invalid modport found on unit %d "
                         " module %d port %d:\n  (fabric_node=%d "
                         "fabric_modid=%d fabric_port=%d)\n",
                         unit, module_id, port, node,
                         modport >> 16, modport & 0xffff));
        return SOC_E_PARAM;
    }

    *node_id = node;

    if (module_id == mymod) {
        *fab_unit = SOC_SBX_QE_FROM_FE(unit, port);
    } else {
        *fab_unit = SBX_INVALID_UNIT;
    }

    return SOC_E_NONE;
}


int
soc_sbx_modid_get(int unit, int f_node, int f_port, int *module)
{
    int i, j;

    if (!(SOC_SBX_NODE_VALID(unit, f_node)))
        return SOC_E_NOT_FOUND;

    if (SOC_IS_SBX_FE2000(unit)) {
        for (i = 0; i < SBX_MAX_MODIDS; i++)    {
            for (j = 0; j < SBX_MAX_PORTS; j++) {
                int modport, node, port;
                modport = SOC_SBX_CONTROL(unit)->modport[i][j];
                node = ((modport >> 16) & 0xffff) - SBX_QE_BASE_MODID;
                port = modport & 0xffff;
                if (f_node == node && f_port == port) {
                    *module = i;
                    return SOC_E_NONE;
                }
            }
        }
    } else if (SOC_IS_SBX_QE2000(unit)) {
        SOC_SBX_MODID_FROM_NODE(f_node, *module);
    }

    return SOC_E_NONE;
}

int
soc_sbx_register(int unit, const soc_sbx_functions_t *sbx_functions)
{
    soc_control_t *drv;
    soc_sbx_control_t *sbx_drv;

    drv = SOC_CONTROL(unit);
    if (!drv->soc_flags & SOC_F_ATTACHED) {
        return SOC_E_FAIL;
    }

    sbx_drv = SOC_SBX_CONTROL(unit);
    if (sbx_drv == NULL) {
        return SOC_E_FAIL;
    }

    sbx_drv->sbx_functions.sram_init = sbx_functions->sram_init;
    sbx_drv->sbx_functions.ddr_train = sbx_functions->ddr_train;
#ifdef BCM_FE2000_SUPPORT
    sbx_drv->sbx_functions.fe2kxt_ddrconfig_rw = sbx_functions->fe2kxt_ddrconfig_rw;
#endif
    return SOC_E_NONE;
}

int
soc_sbx_unregister(int unit)
{
    soc_control_t *drv;
    soc_sbx_control_t *sbx_drv;

    drv = SOC_CONTROL(unit);
    if (!drv->soc_flags & SOC_F_ATTACHED) {
        return SOC_E_FAIL;
    }

    sbx_drv = SOC_SBX_CONTROL(unit);
    if (sbx_drv == NULL) {
        return SOC_E_FAIL;
    }

    sbx_drv->sbx_functions.sram_init = NULL;

    return SOC_E_NONE;
}

void
soc_sbx_xport_type_update(int unit, soc_port_t port, int to_hg_port)
{
    soc_info_t          *si;
    soc_port_t          it_port;

    si = &SOC_INFO(unit);

    /* We need to lock the SOC structures until we finish the update */
    /* SOC_CONTROL_LOCK(unit); */

    if (to_hg_port) {
        /* SOC_PBMP_PORT_ADD(si->st.bitmap, port); */
        SOC_PBMP_PORT_ADD(si->hg.bitmap, port);
        SOC_PBMP_PORT_REMOVE(si->ether.bitmap, port);
        SOC_PBMP_PORT_REMOVE(si->xe.bitmap, port);
    } else {
        SOC_PBMP_PORT_ADD(si->ether.bitmap, port);
        SOC_PBMP_PORT_ADD(si->xe.bitmap, port);
        /* SOC_PBMP_PORT_REMOVE(si->st.bitmap, port); */
        SOC_PBMP_PORT_REMOVE(si->hg.bitmap, port);
    }

#undef RECONFIGURE_PORT_TYPE_INFO
#define RECONFIGURE_PORT_TYPE_INFO(ptype, ptype_str) \
    si->ptype.num = 0; \
    si->ptype.min = si->ptype.max = -1; \
    PBMP_ITER(si->ptype.bitmap, it_port) { \
        if (ptype_str != NULL) { \
            sal_snprintf(si->port_name[it_port], \
                         sizeof(si->port_name[it_port]), \
                         "%s%d", ptype_str, si->ptype.num); \
        } \
        si->ptype.port[si->ptype.num++] = it_port; \
        if (si->ptype.min < 0) { \
            si->ptype.min = it_port; \
        } \
        if (it_port > si->ptype.max) { \
            si->ptype.max = it_port; \
        } \
    }

    /* Recalculate port type data */
    RECONFIGURE_PORT_TYPE_INFO(ether, NULL);
    RECONFIGURE_PORT_TYPE_INFO(st, NULL);
    RECONFIGURE_PORT_TYPE_INFO(hg, "hg");
    RECONFIGURE_PORT_TYPE_INFO(xe, "xe");
#undef  RECONFIGURE_PORT_TYPE_INFO

    /* Release SOC structures lock */
    /* SOC_CONTROL_UNLOCK(unit); */
}

int
soc_sbx_set_epoch_length(int unit)
{
    int     rc = SOC_E_NONE, num_queues, num_cos;
    uint32  epoch_length;

    if ((SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME) &&
        !(SOC_SBX_CFG(unit)->bHybridMode)) {
        epoch_length = SB_FAB_TME_EPOCH_IN_TIMESLOTS;
    } else {
        epoch_length = SB_FAB_DMODE_EPOCH_IN_TIMESLOTS;
    }
    /* check if "optimize QOS" is set. Applicable only for FIC mode */
    if ((soc_property_get(unit, spn_FABRIC_QOS_OPTIMIZE, FALSE) == TRUE) &&
        (SOC_SBX_CFG(unit)->bHybridMode == FALSE) &&
        (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_FIC)) {

        num_cos = soc_property_get(unit, spn_BCM_NUM_COS, BCM_COS_COUNT);
        /* unicast and multicast queues */
        num_queues = (SOC_SBX_CFG(unit)->cfg_num_nodes *
                      SOC_SBX_CFG(unit)->max_ports * num_cos) +
                     (SOC_SBX_CFG(unit)->num_ds_ids * num_cos);
        SOC_DEBUG_PRINT((DK_VERBOSE, "unit: %d, NumQueues: %d(0x%x)\n",
                         unit, num_queues, num_queues));

        switch (SOC_SBX_CFG(unit)->uFabricConfig) {
        case SOC_SBX_SYSTEM_CFG_DMODE: /* BM3200 + QE2000 */
            SOC_SBX_CFG(unit)->num_queues = num_queues;
            rc = soc_bm3200_epoch_in_timeslot_config_get(unit, num_queues,
                                                         &epoch_length);
            break;
        case SOC_SBX_SYSTEM_CFG_VPORT:        /* Vport - Bm9600 + Qe4000 */
        case SOC_SBX_SYSTEM_CFG_VPORT_LEGACY: /* Vport Legacy - Bm9600 + Qe2000 */
        case SOC_SBX_SYSTEM_CFG_VPORT_MIX:    /* Vport Mix - Bm9600 + Qe2000 + Qe4000 */
            SOC_SBX_CFG(unit)->num_queues = num_queues;
            rc = soc_bm9600_epoch_in_timeslot_config_get(unit, num_queues,
                                                         &epoch_length);
        default:
            /* default epoch length assigned above */
            break;
        }
    }

    SOC_SBX_CFG(unit)->epoch_length_in_timeslots = epoch_length;
    SOC_DEBUG_PRINT((DK_VERBOSE, "unit-%d: epochLen: %d(0x%x)\n",
                unit, epoch_length, epoch_length));
    return rc;
}

soc_sbx_ucode_type_t
soc_sbx_configured_ucode_get(int unit)
{
    char *ucodetype;

    if (SOC_IS_SBX_FE2KXT(unit)) {
        ucodetype = soc_property_get_str(unit, spn_BCM88025_UCODE);
    } else {
        ucodetype = soc_property_get_str(unit, spn_BCM88020_UCODE);
    }

    /* default to g2p3, or if g2p3 occurs anywhere in the named ucode */
    if (!ucodetype || strstr(ucodetype, "g2p3")) {
        return SOC_SBX_UCODE_TYPE_G2P3;
    } else {
        /* else its customer ucode */
        return SOC_SBX_UCODE_TYPE_G2XX;
    }
}


#if defined(BROADCOM_DEBUG)

#define P soc_cm_print
int
soc_sbx_dump(int unit, const char *pfx)
{
    soc_control_t       *soc;
    soc_persist_t       *sop;
    soc_stat_t          *stat;
    uint16              dev_id;
    uint8               rev_id;

    if (!SOC_UNIT_VALID(unit)) {
        return(SOC_E_UNIT);
    }

    soc = SOC_CONTROL(unit);
    sop = SOC_PERSIST(unit);

    stat = &soc->stat;

    P("%sUnit %d Driver Control Structure:\n", pfx, unit);

    soc_cm_get_id(unit, &dev_id, &rev_id);

    P("%sChip=%s Rev=0x%02x Driver=%s\n",
      pfx,
      soc_dev_name(unit),
      rev_id,
      SOC_CHIP_NAME(soc->chip_driver->type));
    P("%sFlags=0x%x:",
      pfx, soc->soc_flags);
    if (soc->soc_flags & SOC_F_ATTACHED)        P(" attached");
    if (soc->soc_flags & SOC_F_INITED)          P(" initialized");
    if (soc->soc_flags & SOC_F_LSE)             P(" link-scan");
    if (soc->soc_flags & SOC_F_SL_MODE)         P(" sl-mode");
    if (soc->soc_flags & SOC_F_POLLED)          P(" polled");
    if (soc->soc_flags & SOC_F_URPF_ENABLED)    P(" urpf");
    if (soc->soc_flags & SOC_F_MEM_CLEAR_USE_DMA) P(" mem-clear-use-dma");
    if (soc->soc_flags & SOC_F_IPMCREPLSHR)     P(" ipmc-repl-shared");
    if (soc->remote_cpu)                P(" rcpu");
    P("; board type 0x%x", soc->board_type);
    P("\n");
    P("%s", pfx);
    soc_cm_dump(unit);

    return(0);
}
#endif /* BROADCOM_DEBUG */

/*
 * Function:
 *     soc_sbx_chip_dump
 * Purpose:
 *     Display SBX driver and chip information.
 * Parameters:
 *      unit - Device unit number
 *      d    - Device driver
 * Notes:
 *      Pass unit -1 to avoid referencing unit number.
 */
void
soc_sbx_chip_dump(int unit, soc_driver_t *d)
{
#ifdef BROADCOM_DEBUG

    soc_info_t          *si;
    int                 i, count = 0;
    soc_port_t          port;
    char                pfmt[SOC_PBMP_FMT_LEN];
    uint16              dev_id;
    uint8               rev_id;
    int                 blk, blk_number, blk_index;
    soc_block_t         blk_type;
    char                *bname;
    soc_sbx_control_t   *sbx;

    if (d == NULL) {
        P("unit %d: no driver attached\n", unit);
        return;
    }

    if (((unit) < 0) || ((unit) >= SOC_MAX_NUM_DEVICES)) {
        P("invalid unit:%d\n", unit);
        return;
    }

    sbx = SOC_SBX_CONTROL(unit);

    P("driver %s (%s)\n", SOC_CHIP_NAME(d->type), d->chip_string);
    P("\tregsfile\t\t%s\n", d->origin);
    P("\tpci identifier\t\tvendor 0x%04x device 0x%04x rev 0x%02x\n",
      d->pci_vendor, d->pci_device, d->pci_revision);
    P("\tclasses of service\t%d\n", d->num_cos);
    P("\tmaximums\t\tblock %d ports %d mem_bytes %d\n",
      SOC_MAX_NUM_BLKS, SOC_MAX_NUM_PORTS, SOC_MAX_MEM_BYTES);

    /* Device Block Information */
    if (d->block_info) {
        for (blk = 0; d->block_info[blk].type >= 0; blk++) {
            bname = soc_block_name_lookup_ext(d->block_info[blk].type, unit);
            P("\tblk %d\t\t%s%d\tschan %d cmic %d\n",
              blk,
              bname,
              d->block_info[blk].number,
              d->block_info[blk].schan,
              d->block_info[blk].cmic);
        }

        /* Device Block Port Information */
        if (d->port_info) {

            for (port = 0; ; port++) {

                /* Front-panel port information */
                blk        = SOC_PORT_BLOCK(unit, port);
                blk_type   = SOC_PORT_BLOCK_TYPE(unit, port);
                blk_number = SOC_PORT_BLOCK_NUMBER(unit, port);
                blk_index  = SOC_PORT_BLOCK_INDEX(unit, port);

                if (blk < 0 && blk_index < 0) {    /* end of list */
                    break;
                }
                if (blk < 0) {                     /* empty slot */
                    continue;
                }

                P("\tport %d\t\t%s\tblk %d %s%d",
                  port,
                  (blk_type == SOC_BLK_GXPORT) ?
                  (SOC_IS_SIRIUS(unit) ? "hg" : ((SOC_IS_SBX_BM9600(unit) ? (IS_SFI_PORT(unit, port) ? "sfi" : "sci") : "xe"))) :
                  ((blk_type == SOC_BLK_BSE) || (blk_type == SOC_BLK_CSE)) ?
                  (IS_SFI_PORT(unit, port) ? "sfi" : "sci") :
                  soc_block_port_name_lookup_ext(blk_type, unit),
                  blk, soc_block_name_lookup_ext(blk_type, unit),
                  blk_number);

                if (blk_index >= 0) {
                    P(".%d", blk_index);
                }

                /* System-side port information */
                if (sbx && sbx->system_port_info) {
                    blk = SOC_SBX_SYSTEM_PORT_BLOCK(unit, port);
                    if (blk >= 0) {
                        blk_type   = SOC_SBX_SYSTEM_PORT_BLOCK_TYPE(unit,
                                                                    port);
                        blk_number = SOC_SBX_SYSTEM_PORT_BLOCK_NUMBER(unit,
                                                                      port);
                        blk_index  = SOC_SBX_SYSTEM_PORT_BLOCK_INDEX(unit,
                                                                     port);
                        P("\t- system blk %d %s%d.%d",
                          blk, soc_block_name_lookup_ext(blk_type, unit),
                          blk_number, blk_index);
                    }
                }

                P("\n");
            }
        }
    }

    si = &SOC_INFO(unit);
    soc_cm_get_id(unit, &dev_id, &rev_id);

    P("unit %d:\n", unit);
    P("\tpci\t\t\tdevice %04x rev %02x\n", dev_id, rev_id);
    P("\tdriver\t\t\ttype %d (%s) group %d (%s)\n",
      si->driver_type, SOC_CHIP_NAME(si->driver_type),
      si->driver_group, soc_chip_group_names[si->driver_group]);
    P("\tchip\t\t\t%s%s%s%s%s%s\n",
      SOC_IS_SBX_FE2000(unit) ? "fe2000 " : "",
      SOC_IS_SBX_QE2000(unit) ? "qe2000 " : "",
      SOC_IS_SBX_BME3200(unit) ? "bme3200 " : "",
      SOC_IS_SBX_BM9600(unit) ? "bm9600 " : "",
      SOC_IS_SBX_SIRIUS(unit) ? "sirius " : "",
      SOC_IS_SBX(unit) ? "sbx " : "");
    P("\tmax modid\t\t%d\n", si->modid_max);
    P("\tnum ports\t\t%d\n", si->port_num);
    P("\tnum modids\t\t%d\n", si->modid_count);
    P("\tnum blocks\t\t%d\n", si->block_num);

    if (si->ge.num)
      P("\tGE ports\t%d\t%s (%d:%d)\n",
        si->ge.num, SOC_PBMP_FMT(si->ge.bitmap, pfmt),
        si->ge.min, si->ge.max);
    if (si->xe.num)
      P("\tXE ports\t%d\t%s (%d:%d)\n",
        si->xe.num, SOC_PBMP_FMT(si->xe.bitmap, pfmt),
        si->xe.min, si->xe.max);
    if (si->hg.num)
      P("\tHG ports\t%d\t%s (%d:%d)\n",
        si->hg.num, SOC_PBMP_FMT(si->hg.bitmap, pfmt),
        si->hg.min, si->hg.max);
    if (si->spi.num)
      P("\tSPI ports\t%d\t%s (%d:%d)\n",
        si->spi.num, SOC_PBMP_FMT(si->spi.bitmap, pfmt),
        si->spi.min, si->spi.max);
    if (si->spi_subport.num)
      P("\tSPI subports\t%d\t%s (%d:%d)\n",
        si->spi_subport.num, SOC_PBMP_FMT(si->spi_subport.bitmap, pfmt),
        si->spi_subport.min, si->spi_subport.max);
    if (si->sci.num)
      P("\tSCI ports\t%d\t%s (%d:%d)\n",
        si->sci.num, SOC_PBMP_FMT(si->sci.bitmap, pfmt),
        si->sci.min, si->sci.max);
    if (si->sfi.num)
      P("\tSFI ports\t%d\t%s (%d:%d)\n",
        si->sfi.num, SOC_PBMP_FMT(si->sfi.bitmap, pfmt),
        si->sfi.min, si->sfi.max);
    if (si->ether.num)
      P("\tETHER ports\t%d\t%s (%d:%d)\n",
        si->ether.num, SOC_PBMP_FMT(si->ether.bitmap, pfmt),
        si->ether.min, si->ether.max);
    if (si->port.num)
      P("\tPORT ports\t%d\t%s (%d:%d)\n",
        si->port.num, SOC_PBMP_FMT(si->port.bitmap, pfmt),
        si->port.min, si->port.max);
    if (si->all.num)
      P("\tALL ports\t%d\t%s (%d:%d)\n",
        si->all.num, SOC_PBMP_FMT(si->all.bitmap, pfmt),
        si->all.min, si->all.max);
    SOC_PBMP_COUNT(si->cmic_bitmap, i);
    if (i)
      P("\tCPU port\t%d\t%s (%d)\n",
        i,
        SOC_PBMP_FMT(si->cmic_bitmap, pfmt), si->cmic_port);

    for (i = 0; i < COUNTOF(si->has_block); i++) {
        if (si->has_block[i]) {
            count++;
        }
    }
    P("\thas blocks\t%d\t", count);
    for (i = 0; i < COUNTOF(si->has_block); i++) {
        if (si->has_block[i]) {
            P("%s ", soc_block_name_lookup_ext(si->has_block[i], unit));
            if ((i) && !(i%6)) {
                P("\n\t\t\t\t");
            }
        }
    }
    P("\n");
    P("\tport names\t\t");
    for (port = 0; port < si->port_num; port++) {
        if (port > 0 && (port % 4) == 0) {
            P("\n\t\t\t\t");
        }
        P("%2d=%-8s ",
          port,
          (si->port_name[port][0] == '?') ? " " : si->port_name[port]);
    }
    P("\n");

    if (d->block_info) {
        i = 0;
        for (blk = 0; d->block_info[blk].type >= 0; blk++) {
            if (SOC_PBMP_IS_NULL(si->block_bitmap[blk])) {
                continue;
            }
            if (++i == 1) {
                P("\tblock bitmap\t");
            } else {
                P("\n\t\t\t");
            }
            P("%d\t%s\t%s (%d ports)",
              blk,
              si->block_name[blk],
              SOC_PBMP_FMT(si->block_bitmap[blk], pfmt),
              si->block_valid[blk]);
        }
        if (i > 0) {
            P("\n");
        }
    }

    {
        soc_feature_t f;

        P("\tfeatures\t");
        i = 0;
        for (f = 0; f < soc_feature_count; f++) {
            if (soc_feature(unit, f)) {
                if (++i > 3) {
                    P("\n\t\t\t");
                    i = 1;
                }
                P("%s ", soc_feature_name[f]);
            }
        }
        P("\n");
    }

#endif /* BROADCOM_DEBUG */
}

#ifdef P
#undef P
#endif /* P */
