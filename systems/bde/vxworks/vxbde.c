/*
 * $Id: vxbde.c 1.101.2.9 Broadcom SDK $
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
 * VxWorks BDE
 */

#include "vxbde.h"

#ifdef BCM_ROBO_SUPPORT
#include <osl.h>
#include <sbconfig.h>
#if defined(KEYSTONE)
#include <bcmutils.h>
#include <siutils.h>
#include <sbchipc.h>
#include <hndpci.h>
#include "../../drv/et/etc_robo_spi.h"
#else
#include <sbutils.h>
#include "../../drv/et/etc_robo.h"
#endif
#include <ethernet.h>
#include <et_dbg.h>
#include <etc53xx.h>

#include <shared/et/osl.h>
#include <shared/et/sbconfig.h>
#include <shared/et/bcmenet47xx.h>
#endif

#if defined(KEYSTONE)
#include <hndsoc.h>
#endif /* KEYSTONE */

#include <sal/types.h>
#include <sal/appl/pci.h>
#include <sal/appl/sal.h>
#include <sal/appl/config.h>
#include <sal/core/boot.h>
#include <sal/core/libc.h>
#include <soc/devids.h>

#include <soc/defs.h>
#include <soc/types.h>

#include <sal/appl/io.h>

#include "intLib.h"
#include <shared/util.h>
/* Structure of private spi device */
typedef struct spi_dev_s {
    uint8          cid;         /* Chip ID */
    uint32          part;        /* Part number of the chip */
    uint8          rev;         /* Revision of the chip */
    void           *robo;       /* ptr to robo info required to access SPI */
    unsigned short phyid_high;  /* PHYID HIGH in MII regs of detected chip */
    unsigned short phyid_low;   /* PHYID LOW in MII regs of detected chip */
} spi_dev_t;

struct spi_device_id {
    unsigned short phyid_high;  /* PHYID HIGH in MII regs of detected chip */
    unsigned short phyid_low;   /* PHYID LOW in MII regs of detected chip */
};

typedef struct vxbde_dev_s {
    /* Specify the type of device, pci, spi, switch, ether ... */
    uint32 dev_type;

    ibde_dev_t bde_dev;
    union {
        pci_dev_t  _pci_dev;    /* PCI device type */
    spi_dev_t  _spi_dev;    /* SPI device type */
    } dev;
#define pci_dev     dev._pci_dev
#define spi_dev     dev._spi_dev
    int iLine;
    int iPin;

#define BDE_FLAG_BUS_RD_16BIT        0x1
#define BDE_FLAG_BUS_WR_16BIT        0x10
    uint32  flags;
} vxbde_dev_t;

#define MAX_SWITCH_DEVICES 16
#define MAX_ETHER_DEVICES 2
#define MAX_DEVICES (MAX_SWITCH_DEVICES + MAX_ETHER_DEVICES)
#define BCM47XX_ENET_ID     0x4713      /* 4710 enet */
#define BCM5300X_GMAC_ID     0x4715    

#define PL0_OFFSET  0x00800000
#define PL0_SIZE    0x00040000

#ifdef METROCORE
#define FPGA_IRQ    37
#else
#define FPGA_IRQ    48
#endif /* METROCORE */

static vxbde_dev_t _devices[MAX_DEVICES];
static int _n_devices = 0;
static int _switch_n_devices = 0;
static int _ether_n_devices = 0;
 
#ifndef BCM_ICS
static int _pri_bus_no = -1;
static int _n_pri_devices = 0;
static int _n_sec_devices = 0;

/* Assume there's only one PLX PCI-to-Local bus bridge if any */
static vxbde_dev_t plx_vxd;
static int num_plx = 0;

#ifndef METROCORE
static int _first_mac = 0;
#endif /* METROCORE */
#endif /* BCM_ICS */

#define VALID_DEVICE(_n) ((_n >= 0) && (_n < _n_devices))
#define DEVICE_INDEX(_n) ((_n < _switch_n_devices) ? _n : \
                  (MAX_SWITCH_DEVICES+_n-_switch_n_devices))

#ifdef BCM_ROBO_SUPPORT
static int _robo_devices = 0;
static void *robo = NULL;
#if defined(KEYSTONE)
static si_t *sih = NULL;
#else
static void *sbh = NULL;
#endif /* KEYSTONE */
#endif

#if defined(KEYSTONE)
static int pcie1_device_exist = 0;
static uint32  coreflags = 0;
#ifndef PCIE_PORT0_HB_BUS
#define PCIE_PORT0_HB_BUS (1)
#endif
#ifndef PCIE_PORT1_HB_BUS
#define PCIE_PORT1_HB_BUS (17)
#endif
#ifndef SI_PCI0_MEM
#define SI_PCI0_MEM 0x08000000
#endif
#ifndef SI_PCI1_MEM
#define SI_PCI1_MEM 0x40000000
#endif
#endif

static vxbde_bus_t _bus;

uint32 bcm_bde_soc_cm_memory_base = 0;

#define BDE_DEV_MEM_MAPPED(d)                               \
            ((d) & (BDE_PCI_DEV_TYPE | BDE_ICS_DEV_TYPE |   \
                    BDE_EB_DEV_TYPE))

#ifdef INCLUDE_RCPU
/*****************EB devices***************/

static uint32  _read(int d, uint32 offset);
static int _write(int d, uint32 offset, uint32 data);


int vxbde_eb_bus_probe(vxbde_bus_t *bus, ibde_t **bde)
{
    char                prop[64], *s;
    int                 unit = 0;
    uint32_t            baddr;

    for (unit = _n_devices; unit < MAX_SWITCH_DEVICES; unit++) {
        sal_sprintf(prop, "eb_dev_addr.%d", unit);

        if ((s = sal_config_get(prop)) && 
            (baddr = sal_ctoi(s, 0))) {
            vxbde_eb_create(bus, bde, baddr);
        }
    }

    return 0;
}
#endif /* INCLUDE_RCPU */

static const char *
_name(void)
{
    return "vxworks-pci-bde";
}

static int
_num_devices(int type)
{
    switch (type) {
        case BDE_ALL_DEVICES:
            return _n_devices;
        case BDE_SWITCH_DEVICES:
            return _switch_n_devices; 
        case BDE_ETHER_DEVICES:
            return _ether_n_devices; 
    }

    return 0;
}

static const ibde_dev_t *
_get_dev(int d)
{
    int _d;

    if (!VALID_DEVICE(d)) {
    return NULL;
    }
    _d = DEVICE_INDEX(d);

    return &_devices[_d].bde_dev;
}

static uint32
_get_dev_type(int d)
{
    int _d;

    if (!VALID_DEVICE(d)) {
        return  0;
    }
    _d = DEVICE_INDEX(d);

    return _devices[_d].dev_type;
}

static uint32 
_pci_read(int d, uint32 addr)
{
    int _d;

    if (!VALID_DEVICE(d)) {
        return (uint32)0xFFFFFFFF;
    }
    _d = DEVICE_INDEX(d);

    if (!(_devices[_d].dev_type & BDE_PCI_DEV_TYPE)) {
        return (uint32)0xFFFFFFFF;
    }

    return pci_config_getw(&_devices[_d].pci_dev, addr);
}

static int
_pci_write(int d, uint32 addr, uint32 data)
{
    int _d;

    if (!VALID_DEVICE(d)) {
        return -1;
    }
    _d = DEVICE_INDEX(d);

    if (!(_devices[_d].dev_type & BDE_PCI_DEV_TYPE)) {
        return -1;
    }
    return pci_config_putw(&_devices[_d].pci_dev, addr, data);
}

static void
_pci_bus_features(int unit, int *be_pio, int *be_packet, int *be_other)
{
    *be_pio = _bus.be_pio;
    *be_packet = _bus.be_packet;
    *be_other = _bus.be_other;
}

#ifdef PCI_DECOUPLED
static uint32  
_read(int d, uint32 offset)
{
    int _d, s;
    uint16  msb, lsb;

    if (!VALID_DEVICE(d)) {
        return -1;
    }
    _d = DEVICE_INDEX(d);

    if (_devices[_d].dev_type & BDE_PCI_DEV_TYPE) {
        return sysPciRead(
            &(((uint32 *)_devices[_d].bde_dev.base_address)[offset / 4]));
    }

    if (_devices[_d].dev_type & (BDE_ICS_DEV_TYPE | BDE_EB_DEV_TYPE )) {
        if (_devices[_d].flags & BDE_FLAG_BUS_RD_16BIT) {
        s = intLock();
            offset = (offset & 0xffff0000) | ((offset & 0xffff) << 1);
            lsb = *((uint16 *)(_devices[_d].bde_dev.base_address + offset));
            msb = *((uint16 *)(_devices[_d].bde_dev.base_address + offset));
        intUnlock(s);
            return (msb << 16) | lsb;
        } else {
            return ((uint32 *)_devices[_d].bde_dev.base_address)[offset / 4];
        }
    }
    return -1;
}

