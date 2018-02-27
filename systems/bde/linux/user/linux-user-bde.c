/*
 * $Id: linux-user-bde.c 1.45.6.6 Broadcom SDK $
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
 * Linux User BDE User Library
 */

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sched.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <endian.h>

#include <sal/core/sync.h>
#include <sal/core/spl.h>
#include <sal/core/thread.h>
#include <sal/core/time.h>
#include <sal/core/alloc.h>
#include <soc/cmic.h>
#include <soc/devids.h>


#include <mpool.h>
#include <linux-bde.h>

#include "kernel/linux-user-bde.h"

#include <linux/version.h>

#ifdef KEYSTONE
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include <sal/core/libc.h>
#include <sal/appl/config.h>

#define LINUX_SPIDEV_SUPPORT 1
#else
#define LINUX_SPIDEV_SUPPORT 0
#endif
#else /* !KEYSTONE */
#define LINUX_SPIDEV_SUPPORT 0
#endif /* KEYSTONE */

#if defined(PHYS_ADDRS_ARE_64BITS)
#include <asm/page.h>
#include <sys/mman.h>
#define MMAP	mmap64
typedef uint64 phys_addr_t;
#else 
#define MMAP	mmap
typedef uint32 phys_addr_t;
#endif  /* PHYS_ADDRS_ARE_64BITS */

/* 
 * The path to the driver device file 
 */
#define LUBDE_DEVICE_NAME "/dev/" LINUX_USER_BDE_NAME

/* 
 * The path to the mem device file 
 */
#define MEM_DEVICE_NAME "/dev/mem"

/*
 * Device information structure 
 */
typedef struct bde_dev_s {
    uint32              dev_type;       /* Type of underlaying device */
    ibde_dev_t          bde_dev;        /* BDE device description */
    phys_addr_t         pbase;          /* Physical base address of the device */
    unsigned int       *vbase;          /* Virtual base address of the device */
#ifdef BCM_MODENA_SUPPORT    
    unsigned int        mask;           /* Shadowed mask register */
#endif    
} bde_dev_t;

static bde_dev_t* _devices[LINUX_BDE_MAX_DEVICES];
static int        _ndevices = 0;
static int        _switch_ndevices = 0;
static int        _ether_ndevices = 0;
static int        _cpu_ndevices = 0;

/*
 * Global DMA pool. 
 *
 * One DMA memory pool is shared by all devices. 
 */
static uint32* _dma_vbase = NULL;
static uint32 _dma_pbase = 0;
static uint32 _dma_size = 0;
static mpool_handle_t _dma_pool;

/* 
 * Device File Descriptors
 */
static int _devfd = -1;
#ifndef BCM_PLX9656_LOCAL_BUS
static int _memfd = -1;
static int _sync_flags = O_SYNC | O_DSYNC | O_RSYNC;
#endif

#ifdef BCM_MODENA_SUPPORT    
#ifdef CMIC_IRQ_MASK
#undef CMIC_IRQ_MASK
#define CMIC_IRQ_MASK  0x00000018
#endif

#ifdef CMIC_IRQ_STAT
#undef CMIC_IRQ_STAT
#define CMIC_IRQ_STAT  0x00000014
#endif

#endif

#if LINUX_SPIDEV_SUPPORT
#ifdef BCM_ROBO_SUPPORT

#define USER_SPI_FREQ_DEFAULT 2000000  /* 2MHz */
#define USER_SPI_FREQ_20MHZ   20000000 /* 20MHz */

#define DEFAULT_SPI_TIMEOUT 50
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

/* 
 * The path to the spi driver device file 
 */
#define LINUX_USER_SPIDEV0_NAME "spidev0.0"
#define LUSPI_DEVICE0_NAME "/dev/" LINUX_USER_SPIDEV0_NAME

#define LINUX_USER_SPIDEV1_NAME "spidev0.1"
#define LUSPI_DEVICE1_NAME "/dev/" LINUX_USER_SPIDEV1_NAME

#define LINUX_USER_SPIDEV2_NAME "spidev0.2"
#define LUSPI_DEVICE2_NAME "/dev/" LINUX_USER_SPIDEV2_NAME

/* Data array to record Read/Wirte device through the Linux spidev driver or not */
static int spi_dev_linux[LINUX_BDE_MAX_SWITCH_DEVICES];

/*
 * Actual spi devices in our system
 */
static struct {
    int unit;

    /* For devices using Linux spidev driver */
    int spi_fd; /* SPI device descriptor */
    uint8 spi_mode;
    uint8 spi_bits;
    uint32 spi_speed;
    uint16 spi_delay;
} _sys_devs[LINUX_BDE_MAX_SWITCH_DEVICES];

static int spi_log_on = 0;

#endif /* BCM_ROBO_SUPPORT */
#endif /* LINUX_SPIDEV_SUPPORT */

/*
 * Function: _ioctl
 *
 * Purpose:
 *    Helper function for performing device ioctls
 * Parameters:  
 *    command - ioctl command code
 * Returns:
 *    Asserts if ioctl() system call fails. 
 *    Returns devio.rc value. 
 * Notes:
 *    You must program the devio structure with your parameters
 *    before calling this function. 
 */
static int
_ioctl(unsigned int command, lubde_ioctl_t *pdevio)
{
    pdevio->rc = -1;
    assert(ioctl(_devfd, command, pdevio) == 0);
    return pdevio->rc;
}

/*
 * Function: _mmap
 *
 * Purpose:
 *    Helper function for address mmapping. 
 * Parameters:
 *    p - physical address start
 *    size - size of region
 * Returns:
 *    Pointer to mapped region, or NULL on failure. 
 */
static void *
_mmap(phys_addr_t p, int size) 
{  
    void *map;
#ifdef BCM_PLX9656_LOCAL_BUS
    /*
     * On 440GX board with 2.6 kernel, /dev/mem and mmap don't work well
     * together.  So we use /dev/linux-user-bde for mapping instead
     * (see systems/linux/kernel/modules/shared/gmodule.c) -- hqian 8/15/07
     */
    map = MMAP(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, _devfd, p);
    if (map == MAP_FAILED) {
        perror("plx mmap failed: ");
        map = NULL;
    }
    return map;
#else
    unsigned int page_size, page_mask,offset;
    phys_addr_t  paddr;
    sal_vaddr_t  vmap;

    page_size = getpagesize();
    page_mask = ~(page_size - 1);

    if (p & ~page_mask) {
        /*
        * If address (p) not aligned to page_size, we could not get the virtual 
        * address. So we make the paddr aligned with the page size.
        * Get the _map by using the aligned paddr.
        * Add the offset back to return the virtual mapped region of p.
        */

        paddr = p & page_mask;
        offset = p - paddr;
        size += offset;

        map = MMAP(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, _memfd, paddr);
        if (map == MAP_FAILED) {
            perror("aligned mmap failed: ");
            map = NULL;
        }
        vmap = PTR_TO_UINTPTR(map) + offset;
        return (void *)(vmap);
    }

    map = MMAP(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, _memfd, p);
    if (map == MAP_FAILED) {
        perror("mmap failed: ");
        map = NULL;
    }
    return map;
#endif
}

