/* 
 * $Id: pciutil.c 1.1.2.14 Broadcom SDK $
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
 */
#if defined(PTP_KEYSTONE_STACK) 
#include <sal/types.h>
#include <sal/core/spl.h>
#include <sal/core/libc.h>
#include <soc/types.h>
#include <soc/cm.h>
#include <soc/cmext.h>
#include <soc/defs.h>
#include <soc/drv.h>
#include <assert.h>
#ifndef __KERNEL__
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#endif

#include <bcm/ptp.h>
#include <bcm/error.h>
#include <bcm_int/esw/ptp.h>

/* PCI configuration.*/
bcm_ptp_pci_setconfig_t esw_set_pciconfig_func;

/* Note: adjustments to this window base need to be coupled to the following read/write from the window */
/*       this is accomplished by disabling interrupts during the (very quick) access */
static uint32 cur_window_base = 0;
static uint32 keystone_dev_id = 0;

int esw_init_pci (bcm_ptp_pci_setconfig_t pci_setconfig, uint8 *window)
{
    esw_set_pciconfig_func = pci_setconfig;
    return 0;
}

/*
 * Function:
 *      _bcm_ptp_write_pcishared_uint32
 * Purpose:
 *      Write a word to PCI shared memory
 * Parameters:
 *      cookie  - (IN) unused
 *      addr    - (IN) address to write to
 *      val     - (IN) value to write
 * Returns:
 *      BCM_E_NONE
 * Notes:
 *      used for _bcm_ptp_stack_info_t->writed_fn
 */
int _bcm_ptp_write_pcishared_uint32(void *cookie, uint32 addr, uint32 val)
{
    uint32 new_window_base = (addr & 0xfffff000);
    uint32 offset = addr - new_window_base;
    volatile uint32 *mapped_addr = (volatile uint32*)(cookie + offset);
    int spl;
    int rv;
    
    spl = sal_splhi();

    if (cur_window_base != new_window_base) {
        cur_window_base = new_window_base;
        rv = esw_set_pciconfig_func(0x80, cur_window_base);
        if (rv != BCM_E_NONE)
            return rv;
    }    
    *mapped_addr = val;

    sal_spl(spl);
    return 0;
}

/*
 * Function:
 *      _bcm_ptp_read_pcishared_uint32
 * Purpose:
 *      Read a word from PCI shared memory
 * Parameters:
 *      cookie  - (IN) unused
 *      addr    - (IN) address to read from
 *      value   - (OUT) value read
 * Returns:
 *      BCM_E_NONE
 * Notes:
 *      used for _bcm_ptp_stack_info_t->read_fn
 */
int _bcm_ptp_read_pcishared_uint32(void *cookie, uint32 addr, uint32 *value)
{
    uint32 new_window_base = (addr & 0xfffff000);
    uint32 offset = addr - new_window_base;
    volatile uint32 *mapped_addr = (volatile uint32*)(cookie + offset);
    int rv;
    
    int spl = sal_splhi();

    if (cur_window_base != new_window_base) {
        cur_window_base = new_window_base;
        rv = esw_set_pciconfig_func(0x80, cur_window_base);
        if (rv != BCM_E_NONE)
            return rv;
    }    
    *value = *mapped_addr;

    sal_spl(spl);
    return BCM_E_NONE;
}

void esw_write_pcishared_uint8(bcm_ptp_external_stack_info_t *stack_p, uint32 addr, uint8 val)
{
    uint32 aligned_addr = (addr & 0xfffffffc);
    uint32 orig_word;
    unsigned shift;
    uint32 masked_word;
    uint32 new_word;
    
    stack_p->read_fn(stack_p->cookie, aligned_addr, &orig_word);

    /* Find shift needed to put the low byte of the word into the desired position */
    /* BCM53903 is big-endian, so first byte (offset 0) has shift 24,              */
    /*    last byte (offset 3) has shift 0                                         */
    shift = 8 * (3 - (addr - aligned_addr));

    masked_word = orig_word & ~(0xff << shift);
    new_word = masked_word | (((uint32)val) << shift);
   
    stack_p->write_fn(stack_p->cookie, aligned_addr, new_word);
}