static int
_write(int d, uint32 offset, uint32 data)
{
    int _d, s;

    if (!VALID_DEVICE(d)) {
        return -1;
    }
    _d = DEVICE_INDEX(d);

    if (_devices[_d].dev_type & BDE_PCI_DEV_TYPE) {
        sysPciWrite(
            &(((uint32 *)_devices[_d].bde_dev.base_address)[offset / 4]),
            data);
        return 0;
    }

    if (_devices[_d].dev_type & (BDE_ICS_DEV_TYPE | BDE_EB_DEV_TYPE )) {
        if (_devices[_d].flags & BDE_FLAG_BUS_WR_16BIT) {
        s = intLock();
            offset = (offset & 0xffff0000) | ((offset & 0xffff) << 1);
            *((uint16 *)(_devices[_d].bde_dev.base_address + offset)) =
                (data & 0xffff);
            *((uint16 *)(_devices[_d].bde_dev.base_address + offset)) = 
                ((data>>16) & 0xffff);
        intUnlock(s);
        } else {
            ((uint32 *)_devices[_d].bde_dev.base_address)[offset / 4] = data;
        }
        return 0;
    }

    return -1;
}
#else
static uint32  
_read(int d, uint32 offset)
{
    int _d, s;
    volatile uint16  msb, lsb;

    if (!VALID_DEVICE(d)) {
        return -1;
    }
    _d = DEVICE_INDEX(d);

    if (!(BDE_DEV_MEM_MAPPED(_devices[_d].dev_type))) {
        return -1;
    }

    if (_devices[_d].flags & BDE_FLAG_BUS_RD_16BIT) {
        s = intLock();
        offset = (offset & 0xffff0000) | ((offset & 0xffff) << 1);
        lsb = *((uint16 *)(_devices[_d].bde_dev.base_address + offset));
        msb = *((uint16 *)(_devices[_d].bde_dev.base_address + offset));
    intUnlock(s);
        return (msb << 16) | lsb;
    } else {
        return ((uint32 *)_devices[_d].bde_dev.base_address)[offset / 4];
    }
}

static int
_write(int d, uint32 offset, uint32 data)
{
    int _d, s;

    if (!VALID_DEVICE(d)) {
        return -1;
    }
    _d = DEVICE_INDEX(d);

    if (!(BDE_DEV_MEM_MAPPED(_devices[_d].dev_type))) {
        return -1;
    }

    if (_devices[_d].flags & BDE_FLAG_BUS_WR_16BIT) {
        s = intLock();
        offset = (offset & 0xffff0000) | ((offset & 0xffff) << 1);
        *((uint16 *)(_devices[_d].bde_dev.base_address + offset)) =
            (data & 0xffff);
        *((uint16 *)(_devices[_d].bde_dev.base_address + offset)) = 
            ((data>>16) & 0xffff);
    intUnlock(s);
    } else {
        ((uint32 *)_devices[_d].bde_dev.base_address)[offset / 4] = data;
    }
    return 0;
}
#endif

static uint32 * 
_salloc(int d, int size, const char *name)
{
    COMPILER_REFERENCE(d);

    return sal_dma_alloc(size, (char *)name);
}

static void
_sfree(int d, void *ptr)
{
    COMPILER_REFERENCE(d);

    sal_dma_free(ptr);
}

static int 
_sflush(int d, void *addr, int length)
{
    COMPILER_REFERENCE(d);

    sal_dma_flush(addr, length);

    return 0;
}

static int
_sinval(int d, void *addr, int length)
{
    COMPILER_REFERENCE(d);

    sal_dma_inval(addr, length);

    return 0;
}

#ifdef METROCORE
extern STATUS sysFPGAIntConnect(VOIDFUNCPTR isr, int arg);
#endif

static int 
_interrupt_connect(int d, void (*isr)(void *), void *data)
{
    int _d;
#ifdef METROCORE
    STATUS status;
    static int FPGAIntConnected = 0;
#endif

    if (!VALID_DEVICE(d)) {
        return -1;
    }
    _d = DEVICE_INDEX(d);

    if (!(_devices[_d].dev_type & 
          (BDE_PCI_DEV_TYPE | BDE_ICS_DEV_TYPE | BDE_EB_DEV_TYPE))) {
        return -1;
    }

#ifdef METROCORE
    if ((_devices[_d].iLine == FPGA_IRQ) && !FPGAIntConnected) {
        status = sysFPGAIntConnect(isr, (int)data);
    FPGAIntConnected = 1;
        return status;
    }
    /*
     * FPGA ISR can only be connected once.
     * One ISR will have to handle all the FPGA interrupts.
     */
    if ((_devices[_d].iLine == FPGA_IRQ) && FPGAIntConnected) {
    return OK;
    }
#endif

    return pci_int_connect(_devices[_d].iLine, isr, data);
}

static int
_interrupt_disconnect(int d)
{
    COMPILER_REFERENCE(d);
    return 0;
}

#ifdef   BCM_ICS
#define SB_K1_TO_PHYS(x) ((uint32)x & 0x1fffffff)
#define SB_PHYS_TO_K1(x) ((uint32)x | 0xa0000000)

static uint32 
_l2p(int d, void *laddr)
{
    COMPILER_REFERENCE(d);

    return (uint32) SB_K1_TO_PHYS(laddr);
}

static uint32 *
_p2l(int d, uint32 paddr)
{
    COMPILER_REFERENCE(d);

    return (uint32 *) SB_PHYS_TO_K1(paddr);
}

#else
   
#if defined(NSX) || defined(METROCORE)
#define SB_K0_TO_PHYS(x) ((uint32)x & 0x7fffffff)
#define SB_PHYS_TO_K0(x) ((uint32)x | 0x80000000)
#endif
#define SB_K1_TO_PHYS(x) ((uint32)x & 0x1fffffff)
#define SB_PHYS_TO_K1(x) ((uint32)x | 0xa0000000)

static uint32 
_l2p(int d, void *laddr)
{

#if defined(NSX) || defined(METROCORE)
    COMPILER_REFERENCE(d);
    return (uint32) SB_K0_TO_PHYS(laddr);
#else
    int _d;

    if (!VALID_DEVICE(d)) {
        return -1;
    }
    _d = DEVICE_INDEX(d);

#if defined(KEYSTONE)
    return (uint32) SB_K1_TO_PHYS(laddr);
#else
    if ((_devices[_d].dev_type & BDE_ETHER_DEV_TYPE)) {
        return (uint32) SB_K1_TO_PHYS(laddr);
    }else {
    return (uint32)laddr;
    }
#endif

#endif
}

static uint32 *
_p2l(int d, uint32 paddr)
{

#if defined(NSX) || defined(METROCOORE)
    COMPILER_REFERENCE(d);
    return (uint32 *) SB_PHYS_TO_K0(paddr);
#else
    int _d;

    if (!VALID_DEVICE(d)) {
        return NULL;
    }
    _d = DEVICE_INDEX(d);

#if defined(KEYSTONE)
    return (uint32 *) SB_PHYS_TO_K1(paddr);
#else
    if ((_devices[_d].dev_type & BDE_ETHER_DEV_TYPE)) {
        return (uint32 *) SB_PHYS_TO_K1(paddr);
    }else {
    return (uint32 *) paddr;
    }
#endif
#endif
}

#endif
#ifdef BCM_ROBO_SUPPORT
#define SOC_ROBO_PAGE_BP        8    /* for Robo Chip only */
static int
_spi_read(int d, uint32 addr, uint8 *buf, int len)
{
    int _d;
    uint8 page, offset;

    if (!VALID_DEVICE(d)) return -1;
    _d = DEVICE_INDEX(d);

    if (!(_devices[_d].dev_type & BDE_SPI_DEV_TYPE)) {
        return -1;
    }

    page = (addr >> SOC_ROBO_PAGE_BP) & 0xFF;
    offset = addr & 0xFF;

    robo_rreg(_devices[_d].spi_dev.robo, _devices[_d].spi_dev.cid,
              page, offset, buf, (uint)len);

    return 0;
}

static int
_spi_write(int d, uint32 addr, uint8 *buf, int len)
{
    int _d;
    uint8 page, offset;

    if (!VALID_DEVICE(d)) return -1;
    _d = DEVICE_INDEX(d);

    if (!(_devices[_d].dev_type & BDE_SPI_DEV_TYPE)) {
        return -1;
    }

    page = (addr >> SOC_ROBO_PAGE_BP) & 0xFF;
    offset = addr & 0xFF;

    robo_wreg(_devices[_d].spi_dev.robo, _devices[_d].spi_dev.cid,
              page, offset, buf, (uint)len);

    return 0;
}
#ifdef INCLUDE_ROBO_I2C
static int
_i2c_read(int d, uint16 addr, uint8 *buf, int len)
{
    int _d;
    uint8 chipid, reg_addr;
    
    if (!VALID_DEVICE(d)) return -1;
    _d = DEVICE_INDEX(d);

    if (!(_devices[_d].dev_type & BDE_SPI_DEV_TYPE)) {
        return -1;
    }

    chipid = (addr >> 8) & 0xFF;
    reg_addr = addr & 0xFF;

    robo_i2c_rreg(_devices[_d].spi_dev.robo, chipid, reg_addr, buf, (uint)len);

    return 0;
}

static int
_i2c_write(int d, uint16 addr, uint8 *buf, int len)
{
    int _d;
    uint8 chipid, reg_addr;

    if (!VALID_DEVICE(d)) return -1;
    _d = DEVICE_INDEX(d);

    if (!(_devices[_d].dev_type & BDE_SPI_DEV_TYPE)) {
        return -1;
    }

    chipid = (addr >> 8) & 0xFF;
    reg_addr = addr & 0xFF;

    robo_i2c_wreg(_devices[_d].spi_dev.robo, chipid, reg_addr, buf, (uint)len);

    return 0;
}