/*
 * Function: _get_dma_info
 *
 * Purpose:
 *    Retrieve the size and base address of the DMA memory pool
 * Parameters:
 *    pbase - (out) physical address of the memory pool
 *    size  - (out) size of the memory pool
 * Returns:
 *    0
 */
static int
_get_dma_info(uint32* pbase, uint32* size)
{
    lubde_ioctl_t devio;
    devio.dev = 0;
    _ioctl(LUBDE_GET_DMA_INFO, &devio);
    *pbase = devio.d0;
    *size = devio.d1;
    return 0;
}

/*
 * Function: _open
 *
 * Purpose:
 *    Open device driver
 *    Initialize device structures
 *    Initialize DMA memory
 * Parameters:
 *    None
 * Returns:
 *    0 on successfully initialization
 *    -1 on error. 
 */
static int 
_open(void)
{ 
    lubde_ioctl_t devio;
    int i;
    int linux24;
    int procfd;
    char procbuf[4];
    
    if (_devfd >= 0) {
        /* Already open */
        return 0;
    }

    /* Open the device driver */
    if ((_devfd = open(LUBDE_DEVICE_NAME, 
                       O_RDWR | O_SYNC | O_DSYNC | O_RSYNC)) < 0) {

        /* Check Linux kernel version */
        linux24 = 0;
        if ((procfd = open("/proc/sys/kernel/osrelease", O_RDONLY)) >= 0) {
            if ((read(procfd, procbuf, sizeof(procbuf))) == 4) {
                if (procbuf[2] == '4') {
                    linux24 = 1;
                }
            }
            close(procfd);
        }

        /* Try inserting modules from the current directory */
        if (linux24) {
            system("/sbin/insmod linux-kernel-bde.o");
            system("/sbin/insmod linux-user-bde.o");
        } else {
            system("/sbin/insmod linux-kernel-bde.ko");
            system("/sbin/insmod linux-user-bde.ko");
        }

        if ((_devfd = open(LUBDE_DEVICE_NAME, 
                           O_RDWR | O_SYNC | O_DSYNC | O_RSYNC)) < 0) {
            perror("open " LUBDE_DEVICE_NAME ": ");
            return -1;
        }
    }

#ifndef BCM_PLX9656_LOCAL_BUS
    /*
     * Optionally use cached memory for DMA to improve performance.
     * Should be enabled on cache-coherent platforms only to avoid
     * data corruption and other fatal errors.
     */
#ifdef SAL_BDE_CACHE_DMA_MEM
    _sync_flags = 0;
#endif
    if ((_memfd = open(MEM_DEVICE_NAME, O_RDWR | _sync_flags)) < 0) {
        perror("open " MEM_DEVICE_NAME ": ");
        close(_devfd);
        return -1;
    }
#endif

    /* Get the availabled devices from the driver */
    memset(_devices, 0, sizeof(_devices));  
 
    /* first, get all available devices */
    devio.dev = BDE_ALL_DEVICES;
    assert(_ioctl(LUBDE_GET_NUM_DEVICES, &devio) == 0);
    _ndevices = devio.d0;
    if (_ndevices == 0) {
        printf("linux-user-bde: no devices\n");
    }

    /* second, get all available switching devices */
    devio.dev = BDE_SWITCH_DEVICES;
    assert(_ioctl(LUBDE_GET_NUM_DEVICES, &devio) == 0);
    _switch_ndevices = devio.d0;
    if(_switch_ndevices == 0) {
	printf("linux-user-bde: no switching devices\n");
    }

    /* then, get all available ethernet devices */
    devio.dev = BDE_ETHER_DEVICES;
    assert(_ioctl(LUBDE_GET_NUM_DEVICES, &devio) == 0);
    _ether_ndevices = devio.d0;

    /* Finally, get all available CPU devices */
    devio.dev = BDE_CPU_DEVICES;
    assert(_ioctl(LUBDE_GET_NUM_DEVICES, &devio) == 0);
    _cpu_ndevices = devio.d0;


    /* Initialize device structures for each device */
    for (i = 0; i < _ndevices; i++) {
	uint32 dev_type;

	/* Get the type of device */
	devio.dev = i;
	assert(_ioctl(LUBDE_GET_DEVICE_TYPE, &devio) == 0);

	dev_type = devio.d0;

	/* Get the detail info of the device */
        devio.dev = i;
        assert(_ioctl(LUBDE_GET_DEVICE, &devio) == 0);

        _devices[i] = (bde_dev_t*)malloc(sizeof(bde_dev_t));    
        memset(_devices[i], 0, sizeof(_devices[i]));
        _devices[i]->bde_dev.device = devio.d0;
        _devices[i]->bde_dev.rev = devio.d1;
        if ((dev_type & BDE_PCI_DEV_TYPE) ||
            (dev_type & BDE_ICS_DEV_TYPE) ||
            (dev_type & BDE_EB_DEV_TYPE)) {
	    int size;

	    size = 64 * 1024;
            if (dev_type & BDE_128K_REG_SPACE) {
                size = 128 * 1024;
            } else if (dev_type & BDE_256K_REG_SPACE) {
                size = 256 * 1024;
            } else if (dev_type & BDE_320K_REG_SPACE) {
                size = 320 * 1024;
            }

#if defined(PHYS_ADDRS_ARE_64BITS)
            _devices[i]->pbase = devio.d3;
            _devices[i]->pbase <<= 32;
            _devices[i]->pbase |= devio.d2;
#else
            _devices[i]->pbase = devio.d2;
#endif /* PHYS_ADDRS_ARE_64BITS */
            /* 64K memory window */

#ifdef KEYSTONE
            if (_devices[i]->bde_dev.device == BCM53000_GMAC_DEVICE_ID) {
                /* It covers core registers on APB0, APB1 and APB2 */
                size = 0x300000;
            }
#endif /* KEYSTONE */

            _devices[i]->vbase = _mmap(_devices[i]->pbase, size);   
            _devices[i]->bde_dev.base_address = 
                (sal_vaddr_t)_devices[i]->vbase;
        }
	_devices[i]->dev_type = dev_type;
    }

    /* Initialize DMA memory pool */
    mpool_init();
    _get_dma_info(&_dma_pbase, &_dma_size);
    assert(_dma_vbase = _mmap(_dma_pbase, _dma_size));
    assert(_dma_pool = mpool_create(_dma_vbase, _dma_size));

    /* calibrate */
    sal_udelay(0);

    return 0;
}

/* 
 * Function: _close
 *
 * Purpose:
 *    close the driver
 * Parameters:
 *    None
 * Returns:
 *    0
 */
static int
_close(void)
{       
#ifndef BCM_PLX9656_LOCAL_BUS
    close(_memfd);
    _memfd = -1;
#endif
    close(_devfd);
    _devfd = -1;
    return 0;
}

/*
 * Function: _enable_interrupts
 *
 * Purpose:
 *    Enable interrupts on all devices in the driver
 * Parameters:
 *	d - 0      the switching devices
 *	    others the ether device id of _devices.
 * Returns:
 *    0
 * Notes:
 *    When a real device interrupt occurs, the driver masks all 
 *    interrupts and wakes up the interrupt thread.
 *    It is assumed that the interrupt handler will unmask 
 *    interrupts upon exit.
 */