/* Set a value both on BCM53903 and in the persistent config used to reset BCM53903 after a load. */
void
esw_set_ext_stack_config_uint32(_bcm_ptp_stack_info_t *stack_p, uint32 offset, uint32 value)
{
    _bcm_ptp_write_pcishared_uint32(stack_p->ext_info.cookie, CONFIG_BASE + offset, value);
    _bcm_ptp_uint32_write(&stack_p->persistent_config[offset], value);
}

/* Set an array of values as above */
void
esw_set_ext_stack_config_array(_bcm_ptp_stack_info_t* stack_p, uint32 offset, const uint8 * array, int len)
{
    while (len--) {
        esw_write_pcishared_uint8(&stack_p->ext_info, CONFIG_BASE + offset, *array);
        stack_p->persistent_config[offset] = *array;
        ++array;
        ++offset;
    }
}

/* Note, this implementation can be optimized if it is known that 8-bit accesses */
/*   via PCI shared memory work correctly.  This is meant to be a                */
/*   lowest-common-denominator portable version.                                 */

/* Note, this implementation can be optimized if it is known that 8-bit accesses */
/*   via PCI shared memory work correctly.  This is meant to be a                */
/*   lowest-common-denominator portable version.                                 */
uint8 _bcm_ptp_read_pcishared_uint8(bcm_ptp_external_stack_info_t *stack_p, uint32 addr)
{
    uint32 aligned_addr = (addr & 0xfffffffc);
    uint32 value32;
    unsigned shift;
    uint8 ret;

    stack_p->read_fn(stack_p->cookie, aligned_addr, &value32);

    /* Find shift needed to put the low byte of the word into the desired position */
    /* BCM53903 is big-endian, so first byte (offset 0) has shift 24,              */
    /*    last byte (offset 3) has shift 0                                         */
    shift = 8 * (3 - (addr - aligned_addr));
    ret = value32 >> shift;

    return ret;
}


void _bcm_ptp_write_pcishared_uint8_aligned_array(bcm_ptp_external_stack_info_t *stack_p, uint32 addr, uint8 * array, int array_len)
{
    while (array_len > 0) {
        uint32 value = ( (((uint32)array[0]) << 24) | (((uint32)array[1]) << 16) |
                         (((uint32)array[2]) << 8)  | (uint32)array[3] );
        _bcm_ptp_write_pcishared_uint32(stack_p->cookie, addr, value);
        array += 4;
        array_len -= 4;
        addr += 4;
    }
}


void _bcm_ptp_read_pcishared_uint8_aligned_array(bcm_ptp_external_stack_info_t *stack_p, uint32 addr, uint8 * array, int array_len)
{
    while (array_len > 0) {
        uint32 value;
        _bcm_ptp_read_pcishared_uint32(stack_p->cookie, addr, &value);
        array[0] = (value >> 24);
        array[1] = (value >> 16);
        array[2] = (value >> 8);
        array[3] = value;
        array += 4;
        array_len -= 4;
        addr += 4;
    }
}

int set_ext_stack_pciconfig(uint32 pciconfig_register, uint32 value) {
    soc_cm_pci_conf_write(keystone_dev_id, pciconfig_register, value);
    return 0;
}

#if defined(PTP_KEYSTONE_STACK)
int ks_pci_init(int ks_num, void** memBase)
{
    int dev;
    uint16 dev_id;
    uint8 rev_id;
    int dev_count;
    int ks_count = 0;
    uint32 base_addr;

    if (soc_cm_device_supported(BCM53000PCIE_DEVICE_ID, 
            BCM53000_A0_REV_ID)) {
        soc_cm_print("Keystone device not supported\n");
        return BCM_E_UNAVAIL;
    }

    dev_count = soc_cm_get_num_devices();
    for (dev = 0; dev < dev_count; dev++) {
        dev_id = 0;
        rev_id = 0;
        soc_cm_get_id(dev, &dev_id, &rev_id);
        if (dev_id == BCM53000PCIE_DEVICE_ID) {
            if (ks_count == ks_num) {
                keystone_dev_id = dev;
                /* This is the Keystone we want */
                base_addr = (uint32) soc_cm_get_base_address(dev);
                *memBase = (void *)base_addr;
                /*Update  Command/Status (0x4) <- 0x00100146 */
                return BCM_E_NONE;
            }
            ks_count++;
        }
    }
    return BCM_E_FAIL;
}
#endif
#endif /* defuned(PTP_KEYSTONE_STACK) */