static int
_i2c_read_intr(int d, uint8 chipid, uint8 *buf, int len)
{
    int _d;
    
    if (!VALID_DEVICE(d)) return -1;
    _d = DEVICE_INDEX(d);

    if (!(_devices[_d].dev_type & BDE_SPI_DEV_TYPE)) {
        return -1;
    }

    robo_i2c_rreg_intr(_devices[_d].spi_dev.robo, chipid, buf);

    return 0;
}

static int
_i2c_read_ARA(int d, uint8 *chipid, int len)
{
    int _d;
    
    if (!VALID_DEVICE(d)) return -1;
    _d = DEVICE_INDEX(d);

    if (!(_devices[_d].dev_type & BDE_SPI_DEV_TYPE)) {
        return -1;
    }

    robo_i2c_read_ARA(_devices[_d].spi_dev.robo, chipid);

    return 0;
}
#endif
#endif
static ibde_t _ibde = {
    _name, 
    _num_devices, 
    _get_dev, 
    _get_dev_type, 
    _pci_read,
    _pci_write,
    _pci_bus_features,
    _read,
    _write,
    _salloc,
    _sfree,
    _sflush,
    _sinval,
    _interrupt_connect,
    _interrupt_disconnect,
    _l2p,
    _p2l,
#ifdef BCM_ROBO_SUPPORT
    _spi_read,
    _spi_write,
#ifdef INCLUDE_ROBO_I2C
    _i2c_read,
    _i2c_write,
    _i2c_read_intr,
    _i2c_read_ARA,
#endif
#endif
};
#ifdef BCM_ROBO_SUPPORT
#ifdef MDC_MDIO_SUPPORT
#define PHYRD_TIMEOUT 500
#define PSEUDO_PHY_ADDR 0x1e
static int page_number = 0xff; /* init value */

uint16
_chipphyrd(uint8 cid, uint phyaddr, uint reg)
{
    uint16 process = 1;
    uint16 chipphyrd_timeout_count = 0;
    uint16 v;

    /* local phy: our emac controls our phy */

    /* clear mii_int */
    _write(cid, 0x41c, EI_MII);

    /* issue the read */
    _write(cid, 0x414,  (MD_SB_START | MD_OP_READ | 
    (phyaddr << MD_PMD_SHIFT) | (reg << MD_RA_SHIFT) | MD_TA_VALID));

    /* wait for it to complete */
    while (process) {
        if ((_read(cid, 0x41c) & EI_MII) != 0) {
            process = 0;
        }
        chipphyrd_timeout_count ++;
        
        if (chipphyrd_timeout_count > PHYRD_TIMEOUT) {
            break;
        }
    }
        
    if ((_read(cid, 0x41c) & EI_MII) == 0) {
        /* printk("_chipphyrd: did not complete\n"); */
    }

    v = _read(cid, 0x414) & MD_DATA_MASK;
    return v;
}

void
_chipphywr(uint8 cid, uint phyaddr, uint reg, uint16 v)
{
    uint16 process = 1;
    uint16 chipphyrd_timeout_count = 0;

    /* local phy: our emac controls our phy */

    /* clear mii_int */
    _write(cid, 0x41c, EI_MII);
    
    /* issue the write */
    _write(cid, 0x414,  (MD_SB_START | MD_OP_WRITE 
    | (phyaddr << MD_PMD_SHIFT) | (reg << MD_RA_SHIFT) | MD_TA_VALID | v));

    /* wait for it to complete */
    while (process) {
        if ((_read(cid, 0x41c) & EI_MII) != 0) {
            process = 0;
        }
        chipphyrd_timeout_count ++;
        
        if (chipphyrd_timeout_count > PHYRD_TIMEOUT) {
            break;
        }
    }
    
    if ((_read(cid, 0x41c) & EI_MII) == 0) {
        /* printk("_chipphywr: did not complete\n"); */
    }
}

int
_soc_reg_read(uint8 cid, uint32 addr, uint8 *buf, int len)
{
    uint8 page, offset;
    uint16 phywr_val = 0, phyrd_val = 0;
    uint16 process,phyrd_timeout_count;
    uint16 *rd_buf = NULL;

    page = (addr >> SOC_ROBO_PAGE_BP) & 0xFF;
    offset = addr & 0xFF;

    /* If accessing register is in another page*/
    if (page != page_number) {
        phywr_val = (page << 8) | (cid & 0x3) << 1 | 0x1;
        _chipphywr(cid, PSEUDO_PHY_ADDR, 16, phywr_val);
        page_number = page;
    }

    phywr_val = (offset << 8) | 0x2; /*OP code read.*/
    _chipphywr(cid, PSEUDO_PHY_ADDR, 17, phywr_val);
        
    process = 1;
    phyrd_timeout_count = 0;

    while (process) {
        phyrd_val = _chipphyrd(cid, PSEUDO_PHY_ADDR, 17);
        if (!(phyrd_val & 0x03)) {
            process = 0;
        }
        phyrd_timeout_count ++;
        
        if (phyrd_timeout_count > PHYRD_TIMEOUT) {
            printf("%d: _soc_reg_read, TIMEOUT\n", __LINE__);
            return -ETIMEDOUT;
        }
    }
    rd_buf = (uint16 *)buf; 
    *rd_buf = _chipphyrd(cid, PSEUDO_PHY_ADDR, 24);
    *(rd_buf + 1) = _chipphyrd(cid, PSEUDO_PHY_ADDR, 25);
    *(rd_buf + 2) = _chipphyrd(cid, PSEUDO_PHY_ADDR, 26);
    *(rd_buf + 3) = _chipphyrd(cid, PSEUDO_PHY_ADDR, 27);
#ifdef BE_HOST
    *rd_buf = (*rd_buf >> 8)|(*rd_buf << 8);
    *(rd_buf + 1) = (*(rd_buf + 1) >> 8)|(*(rd_buf + 1) << 8);
    *(rd_buf + 2) = (*(rd_buf + 2) >> 8)|(*(rd_buf + 2) << 8);
    *(rd_buf + 3) = (*(rd_buf + 3) >> 8)|(*(rd_buf + 3) << 8);
#endif
    return 0;
}

int
robo_mdio_reg_read(uint8 cid, uint32 addr, void *data, int len)
{
#ifdef BE_HOST
    uint32  i;
#endif
    uint32      *data_ptr;
    uint64      data_rw;
    uint8           *data8_ptr;
#ifdef BE_HOST
    uint8       tmp;
#endif

    COMPILER_64_ZERO(data_rw);
    _soc_reg_read(cid, addr, (uint8 *)&data_rw, len);

    /* endian translation */
    data8_ptr = (uint8 *)&data_rw;
#ifdef BE_HOST
    if (len > 4) {
        for (i=0; i < 4; i++) {
            tmp = data8_ptr[i];
            data8_ptr[i] = data8_ptr[7-i];
            data8_ptr[7-i] = tmp;
        }
    } else {
        for (i = 0; i < 2; i++) {
            tmp = data8_ptr[i];
            data8_ptr[i] = data8_ptr[3-i];
            data8_ptr[3-i] = tmp;
        }
    }
#endif
    
    data_ptr = (uint32 *)data;
 
    if (len > 4) {
        sal_memcpy(data, data8_ptr, 8);
    } else {
        sal_memcpy(data, data8_ptr, 4);
    }
    return 0;
}

#endif
    
static struct spi_device_id _spi_id_table[] = {
    { BCM5324_PHYID_HIGH, BCM5324_PHYID_LOW },
    { BCM5324_A1_PHYID_HIGH, BCM5324_PHYID_LOW },
    { BROADCOM_PHYID_HIGH, BCM5338_PHYID_LOW },
    { BROADCOM_PHYID_HIGH, BCM5380_PHYID_LOW },
    { BROADCOM_PHYID_HIGH, BCM5388_PHYID_LOW },
    { BCM5396_PHYID_HIGH, BCM5396_PHYID_LOW },
    { BCM5389_PHYID_HIGH, BCM5389_PHYID_LOW },
    { BCM5398_PHYID_HIGH, BCM5398_PHYID_LOW },
    { BCM5325_PHYID_HIGH, BCM5325_PHYID_LOW },
    { BCM5348_PHYID_HIGH, BCM5348_PHYID_LOW },
    { BCM5395_PHYID_HIGH, BCM5395_PHYID_LOW },
    { BCM53242_PHYID_HIGH, BCM53242_PHYID_LOW },
    { BCM53262_PHYID_HIGH, BCM53262_PHYID_LOW },
    { BCM53115_PHYID_HIGH, BCM53115_PHYID_LOW },
    { BCM53118_PHYID_HIGH, BCM53118_PHYID_LOW },
    { BCM53280_PHYID_HIGH, BCM53280_PHYID_LOW },
    { BCM53101_PHYID_HIGH, BCM53101_PHYID_LOW },
    { BCM53125_PHYID_HIGH, BCM53125_PHYID_LOW },
    { BCM53128_PHYID_HIGH, BCM53128_PHYID_LOW },
    { BCM53600_PHYID_HIGH, BCM53600_PHYID_LOW },    
    { 0, 0 },
};

static int
_spi_device_valid_check(unsigned short phyidh)
{
    struct spi_device_id *_ids;

    for (_ids = _spi_id_table;
         _ids->phyid_high && _ids->phyid_low; _ids++) {
            if (_ids->phyid_high == phyidh) {
                return 0;
            }
    }

    /* No valid spi devices found */
    return 1;
}

