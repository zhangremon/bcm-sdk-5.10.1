/*
 * $Id: cm.h 1.12.120.1 Broadcom SDK $
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

#ifndef _SOC_CM_H
#define _SOC_CM_H

#include <sal/types.h>
#include <soc/cmtypes.h>
#include <soc/devids.h>
#include <soc/defs.h>		/* for SOC_MAX_NUM_DEVICES */

/* 
 * Configuration Manager
 *
 * This interface is for the driver side only
 *
 * All driver access to the chip must be through this
 * interface and this interface ONLY. 
 */

/* Names of debug flags by bit position (see soc/debug.h) */

extern char     *soc_cm_debug_names[32];

/* Debug Output */

extern int      soc_cm_print(const char *format, ...)
                             COMPILER_ATTRIBUTE ((format (printf, 1, 2)));
extern int      soc_cm_vprint(const char *fmt, va_list varg);
extern int      soc_cm_debug(uint32 flags, const char *format, ...)
                             COMPILER_ATTRIBUTE ((format (printf, 2, 3)));
extern int      soc_cm_debug_check(uint32 flags);
extern int      soc_cm_dump(int dev);


/* Register Initialization */

extern char     *soc_cm_config_var_get(int dev, const char *name);

/* Device information */

extern int      soc_cm_get_id(int dev, uint16 *dev_id, uint8 *rev_id);
extern int      soc_cm_get_id_driver(uint16 dev_id, uint8 rev_id,
                                     uint16 *dev_id_driver,
				     uint8 *rev_id_driver);
extern const char *soc_cm_get_name(int dev);
extern uint32 soc_cm_get_dev_type(int dev);
extern const char *soc_cm_get_device_name(uint16 dev_id, uint8 rev_id);
extern int      soc_cm_get_num_devices(void);
extern int      soc_cm_get_endian(int dev, int *pio, int *packet, int *other);
extern sal_vaddr_t   soc_cm_get_base_address(int unit);

/* Device interrupts */

extern int
soc_cm_interrupt_connect(int unit, soc_cm_isr_func_t f, void *data);

extern int      soc_cm_interrupt_enable(int unit);
extern void     soc_cm_interrupt_disable(int unit);
extern int      soc_cm_interrupt_disconnect(int unit);

/* Device Shared Memory Management */

extern void     *soc_cm_salloc(int unit, int size, const char *name);
extern void     soc_cm_sfree(int unit, void *ptr);
extern int      soc_cm_sflush(int unit, void *addr, int length);
extern int      soc_cm_sinval(int unit, void *addr, int length);

/* Device PCI config read/write */

extern void     soc_cm_pci_conf_write(int dev, uint32 addr, uint32 data);

/* Device Address Translations */

extern sal_paddr_t   soc_cm_l2p(int unit, void *addr);
extern void     *soc_cm_p2l(int unit, sal_paddr_t addr);

extern void     soc_cm_display_known_devices(void);

extern uint32  soc_cm_get_bus_type(int dev);

/* Register Access */

typedef struct {
    soc_cm_dev_t		dev;
    soc_cm_device_vectors_t	vectors;
} cm_device_t;

extern cm_device_t              soc_cm_device[SOC_MAX_NUM_DEVICES];
extern int                      soc_cm_device_count;

#define	CMDEV(dev)		soc_cm_device[dev]
#define	CMVEC(dev)		CMDEV(dev).vectors

/*
 * There are actually several different implementations that can be
 * configured for CMREAD and CMWRITE (which are effectively, pci read
 * and write).
 *
 * 1. If SOC_CM_MEMORY_BASE is defined, then that macro contains
 *    the constant base address of all pci devices.  The macro
 *    SOC_CM_MEMORY_OFFSET will be multiplied by the device and added
 *    to the base to create the actual pci address.  The cm vector
 *    base addresses will be checked against this.
 * 2. If SOC_CM_MEMORY is defined then the cm vector base_address
 *    will be used as a directo derefence to access pci space
 * 3. If SOC_CM_FUNCTION is defined then the cm vector read and
 *    write routines will be called.
 * 4. If none of the above, then a runtime check of both the cm vector
 *    base_address and the read/write routines will happen.
 */
#ifdef KEYSTONE
/*
 * Enforce PCIE transaction ordering. Commit the write transaction.
 */
#define _SSOC_CMREAD(_d,_a) ({__asm__ __volatile__("sync"); SOC_E_NONE;})
#else
#define _SSOC_CMREAD(_d,_a) SOC_E_NONE
#endif

#ifdef	SOC_CM_MEMORY_BASE
#ifdef  EXTERN_SOC_CM_MEMORY_BASE
extern uint32 EXTERN_SOC_CM_MEMORY_BASE;
#endif
#define	CMREAD(_d,_a)	\
	(((VOL uint32 *)(SOC_CM_MEMORY_BASE+(SOC_CM_MEMORY_OFFSET*_d)))[(_a)/4])
#define	CMWRITE(_d,_a,_data)	\
	((CMREAD(_d,_a) = _data), _SSOC_CMREAD(_d,_a))
#else
#ifdef	SOC_CM_MEMORY
#define	CMREAD(_d,_a)	\
	(((VOL uint32 *)CMVEC(_d).base_address)[(_a)/4])
#define	CMWRITE(_d,_a,_data)	\
	((CMREAD(_d,_a) = _data), _SSOC_CMREAD(_d,_a))
#else
#ifdef	SOC_CM_FUNCTION
#define	CMREAD(_d,_a)	\
	(CMVEC(_d).read(&CMDEV(_d).dev, _a))
#define	CMWRITE(_d,_a,_data)	\
	((CMVEC(_d).write(&CMDEV(_d).dev, _a, _data)), _SSOC_CMREAD(_d,_a))
#else
#define	CMREAD(_d,_a)	\
	(CMVEC(_d).base_address ? \
	    ((VOL uint32 *)CMVEC(_d).base_address)[(_a)/4] : \
	    CMVEC(_d).read(&CMDEV(_d).dev, _a))
#define	CMWRITE(_d,_a,_data)	\
	(CMVEC(_d).base_address ? \
	    ((void)(((VOL uint32 *)CMVEC(_d).base_address)[(_a)/4] = _data), _SSOC_CMREAD(_d,_a)) : \
	    (CMVEC(_d).write(&CMDEV(_d).dev, _a, _data), _SSOC_CMREAD(_d,_a)))
#endif	/* SOC_CM_FUNCTION */
#endif	/* SOC_CM_MEMORY */
#endif	/* SOC_CM_MEMORY_BASE */

#define	CMCONFREAD(_d,_a)	\
	(CMVEC(_d).pci_conf_read(&CMDEV(_d).dev, _a))
#define	CMCONFWRITE(_d,_a,_data)	\
	(CMVEC(_d).pci_conf_write(&CMDEV(_d).dev, _a, _data))

#endif  /* !_SOC_CM_H */