static int
_enable_interrupts(int d)
{
    lubde_ioctl_t devio;

    if (_switch_ndevices > 0 || _devices[d]->dev_type & BDE_ETHER_DEV_TYPE) {
        devio.dev = d;
        assert(_ioctl(LUBDE_ENABLE_INTERRUPTS, &devio) == 0);
    }
    return 0;
}

/* 
 * Function: _disable_interrupts
 *
 * Purpose:
 *    Disable interrupts on all devices in the driver
 * Parameters:
 *	d - 0      the switching devices
 *	    others the ether device id of _devices.
 * Returns:
 *    0
 */
static int
_disable_interrupts(int d)
{
    lubde_ioctl_t devio;

    if (_switch_ndevices > 0 || _devices[d]->dev_type & BDE_ETHER_DEV_TYPE) {
        devio.dev = d;
        assert(_ioctl(LUBDE_DISABLE_INTERRUPTS, &devio) == 0);
    }
    return 0;
}

/* 
 * Function: _pci_config_put32
 *
 * Purpose:
 *    Write a PCI configuration register on the device
 * Parameters:
 *    d - device number
 *    offset - register offset
 *    data - register data
 * Returns:
 *    0
 */
int 
_pci_config_put32(int d, unsigned int offset, unsigned int data)
{
    lubde_ioctl_t devio;

    devio.dev = d;
    devio.d0 = offset;
    devio.d1 = data;
    _ioctl(LUBDE_PCI_CONFIG_PUT32, &devio);
    return 0;
}

/* 
 * Function: _pci_config_get32
 *
 * Purpose:
 *    Read a PCI configuration register on the device
 * Parameters:
 *    d - device number
 *    offset - register offset
 * Returns:
 *    register value
 */
unsigned int 
_pci_config_get32(int d, unsigned int offset)
{
    lubde_ioctl_t devio;

    devio.dev = d;
    devio.d0 = offset;
    _ioctl(LUBDE_PCI_CONFIG_GET32, &devio);
    return devio.d0;
}

/*
 * Function: _read
 *
 * Purpose:
 *    Read a register
 * Parameters:
 *    d - device number
 *    addr - register address
 * Returns:
 *    register value
 */
static unsigned int 
_read(int d, unsigned int addr)
{
    lubde_ioctl_t _devio;

    assert(_devices[d]->vbase);

    if (_devices[d]->dev_type & BDE_DEV_BUS_RD_16BIT) {
        _devio.dev = d;
        _devio.d0 = addr;
        if (_ioctl(LUBDE_READ_REG_16BIT_BUS, &_devio) != 0) {
            printf("linux-user-bde: warnning: eb_read from kernel failed \
                   unit=%d addr=%x", d, addr);
            return -1;
        }
        return _devio.d1;
    } else {
        return _devices[d]->vbase[addr/sizeof(uint32)];
    }

}

/*
 * Function: _write
 *
 * Purpose:
 *    Write a register
 * Parameters:
 *    d - device number
 *    addr - register address
 *    data - register data
 * Returns:
 *    0
 */
int
_write(int d, uint32 addr, uint32 data)
{
    lubde_ioctl_t _devio;

    assert(_devices[d]->vbase);

    if (_devices[d]->dev_type & BDE_DEV_BUS_WR_16BIT) {
        _devio.dev = d;
        _devio.d0 = addr;
        _devio.d1 = data;
        if (_ioctl(LUBDE_WRITE_REG_16BIT_BUS, &_devio) != 0) {
            printf("linux-user-bde: warnning: eb_write failed \
                   unit=%d addr=%x", d, addr);
            return -1;
        }
    } else {
        _devices[d]->vbase[addr/sizeof(uint32)] = data;
    }
    return 0;
}

/*
 * Function: _salloc
 *
 * Purpose:
 *    Allocate DMA memory
 * Parameters:
 *    d - device number
 *    size - size of block
 *    name - name of block (debugging, unused)
 * Returns:
 *    0
 */
static uint32*
_salloc(int d, int size, const char *name)
{
    /* All devices use the same dma memory pool */
    return mpool_alloc(_dma_pool, size);
}

/* 
 * Function: _sfree
 *
 * Purpose:
 *    Free DMA memory
 * Parameters:
 *    d - device number
 *    ptr - ptr to memory to free
 * Returns:
 *    0
 */
static void
_sfree(int d, void *ptr)
{
    /* All devices use the same dma memory pool */
    mpool_free(_dma_pool, ptr);
}       


/*
 * Thread ID of the signal handler/interrupt thread
 */
static volatile sal_thread_t _intr_thread = NULL;

/*
 * Thread ID of the signal handler/interrupt thread for ethernet devices
 */
static volatile sal_thread_t _ether_intr_thread = NULL;

/*
 * Function: intr_int_context
 *
 * Purpose:
 *    Used by the linux kernel SAL to implement sal_int_context(). 
 * Parameters:
 *    None
 * Returns:
 *    When the current thread is the interrupt thread. 
 */
int intr_int_context(void) 
{
    return ((_intr_thread == sal_thread_self()) ||
            (_ether_intr_thread == sal_thread_self()));
}


/* 
 * Client Interrrupt Management
 */

typedef struct intr_handler_s {
    void *data;
    void (*handler)(void*);
} intr_handler_t;

/* for switching devices */
static intr_handler_t _handlers[LINUX_BDE_MAX_SWITCH_DEVICES];
static int _nhandlers = 0;

/* for ethernet devices */ 
static intr_handler_t _ether_handler;
static int _ether_dev_handler = 0; /* device id for ethernet device */

/*
 * Function: _run_intr_handlers
 *
 * Purpose:
 *    Run application level interrupt handlers.
 * Parameters:
 *    None
 * Returns:
 *    Nothing
 */
static void
_run_intr_handlers(void)
{
    int i;
    int spl;

    /* 
     * Protect applications threads from interrupt thread.
     */
    spl = sal_splhi();

    /*
     * Run all of the client interrupt handlers
     */
    for (i = 0; i < _nhandlers; i++) {
        _handlers[i].handler(_handlers[i].data);
    }

    /* 
     * Restore spl
     */
    sal_spl(spl);
}
static void
_run_ether_intr_handlers(void)
{
    int spl;

    /* 
     * Protect applications threads from interrupt thread.
     */
    spl = sal_splhi();

    /*
     * Run all of the client interrupt handlers
     */
    
    _ether_handler.handler(_ether_handler.data);
    

    /* 
     * Restore spl
     */
    sal_spl(spl);
}

#ifdef SAL_BDE_THREAD_PRIO_DEFAULT
#else
/*
 * Function: _set_thread_priority
 *
 * Purpose:
 *    Raise the current thread's execution priority
 *    from regular to realtime. 
 * Parameters:
 *    prio - The realtime scheduling priority (0 - 99)
 * Returns:
 *    Nothing
 */
static void
_set_thread_priority(int prio)
{
    struct sched_param param;
    param.sched_priority = prio;
    if (sched_setscheduler(0, SCHED_RR, &param)) {       
        perror("\ninterrupt priority set: ");
    }
}       
#endif /* SAL_BDE_THREAD_PRIO_DEFAULT */


/*
 * Function: _interrupt_thread
 *
 * Purpose:
 *    Provides a thread context for interrupt handling. 
 * Parameters:
 *    context - unused
 * Returns:
 *    Nothing
 * Notes:
 */
