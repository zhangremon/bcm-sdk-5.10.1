/*
 * $Id: fe2000.c 1.113 Broadcom SDK $
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
 * Implementation of FE2000 SOC Initialization
 *
 * NOTE:
 * SOC driver infrastructure cleanup pending.
 */

#ifdef BCM_FE2000_SUPPORT

#include <sal/appl/sal.h>

#include <soc/drv.h>
#include <soc/cmtypes.h>
#include <soc/debug.h>
#include <soc/linkctrl.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/sbx_txrx.h>
#include <soc/sbx/fe2000.h>
#include <soc/sbx/fe2k/sbFe2000Init.h>
#include <soc/sbx/fe2kxt/sbFe2000XtInit.h>
#include <soc/sbx/fe2k/sbFe2000Util.h>
#include <soc/sbx/fe2k_common/sbFe2000UcodeLoad.h>
#include <soc/sbx/fe2k_common/sbFe2000CmuMgr.h>
#ifdef BCM_FE2000_P3_SUPPORT
#include <soc/sbx/g2p3/g2p3.h>
#endif /* BCM_FE2000_P3_SUPPORT */
#ifdef BCM_FE2000_G2XX_SUPPORT
#include <soc/sbx/g2xx/g2xx.h>
#endif /* BCM_FE2000_G2XX_SUPPORT */
#include <soc/sbx/hal_ca_c2.h>
#include <bcm_int/sbx/error.h>

/*
 * Debug Output Macros
 */
#define INTR_DEBUG(stuff)  SOC_DEBUG(SOC_DBG_INTR, stuff)
extern uint32 soc_debug_level;

extern mac_driver_t           soc_fe2000_unimac;
extern mac_driver_t           soc_fe2000_xmac;
extern soc_linkctrl_driver_t  soc_linkctrl_driver_fe2000;

static soc_block_info_t soc_blocks_bcm88020_a0[] = {
    { SOC_BLK_GPORT,    0,    -1,    -1 },
    { SOC_BLK_GPORT,    1,    -1,    -1 },
    { SOC_BLK_XPORT,    0,    -1,    -1 },
    { SOC_BLK_XPORT,    1,    -1,    -1 },
    { SOC_BLK_SPI,      0,    -1,    -1 },
    { SOC_BLK_SPI,      1,    -1,    -1 },
    { SOC_BLK_CMIC,     0,    -1,    -1 },
    {-1,               -1,    -1,    -1 } /* end */
};

static soc_block_info_t soc_blocks_bcm88025_a0[] = {
    { SOC_BLK_GPORT,    0,    -1,    -1 },
    { SOC_BLK_GPORT,    1,    -1,    -1 },
    { SOC_BLK_XPORT,    0,    -1,    -1 },
    { SOC_BLK_XPORT,    1,    -1,    -1 },
    { SOC_BLK_XPORT,    2,    -1,    -1 },
    { SOC_BLK_XPORT,    3,    -1,    -1 },
    { SOC_BLK_SPI,      0,    -1,    -1 },
    { SOC_BLK_SPI,      1,    -1,    -1 },
    { SOC_BLK_CMIC,     0,    -1,    -1 },
    {-1,               -1,    -1,    -1 } /* end */
};


/*
 * Default Port Mapping
 *
 * The following 'soc_port_info_t' tables define the default port
 * mapping of ports (ucode port) to the device physical
 * front-panel ports and corresponding system-side ports.
 *
 * Table information:
 * - The table is index by the port number (port number
 *   used in SDK routines, ucode port number).
 * - The 'blk' field is the index to the device 'soc_block_info'
 *   table (soc_blocks_bcm88020_a0).
 * - The 'bindex' field is the physical port offset within the block.
 *
 * Notes:
 * (a) SPI buses are not front-panel ports, but they
 *     are assigned a port number.  These will contain an invalid
 *     'bindex' and will not have any system-side block/port mapped.
 */

/*
 * Front-panel Default Port Mapping
 *
 * Mapping of port to device physical front-panel port.
 */
static soc_port_info_t soc_ports_bcm88020_a0[] = {
    { 0,    0  },    /* 0  G0.0  */
    { 0,    1  },    /* 1  G0.1  */
    { 0,    2  },    /* 2  G0.2  */
    { 0,    3  },    /* 3  G0.3  */
    { 0,    4  },    /* 4  G0.4  */
    { 0,    5  },    /* 5  G0.5  */
    { 0,    6  },    /* 6  G0.6  */
    { 0,    7  },    /* 7  G0.7  */
    { 0,    8  },    /* 8  G0.8  */
    { 0,    9  },    /* 9  G0.9  */
    { 0,    10 },    /* 10 G0.10 */
    { 0,    11 },    /* 11 G0.11 */
    { 1,    0  },    /* 12 G1.0  */
    { 1,    1  },    /* 13 G1.1  */
    { 1,    2  },    /* 14 G1.2  */
    { 1,    3  },    /* 15 G1.3  */
    { 1,    4  },    /* 16 G1.4  */
    { 1,    5  },    /* 17 G1.5  */
    { 1,    6  },    /* 18 G1.6  */
    { 1,    7  },    /* 19 G1.7  */
    { 1,    8  },    /* 20 G1.8  */
    { 1,    9  },    /* 21 G1.9  */
    { 1,    10 },    /* 22 G1.10 */
    { 1,    11 },    /* 23 G1.11 */
    { 2,    0  },    /* 24 X0.0  */
    { 3,    0  },    /* 25 X1.0  */
    { 4,    -1 },    /* 26 SPI0  */
    { 5,    -1 },    /* 27 SPI1  */
    { -1,    0 },    /* 28 unused */
    { -1,    0 },    /* 29 unused */
    { 4,    13 },    /* 30 SPI LB */
    { 6,    -1 },    /* 31 CPU    */
    { -1,   -1 },    /* end       */
};

static soc_port_info_t soc_ports_bcm88025_a0[] = {
    { 0,    0  },    /* 0  G0.0  */
    { 0,    1  },    /* 1  G0.1  */
    { 0,    2  },    /* 2  G0.2  */
    { 0,    3  },    /* 3  G0.3  */
    { 0,    4  },    /* 4  G0.4  */
    { 0,    5  },    /* 5  G0.5  */
    { 0,    6  },    /* 6  G0.6  */
    { 0,    7  },    /* 7  G0.7  */
    { 0,    8  },    /* 8  G0.8  */
    { 0,    9  },    /* 9  G0.9  */
    { 0,    10 },    /* 10 G0.10 */
    { 0,    11 },    /* 11 G0.11 */
    { 1,    0  },    /* 12 G1.0  */
    { 1,    1  },    /* 13 G1.1  */
    { 1,    2  },    /* 14 G1.2  */
    { 1,    3  },    /* 15 G1.3  */
    { 1,    4  },    /* 16 G1.4  */
    { 1,    5  },    /* 17 G1.5  */
    { 1,    6  },    /* 18 G1.6  */
    { 1,    7  },    /* 19 G1.7  */
    { 1,    8  },    /* 20 G1.8  */
    { 1,    9  },    /* 21 G1.9  */
    { 1,    10 },    /* 22 G1.10 */
    { 1,    11 },    /* 23 G1.11 */
    { 2,    0  },    /* 24 X0.0  */
    { 3,    0  },    /* 25 X1.0  */
    { 6,    -1 },    /* 26 SPI0  */
    { 7,    -1 },    /* 27 SPI1  */
    { -1,    0 },    /* 28 unused */
    { -1,    0 },    /* 29 unused */
    { 6,    13 },    /* 30 SPI LB */
    { 8,    -1 },    /* 31 CPU    */
    { -1,   -1 },    /* end       */
};

/*
 * System-Side Default Port Mapping
 *
 * Mapping of port to device physical system-side port.
 */
static soc_port_info_t soc_system_ports_bcm88020_a0[] = {
    { 4,    0  },    /* 0  G0.0   to SPI0.0 */
    { 4,    1  },    /* 1  G0.1   to SPI0.1 */
    { 4,    2  },    /* 2  G0.2   to SPI0.2 */
    { 4,    3  },    /* 3  G0.3   to SPI0.3 */
    { 4,    4  },    /* 4  G0.4   to SPI0.4 */
    { 4,    5  },    /* 5  G0.5   to SPI0.5 */
    { 4,    6  },    /* 6  G0.6   to SPI0.6 */
    { 4,    7  },    /* 7  G0.7   to SPI0.7 */
    { 4,    8  },    /* 8  G0.8   to SPI0.8 */
    { 4,    9  },    /* 9  G0.9   to SPI0.9 */
    { 4,    10 },    /* 10 G0.10  to SPI0.10 */
    { 4,    11 },    /* 11 G0.11  to SPI0.11 */
    { 5,    0  },    /* 12 G1.0   to SPI1.0 */
    { 5,    1  },    /* 13 G1.1   to SPI1.1 */
    { 5,    2  },    /* 14 G1.2   to SPI1.2 */
    { 5,    3  },    /* 15 G1.3   to SPI1.3 */
    { 5,    4  },    /* 16 G1.4   to SPI1.4 */
    { 5,    5  },    /* 17 G1.5   to SPI1.5 */
    { 5,    6  },    /* 18 G1.6   to SPI1.6 */
    { 5,    7  },    /* 19 G1.7   to SPI1.7 */
    { 5,    8  },    /* 20 G1.8   to SPI1.8 */
    { 5,    9  },    /* 21 G1.9   to SPI1.9 */
    { 5,    10 },    /* 22 G1.10  to SPI1.10 */
    { 5,    11 },    /* 23 G1.11  to SPI1.11 */
    { 4,    12 },    /* 24 X0.0   to SPI0.12 */
    { 5,    12 },    /* 25 X1.0   to SPI1.12 */
    { -1,   0  },    /* 26 SPI0   -n/a-      */
    { -1,   0  },    /* 27 SPI1   -n/a-      */
    { -1,   0  },    /* 28 unused            */
    { -1,   0  },    /* 29 unused            */
    { 4,    13 },    /* 30 SPI-LB to SPI0.13 */
    { 5,    13 },    /* 31 CPU    to SPI1.13 */
    { -1,   -1 },    /* end                  */
};
static soc_port_info_t soc_system_ports_bcm88025_a0[] = {
    { 6,    0  },    /* 0  G0.0   to SPI0.0 */
    { 6,    1  },    /* 1  G0.1   to SPI0.1 */
    { 6,    2  },    /* 2  G0.2   to SPI0.2 */
    { 6,    3  },    /* 3  G0.3   to SPI0.3 */
    { 6,    4  },    /* 4  G0.4   to SPI0.4 */
    { 6,    5  },    /* 5  G0.5   to SPI0.5 */
    { 6,    6  },    /* 6  G0.6   to SPI0.6 */
    { 6,    7  },    /* 7  G0.7   to SPI0.7 */
    { 6,    8  },    /* 8  G0.8   to SPI0.8 */
    { 6,    9  },    /* 9  G0.9   to SPI0.9 */
    { 6,    10 },    /* 10 G0.10  to SPI0.10 */
    { 6,    11 },    /* 11 G0.11  to SPI0.11 */
    { 7,    0  },    /* 12 G1.0   to SPI1.0 */
    { 7,    1  },    /* 13 G1.1   to SPI1.1 */
    { 7,    2  },    /* 14 G1.2   to SPI1.2 */
    { 7,    3  },    /* 15 G1.3   to SPI1.3 */
    { 7,    4  },    /* 16 G1.4   to SPI1.4 */
    { 7,    5  },    /* 17 G1.5   to SPI1.5 */
    { 7,    6  },    /* 18 G1.6   to SPI1.6 */
    { 7,    7  },    /* 19 G1.7   to SPI1.7 */
    { 7,    8  },    /* 20 G1.8   to SPI1.8 */
    { 7,    9  },    /* 21 G1.9   to SPI1.9 */
    { 7,    10 },    /* 22 G1.10  to SPI1.10 */
    { 7,    11 },    /* 23 G1.11  to SPI1.11 */
    { 6,    12 },    /* 24 X0.0   to SPI0.12 */
    { 7,    12 },    /* 25 X1.0   to SPI1.12 */
    { -1,   0  },    /* 26 SPI0   -n/a-      */
    { -1,   0  },    /* 27 SPI1   -n/a-      */
    { -1,   0  },    /* 28 unused            */
    { -1,   0  },    /* 29 unused            */
    { 6,    13 },    /* 30 SPI-LB to SPI0.13 */
    { 7,    13 },    /* 31 CPU    to SPI1.13 */
    { -1,   -1 },    /* end                  */
};

soc_driver_t soc_driver_bcm88020_a0 = {
    /* type         */    SOC_CHIP_BCM88020_A0,
    /* chip_string  */    "fe2000",
    /* origin       */    "Unknown",
    /* pci_vendor   */    BROADCOM_VENDOR_ID,
    /* pci_device   */    BCM88020_DEVICE_ID,
    /* pci_revision */    BCM88020_A2_REV_ID,
    /* num_cos      */    0,
    /* reg_info     */    NULL,
    /* mem_info     */    NULL,
    /* mem_aggr     */    NULL,
    /* block_info   */    soc_blocks_bcm88020_a0,
    /* port_info    */    soc_ports_bcm88020_a0,
    /* counter_maps */    NULL,
    /* features     */    soc_features_bcm88020_a0,
    /* init         */    NULL
}; /* soc_driver */

soc_driver_t soc_driver_bcm88025_a0 = {
    /* type         */    SOC_CHIP_BCM88025_A0,
    /* chip_string  */    "fe2000",
    /* origin       */    "Unknown",
    /* pci_vendor   */    BROADCOM_VENDOR_ID,
    /* pci_device   */    BCM88025_DEVICE_ID,
    /* pci_revision */    BCM88025_A0_REV_ID,
    /* num_cos      */    0,
    /* reg_info     */    NULL,
    /* mem_info     */    NULL,
    /* mem_aggr     */    NULL,
    /* block_info   */    soc_blocks_bcm88025_a0,
    /* port_info    */    soc_ports_bcm88025_a0,
    /* counter_maps */    NULL,
    /* features     */    soc_features_bcm88025_a0,
    /* init         */    NULL
}; /* soc_driver */


