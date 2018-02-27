/*
 * $Id: linux-user-bde.c 1.39.20.4 Broadcom SDK $
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
 * Linux User BDE Helper Module
 */
#include <gmodule.h>
#include <mpool.h>
#include <linux-bde.h>

#include <sal/core/thread.h>
#include <sal/core/sync.h>
#include <soc/cmic.h>
#include <soc/devids.h>

#ifdef BCM_CMICM_SUPPORT
#include <soc/cmicm.h>
#endif

#include "linux-user-bde.h"

MODULE_AUTHOR("Broadcom Corporation");
MODULE_DESCRIPTION("User BDE Helper Module");
MODULE_LICENSE("Proprietary");

static ibde_t *user_bde = NULL;

typedef struct bde_ctrl_s {
    uint32 dev_type;
    int irq;
    int enabled;
    int devid;
    uint32 *ba;
} bde_ctrl_t;

static bde_ctrl_t _devices[LINUX_BDE_MAX_DEVICES];

static int _ipid;
static int _ether_ipid;

static wait_queue_head_t _interrupt_wq;
static atomic_t _interrupt_has_taken_place = ATOMIC_INIT(0);

static wait_queue_head_t _ether_interrupt_wq;
static atomic_t _ether_interrupt_has_taken_place = ATOMIC_INIT(0);

typedef void (*isr_f)(void *);


#ifdef KEYSTONE
/*
 * Enforce PCIE transaction ordering. Commit the write transaction.
 */

#define SSOC_WRITEL(val, addr)                  \
            do {                                \
                writel((val), (addr));          \
                __asm__ __volatile__("sync");   \
            } while(0)

#else

#define SSOC_WRITEL(val, addr) \
            writel((val), (addr))

#endif
/*
 * Function: _interrupt
 *
 * Purpose:
 *    Interrupt Handler.
 *    Mask all interrupts on device and wake up interrupt
 *    thread. It is assumed that the interrupt thread unmasks
 *    interrupts again when interrupt handling is complete.
 * Parameters:
 *    ctrl - BDE control structure for this device.
 * Returns:
 *    Nothing
 */
static void 
_cmic_interrupt(bde_ctrl_t *ctrl)
{
    int d;

    d = (((uint8 *)ctrl - (uint8 *)_devices) / sizeof (bde_ctrl_t));

    lkbde_irq_mask_set(d, CMIC_IRQ_MASK, 0, 0);
    atomic_set(&_interrupt_has_taken_place, 1);
#ifdef BDE_LINUX_NON_INTERRUPTIBLE
    wake_up(&_interrupt_wq);
#else
    wake_up_interruptible(&_interrupt_wq);
#endif
}

#ifdef BCM_CMICM_SUPPORT

#define CMICM_DEV_REV_ID        (0x10224 / sizeof(uint32))

#ifndef BDE_CMICM_PCIE_CMC
#define BDE_CMICM_PCIE_CMC      0
#endif

static void 
_cmicm_interrupt(bde_ctrl_t *ctrl)
{
    int d;
    int cmc = BDE_CMICM_PCIE_CMC;

    d = (((uint8 *)ctrl - (uint8 *)_devices) / sizeof (bde_ctrl_t));

    lkbde_irq_mask_set(d, CMIC_CMCx_PCIE_IRQ_MASK0_OFFSET(cmc), 0, 0);
    lkbde_irq_mask_set(d, CMIC_CMCx_PCIE_IRQ_MASK1_OFFSET(cmc), 0, 0);
    atomic_set(&_interrupt_has_taken_place, 1);
#ifdef BDE_LINUX_NON_INTERRUPTIBLE
    wake_up(&_interrupt_wq);
#else
    wake_up_interruptible(&_interrupt_wq);
#endif
}

#endif /* BCM_CMICM_SUPPORT */

static void
_qe2k_interrupt(bde_ctrl_t *ctrl)
{
    SSOC_WRITEL(0xffffffff, ctrl->ba + 0x20/sizeof(uint32));

    atomic_set(&_interrupt_has_taken_place, 1);
#ifdef BDE_LINUX_NON_INTERRUPTIBLE
    wake_up(&_interrupt_wq);
#else
    wake_up_interruptible(&_interrupt_wq);
#endif
}

