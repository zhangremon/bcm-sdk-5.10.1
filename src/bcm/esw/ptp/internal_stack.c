/*
 * $Id: internal_stack.c 1.1.2.7 Broadcom SDK $
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

#include <sal/core/dpc.h>

#include <bcm/ptp.h>
#include <bcm_int/esw/ptp.h>
#include <bcm/error.h>

#if defined(BCM_KATANA_SUPPORT)

#include <bcm_int/esw/katana.h>

#endif /* defined(BCM_KATANA_SUPPORT) */

/*
 * Function:
 *      _bcm_ptp_internal_stack_create
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
_bcm_ptp_internal_stack_create(
    int unit, 
    bcm_ptp_stack_info_t *info,
    bcm_ptp_stack_id_t ptp_id)
{

#if defined(BCM_KATANA_SUPPORT)
    int rv = BCM_E_NONE;
    _bcm_ptp_info_t *ptp_info_p;
    _bcm_ptp_stack_info_t *stack_p;
    int i;

	if (!SOC_IS_KATANA(unit)) {
		return BCM_E_UNAVAIL;
	}

    SET_PTP_INFO;
    if (soc_feature(unit, soc_feature_ptp))
    {
        stack_p = &ptp_info_p->stack_info[ptp_id];

        /* Set up dispatch for internal transport */
        stack_p->transport_init = _bcm_ptp_internal_transport_init;
        stack_p->tx = _bcm_ptp_internal_tx;
        stack_p->rx_free = _bcm_ptp_internal_rx_response_free;
        /* TODO */
        /* stack_p->transport_terminate = _bcm_ptp_internal_transport_terminate; */

        /* allocate space for mboxes */
        stack_p->int_state.mbox = sal_alloc(sizeof(_bcm_ptp_internal_stack_mbox_t), "ptp msg");
        if (!stack_p->int_state.mbox) {
            return BCM_E_MEMORY;
        }
        /* clear state of message mboxes */
        stack_p->int_state.mbox->num_buffers = soc_ntohl(BCM_PTP_MAX_BUFFERS);
        for (i = 0; i < BCM_PTP_MAX_BUFFERS; ++i) {
            stack_p->int_state.mbox->status[i] = MBOX_STATUS_EMPTY;
            stack_p->int_state.mbox->clock_num[i] = 0;
        }

        /* allocate space for debug log */
        /* size is the size of the structure without the placeholder space for debug->buf, plus the real space for it */
        stack_p->int_state.log = sal_alloc(sizeof(_bcm_ptp_internal_stack_log_t)
                                           - sizeof(stack_p->int_state.log->buf)
                                           + BCM_PTP_MAX_LOG, "ptp log");
        if (!stack_p->int_state.log) {
            sal_free(stack_p->int_state.mbox);
            return BCM_E_MEMORY;
        }

        /* initialize debug */
        stack_p->int_state.log->size = soc_htonl(BCM_PTP_MAX_LOG);
        stack_p->int_state.log->head = 0;
        stack_p->int_state.log->tail = 0;

        /* set up the network-byte-order pointers so that CMICm can access the shared memory */
        stack_p->int_state.mbox_ptr = soc_htonl((uint32)stack_p->int_state.mbox);
        stack_p->int_state.log_ptr = soc_htonl((uint32)stack_p->int_state.log);

        soc_cm_print("DEBUG SPACE: %p\n", (void *)stack_p->int_state.log->buf);
    }


    return rv;

#else /* defined(BCM_KATANA_SUPPORT) */

    return BCM_E_UNAVAIL;

#endif /* defined(BCM_KATANA_SUPPORT) */

}

#endif /* defined(INCLUDE_PTP) && defined(BCM_KATANA_SUPPORT)*/