static soc_port_info_t *_fe2000_port_info[SOC_MAX_NUM_DEVICES];
static soc_port_info_t *_fe2000_system_port_info[SOC_MAX_NUM_DEVICES];

static soc_cm_isr_func_t isrs[SOC_MAX_NUM_DEVICES][SOC_SBX_FE2000_ISR_TYPE_MAX];
static int _isrs_initialized[SOC_MAX_NUM_DEVICES]={0};

static char *_fe2000_mem_err_names[] = {
    "configuration error",
    "internal RAM0 correctable error",
    "internal RAM0 uncorrectable error",
    "internal RAM1 correctable error",
    "internal RAM1 uncorrectable error",
    "narrow RAM0 correctable error",
    "narrow RAM0 uncorrectable error",
    "narrow RAM1 correctable error",
    "narrow RAM1 uncorrectable error",
    "wide RAM correctable error",
    "wide RAM uncorrectable error",
    "narrow RAM0 cache FIFO loss",
    "narrow RAM1 cache FIFO loss",
    "wide RAM cache FIFO loss",
    NULL,
};

static char *_fe2000_mem_client_names[] = {
    "LRP port 0",
    "LRP port 1",
    "LRP port 2",
    "LRP port 3",
    "LRP port 4",
    "PMU",
    "CMU0",
    "CMU1",
    "",                         /* unused */
    "",                         /* unused */
    "CPU PIO",
    "DMA"
};

/*
 * Mutex Locks required for FE2000 device
 *
 * Current defined locks are:
 *     am - Mutex lock to access AM block register, one per AM block instance
 *     xm - Mutex lock to access XM block register, one per XM block instance
 *
 */
typedef struct _fe2000_mlock_s {
    sal_mutex_t    am[SB_FE2000_NUM_AG_INTERFACES];
    sal_mutex_t    xm[SB_FE2000XT_NUM_XG_INTERFACES];
} _fe2000_mlock_t;

static _fe2000_mlock_t  *_fe2000_mlock[SOC_MAX_NUM_DEVICES];

/*
 * Mutex locks for AM and XM blocks
 *
 * Notes:
 *     Assumes unit and block are valid
 *     Mutex locks are initialized
 */
#define FE2000_AM_LOCK(_unit, _block) \
        sal_mutex_take(_fe2000_mlock[_unit]->am[_block], sal_mutex_FOREVER)
#define FE2000_AM_UNLOCK(_unit, _block) \
        sal_mutex_give(_fe2000_mlock[_unit]->am[_block])

#define FE2000_XM_LOCK(_unit, _block) \
        sal_mutex_take(_fe2000_mlock[_unit]->xm[_block], sal_mutex_FOREVER)
#define FE2000_XM_UNLOCK(_unit, _block) \
        sal_mutex_give(_fe2000_mlock[_unit]->xm[_block])

#define FE2000_LOCK_INIT_CHECK(_unit)  \
        if (_fe2000_mlock[_unit] == NULL) { return SOC_E_INIT; }


/*
 * Function:
 *     _soc_sbx_fe2000_mlock_destroy
 * Purpose:
 *     Deallocate resources used for mutex locks.
 * Parameters:
 *     unit - Device number
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 * Notes:
 *     Assumes valid unit.
 */
STATIC int
_soc_sbx_fe2000_mlock_destroy(int unit)
{
    int  i;

    if (_fe2000_mlock[unit] == NULL) {
        return SOC_E_NONE;
    }

    for (i = 0; i < SB_FE2000_NUM_AG_INTERFACES; i++) {
        if (_fe2000_mlock[unit]->am[i] != NULL) {
            sal_mutex_destroy(_fe2000_mlock[unit]->am[i]);
        }
    }

    for (i = 0; i < SB_FE2000XT_NUM_XG_INTERFACES; i++) {
        if (_fe2000_mlock[unit]->xm[i] != NULL) {
            sal_mutex_destroy(_fe2000_mlock[unit]->xm[i]);
        }
    }

    sal_free(_fe2000_mlock[unit]);
    _fe2000_mlock[unit] = NULL;

    return SOC_E_NONE;
}


/*
 * Function:
 *     _soc_sbx_fe2000_mlock_init
 * Purpose:
 *     Initialize and create mutex lock structure.
 * Parameters:
 *     unit - Device number
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 * Notes:
 *     Assumes valid unit.
 */
STATIC int
_soc_sbx_fe2000_mlock_init(int unit)
{
    int  i;

    if (_fe2000_mlock[unit] == NULL) {
        _fe2000_mlock[unit] = sal_alloc(sizeof(_fe2000_mlock_t),
                                        "soc_reg_mlock_t");
        if (_fe2000_mlock[unit] == NULL) {
            return SOC_E_MEMORY;
        }
        sal_memset(_fe2000_mlock[unit], 0, sizeof(_fe2000_mlock_t));
    }

    for (i = 0; i < SB_FE2000_NUM_AG_INTERFACES; i++) {
        if (_fe2000_mlock[unit]->am[i] == NULL) {
            _fe2000_mlock[unit]->am[i] = sal_mutex_create("soc_reg_mlock");
            if (_fe2000_mlock[unit]->am[i] == NULL) {
                _soc_sbx_fe2000_mlock_destroy(unit);
                return SOC_E_MEMORY;
            }
        }
    }

    for (i = 0; i < SB_FE2000XT_NUM_XG_INTERFACES; i++) {
        if (_fe2000_mlock[unit]->xm[i] == NULL) {
            _fe2000_mlock[unit]->xm[i] = sal_mutex_create("soc_reg_mlock");
            if (_fe2000_mlock[unit]->xm[i] == NULL) {
                _soc_sbx_fe2000_mlock_destroy(unit);
                return SOC_E_MEMORY;
            }
        }
    }

    return SOC_E_NONE;
}


/*
 * Function:
 *     soc_sbx_fe2000_am_read
 * Purpose:
 *     Read data from specified AM register with mutex lock protection.
 * Parameters:
 *     unit         - Device number
 *     block_num    - AM block number, 0 or 1
 *     block_offset - Port within block
 *     reg          - Register to read
 *     data         - (OUT) Data read
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 * Notes:
 *     Assumes argument values are within valid range.
 */
int
soc_sbx_fe2000_am_read(int unit, int32 block_num, int32 block_offset,
                       uint32 reg, uint32 *data)
{
    sbStatus_t  rv;

    FE2000_LOCK_INIT_CHECK(unit);

    FE2000_AM_LOCK(unit, block_num);
    rv = sbFe2000UtilAgmRead(SOC_SBX_SBHANDLE(unit),
                             block_num , block_offset, reg, data);
    FE2000_AM_UNLOCK(unit, block_num);

    if (rv != SB_FE2000_STS_INIT_OK_K) {
        return SOC_E_INTERNAL;
    }

    return SOC_E_NONE;
}


/*
 * Function:
 *     soc_sbx_fe2000_am_write
 * Purpose:
 *     Write data to specified AM register with mutex lock protection.
 * Parameters:
 *     unit         - Device number
 *     block_num    - AM block number, 0 or 1
 *     block_offset - Port within block
 *     reg          - Register to write
 *     data         - Data to write
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 * Notes:
 *     Assumes argument values are within valid range.
 */
int
soc_sbx_fe2000_am_write(int unit, int32 block_num, int32 block_offset,
                        uint32 reg, uint32 data)
{
    sbStatus_t  rv;

    FE2000_LOCK_INIT_CHECK(unit);

    FE2000_AM_LOCK(unit, block_num);
    rv = sbFe2000UtilAgmWrite(SOC_SBX_SBHANDLE(unit),
                              block_num , block_offset, reg, data);
    FE2000_AM_UNLOCK(unit, block_num);

    if (rv != SB_FE2000_STS_INIT_OK_K) {
        return SOC_E_INTERNAL;
    }

    return SOC_E_NONE;
}


/*
 * Function:
 *     soc_sbx_fe2000_xm_read
 * Purpose:
 *     Read data from specified XM register with mutex lock protection.
 * Parameters:
 *     unit         - Device number
 *     block_num    - XM block number, 0 or 1
 *     reg          - Register to read
 *     data_hi      - (OUT) Data read hi-word
 *     data_lo      - (OUT) Data read lo-word
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 * Notes:
 *     Assumes argument values are within valid range.
 */
int
soc_sbx_fe2000_xm_read(int unit, int32 block_num,
                       uint32 reg, uint32 *data_hi, uint32 *data_lo)
{
    sbStatus_t  rv;

    FE2000_LOCK_INIT_CHECK(unit);

    FE2000_XM_LOCK(unit, block_num);
    rv = sbFe2000UtilXgmRead(SOC_SBX_SBHANDLE(unit),
                             block_num, reg, data_hi, data_lo);
    FE2000_XM_UNLOCK(unit, block_num);

    if (rv != SB_FE2000_STS_INIT_OK_K) {
        return SOC_E_INTERNAL;
    }

    return SOC_E_NONE;
}


/*
 * Function:
 *     soc_sbx_fe2000_xm_write
 * Purpose:
 *     Write data to specified XM register with mutex lock protection.
 * Parameters:
 *     unit         - Device number
 *     block_num    - XM block number, 0 or 1
 *     reg          - Register to write
 *     data_hi      - Data to write hi-word
 *     data_lo      - Data to write lo-word
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 * Notes:
 *     Assumes argument values are within valid range.
 */