static void
_interrupt_thread(void *context)
{
    lubde_ioctl_t devio;
    devio.dev = 0;

    /* 
     * We are the interrupt thread for intr_int_context()
     */
    _intr_thread = sal_thread_self();

#ifdef SAL_BDE_THREAD_PRIO_DEFAULT
#else
    /* increase our priority */
    _set_thread_priority(90);
#endif /* SAL_BDE_THREAD_PRIO_DEFAULT */

    /* enable interrupts on the device, and wait for signals */
    _enable_interrupts(0);

    for (;;) {
        _ioctl(LUBDE_WAIT_FOR_INTERRUPT, &devio);
        _run_intr_handlers();
    }
}

/*
 * Function:
 *	_ether_interrupt_thread
 * Purpose:
 *	Provides a thread context for ethernet interrupt handling. 
 * 	Interrupts are signaled, it provides the 
 *	unique context needed by the signal handler. 
 * Parameters:
 *	d - the device id in _devices
 * Returns:
 *	Nothing
 * Notes:
 */

static void
_ether_interrupt_thread(void* d)
{
    lubde_ioctl_t devio;
    devio.dev = PTR_TO_INT(d);
    /* 
     * We are the interrupt thread for intr_int_context()
     */
    _ether_intr_thread = sal_thread_self();
    
#ifdef SAL_BDE_THREAD_PRIO_DEFAULT
#else
    /* increase our priority */
    _set_thread_priority(90);
#endif /* SAL_BDE_THREAD_PRIO_DEFAULT */

    /* enable interrupts on the device, and wait for signals */
    _enable_interrupts(PTR_TO_INT(d));
    for (;;) {
        _ioctl(LUBDE_WAIT_FOR_INTERRUPT, &devio);
        _run_ether_intr_handlers();
    }
}

/*
 * Function:
 *	_interrupt_connect
 * Purpose:
 *    BDE vector for connecting client interrupts to the device. 
 *    Initialized the SW interrupt controller and registers the handler. 
 * Parameters:
 *    d - device number
 *    handler - client interrupt handler
 *    data    - client interrupt handler data
 * Returns:
 *    0
 */
static int 
_interrupt_connect(int d, 
                   void (*handler)(void*),
                   void *data)
{
    if (_devices[d]->dev_type & BDE_ETHER_DEV_TYPE) {
       _ether_dev_handler = d;
        _ether_handler.handler = handler;
        _ether_handler.data = data;

	/* Create the ethernet interrupt thread */
	sal_thread_create("_ether_interrupt_thread",
			  8096, 0,
			  (void (*)(void*))_ether_interrupt_thread,
			  INT_TO_PTR(d));	

        return 0;
    }

    _handlers[_nhandlers].handler = handler;
    _handlers[_nhandlers].data = data;
    
    /*
     * Start up interrupt processing if this is the first connect
     */
    if (_nhandlers == 0) {       
        sal_thread_create("_interrupt_thread",
                          8096, 0,
                          (void (*)(void*))_interrupt_thread,
                          NULL); 
    }
    
    _nhandlers++;
    return 0;
}

/*
 * Function: _interrupt_disconnect
 *
 * Purpose:
 *    BDE interrupt disconnect function
 * Parameters:
 *    d - device number
 * Returns:
 *    0
 */
static int 
_interrupt_disconnect(int d)
{
    if ((d >=0) && (d < _switch_ndevices)) {
        _disable_interrupts(0); /* all switching devices */
    } else if ((d >= _switch_ndevices) && (d < _ndevices)) {
        _disable_interrupts(d); /* specified ether device */
    }
    return 0;
}

/*
 * Function: _l2p
 *
 * Purpose:
 *    BDE l2p function. Converts CPU virtual DMA addresses
 *    to Device Physical DMA addresses. 
 * Parameters:
 *    d - device number
 *    laddr - logical address to convert
 * Returns:
 *    Physical address
 */
static uint32
_l2p(int d, void *laddr)
{
    /* DMA memory is one contiguous block */
    uint32 pbase = (uint32)_dma_pbase;
    sal_vaddr_t vbase = PTR_TO_UINTPTR(_dma_vbase);
    sal_vaddr_t addr = PTR_TO_UINTPTR(laddr);

    uint32 rc;

    if (!laddr) return 0;

    rc = pbase + (uint32)(addr-vbase);

    return rc;
}       

/* 
 * Function: _p2l
 *
 * Purpose:
 *    BDE p2l function. Converts Device Physical DMA addresses
 *    to CPU virtual DMA addresses. 
 * Parameters:
 *    d - device number
 *    paddr - physical address to convert
 * Returns:
 *    Virtual address
 */
static uint32* 
_p2l(int d, uint32 paddr)
{
    /* DMA memory is one contiguous block */
    uint32 pbase = (uint32)_dma_pbase;
    sal_vaddr_t vbase = PTR_TO_UINTPTR(_dma_vbase);
    sal_vaddr_t rc;

    if (!paddr) return NULL;

    rc = vbase + (paddr-pbase);

    return (uint32*)UINTPTR_TO_PTR(rc);
}

#if LINUX_SPIDEV_SUPPORT /* Linux spidev driver for switch register access */

#ifdef BCM_ROBO_SUPPORT
/* 
 * Function:
 *    _spi_dev_read
 * Purpose:
 *    Issue to send spi read message for Robo devices.
 * Parameters:
 *    dev - device number
 *    reg - register addr to read
 *    buf - (out) buffer addr to store the reading result
 *    len - number bytes to read
 * Returns:
 *    0 - Success
 *   -1 - Failed
 */
static int
_spi_dev_read(int dev, int reg, uint8 *buf, int len)
{	
    int i;
    int ret;	
    int fd = _sys_devs[dev].spi_fd;

    uint8 tx[2];
    uint8 rx[8];
    struct spi_ioc_transfer tr[2];

    memset(&rx, 0, ARRAY_SIZE(rx) * sizeof(uint8));

    tx[0] = 0x60;
    tx[1] = (uint8)reg;

    memset(tr, 0, sizeof(tr));
    tr[0].tx_buf = (unsigned long)tx;
    tr[0].len = ARRAY_SIZE(tx);

    tr[1].rx_buf = (unsigned long)rx;
    tr[1].len = len;

    ret = ioctl(fd, SPI_IOC_MESSAGE(2), &tr);
    if (ret == 1) {
        printf("Can't send spi message: read(ret=%d)\n", ret);
    }
    for (i = 0; i < len; i++) {
        *(buf + i) = rx[i];
    }
    return ret;
}

/* 
 * Function:
 *    _spi_dev_write
 * Purpose:
 *    Issue to send spi write message for Robo devices.
 * Parameters:
 *    dev - device number
 *    reg - register addr to write
 *    buf - buffer to write to spi
 *    len - number bytes to write
 * Returns:
 *    0 - Success
 *   -1 - Failed
 */