static void
_fe2k_interrupt(bde_ctrl_t *ctrl)
{
    SSOC_WRITEL(0xffffffff, ctrl->ba + 0x18/sizeof(uint32)); /* PC_INTERRUPT_MASK */
    SSOC_WRITEL(0xffffffff, ctrl->ba + 0x24/sizeof(uint32)); /* PC_ERROR0_MASK    */
    SSOC_WRITEL(0xffffffff, ctrl->ba + 0x2c/sizeof(uint32)); /* PC_ERROR1_MASK    */
    SSOC_WRITEL(0xffffffff, ctrl->ba + 0x34/sizeof(uint32)); /* PC_UNIT_MASK      */

    atomic_set(&_interrupt_has_taken_place, 1);
#ifdef BDE_LINUX_NON_INTERRUPTIBLE
    wake_up(&_interrupt_wq);
#else
    wake_up_interruptible(&_interrupt_wq);
#endif
}

static void
_fe2kxt_interrupt(bde_ctrl_t *ctrl)
{
    SSOC_WRITEL(0xffffffff, ctrl->ba + 0x2c/sizeof(uint32)); /* PC_INTERRUPT_MASK */
    SSOC_WRITEL(0xffffffff, ctrl->ba + 0x38/sizeof(uint32)); /* PC_ERROR0_MASK    */
    SSOC_WRITEL(0xffffffff, ctrl->ba + 0x40/sizeof(uint32)); /* PC_ERROR1_MASK    */
    SSOC_WRITEL(0xffffffff, ctrl->ba + 0x50/sizeof(uint32)); /* PC_UNIT_MASK      */

    atomic_set(&_interrupt_has_taken_place, 1);
#ifdef BDE_LINUX_NON_INTERRUPTIBLE
    wake_up(&_interrupt_wq);
#else
    wake_up_interruptible(&_interrupt_wq);
#endif
}

static void
_bme3200_interrupt(bde_ctrl_t *ctrl)
{
    SSOC_WRITEL(0xffffffff, ctrl->ba + 0x54/sizeof(uint32)); /* PI_PT_ERROR0 */
    SSOC_WRITEL(0xffffffff, ctrl->ba + 0x5c/sizeof(uint32)); /* PI_PT_ERROR1 */
    SSOC_WRITEL(0xffffffff, ctrl->ba + 0x64/sizeof(uint32)); /* PI_PT_ERROR2 */
    SSOC_WRITEL(0xffffffff, ctrl->ba + 0x6c/sizeof(uint32)); /* PI_PT_ERROR3 */

    atomic_set(&_interrupt_has_taken_place, 1);
#ifdef BDE_LINUX_NON_INTERRUPTIBLE
    wake_up(&_interrupt_wq);
#else
    wake_up_interruptible(&_interrupt_wq);
#endif
}


static void
_bm9600_interrupt(bde_ctrl_t *ctrl)
{
    SSOC_WRITEL(0xffffffff, ctrl->ba + 0x5c/sizeof(uint32));  /* PI_INTERRUPT_MASK */
    SSOC_WRITEL(0xffffffff, ctrl->ba + 0xc/sizeof(uint32));   /* PI_UNIT_INTERRUPT0_MASK */
    SSOC_WRITEL(0xffffffff, ctrl->ba + 0x14/sizeof(uint32));  /* PI_UNIT_INTERRUPT1_MASK */
    SSOC_WRITEL(0xffffffff, ctrl->ba + 0x1c/sizeof(uint32));  /* PI_UNIT_INTERRUPT2_MASK */
    SSOC_WRITEL(0xffffffff, ctrl->ba + 0x24/sizeof(uint32));  /* PI_UNIT_INTERRUPT3_MASK */
    SSOC_WRITEL(0xffffffff, ctrl->ba + 0x2c/sizeof(uint32));  /* PI_UNIT_INTERRUPT4_MASK */
    SSOC_WRITEL(0xffffffff, ctrl->ba + 0x34/sizeof(uint32));  /* PI_UNIT_INTERRUPT5_MASK */
    SSOC_WRITEL(0xffffffff, ctrl->ba + 0x3c/sizeof(uint32));  /* PI_UNIT_INTERRUPT6_MASK */
    SSOC_WRITEL(0xffffffff, ctrl->ba + 0x44/sizeof(uint32));  /* PI_UNIT_INTERRUPT7_MASK */
    SSOC_WRITEL(0xffffffff, ctrl->ba + 0x4c/sizeof(uint32));  /* PI_UNIT_INTERRUPT8_MASK */
    SSOC_WRITEL(0xffffffff, ctrl->ba + 0x54/sizeof(uint32));  /* PI_UNIT_INTERRUPT9_MASK */

    atomic_set(&_interrupt_has_taken_place, 1);
#ifdef BDE_LINUX_NON_INTERRUPTIBLE
    wake_up(&_interrupt_wq);
#else
    wake_up_interruptible(&_interrupt_wq);
#endif
}



