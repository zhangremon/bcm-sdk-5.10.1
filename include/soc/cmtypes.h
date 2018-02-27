/*
 * $Id: cmtypes.h 1.6.314.2 Broadcom SDK $
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

#ifndef _SOC_CMTYPES_H
#define _SOC_CMTYPES_H

#include <sal/types.h>
#include <sal/core/libc.h>

typedef struct soc_cm_device_info_s {
    uint16          dev_id;	       /* Chip ID as found in table */
    uint8           rev_id;
    const char     *dev_name;
    uint16          dev_id_driver;     /* Chip ID of driver to use */
    uint8           rev_id_driver;     /* (zero if same as real ID) */
    uint32          dev_type;
} soc_cm_device_info_t;

typedef void (*soc_cm_isr_func_t)(void *data);

typedef struct soc_cm_dev_s {
    int             dev;
    const soc_cm_device_info_t *info;
    void           *cookie;
    uint16          dev_id;	       /* True PCI chip ID */
    uint8           rev_id;
} soc_cm_dev_t;

typedef struct soc_cm_init_s {
    int             (*debug_out)(uint32 flags, const char *format,
				 va_list args);
    int             (*debug_check)(uint32 flags);
    int             (*debug_dump)(soc_cm_dev_t *dev);
} soc_cm_init_t;