static int
_spi_dev_write(int dev, int reg, const uint8 *buf, int len)
{	
    int i;
    int ret;	
    int fd = _sys_devs[dev].spi_fd;
    uint8 tx1[2];
    uint8 tx2[8];
    
    struct spi_ioc_transfer tr[2];

    memset(tx2, 0, ARRAY_SIZE(tx2) * sizeof(uint8));
    for (i = 0; i < len; i++) {
        tx2[i] = *(buf+i);
    }

    tx1[0] = 0x61;
    tx1[1] = (uint8)reg;
    
    memset(&tr, 0, sizeof(tr));
    tr[0].tx_buf = (unsigned long)tx1;
    tr[0].len = ARRAY_SIZE(tx1);

    tr[1].tx_buf = (unsigned long)tx2;
    tr[1].len = len;

    ret = ioctl(fd, SPI_IOC_MESSAGE(2), &tr);
    if (ret == 1) {
        printf("Can't send spi message: write(ret=%d)\n", ret);
    }

    return ret;
}

/* 
 * Function:
 *    _spi_dev_poll_for_spif
 * Purpose:
 *    Polling the SPI status of device for SPIF (bit 7).
 * Parameters:
 *    dev - device number
 * Returns:
 *    0 - Success
 *   -1 - Timeout
 */
static int
_spi_dev_poll_for_spif(int dev)
{
    int i;
    int timeout;
    uint8 byte;

    for (i = 0, timeout = DEFAULT_SPI_TIMEOUT; timeout;) {
        _spi_dev_read(dev, 0xfe, &byte, 1);
        /* SPIF is bit 7 of SPI_STS */
        /* check SPIF = 0 ? */
        if (!(byte & 0x80)) {
            break;
        } else {
            timeout--;
        }
    }

    if (timeout == 0) {
        return -1;
    }
    return 0;
}

/* 
 * Function:
 *    _spi_dev_poll_for_rack
 * Purpose:
 *    Polling the SPI status of device for rack (bit 5).
 * Parameters:
 *    dev - device number
 * Returns:
 *    0 - Success
 *   -1 - Timeout
 */
static int
_spi_dev_poll_for_rack(int dev)
{
    int i;
    int timeout;
    uint8 byte;

    for (i = 0, timeout = DEFAULT_SPI_TIMEOUT; timeout;) {
        _spi_dev_read(dev, 0xfe, &byte, 1);
         /*  In normal read mode */
         /*  check bit 5 are high */
        if (byte & 0x20) {
            break;
        } else {
            timeout--;
        }
    }

    if (timeout == 0) {
        return -1;
    }
    return 0;
}

/* 
 * Function:
 *    _spi_dev_select
 * Purpose:
  *    Issue to send spi write message for Robo register's page selection.
 * Parameters:
 *    dev - device number
 *    page - page address
 * Returns:
 *    0 - Success
 *   -1 - Failed
 */
static int
_spi_dev_select(int dev, int page)
{
    int ret;
    int fd = _sys_devs[dev].spi_fd;

    uint8 tx[3];
    struct spi_ioc_transfer tr[1];

    tx[0] = 0x61;
    tx[1] = 0xff;
    tx[2] = (uint8)page;

    memset(&tr, 0, sizeof(tr));
    tr[0].tx_buf = (unsigned long)tx;
    tr[0].len = ARRAY_SIZE(tx);

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret == 1) {
        printf("Can't send spi message: select(ret=%d)\n", ret);
    }
    return ret;
}

/* 
 * Function:
 *    _sys_spi_read
 * Purpose:
 *    Linux spidev read function. Issue spi read via Linux spidev driver.
 * Parameters:
 *    dev - device number
 *    addr - register addr to read
 *    data - (out) data addr to store the reading result
 *    len - number bytes to read
 * Returns:
 *    0 - Success
 *   -1 - Failed
 */
static int
_sys_spi_read(int dev, uint32 addr, uint8 *data, int len)
{
    uint8 page;
    uint8 offset;
    uint8 byte;

    page = (addr >> 8) & 0xff;
    offset = addr & 0xff;

    /* Poll for SPIF */
    if (_spi_dev_poll_for_spif(dev) < 0) {
        /* timeout */
        if (spi_log_on) {
            printf("SPI poll for SPIF: timeout\n");
        }
        return -1;
    }

    /* Select chip and page */
    if (_spi_dev_select(dev, page) < 0) {
        if (spi_log_on) {
            printf("SPI select chip and page failed\n");
        }
        return -1;
    }

    /* Normal read. Discard first read */
    if (_spi_dev_read(dev, offset, &byte, 1) < 0) {
        if (spi_log_on) {
            printf("SPI device read failed\n");
        }
        return -1;
    }

    /* Poll for RACK */
    if (_spi_dev_poll_for_rack(dev) < 0) {
        /* timeout */
        if (spi_log_on) {
            printf("SPI poll for RACK: timeout\n");
        }
        return -1;
    }

    /* Read register from dataport */
    if (_spi_dev_read(dev, 0xf0, data, len) < 0) {
        if (spi_log_on) {
            printf("SPI device read failed\n");
        }
        return -1;
    }

    return 0;
}

/* 
 * Function:
 *    _sys_spi_write
 * Purpose:
 *    Linux spidev write function. Issue spi write via Linux spidev driver.
 * Parameters:
 *    dev - device number
 *    addr - register addr to write
 *    data - data buffer to write to spi
 *    len - number bytes to write
 * Returns:
 *    0 - Success
 *   -1 - Failed
 */
static int
_sys_spi_write(int dev, uint32 addr, const uint8 *data, int len)
{
    uint8 page;
    uint8 offset;

    page = (addr >> 8) & 0xff;
    offset = addr & 0xff;

    /* Poll for SPIF */
    if (_spi_dev_poll_for_spif(dev) < 0) {
        /* timeout */
        if (spi_log_on) {
            printf("SPI poll for SPIF: timeout\n");
        }
        return -1;
    }

    /* Select chip and page */
    if (_spi_dev_select(dev, page) < 0) {
        if (spi_log_on) {
            printf("SPI select chip and page failed\n");
        }
        return -1;
    }

    /* Write register */
    if (_spi_dev_write(dev, offset, data, len) < 0) {
        if (spi_log_on) {
            printf("SPI device write failed\n");
        }
        return -1;
    }

    return 0;
}

/* 
 * Function:
 *    _sys_spi_dev_init
 * Purpose:
 *    Initialize to setup linux spidev driver
 * Parameters:
 *    dev - device number
 *    fd - SPI device descriptor
 * Returns:
 *    0 - Success
 *   -1 - Failed
 */
static int
_sys_spi_dev_init(int dev, int fd)
{
    uint8 mode;
    uint32 speed;

    _sys_devs[dev].unit = fd;
    _sys_devs[dev].spi_fd = fd;

    mode = SPI_MODE_3; /* SPI_CPOL|SPI_CPHA */
    if (ioctl(fd, SPI_IOC_WR_MODE, &mode) < 0) {
        printf("Can not set mode for SPI device %d.\n", dev);
        close(fd);
        return -1;
    }
    _sys_devs[dev].spi_mode = mode;

    if (ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed) < 0) {
        printf("Can not get max speed for SPI device %d.\n", dev);
        close(fd);
        return -1;
    }

    if (speed != USER_SPI_FREQ_DEFAULT) {
        speed = USER_SPI_FREQ_DEFAULT;
        if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
            printf("Can not set max speed %dHz for SPI device %d.\n", speed, dev);
            close(fd);
            return -1;
        }
    }
    _sys_devs[dev].spi_speed = speed;

    _sys_devs[dev].spi_bits = 0;
    _sys_devs[dev].spi_delay = 0;

    return 0;
}