static int
_spi_attach(void)
{
    int dev;
    vxbde_dev_t *vxd;
    bool eb_bus=0;
#ifndef MDC_MDIO_SUPPORT
    bool pseudo_phy=0;
#if defined(KEYSTONE)
    uint32 spi_freq = SPI_FREQ_20MHZ; /* The hightest supported SPI frequency, as of now 20MHz */
    uint32 spi_freq_override;         /* Used for user overrided SPI frequency */
    char *spi_freq_str = NULL;

    if (!(sih = si_kattach(SI_OSH))) {
        return -ENODEV;
    }

    /* Use GPIO 1, 0, 2, 3 and ss=0 to operate on SPI interface */
    if (!(robo = (void *)robo_attach((void *)sih,
                         (1<<1), (1<<0), (1<<2), (1<<3), 0))) {
        return -ENODEV;
    }
#ifdef ROBODVT
    else {
        /* For ROBO DVT, no need to probe the chips */
        return 0;
    }
#endif


#else 

    if (!(sbh = (void *)sb_kattach(BCM4710_DEVICE_ID, 0))) {
        return -ENODEV;
    }
    /* Use GPIO 2-5 to operate on SPI interface */
    if (!(robo = (void *)robo_attach(sbh,
                         (1<<2), (1<<3), (1<<4), (1<<5)))) {
        return -ENODEV;
    }
#endif
    
#endif

    for (dev = 0; (dev<MAX_SWITCH_DEVICES) &&
              (_switch_n_devices<MAX_SWITCH_DEVICES); dev++) {
        unsigned short phyidh = 0, phyidl = 0;
        struct spi_device_id *_ids;
        uint8 part_8 = 0, rev = 0;
        uint16 part_16 = 0;
        uint32 part_32 = 0;
        uint64 part_64;

        COMPILER_64_ZERO(part_64);
    /*
     * Read MII PHY Identifier registers for checking
     * the vendor/device id of robo chips.
     * Assume Port 0 MII page number (0x10) are fixed for all
     * robo chips.
     */
    
#ifdef MDC_MDIO_SUPPORT
        phyidh = _chipphyrd(0, 0, 2);
        printk("Pseudo Phy access mode.\n");
#else
        if (pseudo_phy == 1) {
        /* For psedo_phy access, only support one robo switch*/
            break;
        }

        /* first try EB_BUS */
        robo_rreg(robo, dev, 0x10, 0x04, (uint8 *)&phyidh, (uint)2);
#ifdef BE_HOST
        phyidh = (phyidh>>8)|(phyidh<<8);
#endif
        if (dev < 1) {
            /* decide which bus on the very first try */
            if (_spi_device_valid_check(phyidh)) {
                /*second try pseudo phy */
                robo_switch_bus(robo, 2);
                robo_rreg(robo, dev, 0x10, 0x04, (uint8 *)&phyidh, (uint)2);
#ifdef BE_HOST
                phyidh = (phyidh>>8)|(phyidh<<8);
#endif
                if (_spi_device_valid_check(phyidh)) {
                    /*then try SPI */
                    robo_switch_bus(robo, 1);
                    robo_rreg(robo, dev, 0x10, 0x04, (uint8 *)&phyidh, (uint)2);
#ifdef BE_HOST
                    phyidh = (phyidh>>8)|(phyidh<<8);
#endif
                } else {
                    pseudo_phy = 1;    
                }
            } else {
            /* reg_access via EB bus*/
                eb_bus = 1;
            }
        }
#endif

    /* Check if valid phyid */
    if (_spi_device_valid_check(phyidh)) {
        
        /* printk("found %d robo device(s).\n", _robo_devices);*/
        break;
    } else {
#ifdef MDC_MDIO_SUPPORT
        phyidl = _chipphyrd(0, 0, 3);
#else
        robo_rreg(robo, dev, 0x10, 0x06, (uint8 *)&phyidl, (uint)2);
#ifdef BE_HOST
        phyidl = (phyidl >> 8) | (phyidl << 8);
#endif
#endif

        if ((phyidl == 0xFFFF) || (phyidl == 0x0)) {
            
            /* printk("found %d robo device(s).\n", _robo_devices);*/
            break;
        }
    }

#ifdef MDC_MDIO_SUPPORT
        if ((phyidh == BCM5348_PHYID_HIGH) && \
            ((phyidl & 0xFFF0) == BCM5348_PHYID_LOW)) {
            robo_mdio_reg_read(0,0x388, &part_32, 1);
            part_8 = (part_32 & 0xFF);
            part_16 = (part_32 & 0xFFFF);
        } else if (((phyidh == BCM53242_PHYID_HIGH) && \
                ((phyidl & 0xFFF0) == BCM53242_PHYID_LOW)) || \
                ((phyidh == BCM53262_PHYID_HIGH) && \
                ((phyidl & 0xFFF0) == BCM53262_PHYID_LOW))){
                part_8 = 0;
                part_16 = part_8;
                rev = 0;
        } else {
            /* Register Model ID len = 4 for BCM53115, BCM53118 */
            if (((phyidh == BCM53115_PHYID_HIGH) && \
                ((phyidl & 0xFFF0) == BCM53115_PHYID_LOW)) || \
                ((phyidh == BCM53118_PHYID_HIGH) && \
                ((phyidl & 0xFFF0) == BCM53118_PHYID_LOW)) || \
                ((phyidh == BCM53101_PHYID_HIGH) && \
                ((phyidl & 0xFFF0) == BCM53101_PHYID_LOW)) || \
                ((phyidh == BCM53125_PHYID_HIGH) && \
                ((phyidl & 0xFFF0) == BCM53125_PHYID_LOW)) || \
                ((phyidh == BCM53128_PHYID_HIGH) && \
                ((phyidl & 0xFFF0) == BCM53128_PHYID_LOW))) {
                robo_mdio_reg_read(0,0x230, &part_32, 4);
#ifdef BE_HOST
                part_32 = (((part_32 >> 24) & 0x000000FF)|((part_32 >> 8) & 0x0000FF00)|
                               ((part_32 << 8) & 0x00FF0000)|((part_32 << 24) & 0xFF000000));
#endif     
                part_16 = (part_32 & 0x0000FFFF);
            }  else if (((phyidh == BCM53280_PHYID_HIGH) && 
                ((phyidl & 0xFFF0) == BCM53280_PHYID_LOW)) ||
                ((phyidh == BCM53600_PHYID_HIGH) &&
                ((phyidl & 0xFFF0) == BCM53600_PHYID_LOW))
                ){
                _soc_reg_read(0, 0xe8, (uint8 *)&part_64, 8);
                part_8 =  *(uint8 *)&part_64;
                part_16 = part_8;
            } else {
                robo_mdio_reg_read(0,0x230, &part_32, 1);
                part_16 = (part_32 & 0xFFFF);
            }

            robo_mdio_reg_read(0,0x240, &part_32, 1);
            rev = (part_32 & 0xFF);
        }
#else
        if ((phyidh == BCM5348_PHYID_HIGH) && \
            ((phyidl & 0xFFF0) == BCM5348_PHYID_LOW)) {
            robo_rreg(robo, dev, 0x3, 0x88, (uint8 *)&part_8, (uint)1);
            part_16 = part_8;
        } else if (((phyidh == BCM53242_PHYID_HIGH) && \
                ((phyidl & 0xFFF0) == BCM53242_PHYID_LOW)) || \
                ((phyidh == BCM53262_PHYID_HIGH) && \
                ((phyidl & 0xFFF0) == BCM53262_PHYID_LOW))){
                part_8 = 0;
                part_16 = part_8;
                rev = 0;
        } else {
            /* Register Model ID len = 4 for BCM53115, BCM53118 */
            if (((phyidh == BCM53115_PHYID_HIGH) && \
                ((phyidl & 0xFFF0) == BCM53115_PHYID_LOW)) || \
                ((phyidh == BCM53118_PHYID_HIGH) && \
                ((phyidl & 0xFFF0) == BCM53118_PHYID_LOW)) || \
                ((phyidh == BCM53101_PHYID_HIGH) && \
                ((phyidl & 0xFFF0) == BCM53101_PHYID_LOW)) || \
                ((phyidh == BCM53125_PHYID_HIGH) && \
                ((phyidl & 0xFFF0) == BCM53125_PHYID_LOW)) || \
                ((phyidh == BCM53128_PHYID_HIGH) && \
                ((phyidl & 0xFFF0) == BCM53128_PHYID_LOW))) {
                robo_rreg(robo, dev, 0x2, 0x30, (uint8 *)&part_32, (uint)4);
#ifdef BE_HOST
                part_32 = (((part_32 >> 24) & 0x000000FF)|((part_32 >> 8) & 0x0000FF00)|
                               ((part_32 << 8) & 0x00FF0000)|((part_32 << 24) & 0xFF000000));
#endif
                part_16 = (part_32 & 0x0000FFFF);
                robo_rreg(robo, dev, 0x2, 0x40, (uint8 *)&rev, (uint)1);        
            }else if (((phyidh == BCM53280_PHYID_HIGH) && 
                ((phyidl & 0xFFF0) == BCM53280_PHYID_LOW)) ||
                ((phyidh == BCM53600_PHYID_HIGH) &&
                ((phyidl & 0xFFF0) == BCM53600_PHYID_LOW))
                ){
                robo_rreg(robo, dev, 0x0, 0xe8, (uint8 *)&part_64, (uint)8); 
                part_8 =  *(uint8 *)&part_64;
                part_16 = part_8;               
            } else {
                robo_rreg(robo, dev, 0x2, 0x30, (uint8 *)&part_8, (uint)1);
                part_16 = part_8;
                robo_rreg(robo, dev, 0x2, 0x40, (uint8 *)&rev, (uint)1);        
            }
        }
#endif

    
    /* printk("found robo device with %d:%04x:%04x:%04x:%02x\n",
            dev, phyidh, phyidl, part_16, rev); */
        robo_select_device(robo, phyidh, phyidl);

        for (_ids = _spi_id_table;
             _ids->phyid_high && _ids->phyid_low; _ids++) {

            if ((_ids->phyid_high == phyidh) &&    /* exclude revision */
                (_ids->phyid_low == (phyidl & 0xFFF0))) {

                /* Match supported chips */
                vxd = _devices + _switch_n_devices++;

                vxd->dev_type = (BDE_SPI_DEV_TYPE | BDE_SWITCH_DEV_TYPE);
                if (eb_bus) {
                    vxd->dev_type |= BDE_EB_DEV_TYPE;
                }
                vxd->spi_dev.cid = dev;
                vxd->spi_dev.part = part_16;
                vxd->spi_dev.rev = rev;
                vxd->spi_dev.robo = robo;
                vxd->spi_dev.phyid_high = phyidh;
                vxd->spi_dev.phyid_low = phyidl;
                vxd->bde_dev.device = phyidl & 0xFFF0;
                vxd->bde_dev.rev = phyidl & 0xF;
                if ((phyidh == BCM5396_PHYID_HIGH) && \
                        (phyidl == BCM5396_PHYID_LOW)) {
                    vxd->bde_dev.device = part_16;
                    vxd->bde_dev.rev = rev;
                }
                if ((phyidh == BCM5398_PHYID_HIGH) && \
                    (phyidl == BCM5398_PHYID_LOW)) {
                    vxd->bde_dev.device = part_16;
                    vxd->bde_dev.rev = rev;
                }
                if ((phyidh == BCM5324_A1_PHYID_HIGH) && \
                    (phyidl == BCM5324_PHYID_LOW)) {
                    vxd->bde_dev.rev = 0x1;
                }
                if ((phyidh == BCM5325_PHYID_HIGH) && \
                    (phyidl == BCM5325_PHYID_LOW)) {
                    vxd->bde_dev.rev = 0x1;
                }
                 if ((phyidh == BCM5348_PHYID_HIGH) && \
                    ((phyidl & 0xFFF0) == BCM5348_PHYID_LOW)) {
                    vxd->bde_dev.device = part_16;
                }
                if (((phyidh == BCM53280_PHYID_HIGH) && \
                    ((phyidl & 0xFFF0) == BCM53280_PHYID_LOW)) ||
                    ((phyidh == BCM53600_PHYID_HIGH) &&
                    ((phyidl & 0xFFF0) == BCM53600_PHYID_LOW))
                ){
                    vxd->bde_dev.device = part_16 | (phyidl & 0xFFF0);
                    part_8 = (uint8)(phyidl&0xF);
                    vxd->bde_dev.rev =part_8 ;                    
                }
                if (((phyidh == BCM53128_PHYID_HIGH) && \
                    ((phyidl & 0xFFF0) == BCM53128_PHYID_LOW)) || 
                    ((phyidh == BCM53125_PHYID_HIGH) && \
                    ((phyidl & 0xFFF0) == BCM53125_PHYID_LOW)) ||
                    ((phyidh == BCM53101_PHYID_HIGH) && \
                    ((phyidl & 0xFFF0) == BCM53101_PHYID_LOW))) {
                    vxd->bde_dev.rev = rev;
                }
                vxd->bde_dev.base_address = (sal_vaddr_t)NULL;
                vxd->iLine = 0;
                vxd->iPin = 0;
                vxd->flags = 0;
                _robo_devices++;
                _n_devices++;
                break;
            }
        }
#ifndef MDC_MDIO_SUPPORT 
#if defined(KEYSTONE) 
        /* 
         * Thunderbolt and Lotus can support SPI frequency up to 20MHz 
         */ 
        if (((phyidh == BCM53280_PHYID_HIGH) && ((phyidl & 0xFFF0) == BCM53280_PHYID_LOW)) || 
            ((phyidh == BCM53101_PHYID_HIGH) && ((phyidl & 0xFFF0) == BCM53101_PHYID_LOW)) ||
            ((phyidh == BCM53125_PHYID_HIGH) && ((phyidl & 0xFFF0) == BCM53125_PHYID_LOW)) ||
            ((phyidh == BCM53128_PHYID_HIGH) && ((phyidl & 0xFFF0) == BCM53128_PHYID_LOW))) { 
            if (SPI_FREQ_20MHZ < spi_freq) { 
                spi_freq = SPI_FREQ_20MHZ; 
            } 
        } else { 
            spi_freq = SPI_FREQ_DEFAULT; 
        } 

        /* 
         * Override the SPI frequency from user configuration
         */ 
        if ((spi_freq_str = sal_config_get("spi_freq_override")) != NULL) {
            if ((spi_freq_override = sal_ctoi(spi_freq_str, 0)) != 0) {
                spi_freq = spi_freq_override;
            }
        }
#endif 
#endif 
    }

#ifndef MDC_MDIO_SUPPORT 
#if defined(KEYSTONE) 
    /* The underlying chip can support the SPI frequency higher than default (2MHz) */ 
    if (spi_freq != SPI_FREQ_DEFAULT) { 
        chipc_spi_set_freq(robo, 0, spi_freq); 
    } 
#endif 
#endif 

    return _robo_devices;
}
#endif

