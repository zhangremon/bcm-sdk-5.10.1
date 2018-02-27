/*
 * $Id: stubs.c 1.6 Broadcom SDK $
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

#include <sal/appl/pci.h>
#include <appl/diag/sysconf.h>
#include <appl/diag/system.h>
#include <appl/stktask/attach.h>

/*
 * Satisfy shell.c w/o BCMX library
 */

int bcmx_unit_count;

#if defined(BROADCOM_DEBUG)
uint32 bcmx_debug_level;
char *bcmx_debug_names[] = {""};
#endif /*defined(BROADCOM_DEBUG)*/


/* 
 * Satisfy dispatch.c w/o stacking library
 */

int
bcm_stack_attach_register(bcm_stack_attach_cb_f callback)
{
    return 0;
}

int
bcm_stack_attach_unregister(bcm_stack_attach_cb_f callback)
{
    return 0;
}

/*
 * These stubs are here for legacy compatability reasons. 
 * They are used only by the diag/test code, not the driver, 
 * so they are really not that important. 
 */

int
pci_dma_putw(pci_dev_t *dev, sal_paddr_t addr, unsigned int data)
{
  void *vaddr = soc_cm_p2l(0,addr);

  *((unsigned int *)vaddr) = data;
  return 0;
}

int
pci_dma_puth(pci_dev_t *dev, sal_paddr_t addr, unsigned short data)
{
  void *vaddr = soc_cm_p2l(0,addr);

  *((unsigned short *)vaddr) = data;
  return 0;
}

int
pci_dma_putb(pci_dev_t *dev, sal_paddr_t addr, unsigned char data)
{
  void *vaddr = soc_cm_p2l(0,addr);

  *((unsigned char *)vaddr) = data;
  return 0;
}

unsigned int
pci_dma_getw(pci_dev_t *dev, sal_paddr_t addr)
{
  void *vaddr = soc_cm_p2l(0,addr);
  unsigned int data;

  data = *((unsigned int *)vaddr);
  return data;
}

unsigned short
pci_dma_geth(pci_dev_t *dev, sal_paddr_t addr)
{
  void *vaddr = soc_cm_p2l(0,addr);
  unsigned short data;

  data = *((unsigned short *)vaddr);
  return data;
}

unsigned char
pci_dma_getb(pci_dev_t *dev, sal_paddr_t addr)
{
  void *vaddr = soc_cm_p2l(0,addr);
  unsigned char data;

  data = *((unsigned char *)vaddr);
  return data;
}

void
pci_print_all(void)
{
}