/* 
 * Function:
 *    _sys_spi_dev_deinit
 * Purpose:
 *    close linux spidev driver device file
 * Parameters:
 *    dev - device number
 * Returns:
 *    void
 */
static void
_sys_spi_dev_deinit(int dev)
{
    int fd;

    fd = _sys_devs[dev].spi_fd;
    close(fd);
}

/* 
 * Function:
 *    _sys_spi_dev_check
 * Purpose:
 *    Check the device ID (phyid_low) is valid or not (compare with the result of Kernel BDE)
 * Parameters:
 *    phyidl_nr - phy id low without revision
 * Returns:
 *    0 - Found valid SPI devices
 *    1 - No valid SPI devices found
 */
static int
_sys_spi_dev_check(int dev, unsigned short phyidl_nr)
{
    int ndevs;
    uint8 part_8 = 0;
    uint16 part_16 = 0;
    uint32 part_32 = 0;
    uint64 part_64 = 0;

    if (phyidl_nr == BCM5348_PHYID_LOW) {
        _sys_spi_read(dev, 0x0388, (uint8 *)&part_8, 1);
        part_16 = part_8;
    } else if ((phyidl_nr == BCM53242_PHYID_LOW) ||
               (phyidl_nr == BCM53262_PHYID_LOW)) {
        part_16 = 0;
    } else if ((phyidl_nr == BCM53115_PHYID_LOW) ||
               (phyidl_nr == BCM53118_PHYID_LOW) ||
               (phyidl_nr == BCM53101_PHYID_LOW) ||
               (phyidl_nr == BCM53125_PHYID_LOW) ||
               (phyidl_nr == BCM53128_PHYID_LOW)) {
        /* Register Model ID len = 4 for BCM53115, BCM53118 */
        _sys_spi_read(dev, 0x0230, (uint8 *)&part_32, 4);
#ifdef BE_HOST
        part_32 = (((part_32 >> 24) & 0x000000FF) |
                   ((part_32 >> 8)  & 0x0000FF00) |
                   ((part_32 << 8)  & 0x00FF0000) |
                   ((part_32 << 24) & 0xFF000000));
#endif
        part_16 = (part_32 & 0x0000FFFF);
    } else if ((phyidl_nr == BCM53280_PHYID_LOW) ||
               (phyidl_nr == BCM53600_PHYID_LOW)) {
        _sys_spi_read(dev, 0x00e8, (uint8 *)&part_64, 8);
        part_8 =  *(uint8 *)&part_64;
        part_16 = part_8;
    } else {
        _sys_spi_read(dev, 0x0230, (uint8 *)&part_8, 1);
        part_16 = part_8;
    }

    for (ndevs = 0; ndevs < _ndevices; ndevs++) {
        if ((_devices[ndevs]->dev_type & BDE_SPI_DEV_TYPE) &&
            (_devices[ndevs]->dev_type & BDE_SWITCH_DEV_TYPE)) {
            if ((phyidl_nr == (_devices[ndevs]->bde_dev.device & 0xfff0)) ||
                (part_16 == _devices[ndevs]->bde_dev.device)) {
                return 0;
            }
        }
    }

    /* No valid SPI devices found */
    return 1;
}

static int
linux_spidev_setup(void)
{
    int rv = 0;
    char *spi_device_name = LUSPI_DEVICE0_NAME;
    int i = 0, fd = 0;
    int ndevs = 0, spi_freq = 0;
    unsigned short phyidl = 0, phyidl_nr = 0;
    int spi_freq_override;         /* Used for user overrided SPI frequency */
    char *spi_freq_str = NULL;


    /* Initialization */
    memset(spi_dev_linux, 0, sizeof(spi_dev_linux));

    /* 
     * if (spi_freq_str != NULL): try to use linux spidev bus driver
     * else (spi_freq_str == NULL): use kernel BDE bus decision
     */
    if ((spi_freq_str = sal_config_get("spi_freq_override")) != NULL) {
        /* For Keystone, maximum three spi devices */
        for (i = 0; i < 3; i++) {
	        /* Loop to open the spi driver device file: /dev/spidev0.x */
            switch (i) {
                case 0:
                    spi_device_name = LUSPI_DEVICE0_NAME;
                    break;
                case 1:
                    spi_device_name = LUSPI_DEVICE1_NAME;
                    break;
                case 2:
                    spi_device_name = LUSPI_DEVICE2_NAME;
                    break;
            }

            /* Open the device file */
            fd = open(spi_device_name, O_RDWR);
            if (fd >= 0) {
                /* Open successful and then initialize to setup the SPI device */
                if (_sys_spi_dev_init(ndevs, fd) == 0) {
                    /* Setup successful */

                    /* Try to read device phyid low by Linux spidev driver */
                    rv = _sys_spi_read(ndevs, 0x1006, (uint8 *)&phyidl, 2);
                    if (rv < 0) {
                         /* Read failed for Linux spidev driver */
                        _sys_spi_dev_deinit(ndevs);
                    } else {
                        /* Read successful and check if it is a valid SPI device */
#ifdef BE_HOST
                        phyidl = (phyidl >> 8) | (phyidl << 8);
#endif
                        /* Strip revision */
                        phyidl_nr = phyidl & 0xfff0;

                        rv = _sys_spi_dev_check(ndevs, phyidl_nr);
                        if (rv == 0) {
                            /* Valid SPI device */
                            /* Thunderbolt and Lotus can support SPI Frequency up to 20MHz */
                            if ((phyidl_nr == BCM53280_PHYID_LOW) ||
                                (phyidl_nr == BCM53101_PHYID_LOW) ||
                                (phyidl_nr == BCM53125_PHYID_LOW) || 
                                (phyidl_nr == BCM53128_PHYID_LOW) ||
                                (phyidl_nr == BCM53600_PHYID_LOW)) {
                                spi_freq = USER_SPI_FREQ_20MHZ;
                            } else {
                                spi_freq = USER_SPI_FREQ_DEFAULT;
                            }

                            /* 
                             * if (spi_freq_override > 0) Override the SPI frequency from user
                             * else (spi_freq_override <= 0) Use device probed SPI frequency
                             */
                            if ((spi_freq_override = sal_ctoi(spi_freq_str, 0)) > 0) {
                                spi_freq = spi_freq_override;
                            }

                            rv = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_freq);
                            if (rv < 0) {
                                _sys_spi_dev_deinit(ndevs);
                            } else {
                                _sys_devs[ndevs].spi_speed = (uint32)spi_freq;
                                /* Valid SPI device for linux spidev driver */
                                spi_dev_linux[ndevs] = 1;
                                ndevs++;
                            }
                        } else {
                             /* Not a valid SPI device */
                            _sys_spi_dev_deinit(ndevs);
                        }
                    }
                }
            }
            if (ndevs >= _switch_ndevices) {
                break;
            }
        }
    }
    spi_log_on = 1;

    return 0;

}
#endif /* BCM_ROBO_SUPPORT */

#endif /* LINUX_SPIDEV_SUPPORT */