#ifdef   BCM_ICS
int
vxbde_create(vxbde_bus_t *bus, 
         ibde_t **bde)
{
    if (_n_devices == 0) {
        vxbde_dev_t *vxd;
        uint32  dev_rev_id = 0x0;
    _bus = *bus;

        bcm_bde_soc_cm_memory_base = _bus.base_addr_start;
        vxd = _devices + _n_devices++;
        _switch_n_devices++;
      
        vxd->bde_dev.base_address = _bus.base_addr_start;
        vxd->iLine = _bus.int_line;
        vxd->iPin  = 0;
        vxd->flags  = 0;
        if (_bus.base_addr_start)
            _bus.base_addr_start += PCI_SOC_MEM_WINSZ;

        vxd->dev_type = BDE_ICS_DEV_TYPE | BDE_SWITCH_DEV_TYPE;

        dev_rev_id = _read(0, 0x178);  /* CMIC_DEV_REV_ID */
        vxd->bde_dev.device = dev_rev_id & 0xFFFF;
        vxd->bde_dev.rev = (dev_rev_id >> 16) & 0xFF;
    }
    *bde = &_ibde;

#ifdef INCLUDE_RCPU
    /*
     * Check if devices are connected to bus in EB mode.
     */
    if (sal_config_get("eb_probe") != NULL) {
        vxbde_eb_bus_probe(bus, bde);
    }
#endif /* INCLUDE_RCPU */

    return 0;  
}
#else

#define PLX9656_LAS0_BA         0x00000004  /* LAS0 Local Base Address Remap   */
#define PLX9656_LAS1_BA         0x000000f4  /* LAS1 Local Base Address Remap   */
#define PLX9656_LAS_EN          0x01000000  /* Space Enable bit                */

#define PLX8608_DEVICE_ID       0x8608      /* PLX PEX8608 device id           */
#define PLX8608_DEV_CTRL_REG    0x70        /* PLX PEX8608 control register    */

STATIC int 
_plx_las_baroff_get(pci_dev_t *dev)
{
    uint32      local_config_bar;
    int         baroff = -1;

    /* Get base of local configuration registers */
    local_config_bar = pci_config_getw(dev, PCI_CONF_BAR0);
    if (local_config_bar) {
        /* 
         * Make sure LAS0BA or LAS1BA is enabled before returning
         * BAR that will be used to access the Local Bus
         */
        if ((*(uint32 *)(local_config_bar + PLX9656_LAS0_BA)) & PLX9656_LAS_EN) {
            baroff = PCI_CONF_BAR2;
        } else if ((*(uint32 *)(local_config_bar + PLX9656_LAS1_BA)) & 
                   PLX9656_LAS_EN) {
            baroff = PCI_CONF_BAR3;
        } 
    }
    return baroff;
}


