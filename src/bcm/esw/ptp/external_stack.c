/*
 * $Id: external_stack.c 1.1.2.10 Broadcom SDK $
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
#if defined(INCLUDE_PTP)

#include <soc/defs.h>
#include <soc/drv.h>

#include <sal/appl/io.h>
#include <sal/core/dpc.h>

#include <bcm/ptp.h>
#include <bcm_int/esw/ptp.h>
#include <bcm/error.h>

#include <bcm_int/esw/ptp.h>

#if defined(PTP_KEYSTONE_STACK)
#include "ext_stack_fw.h"

/* Constants */
#define FAULT_CHECK_US (100000)  /* every .1 seconds */

/* Event support (ToP OUT-OF-MEMORY).*/
#define TOP_OOM_MSGDATA_SIZE_OCTETS                 (8)
#define TOP_OOM_MINIMUM_FREE_MEMORY_THRESHOLD_BYTES (1024)   
#define TOP_OOM_ORDBLK_FREE_MEMORY_THRESHOLD_BYTES  (1024)

extern int ks_pci_init(int ks_num, void** memBase);
extern int esw_init_pci (bcm_ptp_pci_setconfig_t pci_setconfig, uint8 *window);
extern int set_ext_stack_pciconfig(uint32 pciconfig_register, uint32 value);

/* Static functions */
static int _bcm_ptp_ext_stack_firmware_load(
    _bcm_ptp_stack_info_t *stack_p, 
    int size, 
    uint8 *image, 
    _bcmPTPBootType_t mode);
#endif

/*
 * Function:
 *      _bcm_ptp_external_stack_create
 * Purpose:
 *      Create a PTP stack instance
 * Parameters:
 *      unit - (IN) Unit number.
 *      ptp_info - (IN/OUT) Pointer to an PTP Stack Info structure
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
_bcm_ptp_external_stack_create(
    int unit, 
    bcm_ptp_stack_info_t *info,
    bcm_ptp_stack_id_t ptp_id)
{
#if defined(PTP_KEYSTONE_STACK)
    int rv = BCM_E_NONE;
    _bcm_ptp_info_t *ptp_info_p;
    _bcm_ptp_stack_info_t *stack_p;
    uint32 tpidvlan;
    void *cookie;

    SET_PTP_INFO;
    if (soc_feature(unit, soc_feature_ptp))
    {
        stack_p = &ptp_info_p->stack_info[ptp_id];

        /* Set up dispatch for external transport */
        stack_p->transport_init = _bcm_ptp_external_transport_init;
        stack_p->tx = _bcm_ptp_external_tx;
        stack_p->rx_free = _bcm_ptp_external_rx_response_free;
        /* TODO */
        /* stack_p->transport_terminate = _bcm_ptp_external_transport_terminate; */

        /* Assuming that the unit has been locked by the caller */
        sal_memcpy(&stack_p->ext_info, info->ext_stack_info, sizeof(bcm_ptp_external_stack_info_t));

        /* Set the PCI read and write functions */
        if (BCM_FAILURE(rv = ks_pci_init(info->id, &stack_p->ext_info.cookie))) {
            PTP_ERROR_FUNC("ks_pci_init() failed\n");
            return rv;
        }
        
        stack_p->ext_info.read_fn = &_bcm_ptp_read_pcishared_uint32;
        stack_p->ext_info.write_fn = &_bcm_ptp_write_pcishared_uint32;

        /* Write values for BCM53903 */
        cookie = stack_p->ext_info.cookie;
        esw_init_pci(&set_ext_stack_pciconfig, cookie);

        /* reset Core DMP */
        /* resetctrl (0x18103800) <- 0x0 (bit[0] <- 0 exit reset) */
        stack_p->ext_info.write_fn(cookie, 0x18103800, 1);

        /* reset ChipCommon */
        /* resetctrl (0x18100800) <- 0x0 (bit[0] <- 0 exit reset) */
        stack_p->ext_info.write_fn(cookie, 0x18100800, 1);

        sal_usleep(1000);  /* delay 1ms */
    
        /* enable Core DMP */
        /* ioctrl (0x18103408) <- 0x1 (bit[0] <- 1 to enable clock) */
        stack_p->ext_info.write_fn(cookie, 0x18103408, 1);

        /* resetctrl (0x18103800) <- 0x0 (bit[0] <- 0 exit reset)    */
        stack_p->ext_info.write_fn(cookie, 0x18103800, 0);

        /* enable ChipCommon */
        /* ioctrl (0x18100408) <- 0x1 (bit[0] <- 1 to enable clock) */
        stack_p->ext_info.write_fn(cookie, 0x18100408, 1);

        /* resetctrl (0x18100800) <- 0x0 (bit[0] <- 0 exit reset) */
        stack_p->ext_info.write_fn(cookie, 0x18100800, 0);

        /* initialize SOCRAM */
        /* enable clock */
        stack_p->ext_info.write_fn(cookie, 0x18107408, 1);
        /* exit reset */
        stack_p->ext_info.write_fn(cookie, 0x18107800, 0);

        /* set MIPS resetvec to start of SOCRAM */
        /* set resetvec to start of SOCRAM */
        stack_p->ext_info.write_fn(cookie, 0x18003004, 0xb9000000);

        /* Note: did not change state of MIPS core reset.  If was running, is still running */
        /* Config for Host <-> BCM53903 comms */
        esw_set_ext_stack_config_array(stack_p, CONFIG_HOST_OFFSET, stack_p->ext_info.host_mac, 6);

        esw_set_ext_stack_config_array(stack_p, CONFIG_HOST_OFFSET + 8, stack_p->ext_info.top_mac, 6);

        esw_set_ext_stack_config_uint32(stack_p, CONFIG_HOST_OFFSET + 16, stack_p->ext_info.host_ip_addr);

        esw_set_ext_stack_config_uint32(stack_p, CONFIG_HOST_OFFSET + 20, stack_p->ext_info.top_ip_addr);

        tpidvlan = 0x81000000 + ((int)(stack_p->ext_info.vlan_pri) << 13) + stack_p->ext_info.vlan;

        esw_set_ext_stack_config_uint32(stack_p, CONFIG_HOST_OFFSET + 24, tpidvlan);

        /* Config for BCM53903 that is currently hardwired on host side */
        /* outer / inner TPIDs for VLAN */
        esw_set_ext_stack_config_uint32(stack_p, CONFIG_VLAN_OFFSET, 0x91008100);
        /* MPLS label ethertype */
        esw_set_ext_stack_config_uint32(stack_p, CONFIG_MPLS_OFFSET, 0x88470000);
        