typedef struct soc_cm_device_vectors_s {
    /* Specify if the vectors are initialized */
    int init;

    /* PCI Specific Endian Configuration */
    int big_endian_pio;
    int big_endian_packet;
    int big_endian_other;

    /* Communication bus type */
    uint32 bus_type;

    /* Startup Register Initialization */
    /* This should be changed to return a uint32 */

    char *(*config_var_get)(soc_cm_dev_t *dev, const char *name);

    /*
     * POLLING/INTERRUPT MANAGEMENT
     * --------------------
     * These routines provide the ability to install and remove
     * an interrupt handler for this device. 
     */

    /*
     * Syntax:    int interrupt_connect(int dev, cm_isr_func_t handler, 
     *                                  void *data)
     *
     * Purpose:   Install an interrupt handler for this device
     *
     * Parameters: 
     *             dev     - The device id (from cm_device_create())
     *             handler - The interrupt handler function. 
     *                       It must be called with the 'data' argument. 
     *             data    - Argument to be used when calling the handler
     *                       function at interrupt time.
     * Returns:
     *             0 if successful
     *             <0 if failed. 
     *
     */		   
    int (*interrupt_connect)(soc_cm_dev_t *dev, 
			     soc_cm_isr_func_t handler, void *data);
  
    /*
     * Syntax:   int interrupt_disconnect(int dev)
     *
     * Purpose:  remove an interrupt handler for this device
     *
     * Parameters: 
     *             dev     - The device id (from cm_device_create())
     *
     * Returns:
     *           0 if successful
     *           <0 if failed. 
     */
    int (*interrupt_disconnect)(soc_cm_dev_t *dev);
  
  



    /*
     * REGISTER ACCESS
     * ---------------
     */

    /* 
     * Syntax:     sal_vaddr_t base_address;
     * Purpose:    provides the ability to add an offset to 
     *             device addresses before accessors are called. 
     * 
     * If adding a base address to register offsets makes your
     * accessor functions easier, set it here. The value of base_address
     * will be added to all address offsets.  
     */
    sal_vaddr_t       base_address;

    /*
     * Syntax:    uint32 read(int dev, uint32 addr);
     *
     * Purpose:   Reads a device register. 
     * 
     * Parameters:
     *            dev     - The device id (from cm_device_create())
     *            addr -- register offset to read from. The value
     *                    of 'base_address' above will be added to 
     *                    the offset. 
     *
     * Returns:   the 32 bit register value. 
     *
     */
    uint32 (*read)(soc_cm_dev_t *dev, uint32 addr);


    /*
     * Syntax:    void write(int dev, uint32 addr, uint32 data);
     *
     * Purpose:   Writes a device register
     *
     * Parameters:
     *             dev     - The device id (from cm_device_create())
     *            addr -- register offset of the write. The value
     *                    of 'base_address' above will be added to 
     *                    the offset. 
     *            data -- 32 bit register data to be written. 
     */
    void (*write)(soc_cm_dev_t *dev, uint32 addr, uint32 data);
  
    /*
     * Syntax:    uint32 pci_conf_read(int dev, uint32 addr);
     *
     * Purpose:   Reads a PCI config register. 
     * 
     * Parameters:
     *            dev     - The device id (from cm_device_create())
     *            addr -- register offset to read from.
     *
     * Returns:   the 32 bit register value. 
     *
     */
    uint32 (*pci_conf_read)(soc_cm_dev_t *dev, uint32 addr);


    /*
     * Syntax:    void pci_conf_write(int dev, uint32 addr, uint32 data);
     *
     * Purpose:   Writes a device PCI configregister
     *
     * Parameters:
     *             dev     - The device id (from cm_device_create())
     *            addr -- register offset of the write.
     *            data -- 32 bit register data to be written. 
     */
    void (*pci_conf_write)(soc_cm_dev_t *dev, uint32 addr, uint32 data);

    /*
     * SHARED MEMORY ALLOCATION
     * ------------------------
     * Data communication between the driver and the device
     * is often accomplished through shared memory buffers. 
     * These buffers must usually be allocated in a special 
     * way to make them shareable between the system and the device. 
     * When the SOC driver needs to allocate on of these buffers, 
     * It will call these functions. 
     */

    /*
     * Syntax:    void *salloc(int dev, int size, name);
     *
     * Purpose:   Allocates a region of shareable memory.
     *
     * Parameters: 
     *             dev     - The device id (from cm_device_create())
     *            size -- The amount of memory to allocate. 
     *
     * Returns:   A pointer to the memory if successful. 
     *            NULL if a failure occurs. 
     *
     */
    void *(*salloc)(soc_cm_dev_t *dev, int size, const char *name);

    /*
     * Syntax:    void sfree(int dev, void *ptr); 
     * 
     * Purpose:   Deallocate a region of shareable memory. 
     * 
     * Parameters: 
     *             dev     - The device id (from cm_device_create())
     *            ptr -- A pointer to the memory region. 
     *
     * Returns: NONE
     *
     */
    void  (*sfree)(soc_cm_dev_t *dev, void *ptr);


    /* Flush/Invalidate shared memory */
    int (*sflush)(soc_cm_dev_t *dev, void *addr, int length);
    int (*sinval)(soc_cm_dev_t *dev, void *addr, int length);


    /* 
     * ADDRESS TRANSLATION
     *
     * Depending upon architecture, the address space the driver deals
     * with may be different from the address space the device deals
     * with. For this reason, and if this translation is required to
     * communicate with this device, you must provide the translation
     * functions to map between the two domains.
     *
     */

    /*
     * Syntax: uint32 l2p(soc_cm_dev_t *dev, void *addr);
     *
     * Purpose: Translate the logical address the driver uses to a
     *		physical address the device can access.
     *
     * Parameters:
     *             dev     - The device id (from cm_device_create())
     *          addr  -- Logical address to translate. 
     *
     * Returns:
     *          32 bit physical address the device can handle. 
     *
     * NOTE: The address to translate will always be within a region of memory
     *       allocated by the salloc() function above.
     */

    sal_paddr_t (*l2p)(soc_cm_dev_t *dev, void *addr);
  
    /*
     * Syntax: void *p2l(soc_cm_dev_t *dev, uint32 addr)
     *
     * Purpose: Translate the physical address the driver uses to a
     *		logical address the driver can access
     *
     * Parameters:
     *             dev     - The device id (from cm_device_create())
     *          addr -- physical device address
     *
     * Returns:
     *          Logical driver address
     *
     */

    void *(*p2l)(soc_cm_dev_t *dev, sal_paddr_t addr);
#ifdef BCM_ROBO_SUPPORT
    /*
     * Syntax:    
     *  void spi_read(soc_cm_dev_t *dev, uint32 addr, uint8 *buf, int len);
     *
     * Purpose:   Read a device register via spi interface
     *
     * Parameters:
     *             dev     - The device id (from cm_device_create())
     *            addr -- register offset of the write.
     *            data -- a ptr to the buffer to be read. 
     *             len -- length of total data to be read.
     */

    void  (*spi_read)(soc_cm_dev_t *dev, uint32 addr, uint8 *buf, int len);
    /*
     * Syntax:    
     *  void spi_write(soc_cm_dev_t *dev, uint32 addr, uint8 *buf, int len);
     *
     * Purpose:   Writes a device register via spi interface
     *
     * Parameters:
     *             dev     - The device id (from cm_device_create())
     *            addr -- register offset of the write. The value
     *                    of 'base_address' above will be added to 
     *                    the offset. 
     *            data -- a ptr to the buffer to be written. 
     *             len -- length of total data to be written.
     */

    void  (*spi_write)(soc_cm_dev_t *dev, uint32 addr, uint8 *buf, int len);
#ifdef INCLUDE_ROBO_I2C
    void  (*i2c_read)(soc_cm_dev_t *dev, uint16 addr, uint8 *buf, int len);
    void  (*i2c_write)(soc_cm_dev_t *dev, uint16 addr, uint8 *buf, int len);
    void  (*i2c_read_intr)(soc_cm_dev_t *dev, uint8 chipid, uint8 *buf, 
    		int len);
    void  (*i2c_read_ARA)(soc_cm_dev_t *dev, uint8 *chipid, int len);
#endif
#endif
} soc_cm_device_vectors_t;

#define init_register_get config_var_get	/* compat. */

#endif	/* !_SOC_CMTYPES_H */