#ifdef BCM_ROBO_SUPPORT

/* 
 * Function:
 *	_spi_read
 * Purpose:
 *	BDE spi_read function. Issue spi read via SMP
 * Parameters:
 *	d - device number
 *	addr - register addr to read
 *	buf - buffer addr to store the reading result
 *	int - number bytes to read
 * Returns:
 *	0 - Success
 *     -1 - Failed
 */

static int
_spi_read(int d, uint32 addr, uint8 *buf, int len)
{
    lubde_ioctl_t _devio;

#if LINUX_SPIDEV_SUPPORT
    if (spi_dev_linux[d]) {
        return _sys_spi_read(d, addr, buf, len);
    }
#endif /* LINUX_SPIDEV_SUPPORT */

    _devio.dev = d;
    _devio.d0 = addr;
    _devio.d1 = len;
    memset(_devio.dx.buf, 0, sizeof(_devio.dx.buf));

    if (_ioctl(LUBDE_SPI_READ_REG, &_devio) != 0) {
	printf("linux-user-bde: warnning: spi_read failed \
		unit=%d addr=%x, len=%d", d, addr, len);
        return -1;
    }

    memcpy(buf, _devio.dx.buf, len);

    return 0;
}

/* 
 * Function:
 *	_spi_write
 * Purpose:
 *	BDE spi_write function. Issue spi read via SMP
 * Parameters:
 *	d - device number
 *	addr - register addr to write
 *	buf - buffer to write to spi
 *	int - number bytes for write
 * Returns:
 *	0 - Success
 *     -1 - Failed
 */

static int
_spi_write(int d, uint32 addr, uint8 *buf, int len)
{
    lubde_ioctl_t _devio;

#if LINUX_SPIDEV_SUPPORT
    if (spi_dev_linux[d]) {
        return _sys_spi_write(d, addr, buf, len);
    }
#endif /* LINUX_SPIDEV_SUPPORT */

    _devio.dev = d;
    _devio.d0 = addr;
    _devio.d1 = len;
    memcpy(_devio.dx.buf, buf, len);

    if (_ioctl(LUBDE_SPI_WRITE_REG, &_devio) != 0) {
	printf("linux-user-bde: warnning: spi_write failed \
		unit=%d addr=%x, len=%d", d, addr, len);
        return -1;
    }

    return 0;
}
#ifdef INCLUDE_ROBO_I2C
static int
_i2c_read(int d, uint16 addr, uint8 *buf, int len)
{
    lubde_ioctl_t _devio;

    _devio.dev = d;
    _devio.d0 = addr;
    _devio.d1 = len;

    memset(_devio.dx.buf, 0, sizeof(_devio.dx.buf));

    if (_ioctl(LUBDE_I2C_READ_REG, &_devio) != 0) {
	    printf("linux-user-bde: warnning: i2c_read failed \
		unit=%d addr=%x, len=%d", d, addr, len);
        return -1;
    }

    memcpy(buf, _devio.dx.buf, len);

    return 0;
}

static int
_i2c_write(int d, uint16 addr, uint8 *buf, int len)
{
    lubde_ioctl_t _devio;

    _devio.dev = d;
    _devio.d0 = addr;
    _devio.d1 = len;

    memcpy(_devio.dx.buf, buf, len);

    if (_ioctl(LUBDE_I2C_WRITE_REG, &_devio) != 0) {
	    printf("linux-user-bde: warnning: i2c_write failed \
		unit=%d addr=%x, len=%d", d, addr, len);
        return -1;
    }

    return 0;
}

static int
_i2c_read_intr(int d, uint8 chipid, uint8 *buf, int len)
{
    lubde_ioctl_t _devio;

    _devio.dev = d;
    _devio.d0 = chipid;
    _devio.d1 = len;

    memset(_devio.dx.buf, 0, sizeof(_devio.dx.buf));

    if (_ioctl(LUBDE_I2C_READ_INTR_REG, &_devio) != 0) {
	    printf("linux-user-bde: warnning: i2c_read_intr failed \
		unit=%d addr=%x, len=%d", d, chipid, len);
        return -1;
    }

    memcpy(buf, _devio.dx.buf, len);

    return 0;
}

static int
_i2c_read_ARA(int d, uint8 *chipid, int len)
{
    lubde_ioctl_t _devio;

    _devio.dev = d;

    _devio.d1 = len;

    memset(_devio.dx.buf, 0, sizeof(_devio.dx.buf));

    if (_ioctl(LUBDE_I2C_READ_ARA, &_devio) != 0) {
	    printf("linux-user-bde: warnning: i2c_read_ARA failed \
		unit=%d len=%d", d, len);
        return -1;
    }

    memcpy(chipid, _devio.dx.buf, len);

    return 0;
}
#endif
#endif

/* 
 * Function:
 *	_num_devices
 * Purpose:
 *    BDE num_devices function. Returns the number of devices. 
 * Parameters:
 *    None
 * Returns:
 *    number of devices in this bde
 */
static int
_num_devices(int type)
{
    switch (type) {
    case BDE_ALL_DEVICES:
        return _ndevices;
    case BDE_SWITCH_DEVICES:
        return _switch_ndevices;
    case BDE_ETHER_DEVICES:
        return _ether_ndevices;
    case BDE_CPU_DEVICES:
        return _cpu_ndevices;
    }
    return 0;
}

/* 
 * Function: _get_dev
 *
 * Purpose:
 *    BDE get_dev function. Returns device information structure. 
 * Parameters:
 *    d - device number
 * Returns:
 *    const pointer to BDE device information structure.
 */
static const ibde_dev_t*
_get_dev(int d)
{
    assert(d >= 0 || d < _ndevices);
    return &_devices[d]->bde_dev;
}

/* 
 * Function:
 * 	_get_dev_type
 * Purpose:
 *	BDE get_dev_type function. Returns device type of BUS(PCI,SPI)/
 *	FUNCTIONALITY(SWITCH/ETHERNET). 
 * Parameters:
 *	d - device number
 * Returns:
 *	unsigned dword ORed with capablities of underlaying device.
 */
static uint32
_get_dev_type(int d)
{
    assert(d >= 0 || d < _ndevices);
    return _devices[d]->dev_type;
}

/* 
 * Function:
 *	_name
 * Purpose:
 *    BDE name function. Returns the name of the BDE. 
 * Parameters:
 *    None
 * Returns:
 *    Name of this BDE
 */
static const char*
_name(void)
{
    return LINUX_USER_BDE_NAME;
}


/* 
 * Function: _bus_features
 *
 * Purpose:
 *    BDE bus features function. Returns the endian features of the system bus. 
 * Parameters:
 *    be_pio - (out) returns the big endian pio bit. 
 *    be_packet - (out) returns the big endian packet bit. 
 *    be_other - (out) returns the big endian other bit. 
 * Returns:
 *    nothing
 * Notes:
 *    This just uses the values passed in by the BDE creator. 
 *    See linux_bde_create(). 
 */ 
static linux_bde_bus_t _bus;