#if 0
        /* TODO: use stack_p->ext_info.port, not hardcode */
        SOC_PBMP_PORT_ADD(pbmp, 0x03008000); 
#endif
        /* Load the firmware */
        if (BCM_FAILURE(rv = _bcm_ptp_ext_stack_firmware_load(stack_p, 
                ext_stack_fw_bin_len, ext_stack_fw_bin, 0)))
            PTP_ERROR_FUNC("_bcm_ptp_ext_stack_firmware_load() failed\n");
            return rv;
    }
    return rv;
#else
    return BCM_E_UNAVAIL;
#endif
}

#define MAGIC_READ_VALUE    0xdeadbeef

#if defined(PTP_KEYSTONE_STACK)

/*
 * Function:
 *      _bcm_ptp_ext_stack_firmware_load
 * Purpose:
 *      Load PTP external stack firmware onto RCPU.  BCM53903 only.
 * Parameters:
 *      unit     - (IN) switch unit 
 *      size     - (IN) image size
 *      image    - (IN) binary image 
 *      mode     - (IN) BCM_PTP_WARMSTART / BCM_PTP_COLDSTART
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int
_bcm_ptp_ext_stack_firmware_load(
    _bcm_ptp_stack_info_t *stack_p,
    int size, 
    uint8 *image, 
    _bcmPTPBootType_t mode)
{
    int rv = BCM_E_NONE;
    int boot_iter;
    void *cookie;
    uint32 value = MAGIC_READ_VALUE;

    cookie = stack_p->ext_info.cookie;
#if 0
    /* TODO add fault mgmt and check for fault condition */
    /* Cancel fault checks while we wait for the system to boot*/
    sal_dpc_cancel(&check_fault_condition);
#endif

    switch (mode) 
    {
        /* Currently, warmstart and coldstart are equivalent.
         * warmstart is TBD
         */
    case BCM_PTP_WARMSTART:
    case BCM_PTP_COLDSTART:
        /* corecontrol (0x18003000) <- 0x07: reset */
        stack_p->ext_info.write_fn(cookie, 0x18003000, 7);
        _bcm_ptp_write_pcishared_uint8_aligned_array(&stack_p->ext_info, 0x19000000, image, size);
        _bcm_ptp_write_pcishared_uint8_aligned_array(&stack_p->ext_info, CONFIG_BASE, stack_p->persistent_config, CONFIG_TOTAL_SIZE);
        /* corecontrol (0x18003000) <- 0x06: take out of reset */
        stack_p->ext_info.write_fn(cookie, 0x18003000, 6);

        for (boot_iter = 0; boot_iter < MAX_BOOT_ITER; ++boot_iter) 
        {
            stack_p->ext_info.read_fn(cookie, BOOT_STATUS_ADDR, &value);
            if (value != MAGIC_READ_VALUE) {
                    break;
            }

            /* delay 1 ms */
            sal_usleep(1000);
        }

        if (boot_iter == MAX_BOOT_ITER) {
            rv = BCM_E_FAIL;
            SOC_DEBUG_PRINT((DK_ERR | DK_VERBOSE,
                "%s() failed %s\n", __func__, 
                "external stack firmware load failed"));
        }
        break;
    
    default:
        rv  = BCM_E_PARAM;
        break;
    }
    return rv;
}

#endif /* defined(PTP_KEYSTONE_STACK) */

#endif /* defined(INCLUDE_PTP)*/