/* The actual interrupt handler of ethernet devices */
static void 
_ether_interrupt(bde_ctrl_t *ctrl)
{
    SSOC_WRITEL(0, ctrl->ba + 0x024/4); 

    atomic_set(&_ether_interrupt_has_taken_place, 1);
#ifdef BDE_LINUX_NON_INTERRUPTIBLE
    wake_up(&_ether_interrupt_wq);
#else
    wake_up_interruptible(&_ether_interrupt_wq);
#endif
}



/*
 * Function: _init
 *
 * Purpose:
 *    Module initialization.
 *    Attaches to kernel BDE.
 * Parameters:
 *    None
 * Returns:
 *    Always 0
 */
static int
_init(void)
{
    int i;

    /* Connect to the kernel bde */
    if ((linux_bde_create(NULL, &user_bde) < 0) || user_bde == NULL) {
        return -ENODEV;
    }

    memset(_devices, 0, sizeof(_devices));

    for (i = 0; i < user_bde->num_devices(BDE_ALL_DEVICES); i++) {
        /* Initialize our control info */
        _devices[i].dev_type = user_bde->get_dev_type(i);
        _devices[i].devid = user_bde->get_dev(i)->device;
        if (_devices[i].dev_type & 
            (BDE_PCI_DEV_TYPE | BDE_ICS_DEV_TYPE | BDE_EB_DEV_TYPE)) {
            _devices[i].enabled = 0;
            _devices[i].ba = lkbde_get_dev_virt(i);
        }
    }
    return 0;
}

/*
 * Function: _cleanup
 *
 * Purpose:
 *    Module cleanup function.
 * Parameters:
 *    None
 * Returns:
 *    Always 0
 */
static int
_cleanup(void)
{
    int i;

    if (user_bde) {
	for (i = 0; i < user_bde->num_devices(BDE_ALL_DEVICES); i++) {
	    if (_devices[i].enabled &&
	       (_devices[i].dev_type & 
                (BDE_PCI_DEV_TYPE | BDE_ICS_DEV_TYPE | BDE_EB_DEV_TYPE))) {
                user_bde->interrupt_disconnect(i);
            }
        }
        linux_bde_destroy(user_bde);
        user_bde = NULL;
    }
    return 0;
}

/*
 * Function: _pprint
 *
 * Purpose:
 *    Print proc filesystem information.
 * Parameters:
 *    None
 * Returns:
 *    Always 0
 */
static int
_pprint(void)
{
    pprintf("Broadcom Device Enumerator (%s)\n", LINUX_USER_BDE_NAME);
    return 0;
}

/*
 * Function: _ioctl
 *
 * Purpose:
 *    Handle IOCTL commands from user mode.
 * Parameters:
 *    cmd - IOCTL cmd
 *    arg - IOCTL parameters
 * Returns:
 *    Always 0
 */