static void
_bus_features(int unit, int *be_pio, int *be_packet, int *be_other)
{
    /*
     * XGS BCM 56xxx/53xxx devices get the endianness from compile flags
     * whereas SBX devices (BCM88020 and QE-2000) get it by querying
     * the kernel BDE (ioctl)
     */
    if ((_devices[unit]->bde_dev.device & 0xFF00) != 0x5600 &&
        (_devices[unit]->bde_dev.device & 0xF000) != 0xc000 &&
        (_devices[unit]->bde_dev.device & 0xF000) != 0xb000 &&
        (_devices[unit]->bde_dev.device & 0xF000) != 0x8000 &&
        (_devices[unit]->bde_dev.device & 0xFFFF) != 0x0732 &&
        (_devices[unit]->bde_dev.device & 0xFFF0) != 0x0230) {
        lubde_ioctl_t devio;

        devio.dev = unit;
        _ioctl(LUBDE_GET_BUS_FEATURES, &devio);
        *be_pio = devio.d0;
	*be_packet = devio.d1;
	*be_other = devio.d2;
    } else {
        *be_pio = _bus.be_pio;
        *be_packet = _bus.be_packet;
        *be_other = _bus.be_other;
    }
}

/* 
* Our BDE interface structure
*/
static ibde_t _ibde = {
    _name, 
    _num_devices, 
    _get_dev, 
    _get_dev_type, 
    _pci_config_get32, 
    _pci_config_put32,
    _bus_features,
    _read, 
    _write, 
    _salloc, 
    _sfree, 
    NULL, 
    NULL, 
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


/* 
 * Function: linux_bde_create
 *
 * Purpose:
 *    Creator function for this BDE interface. 
 * Parameters:
 *    bus - pointer to the bus features structure you want this 
 *          bde to export. Depends on the system. 
 *    ibde - pointer to a location to recieve the bde interface pointer. 
 * Returns:
 *    0 on success
 *    -1 on failure. 
 * Notes:
 *    This is the main BDE create function for this interface. 
 *    Used by the external system initialization code.
 */
int 
linux_bde_create(linux_bde_bus_t* bus, ibde_t** ibde)
{
    static int _init = 0;
    
    if (!_init) {
        if (_open() == -1) {       
            *ibde = NULL;
            return -1;
        }       
        _init = 1;
    }
    memset(&_bus, 0, sizeof(_bus));
    if (bus) {
        _bus = *bus;
    }
    *ibde = &_ibde;

#if LINUX_SPIDEV_SUPPORT
#ifdef BCM_ROBO_SUPPORT
    linux_spidev_setup();
#endif /* BCM_ROBO_SUPPORT */
#endif /* LINUX_SPIDEV_SUPPORT */

    return 0;
}
    
/*
 * Function: linux_bde_destroy
 *
 * Purpose:
 *    destroy this bde
 * Parameters:
 *    BDE interface pointer
 * Returns:
 *    0 on success, < 0 on error. 
 */
int
linux_bde_destroy(ibde_t* ibde)
{
#if LINUX_SPIDEV_SUPPORT
#ifdef BCM_ROBO_SUPPORT
    int ndevs;

    for (ndevs = 0; ndevs < _switch_ndevices; ndevs++) {
        if (spi_dev_linux[ndevs]) {
            _sys_spi_dev_deinit(ndevs);
            spi_dev_linux[ndevs] = 0;
        }
    }
#endif /* BCM_ROBO_SUPPORT */
#endif /* LINUX_SPIDEV_SUPPORT */

    return _close();
}

/*
 * Function: bde_irq_mask_set
 *
 * Purpose:
 *    Set interrupt mask from user space interrupt handler
 * Parameters:
 *    unit - unit number
 *    addr - PCI address of interrupt mask register
 *    mask - interrupt mask
 * Returns:
 *    0 on success, < 0 on error. 
 */
int
bde_irq_mask_set(int unit, uint32 addr, uint32 mask)
{
    lubde_ioctl_t devio;

    devio.dev = unit;
    devio.d0 = addr;
    devio.d1 = mask;
    _ioctl(LUBDE_WRITE_IRQ_MASK, &devio);
    return devio.rc;
}

#ifdef LINUX_SAL_USLEEP_OVERRIDE

/*
 * Function: sal_usleep
 *
 * Notes:
 *    See src/sal/core/unix/thread.c for details.
 */
void
sal_usleep(uint32 usec)
{
    lubde_ioctl_t devio;

    if (_devfd < 0 && _open() < 0) {
        /* Should not get here unless we are broken */
        sal_sleep(1);
        return;
    }
    devio.d0 = usec;
    _ioctl(LUBDE_USLEEP, &devio);
}

#endif /* LINUX_SAL_USLEEP_OVERRIDE */

#ifdef LINUX_SAL_UDELAY_OVERRIDE

/*
 * Function: sal_udelay
 *
 * Notes:
 *    See src/sal/core/unix/thread.c for details.
 */
void
sal_udelay(uint32 usec)
{
    lubde_ioctl_t devio;

    if (_devfd < 0) {
        return;
    }
    devio.d0 = usec;
    _ioctl(LUBDE_UDELAY, &devio);
}

#endif /* LINUX_SAL_UDELAY_OVERRIDE */

#ifdef LINUX_SAL_DMA_ALLOC_OVERRIDE

/*
 * Function: sal_dma_alloc
 *
 * Notes:
 *    See src/sal/core/unix/alloc.c for details.
 */
void *
sal_dma_alloc(unsigned int sz, char *name)
{
    return _ibde.salloc(0, sz, name); 
}

/*
 * Function: sal_dma_free
 *
 * Notes:
 *    See src/sal/core/unix/alloc.c for details.
 */
void
sal_dma_free(void *ptr)
{
    _ibde.sfree(0, ptr); 
}

#endif /* LINUX_SAL_DMA_ALLOC_OVERRIDE */

#ifdef LINUX_SAL_SEM_OVERRIDE

sal_sem_t
sal_sem_create(char *desc, int binary, int initial_count)
{
    lubde_ioctl_t devio;

    if (_devfd < 0 && _open() < 0) {
        /* Should not get here unless we are broken */
        return NULL;
    }
    devio.d0 = LUBDE_SEM_OP_CREATE;
    devio.d1 = binary;
    devio.d2 = initial_count;
    _ioctl(LUBDE_SEM_OP, &devio);
    return (sal_sem_t)devio.p0;
}

void
sal_sem_destroy(sal_sem_t b)
{
    lubde_ioctl_t devio;

    devio.d0 = LUBDE_SEM_OP_DESTROY;
    devio.p0 = b;
    _ioctl(LUBDE_SEM_OP, &devio);
}

int
sal_sem_take(sal_sem_t b, int usec)
{
    lubde_ioctl_t devio;

    devio.d0 = LUBDE_SEM_OP_TAKE;
    devio.p0 = b;
    devio.d2 = usec;
    _ioctl(LUBDE_SEM_OP, &devio);

    /* Handle SIGSTOP gracefully in user mode */
    while (devio.rc == -2) {
        _ioctl(LUBDE_SEM_OP, &devio);
    }

    return devio.rc;
}

int
sal_sem_give(sal_sem_t b)
{
    lubde_ioctl_t devio;

    devio.d0 = LUBDE_SEM_OP_GIVE;
    devio.p0 = b;
    _ioctl(LUBDE_SEM_OP, &devio);
    return devio.rc;
}

#endif /* LINUX_SAL_SEM_OVERRIDE */