static int
_setup(pci_dev_t *dev, 
       uint16 pciVenID, 
       uint16 pciDevID,
       uint8 pciRevID)
{
    uint32 flags = 0;
    vxbde_dev_t *vxd = 0;
    char    pll_bypass[20];
    char    pci_soccfg0[20];
    char    pci_soccfg1[20];
    char    pci2eb_override[20];
    char    *val_str;
    int     cap_base;
    uint32  rval, size_code;
    int     baroff = PCI_CONF_BAR0;
#ifndef METROCORE
    int     done = 0;
#endif

#ifdef METROCORE
    if (pciVenID != SANDBURST_VENDOR_ID) {
        return 0;
    }
    flags |= BDE_SWITCH_DEV_TYPE;
    vxd = _devices + _switch_n_devices++;
    _n_devices++;
#else 
    /* Common code for SBX and XGS boards */
    if ((pciVenID != SANDBURST_VENDOR_ID) &&
        (pciVenID != PLX_VENDOR_ID) &&
        (pciVenID != BROADCOM_VENDOR_ID)) {
        return 0;
    }

    /*
     * The presence of the PLX bridge indicates this is
     * a Metrocore GTO/XMC platform.
     * The PLX bridge itself won't be part of _devices[].
     * It's the devices behind PLX can be part of _devices[]
     */
    if (pciVenID == PLX_VENDOR_ID) {

        /* Fix up max payload on all PLX PEX8608 ports */        
        if (pciDevID == PLX8608_DEVICE_ID) {
            rval = pci_config_getw(dev, PLX8608_DEV_CTRL_REG);
            rval = (rval & ~(7<<5)) | (2 << 5);
            pci_config_putw(dev, PLX8608_DEV_CTRL_REG, rval);
            return 0;
        }

        if ((pciDevID == PLX9656_DEVICE_ID) ||
            (pciDevID == PLX9056_DEVICE_ID)) {
            baroff = _plx_las_baroff_get(dev);
            if (baroff == -1) {
                printf("No Local Address Space enabled in PLX\n");
                return 0;
            }
            vxd = &plx_vxd;
            num_plx++;
            /* Adjust bus for METROCORE GTO */
            _bus.be_pio    = 0;
            _bus.be_packet = 0;
            _bus.be_other  = 0;
        }
    } else if (pciVenID == BROADCOM_VENDOR_ID &&
               ((pciDevID & 0xFF00) != 0x5600) &&
               ((pciDevID & 0xF000) != 0xc000) &&
               ((pciDevID & 0xFFF0) != 0x0230) &&
               ((pciDevID & 0xFFFF) != 0x0732) &&
               ((pciDevID & 0xF000) != 0x8000) &&
               ((pciDevID & 0xF000) != 0xb000)) {
        /* don't want to expose non 56XX/53XXX devices */
#ifdef KEYSTONE
        if ((pciDevID == BCM47XX_ENET_ID) || (pciDevID == BCM5300X_GMAC_ID)) {
#else /* !KEYSTONE */
        if (pciDevID == BCM47XX_ENET_ID) {
#endif /* KEYSTONE */
        /*
         * BCM5836/4704 Use ENET MAC 1 
         */
        if (pciDevID == BCM47XX_ENET_ID) {
            if (_first_mac == 0) {
                _first_mac++;
                return 0;
            }
        }
#ifdef KEYSTONE
        if (pciDevID == BCM5300X_GMAC_ID) {
            _first_mac++ ;
            coreflags = (uint32)getintvar(NULL, "coreflags");
            if (coreflags == 0) {
                /* 
                 * The ROBO switch is connnected with GMAC core 0
                 * in the old ROBO boards.
                 * So set the bit of CORE_GMAC1 for Vxworks BSP.
                 */
                coreflags = CORE_GMAC1 | CORE_PCIE0 | CORE_PCIE1;
            }
            if ((coreflags & CORE_GMAC0) &&
                (_first_mac == 1)) {
                return 0;
            }
            if ((coreflags & CORE_GMAC1) &&
                (_first_mac == 2)) {
                return 0;
            }
        }   
#endif /* KEYSTONE */
        
        flags |= BDE_ETHER_DEV_TYPE;
        vxd = _devices + MAX_SWITCH_DEVICES + _ether_n_devices++;
        _n_devices++;
        done = 1;
        } else {
            return 0;
        }
    } else {
        flags |= BDE_SWITCH_DEV_TYPE;
        vxd = _devices + _switch_n_devices++;
        _n_devices++;
    }
#endif /* METROCORE */

    sal_sprintf(pll_bypass, "pll_bypass.%d", _n_devices);
    sal_sprintf(pci_soccfg0, "pci_conf_soccfg0.%d", _n_devices);
    sal_sprintf(pci_soccfg1, "pci_conf_soccfg1.%d", _n_devices);
    sal_sprintf(pci2eb_override, "pci2eb_override.%d", _n_devices);
  
    flags |= BDE_PCI_DEV_TYPE;
    vxd->bde_dev.device = pciDevID;
    vxd->bde_dev.rev = pciRevID;
    vxd->bde_dev.base_address = 0; /* read back */
    vxd->pci_dev = *dev;
    vxd->dev_type = flags;

    /* Configure PCI */

    /* Write control word (turns on parity detect) */
    pci_config_putw(dev, PCI_CONF_COMMAND, (PCI_CONF_COMMAND_BM |
                     PCI_CONF_COMMAND_MS |
                     PCI_CONF_COMMAND_PERR));

#if defined(KEYSTONE)
    if ((dev->busNo == PCIE_PORT0_HB_BUS) && \
        (dev->devNo == 1) && (dev->funcNo == 0)) {
        _bus.base_addr_start = SI_PCI0_MEM;
        _bus.int_line = 3;
    }

    if ((dev->busNo == PCIE_PORT1_HB_BUS) && \
        (dev->devNo == 1) && (dev->funcNo == 0)) {
        _bus.base_addr_start = SI_PCI1_MEM;
        _bus.int_line = 4;
        pcie1_device_exist = 1;
    }
#endif
    if (_bus.base_addr_start) {
        uint32 tmp;

        /*
         * The standard procedure to determine device window size is to
         * write 0xffffffff to BASE0, read it back, and see how many
         * LSBs are hardwired to zero.  In our case, we would get
         * 0xffff0000 indicating a window size of 64kB.
         *
         * While the window size could be assumed 64kB, we must still do
         * the standard write and read because some PCI bridges (most
         * notably the Quickturn Speed Bridge) observe these
         * transactions to record the window size internally.
         */
        
        pci_config_putw(dev, baroff, 0xffffffff);
        rval = pci_config_getw(dev, baroff);

        pci_config_putw(dev, baroff, _bus.base_addr_start);
        if ((rval & PCI_CONF_BAR_TYPE_MASK) == PCI_CONF_BAR_TYPE_64B) {
            pci_config_putw(dev, PCI_CONF_BAR1, 0);
        }
        
        if (!(flags & BDE_ETHER_DEV_TYPE)) {
            tmp = pci_config_getw(dev, PCI_CONF_INTERRUPT_LINE);
            tmp = (tmp & ~0xff) | _bus.int_line;
            
            pci_config_putw(dev, PCI_CONF_INTERRUPT_LINE, tmp);
        }

        /* should read window size from device */
        _bus.base_addr_start += PCI_SOC_MEM_WINSZ;
    }
#if defined(NSX) || defined(METROCORE)
    else {
        uint32 tmp;
        
        tmp = pci_config_getw(dev, PCI_CONF_INTERRUPT_LINE);
        tmp = (tmp & ~0xff) | _bus.int_line;
        
        pci_config_putw(dev, PCI_CONF_INTERRUPT_LINE, tmp);
    }
#endif /* NSX */
    
#if defined(KEYSTONE)
    if ((dev->busNo == PCIE_PORT0_HB_BUS) && \
        (dev->devNo == 1) && (dev->funcNo == 0)) {
        vxd->bde_dev.base_address = 0xa8000000;
    }
    if ((dev->busNo == PCIE_PORT1_HB_BUS) && \
        (dev->devNo == 1) && (dev->funcNo == 0)) {
        vxd->bde_dev.base_address = 0xe0000000;
    }

    if ((pciDevID == BCM5300X_GMAC_ID) && (dev->busNo == 0)) {
        /* GMAC core in SI Bus */
        vxd->bde_dev.base_address = 0xb8000000;
    }
#else
    vxd->bde_dev.base_address = (sal_vaddr_t) 
                                ((uint32*)(pci_config_getw(dev, baroff) & PCI_CONF_BAR_MASK));
#if defined(VENDOR_BROADCOM) && defined(SHADOW_SVK)
    if (num_plx) {
        sal_vaddr_t base_address;
        /* Enable Interrupts */
        base_address = (sal_vaddr_t) 
                   ((uint32*)(pci_config_getw(dev, PCI_CONF_BAR0) & PCI_CONF_BAR_MASK));
        *(uint32 *)(base_address + 0x68) |= 0x00080000;
    }
#endif
#endif
    vxd->flags = 0x0;
    if(flags & BDE_ETHER_DEV_TYPE){
#if defined(KEYSTONE)
        if (_first_mac == 1) {
            /* GMAC core 0 */
            vxd->iLine = 0x2;
        } else {
            /* GMAC core 1 */
            vxd->iLine = 0x1;
        }
#else /* !KEYSTONE */
        vxd->iLine = 0x2;
#endif /* KEYSTONE */
        vxd->iPin = 0x0;
        /*
         * Have at it.
         * ETHER_DEV_TYPE is for the internal emac in BCM5836/4704.
         * It is just a virtual/pseudo PCI device in PCI configuration space.
         */
        return 0;
    } else
        vxd->iLine = pci_config_getw(dev, PCI_CONF_INTERRUPT_LINE) >> 0 & 0xff;
        vxd->iPin  = pci_config_getw(dev, PCI_CONF_INTERRUPT_LINE) >> 8 & 0xff;
#ifndef METROCORE
        if (pciVenID == SANDBURST_VENDOR_ID || pciVenID == PLX_VENDOR_ID) {
            vxd->iLine = FPGA_IRQ; /* all interrupts are from FPGA line */
            vxd->iPin  = 0;
        } 
#endif

    cap_base = pci_config_getw(dev, PCI_CONF_CAPABILITY_PTR) & 0xff;
    if (!cap_base) {
        /*
         * Set # retries to infinite.  Otherwise other devices using the bus
         * may monopolize it long enough for us to time out.
         */

        pci_config_putw(dev, PCI_CONF_TRDY_TO, 0x0080);

        /*
         * Optionally enable PLL bypass in reserved register.
         */
        if ((val_str = sal_config_get(pll_bypass)) != NULL) {
            if (_shr_ctoi(val_str)) {
                pci_config_putw(dev, PCI_CONF_PLL_BYPASS, 0x2);
                if (SAL_BOOT_QUICKTURN) {
                    sal_usleep(100000);
                }
            }
        }

        /*
         * Optionally configure clocks etc. in reserved registers.
         * These registers may contain multiple configuration bits.
         */
        if ((val_str = sal_config_get(pci_soccfg0)) != NULL) {
            pci_config_putw(dev, PCI_CONF_SOCCFG0, _shr_ctoi(val_str));
            if (SAL_BOOT_QUICKTURN) {
                sal_usleep(100000);
            }
        }
        if ((val_str = sal_config_get(pci_soccfg1)) != NULL) {

            pci_config_putw(dev, PCI_CONF_SOCCFG1, _shr_ctoi(val_str));
            if (SAL_BOOT_QUICKTURN) {
                sal_usleep(100000);
            }
        }
    } else {
        while (cap_base) {
            rval = pci_config_getw(dev, cap_base);
            /*
             * PCIE spec 1.1 section 7.8.1
             * PCI-Express capability ID = 0x10
             * Locate PCI-E capability structure
             */
            if ((rval & 0xff) != 0x10) {
                cap_base = (rval >> 8) & 0xff;
                continue;
            }
            /*
             * PCIE spec 1.1 section 7.8.1
             * offset 0x04 : Device capabilities register
             * bit 2-0 : Max_payload_size supported
             * 000 - 128 Bytes max payload size
             * 001 - 256 Bytes max payload size
             * 010 - 512 Bytes max payload size
             * 011 - 1024 Bytes max payload size
             * 100 - 2048 Bytes max payload size
             * 101 - 4096 Bytes max payload size
             */
            size_code = pci_config_getw(dev, cap_base + 0x04) & 0x07;
#if 1
            if (size_code > 1) {
                /*
                 * Restrict Max_payload_size to 256
                 */
                size_code = 1;
            }
            /*
             * PCIE spec 1.1 section 7.8.4
             * offset 0x08 : Device control register
             * bit 4-4 : Enable relaxed ordering (Disable)
             * bit 5-7 : Max_payload_size        (256)
             * bit 12-14 : Max_Read_Request_Size (256)
             */
            rval = pci_config_getw(dev, cap_base + 0x08);

            /* Max_Payload_Size = 1 (256 bytes) */
            rval &= ~0x00e0;
            rval |= size_code << 5;

            /* Max_Read_Request_Size = 1 (256 bytes) */
            rval &= ~0x7000;
            rval |= size_code << 12;
#else
            /*
             * Restrict Max_payload_size to 128
             */
            size_code = 0;

            /*
             * PCIE spec 1.1 section 7.8.4
             * offset 0x08 : Device control register
             * bit 4-4 : Enable relaxed ordering (Disable)
             * bit 5-7 : Max_payload_size        (256)
             * bit 12-14 : Max_Read_Request_Size (256)
             */
            rval = pci_config_getw(dev, cap_base + 0x08);

            /* Max_Payload_Size = 0 (128 bytes) */
            rval &= ~0x00e0;
            rval |= size_code << 5;

            /* Max_Read_Request_Size = 0 (128 bytes) */
            rval &= ~0x7000;
            rval |= size_code << 12;
#endif

            rval &= ~0x0010; /* Enable Relex Ordering = 0 (disabled) */
            pci_config_putw(dev, cap_base + 0x08, rval);
            break;
        }
    }

    /* Have at it */
    return 0;
}

#if defined(KEYSTONE)
/* PCI-E  host Bridge */
#define BCM53000PCIE0_REG_BASE 0xb8005400
#define BCM53000PCIE1_REG_BASE 0xb800e400
#define BCM53000PCIE_DEV_CAP_REG  0xd4
#define BCM53000PCIE_DEV_CTRL_REG 0xd8
#define BCM53000PCIE_MAX_PAYLOAD_MASK  0x7

#define BCM53000PCIE0_SPROM_SHADOW_OFF_14 0xb800581c
#define BCM53000PCIE1_SPROM_SHADOW_OFF_14 0xb800e81c
#define BCM53000PCIE1_SPROM_SHADOW_MAX_PAYLOAD_MASK 0xe000
#define BCM53000PCIE1_SPROM_SHADOW_MAX_PAYLOAD_OFFSET 13

#define MAX_PAYLOAD_256B       (1 << 5)
#define MAX_READ_REQ_256B      (1 << 12)

static void
fixup_pcie(int port)
{
    uint32 tmp;
    uint16 tmp16, tmp161;
    uint32 base;
    uint32 rom_data;
    uint32 mask;
    uint32 offset;
    /* configure the PCIE cap: Max payload size: 256, Max Read
     * Request size: 256, disabling relax ordering.
     * Writes to the PCIE capability device control register
     */

    if ((port == 0) && (coreflags & CORE_PCIE0)) {
        base = BCM53000PCIE0_REG_BASE;
        rom_data = BCM53000PCIE0_SPROM_SHADOW_OFF_14;
    } else if ((port == 1) && (coreflags & CORE_PCIE1)) {
        base = BCM53000PCIE1_REG_BASE;
        rom_data = BCM53000PCIE1_SPROM_SHADOW_OFF_14;
    } else {
        return;
    }

    tmp = *((uint32 *)(base+BCM53000PCIE_DEV_CAP_REG));
    if ((tmp & BCM53000PCIE_MAX_PAYLOAD_MASK) != 1) {
        tmp16 = *((uint16 *)rom_data);
        mask = BCM53000PCIE1_SPROM_SHADOW_MAX_PAYLOAD_MASK;
        offset = BCM53000PCIE1_SPROM_SHADOW_MAX_PAYLOAD_OFFSET;
        if (((tmp16 & mask) >> offset) != 0x1) {
            tmp161 = (tmp16 & ~mask) | (0x1 << offset);
            *((uint16 *)rom_data) = tmp161;
        }
    }

    tmp16 = *((uint16 *)(base+BCM53000PCIE_DEV_CTRL_REG));
    if (!(tmp16 & MAX_PAYLOAD_256B) || !(tmp16 & MAX_READ_REQ_256B)) {
        tmp161 = tmp16 | MAX_PAYLOAD_256B | MAX_READ_REQ_256B;
        *((uint16 *)(base+BCM53000PCIE_DEV_CTRL_REG)) = tmp161;
    }

    return;
}
#endif 

static int
_soc_dev_probe(pci_dev_t *dev, 
       uint16 pciVenID, 
       uint16 pciDevID,
       uint8 pciRevID)
{
#ifdef METROCORE
    if (pciVenID != SANDBURST_VENDOR_ID) {
        return 0;
    }
#else
    if (pciVenID != BROADCOM_VENDOR_ID) {
        return 0;
    }

    /* Expose 56XX/53XXX and some non 56XX/53XXX devices */
    if(((pciDevID & 0xFF00) != 0x5600) &&
       ((pciDevID & 0xF000) != 0xc000) &&
       ((pciDevID & 0xFFF0) != 0x0230) &&
       ((pciDevID & 0xF000) != 0xb000) &&
       ((pciDevID & 0xF000) != 0x8000)) {
        return 0;
    }
#endif

    if (_pri_bus_no ==  -1) {
        _pri_bus_no = dev->busNo;
    }

    if (dev->busNo == _pri_bus_no) {
        _n_pri_devices++;
    } else {
        _n_sec_devices++;
    }
  
    return 0;
}

static void
_adjust_bus_base_addr_start()
{
    if (!_bus.base_addr_start) {
        return;
    }
    pci_device_iter(_soc_dev_probe);
    if (_n_sec_devices) {
        if (_bus.base_addr_start)
            _bus.base_addr_start -= PCI_SOC_MEM_WINSZ * _n_pri_devices;
    }
    bcm_bde_soc_cm_memory_base = _bus.base_addr_start;

    /* printk("_n_pri_devices = %d _n_sec_devices = %d _bus.base_addr_start = %08x\n", _n_pri_devices, _n_sec_devices, _bus.base_addr_start);*/
}
    
/*
 * SBX platform has both PCI- and local bus-attached devices
 * The local bus devices have fixed address ranges (and don't
 * support or require DMA), but are otherwise the same as PCI devices
 */
#define FPGA_PHYS               0x100E0000
#define BME_PHYS                0x100C0000
#define SE_PHYS                 0x100D0000

/*
 * Please refer to "Supervisor Fabric Module (SFM) Specification"
 * page 23 for the following registers.
 */
#define FPGA_LC_POWER_DISABLE_OFFSET             0x4
#define FPGA_LC_POWER_DISABLE_ENABLE_ALL_MASK    0x1e

#define FPGA_LC_POWER_RESET_OFFSET               0x5
#define FPGA_LC_POWER_RESET_ENABLE_ALL_MASK      0x1e

#define FPGA_SW_SFM_MASTER_MODE_OFFSET           0x14
#define FPGA_SW_SFM_MASTER_MODE_ENABLE_MASK      0x10

#ifdef METROCORE
static int
probe_metrocore_local_bus(void)
{
    vxbde_dev_t         *vxd;
    uint32              dev_rev_id;
    volatile uint8_t    *fpga;

    /*
     * Write the FPGA on the fabric card, to let metrocore
     * line cards out of reset.  We actually don't bother to determine whether
     * the card is a line card or a fabric card because when we do
     * this on the line cards, it has no effect.
     */
    fpga = (uint8_t *)FPGA_PHYS;

    fpga[FPGA_SW_SFM_MASTER_MODE_OFFSET]
        |= FPGA_SW_SFM_MASTER_MODE_ENABLE_MASK;
    fpga[FPGA_LC_POWER_DISABLE_OFFSET]
        |= FPGA_LC_POWER_DISABLE_ENABLE_ALL_MASK;
    fpga[FPGA_LC_POWER_RESET_OFFSET]
        |= FPGA_LC_POWER_RESET_ENABLE_ALL_MASK;

    /*
     * Metrocore Local Bus device: BME 3200
     */
    vxd = _devices + _n_devices;
    _n_devices++;
    _switch_n_devices++;

    vxd->bde_dev.base_address = (sal_vaddr_t) BME_PHYS;
    vxd->dev_type = BDE_EB_DEV_TYPE | BDE_SWITCH_DEV_TYPE;

    dev_rev_id = *(uint32 *)vxd->bde_dev.base_address;

    vxd->bde_dev.device = (dev_rev_id >> 16) & 0xFFFF;
    vxd->bde_dev.rev = dev_rev_id & 0xFFFF;
    vxd->iLine = FPGA_IRQ;
    vxd->iPin  = 0;

    if (vxd->bde_dev.device != BME3200_DEVICE_ID) {
        /* printk("probe_metrocore_local_bus: wrong BME type: "
                "0x%x (vs 0x%x)\n", vxd->bde_dev.device, BME3200_DEVICE_ID); */
        return -1;
    }

    /*
     * Metrocore Local Bus device: SE 3200
     */
    vxd = _devices + _n_devices;
    _n_devices++;
    _switch_n_devices++;

    vxd->bde_dev.base_address = (sal_vaddr_t) SE_PHYS;
    vxd->dev_type = BDE_EB_DEV_TYPE | BDE_SWITCH_DEV_TYPE;

    dev_rev_id = *(uint32 *)vxd->bde_dev.base_address;

    vxd->bde_dev.device = (dev_rev_id >> 16) & 0xFFFF;
    vxd->bde_dev.rev = dev_rev_id & 0xFFFF;
    vxd->iLine = FPGA_IRQ;
    vxd->iPin  = 0;

    if (vxd->bde_dev.device != BME3200_DEVICE_ID) {
        /* printk("probe_metrocore_local_bus: wrong SE type: "
                "0x%x (vs 0x%x)\n", vxd->bde_dev.device, BME3200_DEVICE_ID); */
        return -1;
    }
    return 0;
}
#endif /* METROCORE */

/*
 * The address space behind the PLX9656 PCI-to-LOCAL bridge has already
 * been allocated by the system (vxworks) during the bus enumeration.
 *
 * Devices which have registers mapped in the local bus space:
 * BME9600 IO space start at PL0_OFFSET, PL0_SIZE(64KB).
 * FPGA IO space is right next, PL0_OFFSET + PL0_SIZE
 
 *
 */
#if defined(VENDOR_BROADCOM) && defined(SHADOW_SVK)
 /* [VENDOR_BROADCOM]
 *  SHADOW in EB3 or VLI (non-PCIE mode). Registers are mapped
 *  in the local address space.
 */   
#define DEV_REG_BASE_OFFSET     0x800000          /* Register base local bus offset */
#define DEV_REG_DEVID           0x178             /* Device ID is CMID devid */
#else
#define DEV_REG_BASE_OFFSET     PL0_OFFSET /* Polaris register base */
#define DEV_REG_DEVID           0          /* Device ID is first register */
#endif

static int
probe_plx_local_bus(void)
{
    vxbde_dev_t *vxd;
    uint32 dev_rev_id;

    if (num_plx == 0) {
            printf("Not found PLX 9656/9056 chip\n");
        return -1;
    }
    if (num_plx > 1) {
            printf("There's more than one PLX 9656/9056 chip\n");
        return -1;
    }

    /*
     * PLX Local Bus device: BME 9600
     */
    vxd = _devices + _n_devices;
    _n_devices++;
    _switch_n_devices++;

#if defined (VENDOR_BROADCOM) && defined(SHADOW_SVK)
    vxd->bde_dev.base_address =
        (int)plx_vxd.bde_dev.base_address + DEV_REG_BASE_OFFSET;
    *(uint32 *)(vxd->bde_dev.base_address + 0x174) = 0x30303030;
    dev_rev_id = *(uint32 *)(vxd->bde_dev.base_address + DEV_REG_DEVID);
    vxd->bde_dev.device = dev_rev_id & 0xFFFF;
    vxd->bde_dev.rev = (dev_rev_id >> 16) & 0xFFFF;
#else
    vxd->bde_dev.base_address =
        (int)plx_vxd.bde_dev.base_address + PL0_OFFSET;
    dev_rev_id = *(uint32 *)(vxd->bde_dev.base_address);
    dev_rev_id = _shr_swap32(dev_rev_id);
    vxd->bde_dev.device = (dev_rev_id >> 16) & 0xFFFF;
    vxd->bde_dev.rev = dev_rev_id & 0xFFFF;
#endif

    vxd->dev_type = BDE_EB_DEV_TYPE | BDE_SWITCH_DEV_TYPE;
    vxd->iLine = FPGA_IRQ;
    vxd->iPin  = 0;

    switch (vxd->bde_dev.device) {
    case BM9600_DEVICE_ID:
        break;
#if defined (VENDOR_BROADCOM) && defined(SHADOW_SVK)
    case BCM88732_DEVICE_ID:
    case BCM8371_DEVICE_ID:
    case BCM8372_DEVICE_ID:
        break;
#endif
    default:
        printf("probe_plx_local_bus: unexpected device type type: "
               "0x%x \n", vxd->bde_dev.device);
        return -1;
    }
    return 0;
}

#ifdef BCM_EA_SUPPORT
#define MAX_EA_DEVICES 2

static int
_ea_attach(void)
{
    vxbde_dev_t *vxd;
    char                prop[64], *s;
    int dev = 0;
    int port = 0;

    for (dev = -1; dev < SOC_MAX_NUM_DEVICES; dev++) {
        for (port = 0; port < SOC_PBMP_PORT_MAX; port++) {
            sal_sprintf(prop, "ea_attach.port%d.%d", port,dev);
            if ((s = sal_config_get(prop))) {
                    vxd = _devices + _switch_n_devices++;
                    vxd->dev_type = (BDE_ET_DEV_TYPE | BDE_SWITCH_DEV_TYPE);
                    vxd->bde_dev.device = TK371X_DEVICE_ID;
                    vxd->bde_dev.rev = 0x0;
                    vxd->bde_dev.base_address = (sal_vaddr_t)NULL;
                    vxd->iLine = 0;
                    vxd->iPin = 0;
                    vxd->flags = 0;
                    _n_devices++;
            }
        }
    }

    return 0;
}
#endif

int
vxbde_create(vxbde_bus_t *bus, 
         ibde_t **bde)
{
    if (_n_devices == 0) {
    _bus = *bus;
        sal_sleep(1);
        _adjust_bus_base_addr_start();
    pci_device_iter(_setup);
    }

#if defined(KEYSTONE)
    fixup_pcie(0);
    if (pcie1_device_exist) {
        fixup_pcie(1);
    }
#endif

#ifdef BCM_ROBO_SUPPORT
    _spi_attach();
#endif    

#ifdef BCM_EA_SUPPORT
    _ea_attach();
#endif

    *bde = &_ibde;
#ifdef PCI_DECOUPLED 
    sysPciRWInit();
#endif

#ifdef INCLUDE_RCPU
    /*
     * Check if devices are connected to bus in EB mode.
     */
    if (sal_config_get("eb_probe") != NULL) {
        vxbde_eb_bus_probe(bus, bde);
    }
#endif /* INCLUDE_RCPU */

#ifdef METROCORE
    if (probe_metrocore_local_bus()) {
        return -1;
    }
#endif

    if (num_plx) {
        if (probe_plx_local_bus()) {
            return -1;
        }
    }

    return 0;  
}
#endif

/*
 * Create EB device at the specified base address.
 */
int vxbde_eb_create(vxbde_bus_t *bus, 
                    ibde_t **bde, sal_vaddr_t base_address)
{
    int                 devId;
    vxbde_dev_t         *vxd;
    char                prop[64], *s;
    uint32              dev_rev_id = 0x0;

    vxd = _devices + _n_devices;

    if (_n_devices == 0) {
        bcm_bde_soc_cm_memory_base = _bus.base_addr_start = base_address;
    }

    vxd->bde_dev.base_address = (sal_vaddr_t) base_address;
    vxd->dev_type = BDE_EB_DEV_TYPE | BDE_SWITCH_DEV_TYPE;

    devId = _n_devices;

    _n_devices++;
    _switch_n_devices++;

    sal_sprintf(prop, "eb_bus_read_16bit.%d", devId);
    if ((s = sal_config_get(prop))) {
        vxd->flags |= BDE_FLAG_BUS_RD_16BIT;
    }
    sal_sprintf(prop, "eb_bus_write_16bit.%d", devId);
    if ((s = sal_config_get(prop))) {
        vxd->flags |= BDE_FLAG_BUS_WR_16BIT;
    }

    dev_rev_id = _read(devId, 0x178);  /* CMIC_DEV_REV_ID */

    vxd->bde_dev.device = dev_rev_id & 0xFFFF;
    vxd->bde_dev.rev = (dev_rev_id >> 16) & 0xFF;
#ifdef   BCM_ICS
    vxd->iLine = 1;
#else
    vxd->iLine = 0;
#endif /* BCM_ICS */
    vxd->iPin  = 0;

    if (bde) {
        *bde = &_ibde;
    }
    return 0;
}