int
soc_sbx_fe2000_xm_write(int unit, int32 block_num,
                        uint32 reg, uint32 data_hi, uint32 data_lo)
{
    sbStatus_t  rv;

    FE2000_LOCK_INIT_CHECK(unit);

    FE2000_XM_LOCK(unit, block_num);
    rv = sbFe2000UtilXgmWrite(SOC_SBX_SBHANDLE(unit),
                              block_num, reg, data_hi, data_lo);
    FE2000_XM_UNLOCK(unit, block_num);

    if (rv != SB_FE2000_STS_INIT_OK_K) {
        return SOC_E_INTERNAL;
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *     soc_sbx_fe2000_xm_rxtx
 * Purpose:
 *     Write RX/TX_ENABLE on XM.
 * Parameters:
 *     unit         - Device number
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 * Notes:
 *     Assumes argument values are within valid range.
 */
int
soc_sbx_fe2000_xm_rxtx(int unit, int port)
{
    sbStatus_t  rv;

    /* Ten Gigabit Ethernet MACs RX/TX_ENABLE */
    if (SOC_IS_SBX_FE2KXT(unit)) {
      sbFe2000XtInitParams_t *feip;

      feip = &SOC_SBX_CFG_FE2000(unit)->xt_init_params;
      
      rv = sbFe2000XtInitXgmRxTx(SOC_SBX_SBHANDLE(unit), SOC_PORT_BLOCK_NUMBER(unit, port), feip);
    } else {
      sbFe2000InitParams_t *feip;

      feip = &SOC_SBX_CFG_FE2000(unit)->init_params;
       rv = sbFe2000InitXgmRxTx(SOC_SBX_SBHANDLE(unit), SOC_PORT_BLOCK_NUMBER(unit, port), feip);
    }
    if (rv != SB_FE2000_STS_INIT_OK_K) {
      return SOC_E_INTERNAL;
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *     soc_sbx_fe2000_detach
 * Purpose:
 *     Cleanup and free all resources allocated during device specific
 *     initialization routine.
 * Parameters:
 *     unit - Device number
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 * Notes:
 *     Assumes valid unit, soc control.
 */
int
soc_sbx_fe2000_detach(int unit)
{
    soc_sbx_control_t  *sbx;

    
    _isrs_initialized[unit] = 0;

    /* Free mutex locks */
    _soc_sbx_fe2000_mlock_destroy(unit);

    /* Free port info configuration */
    if (_fe2000_port_info[unit]) {
        sal_free(_fe2000_port_info[unit]);
        _fe2000_port_info[unit] = NULL;
    }
    if (_fe2000_system_port_info[unit]) {
        sal_free(_fe2000_system_port_info[unit]);
        _fe2000_system_port_info[unit] = NULL;
    }

    sbx = SOC_SBX_CONTROL(unit);
    if (sbx != NULL) {
        
        /* Teardown ucode table resources, disable automatic hardware features,
         * and clear out memory pointers supplied to hardware.
         */
        switch (sbx->ucodetype) {
        case SOC_SBX_UCODE_TYPE_G2P3:
            soc_sbx_g2p3_ilib_uninit(unit);
            break;
        default:
            ;
        }

        sal_free(sbx->drv);
        sbx->drv = NULL;
    }

    return SOC_E_NONE;
}


int
soc_sbx_fe2000_check_mem(int unit)
{
    char *s;
    int i, j;
    int rv = FALSE;
    uint32_t v;

    for (i = 0; i < 2; i++) {
        if (i) {
            v = SAND_HAL_FE2000_READ(unit, MM1_ERROR);
        } else {
            v = SAND_HAL_FE2000_READ(unit, MM0_ERROR);
        }
        if (v) {
            rv = TRUE;
            for (j = 0; _fe2000_mem_err_names[j]; j++) {
                if (v & (1 << j)) {
                    soc_cm_print("MM%d error: %s\n", i,
                                 _fe2000_mem_err_names[j]);
                }
            }
            if (i) {
                v = SAND_HAL_FE2000_READ(unit, MM1_ERROR_ADDRESS);
            } else {
                v = SAND_HAL_FE2000_READ(unit, MM0_ERROR_ADDRESS);
            }
            s = _fe2000_mem_client_names[SAND_HAL_FE2000_GET_FIELD(unit, MM0_ERROR_ADDRESS,
                                                            CLIENT_ID, v)];
            soc_cm_print("MM%d error client %s, 0x%x: ", i, s,
                         SAND_HAL_FE2000_GET_FIELD(unit, MM0_ERROR_ADDRESS, ADDRESS,
                                            v));
            if (i) {
                soc_cm_print("0x%08x/0x%08x/0x%08x\n",
                             SAND_HAL_FE2000_READ(unit, MM1_ERROR_READ_DATA2),
                             SAND_HAL_FE2000_READ(unit, MM1_ERROR_READ_DATA1),
                             SAND_HAL_FE2000_READ(unit, MM1_ERROR_READ_DATA0));
            } else {
                soc_cm_print("0x%08x/0x%08x/0x%08x\n",
                             SAND_HAL_FE2000_READ(unit, MM0_ERROR_READ_DATA2),
                             SAND_HAL_FE2000_READ(unit, MM0_ERROR_READ_DATA1),
                             SAND_HAL_FE2000_READ(unit, MM0_ERROR_READ_DATA0));
            }
        }
    }
    return rv;
}

void
soc_sbx_fe2000_check_mem_and_die(int unit)
{
    if (soc_sbx_fe2000_check_mem(unit)) {
        *(volatile uint32_t *) NULL;
    }
}

static int
_soc_sbx_fe2000_ped_work_around(int unit)
{
    soc_sbx_control_t *sbx;
    sbFe2000Connection_t *c;
    sbFe2000InitParams_t *feip;
    char *ped_work_around_buffer;
    int i;
    uint32_t v;

    sbx = SOC_SBX_CONTROL(unit);
    feip = &SOC_SBX_CFG_FE2000(unit)->init_params;

    /* The work-around is to push 4 packets to the HPP with
     * PR7 disabled, then bring up the chip again
     */
    sbFe2000InitDefaultParams(feip);

    feip->bSimulation = FALSE;
    feip->lr.bLrpBypass = 1;
    feip->queues.n = 1;
    c = &feip->queues.connections[0];
    c->ulUcodePort = 0;
    c->bEgress = FALSE;
    c->from.ulPortId = 0;
    c->from.ulInterfacePortType = SB_FE2000_IF_PTYPE_PCI;
    c->to = c->from;
    feip->queues.pbsel[0] = -1;

    v = sbFe2000SetQueueParams(unit, feip);
    if (v) {
        soc_cm_print("sbFe2000SetQueueParams() failed with error code:0x%x\n",
                     v);
        return SOC_E_INIT;
    }

    feip->pr.bPrEnable[7] = FALSE;

    sbFe2000Reset((sbhandle) unit);

    i = soc_sbx_txrx_init(unit);
    if (i != SOC_E_NONE) {
        SOC_ERROR_PRINT((DK_ERR, "soc_sbx_fe2000_init: unit %d "
                         "TXRX init failed e=%d\n", unit, i));
        soc_cm_print("failed txrx init\n");
        return i;
    }
    v = sbFe2000Init((sbhandle) unit, feip);
    if (v) {
        soc_cm_print("sbFe2000Init() failed with error code:0x%x\n", v);
        return SOC_E_INIT;
    }
    /* set up queues here */
    v = sbFe2000InitQueues((sbhandle) unit, &feip->queues);
    if (v) {
        soc_cm_print("sbFe2000InitQueues() failed with error code:0x%x\n", v);
        return SOC_E_INIT;
    }

    ped_work_around_buffer = soc_cm_salloc(unit, 64,
                                           "ped work-around buffer");
    if (!ped_work_around_buffer) {
        return SOC_E_MEMORY;
    }

    for (i = 0; i < 4; i++) {
         v = soc_sbx_txrx_sync_tx(unit, NULL, 0, ped_work_around_buffer, 64, 0);
    }

    sal_usleep(500);
    soc_cm_sfree(unit, ped_work_around_buffer);

    v = SAND_HAL_FE2000_READ(unit, PR_PRE7_DROPPED_NO_FREE_PAGE_PKT_CNT);
    if (v < 4) {
        soc_cm_print("PED work-around TX failed (only saw %d packets)\n", v);
        return SOC_E_INIT;
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *     soc_sbx_fe2000_fte_partition
 * Purpose:
 *     compute the FTE segmentation based on user configurable values; store
 *    base addresses in the fe2k soc state struct
 * Parameters:
 *     unit - Device number
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 * Notes:
 *     Assumes valid unit, soc control.
 */
static int
soc_sbx_fe2000_fte_partition(int unit)
{
    uint32 fteBase, maxFtes;
    uint32 numLocalGportFtes, numNodes;
    soc_port_info_t* portInfo;
    int    rv;

    maxFtes = soc_sbx_g2p3_ft_table_size_get(unit);


    /* Cache the vsi end param for other layers to reference */
    SOC_SBX_CFG_FE2000(unit)->vsiEnd = 
        soc_sbx_g2p3_v2e_table_size_get(unit) - 1;

    /* count the number of configured ucode ports */
    SOC_SBX_CFG_FE2000(unit)->numUcodePorts = 0;
    portInfo = SOC_DRIVER(unit)->port_info;

    while (!(portInfo->blk == -1 && portInfo->bindex == -1) ) {
        /* count the port even if it's unused (if blk == -1) because
         * it still counts a used port slot, it's just not a valid port slot
         */
        SOC_SBX_CFG_FE2000(unit)->numUcodePorts++;
        portInfo++;
    }

    if (SOC_SBX_CFG_FE2000(unit)->numUcodePorts > 
        SOC_SBX_CFG(unit)->nMaxFabricPortsOnModule) 
    {
        SOC_ERROR_PRINT((DK_ERR, "ERROR: Line card found to have more "
                         "ports (%d) than specified "
                         "num_max_fabric_ports_on_module (%d).\n",
                         SOC_SBX_CFG_FE2000(unit)->numUcodePorts,
                         SOC_SBX_CFG(unit)->nMaxFabricPortsOnModule));
        return SOC_E_CONFIG;
    }

    numLocalGportFtes = 
        soc_property_get(unit, spn_FTE_NUM_LOCAL_GPORTS, SBX_MAX_GPORTS);

    numNodes = soc_property_get(unit, spn_NUM_MODULES, SBX_MAX_NODES);

    /*    FTE partitions:
     *      +------------------------+
     *      |       PortMesh         |
     *      +------------------------+
     *      |        Drop            |
     *      +------------------------+
     *      |        CPU             |
     *      +------------------------+
     *      |       Trunk            |
     *      +------------------------+
     *      |     HiGig Port         |
     *      +------------------------+
     *      |      Local Gport       |
     *      +------------------------+
     *      |      Global Gport      |
     *      +------------------------+--->+------------------------+
     *      |                        |    |     VLAN Flood (vid)   |
     *      |      VSI Flood         |    +------------------------+
     *      |                        |    |     VLAN Flood (vsi)   |
     *      +------------------------+--->+------------------------+   +----------------+
     *      |    ucast, etc FTE      |                     ----------> + c2 mplstp vpws +
     *      +------------------------+--->+------------------------+   +----------------+
     *      |                        |    |  uknown mc Flood (vid) |
     *      |  unknown MC Flood      |    +------------------------+
     *      |                        |    |  uknown mc Flood (vsi) |
     *      +------------------------+--->+------------------------+   +---------------------+
     *      |  Extra FTEs            |                     ----------> + c2mplstp dynamic ft +
     *      +------------------------+                                 +---------------------+
     *
     *  The VSI and unknown MC Flood FTEs have special meaning.  The first 4K
     * are implied reserved by vlan.c for use per vid.  The remaining are 
     * implied reserved whenever a VSI is allocated.
     */

    fteBase = 0;
    SOC_SBX_CFG_FE2000(unit)->fteMap[SOC_SBX_FSEG_PORT_MESH] = fteBase;
    fteBase += (SOC_SBX_CFG(unit)->nMaxFabricPortsOnModule * numNodes);

    SOC_SBX_CFG_FE2000(unit)->fteMap[SOC_SBX_FSEG_DROP]    = fteBase++;
    SOC_SBX_CFG_FE2000(unit)->fteMap[SOC_SBX_FSEG_CPU]     = fteBase++;
    SOC_SBX_CFG_FE2000(unit)->fteMap[SOC_SBX_FSEG_TRUNK]   = fteBase;
    fteBase += SBX_MAX_TRUNKS;

    SOC_SBX_CFG_FE2000(unit)->fteMap[SOC_SBX_FSEG_HG_PORT] = fteBase;

    SOC_SBX_CFG_FE2000(unit)->fteMap[SOC_SBX_FSEG_LGPORT]  = fteBase;
    fteBase +=  numLocalGportFtes;

    SOC_SBX_CFG_FE2000(unit)->fteMap[SOC_SBX_FSEG_GGPORT]  = fteBase;

    /* VSI FTEs are defined by a ucode pushdown variable, the first
     * 4K are for VIDs the remaining are for dynamic VSIs
     */
    rv = soc_sbx_g2p3_vlan_ft_base_get(unit, &fteBase);
    if (SOC_FAILURE(rv)) {
        SOC_ERROR_PRINT((DK_ERR, "Failed to get vlan fte base: %d %s\n",
                         rv, soc_errmsg(rv)));
        return rv;
    }
    SOC_SBX_CFG_FE2000(unit)->fteMap[SOC_SBX_FSEG_VID_VSI] = fteBase;    

    fteBase += SBX_MAX_VID;
    SOC_SBX_CFG_FE2000(unit)->fteMap[SOC_SBX_FSEG_DYN_VSI] = fteBase;

    fteBase = (SOC_SBX_CFG_FE2000(unit)->fteMap[SOC_SBX_FSEG_VID_VSI] +
               SOC_SBX_CFG_FE2000(unit)->vsiEnd + 1);

    /* If C2 mplstp support, enable vpws uni pool */
    if(SOC_SBX_CFG(unit)->mplstp_ena) {
        SOC_SBX_CFG_FE2000(unit)->fteMap[SOC_SBX_FSEG_VPWS_UNI] = fteBase;
        /* for c1 this area must be split between both as required for c2 use
         * space below umc for dynamic ft */
        SOC_SBX_CFG_FE2000(unit)->fteMap[SOC_SBX_FSEG_DYNAMIC] = 0;
    } else {
        /* Dynamic FTEs - unpriveldege FTEs */
        SOC_SBX_CFG_FE2000(unit)->fteMap[SOC_SBX_FSEG_DYNAMIC] = fteBase;
        /* dummy space nothing allocate on non-mplstp */
        SOC_SBX_CFG_FE2000(unit)->fteMap[SOC_SBX_FSEG_VPWS_UNI] = fteBase;
    }

    /* Unknown MC flood FTEs are tied to the VSI FTEs.  That is, when a
     * VSI FTE is allocated, the Unknown MC flood FTE is implied.
     */
    rv = soc_sbx_g2p3_mc_ft_offset_get(unit, &fteBase);
    if (SOC_FAILURE(rv)) {
        SOC_ERROR_PRINT((DK_ERR, "Failed to get mc fte base: %d %s\n",
                         rv, soc_errmsg(rv)));
        return rv;
    }

    if (fteBase) {
        /* Unknow MC FTEs start base + vid_flood base; just after the 
         * Dynamic FTEs
         */
        fteBase += SOC_SBX_CFG_FE2000(unit)->fteMap[SOC_SBX_FSEG_VID_VSI];
    } else {
        /* Unknown MC FTE feature not enabled, allow Dyamic to consume
         * all FTEs
         */
        fteBase = maxFtes;
    }
    SOC_SBX_CFG_FE2000(unit)->fteMap[SOC_SBX_FSEG_UMC] = fteBase;


    if (fteBase < maxFtes) {
        fteBase += SOC_SBX_CFG_FE2000(unit)->vsiEnd + 1;
        
        if(SOC_SBX_CFG(unit)->mplstp_ena) {
            /* Dynamic FTEs - unpriveldege FTEs */
            SOC_SBX_CFG_FE2000(unit)->fteMap[SOC_SBX_FSEG_DYNAMIC] = fteBase;            
            SOC_SBX_CFG_FE2000(unit)->fteMap[SOC_SBX_FSEG_EXTRA]   = 0;
        } else {
            SOC_SBX_CFG_FE2000(unit)->fteMap[SOC_SBX_FSEG_EXTRA] = fteBase;
        }
            
    } else {
        SOC_SBX_CFG_FE2000(unit)->fteMap[SOC_SBX_FSEG_EXTRA] = 0;
    }

    SOC_SBX_CFG_FE2000(unit)->fteMap[SOC_SBX_FSEG_END] = maxFtes;

    return rv;
}


int
soc_sbx_fe2000_init(int unit, soc_sbx_config_t *cfg)
{
    soc_sbx_control_t *sbx;
    pbmp_t pbmp;
    int rv;
    soc_port_t port;
    int frontiface, frontport, systemiface, systemport, egress,
        singleport, v;
    sbFe2000Connection_t *c;
    sbFe2000InitParams_t *feip;
    int ped_work_around = 1;
    uint32_t uData, uRevision;
    int muxfront, muxsys;
    int i;
    SOC_SBX_WARM_BOOT_DECLARE(int wb);

    systemiface = 0;
    systemport = 0;

/* FIX: please leave this in the code until SAL_BOOT_BCMSIM is working */
#ifdef PLISIM
    ped_work_around = 0;
#endif

    sbx = SOC_SBX_CONTROL(unit);

    if (sbx == NULL) {
        return SOC_E_INIT;
    }

    if (SOC_WARM_BOOT(unit)) {
        SOC_SBX_WARM_BOOT_IGNORE(unit, wb);
        sbFe2000CmuWbRecover(unit);
        soc_sbx_txrx_uninit_hw_only(unit);
        SOC_SBX_WARM_BOOT_OBSERVE(unit, wb);
    }

    /* initialize with illegal values */
    sal_memset(sbx->modport, 0xFF, sizeof(sbx->modport));

    if (SOC_CONTROL(unit)->soc_flags & (SOC_F_INITED)) {
        if (SOC_IS_SBX_G2P3(unit)) {
            SOC_IF_ERROR_RETURN(soc_sbx_g2p3_uninit(unit));
            SOC_CONTROL(unit)->soc_flags &= ~SOC_F_INITED; 
        } else {
            return SOC_E_UNAVAIL;
        }
    }

    feip = &SOC_SBX_CFG_FE2000(unit)->init_params;

    sal_memset(&isrs[unit], 0, 
               sizeof(isrs[0][0]) * SOC_SBX_FE2000_ISR_TYPE_MAX);

    SOC_IRQ_MASK(unit) = 0x00001fff;

    /*    coverity[new_values]    */
    if (SOC_WARM_BOOT(unit) == FALSE) {
        if (ped_work_around && !SAL_BOOT_BCMSIM) {
            v = _soc_sbx_fe2000_ped_work_around(unit);
            if (v != SOC_E_NONE) {
                return v;
            }
        }
    }

    sbFe2000InitDefaultParams(feip);

    feip->resetDll = sbx->sbx_functions.sram_init;
    feip->trainDdr = sbx->sbx_functions.ddr_train;
    feip->queues.n = 0;

    uData = SAND_HAL_FE2000_READ(unit, PC_REVISION);
    uRevision = SAND_HAL_FE2000_GET_FIELD(unit, PC_REVISION, REVISION, uData);
    if(uRevision <= 1) {
      feip->pr.PipelinePriority = SB_FE2000_PR_PIPELINE_HIGHEST_PRIORITY;
    }

    SOC_PBMP_ASSIGN(pbmp, PBMP_PORT_ALL(unit));
    SOC_PBMP_OR(pbmp, PBMP_CMIC(unit));
    SOC_PBMP_ITER(pbmp, port) {
        singleport = SOC_SBX_SYSTEM_PORT_BLOCK(unit, port) < 0;
        frontport = SOC_PORT_BLOCK_INDEX(unit, port);
        if (frontport < 0) {
            frontport = 0;
        }
        frontiface =
            sbFe2000TranslateBlock(SOC_PORT_BLOCK_TYPE(unit, port),
                                    SOC_PORT_BLOCK_NUMBER(unit, port));
        if (!singleport) {
            systemport = SOC_SBX_SYSTEM_PORT_BLOCK_INDEX(unit, port);
            if (systemport < 0) {
                systemport = 0;
            }
            systemiface =
                sbFe2000TranslateBlock(
                              SOC_SBX_SYSTEM_PORT_BLOCK_TYPE(unit, port),
                              SOC_SBX_SYSTEM_PORT_BLOCK_NUMBER(unit, port));
            if (frontiface == systemiface && frontport == systemport) {
                singleport = TRUE;
            }
        }

        if (singleport) {
            systemport = frontport;
            systemiface = frontiface;
            egress = soc_property_port_get(unit, port, "spn_8802x_EGRESS_ONLY", 0);
            /* remove from port bit map */
            SOC_PBMP_PORT_REMOVE(SOC_INFO(unit).port.bitmap, port);
        } else {
            egress = FALSE;
        }

        muxfront = 0;
        muxsys = 0;
        for (i = 0; i < feip->queues.n; i++) {
            c = &feip->queues.connections[i];
            if (!c->bEgress
                && c->from.ulPortId == frontport
                && c->from.ulInterfacePortType == frontiface) {
                muxfront = 1;
            }
            if (c->bEgress
                && c->to.ulPortId == systemport
                && c->to.ulInterfacePortType == systemiface) {
                muxsys = 1;
            }
        }

        c = &feip->queues.connections[feip->queues.n];
        feip->queues.pbsel[feip->queues.n]
            = soc_property_port_get(unit, port, spn_PORT_PB_SELECT, -1);
        feip->queues.n++;
        if (!muxfront) {
            c->ulUcodePort = port;
            c->bEgress = egress;
            c->from.ulPortId = frontport;
            c->from.ulInterfacePortType = frontiface;
            c->to.ulPortId = systemport;
            c->to.ulInterfacePortType = systemiface;
        }
        if (!singleport && !muxsys) {
            c = &feip->queues.connections[feip->queues.n];
            feip->queues.pbsel[feip->queues.n] = -1;
            feip->queues.n++;
            c->ulUcodePort = port;
            c->bEgress = TRUE;
            c->from.ulPortId = systemport;
            c->from.ulInterfacePortType = systemiface;
            c->to.ulPortId = frontport;
            c->to.ulInterfacePortType = frontiface;
        }
    }

    SOC_SBX_CONTROL(unit)->ucodetype = soc_sbx_configured_ucode_get(unit);
    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
    case SOC_SBX_UCODE_TYPE_G2P3:
        SOC_IF_ERROR_RETURN(soc_sbx_g2p3_drv_hw_init(unit));
        break;
#endif
    default:
        return SOC_E_CONFIG;
    }

    if (!soc_property_get(unit, spn_LRP_BYPASS, 1)) {
        feip->lr.bLrpBypass = TRUE;
        feip->lr.bBringUp = TRUE;
    } else {
        feip->lr.bBringUp = FALSE;
    }

/* FIX: please leave this in the code until SAL_BOOT_BCMSIM is working */
#ifdef PLISIM
    feip->bSimulation = TRUE;
#else
    feip->bSimulation = SAL_BOOT_BCMSIM;
#endif

    feip->mm[0].ramconfiguration.WidePortRamConfiguration =
        SOC_SBX_CFG_FE2000(unit)->mm0wx18
        ? SB_FE2000_MM_TWO_BY_18_RAMS_CONNECTED
        : SB_FE2000_MM_ONE_BY_36_RAM_CONNECTED;
    feip->mm[1].ramconfiguration.WidePortRamConfiguration =
        SOC_SBX_CFG_FE2000(unit)->mm1wx18
        ? SB_FE2000_MM_TWO_BY_18_RAMS_CONNECTED
        : SB_FE2000_MM_ONE_BY_36_RAM_CONNECTED;

    feip->mm[0].ramconfiguration.NarrowPortRam0Words
        = 1 << SOC_SBX_CFG_FE2000(unit)->mm0n0awidth;
    feip->mm[0].ramconfiguration.NarrowPortRam1Words
        = 1 << SOC_SBX_CFG_FE2000(unit)->mm0n1awidth;
    feip->mm[0].ramconfiguration.WidePortRamWords
        = 1 << SOC_SBX_CFG_FE2000(unit)->mm0wawidth;
    feip->mm[1].ramconfiguration.NarrowPortRam0Words
        = 1 << SOC_SBX_CFG_FE2000(unit)->mm1n0awidth;
    feip->mm[1].ramconfiguration.NarrowPortRam1Words
        = 1 << SOC_SBX_CFG_FE2000(unit)->mm1n1awidth;
    feip->mm[1].ramconfiguration.WidePortRamWords
        = 1 << SOC_SBX_CFG_FE2000(unit)->mm1wawidth;

    if (soc_property_get(unit, spn_SPI_LOOPBACK, 0)) {
        feip->st[0].bLoopback = 1;
        feip->sr[0].bLoopback = 1;
        feip->st[1].bLoopback = 1;
        feip->sr[1].bLoopback = 1;
    }

    /* bring down the chip (& disable the CII) */
    sbFe2000Reset((sbhandle) unit);

    /* A warmboot needs to to completely re-init txrx, but
     * the ped workaround is not needed - it was done during the cold boot
     */
    if (ped_work_around && (SOC_WARM_BOOT(unit) == FALSE)) {
      v = soc_sbx_txrx_init_hw_only(unit);
    } else {
      v = soc_sbx_txrx_init(unit);
    }

    if (v != SOC_E_NONE) {
        SOC_ERROR_PRINT((DK_ERR, "soc_sbx_fe2000_init: unit %d "
                         "TXRX init failed e=%d\n", unit, v));
        soc_cm_print("failed txrx init\n");
        return v;
    }

    /* hw chip bringup & enable the CII */
    rv = sbFe2000Init(sbx->sbhdl, feip);

    if (rv) {
        soc_cm_print("sbFe2000Init() failed with error code:0x%x\n", rv);
        return SOC_E_INIT;
    }

    SOC_IF_ERROR_RETURN(soc_sbx_g2p3_drv_init(unit));

    sal_memcpy(sbx->uver_name, ((fe2kAsm2IntD *) sbx->ucode)->m_uver_name,
               strlen(((fe2kAsm2IntD *) sbx->ucode)->m_uver_name));
    sbx->uver_maj = ((fe2kAsm2IntD *) sbx->ucode)->m_uver_maj;
    sbx->uver_min = ((fe2kAsm2IntD *) sbx->ucode)->m_uver_min;
    sbx->uver_patch = ((fe2kAsm2IntD *) sbx->ucode)->m_uver_patch;

    if (!soc_sbx_ucode_versions[sbx->ucodetype]) {
        soc_sbx_ucode_versions[sbx->ucodetype] =
            sal_alloc(64, "ucode ver");
        if (!soc_sbx_ucode_versions[sbx->ucodetype]) {
            return SOC_E_MEMORY;
        }
    }
    sal_sprintf(soc_sbx_ucode_versions[sbx->ucodetype], "%s %d.%d.%d",
                sbx->uver_name, sbx->uver_maj,
                sbx->uver_min, sbx->uver_patch);

    /* set up queues here */
    rv = sbFe2000InitQueues(sbx->sbhdl, &feip->queues);
    if (rv) {
        soc_cm_print("sbFe2000InitQueues() failed with error code:0x%x\n", rv);
        return SOC_E_INIT;
    }

/* FIX: this crashes bcmsim */
#ifndef PLISIM
    if (SOC_WARM_BOOT(unit) == FALSE) {
        rv = sbFe2000InitClearMems(sbx->sbhdl, feip);
        if (rv) {
            soc_cm_print("sbFe2000InitClearMems() failed with error code:0x%x\n", rv);
            return SOC_E_INIT;
        }
    }
#endif

    soc_sbx_fe2000_register_isr(unit, SOC_SBX_FE2000_ISR_TYPE_CMU,
                                soc_sbx_fe2000_isr);

    SOC_IRQ_MASK(unit) &=
        ~SAND_HAL_FE2000_FIELD_MASK(unit, PC_INTERRUPT_MASK, PCI_COMPLETION_DISINT);

    SOC_SBX_WARM_BOOT_IGNORE(unit, wb);
    SAND_HAL_FE2000_WRITE(unit, PC_INTERRUPT_MASK, SOC_IRQ_MASK(unit));
    SOC_SBX_WARM_BOOT_OBSERVE(unit, wb);

    /* Initialize SOC link control module */
    soc_linkctrl_init(unit, &soc_linkctrl_driver_fe2000);

    /* Initialize mutex locks for register access (am and xm blocks) */
    rv = _soc_sbx_fe2000_mlock_init(unit);
    if (SOC_FAILURE(rv)) {
        SOC_ERROR_PRINT((DK_ERR, "soc_sbx_fe2000_init: unit=%d "
                         "mlock init failed rv=%d\n", unit, rv));
        return rv;
    }

    _isrs_initialized[unit] = 1;

    if (sbx->ucodetype == SOC_SBX_UCODE_TYPE_G2P3) {
        rv = soc_sbx_g2p3_ilib_init(unit);
        if (rv) {
            soc_cm_print("g2p3_ilib init failed: %d\n", rv);
            return rv;
        }
    }

    
    rv = soc_sbx_fe2000_fte_partition(unit);
    if (SOC_FAILURE(rv)) {
        soc_cm_print("Failed to partition FTE space: %d %s\n",
                     rv, soc_errmsg(rv));
        return rv;
    }

    /* run the ucode recovery sequence after the complete init.  This
     * allows any computations to reach the same initial result based on the
     * ucode package defaults.  Recovering to final values during, or before
     * init will compute incorrect values (particularly in the ucode 
     * table manager).
     */
    if (SOC_WARM_BOOT(unit)) {
        SOC_IF_ERROR_RETURN
            (fe2kAsm2IndD__recoverNcv(SOC_SBX_CONTROL(unit)->ucode));
        SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ilib_recover(unit));
    }

    return rv;
}

int
soc_sbx_fe2000xt_init(int unit, soc_sbx_config_t *cfg)
{
    soc_sbx_control_t *sbx;
    pbmp_t pbmp;
    int rv;
    int port, frontiface, frontport, systemiface, systemport, egress,
        singleport, v;
    sbFe2000Connection_t *c;
    sbFe2000XtInitParams_t *feip;
    uint32_t uData, uRevision;
    int muxfront, muxsys;
    int i;
    char *ucodestr = NULL;
    char *sPropName = NULL, *sTuningParams = NULL;
    SOC_SBX_WARM_BOOT_DECLARE(int wb);

    systemiface = 0;
    systemport = 0;

    sbx = SOC_SBX_CONTROL(unit);

    if (sbx == NULL) {
        return SOC_E_INIT;
    }

    if (SOC_WARM_BOOT(unit)) {
        SOC_SBX_WARM_BOOT_IGNORE(unit, wb);
        sbFe2000CmuWbRecover(unit);
        soc_sbx_txrx_uninit_hw_only(unit);
        SOC_SBX_WARM_BOOT_OBSERVE(unit, wb);
    }

    /* initialize with illegal values */
    sal_memset(sbx->modport, 0xFF, sizeof(sbx->modport));

    feip = &SOC_SBX_CFG_FE2000(unit)->xt_init_params;
    sal_memset(feip, 0 ,sizeof(*feip));
    feip->uHppFreq = soc_property_get(unit, spn_BCM88025_HPP_FREQ, SB_FE2000XT_HPP_FREQ_375000);
    if (feip->uHppFreq < 1000) {
        feip->uHppFreq = feip->uHppFreq * 1000;
    }
    feip->uSwsFreq = soc_property_get(unit, spn_BCM88025_SWS_FREQ, SB_FE2000XT_SWS_FREQ_340000);
    if (feip->uSwsFreq < 1000) {
        feip->uSwsFreq = feip->uSwsFreq * 1000;
    }

    sal_memset(&isrs[unit], 0, 
               sizeof(isrs[0][0]) * SOC_SBX_FE2000_ISR_TYPE_MAX);

    SOC_IRQ_MASK(unit) = 0x00001fff;

    sbFe2000XtInitDefaultParams(unit, feip);

    feip->ddrConfigRw = sbx->sbx_functions.fe2kxt_ddrconfig_rw;
    feip->queues.n = 0;

    uData = SAND_HAL_FE2000_READ(unit, PC_REVISION);
    uRevision = SAND_HAL_FE2000_GET_FIELD(unit, PC_REVISION, REVISION, uData);
    if(uRevision <= 1) {
      feip->pr.PipelinePriority = SB_FE2000_PR_PIPELINE_HIGHEST_PRIORITY;
    }

    SOC_PBMP_ASSIGN(pbmp, PBMP_PORT_ALL(unit));
    SOC_PBMP_OR(pbmp, PBMP_CMIC(unit));
    SOC_PBMP_ITER(pbmp, port) {

        singleport = SOC_SBX_SYSTEM_PORT_BLOCK(unit, port) < 0;
        frontport = SOC_PORT_BLOCK_INDEX(unit, port);
        if ( singleport && (frontport < 0) ){
            continue;
        }
        if (frontport < 0) {
            frontport = 0;
        }
        frontiface =
            sbFe2000TranslateBlock(SOC_PORT_BLOCK_TYPE(unit, port),
                                    SOC_PORT_BLOCK_NUMBER(unit, port));
        if (!singleport) {
            systemport = SOC_SBX_SYSTEM_PORT_BLOCK_INDEX(unit, port);
            if (systemport < 0) {
                systemport = 0;
            }
            systemiface =
                sbFe2000TranslateBlock(
                              SOC_SBX_SYSTEM_PORT_BLOCK_TYPE(unit, port),
                              SOC_SBX_SYSTEM_PORT_BLOCK_NUMBER(unit, port));
            if (frontiface == systemiface && frontport == systemport) {
                singleport = TRUE;
            }
        }

        if (singleport) {
            systemport = frontport;
            systemiface = frontiface;
            egress = soc_property_port_get(unit, port, "spn_8802x_EGRESS_ONLY", 0);
            /* remove from port bit map */
            SOC_PBMP_PORT_REMOVE(SOC_INFO(unit).port.bitmap, port);
        } else {
            egress = FALSE;
        }

        muxfront = 0;
        muxsys = 0;
        for (i = 0; i < feip->queues.n; i++) {
            c = &feip->queues.connections[i];
            if (!c->bEgress
                && c->from.ulPortId == frontport
                && c->from.ulInterfacePortType == frontiface) {
                muxfront = 1;
            }
            if (c->bEgress
                && c->to.ulPortId == systemport
                && c->to.ulInterfacePortType == systemiface) {
                muxsys = 1;
            }
        }

        c = &feip->queues.connections[feip->queues.n];
        feip->queues.pbsel[feip->queues.n]
            = soc_property_port_get(unit, port, spn_PORT_PB_SELECT, -1);
        feip->queues.n++;
        if (!muxfront) {
            c->ulUcodePort = port;
            c->bEgress = egress;
            c->from.ulPortId = frontport;
            c->from.ulInterfacePortType = frontiface;
            c->to.ulPortId = systemport;
            c->to.ulInterfacePortType = systemiface;
        }
        if (!singleport && !muxsys) {
            c = &feip->queues.connections[feip->queues.n];
            feip->queues.pbsel[feip->queues.n] = -1;
            feip->queues.n++;
            c->ulUcodePort = port;
            c->bEgress = TRUE;
            c->from.ulPortId = systemport;
            c->from.ulInterfacePortType = systemiface;
            c->to.ulPortId = frontport;
            c->to.ulInterfacePortType = frontiface;
        }


    }

    ucodestr = soc_property_get_str(unit, spn_BCM88025_UCODE);
    
    if ( ucodestr && (sal_strcmp(ucodestr, "c2_g2xx") == 0)) {
#ifdef BCM_FE2000_G2XX_SUPPORT
        SOC_IF_ERROR_RETURN(soc_sbx_g2xx_drv_hw_init(unit));
#endif
    }else{
#ifdef BCM_FE2000_P3_SUPPORT
        SOC_IF_ERROR_RETURN(soc_sbx_g2p3_drv_hw_init(unit));
#endif
    }

    if (!soc_property_get(unit, spn_LRP_BYPASS, 1)) {
        feip->lr.bLrpBypass = TRUE;
        feip->lr.bBringUp = TRUE;
    } else {
        feip->lr.bBringUp = FALSE;
    }

/* FIX: please leave this in the code until SAL_BOOT_BCMSIM is working */
#ifdef PLISIM
    feip->bSimulation = TRUE;
#else
    feip->bSimulation = SAL_BOOT_BCMSIM;
#endif
    feip->mm[0].ramconfiguration.WidePortRamConfiguration =
        SOC_SBX_CFG_FE2000(unit)->mm0wx18
        ? SB_FE2000_MM_TWO_BY_18_RAMS_CONNECTED
        : SB_FE2000_MM_ONE_BY_36_RAM_CONNECTED;
    feip->mm[1].ramconfiguration.WidePortRamConfiguration =
        SOC_SBX_CFG_FE2000(unit)->mm1wx18
        ? SB_FE2000_MM_TWO_BY_18_RAMS_CONNECTED
        : SB_FE2000_MM_ONE_BY_36_RAM_CONNECTED;

    feip->mm[0].ramconfiguration.NarrowPortRam0Words
        = 1 << SOC_SBX_CFG_FE2000(unit)->mm0n0awidth;
    feip->mm[0].ramconfiguration.NarrowPortRam1Words
        = 1 << SOC_SBX_CFG_FE2000(unit)->mm0n1awidth;
    feip->mm[0].ramconfiguration.WidePortRamWords
        = 1 << SOC_SBX_CFG_FE2000(unit)->mm0wawidth;
    feip->mm[1].ramconfiguration.NarrowPortRam0Words
        = 1 << SOC_SBX_CFG_FE2000(unit)->mm1n0awidth;
    feip->mm[1].ramconfiguration.NarrowPortRam1Words
        = 1 << SOC_SBX_CFG_FE2000(unit)->mm1n1awidth;
    feip->mm[1].ramconfiguration.WidePortRamWords
        = 1 << SOC_SBX_CFG_FE2000(unit)->mm1wawidth;

    if (soc_property_get(unit, spn_SPI_LOOPBACK, 0)) {
        feip->st[0].bLoopback = 1;
        feip->sr[0].bLoopback = 1;
        feip->st[1].bLoopback = 1;
        feip->sr[1].bLoopback = 1;
    }

    /* bring down the chip (& disable the CII) */
    sbFe2000Reset((sbhandle) unit);

    v = soc_sbx_txrx_init(unit);

    if (v != SOC_E_NONE) {
        SOC_ERROR_PRINT((DK_ERR, "soc_sbx_fe2000_init: unit %d "
                         "TXRX init failed e=%d\n", unit, v));
        soc_cm_print("failed txrx init\n");
        return v;
    }

    /* Configure Memory Parameters */
    feip->uDdr2MemorySelect = soc_property_get(unit, spn_BCM88025_DDR_TYPE, SB_FE2000XT_DDR2_PLUS_25);

    feip->uHppFreq = soc_property_get(unit, spn_BCM88025_HPP_FREQ, SB_FE2000XT_HPP_FREQ_375000);
    if (feip->uHppFreq < 1000) {
        feip->uHppFreq = feip->uHppFreq * 1000;
    }
    feip->uSwsFreq = soc_property_get(unit, spn_BCM88025_SWS_FREQ, SB_FE2000XT_SWS_FREQ_340000);
    if (feip->uSwsFreq < 1000) {
        feip->uSwsFreq = feip->uSwsFreq * 1000;
    }
    feip->uSpiFreq = soc_property_get(unit, spn_BCM88025_SPI_FREQ, SB_FE2000XT_SPI_FREQ_500000);
    if (feip->uSpiFreq < 1000) {
        feip->uSpiFreq = feip->uSpiFreq * 1000;
    }

#ifdef BROADCOM_SVK
    feip->bDVTboard = soc_property_get(unit,"bcm88025_dv_board",FALSE);
#endif

    /* soc_properties override NV settings */
    sPropName = "bcm88025_mmu1_narrow0_config";
    sTuningParams = soc_property_get_str(unit, sPropName);
    if (!sTuningParams) {
        if (feip->ddrConfigRw) {
            soc_cm_print("Loading DDR Tuning Parameters from NV\n");
            rv = feip->ddrConfigRw(unit, 0, feip->ddrconfig);
            if (rv) {
                /* error no valid tuning paramters */
                soc_cm_print("ERROR:No DDR tuning parameters found.  Run exttsbx and ddrconfig\n");
                /* keep going for soc init completes and exttsbx can be run */
            }
        }
    }

    /* hw chip bringup & enable the CII */
    rv = sbFe2000XtInit(sbx->sbhdl, feip);
    if (rv) {
        soc_cm_print("sbFe2000XtInit() failed with error code:0x%x\n", rv);
        return SOC_E_INIT;
    }

    if ( ucodestr && (sal_strcmp(ucodestr, "c2_g2xx") == 0) ) {
#ifdef BCM_FE2000_G2XX_SUPPORT
            SOC_IF_ERROR_RETURN(soc_sbx_g2xx_drv_init(unit));
#else
            return(SOC_E_INIT);
#endif
    }else{
#ifdef BCM_FE2000_P3_SUPPORT
            SOC_IF_ERROR_RETURN(soc_sbx_g2p3_drv_init(unit));
#endif
    }

    sal_memcpy(sbx->uver_name, ((fe2kAsm2IntD *) sbx->ucode)->m_uver_name,
               strlen(((fe2kAsm2IntD *) sbx->ucode)->m_uver_name));
    sbx->uver_maj = ((fe2kAsm2IntD *) sbx->ucode)->m_uver_maj;
    sbx->uver_min = ((fe2kAsm2IntD *) sbx->ucode)->m_uver_min;
    sbx->uver_patch = ((fe2kAsm2IntD *) sbx->ucode)->m_uver_patch;

    if (!soc_sbx_ucode_versions[sbx->ucodetype]) {
        soc_sbx_ucode_versions[sbx->ucodetype] =
            sal_alloc(64, "ucode ver");
        if (!soc_sbx_ucode_versions[sbx->ucodetype]) {
            return SOC_E_MEMORY;
        }
    }
    sal_sprintf(soc_sbx_ucode_versions[sbx->ucodetype], "%s %d.%d.%d",
                sbx->uver_name, sbx->uver_maj,
                sbx->uver_min, sbx->uver_patch);


#ifdef BROADCOM_SVK
/* FIX: this crashes bcmsim */
    if (feip->bDVTboard) {
    } else 
#endif
    {
#ifndef PLISIM
        if (SOC_WARM_BOOT(unit) == FALSE) {
            rv = sbFe2000XtInitClearMems(sbx->sbhdl, feip);
            if (rv) {
                soc_cm_print("sbFe2000InitClearMems() failed with error code:0x%x\n", rv);
                return SOC_E_INIT;
            }
        }
#endif
    }

    SOC_IRQ_MASK(unit) &=
        ~SAND_HAL_FE2000_FIELD_MASK(unit, PC_INTERRUPT_MASK, PCI_COMPLETION_DISINT);

    SOC_SBX_WARM_BOOT_IGNORE(unit, wb);
    SAND_HAL_FE2000_WRITE(unit, PC_INTERRUPT_MASK, SOC_IRQ_MASK(unit));
    SOC_SBX_WARM_BOOT_OBSERVE(unit, wb);

    /* Initialize SOC link control module */
    soc_linkctrl_init(unit, &soc_linkctrl_driver_fe2000);

    /* Initialize mutex locks for register access (am and xm blocks) */
    rv = _soc_sbx_fe2000_mlock_init(unit);
    if (SOC_FAILURE(rv)) {
        SOC_ERROR_PRINT((DK_ERR, "soc_sbx_fe2000_init: unit=%d "
                         "mlock init failed rv=%d\n", unit, rv));
        return rv;
    }

    _isrs_initialized[unit] = 1;


    if (SOC_WARM_BOOT(unit) == FALSE) {

        /* clear out ecc errors before queues are setup because the ecc
         * clear will configure queues to flush the first ecc errors on
         * the spis
         */
        rv = sbFe2000XtEccClear(unit);
        if (rv) {
            soc_cm_print("ecc clear failed: %d %s\n", rv, soc_errmsg(rv));
            return SOC_E_INIT;
        }

        /* set up queues here - AFTER ecc clear */
        rv = sbFe2000XtInitQueues(sbx->sbhdl, &feip->queues);
        if (rv) {
            soc_cm_print("sbFe2000InitQueues() failed with error code:0x%x\n", rv);
            return SOC_E_INIT;
        }
    }

    soc_sbx_fe2000_register_isr(unit, SOC_SBX_FE2000_ISR_TYPE_CMU,
                                soc_sbx_fe2000_isr);

    if (sbx->ucodetype == SOC_SBX_UCODE_TYPE_G2P3) {
        rv = soc_sbx_g2p3_ilib_init(unit);
        if (rv) {
            soc_cm_print("g2p3_ilib init failed: %d\n", rv);
            return rv;
        }
        rv = soc_sbx_fe2000_fte_partition(unit);
        if (SOC_FAILURE(rv)) {
            soc_cm_print("Failed to partition FTE space: %d %s\n",
                         rv, soc_errmsg(rv));
            return rv;
        }
    }else if (sbx->ucodetype == SOC_SBX_UCODE_TYPE_G2XX) {
#ifdef BCM_FE2000_G2XX_SUPPORT
        rv = soc_sbx_g2xx_ilib_init(unit);
        if (rv) {
            soc_cm_print("xxx_ilib init failed: %d\n", rv);
            return rv;
        }
#endif
    }

    /* enable packet receive on active interfaces */
    rv = sbFe2000XtEnablePrPorts(sbx->sbhdl, feip);
    if (rv) {
        soc_cm_print("sbFe2000XtEnablePr() failed with error code:0x%x\n", rv);
        return SOC_E_INIT;
    }

    /* run the ucode recovery sequence after the complete init.  This
     * allows any computations to reach the same initial result based on the
     * ucode package defaults.  Recovering to final values during, or before
     * init will compute incorrect values (particularly in the ucode 
     * table manager).
     */
    if (SOC_WARM_BOOT(unit)) {
        SOC_IF_ERROR_RETURN
            (fe2kAsm2IndD__recoverNcv(SOC_SBX_CONTROL(unit)->ucode));
        SOC_IF_ERROR_RETURN(soc_sbx_g2p3_ilib_recover(unit));
    }


    return rv;
}

soc_cm_isr_func_t
soc_sbx_fe2000_unregister_isr(int unit,  soc_sbx_fe2000_isr_type_t isrtype)
{
    soc_cm_isr_func_t isr = NULL;

    switch (isrtype) {
    case SOC_SBX_FE2000_ISR_TYPE_PMU_WATCHDOG:
        isr = isrs[unit][isrtype];
        isrs[unit][isrtype] = NULL;
        break;
    default:
        return NULL;
    }
    return isr;
}

void
soc_sbx_fe2000_register_isr(int unit, soc_sbx_fe2000_isr_type_t isrtype,
                            soc_cm_isr_func_t isr)
{
    uint32_t v;
    SOC_SBX_WARM_BOOT_DECLARE(int wb);

    SOC_SBX_WARM_BOOT_IGNORE(unit, wb);
    switch (isrtype) {
    case SOC_SBX_FE2000_ISR_TYPE_PMU_WATCHDOG:
        isrs[unit][isrtype] = isr;
        SOC_IRQ_MASK(unit) &=
            ~SAND_HAL_FE2000_FIELD_MASK(unit,PC_INTERRUPT_MASK, UNIT_DISINT);
        SOC_IRQ_MASK(unit) |= (1 << isrtype);
        SAND_HAL_FE2000_WRITE(unit, PC_INTERRUPT_MASK, SOC_IRQ_MASK(unit));
        v = SAND_HAL_FE2000_READ(unit, PC_UNIT_INTERRUPT_MASK);
        v &= ~SAND_HAL_FE2000_FIELD_MASK(unit, PC_UNIT_INTERRUPT_MASK, PM_DISINT);
        SAND_HAL_FE2000_WRITE(unit, PC_UNIT_INTERRUPT_MASK, v);
        v = SAND_HAL_FE2000_READ(unit, PM_ERROR_MASK);
        v &=
            ~(SAND_HAL_FE2000_FIELD_MASK(unit, PM_ERROR_MASK, TIMER_EXPIRED_FIFO_NONEMPTY_DISINT) |
              SAND_HAL_FE2000_FIELD_MASK(unit, PM_ERROR_MASK, TIMER_EXPIRED_FIFO_OVERFLOW_DISINT));
        SAND_HAL_FE2000_WRITE(unit, PM_ERROR_MASK, v);
        break;
    case SOC_SBX_FE2000_ISR_TYPE_CMU:
        /* configure CMU interrupts */
        SAND_HAL_FE2000_WRITE(unit, CM_FORCE_FLUSH_EVENT_MASK, 0x0);
        SAND_HAL_FE2000_WRITE(unit, CM_SEGMENT_FLUSH_EVENT_MASK, 0xffffffff);
        SAND_HAL_FE2000_WRITE(unit, CM_ERROR_MASK, 0xffffffff);
        SAND_HAL_FE2000_WRITE(unit, CM_SEGMENT_RANGE_ERROR_MASK, 0xffffffff);
        SAND_HAL_FE2000_WRITE(unit, CM_SEGMENT_DISABLE_ERROR_MASK, 0xffffffff);

        isrs[unit][isrtype] = isr;

        SOC_IRQ_MASK(unit) &=
            ~SAND_HAL_FE2000_FIELD_MASK(unit, PC_INTERRUPT_MASK, UNIT_DISINT);
        SOC_IRQ_MASK(unit) |= (1 << isrtype);
        SAND_HAL_FE2000_WRITE(unit, PC_INTERRUPT_MASK, SOC_IRQ_MASK(unit));
        v = SAND_HAL_FE2000_READ(unit, PC_UNIT_INTERRUPT_MASK);
        v &= ~SAND_HAL_FE2000_FIELD_MASK(unit, PC_UNIT_INTERRUPT_MASK, CM_DISINT);
        SAND_HAL_FE2000_WRITE(unit, PC_UNIT_INTERRUPT_MASK, v);

        break;
    default:
        soc_cm_print("soc_sbx_fe2000_register_isr invalid isrtype: %d\n",
                     isrtype);
    }
    SOC_SBX_WARM_BOOT_OBSERVE(unit, wb);
}

void
soc_sbx_fe2000_isr(void *_unit)
{
    int unit = PTR_TO_INT(_unit);
    soc_sbx_control_t *sbx;
    uint32 v, unit_v, unit_v_mask;
    SOC_SBX_WARM_BOOT_DECLARE(int wb);

    if (!_isrs_initialized[unit]) {
        /* We are here if chip is out of reset, yet the driver is not initialized,
         * or the device has been detached
         */
        return;
    }

    sbx = SOC_SBX_CONTROL(unit);
    assert(sbx);

    v = SAND_HAL_FE2000_READ(unit, PC_INTERRUPT);
    v |= SAND_HAL_FE2000_READ(unit, PC_UNIT_INTERRUPT);
    if (v==0){
        return;
    }

    SOC_SBX_WARM_BOOT_IGNORE(unit, wb);

    unit_v_mask = 0xffffffff;
    if (isrs[unit][SOC_SBX_FE2000_ISR_TYPE_PMU_WATCHDOG]) {
        unit_v_mask &= ~SAND_HAL_FE2000_FIELD_MASK(unit, PC_UNIT_INTERRUPT_MASK, PM_DISINT);
    }

    if (isrs[unit][SOC_SBX_FE2000_ISR_TYPE_CMU]) {
        unit_v_mask &= ~SAND_HAL_FE2000_FIELD_MASK(unit, PC_UNIT_INTERRUPT_MASK, CM_DISINT);
    }

    for (;;) {
        v = SAND_HAL_FE2000_READ(unit, PC_INTERRUPT);
        v &= ~SOC_IRQ_MASK(unit);

        /* read unit interrupt info */
        unit_v = SAND_HAL_FE2000_READ(unit, PC_UNIT_INTERRUPT);

        INTR_DEBUG(("%d:fe2000_isr: unit interrupts: 0x%08x unit_int_mask: 0x%08x\n", unit, unit_v, ~unit_v_mask));
        if (v & (SAND_HAL_FE2000_FIELD_MASK(unit, PC_INTERRUPT, DMA_0_DONE)
                 | SAND_HAL_FE2000_FIELD_MASK(unit, PC_INTERRUPT, DMA_1_DONE)
                 | SAND_HAL_FE2000_FIELD_MASK(unit, PC_INTERRUPT, DMA_2_DONE)
                 | SAND_HAL_FE2000_FIELD_MASK(unit, PC_INTERRUPT, DMA_3_DONE)
                 | SAND_HAL_FE2000_FIELD_MASK(unit, PC_INTERRUPT, DMA_SG_DONE))) {
            /* INTR_DEBUG(("%d:fe2000_isr:DMA completion\n", unit)); */
            (sbx->lib_isr)(_unit, v);

        } else if (v & SAND_HAL_FE2000_FIELD_MASK(unit, PC_INTERRUPT, LINK_STATUS_CHANGE) ) {
 
         INTR_DEBUG(("%d:fe2000_isr: link status change interrupt NOT IMPLEMENTED\n", unit));
          SAND_HAL_FE2000_WRITE(unit, PC_INTERRUPT,
                         SAND_HAL_FE2000_FIELD_MASK(unit, PC_INTERRUPT, LINK_STATUS_CHANGE));
          SAND_HAL_FE2000_WRITE(unit, PC_INTERRUPT_MASK, SOC_IRQ_MASK(unit));

        } else if (v & SAND_HAL_FE2000_FIELD_MASK(unit, PC_INTERRUPT, MIIM_OP_COMPLETE)) {

          INTR_DEBUG(("%d:fe2000_isr:MIIM op complete NOT IMPLEMENTED", unit));
          SAND_HAL_FE2000_WRITE(unit, PC_INTERRUPT,
                         SAND_HAL_FE2000_FIELD_MASK(unit, PC_INTERRUPT, MIIM_OP_COMPLETE));
          SAND_HAL_FE2000_WRITE(unit, PC_INTERRUPT_MASK, SOC_IRQ_MASK(unit));

        } else if (v & SAND_HAL_FE2000_FIELD_MASK(unit, PC_INTERRUPT, COUNTER_FIFO_DATA_AVAIL)) {

            /* INTR_DEBUG(("%d:fe2000_isr:Counter Ring FIFO Data avail\n", unit)); */
            (sbx->lib_isr)(_unit, v);
            SAND_HAL_FE2000_WRITE(unit, PC_INTERRUPT,
                                  SAND_HAL_FE2000_FIELD_MASK(unit, PC_INTERRUPT, COUNTER_FIFO_DATA_AVAIL) );

        } else if (v & SAND_HAL_FE2000_FIELD_MASK(unit, PC_INTERRUPT, PCI_COMPLETION) ) {

          soc_sbx_txrx_intr(unit, 0);
          INTR_DEBUG(("%d:fe2000_isr:PCI completion\n", unit));
          SAND_HAL_FE2000_WRITE(unit, PC_INTERRUPT,
                         SAND_HAL_FE2000_FIELD_MASK(unit, PC_INTERRUPT, PCI_COMPLETION) );

        } else if (v & SAND_HAL_FE2000_FIELD_MASK(unit, PC_INTERRUPT, PCI_ERROR0) ) {

          INTR_DEBUG(("%d:fe2000_isr:unimplemented PCI error0 "
                       "interrupt 0x%x\n", unit,
                       SAND_HAL_FE2000_READ(unit, PC_ERROR0)));
          SAND_HAL_FE2000_WRITE(unit, PC_INTERRUPT,
                         SAND_HAL_FE2000_FIELD_MASK(unit, PC_INTERRUPT, PCI_ERROR0));
          SAND_HAL_FE2000_WRITE(unit, PC_ERROR0_MASK, 0xffffffff);

        } else if (v & SAND_HAL_FE2000_FIELD_MASK(unit, PC_INTERRUPT, PCI_ERROR1) ) {

          INTR_DEBUG(("%d:fe2000_isr:unimplemented PCI error1 "
                       "interrupt 0x%x\n", unit,
                       SAND_HAL_FE2000_READ(unit, PC_ERROR1)));
          SAND_HAL_FE2000_WRITE(unit, PC_INTERRUPT,
                         SAND_HAL_FE2000_FIELD_MASK(unit, PC_INTERRUPT, PCI_ERROR1));
          SAND_HAL_FE2000_WRITE(unit, PC_ERROR1_MASK, 0xffffffff);

        }  else if (unit_v & (~unit_v_mask) ) {

            INTR_DEBUG(("%d:fe2000_isr: unit interrupts: 0x%08x unit_int_mask: 0x%08x\n", unit, unit_v, ~unit_v_mask));
            if ((unit_v & SAND_HAL_FE2000_FIELD_MASK(unit, PC_UNIT_INTERRUPT, PM))
                && (SAND_HAL_FE2000_READ(unit, PM_ERROR)
                    & (SAND_HAL_FE2000_FIELD_MASK(unit, PM_ERROR, TIMER_EXPIRED_FIFO_NONEMPTY) |
                       SAND_HAL_FE2000_FIELD_MASK(unit, PM_ERROR, TIMER_EXPIRED_FIFO_OVERFLOW)))
                && isrs[unit][SOC_SBX_FE2000_ISR_TYPE_PMU_WATCHDOG]) {

                (isrs[unit][SOC_SBX_FE2000_ISR_TYPE_PMU_WATCHDOG])((void *) unit);
                INTR_DEBUG(("%d:fe2000_isr:PMU unit interrupt\n", unit));

            } else if ((unit_v & SAND_HAL_FE2000_FIELD_MASK(unit, PC_UNIT_INTERRUPT, PM))
                && (v = SAND_HAL_FE2000_READ(unit, PM_ERROR))) {

                INTR_DEBUG(("%d:fe2000_isr:PMU unit interrupt (unsupported) 0x%x\n", unit, v));
                SAND_HAL_FE2000_WRITE(unit, PM_ERROR, v);

            } else if ((unit_v & SAND_HAL_FE2000_FIELD_MASK(unit, PC_UNIT_INTERRUPT, CM) )) {

                INTR_DEBUG(("%d:fe2000_isr:CM unit interrupt\n", unit));
                (sbx->lib_isr)(_unit, v);

            }else if (unit_v) {
                INTR_DEBUG(("%d:fe2000_isr:unimplemented unit interrupt 0x%x "
                             "(0x%x/0x%x)\n", unit,
                             SAND_HAL_FE2000_READ(unit, PC_UNIT_INTERRUPT),
                             SAND_HAL_FE2000_READ(unit, PC_UNIT_INTERRUPT_MASK),
                             ~unit_v_mask));
                unit_v_mask |= unit_v;
                break;
            }
        } else if (v) {
          INTR_DEBUG(("%d:fe2000_isr:unimplemented interrupt 0x%x "
                       "(0x%x/0x%x)\n", unit,
                       SAND_HAL_FE2000_READ(unit, PC_INTERRUPT),
                       SAND_HAL_FE2000_READ(unit, PC_INTERRUPT_MASK),
                       SOC_IRQ_MASK(unit)));
          SAND_HAL_FE2000_WRITE(unit, PC_INTERRUPT, v);
          SAND_HAL_FE2000_WRITE(unit, PC_INTERRUPT_MASK, SOC_IRQ_MASK(unit));
        } else{
            break;
        }
    }

    if (!(v & SAND_HAL_FE2000_FIELD_MASK(unit, PC_INTERRUPT_MASK, UNIT_DISINT) )) {
        if (isrs[unit][SOC_SBX_FE2000_ISR_TYPE_PMU_WATCHDOG]) {
            v = SAND_HAL_FE2000_READ(unit, PM_ERROR_MASK);
            v &=
            ~(SAND_HAL_FE2000_FIELD_MASK(unit, PM_ERROR_MASK, TIMER_EXPIRED_FIFO_NONEMPTY_DISINT) |
              SAND_HAL_FE2000_FIELD_MASK(unit, PM_ERROR_MASK, TIMER_EXPIRED_FIFO_OVERFLOW_DISINT));
            SAND_HAL_FE2000_WRITE(unit, PM_ERROR_MASK, v);
        } 

        if (isrs[unit][SOC_SBX_FE2000_ISR_TYPE_CMU]) {
        }

        INTR_DEBUG(("%d:fe2000_isr:re-enable unit_interrupt_mask: 0x%08x\n", unit, unit_v_mask));
        SAND_HAL_FE2000_WRITE(unit, PC_UNIT_INTERRUPT_MASK, unit_v_mask);
    }

    v = SOC_IRQ_MASK(unit);
    INTR_DEBUG(("%d:fe2000_isr:re-enable pc_interrupt_mask: 0x%08x\n", unit, v));
    SAND_HAL_FE2000_WRITE(unit, PC_INTERRUPT_MASK, v);

    SOC_SBX_WARM_BOOT_OBSERVE(unit, wb);
}

/*
 * Function:
 *      soc_sbx_fe2000_mac_probe
 * Purpose:
 *      Probe the specified unit/port for MAC type and return
 *      a mac_driver_t pointer used to drive the MAC.
 * Parameters:
 *      unit     - Device number
 *      port     - Port to probe
 *      macd_ptr - (OUT) Pointer to MAC driver.
 * Returns:
 *      SOC_E_NONE  - Success
 *      SOC_E_PARAM - Failure
 */
int
soc_sbx_fe2000_mac_probe(int unit, soc_port_t port, mac_driver_t **macdp)
{
    *macdp = NULL;

    if (IS_GE_PORT(unit, port)) {
        *macdp = &soc_fe2000_unimac;
    } else if (IS_XE_PORT(unit, port) || IS_HG_PORT(unit, port)) {
        *macdp = &soc_fe2000_xmac;
    } else {
        return SOC_E_PARAM;
    }

    return SOC_E_NONE;
}


/*
 *
 * Function:
 *     _soc_sbx_parse_block
 * Purpose:
 *     Parse and return block information type, number, and index (port
 *     offset) for given string of format,
 *         <block-type><block-num>.<block-port>
 *     where,
 *         <block-type> is one of the following block types ge, xe, spi, pci
 *         <block-num>  is the block number, 0-1
 *         <block-port> is the block port index (within the block)
 * Parameters:
 *     spn_str - String to parse
 *     type    - Returns parsed block type, SOC_BLK_xxx
 *     number  - Returns parsed block number
 *     port    - Returns parsed block port index
 * Returns:
 *     Number of characters parsed in given string.
 * Notes:
 *     If parameter was not found, returning parameter value is (-1).
 *     This routine parses block information based on above format, so
 *     it is NOT FE2000 specific.
 */
STATIC int
_soc_sbx_parse_block(int unit, char *spn_str, int *type, int *number, int *port)
{
    char  *s, *ts;
    int   value;
    int higig = 0;

    s = spn_str;
    *type = *number = *port = -1;

    /* Parse block type */
    if (!sal_strncasecmp(s, spn_UCODE_PORT_TYPE_GE,
                         sal_strlen(spn_UCODE_PORT_TYPE_GE))) {
        s += sal_strlen(spn_UCODE_PORT_TYPE_GE);
        *type = SOC_BLK_GPORT;

    } else if (!sal_strncasecmp(s, spn_UCODE_PORT_TYPE_XE,
                                sal_strlen(spn_UCODE_PORT_TYPE_XE))) {
        s += sal_strlen(spn_UCODE_PORT_TYPE_XE);
        *type = SOC_BLK_XPORT;

    } else if (!sal_strncasecmp(s, spn_UCODE_PORT_TYPE_HG,
                                sal_strlen(spn_UCODE_PORT_TYPE_HG))) {
        s += sal_strlen(spn_UCODE_PORT_TYPE_HG);
        *type = SOC_BLK_XPORT;
        higig = 1;
    } else if (!sal_strncasecmp(s, spn_UCODE_PORT_TYPE_SPI,
                                sal_strlen(spn_UCODE_PORT_TYPE_SPI))) {
        s += sal_strlen(spn_UCODE_PORT_TYPE_SPI);
        *type = SOC_BLK_SPI;

    } else if (!sal_strncasecmp(s, spn_UCODE_PORT_TYPE_PCI,
                                sal_strlen(spn_UCODE_PORT_TYPE_PCI))) {
        s += sal_strlen(spn_UCODE_PORT_TYPE_PCI);
        *type = SOC_BLK_CMIC;
    }

    /* Parse block number */
    value = sal_ctoi(s, &ts);
    if (ts != s) {
        *number = value;
        s = ts;
    }
    if (higig) {
        SOC_SBX_CFG_FE2000(unit)->ifmaskhigig |= 1 << sbFe2000TranslateBlock(*type, *number);
    }else if (*type == SOC_BLK_XPORT) {
        SOC_SBX_CFG_FE2000(unit)->ifmaskxg |= 1 << sbFe2000TranslateBlock(*type, *number);
    }

    /* Parse port number */
    if (*s == '.') {
        s++;
        value = sal_ctoi(s, &ts);
        if (ts != s) {
            *port = value;
            s = ts;
        }
    }

    return s - spn_str;
}

static int
_soc_sbx_is_valid_port(int unit, int blk_type, int blk_number, int blk_port)
{
    int num_ag_int, max_ag_port;
    int num_xg_int, max_xg_port;
    int num_spi_int, max_spi_port;

    num_ag_int = SB_FE2000_NUM_AG_INTERFACES;
    max_ag_port = SB_FE2000_MAX_AG_PORTS;
    max_xg_port = SB_FE2000_MAX_XG_PORTS;
    num_spi_int = SB_FE2000_NUM_SPI_INTERFACES;
    max_spi_port = SB_FE2000_MAX_PORTS_PER_SPI;
    if (SOC_IS_SBX_FE2KXT(unit)) {
        num_xg_int = SB_FE2000XT_NUM_XG_INTERFACES;
    }else{
        num_xg_int = SB_FE2000_NUM_XG_INTERFACES;
    }

    switch (blk_type) {
    case SOC_BLK_GPORT:
        if ((blk_number < 0) || (blk_port < 0) ||
            (blk_number >= num_ag_int) ||
            (blk_port >= max_ag_port)) {
            return FALSE;
        }
        break;
    case SOC_BLK_XPORT:
        /*
         * For XPORT block type, if port is -1 (not given)
         * the mapping corresponds to a HG interface.
         */
        if ((blk_number < 0) || /*(blk_port < 0) ||*/
            (blk_number >= num_xg_int) ||
            (blk_port >= max_ag_port)) {
            return FALSE;
        }
        break;
    case SOC_BLK_SPI:
        /*
         * For SPI block type, if port is -1 (not given),
         * the mapping corresponds to a SPI port rather
         * than a SPI subport.
         */
        if ((blk_number < 0) ||
            (blk_number >= num_spi_int) ||
            (blk_port >= max_spi_port)) {
            return FALSE;
        }
        break;

    case SOC_BLK_CMIC:
        /*
         * For CMIC block type, block number must be 0 and port -1
         */
        if ((blk_number != 0) || (blk_port != -1)) {
            return FALSE;
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}


/*
 *
 * Function:
 *     _soc_sbx_fe2000_ucode_value_get
 * Purpose:
 *     Parse ucode property value string into front-panel and
 *     system-side block information.
 *
 * Parameters:
 *     spn_str        - String to parse
 *     blk_type       - Returns parsed front panel block type, SOC_BLK_xxx
 *     blk_number     - Returns parsed front panel block number
 *     blk_port       - Returns parsed front panel port number
 *     sys_blk_type   - Returns parsed system-side block type, SOC_BLK_xxx
 *     sys_blk_number - Returns parsed system-side block number
 *     sys_blk_port   - Returns parsed system-side port number
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Invalid string value for ucode property
 *
 * Notes:
 *     (a) This routine performs parameter checking for valid block type,
 *         number and port on a FE2000, so it IS FE2000 specific.
 *     (b) For PCI (CPU) block, only <b-type> and <b-num> are expected.
 *     (c) For SPI block, if only <b-type> and <b-num> are given,
 *         ucode port mapping corresponds to a SPI port; otherwise,
 *         mapping correspond to a SPI subport.
 */
STATIC int
_soc_sbx_fe2000_ucode_get(int unit, char *spn_str,
                          int *blk_type, int *blk_number, int *blk_port,
                          int *sys_blk_type, int *sys_blk_number,
                          int *sys_blk_port)
{
    char  *s;
    int   n_chars;

    s = spn_str;
    *blk_type = *blk_number = *blk_port = -1;
    *sys_blk_type = *sys_blk_number = *sys_blk_port = -1;

    /* Parse Front panel block type/number/port */
    n_chars = _soc_sbx_parse_block(unit, s, blk_type, blk_number, blk_port);

    /* Check for block values */
    if (!_soc_sbx_is_valid_port(unit, *blk_type, *blk_number, *blk_port)) {
        return SOC_E_PARAM;
    }

    if (*blk_type == SOC_BLK_SPI && *blk_port < 0) {
        return SOC_E_NONE;
    }

    /* Parse System-side block type/number/port */
    s += n_chars;
    if (*s != ':') {
        if (*s == 0 && *blk_type == SOC_BLK_CMIC) {
          return SOC_E_NONE;
        } else {
          return SOC_E_PARAM;
        }
    }
    s++;
    n_chars = _soc_sbx_parse_block(unit, s, sys_blk_type, sys_blk_number,
                                   sys_blk_port);

    if (!_soc_sbx_is_valid_port(unit, *sys_blk_type, *sys_blk_number, *sys_blk_port)
        || s[n_chars] != 0) {
        return SOC_E_PARAM;
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *     _soc_sbx_fe2000_port_info_load
 * Purpose:
 *     Retrieves soc properties for mapping microcode ports to
 *     FE2000 device physical ports.
 *     If microcode port mapping is not given, the default
 *     FE2000 port mapping configuration is used.
 *
 *     SOC properties:
 *     "ucode_num_ports" - Indicates number of microcode ports.
 *                         If property is not defined or is invalid,
 *                         the default port mapping is loaded.
 *     "ucode_port"      - Mapping for microcode port 'x'.
 *
 *     For more information, see 'property.h'
 * Parameters:
 *     unit - Device number
 * Returns:
 *     SOC_E_NONE - Microcode port mapping loaded successfully (or default)
 *     SOC_E_XXX  - Failed to load microcode port mapping properties
 */
STATIC int
_soc_sbx_fe2000_port_info_load(int unit)
{
    int                i, port, max_port, num_ports_found, total_ports;
    uint32             ucode_num_ports;
    soc_driver_t       *driver;
    soc_sbx_control_t  *sbx;
    soc_port_info_t    *port_info;
    soc_port_info_t    *sys_port_info;
    char               property[20];
    char               *spn_str;
    int                blk, blk_type, blk_number, blk_port;
    int                sys_blk, sys_blk_type, sys_blk_number, sys_blk_port;
    int                cpu_port, hg_port[SB_FE2000_IF_PTYPE_MAX];
    int                spi_port[SB_FE2000_NUM_SPI_INTERFACES];
    int                spi_seen[SB_FE2000_NUM_SPI_INTERFACES];
    int                rv;

    /* Free previously dynamic allocated port info */
    if (_fe2000_port_info[unit]) {
        sal_free(_fe2000_port_info[unit]);
        _fe2000_port_info[unit] = NULL;
    }
    if (_fe2000_system_port_info[unit]) {
        sal_free(_fe2000_system_port_info[unit]);
        _fe2000_system_port_info[unit] = NULL;
    }

    /* Load default configuration (set to this first) */
    driver = SOC_DRIVER(unit);
    sbx    = SOC_SBX_CONTROL(unit);

    SOC_SBX_CFG_FE2000(unit)->ifmaskhigig = 0;
    SOC_SBX_CFG_FE2000(unit)->ifmaskxg = 0;

    if (SOC_IS_SBX_FE2KXT(unit)) {
        driver->port_info     = soc_ports_bcm88025_a0;
        sbx->system_port_info = soc_system_ports_bcm88025_a0;
    }else{
        driver->port_info     = soc_ports_bcm88020_a0;
        sbx->system_port_info = soc_system_ports_bcm88020_a0;
    }

    ucode_num_ports = soc_property_get(unit, spn_UCODE_NUM_PORTS, 0);
    if (ucode_num_ports == 0 || SBX_MAX_PORTS < ucode_num_ports) {
        /* Use default */
        SOC_SBX_CFG_FE2000(unit)->ifmaskxg = (1 << SB_FE2000_IF_PTYPE_XGM0) |
                                            (1 << SB_FE2000_IF_PTYPE_XGM1);
        return SOC_E_NONE;
    }

    /* Load user configuration */
    /* find maximum user defined port number */
    max_port = 0;
    num_ports_found = 0;
    for (port = 0; port < SBX_MAX_PORTS; port++) {
        spn_str = soc_property_port_get_str(unit, port, spn_UCODE_PORT);
        if (spn_str == NULL) {
            /* not found */
            continue;
        }
        max_port = port + 1;
        num_ports_found++;
    }

    if (num_ports_found != ucode_num_ports) {
        SOC_ERROR_PRINT((DK_WARN, "Found %d ucode_port connections, "
                         "but ucode_num_ports=%d\n", 
                         num_ports_found, ucode_num_ports));
    }

    /* Allocate for port_info structure */
    /* Note: Add space in array for SPI0, SPI1, CPU, end-of-list */
    total_ports = max_port + 4;
    port_info = sal_alloc((sizeof(soc_port_info_t) * total_ports),
                          "soc_sbx_port_info");
    if (port_info == NULL) {
        return SOC_E_MEMORY;
    }

    sys_port_info = sal_alloc((sizeof(soc_port_info_t) * total_ports),
                              "soc_sbx_sys_port_info");
    if (sys_port_info == NULL) {
        sal_free(port_info);
        return SOC_E_MEMORY;
    }

    /* Init array to indicate invalid entry and assign to device soc driver */
    for (port = 0; port < total_ports; port++) {
        port_info[port].blk        = -1;
        port_info[port].bindex     = 0;
        sys_port_info[port].blk    = -1;
        sys_port_info[port].bindex = 0;
    }
    driver->port_info     = _fe2000_port_info[unit]        = port_info;
    sbx->system_port_info = _fe2000_system_port_info[unit] = sys_port_info;

    /*
     * Load property ucode information to port info (skip invalid values)
     *
     * Notes:
     * Ports assigned for the SPI blocks (e.g. the
     * SPI buses) will contain a value of '-1' in the
     * 'bindex' field.  Also, these ports will not contain a valid
     * entry in the 'system-side'port block information.
     * The PCI block may or may not have a system-side port.  If
     * it does not have a system-side port, it is currently looped
     * from PCI->HPP->PCI.  For microcode that switches packets to
     * different ports (not currently supported), this configuration
     * would allow PCI port traffic to go anywhere, and for other
     * ports to reach the PCI port.  In other cases, this loop-back is
     * used for development & testing.
     */
    
    for (i=0; i<SB_FE2000_NUM_SPI_INTERFACES; i++) {
        spi_seen[i] =  spi_port[i] = FALSE;
    }
    for (i=0; i < SB_FE2000_IF_PTYPE_MAX; i++) {
        hg_port[i] = FALSE;
    }
    cpu_port = FALSE;

    for (port = 0; port < max_port; port++) {

        /* Search for ucode property */
        if ((spn_str = soc_property_port_get_str(unit, port, spn_UCODE_PORT))
            == NULL) {
            continue;
        }

        rv = _soc_sbx_fe2000_ucode_get(unit,
                                       spn_str,
                                       &blk_type, &blk_number, &blk_port,
                                       &sys_blk_type, &sys_blk_number,
                                       &sys_blk_port);
        if (SOC_FAILURE(rv)) {
            soc_cm_print("Invalid %s property value: %s\n",
                         property, spn_str);
            continue;
        }

        /* Set front-panel port block information */
        blk = soc_sbx_block_find(unit, blk_type, blk_number);
        if (blk >= 0) {
            port_info[port].blk    = blk;
            port_info[port].bindex = blk_port;
            /*
             * Set flags to indicate when user has already assigned a
             * port to SPI ports or CPU port.
             */
            if (blk_type == SOC_BLK_SPI) {
                spi_seen[!!blk_number] = TRUE;
                if (blk_port < 0) {    /* Indicates SPI port (not subport) */
                    spi_port[!!blk_number] = TRUE;
                }
            } else if (blk_type == SOC_BLK_CMIC) {
                cpu_port = TRUE;
            } else if ( SOC_SBX_CFG_FE2000_IF_IS_HIGIG(unit, sbFe2000TranslateBlock(blk_type, blk_number)) ) {
                if (blk_port < 0) {
                    hg_port[sbFe2000TranslateBlock(blk_type, blk_number)] = TRUE;
                }
            }
        }

        /* Set system-side port block information */
        if (sys_blk_type >= 0) {
            sys_blk = soc_sbx_block_find(unit, sys_blk_type, sys_blk_number);
            if (sys_blk >= 0) {
                sys_port_info[port].blk    = sys_blk;
                sys_port_info[port].bindex = sys_blk_port;
            }
        }
    }

    /* add ports for SPIs, CPU, and HGs as needed */
    /* If there is SPI0 or SPI1 subports were found, but the corresponding SPI
     * bus was not, add the appropriate port block
     */
    for (i=0; i<SB_FE2000_NUM_SPI_INTERFACES; i++) {
        if ((spi_seen[i] == TRUE) && (spi_port[i] == FALSE)) {
            blk = soc_sbx_block_find(unit, SOC_BLK_SPI, i);
            if (blk >= 0) {
                port_info[port].blk      = blk;
                port_info[port++].bindex = -1;
            }
        }
    }

    for (i=0; i < max_port; i++) {
        sys_blk_type = SOC_BLOCK_INFO(unit, sys_port_info[i].blk).type;
        sys_blk_number = SOC_BLOCK_INFO(unit, sys_port_info[i].blk).number;
        blk_type = SOC_BLOCK_INFO(unit, port_info[i].blk).type;
        blk_number = SOC_BLOCK_INFO(unit, port_info[i].blk).number;
        switch (sys_blk_type) {
            case SOC_BLK_CMIC:
                if (!cpu_port){
                    port_info[port].blk      = sys_port_info[i].blk;
                    port_info[port++].bindex = -1;
                    cpu_port = TRUE;
                }
                break;
            case SOC_BLK_XPORT:
                /* 10G system ports are higig */
                if (!hg_port[sys_port_info[i].blk]){ /*sys_blk_number]){*/
                    port_info[port].blk      = sys_port_info[i].blk;
                    port_info[port++].bindex = -1;
                    hg_port[sys_port_info[i].blk] = TRUE;
                    SOC_SBX_CFG_FE2000(unit)->ifmaskhigig |= 1 << sbFe2000TranslateBlock(sys_blk_type, sys_blk_number);
                }
                break;
            default:
                continue;
        }
    }

    /* Mark end of list */
    port_info[port].blk        = -1;
    port_info[port].bindex     = -1;
    sys_port_info[port].blk    = -1;
    sys_port_info[port].bindex = -1;

    assert(port < total_ports);

    return SOC_E_NONE;
}


/*
 * Function:
 *     soc_sbx_fe2000_port_info_config
 * Purpose:
 *     Configures mapping of microcode port to FE2000 physical ports.
 * Parameters:
 *     unit - Device number
 * Returns:
 *     SOC_E_NONE - Microcode port mapping loaded successfully
 * Notes:
 *     If user configuration is invalid not specified, the default
 *     microcode port mapping is loaded.
 */
int
soc_sbx_fe2000_port_info_config(int unit)
{
    int                 blk, bindex, blktype;
    int                 port, disabled_port, pno;
    char                *bname;
    soc_pbmp_t          pbmp_valid;
    soc_sbx_control_t   *sbx;
    soc_info_t          *si;
    soc_pbmp_t          pbmp_xport_xe;

    /* Load microcode port mapping into 'port_info' device driver */
    _soc_sbx_fe2000_port_info_load(unit);

    sbx = SOC_SBX_CONTROL(unit);
    pbmp_valid = soc_property_get_pbmp(unit, spn_PBMP_VALID, 1);

    pbmp_xport_xe = soc_property_get_pbmp(unit, spn_PBMP_XPORT_XE, 0);

    si  = &SOC_INFO(unit);
    for (blk = 0; blk < SOC_MAX_NUM_BLKS; blk++) {
        si->block_port[blk] = REG_PORT_ANY;
    }

#define ADD_PORT(ptype, port) \
            si->ptype.port[si->ptype.num++] = port; \
            if (si->ptype.min < 0) { \
                si->ptype.min = port; \
            } \
            if (port > si->ptype.max) { \
                si->ptype.max = port; \
            } \
            SOC_PBMP_PORT_ADD(si->ptype.bitmap, port);

    for (port = 0; ; port++) {
        disabled_port = FALSE;
        blk    = SOC_PORT_BLOCK(unit, port);
        bindex = SOC_PORT_BINDEX(unit, port);

        if (blk < 0 && bindex < 0) {  /* End of list */
            break;
        }
        if (blk < 0) {                /* Empty slot */
            disabled_port = TRUE;
            blktype = 0;
        } else {
            blktype = SOC_BLOCK_TYPE(unit, blk);
            if (!SOC_PBMP_MEMBER(pbmp_valid, port)) {   /* disabled port */
                if (blktype & SOC_BLK_CMIC) {
                    SOC_ERROR_PRINT((DK_WARN,
                                     "soc_sbx_info_port_config: "
                                     "cannot disable cpu port\n"));
                } else {
                    disabled_port = TRUE;
                }
            }
        }

        if (disabled_port) {
            sal_snprintf(si->port_name[port], sizeof(si->port_name[port]),
                         "?%d", port);
            si->port_offset[port] = port;

            continue;
        }

        bname = soc_block_port_name_lookup_ext(blktype, unit);

        switch (blktype) {
        case SOC_BLK_GPORT:
            pno = si->ge.num;
            ADD_PORT(ge,port);
            ADD_PORT(ether, port);
            ADD_PORT(port, port);
            ADD_PORT(all, port);
            bname = "ge";
            break;
        case SOC_BLK_XPORT:
            if (SOC_PBMP_NOT_NULL(pbmp_xport_xe) && \
                SOC_PBMP_MEMBER(pbmp_xport_xe, port) && \
                (soc_property_port_get(unit, port, spn_10G_IS_CX4, TRUE) )) { /* XE  vs HG  */
                pno = si->xe.num;
                ADD_PORT(xe, port);
                ADD_PORT(ether, port);
                bname = "xe";
                si->port_speed_max[port] = 10000;
                ADD_PORT(port, port);
                ADD_PORT(all, port);
            } else {
                if (bindex < 0) {
                    /* HG interface port */
                    pno = si->hg.num;
                    ADD_PORT(hg, port);
                    ADD_PORT(ether, port);
                    si->port_speed_max[port] = 12000;
                    ADD_PORT(port, port);
                    ADD_PORT(all, port);
                    bname = "hg";
                }else{
                    /* HG subport */
                    pno = si->hg.num;
                    si->port_speed_max[port] = 12000;
                    ADD_PORT(hg_subport, port);
                    ADD_PORT(port, port);
                    ADD_PORT(all, port);
                }
            }
            break;
        case SOC_BLK_SPI:
            /*
             * Set appropriate port bitmap in
             * SPI interface port or SPI subport
             */
            if (bindex < 0) {
                /* SPI interface port */
                pno = si->spi.num;
                ADD_PORT(spi, port);
                ADD_PORT(all, port);
                bname = "spi";
            } else {
                /* SPI subport */
                pno = si->spi_subport.num;
                ADD_PORT(spi_subport, port);
                ADD_PORT(port, port);
                ADD_PORT(all, port);
                bname = "spi_s";
            }
            break;
        case SOC_BLK_CMIC:
            pno = 0;
            si->cmic_port = port;
            si->cmic_block = blk;
            SOC_PBMP_PORT_ADD(si->cmic_bitmap, port);
            ADD_PORT(all, port);
            bname = "cpu";
            break;
        default:
            pno = 0;
            break;
        }

        if (bname[0] == '?') {
            pno = port;
        }
        sal_snprintf(si->port_name[port], sizeof(si->port_name[port]),
                     "%s%d", bname, pno);
        si->port_type[port] = blktype;
        si->port_offset[port] = pno;
        si->block_valid[blk] += 1;
        if (si->block_port[blk] < 0) {
            si->block_port[blk] = port;
        }
        SOC_PBMP_PORT_ADD(si->block_bitmap[blk], port);
    }

    si->port_num = port;

#undef ADD_PORT

    return SOC_E_NONE;
}


/*
 * Function:
 *     soc_sbx_fe2000_ucode_value_set
 * Purpose:
 *     Update the name constant in the ucode package,
 *     then update the proper named constant value in the LRP memory
 * Parameters:
 *     unit  - Device number
 *     ucode - Pointer to the ucode package
 *     a_n   - String name of the named constant
 *     value - New value of the named constant
 * Returns:
 *     SOC_E_NONE - Constant value gets updated successfully
 */

int
soc_sbx_fe2000_ucode_value_set(int unit,
			       char *a_n, unsigned int value)
{
  unsigned int status;
  soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);

  status = sbFe2000SetNamedConstant(sbx->drv, sbx->ucode, a_n, value);
  if(status != SB_OK)
    return status;

  return SOC_E_NONE;
}

/*
 * Function:
 *     soc_sbx_fe2000_ucode_value_get
 * Purpose:
 *     Get the name constant value from the ucode package.
 * Parameters:
 *     unit  - Device number
 *     ucode - Pointer to the ucode package
 *     a_n   - String name of the named constant
 *     value - New value of the named constant
 * Returns:
 *     SOC_E_NONE - Constant value gets updated successfully
 * Note:
 *     We assume the value in package file sync correctly with
 *     the one in LRP memory
 */
int
soc_sbx_fe2000_ucode_value_get(int unit,
			       char *a_n, unsigned int *value)
{
  unsigned int status;
  soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);

  status = sbFe2000GetNamedConstant(sbx->drv, sbx->ucode, a_n, (unsigned int *)&value);
  if(status != SB_OK)
    return status;

  /* no need to read from HW, it should sync with the package */

  return SOC_E_NONE;
}

/*
 * Function:
 *     soc_sbx_fe2000_ucode_load
 * Purpose:
 *     Load a new ucode package into LRP memory
 * Parameters:
 *     unit  - Device number
 *     ucode - Pointer to the ucode package
 * Returns:
 *     SOC_E_NONE - Constant value gets updated successfully
 */
int
soc_sbx_fe2000_ucode_load(int unit, void *ucode)
{
    soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);

    BCM_SBX_IF_ERROR_RETURN(sbFe2000ReloadUcode(SOC_SBX_SBHANDLE(unit), ucode));
    SOC_IF_ERROR_RETURN(fe2kAsm2IndD__initQat(sbx->ucode));
  
    sbx->ucode = ucode;
    if (sbx->ucode_update_func) {
        SOC_IF_ERROR_RETURN((sbx->ucode_update_func)(unit));
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *     soc_sbx_fe2000_ucode_load_from_buffer
 * Purpose:
 *     Load a new ucode package from buffer into ucode interface structure
 * Parameters:
 *     unit  - Device number
 *     buf   - ucode buffer, input of this function
 *     len   - length of the ucode buffer
 * Returns:
 *     SOC_E_NONE - Constant value gets updated successfully
 */
int
soc_sbx_fe2000_ucode_load_from_buffer(int unit, void *buf, unsigned int len)
{
    soc_sbx_control_t *sbx = SOC_SBX_CONTROL(unit);
    unsigned int ret = SB_OK;

    /* fe2kAsm2IntD__init(sbx->ucode) is called in destroy */
    if((ret = fe2kAsm2IntD__destroy(sbx->ucode)) != 0)
        return ret;

    SOC_IF_ERROR_RETURN(fe2kAsm2IntD__readBuf(sbx->ucode, buf, len));
    SOC_IF_ERROR_RETURN(fe2kAsm2IndD__initQat(sbx->ucode));

    if (sbx->ucode_update_func) {
        SOC_IF_ERROR_RETURN((sbx->ucode_update_func)(unit));
    }
  
    return SOC_E_NONE;
}

/*
 * Function:
 *     soc_sbx_fe2000_egress_pages_set
 * Purpose:
 *     Set the flow control thresholds for egress SWS queues based on
 *       port rate and MTU
 * Parameters:
 *     unit  - Device number
 *     port  - port number
 *     mbps  - port rate, in mbits/sec
 *     mtu   - port MTU, in bytes
 * Returns:
 *     SOC_E_NONE - Constant value gets updated successfully
 */
int
soc_sbx_fe2000_egress_pages_set(int unit, int port, int mbps, int mtu)
{
    sbStatus_t status;

    if( !SOC_SBX_INIT(unit) ){
        return SOC_E_INIT;
    }

    status = sbFe2000SetPortPages(SOC_SBX_SBHANDLE(unit),
                                  &SOC_SBX_CFG_FE2000(unit)->init_params,
                                  port, mbps, mtu);

    return soc_sbx_translate_status(status);
}

#endif /* BCM_FE2000_SUPPORT */