static int 
_ioctl(unsigned int cmd, unsigned long arg)
{
    lubde_ioctl_t io;
    int i;
    uint32 pbase, size;
    const ibde_dev_t *bde_dev;
    isr_f v;

    if (copy_from_user(&io, (void *)arg, sizeof(io))) {
        return -EFAULT;
    }
  
    io.rc = LUBDE_SUCCESS;
  
    switch(cmd) {
    case LUBDE_VERSION:
        io.d0 = 0;
        break;
    case LUBDE_GET_NUM_DEVICES:
        io.d0 = user_bde->num_devices(io.dev);
        break;
    case LUBDE_GET_DEVICE:
        bde_dev = user_bde->get_dev(io.dev);
        if (bde_dev) {
            io.d0 = bde_dev->device;
            io.d1 = bde_dev->rev;
            if (_devices[io.dev].dev_type & 
                (BDE_PCI_DEV_TYPE | BDE_ICS_DEV_TYPE |
                 BDE_EB_DEV_TYPE)) {
                /* Get physical address to map */
                io.d2 = lkbde_get_dev_phys(io.dev);
                io.d3 = lkbde_get_dev_phys_hi(io.dev);
            }
        } else {
            io.rc = LUBDE_FAIL;
        }
        break;
    case LUBDE_GET_DEVICE_TYPE:
        io.d0 = _devices[io.dev].dev_type;
        break;
    case LUBDE_GET_BUS_FEATURES:
        user_bde->pci_bus_features(io.dev, (int *) &io.d0, (int *) &io.d1,
                                   (int *) &io.d2);
        break;
    case LUBDE_PCI_CONFIG_PUT32:
        if (_devices[io.dev].dev_type & BDE_PCI_DEV_TYPE) {
            user_bde->pci_conf_write(io.dev, io.d0, io.d1);
        } else {
            io.rc = LUBDE_FAIL;
        }
        break;
    case LUBDE_PCI_CONFIG_GET32:
        if (_devices[io.dev].dev_type & BDE_PCI_DEV_TYPE) {
            io.d0 = user_bde->pci_conf_read(io.dev, io.d0);
        } else {
            io.rc = LUBDE_FAIL;
        }
        break;
    case LUBDE_GET_DMA_INFO:
        lkbde_get_dma_info(&pbase, &size);
        io.d0 = pbase;
        io.d1 = size; 
        break;
    case LUBDE_ENABLE_INTERRUPTS:
        /* This code assumes that the first switch device is device 0 */
        if (io.dev == 0 && user_bde->num_devices(BDE_SWITCH_DEVICES) > 0) {
            /* Process all active switching interrupts */
            init_waitqueue_head(&_interrupt_wq);
            _ipid = current->pid;
            for (i = 0; i < user_bde->num_devices(BDE_ALL_DEVICES); i++) {
                if ((_devices[i].dev_type & BDE_SWITCH_DEV_TYPE) &&
                    !_devices[i].enabled) {
                    switch (user_bde->get_dev(i)->device) {
                    case QE2000_DEVICE_ID:
                        v = (isr_f) _qe2k_interrupt;
                        break;
                    case BCM88020_DEVICE_ID:
                        v = (isr_f) _fe2k_interrupt;
                        break;
                    case BCM88025_DEVICE_ID:
                        v = (isr_f) _fe2kxt_interrupt;
                        break;
                    case BME3200_DEVICE_ID:
                        v = (isr_f) _bme3200_interrupt;
                        break;
                    case BM9600_DEVICE_ID:
                        v = (isr_f) _bm9600_interrupt;
                        break;
                    default:
                        v = (isr_f) _cmic_interrupt;
#ifdef BCM_CMICM_SUPPORT
                        if ((_devices[i].dev_type & BDE_256K_REG_SPACE) &&
                            readl(_devices[i].ba + CMICM_DEV_REV_ID) != 0) {
                            v = (isr_f) _cmicm_interrupt;
                        }
#endif
                        break;
                    }
                    user_bde->interrupt_connect(i, v, _devices+i);
                    _devices[i].enabled = 1;
                }
            }
        } else {
            /* Process ethernet device interrupt */
            init_waitqueue_head(&_ether_interrupt_wq);
            
            _ether_ipid = current->pid;
            if (!_devices[io.dev].enabled) {
                user_bde->interrupt_connect(io.dev,
                                            (void(*)(void *))_ether_interrupt, 
                                            _devices+io.dev);
                _devices[io.dev].enabled = 1;
            }
        }
        break;
    case LUBDE_DISABLE_INTERRUPTS:
        /* This code assumes that the first switch device is device 0 */
        if (io.dev == 0 && user_bde->num_devices(BDE_SWITCH_DEVICES) > 0) {
            for (i = 0; i < user_bde->num_devices(BDE_ALL_DEVICES); i++) {
                if ((_devices[i].dev_type & BDE_SWITCH_DEV_TYPE) &&
                    _devices[i].enabled) {
                    user_bde->interrupt_disconnect(i);
                    _devices[i].enabled = 0;
                } 
            }
        } else {
            if (_devices[io.dev].enabled) {
                user_bde->interrupt_disconnect(io.dev);
                _devices[io.dev].enabled = 0;
            }
        }
        break;
    case LUBDE_WAIT_FOR_INTERRUPT:
        /* This code assumes that the first switch device is device 0 */
        if (io.dev == 0 && user_bde->num_devices(BDE_SWITCH_DEVICES) > 0) {
#ifdef BDE_LINUX_NON_INTERRUPTIBLE
            wait_event_timeout(_interrupt_wq, 
			       atomic_read(&_interrupt_has_taken_place) != 0, 100);
#else
            wait_event_interruptible(_interrupt_wq, 
                                     atomic_read(&_interrupt_has_taken_place) != 0);
#endif
            /* 
             * Even if we get multiple interrupts, we 
             * only run the interrupt handler once.
             */
            atomic_set(&_interrupt_has_taken_place, 0);
        } else {
#ifdef BDE_LINUX_NON_INTERRUPTIBLE
            wait_event_timeout(_ether_interrupt_wq,     
			       atomic_read(&_ether_interrupt_has_taken_place) != 0,100);
#else
            wait_event_interruptible(_ether_interrupt_wq,     
                                     atomic_read(&_ether_interrupt_has_taken_place) != 0);
#endif
            /* 
             * Even if we get multiple interrupts, we 
             * only run the interrupt handler once.
             */
            atomic_set(&_ether_interrupt_has_taken_place, 0);
        }
        break;
    case LUBDE_USLEEP:
        sal_usleep(io.d0);
        break;
    case LUBDE_UDELAY:
        sal_udelay(io.d0);
        break;
    case LUBDE_SEM_OP:
        switch (io.d0) {
        case LUBDE_SEM_OP_CREATE:
            io.p0 = sal_sem_create("", io.d1, io.d2);
            break;
        case LUBDE_SEM_OP_DESTROY:
            sal_sem_destroy((sal_sem_t)io.p0);
            break;
        case LUBDE_SEM_OP_TAKE:
            io.rc = sal_sem_take((sal_sem_t)io.p0, io.d2);
            break;
        case LUBDE_SEM_OP_GIVE:
            io.rc = sal_sem_give((sal_sem_t)io.p0);
            break;
        default:
            io.rc = LUBDE_FAIL;
            break;
        }
        break;
    case LUBDE_WRITE_IRQ_MASK:
        io.rc = lkbde_irq_mask_set(io.dev, io.d0, io.d1, 0);
        break;
    case LUBDE_SPI_READ_REG:
        if (user_bde->spi_read(io.dev, io.d0, io.dx.buf, io.d1) == -1) {
            io.rc = LUBDE_FAIL;
        } 
        break;
    case LUBDE_SPI_WRITE_REG:
        if (user_bde->spi_write(io.dev, io.d0, io.dx.buf, io.d1) == -1) {
            io.rc = LUBDE_FAIL;
        }
        break;
    case LUBDE_I2C_READ_REG:
        if (user_bde->i2c_read(io.dev, io.d0, io.dx.buf, io.d1) == -1) {
            io.rc = LUBDE_FAIL;
        } 
        break;
    case LUBDE_I2C_WRITE_REG:
        if (user_bde->i2c_write(io.dev, io.d0, io.dx.buf, io.d1) == -1) {
            io.rc = LUBDE_FAIL;
        }
        break;
    case LUBDE_I2C_READ_INTR_REG:
        if (user_bde->i2c_read_intr(io.dev, io.d0, io.dx.buf, io.d1) == -1) {
            io.rc = LUBDE_FAIL;
        } 
        break;
    case LUBDE_I2C_READ_ARA:
        if (user_bde->i2c_read_ARA(io.dev, io.dx.buf, io.d1) == -1) {
            io.rc = LUBDE_FAIL;
        } 
        break;
    case LUBDE_READ_REG_16BIT_BUS:
        io.d1 = user_bde->read(io.dev, io.d0);
        break;
    case LUBDE_WRITE_REG_16BIT_BUS:
        io.rc = user_bde->write(io.dev, io.d0, io.d1);
        break;
    default:
        gprintk("Error: Invalid ioctl (%08x)\n", cmd);
        io.rc = LUBDE_FAIL;
        break;
    }
  
    if (copy_to_user((void *)arg, &io, sizeof(io))) {
        return -EFAULT;
    }

    return 0;
}

/* Workaround for broken Busybox/PPC insmod */
static char _modname[] = LINUX_USER_BDE_NAME;

static gmodule_t _gmodule = 
{
    name: LINUX_USER_BDE_NAME, 
    major: LINUX_USER_BDE_MAJOR, 
    init: _init, 
    cleanup: _cleanup, 
    pprint: _pprint, 
    ioctl: _ioctl, 
}; 

gmodule_t*
gmodule_get(void)
{
    _gmodule.name = _modname;
    return &_gmodule;
}
