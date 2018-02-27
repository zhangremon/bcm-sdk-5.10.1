/*
 * $Id: qe2000_intr.h 1.4.62.1 Broadcom SDK $
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
#ifndef _QE2000_INTR_H
#define _QE2000_INTR_H

#include <sal/types.h>

void soc_qe2000_pci_error0(int unit, uint32 ignored);
void soc_qe2000_pci_error1(int unit, uint32 ignored);
void soc_qe2000_dma_done_intr(int unit, uint32 ignored);
void soc_qe2000_qmgr_intr(int unit, uint32 ignored);
void soc_qe2000_qmgr_intr_enable(int unit);

#define  SBQE_PCI_ERR1_GET_LARGE_DROP(r)	    (((r) >> 24) & 0xff)
#define  SBQE_PCI_ERR1_GET_EOP_MISSING(r)	    (((r) >> 20) & 0x0f)
#define  SBQE_PCI_ERR1_GET_EXTRA_SOP(r)		    (((r) >> 16) & 0x0f)
#define  SBQE_PCI_ERR1_GET_RXPKT_ABRT(r)	    (((r) >> 12) & 0x0f)
#define  SBQE_PCI_ERR1_GET_DROP_SMALL_PKT(r)    (((r) >> 0) & 0x0f)

/* Keep these interfaces for now so that merge won't break customer exisiting code */
#ifndef TO_BE_DEPRECIATED_CODE
#define TO_BE_DEPRECIATED_CODE
#endif 
#ifdef TO_BE_DEPRECIATED_CODE
void soc_sbx_qe2000_pci_error0(int unit, uint32 ignored);
void soc_sbx_qe2000_pci_error1(int unit, uint32 ignored);
#endif

#endif /* _QE2000_INTR_H */
