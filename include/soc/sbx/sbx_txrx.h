/*
 * $Id: sbx_txrx.h 1.7 Broadcom SDK $
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
 */
#ifndef _SBX_TXRX_H
#define _SBX_TXRX_H

#include <sal/types.h>
#include <soc/sbx/sbx_drv.h>

typedef enum SBX_rh_fields_e {
    SBX_rhf_queue_id,
    SBX_rhf_ksop,
    SBX_rhf_sdp,
    SBX_rhf_ecn,
    SBX_rhf_ect,
    SBX_rhf_test,
    SBX_rhf_length,
    SBX_rhf_length_adjust,
    SBX_rhf_ttl,
    SBX_rhf_s,
    SBX_rhf_fdp2,
    SBX_rhf_lbid,
    SBX_rhf_fcos,
    SBX_rhf_fcos2,
    SBX_rhf_fdp,
    SBX_rhf_rcos,
    SBX_rhf_rdp,
    SBX_rhf_ppeswop,
    SBX_rhf_sid,
    SBX_rhf_mc,
    SBX_rhf_outunion,
    SBX_rhf_invalid /* last */
} SBX_rh_fields_t;


extern int soc_sbx_hdr_field_set(int unit, uint8 *hdr, uint8 hdr_len,
                                 SBX_rh_fields_t field, uint32 val);

extern int soc_sbx_hdr_field_get(int unit, uint8 *hdr, uint8 hdr_len,
                                 SBX_rh_fields_t field, uint32 *val);


extern int soc_sbx_txrx_init(int unit);
extern int soc_sbx_txrx_init_hw_only(int unit);
extern int soc_sbx_txrx_uninit_hw_only(int unit);
extern int soc_sbx_txrx_give_rx_buffers(int unit, int bufs, void **bufps,
                                        soc_sbx_txrx_done_f donecb, 
                                        void **cookies);
extern int soc_sbx_txrx_remove_rx_buffers (int unit, int bufs);
extern int soc_sbx_txrx_tx(int unit, char *hdr, int hdrlen, int bufs,
                           void **bufps, int *buflens, soc_sbx_txrx_done_f 
                           donecb, void *cookie);
extern int soc_sbx_txrx_sync_tx(int unit, char *hdr, int hdrlen, char *buf,
                                int buflen, int waitusec);
int soc_sbx_txrx_sync_rx(int unit, char *buf, int *buflen, int waitusec);
extern void soc_sbx_txrx_intr(int unit, uint32 unused);

#endif /* _SBX_TXRX_H */
